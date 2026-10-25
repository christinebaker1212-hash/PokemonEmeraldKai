// Moemon Co-op: the MOéMON GARDEN's decorations (docs/POKEMON_GARDEN.md 11.3).
//
// Players place earned SECRET BASE decorations: the ones that are figures (dolls, big dolls, cushions:
// DECORPERM_SPRITE, drawn from their object graphics). They come from the placer's own collection and go back
// any time; a decoration out here isn't in the Secret Base or the bedroom at the same time (decoration.c
// counts these). Pokémon use them their own way: dolls and balls for play, cushions for naps; one may grow a
// favorite. No stats, no Friendship: chances for behavior, not a layout to optimize. Friends' decorations
// show while they're here (mp_garden_net.c).
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "mp.h"
#include "mp_garden.h"
#include "mp_life.h"
#include "decoration.h"
#include "decoration_inventory.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "fieldmap.h"
#include "main.h"
#include "metatile_behavior.h"
#include "script.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "task.h"
#include "constants/decorations.h"
#include "constants/event_objects.h"
#include "constants/songs.h"
#include "mp_garden_map.h"

#define REMOTE_DECOR (MP_MAX_PLAYERS * GARDEN_DECOR_MAX)
#define DECOR_MAGIC  0x4744   // "GD"
#define GX(x) ((x) + MAP_OFFSET)
#define GY(y) ((y) + MAP_OFFSET)

bool8 MP_GardenLife_TileToSprite(s16 tx, s16 ty, s16 *x, s16 *y);
bool8 MP_Garden_StoneShown(void);

struct DecorSprite
{
    u8 decor;       // DECOR_* (0 none)
    s16 x, y;       // map tile with MAP_OFFSET
    u8 spriteId;
    u8 gfx;
};

static struct
{
    struct DecorSprite local[GARDEN_DECOR_MAX + 1];   // + the storm stone
    struct DecorSprite remote[REMOTE_DECOR];          // friends' (player id * 16 + index)
    u16 seq;                                          // our list changed (mp_garden_net.c sends it)
} sD;

static void SpriteCB_Decor(struct Sprite *sprite)
{
}

static bool8 IsSpriteDecor(u8 decor)
{
    return decor != DECOR_NONE && decor < NUM_DECORATIONS && gDecorations[decor].permission == DECORPERM_SPRITE;
}

static bool8 SpriteIsOurs(struct DecorSprite *d, u16 tag)
{
    return d->spriteId < MAX_SPRITES && gSprites[d->spriteId].inUse && gSprites[d->spriteId].callback == SpriteCB_Decor
        && gSprites[d->spriteId].data[6] == DECOR_MAGIC && gSprites[d->spriteId].data[7] == (s16)tag;
}

static void Remove(struct DecorSprite *d, u16 tag)
{
    if (SpriteIsOurs(d, tag))
        DestroySprite(&gSprites[d->spriteId]);
    d->spriteId = MAX_SPRITES;
}

static void DrawOne(struct DecorSprite *d, u16 tag)
{
    s16 x, y;
    struct Sprite *sp;
    bool8 big;
    if (d->decor == DECOR_NONE)
    {
        Remove(d, tag);
        return;
    }
    if (!MP_GardenLife_TileToSprite(d->x, d->y, &x, &y))
        return;
    if (!SpriteIsOurs(d, tag))
    {
        u16 gfx = d->decor == 0xFF ? OBJ_EVENT_GFX_PUSHABLE_BOULDER : gDecorations[d->decor].tiles[0];
        d->spriteId = CreateObjectGraphicsSprite(gfx, SpriteCB_Decor, 0, 0, 0);
        if (d->spriteId >= MAX_SPRITES)
        {
            d->spriteId = MAX_SPRITES;
            return;
        }
        gSprites[d->spriteId].data[6] = DECOR_MAGIC;
        gSprites[d->spriteId].data[7] = tag;
        gSprites[d->spriteId].coordOffsetEnabled = TRUE;
    }
    sp = &gSprites[d->spriteId];
    big = d->decor != 0xFF && gDecorations[d->decor].shape != DECORSHAPE_1x1;
    // the tile's bottom edge, like a person standing there (a big doll covers the tile and the ones above / right)
    sp->x = x + (big ? 8 : 0);
    sp->y = y + (big ? 0 : 8);
    sp->oam.priority = gSprites[gObjectEvents[gPlayerAvatar.objectEventId].spriteId].oam.priority;
    SetObjectSubpriorityByElevation(3, sp, 1);
}

