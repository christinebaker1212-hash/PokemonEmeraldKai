/*
 * Moemon Fight: the fight HUD fitted to the picture (docs/FIGHT_DECISIONS.md #5, #7).
 *
 * A 4:3 screenpack is drawn centred in the 3:2 picture (MfHost_FitMotif). Its fight HUD is then
 * measured, sprite by sprite, in the pack's own coordinates, when a session starts (MfFit_Hud):
 *   - each player's part moves out to its real edge of the picture, as far as the pack had it
 *     from the edge of its own screen, but never past the safe margin;
 *   - the top part (whatever sits in the top half) goes to the cabinet's hud.top, pixels from the
 *     top of the picture (cabinets.ini; -1 keeps the pack's place);
 *   - the bottom part only moves when it is past the safe margin, back inside;
 *   - a sprite across the middle of the screen belongs to both sides (SFEX draws both players'
 *     round boxes as one strip) and keeps its centred place.
 * What moved, and how far, goes to mugen/fightfit.log.
 */
#include <math.h>

#include "mugen.h"
#include "mf_compat.h"

#define FIT_MARGIN 8        /* the safe margin, in picture pixels */

int g_mfMotifOffsetX;
int g_mfHudShift;
int g_mfHudSideX[2];
int g_mfHudShiftY;
int g_mfHudTopY, g_mfHudBottomY;
int g_mfHudVertical;

typedef struct FitBox {
    float l, t, r, b;
    int have;
} FitBox;

static void Box_Add(FitBox *box, float l, float t, float r, float b)
{
    if (!box->have) {
        box->l = l;
        box->t = t;
        box->r = r;
        box->b = b;
        box->have = 1;
        return;
    }
    box->l = l < box->l ? l : box->l;
    box->t = t < box->t ? t : box->t;
    box->r = r > box->r ? r : box->r;
    box->b = b > box->b ? b : box->b;
}

/* A sprite at (x, y) with its axis, scale and facing */
static void Box_AddSprite(FitBox *box, const Sprite *spr, float x, float y, float sx, float sy, int flip)
{
    float l;

    if (spr == NULL || spr->w <= 0 || spr->h <= 0) {
        return;
    }
    l = flip ? x - (spr->w - spr->x) * sx : x - spr->x * sx;
    Box_Add(box, l, y - spr->y * sy, l + spr->w * sx, y - spr->y * sy + spr->h * sy);
}

/* Where a fight.def element can draw at (px, py), over every frame of an animation. Fonts aren't measured. */
static FitBox Fit_Element(const SysElem *el, float px, float py)
{
    FightDef *fd = &g_game->fight;
    FitBox box = {0, 0, 0, 0, 0};
    float x = px + el->x, y = py + el->y;
    float sx = fabsf(el->scale_x), sy = fabsf(el->scale_y);
    int flip = el->facing < 0;

    if (sx == 0.0f || sy == 0.0f) {
        return box;     /* scale 0: never shown */
    }
    if (el->type == SYSELEM_SPRITE) {
        Box_AddSprite(&box, SpriteSet_FindSprite(fd->sff, el->group, el->number), x, y, sx, sy, flip);
    } else if (el->type == SYSELEM_ANIM && fd->anim != NULL) {
        AnimAction *action = (AnimAction *)KeyedArray_Get(fd->anim->actions, 0, el->group);
        int i;
        for (i = 0; action != NULL && i < action->elem_count; i++) {
            AnimElem *ae = (AnimElem *)KeyedArray_Get(action->elems, 0, i);
            if (ae == NULL || ae->group < 0) {
                continue;
            }
            Box_AddSprite(&box, SpriteSet_FindSprite(fd->sff, ae->group, ae->number), x + ae->dp.x * sx,
                          y + ae->dp.y * sy, sx, sy, flip);
        }
    }
    return box;
}

static int Fit_CrossesMiddle(const FitBox *box)
{
    float middle = (float)(g_game->motif_coord[0] / 2.0);

    return box->l < middle - 1.0f && box->r > middle + 1.0f;
}

/* The HUD's parts while it is measured */
typedef struct FitParts {
    FitBox side[2];     /* each player's part, in the pack's coordinates */
    FitBox top;
    FitBox bottom;
} FitParts;

static void Parts_Add(FitParts *parts, int side, FitBox box)
{
    if (!box.have) {
        return;
    }
    if (side >= 0 && !Fit_CrossesMiddle(&box)) {
        Box_Add(&parts->side[side], box.l, box.t, box.r, box.b);
    }
    if ((box.t + box.b) / 2.0f < (float)(g_game->motif_coord[1] / 2.0)) {
        Box_Add(&parts->top, box.l, box.t, box.r, box.b);
    } else {
        Box_Add(&parts->bottom, box.l, box.t, box.r, box.b);
    }
}

static void Parts_AddBar(FitParts *parts, int side, const FightBar *bar)
{
    float x = (float)bar->pos[0], y = (float)bar->pos[1];
    int k;

    for (k = 0; k < 3; k++) {
        Parts_Add(parts, side, Fit_Element(&bar->bg[k], x, y));
    }
    Parts_Add(parts, side, Fit_Element(&bar->mid, x, y));
    Parts_Add(parts, side, Fit_Element(&bar->front, x, y));
}

