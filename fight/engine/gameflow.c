/*
 * The game flow and the match loop (original module 0x4124A0-0x419A4F, part
 * 1: 0x4124A0-0x412EBF): references to players by id and the conversions
 * between screen and game coordinates.
 *
 * Floating point follows the x87 code of the original: intermediate results
 * stay in double and are rounded to float where the original stores a float;
 * float to int conversions truncate like __ftol2.
 */
#define HAVE_STRUCT_TIMESPEC 1      /* the CRT's time.h already has it */
#include <pthread.h>

#include "mugen.h"

/* ------------------------------------------------------------------ player references */

/* ------------------------------------------------------------------ the game flow */

/*
 * 0x004124A0: the game flow: a state machine over the screens of the game
 * (GameVars state in g_gameflowState, allocated on the first call) until
 * state 20 (exit); the BGM is unloaded at the end.
 *
 *   0 start, 1 logo storyboard, 2 select.def, 3 intro storyboard, 4 title,
 *   5 options, 6 match setup, 7 character select, 8 stage pick, 9 arcade
 *   intro of the character, 10 versus screen, 11 match, 12 victory screen,
 *   13 continue screen, 14 game over, 15 survival results, 16 win screen,
 *   17 default ending, 18 character ending, 19 end credits.
 */
void Gameflow_Run(void)
{
    int *state = g_gameflowState;
    char path[0x200];

    if (state == NULL) {
        state = calloc(1, sizeof(int));
        if (state == NULL) {
            g_outOfMemory = 1;
        }
        g_gameflowState = state;
        if (state != NULL) {
            *state = 0;
        }
    }
    while (*state != 20) {
        GameVars *g;

        Log_Printf("Gameflow %i\n", *state);
        switch (*state) {
        case 0:
            *state = 1;
            break;
        case 1:
            if (g_game->quick_vs == 0) {
                Storyboard_PlayFile(g_game->logo_storyboard);
            }
            *state = 2;
            break;
        case 2:
            Select_Init();
            *state = 3;
            break;
        case 3:
            g = g_game;
            if (g->quick_vs != 0) {
                *state = 6;
                break;
            }
            if (g->intro_storyboard[0] != '\0' && g->intro_played == 0) {
                Storyboard_PlayFile(g->intro_storyboard);
                g_game->intro_played = 1;
            }
            *state = 4;
            break;
        case 4:
            *state = Title_Run();
            break;
        case 5:
            *state = Options_Run();
            break;
        case 6:
            *state = Gameflow_SetupMatch();
            break;
        case 7:
            *state = Charsel_Run();
            break;
        case 8:
            Gameflow_PickStage();
            *state = 9;
            break;
        case 9:
            g = g_game;
            if (g->quick_vs == 0 && (g->demo_mode == 0 || g->demo_vsscreen_enabled != 0) && g->arcade_side != 0 &&
                g->arcade_match == 1) {
                int ci = g->teams[g->arcade_side - 1].members[0].slots[0].char_index;
                if (ci != -1) {
                    CharInfo *info = &g->char_info[ci];
                    Path_Join(info->intro_storyboard, info->dir, path);
                    if (!File_Exists(path)) {
                        path[0] = '\0';
                        strncat(path, info->intro_storyboard, sizeof(path) - 1);
                    }
                    Storyboard_PlayFile(path);
                }
            }
            *state = 10;
            break;
        case 10:
            g = g_game;
            *state = 11;
            if (g->quick_vs == 0 && (g->demo_mode == 0 || g->demo_vsscreen_enabled != 0)) {
                *state = Versus_Run();
            }
            break;
        case 11:
            *state = Match_Run();
            break;
        case 12:
            g = g_game;
            if (g->quick_vs == 0 && (g->demo_mode == 0 || g->demo_vsscreen_enabled != 0)) {
                *state = Victory_Run();
            } else {
                *state = 3;
            }
            break;
        case 13:
            if (g_game->continue_screen.enabled != 0) {
                *state = Continue_Run();
            } else {
                *state = 14;
            }
            break;
        case 14:
            g = g_game;
            if (g->game_over.enabled != 0) {
                Storyboard_PlayFile(g->game_over.storyboard);
                g = g_game;
            }
            g->intro_played = 0;
            *state = 3;
            break;
        case 15:
            Bgm_Stop();
            if (g_game->survival_results.enabled != 0) {
                Result_Run();
            }
            *state = 6;
            break;
        case 16:
            Bgm_Stop();
            if (g_game->win_screen.enabled != 0) {
                WinScreen_Run();
            }
            *state = 17;
            break;
        case 17:
            g = g_game;
            if (g->default_ending.enabled != 0) {
                Storyboard_PlayFile(g->default_ending.storyboard);
            }
            *state = 19;
            break;
        case 18:
            Storyboard_PlayFile(g_game->ending_storyboard);
            *state = 19;
            break;
        case 19:
            g = g_game;
            if (g->end_credits.enabled != 0) {
                Storyboard_PlayFile(g->end_credits.storyboard);
                g = g_game;
            }
            g->intro_played = 0;
            *state = 3;
            break;
        default:
            break;
        }
    }
    if (g_bgmActive) {
        pthread_mutex_lock((pthread_mutex_t *)&g_bgmMutex);
        BgmQueue_Unload(g_bgmGetHandle());
        pthread_mutex_unlock((pthread_mutex_t *)&g_bgmMutex);
        g_bgmBackgroundChanged = 1;
    }
}

