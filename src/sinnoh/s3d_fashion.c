// Moemon Kai: Platinum's FASHION CASE (SUPER CONTESTS' accessories and backdrops, the user's 2026-11-02 call to build
// them): how many of each of Platinum's 100 accessories the player has (the first 61 up to 9 each, the rest one) and
// which of its 18 backdrops, kept in "<save>.region" (chunk FASH, mp_region.c). Its names are Platinum's (the
// contest_accessory_names, _with_articles and contest_backdrop_names banks). The scripts' commands
// (AddAccessory, CanFitAccessory, CheckBackdrop...: tools/sinnoh/gen_scripts.py) come here through `sinnoh SINNOH_FASHION`.
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "event_data.h"
#include "random.h"
#include "pokeblock.h"
#include "string_util.h"
#include "mp.h"
#include "mp_region.h"
#include "s3d.h"
#include "s3d_contest.h"

#define ACCESSORY_COUNT 100
#define NON_UNIQUE_ACCESSORY_COUNT 61
#define BACKDROP_COUNT 18
#define MAX_NON_UNIQUE 9
#define ACCESSORY_PRETTY_DEWDROP 34
#define ACCESSORY_GLITTER_POWDER 49
#define FASHION_MAGIC 0x31534146   // "FAS1"

static struct
{
    u32 magic;
    u8 accessories[ACCESSORY_COUNT];
    u8 backdrops[BACKDROP_COUNT];
    u8 pad[2];
} sCase;

static void Fresh(void)
{
    if (sCase.magic != FASHION_MAGIC)
    {
        memset(&sCase, 0, sizeof(sCase));
        sCase.magic = FASHION_MAGIC;
    }
}

const void *Sinnoh_FashionData(u32 *size)
{
    Fresh();
    *size = sizeof(sCase);
    return &sCase;
}

void Sinnoh_FashionLoad(const void *data, u32 size)
{
    memset(&sCase, 0, sizeof(sCase));
    if (size == sizeof(sCase) && ((const u32 *)data)[0] == FASHION_MAGIC)
        memcpy(&sCase, data, size);
    Fresh();
}

// The contest hall's photos: the winning POKéMON of each type, dressed as it won (chunk CPHO; Platinum's image clips)
#define PHOTO_MAGIC 0x4F485043   // "CPHO"
static struct
{
    u32 magic;
    struct ContestPhoto
    {
        u16 species;
        u8 kept, rank;
        u32 personality, otId;
        u8 nick[12], trainer[12];
        u8 wornCount;
        s8 backdrop;
        u8 worn[CONTEST_MAX_WORN][3];
        u16 title;           // (the studio's photo: its one-word title, an easy chat word)
    } photo[6];              // (0-4: the contest hall's winners by type; 5: JUBILIFE TV's dress-up studio)
} sPhotos;

const void *Sinnoh_PhotoData(u32 *size)
{
    sPhotos.magic = PHOTO_MAGIC;
    *size = sizeof(sPhotos);
    return &sPhotos;
}

void Sinnoh_PhotoLoad(const void *data, u32 size)
{
    memset(&sPhotos, 0, sizeof(sPhotos));
    if (size == sizeof(sPhotos) && ((const u32 *)data)[0] == PHOTO_MAGIC)
        memcpy(&sPhotos, data, size);
}

void Sinnoh_ContestKeepPhoto(u8 type, const struct ContestEntrant *e)
{
    struct ContestPhoto *p = &sPhotos.photo[type % 6];
    u8 i;
    memset(p, 0, sizeof(*p));
    p->kept = TRUE;
    p->rank = gSinnohContest.rank;
    p->species = e->species;
    p->personality = e->personality;
    p->otId = e->otId;
    memcpy(p->nick, e->nick, sizeof(p->nick));
    memcpy(p->trainer, e->trainer, sizeof(p->trainer));
    p->wornCount = e->wornCount;
    p->backdrop = e->backdrop;
    for (i = 0; i < e->wornCount && i < CONTEST_MAX_WORN; i++)
    {
        p->worn[i][0] = e->worn[i].accessory;
        p->worn[i][1] = e->worn[i].x;
        p->worn[i][2] = e->worn[i].y;
    }
}

// a kept photo as an entrant the contest's screens can draw (NULL: none kept)
u16 *Sinnoh_PhotoTitle(u8 type)
{
    return &sPhotos.photo[type % 6].title;
}

