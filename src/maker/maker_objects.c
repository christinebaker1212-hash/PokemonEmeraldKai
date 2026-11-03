// Moemon Maker: a map's people, signs, triggers and warps for the editor (include/maker_api.h), from the map's own
// compiled events, named by map.json (tools/maker/gen_names.py: a person's LOCALID_, look and script), and their
// sprites as the field draws them (GetObjectEventGraphicsInfo, the look's palette by the field's own slot rules).
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "global.fieldmap.h"
#include "overworld.h"
#include "event_object_movement.h"
#include "constants/event_object_movement.h"
#include "maker.h"
#include "maker_api.h"

const u16 *ObjectEvent_MakerPalette(u16 graphicsId);   // event_object_movement.c
extern const u8 gInitialMovementTypeFacingDirections[];

static const struct MapHeader *Header(int index)
{
    if (index < 0 || index >= Maker_MapCount())
        return NULL;
    return Maker_BaseMapHeader(Maker_Map(index)->mapGroup, Maker_Map(index)->mapNum);
}

static const struct MakerMapEventNames *Names(int index)
{
    return Maker_MapEventNames(index);
}

int MakerApi_ObjectCount(int map)
{
    const struct MapHeader *h = Header(map);
    return h && h->events ? h->events->objectEventCount : 0;
}

int MakerApi_ObjectInfo(int map, int i, struct MakerObjectInfo *o)
{
    const struct MapHeader *h = Header(map);
    const struct MakerMapEventNames *n = Names(map);
    const struct ObjectEventTemplate *t;
    memset(o, 0, sizeof(*o));
    if (h == NULL || h->events == NULL || i < 0 || i >= h->events->objectEventCount)
        return 0;
    t = &h->events->objectEvents[i];
    o->localId = t->localId;
    o->graphicsId = t->graphicsIdLo | (t->graphicsIdHi << 8);
    o->kind = t->kind;
    o->x = t->x;
    o->y = t->y;
    o->elevation = t->elevation;
    o->movementType = t->movementType;
    o->rangeX = t->movementRangeX;
    o->rangeY = t->movementRangeY;
    o->trainerType = t->trainerType;
    o->sight = t->trainerRange_berryTreeId;
    o->flag = t->flagId;
    {
        // (the way it faces when the map loads: DIR_SOUTH 1, NORTH 2, WEST 3, EAST 4 -> 0-3)
        u8 d = gInitialMovementTypeFacingDirections[t->movementType];
        o->facing = d >= 1 && d <= 4 ? d - 1 : 0;
    }
    o->localIdName = o->gfxName = o->scriptName = "";
    if (n && i < n->objCount)
    {
        o->localIdName = Maker_ObjName(n->obj + i)->localId;
        o->gfxName = Maker_ObjName(n->obj + i)->gfx;
        o->scriptName = Maker_ObjName(n->obj + i)->script;
    }
    return 1;
}

int MakerApi_WarpCount(int map)
{
    const struct MapHeader *h = Header(map);
    return h && h->events ? h->events->warpCount : 0;
}

int MakerApi_WarpInfo(int map, int i, struct MakerWarpInfo *w)
{
    const struct MapHeader *h = Header(map);
    const struct WarpEvent *e;
    int d;
    memset(w, 0, sizeof(*w));
    if (h == NULL || h->events == NULL || i < 0 || i >= h->events->warpCount)
        return 0;
    e = &h->events->warps[i];
    w->x = e->x;
    w->y = e->y;
    w->elevation = e->elevation;
    w->destWarp = e->warpId;
    w->destMap = -1;
    for (d = 0; d < Maker_MapCount(); d++)
        if (Maker_Map(d)->mapGroup == e->mapGroup && Maker_Map(d)->mapNum == e->mapNum)
        {
            w->destMap = d;
            break;
        }
    return 1;
}

int MakerApi_BgCount(int map)
{
    const struct MapHeader *h = Header(map);
    return h && h->events ? h->events->bgEventCount : 0;
}

