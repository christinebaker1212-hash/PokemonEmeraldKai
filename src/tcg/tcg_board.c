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
    KEY_L = 512
};

/* ------------------------------------------------------------------ */
/* Layout: the DS mat turned to the wide screen. You are at the bottom. */
/* ------------------------------------------------------------------ */
#define MAT 0x018242
#define MAT_LINE 0x1bc55c
#define MAT_LABEL 0x16a457
#define BENCH_W 22
#define BENCH_H 30
#define ACT_W 30
#define ACT_H 42
#define ACT_X 105
#define OPP_ACT_Y 33
#define MY_ACT_Y 81
#define OPP_BENCH_Y 1
#define MY_BENCH_Y 123
#define PILE_W 18
#define PILE_H 25
#define PRIZE_W 12
#define PRIZE_H 16
#define HAND_W 30
#define HAND_H 42
#define CENTER_X 120
#define CENTER_Y 78
#define BAR_CLOSED 152
#define BAR_OPEN 118
#define PAN 37
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
static struct Rect SlotRect(unsigned who, unsigned slot)
{
    unsigned column;
    if (!slot)
        return MakeRect(ACT_X, who ? OPP_ACT_Y : MY_ACT_Y, ACT_W, ACT_H);
    /* The opponent's bench fills from their left: our right. */
    column = who ? 5 - slot : slot - 1;
    return MakeRect(41 + (int)column * 34, who ? OPP_BENCH_Y : MY_BENCH_Y, BENCH_W, BENCH_H);
}
static struct Rect DeckRect(unsigned who) { return who ? MakeRect(7, 31, PILE_W, PILE_H) : MakeRect(215, 100, PILE_W, PILE_H); }
static struct Rect DiscardRect(unsigned who)
{
    return who ? MakeRect(7, 3, PILE_W, PILE_H) : MakeRect(215, 128, PILE_W, PILE_H);
}
static struct Rect PrizeRect(unsigned who, unsigned i)
{
    return who ? MakeRect(211 + (int)(i % 2) * 14, 3 + (int)(i / 2) * 18, PRIZE_W, PRIZE_H)
               : MakeRect(3 + (int)(i % 2) * 14, 98 + (int)(i / 2) * 18, PRIZE_W, PRIZE_H);
}
static struct Rect PrizeArea(unsigned who) { return who ? MakeRect(210, 2, 28, 54) : MakeRect(2, 97, 28, 54); }
static const struct Rect sEndButton = {184, 79, 52, 15};
static const struct Rect sOppHand = {2, 62, 28, 30};

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
    FX_SHOWCASE /* a played Trainer card held up in the middle */
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
    M_QUICK,
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
    T_OPP_HAND,
    T_END
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
    FLOW_ATTACK
};
enum
{
    ST_MY_SLOT,
    ST_OPP_SLOT,
    ST_PICK,
    ST_STAGE
};
enum
{
    SRC_HAND,
    SRC_DECK,
    SRC_MY_DISCARD,
    SRC_OPP_DISCARD,
    SRC_MY_ATTACHED,
    SRC_OPP_ATTACHED,
    SRC_TOP5
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
    F_ANY_POKEMON
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
    MENU_STAGE1,
    MENU_STAGE2
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
    u16 music;
    int speed;
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
    const struct TcgCard *c = Field(0, 0);
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
    if (f->condition == TCG_ASLEEP)
        return snprintf(reason, size, "%s is Asleep.", c->name), 0;
    if (f->condition == TCG_PARALYZED)
        return snprintf(reason, size, "%s is Paralyzed.", c->name), 0;
    if (f->noAttack)
        return snprintf(reason, size, "%s can't attack this turn.", c->name), 0;
    if (!Field(1, 0))
        return snprintf(reason, size, "There is no Defending Pokemon."), 0;
    return 1;
}
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
    if (TcgDuel_Energy(&B.view, 0, 0, TCG_NONE) < c->retreat)
        return snprintf(reason, size, "Retreating costs %u Energy.", c->retreat), 0;
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
                    B.sprite[who][f->stack[j]].tangle = (float)StatusAngle(who, s);
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
        Banner(x, who ? y + 30 : y - 30, "KNOCKED OUT!", 0xb02828, fast ? 40 : 64);
        PlaySE(SE_FAINT);
        return fast ? 34 : 56;
    case TCG_EV_TRAINER:
    {
        struct Fx *f = AddFx(FX_SHOWCASE, CENTER_X, CENTER_Y, fast ? 42 : 72);
        if (f)
        {
            f->value = e->card;
            f->player = who;
        }
        PlaySE(SE_RG_CARD_FLIP);
        return fast ? 34 : 60;
    }
    case TCG_EV_RETREAT_FAIL:
        CardCenter(who, 0, &x, &y);
        Text(x, y - 26, "RETREAT FAILED", 0xf8c0c0, 46);
        PlaySE(SE_FAILURE);
        return 40;
    case TCG_EV_PLAY:
    case TCG_EV_PROMOTE:
        if (post)
            PlaySE(SE_RG_CARD_FLIP);
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
    B.npre = B.npost = 0;
    if (next->eventSeq < seq || next->eventSeq - seq > TCG_EVENT_RING)
        seq = next->eventSeq > TCG_EVENT_RING ? next->eventSeq - TCG_EVENT_RING : 0;
    for (; seq < next->eventSeq; seq++)
    {
        const struct TcgEvent *e = TcgDuel_GetEvent(next, seq);
        if (!e)
            continue;
        if (IsPostEvent(e->type))
            B.post[B.npost++] = *e;
        else
            B.pre[B.npre++] = *e;
    }
    B.phase = P_PRE;
    B.index = 0;
    B.wait = 0;
}
static void Commit(const struct TcgDuel *next)
{
    unsigned i, moved = 0, who;
    int before[2][TCG_DECK_SIZE];
    for (who = 0; who < 2; who++)
        for (i = 0; i < TCG_DECK_SIZE; i++)
            before[who][i] = B.sprite[who][i].zone * 16 + B.sprite[who][i].slot;
    B.view = *next;
    ResetShown();
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
void TcgBoard_Begin(const struct TcgDuel *truth, const char *opponent, int online)
{
    unsigned who, i;
    struct Fx *f;
    memset(&B, 0, sizeof(B));
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
            s->settled = 1;
        }
    B.focusKind = T_HAND;
    B.heldHand = -1;
    B.mode = M_FREE;
    B.music = GetCurrentMapMusic();
    PlayBGM(MUS_B_ARENA);
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
        PlayBGM(B.music);
    B.active = 0;
}
void TcgBoard_Sync(const struct TcgDuel *truth)
{
    const struct TcgDuel *last = B.qCount ? &B.queue[(B.qHead + B.qCount - 1) % QUEUE] : &B.view;
    if (truth->eventSeq == last->eventSeq && !memcmp(truth->players, last->players, sizeof(truth->players)) &&
        truth->phase == last->phase && truth->current == last->current)
        return;
    if (B.qCount == QUEUE)
    {
        /* Falling far behind: drop the oldest pending state. */
        B.qHead = (B.qHead + 1) % QUEUE;
        B.qCount--;
        if (B.phase != P_IDLE)
            B.phase = P_IDLE;
    }
    B.queue[(B.qHead + B.qCount) % QUEUE] = *truth;
    B.qCount++;
}
int TcgBoard_Idle(void) { return B.intro <= 0 && B.phase == P_IDLE && !B.qCount && SpritesSettled(); }
void TcgBoard_SetWaiting(int waiting) { B.waiting = waiting; }
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
    ADD(T_END, 0, sEndButton);
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
#undef STEPS
    *count = 0;
    return sNoSteps;
}
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
        *who = TcgDuel_AttackChoice(&Field(0, 0)->attacks[B.action.attack], NULL) == 1;
        return F_BENCH;
    case FLOW_TRAINER:
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
        int n = st->max;
        if (st->source == SRC_TOP5)
            n = B.view.players[0].deckSize < 5 ? B.view.players[0].deckSize : 5;
        OpenPick(st->source, st->filter, st->source == SRC_TOP5 ? n : st->min, n, st->prompt);
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
    }
}

