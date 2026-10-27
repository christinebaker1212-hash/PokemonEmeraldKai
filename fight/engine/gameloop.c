/*
 * The game loop and the common screen loop (original module 0x40C7D0-0x40EBBF,
 * strings "Reloading match" to "Common loop deinit failed").
 *
 * Every screen of the game (title, select, versus, options, storyboards and
 * the fight itself) runs through Screen_Run with a ScreenLoop describing it:
 * callbacks for init, end, update, tick, draw, an optional background and
 * BGM. A ScreenLoop that is not inner is the fight: the game loop functions
 * (Game_*) run around the callbacks.
 */
#include <SDL.h>
#include <SDL_mixer.h>
#define HAVE_STRUCT_TIMESPEC 1      /* the CRT's time.h already has it */
#include <pthread.h>

#include "mugen.h"

#define SCREEN_LOCK(m)      pthread_mutex_lock((pthread_mutex_t *)(m))
#define SCREEN_UNLOCK(m)    pthread_mutex_unlock((pthread_mutex_t *)(m))

/* The global PalFX back to neutral, with a new serial (inlined by the original). */
static void Gl_ResetPalFx(void)
{
    g_palFx.color = 0x100;
    g_palFx.add[0] = 0;
    g_palFx.add[1] = 0;
    g_palFx.add[2] = 0;
    g_palFx.mul[0] = 0x100;
    g_palFx.mul[1] = 0x100;
    g_palFx.mul[2] = 0x100;
    g_palFx.unused_1c = 0;
    g_palFx.invertall = 0;
    g_palFx.serial = g_palFxSerial++;
}

/* ------------------------------------------------------------------ the fight in the screen loop */

/*
 * 0x0040C7D0: before a fight: a match to reload (GameVars.match_reload) is
 * unloaded with the player cache, then the match is loaded
 * (ScreenLoop.loads_match) or unloaded; the training menu starts with training
 * fights, and the pause overlay is installed. Returns 1.
 */
int Game_LoopInit(ScreenLoop *screen)
{
    GameVars *g = g_game;

    if (g->match_reload != 0) {
        Log_Printf("Reloading match\n");
        Log_Printf("Unloading match assets\n");
        Match_FreeAssets();
        Players_ReleaseSlots();
        Players_Free();
        Stage_Unload(&g_game->stage);
        g_game->match_loaded = 0;
        PersistVars_Reset(0);
        PersistVars_Reset(1);
        PlayerCache_Clear();
        g_game->match_reload = 0;
    }
    if (screen->loads_match != 0) {
        Match_LoadAssets();
        g = g_game;
    } else {
        Log_Printf("Unloading match assets\n");
        Match_FreeAssets();
        Players_ReleaseSlots();
        Players_Free();
        Stage_Unload(&g_game->stage);
        g = g_game;
        g->match_loaded = 0;
    }
    g->training_value = 0;
    g->training_cursor = 0;
    g->training_menu = g->team_variant == 3 && g->screen_kind == 4;
    g_gfxPrePresent = Game_PrePresent;
    Log_Printf("Game loop init\n");
    return 1;
}

/* 0x0040C8E0: after a fight: palettes, assets, sounds and the explods are cleared. Returns 1. */
int Game_LoopDeinit(ScreenLoop *screen)
{
    PalGroups_ResetAll();
    Match_FreeAssets();
    if (g_sndActive == 1) {
        Mix_HaltChannel(-1);
    }
    SysDraw_Clear(g_game->explods);
    SysDraw_Clear(g_game->sysexplods);
    if (screen->loads_match != 0) {
        Match_Unload();
    }
    g_gfxPrePresent = NULL;
    Log_Printf("Game loop deinit\n");
    return 1;
}

/*
 * 0x0040C950: the fight keys: Pause (or any key with [Input]
 * AnyKeyUnpauses outside the training menu) pauses and unpauses, Scroll Lock
 * steps one frame while paused, Escape cancels the pause; then the debug
 * keys. Nothing while GameVars.console_open is set. Returns 1.
 */
int Game_ReadInput(ScreenLoop *screen)
{
    GameVars *g = g_game;
    int key = screen->key;

    if (g->console_open != 0) {
        return 1;
    }
    if (screen->kind != 8 && screen->kind != 7 && g->defocus_paused == 0 && key == SDLK_PAUSE &&
        screen->ticks > 0) {
        g->defocus_paused = 1;
        key = 0;
    }
    if (g->defocus_paused != 0 && key != 0 && !(key & 0x200) && key != SDLK_F12) {
        if (key == SDLK_PAUSE || (g->any_key_unpauses != 0 && g->training_menu == 0) || key == SDLK_SCROLLOCK) {
            g->defocus_paused = 0;
            if (key == SDLK_SCROLLOCK) {
                g->frame_step = 1;
                Game_DebugKeys(screen);
                return 1;
            }
        }
    }
    if (key == 27) {
        g->defocus_paused = 0;
        g_pauseToggled = 0;
    }
    Game_DebugKeys(screen);
    return 1;
}

