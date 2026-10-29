#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include "export.h"
#include "inst.h"
#include "engine.h"

static int track_limit(const Project *p) { return p->moreTracks ? ENG_MAX_TRACKS : ENG_GAME_TRACKS; }
static int exported_tracks(const Project *p, const Conv *c) { return c->ntracks < track_limit(p) ? c->ntracks : track_limit(p); }

void export_warnings(const Project *p, const Conv *c, char *buf, int n)
{
    buf[0] = 0;
    if (!p->target[0])
        snprintf(buf + strlen(buf), n - strlen(buf), "It replaces no game song yet (SONG > Replaces, or name the file after one). ");
    if (c->ntracks > track_limit(p))
        snprintf(buf + strlen(buf), n - strlen(buf), "Only %d tracks fit: %d will be left out (mute some). ",
                 track_limit(p), c->ntracks - track_limit(p));

    if (c->dropped)
        snprintf(buf + strlen(buf), n - strlen(buf), "%d track(s) over %d were left out. ", c->dropped, CONV_MAX_OUT);
}

// ------------------------------------------------------------------ .kaisong

typedef struct { uint8_t *b; size_t n, cap; } Buf;

static void bput(Buf *b, const void *d, size_t n)
{
    if (b->n + n > b->cap) { b->cap = (b->n + n) * 2 + 256; b->b = realloc(b->b, b->cap); }
    memcpy(b->b + b->n, d, n);
    b->n += n;
}
static void b8(Buf *b, int v) { uint8_t x = (uint8_t)v; bput(b, &x, 1); }
static void b16(Buf *b, int v) { b8(b, v & 255); b8(b, (v >> 8) & 255); }
static void b32(Buf *b, uint32_t v) { b16(b, (int)(v & 0xFFFF)); b16(b, (int)(v >> 16)); }
static void bpad(Buf *b) { while (b->n & 3) b8(b, 0); }

typedef struct {
    const char *names[256];
    int nnames;
    int custom[INST_MAX_CUSTOM];   // custom refs in SMPL order
    int ncustom;
    int waves[INST_MAX_CUSTOM];    // custom wave refs in WAVE order
    int nwaves;
    int groups[64];          // sf group index for kaisong groups 1..
    int ngroups;
    int keysplits[16];
    int nks;
} Tables;

static int name_index(Tables *t, const char *name)
{
    for (int i = 0; i < t->nnames; i++) if (!strcmp(t->names[i], name)) return i;
    if (t->nnames == 256) return 0;
    t->names[t->nnames] = name;
    return t->nnames++;
}

static void voice_record(Buf *b, Tables *t, const SfVoice *v, int top)
{
    static char symbuf[256][96];
    int kind = 0, ref = 0, ks = 0;
    SfVoice vv = *v;
    if ((vv.type & 0xC0) && !top) vv = VOICE_PLACEHOLDER;   // the game never plays a kit inside a kit
    if (vv.type & 0xC0) {
        int g = -1;
        for (int i = 0; i < t->ngroups; i++) if (t->groups[i] == vv.sub) g = i;
        if (g < 0 && t->ngroups < 64) { g = t->ngroups; t->groups[t->ngroups++] = vv.sub; }
        ref = g + 1;
        kind = 4;
        if (vv.type & 0x40) {
            int k = -1;
            for (int i = 0; i < t->nks; i++) if (t->keysplits[i] == vv.ks) k = i;
            if (k < 0 && t->nks < 16) { k = t->nks; t->keysplits[t->nks++] = vv.ks; }
            ks = k;
            kind = 5;
        }
    } else if ((vv.type & 7) == 3 && inst_is_custom_ref(vv.ref)) {
        kind = 7;
        int k = -1;
        for (int i = 0; i < t->nwaves; i++) if (t->waves[i] == vv.ref) k = i;
        if (k < 0) { k = t->nwaves; t->waves[t->nwaves++] = vv.ref; }
        ref = k;
    } else if ((vv.type & 7) == 3) {
        kind = 2;
        ref = name_index(t, sf_waves[vv.ref < 0 ? 0 : vv.ref].name);
    } else if (vv.type & 7) {
        kind = 3;
        ref = vv.ref < 0 ? 0 : vv.ref;
    } else if (inst_is_custom_ref(vv.ref)) {
        kind = 6;
        int k = -1;
        for (int i = 0; i < t->ncustom; i++) if (t->custom[i] == vv.ref) k = i;
        if (k < 0) { k = t->ncustom; t->custom[t->ncustom++] = vv.ref; }
        ref = k;
    } else {
        kind = 1;
        int s = vv.ref < 0 ? 0 : vv.ref;
        int i = t->nnames;
        snprintf(symbuf[i & 255], sizeof(symbuf[0]), "DirectSoundWaveData_%s", sf_samples[s].name);
        ref = name_index(t, symbuf[i & 255]);
    }
    uint8_t a = vv.attack, d = vv.decay, s = vv.sustain, r = vv.release;
    if (vv.type & 0xC0) a = d = s = r = 0;
    b8(b, vv.type); b8(b, vv.key); b8(b, vv.length); b8(b, vv.panSweep);
    b8(b, a); b8(b, d); b8(b, s); b8(b, r);
    b8(b, kind); b8(b, 0);
    b16(b, ref); b16(b, ks); b16(b, 0);
}

