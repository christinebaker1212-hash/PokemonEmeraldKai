// Moemon Co-op: "where is everyone?" Arrows at the edge of the screen point to friends you can't
// see, at their exact angle, in their player's color, with their name and how many steps away they
// are. Drawn by the port over the finished frame (MP_Compass_Composite, from VDraw), so they take
// none of the GBA's sprites or palettes.
//
// Where a friend is: one world grid for all of Hoenn's outdoor maps, built once by walking the
// map connections out from Littleroot; a friend indoors (a house, a cave, a POKéMON CENTER's 2F)
// is at the door that leads out of their building (found by following the map's warps). Only a
// place none of that reaches (underwater, the islands) falls back to the Town Map's areas.
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "mp.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "fieldmap.h"
#include "main.h"
#include "overworld.h"
#include "palette.h"
#include "region_map.h"
#include "script.h"
#include "string_util.h"
#include "text.h"
#include "constants/map_groups.h"
#include "constants/maps.h"
#include "constants/region_map_sections.h"
#include "constants/rgb.h"

u8 MP_RenderSmallGlyph(u16 glyphId, u8 fgColor, u8 bgColor, u8 shadowColor);

#define SCREEN_CX 120
#define SCREEN_CY 80
#define EDGE_X    110       // where the arrows sit, from the center
#define EDGE_Y    70
#define PING_INSET 11       // a ping sits a little inside its friend's arrow
#define LABEL_W   64
#define LABEL_H   10

// Player colors (the name tags' and the old arrows': red, blue, green, yellow)
static const u16 sColors[MP_MAX_PLAYERS] = {RGB(31, 8, 8), RGB(8, 16, 31), RGB(6, 26, 6), RGB(31, 27, 4)};
#define OUTLINE RGB(2, 2, 4)

u16 MP_Compass_PlayerColor(u8 player)
{
    return sColors[player % MP_MAX_PLAYERS];
}

// ---------------------------------------------------------------------------
// The world grid

enum
{
    W_UNKNOWN,
    W_PLACED,      // an outdoor map: x, y = where its (0, 0) is
    W_DOOR,        // indoors: x, y = the world tile of the door out
    W_NOWHERE,     // neither
};

struct WorldMap
{
    s16 x, y;
    u8 state;
};

static struct WorldMap sWorld[MAP_GROUPS_COUNT][256];
static bool8 sWorldBuilt;

static const struct MapHeader *Header(u8 group, u8 num)
{
    return group < MAP_GROUPS_COUNT ? Overworld_GetMapHeaderByGroupAndId(group, num) : NULL;
}

static void BuildWorld(void)
{
    // A breadth-first walk over the connections; a map keeps the first place it's reached at
    static u16 queue[MAP_GROUPS_COUNT * 64];
    u16 head = 0, tail = 0;
    memset(sWorld, 0, sizeof(sWorld));
    sWorld[MAP_GROUP(MAP_LITTLEROOT_TOWN)][MAP_NUM(MAP_LITTLEROOT_TOWN)].state = W_PLACED;
    queue[tail++] = MAP_LITTLEROOT_TOWN;
    while (head < tail)
    {
        u8 group = queue[head] >> 8, num = queue[head] & 0xFF, i;
        const struct MapHeader *h = Header(group, num);
        struct WorldMap *w = &sWorld[group][num];
        head++;
        if (h == NULL || h->connections == NULL)
            continue;
        for (i = 0; i < h->connections->count; i++)
        {
            const struct MapConnection *c = &h->connections->connections[i];
            const struct MapHeader *n = Header(c->mapGroup, c->mapNum);
            struct WorldMap *nw;
            s32 offset = (s32)(s16)c->offset;
            if (n == NULL || c->mapGroup >= MAP_GROUPS_COUNT)
                continue;
            nw = &sWorld[c->mapGroup][c->mapNum];
            if (nw->state != W_UNKNOWN)
                continue;
            switch (c->direction)
            {
            case CONNECTION_NORTH: nw->x = w->x + offset; nw->y = w->y - n->mapLayout->height; break;
            case CONNECTION_SOUTH: nw->x = w->x + offset; nw->y = w->y + h->mapLayout->height; break;
            case CONNECTION_WEST:  nw->x = w->x - n->mapLayout->width; nw->y = w->y + offset; break;
            case CONNECTION_EAST:  nw->x = w->x + h->mapLayout->width; nw->y = w->y + offset; break;
            default: continue;   // (dive / emerge: another layer)
            }
            nw->state = W_PLACED;
            if (tail < ARRAY_COUNT(queue))
                queue[tail++] = (c->mapGroup << 8) | c->mapNum;
        }
    }
    sWorldBuilt = TRUE;
}