/* ------------------------------------------------------------------ player references and coordinates */

/* 0x00412880: refers to p (when it is in the match); returns 0 otherwise. */
int PlayerRef_Set(PlayerRef *ref, Player *p, int flag)
{
    if (p == NULL || !p->active) {
        return 0;
    }
    ref->valid = 1;
    ref->id = p->id;
    ref->index = p->index;
    ref->flag = flag;
    return 1;
}

/*
 * 0x004128B0: the reference still refers to a player in the match (with p:
 * to p, which must be in the match too).
 */
int PlayerRef_Is(const PlayerRef *ref, const Player *p)
{
    int valid = 0;

    if (ref->valid == 1) {
        Player *o = g_game->players[ref->index];
        if (o != NULL && o->active && o->id == ref->id) {
            valid = 1;
        }
    }
    if (p == NULL) {
        return valid;
    }
    if (valid && p->active && ref->id == p->id) {
        return 1;
    }
    return 0;
}

/*
 * 0x00412910: position (plus the draw offset Player.draw_offset with the
 * reference's flag) and facing of the referenced player, each output
 * optional; ref_id points at PlayerRef.id. The reference is not checked.
 */
int PlayerRef_GetPos(const int *ref_id, float *x, float *y, int *facing)
{
    const PlayerRef *ref = (const PlayerRef *)((const char *)ref_id - offsetof(PlayerRef, id));
    Player *p = g_game->players[ref->index];

    if (x != NULL) {
        *x = (float)((ref->flag != 0 ? p->draw_offset.x : 0.0) + p->pos.x);
    }
    if (y != NULL) {
        *y = (float)((ref->flag != 0 ? p->draw_offset.y : 0.0) + p->pos.y);
    }
    if (facing != NULL) {
        *facing = p->facing;
    }
    return 1;
}

/* 0x00412C50: refers to the player with Player.id `id`; 0 when there is none in the match. */
int PlayerRef_SetById(PlayerRef *ref, int id)
{
    int idx = Player_FindIndexById(id);
    Player *p;

    if (idx == -1) {
        return 0;
    }
    p = g_game->players[idx];
    if (p == NULL || !p->active) {
        return 0;
    }
    ref->valid = 1;
    ref->id = p->id;
    ref->index = p->index;
    ref->flag = 1;
    return 1;
}

/* ------------------------------------------------------------------ coordinates */

