// Moemon Rhythm: the DANCE machine's song list. StepMania's layout next to the game: stepmania/Songs/<group>/<song>/
// with a .sm or .ssc, its music (whatever #MUSIC says, else the folder's .ogg / .mp3), its banner, background and
// the EXTREME wheel's title card (card.png). Groups are named by the pack's sortorder.txt when it has them
// ("08 ddr extreme" names ex08), else by their folder. Read again each time someone steps up to the machine.
// MOEMON_STEPMANIA_DIR moves the folder (tests).
#include <ctype.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "rh_songs.h"

static struct RhEntry *sSongs;
static int sCount, sCap;
static char sGroupNames[RH_GROUPS][RH_TEXT];
static int sGroupCount;

const char *Updater_SongsRoot(void);

// MOEMON_STEPMANIA_DIR, else the songs the machine downloaded next to the exe, else stepmania/ where the game runs
const char *RhSongs_Root(void)
{
    const char *env = getenv("MOEMON_STEPMANIA_DIR");
    struct stat st;
    char songs[1200];
    if (env && env[0])
        return env;
    snprintf(songs, sizeof(songs), "%s/Songs", Updater_SongsRoot());
    if (stat(songs, &st) == 0 && S_ISDIR(st.st_mode))
        return Updater_SongsRoot();
    return "stepmania";
}

static int Exists(const char *path)
{
    struct stat st;
    return stat(path, &st) == 0 && S_ISREG(st.st_mode);
}

static int EndsWith(const char *s, const char *ext)
{
    size_t n = strlen(s), e = strlen(ext);
    size_t i;
    if (n < e)
        return 0;
    for (i = 0; i < e; i++)
        if (tolower((unsigned char)s[n - e + i]) != tolower((unsigned char)ext[i]))
            return 0;
    return 1;
}

// A file the simfile names, else the first in the folder with one of the endings (and not with `not` in it)
static void FindFile(char *out, size_t size, const char *dir, const char *named, const char *const *exts,
                     const char *must, const char *not)
{
    DIR *d;
    struct dirent *e;
    out[0] = 0;
    if (named && named[0])
    {
        snprintf(out, size, "%s/%s", dir, named);
        if (Exists(out))
            return;
        out[0] = 0;
    }
    if (!(d = opendir(dir)))
        return;
    while ((e = readdir(d)) != NULL)
    {
        int k, ok = 0;
        char low[256];
        for (k = 0; e->d_name[k] && k < 255; k++)
            low[k] = (char)tolower((unsigned char)e->d_name[k]);
        low[k] = 0;
        for (k = 0; exts[k]; k++)
            if (EndsWith(e->d_name, exts[k]))
                ok = 1;
        if (!ok || (must && !strstr(low, must)) || (not && strstr(low, not)))
            continue;
        snprintf(out, size, "%s/%s", dir, e->d_name);
        break;
    }
    closedir(d);
}

static void ReadGroupNames(const char *root)
{
    char path[600], line[256];
    FILE *f;
    sGroupCount = 0;
    memset(sGroupNames, 0, sizeof(sGroupNames));
    snprintf(path, sizeof(path), "%s/Songs/sortorder.txt", root);
    if (!(f = fopen(path, "r")))
        return;
    while (fgets(line, sizeof(line), f))
    {
        int num, i, n;
        char *name;
        if (sscanf(line, "%d%n", &num, &n) != 1 || num < 0 || num >= RH_GROUPS)
            continue;
        name = line + n;
        while (*name == ' ')
            name++;
        for (i = 0; name[i] && name[i] != '\r' && name[i] != '\n' && i < RH_TEXT - 1; i++)
            sGroupNames[num][i] = (char)toupper((unsigned char)name[i]);
        sGroupNames[num][i] = 0;
    }
    fclose(f);
}

// "ex08" -> the sortorder's 8, else the folder's name in capitals
static void GroupTitle(char *out, const char *folder)
{
    const char *p = folder;
    int i;
    while (*p && !isdigit((unsigned char)*p))
        p++;
    if (*p && atoi(p) >= 0 && atoi(p) < RH_GROUPS && sGroupNames[atoi(p)][0])
    {
        snprintf(out, RH_TEXT, "%s", sGroupNames[atoi(p)]);
        return;
    }
    for (i = 0; folder[i] && i < RH_TEXT - 1; i++)
        out[i] = (char)toupper((unsigned char)folder[i]);
    out[i] = 0;
}

