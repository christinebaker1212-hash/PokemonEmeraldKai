/*
 * The match (original module 0x4124A0-0x419A4F, strings "Gameflow %i" to
 * "State no: %i; ..."): this file holds the fight screen helpers of the module:
 * the stage back layer, the HUD (life and power bars, faces, names, timer,
 * win icons) and the bar ticks, the afterimages of projectiles and the
 * debug display (collision boxes, the Ctrl+D text). The other parts are in
 * gameflow.c, round.c and camera.c.
 */
#include "mugen.h"

/*
 * 0x00418CD0: the life shown by the life bar of p (Player.lifebar_life) goes
 * down towards its life by a tenth of the difference plus 0.9% of the
 * maximum each tick, or up by at most 4% of the maximum; once it has caught
 * up, the red part (lifebar_red) follows it down by a tenth of the
 * difference, except while p is being hit (lying down in state 5150 for
 * more than 60 ticks excepted). Nothing moves while paused or in slow
 * motion.
 */
void Lifebar_TickLife(Player *p)
{
    GameVars *g = g_game;
    int step_down = p->life_max * 9 / 1000;
    int step_up = p->life_max * 40 / 1000;
    int life, shown;

    if (step_down == 0) {
        step_down = 1;
    }
    if (step_up == 0) {
        step_up = 1;
    }
    if (g->defocus_paused != 0 || g->slow_counter != 0) {
        return;
    }
    life = p->life;
    shown = p->lifebar_life;
    if (shown > life) {
        shown = Ftol((double)shown - ((double)(shown - life) * 0.1 + step_down));
        p->lifebar_life = shown;
        if (shown < life) {
            p->lifebar_life = life;
        }
    }
    shown = p->lifebar_life;
    if (shown < life) {
        int d = life - shown;
        if (d >= step_up) {
            d = step_up;
        }
        p->lifebar_life = shown + d;
    }
    if (p->movetype == 2 && (p->state_no != 5150 || p->state_time <= 60)) {
        return;
    }
    shown = p->lifebar_life;
    if (shown != life) {
        return;
    }
    if (p->lifebar_red > shown) {
        int red = p->lifebar_red;
        p->lifebar_red = Ftol((double)red - (double)(red - shown) * 0.1);
    }
    if (p->lifebar_red < shown) {
        p->lifebar_red = shown;
    }
}

/*
 * 0x00418FB0: the power shown by the power bar of p (Player.powerbar_power) rises
 * towards its power by a twentieth of the difference plus 15 each tick
 * (not while paused or in slow motion), and a new power level (thousands,
 * powerbar_level) plays its [Powerbar] level sound when it goes up.
 */
void Lifebar_TickPower(Player *p)
{
    GameVars *g = g_game;
    int level;

    if (g->defocus_paused == 0 && g->slow_counter == 0) {
        int shown = p->powerbar_power;
        int power = p->power;
        if (shown < power) {
            p->powerbar_power = (power - shown) / 20 + shown + 15;
        }
        if (p->powerbar_power > power) {
            p->powerbar_power = power;
        }
    }
    level = p->power / 1000;
    if (p->powerbar_level == level) {
        return;
    }
    if (p->powerbar_level < level && level < 10) {
        Sound_PlaySystem(g->fight.snd, g->fight.level_snd_group[level], -1, g->fight.level_snd_number[level], 100.0);
    }
    p->powerbar_level = p->power / 1000;
}

/* x * scale rounded to an int as the bar code does it: through float, floor(x + 0.5), float again. */
static int Mt_Scale(int v, float scale)
{
    float f = (float)((double)v * scale);

    f = (float)floor(f + 0.5);
    return Ftol(f);
}

/* The clip rectangle of the render target (g_clipX1..g_clipY2), saved by the bar drawing. */
typedef struct MtClip {
    int x1, x2, y1, y2;
} MtClip;

static void Mt_SaveClip(MtClip *c)
{
    c->x1 = g_clipX1;
    c->x2 = g_clipX2;
    c->y1 = g_clipY1;
    c->y2 = g_clipY2;
}

/*
 * 0x00418790: the mid and front parts of the life bar of p, clipped to the
 * part of [range] they show: front to the life shown (lifebar_life), mid to
 * the red part (lifebar_red); a range going right to left empties from the
 * left. With [Debug] Debug the life and the red part beyond it are
 * printed next to the bar (front layer only). The clip rectangle is
 * restored.
 */
