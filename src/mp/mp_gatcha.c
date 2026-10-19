// Moemon Co-op: the MoeGatcha. Every shared achievement (and a few other
// rewards) gives each player a pull. A pull gives one of:
//   * a Moemon with the moves of a real event Pokemon (WISHMKR Jirachi,
//     AURA Mew, AGETO Celebi, SPACE C Deoxys, the Pokemon Box eggs...)
//   * a useful item (Rare Candy, PP Up, a Master Ball now and then...)
//   * money (more with more players online)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "mp.h"
#include "item.h"
#include "main.h"
#include "money.h"
#include "pokedex.h"
#include "pokemon.h"
#include "random.h"
#include "sound.h"
#include "string_util.h"
#include "task.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/songs.h"
#include "constants/species.h"
#include "constants/region_map_sections.h"

void MP_AsciiToGame(u8 *dst, const char *src, int maxLen);
void MP_GameToAscii(char *dst, const u8 *src, int maxLen);
u8 MP_GiveMonKeepOT(struct Pokemon *mon);

#define PULLS_KEY "gatcha:pulls"

struct EventMon
{
    const char *title;     // shown when you get it
    u16 species;
    u8 level;
    u16 moves[MAX_MON_MOVES];
    const char *otName;    // NULL = you (hatched from an egg)
    u32 otId;
    bool8 fateful;         // Mew/Deoxys obey only when met in a fateful encounter
};

// Movesets of real Generation 3 event Pokemon
static const struct EventMon sEventMons[] = {
    {"WISHMKR JIRACHI", SPECIES_JIRACHI, 5, {MOVE_WISH, MOVE_CONFUSION, MOVE_REST, MOVE_NONE}, "WISHMKR", 20043, TRUE},
    {"AURA MEW", SPECIES_MEW, 10, {MOVE_POUND, MOVE_NONE, MOVE_NONE, MOVE_NONE}, "AURA", 20078, TRUE},
    {"AGETO CELEBI", SPECIES_CELEBI, 10, {MOVE_CONFUSION, MOVE_RECOVER, MOVE_HEAL_BELL, MOVE_SAFEGUARD}, "AGETO", 31121, TRUE},
    {"SPACE C DEOXYS", SPECIES_DEOXYS, 30, {MOVE_TAUNT, MOVE_PURSUIT, MOVE_PSYCHIC, MOVE_SUPERPOWER}, "SPACE C", 0, TRUE},
    {"POKeMON BOX ZIGZAGOON", SPECIES_ZIGZAGOON, 5, {MOVE_TACKLE, MOVE_GROWL, MOVE_TAIL_WHIP, MOVE_EXTREME_SPEED}, NULL, 0, FALSE},
    {"POKeMON BOX PICHU", SPECIES_PICHU, 5, {MOVE_THUNDER_SHOCK, MOVE_CHARM, MOVE_SURF, MOVE_NONE}, NULL, 0, FALSE},
    {"POKeMON BOX SKITTY", SPECIES_SKITTY, 5, {MOVE_GROWL, MOVE_TACKLE, MOVE_TAIL_WHIP, MOVE_PAY_DAY}, NULL, 0, FALSE},
    {"POKeMON BOX SWABLU", SPECIES_SWABLU, 5, {MOVE_PECK, MOVE_GROWL, MOVE_FALSE_SWIPE, MOVE_NONE}, NULL, 0, FALSE},
};

struct ItemPrize
{
    u16 item;
    u8 qty;
    u8 weight;
};

static const struct ItemPrize sItemPrizes[] = {
    {ITEM_RARE_CANDY, 1, 14},
    {ITEM_RARE_CANDY, 3, 4},
    {ITEM_PP_UP, 1, 8},
    {ITEM_PP_MAX, 1, 2},
    {ITEM_ULTRA_BALL, 5, 10},
    {ITEM_MASTER_BALL, 1, 1},
    {ITEM_MAX_REVIVE, 1, 6},
    {ITEM_FULL_RESTORE, 2, 6},
    {ITEM_NUGGET, 2, 6},
    {ITEM_PROTEIN, 1, 4},
    {ITEM_IRON, 1, 4},
    {ITEM_CALCIUM, 1, 4},
    {ITEM_ZINC, 1, 4},
    {ITEM_CARBOS, 1, 4},
    {ITEM_HP_UP, 1, 4},
    {ITEM_LEFTOVERS, 1, 2},
    {ITEM_LUCKY_EGG, 1, 2},
    {ITEM_KINGS_ROCK, 1, 2},
    {ITEM_FIRE_STONE, 1, 2},
    {ITEM_WATER_STONE, 1, 2},
    {ITEM_THUNDER_STONE, 1, 2},
    {ITEM_LEAF_STONE, 1, 2},
    {ITEM_MOON_STONE, 1, 2},
};

