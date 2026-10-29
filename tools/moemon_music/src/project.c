#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "project.h"
#include "inst.h"
#include "rip.h"
#include "rip_song.h"
#include "rip_mp2k.h"
#include "rip_gm.h"

int read_file(const char *path, uint8_t **data, size_t *len)
{
    FILE *f = fopen(path, "rb");
    if (!f) return 0;
    fseek(f, 0, SEEK_END);
    long n = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (n < 0) { fclose(f); return 0; }
    *data = malloc((size_t)n + 1);
    *len = fread(*data, 1, (size_t)n, f);
    (*data)[*len] = 0;
    fclose(f);
    return *len == (size_t)n;
}

const char *path_base(const char *path)
{
    const char *b = path;
    for (const char *p = path; *p; p++) if (*p == '/' || *p == '\\') b = p + 1;
    return b;
}

void project_init(Project *p)
{
    memset(p, 0, sizeof(*p));
    for (int i = 0; i < 128; i++) { p->ripGm[i].type = 0xFF; p->ripGmGain[i] = 1; }
    p->ripGain = 1;
    p->ripCgbNorm = 1;
    p->masterVol = 100;
    p->reverb = 50;
    p->priority = 0;
    p->exactGate = 1;
    p->gmFixes = 1;
    p->loopMode = LOOP_MARKERS;
    p->gbaAccurate = 0;          // off unless turned on: songs play and export as they are
    project_apply_mode(p);
    p->balance = 1;
    p->drumsFirst = 1;
    for (int i = 0; i < 128; i++) p->voice[i] = VOICE_PLACEHOLDER;
    for (int i = 0; i < CONV_MAX_CAND; i++) p->trackLock[i] = -1;
}

void project_apply_mode(Project *p)
{
    p->moreTracks = !p->gbaAccurate;
    p->voices = p->gbaAccurate ? 5 : 64;
}

void project_free(Project *p)
{
    midi_free(&p->midi);
    free(p->midiData);
    if (p->rip) { rip_free(p->rip); free(p->rip); }
    custom_list_free(&p->custom, &p->ncustom);
    if (p->cgroup || p->csplit) inst_custom_groups_bind(NULL, 0, NULL, 0);
    free(p->cgroup);
    free(p->csplit);
    free(p->changeSlot);
    project_init(p);
}

void project_conv_opts(const Project *p, ConvOpts *o)
{
    memset(o, 0, sizeof(*o));
    o->masterVol = p->masterVol;
    o->exactGate = p->exactGate;
    o->gmFixes = p->gmFixes;
    o->loopMode = p->loopMode;
    o->loopStart = p->loopStart;
    o->loopEnd = p->loopEnd;
    o->exclude = p->muted;
    o->voices = p->voice;
    o->drumsFirst = p->drumsFirst;
    o->candLock = p->trackLock;
    o->changeSlot = p->changeSlot;
    o->nchangeSlot = p->nchangeSlot;
    for (int s = 0; s < 128; s++) o->slotTranspose[s] = (int8_t)project_slot_transpose(p, s);
}

static int is_wave(const SfVoice *v) { return !(v->type & 0xC0) && (v->type & 7) == 3; }

void project_set_change(Project *p, int cand, int index, int slot)
{
    for (int j = 0; j < p->nchangeSlot; j++)
        if (p->changeSlot[j].cand == cand && p->changeSlot[j].index == index) {
            if (slot < 0) p->changeSlot[j] = p->changeSlot[--p->nchangeSlot];
            else p->changeSlot[j].slot = (int16_t)slot;
            p->dirty = 1;
            return;
        }
    if (slot < 0) return;
    ConvChangeSet *n = realloc(p->changeSlot, sizeof(ConvChangeSet) * (size_t)(p->nchangeSlot + 1));
    if (!n) return;
    p->changeSlot = n;
    ConvChangeSet c = {(int16_t)cand, (int16_t)index, (int16_t)slot};
    p->changeSlot[p->nchangeSlot++] = c;
    p->dirty = 1;
}

int project_free_slot(const Project *p, const Conv *c)
{
    for (int s = 1; s < 128; s++) {
        if (c->slotUsed[s] || p->voiceSet[s]) continue;
        int taken = 0;
        for (int k = 0; k < c->ncand; k++) {
            if (c->cand[k].program == s || p->trackLock[k] == s) taken = 1;
            for (int j = 0; j < c->cand[k].nchanges; j++) if (c->cand[k].changes[j].from == s) taken = 1;
        }
        for (int j = 0; j < p->nchangeSlot; j++) if (p->changeSlot[j].slot == s) taken = 1;
        if (!taken) return s;
    }
    return -1;
}

