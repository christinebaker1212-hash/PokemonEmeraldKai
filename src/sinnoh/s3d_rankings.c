// Moemon Kai: JUBILIFE TV's rankings machines (Platinum's rankings.c and rankings_machine.c): the 13 records in three
// categories (the BATTLE TOWER, POKéMON, CONTESTS), each machine showing one category's records, the player's own value
// among the six best others it has heard of, by Platinum's rules: the GLOBAL room's machines (0-2) everyone's, the GROUP
// room's (3-5) those of the player's group only. Platinum's records come from record mixing; here they come with CO-OP's
// record mixing (mp_linkx.c: each friend's 13 entries after the mix record), the group being the CO-OP room (its name
// hashed: Platinum's group seed). The player's own values are the game's: the BATTLE TOWER's records (s3d_tower.c), the
// game's stats (catches, eggs, fishing, contests, ribbons) and the POKéMON defeated (counted here: Platinum's
// RECORD_FAINTED_IN_BATTLE has no Emerald stat). Kept in "<save>.region" chunk RANK. The screen: Platinum's, on the
// game's windows (a list of the category's records and EXIT, a record's ranking with DEL RECORD on SELECT).
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "bg.h"
#include "event_data.h"
#include "overworld.h"
#include "list_menu.h"
#include "main.h"
#include "menu.h"
#include "mp.h"
#include "palette.h"
#include "script.h"
#include "sound.h"
#include "string_util.h"
#include "strings.h"
#include "task.h"
#include "text.h"
#include "text_window.h"
#include "window.h"
#include "s3d.h"
#include "constants/characters.h"
#include "constants/game_stat.h"
#include "constants/songs.h"

#define RANK_STATS 13
#define STD_WINDOW_BASE_TILE_NUM 0x214   // (menu.c's: where LoadMessageBoxAndBorderGfx puts the windows' frame)
#define STD_WINDOW_PALETTE_NUM 14
#define RANK_ENTRIES 6
#define RANK_NAME (PLAYER_NAME_LENGTH + 1)

enum
{
    RS_TOWER_SINGLE, RS_TOWER_DOUBLE, RS_TOWER_MULTI, RS_TOWER_LINK_MULTI, RS_TOWER_WIFI, RS_TOWER_AVG,
    RS_DEFEATED, RS_CAUGHT, RS_HATCHED, RS_FISHED,
    RS_CONTEST_WINS, RS_CONTEST_PERCENT, RS_RIBBONS,
};

struct RankEntry
{
    u32 seed;     // the group (0: none)
    u32 value;
    u8 name[RANK_NAME];
};

// what's sent with a mix record: the player's entry for each record
struct RankEntries
{
    struct RankEntry e[RANK_STATS];
};

static struct
{
    struct RankEntry lists[RANK_STATS * 2][RANK_ENTRIES];   // GLOBAL, then GROUP (Platinum's Rankings)
    u32 defeated;                                           // POKéMON defeated in battle
} sRank;

void Sinnoh_RankingsReset(void)
{
    memset(&sRank, 0, sizeof(sRank));
}

const void *Sinnoh_RankingsData(u32 *size)
{
    *size = sizeof(sRank);
    return &sRank;
}

void Sinnoh_RankingsLoad(const void *data, u32 size)
{
    memset(&sRank, 0, sizeof(sRank));
    memcpy(&sRank, data, size < sizeof(sRank) ? size : sizeof(sRank));
}

// (battle_script_commands.c: an opposing POKéMON fainted)
void Sinnoh_RankingsDefeated(void)
{
    if (sRank.defeated < 999999)
        sRank.defeated++;
}

// the player's group: the CO-OP room (Platinum's record mixing group seed)
static u32 GroupSeed(void)
{
    const char *room = MP_IsConnected() ? MP_Lobby_RoomName() : NULL;
    u32 h = 0x811C9DC5;
    if (room == NULL || room[0] == 0)
        return 0;
    for (; *room; room++)
        h = (h ^ (u8)*room) * 0x01000193;
    return h ? h : 1;
}

