/* The Yu-Gi-Oh! duel table: today's layout turned to the GBA's wide screen.
 *
 *   their LP | their Deck   [5 Spell & Trap Zones]   their Fusion Deck | their hand
 *   banished | their GY     [5 Monster Zones     ]   their Field Zone  |
 *   ------------- the card under the cursor, the question, the phases -------------
 *              | your Field  [5 Monster Zones     ]   your GY           | banished
 *              | your Fusion [5 Spell & Trap Zones]   your Deck         | your LP
 *                                 your hand
 *
 * What's drawn is a shown copy of every card's place (Shown) that follows the rules engine's events one at a time,
 * so a summon, an activation, an attack and its damage play out in order; once every event has been shown it
 * settles on the engine's state. Only the engine's questions are asked: no stop where nothing can be done. */
#include <SDL2/SDL.h> // before global.h: its macros break the Windows intrinsics headers
#include "mp_version.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "sound.h"
#include "constants/songs.h"
#include "../tcg/tcg_gfx.h"
#include "ygo_board.h"
#include "ygo_gfx.h"

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
#define NONE 0xFF

/* ------------------------------------------------------------------ */
/* Layout                                                               */
/* ------------------------------------------------------------------ */
#define CW 18          /* a card on the table */
#define CH 25
#define CELL 26
#define ZX 55          /* the first zone column */
#define LCOL 29        /* the columns either side of the zones */
#define RCOL 185
#define OUTL 1         /* the outer columns */
#define OUTR 213
#define ROW_OPP_ST 2
#define ROW_OPP_M 29
#define ROW_MY_M 79
#define ROW_MY_ST 106
#define GAP_Y 55       /* the middle line: 55..77 */
#define HAND_W 22
#define HAND_H 30
#define HAND_Y 135
#define HAND_UP 129
#define FX_MAX 24
#define SPOTS 64

struct Rect
{
    int x, y, w, h;
};
static struct Rect R(int x, int y, int w, int h)
{
    struct Rect r = {x, y, w, h};
    return r;
}
/* side 0: the bottom (this screen's player), 1: the top */
static struct Rect ZoneRect(unsigned side, unsigned spell, unsigned seq)
{
    int x, y;
    if (seq == YGO_FIELD_ZONE)
        return side ? R(RCOL + 4, ROW_OPP_M, CW, CH) : R(LCOL + 4, ROW_MY_M, CW, CH);
    x = ZX + (int)(side ? 4 - seq : seq) * CELL + (CELL - CW) / 2;
    y = side ? (spell ? ROW_OPP_ST : ROW_OPP_M) : (spell ? ROW_MY_ST : ROW_MY_M);
    return R(x, y, CW, CH);
}
enum
{
    PILE_DECK,
    PILE_GRAVE,
    PILE_EXTRA,
    PILE_BANISHED,
    PILES
};
static struct Rect PileRect(unsigned side, unsigned pile)
{
    static const int mine[PILES][2] = {{RCOL + 4, ROW_MY_ST}, {RCOL + 4, ROW_MY_M}, {LCOL + 4, ROW_MY_ST},
                                       {OUTR + 4, ROW_MY_M}};
    static const int theirs[PILES][2] = {{LCOL + 4, ROW_OPP_ST}, {LCOL + 4, ROW_OPP_M}, {RCOL + 4, ROW_OPP_ST},
                                         {OUTL + 4, ROW_OPP_M}};
    const int *p = side ? theirs[pile] : mine[pile];
    return R(p[0], p[1], CW, CH);
}
static struct Rect LpRect(unsigned side) { return side ? R(OUTL, ROW_OPP_ST, 26, CH) : R(OUTR, ROW_MY_ST, 26, CH); }
static unsigned PileOfLoc(unsigned loc)
{
    return loc == YGO_LOC_DECK ? PILE_DECK : loc == YGO_LOC_GRAVE ? PILE_GRAVE : loc == YGO_LOC_EXTRA ? PILE_EXTRA
                                                                                                      : PILE_BANISHED;
}

/* ------------------------------------------------------------------ */
/* State                                                                */
/* ------------------------------------------------------------------ */
struct Shown
{
    uint8_t loc, seq, faceUp, attackPos, controller;
    uint16_t order;   /* hand and pile order: later is further right / on top */
};
struct CardSprite
{
    float x, y, w, h, angle;
    int placed;
    unsigned flash;
};
enum
{
    FX_BANNER,
    FX_BIGCARD,
    FX_NUMBER,
    FX_FLASH,
    FX_ATTACK,
    FX_COIN,
    FX_DICE,
    FX_RETICLE,
    FX_NEGATE,
    FX_SHUFFLE
};
struct Fx
{
    uint8_t kind, obj, side;
    int t, life, x, y, x2, y2, value;
    unsigned card, color;
    char text[32];
};
enum
{
    M_WATCH,       /* not our question: look around */
    M_FREE,        /* Main / Battle Phase: pick a card to act with */
    M_MENU,        /* what to do with the card */
    M_PHASE,       /* START: move on, concede */
    M_PICK_FIELD,  /* cards (or zones) on the table */
    M_PICK_GRID,   /* cards elsewhere, laid out over the table */
    M_WINDOW,      /* yes / no, an option, a position, chain or pass */
    M_PILE,        /* looking through a pile */
    M_CONFIRM,     /* conceding? */
    M_OVER
};
enum
{
    SP_HAND,
    SP_MZONE,
    SP_SZONE,
    SP_PILE,
    SP_OPPHAND,
    SP_DIRECT
};
struct Spot
{
    uint8_t kind, side, index, obj;
    int item;        /* the query item it picks (-1: none) */
    struct Rect r;
};
static struct
{
    int active, online, fast, tutorial, waiting;
    unsigned me;
    const struct YgoDuel *d;
    char opponent[24];
    struct Shown shown[YGO_OBJS];
    struct CardSprite spr[YGO_OBJS];
    unsigned objCount;
    uint16_t orderNext;
    int32_t lp[2], lpGoal[2];
    uint32_t seq;
    int timer, settled;
    uint8_t phase, turnPlayer;
    uint16_t turn;
    struct Fx fx[FX_MAX];
    /* input */
    int mode, cursorKind, cursorSide, cursorIndex;
    struct Spot spots[SPOTS];
    int spotCount;
    int menuItems[16], menuCount, menuCursor, menuObj;
    int windowItems[YGO_QUERY_MAX + 1], windowCount, windowCursor;
    uint8_t picked[YGO_QUERY_MAX];
    int pickedCount;
    int gridCursor, pileSide, pilePile;
    uint8_t pileList[YGO_OBJS];
    int pileCount;
    uint32_t queryStamp, askedStamp;   /* a new question resets the input */
    char log[96], result[200];
    int over, frame, tipShown, toActionable;
    unsigned tips;
    u16 music, duelMusic;
} B;
static u16 sNextMusic;
static int sNextTutorial;

static const struct YgoState *T(void) { return &B.d->s; }
static unsigned SideOf(unsigned player) { return player == B.me ? 0 : 1; }
static unsigned PlayerOf(unsigned side) { return side ? B.me ^ 1 : B.me; }
static const struct YgoCard *ObjCard(unsigned o) { return o < B.objCount ? Ygo_Card(T()->objs[o].card) : NULL; }
static int Fast(void) { return B.fast; }
static int Frames(int normal) { return Fast() ? (normal + 1) / 2 : normal; }

/* Can this screen's player see the card's face? */
static int SeesFace(unsigned o)
{
    const struct Shown *sh = &B.shown[o];
    if (o >= B.objCount)
        return 0;
    switch (sh->loc)
    {
    case YGO_LOC_HAND:
        return sh->controller == B.me;
    case YGO_LOC_MZONE:
    case YGO_LOC_SZONE:
        return sh->faceUp || T()->objs[o].controller == B.me;
    case YGO_LOC_GRAVE:
    case YGO_LOC_BANISHED:
        return 1;
    case YGO_LOC_EXTRA:
        return T()->objs[o].owner == B.me;
    default:
        return 0;
    }
}
static const char *NameOf(unsigned o)
{
    const struct YgoCard *c = ObjCard(o);
    const struct Shown *sh = &B.shown[o];
    if (!c)
        return "a card";
    if ((sh->loc == YGO_LOC_MZONE || sh->loc == YGO_LOC_SZONE) && !sh->faceUp)
        return SeesFace(o) ? c->name : "a face-down card";
    return SeesFace(o) ? c->name : "a card";
}
static const char *Who(unsigned player) { return player == B.me ? "You" : B.opponent; }
/* One line of text, cut to fit */
static void Line(int x, int y, unsigned color, const char *text, int width)
{
    char buf[128];
    size_t n;
    snprintf(buf, sizeof(buf), "%s", text);
    n = strlen(buf);
    while (n > 3 && G_Measure(buf, TF_NARROW) > width)
    {
        buf[--n] = 0;
        buf[n - 1] = '.';
    }
    G_Text(x, y, color, G_NO_SHADOW, buf, 0, TF_NARROW);
}

static struct Fx *AddFx(unsigned kind, int x, int y, int life)
{
    unsigned i;
    for (i = 0; i < FX_MAX; i++)
        if (!B.fx[i].life)
        {
            struct Fx *f = &B.fx[i];
            memset(f, 0, sizeof(*f));
            f->kind = (uint8_t)kind;
            f->x = x;
            f->y = y;
            f->life = life;
            f->obj = NONE;
            return f;
        }
    return NULL;
}
static void Log(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    vsnprintf(B.log, sizeof(B.log), format, ap);
    va_end(ap);
}

/* ------------------------------------------------------------------ */
/* Where everything is drawn                                            */
/* ------------------------------------------------------------------ */
/* The hand of a side as shown, in order */
static unsigned HandList(unsigned side, uint8_t *out)
{
    unsigned n = 0, i, j, player = PlayerOf(side);
    for (i = 0; i < B.objCount; i++)
        if (B.shown[i].loc == YGO_LOC_HAND && B.shown[i].controller == player)
            out[n++] = (uint8_t)i;
    for (i = 1; i < n; i++)
        for (j = i; j > 0 && B.shown[out[j]].order < B.shown[out[j - 1]].order; j--)
        {
            uint8_t t = out[j];
            out[j] = out[j - 1];
            out[j - 1] = t;
        }
    return n;
}
static unsigned PileList(unsigned side, unsigned pile, uint8_t *out)
{
    static const uint8_t locs[PILES] = {YGO_LOC_DECK, YGO_LOC_GRAVE, YGO_LOC_EXTRA, YGO_LOC_BANISHED};
    unsigned n = 0, i, j, player = PlayerOf(side);
    for (i = 0; i < B.objCount; i++)
        if (B.shown[i].loc == locs[pile] && T()->objs[i].owner == player)
            out[n++] = (uint8_t)i;
    for (i = 1; i < n; i++)
        for (j = i; j > 0 && B.shown[out[j]].order < B.shown[out[j - 1]].order; j--)
        {
            uint8_t t = out[j];
            out[j] = out[j - 1];
            out[j - 1] = t;
        }
    return n;
}
static int HandRaised(void)
{
    return (B.mode == M_FREE || B.mode == M_WATCH || B.mode == M_PICK_FIELD || B.mode == M_MENU) &&
           B.cursorKind == SP_HAND;
}
static struct Rect HandRect(unsigned i, unsigned n)
{
    int spacing = n > 1 ? (200 - HAND_W) / (int)(n - 1) : 0, total, x;
    if (spacing > 25)
        spacing = 25;
    total = spacing * (int)(n ? n - 1 : 0) + HAND_W;
    x = 120 - total / 2 + (int)i * spacing;
    return R(x, HandRaised() ? HAND_UP : HAND_Y, HAND_W, HAND_H);
}
static struct Rect OppHandRect(unsigned i, unsigned n)
{
    int rows = ((int)n + 1) / 2, step = rows > 1 ? 36 / (rows - 1) : 0;
    if (step > 9)
        step = 9;
    return R(OUTR + 1 + (int)(i % 2) * 12, ROW_OPP_ST + 1 + (int)(i / 2) * step, 12, 16);
}
/* Where an object's sprite belongs now (from the shown state) */
static struct Rect Home(unsigned o, float *angle)
{
    const struct Shown *sh = &B.shown[o];
    unsigned side = SideOf(sh->controller);
    *angle = 0;
    switch (sh->loc)
    {
    case YGO_LOC_HAND:
    {
        uint8_t list[YGO_HAND_MAX];
        unsigned n = HandList(side, list), i;
        for (i = 0; i < n; i++)
            if (list[i] == o)
                return side ? OppHandRect(i, n) : HandRect(i, n);
        return side ? OppHandRect(0, 1) : HandRect(0, 1);
    }
    case YGO_LOC_MZONE:
        if (!sh->attackPos)
            *angle = 90;
        return ZoneRect(side, 0, sh->seq);
    case YGO_LOC_SZONE:
        return ZoneRect(side, 1, sh->seq);
    case YGO_LOC_NONE:
        return ZoneRect(side, 0, 2);
    default:
        return PileRect(SideOf(T()->objs[o].owner), PileOfLoc(sh->loc));
    }
}
static int InPile(unsigned o)
{
    unsigned l = B.shown[o].loc;
    return l == YGO_LOC_DECK || l == YGO_LOC_GRAVE || l == YGO_LOC_EXTRA || l == YGO_LOC_BANISHED || l == YGO_LOC_NONE;
}