int project_slot_transpose(const Project *p, int s)
{
    const SfVoice *v = &p->voice[s];
    if (v->type & 0x80) return 0;                      // a drum kit: the key picks the drum
    int t = 12 * p->octave[s];
    if (p->isRip && p->ripKind != RIP_GSF) t += 12 * (is_wave(v) - is_wave(&p->ripOrig[s]));
    return t;
}

uint8_t project_reverb_byte(const Project *p)
{
    return p->reverb < 0 ? 0 : (uint8_t)(0x80 | (p->reverb & 0x7F));
}

void project_default_voices(Project *p, const Conv *c, int force)
{
    for (int s = 0; s < 128; s++) {
        if (!c->slotUsed[s] || (p->voiceSet[s] && !force)) continue;
        int preset = c->slotDrum[s] ? SF_GM_DRUM_PRESET : sf_gm_preset[c->slotGm[s] >= 0 ? c->slotGm[s] : s];
        p->voice[s] = sf_presets[preset].voice;
        p->voiceSet[s] = 1;
    }
}

void project_use_song_group(Project *p, const Conv *c, int group)
{
    for (int s = 0; s < 128; s++) {
        if (!c->slotUsed[s]) continue;
        p->voice[s] = sf_song_groups[group][s];
        p->voiceSet[s] = 1;
    }
    p->dirty = 1;
}

static void set_title_from(Project *p, const char *name)
{
    snprintf(p->title, sizeof(p->title), "%.63s", name);
    char *dot = strrchr(p->title, '.');
    if (dot) *dot = 0;
}

static int load_midi_bytes(Project *p, char *err, int errlen)
{
    midi_free(&p->midi);
    return midi_parse(&p->midi, p->midiData, p->midiLen, err, errlen);
}

int project_load_midi(Project *p, const char *path, char *err, int errlen)
{
    uint8_t *data;
    size_t len;
    if (!read_file(path, &data, &len)) { snprintf(err, errlen, "can't read %s", path); return 0; }
    Project np;
    project_init(&np);
    np.midiData = data;
    np.midiLen = len;
    snprintf(np.midiName, sizeof(np.midiName), "%s", path_base(path));
    if (!load_midi_bytes(&np, err, errlen)) { free(data); return 0; }
    set_title_from(&np, np.midiName);
    // one of the game's own MIDIs: its settings and instruments, converted exactly like the game build
    char stem[256];
    snprintf(stem, sizeof(stem), "%s", np.midiName);
    char *dot = strrchr(stem, '.');
    if (dot) *dot = 0;
    for (char *q = stem; *q; q++) *q = (char)tolower(*q);
    int song = song_by_name(stem);
    if (song >= 0) {
        const SfSong *s = &sf_songs[song];
        snprintf(np.target, sizeof(np.target), "%s", s->name);
        np.masterVol = s->vol;
        np.reverb = s->reverb;
        np.priority = s->priority;
        np.exactGate = s->exact;
        np.gmFixes = 0;
        np.balance = 0;
        int g = song_group_by_name(s->group);
        if (g >= 0) {
            Conv c;
            ConvOpts o;
            project_conv_opts(&np, &o);
            if (conv_build(&c, &np.midi, &o, err, errlen)) {
                project_use_song_group(&np, &c, g);
                conv_free(&c);
            }
        }
    }
    project_free(p);
    *p = np;
    p->dirty = 0;
    return 1;
}

// ------------------------------------------------------------------ rips

void project_rip_instruments(Project *p, int useGm)
{
    p->ripUseGm = useGm;
    for (int s = 0; s < 128; s++) {
        if (!p->voiceSet[s]) continue;
        if (useGm && p->ripGm[s].type != 0xFF) p->voice[s] = p->ripGm[s];
        else p->voice[s] = p->ripOrig[s];
    }
    p->dirty = 1;
}

int project_reload_rip(Project *p, const char *biosDir, char *err, int errlen)
{
    if (p->rip) return 1;
    if (!p->isRip || !p->ripPath[0]) { snprintf(err, errlen, "this project isn't from a rip"); return 0; }
    Rip *r = calloc(1, sizeof(Rip));
    if (!r) { snprintf(err, errlen, "out of memory"); return 0; }
    if (!rip_load(r, p->ripPath, biosDir, p->ripKind == RIP_GBS ? p->ripTrack : -1, 0, err, errlen)) { free(r); return 0; }
    p->rip = r;
    return 1;
}