static bool8 Covers(struct DecorSprite *d, s16 x, s16 y)
{
    if (d->decor == DECOR_NONE)
        return FALSE;
    if (d->x == x && d->y == y)
        return TRUE;
    // a big doll (2x2) stands on its tile and the ones above / to the right
    return d->decor != 0xFF && gDecorations[d->decor].shape != DECORSHAPE_1x1 && x >= d->x && x <= d->x + 1 && y >= d->y - 1 && y <= d->y;
}

bool8 MP_GardenDecor_At(s16 x, s16 y)
{
    u16 i;
    if (!MP_Garden_IsHere())
        return FALSE;
    for (i = 0; i <= GARDEN_DECOR_MAX; i++)
        if (Covers(&sD.local[i], x, y))
            return TRUE;
    for (i = 0; i < REMOTE_DECOR; i++)
        if (Covers(&sD.remote[i], x, y))
            return TRUE;
    return FALSE;
}

// The nearest decoration of a kind (0: to play with: dolls, big dolls; 1: a cushion), and its tile
s8 MP_GardenDecor_Near(s16 x, s16 y, u8 radius, u8 kind, s16 *dx, s16 *dy)
{
    u16 i;
    int best = radius + 1;
    s8 found = -1;
    for (i = 0; i < GARDEN_DECOR_MAX + REMOTE_DECOR; i++)
    {
        struct DecorSprite *d = i < GARDEN_DECOR_MAX ? &sD.local[i] : &sD.remote[i - GARDEN_DECOR_MAX];
        int dist;
        bool8 cushion;
        if (d->decor == DECOR_NONE || d->decor == 0xFF)
            continue;
        cushion = gDecorations[d->decor].category == DECORCAT_CUSHION;
        if (cushion != (kind == 1))
            continue;
        dist = abs(d->x - x) + abs(d->y - y);
        if (dist < best)
        {
            best = dist;
            found = i < 127 ? (s8)i : -1;
            *dx = d->x;
            *dy = d->y;
        }
    }
    return found;
}

static u8 DecorTypeOf(s8 index)
{
    if (index < 0)
        return DECOR_NONE;
    if (index < GARDEN_DECOR_MAX)
        return sD.local[index].decor;
    if (index - GARDEN_DECOR_MAX < REMOTE_DECOR)
        return sD.remote[index - GARDEN_DECOR_MAX].decor;
    return DECOR_NONE;
}

// It used one: a favorite grows from playing with the same one again and again (docs 11.3)
void MP_GardenDecor_Used(s8 index, struct MpLifeRec *rec)
{
    u8 type = DecorTypeOf(index), count;
    if (type == DECOR_NONE || type == 0xFF || rec == NULL)
        return;
    count = rec->favDecor >> 8;
    if ((rec->favDecor & 0xFF) == type)
    {
        if (count < 255)
            count++;
        if (count == 5)
            MPNet_Log("garden: a favorite decoration (%u)", type);
    }
    else if (count <= 1)
    {
        rec->favDecor = type;
        count = 1;
    }
    else
    {
        count--;
    }
    rec->favDecor = (rec->favDecor & 0xFF) | (count << 8);
}

bool8 MP_GardenDecor_IsFavorite(s8 index, struct MpLifeRec *rec)
{
    u8 type = DecorTypeOf(index);
    return rec && type != DECOR_NONE && (rec->favDecor & 0xFF) == type && (rec->favDecor >> 8) >= 5;
}

