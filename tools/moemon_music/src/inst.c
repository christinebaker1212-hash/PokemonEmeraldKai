#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "inst.h"

const SfVoice VOICE_PLACEHOLDER = {1, 60, 0, 0, 2, -1, -1, 0, 0, 15, 0};

static EWave sWaves[SF_NUM_SAMPLES];
static EInst sGroups[SF_NUM_GROUPS][128];

// ------------------------------------------------------------------ custom samples
static CustomSample *sCustom;
static EWave sCustomWave[INST_MAX_CUSTOM];
static int sNumCustom;

int inst_is_custom_ref(int ref) { return ref >= INST_CUSTOM && ref < INST_CUSTOM + sNumCustom; }
int inst_custom_count(void) { return sNumCustom; }
const CustomSample *inst_custom_get(int ref) { return inst_is_custom_ref(ref) ? &sCustom[ref - INST_CUSTOM] : NULL; }

void inst_custom_bind(CustomSample *list, int n)
{
    if (n > INST_MAX_CUSTOM) n = INST_MAX_CUSTOM;
    sCustom = list;
    sNumCustom = list ? n : 0;
    for (int i = 0; i < sNumCustom; i++) {
        EWave *w = &sCustomWave[i];
        w->freq = list[i].freq;
        w->loopStart = list[i].loopStart;
        w->size = list[i].size;
        w->loops = list[i].loops;
        w->data = list[i].data;
    }
}

int custom_by_name(const char *name)
{
    for (int i = 0; i < sNumCustom; i++) if (!strcmp(sCustom[i].name, name)) return INST_CUSTOM + i;
    return -1;
}

int custom_list_add(CustomSample **list, int *n, const CustomSample *s)
{
    if (*n >= INST_MAX_CUSTOM) return -1;
    CustomSample *nl = realloc(*list, sizeof(CustomSample) * (size_t)(*n + 1));
    if (!nl) return -1;
    *list = nl;
    CustomSample *d = &nl[*n];
    *d = *s;
    uint32_t len = s->isWave ? 16 : s->size;
    // zero bytes past the end, like the game's samples have (the mixer reads ahead)
    d->data = calloc((size_t)len + 20, 1);
    if (!d->data) return -1;
    if (s->data) memcpy(d->data, s->data, len);
    if (s->isWave) { d->size = 16; d->freq = 0; d->loops = 0; d->loopStart = 0; }
    if (d->loopStart > d->size) d->loopStart = 0;
    return INST_CUSTOM + (*n)++;
}

void custom_list_free(CustomSample **list, int *n)
{
    if (*list == sCustom) inst_custom_bind(NULL, 0);
    for (int i = 0; i < *n; i++) free((*list)[i].data);
    free(*list);
    *list = NULL;
    *n = 0;
}

const EWave *inst_wave(int s)
{
    if (s >= 0 && s < SF_NUM_SAMPLES) return &sWaves[s];
    if (inst_is_custom_ref(s) && !sCustom[s - INST_CUSTOM].isWave) return &sCustomWave[s - INST_CUSTOM];
    return NULL;
}

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
        else if (inst_is_custom_ref(v->ref)) o->wave16 = (const uint8_t *)sCustom[v->ref - INST_CUSTOM].data;
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
    if (!(v->type & 0xC0) && ((v->type & 7) == 0 || (v->type & 7) == 3) && inst_is_custom_ref(v->ref)) {
        snprintf(buf, n, "Original: %s", sCustom[v->ref - INST_CUSTOM].name);
        return;
    }
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
                 inst_is_custom_ref(v->ref) ? sCustom[v->ref - INST_CUSTOM].name :
                 v->ref >= 0 && v->ref < SF_NUM_WAVES ? sf_waves[v->ref].name + strlen("ProgrammableWaveData_") : "?", v->key,
                 v->attack, v->decay, v->sustain, v->release);
    } else if ((v->type & 7) == 4) {
        snprintf(buf, n, "%s %s  key %d  A%d D%d S%d R%d", voice_kind(v), v->ref ? "periodic" : "white", v->key,
                 v->attack, v->decay, v->sustain, v->release);
    } else if (inst_is_custom_ref(v->ref)) {
        const CustomSample *c = &sCustom[v->ref - INST_CUSTOM];
        snprintf(buf, n, "%s %s (original, %u samples%s)  A%d D%d S%d R%d", voice_kind(v), c->name, c->size,
                 c->loops ? ", loops" : "", v->attack, v->decay, v->sustain, v->release);
    } else {
        snprintf(buf, n, "%s %s  key %d  A%d D%d S%d R%d", voice_kind(v),
                 v->ref >= 0 && v->ref < SF_NUM_SAMPLES ? sf_samples[v->ref].name : "?",
                 v->key, v->attack, v->decay, v->sustain, v->release);
    }
}

