/*
 * Explods and the system element draw list (original module 0x40FB50-0x41249F,
 * strings from "illegal explod animno" to "%s.textwrap").
 *
 * A SysDrawList is a fixed array of SysDrawItems: the explods of the match
 * (GameVars.explods, also hit sparks and dust) and the system.def / fight.def
 * elements being drawn (GameVars.sysexplods). An item draws an animation, a
 * sprite or a text, moves with a velocity and an acceleration, may be bound
 * to a player and is freed when its time runs out. System elements
 * (SysElem, read by SysElem_Read) are added to the list by SysElem_Add,
 * which finds the item of the element again on the next frame by its serial
 * number, so an element keeps its animation running while it is drawn.
 *
 * Floating point follows the x87 code of the original: intermediate results
 * stay in double and are rounded to float where the original stores a float;
 * float to int conversions truncate like __ftol2.
 */
#include "mugen.h"

/* x * scale rounded as the original does: through float, floor(x + 0.5), float again, then __ftol2. */
static int Ex_Round(double x)
{
    float f = (float)x;

    f = (float)floor(f + 0.5);
    return Ftol(f);
}

/* The bind reference of an item (a PlayerRef, see char.h). */
#define EX_BIND(e) ((PlayerRef *)(e)->bind)

/* Total time of the action an animation state plays. */
static int Ex_ActionTime(const AnimState *st)
{
    KeyedArray *actions = st->anim->actions;

    return ((AnimAction *)(actions->data + actions->elem_size * st->action))->total_time;
}

/* The bound player of an item when the reference is still valid (active, same id), else NULL. */
static Player *Ex_BoundPlayer(const SysDrawItem *e)
{
    Player *o;

    if (e->bind[0] != 1) {
        return NULL;
    }
    o = g_game->players[e->bind[2]];
    if (o == NULL || !o->active || o->id != e->bind[1]) {
        return NULL;
    }
    return o;
}

/* ------------------------------------------------------------------ the list */

/*
 * 0x0040FB50: a list of count items, each with its animation state (drawn
 * through `layer`) and a sprite set holding [Config] PlayerPalMax palettes.
 * NULL when out of memory (what was allocated is leaked).
 */
SysDrawList *Explods_Create(int count, struct SpriteQueue *layer)
{
    SysDrawList *list = calloc(1, sizeof(SysDrawList));
    int i;

    if (list == NULL) {
        g_outOfMemory = 1;
        return NULL;
    }
    list->capacity = count;
    list->items = calloc(count, sizeof(SysDrawItem));
    if (list->items == NULL) {
        g_outOfMemory = 1;
    }
    if (list->items == NULL) {
        return NULL;
    }
    for (i = 0; i < count; i++) {
        SysDrawItem *e = &list->items[i];
        AnimState *st = calloc(1, sizeof(AnimState));
        SpriteSet *set;
        int pals;

        if (st == NULL) {
            g_outOfMemory = 1;
        } else {
            st->anim = NULL;
            st->sprites = NULL;
        }
        e->anim = st;
        if (st == NULL) {
            return NULL;
        }
        pals = g_game->player_pal_max;
        set = calloc(1, sizeof(SpriteSet));
        if (set == NULL) {
            g_outOfMemory = 1;
        } else if (pals > 0) {
            set->palettes = KeyedArray_Create(pals, sizeof(Palette));
            if (set->palettes == NULL) {
                set = NULL;
            }
        }
        e->sprites = set;
        if (set == NULL) {
            return NULL;
        }
        e->anim->queue = layer;
        e->palfx_own.add[0] = 0;
        e->palfx_own.add[1] = 0;
        e->palfx_own.add[2] = 0;
        e->palfx_own.unused_1c = 0;
        e->palfx_own.invertall = 0;
        e->palfx_own.mul[2] = 0x100;
        e->palfx_own.mul[1] = 0x100;
        e->palfx_own.mul[0] = 0x100;
        e->palfx_own.color = 0x100;
        e->palfx_own.serial = g_palFxSerial++;
        memset(&e->remap_own, 0, sizeof(PalRemap));
    }
    return list;
}

/* 0x0040FD00 */
void Explods_Free(void *list_)
{
    SysDrawList *list = list_;
    int i;

    if (list == NULL) {
        return;
    }
    if (list->items != NULL) {
        for (i = 0; i < list->capacity; i++) {
            SysDrawItem *e = &list->items[i];
            AnimState *st;

            memset(&e->remap_own, 0, sizeof(PalRemap));
            st = e->anim;
            if (st != NULL) {
                if (st->owns_sprites != 0) {
                    SpriteSet_Free(st->sprites);
                }
                free(st);
            }
            SpriteSet_Free(e->sprites);
        }
        free(list->items);
    }
    free(list);
}

/*
 * 0x004105B0: clears an item for reuse, keeping its animation state and
 * sprite set. The original builds the Coords on the stack and leaves their
 * pad_24 as stack garbage; it is zeroed here.
 */
void SysDrawItem_Reset(SysDrawItem *e)
{
    AnimState *st = e->anim;
    SpriteSet *set = e->sprites;
    Coord c;

    memset(e, 0, sizeof(SysDrawItem));
    e->anim = st;
    e->sprites = set;
    c.x = 0.0;
    c.y = 0.0;
    c.z = 0.0;
    c.unit = 1.0;
    c.type = COORD_TYPE_2D;
    c.pad_24 = 0;
    e->accel = c;
    e->vel = c;
    e->pos = c;
    c.x = 1.0;
    c.y = 1.0;
    c.z = 0.0;
    c.unit = 0.0;
    e->scale_mul = 1.0f;
    e->used = 1;
    e->pause_movetime[1] = 0;
    e->pause_movetime[0] = 0;
    e->started = 0;
    e->time = 0;
    e->ignorehitpause = 1;
    e->space = 0;
    e->sprite = -1;
    e->scale = c;
    e->unused_104 = 0.0f;
    e->unused_100 = 0.0f;
    e->bindtime = 0;
    e->unused_108 = 0;
    e->unused_10c = 0;
    e->bind[0] = 0;
    e->offset_y = 0.0f;
    e->flip = 0;
    e->offset_x = 0.0f;
    e->anim->queue = NULL;
    e->anim->layer = 0;
    e->anim->anim = NULL;
    e->anim->sprites = NULL;
    e->palfx = NULL;
    e->remap = NULL;
    e->trans = -1;
    e->alpha_src = 0x100;
    e->alpha_dst = 0;
    e->window_y2 = -1;
    e->window_x2 = -1;
    e->window_y1 = -1;
    e->window_x1 = -1;
    e->font_b = 0x100;
    e->font_g = 0x100;
    e->font_r = 0x100;
}

