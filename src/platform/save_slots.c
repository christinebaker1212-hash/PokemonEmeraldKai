// Moemon Co-op: the save files' own folder and three save files.
//
// The save and everything that belongs with it (its sidecars "<save>.life", ".cards", ".ygo"... and its backups
// "<save>.bak1"...) live in saves/slot1, saves/slot2 or saves/slot3 beside the game, one save file each, so they
// never mix. saves/slot.txt remembers the one in use. A save from before (pokeemerald.sav and its files next to the
// exe) moves into saves/slot1 the first time this version starts, every file as it was. The title screen's
// SAVE FILE item picks another one in place (main_menu.c reloads the save; MP_SaveSlotChanged the rest).
// Settings belong to a save file too (moemon_coop.ini, moemon_controls.ini, moemon_display.ini, lobby.txt:
// SaveSlots_File): File 1's are every file's starting point, and another file keeps only what it changed.
// --save PATH or MOEMON_SAVE (tests, several clients on one machine) use that path and no slots.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#else
#include <unistd.h>
#endif
#include <SDL2/SDL.h>
#include "global.h"
#include "save_slots.h"
#include "aw_file.h"
#include "mp_region.h"

extern char gPlatformSavePath[512];
int gPlatformArgc;
char **gPlatformArgv;

#define SAVES_DIR "saves"
#define SAVE_NAME "pokeemerald.sav"

static int sSlot;   // 1-3, 0: an explicit save path, no slots

static int Exists(const char *path)
{
    struct stat st;
    return stat(path, &st) == 0;
}

static void MakeDir(const char *path)
{
#ifdef _WIN32
    _mkdir(path);
#else
    mkdir(path, 0755);
#endif
}

static void SlotDir(int slot, char *out, size_t size)
{
    snprintf(out, size, SAVES_DIR "/slot%d", slot);
}

static void SetPath(int slot, const char *name, const char *ext, char *out, size_t size)
{
    snprintf(out, size, SAVES_DIR "/slot%d/%s%s", slot, name, ext);
}

static void SlotSave(int slot, char *out, size_t size)
{
    snprintf(out, size, SAVES_DIR "/slot%d/" SAVE_NAME, slot);
}

// pokeemerald.sav and its own files ("pokeemerald.sav.<anything>") from beside the exe into saves/slot1, unchanged
static void MoveOldSave(void)
{
    char dir[64], to[600];
    DIR *d;
    struct dirent *e;
    int moved = 0;
    if (!Exists(SAVE_NAME))
        return;
    SlotDir(1, dir, sizeof(dir));
    SlotSave(1, to, sizeof(to));
    if (Exists(to))
    {
        printf("[slots] %s is here and in %s too: left where it is\n", SAVE_NAME, dir);
        return;
    }
    MakeDir(SAVES_DIR);
    MakeDir(dir);
    d = opendir(".");
    if (d == NULL)
        return;
    while ((e = readdir(d)) != NULL)
    {
        const char *name = e->d_name;
        size_t n = strlen(SAVE_NAME);
        if (strncmp(name, SAVE_NAME, n) != 0 || (name[n] != 0 && name[n] != '.'))
            continue;
        snprintf(to, sizeof(to), "%s/%s", dir, name);
        if (Exists(to))
            continue;
        if (rename(name, to) == 0)
            moved++;
        else
            printf("[slots] could not move %s: %s\n", name, strerror(errno));
    }
    closedir(d);
    printf("[slots] moved %d save file(s) into %s\n", moved, dir);
}

void SaveSlots_Init(bool8 readOnly)
{
    char dir[64], path[600];
    FILE *f;
    int slot = 1;
    if (!readOnly)
        MoveOldSave();
    f = fopen(SAVES_DIR "/slot.txt", "r");
    if (f != NULL)
    {
        if (fscanf(f, "%d", &slot) != 1 || slot < 1 || slot > SAVE_SLOT_COUNT)
            slot = 1;
        fclose(f);
    }
    if (readOnly)
    {
        // (a dev build reads the save in use and never writes: the old place if nothing was moved yet)
        SlotSave(slot, path, sizeof(path));
        if (Exists(path))
            snprintf(gPlatformSavePath, 512, "%s", path);
        return;
    }
    sSlot = slot;
    MakeDir(SAVES_DIR);
    SlotDir(slot, dir, sizeof(dir));
    MakeDir(dir);
    SlotSave(slot, gPlatformSavePath, 512);
    printf("[slots] save file %d: %s\n", slot, gPlatformSavePath);
}