u16 Sinnoh_Tower_RecordStreak(u8 mode);
u32 Sinnoh_Tower_Victories(void);
u32 Sinnoh_Tower_Challenges(void);

// Platinum's GetRecordValues, on the game's records
static u32 OwnValue(u8 stat)
{
    u32 n;
    switch (stat)
    {
    case RS_TOWER_SINGLE: case RS_TOWER_DOUBLE: case RS_TOWER_MULTI: case RS_TOWER_LINK_MULTI:
        return Sinnoh_Tower_RecordStreak(stat - RS_TOWER_SINGLE);
    case RS_TOWER_WIFI:
        return 0;   // (the Wi-Fi rooms: online, closed)
    case RS_TOWER_AVG:
        n = Sinnoh_Tower_Challenges();
        return n ? Sinnoh_Tower_Victories() / n : 0;
    case RS_DEFEATED:
        return sRank.defeated;
    case RS_CAUGHT:
        return GetGameStat(GAME_STAT_POKEMON_CAPTURES);
    case RS_HATCHED:
        return GetGameStat(GAME_STAT_HATCHED_EGGS);
    case RS_FISHED:
        return GetGameStat(GAME_STAT_FISHING_ENCOUNTERS);
    case RS_CONTEST_WINS:
        return GetGameStat(GAME_STAT_WON_CONTEST) + GetGameStat(GAME_STAT_WON_LINK_CONTEST);
    case RS_CONTEST_PERCENT:
        n = GetGameStat(GAME_STAT_ENTERED_CONTEST);
        return n ? OwnValue(RS_CONTEST_WINS) * 100 / n : 0;
    case RS_RIBBONS:
        return GetGameStat(GAME_STAT_RECEIVED_RIBBONS);
    }
    return 0;
}

static void OwnEntry(struct RankEntry *e, u8 stat)
{
    memset(e, 0, sizeof(*e));
    e->seed = GroupSeed();
    e->value = OwnValue(stat);
    StringCopyN(e->name, gSaveBlock2Ptr->playerName, PLAYER_NAME_LENGTH);
    e->name[PLAYER_NAME_LENGTH] = EOS;
}

u32 Sinnoh_RankingsEntriesSize(void)
{
    return sizeof(struct RankEntries);
}

// (mp_linkx.c) the player's entries for a mix record
void Sinnoh_RankingsPrepare(void *dest)
{
    struct RankEntries *out = dest;
    u32 i;
    for (i = 0; i < RANK_STATS; i++)
        OwnEntry(&out->e[i], i);
}

static bool8 HasName(const struct RankEntry *e)
{
    return e->name[0] != EOS && e->name[0] != 0;
}

static bool8 Same(const struct RankEntry *a, const struct RankEntry *b)
{
    return a->seed == b->seed && StringCompareN(a->name, b->name, PLAYER_NAME_LENGTH) == 0;
}

