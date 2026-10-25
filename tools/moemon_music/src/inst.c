#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "inst.h"

const SfVoice VOICE_PLACEHOLDER = {1, 60, 0, 0, 2, -1, -1, 0, 0, 15, 0};

static EWave sWaves[SF_NUM_SAMPLES];
static EInst sGroups[SF_NUM_GROUPS][128];

const EWave *inst_wave(int s) { return s >= 0 && s < SF_NUM_SAMPLES ? &sWaves[s] : NULL; }

static void make(const SfVoice *v, EInst *o, int depth)
{
    memset(o, 0, sizeof(*o));
    o->type = v->type;
    o->key = v->key;
    o->length = v->length;
    o->panSweep = v->panSweep;
    o->attack = v->attack;
    o->decay = v->decay;
    o->sustain = v->sustain;
    o->release = v->release;
    if (v->type & 0xC0) {
        if (v->sub >= 0 && v->sub < SF_NUM_GROUPS && depth == 0) o->group = sGroups[v->sub];
        if ((v->type & 0x40) && v->ks >= 0 && v->ks < SF_NUM_KEYSPLITS) o->split = sf_keysplits[v->ks];
        // a key-split / kit entry has no envelope of its own (those bytes are the table pointer in the game)
        o->attack = o->decay = o->sustain = o->release = 0;
    } else if ((v->type & 7) == 3) {
        if (v->ref >= 0 && v->ref < SF_NUM_WAVES) o->wave16 = sf_pcm + sf_waves[v->ref].offset;
    } else if (v->type & 7) {
        o->cgbParam = (uint32_t)(v->ref < 0 ? 0 : v->ref);
    } else {
        o->wav = inst_wave(v->ref);
    }
}

void inst_init(void)
{
    for (int i = 0; i < SF_NUM_SAMPLES; i++) {
        sWaves[i].freq = sf_samples[i].freq;
        sWaves[i].loopStart = sf_samples[i].loopStart;
        sWaves[i].size = sf_samples[i].size;
        sWaves[i].loops = (sf_samples[i].flags & 0xC0000000u) != 0;
        sWaves[i].data = (const int8_t *)(sf_pcm + sf_samples[i].offset);
    }
    for (int g = 0; g < SF_NUM_GROUPS; g++)
        for (int i = 0; i < 128; i++)
            make(&sf_groups[g][i], &sGroups[g][i], 1);
    // kits can point at key-split groups (one level, like the game's own data)
    for (int g = 0; g < SF_NUM_GROUPS; g++)
        for (int i = 0; i < 128; i++)
            if (sf_groups[g][i].type & 0xC0)
                make(&sf_groups[g][i], &sGroups[g][i], 0);
}

void inst_make(const SfVoice *v, EInst *out) { make(v, out, 0); }

int voice_same(const SfVoice *a, const SfVoice *b)
{
    return a->type == b->type && a->key == b->key && a->length == b->length && a->panSweep == b->panSweep &&
           a->ref == b->ref && a->sub == b->sub && a->ks == b->ks && a->attack == b->attack &&
           a->decay == b->decay && a->sustain == b->sustain && a->release == b->release;
}

int inst_find_preset(const SfVoice *v)
{
    for (int i = 0; i < SF_NUM_PRESETS; i++)
        if (voice_same(&sf_presets[i].voice, v)) return i;
    return -1;
}

int voice_is_cgb(const SfVoice *v) { return !(v->type & 0xC0) && (v->type & 7); }
int voice_is_group(const SfVoice *v) { return (v->type & 0xC0) != 0; }

const char *voice_kind(const SfVoice *v)
{
    if (v->type & 0x80) return "Drum kit";
    if (v->type & 0x40) return "Key-split";
    switch (v->type & 7) {
    case 1: return v->type & 8 ? "Square 1 (alt)" : "Square 1";
    case 2: return v->type & 8 ? "Square 2 (alt)" : "Square 2";
    case 3: return v->type & 8 ? "Wave (alt)" : "Wave";
    case 4: return v->type & 8 ? "Noise (alt)" : "Noise";
    }
    if (v->type & 0x10) return "Sample (reversed)";
    if (v->type & 8) return "Sample (fixed pitch)";
    return "Sample";
}

void voice_name(const SfVoice *v, char *buf, int n)
{
    int p = inst_find_preset(v);
    if (p >= 0) { snprintf(buf, n, "%s", sf_presets[p].name); return; }
    if (voice_same(v, &VOICE_PLACEHOLDER)) { snprintf(buf, n, "(empty)"); return; }
    // an edited voice: name it after what it plays
    SfVoice base = *v;
    for (int i = 0; i < SF_NUM_PRESETS; i++) {
        const SfVoice *q = &sf_presets[i].voice;
        if (q->type == base.type && q->ref == base.ref && q->sub == base.sub && q->ks == base.ks) {
            const char *nm = sf_presets[i].name;
            const char *hash = strstr(nm, " #");
            int len = hash ? (int)(hash - nm) : (int)strlen(nm);
            snprintf(buf, n, "%.*s (edited)", len, nm);
            return;
        }
    }
    snprintf(buf, n, "%s (edited)", voice_kind(v));
}