/*
 * 0x0040CA20: Ctrl + key: V vertical retrace, S speedup, F frame skipping
 * (with GameVars.debug_player or [Debug] AllowDebugKeys), D the debug display
 * of each player in turn ([Debug] AllowDebugMode). Returns 1.
 */
int Game_DebugKeys(ScreenLoop *screen)
{
    GameVars *g = g_game;
    int keys = g->debug_player != 0 || g->allow_debug_keys != 0;
    int key = screen->key;

    if (!g_keyState[SDLK_RCTRL] && !g_keyState[SDLK_LCTRL]) {
        return 1;
    }
    if (keys) {
        if (key == 'v') {
            int v = g->vretrace_on == 0;
            g->vretrace_on = v;
            if (g_vretrace != v) {
                if (g_renderMode == 2 || g_renderMode == 1) {
                    SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, v != 0);
                    g = g_game;
                }
                g_vretrace = v;
            }
        } else if (key == 's') {
            if (g->speedup == 0) {
                g->speedup = 1;
            } else {
                g->speedup = 0;
                Timer_ResetFrame();
                g = g_game;
            }
        } else if (key == 'f') {
            g->frame_skip++;
            if (g->frame_skip == 3) {
                g->frame_skip = 2000;
                g->sound_enabled = 0;
            }
            if (g->frame_skip > 2000) {
                g->frame_skip = -1;
                g->sound_enabled = 1;
            }
        }
        if (g->frame_skip >= 2000 && g->speedup == 0) {
            g->frame_skip = -1;
            g->sound_enabled = 1;
        }
    }
    if (!g_keyState[SDLK_RCTRL] && !g_keyState[SDLK_LCTRL]) {
        return 1;
    }
    if (key == 'd' && g->allow_debug_mode != 0) {
        if (g->match_loaded != 0) {
            Player *p;
            do {
                g->debug_player++;
                g->debug = 1;
                p = g->players[g->debug_player];
            } while ((p == NULL || !p->active) && g->debug_player < g->players_last);
            if (g->debug_player > g->players_last) {
                g->debug = 0;
                g->debug_player = 0;
            }
            return 1;
        }
        g->debug_player = 0;
        g->debug = g->debug == 0;
    }
    return 1;
}

/* Ends a PalFX definition whose time is over (inlined three times by Game_Tick). */
static void Game_TickPalFxDef(PalFxDef *def, int run)
{
    GameVars *g = g_game;
    int dur = def->active;

    if (dur == 0) {
        return;
    }
    if (g->defocus_paused == 0 && g->slow_counter == 0 && run) {
        def->time++;
    }
    if (def->time >= dur && dur != -1) {
        memset(def, 0, sizeof(PalFxDef));
        def->mul[2] = 0x100;
        def->mul[1] = 0x100;
        def->mul[0] = 0x100;
        def->color = 0x100;
        def->reset = 1;
    }
}

/*
 * 0x0040CBF0: the tick of a fight: the console, a pending match reload, the
 * Pause / SuperPause timers, the end of the PalFX definitions (BGPalFX, the
 * players', AllPalFX), the round (Round_Tick: 0 ends the screen), the
 * darkening of a Pause / SuperPause and the application of the palette
 * groups. Returns 0 when the round asks to end the screen, else 1.
 */