int SaveSlots_Current(void)
{
    return sSlot;
}

// "<save>.summary": who and where, written with every save, read by the title screen's SAVE FILE item
void SaveSlots_WriteSummary(void)
{
    char path[600], name[PLAYER_NAME_LENGTH + 4];
    FILE *f;
    void MP_GameToAscii(char *dst, const u8 *src, int maxLen);
    u8 region;
    if (gSaveBlock2Ptr == NULL)
        return;
    snprintf(path, sizeof(path), "%s.summary", gPlatformSavePath);
    f = fopen(path, "w");
    if (f == NULL)
        return;
    MP_GameToAscii(name, gSaveBlock2Ptr->playerName, PLAYER_NAME_LENGTH);
    region = MP_Region_Current();
    fprintf(f, "%s\n%u\n%u\n%u:%02u\n", name, region, MP_Region_BadgeCount(region), gSaveBlock2Ptr->playTimeHours,
            gSaveBlock2Ptr->playTimeMinutes);
    fclose(f);
}

static long FileSize(const char *path)
{
    struct stat st;
    return stat(path, &st) == 0 ? (long)st.st_size : -1;
}

// Without a summary (a save from before): the name and play time from the save's own SaveBlock2 (the newest copy of
// sector 0 among the two save slots in the flash image)
static bool8 ReadSaveHeader(const char *path, struct SaveSlotSummary *out)
{
    FILE *f = fopen(path, "rb");
    u8 sector[0x1000];
    u32 best = 0, i;
    bool8 found = FALSE;
    void MP_GameToAscii(char *dst, const u8 *src, int maxLen);
    if (f == NULL)
        return FALSE;
    for (i = 0; i < 28 && fread(sector, 1, sizeof(sector), f) == sizeof(sector); i++)
    {
        u16 id = sector[0xFF4] | sector[0xFF5] << 8;
        u32 sig = sector[0xFF8] | sector[0xFF9] << 8 | sector[0xFFA] << 16 | (u32)sector[0xFFB] << 24;
        u32 counter = sector[0xFFC] | sector[0xFFD] << 8 | sector[0xFFE] << 16 | (u32)sector[0xFFF] << 24;
        if (id != 0 || sig != 0x8012025 || (found && counter < best))
            continue;
        found = TRUE;
        best = counter;
        MP_GameToAscii(out->name, sector, PLAYER_NAME_LENGTH);
        out->hours = sector[0x0E] | sector[0x0F] << 8;
        out->minutes = sector[0x10];
    }
    fclose(f);
    return found;
}

bool8 SaveSlots_Summary(int slot, struct SaveSlotSummary *out)
{
    char path[600];
    FILE *f;
    unsigned region = 0, badges = 0, hours = 0, minutes = 0;
    memset(out, 0, sizeof(*out));
    SlotSave(slot, path, sizeof(path));
    out->exists = FileSize(path) > 0;   // (the game makes an empty file for a save file never saved)
    if (!out->exists)
        return FALSE;
    out->header = ReadSaveHeader(path, out);
    strcat(path, ".summary");
    f = fopen(path, "r");
    if (f == NULL)
        return TRUE;
    if (fgets(out->name, sizeof(out->name), f) != NULL)
        out->name[strcspn(out->name, "\r\n")] = 0;
    if (fscanf(f, "%u %u %u:%u", &region, &badges, &hours, &minutes) == 4)
    {
        out->known = TRUE;
        out->region = (u8)region;
        out->badges = (u8)badges;
        out->hours = (u16)hours;
        out->minutes = (u8)minutes;
    }
    fclose(f);
    return TRUE;
}