const struct ContestEntrant *Sinnoh_ContestPhoto(u8 type)
{
    static struct ContestEntrant e;
    const struct ContestPhoto *p = &sPhotos.photo[type % 6];
    u8 i;
    if (!p->kept)
        return NULL;
    memset(&e, 0, sizeof(e));
    e.species = p->species;
    e.personality = p->personality;
    e.otId = p->otId;
    memcpy(e.nick, p->nick, sizeof(e.nick));
    memcpy(e.trainer, p->trainer, sizeof(e.trainer));
    e.wornCount = p->wornCount;
    e.backdrop = p->backdrop;
    for (i = 0; i < p->wornCount && i < CONTEST_MAX_WORN; i++)
    {
        e.worn[i].accessory = p->worn[i][0];
        e.worn[i].x = p->worn[i][1];
        e.worn[i].y = p->worn[i][2];
    }
    return &e;
}

void Sinnoh_FashionReset(void)
{
    memset(&sCase, 0, sizeof(sCase));
    memset(&sPhotos, 0, sizeof(sPhotos));
    Fresh();
}

static u8 MaxOf(u16 accessory)
{
    return accessory < NON_UNIQUE_ACCESSORY_COUNT ? MAX_NON_UNIQUE : 1;
}

u8 Sinnoh_AccessoryCount(u16 accessory)
{
    Fresh();
    return accessory < ACCESSORY_COUNT ? sCase.accessories[accessory] : 0;
}

bool8 Sinnoh_CanFitAccessory(u16 accessory, u16 count)
{
    return accessory < ACCESSORY_COUNT && Sinnoh_AccessoryCount(accessory) + count <= MaxOf(accessory);
}

void Sinnoh_AddAccessory(u16 accessory, u16 count)
{
    u32 n;
    if (accessory >= ACCESSORY_COUNT)
        return;
    Fresh();
    n = sCase.accessories[accessory] + count;
    sCase.accessories[accessory] = n > MaxOf(accessory) ? MaxOf(accessory) : n;
    MPNet_Log("sinnoh fashion: accessory %d now %d", accessory, sCase.accessories[accessory]);
}

void Sinnoh_RemoveAccessory(u16 accessory, u16 count)
{
    if (accessory >= ACCESSORY_COUNT)
        return;
    Fresh();
    sCase.accessories[accessory] = sCase.accessories[accessory] > count ? sCase.accessories[accessory] - count : 0;
}

bool8 Sinnoh_HasBackdrop(u16 backdrop)
{
    Fresh();
    return backdrop < BACKDROP_COUNT && sCase.backdrops[backdrop];
}

void Sinnoh_AddBackdrop(u16 backdrop)
{
    Fresh();
    if (backdrop < BACKDROP_COUNT)
        sCase.backdrops[backdrop] = TRUE;
    MPNet_Log("sinnoh fashion: backdrop %d", backdrop);
}

void Sinnoh_AccessoryName(u8 *dest, u16 accessory, bool8 article)
{
    char name[24];
    snprintf(name, sizeof(name), article ? "pl_msg_00000387_%05u" : "pl_msg_00000386_%05u", accessory);
    StringCopy(dest, S3D_TextByName(name));
}

void Sinnoh_BackdropName(u8 *dest, u16 backdrop)
{
    char name[24];
    snprintf(name, sizeof(name), "pl_msg_00000388_%05u", backdrop);
    StringCopy(dest, S3D_TextByName(name));
}

enum {
    FASHCALL_ADD,            // A: the accessory, B: how many (each a value or variable)
    FASHCALL_CAN_FIT,        // A: the accessory, B: variable <- it fits VAR_TEMP_F more (the count: put there first)
    FASHCALL_NAME,           // A: the string slot, B: the accessory
    FASHCALL_NAME_ARTICLE,   // A: the string slot, B: the accessory
    FASHCALL_ADD_BACKDROP,   // A: the backdrop
    FASHCALL_HAS_BACKDROP,   // A: the backdrop, B: variable <- TRUE when the case has it
    FASHCALL_BACKDROP_NAME,  // A: the string slot, B: the backdrop
    FASHCALL_MASSAGE,        // A: variable <- a random DEWDROP...GLITTER POWDER the case can still take (0xFFFF: none)
    FASHCALL_REMOVE,         // A: the accessory, B: how many
    FASHCALL_POFFIN_SPACE,   // A: variable <- the POKéBLOCK case has room (Platinum's CheckHasEmptyPoffinCaseSlot)
    FASHCALL_POFFIN_FREE,    // A: variable <- its free places (GetEmptyPoffinCaseSlotCount)
    FASHCALL_POFFIN_FLAVOR,  // A: 0-5 (spicy, dry, sweet, bitter, sour, smoothness), B: its value (kept for the next)
    FASHCALL_POFFIN_GIVE,    // A: variable <- the POKéBLOCK given (Platinum's GivePoffin: the flavors kept before)
    FASHCALL_DRESS_UP_PHOTO, // A: the photo's place, B: variable <- a dress-up photo is kept there
    FASHCALL_CONTEST_PHOTO,  // A: the photo's place, B: variable <- a contest photo is kept there
    FASHCALL_POFFIN_CAN_COOK,// A: variable <- 0 can cook, 1 no BERRY, 2 the case is full (CheckCanCookPoffin)
};