/* 0x00411480: frees an item (its own sprite set too) without updating the count of the list. */
void SysDrawItem_Free(SysDrawItem *e)
{
    if (!e->used) {
        return;
    }
    if (e->sprites != NULL) {
        SpriteSet_Free(e->sprites);
    }
    e->sprites = NULL;
    e->draw_sprites = NULL;
    e->used = 0;
}

/* 0x004114C0: frees item `index` of the list. */
void SysDraw_Remove(int index, SysDrawList *list)
{
    SysDrawItem *e = &list->items[index];

    if (!e->used) {
        return;
    }
    if (e->sprites != NULL) {
        SpriteSet_Free(e->sprites);
    }
    e->sprites = NULL;
    e->draw_sprites = NULL;
    e->used = 0;
    list->count--;
}

/* 0x00410550: frees every item. */
void SysDraw_Clear(SysDrawList *list)
{
    int i;

    for (i = 0; i < list->capacity; i++) {
        SysDrawItem *e = &list->items[i];
        if (e->used) {
            if (e->sprites != NULL) {
                SpriteSet_Free(e->sprites);
            }
            e->sprites = NULL;
            e->draw_sprites = NULL;
            e->used = 0;
        }
    }
    list->count = 0;
}

/* 0x00411710: the item of a system element (id, owner), or -1. An id of -1 matches any. */
int SysDraw_Find(const SysDrawList *list, int id, int owner_id)
{
    int i;

    for (i = 0; i < list->capacity; i++) {
        const SysDrawItem *e = &list->items[i];
        if (e->used && (e->id == id || id == -1) && e->owner_id == owner_id) {
            return i;
        }
    }
    return -1;
}

/*
 * 0x00411280: a new animation item playing `action` of `anim` with `sprites`;
 * the index of the item, or -1 (no such action, list full).
 */
int Explods_Add(SpriteSet *sprites, SysDrawList *list, int action, Anim *anim)
{
    SysDrawItem *e;
    AnimState *st;
    KeyedArray *actions;
    int i;

    if (KeyedArray_Find(anim->actions, 0, action) == -1) {
        return -1;
    }
    for (i = 0; i < list->capacity; i++) {
        if (!list->items[i].used) {
            break;
        }
    }
    if (i >= list->capacity) {
        return -1;
    }
    e = &list->items[i];
    SysDrawItem_Reset(e);
    e->kind = SYSDRAW_ANIM;
    e->anim->anim = anim;
    e->anim->sprites = sprites;
    e->palfx = NULL;
    e->remap = NULL;
    e->draw_sprites = sprites;
    Anim_SetAction(e->anim, action);
    st = e->anim;
    /* the slot of the action is compared with the number of actions */
    if (st->action > st->anim->action_count) {
        st->action = 0;
    }
    actions = st->anim->actions;
    st->ticks = 0;
    st->time = 0;
    st->elem = (AnimElem *)((AnimAction *)(actions->data + actions->elem_size * st->action))->elems->data;
    st->elem_index = 0;
    Anim_SeekElem(st);
    list->count++;
    return i;
}

/* 0x00411360: a new item drawing sprite group,number of `sprites`; its index, or -1. */
int SysDraw_AddSprite(int group, int number, SysDrawList *list, SpriteSet *sprites)
{
    SysDrawItem *e;
    int sprite = KeyedArray_Find(sprites->sprites, group, number);
    int i;

    if (sprite == -1) {
        return -1;
    }
    for (i = 0; i < list->capacity; i++) {
        if (!list->items[i].used) {
            break;
        }
    }
    if (i >= list->capacity) {
        return -1;
    }
    e = &list->items[i];
    SysDrawItem_Reset(e);
    e->sprite = sprite;
    e->kind = SYSDRAW_SPRITE;
    e->draw_sprites = sprites;
    list->count++;
    return i;
}

/* 0x004113E0: a new item drawing a text (at most 63 characters kept); its index, or -1. */
int SysDraw_AddText(SysDrawList *list, const char *text, struct Font *font, int bank, int align, int r, int g, int b)
{
    SysDrawItem *e;
    int i;

    for (i = 0; i < list->capacity; i++) {
        if (!list->items[i].used) {
            break;
        }
    }
    if (i >= list->capacity) {
        return -1;
    }
    e = &list->items[i];
    SysDrawItem_Reset(e);
    e->font_bank = bank;
    e->font = font;
    e->font_b = b;
    e->font_align = align;
    e->font_r = r;
    e->kind = SYSDRAW_TEXT;
    e->font_g = g;
    e->text[0] = '\0';
    strncat(e->text, text, sizeof(e->text) - 1);
    list->count++;
    return i;
}

/* ------------------------------------------------------------------ per-frame update */

/*
 * 0x0040FDB0: advances item `index` by one tick: frozen by a Pause /
 * SuperPause past its pausemovetime / supermovetime and while its owner is
 * in hit pause (unless ignorehitpause), freed once its time is over,
 * otherwise its animation steps, it moves and follows the player it is
 * bound to.
 */
