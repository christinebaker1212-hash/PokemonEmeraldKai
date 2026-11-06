// Moemon Kai: the Magic duel table, laid out as the POKeMON card table is (tcg_board.c): a cloth mat taller than the
// screen, framed rows for each player's lands and creatures, the piles in the side columns (graveyard, library), the
// opponent's HAND under theirs, the life of each in a plate like the HP plates, power / toughness plates under the
// creatures, the hand on the blue tray at the bottom (the mat slides up while the cursor is in the hand), tooltips
// for the card under the cursor, prompts in the yellow-framed boxes, menus in the game's window beside the card,
// toasts. Cards are faces with their art (mtg_gfx.c): a tapped card lies on its side, alike lands stack "x3", the
// opponent's cards face them (turned half round); the stack's cards sit in the middle with a line to their targets; YOUR TURN / THEIR TURN sweeps through; the
// opponent's spells show big as they're cast; cards slide to their places. It follows the engine (mtg_game.c) by
// its queries and events.
// Buttons: the d-pad moves over the cards (past a row's end: the piles), A plays or uses the card under the cursor (a
// menu when it can do more than one thing, then the cursor goes round the legal targets), on your library CONCEDE,
// on a graveyard LOOK; START passes, B the turn's menu (PASS / END TURN); INSPECT (R2 / D) or SELECT shows a card's
// whole text; L held stops at every chance to act (else the table passes for you when nothing is worth doing).
#include <SDL2/SDL.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "sound.h"
#include "constants/songs.h"
#include "mp.h"
#include "mp_kaisong_scene.h"
#include "tcg/tcg.h"
#include "tcg/integration.h"
#include "mtg/integration.h"
#include "mtg_internal.h"
#include "mtg_board.h"
#include "mtg_gfx.h"
#include "../tcg/tcg_foil.h"
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
enum { MODE_NORMAL, MODE_MENU, MODE_TARGET, MODE_ATTACK, MODE_BLOCK, MODE_BLOCK_PICK, MODE_DISCARD, MODE_RESULT,
       MODE_TURN, MODE_CONCEDE, MODE_VIEW,
       MODE_CHOOSE,     // a yes / no or pay / don't while something resolves (MTG_Q_CHOOSE)
       MODE_PICK };     // a permanent to sacrifice (MTG_Q_CHOOSE's PICK on the table)
// the piles the cursor can rest on (past a row's end)
enum { PILE_NONE = -1, PILE_MY_GRAVE, PILE_MY_LIBRARY, PILE_OPP_LIBRARY, PILE_OPP_GRAVE };

#define ME 0
#define THEM 1
#define ROW_ITEMS 40
#define TICKER 6
#define TICKER_FRAMES 50
#define SWEEP_FRAMES (Tcg_GetCollection()->fastAnimations ? 34 : 52)   // (DUEL SPEED: FAST)
#define BIG_FRAMES (Tcg_GetCollection()->fastAnimations ? 30 : 50)
#define TOAST_FRAMES 90
#define FLOATERS 8

typedef struct { int16_t x, y, w, h; uint8_t on; } Rect;
typedef struct { uint16_t obj; uint8_t count; } Item;

static struct
{
    int active;
    MtgDuel duel;                   // the truth: the rules' own duel
    MtgDuel view;                   // what the table shows: the truth as of the last event played out (Play)
    char opp[24];
    unsigned frame;
    int mode, row, col[ROW_COUNT], pile;
    int aiWait, autoPassWait, autoplay, endTurn;
    // what the player is choosing
    uint16_t menu[8];               // MODE_MENU: action indexes, one per (kind, ability)
    int menuCount, menuCursor;
    uint16_t targets[MTG_MAX_ACTIONS];   // MODE_TARGET: action indexes
    int targetCount, targetCursor;
    uint32_t attack;
    int8_t block[MTG_MAX_COMBAT];
    int blockFrom, blockCursor;
    int inspect, inspectObj;
    int trigTarget;                 // MODE_TARGET for a triggered ability: it must have one (no B)
    int still;                      // frames the table has been still with the player to act (the cursor and its
                                    // tooltip show only then: never flickering through the opponent's turn)
    int viewPlayer, viewCursor;     // MODE_VIEW: a graveyard
    uint16_t viewList[MTG_MAX_OBJS];
    int viewCount;
    // the layout of this frame
    Item items[ROW_COUNT][ROW_ITEMS];
    int itemCount[ROW_COUNT];
    Rect rect[MTG_MAX_OBJS];
    Rect playerRect[2];
    float tray;                     // the hand's tray: 0 down, 1 up (the mat slides up with it)
    // the tray by context, as on the POKeMON table (Refocus): wanted up while the cursor is in the hand; a drawn card
    // shows the hand a moment; at a turn's start, after a card is played, when an answer is in the hand...
    int trayWanted, showHand, refocusTurn, refocusHand, refocusCol, answerFocused;
    // what happened
    char ticker[TICKER][96];
    int tickerCount, tickerAge;
    // the play (Play): the event under way and its looks
    uint32_t evShown;               // events of B.duel played out on B.view
    MtgEvent ev;
    int evOn, evLeft, wasBusy;
    unsigned pendingKeys;
    int pendingAge;
    Rect spawn[MTG_MAX_OBJS];       // where a card that shows up comes from
    uint8_t tapT[MTG_MAX_OBJS], lunge[MTG_MAX_OBJS], flash[MTG_MAX_OBJS];
    Rect castFrom, leaveRect, flyFrom, flyTo;
    int castT, castLife, resolveT, resolveLife, flyT, flyLife, flyBall, lifeFlash[2];
    unsigned leaveColor, flyColor;
    int16_t floatX[FLOATERS], floatY[FLOATERS], floatValue[FLOATERS];
    uint8_t floatAge[FLOATERS];
    unsigned floatColor[FLOATERS];
    char hold[96];                  // a note for the player ("nothing to do with it now"), shown as a toast
    int holdAge;
    int result, resultAge, forced;
    u16 music, mapMusic;
    // the look: cards sliding to their places, the turn's sweep, the opponent's spell shown big
    Rect last[MTG_MAX_OBJS];
    int16_t fromX[MTG_MAX_OBJS], fromY[MTG_MAX_OBJS];
    uint8_t slide[MTG_MAX_OBJS];
    int sweepAge, sweepMine, bigCard, bigAge;
    // the opponent's next move, shown before it's made (the user: their turns went by too fast to follow)
    int teleT, teleObj, teleFromHand;
    MtgAnswer teleAnswer;
    uint32_t teleStamp;
} B;

static u16 sHeldMusic;
static unsigned sTrayRgb = 0x3888e8;
static char sResultText[320] = "";
void MtgBoard_SetTray(unsigned rgb) { sTrayRgb = rgb; }
void MtgBoard_Result(const char *text) { snprintf(sResultText, sizeof(sResultText), "%s", text ? text : ""); }
int MtgBoard_Over(void) { return B.active && B.mode == MODE_RESULT ? B.result : 0; }
const MtgDuel *MtgBoard_Duel(void) { return &B.duel; }
static SDL_Texture *sMat, *sLayer;
static SDL_Renderer *sLayerOwner;

int MtgBoard_Active(void) { return B.active; }
void MtgBoard_Autoplay(int on) { B.autoplay = on; }

static const MtgCardDef *Def(int obj) { return Mtg_Card(B.view.objs[obj].def); }
static int AutoPass(unsigned keys);
#define STILL_FRAMES 8
static int ShowFocus(void) { return B.still >= STILL_FRAMES; }

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
    sResultText[0] = 0;
    B.active = 1;
    B.sweepAge = SWEEP_FRAMES;
    B.bigCard = -1;
    B.bigAge = BIG_FRAMES;
    B.holdAge = TOAST_FRAMES;
    B.pile = PILE_NONE;
    B.endTurn = -1;
    snprintf(B.opp, sizeof(B.opp), "%s", opponent);
    Mtg_DuelInit(&B.duel, mine, theirs, seed);
    memcpy(&B.view, &B.duel, sizeof(B.view));
    B.evShown = B.duel.eventCount;   // (the opening hands: dealt before the table shows)
    {
        int i;
        for (i = 0; i < FLOATERS; i++)
            B.floatAge[i] = 40;
    }
    B.row = ROW_HAND;
    B.trayWanted = 1;
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

// ---- the layout, in the mat's coordinates (240 x LAYER_H; the screen shows it from Pan() down)
#define LAYER_H 200
#define ROW_X 34       // the rows: 34..205, the side columns beside them
#define ROW_W 172
#define CARD_W 22      // a creature or other permanent: 22 x 30, its plate under it
#define CARD_H 30
#define LAND_W 15      // a land: 15 x 20
#define LAND_H 20
#define HAND_W 30      // the hand: 30 x 42 on the tray
#define HAND_H 42
#define PILE_W 18
#define PILE_H 25
#define CENTER_X 120
#define CENTER_Y 77
#define BAR_CLOSED 152
#define BAR_OPEN 118
#define PAN 37
#define SLIDE 10       // frames a card takes to move
// the rows' frames (y, height) and where their cards stand
static const int sFrameY[4] = {1, 26, 88, 130}, sFrameH[4] = {24, 41, 41, 24};
static const int sCardY[4] = {3, 28, 90, 132};

static void Play(void);
static void DrawPlay(void);
static int Frames(int n);
int MtgBoard_Busy(void);
static Rect MakeRect(int x, int y, int w, int h)
{
    Rect r;
    r.x = (int16_t)x, r.y = (int16_t)y, r.w = (int16_t)w, r.h = (int16_t)h, r.on = 1;
    return r;
}
static Rect PileRect(int pile)
{
    switch (pile) {
    case PILE_MY_GRAVE: return MakeRect(215, 125, PILE_W, PILE_H);
    case PILE_MY_LIBRARY: return MakeRect(215, 96, PILE_W, PILE_H);
    case PILE_OPP_LIBRARY: return MakeRect(7, 32, PILE_W, PILE_H);
    default: return MakeRect(7, 3, PILE_W, PILE_H);
    }
}
static const Rect sOppHand = {1, 61, 30, 27, 1};
static Rect LifeRect(int player) { return player == ME ? MakeRect(1, 126, 30, 25) : MakeRect(209, 1, 30, 25); }

static float Ease(float t)
{
    if (t <= 0)
        return 0;
    if (t >= 1)
        return 1;
    return 1 - (1 - t) * (1 - t) * (1 - t);
}
static int Fast(void) { return Tcg_GetCollection()->fastAnimations; }
static float BarY(void) { return BAR_CLOSED + (BAR_OPEN - BAR_CLOSED) * Ease(B.tray); }
static float Pan(void) { return Ease(B.tray) * PAN; }
static float HandSpacing(int n)
{
    float space = 184.0f - HAND_W;
    if (n <= 1)
        return 0;
    return space / (n - 1) > 33 ? 33 : space / (n - 1);
}

static int IsLand(int obj) { return (Def(obj)->types & MTG_T_LAND) != 0; }

