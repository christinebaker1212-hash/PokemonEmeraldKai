// Moemon Kai: OPTIONS > DISPLAY > UI STYLE (docs/ui_styles/UI_STYLES.md "Building it").
//
// A style is a pack (tools/ui_styles/build_pack.py): its pictures, its fonts (a glyph for each of the game's character
// codes) and its SPECS, one for each thing it draws, each a list of ops drawn when their conditions hold, the live
// words and numbers (FIELDS) filled in here. tools/ui_styles/uipack.py is the same drawing in Python (render()): the
// specs are checked there against the approved mock-ups (check_specs.py), and this draws them the same way.
//
// What a style draws, it draws over the finished frame (VDraw), the game's own piece hidden underneath: the battle's
// healthboxes aren't drawn (their sprites' mpNoDraw), BG0's pixels in the box's place aren't (gMpUiHideBg0, read by
// the renderer). Everything the game does stays the game's own: its text printers still print (the words they're
// given are kept here and drawn again in the style's font), its cursors still move (read here), its bars still
// fill (their HP and EXP passed here as they move).
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "platform/gamepad.h"
#include "main.h"
#include "battle.h"
#include "battle_controllers.h"
#include "battle_anim.h"
#include "battle_interface.h"
#include "battle_main.h"
#include "field_message_box.h"
#include "menu.h"
#include "party_menu.h"
#include "constants/party_menu.h"
#include "overworld.h"
#include "gpu_regs.h"
#include "bg.h"
#include "mp_life.h"
#include "palette.h"
#include "pokedex.h"
#include "pokemon.h"
#include "sprite.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "window.h"
#include "constants/battle.h"
#include "constants/characters.h"
#include "constants/species.h"
#include "data.h"
#include "mp.h"
#include "mp_uistyle.h"

const char *Updater_ExeFile(const char *name);
bool8 MP_StartMenu_State(u8 *actions, u8 *count, u8 *cursor, const u8 **names);
bool8 MP_MoveInfoView(void);
void MP_ScrollArrows_Sprites(u8 taskId, u8 *top, u8 *bottom);
u8 GetStartMenuWindowId(void);
extern bool8 gMpUiMoveTypeAlt;
void MP_AsciiToGame(u8 *dst, const char *src, int maxLen);

// ---------------------------------------------------------------------------------------------------- styles
static const char *const sNames[UISTYLE_COUNT] = {"KAI", "RED", "CRYSTAL", "FIRERED", "PLATINUM", "BLACK"};
static const char *const sKeys[UISTYLE_COUNT] = {"kai", "red", "crystal", "firered", "platinum", "black"};
// (RED's, CRYSTAL's and FIRERED's are made from the pret decomps and kept in the repository; PLATINUM's and BLACK's
// from the user's ROMs by the release, beside SINNOH's and UNOVA's packs)
static const char *const sFiles[UISTYLE_COUNT] = {
    NULL, "uistyle/red.pak", "uistyle/crystal.pak", "uistyle/firered.pak", "sinnoh/ui_platinum.pak", "unova/ui_black.pak",
};

const char *MP_UiStyle_Name(u8 style)
{
    return sNames[style < UISTYLE_COUNT ? style : 0];
}

const char *MP_UiStyle_Key(u8 style)
{
    return sKeys[style < UISTYLE_COUNT ? style : 0];
}

u8 MP_UiStyle_FromKey(const char *key)
{
    u8 i;
    for (i = 0; i < UISTYLE_COUNT; i++)
        if (strcmp(key, sKeys[i]) == 0)
            return i;
    return UISTYLE_KAI;
}

// ---------------------------------------------------------------------------------------------------- the pack
// (the numbers tools/ui_styles/uipack.py writes)
enum { OP_IMG = 1, OP_TEXT, OP_BAR, OP_FILL, OP_MSG, OP_LONG, OP_BACKING, OP_IMGSEL, OP_ROWMAP, OP_BLEND, OP_ICON, OP_CHOICES,
       OP_LIST, OP_SNAP, OP_ICONSET };
#define FIELD_CODE 0xFD
#define NFIELDS 192
#define F_NAME 1
#define F_LEVEL 2
#define F_HP 3
#define F_MAXHP 4
#define F_HP3 5
#define F_MAXHP3 6
#define F_ITEM0 16
#define F_PP 32
#define F_MAXPP 33
#define F_TYPE 34
#define F_TEXT0 40
#define F_EXP 64
#define F_HPFRAC 65
#define F_ICON0 70   // (a list row's icon, a style's own: START's from its map.start)
#define F_CHOICE0 80   // an OPTIONS row's choices (80-87), how many (val), the one on (val), its name, the line under
#define F_NCHOICE 88   // the rows, a page's title
#define F_CUR 89
#define F_CURSTR 90
#define F_HINT 91
#define F_TITLE 92
#define F_QTY 93       // a BAG row's count, its count two wide ("%2d"), the button it's registered to
#define F_QTY2 94
#define F_TAG 95
#define F_SPECIES 96   // the summary: its words (MP_Summary_Words)
#define F_DEXNO 97
#define F_OT 98
#define F_IDNO 99
#define F_HELD 100
#define F_NATURE 101
#define F_ABILITY 102
#define F_METLV 103
#define F_METPLACE 104
#define F_EXPPTS 105
#define F_NEXTLV 106
#define F_STAT0 107    // ATK DEF SP.ATK SP.DEF SPEED (107-111)
#define F_IV0 112      // HP ... SPEED: stars 0-3, 4 a perfect 31 (vals, 112-117)
#define F_EV0 118      // HP ... SPEED (118-123)
#define F_TYPE1 124    // (vals)
#define F_TYPE2 125
#define F_POWER 151
#define F_ACC 152
#define F_APPEAL 153   // (vals: hearts)
#define F_JAM 154
#define F_RIBBONS 155
#define F_PAGENO 156   // (val)
#define F_JPAGE 157    // the JOURNAL's page of pages
#define F_CTYPE 158    // a move row's contest type (val)
#define F_PROMPT 159   // the summary: what A does (val: 0 CANCEL, 1 IV RATING, 2 EVs, 3 STATS, 4 INFO, 5 SWITCH, 6 MORE,
                       // 7 nothing)
#define F_PAGETITLE 160
#define F_CREATURES 161   // (val) 1 when the game says POKéMON (OPTIONS > CREATURES), 0 MOéMON
#define C_LV3 (1u << 0)
#define C_MALE (1u << 1)
#define C_FEMALE (1u << 2)
#define C_STATUS (1u << 3)
#define C_PSN (1u << 4)
#define C_LONG (1u << 9)
#define C_SEL0 (1u << 10)
#define C_WAIT (1u << 20)
#define C_EXP (1u << 21)
#define C_LV1 (1u << 22)
#define C_ALT (1u << 23)
#define C_MOEWORD (1u << 25)   // the battle's POKéMON command reads MOéMON (OPTIONS > CREATURES)
#define C_STDCMD (1u << 26)    // the battle's commands are FIGHT, BAG, POKéMON, RUN (a style's own lettering fits)
#define C_HELD0 (1u << 14)     // b.moves: the move held to swap (SELECT), C_HELD0 << its row (C_SEL0's rows 4-7)
#define C_CAUGHT (1u << 27)    // a battle box: a wild foe whose kind the POKéDEX has caught (the ball by its name)
#define C_NOBG (1u << 28)      // a battle box: OPTIONS > DISPLAY > WHITE BG OFF (RED / CRYSTAL: no paper behind it)
#define P_EMPTY (1u << 27)     // a party slot: nobody in it
#define P_EGG (1u << 28)       // ... an EGG (its name only)
#define P_DESC (1u << 29)      // ... its description in its HP's place (ABLE, NOT ABLE...)
#define P_FAINT (1u << 30)     // ... fainted
#define P_SWITCH (1u << 31)    // ... picked to switch
#define S_UP (1u << 27)        // a whole screen's list (not a party slot's): more rows above
#define S_DOWN (1u << 28)      // ... more below
#define S_PAGE (1u << 29)      // OPTIONS: a page's title in place of the tabs
#define S_HINT (1u << 30)      // OPTIONS: the line under the rows says something about the row
#define O_ALL (1u << 31)       // an OPTIONS row: its choices all shown (two or three of them), not just the one on
#define O_VALUE (1u << 27)     // ... it has a value (a row that opens a page has none)
#define B_CLOSE (1u << 27)     // a BAG row: CLOSE BAG
#define B_QTY (1u << 28)       // ... it has a count
#define B_TAG (1u << 29)       // ... it's registered to a button
#define B_SWAP (1u << 30)      // ... it's being moved (SELECT)
#define SM_EV (1u << 20)       // the summary: the SKILLS page shows EVs
#define SM_IV (1u << 21)       // ... IV stars
#define SM_CHOOSING (1u << 23) // ... a move picked (its words shown)
#define SM_LEARN (1u << 26)    // ... the fifth move row is a move to learn
#define SM_TWOTYPES (1u << 27)
#define SM_MEMOPLAIN (1u << 28) // ... met there at that level by its own trainer
#define SM_RENAME (1u << 29)   // ... SELECT renames it
#define SM_CLEAREV (1u << 30)  // ... START clears its EVs (the EV view)
#define SM_EGG (1u << 31)
#define R_NEW (1u << 27)       // a summary move row: the move to learn
#define R_CANCEL (1u << 28)    // ... CANCEL (choosing a move)
#define R_EMPTY (1u << 29)     // ... no move
#define OP_SIZE 38

struct UiImg { u16 w, h; s16 dx, dy; const u8 *px; const u8 *a; s16 *cols; u16 nCols; };   // (px: u16s, unaligned)
struct UiGlyph { u8 adv; s8 dx, dy; u8 w, h; u32 off; };
struct UiFont { u8 cellH; struct UiGlyph g[256]; const u8 *pix; u32 n; };
struct UiOp { u8 kind, font, align, flags; u32 cond, unless; s16 x, y; u16 a, b, c, d, e, ink, shadow, outline, w, h, n; };
struct UiSpec { char name[24]; u16 w, h, cw, ch, nOps; struct UiOp *ops; };
struct UiPack
{
    u8 *data;
    u32 size;
    u16 nImg, nFont, nSpec, nStr;
    struct UiImg *img;
    struct UiFont *font;
    struct UiSpec *spec;
    const u8 **str;
    u16 *strLen;
};

static struct UiPack *sPacks[UISTYLE_COUNT];
static bool8 sTried[UISTYLE_COUNT];

static u16 Rd16(const u8 *p) { return p[0] | p[1] << 8; }
static u32 Rd32(const u8 *p) { return p[0] | p[1] << 8 | p[2] << 16 | (u32)p[3] << 24; }

static void FreePack(struct UiPack *p)
{
    u16 i;
    if (p == NULL)
        return;
    if (p->img)
        for (i = 0; i < p->nImg; i++)
            free(p->img[i].cols);
    if (p->spec)
        for (i = 0; i < p->nSpec; i++)
            free(p->spec[i].ops);
    free(p->img);
    free(p->font);
    free(p->spec);
    free(p->str);
    free(p->strLen);
    free(p->data);
    free(p);
}

static struct UiPack *ParsePack(u8 *d, u32 size)
{
    struct UiPack *p = calloc(1, sizeof(*p));
    u32 o = 18, i, k;   // (the header: 'UIS1' u16 version, four u16 counts, u32 0)
    if (p == NULL)
        return NULL;
    p->data = d, p->size = size;
#define NEED(n) do { if (o + (u32)(n) > size) goto bad; } while (0)
    NEED(0);
    if (size < 18 || memcmp(d, "UIS1", 4) != 0 || Rd16(d + 4) != 2)
        goto bad;
    p->nImg = Rd16(d + 6), p->nFont = Rd16(d + 8), p->nSpec = Rd16(d + 10), p->nStr = Rd16(d + 12);
    p->img = calloc(p->nImg + 1, sizeof(*p->img));
    p->font = calloc(p->nFont + 1, sizeof(*p->font));
    p->spec = calloc(p->nSpec + 1, sizeof(*p->spec));
    p->str = calloc(p->nStr + 1, sizeof(*p->str));
    p->strLen = calloc(p->nStr + 1, sizeof(*p->strLen));
    if (!p->img || !p->font || !p->spec || !p->str || !p->strLen)
        goto bad;
    for (i = 0; i < p->nImg; i++)
    {
        struct UiImg *im = &p->img[i];
        u8 alpha;
        NEED(12);
        im->w = Rd16(d + o), im->h = Rd16(d + o + 2), alpha = d[o + 8];
        im->dx = (s16)Rd16(d + o + 4), im->dy = (s16)Rd16(d + o + 6);
        o += 12;
        NEED((u32)im->w * im->h * 2);
        im->px = d + o;
        o += (u32)im->w * im->h * 2;
        if (alpha)
        {
            NEED((u32)im->w * im->h);
            im->a = d + o;
            o += (u32)im->w * im->h;
        }
    }
    for (i = 0; i < p->nFont; i++)
    {
        struct UiFont *f = &p->font[i];
        NEED(4 + 256 * 12 + 4);
        f->cellH = d[o];
        o += 4;
        for (k = 0; k < 256; k++, o += 12)
        {
            f->g[k].adv = d[o], f->g[k].dx = (s8)d[o + 1], f->g[k].dy = (s8)d[o + 2];
            f->g[k].w = d[o + 3], f->g[k].h = d[o + 4], f->g[k].off = Rd32(d + o + 8);
        }
        f->n = Rd32(d + o);
        o += 4;
        NEED(f->n);
        f->pix = d + o;
        o += f->n;
        for (k = 0; k < 256; k++)
            if (f->g[k].off != 0xFFFFFFFF && f->g[k].off + (u32)f->g[k].w * f->g[k].h > f->n)
                goto bad;
    }
    for (i = 0; i < p->nSpec; i++)
    {
        struct UiSpec *s = &p->spec[i];
        NEED(24 + 12);
        memcpy(s->name, d + o, 24);
        s->name[23] = 0;
        o += 24;
        s->w = Rd16(d + o), s->h = Rd16(d + o + 2), s->cw = Rd16(d + o + 4), s->ch = Rd16(d + o + 6);
        s->nOps = Rd16(d + o + 8);
        o += 12;
        NEED((u32)s->nOps * OP_SIZE);
        s->ops = calloc(s->nOps + 1, sizeof(*s->ops));
        if (s->ops == NULL)
            goto bad;
        for (k = 0; k < s->nOps; k++, o += OP_SIZE)
        {
            struct UiOp *op = &s->ops[k];
            const u8 *q = d + o;
            op->kind = q[0], op->font = q[1], op->align = q[2], op->flags = q[3];
            op->cond = Rd32(q + 4), op->unless = Rd32(q + 8);
            op->x = (s16)Rd16(q + 12), op->y = (s16)Rd16(q + 14);
            op->a = Rd16(q + 16), op->b = Rd16(q + 18), op->c = Rd16(q + 20), op->d = Rd16(q + 22), op->e = Rd16(q + 24);
            op->ink = Rd16(q + 26), op->shadow = Rd16(q + 28), op->outline = Rd16(q + 30);
            op->w = Rd16(q + 32), op->h = Rd16(q + 34), op->n = Rd16(q + 36);
        }
    }
    for (i = 0; i < p->nStr; i++)
    {
        NEED(2);
        p->strLen[i] = Rd16(d + o);
        o += 2;
        NEED(p->strLen[i]);
        p->str[i] = d + o;
        o += p->strLen[i];
    }
#undef NEED
    return p;
bad:
    p->data = NULL;   // (the caller frees the data)
    FreePack(p);
    return NULL;
}

static struct UiPack *GetPack(u8 style)
{
    FILE *f;
    long size;
    u8 *d;
    if (style == UISTYLE_KAI || style >= UISTYLE_COUNT)
        return NULL;
    if (sPacks[style] || sTried[style])
        return sPacks[style];
    sTried[style] = TRUE;
    if ((f = fopen(Updater_ExeFile(sFiles[style]), "rb")) == NULL)
    {
        MPNet_Log("ui style: %s isn't there (%s keeps KAI's look)", sFiles[style], sNames[style]);
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);
    d = size > 0 ? malloc(size) : NULL;
    if (d == NULL || fread(d, 1, size, f) != (size_t)size)
    {
        fclose(f);
        free(d);
        return NULL;
    }
    fclose(f);
    if ((sPacks[style] = ParsePack(d, size)) == NULL)
    {
        free(d);
        MPNet_Log("ui style: %s isn't a pack this game reads", sFiles[style]);
    }
    else
        MPNet_Log("ui style: %s (%d pictures, %d fonts, %d specs)", sNames[style], sPacks[style]->nImg,
                  sPacks[style]->nFont, sPacks[style]->nSpec);
    return sPacks[style];
}

bool8 MP_UiStyle_Available(u8 style)
{
    if (style == UISTYLE_KAI)
        return TRUE;
    if (style >= UISTYLE_COUNT)
        return FALSE;
    if (sTried[style])
        return sPacks[style] != NULL;
    {
        FILE *f = fopen(Updater_ExeFile(sFiles[style]), "rb");
        if (f == NULL)
            return FALSE;
        fclose(f);
    }
    return TRUE;
}

u8 MP_UiStyle_Active(void)
{
    u8 s = gMpConfig.uiStyle;
    if (s == UISTYLE_KAI || s >= UISTYLE_COUNT)
        return UISTYLE_KAI;
    return GetPack(s) ? s : UISTYLE_KAI;
}

// ---------------------------------------------------------------------------------------------------- the JOURNAL
static u8 sJPage, sJPages = 1;

// A on the summary's JOURNAL: the style's next page (TRUE: the style turned it)
bool8 MP_UiStyle_JournalNext(void)
{
    if (MP_UiStyle_Active() == UISTYLE_KAI)
        return FALSE;
    if (sJPages > 1)
        sJPage = (sJPage + 1) % sJPages;
    return TRUE;
}

void MP_UiStyle_JournalReset(void)
{
    sJPage = 0;
}

// ---------------------------------------------------------------------------------------------------- the D-pad
// A style that lays a menu out other than KAI does (RED's moves a list, PLATINUM's FIGHT above the other three, a
// party in a list or two columns) moves its cursor as it's laid out: each entry's place on the style's own grid, the
// D-pad going to the nearest entry that way. KAI's own layouts keep KAI's own code.
static bool8 NavPlace(u8 style, u8 kind, u8 i, s8 *x, s8 *y)
{
    switch (kind)
    {
    case MP_UINAV_COMMAND:   // (0 FIGHT, 1 BAG, 2 POKéMON, 3 RUN)
        if (style == UISTYLE_PLATINUM)
        {
            // (FIGHT across the top; BAG, RUN, POKéMON along the foot)
            static const s8 sX[4] = {2, 0, 4, 2}, sY[4] = {0, 1, 1, 1};
            *x = sX[i & 3], *y = sY[i & 3];
            return TRUE;
        }
        return FALSE;
    case MP_UINAV_MOVES:     // (the four moves)
        if (style == UISTYLE_RED || style == UISTYLE_CRYSTAL)
        {
            *x = 0, *y = i;
            return TRUE;
        }
        return FALSE;
    case MP_UINAV_PARTY:     // (0-5 the slots, 6 CONFIRM, 7 CANCEL)
        if (style == UISTYLE_RED || style == UISTYLE_CRYSTAL)
        {
            // (the Game Boy's list has no CANCEL: B closes it, and the cursor goes round the POKéMON)
            if (i == 7)
                return FALSE;
            *x = 0, *y = i;
            return TRUE;
        }
        if (style == UISTYLE_PLATINUM || style == UISTYLE_BLACK)
        {
            if (i < 6)
                *x = (i & 1) * 2, *y = i / 2;
            else
                *x = i == 6 ? 0 : 2, *y = 3;
            return TRUE;
        }
        return FALSE;
    }
    return FALSE;
}