void Lifebar_Draw(Player *p, FightBar *bar, int layer)
{
    MtClip clip;
    float s, px, py;
    int x1, x2, y, w, red, shown, len, dir = -1;

    Mt_SaveClip(&clip);
    s = (float)g_game->motif_scale;
    x1 = Mt_Scale(bar->range[0] + bar->pos[0], s);
    y = Mt_Scale(bar->pos[1], s);
    x2 = Mt_Scale(bar->range[1] + bar->pos[0], s);
    if (x1 > x2) {
        int t = x1;
        x1 = x2;
        x2 = t;
        dir = 1;
    }
    w = x2 - x1;
    red = p->lifebar_red * w / p->life_max;
    shown = p->lifebar_life * w / p->life_max;
    len = shown + 1;
    if (p->lifebar_life == 0) {
        len = 0;
    }
    if (len > red) {
        len = red;
    }
    Gfx_SetClipRect(x1, clip.y1, x2 + 1, clip.y2);
    px = (float)bar->pos[0];
    py = (float)bar->pos[1];
    if (dir == 1) {
        if (p->lifebar_red > p->lifebar_life) {
            Gfx_SetClipRect(x2 - red, clip.y1, x2 - len + 1, clip.y2);
        } else {
            Gfx_SetClipRect(0, 0, 1, 1);
        }
        SysElem_DrawFight(&bar->mid, layer, px, py);
        if (p->lifebar_life > 0) {
            Gfx_SetClipRect(x2 - shown, clip.y1, x2 + 1, clip.y2);
        } else {
            Gfx_SetClipRect(0, 0, 1, 1);
        }
        SysElem_DrawFight(&bar->front, layer, px, py);
        if (g_game->debug != 0 && bar->front.layerno == layer) {
            char text[0x100];
            Gfx_SetClipRect(x1, 0, x2 + 1, g_targetHeight);
            _snprintf(text, sizeof(text), "%i", p->life);
            Font_DrawText(text, g_game->font_small, x2 - shown + 1, y + 5, 1, 0, 0x80, 0x80, 0);
            _snprintf(text, sizeof(text), "%3i", p->lifebar_red - p->life);
            if (p->lifebar_red - p->life > 0) {
                Font_DrawText(text, g_game->font_small, x2 - len - 11, y + 5, 1, 0, 0x80, 0, 0);
            }
        }
    } else {
        if (p->lifebar_red > p->lifebar_life) {
            Gfx_SetClipRect(x1 + len, clip.y1, x1 + red + 1, clip.y2);
        } else {
            Gfx_SetClipRect(0, 0, 1, 1);
        }
        SysElem_DrawFight(&bar->mid, layer, px, py);
        if (p->lifebar_life > 0) {
            Gfx_SetClipRect(x1, clip.y1, x1 + shown + 1, clip.y2);
        } else {
            Gfx_SetClipRect(0, 0, 1, 1);
        }
        SysElem_DrawFight(&bar->front, layer, px, py);
        if (g_game->debug != 0 && bar->front.layerno == layer) {
            char text[0x100];
            Gfx_SetClipRect(x1, 0, x2 + 1, g_targetHeight);
            _snprintf(text, sizeof(text), "%i", p->life);
            Font_DrawText(text, g_game->font_small, shown + x1 - 1, y + 5, -1, 0, 0x80, 0x80, 0);
            _snprintf(text, sizeof(text), "%i", p->lifebar_red - p->life);
            if (p->lifebar_red - p->life > 0) {
                Font_DrawText(text, g_game->font_small, len + x1 + 1, y + 5, 1, 0, 0x80, 0, 0);
            }
        }
    }
    Gfx_SetClipRect(clip.x1, clip.y1, clip.x2, clip.y2);
}

/*
 * 0x00418E20: the front part of the power bar of team `team` (1, 2) for p,
 * clipped to the power shown (powerbar_power) between x1 and x2; a range going
 * right to left empties from the left. The clip rectangle is restored.
 */