// The world tile of map-local (x, y) (MAP_OFFSET included, as object coords are), FALSE when
// the map is nowhere on the grid. An indoor map gives its door out (*indoors set).
static bool8 WorldPos(u8 group, u8 num, s16 x, s16 y, s32 *wx, s32 *wy, bool8 *indoors)
{
    struct WorldMap *w;
    if (group >= MAP_GROUPS_COUNT)
        return FALSE;
    if (!sWorldBuilt)
        BuildWorld();
    w = &sWorld[group][num];
    if (w->state == W_UNKNOWN)
    {
        // Indoors: the nearest door out, by the fewest warps (POKéMON CENTER 2F -> 1F -> town)
        static u16 queue[64];
        u16 head = 0, tail = 0;
        w->state = W_NOWHERE;
        queue[tail++] = (group << 8) | num;
        while (head < tail && w->state == W_NOWHERE)
        {
            u8 g = queue[head] >> 8, n = queue[head] & 0xFF, i;
            const struct MapHeader *h = Header(g, n);
            head++;
            if (h == NULL || h->events == NULL)
                continue;
            for (i = 0; i < h->events->warpCount; i++)
            {
                const struct WarpEvent *e = &h->events->warps[i];
                const struct MapHeader *d = Header(e->mapGroup, e->mapNum);
                u16 key = (e->mapGroup << 8) | e->mapNum, k;
                if (d == NULL || e->mapGroup >= MAP_GROUPS_COUNT || d->events == NULL)
                    continue;   // (a dynamic warp: wherever the player came from)
                if (sWorld[e->mapGroup][e->mapNum].state == W_PLACED && e->warpId < d->events->warpCount)
                {
                    const struct WarpEvent *door = &d->events->warps[e->warpId];
                    w->x = sWorld[e->mapGroup][e->mapNum].x + door->x;
                    w->y = sWorld[e->mapGroup][e->mapNum].y + door->y;
                    w->state = W_DOOR;
                    break;
                }
                for (k = 0; k < tail && queue[k] != key; k++)
                    ;
                if (k == tail && tail < ARRAY_COUNT(queue))
                    queue[tail++] = key;
            }
        }
    }
    *indoors = w->state == W_DOOR;
    switch (w->state)
    {
    case W_PLACED:
        *wx = w->x + x - MAP_OFFSET;
        *wy = w->y + y - MAP_OFFSET;
        return TRUE;
    case W_DOOR:
        *wx = w->x;
        *wy = w->y;
        return TRUE;
    }
    return FALSE;
}

// ---------------------------------------------------------------------------
// The marks: a friend's arrow, or a ping's (blinking, a little inside)

struct Mark
{
    bool8 on;
    bool8 ping;
    u8 color;          // player
    s16 x, y;          // the arrow's center on the screen
    s16 dx, dy;        // toward them (any length)
    u8 label[LABEL_H][LABEL_W];   // 0 none, 1 text, 2 shadow
    u8 labelW;
    char key[24];      // what the label says now (redrawn when it changes)
};

static struct Mark sMarks[MP_MAX_PLAYERS];
static struct Mark sPings[MP_MAX_PLAYERS];
static u8 sFresh;      // frames the marks are good for (set by MP_Compass_Update)

static void LabelPut(struct Mark *m, int x, int y, u8 c)
{
    if (x >= 0 && x < LABEL_W && y >= 0 && y < LABEL_H && (c == 1 || m->label[y][x] == 0))
        m->label[y][x] = c;
}

