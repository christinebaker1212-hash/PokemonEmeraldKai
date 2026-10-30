// Moemon Kai: the Triple Triad table. Your hand (blue) down the left, theirs (red) down the right, the 3x3 board
// between them on green felt, as Final Fantasy XIV lays it out. It follows the rules engine (src/tt/tt_duel.c)
// through its events, one at a time: a card arcs from the hand and lands with a puff of dust, taken cards spin over
// to their new color in a burst of sparks (in step with each other, a COMBO's waves one after another), SAME / PLUS /
// COMBO sweep across the board, ASCENSION's +1s float up. Before the first card: the hands are dealt, the rules
// shown, and a light runs between the two names to pick who starts. Buttons as on the other card tables: A acts,
// R looks closer, B backs out (and on your hand opens the turn menu), START / SELECT open the turn menu.
#include <SDL2/SDL.h>
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "sound.h"
#include "constants/songs.h"
#include "tt/duel.h"
#include "tt_board.h"
#include "tt_gfx.h"
#include "../tcg/tcg_gfx.h"
#include "../tcg/tcg_ui_kit.h"

u16 GetCurrentMapMusic(void);

enum
{
    KEY_A = 1, KEY_B = 2, KEY_SELECT = 4, KEY_START = 8, KEY_RIGHT = 16, KEY_LEFT = 32, KEY_UP = 64, KEY_DOWN = 128,
    KEY_R = 256, KEY_L = 512,
    KEY_INSPECT = 2048   /* R2 / ZR / RT, or D on the keyboard: a card up close (gamepad.h PLATFORM_KEY_INSPECT) */
};
enum { PH_OFF, PH_DEAL, PH_RULES, PH_SPIN, PH_PLAY, PH_EVENTS, PH_END };
enum { M_HAND, M_CELL, M_THEIRS };
enum { BAN_SAME, BAN_PLUS, BAN_COMBO, BAN_WALL, BAN_SUDDEN, BAN_FIRST };

#define BOARD_X 57
#define BOARD_Y 4
#define CELL_DX 43
#define CELL_DY 52
#define HAND_STEP 21
#define MAX_PARTS 160
#define MAX_FLOATS 12

struct Part
{
    float x, y, vx, vy, size, spin;
    int life, max;
    unsigned color;
    uint8_t kind;   // 0 spark, 1 confetti, 2 star, 3 dust
};
struct Floater
{
    char text[6];
    float x, y;
    int t;
    unsigned color;
};
struct SCell
{
    uint16_t card;
    uint8_t owner, tile;
    int8_t elem;
    int flipT, flipLen, flipAxis, flipFrom;   // a flip running (flipT counts up to flipLen)
    int landT;                                // frames since it landed
    int glow;
};
static struct
{
    int active, phase, t, fast, online, lesson, waiting;
    unsigned me;
    const struct TtDuel *d;
    char names[2][24];   // [side]: 0 left (me), 1 right
    char result[200];
    u16 music, mapMusic;
    /* what the table shows (the engine is ahead while events play) */
    struct SCell cell[TT_CELLS];
    uint16_t hand[2][TT_HAND];     // [seat]
    uint8_t played[2], faceUp[2];
    float lift[2][TT_HAND];        // a hand card's slide toward the board (0..1)
    int score[2], scorePulse[2];
    /* events */
    struct TtEvent ev[TT_MAX_EVENTS];
    int evCount, evAt, wait;
    /* the card in the air */
    int flying, flyT, flyLen, flySeat, flySlot, flyCell, flyCard, flyHold, flyWasUp;
    /* banners */
    int banner, bannerT, bannerLen;
    char bannerText[24];
    /* the cursor */
    int mode, slot, cellCursor, theirSlot, menuOn, menuCursor, askConcede, inspect, inspectCard, inspectSeat;
    int lastTurnSeen;
    /* effects */
    struct Part parts[MAX_PARTS];
    struct Floater floats[MAX_FLOATS];
    int shake;
    unsigned frame;
    int spinFirst;   // PH_SPIN: who it lands on (side)
    int rulesOnly;   // PH_RULES opened from the turn menu: back to the game after
    int endShown, resultSE;
} B;
static SDL_Texture *sSoft;   // a soft round light (glows, sparks)

static int Side(unsigned seat) { return seat == B.me ? 0 : 1; }
static unsigned SeatOf(int side) { return side ? B.me ^ 1 : B.me; }
static int Dur(int frames) { return B.fast ? (frames + 1) / 2 : frames; }
/* Who starts: the light runs from side to side, slowing down. Its phase counts the runs (whole = at a name), from
   the left name (0) at the start to whoever starts at the stop: an even count ends on the left, an odd one on the
   right. The tick plays as it reaches each name, so the sound slows with the light and stops with it. */
static float SpinPhase(int t, int stop, int first)
{
    float k = t >= stop ? 1.0f : (float)t / (float)stop;
    return (1 - (1 - k) * (1 - k)) * (first ? 7.0f : 8.0f);
}
static float Ease(float t) { t = t < 0 ? 0 : t > 1 ? 1 : t; return 1 - (1 - t) * (1 - t) * (1 - t); }
static float EaseInOut(float t)
{
    t = t < 0 ? 0 : t > 1 ? 1 : t;
    return t < 0.5f ? 4 * t * t * t : 1 - (-2 * t + 2) * (-2 * t + 2) * (-2 * t + 2) / 2;
}
static float CellX(int c) { return (float)(BOARD_X + (c % 3) * CELL_DX); }
static float CellY(int c) { return (float)(BOARD_Y + (c / 3) * CELL_DY); }
static float HandX(int side, int slot)
{
    float base = side ? 192.0f : 8.0f;
    float lift = B.lift[SeatOf(side)][slot] * 9;
    return side ? base - lift : base + lift;
}
static float HandY(int slot) { return (float)(4 + slot * HAND_STEP); }

static SDL_Texture *Soft(void)
{
    if (!sSoft && G_R())
    {
        static uint32_t px[32 * 32];
        int x, y;
        for (y = 0; y < 32; y++)
            for (x = 0; x < 32; x++)
            {
                float dx = (x - 15.5f) / 16, dy = (y - 15.5f) / 16, r = sqrtf(dx * dx + dy * dy);
                float a = r >= 1 ? 0 : (1 - r) * (1 - r);
                px[y * 32 + x] = (uint32_t)(a * 255) << 24 | 0xffffff;
            }
        sSoft = SDL_CreateTexture(G_R(), SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, 32, 32);
        if (sSoft)
        {
            SDL_UpdateTexture(sSoft, NULL, px, 32 * 4);
            SDL_SetTextureScaleMode(sSoft, SDL_ScaleModeLinear);
        }
    }
    return sSoft;
}
// A soft light: additive when add, else blended
static void Light(float cx, float cy, float radius, unsigned color, unsigned alpha, int add)
{
    SDL_Texture *t = Soft();
    SDL_FRect r = {cx - radius, cy - radius, radius * 2, radius * 2};
    if (!t || radius <= 0 || !alpha)
        return;
    SDL_SetTextureBlendMode(t, add ? SDL_BLENDMODE_ADD : SDL_BLENDMODE_BLEND);
    SDL_SetTextureColorMod(t, (color >> 16) & 255, (color >> 8) & 255, color & 255);
    SDL_SetTextureAlphaMod(t, (Uint8)(alpha > 255 ? 255 : alpha));
    SDL_RenderCopyF(G_R(), t, NULL, &r);
}

