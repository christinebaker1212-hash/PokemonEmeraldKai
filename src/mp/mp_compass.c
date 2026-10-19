// Moemon Co-op: "where is everyone?" Little arrows at the edge of the screen
// point to friends you can't see: straight at them on the same or a connected
// map, or in their direction on the Town Map when they're further away.
// One color per player.
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "mp.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "main.h"
#include "region_map.h"
#include "script.h"
#include "sprite.h"
#include "constants/region_map_sections.h"
#include "constants/rgb.h"

#define TAG_COMPASS     0x5490   // + player
#define TAG_COMPASS_PAL 0x5498
#define SCREEN_CX       120
#define SCREEN_CY       80
#define EDGE_X          108      // how far from the center the arrows sit
#define EDGE_Y          66

// Player colors: 1 red, 2 blue, 3 green, 4 yellow; 5 outline
static const u16 sPal[16] = {
    0, RGB(31, 8, 8), RGB(8, 16, 31), RGB(6, 26, 6), RGB(31, 27, 4), RGB(2, 2, 4),
};

static const struct OamData sOam = {
    .shape = SPRITE_SHAPE(8x8),
    .size = SPRITE_SIZE(8x8),
    .priority = 1,
};

// 8 directions, starting east, clockwise (screen y grows downward)
static const s8 sDirs[8][2] = {{1, 0}, {1, 1}, {0, 1}, {-1, 1}, {-1, 0}, {-1, -1}, {0, -1}, {1, -1}};

static u8 sTiles[MP_MAX_PLAYERS][8][32];
static bool8 sBuilt;
static u8 sSprites[MP_MAX_PLAYERS];

static const union AnimCmd sAnim0[] = {ANIMCMD_FRAME(0, 8), ANIMCMD_END};
static const union AnimCmd sAnim1[] = {ANIMCMD_FRAME(1, 8), ANIMCMD_END};
static const union AnimCmd sAnim2[] = {ANIMCMD_FRAME(2, 8), ANIMCMD_END};
static const union AnimCmd sAnim3[] = {ANIMCMD_FRAME(3, 8), ANIMCMD_END};
static const union AnimCmd sAnim4[] = {ANIMCMD_FRAME(4, 8), ANIMCMD_END};
static const union AnimCmd sAnim5[] = {ANIMCMD_FRAME(5, 8), ANIMCMD_END};
static const union AnimCmd sAnim6[] = {ANIMCMD_FRAME(6, 8), ANIMCMD_END};
static const union AnimCmd sAnim7[] = {ANIMCMD_FRAME(7, 8), ANIMCMD_END};
static const union AnimCmd *const sAnims[] = {sAnim0, sAnim1, sAnim2, sAnim3, sAnim4, sAnim5, sAnim6, sAnim7};

static void Put(u8 *tile, int x, int y, u8 c)
{
    u8 *b;
    if (x < 0 || x > 7 || y < 0 || y > 7)
        return;
    b = &tile[y * 4 + x / 2];
    *b = (x & 1) ? ((*b & 0x0F) | (c << 4)) : ((*b & 0xF0) | c);
}