/* ------------------------------------------------------------------ */
/* Following the engine                                                 */
/* ------------------------------------------------------------------ */
static void TakeTruth(unsigned o)
{
    const struct YgoObj *t = &T()->objs[o];
    struct Shown *sh = &B.shown[o];
    sh->loc = t->location;
    sh->seq = t->seq;
    sh->faceUp = t->faceUp;
    sh->attackPos = t->attackPos;
    sh->controller = t->controller;
}
/* The first zone free in what's shown */
static unsigned FreeZone(unsigned player, unsigned loc)
{
    unsigned z, i, used;
    for (z = 0; z < YGO_ZONES; z++)
    {
        used = 0;
        for (i = 0; i < B.objCount; i++)
            used |= B.shown[i].loc == loc && B.shown[i].controller == player && B.shown[i].seq == z;
        if (!used)
            return z;
    }
    return 0;
}
/* An object moves to loc in the shown state: where the engine has it if it's still there, else a guess */
static void MoveShown(unsigned o, unsigned loc, unsigned player)
{
    const struct YgoObj *t = &T()->objs[o];
    struct Shown *sh = &B.shown[o];
    if (t->location == loc)
    {
        TakeTruth(o);
    }
    else
    {
        sh->loc = (uint8_t)loc;
        sh->controller = (uint8_t)player;
        if (loc == YGO_LOC_MZONE || loc == YGO_LOC_SZONE)
            sh->seq = (uint8_t)FreeZone(player, loc);
        sh->faceUp = loc != YGO_LOC_DECK && loc != YGO_LOC_EXTRA;
        sh->attackPos = 1;
    }
    sh->order = ++B.orderNext;
}
/* Everything shown has caught up: the shown state becomes the engine's */
static void Settle(void)
{
    const struct YgoState *s = T();
    unsigned o, p, i;
    for (o = 0; o < B.objCount; o++)
        TakeTruth(o);
    /* order: the engine's own */
    for (p = 0; p < 2; p++)
    {
        const struct YgoPlayer *pl = &s->players[p];
        for (i = 0; i < pl->handCount; i++)
            B.shown[pl->hand[i]].order = (uint16_t)(i + 1);
        for (i = 0; i < pl->graveCount; i++)
            B.shown[pl->grave[i]].order = (uint16_t)(i + 1);
        for (i = 0; i < pl->banishedCount; i++)
            B.shown[pl->banished[i]].order = (uint16_t)(i + 1);
        for (i = 0; i < pl->deckCount; i++)
            B.shown[pl->deck[i]].order = (uint16_t)(i + 1);
        for (i = 0; i < pl->extraCount; i++)
            B.shown[pl->extra[i]].order = (uint16_t)(i + 1);
        B.lpGoal[SideOf(p)] = pl->lp;
    }
    B.orderNext = 200;
    B.phase = s->phase;
    B.turnPlayer = s->current;
    B.turn = s->turn;
    B.settled = 1;
}

static int ObjCenter(unsigned o, int *x, int *y)
{
    float a;
    struct Rect r;
    if (o >= B.objCount)
        return 0;
    r = Home(o, &a);
    *x = r.x + r.w / 2;
    *y = r.y + r.h / 2;
    return 1;
}
static void BigCard(unsigned card, const char *label, unsigned color, int life)
{
    struct Fx *f = AddFx(FX_BIGCARD, 120, 66, life);
    if (!f)
        return;
    f->card = card;
    f->color = color;
    snprintf(f->text, sizeof(f->text), "%s", label);
}
static void Banner(const char *text, unsigned color, int life)
{
    struct Fx *f = AddFx(FX_BANNER, 120, 66, life);
    if (!f)
        return;
    f->color = color;
    snprintf(f->text, sizeof(f->text), "%s", text);
}
static void Flash(unsigned o, unsigned color, int life)
{
    struct Fx *f = AddFx(FX_FLASH, 0, 0, life);
    if (f)
        f->obj = (uint8_t)o, f->color = color;
}
static void Number(unsigned side, int value, unsigned color)
{
    struct Rect r = LpRect(side);
    struct Fx *f = AddFx(FX_NUMBER, r.x + r.w / 2, side ? r.y + r.h + 4 : r.y - 6, 50);
    if (f)
        f->value = value, f->color = color;
}

static const char *const sPhaseNames[] = {"DRAW PHASE", "STANDBY PHASE", "MAIN PHASE 1", "BATTLE PHASE",
                                          "MAIN PHASE 2", "END PHASE"};