void voice_to_text(const SfVoice *v, char *buf, int n)
{
    char ref[160];
    if (v->type & 0x80) snprintf(ref, sizeof(ref), "kit:%s", v->sub >= 0 ? sf_group_names[v->sub] : "");
    else if (v->type & 0x40) snprintf(ref, sizeof(ref), "split:%s:%s", v->sub >= 0 ? sf_group_names[v->sub] : "",
                                      v->ks >= 0 ? sf_keysplit_names[v->ks] : "");
    else if (((v->type & 7) == 3 || !(v->type & 7)) && inst_is_custom_ref(v->ref))
        snprintf(ref, sizeof(ref), "custom:%s", sCustom[v->ref - INST_CUSTOM].name);
    else if ((v->type & 7) == 3) snprintf(ref, sizeof(ref), "wave:%s", v->ref >= 0 && v->ref < SF_NUM_WAVES ? sf_waves[v->ref].name : "");
    else if (v->type & 7) snprintf(ref, sizeof(ref), "value:%d", v->ref);
    else snprintf(ref, sizeof(ref), "sample:%s", v->ref >= 0 && v->ref < SF_NUM_SAMPLES ? sf_samples[v->ref].name : "");
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
    if (!strncmp(ref, "custom:", 7)) { v->ref = (int16_t)custom_by_name(ref + 7); return v->ref >= 0; }
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

// General MIDI drums that can stand in for each other, closest first.
static const struct { uint8_t key; uint8_t alt[8]; } kDrumAlt[] = {
    {35, {36}}, {36, {35}},
    {37, {38, 40, 39, 75, 76}}, {38, {40, 37, 39}}, {39, {38, 40, 37}}, {40, {38, 39, 37}},
    {41, {43, 45, 47, 48, 50, 35, 36}}, {43, {41, 45, 47, 48, 50}}, {45, {47, 43, 48, 41, 50}},
    {47, {45, 48, 43, 50, 41}}, {48, {50, 47, 45, 43, 41}}, {50, {48, 47, 45, 43, 41}},
    {42, {44, 46, 54, 70, 69}}, {44, {42, 46, 54}}, {46, {42, 44, 49, 55, 54}},
    {49, {57, 55, 52, 51, 46}}, {57, {49, 55, 52, 51}}, {55, {49, 57, 52, 46}}, {52, {49, 57, 55}},
    {51, {59, 53, 49, 42}}, {59, {51, 53, 49}}, {53, {51, 59, 56, 80, 81}},
    {54, {69, 70, 42, 46}}, {56, {53, 67, 68, 80}}, {58, {69, 70, 54}},
    {60, {61, 62, 63, 64, 65, 66}}, {61, {60, 64, 63, 62, 66, 65}}, {62, {63, 60, 64, 61}},
    {63, {62, 64, 60, 61}}, {64, {63, 61, 62, 66}}, {65, {66, 60, 61}}, {66, {65, 61, 64}},
    {67, {68, 56, 76}}, {68, {67, 56, 77}}, {69, {70, 54, 42}}, {70, {69, 54, 42}},
    {71, {72, 78, 79}}, {72, {71, 79}}, {73, {74, 69, 70}}, {74, {73, 69}},
    {75, {76, 77, 37}}, {76, {77, 75, 37}}, {77, {76, 75}}, {78, {79, 62}}, {79, {78, 63}},
    {80, {81, 53}}, {81, {80, 53}},
};

static int kit_has(const SfVoice *g, int key)
{
    if (key < 0 || key > 127) return 0;
    const SfVoice *v = &g[key];
    if (v->type & 0xC0) return 0;                       // the game never plays a kit inside a kit
    if (voice_same(v, &VOICE_PLACEHOLDER)) return 0;    // an empty slot (it would beep)
    if (!(v->type & 7) && v->ref < 0) return 0;          // a sample this build doesn't have
    return 1;
}

int drum_resolve(const SfVoice *kit, int key)
{
    if (!(kit->type & 0x80) || kit->sub < 0 || kit->sub >= SF_NUM_GROUPS) return key;
    const SfVoice *g = sf_groups[kit->sub];
    if (kit_has(g, key)) return key;
    for (size_t i = 0; i < sizeof(kDrumAlt) / sizeof(kDrumAlt[0]); i++) {
        if (kDrumAlt[i].key != key) continue;
        for (int j = 0; j < 8 && kDrumAlt[i].alt[j]; j++)
            if (kit_has(g, kDrumAlt[i].alt[j])) return kDrumAlt[i].alt[j];
        break;
    }
    for (int d = 1; d < 128; d++) {   // the nearest note with a sound, lower first
        if (kit_has(g, key - d)) return key - d;
        if (kit_has(g, key + d)) return key + d;
    }
    return key;
}

static float loudness_at(const SfVoice *v, int key)
{
    static Engine e;
    static EInst inst;
    static float buf[ENG_FRAME_SAMPLES * 2];
    eng_init(&e);
    inst_make(v, &inst);
    eng_preview_on(&e, &inst, key, 127);
    float best = 0;
    for (int f = 0; f < 30; f++) {
        if (f == 20) eng_preview_off(&e);
        eng_frame(&e, buf);
        double sum = 0;
        for (int i = 0; i < ENG_FRAME_SAMPLES * 2; i++) sum += (double)buf[i] * buf[i];
        float rms = (float)sqrt(sum / (ENG_FRAME_SAMPLES * 2));
        if (rms > best) best = rms;
    }
    return best;
}

float voice_loudness(const SfVoice *v)
{
    if (v->type & 0x80) {
        static const int keys[] = {36, 38, 42};
        float sum = 0;
        int n = 0;
        for (int i = 0; i < 3; i++) {
            float l = loudness_at(v, drum_resolve(v, keys[i]));
            if (l > 0) { sum += l; n++; }
        }
        return n ? sum / n : 0;
    }
    return loudness_at(v, 60);
}

float voice_gm_gain(const SfVoice *v)
{
    static float ref = -1;
    if (ref < 0) ref = voice_loudness(&sf_presets[sf_gm_preset[0]].voice);
    if (voice_same(v, &VOICE_PLACEHOLDER)) return 1.0f;
    float l = voice_loudness(v);
    if (l <= 0 || ref <= 0) return 1.0f;
    float g = ref / l;
    return g < 0.25f ? 0.25f : g > 4.0f ? 4.0f : g;
}
