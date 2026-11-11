// Moemon Kai: custom songs from Moemon Music Studio (.kaisong, docs/KAISONG_FORMAT.md).
//
// music/<song>.kaisong plays instead of that song everywhere, music/<song>@<place>.kaisong only on one map (place =
// the map's MAP_ id without MAP_, lowercase). The folder ships with the game, so every player gets the same music in
// updates; a player's own files in it work too. Files are found once at the first song, loaded the first time they
// play and kept (the same header pointer every time: m4aSongNumStartOrChange and spectating compare headers).
//
// Scenes: music/<song>@<scene>.kaisong plays only in that scene (a card game, the BATTLE SIMULATOR, the title screen...).
// Every call that starts a song says which source file asked (include/mp_kaisong_scene.h); tools/kaisong/gen.py lists
// the scenes, the songs each one names and their files (src/data/mp_kaisong_names.h). A song a scene names, asked for by
// that scene, plays the scene's file, else the everywhere one: never a map's (a map's file is for the map's own music,
// events and battles), so one scene's music never changes another's. A song the scene doesn't name (the map's own song
// when it ends) is the map's as usual. A scene that borrows the game's battle code holds the music while it runs
// (MP_MUSIC_SCENE_HOLD: every song is its own then).
//
// Each file keeps its own settings: priority and reverb (header), its track count (up to 64: the BGM player has 64
// tracks for songs made without the GBA's limit) and the sample voices it was made for (INFO voices=, up to
// MAX_DIRECTSOUND_CHANNELS = 64; the game's songs play with 5). A file this game can't read (an unknown sample name,
// a newer format) is skipped with a line in the log, and the game's own song plays.
//
// Moemon Maker: MoemonMaker/music/ beside the game is read too (the user's music, docs/MOEMON_MAKER.md): its
// <song>[@<place>].kaisong files win over music/'s, and a file whose name is no song of the game's (mus_my_theme) is a
// new track with a song number of its own (MAKER_SONG_FIRST..., by its name in order) that maps can play. A new track's
// song number has no entry in gSongTable: MP_Kaisong_TableIndex gives the BGM entry it plays on.
#define MP_KAISONG_NO_SCENE
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "global.h"
#include "gba/m4a_internal.h"
#include "music_player.h"
#include "m4a.h"
#include "mp_kaisong.h"
#include "constants/songs.h"

#include "../data/mp_kaisong_samples.h"
#include "../data/mp_kaisong_names.h"

static const char *SongName(u16 song)   // (a new track's number is past the table: Moemon Maker's, STADIUM's)
{
    return song < ARRAY_COUNT(sKaisongSongNames) ? sKaisongSongNames[song] : "a new track";
}

void MPNet_Log(const char *fmt, ...);
const char *Updater_ExeFile(const char *name);

#define KAI_MAX_FILES   512
#define KAI_MAX_TRACKS  64
#define KAI_GAME_VOICES 5

_Static_assert(sizeof(struct MP2KInstrument) == sizeof(struct ToneData), "voice layout");

struct KaiFile
{
    char file[96];          // file name in music/ (or MoemonMaker/music/: maker)
    u8 maker;               // Moemon Maker: the user's (wins over music/'s)
    u16 song;
    s16 group, num;         // the map (-1 = not one map)
    s16 scene;              // the scene (sKaisongScenes; -1 = not a scene); neither = everywhere
    u8 variant;             // 0 everywhere, 1-63 a hash of the place (spectating)
    u8 failed;
    u8 voices;
    u16 gain;               // its gain as a factor in 256ths: music/kaisong_gains.txt's (tools/kaisong/normalize.py),
                            // else INFO gain= (dB)
    s16 tableGain;          // music/kaisong_gains.txt's, tenths of a dB (KAI_NO_GAIN: not in it)
    u32 music;              // a hash of everything but INFO: files with the same music (MP_Kaisong_SameMusic)
    struct SongHeader *header;
};

static struct
{
    bool8 scanned;
    u16 count;
    struct KaiFile files[KAI_MAX_FILES];
    u8 chans;               // the voice count set for the BGM player's song (0 = the game's)
    s16 asked;              // the scene of the call that asked for a song (-1 = the world: the map's places)
    u8 depth;               // wrapped calls inside a wrapped call keep the outer caller
    s16 held;               // a scene holding the music (-1 none)
} sKai = {.asked = -1, .held = -1};

static int SceneByKey(const char *key)
{
    int i;
    for (i = 0; i < (int)ARRAY_COUNT(sKaisongScenes); i++)
        if (strcmp(sKaisongScenes[i].key, key) == 0)
            return i;
    return -1;
}

// a source file's scene by its path (__FILE__: "src/tcg/tcg_board.c", or with a directory in front)
static int SceneOfFile(const char *file)
{
    const char *p, *rel = NULL;
    int i;
    if (!file)
        return -1;
    for (p = file; *p; p++)
        if ((p == file || p[-1] == '/' || p[-1] == '\\') && strncmp(p, "src", 3) == 0 && (p[3] == '/' || p[3] == '\\'))
            rel = p + 4;
    if (!rel)
        rel = file;
    for (i = 0; i < (int)ARRAY_COUNT(sKaisongSceneFiles); i++)
    {
        const char *a = sKaisongSceneFiles[i].path, *b = rel;
        while (*a && (*a == *b || (*a == '/' && *b == '\\')))
            a++, b++;
        if (!*a && !*b)
            return sKaisongSceneFiles[i].scene;
    }
    return -1;
}