/* One event: what changes on the table, and how long it's shown */
static int Present(const struct YgoEvent *e)
{
    unsigned o = e->obj, t = e->target;
    int x, y;
    const struct YgoCard *c = o < B.objCount ? ObjCard(o) : NULL;
    switch (e->type)
    {
    case YGO_EV_TURN:
        B.turnPlayer = e->player;
        B.turn = (uint16_t)e->value;
        Banner(e->player == B.me ? "YOUR TURN" : "OPPONENT'S TURN", e->player == B.me ? 0x3070d8 : 0xc83838, Frames(44));
        Log("Turn %d: %s.", e->value, e->player == B.me ? "your turn" : "the opponent's turn");
        PlaySE(SE_PC_LOGIN);
        return Frames(46);
    case YGO_EV_PHASE:
        B.phase = (uint8_t)e->value;
        if (e->value == YGO_PHASE_BATTLE)
        {
            Banner("BATTLE PHASE", 0xd86020, Frames(30));
            PlaySE(SE_M_DETECT);
            return Frames(30);
        }
        return e->value == YGO_PHASE_MAIN1 || e->value == YGO_PHASE_MAIN2 ? Frames(6) : 0;
    case YGO_EV_DRAW:
        if (o >= B.objCount)
            return 0;
        MoveShown(o, YGO_LOC_HAND, e->player);
        PlaySE(SE_RG_CARD_FLIP);
        if (e->player == B.me)
            Log("You drew %s.", c ? c->name : "a card");
        else
            Log("%s drew a card.", B.opponent);
        return Frames(e->player == B.me ? 12 : 8);
    case YGO_EV_SUMMON:
        if (o >= B.objCount)
            return 0;
        MoveShown(o, YGO_LOC_MZONE, e->player);
        B.shown[o].faceUp = 1;
        PlaySE(SE_M_TELEPORT);
        Log("%s %s %s!", Who(e->player), e->value == YGO_SUMMON_FLIP ? "Flip Summoned" : e->value == YGO_SUMMON_TRIBUTE ? "Tribute Summoned" :
                         e->value == YGO_SUMMON_FUSION ? "Fusion Summoned" : e->value == YGO_SUMMON_RITUAL ? "Ritual Summoned" :
                         e->value == YGO_SUMMON_NORMAL ? "Normal Summoned" : "Special Summoned", c ? c->name : "a monster");
        if (c && (e->value == YGO_SUMMON_FUSION || e->value == YGO_SUMMON_RITUAL || e->value == YGO_SUMMON_TRIBUTE ||
                  c->level >= 7))
        {
            BigCard(c->id, e->value == YGO_SUMMON_FUSION ? "FUSION SUMMON" : e->value == YGO_SUMMON_RITUAL ? "RITUAL SUMMON"
                                                                                                         : "SUMMON",
                    0xf0c040, Frames(34));
            return Frames(36);
        }
        Flash(o, 0xffffff, Frames(16));
        return Frames(22);
    case YGO_EV_SET:
        if (o >= B.objCount)
            return 0;
        MoveShown(o, c && Ygo_IsMonster(c) && T()->objs[o].location != YGO_LOC_SZONE ? YGO_LOC_MZONE : YGO_LOC_SZONE,
                  e->player);
        B.shown[o].faceUp = 0;
        if (B.shown[o].loc == YGO_LOC_MZONE)
            B.shown[o].attackPos = 0;
        PlaySE(SE_RG_CARD_FLIP);
        Log("%s set a card.", Who(e->player));
        return Frames(14);
    case YGO_EV_FLIP:
        if (o >= B.objCount)
            return 0;
        B.shown[o].faceUp = 1;
        Flash(o, 0xffffff, Frames(14));
        PlaySE(SE_RG_CARD_FLIP);
        Log("%s was flipped face-up.", c ? c->name : "A card");
        return Frames(18);
    case YGO_EV_POSITION:
        if (o >= B.objCount)
            return 0;
        B.shown[o].attackPos ^= 1;
        if (T()->objs[o].location == YGO_LOC_MZONE)
            B.shown[o].attackPos = T()->objs[o].attackPos, B.shown[o].faceUp = T()->objs[o].faceUp;
        PlaySE(SE_M_HARDEN);
        Log("%s changed %s to %s Position.", Who(e->player), NameOf(o), B.shown[o].attackPos ? "Attack" : "Defense");
        return Frames(12);
    case YGO_EV_ACTIVATE:
        if (o >= B.objCount)
            return 0;
        if (B.shown[o].loc == YGO_LOC_HAND && c && !Ygo_IsMonster(c))
            MoveShown(o, YGO_LOC_SZONE, e->player);
        if (B.shown[o].loc == YGO_LOC_SZONE || B.shown[o].loc == YGO_LOC_MZONE)
            B.shown[o].faceUp = 1;
        BigCard(T()->objs[o].card, e->value > 1 ? "CHAIN" : "ACTIVATE", e->player == B.me ? 0x60a0f0 : 0xf07060,
                Frames(40));
        PlaySE(SE_USE_ITEM);
        Log("%s activated %s%s", Who(e->player), c ? c->name : "a card", e->value > 1 ? " in a chain." : ".");
        return Frames(44);
    case YGO_EV_RESOLVE:
        if (o < B.objCount)
            Flash(o, 0xfff0a0, Frames(10));
        return Frames(10);
    case YGO_EV_NEGATE:
        if (o < B.objCount)
        {
            struct Fx *f = AddFx(FX_NEGATE, 0, 0, Frames(30));
            if (f)
                f->obj = (uint8_t)o;
        }
        PlaySE(SE_M_BARRIER);
        Log("%s was negated!", c ? c->name : "It");
        return Frames(30);
    case YGO_EV_ATTACK:
        if (o < B.objCount && ObjCenter(o, &x, &y))
        {
            struct Fx *f = AddFx(FX_ATTACK, x, y, Frames(28));
            int tx, ty;
            if (f)
            {
                struct Rect lp = LpRect(SideOf(e->player ^ 1));
                f->obj = (uint8_t)o;
                if (t < B.objCount && ObjCenter(t, &tx, &ty))
                    f->x2 = tx, f->y2 = ty;
                else
                    f->x2 = 120, f->y2 = SideOf(e->player) ? 150 : lp.y + lp.h / 2 - 20;
            }
        }
        PlaySE(SE_M_MEGA_KICK);
        if (t < B.objCount)
            Log("%s attacks %s!", c ? c->name : "A monster", NameOf(t));
        else
            Log("%s attacks directly!", c ? c->name : "A monster");
        return Frames(30);
    case YGO_EV_BATTLE:
        PlaySE(SE_EFFECTIVE);
        return Frames(16);
    case YGO_EV_DESTROY:
        if (o >= B.objCount)
            return 0;
        Flash(o, 0xff5030, Frames(18));
        PlaySE(SE_FAINT);
        Log("%s was destroyed.", NameOf(o));
        return Frames(20);
    case YGO_EV_TO_GRAVE:
        if (o >= B.objCount)
            return 0;
        MoveShown(o, YGO_LOC_GRAVE, T()->objs[o].owner);
        return Frames(12);
    case YGO_EV_TO_HAND:
        if (o >= B.objCount)
            return 0;
        MoveShown(o, YGO_LOC_HAND, T()->objs[o].owner);
        Log("%s went to %s hand.", SeesFace(o) || T()->objs[o].faceUp ? (c ? c->name : "A card") : "A card",
            T()->objs[o].owner == B.me ? "your" : "the opponent's");
        PlaySE(SE_RG_CARD_FLIP);
        return Frames(16);
    case YGO_EV_TO_DECK:
        if (o >= B.objCount)
            return 0;
        MoveShown(o, T()->objs[o].location == YGO_LOC_EXTRA ? YGO_LOC_EXTRA : YGO_LOC_DECK, T()->objs[o].owner);
        return Frames(12);
    case YGO_EV_BANISH:
        if (o >= B.objCount)
            return 0;
        Flash(o, 0xa0c0ff, Frames(12));
        MoveShown(o, YGO_LOC_BANISHED, T()->objs[o].owner);
        Log("%s was banished.", c ? c->name : "A card");
        return Frames(16);
    case YGO_EV_DAMAGE:
    {
        unsigned side = SideOf(e->player);
        B.lpGoal[side] -= e->value;
        if (B.lpGoal[side] < 0)
            B.lpGoal[side] = 0;
        Number(side, -e->value, 0xff5040);
        PlaySE(e->value >= 2000 ? SE_SUPER_EFFECTIVE : SE_M_PAY_DAY);
        Log("%s lost %d LP.", Who(e->player), e->value);
        return Frames(32);
    }
    case YGO_EV_RECOVER:
    {
        unsigned side = SideOf(e->player);
        B.lpGoal[side] += e->value;
        Number(side, e->value, 0x50e070);
        PlaySE(SE_M_MORNING_SUN);
        Log("%s gained %d LP.", Who(e->player), e->value);
        return Frames(28);
    }
    case YGO_EV_STAT:
        if (o < B.objCount)
            Flash(o, e->value >= 0 ? 0x60ff80 : 0xff6060, Frames(14));
        PlaySE(e->value >= 0 ? SE_M_SWORDS_DANCE : SE_M_LEER);
        return Frames(16);
    case YGO_EV_EQUIP:
        if (o < B.objCount)
            Flash(o, 0x80c0ff, Frames(14));
        if (t < B.objCount)
            Flash(t, 0x80c0ff, Frames(14));
        PlaySE(SE_M_REFLECT);
        Log("%s was equipped to %s.", c ? c->name : "A card", NameOf(t));
        return Frames(18);
    case YGO_EV_COIN:
    {
        struct Fx *f = AddFx(FX_COIN, 120, 66, Frames(56));
        if (f)
            f->value = e->value;
        PlaySE(SE_M_PAY_DAY);
        Log("%s tossed a coin: %s!", Who(e->player), e->value ? "heads" : "tails");
        return Frames(58);
    }
    case YGO_EV_DICE:
    {
        struct Fx *f = AddFx(FX_DICE, 120, 66, Frames(56));
        if (f)
            f->value = e->value;
        PlaySE(SE_BALL_TRAY_ENTER);
        Log("%s rolled a %d.", Who(e->player), e->value);
        return Frames(58);
    }
    case YGO_EV_SHUFFLE:
    {
        struct Rect r = PileRect(SideOf(e->player), PILE_DECK);
        AddFx(FX_SHUFFLE, r.x, r.y, Frames(16));
        PlaySE(SE_BALL_TRAY_ENTER);
        return Frames(12);
    }
    case YGO_EV_REVEAL:
        if (!c)
            return 0;
        BigCard(c->id, "REVEALED", 0xc0c0c0, Frames(30));
        Log("%s revealed %s.", Who(e->player), c->name);
        return Frames(32);
    case YGO_EV_LOOK:
        if (e->player != B.me || !c)
            return 0;
        BigCard(c->id, "", 0xc0c0c0, Frames(34));
        return Frames(36);
    case YGO_EV_TARGET:
        if (t < B.objCount)
        {
            struct Fx *f = AddFx(FX_RETICLE, 0, 0, Frames(18));
            if (f)
                f->obj = (uint8_t)t;
        }
        return Frames(16);
    case YGO_EV_CONTROL:
        if (o >= B.objCount)
            return 0;
        MoveShown(o, YGO_LOC_MZONE, e->player);
        Flash(o, 0xe080ff, Frames(18));
        PlaySE(SE_M_BIND);
        Log("%s took control of %s!", Who(e->player), c ? c->name : "a monster");
        return Frames(22);
    case YGO_EV_WIN:
        return 0;
    default:
        return 0;
    }
}

/* ------------------------------------------------------------------ */
/* Questions                                                            */
/* ------------------------------------------------------------------ */
static int MyQuestion(void)
{
    const struct YgoDuel *d = B.d;
    return d && !d->s.over && d->query.type != YGO_Q_NONE && d->query.player == B.me && YgoBoard_Idle() && !B.waiting;
}
static const char *KindLabel(const struct YgoChoice *c)
{
    const struct YgoCard *card = ObjCard(c->obj);
    switch (c->kind)
    {
    case YGO_ACT_SUMMON:
        return card && card->level >= 5 ? "TRIBUTE SUMMON" : "SUMMON";
    case YGO_ACT_SET_MONSTER:
    case YGO_ACT_SET_SPELL:
        return "SET";
    case YGO_ACT_FLIP_SUMMON:
        return "FLIP SUMMON";
    case YGO_ACT_POSITION:
        return "CHANGE POSITION";
    case YGO_ACT_ACTIVATE:
        return card && Ygo_IsMonster(card) ? "USE EFFECT" : "ACTIVATE";
    case YGO_ACT_SPECIAL:
        return "SPECIAL SUMMON";
    case YGO_ACT_ATTACK:
        return "ATTACK";
    case YGO_ACT_TO_BATTLE:
        return "BATTLE PHASE";
    case YGO_ACT_TO_MAIN2:
        return "MAIN PHASE 2";
    case YGO_ACT_END_TURN:
        return "END TURN";
    default:
        return "?";
    }
}
static const char *PromptText(const struct YgoQuery *q, char *buf, size_t size)
{
    static const char *const verbs[] = {
        [YGO_P_NONE] = "Choose",
        [YGO_P_TRIBUTE] = "Tribute",
        [YGO_P_TARGET] = "Target",
        [YGO_P_ATTACK_TARGET] = "Attack",
        [YGO_P_DISCARD] = "Discard",
        [YGO_P_DESTROY] = "Destroy",
        [YGO_P_SUMMON] = "Summon",
        [YGO_P_ADD_TO_HAND] = "Add to your hand",
        [YGO_P_TO_GRAVE] = "Send to the Graveyard",
        [YGO_P_BANISH] = "Banish",
        [YGO_P_EQUIP] = "Equip to",
        [YGO_P_MATERIAL] = "Fusion Materials",
        [YGO_P_FUSION] = "Fusion Summon",
        [YGO_P_RITUAL] = "Ritual Summon",
        [YGO_P_ORDER] = "Chain first",
    };
    const char *verb = q->prompt < sizeof(verbs) / sizeof(verbs[0]) && verbs[q->prompt] ? verbs[q->prompt] : "Choose";
    if (q->type == YGO_Q_ZONE)
        snprintf(buf, size, "Choose a zone");
    else if (q->prompt == YGO_P_ATTACK_TARGET)
        snprintf(buf, size, "Attack which?");
    else if (q->max > 1)
        snprintf(buf, size, "%s: %d%s%d (%d)", verb, q->min, q->min == q->max ? "" : "-", q->max, B.pickedCount);
    else
        snprintf(buf, size, "%s%s", verb, q->min ? "" : " (or none)");
    if (q->max > 1 && q->min == q->max)
        snprintf(buf, size, "%s %d (%d/%d)", verb, q->max, B.pickedCount, q->max);
    return buf;
}
static int ItemOnField(const struct YgoChoice *c)
{
    unsigned l;
    if (c->obj == NONE)
        return 1;   /* a direct attack: the opponent's side */
    if (c->obj >= B.objCount)
        return 0;
    l = T()->objs[c->obj].location;
    return l == YGO_LOC_MZONE || l == YGO_LOC_SZONE;
}

static void Tip(const char *text);
/* A new question: the input starts over */
static void NewQuestion(void)
{
    const struct YgoQuery *q = &B.d->query;
    unsigned i;
    int field = 1;
    B.pickedCount = 0;
    B.menuCount = 0;
    B.windowCount = 0;
    B.windowCursor = 0;
    B.gridCursor = 0;
    switch (q->type)
    {
    case YGO_Q_IDLE:
    case YGO_Q_BATTLE:
        B.mode = M_FREE;
        B.toActionable = 1;
        if (q->type == YGO_Q_BATTLE)
            Tip("Your Battle Phase: choose one of your monsters with {A} to attack. {START} moves on to Main Phase 2.");
        else
            Tip("Your Main Phase: choose a card with {A} to Summon, Set or activate it. {R} reads a card. {START} "
                "moves on to the Battle Phase or ends your turn.");
        break;
    case YGO_Q_CARDS:
        for (i = 0; i < q->count; i++)
            field &= ItemOnField(&q->items[i]);
        B.mode = field ? M_PICK_FIELD : M_PICK_GRID;
        if (field && q->count)
        {
            /* the cursor on the first choice */
            B.cursorKind = -1;
        }
        if (q->prompt == YGO_P_TRIBUTE)
            Tip("To Normal Summon a Level 5 or 6 monster, Tribute 1 monster you control; Level 7 or higher takes 2.");
        else if (q->prompt == YGO_P_ATTACK_TARGET)
            Tip("Choose the monster to attack. When your opponent has no monsters, you can attack their Life Points "
                "directly.");
        break;
    case YGO_Q_ZONE:
        B.mode = M_PICK_FIELD;
        B.cursorKind = -1;
        break;
    case YGO_Q_CHAIN:
        B.mode = M_WINDOW;
        for (i = 0; i < q->count; i++)
            B.windowItems[B.windowCount++] = (int)i;
        B.windowItems[B.windowCount++] = -1;   /* pass */
        Tip("You can respond! A Trap or Quick-Play Spell can be activated now, in a chain. Or pass with {B}.");
        break;
    default:
        B.mode = M_WINDOW;
        for (i = 0; i < q->count; i++)
            B.windowItems[B.windowCount++] = (int)i;
        if (q->type == YGO_Q_YESNO)
            B.windowCursor = 1;
        break;
    }
}

