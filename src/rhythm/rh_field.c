// Moemon Rhythm: a player's field on the DANCE machine's screen: the arrows coming up (or down with REVERSE) to
// the receptors, freeze arrows, the flash of a step, the judgment and combo, the O.K. / N.G. of a freeze, and the
// life bar and score in the frames at the top and bottom. Sized for 240x160: 20 pixel arrows, 21 pixels apart,
// a beat is 20 pixels at x1 (the arcade's arrow a beat).
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "rh.h"

#define ARROW 20
#define COL_W 21
#define RECEPTOR_Y 22
#define RECEPTOR_Y_REV (RH_H - 20 - ARROW - 2)
#define FIELD_H 150

const float gRhSpeeds[RH_SPEEDS] = {1, 1.5f, 2, 3, 5, 8, 0.5f};
static const int sRot[RH_COLS] = {RH_ROT_90, RH_ROT_0, RH_ROT_180, RH_ROT_270};

void RhField_Begin(struct RhField *f, int side, int remote)
{
    struct RhOptions opt = f->opt;
    int diff = f->diff, meter = f->meter;
    memset(f, 0, sizeof(*f));
    f->opt = opt;
    f->diff = diff;
    f->meter = meter;
    f->used = 1;
    f->side = (uint8_t)side;
    f->remote = (uint8_t)remote;
    f->lifeShown = 0.5f;
    f->netLife = 0.5f;
}

int RhField_Score(const struct RhField *f)
{
    return f->remote ? f->netScore : f->play.score;
}

int RhField_Combo(const struct RhField *f)
{
    return f->remote ? f->netCombo : f->play.combo;
}

float RhField_Life(const struct RhField *f)
{
    return f->remote ? f->netLife : f->play.life;
}

int RhField_Failed(const struct RhField *f)
{
    return f->remote ? f->netFailed : f->play.failed;
}

static void ComboCall(int before, int now)
{
    // the announcer at each 100 (to 2000)
    int b = before / 100, n = now / 100;
    if (n > b && n >= 1 && n <= 20)
    {
        char folder[40];
        snprintf(folder, sizeof(folder), "gameplay %d combo", n * 100);
        RhAudio_Announce(folder);
    }
}

void RhField_Events(struct RhField *f, float dt, int announce)
{
    int i, comboBefore = f->comboShown;
    f->judgeTime += dt;
    for (i = 0; i < RH_COLS; i++)
    {
        f->flash[i] -= dt * 6;
        if (f->flash[i] < 0)
            f->flash[i] = 0;
        f->press[i] -= dt * 8;
        if (f->press[i] < 0)
            f->press[i] = 0;
        if (f->held[i])
            f->press[i] = 1;
        f->holdJudgeTime[i] += dt;
    }
    for (i = 0; i < f->play.eventCount; i++)
    {
        const struct RhEvent *e = &f->play.events[i];
        switch (e->kind)
        {
        case RH_EV_TAP:
            if (e->judge >= RH_J_GOOD && e->col < RH_COLS)
            {
                f->flash[e->col] = 1;
                f->flashBright[e->col] = e->judge >= RH_J_PERFECT;
            }
            break;
        case RH_EV_ROW:
            f->judge = e->judge;
            f->judgeTime = 0;
            break;
        case RH_EV_HOLD:
            if (e->col < RH_COLS && (e->judge == RH_HOLD_OK || e->offset > 0))
            {
                f->holdJudge[e->col] = e->judge;
                f->holdJudgeTime[e->col] = 0;
                if (e->judge == RH_HOLD_OK)
                {
                    f->flash[e->col] = 1;
                    f->flashBright[e->col] = 1;
                }
            }
            break;
        }
    }
    f->play.eventCount = 0;
    f->comboShown = RhField_Combo(f);
    if (announce && f->comboShown > comboBefore)
        ComboCall(comboBefore, f->comboShown);
    {
        float life = RhField_Life(f);
        f->lifeShown += (life - f->lifeShown) * (dt * 12 > 1 ? 1 : dt * 12);
    }
}