void MP_Kaisong_Request(const char *file)
{
    if (sKai.depth++ == 0)
        sKai.asked = SceneOfFile(file);
}

void MP_Kaisong_RequestEnd(void)
{
    if (sKai.depth)
        sKai.depth--;
}

void MP_Kaisong_Hold(const char *file)
{
    sKai.held = SceneOfFile(file);
}

void MP_Kaisong_Release(void)
{
    sKai.held = -1;
}

static bool8 SceneNames(int scene, u16 song)
{
    int i;
    for (i = 0; i < sKaisongScenes[scene].count; i++)
        if (sKaisongScenes[scene].songs[i] == song)
            return TRUE;
    return FALSE;
}

// The scene song n plays in now (-1 = the world)
static int SceneFor(u16 song)
{
    if (sKai.held >= 0)
        return sKai.held;
    if (sKai.asked >= 0 && SceneNames(sKai.asked, song))
        return sKai.asked;
    return -1;
}

static const char *MusicDir(void)
{
    const char *env = getenv("MOEMON_MUSIC_DIR");
    return env && env[0] ? env : Updater_ExeFile("music");
}

const char *MP_Kaisong_MusicDir(void)   // (ORRE's gains live there too: src/orre/orre_audio.c)
{
    return MusicDir();
}

#define KAI_NO_GAIN 0x7FFF

// dB, -12 to +12, to a factor in 256ths without libm: 10^(dB/20) by steps of a tenth of a dB
static u16 GainFactor(int tenths)
{
    double g = 1.0;
    int i;
    tenths = tenths < -120 ? -120 : tenths > 120 ? 120 : tenths;
    for (i = 0; i < (tenths < 0 ? -tenths : tenths); i++)
        g *= tenths < 0 ? 0.98855309 : 1.01157945;   // 10^(-0.1/20), 10^(0.1/20)
    return (u16)(g * 256.0 + 0.5);
}

// music/kaisong_gains.txt (tools/kaisong/normalize.py, at build time): "<file>\t<dB>" a line, every music/ file's gain so
// all play equally loud (measured in this game's own sound engine: MOEMON_MEASURE_MUSIC, mp_export_sound.c). It wins
// over a file's INFO gain=; files not in it (MoemonMaker/music/, one added since) keep their INFO's
static void ReadGainTable(void)
{
    char path[600], line[200];
    FILE *fp;
    int i, n = 0;
    for (i = 0; i < sKai.count; i++)
        sKai.files[i].tableGain = KAI_NO_GAIN;
    snprintf(path, sizeof(path), "%s/kaisong_gains.txt", MusicDir());
    fp = fopen(path, "r");
    if (fp == NULL)
        return;
    while (fgets(line, sizeof(line), fp))
    {
        char *tab = strchr(line, '\t');
        double db;
        if (line[0] == '#' || tab == NULL)
            continue;
        *tab = 0;
        db = atof(tab + 1);
        for (i = 0; i < sKai.count; i++)
            if (!sKai.files[i].maker && strcmp(sKai.files[i].file, line) == 0)
                sKai.files[i].tableGain = (s16)(db * 10.0 + (db < 0 ? -0.5 : 0.5)), n++;
    }
    fclose(fp);
    MPNet_Log("kaisong: %d gains from kaisong_gains.txt", n);
}

// Tools (MOEMON_MEASURE_MUSIC): the gain a file is measured at, whatever its table's or INFO's (KAI_NO_GAIN: theirs)
static const char *sMeasureFile;
static int sMeasureTenths = KAI_NO_GAIN;

static u8 PlaceVariant(const char *place)
{
    u32 h = 2166136261u;
    while (*place)
        h = (h ^ (u8)*place++) * 16777619u;
    return 1 + h % 63;
}

static int SongByName(const char *name)
{
    int i;
    for (i = 1; i < (int)ARRAY_COUNT(sKaisongSongNames); i++)
        if (sKaisongSongNames[i][0] && strcmp(sKaisongSongNames[i], name) == 0)
            return i;
    return -1;
}

static bool8 MapByKey(const char *key, s16 *group, s16 *num)
{
    int g, n;
    for (g = 0; g < (int)ARRAY_COUNT(sKaisongMapGroups); g++)
        for (n = 0; n < sKaisongMapGroups[g].count; n++)
            if (strcmp(sKaisongMapGroups[g].keys[n], key) == 0)
            {
                *group = g;
                *num = n;
                return TRUE;
            }
    return FALSE;
}

static int CmpFiles(const void *a, const void *b)
{
    const struct KaiFile *x = a, *y = b;
    if (x->maker != y->maker)
        return y->maker - x->maker;   // (the user's first: Find takes the first that fits)
    return strcmp(x->file, y->file);
}

static const char *MakerMusicDir(void)
{
    const char *Maker_ReadDir(void);
    static char dir[1200];
    snprintf(dir, sizeof(dir), "%s/music", Maker_ReadDir());   // (the working copy's, else the published pack's)
    return dir;
}