// The photos: the contest hall's winners (slot: the contest type); JUBILIFE TV's gallery: the player's own dress-up
// photo in its first frame (the other frames held friends' photos by wireless in Platinum: empty here)
bool8 Sinnoh_PhotoKept(bool8 contest, u16 slot)
{
    if (!contest)
        return slot == 0 && sPhotos.photo[5].kept;
    return slot < 5 && sPhotos.photo[slot].kept;
}

// Platinum's POFFINS are this game's POKéBLOCKS (the same five flavors raise the same five conditions; smoothness as
// their feel): the notes' rule, what Kai has SINNOH uses
static u8 sPoffin[6];

static u16 FreePokeblockSlots(void)
{
    u16 i, n = 0;
    for (i = 0; i < POKEBLOCKS_COUNT; i++)
        n += gSaveBlock1Ptr->pokeblocks[i].color == PBLOCK_CLR_NONE;
    return n;
}

u8 *StringVar(u16 slot);   // (s3d_scrcmd.c)

void Sinnoh_FashionCall(u16 call, u16 a, u16 b)
{
    switch (call)
    {
    case FASHCALL_ADD:
        Sinnoh_AddAccessory(VarGet(a), VarGet(b));
        break;
    case FASHCALL_CAN_FIT:
        VarSet(b, Sinnoh_CanFitAccessory(VarGet(a), VarGet(0x400F)));
        break;
    case FASHCALL_NAME:
    case FASHCALL_NAME_ARTICLE:
        Sinnoh_AccessoryName(StringVar(a), VarGet(b), call == FASHCALL_NAME_ARTICLE);
        break;
    case FASHCALL_ADD_BACKDROP:
        Sinnoh_AddBackdrop(VarGet(a));
        break;
    case FASHCALL_HAS_BACKDROP:
        VarSet(b, Sinnoh_HasBackdrop(VarGet(a)));
        break;
    case FASHCALL_BACKDROP_NAME:
        Sinnoh_BackdropName(StringVar(a), VarGet(b));
        break;
    case FASHCALL_MASSAGE:
    {
        u16 list[ACCESSORY_GLITTER_POWDER - ACCESSORY_PRETTY_DEWDROP + 1], i, n = 0;
        for (i = ACCESSORY_PRETTY_DEWDROP; i <= ACCESSORY_GLITTER_POWDER; i++)
            if (Sinnoh_CanFitAccessory(i, 1))
                list[n++] = i;
        VarSet(a, n ? list[Random() % n] : 0xFFFF);
        break;
    }
    case FASHCALL_REMOVE:
        Sinnoh_RemoveAccessory(VarGet(a), VarGet(b));
        break;
    case FASHCALL_DRESS_UP_PHOTO:
    case FASHCALL_CONTEST_PHOTO:
        VarSet(b, Sinnoh_PhotoKept(call == FASHCALL_CONTEST_PHOTO, VarGet(a)));
        break;
    case FASHCALL_POFFIN_CAN_COOK:
    {
        bool8 PlayerHasBerries(void);
        VarSet(a, !PlayerHasBerries() ? 1 : FreePokeblockSlots() == 0 ? 2 : 0);
        break;
    }
    case FASHCALL_POFFIN_SPACE:
        VarSet(a, FreePokeblockSlots() != 0);
        break;
    case FASHCALL_POFFIN_FREE:
        VarSet(a, FreePokeblockSlots());
        break;
    case FASHCALL_POFFIN_FLAVOR:
        if (a < 6)
            sPoffin[a] = VarGet(b);
        break;
    case FASHCALL_POFFIN_GIVE:
    {
        // (its color the strongest flavor's: red spicy, blue dry, pink sweet, green bitter, yellow sour)
        static const u8 sColors[5] = {PBLOCK_CLR_RED, PBLOCK_CLR_BLUE, PBLOCK_CLR_PINK, PBLOCK_CLR_GREEN, PBLOCK_CLR_YELLOW};
        struct Pokeblock block;
        u8 i, best = 0;
        for (i = 1; i < 5; i++)
            if (sPoffin[i] > sPoffin[best])
                best = i;
        block.color = sColors[best];
        block.spicy = sPoffin[0];
        block.dry = sPoffin[1];
        block.sweet = sPoffin[2];
        block.bitter = sPoffin[3];
        block.sour = sPoffin[4];
        block.feel = sPoffin[5];
        VarSet(a, AddPokeblock(&block) ? TRUE : FALSE);
        break;
    }
    }
}