/*
 * The camera terms shared by the conversions: s, the draw scale of the
 * stage (zoom), k, the game units per stage unit, and the screen position of
 * the camera. The original leaves pad_24 of the results as stack garbage; it
 * is zeroed here.
 */

/* 0x00412970: a screen position (x, y) to game (stage space) coordinates. */
Coord *Coord_ScreenToGame(Coord *out, double x, double y, double z, double unit, int type, int pad)
{
    GameVars *g = g_game;
    float s = g->stage_scale * g->camera_zoom;
    double k = g_gameCoord[0] / g->stage.unit;
    int half = Ftol(g->game_coord[0]) / 2;
    float cx, cy, t;
    Coord c;

    cx = (float)(g->camera_x * k + g->camera_offset_x);
    c.x = (x - half) / s + cx;
    cy = (float)(g->camera_y * k + (double)g->camera_offset_y * g->camera_zoom);
    t = (float)((double)g->draw_offset_y - cy);
    c.y = (y - t) / s;
    c.z = 0.0;
    c.unit = 1.0;
    c.type = COORD_TYPE_2D;
    c.pad_24 = 0;
    *out = c;
    return out;
}

/* 0x00412A40: a game (stage space) position to screen coordinates. */
Coord *Coord_GameToScreen(Coord *out, double x, double y, double z, double unit, int type, int pad)
{
    GameVars *g = g_game;
    float s = g->stage_scale * g->camera_zoom;
    double k = g_gameCoord[0] / g->stage.unit;
    float cx, cy, t;
    Coord c;

    cx = (float)(g->camera_x * k + g->camera_offset_x);
    c.x = (x - cx) * s + (double)(Ftol(g->game_coord[0]) / 2);
    cy = (float)(g->camera_y * k + (double)g->camera_offset_y * g->camera_zoom);
    t = (float)((double)g->draw_offset_y - cy);
    c.y = y * s + t;
    c.z = 0.0;
    c.unit = 1.0;
    c.type = COORD_TYPE_2D;
    c.pad_24 = 0;
    *out = c;
    return out;
}

/*
 * 0x00412B10: a screen position to stage coordinates: as Coord_ScreenToGame,
 * with the whole part of the camera position (camera_x_base, camera_y_base) left out.
 */
Coord *Coord_ScreenToStage(Coord *out, double x, double y, double z, double unit, int type, int pad)
{
    GameVars *g = g_game;
    float s = g->stage_scale * g->camera_zoom;
    double k = g_gameCoord[0] / g->stage.unit;
    float cx, cy, t;
    int half;
    Coord c;

    cx = (float)(((double)g->camera_x - g->camera_x_base) * k + g->camera_offset_x);
    half = Ftol(g->game_coord[0]) / 2;
    c.x = (x - half) / s + cx;
    cy = (float)(((double)g->camera_y - g->camera_y_base) * k + (double)g->camera_offset_y * g->camera_zoom);
    t = (float)((double)g->draw_offset_y - cy);
    c.y = (y - t) / s;
    c.z = 0.0;
    c.unit = 1.0;
    c.type = COORD_TYPE_2D;
    c.pad_24 = 0;
    *out = c;
    return out;
}

/*
 * 0x00412BF0: c divided by the camera zoom (x and y). The original builds the
 * result in an uninitialised temporary of which it only sets x and y (its z,
 * unit, type and pad_24 are stack garbage); they are taken from c here.
 */
Coord *Coord_Unzoom(Coord *out, double x, double y, double z, double unit, int type, int pad)
{
    GameVars *g = g_game;
    Coord c;

    c.x = x;
    c.y = y;
    c.z = z;
    c.unit = unit;
    c.type = type;
    c.pad_24 = pad;
    if (g->camera_zoom != 0.0f) {
        double r = 1.0 / g->camera_zoom;
        c.x = x * r;
        c.y = r * y;
    }
    *out = c;
    return out;
}