static void chunk(Buf *out, const char *id, Buf *data)
{
    bput(out, id, 4);
    b32(out, (uint32_t)data->n);
    bput(out, data->b, data->n);
    bpad(out);
    free(data->b);
    memset(data, 0, sizeof(*data));
}

int export_kaisong(const Project *p, const Conv *c, const char *path, char *err, int errlen)
{
    Tables t = {0};
    Buf info = {0}, names = {0}, grp = {0}, ksp = {0}, trk = {0}, out = {0};
    int ntr = exported_tracks(p, c);

    // group 0: the song's voicegroup (sub-groups get added to t as they are met)
    Buf g0 = {0};
    for (int s = 0; s < 128; s++) {
        const SfVoice *v = c->slotUsed[s] ? &p->voice[s] : &VOICE_PLACEHOLDER;
        voice_record(&g0, &t, v, 1);
    }
    Buf gs = {0};
    for (int i = 0; i < t.ngroups; i++)
        for (int s = 0; s < 128; s++)
            voice_record(&gs, &t, &group_voices(t.groups[i])[s], 0);
    b32(&grp, (uint32_t)(1 + t.ngroups));
    bput(&grp, g0.b, g0.n);
    if (gs.n) bput(&grp, gs.b, gs.n);
    free(g0.b);
    free(gs.b);

    b32(&ksp, (uint32_t)t.nks);
    for (int i = 0; i < t.nks; i++) bput(&ksp, keysplit_table(t.keysplits[i]), 128);

    // the song's own samples, laid out like the game's WaveData (header, then the 8-bit PCM)
    Buf smpl = {0}, wave = {0};
    b32(&smpl, (uint32_t)t.ncustom);
    for (int i = 0; i < t.ncustom; i++) {
        const CustomSample *cs = inst_custom_get(t.custom[i]);
        int l = (int)strlen(cs->name);
        b16(&smpl, l);
        bput(&smpl, cs->name, (size_t)l);
        bpad(&smpl);
        b16(&smpl, 0);                              // WaveData.type
        b16(&smpl, cs->loops ? 0x4000 : 0);         // WaveData.status: loops
        b32(&smpl, cs->freq);
        b32(&smpl, cs->loopStart);
        b32(&smpl, cs->size);
        bput(&smpl, cs->data, cs->size);
        bpad(&smpl);
    }
    b32(&wave, (uint32_t)t.nwaves);
    for (int i = 0; i < t.nwaves; i++) {
        const CustomSample *cs = inst_custom_get(t.waves[i]);
        int l = (int)strlen(cs->name);
        b16(&wave, l);
        bput(&wave, cs->name, (size_t)l);
        bpad(&wave);
        bput(&wave, cs->data, 16);
    }
    int custom = t.ncustom || t.nwaves;

    b32(&names, (uint32_t)t.nnames);
    for (int i = 0; i < t.nnames; i++) {
        int l = (int)strlen(t.names[i]);
        b16(&names, l);
        bput(&names, t.names[i], (size_t)l);
    }

    b32(&trk, (uint32_t)ntr);
    for (int i = 0; i < ntr; i++) {
        b32(&trk, (uint32_t)c->len[i]);
        b32(&trk, (uint32_t)c->nreloc[i]);
        bput(&trk, c->data[i], (size_t)c->len[i]);
        for (int r = 0; r < c->nreloc[i]; r++) b32(&trk, (uint32_t)c->reloc[i][r]);
        bpad(&trk);
    }

    char line[600];
    char when[32];
    time_t now = time(NULL);
    strftime(when, sizeof(when), "%Y-%m-%d %H:%M", localtime(&now));
    snprintf(line, sizeof(line),
             "song=%s\ntitle=%s\nmidi=%s\nvolume=%d\nreverb=%d\npriority=%d\nloop=%s\nvoices=%d\ntool=Moemon Music Studio %s\nmade=%s\n",
             p->target, p->title, p->midiName, p->masterVol, p->reverb, p->priority, c->hasLoop ? "yes" : "no", p->voices,
             STUDIO_VERSION, when);
    bput(&info, line, strlen(line));
    if (p->place[0]) {
        snprintf(line, sizeof(line), "place=%s\n", p->place);
        bput(&info, line, strlen(line));
    }
    if (p->isRip) {
        snprintf(line, sizeof(line), "source=%s\n", p->ripInfo);
        bput(&info, line, strlen(line));
    }

    bput(&out, "KAISONG", 8);              // 8 bytes with the terminating 0
    b16(&out, custom ? KAISONG_VERSION_CUSTOM : KAISONG_VERSION);
    b16(&out, 32);                         // header size
    b8(&out, ntr);
    b8(&out, p->priority);
    b8(&out, project_reverb_byte(p));
    b8(&out, p->masterVol);
    b32(&out, custom ? 7 : 5);             // chunk count
    while (out.n < 32) b8(&out, 0);
    chunk(&out, "INFO", &info);
    chunk(&out, "NAME", &names);
    chunk(&out, "VGRP", &grp);
    chunk(&out, "KSPL", &ksp);
    chunk(&out, "TRAK", &trk);
    if (custom) {
        chunk(&out, "SMPL", &smpl);
        chunk(&out, "WAVE", &wave);
    } else {
        free(smpl.b);
        free(wave.b);
    }

    FILE *f = fopen(path, "wb");
    if (!f) { snprintf(err, errlen, "can't write %s", path); free(out.b); return 0; }
    int ok = fwrite(out.b, 1, out.n, f) == out.n;
    if (fclose(f) != 0) ok = 0;
    free(out.b);
    if (!ok) { snprintf(err, errlen, "couldn't finish writing %s", path); return 0; }
    return 1;
}

