// Moemon Kai: POKéMON forms outside battle (the user, 2026-11-15: "form changes, and they must scale across all
// regions"). One system for every region: a POKéMON's form is kept in its own data (the high four bits of its language
// byte: MON_DATA_FORM; the save's layout unchanged, and it goes with the POKéMON in trades and co-op), or worked out
// as the original games do:
//   - stored: DEOXYS (0 the game's own Speed Forme as Emerald's, 1 Normal, 2 Attack, 3 Defense; caught in KANTO
//     FireRed's Attack, in SINNOH / UNOVA / JOHTO the Normal Forme), ROTOM (1-5 HEAT, WASH, FROST, FAN, MOW: the
//     appliances), SHAYMIN (1 Sky: the GRACIDEA by day; back at night), BURMY / WORMADAM (0 Plant, 1 Sandy, 2 Trash:
//     the cloak of its last battle's ground; WORMADAM keeps the cloak it evolved in), SHELLOS / GASTRODON (1 East:
//     where it was met: SINNOH's east of MT. CORONET);
//   - worked out: GIRATINA (Origin holding the GRISEOUS ORB), ARCEUS (its PLATE's type), DEERLING / SAWSBUCK (the
//     season: Black's by the month, in every region), BASCULIN (blue stripes from its personality, as UNOWN's letter).
// Each form's stats, types, Abilities, MOéMON and POKéMON pictures and icon: src/mp/mp_forms_data.c
// (tools/moemon_forms/gen_forms.py); the pictures are picked where the game loads them (decompress.c, pokemon.c,
// pokemon_icon.c) by the POKéMON's personality.
#include "global.h"
#include "battle.h"
#include "constants/abilities.h"
#include "constants/battle.h"
#include "constants/items.h"
#include "constants/map_groups.h"
#include "constants/moves.h"
#include "constants/species.h"
#include "data.h"
#include "event_data.h"
#include "main.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "rtc.h"
#include "mp.h"
#include "mp_forms.h"
#include "mp_region.h"
#include "u3d.h"

bool8 MP_MonArtPokemon(void);
struct PokemonStorage *MP_PersonalStorage(void);

static u8 SpeciesFormCount(u16 species)
{
    switch (species)
    {
    case SPECIES_DEOXYS: return 4;
    case SPECIES_ROTOM: return 6;
    case SPECIES_SHAYMIN: case SPECIES_GIRATINA: case SPECIES_SHELLOS: case SPECIES_GASTRODON: case SPECIES_BASCULIN:
        return 2;
    case SPECIES_BURMY: case SPECIES_WORMADAM: return 3;
    case SPECIES_DEERLING: case SPECIES_SAWSBUCK: return 4;
    case SPECIES_ARCEUS: return 18;
    }
    return 1;
}

bool8 MP_Form_HasForms(u16 species)
{
    return SpeciesFormCount(species) > 1;
}

const struct MpFormData *MP_Form_Data(u16 species, u8 form)
{
    u32 i;
    if (form == 0)
        return NULL;
    for (i = 0; i < gMpFormCount; i++)
        if (gMpForms[i].species == species && gMpForms[i].form == form)
            return &gMpForms[i];
    return NULL;
}

// ARCEUS's PLATE: its form (tools/moemon_forms/gen_forms.py ARCEUS order, 0 none)
static u8 PlateForm(u16 item)
{
    static const u16 sPlates[17] = {ITEM_FIST_PLATE, ITEM_SKY_PLATE, ITEM_TOXIC_PLATE, ITEM_EARTH_PLATE,
                                    ITEM_STONE_PLATE, ITEM_INSECT_PLATE, ITEM_SPOOKY_PLATE, ITEM_IRON_PLATE,
                                    ITEM_FLAME_PLATE, ITEM_SPLASH_PLATE, ITEM_MEADOW_PLATE, ITEM_ZAP_PLATE,
                                    ITEM_MIND_PLATE, ITEM_ICICLE_PLATE, ITEM_DRACO_PLATE, ITEM_DREAD_PLATE,
                                    ITEM_PIXIE_PLATE};
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sPlates); i++)
        if (sPlates[i] == item)
            return i + 1;
    return 0;
}