// ---- particles
static uint32_t sRng = 0x51ed27;
static float Rnd(void) { return (float)(Tt_Rand(&sRng) % 10000) / 10000.0f; }
static struct Part *AddPart(unsigned kind, float x, float y, unsigned color)
{
    int i;
    for (i = 0; i < MAX_PARTS; i++)
        if (B.parts[i].life <= 0)
        {
            struct Part *p = &B.parts[i];
            memset(p, 0, sizeof(*p));
            p->kind = (uint8_t)kind;
            p->x = x;
            p->y = y;
            p->color = color;
            return p;
        }
    return NULL;
}
static void Burst(float cx, float cy, unsigned color, int n, float speed)
{
    int i;
    for (i = 0; i < n; i++)
    {
        struct Part *p = AddPart(i % 4 ? 0 : 2, cx, cy, i % 3 ? color : 0xffffff);
        float a = Rnd() * 6.2831853f, v = speed * (0.4f + Rnd());
        if (!p)
            return;
        p->vx = cosf(a) * v;
        p->vy = sinf(a) * v - 0.6f;
        p->max = p->life = 18 + (int)(Rnd() * 16);
        p->size = p->kind == 2 ? 5 + Rnd() * 3 : 2 + Rnd() * 3;
    }
}
static void Dust(float cx, float cy)
{
    int i;
    for (i = 0; i < 14; i++)
    {
        struct Part *p = AddPart(3, cx + (Rnd() - 0.5f) * 30, cy, 0xe8e0d0);
        if (!p)
            return;
        p->vx = (p->x - cx) * 0.09f;
        p->vy = -0.3f - Rnd() * 0.5f;
        p->max = p->life = 16 + (int)(Rnd() * 10);
        p->size = 3 + Rnd() * 4;
    }
}
static void Confetti(int n)
{
    static const unsigned colors[] = {0xf8d048, 0x58b8ff, 0xff6878, 0x78f088, 0xc890ff, 0xffffff};
    int i;
    for (i = 0; i < n; i++)
    {
        struct Part *p = AddPart(1, Rnd() * 240, -4 - Rnd() * 40, colors[i % 6]);
        if (!p)
            return;
        p->vx = (Rnd() - 0.5f) * 1.2f;
        p->vy = 0.6f + Rnd() * 1.2f;
        p->spin = Rnd() * 6.28f;
        p->max = p->life = 120 + (int)(Rnd() * 60);
        p->size = 2 + Rnd() * 2;
    }
}
static void Float(const char *text, float x, float y, unsigned color)
{
    int i;
    for (i = 0; i < MAX_FLOATS; i++)
        if (B.floats[i].t <= 0)
        {
            snprintf(B.floats[i].text, sizeof(B.floats[i].text), "%s", text);
            B.floats[i].x = x;
            B.floats[i].y = y;
            B.floats[i].t = 40;
            B.floats[i].color = color;
            return;
        }
}
static void TickEffects(void)
{
    int i;
    for (i = 0; i < MAX_PARTS; i++)
    {
        struct Part *p = &B.parts[i];
        if (p->life <= 0)
            continue;
        p->life--;
        p->x += p->vx;
        p->y += p->vy;
        switch (p->kind)
        {
        case 1:
            p->vy += 0.01f;
            p->vx += sinf(p->spin) * 0.03f;
            p->spin += 0.15f;
            break;
        case 3:
            p->vx *= 0.9f;
            p->vy *= 0.92f;
            break;
        default:
            p->vx *= 0.94f;
            p->vy = p->vy * 0.94f + 0.06f;
            break;
        }
    }
    for (i = 0; i < MAX_FLOATS; i++)
        if (B.floats[i].t > 0)
        {
            B.floats[i].t--;
            B.floats[i].y -= 0.35f;
        }
    if (B.shake > 0)
        B.shake--;
}
static void DrawParts(void)
{
    int i;
    for (i = 0; i < MAX_PARTS; i++)
    {
        const struct Part *p = &B.parts[i];
        float f;
        if (p->life <= 0)
            continue;
        f = (float)p->life / (float)(p->max ? p->max : 1);
        switch (p->kind)
        {
        case 1:
        {
            int w = (int)(p->size * (0.4f + 0.6f * fabsf(cosf(p->spin)))) + 1;
            G_FillA((int)p->x, (int)p->y, w, (int)p->size, p->color, (unsigned)(255 * (f < 0.2f ? f * 5 : 1)));
            break;
        }
        case 2:
            Tt_DrawStar(p->x - p->size / 2, p->y - p->size / 2, p->size * (0.5f + f * 0.5f), (unsigned)(255 * f));
            break;
        case 3:
            Light(p->x, p->y, p->size * (1.6f - f * 0.6f), p->color, (unsigned)(120 * f), 0);
            break;
        default:
            Light(p->x, p->y, p->size * 2.2f * f + 1, p->color, (unsigned)(255 * f), 1);
            G_FillA((int)p->x, (int)p->y, 1, 1, 0xffffff, (unsigned)(255 * f));
            break;
        }
    }
    for (i = 0; i < MAX_FLOATS; i++)
    {
        const struct Floater *fl = &B.floats[i];
        if (fl->t <= 0)
            continue;
        Tt_Text(fl->x - Tt_TextWidth(fl->text, 1) / 2, fl->y, fl->text, 1, fl->color, 0x101010,
                fl->t > 10 ? 255 : (unsigned)(fl->t * 25));
    }
}

// ---- what the table shows, from the engine
static void Score(void)
{
    int s, i;
    for (s = 0; s < 2; s++)
    {
        int n = 0;
        for (i = 0; i < TT_CELLS; i++)
            n += B.cell[i].card && B.cell[i].owner == s;
        for (i = 0; i < TT_HAND; i++)
            n += B.hand[s][i] && !(B.played[s] >> i & 1);
        if (n != B.score[s])
            B.scorePulse[s] = 14;
        B.score[s] = n;
    }
}
static void TakeHands(const struct TtDuel *d)
{
    int s;
    memcpy(B.hand, d->hand, sizeof(B.hand));
    for (s = 0; s < 2; s++)
    {
        B.played[s] = d->played[s];
        B.faceUp[s] = s == (int)B.me ? 0x1F : d->shown[s];
    }
}
static void TakeTiles(const struct TtDuel *d)
{
    int i;
    for (i = 0; i < TT_CELLS; i++)
    {
        memset(&B.cell[i], 0, sizeof(B.cell[i]));
        B.cell[i].tile = d->cell[i].tile;
    }
}

void TtBoard_SetFast(int fast) { B.fast = fast; }
void TtBoard_SetWaiting(int waiting) { B.waiting = waiting; }
void TtBoard_Lesson(int on) { B.lesson = on; }
void TtBoard_Result(const char *text) { snprintf(B.result, sizeof(B.result), "%s", text); }

void TtBoard_Begin(const struct TtDuel *truth, unsigned me, const char *myName, const char *theirName, unsigned music,
                   int online)
{
    int fast = B.fast, lesson = B.lesson;
    memset(&B, 0, sizeof(B));
    B.fast = fast;
    B.lesson = lesson;
    B.active = 1;
    B.d = truth;
    B.me = me;
    B.online = online;
    snprintf(B.names[0], sizeof(B.names[0]), "%s", myName);
    snprintf(B.names[1], sizeof(B.names[1]), "%s", theirName);
    TakeHands(truth);
    B.played[0] = B.played[1] = 0x1F;   // (dealt in one by one)
    TakeTiles(truth);
    B.phase = PH_DEAL;
    B.slot = 0;
    B.cellCursor = 4;
    B.lastTurnSeen = -1;
    B.spinFirst = Side(truth->first);
    sRng ^= truth->seed;
    B.mapMusic = GetCurrentMapMusic();
    B.music = (u16)(music ? music : MUS_RG_VS_TRAINER);   // a battle song for every game (the CARD CASE side's @tt files)
    PlayBGM(B.music);
    printf("[tt] table: %s vs %s, seat %u, rules %x\n", myName, theirName, me, truth->rules);
}
void TtBoard_End(void)
{
    if (B.active && B.mapMusic)
        MP_MUSIC_TO_MAP(PlayBGM(B.mapMusic));   // the map's own song again (custom music: the map's, not the game's)
    B.active = 0;
    B.phase = PH_OFF;
}
/* A won game going on to the cards to take: the victory song plays on through them, and the map's song comes back
   with TtBoard_ReleaseMusic when they are done */
static u16 sHeldMusic;
void TtBoard_EndHoldMusic(void)
{
    if (B.active && B.mapMusic)
        sHeldMusic = B.mapMusic;
    B.active = 0;
    B.phase = PH_OFF;
}
void TtBoard_ReleaseMusic(void)
{
    u16 m = sHeldMusic;
    sHeldMusic = 0;
    if (m)
        MP_MUSIC_TO_MAP(PlayBGM(m));
}