void Powerbar_Draw(int team, Player *p, int x1, int x2, int layer)
{
    GameVars *g = g_game;
    FightBar *bar = &g->fight.powerbar[team - 1];
    MtClip clip;
    float s;
    int a, b, len, dir = -1;

    Mt_SaveClip(&clip);
    s = (float)g->motif_scale;
    a = Mt_Scale(x1, s);
    b = Mt_Scale(x2, s);
    if (a > b) {
        int t = a;
        a = b;
        b = t;
        dir = 1;
    }
    len = (b - a + 1) * p->powerbar_power / *p->power_max_ref;
    if (dir == 1) {
        if (len > 0) {
            Gfx_SetClipRect(b - len + 1, 0, b + 1, g_targetHeight);
        } else {
            Gfx_SetClipRect(0, 0, 1, 1);
        }
    } else {
        if (len > 0) {
            Gfx_SetClipRect(a, 0, len + a, g_targetHeight);
        } else {
            Gfx_SetClipRect(0, 0, 1, 1);
        }
    }
    SysElem_DrawFight(&bar->front, layer, (float)bar->pos[0], (float)bar->pos[1]);
    Gfx_SetClipRect(clip.x1, clip.y1, clip.x2, clip.y2);
}

/*
 * A face (the character's own sprites) or turns teammate face of the HUD:
 * added to the system draw list with the sprite scale, drawn at once
 * (SysElem_DrawFight with other sprites, inlined by Fight_DrawHud).
 */
static void Mt_DrawFace(const SysElem *elem, int layer, SpriteSet *sprites, Anim *anims, float x, float y,
                        float sprite_scale)
{
    GameVars *g = g_game;
    SysDrawList *list = g->sysexplods;
    int idx;

    if (elem->layerno != layer && layer != -1) {
        return;
    }
    idx = SysElem_Add(elem, list, sprites, anims, (void **)g->fight.fonts, x, y);
    g = g_game;
    if (idx >= 0) {
        float ms = (float)g->motif_scale;
        SysElem_SetScale(idx, ms, sprite_scale);
    }
    if (idx != -1) {
        SysDraw_DrawItem(list, idx);
    }
}

/* The life bars (simul, turns or single) of side `side`, and how many player slots they cover (by 2). */
static FightBar *Mt_SideBars(int side, int *count)
{
    GameVars *g = g_game;

    if (g->players[3 + side] != NULL) {
        *count = 3;
        return g->fight.simul_lifebar;
    }
    *count = 1;
    return g->teams[side].member_count >= 2 ? g->fight.turns_lifebar : g->fight.lifebar;
}

static FightFace *Mt_SideFaces(int side, int *count)
{
    GameVars *g = g_game;

    if (g->players[3 + side] != NULL) {
        *count = 3;
        return &g->fight.simul_face;
    }
    *count = 1;
    return g->teams[side].member_count >= 2 ? &g->fight.turns_face : &g->fight.face;
}

static FightName *Mt_SideNames(int side, int *count)
{
    GameVars *g = g_game;

    if (g->players[3 + side] != NULL) {
        *count = 3;
        return &g->fight.simul_name;
    }
    *count = 1;
    return g->teams[side].member_count >= 2 ? &g->fight.turns_name : &g->fight.name;
}

/*
 * 0x00417880: the fight HUD elements of layer `layer` (-1: all): life bars
 * (backgrounds, then mid and front parts; simul bars with 4 players, turns
 * bars for teams), power bars with their level counters, the turns
 * teammates (backgrounds, faces with their sprite file, KO marks), faces
 * (with the character's sprites at its localcoord scale), names, the
 * timer ("o" without a time limit) and the win icons (or a counter past
 * useiconupto). Drawn with the PalFX of palette group 0x200.
 *
 * Quirk kept: the loops over the bars and faces of a side step by 2 player
 * slots (players 1/3 on side 1, 2/4 on side 2).
 */