// Platinum's SortRankings: the friends' new entries first, then the kept ones not among them, by value (best first, the
// newer ahead of an equal kept one); the six best kept. GROUP: only entries of the player's group
static void Sort(u8 list, u32 seed, const struct RankEntries **mixed, u8 count, bool8 group)
{
    struct RankEntry kept[RANK_ENTRIES], *sorted[RANK_ENTRIES * 2], fresh[RANK_ENTRIES];
    u32 i, j, n = 0, m = 0;
    u8 stat = list % RANK_STATS;
    memcpy(kept, sRank.lists[list], sizeof(kept));
    for (i = 0; i < count && m < RANK_ENTRIES; i++)
    {
        if (group && (mixed[i]->e[stat].seed == 0 || mixed[i]->e[stat].seed != seed))
            continue;
        fresh[m] = mixed[i]->e[stat];
        sorted[n++] = &fresh[m++];
    }
    for (i = 0; i < RANK_ENTRIES; i++)
    {
        if (!HasName(&kept[i]) || (group && kept[i].seed != seed))
            continue;
        for (j = 0; j < m && !Same(&fresh[j], &kept[i]); j++)
            ;
        if (j == m)
            sorted[n++] = &kept[i];
    }
    for (i = 0; i + 1 < n; i++)
        for (j = n - 1; j > i; j--)
            if (sorted[i]->value < sorted[j]->value)
            {
                struct RankEntry *t = sorted[i];
                sorted[i] = sorted[j];
                sorted[j] = t;
            }
    {
        struct RankEntry out[RANK_ENTRIES];
        memset(out, 0, sizeof(out));
        for (i = 0; i < n && i < RANK_ENTRIES; i++)
            out[i] = *sorted[i];
        memcpy(sRank.lists[list], out, sizeof(out));
    }
}

// (mp_linkx.c) the friends' entries after a CO-OP record mix: COUNT players' entries, SIZE apart, ours at MINE
// (Platinum's SaveData_UpdateRankings)
void Sinnoh_RankingsMix(const u8 *entries, u8 count, u8 mine, u32 size)
{
    const struct RankEntries *mixed[8];
    u32 seed = GroupSeed(), i;
    u8 n = 0;
    for (i = 0; i < count && n < ARRAY_COUNT(mixed); i++)
    {
        const struct RankEntries *r = (const void *)(entries + i * size);
        if (i == mine || !HasName(&r->e[0]))
            continue;
        mixed[n++] = r;
    }
    if (n == 0)
        return;
    for (i = 0; i < RANK_STATS; i++)
    {
        Sort(i, seed, mixed, n, FALSE);
        if (seed != 0)
            Sort(i + RANK_STATS, seed, mixed, n, TRUE);
    }
    MPNet_Log("rankings: %d friends' records mixed in", n);
}

// ----------------------------------------------------------------------------------------------------- the machine

static const u8 sCategoryFirst[3] = {RS_TOWER_SINGLE, RS_DEFEATED, RS_CONTEST_WINS};
static const u8 sCategoryCount[3] = {6, 4, 3};
// Platinum's RecordPrintingData: the most shown, its digits, its value line (RankingsMachine_Text_*Value)
static const struct { u32 max; u8 digits; const char *line; } sPrint[RANK_STATS] = {
    {9999, 4, "RankingsMachine_Text_BattleWinsValue"}, {9999, 4, "RankingsMachine_Text_BattleWinsValue"},
    {9999, 4, "RankingsMachine_Text_BattleWinsValue"}, {9999, 4, "RankingsMachine_Text_BattleWinsValue"},
    {9999, 4, "RankingsMachine_Text_BattleWinsValue"}, {7, 1, "RankingsMachine_Text_AvgWinStreakValue"},
    {999999, 6, "RankingsMachine_Text_PokemonCaughtValue"}, {999999, 6, "RankingsMachine_Text_PokemonCaughtValue"},
    {999999, 6, "RankingsMachine_Text_PokemonHatchedValue"}, {999999, 6, "RankingsMachine_Text_PokemonCaughtValue"},
    {9999, 4, "RankingsMachine_Text_ContestWinsValue"}, {100, 4, "RankingsMachine_Text_ContestWinPercentValue"},
    {65535, 5, "RankingsMachine_Text_RibbonsValue"},
};
static const char *const sTitles[RANK_STATS] = {
    "RankingsMachine_Text_SingleBattlesRecordTitle", "RankingsMachine_Text_DoubleBattlesRecordTitle",
    "RankingsMachine_Text_MultiBattlesRecordTitle", "RankingsMachine_Text_LinkMultiBattlesRecordTitle",
    "RankingsMachine_Text_WiFiBattleRoomRecordTitle", "RankingsMachine_Text_AvgWinStreakRecordTitle",
    "RankingsMachine_Text_PokemonDefeatedRecordTitle", "RankingsMachine_Text_PokemonCaughtRecordTitle",
    "RankingsMachine_Text_PokemonEggsRecordTitle", "RankingsMachine_Text_EncounteredWhileFishingRecordTitle",
    "RankingsMachine_Text_ContestWinsRecordTitle", "RankingsMachine_Text_ContestWinPercentRecordTitle",
    "RankingsMachine_Text_RibbonsRecordTitle",
};
static const char *const sExplanations[RANK_STATS] = {
    "RankingsMachine_Text_SingleBattlesRecordExplanation", "RankingsMachine_Text_DoubleBattlesRecordExplanation",
    "RankingsMachine_Text_MultiBattlesRecordExplanation", "RankingsMachine_Text_LinkMultiBattlesRecordExplanation",
    "RankingsMachine_Text_WiFiBattlesRecordExplanation", "RankingsMachine_Text_AvgWinStreakRecordExplanation",
    "RankingsMachine_Text_PokemonDefeatedRecordExplanation", "RankingsMachine_Text_PokemonCaughtRecordExplanation",
    "RankingsMachine_Text_PokemonEggsRecordExplanation", "RankingsMachine_Text_EncounteredWhileFishingRecordExplanation",
    "RankingsMachine_Text_ContestWinsRecordExplanation", "RankingsMachine_Text_ContestWinPercentRecordExplanation",
    "RankingsMachine_Text_RibbonsRecordExplanation",
};