void TtBoard_Show(const struct TtDuel *truth)
{
    int i;
    B.d = truth;
    for (i = 0; i < truth->evCount && B.evCount < TT_MAX_EVENTS; i++)
        B.ev[B.evCount++] = truth->ev[i];
    if (B.phase == PH_PLAY)
    {
        B.phase = PH_EVENTS;
        B.t = 0;
    }
}
int TtBoard_Idle(void)
{
    return B.active && (B.phase == PH_PLAY || B.phase == PH_END) && B.evAt >= B.evCount && !B.flying &&
           !B.bannerT;
}

// ---- events
static void StartBanner(int kind, const char *text, int len)
{
    B.banner = kind;
    B.bannerT = 1;
    B.bannerLen = Dur(len);
    snprintf(B.bannerText, sizeof(B.bannerText), "%s", text);
}
static int NextIsFlip(int wave)
{
    return B.evAt < B.evCount && B.ev[B.evAt].kind == TTE_FLIP && B.ev[B.evAt].wave == wave;
}
static void Land(void)
{
    struct SCell *c = &B.cell[B.flyCell];
    const struct TtCell *truth = &B.d->cell[B.flyCell];
    float cx = CellX(B.flyCell) + TT_BOARD_W / 2, cy = CellY(B.flyCell) + TT_BOARD_H;
    c->card = (uint16_t)B.flyCard;
    c->owner = (uint8_t)B.flySeat;
    c->elem = truth->elem;
    c->landT = 1;
    B.flying = 0;
    B.shake = 6;
    Dust(cx, cy - 2);
    Light(cx, cy - 20, 30, Tt_SeatColor(Side(B.flySeat), 1), 200, 1);
    PlaySE(SE_RG_CARD_OPEN);
    if (c->elem)
        Float(c->elem > 0 ? "+1" : "-1", cx, cy - 34, c->elem > 0 ? 0x90ff70 : 0xff8070);
    Score();
}
static void Play(const struct TtEvent *e)
{
    switch (e->kind)
    {
    case TTE_PLACE:
        B.flying = 1;
        B.flyT = 0;
        B.flySeat = e->seat;
        B.flySlot = e->slot;
        B.flyCell = e->cell;
        B.flyCard = e->card;
        // the other side's card first slides out of their hand (and turns over if it was face down)
        B.flyHold = Side(e->seat) ? Dur(16) : 0;
        B.flyLen = Dur(18);
        B.flyWasUp = B.faceUp[e->seat] >> e->slot & 1;
        B.faceUp[e->seat] |= 1 << e->slot;
        PlaySE(SE_RG_CARD_FLIPPING);
        B.wait = B.flyHold + B.flyLen + Dur(6);
        break;
    case TTE_SHIFT:
    {
        int i;
        char text[6];
        snprintf(text, sizeof(text), "%+d", e->amount);
        for (i = 0; i < TT_CELLS; i++)
            if (B.cell[i].card && Tt_Card(B.cell[i].card)->type == e->dir)
                Float(text, CellX(i) + TT_BOARD_W / 2, CellY(i) + 10, e->amount > 0 ? 0x90ff70 : 0xff8070);
        PlaySE(e->amount > 0 ? SE_M_STAT_INCREASE : SE_M_STAT_DECREASE);
        B.wait = Dur(22);
        break;
    }
    case TTE_RULE:
        if (e->by == TT_BY_SAME)
            StartBanner(BAN_SAME, "SAME!", 46), PlaySE(SE_M_SWORDS_DANCE);
        else if (e->by == TT_BY_PLUS)
            StartBanner(BAN_PLUS, "PLUS!", 46), PlaySE(SE_M_SWORDS_DANCE);
        else
            StartBanner(BAN_COMBO, "COMBO!", 50), PlaySE(SE_M_COSMIC_POWER);
        {
            float cx = CellX(e->cell) + TT_BOARD_W / 2, cy = CellY(e->cell) + TT_BOARD_H / 2;
            Burst(cx, cy, e->by == TT_BY_PLUS ? 0x60e0ff : e->by == TT_BY_SAME ? 0xffd860 : 0xff70e0, 24, 2.6f);
        }
        B.wait = Dur(e->by == TT_BY_COMBO ? 40 : 34);
        break;
    case TTE_FLIP:
    {
        struct SCell *c = &B.cell[e->cell];
        c->flipFrom = c->owner;
        c->flipT = 1;
        c->flipLen = Dur(18);
        c->flipAxis = e->dir == TT_UP || e->dir == TT_DOWN;
        c->owner = e->seat;   // (shown from the flip's middle)
        PlaySE(SE_RG_CARD_FLIP);
        B.wait = NextIsFlip(e->wave) ? 0 : Dur(e->wave ? 22 : 20);
        break;
    }
    case TTE_ROUND:
        StartBanner(BAN_SUDDEN, "SUDDEN DEATH!", 70);
        PlaySE(SE_BANG);
        B.wait = Dur(74);
        break;
    case TTE_END:
        B.phase = PH_END;
        B.t = 0;
        B.wait = 0;
        break;
    }
}
static void NewRound(void)
{
    int i;
    // the board's cards burst into light and the new hands are dealt
    for (i = 0; i < TT_CELLS; i++)
        if (B.cell[i].card)
            Burst(CellX(i) + 20, CellY(i) + 24, Tt_SeatColor(Side(B.cell[i].owner), 1), 6, 2);
    TakeHands(B.d);
    B.played[0] = B.played[1] = 0x1F;
    TakeTiles(B.d);
    B.phase = PH_DEAL;
    B.t = 0;
    B.spinFirst = Side(B.d->first);
}

static void TickEvents(void)
{
    if (B.flying)
    {
        B.flyT++;
        if (B.flyT == B.flyHold)
            B.played[B.flySeat] |= 1 << B.flySlot;
        if (B.flyT >= B.flyHold + B.flyLen)
        {
            B.played[B.flySeat] |= 1 << B.flySlot;
            Land();
        }
    }
    if (B.wait > 0)
    {
        B.wait--;
        return;
    }
    if (B.evAt < B.evCount)
    {
        const struct TtEvent *e = &B.ev[B.evAt++];
        if (e->kind == TTE_ROUND)
        {
            Play(e);
            B.evAt = B.evCount;   // (the new round is dealt from the engine's state)
            B.evCount = B.evAt = 0;
            B.phase = PH_EVENTS;
            B.t = -1;   // (NewRound once the banner is done)
            return;
        }
        Play(e);
        return;
    }
    if (B.t < 0)
    {
        NewRound();
        return;
    }
    B.evCount = B.evAt = 0;
    if (B.phase == PH_EVENTS)
        B.phase = B.d->over ? PH_END : PH_PLAY;
    if (B.phase == PH_END && B.t == 0)
        B.t = 0;
}

static void TickCells(void)
{
    int i, s, k;
    for (i = 0; i < TT_CELLS; i++)
    {
        struct SCell *c = &B.cell[i];
        if (c->landT && ++c->landT > 40)
            c->landT = 0;
        if (c->flipT)
        {
            if (++c->flipT == c->flipLen / 2)
            {
                float cx = CellX(i) + TT_BOARD_W / 2, cy = CellY(i) + TT_BOARD_H / 2;
                Burst(cx, cy, Tt_SeatColor(Side(c->owner), 1), 12, 1.8f);
                Score();
            }
            if (c->flipT > c->flipLen)
            {
                c->flipT = 0;
                c->glow = 24;
            }
        }
        if (c->glow > 0)
            c->glow--;
    }
    // hand cards slide toward the board when chosen (and the other side's as it plays)
    for (s = 0; s < 2; s++)
        for (k = 0; k < TT_HAND; k++)
        {
            float want = 0;
            if (Side((unsigned)s) == 0 && B.phase == PH_PLAY && B.d->toMove == B.me && k == B.slot &&
                (B.mode == M_HAND || B.mode == M_CELL))
                want = B.mode == M_CELL ? 1 : 0.6f;
            if (B.flying && B.flySeat == s && B.flySlot == k && B.flyT < B.flyHold)
                want = 1;
            if (Side((unsigned)s) == 1 && B.mode == M_THEIRS && k == B.theirSlot)
                want = 0.5f;
            B.lift[s][k] += (want - B.lift[s][k]) * 0.3f;
        }
    for (s = 0; s < 2; s++)
        if (B.scorePulse[s] > 0)
            B.scorePulse[s]--;
}