static const char *DirOf(const struct KaiFile *f)
{
    return f->maker ? MakerMusicDir() : MusicDir();
}

static u16 sTrackCount;
static char sTracks[MAKER_SONG_COUNT][64];   // the user's new tracks' names, by song number - MAKER_SONG_FIRST

static void ScanDir(const char *dir, u8 maker);

static void Scan(void)
{
    sKai.scanned = TRUE;
    sKai.count = 0;
    sTrackCount = 0;
    ScanDir(MusicDir(), FALSE);
    ScanDir(MakerMusicDir(), TRUE);
    qsort(sKai.files, sKai.count, sizeof(sKai.files[0]), CmpFiles);
    ReadGainTable();
    if (sKai.count)
        MPNet_Log("kaisong: %d custom songs in %s", sKai.count, MusicDir());
}

static int CmpNames(const void *a, const void *b)
{
    return strcmp(*(const char *const *)a, *(const char *const *)b);
}

static void ScanDir(const char *dir, u8 maker)
{
    DIR *d;
    struct dirent *e;
    char *names[KAI_MAX_FILES];
    int count = 0, k;
    if (!(d = opendir(dir)))
        return;
    // (by name, so a new track's song number is the same every time: they're given in this order)
    while ((e = readdir(d)) != NULL && count < KAI_MAX_FILES)
        names[count++] = strdup(e->d_name);
    closedir(d);
    qsort(names, count, sizeof(names[0]), CmpNames);
    for (k = 0; k < count && sKai.count < KAI_MAX_FILES; free(names[k]), k++)
    {
        const char *name = names[k];
        char stem[96], *at;
        size_t len = strlen(name);
        struct KaiFile *f = &sKai.files[sKai.count];
        int song;
        if (len <= 8 || len >= sizeof(stem) || strcmp(name + len - 8, ".kaisong") != 0)
            continue;
        memcpy(stem, name, len - 8);
        stem[len - 8] = 0;
        memset(f, 0, sizeof(*f));
        snprintf(f->file, sizeof(f->file), "%s", name);
        f->maker = maker;
        f->group = f->num = f->scene = -1;
        if ((at = strchr(stem, '@')) != NULL)
        {
            *at++ = 0;
            if ((f->scene = SceneByKey(at)) < 0 && !MapByKey(at, &f->group, &f->num))
            {
                MPNet_Log("kaisong: %s: no map or scene called %s (docs/SONG_TABLE.md has the place names)", f->file, at);
                continue;
            }
            f->variant = PlaceVariant(at);
        }
        if ((song = SongByName(stem)) < 0 && (maker || strncmp(stem, "stadium_", 8) == 0) && !at
         && sTrackCount < MAKER_SONG_COUNT)
        {
            // Moemon Maker: a new track (and POKéMON STADIUM's own songs, shipped in music/: played by name,
            // src/mp/mp_stadium_cup.c MP_Stadium_Track)
            snprintf(sTracks[sTrackCount], sizeof(sTracks[0]), "%s", stem);
            song = MAKER_SONG_FIRST + sTrackCount++;
        }
        else if (song < 0 || gSongTable[song].ms != 0)
        {
            MPNet_Log("kaisong: %s: no music called %s", f->file, stem);
            continue;
        }
        f->song = song;
        sKai.count++;
    }
}

// Moemon Maker: a song number's entry in gSongTable (a new track's: the BGM entry it plays on)
u16 MP_Kaisong_TableIndex(u16 song)
{
    return song >= MAKER_SONG_FIRST ? MUS_LITTLEROOT : song;
}

// Moemon Maker: the user's new tracks (MoemonMaker/music/), by name
int MP_Kaisong_TrackCount(void)
{
    if (!sKai.scanned)
        Scan();
    return sTrackCount;
}

const char *MP_Kaisong_TrackName(int i)
{
    if (!sKai.scanned)
        Scan();
    return i >= 0 && i < sTrackCount ? sTracks[i] : NULL;
}

int MP_Kaisong_TrackId(const char *name)
{
    int i;
    if (!sKai.scanned)
        Scan();
    for (i = 0; i < sTrackCount; i++)
        if (strcmp(sTracks[i], name) == 0)
            return MAKER_SONG_FIRST + i;
    return -1;
}

// Moemon Maker: the folders read again (the editor added or removed a file)
void MP_Kaisong_Rescan(void)
{
    sKai.scanned = FALSE;
}

// ---------------------------------------------------------------------------
// Loading a file (docs/KAISONG_FORMAT.md "Loading it in the game")

struct Reader
{
    const u8 *p, *end;
    bool8 bad;
};

static u32 R32(struct Reader *r)
{
    u32 v;
    if (r->end - r->p < 4) { r->bad = TRUE; r->p = r->end; return 0; }
    v = r->p[0] | (r->p[1] << 8) | (r->p[2] << 16) | ((u32)r->p[3] << 24);
    r->p += 4;
    return v;
}

static u16 R16(struct Reader *r)
{
    u16 v;
    if (r->end - r->p < 2) { r->bad = TRUE; r->p = r->end; return 0; }
    v = r->p[0] | (r->p[1] << 8);
    r->p += 2;
    return v;
}