#define ROW_OWN 0xFF
enum { ST_LIST, ST_RANKING, ST_DELETE, ST_ASK, ST_CANT };

static struct
{
    u8 category, mode, record, state;
    u8 listWin, msgWin, delWin, listTask;
    u8 rows, cursor;
    u8 rowEntry[RANK_ENTRIES + 1];   // each shown row's entry in the saved list (ROW_OWN: the player's own)
    u16 scroll, selected;
    struct ListMenuItem items[7];
    u8 msg[200];
} sM;

static const u8 *T(const char *name)
{
    const u8 *t = S3D_TextByName(name);
    return t ? t : gText_EmptyString2;
}

static void Say(const u8 *text)
{
    FillWindowPixelBuffer(sM.msgWin, PIXEL_FILL(1));
    StringExpandPlaceholders(sM.msg, text);
    AddTextPrinterParameterized(sM.msgWin, FONT_NORMAL, sM.msg, 0, 1, TEXT_SKIP_DRAW, NULL);
    CopyWindowToVram(sM.msgWin, COPYWIN_FULL);
}

// (the windows' tiles below the dialogue frame's, 0x200; the YES / NO box after them)
static const struct WindowTemplate sListTemplate = {0, 1, 1, 17, 12, 15, 0x60};
static const struct WindowTemplate sMsgTemplate = {0, 2, 15, 27, 4, 15, 0x150};
static const struct WindowTemplate sDelTemplate = {0, 20, 1, 9, 2, 15, 0x1C0};
static const struct WindowTemplate sYesNoTemplate = {0, 23, 9, 5, 4, 15, 0x1D0};