void voice_detail(const SfVoice *v, char *buf, int n)
{
    if (v->type & 0x80) {
        snprintf(buf, n, "Drum kit %s", v->sub >= 0 ? sf_group_names[v->sub] : "?");
    } else if (v->type & 0x40) {
        snprintf(buf, n, "Key-split %s / table %s", v->sub >= 0 ? sf_group_names[v->sub] : "?",
                 v->ks >= 0 ? sf_keysplit_names[v->ks] : "?");
    } else if ((v->type & 7) == 1 || (v->type & 7) == 2) {
        static const char *duty[] = {"12.5%", "25%", "50%", "75%"};
        snprintf(buf, n, "%s duty %s  key %d  A%d D%d S%d R%d", voice_kind(v), duty[v->ref & 3], v->key,
                 v->attack, v->decay, v->sustain, v->release);
    } else if ((v->type & 7) == 3) {
        snprintf(buf, n, "%s %s  key %d  A%d D%d S%d R%d", voice_kind(v),
                 v->ref >= 0 ? sf_waves[v->ref].name + strlen("ProgrammableWaveData_") : "?", v->key,
                 v->attack, v->decay, v->sustain, v->release);
    } else if ((v->type & 7) == 4) {
        snprintf(buf, n, "%s %s  key %d  A%d D%d S%d R%d", voice_kind(v), v->ref ? "periodic" : "white", v->key,
                 v->attack, v->decay, v->sustain, v->release);
    } else {
        snprintf(buf, n, "%s %s  key %d  A%d D%d S%d R%d", voice_kind(v), v->ref >= 0 ? sf_samples[v->ref].name : "?",
                 v->key, v->attack, v->decay, v->sustain, v->release);
    }
}

void voice_to_text(const SfVoice *v, char *buf, int n)
{
    char ref[160];
    if (v->type & 0x80) snprintf(ref, sizeof(ref), "kit:%s", v->sub >= 0 ? sf_group_names[v->sub] : "");
    else if (v->type & 0x40) snprintf(ref, sizeof(ref), "split:%s:%s", v->sub >= 0 ? sf_group_names[v->sub] : "",
                                      v->ks >= 0 ? sf_keysplit_names[v->ks] : "");
    else if ((v->type & 7) == 3) snprintf(ref, sizeof(ref), "wave:%s", v->ref >= 0 ? sf_waves[v->ref].name : "");
    else if (v->type & 7) snprintf(ref, sizeof(ref), "value:%d", v->ref);
    else snprintf(ref, sizeof(ref), "sample:%s", v->ref >= 0 ? sf_samples[v->ref].name : "");
    snprintf(buf, n, "type=0x%02X key=%d length=%d pansweep=0x%02X ref=%s adsr=%d,%d,%d,%d",
             v->type, v->key, v->length, v->panSweep, ref, v->attack, v->decay, v->sustain, v->release);
}

int voice_from_text(const char *s, SfVoice *v)
{
    unsigned type, ps;
    int key, len, a, d, su, r;
    char ref[200];
    if (sscanf(s, " type=%x key=%d length=%d pansweep=%x ref=%199s adsr=%d,%d,%d,%d", &type, &key, &len, &ps, ref,
               &a, &d, &su, &r) != 9)
        return 0;
    memset(v, 0, sizeof(*v));
    v->type = (uint8_t)type; v->key = (uint8_t)key; v->length = (uint8_t)len; v->panSweep = (uint8_t)ps;
    v->attack = (uint8_t)a; v->decay = (uint8_t)d; v->sustain = (uint8_t)su; v->release = (uint8_t)r;
    v->ref = v->sub = v->ks = -1;
    if (!strncmp(ref, "kit:", 4)) { v->sub = (int16_t)group_by_name(ref + 4); return v->sub >= 0; }
    if (!strncmp(ref, "split:", 6)) {
        char *c = strchr(ref + 6, ':');
        if (!c) return 0;
        *c = 0;
        v->sub = (int16_t)group_by_name(ref + 6);
        v->ks = (int16_t)keysplit_by_name(c + 1);
        return v->sub >= 0 && v->ks >= 0;
    }
    if (!strncmp(ref, "wave:", 5)) { v->ref = (int16_t)wave_by_name(ref + 5); return v->ref >= 0; }
    if (!strncmp(ref, "value:", 6)) { v->ref = (int16_t)atoi(ref + 6); return 1; }
    if (!strncmp(ref, "sample:", 7)) { v->ref = (int16_t)sample_by_name(ref + 7); return v->ref >= 0; }
    return 0;
}

int sample_by_name(const char *name)
{
    for (int i = 0; i < SF_NUM_SAMPLES; i++) if (!strcmp(sf_samples[i].name, name)) return i;
    return -1;
}
int wave_by_name(const char *name)
{
    for (int i = 0; i < SF_NUM_WAVES; i++) if (!strcmp(sf_waves[i].name, name)) return i;
    return -1;
}
int group_by_name(const char *name)
{
    for (int i = 0; i < SF_NUM_GROUPS; i++) if (!strcmp(sf_group_names[i], name)) return i;
    return -1;
}
int keysplit_by_name(const char *name)
{
    for (int i = 0; i < SF_NUM_KEYSPLITS; i++) if (!strcmp(sf_keysplit_names[i], name)) return i;
    return -1;
}
int song_by_name(const char *name)
{
    for (int i = 0; i < SF_NUM_SONGS; i++) if (!strcmp(sf_songs[i].name, name)) return i;
    return -1;
}
int song_group_by_name(const char *name)
{
    for (int i = 0; i < SF_NUM_SONG_GROUPS; i++) if (!strcmp(sf_song_group_names[i], name)) return i;
    return -1;
}