/* ------------------------------------------------------------------ */
/* Spots: what the cursor can be on                                     */
/* ------------------------------------------------------------------ */
static void AddSpot(unsigned kind, unsigned side, unsigned index, unsigned obj, int item, struct Rect r)
{
    struct Spot *s;
    if (B.spotCount >= SPOTS)
        return;
    s = &B.spots[B.spotCount++];
    s->kind = (uint8_t)kind;
    s->side = (uint8_t)side;
    s->index = (uint8_t)index;
    s->obj = (uint8_t)obj;
    s->item = item;
    s->r = r;
}
static int ObjSpot(unsigned o, int item)
{
    const struct Shown *sh = &B.shown[o];
    unsigned side = SideOf(sh->controller);
    float a;
    struct Rect r = Home(o, &a);
    if (a == 90)
        r = R(r.x - 3, r.y + 3, r.h, r.w);
    switch (sh->loc)
    {
    case YGO_LOC_HAND:
    {
        uint8_t list[YGO_HAND_MAX];
        unsigned n = HandList(side, list), i;
        for (i = 0; i < n; i++)
            if (list[i] == o)
                AddSpot(side ? SP_OPPHAND : SP_HAND, side, i, o, item, r);
        return 1;
    }
    case YGO_LOC_MZONE:
        AddSpot(SP_MZONE, side, sh->seq, o, item, r);
        return 1;
    case YGO_LOC_SZONE:
        AddSpot(SP_SZONE, side, sh->seq, o, item, r);
        return 1;
    default:
        return 0;
    }
}
static void BuildSpots(void)
{
    const struct YgoQuery *q = &B.d->query;
    unsigned o, side, p, i;
    B.spotCount = 0;
    if (B.mode == M_PICK_FIELD)
    {
        if (q->type == YGO_Q_ZONE)
        {
            for (i = 0; i < q->count; i++)
                AddSpot(SP_MZONE, 0, q->items[i].value, NONE, (int)i,
                        ZoneRect(SideOf(q->player), q->prompt == YGO_P_SUMMON ? 0 : 1, q->items[i].value));
            return;
        }
        for (i = 0; i < q->count; i++)
        {
            if (q->items[i].obj == NONE)
            {
                struct Rect r = LpRect(SideOf(q->player ^ 1));
                AddSpot(SP_DIRECT, 1, 0, NONE, (int)i, r);
            }
            else
                ObjSpot(q->items[i].obj, (int)i);
        }
        return;
    }
    /* everything there is to look at */
    for (o = 0; o < B.objCount; o++)
    {
        unsigned l = B.shown[o].loc;
        if (l == YGO_LOC_HAND || l == YGO_LOC_MZONE || l == YGO_LOC_SZONE)
            ObjSpot(o, -1);
    }
    for (side = 0; side < 2; side++)
        for (p = 0; p < PILES; p++)
            AddSpot(SP_PILE, side, p, NONE, -1, PileRect(side, p));
}
static int FindSpot(int kind, int side, int index)
{
    int i;
    for (i = 0; i < B.spotCount; i++)
        if (B.spots[i].kind == kind && B.spots[i].side == side && B.spots[i].index == index)
            return i;
    return -1;
}
static int CurrentSpot(void)
{
    int i = FindSpot(B.cursorKind, B.cursorSide, B.cursorIndex), k;
    if (i >= 0 || !B.spotCount)
        return i;
    /* the cursor's spot is gone: the nearest of its kind, else the first one of ours */
    for (k = 0; k < B.spotCount; k++)
        if (B.spots[k].kind == B.cursorKind && B.spots[k].side == B.cursorSide)
            i = k;
    if (i < 0)
        for (k = B.spotCount - 1; k >= 0; k--)
            if (B.spots[k].kind == SP_HAND)
                i = k;
    if (i < 0)
        for (k = B.spotCount - 1; k >= 0; k--)
            if (B.spots[k].side == 0 || B.mode == M_PICK_FIELD)
                i = k;
    if (i < 0)
        i = 0;
    B.cursorKind = B.spots[i].kind;
    B.cursorSide = B.spots[i].side;
    B.cursorIndex = B.spots[i].index;
    return i;
}
static void SetCursor(int i)
{
    if (i < 0 || i >= B.spotCount)
        return;
    B.cursorKind = B.spots[i].kind;
    B.cursorSide = B.spots[i].side;
    B.cursorIndex = B.spots[i].index;
}
/* The nearest spot in a direction */
static void MoveCursor(unsigned pressed)
{
    int cur = CurrentSpot(), i, best = -1;
    float bestScore = 1e9f, cx, cy;
    if (cur < 0)
        return;
    cx = B.spots[cur].r.x + B.spots[cur].r.w / 2.0f;
    cy = B.spots[cur].r.y + B.spots[cur].r.h / 2.0f;
    for (i = 0; i < B.spotCount; i++)
    {
        float x = B.spots[i].r.x + B.spots[i].r.w / 2.0f, y = B.spots[i].r.y + B.spots[i].r.h / 2.0f;
        float dx = x - cx, dy = y - cy, along, across, score;
        if (i == cur)
            continue;
        if (pressed & KEY_RIGHT)
            along = dx, across = dy;
        else if (pressed & KEY_LEFT)
            along = -dx, across = dy;
        else if (pressed & KEY_DOWN)
            along = dy, across = dx;
        else
            along = -dy, across = dx;
        if (along < 3)
            continue;
        score = along + (across < 0 ? -across : across) * 2.2f;
        if (score < bestScore)
            bestScore = score, best = i;
    }
    if (best >= 0)
    {
        SetCursor(best);
        PlaySE(SE_SELECT);
    }
}

/* ------------------------------------------------------------------ */
/* Begin / end                                                          */
/* ------------------------------------------------------------------ */
void YgoBoard_SetMusic(unsigned song) { sNextMusic = (u16)song; }
void YgoBoard_Tutorial(int on) { sNextTutorial = on; }
void YgoBoard_SetFast(int fast) { B.fast = fast; }
void YgoBoard_SetWaiting(int waiting) { B.waiting = waiting; }
void YgoBoard_Begin(const struct YgoDuel *truth, unsigned me, const char *opponent, int online)
{
    unsigned o, side;
    int fast = B.fast;
    memset(&B, 0, sizeof(B));
    B.fast = fast;
    B.active = 1;
    B.online = online;
    B.me = me;
    B.d = truth;
    B.tutorial = sNextTutorial;
    sNextTutorial = 0;
    B.duelMusic = sNextMusic;
    sNextMusic = 0;
    snprintf(B.opponent, sizeof(B.opponent), "%s", opponent);
    B.objCount = truth->s.objCount;
    /* everything starts in the Decks: the opening hands are drawn as events */
    for (o = 0; o < B.objCount; o++)
    {
        const struct YgoObj *t = &truth->s.objs[o];
        B.shown[o].loc = t->location == YGO_LOC_EXTRA ? YGO_LOC_EXTRA : YGO_LOC_DECK;
        B.shown[o].controller = t->owner;
        B.shown[o].order = (uint16_t)o;
    }
    for (side = 0; side < 2; side++)
        B.lp[side] = B.lpGoal[side] = YGO_LP;
    B.seq = 0;
    B.cursorKind = SP_HAND;
    B.mode = M_WATCH;
    B.queryStamp = 0;
    B.askedStamp = 0xFFFFFFFF;
    B.music = GetCurrentMapMusic();
    PlayBGM(B.duelMusic ? B.duelMusic : MUS_B_ARENA);
    printf("[ygo] table: %s, seat %u\n", opponent, me);
}
void YgoBoard_End(void)
{
    if (B.active && B.music)
        PlayBGM(B.music);
    B.active = 0;
}
void YgoBoard_Sync(const struct YgoDuel *truth)
{
    B.d = truth;
    B.objCount = truth->s.objCount;
}
int YgoBoard_Idle(void)
{
    return B.active && B.d && B.seq >= B.d->s.eventSeq && B.timer <= 0;
}
void YgoBoard_Result(const char *message)
{
    snprintf(B.result, sizeof(B.result), "%s", message);
}
static void Tip(const char *text)
{
    unsigned bit;
    if (!B.tutorial)
        return;
    bit = 1u << (unsigned)((text[0] + text[5] * 7 + (int)strlen(text)) % 31);
    if (B.tips & bit)
        return;
    B.tips |= bit;
    snprintf(B.result, sizeof(B.result), "%s", text);
    B.tipShown = 1;
}
void YgoBoard_DebugFocus(int kind, int index)
{
    B.cursorKind = kind == 0 ? SP_HAND : kind == 1 ? SP_MZONE : kind == 2 ? SP_SZONE : SP_MZONE;
    B.cursorSide = kind == 3;
    B.cursorIndex = index;
}