// ---- the player's turn
static int Playable(int slot) { return TtDuel_Playable(B.d) >> slot & 1; }
static int FirstPlayable(void)
{
    int k;
    for (k = 0; k < TT_HAND; k++)
        if (Playable(k))
            return k;
    return 0;
}
static void MoveHand(int dir)
{
    int k, s = B.slot;
    for (k = 0; k < TT_HAND; k++)
    {
        s = (s + dir + TT_HAND) % TT_HAND;
        if (!(B.played[B.me] >> s & 1))
        {
            B.slot = s;
            PlaySE(SE_RG_BAG_CURSOR);
            return;
        }
    }
}
static int TheirFirst(void)
{
    int k;
    for (k = 0; k < TT_HAND; k++)
        if (!(B.played[B.me ^ 1] >> k & 1))
            return k;
    return -1;
}
static void MoveTheirs(int dir)
{
    int k, s = B.theirSlot;
    for (k = 0; k < TT_HAND; k++)
    {
        s = (s + dir + TT_HAND) % TT_HAND;
        if (!(B.played[B.me ^ 1] >> s & 1))
        {
            B.theirSlot = s;
            PlaySE(SE_RG_BAG_CURSOR);
            return;
        }
    }
}
static void OpenInspect(unsigned card, int seat)
{
    if (!card)
        return;
    B.inspect = 1;
    B.inspectCard = (int)card;
    B.inspectSeat = seat;
    PlaySE(SE_SELECT);
}
static const char *const sMenu[] = {"KEEP PLAYING", "RULES", "CONCEDE"};

int TtBoard_Step(unsigned pressed, unsigned held, unsigned *slot, unsigned *cell)
{
    (void)held;
    if (!B.active)
        return TB_NONE;
    B.frame++;
    B.t++;
    TickEffects();
    TickCells();
    if (B.bannerT && ++B.bannerT > B.bannerLen)
        B.bannerT = 0;
    if (B.inspect)
    {
        if (pressed & (KEY_A | KEY_B | KEY_INSPECT))
        {
            B.inspect = 0;
            PlaySE(SE_SELECT);
        }
        return TB_NONE;
    }
    switch (B.phase)
    {
    case PH_DEAL:
    {
        int per = Dur(6), n = B.t / per;
        if (B.t % per == 1 && n < 10)
        {
            int seat = n % 2 ? (int)(B.d->first ^ 1) : (int)B.d->first, k = n / 2;
            B.played[seat] &= (uint8_t)~(1 << k);
            PlaySE(n % 2 ? SE_RG_CARD_FLIPPING : SE_BALL_TRAY_ENTER);
        }
        if (B.t > per * 10 + Dur(12))   // (no skipping: the deal plays out, as every animation does)
        {
            B.played[0] = B.d->played[0];
            B.played[1] = B.d->played[1];
            Score();
            B.phase = B.d->round > 1 ? PH_SPIN : PH_RULES;
            B.t = 0;
        }
        return TB_NONE;
    }
    case PH_RULES:
        if (B.t > Dur(150) || ((pressed & (KEY_A | KEY_B)) && B.t > 10))
        {
            B.phase = B.rulesOnly ? PH_PLAY : PH_SPIN;
            B.rulesOnly = 0;
            B.t = 0;
        }
        return TB_NONE;
    case PH_SPIN:
    {
        int len = Dur(80);
        if (B.t > 0 && B.t < len - Dur(18) &&
            (int)SpinPhase(B.t, len - Dur(18), B.spinFirst) > (int)SpinPhase(B.t - 1, len - Dur(18), B.spinFirst))
            PlaySE(SE_RG_BAG_CURSOR);   // (the light reached a name)
        if (B.t == len - Dur(18))
        {
            PlaySE(SE_PIN);
            Burst(B.spinFirst ? 180 : 60, 80, Tt_SeatColor(B.spinFirst, 1), 20, 2.4f);
        }
        if (B.t > len + Dur(30))   // (no skipping: the spin plays out)
        {
            B.phase = PH_PLAY;
            B.t = 0;
            B.slot = FirstPlayable();
            B.mode = M_HAND;
        }
        return TB_NONE;
    }
    case PH_EVENTS:
        TickEvents();
        return TB_NONE;
    case PH_END:
        if (B.t == 1)
        {
            unsigned w = B.d->winner;
            printf("[tt] game over: %s\n", w == B.me ? "won" : w == 2 ? "draw" : "lost");
            if (w == B.me)
            {
                PlayBGM(MUS_VICTORY_TRAINER);
                Confetti(90);
            }
            else if (w == 2)
                PlaySE(SE_FAILURE);
            else
                FadeOutBGM(4);
        }
        if (B.t > Dur(70) && (pressed & (KEY_A | KEY_B)))
        {
            PlaySE(SE_SELECT);
            return TB_DONE;
        }
        return TB_NONE;
    default:
        break;
    }
    // PH_PLAY (events shown while the table was still being set up are played now)
    if (B.evAt < B.evCount)
    {
        B.phase = PH_EVENTS;
        return TB_NONE;
    }
    /* A card still flying to the table or a banner up (SAME!, PLUS!, COMBO!): the buttons do nothing until it has
       played out */
    if (B.flying || B.bannerT)
        pressed = 0;
    if (B.lastTurnSeen != B.d->turn + B.d->round * 16)
    {
        B.lastTurnSeen = B.d->turn + B.d->round * 16;
        if (B.d->toMove == B.me)
        {
            B.slot = Playable(B.slot) ? B.slot : FirstPlayable();
            B.mode = M_HAND;
        }
    }
    if (B.menuOn)
    {
        if (B.askConcede)
        {
            if (pressed & (KEY_UP | KEY_DOWN))
                B.askConcede = 3 - B.askConcede, PlaySE(SE_SELECT);
            if (pressed & KEY_A)
            {
                int yes = B.askConcede == 1;
                B.askConcede = 0;
                B.menuOn = 0;
                PlaySE(SE_SELECT);
                if (yes)
                    return TB_CONCEDE;
            }
            else if (pressed & KEY_B)
                B.askConcede = 0, PlaySE(SE_SELECT);
            return TB_NONE;
        }
        if (pressed & KEY_UP)
            B.menuCursor = (B.menuCursor + 2) % 3, PlaySE(SE_SELECT);
        if (pressed & KEY_DOWN)
            B.menuCursor = (B.menuCursor + 1) % 3, PlaySE(SE_SELECT);
        if (pressed & (KEY_B | KEY_START | KEY_SELECT))
            B.menuOn = 0, PlaySE(SE_SELECT);
        else if (pressed & KEY_A)
        {
            PlaySE(SE_SELECT);
            if (B.menuCursor == 0)
                B.menuOn = 0;
            else if (B.menuCursor == 1)
            {
                B.menuOn = 0;
                B.phase = PH_RULES;
                B.t = 11;
                B.rulesOnly = 1;
            }
            else
                B.askConcede = 2;
        }
        return TB_NONE;
    }
    if (pressed & (KEY_START | KEY_SELECT))
    {
        B.menuOn = 1;
        B.menuCursor = 0;
        PlaySE(SE_WIN_OPEN);
        return TB_NONE;
    }
    if (pressed & KEY_INSPECT)
    {
        if (B.mode == M_CELL && B.cell[B.cellCursor].card)
            OpenInspect(B.cell[B.cellCursor].card, B.cell[B.cellCursor].owner);
        else if (B.mode == M_THEIRS && B.theirSlot >= 0 && (B.faceUp[B.me ^ 1] >> B.theirSlot & 1))
            OpenInspect(B.hand[B.me ^ 1][B.theirSlot], (int)(B.me ^ 1));
        else if (B.mode != M_THEIRS && !(B.played[B.me] >> B.slot & 1))
            OpenInspect(B.hand[B.me][B.slot], (int)B.me);
        return TB_NONE;
    }
    switch (B.mode)
    {
    case M_HAND:
        if (pressed & KEY_UP)
            MoveHand(-1);
        else if (pressed & KEY_DOWN)
            MoveHand(1);
        else if (pressed & KEY_B)
        {
            B.menuOn = 1;
            B.menuCursor = 0;
            PlaySE(SE_WIN_OPEN);
        }
        else if (pressed & (KEY_A | KEY_RIGHT))
        {
            if (B.d->toMove != B.me)
            {
                if (pressed & KEY_RIGHT)
                    B.mode = M_CELL, PlaySE(SE_SELECT);
            }
            else if (!Playable(B.slot))
            {
                PlaySE(SE_FAILURE);
                B.shake = 4;
            }
            else
            {
                B.mode = M_CELL;
                if (B.cell[B.cellCursor].card)
                {
                    int c;
                    for (c = 0; c < TT_CELLS; c++)
                        if (!B.cell[(4 + c) % TT_CELLS].card)
                        {
                            B.cellCursor = (4 + c) % TT_CELLS;
                            break;
                        }
                }
                PlaySE(SE_SELECT);
            }
        }
        break;
    case M_CELL:
    {
        int row = B.cellCursor / 3, col = B.cellCursor % 3;
        if (pressed & KEY_UP && row > 0)
            B.cellCursor -= 3, PlaySE(SE_RG_BAG_CURSOR);
        else if (pressed & KEY_DOWN && row < 2)
            B.cellCursor += 3, PlaySE(SE_RG_BAG_CURSOR);
        else if (pressed & KEY_LEFT)
        {
            if (col > 0)
                B.cellCursor--, PlaySE(SE_RG_BAG_CURSOR);
            else
                B.mode = M_HAND, PlaySE(SE_SELECT);
        }
        else if (pressed & KEY_RIGHT)
        {
            if (col < 2)
                B.cellCursor++, PlaySE(SE_RG_BAG_CURSOR);
            else if (TheirFirst() >= 0)
            {
                B.mode = M_THEIRS;
                if (B.played[B.me ^ 1] >> B.theirSlot & 1)
                    B.theirSlot = TheirFirst();
                PlaySE(SE_SELECT);
            }
        }
        else if (pressed & KEY_B)
            B.mode = M_HAND, PlaySE(SE_SELECT);
        else if (pressed & KEY_A)
        {
            if (B.d->toMove == B.me && !B.waiting && !B.cell[B.cellCursor].card && Playable(B.slot) &&
                TtDuel_CanPlay(B.d, (unsigned)B.slot, (unsigned)B.cellCursor))
            {
                *slot = (unsigned)B.slot;
                *cell = (unsigned)B.cellCursor;
                B.mode = M_HAND;
                return TB_MOVE;
            }
            PlaySE(SE_FAILURE);
        }
        break;
    }
    case M_THEIRS:
        if (pressed & KEY_UP)
            MoveTheirs(-1);
        else if (pressed & KEY_DOWN)
            MoveTheirs(1);
        else if (pressed & (KEY_LEFT | KEY_B))
        {
            B.mode = pressed & KEY_LEFT ? M_CELL : M_HAND;
            PlaySE(SE_SELECT);
        }
        break;
    }
    return TB_NONE;
}