bool8 MP_UiStyle_NavOwn(u8 kind)
{
    s8 x, y;
    return NavPlace(MP_UiStyle_Active(), kind, 0, &x, &y);
}

int MP_UiStyle_NavMove(u8 kind, int cur, u16 keys, u32 usable, bool8 wrap)
{
    u8 style = MP_UiStyle_Active();
    s8 cx, cy, x, y;
    int dx = 0, dy = 0, i, best = -1, bestMain = 0, bestSide = 0;
    if (!NavPlace(style, kind, cur, &cx, &cy))
        return cur;
    if (keys & DPAD_UP)
        dy = -1;
    else if (keys & DPAD_DOWN)
        dy = 1;
    else if (keys & DPAD_LEFT)
        dx = -1;
    else if (keys & DPAD_RIGHT)
        dx = 1;
    else
        return cur;
    for (i = 0; i < 32; i++)
    {
        int main, side;
        if (i == cur || !(usable & (1u << i)) || !NavPlace(style, kind, i, &x, &y))
            continue;
        main = dx ? (x - cx) * dx : (y - cy) * dy;
        side = dx ? abs(y - cy) : abs(x - cx);
        if (main <= 0 || (dx && side > 0))   // (left and right stay on their row)
            continue;
        if (best < 0 || main < bestMain || (main == bestMain && side < bestSide))
            best = i, bestMain = main, bestSide = side;
    }
    if (best < 0 && wrap && dy)
    {
        // (round to the far end, the nearest column there)
        for (i = 0; i < 32; i++)
        {
            int main, side;
            if (i == cur || !(usable & (1u << i)) || !NavPlace(style, kind, i, &x, &y))
                continue;
            main = (cy - y) * dy;
            side = abs(x - cx);
            if (main <= 0)
                continue;
            if (best < 0 || main > bestMain || (main == bestMain && side < bestSide))
                best = i, bestMain = main, bestSide = side;
        }
    }
    return best < 0 ? cur : best;
}

void MP_UiStyle_Changed(void)
{
    MPNet_Log("ui style: %s", sNames[MP_UiStyle_Active()]);
}

static const struct UiSpec *FindSpec(const struct UiPack *p, const char *name)
{
    u16 i;
    for (i = 0; i < p->nSpec; i++)
        if (strcmp(p->spec[i].name, name) == 0)
            return &p->spec[i];
    return NULL;
}

// ---------------------------------------------------------------------------------------------------- the words kept
// (what each window was last given to print, the filtered copy the printer reads; its printer shows how far it's got)
#define CAP_LEN 640
struct Capture
{
    u8 text[CAP_LEN];
    u16 len;
    const u8 *base;   // the string the printer was given (it walks from here)
};
static struct Capture sCap[WINDOWS_MAX];
u8 gMpUiTextBlit;
static bool8 sPics[WINDOWS_MAX];   // (a picture drawn into the window: its inside stays Kai's)
static u8 sFramed[WINDOWS_MAX];   // 1 its dialogue frame drawn, 2 the standard frame, 0 neither
// (each window's words line by line as printed, where and in what colour: a pop-up drawn in a style's own font)
#define RUNS 32
struct Run { u8 x, y, fg, lineH; u16 kaiW; u8 text[64]; };   // (LINEH: its new lines' spacing)
static struct Run sRuns[WINDOWS_MAX][RUNS];
static u8 sRunCount[WINDOWS_MAX];
static bool8 sShown[WINDOWS_MAX];
static s8 sBox[WINDOWS_MAX][4];   // a battle box round a window (HandleBattleWindow): its inside in tiles, x0 < 0 none
static u32 sHandled;   // the windows a screen drew itself this frame


void MP_UiStyle_Framed(u8 windowId, u8 kind)
{
    if (windowId < WINDOWS_MAX)
        sFramed[windowId] = kind, sBox[windowId][0] = -1;
}

// the battle's own boxes (YES / NO, the level up's stats: HandleBattleWindow draws their frames as tiles round
// windows of their own): the windows inside one framed (in the style's pop-up) or no longer, the box's inside kept
// (YES / NO's cursor is a tile in it, left of its window)
void MP_UiStyle_BattleBox(u8 x0, u8 y0, u8 x1, u8 y1, bool8 bg1, bool8 clear)
{
    u8 w;
    for (w = 0; w < WINDOWS_MAX; w++)
    {
        const struct WindowTemplate *t = &gWindows[w].window;
        if (gWindows[w].tileData == NULL || t->bg != (bg1 ? 1 : 0) || t->width == 0)
            continue;
        if (t->tilemapLeft <= x0 || t->tilemapLeft + t->width - 1 >= x1 || t->tilemapTop <= y0
         || t->tilemapTop + t->height - 1 >= y1)
            continue;
        sFramed[w] = clear ? 0 : 2;
        sBox[w][0] = clear ? -1 : x0 + 1;
        sBox[w][1] = y0 + 1, sBox[w][2] = x1 - 1, sBox[w][3] = y1 - 1;
    }
}

void MP_UiStyle_TextPrinted(u8 windowId, const u8 *str)
{
    struct Capture *c;
    u16 n = 0;
    if (windowId >= WINDOWS_MAX || str == NULL)
        return;
    c = &sCap[windowId];
    while (n < CAP_LEN - 1 && str[n] != EOS)
        n++;
    memcpy(c->text, str, n);
    c->text[n] = EOS;
    c->len = n;
    c->base = str;
}

void MP_UiStyle_WindowFilled(u8 windowId)
{
    bool8 active;
    u8 state;
    if (windowId >= WINDOWS_MAX)
        return;
    MP_TextPrinter_Cur(windowId, &active, &state);
    if (active)   // (the printer clearing its own page: its words go on)
        return;
    sRunCount[windowId] = 0;
    sPics[windowId] = FALSE;
    sCap[windowId].len = 0;
    sCap[windowId].text[0] = EOS;
    sCap[windowId].base = NULL;
}

void MP_UiStyle_TextRun(u8 windowId, const u8 *str, u8 x, u8 y, u8 fontId, u8 fg, u8 letterSpacing, u8 lineSpacing)
{
    struct Run *r;
    u8 k, n;
    if (windowId >= WINDOWS_MAX || str == NULL)
        return;
    for (k = 0; k < sRunCount[windowId] && (sRuns[windowId][k].x != x || sRuns[windowId][k].y != y); k++)
        ;
    if (k == RUNS)
    {
        memmove(&sRuns[windowId][0], &sRuns[windowId][1], sizeof(struct Run) * (RUNS - 1));
        k = RUNS - 1;
    }
    else if (k == sRunCount[windowId])
        sRunCount[windowId]++;
    r = &sRuns[windowId][k];
    r->x = x, r->y = y, r->fg = fg;
    for (n = 0; n < sizeof(r->text) - 1 && str[n] != EOS; n++)
        r->text[n] = str[n];
    r->text[n] = EOS;
    r->kaiW = GetStringWidth(fontId, r->text, letterSpacing);
    r->lineH = GetFontAttribute(fontId, FONTATTR_MAX_LETTER_HEIGHT) + lineSpacing;
    if (r->lineH < 8 || r->lineH > 24)
        r->lineH = 16;
}

void MP_UiStyle_WindowRectFilled(u8 windowId, u16 x, u16 y, u16 w, u16 h)
{
    u8 k = 0;
    if (windowId >= WINDOWS_MAX)
        return;
    while (k < sRunCount[windowId])
    {
        struct Run *r = &sRuns[windowId][k];
        if (r->x >= x && r->x < x + w && r->y >= y && r->y < y + h)
        {
            memmove(r, r + 1, sizeof(struct Run) * (sRunCount[windowId] - k - 1));
            sRunCount[windowId]--;
            continue;
        }
        k++;
    }
}

void MP_UiStyle_WindowPicture(u8 windowId)
{
    if (windowId < WINDOWS_MAX)
        sPics[windowId] = TRUE;
}

void MP_UiStyle_WindowShown(u8 windowId, bool8 shown)
{
    if (windowId < WINDOWS_MAX)
        sShown[windowId] = shown;
}

void MP_UiStyle_WindowGone(u8 windowId)
{
    if (windowId >= WINDOWS_MAX)
        return;
    sRunCount[windowId] = 0;
    sPics[windowId] = FALSE;
    sShown[windowId] = FALSE;
    sFramed[windowId] = 0;
    sBox[windowId][0] = -1;
    sCap[windowId].len = 0;
    sCap[windowId].text[0] = EOS;
    sCap[windowId].base = NULL;
}

// how much of window W's words is printed, and whether its printer waits for A
static u16 Printed(u8 w, bool8 *waiting)
{
    bool8 active;
    u8 state;
    const u8 *cur = MP_TextPrinter_Cur(w, &active, &state);
    struct Capture *c = &sCap[w];
    *waiting = active && (state == RENDER_STATE_WAIT || state == RENDER_STATE_CLEAR || state == RENDER_STATE_SCROLL_START);
    if (!active || c->base == NULL || cur < c->base || cur > c->base + c->len)
        return c->len;
    return cur - c->base;
}

// ---------------------------------------------------------------------------------------------------- drawing
struct UiData
{
    const u8 *str[NFIELDS];
    s32 frac[NFIELDS];   // (0-65536)
    s32 val[NFIELDS];
    u32 flags;
    const u8 *msg[WINDOWS_MAX];   // (a captured window's words, how many there are, how far they're printed, and
    u16 msgLen[WINDOWS_MAX];      // whether its printer waits at the end of a page, that page still shown)
    u16 msgPrinted[WINDOWS_MAX];
    bool8 msgHold[WINDOWS_MAX];
    u16 msgFirst[WINDOWS_MAX];    // (a page a style turns itself: its first line + 1; 0: as the words are printed)
};

struct Surf
{
    u16 *px;      // BGR555, bit 15 drawn
    u8 *a;        // a canvas's alpha (NULL: the frame)
    s16 w, h;
    const struct MpFadeFit *fit;
};

static u16 Fade(const struct Surf *s, u16 c)
{
    return s->fit && s->fit->on ? MP_FadeFit_Apply(s->fit, c & 0x7FFF) : (c & 0x7FFF);
}

static u16 Mix(u16 fg, u16 bg, u32 a)
{
    u32 r = ((fg & 31) * a + (bg & 31) * (255 - a)) / 255;
    u32 g = (((fg >> 5) & 31) * a + ((bg >> 5) & 31) * (255 - a)) / 255;
    u32 b = (((fg >> 10) & 31) * a + ((bg >> 10) & 31) * (255 - a)) / 255;
    return r | (g << 5) | (b << 10);
}

static void Put(struct Surf *s, int x, int y, u16 c, u8 alpha)
{
    int i;
    if (x < 0 || y < 0 || x >= s->w || y >= s->h || alpha == 0)
        return;
    i = y * s->w + x;
    if (s->a)
    {
        u8 ea = s->a[i];
        if (alpha == 255 || ea == 0)
        {
            s->px[i] = (c & 0x7FFF) | 0x8000;
            s->a[i] = alpha;
        }
        else
        {
            s->px[i] = Mix(c, s->px[i], alpha) | 0x8000;
            s->a[i] = alpha + ea * (255 - alpha) / 255;
        }
        return;
    }
    c = Fade(s, c);
    s->px[i] = (alpha == 255 ? c : Mix(c, s->px[i], alpha)) | 0x8000;
}

static void DrawImg(struct Surf *s, const struct UiImg *im, int x, int y)
{
    int i, j;
    x += im->dx, y += im->dy;
    for (j = 0; j < im->h; j++)
        for (i = 0; i < im->w; i++)
        {
            u16 c = Rd16(im->px + 2 * (j * im->w + i));
            if (c & 0x8000)
                Put(s, x + i, y + j, c, im->a ? im->a[j * im->w + i] : 255);
        }
}

// (the DS's BLDALPHA, BG1 over the rest at EVA 16, EVB: each pixel its colour plus EVB / 16 of what's under it)
static void BlendImg(struct Surf *s, const struct UiImg *im, int x, int y, int evb)
{
    int i, j;
    if (s->a)
    {
        DrawImg(s, im, x, y);
        return;
    }
    x += im->dx, y += im->dy;
    for (j = 0; j < im->h; j++)
        for (i = 0; i < im->w; i++)
        {
            u16 c = Rd16(im->px + 2 * (j * im->w + i)), u, o;
            int X = x + i, Y = y + j, k;
            if (!(c & 0x8000) || X < 0 || Y < 0 || X >= s->w || Y >= s->h)
                continue;
            u = s->px[Y * s->w + X];
            o = 0;
            for (k = 0; k < 3; k++)
            {
                u32 v = ((c >> (5 * k)) & 31) + ((((u >> (5 * k)) & 31) * evb) >> 4);
                o |= (v > 31 ? 31 : v) << (5 * k);
            }
            s->px[Y * s->w + X] = Fade(s, o) | 0x8000;
        }
}

// the columns of a bar's fill picture, in order (computed once)
static void BarCols(struct UiImg *im)
{
    int i, j;
    if (im->cols)
        return;
    im->cols = malloc(sizeof(s16) * (im->w + 1));
    im->nCols = 0;
    if (im->cols == NULL)
        return;
    for (i = 0; i < im->w; i++)
        for (j = 0; j < im->h; j++)
            if (Rd16(im->px + 2 * (j * im->w + i)) & 0x8000)
            {
                im->cols[im->nCols++] = i;
                break;
            }
}

static void DrawBar(struct Surf *s, struct UiPack *p, const struct UiOp *op, s32 frac, int x, int y)
{
    // (its colour as the games pick it, by the pixels filled: over half green, over a fifth yellow, else red)
    struct UiImg *im;
    u16 id;
    int n, k, j, cols;
    if (op->b >= p->nImg)
        return;
    BarCols(&p->img[op->b]);
    cols = p->img[op->b].nCols;
    if (cols == 0)
        return;
    n = (frac * cols + 32768) >> 16;
    if (frac > 0 && n == 0)
        n = 1;   // (a sliver for anything above nothing)
    if (n > cols)
        n = cols;
    id = n > cols * 50 / 100 ? op->b : n > cols * 20 / 100 ? op->c : op->d;
    if (id >= p->nImg)
        return;
    im = &p->img[id];
    BarCols(im);
    if (im->cols == NULL || im->nCols == 0)
        return;
    if (n > im->nCols)
        n = im->nCols;
    x += im->dx, y += im->dy;
    for (k = 0; k < n; k++)
    {
        int cx = im->cols[op->e ? im->nCols - 1 - k : k];
        for (j = 0; j < im->h; j++)
        {
            u16 c = Rd16(im->px + 2 * (j * im->w + cx));
            if (c & 0x8000)
                Put(s, x + cx, y + j, c, im->a ? im->a[j * im->w + cx] : 255);
        }
    }
}

// how many argument bytes the game's control code at STR (after its EXT_CTRL_CODE_BEGIN) takes (text.c's own walk)
static int CtrlArgs(u8 code)
{
    switch (code)
    {
    case EXT_CTRL_CODE_COLOR_HIGHLIGHT_SHADOW:
        return 3;
    case EXT_CTRL_CODE_PLAY_BGM:
    case EXT_CTRL_CODE_PLAY_SE:
        return 2;
    case EXT_CTRL_CODE_COLOR:
    case EXT_CTRL_CODE_HIGHLIGHT:
    case EXT_CTRL_CODE_SHADOW:
    case EXT_CTRL_CODE_PALETTE:
    case EXT_CTRL_CODE_FONT:
    case EXT_CTRL_CODE_PAUSE:
    case EXT_CTRL_CODE_ESCAPE:
    case EXT_CTRL_CODE_SHIFT_RIGHT:
    case EXT_CTRL_CODE_SHIFT_DOWN:
    case EXT_CTRL_CODE_CLEAR:
    case EXT_CTRL_CODE_SKIP:
    case EXT_CTRL_CODE_CLEAR_TO:
    case EXT_CTRL_CODE_MIN_LETTER_SPACING:
        return 1;
    }
    return 0;
}

// a button's icon in the game's words (CHAR_KEYPAD_ICON and its number) as its letters in a style's font
static const u8 *KeypadWord(u8 icon)
{
    static const u8 sA[] = _("A"), sB[] = _("B"), sL[] = _("L"), sR[] = _("R"), sStart[] = _("START"),
                    sSelect[] = _("SELECT");
    // (the controller-only buttons by the names the controller in hand prints on them, as mp_buttons.c draws them)
    static const u8 sLt[3][3] = {_("ZL"), _("L2"), _("LT")}, sRt[3][3] = {_("ZR"), _("R2"), _("RT")},
                    sLs[3][3] = {_("LS"), _("L3"), _("LS")}, sRs[3][3] = {_("RS"), _("R3"), _("RS")},
                    sWest[3][7] = {_("Y"), _("SQUARE"), _("X")}, sNorth[3][9] = {_("X"), _("TRIANGLE"), _("Y")};
    int fam = Gamepad_IconFamily();
    int k = fam == GAMEPAD_ICONS_NINTENDO ? 0 : fam == GAMEPAD_ICONS_PLAYSTATION ? 1 : 2;
    switch (icon)
    {
    case CHAR_A_BUTTON: return sA;
    case CHAR_B_BUTTON: return sB;
    case CHAR_L_BUTTON: return sL;
    case CHAR_R_BUTTON: return sR;
    case CHAR_START_BUTTON: return sStart;
    case CHAR_SELECT_BUTTON: return sSelect;
    case CHAR_MP_LT: return sLt[k];
    case CHAR_MP_RT: return sRt[k];
    case CHAR_MP_LS: return sLs[k];
    case CHAR_MP_RS: return sRs[k];
    case CHAR_MP_WEST: return fam == GAMEPAD_ICONS_KEYBOARD ? sSelect : sWest[k];
    case CHAR_MP_NORTH: return fam == GAMEPAD_ICONS_KEYBOARD ? sStart : sNorth[k];
    }
    return NULL;
}