static const u8 *RBytes(struct Reader *r, u32 n)
{
    const u8 *p = r->p;
    if ((u32)(r->end - r->p) < n) { r->bad = TRUE; r->p = r->end; return NULL; }
    r->p += n;
    return p;
}

static void RAlign(struct Reader *r, const u8 *base)
{
    while (((r->p - base) & 3) && r->p < r->end)
        r->p++;
}

struct Loaded
{
    const void **names;     // NAME: the game's sample / wave each name is
    u32 nnames;
    void **smpl, **wave;    // SMPL / WAVE: the song's own
    u32 nsmpl, nwave;
    u8 *kspl;
    u32 nkspl;
    struct MP2KInstrument *groups;
    u32 ngroups;
};

static const void *GameSample(const char *name, u16 len)
{
    int i;
    for (i = 0; i < (int)ARRAY_COUNT(sKaisongSamples); i++)
        if (strlen(sKaisongSamples[i].name) == len && memcmp(sKaisongSamples[i].name, name, len) == 0)
            return sKaisongSamples[i].data;
    return NULL;
}

static int CmpU32(const void *a, const void *b)
{
    u32 x = *(const u32 *)a, y = *(const u32 *)b;
    return x < y ? -1 : x > y;
}

// A track's code with each 4-byte jump offset grown into a pointer (relocs sorted)
static u8 *BuildTrack(const u8 *code, u32 len, u32 *relocs, u32 n)
{
    const u32 P = sizeof(uintptr_t);
    u32 i, k, out = 0, newLen = len + n * (P - 4) + 1;
    u8 *buf;
    for (i = 0; i < n; i++)
        if (relocs[i] + 4 > len || (i && relocs[i] < relocs[i - 1] + 4))
            return NULL;
    if (!(buf = malloc(newLen)))
        return NULL;
    // copy the code, leaving room for the pointers
    for (i = 0, k = 0; i < len; )
    {
        if (k < n && i == relocs[k])
        {
            memset(buf + out, 0, P);
            out += P;
            i += 4;
            k++;
        }
        else
            buf[out++] = code[i++];
    }
    buf[out] = 0xB1;    // FINE, should a track ever run past its end
    for (k = 0; k < n; k++)
    {
        u32 r = relocs[k], target, j, before = 0, at;
        uintptr_t ptr;
        target = code[r] | (code[r + 1] << 8) | (code[r + 2] << 16) | ((u32)code[r + 3] << 24);
        if (target >= len)
        {
            free(buf);
            return NULL;
        }
        for (j = 0; j < n && relocs[j] < target; j++)
            before++;
        ptr = (uintptr_t)(buf + target + (P - 4) * before);
        at = r + (P - 4) * k;
        for (j = 0; j < P; j++)
            buf[at + j] = (u8)(ptr >> (8 * j));
    }
    return buf;
}

static bool8 Fail(struct KaiFile *f, const char *why)
{
    MPNet_Log("kaisong: %s: %s; the game's own %s plays", f->file, why, SongName(f->song));
    f->failed = TRUE;
    return FALSE;
}