// ------------------------------------------------------------------ decomp sources

static const char *cmd_name(uint8_t op)
{
    switch (op) {
    case 0xB1: return "FINE"; case 0xB2: return "GOTO"; case 0xB3: return "PATT"; case 0xB4: return "PEND";
    case 0xB5: return "REPT"; case 0xB9: return "MEMACC"; case 0xBA: return "PRIO"; case 0xBB: return "TEMPO";
    case 0xBC: return "KEYSH"; case 0xBD: return "VOICE"; case 0xBE: return "VOL"; case 0xBF: return "PAN";
    case 0xC0: return "BEND"; case 0xC1: return "BENDR"; case 0xC2: return "LFOS"; case 0xC3: return "LFODL";
    case 0xC4: return "MOD"; case 0xC5: return "MODT"; case 0xC8: return "TUNE"; case 0xCC: return "PORT";
    case 0xCD: return "XCMD"; case 0xCE: return "EOT"; case 0xCF: return "TIE";
    }
    return NULL;
}

static int is_reloc(const Conv *c, int t, int off)
{
    for (int r = 0; r < c->nreloc[t]; r++) if (c->reloc[t][r] == off) return 1;
    return 0;
}

static uint32_t rd32le(const uint8_t *p) { return (uint32_t)p[0] | p[1] << 8 | p[2] << 16 | (uint32_t)p[3] << 24; }