static void ShowList(void)
{
    struct ListMenuTemplate t;
    u8 i, n = sCategoryCount[sM.category];
    for (i = 0; i < n; i++)
    {
        sM.items[i].name = T(sTitles[sCategoryFirst[sM.category] + i]);
        sM.items[i].id = i;
    }
    sM.items[n].name = T("RankingsMachine_Text_Exit");
    sM.items[n].id = LIST_CANCEL;
    memset(&t, 0, sizeof(t));
    t.items = sM.items;
    t.totalItems = n + 1;
    t.maxShowed = 6;
    t.windowId = sM.listWin;
    t.item_X = 8;
    t.cursor_X = 0;
    t.upText_Y = 1;
    t.cursorPal = 2;
    t.fillValue = 1;
    t.cursorShadowPal = 3;
    t.fontId = FONT_NORMAL;
    t.cursorKind = CURSOR_BLACK_ARROW;
    FillWindowPixelBuffer(sM.listWin, PIXEL_FILL(1));
    DrawStdWindowFrame(sM.listWin, FALSE);
    sM.listTask = ListMenuInit(&t, sM.scroll, sM.selected);
    CopyWindowToVram(sM.listWin, COPYWIN_FULL);
    Say(T("RankingsMachine_Text_WhichRecordWouldYouLikeToSee"));
    ClearStdWindowAndFrame(sM.delWin, TRUE);
    sM.state = ST_LIST;
}

// a record's ranking: the saved entries with the player's own among them (Platinum's RankingsMachine_NewRankings), rank
// numbers shared by equal values, the value capped at the most shown
static void DrawRanking(void)
{
    u8 stat = sCategoryFirst[sM.category] + sM.record, list = sM.mode * RANK_STATS + stat;
    struct RankEntry own, *e;
    u32 prev = 0, i, k = 0, rank = 1;
    bool8 ownShown = FALSE;
    OwnEntry(&own, stat);
    FillWindowPixelBuffer(sM.listWin, PIXEL_FILL(1));
    sM.rows = 0;
    for (i = 0; i <= RANK_ENTRIES && sM.rows < RANK_ENTRIES; i++)
    {
        u8 which;
        if (!ownShown && (i == RANK_ENTRIES || !HasName(&sRank.lists[list][i]) || own.value >= sRank.lists[list][i].value))
        {
            e = &own, which = ROW_OWN, ownShown = TRUE, i--;
        }
        else if (i < RANK_ENTRIES && HasName(&sRank.lists[list][i]))
            e = &sRank.lists[list][i], which = i;
        else
            continue;
        {
            u32 v = e->value > sPrint[stat].max ? sPrint[stat].max : e->value;
            u8 line[64], name[RANK_NAME];
            if (v < prev)
                rank++;
            prev = v;
            ConvertIntToDecimalStringN(gStringVar1, rank, STR_CONV_MODE_LEFT_ALIGN, 1);
            memcpy(name, e->name, RANK_NAME);
            name[PLAYER_NAME_LENGTH] = EOS;
            StringCopy(gStringVar2, name);
            StringExpandPlaceholders(line, T("RankingsMachine_Text_RecordEntryInfo"));
            AddTextPrinterParameterized(sM.listWin, FONT_NORMAL, line, 8, 1 + 16 * sM.rows, TEXT_SKIP_DRAW, NULL);
            ConvertIntToDecimalStringN(gStringVar1, v, STR_CONV_MODE_LEFT_ALIGN, sPrint[stat].digits);
            StringExpandPlaceholders(line, T(sPrint[stat].line));
            AddTextPrinterParameterized(sM.listWin, FONT_NORMAL, line, 17 * 8 - 4 - GetStringWidth(FONT_NORMAL, line, 0),
                                        1 + 16 * sM.rows, TEXT_SKIP_DRAW, NULL);
            {
                char ascii[16];
                MP_GameToAscii(ascii, name, PLAYER_NAME_LENGTH);
                MPNet_Log("rankings: %u. %s %u", rank, ascii, v);
            }
            sM.rowEntry[sM.rows++] = which;
            k++;
        }
    }
    if (sM.state == ST_DELETE)
        AddTextPrinterParameterized(sM.listWin, FONT_NORMAL, gText_SelectorArrow2, 0, 1 + 16 * sM.cursor, TEXT_SKIP_DRAW,
                                    NULL);
    CopyWindowToVram(sM.listWin, COPYWIN_FULL);
}

