// Moemon Kai: the Magic duel table on the card screens' 240x160 canvas. From the top: the opponent's bar (name,
// life, hand, library), their lands (alike ones stacked, "x3"), their creatures and other permanents, the middle line
// (the step, the top of the stack, what just happened), yours, your lands, your hand, your bar. A tapped card lies on
// its side. It follows the engine (mtg_game.c) by its queries and events.
// Buttons: the d-pad moves over the cards, A plays or uses the one under the cursor (a menu when it can do more than
// one thing, then the cursor goes round the legal targets), START passes, INSPECT (R2 / D) or SELECT shows a card's
// whole text, L held stops at every chance to act (else the table passes for you when nothing is worth doing).
#include <SDL2/SDL.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "sound.h"
#include "constants/songs.h"
#include "mp.h"
#include "mp_kaisong_scene.h"
#include "mtg/integration.h"
#include "mtg_internal.h"
#include "mtg_board.h"
#include "../tcg/tcg_gfx.h"
#include "../tcg/tcg_ui_kit.h"

u16 GetCurrentMapMusic(void);

enum
{
    KEY_A = 1, KEY_B = 2, KEY_SELECT = 4, KEY_START = 8, KEY_RIGHT = 16, KEY_LEFT = 32, KEY_UP = 64, KEY_DOWN = 128,
    KEY_R = 256, KEY_L = 512,
    KEY_INSPECT = 2048
};
enum { ROW_OPP_LANDS, ROW_OPP_PERMS, ROW_MY_PERMS, ROW_MY_LANDS, ROW_HAND, ROW_COUNT };
enum { MODE_NORMAL, MODE_MENU, MODE_TARGET, MODE_ATTACK, MODE_BLOCK, MODE_BLOCK_PICK, MODE_DISCARD, MODE_RESULT };

#define ME 0
#define THEM 1
#define ROW_ITEMS 40
#define TICKER 6
#define TICKER_FRAMES 50

typedef struct { int16_t x, y, w, h; uint8_t on; } Rect;
typedef struct { uint16_t obj; uint8_t count; } Item;

static struct
{
    int active;
    MtgDuel duel;
    char opp[24];
    unsigned frame;
    int mode, row, col[ROW_COUNT];
    int aiWait, autoPassWait, autoplay;
    // what the player is choosing
    uint16_t menu[8];               // MODE_MENU: action indexes, one per (kind, ability)
    int menuCount, menuCursor;
    uint16_t targets[MTG_MAX_ACTIONS];   // MODE_TARGET: action indexes
    int targetCount, targetCursor;
    uint32_t attack;
    int8_t block[MTG_MAX_COMBAT];
    int blockFrom, blockCursor;
    int inspect, inspectObj;
    // the layout of this frame
    Item items[ROW_COUNT][ROW_ITEMS];
    int itemCount[ROW_COUNT];
    Rect rect[MTG_MAX_OBJS];
    Rect playerRect[2];
    // what happened
    char ticker[TICKER][96];
    int tickerCount, tickerAge;
    uint32_t eventSeen;
    char hold[96];                  // a note for the player ("nothing to do with it now")
    int holdAge;
    int result, resultAge, forced;
    u16 music, mapMusic;
} B;

static u16 sHeldMusic;

int MtgBoard_Active(void) { return B.active; }
void MtgBoard_Autoplay(int on) { B.autoplay = on; }

static const MtgCardDef *Def(int obj) { return Mtg_Card(B.duel.objs[obj].def); }

static void CostText(char *out, unsigned cap, const MtgManaCost *c);
static void Name(char *out, unsigned cap, int obj)
{
    Mtg_PlainText(out, cap, Def(obj)->name);
}

static const char *Who(int player)
{
    return player == ME ? "YOU" : B.opp;
}