// The pieces a file brings live as long as the game (a song plays again and again; a few KB to a few MB each)
static bool8 Load(struct KaiFile *f)
{
    char path[1200];
    FILE *fp;
    long size;
    u8 *data;
    struct Reader r, c;
    struct Loaded L = {0};
    const u8 *info = NULL, *names = NULL, *vgrp = NULL, *kspl = NULL, *trak = NULL, *smpl = NULL, *wave = NULL;
    u32 infoLen = 0, namesLen = 0, vgrpLen = 0, ksplLen = 0, trakLen = 0, smplLen = 0, waveLen = 0;
    u32 version, chunks, i, s, trackCount;
    u8 priority, reverb;
    struct SongHeader *h;

    snprintf(path, sizeof(path), "%s/%s", DirOf(f), f->file);
    if (!(fp = fopen(path, "rb")))
        return Fail(f, "can't open it");
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (size < 32 || size > 64 * 1024 * 1024 || !(data = malloc(size)))
    {
        fclose(fp);
        return Fail(f, "not a song file");
    }
    if (fread(data, 1, size, fp) != (size_t)size)
    {
        fclose(fp);
        free(data);
        return Fail(f, "can't read it");
    }
    fclose(fp);

    r.p = data;
    r.end = data + size;
    r.bad = FALSE;
    if (memcmp(data, "KAISONG\0", 8) != 0)
    {
        free(data);
        return Fail(f, "not a .kaisong file");
    }
    r.p += 8;
    version = R16(&r);
    s = R16(&r);
    trackCount = data[12];
    priority = data[13];
    reverb = data[14];
    r.p = data + 16;
    chunks = R32(&r);
    if (version < 1 || version > 2)
    {
        free(data);
        return Fail(f, "made by a newer Moemon Music Studio (format version too new)");
    }
    if (s < 32 || s > (u32)size || trackCount < 1 || trackCount > KAI_MAX_TRACKS)
    {
        free(data);
        return Fail(f, "a broken header");
    }
    r.p = data + s;
    for (i = 0; i < chunks && r.p < r.end && !r.bad; i++)
    {
        const u8 *id = RBytes(&r, 4), *body;
        u32 len = R32(&r);
        body = RBytes(&r, len);
        if (!body)
            break;
        RAlign(&r, data);
        if (!memcmp(id, "INFO", 4)) info = body, infoLen = len;
        else if (!memcmp(id, "NAME", 4)) names = body, namesLen = len;
        else if (!memcmp(id, "VGRP", 4)) vgrp = body, vgrpLen = len;
        else if (!memcmp(id, "KSPL", 4)) kspl = body, ksplLen = len;
        else if (!memcmp(id, "TRAK", 4)) trak = body, trakLen = len;
        else if (!memcmp(id, "SMPL", 4)) smpl = body, smplLen = len;
        else if (!memcmp(id, "WAVE", 4)) wave = body, waveLen = len;
    }
    if (r.bad || !names || !vgrp || !kspl || !trak)
    {
        free(data);
        return Fail(f, "cut short (a chunk is missing)");
    }
    {
        // the music itself: the header's song fields and every chunk but INFO (FNV-1a)
        const u8 *parts[7] = {data + 8, names, vgrp, kspl, trak, smpl, wave};
        u32 lens[7] = {24, namesLen, vgrpLen, ksplLen, trakLen, smpl ? smplLen : 0, wave ? waveLen : 0}, k, n;
        f->music = 2166136261u;
        for (k = 0; k < 7; k++)
            for (n = 0; n < lens[k]; n++)
                f->music = (f->music ^ parts[k][n]) * 16777619u;
    }

    // INFO: the voices it was made for, and its gain (tools/kaisong/loudness.py measures it against the song it replaces)
    f->voices = KAI_GAME_VOICES;
    f->gain = 256;
    if (info)
    {
        char line[80];
        u32 a = 0;
        while (a < infoLen)
        {
            u32 b = a, n;
            while (b < infoLen && info[b] != '\n')
                b++;
            n = b - a < sizeof(line) - 1 ? b - a : sizeof(line) - 1;
            memcpy(line, info + a, n);
            line[n] = 0;
            if (!strncmp(line, "voices=", 7))
            {
                int v = atoi(line + 7);
                f->voices = v < 1 ? KAI_GAME_VOICES : v > MAX_DIRECTSOUND_CHANNELS ? MAX_DIRECTSOUND_CHANNELS : v;
            }
            else if (!strncmp(line, "gain=", 5))
            {
                double db = atof(line + 5);
                f->gain = GainFactor((int)(db * 10.0 + (db < 0 ? -0.5 : 0.5)));
            }
            a = b + 1;
        }
    }
    if (f->tableGain != KAI_NO_GAIN)
        f->gain = GainFactor(f->tableGain);
    if (sMeasureFile != NULL && strcmp(sMeasureFile, f->file) == 0 && sMeasureTenths != KAI_NO_GAIN)
        f->gain = GainFactor(sMeasureTenths);

    // NAME: the game's samples and waves
    c.p = names; c.end = names + namesLen; c.bad = FALSE;
    L.nnames = R32(&c);
    if (L.nnames > 4096 || !(L.names = calloc(L.nnames + 1, sizeof(*L.names))))
    {
        free(data);
        return Fail(f, "a broken sample list");
    }
    for (i = 0; i < L.nnames && !c.bad; i++)
    {
        u16 n = R16(&c);
        const char *nm = (const char *)RBytes(&c, n);
        if (!nm)
            break;
        if (!(L.names[i] = GameSample(nm, n)))
        {
            char why[160];
            snprintf(why, sizeof(why), "plays %.*s, which this game doesn't have (made for a newer game?)", n > 80 ? 80 : n, nm);
            free(data);
            return Fail(f, why);
        }
    }
    if (c.bad)
    {
        free(data);
        return Fail(f, "a broken sample list");
    }

    // SMPL / WAVE (version 2): the song's own samples, laid out as the game's WaveData
    if (smpl)
    {
        c.p = smpl; c.end = smpl + smplLen; c.bad = FALSE;
        L.nsmpl = R32(&c);
        if (L.nsmpl > 4096 || !(L.smpl = calloc(L.nsmpl + 1, sizeof(*L.smpl))))
            c.bad = TRUE;
        for (i = 0; i < L.nsmpl && !c.bad; i++)
        {
            u16 n = R16(&c);
            const u8 *hdr;
            u32 count;
            RBytes(&c, n);
            RAlign(&c, smpl);
            hdr = RBytes(&c, 16);
            if (!hdr)
                break;
            count = hdr[12] | (hdr[13] << 8) | (hdr[14] << 16) | ((u32)hdr[15] << 24);
            if (count > 16 * 1024 * 1024 || (u32)(c.end - c.p) < count || !(L.smpl[i] = calloc(1, 16 + count + 16)))
            {
                c.bad = TRUE;
                break;
            }
            memcpy(L.smpl[i], hdr, 16);
            memcpy((u8 *)L.smpl[i] + 16, c.p, count);   // then zeros: the mixer reads a sample past the end
            c.p += count;
            RAlign(&c, smpl);
        }
        if (c.bad)
        {
            free(data);
            return Fail(f, "its own samples are cut short");
        }
    }
    if (wave)
    {
        c.p = wave; c.end = wave + waveLen; c.bad = FALSE;
        L.nwave = R32(&c);
        if (L.nwave > 4096 || !(L.wave = calloc(L.nwave + 1, sizeof(*L.wave))))
            c.bad = TRUE;
        for (i = 0; i < L.nwave && !c.bad; i++)
        {
            u16 n = R16(&c);
            const u8 *w;
            RBytes(&c, n);
            RAlign(&c, wave);
            if (!(w = RBytes(&c, 16)) || !(L.wave[i] = malloc(16)))
            {
                c.bad = TRUE;
                break;
            }
            memcpy(L.wave[i], w, 16);
        }
        if (c.bad)
        {
            free(data);
            return Fail(f, "its own Game Boy waves are cut short");
        }
    }

    // KSPL: key-split tables
    c.p = kspl; c.end = kspl + ksplLen; c.bad = FALSE;
    L.nkspl = R32(&c);
    if (L.nkspl > 1024 || (u32)(c.end - c.p) < L.nkspl * 128 || !(L.kspl = malloc(L.nkspl * 128 + 1)))
    {
        free(data);
        return Fail(f, "broken key-split tables");
    }
    memcpy(L.kspl, c.p, L.nkspl * 128);

    // VGRP: voice groups (group 0 = the song's voicegroup)
    c.p = vgrp; c.end = vgrp + vgrpLen; c.bad = FALSE;
    L.ngroups = R32(&c);
    if (L.ngroups < 1 || L.ngroups > 1024 || (u32)(c.end - c.p) < L.ngroups * 128 * 16
     || !(L.groups = calloc(L.ngroups * 128, sizeof(struct MP2KInstrument))))
    {
        free(data);
        return Fail(f, "broken voice groups");
    }
    for (i = 0; i < L.ngroups * 128; i++)
    {
        const u8 *v = c.p + i * 16;
        struct MP2KInstrument *in = &L.groups[i];
        u8 kind = v[8];
        u16 ref = v[10] | (v[11] << 8), ks = v[12] | (v[13] << 8);
        bool8 ok = TRUE;
        in->type = v[0];
        in->drumKey = v[1];
        in->cgbLength = v[2];
        in->panSweep = v[3];
        in->attack = v[4];
        in->decay = v[5];
        in->sustain = v[6];
        in->release = v[7];
        switch (kind)
        {
        case 0: in->wav = NULL; break;
        case 1: ok = ref < L.nnames; if (ok) in->wav = (struct WaveData *)L.names[ref]; break;
        case 2: ok = ref < L.nnames; if (ok) in->cgb3Sample = (u32 *)L.names[ref]; break;
        case 3: in->wav = NULL; in->squareNoiseConfig = ref; break;
        case 4:
        case 5:
            ok = ref >= 1 && ref < L.ngroups && i < 128 && (kind == 4 || ks < L.nkspl);
            if (ok)
            {
                in->group = &L.groups[ref * 128];
                in->keySplitTable = kind == 5 ? L.kspl + ks * 128 : NULL;   // after the envelope bytes: they share memory
            }
            break;
        case 6: ok = ref < L.nsmpl; if (ok) in->wav = (struct WaveData *)L.smpl[ref]; break;
        case 7: ok = ref < L.nwave; if (ok) in->cgb3Sample = (u32 *)L.wave[ref]; break;
        default: ok = FALSE;
        }
        if (!ok)
        {
            free(data);
            return Fail(f, "a voice points at something the file doesn't have");
        }
    }

    // TRAK: the tracks, jump offsets turned into pointers
    c.p = trak; c.end = trak + trakLen; c.bad = FALSE;
    if (R32(&c) != trackCount)
    {
        free(data);
        return Fail(f, "the track count doesn't match");
    }
    if (!(h = calloc(1, sizeof(struct SongHeader) + trackCount * sizeof(u8 *))))
    {
        free(data);
        return Fail(f, "out of memory");
    }
    for (i = 0; i < trackCount; i++)
    {
        u32 len = R32(&c), nrel = R32(&c), k, *rel;
        const u8 *code = RBytes(&c, len);
        if (!code || nrel > len || (u32)(c.end - c.p) < nrel * 4 || !(rel = malloc((nrel + 1) * 4)))
        {
            free(data);
            return Fail(f, "a track is cut short");
        }
        for (k = 0; k < nrel; k++)
            rel[k] = R32(&c);
        RAlign(&c, trak);
        qsort(rel, nrel, 4, CmpU32);
        h->part[i] = BuildTrack(code, len, rel, nrel);
        free(rel);
        if (!h->part[i])
        {
            free(data);
            return Fail(f, "a track jumps outside itself");
        }
    }
    h->trackCount = trackCount;
    h->blockCount = 0;
    h->priority = priority;
    h->reverb = reverb;
    h->tone = (struct ToneData *)L.groups;
    free(data);
    free(L.names);   // the samples themselves stay (the game's, or the ones in L.smpl / L.wave)
    free(L.smpl);
    free(L.wave);
    f->header = h;
    MPNet_Log("kaisong: %s plays as %s%s (%d tracks, %d voices, %u bytes)", f->file, SongName(f->song),
              f->scene >= 0 ? " in its scene" : f->group < 0 ? " everywhere" : " here", trackCount, f->voices, (unsigned)size);
    return TRUE;
}

