#include <SDL2/SDL.h> // before global.h: its macros break the Windows intrinsics headers
#include "mp_version.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "sound.h"
#include "constants/songs.h"
#include "tcg/card_ids.h"
#include "tcg/integration.h"
#include "tcg_board.h"
#include "tcg_gfx.h"

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
    KEY_L = 512,
    KEY_INSPECT = 2048   /* R2 / ZR / RT, or D on the keyboard: a card up close (gamepad.h PLATFORM_KEY_INSPECT) */
};

/* ------------------------------------------------------------------ */
/* Layout: the DS mat turned to the wide screen. You are at the bottom. */
/* ------------------------------------------------------------------ */
#define BENCH_W 22
#define BENCH_H 30
#define ACT_W 30
#define ACT_H 42
#define ACT_X 105
#define OPP_ACT_Y 33
#define MY_ACT_Y 78
#define OPP_BENCH_Y 2
#define MY_BENCH_Y 122
#define BENCH_X 34   /* the Bench row's frame: five 34-pixel spaces */
#define BENCH_CELL 34
#define PILE_W 18
#define PILE_H 25
#define PRIZE_W 16
#define PRIZE_H 22
#define HAND_W 30
#define HAND_H 42
#define CENTER_X 120
#define CENTER_Y 76
#define BAR_CLOSED 152
#define BAR_OPEN 118
#define PAN 37
#define LAYER_H 200 /* the mat is taller than the screen: it scrolls up with the hand */
#define QUEUE 8
#define FX_MAX 32

struct Rect
{
    int x, y, w, h;
};
static struct Rect MakeRect(int x, int y, int w, int h)
{
    struct Rect r = {x, y, w, h};
    return r;
}
static struct Rect BenchCell(unsigned who, unsigned column)
{
    return MakeRect(BENCH_X + (int)column * BENCH_CELL, who ? 0 : MY_BENCH_Y - 2, BENCH_CELL, BENCH_H + 4);
}
static struct Rect SlotRect(unsigned who, unsigned slot)
{
    unsigned column;
    struct Rect cell;
    if (!slot)
        return MakeRect(ACT_X, who ? OPP_ACT_Y : MY_ACT_Y, ACT_W, ACT_H);
    /* The opponent's bench fills from their left: our right. */
    column = who ? 5 - slot : slot - 1;
    cell = BenchCell(who, column);
    return MakeRect(cell.x + (BENCH_CELL - BENCH_W) / 2, who ? OPP_BENCH_Y : MY_BENCH_Y, BENCH_W, BENCH_H);
}
/* The side columns: the opponent's discard and deck at the top left, their
 * Prizes at the top right; yours mirrored at the bottom. */
static struct Rect DeckRect(unsigned who) { return who ? MakeRect(7, 32, PILE_W, PILE_H) : MakeRect(215, 96, PILE_W, PILE_H); }
static struct Rect DiscardRect(unsigned who)
{
    return who ? MakeRect(7, 3, PILE_W, PILE_H) : MakeRect(215, 125, PILE_W, PILE_H);
}
/* Prizes: three rows of two, the second card of a row tucked under the first
 * (the DS layout). Your first Prize is the top left one. */
static struct Rect PrizeArea(unsigned who) { return who ? MakeRect(209, 1, 30, 62) : MakeRect(1, 91, 30, 62); }
static struct Rect PrizeRect(unsigned who, unsigned i)
{
    struct Rect a = PrizeArea(who);
    unsigned row = i / 2, col = i % 2;
    if (who)
        row = 2 - row, col ^= 1;
    return MakeRect(a.x + 3 + (int)col * 8, a.y + 2 + (int)row * 18 + (int)col * 2, PRIZE_W, PRIZE_H);
}
static const struct Rect sOppHand = {1, 61, 30, 27};

/* ------------------------------------------------------------------ */
/* State                                                                */
/* ------------------------------------------------------------------ */
enum
{
    Z_NONE,
    Z_DECK,
    Z_HAND,
    Z_DISCARD,
    Z_PRIZE,
    Z_FIELD,
    Z_UNDER,
    Z_ATTACHED
};
struct CardSprite
{
    float x, y, w, h, angle;
    float tx, ty, tw, th, tangle;
    float flip;
    int delay, zone, slot, index, face, shownFace, visible, layer, settled;
};
enum
{
    FX_TEXT,    /* floating text (damage numbers use digits) */
    FX_NUMBER,  /* "-40" / "+20" */
    FX_BANNER,  /* WEAKNESS x2 style glass banner */
    FX_SWEEP,   /* YOUR TURN band across the mat */
    FX_COIN,    /* spinning coin */
    FX_SPARK,   /* particle */
    FX_SHOWCASE, /* a played Trainer card held up in the middle */
    FX_IMPACT,  /* an attack landing: a flash, a ring going out, rays (as the Yu-Gi-Oh! table's) */
    FX_SHARDS,  /* a Pokemon knocked out: the card breaks apart */
    FX_RISE     /* a Pokemon put into play: a ring opening and a beam */
};
struct Fx
{
    int type, t, duration, x, y, value, player;
    float vx, vy;
    unsigned color, color2;
    char text[40];
};
enum
{
    P_IDLE,
    P_PRE,
    P_COMMIT,
    P_POST
};
enum
{
    M_FREE,
    M_MENU,
    M_TARGET,
    M_PICK,
    M_ATTACK,
    M_CONFIRM,
    M_VIEW,
    M_RESULT
};
enum
{
    T_HAND,
    T_MINE,
    T_THEIRS,
    T_MY_DECK,
    T_MY_DISCARD,
    T_OPP_DECK,
    T_OPP_DISCARD,
    T_MY_PRIZES,
    T_OPP_PRIZES,
    T_OPP_HAND
};
struct Target
{
    int kind, index;
    struct Rect r;
};
enum
{
    FLOW_NONE,
    FLOW_BASIC,
    FLOW_EVOLVE,
    FLOW_ATTACH,
    FLOW_TRAINER,
    FLOW_RETREAT,
    FLOW_ATTACK,
    FLOW_POWER
};
enum
{
    ST_MY_SLOT,
    ST_OPP_SLOT,
    ST_PICK,
    ST_STAGE,
    ST_CHOICE,       /* a Pokemon Power's menu: SHIFT's types, PEEK's places */
    ST_ATTACK_OPTION /* an attack's menu (TcgDuel_AttackOptions): Amnesia, Metronome, Wildfire, Conversion... */
};
enum
{
    SRC_HAND,
    SRC_DECK,
    SRC_MY_DISCARD,
    SRC_OPP_DISCARD,
    SRC_MY_ATTACHED,
    SRC_OPP_ATTACHED,
    SRC_TOP5,
    SRC_TOP3_MINE,  /* Prophecy: the top 3 of a deck, to put in order */
    SRC_TOP3_THEIRS
};
enum
{
    F_ANY,
    F_OCCUPIED,
    F_BENCH,
    F_EMPTY_BENCH,
    F_DAMAGED,
    F_DAMAGED_ENERGY,
    F_HAS_ENERGY,
    F_EVOLVED,
    F_BREEDER_BASIC,
    F_POKEMON,
    F_BASIC,
    F_BASIC_ENERGY,
    F_ENERGY,
    F_TRAINER,
    F_STAGE2,
    F_EVOLVE_TARGET,
    F_BASIC_SLOT,
    F_ANY_POKEMON,
    /* Pokemon Powers */
    F_HAS_GRASS,
    F_OTHER,       /* occupied, not the first choice */
    F_WATER_POKEMON,
    F_ROOM_OTHER,  /* takes a damage counter without a Knock Out, not the first choice */
    F_DAMAGED_OTHER /* damaged, not the Pokemon using the power */
};
struct Step
{
    uint8_t kind, filter, source, min, max;
    const char *prompt;
};
enum
{
    MENU_PLAY,
    MENU_ATTACK,
    MENU_RETREAT,
    MENU_POWER,
    MENU_CHECK,
    MENU_LOOK,
    MENU_YES,
    MENU_NO,
    MENU_HAND,
    MENU_DONE,
    MENU_CONCEDE,
    MENU_STAGE1,
    MENU_STAGE2,
    MENU_DISCARD,
    MENU_CHOICE /* + the value (ST_CHOICE) */
};
struct MenuItem
{
    const char *label;
    int id, enabled;
    char reason[64];
};

static struct
{
    int active, online, waiting;
    char opponent[40];
    struct TcgDuel view;
    struct TcgDuel queue[QUEUE];
    unsigned qHead, qCount;
    uint32_t lastSeq;
    int phase, wait, index;
    struct TcgEvent pre[TCG_EVENT_RING], post[TCG_EVENT_RING];
    int npre, npost;
    struct CardSprite sprite[2][TCG_DECK_SIZE];
    float damage[2][TCG_FIELD_SIZE], shownDamage[2][TCG_FIELD_SIZE];
    uint8_t cond[2][TCG_FIELD_SIZE], poison[2][TCG_FIELD_SIZE];
    int shake[2][TCG_FIELD_SIZE], flash[2][TCG_FIELD_SIZE], lunge[2], ko[2][TCG_FIELD_SIZE];
    int pops[2];
    struct Fx fx[FX_MAX];
    unsigned frame;
    int intro;
    /* Interface */
    int mode, focusKind, focusIndex, lastHand, trayWanted;
    int checkDone;              /* look once the table is idle: anything left to play from the hand? */
    int refocus, refocusHand;   /* the action of ours a commit is waiting for (+1) and the hand card it played:
                                   where the cursor goes next (Refocus) */
    float tray, focusPulse;
    struct MenuItem menu[8];
    int menuCount, menuCursor, menuX, menuY;
    char confirmText[80];
    int confirmKind;
    /* What a choice is for */
    int flow, step, nsteps, heldHand, heldPhysical, attackCursor;
    unsigned flowCard;
    const struct Step *steps;
    struct TcgAction action;
    /* Card picker / viewer */
    uint8_t pickCards[TCG_DECK_SIZE], pickOwner, pickCount, pickSel[TCG_DECK_SIZE], pickOrder[TCG_DECK_SIZE];
    int pickCursor, pickScroll, pickMin, pickMax, pickSource, pickFilter, pickChosen, pickRetreat;
    char pickTitle[64];
    char toast[200];
    int toastTime;
    char result[200];
    int resultShown, resultTime;
    u16 music, duelMusic;
    int speed;
    /* Showing what happened: the hand after OAK and the like, the Bench
     * after a Knock Out, a Prize card held up before it joins the hand. */
    int showHand, promoFocused;
    /* The CARD CLUB's lesson: a tip at each first time (TIP_* bits shown) */
    int tutorial, tipsShown;
    int spotlight, spotTime, spotHold;
    uint8_t prizeQueue[6], prizeCount, prizeNext;
} B;

static const struct Step sNoSteps[1] = {{0, 0, 0, 0, 0, NULL}};
static void Toast(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    vsnprintf(B.toast, sizeof(B.toast), format, ap);
    va_end(ap);
    B.toastTime = 150;
}
static float Ease(float t)
{
    if (t <= 0)
        return 0;
    if (t >= 1)
        return 1;
    return 1 - (1 - t) * (1 - t) * (1 - t);
}
static int Fast(void) { return Tcg_GetCollection()->fastAnimations; }

/* ------------------------------------------------------------------ */
/* Rules helpers on the shown state                                     */
/* ------------------------------------------------------------------ */
static const struct TcgCard *Field(unsigned who, unsigned slot) { return TcgDuel_Field(&B.view, who, slot); }
static unsigned CardOf(unsigned who, unsigned physical) { return B.view.players[who].cards[physical]; }
static unsigned MaxHp(const struct TcgCard *c) { return !c ? 0 : c->kind == TCG_TRAINER ? 10 : c->hp; }
static int MyTurn(void)
{
    const struct TcgDuel *g = &B.view;
    if (g->phase == TCG_FINISHED)
        return 0;
    if (g->phase == TCG_PROMOTION)
        return g->promotePlayer == 0;
    return g->current == 0;
}
static int CanAct(void) { return MyTurn() && TcgBoard_Idle() && !B.waiting; }
static unsigned EnergyValue(unsigned id) { return id == TCG_CARD_DOUBLE_COLORLESS_ENERGY ? 2 : 1; }
static int HasEnergy(unsigned who, unsigned slot)
{
    const struct TcgFieldCard *f = &B.view.players[who].field[slot];
    unsigned i;
    for (i = 0; i < f->attachedSize; i++)
        if (Tcg_Card(CardOf(who, f->attached[i]))->kind == TCG_ENERGY)
            return 1;
    return 0;
}
static int AttackReady(unsigned attack, char *reason, size_t size)
{
    const struct TcgCard *c = TcgDuel_Attacker(&B.view, 0), *active = Field(0, 0);
    const struct TcgFieldCard *f = &B.view.players[0].field[0];
    if (!c || c->kind != TCG_POKEMON || !c->attacks[attack].name[0])
        return snprintf(reason, size, "No attack."), 0;
    if (c->attacks[attack].category == TCG_POKEMON_POWER)
        return snprintf(reason, size, "This is a Pokemon Power."), 0;
    if (!TcgDuel_EffectSupported(c->attacks[attack].effect))
        return snprintf(reason, size, "This attack's effect isn't ready yet."), 0;
    if (TcgDuel_EffectBlock(&B.view, 0, &c->attacks[attack]))
        return snprintf(reason, size, "%s", TcgDuel_EffectBlock(&B.view, 0, &c->attacks[attack])), 0;
    if (!TcgDuel_CanAttack(&B.view, 0, attack))
        return snprintf(reason, size, "Not enough Energy attached."), 0;
    if (f->condition == TCG_ASLEEP && !TcgDuel_SleepAttack(&c->attacks[attack]))
        return snprintf(reason, size, "%s is Asleep.", active->name), 0;
    if (f->condition == TCG_PARALYZED)
        return snprintf(reason, size, "%s is Paralyzed.", active->name), 0;
    if (!TcgDuel_CanAttackNow(&B.view, 0, attack))
        return snprintf(reason, size, "%s can't use that attack this turn.", active->name), 0;
    if (!Field(1, 0))
        return snprintf(reason, size, "There is no Defending Pokemon."), 0;
    return 1;
}
/* The Active Pokemon's Retreat Cost, less a Colorless for each Dodrio on the Bench */
static unsigned RetreatCostNow(void) { return TcgDuel_RetreatCost(&B.view, 0); }
static int CanRetreat(char *reason, size_t size)
{
    const struct TcgCard *c = Field(0, 0);
    const struct TcgPlayer *p = &B.view.players[0];
    const struct TcgFieldCard *f = &p->field[0];
    unsigned s, bench = 0;
    for (s = 1; s < TCG_FIELD_SIZE; s++)
        bench += p->field[s].stackSize != 0;
    if (!c)
        return snprintf(reason, size, "No Active Pokemon."), 0;
    if (!bench)
        return snprintf(reason, size, "No Benched Pokemon to switch in."), 0;
    if (p->retreated || p->confusionRetreatFailed)
        return snprintf(reason, size, "You can only retreat once each turn."), 0;
    if (c->kind == TCG_TRAINER || f->noRetreat)
        return snprintf(reason, size, "%s can't retreat.", c->name), 0;
    if (f->condition == TCG_ASLEEP || f->condition == TCG_PARALYZED)
        return snprintf(reason, size, "%s is %s.", c->name, f->condition == TCG_ASLEEP ? "Asleep" : "Paralyzed"), 0;
    if (f->lockTurn == B.view.turn && f->lockAttack == 3)
        return snprintf(reason, size, "%s can't retreat this turn.", c->name), 0;
    if (TcgPower_RetreatBlock(&B.view, 0))
        return snprintf(reason, size, "%s", TcgPower_RetreatBlock(&B.view, 0)), 0;
    if (TcgDuel_Energy(&B.view, 0, 0, TCG_NONE) < RetreatCostNow())
        return snprintf(reason, size, "Retreating costs %u Energy.", RetreatCostNow()), 0;
    return 1;
}
static int HasPower(const struct TcgCard *c)
{
    return c && c->kind == TCG_POKEMON && c->attacks[0].category == TCG_POKEMON_POWER && c->attacks[0].name[0];
}

/* ------------------------------------------------------------------ */
/* Sprites: every physical card has one, following the shown state.     */
/* ------------------------------------------------------------------ */
static float HandSpacing(unsigned n)
{
    float space = 184.0f - HAND_W;
    if (n <= 1)
        return 0;
    return space / (n - 1) > 33 ? 33 : space / (n - 1);
}
static float BarY(void) { return BAR_CLOSED + (BAR_OPEN - BAR_CLOSED) * Ease(B.tray); }
/* The mat scrolls up while the hand is open, like the DS duel screens,
 * so your Bench stays in view above the cards. Board coordinates are
 * screen coordinates plus this. */
static float Pan(void) { return Ease(B.tray) * PAN; }
static struct Rect HandRect(unsigned i, unsigned n)
{
    float spacing = HandSpacing(n), width = spacing * (n ? n - 1 : 0) + HAND_W;
    int x = (int)(120 - width / 2 + spacing * i), y = (int)(BarY() + Pan()) + 2;
    if (B.focusKind == T_HAND && B.focusIndex == (int)i && B.tray > 0.5f && B.mode != M_PICK)
        y -= 4;
    return MakeRect(x, y, HAND_W, HAND_H);
}
static int StatusAngle(unsigned who, unsigned slot)
{
    if (slot)
        return 0;
    switch (B.cond[who][slot])
    {
    case TCG_ASLEEP:
        return -90;
    case TCG_PARALYZED:
        return 90;
    case TCG_CONFUSED:
        return 180;
    }
    return 0;
}
static void Place(unsigned who, unsigned physical, int zone, int slot, int index, struct Rect r, int face, int layer,
                  int visible)
{
    struct CardSprite *s = &B.sprite[who][physical];
    if (s->zone != zone || s->slot != slot)
    {
        if (s->zone != Z_NONE && (s->zone != zone || s->slot != slot))
            s->delay = B.pops[who]++ * (Fast() ? 2 : 4);
        s->settled = 0;
    }
    s->zone = zone;
    s->slot = slot;
    s->index = index;
    s->tx = (float)r.x;
    s->ty = (float)r.y;
    s->tw = (float)r.w;
    s->th = (float)r.h;
    s->face = face;
    s->layer = layer;
    s->visible = visible;
    /* The opponent's cards face them. */
    s->tangle = who ? 180.0f : 0.0f;
}
static void Layout(void)
{
    unsigned who, i, s, j;
    for (who = 0; who < 2; who++)
    {
        const struct TcgPlayer *p = &B.view.players[who];
        int hidden = who == 1 && B.view.phase == TCG_SETUP;
        B.pops[who] = 0;
        for (i = 0; i < p->deckSize; i++)
            Place(who, p->deck[i], Z_DECK, 0, i, DeckRect(who), 0, 1, 0);
        for (i = 0; i < p->discardSize; i++)
            Place(who, p->discard[i], Z_DISCARD, 0, i, DiscardRect(who), 1, 2, i + 1 == p->discardSize);
        for (i = 0; i < p->prizeSize; i++)
            Place(who, p->prizes[i], Z_PRIZE, i, i, PrizeRect(who, i), 0, 2, 1);
        for (i = 0; i < p->handSize; i++)
        {
            struct Rect r = who ? MakeRect(105, -50, HAND_W, HAND_H) : HandRect(i, p->handSize);
            int layer = 10 + (int)i;
            if (!who && B.spotlight == p->hand[i] && (B.spotHold || B.spotTime > 0))
            {
                /* A Prize card you took, held up in the middle first. */
                Place(who, p->hand[i], Z_HAND, 0, i, MakeRect(CENTER_X - 22, CENTER_Y - 36, 44, 61), 1, 48, 1);
                continue;
            }
            if (!who && B.mode == M_TARGET && (int)i == B.heldHand)
            {
                /* The picked card floats over the slot the cursor is on. */
                struct Rect t = SlotRect(B.focusKind == T_THEIRS, B.focusIndex);
                if (B.focusKind != T_MINE && B.focusKind != T_THEIRS)
                    t = MakeRect(105, 100, 0, 0);
                r = MakeRect(t.x + t.w / 2 - HAND_W / 2 + 8, t.y - 10, HAND_W, HAND_H);
                layer = 40;
            }
            Place(who, p->hand[i], Z_HAND, 0, i, r, !who, layer, who == 0 || 1);
        }
        for (s = 0; s < TCG_FIELD_SIZE; s++)
        {
            const struct TcgFieldCard *f = &p->field[s];
            struct Rect r = SlotRect(who, s);
            for (j = 0; j < f->stackSize; j++)
            {
                int top = j + 1 == f->stackSize;
                Place(who, f->stack[j], top ? Z_FIELD : Z_UNDER, (int)s, j, r, !hidden, top ? 5 : 4, top);
                if (top)
                    B.sprite[who][f->stack[j]].tangle = (who ? 180.0f : 0.0f) + (float)StatusAngle(who, s);
            }
            for (j = 0; j < f->attachedSize; j++)
                Place(who, f->attached[j], Z_ATTACHED, (int)s, j, r, 1, 3, 0);
        }
    }
}
static void SettleAll(void)
{
    unsigned who, i;
    for (who = 0; who < 2; who++)
        for (i = 0; i < TCG_DECK_SIZE; i++)
        {
            struct CardSprite *s = &B.sprite[who][i];
            s->x = s->tx;
            s->y = s->ty;
            s->w = s->tw;
            s->h = s->th;
            s->angle = s->tangle;
            s->shownFace = s->face;
            s->flip = 0;
            s->delay = 0;
            s->settled = 1;
        }
}
static void MoveSprites(void)
{
    unsigned who, i;
    for (who = 0; who < 2; who++)
        for (i = 0; i < TCG_DECK_SIZE; i++)
        {
            struct CardSprite *s = &B.sprite[who][i];
            float k = Fast() ? 0.34f : 0.24f, d;
            if (s->zone == Z_NONE)
                continue;
            if (s->delay > 0)
            {
                s->delay--;
                continue;
            }
            if (s->zone == Z_HAND && who == 0)
                k = 0.36f;
            s->x += (s->tx - s->x) * k;
            s->y += (s->ty - s->y) * k;
            s->w += (s->tw - s->w) * k;
            s->h += (s->th - s->h) * k;
            s->angle += (s->tangle - s->angle) * 0.2f;
            d = fabsf(s->tx - s->x) + fabsf(s->ty - s->y) + fabsf(s->tw - s->w) + fabsf(s->th - s->h);
            if (d < 0.6f)
            {
                s->x = s->tx;
                s->y = s->ty;
                s->w = s->tw;
                s->h = s->th;
            }
            if (fabsf(s->tangle - s->angle) < 0.5f)
                s->angle = s->tangle;
            if (s->face != s->shownFace)
            {
                s->flip += Fast() ? 0.2f : 0.1f;
                if (s->flip >= 0.5f)
                {
                    s->shownFace = s->face;
                    s->flip = s->flip - 1.0f;
                }
            }
            else if (s->flip < 0)
            {
                s->flip += Fast() ? 0.2f : 0.1f;
                if (s->flip > 0)
                    s->flip = 0;
            }
            s->settled = d < 0.6f && s->angle == s->tangle && s->face == s->shownFace && s->flip == 0;
        }
}
static int SpritesSettled(void)
{
    unsigned who, i;
    for (who = 0; who < 2; who++)
        for (i = 0; i < TCG_DECK_SIZE; i++)
        {
            const struct CardSprite *s = &B.sprite[who][i];
            if (s->zone != Z_NONE && !s->settled && !(who == 0 && s->zone == Z_HAND))
                return 0;
        }
    return 1;
}
static void ResetShown(void)
{
    unsigned who, s;
    for (who = 0; who < 2; who++)
        for (s = 0; s < TCG_FIELD_SIZE; s++)
        {
            const struct TcgFieldCard *f = &B.view.players[who].field[s];
            B.damage[who][s] = f->stackSize ? f->damage : 0;
            B.cond[who][s] = f->condition;
            B.poison[who][s] = f->poison;
            B.ko[who][s] = 0;
        }
}

