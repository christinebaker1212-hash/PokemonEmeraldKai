/*
 * Font references, the game resolution and the versus, victory and win
 * screens (original module 0x45FC80-0x4612DF, strings "%i,%i,%i,%i,%i,%i"
 * and "Setting game resolution %ix%i..." to "Entering win screen.").
 *
 * The three screens are screen loops (Screen_Run, gameloop.c) like the
 * continue screen: the versus screen shows the faces and names of both
 * teams and the match number, the victory screen the winner's portrait,
 * name and win quote, and the win screen (arcade) poses the human team's
 * characters on the stage in state 180 under the motif's wintext.
 */
#define HAVE_STRUCT_TIMESPEC 1      /* the CRT's time.h already has it */
#include <pthread.h>

#include "mugen.h"

/* ---- font references and the game resolution ---- */

/*
 * 0x0045FC80: reads "key = font, bank, align, r, g, b" into spec (font -1
 * when missing or out of 0..9, colour 256 = unchanged). Returns 1 when at
 * least one value was read.
 */
int FontSpec_Read(FontSpec *spec, Ini *ini, const char *key)
{
    int n;

    spec->bank = 0;
    spec->align = 0;
    spec->r = 0;
    spec->g = 0;
    spec->b = 0;
    spec->fight_font = 0;
    spec->b = 0x100;
    spec->g = 0x100;
    spec->r = 0x100;
    spec->font = -1;
    n = Ini_Scanf(ini, key, "%i,%i,%i,%i,%i,%i", &spec->font, &spec->bank, &spec->align, &spec->r, &spec->g,
                  &spec->b);
    if (spec->font < 0 || spec->font >= 10) {
        spec->font = -1;
    }
    return n > 0;
}

/*
 * 0x0045FD00: draws text at (x, y) in motif coordinates with the font of
 * spec (a system.def font, or a fight.def one when spec->fight_font is 1),
 * scaled to the game resolution.
 */
void FontSpec_DrawText(const FontSpec *spec, const char *text, int x, int y)
{
    GameVars *g = g_game;
    void **fonts = spec->fight_font == 1 ? (void **)g->fight.fonts : g->motif_fonts;
    float scale, fx, fy;

    if (spec->font < 0 || spec->font >= 10 || fonts[spec->font] == NULL) {
        return;
    }
    scale = (float)g->motif_scale;
    fy = (float)(y * g->motif_scale);
    fx = (float)(x * g->motif_scale);
    Font_DrawTextScaled(text, fonts[spec->font], fx, fy, scale, scale, spec->align, spec->bank, spec->r, spec->g,
                        spec->b);
}

/*
 * 0x0045FDA0: sets the game resolution (the [Config] GameWidth / GameHeight
 * when either is below 1): GameVars.game_coord and its copy g_gameCoord,
 * a new screen buffer (none in OpenGL safe mode) as the render target, and
 * the motif scale. Returns 0 when the screen buffer cannot be allocated,
 * else 1 (also when nothing changes).
 */
int Game_SetResolution(int width, int height)
{
    GameVars *g = g_game;

    if (width < 1 || height < 1) {
        width = g->game_width;
        height = g->game_height;
    }
    if (g->screen != NULL && width == Ftol(g->game_coord[0]) && height == Ftol(g->game_coord[1])) {
        return 1;
    }
    Log_Printf("Setting game resolution %ix%i...", width, height);
    g = g_game;
    g->game_coord[0] = (double)width;
    g->game_coord[1] = (double)height;
    g->game_coord[2] = (double)((width - 1) / 2);
    g->game_coord[3] = (double)((height - 1) / 2);
    memcpy(g_gameCoord, g->game_coord, sizeof(g->game_coord));
    if (g->screen != NULL) {
        SpriteSet *screen = g->screen;
        if (g_renderTarget == screen) {
            Gfx_SetRenderTarget(NULL, 0);
            g_renderTarget = NULL;
            g_renderTargetSprite = 0;
        }
        SpriteSet_Free(screen);
        g = g_game;
    }
    g->screen = NULL;
    if (g->safe_mode_opengl == 0) {
        int h = Ftol(g->game_coord[1]);
        int w = Ftol(g->game_coord[0]);
        SpriteSet *screen = Screen_Create(g->depth, w, h);
        g_game->screen = screen;
        if (screen == NULL) {
            Error_Printf("Can't allocate screen buffer.\n");
            return 0;
        }
    }
    g = g_game;
    Gfx_SetRenderTarget(g->screen, 0);
    g_gfxClear(0);
    g = g_game;
    g->render_target = g->screen;
    g->motif_scale = g->game_coord[0] / g->motif_coord[0];
    Log_Printf("OK\n");
    return 1;
}

