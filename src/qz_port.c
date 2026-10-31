// Moemon Kai: Moemon Quetzal's script commands (0xE3-0xF5), used by the Johto and Kanto scripts
// (asm/macros/qz_event.inc; what each one does in Quetzal: docs/quetzal/SCRIPT_COMMANDS.md).
#include <stdio.h>
#include "global.h"
#include "pokemon_storage_system.h"
#include "battle_setup.h"
#include "event_data.h"
#include "frontier_util.h"
#include "mp.h"
#include "pokedex.h"
#include "pokemon.h"
#include "random.h"
#include "script.h"
#include "script_pokemon_util.h"
#include "sound.h"
#include "string_util.h"
#include "strings.h"
#include "constants/battle_frontier.h"
#include "constants/daycare.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/species.h"
#include "constants/event_objects.h"
#include "event_object_movement.h"

static u8 *const sStringVars[] = {gStringVar1, gStringVar2, gStringVar3};

// E3 b: the follower's species name into a string var (and its cry); VAR_RESULT-free
bool8 ScrCmd_qz_bufferfollowerspecies(struct ScriptContext *ctx)
{
    u8 var = ScriptReadByte(ctx);
    struct Pokemon *mon = MP_Follower_LocalMon();
    u16 species = mon ? GetMonData(mon, MON_DATA_SPECIES) : SPECIES_NONE;
    if (species != SPECIES_NONE)
        PlayCry_Normal(species, 0);
    if (var < 3)
        GetSpeciesName(sStringVars[var], species);
    return FALSE;
}

// E4: VAR_RESULT = the player's gender
bool8 ScrCmd_qz_getplayergender(struct ScriptContext *ctx)
{
    gSpecialVar_Result = gSaveBlock2Ptr->playerGender;
    return FALSE;
}

// E5 b b: Quetzal's quest log, which Kai doesn't have (unused by the maps)
bool8 ScrCmd_qz_questmenu(struct ScriptContext *ctx)
{
    ScriptReadByte(ctx);
    ScriptReadByte(ctx);
    gSpecialVar_Result = 0;
    return FALSE;
}

// E6 b h h: waits a frame
bool8 ScrCmd_qz_waitframe_vars(struct ScriptContext *ctx)
{
    ScriptReadByte(ctx);
    ScriptReadHalfword(ctx);
    ScriptReadHalfword(ctx);
    return TRUE;
}

// E7 b: the follower as an object's graphics (unused by the maps; Kai's followers aren't objects)
bool8 ScrCmd_qz_setfollowergfxvar(struct ScriptContext *ctx)
{
    ScriptReadByte(ctx);
    return FALSE;
}

// E8: the wild battle set up by setstaticwildmon / setwildbattle, as a legendary's
bool8 ScrCmd_qz_dostaticwildbattle(struct ScriptContext *ctx)
{
    BattleSetup_StartLegendaryBattle();
    ScriptContext_Stop();
    return TRUE;
}

static const u8 sText_Kenya[] = _("KENYA");
static const u8 sText_Shuckie[] = _("SHUCKIE");

// E9 h: the story gifts (1 KENYA the SPEAROW, 2 SHUCKIE the SHUCKLE, 3 ROTOM, 4 DRATINI); VAR_RESULT = where it went
bool8 ScrCmd_qz_givestorymon(struct ScriptContext *ctx)
{
    u16 id = ScriptReadHalfword(ctx);
    u16 species = SPECIES_NONE, item = ITEM_NONE;
    u8 level = 20;
    const u8 *nick = NULL;
    struct Pokemon mon;

    switch (id)
    {
    case 1: species = SPECIES_SPEAROW; nick = sText_Kenya; break;
    case 2: species = SPECIES_SHUCKLE; nick = sText_Shuckie; item = ITEM_BERRY_JUICE; break;
    case 3: species = SPECIES_ROTOM; break;
    case 4: species = SPECIES_DRATINI; level = 35; item = ITEM_EVERSTONE; break;
    default: gSpecialVar_Result = MON_CANT_GIVE; return FALSE;
    }
    CreateMon(&mon, species, level, USE_RANDOM_IVS, FALSE, 0, OT_ID_PLAYER_ID, 0);
    SetMonData(&mon, MON_DATA_HELD_ITEM, &item);
    if (nick)
        SetMonData(&mon, MON_DATA_NICKNAME, nick);
    gSpecialVar_Result = GiveMonToPlayer(&mon);
    if (gSpecialVar_Result != MON_CANT_GIVE)
    {
        u16 dex = SpeciesToNationalPokedexNum(species);
        GetSetPokedexFlag(dex, FLAG_SET_SEEN);
        GetSetPokedexFlag(dex, FLAG_SET_CAUGHT);
    }
    return FALSE;
}