/* ------------------------------------------------------------------ */
/* Effects                                                              */
/* ------------------------------------------------------------------ */
static struct Fx *AddFx(int type, int x, int y, int duration)
{
    unsigned i;
    for (i = 0; i < FX_MAX; i++)
        if (!B.fx[i].duration)
        {
            struct Fx *f = &B.fx[i];
            memset(f, 0, sizeof(*f));
            f->type = type;
            f->x = x;
            f->y = y;
            f->duration = duration;
            return f;
        }
    return NULL;
}
static void Text(int x, int y, const char *text, unsigned color, int duration)
{
    struct Fx *f = AddFx(FX_TEXT, x, y, duration);
    if (f)
    {
        snprintf(f->text, sizeof(f->text), "%s", text);
        f->color = color;
    }
}
static void Banner(int x, int y, const char *text, unsigned color, int duration)
{
    struct Fx *f = AddFx(FX_BANNER, x, y, duration);
    if (f)
    {
        snprintf(f->text, sizeof(f->text), "%s", text);
        f->color = color;
    }
}
static void Sparks(int x, int y, unsigned color, int count, float speed)
{
    int i;
    for (i = 0; i < count; i++)
    {
        struct Fx *f = AddFx(FX_SPARK, x, y, 16 + rand() % 10);
        float a = (float)i / count * 6.2831853f + (rand() % 100) / 200.0f;
        if (!f)
            return;
        f->vx = cosf(a) * speed * (0.6f + (rand() % 50) / 100.0f);
        f->vy = sinf(a) * speed * (0.6f + (rand() % 50) / 100.0f) - 0.4f;
        f->color = i & 1 ? color : G_Mix(color, 0xffffff, 0.6f);
        f->value = 2 + (i % 2);
    }
}
static void CardCenter(unsigned who, unsigned slot, int *x, int *y)
{
    struct Rect r = SlotRect(who, slot);
    *x = r.x + r.w / 2;
    *y = r.y + r.h / 2;
}
static unsigned AttackSe(const char *anim, unsigned type, unsigned *color)
{
    static const struct
    {
        const char *key;
        unsigned se, color;
    } map[] = {
        {"FLAME", SE_M_FLAMETHROWER, 0xf86828}, {"FIRE", SE_M_FIRE_PUNCH, 0xf86828},
        {"WATER", SE_M_WATERFALL, 0x48a0f8},    {"HYDRO", SE_M_HYDRO_PUMP, 0x48a0f8},
        {"BUBBLE", SE_M_BUBBLE_BEAM, 0x78c0f8}, {"WHIRLPOOL", SE_M_WHIRLPOOL, 0x48a0f8},
        {"THUNDER", SE_M_THUNDERBOLT, 0xf8e040}, {"LIGHTNING", SE_M_THUNDERBOLT, 0xf8e040},
        {"MAGNETIC", SE_M_THUNDER_WAVE, 0xf8e040}, {"PSYCHIC", SE_M_PSYBEAM, 0xc070f0},
        {"DARK", SE_M_NIGHTMARE, 0x9058c0},     {"CONFUSE", SE_M_CONFUSE_RAY, 0xd080f0},
        {"HYPNOSIS", SE_M_PSYBEAM2, 0xc070f0},  {"SUPERSONIC", SE_M_SUPERSONIC, 0xd0d0f8},
        {"SING", SE_M_SING, 0xf8a0d0},          {"LULLABY", SE_M_SING, 0xf8a0d0},
        {"POWDER", SE_M_POISON_POWDER, 0xc070e0}, {"SPORE", SE_M_POISON_POWDER, 0xa0d050},
        {"POISON", SE_M_TOXIC, 0xb048d0},       {"TOXIC", SE_M_TOXIC, 0xb048d0},
        {"GAS", SE_M_HAZE, 0xa070c0},           {"SMOG", SE_M_HAZE, 0xa070c0},
        {"GOO", SE_M_ACID_ARMOR, 0x90c050},     {"BLIZZARD", SE_M_BLIZZARD, 0xc8f0ff},
        {"FREEZE", SE_M_ICY_WIND, 0xc8f0ff},    {"ROCK", SE_M_ROCK_THROW, 0xc09060},
        {"STONE", SE_M_ROCK_THROW, 0xc09060},   {"BONE", SE_M_BONEMERANG, 0xe8e0c8},
        {"SEISMIC", SE_M_STRENGTH, 0xc09060},   {"BEAM", SE_M_HYPER_BEAM, 0xf8f8a0},
        {"SELFDESTRUCT", SE_M_EXPLOSION, 0xf8a040}, {"DRAIN", SE_M_GIGA_DRAIN, 0x80e070},
        {"WHIRLWIND", SE_M_GUST, 0xe0f0f0},     {"SONICBOOM", SE_M_RAZOR_WIND, 0xe0f0f0},
        {"SLASH", SE_M_CUT, 0xf8f8f8},          {"NEEDLE", SE_M_HORN_ATTACK, 0xf8f8f8},
        {"DRILL", SE_M_HORN_ATTACK, 0xf8f8f8},  {"WHIP", SE_M_VITAL_THROW, 0x80d050},
        {"STRING", SE_M_STRING_SHOT, 0xf8f8f8}, {"PETAL", SE_M_PETAL_DANCE, 0xf8a0c0},
        {"PUNCH", SE_M_MEGA_KICK, 0xf8f8f8},    {"KICK", SE_M_MEGA_KICK2, 0xf8f8f8},
        {"BIG_HIT", SE_M_MEGA_KICK, 0xf8f8f8},  {"RAMPAGE", SE_M_TAKE_DOWN, 0xf8f8f8},
        {"QUICK", SE_M_COMET_PUNCH, 0xf8f8f8},  {"BARRIER", SE_M_BARRIER, 0x98e0f8},
        {"PROTECT", SE_M_BARRIER, 0x98e0f8},    {"GLOW", SE_M_MORNING_SUN, 0xf8f0a0},
        {"RECOVER", SE_M_MORNING_SUN, 0x80e070}, {"HEAL", SE_M_MORNING_SUN, 0x80e070},
        {"CRY", SE_M_UPROAR, 0xf8f8f8},         {"DRAGON", SE_M_DRAGON_RAGE, 0xf86828},
    };
    unsigned i;
    for (i = 0; i < sizeof(map) / sizeof(map[0]); i++)
        if (anim && strstr(anim, map[i].key))
        {
            *color = map[i].color;
            return map[i].se;
        }
    *color = G_TypeColor(type, 1);
    return SE_M_COMET_PUNCH;
}
static const char *PlayerName(unsigned who) { return who ? B.opponent : "YOU"; }

/* Trainer cards whose effect lands in your hand (or the opponent's that hit yours). */
static int ChangesHand(unsigned card, unsigned who)
{
    switch (card)
    {
    case TCG_CARD_IMPOSTER_PROFESSOR_OAK:
    case TCG_CARD_LASS:
        return 1;
    case TCG_CARD_PROFESSOR_OAK:
    case TCG_CARD_BILL:
    case TCG_CARD_GAMBLER:
    case TCG_CARD_COMPUTER_SEARCH:
    case TCG_CARD_ITEM_FINDER:
    case TCG_CARD_ENERGY_RETRIEVAL:
    case TCG_CARD_SUPER_ENERGY_RETRIEVAL:
    case TCG_CARD_ENERGY_SEARCH:
    case TCG_CARD_POKE_BALL:
    case TCG_CARD_POKEMON_TRADER:
    case TCG_CARD_MAINTENANCE:
    case TCG_CARD_SCOOP_UP:
        return who == 0;
    }
    return 0;
}
/* Starts one presented event; returns how many frames it holds the stage. */
static int StartEvent(const struct TcgEvent *e, int post)
{
    int x, y, fast = Fast();
    char line[64];
    unsigned who = e->player & 1, slot = e->slot < TCG_FIELD_SIZE ? e->slot : 0;
    const struct TcgCard *c = e->card ? Tcg_Card(e->card) : NULL;
    /* For the test harness: the moments worth a screenshot. */
    if (e->type == TCG_EV_KO || e->type == TCG_EV_WIN || e->type == TCG_EV_PROMOTE || e->type == TCG_EV_STATUS ||
        e->type == TCG_EV_COIN)
        printf("[tcg] show event %u player %u slot %u value %u frame %u\n", e->type, e->player, e->slot, e->value,
               B.frame);
    if (e->type == TCG_EV_TURN) /* (the hands as the banner sweeps: the new turn's draw comes after) */
        printf("[tcg] show turn %u player %u hands %u %u\n", e->value, e->player, B.view.players[0].handSize,
               B.view.players[1].handSize);
    switch (e->type)
    {
    case TCG_EV_TURN:
    {
        struct Fx *f = AddFx(FX_SWEEP, 0, CENTER_Y, fast ? 34 : 52);
        if (f)
        {
            if (who)
            {
                if (B.online)
                {
                    unsigned k;
                    snprintf(f->text, sizeof(f->text), "%s'S TURN", B.opponent);
                    for (k = 0; f->text[k]; k++)
                        if (f->text[k] >= 'a' && f->text[k] <= 'z')
                            f->text[k] -= 32;
                }
                else
                    snprintf(f->text, sizeof(f->text), "OPPONENT'S TURN");
            }
            else
                snprintf(f->text, sizeof(f->text), "YOUR TURN");
            f->color = who ? 0xe85050 : 0x3878f0;
        }
        PlaySE(who ? SE_BALL_TRAY_EXIT : SE_BALL_TRAY_ENTER);
        if (!who && B.mode == M_FREE)
        {
            B.focusKind = T_HAND;
            B.focusIndex = B.view.players[0].handSize ? B.view.players[0].handSize - 1 : 0;
            B.trayWanted = 1;
            B.checkDone = 1;
        }
        return fast ? 26 : 40;
    }
    case TCG_EV_ATTACK:
    {
        unsigned color, se;
        const struct TcgAttack *a = c ? &c->attacks[e->slot & 1] : NULL;
        if (!a)
            return 0;
        se = AttackSe(a->animation, c->type, &color);
        B.lunge[who] = fast ? 12 : 18;
        snprintf(line, sizeof(line), "%s", a->name);
        Banner(CENTER_X + (who ? 52 : -52), CENTER_Y, line, 0x283848, fast ? 24 : 36);
        PlaySE(se);
        CardCenter(who ^ 1, 0, &x, &y);
        Sparks(x, y, color, 10, 1.6f);
        return fast ? 22 : 34;
    }
    case TCG_EV_COIN:
    {
        struct Fx *f = AddFx(FX_COIN, CENTER_X, CENTER_Y, fast ? 40 : 64);
        if (f)
        {
            f->value = e->value != 0;
            f->player = who;
        }
        PlaySE(SE_M_PAY_DAY);
        return fast ? 40 : 64;
    }
    case TCG_EV_DAMAGE:
    {
        struct Fx *f;
        int strong = e->flags & TCG_DMG_WEAK, weak = e->flags & TCG_DMG_RESIST;
        CardCenter(who, slot, &x, &y);
        B.damage[who][slot] += e->value;
        B.shake[who][slot] = strong ? 22 : 14;
        B.flash[who][slot] = 6;
        f = AddFx(FX_NUMBER, x + 10, y - 10, fast ? 44 : 66);
        if (f)
        {
            snprintf(f->text, sizeof(f->text), "-%u", e->value);
            f->color = e->flags & TCG_DMG_POISON ? 0xf0c8ff : 0xe8f8ff;
            f->color2 = e->flags & TCG_DMG_POISON ? 0x602880 : 0x2850a0;
        }
        if (strong)
            Banner(x, who ? y + 30 : y - 30, "WEAKNESS *2", 0x7040b0, fast ? 44 : 70);
        else if (weak)
            Banner(x, who ? y + 30 : y - 30, "RESISTANCE -30", 0x405070, fast ? 44 : 70);
        if (e->flags & TCG_DMG_POISON)
            Text(x, y - 30, "POISON", 0xd8a0f8, fast ? 30 : 46);
        if (e->flags & TCG_DMG_SELF)
            Text(x, y - 30, "IT HURT ITSELF!", 0xf8c0e0, fast ? 30 : 46);
        PlaySE(e->flags & TCG_DMG_POISON ? SE_FIELD_POISON : strong ? SE_SUPER_EFFECTIVE : weak ? SE_NOT_EFFECTIVE
                                                                                                    : SE_EFFECTIVE);
        Sparks(x, y, 0xf8f8f8, strong ? 14 : 8, strong ? 2.2f : 1.5f);
        if (!(e->flags & (TCG_DMG_POISON | TCG_DMG_SELF)))
        {
            f = AddFx(FX_IMPACT, x, y, fast ? 16 : 24);
            if (f)
                f->player = who;
        }
        return fast ? 30 : strong ? 64 : 48;
    }
    case TCG_EV_HEAL:
    {
        struct Fx *f;
        CardCenter(who, slot, &x, &y);
        B.damage[who][slot] = B.damage[who][slot] > e->value ? B.damage[who][slot] - e->value : 0;
        f = AddFx(FX_NUMBER, x + 10, y - 10, fast ? 36 : 54);
        if (f)
        {
            snprintf(f->text, sizeof(f->text), "+%u", e->value);
            f->color = 0xd0ffd0;
            f->color2 = 0x187030;
        }
        Sparks(x, y, 0x80f080, 8, 1.0f);
        PlaySE(SE_USE_ITEM);
        return fast ? 22 : 36;
    }
    case TCG_EV_STATUS:
    {
        static const char *const names[] = {"RECOVERED", "ASLEEP", "PARALYZED", "CONFUSED"};
        static const unsigned se[] = {SE_USE_ITEM, SE_M_SING, SE_M_THUNDER_WAVE, SE_M_CONFUSE_RAY};
        int poisoned = e->flags && !B.poison[who][slot];
        CardCenter(who, slot, &x, &y);
        if (poisoned && e->value == B.cond[who][slot])
        {
            Text(x, y - 26, "POISONED", 0xd8a0f8, fast ? 32 : 50);
            PlaySE(SE_M_TOXIC);
        }
        else if (e->value < 4)
        {
            Text(x, y - 26, names[e->value], e->value ? 0xf8f0a0 : 0xc0f8c0, fast ? 32 : 50);
            PlaySE(se[e->value]);
        }
        B.cond[who][slot] = (uint8_t)e->value;
        B.poison[who][slot] = e->flags;
        return fast ? 24 : 40;
    }
    case TCG_EV_NO_DAMAGE:
        CardCenter(who, slot, &x, &y);
        Banner(x, who ? y + 30 : y - 30, e->value ? "THE ATTACK MISSED!" : "NO DAMAGE!", 0x2870a0, fast ? 36 : 54);
        PlaySE(SE_M_BARRIER);
        return fast ? 26 : 40;
    case TCG_EV_KO:
        CardCenter(who, slot, &x, &y);
        B.ko[who][slot] = 1;
        B.flash[who][slot] = 10;
        {
            struct Fx *f = AddFx(FX_SHARDS, x, y, fast ? 24 : 32);
            if (f)
                f->value = (int)((unsigned)(who * 7 + slot + 1) * 2654435761u >> 8);
        }
        Banner(x, who ? y + 30 : y - 30, "KNOCKED OUT!", 0xb02828, fast ? 40 : 64);
        PlaySE(SE_FAINT);
        if (!who)
        {
            /* Your Bench comes into view to choose who goes in next. */
            B.trayWanted = 0;
            B.promoFocused = 0;
        }
        return fast ? 34 : 56;
    case TCG_EV_TRAINER:
    {
        struct Fx *f = AddFx(FX_SHOWCASE, CENTER_X, CENTER_Y, fast ? 42 : 72);
        if (ChangesHand(e->card, who))
        {
            B.showHand = fast ? 50 : 80;
            if (!who)
            {
                B.trayWanted = 1;
                B.focusKind = T_HAND;
                B.focusIndex = 0;
            }
        }
        if (f)
        {
            f->value = e->card;
            f->player = who;
        }
        PlaySE(SE_RG_CARD_FLIP);
        return fast ? 34 : 60;
    }
    case TCG_EV_POWER:
        /* The power's name over its Pokemon */
        printf("[tcg] show power %s player %u slot %u\n", TcgPower_Name(e->value), who, slot);
        CardCenter(who, slot, &x, &y);
        Banner(x, who ? y + 30 : y - 30, TcgPower_Name(e->value), 0xb83838, fast ? 30 : 48);
        Sparks(x, y, 0xf87878, 10, 1.4f);
        B.flash[who][slot] = 6;
        PlaySE(SE_M_DETECT);
        return fast ? 24 : 40;
    case TCG_EV_PEEK:
    {
        static const char *const mine[] = {"TOP OF YOUR DECK", "TOP OF THEIR DECK", "A CARD IN THEIR HAND",
                                           "ONE OF YOUR PRIZES", "ONE OF THEIR PRIZES"},
                                 *theirs[] = {"THE TOP OF THEIR DECK", "THE TOP OF YOUR DECK", "A CARD IN YOUR HAND",
                                              "ONE OF THEIR PRIZES", "ONE OF YOUR PRIZES"};
        if (e->value > TCG_PEEK_THEIR_PRIZE)
            return 0;
        if (!who)
        {
            /* Only its player sees the card */
            struct Fx *f = AddFx(FX_SHOWCASE, CENTER_X, CENTER_Y, fast ? 60 : 96);
            if (f)
            {
                f->value = e->card;
                f->player = 0;
                f->color = 1; /* (back where it came from, not to the discard pile) */
                snprintf(f->text, sizeof(f->text), "%s", mine[e->value]);
            }
            PlaySE(SE_RG_CARD_FLIP);
            return fast ? 52 : 88;
        }
        snprintf(line, sizeof(line), "THEY LOOKED AT %s", theirs[e->value]);
        Banner(CENTER_X, CENTER_Y, line, 0x283848, fast ? 40 : 64);
        PlaySE(SE_RG_CARD_FLIP);
        return fast ? 34 : 56;
    }
    case TCG_EV_RETREAT_FAIL:
        CardCenter(who, 0, &x, &y);
        Text(x, y - 26, "RETREAT FAILED", 0xf8c0c0, 46);
        PlaySE(SE_FAILURE);
        return 40;
    case TCG_EV_PLAY:
    case TCG_EV_PROMOTE:
        if (post)
        {
            PlaySE(SE_RG_CARD_FLIP);
            if (e->type == TCG_EV_PLAY)
            {
                struct Fx *f;
                CardCenter(who, slot, &x, &y);
                f = AddFx(FX_RISE, x, y, fast ? 14 : 20);
                if (f)
                    f->color = c ? G_TypeColor(c->type, 0) : 0xf8f0a0;
            }
        }
        return 4;
    case TCG_EV_EVOLVE:
        CardCenter(who, slot, &x, &y);
        Sparks(x, y, 0xf8f0a0, 16, 1.8f);
        Text(x, y - 26, "EVOLVED!", 0xf8f0a0, 44);
        PlaySE(SE_SHINY);
        return fast ? 16 : 30;
    case TCG_EV_ATTACH:
        CardCenter(who, slot, &x, &y);
        Sparks(x, y + 8, c ? G_TypeColor(c->type, 0) : 0xf8f8f8, 8, 1.2f);
        PlaySE(SE_BALL_TRAY_BALL);
        return fast ? 6 : 12;
    case TCG_EV_PRIZE:
        printf("[tcg] prize taken by %u, shown %d\n", who, !who && B.spotlight >= 0);
        if (!who && B.spotlight >= 0 && (B.spotHold || B.prizeNext < B.prizeCount))
        {
            /* Your Prize, face up in the middle, then into your hand. */
            if (!B.spotHold)
                B.spotlight = B.prizeQueue[B.prizeNext++];
            B.spotHold = 0;
            B.spotTime = fast ? 34 : 58;
            PlaySE(SE_RG_CARD_FLIP);
            return B.spotTime + 6;
        }
        PlaySE(SE_RG_CARD_FLIPPING);
        return fast ? 8 : 16;
    case TCG_EV_RETREAT:
    case TCG_EV_SWITCH:
        PlaySE(SE_RG_CARD_FLIP);
        return 8;
    case TCG_EV_SUDDEN_DEATH:
    {
        struct Fx *f = AddFx(FX_SWEEP, 0, CENTER_Y, 80);
        if (f)
        {
            snprintf(f->text, sizeof(f->text), "SUDDEN DEATH!");
            f->color = 0xd8a020;
        }
        PlaySE(SE_BANG);
        return 70;
    }
    case TCG_EV_WIN:
        B.resultShown = 1;
        B.resultTime = 0;
        B.mode = M_RESULT;
        if (who == 0)
            PlayBGM(MUS_VICTORY_TRAINER);
        else
            FadeOutBGM(4);
        if (who == 0)
            snprintf(B.confirmText, sizeof(B.confirmText), "%s",
                     e->value == 3 ? "Your opponent conceded." : e->value == 2 ? "Your opponent's deck ran out."
                                                             : e->value == 1 ? "Your opponent has no Pokemon left."
                                                                             : "You took all your Prize cards!");
        else
            snprintf(B.confirmText, sizeof(B.confirmText), "%s",
                     e->value == 3 ? "You conceded the duel." : e->value == 2 ? "Your deck ran out."
                                                            : e->value == 1 ? "You have no Pokemon left in play."
                                                                            : "Your opponent took all their Prizes.");
        return 30;
    case TCG_EV_DEVOLVE:
        CardCenter(who, slot, &x, &y);
        Text(x, y - 26, "DEVOLVED!", 0xe0c8ff, fast ? 30 : 46);
        Sparks(x, y, 0xc0a0f8, 12, 1.6f);
        B.flash[who][slot] = 8;
        PlaySE(SE_M_TELEPORT);
        return fast ? 24 : 40;
    case TCG_EV_TO_HAND:
        CardCenter(who, 0, &x, &y);
        Banner(x, who ? y + 30 : y - 30, "BACK TO THE HAND!", 0x2870a0, fast ? 36 : 54);
        PlaySE(SE_M_GUST);
        return fast ? 26 : 40;
    case TCG_EV_MORPH:
    {
        struct Fx *f;
        CardCenter(who, slot, &x, &y);
        snprintf(line, sizeof(line), "%s!", c ? c->name : "MORPH");
        Text(x, y - 26, line, 0xf8f0a0, fast ? 32 : 50);
        Sparks(x, y, 0xf0c8f8, 16, 1.8f);
        f = AddFx(FX_RISE, x, y, fast ? 14 : 20);
        if (f)
            f->color = 0xd890e8;
        PlaySE(SE_SHINY);
        return fast ? 24 : 40;
    }
    case TCG_EV_DECK_CHANGED:
    {
        struct Rect d = DeckRect(who);
        if (e->value)
            snprintf(line, sizeof(line), "-%u", e->value);
        else
            snprintf(line, sizeof(line), "%s", "SHUFFLED");
        Text(d.x + d.w / 2, d.y - 6, line, 0xf8d0c0, fast ? 30 : 46);
        PlaySE(e->value ? SE_M_FLAME_WHEEL : SE_RG_CARD_FLIPPING);
        return fast ? 18 : 30;
    }
    default:
        return 0;
    }
}
static int IsPostEvent(unsigned type)
{
    return type == TCG_EV_TURN || type == TCG_EV_WIN || type == TCG_EV_SUDDEN_DEATH || type == TCG_EV_EVOLVE ||
           type == TCG_EV_ATTACH || type == TCG_EV_PLAY || type == TCG_EV_PRIZE || type == TCG_EV_PROMOTE ||
           type == TCG_EV_RETREAT || type == TCG_EV_SWITCH;
}
static void BeginState(const struct TcgDuel *next)
{
    uint32_t seq = B.view.eventSeq;
    int late = 0;
    B.npre = B.npost = 0;
    if (next->eventSeq < seq || next->eventSeq - seq > TCG_EVENT_RING)
        seq = next->eventSeq > TCG_EVENT_RING ? next->eventSeq - TCG_EVENT_RING : 0;
    for (; seq < next->eventSeq; seq++)
    {
        const struct TcgEvent *e = TcgDuel_GetEvent(next, seq);
        if (!e)
            continue;
        /* A power that works as its Pokemon is played or evolves (Firegiver, Healing
         * Wind...): it and what it does come once the card is down */
        if (e->type == TCG_EV_POWER && B.npost &&
            (B.post[B.npost - 1].type == TCG_EV_PLAY || B.post[B.npost - 1].type == TCG_EV_EVOLVE))
            late = 1;
        /* A step that starts with a new turn: its banner, then the draw */
        if (late || (IsPostEvent(e->type) && !(e->type == TCG_EV_TURN && !B.npre && !B.npost)))
            B.post[B.npost++] = *e;
        else
            B.pre[B.npre++] = *e;
    }
    B.phase = P_PRE;
    B.index = 0;
    B.wait = 0;
}
static void OpenTurnMenu(void);
/* After an action of ours, the cursor goes where the next one is most likely: a Basic Pokemon played (setting up
 * or not), an evolution or a Trainer card go back to the hand (setting up: on the next Basic Pokemon while the
 * Bench has room, or the turn menu with READY when nothing else can be put down); Energy attached that lets the
 * Active Pokemon attack goes to it, else back to the hand. */
