// ORRE's sound (include/orre_sound.h): Colosseum's songs and sound effects from the disc's own MusyX data, played by
// the MusyX runtime beside this file (AxioDL's decompilation and PC renderer, CC0: LICENSE, COMMIT).
//
// What Colosseum does (the decomp's gs_range_801653CC / gs_dvd.c): a sound id is a row of its sound table
// (common_rel .data 0x13FE08, 12 bytes: flags (0x80 an effect), the song's group, reverb, the song file's row, then
// the MusyX id: a song's number in its group or an effect's id); the song files are the song table's (.data
// 0x111B8C: "sound/NAME.song", in the pack as bgm_archive/NAME_song.snd); the sample archives (.data 0x143918)
// are pushed once: music, its atmosphere, the effects, the motions and the cries. One song at a time: a new one
// fades the old out.
//
// The GBA filter (the user, 2026-11-10: "compress songs down to a GBA sound filter"): MusyX mixes at the GBA's own
// 13379 Hz and the mix is cut to 8 bits, then held up to the game's rate as the GBA's output does, with the light
// low-pass a small speaker gives. The data stays as the disc has it (DSP ADPCM, 4 bits a sample), in the pack.
#include <musyx/musyx.h>
#include <musyx/pc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/orre_sound.h"

uint8_t *OrreData_Read(const char *name, uint32_t *size);   // src/orre/orre_data.c
int OrreData_ResourceName(uint32_t id, char *out, int outSize);

#define GBA_RATE 13379
#define SOUND_TABLE 0x13FE08
#define SOUND_COUNT 1236
#define SONG_TABLE 0x111B8C
#define SONG_COUNT 79
#define GAIN 0.85f   // (Colosseum's mix as loud as Kai's town songs: RMS 0.09 on the game's output, measured 2026-11-10)

static struct
{
    int tried, ok;
    uint8_t *rel, *data;
    uint32_t dataSize, relSize;
    uint8_t *song[SONG_COUNT];
    // two songs at once as Colosseum has them: 1 the music (std104), 0 a song started as an effect (std105: the
    // wind's loop, a fanfare)
    SND_SEQID seq[2];
    uint32_t bgm[2];       // (each one's sound id)
    uint8_t bgmVolume[2];
    SND_VOICEID fx[SOUND_COUNT];
    // the filter's state
    int16_t held[2];
    double phase;
    float lp[2];
    // loudness (tools/kaisong/normalize.py: music/orre_gains.txt): each song's gain in dB, the effects' when no song
    // plays; the whole mix takes the gain of the song playing, so Colosseum's own balance of its music and its
    // sounds stays as it is
    float songDb[SOUND_COUNT], fxDb, gain;
    int measure, gainSet;
} s;

static uint32_t Be32(const uint8_t *p) { return (uint32_t)p[0] << 24 | p[1] << 16 | p[2] << 8 | p[3]; }
static uint16_t Be16(const uint8_t *p) { return (uint16_t)(p[0] << 8 | p[1]); }

// A group's four files; the directory's "no loop" written as loop start -1 (one of the music's samples) made plain
static int Push(const char *name, uint16_t group)
{
    char path[96];
    uint32_t n[4], o;
    uint8_t *f[4];
    static const char *const parts[3] = {"proj", "pool", "sdir"};
    int i;
    for (i = 0; i < 3; i++)
    {
        snprintf(path, sizeof(path), "common/%s_%s.bin", name, parts[i]);
        if ((f[i] = OrreData_Read(path, &n[i])) == NULL)
            return 0;
    }
    snprintf(path, sizeof(path), "sound/%s.samp", name);
    if ((f[3] = OrreData_Read(path, &n[3])) == NULL)
        return 0;
    for (o = 0; o + 32 <= n[2] && Be16(f[2] + o) != 0xFFFF; o += 32)
        if (Be32(f[2] + o + 20) == 0xFFFFFFFF)
            memset(f[2] + o + 20, 0, 8);
    if (!sndPushGroup(f[0], group, f[3], f[2], f[1]))
    {
        printf("[orre] sound: %s (group %u) wasn't taken\n", name, group);
        return 0;
    }
    return 1;   // (the files stay: MusyX plays from them)
}