// The game's small font into the label (name, then the steps)
static void BuildLabel(struct Mark *m, const u8 *name, s32 steps)
{
    u8 text[24], *end;
    int i, x = 1;
    char key[24];
    end = StringCopy(text, name);
    if (steps >= 0)
    {
        *end++ = CHAR_SPACE;
        ConvertIntToDecimalStringN(end, steps > 9999 ? 9999 : steps, STR_CONV_MODE_LEFT_ALIGN, 4);
    }
    for (i = 0; text[i] != EOS && i < 23; i++)
        key[i] = (char)(text[i] + 1);   // (the game's space is 0)
    key[i] = 0;
    if (!strcmp(key, m->key))
        return;
    strcpy(m->key, key);
    memset(m->label, 0, sizeof(m->label));
    for (i = 0; text[i] != EOS && x < LABEL_W - 4; i++)
    {
        int w, gx, gy;
        if (text[i] == CHAR_SPACE)
        {
            x += 3;
            continue;
        }
        w = MP_RenderSmallGlyph(text[i], 1, 0, 2);
        for (gy = 0; gy < 16; gy++)
        {
            const u32 *rows = (gy < 8) ? gCurGlyph.gfxBufferTop : gCurGlyph.gfxBufferBottom;
            for (gx = 0; gx < w && gx < 16; gx++)
            {
                u8 c = (rows[(gx < 8 ? 0 : 8) + (gy % 8)] >> ((gx % 8) * 4)) & 0xF;
                // (the small font's glyphs sit 2 rows down in their 16)
                if (c == 1 || c == 2)
                    LabelPut(m, x + gx, gy - 1, c);
            }
        }
        x += w;
    }
    m->labelW = x + 1 > LABEL_W ? LABEL_W : x + 1;
}

// Where the line from the center toward (dx, dy) leaves the screen, `inset` pixels in
static void EdgePoint(s32 dx, s32 dy, s32 inset, s16 *x, s16 *y)
{
    s32 ex = EDGE_X - inset, ey = EDGE_Y - inset;
    if (abs(dx) * ey > abs(dy) * ex)
    {
        *x = SCREEN_CX + (dx > 0 ? ex : -ex);
        *y = SCREEN_CY + dy * ex / abs(dx);
    }
    else
    {
        *y = SCREEN_CY + (dy > 0 ? ey : -ey);
        *x = SCREEN_CX + dx * ey / abs(dy);
    }
}