static void ShowRanking(void)
{
    u8 stat = sCategoryFirst[sM.category] + sM.record;
    static const u8 sDel[] = _("{SELECT_BUTTON}");
    sM.state = ST_RANKING;
    DrawStdWindowFrame(sM.listWin, FALSE);
    DrawRanking();
    Say(T(sExplanations[stat]));
    DrawStdWindowFrame(sM.delWin, FALSE);   // (it clears the window: before the label)
    StringCopy(sM.msg, sDel);
    StringAppend(sM.msg, T("RankingsMachine_Text_DeleteRecord"));
    AddTextPrinterParameterized(sM.delWin, FONT_SMALL_NARROW, sM.msg, 0, 1, TEXT_SKIP_DRAW, NULL);
    CopyWindowToVram(sM.delWin, COPYWIN_FULL);
}

static void Close(u8 taskId)
{
    ClearStdWindowAndFrameToTransparent(sM.listWin, FALSE);
    ClearStdWindowAndFrameToTransparent(sM.delWin, FALSE);
    ClearDialogWindowAndFrameToTransparent(sM.msgWin, FALSE);
    RemoveWindow(sM.listWin);
    RemoveWindow(sM.delWin);
    RemoveWindow(sM.msgWin);
    ScheduleBgCopyTilemapToVram(0);
    DestroyTask(taskId);
    ScriptContext_Enable();
}

static void Task_RankingsMachine(u8 taskId)
{
    switch (sM.state)
    {
    case ST_LIST:
    {
        s32 input = ListMenu_ProcessInput(sM.listTask);
        if (JOY_NEW(B_BUTTON) || (JOY_NEW(A_BUTTON) && input == LIST_CANCEL))
        {
            PlaySE(SE_SELECT);
            DestroyListMenuTask(sM.listTask, &sM.scroll, &sM.selected);
            Close(taskId);
            return;
        }
        if (JOY_NEW(A_BUTTON) && input >= 0)
        {
            PlaySE(SE_SELECT);
            DestroyListMenuTask(sM.listTask, &sM.scroll, &sM.selected);
            sM.record = input;
            sM.cursor = 0;
            ShowRanking();
        }
        break;
    }
    case ST_RANKING:
        if (JOY_NEW(A_BUTTON | B_BUTTON))
        {
            PlaySE(SE_SELECT);
            ShowList();
        }
        else if (JOY_NEW(SELECT_BUTTON))
        {
            PlaySE(SE_SELECT);
            sM.state = ST_DELETE;
            DrawRanking();
            Say(T("RankingsMachine_Text_SelectTheRecordYouWantToDeleteThenPressA"));
        }
        break;
    case ST_DELETE:
        if (JOY_NEW(B_BUTTON))
        {
            PlaySE(SE_SELECT);
            sM.state = ST_RANKING;
            DrawRanking();
            Say(T(sExplanations[sCategoryFirst[sM.category] + sM.record]));
        }
        else if (JOY_NEW(A_BUTTON))
        {
            PlaySE(SE_SELECT);
            if (sM.rowEntry[sM.cursor] == ROW_OWN)
            {
                sM.state = ST_CANT;
                Say(T("RankingsMachine_Text_YourOwnRecordCantBeDeleted"));
            }
            else
            {
                sM.state = ST_ASK;
                Say(T("RankingsMachine_Text_AskDeleteThisRecord"));
                CreateYesNoMenu(&sYesNoTemplate, STD_WINDOW_BASE_TILE_NUM, STD_WINDOW_PALETTE_NUM, 0);
            }
        }
        else if (JOY_NEW(DPAD_UP) && sM.cursor > 0)
        {
            PlaySE(SE_SELECT);
            sM.cursor--;
            DrawRanking();
        }
        else if (JOY_NEW(DPAD_DOWN) && sM.cursor + 1 < sM.rows)
        {
            PlaySE(SE_SELECT);
            sM.cursor++;
            DrawRanking();
        }
        break;
    case ST_ASK:
    {
        s8 answer = Menu_ProcessInputNoWrapClearOnChoose();
        if (answer == MENU_NOTHING_CHOSEN)
            break;
        if (answer == 0)
        {
            // (Platinum's Rankings_RemoveEntry: the ones below move up)
            u8 list = sM.mode * RANK_STATS + sCategoryFirst[sM.category] + sM.record, at = sM.rowEntry[sM.cursor], i;
            MPNet_Log("rankings: record %d entry %d deleted", list, at);
            for (i = at; i + 1 < RANK_ENTRIES; i++)
                sRank.lists[list][i] = sRank.lists[list][i + 1];
            memset(&sRank.lists[list][RANK_ENTRIES - 1], 0, sizeof(struct RankEntry));
        }
        sM.state = ST_DELETE;
        DrawRanking();
        if (sM.cursor >= sM.rows && sM.cursor > 0)
        {
            sM.cursor--;
            DrawRanking();
        }
        Say(T("RankingsMachine_Text_SelectTheRecordYouWantToDeleteThenPressA"));
        break;
    }
    case ST_CANT:
        if (JOY_NEW(A_BUTTON | B_BUTTON))
        {
            sM.state = ST_DELETE;
            Say(T("RankingsMachine_Text_SelectTheRecordYouWantToDeleteThenPressA"));
        }
        break;
    }
}

