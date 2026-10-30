/* HOW TO PLAY: a short guide to each of the CARD CASE's three games (SELECT on a game's first screen).
 *
 * Each page is a title, a few terse lines along the bottom strip, and a stage that acts the rule out with the games'
 * own cards (G_Card, Ygo_DrawCard, Tt_DrawCard) on a loop. LEFT / RIGHT (or A) turn the pages, B closes it. The
 * stages are drawn from a frame counter only, so a page always plays the same way. */
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>
#include "global.h"
#include "sound.h"
#include "constants/songs.h"
#include "tcg/tcg.h"
#include "tcg/card_ids.h"
#include "tcg_gfx.h"
#include "tcg_ui_kit.h"
#include "tcg_guide.h"
#include "ygo/ygo.h"
#include "../ygo/ygo_gfx.h"
#include "tt/tt.h"
#include "../tt/tt_gfx.h"
#include "mp.h"

/* The buttons (as the card screens number them: the GBA's) */
enum
{
    KEY_A = 1,
    KEY_B = 2,
    KEY_SELECT = 4,
    KEY_START = 8,
    KEY_RIGHT = 16,
    KEY_LEFT = 32,
    KEY_UP = 64,
    KEY_DOWN = 128,
    KEY_R = 256,
    KEY_L = 512
};

/* The stage */
#define SX 4
#define SY 20
#define SW 232
#define SH 88
#define CX (SX + SW / 2)
#define STRIP 110   /* the words: two lines of the small font, or three of the narrow one, above the hints */

struct Page
{
    const char *title, *text;
    void (*draw)(int t);
    int loop;   /* frames before the stage starts again */
};

static struct
{
    int active, leaving, page, lastPage, slide;
    unsigned game, t, frame;
} G;

/* ------------------------------------------------------------------ */
/* Motion                                                               */
/* ------------------------------------------------------------------ */
static float Clamp01(float v) { return v < 0 ? 0 : v > 1 ? 1 : v; }
/* 0..1 as t goes from a to b */
static float Seg(int t, int a, int b) { return b <= a ? (t >= b ? 1.0f : 0.0f) : Clamp01((float)(t - a) / (float)(b - a)); }
static float Out(float k) { k = Clamp01(k); return 1 - (1 - k) * (1 - k) * (1 - k); }
static float InOut(float k) { k = Clamp01(k); return k < 0.5f ? 4 * k * k * k : 1 - powf(-2 * k + 2, 3) / 2; }
static float Lerp(float a, float b, float k) { return a + (b - a) * k; }
/* A small hop at the end of a move (cards landing) */
static float Land(float k) { return k >= 1 ? 0 : sinf(Clamp01(k) * 3.14159f) * -6; }
/* Fades in over the first frames of [a, b] and out over the last */
static unsigned Show(int t, int a, int b)
{
    float k;
    if (t < a || t > b)
        return 0;
    k = Clamp01((float)(t - a) / 8.0f);
    if (b - t < 8)
        k = k < (float)(b - t) / 8.0f ? k : (float)(b - t) / 8.0f;
    return (unsigned)(255 * k);
}

/* ------------------------------------------------------------------ */
/* Pieces of the stage                                                  */
/* ------------------------------------------------------------------ */
static unsigned Felt(void)
{
    return G.game == KIT_GAME_YUGIOH ? 0x283464 : G.game == KIT_GAME_TRIAD ? 0x5a3c70 : 0x2f7a4c;
}
static unsigned Accent(void)
{
    return G.game == KIT_GAME_YUGIOH ? 0x8070e8 : G.game == KIT_GAME_TRIAD ? 0xd8a838 : 0xe85048;
}
static void Stage(void)
{
    unsigned felt = Felt();
    int y;
    G_FillA(SX + 2, SY + 3, SW, SH, 0x000000, 70);
    G_RoundFill(SX, SY, SW, SH, felt, 255);
    for (y = 2; y < SH - 2; y += 3)
        G_FillA(SX + 2, SY + y, SW - 4, 1, 0xffffff, 8);
    G_FillA(SX + 2, SY + 2, SW - 4, 12, 0xffffff, 14);
    G_RoundFrame(SX, SY, SW, SH, G_Mix(felt, 0x000000, 0.5f), 255);
    G_RoundFrame(SX + 1, SY + 1, SW - 2, SH - 2, G_Mix(felt, 0xffffff, 0.3f), 110);
}
/* A place on the table: a faint outline and its name */
static void Zone(int x, int y, int w, int h, const char *label)
{
    G_FillA(x, y, w, h, 0x000000, 34);
    G_Frame(x, y, w, h, 0xffffff, 70);
    if (label && *label)
        G_TextAlign(x + w / 2, y + h + 1, TA_CENTER, 0xe8f0e8, G_NO_SHADOW, label, TF_NARROW);
}
/* A label on the stage: a pill with a word or two */
static void Tag(int cx, int cy, const char *text, unsigned color, unsigned alpha)
{
    int w = G_Measure(text, TF_NARROW) + 8;
    if (!alpha)
        return;
    G_RoundFill(cx - w / 2 + 1, cy - 5, w, 11, 0x000000, alpha / 3);
    G_RoundFill(cx - w / 2, cy - 6, w, 11, color, alpha);
    G_RoundFrame(cx - w / 2, cy - 6, w, 11, G_Mix(color, 0xffffff, 0.5f), alpha);
    G_TextAlign(cx, cy - 7, TA_CENTER, G_WHITE, G_Mix(color, 0x000000, 0.5f), text, TF_NARROW);
}
/* White words on the felt */
static void Say(int cx, int y, const char *text, unsigned alpha)
{
    if (alpha < 40)
        return;
    G_TextAlign(cx, y, TA_CENTER, G_WHITE, G_Mix(Felt(), 0x000000, 0.6f), text, TF_SMALL);
}
static void Ring(int cx, int cy, float k, unsigned color)
{
    int i;
    if (k <= 0 || k >= 1)
        return;
    for (i = 0; i < 24; i++)
    {
        float a = i * 6.2832f / 24, r = 6 + k * 22;
        G_FillA(cx + (int)(cosf(a) * r), cy + (int)(sinf(a) * r), 2, 2, color, (unsigned)(255 * (1 - k)));
    }
}
static void Burst(int cx, int cy, float k, unsigned color)
{
    int i;
    if (k <= 0 || k >= 1)
        return;
    for (i = 0; i < 10; i++)
    {
        float a = i * 0.628f + 0.3f, r = 4 + k * 18;
        G_FillA(cx + (int)(cosf(a) * r), cy + (int)(sinf(a) * r) + (int)(k * k * 10), 3, 3, color, (unsigned)(255 * (1 - k)));
    }
}
/* A card breaking up: pieces falling away */
static void Shatter(int x, int y, int w, int h, float k, unsigned color)
{
    int i;
    if (k <= 0 || k >= 1)
        return;
    for (i = 0; i < 12; i++)
    {
        int px = x + (i % 3) * w / 3, py = y + (i / 3) * h / 4;
        float dx = ((i % 3) - 1) * 12 * k, dy = k * k * 30 - 6 * k;
        G_FillA(px + (int)dx, py + (int)dy, w / 3 - 1, h / 4 - 1, color, (unsigned)(220 * (1 - k)));
    }
}
static void Arrow(int x0, int y0, int x1, int y1, unsigned color, unsigned alpha)
{
    float dx = (float)(x1 - x0), dy = (float)(y1 - y0), len = sqrtf(dx * dx + dy * dy);
    if (len < 1 || !alpha)
        return;
    dx /= len, dy /= len;
    G_Line(x0, y0, x1, y1, color, alpha);
    G_Line(x0 + 1, y0, x1 + 1, y1, color, alpha);
    G_Line(x1, y1, x1 - (int)(dx * 5 - dy * 4), y1 - (int)(dy * 5 + dx * 4), color, alpha);
    G_Line(x1, y1, x1 - (int)(dx * 5 + dy * 4), y1 - (int)(dy * 5 - dx * 4), color, alpha);
}
/* A list of button hints down the stage's right side */
static void Controls(int x, int y, const char *const *lines, int count, int lit)
{
    int i;
    for (i = 0; i < count; i++)
    {
        unsigned on = i == lit;
        if (on)
            G_RoundFill(x - 3, y + i * 13 - 1, SX + SW - x - 2, 13, 0xffffff, 40);
        G_Hint(x, y + i * 13, on ? G_WHITE : 0xd0d8e0, G_Mix(Felt(), 0, 0.6f), lines[i], TF_SMALL, 1);
    }
}