int TtBoard_DebugPhase(void) { return B.phase * 1000 + (B.t > 999 ? 999 : B.t); }
void TtBoard_DebugFocus(int where)
{
    if (where < TT_HAND)
    {
        B.mode = M_HAND;
        B.slot = where;
    }
    else if (where < TT_HAND + TT_CELLS)
    {
        B.mode = M_CELL;
        B.cellCursor = where - TT_HAND;
    }
}

// ---- drawing
static unsigned sHueTable[6] = {0xff5050, 0xffb040, 0xfff050, 0x60f070, 0x50c0ff, 0xc070ff};
static unsigned Hue(float t)
{
    int i = (int)t;
    float f = t - (float)i;
    return G_Mix(sHueTable[((i % 6) + 6) % 6], sHueTable[(((i + 1) % 6) + 6) % 6], f);
}

static void Background(void)
{
    int y, i;
    for (y = 0; y < 160; y += 2)
        G_FillA(0, y, 240, 2, G_Mix(0x14324a, 0x081420, (float)y / 160), 255);
    // drifting lights
    for (i = 0; i < 9; i++)
    {
        float t = (float)B.frame * (0.004f + i * 0.0007f) + i * 1.7f;
        float x = 120 + sinf(t) * (110 - i * 7), yy = 80 + cosf(t * 1.3f + i) * (70 - i * 4);
        Light(x, yy, 18 + i * 3, i % 2 ? 0x2860a0 : 0x206870, 70, 1);
    }
    // the two hands' sides
    for (i = 0; i < 2; i++)
    {
        unsigned color = Tt_SeatColor(i, 2);
        int x = i ? 186 : 0, active = B.phase == PH_PLAY && Side(B.d->toMove) == i;
        float pulse = active ? 0.5f + 0.5f * sinf((float)B.frame * 0.12f) : 0;
        G_FillA(x, 0, 54, 160, color, 70 + (unsigned)(pulse * 50));
        G_FillA(i ? 186 : 53, 0, 1, 160, Tt_SeatColor(i, 1), 90 + (unsigned)(pulse * 120));
    }
}
static void BoardFelt(void)
{
    int c;
    G_RoundFill(BOARD_X - 4, BOARD_Y - 3, 3 * CELL_DX + 5, 3 * CELL_DY + 3, 0x2a1a0e, 255);
    G_RoundFill(BOARD_X - 3, BOARD_Y - 2, 3 * CELL_DX + 3, 3 * CELL_DY + 1, 0x16402c, 255);
    G_RoundFrame(BOARD_X - 4, BOARD_Y - 3, 3 * CELL_DX + 5, 3 * CELL_DY + 3, 0xc8a050, 255);
    // a slow sheen across the felt
    {
        float sx = (float)((B.frame * 1) % 400) - 80;
        Light(BOARD_X - 4 + sx, 80, 40, 0x58a070, 40, 1);
    }
    for (c = 0; c < TT_CELLS; c++)
    {
        int x = (int)CellX(c), y = (int)CellY(c);
        const struct SCell *s = &B.cell[c];
        G_RoundFill(x - 1, y - 1, TT_BOARD_W + 2, TT_BOARD_H + 2, 0x0c2a1c, 255);
        G_RoundFrame(x - 1, y - 1, TT_BOARD_W + 2, TT_BOARD_H + 2, 0x2e6848, 255);
        if (s->tile)
        {
            float pulse = 0.6f + 0.4f * sinf((float)B.frame * 0.08f + c);
            Light(x + 20, y + 24, 22, 0xf0e8a0, (unsigned)(60 * pulse), 1);
            if (!s->card)
                Tt_DrawTypeIcon(s->tile, (float)x + 8, (float)y + 12, 24, (unsigned)(200 * pulse));
            else
                Tt_DrawTypeIcon(s->tile, (float)x + TT_BOARD_W - 11, (float)y + TT_BOARD_H - 11, 10, 220);
        }
    }
}
static void CardFace(struct TtFace *f, unsigned id, unsigned owner, int cell)
{
    const struct TtCard *c = Tt_Card(id);
    int side;
    memset(f, 0, sizeof(*f));
    f->seat = (uint8_t)Side(owner);
    if (!c)
        return;
    for (side = 0; side < 4; side++)
    {
        int shift = TtDuel_TypeShift(B.d, c->type);
        f->mod[side] = (int8_t)(cell >= 0 ? shift + B.cell[cell].elem : 0);
    }
}
static void DrawCells(void)
{
    int i;
    for (i = 0; i < TT_CELLS; i++)
    {
        const struct SCell *s = &B.cell[i];
        struct TtFace face;
        float x = CellX(i), y = CellY(i), sx = 1, sy = 1, lift = 0;
        unsigned owner = s->owner;
        if (!s->card)
            continue;
        CardFace(&face, s->card, owner, i);
        if (s->flipT)
        {
            float t = (float)s->flipT / (float)s->flipLen, k = cosf(t * 3.14159f);
            if (t < 0.5f)
                owner = s->flipFrom;
            if (s->flipAxis)
                sy = fabsf(k);
            else
                sx = fabsf(k);
            lift = sinf(t * 3.14159f) * 6;
            face.seat = (uint8_t)Side(owner);
            face.glow = (uint8_t)(sinf(t * 3.14159f) * 160);
            // a shadow while it's in the air
            G_FillA((int)x + 3, (int)y + 4, TT_BOARD_W - 4, TT_BOARD_H - 4, 0x000000, (unsigned)(60 * sinf(t * 3.14159f)));
        }
        else if (s->glow)
            face.glow = (uint8_t)(s->glow * 5);
        if (s->landT && s->landT < 12)
        {
            float t = s->landT / 12.0f;
            face.glow = (uint8_t)(200 * (1 - t));
            // a ring of light spreading from where it landed
            G_RoundFrame((int)(x - t * 6), (int)(y - t * 6), (int)(TT_BOARD_W + t * 12), (int)(TT_BOARD_H + t * 12),
                         Tt_SeatColor(Side(owner), 1), (unsigned)(220 * (1 - t)));
        }
        Tt_DrawCard(s->card, x - (TT_BOARD_W * (1 + lift / 40) - TT_BOARD_W) / 2,
                    y - lift - (TT_BOARD_H * (1 + lift / 40) - TT_BOARD_H) / 2, TT_BOARD_W * (1 + lift / 40),
                    TT_BOARD_H * (1 + lift / 40), sx, sy, 255, &face);
    }
}
// Is hand card k of this side the one being looked at (drawn last, on top of the others)?
static int HandPicked(int side, int k)
{
    if (B.phase != PH_PLAY)
        return 0;
    if (side == 0)
        return (B.mode == M_HAND || B.mode == M_CELL) && k == B.slot && B.d->toMove == B.me;
    return B.mode == M_THEIRS && k == B.theirSlot;
}
static void DrawHandCard(int side, unsigned seat, int k)
{
    struct TtFace face;
    float x = HandX(side, k), y = HandY(k);
    int up = B.faceUp[seat] >> k & 1, picked = HandPicked(side, k);
    CardFace(&face, B.hand[seat][k], seat, -1);
    face.back = !up;
    if (picked && side == 0)
    {
        float pulse = 0.5f + 0.5f * sinf((float)B.frame * 0.2f);
        Light(x + 20, y + 24, 34, Tt_SeatColor(0, 1), (unsigned)(90 + 80 * pulse), 1);
    }
    // ORDER / CHAOS: the card that must be played wears a gold edge
    if (B.phase == PH_PLAY && B.d->forced < TT_HAND && seat == B.d->toMove && k == B.d->forced)
        G_RoundFrame((int)x - 2, (int)y - 2, TT_BOARD_W + 4, TT_BOARD_H + 4, 0xf8d048,
                     150 + (unsigned)(100 * sinf((float)B.frame * 0.2f)));
    G_FillA((int)x + 2, (int)y + 3, TT_BOARD_W, TT_BOARD_H - 2, 0x000000, picked ? 110 : 70);
    Tt_DrawCard(B.hand[seat][k], x, y, TT_BOARD_W, TT_BOARD_H, 1, 1, 255, &face);
    if (picked && side == 1)
        G_RoundFrame((int)x - 1, (int)y - 1, TT_BOARD_W + 2, TT_BOARD_H + 2, 0xffffff,
                     150 + (unsigned)(100 * sinf((float)B.frame * 0.25f)));
}
static void DrawHands(void)
{
    int side, k;
    for (side = 0; side < 2; side++)
    {
        unsigned seat = SeatOf(side);
        int top = -1;
        // from the bottom of the column up: each card covers the top of the one below it, whose numbers (along
        // its bottom edge, as in XIV) stay in sight; the card being looked at goes last, over its neighbours
        for (k = TT_HAND - 1; k >= 0; k--)
        {
            if (B.played[seat] >> k & 1 || !B.hand[seat][k])
                continue;
            if (B.flying && B.flySeat == (int)seat && B.flySlot == k)
                continue;
            if (HandPicked(side, k))
            {
                top = k;
                continue;
            }
            DrawHandCard(side, seat, k);
        }
        if (top >= 0)
            DrawHandCard(side, seat, top);
    }
}
static void DrawFlying(void)
{
    float t, x0, y0, x1, y1, x, y, arc, scale;
    struct TtFace face;
    int side = Side((unsigned)B.flySeat);
    if (!B.flying)
        return;
    x0 = HandX(side, B.flySlot);
    y0 = HandY(B.flySlot);
    x1 = CellX(B.flyCell);
    y1 = CellY(B.flyCell);
    CardFace(&face, (unsigned)B.flyCard, (unsigned)B.flySeat, -1);
    if (B.flyT < B.flyHold)
    {
        // the other side holds it up for a moment, turning it over
        float h = (float)B.flyT / (float)(B.flyHold ? B.flyHold : 1);
        float k = cosf(h * 3.14159f);
        face.back = h < 0.5f && !B.flyWasUp;
        G_FillA((int)x0 + 2, (int)y0 + 3, TT_BOARD_W, TT_BOARD_H - 2, 0x000000, 70);
        Light(x0 + 20, y0 + 24, 30, Tt_SeatColor(side, 1), (unsigned)(160 * h), 1);
        Tt_DrawCard((unsigned)B.flyCard, x0, y0 - h * 4, TT_BOARD_W, TT_BOARD_H, fabsf(k) < 0.15f ? 0.15f : 1, 1,
                    255, &face);
        return;
    }
    t = (float)(B.flyT - B.flyHold) / (float)B.flyLen;
    x = x0 + (x1 - x0) * EaseInOut(t);
    y = y0 + (y1 - y0) * EaseInOut(t);
    arc = sinf(t * 3.14159f) * 18;
    scale = 1 + sinf(t * 3.14159f) * 0.3f;
    // its shadow on the table, then the card above it
    G_FillA((int)(x + 4 + arc * 0.4f), (int)(y + 6 + arc * 0.3f), TT_BOARD_W - 2, TT_BOARD_H - 4, 0x000000,
            (unsigned)(90 - arc * 2));
    // a trail of light behind
    Light(x + 20 - (x1 - x0) * 0.08f, y + 24 - arc - (y1 - y0) * 0.08f, 24, Tt_SeatColor(side, 1), 90, 1);
    Tt_DrawCard((unsigned)B.flyCard, x - (scale - 1) * TT_BOARD_W / 2, y - arc - (scale - 1) * TT_BOARD_H / 2,
                TT_BOARD_W * scale, TT_BOARD_H * scale, 1, 1, 255, &face);
}
static void DrawCursor(void)
{
    float pulse;
    int x, y, len, k;
    if (B.phase != PH_PLAY || B.mode != M_CELL || B.menuOn)
        return;
    x = (int)CellX(B.cellCursor);
    y = (int)CellY(B.cellCursor);
    pulse = 0.5f + 0.5f * sinf((float)B.frame * 0.2f);
    // the card, faint, where it would go
    if (B.d->toMove == B.me && !B.cell[B.cellCursor].card && !(B.played[B.me] >> B.slot & 1))
    {
        struct TtFace face;
        CardFace(&face, B.hand[B.me][B.slot], B.me, -1);
        Tt_DrawCard(B.hand[B.me][B.slot], (float)x, (float)y, TT_BOARD_W, TT_BOARD_H, 1, 1,
                    (unsigned)(110 + 50 * pulse), &face);
    }
    // corner brackets, breathing in and out
    len = 9;
    k = (int)(pulse * 2);
    {
        unsigned col = B.d->toMove == B.me ? 0xffffff : 0xa0a0a0;
        int x0 = x - 3 - k, y0 = y - 3 - k, x1 = x + TT_BOARD_W + 2 + k, y1 = y + TT_BOARD_H + 2 + k;
        G_FillA(x0, y0, len, 2, col, 255);
        G_FillA(x0, y0, 2, len, col, 255);
        G_FillA(x1 - len + 1, y0, len, 2, col, 255);
        G_FillA(x1 - 1, y0, 2, len, col, 255);
        G_FillA(x0, y1 - 1, len, 2, col, 255);
        G_FillA(x0, y1 - len + 1, 2, len, col, 255);
        G_FillA(x1 - len + 1, y1 - 1, len, 2, col, 255);
        G_FillA(x1 - 1, y1 - len + 1, 2, len, col, 255);
    }
}
static void DrawScores(void)
{
    int side;
    for (side = 0; side < 2; side++)
    {
        unsigned seat = SeatOf(side);
        char n[4];
        float pulse = B.scorePulse[seat] / 14.0f, scale = 2 + pulse * 1.2f;
        float cx = side ? 213 : 27, y = 141;
        snprintf(n, sizeof(n), "%d", B.score[seat]);
        if (B.phase == PH_DEAL || B.phase == PH_RULES || B.phase == PH_SPIN)
            continue;
        Light(cx, y + 8, 14 + pulse * 10, Tt_SeatColor(side, 1), 90 + (unsigned)(pulse * 140), 1);
        Tt_Text(cx - Tt_TextWidth(n, scale) / 2, y + 8 - 3.5f * scale, n, scale, 0xffffff, Tt_SeatColor(side, 2), 255);
    }
    // whose turn: a soft arrow over the board's edge
    if (B.phase == PH_PLAY || B.phase == PH_EVENTS)
    {
        int side = Side(B.d->toMove);
        float bob = sinf((float)B.frame * 0.15f) * 2;
        int x = side ? 184 - 1 : 55, y = 150;
        if (B.phase == PH_PLAY)
        {
            G_Arrow((int)(x + (side ? -2 : -5) + (side ? -bob : bob)), y - 2, side ? 3 : 2, Tt_SeatColor(side, 1));
            if (side == 1 || B.waiting)
            {
                // thinking
                int k;
                for (k = 0; k < 3; k++)
                    G_FillA(side ? 205 + k * 5 : 19 + k * 5, 133,
                            2, 2, 0xffffff, ((B.frame / 8) % 3) == (unsigned)k ? 255 : 90);
            }
        }
    }
}
static void Banner(void)
{
    float t, x, scale, w;
    int len = B.bannerLen, i, n;
    unsigned band;
    if (!B.bannerT)
        return;
    t = (float)B.bannerT / (float)len;
    n = (int)strlen(B.bannerText);
    scale = 3;
    w = Tt_TextWidth(B.bannerText, scale);
    if (w > 220)
    {
        scale = 2;
        w = Tt_TextWidth(B.bannerText, scale);
    }
    // the band opens, holds, closes; the word slides in, waits, slides on out
    {
        float open = t < 0.15f ? t / 0.15f : t > 0.85f ? (1 - t) / 0.15f : 1;
        int h = (int)(34 * Ease(open));
        band = B.banner == BAN_PLUS ? 0x103850 : B.banner == BAN_SAME ? 0x403010 : B.banner == BAN_SUDDEN ? 0x401010
                                                                                                          : 0x381040;
        G_FillA(0, 80 - h / 2, 240, h, band, 200);
        G_FillA(0, 80 - h / 2, 240, 1, 0xffffff, 120);
        G_FillA(0, 80 + h / 2 - 1, 240, 1, 0xffffff, 120);
    }
    if (t < 0.2f)
        x = 240 - (240 - (120 - w / 2)) * Ease(t / 0.2f);
    else if (t < 0.8f)
        x = 120 - w / 2 - (t - 0.2f) * 10;
    else
        x = 120 - w / 2 - 6 - (240) * EaseInOut((t - 0.8f) / 0.2f);
    // a smear of motion behind, then each letter
    for (i = 0; i < n; i++)
    {
        unsigned fill;
        float lx = x + i * 6 * scale, wob = sinf((float)B.frame * 0.3f + i * 0.7f) * (B.banner == BAN_COMBO ? 2 : 0.6f);
        switch (B.banner)
        {
        case BAN_PLUS: fill = G_Mix(0x80f0ff, 0xffffff, 0.5f + 0.5f * sinf(B.frame * 0.2f + i)); break;
        case BAN_SAME: fill = G_Mix(0xffd040, 0xfff8c0, 0.5f + 0.5f * sinf(B.frame * 0.2f + i)); break;
        case BAN_COMBO: fill = Hue((float)B.frame * 0.15f + i * 0.8f); break;
        case BAN_SUDDEN: fill = G_Mix(0xff5040, 0xffd0a0, 0.5f + 0.5f * sinf(B.frame * 0.25f + i)); break;
        default: fill = 0xffffff; break;
        }
        if (t < 0.2f || t > 0.8f)
            Tt_Char(lx + 10, 80 - 3.5f * scale + wob, B.bannerText[i], scale, scale, fill, G_NO_SHADOW, 70);
        Tt_Char(lx, 80 - 3.5f * scale + wob, B.bannerText[i], scale, scale, fill, 0x101010, 255);
    }
    if (B.bannerT % 3 == 0 && t > 0.15f && t < 0.8f)
    {
        struct Part *p = AddPart(2, 120 - w / 2 + Rnd() * w, 80 + (Rnd() - 0.5f) * 26, 0xffffff);
        if (p)
        {
            p->max = p->life = 20;
            p->size = 6;
            p->vy = -0.4f;
        }
    }
}
static void RuleChips(int y, unsigned alpha)
{
    char line[160];
    int r, x, count = 0, rowW[3] = {0, 0, 0}, row = 0;
    // measure the rows first: the chips are centered
    for (r = 0; r < TT_RULE_COUNT; r++)
        if (B.d->rules >> r & 1)
        {
            int w = G_Measure(Tt_RuleName((unsigned)r), TF_NARROW) + 10;
            if (rowW[row] + w > 200 && row < 2)
                row++;
            rowW[row] += w + 3;
            count++;
        }
    if (!count)
    {
        G_TextAlign(120, y + 2, TA_CENTER, 0xe0e8f0, 0x101820, "No special rules", TF_NARROW);
        y += 18;
    }
    else
    {
        row = 0;
        x = 120 - rowW[0] / 2;
        for (r = 0; r < TT_RULE_COUNT; r++)
        {
            const char *name = Tt_RuleName((unsigned)r);
            int w, lean;
            unsigned fill;
            if (!(B.d->rules >> r & 1))
                continue;
            w = G_Measure(name, TF_NARROW) + 10;
            if (x + w > 120 + rowW[row] / 2 + 1 && row < 2)
            {
                row++;
                x = 120 - rowW[row] / 2;
                y += 16;
            }
            lean = Tt_RuleLean((unsigned)r);
            fill = lean > 0 ? 0x2c6898 : lean < 0 ? 0x985830 : 0x5c5c78;
            G_RoundFill(x, y, w, 14, fill, alpha);
            G_RoundFrame(x, y, w, 14, G_Mix(fill, 0xffffff, 0.5f), alpha);
            G_TextAlign(x + w / 2, y, TA_CENTER, G_WHITE, G_Mix(fill, 0, 0.5f), name, TF_NARROW);
            x += w + 3;
        }
        y += 18;
    }
    snprintf(line, sizeof(line), "TRADE RULE: %s", Tt_TradeName(B.d->trade));
    G_TextAlign(120, y, TA_CENTER, 0xf8e8a0, 0x302010, line, TF_NARROW);
}
static void RulesPanel(void)
{
    float t = Ease((float)B.t / 14.0f);
    int h = (int)(92 * t), y = 80 - h / 2;
    if (h < 6)
        return;
    G_FillA(0, 0, 240, 160, 0x000000, (unsigned)(120 * t));
    G_RoundFill(20, y, 200, h, 0x182838, 240);
    G_RoundFrame(20, y, 200, h, 0xc8a050, 255);
    G_RoundFrame(22, y + 2, 196, h - 4, 0x4a6080, 255);
    if (t < 0.95f)
        return;
    Tt_Text(120 - Tt_TextWidth("RULES", 2) / 2, (float)y + 8, "RULES", 2, 0xf8d878, 0x302010, 255);
    RuleChips(y + 30, 255);
    if (B.lesson && !B.rulesOnly)
        G_TextAlign(120, y + h - 16, TA_CENTER, 0xc0d0e0, 0x101820, "Higher numbers take the cards they face.",
                    TF_NARROW);
}
static void Spin(void)
{
    int len = Dur(80), stop = len - Dur(18), i;
    float pos;
    for (i = 0; i < 2; i++)
    {
        int x = i ? 132 : 16, w = 92;
        unsigned c = Tt_SeatColor(i, 0);
        G_RoundFill(x, 68, w, 24, G_Mix(c, 0x000000, 0.45f), 235);
        G_RoundFrame(x, 68, w, 24, Tt_SeatColor(i, 1), 255);
        G_TextAlign(x + w / 2, 72, TA_CENTER, G_WHITE, G_Mix(c, 0, 0.6f), B.names[i], TF_NARROW);
    }
    // the light runs back and forth, slowing down, and stops on whoever starts
    if (B.t < stop)
        pos = 0.5f - 0.5f * cosf(SpinPhase(B.t, stop, B.spinFirst) * 3.14159f);
    else
        pos = (float)B.spinFirst;
    {
        float cx = 62 + pos * 116;
        Light(cx, 80, 40, Tt_SeatColor(pos > 0.5f, 1), 200, 1);
        G_RoundFrame((int)(cx - 48), 66, 96, 28, 0xffffff, 255);
    }
    if (B.t >= stop)
    {
        char line[48];
        float k = Ease((float)(B.t - stop) / 10.0f);
        snprintf(line, sizeof(line), "%s FIRST!", B.spinFirst == 0 ? "YOU GO" : "THEY GO");
        Tt_Text(120 - Tt_TextWidth(line, 2) / 2, 104 - (1 - k) * 10, line, 2, 0xffffff,
                Tt_SeatColor(B.spinFirst, 2), (unsigned)(255 * k));
    }
    Tt_Text(120 - Tt_TextWidth("WHO STARTS?", 2) / 2, 40, "WHO STARTS?", 2, 0xf8d878, 0x302010, 255);
}
static void Inspect(void)
{
    const struct TtCard *c = Tt_Card((unsigned)B.inspectCard);
    struct TtFace face;
    int y;
    if (!B.inspect || !c)
        return;
    G_FillA(0, 0, 240, 160, 0x000000, 170);
    memset(&face, 0, sizeof(face));
    face.seat = (uint8_t)Side((unsigned)B.inspectSeat);
    Light(64, 80, 70, Tt_SeatColor(face.seat, 1), 70, 1);
    Tt_DrawCard((unsigned)B.inspectCard, 12, 16, TT_BIG_W, TT_BIG_H, 1, 1, 255, &face);
    G_RoundFill(122, 12, 112, 136, 0x182838, 235);
    G_RoundFrame(122, 12, 112, 136, 0xc8a050, 255);
    y = G_Text(127, 15, G_WHITE, 0x101820, c->name, 104, TF_NARROW);
    Tt_DrawStars(c->stars, 127, y + 1, 9, 255);
    y += 12;
    if (c->type)
    {
        Tt_DrawTypeIcon(c->type, 127, (float)y, 12, 255);
        G_Text(141, y, 0xd0e0f0, 0x101820, Tt_TypeName(c->type), 90, TF_NARROW);
        y += 14;
    }
    G_Text(127, y, 0xc0c8d0, G_NO_SHADOW, c->text, 104, TF_SMALL);
    G_Hint(4, 152, G_WHITE, 0x101820, "{B} BACK", TF_NARROW, 1);
}
static void Menu(void)
{
    int i;
    if (!B.menuOn)
        return;
    G_FillA(0, 0, 240, 160, 0x000000, 90);
    G_Window(148, 100, 88, 56);
    for (i = 0; i < 3; i++)
    {
        G_Text(162, 106 + i * 15, G_INK, G_INK_SHADOW, sMenu[i], 0, TF_NORMAL);
        if (i == B.menuCursor)
            G_Cursor(152, 106 + i * 15);
    }
    if (B.askConcede)
    {
        G_Window(8, 104, 136, 52);
        G_Text(14, 108, G_INK, G_INK_SHADOW, "Give up this game?", 120, TF_NORMAL);
        G_Text(26, 124, G_INK, G_INK_SHADOW, "YES", 0, TF_NORMAL);
        G_Text(26, 138, G_INK, G_INK_SHADOW, "NO", 0, TF_NORMAL);
        G_Cursor(14, B.askConcede == 1 ? 124 : 138);
    }
}
static void End(void)
{
    unsigned w = B.d->winner;
    const char *text = w == B.me ? "YOU WIN!" : w == 2 ? "DRAW" : "YOU LOSE...";
    float scale = Tt_TextWidth(text, 4) > 228 ? 3 : 4, width = Tt_TextWidth(text, scale), x = 120 - width / 2;
    int n = (int)strlen(text), i;
    unsigned base = w == B.me ? 0xf8d048 : w == 2 ? 0xe0e8f0 : 0x9098a8;
    G_FillA(0, 0, 240, 160, 0x000000, (unsigned)(B.t > 20 ? 150 : B.t * 7));
    for (i = 0; i < n; i++)
    {
        // each letter drops in after the last and bounces
        int lt = B.t - i * 3;
        float t = lt <= 0 ? 0 : lt / 18.0f, y;
        unsigned fill = w == B.me ? G_Mix(base, 0xffffff, 0.5f + 0.5f * sinf(B.frame * 0.2f + i)) : base;
        if (lt <= 0)
            continue;
        if (t < 1)
            y = 50 - (1 - t) * 60 + (t > 0.6f ? sinf((t - 0.6f) / 0.4f * 3.14159f) * -6 : 0);
        else
            y = 50 + (w == B.me ? sinf(B.frame * 0.12f + i * 0.6f) * 2 : 0);
        Tt_Char(x + i * 6 * scale, y, text[i], scale, scale, fill, 0x101010, 255);
    }
    if (B.t > 30)
    {
        char line[40];
        snprintf(line, sizeof(line), "%d - %d", B.score[B.me], B.score[B.me ^ 1]);
        Tt_Text(120 - Tt_TextWidth(line, 2) / 2, 90, line, 2, 0xffffff, 0x101820, 255);
    }
    if (B.t > Dur(50) && B.result[0])
    {
        int lines = TcgUi_WrapLines(B.result, 196, TF_NARROW);
        G_RoundFill(16, 108, 208, 8 + lines * 11, 0x101828, 220);
        G_RoundFrame(16, 108, 208, 8 + lines * 11, 0xc8a050, 255);
        G_Text(22, 111, G_WHITE, 0x101820, B.result, 196, TF_NARROW);
    }
    if (B.t > Dur(70))
        G_Hint(176, 150, G_WHITE, 0x101820, "{A} OK", TF_NARROW, 1);
}

void TtBoard_Render(void)
{
    int dx = 0, dy = 0;
    SDL_Rect view;
    if (!B.active)
        return;
    if (B.shake)
    {
        dx = (int)((Rnd() - 0.5f) * B.shake * 0.8f);
        dy = (int)((Rnd() - 0.5f) * B.shake * 0.8f);
    }
    view.x = dx;
    view.y = dy;
    view.w = 240;
    view.h = 160;
    Background();
    if (dx || dy)
        SDL_RenderSetViewport(G_R(), &view);
    BoardFelt();
    DrawCells();
    DrawHands();
    DrawCursor();
    DrawFlying();
    if (dx || dy)
        SDL_RenderSetViewport(G_R(), NULL);
    DrawScores();
    DrawParts();
    Banner();
    if (B.phase == PH_RULES)
        RulesPanel();
    if (B.phase == PH_SPIN)
        Spin();
    if (B.phase == PH_END)
        End();
    Menu();
    Inspect();
}