static void Refocus(void)
{
    const struct TcgPlayer *p = &B.view.players[0];
    int type = B.refocus - 1, i, benched = 0;
    unsigned h;
    B.refocus = 0;
    if (B.mode != M_FREE || !MyTurn() || type < 0)
        return;
    for (i = 0; i < TCG_FIELD_SIZE; i++)
        benched += p->field[i].stackSize != 0;
    if (B.view.phase == TCG_SETUP && type == TCG_PLAY_BASIC)
    {
        for (h = 0; h < p->handSize && benched < TCG_FIELD_SIZE; h++)
        {
            const struct TcgCard *c = Tcg_Card(p->cards[p->hand[h]]);
            if (c->kind == TCG_POKEMON && !c->stage)
            {
                B.focusKind = T_HAND;
                B.focusIndex = (int)h;
                B.trayWanted = 1;
                printf("[tcg] refocus: the next Basic Pokemon, hand %u\n", h);
                return;
            }
        }
        printf("[tcg] refocus: no Basic Pokemon left to put down (%d in play)\n", benched);
        OpenTurnMenu();   /* nothing else to put down: READY is one A away */
        return;
    }
    if (B.view.phase != TCG_TURN)
        return;
    if (type == TCG_ATTACH && p->field[0].stackSize &&
        (TcgDuel_CanAttack(&B.view, 0, 0) || TcgDuel_CanAttack(&B.view, 0, 1)))
    {
        B.focusKind = T_MINE;
        B.focusIndex = 0;
        return;
    }
    if ((type == TCG_PLAY_BASIC || type == TCG_EVOLVE || type == TCG_PLAY_TRAINER || type == TCG_ATTACH) && p->handSize)
    {
        B.focusKind = T_HAND;
        B.focusIndex = B.refocusHand < (int)p->handSize ? B.refocusHand : (int)p->handSize - 1;
        B.trayWanted = 1;
    }
}
static void Commit(const struct TcgDuel *next)
{
    unsigned i, moved = 0, who;
    int before[2][TCG_DECK_SIZE];
    for (who = 0; who < 2; who++)
        for (i = 0; i < TCG_DECK_SIZE; i++)
            before[who][i] = B.sprite[who][i].zone * 16 + B.sprite[who][i].slot;
    B.view = *next;
    printf("[tcg] commit hands %u %u\n", B.view.players[0].handSize, B.view.players[1].handSize);
    if (B.refocus && B.qCount <= 1)   /* (the state being shown is the last one queued) */
    {
        Refocus();
        B.checkDone = 1;
    }
    ResetShown();
    for (i = 0; i < (unsigned)B.npost; i++)
    {
        /* (damage still to be shown after the commit: see BeginState) */
        const struct TcgEvent *e = &B.post[i];
        float *d = &B.damage[e->player & 1][e->slot < TCG_FIELD_SIZE ? e->slot : 0];
        if (e->type == TCG_EV_DAMAGE)
            *d = *d > e->value ? *d - e->value : 0;
        if (e->type == TCG_EV_HEAL)
            *d += e->value;
    }
    B.prizeCount = B.prizeNext = 0;
    for (i = 0; i < B.view.players[0].handSize; i++)
    {
        unsigned physical = B.view.players[0].hand[i];
        if (B.sprite[0][physical].zone == Z_PRIZE && B.prizeCount < 6)
            B.prizeQueue[B.prizeCount++] = (uint8_t)physical;
    }
    if (B.prizeCount)
    {
        /* It flies from its spot to the middle and waits for its event. */
        B.spotlight = B.prizeQueue[B.prizeNext++];
        B.spotHold = 1;
        B.spotTime = 0;
    }
    Layout();
    for (who = 0; who < 2; who++)
        for (i = 0; i < TCG_DECK_SIZE; i++)
            moved += before[who][i] != B.sprite[who][i].zone * 16 + B.sprite[who][i].slot;
    if (moved)
        PlaySE(moved > 2 ? SE_RG_CARD_FLIPPING : SE_RG_CARD_FLIP);
}
static void Present(void)
{
    if (B.phase == P_IDLE)
    {
        if (!B.qCount)
            return;
        BeginState(&B.queue[B.qHead]);
    }
    if (B.wait > 0)
    {
        B.wait--;
        return;
    }
    switch (B.phase)
    {
    case P_PRE:
        if (B.index < B.npre)
        {
            B.wait = StartEvent(&B.pre[B.index++], 0);
            break;
        }
        Commit(&B.queue[B.qHead]);
        B.phase = P_COMMIT;
        B.wait = 4;
        break;
    case P_COMMIT:
        if (SpritesSettled())
        {
            B.phase = P_POST;
            B.index = 0;
        }
        break;
    case P_POST:
        if (B.index < B.npost)
        {
            B.wait = StartEvent(&B.post[B.index++], 1);
            break;
        }
        B.qHead = (B.qHead + 1) % QUEUE;
        B.qCount--;
        B.phase = P_IDLE;
        break;
    }
}

/* ------------------------------------------------------------------ */
/* Begin / sync                                                         */
/* ------------------------------------------------------------------ */
static u16 sNextMusic;
static int sNextTutorial;
void TcgBoard_SetMusic(unsigned song) { sNextMusic = (u16)song; }
/* Harness helper (mp cards fx N): an attack landing (0), a knock out (1) or a Pokemon put into play (2) on
   their Active, the effect alone */
void TcgBoard_TestFx(unsigned kind)
{
    int x, y;
    struct Fx *f;
    CardCenter(1, 0, &x, &y);
    f = AddFx(kind == 0 ? FX_IMPACT : kind == 1 ? FX_SHARDS : FX_RISE, x, y, kind == 2 ? 20 : kind ? 32 : 24);
    if (f)
        f->color = 0xf8f0a0, f->value = 12345, f->player = 1;
}
void TcgBoard_Tutorial(int on) { sNextTutorial = on; }
void TcgBoard_Begin(const struct TcgDuel *truth, const char *opponent, int online)
{
    unsigned who, i;
    struct Fx *f;
    memset(&B, 0, sizeof(B));
    B.duelMusic = sNextMusic;
    sNextMusic = 0;
    B.tutorial = sNextTutorial;
    sNextTutorial = 0;
    B.active = 1;
    B.online = online;
    snprintf(B.opponent, sizeof(B.opponent), "%s", opponent ? opponent : "OPPONENT");
    B.view = *truth;
    B.lastSeq = truth->eventSeq;
    ResetShown();
    for (who = 0; who < 2; who++)
        for (i = 0; i < TCG_DECK_SIZE; i++)
        {
            struct Rect r = DeckRect(who);
            struct CardSprite *s = &B.sprite[who][i];
            s->x = s->tx = (float)r.x;
            s->y = s->ty = (float)r.y;
            s->w = s->tw = (float)r.w;
            s->h = s->th = (float)r.h;
            s->zone = Z_DECK;
            s->angle = s->tangle = who ? 180.0f : 0.0f;
            s->settled = 1;
        }
    B.focusKind = T_HAND;
    B.heldHand = -1;
    B.spotlight = -1;
    B.mode = M_FREE;
    B.music = GetCurrentMapMusic();
    PlayBGM(B.duelMusic ? B.duelMusic : MUS_B_ARENA);
    /* Who goes first is decided before the deal, with a coin. */
    f = AddFx(FX_COIN, CENTER_X, CENTER_Y, 90);
    if (f)
    {
        f->value = truth->first == 0;
        f->player = 2;
    }
    PlaySE(SE_M_PAY_DAY);
    B.intro = 92;
}
void TcgBoard_End(void)
{
    if (B.active && B.music)
        MP_MUSIC_TO_MAP(PlayBGM(B.music));   // the map's own song again (custom music: the map's, not the duel's)
    B.active = 0;
}
/* A won game going on to the cards to take: the victory song plays on through them, and the map's song comes back
   with TcgBoard_ReleaseMusic when they are done */
static u16 sHeldMusic;
void TcgBoard_EndHoldMusic(void)
{
    if (B.active && B.music)
        sHeldMusic = B.music;
    B.active = 0;
}
void TcgBoard_ReleaseMusic(void)
{
    u16 m = sHeldMusic;
    sHeldMusic = 0;
    if (m)
        MP_MUSIC_TO_MAP(PlayBGM(m));
}
static void Enqueue(const struct TcgDuel *state)
{
    if (B.qCount == QUEUE)
    {
        /* Falling far behind: drop the oldest pending state. */
        B.qHead = (B.qHead + 1) % QUEUE;
        B.qCount--;
        if (B.phase != P_IDLE)
            B.phase = P_IDLE;
    }
    B.queue[(B.qHead + B.qCount) % QUEUE] = *state;
    B.qCount++;
}
void TcgBoard_Sync(const struct TcgDuel *truth)
{
    const struct TcgDuel *last = B.qCount ? &B.queue[(B.qHead + B.qCount - 1) % QUEUE] : &B.view;
    uint32_t seq;
    if (truth->eventSeq == last->eventSeq && !memcmp(truth->players, last->players, sizeof(truth->players)) &&
        truth->phase == last->phase && truth->current == last->current)
        return;
    /* An action that ends the turn also starts the next one, draw included: shown in one step,
     * that draw would slide in with the action's own cards (after Kangaskhan's Fetch both
     * players seemed to draw). The next turn becomes a step of its own: its banner, then its draw. */
    for (seq = last->eventSeq + 1; truth->eventSeq > last->eventSeq && seq < truth->eventSeq; seq++)
    {
        const struct TcgEvent *turn = TcgDuel_GetEvent(truth, seq), *draw = TcgDuel_GetEvent(truth, seq + 1);
        if (turn && turn->type == TCG_EV_TURN && draw && draw->type == TCG_EV_DRAW && draw->player == turn->player &&
            seq + 2 == truth->eventSeq && truth->players[turn->player].handSize)
        {
            static struct TcgDuel before;
            struct TcgPlayer *p;
            before = *truth;
            before.eventSeq = seq;
            p = &before.players[turn->player];
            p->deck[p->deckSize++] = p->hand[--p->handSize];
            Enqueue(&before);
            break;
        }
    }
    Enqueue(truth);
}
int TcgBoard_Idle(void) { return B.intro <= 0 && B.phase == P_IDLE && !B.qCount && SpritesSettled(); }
int TcgBoard_Busy(void) { return B.intro > 0 || B.phase != P_IDLE || B.qCount || B.spotTime > 0; }
void TcgBoard_SetWaiting(int waiting) { B.waiting = waiting; }
/* Test harness: look again whether anything is left to play from the hand (FocusActiveIfDone) */
void TcgBoard_CheckDone(void)
{
    B.checkDone = 1;
}
/* Test harness: put the cursor on a target (0 hand, 1 yours, 2 theirs, 10 END). */
void TcgBoard_DebugFocus(int kind, int index)
{
    B.focusKind = kind;
    B.focusIndex = kind == T_HAND && index >= (int)B.view.players[0].handSize ? (int)B.view.players[0].handSize - 1 : index;
    B.trayWanted = kind == T_HAND;
    printf("[tcg] board focus %d %d mode %d idle %d\n", kind, index, B.mode, TcgBoard_Idle());
}
void TcgBoard_Reject(const char *message)
{
    Toast("%s", message && message[0] ? message : "That can't be done now.");
    PlaySE(SE_FAILURE);
}
void TcgBoard_Result(const char *message) { snprintf(B.result, sizeof(B.result), "%s", message ? message : ""); }

/* ------------------------------------------------------------------ */
/* Targets and navigation                                               */
/* ------------------------------------------------------------------ */
static int Targets(struct Target *out)
{
    int n = 0;
    unsigned s, i;
    const struct TcgPlayer *me = &B.view.players[0];
#define ADD(k, idx, rect)                                                                                              \
    do                                                                                                                 \
    {                                                                                                                  \
        out[n].kind = (k);                                                                                             \
        out[n].index = (idx);                                                                                          \
        out[n].r = (rect);                                                                                             \
        n++;                                                                                                           \
    } while (0)
    if (B.tray > 0.5f || B.mode == M_FREE)
        for (i = 0; i < me->handSize; i++)
            ADD(T_HAND, (int)i, HandRect(i, me->handSize));
    for (s = 0; s < TCG_FIELD_SIZE; s++)
    {
        ADD(T_MINE, (int)s, SlotRect(0, s));
        ADD(T_THEIRS, (int)s, SlotRect(1, s));
    }
    ADD(T_MY_DECK, 0, DeckRect(0));
    ADD(T_MY_DISCARD, 0, DiscardRect(0));
    ADD(T_OPP_DECK, 0, DeckRect(1));
    ADD(T_OPP_DISCARD, 0, DiscardRect(1));
    ADD(T_MY_PRIZES, 0, PrizeArea(0));
    ADD(T_OPP_PRIZES, 0, PrizeArea(1));
    ADD(T_OPP_HAND, 0, sOppHand);
#undef ADD
    return n;
}
static int Valid(int kind, int index);
static struct Rect FocusRect(void)
{
    struct Target t[40];
    int n = Targets(t), i;
    for (i = 0; i < n; i++)
        if (t[i].kind == B.focusKind && t[i].index == B.focusIndex)
            return t[i].r;
    return MakeRect(ACT_X, MY_ACT_Y, ACT_W, ACT_H);
}
static void Navigate(int dx, int dy)
{
    struct Target t[40];
    int n = Targets(t), i, best = -1;
    float bestScore = 1e9f;
    struct Rect cur = FocusRect();
    float cx = cur.x + cur.w / 2.0f, cy = cur.y + cur.h / 2.0f;
    /* In the hand, left/right walks the cards and up leaves it. */
    if (B.focusKind == T_HAND && dx && B.mode != M_TARGET)
    {
        int count = B.view.players[0].handSize;
        if (count)
        {
            B.focusIndex = (B.focusIndex + dx + count) % count;
            PlaySE(SE_RG_BAG_CURSOR);
        }
        return;
    }
    for (i = 0; i < n; i++)
    {
        float tx = t[i].r.x + t[i].r.w / 2.0f, ty = t[i].r.y + t[i].r.h / 2.0f, along, across, score;
        if (t[i].kind == B.focusKind && t[i].index == B.focusIndex)
            continue;
        if (B.mode == M_TARGET && !Valid(t[i].kind, t[i].index))
            continue;
        if (t[i].kind == T_HAND && B.mode == M_TARGET)
            continue;
        /* Leaving the hand goes up to the bench; entering it needs DOWN. */
        if (t[i].kind == T_HAND && (dy <= 0 || B.focusKind == T_HAND))
            continue;
        if (B.focusKind == T_HAND && t[i].kind != T_HAND && dy >= 0)
            continue;
        along = dx ? (tx - cx) * dx : (ty - cy) * dy;
        across = dx ? fabsf(ty - cy) : fabsf(tx - cx);
        if (along <= 2)
            continue;
        if (B.focusKind == T_HAND && t[i].kind != T_HAND)
            along = 1, across = fabsf(tx - cx);
        score = along + across * 2.2f;
        if (score < bestScore)
        {
            bestScore = score;
            best = i;
        }
    }
    if (best < 0)
        return;
    if (t[best].kind == T_HAND)
    {
        /* Re-enter the hand on the card nearest the cursor. */
        int count = B.view.players[0].handSize, k, pick = 0;
        float d = 1e9f;
        for (k = 0; k < count; k++)
        {
            struct Rect r = HandRect(k, count);
            float dd = fabsf(r.x + r.w / 2.0f - cx);
            if (dd < d)
                d = dd, pick = k;
        }
        B.focusKind = T_HAND;
        B.focusIndex = pick;
        B.trayWanted = 1;
    }
    else
    {
        B.focusKind = t[best].kind;
        B.focusIndex = t[best].index;
    }
    PlaySE(SE_RG_BAG_CURSOR);
}

/* ------------------------------------------------------------------ */
/* Choices                                                              */
/* ------------------------------------------------------------------ */
static const struct Step *TrainerSteps(unsigned id, int *count)
{
#define STEPS(...)                                                                                                     \
    do                                                                                                                 \
    {                                                                                                                  \
        static const struct Step s[] = {__VA_ARGS__};                                                                  \
        *count = sizeof(s) / sizeof(s[0]);                                                                             \
        return s;                                                                                                      \
    } while (0)
    switch (id)
    {
    case TCG_CARD_POTION:
        STEPS({ST_MY_SLOT, F_DAMAGED, 0, 0, 0, "Remove 2 damage counters from which Pokemon?"});
    case TCG_CARD_SUPER_POTION:
        STEPS({ST_MY_SLOT, F_DAMAGED_ENERGY, 0, 0, 0, "Heal which Pokemon? (discard 1 Energy from it)"},
              {ST_PICK, F_ENERGY, SRC_MY_ATTACHED, 1, 1, "Discard which Energy?"});
    case TCG_CARD_DEFENDER:
        STEPS({ST_MY_SLOT, F_OCCUPIED, 0, 0, 0, "Attach DEFENDER to which Pokemon?"});
    case TCG_CARD_SWITCH:
        STEPS({ST_MY_SLOT, F_BENCH, 0, 0, 0, "Switch in which Benched Pokemon?"});
    case TCG_CARD_GUST_OF_WIND:
        STEPS({ST_OPP_SLOT, F_BENCH, 0, 0, 0, "Bring out which of their Benched Pokemon?"});
    case TCG_CARD_SCOOP_UP:
        STEPS({ST_MY_SLOT, F_OCCUPIED, 0, 0, 0, "Return which Pokemon to your hand?"});
    case TCG_CARD_MR_FUJI:
        STEPS({ST_MY_SLOT, F_BENCH, 0, 0, 0, "Shuffle which Benched Pokemon into your deck?"});
    case TCG_CARD_CLEFAIRY_DOLL:
    case TCG_CARD_MYSTERIOUS_FOSSIL:
        STEPS({ST_MY_SLOT, F_EMPTY_BENCH, 0, 0, 0, "Put it on which Bench space?"});
    case TCG_CARD_DEVOLUTION_SPRAY:
        STEPS({ST_MY_SLOT, F_EVOLVED, 0, 0, 0, "Devolve which Pokemon?"}, {ST_STAGE, 0, 0, 0, 0, "Discard how far?"});
    case TCG_CARD_ENERGY_REMOVAL:
        STEPS({ST_OPP_SLOT, F_HAS_ENERGY, 0, 0, 0, "Remove Energy from which Pokemon?"},
              {ST_PICK, F_ENERGY, SRC_OPP_ATTACHED, 1, 1, "Discard which Energy?"});
    case TCG_CARD_SUPER_ENERGY_REMOVAL:
        STEPS({ST_MY_SLOT, F_HAS_ENERGY, 0, 0, 0, "First, discard Energy from one of yours."},
              {ST_PICK, F_ENERGY, SRC_MY_ATTACHED, 1, 1, "Discard which of your Energy?"},
              {ST_OPP_SLOT, F_HAS_ENERGY, 0, 0, 0, "Then remove Energy from which of theirs?"},
              {ST_PICK, F_ENERGY, SRC_OPP_ATTACHED, 1, 2, "Discard up to 2 of their Energy."});
    case TCG_CARD_ENERGY_SEARCH:
        STEPS({ST_PICK, F_BASIC_ENERGY, SRC_DECK, 1, 1, "Take which basic Energy?"});
    case TCG_CARD_POKE_BALL:
        STEPS({ST_PICK, F_POKEMON, SRC_DECK, 1, 1, "If heads, take which Pokemon?"});
    case TCG_CARD_COMPUTER_SEARCH:
        STEPS({ST_PICK, F_ANY, SRC_HAND, 2, 2, "Discard 2 cards from your hand."},
              {ST_PICK, F_ANY, SRC_DECK, 1, 1, "Take any 1 card from your deck."});
    case TCG_CARD_ITEM_FINDER:
        STEPS({ST_PICK, F_ANY, SRC_HAND, 2, 2, "Discard 2 cards from your hand."},
              {ST_PICK, F_TRAINER, SRC_MY_DISCARD, 1, 1, "Take which Trainer from your discard pile?"});
    case TCG_CARD_ENERGY_RETRIEVAL:
        STEPS({ST_PICK, F_ANY, SRC_HAND, 1, 1, "Discard 1 card from your hand."},
              {ST_PICK, F_BASIC_ENERGY, SRC_MY_DISCARD, 1, 2, "Take up to 2 basic Energy."});
    case TCG_CARD_SUPER_ENERGY_RETRIEVAL:
        STEPS({ST_PICK, F_ANY, SRC_HAND, 2, 2, "Discard 2 cards from your hand."},
              {ST_PICK, F_BASIC_ENERGY, SRC_MY_DISCARD, 1, 4, "Take up to 4 basic Energy."});
    case TCG_CARD_MAINTENANCE:
        STEPS({ST_PICK, F_ANY, SRC_HAND, 2, 2, "Shuffle 2 cards into your deck."});
    case TCG_CARD_RECYCLE:
        STEPS({ST_PICK, F_ANY, SRC_MY_DISCARD, 1, 1, "If heads, put which card on your deck?"});
    case TCG_CARD_POKEMON_TRADER:
        STEPS({ST_PICK, F_POKEMON, SRC_HAND, 1, 1, "Trade away which Pokemon?"},
              {ST_PICK, F_POKEMON, SRC_DECK, 1, 1, "Take which Pokemon from your deck?"});
    case TCG_CARD_POKEMON_BREEDER:
        STEPS({ST_PICK, F_STAGE2, SRC_HAND, 1, 1, "Play which Stage 2 card?"},
              {ST_MY_SLOT, F_BREEDER_BASIC, 0, 0, 0, "Put it on which Basic Pokemon?"});
    case TCG_CARD_REVIVE:
        STEPS({ST_PICK, F_BASIC, SRC_MY_DISCARD, 1, 1, "Revive which Basic Pokemon?"},
              {ST_MY_SLOT, F_EMPTY_BENCH, 0, 0, 0, "Put it on which Bench space?"});
    case TCG_CARD_POKEMON_FLUTE:
        STEPS({ST_PICK, F_BASIC, SRC_OPP_DISCARD, 1, 1, "Put which of their Basic Pokemon in play?"},
              {ST_OPP_SLOT, F_EMPTY_BENCH, 0, 0, 0, "Put it on which of their Bench spaces?"});
    case TCG_CARD_POKEDEX:
        STEPS({ST_PICK, F_ANY, SRC_TOP5, 0, 0, "Choose the order: the first is drawn next."});
    }
    *count = 0;
    return sNoSteps;
}
/* A used Pokemon Power's choices: the first Pokemon chosen is action.target, the
 * second action.attack (src/tcg/tcg_powers.c TcgDuel_Power) */