// ERASE: every file of save file slot (the save, its sidecars, backups and summary); the folder stays, empty
int SaveSlots_Erase(int slot)
{
    char dir[64], path[600];
    DIR *d;
    struct dirent *e;
    int n = 0;
    if (slot < 1 || slot > SAVE_SLOT_COUNT || sSlot == 0)
        return 0;
    SlotDir(slot, dir, sizeof(dir));
    AwFile_Flush();   // (a side file still being written would come back after it's gone)
    if (slot == sSlot)
    {
        void Platform_CloseSaveFile(void);
        Platform_CloseSaveFile();   // (Windows deletes no open file; SaveSlots_Switch opens it again, empty)
    }
    d = opendir(dir);
    if (d == NULL)
        return 0;
    while ((e = readdir(d)) != NULL)
    {
        size_t len = strlen(SAVE_NAME);
        if (strncmp(e->d_name, SAVE_NAME, len) != 0 || (e->d_name[len] != 0 && e->d_name[len] != '.'))
            continue;
        snprintf(path, sizeof(path), "%s/%s", dir, e->d_name);
        if (remove(path) == 0)
            n++;
    }
    closedir(d);
    if (slot != 1)
    {
        // (its own settings too: it follows File 1's again. File 1's settings stay: every file's starting point)
        static const char *const sSettings[] = {"moemon_coop.ini", "moemon_display.ini", "moemon_controls.ini", "lobby.txt"};
        int k;
        for (k = 0; k < (int)(sizeof(sSettings) / sizeof(sSettings[0])); k++)
        {
            SetPath(slot, sSettings[k], ".own", path, sizeof(path));
            n += remove(path) == 0;
            SetPath(slot, sSettings[k], "", path, sizeof(path));
            remove(path);
        }
    }
    printf("[slots] erased save file %d (%d files)\n", slot, n);
    return n;
}

static void CopyFileTo(const char *from, const char *to)
{
    unsigned char buf[4096];
    size_t n;
    FILE *in = fopen(from, "rb"), *out;
    if (in == NULL)
        return;
    out = fopen(to, "wb");
    if (out != NULL)
    {
        while ((n = fread(buf, 1, sizeof(buf), in)) > 0)
            fwrite(buf, 1, n, out);
        fclose(out);
        printf("[slots] %s: the shared one copied in\n", to);
    }
    fclose(in);
}

// ---- Settings that follow File 1 (the user, 2026-11-11: "File 1 determines the default options for every subsequent
// save file, with changes only being made if the file changes them")
//
// saves/slot1/<name> is the whole settings file: File 1's, and every other file's starting point. Another save file
// keeps only what it changed, in saves/slotN/<name>.own; saves/slotN/<name> is File 1's file with those lines laid
// over it, made again whenever the file is asked for, so a change in File 1 reaches every file that left it alone.
// The game writes the whole settings into saves/slotN/<name> as before; SaveSlots_Written then keeps in .own the lines
// that differ from File 1's. A key written more than once (spectate_friend) is one setting: all its lines go together.
// File 1 without a settings file takes the shared one beside the game (an older install) or, with none, the canonical
// settings below (a brand-new game: the user's list, 2026-11-11).

#define SET_LINES 600
#define SET_LEN 300

struct SetFile
{
    int n;
    char line[SET_LINES][SET_LEN];
    char key[SET_LINES][64];   // "" for a comment / blank line
};

static void KeyOf(const char *line, char *key, size_t size)
{
    const char *eq = strchr(line, '='), *a = line, *b;
    size_t n;
    key[0] = 0;
    if (eq == NULL || line[0] == ';' || line[0] == '#')
        return;
    while (*a == ' ' || *a == '\t')
        a++;
    b = eq;
    while (b > a && (b[-1] == ' ' || b[-1] == '\t'))
        b--;
    n = (size_t)(b - a) < size - 1 ? (size_t)(b - a) : size - 1;
    memcpy(key, a, n);
    key[n] = 0;
}

static void ReadSet(const char *path, struct SetFile *out)
{
    FILE *f = fopen(path, "r");
    char buf[SET_LEN];
    out->n = 0;
    if (f == NULL)
        return;
    while (out->n < SET_LINES && fgets(buf, sizeof(buf), f) != NULL)
    {
        buf[strcspn(buf, "\r\n")] = 0;
        snprintf(out->line[out->n], SET_LEN, "%s", buf);
        KeyOf(buf, out->key[out->n], sizeof(out->key[0]));
        out->n++;
    }
    fclose(f);
}

// the lines of KEY in F, joined (to compare a setting's whole value, all its lines for a repeated key)
static void Group(const struct SetFile *f, const char *key, char *out, size_t size)
{
    int i;
    out[0] = 0;
    for (i = 0; i < f->n; i++)
        if (strcmp(f->key[i], key) == 0)
        {
            const char *v = strchr(f->line[i], '=') + 1;
            while (*v == ' ' || *v == '\t')
                v++;
            if (strlen(out) + strlen(v) + 2 < size)
            {
                strcat(out, v);
                strcat(out, "\n");
            }
        }
}

