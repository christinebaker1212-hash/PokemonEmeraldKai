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
#include "palette.h"
#include "pokemon.h"
#include "sprite.h"
#include "string_util.h"
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
enum { OP_IMG = 1, OP_TEXT, OP_BAR, OP_FILL, OP_MSG, OP_LONG, OP_BACKING, OP_IMGSEL, OP_ROWMAP, OP_BLEND, OP_ICON };
#define FIELD_CODE 0xFD
#define NFIELDS 96
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
#define P_EMPTY (1u << 27)     // a party slot: nobody in it
#define P_EGG (1u << 28)       // ... an EGG (its name only)
#define P_DESC (1u << 29)      // ... its description in its HP's place (ABLE, NOT ABLE...)
#define P_FAINT (1u << 30)     // ... fainted
#define P_SWITCH (1u << 31)    // ... picked to switch
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
static u8 sFramed[WINDOWS_MAX];   // 1 its dialogue frame drawn, 2 the standard frame, 0 neither

void MP_UiStyle_Framed(u8 windowId, u8 kind)
{
    if (windowId < WINDOWS_MAX)
        sFramed[windowId] = kind;
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
    sCap[windowId].len = 0;
    sCap[windowId].text[0] = EOS;
    sCap[windowId].base = NULL;
}

void MP_UiStyle_WindowGone(u8 windowId)
{
    if (windowId >= WINDOWS_MAX)
        return;
    sFramed[windowId] = 0;
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
    if (first < 0)
        first = 0;
    for (i = first; i < nl && i < first + rows; i++)
    {
        int lx = x;
        if (op->align == 1)
        {
            int wd = 0;
            for (k = sLineStart[i]; k < sLineEnd[i]; k++)
                wd += f->g[sFlow[k].c].adv;
            lx -= wd;
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
        if (sTextOnly && op->kind != OP_TEXT && op->kind != OP_LONG)
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
            regs[(op->flags >> 1) & 7] = Line(s, f, buf, n, x, y, op->ink, op->shadow, op->outline,
                                              op->h ? y + op->h : 0x7FFF) - ox;
            break;
        }
        case OP_MSG:
            Msg(s, p, op, d, x, y, oy, regs);
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
                | (side == B_SIDE_PLAYER ? C_EXP : 0);   // (a style's box shows it where its game does)
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
    return gMain.inBattle && gMain.callback2 == BattleMainCB2;
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
        struct ChooseMoveStruct *mi = (struct ChooseMoveStruct *)(&gBattleBufferA[sChooser][4]);
        u8 cur = gMoveSelectionCursor[sChooser] & 3, i;
        u16 move = mi->moves[cur];
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

static void FieldMessage(struct UiPack *p, u16 *frame)
{
    const struct UiSpec *sp = FindSpec(p, "f.msg");
    struct UiData d;
    struct MpFadeFit fit;
    bool8 waiting;
    if (sp == NULL)
        return;
    memset(&d, 0, sizeof(d));
    d.msg[0] = sCap[0].text;
    d.msgLen[0] = sCap[0].len;
    d.msgPrinted[0] = Printed(0, &waiting);
    d.msgHold[0] = waiting;
    if (waiting)
        d.flags |= C_WAIT;
    MP_FadeFit_FromPalette(&fit, BG_PLTT_ID(15));
    DrawSpec(p, sp, &d, frame, 0, 0, &fit);
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

// where a spec's ICON ops whose conditions hold put Kai's sprites: OUT[a] = (x, y) of the sprite's middle, -1 none
static void IconPlaces(const struct UiSpec *sp, u32 flags, s16 out[4][2])
{
    u16 k;
    for (k = 0; k < 4; k++)
        out[k][0] = out[k][1] = -1;
    for (k = 0; k < sp->nOps; k++)
    {
        const struct UiOp *op = &sp->ops[k];
        if (op->kind != OP_ICON || op->a >= 4 || (flags & op->cond) != op->cond || (flags & op->unless))
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

static bool8 PartyScreenShown(struct MpPartyState *st)
{
    return MP_UiStyle_Active() != UISTYLE_KAI && MP_PartyMenu_GetState(st) && st->layout == PARTY_LAYOUT_SINGLE;
}

static void PartyScreen(struct UiPack *p, u16 *frame)
{
    struct MpPartyState st;
    struct UiData d;
    struct MpFadeFit fit;
    const struct UiSpec *sp;
    u8 skip[32], nSkip = 0, i, w;
    s16 places[PARTY_SIZE][4][2];
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
        RedrawSprite(frame, mon, mx, my);
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
    // Kai's pop-ups (what to do, which move...) as Kai draws them
    for (w = 0; w < WINDOWS_MAX; w++)
    {
        bool8 own = w == 6 || (w == st.msgWin && !popup);
        for (i = 0; i < PARTY_SIZE; i++)
            own |= w == st.window[i];
        if (!own && gWindows[w].tileData != NULL && sFramed[w])
            PasteKaiWindow(frame, w);
    }
}

// ---------------------------------------------------------------------------------------------------- each frame
s16 gMpUiHideBg0[4];
static u8 sDrawing;   // what this frame draws: 1 the battle, 2 the field's message box, 4 START

// the game's own healthbox sprites (its two halves and its bar) drawn or not
static void HideHealthboxes(bool8 hide)
{
    u8 b;
    for (b = 0; b < MAX_BATTLERS_COUNT; b++)
    {
        u8 id = gHealthboxSpriteIds[b], other, bar;
        if (id >= MAX_SPRITES || !gSprites[id].inUse)
            continue;
        // (the SAFARI ZONE's own box, its balls, stays the game's)
        if ((gBattleTypeFlags & BATTLE_TYPE_SAFARI) && GetBattlerSide(b) == B_SIDE_PLAYER)
            hide = FALSE;
        other = gSprites[id].oam.affineParam;
        bar = gSprites[id].data[5];
        gSprites[id].mpNoDraw = hide;
        if (other < MAX_SPRITES && gSprites[other].inUse)
            gSprites[other].mpNoDraw = hide;
        if (bar < MAX_SPRITES && gSprites[bar].inUse)
            gSprites[bar].mpNoDraw = hide;
    }
}

void MP_UiStyle_PreFrame(void)
{
    u8 style = MP_UiStyle_Active();
    static bool8 sHid;
    gMpUiHideBg0[0] = gMpUiHideBg0[2] = 0;
    sDrawing = 0;
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
            bool8 hide = style != UISTYLE_KAI && st.layout == PARTY_LAYOUT_SINGLE;
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
        if (MP_StartMenu_State(actions, &count, &cursor, names) && count)
        {
            // START's window with its frame (menu.c AddStartMenuWindow: from tile (21, 1), 8 wide)
            u8 h = count > 8 ? count * 2 : count * 2 + 2;
            gMpUiHideBg0[0] = 20 * 8, gMpUiHideBg0[1] = 0, gMpUiHideBg0[2] = DISPLAY_WIDTH, gMpUiHideBg0[3] = (h + 2) * 8;
            sDrawing |= 4;
        }
        else if (FieldMessageShown())
        {
            gMpUiHideBg0[0] = 0, gMpUiHideBg0[1] = 112, gMpUiHideBg0[2] = DISPLAY_WIDTH, gMpUiHideBg0[3] = DISPLAY_HEIGHT;
            sDrawing |= 2;
        }
    }
}

void MP_UiStyle_Composite(u16 *frame)
{
    struct UiPack *p;
    u8 style = MP_UiStyle_Active();
    if (style == UISTYLE_KAI || sDrawing == 0 || (p = GetPack(style)) == NULL)
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
}