// one of the game's extra symbols (CHAR_EXTRA_SYMBOL and its number: the level up's +, a second Lv, PP, ID...) as
// the style's own letters
static const u8 *SymbolWord(u8 sym)
{
    static const u8 sUp[] = _("{UP_ARROW}"), sDown[] = _("{DOWN_ARROW}"), sLeft[] = _("{LEFT_ARROW}"),
                    sRight[] = _("{RIGHT_ARROW}"), sPlus[] = _("+"), sLv[] = _("{LV}"), sPp[] = _("PP"), sId[] = _("ID"),
                    sNo[] = _("No"), sOpen[] = _("("), sClose[] = _(")");
    static u8 sDigit[2];
    switch (sym)
    {
    case 0x00: return sUp;
    case 0x01: return sDown;
    case 0x02: return sLeft;
    case 0x03: return sRight;
    case 0x04: return sPlus;
    case 0x05: return sLv;
    case 0x06: return sPp;
    case 0x07: return sId;
    case 0x08: return sNo;
    case 0x13: return sOpen;
    case 0x14: return sClose;
    }
    if (sym >= 0x0A && sym <= 0x12)   // (the circled 1-9)
    {
        sDigit[0] = CHAR_1 + (sym - 0x0A), sDigit[1] = EOS;
        return sDigit;
    }
    return NULL;
}

// a two-byte code (a button's icon, an extra symbol) as the letters it stands for, or NULL
static const u8 *CodeWord(u8 c, u8 arg)
{
    if (c == CHAR_KEYPAD_ICON)
        return KeypadWord(arg);
    if (c == CHAR_EXTRA_SYMBOL)
        return SymbolWord(arg);
    return NULL;
}

// one line of S (up to a new line or its end; N bytes at most): its width in F
static int Width(const struct UiFont *f, const u8 *s, int n)
{
    int w = 0, i = 0;
    while (i < n && s[i] != EOS && s[i] != CHAR_NEWLINE)
    {
        u8 c = s[i++];
        if (c == EXT_CTRL_CODE_BEGIN)
        {
            if (i < n)
                i += 1 + CtrlArgs(s[i]);
            continue;
        }
        if ((c == CHAR_KEYPAD_ICON || c == CHAR_EXTRA_SYMBOL) && i < n && CodeWord(c, s[i]))
        {
            const u8 *k = CodeWord(c, s[i++]);
            while (*k != EOS)
                w += f->g[*k++].adv;
            continue;
        }
        if (c == PLACEHOLDER_BEGIN || c == CHAR_DYNAMIC || c == CHAR_KEYPAD_ICON || c == CHAR_EXTRA_SYMBOL)
        {
            i++;
            continue;
        }
        if (c == CHAR_PROMPT_SCROLL || c == CHAR_PROMPT_CLEAR)
            continue;
        if (f->g[c].off != 0xFFFFFFFF || f->g[c].adv)
            w += f->g[c].adv;
    }
    return w;
}

static int sGlyphEvb;   // (a style's text blended as its window is: the DS's BLDALPHA, EVB sixteenths)

static void PutBlend(struct Surf *s, int x, int y, u16 c)
{
    int k;
    u16 u, o = 0;
    if (!sGlyphEvb || s->a || x < 0 || y < 0 || x >= s->w || y >= s->h)
    {
        Put(s, x, y, c, 255);
        return;
    }
    u = s->px[y * s->w + x];
    for (k = 0; k < 3; k++)
    {
        u32 v = ((c >> (5 * k)) & 31) + ((((u >> (5 * k)) & 31) * sGlyphEvb) >> 4);
        o |= (v > 31 ? 31 : v) << (5 * k);
    }
    s->px[y * s->w + x] = Fade(s, o) | 0x8000;
}

static void Glyph(struct Surf *s, const struct UiFont *f, u8 c, int x, int y, u16 ink, u16 shadow, u16 outline,
                  int clipY)
{
    const struct UiGlyph *g = &f->g[c];
    const u8 *pix;
    u16 cols[4];
    int i, j;
    if (g->off == 0xFFFFFFFF)
        return;
    cols[0] = 0, cols[1] = ink, cols[2] = shadow, cols[3] = outline;
    pix = f->pix + g->off;
    for (j = 0; j < g->h; j++)
        for (i = 0; i < g->w; i++)
        {
            u8 v = pix[j * g->w + i];
            if (v && (cols[v] & 0x8000) && y + g->dy + j < clipY)
                PutBlend(s, x + g->dx + i, y + g->dy + j, cols[v]);
        }
}

// one line of S in F from (x, y): the x after it
static int Line(struct Surf *s, const struct UiFont *f, const u8 *str, int n, int x, int y, u16 ink, u16 shadow,
                u16 outline, int clipY)
{
    int i = 0;
    while (i < n && str[i] != EOS && str[i] != CHAR_NEWLINE)
    {
        u8 c = str[i++];
        if (c == EXT_CTRL_CODE_BEGIN)
        {
            if (i < n)
                i += 1 + CtrlArgs(str[i]);
            continue;
        }
        if ((c == CHAR_KEYPAD_ICON || c == CHAR_EXTRA_SYMBOL) && i < n && CodeWord(c, str[i]))
        {
            const u8 *k = CodeWord(c, str[i++]);
            while (*k != EOS)
            {
                Glyph(s, f, *k, x, y, ink, shadow, outline, clipY);
                x += f->g[*k++].adv;
            }
            continue;
        }
        if (c == PLACEHOLDER_BEGIN || c == CHAR_DYNAMIC || c == CHAR_KEYPAD_ICON || c == CHAR_EXTRA_SYMBOL)
        {
            i++;
            continue;
        }
        if (c == CHAR_PROMPT_SCROLL || c == CHAR_PROMPT_CLEAR)
            continue;
        Glyph(s, f, c, x, y, ink, shadow, outline, clipY);
        x += f->g[c].adv;
    }
    return x;
}

// a pack's string with its fields filled in
static int Expand(const struct UiPack *p, u16 id, const struct UiData *d, u8 *out, int cap)
{
    int n = 0, i;
    if (id >= p->nStr)
        return 0;
    for (i = 0; i < p->strLen[id] && n < cap - 1; i++)
    {
        u8 c = p->str[id][i];
        if (c == FIELD_CODE && i + 1 < p->strLen[id])
        {
            const u8 *v = d->str[p->str[id][++i] % NFIELDS];
            while (v && *v != EOS && n < cap - 1)
                out[n++] = *v++;
            continue;
        }
        out[n++] = c;
    }
    out[n] = EOS;
    return n;
}

// a page of a window's words laid out in the style's font: Kai's own lines, any wider than MAXW broken at its last
// space that fits (the rest on a line of its own)
struct FlowCh { u16 src; u8 c; };
#define FLOW_MAX 768
#define FLOW_LINES 48
static struct FlowCh sFlow[FLOW_MAX];
static u16 sLineStart[FLOW_LINES], sLineEnd[FLOW_LINES];

static int FlowPage(const struct UiFont *f, const u8 *t, int from, int to, int maxW)
{
    int n = 0, nl = 0, i, k, lineFirst = 0, w = 0, lastSpace = -1;
    sLineStart[0] = 0;
    for (i = from; i < to && n < FLOW_MAX; i++)
    {
        u8 c = t[i];
        if (c == EXT_CTRL_CODE_BEGIN)
        {
            if (i + 1 < to)
                i += 1 + CtrlArgs(t[i + 1]);
            continue;
        }
        if (c == PLACEHOLDER_BEGIN || c == CHAR_DYNAMIC || c == CHAR_KEYPAD_ICON || c == CHAR_EXTRA_SYMBOL)
        {
            i++;
            continue;
        }
        if (c == CHAR_PROMPT_CLEAR)
            break;
        if (c == CHAR_NEWLINE || c == CHAR_PROMPT_SCROLL)
        {
            if (nl < FLOW_LINES - 1)
            {
                sLineEnd[nl++] = n;
                sLineStart[nl] = n;
            }
            w = 0, lastSpace = -1, lineFirst = n;
            continue;
        }
        sFlow[n].src = i, sFlow[n].c = c;
        if (c == CHAR_SPACE)
            lastSpace = n;
        w += f->g[c].adv;
        n++;
        if (maxW && w > maxW && lastSpace > lineFirst && nl < FLOW_LINES - 1)
        {
            sLineEnd[nl++] = lastSpace;
            sLineStart[nl] = lastSpace + 1;
            lineFirst = lastSpace + 1;
            for (w = 0, k = lineFirst; k < n; k++)
                w += f->g[sFlow[k].c].adv;
            lastSpace = -1;
        }
    }
    sLineEnd[nl++] = n;
    return nl;
}

// OP_MSG: window A's words as far as they're printed, their page only (a new page clears them): from (x, y) N lines H
// apart, lines wider than W wrapped; a page of more lines than N closes them up to rows B apart from y D (C of them
// at most, the box's whole inside), and past that the last ones printed show (\l scrolls them)
static void Msg(struct Surf *s, struct UiPack *p, const struct UiOp *op, const struct UiData *d, int x, int y, int oy,
                int *regs)
{
    const struct UiFont *f;
    const u8 *t;
    int len, shown, i, k, pageFrom = 0, pageTo, nl, lastLine = -1, first, rows, step, y0;
    if (op->font >= p->nFont || op->a >= WINDOWS_MAX || d->msg[op->a] == NULL || op->n == 0)
        return;
    f = &p->font[op->font];
    t = d->msg[op->a];
    len = d->msgLen[op->a];
    shown = d->msgPrinted[op->a];
    if (shown > len)
        shown = len;
    // (waiting at a page's end, past its \p: that page is still the one up)
    if (d->msgHold[op->a] && shown > 0 && t[shown - 1] == CHAR_PROMPT_CLEAR)
        shown--;
    for (i = 0; i < shown; i++)
    {
        u8 c = t[i];
        if (c == EXT_CTRL_CODE_BEGIN)
        {
            if (i + 1 < len)
                i += 1 + CtrlArgs(t[i + 1]);
            continue;
        }
        if (c == PLACEHOLDER_BEGIN || c == CHAR_DYNAMIC || c == CHAR_KEYPAD_ICON || c == CHAR_EXTRA_SYMBOL)
        {
            i++;
            continue;
        }
        if (c == CHAR_PROMPT_CLEAR)
            pageFrom = i + 1;
    }
    if (pageFrom > 0 && pageFrom >= len && shown >= len)
    {
        // (the words ending on a \p, nothing after it: Kai leaves that last page up, as the battle's GREW TO LV. does
        // under the level up's box)
        int last = pageFrom - 1;
        pageFrom = 0;
        for (i = 0; i < last; i++)
        {
            u8 c = t[i];
            if (c == EXT_CTRL_CODE_BEGIN)
            {
                if (i + 1 < len)
                    i += 1 + CtrlArgs(t[i + 1]);
                continue;
            }
            if (c == PLACEHOLDER_BEGIN || c == CHAR_DYNAMIC || c == CHAR_KEYPAD_ICON || c == CHAR_EXTRA_SYMBOL)
            {
                i++;
                continue;
            }
            if (c == CHAR_PROMPT_CLEAR)
                pageFrom = i + 1;
        }
    }
    for (pageTo = pageFrom; pageTo < len && t[pageTo] != CHAR_PROMPT_CLEAR; pageTo++)
    {
        if (t[pageTo] == EXT_CTRL_CODE_BEGIN && pageTo + 1 < len)
            pageTo += 1 + CtrlArgs(t[pageTo + 1]);
        else if (t[pageTo] == PLACEHOLDER_BEGIN || t[pageTo] == CHAR_DYNAMIC || t[pageTo] == CHAR_KEYPAD_ICON
              || t[pageTo] == CHAR_EXTRA_SYMBOL)
            pageTo++;
    }
    if (pageTo > len)
        pageTo = len;
    nl = FlowPage(f, t, pageFrom, pageTo, op->w);
    if (nl > op->n && op->e && op->e - 1 < p->nFont && !op->b)
    {
        // (E: a narrower font for a page that would need more lines than the box has)
        const struct UiFont *f2 = &p->font[op->e - 1];
        int nl2 = FlowPage(f2, t, pageFrom, pageTo, op->w);
        if (nl2 <= op->n)
            f = f2, nl = nl2;
        else
            nl = FlowPage(f, t, pageFrom, pageTo, op->w);
    }
    {
        // (the line the last printed letter is on)
        int nShown = 0;
        while (nShown < sLineEnd[nl - 1] && sFlow[nShown].src < shown)
            nShown++;
        if (nShown == 0)
            return;
        for (i = 0; i < nl; i++)
            if (sLineStart[i] <= nShown - 1 && sLineEnd[i] > sLineStart[i])
                lastLine = i;
    }
    if (lastLine < 0)
        return;
    if (nl <= op->n || op->b == 0)
        rows = op->n, step = op->h, y0 = y;
    else
        rows = op->c ? op->c : op->n, step = op->b, y0 = op->d + oy;
    first = nl > rows ? lastLine + 1 - rows : 0;
    if (d->msgFirst[op->a])
        first = d->msgFirst[op->a] - 1, rows = op->n, step = op->h, y0 = y;
    if (first < 0)
        first = 0;
    for (i = first; i < nl && i < first + rows; i++)
    {
        int lx = x;
        if (op->align)
        {
            int wd = 0;
            for (k = sLineStart[i]; k < sLineEnd[i]; k++)
                wd += f->g[sFlow[k].c].adv;
            if (op->align == 1)
                lx -= wd;
            else if (op->align == 2)   // (each line centred in W)
                lx += (op->w - wd) / 2;
            else if (op->align == 3)   // (... on the Game Boy's 8-pixel grid)
                lx += ((op->w - wd) / 2 / 8) * 8;
        }
        for (k = sLineStart[i]; k < sLineEnd[i] && sFlow[k].src < shown; k++)
        {
            Glyph(s, f, sFlow[k].c, lx, y0 + (i - first) * step, op->ink, op->shadow, op->outline, 0x7FFF);
            lx += f->g[sFlow[k].c].adv;
        }
        if (k > sLineStart[i] || i == lastLine)
        {
            // (where the words end: a style's ▼ goes after them)
            regs[(op->flags >> 1) & 7] = lx - (x - op->x);
            regs[((op->flags >> 1) + 1) & 7] = y0 + (i - first) * step - oy;
        }
    }
}

static u16 sCanvasPx[DISPLAY_WIDTH * DISPLAY_HEIGHT];
static u8 sCanvasA[DISPLAY_WIDTH * DISPLAY_HEIGHT];

static bool8 sTextOnly;   // (a spec's words drawn again, over Kai's sprites: DrawSpecText)
static u16 sSnap[DISPLAY_WIDTH * DISPLAY_HEIGHT];   // (the last field or battle frame: OP_SNAP)
static bool8 sSnapOk;

// an OPTIONS row's choices (fields A...; F_NCHOICE of them, F_CUR the one on) from (x, y) in W: all of them N apart
// (ALIGN 0 from x, 1 ending at x + W), the one on in the ink / shadow, the others in C / D; only the one on when they
// don't fit (in font E - 1 when it's wider than W); the mark B (a glyph) just before the one on
static void Choices(struct Surf *s, struct UiPack *p, const struct UiOp *op, const struct UiData *d, int x, int y)
{
    const struct UiFont *f;
    int n = d->val[F_NCHOICE], cur = d->val[F_CUR], total = 0, i, at;
    if (op->font >= p->nFont || n <= 0 || n > 8 || op->a + n > NFIELDS)
        return;
    f = &p->font[op->font];
    for (i = 0; i < n; i++)
        total += (d->str[op->a + i] ? Width(f, d->str[op->a + i], 255) : 0) + (i ? op->n : 0);
    if (total > op->w || n == 1)
    {
        const u8 *t = cur >= 0 && cur < n ? d->str[op->a + cur] : NULL;
        int wd;
        if (t == NULL)
            return;
        if (op->e && op->e - 1 < p->nFont && Width(f, t, 255) > op->w)
            f = &p->font[op->e - 1];
        wd = Width(f, t, 255);
        at = op->align == 1 ? x + op->w - wd : x;
        if (op->b)
            Glyph(s, f, op->b, at - f->g[op->b].adv, y, op->ink, op->shadow, op->outline, 0x7FFF);
        Line(s, f, t, 255, at, y, op->ink, op->shadow, op->outline, 0x7FFF);
        return;
    }
    at = op->align == 1 ? x + op->w - total : x;
    for (i = 0; i < n; i++)
    {
        const u8 *t = d->str[op->a + i];
        bool8 on = i == cur;
        if (t == NULL)
            continue;
        if (on && op->b)
            Glyph(s, f, op->b, at - f->g[op->b].adv, y, op->ink, op->shadow, op->outline, 0x7FFF);
        at = Line(s, f, t, 255, at, y, on ? op->ink : op->c, on ? op->shadow : op->d, op->outline, 0x7FFF) + op->n;
    }
}