// Platinum's StartRankingsMachine MACHINE: 0-2 the GLOBAL room's (BATTLE TOWER, POKéMON, CONTESTS), 3-5 the GROUP room's.
// The script waits until it closes
void Sinnoh_RankingsStart(u16 machine)
{
    memset(&sM, 0, sizeof(sM));
    sM.category = machine % 3;
    sM.mode = (machine / 3) & 1;
    LoadMessageBoxAndBorderGfx();
    sM.listWin = AddWindow(&sListTemplate);
    sM.msgWin = AddWindow(&sMsgTemplate);
    sM.delWin = AddWindow(&sDelTemplate);
    DrawDialogueFrame(sM.msgWin, FALSE);
    ShowList();
    CreateTask(Task_RankingsMachine, 80);
    MPNet_Log("rankings: machine %d (category %d, %s)", machine, sM.category, sM.mode ? "group" : "global");
}

// (tests: `mp rankings [add STAT VALUE NAME [GROUP]]`)
void Sinnoh_RankingsDebug(const char *arg)
{
    int stat, group = 0;
    unsigned value;
    char name[16];
    if (sscanf(arg, "add %d %u %15s %d", &stat, &value, name, &group) >= 3 && stat >= 0 && stat < RANK_STATS)
    {
        struct RankEntries r;
        const struct RankEntries *one = &r;
        u32 i;
        memset(&r, 0, sizeof(r));
        for (i = 0; i < RANK_STATS; i++)
        {
            r.e[i].seed = group ? GroupSeed() : 0x1234;
            r.e[i].value = i == (u32)stat ? value : 0;
            MP_AsciiToGame(r.e[i].name, name, PLAYER_NAME_LENGTH);
            r.e[i].name[PLAYER_NAME_LENGTH] = EOS;
        }
        for (i = 0; i < RANK_STATS; i++)
            if (i == (u32)stat)
                Sort(i, GroupSeed(), &one, 1, FALSE);
    }
    for (stat = 0; stat < RANK_STATS * 2; stat++)
        if (HasName(&sRank.lists[stat][0]))
        {
            char ascii[16];
            MP_GameToAscii(ascii, sRank.lists[stat][0].name, PLAYER_NAME_LENGTH);
            printf("[test] rankings: list %d %s %u\n", stat, ascii, sRank.lists[stat][0].value);
        }
    printf("[test] rankings: defeated %u, own", sRank.defeated);
    for (stat = 0; stat < RANK_STATS; stat++)
        printf(" %u", OwnValue(stat));
    printf("\n");
}