static int Init(void)
{
    static const struct { const char *name; uint16_t group; } groups[] = {
        {"snd_music", 0}, {"snd_music_atmos", 1}, {"snd_se", 6}, {"snd_se_motion", 8}, {"snd_se_nakigoe", 4},
        {"snd_se_nakigoe2", 9},
    };
    SND_HOOKS hooks = {malloc, free};
    SND_PC_CONFIG cfg = {GBA_RATE, 2};
    uint32_t st, off, i;
    if (s.tried)
        return s.ok;
    if ((s.rel = OrreData_Read("common/common_rel.rel", &s.relSize)) == NULL)
        return 0;   // (no pack yet: looked for again)
    s.tried = 1;
    st = Be32(s.rel + 0x10);
    if (s.relSize < 0x40 || Be32(s.rel + 0x0C) < 5 || st + 8 * 5 > s.relSize)
        return 0;
    off = Be32(s.rel + st + 8 * 4) & ~3u;
    s.dataSize = Be32(s.rel + st + 8 * 4 + 4);
    if (off + s.dataSize > s.relSize || s.dataSize < SOUND_TABLE + SOUND_COUNT * 12)
        return 0;
    s.data = s.rel + off;
    sndSetHooks(&hooks);
    sndPCConfigure(&cfg);
    if (sndInit(64, 48, 16, 1, 0, 16 * 1024 * 1024) != 0)
    {
        printf("[orre] sound: MusyX didn't start\n");
        return 0;
    }
    if (!Push(groups[0].name, groups[0].group))
        return 0;
    for (i = 1; i < sizeof(groups) / sizeof(groups[0]); i++)
        Push(groups[i].name, groups[i].group);
    sndVolume(127, 0, SND_ALL_VOLGROUPS);
    s.seq[0] = s.seq[1] = SND_SEQ_ERROR_ID;
    for (i = 0; i < SOUND_COUNT; i++)
        s.fx[i] = SND_ID_ERROR;
    printf("[orre] sound: MusyX up, %u sounds\n", SOUND_COUNT);
    return s.ok = 1;
}

// the song file of the song table's row: its resource id (the row's first word), by the pack's own index
static uint8_t *Song(uint32_t row)
{
    uint32_t n, id;
    char path[96];
    if (row >= SONG_COUNT)
        return NULL;
    if (s.song[row] != NULL)
        return s.song[row];
    id = Be32(s.data + SONG_TABLE + row * 8);
    if (id == 0 || !OrreData_ResourceName(id, path, sizeof(path)))
        return NULL;   // (row 10 has no file)
    s.song[row] = OrreData_Read(path, &n);
    if (s.song[row] == NULL)
        printf("[orre] sound: no %s\n", path);
    return s.song[row];
}

int OrreSound_Play(uint32_t id, uint32_t volume, int bgm)
{
    const uint8_t *e;
    uint8_t vol = volume > 127 ? 127 : (uint8_t)volume;
    if (!Init() || id == 0 || id >= SOUND_COUNT)
        return 0;
    e = s.data + SOUND_TABLE + id * 12;
    if (e[0] & 0x80)
    {
        // an effect: again from its start if it's still sounding
        if (s.fx[id] != SND_ID_ERROR && sndFXCheck(s.fx[id]) != SND_ID_ERROR)
            sndFXKeyOff(s.fx[id]);
        s.fx[id] = sndFXStart(Be16(e + 4), vol, 64);
        if (getenv("MOEMON_ORRE_SOUND_LOG"))
            printf("[orre] sound: effect %u (fx %u) vol %u -> %s\n", id, Be16(e + 4), vol,
                   s.fx[id] != SND_ID_ERROR ? "on" : "none");
        return s.fx[id] != SND_ID_ERROR;
    }
    else
    {
        uint8_t *song = Song(e[3]);
        int k = bgm ? 1 : 0;
        SND_PLAYPARA para;
        if (s.bgm[k] == id && s.seq[k] != SND_SEQ_ERROR_ID)
        {
            if (vol != s.bgmVolume[k])
                sndSeqVolume(s.bgmVolume[k] = vol, 500, s.seq[k], SND_SEQVOL_CONTINUE);
            return 1;   // (the same song goes on)
        }
        if (s.seq[k] != SND_SEQ_ERROR_ID)
            sndSeqVolume(0, 500, s.seq[k], SND_SEQVOL_STOP);
        s.seq[k] = SND_SEQ_ERROR_ID, s.bgm[k] = 0;
        if (song == NULL)
            return 0;   // (the silent row: the music stops)
        memset(&para, 0, sizeof(para));
        para.flags = SND_PLAYPARA_VOLUME;
        para.volume.time = 0, para.volume.target = vol;
        s.seq[k] = sndSeqPlay(e[1], Be16(e + 4), song, &para);
        if (getenv("MOEMON_ORRE_SOUND_LOG"))
            printf("[orre] sound: song %u (file %u, group %u song %u) vol %u slot %d -> %s\n", id, e[3], e[1],
                   Be16(e + 4), vol, k, s.seq[k] != SND_SEQ_ERROR_ID ? "on" : "none");
        if (s.seq[k] == SND_SEQ_ERROR_ID)
            return 0;
        s.bgm[k] = id, s.bgmVolume[k] = vol;
        return 1;
    }
}

static void StopSlot(int k, uint32_t fadeMs)
{
    if (!s.ok || s.seq[k] == SND_SEQ_ERROR_ID)
        return;
    sndSeqVolume(0, (uint16_t)(fadeMs > 60000 ? 60000 : fadeMs), s.seq[k], SND_SEQVOL_STOP);
    s.seq[k] = SND_SEQ_ERROR_ID, s.bgm[k] = 0;
}

void OrreSound_StopBgm(uint32_t fadeMs)
{
    StopSlot(1, fadeMs);
}

void OrreSound_StopAmbient(uint32_t fadeMs)
{
    StopSlot(0, fadeMs);
}

