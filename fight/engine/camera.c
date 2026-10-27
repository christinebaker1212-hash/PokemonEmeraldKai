/*
 * The camera of a match (original module 0x4124A0-0x419A4F, part 2:
 * 0x415A10-0x416E9F).
 *
 * The camera keeps the screen edges in game units: screen_left / screen_right,
 * the current left / right edge, move towards screen_left_target / screen_right_target, the
 * target edges given by the players (ScreenBound movecamera), the tension
 * and the stage bounds; the zoom is the screen width over the distance
 * between the edges. camera_offset_y follows camera_y_target vertically.
 *
 * Floating point follows the x87 code of the original: intermediate results
 * stay in double and are rounded to float where the original stores a float;
 * float to int conversions truncate like __ftol2. The comparisons keep the
 * original's handling of unordered (NaN) operands.
 */
#include "mugen.h"

/* x rounded as the original does it: through float, floor(x + 0.5), float again, then __ftol2. */
static int Cam_Round(double x)
{
    float f = (float)x;

    f = (float)floor(f + 0.5);
    return Ftol(f);
}

/* Half the width of a player (Width controller and size) on the side it is facing away from (-1) or towards (1). */
static int Cam_PlayerEdge(const Player *p, int side)
{
    int w = p->facing == side ? p->width_edge_front : p->width_edge_back;

    return Cam_Round(g_gameCoord[0] / p->data->localcoord[0] * w);
}

/*
 * 0x004157B0: the view of the Zoom controller (GameVars.zoom_active set):
 * the zoomed screen edges (zoom_left, zoom_right) approach the edges of the
 * screen scaled by 1 / Zoom scale around the Zoom position, pushed back
 * between the camera edges, with Math_Approach and the Zoom lag; the draw
 * offset x (shake_x) follows their middle, the offset y (shake_y) the
 * Zoom pos y, and the stage draw scale (stage_scale) is the camera width over
 * the zoomed width. Without a zoom everything goes back to neutral.
 */
void Camera_UpdateZoom(void)
{
    GameVars *g = g_game;
    float lag = g->zoom_lag;
    float scale, w, cx, l, r, d;

    if (g->zoom_active == 0.0f) {
        g->zoom_active = 0.0f;
        g->zoom_x = 0.0f;
        g->shake_x = 0.0f;
        g->zoom_y = 0.0f;
        g->shake_y = 0.0f;
        g->zoom_scale = 1.0f;
        g->stage_scale = 1.0f;
        g->zoom_left = g->screen_left;
        g->zoom_right = g->screen_right;
        return;
    }
    scale = g->zoom_scale;
    w = (float)(((double)g->screen_right - g->screen_left) / scale);
    cx = (float)(g_gameCoord[0] / g->stage.unit * g->camera_x + g->camera_offset_x);
    cx = (float)((double)scale * g->zoom_x + cx);
    l = (float)(cx - (double)w * 0.5);
    r = (float)(cx + (double)w * 0.5);
    if (g->screen_left > l) {
        d = (float)((double)g->screen_left - l);
        l = g->screen_left;
        r = (float)((double)d + r);
    }
    if (g->screen_right < r) {
        d = (float)((double)r - g->screen_right);
        r = g->screen_right;
        l = (float)((double)l - d);
        if (g->screen_left > l) {
            l = g->screen_left;
        }
    }
    l = Math_Approach(g->zoom_left, l, 0.0f, lag, 1.0f);
    g->zoom_left = l;
    r = Math_Approach(g->zoom_right, r, 0.0f, lag, 1.0f);
    g->zoom_right = r;
    cx = (float)(g_gameCoord[0] / g->stage.unit * g->camera_x + g->camera_offset_x);
    g->shake_x = (float)(((double)r + l) * 0.5 - cx);
    g->shake_y = Math_Approach(g->shake_y, g->zoom_y, 0.0f, lag, 1.0f);
    g->stage_scale = (float)(((double)g->screen_right - g->screen_left) / ((double)r - l));
}
/*
 * 0x00416DC0: the vertical crop of the stage: the part of the stage height
 * (Stage.height) that does not fit on the screen is cut at the top and the
 * bottom (cuthigh / cutlow), or the stage is centred with overdrawlow when it
 * is smaller than the screen.
 */