void Fight_DrawHud(int layer)
{
    GameVars *g = g_game;
    FightDef *fd = &g->fight;
    FightTime *tm = &fd->time;
    FightWinIcon *wi = &fd->winicon;
    int side, k, i, n;

    g_palFxCur = &g->palgroup_200_palfx;
    /* life bar backgrounds */
    for (k = 0; k < 3; k++) {
        for (side = 0; side < 2; side++) {
            FightBar *bars = Mt_SideBars(side, &n);
            for (i = side; i < side + n; i += 2) {
                FightBar *bar = &bars[i];
                SysElem_DrawFight(&bar->bg[k], layer, (float)bar->pos[0], (float)bar->pos[1]);
            }
            g = g_game;
        }
    }
    /* life bars */
    for (side = 0; side < 2; side++) {
        FightBar *bars = Mt_SideBars(side, &n);
        for (i = side; i < side + n; i += 2) {
            FightBar *bar = &bars[i];
            Lifebar_Draw(g->players[1 + i], bar, layer);
            g = g_game;
        }
    }
    /* power bars */
    for (k = 0; k < 3; k++) {
        for (side = 0; side < 2; side++) {
            FightBar *bar = &fd->powerbar[side];
            SysElem_DrawFight(&bar->bg[k], layer, (float)bar->pos[0], (float)bar->pos[1]);
        }
    }
    for (side = 0; side < 2; side++) {
        FightBar *bar = &fd->powerbar[side];
        Powerbar_Draw(side + 1, g_game->players[1 + side], bar->pos[0] + bar->range[0], bar->pos[0] + bar->range[1],
                      layer);
    }
    for (side = 0; side < 2; side++) {
        FightBar *bar = &fd->powerbar[side];
        SysElem *counter = &fd->powerbar_counter[side];
        _snprintf(counter->text, 0x40, "%i", g_game->players[1 + side]->powerbar_power / 1000);
        SysElem_DrawFight(counter, layer, (float)bar->pos[0], (float)bar->pos[1]);
    }
    g = g_game;
    /* turns teammates: backgrounds */
    for (side = 0; side < 2; side++) {
        FightTeammate *mate = &fd->turns_teammate[side];
        int x, y;
        if (g->survival_side == 2 - side || g->teams[side].member_count < 2) {
            continue;
        }
        x = mate->pos[0];
        y = mate->pos[1];
        for (k = 0; k < g->teams[side].member_count - 1; k++) {
            float fx = (float)x, fy = (float)y;
            for (i = 0; i < 3; i++) {
                SysElem_DrawFight(&mate->bg[i], layer, fx, fy);
            }
            x += mate->spacing[0];
            y += mate->spacing[1];
        }
        g = g_game;
    }
    /* face backgrounds */
    for (side = 0; side < 2; side++) {
        FightFace *faces = Mt_SideFaces(side, &n);
        for (i = side; i < side + n; i += 2) {
            for (k = 0; k < 3; k++) {
                SysElem_DrawFight(&faces->p[i].bg[k], layer, (float)faces->pos_x[i],
                                  (float)faces->pos_y[i]);
            }
        }
        g = g_game;
    }
    /* faces, last slot first */
    for (side = 0; side < 2; side++) {
        FightFace *faces = Mt_SideFaces(side, &n);
        for (i = n + side - 1; i >= 0; i -= 2) {
            Player *p = g->players[1 + i];
            CharData *c = p->data;
            float fx = (float)faces->pos_x[i];
            float fy = (float)faces->pos_y[i];
            float scale = (float)(g_gameCoord[0] / c->localcoord[0] / g->motif_scale);
            Mt_DrawFace(&faces->p[i].face, layer, p->sprites, c->anim, fx, fy, scale);
            g = g_game;
        }
    }
    /* turns teammates: faces of the members waiting */
    for (side = 0; side < 2; side++) {
        FightTeammate *mate = &fd->turns_teammate[side];
        TeamSel *ts = &g->teams[side];
        int x, y;
        if (g->survival_side == 2 - side || ts->member_count < 2) {
            continue;
        }
        x = mate->pos[0];
        y = mate->pos[1];
        for (k = 0; k < ts->member_count; k++) {
            SpriteSet *set;
            if (k == ts->cur_member) {
                continue;
            }
            set = g->team_faces[side].sprites[k];
            if (set != NULL) {
                float scale = g->team_faces[side].scale[k];
                Mt_DrawFace(&mate->face, layer, set, fd->anim, (float)x, (float)y, scale);
                g = g_game;
            }
            x += mate->spacing[0];
            y += mate->spacing[1];
        }
    }
    /* turns teammates: KO marks of the members gone */
    for (side = 0; side < 2; side++) {
        FightTeammate *mate = &fd->turns_teammate[side];
        TeamSel *ts = &g->teams[side];
        int x, y;
        if (ts->member_count < 2) {
            continue;
        }
        x = mate->pos[0];
        y = mate->pos[1];
        for (k = 0; k < ts->cur_member; k++) {
            SysElem_DrawFight(&mate->ko, layer, (float)x, (float)y);
            x += mate->spacing[0];
            y += mate->spacing[1];
        }
        g = g_game;
    }
    /* name backgrounds, names */
    for (side = 0; side < 2; side++) {
        FightName *names = Mt_SideNames(side, &n);
        for (i = side; i < side + n; i += 2) {
            SysElem_DrawFight(&names->bg[i], layer, (float)names->pos_x[i], (float)names->pos_y[i]);
        }
        g = g_game;
    }
    for (side = 0; side < 2; side++) {
        FightName *names = Mt_SideNames(side, &n);
        for (i = side; i < side + n; i += 2) {
            strncpy(names->name[i].text, g->players[1 + i]->data->displayname, 0x3F);
            SysElem_DrawFight(&names->name[i], layer, (float)names->pos_x[i],
                              (float)names->pos_y[i]);
            g = g_game;
        }
    }
    /* the timer */
    SysElem_DrawFight(&tm->bg, layer, (float)tm->pos[0], (float)tm->pos[1]);
    if (g_game->round_time != -1) {
        _snprintf(tm->counter.text, 0x40, "%i", g_game->round_time / tm->frames_per_count);
    } else {
        _snprintf(tm->counter.text, 0x40, "%c", 'o');
    }
    SysElem_DrawFight(&tm->counter, layer, (float)tm->pos[0], (float)tm->pos[1]);
    /* win icons */
    g = g_game;
    for (side = 0; side < 2; side++) {
        int x = wi->pos_x[side];
        int y = wi->pos_y[side];
        int upto = wi->useiconupto;
        int wins = g->rounds_won[side];
        if (wins > upto) {
            float fy = (float)y, fx = (float)x;
            SysElem_DrawFight(&wi->bg0[side], layer, fx, fy);
            _snprintf(wi->counter[side].text, 0x40, "%i", g_game->rounds_won[side]);
            SysElem_DrawFight(&wi->counter[side], layer, fx, fy);
            g = g_game;
            continue;
        }
        n = g->teams[side].wins_needed;
        if (n > upto) {
            n = upto;
        }
        for (k = 0; k < n; k++) {
            SysElem_DrawFight(&wi->bg0[side], layer, (float)(x + k * wi->iconoffset_x[side]),
                              (float)(y + k * wi->iconoffset_y[side]));
        }
        g = g_game;
        for (k = 0; k < g->rounds_won[side]; k++) {
            int type = g->win_icons[side][k];
            if (type != -1) {
                SysElem_DrawFight(&wi->icon[side][type], layer,
                                  (float)(x + k * wi->iconoffset_x[side]), (float)(y + k * wi->iconoffset_y[side]));
                g = g_game;
            }
        }
        for (k = 0; k < g->rounds_won[side]; k++) {
            int type = g->win_perfect[side][k];
            if (type != -1) {
                SysElem_DrawFight(&wi->icon[side][type], layer,
                                  (float)(x + k * wi->iconoffset_x[side]), (float)(y + k * wi->iconoffset_y[side]));
                g = g_game;
            }
        }
    }
    g_palFxCur = NULL;
}