void OrreSound_Stop(uint32_t id, uint32_t fadeMs)
{
    if (!s.ok || id == 0 || id >= SOUND_COUNT)
        return;
    if (id == s.bgm[1])
        StopSlot(1, fadeMs);
    else if (id == s.bgm[0])
        StopSlot(0, fadeMs);
    else if (s.fx[id] != SND_ID_ERROR)
    {
        if (sndFXCheck(s.fx[id]) != SND_ID_ERROR)
            sndFXKeyOff(s.fx[id]);
        s.fx[id] = SND_ID_ERROR;
    }
}

// Every effect still sounding ends (a room's loops: machines, water, a crowd): the room is left, or a battle begins
void OrreSound_StopEffects(void)
{
    uint32_t i;
    if (!s.ok)
        return;
    for (i = 0; i < SOUND_COUNT; i++)
        if (s.fx[i] != SND_ID_ERROR)
        {
            if (sndFXCheck(s.fx[i]) != SND_ID_ERROR)
                sndFXKeyOff(s.fx[i]);
            s.fx[i] = SND_ID_ERROR;
        }
}

void OrreSound_Volume(uint32_t id, uint32_t volume, uint32_t fadeMs)
{
    uint8_t vol = volume > 127 ? 127 : (uint8_t)volume;
    int k;
    if (!s.ok || id == 0 || id >= SOUND_COUNT)
        return;
    for (k = 0; k < 2; k++)
        if (id == s.bgm[k] && s.seq[k] != SND_SEQ_ERROR_ID)
        {
            sndSeqVolume(s.bgmVolume[k] = vol, (uint16_t)(fadeMs > 60000 ? 60000 : fadeMs), s.seq[k], SND_SEQVOL_CONTINUE);
            return;
        }
    if (s.fx[id] != SND_ID_ERROR && sndFXCheck(s.fx[id]) != SND_ID_ERROR)
        sndFXVolume(s.fx[id], vol);
}

void OrreSound_MasterVolume(uint32_t volume, uint32_t fadeMs)
{
    if (s.ok)
        sndMasterVolume(volume > 127 ? 127 : (uint8_t)volume, (uint16_t)(fadeMs > 60000 ? 60000 : fadeMs), 1, 1);
}

uint32_t OrreSound_Bgm(void)
{
    return s.ok && s.seq[1] != SND_SEQ_ERROR_ID ? s.bgm[1] : 0;
}

void OrreSound_Silence(void)
{
    uint32_t i;
    if (!s.ok)
        return;
    sndSilence();
    s.seq[0] = s.seq[1] = SND_SEQ_ERROR_ID, s.bgm[0] = s.bgm[1] = 0;
    for (i = 0; i < SOUND_COUNT; i++)
        s.fx[i] = SND_ID_ERROR;
    sndMasterVolume(127, 0, 1, 1);
}

void OrreSound_SetGain(uint32_t id, float db)
{
    if (id == 0)
        s.fxDb = db;
    else if (id < SOUND_COUNT)
        s.songDb[id] = db;
}

void OrreSound_Measure(int on)
{
    s.measure = on;
    s.gainSet = 0;
}

int OrreSound_IsSong(uint32_t id)
{
    const uint8_t *e;
    if (!Init() || id == 0 || id >= SOUND_COUNT)
        return 0;
    e = s.data + SOUND_TABLE + id * 12;
    return !(e[0] & 0x80) && Song(e[3]) != NULL;
}

void OrreSound_Mix(float *buffer, int32_t count, int32_t rate, int on)
{
    const double step = (double)GBA_RATE / rate;
    int32_t n;
    float want;
    if (!s.ok || !on)
        return;   // (off the field: nothing rendered, so the song waits where it is)
    {
        float db = s.measure ? 0.0f : s.bgm[1] ? s.songDb[s.bgm[1]] : s.fxDb;
        float lin = 1.0f, x = db / 20.0f * 2.302585f, term = 1.0f;   // (10^(db/20) without libm: e^x by its series)
        int k;
        for (k = 1; k < 12; k++)
            term *= x / k, lin += term;
        want = lin;
        if (!s.gainSet)
            s.gain = want, s.gainSet = 1;
    }
    for (n = 0; n + 1 < count; n += 2)
    {
        int c;
        s.gain += (want - s.gain) * 0.0002f;   // (a song's gain to the next's over about a tenth of a second)
        s.phase += step;
        if (s.phase >= 1.0)
        {
            int16_t frame[2];
            s.phase -= 1.0;
            if (sndPCRender(frame, 1))
            {
                // (the GBA's 8 bits)
                s.held[0] = (int16_t)(frame[0] & ~0xFF);
                s.held[1] = (int16_t)(frame[1] & ~0xFF);
            }
        }
        for (c = 0; c < 2; c++)
        {
            float v = s.held[c] * (GAIN / 32768.0f) * s.gain;
            s.lp[c] += (v - s.lp[c]) * 0.55f;
            v = buffer[n + c] + s.lp[c];
            buffer[n + c] = v > 1.0f ? 1.0f : v < -1.0f ? -1.0f : v;
        }
    }
}