void MfFit_Hud(int top_px)
{
    GameVars *g = g_game;
    FightDef *fd = &g->fight;
    FitParts parts;
    float s = (float)g->motif_scale;
    float off = (float)g_mfMotifOffsetX;
    float w = (float)g->game_coord[0], h = (float)g->game_coord[1];
    FILE *log;
    int side, k, n;

    memset(&parts, 0, sizeof(parts));
    for (side = 0; side < 2; side++) {
        const FightFacePlayer *face = &fd->face.p[side];
        const FightWinIcon *wi = &fd->winicon;
        Parts_AddBar(&parts, side, &fd->lifebar[side]);
        Parts_AddBar(&parts, side, &fd->powerbar[side]);
        for (k = 0; k < 3; k++) {
            Parts_Add(&parts, side, Fit_Element(&face->bg[k], (float)fd->face.pos_x[side], (float)fd->face.pos_y[side]));
        }
        Parts_Add(&parts, side, Fit_Element(&fd->name.bg[side], (float)fd->name.pos_x[side], (float)fd->name.pos_y[side]));
        n = fd->round.match_wins < wi->useiconupto ? fd->round.match_wins : wi->useiconupto;
        for (k = 0; k < n; k++) {
            float x = (float)(wi->pos_x[side] + k * wi->iconoffset_x[side]);
            float y = (float)(wi->pos_y[side] + k * wi->iconoffset_y[side]);
            int t;
            Parts_Add(&parts, side, Fit_Element(&wi->bg0[side], x, y));
            for (t = 0; t < WIN_ICON_COUNT; t++) {
                Parts_Add(&parts, side, Fit_Element(&wi->icon[side][t], x, y));
            }
        }
    }
    Parts_Add(&parts, -1, Fit_Element(&fd->time.bg, (float)fd->time.pos[0], (float)fd->time.pos[1]));

    /* each side to its real edge, inside the margin */
    g_mfHudSideX[0] = -g_mfMotifOffsetX;
    g_mfHudSideX[1] = g_mfMotifOffsetX;
    if (parts.side[0].have) {
        float left = parts.side[0].l * s + off;
        g_mfHudSideX[0] = (int)floorf(fmaxf(-off, FIT_MARGIN - left) + 0.5f);
    }
    if (parts.side[1].have) {
        float right = parts.side[1].r * s + off;
        g_mfHudSideX[1] = (int)floorf(fminf(off, w - FIT_MARGIN - right) + 0.5f);
    }
    /* the top part to hud.top (or back inside), the bottom part back inside */
    g_mfHudTopY = 0;
    if (parts.top.have) {
        float top = parts.top.t * s;
        if (top_px >= 0) {
            g_mfHudTopY = (int)floorf(top_px - top + 0.5f);
        } else if (top < FIT_MARGIN) {
            g_mfHudTopY = (int)ceilf(FIT_MARGIN - top);
        }
    }
    g_mfHudBottomY = 0;
    if (parts.bottom.have && parts.bottom.b * s > h - FIT_MARGIN) {
        g_mfHudBottomY = -(int)ceilf(parts.bottom.b * s - (h - FIT_MARGIN));
    }

    log = fopen("fightfit.log", "w");
    if (log != NULL) {
        fprintf(log, "Fight HUD fitted to %gx%g (the pack's %gx%g at %.4f, %d from the left)\n", w, h,
                g->motif_coord[0], g->motif_coord[1], s, g_mfMotifOffsetX);
        for (side = 0; side < 2; side++) {
            if (parts.side[side].have) {
                fprintf(log, "P%d's part (%g..%g in the pack): moved %+d, to %d..%d\n", side + 1, parts.side[side].l,
                        parts.side[side].r, g_mfHudSideX[side],
                        (int)(parts.side[side].l * s + off) + g_mfHudSideX[side],
                        (int)(parts.side[side].r * s + off) + g_mfHudSideX[side]);
            }
        }
        if (parts.top.have) {
            fprintf(log, "Top part (%g..%g in the pack): moved %+d, to %d..%d (hud.top %d)\n", parts.top.t,
                    parts.top.b, g_mfHudTopY, (int)(parts.top.t * s) + g_mfHudTopY,
                    (int)(parts.top.b * s) + g_mfHudTopY, top_px);
        }
        if (parts.bottom.have) {
            fprintf(log, "Bottom part (%g..%g in the pack): moved %+d\n", parts.bottom.t, parts.bottom.b,
                    g_mfHudBottomY);
        }
        fclose(log);
    }
}

void MfFit_PlaceItem(const SysDrawItem *e)
{
    FitBox box = {0, 0, 0, 0, 0};
    const Sprite *spr = NULL;
    float sx = fabsf(e->scale.x), sy = fabsf(e->scale.y);

    if (e->kind == SYSDRAW_SPRITE && e->sprite >= 0 && e->draw_sprites != NULL) {
        KeyedArray *set = e->draw_sprites->sprites;
        spr = (const Sprite *)(set->data + set->elem_size * e->sprite);
    } else if (e->anim != NULL && e->anim->elem != NULL && e->anim->sprites != NULL && e->anim->elem->group >= 0) {
        spr = SpriteSet_FindSprite(e->anim->sprites, e->anim->elem->group, e->anim->elem->number);
    }
    Box_AddSprite(&box, spr, e->pos.x, e->pos.y, sx > 0 ? sx : 1.0f, sy > 0 ? sy : 1.0f, (e->flip & DRAW_FLIP_H) != 0);
    if (g_mfHudShift != 0 && box.have && Fit_CrossesMiddle(&box)) {
        g_mfHudShift = 0;   /* shared by both sides */
    }
    g_mfHudShiftY = 0;
    if (g_mfHudVertical) {
        float middle = box.have ? (box.t + box.b) / 2.0f : e->pos.y;
        g_mfHudShiftY = middle < (float)(g_game->motif_coord[1] / 2.0) ? g_mfHudTopY : g_mfHudBottomY;
    }
}