static u8 Worked(u16 species, u32 personality, u16 item, u8 stored)
{
    switch (species)
    {
    case SPECIES_GIRATINA: return item == ITEM_GRISEOUS_ORB;
    case SPECIES_ARCEUS: return PlateForm(item);
    case SPECIES_DEERLING: case SPECIES_SAWSBUCK: return U3D_Season();
    case SPECIES_BASCULIN: return (personality >> 8) & 1;
    }
    return stored < SpeciesFormCount(species) ? stored : 0;
}

u8 MP_Form_Of(const struct BoxPokemon *box)
{
    u16 species = GetBoxMonData((struct BoxPokemon *)box, MON_DATA_SPECIES, NULL);
    if (!MP_Form_HasForms(species) || GetBoxMonData((struct BoxPokemon *)box, MON_DATA_IS_EGG, NULL))
        return 0;
    return Worked(species, GetBoxMonData((struct BoxPokemon *)box, MON_DATA_PERSONALITY, NULL),
                  GetBoxMonData((struct BoxPokemon *)box, MON_DATA_HELD_ITEM, NULL),
                  GetBoxMonData((struct BoxPokemon *)box, MON_DATA_FORM, NULL));
}

void MP_Form_Set(struct BoxPokemon *box, u8 form)
{
    u32 f = form;
    SetBoxMonData(box, MON_DATA_FORM, &f);
}

// The POKeMON with this personality the game holds now (the party, the foes, the boxes): its form, for the pictures
// (the game loads them by species and personality)
u8 MP_Form_OfPersonality(u16 species, u32 personality)
{
    static u32 sLastPid, sLastFrame;
    static u16 sLastSpecies;
    static u8 sLastForm;
    struct PokemonStorage *st;
    u32 i;
    if (!MP_Form_HasForms(species))
        return 0;
    if (species == SPECIES_BASCULIN || species == SPECIES_DEERLING || species == SPECIES_SAWSBUCK)
        return Worked(species, personality, ITEM_NONE, 0);
    if (sLastPid == personality && sLastSpecies == species && sLastFrame == gMain.vblankCounter1)
        return sLastForm;
    sLastPid = personality, sLastSpecies = species, sLastFrame = gMain.vblankCounter1, sLastForm = 0;
    for (i = 0; i < PARTY_SIZE * 2; i++)
    {
        struct Pokemon *m = i < PARTY_SIZE ? &gPlayerParty[i] : &gEnemyParty[i - PARTY_SIZE];
        if (GetMonData(m, MON_DATA_PERSONALITY, NULL) == personality && GetMonData(m, MON_DATA_SPECIES, NULL) == species)
            return sLastForm = MP_Form_Of(&m->box);
    }
    st = MP_PersonalStorage();
    for (i = 0; st && i < TOTAL_BOXES_COUNT * IN_BOX_COUNT; i++)
    {
        struct BoxPokemon *b = &st->boxes[i / IN_BOX_COUNT][i % IN_BOX_COUNT];
        if (GetBoxMonData(b, MON_DATA_PERSONALITY, NULL) == personality && GetBoxMonData(b, MON_DATA_SPECIES, NULL) == species)
            return sLastForm = MP_Form_Of(b);
    }
    return 0;
}

// ------------------------------------------------------------------------------------------------- the pictures
// decompress.c LoadSpecialPokePic: the form's picture (NULL: the species' own)
const u32 *MP_Form_Pic(u16 species, u32 personality, bool8 front)
{
    const struct MpFormData *d;
    if (!MP_Form_HasForms(species))
        return NULL;
    d = MP_Form_Data(species, MP_Form_OfPersonality(species, personality));
    return d ? (front ? d->front : d->back)[MP_MonArtPokemon() ? 1 : 0] : NULL;
}

// DEOXYS: TRUE when its picture is the Normal Forme's (the picture's first frame: no copying the game's own over it)
bool8 MP_Form_DeoxysNormal(u32 personality)
{
    return MP_Form_OfPersonality(SPECIES_DEOXYS, personality) == 1;
}