static bool8 HasKey(const struct SetFile *f, const char *key)
{
    int i;
    for (i = 0; i < f->n; i++)
        if (strcmp(f->key[i], key) == 0)
            return TRUE;
    return FALSE;
}

static struct SetFile sSetA, sSetB;   // (big: kept out of the stack)
static char sGroupA[8192], sGroupB[8192];

// OWN = the settings in FULL that differ from BASE's (File 1's)
static void DiffSet(const char *full, const char *base, const char *own)
{
    FILE *f;
    int i, j;
    ReadSet(full, &sSetA);
    ReadSet(base, &sSetB);
    f = fopen(own, "w");
    if (f == NULL)
        return;
    fprintf(f, "; what this save file changed from File 1's settings (the game writes this file itself)\n");
    for (i = 0; i < sSetA.n; i++)
    {
        const char *key = sSetA.key[i];
        if (key[0] == 0)
            continue;
        for (j = 0; j < i; j++)
            if (strcmp(sSetA.key[j], key) == 0)
                break;
        if (j < i)
            continue;   // (a repeated key's group: written with its first line)
        Group(&sSetA, key, sGroupA, sizeof(sGroupA));
        Group(&sSetB, key, sGroupB, sizeof(sGroupB));
        if (HasKey(&sSetB, key) && strcmp(sGroupA, sGroupB) == 0)
            continue;
        for (j = i; j < sSetA.n; j++)
            if (strcmp(sSetA.key[j], key) == 0)
                fprintf(f, "%s\n", sSetA.line[j]);
    }
    // a repeated key File 1 has and this file emptied: one empty line says so
    for (i = 0; i < sSetB.n; i++)
    {
        const char *key = sSetB.key[i];
        int count = 0;
        if (key[0] == 0 || HasKey(&sSetA, key))
            continue;
        for (j = 0; j < sSetB.n; j++)
            count += strcmp(sSetB.key[j], key) == 0;
        for (j = 0; j < i; j++)
            if (strcmp(sSetB.key[j], key) == 0)
                break;
        if (count > 1 && j == i)
            fprintf(f, "%s =\n", key);
    }
    fclose(f);
}

// OUT = BASE (File 1's) with OWN's settings laid over it
static void MergeSet(const char *base, const char *own, const char *out)
{
    FILE *f;
    int i, j;
    ReadSet(base, &sSetA);
    ReadSet(own, &sSetB);
    f = fopen(out, "w");
    if (f == NULL)
        return;
    for (i = 0; i < sSetA.n; i++)
    {
        const char *key = sSetA.key[i];
        if (key[0] == 0 || !HasKey(&sSetB, key))
        {
            fprintf(f, "%s\n", sSetA.line[i]);
            continue;
        }
        for (j = 0; j < i; j++)
            if (strcmp(sSetA.key[j], key) == 0)
                break;
        if (j < i)
            continue;   // (the file's own lines for it went in already)
        for (j = 0; j < sSetB.n; j++)
            if (strcmp(sSetB.key[j], key) == 0)
                fprintf(f, "%s\n", sSetB.line[j]);
    }
    for (i = 0; i < sSetB.n; i++)
        if (sSetB.key[i][0] != 0 && !HasKey(&sSetA, sSetB.key[i]))
            fprintf(f, "%s\n", sSetB.line[i]);
    fclose(f);
}

// A brand-new game's settings (no File 1 settings, nothing beside the game): the user's list (2026-11-11)
static const char *const sCanonicalCoop =
    "; Moemon Emerald Kai Co-op settings (a new game's own; the game writes this file itself)\n"
    "text_speed = fast\nwindow_frame = 1\nname_case = uncorrected\ncreatures = moemon\nbutton_mode = normal\n"
    "sound = stereo\nmute_in_background = 1\nbike_music = 1\nsurf_music = 1\nbadge_fade = platinum\ncard_title = 1\n"
    "card_scaling = 1\ncard_art = printed_art\ncard_back = printed\nbattle_scene = 1\nbattle_style = shift\n"
    "battle_anims = modern\nstadium_announcer = 1\nstadium_announcer_jp = 0\ntype_icons = 0\nlevel_scaling = 1\n"
    "level_scaling_rules = 2 2 0 1 0\ngeneration = 4\nmove_split = 1\ngimmicks = 168\nfollower_sprites = pmd\n"
    "sinnoh_sprites = mixed\nunova_sprites = mixed\nscale_followers = 1\nscale_followers_percent = 80\nscale_garden = 1\n"
    "scale_garden_percent = 80\nshadows = 1\nspectate_panel = 1\nspectate_size = 2\nautosave = 5\ntrainer_range = 5\n"
    "moenav_calls = battles\nfriend_finder = both\nstreamer_mode = off\n";