void MP_GardenDecor_Update(void)
{
    struct MpGardenSave *gs = MP_Life_GardenSave();
    u16 i;
    for (i = 0; i < GARDEN_DECOR_MAX; i++)
    {
        struct DecorSprite *d = &sD.local[i];
        u8 want = IsSpriteDecor(gs->decor[i].decor) ? gs->decor[i].decor : DECOR_NONE;
        if (d->decor != want || d->x != GX(gs->decor[i].x) || d->y != GY(gs->decor[i].y))
        {
            Remove(d, i);
            d->decor = want;
            d->x = GX(gs->decor[i].x);
            d->y = GY(gs->decor[i].y);
        }
        DrawOne(d, i);
    }
    // the storm stone (a growth mark)
    {
        struct DecorSprite *s = &sD.local[GARDEN_DECOR_MAX];
        u8 want = MP_Garden_StoneShown() ? 0xFF : DECOR_NONE;
        if (s->decor != want)
        {
            Remove(s, GARDEN_DECOR_MAX);
            s->decor = want;
            s->x = GX(GROWTH_STONE_X);
            s->y = GY(GROWTH_STONE_Y);
        }
        DrawOne(s, GARDEN_DECOR_MAX);
    }
    for (i = 0; i < REMOTE_DECOR; i++)
    {
        u8 owner = i / GARDEN_DECOR_MAX;
        if (sD.remote[i].decor != DECOR_NONE && !(gMpPlayers[owner].present && gMpPlayers[owner].hasState
             && gMpPlayers[owner].state.mapGroup == gSaveBlock1Ptr->location.mapGroup && gMpPlayers[owner].state.mapNum == gSaveBlock1Ptr->location.mapNum))
            sD.remote[i].decor = DECOR_NONE;   // they left: theirs go with them
        DrawOne(&sD.remote[i], 100 + i);
    }
}

// mp_garden_net.c: a friend's list (u8 decor, x, y each)
void MP_GardenDecor_SetRemote(u8 owner, const u8 *list, u8 count)
{
    u8 i;
    if (owner >= MP_MAX_PLAYERS)
        return;
    for (i = 0; i < GARDEN_DECOR_MAX; i++)
    {
        struct DecorSprite *d = &sD.remote[owner * GARDEN_DECOR_MAX + i];
        u8 want = i < count && IsSpriteDecor(list[i * 3]) ? list[i * 3] : DECOR_NONE;
        s16 x = i < count ? GX(list[i * 3 + 1]) : 0, y = i < count ? GY(list[i * 3 + 2]) : 0;
        if (d->decor != want || d->x != x || d->y != y)
        {
            if (want != DECOR_NONE)
                MPNet_Log("garden: player %d's decoration %u at %d,%d", owner + 1, want, x - MAP_OFFSET, y - MAP_OFFSET);
            Remove(d, 100 + owner * GARDEN_DECOR_MAX + i);
            d->decor = want;
            d->x = x;
            d->y = y;
        }
    }
}

u8 MP_GardenDecor_PackLocal(u8 *out)
{
    struct MpGardenSave *gs = MP_Life_GardenSave();
    u8 i, n = 0;
    for (i = 0; i < GARDEN_DECOR_MAX; i++)
    {
        if (!IsSpriteDecor(gs->decor[i].decor))
            continue;
        out[n * 3] = gs->decor[i].decor;
        out[n * 3 + 1] = gs->decor[i].x;
        out[n * 3 + 2] = gs->decor[i].y;
        n++;
    }
    return n;
}

u16 MP_GardenDecor_Seq(void)
{
    return sD.seq;
}

// ---------------------------------------------------------------------------
// DECORATE (the GARDEN BOARD)

// How many of this decoration are free to place: owned, minus the Secret Base's, the bedroom's and the Garden's
static u8 FreeCopies(u8 decor)
{
    int n = 0;
    u16 i;
    for (i = 0; i < 40; i++)
        n += gSaveBlock1Ptr->decorationDolls[i] == decor;
    for (i = 0; i < 10; i++)
        n += gSaveBlock1Ptr->decorationCushions[i] == decor;
    for (i = 0; i < DECOR_MAX_SECRET_BASE; i++)
        n -= gSaveBlock1Ptr->secretBases[0].decorations[i] == decor;
    for (i = 0; i < DECOR_MAX_PLAYERS_HOUSE; i++)
        n -= gSaveBlock1Ptr->playerRoomDecorations[i] == decor;
    for (i = 0; i < GARDEN_DECOR_MAX; i++)
        n -= MP_Life_GardenSave()->decor[i].decor == decor;
    return n > 0 ? n : 0;
}

#define DM_ROWS 7

static struct
{
    u8 mode;            // 0 top, 1 place (list), 2 put away (list)
    u16 cursor;
    u16 scroll;
    u8 list[50];
    u8 count;
    char note[48];
} sM;

static void BuildPlaceList(void)
{
    u16 d;
    sM.count = 0;
    for (d = 1; d < NUM_DECORATIONS && sM.count < ARRAY_COUNT(sM.list); d++)
        if (IsSpriteDecor(d) && FreeCopies(d))
            sM.list[sM.count++] = d;
}