void SysDraw_Start(SysDrawList *list, int index)
{
    SysDrawItem *e = &list->items[index];
    GameVars *g;
    int pause, bindtime;

    if (!e->used) {
        return;
    }
    g = g_game;
    pause = g->pause_kind;
    if (pause != -1) {
        int t = e->pause_movetime[pause];
        if (e->started > t && t != -1) {
            return;
        }
    }
    if (e->owner_id != 0 && e->ignorehitpause == 0) {
        int idx = Player_FindIndexById(e->owner_id);
        if (idx != -1 && g->players[idx] != NULL && g->players[idx]->paused != 0) {
            return;
        }
    }
    if (e->started >= e->time && e->time != -1) {
        SysDrawItem_Free(e);
        list->count--;
        return;
    }
    if (e->kind == SYSDRAW_ANIM) {
        Anim_Step(e->anim);
    }
    if (e->started > 0) {
        Coord c;
        e->pos = *Coord_Add(&c, e->pos.x, e->pos.y, e->pos.z, e->pos.unit, e->pos.type, e->pos.pad_24, e->vel.x,
                            e->vel.y, e->vel.z, e->vel.unit, e->vel.type, e->vel.pad_24);
        e->vel = *Coord_Add(&c, e->vel.x, e->vel.y, e->vel.z, e->vel.unit, e->vel.type, e->vel.pad_24, e->accel.x,
                            e->accel.y, e->accel.z, e->accel.unit, e->accel.type, e->accel.pad_24);
    }
    bindtime = e->bindtime;
    if (bindtime != 0) {
        float x, y;
        int facing;
        if (e->bind[0] == 1 && Ex_BoundPlayer(e) != NULL && PlayerRef_GetPos(&e->bind[1], &x, &y, &facing)) {
            Coord c;
            e->pos = *Coord_MakeUnit(&c, (double)e->offset_x + x, (double)e->offset_y + y);
        }
        if (bindtime > 0) {
            e->bindtime = bindtime - 1;
        }
    }
    e->started++;
}

/*
 * 0x0040FF90: the screen position of an item: stage space positions are
 * moved by the camera and scaled by the zoom (and the screen shake), screen
 * space ones are kept. The original leaves pad_24 as stack garbage in the
 * stage case; it is zeroed here.
 */
Coord *SysDraw_ScreenPos(const SysDrawItem *e, Coord *out)
{
    GameVars *g;
    Coord c;
    float s, cx, cy, t;
    double k;

    if (e->space != 0) {
        *out = e->pos;
        return out;
    }
    g = g_game;
    s = g->stage_scale * g->camera_zoom;
    k = g_gameCoord[0] / g->stage.unit;
    cx = (float)(g->camera_x * k + g->camera_offset_x);
    c.x = (e->pos.x - cx) * s + (double)(Ftol(g->game_coord[0]) / 2);
    cy = (float)(g->camera_y * k + (double)g->camera_offset_y * g->camera_zoom);
    t = (float)((double)g->draw_offset_y - cy);
    c.y = e->pos.y * s + t;
    c.z = 0.0;
    c.unit = 1.0;
    c.type = COORD_TYPE_2D;
    c.pad_24 = 0;
    *out = c;
    if (g->zoom_active != 0.0f) {
        out->x = out->x - (double)g->shake_x * g->stage_scale;
        out->y = out->y - (double)g->shake_y * g->stage_scale;
    }
    return out;
}

/* 0x004100C0: the draw parameters of an item (position, flip, scale, angles, transparency). */
void SysDraw_MakeParams(DrawParams *dp, const SysDrawItem *e)
{
    float sc = 1.0f;
    float xangle;
    Coord c, pos;

    if (e->space == 0) {
        sc = g_game->stage_scale * g_game->camera_zoom;
    }
    memset(dp, 0, sizeof(DrawParams));
    dp->scale2_y = 1.0f;
    dp->flags = 0x100F;
    dp->scale2_x = 1.0f;
    dp->masked = 1;
    dp->scale_y = 1.0f;
    dp->scale_x = 1.0f;
    dp->alpha_src = 1.0f;
    dp->bottom_stretch = 1.0f;
    dp->top_stretch = 1.0f;
    dp->width_ratio = 1.0f;
    dp->width_anchor = 0.5f;
    dp->scale_x = (float)(sc * e->scale.x);
    dp->scale_y = (float)(sc * e->scale.y);
    dp->angle = (float)e->angle;
    dp->angle_z = (float)e->yangle;
    xangle = (float)e->xangle;
    dp->angle_y = xangle;
    if (dp->angle != 0.0f || dp->angle_z != 0.0f || xangle != 0.0f) {
        dp->flags = 0x102F;
    }
    pos = *SysDraw_ScreenPos(e, &c);
    dp->x = (float)pos.x;
    dp->y = (float)pos.y;
    dp->flip2 = e->flip;
    DrawParams_SetBlend(e->trans, dp, e->alpha_src, e->alpha_dst);
}

/*
 * 0x004101F0: draws an item with its window as the clip rectangle, its PalFX
 * combined with the current one and its palette remapping.
 */
void SysDraw_Update(SysDrawItem *e)
{
    int cx1 = g_clipX1, cx2 = g_clipX2, cy1 = g_clipY1, cy2 = g_clipY2;
    PalRemap *saved_remap;
    PalFx *saved_fx;
    DrawParams dp;

    SysDraw_MakeParams(&dp, e);
    if (e->window_y2 != -1) {
        Gfx_SetClipRect(e->window_x1, e->window_y1, e->window_x2, e->window_y2);
    }
    saved_remap = g_palRemapCur;
    g_palRemapCur = e->remap;
    saved_fx = g_palFxCur;
    if (e->palfx != NULL && e->palfx != &e->palfx_own) {
        e->palfx_own = *e->palfx;
        if (saved_fx != NULL) {
            PalFx_Combine(&e->palfx_own, &e->palfx_own, saved_fx);
        }
    } else {
        PalFx_Init(&e->palfx_own, saved_fx);
    }
    g_palFxCur = &e->palfx_own;
    switch (e->kind) {
    case SYSDRAW_ANIM:
        Anim_Draw(e->anim, &dp, 1.0f, 1.0f);
        break;
    case SYSDRAW_SPRITE:
        if (g_renderMode == RENDER_MODE_GL) {
            Sprite_DrawGL(e->draw_sprites, e->sprite, &dp);
        } else {
            Sprite_DrawSoft(e->draw_sprites, e->sprite, &dp);
        }
        break;
    case SYSDRAW_TEXT: {
        const char *text = e->text_ptr != NULL ? e->text_ptr : e->text;
        int wrap = g_fontWrapMode;
        g_fontWrapMode = e->textwrap;
        Font_DrawTextEx(text, e->font, &dp, e->font_align, e->font_bank, e->font_r, e->font_g, e->font_b);
        g_fontWrapMode = wrap;
        break;
    }
    default:
        break;
    }
    g_palFxCur = saved_fx;
    g_palRemapCur = saved_remap;
    if (e->window_y2 != -1) {
        Gfx_SetClipRect(cx1, cy1, cx2, cy2);
    }
}