static void write_track_asm(FILE *f, const Conv *c, int t, const char *label)
{
    const uint8_t *d = c->data[t];
    int n = c->len[t];
    uint8_t *isTarget = calloc((size_t)n + 1, 1);
    for (int r = 0; r < c->nreloc[t]; r++) {
        uint32_t to = rd32le(d + c->reloc[t][r]);
        if (to <= (uint32_t)n) isTarget[to] = 1;
    }
    fprintf(f, "\n/********************** Track %2d **********************/\n\n%s_%d:\n", t + 1, label, t + 1);
    int i = 0;
    while (i < n) {
        if (isTarget[i]) fprintf(f, "%s_%d_L%d:\n", label, t + 1, i);
        uint8_t op = d[i++];
        if (op >= 0x80 && op <= 0xB0) {
            fprintf(f, "\t.byte\tW%02d\n", sf_gClockTable[op - 0x80]);
            continue;
        }
        const char *nm = cmd_name(op);
        char name[16];
        if (op > 0xCF) { snprintf(name, sizeof(name), "N%02d", sf_gClockTable[op - 0xCF]); nm = name; }
        if (!nm) { fprintf(f, "\t.byte\t0x%02X\n", op); continue; }
        fprintf(f, "\t.byte\t\t%s", nm);
        if (op == 0xB2 || op == 0xB3) {
            fprintf(f, "\n\t ptrvalue\t%s_%d_L%u\n", label, t + 1, rd32le(d + i));
            i += 4;
            continue;
        }
        if (op == 0xB9) {   // MEMACC op, addr, data [, target]
            int mop = d[i];
            fprintf(f, ", %d, %d, %d\n", d[i], d[i + 1], d[i + 2]);
            i += 3;
            if (mop >= 6 && is_reloc(c, t, i)) {
                fprintf(f, "\t ptrvalue\t%s_%d_L%u\n", label, t + 1, rd32le(d + i));
                i += 4;
            }
            continue;
        }
        if (op == 0xCD) { fprintf(f, ", %s, %d\n", d[i] == 8 ? "xIECV" : d[i] == 9 ? "xIECL" : "0", d[i + 1]); i += 2; continue; }
        if (op == 0xBB) { fprintf(f, ", %d*%s_tbs/2\n", d[i] * 2, label); i++; continue; }
        if (op == 0xBF || op == 0xC0 || op == 0xC8) { fprintf(f, ", c_v%+d\n", d[i] - 64); i++; continue; }
        // everything else: data bytes below 0x80 that follow (notes: key, velocity, gate time)
        int argc = (op >= 0xCE) ? 3 : (op == 0xB1 || op == 0xB4) ? 0 : 1;
        if (op == 0xCE) argc = 1;
        for (int a = 0; a < argc && i < n && d[i] < 0x80; a++) {
            if (op >= 0xCF && a == 1) fprintf(f, ", v%03d", d[i]);
            else if (op >= 0xCF && a == 2) fprintf(f, ", gtp%d", d[i]);
            else fprintf(f, ", %d", d[i]);
            i++;
        }
        fprintf(f, "\n");
    }
    free(isTarget);
}

static const char *voice_macro_line(const SfVoice *v, char *buf, int n)
{
    int pan = v->panSweep & 0x80 ? v->panSweep & 0x7F : 0;
    int cpan = v->length & 0x80 ? v->length & 0x7F : 0;
    if (v->type & 0x80)
        snprintf(buf, n, "voice_keysplit_all voicegroup_%s", group_voices(v->sub) ? group_name(v->sub) : "dummy");
    else if (v->type & 0x40)
        snprintf(buf, n, "voice_keysplit voicegroup_%s, keysplit_%s", group_name(v->sub), keysplit_name(v->ks));
    else switch (v->type & 7) {
    case 1:
        snprintf(buf, n, "voice_square_1%s %d, %d, %d, %d, %d, %d, %d, %d", v->type & 8 ? "_alt" : "", v->key, cpan,
                 v->panSweep, v->ref & 3, v->attack, v->decay, v->sustain, v->release);
        break;
    case 2:
        snprintf(buf, n, "voice_square_2%s %d, %d, %d, %d, %d, %d, %d", v->type & 8 ? "_alt" : "", v->key, cpan,
                 v->ref & 3, v->attack, v->decay, v->sustain, v->release);
        break;
    case 3:
        if (inst_is_custom_ref(v->ref)) {
            snprintf(buf, n, "voice_programmable_wave%s %d, %d, ProgrammableWaveData_%s, %d, %d, %d, %d", v->type & 8 ? "_alt" : "",
                     v->key, cpan, inst_custom_get(v->ref)->name, v->attack, v->decay, v->sustain, v->release);
            break;
        }
        snprintf(buf, n, "voice_programmable_wave%s %d, %d, %s, %d, %d, %d, %d", v->type & 8 ? "_alt" : "", v->key, cpan,
                 sf_waves[v->ref < 0 ? 0 : v->ref].name, v->attack, v->decay, v->sustain, v->release);
        break;
    case 4:
        snprintf(buf, n, "voice_noise%s %d, %d, %d, %d, %d, %d, %d", v->type & 8 ? "_alt" : "", v->key, cpan,
                 v->ref & 1, v->attack, v->decay, v->sustain, v->release);
        break;
    default:
        snprintf(buf, n, "voice_directsound%s %d, %d, DirectSoundWaveData_%s, %d, %d, %d, %d",
                 v->type & 0x10 ? "_alt" : v->type & 8 ? "_no_resample" : "", v->key, pan,
                 inst_is_custom_ref(v->ref) ? inst_custom_get(v->ref)->name : sf_samples[v->ref < 0 ? 0 : v->ref].name,
                 v->attack, v->decay, v->sustain, v->release);
        break;
    }
    return buf;
}