static void BuildRows(void)
{
    const MtgDuel *d = &B.view;
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

static void Layout(void)
{
    int r, i;
    memset(B.rect, 0, sizeof(B.rect));
    for (r = 0; r < ROW_COUNT; r++) {
        int n = B.itemCount[r], land = r == ROW_MY_LANDS || r == ROW_OPP_LANDS;
        if (r == ROW_HAND) {   // on the tray, spread as the POKeMON hand is
            float spacing = HandSpacing(n), width = spacing * (n ? n - 1 : 0) + HAND_W;
            for (i = 0; i < n; i++) {
                int x = (int)(120 - width / 2 + spacing * i), y = (int)(BarY() + Pan()) + 2;
                if (B.row == ROW_HAND && B.col[r] == i && B.pile == PILE_NONE && B.tray > 0.5f && B.mode != MODE_VIEW)
                    y -= 4;
                B.rect[B.items[r][i].obj] = MakeRect(x, y, HAND_W, HAND_H);
            }
            continue;
        }
        {
            int cw = land ? LAND_W : CARD_W, ch = land ? LAND_H : CARD_H, width[ROW_ITEMS], total = 0;
            float squeeze = 1, fx;
            for (i = 0; i < n; i++) {
                const MtgObj *o = &B.view.objs[B.items[r][i].obj];
                width[i] = o->tapped ? ch : cw;
                total += width[i] + (i ? 3 : 0);
            }
            if (total > ROW_W - 6 && n > 1)
                squeeze = (float)(ROW_W - 6 - width[n - 1]) / (total - width[n - 1]);
            fx = total < ROW_W - 6 ? CENTER_X - total / 2.0f : ROW_X + 3;   // (centered, as the Active Pokemon is)
            for (i = 0; i < n; i++) {
                const MtgObj *o = &B.view.objs[B.items[r][i].obj];
                Rect rc = o->tapped ? MakeRect((int)fx, sCardY[r] + ch - cw, ch, cw) : MakeRect((int)fx, sCardY[r], cw, ch);
                if (o->attacking)   // (an attacker steps toward the other side)
                    rc.y += o->controller == ME ? -5 : 5;
                B.rect[B.items[r][i].obj] = rc;
                if (land) {   // every land of a stack answers to its rect
                    int k;
                    for (k = 0; k < B.view.objCount; k++)
                        if (B.view.objs[k].zone == MTG_ZONE_BATTLEFIELD && B.view.objs[k].def == o->def
                            && B.view.objs[k].tapped == o->tapped && B.view.objs[k].controller == o->controller)
                            B.rect[k] = rc;
                }
                fx += (width[i] + 3) * squeeze;
            }
        }
    }
    B.playerRect[THEM] = LifeRect(THEM);
    B.playerRect[ME] = LifeRect(ME);
}

static void Slides(void)   // (once a frame, from MtgBoard_Step)
{
    int i;
    for (i = 0; i < B.view.objCount && i < MTG_MAX_OBJS; i++) {
        const Rect *now = &B.rect[i], *was = &B.last[i];
        int inHand = B.view.objs[i].zone == MTG_ZONE_HAND;
        if (!now->on) {
            B.slide[i] = 0;
            continue;
        }
        // (the hand follows the tray as it moves: only a new card in it slides, from the library)
        if (!was->on || (!inHand && (was->x != now->x || was->y != now->y))) {
            Rect from = *was;
            if (!was->on) {
                if (B.spawn[i].on)
                    from = B.spawn[i];
                else if (inHand)
                    from = PileRect(PILE_MY_LIBRARY);
                else if (B.view.objs[i].controller == THEM)
                    from = MakeRect(105, -50, CARD_W, CARD_H);
                else
                    from.on = 0;
            }
            if (from.on) {
                float p = B.slide[i] ? 1.0f - (float)B.slide[i] / SLIDE : 1.0f;
                B.fromX[i] = (int16_t)(B.slide[i] ? B.fromX[i] + (was->x - B.fromX[i]) * p : from.x);
                B.fromY[i] = (int16_t)(B.slide[i] ? B.fromY[i] + (was->y - B.fromY[i]) * p : from.y);
                B.slide[i] = SLIDE;
            }
        } else if (B.slide[i])
            B.slide[i]--;
        B.spawn[i].on = 0;
    }
    memcpy(B.last, B.rect, sizeof(B.last));
}
static void DrawPos(int obj, const Rect *rc, float *x, float *y)
{
    float p = B.slide[obj] ? 1.0f - (float)B.slide[obj] / SLIDE : 1.0f;
    p = 1 - (1 - p) * (1 - p);
    *x = B.slide[obj] ? B.fromX[obj] + (rc->x - B.fromX[obj]) * p : rc->x;
    *y = B.slide[obj] ? B.fromY[obj] + (rc->y - B.fromY[obj]) * p : rc->y;
}

static int Focused(void)
{
    int r = B.row, c = B.col[r];
    if (B.pile != PILE_NONE || B.itemCount[r] == 0)
        return MTG_NO_OBJ;
    if (c >= B.itemCount[r])
        c = B.col[r] = B.itemCount[r] - 1;
    return B.items[r][c].obj;
}

// ---- drawing
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

static int HasAction(int obj)   // the player could do something with it now
{
    const MtgQuery *q = &B.duel.query;
    int i;
    if (q->kind != MTG_Q_PRIORITY || q->player != ME)
        return 0;
    for (i = 1; i < q->actionCount; i++) {
        const MtgObj *o = &B.view.objs[q->actions[i].obj];
        if (q->actions[i].obj == obj || (IsLand(obj) && o->zone == B.view.objs[obj].zone && o->def == B.view.objs[obj].def
                                          && o->tapped == B.view.objs[obj].tapped && o->controller == B.view.objs[obj].controller))
            return 1;
    }
    return 0;
}

// the mat's colors: a warm leather cloth with gold lines (the CARD CASE's MAGIC side)
#define MAT_BASE 0x5a3c26
#define MAT_DARK 0x3a2414
#define MAT_LINE 0xd8a868
#define MAT_LABEL 0xb88a50

/* A place for a card: a soft hollow in the cloth and a clean line round it (tcg_board.c CardSpot) */
static void CardSpot(struct GSurface *s, Rect r, unsigned line, unsigned alpha)
{
    S_RoundFill(s, r.x, r.y, r.w, r.h, 1, ARGB(0x000000, alpha / 4));
    S_RoundFrame(s, r.x - 1, r.y - 1, r.w + 2, r.h + 2, 2, ARGB(line, alpha));
}
static void BuildMat(void)
{
    struct GSurface s = G_SurfaceNew(240, LAYER_H);
    int x, y, k, who;
    if (!s.px)
        return;
    /* Cloth: the colour darkens towards the edges, a fine weave over it */
    for (y = 0; y < LAYER_H; y++)
        for (x = 0; x < 240; x++) {
            float dx = (x - 120) / 120.0f, dy = (y - CENTER_Y) / 90.0f, d = dx * dx + dy * dy;
            S_Px(&s, x, y, ARGB(G_Mix(MAT_BASE, MAT_DARK, d > 1 ? 0.6f : d * 0.6f), 255));
        }
    for (y = 0; y < LAYER_H; y++)   /* (a leather grain: soft specks, no lines) */
        for (x = 0; x < 240; x++)
        {
            uint32_t h = (uint32_t)(x * 73856093u) ^ (uint32_t)(y * 19349663u);
            h ^= h >> 13, h *= 0x5bd1e995u, h ^= h >> 15;
            if (h % 7 == 0)
                S_Blend(&s, x, y, G_Mix(MAT_BASE, MAT_LINE, 0.07f), 255);
            else if (h % 11 == 0)
                S_Blend(&s, x, y, G_Mix(MAT_BASE, 0x000000, 0.12f), 255);
        }
    /* The rows: a frame round each (as the Benches have), a hollow along it */
    for (k = 0; k < 4; k++) {
        S_RoundFrame(&s, ROW_X, sFrameY[k], ROW_W, sFrameH[k], 3, ARGB(MAT_LINE, 255));
        S_RoundFill(&s, ROW_X + 2, sFrameY[k] + 2, ROW_W - 4, sFrameH[k] - 4, 2, ARGB(0x000000, 22));
    }
    /* Side columns: a spot for each pile, the life plates' frames, the opponent's hand */
    for (who = 0; who < 4; who++) {
        Rect p = PileRect(who);
        S_RoundFrame(&s, p.x - 4, p.y - 2, p.w + 8, p.h + 4, 3, ARGB(MAT_LINE, 255));
        CardSpot(&s, p, MAT_LINE, 110);
    }
    for (who = 0; who < 2; who++) {
        Rect l = LifeRect(who);
        S_RoundFrame(&s, l.x, l.y, l.w, l.h, 3, ARGB(MAT_LINE, 255));
    }
    S_RoundFrame(&s, sOppHand.x, sOppHand.y, sOppHand.w, sOppHand.h, 3, ARGB(MAT_LINE, 150));
    sMat = G_SurfaceTexture(&s);
    G_SurfaceFree(&s);
}
static void Labels(void)
{
    G_TextAlign(204, sFrameY[3] - 10, TA_RIGHT, MAT_LABEL, G_NO_SHADOW, "LANDS", TF_NARROW);
}

// A plate like the HP plates (G_HpPlate): the frame, the dark glass, digits with their unlit segments behind
static void Plate(int x, int y, int w, int h, const char *label, const char *digits, int big, unsigned text)
{
    unsigned bg = 0x14203a, ink = 0x5aa8ff, ghost = 0x223456;
    char unlit[8];
    int n = (int)strlen(digits), i, size = big ? 1 : 0;
    G_RoundFill(x, y, w, h, 0x282828, 255);
    G_FillA(x + 1, y + 1, w - 2, 1, 0x9a9a9a, 255);
    G_FillA(x + 1, y + h - 2, w - 2, 1, 0x585858, 255);
    G_FillA(x + 2, y + 2, w - 4, h - 4, bg, 255);
    if (label)
        G_Text(x + 3, y + (big ? 0 : -2), ink, G_NO_SHADOW, label, 0, TF_NARROW);
    for (i = 0; i < n && i < 7; i++)
        unlit[i] = digits[i] >= '0' && digits[i] <= '9' ? '8' : digits[i];
    unlit[i] = 0;
    G_Digits(x + w - 3 - G_DigitsWidth(unlit, size, 1), y + (big ? h - 10 : 2), unlit, size, 1, ghost, 255);
    G_Digits(x + w - 3 - G_DigitsWidth(digits, size, 1), y + (big ? h - 10 : 2), digits, size, 1, text, 255);
}

// A cost at a glance, centered on cx with its top at y: the generic amount in a grey orb with its number, a dot for
// each colored mana, X; a {T} first when it taps (tap)
static void TapIcon(int x, int y)
{
    G_RoundFill(x, y, 7, 7, 0x202020, 255);
    G_RoundFill(x + 1, y + 1, 5, 5, 0xe8e4dc, 255);
    G_Fill(x + 2, y + 2, 3, 1, 0x202020);   // (a curved arrow: the tap symbol)
    G_Fill(x + 2, y + 2, 1, 3, 0x202020);
    G_Fill(x + 3, y + 4, 2, 1, 0x202020);
    G_Fill(x + 4, y + 1, 1, 2, 0x202020);
}
static int CompactCostWidth(const MtgManaCost *m, int tap)
{
    int w = tap ? 8 : 0, k;
    if (m->generic || m->x)
        w += 8 * ((m->generic ? 1 : 0) + m->x);
    for (k = 0; k < MTG_MANA_KINDS; k++)
        w += m->pip[k] * 6;
    return w ? w + 3 : 0;
}
static void CompactCost(int cx, int y, const MtgManaCost *m, int tap)
{
    int w = CompactCostWidth(m, tap), x, k, i;
    char n[8];
    if (!w)
        return;
    x = cx - w / 2;
    G_RoundFill(x, y, w, 9, 0x101418, 225);   // (the pill)
    x += 2;
    if (tap)
        TapIcon(x, y + 1), x += 8;
    for (i = 0; i < m->x; i++, x += 8) {
        G_RoundFill(x, y + 1, 7, 7, 0xc8c4bc, 255);
        G_Text(x + 1, y - 2, 0x202020, G_NO_SHADOW, "X", 0, TF_NARROW);
    }
    if (m->generic) {
        snprintf(n, sizeof(n), "%u", m->generic > 9 ? 9 : m->generic);
        G_RoundFill(x, y + 1, 7, 7, 0xc8c4bc, 255);
        G_Digits(x + 2, y + 2, n, 0, 1, 0x202020, 255);
        x += 8;
    }
    for (k = 0; k < MTG_MANA_KINDS; k++)
        for (i = 0; i < m->pip[k]; i++, x += 6) {
            G_RoundFill(x, y + 2, 5, 5, 0x202020, 255);
            G_Fill(x + 1, y + 3, 3, 3, MtgGfx_ManaColor(k));
        }
}
// what using a permanent costs: its first activated ability's cost (not a land's mana)
static const MtgCost *ActivatedCost(int obj)
{
    const MtgCardDef *c = Def(obj);
    int k;
    if (c->types & MTG_T_LAND)
        return NULL;
    for (k = 0; k < c->abilityCount; k++)
        if (gMtgAbilities[c->abilities[k]].kind == MTG_AB_ACTIVATED)
            return &gMtgAbilities[c->abilities[k]].cost;
    return NULL;
}

// choosing attackers and blockers: who's chosen, who can be, why not
static int ObjIndex(const uint16_t *list, int n, int obj);
static int ChosenAttacker(int obj)
{
    int i;
    if (B.mode != MODE_ATTACK)
        return 0;
    i = ObjIndex(B.duel.query.candidates, B.duel.query.candidateCount, obj);
    return i >= 0 && ((B.attack >> i) & 1);
}
static const char *WhyCantAttack(int obj)
{
    const MtgObj *o = &B.duel.objs[obj];
    uint32_t kw = Mtg_Keywords(&B.duel, obj);
    if (!(Def(obj)->types & MTG_T_CREATURE))
        return "Only creatures attack.";
    if (o->tapped)
        return "It's tapped: it can't attack.";
    if (kw & MTG_KW_BIT(MTG_KW_DEFENDER))
        return "It has DEFENDER: it never attacks.";
    if (o->sick && !(kw & MTG_KW_BIT(MTG_KW_HASTE)))
        return "It arrived this turn: it can attack next turn.";
    return "It can't attack now.";
}
static int AttackPower(void)   // the damage the chosen attackers would deal unblocked
{
    const MtgQuery *q = &B.duel.query;
    int i, n = 0;
    for (i = 0; i < q->candidateCount; i++)
        if ((B.attack >> i) & 1) {
            int p = Mtg_Power(&B.duel, q->candidates[i]);
            n += p > 0 ? p : 0;
        }
    return n;
}

// a card on the table: its face (on its side when tapped), the plate under a creature, what it's doing
static void DrawCard(int obj, const Rect *rc, int count, float x, float y)
{
    const MtgObj *o = &B.view.objs[obj];
    const MtgCardDef *c = Def(obj);
    int onTable = o->zone == MTG_ZONE_BATTLEFIELD, tapped = o->tapped && onTable;
    int cw = tapped ? rc->h : rc->w, ch = tapped ? rc->w : rc->h;
    char pt[16];
    float cx, cy, turn = tapped ? 90.0f : 0.0f;
    if (ChosenAttacker(obj))   // (chosen to attack: a step forward)
        y -= 5;
    if (B.lunge[obj]) {   // attacking or striking: a lunge toward the other side
        float k = (float)B.lunge[obj] / (float)Frames(14);
        y += (o->controller == ME ? -1 : 1) * sinf((1 - (k > 1 ? 1 : k)) * 3.14159f) * 7;
    }
    if (B.tapT[obj] && onTable) {   // tapping or untapping: it turns
        float k = (float)B.tapT[obj] / (float)Frames(6);
        turn = o->tapped ? 90 * (1 - k) : 90 * k;
    }
    cx = x + rc->w / 2.0f, cy = y + rc->h / 2.0f;
    if (onTable)
        G_FillA((int)x + 1, (int)y + 1, rc->w, rc->h, 0x000000, 70);
    // (the opponent's cards face them, as on the other card tables: turned half round, tapped a quarter more)
    MtgGfx_DrawCard(o->def, cx - cw / 2.0f, cy - ch / 2.0f, (float)cw, (float)ch,
                    turn + (onTable && o->controller == THEM ? 180 : 0), 255,
                    B.flash[obj] ? (unsigned)(B.flash[obj] * 14) : 0);
    if (o->attacking)
        G_RoundFrame((int)x - 1, (int)y - 1, rc->w + 2, rc->h + 2, 0xf04030, 255);
    if (o->blocking != MTG_NO_OBJ)
        G_RoundFrame((int)x - 1, (int)y - 1, rc->w + 2, rc->h + 2, 0x4080f0, 255);
    // what it costs to cast (in the hand) or to use (on the table), on its top edge: the card under the cursor only
    // (the user: for the table's looks)
    if (Focused() == obj && ShowFocus() && B.mode != MODE_RESULT && B.mode != MODE_VIEW && B.mode != MODE_CHOOSE
        && B.mode != MODE_PICK) {
        if (o->zone == MTG_ZONE_HAND && !(c->types & MTG_T_LAND)) {
            MtgManaCost cost;   // (as it would be paid now: what cost-changing cards on the table add or take off)
            Mtg_SpellCost(&B.view, o->owner, obj, &cost);
            CompactCost((int)cx, (int)y - 4, &cost, 0);
        }
        else if (onTable) {
            const MtgCost *k = ActivatedCost(obj);
            if (k)
                CompactCost((int)cx, (int)y - 4, &k->mana, k->tap);
        }
    }
    if (!onTable)
        return;
    {   // Auras: the creature under the cursor and its Auras (or the Aura and its creature) framed in gold; a creature
        // wearing one has a gold mark in its corner
        int f = Focused(), k, worn = 0;
        if (f != MTG_NO_OBJ && f != obj && B.view.objs[f].zone == MTG_ZONE_BATTLEFIELD
            && (B.view.objs[f].attachedTo == obj || o->attachedTo == f))
            G_RoundFrame((int)x - 2, (int)y - 2, rc->w + 4, rc->h + 4, 0xf0c040, 255);
        for (k = 0; k < B.view.objCount && !worn; k++)
            worn = B.view.objs[k].zone == MTG_ZONE_BATTLEFIELD && B.view.objs[k].attachedTo == obj;
        if (worn) {
            G_RoundFill((int)x - 1, (int)y - 1, 6, 6, 0xf0c040, 255);
            G_RoundFrame((int)x - 1, (int)y - 1, 6, 6, 0x5a3a08, 255);
        }
        if (o->regen) {   // a regeneration shield: a green shield in the top right corner
            int sx = (int)x + rc->w - 6, sy = (int)y - 1;
            G_RoundFill(sx, sy, 7, 6, 0x50d080, 255);
            G_Fill(sx + 2, sy + 6, 3, 1, 0x50d080);
            G_RoundFrame(sx, sy, 7, 6, 0x0c3a1c, 255);
        }
    }
    if (IsLand(obj) && count > 1) {   // (the count tab, as on the piles)
        int w;
        snprintf(pt, sizeof(pt), "%d", count);
        w = G_DigitsWidth(pt, 0, 1) + 4;
        G_RoundFill((int)x + rc->w - w + 3, (int)y + rc->h - 5, w, 9, 0x1c2c48, 235);
        G_RoundFrame((int)x + rc->w - w + 3, (int)y + rc->h - 5, w, 9, 0x8ab0e0, 255);
        G_Digits((int)x + rc->w - w + 5, (int)y + rc->h - 3, pt, 0, 1, 0xe0f0ff, 255);
    }
    if (c->types & MTG_T_CREATURE) {
        int power = Mtg_Power(&B.view, obj), tough = Mtg_Toughness(&B.view, obj) - o->damage;
        unsigned text = o->damage ? 0xffd860 : ((power > c->power && !(c->starPT & 1)) || (tough > c->toughness && !(c->starPT & 2)))
                                               ? 0x9cf0b0 : 0xc8ecff;
        snprintf(pt, sizeof(pt), "%d/%d", power < 0 ? 0 : power, tough < 0 ? 0 : tough);
        Plate((int)cx - 13, (int)y + rc->h, 26, 9, NULL, pt, 0, text);
        if (o->sick && !(Mtg_Keywords(&B.view, obj) & MTG_KW_BIT(MTG_KW_HASTE)))
            G_FillA((int)x, (int)y, rc->w, rc->h, 0xffffff, 40);   // (can't attack yet)
    }
}

static const char *const sStepNames[MTG_STEP_COUNT] = {
    "UNTAP", "UPKEEP", "DRAW", "MAIN 1", "COMBAT", "ATTACK", "BLOCK", "FIRST STRIKE", "DAMAGE", "END COMBAT",
    "MAIN 2", "END", "CLEANUP"};

static int HandCount(int player)
{
    int i, n = 0;
    for (i = 0; i < B.view.objCount; i++)
        n += B.view.objs[i].zone == MTG_ZONE_HAND && B.view.objs[i].owner == player;
    return n;
}
static int Graveyard(int player, uint16_t *out)   // newest first; returns how many
{
    int i, n = 0, k;
    for (i = 0; i < B.view.objCount; i++)
        if (B.view.objs[i].zone == MTG_ZONE_GRAVEYARD && B.view.objs[i].owner == player) {
            for (k = n; k > 0 && B.view.objs[out[k - 1]].seq < B.view.objs[i].seq; k--)
                out[k] = out[k - 1];
            out[k] = (uint16_t)i;
            n++;
        }
    return n;
}

/* A pile (tcg_board.c DrawPile): the backs stacked thicker as it grows, its count in a tab */
static void DrawPile(Rect r, int count, int deck, int top, int theirs)
{
    char n[8];
    int i, layers = count > 40 ? 3 : count > 15 ? 2 : count > 1 ? 1 : 0;
    if (!count)
        return;
    if (deck) {
        for (i = layers; i > 0; i--)
            MtgGfx_DrawCard(MTG_GFX_BACK, (float)r.x + i, (float)r.y + i, (float)r.w, (float)r.h, theirs ? 180 : 0, 255, 0);
        MtgGfx_DrawCard(MTG_GFX_BACK, (float)r.x, (float)r.y, (float)r.w, (float)r.h, theirs ? 180 : 0, 255, 0);
    } else
        MtgGfx_DrawCard(B.view.objs[top].def, (float)r.x, (float)r.y, (float)r.w, (float)r.h, theirs ? 180 : 0, 255, 0);
    snprintf(n, sizeof(n), "%d", count);
    {
        int w = G_DigitsWidth(n, 0, 1) + 4, x = r.x + r.w - w + 3, y = r.y + r.h - 5;
        G_RoundFill(x, y, w, 9, 0x1c2c48, 235);
        G_RoundFrame(x, y, w, 9, 0x8ab0e0, 255);
        G_Digits(x + 2, y + 2, n, 0, 1, 0xe0f0ff, 255);
    }
}

static void DrawLife(int player)
{
    Rect l = LifeRect(player);
    char n[8];
    int life = B.view.players[player].life;
    snprintf(n, sizeof(n), "%d", life < 0 ? 0 : life);
    Plate(l.x + 1, l.y + 1, l.w - 2, l.h - 2, "LIFE", n, 1, life <= 0 ? 0xff3c3c : life <= 5 ? 0xffd860 : 0xc8ecff);
    if (B.lifeFlash[player]) {   // (it just changed: lit, a little shake)
        G_FillAdd(l.x + 1, l.y + 1, l.w - 2, l.h - 2, 0xffffff, (unsigned)(B.lifeFlash[player] * 10));
        G_RoundFrame(l.x + (B.lifeFlash[player] & 2 ? 1 : -1), l.y, l.w, l.h, 0xffffff, 200);
    }
}

/* The opponent's hand (tcg_board.c DrawOppHand): HAND, up to three backs fanned, how many */
static void DrawOppHand(void)
{
    char n[8];
    int i, count = HandCount(THEM), shown = count > 3 ? 3 : count;
    G_TextAlign(16, 62, TA_CENTER, MAT_LABEL, G_NO_SHADOW, "HAND", TF_NARROW);
    for (i = 0; i < shown; i++)
        MtgGfx_DrawCard(MTG_GFX_BACK, (float)(5 + i * 5), 73.0f - (i == 1), 10, 14, (i - 1) * 8.0, 255, 0);
    if (B.teleT > 0 && B.teleFromHand) {
        // the card they're about to play, out of their hand and glowing
        int life = Frames(30), age = life - B.teleT;
        float k = age < 8 ? age / 8.0f : 1.0f;
        float x = 10 + 20 * k, y = 70 - 6 * k;
        MtgGfx_DrawCard(MTG_GFX_BACK, x, y, 12, 17, 0, 255, (unsigned)(90 + 60 * ((B.frame / 6) & 1)));
    }
    snprintf(n, sizeof(n), "%d", count);
    G_Digits(18 + (shown ? 4 : -6), 82, n, 0, 1, 0xf8f8f8, 255);
}

/* The hand's tray, as every card table's (TcgUi_DrawTray), in the deck's color (MtgBoard_SetTray) */
static void DrawHandBar(void)
{
    TcgUi_DrawTray((int)(BarY() + Pan()), sTrayRgb, (unsigned)HandCount(ME), B.tray > 0.5f);
}

// the step of the turn in the middle: UNT UPK DRW M1 CMB M2 END, the one now lit in the turn's color
static void PhaseBar(void)
{
    static const char *const names[7] = {"UNT", "UPK", "DRW", "M1", "CMB", "M2", "END"};
    static const uint8_t group[MTG_STEP_COUNT] = {0, 1, 2, 3, 4, 4, 4, 4, 4, 4, 5, 6, 6};
    int i, w = 0, x, on = group[B.view.step < MTG_STEP_COUNT ? B.view.step : 0], y = 67;
    unsigned mine = B.view.active == ME ? 0x3878f0 : 0xe85050;
    for (i = 0; i < 7; i++)
        w += G_Measure(names[i], TF_NARROW) + 4;
    x = CENTER_X - w / 2;
    G_RoundFill(x - 3, y + 1, w + 4, 10, 0x1c1008, 190);
    for (i = 0; i < 7; i++) {
        int tw = G_Measure(names[i], TF_NARROW);
        if (i == on) {
            G_RoundFill(x - 2, y + 1, tw + 4, 10, mine, 255);
            G_Text(x, y, G_WHITE, G_NO_SHADOW, names[i], 0, TF_NARROW);
        } else
            G_Text(x, y, 0xa08a6a, G_NO_SHADOW, names[i], 0, TF_NARROW);
        x += tw + 4;
    }
}

// what just happened (or the top of the stack), under the step
static void StackLine(char *out, unsigned cap)
{
    const MtgStackEntry *e = &B.view.stack[B.view.stackCount - 1];
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
static void NewsLine(void)
{
    char line[120];
    if (!MtgBoard_Busy() && B.row == ROW_MY_PERMS && B.pile == PILE_NONE && B.itemCount[ROW_MY_PERMS])
        return;   // (the tooltip over your creatures sits here)
    line[0] = 0;
    if (B.view.stackCount)
        StackLine(line, sizeof(line));
    else if (B.tickerCount)
        snprintf(line, sizeof(line), "%s", B.ticker[B.tickerCount - 1]);
    if (!line[0])
        return;
    TcgUi_FitText(line, 168, TF_NARROW);
    G_TextAlign(CENTER_X, 77, TA_CENTER, G_WHITE, 0x1c1008, line, TF_NARROW);
}

// the stack: its cards in the middle, the newest in front, a line to what it aims at
static void StackCards(void)
{
    int i, n = B.view.stackCount, first = n > 3 ? n - 3 : 0;
    for (i = first; i < n; i++) {
        const MtgStackEntry *e = &B.view.stack[i];
        int k = i - first, x = CENTER_X - 11 + k * 9 - (n - first - 1) * 4, y = CENTER_Y - 15 - 18 + k * 2, top = i == n - 1;
        unsigned glow = 0;
        if (top && B.castT && B.castLife) {   // (just put on the stack: on its way from where it was)
            float t = 1.0f - (float)B.castT / (float)B.castLife;
            t = t * t * (3 - 2 * t);
            x = (int)(B.castFrom.x + (x - B.castFrom.x) * t);
            y = (int)(B.castFrom.y + (y - B.castFrom.y) * t);
        }
        if (top && B.resolveT && B.resolveLife)   // (resolving: it lights up)
            glow = (unsigned)(220 * sinf((1.0f - (float)B.resolveT / (float)B.resolveLife) * 3.14159f));
        G_FillA(x + 2, y + 3, CARD_W, CARD_H, 0x000000, 100);
        if (top)
            G_FoilHalo(x, y, CARD_W, CARD_H, e->controller == ME ? 0x80b0ff : 0xff9080, B.frame, 0);
        MtgGfx_DrawCard(B.view.objs[e->source].def, (float)x, (float)y, CARD_W, CARD_H, e->controller == THEM ? 180 : 0,
                        255, glow);
        if (top && (e->target.kind == MTG_TGT_OBJ || e->target.kind == MTG_TGT_PLAYER)) {
            Rect t = e->target.kind == MTG_TGT_PLAYER ? B.playerRect[e->target.id] : B.rect[e->target.id];
            if (t.on) {
                unsigned col = e->controller == ME ? 0x80b0ff : 0xff9080;
                G_Line(x + 11, y + 15, t.x + t.w / 2, t.y + t.h / 2, col, 200);
                G_Line(x + 12, y + 15, t.x + t.w / 2 + 1, t.y + t.h / 2, col, 120);
                G_RoundFrame(t.x - 2, t.y - 2, t.w + 4, t.h + 4, col, 255);
            }
        }
    }
}

static Rect FocusRect(void)
{
    int f;
    if (B.pile != PILE_NONE)
        return PileRect(B.pile);
    f = Focused();
    if (f == MTG_NO_OBJ)
        return MakeRect(0, 0, 0, 0);
    return B.rect[f];
}

/* What's under the cursor, in the tooltip box beside it (tcg_board.c Tooltip): what it is and what A does */
static void Tooltip(void)
{
    char line[120] = "", keys[120] = "";
    Rect r = FocusRect();
    int w, x, y, f = Focused(), h = 12;
    if (!r.w)
        return;
    if (B.pile == PILE_MY_LIBRARY || B.pile == PILE_OPP_LIBRARY) {
        int p = B.pile == PILE_MY_LIBRARY ? ME : THEM;
        uint16_t list[MTG_MAX_OBJS];
        snprintf(line, sizeof(line), "%s LIBRARY %d  HAND %d  GRAVEYARD %d%s", p == ME ? "YOUR" : "THEIR",
                 B.view.players[p].libraryCount, HandCount(p), Graveyard(p, list), p == ME ? "  {A} CONCEDE" : "");
    } else if (B.pile != PILE_NONE) {
        uint16_t list[MTG_MAX_OBJS];
        snprintf(line, sizeof(line), "%s GRAVEYARD: %d  {A} LOOK", B.pile == PILE_MY_GRAVE ? "YOUR" : "THEIR",
                 Graveyard(B.pile == PILE_MY_GRAVE ? ME : THEM, list));
    } else if (f != MTG_NO_OBJ) {
        const MtgCardDef *c = Def(f);
        const MtgObj *o = &B.view.objs[f];
        char name[64];
        Name(name, sizeof(name), f);
        if (c->types & MTG_T_CREATURE) {
            if (o->zone == MTG_ZONE_BATTLEFIELD)
                snprintf(line, sizeof(line), "%s  %d/%d", name, Mtg_Power(&B.view, f), Mtg_Toughness(&B.view, f));
            else
                snprintf(line, sizeof(line), "%s  %s", name, Mtg_PTText(c));
        } else
            snprintf(line, sizeof(line), "%s", name);
        if (o->zone == MTG_ZONE_BATTLEFIELD && o->attachedTo != MTG_NO_OBJ) {   // an Aura: what it's on
            char on[40];
            Name(on, sizeof(on), o->attachedTo);
            strncat(line, "  ON ", sizeof(line) - strlen(line) - 1);
            strncat(line, on, sizeof(line) - strlen(line) - 1);
        }
        if (HasAction(f) && B.mode == MODE_NORMAL)
            strncat(line, "  {A}", sizeof(line) - strlen(line) - 1);
        else if (B.mode == MODE_ATTACK && o->controller == ME && o->zone == MTG_ZONE_BATTLEFIELD)
            strncat(line, ObjIndex(B.duel.query.candidates, B.duel.query.candidateCount, f) < 0 ? ""
                          : ChosenAttacker(f) ? "  {A} HOLD BACK" : "  {A} ATTACK", sizeof(line) - strlen(line) - 1);
        else if (B.mode == MODE_BLOCK && o->controller == ME && o->zone == MTG_ZONE_BATTLEFIELD) {
            int k = ObjIndex(B.duel.query.candidates, B.duel.query.candidateCount, f);
            strncat(line, k < 0 ? "" : B.block[k] >= 0 ? "  {A} DON'T BLOCK" : "  {A} BLOCK", sizeof(line) - strlen(line) - 1);
        }
        // its keywords under its name (what it has now: a pump's too)
        MtgGfx_KeywordLine(o->def, o->zone == MTG_ZONE_BATTLEFIELD ? Mtg_Keywords(&B.view, f) : c->keywords, keys,
                           sizeof(keys));
    }
    if (!line[0])
        return;
    w = G_Hint(0, 0, 0, 0, line, TF_NARROW, 0) + 8;
    if (keys[0]) {
        int kw = G_Measure(keys, TF_NARROW) + 8;
        if (kw > 236)
            TcgUi_FitText(keys, 228, TF_NARROW), kw = G_Measure(keys, TF_NARROW) + 8;
        if (kw > w)
            w = kw;
        h = 22;
    }
    x = r.x + r.w / 2 - w / 2;
    y = r.y + r.h + 4;
    if (f != MTG_NO_OBJ && (Def(f)->types & MTG_T_CREATURE) && B.view.objs[f].zone == MTG_ZONE_BATTLEFIELD)
        y += 9;   // (under its plate)
    if (B.row == ROW_HAND && B.pile == PILE_NONE)
        y = r.y - 3 - h;
    else if (y + h - Pan() > 150 || ((B.row == ROW_MY_LANDS || B.row == ROW_MY_PERMS) && B.pile == PILE_NONE))
        y = r.y - 4 - h;   // (your own cards: above them, as on the POKeMON Bench)
    if (x < 1)
        x = 1;
    if (x + w > 239)
        x = 239 - w;
    if (y < 1)
        y = r.y + r.h + 4;
    G_RoundFill(x, y, w, h, 0x1c1008, 215);
    G_RoundFrame(x, y, w, h, MAT_LINE, 255);
    G_Hint(x + 4, y, G_WHITE, G_NO_SHADOW, line, TF_NARROW, 1);
    if (keys[0])
        G_Text(x + 4, y + 10, 0xf8d878, G_NO_SHADOW, keys, 0, TF_NARROW);
}

/* What the player is being asked, in the yellow-framed box (tcg_board.c Prompt): at the top or the bottom */
static void Prompt(const char *text, int top)
{
    int w = G_Hint(0, 0, 0, 0, text, TF_NARROW, 0) + 12, x = 120 - w / 2, y = top ? 1 : 140;
    if (w > 236) {   /* too long for one line: split at spaces into lines that fit (two as tcg_board.c's, more for an
                        ability's words), each centered */
        char lines[4][160];
        int n = 0, i, widest = 0, h;
        const char *p = text;
        while (*p && n < 4) {
            size_t len = strlen(p), cut = len, k;
            char trial[160];
            while (*p == ' ')
                p++, len--;
            cut = len;
            for (k = 1; k <= len && k < sizeof(trial); k++) {
                if (p[k] != ' ' && p[k] != 0)
                    continue;
                snprintf(trial, sizeof(trial), "%.*s", (int)k, p);
                if (G_Hint(0, 0, 0, 0, trial, TF_NARROW, 0) > 224 && k != len) {
                    break;
                }
                cut = k;
                if (p[k] == 0)
                    break;
            }
            if (cut == len || n == 3) {
                snprintf(lines[n++], sizeof(lines[0]), "%s", p);
                break;
            }
            snprintf(lines[n++], sizeof(lines[0]), "%.*s", (int)cut, p);
            p += cut;
        }
        for (i = 0; i < n; i++) {
            int lw = G_Hint(0, 0, 0, 0, lines[i], TF_NARROW, 0);
            if (lw > widest)
                widest = lw;
        }
        w = widest + 12 > 236 ? 236 : widest + 12;
        x = 120 - w / 2;
        h = 3 + 11 * n;
        y = top ? 1 : 153 - h;
        G_RoundFill(x, y, w, h, 0x081830, 225);
        G_RoundFrame(x, y, w, h, 0xf8e070, 255);
        for (i = 0; i < n; i++) {
            int lw = G_Hint(0, 0, 0, 0, lines[i], TF_NARROW, 0);
            G_Hint(120 - lw / 2, y + 1 + 11 * i, 0xf8f0c0, G_NO_SHADOW, lines[i], TF_NARROW, 1);
        }
        return;
    }
    G_RoundFill(x, y, w, 13, 0x081830, 225);
    G_RoundFrame(x, y, w, 13, 0xf8e070, 255);
    G_Hint(x + 6, y + 1, 0xf8f0c0, G_NO_SHADOW, text, TF_NARROW, 1);
}
static void Menu(int x, int y, const char *const *items, int count, int cursor, unsigned disabled)
{
    int i, w = 0, h;
    for (i = 0; i < count; i++) {
        int m = G_Measure(items[i], TF_SMALL);
        if (m > w)
            w = m;
    }
    w += 24;
    h = count * 13 + 10;
    if (x + w > 238)
        x = 238 - w;
    if (x < 2)
        x = 2;
    if (y + h > 158)
        y = 158 - h;
    if (y < 2)
        y = 2;
    G_Window(x, y, w, h);
    for (i = 0; i < count; i++) {
        int off = (disabled >> i) & 1;
        if (i == cursor)
            G_Cursor(x + 7, y + 7 + i * 13);
        G_Text(x + 14, y + 4 + i * 13, off ? 0xa8a8a8 : G_INK, off ? G_NO_SHADOW : G_INK_SHADOW, items[i], 0, TF_SMALL);
    }
}

/* A note for the player (tcg_board.c's toast): the game's window over the middle a moment */
static void Toast(void)
{
    int w, h = 24;
    if (B.holdAge >= TOAST_FRAMES || !B.hold[0])
        return;
    w = G_Measure(B.hold, TF_SMALL) + 16;
    if (w > 224)
        w = 224, h = 36;
    G_Window(120 - w / 2, 62, w, h);
    G_Text(120 - w / 2 + 8, 66, G_INK, G_INK_SHADOW, B.hold, w - 16, TF_SMALL);
}

// a card's view over the table: its face big, its words (and how it stands now), as the card screens show one
static void InspectWindow(int obj)
{
    const MtgCardDef *c = Def(obj);
    const MtgObj *o = &B.view.objs[obj];
    char name[64], text[800], line[160];
    int y;
    G_FillA(0, 0, 240, 160, 0x000000, 150);
    G_FillA(10, 34, 72, 100, 0x000000, 90);
    MtgGfx_DrawCard(o->def, 8, 30, 72, 100, 0, 255, 0);
    TcgUi_ListBox(86, 4, 152, 140);
    Name(name, sizeof(name), obj);
    {
        int costW = MtgGfx_Cost(o->def, 232, 12, 1, 1);
        char keys[120];
        y = G_Text(94, 8, G_INK, G_INK_SHADOW, name, 136 - (costW ? costW + 4 : 0), TF_NORMAL);
        y = y + 2 > 23 ? y + 2 : 23;
        MtgGfx_KeywordLine(o->def, o->zone == MTG_ZONE_BATTLEFIELD ? Mtg_Keywords(&B.view, obj) : c->keywords, keys,
                           sizeof(keys));
        if (keys[0])   // (its keywords under its name)
            y = G_Text(94, y - 2, 0xb07818, G_NO_SHADOW, keys, 136, TF_NARROW) + 1;
    }
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
    y = G_Text(94, y, 0x586878, G_NO_SHADOW, text, 136, TF_NARROW) + 1;
    G_FillA(92, y, 140, 1, 0xd8d8d0, 255);
    Mtg_PlainText(text, sizeof(text), c->oracle);
    y = G_Text(94, y + 2, 0x404040, G_NO_SHADOW, text[0] ? text : "(No rules text.)", 136, TF_NARROW);
    if (c->types & MTG_T_CREATURE) {
        if (o->zone == MTG_ZONE_BATTLEFIELD && (Mtg_Power(&B.view, obj) != c->power
                                                || Mtg_Toughness(&B.view, obj) != c->toughness || o->damage))
            snprintf(line, sizeof(line), "%d/%d now (printed %s)%s", Mtg_Power(&B.view, obj),
                     Mtg_Toughness(&B.view, obj), Mtg_PTText(c), o->damage ? ", hurt" : "");
        else
            snprintf(line, sizeof(line), "%s", Mtg_PTText(c));
        G_TextAlign(230, y + 1, TA_RIGHT, G_INK, G_INK_SHADOW, line, TF_SMALL);
        y += 13;
    }
    G_FillA(92, y, 140, 1, 0xd8d8d0, 255);
    if (o->zone == MTG_ZONE_BATTLEFIELD) {
        snprintf(line, sizeof(line), "%s%s%s", o->controller == ME ? "Yours" : "Theirs", o->tapped ? ", tapped" : "",
                 o->sick && (c->types & MTG_T_CREATURE) ? ", arrived this turn" : "");
        G_Text(94, y + 2, 0x707878, G_NO_SHADOW, line, 136, TF_NARROW);
    }
    TcgUi_Hints("{B} BACK");
}

// a graveyard, card by card (as the POKeMON table shows a discard pile)
static void DrawView(void)
{
    int i, start = B.viewCursor / 14 * 14;
    char line[96];
    G_FillA(0, 0, 240, 160, 0x000000, 160);
    G_RoundFill(0, 0, 240, 17, 0x203040, 230);
    snprintf(line, sizeof(line), "%s GRAVEYARD", B.viewPlayer == ME ? "YOUR" : "THEIR");
    G_Text(8, 1, G_WHITE, 0x101820, line, 0, TF_NORMAL);
    snprintf(line, sizeof(line), "%d/%d", B.viewCursor + 1, B.viewCount);
    G_TextAlign(236, 3, TA_RIGHT, G_WHITE, G_NO_SHADOW, line, TF_NARROW);
    for (i = 0; i < 14 && start + i < B.viewCount; i++) {
        int x = 8 + (i % 7) * 33, y = 24 + (i / 7) * 46, on = start + i == B.viewCursor;
        if (on)
            y -= 2;
        MtgGfx_DrawCard(B.view.objs[B.viewList[start + i]].def, (float)x, (float)y, 30, 42, 0, 255, 0);
        if (on) {
            int o = (B.frame / 8) % 2;
            G_Frame(x - 2 - o, y - 2 - o, 34 + 2 * o, 46 + 2 * o, 0xe82820, 255);
        }
    }
    if (B.viewCursor < B.viewCount) {
        char name[64];
        Name(name, sizeof(name), B.viewList[B.viewCursor]);
        TcgUi_Strip(NULL, 0, name);
    }
    TcgUi_Hints("{INSPECT} LOOK  {B} BACK");
}

static int sMenuLabelsInit;
static char sMenuLabels[8][48];
static const char *const sTurnMenu[2] = {"PASS", "END TURN"};

static void DrawResult(void)
{
    const char *reason = B.view.players[B.result == 1 ? THEM : ME].drewFromEmpty ? "No cards left to draw."
                       : B.forced ? "" : "Life down to 0.";
    if (B.result == 3)
        reason = "Neither of you won.";
    G_ResultScreen(B.result == 1, B.resultAge, B.frame, B.opp, reason,
                   sResultText);
}

// YOUR TURN / THEIR TURN: a band across the middle, the words sweeping through (the other card tables' look)
static void DrawSweep(void)
{
    float t = (float)B.sweepAge / SWEEP_FRAMES, in, out;
    int w, x, band, cy = 80;
    unsigned color = B.sweepMine ? 0x3878f0 : 0xe85050;
    const char *text = B.sweepMine ? "YOUR TURN" : "THEIR TURN";
    if (B.sweepAge >= SWEEP_FRAMES)
        return;
    in = t * 4 > 1 ? 1 : t * 4;
    in = 1 - (1 - in) * (1 - in);
    out = t > 0.75f ? (t - 0.75f) * 4 : 0;
    out = out * out;
    w = G_Measure(text, TF_NORMAL);
    x = (int)(-w - 20 + (120 + w / 2 + 20) * in + 300 * out) - w / 2;
    band = (int)(22 * (t < 0.1f ? t / 0.1f : t > 0.9f ? (1 - t) / 0.1f : 1));
    G_FillA(0, cy - band / 2, 240, band, 0x081018, 170);
    G_FillA(0, cy - band / 2, 240, 1, color, 255);
    G_FillA(0, cy + band / 2 - 1, 240, 1, color, 255);
    if (band > 14)
        G_Text(x, cy - 7, G_WHITE, color, text, 0, TF_NORMAL);
}

// the opponent's spell, shown big a moment as it's cast
static void DrawBigCard(void)
{
    float t, in, w, h;
    if (B.bigAge >= BIG_FRAMES || B.bigCard < 0)
        return;
    t = (float)B.bigAge / BIG_FRAMES;
    in = t < 0.2f ? t / 0.2f : t > 0.8f ? (1 - t) / 0.2f : 1;
    w = 44 * (0.6f + 0.4f * in);
    h = 61 * (0.6f + 0.4f * in);
    G_FillA(0, 0, 240, 160, 0x000000, (unsigned)(90 * in));
    G_FillA((int)(120 - w / 2) + 2, (int)(70 - h / 2) + 3, (int)w, (int)h, 0x000000, (unsigned)(90 * in));
    MtgGfx_DrawCard(B.bigCard, 120 - w / 2, 70 - h / 2, w, h, 0, (unsigned)(255 * in),
                    t < 0.3f ? (unsigned)(160 * (1 - t / 0.3f)) : 0);
    if (in > 0.98f) {
        char name[64];
        Mtg_PlainText(name, sizeof(name), Mtg_Card(B.bigCard)->name);
        G_Banner(120, 70 + 30 + 10, name, 0xe85050, 1, 255);
    }
}

static void DrawRow(int r)
{
    int i, lift = r == ROW_HAND;
    for (i = 0; i < B.itemCount[r]; i++) {
        int obj = B.items[r][i].obj;
        float x, y;
        DrawPos(obj, &B.rect[obj], &x, &y);
        if (lift && B.mode == MODE_TARGET && B.targetCount && B.duel.query.actions[B.targets[0]].obj == obj)
            continue;   // (the card being played is drawn last, held over the table)
        DrawCard(obj, &B.rect[obj], B.items[r][i].count, x, y);
    }
}

// what an ability says (its trigger's words, or its spell's), CARDNAME as its name
static void AbilityWords(char *out, unsigned cap, int source, int ability, int trigger)
{
    const char *desc = NULL;
    char raw[256], name[64];
    unsigned n = 0;
    const char *p;
    if (trigger != 0xFFFF)
        desc = Mtg_TriggerParam(&gMtgTriggers[trigger], "TriggerDescription");
    if (!desc && ability != 0xFFFF)
        desc = Mtg_Param(&gMtgAbilities[ability], "SpellDescription");
    out[0] = 0;
    if (!desc)
        return;
    Name(name, sizeof(name), source);
    for (p = desc; *p && n + 1 < sizeof(raw); p++) {
        if (strncmp(p, "CARDNAME", 8) == 0) {
            n += (unsigned)snprintf(raw + n, sizeof(raw) - n, "%s", name);
            if (n >= sizeof(raw))
                n = sizeof(raw) - 1;
            p += 7;
        } else
            raw[n++] = *p;
    }
    raw[n] = 0;
    Mtg_PlainText(out, cap, raw);
}

static char sChooseText[200];
static const char *ChooseText(void)
{
    const MtgQuery *q = &B.duel.query;
    char name[64], cost[24], words[160];
    Name(name, sizeof(name), q->source);
    AbilityWords(words, sizeof(words), q->source, q->ability, B.duel.res.trigger);
    if (q->choice == MTG_CHOICE_PAY) {
        CostText(cost, sizeof(cost), &q->pay);
        snprintf(sChooseText, sizeof(sChooseText), "%s: pay %s?  %s", name, cost[0] ? cost : "0", words);
    } else
        snprintf(sChooseText, sizeof(sChooseText), "%s: %s", name, words[0] ? words : "do it?");
    return sChooseText;
}

static const char *PickVerb(void)
{
    const MtgQuery *q = &B.duel.query;
    return q->forCost == 1 || (q->ability != 0xFFFF && gMtgAbilities[q->ability].api == MTG_API_SACRIFICE) ? "Sacrifice"
                                                                                                      : "Choose";
}

// the card asking (a yes / no, a payment): big at the left, as the opponent's spells show
static void AskedCard(int obj)
{
    if (obj == MTG_NO_OBJ || obj >= MTG_MAX_OBJS)
        return;
    G_FillA(0, 0, 240, 160, 0x000000, 90);
    G_FillA(62, 44, 44, 61, 0x000000, 90);
    MtgGfx_DrawCard(B.duel.objs[obj].def, 60, 41, 44, 61, 0, 255, 0);
}

// a target in a graveyard: the card big over the middle, IN YOUR / THEIR GRAVEYARD under it
static void GraveTarget(int obj)
{
    char name[64];
    int mine = B.duel.objs[obj].owner == ME;
    G_FillA(0, 0, 240, 160, 0x000000, 110);
    G_FillA(100, 43, 44, 61, 0x000000, 90);
    MtgGfx_DrawCard(B.duel.objs[obj].def, 98, 40, 44, 61, 0, 255, 0);
    Name(name, sizeof(name), obj);
    G_Banner(120, 111, name, mine ? 0x3878f0 : 0xe85050, 1, 255);
    G_TextAlign(120, 122, TA_CENTER, G_WHITE, 0x101820, mine ? "IN YOUR GRAVEYARD" : "IN THEIR GRAVEYARD", TF_NARROW);
}

void MtgBoard_Draw(void)
{
    SDL_Renderer *renderer = G_R();
    SDL_Texture *canvas = SDL_GetRenderTarget(renderer);
    int r, i;
    if (!B.active)
        return;
    BuildRows();
    Layout();
    // the mat and everything on it go into a layer taller than the screen, shown from Pan() down
    if (sLayerOwner != renderer)
        sLayer = NULL, sMat = NULL;
    if (!sLayer) {
        sLayer = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 240, LAYER_H);
        sLayerOwner = renderer;
        if (sLayer) {
            SDL_SetTextureBlendMode(sLayer, SDL_BLENDMODE_BLEND);
            SDL_SetTextureScaleMode(sLayer, SDL_ScaleModeNearest);
        }
    }
    if (sLayer) {
        SDL_SetRenderTarget(renderer, sLayer);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);
    }
    if (!sMat)
        BuildMat();
    if (sMat) {
        SDL_Rect whole = {0, 0, 240, LAYER_H};
        SDL_RenderCopy(renderer, sMat, NULL, &whole);
    }
    Labels();
    // the piles, the life plates, the opponent's hand
    {
        uint16_t list[MTG_MAX_OBJS];
        int n;
        DrawPile(PileRect(PILE_OPP_LIBRARY), B.view.players[THEM].libraryCount, 1, 0, 1);
        DrawPile(PileRect(PILE_MY_LIBRARY), B.view.players[ME].libraryCount, 1, 0, 0);
        n = Graveyard(THEM, list);
        DrawPile(PileRect(PILE_OPP_GRAVE), n, 0, n ? list[0] : 0, 1);
        n = Graveyard(ME, list);
        DrawPile(PileRect(PILE_MY_GRAVE), n, 0, n ? list[0] : 0, 0);
    }
    DrawLife(THEM);
    DrawLife(ME);
    DrawOppHand();
    PhaseBar();
    for (r = 0; r < ROW_HAND; r++)
        DrawRow(r);
    // the attackers each blocker blocks
    for (i = 0; i < B.view.objCount; i++) {
        const MtgObj *o = &B.view.objs[i];
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
                G_RoundFrame(a->x - 1, a->y - 1, a->w + 2, a->h + 2, 0x4080f0, 255);
            }
    }
    if (B.mode == MODE_ATTACK || B.mode == MODE_TARGET) {   // (the spaces that can be chosen: lit, as on the POKeMON table)
        const MtgQuery *q = &B.duel.query;
        for (i = 0; i < (B.mode == MODE_ATTACK ? q->candidateCount : B.targetCount); i++) {
            Rect a;
            int chosen = 0;
            if (B.mode == MODE_ATTACK) {
                a = B.rect[q->candidates[i]];
                chosen = (B.attack >> i) & 1;
                if (chosen)
                    a.y -= 5;
            } else {
                const MtgTarget *t = &q->actions[B.targets[i]].target;
                if (t->kind == MTG_TGT_OBJ)
                    a = B.rect[t->id];
                else if (t->kind == MTG_TGT_PLAYER)
                    a = B.playerRect[t->id];
                else
                    continue;
            }
            if (!a.on)
                continue;
            if (chosen) {   // (an attacker: framed red, an arrow at the opponent, ATTACK over it)
                Rect l = LifeRect(THEM);
                int x0 = a.x + a.w / 2, y0 = a.y, x1 = l.x + l.w / 2, y1 = l.y + l.h;
                G_RoundFrame(a.x - 1, a.y - 1, a.w + 2, a.h + 2, 0xf04030, 255);
                G_RoundFrame(a.x - 2, a.y - 2, a.w + 4, a.h + 4, 0xf04030, 140);
                G_Line(x0, y0, x1, y1, 0xf04030, 170);
                G_Line(x0 + 1, y0, x1 + 1, y1, 0xf04030, 110);
                if (q->candidates[i] != Focused()) {   // (the one under the cursor: its tooltip says so)
                    G_RoundFill(a.x + a.w / 2 - 15, a.y - 11, 30, 9, 0xc02818, 240);
                    G_TextAlign(a.x + a.w / 2, a.y - 13, TA_CENTER, G_WHITE, 0x601008, "ATTACK", TF_NARROW);
                }
            } else {
                G_RoundFill(a.x - 1, a.y - 1, a.w + 2, a.h + 2, 0xf8f8a0, 60 + (unsigned)(40 * sinf(B.frame * 0.15f)));
                G_RoundFrame(a.x - 2, a.y - 2, a.w + 4, a.h + 4, 0xf8f080, 255);
            }
        }
    }
    if (B.mode == MODE_ATTACK || B.mode == MODE_BLOCK) {   // (who can't be chosen: dimmed)
        const MtgQuery *q = &B.duel.query;
        for (i = 0; i < B.itemCount[ROW_MY_PERMS]; i++) {
            int obj = B.items[ROW_MY_PERMS][i].obj;
            const Rect *a = &B.rect[obj];
            if (ObjIndex(q->candidates, q->candidateCount, obj) < 0 && a->on)
                G_FillA(a->x, a->y, a->w, a->h, 0x000000, 120);
        }
    }
    if (B.mode == MODE_BLOCK || B.mode == MODE_BLOCK_PICK) {   // (who could block: lit; the attackers it could block)
        const MtgQuery *q = &B.duel.query;
        for (i = 0; i < q->candidateCount; i++) {
            Rect a = B.rect[q->candidates[i]];
            if (!a.on || B.block[i] >= 0 || B.mode == MODE_BLOCK_PICK)
                continue;
            G_RoundFill(a.x - 1, a.y - 1, a.w + 2, a.h + 2, 0xa0c8f8, 50 + (unsigned)(40 * sinf(B.frame * 0.15f)));
            G_RoundFrame(a.x - 2, a.y - 2, a.w + 4, a.h + 4, 0x80b0f8, 255);
        }
        if (B.mode == MODE_BLOCK_PICK)
            for (i = 0; i < q->attackerCount; i++)
                if (Mtg_CanBlock(&B.duel, q->candidates[B.blockFrom], q->attackers[i])) {
                    Rect a = B.rect[q->attackers[i]];
                    if (!a.on)
                        continue;
                    G_RoundFill(a.x - 1, a.y - 1, a.w + 2, a.h + 2, 0xf8f8a0, 60 + (unsigned)(40 * sinf(B.frame * 0.15f)));
                    G_RoundFrame(a.x - 2, a.y - 2, a.w + 4, a.h + 4, 0xf8f080, 255);
                }
    }
    if (B.view.stackCount)
        StackCards();
    else
        NewsLine();
    DrawPlay();
    DrawHandBar();
    DrawRow(ROW_HAND);
    if (B.mode == MODE_TARGET && B.targetCount) {   // the card being played, held up
        int obj = B.duel.query.actions[B.targets[0]].obj;
        if (B.rect[obj].on && B.view.objs[obj].zone == MTG_ZONE_HAND)
            DrawCard(obj, &B.rect[obj], 1, B.rect[obj].x, BarY() + Pan() - 22);
    }
    // the cursor and the tooltip
    if (B.mode == MODE_PICK) {   // the permanents that can be picked: lit; the cursor on one
        const MtgQuery *q = &B.duel.query;
        for (i = 0; i < q->candidateCount; i++) {
            Rect a = B.rect[q->candidates[i]];
            if (!a.on)
                continue;
            G_RoundFill(a.x - 1, a.y - 1, a.w + 2, a.h + 2, 0xf8a0a0, 50 + (unsigned)(40 * sinf(B.frame * 0.15f)));
            G_RoundFrame(a.x - 2, a.y - 2, a.w + 4, a.h + 4, 0xf08070, 255);
        }
        if (B.targetCursor < q->candidateCount && B.rect[q->candidates[B.targetCursor]].on) {
            Rect t = B.rect[q->candidates[B.targetCursor]];
            G_Focus(t.x, t.y, t.w, t.h, 0xf8f070, B.frame);
        }
    } else if (B.mode == MODE_TARGET || B.mode == MODE_BLOCK_PICK) {
        Rect t = MakeRect(0, 0, 0, 0);
        if (B.mode == MODE_TARGET) {
            const MtgAction *a = &B.duel.query.actions[B.targets[B.targetCursor]];
            t = a->target.kind == MTG_TGT_PLAYER ? B.playerRect[a->target.id] : B.rect[a->target.id];
            if (a->target.kind == MTG_TGT_STACK)
                t = MakeRect(CENTER_X - 11, CENTER_Y - 33, CARD_W, CARD_H);
            else if (a->target.kind == MTG_TGT_OBJ && B.duel.objs[a->target.id].zone == MTG_ZONE_GRAVEYARD)
                t = PileRect(B.duel.objs[a->target.id].owner == ME ? PILE_MY_GRAVE : PILE_OPP_GRAVE);
        } else
            t = B.rect[B.duel.query.attackers[B.blockCursor]];
        if (t.on && t.w)
            G_Focus(t.x, t.y, t.w, t.h, 0xf8f070, B.frame);
    } else if (B.mode != MODE_RESULT && B.mode != MODE_VIEW && B.mode != MODE_CONCEDE && B.mode != MODE_CHOOSE
               && ShowFocus()) {
        Rect f = FocusRect();
        if (f.w) {
            G_Focus(f.x, f.y, f.w, f.h, G_WHITE, B.frame);
            if (B.mode == MODE_NORMAL || B.mode == MODE_ATTACK || B.mode == MODE_BLOCK || B.mode == MODE_DISCARD)
                Tooltip();
        }
    }
    if (sLayer) {
        SDL_Rect dst = {0, -(int)(Pan() + 0.5f), 240, LAYER_H};
        SDL_SetRenderTarget(renderer, canvas);
        SDL_RenderCopy(renderer, sLayer, NULL, &dst);
    }
    // over the screen: the menus, what the player is asked, the sweep, the big card
    if (B.mode == MODE_MENU) {
        const char *items[8];
        Rect f = FocusRect();
        for (i = 0; i < B.menuCount; i++)
            items[i] = sMenuLabels[i];
        Menu(f.x + f.w + 4, f.y - (int)Pan() - 4, items, B.menuCount, B.menuCursor, 0);
    } else if (B.mode == MODE_TURN)
        Menu(160, 100, sTurnMenu, 2, B.menuCursor, B.view.active == ME ? 0 : 2);
    else if (B.mode == MODE_CHOOSE) {
        static const char *const sYesNo[2] = {"YES", "NO"}, *const sPay[2] = {"PAY", "DON'T PAY"};
        AskedCard(B.duel.query.source);
        Menu(166, 92, B.duel.query.choice == MTG_CHOICE_PAY ? sPay : sYesNo, 2, B.menuCursor, 0);
    }
    switch (B.mode) {
    case MODE_TARGET: {
        const MtgAction *a = &B.duel.query.actions[B.targets[B.targetCursor]];
        char line[160], name[64], who[64];
        if (a->target.kind == MTG_TGT_PLAYER)
            snprintf(name, sizeof(name), "%s", a->target.id == ME ? "YOU" : B.opp);
        else
            Name(name, sizeof(name), a->target.id);
        if (a->target.kind == MTG_TGT_OBJ && B.duel.objs[a->target.id].zone == MTG_ZONE_GRAVEYARD)
            GraveTarget(a->target.id);   // (a card in a graveyard: shown big, over its pile)
        if (B.trigTarget) {
            Name(who, sizeof(who), a->obj);
            snprintf(line, sizeof(line), "%s's ability: choose its target.  %s  {A} OK", who, name);
        } else
            snprintf(line, sizeof(line), "Target: %s  {A} OK  {B}", name);
        Prompt(line, 1);
        break;
    }
    case MODE_CHOOSE:
        Prompt(ChooseText(), 1);
        break;
    case MODE_PICK: {
        char line[120], name[64];
        const MtgQuery *q = &B.duel.query;
        Name(name, sizeof(name), q->candidates[B.targetCursor]);
        snprintf(line, sizeof(line), "%s which?  %s  {A} OK", PickVerb(), name);
        Prompt(line, 1);
        break;
    }
    case MODE_ATTACK: {
        char line[120];
        int n = __builtin_popcount(B.attack);
        if (n)
            snprintf(line, sizeof(line), "%d attacking: %d damage if not blocked  {START} ATTACK!", n, AttackPower());
        else
            snprintf(line, sizeof(line), "Choose your attackers  {A} CHOOSE  {SELECT} ALL  {START} NO ATTACK");
        Prompt(line, 1);
        break;
    }
    case MODE_BLOCK: {
        char line[120];
        const MtgQuery *q = &B.duel.query;
        int i, n = 0, incoming = 0;
        for (i = 0; i < q->candidateCount; i++)
            n += B.block[i] >= 0;
        for (i = 0; i < q->attackerCount; i++) {
            int k, blocked = 0, p = Mtg_Power(&B.duel, q->attackers[i]);
            for (k = 0; k < q->candidateCount; k++)
                blocked |= B.block[k] == i;
            if (!blocked)
                incoming += p > 0 ? p : 0;
        }
        if (n)
            snprintf(line, sizeof(line), "%d blocking: %d damage still coming  {START} DONE", n, incoming);
        else
            snprintf(line, sizeof(line), "%d damage coming! Block?  {A} CHOOSE  {START} NO BLOCK", incoming);
        Prompt(line, 0);
        break;
    }
    case MODE_BLOCK_PICK: Prompt("Block which attacker? (lit)  {A} OK  {B}", 0); break;
    case MODE_DISCARD:
        Prompt(B.duel.query.kind == MTG_Q_CHOOSE && B.duel.query.forCost == 2 ? "Discard a card to pay the cost.  {A}"
               : B.duel.query.kind == MTG_Q_CHOOSE ? "Discard a card of your choice.  {A}"
                                                 : "Too many cards in your hand: discard one.  {A}", 1);
        break;
    case MODE_CONCEDE: Prompt("Concede the duel?  {A} CONCEDE  {B} KEEP PLAYING", 0); break;
    default:
        if (B.mode == MODE_NORMAL && B.duel.query.kind == MTG_Q_PRIORITY && B.duel.query.player == ME && B.view.stackCount
            && B.view.stack[B.view.stackCount - 1].controller == THEM)
            Prompt("Answer it?  {START} LET IT RESOLVE", 1);
        break;
    }
    DrawSweep();
    DrawBigCard();
    Toast();
    if (B.mode == MODE_VIEW)
        DrawView();
    if (B.inspect && B.inspectObj != MTG_NO_OBJ)
        InspectWindow(B.inspectObj);
    if (B.mode == MODE_RESULT)
        DrawResult();
}