/* 0x00418C60: the life bars of players 1-4, then the power bars of players 1 and 2. */
void Lifebars_Tick(void)
{
    GameVars *g = g_game;
    Player *p1 = g->players[1];
    Player *p2 = g->players[2];
    Player *p3 = g->players[3];
    Player *p4 = g->players[4];

    if (p1 != NULL) {
        Lifebar_TickLife(p1);
    }
    if (p2 != NULL) {
        Lifebar_TickLife(p2);
    }
    if (p3 != NULL) {
        Lifebar_TickLife(p3);
    }
    if (p4 != NULL) {
        Lifebar_TickLife(p4);
    }
    if (p1 != NULL) {
        Lifebar_TickPower(p1);
    }
    if (p2 != NULL) {
        Lifebar_TickPower(p2);
    }
}
/* 0x00419080: the first afterimage buffer that no animation uses, or NULL. */
AfterImage *AfterImages_FindFree(KeyedArray *list)
{
    int i;

    for (i = 0; i < list->capacity; i++) {
        int slot = KeyedArray_Find(list, 0, i);
        AfterImage *ai = slot != -1 ? (AfterImage *)(list->data + list->elem_size * slot) : NULL;
        /* a missing buffer is not checked for (the original crashes) */
        if (ai->anim == NULL) {
            return ai;
        }
    }
    return NULL;
}