/* ------------------------------------------------------------------ */
/* POKeMON cards                                                        */
/* ------------------------------------------------------------------ */
#define PW 30
#define PH 42
static void Shadow(float x, float y, float w, float h, double angle, unsigned alpha)
{
    int side = fabs(fmod(fabs(angle), 180.0) - 90) < 45;   /* (turned sideways: the shadow too) */
    float sw = side ? h : w, sh = side ? w : h;
    G_FillA((int)(x + w / 2 - sw / 2) + 2, (int)(y + h / 2 - sh / 2) + 2, (int)sw, (int)sh, 0x000000, alpha / 4);
}
static void PCard(unsigned id, float x, float y, float w, float h, double angle, unsigned alpha, unsigned flash)
{
    Shadow(x, y, w, h, angle, alpha);
    G_Card(id, x, y, w, h, angle, alpha, flash);
}
/* A face-down card turning over: k 0 back .. 1 face */
static void PFlip(unsigned id, float x, float y, float w, float h, float k)
{
    float s = fabsf(cosf(Clamp01(k) * 3.14159f));
    float dw = w * (s < 0.06f ? 0.06f : s);
    PCard(k < 0.5f ? 0 : id, x + (w - dw) / 2, y, dw, h, 0, 255, 0);
}
static void PokeGoal(int t)
{
    static const unsigned prizes[6] = {TCG_CARD_POTION, TCG_CARD_BILL, TCG_CARD_FIRE_ENERGY, TCG_CARD_SQUIRTLE,
                                       TCG_CARD_WATER_ENERGY, TCG_CARD_BULBASAUR};
    int i;
    Zone(12, 26, 46, 66, "PRIZES");
    Zone(150, 32, 74, 48, NULL);
    G_TextAlign(187, 21, TA_CENTER, 0xe8f0e8, G_NO_SHADOW, "YOUR HAND", TF_NARROW);
    for (i = 0; i < 6; i++)
    {
        int start = 20 + i * 34;
        float k = InOut(Seg(t, start, start + 22));
        float x0 = 15 + (i % 2) * 21, y0 = 29 + (i / 2) * 21, x1 = 154 + i * 10, y1 = 36;
        float flip = Seg(t, start + 8, start + 18);
        float x = Lerp(x0, x1, k), y = Lerp(y0, y1, k) + Land(Seg(t, start + 22, start + 30)) * 0.5f;
        if (k <= 0)
            PCard(0, x0, y0, 19, 20, 0, 255, 0);
        else
            PFlip(prizes[i], x, y, Lerp(19, 28, k), Lerp(20, 40, k), flip);
    }
    for (i = 0; i < 6; i++)
        if (t >= 20 + i * 34 && t < 20 + i * 34 + 14)
            Tag(120, 98, "KNOCK OUT: TAKE A PRIZE", 0xc84040, Show(t, 20 + i * 34, 20 + i * 34 + 14));
    if (t > 230)
    {
        float k = Out(Seg(t, 230, 246));
        G_Banner(CX, 92, "ALL PRIZES TAKEN: YOU WIN!", 0x283848, k, (unsigned)(255 * k));
    }
}
static void PokeSetup(int t)
{
    static const unsigned hand[7] = {TCG_CARD_CHARMANDER, TCG_CARD_FIRE_ENERGY, TCG_CARD_BULBASAUR, TCG_CARD_BILL,
                                     TCG_CARD_SQUIRTLE, TCG_CARD_POTION, TCG_CARD_FIRE_ENERGY};
    int i;
    /* the table: Prizes on the left, the Active in the middle, the Bench below it, the deck on the right */
    Zone(8, 24, 36, 66, "PRIZES");
    Zone(103, 23, 32, 44, NULL);
    G_Text(138, 38, 0xe8f0e8, G_NO_SHADOW, "ACTIVE", 0, TF_NARROW);
    for (i = 0; i < 5; i++)
        Zone(64 + i * 22, 76, 20, 26, NULL);
    G_Text(176, 82, 0xe8f0e8, G_NO_SHADOW, "BENCH", 0, TF_NARROW);
    PCard(0, 204, 26, 22, 31, 0, 255, 0);
    G_TextAlign(215, 58, TA_CENTER, 0xe8f0e8, G_NO_SHADOW, "DECK", TF_NARROW);
    /* draw 7: a row across the middle; the Basics go into play face down, the rest stay in the hand */
    for (i = 0; i < 7; i++)
    {
        int s = 6 + i * 6;
        float k = Out(Seg(t, s, s + 14));
        float hx = 50 + i * 21, hy = 44;
        float x, y, w = 19, h = 27;
        int faceUp = 1, slot = i == 0 ? -1 : i == 2 ? 0 : i == 4 ? 1 : -2;
        if (t < s)
            continue;
        x = Lerp(204, hx, k), y = Lerp(26, hy, k);
        if (slot != -2 && t > 64)
        {
            int start = 64 + (slot + 1) * 10;
            float m = InOut(Seg(t, start, start + 18));
            float tx = slot < 0 ? 104 : 64 + slot * 22, ty = slot < 0 ? 24 : 76, tw = slot < 0 ? 30 : 20, th = slot < 0 ? 42 : 26;
            x = Lerp(hx, tx, m), y = Lerp(hy, ty, m), w = Lerp(19, tw, m), h = Lerp(27, th, m);
            faceUp = 0;
            if (t >= 166)
            {
                PFlip(hand[i], x, y, w, h, Seg(t, 166, 184));
                continue;
            }
        }
        else if (t > 100)
        {
            unsigned a = (unsigned)(255 * (1 - Seg(t, 100, 116)));
            if (a)
                PCard(hand[i], x, y, w, h, 0, a, 0);
            continue;
        }
        PCard(faceUp ? hand[i] : 0, x, y, w, h, 0, 255, 0);
    }
    Tag(122, 38, "DRAW 7", 0x506070, Show(t, 8, 60));
    Tag(64, 34, "BASICS: FACE DOWN", 0x3870c8, Show(t, 66, 120));
    /* six Prizes from the deck */
    for (i = 0; i < 6; i++)
    {
        int s = 120 + i * 6;
        float k = Out(Seg(t, s, s + 12));
        if (t >= s)
            PCard(0, Lerp(204, 11 + (i % 2) * 16, k), Lerp(26, 27 + (i / 2) * 20, k), 14, 19, 0, 255, 0);
    }
    Tag(26, 98 - 4, "6 PRIZES", 0x806020, Show(t, 134, 166));
    Tag(120, 98 - 4, "ALL TURN FACE UP", 0x3870c8, Show(t, 170, 200));
    /* who goes first */
    if (t > 200)
    {
        float spin = Seg(t, 202, 246) * 6;
        G_RoundFill(150, 22, 52, 36, 0x000000, 70);
        G_Coin(176, 36, spin, 1, 255);
        Tag(176, 52, t > 248 ? "HEADS: YOU START" : "WHO STARTS?", t > 248 ? 0x3870c8 : 0x606060, 255);
    }
}
static void PokeTurn(int t)
{
    static const char *const steps[] = {"DRAW A CARD", "BENCH BASICS (5 MAX)", "EVOLVE", "ATTACH 1 ENERGY",
                                        "PLAY TRAINERS", "USE POKeMON POWERS", "RETREAT", "ATTACK: ENDS TURN"};
    int i, lit = t / 40 % 8;
    Zone(30, 26, 32, 44, "ACTIVE");
    PCard(TCG_CARD_CHARMANDER, 31, 27, PW, PH, 0, 255, 0);
    for (i = 0; i < 3; i++)
        Zone(14 + i * 22, 82, 20, 22, "");
    PCard(TCG_CARD_BULBASAUR, 15, 83, 18, 20, 0, 255, 0);
    G_RoundFill(96, 24, 132, 80, 0x000000, 70);
    for (i = 0; i < 8; i++)
    {
        unsigned on = i == lit, done = i < lit;
        int y = 26 + i * 10;
        if (on)
            G_RoundFill(98, y, 128, 10, Accent(), 210);
        G_Text(112, y - 1, on ? G_WHITE : done ? 0x90a898 : 0xd8e0d8, G_NO_SHADOW, steps[i], 0, TF_NARROW);
        G_FillA(102, y + 3, 5, 5, done ? 0x70d070 : on ? 0xffffff : 0x607068, 255);
    }
    if (lit == 0)
    {
        float k = Out(Seg(t % 40, 4, 24));
        PCard(TCG_CARD_FIRE_ENERGY, Lerp(70, 70, k), Lerp(20, 50, k), 20, 28, 0, (unsigned)(255 * k), 0);
    }
    if (lit == 3)
        G_EnergyOrb(56, 64, TCG_FIRE, 1);
    if (lit == 7)
        Burst(46, 48, Seg(t % 40, 6, 30), 0xffe080);
}
static void PokeEnergy(int t)
{
    const struct TcgCard *c = Tcg_Card(TCG_CARD_CHARMANDER);
    int i, attached = t < 40 ? 0 : t < 130 ? 1 : 2;
    PCard(TCG_CARD_CHARMANDER, 28, 30, 40, 56, 0, 255, 0);
    /* the Energy cards slide in, one a turn */
    for (i = 0; i < 2; i++)
    {
        int s = i ? 100 : 12;
        float k = InOut(Seg(t, s, s + 26));
        if (t >= s && k < 1)
            PCard(TCG_CARD_FIRE_ENERGY, Lerp(120, 40, k), Lerp(30, 50, k), 24, 34, 0, 255, 0);
        else if (k >= 1)
            G_EnergyOrb(32 + i * 9, 76, TCG_FIRE, 1);
        Ring(48, 58, Seg(t, s + 26, s + 40), 0xff9040);
    }
    Tag(48, 26, attached == 1 && t < 96 ? "1 ENERGY A TURN" : attached == 2 ? "NEXT TURN: ONE MORE" : "",
        0x806020, attached ? Show(t, 40, attached == 1 ? 94 : 170) : 0);
    /* the attacks and what they need */
    G_RoundFill(96, 34, 130, 44, 0x000000, 80);
    for (i = 0; i < 2 && c; i++)
    {
        const struct TcgAttack *a = &c->attacks[i];
        unsigned need = 0, k, ready;
        char dmg[8];
        int x = 102, y = 38 + i * 20;
        for (k = 0; k < TCG_TYPES; k++)
            need += a->cost[k];
        ready = (unsigned)attached >= need;
        if (ready)
            G_RoundFill(98, y - 2, 126, 18, i == 1 && t > 140 ? Accent() : 0x406050, 200);
        for (k = 0; k < TCG_TYPES; k++)
        {
            unsigned n;
            for (n = 0; n < a->cost[k]; n++, x += 9)
                G_EnergyOrb(x, y + 4, k, 1);
        }
        G_Text(x + 4, y, G_WHITE, G_NO_SHADOW, a->name, 0, TF_SMALL);
        snprintf(dmg, sizeof(dmg), "%u", a->damage);
        G_TextAlign(220, y, TA_RIGHT, ready ? 0xfff0a0 : 0xa0a8a0, G_NO_SHADOW, dmg, TF_SMALL);
    }
    if (t > 140)
        Tag(160, 90, "ENOUGH ENERGY: ATTACK!", 0xc84040, Show(t, 140, 220));
}
static void PokeDamage(int t)
{
    float lunge = sinf(Seg(t, 30, 46) * 3.14159f) * 26 + sinf(Seg(t, 150, 166) * 3.14159f) * 26;
    int hp = t < 44 ? 50 : t < 164 ? 10 : 0;
    unsigned dead = t >= 176;
    PCard(TCG_CARD_SQUIRTLE, 40 + lunge, 32, 40, 56, 0, 255, 0);
    G_TextAlign(60, 90, TA_CENTER, 0xe8f0e8, G_NO_SHADOW, "YOURS", TF_NARROW);
    if (!dead)
    {
        float shake = (t >= 44 && t < 56) || (t >= 164 && t < 176) ? sinf(t * 2.0f) * 2 : 0;
        PCard(TCG_CARD_CHARMANDER, 160 + shake, 32, 40, 56, 0, 255, (t >= 44 && t < 52) || (t >= 164 && t < 172) ? 200 : 0);
    }
    Shatter(160, 32, 40, 56, Seg(t, 176, 206), 0xf89850);
    G_HpPlate(162, 24, hp, 50, 0, dead ? (unsigned)(255 * (1 - Seg(t, 176, 190))) : 255);
    G_TextAlign(180, 90, TA_CENTER, 0xe8f0e8, G_NO_SHADOW, "THEIRS", TF_NARROW);
    if (t >= 44 && t < 130)
    {
        Tag(120, 40, "ATTACK 20", 0x3870c8, Show(t, 44, 130));
        Tag(120, 54, "WEAKNESS x2: 40", 0xc84040, Show(t, 60, 130));
        Tag(120, 68, "50 - 40: 10 HP LEFT", 0x506070, Show(t, 76, 130));
    }
    Burst(180, 60, Seg(t, 44, 64), 0x80c8ff);
    Burst(180, 60, Seg(t, 164, 184), 0x80c8ff);
    if (t >= 182)
    {
        G_Banner(CX, 46, "KNOCKED OUT!", 0x602020, Out(Seg(t, 182, 196)), Show(t, 182, 300));
        if (t > 206)
        {
            float k = InOut(Seg(t, 206, 230));
            PFlip(TCG_CARD_POTION, Lerp(200, 100, k), Lerp(70, 64, k), 20, 28, Seg(t, 214, 228));
            Tag(120, 98, "YOU TAKE A PRIZE CARD", 0x806020, Show(t, 214, 300));
        }
    }
}
static void PokeEvolve(int t)
{
    float k = Out(Seg(t, 40, 70));
    PCard(TCG_CARD_CHARMANDER, 60, 30, 40, 56, 0, t < 70 ? 255 : 0, 0);
    G_EnergyOrb(64, 80, TCG_FIRE, 1);
    if (t >= 30)
        PCard(TCG_CARD_CHARMELEON, 60, Lerp(-60, 30, k) + Land(Seg(t, 70, 80)), 40, 56, 0, 255,
              t >= 70 && t < 80 ? 220 : 0);
    if (t >= 70)
        G_EnergyOrb(64, 80, TCG_FIRE, 1);
    Ring(80, 58, Seg(t, 70, 96), 0xfff0a0);
    Tag(80, 24, t < 70 ? "BASIC" : "STAGE 1", t < 70 ? 0x506070 : 0x3870c8, 255);
    G_RoundFill(124, 30, 102, 58, 0x000000, 70);
    G_Text(130, 32, G_WHITE, G_NO_SHADOW, "Basic", 0, TF_SMALL);
    Arrow(140, 47, 140, 55, 0xfff0a0, 255);
    G_Text(130, 56, G_WHITE, G_NO_SHADOW, "Stage 1", 0, TF_SMALL);
    Arrow(140, 71, 140, 79, 0xfff0a0, 160);
    G_Text(150, 71, 0xc0c8c0, G_NO_SHADOW, "Stage 2", 0, TF_SMALL);
    if (t > 100)
        Tag(80, 96, "KEEPS ENERGY AND DAMAGE", 0x806020, Show(t, 100, 220));
}
static void PokeRetreat(int t)
{
    float off = Out(Seg(t, 30, 50)), swap = InOut(Seg(t, 70, 110));
    float ax = Lerp(40, 150, swap), ay = Lerp(30, 40, swap) - sinf(swap * 3.14159f) * 14;
    float bx = Lerp(150, 40, swap), by = Lerp(40, 30, swap) + sinf(swap * 3.14159f) * 14;
    Zone(38, 28, 44, 60, "ACTIVE");
    Zone(146, 36, 36, 48, "BENCH");
    PCard(TCG_CARD_BULBASAUR, bx, by, Lerp(32, 40, swap), Lerp(44, 56, swap), 0, 255, 0);
    PCard(TCG_CARD_CHARMANDER, ax, ay, Lerp(40, 32, swap), Lerp(56, 44, swap), 0, 255, 0);
    if (off < 1)
        G_EnergyOrb((int)Lerp(46, 110, off), (int)Lerp(80, 96, off), TCG_FIRE, 1);
    Tag(110, 30, "RETREAT COST: 1 ENERGY", 0x806020, Show(t, 10, 66));
    Tag(110, 30, "SWAP WITH THE BENCH", 0x3870c8, Show(t, 70, 150));
    if (t > 160)
        Tag(110, 98, "KNOCKED OUT? PROMOTE ONE FROM THE BENCH", 0x506070, Show(t, 160, 290));
}
static void PokeConditions(int t)
{
    static const struct
    {
        const char *name, *line;
        int condition, poison;
        double angle;
    } c[4] = {
        {"ASLEEP", "Can't attack or retreat. A coin each turn to wake up.", 1, 0, -90},
        {"PARALYZED", "Can't attack or retreat for one turn.", 2, 0, 90},
        {"CONFUSED", "A coin to attack: tails, it hurts itself.", 3, 0, 180},
        {"POISONED", "10 damage between every turn.", 0, 1, 0},
    };
    int i = t / 90 % 4, local = t % 90;
    float k = InOut(Seg(local, 6, 24));
    PCard(TCG_CARD_BULBASAUR, 42, 32, 40, 56, c[i].angle * k, 255, 0);
    if (c[i].poison)
        G_Circle(62, 60, 6 + sinf(local * 0.2f) * 1.5f, 0x9040c0, 120);
    G_StatusBadges(40, 24, c[i].condition, c[i].poison);
    G_RoundFill(104, 32, 122, 50, 0x000000, 80);
    G_Text(110, 34, 0xfff0a0, G_NO_SHADOW, c[i].name, 0, TF_NORMAL);
    G_Text(110, 52, G_WHITE, G_NO_SHADOW, c[i].line, 114, TF_SMALL);
    G_TextAlign(CX, 92, TA_CENTER, 0xc8d8c8, G_NO_SHADOW, "Switching to the Bench clears them all.", TF_NARROW);
}
static void PokeControls(int t)
{
    static const char *const lines[] = {"{DPAD} MOVE AROUND", "{A} CHOOSE / PLAY", "{B} TURN MENU: END TURN",
                                        "{A} ON DECK: CONCEDE", "{INSPECT} LOOK AT A CARD"};
    int spot = t / 50 % 3;
    float x = spot == 0 ? 18 : spot == 1 ? 52 : 86;
    PCard(TCG_CARD_CHARMANDER, 18, 30, PW, PH, 0, 255, 0);
    PCard(TCG_CARD_FIRE_ENERGY, 52, 30, PW, PH, 0, 255, 0);
    PCard(0, 86, 30, PW, PH, 0, 255, 0);
    G_Focus((int)x, 30, PW, PH, G_FOCUS_TARGET, G.frame);
    G_TextAlign(66, 80, TA_CENTER, 0xe8f0e8, G_NO_SHADOW, spot == 2 ? "YOUR DECK" : spot ? "YOUR HAND" : "ACTIVE", TF_NARROW);
    Controls(124, 30, lines, 5, t / 50 % 5);
}
static const struct Page sPokemon[] = {
    {"THE GOAL",
     "Win by taking all your Prize cards, by Knocking Out your opponent's last Pokemon in play, or when they can't "
     "draw at the start of their turn.",
     PokeGoal, 300},
    {"SETTING UP",
     "Decks are 60 cards. Draw 7, put a Basic Pokemon face down as your Active and up to 5 on your Bench, then "
     "set Prizes. A coin picks who starts.",
     PokeSetup, 300},
    {"YOUR TURN",
     "Draw a card, then do any of these in any order. Attaching Energy is once a turn. Attacking ends your turn.",
     PokeTurn, 320},
    {"ENERGY AND ATTACKS",
     "An attack needs the Energy shown beside it attached to your Active Pokemon. Attach one Energy card each turn.",
     PokeEnergy, 240},
    {"DAMAGE",
     "Damage stays on the Pokemon. Weakness doubles it, Resistance takes 30 off. When damage reaches its HP, it is "
     "Knocked Out and you take a Prize card.",
     PokeDamage, 300},
    {"EVOLUTION",
     "Play a Stage 1 card on the Basic it evolves from (not on the turn it came into play), and a Stage 2 on that "
     "Stage 1.",
     PokeEvolve, 230},
    {"RETREAT AND THE BENCH",
     "To retreat, discard Energy equal to the Retreat cost and swap your Active with a Benched Pokemon. Once a turn.",
     PokeRetreat, 300},
    {"SPECIAL CONDITIONS",
     "Some attacks leave a Pokemon Asleep, Paralyzed, Confused or Poisoned. The badge on the card says which.",
     PokeConditions, 360},
    {"CONTROLS", "Choose a card to see what it can do. DUEL SPEED in the CARD CASE sets how quickly the cards move.",
     PokeControls, 250},
};