/* 0x004104F0: draws the shadow of an item (a = stage shadow parameters, b = the stage's shadow fade). */
void SysDraw_DrawShadow(const SysDrawItem *e, const int *a, const int *b)
{
    DrawParams dp;
    float y;

    SysDraw_MakeParams(&dp, e);
    y = (float)e->pos.y;
    Shadow_Draw(e->anim, &dp, y, 0.0f, 1.0f, 1.0f, a, b);
}

/*
 * 0x004103E0: draws the shadows of the items that have one. The current PalFX
 * is not restored between items: each shadow combines its PalFX with the one
 * of the previous shadow (quirk of the original).
 */
void Explods_DrawShadows(SysDrawList *list)
{
    PalFx *saved_fx = g_palFxCur;
    PalRemap *saved_remap = g_palRemapCur;
    int seen = 0;
    int i;

    for (i = 0; i < list->capacity; i++) {
        SysDrawItem *e = &list->items[i];
        if (seen > list->count) {
            break;
        }
        if (!e->used) {
            continue;
        }
        seen++;
        if (e->shadow == 0) {
            continue;
        }
        g_palRemapCur = e->remap;
        if (e->palfx != NULL && e->palfx != &e->palfx_own) {
            PalFx *cur = g_palFxCur;
            e->palfx_own = *e->palfx;
            if (cur != NULL) {
                PalFx_Combine(&e->palfx_own, &e->palfx_own, cur);
            }
        } else {
            PalFx_Init(&e->palfx_own, g_palFxCur);
        }
        g_palFxCur = &e->palfx_own;
        SysDraw_DrawShadow(e, g_game->stage.shadow, &g_game->stage.reflection_intensity);
    }
    g_palFxCur = saved_fx;
    g_palRemapCur = saved_remap;
}

/* ------------------------------------------------------------------ explods */

/*
 * 0x004109E0: applies the parameters of an Explod (modify = 0) or a
 * ModifyExplod (modify = 1) controller to item e. ModifyExplod leaves the
 * parameters it was not given at INT_MIN / -FLT_MAX, which are skipped.
 * The facing and vfacing flags are only ever added (quirk of the original).
 */