static const struct Step *PowerSteps(unsigned power, int *count)
{
    switch (power)
    {
    case TCG_POWER_ENERGY_TRANS:
        STEPS({ST_MY_SLOT, F_HAS_GRASS, 0, 0, 0, "Move a Grass Energy from which Pokemon?"},
              {ST_MY_SLOT, F_OTHER, 0, 0, 0, "Move it to which Pokemon?"});
    case TCG_POWER_HEAL:
        STEPS({ST_MY_SLOT, F_DAMAGED, 0, 0, 0, "Heal which Pokemon? (a coin: heads, 1 counter)"});
    case TCG_POWER_SHIFT:
        STEPS({ST_CHOICE, 0, 0, 0, 0, "Change its type to which?"});
    case TCG_POWER_RAIN_DANCE:
        STEPS({ST_MY_SLOT, F_WATER_POKEMON, 0, 0, 0, "Attach a Water Energy to which Water Pokemon?"});
    case TCG_POWER_PEEK:
        STEPS({ST_CHOICE, 0, 0, 0, 0, "Look at which card?"});
    case TCG_POWER_DAMAGE_SWAP:
        STEPS({ST_MY_SLOT, F_DAMAGED, 0, 0, 0, "Move a damage counter from which Pokemon?"},
              {ST_MY_SLOT, F_ROOM_OTHER, 0, 0, 0, "Move it to which Pokemon?"});
    case TCG_POWER_STRANGE_BEHAVIOR:
        STEPS({ST_MY_SLOT, F_DAMAGED_OTHER, 0, 0, 0, "Move a damage counter from which Pokemon?"});
    case TCG_POWER_CURSE:
        STEPS({ST_OPP_SLOT, F_DAMAGED, 0, 0, 0, "Move a damage counter from which of their Pokemon?"},
              {ST_OPP_SLOT, F_OTHER, 0, 0, 0, "Move it to which of their Pokemon?"});
    }
    *count = 0;
    return sNoSteps;
}
#undef STEPS
static int CardMatches(unsigned owner, unsigned physical, int filter)
{
    const struct TcgCard *c = Tcg_Card(CardOf(owner, physical));
    if (!c)
        return 0;
    switch (filter)
    {
    case F_POKEMON:
        return c->kind == TCG_POKEMON;
    case F_BASIC:
        return c->kind == TCG_POKEMON && !c->stage;
    case F_BASIC_ENERGY:
        return c->kind == TCG_ENERGY && c->id <= 6;
    case F_ENERGY:
        return c->kind == TCG_ENERGY;
    case F_TRAINER:
        return c->kind == TCG_TRAINER;
    case F_STAGE2:
        return c->kind == TCG_POKEMON && c->stage == 2;
    default:
        return 1;
    }
}
static int SlotMatches(unsigned who, unsigned slot, int filter)
{
    const struct TcgPlayer *p = &B.view.players[who];
    const struct TcgFieldCard *f = &p->field[slot];
    const struct TcgCard *c = Field(who, slot);
    switch (filter)
    {
    case F_ANY:
        return 1;
    case F_OCCUPIED:
        return c != NULL;
    case F_BENCH:
        return slot && c;
    case F_EMPTY_BENCH:
        return slot && !c;
    case F_DAMAGED:
        return c && f->damage;
    case F_DAMAGED_ENERGY:
        return c && f->damage && HasEnergy(who, slot);
    case F_HAS_ENERGY:
        return c && HasEnergy(who, slot);
    case F_EVOLVED:
        return c && f->stackSize >= 2;
    case F_BREEDER_BASIC:
    {
        const struct TcgCard *stage2 = B.action.selectionCount ? Tcg_Card(CardOf(0, B.action.selections[0])) : NULL;
        unsigned i;
        if (!c || c->kind != TCG_POKEMON || c->stage || !stage2)
            return 0;
        for (i = 1; i <= TCG_CARD_COUNT; i++)
            if (gTcgCards[i].kind == TCG_POKEMON && gTcgCards[i].stage == 1 &&
                !strcmp(gTcgCards[i].name, stage2->evolvesFrom) && !strcmp(gTcgCards[i].evolvesFrom, c->name))
                return 1;
        return 0;
    }
    case F_EVOLVE_TARGET:
    {
        const struct TcgCard *e = Tcg_Card(B.flowCard);
        return c && e && c->kind == TCG_POKEMON && !strcmp(e->evolvesFrom, c->name);
    }
    case F_BASIC_SLOT:
        return !c && (slot || B.view.phase == TCG_SETUP);
    case F_ANY_POKEMON:
        return c && c->kind == TCG_POKEMON;
    case F_HAS_GRASS:
    {
        unsigned i;
        for (i = 0; c && i < f->attachedSize; i++)
            if (CardOf(who, f->attached[i]) == TCG_CARD_GRASS_ENERGY)
                return 1;
        return 0;
    }
    case F_OTHER:
        return c && slot != B.action.target;
    case F_WATER_POKEMON:
        return c && c->kind == TCG_POKEMON && TcgDuel_Type(&B.view, who, slot) == TCG_WATER;
    case F_ROOM_OTHER:
        return c && slot != B.action.target && f->damage + 10u < (c->kind == TCG_TRAINER ? 10u : c->hp);
    case F_DAMAGED_OTHER:
        return c && f->damage && slot != B.action.slot;
    }
    return 0;
}
static int TargetFilter(int *who)
{
    *who = 0;
    switch (B.flow)
    {
    case FLOW_BASIC:
        return F_BASIC_SLOT;
    case FLOW_EVOLVE:
        return F_EVOLVE_TARGET;
    case FLOW_ATTACH:
        return F_ANY_POKEMON;
    case FLOW_RETREAT:
        return F_BENCH;
    case FLOW_ATTACK:
    case FLOW_TRAINER:
    case FLOW_POWER:
        if (B.step < B.nsteps)
        {
            *who = B.steps[B.step].kind == ST_OPP_SLOT;
            return B.steps[B.step].filter;
        }
    }
    return F_ANY;
}
static int Valid(int kind, int index)
{
    int who, filter;
    if (B.mode != M_TARGET)
        return 1;
    filter = TargetFilter(&who);
    if (kind != (who ? T_THEIRS : T_MINE))
        return 0;
    return SlotMatches(who, index, filter);
}
static void CancelFlow(void)
{
    B.flow = FLOW_NONE;
    B.mode = M_FREE;
    B.heldHand = -1;
    B.step = 0;
}
static void Submit(struct TcgAction *out, int *result)
{
    B.refocus = B.action.type + 1;
    B.refocusHand = B.action.hand;
    *out = B.action;
    *result = BOARD_ACTION;
    B.flow = FLOW_NONE;
    B.mode = M_FREE;
    B.heldHand = -1;
    PlaySE(SE_SELECT);
}
static int FocusFirstValid(void)
{
    struct Target t[40];
    int n = Targets(t), i;
    if (Valid(B.focusKind, B.focusIndex) && (B.focusKind == T_MINE || B.focusKind == T_THEIRS))
        return 1;
    for (i = 0; i < n; i++)
        if ((t[i].kind == T_MINE || t[i].kind == T_THEIRS) && Valid(t[i].kind, t[i].index))
        {
            B.focusKind = t[i].kind;
            B.focusIndex = t[i].index;
            return 1;
        }
    return 0;
}
static void OpenPick(int source, int filter, int min, int max, const char *title);
static void ContinueTrainer(struct TcgAction *out, int *result);
static void PowerChoices(void);
static void AttackOptionMenu(void);
static void Menu(int x, int y);
static void AddMenu(const char *label, int id, int enabled, const char *reason);

/* An attack's choices as steps (the Trainers' machinery): its menu first (TcgDuel_AttackOptions), then what
   that menu decides (AttackStepsAfterOption) */
static struct Step sAttackSteps[4];
static char sAttackPrompt[64];
static int BenchOn(unsigned who)
{
    unsigned s;
    for (s = 1; s < TCG_FIELD_SIZE; s++)
        if (Field(who, s))
            return 1;
    return 0;
}
static void AttackStepsAfterOption(const struct TcgAttack *a)
{
    unsigned choice = a ? TcgDuel_AttackChoice(a, NULL) : 0, ask = TcgDuel_AttackAsk(a);
    int n = 0;
    if ((choice == 1 && BenchOn(1)) || (choice == 2 && BenchOn(0)))
    {
        struct Step st = {choice == 1 ? ST_OPP_SLOT : ST_MY_SLOT, F_BENCH, 0, 0, 0,
                          choice == 1 ? "Aim at which of their Benched Pokemon?" : "Switch with which of your Benched Pokemon?"};
        sAttackSteps[n++] = st;
    }
    if (ask == TCG_ASK_DISCARD_TRAINER)
    {
        struct Step st = {ST_PICK, F_TRAINER, SRC_MY_DISCARD, 1, 1, "Put which Trainer into your hand?"};
        sAttackSteps[n++] = st;
    }
    else if (ask == TCG_ASK_ORDER_TOP3)
    {
        struct Step st = {ST_PICK, F_ANY, B.action.option ? SRC_TOP3_THEIRS : SRC_TOP3_MINE, 0, 3,
                          "Put them back in order: the first chosen goes on top."};
        sAttackSteps[n++] = st;
    }
    else if (ask == TCG_ASK_EVOLVED)
    {
        struct Step st = {B.action.option ? ST_OPP_SLOT : ST_MY_SLOT, F_EVOLVED, 0, 0, 0, "Devolve which Pokemon?"};
        sAttackSteps[n++] = st;
    }
    B.steps = sAttackSteps;
    B.nsteps = n;
}
static void AttackSteps(const struct TcgAttack *a)
{
    const char *labels[TCG_MAX_OPTIONS];
    uint8_t ok[TCG_MAX_OPTIONS];
    if (a && TcgDuel_AttackOptions(&B.view, 0, a, labels, ok))
    {
        struct Step st = {ST_ATTACK_OPTION, 0, 0, 0, 0, sAttackPrompt};
        static const char *const prompts[][2] = {
            {"Metronome", "Copy which of their attacks?"},
            {"Amnesia", "Which attack can't they use next turn?"},
            {"Mirror Move", "Which attack can't they use next turn?"},
            {"Wildfire", "Discard how many Fire Energy?"},
            {"Conversion 1", "Change their Weakness to which type?"},
            {"Conversion 2", "Change its Resistance to which type?"},
            {"Prophecy", "Look at the top of which deck?"},
            {"Devolution Beam", "Devolve whose Pokemon?"},
        };
        unsigned k;
        snprintf(sAttackPrompt, sizeof(sAttackPrompt), "%s", "Choose for the attack.");
        for (k = 0; k < sizeof(prompts) / sizeof(prompts[0]); k++)
            if (!strcmp(a->name, prompts[k][0]))
                snprintf(sAttackPrompt, sizeof(sAttackPrompt), "%s", prompts[k][1]);
        sAttackSteps[0] = st;
        B.steps = sAttackSteps;
        B.nsteps = 1;
        return;
    }
    AttackStepsAfterOption(a);
}
static void AttackOptionMenu(void)
{
    const struct TcgAttack *a = &TcgDuel_Attacker(&B.view, 0)->attacks[B.action.attack];
    const char *labels[TCG_MAX_OPTIONS];
    uint8_t ok[TCG_MAX_OPTIONS];
    unsigned n, i;
    if (TcgDuel_IsMetronome(a) && B.action.copy)
        a = TcgDuel_MetronomeAttack(&B.view, 0, B.action.copy);
    n = a ? TcgDuel_AttackOptions(&B.view, 0, a, labels, ok) : 0;
    B.menuCount = 0;
    for (i = 0; i < n; i++)
        AddMenu(labels[i], MENU_CHOICE + (int)i, ok[i], "That can't be chosen.");
    printf("[tcg] attack menu: %u choices\n", n);
    Menu(150, 60);
}
static void StartTarget(void)
{
    B.mode = M_TARGET;
    if (!FocusFirstValid())
    {
        Toast("There is nothing that can be chosen.");
        PlaySE(SE_FAILURE);
        CancelFlow();
    }
}
static void ContinueTrainer(struct TcgAction *out, int *result)
{
    const struct Step *st;
    if (B.step >= B.nsteps)
    {
        Submit(out, result);
        return;
    }
    st = &B.steps[B.step];
    if (st->kind == ST_MY_SLOT || st->kind == ST_OPP_SLOT)
    {
        StartTarget();
        return;
    }
    if (st->kind == ST_PICK)
    {
        int n = st->max, all = st->source == SRC_TOP5 || st->source == SRC_TOP3_MINE || st->source == SRC_TOP3_THEIRS;
        if (st->source == SRC_TOP5)
            n = B.view.players[0].deckSize < 5 ? B.view.players[0].deckSize : 5;
        if (st->source == SRC_TOP3_MINE || st->source == SRC_TOP3_THEIRS)
        {
            n = B.view.players[st->source == SRC_TOP3_THEIRS].deckSize;
            n = n < 3 ? n : 3;
        }
        OpenPick(st->source, st->filter, all ? n : st->min, n, st->prompt);
        return;
    }
    if (st->kind == ST_ATTACK_OPTION)
    {
        AttackOptionMenu();
        return;
    }
    if (st->kind == ST_STAGE)
    {
        const struct TcgCard *top = Field(0, B.action.slot);
        if (top && top->stage == 2)
        {
            B.mode = M_MENU;
            B.menuCount = 2;
            B.menuCursor = 0;
            B.menu[0].label = "TO STAGE 1";
            B.menu[0].id = MENU_STAGE2;
            B.menu[0].enabled = 1;
            B.menu[1].label = "TO BASIC";
            B.menu[1].id = MENU_STAGE1;
            B.menu[1].enabled = 1;
            B.menuX = 150;
            B.menuY = 90;
            return;
        }
        B.action.attack = 1;
        B.step++;
        ContinueTrainer(out, result);
        return;
    }
    if (st->kind == ST_CHOICE)
        PowerChoices();
}

/* Picker over a pile: the hand, the deck while searching, attached Energy... */
static void OpenPick(int source, int filter, int min, int max, const char *title)
{
    const struct TcgPlayer *p;
    unsigned owner = source == SRC_OPP_DISCARD || source == SRC_OPP_ATTACHED || source == SRC_TOP3_THEIRS, i, n = 0;
    const uint8_t *pile = NULL;
    unsigned count = 0;
    uint8_t sorted[TCG_DECK_SIZE];
    p = &B.view.players[owner];
    switch (source)
    {
    case SRC_HAND:
        pile = p->hand;
        count = p->handSize;
        break;
    case SRC_DECK:
        pile = p->deck;
        count = p->deckSize;
        break;
    case SRC_MY_DISCARD:
    case SRC_OPP_DISCARD:
        pile = p->discard;
        count = p->discardSize;
        break;
    case SRC_MY_ATTACHED:
        pile = p->field[B.action.slot].attached;
        count = p->field[B.action.slot].attachedSize;
        break;
    case SRC_OPP_ATTACHED:
        pile = p->field[B.action.target].attached;
        count = p->field[B.action.target].attachedSize;
        break;
    case SRC_TOP5:
    case SRC_TOP3_MINE:
    case SRC_TOP3_THEIRS:
        for (i = 0; i < (unsigned)max; i++)
            sorted[i] = p->deck[p->deckSize - 1 - i];
        pile = sorted;
        count = max;
        break;
    }
    memset(B.pickSel, 0, sizeof(B.pickSel));
    for (i = 0; i < count; i++)
    {
        unsigned physical = pile[i];
        if (source == SRC_HAND && B.flow == FLOW_TRAINER && (int)i == B.action.hand)
            continue;
        B.pickCards[n++] = (uint8_t)physical;
    }
    if (source == SRC_DECK)
    {
        /* A searched deck is shown in catalogue order, never its real order. */
        unsigned a, b;
        for (a = 0; a < n; a++)
            for (b = a + 1; b < n; b++)
                if (CardOf(owner, B.pickCards[b]) < CardOf(owner, B.pickCards[a]))
                {
                    uint8_t t = B.pickCards[a];
                    B.pickCards[a] = B.pickCards[b];
                    B.pickCards[b] = t;
                }
    }
    B.pickCount = (uint8_t)n;
    B.pickOwner = (uint8_t)owner;
    B.pickSource = source;
    B.pickFilter = filter;
    B.pickMin = min;
    B.pickMax = max;
    B.pickChosen = 0;
    B.pickCursor = 0;
    B.pickScroll = 0;
    snprintf(B.pickTitle, sizeof(B.pickTitle), "%s", title);
    for (i = 0; i < n; i++)
        if (CardMatches(owner, B.pickCards[i], filter))
        {
            B.pickCursor = i;
            break;
        }
    B.mode = M_PICK;
    if (min > 0)
    {
        unsigned valid = 0;
        for (i = 0; i < n; i++)
            valid += CardMatches(owner, B.pickCards[i], filter);
        if (valid < (unsigned)min && source != SRC_DECK)
        {
            Toast("There aren't enough cards to choose.");
            PlaySE(SE_FAILURE);
            CancelFlow();
        }
    }
}
static void OpenViewer(unsigned owner)
{
    const struct TcgPlayer *p = &B.view.players[owner];
    unsigned i;
    for (i = 0; i < p->discardSize; i++)
        B.pickCards[i] = p->discard[p->discardSize - 1 - i];
    B.pickCount = p->discardSize;
    B.pickOwner = (uint8_t)owner;
    B.pickCursor = B.pickScroll = 0;
    B.pickMin = B.pickMax = 0;
    B.pickFilter = F_ANY;
    snprintf(B.pickTitle, sizeof(B.pickTitle), "%s", owner ? "THEIR DISCARD PILE" : "YOUR DISCARD PILE");
    B.mode = M_VIEW;
}
/* Omanyte's Clairvoyance: its player plays with the opponent's hand face up */
static int Clairvoyant(void) { return TcgPower_InPlay(&B.view, 0, TCG_POWER_CLAIRVOYANCE); }
static void OpenHandViewer(void)
{
    const struct TcgPlayer *p = &B.view.players[1];
    unsigned i;
    for (i = 0; i < p->handSize; i++)
        B.pickCards[i] = p->hand[i];
    B.pickCount = p->handSize;
    B.pickOwner = 1;
    B.pickCursor = B.pickScroll = 0;
    B.pickMin = B.pickMax = 0;
    B.pickFilter = F_ANY;
    snprintf(B.pickTitle, sizeof(B.pickTitle), "THEIR HAND (CLAIRVOYANCE)");
    B.mode = M_VIEW;
}
static void FinishPick(struct TcgAction *out, int *result)
{
    unsigned i, value = 0;
    if (B.pickRetreat)
    {
        B.pickRetreat = 0;
        B.action.selectionCount = 0;
        for (i = 0; i < B.pickChosen; i++)
            B.action.selections[B.action.selectionCount++] = B.pickOrder[i];
        Submit(out, result);
        return;
    }
    (void)value;
    for (i = 0; i < B.pickChosen; i++)
        B.action.selections[B.action.selectionCount++] = B.pickOrder[i];
    B.step++;
    B.mode = M_FREE;
    ContinueTrainer(out, result);
}
static void BeginRetreat(void)
{
    char reason[80];
    if (!CanRetreat(reason, sizeof(reason)))
    {
        TcgBoard_Reject(reason);
        return;
    }
    memset(&B.action, 0, sizeof(B.action));
    B.action.type = TCG_RETREAT;
    B.flow = FLOW_RETREAT;
    B.heldHand = -1;
    StartTarget();
}
static void RetreatCost(struct TcgAction *out, int *result)
{
    const struct TcgFieldCard *f = &B.view.players[0].field[0];
    unsigned i, total = 0, types = 0, first = TCG_NONE, cost = RetreatCostNow();
    for (i = 0; i < f->attachedSize; i++)
    {
        const struct TcgCard *e = Tcg_Card(CardOf(0, f->attached[i]));
        if (e->kind != TCG_ENERGY)
            continue;
        total += EnergyValue(e->id);
        if (first == TCG_NONE || e->id != first)
            types++;
        if (first == TCG_NONE)
            first = e->id;
    }
    B.action.selectionCount = 0;
    if (!cost || total == cost || types == 1)
    {
        /* Nothing to choose: pay with the first Energy that covers it. */
        unsigned paid = 0;
        for (i = 0; i < f->attachedSize && paid < cost; i++)
        {
            const struct TcgCard *e = Tcg_Card(CardOf(0, f->attached[i]));
            if (e->kind == TCG_ENERGY)
            {
                B.action.selections[B.action.selectionCount++] = f->attached[i];
                paid += EnergyValue(e->id);
            }
        }
        Submit(out, result);
        return;
    }
    {
        uint8_t slot = B.action.slot;
        B.action.slot = 0;
        OpenPick(SRC_MY_ATTACHED, F_ENERGY, 1, (int)cost, "Discard Energy to retreat.");
        B.action.slot = slot;
        B.pickRetreat = (int)cost;
    }
}

static void Menu(int x, int y)
{
    B.menuX = x;
    B.menuY = y;
    B.menuCursor = 0;
    while (B.menuCursor < B.menuCount - 1 && !B.menu[B.menuCursor].enabled)
        B.menuCursor++;
    B.mode = M_MENU;
    PlaySE(SE_SELECT);
}
static void AddMenu(const char *label, int id, int enabled, const char *reason)
{
    struct MenuItem *m = &B.menu[B.menuCount++];
    m->label = label;
    m->id = id;
    m->enabled = enabled;
    snprintf(m->reason, sizeof(m->reason), "%s", reason ? reason : "");
}
static const char *HandVerb(const struct TcgCard *c)
{
    return c->kind == TCG_ENERGY ? "ATTACH" : c->kind == TCG_TRAINER ? "PLAY" : c->stage ? "EVOLVE" : "PLAY";
}
static int HandPlayable(const struct TcgCard *c, char *reason, size_t size)
{
    const struct TcgPlayer *p = &B.view.players[0];
    unsigned s;
    if (!CanAct())
        return snprintf(reason, size, "%s", MyTurn() ? "Wait a moment." : "It isn't your turn."), 0;
    if (B.view.phase == TCG_PROMOTION)
        return snprintf(reason, size, "Choose your next Active Pokemon first."), 0;
    if (B.view.phase == TCG_SETUP)
    {
        if (c->kind != TCG_POKEMON || c->stage)
            return snprintf(reason, size, "Only Basic Pokemon are placed now."), 0;
        for (s = 0; s < TCG_FIELD_SIZE; s++)
            if (!p->field[s].stackSize)
                return 1;
        return snprintf(reason, size, "Your Bench is full."), 0;
    }
    if (c->kind == TCG_ENERGY && p->energyPlayed)
        return snprintf(reason, size, "You already attached Energy this turn."), 0;
    if (c->kind == TCG_POKEMON && !c->stage)
    {
        for (s = 1; s < TCG_FIELD_SIZE; s++)
            if (!p->field[s].stackSize)
                return 1;
        return snprintf(reason, size, "Your Bench is full."), 0;
    }
    if (c->kind == TCG_POKEMON)
    {
        for (s = 0; s < TCG_FIELD_SIZE; s++)
        {
            const struct TcgCard *f = Field(0, s);
            if (f && f->kind == TCG_POKEMON && !strcmp(c->evolvesFrom, f->name))
            {
                if (p->turnsTaken < 2 || p->field[s].enteredTurn == B.view.turn)
                    return snprintf(reason, size, "%s can't evolve on its first turn in play.", f->name), 0;
                return 1;
            }
        }
        return snprintf(reason, size, "You have no %s in play.", c->evolvesFrom), 0;
    }
    if (c->kind == TCG_TRAINER && p->noTrainers)
        return snprintf(reason, size, "Headache: no Trainer cards this turn."), 0;
    return 1;
}
/* Would this Trainer do anything now? Only the plain cases say no (nothing to heal, nothing to switch to, an empty
   deck...); anything unsure counts as playable. */