static bool8 NicknameIs(struct Pokemon *mon, const u8 *name)
{
    u8 nick[POKEMON_NAME_LENGTH + 1];
    GetMonData(mon, MON_DATA_NICKNAME, nick);
    return StringCompare(nick, name) == 0;
}

// EA h: takes back KENYA (1) or SHUCKIE (2); VAR_RESULT 0 taken, 2 not found, 3 SHUCKIE likes you too much to go
bool8 ScrCmd_qz_takebackstorymon(struct ScriptContext *ctx)
{
    u16 id = ScriptReadHalfword(ctx);
    const u8 *name = id == 1 ? sText_Kenya : sText_Shuckie;
    u16 species = id == 1 ? SPECIES_SPEAROW : SPECIES_SHUCKLE;
    u8 i;

    gSpecialVar_Result = 2;
    if (CalculatePlayerPartyCount() < 2)
        return FALSE;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gPlayerParty[i];
        if (GetMonData(mon, MON_DATA_SPECIES) != species || GetMonData(mon, MON_DATA_IS_EGG) || !NicknameIs(mon, name))
            continue;
        if (id == 2 && GetMonData(mon, MON_DATA_FRIENDSHIP) > 200)
        {
            gSpecialVar_Result = 3;
            return FALSE;
        }
        ZeroMonData(mon);
        CompactPartySlots();
        CalculatePlayerPartyCount();
        gSpecialVar_Result = 0;
        return FALSE;
    }
    return FALSE;
}

// EB: keeps only the lead (scripts save the party first); VAR_RESULT 2 when the lead can't go alone
bool8 ScrCmd_qz_keeponlyleadmon(struct ScriptContext *ctx)
{
    u8 i;
    if (GetMonData(&gPlayerParty[0], MON_DATA_HP) == 0 || GetMonData(&gPlayerParty[0], MON_DATA_IS_EGG))
    {
        gSpecialVar_Result = 2;
        return FALSE;
    }
    for (i = 1; i < PARTY_SIZE; i++)
        ZeroMonData(&gPlayerParty[i]);
    CalculatePlayerPartyCount();
    gSpecialVar_Result = 0;
    return FALSE;
}

// EC h b h: the wild Pokémon of a scripted battle (like setwildbattle)
bool8 ScrCmd_qz_setstaticwildmon(struct ScriptContext *ctx)
{
    u16 species = ScriptReadHalfword(ctx);
    u8 level = ScriptReadByte(ctx);
    u16 item = ScriptReadHalfword(ctx);
    CreateScriptedWildMon(species, level, item);
    return FALSE;
}

// ED h: buys party[VAR_0x8004] if it is that species. VAR_RESULT 0 no, 3 your last one, 4 it holds an item,
// 1 a Lv 100 MAGIKARP, 2 another
bool8 ScrCmd_qz_takepartymonofspecies(struct ScriptContext *ctx)
{
    u16 species = ScriptReadHalfword(ctx);
    u16 slot = gSpecialVar_0x8004;
    struct Pokemon *mon;
    u8 i, usable = 0;

    gSpecialVar_Result = 0;
    if (slot >= PARTY_SIZE)
        return FALSE;
    mon = &gPlayerParty[slot];
    if (GetMonData(mon, MON_DATA_SPECIES) != species || GetMonData(mon, MON_DATA_IS_EGG))
        return FALSE;
    for (i = 0; i < PARTY_SIZE; i++)
        if (i != slot && GetMonData(&gPlayerParty[i], MON_DATA_SPECIES) != SPECIES_NONE
         && !GetMonData(&gPlayerParty[i], MON_DATA_IS_EGG) && GetMonData(&gPlayerParty[i], MON_DATA_HP) != 0)
            usable++;
    if (usable == 0)
    {
        gSpecialVar_Result = 3;
        return FALSE;
    }
    if (GetMonData(mon, MON_DATA_HELD_ITEM) != ITEM_NONE)
    {
        gSpecialVar_Result = 4;
        return FALSE;
    }
    gSpecialVar_Result = (species == SPECIES_MAGIKARP && GetMonData(mon, MON_DATA_LEVEL) == MAX_LEVEL) ? 1 : 2;
    ZeroMonData(mon);
    CompactPartySlots();
    CalculatePlayerPartyCount();
    return FALSE;
}