/* ------------------------------------------------------------------ the match loop */

/* The state of the match screen loop (ScreenLoop.state, 4 bytes). */
typedef struct MatchState {
    int unused_00;                  /* 0x00 cleared by Match_Init and Match_End, never read */
} MatchState;

/* 0x00412CA0: the match screen starts. Returns 1. */
int Match_Init(ScreenLoop *screen, void *state)
{
    ((MatchState *)state)->unused_00 = 0;
    Log_Printf("Match loop init\n");
    return 1;
}

/* Unsigned int to double as the original converts it (fild, plus 2^32 when negative). */
static double Gf_UintToDouble(int v)
{
    double d = (double)v;

    if (v < 0) {
        d += 4294967296.0;
    }
    return d;
}

/* 0x00412CC0: the match screen ends: timing statistics in mugen.log (unless disabled). Returns 1. */
int Match_End(ScreenLoop *screen, void *state)
{
    Log_Printf("End of match loop\n");
    if (g_game->no_frame_stats == 0) {
        int ticks = screen->end_ticks - screen->start_ticks;
        int frames = screen->end_frame - (int)screen->start_frame;
        int drawn = screen->end_frame - screen->skipped_frames - (int)screen->start_frame;

        Log_Printf("Total time: %.2fs\n", Gf_UintToDouble(ticks) / g_timerTicksPerSecond);
        Log_Printf("Total frames: %u \tSkipped frames: %u\n", frames, screen->skipped_frames);
        Log_Printf("Avg game rate: %.2f\n", (double)frames * g_timerTicksPerSecond / Gf_UintToDouble(ticks));
        Log_Printf("Avg frame rate: %.2f\n\n", Gf_UintToDouble(drawn) * g_timerTicksPerSecond / Gf_UintToDouble(ticks));
    }
    ((MatchState *)state)->unused_00 = 0;
    return 1;
}

/*
 * 0x00412DF0: the training menu keys, and in demo mode any key (but the
 * function keys of 0x200 and 0x12E) or joystick button ends the demo.
 * Returns 1.
 */
int Match_Update(ScreenLoop *screen, void *state)
{
    GameVars *g;
    int key = screen->key;
    unsigned int pad = screen->input2;

    if (Training_HandleKey(screen->key_code)) {
        g_game->defocus_paused = 0;
    }
    g = g_game;
    if (g->demo_mode != 0) {
        if ((key != 0 && !(key & 0x200) && key != 0x12E) || (pad != 0 && !(pad & 0x100))) {
            screen->done = 1;
            g->speedup = 0;
        }
    }
    return 1;
}

/* 0x00412E60: once the screen is done, it fades out. Returns 1. */
int Match_Tick(ScreenLoop *screen, void *state)
{
    if (screen->done != 0 && screen->fade_out == 0) {
        GameVars *g = g_game;
        int t = screen->fadeout_time;
        screen->fade_out = 1;
        screen->fade_ticks = t;
        g->fade.type = FADE_PALETTE;
        g->fade.ticks = 0;
        g->fade.time = t;
        g->fade.out = 1;
    }
    return 1;
}

/* 0x00412EB0: a screen callback that does nothing (ScreenLoop.overlay of the continue screen and the match). Returns 1. */
int Screen_TickNop(ScreenLoop *screen, void *state)
{
    return 1;
}

/*
 * 0x00412EC0: the screen loop of a match: kind 4, at [Options] GameSpeed * 5
 * + [Config] GameSpeed frames per second (at least 10). 0 when its state
 * cannot be allocated.
 */