// ---- what happened, played out: the table shows B.view, which trails the rules' duel by one event at a time. Each
// event is acted out on the table (a card flying from the hand or the library, a tap turning, an attacker lunging,
// damage flying to its target with its number, a card shattering as it dies, life ticking, the stack resolving) and
// told in the middle line; nothing else happens (the computer waits, the buttons wait) until it's over. Once every
// event has played, the view is the truth again.
static Rect StackSpot(void) { return MakeRect(CENTER_X - 11, CENTER_Y - 33, CARD_W, CARD_H); }
static int Frames(int n) { return Fast() ? (n * 3 + 4) / 5 : n; }
int MtgBoard_Busy(void) { return B.active && (B.evOn || B.evShown < B.duel.eventCount); }

static void Floater(int x, int y, int value, unsigned color)
{
    int i, best = 0;
    for (i = 0; i < FLOATERS; i++)
        if (B.floatAge[i] >= 40) {
            best = i;
            break;
        } else if (B.floatAge[i] > B.floatAge[best])
            best = i;
    B.floatX[best] = (int16_t)x, B.floatY[best] = (int16_t)y, B.floatValue[best] = (int16_t)value;
    B.floatColor[best] = color, B.floatAge[best] = 0;
}
static Rect ObjRect(int obj)   // where an object is drawn now: its card on the table, the stack's spot, or none
{
    if (obj >= 0 && obj < MTG_MAX_OBJS && B.rect[obj].on)
        return B.rect[obj];
    if (obj >= 0 && obj < MTG_MAX_OBJS && B.view.objs[obj].zone == MTG_ZONE_STACK)
        return StackSpot();
    return MakeRect(0, 0, 0, 0);
}
static void ViewCopyObj(int obj, int zone)
{
    if (obj < 0 || obj >= MTG_MAX_OBJS)
        return;
    B.view.objs[obj] = B.duel.objs[obj];
    B.view.objs[obj].zone = (uint8_t)zone;
    if (obj >= B.view.objCount)
        B.view.objCount = obj + 1;
}
static void StackRemove(int source)
{
    int i;
    for (i = B.view.stackCount - 1; i >= 0; i--)
        if (B.view.stack[i].source == source) {
            memmove(&B.view.stack[i], &B.view.stack[i + 1], sizeof(B.view.stack[0]) * (size_t)(B.view.stackCount - 1 - i));
            B.view.stackCount--;
            return;
        }
}