static const u16 sHoennLists[4][9] =
{
    {SPECIES_CACNEA, SPECIES_LOTAD, SPECIES_MAKUHITA, SPECIES_LOMBRE, SPECIES_TRAPINCH, SPECIES_BELDUM, SPECIES_VIBRAVA},
    {SPECIES_TROPIUS, SPECIES_CHIMECHO, SPECIES_ABSOL, SPECIES_CASTFORM},
    {SPECIES_BARBOACH, SPECIES_WHISCASH, SPECIES_MEDITITE, SPECIES_NUMEL, SPECIES_BALTOY, SPECIES_ABSOL, SPECIES_MEDICHAM,
     SPECIES_CAMERUPT},
    {SPECIES_WHISMUR, SPECIES_NOSEPASS, SPECIES_BAGON, SPECIES_RELICANTH, SPECIES_FEEBAS},
};

// EE b: VAR_RESULT = party[VAR_0x8004] is in list 1-4 (the Johto Safari Zone gate's Hoenn Pokémon wishes)
bool8 ScrCmd_qz_checkpartymoninlist(struct ScriptContext *ctx)
{
    u8 list = ScriptReadByte(ctx);
    u16 species;
    u8 i;

    gSpecialVar_Result = FALSE;
    if (list < 1 || list > 4 || gSpecialVar_0x8004 >= PARTY_SIZE || GetMonData(&gPlayerParty[gSpecialVar_0x8004], MON_DATA_IS_EGG))
        return FALSE;
    species = GetMonData(&gPlayerParty[gSpecialVar_0x8004], MON_DATA_SPECIES);
    for (i = 0; i < ARRAY_COUNT(sHoennLists[0]) && sHoennLists[list - 1][i] != SPECIES_NONE; i++)
        if (sHoennLists[list - 1][i] == species)
            gSpecialVar_Result = TRUE;
    return FALSE;
}

static const u16 sOddEggSpecies[] = {SPECIES_PICHU, SPECIES_CLEFFA, SPECIES_IGGLYBUFF, SPECIES_SMOOCHUM, SPECIES_MAGBY,
                                     SPECIES_ELEKID, SPECIES_TYROGUE};

// EF h: the ODD EGG (index 1-7 in the var, Crystal's babies; knows DIZZY PUNCH); VAR_RESULT = where it went
bool8 ScrCmd_qz_giveoddegg(struct ScriptContext *ctx)
{
    u16 n = VarGet(ScriptReadHalfword(ctx));
    struct Pokemon mon;
    u16 move = MOVE_DIZZY_PUNCH;
    u8 cycles = 10, isEgg = TRUE;

    if (n < 1 || n > ARRAY_COUNT(sOddEggSpecies))
        n = 1;
    CreateMon(&mon, sOddEggSpecies[n - 1], EGG_HATCH_LEVEL, USE_RANDOM_IVS, FALSE, 0, OT_ID_PLAYER_ID, 0);
    SetMonMoveSlot(&mon, move, 0);
    SetMonData(&mon, MON_DATA_FRIENDSHIP, &cycles);
    SetMonData(&mon, MON_DATA_IS_EGG, &isEgg);
    SetMonData(&mon, MON_DATA_NICKNAME, gText_EggNickname);
    gSpecialVar_Result = GiveMonToPlayer(&mon);
    return FALSE;
}

// F0 h: BATTLE POINTS (the amount into gStringVar1)
bool8 ScrCmd_qz_addbattlepoints(struct ScriptContext *ctx)
{
    u16 v = ScriptReadHalfword(ctx);
    u32 bp = gSaveBlock2Ptr->frontier.battlePoints + v;
    gSaveBlock2Ptr->frontier.battlePoints = bp > MAX_BATTLE_FRONTIER_POINTS ? MAX_BATTLE_FRONTIER_POINTS : bp;
    ConvertIntToDecimalStringN(gStringVar1, v, STR_CONV_MODE_LEFT_ALIGN, 3);
    return FALSE;
}

bool8 ScrCmd_qz_nop_f1(struct ScriptContext *ctx) { return FALSE; }
bool8 ScrCmd_qz_nop_f2(struct ScriptContext *ctx) { return FALSE; }
bool8 ScrCmd_qz_nop_f3(struct ScriptContext *ctx) { return FALSE; }
bool8 ScrCmd_qz_nop_f4(struct ScriptContext *ctx) { return FALSE; }

// F5: VAR_RESULT = in a co-op room (Quetzal: its own link play)
bool8 ScrCmd_qz_checkmultiplayerorflag(struct ScriptContext *ctx)
{
    gSpecialVar_Result = MP_Lobby_InRoom();
    return FALSE;
}

// A Quetzal special not ported yet (src/qz_specials_stub.c): says so in the log, does nothing
u16 QzSpecialStub(const char *name)
{
    printf("[qz] special %s not ported\n", name);
    gSpecialVar_Result = 0;   // "not available": every caller's path for 0 leaves the story as it is
    ScriptContext_Enable();   // in case the script waits on it (waitstate: scrcmd.c lets it go on)
    return 0;
}