// pokemon.c MonSpritePal / MonSpritePalStruct: the form's palette (NULL: the species' own), under the species' own tag
// (the game makes the POKeMON's sprite with that tag)
const struct CompressedSpritePalette *MP_Form_PalStruct(u16 species, u32 personality, bool8 shiny)
{
    static struct CompressedSpritePalette sPals[64][2];
    const struct MpFormData *d;
    u32 k;
    if (!MP_Form_HasForms(species) || (d = MP_Form_Data(species, MP_Form_OfPersonality(species, personality))) == NULL)
        return NULL;
    k = d - gMpForms;
    if (k >= ARRAY_COUNT(sPals))
        return NULL;
    sPals[k][shiny].data = (shiny ? d->shiny : d->pal)[MP_MonArtPokemon() ? 1 : 0];
    sPals[k][shiny].tag = species + (shiny ? SPECIES_SHINY_TAG : 0);   // (the species' own tag: its sprites look for it)
    return &sPals[k][shiny];
}

// pokemon_icon.c GetMonIconPtr: the form's icon (NULL: the species' own)
const u8 *MP_Form_Icon(u16 species, u32 personality)
{
    const struct MpFormData *d;
    if (!MP_Form_HasForms(species) || (d = MP_Form_Data(species, MP_Form_OfPersonality(species, personality))) == NULL)
        return NULL;
    return d->icon;
}

// ------------------------------------------------------------------------------------------------- stats, types
// pokemon.c CalculateMonStats: STAT's base (HP, Attack, Defense, Speed, Sp. Atk, Sp. Def), the form's
u8 MP_Form_BaseStat(struct Pokemon *mon, u8 stat, u8 own)
{
    u16 species = GetMonData(mon, MON_DATA_SPECIES, NULL);
    const struct MpFormData *d = MP_Form_HasForms(species) ? MP_Form_Data(species, MP_Form_Of(&mon->box)) : NULL;
    return d && d->stats[0] ? d->stats[stat] : own;
}

static void TypesOf(u16 species, u8 form, u8 *t0, u8 *t1)
{
    const struct MpFormData *d = MP_Form_Data(species, form);
    *t0 = d ? d->types[0] : gSpeciesInfo[species].types[0];
    *t1 = d ? d->types[1] : gSpeciesInfo[species].types[1];
}

void MP_Form_MonTypes(struct Pokemon *mon, u8 *t0, u8 *t1)
{
    u16 species = GetMonData(mon, MON_DATA_SPECIES, NULL);
    TypesOf(species, MP_Form_HasForms(species) ? MP_Form_Of(&mon->box) : 0, t0, t1);
}

u16 MP_Form_Ability(struct Pokemon *mon, u16 own)
{
    u16 species = GetMonData(mon, MON_DATA_SPECIES, NULL);
    const struct MpFormData *d = MP_Form_HasForms(species) ? MP_Form_Data(species, MP_Form_Of(&mon->box)) : NULL;
    u8 n = GetMonData(mon, MON_DATA_ABILITY_NUM, NULL);
    if (d == NULL || d->abilities[0] == ABILITY_NONE)
        return own;
    return n && d->abilities[1] != ABILITY_NONE ? d->abilities[1] : d->abilities[0];
}

// battle_main.c / battle_script_commands.c: a battler's types and Ability as its form's (gBattleMons set from species)
void MP_Form_FitBattler(u8 battler)
{
    u16 species = gBattleMons[battler].species;
    u8 form;
    const struct MpFormData *d;
    if (!MP_Form_HasForms(species))
        return;
    form = MP_Form_OfPersonality(species, gBattleMons[battler].personality);
    if ((d = MP_Form_Data(species, form)) == NULL)
        return;
    gBattleMons[battler].types[0] = d->types[0];
    gBattleMons[battler].types[1] = d->types[1];
    if (d->abilities[0] != ABILITY_NONE)
        gBattleMons[battler].ability = gBattleMons[battler].abilityNum && d->abilities[1] != ABILITY_NONE
                                     ? d->abilities[1] : d->abilities[0];
}