static void BuildAwayList(void)
{
    u8 i;
    sM.count = 0;
    for (i = 0; i < GARDEN_DECOR_MAX; i++)
        if (MP_Life_GardenSave()->decor[i].decor != DECOR_NONE)
            sM.list[sM.count++] = i;
}

static void DrawMenu(void)
{
    u8 i;
    if (sM.mode == 0)
    {
        MpUi_Begin("DECORATE");
        MpUi_RowAscii(0, sM.cursor == 0, "PLACE ONE HERE", NULL);
        MpUi_RowAscii(1, sM.cursor == 1, "PUT ONE AWAY", NULL);
        MpUi_RowAscii(2, sM.cursor == 2, "DONE", NULL);
        MpUi_End();
        MpUi_HelpAscii(sM.note[0] ? sM.note : "Dolls and cushions from your SECRET BASE collection.");
        return;
    }
    MpUi_Begin(sM.mode == 1 ? "PLACE ONE HERE" : "PUT ONE AWAY");
    if (sM.cursor < sM.scroll)
        sM.scroll = sM.cursor;
    if (sM.cursor >= sM.scroll + DM_ROWS)
        sM.scroll = sM.cursor - DM_ROWS + 1;
    for (i = 0; i < DM_ROWS && sM.scroll + i <= sM.count; i++)
    {
        u16 row = sM.scroll + i;
        if (row == sM.count)
        {
            MpUi_RowAscii(i, row == sM.cursor, "BACK", NULL);
            continue;
        }
        {
            u8 decor = sM.mode == 1 ? sM.list[row] : MP_Life_GardenSave()->decor[sM.list[row]].decor;
            MpUi_Row(i, row == sM.cursor, gDecorations[decor].name, NULL);
        }
    }
    MpUi_End();
    MpUi_HelpAscii(sM.mode == 1 ? (sM.count ? "{A} PLACE IT IN FRONT OF YOU   {B} BACK" : "No dolls or cushions to place.")
                                : (sM.count ? "{A} PUT IT AWAY   {B} BACK" : "Nothing placed yet."));
}

// Open ground of the meadow (map coords with MAP_OFFSET): not a tree, the pond, the escalator's landing
static bool8 GroundFree(s16 x, s16 y)
{
    s16 mx = x - MAP_OFFSET, my = y - MAP_OFFSET;
    return mx >= GARDEN_X0 && mx <= GARDEN_X1 && my >= GARDEN_Y0 && my <= GARDEN_Y1
        && !(mx <= GARDEN_PAD_X1 && my >= GARDEN_PAD_Y0 && my <= GARDEN_PAD_Y1)
        && MapGridGetCollisionAt(x, y) == 0 && !MetatileBehavior_IsSurfableWaterOrUnderwater(MapGridGetMetatileBehaviorAt(x, y));
}

// On load (and when the Garden grows): a decoration whose place isn't open ground any more (a save from an older
// Garden, a grove grown there) goes back to the collection
void MP_GardenDecor_CheckPlaces(void)
{
    struct MpGardenSave *gs = MP_Life_GardenSave();
    u8 i;
    for (i = 0; i < GARDEN_DECOR_MAX; i++)
    {
        u8 decor = gs->decor[i].decor;
        s16 x = GX(gs->decor[i].x), y = GY(gs->decor[i].y);
        if (decor == DECOR_NONE)
            continue;
        if (GroundFree(x, y) && (gDecorations[decor].shape == DECORSHAPE_1x1
             || (MapGridGetCollisionAt(x + 1, y) == 0 && MapGridGetCollisionAt(x, y - 1) == 0 && MapGridGetCollisionAt(x + 1, y - 1) == 0)))
            continue;
        gs->decor[i].decor = DECOR_NONE;
        sD.seq++;
        MPNet_Log("garden: decoration %u put away (its place is gone)", decor);
    }
}

static bool8 FrontTile(s16 *x, s16 *y)
{
    struct ObjectEvent *me = &gObjectEvents[gPlayerAvatar.objectEventId];
    *x = me->currentCoords.x;
    *y = me->currentCoords.y;
    MoveCoords(me->facingDirection, x, y);
    return GroundFree(*x, *y) && GetObjectEventIdByXY(*x, *y) == OBJECT_EVENTS_COUNT && !MP_Garden_ActorAt(*x, *y);
}