// How far from the receptor an arrow this many beats away is drawn (BOOST speeds them up as they come)
static float Offset(const struct RhField *f, double beats)
{
    float y = (float)(beats * ARROW * gRhSpeeds[f->opt.speed % RH_SPEEDS]);
    if (f->opt.boost && y > 0)
        y = y * 1.5f / ((y + FIELD_H / 1.2f) / FIELD_H);
    return y;
}

static int Alpha(const struct RhField *f, float y)
{
    float a = 1;
    switch (f->opt.appearance)
    {
    case RH_APP_HIDDEN:
        a = (y - 40) / 16;
        break;
    case RH_APP_SUDDEN:
        a = (86 - y) / 16;
        break;
    case RH_APP_STEALTH:
        a = 0;
        break;
    }
    if (a > 1)
        a = 1;
    if (a < 0)
        a = 0;
    return (int)(a * 256);
}

static const struct RhPic *ArrowPic(const struct RhField *f, int quant)
{
    char name[24];
    switch (f->opt.arrows)
    {
    case RH_ARROWS_NOTE:
        snprintf(name, sizeof(name), "note_%d", quant);
        break;
    case RH_ARROWS_FLAT:
        return RhPic_Get("flat");
    case RH_ARROWS_SOLO:
        snprintf(name, sizeof(name), "solo_%d", quant);
        break;
    default:
        snprintf(name, sizeof(name), "vivid_%d", quant);
        break;
    }
    return RhPic_Get(name);
}

static int ArrowFrame(const struct RhField *f, const struct RhPic *pic, double beat, double noteBeat)
{
    double phase;
    if (!pic)
        return 0;
    // VIVID's colors run with the note's place in the beat; the others with the song's beat
    phase = f->opt.arrows == RH_ARROWS_VIVID ? beat - noteBeat + noteBeat * 0 : beat;
    phase = phase - floor(phase);
    return (int)(phase * pic->frames) % pic->frames;
}

static void DrawHold(const struct RhField *f, int i, double beat, int x, int ry, int dir)
{
    const struct RhPlay *p = &f->play;
    const struct RhNote *n = &p->notes[i];
    int on = p->hold[i] == RH_HOLD_ACTIVE && f->held[n->col];
    int dead = p->hold[i] == RH_HOLD_NG || (p->judge[i] == RH_J_MISS);
    const struct RhPic *body = RhPic_Get(dead || !on ? "hold_body_off" : "hold_body_on");
    const struct RhPic *cap = RhPic_Get(dead || !on ? "hold_cap_off" : "hold_cap_on");
    float y0, y1, y;
    int alpha;
    if (p->hold[i] == RH_HOLD_OK)
        return;
    // the head stays on the receptor while it's held
    y0 = (p->hold[i] == RH_HOLD_ACTIVE && beat > n->beat) ? 0 : Offset(f, n->beat - beat);
    y1 = Offset(f, n->endBeat - beat);
    if (y1 < y0)
        return;
    alpha = dead ? 160 : 256;
    if (!body)
        return;
    // the body from the head's middle to the tail, then the cap
    for (y = y0 + ARROW / 2; y < y1 + ARROW / 2; y += body->h)
    {
        int h = (int)(y1 + ARROW / 2 - y);
        int sy;
        // (less than a pixel left: nothing; a height of 0 would draw the whole tile, a flash of tail past the end)
        if (h <= 0)
            break;
        if (h > body->h)
            h = body->h;
        sy = dir > 0 ? ry + (int)y : ry + ARROW - (int)y - h;
        if ((dir > 0 && sy > RH_H) || (dir < 0 && sy + h < 0))
            break;
        RhGfx_PicPart(body, 0, x, sy, 0, h, dir > 0 ? RH_ROT_0 : RH_ROT_180, alpha * Alpha(f, y) >> 8);
    }
    if (cap)
    {
        int cy = dir > 0 ? ry + (int)y1 + ARROW / 2 : ry - (int)y1 + ARROW / 2 - cap->h;
        RhGfx_Pic(cap, 0, x, cy, dir > 0 ? RH_ROT_0 : RH_ROT_180, alpha * Alpha(f, y1) >> 8);
    }
}