void Explod_SetParams(Player *p, SysDrawItem *e, const ExplodParams *ep, StateRunCtx *ctx, int modify,
                      const StateCtrl *sc)
{
    int v, r, gg, b;
    float f;

    if (!modify) {
        e->id = Expr_EvalInt(p, &ep->id, 1);
        if (e->id < 0) {
            e->id = 0;
            Player_WarnAt(p, sc, "%s", "negative explod ID");
        }
        e->owner_id = p->id;
    }
    if (ep->postype != INT_MIN) {
        float x, y, off_x, off_y;
        int facing, face, rnd;
        double dir;

        if (ep->space != INT_MIN) {
            e->space = ep->space;
            if (ep->space == -1) {
                int pt = ep->postype;
                e->space = (pt == POSTYPE_FRONT || pt == POSTYPE_BACK || pt == POSTYPE_RIGHT || pt == POSTYPE_LEFT)
                                 ? 1 : 0;
            }
        }
        Player_GetPosOrigin(EX_BIND(e), p, ep->postype, &ctx->enemy, e->space, &x, &y, &off_x, &off_y, &facing);
        if (ep->postype == POSTYPE_FRONT) {
            facing = 1;
        }
        e->postype = ep->postype;
        face = Expr_EvalInt(p, &ep->facing, 1);
        if (face != 1 && face != -1) {
            face = 1;
            Player_WarnAt(p, sc, "%s", "explod facing param must be 1 or -1");
        }
        if (face * facing == -1) {
            e->flip |= DRAW_FLIP_H;
        }
        v = Expr_EvalInt(p, &ep->vfacing, 1);
        if (v == -1) {
            e->flip |= DRAW_FLIP_V;
        } else if (v != 1) {
            Player_WarnAt(p, sc, "%s", "explod vfacing param must be 1 or -1");
        }
        dir = (double)facing;
        e->offset_x = (float)(Player_FromLocalCoord(p, Expr_EvalFloat(p, &ep->pos[0])) * dir);
        e->offset_y = (float)Player_FromLocalCoord(p, Expr_EvalFloat(p, &ep->pos[1]));
        e->offset_x = off_x + e->offset_x;
        e->offset_y = e->offset_y + off_y;

        rnd = Expr_EvalInt(p, &ep->random[0], 1);
        if (rnd < 0) {
            rnd = 0;
            Player_WarnAt(p, sc, "%s", "negative explod random param");
        }
        v = Match_Random() % (rnd + 1) - ((rnd + 1) >> 1);
        e->offset_x = (float)(Player_FromLocalCoord(p, (double)v) + e->offset_x);
        rnd = Expr_EvalInt(p, &ep->random[1], 1);
        if (rnd < 0) {
            rnd = 0;
            Player_WarnAt(p, sc, "%s", "negative explod random param");
        }
        v = Match_Random() % (rnd + 1) - ((rnd + 1) >> 1);
        e->offset_y = (float)(Player_FromLocalCoord(p, (double)v) + e->offset_y);

        e->pos.x = (double)x + e->offset_x;
        e->pos.y = (double)e->offset_y + y;
        e->vel.x = Player_FromLocalCoord(p, Expr_EvalFloat(p, &ep->vel[0])) * dir;
        e->vel.y = Player_FromLocalCoord(p, Expr_EvalFloat(p, &ep->vel[1]));
        e->accel.x = Player_FromLocalCoord(p, Expr_EvalFloat(p, &ep->accel[0])) * dir;
        e->accel.y = Player_FromLocalCoord(p, Expr_EvalFloat(p, &ep->accel[1]));
        if (modify && e->bindtime == 0) {
            e->bindtime = 1;
        }
    }

    f = Expr_EvalFloat(p, &ep->scale[0]);
    if (f != -FLT_MAX) {
        e->scale.x = Player_FromLocalCoord(p, f) * e->scale_mul;
    }
    f = Expr_EvalFloat(p, &ep->scale[1]);
    if (f != -FLT_MAX) {
        e->scale.y = Player_FromLocalCoord(p, f) * e->scale_mul;
    }
    f = Expr_EvalFloat(p, &ep->angle);
    if (f != -FLT_MAX) {
        e->angle = f;
    }
    f = Expr_EvalFloat(p, &ep->yangle);
    if (f != -FLT_MAX) {
        e->yangle = f;
    }
    f = Expr_EvalFloat(p, &ep->xangle);
    if (f != -FLT_MAX) {
        e->xangle = f;
    }

    v = Expr_EvalInt(p, &ep->bindtime, 1);
    if (v != INT_MIN) {
        if (v < -1) {
            v = -1;
            Player_WarnAt(p, sc, "%s", "Illegal bindtime for explod");
        }
        e->bindtime = v;
    }
    v = Expr_EvalInt(p, &ep->bindid, 1);
    if (v != INT_MIN && v != -2) {
        if (e->space == 1) {
            Player_WarnAt(p, sc, "%s", "Explod in screen space cannot be bound to a player");
        } else if (v >= 0) {
            if (!PlayerRef_SetById(EX_BIND(e), v)) {
                Player_WarnAt(p, sc, "bindid %i not found for explod", v);
            }
        } else if (v == -1) {
            PlayerRef_Set(EX_BIND(e), p, 1);
        }
    }
    v = Expr_EvalInt(p, &ep->ontop, 1);
    if (v != INT_MIN) {
        e->layer = v != 0 ? 2 : 0;
    }
    v = Expr_EvalInt(p, &ep->removetime, 1);
    if (v != INT_MIN) {
        if (v < -2) {
            Player_WarnAt(p, sc, "%s", "Illegal removetime for explod");
            v = -2;
        }
        if (v == -2) {
            e->time = Ex_ActionTime(e->anim) - e->started;
        } else {
            e->time = v;
        }
    }
    /* supermovetime, or the removetime with supermove (only when supermovetime is 0 or not given) */
    v = Expr_EvalInt(p, &ep->supermovetime, 1);
    if (v == 0 || v == INT_MIN) {
        int sm = Expr_EvalInt(p, &ep->supermove, 1);
        if (sm != INT_MIN && sm != 0) {
            e->pause_movetime[1] = e->time;
        } else if (v != INT_MIN) {
            e->pause_movetime[1] = v;
        }
    } else {
        e->pause_movetime[1] = v == -2 ? e->time : v;
    }
    v = Expr_EvalInt(p, &ep->pausemovetime, 1);
    if (v != INT_MIN) {
        e->pause_movetime[0] = v == -2 ? e->time : v;
    }
    if (e->layer == 0) {
        v = Expr_EvalInt(p, &ep->sprpriority, 1);
        if (v != INT_MIN) {
            e->anim->layer = v;
            e->anim->queue = g_game->sprite_layer1;
        }
    } else {
        e->anim->queue = NULL;
    }
    r = Expr_EvalInt(p, &ep->shadow[0], 1);
    if (r != INT_MIN) {
        gg = Expr_EvalInt(p, &ep->shadow[1], 1);
        b = Expr_EvalInt(p, &ep->shadow[2], 1);
        if (r < -1 || gg < -1 || b < -1 || r > 0xFF || gg > 0xFF || b > 0xFF) {
            Player_WarnAt(p, sc, "%s", "Illegal shadow color");
            e->shadow = -1;
        } else if (r == -1) {
            e->shadow = -1;
        } else {
            e->shadow = (int)Video_MapRGB(r, gg, b);
        }
    }
    v = Expr_EvalInt(p, &ep->removeongethit, 1);
    if (v != INT_MIN) {
        e->removeongethit = v;
    }
    v = Expr_EvalInt(p, &ep->ignorehitpause, 1);
    if (v != INT_MIN) {
        e->ignorehitpause = v;
    }
    if (ep->trans != INT_MIN) {
        e->trans = ep->trans;
    }
    v = Expr_EvalInt(p, &ep->alpha[0], 1);
    if (v != INT_MIN) {
        e->alpha_src = v;
    }
    if (e->alpha_src < 0) {
        e->alpha_src = 0;
    }
    if (e->alpha_src > 0x100) {
        e->alpha_src = 0x100;
    }
    v = Expr_EvalInt(p, &ep->alpha[1], 1);
    if (v != INT_MIN) {
        e->alpha_dst = v;
    }
    if (e->alpha_dst < 0) {
        e->alpha_dst = 0;
    }
    if (e->alpha_dst > 0x100) {
        e->alpha_dst = 0x100;
    }
}

/*
 * 0x00410740: the Explod controller: a new explod playing the animation of
 * the player (or of fightfx.air with the F prefix), with its PalFX and
 * palette remapping shared with the player unless ownpal is set.
 */
void Explod_Create(Player *p, ExplodParams *ep, StateRunCtx *ctx, StateCtrl *sc)
{
    GameVars *g = g_game;
    SysDrawItem *e;
    int action, idx, ownpal, rg, rn;

    action = Expr_EvalInt(p, &ep->anim, 1);
    if (action < 0) {
        action = 0;
        Player_WarnAt(p, sc, "%s", "illegal explod animno");
    }
    if (ep->anim_fight) {
        idx = Explods_Add(g->fightfx_sff, g->explods, action, g->fightfx_anim);
    } else {
        idx = Explods_Add(p->sprites, g_game->explods, action, p->data->anim);
    }
    ownpal = Expr_EvalInt(p, &ep->ownpal, 1);
    rg = Expr_EvalInt(p, &ep->remappal[0], 0);
    rn = Expr_EvalInt(p, &ep->remappal[1], 0);
    if (idx == -1) {
        return;
    }
    g = g_game;
    e = &g->explods->items[idx];
    if (ep->anim_fight) {
        e->scale_mul = (float)(Player_ToLocalCoord(p, g->fightfx_scale) * g_game->motif_scale);
    }
    Explod_SetParams(p, e, ep, ctx, 0, sc);
    if (ownpal == 0) {
        e->palfx = p->palfx;
        if (ep->anim_fight) {
            e->remap = NULL;
            return;
        }
        e->remap = p->remap;
        return;
    }
    e->palfx = NULL;
    if (ep->anim_fight) {
        e->remap = NULL;
        return;
    }
    if (rg == -1) {
        e->remap_own = *p->remap;
    } else if (SpriteSet_FindPalette(e->draw_sprites, rg, rn) != NULL) {
        PalRemapEntry en;
        en.src_group = -1;
        en.src_index = 0;
        en.dst_group = rg;
        en.dst_index = rn;
        memset(&e->remap_own, 0, sizeof(PalRemap));
        PalRemap_Set(&e->remap_own, &en);
    } else {
        Player_WarnAt(p, sc, "Explod invalid remappal %i,%i", rg, rn);
    }
    e->remap = &e->remap_own;
}