static void put16(FILE *f, int v) { fputc(v & 255, f); fputc((v >> 8) & 255, f); }
static void put32(FILE *f, uint32_t v) { put16(f, (int)(v & 0xFFFF)); put16(f, (int)(v >> 16)); }

// An 8-bit WAV the game's wav2agb turns back into exactly this sample: 'smpl' has the loop and
// 'agbp' the exact pitch value (WaveData.freq).
static int write_sample_wav(const char *path, const CustomSample *cs)
{
    FILE *f = fopen(path, "wb");
    if (!f) return 0;
    uint32_t rate = (uint32_t)(cs->freq / 1024.0 + 0.5);
    if (!rate) rate = 13379;
    uint32_t smplSize = 36 + (cs->loops ? 24 : 0);
    uint32_t dataPad = cs->size & 1;
    uint32_t riff = 4 + 8 + 16 + 8 + cs->size + dataPad + 8 + smplSize + 8 + 4;
    fwrite("RIFF", 1, 4, f); put32(f, riff); fwrite("WAVE", 1, 4, f);
    fwrite("fmt ", 1, 4, f); put32(f, 16); put16(f, 1); put16(f, 1); put32(f, rate); put32(f, rate); put16(f, 1); put16(f, 8);
    fwrite("data", 1, 4, f); put32(f, cs->size);
    for (uint32_t i = 0; i < cs->size; i++) fputc((uint8_t)(cs->data[i] + 128), f);   // WAV 8-bit is unsigned
    if (dataPad) fputc(0, f);
    fwrite("smpl", 1, 4, f); put32(f, smplSize);
    put32(f, 0); put32(f, 0); put32(f, 1000000000u / rate); put32(f, 60); put32(f, 0); put32(f, 0); put32(f, 0);
    put32(f, cs->loops ? 1 : 0); put32(f, 0);
    if (cs->loops) { put32(f, 0); put32(f, 0); put32(f, cs->loopStart); put32(f, cs->size - 1); put32(f, 0); put32(f, 0); }
    fwrite("agbp", 1, 4, f); put32(f, 4); put32(f, cs->freq);
    return fclose(f) == 0;
}