void RhField_Draw(struct RhField *f, const struct RhSong *song, double t, double beat, int cx)
{
    const struct RhPlay *p = &f->play;
    int dir = f->opt.reverse ? -1 : 1;
    int ry = f->opt.reverse ? RECEPTOR_Y_REV : RECEPTOR_Y;
    int left = cx - (COL_W * RH_COLS) / 2, i;
    const struct RhPic *receptor = RhPic_Get("receptor"), *go = RhPic_Get("receptor_go");
    (void)song;
    (void)t;
    if (!f->used)
        return;
    // receptors (DARK hides them); they flash on the beat and when stepped on
    if (!f->opt.dark)
        for (i = 0; i < RH_COLS; i++)
        {
            double frac = beat - floor(beat);
            int glow = beat >= 0 && frac < 0.15 ? (int)((0.15 - frac) / 0.15 * 140) : 0;
            int x = left + i * COL_W;
            if (f->press[i] > 0 && go)
                RhGfx_Pic(go, 1, x, ry, sRot[i], 256);
            else
                RhGfx_PicGlow(receptor, 0, x, ry, sRot[i], 256, glow);
        }
    if (!p->notes)
        goto overlay;
    // freezes, then arrows far to near (the nearest on top)
    for (i = p->count - 1; i >= p->first; i--)
    {
        const struct RhNote *n = &p->notes[i];
        float y;
        if (n->type != RH_NOTE_HOLD)
            continue;
        y = Offset(f, n->beat - beat);
        if (y > FIELD_H + 40)
            continue;
        DrawHold(f, i, beat, left + n->col * COL_W, ry, dir);
    }
    for (i = p->count - 1; i >= p->first; i--)
    {
        const struct RhNote *n = &p->notes[i];
        float y;
        int sy, x = left + n->col * COL_W, a;
        if (n->type == RH_NOTE_MINE)
            continue;
        if (p->judge[i] != RH_J_NONE && p->judge[i] != RH_J_MISS && n->type != RH_NOTE_HOLD)
            continue;   // stepped on: gone
        if (n->type == RH_NOTE_HOLD && (p->hold[i] == RH_HOLD_OK || (p->hold[i] == RH_HOLD_NG && p->judge[i] != RH_J_MISS)))
            continue;
        y = Offset(f, n->beat - beat);
        if (n->type == RH_NOTE_HOLD && p->hold[i] == RH_HOLD_ACTIVE && y < 0)
            y = 0;
        if (y > FIELD_H + 4)
            continue;
        if (y < -ARROW * 2)
            continue;
        sy = ry + (int)(dir * y);
        a = Alpha(f, y);
        if (p->judge[i] == RH_J_MISS)
            a = a * 3 / 4;
        if (n->type == RH_NOTE_HOLD)
        {
            const struct RhPic *head = RhPic_Get(p->hold[i] == RH_HOLD_ACTIVE && f->held[n->col] ? "hold_head_on"
                                                 : "hold_head_off");
            RhGfx_Pic(head, 0, x, sy, sRot[n->col], a);
        }
        else
        {
            const struct RhPic *pic = ArrowPic(f, n->quant);
            RhGfx_Pic(pic, ArrowFrame(f, pic, beat, n->beat), x, sy, sRot[n->col], a);
        }
    }
overlay:
    // the step's flash over the receptor
    for (i = 0; i < RH_COLS; i++)
        if (f->flash[i] > 0)
        {
            const struct RhPic *flash = RhPic_Get(f->flashBright[i] ? "flash_bright" : "flash_dim");
            if (flash)
                RhGfx_Pic(flash, 0, left + i * COL_W + ARROW / 2 - flash->w / 2, ry + ARROW / 2 - flash->h / 2, 0,
                          (int)(f->flash[i] * 256));
        }
    // O.K. / N.G. under the receptors
    for (i = 0; i < RH_COLS; i++)
        if (f->holdJudge[i] && f->holdJudgeTime[i] < 0.6f)
        {
            const struct RhPic *hj = RhPic_Get("holdjudge");
            int a = f->holdJudgeTime[i] > 0.4f ? (int)((0.6f - f->holdJudgeTime[i]) / 0.2f * 256) : 256;
            if (hj)
                RhGfx_Pic(hj, f->holdJudge[i] == RH_HOLD_OK ? 0 : 1, left + i * COL_W + ARROW / 2 - hj->w / 2,
                          ry + dir * (ARROW + 3) + (dir < 0 ? ARROW - hj->h : 0), 0, a);
        }
    // the judgment pops up, then fades; the combo under it
    if (f->judge && f->judgeTime < 0.9f)
    {
        const struct RhPic *jp = RhPic_Get("judge");
        static const int frames[RH_J_COUNT] = {0, 5, 4, 3, 2, 1, 0};
        int frame = frames[f->judge], jy = RH_H / 2 - 8 + (f->opt.reverse ? 8 : 0);
        float pop = f->judgeTime < 0.08f ? 1.25f - f->judgeTime / 0.08f * 0.25f : 1;
        int a = f->judgeTime > 0.7f ? (int)((0.9f - f->judgeTime) / 0.2f * 256) : 256;
        if (f->judge == RH_J_MARVELOUS && !f->opt.marvelous)
            frame = 1;
        if (jp)
            RhGfx_PicScaled(jp, frame, cx, jy, (int)(jp->w * pop), (int)(jp->h * pop), a);
        if (RhField_Combo(f) >= 4 && f->judge >= RH_J_GREAT)
        {
            const struct RhPic *digits = RhPic_Get("combo_digits"), *label = RhPic_Get("combo_label");
            int w = RhGfx_Number(digits, -1000, 0, (uint32_t)RhField_Combo(f), RH_LEFT, 1, 0, -1);
            int lw = label ? label->w : 0, x0 = cx - (w + 2 + lw) / 2;
            RhGfx_Number(digits, x0, jy + 7, (uint32_t)RhField_Combo(f), RH_LEFT, 1, a, -1);
            if (label)
                RhGfx_Pic(label, 0, x0 + w + 2, jy + 7 + (digits ? digits->h - label->h : 0), 0, a);
        }
    }
}