/* ------------------------------------------------------------------ */
/* Step                                                                 */
/* ------------------------------------------------------------------ */
static void Advance(void)
{
    const struct YgoState *s = T();
    int guard = 0;
    unsigned i;
    for (i = 0; i < FX_MAX; i++)
        if (B.fx[i].life)
        {
            B.fx[i].t++;
            if (B.fx[i].t >= B.fx[i].life)
                B.fx[i].life = 0;
        }
    for (i = 0; i < 2; i++)
        if (B.lp[i] != B.lpGoal[i])
        {
            int diff = B.lpGoal[i] - B.lp[i], step = diff / 6;
            if (!step)
                step = diff > 0 ? 1 : -1;
            if (step > 0 && step < 10 && diff > 10)
                step = 10;
            if (step < 0 && step > -10 && diff < -10)
                step = -10;
            B.lp[i] += step;
        }
    if (B.timer > 0)
    {
        B.timer--;
        return;
    }
    if (s->eventSeq - B.seq > YGO_EVENT_RING)
        B.seq = s->eventSeq - YGO_EVENT_RING;
    while (B.seq < s->eventSeq && B.timer <= 0 && guard++ < 64)
    {
        const struct YgoEvent *e = YgoDuel_GetEvent(s, B.seq++);
        B.settled = 0;
        if (e)
            B.timer = Present(e);
    }
    if (B.seq >= s->eventSeq && B.timer <= 0 && !B.settled)
        Settle();
}
static void Sprites(void)
{
    unsigned o;
    float k = Fast() ? 0.45f : 0.3f;
    for (o = 0; o < B.objCount; o++)
    {
        struct CardSprite *sp = &B.spr[o];
        float a;
        struct Rect r = Home(o, &a);
        if (!sp->placed)
        {
            sp->x = (float)r.x, sp->y = (float)r.y, sp->w = (float)r.w, sp->h = (float)r.h, sp->angle = a;
            sp->placed = 1;
            continue;
        }
        sp->x += (r.x - sp->x) * k;
        sp->y += (r.y - sp->y) * k;
        sp->w += (r.w - sp->w) * k;
        sp->h += (r.h - sp->h) * k;
        sp->angle += (a - sp->angle) * k;
        if (SDL_fabsf(r.x - sp->x) < 0.3f && SDL_fabsf(r.y - sp->y) < 0.3f)
            sp->x = (float)r.x, sp->y = (float)r.y, sp->w = (float)r.w, sp->h = (float)r.h, sp->angle = a;
    }
}
static int Settled(unsigned o)
{
    float a;
    struct Rect r = Home(o, &a);
    const struct CardSprite *sp = &B.spr[o];
    return (int)(sp->x + 0.5f) == r.x && (int)(sp->y + 0.5f) == r.y;
}
static unsigned Answer1(struct YgoAnswer *a, int item)
{
    memset(a, 0, sizeof(*a));
    if (item >= 0)
    {
        a->count = 1;
        a->picks[0] = (uint8_t)item;
    }
    return YB_ANSWER;
}
/* The menu for a card in the Main / Battle Phase: the query's items for it */
static int OpenMenu(unsigned obj)
{
    const struct YgoQuery *q = &B.d->query;
    unsigned i;
    B.menuCount = 0;
    for (i = 0; i < q->count && B.menuCount < 16; i++)
        if (q->items[i].obj == obj)
            B.menuItems[B.menuCount++] = (int)i;
    if (!B.menuCount)
        return 0;
    B.menuCursor = 0;
    B.menuObj = (int)obj;
    B.mode = M_MENU;
    return 1;
}
static int OpenPhaseMenu(void)
{
    const struct YgoQuery *q = &B.d->query;
    unsigned i;
    B.menuCount = 0;
    if (q->player == B.me && (q->type == YGO_Q_IDLE || q->type == YGO_Q_BATTLE))
        for (i = 0; i < q->count && B.menuCount < 15; i++)
            if (q->items[i].obj == NONE)
                B.menuItems[B.menuCount++] = (int)i;
    B.menuItems[B.menuCount++] = -2;   /* CONCEDE */
    B.menuCursor = 0;
    B.mode = M_PHASE;
    return 1;
}
static int Picked(int item)
{
    int i;
    for (i = 0; i < B.pickedCount; i++)
        if (B.picked[i] == item)
            return i + 1;
    return 0;
}
/* A pick toggled; 1: the answer is ready */
static int TogglePick(int item, struct YgoAnswer *a)
{
    const struct YgoQuery *q = &B.d->query;
    int k = Picked(item);
    if (q->max <= 1)
    {
        Answer1(a, item);
        return 1;
    }
    if (k)
    {
        memmove(&B.picked[k - 1], &B.picked[k], (size_t)(B.pickedCount - k));
        B.pickedCount--;
        PlaySE(SE_SELECT);
        return 0;
    }
    if (B.pickedCount >= q->max)
    {
        PlaySE(SE_FAILURE);
        return 0;
    }
    B.picked[B.pickedCount++] = (uint8_t)item;
    PlaySE(SE_SELECT);
    if (q->min == q->max && B.pickedCount == q->max)
    {
        memset(a, 0, sizeof(*a));
        a->count = (uint8_t)B.pickedCount;
        memcpy(a->picks, B.picked, (size_t)B.pickedCount);
        return 1;
    }
    return 0;
}
static int ConfirmPicks(struct YgoAnswer *a)
{
    const struct YgoQuery *q = &B.d->query;
    if (B.pickedCount < q->min)
    {
        PlaySE(SE_FAILURE);
        return 0;
    }
    memset(a, 0, sizeof(*a));
    a->count = (uint8_t)B.pickedCount;
    memcpy(a->picks, B.picked, (size_t)B.pickedCount);
    return 1;
}
static unsigned SpotCard(int i)
{
    const struct Spot *s;
    if (i < 0 || i >= B.spotCount)
        return 0;
    s = &B.spots[i];
    if (s->kind == SP_PILE)
    {
        uint8_t list[YGO_OBJS];
        unsigned n = PileList(s->side, s->index, list);
        return n && SeesFace(list[n - 1]) ? T()->objs[list[n - 1]].card : 0;
    }
    return s->obj < B.objCount && SeesFace(s->obj) ? T()->objs[s->obj].card : 0;
}
static void OpenPile(unsigned side, unsigned pile)
{
    unsigned n, i;
    uint8_t list[YGO_OBJS];
    n = PileList(side, pile, list);
    B.pileCount = 0;
    /* the top first; a Deck shows nothing (its count only), a Fusion Deck only to its owner */
    for (i = n; i-- > 0;)
        if (SeesFace(list[i]))
            B.pileList[B.pileCount++] = list[i];
    B.pileSide = (int)side;
    B.pilePile = (int)pile;
    B.gridCursor = 0;
    B.mode = M_PILE;
}

int YgoBoard_Step(unsigned pressed, unsigned held, struct YgoAnswer *answer, unsigned *card)
{
    const struct YgoQuery *q;
    int cur;
    (void)held;
    if (!B.active || !B.d)
        return YB_NONE;
    B.frame++;
    Advance();
    Sprites();
    q = &B.d->query;
    /* A tip of the lesson, or the result, first */
    if (B.tipShown)
    {
        if (pressed & (KEY_A | KEY_B))
        {
            B.tipShown = 0;
            B.result[0] = 0;
            PlaySE(SE_SELECT);
        }
        return YB_NONE;
    }
    if (B.d->s.over && YgoBoard_Idle())
    {
        if (B.mode != M_OVER)
        {
            unsigned won = B.d->s.winner == B.me;
            B.mode = M_OVER;
            B.over = B.frame;
            Banner(won ? "YOU WIN!" : "YOU LOSE...", won ? 0xe0a020 : 0x6070a0, 100000);
            PlaySE(won ? SE_EXP_MAX : SE_FAILURE);
        }
        if (pressed & KEY_A && B.frame - B.over > 40)
        {
            PlaySE(SE_SELECT);
            return YB_LEAVE;
        }
        return YB_NONE;
    }
    /* A new question of ours: the input starts over */
    if (MyQuestion())
    {
        uint32_t stamp = B.d->s.eventSeq * 131u + B.d->answerCount * 7u + q->type + q->count * 3u + q->prompt;
        if (B.mode == M_WATCH || stamp != B.askedStamp)
        {
            B.askedStamp = stamp;
            NewQuestion();
            if (B.tipShown)
                return YB_NONE;
        }
    }
    else if (B.mode != M_PILE && B.mode != M_PHASE && B.mode != M_CONFIRM)
        B.mode = M_WATCH;
    BuildSpots();
    if (B.toActionable)
    {
        /* a new Main / Battle Phase question: the cursor on a card that can do something (the hand first) */
        int i, k, found = -1, curIndex = FindSpot(B.cursorKind, B.cursorSide, B.cursorIndex);
        B.toActionable = 0;
        for (i = 0; i < B.spotCount && found < 0; i++)
            for (k = 0; k < q->count; k++)
                if (B.spots[i].obj < B.objCount && q->items[k].obj == B.spots[i].obj)
                {
                    if (i == curIndex)
                        found = i;
                    break;
                }
        for (i = 0; i < B.spotCount && found < 0; i++)
            if (B.spots[i].kind == SP_HAND || B.spots[i].kind == SP_MZONE)
                for (k = 0; k < q->count; k++)
                    if (B.spots[i].obj < B.objCount && q->items[k].obj == B.spots[i].obj &&
                        (found < 0 || B.spots[i].kind == SP_HAND))
                    {
                        found = i;
                        break;
                    }
        if (found >= 0)
            SetCursor(found);
    }
    cur = CurrentSpot();
    if (cur >= 0 && B.mode == M_PICK_FIELD && B.cursorKind < 0)
        cur = 0, SetCursor(0);
    switch (B.mode)
    {
    case M_WATCH:
    case M_FREE:
        if (pressed & (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT))
            MoveCursor(pressed);
        cur = CurrentSpot();
        if (pressed & KEY_R)
        {
            unsigned id = SpotCard(cur);
            if (id)
            {
                *card = id;
                PlaySE(SE_SELECT);
                return YB_INSPECT;
            }
        }
        if (pressed & KEY_START)
        {
            PlaySE(SE_SELECT);
            OpenPhaseMenu();
            return YB_NONE;
        }
        if (pressed & KEY_A && cur >= 0)
        {
            const struct Spot *s = &B.spots[cur];
            if (s->kind == SP_PILE)
            {
                uint8_t list[YGO_OBJS];
                if (s->index != PILE_DECK && !(s->index == PILE_EXTRA && s->side) && PileList(s->side, s->index, list))
                {
                    PlaySE(SE_SELECT);
                    OpenPile(s->side, s->index);
                }
                else
                    PlaySE(SE_FAILURE);
                return YB_NONE;
            }
            if (B.mode == M_FREE && s->obj < B.objCount && OpenMenu(s->obj))
            {
                PlaySE(SE_SELECT);
                return YB_NONE;
            }
            if (SpotCard(cur))
            {
                *card = SpotCard(cur);
                PlaySE(SE_SELECT);
                return YB_INSPECT;
            }
        }
        return YB_NONE;
    case M_MENU:
        if (pressed & KEY_UP)
            B.menuCursor = (B.menuCursor + B.menuCount - 1) % B.menuCount, PlaySE(SE_SELECT);
        if (pressed & KEY_DOWN)
            B.menuCursor = (B.menuCursor + 1) % B.menuCount, PlaySE(SE_SELECT);
        if (pressed & KEY_B)
        {
            B.mode = M_FREE;
            PlaySE(SE_SELECT);
            return YB_NONE;
        }
        if (pressed & KEY_R && B.menuObj < (int)B.objCount)
        {
            *card = T()->objs[B.menuObj].card;
            return YB_INSPECT;
        }
        if (pressed & KEY_A)
        {
            PlaySE(SE_SELECT);
            B.mode = M_WATCH;
            return Answer1(answer, B.menuItems[B.menuCursor]);
        }
        return YB_NONE;
    case M_PHASE:
        if (pressed & KEY_UP)
            B.menuCursor = (B.menuCursor + B.menuCount - 1) % B.menuCount, PlaySE(SE_SELECT);
        if (pressed & KEY_DOWN)
            B.menuCursor = (B.menuCursor + 1) % B.menuCount, PlaySE(SE_SELECT);
        if (pressed & (KEY_B | KEY_START))
        {
            B.mode = MyQuestion() && (q->type == YGO_Q_IDLE || q->type == YGO_Q_BATTLE) ? M_FREE : M_WATCH;
            PlaySE(SE_SELECT);
            return YB_NONE;
        }
        if (pressed & KEY_A)
        {
            int item = B.menuItems[B.menuCursor];
            PlaySE(SE_SELECT);
            if (item == -2)
            {
                B.mode = M_CONFIRM;
                B.windowCursor = 1;
                return YB_NONE;
            }
            if (!MyQuestion())
                return YB_NONE;
            B.mode = M_WATCH;
            return Answer1(answer, item);
        }
        return YB_NONE;
    case M_CONFIRM:
        if (pressed & (KEY_UP | KEY_DOWN))
            B.windowCursor ^= 1, PlaySE(SE_SELECT);
        if (pressed & KEY_B || (pressed & KEY_A && B.windowCursor))
        {
            B.mode = M_WATCH;
            PlaySE(SE_SELECT);
            return YB_NONE;
        }
        if (pressed & KEY_A)
        {
            B.mode = M_WATCH;
            return YB_CONCEDE;
        }
        return YB_NONE;
    case M_PICK_FIELD:
        if (pressed & (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT))
            MoveCursor(pressed);
        cur = CurrentSpot();
        if (pressed & KEY_R && SpotCard(cur))
        {
            *card = SpotCard(cur);
            return YB_INSPECT;
        }
        if (pressed & KEY_A && cur >= 0 && B.spots[cur].item >= 0)
        {
            if (TogglePick(B.spots[cur].item, answer))
            {
                B.mode = M_WATCH;
                return YB_ANSWER;
            }
            return YB_NONE;
        }
        if (pressed & KEY_START && q->max > 1 && ConfirmPicks(answer))
        {
            B.mode = M_WATCH;
            return YB_ANSWER;
        }
        if (pressed & KEY_B && q->min == 0 && q->type == YGO_Q_CARDS)
        {
            PlaySE(SE_SELECT);
            B.mode = M_WATCH;
            if (q->max > 1 && B.pickedCount)
                return ConfirmPicks(answer) ? YB_ANSWER : YB_NONE;
            return Answer1(answer, -1);
        }
        return YB_NONE;
    case M_PICK_GRID:
    case M_PILE:
    {
        int count = B.mode == M_PILE ? B.pileCount : (int)q->count, cols = 7;
        if (count <= 0)
        {
            if (pressed & (KEY_A | KEY_B))
                B.mode = M_WATCH;
            return YB_NONE;
        }
        if (pressed & KEY_LEFT && B.gridCursor > 0)
            B.gridCursor--, PlaySE(SE_SELECT);
        if (pressed & KEY_RIGHT && B.gridCursor < count - 1)
            B.gridCursor++, PlaySE(SE_SELECT);
        if (pressed & KEY_UP && B.gridCursor >= cols)
            B.gridCursor -= cols, PlaySE(SE_SELECT);
        if (pressed & KEY_DOWN && B.gridCursor + cols < count)
            B.gridCursor += cols, PlaySE(SE_SELECT);
        if (B.mode == M_PILE)
        {
            if (pressed & (KEY_A | KEY_R))
            {
                *card = T()->objs[B.pileList[B.gridCursor]].card;
                return YB_INSPECT;
            }
            if (pressed & KEY_B)
            {
                B.mode = M_WATCH;
                PlaySE(SE_SELECT);
            }
            return YB_NONE;
        }
        if (pressed & KEY_R)
        {
            unsigned o = q->items[B.gridCursor].obj;
            if (o < B.objCount)
            {
                *card = T()->objs[o].card;
                return YB_INSPECT;
            }
        }
        if (pressed & KEY_A)
        {
            if (TogglePick(B.gridCursor, answer))
            {
                B.mode = M_WATCH;
                return YB_ANSWER;
            }
            return YB_NONE;
        }
        if (pressed & KEY_START && q->max > 1 && ConfirmPicks(answer))
        {
            B.mode = M_WATCH;
            return YB_ANSWER;
        }
        if (pressed & KEY_B && q->min == 0)
        {
            PlaySE(SE_SELECT);
            B.mode = M_WATCH;
            if (q->max > 1 && B.pickedCount)
                return ConfirmPicks(answer) ? YB_ANSWER : YB_NONE;
            return Answer1(answer, -1);
        }
        return YB_NONE;
    }
    case M_WINDOW:
        if (!B.windowCount)
            return YB_NONE;
        if (pressed & KEY_UP)
            B.windowCursor = (B.windowCursor + B.windowCount - 1) % B.windowCount, PlaySE(SE_SELECT);
        if (pressed & KEY_DOWN)
            B.windowCursor = (B.windowCursor + 1) % B.windowCount, PlaySE(SE_SELECT);
        if (pressed & KEY_R)
        {
            int item = B.windowItems[B.windowCursor];
            unsigned o = item >= 0 ? q->items[item].obj : NONE;
            unsigned id = o < B.objCount ? T()->objs[o].card : q->card;
            if (id)
            {
                *card = id;
                return YB_INSPECT;
            }
        }
        if (pressed & KEY_B && q->type == YGO_Q_CHAIN)
        {
            PlaySE(SE_SELECT);
            B.mode = M_WATCH;
            return Answer1(answer, -1);
        }
        if (pressed & KEY_B && q->type == YGO_Q_YESNO)
        {
            PlaySE(SE_SELECT);
            B.mode = M_WATCH;
            return Answer1(answer, 0);
        }
        if (pressed & KEY_A)
        {
            PlaySE(SE_SELECT);
            B.mode = M_WATCH;
            return Answer1(answer, B.windowItems[B.windowCursor]);
        }
        return YB_NONE;
    default:
        return YB_NONE;
    }
}