// The song's own samples: a WAV (or a 16-byte .pcm wave) each, and the labels for the game's
// sound/direct_sound_data.inc and sound/programmable_wave_data.inc.
static int export_decomp_samples(const Project *p, const Conv *c, const char *dir, const char *label, char *err, int errlen)
{
    int used[INST_MAX_CUSTOM] = {0}, any = 0;
    for (int s = 0; s < 128; s++) {
        if (!c->slotUsed[s]) continue;
        const SfVoice *v = &p->voice[s];
        if ((v->type & 0xC0) || !(((v->type & 7) == 0) || ((v->type & 7) == 3))) continue;
        if (inst_is_custom_ref(v->ref)) { used[v->ref - INST_CUSTOM] = 1; any = 1; }
    }
    if (!any) return 1;
    char path[1200];
    snprintf(path, sizeof(path), "%s/%s_samples.inc", dir, label);
    FILE *inc = fopen(path, "w");
    if (!inc) { snprintf(err, errlen, "can't write %s", path); return 0; }
    fprintf(inc, "/* The samples %s brings with it. Put the WAVs in sound/direct_sound_samples/ (wav2agb converts\n"
                 "   them) and the .pcm waves in sound/programmable_wave_samples/, then add these to\n"
                 "   sound/direct_sound_data.inc and sound/programmable_wave_data.inc. */\n", label);
    for (int i = 0; i < inst_custom_count(); i++) {
        if (!used[i]) continue;
        const CustomSample *cs = inst_custom_get(INST_CUSTOM + i);
        if (cs->isWave) {
            snprintf(path, sizeof(path), "%s/%s.pcm", dir, cs->name);
            FILE *w = fopen(path, "wb");
            if (!w || fwrite(cs->data, 1, 16, w) != 16) { if (w) fclose(w); fclose(inc); snprintf(err, errlen, "can't write %s", path); return 0; }
            fclose(w);
            fprintf(inc, "\n\t.align 2\nProgrammableWaveData_%s::\n\t.incbin \"sound/programmable_wave_samples/%s.pcm\"\n", cs->name, cs->name);
        } else {
            snprintf(path, sizeof(path), "%s/%s.wav", dir, cs->name);
            if (!write_sample_wav(path, cs)) { fclose(inc); snprintf(err, errlen, "can't write %s", path); return 0; }
            fprintf(inc, "\n\t.align 2\nDirectSoundWaveData_%s::\n\t.incbin \"sound/direct_sound_samples/%s.bin\"\n", cs->name, cs->name);
        }
    }
    if (fclose(inc) != 0) { snprintf(err, errlen, "couldn't finish writing the sample list"); return 0; }
    return 1;
}