int MakerApi_BgInfo(int map, int i, struct MakerBgInfo *b)
{
    const struct MapHeader *h = Header(map);
    const struct MakerMapEventNames *n = Names(map);
    const struct BgEvent *e;
    memset(b, 0, sizeof(*b));
    if (h == NULL || h->events == NULL || i < 0 || i >= h->events->bgEventCount)
        return 0;
    e = &h->events->bgEvents[i];
    b->x = e->x;
    b->y = e->y;
    b->elevation = e->elevation;
    b->kind = e->kind;
    b->typeName = b->scriptName = "";
    if (n && i < n->bgCount)
    {
        b->typeName = Maker_BgName(n->bg + i)->type;
        b->scriptName = Maker_BgName(n->bg + i)->script;
    }
    return 1;
}

int MakerApi_CoordCount(int map)
{
    const struct MapHeader *h = Header(map);
    return h && h->events ? h->events->coordEventCount : 0;
}

int MakerApi_CoordInfo(int map, int i, struct MakerCoordInfo *c)
{
    const struct MapHeader *h = Header(map);
    const struct MakerMapEventNames *n = Names(map);
    const struct CoordEvent *e;
    memset(c, 0, sizeof(*c));
    if (h == NULL || h->events == NULL || i < 0 || i >= h->events->coordEventCount)
        return 0;
    e = &h->events->coordEvents[i];
    c->x = e->x;
    c->y = e->y;
    c->elevation = e->elevation;
    c->varName = c->scriptName = "";
    if (n && i < n->coordCount)
    {
        c->varName = Maker_CoordName(n->coord + i)->var;
        c->scriptName = Maker_CoordName(n->coord + i)->script;
    }
    return 1;
}

// A look's standing picture facing `facing` (0 south, 1 north, 2 west, 3 east), as the field shows it
int MakerApi_RenderObject(int graphicsId, int facing, unsigned int *rgba, int maxW, int maxH, int *outW, int *outH)
{
    const struct ObjectEventGraphicsInfo *info = GetObjectEventGraphicsInfo(graphicsId);
    const union AnimCmd *anim;
    const u16 *pal;
    const u8 *data;
    int w, h, frame = 0, hflip = 0, x, y, tilesW;
    if (info == NULL || info->images == NULL)
        return 0;
    w = info->width;
    h = info->height;
    if (w <= 0 || h <= 0 || w > maxW || h > maxH)
        return 0;
    if (info->anims && !info->inanimate && facing >= 0 && facing < 4)
    {
        anim = info->anims[ANIM_STD_FACE_SOUTH + facing];
        if (anim && anim[0].type >= 0)
        {
            frame = anim[0].frame.imageValue;
            hflip = anim[0].frame.hFlip;
        }
    }
    else if (info->anims && info->anims[0] && info->anims[0][0].type >= 0)
        frame = info->anims[0][0].frame.imageValue;
    data = info->images[frame].data;
    if (data == NULL)
        return 0;
    pal = ObjectEvent_MakerPalette(graphicsId);
    tilesW = w / 8;
    for (y = 0; y < h; y++)
        for (x = 0; x < w; x++)
        {
            int sx = hflip ? w - 1 - x : x;
            int tile = (y / 8) * tilesW + sx / 8;
            u8 b = data[tile * 32 + (y % 8) * 4 + (sx % 8) / 2];
            u8 c = (sx & 1) ? b >> 4 : b & 15;
            u16 col = pal[c];
            rgba[y * w + x] = c == 0 ? 0 : 0xFF000000u | (((col >> 10) & 31) * 255 / 31) << 16
                                         | (((col >> 5) & 31) * 255 / 31) << 8 | ((col & 31) * 255 / 31);
        }
    *outW = w;
    *outH = h;
    return 1;
}

// The way a movement type faces when the map loads (0 south, 1 north, 2 west, 3 east), as the field sets it
int MakerApi_MovementFacing(int movementType)
{
    u8 d;
    if (movementType < 0 || movementType >= NUM_MOVEMENT_TYPES)
        return 0;
    d = gInitialMovementTypeFacingDirections[movementType];
    return d >= 1 && d <= 4 ? d - 1 : 0;
}