// an event begins: the view takes what can change at once, the look starts; returns its frames
static int StartEvent(const MtgEvent *e)
{
    MtgDuel *v = &B.view;
    char a[64], b[64];
    int obj = e->a;
    switch (e->kind) {
    case MTG_EV_TURN:
        v->active = (uint8_t)e->a, v->turn = (uint16_t)e->b;
        Ticker("Turn %d: %s.", e->b, e->a == ME ? "your turn" : "their turn");
        B.sweepAge = 0;
        B.sweepMine = e->a == ME;
        PlaySE(e->a == ME ? SE_BALL_TRAY_ENTER : SE_BALL_TRAY_EXIT);
        B.refocusTurn = e->a == ME ? 1 : 2;   // (once the table is still: the hand on yours, the field on theirs)
        if (e->a != ME)
            B.trayWanted = 0;
        return SWEEP_FRAMES * 3 / 4;
    case MTG_EV_STEP:
        v->step = (uint8_t)e->a, v->active = (uint8_t)e->b;
        return Frames(3);   // (the step bar ticks along)
    case MTG_EV_ZONE:
        if (e->b == MTG_ZONE_BATTLEFIELD) {   // leaving the table: it breaks up (or fades) where it stands, then goes
            Rect r = ObjRect(obj);
            B.leaveRect = r;
            B.leaveColor = MtgGfx_FrameColor(v->objs[obj].def, 0);
            Name(a, sizeof(a), obj);
            Ticker("%s %s.", a, e->c == MTG_ZONE_GRAVEYARD ? ((Def(obj)->types & MTG_T_CREATURE) ? "died" : "was destroyed")
                                : e->c == MTG_ZONE_EXILE ? "was exiled" : e->c == MTG_ZONE_HAND ? "went back to a hand"
                                : "left");
            if (e->c == MTG_ZONE_GRAVEYARD)
                PlaySE(SE_FAINT);
            return Frames(20);
        }
        if (e->b == MTG_ZONE_LIBRARY && v->players[v->objs[obj].owner].libraryCount)
            v->players[v->objs[obj].owner].libraryCount--;
        // where it comes from when it shows up: the opponent's hand, the stack, their library
        if (e->b == MTG_ZONE_NONE)   // a token: it appears out of the spell or ability that made it
            B.spawn[obj] = StackSpot();
        else if (B.duel.objs[obj].controller == THEM && e->b == MTG_ZONE_HAND)
            B.spawn[obj] = sOppHand;
        else if (e->b == MTG_ZONE_STACK)
            B.spawn[obj] = StackSpot();
        else
            B.spawn[obj] = PileRect(B.duel.objs[obj].owner == ME ? PILE_MY_LIBRARY : PILE_OPP_LIBRARY);
        if (e->b == MTG_ZONE_HAND && e->c == MTG_ZONE_STACK) {   // a spell cast: CAST flies it
            ViewCopyObj(obj, e->c);
            return 0;
        }
        if (e->b == MTG_ZONE_LIBRARY && e->c == MTG_ZONE_HAND) {
            ViewCopyObj(obj, e->c);
            if (B.duel.objs[obj].owner == THEM) {   // their draw: a card back flies to their hand
                B.flyFrom = PileRect(PILE_OPP_LIBRARY), B.flyTo = sOppHand, B.flyT = B.flyLife = Frames(12);
                return Frames(12);
            }
            PlaySE(SE_RG_CARD_FLIP);
            B.showHand = Frames(50);   // (your draw: the hand comes up a moment to show it)
            return Frames(10);
        }
        ViewCopyObj(obj, e->c);
        return e->c == MTG_ZONE_BATTLEFIELD ? Frames(12) : Frames(6);
    case MTG_EV_DRAW:
        return 0;
    case MTG_EV_LAND:
        Name(a, sizeof(a), obj);
        Ticker("%s played %s.", Who(e->b), a);
        B.flash[obj] = 14;
        PlaySE(SE_RG_CARD_FLIP);
        if (e->b == THEM && !B.autoplay) {   // their land: shown big too (you see what mana they have coming)
            B.bigCard = v->objs[obj].def;
            B.bigAge = BIG_FRAMES / 3;   // (a shorter look than a spell's)
            B.flash[obj] = 30;
            return BIG_FRAMES - BIG_FRAMES / 3 + Frames(6);
        }
        return Frames(14);
    case MTG_EV_CAST:
    case MTG_EV_ACTIVATE: {
        MtgStackEntry s;
        int theirs = B.duel.objs[obj].controller == THEM;
        memset(&s, 0, sizeof(s));
        s.isSpell = e->kind == MTG_EV_CAST;
        s.controller = B.duel.objs[obj].controller;
        s.source = (uint16_t)obj;
        s.target.kind = (uint8_t)e->b, s.target.id = (uint16_t)e->c;
        if (v->stackCount < MTG_MAX_STACK)
            v->stack[v->stackCount++] = s;
        Name(a, sizeof(a), obj);
        if (e->b == MTG_TGT_OBJ || e->b == MTG_TGT_STACK) {
            Name(b, sizeof(b), e->c);
            Ticker("%s %s %s on %s.", Who(s.controller), s.isSpell ? "cast" : "used", a, b);
        } else if (e->b == MTG_TGT_PLAYER)
            Ticker("%s %s %s on %s.", Who(s.controller), s.isSpell ? "cast" : "used", a, e->c == ME ? "you" : B.opp);
        else
            Ticker("%s %s %s.", Who(s.controller), s.isSpell ? "cast" : "used", a);
        // it flies from where it was to the stack's spot (an ability: a flash on its card, then the flight)
        B.castFrom = s.isSpell ? (B.last[obj].on ? B.last[obj] : sOppHand) : ObjRect(obj);
        if (!s.isSpell)
            B.flash[obj] = 12;
        PlaySE(SE_RG_CARD_FLIPPING);
        if (s.isSpell && theirs && !B.autoplay) {   // their spell: shown big first
            B.bigCard = v->objs[obj].def;
            B.bigAge = 0;
            B.castT = B.castLife = Frames(14);
            return BIG_FRAMES + Frames(14);
        }
        B.castT = B.castLife = Frames(14);
        return Frames(16);
    }
    case MTG_EV_TRIGGER: {   // a triggered ability: its card lights up, it flies to the stack (theirs: shown big first)
        MtgStackEntry s;
        int theirs = B.duel.objs[obj].controller == THEM && B.duel.objs[obj].zone == MTG_ZONE_BATTLEFIELD;
        memset(&s, 0, sizeof(s));
        s.controller = B.duel.objs[obj].controller;
        s.source = (uint16_t)obj;
        s.ability = 0xFFFF;
        s.trigger = 0xFFFF;
        s.target.kind = (uint8_t)e->b, s.target.id = (uint16_t)e->c;
        {
            int k;   // (the entry itself: its ability and trigger, for the words)
            for (k = B.duel.stackCount - 1; k >= 0; k--)
                if (B.duel.stack[k].source == obj && B.duel.stack[k].trigger != 0xFFFF) {
                    s = B.duel.stack[k];
                    break;
                }
        }
        if (v->stackCount < MTG_MAX_STACK)
            v->stack[v->stackCount++] = s;
        Name(a, sizeof(a), obj);
        if (e->b == MTG_TGT_OBJ) {
            Name(b, sizeof(b), e->c);
            Ticker("%s's ability: on %s.", a, b);
        } else if (e->b == MTG_TGT_PLAYER)
            Ticker("%s's ability: on %s.", a, e->c == ME ? "you" : B.opp);
        else
            Ticker("%s's ability triggers.", a);
        B.castFrom = ObjRect(obj);
        if (!B.castFrom.w)
            B.castFrom = PileRect(B.duel.objs[obj].owner == ME ? PILE_MY_GRAVE : PILE_OPP_GRAVE);
        B.flash[obj] = 16;
        PlaySE(SE_M_DETECT);
        B.castT = B.castLife = Frames(14);
        if (theirs && !B.autoplay) {
            B.bigCard = v->objs[obj].def;
            B.bigAge = 0;
            return BIG_FRAMES + Frames(14);
        }
        return Frames(22);
    }
    case MTG_EV_COUNTERS:
        if (obj >= 0 && obj < MTG_MAX_OBJS) {
            Rect r = ObjRect(obj);
            v->objs[obj].counters11 = e->c;
            B.flash[obj] = 14;
            if (r.w)
                Floater(r.x + r.w / 2, r.y, e->b, e->b > 0 ? 0x60e070 : 0xff5040);
            Name(a, sizeof(a), obj);
            Ticker(e->b > 0 ? "%s gets %d +1/+1 counter%s." : "%s gets %d -1/-1 counter%s.", a, e->b > 0 ? e->b : -e->b,
                   (e->b == 1 || e->b == -1) ? "" : "s");
            PlaySE(e->b > 0 ? SE_USE_ITEM : SE_M_DETECT);
        }
        return Frames(16);
    case MTG_EV_RESOLVE:
        B.resolveT = B.resolveLife = Frames(14);
        return Frames(14);
    case MTG_EV_COUNTERED:
    case MTG_EV_FIZZLE:
        Name(a, sizeof(a), obj);
        Ticker(e->kind == MTG_EV_COUNTERED ? "%s was countered." : "%s lost its target.", a);
        if (e->kind == MTG_EV_COUNTERED && B.duel.objs[obj].owner == THEM && !B.autoplay)
            MP_Achieve_Unlock("mtgcounter");
        B.leaveRect = StackSpot();
        B.leaveColor = MtgGfx_FrameColor(v->objs[obj].def, 0);
        PlaySE(SE_BALL_OPEN);
        return Frames(18);
    case MTG_EV_REGENERATE:   // a shield given, or used: tapped, out of combat, healed
        if (obj >= 0 && obj < MTG_MAX_OBJS) {
            Rect r = ObjRect(obj);
            v->objs[obj].regen = (uint8_t)e->c;
            B.flash[obj] = 16;
            Name(a, sizeof(a), obj);
            if (e->b) {
                Ticker("%s will regenerate.", a);
                PlaySE(SE_M_HARDEN);
                return Frames(14);
            }
            v->objs[obj].damage = 0;
            v->objs[obj].attacking = 0;
            v->objs[obj].blocking = MTG_NO_OBJ;
            if (r.w)
                G_FillAdd(r.x, r.y, r.w, r.h, 0x60f0a0, 120);
            Ticker("%s regenerates!", a);
            PlaySE(SE_SHINY);
        }
        return Frames(22);
    case MTG_EV_TAP:
        if (obj >= 0 && obj < MTG_MAX_OBJS) {
            v->objs[obj].tapped = (uint8_t)e->b;
            B.tapT[obj] = (uint8_t)Frames(6);
        }
        return Frames(5);
    case MTG_EV_ATTACK:
        if (obj >= 0 && obj < MTG_MAX_OBJS) {
            v->objs[obj].attacking = 1;
            B.lunge[obj] = (uint8_t)Frames(14);
        }
        Name(a, sizeof(a), obj);
        Ticker("%s attacks!", a);
        PlaySE(SE_M_TAKE_DOWN);
        if (obj >= 0 && obj < MTG_MAX_OBJS && B.duel.objs[obj].controller == THEM) {
            B.flash[obj] = (uint8_t)Frames(24);   // (each of their attackers, one by one, so you can see who's coming)
            return Frames(24);
        }
        return Frames(14);
    case MTG_EV_BLOCK:
        if (obj >= 0 && obj < MTG_MAX_OBJS) {
            v->objs[obj].blocking = (uint16_t)e->b;
            B.flash[obj] = 12;
        }
        Name(a, sizeof(a), obj);
        Name(b, sizeof(b), e->b);
        Ticker("%s blocks %s.", a, b);
        PlaySE(SE_M_DETECT);
        return obj >= 0 && obj < MTG_MAX_OBJS && B.duel.objs[obj].controller == THEM ? Frames(24) : Frames(16);
    case MTG_EV_DAMAGE: {
        Rect from = ObjRect(obj), to = e->b < 0 ? LifeRect(-1 - e->b) : ObjRect(e->b);
        if (!from.w)
            from = StackSpot();
        B.flyFrom = from, B.flyTo = to, B.flyT = B.flyLife = Frames(14), B.flyBall = 1;
        B.flyColor = obj >= 0 && obj < MTG_MAX_OBJS ? MtgGfx_FrameColor(v->objs[obj].def, 1) : 0xffe080;
        if (obj >= 0 && obj < MTG_MAX_OBJS && B.rect[obj].on)
            B.lunge[obj] = (uint8_t)Frames(12);
        if (obj >= 0 && obj < MTG_MAX_OBJS && e->b < 0 && e->c >= 3) {
            Name(a, sizeof(a), obj);
            Ticker("%s: %d damage to %s!", a, e->c, e->b == -1 - ME ? "you" : B.opp);
        }
        return Frames(16);
    }
    case MTG_EV_LIFE:
        return Frames(12);
    }
    return 0;
}