int Game_Tick(ScreenLoop *screen)
{
    GameVars *g;
    Console *c;
    int i, k;

    Console_Tick();
    c = g_console;
    if (c->open != 0) {
        c->ticks[1]++;
        c->ticks[0]++;
        c->ticks[2]++;
    }
    g = g_game;
    if (g->match_reload != 0) {
        screen->fade_out = 1;
        screen->fade_ticks = 0;
    }
    if (g->slow_step > 0 && g->defocus_paused == 0) {
        int v = g->slow_counter;
        g->slow_counter = v <= 0 ? v + g->slow_step : v - 100;
    }
    if (g->defocus_paused == 0) {
        PalGroups_ResetAll();
    }
    if (g->defocus_paused == 0) {
        g->pause_kind = -1;
        for (k = 1; k >= 0; k--) {
            if (g->pauses[k][6] > 0) {
                g->pause_kind = k;
                break;
            }
        }
        if (g->pause_kind != -1) {
            int *t = &g->pauses[g->pause_kind][6];
            if (*t > 0 && g->slow_counter == 0) {
                (*t)--;
            }
        }
    }
    Game_TickPalFxDef(&g->bg_palfx_def, 1);
    for (i = g->players_last; i >= g->players_first; i--) {
        Player *p = g->players[i];
        int run;
        if (p == NULL || !p->active) {
            continue;
        }
        for (k = 0; k < 5; k++) {
            if (p->pal_groups[k] != 0) {
                break;
            }
        }
        if (k == 5) {
            continue;
        }
        run = g->pause_kind == -1 || p->pause_movetime[g->pause_kind] != 0;
        Game_TickPalFxDef(&p->palfx_def, run);
    }
    Game_TickPalFxDef(&g->all_palfx_def, 1);
    if (Round_Tick(screen)) {
        return 0;
    }
    g = g_game;
    if (g->defocus_paused == 0 && g->pause_kind != -1) {
        int *pause = g->pauses[g->pause_kind];
        if (pause[6] != 0 && pause[5] != 0) {
            PalGroups *pg = g->palgroups;
            unsigned int serial = g_palFxSerial;
            screen->palgroup_masks[0] = 0xB00;
            screen->palgroup_masks[1] = 0;
            screen->palgroup_masks[2] = 0;
            screen->palgroup_masks[3] = 0;
            screen->palgroup_masks[4] = 0;
            for (i = 0; i < pg->count; i++) {
                PalGroupEntry *e = &pg->entries[i];
                if (e->mask[0] & 0xB00) {
                    PalFx *fx = e->fx;
                    fx->mul[0] = (int)((unsigned int)(fx->mul[0] * 5) << 4) >> 8;
                    fx->mul[1] = (int)((unsigned int)(fx->mul[1] * 5) << 4) >> 8;
                    fx->mul[2] = (int)((unsigned int)fx->mul[2] << 7) >> 8;
                    fx->serial = serial++;
                }
            }
            g = g_game;
            g_palFxSerial = serial;
            pg = g->palgroups;
            for (i = 0; i < pg->count; i++) {
                PalGroupEntry *e = &pg->entries[i];
                for (k = 0; k < 5; k++) {
                    if ((unsigned int)pause[k] & e->mask[k]) {
                        PalFx *fx = e->fx;
                        fx->mul[0] = (int)((unsigned int)fx->mul[0] << 7) >> 8;
                        fx->mul[1] = (int)((unsigned int)fx->mul[1] << 7) >> 8;
                        fx->mul[2] = (int)((unsigned int)(fx->mul[2] * 3) << 6) >> 8;
                        fx->serial = serial++;
                        break;
                    }
                }
            }
            g = g_game;
            g_palFxSerial = serial;
        }
    }
    PalGroups_Apply(&g->bg_palfx_def, 0x100, 0, 0, 0, 0, 0);
    for (i = g->players_last; i >= g->players_first; i--) {
        Player *p = g->players[i];
        if (p == NULL || !p->active) {
            continue;
        }
        for (k = 0; k < 5; k++) {
            if (p->pal_groups[k] != 0) {
                break;
            }
        }
        if (k == 5) {
            continue;
        }
        PalGroups_Apply(&p->palfx_def, p->pal_groups[0], p->pal_groups[1], p->pal_groups[2], p->pal_groups[3],
                        p->pal_groups[4], 0);
    }
    PalGroups_Apply(&g->all_palfx_def, screen->palgroup_masks[0], screen->palgroup_masks[1], screen->palgroup_masks[2],
                    screen->palgroup_masks[3], screen->palgroup_masks[4], 1);
    if (g->defocus_paused == 0 && g->slow_counter == 0 && g->envcolor_time > 0) {
        g->envcolor_time--;
    }
    return 1;
}

/* The persistent animations of layer mask `mask`, placed by the camera (in game units). */
static void Gl_DrawAnims(unsigned int mask)
{
    GameVars *g = g_game;
    double scale = g_gameCoord[0] / g->stage.unit;
    float fy = (float)((double)g->camera_zoom * g->camera_offset_y + g->camera_y * scale);
    float fx;
    int x, y;

    fy = (float)floor(fy + 0.5);
    y = -Ftol(fy);
    fx = (float)(g->camera_x * scale + g->camera_offset_x);
    fx = (float)floor(fx + 0.5);
    x = -Ftol(fx);
    AnimSet_Draw(g->anims, x, y, mask, 1.0f);
}

/* Updates (draws) the items of an explod list of layer `layer` (SysDrawItem.layer; -1: all) not drawn yet. */
static void Gl_DrawExplods(SysDrawList *list, int layer)
{
    int i, n = 0;

    for (i = 0; i < list->capacity; i++) {
        SysDrawItem *e = &list->items[i];
        if (n > list->count) {
            break;
        }
        if (e->used) {
            n++;
            if ((layer == -1 || e->layer == layer) && e->drawn == 0) {
                SysDraw_Update(e);
            }
        }
    }
}

