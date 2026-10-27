/*
 * Coordinates, the end of a frame, safe mode and screenshots (original
 * module 0x40F540-0x40FB4F, strings "%i,%i,%i" to "mugen%03i.png").
 *
 * Floating point follows the x87 code of the original: intermediate results
 * stay in double; float to int conversions truncate like __ftol2.
 */
#include "mugen.h"

/* ------------------------------------------------------------------ coordinates */

/*
 * 0x0040F540: (x, y) in the units of character c (its localcoord) as a value
 * in game units. The original leaves pad_24 as stack garbage; it is zeroed here.
 */
Coord *Coord_MakeScaled(Coord *out, const CharData *c, double x, double y)
{
    double k = g_gameCoord[0] / c->localcoord[0];
    Coord r;

    r.x = x * k;
    r.y = y * k;
    r.z = k * 0.0;
    r.unit = 1.0;
    r.type = COORD_TYPE_2D;
    r.pad_24 = 0;
    *out = r;
    return out;
}

/* The localcoord scale of a player: of the custom state owner whose states run, or its own. */
static double Player_LocalScale(const Player *p)
{
    if (p->custom_states != NULL && p->custom_owner != -1) {
        Player *o = g_game->players[p->custom_owner];
        if (o != NULL) {
            return g_gameCoord[0] / o->data->localcoord[0];
        }
    }
    return g_gameCoord[0] / p->data->localcoord[0];
}

/* 0x0040F5C0: v in the player's units (localcoord) to game units. */
double Player_FromLocalCoord(Player *p, double v)
{
    return Player_LocalScale(p) * v;
}

/* 0x0040F610: v in game units to the player's units. */
double Player_ToLocalCoord(Player *p, double v)
{
    return v / Player_LocalScale(p);
}

/* 0x0040F660: a 2D value with unit 1. The original leaves pad_24 as stack garbage; it is zeroed here. */
Coord *Coord_MakeUnit(Coord *out, double x, double y)
{
    Coord c;

    c.x = x;
    c.y = y;
    c.z = 0.0;
    c.unit = 1.0;
    c.type = COORD_TYPE_2D;
    c.pad_24 = 0;
    *out = c;
    return out;
}

/*
 * 0x0040F6A0: "a,b,c" as c * 10000 + a * 100 + b, or 0 when it does not parse
 * as three numbers (the numbers are stored in that order by the original:
 * "1,0,0" gives 100; callers compare with such values).
 */
int Version_ParseTriple(const char *s)
{
    int a = 0, b = 0, c = 0;

    if (sscanf(s, "%i,%i,%i", &a, &b, &c) != 3) {
        return 0;
    }
    return (c * 100 + a) * 100 + b;
}

/* ------------------------------------------------------------------ end of a frame */

/* The global PalFX back to neutral, with a new serial (inlined by the original). */
static void Frame_ResetPalFx(void)
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

/*
 * 0x0040F9E0: saves the screen as the first free mugen%03i.png (in the
 * working directory; screens of less than 24 bits go through a 32-bit copy).
 */
void Screen_SaveShot(void)
{
    char path[0x200];
    SDL_Surface *s, *copy;
    FILE *fp;

    sprintf(path, "mugen%03i.png", g_screenshotNumber++);
    for (fp = fopen(path, "rb"); fp != NULL; fp = fopen(path, "rb")) {
        fclose(fp);
        sprintf(path, "mugen%03i.png", g_screenshotNumber++);
    }
    s = Gfx_GetScreenSurface();
    if (s == NULL) {
        return;
    }
    if (s->format->BitsPerPixel < 24) {
        copy = SDL_CreateRGBSurface(0, s->w, s->h, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
        if (copy == NULL) {
            return;
        }
        SDL_UpperBlit(s, NULL, copy, NULL);
        fp = fopen(path, "wb");
        if (fp == NULL) {
            MatchLog_Printf(3, "libpng: fopen error\n");
            SDL_FreeSurface(copy);
            return;
        }
        Png_WriteSurface(copy, fp);
        SDL_FreeSurface(copy);
        return;
    }
    fp = fopen(path, "wb");
    if (fp == NULL) {
        MatchLog_Printf(3, "libpng: fopen error\n");
        return;
    }
    Png_WriteSurface(s, fp);
}

/*
 * 0x0040F830: shows the frame (with the screenshot requested by F12 taken
 * first): the render target goes to the window, letterboxed with KeepAspect,
 * with a "SAFE MODE" banner over the first 180 frames in safe mode.
 */
void Screen_Present(void)
{
    GameVars *g = g_game;
    int x, y, w, h;

    if (g->screenshot_requested != 0) {
        Screen_SaveShot();
        g = g_game;
        g->screenshot_requested = 0;
    }
    if (g->skip_draw != 0) {
        return;
    }
    x = 0;
    y = 0;
    w = g_videoWidth;
    h = g_videoHeight;
    if (g->keep_aspect != 0) {
        float aspect, t;
        int fit;
        int gw = Ftol(g->game_coord[0]);
        int gh = Ftol(g->game_coord[1]);
        aspect = (float)((double)gw / gh);
        t = (float)((double)g_videoWidth / aspect);
        t = (float)floor(t + 0.5);
        fit = Ftol(t);
        if (fit > g_videoHeight) {
            w = Ftol((double)g_videoHeight * aspect);
        } else if (fit < g_videoHeight) {
            h = fit;
        }
        x = (g_videoWidth - w) / 2;
        y = (g_videoHeight - h) / 2;
    }
    if (g->safe_mode != 0 && (int)g->frames < 180) {
        unsigned int color = Video_MapRGB(0, 0xFF, 0);
        g_gfxBlendRect(0, 0, g_targetWidth, 11, color, 0x80, 0x80);
        Font_DrawText("SAFE MODE", g_game->font_medium, g_targetWidth / 2, 9, 0, 0, 0xFF, 0xFF, 0xFF);
        g = g_game;
    }
    g_gfxPresent(g->render_target, 0, (float)x, (float)y, (float)w, (float)h);
}

/*
 * 0x0040F780: frame skipping: a frame is skipped every GameVars.frame_skip
 * frames when that is set (speed tests), else after a late frame (more than
 * 4 frames late: the timer is reset instead). Sets GameVars.skip_draw.
 */
void Frame_UpdateSkip(void)
{
    GameVars *g;
    __int64 lag;
    int late;

    g_timerFrame++;
    lag = Timer_GetFrame() - g_timerFrame;
    late = lag < 0 ? 0 : (int)lag;
    g = g_game;
    g->skip_draw = 0;
    if (g->frame_skip != -1) {
        g->frame_skip_count++;
        if (g->frame_skip_count > g->frame_skip) {
            g->frame_skip_count = 0;
        }
    } else if (g->speedup != 0) {
        g->frame_skip_count = 0;
    } else {
        g->frame_skip_count = 0;
        if (late > 4) {
            Timer_ResetFrame();
            g = g_game;
        } else if (late > 1) {
            g->frame_skip_count = 1;
        }
    }
    if (g->frame_skip_count > 0) {
        g->skip_draw = 1;
    }
}

/*
 * 0x0040F700: the end of a frame of the screen loops: waits for the frame
 * time (unless [Debug] Speedup), shows the frame, resets the global PalFX
 * and decides whether the next frame is drawn.
 */
void Screen_EndFrame(void)
{
    /* Moemon Fight: no waiting or skipping by the clock: the frame is shown, then the engine waits for the
     * game to step it again (MfHost_FrameEnd decides whether the next frame is drawn). */
    Screen_Present();
    Frame_ResetPalFx();
    MfHost_FrameEnd();
}