static void Add(const char *dir, const char *group, int groupIndex)
{
    static const char *const simfiles[] = {".ssc", NULL};
    static const char *const smfiles[] = {".sm", NULL};
    static const char *const audio[] = {".ogg", ".mp3", ".oga", NULL};
    static const char *const pics[] = {".png", ".jpg", ".jpeg", ".bmp", NULL};
    char sim[600];
    struct RhEntry *e;
    FindFile(sim, sizeof(sim), dir, NULL, simfiles, NULL, NULL);
    if (!sim[0])
        FindFile(sim, sizeof(sim), dir, NULL, smfiles, NULL, NULL);
    if (!sim[0])
        return;
    if (sCount == sCap)
    {
        sCap = sCap ? sCap * 2 : 256;
        sSongs = realloc(sSongs, sCap * sizeof(*sSongs));
    }
    e = &sSongs[sCount];
    memset(e, 0, sizeof(*e));
    if (!RhSong_Load(&e->song, sim, group))
        return;
    FindFile(e->music, sizeof(e->music), dir, e->song.music, audio, NULL, NULL);
    if (!e->music[0])
    {
        RhSong_Free(&e->song);
        return;
    }
    FindFile(e->banner, sizeof(e->banner), dir, e->song.banner, pics, NULL, "bg");
    FindFile(e->background, sizeof(e->background), dir, e->song.background, pics, "bg", NULL);
    snprintf(e->card, sizeof(e->card), "%s/card.png", dir);
    if (!Exists(e->card))
        e->card[0] = 0;
    e->group = groupIndex;
    sCount++;
}

static int CompareName(const void *a, const void *b)
{
    return strcmp(*(const char *const *)a, *(const char *const *)b);
}

void RhSongs_Free(void)
{
    int i;
    for (i = 0; i < sCount; i++)
        RhSong_Free(&sSongs[i].song);
    free(sSongs);
    sSongs = NULL;
    sCount = sCap = 0;
}

int RhSongs_Scan(void)
{
    char songsDir[600];
    char *groups[RH_GROUPS];
    int groupCount = 0, g;
    DIR *d;
    struct dirent *e;
    RhSongs_Free();
    ReadGroupNames(RhSongs_Root());
    snprintf(songsDir, sizeof(songsDir), "%s/Songs", RhSongs_Root());
    if (!(d = opendir(songsDir)))
        return 0;
    while ((e = readdir(d)) != NULL && groupCount < RH_GROUPS)
    {
        char p[1200];
        struct stat st;
        if (e->d_name[0] == '.')
            continue;
        snprintf(p, sizeof(p), "%s/%s", songsDir, e->d_name);
        if (stat(p, &st) == 0 && S_ISDIR(st.st_mode))
            groups[groupCount++] = strdup(e->d_name);
    }
    closedir(d);
    qsort(groups, groupCount, sizeof(groups[0]), CompareName);
    for (g = 0; g < groupCount; g++)
    {
        char gp[1200];
        char *songs[1024];
        int n = 0, i;
        char title[RH_TEXT];
        snprintf(gp, sizeof(gp), "%s/%s", songsDir, groups[g]);
        GroupTitle(title, groups[g]);
        if ((d = opendir(gp)) != NULL)
        {
            while ((e = readdir(d)) != NULL && n < 1024)
                if (e->d_name[0] != '.')
                    songs[n++] = strdup(e->d_name);
            closedir(d);
        }
        qsort(songs, n, sizeof(songs[0]), CompareName);
        for (i = 0; i < n; i++)
        {
            char sp[2400];
            struct stat st;
            snprintf(sp, sizeof(sp), "%s/%s", gp, songs[i]);
            if (stat(sp, &st) == 0 && S_ISDIR(st.st_mode))
                Add(sp, title, g);
            free(songs[i]);
        }
        free(groups[g]);
    }
    return sCount;
}

int RhSongs_Count(void)
{
    return sCount;
}

struct RhEntry *RhSongs_Get(int i)
{
    return i >= 0 && i < sCount ? &sSongs[i] : NULL;
}

// A chart's key for the records: the notes' hash and the difficulty (the same chart in another folder shares them)
uint32_t RhSongs_ChartKey(const struct RhEntry *e, int diff)
{
    return Rh_Hash(&diff, sizeof(diff), e->song.charts[diff].hash);
}