s32 MP_Gatcha_Pulls(void)
{
    return MP_SaveData_Get(PULLS_KEY);
}

void MP_Gatcha_AddPulls(s32 n)
{
    MP_SaveData_Add(PULLS_KEY, n);
}

// Gives a random prize and writes what it was (ASCII, may have a '\n')
static void DrawPrize(char *out, int outLen)
{
    u16 roll = Random() % 100;
    if (roll < 10)
    {
        const struct EventMon *e = &sEventMons[Random() % ARRAY_COUNT(sEventMons)];
        struct Pokemon mon;
        u8 i, result;
        CreateMon(&mon, e->species, e->level, USE_RANDOM_IVS, FALSE, 0, e->otName ? OT_ID_PRESET : OT_ID_PLAYER_ID, e->otId);
        for (i = 0; i < MAX_MON_MOVES; i++)
            SetMonMoveSlot(&mon, e->moves[i], i);
        if (e->otName)
        {
            u8 ot[PLAYER_NAME_LENGTH + 1];
            u8 gender = MALE;
            MP_AsciiToGame(ot, e->otName, PLAYER_NAME_LENGTH);
            SetMonData(&mon, MON_DATA_OT_NAME, ot);
            SetMonData(&mon, MON_DATA_OT_GENDER, &gender);
        }
        if (e->fateful)
        {
            u8 one = TRUE, loc = METLOC_FATEFUL_ENCOUNTER;
            SetMonData(&mon, MON_DATA_MODERN_FATEFUL_ENCOUNTER, &one);
            SetMonData(&mon, MON_DATA_MET_LOCATION, &loc);
        }
        result = MP_GiveMonKeepOT(&mon);
        if (result == MON_CANT_GIVE)
        {
            AddMoney(&gSaveBlock1Ptr->money, 10000);
            snprintf(out, outLen, "%s!\nNo room for it: $10000 instead.", e->title);
            return;
        }
        GetSetPokedexFlag(SpeciesToNationalPokedexNum(e->species), FLAG_SET_SEEN);
        GetSetPokedexFlag(SpeciesToNationalPokedexNum(e->species), FLAG_SET_CAUGHT);
        snprintf(out, outLen, "An event Moemon: %s!\n%s", e->title,
                 result == MON_GIVEN_TO_PARTY ? "It joined your party." : "It was sent to your PC.");
    }
    else if (roll < 65)
    {
        u16 total = 0, i, pick;
        const struct ItemPrize *p = &sItemPrizes[0];
        u8 name[24];
        char ascii[24];
        for (i = 0; i < ARRAY_COUNT(sItemPrizes); i++)
            total += sItemPrizes[i].weight;
        pick = Random() % total;
        for (i = 0; i < ARRAY_COUNT(sItemPrizes); i++)
        {
            if (pick < sItemPrizes[i].weight)
            {
                p = &sItemPrizes[i];
                break;
            }
            pick -= sItemPrizes[i].weight;
        }
        CopyItemName(p->item, name);
        MP_GameToAscii(ascii, name, 20);
        if (AddBagItem(p->item, p->qty))
            snprintf(out, outLen, "%s x%d!", ascii, p->qty);
        else
        {
            AddMoney(&gSaveBlock1Ptr->money, 3000);
            snprintf(out, outLen, "%s x%d!\nYour BAG was full: $3000 instead.", ascii, p->qty);
        }
    }
    else
    {
        u32 money = 1000 + (Random() % 10) * 1000 + 500 * (MP_ConnectedCount() > 1 ? MP_ConnectedCount() - 1 : 0);
        AddMoney(&gSaveBlock1Ptr->money, money);
        snprintf(out, outLen, "$%u!", (unsigned)money);
    }
}