/*
 * 0x0040D1C0: draws a frame of the fight: the stage's back layer (or a
 * clear with nobg), the HUD layer 0 (fight screens only) and the bar ticks,
 * the shadows (players, projectiles, explods; not with noshadow), the
 * persistent animations of mask 2, EnvColor (under), the players (first
 * to last), projectiles and explods (last to first), afterimages, the
 * layer-1 sprite queue, the explods of layer 1, the HUD layer 1, the stage
 * front layer (not with nofg), the system elements, EnvColor (over), the
 * animations of mask 1, the ontop explods; then on fight screens the HUD
 * layer 2, the debug collision boxes, the combo counters (drawn and
 * ticked). Nothing but the HUD and the counters with skip_draw. The HUD
 * is drawn when GameVars.hud_enabled is set without nobardisplay, and not
 * in attract mode without [Demo Mode] fight.bars.display. Returns 1.
 */
int Game_Draw(ScreenLoop *screen)
{
    GameVars *g = g_game;
    int hud = 0;
    int i;

    if (g->skip_draw == 0 && g->envcolor_time == 0) {
        if (g->assert_nobg == 0) {
            Match_DrawStageBack();
        } else {
            g_gfxClear(0);
        }
        g = g_game;
    }
    if (g->match_loaded != 0) {
        if (screen->kind == 4) {
            hud = g->hud_enabled != 0 && g->assert_nobardisplay == 0;
            if (g->demo_mode != 0 && g->demo_fight_bars_display == 0) {
                hud = 0;
            } else if (hud) {
                Fight_DrawHud(0);
            }
        }
        Lifebars_Tick();
        g = g_game;
    }
    if (g->skip_draw == 0) {
        if (g->envcolor_time == 0) {
            if (g->assert_globalnoshadow == 0) {
                for (i = g->players_last; i >= g->players_first; i--) {
                    Player *p = g->players[i];
                    if (p != NULL && p->active && p->noshadow == 0 && p->visible != 0) {
                        Player_DrawShadow(p, g->stage.shadow, &g->stage.reflection_intensity);
                        g = g_game;
                    }
                }
                for (i = g->players_last; i >= g->players_first; i--) {
                    Player *p = g->players[i];
                    if (p != NULL && p->active) {
                        Projectiles_DrawShadow((ProjectileList *)p->projectiles);
                        g = g_game;
                    }
                }
                Explods_DrawShadows(g->explods);
                g = g_game;
            }
            Gl_DrawAnims(2);
            g = g_game;
        }
        if (g->skip_draw == 0) {
            SpriteQueue *queue;
            KeyedArray *items;
            if (g->envcolor_time > 0 && g->envcolor_under != 0) {
                g_gfxClear(g->envcolor_color);
                g = g_game;
            }
            for (i = g->players_first; i <= g->players_last; i++) {
                Player *p = g->players[i];
                if (p != NULL && p->active && p->visible != 0) {
                    Player_Draw(p);
                    g = g_game;
                }
            }
            for (i = g->players_last; i >= g->players_first; i--) {
                Player *p = g->players[i];
                if (p != NULL && p->active) {
                    Projectiles_Draw((ProjectileList *)p->projectiles);
                    g = g_game;
                }
            }
            Gl_DrawExplods(g->explods, 0);
            g = g_game;
            for (i = g->players_last; i >= g->players_first; i--) {
                Player *p = g->players[i];
                if (p != NULL && p->active) {
                    if (p->afterimage != NULL) {
                        AfterImage_Draw(p->afterimage, p->sprpriority);
                    }
                    Projectiles_DrawAfterImages((ProjectileList *)g->players[i]->projectiles);
                }
            }
            queue = g->sprite_layer1;
            items = queue->items;
            if (items->count > 0) {
                PalFx *fx = g_palFxCur;
                PalRemap *remap = g_palRemapCur;
                for (i = 0; i < items->count; i++) {
                    SpriteQueueItem *it = (SpriteQueueItem *)(queue->order[i] * items->elem_size + items->data);
                    if (it == NULL) {
                        continue;
                    }
                    g_palFxCur = it->palfx;
                    g_palRemapCur = it->remap;
                    if (it->sprites != NULL) {
                        if (g_renderMode == 2) {
                            Sprite_DrawGL(it->sprites, it->sprite, &it->dp);
                        } else {
                            Sprite_DrawSoft(it->sprites, it->sprite, &it->dp);
                        }
                    }
                    g_palFxCur = fx;
                    g_palRemapCur = remap;
                }
                g = g_game;
            }
            Gl_DrawExplods(g->explods, 1);
            g = g_game;
        }
    }
    if (g->match_loaded != 0 && hud) {
        Fight_DrawHud(1);
        g = g_game;
    }
    if (g->skip_draw == 0) {
        if (g->assert_nofg == 0 && g->stage_bg != NULL) {
            g_palFxCur = &g->stage_palfx;
            Bg_Draw(g->stage_bg, 1);
            g = g_game;
            g_palFxCur = NULL;
        }
        if (g->skip_draw == 0) {
            Gl_DrawExplods(g->sysexplods, -1);
            g = g_game;
            if (g->skip_draw == 0) {
                if (g->envcolor_time > 0 && g->envcolor_under == 0) {
                    g_gfxClear(g->envcolor_color);
                }
                Gl_DrawAnims(1);
                g = g_game;
                Gl_DrawExplods(g->explods, 2);
                g = g_game;
            }
        }
    }
    if (screen->kind != 4) {
        return 1;
    }
    if (hud) {
        Fight_DrawHud(2);
        g = g_game;
    }
    if (g->skip_draw == 0) {
        if (g->clsn_draw_mode != 0) {
            for (i = g->players_last; i >= g->players_first; i--) {
                Player *p = g->players[i];
                if (p != NULL && p->active) {
                    Player_DrawDebug(p);
                    g = g_game;
                }
            }
            for (i = g->players_last; i >= g->players_first; i--) {
                Player *p = g->players[i];
                if (p != NULL && p->active) {
                    Projectiles_DrawClsn((ProjectileList *)p->projectiles);
                    g = g_game;
                }
            }
        }
        if (g->hud_enabled != 0) {
            /* team 1 is inlined by the original, without Combo_Draw's team check */
            Combo_Draw(1);
            Combo_Draw(2);
            g = g_game;
        }
    }
    if (g->defocus_paused == 0 && g->slow_counter == 0) {
        /* team 1 is inlined as well */
        Combo_Tick(1);
        Combo_Tick(2);
    }
    return 1;
}

