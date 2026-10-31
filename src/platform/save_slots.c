// Moemon Co-op: the save files' own folder and three save files.
//
// The save and everything that belongs with it (its sidecars "<save>.life", ".cards", ".ygo"... and its backups
// "<save>.bak1"...) live in saves/slot1, saves/slot2 or saves/slot3 beside the game, one save file each, so they
// never mix. saves/slot.txt remembers the one in use. A save from before (pokeemerald.sav and its files next to the
// exe) moves into saves/slot1 the first time this version starts, every file as it was. The title screen's
// SAVE FILE item picks another one: the game starts again on it (nothing of the other file stays in memory).
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
    printf("[slots] erased save file %d (%d files)\n", slot, n);
    return n;
}

// The game starts again on save file slot (tests: says so and ends, the next run is on it)
void SaveSlots_Switch(int slot)
{
    FILE *f;
    if (slot < 1 || slot > SAVE_SLOT_COUNT || sSlot == 0)
        return;
    MakeDir(SAVES_DIR);
    f = fopen(SAVES_DIR "/slot.txt", "w");
    if (f != NULL)
    {
        fprintf(f, "%d\n", slot);
        fclose(f);
    }
    printf("[slots] starting again on save file %d\n", slot);
    fflush(stdout);
    if (getenv("MOEMON_TEST"))
        exit(0);
    {
        void Display_Shutdown(void);
        Display_Shutdown();
        SDL_Quit();
    }
#ifdef _WIN32
    {
        wchar_t exe[MAX_PATH];
        STARTUPINFOW si;
        PROCESS_INFORMATION pi;
        memset(&si, 0, sizeof(si));
        si.cb = sizeof(si);
        GetModuleFileNameW(NULL, exe, MAX_PATH);
        if (CreateProcessW(exe, GetCommandLineW(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
        {
            CloseHandle(pi.hThread);
            CloseHandle(pi.hProcess);
        }
        exit(0);
    }
#else
    if (gPlatformArgv != NULL)
    {
        execv("/proc/self/exe", gPlatformArgv);
        execv(gPlatformArgv[0], gPlatformArgv);
    }
    exit(0);
#endif
}