// The life bar's window in life_frame: the left one from x 34 to 108 and the right from 132 to 206, rows 4-10
void RhField_DrawLife(const struct RhField *f, double beat)
{
    int x0 = f->side ? 132 : 34, w = 74, fill, i;
    float life = f->lifeShown;
    int hot = life >= 0.999f, danger = life < 0.25f;
    fill = (int)(life * w + 0.5f);
    for (i = 0; i < w; i++)
    {
        // 1P fills from its outer edge inwards, 2P mirrored; stripes move with the beat
        int x = f->side ? x0 + w - 1 - i : x0 + i;
        int stripe = ((i + (int)(beat * 8)) / 3) & 1;
        uint32_t c;
        if (i >= fill)
            c = RH_RGB(16, 16, 24);
        else if (hot)
            c = stripe ? RH_RGB(255, 255, 255) : RH_RGB(255, 230, 90);
        else if (danger)
            c = stripe ? RH_RGB(255, 60, 40) : RH_RGB(200, 20, 20);
        else
            c = stripe ? RH_RGB(120, 230, 255) : RH_RGB(40, 170, 240);
        RhGfx_Fill(x, 4, 1, 7, c, 256);
    }
    if (RhField_Failed(f))
        RhGfx_Fill(x0, 4, w, 7, RH_RGB(0, 0, 0), 200);
}

void RhField_DrawScore(const struct RhField *f)
{
    const struct RhPic *digits = RhPic_Get("score_digits");
    if (f->side == 0)
        RhGfx_Number(digits, 6, RH_H - 15, (uint32_t)RhField_Score(f), RH_LEFT, 9, 256, 0);
    else
        RhGfx_Number(digits, RH_W - 6, RH_H - 15, (uint32_t)RhField_Score(f), RH_RIGHT, 9, 256, 0);
}