int export_decomp(const Project *p, const Conv *c, const char *dir, char *err, int errlen)
{
    char path[1200], label[80], vg[96];
    snprintf(label, sizeof(label), "%s", p->target[0] ? p->target : "mus_custom");
    snprintf(vg, sizeof(vg), "%s_kai", label);
    int ntr = exported_tracks(p, c);

    snprintf(path, sizeof(path), "%s/%s.s", dir, label);
    FILE *f = fopen(path, "w");
    if (!f) { snprintf(err, errlen, "can't write %s", path); return 0; }
    fprintf(f, "/* %s, made by Moemon Music Studio %s from %s */\n", p->title, STUDIO_VERSION, p->midiName);
    fprintf(f, "\t.include \"asm/macros/bit_width.inc\"\n\n\t.include \"MPlayDef.s\"\n\n");
    fprintf(f, "\t.equ\t%s_grp, voicegroup_%s\n", label, vg);
    fprintf(f, "\t.equ\t%s_pri, %d\n", label, p->priority);
    if (p->reverb >= 0) fprintf(f, "\t.equ\t%s_rev, reverb_set+%d\n", label, p->reverb);
    else fprintf(f, "\t.equ\t%s_rev, 0\n", label);
    fprintf(f, "\t.equ\t%s_mvl, %d\n", label, p->masterVol);
    fprintf(f, "\t.equ\t%s_key, 0\n", label);
    fprintf(f, "\t.equ\t%s_tbs, 1\n", label);
    fprintf(f, "\t.equ\t%s_exg, %d\n", label, p->exactGate);
    fprintf(f, "\t.equ\t%s_cmp, 0\n", label);
    fprintf(f, "\n\t.section .rodata\n\t.global\t%s\n\t.align\t2\n", label);
    for (int t = 0; t < ntr; t++) write_track_asm(f, c, t, label);
    fprintf(f, "\n/******************************************************/\n\t.align\t2\n\n%s:\n", label);
    fprintf(f, "\t.byte\t%d\t/* NumTrks */\n\t.byte\t0\t/* NumBlks */\n\t.byte\t%s_pri\t/* Priority */\n\t.byte\t%s_rev\t/* Reverb */\n",
            ntr, label, label);
    fprintf(f, "space64 4\n\tptrvalue\t%s_grp\n\n", label);
    for (int t = 0; t < ntr; t++) fprintf(f, "\tptrvalue\t%s_%d\n", label, t + 1);
    fprintf(f, "\n\t.end\n");
    int ok = fclose(f) == 0;

    snprintf(path, sizeof(path), "%s/%s.inc", dir, vg);
    f = fopen(path, "w");
    if (!f) { snprintf(err, errlen, "can't write %s", path); return 0; }
    fprintf(f, "/* Instruments for %s (%s), made by Moemon Music Studio %s */\n", label, p->title, STUDIO_VERSION);
    fprintf(f, "voice_group %s\n", vg);
    for (int s = 0; s < 128; s++) {
        char line[300];
        const SfVoice *v = c->slotUsed[s] ? &p->voice[s] : &VOICE_PLACEHOLDER;
        fprintf(f, "\t%s\n", voice_macro_line(v, line, sizeof(line)));
    }
    // the song's own drum kits / key-split groups and their tables
    int doneG[64], ndg = 0, doneK[64], ndk = 0;
    for (int s = 0; s < 128; s++) {
        const SfVoice *v = &p->voice[s];
        if (!c->slotUsed[s] || !(v->type & 0xC0)) continue;
        if (inst_is_custom_group(v->sub)) {
            int seen = 0;
            for (int i = 0; i < ndg; i++) if (doneG[i] == v->sub) seen = 1;
            if (!seen && ndg < 64) {
                doneG[ndg++] = v->sub;
                fprintf(f, "\n/* a group the song brings */\nvoice_group %s\n", group_name(v->sub));
                const SfVoice *g = group_voices(v->sub);
                for (int k = 0; k < 128; k++) {
                    char line[300];
                    SfVoice gv = g[k];
                    if (gv.type & 0xC0) gv = VOICE_PLACEHOLDER;
                    fprintf(f, "\t%s\n", voice_macro_line(&gv, line, sizeof(line)));
                }
            }
        }
        if ((v->type & 0x40) && inst_is_custom_split(v->ks)) {
            int seen = 0;
            for (int i = 0; i < ndk; i++) if (doneK[i] == v->ks) seen = 1;
            if (!seen && ndk < 64) {
                doneK[ndk++] = v->ks;
                const uint8_t *t = keysplit_table(v->ks);
                fprintf(f, "\n/* a key-split table the song brings (sound/keysplit_tables.inc form) */\nkeysplit %s, 0\n", keysplit_name(v->ks));
                for (int k = 0; k < 128; k++)
                    if (k == 127 || t[k + 1] != t[k]) fprintf(f, "\tsplit %d, %d\n", t[k], k);
            }
        }
    }
    if (fclose(f) != 0) ok = 0;
    if (!ok) { snprintf(err, errlen, "couldn't finish writing the files in %s", dir); return 0; }
    return export_decomp_samples(p, c, dir, label, err, errlen);
}

// ------------------------------------------------------------------ the song table (SONG_TABLE.md)

void export_default_name(const Project *p, char *buf, int n, const char *ext)
{
    if (p->target[0] && p->place[0]) snprintf(buf, n, "%s@%s%s", p->target, p->place, ext);
    else snprintf(buf, n, "%s%s", p->target[0] ? p->target : p->title[0] ? p->title : "song", ext);
}

typedef struct { char file[256], song[128], place[128], title[256]; } TableSong;

static int cmp_table_song(const void *a, const void *b) { return strcmp(((const TableSong *)a)->file, ((const TableSong *)b)->file); }

static void trim(char *s)
{
    size_t n = strlen(s), a = 0;
    while (n && (s[n - 1] == ' ' || s[n - 1] == '\t' || s[n - 1] == '\r' || s[n - 1] == '\n')) s[--n] = 0;
    while (s[a] == ' ' || s[a] == '\t') a++;
    memmove(s, s + a, n - a + 1);
}