// spec SP over FRAME with its top-left at (ax, ay)
static void DrawSpec(struct UiPack *p, const struct UiSpec *sp, struct UiData *d, u16 *frame, int ax, int ay,
                     const struct MpFadeFit *fit)
{
    struct Surf fr = {frame, NULL, DISPLAY_WIDTH, DISPLAY_HEIGHT, fit}, cv, *s;
    int regs[8] = {0}, ox = 0, oy = 0, k;
    u32 flags = d->flags;
    const struct UiOp *rowmap = NULL, *backing = NULL;
    u8 buf[256];

    if (sp->cw && sp->ch)
    {
        if (sp->cw > DISPLAY_WIDTH || sp->ch > DISPLAY_HEIGHT)
            return;
        cv.px = sCanvasPx, cv.a = sCanvasA, cv.w = sp->cw, cv.h = sp->ch, cv.fit = NULL;
        memset(sCanvasA, 0, sizeof(sCanvasA));
        s = &cv;
    }
    else
    {
        s = &fr;
        ox = ax, oy = ay;
    }
    for (k = 0; k < sp->nOps; k++)
    {
        const struct UiOp *op = &sp->ops[k];
        int x, y;
        if ((flags & op->cond) != op->cond || (flags & op->unless))
            continue;
        if (sTextOnly && op->kind != OP_TEXT && op->kind != OP_LONG && op->kind != OP_CHOICES)
            continue;
        if (op->kind == OP_LONG)
        {
            if (op->font < p->nFont && op->a < NFIELDS && d->str[op->a]
             && Width(&p->font[op->font], d->str[op->a], 255) > op->w)
                flags |= C_LONG;
            continue;
        }
        sGlyphEvb = (op->flags >> 5) & 7;
        x = op->x + ((op->flags & 1) ? regs[(op->flags >> 1) & 7] : 0) + ox;
        y = op->y + ((op->flags & 0x10) ? regs[((op->flags >> 1) + 1) & 7] : 0) + oy;
        switch (op->kind)
        {
        case OP_IMG:
            if (op->a < p->nImg)
                DrawImg(s, &p->img[op->a], x, y);
            break;
        case OP_IMGSEL:
        {
            u32 id = op->a + (op->b < NFIELDS ? d->val[op->b] : 0);
            if (id < p->nImg)
                DrawImg(s, &p->img[id], x, y);
            break;
        }
        case OP_BLEND:
            if (op->a < p->nImg)
                BlendImg(s, &p->img[op->a], x, y, op->b);
            break;
        case OP_FILL:
        {
            int i, j;
            for (j = 0; j < op->h; j++)
                for (i = 0; i < op->w; i++)
                    Put(s, x + i, y + j, op->ink, op->a);
            break;
        }
        case OP_TEXT:
        {
            const struct UiFont *f;
            int n, wd;
            if (op->font >= p->nFont)
                break;
            f = &p->font[op->font];
            n = Expand(p, op->a, d, buf, sizeof(buf));
            if (op->w && op->b && op->b - 1 < p->nFont && Width(f, buf, n) > op->w)
                f = &p->font[op->b - 1];
            wd = Width(f, buf, n);
            if (op->align == 1)
                x -= wd;
            else if (op->align == 2)
                x -= wd / 2;
            else if (op->align == 3)   // (centred on the Game Boy's 8-pixel grid)
                x = ((x - wd / 2) / 8) * 8;
            regs[(op->flags >> 1) & 7] = Line(s, f, buf, n, x, y, op->ink, op->shadow, op->outline,
                                              op->h ? y + op->h : 0x7FFF) - ox;
            break;
        }
        case OP_MSG:
            Msg(s, p, op, d, x, y, oy, regs);
            break;
        case OP_CHOICES:
            Choices(s, p, op, d, x, y);
            break;
        case OP_SNAP:   // (the last field or battle frame shown: a style whose menus open over it)
            if (s == &fr && sSnapOk)
            {
                int i;
                for (i = 0; i < DISPLAY_WIDTH * DISPLAY_HEIGHT; i++)
                    frame[i] = Fade(&fr, sSnap[i]) | 0x8000;
            }
            break;
        case OP_BAR:
            DrawBar(s, p, op, op->a < NFIELDS ? d->frac[op->a] : 0, x, y);
            break;
        case OP_BACKING:
            backing = op;
            break;
        case OP_ROWMAP:
            rowmap = op;
            break;
        }
    }
    sGlyphEvb = 0;
    if (s == &cv)
    {
        // (the canvas: its rows kept, placed in the spec's W x H, the backing behind its drawn pixels)
        int nRows = cv.h, px = 0, py = 0, bx0 = 9999, by0 = 9999, bx1 = -1, by1 = -1, i, j;
        const u8 *rows = NULL;
        if (rowmap && rowmap->a < p->nStr)
        {
            rows = p->str[rowmap->a];
            nRows = p->strLen[rowmap->a];
            px = rowmap->x, py = rowmap->y;
        }
        for (j = 0; j < nRows; j++)
        {
            int sy = rows ? rows[j] : j;
            if (sy >= cv.h)
                continue;
            for (i = 0; i < cv.w; i++)
                if (sCanvasA[sy * cv.w + i])
                {
                    if (i < bx0) bx0 = i;
                    if (i > bx1) bx1 = i;
                    if (j < by0) by0 = j;
                    if (j > by1) by1 = j;
                }
        }
        if (backing && bx1 >= 0)
        {
            int r = backing->b;
            int x0 = px + bx0 - r, y0 = py + by0 - r, x1 = px + bx1 + 1 + r, y1 = py + by1 + 1 + r;
            if (x0 < 0) x0 = 0;
            if (y0 < 0) y0 = 0;
            if (x1 > sp->w) x1 = sp->w;
            if (y1 > sp->h) y1 = sp->h;
            for (j = y0; j < y1; j++)
                for (i = x0; i < x1; i++)
                    Put(&fr, ax + i, ay + j, backing->ink, backing->a);
        }
        for (j = 0; j < nRows; j++)
        {
            int sy = rows ? rows[j] : j;
            if (sy >= cv.h)
                continue;
            for (i = 0; i < cv.w; i++)
            {
                u8 a = sCanvasA[sy * cv.w + i];
                if (a)
                    Put(&fr, ax + px + i, ay + py + j, sCanvasPx[sy * cv.w + i], a);
            }
        }
    }
}

// ---------------------------------------------------------------------------------------------------- numbers, names
static u8 *Digits(u8 *dst, s32 n, int width)
{
    u8 tmp[12];
    int k = 0, i;
    if (n < 0)
        n = 0;
    do
        tmp[k++] = CHAR_0 + n % 10, n /= 10;
    while (n && k < 11);
    for (i = k; i < width; i++)
        *dst++ = CHAR_SPACE;
    while (k)
        *dst++ = tmp[--k];
    *dst = EOS;
    return dst;
}

// ---------------------------------------------------------------------------------------------------- the battle's boxes
struct Hud
{
    bool8 used;
    u8 name[POKEMON_NAME_LENGTH + 12];
    u8 level, gender, status;   // (gender 0 none, 1 male, 2 female; status 0 none, 1 PSN 2 PAR 3 SLP 4 FRZ 5 BRN)
    bool8 caught;               // (a wild foe whose kind is caught: battle_interface.c TryAddPokeballIconToHealthbox)
    s32 hp, maxHp;
    s32 exp, expMax;
};
static struct Hud sHud[MAX_BATTLERS_COUNT];

bool8 MP_UiStyle_DrawsHealthboxes(void)
{
    return MP_UiStyle_Active() != UISTYLE_KAI;
}

static u8 StatusOf(u32 s)
{
    if (s & STATUS1_SLEEP)
        return 3;
    if (s & STATUS1_PSN_ANY)
        return 1;
    if (s & STATUS1_BURN)
        return 5;
    if (s & STATUS1_FREEZE)
        return 4;
    if (s & STATUS1_PARALYSIS)
        return 2;
    return 0;
}

static void ExpOf(struct Pokemon *mon, s32 *exp, s32 *max)
{
    u16 species = GetMonData(mon, MON_DATA_SPECIES);
    u8 level = GetMonData(mon, MON_DATA_LEVEL);
    u32 e = GetMonData(mon, MON_DATA_EXP), lo, hi;
    *exp = 0, *max = 1;
    if (species == SPECIES_NONE || species >= NUM_SPECIES || level >= MAX_LEVEL)
        return;
    lo = gExperienceTables[gSpeciesInfo[species].growthRate][level];
    hi = gExperienceTables[gSpeciesInfo[species].growthRate][level + 1];
    if (hi <= lo)
        return;
    *exp = e > lo ? (s32)(e - lo) : 0;
    *max = hi - lo;
    if (*exp > *max)
        *exp = *max;
}

void MP_UiStyle_HudUpdate(u8 battler, struct Pokemon *mon, bool8 hpToo)
{
    struct Hud *h;
    const u8 *name;
    u16 species;
    if (battler >= MAX_BATTLERS_COUNT || mon == NULL)
        return;
    h = &sHud[battler];
    h->used = TRUE;
    GetMonData(mon, MON_DATA_NICKNAME, h->name);
    h->name[POKEMON_NAME_LENGTH] = EOS;
    StringGet_Nickname(h->name);
    name = MP_TextOpts_Filter(h->name, WINDOWS_MAX);   // (OPTIONS > NAME CASE / CREATURES, as the healthbox's own)
    if (name != h->name)
    {
        int n = 0;
        while (n < (int)sizeof(h->name) - 1 && name[n] != EOS)
            h->name[n] = name[n], n++;
        h->name[n] = EOS;
    }
    h->level = GetMonData(mon, MON_DATA_LEVEL);
    h->status = StatusOf(GetMonData(mon, MON_DATA_STATUS));
    switch (GetMonGender(mon))
    {
    case MON_MALE:   h->gender = 1; break;
    case MON_FEMALE: h->gender = 2; break;
    default:         h->gender = 0; break;
    }
    species = GetMonData(mon, MON_DATA_SPECIES);
    h->caught = GetBattlerSide(battler) == B_SIDE_OPPONENT
             && !(gBattleTypeFlags & (BATTLE_TYPE_TRAINER | BATTLE_TYPE_WALLY_TUTORIAL))
             && GetSetPokedexFlag(SpeciesToNationalPokedexNum(species), FLAG_GET_CAUGHT);
    if ((species == SPECIES_NIDORAN_F || species == SPECIES_NIDORAN_M) && StringCompare(h->name, gSpeciesNames[species]) == 0)
        h->gender = 0;
    if (hpToo)
    {
        h->hp = GetMonData(mon, MON_DATA_HP);
        h->maxHp = GetMonData(mon, MON_DATA_MAX_HP);
        ExpOf(mon, &h->exp, &h->expMax);
    }
}

void MP_UiStyle_HudBar(u8 battler, s32 hp, s32 maxHp)
{
    if (battler >= MAX_BATTLERS_COUNT)
        return;
    sHud[battler].hp = hp < 0 ? 0 : hp;
    sHud[battler].maxHp = maxHp;
}

void MP_UiStyle_HudExp(u8 battler, s32 exp, s32 expMax)
{
    if (battler >= MAX_BATTLERS_COUNT)
        return;
    sHud[battler].exp = exp < 0 ? 0 : exp;
    sHud[battler].expMax = expMax > 0 ? expMax : 1;
}

// where each kind of box goes (tools/ui_styles/layout.py: KAI's own, measured from the game's screens) and where the
// healthbox sprite rests that it stands for (battle_interface.c InitBattlerHealthboxCoords)
static const s16 sSingleRect[2][2] = {{127, 74}, {13, 16}};       // [side] mine, foe
static const s16 sSingleHome[2][2] = {{158, 88}, {44, 30}};
static const s16 sDoubleRect[4][2] = {{128, 62}, {13, 5}, {139, 87}, {1, 30}};    // by position
static const s16 sDoubleHome[4][2] = {{159, 76}, {44, 19}, {171, 101}, {32, 44}};
static const s16 sTripleFoe[3][2] = {{1, 1}, {1, 22}, {1, 43}};
static const s16 sTripleMine[3][2] = {{164, 48}, {164, 69}, {164, 90}};

static void BattleHuds(struct UiPack *p, u16 *frame)
{
    u8 b;
    for (b = 0; b < gBattlersCount && b < MAX_BATTLERS_COUNT; b++)
    {
        struct Hud *h = &sHud[b];
        struct Sprite *box;
        struct UiData d;
        const struct UiSpec *sp;
        struct MpFadeFit fit;
        u8 side, pos, lv[12], hp[12], mhp[12], hp3[12], mhp3[12];
        s16 x, y;
        const char *kind;
        char name[24];

        if (!h->used || gHealthboxSpriteIds[b] >= MAX_SPRITES)
            continue;
        if ((gBattleTypeFlags & BATTLE_TYPE_SAFARI) && GetBattlerSide(b) == B_SIDE_PLAYER)
            continue;   // (the SAFARI ZONE's own box)
        box = &gSprites[gHealthboxSpriteIds[b]];
        if (!box->inUse || box->invisible)
            continue;
        side = GetBattlerSide(b);
        pos = GetBattlerPosition(b);
        if (IS_TRIPLE_BATTLE())
        {
            static const u8 sFoeRow[MAX_POSITION_COUNT] = {
                [B_POSITION_OPPONENT_RIGHT] = 0, [B_POSITION_OPPONENT_CENTER] = 1, [B_POSITION_OPPONENT_LEFT] = 2,
            };
            static const u8 sTeamRow[MAX_POSITION_COUNT] = {
                [B_POSITION_PLAYER_LEFT] = 0, [B_POSITION_PLAYER_CENTER] = 1, [B_POSITION_PLAYER_RIGHT] = 2,
            };
            if (pos >= MAX_POSITION_COUNT)
                continue;
            kind = side == B_SIDE_OPPONENT ? "hud.foe3" : "hud.mine3";
            x = (side == B_SIDE_OPPONENT ? sTripleFoe[sFoeRow[pos]][0] : sTripleMine[sTeamRow[pos]][0]) + box->x2;
            y = (side == B_SIDE_OPPONENT ? sTripleFoe[sFoeRow[pos]][1] : sTripleMine[sTeamRow[pos]][1]) + box->y2;
        }
        else if (IsDoubleBattle())
        {
            if (pos >= 4)
                continue;
            kind = side == B_SIDE_OPPONENT ? "hud.foe2" : "hud.mine2";
            x = sDoubleRect[pos][0] + box->x + box->x2 - sDoubleHome[pos][0];
            y = sDoubleRect[pos][1] + box->y + box->y2 - sDoubleHome[pos][1];
        }
        else
        {
            kind = side == B_SIDE_OPPONENT ? "hud.foe1" : "hud.mine1";
            x = sSingleRect[side][0] + box->x + box->x2 - sSingleHome[side][0];
            y = sSingleRect[side][1] + box->y + box->y2 - sSingleHome[side][1];
        }
        // (a style's box for this spot on the screen when it has one: Black's gauges stand differently per spot)
        snprintf(name, sizeof(name), "%s.%d", kind, pos);
        if ((sp = FindSpec(p, name)) == NULL && (sp = FindSpec(p, kind)) == NULL)
            continue;
        memset(&d, 0, sizeof(d));
        d.str[F_NAME] = h->name;
        Digits(lv, h->level, 0);
        Digits(hp, h->hp, 0);
        Digits(mhp, h->maxHp, 0);
        Digits(hp3, h->hp, 3);
        Digits(mhp3, h->maxHp, 3);
        d.str[F_LEVEL] = lv, d.str[F_HP] = hp, d.str[F_MAXHP] = mhp, d.str[F_HP3] = hp3, d.str[F_MAXHP3] = mhp3;
        d.frac[F_HPFRAC] = h->maxHp > 0 ? (s32)(((s64)h->hp << 16) / h->maxHp) : 0;
        d.frac[F_EXP] = h->expMax > 0 ? (s32)(((s64)h->exp << 16) / h->expMax) : 0;
        d.flags = (h->level >= 100 ? C_LV3 : h->level < 10 ? C_LV1 : 0)
                | (h->gender == 1 ? C_MALE : h->gender == 2 ? C_FEMALE : 0)
                | (h->status ? (C_STATUS | (C_PSN << (h->status - 1))) : 0)
                | (side == B_SIDE_PLAYER ? C_EXP : 0)    // (a style's box shows it where its game does)
                | (h->caught ? C_CAUGHT : 0)
                | (gMpConfig.uiNoWhiteBg ? C_NOBG : 0);   // (RED's and CRYSTAL's boxes: OPTIONS > DISPLAY > WHITE BG)
        MP_FadeFit_FromPalette(&fit, OBJ_PLTT_ID(box->oam.paletteNum));
        DrawSpec(p, sp, &d, frame, x, y, &fit);
    }
}

// ---------------------------------------------------------------------------------------------------- the battle's panel
static u8 sChooser;   // the player's battler choosing now (battle_controller_player.c)

void MP_UiStyle_SetChooser(u8 battler)
{
    sChooser = battler < MAX_BATTLERS_COUNT ? battler : 0;
}

static u16 Bg0VOffset(void)
{
    return *(vu16 *)REG_ADDR_BG0VOFS & 0x1FF;
}

static bool8 InBattle(void)
{
    bool8 MP_DexCaughtPageShown(void);   // (pokedex.c: a catch's MOéDEX page over the battle, not the battle's own)
    return gMain.inBattle && gMain.callback2 == BattleMainCB2 && !MP_DexCaughtPageShown();
}

static bool8 InField(void)
{
    return !gMain.inBattle && gMain.callback2 == CB2_Overworld;
}

// FIGHT BAG / POKéMON RUN as the game printed them: its words split at its CLEAR_TOs and new lines
static void SplitMenu(const u8 *t, int len, u8 out[4][24])
{
    int i, k = 0, n = 0;
    for (i = 0; i < 4; i++)
        out[i][0] = EOS;
    for (i = 0; i < len && k < 4; i++)
    {
        u8 c = t[i];
        if (c == EXT_CTRL_CODE_BEGIN && i + 1 < len)
        {
            u8 code = t[i + 1];
            i += 1 + CtrlArgs(code);
            if (code == EXT_CTRL_CODE_CLEAR_TO && n)
                out[k][n] = EOS, k++, n = 0;
            continue;
        }
        if (c == CHAR_NEWLINE)
        {
            if (n)
                out[k][n] = EOS, k++, n = 0;
            continue;
        }
        if (n < 23)
            out[k][n++] = c, out[k][n] = EOS;
    }
}

static void BattlePanel(struct UiPack *p, u16 *frame)
{
    struct UiData d;
    struct MpFadeFit fit;
    const struct UiSpec *sp = NULL;
    u16 vofs = Bg0VOffset();
    u8 labels[4][24], pp[12], mpp[12], w;
    bool8 waiting = FALSE;

    memset(&d, 0, sizeof(d));
    for (w = 0; w < 11; w++)
    {
        bool8 wt;
        d.msg[w] = sCap[w].text;
        d.msgLen[w] = sCap[w].len;
        d.msgPrinted[w] = Printed(w, &wt);
        d.msgHold[w] = wt;
        if (w == B_WIN_MSG)
            waiting = wt;
    }

    if (vofs == 0)
        sp = FindSpec(p, "b.msg");
    else if (vofs == DISPLAY_HEIGHT)
    {
        int i;
        sp = FindSpec(p, "b.cmd");
        static const u8 sFight[] = _("FIGHT"), sBag[] = _("BAG"), sMon[] = _("POKéMON"), sMoe[] = _("MOéMON"),
                        sRun[] = _("RUN");
        SplitMenu(sCap[B_WIN_ACTION_MENU].text, sCap[B_WIN_ACTION_MENU].len, labels);
        for (i = 0; i < 4; i++)
            d.str[F_ITEM0 + i] = labels[i];
        if (StringCompare(labels[0], sFight) == 0 && StringCompare(labels[1], sBag) == 0
         && StringCompare(labels[3], sRun) == 0
         && (StringCompare(labels[2], sMon) == 0 || StringCompare(labels[2], sMoe) == 0))
            d.flags |= C_STDCMD | (StringCompare(labels[2], sMoe) == 0 ? C_MOEWORD : 0);
        d.flags |= C_SEL0 << (gActionSelectionCursor[sChooser] & 3);
    }
    else if (vofs == DISPLAY_HEIGHT * 2)
    {
        // (the moves as the menu was given them: once a move is picked the battle reuses that buffer while the
        // panel is still up, so the last one seen choosing is kept)
        static struct ChooseMoveStruct sMoves;
        struct ChooseMoveStruct *mi = &sMoves;
        u8 cur = gMoveSelectionCursor[sChooser] & 3, i, to, held = 0xFF;
        if (MP_BattleChoosingMove(sChooser))
            memcpy(&sMoves, &gBattleBufferA[sChooser][4], sizeof(sMoves));
        u16 move;
        // (SELECT's swap: the cursor on the move it would swap with, the held one marked: C_HELD0)
        if (MP_BattleMoveSwapping(sChooser, &to))
            held = cur, cur = to & 3;
        move = mi->moves[cur];
        sp = FindSpec(p, "b.moves");
        for (i = 0; i < 4; i++)
        {
            d.str[F_ITEM0 + i] = sCap[B_WIN_MOVE_NAME_1 + i].text;
            d.val[F_ITEM0 + 8 + i] = mi->moves[i] ? gBattleMoves[mi->moves[i]].type : 0;
        }
        Digits(pp, mi->currentPP[cur], 0);
        Digits(mpp, mi->maxPP[cur], 0);
        d.str[F_PP] = pp, d.str[F_MAXPP] = mpp;
        if (move && move < MOVES_COUNT)
        {
            d.str[F_TYPE] = gTypeNames[gBattleMoves[move].type];
            d.val[F_TYPE] = gBattleMoves[move].type;
        }
        d.flags |= C_SEL0 << cur;
        if (held != 0xFF && held != cur)
            d.flags |= C_HELD0 << held;
        // (R's view of the move or START's gimmick line in the type's place)
        if (gMpUiMoveTypeAlt)
            d.flags |= C_ALT;
        if (gMpUiMoveTypeAlt && !MP_MoveInfoView())
        {
            // the gimmick: its words on two short lines ("START:" and the gimmick START picks, or the one chosen)
            static const u8 sPrefix[] = _("START: ");
            static const u8 sStart[] = _("START:");
            static u8 sAlt[48];
            const u8 *t = sCap[B_WIN_MOVE_TYPE].text;
            u8 *e = sAlt;
            int i, plen = StringLength(sPrefix);
            while (*t == EXT_CTRL_CODE_BEGIN && t[1] != EOS)
                t += 2 + CtrlArgs(t[1]);
            if (StringCompareN(t, sPrefix, plen) == 0)
            {
                e = StringCopy(e, sStart);
                *e++ = CHAR_NEWLINE;
                t += plen;
                for (i = 0; t[i] != EOS && t[i] != CHAR_SPACE && i < 20; i++)
                    *e++ = t[i];
            }
            else
            {
                for (i = 0; t[i] != EOS && i < 40; i++)
                    *e++ = (t[i] == CHAR_SPACE && !memchr(sAlt, CHAR_NEWLINE, e - sAlt)) ? CHAR_NEWLINE : t[i];
            }
            *e = EOS;
            d.msg[B_WIN_MOVE_TYPE] = sAlt;
            d.msgLen[B_WIN_MOVE_TYPE] = d.msgPrinted[B_WIN_MOVE_TYPE] = e - sAlt;
        }
        if (MP_MoveInfoView() && move && move < MOVES_COUNT)
        {
            // its category and power, its accuracy: two short lines (the type's two)
            static u8 sAlt[48];
            const struct BattleMove *m = &gBattleMoves[move];
            char buf[48];
            const char *cat = m->power == 0 ? "STATUS" : IS_TYPE_PHYSICAL(m->type) ? "PHYS." : "SPEC.";
            char pow[8], acc[8];
            if (m->power > 1)
                snprintf(pow, sizeof(pow), "%d", m->power);
            else
                snprintf(pow, sizeof(pow), "--");
            if (m->accuracy)
                snprintf(acc, sizeof(acc), "%d%%", m->accuracy);
            else
                snprintf(acc, sizeof(acc), "--");
            u8 *e;
            memset(sAlt, EOS, sizeof(sAlt));
            snprintf(buf, sizeof(buf), "%s %s", cat, pow);
            MP_AsciiToGame(sAlt, buf, 20);
            e = sAlt + StringLength(sAlt);
            *e++ = CHAR_NEWLINE;
            snprintf(buf, sizeof(buf), "ACC. %s", acc);
            MP_AsciiToGame(e, buf, 20);
            d.msg[B_WIN_MOVE_TYPE] = sAlt;
            d.msgLen[B_WIN_MOVE_TYPE] = d.msgPrinted[B_WIN_MOVE_TYPE] = StringLength(sAlt);
            d.flags |= C_ALT;
        }
    }
    if (sp == NULL)
        return;
    if (waiting)
        d.flags |= C_WAIT;
    MP_FadeFit_FromPalette(&fit, BG_PLTT_ID(0));
    DrawSpec(p, sp, &d, frame, 0, 0, &fit);
}