static int Has(unsigned who, int (*test)(const struct TcgFieldCard *f, unsigned who), int benchOnly)
{
    unsigned s;
    for (s = benchOnly ? 1 : 0; s < TCG_FIELD_SIZE; s++)
        if (B.view.players[who].field[s].stackSize && test(&B.view.players[who].field[s], who))
            return 1;
    return 0;
}
static int AnyCard(const struct TcgFieldCard *f, unsigned who) { (void)f, (void)who; return 1; }
static int Damaged(const struct TcgFieldCard *f, unsigned who) { (void)who; return f->damage > 0; }
static int Evolved(const struct TcgFieldCard *f, unsigned who) { (void)who; return f->stackSize > 1; }
static int WithEnergy(const struct TcgFieldCard *f, unsigned who)
{
    unsigned i;
    for (i = 0; i < f->attachedSize; i++)
        if (Tcg_Card(B.view.players[who].cards[f->attached[i]])->kind == TCG_ENERGY)
            return 1;
    return 0;
}
static int DamagedWithEnergy(const struct TcgFieldCard *f, unsigned who) { return f->damage && WithEnergy(f, who); }
static int InDiscard(int kind, int basicOnly)
{
    const struct TcgPlayer *p = &B.view.players[0];
    unsigned i;
    for (i = 0; i < p->discardSize; i++)
    {
        const struct TcgCard *c = Tcg_Card(p->cards[p->discard[i]]);
        if (c->kind == kind && (!basicOnly || (kind == TCG_POKEMON ? !c->stage : p->cards[p->discard[i]] <= 6)))
            return 1;
    }
    return 0;
}
static int TrainerHasUse(unsigned id)
{
    const struct TcgPlayer *p = &B.view.players[0];
    int bench = 0;
    unsigned s;
    for (s = 1; s < TCG_FIELD_SIZE; s++)
        bench += !p->field[s].stackSize;
    switch (id)
    {
    case TCG_CARD_POTION:
    case TCG_CARD_POKEMON_CENTER:
        return Has(0, Damaged, 0);
    case TCG_CARD_SUPER_POTION:
        return Has(0, DamagedWithEnergy, 0);
    case TCG_CARD_FULL_HEAL:
        return p->field[0].stackSize && (p->field[0].condition || p->field[0].poison);
    case TCG_CARD_SWITCH:
        return Has(0, AnyCard, 1);
    case TCG_CARD_GUST_OF_WIND:
        return Has(1, AnyCard, 1);
    case TCG_CARD_ENERGY_REMOVAL:
        return Has(1, WithEnergy, 0);
    case TCG_CARD_SUPER_ENERGY_REMOVAL:
        return Has(1, WithEnergy, 0) && Has(0, WithEnergy, 0);
    case TCG_CARD_DEVOLUTION_SPRAY:
        return Has(0, Evolved, 0);
    case TCG_CARD_CLEFAIRY_DOLL:
    case TCG_CARD_MYSTERIOUS_FOSSIL:
        return bench > 0;
    case TCG_CARD_REVIVE:
        return bench > 0 && InDiscard(TCG_POKEMON, 1);
    case TCG_CARD_BILL:
    case TCG_CARD_POKEDEX:
    case TCG_CARD_POKE_BALL:
    case TCG_CARD_ENERGY_SEARCH:
        return p->deckSize > 0;
    case TCG_CARD_COMPUTER_SEARCH:
        return p->deckSize > 0 && p->handSize >= 3;
    case TCG_CARD_ITEM_FINDER:
        return p->handSize >= 3 && InDiscard(TCG_TRAINER, 0);
    case TCG_CARD_ENERGY_RETRIEVAL:
        return p->handSize >= 2 && InDiscard(TCG_ENERGY, 1);
    case TCG_CARD_SUPER_ENERGY_RETRIEVAL:
        return p->handSize >= 3 && InDiscard(TCG_ENERGY, 1);
    case TCG_CARD_MAINTENANCE:
        return p->handSize >= 3;
    }
    return 1;
}
static int HandPlayable(const struct TcgCard *c, char *reason, size_t size);
/* Is there still a card in your hand to play this turn? (a Basic with room for it, an evolution, Energy not yet
   attached, a Trainer with something to do) */
static int HandHasMove(void)
{
    const struct TcgPlayer *p = &B.view.players[0];
    unsigned h;
    char reason[80];
    for (h = 0; h < p->handSize; h++)
    {
        const struct TcgCard *c = Tcg_Card(p->cards[p->hand[h]]);
        if (HandPlayable(c, reason, sizeof(reason)) && (c->kind != TCG_TRAINER || TrainerHasUse(c->id)))
            return 1;
    }
    return 0;
}
/* Nothing left to play from the hand: the cursor goes to your Active Pokemon, where the attack is */
static void FocusActiveIfDone(void)
{
    if (B.mode != M_FREE || !MyTurn() || B.view.phase != TCG_TURN || !B.view.players[0].field[0].stackSize || HandHasMove())
        return;
    printf("[tcg] refocus: nothing left to play, your Active\n");
    B.focusKind = T_MINE;
    B.focusIndex = 0;
    B.trayWanted = 0;
}
static void OpenHandMenu(void)
{
    const struct TcgPlayer *p = &B.view.players[0];
    const struct TcgCard *c;
    char reason[80];
    struct Rect r;
    int ok;
    if ((unsigned)B.focusIndex >= p->handSize)
        return;
    c = Tcg_Card(p->cards[p->hand[B.focusIndex]]);
    ok = HandPlayable(c, reason, sizeof(reason));
    B.menuCount = 0;
    AddMenu(HandVerb(c), MENU_PLAY, ok, reason);
    AddMenu("CHECK", MENU_CHECK, 1, NULL);
    r = HandRect(B.focusIndex, p->handSize);
    Menu(r.x + r.w / 2 > 150 ? r.x - 60 : r.x + r.w + 2, (int)(BarY() + Pan()) - 34);
}
static unsigned InPlayCount(unsigned who)
{
    unsigned s, n = 0;
    for (s = 0; s < TCG_FIELD_SIZE; s++)
        n += Field(who, s) != NULL;
    return n;
}
/* The first of your Pokemon whose power can be used now, or -1 */
static int UsablePower(void)
{
    unsigned s;
    for (s = 0; s < TCG_FIELD_SIZE; s++)
        if (HasPower(Field(0, s)) && !TcgPower_Block(&B.view, 0, s))
            return (int)s;
    return -1;
}
static void BeginPower(unsigned slot, struct TcgAction *out, int *result)
{
    unsigned power = TcgPower_Id(Field(0, slot));
    const char *block = TcgPower_Block(&B.view, 0, slot);
    if (block)
    {
        TcgBoard_Reject(block);
        return;
    }
    memset(&B.action, 0, sizeof(B.action));
    B.action.type = TCG_USE_POWER;
    B.action.slot = (uint8_t)slot;
    B.flow = FLOW_POWER;
    B.heldHand = -1;
    B.mode = M_FREE;
    B.steps = PowerSteps(power, &B.nsteps);
    B.step = 0;
    if (power == TCG_POWER_RAIN_DANCE)
    {
        /* (any Water Energy card in the hand is as good as another) */
        const struct TcgPlayer *p = &B.view.players[0];
        unsigned i;
        for (i = 0; i < p->handSize && CardOf(0, p->hand[i]) != TCG_CARD_WATER_ENERGY; i++)
            ;
        B.action.hand = (uint8_t)i;
    }
    if (B.nsteps && B.steps[0].kind == ST_MY_SLOT)
        B.focusKind = T_MINE, B.focusIndex = 0;
    if (B.nsteps && B.steps[0].kind == ST_OPP_SLOT)
        B.focusKind = T_THEIRS, B.focusIndex = 0;
    ContinueTrainer(out, result);
}
/* SHIFT's types in play and PEEK's five places, as a menu */
static void PowerChoices(void)
{
    unsigned power = TcgPower_Id(Field(0, B.action.slot)), t, who, s;
    B.menuCount = 0;
    if (power == TCG_POWER_SHIFT)
    {
        static const char *const names[TCG_COLORLESS] = {"FIRE", "GRASS", "LIGHTNING", "WATER", "FIGHTING", "PSYCHIC"};
        for (t = 0; t < TCG_COLORLESS; t++)
        {
            int found = 0;
            for (who = 0; who < 2; who++)
                for (s = 0; s < TCG_FIELD_SIZE; s++)
                    found |= (who || s != B.action.slot) && Field(who, s) && Field(who, s)->kind == TCG_POKEMON &&
                             TcgDuel_Type(&B.view, who, s) == t;
            if (found && t != TcgDuel_Type(&B.view, 0, B.action.slot))
                AddMenu(names[t], MENU_CHOICE + (int)t, 1, NULL);
        }
        if (!B.menuCount)
        {
            TcgBoard_Reject("Every other type in play is already its own.");
            CancelFlow();
            return;
        }
    }
    else
    {
        const struct TcgPlayer *me = &B.view.players[0], *them = &B.view.players[1];
        AddMenu("TOP OF YOUR DECK", MENU_CHOICE + TCG_PEEK_MY_DECK, me->deckSize != 0, "Your deck is empty.");
        AddMenu("TOP OF THEIR DECK", MENU_CHOICE + TCG_PEEK_THEIR_DECK,
                them->deckSize != 0, "Their deck is empty.");
        AddMenu("A CARD IN THEIR HAND", MENU_CHOICE + TCG_PEEK_THEIR_HAND, them->handSize != 0, "Their hand is empty.");
        AddMenu("ONE OF YOUR PRIZES", MENU_CHOICE + TCG_PEEK_MY_PRIZE, me->prizeSize != 0, "You have no Prizes left.");
        AddMenu("ONE OF THEIR PRIZES", MENU_CHOICE + TCG_PEEK_THEIR_PRIZE, them->prizeSize != 0,
                "They have no Prizes left.");
    }
    {
        struct Rect r = SlotRect(0, B.action.slot);
        Menu(r.x + r.w + 4 > 150 ? r.x - 100 : r.x + r.w + 4, r.y > 100 ? 70 : r.y);
    }
}
static void OpenFieldMenu(void)
{
    const struct TcgCard *c = Field(0, B.focusIndex);
    char reason[80];
    struct Rect r = SlotRect(0, B.focusIndex);
    B.menuCount = 0;
    if (!c)
        return;
    if (B.focusIndex == 0 && B.view.phase == TCG_TURN)
    {
        int any = 0;
        unsigned i;
        char why[80] = "";
        for (i = 0; i < 2; i++)
            if (AttackReady(i, why, sizeof(why)))
                any = 1;
        if (!CanAct())
            snprintf(why, sizeof(why), "%s", MyTurn() ? "Wait a moment." : "It isn't your turn.");
        AddMenu("ATTACK", MENU_ATTACK, c->kind == TCG_POKEMON && CanAct(), any ? NULL : why);
        AddMenu("RETREAT", MENU_RETREAT, CanAct() && CanRetreat(reason, sizeof(reason)), reason);
    }
    if (HasPower(c))
    {
        const char *block = CanAct() ? TcgPower_Block(&B.view, 0, B.focusIndex)
                                     : MyTurn() ? "Wait a moment." : "It isn't your turn.";
        AddMenu("PKMN POWER", MENU_POWER, !block, block);
    }
    if (c->kind == TCG_TRAINER && B.view.phase == TCG_TURN)
    {
        /* Clefairy Doll / Mysterious Fossil: "you may discard it from play" */
        const char *why = !CanAct() ? (MyTurn() ? "Wait a moment." : "It isn't your turn.")
                          : InPlayCount(0) < 2 ? "It's your only Pokemon in play." : NULL;
        AddMenu("DISCARD", MENU_DISCARD, !why, why);
    }
    AddMenu("CHECK", MENU_CHECK, 1, NULL);
    Menu(r.x + r.w + 4 > 170 ? r.x - 74 : r.x + r.w + 4, r.y > 100 ? 92 : r.y);
}
/* The turn menu (B, START or SELECT on the table, as on the Yu-Gi-Oh! table): END TURN (READY while setting up), at
 * the bottom right. Everything else is done on the cards themselves; CONCEDE is A on your own deck. */
static void OpenTurnMenu(void)
{
    B.menuCount = 0;
    if (B.view.phase == TCG_SETUP)
        AddMenu("READY", MENU_DONE, MyTurn() && B.view.players[0].field[0].stackSize,
                MyTurn() ? "Choose your Active Pokemon first." : "Waiting for your opponent.");
    else
        AddMenu("END TURN", MENU_DONE, CanAct() && B.view.phase == TCG_TURN, "It isn't your turn.");
    B.menuX = 238;
    B.menuY = 158;
    B.mode = M_MENU;
    B.menuCursor = 0;
    PlaySE(SE_SELECT);
    printf("[tcg] turn menu: %s\n", B.menu[0].label);
}
/* A on your own deck: CONCEDE (as on the Yu-Gi-Oh! table), beside the deck */
static void OpenDeckMenu(void)
{
    struct Rect r = DeckRect(0);
    B.menuCount = 0;
    AddMenu("CONCEDE", MENU_CONCEDE, B.view.phase != TCG_FINISHED, "The duel is over.");
    Menu(r.x - 76, r.y);
    printf("[tcg] deck menu: CONCEDE\n");
}
static void Confirm(int kind, const char *text)
{
    B.confirmKind = kind;
    snprintf(B.confirmText, sizeof(B.confirmText), "%s", text);
    printf("[tcg] board asks: %s\n", text);
    B.menuCursor = 0;
    B.mode = M_CONFIRM;
    PlaySE(SE_SELECT);
}
/* END TURN (READY) from the turn menu: done at once, as on the Yu-Gi-Oh! table; only a turn ending without the
 * Energy attachment that's still possible asks first */
static void EndTurn(struct TcgAction *out, int *result)
{
    if (B.view.phase == TCG_SETUP)
    {
        if (!B.view.players[0].field[0].stackSize)
        {
            TcgBoard_Reject("Choose your Active Pokemon first.");
            return;
        }
        memset(&B.action, 0, sizeof(B.action));
        B.action.type = TCG_FINISH_SETUP;
        Submit(out, result);
        return;
    }
    if (!B.view.players[0].energyPlayed)
    {
        const struct TcgPlayer *p = &B.view.players[0];
        unsigned i;
        for (i = 0; i < p->handSize; i++)
            if (Tcg_Card(p->cards[p->hand[i]])->kind == TCG_ENERGY)
            {
                Confirm(2, "End your turn without attaching Energy?");
                return;
            }
    }
    memset(&B.action, 0, sizeof(B.action));
    B.action.type = TCG_PASS;
    Submit(out, result);
}
static void Choose(struct TcgAction *out, int *result, unsigned *card)
{
    int id = B.menu[B.menuCursor].id;
    const struct TcgPlayer *p = &B.view.players[0];
    if (!B.menu[B.menuCursor].enabled)
    {
        TcgBoard_Reject(B.menu[B.menuCursor].reason);
        return;
    }
    switch (id)
    {
    case MENU_PLAY:
    {
        const struct TcgCard *c = Tcg_Card(p->cards[p->hand[B.focusIndex]]);
        memset(&B.action, 0, sizeof(B.action));
        B.action.hand = (uint8_t)B.focusIndex;
        B.heldHand = B.focusIndex;
        B.heldPhysical = p->hand[B.focusIndex];
        B.flowCard = c->id;
        B.mode = M_FREE;
        if (c->kind == TCG_ENERGY)
        {
            B.action.type = TCG_ATTACH;
            B.flow = FLOW_ATTACH;
            B.focusKind = T_MINE;
            B.focusIndex = 0;
            StartTarget();
        }
        else if (c->kind == TCG_POKEMON)
        {
            B.action.type = c->stage ? TCG_EVOLVE : TCG_PLAY_BASIC;
            B.flow = c->stage ? FLOW_EVOLVE : FLOW_BASIC;
            B.focusKind = T_MINE;
            B.focusIndex = !c->stage && !p->field[0].stackSize ? 0 : 1;
            StartTarget();
        }
        else
        {
            B.action.type = TCG_PLAY_TRAINER;
            B.flow = FLOW_TRAINER;
            B.steps = TrainerSteps(c->id, &B.nsteps);
            B.step = 0;
            if (B.nsteps && B.steps[0].kind == ST_MY_SLOT)
                B.focusKind = T_MINE, B.focusIndex = 0;
            if (B.nsteps && B.steps[0].kind == ST_OPP_SLOT)
                B.focusKind = T_THEIRS, B.focusIndex = 1;
            if (!B.nsteps || B.steps[0].kind != ST_MY_SLOT)
                B.heldHand = -1;
            ContinueTrainer(out, result);
        }
        break;
    }
    case MENU_CHECK:
        B.mode = M_FREE;
        if (B.focusKind == T_HAND)
            *card = p->cards[p->hand[B.focusIndex]];
        else
        {
            const struct TcgCard *c = Field(B.focusKind == T_THEIRS, B.focusIndex);
            *card = c ? c->id : 0;
        }
        if (*card)
            *result = BOARD_INSPECT;
        break;
    case MENU_ATTACK:
        B.mode = M_ATTACK;
        B.attackCursor = 0;
        {
            char reason[80];
            if (!AttackReady(0, reason, sizeof(reason)) && AttackReady(1, reason, sizeof(reason)))
                B.attackCursor = 1;
        }
        B.focusKind = T_MINE;
        B.focusIndex = 0;
        PlaySE(SE_SELECT);
        break;
    case MENU_RETREAT:
        B.mode = M_FREE;
        B.focusKind = T_MINE;
        B.focusIndex = 1;
        BeginRetreat();
        break;
    case MENU_HAND:
        B.mode = M_FREE;
        B.focusKind = T_HAND;
        B.focusIndex = 0;
        B.trayWanted = 1;
        break;
    case MENU_DONE:
        B.mode = M_FREE;
        EndTurn(out, result);
        break;
    case MENU_CONCEDE:
        B.mode = M_FREE;
        Confirm(3, B.online ? "Concede this duel?" : "Concede this duel? You won't lose any cards.");
        break;
    case MENU_STAGE1:
    case MENU_STAGE2:
        B.action.attack = id == MENU_STAGE1 ? 1 : 2;
        B.step++;
        B.mode = M_FREE;
        ContinueTrainer(out, result);
        break;
    case MENU_POWER:
        BeginPower((unsigned)B.focusIndex, out, result);
        break;
    case MENU_DISCARD:
        memset(&B.action, 0, sizeof(B.action));
        B.action.type = TCG_DISCARD_DOLL;
        B.action.slot = (uint8_t)B.focusIndex;
        Submit(out, result);
        break;
    default:
        if (id >= MENU_CHOICE && B.flow == FLOW_POWER)
        {
            B.action.target = (uint8_t)(id - MENU_CHOICE);
            B.step++;
            B.mode = M_FREE;
            ContinueTrainer(out, result);
            break;
        }
        if (id >= MENU_CHOICE && B.flow == FLOW_ATTACK)
        {
            /* The pick decides what comes next (Metronome's copied attack, Prophecy's deck, the side to devolve) */
            const struct TcgAttack *a = &TcgDuel_Attacker(&B.view, 0)->attacks[B.action.attack];
            if (TcgDuel_IsMetronome(a) && !B.action.copy)
            {
                B.action.copy = (uint8_t)(id - MENU_CHOICE + 1);
                a = TcgDuel_MetronomeAttack(&B.view, 0, B.action.copy);
                printf("[tcg] metronome copies %s\n", a ? a->name : "?");
                AttackSteps(a);
            }
            else
            {
                B.action.option = (uint8_t)(id - MENU_CHOICE);
                printf("[tcg] attack option %u\n", B.action.option);
                AttackStepsAfterOption(TcgDuel_IsMetronome(a) ? TcgDuel_MetronomeAttack(&B.view, 0, B.action.copy) : a);
            }
            B.step = 0;
            B.mode = M_FREE;
            ContinueTrainer(out, result);
            break;
        }
        B.mode = M_FREE;
        break;
    }
}
static int PickValid(unsigned i)
{
    if (B.mode == M_VIEW)
        return 1;
    return CardMatches(B.pickOwner, B.pickCards[i], B.pickFilter);
}
static unsigned PickValue(void)
{
    unsigned i, v = 0;
    for (i = 0; i < B.pickChosen; i++)
        v += EnergyValue(CardOf(B.pickOwner, B.pickOrder[i]));
    return v;
}
static int PickDone(void)
{
    if (B.pickRetreat)
        return PickValue() >= (unsigned)B.pickRetreat;
    return (int)B.pickChosen >= B.pickMin && (int)B.pickChosen <= B.pickMax;
}
static void StepPick(unsigned pressed, struct TcgAction *out, int *result, unsigned *card)
{
    int n = B.pickCount;
    if (pressed & (KEY_LEFT | KEY_RIGHT | KEY_UP | KEY_DOWN) && n)
    {
        int d = pressed & KEY_LEFT ? -1 : pressed & KEY_RIGHT ? 1 : pressed & KEY_UP ? -7 : 7;
        B.pickCursor += d;
        if (B.pickCursor < 0)
            B.pickCursor = d == -1 ? n - 1 : B.pickCursor + 7 < n ? B.pickCursor + 7 : 0;
        if (B.pickCursor >= n)
            B.pickCursor = d == 1 ? 0 : n - 1;
        PlaySE(SE_RG_BAG_CURSOR);
    }
    if (pressed & KEY_INSPECT && n)
    {
        *card = CardOf(B.pickOwner, B.pickCards[B.pickCursor]);
        *result = BOARD_INSPECT;
        return;
    }
    if (pressed & KEY_B)
    {
        if (B.mode == M_VIEW)
            B.mode = M_FREE;
        else
        {
            CancelFlow();
            B.pickRetreat = 0;
        }
        PlaySE(SE_SELECT);
        return;
    }
    if (B.mode == M_VIEW)
    {
        if (pressed & KEY_A && n)
        {
            *card = CardOf(B.pickOwner, B.pickCards[B.pickCursor]);
            *result = BOARD_INSPECT;
        }
        return;
    }
    if (pressed & KEY_A)
    {
        unsigned i;
        int at = -1;
        if (!n)
        {
            if (B.pickMin == 0)
                FinishPick(out, result);
            return;
        }
        for (i = 0; i < B.pickChosen; i++)
            if (B.pickOrder[i] == B.pickCards[B.pickCursor])
                at = (int)i;
        if (at >= 0)
        {
            memmove(B.pickOrder + at, B.pickOrder + at + 1, B.pickChosen - at - 1);
            B.pickChosen--;
            PlaySE(SE_SELECT);
        }
        else if (!PickValid(B.pickCursor))
            TcgBoard_Reject("That card can't be chosen here.");
        else if ((int)B.pickChosen >= B.pickMax && !B.pickRetreat)
            TcgBoard_Reject("You've already chosen enough cards.");
        else
        {
            B.pickOrder[B.pickChosen++] = B.pickCards[B.pickCursor];
            PlaySE(SE_RG_CARD_FLIP);
            if (B.pickChosen == B.pickMax && B.pickMin == B.pickMax && !B.pickRetreat)
                FinishPick(out, result);
            else if (B.pickRetreat && PickDone())
                FinishPick(out, result);
        }
        return;
    }
    if (pressed & (KEY_START | KEY_SELECT))
    {
        if (PickDone())
            FinishPick(out, result);
        else
            TcgBoard_Reject(B.pickSource == SRC_DECK && !B.pickChosen && B.pickMin
                                ? "Choose a card (B cancels)."
                                : "Choose the required number of cards.");
    }
}