/* ------------------------------------------------------------------ the common screen loop */

/*
 * 0x0040E110: prepares a screen: the motif's coordinate space, empty input
 * queues, the key maps of both players for the menus, an empty system
 * element list, the screen as the render target (cleared), the vertical crop,
 * no fade; then reloads the motif (ScreenLoop.reload_motif) or unloads it when
 * [Misc] UnloadSystem is set. Returns 1.
 */
int Screen_Init(ScreenLoop *screen)
{
    GameVars *g = g_game;
    float f;

    Gfx_SetCoordSpace(Ftol(g->motif_coord[0]), Ftol(g->motif_coord[1]), g->system_fit);
    Events_Poll();
    g_keyboardQueueWrite = 0;
    g_keyboardQueueRead = 0;
    g_padQueueWrite = 0;
    g_padQueueRead = 0;
    Input_ResetState();
    g = g_game;
    g_keyQueue.write = 0;
    g_keyQueue.read = 0;
    g_screenKeys[0] = g->input_keys[0];
    g_screenKeys[1] = g->input_keys[1];
    screen->key_table = (intptr_t)g_screenKeys;
    screen->key_count = 2;
    SysDraw_Clear(g->sysexplods);
    Gfx_SetRenderTarget(g_game->screen, 0);
    Gfx_SetClipRect(0, 0, g_targetWidth, g_targetHeight);
    g_gfxClear(0);
    f = (float)floor(0.0 + 0.5);
    g = g_game;
    g->camera_y_base = Ftol(f);
    Camera_UpdateCrop();
    g->fade.bar_size = 0;
    g->fade.progress = 0.0f;
    g->fade.type = FADE_NONE;
    g->fade.unused_04 = 0x100;
    g->fade.ticks = 0;
    g->fade.time = 0;
    g->fade.out = 0;
    screen->palgroup_masks[0] = -1;
    screen->palgroup_masks[1] = 0;
    screen->palgroup_masks[2] = 0;
    screen->palgroup_masks[3] = 0;
    screen->palgroup_masks[4] = 0;
    if (screen->reload_motif != 0) {
        System_ReloadMotif();
        return 1;
    }
    if (g->unload_system != 0) {
        System_UnloadMotif();
    }
    return 1;
}

/*
 * 0x0040E280: starts a screen: its background from the start, a fade in of
 * ScreenLoop.fadein_time ticks unless a fade is running, the frame rate
 * (ScreenLoop.fps) and the frame timer. Returns 1.
 */
int Screen_Start(ScreenLoop *screen)
{
    GameVars *g;
    int fps;

    if (screen->bg != NULL) {
        Bg_Reset(screen->bg);
    }
    g = g_game;
    if (g->fade.type == FADE_NONE) {
        g->fade.type = FADE_PALETTE;
        g->fade.ticks = 0;
        g->fade.time = screen->fadein_time;
        g->fade.out = 0;
    }
    fps = screen->fps;
    g_timerFrameRate = fps;
    g_frameRate = fps;
    g_timerFrameTicks = (g_timerFrequency * 2 + 1) / fps / 2;
    Timer_ResetFrame();
    g = g_game;
    g->frame_skip_count = 0;
    g->skip_draw = 0;
    Timer_ResetFrame();
    return 1;
}

