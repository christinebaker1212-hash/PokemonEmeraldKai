#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "export.h"
#include "inst.h"
#include "engine.h"

static int track_limit(const Project *p) { return p->moreTracks ? ENG_MAX_TRACKS : ENG_GAME_TRACKS; }
static int exported_tracks(const Project *p, const Conv *c) { return c->ntracks < track_limit(p) ? c->ntracks : track_limit(p); }

void export_warnings(const Project *p, const Conv *c, char *buf, int n)
{
    buf[0] = 0;
    if (!p->target[0])
        snprintf(buf + strlen(buf), n - strlen(buf), "Pick the game song it replaces. ");
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
    } else if ((vv.type & 7) == 3) {
        kind = 2;
        ref = name_index(t, sf_waves[vv.ref < 0 ? 0 : vv.ref].name);
    } else if (vv.type & 7) {
        kind = 3;
        ref = vv.ref < 0 ? 0 : vv.ref;
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
            voice_record(&gs, &t, &sf_groups[t.groups[i]][s], 0);
    b32(&grp, (uint32_t)(1 + t.ngroups));
    bput(&grp, g0.b, g0.n);
    if (gs.n) bput(&grp, gs.b, gs.n);
    free(g0.b);
    free(gs.b);

    b32(&ksp, (uint32_t)t.nks);
    for (int i = 0; i < t.nks; i++) bput(&ksp, sf_keysplits[t.keysplits[i]], 128);

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

    bput(&out, "KAISONG", 8);              // 8 bytes with the terminating 0
    b16(&out, KAISONG_VERSION);
    b16(&out, 32);                         // header size
    b8(&out, ntr);
    b8(&out, p->priority);
    b8(&out, project_reverb_byte(p));
    b8(&out, p->masterVol);
    b32(&out, 5);                          // chunk count
    while (out.n < 32) b8(&out, 0);
    chunk(&out, "INFO", &info);
    chunk(&out, "NAME", &names);
    chunk(&out, "VGRP", &grp);
    chunk(&out, "KSPL", &ksp);
    chunk(&out, "TRAK", &trk);

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
        snprintf(buf, n, "voice_keysplit_all voicegroup_%s", v->sub >= 0 ? sf_group_names[v->sub] : "dummy");
    else if (v->type & 0x40)
        snprintf(buf, n, "voice_keysplit voicegroup_%s, keysplit_%s", sf_group_names[v->sub], sf_keysplit_names[v->ks]);
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
                 sf_samples[v->ref < 0 ? 0 : v->ref].name, v->attack, v->decay, v->sustain, v->release);
        break;
    }
    return buf;
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
    if (fclose(f) != 0) ok = 0;
    if (!ok) { snprintf(err, errlen, "couldn't finish writing the files in %s", dir); return 0; }
    return 1;
}