// ------------------------------------------------------------------------------------------------- changes
// SINNOH's east sea (Platinum's East Sea SHELLOS / GASTRODON: east of MT. CORONET and the islands past it)
static bool8 EastSea(void)
{
    static const u16 sEast[] = {MAP_SINNOH_ROUTE213, MAP_SINNOH_ROUTE220, MAP_SINNOH_ROUTE221, MAP_SINNOH_ROUTE222,
                                MAP_SINNOH_ROUTE223, MAP_SINNOH_ROUTE224, MAP_SINNOH_ROUTE225, MAP_SINNOH_ROUTE226,
                                MAP_SINNOH_ROUTE227, MAP_SINNOH_ROUTE228, MAP_SINNOH_ROUTE229, MAP_SINNOH_ROUTE230,
                                MAP_SINNOH_SUNYSHORE_CITY, MAP_SINNOH_PASTORIA_CITY, MAP_SINNOH_VALOR_LAKEFRONT,
                                MAP_SINNOH_FIGHT_AREA, MAP_SINNOH_SURVIVAL_AREA, MAP_SINNOH_RESORT_AREA};
    u16 here = gSaveBlock1Ptr->location.mapNum | (gSaveBlock1Ptr->location.mapGroup << 8);
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sEast); i++)
        if (sEast[i] == here)
            return TRUE;
    return FALSE;
}

// pokemon.c CreateBoxMon: a new POKeMON's form by where it is met (DEOXYS by the region's own game, SHELLOS's sea)
void MP_Form_Init(struct BoxPokemon *box)
{
    u16 species = GetBoxMonData(box, MON_DATA_SPECIES, NULL);
    u8 r = MP_Region_Current();
    if (species == SPECIES_DEOXYS)
        MP_Form_Set(box, r == REGION_KANTO ? 2 : (r == REGION_SINNOH || r == REGION_UNOVA || r == REGION_JOHTO) ? 1 : 0);
    else if (species == SPECIES_SHELLOS || species == SPECIES_GASTRODON)
        MP_Form_Set(box, r == REGION_SINNOH && EastSea());
}

// ROTOM's moves (Black's POKE_TOOL_SetRotomFormMove; Platinum's the same): a form's own move in place of another
// form's, in the slot given when it knows none of them; back to the Normal form, a ROTOM with no move left learns
// THUNDERSHOCK
void MP_Form_SetRotom(struct Pokemon *mon, u8 form, u8 slot)
{
    static const u16 sMoves[6] = {MOVE_NONE, MOVE_OVERHEAT, MOVE_HYDRO_PUMP, MOVE_BLIZZARD, MOVE_AIR_SLASH, MOVE_LEAF_STORM};
    u32 i, k, known = 0;
    u16 none = MOVE_NONE;
    if (form >= ARRAY_COUNT(sMoves))
        return;
    for (i = 0; i < MAX_MON_MOVES; i++)
    {
        u16 m = GetMonData(mon, MON_DATA_MOVE1 + i, NULL);
        for (k = 1; k < ARRAY_COUNT(sMoves); k++)
            if (m == sMoves[k])
            {
                if (form != 0)
                {
                    u8 pp = gBattleMoves[sMoves[form]].pp;
                    SetMonData(mon, MON_DATA_MOVE1 + i, &sMoves[form]);
                    SetMonData(mon, MON_DATA_PP1 + i, &pp);
                }
                else
                {
                    SetMonData(mon, MON_DATA_MOVE1 + i, &none);
                    SetMonData(mon, MON_DATA_PP1 + i, &none);
                }
                known = 1;
            }
    }
    if (!known && form != 0 && slot < MAX_MON_MOVES)
    {
        u8 pp = gBattleMoves[sMoves[form]].pp;
        SetMonData(mon, MON_DATA_MOVE1 + slot, &sMoves[form]);
        SetMonData(mon, MON_DATA_PP1 + slot, &pp);
    }
    for (i = 0, k = 0; i < MAX_MON_MOVES; i++)
        k += GetMonData(mon, MON_DATA_MOVE1 + i, NULL) != MOVE_NONE;
    if (k == 0)
    {
        u16 m = MOVE_THUNDER_SHOCK;
        u8 pp = gBattleMoves[m].pp;
        SetMonData(mon, MON_DATA_MOVE1, &m);
        SetMonData(mon, MON_DATA_PP1, &pp);
    }
    MP_Form_Set(&mon->box, form);
    CalculateMonStats(mon);
    MPNet_Log("forms: ROTOM to form %u", form);
}