int project_rip_sync_drums(Project *p)
{
    if (!p->isRip || p->ripKind != RIP_GBS || !p->rip) return 0;
    int want = 0;
    for (int s = 0; s < 128; s++)
        if (p->voiceSet[s] && (p->ripOrig[s].type & 7) == 4 && p->ripGm[s].type != 0xFF && voice_same(&p->voice[s], &p->ripGm[s])) want = 1;
    if (want == p->ripGbaDrums) return 0;
    p->ripGbaDrums = want;
    char err[256];
    if (!project_rip_regain(p, p->ripGain, err, sizeof(err))) { p->ripGbaDrums = !want; return 0; }
    return 1;
}

void project_bind(Project *p)
{
    inst_custom_bind(p->custom, p->ncustom);
    inst_custom_groups_bind(p->cgroup, p->ncgroup, p->csplit, p->ncsplit);
}

int project_rip_regain(Project *p, float gain, char *err, int errlen)
{
    if (!p->rip) { snprintf(err, errlen, "the rip isn't loaded"); return 0; }
    if (p->ripKind == RIP_GSF) { snprintf(err, errlen, "GBA songs play at their own volume"); return 0; }
    RipSong song;
    if (!rip_song_build(p->rip, &song, gain, p->ripGbaDrums, err, errlen)) return 0;
    Midi m;
    if (!midi_parse(&m, song.mid, song.midLen, err, errlen)) { rip_song_free(&song); return 0; }
    midi_free(&p->midi);
    free(p->midiData);
    p->midi = m;
    p->midiData = song.mid;
    p->midiLen = song.midLen;
    song.mid = NULL;
    // the same instruments in the same order: swap the sample data in
    custom_list_free(&p->custom, &p->ncustom);
    p->custom = song.custom;
    p->ncustom = song.ncustom;
    song.custom = NULL;
    song.ncustom = 0;
    p->ripGain = gain;
    project_bind(p);
    rip_song_free(&song);
    return 1;
}

// A GBA song on the game's own engine: its commands and instruments as they are (no recording needed).
static int load_gsf(Project *p, Rip *r, const char *path, char *err, int errlen)
{
    if (!r->mp2kHeader || !r->rom) {
        snprintf(err, errlen, "this GBA game doesn't use the game's sound engine (MP2K), so its notes can't be read");
        rip_free(r);
        free(r);
        return 0;
    }
    RipSong pre;
    memset(&pre, 0, sizeof(pre));
    // names for the song's own samples: from the title
    char prefix[24] = "gba";
    {
        int k = 0;
        for (const char *q = r->title; *q && k < 14; q++) {
            unsigned char ch = (unsigned char)*q;
            if (isalnum(ch)) prefix[k++] = (char)tolower(ch);
            else if (k && prefix[k - 1] != '_') prefix[k++] = '_';
        }
        while (k && prefix[k - 1] == '_') k--;
        if (k) prefix[k] = 0; else strcpy(prefix, "gba");
        if (isdigit((unsigned char)prefix[0])) { memmove(prefix + 1, prefix, strlen(prefix) + 1); prefix[0] = 'g'; }
    }
    Mp2kSong song;
    if (!rip_mp2k_song(r->rom, r->romLen, r->mp2kHeader, prefix, &song, err, errlen)) { rip_free(r); free(r); return 0; }
    Project np;
    project_init(&np);
    np.midiData = song.mid;
    np.midiLen = song.midLen;
    song.mid = NULL;
    snprintf(np.midiName, sizeof(np.midiName), "%s.mid", prefix);
    np.custom = song.custom; np.ncustom = song.ncustom; song.custom = NULL; song.ncustom = 0;
    np.cgroup = song.groups; np.ncgroup = song.ngroups; song.groups = NULL; song.ngroups = 0;
    np.csplit = song.splits; np.ncsplit = song.nsplits; song.splits = NULL; song.nsplits = 0;
    project_bind(&np);
    if (!load_midi_bytes(&np, err, errlen)) { project_free(&np); project_bind(p); rip_free(r); free(r); return 0; }
    snprintf(np.title, sizeof(np.title), "%.63s", r->title);
    np.isRip = 1;
    np.ripKind = r->kind;
    snprintf(np.ripPath, sizeof(np.ripPath), "%s", path);
    snprintf(np.ripInfo, sizeof(np.ripInfo), "%s%s%s%s%s", r->game, r->game[0] && r->artist[0] ? " - " : "", r->artist,
             (r->game[0] || r->artist[0]) ? " - " : "", r->console);
    // converted exactly like the game's own songs: the same engine, the same commands
    np.gmFixes = 0;
    np.exactGate = 1;
    np.balance = 0;
    np.drumsFirst = 0;
    np.masterVol = 127;
    np.priority = song.priority;
    np.reverb = song.reverb;
    np.loopMode = song.hasLoop ? LOOP_MARKERS : LOOP_NONE;
    for (int s = 0; s < 128; s++) {
        np.voice[s] = song.voice[s];
        np.ripOrig[s] = song.voice[s];
        np.voiceSet[s] = !voice_same(&song.voice[s], &VOICE_PLACEHOLDER);
    }
    rip_mp2k_free(&song);
    (void)pre;
    np.rip = r;
    project_free(p);
    *p = np;
    project_bind(p);
    p->dirty = 1;
    return 1;
}