// ---------------------------------------------------------------------------------------------------- the field
// a style's own value for entry ENTRY of a menu (its map.<menu> spec: START's icons), DEF when it has none
static int Lookup(const struct UiPack *p, const char *menu, u8 entry, int def)
{
    char name[24];
    const struct UiSpec *sp;
    u16 k;
    snprintf(name, sizeof(name), "map.%s", menu);
    if ((sp = FindSpec(p, name)) == NULL)
        return def;
    for (k = 0; k < sp->nOps; k++)
        if (sp->ops[k].b == entry)
            return sp->ops[k].c;
    return def;
}

// a style's own words for entry ENTRY of a menu (its words.<menu> spec: Crystal's MENU ACCOUNT, FireRed's help bar),
// line LINE, as the game's text options show them (MOéMON / POKéMON), into OUT
static const u8 *Words(const struct UiPack *p, const char *menu, u8 entry, u8 line, u8 *out, int cap)
{
    char name[24];
    const struct UiSpec *sp;
    const u8 *f;
    u16 k;
    int n;
    snprintf(name, sizeof(name), "words.%s", menu);
    if ((sp = FindSpec(p, name)) == NULL)
        return NULL;
    for (k = 0; k < sp->nOps; k++)
    {
        const struct UiOp *op = &sp->ops[k];
        if (op->kind != OP_TEXT || op->b != entry || op->c != line || op->a >= p->nStr)
            continue;
        n = p->strLen[op->a] < cap - 1 ? p->strLen[op->a] : cap - 1;
        memcpy(out, p->str[op->a], n);
        out[n] = EOS;
        f = MP_TextOpts_Filter(out, WINDOWS_MAX);
        if (f != out)
        {
            for (n = 0; n < cap - 1 && f[n] != EOS; n++)
                out[n] = f[n];
            out[n] = EOS;
        }
        return out;
    }
    return NULL;
}

static void StartMenu(struct UiPack *p, u16 *frame)
{
    u8 actions[16], count = 0, cursor = 0, i;
    const u8 *names[16];
    u8 expanded[16][32];
    char specName[24];
    const struct UiSpec *sp;
    struct UiData d;
    struct MpFadeFit fit;

    if (!MP_StartMenu_State(actions, &count, &cursor, names) || count == 0)
        return;
    if (GetStartMenuWindowId() < WINDOWS_MAX)
        sHandled |= 1u << GetStartMenuWindowId();
    snprintf(specName, sizeof(specName), "start.%d", count > 10 ? 10 : count);
    if ((sp = FindSpec(p, specName)) == NULL)
        return;
    memset(&d, 0, sizeof(d));
    for (i = 0; i < count && i < 10; i++)
    {
        const u8 *f;
        int n;
        StringExpandPlaceholders(expanded[i], names[i]);
        f = MP_TextOpts_Filter(expanded[i], WINDOWS_MAX);   // (as START prints them: MOéMON / POKéMON)
        for (n = 0; f != expanded[i] && n < (int)sizeof(expanded[i]) - 1 && f[n] != EOS; n++)
            expanded[i][n] = f[n];
        if (f != expanded[i])
            expanded[i][n] = EOS;
        d.str[F_ITEM0 + i] = expanded[i];
        d.val[F_ICON0 + i] = Lookup(p, "start", actions[i], 0);
    }
    if (cursor < count)
    {
        static u8 sLine[2][64];
        d.str[F_TEXT0 + 30] = Words(p, "start", actions[cursor], 0, sLine[0], sizeof(sLine[0]));
        d.str[F_TEXT0 + 31] = Words(p, "start", actions[cursor], 1, sLine[1], sizeof(sLine[1]));
    }
    d.flags = C_SEL0 << (cursor < 10 ? cursor : 0);
    MP_FadeFit_FromPalette(&fit, BG_PLTT_ID(15));
    DrawSpec(p, sp, &d, frame, 0, 0, &fit);
}

// the field's message box: window 0, the standard text box (InitStandardTextBoxWindows), its frame up
static bool8 FieldMessageShown(void)
{
    const struct WindowTemplate *w = &gWindows[0].window;
    if (!InField() || !sFramed[0] || w->bg != 0 || w->tilemapTop != 15 || w->width != 27)
        return FALSE;
    return TRUE;
}

// window W's words in the style's message box (the field's, or a screen's at the foot of the screen)
static void MessageBox(struct UiPack *p, u16 *frame, u8 w)
{
    const struct UiSpec *sp = FindSpec(p, "f.msg");
    struct UiData d;
    struct MpFadeFit fit;
    bool8 waiting;
    if (sp == NULL)
        return;
    memset(&d, 0, sizeof(d));
    d.msg[0] = sCap[w].text;
    d.msgLen[0] = sCap[w].len;
    d.msgPrinted[0] = Printed(w, &waiting);
    d.msgHold[0] = waiting;
    if (waiting)
        d.flags |= C_WAIT;
    MP_FadeFit_FromPalette(&fit, BG_PLTT_ID(15));
    DrawSpec(p, sp, &d, frame, 0, 0, &fit);
    sHandled |= 1u << w;
}

static void FieldMessage(struct UiPack *p, u16 *frame)
{
    MessageBox(p, frame, 0);
}

// only SP's words (a screen's words over Kai's sprites, as the original screens print theirs over their icons)
static void DrawSpecText(struct UiPack *p, const struct UiSpec *sp, struct UiData *d, u16 *frame,
                         const struct MpFadeFit *fit)
{
    sTextOnly = TRUE;
    DrawSpec(p, sp, d, frame, 0, 0, fit);
    sTextOnly = FALSE;
}

// ---------------------------------------------------------------------------------------------------- full screens
// A style's own screen (the party...) drawn over the whole frame: Kai's sprites are drawn again over it (some where the
// style puts them: its ICON ops), and Kai's pop-up windows (a menu of what to do, a question) put back over it as Kai
// drew them, so everything the screen does stays in reach.
static u16 sKaiFrame[DISPLAY_WIDTH * DISPLAY_HEIGHT];   // (the frame as Kai drew it, before the style's)

// sprite S drawn over FRAME, moved by (dx, dy)
static void RedrawSprite(u16 *frame, const struct Sprite *s, int dx, int dy)
{
    static const u8 sDims[3][4][2] = {
        {{8, 8}, {16, 16}, {32, 32}, {64, 64}},
        {{16, 8}, {32, 8}, {32, 16}, {64, 32}},
        {{8, 16}, {8, 32}, {16, 32}, {32, 64}},
    };
    const u8 *tiles;
    const u16 *pal;
    int w, h, x0, y0, px, py;
    if (!s->inUse || s->invisible || s->mpNoDraw || s->oam.affineMode != ST_OAM_AFFINE_OFF || s->oam.shape > 2
     || s->oam.bpp != ST_OAM_4BPP || s->oam.objMode == ST_OAM_OBJ_WINDOW)
        return;
    w = sDims[s->oam.shape][s->oam.size][0];
    h = sDims[s->oam.shape][s->oam.size][1];
    x0 = (s->oam.x >= 256 ? s->oam.x - 512 : s->oam.x) + dx;
    y0 = (s->oam.y >= 160 ? s->oam.y - 256 : s->oam.y) + dy;
    tiles = (const u8 *)(OBJ_VRAM0) + s->oam.tileNum * TILE_SIZE_4BPP;
    pal = (const u16 *)(OBJ_PLTT) + s->oam.paletteNum * 16;
    for (py = 0; py < h; py++)
    {
        int sy = y0 + py, ty = s->oam.matrixNum & ST_OAM_VFLIP ? h - 1 - py : py;
        if (sy < 0 || sy >= DISPLAY_HEIGHT)
            continue;
        for (px = 0; px < w; px++)
        {
            int sx = x0 + px, tx = s->oam.matrixNum & ST_OAM_HFLIP ? w - 1 - px : px;
            u8 v;
            if (sx < 0 || sx >= DISPLAY_WIDTH)
                continue;
            v = tiles[((ty / 8) * (w / 8) + tx / 8) * TILE_SIZE_4BPP + (ty % 8) * 4 + (tx % 8) / 2];
            v = (tx & 1) ? (v >> 4) : (v & 0xF);
            if (v)
                frame[sy * DISPLAY_WIDTH + sx] = pal[v] | 0x8000;
        }
    }
}

// every sprite shown, in the order the GBA draws them (lower priority under), but those in SKIP (their ids + 1)
static void RedrawSprites(u16 *frame, const u8 *skip, int nSkip)
{
    int pr, i, k;
    for (pr = 3; pr >= 0; pr--)
        for (i = MAX_SPRITES - 1; i >= 0; i--)
        {
            const struct Sprite *sp = &gSprites[i];
            if (!sp->inUse || sp->invisible || sp->mpNoDraw || sp->oam.priority != pr)
                continue;
            for (k = 0; k < nSkip && skip[k] != i + 1; k++)
                ;
            if (k == nSkip)
                RedrawSprite(frame, sp, 0, 0);
        }
}

// Kai's window W put back over the frame as Kai drew it, with its frame (a pop-up over a style's screen)
static void PasteKaiWindow(u16 *frame, u8 w)
{
    const struct WindowTemplate *t;
    int x0, y0, x1, y1, x, y;
    if (w >= WINDOWS_MAX || gWindows[w].tileData == NULL)
        return;
    t = &gWindows[w].window;
    if (t->bg > 3 || t->width == 0)
        return;
    x0 = (t->tilemapLeft - 1) * 8, y0 = (t->tilemapTop - 1) * 8;
    x1 = (t->tilemapLeft + t->width + 1) * 8, y1 = (t->tilemapTop + t->height + 1) * 8;
    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 > DISPLAY_WIDTH) x1 = DISPLAY_WIDTH;
    if (y1 > DISPLAY_HEIGHT) y1 = DISPLAY_HEIGHT;
    for (y = y0; y < y1; y++)
        for (x = x0; x < x1; x++)
            frame[y * DISPLAY_WIDTH + x] = sKaiFrame[y * DISPLAY_WIDTH + x];
}

// OPTIONS > DISPLAY > MENU POKéMON: the style's own party icon for SPECIES (an EGG: EGG), in its set (0 ORIGINAL, 1
// EXTENDED), drawn with its middle at (X, Y); FALSE when the setting is KAI's or the style has none
static bool8 StyleMonIcon(struct UiPack *p, struct Surf *s, u16 species, bool8 egg, u8 frameNo, int x, int y)
{
    const struct UiSpec *sp;
    u8 want = gMpConfig.uiMenuMons, set;
    u16 k;
    if (want == MP_MENU_MONS_KAI || (sp = FindSpec(p, "icons")) == NULL)
        return FALSE;
    // (EXTENDED where the style has it, else ORIGINAL)
    for (set = want == MP_MENU_MONS_EXTENDED ? 1 : 0, k = 0; ; k++)
    {
        const struct UiOp *op;
        u16 nat, len;
        u32 id;
        if (k >= sp->nOps)
        {
            if (set == 0)
                return FALSE;
            set = 0, k = 0;
        }
        op = &sp->ops[k];
        if (op->kind != OP_ICONSET || op->d != set || op->a >= p->nStr)
            continue;
        len = p->strLen[op->a];
        if (len < 2)
            return FALSE;
        nat = egg ? 0 : SpeciesToNationalPokedexNum(species);
        id = (nat > 0 && nat < len - 1) ? p->str[op->a][nat] : egg ? p->str[op->a][0] : p->str[op->a][len - 1];
        if (id >= op->c)
            return FALSE;
        id = op->b + 2 * id + (frameNo & 1);
        if (id >= p->nImg)
            return FALSE;
        DrawImg(s, &p->img[id], x - p->img[id].w / 2, y - p->img[id].h / 2);
        return TRUE;
    }
}

// OPTIONS: whether STYLE's pack has the party icons of SET (0 ORIGINAL, 1 EXTENDED)
bool8 MP_UiStyle_HasIconSet(u8 style, u8 set)
{
    struct UiPack *p = GetPack(style);
    const struct UiSpec *sp = p ? FindSpec(p, "icons") : NULL;
    u16 k;
    for (k = 0; sp && k < sp->nOps; k++)
        if (sp->ops[k].kind == OP_ICONSET && sp->ops[k].d == set)
            return TRUE;
    return FALSE;
}

// where a spec's ICON ops whose conditions hold put Kai's sprites: OUT[a] = (x, y) of the sprite's middle, -1 none
static void IconPlaces(const struct UiSpec *sp, u32 flags, s16 out[5][2])
{
    u16 k;
    for (k = 0; k < 5; k++)
        out[k][0] = out[k][1] = -1;
    for (k = 0; k < sp->nOps; k++)
    {
        const struct UiOp *op = &sp->ops[k];
        if (op->kind != OP_ICON || op->a >= 5 || (flags & op->cond) != op->cond || (flags & op->unless))
            continue;
        out[op->a][0] = op->x, out[op->a][1] = op->y;
    }
}

static const u8 *Filtered(const u8 *str, u8 *buf, int cap)
{
    const u8 *f = MP_TextOpts_Filter(str, WINDOWS_MAX);
    int n;
    for (n = 0; n < cap - 1 && f[n] != EOS; n++)
        buf[n] = f[n];
    buf[n] = EOS;
    return buf;
}

// the style's party screen for every layout a battle uses (single, double, triple, co-op multi: the games these
// styles come from show one party screen whatever the battle); only the multi battle's showcase stays Kai's
static bool8 PartyLayoutStyled(u8 layout)
{
    return layout != PARTY_LAYOUT_MULTI_SHOWCASE;
}

// (party_menu.c: the party a style draws moves as the style's single screen does, whatever its layout)
bool8 MP_UiStyle_PartyStyled(void)
{
    u8 style = MP_UiStyle_Active();
    return style != UISTYLE_KAI && FindSpec(GetPack(style), "party") != NULL && PartyLayoutStyled(gPartyMenu.layout);
}

static bool8 PartyScreenShown(struct MpPartyState *st)
{
    return MP_UiStyle_Active() != UISTYLE_KAI && MP_PartyMenu_GetState(st) && PartyLayoutStyled(st->layout);
}