// ---------------------------------------------------------------------------

// The file for song n: in a scene its own, else the everywhere one; in the world the map's, else the everywhere one
static struct KaiFile *Find(u16 song, int scene, s16 group, s16 num)
{
    struct KaiFile *all = NULL;
    int i;
    if (!sKai.scanned)
        Scan();
    for (i = 0; i < sKai.count; i++)
    {
        struct KaiFile *f = &sKai.files[i];
        if (f->song != song || f->failed)
            continue;
        if (scene >= 0 ? f->scene == scene : f->scene < 0 && f->group >= 0 && f->group == group && f->num == num)
            return f;
        if (f->group < 0 && f->scene < 0 && !all)
            all = f;
    }
    return all;
}

static struct SongHeader *HeaderOf(struct KaiFile *f, struct SongHeader *builtin)
{
    if (!f)
        return builtin;
    if (!f->header && !f->failed)
        Load(f);
    return f->header ? f->header : builtin;
}

struct SongHeader *MP_Kaisong_Header(u16 song, struct SongHeader *builtin)
{
    if (!sKai.scanned)
        Scan();
    if (!sKai.count || !gSaveBlock1Ptr)
        return builtin;
    {
        int scene = SceneFor(song);
        struct SongHeader *h = HeaderOf(Find(song, scene, gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum), builtin);
        if (getenv("MOEMON_MUSIC_LOG") && gSongTable[MP_Kaisong_TableIndex(song)].ms == 0)   // tests: which scene each song plays for
            MPNet_Log("kaisong: song %d (%s) for %s%s", song, song < ARRAY_COUNT(sKaisongSongNames) ? sKaisongSongNames[song]
                      : song >= MAKER_SONG_FIRST && MP_Kaisong_TrackName(song - MAKER_SONG_FIRST) ? MP_Kaisong_TrackName(song - MAKER_SONG_FIRST) : "?", scene >= 0 ? "scene " : "the map",
                      scene >= 0 ? sKaisongScenes[scene].key : "");
        return h;
    }
}