/* ------------------------------------------------------------------ */
/* Drawing                                                              */
/* ------------------------------------------------------------------ */
static void Mat(void)
{
    int y, side, i;
    for (y = 0; y < 160; y++)
    {
        unsigned top = 0x2a1c30, mid = 0x1c2434, bottom = 0x18283c;
        unsigned c = y < 66 ? G_Mix(top, mid, y / 66.0f) : G_Mix(mid, bottom, (y - 66) / 94.0f);
        G_FillA(0, y, 240, 1, c, 255);
    }
    G_FillA(0, GAP_Y, 240, 22, 0x0c1018, 150);
    G_FillA(0, GAP_Y, 240, 1, 0x6070a0, 120);
    G_FillA(0, GAP_Y + 21, 240, 1, 0x6070a0, 120);
    for (side = 0; side < 2; side++)
    {
        unsigned line = side ? 0xc07888 : 0x7898c8;
        for (i = 0; i < 5; i++)
        {
            struct Rect m = ZoneRect((unsigned)side, 0, (unsigned)i), s = ZoneRect((unsigned)side, 1, (unsigned)i);
            G_RoundFrame(m.x - 3, m.y, m.w + 6, m.h, line, 70);
            G_RoundFrame(s.x - 1, s.y, s.w + 2, s.h, line, 55);
        }
        {
            struct Rect f = ZoneRect((unsigned)side, 0, YGO_FIELD_ZONE);
            G_RoundFrame(f.x - 1, f.y, f.w + 2, f.h, 0x70c090, 70);
        }
        for (i = 0; i < PILES; i++)
        {
            struct Rect p = PileRect((unsigned)side, (unsigned)i);
            G_RoundFrame(p.x - 1, p.y, p.w + 2, p.h, i == PILE_GRAVE ? 0x9090a0 : i == PILE_EXTRA ? 0xa070d0 : line, 55);
        }
    }
}
/* A card of the table, as this screen's player sees it */
static void DrawObj(unsigned o, float x, float y, float w, float h, float angle, unsigned alpha, unsigned flash)
{
    const struct Shown *sh = &B.shown[o];
    unsigned id = T()->objs[o].card;
    int onField = sh->loc == YGO_LOC_MZONE || sh->loc == YGO_LOC_SZONE;
    if (onField && !sh->faceUp)
    {
        if (T()->objs[o].controller == B.me)
        {
            /* your own Set card: its face under the back */
            Ygo_DrawCard(id, x, y, w, h, angle, alpha, flash);
            Ygo_DrawCard(0, x, y, w, h, angle, alpha * 150 / 255, 0);
        }
        else
            Ygo_DrawCard(0, x, y, w, h, angle, alpha, flash);
        return;
    }
    Ygo_DrawCard(SeesFace(o) ? id : 0, x, y, w, h, angle, alpha, flash);
}
static void Piles(void)
{
    unsigned side, p;
    char line[16];
    for (side = 0; side < 2; side++)
        for (p = 0; p < PILES; p++)
        {
            uint8_t list[YGO_OBJS];
            unsigned n = PileList(side, p, list), i, shown = 0;
            struct Rect r = PileRect(side, p);
            for (i = 0; i < n; i++)
                shown += Settled(list[i]);
            if (!shown)
                continue;
            /* the stack's edge, then the top card */
            G_FillA(r.x + 1, r.y + 2, r.w, r.h, 0x000000, 90);
            if (shown > 8)
                G_FillA(r.x + 1, r.y + 1, r.w, r.h, 0x403020, 255);
            for (i = n; i-- > 0;)
                if (Settled(list[i]))
                {
                    unsigned o = list[i];
                    Ygo_DrawCard(p == PILE_DECK || !SeesFace(o) ? 0 : T()->objs[o].card, (float)r.x, (float)r.y,
                                 (float)r.w, (float)r.h, 0, 255, 0);
                    break;
                }
            snprintf(line, sizeof(line), "%u", n);
            G_RoundFill(r.x + r.w - G_DigitsWidth(line, 0, 1) - 3, r.y + r.h - 8, G_DigitsWidth(line, 0, 1) + 4, 8,
                        0x101010, 200);
            G_Digits(r.x + r.w - G_DigitsWidth(line, 0, 1) - 1, r.y + r.h - 7, line, 0, 1, 0xf0f0f0, 255);
        }
}
static void Stats(unsigned o, struct Rect r)
{
    const struct YgoCard *c = ObjCard(o);
    const struct Shown *sh = &B.shown[o];
    char line[12];
    int v, base;
    unsigned color;
    if (!c || !Ygo_IsMonster(c) || sh->loc != YGO_LOC_MZONE || !sh->faceUp || !Settled(o))
        return;
    v = sh->attackPos ? YgoDuel_Atk(T(), o) : YgoDuel_Def(T(), o);
    base = sh->attackPos ? c->atk : c->def;
    color = v > base ? 0x80ff90 : v < base ? 0xff8070 : 0xf8f8f8;
    snprintf(line, sizeof(line), "%d", v < 0 ? 0 : v);
    if (sh->attackPos)
    {
        int w = G_DigitsWidth(line, 0, 1);
        G_RoundFill(r.x + r.w / 2 - w / 2 - 2, r.y + r.h - 8, w + 4, 8, 0x401010, 210);
        G_Digits(r.x + r.w / 2 - w / 2, r.y + r.h - 7, line, 0, 1, color, 255);
    }
    else
    {
        int w = G_DigitsWidth(line, 0, 1), cx = r.x + r.w / 2, cy = r.y + r.h / 2;
        G_RoundFill(cx - w / 2 - 2, cy + 3, w + 4, 8, 0x102040, 210);
        G_Digits(cx - w / 2, cy + 4, line, 0, 1, color, 255);
    }
}
static unsigned FlashOf(unsigned o)
{
    unsigned i, best = 0;
    for (i = 0; i < FX_MAX; i++)
        if (B.fx[i].life && B.fx[i].kind == FX_FLASH && B.fx[i].obj == o)
        {
            const struct Fx *f = &B.fx[i];
            unsigned v = (unsigned)(200 * (1.0f - (float)f->t / f->life));
            if (v > best)
                best = v;
        }
    return best;
}
static void Cards(void)
{
    unsigned o, pass;
    uint8_t hand[YGO_HAND_MAX];
    unsigned n = HandList(0, hand), i;
    int cur = CurrentSpot();
    /* the table first, then cards on the move above it, the hand last */
    for (pass = 0; pass < 2; pass++)
        for (o = 0; o < B.objCount; o++)
        {
            const struct CardSprite *sp = &B.spr[o];
            int moving = !Settled(o), field = B.shown[o].loc == YGO_LOC_MZONE || B.shown[o].loc == YGO_LOC_SZONE;
            int attackLunge = 0;
            float x = sp->x, y = sp->y;
            if (B.shown[o].loc == YGO_LOC_HAND && SideOf(B.shown[o].controller) == 0 && !moving)
                continue;
            if ((int)pass != moving)
                continue;
            if (!moving && !field && B.shown[o].loc != YGO_LOC_HAND)
                continue;   /* in a pile: Piles draws the top */
            for (i = 0; i < FX_MAX; i++)
                if (B.fx[i].life && B.fx[i].kind == FX_ATTACK && B.fx[i].obj == o)
                {
                    float t = (float)B.fx[i].t / B.fx[i].life, k = t < 0.5f ? t * 2 : (1 - t) * 2;
                    x += (B.fx[i].x2 - B.fx[i].x) * 0.35f * k;
                    y += (B.fx[i].y2 - B.fx[i].y) * 0.35f * k;
                    attackLunge = 1;
                }
            if (moving || attackLunge)
                G_FillA((int)x + 2, (int)y + 3, (int)sp->w, (int)sp->h, 0x000000, 70);
            DrawObj(o, x, y, sp->w, sp->h, sp->angle, 255, FlashOf(o));
            if (field && !moving)
            {
                float a;
                struct Rect r = Home(o, &a);
                if (a == 90)
                    r = R(r.x - 3, r.y + 3, r.h, r.w);
                Stats(o, r);
            }
        }
    /* the hand: the card under the cursor a little higher */
    for (i = 0; i < n; i++)
    {
        unsigned obj = hand[i];
        const struct CardSprite *sp = &B.spr[obj];
        int on = cur >= 0 && B.spots[cur].kind == SP_HAND && B.spots[cur].obj == obj;
        if (!Settled(obj))
            continue;
        DrawObj(obj, sp->x, sp->y - (on ? 4 : 0), sp->w, sp->h, 0, 255, FlashOf(obj));
    }
    for (o = 0; o < B.objCount; o++)
        if (B.shown[o].loc == YGO_LOC_HAND && SideOf(B.shown[o].controller) == 0 && !Settled(o))
        {
            const struct CardSprite *sp = &B.spr[o];
            DrawObj(o, sp->x, sp->y, sp->w, sp->h, 0, 255, 0);
        }
}
static void Lp(void)
{
    unsigned side;
    char line[16];
    for (side = 0; side < 2; side++)
    {
        struct Rect r = LpRect(side);
        int lp = B.lp[side], fill = lp >= YGO_LP ? r.w - 4 : lp * (r.w - 4) / YGO_LP;
        unsigned color = lp > 4000 ? 0x48c860 : lp > 2000 ? 0xe0c040 : 0xe05040;
        G_RoundFill(r.x, r.y, r.w, r.h, side ? 0x401820 : 0x182848, 230);
        G_RoundFrame(r.x, r.y, r.w, r.h, side ? 0xc06070 : 0x6080c8, 255);
        G_Text(r.x + 3, r.y - 1, side ? 0xf0c0c8 : 0xc0d0f8, G_NO_SHADOW, "LP", 0, TF_NARROW);
        snprintf(line, sizeof(line), "%d", lp < 0 ? 0 : lp);
        G_Digits(r.x + r.w - 2 - G_DigitsWidth(line, 0, 1), r.y + 11, line, 0, 1, 0xf8f8f8, 255);
        G_FillA(r.x + 2, r.y + r.h - 5, r.w - 4, 3, 0x000000, 160);
        if (fill > 0)
            G_FillA(r.x + 2, r.y + r.h - 5, fill, 3, color, 255);
    }
    /* the opponent's hand: small backs */
    {
        uint8_t list[YGO_HAND_MAX];
        unsigned n = HandList(1, list);
        if (n)
        {
            snprintf(line, sizeof(line), "%u", n);
            G_Digits(OUTR + 13 - G_DigitsWidth(line, 0, 1) / 2, ROW_OPP_M + 18, line, 0, 1, 0xd0d0e0, 255);
        }
    }
}
static void Chain(void)
{
    const struct YgoState *s = T();
    unsigned i;
    char line[8];
    if (!YgoBoard_Idle())
        return;
    for (i = 0; i < s->chainCount; i++)
    {
        unsigned o = s->chain[i].obj;
        int x, y;
        if (o >= B.objCount || !ObjCenter(o, &x, &y))
            continue;
        snprintf(line, sizeof(line), "%u", i + 1);
        G_RoundFill(x - 5, y - 6, 10, 11, 0xf0a020, 240);
        G_RoundFrame(x - 5, y - 6, 10, 11, 0x603000, 255);
        G_Digits(x - G_DigitsWidth(line, 0, 1) / 2, y - 3, line, 0, 1, 0x301800, 255);
    }
}
static void Gap(void)
{
    static const char *const phases[] = {"DP", "SP", "M1", "BP", "M2", "EP"};
    const struct YgoQuery *q = &B.d->query;
    char line[128], prompt[64];
    int i, x = 152, cur = CurrentSpot();
    unsigned id = 0;
    /* the phases */
    for (i = 0; i < 6; i++)
    {
        int on = i == B.phase;
        unsigned c = on ? (B.turnPlayer == B.me ? 0x60a0ff : 0xff7070) : 0x606878;
        if (on)
            G_RoundFill(x - 1, GAP_Y + 2, 14, 9, c, 200);
        G_Text(x + 1, GAP_Y, on ? G_WHITE : c, G_NO_SHADOW, phases[i], 0, TF_NARROW);
        x += 14;
    }
    /* line 1: the question, what just happened, or the card under the cursor; line 2: about that card */
    {
        const char *first = NULL;
        unsigned firstColor = 0xf8e070;
        if (B.mode == M_PICK_FIELD || B.mode == M_PICK_GRID)
            first = PromptText(q, prompt, sizeof(prompt));
        else if (B.mode == M_WATCH && !MyQuestion() && B.log[0])
            first = B.log, firstColor = 0xb8c0d0;
        if (B.mode == M_PICK_GRID || B.mode == M_PILE)
        {
            unsigned o = B.mode == M_PILE ? B.pileList[B.gridCursor] : q->items[B.gridCursor].obj;
            id = o < B.objCount ? T()->objs[o].card : 0;
        }
        else if (B.mode == M_MENU && B.menuObj < (int)B.objCount)
            id = T()->objs[B.menuObj].card;
        else
            id = SpotCard(cur);
        line[0] = 0;
        if (id)
        {
            const struct YgoCard *c = Ygo_Card(id);
            char stats[64];
            if (Ygo_IsMonster(c))
            {
                int atk = c->atk, def = c->def;
                if (cur >= 0 && B.mode != M_PICK_GRID && B.mode != M_PILE && B.spots[cur].obj < B.objCount &&
                    B.shown[B.spots[cur].obj].loc == YGO_LOC_MZONE && B.shown[B.spots[cur].obj].faceUp)
                    atk = YgoDuel_Atk(T(), B.spots[cur].obj), def = YgoDuel_Def(T(), B.spots[cur].obj);
                snprintf(stats, sizeof(stats), "LV%u  ATK %d  DEF %d", c->level, atk < 0 ? 0 : atk, def < 0 ? 0 : def);
            }
            else
                snprintf(stats, sizeof(stats), "%s", c->frame == YGO_SPELL ? "SPELL CARD" : "TRAP CARD");
            if (first)
                snprintf(line, sizeof(line), "%s  %s", c->name, stats);
            else
            {
                first = c->name;
                firstColor = G_WHITE;
                snprintf(line, sizeof(line), "%s", stats);
            }
        }
        else if (cur >= 0 && B.spots[cur].kind == SP_PILE && B.mode != M_PICK_GRID && B.mode != M_PILE)
        {
            static const char *const names[] = {"DECK", "GRAVEYARD", "FUSION DECK", "BANISHED"};
            uint8_t list[YGO_OBJS];
            unsigned n = PileList(B.spots[cur].side, B.spots[cur].index, list);
            snprintf(line, sizeof(line), "%s %s: %u card%s", B.spots[cur].side ? "Their" : "Your",
                     names[B.spots[cur].index], n, n == 1 ? "" : "s");
        }
        else if (cur >= 0 && B.spots[cur].kind == SP_DIRECT)
            snprintf(line, sizeof(line), "A direct attack on their Life Points");
        if (first)
            Line(4, GAP_Y, firstColor, first, 146);
        if (line[0])
            Line(4, GAP_Y + 10, 0xc8d0e0, line, 112);
    }
    /* hints */
    {
        const char *hint = "";
        switch (B.mode)
        {
        case M_FREE:
            hint = "{A}ACT {R}READ {START}MENU";
            break;
        case M_WATCH:
            hint = B.waiting ? "Waiting..." : "{R}READ {START}MENU";
            break;
        case M_PICK_FIELD:
        case M_PICK_GRID:
            hint = q->max > 1 ? "{A}PICK {START}OK" : q->min ? "{A}CHOOSE {R}READ" : "{A}CHOOSE {B}NONE";
            break;
        case M_PILE:
            hint = "{A}READ {B}BACK";
            break;
        default:
            hint = "{A}OK {R}READ";
            break;
        }
        G_Hint(238 - G_Hint(0, 0, 0, 0, hint, TF_NARROW, 0), GAP_Y + 11, 0xc0c8e0, G_NO_SHADOW, hint, TF_NARROW, 1);
    }
}
static void Highlights(void)
{
    int i, cur = CurrentSpot();
    unsigned pulse = (unsigned)(B.frame / 8) % 2;
    /* what can be chosen, glowing; picks numbered */
    if (B.mode == M_PICK_FIELD)
        for (i = 0; i < B.spotCount; i++)
        {
            const struct Spot *s = &B.spots[i];
            int k = s->item >= 0 ? Picked(s->item) : 0;
            G_Frame(s->r.x - 1, s->r.y - 1, s->r.w + 2, s->r.h + 2, k ? 0x40a0ff : 0xf0d040, 200);
            if (k)
            {
                char line[4];
                snprintf(line, sizeof(line), "%d", k);
                G_RoundFill(s->r.x - 2, s->r.y - 2, 8, 9, 0x2060c0, 255);
                G_Digits(s->r.x, s->r.y, line, 0, 1, 0xffffff, 255);
            }
        }
    if (B.mode == M_FREE && MyQuestion())
    {
        /* the cards that can do something now */
        const struct YgoQuery *q = &B.d->query;
        for (i = 0; i < B.spotCount; i++)
        {
            unsigned k;
            const struct Spot *s = &B.spots[i];
            if (s->obj >= B.objCount)
                continue;
            for (k = 0; k < q->count; k++)
                if (q->items[k].obj == s->obj)
                {
                    int y = s->r.y - (s->kind == SP_HAND && i == cur ? 4 : 0);
                    G_Frame(s->r.x - 1, y - 1, s->r.w + 2, s->r.h + 2, 0x60d0ff, 130 + 60 * pulse);
                    break;
                }
        }
    }
    if (cur >= 0 && (B.mode == M_FREE || B.mode == M_WATCH || B.mode == M_PICK_FIELD || B.mode == M_MENU))
    {
        const struct Spot *s = &B.spots[cur];
        int y = s->r.y - (s->kind == SP_HAND ? 4 : 0), o = (int)pulse;
        G_Frame(s->r.x - 2 - o, y - 2 - o, s->r.w + 4 + 2 * o, s->r.h + 4 + 2 * o, 0xe82820, 255);
    }
}
static void Fxs(void)
{
    unsigned i;
    for (i = 0; i < FX_MAX; i++)
    {
        const struct Fx *f = &B.fx[i];
        float t;
        if (!f->life)
            continue;
        t = (float)f->t / f->life;
        switch (f->kind)
        {
        case FX_BANNER:
        {
            float in = f->life >= 10000 ? (f->t < 10 ? f->t / 10.0f : 1)
                       : t < 0.15f                 ? t / 0.15f
                       : t > 0.85f                 ? (1 - t) / 0.15f
                                                   : 1;
            G_Banner(f->x, f->y, f->text, f->color, in, (unsigned)(255 * (in > 1 ? 1 : in)));
            break;
        }
        case FX_BIGCARD:
        {
            float in = t < 0.2f ? t / 0.2f : t > 0.8f ? (1 - t) / 0.2f : 1, w = 44 * (0.6f + 0.4f * in), h = 61 * (0.6f + 0.4f * in);
            G_FillA(0, 0, 240, 160, 0x000000, (unsigned)(90 * in));
            G_FillA((int)(f->x - w / 2) + 2, (int)(f->y - h / 2) + 3, (int)w, (int)h, 0x000000, (unsigned)(90 * in));
            Ygo_DrawCard(f->card, f->x - w / 2, f->y - h / 2, w, h, 0, (unsigned)(255 * in), t < 0.3f ? (unsigned)(160 * (1 - t / 0.3f)) : 0);
            if (f->text[0])
                G_Banner(f->x, (int)(f->y + h / 2 + 8), f->text, f->color, in, (unsigned)(255 * in));
            break;
        }
        case FX_NUMBER:
        {
            char line[16];
            snprintf(line, sizeof(line), "%+d", f->value);
            G_DigitsOutlined(f->x - G_DigitsWidth(line, 0, 1) / 2, f->y - (int)(t * 10), line, 0, 1, f->color, 0x101010,
                             (unsigned)(255 * (t > 0.7f ? (1 - t) / 0.3f : 1)));
            break;
        }
        case FX_ATTACK:
        {
            unsigned a = (unsigned)(200 * (1 - t));
            G_Line(f->x, f->y, f->x2, f->y2, 0xffd040, a);
            G_Line(f->x + 1, f->y, f->x2 + 1, f->y2, 0xff8020, a);
            break;
        }
        case FX_COIN:
            G_FillA(0, 0, 240, 160, 0x000000, 70);
            G_Coin(f->x, f->y, t < 0.6f ? t * 6 : 3.6f, f->value, 255);
            if (t > 0.6f)
                G_Banner(f->x, f->y + 22, f->value ? "HEADS" : "TAILS", 0xe0b030, 1, 255);
            break;
        case FX_DICE:
        {
            char line[4];
            int face = t < 0.6f ? 1 + (f->t / 3) % 6 : f->value;
            G_FillA(0, 0, 240, 160, 0x000000, 70);
            G_RoundFill(f->x - 10, f->y - 10, 20, 20, 0xf8f8f8, 255);
            G_RoundFrame(f->x - 10, f->y - 10, 20, 20, 0x404040, 255);
            snprintf(line, sizeof(line), "%d", face);
            G_TextAlign(f->x, f->y - 8, TA_CENTER, G_INK, G_NO_SHADOW, line, TF_NORMAL);
            break;
        }
        case FX_RETICLE:
        case FX_NEGATE:
        {
            int x, y;
            if (f->obj >= B.objCount || !ObjCenter(f->obj, &x, &y))
                break;
            if (f->kind == FX_RETICLE)
            {
                int r = 14 - (int)(t * 6);
                G_Frame(x - r, y - r, 2 * r, 2 * r, 0xff4040, 230);
                G_Line(x - r - 3, y, x - r + 4, y, 0xff4040, 230);
                G_Line(x + r - 4, y, x + r + 3, y, 0xff4040, 230);
            }
            else
            {
                G_Line(x - 9, y - 12, x + 9, y + 12, 0xff3030, 255);
                G_Line(x - 8, y - 12, x + 10, y + 12, 0xff3030, 255);
                G_Line(x + 9, y - 12, x - 9, y + 12, 0xff3030, 255);
                G_Line(x + 10, y - 12, x - 8, y + 12, 0xff3030, 255);
            }
            break;
        }
        case FX_SHUFFLE:
        {
            int dx = (int)(SDL_sinf(t * 18) * 4);
            G_FillA(f->x + dx, f->y, CW, CH, 0x6a3a14, 160);
            break;
        }
        default:
            break;
        }
    }
}
static void WindowBox(int x, int y, int w, const char *title, const char *const *rows, int count, int cursor,
                      const unsigned *gray)
{
    int h = count * 14 + (title ? 14 : 0) + 8, i, ty = y + 4;
    G_Window(x, y, w, h);
    if (title)
    {
        G_Text(x + 8, ty - 1, 0x506070, G_NO_SHADOW, title, w - 12, TF_SMALL);
        ty += 14;
    }
    for (i = 0; i < count; i++)
    {
        if (i == cursor)
            G_Cursor(x + 6, ty + i * 14 + 4);
        G_Text(x + 14, ty + i * 14 - 2, gray && gray[i] ? 0xa0a0a0 : G_INK, G_INK_SHADOW, rows[i], w - 18, TF_NORMAL);
    }
}
static void Menus(void)
{
    const struct YgoQuery *q = &B.d->query;
    const char *rows[YGO_QUERY_MAX + 2];
    char names[YGO_QUERY_MAX + 2][40];
    int i;
    switch (B.mode)
    {
    case M_MENU:
    case M_PHASE:
    {
        int w = 96, x, y, cur = CurrentSpot();
        for (i = 0; i < B.menuCount; i++)
            rows[i] = B.menuItems[i] == -2 ? "CONCEDE" : KindLabel(&q->items[B.menuItems[i]]);
        x = 140;
        y = 160 - (B.menuCount * 14 + 8) - 34;
        if (B.mode == M_MENU && cur >= 0)
        {
            const struct Spot *s = &B.spots[cur];
            x = s->r.x + s->r.w + 4;
            if (x + w > 238)
                x = s->r.x - w - 4;
            if (x < 2)
                x = 2;
            y = s->r.y + s->r.h / 2 - (B.menuCount * 14 + 8) / 2;
            if (y + B.menuCount * 14 + 8 > 158)
                y = 158 - (B.menuCount * 14 + 8);
            if (y < 2)
                y = 2;
        }
        WindowBox(x, y, w, NULL, rows, B.menuCount, B.menuCursor, NULL);
        break;
    }
    case M_CONFIRM:
    {
        static const char *const yesNo[] = {"YES", "NO"};
        G_Window(0, 112, 240, 48);
        G_Text(8, 116, G_INK, G_INK_SHADOW, "Concede this duel? It counts as a loss.", 168, TF_NORMAL);
        WindowBox(184, 74, 56, NULL, yesNo, 2, B.windowCursor, NULL);
        break;
    }
    case M_WINDOW:
    {
        char title[64];
        const struct YgoCard *asking = q->card ? Ygo_Card(q->card) : NULL;
        int w = 132;
        for (i = 0; i < B.windowCount; i++)
        {
            int item = B.windowItems[i];
            rows[i] = names[i];
            if (item < 0)
                snprintf(names[i], sizeof(names[i]), "PASS");
            else if (q->type == YGO_Q_CHAIN)
            {
                const struct YgoCard *c = ObjCard(q->items[item].obj);
                snprintf(names[i], sizeof(names[i]), "%s", c ? c->name : "?");
            }
            else if (q->type == YGO_Q_YESNO)
                snprintf(names[i], sizeof(names[i]), "%s", item ? "YES" : "NO");
            else if (q->type == YGO_Q_POSITION)
                snprintf(names[i], sizeof(names[i]), "%s",
                         q->items[item].value == 0 ? "ATTACK POSITION" : q->items[item].value == 1 ? "DEFENSE POSITION"
                                                                                                   : "SET (FACE-DOWN)");
            else if (q->prompt == YGO_P_COIN_CALL)
                snprintf(names[i], sizeof(names[i]), "%s", q->items[item].value ? "HEADS" : "TAILS");
            else if (q->prompt == YGO_P_SUMMON_OR_SET)
                snprintf(names[i], sizeof(names[i]), "%s", q->items[item].value ? "SET" : "SUMMON");
            else
                snprintf(names[i], sizeof(names[i]), "%u", q->items[item].value);
        }
        if (q->type == YGO_Q_CHAIN)
            snprintf(title, sizeof(title), "Activate a card?");
        else if (q->type == YGO_Q_YESNO)
            snprintf(title, sizeof(title), "Use %s?", asking ? asking->name : "the effect");
        else if (q->type == YGO_Q_POSITION)
            snprintf(title, sizeof(title), "%s:", asking ? asking->name : "Position");
        else if (q->prompt == YGO_P_COIN_CALL)
            snprintf(title, sizeof(title), "Call the coin toss!");
        else
            snprintf(title, sizeof(title), "%s", asking ? asking->name : "Choose");
        WindowBox(240 - w - 4, GAP_Y - (B.windowCount * 14 + 22) / 2 + 11, w, title, rows, B.windowCount,
                  B.windowCursor, NULL);
        break;
    }
    default:
        break;
    }
}
static void Grid(void)
{
    const struct YgoQuery *q = &B.d->query;
    int count = B.mode == M_PILE ? B.pileCount : (int)q->count, cols = 7, page = 14, start, i;
    char title[64], prompt[64];
    if (B.mode != M_PILE && B.mode != M_PICK_GRID)
        return;
    G_FillA(0, 0, 240, 160, 0x080c14, 210);
    if (B.mode == M_PILE)
    {
        static const char *const names[] = {"DECK", "GRAVEYARD", "FUSION DECK", "BANISHED"};
        snprintf(title, sizeof(title), "%s %s", B.pileSide ? "THEIR" : "YOUR", names[B.pilePile]);
    }
    else
        snprintf(title, sizeof(title), "%s", PromptText(q, prompt, sizeof(prompt)));
    G_RoundFill(0, 0, 240, 17, 0x203040, 230);
    G_Text(8, 1, G_WHITE, 0x101820, title, 228, TF_NORMAL);
    if (count <= 0)
    {
        G_TextAlign(120, 60, TA_CENTER, G_WHITE, G_NO_SHADOW, "No cards you can see.", TF_SMALL);
        return;
    }
    start = B.gridCursor / page * page;
    for (i = 0; i < page && start + i < count; i++)
    {
        int k = start + i, x = 8 + (i % cols) * 33, y = 21 + (i / cols) * 45, on = k == B.gridCursor;
        unsigned o = B.mode == M_PILE ? B.pileList[k] : q->items[k].obj;
        int mark = B.mode == M_PICK_GRID ? Picked(k) : 0;
        if (on)
            y -= 2;
        if (o < B.objCount)
            Ygo_DrawCard(SeesFace(o) || B.mode == M_PICK_GRID ? T()->objs[o].card : 0, (float)x, (float)y, 30, 42, 0, 255, 0);
        if (mark)
        {
            char line[4];
            snprintf(line, sizeof(line), "%d", mark);
            G_Frame(x - 1, y - 1, 32, 44, 0x40a0ff, 255);
            G_RoundFill(x - 2, y - 2, 9, 10, 0x2060c0, 255);
            G_Digits(x, y, line, 0, 1, 0xffffff, 255);
        }
        if (on)
        {
            int p = (B.frame / 8) % 2;
            G_Frame(x - 2 - p, y - 2 - p, 34 + 2 * p, 46 + 2 * p, 0xe82820, 255);
        }
    }
    if (count > page)
    {
        char line[16];
        snprintf(line, sizeof(line), "%d/%d", start / page + 1, (count + page - 1) / page);
        G_TextAlign(236, 2, TA_RIGHT, G_WHITE, G_NO_SHADOW, line, TF_NARROW);
    }
    {
        unsigned o = B.mode == M_PILE ? B.pileList[B.gridCursor] : q->items[B.gridCursor].obj;
        const struct YgoCard *c = ObjCard(o);
        if (c)
        {
            char line[96];
            G_FillA(0, 112, 240, 48, 0x5848c8, 255);
            G_Text(8, 114, G_WHITE, 0x302878, c->name, 226, TF_NORMAL);
            if (Ygo_IsMonster(c))
                snprintf(line, sizeof(line), "%s  %s  LV%u  ATK %d  DEF %d", Ygo_AttributeName(c->attribute),
                         Ygo_RaceName(c->race), c->level, c->atk, c->def);
            else
                snprintf(line, sizeof(line), "%s", c->frame == YGO_SPELL ? "SPELL CARD" : "TRAP CARD");
            G_Text(8, 129, 0xd8d8f8, 0x302878, line, 226, TF_SMALL);
            G_Hint(8, 152, 0xe0e0ff, G_NO_SHADOW,
                   B.mode == M_PILE ? "{A} READ  {B} BACK"
                   : q->max > 1     ? "{A} PICK  {START} OK  {R} READ"
                   : q->min         ? "{A} CHOOSE  {R} READ"
                                    : "{A} CHOOSE  {B} NONE  {R} READ",
                   TF_NARROW, 1);
        }
    }
}
void YgoBoard_Render(void)
{
    if (!B.active || !B.d)
        return;
    Mat();
    Piles();
    Cards();
    Chain();
    Lp();
    Gap();
    Highlights();
    Fxs();
    Menus();
    Grid();
    if (B.result[0] && (B.tipShown || B.mode == M_OVER))
    {
        G_Window(0, 112, 240, 48);
        G_Text(8, 116, G_INK, G_INK_SHADOW, B.result, 214, TF_NORMAL);
        if ((B.frame / 20) % 2)
            G_ButtonIcon(226, 152, 0);
    }
}