// a form change in place (DEOXYS at a meteorite, SHAYMIN's GRACIDEA...): stats worked out again
void MP_Form_Change(struct Pokemon *mon, u8 form)
{
    MP_Form_Set(&mon->box, form);
    CalculateMonStats(mon);
    MPNet_Log("forms: %u to form %u", GetMonData(mon, MON_DATA_SPECIES, NULL), form);
}

// battle_main.c, a battle's end: each BURMY that fought takes the cloak of the ground it fought on (Platinum's rule)
void MP_Form_AfterBattle(void)
{
    u8 cloak;
    u32 i;
    switch (gBattleEnvironment)
    {
    case BATTLE_ENVIRONMENT_SAND: case BATTLE_ENVIRONMENT_CAVE: case BATTLE_ENVIRONMENT_MOUNTAIN: cloak = 1; break;
    case BATTLE_ENVIRONMENT_BUILDING: case BATTLE_ENVIRONMENT_PLAIN: cloak = 2; break;
    default: cloak = 0; break;
    }
    for (i = 0; i < PARTY_SIZE; i++)
        if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES, NULL) == SPECIES_BURMY
         && (gBattleStruct != NULL ? (gSentPokesToOpponent[0] | gSentPokesToOpponent[1]) & (1u << i) : 0)
         && MP_Form_Of(&gPlayerParty[i].box) != cloak)
            MP_Form_Change(&gPlayerParty[i], cloak);
}

// Each map load (mp_region.c MP_Region_GiveDevices): SHAYMIN's Sky Forme ends at night (Platinum)
void MP_Form_MapLoaded(void)
{
    u32 i;
    RtcCalcLocalTime();
    if (gLocalTime.hours >= 6 && gLocalTime.hours < 20)
        return;
    for (i = 0; i < PARTY_SIZE; i++)
        if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES, NULL) == SPECIES_SHAYMIN && MP_Form_Of(&gPlayerParty[i].box) == 1)
            MP_Form_Change(&gPlayerParty[i], 0);
}

// The GRACIDEA used on a POKeMON (item_use.c / party_menu.c): TRUE when SHAYMIN took its Sky Forme (by day)
bool8 MP_Form_Gracidea(struct Pokemon *mon)
{
    RtcCalcLocalTime();
    if (GetMonData(mon, MON_DATA_SPECIES, NULL) != SPECIES_SHAYMIN || MP_Form_Of(&mon->box) == 1
     || gLocalTime.hours < 6 || gLocalTime.hours >= 20 || (GetMonData(mon, MON_DATA_STATUS, NULL) & STATUS1_FREEZE))
        return FALSE;
    MP_Form_Change(mon, 1);
    return TRUE;
}

// Tests: `mp form SLOT FORM`: party SLOT's form; prints it and its types
void MP_Form_Debug(u8 slot, s16 form)
{
    u8 t0, t1;
    if (slot >= PARTY_SIZE)
        return;
    if (form >= 0)
    {
        if (GetMonData(&gPlayerParty[slot], MON_DATA_SPECIES, NULL) == SPECIES_ROTOM)
            MP_Form_SetRotom(&gPlayerParty[slot], form, 0);
        else
            MP_Form_Change(&gPlayerParty[slot], form);
    }
    MP_Form_MonTypes(&gPlayerParty[slot], &t0, &t1);
    printf("[test] form: slot %u species %u form %u types %u %u atk %u ability %u\n", slot,
           GetMonData(&gPlayerParty[slot], MON_DATA_SPECIES, NULL), MP_Form_Of(&gPlayerParty[slot].box), t0, t1,
           GetMonData(&gPlayerParty[slot], MON_DATA_ATK, NULL), GetMonAbility(&gPlayerParty[slot]));
}