int project_load_rip(Project *p, const char *path, const char *biosDir, int gbsTrack, char *err, int errlen)
{
    Rip *r = calloc(1, sizeof(Rip));
    if (!r) { snprintf(err, errlen, "out of memory"); return 0; }
    err[0] = 0;
    if (!rip_load(r, path, biosDir, gbsTrack, 0, err, errlen)) { free(r); return 0; }
    if (r->kind == RIP_GSF) return load_gsf(p, r, path, err, errlen);
    RipSong song;
    if (!rip_song_build(r, &song, 1.0f, 0, err, errlen)) { rip_free(r); free(r); return 0; }
    Project np;
    project_init(&np);
    np.midiData = song.mid;
    np.midiLen = song.midLen;
    song.mid = NULL;
    snprintf(np.midiName, sizeof(np.midiName), "%s.mid", song.prefix);
    if (!load_midi_bytes(&np, err, errlen)) { free(np.midiData); rip_free(r); free(r); return 0; }
    snprintf(np.title, sizeof(np.title), "%.63s", r->title);
    np.isRip = 1;
    np.ripKind = r->kind;
    np.ripTrack = r->track;
    snprintf(np.ripPath, sizeof(np.ripPath), "%s", path);
    snprintf(np.ripInfo, sizeof(np.ripInfo), "%s%s%s%s%s", r->game, r->game[0] && r->artist[0] ? " - " : "", r->artist,
             (r->game[0] || r->artist[0]) ? " - " : "", r->console);
    np.hifi = 1;               // the originals in 16 bits (GBA accurate turns it off)
    np.ripNativeRate = r->nativeRate > 0 ? r->nativeRate : r->rate;
    np.gmFixes = 1;            // expression, bend range, notes clipped at the loop end
    np.exactGate = 1;
    np.balance = 0;
    np.drumsFirst = 0;
    np.masterVol = 127;
    np.reverb = song.reverb;
    np.loopMode = song.hasLoop ? LOOP_CUSTOM : LOOP_NONE;
    np.loopStart = song.loopStart;
    np.loopEnd = song.loopEnd;
    for (int s = 0; s < 128; s++) {
        np.ripOrig[s] = song.orig[s];
        if (song.slotSample[s] >= 0) { np.voice[s] = song.orig[s]; np.voiceSet[s] = 1; }
    }
    // the closest General MIDI instruments of the game (the "All GM" set)
    {
        static RipGm gm;
        inst_custom_bind(song.custom, song.ncustom);      // the original voices are measured against the GM ones
        rip_gm_match(r, &song, &gm);
        for (int s = 0; s < 128; s++) { np.ripGm[s] = gm.voice[s]; np.ripGmGain[s] = gm.gain[s]; }
        np.cgroup = gm.groups;
        np.ncgroup = gm.ngroups;
        np.ripCgbNorm = gm.cgbNorm;
    }
    np.rip = r;
    np.custom = song.custom;
    np.ncustom = song.ncustom;
    song.custom = NULL;
    song.ncustom = 0;
    rip_song_free(&song);
    project_free(p);
    *p = np;
    project_bind(p);
    p->dirty = 1;
    return 1;
}

// ------------------------------------------------------------------ project file

static const char B64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static void b64_write(FILE *f, const uint8_t *d, size_t n)
{
    size_t col = 0;
    for (size_t i = 0; i < n; i += 3) {
        uint32_t v = (uint32_t)d[i] << 16 | (i + 1 < n ? d[i + 1] << 8 : 0) | (i + 2 < n ? d[i + 2] : 0);
        char o[4] = {B64[v >> 18 & 63], B64[v >> 12 & 63], i + 1 < n ? B64[v >> 6 & 63] : '=', i + 2 < n ? B64[v & 63] : '='};
        fwrite(o, 1, 4, f);
        if ((col += 4) >= 76) { fputc('\n', f); col = 0; }
    }
    if (col) fputc('\n', f);
}