void Camera_UpdateCrop(void)
{
    GameVars *g = g_game;
    Stage *st = &g->stage;
    float h;
    int screen_h, stage_h, d, top, bottom;

    h = (float)((double)Ftol(g->game_coord[1]) / (g_gameCoord[0] / st->unit));
    h = (float)floor(h + 0.5);
    screen_h = Ftol(h);
    stage_h = Ftol(st->height);
    d = stage_h - screen_h;
    if (d < 0) {
        top = d / 2;
        bottom = d - top;
        if (-bottom > st->overdrawlow) {
            bottom = -st->overdrawlow;
            top = d - bottom;
        }
    } else {
        int sum = st->cuthigh + st->cutlow;
        top = sum > 0 ? st->cuthigh * d / sum : d;
        bottom = d - top;
        if (bottom > st->cutlow) {
            bottom = st->cutlow;
            top = d - bottom;
        }
    }
    g->crop_bottom = bottom;
    g->crop_top = top;
    g->camera_y = (float)(g->camera_y_base + top);
}

/*
 * 0x004174C0: the stage backgrounds follow the camera: they tick (always on
 * the first tick of the screen; else not on the special screens, during a
 * Pause / SuperPause with pausebg, paused or in slow motion), get the stage
 * scale (xscale, yscale) and the camera position in stage units (whole
 * units when the background asks for it, half with hires), with the
 * zoffsetlink element moving the screen offset of the stage (draw_offset_y),
 * the screen shake as offset and the zoom.
 */