/* ------------------------------------------------------------------ */
/* YU-GI-OH!                                                            */
/* ------------------------------------------------------------------ */
#define YW 30
#define YH 43
static void YCard(unsigned id, float x, float y, float w, float h, double angle, unsigned alpha, unsigned flash)
{
    Shadow(x, y, w, h, angle, alpha);
    Ygo_DrawCard(id, x, y, w, h, angle, alpha, flash);
}
static void YFlip(unsigned id, float x, float y, float w, float h, float k)
{
    float s = fabsf(cosf(Clamp01(k) * 3.14159f));
    float dw = w * (s < 0.06f ? 0.06f : s);
    YCard(k < 0.5f ? 0 : id, x + (w - dw) / 2, y, dw, h, 0, 255, 0);
}
static void Lp(int x, int y, const char *who, int lp, unsigned color)
{
    char n[8];
    snprintf(n, sizeof(n), "%d", lp < 0 ? 0 : lp);
    G_RoundFill(x, y, 70, 30, 0x000000, 110);
    G_Text(x + 4, y, color, G_NO_SHADOW, who, 0, TF_NARROW);
    G_DigitsOutlined(x + 35 - G_DigitsWidth(n, 1, 2) / 2, y + 12, n, 1, 2, lp > 0 ? 0xfff0a0 : 0xff5050, 0x302010, 255);
}
static void YgoGoal(int t)
{
    int hits = t < 40 ? 0 : t < 100 ? 1 : t < 160 ? 2 : 3, lp = 8000 - hits * 3000;
    float lunge = 0;
    int h;
    for (h = 0; h < 3; h++)
        lunge += sinf(Seg(t, 30 + h * 60, 46 + h * 60) * 3.14159f) * 40;
    Lp(10, 24, "YOU", 8000, 0x80c8ff);
    Lp(156, 24, "RIVAL", lp, 0xff9090);
    YCard(YGO_CARD_BLUE_EYES_WHITE_DRAGON, 30 + lunge, 58, 30, 43, 0, 255, 0);
    for (h = 0; h < 3; h++)
        Burst(170, 70, Seg(t, 42 + h * 60, 62 + h * 60), 0xffffff);
    if (t >= 40 && t < 180)
        Tag(190, 64, "-3000", 0xc84040, Show(t, 40 + (hits - 1) * 60, 40 + (hits - 1) * 60 + 40));
    if (t > 170)
        G_Banner(CX, 92, "0 LIFE POINTS: YOU WIN!", 0x283848, Out(Seg(t, 170, 186)), Show(t, 170, 280));
}
static void YgoSetup(int t)
{
    static const unsigned hand[5] = {YGO_CARD_CELTIC_GUARDIAN, YGO_CARD_POT_OF_GREED, YGO_CARD_TRAP_HOLE,
                                     YGO_CARD_SUMMONED_SKULL, YGO_CARD_MYSTICAL_ELF};
    int i;
    YCard(0, 196, 26, 26, 37, 0, 255, 0);
    G_TextAlign(209, 64, TA_CENTER, 0xe0e8ff, G_NO_SHADOW, "DECK 40-60", TF_NARROW);
    Zone(196, 78, 26, 18, NULL);
    G_TextAlign(209, 80, TA_CENTER, 0xe0e8ff, G_NO_SHADOW, "EXTRA", TF_NARROW);
    for (i = 0; i < 5; i++)
    {
        int s = 10 + i * 10;
        float k = Out(Seg(t, s, s + 18));
        if (t < s)
            continue;
        YFlip(hand[i], Lerp(196, 30 + i * 30, k), Lerp(28, 46, k) + Land(Seg(t, s + 18, s + 26)), YW, YH, Seg(t, s + 10, s + 18));
    }
    Tag(90, 34, "DRAW 5", 0x5848c8, Show(t, 12, 110));
    Tag(96, 98, "FIRST TURN: NO DRAW, NO ATTACK", 0xc84040, Show(t, 110, 260));
    Tag(209, 102 - 4, "FUSIONS", 0x806020, Show(t, 60, 260));
}
static void YgoPhases(int t)
{
    static const char *const names[] = {"DRAW", "STANDBY", "MAIN 1", "BATTLE", "MAIN 2", "END"};
    static const char *const lines[] = {"Draw a card.", "Some effects happen here.", "Summon, Set, play Spells.",
                                        "Attack with your monsters.", "Summon and Set again.", "Your turn ends."};
    int i, lit = t / 45 % 6;
    for (i = 0; i < 6; i++)
    {
        int x = 10 + i * 37, on = i == lit;
        G_RoundFill(x, 40 + (on ? -2 : 0), 34, 16, on ? Accent() : 0x404870, 255);
        G_RoundFrame(x, 40 + (on ? -2 : 0), 34, 16, on ? 0xe0d8ff : 0x606890, 255);
        G_TextAlign(x + 17, 41 + (on ? -2 : 0), TA_CENTER, G_WHITE, G_NO_SHADOW, names[i], TF_NARROW);
        if (i < 5)
            G_Arrow(x + 35, 46, 3, 0x9098c0);
    }
    Say(CX, 66, lines[lit], 255);
    if (lit == 0)
        YCard(0, 110, 80, 14, 20, 0, 255, 0);
    if (lit == 3)
        Arrow(100, 96, 140, 82, 0xff8080, 255);
}
static void YgoSummon(int t)
{
    /* a Level 4 straight from the hand; then two monsters Tributed for a Level 8 */
    if (t < 110)
    {
        float k = InOut(Seg(t, 10, 34));
        Zone(100, 30, 36, 50, "MONSTER ZONE");
        YCard(YGO_CARD_CELTIC_GUARDIAN, Lerp(100, 102, k), Lerp(110, 32, k) + Land(Seg(t, 34, 42)), 32, 46, 0, 255,
              t >= 34 && t < 40 ? 200 : 0);
        Ring(118, 55, Seg(t, 34, 56), 0xffffff);
        Tag(176, 44, "LEVEL 1-4: SUMMON FREELY", 0x5848c8, Show(t, 40, 108));
        Tag(176, 60, "ONCE A TURN", 0x806020, Show(t, 56, 108));
    }
    else
    {
        int u = t - 110;
        float gone = Seg(u, 20, 44), k = Out(Seg(u, 50, 74));
        Zone(60, 30, 36, 50, "");
        Zone(140, 30, 36, 50, "");
        if (gone < 1)
        {
            YCard(YGO_CARD_CELTIC_GUARDIAN, 62, 32 - gone * 10, 32, 46, 0, (unsigned)(255 * (1 - gone)), (unsigned)(255 * gone));
            YCard(YGO_CARD_MYSTICAL_ELF, 142, 32 - gone * 10, 32, 46, 0, (unsigned)(255 * (1 - gone)), (unsigned)(255 * gone));
        }
        Tag(118, 26, "TRIBUTE 2", 0xc84040, Show(u, 4, 50));
        if (u >= 50)
        {
            YCard(YGO_CARD_BLUE_EYES_WHITE_DRAGON, 98, Lerp(-50, 30, k), 40, 58, 0, 255, u < 80 ? 180 : 0);
            Ring(118, 58, Seg(u, 74, 100), 0xc0e8ff);
            Tag(118, 98, "LEVEL 5-6: 1 TRIBUTE   LEVEL 7+: 2", 0x5848c8, Show(u, 80, 200));
        }
    }
}
static void YgoPositions(int t)
{
    const struct YgoCard *c = Ygo_Card(YGO_CARD_GIANT_SOLDIER_OF_STONE);
    float rot = InOut(Seg(t, 70, 90)), flip = Seg(t, 150, 166);
    char line[32];
    int mode = t < 80 ? 0 : t < 158 ? 1 : 2;
    if (mode < 2)
        YCard(YGO_CARD_GIANT_SOLDIER_OF_STONE, 46, 32, 36, 52, 90 * rot, 255, 0);
    else
        YCard(0, 46, 32, 36, 52, 90, 255, (unsigned)(200 * (1 - flip)));
    G_RoundFill(110, 36, 116, 44, 0x000000, 80);
    G_Text(116, 38, 0xfff0a0, G_NO_SHADOW, mode == 0 ? "ATTACK POSITION" : mode == 1 ? "DEFENSE POSITION" : "SET", 0, TF_NORMAL);
    if (c)
    {
        if (mode == 0)
            snprintf(line, sizeof(line), "Upright. Uses its ATK %u.", c->atk);
        else if (mode == 1)
            snprintf(line, sizeof(line), "Sideways. Uses its DEF %u.", c->def);
        else
            snprintf(line, sizeof(line), "Face-down Defense.");
        G_Text(116, 56, G_WHITE, G_NO_SHADOW, line, 108, TF_SMALL);
    }
    G_TextAlign(CX, 92, TA_CENTER, 0xd0d8f0, G_NO_SHADOW, "Change it once a turn, not on the turn it arrived.", TF_NARROW);
}
static void YgoSpells(int t)
{
    /* a Spell from the hand; a Trap Set first, then sprung on their Summon */
    if (t < 120)
    {
        float k = InOut(Seg(t, 10, 30));
        Zone(100, 42, 32, 45, "SPELL & TRAP ZONE");
        YCard(YGO_CARD_POT_OF_GREED, Lerp(100, 101, k), Lerp(110, 43, k), 30, 43, 0, 255, t >= 30 && t < 40 ? 220 : 0);
        Tag(116, 30, "SPELL: PLAY IT FROM YOUR HAND", 0x309060, Show(t, 30, 118));
        if (t > 50)
        {
            int i;
            for (i = 0; i < 2; i++)
            {
                float m = Out(Seg(t, 56 + i * 12, 74 + i * 12));
                if (t >= 56 + i * 12)
                    YCard(0, Lerp(200, 170 + i * 20, m), Lerp(40, 70, m), 18, 26, 0, 255, 0);
            }
            Tag(190, 62, "DRAW 2", 0x309060, Show(t, 60, 118));
        }
    }
    else
    {
        int u = t - 120;
        float set = InOut(Seg(u, 6, 24)), mon = Out(Seg(u, 50, 66)), spring = Seg(u, 76, 90), gone = Seg(u, 96, 120);
        Zone(60, 56, 32, 44, "YOURS");
        Zone(150, 24, 32, 44, "THEIRS");
        if (u < 76)
            YCard(0, Lerp(60, 61, set), Lerp(110, 57, set), 30, 43, 0, 255, 0);
        else
            YFlip(YGO_CARD_TRAP_HOLE, 61, 57, 30, 43, spring);
        if (u >= 50 && gone < 1)
            YCard(YGO_CARD_SUMMONED_SKULL, 151, Lerp(-40, 25, mon), 30, 43, 0, (unsigned)(255 * (1 - gone)), 0);
        Shatter(151, 25, 30, 43, gone, 0xc090ff);
        Tag(150, 84, "TRAP: SET IT FIRST...", 0xa04070, Show(u, 10, 74));
        Tag(150, 84, "...SPRING IT ON THEIR MOVE", 0xa04070, Show(u, 76, 150));
    }
}
static void YgoChains(int t)
{
    static const unsigned link[3] = {YGO_CARD_SUMMONED_SKULL, YGO_CARD_TRAP_HOLE, YGO_CARD_MIRROR_FORCE};
    static const char *const what[3] = {"SUMMON", "ANSWER", "ANSWER 2"};
    int i;
    for (i = 0; i < 3; i++)
    {
        int in = 10 + i * 30, out = 140 + (2 - i) * 26;
        float k = Out(Seg(t, in, in + 14)), gone = Seg(t, out, out + 14);
        char n[4];
        int x = 40 + i * 54, y = 30;
        if (t < in || gone >= 1)
            continue;
        YCard(link[i], x, Lerp(y + 30, y, k) - gone * 12, 30, 43, 0, (unsigned)(255 * (1 - gone)), t >= out ? 220 : 0);
        snprintf(n, sizeof(n), "%d", i + 1);
        G_RoundFill(x + 22, y - 4, 14, 14, t >= out ? 0xfff0a0 : Accent(), 255);
        G_TextAlign(x + 29, y - 5, TA_CENTER, t >= out ? 0x403000 : G_WHITE, G_NO_SHADOW, n, TF_SMALL);
        if (t < 130)
            G_TextAlign(x + 15, y + 46, TA_CENTER, 0xe0e8ff, G_NO_SHADOW, what[i], TF_NARROW);
        if (i < 2 && t < 130 && t > in + 20)
            G_Arrow(x + 40, y + 20, 3, 0xffffff);
    }
    if (t >= 130 && t < 230)
        Tag(CX, 100, "IT RESOLVES BACKWARDS: 3, 2, 1", 0xa04070, Show(t, 130, 228));
    else if (t < 130)
        Tag(CX, 100, "EACH ANSWER IS A LINK OF THE CHAIN", 0x5848c8, Show(t, 30, 128));
}
static void YgoBattle(int t)
{
    int scene = t / 110 % 3, u = t % 110;
    float lunge = sinf(Seg(u, 20, 40) * 3.14159f);
    if (scene == 0)
    {
        YCard(YGO_CARD_SUMMONED_SKULL, 40 + lunge * 80, 30, 32, 46, 0, 255, 0);
        if (u < 40)
            YCard(YGO_CARD_CELTIC_GUARDIAN, 160, 30, 32, 46, 0, 255, 0);
        Shatter(160, 30, 32, 46, Seg(u, 40, 70), 0xffd080);
        Tag(56, 84, "ATK 2500", 0x5848c8, 255);
        Tag(176, 84, "ATK 1400", 0x806060, u < 70 ? 255 : 0);
        Tag(CX, 100, "LOWER ATK IS DESTROYED: THEY LOSE 1100", 0xc84040, Show(u, 44, 108));
    }
    else if (scene == 1)
    {
        float bounce = sinf(Seg(u, 20, 36) * 3.14159f) * 60 - sinf(Seg(u, 36, 46) * 3.14159f) * 8;
        YCard(YGO_CARD_CELTIC_GUARDIAN, 40 + bounce, 30, 32, 46, 0, 255, 0);
        YCard(YGO_CARD_GIANT_SOLDIER_OF_STONE, 156, 36, 32, 46, 90, 255, u >= 34 && u < 40 ? 200 : 0);
        Tag(56, 84, "ATK 1400", 0x5848c8, 255);
        Tag(176, 84, "DEF 2000", 0x606070, 255);
        Tag(CX, 100, "HIGHER DEF: NOTHING BREAKS, YOU LOSE 600", 0xc84040, Show(u, 44, 108));
    }
    else
    {
        YCard(YGO_CARD_SUMMONED_SKULL, 40 + lunge * 100, 30, 32, 46, 0, 255, 0);
        Zone(156, 30, 32, 46, "");
        Burst(196, 50, Seg(u, 36, 56), 0xffffff);
        Tag(56, 84, "ATK 2500", 0x5848c8, 255);
        Tag(CX, 100, "NO MONSTERS: A DIRECT ATTACK, -2500", 0xc84040, Show(u, 40, 108));
    }
}
static void YgoFusion(int t)
{
    float k = InOut(Seg(t, 50, 80)), born = Out(Seg(t, 84, 104));
    float spin = Seg(t, 50, 80) * 6.28f;
    YCard(YGO_CARD_POLYMERIZATION, 100, 26, 30, 43, 0, t < 50 ? 255 : (unsigned)(255 * (1 - Seg(t, 50, 60))), t > 30 && t < 50 ? 160 : 0);
    if (t < 84)
    {
        YCard(YGO_CARD_GAIA_THE_FIERCE_KNIGHT, Lerp(30, 100, k) + sinf(spin) * 12 * k, Lerp(50, 50, k), 30, 43, 0,
              (unsigned)(255 * (1 - k * 0.6f)), (unsigned)(200 * k));
        YCard(YGO_CARD_CURSE_OF_DRAGON, Lerp(170, 100, k) - sinf(spin) * 12 * k, Lerp(50, 50, k), 30, 43, 0,
              (unsigned)(255 * (1 - k * 0.6f)), (unsigned)(200 * k));
        if (t < 50)
            G_TextAlign(115, 96, TA_CENTER, 0xe0e8ff, G_NO_SHADOW, "+", TF_NORMAL);
    }
    Ring(115, 70, Seg(t, 80, 110), 0xffe0ff);
    if (t >= 84)
    {
        YCard(YGO_CARD_GAIA_THE_DRAGON_CHAMPION, 96, Lerp(70, 34, born), Lerp(10, 38, born), Lerp(14, 55, born), 0, 255,
              t < 100 ? 200 : 0);
        Tag(172, 52, "FROM THE EXTRA DECK", 0x806020, Show(t, 100, 220));
    }
    Tag(48, 30, "THE LISTED MATERIALS", 0x5848c8, Show(t, 6, 50));
}
static void YgoControls(int t)
{
    static const char *const lines[] = {"{DPAD} MOVE", "{A} CHOOSE / ACT", "{B} BACK / TURN MENU", "{L} HOLD: STOP TO CHAIN",
                                        "{A} ON DECK: CONCEDE", "{INSPECT} LOOK AT A CARD"};
    int spot = t / 50 % 3;
    YCard(YGO_CARD_DARK_MAGICIAN, 18, 30, YW, YH, 0, 255, 0);
    YCard(YGO_CARD_POT_OF_GREED, 52, 30, YW, YH, 0, 255, 0);
    YCard(0, 86, 30, YW, YH, 0, 255, 0);
    G_Focus(18 + spot * 34, 30, YW, YH, G_FOCUS_TARGET, G.frame);
    Controls(124, 26, lines, 6, t / 50 % 6);
}
static const struct Page sYugioh[] = {
    {"THE GOAL", "Both players start with 8000 Life Points. Bring your opponent's to 0, or win when they have to draw "
                 "from an empty Deck.",
     YgoGoal, 280},
    {"SETTING UP", "A Deck is 40 to 60 cards, plus up to 15 Fusion Monsters in the Extra Deck. Both draw 5.", YgoSetup, 270},
    {"THE TURN", "Every turn runs through six phases in order. You can skip the Battle Phase.", YgoPhases, 270},
    {"SUMMONING", "Once a turn, Normal Summon or Set a monster. Higher Levels need monsters you control sent away as "
                  "Tributes first.",
     YgoSummon, 330},
    {"POSITIONS", "A monster attacks in Attack Position and guards in Defense Position. Set monsters are face-down "
                  "until attacked or flipped.",
     YgoPositions, 240},
    {"SPELLS AND TRAPS", "Play Spells in your Main Phase. Traps are Set face-down first and can be used from the next "
                         "turn on, even during your opponent's.",
     YgoSpells, 270},
    {"CHAINS", "Cards can answer each other. The answers make a Chain, and the last one added happens first. Hold L to "
               "stop at every chance to answer.",
     YgoChains, 250},
    {"BATTLE", "An attack compares ATK with the target's ATK or DEF. The weaker monster is destroyed and its owner "
               "takes the difference (none in Defense).",
     YgoBattle, 330},
    {"FUSION", "Polymerization sends the materials a Fusion Monster lists to the Graveyard and Summons it from your "
               "Extra Deck.",
     YgoFusion, 240},
    {"CONTROLS", "The duel asks you whenever there's a choice to make. Quiet chances to answer are passed unless L is "
                 "held.",
     YgoControls, 300},
};

