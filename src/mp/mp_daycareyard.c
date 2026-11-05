// Moemon Kai: ROUTE 34's DAY-CARE yard, Crystal's (pokecrystal maps/Route34.asm DayCareMon1Script / DayCareMon2Script,
// engine/events/daycare.asm DayCareMon1 / DayCareMon2; audit case 47): the POKéMON the MAN and the LADY raise walk in the
// yard. Each is an invisible map person (local ids 21, 22: they walk, block and are talked to as Crystal's do) with the
// POKéMON's follower picture drawn on it (mp_follower.c MP_Follower_CreateMonSprite).
#include "global.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "sprite.h"
#include "string_util.h"
#include "pokemon.h"
#include "daycare.h"
#include "mp_region.h"
#include "mp_life_field.h"
#include "constants/kai_regions.h"
#include "constants/event_objects.h"

u8 MP_Daycare_Compatibility(const struct BoxPokemon *a, const struct BoxPokemon *b);   // (daycare.c)

#define YARD_GROUP 35
#define YARD_NUM   41
#define TAG_YARD_PAL 0x11A0   // + 0 / 1
#define YARD_MAGIC 0x5934

struct YardMon
{
    u8 spriteId;
    u8 size;
    u16 species;
    u8 anim;
    struct SpriteFrameImage frames[8];
    struct SpriteTemplate tmpl;
};

static EWRAM_DATA struct YardMon sYard[2];
static bool8 sYardInit;

static void SpriteCB_Yard(struct Sprite *sprite)
{
}

static bool8 Ours(u8 i)
{
    u8 id = sYard[i].spriteId;
    return id < MAX_SPRITES && gSprites[id].inUse && gSprites[id].callback == SpriteCB_Yard
        && gSprites[id].data[6] == YARD_MAGIC && gSprites[id].data[7] == i;
}

static void Drop(u8 i)
{
    if (Ours(i))
    {
        DestroySprite(&gSprites[sYard[i].spriteId]);
        FreeSpritePaletteByTag(TAG_YARD_PAL + i);
    }
    sYard[i].spriteId = MAX_SPRITES;
    sYard[i].species = SPECIES_NONE;
}

// The POKéMON of the MAN (i = 0) or the LADY (i = 1): its slot in the game's DAY CARE, or -1
static s8 YardSlot(u8 i)
{
    u16 owners = VarGet(VAR_JOHTO_DAYCARE_OWNERS);
    u8 man = (i == 0);
    u32 s;
    for (s = 0; s < DAYCARE_MON_COUNT; s++)
        if (GetBoxMonData(&gSaveBlock1Ptr->daycare.mons[s].mon, MON_DATA_SPECIES) != SPECIES_NONE && ((owners >> s) & 1) == man)
            return s;
    return -1;
}

// Every frame on the field (overworld.c, after the camera)
void MP_DaycareYard_Update(void)
{
    u8 i;
    bool8 here = gSaveBlock1Ptr->location.mapGroup == YARD_GROUP && gSaveBlock1Ptr->location.mapNum == YARD_NUM;
    if (!sYardInit)
    {
        sYard[0].spriteId = sYard[1].spriteId = MAX_SPRITES;
        sYardInit = TRUE;
    }
    for (i = 0; i < 2; i++)
    {
        u8 obj;
        struct ObjectEvent *oe;
        struct Sprite *host, *sp;
        s8 slot = here ? YardSlot(i) : -1;
        u16 species = slot >= 0 ? GetBoxMonData(&gSaveBlock1Ptr->daycare.mons[slot].mon, MON_DATA_SPECIES) : SPECIES_NONE;
        if (!here || TryGetObjectEventIdByLocalIdAndMap(21 + i, YARD_NUM, YARD_GROUP, &obj) || species == SPECIES_NONE)
        {
            if (sYard[i].spriteId != MAX_SPRITES)
                Drop(i);
            continue;
        }
        oe = &gObjectEvents[obj];
        host = &gSprites[oe->spriteId];
        host->invisible = TRUE;   // (the person is only there to walk, block and be talked to)
        if (!Ours(i) || sYard[i].species != species)
        {
            u32 pid = GetBoxMonData(&gSaveBlock1Ptr->daycare.mons[slot].mon, MON_DATA_PERSONALITY);
            bool8 female = GetGenderFromSpeciesAndPersonality(species, pid) == MON_FEMALE;
            Drop(i);
            sYard[i].spriteId = MP_Follower_CreateMonSprite(SpeciesToNationalPokedexNum(species), female, 0, TAG_YARD_PAL + i,
                                                            sYard[i].frames, &sYard[i].tmpl, &sYard[i].size, SpriteCB_Yard);
            if (sYard[i].spriteId == MAX_SPRITES)
                continue;
            gSprites[sYard[i].spriteId].data[6] = YARD_MAGIC;
            gSprites[sYard[i].spriteId].data[7] = i;
            sYard[i].species = species;
            sYard[i].anim = 0xFF;
        }
        sp = &gSprites[sYard[i].spriteId];
        {
            u8 frame = (oe->facingDirection >= DIR_SOUTH && oe->facingDirection <= DIR_EAST ? oe->facingDirection - DIR_SOUTH : 0) * 2;
            if (oe->singleMovementActive || oe->heldMovementActive)
                frame += (host->x + host->y) / 8 & 1;   // (a step as it walks)
            if (frame != sYard[i].anim)
            {
                StartSpriteAnim(sp, frame);
                sYard[i].anim = frame;
            }
        }
        sp->x = host->x;
        sp->y = host->y - (sYard[i].size == 64 ? 16 : 0);
        sp->x2 = host->x2;
        sp->y2 = host->y2;
        sp->oam.priority = host->oam.priority;
        sp->subpriority = host->subpriority;
        sp->invisible = FALSE;
    }
}

// Talking to one (VAR_0x8009: 1 the MAN's, 0 the LADY's): its species (VAR_RESULT) and nickname (STR_VAR_1)
u16 MP_JohtoDaycare_YardMon(void)
{
    s8 slot = YardSlot(gSpecialVar_0x8009 ? 0 : 1);
    if (slot < 0)
        return SPECIES_NONE;
    GetBoxMonNickname(&gSaveBlock1Ptr->daycare.mons[slot].mon, gStringVar1);
    return GetBoxMonData(&gSaveBlock1Ptr->daycare.mons[slot].mon, MON_DATA_SPECIES);
}

// ... and the other's: its nickname in STR_VAR_1, Crystal's line for how they get on (DayCareMonCompatibilityText) in
// VAR_RESULT: 0 there's no other, 1 no interest, 2 shows interest, 3 friendly, 4 appears to care for
u16 MP_JohtoDaycare_YardOther(void)
{
    s8 a = YardSlot(gSpecialVar_0x8009 ? 0 : 1), b = YardSlot(gSpecialVar_0x8009 ? 1 : 0);
    u8 score;
    if (a < 0 || b < 0)
        return 0;
    GetBoxMonNickname(&gSaveBlock1Ptr->daycare.mons[b].mon, gStringVar1);
    score = MP_Daycare_Compatibility(&gSaveBlock1Ptr->daycare.mons[a].mon, &gSaveBlock1Ptr->daycare.mons[b].mon);
    if (score >= 70)
        return 4;
    if (score >= 50)
        return 3;
    if (score >= 20)
        return 2;
    return 1;
}