// an event ends: the rest of what it changes
static void EndEvent(const MtgEvent *e)
{
    MtgDuel *v = &B.view;
    int obj = e->a;
    switch (e->kind) {
    case MTG_EV_ZONE:
        if (e->b == MTG_ZONE_BATTLEFIELD) {
            ViewCopyObj(obj, e->c);
            B.leaveRect.w = 0;
        }
        break;
    case MTG_EV_RESOLVE:
        if (v->stackCount)
            v->stackCount--;
        break;
    case MTG_EV_COUNTERED:
    case MTG_EV_FIZZLE:
        StackRemove(obj);
        B.leaveRect.w = 0;
        break;
    case MTG_EV_DAMAGE: {
        Rect to = e->b < 0 ? LifeRect(-1 - e->b) : ObjRect(e->b);
        if (e->b >= 0 && e->b < MTG_MAX_OBJS) {
            v->objs[e->b].damage += e->c;
            B.flash[e->b] = 10;
            Floater(to.x + to.w / 2, to.y, -e->c, 0xff5040);
        }
        B.flyBall = 0;
        PlaySE(SE_EFFECTIVE);
        break;
    }
    case MTG_EV_LIFE: {
        Rect l = LifeRect(e->a);
        v->players[e->a].life = e->c;
        B.lifeFlash[e->a] = 16;
        Floater(l.x + l.w / 2, l.y + (e->a == ME ? -2 : l.h + 2), e->b, e->b > 0 ? 0x60e070 : 0xff5040);
        if (e->b > 0)
            PlaySE(SE_USE_ITEM);
        break;
    }
    }
}