/*
 * 0x0045FF40: sets the game resolution for a width x height coordinate
 * space: with fit, GameWidth wide with the space's aspect ratio (at most
 * 4096 high, narrower then); else the [Config] resolution.
 */
void Gfx_SetCoordSpace(int width, int height, int fit)
{
    if (fit) {
        int w = g_game->game_width;
        int h;
        float ratio = (float)((double)width / (double)height);
        float t = (float)((double)w / ratio);
        t = (float)floor(t + 0.5);
        h = Ftol(t);
        if (h > 0x1000) {
            h = 0x1000;
            t = (float)(ratio * 4096.0);
            t = (float)floor(t + 0.5);
            w = Ftol(t);
        }
        Game_SetResolution(w, h);
        return;
    }
    Game_SetResolution(0, 0);
}

/* ---- common parts of the screens ---- */

/* The state of the versus and victory screens: the select grid cells of each team's members. */
typedef struct VersusState {
    SelectCell *cells[2][4];        /* 0x00 by team side and member slot, NULL for none */
} VersusState;
MUGEN_ASSERT_SIZE(VersusState, 0x20);

/* The state of the win screen. */
typedef struct WinScreenState {
    Player *p;                      /* 0x00 the human team's character */
    Player *partner;                /* 0x04 its partner, or NULL */
} WinScreenState;
MUGEN_ASSERT_SIZE(WinScreenState, 0x08);

/* The cells of the characters of member `member` of both teams. */
static void Vs_FillCells(VersusState *state, int member)
{
    GameVars *g = g_game;
    SelectCell *cells = g->select_info.cells;
    int side, i;

    memset(state, 0, sizeof(*state));
    for (side = 0; side < 2; side++) {
        const TeamMember *m = &g->teams[side].members[member];
        for (i = 0; i < m->count; i++) {
            int ch = m->slots[i].char_index;
            state->cells[side][i] = ch == -1 ? NULL : &cells[g->char_info[ch].cell];
        }
    }
}

/* Starts a palette fade of the whole screen: in (out = 0) or out. */
static void Vs_StartFade(int time, int out)
{
    GameVars *g = g_game;

    g->fade.type = FADE_PALETTE;
    g->fade.ticks = 0;
    g->fade.time = time;
    g->fade.out = out;
}

/* Starts the fade out of the screen. */
static void Vs_FadeOut(ScreenLoop *screen, int time)
{
    screen->fade_out = 1;
    screen->fade_ticks = time;
    Vs_StartFade(time, 1);
}

/*
 * The update callback of the versus and victory screens: during the fade
 * out escape or a quit request ends it at once (returns 1 while it runs);
 * else any button starts it.
 */
static int Vs_Update(ScreenLoop *screen, int fadeout_time)
{
    if (screen->fade_out) {
        if (screen->fade_ticks > 0) {
            if (screen->key == 27 || g_quitRequested) {
                screen->fade_out = 1;
                screen->fade_ticks = 0;
            }
            return 1;
        }
        return 0;
    }
    if (screen->input & 0xFF0) {
        Vs_FadeOut(screen, fadeout_time);
    }
    return 0;
}

/*
 * The tick callback of the versus and victory screens: a screen ended by
 * escape stops the precaching and fades out in 10 ticks; else the fade out
 * starts after `time` ticks.
 */
static int Vs_Tick(ScreenLoop *screen, int time, int fadeout_time)
{
    if (screen->done) {
        if (!screen->fade_out) {
            GameVars *g = g_game;
            pthread_mutex_lock((pthread_mutex_t *)&g->precache_mutex);
            if (g->precache_running) {
                g->precache_stop = 1;
            }
            pthread_mutex_unlock((pthread_mutex_t *)&g->precache_mutex);
            Vs_FadeOut(screen, 10);
        }
        return 1;
    }
    if (!screen->fade_out && screen->ticks >= time) {
        Vs_FadeOut(screen, fadeout_time);
    }
    return 1;
}

/*
 * SysDraw_DrawItem as the screens inline it (the item is freed at the end
 * of its time, as SysDraw_Remove does).
 */