/*
 * 0x004190D0: records a frame in the afterimage of every projectile that has
 * one and is not paused; the projectile lets go of the buffer when it is over.
 */
void Projectiles_UpdateAfterImages(ProjectileList *pl)
{
    KeyedArray *list = pl->list;
    Projectile *e;

    for (e = KeyedArray_First(list); e != NULL; e = KeyedArray_Next(list)) {
        AfterImage *ai = e->afterimage;
        if (e->paused == 0 && ai != NULL) {
            if (!AfterImage_Update(&e->draw, ai, e->remap, e->hitpause_time == 0)) {
                ai->anim = NULL;
                e->afterimage = NULL;
            }
        }
    }
}

/* 0x00419160: draws the afterimages of the projectiles. */
void Projectiles_DrawAfterImages(ProjectileList *pl)
{
    KeyedArray *list = pl->list;
    Projectile *e;

    for (e = KeyedArray_First(list); e != NULL; e = KeyedArray_Next(list)) {
        if (e->afterimage != NULL) {
            AfterImage_Draw(e->afterimage, e->sprpriority);
        }
    }
}

/*
 * 0x004177A0: the stage background layer 0 (with the stage PalFX), then
 * black bars over the screen rows the stage does not cover when the camera
 * crop shifts it (Camera_UpdateCrop), past [Camera] overdrawlow (below) or
 * overdrawhigh (above).
 */
void Match_DrawStageBack(void)
{
    GameVars *g = g_game;
    int shift, h;

    if (g->stage_bg == NULL) {
        return;
    }
    g_palFxCur = &g->stage_palfx;
    Bg_Draw(g->stage_bg, 0);
    g = g_game;
    shift = g->camera_y_base;
    if (shift > 0 || g->crop_bottom < 0) {
        h = Ftol(g_gameCoord[0] / g->stage.unit * shift - g->crop_bottom - g->stage.overdrawlow);
        if (h > 0) {
            g_gfxFillRect(0, g_targetHeight - 1, g_targetWidth - 1, g_targetHeight - h, 0);
        }
    } else if (shift < 0 || g->crop_top < 0) {
        h = Ftol(-(g_gameCoord[0] / g->stage.unit * shift) - g->crop_top - g->stage.overdrawhigh);
        if (h > 0) {
            g_gfxFillRect(0, 0, g_targetWidth - 1, h - 1, 0);
        }
    }
    g_palFxCur = NULL;
}
/*
 * 0x004191D0: draws the boxes of a collision box list (debug display) at
 * x, y, scaled and mirrored by flip bit 0: a grey outline, then the box in
 * `color`, filled or as an outline. Nothing is drawn with a zero scale.
 */
void Clsn_Draw(const Clsn *clsn, float x, float y, float sx, float sy, unsigned int color, int flip, int filled)
{
    int dir = (flip & 1) ? -1 : 1;
    int i;

    if (clsn == NULL || sx == 0.0f || sy == 0.0f) {
        return;
    }
    for (i = 0; i < clsn->capacity; i++) {
        const ClsnBox *b = &clsn->boxes[i];
        float fdir;
        int x1, y1, x2, y2;
        unsigned int shade;

        if (b->hidden != 0) {
            continue;
        }
        fdir = (float)dir;
        x1 = Ftol((double)b->x1 * sx * fdir + x);
        y1 = Ftol((double)b->y1 * sy + y);
        x2 = Ftol((double)sx * b->x2 * fdir + x);
        y2 = Ftol((double)sy * b->y2 + y);
        shade = Video_MapRGB(0x20, 0x20, 0x20);
        ((GfxRectFn)g_gfxDrawRectSub)(x1, y1, x2, y2, shade);
        if (filled) {
            g_gfxFillRectColor(x1, y1, x2, y2, color);
        } else {
            ((GfxRectFn)g_gfxDrawRectAdd)(x1, y1, x2, y2, color);
        }
    }
}
/*
 * 0x004195D0: the debug display (Ctrl+D): the debug clipboard, the frame
 * counter and rate, the speed settings and, for player `index` (none with
 * 0), its name, animation, state and own clipboard.
 */