/* Picker over a pile: the hand, the deck while searching, attached Energy... */
static void OpenPick(int source, int filter, int min, int max, const char *title)
{
    const struct TcgPlayer *p;
    unsigned owner = source == SRC_OPP_DISCARD || source == SRC_OPP_ATTACHED, i, n = 0;
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
    const struct TcgCard *active = Field(0, 0);
    const struct TcgFieldCard *f = &B.view.players[0].field[0];
    unsigned i, total = 0, types = 0, first = TCG_NONE;
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
    if (!active->retreat || total == active->retreat || types == 1)
    {
        /* Nothing to choose: pay with the first Energy that covers it. */
        unsigned paid = 0;
        for (i = 0; i < f->attachedSize && paid < active->retreat; i++)
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
        OpenPick(SRC_MY_ATTACHED, F_ENERGY, 1, (int)active->retreat, "Discard Energy to retreat.");
        B.action.slot = slot;
        B.pickRetreat = (int)active->retreat;
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
        AddMenu("POKEMON POWER", MENU_POWER, 0, "This Pokemon Power isn't ready yet.");
    AddMenu("CHECK", MENU_CHECK, 1, NULL);
    Menu(r.x + r.w + 4 > 170 ? r.x - 74 : r.x + r.w + 4, r.y > 100 ? 92 : r.y);
}
static void OpenQuickMenu(void)
{
    char reason[80];
    const struct TcgCard *c = Field(0, 0);
    B.menuCount = 0;
    AddMenu("HAND", MENU_HAND, 1, NULL);
    AddMenu("ATTACK", MENU_ATTACK, CanAct() && c && B.view.phase == TCG_TURN, "You can't attack now.");
    AddMenu("CHECK", MENU_LOOK, 1, NULL);
    AddMenu("PKMN POWER", MENU_POWER, 0, "Pokemon Powers aren't ready yet.");
    AddMenu("RETREAT", MENU_RETREAT, CanAct() && B.view.phase == TCG_TURN && CanRetreat(reason, sizeof(reason)), reason);
    AddMenu("DONE", MENU_DONE, CanAct() && B.view.phase == TCG_TURN, "It isn't your turn.");
    B.mode = M_QUICK;
    B.menuCursor = 0;
    PlaySE(SE_SELECT);
}
static void Confirm(int kind, const char *text)
{
    B.confirmKind = kind;
    snprintf(B.confirmText, sizeof(B.confirmText), "%s", text);
    B.menuCursor = 0;
    B.mode = M_CONFIRM;
    PlaySE(SE_SELECT);
}
static void EndTurn(void)
{
    if (B.view.phase == TCG_SETUP)
    {
        if (!B.view.players[0].field[0].stackSize)
        {
            TcgBoard_Reject("Choose your Active Pokemon first.");
            return;
        }
        Confirm(1, "Ready to start the duel?");
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
    Confirm(2, "End your turn?");
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
    case MENU_LOOK:
        B.mode = M_FREE;
        B.focusKind = T_THEIRS;
        B.focusIndex = 0;
        Toast("Move over any face-up card. START shows it up close.");
        break;
    case MENU_DONE:
        B.mode = M_FREE;
        EndTurn();
        break;
    case MENU_STAGE1:
    case MENU_STAGE2:
        B.action.attack = id == MENU_STAGE1 ? 1 : 2;
        B.step++;
        B.mode = M_FREE;
        ContinueTrainer(out, result);
        break;
    default:
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
    if (pressed & KEY_START && n)
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
    if (pressed & KEY_SELECT)
    {
        if (PickDone())
            FinishPick(out, result);
        else
            TcgBoard_Reject(B.pickSource == SRC_DECK && !B.pickChosen && B.pickMin
                                ? "Choose a card (B cancels)."
                                : "Choose the required number of cards.");
    }
}

/* ------------------------------------------------------------------ */
/* Input                                                                */
/* ------------------------------------------------------------------ */
int TcgBoard_Step(unsigned pressed, unsigned held, struct TcgAction *out, unsigned *card)
{
    int result = BOARD_NONE, i, steps;
    B.frame++;
    B.speed = held & (KEY_A | KEY_B) && !TcgBoard_Idle() ? 3 : 1;
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
        float want = B.trayWanted && B.focusKind == T_HAND && (B.mode == M_FREE || B.mode == M_MENU) && MyTurn() &&
                             B.phase == P_IDLE && !B.qCount && B.intro <= 0
                         ? 1.0f
                         : 0.0f;
        B.tray += (want - B.tray) * 0.3f;
        if (fabsf(want - B.tray) < 0.01f)
            B.tray = want;
    }
    /* Hand positions follow the tray continuously. */
    if (!B.intro)
        Layout();
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
    case M_QUICK:
    {
        int cols = B.mode == M_QUICK ? 3 : 1, n = B.menuCount;
        if (pressed & KEY_UP)
            B.menuCursor = cols == 1 ? (B.menuCursor + n - 1) % n : B.menuCursor ^ 1;
        if (pressed & KEY_DOWN)
            B.menuCursor = cols == 1 ? (B.menuCursor + 1) % n : B.menuCursor ^ 1;
        if (cols > 1 && pressed & KEY_LEFT)
            B.menuCursor = (B.menuCursor + n - 2) % n;
        if (cols > 1 && pressed & KEY_RIGHT)
            B.menuCursor = (B.menuCursor + 2) % n;
        if (pressed & (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT))
            PlaySE(SE_RG_BAG_CURSOR);
        if (pressed & KEY_B)
        {
            if (B.flow == FLOW_TRAINER)
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
            const struct TcgCard *c = Field(0, 0);
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
        else if (pressed & KEY_START)
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
                const struct TcgCard *c = Field(0, 0);
                unsigned choice = TcgDuel_AttackChoice(&c->attacks[B.attackCursor], NULL), k, any = 0;
                memset(&B.action, 0, sizeof(B.action));
                B.action.type = TCG_ATTACK;
                B.action.attack = (uint8_t)B.attackCursor;
                for (k = 1; k < TCG_FIELD_SIZE && choice; k++)
                    any |= Field(choice == 1, k) != NULL;
                if (choice && any)
                {
                    /* Spark, Lure, Teleport...: pick the Benched Pokemon first. */
                    B.flow = FLOW_ATTACK;
                    B.heldHand = -1;
                    B.mode = M_FREE;
                    B.focusKind = choice == 1 ? T_THEIRS : T_MINE;
                    B.focusIndex = 1;
                    StartTarget();
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
                Submit(out, &result);
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
    if (pressed & KEY_START)
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
    if (pressed & KEY_SELECT)
    {
        OpenQuickMenu();
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
        else
            Confirm(3, "Concede this duel? You won't lose any cards.");
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
        case T_END:
            if (!CanAct() || B.view.phase == TCG_PROMOTION)
                TcgBoard_Reject(MyTurn() ? "Wait a moment." : "It isn't your turn.");
            else
                EndTurn();
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
static void BuildMat(void)
{
    struct GSurface s = G_SurfaceNew(240, 160);
    int x;
    if (!s.px)
        return;
    S_Fill(&s, 0, 0, 240, 160, ARGB(MAT, 255));
    /* Pokemon-ball circle in the middle of the mat. */
    S_Ring(&s, CENTER_X, CENTER_Y, 37, 2.2f, ARGB(MAT_LINE, 255));
    for (x = CENTER_X - 36; x <= CENTER_X + 36; x++)
        if (abs(x - CENTER_X) > 9)
        {
            S_Px(&s, x, CENTER_Y - 1, ARGB(MAT_LINE, 255));
            S_Px(&s, x, CENTER_Y, ARGB(MAT_LINE, 255));
        }
    S_Ring(&s, CENTER_X, CENTER_Y, 10, 2.2f, ARGB(MAT_LINE, 255));
    S_Ring(&s, CENTER_X, CENTER_Y, 5.5f, 1.6f, ARGB(MAT_LINE, 255));
    /* Zones. */
    S_RoundFrame(&s, 34, 0, 172, 33, 3, ARGB(MAT_LINE, 255));
    S_RoundFrame(&s, 34, 121, 172, 33, 3, ARGB(MAT_LINE, 255));
    for (x = 1; x < 5; x++)
    {
        S_Fill(&s, 34 + x * 34, 3, 1, 27, ARGB(MAT_LINE, 110));
        S_Fill(&s, 34 + x * 34, 124, 1, 27, ARGB(MAT_LINE, 110));
    }
    S_RoundFrame(&s, 1, 1, 30, 58, 3, ARGB(MAT_LINE, 255));
    S_RoundFrame(&s, 209, 1, 30, 58, 3, ARGB(MAT_LINE, 255));
    S_RoundFrame(&s, 1, 96, 30, 57, 3, ARGB(MAT_LINE, 255));
    S_RoundFrame(&s, 209, 96, 30, 57, 3, ARGB(MAT_LINE, 255));
    S_RoundFrame(&s, 1, 61, 30, 33, 3, ARGB(MAT_LINE, 150));
    /* Pile and prize outlines. */
    {
        unsigned who, i;
        for (who = 0; who < 2; who++)
        {
            struct Rect d = DeckRect(who), t = DiscardRect(who);
            S_RoundFrame(&s, d.x, d.y, d.w, d.h, 1, ARGB(MAT_LINE, 120));
            S_RoundFrame(&s, t.x, t.y, t.w, t.h, 1, ARGB(MAT_LINE, 120));
            for (i = 0; i < 6; i++)
            {
                struct Rect p = PrizeRect(who, i);
                S_RoundFrame(&s, p.x, p.y, p.w, p.h, 1, ARGB(MAT_LINE, 90));
            }
        }
    }
    sMat = G_SurfaceTexture(&s);
    G_SurfaceFree(&s);
}
static void Labels(void)
{
    G_TextAlign(120, 8, TA_CENTER, MAT_LABEL, G_NO_SHADOW, "BENCH", TF_NARROW);
    G_TextAlign(120, 130, TA_CENTER, MAT_LABEL, G_NO_SHADOW, "BENCH", TF_NARROW);
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
static void Counters(struct Rect r, unsigned damage)
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
        G_Fill(x + 1, y, 3, 1, 0x501018);
        G_Fill(x, y + 1, 5, 3, 0x501018);
        G_Fill(x + 1, y + 4, 3, 1, 0x501018);
        G_Fill(x + 1, y + 1, 3, 3, c);
        G_Fill(x + 1, y + 1, 1, 1, G_Mix(c, 0xffffff, 0.6f));
    }
}
/* The original's substatus as small tags beside the conditions. Returns the
 * width; right-aligned to x when right is set. */
static int Substatus(int x, int y, const struct TcgFieldCard *f, int right, int draw)
{
    static const char *const guards[] = {"", "-10", "-20", "<40", "HALF"};
    static const char *const weakens[] = {"", "ATK-10", "ATK-20", "BLIND"};
    const char *labels[6];
    unsigned colors[6], n = 0, i;
    int w = 0;
    if (f->barrier)
        labels[n] = "SAFE", colors[n++] = 0x3070c8;
    else if (f->guard && f->guard < 5)
        labels[n] = guards[f->guard], colors[n++] = 0x3070c8;
    if (f->weaken && f->weaken < 4)
        labels[n] = weakens[f->weaken], colors[n++] = 0xc06020;
    if (f->noAttack)
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
            Substatus(x, statusY, f, 0, 1);
            for (i = 0; i < n && i < 8; i++)
                G_EnergyOrb(plateX + i * 8, orbY, types[i], 1);
        }
        else
        {
            int right = 98, w;
            plateX = right - G_HpPlateWidth(0), plateY = 82, orbY = 95, statusY = 104, nameY = 111;
            nameX = right - G_Measure(name, TF_NARROW);
            G_Text(nameX, nameY, G_WHITE, 0x104020, name, 0, TF_NARROW);
            w = (B.poison[who][slot] ? G_Measure("PSN", TF_NARROW) + 5 : 0) +
                (B.cond[who][slot] ? G_Measure("SLP", TF_NARROW) + 5 : 0);
            G_StatusBadges(right - w, statusY, B.cond[who][slot], B.poison[who][slot]);
            Substatus(right - w, statusY, f, 1, 1);
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
        /* Cards under an evolution peek out by one pixel. */
        if (B.view.players[who].field[slot].stackSize > 1 && s->settled && s->angle == 0)
            G_RoundFill((int)x + 1, (int)y + 1, (int)w, (int)h, 0xe8e8e0, alpha);
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
    int o = (B.frame / 8) % 2 + 1, len = 4, x0 = r.x - o - 1, y0 = r.y - o - 1, x1 = r.x + r.w + o, y1 = r.y + r.h + o;
    unsigned shadow = 0x102010;
    G_Fill(x0 + 1, y0 + 1, len, 1, shadow);
    G_Fill(x0 + 1, y0 + 1, 1, len, shadow);
    G_Fill(x1 - len + 1, y0 + 1, len, 1, shadow);
    G_Fill(x1 + 1, y0 + 1, 1, len, shadow);
    G_Fill(x0 + 1, y1 + 1, len, 1, shadow);
    G_Fill(x0 + 1, y1 - len + 2, 1, len, shadow);
    G_Fill(x1 - len + 1, y1 + 1, len + 1, 1, shadow);
    G_Fill(x1 + 1, y1 - len + 2, 1, len, shadow);
    G_Fill(x0, y0, len, 1, color);
    G_Fill(x0, y0, 1, len, color);
    G_Fill(x1 - len + 1, y0, len, 1, color);
    G_Fill(x1, y0, 1, len, color);
    G_Fill(x0, y1, len, 1, color);
    G_Fill(x0, y1 - len + 1, 1, len, color);
    G_Fill(x1 - len + 1, y1, len, 1, color);
    G_Fill(x1, y1 - len + 1, 1, len, color);
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
        snprintf(line, sizeof(line), "%s DECK: %u cards", B.focusKind == T_MY_DECK ? "YOUR" : "THEIR",
                 B.view.players[B.focusKind == T_OPP_DECK].deckSize);
        break;
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
        snprintf(line, sizeof(line), "THEIR HAND: %u cards", B.view.players[1].handSize);
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
        w = 236, x = 2;
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
}
static void DrawQuick(void)
{
    int i;
    G_Window(0, 116, 240, 44);
    for (i = 0; i < B.menuCount; i++)
    {
        int x = 16 + (i / 2) * 76, y = 122 + (i % 2) * 16;
        unsigned color = B.menu[i].enabled ? G_INK : 0xa8a8a8;
        if (i == B.menuCursor)
            G_Cursor(x - 8, y + 3);
        G_Text(x, y, color, B.menu[i].enabled ? G_INK_SHADOW : G_NO_SHADOW, B.menu[i].label, 0, TF_SMALL);
    }
}
static void DrawAttacks(void)
{
    const struct TcgCard *c = Field(0, 0);
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
                               : B.pickMin != B.pickMax && !B.pickRetreat ? "{A} CHOOSE  {SELECT} DONE  {START} LOOK"
                                                                          : "{A} CHOOSE  {START} LOOK  {B} BACK";
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
        sLayer = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 240, 200);
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
        SDL_RenderCopy(G_R(), sMat, NULL, NULL);
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
                Counters(SlotRect(who, s), (unsigned)(B.shownDamage[who][s] + 0.5f));
            FieldInfo(who, s);
        }
    }
    DrawOppHand();
    /* END TURN / READY. */
    {
        int state = !CanAct() || B.view.phase == TCG_PROMOTION ? G_BUTTON_DISABLED
                    : B.focusKind == T_END && B.mode == M_FREE     ? G_BUTTON_FOCUS
                                                                   : G_BUTTON_NORMAL;
        const char *label = B.view.phase == TCG_SETUP ? "READY" : MyTurn() ? "END TURN" : "WAIT...";
        if (B.view.phase == TCG_FINISHED)
            label = "END TURN";
        G_Button(sEndButton.x, sEndButton.y, sEndButton.w, sEndButton.h, label, state);
    }
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
            if ((int)i != B.heldHand || B.mode != M_TARGET)
                DrawSprite(0, me->hand[i]);
    }
    DrawSprites(1);
    if (B.mode == M_TARGET && B.heldHand >= 0 && (unsigned)B.heldHand < B.view.players[0].handSize)
        DrawSprite(0, B.view.players[0].hand[B.heldHand]);
    DrawFx();
    if (TcgBoard_Idle() && B.mode != M_PICK && B.mode != M_VIEW && B.mode != M_RESULT && B.mode != M_QUICK)
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
        SDL_Rect dst = {0, -(int)(Pan() + 0.5f), 240, 200};
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
                               : B.flow == FLOW_ATTACK  ? (TcgDuel_AttackChoice(&Field(0, 0)->attacks[B.action.attack], NULL) == 1
                                                               ? "Aim at which of their Benched Pokemon?"
                                                               : "Switch with which of your Benched Pokemon?")
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
            Prompt(B.view.players[0].field[0].stackSize ? "Add Benched Pokemon if you like, then READY."
                                                        : "Choose a Basic Pokemon to be your Active Pokemon.",
                   1);
        else if (B.waiting)
            Prompt("Waiting for your friend...", 1);
    }
    switch (B.mode)
    {
    case M_QUICK:
        DrawQuick();
        break;
    case M_ATTACK:
        DrawAttacks();
        Prompt("{A} ATTACK   {UPDOWN} CHOOSE   {START} CARD   {B} BACK", 1);
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