/* 0x00410950: the ModifyExplod controller: the explods of the player with the ID (-1: all of them). */
void Explod_Modify(Player *p, ExplodParams *ep, StateRunCtx *ctx, StateCtrl *sc)
{
    SysDrawList *list = g_game->explods;
    int id = Expr_EvalInt(p, &ep->id, 1);
    int owner = p->id;
    int i;

    for (i = 0; i < list->capacity; i++) {
        SysDrawItem *e = &list->items[i];
        if (e->used && (e->id == id || id == -1) && e->owner_id == owner) {
            Explod_SetParams(p, e, ep, ctx, 1, sc);
        }
    }
}

/* 0x00411510: RemoveExplod: the explods of a player with the ID (-1: all of them). */
void Explods_RemoveOwner(SysDrawList *list, int owner_id, int id)
{
    int i;

    for (i = 0; i < list->capacity; i++) {
        SysDrawItem *e = &list->items[i];
        if (e->used && (e->id == id || id == -1) && e->owner_id == owner_id) {
            if (e->sprites != NULL) {
                SpriteSet_Free(e->sprites);
            }
            e->sprites = NULL;
            e->draw_sprites = NULL;
            e->used = 0;
            list->count--;
        }
    }
}

/* 0x00411580: the explods with removeongethit of a player that was hit. */
void Explods_RemoveOnHit(SysDrawList *list, int owner_id)
{
    int i;

    for (i = 0; i < list->capacity; i++) {
        SysDrawItem *e = &list->items[i];
        if (e->used && e->owner_id == owner_id && e->removeongethit != 0) {
            if (e->sprites != NULL) {
                SpriteSet_Free(e->sprites);
            }
            e->sprites = NULL;
            e->draw_sprites = NULL;
            e->used = 0;
            list->count--;
        }
    }
}

/*
 * 0x004115F0: a helper goes away: the explods bound to it (to any invalid
 * player when p is NULL) are unbound, and those using its PalFX or palette
 * remapping get their own copy.
 */
void Explods_Unbind(Player *p, SysDrawList *list)
{
    PalFx *fx = (PalFx *)((char *)p + offsetof(Player, palfx_own));
    PalRemap *remap = (PalRemap *)((char *)p + offsetof(Player, remap_own));
    int i;

    for (i = 0; i < list->capacity; i++) {
        SysDrawItem *e = &list->items[i];
        int bound;

        if (!e->used) {
            continue;
        }
        bound = e->bind[0] == 1 && Ex_BoundPlayer(e) != NULL;
        if (p != NULL ? bound && p->active && e->bind[1] == p->id : bound) {
            e->unused_104 = 0.0f;
            e->bindtime = 0;
            e->unused_100 = 0.0f;
            e->unused_108 = 0;
            e->unused_10c = 0;
            e->bind[0] = 0;
        }
        if (e->palfx == fx) {
            e->palfx_own = *fx;
            e->palfx = &e->palfx_own;
        }
        if (e->remap == remap) {
            e->remap_own = *remap;
            e->remap = &e->remap_own;
        }
    }
}

/* 0x00411750: ExplodBindTime: sets the bind time of the explods of a player with the ID (-1: all). */
void Explods_SetBindTime(SysDrawList *list, int owner_id, int id, int time)
{
    int i;

    for (i = 0; i < list->capacity; i++) {
        SysDrawItem *e = &list->items[i];
        if (e->used && (e->id == id || id == -1) && e->owner_id == owner_id) {
            e->bindtime = time;
        }
    }
}

/* 0x00411790: NumExplod: the explods of a player with the ID (-1: all). */
int Explods_Count(SysDrawList *list, int owner_id, int id)
{
    int n = 0;
    int i;

    for (i = 0; i < list->capacity; i++) {
        SysDrawItem *e = &list->items[i];
        if (e->used && (e->id == id || id == -1) && e->owner_id == owner_id) {
            n++;
        }
    }
    return n;
}

/* ------------------------------------------------------------------ system elements */

/*
 * 0x004117D0: reads "<prefix>.spr", ".anim" or ".font" (with ".text") and,
 * when one is there, the other parameters of a system.def / fight.def
 * element. The element gets a new serial number in any case. Returns 1.
 */