// a frame of the play: the event under way goes on, ends, or the next one starts
static void Play(void)
{
    int i;
    for (i = 0; i < MTG_MAX_OBJS; i++) {
        if (B.tapT[i])
            B.tapT[i]--;
        if (B.lunge[i])
            B.lunge[i]--;
        if (B.flash[i])
            B.flash[i]--;
    }
    for (i = 0; i < FLOATERS; i++)
        if (B.floatAge[i] < 40)
            B.floatAge[i]++;
    for (i = 0; i < 2; i++)
        if (B.lifeFlash[i])
            B.lifeFlash[i]--;
    if (B.teleT > 0)
        B.teleT--;
    if (B.castT)
        B.castT--;
    if (B.resolveT)
        B.resolveT--;
    if (B.flyT)
        B.flyT--;
    if (B.autoplay) {   // (the test harness at speed: no show)
        B.evShown = B.duel.eventCount;
        B.evOn = 0;
    }
    for (;;) {
        if (B.evOn) {
            if (--B.evLeft > 0)
                return;
            EndEvent(&B.ev);
            B.evOn = 0;
            B.evShown++;
        }
        if (B.evShown >= B.duel.eventCount) {
            memcpy(&B.view, &B.duel, sizeof(B.view));   // caught up: the view is the truth again
            return;
        }
        {
            const MtgEvent *e = Mtg_EventAt(&B.duel, B.evShown);
            if (!e) {   // (too far behind for the ring: catch up at once)
                B.evShown = B.duel.eventCount;
                continue;
            }
            B.ev = *e;
            B.evOn = 1;
            B.evLeft = StartEvent(&B.ev);
            if (B.evLeft > 0)
                return;   // (it shows: one at a time)
        }
    }
}