// Where the friend is from you, in tiles; *steps = -1 when only the direction is known
static bool8 FriendOffset(u8 id, s32 *dx, s32 *dy, s32 *steps, bool8 *onScreen)
{
    struct ObjectEvent *me = &gObjectEvents[gPlayerAvatar.objectEventId];
    struct MpPlayer *p = &gMpPlayers[id];
    s16 rx, ry;
    s32 mx, my, fx, fy;
    bool8 meIn, themIn;
    *onScreen = FALSE;
    *steps = -1;
    if (MP_GetRemoteMapPos(id, &rx, &ry))
    {
        // Same or a connected map: exact
        *dx = rx - me->currentCoords.x;
        *dy = ry - me->currentCoords.y;
        *onScreen = abs(*dx) <= 7 && abs(*dy) <= 5;
        *steps = abs(*dx) + abs(*dy);
        return TRUE;
    }
    if (WorldPos(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum, me->currentCoords.x,
                 me->currentCoords.y, &mx, &my, &meIn)
     && WorldPos(p->state.mapGroup, p->state.mapNum, p->state.x, p->state.y, &fx, &fy, &themIn))
    {
        *dx = fx - mx;
        *dy = fy - my;
        *steps = abs(*dx) + abs(*dy);
        // (their door in view: its friend tag shows them)
        *onScreen = !meIn && abs(*dx) <= 7 && abs(*dy) <= 5;
        // (two floors of one building: no direction to show)
        return *dx != 0 || *dy != 0;
    }
    {
        // Nowhere on the grid: compare places on the Town Map
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

void MP_Compass_PointAt(u8 slot, u8 colorPlayer, s32 dx, s32 dy, bool8 show)
{
    struct Mark *m;
    if (slot >= MP_MAX_PLAYERS || colorPlayer >= MP_MAX_PLAYERS)
        return;
    m = &sPings[slot];
    if (!show || (dx == 0 && dy == 0) || ArePlayerFieldControlsLocked())
    {
        m->on = FALSE;
        return;
    }
    m->on = TRUE;
    m->ping = TRUE;
    m->color = colorPlayer;
    m->dx = dx;
    m->dy = dy;
    m->labelW = 0;
    EdgePoint(dx, dy, PING_INSET, &m->x, &m->y);
}

bool8 MP_Compass_AnyShown(void)
{
    u8 p;
    for (p = 0; p < MP_MAX_PLAYERS; p++)
        if (sMarks[p].on || sPings[p].on)
            return TRUE;
    return FALSE;
}

// Every overworld frame, after the camera moved
void MP_Compass_Update(void)
{
    u8 p;
    sFresh = 2;
    for (p = 0; p < MP_MAX_PLAYERS; p++)
    {
        struct Mark *m = &sMarks[p];
        s32 dx, dy, steps;
        bool8 onScreen;
        if (!MP_IsConnected() || !MP_IsInOverworld() || p == gMpLocalId || !gMpPlayers[p].present
         || !gMpPlayers[p].hasState || !(gMpPlayers[p].state.flags & (MPF_IN_OVERWORLD | MPF_IN_BATTLE | MPF_IN_MENU))
         || !FriendOffset(p, &dx, &dy, &steps, &onScreen) || onScreen || ArePlayerFieldControlsLocked())
        {
            m->on = FALSE;
            continue;
        }
        m->on = TRUE;
        m->ping = FALSE;
        m->color = p;
        // (in tiles, so the angle is the true one; the screen's 16-pixel tiles keep it)
        if (m->dx != dx || m->dy != dy)
            MPNet_Log("compass: player %d at %d,%d (%d steps)", p, (int)dx, (int)dy, (int)steps);
        m->dx = dx;
        m->dy = dy;
        EdgePoint(dx, dy, 0, &m->x, &m->y);
        BuildLabel(m, gMpPlayers[p].name, steps);
    }
}

// ---------------------------------------------------------------------------
// Drawing

static inline u16 Mix(u16 a, u16 b, s32 k16)   // k/16 of b over a
{
    s32 r, g, bl;
    if (k16 <= 0)
        return a;
    if (k16 >= 16)
        return b | 0x8000;
    r = ((a & 31) * (16 - k16) + (b & 31) * k16) / 16;
    g = (((a >> 5) & 31) * (16 - k16) + ((b >> 5) & 31) * k16) / 16;
    bl = (((a >> 10) & 31) * (16 - k16) + ((b >> 10) & 31) * k16) / 16;
    return r | (g << 5) | (bl << 10) | 0x8000;
}

// A rounded arrow head along (ux, uy) (unit, 1/256ths), centered at (cx, cy): filled with the
// player's color, a dark outline, soft at the edges
static void DrawArrow(u16 *frame, s32 cx, s32 cy, s32 ux, s32 uy, u16 color, s32 scale)
{
    // the triangle: tip, and two back corners, in 1/16 pixels
    s32 vx = -uy, vy = ux;
    s32 px[3], py[3], x, y, i, wind;
    s32 len = 9 * scale, back = 6 * scale, half = 7 * scale;   // (scale in 1/16)
    px[0] = cx * 16 + ux * len / 256;               py[0] = cy * 16 + uy * len / 256;
    px[1] = cx * 16 - ux * back / 256 + vx * half / 256; py[1] = cy * 16 - uy * back / 256 + vy * half / 256;
    px[2] = cx * 16 - ux * back / 256 - vx * half / 256; py[2] = cy * 16 - uy * back / 256 - vy * half / 256;
    wind = ((px[1] - px[0]) * (py[2] - py[0]) - (py[1] - py[0]) * (px[2] - px[0])) > 0 ? 1 : -1;
    for (y = cy - 12; y <= cy + 12; y++)
    {
        if (y < 0 || y >= DISPLAY_HEIGHT)
            continue;
        for (x = cx - 12; x <= cx + 12; x++)
        {
            s32 d = 0x7FFFFFFF, sx = x * 16 + 8, sy = y * 16 + 8;
            u16 *dst;
            if (x < 0 || x >= DISPLAY_WIDTH)
                continue;
            // the signed distance to the triangle's nearest edge (1/16 pixels, > 0 inside, whichever
            // way the corners wind)
            for (i = 0; i < 3; i++)
            {
                s32 ax = px[i], ay = py[i], bx = px[(i + 1) % 3], by = py[(i + 1) % 3];
                s32 ex = bx - ax, ey = by - ay;
                s32 elen = abs(ex) > abs(ey) ? abs(ex) + abs(ey) / 2 : abs(ey) + abs(ex) / 2;   // ~|e|
                s32 cross = ex * (sy - ay) - ey * (sx - ax);
                s32 e = elen ? cross * wind / elen : 0;
                if (e < d)
                    d = e;
            }
            dst = &frame[y * DISPLAY_WIDTH + x];
            if (d >= 16)
                *dst = color | 0x8000;
            else if (d >= 0)
                *dst = Mix(OUTLINE, color, 4 + d * 12 / 16);   // the inner edge: outline into fill
            else if (d >= -16)
                *dst = Mix(*dst, OUTLINE, d >= -10 ? 14 : (d + 16) * 2);
        }
    }
}

static void DrawLabel(u16 *frame, const struct Mark *m, s32 ax, s32 ay)
{
    // inside the screen from the arrow, toward the center
    s32 w = m->labelW, h = LABEL_H, x, y, lx, ly;
    s32 tx = SCREEN_CX - ax, ty = SCREEN_CY - ay;
    s32 tl = abs(tx) + abs(ty);
    if (!w)
        return;
    // beside the arrow, on its inner side: to the right of a west arrow, under a north one...
    if (abs(tx) * EDGE_Y >= abs(ty) * EDGE_X)
    {
        lx = tx > 0 ? ax + 10 : ax - 10 - w;
        ly = ay - h / 2;
    }
    else
    {
        lx = ax - w / 2;
        ly = ty > 0 ? ay + 10 : ay - 10 - h;
    }
    (void)tl;
    if (lx < 2)
        lx = 2;
    if (lx + w > DISPLAY_WIDTH - 2)
        lx = DISPLAY_WIDTH - 2 - w;
    if (ly < 2)
        ly = 2;
    if (ly + h > DISPLAY_HEIGHT - 2)
        ly = DISPLAY_HEIGHT - 2 - h;
    for (y = -1; y <= h; y++)
        for (x = -1; x <= w; x++)
        {
            u16 *dst = &frame[(ly + y) * DISPLAY_WIDTH + lx + x];
            bool8 corner = (x == -1 || x == w) && (y == -1 || y == h);
            if (corner)
                continue;
            // a dark pill, a stripe of the player's color down the left
            *dst = x <= 0 ? Mix(*dst, sColors[m->color], 13) : Mix(*dst, RGB(2, 2, 5), 11);
            if (x >= 0 && x < w && y >= 0 && y < h && m->label[y][x])
                *dst = m->label[y][x] == 1 ? RGB(31, 31, 31) | 0x8000 : Mix(*dst, 0, 12);
        }
}

static void DrawMark(u16 *frame, const struct Mark *m)
{
    s32 len = abs(m->dx) > abs(m->dy) ? abs(m->dx) + abs(m->dy) * 3 / 8 : abs(m->dy) + abs(m->dx) * 3 / 8;
    s32 ux, uy, bob, ax, ay;
    if (!len)
        return;
    ux = m->dx * 256 / len;
    uy = m->dy * 256 / len;
    // a slow nudge toward them
    bob = (gMain.vblankCounter1 / 6) % 8;
    bob = bob < 4 ? bob : 8 - bob;
    ax = m->x + ux * (bob - 2) / 512;
    ay = m->y + uy * (bob - 2) / 512;
    DrawArrow(frame, ax, ay, ux, uy, sColors[m->color], m->ping ? 12 : 16);
    if (!m->ping)
        DrawLabel(frame, m, m->x, m->y);
}

void MP_Compass_Composite(u16 *frame)
{
    u8 p;
    if (!sFresh)
        return;
    sFresh--;
    if (gPaletteFade.active || gMain.callback2 != CB2_Overworld || !MP_OBJ_LAYER_SHOWN())
        return;
    for (p = 0; p < MP_MAX_PLAYERS; p++)
        if (sMarks[p].on)
            DrawMark(frame, &sMarks[p]);
    for (p = 0; p < MP_MAX_PLAYERS; p++)
        if (sPings[p].on && ((gMain.vblankCounter1 / 10) & 1))   // blinks: a ping, not a friend
            DrawMark(frame, &sPings[p]);
}