/*
 * 0x0040E330: ends a screen: after a fight (kind 4) the BGM goes on (demo
 * fights that keep it) or is unloaded; frame skipping and the global PalFX
 * are reset. Returns 1.
 */
int Screen_Finish(ScreenLoop *screen)
{
    GameVars *g = g_game;

    if (screen->kind == 4) {
        if (g->demo_mode != 0 && g->demo_fight_playbgm == 0 && g->demo_fight_stopbgm == 0) {
            Bgm_Unpause();
            g = g_game;
        } else if (g_bgmActive) {
            SCREEN_LOCK(&g_bgmMutex);
            BgmQueue_Unload(g_bgmGetHandle());
            SCREEN_UNLOCK(&g_bgmMutex);
            g_bgmBackgroundChanged = 1;
            g = g_game;
        }
    }
    g->frame_skip_count = 0;
    g->skip_draw = 0;
    Timer_ResetFrame();
    Gl_ResetPalFx();
    return 1;
}

/*
 * 0x0040E420: the input of a frame: the key pressed (ScreenLoop.key), the
 * joystick button (input2), the input code queued last (key_code, also copied
 * to GameVars.key_codes) and its menu meaning for both players (input,
 * input_side). Escape ends the screen, F12 takes a screenshot. A pending
 * code with an empty queue hangs the game (the original loops forever).
 * Returns 1.
 */
int Screen_ReadInput(ScreenLoop *screen)
{
    GameVars *g;
    int key;

    screen->key_code = 0;
    screen->key = 0;
    screen->input2 = 0;
    screen->input = 0;
    screen->input_side = 0;
    Events_Poll();
    if (g_keyboardQueueRead != g_keyboardQueueWrite) {
        if (!g_keyboardActive) {
            key = 0;
        } else {
            key = g_keyboardQueue[g_keyboardQueueRead];
            g_keyboardQueueRead++;
            if (g_keyboardQueueRead >= INPUT_RING_SIZE) {
                g_keyboardQueueRead = 0;
            }
        }
        screen->key = key;
    }
    if (g_padsActive && g_padQueueRead != g_padQueueWrite) {
        screen->input2 = (unsigned int)Pads_PopCode();
    }
    g = g_game;
    if (g->safe_mode == 0) {
        Video_HandleFullscreenKey(&screen->key);
        g = g_game;
    }
    g->key_codes.write = 0;
    g->key_codes.read = 0;
    while (Input_CodePending()) {
        int code;
        if (!g_padsActive && !g_keyboardActive) {
            code = 0;
        } else {
            if (g_keyQueue.read == g_keyQueue.write) {
                for (;;) {
                }
            }
            code = g_keyQueue.data[g_keyQueue.read];
            g_keyQueue.read++;
            if (g_keyQueue.read >= g_keyQueue.capacity) {
                g_keyQueue.read = 0;
            }
        }
        screen->key_code = code;
        if (code != 0) {
            IntArray *q = &g->key_codes;
            q->data[q->write] = code;
            q->write++;
            if (q->write >= q->capacity) {
                q->write = 0;
            }
            if (q->write == q->read) {
                q->read++;
                if (q->read >= q->capacity) {
                    q->read = 0;
                }
            }
        }
    }
    if (screen->key_count > 0) {
        long long r = Input_FindKey((int *const *)(intptr_t)screen->key_table, screen->key_count, screen->key_code);
        screen->input = (unsigned int)r;
        screen->input_side = (int)(r >> 32);
    }
    key = screen->key;
    if (key != 0 || screen->key_code != 0) {
        screen->idle_ticks = 0;
    }
    if (key == 27) {
        screen->done = 1;
    }
    if (key == SDLK_F12) {
        g->screenshot_requested = 1;
        g->skip_draw = 0;
    }
    return 1;
}

/* The current BGM stream is paused (BgmStream.flags), under the BGM lock. */
static int Screen_BgmPaused(void)
{
    unsigned int flags;

    if (!g_bgmActive) {
        return 0;
    }
    SCREEN_LOCK(&g_bgmMutex);
    flags = g_bgmStream->flags;
    SCREEN_UNLOCK(&g_bgmMutex);
    return flags != 0;
}

/*
 * 0x0040E5D0: the tick of a screen: the BGM is paused or resumed (fights
 * play it unless AssertSpecial nomusic is set, menus play it; screen kinds
 * 6-8 never do), the background and the storyboard advance (not while paused
 * by the focus), the end of the storyboard or a quit request starts the fade
 * out, which then counts down, and the screen fade advances. Returns 1.
 */