// the play's looks, drawn on the layer: the card that leaves, the flight, the numbers
static void DrawPlay(void)
{
    int i;
    if (B.evOn && B.leaveRect.w && (B.ev.kind == MTG_EV_ZONE || B.ev.kind == MTG_EV_COUNTERED
                                    || B.ev.kind == MTG_EV_FIZZLE)) {
        int life = B.ev.kind == MTG_EV_ZONE ? Frames(20) : Frames(18);
        float k = 1.0f - (float)B.evLeft / (float)(life > 0 ? life : 1);
        Rect r = B.leaveRect;
        for (i = 0; i < 12; i++) {   // pieces falling away
            int px = r.x + (i % 3) * r.w / 3, py = r.y + (i / 3) * r.h / 4;
            float dx = ((i % 3) - 1) * 12 * k + (i & 1 ? 2 : -2) * k, dy = k * k * 26 - 6 * k;
            G_FillA(px + (int)dx, py + (int)dy, r.w / 3 + 1, r.h / 4 + 1, i & 1 ? B.leaveColor : G_Mix(B.leaveColor, 0xffffff, 0.4f),
                    (unsigned)(240 * (1 - k)));
        }
        if (k < 0.3f)
            G_FillAdd(r.x, r.y, r.w, r.h, 0xffffff, (unsigned)(200 * (1 - k / 0.3f)));
    }
    if (B.flyT && B.flyLife) {
        float k = 1.0f - (float)B.flyT / (float)B.flyLife, x, y;
        float x0 = B.flyFrom.x + B.flyFrom.w / 2.0f, y0 = B.flyFrom.y + B.flyFrom.h / 2.0f;
        float x1 = B.flyTo.x + B.flyTo.w / 2.0f, y1 = B.flyTo.y + B.flyTo.h / 2.0f;
        k = k * k * (3 - 2 * k);
        x = x0 + (x1 - x0) * k;
        y = y0 + (y1 - y0) * k - sinf(k * 3.14159f) * 10;
        if (B.flyBall) {   // damage: a glowing shot with a trail
            int t;
            for (t = 4; t >= 0; t--) {
                float kk = k - t * 0.05f, tx, ty;
                if (kk < 0)
                    continue;
                tx = x0 + (x1 - x0) * kk;
                ty = y0 + (y1 - y0) * kk - sinf(kk * 3.14159f) * 10;
                G_FillAdd((int)tx - 2, (int)ty - 2, 5, 5, B.flyColor, (unsigned)(160 - t * 30));
            }
            G_Circle((int)x, (int)y, 4, 0xffffff, 230);
            G_FillAdd((int)x - 3, (int)y - 3, 7, 7, B.flyColor, 255);
            G_Fill((int)x - 1, (int)y - 1, 3, 3, 0xffffff);
        } else   // their draw: a card back on its way to their hand
            MtgGfx_DrawCard(MTG_GFX_BACK, x - 6, y - 8, 12, 17, 180, 255, 0);
    }
    for (i = 0; i < FLOATERS; i++)   // the numbers: up and away
        if (B.floatAge[i] < 40) {
            char n[12];
            int age = B.floatAge[i];
            unsigned alpha = age < 28 ? 255 : (unsigned)(255 * (40 - age) / 12);
            snprintf(n, sizeof(n), "%+d", B.floatValue[i]);
            G_DigitsOutlined(B.floatX[i] - G_DigitsWidth(n, 1, 1) / 2, B.floatY[i] - 4 - (age < 10 ? age : 10 + (age - 10) / 3),
                             n, 1, 1, B.floatColor[i], 0x101010, alpha);
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
    if (act->kind != MTG_ACT_PASS && B.duel.objs[act->obj].zone == MTG_ZONE_HAND && B.duel.objs[act->obj].owner == ME) {
        B.refocusHand = 1;   // (played from the hand: the next card to play, or the field when there's none)
        B.refocusCol = B.col[ROW_HAND];
    }
    memset(&a, 0, sizeof(a));
    a.action = (uint16_t)index;
    Answer(&a);
}

// Is there a card in the hand the player could play now (a land or a spell)? Its column, or -1
static int HandMove(int from)
{
    const MtgQuery *q = &B.duel.query;
    int i, k, best = -1, bestD = 1 << 30;
    if (q->kind != MTG_Q_PRIORITY || q->player != ME)
        return -1;
    BuildRows();
    for (i = 0; i < B.itemCount[ROW_HAND]; i++) {
        int obj = B.items[ROW_HAND][i].obj, d = i > from ? i - from : from - i;
        for (k = 1; k < q->actionCount; k++)
            if (q->actions[k].obj == obj && (q->actions[k].kind == MTG_ACT_LAND || q->actions[k].kind == MTG_ACT_CAST)) {
                if (d < bestD)
                    bestD = d, best = i;
                break;
            }
    }
    return best;
}
static void ToField(void)   // the cursor down onto your field, the tray down
{
    B.row = B.itemCount[ROW_MY_PERMS] ? ROW_MY_PERMS : ROW_MY_LANDS;
    B.pile = PILE_NONE;
    B.trayWanted = 0;
}
static void ToHand(int col)
{
    B.row = ROW_HAND;
    B.col[ROW_HAND] = col < 0 ? 0 : col;
    B.pile = PILE_NONE;
    B.trayWanted = 1;
}
// The tray and the cursor by what's going on (the POKeMON table's way), once the table is still and the player may act
static void Refocus(void)
{
    const MtgQuery *q = &B.duel.query;
    int col;
    if (q->player != ME || B.autoplay)
        return;
    if (B.refocusTurn) {
        if (B.refocusTurn == 1 && q->kind == MTG_Q_PRIORITY) {   // your turn: the hand, on a card you can play
            col = HandMove(0);
            ToHand(col >= 0 ? col : 0);
            MPNet_Log("mtg: refocus: your turn, the hand");
        } else if (B.refocusTurn == 2) {
            ToField();
            MPNet_Log("mtg: refocus: their turn, the field");
        }
        B.refocusTurn = 0;
    }
    if (B.refocusHand && q->kind == MTG_Q_PRIORITY) {   // a card just played from the hand
        B.refocusHand = 0;
        col = HandMove(B.refocusCol);
        if (col >= 0 && B.duel.active == ME)
            ToHand(col);
        else {
            ToField();
            MPNet_Log("mtg: refocus: nothing left to play, the field");
        }
    }
    // their spell on the stack and an answer in the hand: the tray comes up on it
    if (q->kind == MTG_Q_PRIORITY && B.duel.stackCount && B.duel.stack[B.duel.stackCount - 1].controller == THEM
        && B.answerFocused != (int)(B.duel.stackCount * 64 + B.duel.turn)) {
        B.answerFocused = (int)(B.duel.stackCount * 64 + B.duel.turn);
        col = HandMove(0);
        if (col >= 0) {
            ToHand(col);
            MPNet_Log("mtg: refocus: an answer in the hand");
        }
    }
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
                B.pile = PILE_NONE;
                B.trayWanted = r == ROW_HAND;
                MPNet_Log("mtg: focus %s", name);
                return;
            }
    MPNet_Log("mtg: no %s to focus", name);
}