int SysElem_Read(SysElem *elem, const char *prefix, Ini *ini)
{
    char key[0x40];
    const char *v;
    char *dup;
    int id;

    elem->scale_y = 1.0f;
    elem->scale_x = 1.0f;
    id = g_sysElemSerial++;
    sprintf(key, "%s.spr", prefix);
    elem->type = SYSELEM_NONE;
    elem->font_color[2] = 0x100;
    elem->font_color[1] = 0x100;
    elem->font_color[0] = 0x100;
    elem->window_y[1] = -1;
    elem->window_y[0] = -1;
    elem->window_x[1] = -1;
    elem->window_x[0] = -1;
    elem->owner_id = 0;
    elem->id = id;
    if (Ini_Scanf(ini, key, "%i,%i", &elem->group, &elem->number) == 2) {
        elem->type = SYSELEM_SPRITE;
    } else if (elem->type == SYSELEM_NONE) {
        sprintf(key, "%s.anim", prefix);
        v = Ini_GetValue(ini, key);
        if (v != NULL) {
            elem->group = atoi(v);
            elem->type = SYSELEM_ANIM;
        } else if (elem->type == SYSELEM_NONE) {
            sprintf(key, "%s.font", prefix);
            if (Ini_Scanf(ini, key, "%i,%i,%i,%i,%i,%i", &elem->group, &elem->number, &elem->font_align,
                          &elem->font_color[0], &elem->font_color[1], &elem->font_color[2]) >= 1) {
                sprintf(key, "%s.text", prefix);
                elem->type = SYSELEM_FONT;
                v = Ini_GetValue(ini, key);
                if (v != NULL) {
                    elem->text[0] = '\0';
                    strncat(elem->text, v, sizeof(elem->text) - 1);
                    Str_Unquote(elem->text);
                } else {
                    elem->text[0] = '\0';
                }
            }
            if (elem->type == SYSELEM_NONE) {
                return 1;
            }
        }
    }
    sprintf(key, "%s.displaytime", prefix);
    elem->displaytime = -2;
    v = Ini_GetValue(ini, key);
    if (v != NULL) {
        elem->displaytime = atoi(v);
    }
    if (elem->type != SYSELEM_ANIM && elem->displaytime == -2) {
        elem->displaytime = 1;
    }
    sprintf(key, "%s.offset", prefix);
    Ini_Scanf(ini, key, "%f,%f", &elem->x, &elem->y);
    sprintf(key, "%s.facing", prefix);
    v = Ini_GetValue(ini, key);
    if (v != NULL) {
        elem->facing = atoi(v);
    }
    sprintf(key, "%s.vfacing", prefix);
    v = Ini_GetValue(ini, key);
    if (v != NULL) {
        elem->vfacing = atoi(v);
    }
    sprintf(key, "%s.layerno", prefix);
    v = Ini_GetValue(ini, key);
    if (v != NULL) {
        elem->layerno = atoi(v);
    }
    sprintf(key, "%s.scale", prefix);
    Ini_Scanf(ini, key, "%f,%f", &elem->scale_x, &elem->scale_y);
    sprintf(key, "%s.angle", prefix);
    Ini_Scanf(ini, key, "%f", &elem->angle);
    sprintf(key, "%s.yangle", prefix);
    Ini_Scanf(ini, key, "%f", &elem->yangle);
    sprintf(key, "%s.xangle", prefix);
    Ini_Scanf(ini, key, "%f", &elem->xangle);
    sprintf(key, "%s.window", prefix);
    Ini_Scanf(ini, key, "%i,%i,%i,%i", &elem->window_x[0], &elem->window_y[0], &elem->window_x[1], &elem->window_y[1]);
    sprintf(key, "%s.textwrap", prefix);
    dup = Ini_GetValueDup(ini, key);
    if (dup != NULL) {
        int c = dup[0];
        if ((unsigned)(c - 1) <= 0x7E) {
            c = tolower(c);
        }
        if (c == 0 || c == 'n') {
            elem->textwrap = 0;
        } else if (c == 'c') {
            elem->textwrap = 1;
        } else {
            elem->textwrap = 2;
        }
        free(dup);
    }
    return 1;
}

/*
 * 0x00411C00: puts a system element on the list at x, y (screen space): the
 * item of the element when it is already there (its time keeps running),
 * else a new one. Returns the index of the item, or -1. When the animation
 * of a new item cannot be set, the item is freed but still filled in and its
 * index returned (quirk of the original).
 */
int SysElem_Add(const SysElem *elem, SysDrawList *list, SpriteSet *sprites, Anim *anims, void **fonts, float x,
                float y)
{
    SysDrawItem *e;
    int created = 1;
    int idx;
    float fy;
    Coord c;

    if (elem->type == SYSELEM_NONE) {
        return -1;
    }
    idx = SysDraw_Find(list, elem->id, elem->owner_id);
    if (idx != -1) {
        created = 0;
    } else {
        switch (elem->type) {
        case SYSELEM_ANIM:
            if (anims == NULL || sprites == NULL) {
                return -1;
            }
            idx = Explods_Add(sprites, list, elem->group, anims);
            if (idx == -1) {
                return -1;
            }
            e = &list->items[idx];
            if (Anim_SetAction(e->anim, elem->group) == 0) {
                SysDrawItem_Free(e);
                list->count--;
            } else {
                Anim_Reset(e->anim);
            }
            break;
        case SYSELEM_SPRITE:
            if (sprites == NULL) {
                return -1;
            }
            idx = SysDraw_AddSprite(elem->group, elem->number, list, sprites);
            if (idx == -1) {
                return -1;
            }
            break;
        case SYSELEM_FONT:
            if (elem->group < 0 || elem->group >= 10) {
                return -1;
            }
            if (elem->text[0] == '\0' && elem->text_ptr == 0) {
                return -1;
            }
            if (fonts[elem->group] == NULL) {
                return -1;
            }
            idx = SysDraw_AddText(list, elem->text, fonts[elem->group], elem->number, elem->font_align,
                                  elem->font_color[0], elem->font_color[1], elem->font_color[2]);
            if (idx == -1) {
                return -1;
            }
            if (elem->text_ptr != 0) {
                list->items[idx].text_ptr = (const char *)(intptr_t)elem->text_ptr;
            }
            break;
        default:
            return -1;
        }
    }
    e = &list->items[idx];
    e->owner_id = elem->owner_id;
    e->ignorehitpause = 1;
    e->id = elem->id;
    e->space = 1;
    e->postype = 3;
    if (elem->facing == -1) {
        e->flip |= DRAW_FLIP_H;
    }
    if (elem->vfacing == -1) {
        e->flip |= DRAW_FLIP_V;
    }
    e->offset_x = elem->x + x;
    fy = elem->y + y;
    e->offset_y = fy;
    /* the original leaves pad_24 of these Coords as stack garbage */
    c.x = e->offset_x;
    c.y = fy;
    c.z = 0.0;
    c.unit = 1.0;
    c.type = COORD_TYPE_2D;
    c.pad_24 = 0;
    e->pos = c;
    c.x = 0.0;
    c.y = 0.0;
    e->vel = c;
    e->accel = c;
    e->scale.x = elem->scale_x;
    e->scale.y = elem->scale_y;
    e->angle = elem->angle;
    e->yangle = elem->yangle;
    e->xangle = elem->xangle;
    e->layer = elem->layerno;
    if (created) {
        int t = elem->displaytime;
        if (t == -2 && e->kind == SYSDRAW_ANIM) {
            t = Ex_ActionTime(e->anim);
        }
        e->time = t;
        e->pause_movetime[1] = t;
        e->pause_movetime[0] = t;
    }
    e->window_x1 = elem->window_x[0];
    e->window_y1 = elem->window_y[0];
    e->window_x2 = elem->window_x[1];
    e->window_y2 = elem->window_y[1];
    e->textwrap = elem->textwrap;
    return idx;
}