void Debug_Draw(int index)
{
    GameVars *g = g_game;
    Player *p = g->players[index];
    char text[0x100];
    char extra[0x40];
    float fps;
    int y, action, spr_group, spr_number;

    Clipboard_Draw(g->clipboard, g->clipboard_lines, g->font_small, 40);
    if (index == 0) {
        return;
    }
    y = g_targetHeight - 39;
    fps = Timer_GetFps();
    if (!(fps > 0.0f)) {
        fps = 0.0f;
    }
    _snprintf(text, sizeof(text), "Frames: %i (%.1f FPS)", g_game->frames, (double)fps);
    Font_DrawText(text, g_game->font_small, 10, y, 1, 0, 0x100, 0x100, 0x100);
    g = g_game;
    _snprintf(text, sizeof(text), "VRet: %i, Speed: %i, Skip: %c", g->vretrace_on, g->speedup,
              g->frame_skip == -1 ? 'a' : g->frame_skip + '0');
    Font_DrawText(text, g_game->font_small, 110, y, 1, 0, 0x100, 0x100, 0x100);
    if (p == NULL || !p->active) {
        _snprintf(text, sizeof(text), "Player %i Disabled", index - 1);
        Font_DrawText(text, g_game->font_small, 10, y + 8, 1, 0, 0xC8, 0xC8, 0xDC);
        return;
    }
    if (p->id == index - 1) {
        _snprintf(text, sizeof(text), "%s %i", p->name, p->id);
    } else {
        _snprintf(text, sizeof(text), "%s %i (%i)", p->name, p->id, index - 1);
    }
    if (p->custom_states != NULL) {
        Player *o = g_game->players[p->custom_owner];
        _snprintf(extra, sizeof(extra), " (in %s %i's state rc)", o->name, o->id);
        strncat(text, extra, 0xFF - strlen(text));
    }
    Font_DrawText(text, g_game->font_small, 10, y + 8, 1, 0, 0xC8, 0xC8, 0xDC);
    {
        AnimState *st = p->anim;
        KeyedArray *actions = st->anim->actions;
        AnimAction *a = (AnimAction *)(actions->data + actions->elem_size * st->action);

        spr_number = st->elem != NULL ? st->elem->number : -1;
        spr_group = st->elem != NULL ? st->elem->group : -1;
        action = actions->slots[st->action].key2;
        _snprintf(text, sizeof(text), "ActionID: %i; Spr: %i,%i; Elemno: %i/%i; Time: %i/%i", action, spr_group,
                  spr_number, st->elem_index + 1, a->elem_count, st->time, a->total_time);
    }
    if (p->action_failed != 0) {
        strncat(text, " (invalid)", 0xFF - strlen(text));
    }
    if (p->anim_from_owner == 0) {
        Font_DrawText(text, g_game->font_small, 10, y + 15, 1, 0, 0x100, 0x100, 0x100);
    } else {
        Font_DrawText(text, g_game->font_small, 10, y + 15, 1, 0, 0x100, 0x100, 0x80);
    }
    _snprintf(text, sizeof(text), "State no: %i; Ctrl: %i; Type: %i; Movetype %i; Time: %i", p->state_no, p->ctrl,
              p->statetype, p->movetype, p->state_time - 1);
    if (p->state_exists == 0 && p->state_no >= 200) {
        strncat(text, " (invalid)", 0xFF - strlen(text));
    }
    if (p->custom_states == NULL) {
        Font_DrawText(text, g_game->font_small, 10, y + 22, 1, 0, 0x100, 0x100, 0x100);
    } else {
        Font_DrawText(text, g_game->font_small, 10, y + 22, 1, 0, 0x100, 0x100, 0x80);
    }
    Clipboard_Draw(p->warnings, -1, g_game->font_small, g_targetHeight - 10);
}