/* The lesson's tips, each shown once when its moment first comes. */
static void Tip(void)
{
    static const char *const tips[] = {
        "Pick a Basic Pokemon from your hand for your Active space. You can add up to 5 more to your Bench.",
        "Your turn! Attach one Energy card from your hand to a Pokemon each turn.",
        "Your Active Pokemon has the Energy it needs: choose it and ATTACK!",
        "Knock out a Pokemon and you take one of your 6 Prize cards. Take them all to win!",
        "Your Pokemon was Knocked Out. Choose a Benched Pokemon to go in.",
        "Done for this turn? Press B, then END TURN.",
    };
    const struct TcgPlayer *me = &B.view.players[0];
    int tip = -1;
    if (!MyTurn())
        return;
    if (B.view.phase == TCG_SETUP)
        tip = 0;
    else if (B.view.phase == TCG_PROMOTION)
        tip = 4;
    else if (B.view.phase == TCG_TURN && !me->energyPlayed)
        tip = 1;
    else if (B.view.phase == TCG_TURN && Field(0, 0) && (TcgDuel_CanAttack(&B.view, 0, 0) || TcgDuel_CanAttack(&B.view, 0, 1)))
        tip = 2;
    else if (B.view.phase == TCG_TURN && me->prizeSize < B.view.prizeCount)
        tip = 3;
    else if (B.view.phase == TCG_TURN && me->energyPlayed)
        tip = 5;
    if (tip < 0 || (B.tipsShown & (1 << tip)))
        return;
    B.tipsShown |= 1 << tip;
    Toast("%s", tips[tip]);
    B.toastTime = 400;
}

/* ------------------------------------------------------------------ */
/* Input                                                                */
/* ------------------------------------------------------------------ */
int TcgBoard_Step(unsigned pressed, unsigned held, struct TcgAction *out, unsigned *card)
{
    int result = BOARD_NONE, i, steps;
    B.frame++;
    (void)held;
    B.speed = 1;   /* (no hurrying by holding a button: both games' tables play every animation the same way) */
    steps = B.speed;
    for (i = 0; i < steps; i++)
    {
        unsigned k;
        if (B.intro > 0)
        {
            B.intro--;
            if (B.intro == 0)
            {
                /* The deal: hands, then Prizes, one card at a time. */
                const struct TcgPlayer *p = &B.view.players[0];
                unsigned h;
                Layout();
                PlaySE(SE_RG_CARD_FLIPPING);
                /* Setup starts on the first Basic Pokemon in hand. */
                B.focusKind = T_HAND;
                B.focusIndex = 0;
                for (h = 0; h < p->handSize; h++)
                {
                    const struct TcgCard *c = Tcg_Card(p->cards[p->hand[h]]);
                    if (c->kind == TCG_POKEMON && !c->stage)
                    {
                        B.focusIndex = (int)h;
                        break;
                    }
                }
                B.trayWanted = 1;
            }
        }
        else
            Present();
        if (B.spotTime > 0 && --B.spotTime == 0)
            B.spotlight = -1;   /* into the hand it goes */
        if (B.showHand > 0 && B.phase == P_IDLE && !B.qCount)
            B.showHand--;
        if (!B.intro)
            Layout();
        if (B.checkDone && TcgBoard_Idle() && !B.showHand && B.spotlight < 0)
        {
            B.checkDone = 0;
            FocusActiveIfDone();
        }
        MoveSprites();
        for (k = 0; k < FX_MAX; k++)
            if (B.fx[k].duration && ++B.fx[k].t >= B.fx[k].duration)
                B.fx[k].duration = 0;
        {
            unsigned who, s;
            for (who = 0; who < 2; who++)
            {
                if (B.lunge[who])
                    B.lunge[who]--;
                for (s = 0; s < TCG_FIELD_SIZE; s++)
                {
                    if (B.shake[who][s])
                        B.shake[who][s]--;
                    if (B.flash[who][s])
                        B.flash[who][s]--;
                    if (B.ko[who][s] && B.ko[who][s] < 40)
                        B.ko[who][s]++;
                    B.shownDamage[who][s] += (B.damage[who][s] - B.shownDamage[who][s]) * 0.18f;
                    if (fabsf(B.damage[who][s] - B.shownDamage[who][s]) < 0.5f)
                        B.shownDamage[who][s] = B.damage[who][s];
                }
            }
        }
    }
    if (B.toastTime)
        B.toastTime--;
    {
        /* The hand opens for browsing on your own turn, once the mat is still. */
        float want = (B.trayWanted && B.focusKind == T_HAND && (B.mode == M_FREE || B.mode == M_MENU) && MyTurn() &&
                      B.phase == P_IDLE && !B.qCount && B.intro <= 0) ||
                             (B.showHand > 0 && B.intro <= 0 && B.mode != M_PICK && B.mode != M_RESULT)
                         ? 1.0f
                         : 0.0f;
        B.tray += (want - B.tray) * 0.3f;
        if (fabsf(want - B.tray) < 0.01f)
            B.tray = want;
    }
    /* Hand positions follow the tray continuously. */
    if (!B.intro)
        Layout();
    if (B.view.phase == TCG_PROMOTION && MyTurn() && TcgBoard_Idle() && B.mode == M_FREE && !B.promoFocused)
    {
        /* After a Knock Out: straight to the Bench, on the first Pokemon there. */
        unsigned slot;
        B.promoFocused = 1;
        B.trayWanted = 0;
        for (slot = 1; slot < TCG_FIELD_SIZE; slot++)
            if (Field(0, slot))
            {
                B.focusKind = T_MINE;
                B.focusIndex = (int)slot;
                break;
            }
    }
    else if (B.view.phase != TCG_PROMOTION)
        B.promoFocused = 0;
    if (B.tutorial && TcgBoard_Idle() && B.mode == M_FREE && !B.toastTime)
        Tip();
    /* While the duel shows what happened (the deal, an attack, a draw, a coin, a Knock Out, a Prize card...), the
       buttons do nothing: nothing can be chosen until it has played out, and no press waits to land after it.
       (The sprites still gliding after a cursor move don't count: that would make the cursor feel slow.) */
    if (TcgBoard_Busy())
        pressed = 0;
    if (B.toast[0] && B.toastTime && pressed & (KEY_A | KEY_B))
    {
        B.toastTime = 0;
        return BOARD_NONE;
    }
    if (B.mode == M_RESULT)
    {
        B.resultTime++;
        if (B.resultTime > 40 && pressed & KEY_A)
        {
            PlaySE(SE_SELECT);
            return BOARD_LEAVE;
        }
        return BOARD_NONE;
    }
    if (B.intro > 0)
        return BOARD_NONE;
    switch (B.mode)
    {
    case M_MENU:
    {
        int n = B.menuCount;
        if (pressed & KEY_UP)
            B.menuCursor = (B.menuCursor + n - 1) % n;
        if (pressed & KEY_DOWN)
            B.menuCursor = (B.menuCursor + 1) % n;
        if (pressed & (KEY_UP | KEY_DOWN))
            PlaySE(SE_RG_BAG_CURSOR);
        if (pressed & KEY_B)
        {
            if (B.flow == FLOW_TRAINER || B.flow == FLOW_POWER)
                CancelFlow();
            B.mode = M_FREE;
            PlaySE(SE_SELECT);
        }
        else if (pressed & KEY_A)
            Choose(out, &result, card);
        return result;
    }
    case M_CONFIRM:
        if (pressed & (KEY_UP | KEY_DOWN))
        {
            B.menuCursor ^= 1;
            PlaySE(SE_RG_BAG_CURSOR);
        }
        if (pressed & KEY_B)
        {
            B.mode = M_FREE;
            PlaySE(SE_SELECT);
        }
        else if (pressed & KEY_A)
        {
            B.mode = M_FREE;
            if (B.menuCursor == 0)
            {
                memset(out, 0, sizeof(*out));
                out->type = B.confirmKind == 1 ? TCG_FINISH_SETUP : B.confirmKind == 2 ? TCG_PASS : TCG_CONCEDE;
                PlaySE(SE_SELECT);
                return B.confirmKind == 3 ? BOARD_CONCEDE : BOARD_ACTION;
            }
            PlaySE(SE_SELECT);
        }
        return BOARD_NONE;
    case M_ATTACK:
        if (pressed & (KEY_UP | KEY_DOWN))
        {
            const struct TcgCard *c = TcgDuel_Attacker(&B.view, 0);
            if (c && c->attacks[B.attackCursor ^ 1].name[0])
            {
                B.attackCursor ^= 1;
                PlaySE(SE_RG_BAG_CURSOR);
            }
        }
        if (pressed & KEY_B)
        {
            B.mode = M_FREE;
            PlaySE(SE_SELECT);
        }
        else if (pressed & KEY_INSPECT)
        {
            const struct TcgCard *c = Field(0, 0);
            if (c)
            {
                *card = c->id;
                return BOARD_INSPECT;
            }
        }
        else if (pressed & KEY_A)
        {
            char reason[80];
            if (!AttackReady(B.attackCursor, reason, sizeof(reason)))
                TcgBoard_Reject(reason);
            else
            {
                const struct TcgCard *c = TcgDuel_Attacker(&B.view, 0);
                memset(&B.action, 0, sizeof(B.action));
                B.action.type = TCG_ATTACK;
                B.action.attack = (uint8_t)B.attackCursor;
                AttackSteps(&c->attacks[B.attackCursor]);
                if (B.nsteps)
                {
                    /* Spark, Lure, Teleport, Amnesia, Metronome...: the attack's choices first */
                    B.flow = FLOW_ATTACK;
                    B.heldHand = -1;
                    B.mode = M_FREE;
                    B.step = 0;
                    if (B.steps[0].kind == ST_MY_SLOT || B.steps[0].kind == ST_OPP_SLOT)
                    {
                        B.focusKind = B.steps[0].kind == ST_OPP_SLOT ? T_THEIRS : T_MINE;
                        B.focusIndex = 1;
                    }
                    ContinueTrainer(out, &result);
                }
                else
                    Submit(out, &result);
            }
        }
        return result;
    case M_PICK:
    case M_VIEW:
        StepPick(pressed, out, &result, card);
        return result;
    default:
        break;
    }
    /* Free cursor or choosing a target on the mat. */
    if (pressed & KEY_UP)
        Navigate(0, -1);
    if (pressed & KEY_DOWN)
        Navigate(0, 1);
    if (pressed & KEY_LEFT)
        Navigate(-1, 0);
    if (pressed & KEY_RIGHT)
        Navigate(1, 0);
    if (B.focusKind == T_HAND)
    {
        unsigned n = B.view.players[0].handSize;
        if (!n && B.mode == M_FREE)
            B.focusKind = T_MINE, B.focusIndex = 0;
        else if ((unsigned)B.focusIndex >= n)
            B.focusIndex = n ? (int)n - 1 : 0;
    }
    if (B.mode == M_TARGET)
    {
        if (pressed & KEY_B)
        {
            int back = B.heldHand;
            CancelFlow();
            if (back >= 0)
                B.focusKind = T_HAND, B.focusIndex = back, B.trayWanted = 1;
            PlaySE(SE_SELECT);
            return BOARD_NONE;
        }
        if (pressed & KEY_A)
        {
            if (!Valid(B.focusKind, B.focusIndex))
            {
                TcgBoard_Reject("Choose one of the highlighted spaces.");
                return BOARD_NONE;
            }
            if (B.flow == FLOW_POWER)
            {
                if (B.step == 0)
                    B.action.target = (uint8_t)B.focusIndex;
                else
                    B.action.attack = (uint8_t)B.focusIndex;
                B.step++;
                B.mode = M_FREE;
                ContinueTrainer(out, &result);
                return result;
            }
            if (B.flow == FLOW_TRAINER)
            {
                if (B.steps[B.step].kind == ST_OPP_SLOT)
                    B.action.target = (uint8_t)B.focusIndex;
                else
                    B.action.slot = (uint8_t)B.focusIndex;
                B.step++;
                B.mode = M_FREE;
                ContinueTrainer(out, &result);
                return result;
            }
            if (B.flow == FLOW_ATTACK)
            {
                B.action.target = (uint8_t)B.focusIndex;
                B.step++;
                B.mode = M_FREE;
                ContinueTrainer(out, &result);
                return result;
            }
            B.action.slot = (uint8_t)B.focusIndex;
            if (B.flow == FLOW_RETREAT)
            {
                B.mode = M_FREE;
                RetreatCost(out, &result);
                return result;
            }
            Submit(out, &result);
            return result;
        }
        return BOARD_NONE;
    }
    /* R: the card under the cursor up close (as on the Yu-Gi-Oh! table) */
    if (pressed & KEY_INSPECT)
    {
        unsigned id = 0;
        if (B.focusKind == T_HAND && (unsigned)B.focusIndex < B.view.players[0].handSize)
            id = CardOf(0, B.view.players[0].hand[B.focusIndex]);
        else if ((B.focusKind == T_MINE || B.focusKind == T_THEIRS) &&
                 !(B.focusKind == T_THEIRS && B.view.phase == TCG_SETUP))
        {
            const struct TcgCard *c = Field(B.focusKind == T_THEIRS, B.focusIndex);
            id = c ? c->id : 0;
        }
        else if (B.focusKind == T_MY_DISCARD || B.focusKind == T_OPP_DISCARD)
        {
            const struct TcgPlayer *p = &B.view.players[B.focusKind == T_OPP_DISCARD];
            id = p->discardSize ? p->cards[p->discard[p->discardSize - 1]] : 0;
        }
        if (id)
        {
            *card = id;
            return BOARD_INSPECT;
        }
    }
    /* START / SELECT: the turn menu; B leaves the hand, then opens it too (the cursor on END TURN) */
    if (pressed & (KEY_START | KEY_SELECT))
    {
        OpenTurnMenu();
        return BOARD_NONE;
    }
    if (pressed & KEY_B)
    {
        if (B.focusKind == T_HAND)
        {
            B.focusKind = T_MINE;
            B.focusIndex = 0;
            B.trayWanted = 0;
            PlaySE(SE_SELECT);
        }
        else if (B.view.phase != TCG_PROMOTION)
            OpenTurnMenu();
        return BOARD_NONE;
    }
    if (pressed & KEY_A)
    {
        /* Promotion: any Benched Pokemon becomes Active. */
        if (B.view.phase == TCG_PROMOTION && MyTurn())
        {
            if (B.focusKind == T_MINE && B.focusIndex > 0 && Field(0, B.focusIndex))
            {
                if (!CanAct())
                    return BOARD_NONE;
                memset(&B.action, 0, sizeof(B.action));
                B.action.type = TCG_PROMOTE;
                B.action.slot = (uint8_t)B.focusIndex;
                Submit(out, &result);
                return result;
            }
            TcgBoard_Reject("Choose a Benched Pokemon to become your Active Pokemon.");
            return BOARD_NONE;
        }
        switch (B.focusKind)
        {
        case T_HAND:
            OpenHandMenu();
            break;
        case T_MINE:
            if (Field(0, B.focusIndex))
                OpenFieldMenu();
            else
                PlaySE(SE_FAILURE);
            break;
        case T_THEIRS:
        {
            const struct TcgCard *c = Field(1, B.focusIndex);
            if (c && B.view.phase != TCG_SETUP)
            {
                *card = c->id;
                return BOARD_INSPECT;
            }
            PlaySE(SE_FAILURE);
            break;
        }
        case T_MY_DISCARD:
        case T_OPP_DISCARD:
            OpenViewer(B.focusKind == T_OPP_DISCARD);
            PlaySE(SE_SELECT);
            break;
        case T_MY_DECK:
            OpenDeckMenu();
            break;
        case T_OPP_HAND:
            if (Clairvoyant() && B.view.players[1].handSize)
            {
                OpenHandViewer();
                PlaySE(SE_SELECT);
            }
            else
                PlaySE(SE_FAILURE);
            break;
        default:
            PlaySE(SE_FAILURE);
            break;
        }
    }
    return result;
}

/* ------------------------------------------------------------------ */
/* Drawing                                                              */
/* ------------------------------------------------------------------ */
static SDL_Texture *sMat;
static unsigned sMatTheme, sMatType;
/* A cloth play mat per kind of opponent: base, shadow, zone lines, a motif. */
static const struct
{
    unsigned base, dark, line, label;
} sMats[TCG_MAT_COUNT] = {
    [TCG_MAT_CLASSIC] = {0x1c7a44, 0x125a32, 0x3cc070, 0x2fa05c},
    [TCG_MAT_BEACH] = {0xc8a868, 0xa88848, 0xf4e8c0, 0xe8d4a0},
    [TCG_MAT_FOREST] = {0x2c5c2c, 0x1c401c, 0x78b858, 0x5c9c44},
    [TCG_MAT_CAVE] = {0x6c5c4c, 0x4c4034, 0xb4a080, 0x988468},
    [TCG_MAT_CITY] = {0x3c5070, 0x283850, 0x88a8d0, 0x6c8cb8},
    [TCG_MAT_SKY] = {0x5c98d0, 0x4078b0, 0xe0f0ff, 0xc0dcf8},
    [TCG_MAT_NIGHT] = {0x2c2c58, 0x1c1c3c, 0x8c84d8, 0x6c64b8},
    [TCG_MAT_GYM] = {0, 0, 0, 0},
    [TCG_MAT_LEAGUE] = {0x40285c, 0x2c1840, 0xe0c060, 0xc0a048},
    [TCG_MAT_AQUA] = {0x18386c, 0x0c2448, 0x58b0e8, 0x4890c8},
    [TCG_MAT_MAGMA] = {0x6c2418, 0x48160c, 0xf08848, 0xc86c38},
    [TCG_MAT_FRONTIER] = {0x2c2c30, 0x1c1c20, 0xd8b058, 0xa88c48},
    [TCG_MAT_ONLINE] = {0x186c6c, 0x0c4c4c, 0x58d8c8, 0x40b0a4},
};
static void MatColors(unsigned *base, unsigned *dark, unsigned *line, unsigned *label)
{
    if (sMatTheme == TCG_MAT_GYM)
    {
        /* The Gym's type, dimmed so the cards stand out. */
        unsigned t = G_TypeColor(sMatType, 2);
        *base = G_Mix(t, 0x202028, 0.35f);
        *dark = G_Mix(t, 0x101018, 0.6f);
        *line = G_Mix(G_TypeColor(sMatType, 1), 0xffffff, 0.2f);
        *label = G_Mix(*line, *base, 0.3f);
        return;
    }
    *base = sMats[sMatTheme].base;
    *dark = sMats[sMatTheme].dark;
    *line = sMats[sMatTheme].line;
    *label = sMats[sMatTheme].label;
}
static uint32_t MatRandom(uint32_t *state)
{
    *state = *state * 1103515245u + 12345u;
    return *state >> 16;
}
/* The theme's pattern, faint under the zones. */
static void MatMotif(struct GSurface *s, unsigned base, unsigned line)
{
    uint32_t seed = 0x5eed0000u + sMatTheme * 977u + sMatType;
    int x, y, i;
    unsigned soft = G_Mix(base, line, 0.16f), softer = G_Mix(base, line, 0.09f);
    switch (sMatTheme)
    {
    case TCG_MAT_BEACH:
    case TCG_MAT_AQUA:
        /* Rolling waves. */
        for (y = 6; y < LAYER_H; y += 14)
            for (x = 0; x < 240; x++)
            {
                int wy = y + (int)(sinf((x + y * 3) * 0.13f) * 2.5f);
                S_Blend(s, x, wy, soft, 255);
                if (sMatTheme == TCG_MAT_BEACH && (x + y) % 23 == 0)
                    S_Fill(s, x, wy + 6, 2, 2, ARGB(G_Mix(base, 0xffffff, 0.25f), 255));
            }
        break;
    case TCG_MAT_FOREST:
        /* Scattered leaves. */
        for (i = 0; i < 70; i++)
        {
            int lx = (int)(MatRandom(&seed) % 240), ly = (int)(MatRandom(&seed) % LAYER_H), k;
            for (k = 0; k < 4; k++)
            {
                S_Blend(s, lx + k, ly - k / 2, soft, 255);
                S_Blend(s, lx + k, ly + 1 - k / 2, softer, 255);
            }
            S_Blend(s, lx - 1, ly + 1, soft, 255);
        }
        break;
    case TCG_MAT_CAVE:
    case TCG_MAT_MAGMA:
        /* Rock speckles and cracks (glowing for Team Magma). */
        for (i = 0; i < 260; i++)
            S_Blend(s, (int)(MatRandom(&seed) % 240), (int)(MatRandom(&seed) % LAYER_H), i & 1 ? soft : softer, 255);
        for (i = 0; i < 9; i++)
        {
            int cx = (int)(MatRandom(&seed) % 240), cy = (int)(MatRandom(&seed) % LAYER_H), k;
            for (k = 0; k < 18; k++)
            {
                cx += (int)(MatRandom(&seed) % 3) - 1;
                cy += 1;
                S_Blend(s, cx, cy, sMatTheme == TCG_MAT_MAGMA ? G_Mix(base, 0xf87830, 0.45f) : soft, 255);
            }
        }
        break;
    case TCG_MAT_CITY:
    case TCG_MAT_ONLINE:
        /* A city grid, or the link's hexagons. */
        for (y = 0; y < LAYER_H; y++)
            for (x = 0; x < 240; x++)
            {
                int on = sMatTheme == TCG_MAT_CITY ? (x % 12 == 0 || y % 12 == 0)
                                                   : ((x + (y / 10 % 2) * 6) % 12 == 0 && y % 10 < 6) || (y % 10 == 0 && (x + (y / 10 % 2) * 6) % 12 > 6);
                if (on)
                    S_Blend(s, x, y, softer, 255);
            }
        break;
    case TCG_MAT_SKY:
        /* Soft clouds. */
        for (i = 0; i < 14; i++)
        {
            float cx = (float)(MatRandom(&seed) % 240), cy = (float)(MatRandom(&seed) % LAYER_H);
            S_Disc(s, cx, cy, 6, ARGB(G_Mix(base, 0xffffff, 0.12f), 255));
            S_Disc(s, cx + 7, cy + 1, 5, ARGB(G_Mix(base, 0xffffff, 0.12f), 255));
            S_Disc(s, cx - 6, cy + 2, 4, ARGB(G_Mix(base, 0xffffff, 0.12f), 255));
        }
        break;
    case TCG_MAT_NIGHT:
    case TCG_MAT_LEAGUE:
    case TCG_MAT_FRONTIER:
        /* Stars, and a gold sheen for the League and the Frontier. */
        for (i = 0; i < 90; i++)
        {
            int sx = (int)(MatRandom(&seed) % 240), sy = (int)(MatRandom(&seed) % LAYER_H);
            S_Blend(s, sx, sy, G_Mix(base, line, i % 7 ? 0.25f : 0.55f), 255);
            if (i % 9 == 0)
            {
                S_Blend(s, sx - 1, sy, soft, 255);
                S_Blend(s, sx + 1, sy, soft, 255);
                S_Blend(s, sx, sy - 1, soft, 255);
                S_Blend(s, sx, sy + 1, soft, 255);
            }
        }
        if (sMatTheme != TCG_MAT_NIGHT)
            for (y = 0; y < LAYER_H; y++)
                for (x = 0; x < 240; x++)
                    if ((x + y) % 46 < 2)
                        S_Blend(s, x, y, softer, 255);
        break;
    case TCG_MAT_GYM:
        /* Diagonal stripes in the Gym's colours. */
        for (y = 0; y < LAYER_H; y++)
            for (x = 0; x < 240; x++)
                if ((x - y + 400) % 20 < 6)
                    S_Blend(s, x, y, softer, 255);
        break;
    default:
        /* Felt: a fine weave. */
        for (y = 0; y < LAYER_H; y++)
            for (x = (y & 1); x < 240; x += 2)
                if ((x * 7 + y * 13) % 5 == 0)
                    S_Blend(s, x, y, softer, 255);
        break;
    }
}
/* A place for a card: a soft hollow in the cloth and a clean line round it */
static void CardSpot(struct GSurface *s, struct Rect r, unsigned line, unsigned alpha)
{
    S_RoundFill(s, r.x, r.y, r.w, r.h, 1, ARGB(0x000000, alpha / 4));
    S_RoundFrame(s, r.x - 1, r.y - 1, r.w + 2, r.h + 2, 2, ARGB(line, alpha));
}
static void BuildMat(void)
{
    struct GSurface s = G_SurfaceNew(240, LAYER_H);
    unsigned base, dark, line, label, who, i;
    int x, y;
    if (!s.px)
        return;
    MatColors(&base, &dark, &line, &label);
    /* Cloth: the colour darkens towards the edges. */
    for (y = 0; y < LAYER_H; y++)
        for (x = 0; x < 240; x++)
        {
            float dx = (x - 120) / 120.0f, dy = (y - CENTER_Y) / 90.0f, d = dx * dx + dy * dy;
            S_Px(&s, x, y, ARGB(G_Mix(base, dark, d > 1 ? 0.6f : d * 0.6f), 255));
        }
    MatMotif(&s, base, line);
    /* The POKe BALL between the Active Pokemon: a round ring (the layer is
     * drawn 1:1, never stretched), its band and the button. */
    S_Ring(&s, CENTER_X, CENTER_Y, 44, 2.4f, ARGB(line, 255));
    for (x = CENTER_X - 43; x <= CENTER_X + 43; x++)
        if (abs(x - CENTER_X) > 15)
        {
            S_Px(&s, x, CENTER_Y - 1, ARGB(line, 255));
            S_Px(&s, x, CENTER_Y, ARGB(line, 255));
        }
    S_Ring(&s, CENTER_X, CENTER_Y, 15, 2.2f, ARGB(line, 255));
    S_Ring(&s, CENTER_X, CENTER_Y, 8, 1.6f, ARGB(line, 200));
    /* Benches: a frame around the row, a space per card. */
    for (who = 0; who < 2; who++)
    {
        struct Rect first = BenchCell(who, 0);
        S_RoundFrame(&s, first.x, first.y, BENCH_CELL * 5 + 1, first.h, 3, ARGB(line, 255));
        for (i = 1; i < 6; i++)
            CardSpot(&s, SlotRect(who, i), line, 110);
        CardSpot(&s, SlotRect(who, 0), line, 150);
        /* Side columns: a spot for each pile and each Prize. */
        {
            struct Rect d = DeckRect(who), t = DiscardRect(who), p = PrizeArea(who);
            S_RoundFrame(&s, d.x - 4, d.y - 2, d.w + 8, d.h + 4, 3, ARGB(line, 255));
            S_RoundFrame(&s, t.x - 4, t.y - 2, t.w + 8, t.h + 4, 3, ARGB(line, 255));
            CardSpot(&s, d, line, 110);
            CardSpot(&s, t, line, 110);
            S_RoundFrame(&s, p.x, p.y, p.w, p.h, 3, ARGB(line, 255));
            for (i = 0; i < 6; i++)
                CardSpot(&s, PrizeRect(who, i), line, 70);
        }
    }
    S_RoundFrame(&s, sOppHand.x, sOppHand.y, sOppHand.w, sOppHand.h, 3, ARGB(line, 150));
    (void)label;
    sMat = G_SurfaceTexture(&s);
    G_SurfaceFree(&s);
}
void TcgBoard_SetMat(unsigned theme, unsigned type)
{
    if (theme >= TCG_MAT_COUNT)
        theme = TCG_MAT_CLASSIC;
    if (theme != sMatTheme || type != sMatType)
    {
        if (sMat)
            SDL_DestroyTexture(sMat);
        sMat = NULL;
    }
    sMatTheme = theme;
    sMatType = type;
}
static void Labels(void)
{
    unsigned base, dark, line, label;
    MatColors(&base, &dark, &line, &label);
    (void)base, (void)dark, (void)line;
    G_TextAlign(204, 110, TA_RIGHT, label, G_NO_SHADOW, "BENCH", TF_NARROW);
}
static void DrawPile(unsigned who, struct Rect r, unsigned count, int deck)
{
    char n[8];
    int i, layers = count > 40 ? 3 : count > 15 ? 2 : count > 1 ? 1 : 0;
    if (!count)
        return;
    if (deck)
    {
        for (i = layers; i > 0; i--)
            G_Card(0, (float)r.x + i, (float)r.y + i, (float)r.w, (float)r.h, 0, 255, 0);
        G_Card(0, (float)r.x, (float)r.y, (float)r.w, (float)r.h, 0, 255, 0);
    }
    snprintf(n, sizeof(n), "%u", count);
    {
        int w = G_DigitsWidth(n, 0, 1) + 4, x = r.x + r.w - w + 3, y = r.y + r.h - 5;
        G_RoundFill(x, y, w, 9, 0x1c2c48, 235);
        G_RoundFrame(x, y, w, 9, 0x8ab0e0, 255);
        G_Digits(x + 2, y + 2, n, 0, 1, 0xe0f0ff, 255);
    }
    (void)who;
}
static void Counters(struct Rect r, unsigned damage, int turned)
{
    unsigned n10 = damage / 10, n50 = 0, i, k = 0;
    if (n10 > 12)
    {
        n50 = n10 / 5;
        n10 %= 5;
    }
    for (i = 0; i < n50 + n10 && k < 15; i++, k++)
    {
        int x = r.x + 3 + (int)(k % 4) * 6, y = r.y + 7 + (int)(k / 4) * 6;
        unsigned c = i < n50 ? 0xe87818 : 0xd82838;
        if (turned)
            x = r.x + r.w - 8 - (int)(k % 4) * 6, y = r.y + r.h - 12 - (int)(k / 4) * 6;
        G_Fill(x + 1, y, 3, 1, 0x501018);
        G_Fill(x, y + 1, 5, 3, 0x501018);
        G_Fill(x + 1, y + 4, 3, 1, 0x501018);
        G_Fill(x + 1, y + 1, 3, 3, c);
        G_Fill(x + 1, y + 1, 1, 1, G_Mix(c, 0xffffff, 0.6f));
    }
}
/* The original's substatus as small tags beside the conditions. Returns the
 * width; right-aligned to x when right is set. */