// INFO title= of a .kaisong ("" if none)
static void kaisong_title(const char *path, char *title, int n)
{
    title[0] = 0;
    FILE *f = fopen(path, "rb");
    if (!f) return;
    uint8_t h[32];
    if (fread(h, 1, 32, f) == 32 && !memcmp(h, "KAISONG", 8)) {
        unsigned off = h[10] | (h[11] << 8);
        uint8_t ch[8];
        if (fseek(f, off, SEEK_SET) == 0 && fread(ch, 1, 8, f) == 8 && !memcmp(ch, "INFO", 4)) {
            uint32_t len = ch[4] | (ch[5] << 8) | (ch[6] << 16) | ((uint32_t)ch[7] << 24);
            char *txt = len < (1u << 20) ? malloc(len + 1) : NULL;
            if (txt && fread(txt, 1, len, f) == len) {
                txt[len] = 0;
                for (char *line = txt, *next; line; line = next) {
                    next = strchr(line, '\n');
                    if (next) *next++ = 0;
                    char *eq = strchr(line, '=');
                    if (!eq) continue;
                    *eq = 0;
                    char k[128], v[512];
                    snprintf(k, sizeof(k), "%s", line);
                    snprintf(v, sizeof(v), "%s", eq + 1);
                    trim(k); trim(v);
                    if (!strcmp(k, "title")) snprintf(title, n, "%s", v);   // the last one wins, as in gen.py's dict
                }
            }
            free(txt);
        }
    }
    fclose(f);
}

int export_song_table(const char *dir, char *err, int errlen)
{
    TableSong *ts = NULL;
    int nts = 0;
    DIR *d = opendir(dir);
    if (d) {
        struct dirent *e;
        while ((e = readdir(d)) != NULL) {
            size_t l = strlen(e->d_name);
            if (l <= 8 || strcmp(e->d_name + l - 8, ".kaisong") != 0 || l >= sizeof(ts[0].file)) continue;
            ts = realloc(ts, (nts + 1) * sizeof(*ts));
            TableSong *t = &ts[nts++];
            memset(t, 0, sizeof(*t));
            snprintf(t->file, sizeof(t->file), "%s", e->d_name);
            char stem[256];
            snprintf(stem, sizeof(stem), "%.*s", (int)(l - 8), e->d_name);
            char *at = strchr(stem, '@');
            if (at) { *at = 0; snprintf(t->place, sizeof(t->place), "%s", at + 1); }
            snprintf(t->song, sizeof(t->song), "%s", stem);
            char path[2048];
            snprintf(path, sizeof(path), "%s/%s", dir, e->d_name);
            kaisong_title(path, t->title, sizeof(t->title));
            for (char *q = t->title; *q; q++) if (*q == '|') *q = '/';
        }
        closedir(d);
    }
    if (nts) qsort(ts, nts, sizeof(*ts), cmp_table_song);

    char path[2048];
    snprintf(path, sizeof(path), "%s/SONG_TABLE.md", dir);
    FILE *f = fopen(path, "wb");
    if (!f) { snprintf(err, errlen, "can't write %s", path); free(ts); return 0; }
    // the same lines as tools/kaisong/gen.py table_md()
    fputs(sf_table_intro, f);
    fputs("## Custom music in this version\n\n", f);
    if (nts) {
        fputs("| file | replaces | where | title |\n|---|---|---|---|\n", f);
        for (int i = 0; i < nts; i++) {
            const char *where = ts[i].place[0] ? ts[i].place : "everywhere";
            for (int m = 0; m < SF_NUM_MAPS && ts[i].place[0]; m++)
                if (!strcmp(sf_maps[m].key, ts[i].place)) where = sf_maps[m].name;
            fprintf(f, "| `%s` | %s | %s | %s |\n", ts[i].file, ts[i].song, where, ts[i].title);
        }
    } else fputs("None yet.\n", f);
    fputs("\n## Songs\n\n", f);
    for (int s = 0; s < SF_NUM_SONGS; s++) {
        fprintf(f, "### %s (%d)\n", sf_songs[s].name, sf_song_num[s]);
        int first = 1;
        for (int i = 0; i < nts; i++) {
            if (strcmp(ts[i].song, sf_songs[s].name)) continue;
            fprintf(f, "%s`%s`", first ? "Custom: " : ", ", ts[i].file);
            first = 0;
        }
        if (!first) fputc('\n', f);
        fputs(sf_song_md[s], f);
        if (s + 1 < SF_NUM_SONGS) fputc('\n', f);
    }
    free(ts);
    if (fclose(f) != 0) { snprintf(err, errlen, "couldn't finish writing %s", path); return 0; }
    return 1;
}