static void Vs_DrawItem(SysDrawList *list, int index)
{
    SysDrawItem *e;

    if (index == -1) {
        return;
    }
    e = &list->items[index];
    if (e->started == 0 && e->kind == SYSDRAW_ANIM) {
        SysDraw_Start(list, index);
    }
    if (g_game->skip_draw == 0) {
        SysDraw_Update(e);
    }
    e->drawn = 1;
    if (e->time <= 1 && e->time != -1) {
        e = &list->items[index];
        if (e->used) {
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
 * A face (portrait) element of layer 0 with the sprites of a cell, scaled
 * to the motif and by the character's own scale. Quirk: SysDraw_DrawItem is
 * called with the result of SysElem_Add even when it failed.
 */
static void Vs_DrawFace(const SysElem *face, const SelectCell *cell)
{
    SysDrawList *list = g_game->sysexplods;
    float scale = cell->scale;
    int idx;

    if (face->layerno != 0) {
        return;
    }
    idx = SysElem_Add(face, list, cell->sprites, NULL, NULL, 0.0f, 0.0f);
    if (idx >= 0) {
        float s = (float)g_game->motif_scale;
        SysElem_SetScale(idx, s, scale);
    }
    SysDraw_DrawItem(list, idx);
}

/* A setup common to the three screens; 0 when the state cannot be allocated. */
static int Vs_Setup(ScreenLoop *screen, int kind, size_t state_size)
{
    memset(screen, 0, sizeof(*screen));
    screen->scale = 1.0f;
    screen->fadein_time = 10;
    screen->fadeout_time = 10;
    screen->kind = kind;
    screen->fps = 60;
    screen->reload_motif = 1;
    screen->start_frame = g_game->frames;
    screen->state = calloc(1, state_size);
    return screen->state != NULL;
}

/* ---- the versus screen ---- */

/* 0x00460010: the cells of the first members, the VS background and music, a fade in. Returns 1. */
int Versus_Init(ScreenLoop *screen, void *state)
{
    GameVars *g = g_game;

    Vs_FillCells(state, 0);
    screen->bg = g->versus_bg;
    screen->scale = (float)g->motif_scale;
    screen->bgm = &g->vs_bgm;
    Vs_StartFade(g->vs_screen.fadein_time, 0);
    Log_Printf("Versus screen init\n");
    return 1;
}

/* 0x00460110 */
int Versus_End(ScreenLoop *screen, void *state)
{
    Log_Printf("End of versus screen loop\n");
    memset(state, 0, sizeof(VersusState));
    return 1;
}

/* 0x00460140: any button fades out (fadeout.time). */
int Versus_Update(ScreenLoop *screen, void *state)
{
    return Vs_Update(screen, g_game->vs_screen.fadeout_time);
}

/* 0x004601C0: fades out after time ticks. Returns 1. */
int Versus_Tick(ScreenLoop *screen, void *state)
{
    const VsScreenInfo *vs = &g_game->vs_screen;

    return Vs_Tick(screen, vs->time, vs->fadeout_time);
}

/*
 * 0x00460280: the faces (sprite 9000,1 needed, single and simul teams
 * only) and names of both teams, and in arcade modes the match number
 * (match.text used as a printf format). Returns 1.
 */
int Versus_Draw(ScreenLoop *screen, void *state_)
{
    VersusState *state = state_;
    GameVars *g = g_game;
    VsScreenInfo *vs = &g->vs_screen;
    int side;

    if (g->skip_draw) {
        return 1;
    }
    for (side = 0; side < 2; side++) {
        const SelectCell *cell = state->cells[side][0];
        if ((unsigned int)(g_game->teams[side].members[0].count - 1) <= 1 && cell != NULL && cell->sprites != NULL &&
            KeyedArray_Find(cell->sprites->sprites, 9000, 1) != -1) {
            Vs_DrawFace(&vs->sides[side].face, cell);
        }
    }
    for (side = 0; side < 2; side++) {
        const VsSideLayout *l = &vs->sides[side];
        const int *count = &g_game->teams[side].members[0].count;
        int x = l->name_pos[0];
        int y = l->name_pos[1];
        if (*count == 1) {
            if (state->cells[side][0] != NULL) {
                FontSpec_DrawText(&l->name_font, state->cells[side][0]->name, x, y);
            }
        } else if (*count == 2) {
            int i;
            for (i = 0; i < *count; i++) {
                if (state->cells[side][i] != NULL) {
                    FontSpec_DrawText(&l->name_font, state->cells[side][i]->name, x, y);
                }
                x += l->name_spacing[0];
                y += l->name_spacing[1];
            }
        }
    }
    if (g_game->arcade_side != 0) {
        char text[0x100];
        SysDrawList *list;
        int idx;

        _snprintf(text, sizeof(text), vs->match.text, g_game->arcade_match);
        vs->match.text_ptr = (intptr_t)text;
        g = g_game;
        list = g->sysexplods;
        if (vs->match.layerno == 0) {
            idx = SysElem_Add(&vs->match, list, g->motif_sprites, g->motif_anims, g->motif_fonts, 0.0f, 0.0f);
            if (idx >= 0) {
                float s = (float)g_game->motif_scale;
                SysElem_SetScale(idx, s, 1.0f);
            }
            Vs_DrawItem(list, idx);
        }
        vs->match.text_ptr = 0;
    }
    return 1;
}

/* 0x004605E0: the screen loop of the versus screen; 0 when its state cannot be allocated. */
int Versus_Setup(ScreenLoop *screen)
{
    if (!Vs_Setup(screen, 9, sizeof(VersusState))) {
        return 0;
    }
    screen->init = Versus_Init;
    screen->end = Versus_End;
    screen->update = Versus_Update;
    screen->tick = Versus_Tick;
    screen->draw = Versus_Draw;
    screen->overlay = Screen_TickNop;
    return 1;
}

/*
 * 0x00460680: runs the versus screen (precaching both teams meanwhile);
 * returns the next game flow step: 11, 6 when cancelled with escape, 4 when
 * the screen loop failed.
 */
int Versus_Run(void)
{
    ScreenLoop screen;
    int next = 11;
    int done;

    if (g_game->precache_enabled) {
        int side;
        for (side = 0; side < 2; side++) {
            Precache_QueueTeam(&g_game->teams[side].members[0]);
        }
        Precache_Start();
    }
    Log_Printf("Entering versus screen.\n");
    Versus_Setup(&screen);      /* the result is not checked */
    if (!Screen_Run(&screen)) {
        next = 4;
    }
    done = screen.done;
    screen.result = done;
    if (screen.state != NULL) {
        free(screen.state);
    }
    memset(&screen, 0, sizeof(screen));
    if (done) {
        Log_Printf("Canceled versus screen.\n");
        next = 6;
    }
    Precache_Stop();
    return next;
}

/* ---- the victory screen ---- */

/* 0x00460750: the cells of the shown members (GameVars.winner_member), the victory background and music. Returns 1. */
int Victory_Init(ScreenLoop *screen, void *state)
{
    GameVars *g = g_game;

    Vs_FillCells(state, g->winner_member);
    screen->bg = g->victory_bg;
    screen->scale = (float)g->motif_scale;
    screen->bgm = &g->victory_bgm;
    Vs_StartFade(g->victory_screen.fadein_time, 0);
    Log_Printf("Victory screen init\n");
    return 1;
}

/* 0x00460860 */
int Victory_End(ScreenLoop *screen, void *state)
{
    Log_Printf("End of victory screen loop\n");
    memset(state, 0, sizeof(VersusState));
    return 1;
}

/* 0x00460890: any button fades out (fadeout.time). */
int Victory_Update(ScreenLoop *screen, void *state)
{
    return Vs_Update(screen, g_game->victory_screen.fadeout_time);
}

/* 0x00460910: fades out after time ticks. Returns 1. */
int Victory_Tick(ScreenLoop *screen, void *state)
{
    const VictoryInfo *vi = &g_game->victory_screen;

    return Vs_Tick(screen, vi->time, vi->fadeout_time);
}

/*
 * 0x004609E0: the portrait (p1.spr, else 9000,1 with a note on the debug
 * clipboard the first tick), the name of the winner's first character and
 * the win quote (GameVars.victory_quote). Returns 1.
 *
 * Quirk: the name is drawn whenever the screen has a state, so a missing
 * cell gives the text at address 8.
 */
int Victory_Draw(ScreenLoop *screen, void *state_)
{
    VersusState *state = state_;
    GameVars *g = g_game;
    VictoryInfo *vi = &g->victory_screen;
    const SelectCell *cell;
    SysDrawList *list;
    int idx;

    if (g->match_winner == 0 || g->skip_draw) {
        return 1;
    }
    cell = state->cells[g->match_winner - 1][0];
    if (cell != NULL && cell->sprites != NULL) {
        int group = vi->face.group;
        int number = vi->face.number;
        int found = KeyedArray_Find(cell->sprites->sprites, group, number) != -1;
        if (!found) {
            if (screen->ticks == 0) {
                TextRing_Printf(g_game->clipboard, "Info: %s has no victory portrait. Using 9000,1.\n", cell->name);
            }
            if (KeyedArray_Find(cell->sprites->sprites, 9000, 1) != -1) {
                vi->face.group = 9000;
                vi->face.number = 1;
                found = 1;
            }
        }
        if (found) {
            Vs_DrawFace(&vi->face, cell);
        }
        vi->face.group = group;
        vi->face.number = number;
    }
    if (state_ != NULL) {
        FontSpec_DrawText(&vi->name_font, (const char *)((uintptr_t)cell + offsetof(SelectCell, name)),
                          vi->name_offset[0], vi->name_offset[1]);
    }
    g = g_game;
    vi->winquote.text_ptr = g->victory_quote[0] != '\0' ? (intptr_t)g->victory_quote : 0;
    list = g->sysexplods;
    if (vi->winquote.layerno == 0) {
        idx = SysElem_Add(&vi->winquote, list, NULL, NULL, g->motif_fonts, 0.0f, 0.0f);
        if (idx >= 0) {
            float s = (float)g_game->motif_scale;
            SysElem_SetScale(idx, s, 1.0f);
        }
        SysDraw_DrawItem(list, idx);
    }
    vi->winquote.text_ptr = 0;
    return 1;
}

/* 0x00460C10: the screen loop of the victory screen; 0 when its state cannot be allocated. */
int Victory_Setup(ScreenLoop *screen)
{
    if (!Vs_Setup(screen, 5, sizeof(VersusState))) {
        return 0;
    }
    screen->init = Victory_Init;
    screen->end = Victory_End;
    screen->update = Victory_Update;
    screen->tick = Victory_Tick;
    screen->draw = Victory_Draw;
    screen->overlay = Screen_TickNop;
    return 1;
}

/* 0x00460CB0: runs the victory screen when enabled; the next game flow step: 8 in arcade modes, else 6. */
int Victory_Run(void)
{
    ScreenLoop screen;

    Log_Printf("Entering victory screen.\n");
    if (g_game->victory_screen.enabled) {
        Victory_Setup(&screen);
        Screen_Run(&screen);
        screen.result = screen.done;
        if (screen.state != NULL) {
            free(screen.state);
        }
        memset(&screen, 0, sizeof(screen));
    }
    return (g_game->arcade_side != 0) * 2 + 6;
}

/* ---- the win screen ---- */

/*
 * 0x00460D30: the human team's players go to state 180 at the camera
 * position (the partner 50 game units in front, facing back), the others
 * leave the match. Returns 1.
 *
 * Quirk: the partner is placed with the first player's facing even when
 * there is no first player.
 */
int WinScreen_Init(ScreenLoop *screen, void *state_)
{
    WinScreenState *state = state_;
    GameVars *g = g_game;
    Player *o;

    state->p = NULL;
    state->partner = NULL;
    if (g->arcade_side == 2) {
        state->p = g->players[2];
        state->partner = g->players[4];
        if (g->players[1] != NULL && g->players[1]->active) {
            Players_SetActive(g->players[1]->index, 0);
        }
        o = g->players[3];
    } else {
        state->p = g->players[1];
        state->partner = g->players[3];
        if (g->players[2] != NULL && g->players[2]->active) {
            Players_SetActive(g->players[2]->index, 0);
        }
        o = g->players[4];
    }
    if (o != NULL && o->active) {
        Players_SetActive(o->index, 0);
    }
    if (state->p != NULL) {
        Player *p = state->p;
        Players_SetActive(p->index, 1);
        Player_Reset(p);
        Player_ChangeState(180, p);
        g = g_game;
        p->pos.x = g->camera_offset_x;
        if (state->partner != NULL) {
            float w = (float)((double)Ftol(g->game_coord[0]) * 50.0 / 320.0);
            p->pos.x = p->pos.x - (double)w * p->facing;
        }
        p->no_keyctrl = 1;
    }
    if (state->partner != NULL) {
        Player *p = state->partner;
        float w;
        Players_SetActive(p->index, 1);
        Player_Reset(p);
        Player_ChangeState(180, p);
        g = g_game;
        w = (float)((double)Ftol(g->game_coord[0]) * 50.0 / 320.0);
        p->pos.x = (double)w * state->p->facing + g->camera_offset_x;
        if (p->facing == state->p->facing) {
            p->facing = -p->facing;
            Player_UpdateInputFacing(p);
        }
        p->no_keyctrl = 1;
    }
    Log_Printf("Win screen init\n");
    return 1;
}

/* 0x00460FC0 */
int WinScreen_End(ScreenLoop *screen, void *state_)
{
    WinScreenState *state = state_;

    Log_Printf("End of win screen loop\n");
    state->p = NULL;
    state->partner = NULL;
    return 1;
}

/*
 * 0x00460FE0: a button of the human side (either side when both are human)
 * or escape fades out; during the fade out escape or a quit request ends
 * it at once.
 */
int WinScreen_Update(ScreenLoop *screen, void *state)
{
    GameVars *g;
    int pressed = 0;

    if (screen->fade_out) {
        if (screen->fade_ticks > 0) {
            if (screen->key == 27 || g_quitRequested) {
                screen->fade_ticks = 0;
                screen->fade_out = 1;
            }
            return 1;
        }
        return 0;
    }
    g = g_game;
    if ((screen->input_side + 1 == g->arcade_side || g->player_sides[0] == g->player_sides[1]) && (screen->input & 0xFF0)) {
        pressed = 1;
    }
    if (screen->done || pressed) {
        Vs_FadeOut(screen, screen->fadeout_time);
    }
    return 0;
}

/*
 * 0x00461080: fades out after pose.time; halves the colour factors of the
 * palette groups of the first mask word (the players) every tick. Returns 1.
 */
int WinScreen_Tick(ScreenLoop *screen, void *state)
{
    PalGroups *pg;
    static const unsigned int select[5] = { 0xFFFFFFFFu, 0, 0, 0, 0 };
    int i, k;

    if (!screen->fade_out && screen->ticks == g_game->win_screen.pose_time) {
        Vs_FadeOut(screen, screen->fadeout_time);
    }
    pg = g_game->palgroups;
    for (i = 0; i < pg->count; i++) {
        PalGroupEntry *e = &pg->entries[i];
        unsigned int mask[5];
        for (k = 0; k < 5; k++) {
            mask[k] = e->mask[k];
        }
        for (k = 0; k < 5; k++) {
            if (select[k] & mask[k]) {
                PalFx *fx = e->fx;
                fx->mul[0] = (int)((unsigned int)fx->mul[0] << 7) >> 8;
                fx->mul[1] = (int)((unsigned int)fx->mul[1] << 7) >> 8;
                fx->mul[2] = (int)((unsigned int)fx->mul[2] << 7) >> 8;
                fx->serial = g_palFxSerial++;
                break;
            }
        }
    }
    return 1;
}

/* 0x00461190: the motif's wintext. Returns 1. */
int WinScreen_Draw(ScreenLoop *screen, void *state)
{
    if (g_game->skip_draw == 0) {
        SysElem_AddMotif(&g_game->win_screen.wintext, 0.0f, 0.0f);
    }
    return 1;
}

/* 0x004611C0: the screen loop of the win screen; 0 when its state cannot be allocated. */
int WinScreen_Setup(ScreenLoop *screen)
{
    GameVars *g = g_game;

    if (!Vs_Setup(screen, 6, sizeof(WinScreenState))) {
        return 0;
    }
    screen->init = WinScreen_Init;
    screen->end = WinScreen_End;
    screen->update = WinScreen_Update;
    screen->tick = WinScreen_Tick;
    screen->draw = WinScreen_Draw;
    screen->overlay = Screen_TickNop;
    screen->loads_match = 1;
    screen->reload_motif = 1;
    screen->fadein_time = g->win_screen.fadein_time;
    screen->fadeout_time = g->win_screen.fadeout_time;
    return 1;
}

/* 0x00461280: runs the win screen; the next game flow step: 17, or 4 when the screen loop failed. */
int WinScreen_Run(void)
{
    ScreenLoop screen;
    int next = 17;

    Log_Printf("Entering win screen.\n");
    WinScreen_Setup(&screen);   /* the result is not checked */
    if (!Screen_Run(&screen)) {
        next = 4;
    }
    screen.result = screen.done;
    if (screen.state != NULL) {
        free(screen.state);
    }
    return next;
}