/* ------------------------------------------------------------------ */
/* TRIPLE TRIAD                                                         */
/* ------------------------------------------------------------------ */
#define TW TT_BOARD_W
#define TH TT_BOARD_H
static void TCard(unsigned id, float x, float y, float w, float h, unsigned seat, float flip, unsigned glow)
{
    struct TtFace f;
    memset(&f, 0, sizeof(f));
    f.seat = (uint8_t)seat;
    f.glow = (uint8_t)glow;
    G_FillA((int)x + 2, (int)y + 2, (int)w, (int)h, 0x000000, 60);
    Tt_DrawCard(id, x, y, w, h, flip, 1, 255, &f);
}
/* A card turning to the other color: k 0..1 */
static void TTurn(unsigned id, float x, float y, float w, float h, unsigned from, unsigned to, float k)
{
    float s = cosf(Clamp01(k) * 3.14159f);
    TCard(id, x, y, w, h, k < 0.5f ? from : to, fabsf(s) < 0.06f ? 0.06f : fabsf(s), k > 0 && k < 1 ? 160 : 0);
}
/* Cards that fit the examples, found once in the catalog (the low-star ones first) */
static struct
{
    int ready;
    unsigned basicA, basicB, sameP, sameU, sameL, plusP, plusU, plusL, comboC, any[10];
} T;
static int Low(unsigned id) { const struct TtCard *c = Tt_Card(id); return c && c->stars <= 2; }
static void FindCards(void)
{
    unsigned a, b, n = 0;
    if (T.ready)
        return;
    memset(&T, 0, sizeof(T));
    T.ready = 1;
    for (a = 1; a <= TT_CARD_MAX && n < 10; a++)
        if (Low(a))
            T.any[n++] = a;
    /* a plain capture: the placed card's right beats the neighbor's left by a few */
    for (a = 1; a <= TT_CARD_MAX && !T.basicA; a++)
        if (Low(a) && Tt_Card(a)->side[TT_RIGHT] >= 6)
            for (b = 1; b <= TT_CARD_MAX; b++)
                if (b != a && Low(b) && Tt_Card(b)->side[TT_LEFT] + 3 <= Tt_Card(a)->side[TT_RIGHT] &&
                    Tt_Card(b)->side[TT_LEFT] >= 2)
                {
                    T.basicA = a, T.basicB = b;
                    break;
                }
    /* SAME: the placed card's top and left equal the numbers they touch */
    for (a = 1; a <= TT_CARD_MAX && !T.sameP; a++)
    {
        unsigned u = 0, l = 0;
        if (!Low(a))
            continue;
        for (b = 1; b <= TT_CARD_MAX && (!u || !l); b++)
        {
            if (b == a || !Low(b))
                continue;
            if (!u && Tt_Card(b)->side[TT_DOWN] == Tt_Card(a)->side[TT_UP])
                u = b;
            else if (!l && Tt_Card(b)->side[TT_RIGHT] == Tt_Card(a)->side[TT_LEFT] && b != u)
                l = b;
        }
        if (u && l)
        {
            /* COMBO: once the card above is taken, its left side beats the corner card's right */
            for (b = 1; b <= TT_CARD_MAX; b++)
                if (b != a && b != u && b != l && Low(b) && Tt_Card(b)->side[TT_RIGHT] < Tt_Card(u)->side[TT_LEFT])
                {
                    T.sameP = a, T.sameU = u, T.sameL = l, T.comboC = b;
                    break;
                }
        }
    }
    /* PLUS: the two sums are alike (and the numbers aren't simply equal: that would be SAME) */
    for (a = 1; a <= TT_CARD_MAX && !T.plusP; a++)
    {
        unsigned u;
        if (!Low(a))
            continue;
        for (u = 1; u <= TT_CARD_MAX && !T.plusP; u++)
        {
            int sum;
            if (u == a || !Low(u) || Tt_Card(u)->side[TT_DOWN] == Tt_Card(a)->side[TT_UP])
                continue;
            sum = Tt_Card(u)->side[TT_DOWN] + Tt_Card(a)->side[TT_UP];
            for (b = 1; b <= TT_CARD_MAX; b++)
                if (b != a && b != u && Low(b) && Tt_Card(b)->side[TT_RIGHT] + Tt_Card(a)->side[TT_LEFT] == sum &&
                    Tt_Card(b)->side[TT_RIGHT] != Tt_Card(a)->side[TT_LEFT])
                {
                    T.plusP = a, T.plusU = u, T.plusL = b;
                    break;
                }
        }
    }
}
static void Num(int x, int y, int v, unsigned color)
{
    G_RoundFill(x - 2, y - 2, 11, 13, 0x000000, 150);
    Tt_Number(x, y, v, 1, color, 255);
}
static void TriadGoal(int t)
{
    /* nine moves (some take a card), then the count: blue 6, red 3 + the card left in their hand */
    static const int8_t cell[9] = {4, 0, 8, 2, 6, 1, 3, 5, 7};
    static const int8_t takes[9] = {-1, -1, -1, -1, -1, 4, 0, -1, 4};
    unsigned id[9] = {0}, owner[9], before[9];
    float drop[9], turn[9];
    int i, w = 24, h = 29, bx = 82, by = 22;
    for (i = 0; i < 9; i++)
        owner[i] = before[i] = TT_SEAT_NONE, drop[i] = 1, turn[i] = 1;
    for (i = 0; i < 9; i++)
    {
        int s = 6 + i * 20, c = cell[i];
        unsigned seat = i % 2 ? TT_SEAT_RED : TT_SEAT_BLUE;
        if (t < s)
            break;
        id[c] = T.any[i];
        owner[c] = seat;
        drop[c] = Out(Seg(t, s, s + 10));
        if (takes[i] >= 0 && t >= s + 10)
        {
            int o = takes[i];
            before[o] = owner[o];
            owner[o] = seat;
            turn[o] = Seg(t, s + 10, s + 20);
        }
    }
    for (i = 0; i < 9; i++)
    {
        float x = bx + (i % 3) * (w + 2), y = by + (i / 3) * (h + 1);
        G_FillA((int)x, (int)y, w, h, 0x000000, 40);
        if (!id[i])
            continue;
        if (turn[i] < 1)
            TTurn(id[i], x, y, w, h, before[i], owner[i], turn[i]);
        else
            TCard(id[i], x, Lerp(y - 12, y, drop[i]), w, h, owner[i], 1, 0);
    }
    if (t > 196)
    {
        int blue = 0, red = 1;   /* (the card left in the second player's hand counts for them) */
        char line[32];
        for (i = 0; i < 9; i++)
            blue += owner[i] == TT_SEAT_BLUE, red += owner[i] == TT_SEAT_RED;
        snprintf(line, sizeof(line), "BLUE %d - RED %d", blue, red);
        G_Banner(CX, 94, line, 0x283848, Out(Seg(t, 196, 210)), Show(t, 196, 300));
    }
    Tag(44, 40, "YOU", Tt_SeatColor(TT_SEAT_BLUE, 2), 255);
    Tag(196, 40, "THEM", Tt_SeatColor(TT_SEAT_RED, 2), 255);
}
static void TriadNumbers(int t)
{
    static const char *const names[4] = {"TOP", "RIGHT", "BOTTOM", "LEFT"};
    const struct TtCard *c = Tt_Card(T.basicA ? T.basicA : T.any[0]);
    int lit = t / 40 % 4, cx = 168, cy = 60;
    static const int dx[4] = {0, 22, 0, -22}, dy[4] = {-20, 0, 20, 0};
    int i;
    TCard(T.basicA ? T.basicA : T.any[0], 30, 26, 56, 69, TT_SEAT_BLUE, 1, 0);
    if (!c)
        return;
    G_RoundFill(cx - 36, cy - 32, 72, 64, 0x000000, 70);
    for (i = 0; i < 4; i++)
    {
        unsigned on = i == lit;
        Num(cx + dx[i] - 3, cy + dy[i] - 4, c->side[i], on ? 0xfff070 : 0xffffff);
        if (on)
        {
            G_Frame(cx + dx[i] - 6, cy + dy[i] - 7, 13, 15, 0xfff070, 255);
            G_TextAlign(cx, cy + 34, TA_CENTER, 0xfff0a0, G_NO_SHADOW, names[i], TF_SMALL);
        }
    }
    G_FillA(cx - 1, cy - 1, 3, 3, 0xffffff, 120);
    Tag(cx, 100 - 2, "A IS 10", Tt_SeatColor(TT_SEAT_BLUE, 2), Show(t, 0, 400));
}
/* Two cards side by side: the placed one slides in, the numbers that touch are compared, the other turns */
static void TriadCapture(int t)
{
    const struct TtCard *a = Tt_Card(T.basicA), *b = Tt_Card(T.basicB);
    float k = Out(Seg(t, 16, 34)), turn = Seg(t, 90, 110);
    int ax = 76, bx = 76 + TW + 2, y = 30;
    if (!a || !b)
        return;
    G_FillA(ax, y, TW, TH, 0x000000, 40);
    TTurn(T.basicB, bx, y, TW, TH, TT_SEAT_RED, TT_SEAT_BLUE, turn);
    TCard(T.basicA, ax, Lerp(y + 50, y, k), TW, TH, TT_SEAT_BLUE, 1, 0);
    if (t > 40 && t < 160)
    {
        unsigned al = Show(t, 40, 160);
        char line[16];
        G_FillA(bx - 2, y + 4, 4, TH - 8, 0xfff070, al);
        Num(ax + TW - 14, y + 18, a->side[TT_RIGHT], 0xfff070);
        Num(bx + 6, y + 18, b->side[TT_LEFT], 0xff9090);
        snprintf(line, sizeof(line), "%d BEATS %d", a->side[TT_RIGHT], b->side[TT_LEFT]);
        Tag(bx, y - 4, line, 0x806020, al);
    }
    if (t > 110)
        Tag(CX, 98, "HIGHER NUMBER: THE CARD IS YOURS", Tt_SeatColor(TT_SEAT_BLUE, 2), Show(t, 110, 230));
}
/* The corner example (SAME, PLUS and COMBO): placed card in the middle, one above, one to the left, (one in the corner) */
static void Corner(int t, unsigned p, unsigned u, unsigned l, unsigned c, int plus, int combo)
{
    const struct TtCard *P = Tt_Card(p), *U = Tt_Card(u), *L = Tt_Card(l);
    int w = 32, h = 39, gx = 44, gy = 22;
    int px = gx + w + 2, py = gy + h + 2;
    float k = Out(Seg(t, 16, 32)), turn = Seg(t, 96, 114), chain = Seg(t, 150, 168);
    char line[40];
    if (!P || !U || !L)
        return;
    G_FillA(px, py, w, h, 0x000000, 40);
    if (combo && c)
        TTurn(c, gx, gy, w, h, TT_SEAT_RED, TT_SEAT_BLUE, chain);
    TTurn(u, px, gy, w, h, TT_SEAT_RED, TT_SEAT_BLUE, turn);
    TTurn(l, gx, py, w, h, TT_SEAT_RED, TT_SEAT_BLUE, turn);
    TCard(p, Lerp(px + 60, px, k), py, w, h, TT_SEAT_BLUE, 1, 0);
    if (t > 36 && t < 150)
    {
        unsigned al = Show(t, 36, 150);
        G_FillA(px + 4, py - 2, w - 8, 3, 0xfff070, al);
        G_FillA(px - 2, py + 4, 3, h - 8, 0xfff070, al);
        Num(px + w / 2 - 3, py + 2, P->side[TT_UP], 0xfff070);
        Num(px + w / 2 - 3, gy + h - 12, U->side[TT_DOWN], 0xff9090);
        Num(px + 2, py + h / 2 - 4, P->side[TT_LEFT], 0xfff070);
        Num(gx + w - 10, py + h / 2 - 4, L->side[TT_RIGHT], 0xff9090);
        if (plus)
            snprintf(line, sizeof(line), "%d+%d AND %d+%d: BOTH %d", P->side[TT_UP], U->side[TT_DOWN], P->side[TT_LEFT],
                     L->side[TT_RIGHT], P->side[TT_UP] + U->side[TT_DOWN]);
        else
            snprintf(line, sizeof(line), "%d AND %d, %d AND %d: SAME", P->side[TT_UP], U->side[TT_DOWN], P->side[TT_LEFT],
                     L->side[TT_RIGHT]);
        Tag(176, 48, line, 0x806020, al);
    }
    if (t >= 96 && t < 150)
        G_Banner(176, 70, plus ? "PLUS!" : "SAME!", 0x283848, Out(Seg(t, 96, 108)), Show(t, 96, 150));
    if (combo && t >= 140)
    {
        const struct TtCard *C = Tt_Card(c);
        if (C && t < 200)
        {
            unsigned al = Show(t, 140, 200);
            G_FillA(px - 2, gy + 4, 3, h - 8, 0xfff070, al);
            Num(px + 2, gy + h / 2 - 4, U->side[TT_LEFT], 0xfff070);
            Num(gx + w - 10, gy + h / 2 - 4, C->side[TT_RIGHT], 0xff9090);
        }
        G_Banner(176, 70, "COMBO!", 0x283848, Out(Seg(t, 150, 162)), Show(t, 150, 230));
        Tag(150, 96, "A TAKEN CARD TAKES ITS NEIGHBORS", 0x806020, Show(t, 166, 240));
    }
}
static void TriadSame(int t) { Corner(t, T.sameP, T.sameU, T.sameL, 0, 0, 0); }
static void TriadPlus(int t) { Corner(t, T.plusP, T.plusU, T.plusL, 0, 1, 0); }
static void TriadCombo(int t) { Corner(t, T.sameP, T.sameU, T.sameL, T.comboC, 0, 1); }
static void TriadRules(int t)
{
    static const unsigned rules[] = {TT_RULE_OPEN, TT_RULE_THREE_OPEN, TT_RULE_SAME_WALL, TT_RULE_ELEMENTAL, TT_RULE_RANDOM,
                                     TT_RULE_SUDDEN_DEATH, TT_RULE_REVERSE, TT_RULE_FALLEN_ACE, TT_RULE_ASCENSION,
                                     TT_RULE_DESCENSION, TT_RULE_ORDER, TT_RULE_CHAOS, TT_RULE_SWAP};
    int n = (int)(sizeof(rules) / sizeof(rules[0])), i = t / 100 % n, u = t % 100, j;
    float k = Out(Seg(u, 0, 14));
    for (j = 0; j < n; j++)
        G_FillA(CX - n * 4 + j * 8, 26, 6, 3, j == i ? Accent() : 0x806890, 255);
    G_RoundFill(20, 36 + (int)((1 - k) * 8), SW - 32, 60, 0x000000, (unsigned)(110 * k));
    G_TextAlign(CX, 38 + (int)((1 - k) * 8), TA_CENTER, 0xfff0a0, G_NO_SHADOW, Tt_RuleName(rules[i]), TF_NORMAL);
    if (k > 0.5f)
        G_Text(28, 58, G_WHITE, G_NO_SHADOW, Tt_RuleHelp(rules[i]), SW - 48, TF_SMALL);
}
static void TriadHoenn(int t)
{
    static const char *const towns[2] = {"PETALBURG", "SLATEPORT"};
    float k = InOut(Seg(t, 40, 90));
    int i;
    for (i = 0; i < 2; i++)
    {
        int x = 24 + i * 120;
        G_RoundFill(x, 30, 76, 50, 0x000000, 80);
        G_TextAlign(x + 38, 32, TA_CENTER, 0xfff0a0, G_NO_SHADOW, towns[i], TF_SMALL);
        Tag(x + 38, 54, i == 0 ? "SAME" : "OPEN", 0x806020, 255);
    }
    /* the rule you bring along: spreads to the next region, or knocks one of its rules out */
    if (t < 150)
    {
        Tag((int)Lerp(62, 182, k), (int)(70 - sinf(k * 3.14159f) * 22), "SAME", Accent(), 255);
        if (t > 92)
            Tag(CX, 96, "SPREAD: NOW PLAYED THERE TOO", 0x3870c8, Show(t, 92, 150));
    }
    else
    {
        float m = Seg(t, 160, 190);
        Tag(182, 70, "SAME", Accent(), 255);
        Tag(182, 54, "OPEN", 0x806020, (unsigned)(255 * (1 - m)));
        if (m > 0)
            G_Line(160, 54 - 4, 204, 54 + 4, 0xff4040, (unsigned)(255 * (1 - m)));
        Tag(CX, 96, "ABOLISH: A RULE THERE GOES", 0xc84040, Show(t, 170, 260));
    }
}
static void TriadTrade(int t)
{
    static const unsigned trades[4] = {TT_TRADE_ONE, TT_TRADE_DIFF, TT_TRADE_DIRECT, TT_TRADE_ALL};
    static const uint8_t taken[4] = {0x04, 0x05, 0x0a, 0x1f};   /* which of their five come over */
    int i = t / 120 % 4, u = t % 120, j;
    for (j = 0; j < 5; j++)
    {
        unsigned take = taken[i] >> j & 1;
        float k = take ? InOut(Seg(u, 40 + j * 4, 64 + j * 4)) : 0;
        TCard(T.any[j], 40 + j * 34, Lerp(24, 42, k), 26, 32, k > 0.5f ? TT_SEAT_BLUE : TT_SEAT_RED, 1, 0);
    }
    G_RoundFill(10, 78, SW - 12, 28, 0x000000, 100);
    G_Text(16, 78, 0xfff0a0, G_NO_SHADOW, Tt_TradeName(trades[i]), 0, TF_NARROW);
    G_Text(16 + G_Measure(Tt_TradeName(trades[i]), TF_NARROW) + 6, 78, G_WHITE, G_NO_SHADOW, Tt_TradeHelp(trades[i]),
           SW - 28 - G_Measure(Tt_TradeName(trades[i]), TF_NARROW), TF_NARROW);
}
static void TriadControls(int t)
{
    static const char *const lines[] = {"{UPDOWN} PICK A CARD", "{A} THEN A SQUARE", "{B} BACK", "{START} MENU: CONCEDE",
                                        "{INSPECT} LOOK AT A CARD"};
    int i, lit = t / 50 % 5;
    for (i = 0; i < 3; i++)
        TCard(T.any[i], 14, 26 + i * 14, 30, 37, TT_SEAT_BLUE, 1, 0);
    G_Focus(14, 26 + (t / 50 % 3) * 14, 30, 37, G_FOCUS_TARGET, G.frame);
    for (i = 0; i < 9; i++)
        G_FillA(58 + (i % 3) * 18, 30 + (i / 3) * 20, 16, 18, 0x000000, 50);
    Controls(124, 30, lines, 5, lit);
}
static const struct Page sTriad[] = {
    {"THE GOAL", "Each player brings five cards to a 3x3 board and places one a turn. When it's full, whoever owns more "
                 "cards (the board and the hand) wins.",
     TriadGoal, 300},
    {"THE NUMBERS", "Every card has four numbers, one for each side. A is 10.", TriadNumbers, 320},
    {"TAKING CARDS", "Place a card next to one of theirs: if your number on that side is higher than theirs, their card "
                     "turns your color.",
     TriadCapture, 240},
    {"SAME (A RULE)", "If two or more of your card's numbers equal the numbers they touch, all those cards are yours.",
     TriadSame, 220},
    {"PLUS (A RULE)", "If two or more sides make the same sum with the numbers they touch, all those cards are yours.",
     TriadPlus, 220},
    {"COMBO", "Cards taken by SAME or PLUS go on to take their neighbors with the higher number, and so on.", TriadCombo,
     260},
    {"MORE RULES", "Each game can carry more rules. The table shows them before the first card is played.", TriadRules,
     1300},
    {"THE RULES OF HOENN", "Each region has its own rules. The last region's rules come with you: they can spread to "
                           "the next region, or abolish one of its rules.",
     TriadHoenn, 270},
    {"WINNING CARDS", "Win against a player and take cards by the TRADE RULE. You never lose a card when you lose.",
     TriadTrade, 480},
    {"CONTROLS", "Choose a card from your hand, then the square to place it on.", TriadControls, 250},
};