static size_t b64_read(const char *s, uint8_t *out)
{
    uint32_t acc = 0;
    int bits = 0;
    size_t n = 0;
    for (; *s; s++) {
        const char *q = strchr(B64, *s);
        if (!q || !*s) continue;
        acc = acc << 6 | (uint32_t)(q - B64);
        bits += 6;
        if (bits >= 8) { bits -= 8; out[n++] = (uint8_t)(acc >> bits); }
    }
    return n;
}

static const char *LOOP_NAMES[] = {"markers", "none", "whole", "custom"};

int project_save(Project *p, const char *path, char *err, int errlen)
{
    FILE *f = fopen(path, "wb");
    if (!f) { snprintf(err, errlen, "can't write %s", path); return 0; }
    fprintf(f, "MoemonMusicStudio 1\n");
    fprintf(f, "# Moemon Music Studio project. Text lines are settings; the MIDI is kept at the end.\n");
    fprintf(f, "title=%s\n", p->title);
    fprintf(f, "song=%s\n", p->target);
    fprintf(f, "place=%s\n", p->place);
    fprintf(f, "volume=%d\n", p->masterVol);
    fprintf(f, "reverb=%d\n", p->reverb);
    fprintf(f, "priority=%d\n", p->priority);
    fprintf(f, "exact_gate=%d\n", p->exactGate);
    fprintf(f, "gm_fixes=%d\n", p->gmFixes);
    fprintf(f, "loop=%s\n", LOOP_NAMES[p->loopMode]);
    fprintf(f, "gba_accurate=%d\n", p->gbaAccurate);
    fprintf(f, "hifi=%d\n", p->hifi);
    fprintf(f, "balance=%d\n", p->balance);
    fprintf(f, "drums_first=%d\n", p->drumsFirst);
    fprintf(f, "loop_start=%u\n", p->loopStart);
    fprintf(f, "loop_end=%u\n", p->loopEnd);
    fprintf(f, "muted=");
    int first = 1;
    for (int i = 0; i < 64; i++) if ((p->muted >> i) & 1) { fprintf(f, first ? "%d" : ",%d", i); first = 0; }
    fprintf(f, "\n");
    if (p->isRip) {
        fprintf(f, "rip_kind=%d\n", p->ripKind);
        fprintf(f, "rip_path=%s\n", p->ripPath);
        fprintf(f, "rip_track=%d\n", p->ripTrack);
        fprintf(f, "rip_info=%s\n", p->ripInfo);
        fprintf(f, "rip_gain=%.6f\n", p->ripGain);
        fprintf(f, "rip_use_gm=%d\n", p->ripUseGm);
        fprintf(f, "rip_cgbnorm=%.6f\n", p->ripCgbNorm);
        fprintf(f, "rip_gba_drums=%d\n", p->ripGbaDrums);
        fprintf(f, "rip_native_rate=%d\n", p->ripNativeRate);
    }
    for (int s = 0; s < 128; s++) if (p->octave[s]) fprintf(f, "octave%d=%d\n", s, p->octave[s]);
    for (int k = 0; k < CONV_MAX_CAND; k++) if (p->trackLock[k] >= 0) fprintf(f, "track_lock%d=%d\n", k, p->trackLock[k]);
    for (int j = 0; j < p->nchangeSlot; j++)
        fprintf(f, "change=%d,%d,%d\n", p->changeSlot[j].cand, p->changeSlot[j].index, p->changeSlot[j].slot);
    // custom samples before the voices that use them
    for (int i = 0; i < p->ncustom; i++) {
        const CustomSample *c = &p->custom[i];
        fprintf(f, "custom=%s,%u,%u,%d,%d,%u\n", c->name, c->freq, c->loopStart, c->loops, c->isWave, c->size);
        b64_write(f, (const uint8_t *)c->data, c->isWave ? 16 : c->size);
        fprintf(f, "end\n");
        if (c->hi && c->hiSize) {
            // the Hi-Fi copy: 16-bit little-endian
            fprintf(f, "customhi=%u,%u,%u\n", c->hiFreq, c->hiLoopStart, c->hiSize);
            uint8_t *le = malloc((size_t)c->hiSize * 2);
            if (le) {
                for (uint32_t i = 0; i < c->hiSize; i++) { le[i * 2] = (uint8_t)(c->hi[i] & 0xFF); le[i * 2 + 1] = (uint8_t)((uint16_t)c->hi[i] >> 8); }
                b64_write(f, le, (size_t)c->hiSize * 2);
                free(le);
            }
            fprintf(f, "end\n");
        }
    }
    for (int i = 0; i < p->ncsplit; i++) {
        fprintf(f, "split=%s", p->csplit[i].name);
        for (int k = 0; k < 128; k++) fprintf(f, ",%d", p->csplit[i].t[k]);
        fprintf(f, "\n");
    }
    for (int i = 0; i < p->ncgroup; i++) {
        fprintf(f, "group=%s\n", p->cgroup[i].name);
        for (int k = 0; k < 128; k++) {
            if (voice_same(&p->cgroup[i].v[k], &VOICE_PLACEHOLDER)) continue;
            char t[300];
            voice_to_text(&p->cgroup[i].v[k], t, sizeof(t));
            fprintf(f, "g%d=%s\n", k, t);
        }
        fprintf(f, "end\n");
    }
    for (int s = 0; s < 128; s++) {
        if (!p->voiceSet[s]) continue;
        char t[300];
        voice_to_text(&p->voice[s], t, sizeof(t));
        fprintf(f, "voice%d=%s\n", s, t);
    }
    if (p->isRip) {
        for (int s = 0; s < 128; s++) {
            char t[300];
            if (p->voiceSet[s]) { voice_to_text(&p->ripOrig[s], t, sizeof(t)); fprintf(f, "rip_orig%d=%s\n", s, t); }
            if (p->voiceSet[s] && p->ripGm[s].type != 0xFF) {
                voice_to_text(&p->ripGm[s], t, sizeof(t));
                fprintf(f, "rip_gm%d=%s\n", s, t);
                fprintf(f, "rip_gmgain%d=%.4f\n", s, p->ripGmGain[s]);
            }
        }
    }
    fprintf(f, "midi_name=%s\n", p->midiName);
    fprintf(f, "midi=\n");
    b64_write(f, p->midiData, p->midiLen);
    fprintf(f, "end\n");
    int ok = !ferror(f);
    if (fclose(f) != 0) ok = 0;
    if (!ok) { snprintf(err, errlen, "couldn't finish writing %s", path); return 0; }
    snprintf(p->path, sizeof(p->path), "%s", path);
    p->dirty = 0;
    return 1;
}