static void Ticker(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
static void Ticker(const char *fmt, ...)
{
    va_list ap;
    if (B.tickerCount == TICKER) {
        memmove(B.ticker[0], B.ticker[1], sizeof(B.ticker[0]) * (TICKER - 1));
        B.tickerCount--;
    }
    va_start(ap, fmt);
    vsnprintf(B.ticker[B.tickerCount++], sizeof(B.ticker[0]), fmt, ap);
    va_end(ap);
    if (B.tickerCount == 1)
        B.tickerAge = 0;
}

static void Hold(const char *text)
{
    snprintf(B.hold, sizeof(B.hold), "%s", text);
    B.holdAge = 0;
}

// ---- starting and ending
void MtgBoard_Start(const MtgDeck *mine, const MtgDeck *theirs, uint32_t seed, const char *opponent)
{
    memset(&B, 0, sizeof(B));
    B.active = 1;
    snprintf(B.opp, sizeof(B.opp), "%s", opponent);
    Mtg_DuelInit(&B.duel, mine, theirs, seed);
    B.row = ROW_HAND;
    B.mapMusic = GetCurrentMapMusic();
    B.music = MUS_RG_VS_TRAINER;
    PlayBGM(B.music);
    B.aiWait = 30;
    MPNet_Log("mtg: table: you vs %s, seed %u, %s first", opponent, seed, Who(B.duel.startingPlayer));
    Ticker("%s goes first.", B.duel.startingPlayer == ME ? "You" : B.opp);
}

void MtgBoard_End(void)
{
    if (B.active && B.mapMusic)
        MP_MUSIC_TO_MAP(PlayBGM(B.mapMusic));
    B.active = 0;
}

void MtgBoard_EndHoldMusic(void)
{
    if (B.active && B.mapMusic)
        sHeldMusic = B.mapMusic;
    B.active = 0;
}

void MtgBoard_ReleaseMusic(void)
{
    u16 m = sHeldMusic;
    sHeldMusic = 0;
    if (m)
        MP_MUSIC_TO_MAP(PlayBGM(m));
}

void MtgBoard_ForceEnd(int win)
{
    if (!B.active)
        return;
    B.duel.over = 1;
    B.duel.winner = (int8_t)(win ? ME : THEM);
    B.duel.query.kind = MTG_Q_GAME_OVER;
    B.duel.query.winner = B.duel.winner;
    B.forced = 1;
}

// ---- the layout
static int IsLand(int obj) { return (Def(obj)->types & MTG_T_LAND) != 0; }

static void BuildRows(void)
{
    const MtgDuel *d = &B.duel;
    int i, r;
    for (r = 0; r < ROW_COUNT; r++)
        B.itemCount[r] = 0;
    for (i = 0; i < d->objCount; i++) {
        const MtgObj *o = &d->objs[i];
        int row = -1;
        if (o->zone == MTG_ZONE_HAND && o->owner == ME)
            row = ROW_HAND;
        else if (o->zone == MTG_ZONE_BATTLEFIELD)
            row = IsLand(i) ? (o->controller == ME ? ROW_MY_LANDS : ROW_OPP_LANDS)
                            : (o->controller == ME ? ROW_MY_PERMS : ROW_OPP_PERMS);
        if (row < 0)
            continue;
        if (row == ROW_MY_LANDS || row == ROW_OPP_LANDS) {   // alike lands stack (same card, same tapped)
            int k, found = 0;
            for (k = 0; k < B.itemCount[row]; k++) {
                const MtgObj *other = &d->objs[B.items[row][k].obj];
                if (other->def == o->def && other->tapped == o->tapped) {
                    B.items[row][k].count++;
                    found = 1;
                    break;
                }
            }
            if (found)
                continue;
        }
        if (B.itemCount[row] < ROW_ITEMS) {
            B.items[row][B.itemCount[row]].obj = (uint16_t)i;
            B.items[row][B.itemCount[row]].count = 1;
            B.itemCount[row]++;
        }
    }
}

static const int sRowY[ROW_COUNT] = {11, 26, 71, 102, 117};
static const int sRowH[ROW_COUNT] = {14, 30, 30, 14, 30};
#define TILE_W 24      // the narrowest a card gets (a full row); up to TILE_MAX when there's room
#define TILE_MAX 40
#define LAND_W 30
#define LAND_MAX 46
#define MID_Y 57
#define BAR_Y 148

static void Layout(void)
{
    int r, i;
    memset(B.rect, 0, sizeof(B.rect));
    for (r = 0; r < ROW_COUNT; r++) {
        int n = B.itemCount[r], land = r == ROW_MY_LANDS || r == ROW_OPP_LANDS;
        int w = n ? (236 - 2 * (n - 1)) / n : TILE_W, step, x;
        if (w > (land ? LAND_MAX : TILE_MAX))
            w = land ? LAND_MAX : TILE_MAX;
        if (w < (land ? LAND_W : TILE_W))
            w = land ? LAND_W : TILE_W;
        step = w + 2;
        if (n > 1 && n * step > 236)
            step = (236 - w) / (n - 1);   // (overlapping when the row is full)
        x = 2;
        for (i = 0; i < n; i++) {
            const MtgObj *o = &B.duel.objs[B.items[r][i].obj];
            Rect rc;
            rc.on = 1;
            rc.x = (int16_t)(x + i * step);
            rc.y = (int16_t)sRowY[r];
            rc.w = (int16_t)w;
            rc.h = (int16_t)sRowH[r];
            if (!land && r != ROW_HAND && o->tapped) {   // on its side
                rc.w = (int16_t)sRowH[r];
                rc.h = (int16_t)w;
                rc.y = (int16_t)(sRowY[r] + sRowH[r] - w);
            }
            B.rect[B.items[r][i].obj] = rc;
            if (land) {   // every land of a stack answers to its rect
                int k;
                for (k = 0; k < B.duel.objCount; k++)
                    if (B.duel.objs[k].zone == MTG_ZONE_BATTLEFIELD && B.duel.objs[k].def == o->def
                        && B.duel.objs[k].tapped == o->tapped && B.duel.objs[k].controller == o->controller)
                        B.rect[k] = rc;
            }
        }
    }
    B.playerRect[THEM].x = 0, B.playerRect[THEM].y = 0, B.playerRect[THEM].w = 240, B.playerRect[THEM].h = 10;
    B.playerRect[ME].x = 0, B.playerRect[ME].y = BAR_Y, B.playerRect[ME].w = 240, B.playerRect[ME].h = 12;
    B.playerRect[ME].on = B.playerRect[THEM].on = 1;
}

static int Focused(void)
{
    int r = B.row, c = B.col[r];
    if (B.itemCount[r] == 0)
        return MTG_NO_OBJ;
    if (c >= B.itemCount[r])
        c = B.col[r] = B.itemCount[r] - 1;
    return B.items[r][c].obj;
}

// ---- drawing
static unsigned BodyColor(int obj)
{
    const MtgCardDef *c = Def(obj);
    uint8_t colors = c->colors;
    if (c->types & MTG_T_LAND) {
        uint8_t m = c->intrinsicMana;
        return m & MTG_COLOR_BIT(MTG_W) ? 0xe8e0c0 : m & MTG_COLOR_BIT(MTG_U) ? 0x90b8e8
             : m & MTG_COLOR_BIT(MTG_B) ? 0x9890a0 : m & MTG_COLOR_BIT(MTG_R) ? 0xe89878
             : m & MTG_COLOR_BIT(MTG_G) ? 0x90c890 : 0xc8b090;
    }
    if (colors & (colors - 1))
        return 0xe0c060;   // more than one color: gold
    return colors & MTG_COLOR_BIT(MTG_W) ? 0xf0e8c8 : colors & MTG_COLOR_BIT(MTG_U) ? 0x70a0e0
         : colors & MTG_COLOR_BIT(MTG_B) ? 0x706878 : colors & MTG_COLOR_BIT(MTG_R) ? 0xe07858
         : colors & MTG_COLOR_BIT(MTG_G) ? 0x70b070 : 0xb8b8c0;
}

static int DarkBody(int obj) { return Def(obj)->colors == MTG_COLOR_BIT(MTG_B) && !IsLand(obj); }

// a name in up to two lines that fit w (words kept whole when they can be)
static void TwoLines(const char *name, int w, char *a, char *b, unsigned cap)
{
    const char *space;
    snprintf(a, cap, "%s", name);
    b[0] = 0;
    if (G_Measure(a, TF_NARROW) <= w)
        return;
    space = strchr(name, ' ');
    if (space) {
        snprintf(a, cap, "%.*s", (int)(space - name), name);
        snprintf(b, cap, "%s", space + 1);
    }
    TcgUi_FitText(a, w, TF_NARROW);
    TcgUi_FitText(b, w, TF_NARROW);
}

static void CostText(char *out, unsigned cap, const MtgManaCost *c)
{
    static const char sym[] = "WUBRGC";
    unsigned n = 0, i, k;
    out[0] = 0;
    if (c->x && n + 1 < cap)
        out[n++] = 'X';
    if (c->generic)
        n += (unsigned)snprintf(out + n, cap - n, "%u", c->generic);
    for (i = 0; i < MTG_MANA_KINDS; i++)
        for (k = 0; k < c->pip[i] && n + 1 < cap; k++)
            out[n++] = sym[i];
    out[n < cap ? n : cap - 1] = 0;
}

static void DrawTile(int obj, const Rect *rc, int count, int faceDown)
{
    const MtgObj *o = &B.duel.objs[obj];
    const MtgCardDef *c = Def(obj);
    unsigned body = faceDown ? 0x6a4a30 : BodyColor(obj), ink = DarkBody(obj) ? G_WHITE : G_INK;
    char name[64], a[40], b[40], pt[16];
    int x = rc->x, y = rc->y, w = rc->w, h = rc->h;
    G_FillA(x + 1, y + 1, w, h, 0x000000, 60);
    G_RoundFill(x, y, w, h, o->tapped && IsLand(obj) ? G_Mix(body, 0x404040, 0.45f) : body, 255);
    G_RoundFrame(x, y, w, h, faceDown ? 0x2a1a10 : G_Mix(body, 0x000000, 0.55f), 255);
    if (faceDown) {
        G_Circle(x + w / 2, y + h / 2, 6, 0xc89050, 255);
        return;
    }
    if (o->attacking)
        G_RoundFrame(x - 1, y - 1, w + 2, h + 2, 0xe83030, 255);
    if (o->blocking != MTG_NO_OBJ)
        G_RoundFrame(x - 1, y - 1, w + 2, h + 2, 0x3070e8, 255);
    Name(name, sizeof(name), obj);
    if (IsLand(obj)) {
        int cw = 0;
        if (count > 1) {
            snprintf(pt, sizeof(pt), "x%d", count);
            cw = G_DigitsWidth(pt, 0, 1) + 2;
            G_Digits(x + w - 1 - cw, y + h - 7, pt, 0, 1, 0x202020, 255);
        }
        snprintf(a, sizeof(a), "%s", name);
        TcgUi_FitText(a, w - 4 - (cw ? cw - 2 : 0), TF_NARROW);
        G_Text(x + 2, y + 1, ink, G_NO_SHADOW, a, 0, TF_NARROW);
        return;
    }
    TwoLines(name, w - 3, a, b, sizeof(a));
    G_Text(x + 2, y + 1, ink, G_NO_SHADOW, a, 0, TF_NARROW);
    if (b[0] && h >= 26)
        G_Text(x + 2, y + 9, ink, G_NO_SHADOW, b, 0, TF_NARROW);
    if (o->zone == MTG_ZONE_HAND && (w >= 34 || !(c->types & MTG_T_CREATURE))) {   // (else in the bottom bar)
        char cost[16];
        CostText(cost, sizeof(cost), &c->cost);
        G_Text(x + 2, y + h - 10, ink, G_NO_SHADOW, cost, 0, TF_NARROW);
    }
    if (c->types & MTG_T_CREATURE) {
        int power = o->zone == MTG_ZONE_BATTLEFIELD ? Mtg_Power(&B.duel, obj) : c->power;
        int tough = o->zone == MTG_ZONE_BATTLEFIELD ? Mtg_Toughness(&B.duel, obj) - o->damage : c->toughness;
        unsigned col = o->damage ? 0xc02020 : (o->tempPower || o->tempToughness ? 0x1060c0 : 0x202020);
        int pw;
        snprintf(pt, sizeof(pt), "%d/%d", power, tough);
        pw = G_DigitsWidth(pt, 0, 1);
        G_FillA(x + w - pw - 3, y + h - 8, pw + 2, 7, 0xffffff, 170);
        G_Digits(x + w - pw - 2, y + h - 7, pt, 0, 1, col, 255);
        if (o->zone == MTG_ZONE_BATTLEFIELD && o->sick && !(Mtg_Keywords(&B.duel, obj) & MTG_KW_BIT(MTG_KW_HASTE)))
            G_FillA(x + 1, y + 1, w - 2, h - 2, 0xffffff, 50);   // (can't attack yet)
        if (Mtg_Keywords(&B.duel, obj) & MTG_KW_BIT(MTG_KW_FLYING))
            G_Arrow(x + 2, y + h - 6, 0, 0x2050a0);              // a flier
    }
}

static const char *const sStepNames[MTG_STEP_COUNT] = {
    "UNTAP", "UPKEEP", "DRAW", "MAIN 1", "COMBAT", "ATTACK", "BLOCK", "FIRST STRIKE", "DAMAGE", "END COMBAT",
    "MAIN 2", "END", "CLEANUP"};

static int HandCount(int player)
{
    int i, n = 0;
    for (i = 0; i < B.duel.objCount; i++)
        n += B.duel.objs[i].zone == MTG_ZONE_HAND && B.duel.objs[i].owner == player;
    return n;
}

static void PlayerBar(int player)
{
    const Rect *r = &B.playerRect[player];
    char line[96];
    G_FillA(r->x, r->y, r->w, r->h, player == ME ? 0x203048 : 0x402028, 220);
    snprintf(line, sizeof(line), "LIFE %d  HAND %d  LIBRARY %d", B.duel.players[player].life, HandCount(player),
             B.duel.players[player].libraryCount);
    if (player == THEM) {
        G_Text(4, r->y + 1, G_WHITE, G_NO_SHADOW, B.opp, 0, TF_NARROW);
        G_TextAlign(236, r->y + 1, TA_RIGHT, G_WHITE, G_NO_SHADOW, line, TF_NARROW);
    } else {
        int f = Focused();
        if (f != MTG_NO_OBJ && B.mode != MODE_TARGET) {   // the card under the cursor: name, cost, power
            char name[64], text[96], cost[16];
            const MtgCardDef *c = Def(f);
            Name(name, sizeof(name), f);
            CostText(cost, sizeof(cost), &c->cost);
            if (c->types & MTG_T_CREATURE)
                snprintf(text, sizeof(text), "%s %s %d/%d", name, cost, c->power, c->toughness);
            else
                snprintf(text, sizeof(text), "%s %s", name, cost);
            TcgUi_FitText(text, 118, TF_NARROW);
            G_Text(4, r->y + 2, 0xf8e8a0, G_NO_SHADOW, text, 0, TF_NARROW);
        }
        G_TextAlign(236, r->y + 2, TA_RIGHT, G_WHITE, G_NO_SHADOW, line, TF_NARROW);
    }
}

static void StackLine(char *out, unsigned cap)
{
    const MtgStackEntry *e = &B.duel.stack[B.duel.stackCount - 1];
    char name[64], target[64] = "";
    Name(name, sizeof(name), e->source);
    if (e->target.kind == MTG_TGT_OBJ || e->target.kind == MTG_TGT_STACK) {
        char t[64];
        Name(t, sizeof(t), e->target.id);
        snprintf(target, sizeof(target), " > %s", t);
    } else if (e->target.kind == MTG_TGT_PLAYER)
        snprintf(target, sizeof(target), " > %s", Who(e->target.id));
    snprintf(out, cap, "%s: %s%s", e->controller == ME ? "YOU" : B.opp, name, target);
}

static const char *Prompt(void)
{
    switch (B.mode) {
    case MODE_TARGET: return "Choose a target: {A} OK  {B} BACK";
    case MODE_ATTACK: return "Attack with? {A} PICK  {START} DONE";
    case MODE_BLOCK: return "Block with? {A} PICK  {START} DONE";
    case MODE_BLOCK_PICK: return "Block which attacker? {A} OK  {B} BACK";
    case MODE_DISCARD: return "Too many cards: {A} DISCARD ONE";
    }
    return NULL;
}

static void MiddleLine(void)
{
    char line[120];
    const char *prompt = Prompt();
    G_FillA(0, MID_Y, 240, 13, 0x102030, 210);
    snprintf(line, sizeof(line), "%s %s", B.duel.active == ME ? "YOUR" : "THEIR", sStepNames[B.duel.step]);
    G_Text(3, MID_Y + 2, B.duel.active == ME ? 0xa0d0ff : 0xffb0a0, G_NO_SHADOW, line, 0, TF_NARROW);
    if (prompt)
        G_Hint(62, MID_Y + 6, 0xf8f070, G_NO_SHADOW, prompt, TF_NARROW, 1);
    else if (B.holdAge < 70 && B.hold[0])
        G_Text(62, MID_Y + 2, 0xf8f070, G_NO_SHADOW, B.hold, 0, TF_NARROW);
    else if (B.duel.stackCount) {
        StackLine(line, sizeof(line));
        TcgUi_FitText(line, 172, TF_NARROW);
        G_Text(62, MID_Y + 2, 0xffffff, G_NO_SHADOW, line, 0, TF_NARROW);
    } else if (B.tickerCount) {
        snprintf(line, sizeof(line), "%s", B.ticker[0]);
        TcgUi_FitText(line, 172, TF_NARROW);
        G_Text(62, MID_Y + 2, 0xe0e0e0, G_NO_SHADOW, line, 0, TF_NARROW);
    } else if (B.duel.query.kind == MTG_Q_PRIORITY && B.duel.query.player == ME)
        G_Hint(62, MID_Y + 6, 0xc0c8d0, G_NO_SHADOW, "{A} PLAY  {START} PASS  {L} STOP", TF_NARROW, 1);
}

static void InspectWindow(int obj)
{
    const MtgCardDef *c = Def(obj);
    char name[64], text[700], line[96], cost[16];
    int y;
    G_FillA(0, 0, 240, 160, 0x000000, 120);
    G_Window(16, 10, 208, 140);
    Name(name, sizeof(name), obj);
    G_Text(24, 15, G_INK, G_INK_SHADOW, name, 150, TF_NORMAL);
    CostText(cost, sizeof(cost), &c->cost);
    if (!(c->types & MTG_T_LAND))
        G_TextAlign(216, 16, TA_RIGHT, G_INK, G_INK_SHADOW, cost, TF_NORMAL);
    {
        static const char *const sTypes[] = {"Land", "Creature", "Artifact", "Enchantment", "Instant", "Sorcery",
                                             "Planeswalker", "Tribal"};
        unsigned n = 0, i;
        line[0] = 0;
        if (c->supertypes & MTG_ST_LEGENDARY)
            n += (unsigned)snprintf(line + n, sizeof(line) - n, "Legendary ");
        if (c->supertypes & MTG_ST_BASIC)
            n += (unsigned)snprintf(line + n, sizeof(line) - n, "Basic ");
        for (i = 0; i < 8; i++)
            if (c->types & (1u << i))
                n += (unsigned)snprintf(line + n, sizeof(line) - n, "%s ", sTypes[i]);
        if (c->subtypes[0] && n < sizeof(line))
            snprintf(line + n, sizeof(line) - n, "- %s", c->subtypes);
    }
    Mtg_PlainText(text, sizeof(text), line);
    G_Text(24, 31, 0x506070, G_NO_SHADOW, text, 192, TF_NARROW);
    Mtg_PlainText(text, sizeof(text), c->oracle);
    y = G_Text(24, 43, G_INK, G_NO_SHADOW, text, 192, TF_NARROW);
    (void)y;
    if (c->types & MTG_T_CREATURE) {
        if (B.duel.objs[obj].zone == MTG_ZONE_BATTLEFIELD)
            snprintf(line, sizeof(line), "%d/%d (printed %d/%d)", Mtg_Power(&B.duel, obj), Mtg_Toughness(&B.duel, obj),
                     c->power, c->toughness);
        else
            snprintf(line, sizeof(line), "%d/%d", c->power, c->toughness);
        G_TextAlign(216, 136, TA_RIGHT, G_INK, G_INK_SHADOW, line, TF_NORMAL);
    }
    G_Hint(24, 141, 0x606870, G_NO_SHADOW, "{B} CLOSE", TF_NARROW, 1);
}

static int sMenuLabelsInit;
static char sMenuLabels[8][48];

static void DrawMenu(void)
{
    const char *items[8];
    int i;
    for (i = 0; i < B.menuCount; i++)
        items[i] = sMenuLabels[i];
    TcgUi_PopMenu(70, 60, 160, items, B.menuCount, B.menuCursor, 0);
}

static void DrawResult(void)
{
    const char *reason = B.duel.players[B.result == 1 ? THEM : ME].drewFromEmpty ? "No cards left to draw."
                       : B.forced ? "" : "Life down to 0.";
    if (B.result == 3)
        reason = "Neither of you won.";
    G_ResultScreen(B.result == 1, B.resultAge, B.frame, B.opp, reason,
                   B.result == 1 ? "A booster pack for the win!" : B.result == 2 ? "Try again any time." : "");
}

void MtgBoard_Draw(void)
{
    int r, i, f;
    if (!B.active)
        return;
    BuildRows();
    Layout();
    G_Fill(0, 0, 240, 160, 0x2a4a3a);   // the felt
    G_FillA(0, MID_Y - 1, 240, 1, 0x000000, 80);
    PlayerBar(THEM);
    for (r = 0; r < ROW_COUNT; r++)
        for (i = 0; i < B.itemCount[r]; i++) {
            int obj = B.items[r][i].obj;
            DrawTile(obj, &B.rect[obj], B.items[r][i].count, 0);
        }
    // the attackers each blocker blocks
    for (i = 0; i < B.duel.objCount; i++) {
        const MtgObj *o = &B.duel.objs[i];
        if (o->zone == MTG_ZONE_BATTLEFIELD && o->blocking != MTG_NO_OBJ && B.rect[i].on && B.rect[o->blocking].on) {
            const Rect *a = &B.rect[i], *t = &B.rect[o->blocking];
            G_Line(a->x + a->w / 2, a->y + a->h / 2, t->x + t->w / 2, t->y + t->h / 2, 0x80c0ff, 220);
        }
    }
    if (B.mode == MODE_BLOCK || B.mode == MODE_BLOCK_PICK) {   // the player's blocks chosen so far
        const MtgQuery *q = &B.duel.query;
        for (i = 0; i < q->candidateCount; i++)
            if (B.block[i] >= 0) {
                const Rect *a = &B.rect[q->candidates[i]], *t = &B.rect[q->attackers[(int)B.block[i]]];
                G_Line(a->x + a->w / 2, a->y + a->h / 2, t->x + t->w / 2, t->y + t->h / 2, 0x80c0ff, 220);
                G_RoundFrame(a->x - 1, a->y - 1, a->w + 2, a->h + 2, 0x3070e8, 255);
            }
    }
    if (B.mode == MODE_ATTACK) {
        const MtgQuery *q = &B.duel.query;
        for (i = 0; i < q->candidateCount; i++) {
            const Rect *a = &B.rect[q->candidates[i]];
            if (B.attack & (1u << i))
                G_RoundFrame(a->x - 1, a->y - 1, a->w + 2, a->h + 2, 0xe83030, 255);
            else
                G_FillA(a->x, a->y + a->h - 2, a->w, 2, 0xf8f070, 200);   // (could attack)
        }
    }
    MiddleLine();
    PlayerBar(ME);
    // the cursor
    if (B.mode == MODE_TARGET || B.mode == MODE_BLOCK_PICK) {
        const Rect *t = NULL;
        if (B.mode == MODE_TARGET) {
            const MtgAction *a = &B.duel.query.actions[B.targets[B.targetCursor]];
            t = a->target.kind == MTG_TGT_PLAYER ? &B.playerRect[a->target.id] : &B.rect[a->target.id];
            if (a->target.kind == MTG_TGT_STACK) {
                static Rect stack = {60, MID_Y, 176, 13, 1};
                t = &stack;
            }
        } else
            t = &B.rect[B.duel.query.attackers[B.blockCursor]];
        if (t && t->on) {
            if (t == &B.playerRect[ME] || t == &B.playerRect[THEM])   // a player: their whole bar lights up
                G_FillA(t->x, t->y, t->w, t->h, G_FOCUS_TARGET, (B.frame / 8) % 2 ? 90 : 60);
            G_Focus(t->x, t->y, t->w, t->h, G_FOCUS_TARGET, B.frame);
        }
        if (B.mode == MODE_TARGET) {   // what the cursor is on, named in the bottom bar
            const MtgAction *a = &B.duel.query.actions[B.targets[B.targetCursor]];
            char name[64] = "", line[96];
            if (a->target.kind == MTG_TGT_PLAYER)
                snprintf(name, sizeof(name), "%s", a->target.id == ME ? "YOU" : B.opp);
            else
                Name(name, sizeof(name), a->target.id);
            snprintf(line, sizeof(line), "TARGET: %s (%d/%d)", name, B.targetCursor + 1, B.targetCount);
            TcgUi_FitText(line, 120, TF_NARROW);
            G_Text(4, BAR_Y + 2, G_FOCUS_TARGET, G_NO_SHADOW, line, 0, TF_NARROW);
        }
    } else if (B.mode != MODE_RESULT && (f = Focused()) != MTG_NO_OBJ)
        G_Focus(B.rect[f].x, B.rect[f].y, B.rect[f].w, B.rect[f].h, G_WHITE, B.frame);
    if (B.mode == MODE_MENU)
        DrawMenu();
    if (B.inspect && B.inspectObj != MTG_NO_OBJ)
        InspectWindow(B.inspectObj);
    if (B.mode == MODE_RESULT)
        DrawResult();
}

// ---- what happened, in words
static void ReadEvents(void)
{
    const MtgDuel *d = &B.duel;
    for (; B.eventSeen < d->eventCount; B.eventSeen++) {
        const MtgEvent *e = Mtg_EventAt(d, B.eventSeen);
        char a[64], b[64];
        if (!e)
            continue;
        switch (e->kind) {
        case MTG_EV_TURN:
            Ticker("Turn %d: %s.", e->b, e->a == ME ? "your turn" : "their turn");
            break;
        case MTG_EV_CAST:
        case MTG_EV_ACTIVATE:
            Name(a, sizeof(a), e->a);
            if (e->b == MTG_TGT_OBJ || e->b == MTG_TGT_STACK) {
                Name(b, sizeof(b), e->c);
                Ticker("%s %s %s on %s.", Who(d->objs[e->a].controller), e->kind == MTG_EV_CAST ? "cast" : "used", a, b);
            } else if (e->b == MTG_TGT_PLAYER)
                Ticker("%s %s %s on %s.", Who(d->objs[e->a].controller), e->kind == MTG_EV_CAST ? "cast" : "used", a,
                       e->c == ME ? "you" : B.opp);
            else
                Ticker("%s %s %s.", Who(d->objs[e->a].controller), e->kind == MTG_EV_CAST ? "cast" : "used", a);
            break;
        case MTG_EV_LAND:
            if (e->b == THEM) {
                Name(a, sizeof(a), e->a);
                Ticker("%s played %s.", B.opp, a);
            }
            break;
        case MTG_EV_ZONE:
            if (e->b == MTG_ZONE_BATTLEFIELD && e->c != MTG_ZONE_BATTLEFIELD) {
                Name(a, sizeof(a), e->a);
                Ticker("%s %s.", a, e->c == MTG_ZONE_GRAVEYARD ? ((Mtg_Card(d->objs[e->a].def)->types & MTG_T_CREATURE)
                                                                      ? "died" : "was destroyed")
                                    : e->c == MTG_ZONE_EXILE ? "was exiled" : e->c == MTG_ZONE_HAND ? "went back to a hand"
                                    : "left");
            }
            break;
        case MTG_EV_BLOCK:
            Name(a, sizeof(a), e->a);
            Name(b, sizeof(b), e->b);
            Ticker("%s blocks %s.", a, b);
            break;
        case MTG_EV_COUNTERED:
            Name(a, sizeof(a), e->a);
            Ticker("%s was countered.", a);
            break;
        case MTG_EV_FIZZLE:
            Name(a, sizeof(a), e->a);
            Ticker("%s lost its target.", a);
            break;
        case MTG_EV_DAMAGE:
            if (e->a != MTG_NO_OBJ && e->b < 0 && e->c >= 3) {   // (big hits to a player)
                Name(a, sizeof(a), e->a);
                Ticker("%s: %d damage to %s!", a, e->c, e->b == -1 - ME ? "you" : B.opp);
            }
            break;
        }
    }
}

// ---- answering
static void Answer(const MtgAnswer *a)
{
    if (!Mtg_Answer(&B.duel, a))
        Hold("That can't be done.");
    B.mode = MODE_NORMAL;
}

static void AnswerAction(int index)
{
    MtgAnswer a;
    const MtgAction *act = &B.duel.query.actions[index];
    if (act->kind != MTG_ACT_PASS) {   // (the log: what the player did, for the test harness)
        char name[64], target[64] = "";
        Name(name, sizeof(name), act->obj);
        if (act->target.kind == MTG_TGT_PLAYER)
            snprintf(target, sizeof(target), " on %s", Who(act->target.id));
        else if (act->target.kind != MTG_TGT_NONE) {
            Name(target + 4, sizeof(target) - 4, act->target.id);
            memcpy(target, " on ", 4);
        }
        MPNet_Log("mtg: you: %s %s%s", act->kind == MTG_ACT_LAND ? "played" : act->kind == MTG_ACT_CAST ? "cast"
                                       : "used", name, target);
    }
    memset(&a, 0, sizeof(a));
    a.action = (uint16_t)index;
    Answer(&a);
}

void MtgBoard_DebugFocus(const char *name)   // "mp mtg focus NAME": the cursor on that card (the test harness)
{
    int r, i;
    BuildRows();
    for (r = ROW_COUNT - 1; r >= 0; r--)
        for (i = 0; i < B.itemCount[r]; i++)
            if (strcmp(Def(B.items[r][i].obj)->name, name) == 0) {
                B.row = r;
                B.col[r] = i;
                MPNet_Log("mtg: focus %s", name);
                return;
            }
    MPNet_Log("mtg: no %s to focus", name);
}

void MtgBoard_DebugState(void)   // "mp mtg table"
{
    MPNet_Log("mtg: table: turn %d %s %s, life you %d them %d, stack %d", B.duel.turn, B.duel.active == ME ? "yours" : "theirs",
              sStepNames[B.duel.step], B.duel.players[ME].life, B.duel.players[THEM].life, B.duel.stackCount);
}

static void YourMove(const char *what)   // (the log, once a decision: the test harness waits for it)
{
    static uint32_t sLast;
    uint32_t key = (uint32_t)B.duel.turn << 16 | (uint32_t)B.duel.step << 8 | B.duel.query.kind;
    if (key != sLast) {
        sLast = key;
        MPNet_Log("mtg: your move: %s, turn %d, %s", what, B.duel.turn, sStepNames[B.duel.step]);
    }
}

static void Pass(void) { AnswerAction(0); }

// the actions of the focused card (a stack of lands: any of them), one per kind and ability
static int GatherMenu(int focused)
{
    const MtgQuery *q = &B.duel.query;
    int i, k, n = 0;
    const MtgObj *fo = &B.duel.objs[focused];
    for (i = 1; i < q->actionCount; i++) {
        const MtgAction *a = &q->actions[i];
        const MtgObj *o = &B.duel.objs[a->obj];
        int same = a->obj == focused || (IsLand(focused) && o->zone == fo->zone && o->def == fo->def
                                         && o->tapped == fo->tapped && o->controller == fo->controller);
        int dup = 0;
        if (!same)
            continue;
        for (k = 0; k < n; k++) {
            const MtgAction *b = &q->actions[B.menu[k]];
            if (b->kind == a->kind && b->ability == a->ability && (b->obj == a->obj || IsLand(focused)))
                dup = 1;
        }
        if (!dup && n < 8)
            B.menu[n++] = (uint16_t)i;
    }
    return n;
}

static void MenuLabel(int k, char *out, unsigned cap)
{
    const MtgAction *a = &B.duel.query.actions[B.menu[k]];
    if (a->kind == MTG_ACT_LAND)
        snprintf(out, cap, "PLAY THE LAND");
    else if (a->kind == MTG_ACT_CAST)
        snprintf(out, cap, a->x ? "CAST (X = %d)" : "CAST", a->x);
    else {
        const char *desc = Mtg_Param(&gMtgAbilities[a->ability], "SpellDescription");
        char text[96];
        const MtgCost *cost = &gMtgAbilities[a->ability].cost;
        char c[16];
        CostText(c, sizeof(c), &cost->mana);
        Mtg_PlainText(text, sizeof(text), desc ? desc : "USE");
        snprintf(out, cap, "%s%s%s%s", c, cost->tap ? (c[0] ? ",T: " : "T: ") : (c[0] ? ": " : ""), "", text);
        TcgUi_FitText(out, 136, TF_NORMAL);
    }
}

static void ChooseKind(int k)
{
    const MtgQuery *q = &B.duel.query;
    const MtgAction *chosen = &q->actions[B.menu[k]];
    int i;
    B.targetCount = 0;
    for (i = 1; i < q->actionCount; i++) {
        const MtgAction *a = &q->actions[i];
        if (a->kind == chosen->kind && a->ability == chosen->ability && a->obj == chosen->obj)
            B.targets[B.targetCount++] = (uint16_t)i;
    }
    if (B.targetCount == 1 && chosen->target.kind == MTG_TGT_NONE) {
        PlaySE(SE_SELECT);
        AnswerAction(B.targets[0]);
        return;
    }
    B.targetCursor = 0;
    // start on an opponent's target when there is one
    for (i = 0; i < B.targetCount; i++) {
        const MtgTarget *t = &q->actions[B.targets[i]].target;
        if ((t->kind == MTG_TGT_PLAYER && t->id == THEM) || (t->kind == MTG_TGT_OBJ && B.duel.objs[t->id].controller == THEM)) {
            B.targetCursor = i;
            break;
        }
    }
    B.mode = MODE_TARGET;
    PlaySE(SE_SELECT);
}

static void MoveCursor(unsigned pressed, int onlyRow)
{
    int r = B.row;
    if (onlyRow < 0 && (pressed & (KEY_UP | KEY_DOWN))) {
        int dir = pressed & KEY_UP ? -1 : 1, n;
        for (n = 0, r = B.row + dir; n < ROW_COUNT && (r < 0 || r >= ROW_COUNT || !B.itemCount[r]); n++)
            r = (r + dir + ROW_COUNT) % ROW_COUNT;
        if (r >= 0 && r < ROW_COUNT && B.itemCount[r] && r != B.row) {
            B.row = r;
            PlaySE(SE_SELECT);
        }
    }
    if (onlyRow >= 0)
        B.row = onlyRow;
    if (pressed & (KEY_LEFT | KEY_RIGHT)) {
        int n = B.itemCount[B.row];
        if (n) {
            B.col[B.row] = (B.col[B.row] + (pressed & KEY_LEFT ? n - 1 : 1)) % n;
            PlaySE(SE_SELECT);
        }
    }
}

// does the table pass for the player now? (nothing worth stopping for: the stronger auto-pass the decisions ask for)
static int AutoPass(unsigned keys)
{
    const MtgDuel *d = &B.duel;
    const MtgQuery *q = &d->query;
    if (q->actionCount <= 1)
        return 1;
    if (keys & KEY_L)
        return 0;
    if (d->stackCount)
        return d->stack[d->stackCount - 1].controller == ME;   // the opponent's spell: a chance to answer it
    if (d->active == ME)
        return !(d->step == MTG_STEP_MAIN1 || d->step == MTG_STEP_MAIN2 || d->step == MTG_STEP_COMBAT_BLOCKERS);
    return !(d->step == MTG_STEP_COMBAT_BLOCKERS || d->step == MTG_STEP_END);
}

static int ObjIndex(const uint16_t *list, int n, int obj)
{
    int i;
    for (i = 0; i < n; i++)
        if (list[i] == obj)
            return i;
    return -1;
}

static void HumanPriority(unsigned pressed, unsigned keys)
{
    int f;
    if (B.mode == MODE_MENU) {
        if (pressed & KEY_UP)
            B.menuCursor = (B.menuCursor + B.menuCount - 1) % B.menuCount, PlaySE(SE_SELECT);
        if (pressed & KEY_DOWN)
            B.menuCursor = (B.menuCursor + 1) % B.menuCount, PlaySE(SE_SELECT);
        if (pressed & KEY_B)
            B.mode = MODE_NORMAL, PlaySE(SE_SELECT);
        else if (pressed & KEY_A) {
            B.mode = MODE_NORMAL;
            ChooseKind(B.menuCursor);
        }
        return;
    }
    if (B.mode == MODE_TARGET) {
        if (pressed & (KEY_LEFT | KEY_UP))
            B.targetCursor = (B.targetCursor + B.targetCount - 1) % B.targetCount, PlaySE(SE_SELECT);
        if (pressed & (KEY_RIGHT | KEY_DOWN))
            B.targetCursor = (B.targetCursor + 1) % B.targetCount, PlaySE(SE_SELECT);
        if (pressed & KEY_B)
            B.mode = MODE_NORMAL, PlaySE(SE_SELECT);
        else if (pressed & KEY_A) {
            PlaySE(SE_SELECT);
            AnswerAction(B.targets[B.targetCursor]);
        }
        return;
    }
    if (!(keys & KEY_L) && AutoPass(keys)) {
        if (++B.autoPassWait >= 3) {
            B.autoPassWait = 0;
            Pass();
        }
        return;
    }
    B.autoPassWait = 0;
    YourMove("priority");
    MoveCursor(pressed, -1);
    if (pressed & KEY_START) {
        PlaySE(SE_SELECT);
        Pass();
        return;
    }
    if (pressed & KEY_A) {
        f = Focused();
        if (f == MTG_NO_OBJ)
            return;
        B.menuCount = GatherMenu(f);
        if (!B.menuCount) {
            PlaySE(SE_FAILURE);
            Hold("Nothing to do with it now.");
            return;
        }
        if (B.menuCount == 1) {
            ChooseKind(0);
            return;
        }
        for (f = 0; f < B.menuCount; f++)
            MenuLabel(f, sMenuLabels[f], sizeof(sMenuLabels[f]));
        sMenuLabelsInit = 1;
        B.menuCursor = 0;
        B.mode = MODE_MENU;
        PlaySE(SE_SELECT);
    }
}

static void HumanAttack(unsigned pressed)
{
    const MtgQuery *q = &B.duel.query;
    int f, i;
    YourMove("attackers");
    if (B.mode != MODE_ATTACK) {
        B.mode = MODE_ATTACK;
        B.attack = 0;
        B.row = ROW_MY_PERMS;
        for (i = 0; i < B.itemCount[ROW_MY_PERMS]; i++)   // the cursor on the first who can attack
            if (ObjIndex(q->candidates, q->candidateCount, B.items[ROW_MY_PERMS][i].obj) >= 0) {
                B.col[ROW_MY_PERMS] = i;
                break;
            }
    }
    MoveCursor(pressed, ROW_MY_PERMS);
    f = Focused();
    if ((pressed & KEY_A) && f != MTG_NO_OBJ) {
        i = ObjIndex(q->candidates, q->candidateCount, f);
        if (i < 0) {
            PlaySE(SE_FAILURE);
            Hold("It can't attack.");
        } else {
            B.attack ^= 1u << i;
            PlaySE(SE_SELECT);
        }
    }
    if (pressed & KEY_SELECT) {   // everyone, or no one
        uint32_t all = q->candidateCount >= 32 ? 0xFFFFFFFFu : (1u << q->candidateCount) - 1;
        B.attack = B.attack == all ? 0 : all;
        PlaySE(SE_SELECT);
    }
    if (pressed & KEY_START) {
        MtgAnswer a;
        MPNet_Log("mtg: you: attack with %d", __builtin_popcount(B.attack));
        memset(&a, 0, sizeof(a));
        a.attack = B.attack;
        PlaySE(SE_SELECT);
        Answer(&a);
    }
}

static void HumanBlock(unsigned pressed)
{
    const MtgQuery *q = &B.duel.query;
    int f, i;
    YourMove("blockers");
    if (B.mode != MODE_BLOCK && B.mode != MODE_BLOCK_PICK) {
        B.mode = MODE_BLOCK;
        memset(B.block, -1, sizeof(B.block));
        B.row = ROW_MY_PERMS;
    }
    if (B.mode == MODE_BLOCK_PICK) {
        if (pressed & (KEY_LEFT | KEY_UP))
            B.blockCursor = (B.blockCursor + q->attackerCount - 1) % q->attackerCount, PlaySE(SE_SELECT);
        if (pressed & (KEY_RIGHT | KEY_DOWN))
            B.blockCursor = (B.blockCursor + 1) % q->attackerCount, PlaySE(SE_SELECT);
        if (pressed & KEY_B)
            B.mode = MODE_BLOCK, PlaySE(SE_SELECT);
        else if (pressed & KEY_A) {
            if (!Mtg_CanBlock(&B.duel, q->candidates[B.blockFrom], q->attackers[B.blockCursor])) {
                PlaySE(SE_FAILURE);
                Hold("It can't block that one.");
            } else {
                B.block[B.blockFrom] = (int8_t)B.blockCursor;
                B.mode = MODE_BLOCK;
                PlaySE(SE_SELECT);
            }
        }
        return;
    }
    MoveCursor(pressed, ROW_MY_PERMS);
    f = Focused();
    if ((pressed & KEY_A) && f != MTG_NO_OBJ) {
        i = ObjIndex(q->candidates, q->candidateCount, f);
        if (i < 0) {
            PlaySE(SE_FAILURE);
            Hold("It can't block.");
        } else if (B.block[i] >= 0) {
            B.block[i] = -1;
            PlaySE(SE_SELECT);
        } else {
            B.blockFrom = i;
            B.blockCursor = 0;
            B.mode = MODE_BLOCK_PICK;
            PlaySE(SE_SELECT);
        }
    }
    if (pressed & KEY_START) {
        MtgAnswer a;
        memset(&a, 0, sizeof(a));
        memcpy(a.block, B.block, sizeof(a.block));
        PlaySE(SE_SELECT);
        Answer(&a);
    }
}

static void HumanDiscard(unsigned pressed)
{
    const MtgQuery *q = &B.duel.query;
    int f, i;
    B.mode = MODE_DISCARD;
    MoveCursor(pressed, ROW_HAND);
    f = Focused();
    if ((pressed & KEY_A) && f != MTG_NO_OBJ && (i = ObjIndex(q->candidates, q->candidateCount, f)) >= 0) {
        MtgAnswer a;
        memset(&a, 0, sizeof(a));
        a.card = (uint8_t)i;
        PlaySE(SE_SELECT);
        Answer(&a);
    }
}

int MtgBoard_Step(unsigned pressed, unsigned keys)
{
    const MtgQuery *q;
    if (!B.active)
        return 0;
    B.frame++;
    B.holdAge++;
    BuildRows();
    Layout();
    ReadEvents();
    if (B.tickerCount && ++B.tickerAge >= TICKER_FRAMES) {   // the next line of what happened
        memmove(B.ticker[0], B.ticker[1], sizeof(B.ticker[0]) * (TICKER - 1));
        B.tickerCount--;
        B.tickerAge = 0;
    }
    q = Mtg_Query(&B.duel);
    if (B.mode == MODE_RESULT) {
        B.resultAge++;
        if (B.resultAge >= G_RESULT_READY && (pressed & (KEY_A | KEY_B)))
            return B.result;
        return 0;
    }
    if (q->kind == MTG_Q_GAME_OVER) {
        B.result = q->winner == ME ? 1 : q->winner == THEM ? 2 : 3;
        B.mode = MODE_RESULT;
        B.inspect = 0;
        B.resultAge = 0;
        if (B.result == 1)
            PlayBGM(MUS_VICTORY_TRAINER);
        MPNet_Log("mtg: duel over: %s (turn %d, life %d to %d)", B.result == 1 ? "won" : B.result == 2 ? "lost" : "a draw",
                  B.duel.turn, B.duel.players[ME].life, B.duel.players[THEM].life);
        return 0;
    }
    // a card's whole text
    if (B.inspect) {
        if (pressed & (KEY_B | KEY_INSPECT | KEY_SELECT))
            B.inspect = 0, PlaySE(SE_SELECT);
        return 0;
    }
    if ((pressed & (KEY_INSPECT | KEY_SELECT)) && B.mode != MODE_ATTACK) {
        int f = Focused();
        if (B.mode == MODE_TARGET) {
            const MtgAction *a = &q->actions[B.targets[B.targetCursor]];
            f = a->target.kind == MTG_TGT_OBJ || a->target.kind == MTG_TGT_STACK ? a->target.id : MTG_NO_OBJ;
        }
        if (f != MTG_NO_OBJ) {
            B.inspect = 1;
            B.inspectObj = f;
            PlaySE(SE_SELECT);
            return 0;
        }
    }
    if (q->player == THEM || B.autoplay) {
        // the computer: a moment between its moves, more while what it did is still being told
        if (B.aiWait > 0) {
            B.aiWait--;
            return 0;
        }
        if (B.tickerCount > 1 && !B.autoplay)
            return 0;
        {
            MtgAnswer a;
            int before = (int)B.duel.eventCount;
            MtgAi_Answer(&B.duel, &a);
            Mtg_Answer(&B.duel, &a);
            B.aiWait = B.autoplay ? 0 : ((int)B.duel.eventCount - before > 2 ? 24 : 2);
        }
        return 0;
    }
    switch (q->kind) {
    case MTG_Q_PRIORITY:
        HumanPriority(pressed, keys);
        break;
    case MTG_Q_ATTACKERS:
        HumanAttack(pressed);
        break;
    case MTG_Q_BLOCKERS:
        HumanBlock(pressed);
        break;
    case MTG_Q_DISCARD:
        HumanDiscard(pressed);
        break;
    }
    return 0;
}