bool8 MP_Kaisong_Differs(u16 song, const struct WarpData *warp)
{
    struct SongHeader *want;
    if (!sKai.count || !gSaveBlock1Ptr || song == 0 || song >= ARRAY_COUNT(sKaisongSongNames) || !gMPlayInfo_BGM.songHeader)
        return FALSE;
    if (MP_Kaisong_SongOf(gMPlayInfo_BGM.songHeader) != song)
        return FALSE;
    if (!warp)
        warp = &gSaveBlock1Ptr->location;
    want = HeaderOf(Find(song, -1, warp->mapGroup, warp->mapNum), gSongTable[MP_Kaisong_TableIndex(song)].header);
    return want != gMPlayInfo_BGM.songHeader && !MP_Kaisong_SameMusic(song, warp);
}

static const struct KaiFile *FileOf(const struct SongHeader *header)
{
    int i;
    for (i = 0; header && i < sKai.count; i++)
        if (sKai.files[i].header == header)
            return &sKai.files[i];
    return NULL;
}

// TRUE when the song that would play for `song` at warp (NULL = here) is the same music the BGM player plays now:
// the same header, or two .kaisong files holding the same music (the GAME CENTER's floors, one song replacing two
// game songs). The overworld then keeps it playing instead of starting it again.
bool8 MP_Kaisong_SameMusic(u16 song, const struct WarpData *warp)
{
    const struct SongHeader *now = gMPlayInfo_BGM.songHeader, *want;
    const struct KaiFile *a, *b;
    if (!sKai.count || !gSaveBlock1Ptr || song == 0 || song >= ARRAY_COUNT(sKaisongSongNames) || !now
     || !(gMPlayInfo_BGM.status & MUSICPLAYER_STATUS_TRACK) || (gMPlayInfo_BGM.status & MUSICPLAYER_STATUS_PAUSE)
     || (gMPlayInfo_BGM.fadeOI && !(gMPlayInfo_BGM.fadeOV & FADE_IN)))   // (fading out: it is on its way out)
        return FALSE;
    if (!warp)
        warp = &gSaveBlock1Ptr->location;
    want = HeaderOf(Find(song, -1, warp->mapGroup, warp->mapNum), gSongTable[MP_Kaisong_TableIndex(song)].header);
    a = FileOf(now);
    b = FileOf(want);
    return a && b && (a == b || a->music == b->music);   // (only custom songs: the game's own behave as they always did)
}

u16 MP_Kaisong_SongOf(const struct SongHeader *header)
{
    u16 i;
    if (!header)
        return 0;
    for (i = 0; i < sKai.count; i++)
        if (sKai.files[i].header == header)
            return sKai.files[i].song;
    for (i = 1; i < ARRAY_COUNT(sKaisongSongNames); i++)
        if (gSongTable[i].header == header)
            return i;
    return 0;
}

u8 MP_Kaisong_VariantOf(const struct SongHeader *header)
{
    u16 i;
    for (i = 0; i < sKai.count; i++)
        if (sKai.files[i].header == header)
            return sKai.files[i].variant;
    return 0;
}

