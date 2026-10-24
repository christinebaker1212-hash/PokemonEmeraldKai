// Moemon Co-op: quality-of-life helpers that don't make the game easier.
//   * Hold SELECT on the field: pick which Pokémon follows you
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "mp.h"
#include "main.h"
#include "pokemon.h"
#include "sound.h"
#include "string_util.h"
#include "task.h"
#include "constants/songs.h"

void MP_AsciiToGame(u8 *dst, const char *src, int maxLen);

// ---------------------------------------------------------------------------
// Follower quick-swap

static EWRAM_DATA struct
{
    u8 taskId;
    u16 cursor;
    u8 slots[PARTY_SIZE];
    u8 count;
} sPick = {0};

static void DrawPicker(void)
{
    u16 i;
    MpUi_Begin("WHO FOLLOWS YOU?");
    for (i = 0; i < sPick.count + 2; i++)
    {
        if (i < sPick.count)
        {
            struct Pokemon *mon = &gPlayerParty[sPick.slots[i]];
            u8 name[POKEMON_NAME_LENGTH + 2], lv[24];
            char buf[24];
            GetMonData(mon, MON_DATA_NICKNAME, name);
            StringGet_Nickname(name);
            if (MP_Life_WantsOut(&mon->box))   // MOéMON GARDEN: it's asking to come out
            {
                snprintf(buf, sizeof(buf), "WANTS TO COME");
                MPNet_Log("life: picker wants out %u", (unsigned)sPick.slots[i]);
            }
            else
                snprintf(buf, sizeof(buf), "%sLv%d", MP_Follower_IsFollowing(mon) ? "FOLLOWING  " : "",
                         (int)GetMonData(mon, MON_DATA_LEVEL));
            MP_AsciiToGame(lv, buf, sizeof(lv) - 1);
            MpUi_Row(i, i == sPick.cursor, name, lv);
        }
        else if (i == sPick.count)
            MpUi_RowAscii(i, i == sPick.cursor, "NOBODY", NULL);
        else
            MpUi_RowAscii(i, i == sPick.cursor, "CANCEL", NULL);
    }
    MpUi_End();
    MpUi_HelpAscii("{A} CHOOSE   {B} CANCEL   (hold {SELECT} on the map)");
}

static void Task_Picker(u8 taskId)
{
    u16 count = sPick.count + 2;
    if (MpUi_MoveCursor(&sPick.cursor, count))
    {
        DrawPicker();
        return;
    }
    if (JOY_NEW(B_BUTTON) || (JOY_NEW(A_BUTTON) && sPick.cursor == count - 1))
    {
        PlaySE(SE_SELECT);
        MpUi_Close();
        DestroyTask(taskId);
        return;
    }
    if (JOY_NEW(A_BUTTON))
    {
        u8 i;
        if (sPick.cursor < sPick.count)
        {
            struct Pokemon *mon = &gPlayerParty[sPick.slots[sPick.cursor]];
            if (!MP_Follower_IsFollowing(mon))
                MP_Follower_Toggle(mon);
            PlayCry_Normal(GetMonData(mon, MON_DATA_SPECIES), 0);
        }
        else
        {
            // Nobody: put whoever follows back in their Ball
            for (i = 0; i < PARTY_SIZE; i++)
                if (MP_Follower_IsFollowing(&gPlayerParty[i]))
                    MP_Follower_Toggle(&gPlayerParty[i]);
            PlaySE(SE_BALL);
        }
        MpUi_Close();
        DestroyTask(taskId);
    }
}

// Field: SELECT held down
bool8 MP_Qol_OpenFollowerPicker(void)
{
    u8 i;
    if (FuncIsActiveTask(Task_Picker) || MpUi_IsOpen())
        return FALSE;
    memset(&sPick, 0, sizeof(sPick));
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gPlayerParty[i];
        if (GetMonData(mon, MON_DATA_SPECIES) != SPECIES_NONE && MP_Follower_HasGraphics(mon))
        {
            if (MP_Follower_IsFollowing(mon))
                sPick.cursor = sPick.count;
            sPick.slots[sPick.count++] = i;
        }
    }
    PlaySE(SE_WIN_OPEN);
    MpUi_Open();
    sPick.taskId = CreateTask(Task_Picker, 80);
    DrawPicker();
    return TRUE;
}

// ---------------------------------------------------------------------------
// Text you've seen before, and scripts you've seen before
//
// Signs and lines you've already read print instantly. Holding START during a
// script you've already seen through (a cutscene, a conversation) plays it at
// 4x and moves its text along; menus and YES/NO questions still wait for you.
// Kept in "<save file>.seen".