/* ------------------------------------------------------------------ */
/* The guide                                                            */
/* ------------------------------------------------------------------ */
static int Font(const char *text) { return TcgUi_WrapLines(text, 228, TF_SMALL) > 2 ? TF_NARROW : TF_SMALL; }
static const struct Page *Pages(unsigned game, int *count)
{
    if (game == KIT_GAME_YUGIOH)
        return *count = (int)(sizeof(sYugioh) / sizeof(sYugioh[0])), sYugioh;
    if (game == KIT_GAME_TRIAD)
        return *count = (int)(sizeof(sTriad) / sizeof(sTriad[0])), sTriad;
    return *count = (int)(sizeof(sPokemon) / sizeof(sPokemon[0])), sPokemon;
}
int TcgGuide_Pages(unsigned game)
{
    int n;
    Pages(game, &n);
    return n;
}
void TcgGuide_Open(unsigned game)
{
    memset(&G, 0, sizeof(G));
    G.active = 1;
    G.game = game;
    if (game == KIT_GAME_TRIAD)
        FindCards();
    PlaySE(SE_RG_CARD_OPEN);
    printf("[guide] HOW TO PLAY opened (game %u, %d pages)\n", game, TcgGuide_Pages(game));
    {
        /* (the words have to fit their strip: three lines of the narrow font) */
        int n, i;
        const struct Page *pages = Pages(game, &n);
        for (i = 0; i < n; i++)
            if (TcgUi_WrapLines(pages[i].text, 228, TF_NARROW) > 3)
                printf("[guide] page %d's words don't fit (%d lines)\n", i + 1, TcgUi_WrapLines(pages[i].text, 228, TF_NARROW));
    }
}
int TcgGuide_Active(void) { return G.active; }
void TcgGuide_Goto(int page)
{
    int n;
    Pages(G.game, &n);
    if (page >= 0 && page < n)
        G.lastPage = G.page, G.page = page, G.t = 0, G.slide = 0;
}
static void Turn(int dir)
{
    int n;
    Pages(G.game, &n);
    if (G.page + dir < 0 || G.page + dir >= n)
        return;
    G.lastPage = G.page;
    G.page += dir;
    G.t = 0;
    G.slide = dir * 16;
    PlaySE(SE_RG_BAG_POCKET);
    printf("[guide] page %d: %s\n", G.page + 1, Pages(G.game, &n)[G.page].title);
}
void TcgGuide_Step(unsigned pressed, unsigned keys)
{
    if (!G.active)
        return;
    G.frame++;
    G.t++;
    if (G.slide)
        G.slide += G.slide > 0 ? -2 : 2;
    if (G.leaving)
    {
        if (!(keys & (KEY_B | KEY_A | KEY_SELECT)))
            G.active = 0;
        return;
    }
    if (pressed & (KEY_RIGHT | KEY_R | KEY_A))
    {
        int n;
        Pages(G.game, &n);
        if ((pressed & KEY_A) && G.page == n - 1)
            pressed = KEY_B;   /* (A on the last page closes it) */
        else
            Turn(1);
    }
    else if (pressed & (KEY_LEFT | KEY_L))
        Turn(-1);
    if (pressed & (KEY_B | KEY_SELECT))
    {
        PlaySE(SE_SELECT);
        G.leaving = 1;
        printf("[guide] closed\n");
    }
}
static const char *GameName(void)
{
    return G.game == KIT_GAME_YUGIOH ? "YU-GI-OH!" : G.game == KIT_GAME_TRIAD ? "TRIPLE TRIAD" : "POKeMON CARDS";
}
void TcgGuide_Draw(void)
{
    int n, i, tabW;
    const struct Page *pages = Pages(G.game, &n), *p = &pages[G.page];
    unsigned accent = Accent();
    char count[16];
    if (G.game == KIT_GAME_YUGIOH)
    {
        Ygo_SetArtMode(gMpConfig.cardArt);
        Ygo_SetBackMode(gMpConfig.cardBack);
    }
    TcgUi_Background();
    /* the title: the game on a tab in its color, the page's name beside it, where you are on the right */
    G_FillA(0, 0, 240, 18, 0x203040, 235);
    G_FillA(0, 17, 240, 1, accent, 255);
    tabW = G_Measure(GameName(), TF_NARROW) + 10;
    G_RoundFill(3, 3, tabW, 12, accent, 255);
    G_TextAlign(3 + tabW / 2, 2, TA_CENTER, G_WHITE, G_Mix(accent, 0x000000, 0.5f), GameName(), TF_NARROW);
    G_Text(tabW + 8, 1, G_WHITE, 0x101820, p->title, 0, TF_NORMAL);
    for (i = 0; i < n; i++)
        G_FillA(236 - (n - i) * 6, 8, 4, 3, i == G.page ? 0xffffff : 0x607080, 255);
    snprintf(count, sizeof(count), "%d/%d", G.page + 1, n);
    if (tabW + 8 + G_Measure(p->title, TF_NORMAL) + 6 < 234 - n * 6 - 2 - G_Measure(count, TF_NARROW))
        G_TextAlign(234 - n * 6 - 2, 3, TA_RIGHT, 0xa8b8c8, G_NO_SHADOW, count, TF_NARROW);
    /* the stage, sliding in from the side the page came from */
    {
        SDL_Rect clip = {SX + 1, SY + 1, SW - 2, SH - 2};
        SDL_Renderer *r = G_R();
        Stage();
        SDL_RenderSetClipRect(r, &clip);
        {
            int t = p->loop ? (int)(G.t % (unsigned)p->loop) : (int)G.t;
            p->draw(t);
        }
        SDL_RenderSetClipRect(r, NULL);
        if (G.slide)
            G_FillA(SX, SY, SW, SH, Felt(), (unsigned)(abs(G.slide) * 255 / 16));
    }
    if (G.page > 0)
        G_Arrow(0, SY + SH / 2 - 3, 2, (G.frame / 16) % 2 ? 0xffffff : 0xc0c8d0);
    if (G.page < n - 1)
        G_Arrow(236, SY + SH / 2 - 3, 3, (G.frame / 16) % 2 ? 0xffffff : 0xc0c8d0);
    /* the words: the strip along the bottom */
    G_FillA(0, STRIP, 240, 160 - STRIP, 0x5848c8, 255);
    G_FillA(0, STRIP, 240, 1, 0x9888f8, 255);
    G_FillA(0, STRIP + 1, 240, 1, 0x7060e0, 255);
    G_Text(6, STRIP + 3, G_WHITE, 0x302878, p->text, 228, Font(p->text));
    TcgUi_Hints(G.page < n - 1 ? "{LEFTRIGHT} PAGES  {B} BACK" : "{LEFT} PAGES  {A}{B} DONE");
}