static void PartyScreen(struct UiPack *p, u16 *frame)
{
    struct MpPartyState st;
    struct UiData d;
    struct MpFadeFit fit;
    const struct UiSpec *sp;
    u8 skip[32], nSkip = 0, i, w;
    s16 places[PARTY_SIZE][5][2];
    bool8 popup;
    static struct UiData sSlotData[PARTY_SIZE];
    const struct UiSpec *slotSpec[PARTY_SIZE];
    bool8 waiting;

    if (!PartyScreenShown(&st) || (sp = FindSpec(p, "party")) == NULL)
        return;
    memcpy(sKaiFrame, frame, sizeof(sKaiFrame));
    MP_FadeFit_FromPalette(&fit, BG_PLTT_ID(0));
    // (a pop-up up: what to do, which move...: Kai's own message window comes back with it, sized for it)
    for (w = 0, popup = FALSE; w < WINDOWS_MAX; w++)
    {
        bool8 own = w == 6 || w == st.msgWin;
        for (i = 0; i < PARTY_SIZE; i++)
            own |= w == st.window[i];
        if (!own && gWindows[w].tileData != NULL && sFramed[w])
            popup = TRUE;
    }
    // the screen: its ground, the message, CANCEL (CONFIRM too while choosing for a battle)
    memset(&d, 0, sizeof(d));
    if (st.msgWin < WINDOWS_MAX && !popup)
    {
        d.msg[0] = sCap[st.msgWin].text;
        d.msgLen[0] = sCap[st.msgWin].len;
        d.msgPrinted[0] = Printed(st.msgWin, &waiting);
    }
    if (st.slot == PARTY_SIZE + 1)
        d.flags |= C_SEL0;
    else if (st.slot == PARTY_SIZE)
        d.flags |= C_SEL0 << 1;
    if (st.chooseHalf)
        d.flags |= C_ALT;
    DrawSpec(p, sp, &d, frame, 0, 0, &fit);
    // each slot
    for (i = 0; i < PARTY_SIZE; i++)
    {
        static u8 sName[PARTY_SIZE][POKEMON_NAME_LENGTH + 12], sLv[PARTY_SIZE][6], sHp[PARTY_SIZE][6],
                  sMax[PARTY_SIZE][6], sHp3[PARTY_SIZE][6], sMax3[PARTY_SIZE][6], sDesc[PARTY_SIZE][32];
        struct Pokemon *mon = &gPlayerParty[i];
        u16 species = GetMonData(mon, MON_DATA_SPECIES);
        char name[16];
        snprintf(name, sizeof(name), "party.%d", i);
        places[i][0][0] = -1;
        slotSpec[i] = NULL;
        if ((sp = FindSpec(p, name)) == NULL)
            continue;
        memset(&d, 0, sizeof(d));
        if (species == SPECIES_NONE)
            d.flags |= P_EMPTY;
        else
        {
            u8 nick[POKEMON_NAME_LENGTH + 12];
            s32 hp = GetMonData(mon, MON_DATA_HP), maxHp = GetMonData(mon, MON_DATA_MAX_HP);
            u8 level = GetMonData(mon, MON_DATA_LEVEL);
            GetMonData(mon, MON_DATA_NICKNAME, nick);
            nick[POKEMON_NAME_LENGTH] = EOS;
            StringGet_Nickname(nick);
            d.str[F_NAME] = Filtered(nick, sName[i], sizeof(sName[i]));
            Digits(sLv[i], level, 0);
            Digits(sHp[i], hp, 0);
            Digits(sMax[i], maxHp, 0);
            Digits(sHp3[i], hp, 3);
            Digits(sMax3[i], maxHp, 3);
            d.str[F_LEVEL] = sLv[i], d.str[F_HP] = sHp[i], d.str[F_MAXHP] = sMax[i];
            d.str[F_HP3] = sHp3[i], d.str[F_MAXHP3] = sMax3[i];
            d.frac[F_HPFRAC] = maxHp > 0 ? (s32)(((s64)hp << 16) / maxHp) : 0;
            d.flags |= level >= 100 ? C_LV3 : level < 10 ? C_LV1 : 0;
            if (GetMonData(mon, MON_DATA_IS_EGG))
                d.flags |= P_EGG;
            else
            {
                u8 g = GetMonGender(mon);
                if (!((species == SPECIES_NIDORAN_F || species == SPECIES_NIDORAN_M)
                   && StringCompare(nick, gSpeciesNames[species]) == 0))
                    d.flags |= g == MON_MALE ? C_MALE : g == MON_FEMALE ? C_FEMALE : 0;
                if (hp == 0)
                    d.flags |= P_FAINT;
            }
            if (st.desc[i])
            {
                d.flags |= P_DESC;
                d.str[F_TEXT0 + 1] = Filtered(st.desc[i], sDesc[i], sizeof(sDesc[i]));
            }
        }
        if (st.statusSprite[i] < MAX_SPRITES && gSprites[st.statusSprite[i]].inUse
         && !gSprites[st.statusSprite[i]].invisible)
            d.flags |= C_STATUS;   // (its status shown: some styles put it in the level's place)
        if (st.slot == i)
            d.flags |= C_SEL0;
        if (st.action == PARTY_ACTION_SWITCH && st.slot2 == i && st.slot != i)
            d.flags |= P_SWITCH;
        DrawSpec(p, sp, &d, frame, 0, 0, &fit);
        IconPlaces(sp, d.flags, places[i]);
        slotSpec[i] = sp;
        sSlotData[i] = d;
    }
    // the party's long messages (WIN_MSG: "... is holding ..."), in the style's message box
    if (sFramed[6] && (sp = FindSpec(p, "f.msg")) != NULL)
    {
        memset(&d, 0, sizeof(d));
        d.msg[0] = sCap[6].text;
        d.msgLen[0] = sCap[6].len;
        d.msgPrinted[0] = Printed(6, &waiting);
        d.msgHold[0] = waiting;
        if (waiting)
            d.flags |= C_WAIT;
        DrawSpec(p, sp, &d, frame, 0, 0, &fit);
    }
    // the sprites: the POKé BALLS the style has its own of stay hidden; each slot's icon, item and status where the
    // style puts them
    for (i = 0; i < PARTY_SIZE && nSkip + 3 < (int)sizeof(skip); i++)
    {
        skip[nSkip++] = st.monSprite[i] + 1;
        skip[nSkip++] = st.itemSprite[i] + 1;
        skip[nSkip++] = st.statusSprite[i] + 1;
    }
    RedrawSprites(frame, skip, nSkip);
    for (i = 0; i < PARTY_SIZE; i++)
    {
        static const s8 sItemDx = 4, sItemDy = 10;
        struct Sprite *mon = st.monSprite[i] < MAX_SPRITES ? &gSprites[st.monSprite[i]] : NULL;
        s16 mx, my;
        if (mon == NULL || !mon->inUse || places[i][0][0] < 0)
            continue;
        mx = places[i][0][0] - mon->x, my = places[i][0][1] - mon->y;
        {
            // (OPTIONS > DISPLAY > MENU POKéMON: the style's own icon, its two frames as Kai's icon turns its own)
            struct Surf is = {frame, NULL, DISPLAY_WIDTH, DISPLAY_HEIGHT, &fit};
            u8 fr = mon->anims ? mon->anims[mon->animNum][mon->animCmdIndex].frame.imageValue : 0;
            if (!StyleMonIcon(p, &is, GetMonData(&gPlayerParty[i], MON_DATA_SPECIES),
                              GetMonData(&gPlayerParty[i], MON_DATA_IS_EGG), fr, places[i][0][0],
                              places[i][0][1] + mon->y2))
                RedrawSprite(frame, mon, mx, my);
        }
        if (st.itemSprite[i] < MAX_SPRITES)
        {
            struct Sprite *it = &gSprites[st.itemSprite[i]];
            if (places[i][1][0] >= 0)
                RedrawSprite(frame, it, places[i][1][0] - it->x, places[i][1][1] - it->y);
            else
                RedrawSprite(frame, it, places[i][0][0] + sItemDx - it->x, places[i][0][1] + sItemDy - it->y);
        }
        if (st.statusSprite[i] < MAX_SPRITES && places[i][2][0] >= 0)
        {
            struct Sprite *ss = &gSprites[st.statusSprite[i]];
            RedrawSprite(frame, ss, places[i][2][0] - ss->x, places[i][2][1] - ss->y);
        }
    }
    for (i = 0; i < PARTY_SIZE; i++)
        if (slotSpec[i])
            DrawSpecText(p, slotSpec[i], &sSlotData[i], frame, &fit);
    // (its own windows drawn; Kai's pop-ups (what to do, which move...) in the style: PopUps)
    sHandled |= 1u << 6;
    if (st.msgWin < WINDOWS_MAX && !popup)
        sHandled |= 1u << st.msgWin;
    for (i = 0; i < PARTY_SIZE; i++)
        if (st.window[i] < WINDOWS_MAX)
            sHandled |= 1u << st.window[i];
}

// OPTIONS in a style: its spec "o" (the screen: tabs, or a page's title; the line under the rows), its OP_LIST's rows
// each drawn with "o.row" from (x, y) H apart, N of them (the style's own scrolling, the cursor's row always shown)
static void OptionsScreen(struct UiPack *p, u16 *frame)
{
    static struct MpOptionsView v;
    static u8 sTop;
    struct UiData d;
    struct MpFadeFit fit;
    const struct UiSpec *sp, *rowSp;
    const struct UiOp *list = NULL;
    u8 title[48], hint[72];
    int vis, k, i;

    if (!MP_Options_View(&v) || (sp = FindSpec(p, "o")) == NULL || (rowSp = FindSpec(p, "o.row")) == NULL)
        return;
    memset(&d, 0, sizeof(d));
    if (v.page)
        d.flags |= S_PAGE;
    else
        d.flags |= C_SEL0 << v.tab;
    if (!v.plainHint)
        d.flags |= S_HINT;
    // (the first list whose conditions hold: a style can show fewer rows while the line under them is up)
    for (k = 0; k < sp->nOps && list == NULL; k++)
        if (sp->ops[k].kind == OP_LIST && (d.flags & sp->ops[k].cond) == sp->ops[k].cond
         && !(d.flags & sp->ops[k].unless))
            list = &sp->ops[k];
    if (list == NULL || list->n == 0)
        return;
    vis = list->n;
    if (v.sel < sTop)
        sTop = v.sel;
    else if (v.sel >= sTop + vis)
        sTop = v.sel - vis + 1;
    if (sTop + vis > v.rows)
        sTop = v.rows > vis ? v.rows - vis : 0;
    MP_FadeFit_FromPalette(&fit, BG_PLTT_ID(0));
    for (i = 0; i < v.nTabs && i < 5; i++)
        d.str[F_ITEM0 + i] = v.tabs[i];
    if (v.page)
        d.str[F_TITLE] = Filtered(v.title, title, sizeof(title));
    d.str[F_HINT] = Filtered(v.hint, hint, sizeof(hint));
    d.msg[0] = d.str[F_HINT];   // (for a style that wraps it: OP_MSG 0)
    d.msgLen[0] = d.msgPrinted[0] = StringLength(d.str[F_HINT]);
    if (sTop > 0)
        d.flags |= S_UP;
    if (sTop + vis < v.rows)
        d.flags |= S_DOWN;
    DrawSpec(p, sp, &d, frame, 0, 0, &fit);
    for (i = 0; i < vis && sTop + i < v.rows; i++)
    {
        static u8 sBuf[40], sLbuf[40], sLabel[48];
        const u8 *label, *names[8];
        u8 cur, n, j;
        struct UiData rd;
        memset(&rd, 0, sizeof(rd));
        n = MP_Options_RowView(sTop + i, &label, names, &cur, sBuf, sLbuf);
        rd.str[F_NAME] = Filtered(label, sLabel, sizeof(sLabel));
        for (j = 0; j < n && j < 8; j++)
            rd.str[F_CHOICE0 + j] = names[j];   // (as written: CREATURES' and NAME CASE's own words)
        rd.val[F_NCHOICE] = n;
        rd.val[F_CUR] = cur;
        if (n)
        {
            rd.str[F_CURSTR] = names[n == 1 ? 0 : cur];
            rd.flags |= O_VALUE;
        }
        if (n >= 2 && n <= 3)
            rd.flags |= O_ALL;
        if (sTop + i == v.sel)
            rd.flags |= C_SEL0;
        DrawSpec(p, rowSp, &rd, frame, list->x, list->y + i * list->h, &fit);
    }
}

// words the style's box can't hold whole (a description longer than its OP_MSG's N lines, and no narrower font for
// it): a page of N lines at a time, turning by itself as Kai's own descriptions do (mp_itemdesc.c: the first page
// 210 frames, each after it 150), from the first page again when the words change
struct AutoPager
{
    u32 sum;
    u16 timer;
};

static void AutoPage(struct UiPack *p, const struct UiSpec *sp, struct UiData *d, int win, struct AutoPager *ap)
{
    const struct UiOp *op = NULL;
    const struct UiFont *f;
    u32 sum = 0;
    int k, nl, pages;
    for (k = 0; k < sp->nOps && op == NULL; k++)
        if (sp->ops[k].kind == OP_MSG && sp->ops[k].a == win && (d->flags & sp->ops[k].cond) == sp->ops[k].cond
         && !(d->flags & sp->ops[k].unless))
            op = &sp->ops[k];
    if (op == NULL || d->msg[win] == NULL || op->n == 0 || op->font >= p->nFont)
        return;
    for (k = 0; k < d->msgLen[win]; k++)
        sum = sum * 31 + d->msg[win][k];
    if (sum != ap->sum)
        ap->sum = sum, ap->timer = 0;
    else if (ap->timer < 0xFFF0)
        ap->timer++;
    f = &p->font[op->font];
    nl = FlowPage(f, d->msg[win], 0, d->msgLen[win], op->w);
    if (nl <= op->n)
        return;
    if (op->e && op->e - 1 < p->nFont && !op->b && FlowPage(&p->font[op->e - 1], d->msg[win], 0, d->msgLen[win],
                                                            op->w) <= op->n)
        return;   // (Msg prints it whole in the narrower font)
    pages = (nl + op->n - 1) / op->n;
    k = ap->timer < 210 ? 0 : 1 + (ap->timer - 210) / 150;
    d->msgFirst[win] = (k % pages) * op->n + 1;
}

// the BAG in a style: its spec "bag" (the screen: the pocket, the description, the item's icon where its ICON 1 says)
// and its OP_LIST's rows, each drawn with "bag.row"; Kai's pop-ups (what to do, how many, SORT) put back over it
static void BagScreen(struct UiPack *p, u16 *frame)
{
    static struct MpBagView v;
    static u16 sTop;
    struct UiData d;
    struct MpFadeFit fit;
    const struct UiSpec *sp, *rowSp;
    const struct UiOp *list = NULL;
    u8 skip[24], nSkip = 0, top, bottom;
    s16 icon[5][2];
    int vis, k, i;

    if (!MP_Bag_View(&v) || (sp = FindSpec(p, "bag")) == NULL || (rowSp = FindSpec(p, "bag.row")) == NULL)
        return;
    memcpy(sKaiFrame, frame, sizeof(sKaiFrame));
    memset(&d, 0, sizeof(d));
    d.flags |= C_SEL0 << v.view;
    if (gSaveBlock2Ptr->playerGender == FEMALE)   // (the style's BAG is the player's own where its game has two)
        d.flags |= C_FEMALE;
    d.val[F_CUR] = v.view;
    for (k = 0; k < sp->nOps && list == NULL; k++)
        if (sp->ops[k].kind == OP_LIST && (d.flags & sp->ops[k].cond) == sp->ops[k].cond
         && !(d.flags & sp->ops[k].unless))
            list = &sp->ops[k];
    if (list == NULL || list->n == 0)
        return;
    vis = list->n;
    if (v.cursor < sTop)
        sTop = v.cursor;
    else if (v.cursor >= sTop + vis)
        sTop = v.cursor - vis + 1;
    if (sTop + vis > v.rows)
        sTop = v.rows > vis ? v.rows - vis : 0;
    if (sTop > 0)
        d.flags |= S_UP;
    if (sTop + vis < v.rows)
        d.flags |= S_DOWN;
    MP_FadeFit_FromPalette(&fit, BG_PLTT_ID(0));
    {
        static u8 sName[24], sDesc[220];
        d.str[F_TITLE] = Filtered(v.pocketName, sName, sizeof(sName));
        d.msg[1] = d.str[F_TITLE];   // (a style that breaks a long pocket name onto two lines: OP_MSG 1)
        d.msgLen[1] = d.msgPrinted[1] = StringLength(d.msg[1]);
        if (v.desc)
        {
            d.msg[0] = Filtered(v.desc, sDesc, sizeof(sDesc));
            d.msgLen[0] = d.msgPrinted[0] = StringLength(d.msg[0]);
        }
    }
    {
        static struct AutoPager sPager;
        AutoPage(p, sp, &d, 0, &sPager);
    }
    DrawSpec(p, sp, &d, frame, 0, 0, &fit);
    for (i = 0; i < vis && sTop + i < v.rows; i++)
    {
        static u8 sRowName[48], sQty[12], sQty2[12], sTag[8];
        struct UiData rd;
        s32 qty;
        const u8 *name;
        memset(&rd, 0, sizeof(rd));
        name = MP_Bag_RowView(sTop + i, &qty, sTag);
        if (name == NULL)
            continue;
        rd.str[F_NAME] = Filtered(name, sRowName, sizeof(sRowName));
        if (v.closeRow && sTop + i == v.rows - 1)
            rd.flags |= B_CLOSE;
        if (qty >= 0)
        {
            Digits(sQty, qty, 0);
            Digits(sQty2, qty, 2);
            rd.str[F_QTY] = sQty, rd.str[F_QTY2] = sQty2;
            rd.flags |= B_QTY;
        }
        if (sTag[0] != EOS)
        {
            rd.str[F_TAG] = sTag;
            rd.flags |= B_TAG;
        }
        if (sTop + i == v.cursor)
            rd.flags |= C_SEL0;
        if (v.swapFrom == sTop + i)
            rd.flags |= B_SWAP;
        DrawSpec(p, rowSp, &rd, frame, list->x, list->y + i * list->h, &fit);
    }
    // the sprites: the BAG's own and its scroll arrows stay hidden (the style draws its own), the item's icon where
    // the style puts it, the rest (a pop-up's arrows) as Kai draws them
    for (i = 0; i < v.nOwn && nSkip < (int)sizeof(skip) - 2; i++)
        skip[nSkip++] = v.own[i] + 1;
    if (v.arrowTask != TASK_NONE)
    {
        MP_ScrollArrows_Sprites(v.arrowTask, &top, &bottom);
        if (top < MAX_SPRITES)
            skip[nSkip++] = top + 1;
        if (bottom < MAX_SPRITES)
            skip[nSkip++] = bottom + 1;
    }
    RedrawSprites(frame, skip, nSkip);
    IconPlaces(sp, d.flags, icon);
    if (icon[1][0] >= 0 && v.itemSprite < MAX_SPRITES && gSprites[v.itemSprite].inUse)
    {
        struct Sprite *it = &gSprites[v.itemSprite];
        RedrawSprite(frame, it, icon[1][0] - (it->x + it->x2), icon[1][1] - (it->y + it->y2));
    }
    // (its own windows drawn; Kai's pop-ups in the style: PopUps)
    sHandled |= (1u << v.listWin) | (1u << v.descWin) | (1u << v.nameWin) | (1u << v.tmWin[0]) | (1u << v.tmWin[1]);
}