// ---------------------------------------------------------------------------
// Screen

enum { GS_MENU, GS_SPIN, GS_RESULT };

static EWRAM_DATA struct
{
    u8 taskId;
    u8 state;
    u16 cursor;
    u16 timer;
    char result[96];
} sGatcha = {0};

static const char *const sSpinNames[] = {
    "RARE CANDY", "MASTER BALL", "JIRACHI", "PP UP", "$5000", "MEW", "NUGGET", "CELEBI", "ULTRA BALL", "DEOXYS",
};

static void Draw(void)
{
    char line[64];
    MpUi_Begin("MOEGATCHA");
    snprintf(line, sizeof(line), "Pulls left: %d", (int)MP_Gatcha_Pulls());
    {
        u8 r[32];
        MP_AsciiToGame(r, line, 30);
        MpUi_TitleRight(r);
    }
    if (sGatcha.state == GS_MENU)
    {
        MpUi_TextAscii(0, "Every shared achievement gives everyone a pull.");
        MpUi_RowAscii(2, sGatcha.cursor == 0, "PULL!", NULL);
        MpUi_RowAscii(3, sGatcha.cursor == 1, "BACK", NULL);
        MpUi_End();
        MpUi_HelpAscii(MP_Gatcha_Pulls() > 0 ? "Event Moemon, items or money!  A: SELECT" : "No pulls yet. Reach team achievements to earn them!");
    }
    else if (sGatcha.state == GS_SPIN)
    {
        MpUi_TextAscii(2, "* * *   PULLING   * * *");
        MpUi_TextAscii(4, sSpinNames[(sGatcha.timer / 6) % ARRAY_COUNT(sSpinNames)]);
        MpUi_End();
        MpUi_HelpAscii("...");
    }
    else
    {
        char *nl = strchr(sGatcha.result, '\n');
        MpUi_TextAscii(1, "You got:");
        if (nl)
        {
            *nl = 0;
            MpUi_TextAscii(3, sGatcha.result);
            MpUi_TextAscii(4, nl + 1);
            *nl = '\n';
        }
        else
            MpUi_TextAscii(3, sGatcha.result);
        MpUi_End();
        MpUi_HelpAscii("A: OK");
    }
}

static void Task_Gatcha(u8 taskId)
{
    switch (sGatcha.state)
    {
    case GS_MENU:
        if (MpUi_MoveCursor(&sGatcha.cursor, 2))
            Draw();
        else if (JOY_NEW(B_BUTTON) || (JOY_NEW(A_BUTTON) && sGatcha.cursor == 1))
        {
            PlaySE(SE_SELECT);
            MpUi_Close();
            DestroyTask(taskId);
        }
        else if (JOY_NEW(A_BUTTON))
        {
            if (MP_Gatcha_Pulls() <= 0)
            {
                PlaySE(SE_FAILURE);
                return;
            }
            MP_Gatcha_AddPulls(-1);
            sGatcha.state = GS_SPIN;
            sGatcha.timer = 0;
            Draw();
        }
        break;
    case GS_SPIN:
        sGatcha.timer++;
        if (sGatcha.timer % 6 == 0)
        {
            PlaySE(SE_BALL_TRAY_ENTER);
            Draw();
        }
        if (sGatcha.timer >= 96)
        {
            DrawPrize(sGatcha.result, sizeof(sGatcha.result));
            PlaySE(SE_SUCCESS);
            sGatcha.state = GS_RESULT;
            Draw();
        }
        break;
    case GS_RESULT:
        if (JOY_NEW(A_BUTTON | B_BUTTON))
        {
            PlaySE(SE_SELECT);
            sGatcha.state = GS_MENU;
            sGatcha.cursor = 0;
            Draw();
        }
        break;
    }
}

void MP_Gatcha_Open(void)
{
    memset(&sGatcha, 0, sizeof(sGatcha));
    MpUi_Open();
    sGatcha.taskId = CreateTask(Task_Gatcha, 80);
    Draw();
}