static const char *const sCanonicalDisplay =
    "; Moemon Emerald Kai display settings (a new game's own)\n"
    "fullscreen = 1\nwindow_scale = 4\nscaling = sharp_fit\nfilter = off\ncolor = default\nvsync = 1\nframerate = 60\n";
static const char *const sCanonicalControls =
    "; Moemon Emerald Kai controller settings (a new game's own)\n"
    "layout = nintendo\nanalog_run = 0\nrumble = 1\ncontroller = all\nbutton_icons = auto\n";

static void Seed(const char *name, const char *path)
{
    const char *text = strcmp(name, "moemon_coop.ini") == 0     ? sCanonicalCoop
                     : strcmp(name, "moemon_display.ini") == 0  ? sCanonicalDisplay
                     : strcmp(name, "moemon_controls.ini") == 0 ? sCanonicalControls : NULL;
    FILE *f;
    if (text == NULL || (f = fopen(path, "w")) == NULL)
        return;
    fputs(text, f);
    fclose(f);
    printf("[slots] %s: a new game's settings\n", path);
}

// The save file in use's settings file NAME, as it reads (File 1's with this file's own changes over it); the game
// writes the whole of it there and calls SaveSlots_Written after. Without slots (--save / MOEMON_SAVE): SHARED itself.
const char *SaveSlots_File(const char *name, const char *shared)
{
    static char sBuf[4][600];
    static int sNext;
    char first[600], own[600], dir[64];
    char *out;
    if (sSlot == 0)
        return shared;
    SlotDir(1, dir, sizeof(dir));
    MakeDir(SAVES_DIR);
    MakeDir(dir);
    SetPath(1, name, "", first, sizeof(first));
    if (!Exists(first))
    {
        if (Exists(shared))
            CopyFileTo(shared, first);
        else
            Seed(name, first);
    }
    out = sBuf[sNext++ & 3];
    if (sSlot == 1)
    {
        snprintf(out, 600, "%s", first);
        return out;
    }
    SetPath(sSlot, name, "", out, 600);
    SetPath(sSlot, name, ".own", own, sizeof(own));
    if (!Exists(own))
    {
        if (Exists(out))
            DiffSet(out, first, own);   // (a save file from before: what it set that File 1 doesn't is kept)
        else
        {
            FILE *f = fopen(own, "w");
            if (f != NULL)
                fclose(f);
        }
    }
    if (Exists(first))
        MergeSet(first, own, out);
    else
        CopyFileTo(own, out);
    return out;
}

// The settings file NAME was written (whole) for the save file in use: another file than File 1 keeps what differs
void SaveSlots_Written(const char *name)
{
    char first[600], full[600], own[600];
    if (sSlot <= 1)
        return;
    SetPath(1, name, "", first, sizeof(first));
    SetPath(sSlot, name, "", full, sizeof(full));
    SetPath(sSlot, name, ".own", own, sizeof(own));
    DiffSet(full, first, own);
}

// The game goes on with save file SLOT in place: the save path and slot.txt (the caller reloads the save itself;
// MP_SaveSlotChanged reloads the settings and forgets the other file's side data)
void SaveSlots_Switch(int slot)
{
    char dir[64];
    FILE *f;
    void MP_SaveSlotChanged(void);
    void Platform_ReopenSaveFile(void);
    if (slot < 1 || slot > SAVE_SLOT_COUNT || sSlot == 0)
        return;
    MakeDir(SAVES_DIR);
    f = fopen(SAVES_DIR "/slot.txt", "w");
    if (f != NULL)
    {
        fprintf(f, "%d\n", slot);
        fclose(f);
    }
    sSlot = slot;
    SlotDir(slot, dir, sizeof(dir));
    MakeDir(dir);
    SlotSave(slot, gPlatformSavePath, 512);
    Platform_ReopenSaveFile();
    printf("[slots] now on save file %d: %s\n", slot, gPlatformSavePath);
    fflush(stdout);
    MP_SaveSlotChanged();
}