int Match_Setup(ScreenLoop *screen)
{
    GameVars *g;

    memset(screen, 0, sizeof(*screen));
    screen->scale = 1.0f;
    g = g_game;
    screen->kind = 4;
    screen->fps = 60;
    screen->fadein_time = 10;
    screen->fadeout_time = 10;
    screen->reload_motif = 1;
    screen->start_frame = g->frames;
    screen->state = calloc(1, sizeof(MatchState));
    if (screen->state == NULL) {
        return 0;
    }
    screen->init = Match_Init;
    screen->end = Match_End;
    screen->update = Match_Update;
    screen->tick = Match_Tick;
    screen->draw = Screen_TickNop;
    screen->overlay = Screen_TickNop;
    screen->loads_match = 1;
    screen->reload_motif = 0;
    if (screen->kind == 4) {
        screen->fps = g->option_game_speed * 5 + g->game_speed;
        if (screen->fps < 10) {
            screen->fps = 10;
        }
    }
    return 1;
}

/*
 * 0x00412FA0: runs a match and returns the next game flow step: 12 (victory
 * screen) by default; 13 (continue) after an arcade loss, 16 / 18 (the
 * default / the character's ending) after the last arcade match, 15 after
 * a survival match, 11 to replay the match (GameVars.match_reload), 6 when
 * escaped, 3 in demo mode, 20 (exit) for a quick VS match.
 */
int Match_Run(void)
{
    ScreenLoop screen;
    char path[0x200];
    GameVars *g;
    int done, next;

    Match_Setup(&screen);
    if (!Screen_Run(&screen)) {
        Error_Exit();
    }
    done = screen.done;
    screen.result = done;
    if (screen.state != NULL) {
        free(screen.state);
    }
    memset(&screen, 0, sizeof(screen));
    g = g_game;
    next = 12;
    if (g->match_reload != 0) {
        return 11;
    }
    if (g->match_has_result != 0) {
        if (done == 0 && g->arcade_side != 0) {
            if (g->match_winner != g->arcade_side) {
                next = 13;
            } else {
                g->arcade_order_wins++;
                g->arcade_order_next += Arcade_CountNext();
                if (g->arcade_order_count == g->arcade_order_next) {
                    CharInfo *info =
                        &g->char_info[g->teams[g->arcade_side - 1].members[0].slots[0].char_index];
                    if (info->ending_storyboard[0] != '\0') {
                        Path_Join(info->ending_storyboard, info->dir, path);
                        if (File_Exists(path)) {
                            g = g_game;
                            strcpy(g->ending_storyboard, path);
                        } else {
                            g = g_game;
                            strcpy(g->ending_storyboard, info->ending_storyboard);
                        }
                        next = 18;
                    } else {
                        next = 16;
                        g->ending_storyboard[0] = '\0';
                    }
                }
            }
        }
        if (g->match_has_result != 0 && g->survival_side != 0) {
            next = 15;
        }
    }
    if (done != 0) {
        next = 6;
    }
    if (g->demo_mode != 0) {
        next = 3;
    }
    if (g->quick_vs != 0) {
        next = 20;
    }
    return next;
}

/*
 * 0x00413E60: the demo fight clock (round state 2 only): a fade out starts
 * 120 ticks before [Demo Mode] fight.endtime and the screen ends at it;
 * after intro.waitcycles demo fights the intro storyboard plays again.
 * Returns 1 when the screen ends.
 */
int Demo_TickFight(ScreenLoop *screen)
{
    GameVars *g = g_game;
    int ended = 0;

    if (g->round_state != 2) {
        return 0;
    }
    if (g->demo_mode != 0) {
        if (g->demo_ticks == g->demo_fight_endtime - 120) {
            g->fade.type = FADE_PALETTE;
            g->fade.ticks = 0;
            g->fade.time = 120;
            g->fade.out = 1;
        }
        if (g->demo_ticks >= g->demo_fight_endtime) {
            int n = g->demo_cycles;
            screen->fade_out = 1;
            screen->fade_ticks = 0;
            g->demo_cycles = n + 1;
            ended = 1;
            if (n >= g->demo_intro_waitcycles) {
                g->demo_cycles = 0;
                g->intro_played = 0;
            }
        }
    }
    g->demo_ticks++;
    return ended;
}