static int Substatus(int x, int y, unsigned who, unsigned slot, int right, int draw)
{
    const struct TcgFieldCard *f = &B.view.players[who].field[slot];
    /* (Growl, Leer and the like stop counting once their Pokemon has gone or evolved) */
    int bound = TcgDuel_BoundEffect(&B.view, who, slot);
    static const char *const guards[] = {"", "-10", "-20", "<40", "HALF"};
    static const char *const weakens[] = {"", "ATK-10", "ATK-20", "BLIND"};
    const char *labels[6];
    unsigned colors[6], n = 0, i;
    int w = 0;
    if (f->barrier)
        labels[n] = "SAFE", colors[n++] = 0x3070c8;
    else if (f->guard && f->guard < 5)
        labels[n] = guards[f->guard], colors[n++] = 0x3070c8;
    if (f->weaken && f->weaken < 4 && (f->weaken == TCG_WEAKEN_SMOKESCREEN || bound))
        labels[n] = weakens[f->weaken], colors[n++] = 0xc06020;
    if (f->noAttack == 1 || (f->noAttack == 2 && bound))
        labels[n] = "NO ATK", colors[n++] = 0xc06020;
    if (f->noRetreat)
        labels[n] = "STUCK", colors[n++] = 0xc06020;
    if (f->charged)
        labels[n] = "x2", colors[n++] = 0xc89010;
    for (i = 0; i < n; i++)
        w += G_Measure(labels[i], TF_NARROW) + 5;
    if (!draw)
        return w;
    if (right)
        x -= w;
    for (i = 0; i < n; i++)
    {
        int lw = G_Measure(labels[i], TF_NARROW) + 4;
        G_RoundFill(x, y, lw, 9, colors[i], 255);
        G_Text(x + 2, y - 1, G_WHITE, G_Mix(colors[i], 0, 0.5f), labels[i], 0, TF_NARROW);
        x += lw + 1;
    }
    return w;
}
static void FieldInfo(unsigned who, unsigned slot)
{
    const struct TcgCard *c = Field(who, slot);
    const struct TcgFieldCard *f = &B.view.players[who].field[slot];
    struct Rect r = SlotRect(who, slot);
    int hp, i, n = 0, x;
    unsigned types[24];
    if (!c || (who && B.view.phase == TCG_SETUP))
        return;
    hp = (int)MaxHp(c) - (int)(B.shownDamage[who][slot] + 0.5f);
    for (i = 0; i < f->attachedSize && n < 24; i++)
    {
        const struct TcgCard *e = Tcg_Card(CardOf(who, f->attached[i]));
        if (e->kind == TCG_ENERGY)
        {
            types[n++] = e->type;
            if (e->id == TCG_CARD_DOUBLE_COLORLESS_ENERGY && n < 24)
                types[n++] = e->type;
        }
    }
    if (slot)
    {
        /* Bench: a small plate over the card's text box, Energy beside it. */
        G_HpPlate(r.x - 2, r.y + r.h - 9, hp, MaxHp(c), 1, 255);
        for (i = 0; i < n && i < 3; i++)
            G_EnergyOrb(r.x + r.w + 3, r.y + 1 + i * 6, types[i], 0);
        if (n > 3)
        {
            char more[8];
            snprintf(more, sizeof(more), "+%d", n - 3);
            G_Digits(r.x + r.w + 2, r.y + 19, more, 0, 1, 0xf8f8f8, 255);
        }
        if (f->poison || f->condition)
            G_FillA(r.x + r.w - 5, r.y + 1, 4, 4, f->poison ? 0xa040e0 : 0xf0d040, 255);
        return;
    }
    {
        char name[40];
        int nameX, nameY, plateX, plateY, orbY, statusY;
        snprintf(name, sizeof(name), "%s", c->name);
        while (G_Measure(name, TF_NARROW) > 62 && strlen(name) > 3)
            name[strlen(name) - 1] = 0;
        if (who)
        {
            nameX = 144, nameY = 33, statusY = 46, orbY = 56, plateX = 144, plateY = 65;
            G_Text(nameX, nameY, G_WHITE, 0x104020, name, 0, TF_NARROW);
            x = nameX + G_StatusBadges(nameX, statusY, B.cond[who][slot], B.poison[who][slot]);
            Substatus(x, statusY, who, slot, 0, 1);
            for (i = 0; i < n && i < 8; i++)
                G_EnergyOrb(plateX + i * 8, orbY, types[i], 1);
        }
        else
        {
            int right = 98, w;
            plateX = right - G_HpPlateWidth(0), plateY = 79, orbY = 92, statusY = 101, nameY = 108;
            nameX = right - G_Measure(name, TF_NARROW);
            G_Text(nameX, nameY, G_WHITE, 0x104020, name, 0, TF_NARROW);
            w = (B.poison[who][slot] ? G_Measure("PSN", TF_NARROW) + 5 : 0) +
                (B.cond[who][slot] ? G_Measure("SLP", TF_NARROW) + 5 : 0);
            G_StatusBadges(right - w, statusY, B.cond[who][slot], B.poison[who][slot]);
            Substatus(right - w, statusY, who, slot, 1, 1);
            x = right - (n < 8 ? n : 8) * 8 + 1;
            for (i = 0; i < n && i < 8; i++)
                G_EnergyOrb(x + i * 8, orbY, types[i], 1);
        }
        G_HpPlate(plateX, plateY, hp, MaxHp(c), 0, 255);
    }
}
static void DrawSprite(unsigned who, unsigned i)
{
    struct CardSprite *s = &B.sprite[who][i];
    float x = s->x, y = s->y, w = s->w, h = s->h, scale;
    unsigned id = s->shownFace ? CardOf(who, i) : 0, alpha = 255, flash = 0;
    if (s->zone == Z_FIELD)
    {
        int slot = s->slot;
        if (B.shake[who][slot])
            x += (float)((B.shake[who][slot] & 2) ? 2 : -2) * B.shake[who][slot] / 14.0f;
        if (!slot && B.lunge[who])
        {
            float t = (float)B.lunge[who] / (Fast() ? 12 : 18), push = sinf(t * 3.14159f) * 8;
            y += who ? push : -push;
        }
        flash = B.flash[who][slot] ? (B.flash[who][slot] & 2 ? 220 : 90) : 0;
        if (B.ko[who][slot])
        {
            alpha = 255 - B.ko[who][slot] * 4;
            y += B.ko[who][slot] / 8.0f;
        }
        /* Cards under an evolution peek out by one pixel (on the far side, turned). */
        if (B.view.players[who].field[slot].stackSize > 1 && s->settled && s->angle == s->tangle && !B.cond[who][slot])
            G_RoundFill((int)x + (who ? -1 : 1), (int)y + (who ? -1 : 1), (int)w, (int)h, 0xe8e8e0, alpha);
    }
    scale = cosf(s->flip * 3.14159f);
    if (scale < 0)
        scale = -scale;
    if (scale < 0.999f)
    {
        x += w * (1 - scale) / 2;
        w *= scale;
    }
    G_Card(id, x, y, w, h, s->angle, alpha, flash);
}
static void DrawSprites(int moving)
{
    unsigned who, i, layer;
    for (layer = 0; layer <= 50; layer++)
        for (who = 0; who < 2; who++)
            for (i = 0; i < TCG_DECK_SIZE; i++)
            {
                struct CardSprite *s = &B.sprite[who][i];
                int show;
                if (s->zone == Z_NONE || s->layer != (int)layer)
                    continue;
                if (moving != !s->settled)
                    continue;
                /* Your hand is drawn over the hand bar, in order. */
                if (!who && s->zone == Z_HAND)
                    continue;
                show = s->visible || !s->settled;
                if (!show)
                    continue;
                DrawSprite(who, i);
            }
}
static void DrawHandBar(void)
{
    const struct TcgPlayer *me = &B.view.players[0];
    int y = (int)(BarY() + Pan()), i;
    char n[8];
    G_FillA(0, y + 6, 240, 60, 0x004da7, 255);
    G_FillA(0, y + 6, 240, 2, 0x0179d0, 255);
    G_FillA(0, y + 8, 240, 1, 0x40a8f0, 255);
    (void)i;
    /* Count tab and the arrow button. */
    G_RoundFill(0, y, 26, 12, 0x303034, 255);
    G_FillA(1, y + 1, 24, 10, 0xa8a8ac, 255);
    G_FillA(1, y + 1, 24, 1, 0xe0e0e4, 255);
    snprintf(n, sizeof(n), "%02u", B.intro > 0 ? 0 : me->handSize);
    G_FillA(5, y + 2, 16, 8, 0x5a6c88, 255);
    G_Digits(13 - G_DigitsWidth(n, 1, 1) / 2, y + 2, n, 1, 1, 0xe8f0ff, 255);
    G_RoundFill(214, y, 26, 12, 0x303034, 255);
    G_FillA(215, y + 1, 24, 10, 0xa8a8ac, 255);
    G_FillA(215, y + 1, 24, 1, 0xe0e0e4, 255);
    G_ButtonIcon(222, y + 6, B.tray > 0.5f ? 6 /* up */ : 7 /* down */);
}
static void DrawOppHand(void)
{
    const struct TcgPlayer *p = &B.view.players[1];
    char n[8];
    int i, shown = B.intro > 0 ? 0 : p->handSize > 3 ? 3 : p->handSize;
    G_TextAlign(16, 62, TA_CENTER, 0x9cf0b0, G_NO_SHADOW, "HAND", TF_NARROW);
    for (i = 0; i < shown; i++)
        G_Card(0, (float)(5 + i * 5), 73.0f - (i == 1), 10, 14, (i - 1) * 8.0, 255, 0);
    snprintf(n, sizeof(n), "%u", B.intro > 0 ? 0 : p->handSize);
    G_Digits(18 + (shown ? 4 : -6), 82, n, 0, 1, 0xf8f8f8, 255);
}
static void DrawFocus(struct Rect r, unsigned color)
{
    G_Focus(r.x, r.y, r.w, r.h, color, B.frame);
}
static void Tooltip(void)
{
    char line[80] = "";
    struct Rect r = FocusRect();
    int w, x, y;
    const struct TcgPlayer *p;
    switch (B.focusKind)
    {
    case T_HAND:
        p = &B.view.players[0];
        if ((unsigned)B.focusIndex < p->handSize)
        {
            const struct TcgCard *c = Tcg_Card(p->cards[p->hand[B.focusIndex]]);
            if (c->kind == TCG_POKEMON)
                snprintf(line, sizeof(line), "%s  HP%u%s", c->name, c->hp, c->stage ? (c->stage == 1 ? "  STAGE 1" : "  STAGE 2") : "  BASIC");
            else
                snprintf(line, sizeof(line), "%s", c->name);
        }
        break;
    case T_MINE:
    case T_THEIRS:
    {
        const struct TcgCard *c = Field(B.focusKind == T_THEIRS, B.focusIndex);
        if (c && !(B.focusKind == T_THEIRS && B.view.phase == TCG_SETUP))
        {
            const struct TcgFieldCard *f = &B.view.players[B.focusKind == T_THEIRS].field[B.focusIndex];
            unsigned hp = MaxHp(c) > f->damage ? MaxHp(c) - f->damage : 0;
            snprintf(line, sizeof(line), "%s  %u/%u HP", c->name, hp, MaxHp(c));
        }
        else if (!c)
            snprintf(line, sizeof(line), "%s", B.focusIndex ? "Empty Bench space" : "Active Pokemon space");
        break;
    }
    case T_MY_DECK:
    case T_OPP_DECK:
    {
        const struct TcgPlayer *p = &B.view.players[B.focusKind == T_OPP_DECK];
        snprintf(line, sizeof(line), "%s DECK %u  HAND %u  DISCARD %u%s", B.focusKind == T_MY_DECK ? "YOUR" : "THEIR",
                 p->deckSize, p->handSize, p->discardSize, B.focusKind == T_MY_DECK ? "  {A} CONCEDE" : "");
        break;
    }
    case T_MY_DISCARD:
    case T_OPP_DISCARD:
        snprintf(line, sizeof(line), "%s DISCARD: %u  {A} LOOK", B.focusKind == T_MY_DISCARD ? "YOUR" : "THEIR",
                 B.view.players[B.focusKind == T_OPP_DISCARD].discardSize);
        break;
    case T_MY_PRIZES:
    case T_OPP_PRIZES:
        snprintf(line, sizeof(line), "%s PRIZES: %u left", B.focusKind == T_MY_PRIZES ? "YOUR" : "THEIR",
                 B.view.players[B.focusKind == T_OPP_PRIZES].prizeSize);
        break;
    case T_OPP_HAND:
        snprintf(line, sizeof(line), "THEIR HAND: %u cards%s", B.view.players[1].handSize,
                 Clairvoyant() && B.view.players[1].handSize ? "  {A} LOOK" : "");
        break;
    default:
        return;
    }
    if (!line[0])
        return;
    /* What A does here, with the controller's own button. */
    if (B.focusKind == T_HAND || (B.focusKind == T_MINE && Field(0, B.focusIndex)))
        strncat(line, "  {A}", sizeof(line) - strlen(line) - 1);
    else if (B.focusKind == T_THEIRS && Field(1, B.focusIndex) && B.view.phase != TCG_SETUP)
        strncat(line, "  {A} LOOK", sizeof(line) - strlen(line) - 1);
    w = G_Hint(0, 0, 0, 0, line, TF_NARROW, 0) + 8;
    x = r.x + r.w / 2 - w / 2;
    y = r.y + r.h + 4;
    if (B.focusKind == T_HAND)
        y = r.y - 15;
    else if (y + 12 > 150 || (B.focusKind == T_MINE && B.focusIndex))
        y = r.y - 16;
    if (x < 1)
        x = 1;
    if (x + w > 239)
        x = 239 - w;
    if (y < 1)
        y = r.y + r.h + 4;
    G_RoundFill(x, y, w, 12, 0x0c1c14, 215);
    G_RoundFrame(x, y, w, 12, 0x58d080, 255);
    G_Hint(x + 4, y, G_WHITE, G_NO_SHADOW, line, TF_NARROW, 1);
}
static void Prompt(const char *text, int top)
{
    int w = G_Hint(0, 0, 0, 0, text, TF_NARROW, 0) + 12, x = 120 - w / 2, y = top ? 1 : 140;
    if (w > 236)
    {
        /* too long for one line: two, split at the space nearest the middle */
        char a[160], b[160];
        size_t n = strlen(text), i, best = 0;
        int wa, wb;
        for (i = 1; i < n; i++)
            if (text[i] == ' ' && (best == 0 || (i > n / 2 ? i - n / 2 : n / 2 - i) < (best > n / 2 ? best - n / 2 : n / 2 - best)))
                best = i;
        snprintf(a, sizeof(a), "%.*s", (int)best, text);
        snprintf(b, sizeof(b), "%s", text + best + 1);
        wa = G_Hint(0, 0, 0, 0, a, TF_NARROW, 0);
        wb = G_Hint(0, 0, 0, 0, b, TF_NARROW, 0);
        w = (wa > wb ? wa : wb) + 12;
        if (w > 236)
            w = 236;
        x = 120 - w / 2;
        y = top ? 1 : 128;
        G_RoundFill(x, y, w, 25, 0x081830, 225);
        G_RoundFrame(x, y, w, 25, 0xf8e070, 255);
        G_Hint(120 - wa / 2, y + 1, 0xf8f0c0, G_NO_SHADOW, a, TF_NARROW, 1);
        G_Hint(120 - wb / 2, y + 12, 0xf8f0c0, G_NO_SHADOW, b, TF_NARROW, 1);
        return;
    }
    G_RoundFill(x, y, w, 13, 0x081830, 225);
    G_RoundFrame(x, y, w, 13, 0xf8e070, 255);
    G_Hint(x + 6, y + 1, 0xf8f0c0, G_NO_SHADOW, text, TF_NARROW, 1);
}
static void DrawFx(void)
{
    unsigned i;
    for (i = 0; i < FX_MAX; i++)
    {
        struct Fx *f = &B.fx[i];
        float t;
        unsigned alpha;
        if (!f->duration)
            continue;
        t = (float)f->t / f->duration;
        alpha = t > 0.8f ? (unsigned)((1 - t) / 0.2f * 255) : 255;
        switch (f->type)
        {
        case FX_TEXT:
        {
            int y = f->y - (int)(Ease(t * 2) * 8);
            int w = G_Measure(f->text, TF_NARROW);
            G_RoundFill(f->x - w / 2 - 3, y - 1, w + 6, 12, 0x101820, alpha * 3 / 4);
            G_Text(f->x - w / 2, y - 1, f->color, G_NO_SHADOW, f->text, 0, TF_NARROW);
            break;
        }
        case FX_NUMBER:
        {
            float pop = t < 0.15f ? t / 0.15f : 1;
            int scale = 2, y = f->y - (int)(Ease(t * 1.5f) * 10) - (pop < 1 ? (int)((1 - pop) * 6) : 0);
            int w = G_DigitsWidth(f->text, 1, scale);
            G_DigitsOutlined(f->x - w / 2, y, f->text, 1, scale, f->color, f->color2, alpha);
            break;
        }
        case FX_BANNER:
        {
            float open = Ease(t * 6);
            G_Banner(f->x, f->y - (int)((1 - open) * 4), f->text, f->color, open, alpha);
            break;
        }
        case FX_SWEEP:
        {
            float in = Ease(t * 4), out = t > 0.75f ? Ease((t - 0.75f) * 4) : 0;
            int w = G_Measure(f->text, TF_NORMAL), x = (int)(-w - 20 + (120 + w / 2 + 20) * in + 300 * out) - w / 2;
            int band = (int)(22 * (t < 0.1f ? t / 0.1f : t > 0.9f ? (1 - t) / 0.1f : 1));
            G_FillA(0, f->y - band / 2, 240, band, 0x081018, 170);
            G_FillA(0, f->y - band / 2, 240, 1, f->color, 255);
            G_FillA(0, f->y + band / 2 - 1, 240, 1, f->color, 255);
            if (band > 14)
                G_Text(x, f->y - 7, G_WHITE, f->color, f->text, 0, TF_NORMAL);
            break;
        }
        case FX_COIN:
        {
            float spin = t < 0.6f ? t / 0.6f * 4 : 4, bounce = t < 0.6f ? sinf(t / 0.6f * 3.14159f) * 22 : 0;
            int heads = f->value;
            if (f->t == (int)(f->duration * 0.6f))
                PlaySE(SE_BALL_BOUNCE_1);
            G_FillA(f->x - 9, f->y + 11, 18, 3, 0x003010, 120);
            G_Coin(f->x, f->y - (int)bounce, spin, heads, 255);
            if (t >= 0.62f)
            {
                const char *text = f->player == 2 ? (heads ? "YOU GO FIRST!" : "OPPONENT GOES FIRST")
                                                  : heads ? "HEADS!" : "TAILS";
                G_Banner(f->x, f->y + 24, text, heads ? 0x2858a8 : 0x704848, Ease((t - 0.62f) * 8), alpha);
            }
            break;
        }
        case FX_IMPACT:
        {
            /* a white flash, a ring going out, rays */
            int i2;
            if (t < 0.25f)
            {
                unsigned a = (unsigned)(230 * (1 - t / 0.25f));
                G_RoundFill(f->x - 9, f->y - 9, 18, 18, 0xffffff, a);
                G_FillA(0, 0, 240, 160, 0xffffff, a / 5);
            }
            G_Circle(f->x, f->y, 6 + 26 * t, f->player ? 0xa0d0ff : 0xffa060, (unsigned)(230 * (1 - t)));
            G_Circle(f->x, f->y, 4 + 18 * t, 0xffffff, (unsigned)(160 * (1 - t)));
            for (i2 = 0; i2 < 10; i2++)
            {
                float a = i2 * 0.6283f + 0.3f, r0 = 5 + 20 * t, r1 = r0 + 7 * (1 - t);
                G_Line(f->x + (int)(cosf(a) * r0), f->y + (int)(sinf(a) * r0), f->x + (int)(cosf(a) * r1),
                       f->y + (int)(sinf(a) * r1), 0xffe060, (unsigned)(255 * (1 - t)));
            }
            break;
        }
        case FX_SHARDS:
        {
            /* the card in pieces, flung out and falling */
            int i2;
            uint32_t seed = (uint32_t)f->value;
            for (i2 = 0; i2 < 12; i2++)
            {
                float vx, vy, px, py;
                seed = seed * 1103515245u + 12345u;
                vx = ((int)(seed >> 16 & 0xff) - 128) / 128.0f * 22;
                seed = seed * 1103515245u + 12345u;
                vy = -((int)(seed >> 16 & 0xff)) / 255.0f * 18 - 4;
                px = f->x + vx * t;
                py = f->y + vy * t + 30 * t * t;
                G_FillA((int)px - 1, (int)py - 1, 3 + (i2 & 1), 3, i2 % 3 ? 0xc8a050 : 0xf8f0c8, (unsigned)(255 * (1 - t)));
            }
            if (t < 0.3f)
                G_Circle(f->x, f->y, 4 + 30 * t, 0xff5030, (unsigned)(220 * (1 - t / 0.3f)));
            break;
        }
        case FX_RISE:
        {
            /* a ring opening on the spot, a beam of light going up */
            unsigned a = (unsigned)(200 * (1 - t));
            G_Circle(f->x, f->y, 4 + 18 * t, f->color, a);
            G_Circle(f->x, f->y, 2 + 11 * t, 0xffffff, a / 2);
            if (t < 0.6f)
            {
                int bw = (int)(14 * (1 - t / 0.6f)) + 2;
                G_FillA(f->x - bw / 2, f->y - 26 - (int)(10 * t), bw, 40, f->color, (unsigned)(90 * (1 - t / 0.6f)));
            }
            break;
        }
        case FX_SPARK:
        {
            int x = f->x + (int)(f->vx * f->t), y = f->y + (int)(f->vy * f->t + 0.05f * f->t * f->t);
            G_FillA(x, y, f->value, f->value, f->color, alpha);
            break;
        }
        case FX_SHOWCASE:
        {
            float in = Ease(t * 4), out = t > 0.8f ? Ease((t - 0.8f) * 5) : 0;
            float sy = f->player ? -40 : 170, w = 44, h = 61;
            struct Rect d = DiscardRect(f->player);
            if (f->color == 1) /* PEEK: it goes back */
                d = MakeRect(CENTER_X - 22, 170, 44, 61);
            float x = CENTER_X - w / 2, y = sy + (CENTER_Y - h / 2 - sy) * in;
            const struct TcgCard *c = Tcg_Card(f->value);
            x += (d.x - x) * out;
            y += (d.y - y) * out;
            w += (d.w - w) * out;
            h += (d.h - h) * out;
            if (in > 0.6f && out < 0.2f)
                G_FillA(0, 0, 240, 160, 0x000000, (unsigned)(70 * (1 - out * 5)));
            G_Card(f->value, x, y, w, h, 0, 255, 0);
            if (c && in >= 1 && out == 0)
            {
                int tw = G_Measure(c->name, TF_SMALL) + 12;
                G_Window(120 - tw / 2 - 2, (int)y + 62, tw + 4, 20);
                G_TextAlign(120, (int)y + 66, TA_CENTER, G_INK, G_INK_SHADOW, c->name, TF_SMALL);
                if (f->text[0])
                {
                    tw = G_Measure(f->text, TF_SMALL) + 12;
                    G_Window(120 - tw / 2 - 2, (int)y - 22, tw + 4, 20);
                    G_TextAlign(120, (int)y - 18, TA_CENTER, G_INK, G_INK_SHADOW, f->text, TF_SMALL);
                }
            }
            break;
        }
        }
    }
}
static void DrawMenu(void)
{
    int i, w = 0, h, x, y;
    for (i = 0; i < B.menuCount; i++)
    {
        int m = G_Measure(B.menu[i].label, TF_SMALL);
        if (m > w)
            w = m;
    }
    w += 24;
    h = B.menuCount * 13 + 10;
    x = B.menuX;
    y = B.menuY;
    if (x + w > 238)
        x = 238 - w;
    if (x < 2)
        x = 2;
    if (y + h > 158)
        y = 158 - h;
    if (y < 2)
        y = 2;
    G_Window(x, y, w, h);
    for (i = 0; i < B.menuCount; i++)
    {
        unsigned color = B.menu[i].enabled ? G_INK : 0xa8a8a8;
        if (i == B.menuCursor)
            G_Cursor(x + 7, y + 7 + i * 13);
        G_Text(x + 14, y + 4 + i * 13, color, B.menu[i].enabled ? G_INK_SHADOW : G_NO_SHADOW, B.menu[i].label, 0,
               TF_SMALL);
    }
    if (!B.menu[B.menuCursor].enabled && B.menu[B.menuCursor].reason[0])
        Prompt(B.menu[B.menuCursor].reason, y > 60);
    else if ((B.flow == FLOW_POWER || B.flow == FLOW_ATTACK) && B.step < B.nsteps)
        Prompt(B.steps[B.step].prompt, y > 60);
}
static void DrawAttacks(void)
{
    const struct TcgCard *c = TcgDuel_Attacker(&B.view, 0);
    int i;
    if (!c)
        return;
    G_Window(0, 98, 240, 62);
    for (i = 0; i < 2; i++)
    {
        const struct TcgAttack *a = &c->attacks[i];
        char reason[80], dmg[16];
        int y = 104 + i * 15, x = 16, t, ready;
        if (!a->name[0])
            continue;
        ready = AttackReady(i, reason, sizeof(reason)) || a->category == TCG_POKEMON_POWER;
        if (i == B.attackCursor)
            G_Cursor(8, y + 3);
        for (t = 0; t < TCG_TYPES; t++)
        {
            unsigned k;
            for (k = 0; k < a->cost[t]; k++)
            {
                G_EnergyOrb(x, y + 2, t, 1);
                x += 8;
            }
        }
        if (x == 16 && a->category != TCG_POKEMON_POWER)
        {
            G_Text(x, y, 0x888888, G_NO_SHADOW, "-", 0, TF_SMALL);
            x += 6;
        }
        G_Text(x + 3, y, ready && a->category != TCG_POKEMON_POWER ? G_INK : 0xa0a0a0,
               ready ? G_INK_SHADOW : G_NO_SHADOW, a->category == TCG_POKEMON_POWER ? "POKEMON POWER" : a->name, 0,
               TF_SMALL);
        if (a->category == TCG_POKEMON_POWER)
            G_Text(x + 3 + G_Measure("POKEMON POWER", TF_SMALL) + 4, y, 0xc04040, G_NO_SHADOW, a->name, 0, TF_NARROW);
        if (a->damage)
        {
            snprintf(dmg, sizeof(dmg), "%u%s", a->damage,
                     a->category == TCG_DAMAGE_PLUS ? "+" : a->category == TCG_DAMAGE_X ? "x" : a->category == TCG_DAMAGE_MINUS ? "-" : "");
            G_TextAlign(228, y, TA_RIGHT, G_INK, G_INK_SHADOW, dmg, TF_NORMAL);
        }
    }
    {
        const struct TcgAttack *a = &c->attacks[B.attackCursor];
        char reason[80];
        G_FillA(8, 134, 224, 1, 0xd0d0c8, 255);
        if (!AttackReady(B.attackCursor, reason, sizeof(reason)))
            G_Text(10, 136, 0xc04040, G_NO_SHADOW, reason, 220, TF_NARROW);
        else
            G_Text(10, 136, 0x505050, G_NO_SHADOW, a->description[0] ? a->description : "No additional effect.", 220,
                   TF_NARROW);
    }
}
static void DrawPick(void)
{
    int i, visible = 7, cw = 30, ch = 42, gap = 3, startX, rows = 1;
    int n = B.pickCount;
    G_FillA(0, 0, 240, 160, 0x000000, 150);
    G_Window(0, 0, 240, 30);
    G_Text(8, 3, G_INK, G_INK_SHADOW, B.pickTitle, 224, TF_SMALL);
    if (B.mode == M_PICK)
    {
        char line[64];
        if (B.pickRetreat)
            snprintf(line, sizeof(line), "Energy chosen: %u of %u", PickValue(), B.pickRetreat);
        else if (B.pickMin == B.pickMax)
            snprintf(line, sizeof(line), "Chosen: %u of %d", B.pickChosen, B.pickMax);
        else
            snprintf(line, sizeof(line), "Chosen: %u (up to %d)", B.pickChosen, B.pickMax);
        G_Text(8, 15, 0x586878, G_NO_SHADOW, line, 224, TF_NARROW);
    }
    if (n > 7)
        rows = 2;
    if (!n)
    {
        G_Window(40, 60, 160, 30);
        G_TextAlign(120, 68, TA_CENTER, G_INK, G_INK_SHADOW, "There are no cards here.", TF_SMALL);
    }
    B.pickScroll = B.pickCursor / (visible * rows) * (visible * rows);
    startX = 120 - (visible * (cw + gap) - gap) / 2;
    for (i = 0; i < visible * rows && B.pickScroll + i < n; i++)
    {
        int at = B.pickScroll + i, x = startX + (i % visible) * (cw + gap), y = rows == 2 ? 34 + (i / visible) * 48 : 56;
        unsigned id = CardOf(B.pickOwner, B.pickCards[at]), k;
        int chosen = -1, ok = PickValid(at);
        for (k = 0; k < B.pickChosen; k++)
            if (B.pickOrder[k] == B.pickCards[at])
                chosen = (int)k;
        if (chosen >= 0)
            y -= 4;
        G_Card(id, (float)x, (float)y, (float)cw, (float)ch, 0, ok ? 255 : 110, 0);
        if (chosen >= 0)
        {
            char num[4];
            snprintf(num, sizeof(num), "%d", chosen + 1);
            G_RoundFill(x + cw - 9, y - 3, 11, 11, 0x184890, 255);
            G_RoundFrame(x + cw - 9, y - 3, 11, 11, 0xf8f8f8, 255);
            G_Digits(x + cw - 5, y, num, 0, 1, G_WHITE, 255);
        }
        if (at == B.pickCursor)
            DrawFocus(MakeRect(x, y, cw, ch), 0xf8f070);
    }
    if (n)
    {
        const struct TcgCard *c = Tcg_Card(CardOf(B.pickOwner, B.pickCards[B.pickCursor]));
        G_Window(0, 128, 240, 32);
        if (c)
        {
            char line[80];
            if (c->kind == TCG_POKEMON)
                snprintf(line, sizeof(line), "%s  HP%u  %s", c->name, c->hp, G_TypeName(c->type));
            else
                snprintf(line, sizeof(line), "%s", c->name);
            G_Text(8, 132, G_INK, G_INK_SHADOW, line, 160, TF_SMALL);
        }
        {
            const char *hint = B.mode == M_VIEW                         ? "{A} LOOK  {B} BACK"
                               : B.pickMin != B.pickMax && !B.pickRetreat ? "{A} CHOOSE  {START} DONE  {INSPECT} LOOK"
                                                                          : "{A} CHOOSE  {INSPECT} LOOK  {B} BACK";
            int w = G_Hint(0, 0, 0, 0, hint, TF_NARROW, 0);
            G_Hint(232 - w, 145, 0x707070, G_NO_SHADOW, hint, TF_NARROW, 1);
        }
        if (n > visible * rows)
        {
            char page[16];
            snprintf(page, sizeof(page), "%d/%d", B.pickScroll / (visible * rows) + 1, (n + visible * rows - 1) / (visible * rows));
            G_TextAlign(232, 132, TA_RIGHT, 0x707070, G_NO_SHADOW, page, TF_NARROW);
        }
    }
}
static void DrawConfirm(void)
{
    int w = G_Measure(B.confirmText, TF_SMALL) + 16;
    if (w < 120)
        w = 120;
    if (w > 236)
        w = 236;
    G_FillA(0, 0, 240, 160, 0x000000, 90);
    G_Window(120 - w / 2, 54, w, 52);
    G_Text(120 - w / 2 + 8, 58, G_INK, G_INK_SHADOW, B.confirmText, w - 16, TF_SMALL);
    G_Cursor(120 - 24, 79 + B.menuCursor * 12);
    G_Text(120 - 16, 75, G_INK, G_INK_SHADOW, B.confirmKind == 3 ? "CONCEDE" : "YES", 0, TF_SMALL);
    G_Text(120 - 16, 87, G_INK, G_INK_SHADOW, B.confirmKind == 3 ? "KEEP PLAYING" : "NO", 0, TF_SMALL);
}
static void DrawResult(void)
{
    int win = B.view.winner == 0, t = B.resultTime;
    float open = Ease(t / 20.0f);
    const char *title = win ? "YOU WIN!" : "YOU LOSE...";
    G_FillA(0, 0, 240, 160, 0x000000, (unsigned)(110 * open));
    if (win && t < 80 && t % 3 == 0)
        Sparks(20 + rand() % 200, 40 + rand() % 40, rand() & 1 ? 0xf8e060 : 0x60c8f8, 3, 1.5f);
    {
        int band = (int)(34 * open);
        G_FillA(0, 48 - band / 2, 240, band, win ? 0x283868 : 0x402828, 230);
        G_FillA(0, 48 - band / 2, 240, 1, win ? 0xf8d850 : 0xa06060, 255);
        G_FillA(0, 48 + band / 2 - 1, 240, 1, win ? 0xf8d850 : 0xa06060, 255);
        if (open > 0.9f)
        {
            int w = G_Measure(title, TF_NORMAL);
            char vs[64];
            (void)w;
            G_TextAlign(120, 36, TA_CENTER, win ? 0xf8e060 : 0xe8c8c8, 0x302810, title, TF_NORMAL);
            snprintf(vs, sizeof(vs), "vs. %s", B.opponent);
            G_TextAlign(120, 51, TA_CENTER, 0xc8d0e0, G_NO_SHADOW, vs, TF_NARROW);
        }
    }
    if (t > 20)
    {
        G_Window(16, 74, 208, 56);
        G_Text(24, 79, G_INK, G_INK_SHADOW, B.confirmText, 192, TF_SMALL);
        if (B.result[0])
            G_Text(24, 93, 0x3858a0, G_INK_SHADOW, B.result, 192, TF_SMALL);
        if (t > 40 && (B.frame / 16) % 2)
            G_ButtonIcon(206, 122, 0);
    }
}
void TcgBoard_Render(void)
{
    static SDL_Texture *sLayer;
    static SDL_Renderer *sLayerOwner;
    SDL_Renderer *renderer = G_R();
    SDL_Texture *canvas = SDL_GetRenderTarget(renderer);
    unsigned who, s;
    struct Rect r;
    if (sLayerOwner != renderer)
        sLayer = NULL, sMat = NULL;
    if (!sLayer)
    {
        sLayer = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 240, LAYER_H);
        sLayerOwner = renderer;
        if (sLayer)
        {
            SDL_SetTextureBlendMode(sLayer, SDL_BLENDMODE_BLEND);
            SDL_SetTextureScaleMode(sLayer, SDL_ScaleModeNearest);
        }
    }
    if (sLayer)
    {
        SDL_SetRenderTarget(renderer, sLayer);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);
    }
    if (!sMat)
        BuildMat();
    if (sMat)
    {
        SDL_Rect whole = {0, 0, 240, LAYER_H};
        SDL_RenderCopy(G_R(), sMat, NULL, &whole);
    }
    Labels();
    /* Piles under everything that moves. */
    for (who = 0; who < 2; who++)
    {
        const struct TcgPlayer *p = &B.view.players[who];
        unsigned i;
        unsigned shownDeck = 0;
        for (i = 0; i < TCG_DECK_SIZE; i++)
            shownDeck += B.sprite[who][i].zone == Z_DECK && B.sprite[who][i].settled;
        if (B.intro > 0)
            shownDeck = TCG_DECK_SIZE;
        DrawPile(who, DeckRect(who), shownDeck, 1);
        (void)p;
    }
    DrawSprites(0);
    for (who = 0; who < 2; who++)
    {
        const struct TcgPlayer *p = &B.view.players[who];
        if (p->discardSize)
            DrawPile(who, DiscardRect(who), p->discardSize, 0);
        for (s = 0; s < TCG_FIELD_SIZE; s++)
        {
            const struct TcgCard *c = Field(who, s);
            if (!c || !(who == 0 || B.view.phase != TCG_SETUP))
                continue;
            if (!s && !B.ko[who][s] && B.sprite[who][p->field[s].stack[p->field[s].stackSize - 1]].settled &&
                StatusAngle(who, s) == 0)
                Counters(SlotRect(who, s), (unsigned)(B.shownDamage[who][s] + 0.5f), who == 1);
            FieldInfo(who, s);
        }
    }
    DrawOppHand();
    if (B.mode == M_TARGET)
    {
        /* Highlight every space the held card or choice can go. */
        for (who = 0; who < 2; who++)
            for (s = 0; s < TCG_FIELD_SIZE; s++)
                if (Valid(who ? T_THEIRS : T_MINE, s))
                {
                    r = SlotRect(who, s);
                    G_RoundFill(r.x - 1, r.y - 1, r.w + 2, r.h + 2, 0xf8f8a0, 60 + (unsigned)(40 * sinf(B.frame * 0.15f)));
                    G_RoundFrame(r.x - 2, r.y - 2, r.w + 4, r.h + 4, 0xf8f080, 255);
                }
    }
    DrawHandBar();
    /* Hand cards are drawn above the bar: redraw them settled or not. */
    {
        const struct TcgPlayer *me = &B.view.players[0];
        unsigned i;
        for (i = 0; i < me->handSize; i++)
            if (((int)i != B.heldHand || B.mode != M_TARGET) && (int)me->hand[i] != B.spotlight)
                DrawSprite(0, me->hand[i]);
    }
    DrawSprites(1);
    if (B.spotlight >= 0 && (B.spotHold || B.spotTime > 0))
    {
        /* The Prize card you took, up close with its name. */
        const struct CardSprite *sp = &B.sprite[0][B.spotlight];
        const struct TcgCard *c = Tcg_Card(CardOf(0, (unsigned)B.spotlight));
        G_FillA(0, 0, 240, LAYER_H, 0x000000, 70);
        DrawSprite(0, (unsigned)B.spotlight);
        if (c && sp->settled)
        {
            int tw = G_Measure(c->name, TF_SMALL) + 16, y = (int)(sp->y + sp->h) + 2;
            G_Window(120 - tw / 2, y, tw, 20);
            G_TextAlign(120, y + 4, TA_CENTER, G_INK, G_INK_SHADOW, c->name, TF_SMALL);
            G_Banner(120, (int)sp->y - 8, "PRIZE CARD", 0x8a6818, 1, 255);
        }
    }
    if (B.mode == M_TARGET && B.heldHand >= 0 && (unsigned)B.heldHand < B.view.players[0].handSize)
        DrawSprite(0, B.view.players[0].hand[B.heldHand]);
    DrawFx();
    if (TcgBoard_Idle() && B.mode != M_PICK && B.mode != M_VIEW && B.mode != M_RESULT)
    {
        r = FocusRect();
        if (B.mode != M_ATTACK)
            DrawFocus(r, B.mode == M_TARGET ? 0xf8f070 : G_WHITE);
        if (B.mode == M_FREE)
            Tooltip();
    }
    if (B.mode == M_MENU)
        DrawMenu();
    if (sLayer)
    {
        SDL_Rect dst = {0, -(int)(Pan() + 0.5f), 240, LAYER_H};
        SDL_SetRenderTarget(renderer, canvas);
        SDL_RenderCopy(renderer, sLayer, NULL, &dst);
    }
    /* What the player is being asked. */
    if (TcgBoard_Idle() && B.mode != M_RESULT)
    {
        if (B.mode == M_TARGET)
        {
            int who;
            const char *text = B.flow == FLOW_BASIC     ? (B.view.phase == TCG_SETUP && !B.view.players[0].field[0].stackSize
                                                               ? "Put it in your Active space."
                                                               : "Put it on which Bench space?")
                               : B.flow == FLOW_EVOLVE  ? "Evolve which Pokemon?"
                               : B.flow == FLOW_ATTACH  ? "Attach the Energy to which Pokemon?"
                               : B.flow == FLOW_RETREAT ? "Switch in which Benched Pokemon?"
                               : B.step < B.nsteps      ? B.steps[B.step].prompt
                                                        : "";
            char line[96];
            TargetFilter(&who);
            snprintf(line, sizeof(line), "%s  {B}", text);
            Prompt(line, !who);
        }
        else if (B.mode == M_FREE && B.view.phase == TCG_PROMOTION && MyTurn())
            Prompt(B.view.switchPending ? "Your Active Pokemon must switch out: choose who comes in. {A}"
                                        : "Choose a Benched Pokemon to be your new Active Pokemon. {A}",
                   0);
        else if (B.mode == M_FREE && B.view.phase == TCG_SETUP && MyTurn())
            Prompt(B.view.players[0].field[0].stackSize ? "Add Benched Pokemon if you like. {B} READY"
                                                        : "Choose a Basic Pokemon to be your Active Pokemon.",
                   1);
        else if (B.waiting)
            Prompt("Waiting for your friend...", 1);
    }
    switch (B.mode)
    {
    case M_ATTACK:
        DrawAttacks();
        Prompt("{A} ATTACK   {UPDOWN} CHOOSE   {INSPECT} LOOK   {B} BACK", 1);
        break;
    case M_PICK:
    case M_VIEW:
        DrawPick();
        break;
    case M_CONFIRM:
        DrawConfirm();
        break;
    case M_RESULT:
        DrawResult();
        DrawFx();
        break;
    }
    if (B.toastTime && B.toast[0])
    {
        int w = G_Measure(B.toast, TF_SMALL) + 16, h = 24;
        if (w > 224)
            w = 224, h = 36;
        G_Window(120 - w / 2, 62, w, h);
        G_Text(120 - w / 2 + 8, 66, G_INK, G_INK_SHADOW, B.toast, w - 16, TF_SMALL);
    }
}