void Stage_UpdateBgCamera(ScreenLoop *screen)
{
    GameVars *g = g_game;
    float camx = g->camera_offset_x;
    float camy = g->camera_offset_y;
    float s = 1.0f;
    float zoom = (float)((double)g->stage_scale * g->camera_zoom);
    float ox = 0.0f, oy = 0.0f;
    float x, y, bx, by, w, t1, t2;
    Bg *bg = g->stage_bg;
    double k;

    if (bg == NULL) {
        return;
    }
    if (screen->ticks == 0 ||
        (g->screen_kind != 6 && g->screen_kind != 7 && g->screen_kind != 8 &&
         (g->pause_kind == -1 || g->pauses[g->pause_kind][7] == 0) && g->defocus_paused == 0 && g->slow_counter == 0)) {
        Bg_Tick(bg);
    }
    k = g_gameCoord[0] / g->stage.unit;
    {
        float sy = (float)(g->stage.yscale * k);
        float sx = (float)(k * g->stage.xscale);
        Bg_SetScale(g->stage_bg, sx, sy);
    }
    if (g->stage.hires != 0) {
        s = 0.5f;
    }
    bg = g->stage_bg;
    k = g_gameCoord[0] / g->stage.unit;
    x = (float)(camx / k * s / g->stage.xscale);
    y = (float)(camy / k * s / g->stage.yscale);
    if (*(const int *)((const char *)bg + 0x64) != 0) {
        /* the background wants whole camera units */
        x = (float)Ftol(x);
    }
    g->draw_offset_y = g->stage_zoffset;
    if (g->stage.zoffsetlink != -1) {
        BgElem *e;
        Bg_SetCamera(bg, x, y, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
        e = Bg_FindById(g->stage_bg, g->stage.zoffsetlink);
        if (e != NULL) {
            float v = (float)(g_gameCoord[0] / g->stage.unit * *(const float *)((const char *)e + 0x88) + g->camera_offset_y);
            v = (float)floor(v + 0.5);
            g->draw_offset_y += Ftol(v);
        }
    }
    bx = (float)(-g->camera_x / g->stage.xscale);
    by = (float)(-g->camera_y / g->stage.yscale);
    if (g->zoom_active != 0.0f) {
        ox = (float)(0.0 - (double)g->shake_x * g->stage_scale);
        oy = (float)(0.0 - (double)g->shake_y * g->stage_scale);
    }
    k = g_gameCoord[0] / g->stage.unit;
    bg = g->stage_bg;
    w = (float)((double)g->draw_offset_y / k / g->stage.yscale);
    t1 = (float)(oy / zoom / k + by);
    t2 = (float)(ox / zoom / k + bx);
    Bg_SetCamera(bg, x, y, t2, t1, zoom, 0.0f, w);
}

/* 0x00415A70: puts the camera at x, y with the current zoom, edges included. */
void Camera_Set(float x, float y)
{
    GameVars *g = g_game;
    double half;
    float e;

    g->camera_offset_x = x;
    g->camera_offset_y = y;
    g->camera_y_target = y;
    half = (double)Ftol(g->game_coord[0]) / g->camera_zoom * 0.5;
    e = (float)(x - half);
    g->screen_left = e;
    g->screen_left_target = e;
    e = (float)(half + x);
    g->screen_right = e;
    g->screen_right_target = e;
}

/* 0x00415A10: the camera at the stage's start position and zoom. */
void Camera_Reset(void)
{
    GameVars *g = g_game;
    float f;

    g->camera_zoom = g->stage.startzoom;
    Camera_Set((float)g->stage.camera_startx, (float)g->stage.camera_starty);
    f = (float)floor(0.0 + 0.5);
    g->camera_y_base = Ftol(f);
    Camera_UpdateCrop();
}

/* !(a <= b): a > b, or unordered. */
#define GT_OR_NAN(a, b) (!((a) <= (b)))
/* !(a >= b): a < b, or unordered. */
#define LT_OR_NAN(a, b) (!((a) >= (b)))

/*
 * 0x00415AE0: one tick of the camera (GameVars.camera_mode >= 2; 2 snaps the
 * edges to their targets): the target edges follow the outermost players
 * that move the camera horizontally (with the tension, the zoom limits and
 * the stage bounds), the edges move towards them at a limited speed, and the
 * camera follows the highest player vertically (verticalfollow >= 0) or
 * zooms to keep the highest and lowest ones on the screen (verticalfollow < 0).
 */
void Camera_Update(void)
{
    GameVars *g = g_game;
    Stage *st = &g->stage;
    double k0 = g_gameCoord[0] / st->unit;
    double k1, hz;
    Player *left = NULL, *right = NULL, *top = NULL, *bottom = NULL;
    int snap = 0;
    float tension, floortension, tensionhigh, tensionlow, vfollow, wf, speed_l, speed_r, old_ty;
    float boundlow, boundhigh, cam_w, zoomout_w, l, r, ll, rl, f;
    int i, w;

    tension = (float)(st->tension * k0);
    floortension = (float)(st->floortension * k0);
    tensionhigh = (float)(st->tensionhigh * k0);
    tensionlow = (float)(st->tensionlow * k0);
    vfollow = st->verticalfollow;
    wf = (float)Ftol(g->game_coord[0]);
    speed_l = (float)((double)0.1f * wf / 320.0);
    speed_r = speed_l;
    old_ty = g->camera_y_target;
    boundlow = (float)(st->boundlow * k0);
    boundhigh = (float)(st->boundhigh * k0);
    cam_w = (float)(wf / st->zoomin);
    zoomout_w = (float)(wf / st->zoomout);
    l = g->screen_left_target;
    r = g->screen_right_target;
    if (g->camera_mode <= 1) {
        Camera_UpdateCrop();
        return;
    }
    if (g->camera_mode == 2) {
        snap = 1;
    }
    for (i = g->players_first; i <= g->players_last; i++) {
        Player *p = g->players[i];
        if (p != NULL && p->active) {
            Player_UpdateDrawParams(p, 1);
        }
    }

    /* the outermost players moving the camera */
    for (i = g->players_first; i <= g->players_last; i++) {
        Player *p = g->players[i];
        if (p == NULL || !p->active) {
            continue;
        }
        if (p->movecamera_x != 0) {
            if (left != NULL) {
                double px = p->pos.x - Cam_PlayerEdge(p, -1);
                double lx = left->pos.x - Cam_PlayerEdge(left, -1);
                if (lx > px) {
                    left = p;
                }
            } else {
                left = p;
            }
            if (right != NULL) {
                double px = Cam_PlayerEdge(p, 1) + p->pos.x;
                double rx = Cam_PlayerEdge(right, 1) + right->pos.x;
                if (rx < px) {
                    right = p;
                }
            } else {
                right = p;
            }
        }
        if (p->movecamera_y != 0) {
            if (top == NULL || top->pos.y > p->pos.y) {
                top = p;
            }
            if (bottom == NULL || bottom->pos.y < p->pos.y) {
                bottom = p;
            }
        }
    }

    k1 = g_gameCoord[0] / st->unit;
    hz = 0.5 * zoomout_w;
    ll = (float)(st->boundleft * k1 - hz);
    rl = (float)(st->boundright * k1 + hz);
    if (top != NULL && bottom != NULL && left != NULL && right != NULL) {
        float xl, xr, sh, q1, q2, d;
        double lv, rv;

        /* vertical target, or the zoom that keeps the top and bottom players on the screen */
        if (0.0f <= vfollow) {
            float t = (float)(floortension + top->pos.y);
            g->camera_y_target = vfollow * t;
        } else {
            float a = (float)(top->pos.y - tensionhigh);
            float b = (float)(tensionlow + bottom->pos.y);
            if (b > a) {
                float z = (float)((double)Ftol(g->game_coord[1]) / ((double)b - a));
                if (z > 0.0f) {
                    float w2 = (float)((double)Ftol(g->game_coord[0]) / z);
                    float wz;
                    if (cam_w > w2) {
                        g->camera_y_target = 0.0f;
                        wz = cam_w;
                    } else {
                        wz = w2;
                    }
                    cam_w = wz > zoomout_w ? zoomout_w : wz;
                }
            }
        }

        /* the edges of the outermost players */
        w = Cam_PlayerEdge(left, -1);
        xl = (float)(left->pos.x - (float)w);
        w = Cam_PlayerEdge(right, 1);
        xr = (float)((double)w + right->pos.x);

        if (cam_w > (double)r - l) {
            float mid = (float)(((double)l + r) * 0.5);
            double hcw = cam_w * 0.5;
            l = (float)(mid - hcw);
            r = (float)(hcw + mid);
        }
        {
            double width = ((double)xr - xl) + ((double)tension + tension);
            if (width < cam_w) {
                float dx = 0.0f;
                if ((double)r - l > cam_w) {
                    float mid = (float)(((double)l + r) * 0.5);
                    double hcw = 0.5 * cam_w;
                    l = (float)(mid - hcw);
                    r = (float)(hcw + mid);
                }
                if ((double)xl - tension < l) {
                    dx = (float)(((double)xl - tension) - l);
                }
                if ((double)xr + tension > r) {
                    dx = (float)(((double)xr + tension) - r);
                }
                l = (float)(l + (double)dx);
                r = (float)(r + (double)dx);
            } else {
                float mid = (float)(((double)xl + xr) * 0.5);
                if (zoomout_w >= width) {
                    l = (float)((double)xl - tension);
                    r = (float)((double)xr + tension);
                } else {
                    l = (float)(mid - hz);
                    r = (float)(mid + hz);
                }
            }
        }

        /* the stage bounds */
        if (ll > l) {
            double xrt;
            sh = (float)((double)ll - l);
            l = ll;
            xrt = (double)xr + tension;
            if ((double)sh + r > xrt) {
                sh = (float)(xrt - r);
            }
            if (((double)sh + r) - ll < cam_w) {
                sh = (float)(cam_w - ((double)r - ll));
            }
            if (0.0f < sh) {
                r = (float)(r + (double)sh);
            }
            if (rl < r) {
                r = rl;
            }
        }
        if (rl < r) {
            sh = (float)((double)r - rl);
            r = rl;
            if ((double)xl - tension > (double)l - sh) {
                sh = (float)(((double)l - xl) + tension);
            }
            if ((double)rl - ((double)l - sh) < cam_w) {
                sh = (float)(cam_w - ((double)rl - l));
            }
            if (0.0f < sh) {
                l = (float)(l - (double)sh);
            }
            if (ll > l) {
                l = ll;
            }
        }
        if (l < r) {
            g->screen_left_target = l;
            g->screen_right_target = r;
        }

        /* zooming to keep the top and bottom players: the vertical target */
        if (0.0f > vfollow && g->screen_left_target < g->screen_right_target) {
            float cyk = (float)(g->camera_y * k1);
            float zf = (float)((double)Ftol(g->game_coord[0]) / ((double)g->screen_right_target - g->screen_left_target));
            float tt = (float)(top->pos.y - tensionhigh);
            g->camera_y_target = (float)(((double)g->draw_offset_y - cyk) / zf + tt);
        }

        /* speed of the edges towards their targets */
        q1 = (float)(((double)xl - g->screen_left) - tension);
        q2 = (float)(((double)g->screen_right - xr) - tension);
        d = (float)((double)g->screen_left_target - g->screen_left);
        f = d < 0.0f ? -1.0f : (d > 0.0f ? 1.0f : 0.0f);
        speed_l = (float)(((double)g->screen_left_target * 0.05 + (double)g->screen_left * 0.95 - g->screen_left) +
                          (double)f * speed_l);
        d = (float)((double)g->screen_right_target - g->screen_right);
        f = d < 0.0f ? -1.0f : (d > 0.0f ? 1.0f : 0.0f);
        speed_r = (float)(((double)g->screen_right_target * 0.05 + (double)g->screen_right * 0.95 - g->screen_right) +
                          (double)f * speed_r);
        lv = left->vel.x;
        if (speed_l > lv && speed_l < 0.0f && 0.0f > q1) {
            speed_l = (float)lv;
            if (speed_r < 0.0f && speed_l < speed_r) {
                speed_r = speed_l;
            }
        }
        rv = right->vel.x;
        if (speed_r < rv && speed_r > 0.0f && 0.0f > q2) {
            speed_r = (float)rv;
            if (speed_l > 0.0f && speed_r > speed_l) {
                speed_l = speed_r;
            }
        }
    } else {
        speed_l = 0.0f;
        speed_r = 0.0f;
    }

    /* the vertical target moves smoothly when it jumps */
    if (g->round_state != 0 && vfollow >= 0.0f) {
        float dy = (float)fabs((double)g->camera_y_target - old_ty);
        if (dy > 1.0f) {
            g->camera_y_target = (float)(old_ty * 0.85 + g->camera_y_target * 0.15);
        }
    }
    if (g->defocus_paused == 0) {
        float a, t, n;

        /* the edges move towards their targets */
        a = g->screen_left;
        t = g->screen_left_target;
        if (t < a) {
            n = (float)(a + (double)speed_l);
            g->screen_left = GT_OR_NAN(t, n) ? t : n;
        } else if (t > a) {
            n = (float)(a + (double)speed_l);
            g->screen_left = LT_OR_NAN(t, n) ? t : n;
        }
        a = g->screen_right;
        t = g->screen_right_target;
        if (t > a) {
            n = (float)(a + (double)speed_r);
            g->screen_right = LT_OR_NAN(t, n) ? t : n;
        } else if (t < a) {
            n = (float)(a + (double)speed_r);
            g->screen_right = GT_OR_NAN(t, n) ? t : n;
        }
        if (snap) {
            g->screen_left = g->screen_left_target;
            g->screen_right = g->screen_right_target;
        }
        {
            float d1 = (float)fabs((double)g->screen_left - g->screen_left_target);
            float av = (float)fabs(speed_l);
            if (av > d1) {
                g->screen_left = g->screen_left_target;
            }
            d1 = (float)fabs((double)g->screen_right - g->screen_right_target);
            av = (float)fabs(speed_r);
            if (av > d1) {
                g->screen_right = g->screen_right_target;
            }
        }
        if (zoomout_w < (double)g->screen_right - g->screen_left) {
            if (g->screen_right_target >= g->screen_right) {
                g->screen_left = (float)((double)g->screen_right - zoomout_w);
            } else if (g->screen_left_target <= g->screen_left) {
                g->screen_right = (float)((double)zoomout_w + g->screen_left);
            }
        }
        if (ll > g->screen_left) {
            g->screen_left = ll;
        }
        if (rl < g->screen_right) {
            g->screen_right = rl;
        }

        /* vertical */
        if (vfollow >= 0.0f) {
            float cy = g->camera_offset_y, ty = g->camera_y_target;
            if (ty > cy) {
                float wf2 = (float)Ftol(g->game_coord[0]);
                float hw = (float)(0.5 * wf2 / 320.0);
                float m;
                n = (float)((double)hw + cy);
                g->camera_offset_y = n;
                m = (float)(wf2 * 5.0 / 320.0);
                if ((double)ty - m > n) {
                    g->camera_offset_y = (float)(n + vfollow * ((double)ty - n) / 10.0);
                }
                if (g->camera_offset_y >= (double)ty - hw) {
                    g->camera_offset_y = ty;
                }
            } else if (ty < cy) {
                float wf2 = (float)Ftol(g->game_coord[0]);
                float hw = (float)(0.5 * wf2 / 320.0);
                float m;
                n = (float)((double)cy - hw);
                g->camera_offset_y = n;
                m = (float)(wf2 * 5.0 / 320.0);
                if ((double)m + ty < n) {
                    g->camera_offset_y = (float)(n - vfollow * ((double)n - ty) / 10.0);
                }
                if (g->camera_offset_y <= (double)ty + hw) {
                    g->camera_offset_y = ty;
                }
            }
        } else {
            float step = (float)((double)Ftol(g->game_coord[0]) * 5.0 / 320.0);
            g->camera_offset_y = Math_Approach(g->camera_offset_y, g->camera_y_target, step, 0.5f, 1.0f);
        }
        if (boundlow < g->camera_offset_y) {
            g->camera_offset_y = boundlow;
        }
        if (boundhigh > g->camera_offset_y) {
            g->camera_offset_y = boundhigh;
        }
    }
    g->camera_offset_x = (float)(((double)g->screen_left + g->screen_right) * 0.5);
    g->camera_zoom = (float)((double)Ftol(g->game_coord[0]) / ((double)g->screen_right - g->screen_left));
    Camera_UpdateCrop();
    for (i = g->players_first; i <= g->players_last; i++) {
        Player *p = g->players[i];
        if (p != NULL && p->active) {
            Player_UpdateDrawParams(p, 1);
        }
    }
}