// the summary in a style: "sum.<page>" (or "sum") over the whole screen; on the move pages each move's row with
// "sum.<page>.row" where the page's OP_LIST says; the POKé BALL page's lines with "sum.4.line" from its OP_LIST's
// corner, the JOURNAL's rows with "sum.5.row"; the POKéMON's picture, its status and POKé BALL where the page's
// ICON 0 / 2 / 3 say, Kai's pop-ups put back over it
static void SummaryScreen(struct UiPack *p, u16 *frame)
{
    static struct MpSummaryView v;
    static struct MpSummaryWords w;
    static struct UiData d;
    static u8 sMemo[200], sNick[32], sSpecies[24], sOt[16], sItem[32], sPlace[40];
    struct MpFadeFit fit;
    const struct UiSpec *sp, *rowSp;
    const struct UiOp *list = NULL;
    char name[24];
    u8 skip[40], nSkip = 0, i, k, wi;
    s16 icon[5][2];
    u32 status;

    if (!MP_Summary_View(&v))
        return;
    snprintf(name, sizeof(name), "sum.%d", v.page);
    if ((sp = FindSpec(p, name)) == NULL && (sp = FindSpec(p, "sum")) == NULL)
        return;
    memcpy(sKaiFrame, frame, sizeof(sKaiFrame));
    MP_Summary_Words(&w);
    memset(&d, 0, sizeof(d));
    d.str[F_NAME] = Filtered(w.nick, sNick, sizeof(sNick));
    d.str[F_SPECIES] = Filtered(w.species, sSpecies, sizeof(sSpecies));
    d.str[F_LEVEL] = w.level;
    d.str[F_DEXNO] = w.dexNo;
    d.str[F_OT] = Filtered(w.ot, sOt, sizeof(sOt));
    d.str[F_IDNO] = w.idNo;
    d.str[F_HELD] = Filtered(w.item, sItem, sizeof(sItem));
    d.str[F_NATURE] = w.nature;
    d.str[F_ABILITY] = w.ability;
    d.str[F_METLV] = w.metLevel;
    d.str[F_METPLACE] = Filtered(w.metPlace, sPlace, sizeof(sPlace));
    d.str[F_HP] = w.hp, d.str[F_MAXHP] = w.maxHp;
    {
        static u8 sHp3[12], sMax3[12];
        Digits(sHp3, w.hpNow, 3);
        Digits(sMax3, w.hpMax, 3);
        d.str[F_HP3] = sHp3, d.str[F_MAXHP3] = sMax3;
    }
    d.frac[F_HPFRAC] = w.hpMax ? (s32)(((s64)w.hpNow << 16) / w.hpMax) : 0;
    d.frac[F_EXP] = w.expFrac;
    d.str[F_EXPPTS] = w.exp, d.str[F_NEXTLV] = w.nextLv, d.str[F_RIBBONS] = w.ribbons;
    for (i = 0; i < 5; i++)
    {
        d.str[F_STAT0 + i] = w.stat[i];
        if (w.statNature[i] == 1)
            d.flags |= C_SEL0 << i;
        else if (w.statNature[i] == 2)
            d.flags |= C_SEL0 << (5 + i);
    }
    for (i = 0; i < 6; i++)
    {
        d.val[F_IV0 + i] = w.ivStars[i];
        d.str[F_EV0 + i] = w.ev[i];
    }
    d.val[F_TYPE1] = w.type1, d.val[F_TYPE2] = w.type2;
    if (w.type1 != w.type2)
        d.flags |= SM_TWOTYPES;
    d.str[F_POWER] = w.power, d.str[F_ACC] = w.accuracy;
    d.val[F_APPEAL] = w.appeal, d.val[F_JAM] = w.jam;
    d.val[F_PAGENO] = v.page;
    if (w.abilityDesc)
    {
        d.msg[0] = w.abilityDesc;
        d.msgLen[0] = d.msgPrinted[0] = StringLength(w.abilityDesc);
        d.str[F_TEXT0] = w.abilityDesc;   // (a style that prints it on one line)
    }
    if (v.choosing)
    {
        const u8 *desc = v.page == 3 ? w.contestDesc : w.moveDesc;
        d.flags |= SM_CHOOSING;
        if (desc)
        {
            // (Kai's own line breaks out: the style breaks the words for its own box)
            static u8 sDesc[160];
            u16 n;
            for (n = 0; desc[n] != EOS && n < sizeof(sDesc) - 1; n++)
                sDesc[n] = desc[n] == CHAR_NEWLINE ? CHAR_SPACE : desc[n];
            sDesc[n] = EOS;
            d.msg[1] = sDesc;
            d.msgLen[1] = d.msgPrinted[1] = n;
        }
    }
    d.msg[2] = Filtered(w.memo, sMemo, sizeof(sMemo));
    d.msgLen[2] = d.msgPrinted[2] = StringLength(d.msg[2]);
    if (w.memoPlain)
        d.flags |= SM_MEMOPLAIN;
    d.flags |= v.statsView == 1 ? SM_IV : v.statsView == 2 ? SM_EV : 0;
    if (v.statsView == 2 && v.canClearEvs)
        d.flags |= SM_CLEAREV;
    if (v.learning)
        d.flags |= SM_LEARN;
    if (v.canRename)
        d.flags |= SM_RENAME;
    {
        // (the page's title as KAI's words say it: POKéMON or MOéMON as OPTIONS > CREATURES has it)
        static const u8 sT0[] = _("MOéMON INFO"), sT1[] = _("MOéMON SKILLS"), sT2[] = _("BATTLE MOVES"),
                        sT3[] = _("CONTEST MOVES"), sT4[] = _("POKé BALL"), sT5[] = _("JOURNAL");
        static const u8 *const sTitles[6] = {sT0, sT1, sT2, sT3, sT4, sT5};
        static u8 sTitle[24], sWord[24];
        d.str[F_PAGETITLE] = Filtered(sTitles[v.page % 6], sTitle, sizeof(sTitle));
        d.val[F_CREATURES] = Filtered(sT0, sWord, sizeof(sWord))[0] == CHAR_P;
    }
    d.val[F_PROMPT] = v.page == 1 ? 1 + v.statsView : (v.page == 2 || v.page == 3) ? (v.choosing ? 5 : 4)
                    : v.page == 5 ? (v.journalPages > 1 ? 6 : 7) : 0;
    if (v.isEgg)
        d.flags |= SM_EGG;
    {
        u8 level = v.mon ? GetMonData(v.mon, MON_DATA_LEVEL) : 1;
        u8 g = v.mon ? GetMonGender(v.mon) : MON_GENDERLESS;
        u16 species = v.mon ? GetMonData(v.mon, MON_DATA_SPECIES) : 0;
        d.flags |= level >= 100 ? C_LV3 : level < 10 ? C_LV1 : 0;
        if (!v.isEgg && !((species == SPECIES_NIDORAN_F || species == SPECIES_NIDORAN_M)
                          && StringCompare(w.nick, gSpeciesNames[species]) == 0))
            d.flags |= g == MON_MALE ? C_MALE : g == MON_FEMALE ? C_FEMALE : 0;
        status = v.mon ? GetMonData(v.mon, MON_DATA_STATUS) : 0;
        if (v.statusSprite < MAX_SPRITES && gSprites[v.statusSprite].inUse && !gSprites[v.statusSprite].invisible)
            d.flags |= C_STATUS | (1u << (3 + StatusOf(status)));
    }
    if (v.page == 5 && v.mon)
    {
        // (every sentence of the JOURNAL, a line each, laid out in the style's own box and turned a page at a time
        // by A: the style's pages, not Kai's, so none is cut short)
        static u8 sJournal[LIFE_JOURNAL_LINES * (LIFE_JOURNAL_TEXT + 4)];
        static u8 lines[LIFE_JOURNAL_LINES][LIFE_JOURNAL_TEXT];
        static const u8 sNone[] = _("Spend time together in your party to get to know it."), sBullet[] = _("- ");
        const struct UiOp *jop = NULL;
        u16 n = 0, k;
        u8 count = MP_Life_Journal(&v.mon->box, lines, LIFE_JOURNAL_LINES);
        for (i = 0; i < count || (count == 0 && i == 0); i++)
        {
            const u8 *t = count ? lines[i] : sNone;
            if (i > 0 && n < sizeof(sJournal) - 1)
                sJournal[n++] = CHAR_NEWLINE;
            for (k = 0; count && sBullet[k] != EOS && n < sizeof(sJournal) - 1; k++)
                sJournal[n++] = sBullet[k];
            while (*t != EOS && n < sizeof(sJournal) - 1)
                sJournal[n++] = *t++;
        }
        sJournal[n] = EOS;
        d.msg[3] = sJournal;
        d.msgLen[3] = d.msgPrinted[3] = n;
        for (k = 0; k < sp->nOps && jop == NULL; k++)
            if (sp->ops[k].kind == OP_MSG && sp->ops[k].a == 3 && sp->ops[k].font < p->nFont)
                jop = &sp->ops[k];
        sJPages = 1;
        if (jop && jop->n)
        {
            int nl = FlowPage(&p->font[jop->font], sJournal, 0, n, jop->w);
            if (jop->e && jop->e - 1 < p->nFont && !jop->b && nl > jop->n
             && FlowPage(&p->font[jop->e - 1], sJournal, 0, n, jop->w) <= jop->n)
                nl = jop->n;   // (the narrower letters fit it on one page)
            sJPages = (nl + jop->n - 1) / jop->n;
            if (sJPages < 1)
                sJPages = 1;
            if (sJPage >= sJPages)
                sJPage = 0;
            if (sJPages > 1)
                d.msgFirst[3] = sJPage * jop->n + 1;
        }
        if (sJPages > 1)
        {
            static u8 sPage[24];
            u8 *e = Digits(sPage, sJPage + 1, 0);
            *e++ = CHAR_SLASH;
            Digits(e, sJPages, 0);
            d.str[F_JPAGE] = sPage;
            d.flags |= S_DOWN;
        }
        d.val[F_PROMPT] = sJPages > 1 ? 6 : 7;
    }
    {
        // (an Ability's or a move's words longer than the style's box: pages that turn by themselves)
        static struct AutoPager sPager[2];
        AutoPage(p, sp, &d, 0, &sPager[0]);
        AutoPage(p, sp, &d, 1, &sPager[1]);
    }
    MP_FadeFit_FromPalette(&fit, BG_PLTT_ID(0));
    DrawSpec(p, sp, &d, frame, 0, 0, &fit);
    for (k = 0; k < sp->nOps && list == NULL; k++)
        if (sp->ops[k].kind == OP_LIST)
            list = &sp->ops[k];
    snprintf(name, sizeof(name), "sum.%d.row", v.page);
    rowSp = FindSpec(p, name);
    if (list && rowSp && (v.page == 2 || v.page == 3))
    {
        // the moves: four, and a fifth row for the move to learn, or CANCEL while one is picked
        u8 rows = v.learning || v.choosing ? 5 : 4;
        for (i = 0; i < rows && i < list->n; i++)
        {
            struct UiData rd;
            memset(&rd, 0, sizeof(rd));
            if (i == 4 && !v.learning)
                rd.flags |= R_CANCEL;
            else if (w.move[i] == MOVE_NONE)
                rd.flags |= R_EMPTY;
            else
            {
                rd.str[F_NAME] = w.moveName[i];
                rd.str[F_PP] = w.pp[i], rd.str[F_MAXPP] = w.maxPp[i];
                rd.val[F_TYPE] = w.moveType[i];
                rd.val[F_CTYPE] = w.contestType[i];
                if (i == 4)
                    rd.flags |= R_NEW;
            }
            if (v.choosing && v.moveCursor == i)
                rd.flags |= C_SEL0;
            if (v.swapping && v.moveSwap == i && v.moveSwap != v.moveCursor)
                rd.flags |= B_SWAP;
            DrawSpec(p, rowSp, &rd, frame, list->x, list->y + i * list->h, &fit);
        }
    }
    else if (list && rowSp && v.page == 4)
    {
        // the POKé BALL page's lines, where Kai prints them in its box (a list's h, when set, the sixteenths of
        // Kai's line spacing the style's own box gives them)
        for (i = 0; i < v.ballBox; i++)
        {
            struct UiData rd;
            u8 x, y;
            const u8 *t = MP_Summary_BallLine(i, &x, &y);
            if (t == NULL)
                break;
            memset(&rd, 0, sizeof(rd));
            rd.str[F_NAME] = t;
            // (a list's N over 1: the lines on the style's own rows, that many pixels apart)
            if (list->n > 1)
                y = ((y + list->n / 2) / list->n) * list->n;
            DrawSpec(p, rowSp, &rd, frame, list->x + x, list->y + (list->h ? y * list->h / 16 : y), &fit);
        }
    }
    else if (list && rowSp && v.page == 5)
    {
        for (i = 0; i < v.journalRows && i < list->n; i++)
        {
            struct UiData rd;
            memset(&rd, 0, sizeof(rd));
            rd.str[F_NAME] = v.journal[i];
            if (v.journalEntry[i] & 1)
                rd.flags |= R_NEW;   // (every other sentence's stripe)
            DrawSpec(p, rowSp, &rd, frame, list->x, list->y + i * list->h, &fit);
        }
    }
    // the sprites: the summary's type icons, move frame and markings stay hidden (the style draws its own); the
    // picture, status and POKé BALL where the style puts them; the rest (the POKé BALL page's ball) as Kai draws them
    for (i = 0; i < v.nOwn && nSkip < (int)sizeof(skip) - 4; i++)
        skip[nSkip++] = v.own[i] + 1;
    if (v.markings)
        skip[nSkip++] = (v.markings - gSprites) + 1;
    if (v.ballIcon < MAX_SPRITES)
        skip[nSkip++] = v.ballIcon + 1;
    IconPlaces(sp, d.flags, icon);
    for (wi = 0; wi < 4; wi++)
    {
        static const u8 sWhich[4] = {0, 0xFF, 2, 1};   // ICON a: 0 the picture, 2 its status, 3 its POKé BALL
        u8 id = sWhich[wi] == 0 ? v.monSprite : sWhich[wi] == 2 ? v.statusSprite : sWhich[wi] == 1 ? v.ballSprite : MAX_SPRITES;
        if (wi == 1 || id >= MAX_SPRITES)
            continue;
        skip[nSkip++] = id + 1;
    }
    RedrawSprites(frame, skip, nSkip);
    for (wi = 0; wi < 4; wi++)
    {
        u8 id = wi == 0 ? v.monSprite : wi == 2 ? v.statusSprite : wi == 3 ? v.ballSprite : MAX_SPRITES;
        struct Sprite *s;
        if (id >= MAX_SPRITES || icon[wi][0] < 0)
            continue;
        s = &gSprites[id];
        if (!s->inUse || s->invisible)
            continue;
        RedrawSprite(frame, s, icon[wi][0] - (s->x + s->x2), icon[wi][1] - (s->y + s->y2));
    }
    // (the POKé BALL page's ball where the style's box puts the page's lines)
    if (v.ballIcon < MAX_SPRITES && list && gSprites[v.ballIcon].inUse && !gSprites[v.ballIcon].invisible)
    {
        // (where the page's ICON 4 says, its middle there; else Kai's place from the box's corner)
        struct Sprite *b = &gSprites[v.ballIcon];
        if (icon[4][0] >= 0)
            RedrawSprite(frame, b, icon[4][0] - (b->x + b->x2), icon[4][1] - (b->y + b->y2));
        else
            RedrawSprite(frame, b, list->x - 88, list->y - 32);
    }
}

// ---------------------------------------------------------------------------------------------------- pop-ups
// Kai's own pop-ups (a menu, YES / NO, the save's box, a shop's lists...) in the style: its window frame ("win": a
// 24 x 24 picture of its 3 x 3 frame tiles, its fill) round the window, the window's words as printed, in the
// style's font ("win.text": C_SEL0 the red ones, C_ALT the pale ones); a message box at the foot of the screen in
// the style's message box. A window a screen drew itself is left alone (sHandled).
// a sprite shown over the window's inside (an icon a screen sets on its menu, as the CO-OP menu's): the window then
// keeps Kai's inside, the sprite with it
static bool8 SpriteOver(const struct WindowTemplate *t, const struct Sprite *s);

static u8 sDrawing;   // what this frame draws: 1 the battle, 2 the field's message box, 4 START, 8 the battle's panel,
                      // 16 the party, 32 OPTIONS, 64 the BAG, 128 the summary

static bool8 SpritesOver(const struct WindowTemplate *t)
{
    int i;
    if (sDrawing & 16)
        return FALSE;   // (the party screen: its icons are the style's to draw, at its own places, under the pop-up)
    for (i = 0; i < MAX_SPRITES; i++)
        if (SpriteOver(t, &gSprites[i]))
            return TRUE;
    return FALSE;
}

// ... those sprites drawn again over the window's inside, back to front
static void RedrawSpritesIn(u16 *frame, const struct WindowTemplate *t)
{
    int pr, i;
    for (pr = 3; pr >= 0; pr--)
        for (i = MAX_SPRITES - 1; i >= 0; i--)
            if (gSprites[i].oam.priority == pr && SpriteOver(t, &gSprites[i]))
                RedrawSprite(frame, &gSprites[i], 0, 0);
}

static bool8 SpriteOver(const struct WindowTemplate *t, const struct Sprite *s)
{
    static const u8 sDims[3][4][2] = {
        {{8, 8}, {16, 16}, {32, 32}, {64, 64}},
        {{16, 8}, {32, 8}, {32, 16}, {64, 32}},
        {{8, 16}, {8, 32}, {16, 32}, {32, 64}},
    };
    int x0 = t->tilemapLeft * 8, y0 = t->tilemapTop * 8, x1 = x0 + t->width * 8, y1 = y0 + t->height * 8;
    u32 prio = GetBgAttribute(t->bg, BG_ATTR_PRIORITY);
    int sx, sy, w, h;
    if (!s->inUse || s->invisible || s->mpNoDraw || s->oam.affineMode == ST_OAM_AFFINE_ERASE || s->oam.shape > 2
     || s->oam.objMode == ST_OAM_OBJ_WINDOW || s->oam.priority > prio)
        return FALSE;
    w = sDims[s->oam.shape][s->oam.size][0];
    h = sDims[s->oam.shape][s->oam.size][1];
    sx = s->oam.x >= 256 ? s->oam.x - 512 : s->oam.x;
    sy = s->oam.y >= 160 ? s->oam.y - 256 : s->oam.y;
    return sx < x1 - 2 && sx + w > x0 + 2 && sy < y1 - 2 && sy + h > y0 + 2;
}

static bool8 PopUpShown(u8 w)
{
    const struct WindowTemplate *t = &gWindows[w].window;
    if (w >= WINDOWS_MAX || gWindows[w].tileData == NULL || !sFramed[w] || !sShown[w] || t->bg > 3 || t->width == 0)
        return FALSE;
    return (GetGpuReg(REG_OFFSET_DISPCNT) & (DISPCNT_BG0_ON << t->bg)) != 0;
}

static bool8 AtFoot(u8 w)
{
    const struct WindowTemplate *t = &gWindows[w].window;
    return sFramed[w] == 1 && t->tilemapTop >= 13 && t->width >= 26;
}

// window W's inside as Kai drew it (its pixels, in its palette as faded now) on the style's paper: its words and
// pictures kept where they are, its own paper (colours 0 and 1) the style's
static void KaiInside(u16 *frame, u8 w)
{
    const struct WindowTemplate *t = &gWindows[w].window;
    const u8 *px = gWindows[w].tileData;
    const u16 *pal = &gPlttBufferFaded[BG_PLTT_ID(t->paletteNum & 15)];
    int x0 = t->tilemapLeft * 8, y0 = t->tilemapTop * 8, x, y;
    if (px == NULL)
        return;
    for (y = 0; y < t->height * 8; y++)
        for (x = 0; x < t->width * 8; x++)
        {
            int X = x0 + x, Y = y0 + y;
            u8 v = px[((y / 8) * t->width + x / 8) * 32 + (y % 8) * 4 + (x % 8) / 2];
            v = (x & 1) ? v >> 4 : v & 15;
            if (v > 1 && X >= 0 && Y >= 0 && X < DISPLAY_WIDTH && Y < DISPLAY_HEIGHT)
                frame[Y * DISPLAY_WIDTH + X] = pal[v] & 0x7FFF;
        }
}