bool8 MP_GardenDecor_Place(u8 decor)
{
    struct MpGardenSave *gs = MP_Life_GardenSave();
    s16 x, y;
    u8 i;
    if (!IsSpriteDecor(decor) || !FreeCopies(decor) || !FrontTile(&x, &y))
        return FALSE;
    if (gDecorations[decor].shape != DECORSHAPE_1x1 && (MapGridGetCollisionAt(x + 1, y) || MapGridGetCollisionAt(x, y - 1)
         || MapGridGetCollisionAt(x + 1, y - 1) || MP_Garden_ActorAt(x + 1, y) || MP_Garden_ActorAt(x, y - 1)))
        return FALSE;
    for (i = 0; i < GARDEN_DECOR_MAX; i++)
        if (gs->decor[i].decor == DECOR_NONE)
        {
            gs->decor[i].decor = decor;
            gs->decor[i].x = x - MAP_OFFSET;
            gs->decor[i].y = y - MAP_OFFSET;
            sD.seq++;
            MPNet_Log("garden: placed decoration %u at %d,%d", decor, x - MAP_OFFSET, y - MAP_OFFSET);
            return TRUE;
        }
    return FALSE;
}

static void Task_Decorate(u8 taskId)
{
    u16 rows = sM.mode == 0 ? 3 : sM.count + 1;
    if (MpUi_MoveCursor(&sM.cursor, rows))
    {
        DrawMenu();
        return;
    }
    if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        if (sM.mode == 0)
        {
            MpUi_Close();
            DestroyTask(taskId);
            ScriptContext_Enable();
            return;
        }
        sM.mode = 0;
        sM.cursor = 0;
        DrawMenu();
        return;
    }
    if (!JOY_NEW(A_BUTTON))
        return;
    sM.note[0] = 0;
    if (sM.mode == 0)
    {
        PlaySE(SE_SELECT);
        if (sM.cursor == 2)
        {
            MpUi_Close();
            DestroyTask(taskId);
            ScriptContext_Enable();
            return;
        }
        sM.mode = sM.cursor + 1;
        sM.cursor = sM.scroll = 0;
        if (sM.mode == 1)
            BuildPlaceList();
        else
            BuildAwayList();
        DrawMenu();
        return;
    }
    if (sM.cursor == sM.count)
    {
        PlaySE(SE_SELECT);
        sM.mode = 0;
        sM.cursor = 0;
        DrawMenu();
        return;
    }
    if (sM.mode == 1)
    {
        if (MP_GardenDecor_Place(sM.list[sM.cursor]))
        {
            PlaySE(SE_SUCCESS);
            MpUi_Close();
            DestroyTask(taskId);
            ScriptContext_Enable();
            return;
        }
        PlaySE(SE_FAILURE);
        snprintf(sM.note, sizeof(sM.note), "Face a free spot of the GARDEN first.");
        sM.mode = 0;
        sM.cursor = 0;
    }
    else
    {
        struct MpGardenSave *gs = MP_Life_GardenSave();
        MPNet_Log("garden: put away decoration %u", gs->decor[sM.list[sM.cursor]].decor);
        gs->decor[sM.list[sM.cursor]].decor = DECOR_NONE;
        sD.seq++;
        PlaySE(SE_SELECT);
        BuildAwayList();
        if (sM.cursor > sM.count)
            sM.cursor = sM.count;
    }
    DrawMenu();
}

void MP_Garden_OpenDecorate(void)
{
    memset(&sM, 0, sizeof(sM));
    PlaySE(SE_WIN_OPEN);
    MpUi_Open();
    CreateTask(Task_Decorate, 80);
    DrawMenu();
}

void MP_GardenDecor_Debug(const char *args)
{
    char cmd[32] = "";
    int a = 0;
    sscanf(args, "%31s %d", cmd, &a);
    if (strcmp(cmd, "give") == 0)
    {
        // give DECOR: one more in the collection
        DecorationAdd(a);
        printf("[test] garden decor give %d\n", a);
    }
    else if (strcmp(cmd, "place") == 0)
    {
        printf("[test] garden decor place %d: %d\n", a, MP_GardenDecor_Place(a));
    }
    else if (strcmp(cmd, "list") == 0)
    {
        u8 i;
        for (i = 0; i < GARDEN_DECOR_MAX; i++)
            if (MP_Life_GardenSave()->decor[i].decor)
                printf("[test] garden decor %u: %u at %u,%u\n", i, MP_Life_GardenSave()->decor[i].decor,
                       MP_Life_GardenSave()->decor[i].x, MP_Life_GardenSave()->decor[i].y);
    }
}