#include "script.h"
#include "overworld.h"

extern char gPlatformSavePath[];
extern const u8 EventScript_PC[];

#define SEEN_SLOTS 16384   // power of two

static u32 *sSeen;         // open addressing, 0 = empty
static bool8 sSeenLoaded;
static char sSeenFor[512];
static const u8 *sRootScript;
static bool8 sRootSeen;
static bool8 sSignActive;

static u32 Fnv1a(const u8 *p, u32 n, u32 h)
{
    u32 i;
    for (i = 0; i < n; i++)
    {
        h ^= p[i];
        h *= 16777619u;
    }
    return h ? h : 1;
}

static void SeenPath(char *out, size_t size)
{
    snprintf(out, size, "%s.seen", gPlatformSavePath);
}

static bool8 SeenInsert(u32 h)
{
    u32 i = h & (SEEN_SLOTS - 1), n;
    for (n = 0; n < SEEN_SLOTS; n++, i = (i + 1) & (SEEN_SLOTS - 1))
    {
        if (sSeen[i] == h)
            return FALSE;
        if (sSeen[i] == 0)
        {
            sSeen[i] = h;
            return TRUE;
        }
    }
    return FALSE;
}

static bool8 SeenHas(u32 h)
{
    u32 i = h & (SEEN_SLOTS - 1), n;
    for (n = 0; n < SEEN_SLOTS; n++, i = (i + 1) & (SEEN_SLOTS - 1))
    {
        if (sSeen[i] == h)
            return TRUE;
        if (sSeen[i] == 0)
            return FALSE;
    }
    return FALSE;
}

static void SeenLoad(void)
{
    char path[540];
    FILE *f;
    if (sSeenLoaded && strcmp(sSeenFor, gPlatformSavePath) == 0)
        return;
    if (!sSeen)
        sSeen = calloc(SEEN_SLOTS, sizeof(u32));
    else
        memset(sSeen, 0, SEEN_SLOTS * sizeof(u32));
    snprintf(sSeenFor, sizeof(sSeenFor), "%s", gPlatformSavePath);
    sSeenLoaded = TRUE;
    SeenPath(path, sizeof(path));
    f = fopen(path, "rb");
    if (f)
    {
        u32 h;
        while (fread(&h, sizeof(h), 1, f) == 1)
            SeenInsert(h);
        fclose(f);
    }
}

static void SeenAdd(u32 h)
{
    char path[540];
    FILE *f;
    SeenLoad();
    if (!SeenInsert(h))
        return;
    SeenPath(path, sizeof(path));
    f = fopen(path, "ab");
    if (f)
    {
        fwrite(&h, sizeof(h), 1, f);
        fclose(f);
    }
}

static u32 ScriptHash(const u8 *script)
{
    // Where the script sits relative to a fixed one (same for every run of a build)
    long off = (long)(script - EventScript_PC);
    return Fnv1a((const u8 *)&off, sizeof(off), 0x5C819u);
}

// ScriptContext_SetupScript
void MP_Qol_OnScriptStart(const u8 *script)
{
    SeenLoad();
    sRootScript = script;
    sRootSeen = SeenHas(ScriptHash(script));
}

// A sign (background event) was read
void MP_Qol_SignStarted(void)
{
    sSignActive = TRUE;
}

// Every frame
void MP_Qol_Update(void)
{
    if (sRootScript && !ScriptContext_IsEnabled() && MP_IsInOverworld())
    {
        SeenAdd(ScriptHash(sRootScript));
        sRootScript = NULL;
        sRootSeen = FALSE;
    }
    if (sSignActive && !ScriptContext_IsEnabled() && MP_IsInOverworld())
    {
        sSignActive = FALSE;
        MP_Social_AfterSign();   // a friend's scribble on that sign
    }
}

// Hold START in a script you've seen before: faster, and text moves along
bool8 MP_Qol_SkipActive(void)
{
    return sRootScript != NULL && sRootSeen && ScriptContext_IsEnabled() && MP_IsInOverworld()
        && (gMain.heldKeys & START_BUTTON) && !gMain.inBattle;
}

// A field message is about to print gStringVar4: instant when it's a sign
// or a line read before
u8 MP_Qol_FieldTextSpeed(u8 normalSpeed)
{
    u32 h;
    SeenLoad();
    h = Fnv1a(gStringVar4, StringLength(gStringVar4), 0x811C9DC5u);
    if (sSignActive || SeenHas(h) || MP_Qol_SkipActive())
    {
        SeenAdd(h);
        return 0;
    }
    SeenAdd(h);
    return normalSpeed;
}