/*
 * 0x00412120: scales an item of GameVars.sysexplods (whatever list it was
 * added to): position, velocity and window by `scale`, the draw scale by
 * scale * sprite_scale; the offset and the window are rounded to whole
 * pixels. The original builds the new position and velocity in uninitialised
 * temporaries of which it only sets x and y: the velocity gets z, unit, type
 * and pad_24 of the old position (left over in the temporary it reuses) and
 * the position gets stack garbage there, kept from the old position here.
 */
void SysElem_SetScale(int index, float scale, float sprite_scale)
{
    SysDrawItem *e = &g_game->sysexplods->items[index];
    Coord old = e->pos;
    Coord vel;
    double ss;
    int v;

    e->pos.x = old.x * scale;
    e->pos.y = scale * old.y;
    vel = old;
    vel.x = e->vel.x * scale;
    vel.y = scale * e->vel.y;
    e->vel = vel;
    ss = (double)scale * sprite_scale;
    e->scale.x = e->scale.x * ss;
    e->scale.y = ss * e->scale.y;
    e->offset_x = (float)Ex_Round((double)scale * e->offset_x);
    e->offset_y = (float)Ex_Round((double)e->offset_y * scale);
    v = e->window_x1;
    if (v != -1) {
        e->window_x1 = Ex_Round((double)v * scale);
    }
    v = e->window_y1;
    if (v != -1) {
        e->window_y1 = Ex_Round((double)v * scale);
    }
    if (scale > 1.0f) {
        v = e->window_x2;
        if (v != -1) {
            e->window_x2 = Ex_Round((double)scale * (v + 1) - 1.0);
        }
        v = e->window_y2;
        if (v != -1) {
            e->window_y2 = Ex_Round((double)scale * (v + 1) - 1.0);
        }
    } else {
        v = e->window_x2;
        if (v != -1) {
            e->window_x2 = Ex_Round((double)scale * v);
        }
        v = e->window_y2;
        if (v != -1) {
            e->window_y2 = Ex_Round((double)scale * v);
        }
    }
}

/*
 * 0x00412430: steps (when it has not started yet and is an animation) and
 * draws item `index`, then frees it when it was its last tick. Returns the
 * index, or -1 when the item is gone.
 */
int SysDraw_DrawItem(SysDrawList *list, int index)
{
    SysDrawItem *e;
    int t;

    if (index == -1) {
        return -1;
    }
    e = &list->items[index];
    if (e->started == 0 && e->kind == SYSDRAW_ANIM) {
        SysDraw_Start(list, index);
    }
    if (g_game->skip_draw == 0) {
        SysDraw_Update(e);
    }
    e->drawn = 1;
    t = e->time;
    if (t <= 1 && t != -1) {
        SysDraw_Remove(index, list);
        return -1;
    }
    return index;
}

/*
 * 0x00411B80: draws a system element of layer `layer` (-1: any) with the
 * given sprites, animations and fonts, scaled by scale * GameVars.motif_scale.
 * Returns the index of its item, or -1.
 */
int SysElem_Draw(int layer, SysDrawList *list, SpriteSet *sprites, Anim *anims, const SysElem *elem, void **fonts,
                 float x, float y, float scale, float sprite_scale)
{
    int idx;

    if (elem->layerno != layer && layer != -1) {
        return -1;
    }
    idx = SysElem_Add(elem, list, sprites, anims, fonts, x, y);
    if (idx >= 0) {
        float s = (float)(scale * g_game->motif_scale);
        SysElem_SetScale(idx, s, sprite_scale);
    }
    return SysDraw_DrawItem(list, idx);
}

/* 0x00411F30: SysElem_Add with the fight.def sprites, animations and fonts, scaled to the motif. */
int SysElem_AddFight(const SysElem *elem, float x, float y)
{
    GameVars *g = g_game;
    int idx = SysElem_Add(elem, g->sysexplods, g->fight.sff, g->fight.anim, (void **)g->fight.fonts, x, y);

    if (idx >= 0) {
        float s = (float)g_game->motif_scale;
        SysElem_SetScale(idx, s, 1.0f);
        return idx;
    }
    return idx;
}

/* 0x00411FB0: SysElem_Add with the motif's sprites, animations and fonts, scaled to the motif. */
int SysElem_AddMotif(const SysElem *elem, float x, float y)
{
    GameVars *g = g_game;
    int idx = SysElem_Add(elem, g->sysexplods, g->motif_sprites, g->motif_anims, g->motif_fonts, x, y);

    if (idx >= 0) {
        float s = (float)g_game->motif_scale;
        SysElem_SetScale(idx, s, 1.0f);
        return idx;
    }
    return idx;
}

/* 0x00412030: draws a fight.def element of layer `layer` (-1: any); the index of its item, or -1. */
int SysElem_DrawFight(const SysElem *elem, int layer, float x, float y)
{
    GameVars *g = g_game;
    int idx;

    if (elem->layerno != layer && layer != -1) {
        return -1;
    }
    idx = SysElem_Add(elem, g->sysexplods, g->fight.sff, g->fight.anim, (void **)g->fight.fonts, x, y);
    if (idx >= 0) {
        float s = (float)g_game->motif_scale;
        SysElem_SetScale(idx, s, 1.0f);
        idx = SysDraw_DrawItem(g_game->sysexplods, idx);
    }
    return idx;
}

/* 0x004120D0: draws a layer 0 element of the motif at x, y. */
void SysElem_DrawMotif(const SysElem *elem, float x, float y)
{
    GameVars *g = g_game;

    SysElem_Draw(0, g->sysexplods, g->motif_sprites, g->motif_anims, elem, g->motif_fonts, x, y, 1.0f, 1.0f);
}