int Screen_Tick(ScreenLoop *screen)
{
    GameVars *g = g_game;
    int paused = 0;
    int play = 0;

    if (g->defocus_paused != 0 && screen->ticks > 0) {
        paused = 1;
    }
    if (screen->inner != 0 || g->assert_nomusic == 0) {
        play = 1;
    }
    if (screen->inner == 0 && (g->screen_kind == 6 || g->screen_kind == 7 || g->screen_kind == 8)) {
        play = 0;
    }
    if (!Screen_BgmPaused() && !play) {
        Bgm_Pause();
    }
    if (Screen_BgmPaused() && play) {
        Bgm_Unpause();
    }
    if (screen->bg != NULL) {
        Bg_SetCamera(screen->bg, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
        if (!paused) {
            Bg_Tick(screen->bg);
        }
    }
    if (screen->cutscene != 0 && !paused) {
        Cutscene *cs = (Cutscene *)(intptr_t)screen->cutscene;
        if (cs->type != CUTSCENE_STORYBOARD || Storyboard_Tick(cs->storyboard) != 0) {
            screen->fade_out = 1;
            screen->fade_ticks = 0;
        }
    }
    if (g_quitRequested && screen->fade_out == 0) {
        Log_Printf("CloseWindow event forced common loop quit.\n");
        screen->fade_out = 1;
        screen->fade_ticks = 0;
        screen->done = 1;
    }
    if (screen->fade_out != 0 && screen->fade_ticks > 0 && !paused) {
        screen->fade_ticks--;
    }
    if (screen->cutscene != 0 && ((Cutscene *)(intptr_t)screen->cutscene)->type == CUTSCENE_STORYBOARD) {
        Storyboard_ApplyFade(((Cutscene *)(intptr_t)screen->cutscene)->storyboard);
    }
    g = g_game;
    Fade_Update(&g->fade, g->defocus_paused == 0);
    return 1;
}

/* Empties a sprite queue (its KeyedArray and lookup cache), as inlined by Screen_Run. */
static void Gl_ClearQueue(SpriteQueue *q)
{
    KeyedArray *a = q->items;
    int i;

    memset(a->slots, 0, a->capacity << 4);
    a->count = 0;
    a->last = 0;
    a->first = 0;
    a->iter = 0;
    for (i = 0; i < a->cache_size; i++) {
        a->cache[i].key1 = 0x7FFFFFFF;
        a->cache[i].key2 = (int)0x80000000;
        a->cache[i].index = -1;
    }
    a->cache_pos = 0;
}

/*
 * 0x0040E7C0: runs a screen until it fades out: one frame per iteration
 * (input, update, tick, drawing between the background layers, the frame
 * end). Speed tests and stress tests (kind 4) skip the drawing; a pending
 * round reset (GameVars.round_reset_pending) is done first. Returns 1.
 */
int Screen_Run(ScreenLoop *screen)
{
    GameVars *g;
    int i;

    g_game->screen_kind = screen->kind;
    Screen_Init(screen);
    if (screen->inner == 0) {
        Game_LoopInit(screen);
    }
    if (screen->init != NULL && !screen->init(screen, screen->state)) {
        goto end;
    }
    Screen_Start(screen);
    screen->start_ticks = (int)Timer_GetTicks();
    if (screen->bgm != NULL && !Bgm_Play(screen->bgm)) {
        Log_Printf("Could not load BGM %s\n", (const char *)screen->bgm);
    }
    g = g_game;
    for (;;) {
        if (g->speedtest != 0) {
            g->frame_skip = 2000;
            g->speedup = 1;
            if ((int)g->frames >= 30000) {
                break;
            }
        } else if (g->stresstest != 0 && screen->kind == 4 && g->stresstest != -1) {
            g->frame_skip = g->stresstest;
            g->speedup = 1;
        }
        if (g->round_reset_pending != 0) {
            SCREEN_LOCK(&g->precache_mutex);
            if (g->precache_running) {
                g->precache_stop = 1;
            }
            SCREEN_UNLOCK(&g->precache_mutex);
            Precache_Stop();
            Round_Reset();
            g = g_game;
            g->round_reset_pending = 0;
        }
        Gl_ClearQueue(g->sprite_layer1);
        Screen_ReadInput(screen);
        if (screen->inner == 0) {
            Game_ReadInput(screen);
        }
        if (screen->update != NULL && g_game->console_open == 0) {
            screen->update(screen, screen->state);
        }
        Screen_Tick(screen);
        if (screen->inner == 0) {
            Game_Tick(screen);
        }
        if (screen->tick != NULL && !screen->tick(screen, screen->state)) {
            g = g_game;
        } else {
            g = g_game;
            if (g->defocus_paused == 0) {
                SysDrawList *list = g->sysexplods;
                for (i = 0; i < list->capacity; i++) {
                    SysDraw_Start(list, i);
                }
            }
            g = g_game;
            if (g->skip_draw == 0) {
                if (screen->bg != NULL) {
                    Bg_SetScale(screen->bg, screen->scale, screen->scale);
                    Bg_Draw(screen->bg, 0);
                }
                if (screen->cutscene != 0 && ((Cutscene *)(intptr_t)screen->cutscene)->type == CUTSCENE_STORYBOARD) {
                    Storyboard_Draw(((Cutscene *)(intptr_t)screen->cutscene)->storyboard);
                }
            }
            if (screen->inner == 0) {
                Game_Draw(screen);
            }
            if (screen->draw != NULL) {
                screen->draw(screen, screen->state);
            }
            g = g_game;
            if (g->skip_draw == 0 && screen->bg != NULL) {
                Bg_SetScale(screen->bg, screen->scale, screen->scale);
                Bg_Draw(screen->bg, 1);
            }
            g = g_game;
            if (g->skip_draw == 0 && g->fade.type == FADE_BARS) {
                Fade_DrawBars(&g->fade);
            }
            if (screen->overlay != NULL) {
                screen->overlay(screen, screen->state);
            }
            if (g_game->skip_draw != 0) {
                screen->skipped_frames++;
            }
            Screen_EndFrame();
            g = g_game;
            g->no_sounds = 0;
            if (g->defocus_paused == 0) {
                g->frames++;
                screen->ticks++;
                screen->idle_ticks++;
            }
            if (g->frame_step != 0) {
                g->frame_step = 0;
                g->defocus_paused = 1;
            }
        }
        if (screen->fade_out != 0 && screen->fade_ticks <= 0) {
            break;
        }
    }
    if (g->precache_enabled != 0) {
        int running;
        SCREEN_LOCK(&g->precache_mutex);
        running = g->precache_running;
        SCREEN_UNLOCK(&g->precache_mutex);
        if (running && screen->done) {
            Precache_RequestStop();
            Precache_Stop();
        }
    }
end:
    screen->end_ticks = (int)Timer_GetTicks();
    screen->end_frame = (int)g_game->frames;
    if (screen->end != NULL && !screen->end(screen, screen->state)) {
        Log_Printf("Common loop deinit failed\n");
    }
    if (screen->inner == 0) {
        Game_LoopDeinit(screen);
    }
    Screen_Finish(screen);
    return 1;
}

/* ------------------------------------------------------------------ pause and fps overlays */

/* 0x0040E080: the frame rate in the top right corner ([Debug] ShowFPS... GameVars.show_fps). */
void Game_DrawFps(void)
{
    char text[0x100];
    float fps = Timer_GetFps();

    if (!(fps > 0.0f)) {
        fps = 0.0f;
    }
    _snprintf(text, sizeof(text), "%.1f", (double)fps);
    Font_DrawText(text, g_game->font_small, g_targetWidth - 2, 7, -1, 0, 0x100, 0x100, 0x100);
}

/*
 * 0x0040DEC0: drawn over the screen buffer before it is shown during a fight
 * (g_gfxPrePresent): the "PAUSED" box or the training menu while paused by
 * the focus, the debug display (or the frame rate) and the console.
 */
void Game_PrePresent(SpriteSet *set, int sprite)
{
    GameVars *g;

    Gfx_SetRenderTarget(set, sprite);
    Gl_ResetPalFx();
    g = g_game;
    if (g->defocus_paused != 0 && g->screenshot_requested == 0) {
        if (g->training_menu == 0) {
            int cx = g_targetWidth / 2;
            ((GfxRectFn)g_gfxDrawRect)(cx - 27, 8, cx + 27, 27, Video_MapRGB(0x50, 0x50, 0x80));
            ((GfxRectFn)g_gfxDrawRect)(cx - 26, 9, cx + 26, 26, Video_MapRGB(0xC0, 0xC0, 0xF0));
            g_gfxBlendRect(cx - 25, 10, cx + 25, 25, Video_MapRGB(0, 0, 0x18), 0x100, 0x80);
            Font_DrawText("PAUSED", g_game->font_medium, cx, 20, 0, 0, 0x100, 0x100, 0x100);
        } else {
            Training_Draw();
        }
        g = g_game;
    }
    if (g->debug != 0 && (g->demo_mode == 0 || g->demo_debuginfo != 0)) {
        Debug_Draw(g->match_loaded != 0 ? g->debug_player : 0);
        Console_Draw();
        return;
    }
    if (g->show_fps != 0) {
        Game_DrawFps();
    }
    Console_Draw();
}