static void PopUpFrame(u16 *frame, const struct UiImg *im, int x0, int y0, int x1, int y1, u16 fill,
                       const struct MpFadeFit *fit)
{
    struct Surf fr = {frame, NULL, DISPLAY_WIDTH, DISPLAY_HEIGHT, fit};
    int x, y;
    for (y = y0; y < y1; y++)
        for (x = x0; x < x1; x++)
            Put(&fr, x, y, fill, 255);
    if (im == NULL)
        return;
    // (the frame's tiles: corners, edges along, from the picture's 3 x 3)
    for (y = y0 - 8; y < y1 + 8; y += 8)
        for (x = x0 - 8; x < x1 + 8; x += 8)
        {
            int tx = x < x0 ? 0 : x >= x1 ? 2 : 1, ty = y < y0 ? 0 : y >= y1 ? 2 : 1, i, j;
            if (tx == 1 && ty == 1)
                continue;
            for (j = 0; j < 8; j++)
                for (i = 0; i < 8; i++)
                {
                    int sx = tx * 8 + i - im->dx, sy = ty * 8 + j - im->dy;
                    u16 c;
                    if (sx < 0 || sy < 0 || sx >= im->w || sy >= im->h)
                        continue;
                    c = Rd16(im->px + 2 * (sy * im->w + sx));
                    if (c & 0x8000)
                        Put(&fr, x + i, y + j, c, im->a ? im->a[sy * im->w + sx] : 255);
                }
        }
}

static u8 sOrder[WINDOWS_MAX], sOrderCount;

static void PopUps(struct UiPack *p, u16 *frame)
{
    const struct UiSpec *ws = FindSpec(p, "win"), *wt = FindSpec(p, "win.text");
    const struct UiImg *im = NULL;
    const struct UiFont *f = NULL;
    u16 fill = 0x7FFF;
    struct MpFadeFit fit;
    u8 w, k, oi;
    int drawn[8][4], nDrawn = 0;   // (the pop-ups drawn so far this frame)
    int put[40][3], nPut, j, cellH = 16;   // (the lines drawn in a window: their y, first and last x)
    static u8 sLines[40][64], sFg[40];
    bool8 fits;
    if (ws == NULL || wt == NULL)
        return;
    for (k = 0; k < ws->nOps; k++)
    {
        if (ws->ops[k].kind == OP_IMG && ws->ops[k].a < p->nImg && im == NULL)
            im = &p->img[ws->ops[k].a];
        else if (ws->ops[k].kind == OP_FILL)
            fill = ws->ops[k].ink;
    }
    for (k = 0; k < wt->nOps && f == NULL; k++)
        if (wt->ops[k].kind == OP_TEXT && wt->ops[k].font < p->nFont)
            f = &p->font[wt->ops[k].font];
    if (f)
        cellH = f->cellH;
    MP_FadeFit_FromPalette(&fit, BG_PLTT_ID(15));
    // (the lower windows first: a menu opened over a message at the foot is drawn over it)
    {
        u8 order[WINDOWS_MAX], n = 0, a, b;
        for (w = 0; w < WINDOWS_MAX; w++)
            if (!(sHandled & (1u << w)) && PopUpShown(w))
                order[n++] = w;
        for (a = 1; a < n; a++)
            for (b = a; b > 0; b--)
            {
                const struct WindowTemplate *t1 = &gWindows[order[b - 1]].window, *t2 = &gWindows[order[b]].window;
                if (t1->tilemapTop >= t2->tilemapTop)
                    break;
                w = order[b - 1], order[b - 1] = order[b], order[b] = w;
            }
        for (k = 0; k < n; k++)
            sOrder[k] = order[k];
        sOrderCount = n;
    }
    for (oi = 0; oi < sOrderCount; oi++)
    {
        const struct WindowTemplate *t;
        int x0, y0, ww;
        w = sOrder[oi];
        t = &gWindows[w].window;
        // (faded as the window's own palette is: palette 15 is the field's window colours, not the battle's)
        MP_FadeFit_FromPalette(&fit, BG_PLTT_ID(t->paletteNum & 15));
        if (AtFoot(w))
        {
            MessageBox(p, frame, w);
            continue;
        }
        x0 = t->tilemapLeft * 8, y0 = t->tilemapTop * 8, ww = t->width * 8;
        // (the lines laid out in the style's letters first: where each goes, Kai's place kept, a line set against
        // the right edge kept there, one reaching a run Kai set after it on its line moved past it)
        nPut = 0;
        fits = !sPics[w] && !SpritesOver(t);
        for (k = 0; k < sRunCount[w] && fits; k++)
        {
            struct Run *r = &sRuns[w][k];
            const u8 *line = r->text;
            int ln = 0;
            // (each of its lines: YES / NO is one string, a new line between)
            while (*line != EOS && ln < 8)
            {
                int n = 0, x = x0 + r->x, sw, ly = r->y + ln * r->lineH;
                while (line[n] != EOS && line[n] != CHAR_NEWLINE && n < (int)sizeof(sLines[0]) - 1)
                {
                    sLines[nPut < (int)ARRAY_COUNT(put) ? nPut : 0][n] = line[n];
                    n++;
                }
                if (nPut >= (int)ARRAY_COUNT(put))
                {
                    fits = FALSE;
                    break;
                }
                sLines[nPut][n] = EOS;
                sw = f ? Width(f, sLines[nPut], 255) : r->kaiW;
                if (r->x > 0 && r->x + r->kaiW >= ww - 3 && ln == 0)
                    x = x0 + ww - sw;
                for (j = 0; j < nPut; j++)
                    if (put[j][0] == ly && put[j][1] < x && put[j][2] >= x)
                        x = put[j][2] + 1;
                put[nPut][0] = ly, put[nPut][1] = x, put[nPut][2] = x + sw;
                sFg[nPut] = r->fg;
                nPut++;
                line += n;
                if (*line == CHAR_NEWLINE)
                    line++;
                ln++;
            }
        }
        // (lines the style's letters make wider than Kai's window: the window grows to hold them, toward the
        // screen's middle, a window against the right edge leftward, as the Game Boy games' menus open)
        if (fits && nPut > 0 && sBox[w][0] < 0)
        {
            int right = x0 + ww, need = 0;
            for (j = 0; j < nPut; j++)
                if (put[j][2] + 2 - right > need)
                    need = put[j][2] + 2 - right;
            if (need > 0)
            {
                need = (need + 7) & ~7;
                if (right + need <= DISPLAY_WIDTH - 8 && x0 + ww / 2 < DISPLAY_WIDTH / 2)
                    ww = x0 <= 8 ? DISPLAY_WIDTH - 2 * x0 : ww + need;   // (one from the left edge: across, as theirs)
                else if (x0 - need >= 8)
                {
                    for (j = 0; j < nPut; j++)
                        put[j][1] -= need, put[j][2] -= need;
                    x0 -= need, ww += need;
                }
            }
        }
        // (the style's letters only where every line stays inside the window, clear of the others: else the window
        // keeps Kai's own words and pictures, as Kai laid them out, in the style's frame)
        for (j = 0; j < nPut && fits; j++)
        {
            int i2;
            if (put[j][1] < x0 || put[j][2] > x0 + ww || put[j][0] + cellH - 3 > t->height * 8 + 2)
                fits = FALSE;
            for (i2 = 0; i2 < j && fits; i2++)
                if (put[i2][0] != put[j][0] && abs(put[i2][0] - put[j][0]) < cellH - 3
                 && put[i2][1] < put[j][2] && put[j][1] < put[i2][2])
                    fits = FALSE;
        }
        // (a pop-up the style widened over one drawn before it: this one goes up to sit just above that one)
        if (fits && sBox[w][0] < 0)
        {
            int d;
            for (d = 0; d < nDrawn; d++)
                if (x0 < drawn[d][2] && drawn[d][0] < x0 + ww && y0 < drawn[d][3] && drawn[d][1] < y0 + t->height * 8
                 && y0 < drawn[d][1] && drawn[d][1] - 16 - t->height * 8 >= 8)
                    y0 = drawn[d][1] - 16 - t->height * 8;
        }
        if (sBox[w][0] < 0 && nDrawn < (int)ARRAY_COUNT(drawn))
        {
            drawn[nDrawn][0] = x0, drawn[nDrawn][1] = y0, drawn[nDrawn][2] = x0 + ww;
            drawn[nDrawn][3] = y0 + t->height * 8, nDrawn++;
        }
        if (sBox[w][0] >= 0)
        {
            // (a battle box: the frame round its inside; YES / NO's cursor, a tile left of the window, the style's)
            const u16 *map = GetBgTilemapBuffer(t->bg);
            int cx, cy;
            PopUpFrame(frame, im, sBox[w][0] * 8, sBox[w][1] * 8, (sBox[w][2] + 1) * 8, (sBox[w][3] + 1) * 8,
                       fill, &fit);
            for (cy = sBox[w][1]; map && fits && cy <= sBox[w][3]; cy++)
                for (cx = sBox[w][0]; cx < t->tilemapLeft; cx++)
                    if ((map[cy * 32 + cx] & 0x3FF) == 1)
                    {
                        static const u8 sArrow[] = _("▶");
                        struct UiData d;
                        memset(&d, 0, sizeof(d));
                        d.str[F_NAME] = sArrow;
                        DrawSpec(p, wt, &d, frame, cx * 8, cy * 8 + 1, &fit);
                    }
        }
        else
            PopUpFrame(frame, im, x0, y0, x0 + ww, y0 + t->height * 8, fill, &fit);
        if (!fits)
        {
            // (Kai's own inside, and the sprites Kai shows over it, as the CO-OP menu's icons)
            KaiInside(frame, w);
            RedrawSpritesIn(frame, t);
        }
        if (fits)
            for (j = 0; j < nPut; j++)
            {
                struct UiData d;
                memset(&d, 0, sizeof(d));
                d.str[F_NAME] = sLines[j];
                if (sFg[j] == TEXT_COLOR_RED || sFg[j] == TEXT_COLOR_LIGHT_RED)
                    d.flags |= C_SEL0;
                else if (sFg[j] == TEXT_COLOR_LIGHT_GRAY || sFg[j] == TEXT_COLOR_GREEN
                      || sFg[j] == TEXT_COLOR_LIGHT_GREEN)
                    d.flags |= C_ALT;
                DrawSpec(p, wt, &d, frame, put[j][1], y0 + put[j][0], &fit);
            }
        sHandled |= 1u << w;
    }
}

// ---------------------------------------------------------------------------------------------------- each frame
s16 gMpUiHideBg0[4];
s16 gMpUiHideRects[MP_UI_HIDE_RECTS][4];
u8 gMpUiHideCount;

// the game's own healthbox sprites (its two halves and its bar) drawn or not
// a healthbox's own sprite (its two halves, its bar): never another sprite that took its number while the battle
// rebuilt its sprites (coming back from the BAG), which was then left undrawn
static bool8 IsHealthboxSprite(u8 id)
{
    u16 tag;
    if (id >= MAX_SPRITES || !gSprites[id].inUse || gSprites[id].template == NULL)
        return FALSE;
    tag = gSprites[id].template->tileTag;
    return (tag >= TAG_HEALTHBOX_PLAYER1_TILE && tag <= TAG_HEALTHBAR_OPPONENT2_TILE) || tag == TAG_HEALTHBOX_SAFARI_TILE;
}

static void HideHealthboxes(bool8 hide)
{
    u8 b;
    for (b = 0; b < MAX_BATTLERS_COUNT; b++)
    {
        u8 id = gHealthboxSpriteIds[b], other, bar;
        if (!IsHealthboxSprite(id))
            continue;
        // (the SAFARI ZONE's own box, its balls, stays the game's)
        if ((gBattleTypeFlags & BATTLE_TYPE_SAFARI) && GetBattlerSide(b) == B_SIDE_PLAYER)
            hide = FALSE;
        other = gSprites[id].oam.affineParam;
        bar = gSprites[id].data[5];
        gSprites[id].mpNoDraw = hide;
        if (IsHealthboxSprite(other))
            gSprites[other].mpNoDraw = hide;
        if (IsHealthboxSprite(bar))
            gSprites[bar].mpNoDraw = hide;
    }
}

// Kai's field message box (BG0 rows 112-159) hidden where it shows: the map name's pop-up slides away by scrolling BG0
// (map_name_popup.c), and the message box's tiles go up with it while a message is open
static void HideKaiMessageBox(void)
{
    int v = Bg0VOffset() & 0xFF;
    if (v >= 128)
        v -= 256;
    gMpUiHideBg0[0] = 0, gMpUiHideBg0[2] = DISPLAY_WIDTH;
    gMpUiHideBg0[1] = 112 - v < 0 ? 0 : 112 - v;
    gMpUiHideBg0[3] = DISPLAY_HEIGHT - v > DISPLAY_HEIGHT ? DISPLAY_HEIGHT : DISPLAY_HEIGHT - v;
}

void MP_UiStyle_PreFrame(void)
{
    u8 style = MP_UiStyle_Active();
    static bool8 sHid;
    gMpUiHideBg0[0] = gMpUiHideBg0[2] = 0;
    gMpUiHideCount = 0;
    sDrawing = 0;
    if (style != UISTYLE_KAI && (InField() || InBattle()) && GetPack(style) && FindSpec(GetPack(style), "win"))
    {
        // (Kai's pop-ups on BG0: the style draws its own there, PopUps; their frames not drawn under it)
        // (a pop-up just closed: its window is gone but the screen shows its old tiles a frame or two more, until
        // the cleared map reaches it; its place stays hidden that long, so KAI's never flashes up as the style's goes)
        static s16 sPopRect[WINDOWS_MAX][4];
        static u8 sPopTail[WINDOWS_MAX];
        u8 w, startWin = InField() ? GetStartMenuWindowId() : WINDOW_NONE;
        for (w = 0; w < WINDOWS_MAX && gMpUiHideCount < MP_UI_HIDE_RECTS; w++)
        {
            const struct WindowTemplate *t = &gWindows[w].window;
            s16 *r;
            int side;
            if (w == startWin || !PopUpShown(w) || t->bg != 0)
            {
                if (w != startWin && sPopTail[w] && --sPopTail[w])
                    memcpy(gMpUiHideRects[gMpUiHideCount++], sPopRect[w], sizeof(sPopRect[w]));
                continue;
            }
            side = sFramed[w] == 1 ? 24 : 8;
            r = gMpUiHideRects[gMpUiHideCount++];
            r[0] = t->tilemapLeft * 8 - side, r[1] = t->tilemapTop * 8 - 8;
            r[2] = (t->tilemapLeft + t->width) * 8 + side, r[3] = (t->tilemapTop + t->height) * 8 + 8;
            memcpy(sPopRect[w], r, sizeof(sPopRect[w]));
            sPopTail[w] = 3;
        }
    }
    if (InBattle() && (style != UISTYLE_KAI || sHid))
    {
        HideHealthboxes(style != UISTYLE_KAI);
        sHid = style != UISTYLE_KAI;
    }
    {
        // (the party's POKé BALLS: a style draws the screen without them; Kai's own come back with KAI)
        struct MpPartyState st;
        if (MP_PartyMenu_GetState(&st))
        {
            bool8 hide = style != UISTYLE_KAI && PartyLayoutStyled(st.layout);
            u8 i;
            for (i = 0; i < PARTY_SIZE; i++)
                if (st.ballSprite[i] < MAX_SPRITES)
                    gSprites[st.ballSprite[i]].mpNoDraw = hide;
            if (st.cancelBall < MAX_SPRITES)
                gSprites[st.cancelBall].mpNoDraw = hide;
            if (st.confirmBall < MAX_SPRITES)
                gSprites[st.confirmBall].mpNoDraw = hide;
            if (hide)
                sDrawing |= 16;
        }
    }
    if (style == UISTYLE_KAI)
        return;
    {
        static struct MpOptionsView v;
        static struct MpBagView b;
        if (MP_Options_View(&v))
        {
            sDrawing |= 32;
            return;
        }
        if (MP_Bag_View(&b))
        {
            sDrawing |= 64;
            return;
        }
        {
            static struct MpSummaryView sv;
            if (MP_Summary_View(&sv) && GetPack(style) && FindSpec(GetPack(style), "sum.0"))
            {
                sDrawing |= 128;
                return;
            }
        }
    }
    if (InBattle())
    {
        u16 vofs = Bg0VOffset();
        sDrawing = 1;
        if (vofs == 0 || vofs == DISPLAY_HEIGHT || vofs == DISPLAY_HEIGHT * 2)
        {
            // (the text box's rows: BG0 there is the box and its windows)
            gMpUiHideBg0[0] = 0, gMpUiHideBg0[1] = 112, gMpUiHideBg0[2] = DISPLAY_WIDTH, gMpUiHideBg0[3] = DISPLAY_HEIGHT;
            sDrawing |= 8;
        }
        return;
    }
    if (InField())
    {
        u8 actions[16], count, cursor;
        const u8 *names[16];
        // (START just closed: its window is gone but the screen shows its old tiles a frame or two more, until the
        // cleared map reaches it; KAI's START stays hidden there so it never flashes up as the style's goes)
        static s16 sStartRect[4];
        static u8 sStartTail, sMsgTail;
        if (MP_StartMenu_State(actions, &count, &cursor, names) && count)
        {
            // START's window with its frame (menu.c AddStartMenuWindow: from tile (21, 1), 8 wide)
            u8 h = count > 8 ? count * 2 : count * 2 + 2;
            gMpUiHideBg0[0] = 20 * 8, gMpUiHideBg0[1] = 0, gMpUiHideBg0[2] = DISPLAY_WIDTH, gMpUiHideBg0[3] = (h + 2) * 8;
            memcpy(sStartRect, gMpUiHideBg0, sizeof(sStartRect));
            sStartTail = 3;
            sDrawing |= 4;
        }
        else if (sStartTail && --sStartTail)
            memcpy(gMpUiHideBg0, sStartRect, sizeof(sStartRect));
        else if (FieldMessageShown())
        {
            HideKaiMessageBox();
            sMsgTail = 3;
            sDrawing |= 2;
        }
        else if (sMsgTail && --sMsgTail)   // (the message box just closed: the same, its old tiles kept hidden)
            HideKaiMessageBox();
    }
}

void MP_UiStyle_Composite(u16 *frame)
{
    struct UiPack *p;
    u8 style = MP_UiStyle_Active();
    sHandled = 0;
    if (style == UISTYLE_KAI)
        return;
    // (the field or the battle as last seen in full light: a style whose BAG opens over it, as RED's ITEM box does)
    if ((InField() || InBattle()) && !gPaletteFade.active && gPaletteFade.y == 0)
    {
        memcpy(sSnap, frame, sizeof(sSnap));
        sSnapOk = TRUE;
    }
    if ((p = GetPack(style)) == NULL)
        return;
    if (sDrawing & 1)
    {
        if (InBattle() && MP_OBJ_LAYER_SHOWN())
            BattleHuds(p, frame);
        if ((sDrawing & 8) && InBattle())
            BattlePanel(p, frame);
    }
    if ((sDrawing & 2) && FieldMessageShown())
        FieldMessage(p, frame);
    if (sDrawing & 4)
        StartMenu(p, frame);
    if (sDrawing & 16)
        PartyScreen(p, frame);
    if (sDrawing & 32)
        OptionsScreen(p, frame);
    if (sDrawing & 64)
        BagScreen(p, frame);
    if (sDrawing & 128)
        SummaryScreen(p, frame);
    // (Kai's pop-ups on the field, in battle and over a style's own screens)
    if (InField() || InBattle() || (sDrawing & (16 | 32 | 64 | 128)))
        PopUps(p, frame);
}