// "mp mtg put PLAYER ZONE NAME" (zone: hand, field, grave): a card put in place on the table (the test harness)
void MtgBoard_DebugPut(int player, const char *zone, const char *name)
{
    int def = Mtg_CardByName(name), z;
    MtgObj *o;
    if (!B.active || def < 0 || B.duel.objCount >= MTG_MAX_OBJS || player < 0 || player > 1) {
        MPNet_Log("mtg: can't put %s", name);
        return;
    }
    z = strcmp(zone, "hand") == 0 ? MTG_ZONE_HAND : strcmp(zone, "grave") == 0 ? MTG_ZONE_GRAVEYARD : MTG_ZONE_BATTLEFIELD;
    o = &B.duel.objs[B.duel.objCount];
    memset(o, 0, sizeof(*o));
    o->def = (uint16_t)def;
    o->owner = o->controller = (uint8_t)player;
    o->zone = (uint8_t)z;
    o->blocking = MTG_NO_OBJ;
    o->attachedTo = MTG_NO_OBJ;
    o->seq = ++B.duel.seq;
    B.duel.objCount++;
    Mtg_RefreshQuery(&B.duel);
    if (!MtgBoard_Busy())
        memcpy(&B.view, &B.duel, sizeof(B.view));
    MPNet_Log("mtg: put %s", name);
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
    if (B.pile != PILE_NONE) {   // on a pile: back onto the table the way it came, or up and down the rows
        int mine = B.pile == PILE_MY_GRAVE || B.pile == PILE_MY_LIBRARY;
        if (onlyRow >= 0 || (pressed & (mine ? KEY_LEFT : KEY_RIGHT))) {
            B.pile = PILE_NONE;
            if (onlyRow >= 0)
                B.row = onlyRow;
            PlaySE(SE_SELECT);
            return;
        }
        if (pressed & (KEY_UP | KEY_DOWN)) {
            static const int above[4] = {PILE_MY_LIBRARY, PILE_MY_LIBRARY, PILE_OPP_GRAVE, PILE_OPP_GRAVE};
            static const int below[4] = {PILE_MY_GRAVE, PILE_MY_GRAVE, PILE_OPP_LIBRARY, PILE_OPP_LIBRARY};
            int to = pressed & KEY_UP ? above[B.pile] : below[B.pile];
            if (to != B.pile) {
                B.pile = to;
                PlaySE(SE_SELECT);
            }
        }
        return;
    }
    if (onlyRow < 0 && (pressed & (KEY_UP | KEY_DOWN))) {
        int dir = pressed & KEY_UP ? -1 : 1, n;
        for (n = 0, r = B.row + dir; n < ROW_COUNT && (r < 0 || r >= ROW_COUNT || !B.itemCount[r]); n++)
            r = (r + dir + ROW_COUNT) % ROW_COUNT;
        if (r >= 0 && r < ROW_COUNT && B.itemCount[r] && r != B.row) {
            B.row = r;
            B.trayWanted = r == ROW_HAND;   // (into the hand: the tray comes up; out of it: down)
            PlaySE(SE_SELECT);
        }
    }
    if (onlyRow >= 0)
        B.row = onlyRow;
    if (pressed & (KEY_LEFT | KEY_RIGHT)) {
        int n = B.itemCount[B.row], left = (pressed & KEY_LEFT) != 0;
        if (B.row == ROW_HAND) {   // (the hand goes round)
            if (n) {
                B.col[B.row] = (B.col[B.row] + (left ? n - 1 : 1)) % n;
                PlaySE(SE_SELECT);
            }
        } else if (onlyRow < 0 && !left && (B.row == ROW_MY_LANDS || B.row == ROW_MY_PERMS)
                   && (!n || B.col[B.row] >= n - 1)) {
            B.pile = B.row == ROW_MY_LANDS ? PILE_MY_GRAVE : PILE_MY_LIBRARY;   // past the row's end: the piles
            PlaySE(SE_SELECT);
        } else if (onlyRow < 0 && left && (B.row == ROW_OPP_LANDS || B.row == ROW_OPP_PERMS) && B.col[B.row] <= 0) {
            B.pile = B.row == ROW_OPP_LANDS ? PILE_OPP_GRAVE : PILE_OPP_LIBRARY;
            PlaySE(SE_SELECT);
        } else if (n) {
            int c = B.col[B.row] + (left ? -1 : 1);
            if (c >= 0 && c < n) {
                B.col[B.row] = c;
                PlaySE(SE_SELECT);
            }
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
    if (B.endTurn == d->turn && d->active == ME && !d->stackCount)   // (END TURN: on to the opponent's turn)
        return 1;
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

// A on a pile: CONCEDE on your library, LOOK at a graveyard; 1 when the cursor was on one
static int PileA(void)
{
    if (B.pile == PILE_NONE)
        return 0;
    if (B.pile == PILE_MY_LIBRARY) {
        PlaySE(SE_SELECT);
        B.mode = MODE_CONCEDE;
    } else if (B.pile == PILE_MY_GRAVE || B.pile == PILE_OPP_GRAVE) {
        B.viewPlayer = B.pile == PILE_MY_GRAVE ? ME : THEM;
        B.viewCount = Graveyard(B.viewPlayer, B.viewList);
        B.viewCursor = 0;
        if (!B.viewCount) {
            PlaySE(SE_FAILURE);
            Hold("There's nothing in it yet.");
        } else {
            PlaySE(SE_SELECT);
            B.mode = MODE_VIEW;
        }
    } else
        PlaySE(SE_FAILURE);
    return 1;
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
        if (pressed & KEY_B) {
            B.mode = MODE_NORMAL, PlaySE(SE_SELECT);
            if (B.row == ROW_HAND)   // (back to the card in the hand, the tray up again)
                B.trayWanted = 1;
        } else if (pressed & KEY_A) {
            PlaySE(SE_SELECT);
            AnswerAction(B.targets[B.targetCursor]);
        }
        return;
    }
    if (B.mode == MODE_TURN) {
        unsigned off = B.duel.active == ME ? 0 : 2;
        B.menuCursor = TcgUi_PopMenuMove(pressed, B.menuCursor, 2, off);
        if (pressed & KEY_B)
            B.mode = MODE_NORMAL, PlaySE(SE_SELECT);
        else if (pressed & KEY_A && !((off >> B.menuCursor) & 1)) {
            PlaySE(SE_SELECT);
            B.mode = MODE_NORMAL;
            if (B.menuCursor == 1) {
                B.endTurn = B.duel.turn;
                MPNet_Log("mtg: you: end the turn");
            }
            Pass();
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
    if (pressed & KEY_B && B.row == ROW_HAND && B.pile == PILE_NONE && B.tray > 0.5f) {   // B in the hand: down to the field
        PlaySE(SE_SELECT);
        ToField();
        return;
    }
    if (pressed & KEY_B) {   // the turn's menu: PASS, END TURN
        PlaySE(SE_SELECT);
        B.mode = MODE_TURN;
        B.menuCursor = 0;
        return;
    }
    if (pressed & KEY_A) {
        if (PileA())
            return;
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
    if (B.endTurn == B.duel.turn) {
        MtgAnswer a;
        memset(&a, 0, sizeof(a));
        Answer(&a);
        return;
    }
    if (B.mode != MODE_ATTACK) {
        B.mode = MODE_ATTACK;
        B.attack = 0;
        B.row = ROW_MY_PERMS;
        B.pile = PILE_NONE;
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
            Hold(WhyCantAttack(f));
        } else {
            B.attack ^= 1u << i;
            PlaySE(B.attack & (1u << i) ? SE_M_TAKE_DOWN : SE_SELECT);
        }
    }
    if ((pressed & KEY_B) && B.attack) {   // (B: no one, after all)
        B.attack = 0;
        PlaySE(SE_SELECT);
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
        B.pile = PILE_NONE;
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
            Hold(B.duel.objs[f].tapped ? "It's tapped: it can't block." : "It can't block now.");
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
    B.pile = PILE_NONE;
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

// a triggered ability's target (MTG_Q_TRIGGER_TARGET): the cursor goes round them; it must have one
static void HumanTriggerTarget(unsigned pressed)
{
    const MtgQuery *q = &B.duel.query;
    int i;
    YourMove("a trigger's target");
    if (B.mode != MODE_TARGET || !B.trigTarget) {
        B.mode = MODE_TARGET;
        B.trigTarget = 1;
        B.targetCount = 0;
        B.targetCursor = 0;
        for (i = 0; i < q->actionCount; i++)
            B.targets[B.targetCount++] = (uint16_t)i;
        for (i = 0; i < B.targetCount; i++) {   // (on one of theirs first, unless it helps: on one of yours)
            const MtgTarget *t = &q->actions[i].target;
            int theirs = (t->kind == MTG_TGT_PLAYER && t->id == THEM)
                      || (t->kind == MTG_TGT_OBJ && B.duel.objs[t->id].controller == THEM);
            if (theirs) {
                B.targetCursor = i;
                break;
            }
        }
        PlaySE(SE_SELECT);
    }
    if (pressed & (KEY_LEFT | KEY_UP))
        B.targetCursor = (B.targetCursor + B.targetCount - 1) % B.targetCount, PlaySE(SE_SELECT);
    if (pressed & (KEY_RIGHT | KEY_DOWN))
        B.targetCursor = (B.targetCursor + 1) % B.targetCount, PlaySE(SE_SELECT);
    if (pressed & KEY_A) {
        MtgAnswer a;
        memset(&a, 0, sizeof(a));
        a.action = (uint16_t)B.targetCursor;
        PlaySE(SE_SELECT);
        MPNet_Log("mtg: you: a trigger's target");
        B.trigTarget = 0;
        Answer(&a);
    }
}

// a choice while something resolves: YES / NO, PAY / DON'T PAY, or a card to discard / sacrifice
static void HumanChoose(unsigned pressed)
{
    const MtgQuery *q = &B.duel.query;
    MtgAnswer a;
    memset(&a, 0, sizeof(a));
    if (q->choice == MTG_CHOICE_PICK) {
        int inHand = q->candidateCount && B.duel.objs[q->candidates[0]].zone == MTG_ZONE_HAND;
        YourMove("a pick");
        if (inHand) {   // (a discard of the player's choice: as at the end of the turn, in the hand)
            if (B.mode != MODE_DISCARD) {
                B.row = ROW_HAND;
                B.trayWanted = 1;
            }
            HumanDiscard(pressed);
            return;
        }
        if (B.mode != MODE_PICK) {
            B.mode = MODE_PICK;
            B.targetCursor = 0;
            PlaySE(SE_SELECT);
        }
        if (pressed & (KEY_LEFT | KEY_UP))
            B.targetCursor = (B.targetCursor + q->candidateCount - 1) % q->candidateCount, PlaySE(SE_SELECT);
        if (pressed & (KEY_RIGHT | KEY_DOWN))
            B.targetCursor = (B.targetCursor + 1) % q->candidateCount, PlaySE(SE_SELECT);
        if (pressed & KEY_A) {
            char name[64];
            Name(name, sizeof(name), q->candidates[B.targetCursor]);
            MPNet_Log("mtg: you: picked %s", name);
            a.card = (uint8_t)B.targetCursor;
            PlaySE(SE_SELECT);
            Answer(&a);
        }
        return;
    }
    YourMove(q->choice == MTG_CHOICE_PAY ? "pay or not" : "yes or no");
    if (B.mode != MODE_CHOOSE) {
        B.mode = MODE_CHOOSE;
        B.menuCursor = 0;
        B.trayWanted = 0;
        PlaySE(SE_SELECT);
    }
    B.menuCursor = TcgUi_PopMenuMove(pressed, B.menuCursor, 2, 0);
    if (pressed & (KEY_A | KEY_B)) {
        a.card = (pressed & KEY_A) && B.menuCursor == 0 ? 1 : 0;
        MPNet_Log("mtg: you: %s", a.card ? (q->choice == MTG_CHOICE_PAY ? "paid" : "yes") : (q->choice == MTG_CHOICE_PAY ? "didn't pay" : "no"));
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
    if (B.sweepAge < SWEEP_FRAMES)
        B.sweepAge++;
    if (B.bigAge < BIG_FRAMES)
        B.bigAge++;
    BuildRows();
    Layout();
    Slides();
    Play();
    {
        const MtgQuery *qq = Mtg_Query(&B.duel);
        int mine = !MtgBoard_Busy() && qq->player == ME && qq->kind != MTG_Q_GAME_OVER && !B.autoplay
                && !(qq->kind == MTG_Q_PRIORITY && B.mode == MODE_NORMAL && !(keys & KEY_L) && AutoPass(keys));
        B.still = mine ? (B.still < 1000 ? B.still + 1 : B.still) : 0;
    }
    if (B.tickerCount && ++B.tickerAge >= TICKER_FRAMES) {   // the next line of what happened
        memmove(B.ticker[0], B.ticker[1], sizeof(B.ticker[0]) * (TICKER - 1));
        B.tickerCount--;
        B.tickerAge = 0;
    }
    if (B.holdAge < TOAST_FRAMES)
        B.holdAge++;
    {
        // the hand's tray: up while the cursor is in the hand (and the hand is where the player acts)
        float want = (B.row == ROW_HAND && B.pile == PILE_NONE && !B.inspect
                      && ((B.trayWanted && (B.mode == MODE_NORMAL || B.mode == MODE_MENU)) || B.mode == MODE_DISCARD))
                     || (B.showHand > 0 && !B.inspect && (B.mode == MODE_NORMAL || B.mode == MODE_MENU))
                     ? 1.0f : 0.0f;
        if (B.showHand > 0)
            B.showHand--;
        B.tray += (want - B.tray) * 0.3f;
        if (fabsf(want - B.tray) < 0.01f)
            B.tray = want;
    }
    if (MtgBoard_Busy() && B.mode != MODE_RESULT) {   // what happened is still being shown: everyone waits
        B.aiWait = Frames(12);   // (and the computer takes a breath after it)
        B.autoPassWait = 0;
        if (pressed)   // (a button pressed as the last move plays out: kept a moment, used once it's over)
            B.pendingKeys = pressed, B.pendingAge = 0;
        else if (B.pendingAge < 60)
            B.pendingAge++;
        B.wasBusy = 1;
        return 0;
    }
    if (B.pendingKeys) {
        if (B.pendingAge < 45)
            pressed |= B.pendingKeys;
        B.pendingKeys = 0;
    }
    Refocus();
    if (B.wasBusy) {
        B.wasBusy = 0;
        if (Mtg_Query(&B.duel)->player == ME && Mtg_Query(&B.duel)->kind != MTG_Q_GAME_OVER && !B.autoplay)
            MPNet_Log("mtg: ready");   // (the test harness: the table is still, the player's turn to act)
    }
    q = Mtg_Query(&B.duel);
    if (B.mode == MODE_CONCEDE) {
        if (pressed & KEY_A) {
            PlaySE(SE_SELECT);
            MPNet_Log("mtg: you: conceded");
            B.mode = MODE_NORMAL;
            MtgBoard_ForceEnd(0);
            return 0;
        }
        if (pressed & KEY_B)
            B.mode = MODE_NORMAL, PlaySE(SE_SELECT);
        if (q->kind != MTG_Q_GAME_OVER)
            return 0;
    }
    if (B.mode == MODE_VIEW) {
        if (B.inspect) {
            if (pressed & (KEY_B | KEY_INSPECT | KEY_SELECT))
                B.inspect = 0, PlaySE(SE_SELECT);
            return 0;
        }
        if (pressed & KEY_LEFT && B.viewCursor > 0)
            B.viewCursor--, PlaySE(SE_SELECT);
        if (pressed & KEY_RIGHT && B.viewCursor + 1 < B.viewCount)
            B.viewCursor++, PlaySE(SE_SELECT);
        if (pressed & KEY_DOWN && B.viewCursor + 7 < B.viewCount)
            B.viewCursor += 7, PlaySE(SE_SELECT);
        if (pressed & KEY_UP && B.viewCursor >= 7)
            B.viewCursor -= 7, PlaySE(SE_SELECT);
        if (pressed & (KEY_INSPECT | KEY_SELECT | KEY_A)) {
            B.inspect = 1;
            B.inspectObj = B.viewList[B.viewCursor];
            PlaySE(SE_SELECT);
        } else if (pressed & KEY_B) {
            B.mode = MODE_NORMAL;
            PlaySE(SE_SELECT);
        }
        if (q->kind != MTG_Q_GAME_OVER)
            return 0;
    }
    if (B.mode == MODE_RESULT) {
        B.resultAge++;
        if (B.resultAge >= G_RESULT_READY && (pressed & (KEY_A | KEY_B)))
            return B.result;
        return 0;
    }
    if (q->kind == MTG_Q_GAME_OVER) {
        B.result = q->winner == ME ? 1 : q->winner == THEM ? 2 : 3;
        B.mode = MODE_RESULT;
        B.pile = PILE_NONE;
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
    if ((pressed & (KEY_INSPECT | KEY_SELECT)) && B.mode != MODE_ATTACK && B.mode != MODE_TURN) {
        int f = Focused();
        if (B.mode == MODE_CHOOSE)
            f = q->source;
        else if (B.mode == MODE_PICK)
            f = q->candidates[B.targetCursor];
        else if (B.mode == MODE_TARGET) {
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
        {
            MtgAnswer a;
            int shown = 0;
            if (!B.autoplay && q->player == THEM && B.teleStamp == B.duel.eventCount + 1u) {
                if (B.teleT > 0)
                    return 0;       // (still showing it)
                a = B.teleAnswer;   // (the move shown: now it's made)
                B.teleStamp = 0;
            } else {
                MtgAi_Answer(&B.duel, &a);
                if (!B.autoplay && q->player == THEM && q->kind == MTG_Q_PRIORITY && a.action < q->actionCount
                    && q->actions[a.action].kind != MTG_ACT_PASS) {
                    // their move, shown first: the card comes out of their hand (or their permanent lights up), the
                    // line says what they're doing, then it's made
                    const MtgAction *act = &q->actions[a.action];
                    char name[64];
                    B.teleAnswer = a;
                    B.teleStamp = B.duel.eventCount + 1u;
                    B.teleT = Frames(30);
                    B.teleObj = act->obj;
                    B.teleFromHand = act->kind != MTG_ACT_ACTIVATE;
                    if (act->kind == MTG_ACT_ACTIVATE) {
                        B.flash[act->obj] = (uint8_t)Frames(30);
                        Name(name, sizeof(name), act->obj);
                        Ticker("%s is using %s...", B.opp, name);
                    } else
                        Ticker(act->kind == MTG_ACT_LAND ? "%s is playing a land..." : "%s is casting a spell...", B.opp);
                    PlaySE(SE_SELECT);
                    shown = 1;
                }
            }
            if (!shown) {
                int acted = q->player == THEM && ((q->kind == MTG_Q_PRIORITY && a.action < q->actionCount
                                                   && q->actions[a.action].kind != MTG_ACT_PASS)
                                                  || (q->kind == MTG_Q_ATTACKERS && a.attack));
                if (!B.autoplay && q->player == THEM && q->kind == MTG_Q_ATTACKERS && a.attack) {
                    // their attack, announced before the creatures charge
                    int n = 0, i;
                    char line[64];
                    for (i = 0; i < q->candidateCount; i++)
                        n += (a.attack >> i) & 1;
                    snprintf(line, sizeof(line), "%s attacks with %d creature%s!", B.opp, n, n == 1 ? "" : "s");
                    Hold(line);
                    PlaySE(SE_M_TAKE_DOWN);
                }
                Mtg_Answer(&B.duel, &a);
                // (a moment after each of their moves: what they did can be seen before the next)
                B.aiWait = B.autoplay ? 0 : acted ? Frames(18) : 2;
            }
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
    case MTG_Q_TRIGGER_TARGET:
        HumanTriggerTarget(pressed);
        break;
    case MTG_Q_CHOOSE:
        HumanChoose(pressed);
        break;
    }
    return 0;
}