int project_load(Project *p, const char *path, char *err, int errlen)
{
    uint8_t *data;
    size_t len;
    if (!read_file(path, &data, &len)) { snprintf(err, errlen, "can't read %s", path); return 0; }
    char *text = (char *)data;
    if (strncmp(text, "MoemonMusicStudio ", 18)) { snprintf(err, errlen, "%s isn't a Moemon Music Studio project", path_base(path)); free(data); return 0; }
    Project np;
    project_init(&np);
    int inMidi = 0, inCustom = 0, inGroup = 0, inHi = 0, bad = 0, bound = 0;
    uint32_t hiFreq = 0, hiLoop = 0, hiSize = 0;
    char *midiText = NULL;
    size_t midiTextLen = 0;
    char *custText = NULL;
    size_t custTextLen = 0;
    CustomSample cur;
    memset(&cur, 0, sizeof(cur));
    for (char *line = strtok(text, "\n"); line; line = strtok(NULL, "\n")) {
        size_t l = strlen(line);
        if (l && line[l - 1] == '\r') line[--l] = 0;
        if (inMidi) {
            if (!strcmp(line, "end")) { inMidi = 0; continue; }
            midiText = realloc(midiText, midiTextLen + l + 1);
            memcpy(midiText + midiTextLen, line, l + 1);
            midiTextLen += l;
            continue;
        }
        if (inGroup) {
            if (!strcmp(line, "end")) { inGroup = 0; continue; }
            char *e2 = strchr(line, '=');
            int k = line[0] == 'g' ? atoi(line + 1) : -1;
            if (e2 && k >= 0 && k < 128) {
                if (!bound) { inst_custom_bind(np.custom, np.ncustom); bound = 1; }
                inst_custom_groups_bind(np.cgroup, np.ncgroup, np.csplit, np.ncsplit);
                if (!voice_from_text(e2 + 1, &np.cgroup[np.ncgroup - 1].v[k])) bad++;
            }
            continue;
        }
        if (inHi) {
            if (strcmp(line, "end")) {
                custText = realloc(custText, custTextLen + l + 1);
                memcpy(custText + custTextLen, line, l + 1);
                custTextLen += l;
                continue;
            }
            inHi = 0;
            uint8_t *d = calloc((size_t)hiSize * 2 + custTextLen + 4, 1);
            size_t got = custText && d ? b64_read(custText, d) : 0;
            if (np.ncustom > 0 && d && got >= (size_t)hiSize * 2) {
                CustomSample *c = &np.custom[np.ncustom - 1];
                free(c->hi);
                c->hi = calloc((size_t)hiSize + 8, sizeof(int16_t));
                if (c->hi) {
                    for (uint32_t i = 0; i < hiSize; i++) c->hi[i] = (int16_t)(d[i * 2] | d[i * 2 + 1] << 8);
                    c->hiSize = hiSize; c->hiLoopStart = hiLoop; c->hiFreq = hiFreq;
                }
            }
            free(d);
            free(custText);
            custText = NULL;
            custTextLen = 0;
            continue;
        }
        if (inCustom) {
            if (strcmp(line, "end")) {
                custText = realloc(custText, custTextLen + l + 1);
                memcpy(custText + custTextLen, line, l + 1);
                custTextLen += l;
                continue;
            }
            inCustom = 0;
            uint32_t want = cur.isWave ? 16 : cur.size;
            uint8_t *d = calloc((size_t)want + custTextLen + 4, 1);
            size_t got = custText ? b64_read(custText, d) : 0;
            if (got >= want) {
                cur.data = (int8_t *)d;
                if (custom_list_add(&np.custom, &np.ncustom, &cur) < 0) bad++;
            } else bad++;
            free(d);
            free(custText);
            custText = NULL;
            custTextLen = 0;
            continue;
        }
        char *eq = strchr(line, '=');
        if (!eq || line[0] == '#') continue;
        *eq = 0;
        const char *k = line, *v = eq + 1;
        if (!strcmp(k, "title")) snprintf(np.title, sizeof(np.title), "%s", v);
        else if (!strcmp(k, "song")) snprintf(np.target, sizeof(np.target), "%s", v);
        else if (!strcmp(k, "place")) snprintf(np.place, sizeof(np.place), "%s", v);
        else if (!strcmp(k, "volume")) np.masterVol = atoi(v);
        else if (!strcmp(k, "reverb")) np.reverb = atoi(v);
        else if (!strcmp(k, "priority")) np.priority = atoi(v);
        else if (!strcmp(k, "exact_gate")) np.exactGate = atoi(v);
        else if (!strcmp(k, "gm_fixes")) np.gmFixes = atoi(v);
        else if (!strcmp(k, "loop")) { for (int i = 0; i < 4; i++) if (!strcmp(v, LOOP_NAMES[i])) np.loopMode = i; }
        else if (!strcmp(k, "gba_accurate")) np.gbaAccurate = atoi(v) != 0;
        else if (!strcmp(k, "hifi")) np.hifi = atoi(v) != 0;
        else if (!strcmp(k, "more_tracks")) { if (atoi(v)) np.gbaAccurate = 0; }   // projects from before GBA accurate
        else if (!strcmp(k, "balance")) np.balance = atoi(v);
        else if (!strcmp(k, "drums_first")) np.drumsFirst = atoi(v);
        else if (!strcmp(k, "voices")) { if (atoi(v) > 5) np.gbaAccurate = 0; }
        else if (!strcmp(k, "loop_start")) np.loopStart = (uint32_t)strtoul(v, NULL, 10);
        else if (!strcmp(k, "loop_end")) np.loopEnd = (uint32_t)strtoul(v, NULL, 10);
        else if (!strcmp(k, "muted")) {
            for (const char *q = v; *q;) {
                int i = atoi(q);
                if (i >= 0 && i < 64) np.muted |= 1ull << i;
                while (*q && *q != ',') q++;
                if (*q) q++;
            }
        } else if (!strcmp(k, "split")) {
            CustomSplit *ns = realloc(np.csplit, sizeof(CustomSplit) * (size_t)(np.ncsplit + 1));
            if (ns) {
                np.csplit = ns;
                CustomSplit *cs = &np.csplit[np.ncsplit++];
                memset(cs, 0, sizeof(*cs));
                const char *q = v;
                int n = 0;
                while (*q && *q != ',' && n < 39) cs->name[n++] = *q++;
                for (int i = 0; i < 128 && *q == ','; i++) { q++; cs->t[i] = (uint8_t)atoi(q); while (*q && *q != ',') q++; }
            }
        } else if (!strcmp(k, "group")) {
            CustomGroup *ng = realloc(np.cgroup, sizeof(CustomGroup) * (size_t)(np.ncgroup + 1));
            if (ng) {
                np.cgroup = ng;
                CustomGroup *g = &np.cgroup[np.ncgroup++];
                snprintf(g->name, sizeof(g->name), "%s", v);
                for (int i = 0; i < 128; i++) g->v[i] = VOICE_PLACEHOLDER;
                inGroup = 1;
            }
        } else if (!strcmp(k, "custom")) {
            memset(&cur, 0, sizeof(cur));
            char nm[64] = {0};
            unsigned fr = 0, ls = 0, sz = 0;
            int lp = 0, wv = 0;
            if (sscanf(v, "%63[^,],%u,%u,%d,%d,%u", nm, &fr, &ls, &lp, &wv, &sz) == 6) {
                snprintf(cur.name, sizeof(cur.name), "%s", nm);
                cur.freq = fr; cur.loopStart = ls; cur.loops = lp; cur.isWave = wv; cur.size = sz;
                inCustom = 1;
            } else bad++;
        } else if (!strcmp(k, "customhi")) {
            if (sscanf(v, "%u,%u,%u", &hiFreq, &hiLoop, &hiSize) == 3 && hiSize > 0 && hiSize < 0x8000000) inHi = 1;
        } else if (!strncmp(k, "rip_gmgain", 10)) {
            int s = atoi(k + 10);
            if (s >= 0 && s < 128) np.ripGmGain[s] = (float)atof(v);
        } else if (!strncmp(k, "voice", 5) || !strncmp(k, "rip_orig", 8) || !strncmp(k, "rip_gm", 6)) {
            if (!bound) { inst_custom_bind(np.custom, np.ncustom); bound = 1; }
            inst_custom_groups_bind(np.cgroup, np.ncgroup, np.csplit, np.ncsplit);
            int isOrig = !strncmp(k, "rip_orig", 8), isGm = !strncmp(k, "rip_gm", 6);
            int s = atoi(k + (isOrig ? 8 : isGm ? 6 : 5));
            if (s >= 0 && s < 128) {
                SfVoice *dst = isOrig ? &np.ripOrig[s] : isGm ? &np.ripGm[s] : &np.voice[s];
                if (voice_from_text(v, dst)) { if (!isOrig && !isGm) np.voiceSet[s] = 1; }
                else bad++;
            }
        } else if (!strcmp(k, "rip_kind")) { np.isRip = 1; np.ripKind = atoi(v); }
        else if (!strcmp(k, "rip_path")) snprintf(np.ripPath, sizeof(np.ripPath), "%s", v);
        else if (!strcmp(k, "rip_track")) np.ripTrack = atoi(v);
        else if (!strcmp(k, "rip_info")) snprintf(np.ripInfo, sizeof(np.ripInfo), "%s", v);
        else if (!strcmp(k, "rip_gain")) np.ripGain = (float)atof(v);
        else if (!strcmp(k, "rip_cgbnorm")) np.ripCgbNorm = (float)atof(v);
        else if (!strcmp(k, "rip_gba_drums")) np.ripGbaDrums = atoi(v);
        else if (!strcmp(k, "rip_native_rate")) np.ripNativeRate = atoi(v);
        else if (!strncmp(k, "track_lock", 10)) {
            int c = atoi(k + 10), sl = atoi(v);
            if (c >= 0 && c < CONV_MAX_CAND && sl >= 0 && sl < 128) np.trackLock[c] = (int16_t)sl;
        } else if (!strcmp(k, "change")) {
            int c, ix, sl;
            if (sscanf(v, "%d,%d,%d", &c, &ix, &sl) == 3 && c >= 0 && c < CONV_MAX_CAND && ix >= 0 && sl >= 0 && sl < 128)
                project_set_change(&np, c, ix, sl);
        }
        else if (!strncmp(k, "octave", 6)) {
            int s = atoi(k + 6), o = atoi(v);
            if (s >= 0 && s < 128 && o >= -4 && o <= 4) np.octave[s] = (int8_t)o;
        }
        else if (!strcmp(k, "rip_use_gm")) np.ripUseGm = atoi(v); else if (!strcmp(k, "midi_name")) snprintf(np.midiName, sizeof(np.midiName), "%s", v);
        else if (!strcmp(k, "midi")) inMidi = 1;
    }
    free(data);
    free(custText);
    if (!midiText) { snprintf(err, errlen, "the project has no MIDI in it"); custom_list_free(&np.custom, &np.ncustom); free(np.cgroup); free(np.csplit); project_bind(p); return 0; }
    np.midiData = malloc(midiTextLen);
    np.midiLen = b64_read(midiText, np.midiData);
    free(midiText);
    if (!load_midi_bytes(&np, err, errlen)) { free(np.midiData); custom_list_free(&np.custom, &np.ncustom); free(np.cgroup); free(np.csplit); project_bind(p); return 0; }
    project_apply_mode(&np);
    if (np.masterVol < 0) np.masterVol = 0;
    if (np.masterVol > 127) np.masterVol = 127;
    project_free(p);
    *p = np;
    project_bind(p);
    snprintf(p->path, sizeof(p->path), "%s", path);
    p->dirty = 0;
    if (bad) snprintf(err, errlen, "%d instrument(s) in the project aren't in this version's sound set and were reset", bad);
    else err[0] = 0;
    return 1;
}