void MP_Kaisong_StartVariant(u16 song, u8 variant)
{
    struct SongHeader *h = gSongTable[MP_Kaisong_TableIndex(song)].header;
    int i;
    if (!sKai.scanned)
        Scan();
    for (i = 0; i < sKai.count; i++)
    {
        struct KaiFile *f = &sKai.files[i];
        if (f->song == song && !f->failed && (variant ? f->variant == variant : f->group < 0 && f->scene < 0))
        {
            h = HeaderOf(f, h);
            break;
        }
    }
    MPlayStart(gMPlayTable[gSongTable[MP_Kaisong_TableIndex(song)].ms].info, h);
}

// The BGM player's song decides how many sample voices mix at once
void MP_Kaisong_Started(const struct SongHeader *header)
{
    struct SoundInfo *soundInfo = SOUND_INFO_PTR;
    u8 want = KAI_GAME_VOICES;
    int i;
    for (i = 0; i < sKai.count; i++)
        if (sKai.files[i].header == header)
            want = sKai.files[i].voices;
    if (!soundInfo || soundInfo->ident != ID_NUMBER || soundInfo->maxChans == want)
        return;
    soundInfo->ident++;
    for (i = want; i < soundInfo->maxChans && i < MAX_DIRECTSOUND_CHANNELS; i++)
        soundInfo->chans[i].statusFlags = 0;
    soundInfo->maxChans = want;
    soundInfo->ident = ID_NUMBER;
}

// The gain of the song a music player plays (256 = as made, also for the game's own songs): music_player.c
// ChnVolSetAsm scales each voice's volume by it
u16 MP_Kaisong_Gain(const struct SongHeader *header)
{
    int i;
    if (header)
        for (i = 0; i < sKai.count; i++)
            if (sKai.files[i].header == header)
                return sKai.files[i].gain;
    return 256;
}

// Tests (mp playkai FILE): the file's song on its music player, whatever the map or scene; FALSE when it can't play
bool8 MP_Kaisong_PlayFile(const char *file)
{
    int i;
    if (!sKai.scanned)
        Scan();
    for (i = 0; i < sKai.count; i++)
    {
        struct KaiFile *f = &sKai.files[i];
        if (strcmp(f->file, file) == 0)
        {
            struct SongHeader *h = HeaderOf(f, NULL);
            if (!h)
                return FALSE;
            MPlayStart(gMPlayTable[gSongTable[MP_Kaisong_TableIndex(f->song)].ms].info, h);
            return TRUE;
        }
    }
    return FALSE;
}

// Tools (MOEMON_MEASURE_MUSIC, mp_export_sound.c): FILE's song started on its player at TENTHS of a dB, whatever its
// table's or INFO's gain; its player, NULL when it can't play
struct MusicPlayerInfo *MP_Kaisong_MeasureStart(const char *file, int tenths)
{
    int i;
    if (!sKai.scanned)
        Scan();
    for (i = 0; i < sKai.count; i++)
    {
        struct KaiFile *f = &sKai.files[i];
        if (strcmp(f->file, file) == 0 && !f->maker)
        {
            struct MusicPlayerInfo *mp = gMPlayTable[gSongTable[MP_Kaisong_TableIndex(f->song)].ms].info;
            struct SongHeader *h;
            sMeasureFile = f->file;
            sMeasureTenths = tenths;
            h = HeaderOf(f, NULL);
            if (h)
                f->gain = GainFactor(tenths);   // (loaded before: set again)
            sMeasureFile = NULL;
            if (!h)
                return NULL;
            MPlayStart(mp, h);
            return mp;
        }
    }
    return NULL;
}

void MP_Kaisong_Log(void)
{
    int i;
    if (!sKai.scanned)
        Scan();
    MPNet_Log("kaisong: %d files in %s", sKai.count, MusicDir());
    for (i = 0; i < sKai.count; i++)
    {
        struct KaiFile *f = &sKai.files[i];
        MPNet_Log("kaisong:   %s -> song %d (%s), map %d.%d, scene %s%s", f->file, f->song, SongName(f->song),
                  f->group, f->num, f->scene >= 0 ? sKaisongScenes[f->scene].key : "-",
                  f->failed ? ", can't be played" : f->header ? ", loaded" : "");
    }
}

// Moemon Maker: the scenes a song can be replaced in (the editor's place picker), and a map's place key
int MP_Kaisong_SceneCount(void)
{
    return ARRAY_COUNT(sKaisongScenes);
}

const char *MP_Kaisong_SceneKey(int i)
{
    return i >= 0 && i < (int)ARRAY_COUNT(sKaisongScenes) ? sKaisongScenes[i].key : NULL;
}

// The lowercase name a song's files use (mus_route101), or NULL
const char *MP_Kaisong_SongFileName(u16 song)
{
    if (song >= MAKER_SONG_FIRST)
        return MP_Kaisong_TrackName(song - MAKER_SONG_FIRST);
    return song < ARRAY_COUNT(sKaisongSongNames) && sKaisongSongNames[song][0] ? sKaisongSongNames[song] : NULL;
}
