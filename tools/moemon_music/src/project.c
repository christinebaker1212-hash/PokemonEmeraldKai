#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "project.h"
#include "inst.h"

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
    p->masterVol = 100;
    p->reverb = 50;
    p->priority = 0;
    p->exactGate = 1;
    p->gmFixes = 1;
    p->loopMode = LOOP_MARKERS;
    p->voices = 5;
    p->balance = 1;
    for (int i = 0; i < 128; i++) p->voice[i] = VOICE_PLACEHOLDER;
}

void project_free(Project *p)
{
    midi_free(&p->midi);
    free(p->midiData);
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
    fprintf(f, "volume=%d\n", p->masterVol);
    fprintf(f, "reverb=%d\n", p->reverb);
    fprintf(f, "priority=%d\n", p->priority);
    fprintf(f, "exact_gate=%d\n", p->exactGate);
    fprintf(f, "gm_fixes=%d\n", p->gmFixes);
    fprintf(f, "loop=%s\n", LOOP_NAMES[p->loopMode]);
    fprintf(f, "more_tracks=%d\n", p->moreTracks);
    fprintf(f, "voices=%d\n", p->voices);
    fprintf(f, "balance=%d\n", p->balance);
    fprintf(f, "loop_start=%u\n", p->loopStart);
    fprintf(f, "loop_end=%u\n", p->loopEnd);
    fprintf(f, "muted=");
    int first = 1;
    for (int i = 0; i < 64; i++) if ((p->muted >> i) & 1) { fprintf(f, first ? "%d" : ",%d", i); first = 0; }
    fprintf(f, "\n");
    for (int s = 0; s < 128; s++) {
        if (!p->voiceSet[s]) continue;
        char t[300];
        voice_to_text(&p->voice[s], t, sizeof(t));
        fprintf(f, "voice%d=%s\n", s, t);
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
    int inMidi = 0, bad = 0;
    char *midiText = NULL;
    size_t midiTextLen = 0;
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
        char *eq = strchr(line, '=');
        if (!eq || line[0] == '#') continue;
        *eq = 0;
        const char *k = line, *v = eq + 1;
        if (!strcmp(k, "title")) snprintf(np.title, sizeof(np.title), "%s", v);
        else if (!strcmp(k, "song")) snprintf(np.target, sizeof(np.target), "%s", v);
        else if (!strcmp(k, "volume")) np.masterVol = atoi(v);
        else if (!strcmp(k, "reverb")) np.reverb = atoi(v);
        else if (!strcmp(k, "priority")) np.priority = atoi(v);
        else if (!strcmp(k, "exact_gate")) np.exactGate = atoi(v);
        else if (!strcmp(k, "gm_fixes")) np.gmFixes = atoi(v);
        else if (!strcmp(k, "loop")) { for (int i = 0; i < 4; i++) if (!strcmp(v, LOOP_NAMES[i])) np.loopMode = i; }
        else if (!strcmp(k, "more_tracks")) np.moreTracks = atoi(v);
        else if (!strcmp(k, "balance")) np.balance = atoi(v);
        else if (!strcmp(k, "voices")) np.voices = atoi(v) < 1 ? 5 : atoi(v) > 12 ? 12 : atoi(v);
        else if (!strcmp(k, "loop_start")) np.loopStart = (uint32_t)strtoul(v, NULL, 10);
        else if (!strcmp(k, "loop_end")) np.loopEnd = (uint32_t)strtoul(v, NULL, 10);
        else if (!strcmp(k, "muted")) {
            for (const char *q = v; *q;) {
                int i = atoi(q);
                if (i >= 0 && i < 64) np.muted |= 1ull << i;
                while (*q && *q != ',') q++;
                if (*q) q++;
            }
        } else if (!strncmp(k, "voice", 5)) {
            int s = atoi(k + 5);
            if (s >= 0 && s < 128) {
                if (voice_from_text(v, &np.voice[s])) np.voiceSet[s] = 1;
                else bad++;
            }
        } else if (!strcmp(k, "midi_name")) snprintf(np.midiName, sizeof(np.midiName), "%s", v);
        else if (!strcmp(k, "midi")) inMidi = 1;
    }
    free(data);
    if (!midiText) { snprintf(err, errlen, "the project has no MIDI in it"); return 0; }
    np.midiData = malloc(midiTextLen);
    np.midiLen = b64_read(midiText, np.midiData);
    free(midiText);
    if (!load_midi_bytes(&np, err, errlen)) { free(np.midiData); return 0; }
    if (np.masterVol < 0) np.masterVol = 0;
    if (np.masterVol > 127) np.masterVol = 127;
    project_free(p);
    *p = np;
    snprintf(p->path, sizeof(p->path), "%s", path);
    p->dirty = 0;
    if (bad) snprintf(err, errlen, "%d instrument(s) in the project aren't in this version's sound set and were reset", bad);
    else err[0] = 0;
    return 1;
}