// A small arrow head pointing along (dx, dy), outlined
static void BuildArrow(u8 *tile, int dx, int dy, u8 color)
{
    int x, y;
    memset(tile, 0, 32);
    for (y = 0; y < 8; y++)
        for (x = 0; x < 8; x++)
        {
            // Coordinates relative to the center, 2x for half pixels
            int px = 2 * x - 7, py = 2 * y - 7;
            int along = px * dx + py * dy;                    // toward the tip
            int across = abs(px * -dy + py * dx);             // away from the axis
            int len = (dx && dy) ? 7 : 5;                     // diagonals are longer
            if (along >= -len && along <= len && across * 2 <= (len - along) + 1)
                Put(tile, x, y, color);
        }
    // Outline: any empty pixel next to a filled one
    {
        u8 copy[32];
        memcpy(copy, tile, 32);
        for (y = 0; y < 8; y++)
            for (x = 0; x < 8; x++)
            {
                int k, filled = 0;
                u8 c = (copy[y * 4 + x / 2] >> ((x & 1) * 4)) & 15;
                if (c)
                    continue;
                for (k = 0; k < 4; k++)
                {
                    static const s8 n[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
                    int nx = x + n[k][0], ny = y + n[k][1];
                    if (nx >= 0 && nx < 8 && ny >= 0 && ny < 8 && ((copy[ny * 4 + nx / 2] >> ((nx & 1) * 4)) & 15))
                        filled = 1;
                }
                if (filled)
                    Put(tile, x, y, 5);
            }
    }
}

static void BuildAll(void)
{
    u8 p, d;
    for (p = 0; p < MP_MAX_PLAYERS; p++)
        for (d = 0; d < 8; d++)
            BuildArrow(sTiles[p][d], sDirs[d][0], sDirs[d][1], 1 + (p % 4));
    sBuilt = TRUE;
}

static void SpriteCB_Compass(struct Sprite *sprite)
{
}

static bool8 SpriteValid(u8 p)
{
    u8 s = sSprites[p];
    return s < MAX_SPRITES && gSprites[s].inUse && gSprites[s].callback == SpriteCB_Compass && gSprites[s].data[0] == p + 1;
}

static void Hide(u8 p)
{
    if (SpriteValid(p))
    {
        DestroySprite(&gSprites[sSprites[p]]);
        FreeSpriteTilesByTag(TAG_COMPASS + p);
    }
    sSprites[p] = MAX_SPRITES;
}

static bool8 Show(u8 p, s16 x, s16 y, u8 dir)
{
    if (!SpriteValid(p))
    {
        struct SpriteSheet sheet = {sTiles[p], sizeof(sTiles[p]), TAG_COMPASS + p};
        struct SpritePalette pal = {sPal, TAG_COMPASS_PAL};
        struct SpriteTemplate t = {
            .tileTag = TAG_COMPASS + p, .paletteTag = TAG_COMPASS_PAL, .oam = &sOam,
            .anims = sAnims, .images = NULL, .affineAnims = gDummySpriteAffineAnimTable, .callback = SpriteCB_Compass,
        };
        if (IndexOfSpritePaletteTag(TAG_COMPASS_PAL) == 0xFF && LoadSpritePalette(&pal) == 0xFF)
            return FALSE;
        if (GetSpriteTileStartByTag(TAG_COMPASS + p) == 0xFFFF && LoadSpriteSheet(&sheet) == 0)
            return FALSE;
        sSprites[p] = CreateSprite(&t, x, y, 0);
        if (sSprites[p] == MAX_SPRITES)
            return FALSE;
        gSprites[sSprites[p]].data[0] = p + 1;
    }
    gSprites[sSprites[p]].x = x;
    gSprites[sSprites[p]].y = y;
    if (gSprites[sSprites[p]].animNum != dir)
        StartSpriteAnim(&gSprites[sSprites[p]], dir);
    return TRUE;
}

static u8 DirFor(s32 dx, s32 dy)
{
    // tan(22.5) ~ 0.414 ~ 5/12
    s32 ax = abs(dx), ay = abs(dy);
    if (ay * 12 <= ax * 5)
        return dx > 0 ? 0 : 4;
    if (ax * 12 <= ay * 5)
        return dy > 0 ? 2 : 6;
    if (dx > 0)
        return dy > 0 ? 1 : 7;
    return dy > 0 ? 3 : 5;
}

// Where the friend is, in tiles from you; FALSE when there's no sensible direction
static bool8 FriendOffset(u8 id, s32 *dx, s32 *dy, bool8 *onScreen)
{
    struct ObjectEvent *me = &gObjectEvents[gPlayerAvatar.objectEventId];
    s16 rx, ry;
    *onScreen = FALSE;
    if (MP_GetRemoteMapPos(id, &rx, &ry))
    {
        *dx = rx - me->currentCoords.x;
        *dy = ry - me->currentCoords.y;
        *onScreen = abs(*dx) <= 7 && abs(*dy) <= 5;
        return TRUE;
    }
    {
        // Further away: compare places on the Town Map
        u16 theirs = MP_Friends_Mapsec(id), mine = gMapHeader.regionMapSectionId;
        if (theirs >= MAPSEC_NONE || mine >= MAPSEC_NONE || theirs == mine)
            return FALSE;
        *dx = (gRegionMapEntries[theirs].x * 2 + gRegionMapEntries[theirs].width)
            - (gRegionMapEntries[mine].x * 2 + gRegionMapEntries[mine].width);
        *dy = (gRegionMapEntries[theirs].y * 2 + gRegionMapEntries[theirs].height)
            - (gRegionMapEntries[mine].y * 2 + gRegionMapEntries[mine].height);
        return *dx != 0 || *dy != 0;
    }
}

// Every overworld frame, after the camera moved
void MP_Compass_Update(void)
{
    u8 p;
    if (!sBuilt)
    {
        BuildAll();
        for (p = 0; p < MP_MAX_PLAYERS; p++)
            sSprites[p] = MAX_SPRITES;
    }
    for (p = 0; p < MP_MAX_PLAYERS; p++)
    {
        s32 dx, dy;
        bool8 onScreen;
        if (!MP_IsConnected() || !MP_IsInOverworld() || p == gMpLocalId || !gMpPlayers[p].present
         || !gMpPlayers[p].hasState || (!(gMpPlayers[p].state.flags & MPF_IN_OVERWORLD) && !(gMpPlayers[p].state.flags & MPF_IN_BATTLE))
         || !FriendOffset(p, &dx, &dy, &onScreen) || onScreen || ArePlayerFieldControlsLocked())
        {
            Hide(p);
            continue;
        }
        {
            // Put it where the line from the center toward them leaves the screen
            s32 x, y;
            if (abs(dx) * EDGE_Y > abs(dy) * EDGE_X)
            {
                x = SCREEN_CX + (dx > 0 ? EDGE_X : -EDGE_X);
                y = SCREEN_CY + dy * EDGE_X / abs(dx);
            }
            else
            {
                y = SCREEN_CY + (dy > 0 ? EDGE_Y : -EDGE_Y);
                x = SCREEN_CX + dx * EDGE_Y / abs(dy);
            }
            if (!Show(p, x, y, DirFor(dx, dy)))
                Hide(p);
        }
    }
}
