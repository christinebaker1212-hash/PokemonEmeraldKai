// Moemon Kai: the PC boxes' L2 / R2 (the user, 2026-11-05). In the box screen a tap of L2 / R2 goes to the box before /
// after; holding one zooms the box out to a scrolling 3x3 grid of every box, inside the box's own place on the PC
// screen (the rest of the PC stays as it is: the user, "retain the original UI"). Each cell is that box as the PC
// draws it, made small: its wallpaper and its POKéMON's icons; the title plate above shows the box under the PC's
// own hand cursor, in the PC's own title letters and colors. The D-pad moves the hand, A or L2 / R2 again picks the
// box (the PC scrolls to it behind the grid, then the grid zooms back into it), B goes back to the box it came from.
// Drawn over the frame (src/platform/sdl2.c VDraw); while it's up the box itself is hidden so the PC's backdrop
// shows between the cells (pokemon_storage_system.c MpBoxGridHide, which also draws the wallpapers for it).
// pokemon_storage_system.c asks MP_BoxGrid_Input in its box screen's input state.
#include "global.h"
#include "main.h"
#include "sound.h"
#include "text.h"
#include "string_util.h"
#include "pokemon.h"
#include "pokemon_icon.h"
#include "pokemon_storage_system.h"
#include "graphics.h"
#include "constants/songs.h"
#include "constants/characters.h"
#include "constants/rgb.h"
#include "platform/gamepad.h"
#include "mp.h"
#include "mp_region.h"

extern u16 gMpExtraHeld;   // main.c ReadKeys: the keys past the GBA's ten (PLATFORM_KEY_SEARCH = L2, _INSPECT = R2)
u8 MP_RenderSmallGlyph(u16 glyphId, u8 fgColor, u8 bgColor, u8 shadowColor);   // text.c

#define HOLD_FRAMES 15      // a hold this long opens the grid; shorter is a tap
#define ZOOM_FRAMES 12
#define COLS 3
#define ROWS_SHOWN 3
// The box's place on the PC screen: the wallpaper at (80, 16), 160x144; its title plate the top 26 rows, the box
// itself (where the POKéMON are) the 156x116 below
#define WALL_X 80
#define WALL_Y 16
#define WALL_W 160
#define WALL_H 144
#define HEAD_H 26
#define BOX_X 82
#define BOX_Y 42
#define BOX_W 156
#define BOX_H 116
#define CELL_W 48
#define CELL_H 36
#define GAP_X 4
#define GAP_Y 3
#define GRID_X (BOX_X + (BOX_W - (COLS * CELL_W + (COLS - 1) * GAP_X)) / 2)
#define GRID_Y (BOX_Y + 1)

bool8 MP_Storage_BoxWallpaper(u8 boxId, u16 *out);                        // pokemon_storage_system.c
void MP_Storage_BoxTitle(u8 boxId, u8 *pix, s16 *left, u16 *colors);
const u8 *MP_Storage_HandCursor(const u16 **pal);
bool8 MP_Storage_BoxSettled(void);

enum { PHASE_NONE, PHASE_ZOOM_OUT, PHASE_GRID, PHASE_SCROLLING, PHASE_ZOOM_IN };

static struct
{
    u8 phase;
    u8 t;              // frames into a zoom
    u8 holdDir;        // 1 L2, 2 R2 while held
    u8 holdFrames;
    u8 origin, sel, target;
    s16 scroll;        // the first row shown, in pixels (eases toward the cursor's row)
    u16 shot[DISPLAY_WIDTH * DISPLAY_HEIGHT];   // the PC as it was before the grid (the zoom out starts from it)
    u16 back[WALL_W * WALL_H];                   // the PC's backdrop over the box's whole place (under the zoom in)
    u16 cells[TOTAL_BOXES_COUNT][CELL_W * CELL_H];   // each box made small
    u16 head[160 * HEAD_H];                      // the title plate of the box under the hand...
    u8 title[64 * 16];                           // ... its name ...
    s16 titleX;
    u16 titleColors[2];
    u8 headFor;                                  // (the box they're for)
} sG;

static u8 BoxCount(void)
{
    return TOTAL_BOXES_COUNT;
}

static u8 Rows(void)
{
    return (BoxCount() + COLS - 1) / COLS;
}

static s16 RowTop(u8 row)
{
    return row * (CELL_H + GAP_Y);
}

// The first row to show for a box: the boxes come in pages of nine, so every box has its own place of the nine
// (box N in slot N mod 9, the user's rule: each box zooms out to and back from where it sits) and the grid
// scrolls a page at a time (the last page's empty places left empty)
static s16 WantScroll(u8 box)
{
    return RowTop((box / (COLS * ROWS_SHOWN)) * ROWS_SHOWN);
}

static void CellRect(u8 box, s16 scroll, s16 *x, s16 *y)
{
    *x = GRID_X + (box % COLS) * (CELL_W + GAP_X);
    *y = GRID_Y + RowTop(box / COLS) - scroll;
}

bool8 MP_BoxGrid_Active(void)
{
    return sG.phase != PHASE_NONE;
}

void MP_BoxGrid_Reset(void)
{
    sG.phase = PHASE_NONE;
    sG.holdDir = sG.holdFrames = 0;
}


// ---- what the cells show, made in the game's own time (pokemon_storage_system.c draws the wallpapers) ----

// An icon's colors as the PC's own icon has them (pokemon_storage_system.c CreateMonIconSprite): a SEALED Pokémon's
// faded gray, a nicknamed one's Stadium hue (mp_hue.c; a Shadow Pokémon's icon keeps its colors there too)
static void IconColors(u8 box, u8 pos, u16 species, u16 *out)
{
    u8 index = GetMonIconPaletteIndexFromSpecies(species);
    s16 hue;
    if (MP_Gen_SealedSpecies(species))
    {
        MP_GrayIconColors(index, out);
        MPNet_Log("box grid: box %d slot %d sealed", box + 1, pos);
        return;
    }
    memcpy(out, gMonIconPalettes[index], 16 * sizeof(u16));
    hue = MP_Hue_OfBoxMon(GetBoxedMonPtr(box, pos));
    if (hue == MP_HUE_SHADOW || hue == 0)
        return;
    MP_Hue_ShiftPalette(out, 0, hue);
    MPNet_Log("box grid: box %d slot %d hue %d", box + 1, pos, hue);
}

// A box as the PC shows it (its wallpaper and its POKéMON's icons where the PC puts them), BOX_W x BOX_H, then made
// CELL_W x CELL_H: each small pixel the mean of the pixels it covers, the icons' where they cover enough of it
static void BuildCell(u8 box, const u16 *wall)
{
    static u16 sFull[BOX_W * BOX_H];
    static u8 sIcon[BOX_W * BOX_H];
    int i, x, y, u, v;
    for (y = 0; y < BOX_H; y++)
        for (x = 0; x < BOX_W; x++)
        {
            u16 c = wall[(BOX_Y - WALL_Y + y) * 160 + (BOX_X - WALL_X + x)];
            sFull[y * BOX_W + x] = c ? c : (RGB(16, 16, 16) | 0x8000);
            sIcon[y * BOX_W + x] = 0;
        }
    for (i = 0; i < IN_BOX_COUNT; i++)
    {
        u16 sp = GetBoxMonDataAt(box, i, MON_DATA_SPECIES_OR_EGG);
        const u8 *tiles;
        const u16 *pal;
        u16 colors[16];
        // (CreateBoxMonIconAtPos: a 32x32 icon centered at 100 + 24 * column, 44 + 24 * row)
        int x0 = 84 + 24 * (i % IN_BOX_COLUMNS) - BOX_X, y0 = 28 + 24 * (i / IN_BOX_COLUMNS) - BOX_Y;
        if (sp == SPECIES_NONE)
            continue;
        tiles = GetMonIconTiles(sp, FALSE);
        if (tiles == NULL)
            continue;
        IconColors(box, i, sp, colors);
        pal = colors;
        for (y = 0; y < 32; y++)
            for (x = 0; x < 32; x++)
            {
                int t = (y / 8) * 4 + x / 8, k = (y % 8) * 8 + x % 8;
                u8 b = tiles[t * 32 + k / 2], c = (k & 1) ? b >> 4 : b & 15;
                if (c && x0 + x >= 0 && x0 + x < BOX_W && y0 + y >= 0 && y0 + y < BOX_H)
                {
                    sFull[(y0 + y) * BOX_W + x0 + x] = pal[c] | 0x8000;
                    sIcon[(y0 + y) * BOX_W + x0 + x] = 1;
                }
            }
    }
    for (v = 0; v < CELL_H; v++)
        for (u = 0; u < CELL_W; u++)
        {
            int xa = u * BOX_W / CELL_W, xb = (u + 1) * BOX_W / CELL_W, ya = v * BOX_H / CELL_H, yb = (v + 1) * BOX_H / CELL_H;
            int n[2] = {0, 0}, r[2] = {0, 0}, g[2] = {0, 0}, bl[2] = {0, 0}, k;
            for (y = ya; y < yb; y++)
                for (x = xa; x < xb; x++)
                {
                    u16 c = sFull[y * BOX_W + x];
                    k = sIcon[y * BOX_W + x];
                    n[k]++, r[k] += c & 31, g[k] += (c >> 5) & 31, bl[k] += (c >> 10) & 31;
                }
            // (an icon wins where it covers a third of the pixel: they stay crisp)
            k = n[1] * 3 >= n[0] + n[1] ? 1 : 0;
            if (!n[k])
                k = !k;
            sG.cells[box][v * CELL_W + u] = RGB(r[k] / n[k], g[k] / n[k], bl[k] / n[k]) | 0x8000;
        }
}

static void BuildCells(void)
{
    static u16 sWall[160 * 144];
    u8 box;
    for (box = 0; box < BoxCount(); box++)
    {
        if (!MP_Storage_BoxWallpaper(box, sWall))
            memset(sWall, 0, sizeof(sWall));
        BuildCell(box, sWall);
    }
}

// The title plate (the wallpaper's top rows) and the name of the box under the hand
static void Head(u8 box)
{
    static u16 sWall[160 * 144];
    if (sG.headFor == box)
        return;
    sG.headFor = box;
    if (!MP_Storage_BoxWallpaper(box, sWall))
        memset(sWall, 0, sizeof(sWall));
    memcpy(sG.head, sWall, sizeof(sG.head));
    MP_Storage_BoxTitle(box, sG.title, &sG.titleX, sG.titleColors);
}

// The box screen's input (pokemon_storage_system.c, MSTATE_HANDLE_INPUT): MP_BOXGRID_NONE = not ours, BUSY = taken,
// LEFT / RIGHT = a tap (the box before / after), JUMP = scroll to *box
int MP_BoxGrid_Input(u8 currentBox, u8 *box)
{
    u16 held = gMpExtraHeld & (PLATFORM_KEY_SEARCH | PLATFORM_KEY_INSPECT);
    u8 dir = (held & PLATFORM_KEY_SEARCH) ? 1 : (held & PLATFORM_KEY_INSPECT) ? 2 : 0;
    static u16 sPrevHeld;
    u16 newHeld = held & ~sPrevHeld;
    sPrevHeld = held;

    switch (sG.phase)
    {
    case PHASE_NONE:
        if (sG.holdDir)
        {
            if (dir == sG.holdDir)
            {
                if (++sG.holdFrames >= HOLD_FRAMES)
                {
                    // the zoom out, from the box as it is now
                    sG.holdDir = 0;
                    sG.origin = sG.sel = currentBox;
                    sG.scroll = WantScroll(currentBox);
                    BuildCells();
                    sG.headFor = 0xFF;
                    Head(currentBox);
                    sG.t = 0;
                    sG.phase = PHASE_ZOOM_OUT;
                    PlaySE(SE_WIN_OPEN);
                    MPNet_Log("box grid: open on box %d", currentBox + 1);
                }
                return MP_BOXGRID_BUSY;
            }
            // let go before the hold: a tap
            dir = sG.holdDir;
            sG.holdDir = 0;
            return dir == 1 ? MP_BOXGRID_LEFT : MP_BOXGRID_RIGHT;
        }
        if (dir && newHeld)
        {
            sG.holdDir = dir;
            sG.holdFrames = 0;
            return MP_BOXGRID_BUSY;
        }
        return MP_BOXGRID_NONE;
    case PHASE_ZOOM_OUT:
    case PHASE_SCROLLING:
    case PHASE_ZOOM_IN:
        return MP_BOXGRID_BUSY;
    case PHASE_GRID:
    {
        u8 n = BoxCount(), old = sG.sel;
        if (JOY_REPEAT(DPAD_LEFT))
            sG.sel = (sG.sel % COLS) ? sG.sel - 1 : sG.sel;
        if (JOY_REPEAT(DPAD_RIGHT) && sG.sel % COLS != COLS - 1 && sG.sel + 1 < n)
            sG.sel++;
        if (JOY_REPEAT(DPAD_UP))
        {
            if (sG.sel >= COLS)
                sG.sel -= COLS;
            else   // (round to the bottom: the last row, or the one above where the last row is short)
                sG.sel = sG.sel + (Rows() - 1) * COLS < n ? sG.sel + (Rows() - 1) * COLS : sG.sel + (Rows() - 2) * COLS;
        }
        if (JOY_REPEAT(DPAD_DOWN))
            sG.sel = sG.sel + COLS < n ? sG.sel + COLS : sG.sel % COLS;
        if (old != sG.sel)
        {
            PlaySE(SE_SELECT);
            MPNet_Log("box grid: on box %d", sG.sel + 1);
        }
        if (JOY_NEW(B_BUTTON))
        {
            PlaySE(SE_SELECT);
            sG.sel = sG.origin;
        }
        Head(sG.sel);
        if (JOY_NEW(A_BUTTON | B_BUTTON) || newHeld)
        {
            sG.target = sG.sel;
            sG.t = 0;
            MPNet_Log("box grid: picked box %d", sG.target + 1);
            if (!JOY_NEW(B_BUTTON))
                PlaySE(SE_SELECT);
            if (sG.target == currentBox)
            {
                sG.phase = PHASE_ZOOM_IN;
                return MP_BOXGRID_BUSY;
            }
            // the PC scrolls to it behind the grid; the zoom in follows once it's there
            sG.phase = PHASE_SCROLLING;
            *box = sG.target;
            return MP_BOXGRID_JUMP;
        }
        return MP_BOXGRID_BUSY;
    }
    }
    return MP_BOXGRID_NONE;
}

// ---- drawing, over the frame ----

static inline void Put(u16 *f, int x, int y, u16 c)
{
    if (x >= 0 && x < DISPLAY_WIDTH && y >= 0 && y < DISPLAY_HEIGHT)
        f[y * DISPLAY_WIDTH + x] = c | 0x8000;   // (bit 15: opaque in the ABGR1555 frame)
}

// The picture src (sw x sh, its rows `stride` apart) made the size of the rectangle, each pixel the mean of the ones
// it covers (nearest where it grows), clipped to the box's place
static void Scaled(u16 *f, const u16 *src, int sw, int sh, int stride, int rx, int ry, int rw, int rh)
{
    int u, v, x, y;
    if (rw < 1 || rh < 1)
        return;
    for (v = 0; v < rh; v++)
    {
        int ya = v * sh / rh, yb = (v + 1) * sh / rh;
        if (ry + v < BOX_Y || ry + v >= BOX_Y + BOX_H)
            continue;
        if (yb <= ya)
            yb = ya + 1;
        for (u = 0; u < rw; u++)
        {
            int xa = u * sw / rw, xb = (u + 1) * sw / rw, n = 0, r = 0, g = 0, b = 0;
            if (rx + u < BOX_X || rx + u >= BOX_X + BOX_W)
                continue;
            if (xb <= xa)
                xb = xa + 1;
            for (y = ya; y < yb; y++)
                for (x = xa; x < xb; x++)
                {
                    u16 c = src[y * stride + x];
                    n++, r += c & 31, g += (c >> 5) & 31, b += (c >> 10) & 31;
                }
            Put(f, rx + u, ry + v, RGB(r / n, g / n, b / n));
        }
    }
}

// The PC's hand cursor (frame 0 of its sheet: four tiles a row), its top-left corner at x, y
static void Hand(u16 *f, int x0, int y0)
{
    const u16 *pal;
    const u8 *gfx = MP_Storage_HandCursor(&pal);
    int x, y;
    for (y = 0; y < 32; y++)
        for (x = 0; x < 32; x++)
        {
            int t = (y / 8) * 4 + x / 8, k = (y % 8) * 8 + x % 8;
            u8 b = gfx[t * 32 + k / 2], c = (k & 1) ? b >> 4 : b & 15;
            if (c)
                Put(f, x0 + x, y0 + y, pal[c]);
        }
}

// The title plate of the box under the hand, as the PC draws it (over the backdrop, which shows through its clear
// pixels)
static void DrawHead(u16 *f)
{
    int x, y;
    for (y = 0; y < HEAD_H; y++)
        for (x = 0; x < 160; x++)
            if (sG.head[y * 160 + x])
                Put(f, WALL_X + x, WALL_Y + y, sG.head[y * 160 + x]);
    for (y = 0; y < 16; y++)
        for (x = 0; x < 64; x++)
        {
            u8 c = sG.title[y * 64 + x];
            if (c == 14 || c == 15)
                Put(f, sG.titleX + x, 20 + y, sG.titleColors[c == 15]);
        }
}

// Every box's cell in the box's place (the rows scrolled past it cut off at its edges); `skip` isn't drawn
static void DrawCells(u16 *f, int skip)
{
    int box;
    for (box = 0; box < BoxCount(); box++)
    {
        s16 cx, cy;
        int x, y;
        CellRect(box, sG.scroll, &cx, &cy);
        if (box == skip || cy + CELL_H <= BOX_Y || cy >= BOX_Y + BOX_H)
            continue;
        for (y = 0; y < CELL_H; y++)
        {
            if (cy + y < BOX_Y || cy + y >= BOX_Y + BOX_H)
                continue;
            for (x = 0; x < CELL_W; x++)
                Put(f, cx + x, cy + y, sG.cells[box][y * CELL_W + x]);
        }
    }
}

// The hand over the box it's on, bobbing as the PC's does
static void DrawHand(u16 *f)
{
    s16 cx, cy;
    CellRect(sG.sel, sG.scroll, &cx, &cy);
    Hand(f, cx + CELL_W / 2 - 16, cy - 20 + ((gMain.vblankCounter1 / 16) & 1));
}

// The rectangle between the box's whole place (t = 0) and its cell (t = ZOOM_FRAMES), eased
static void ZoomRect(u8 box, int t, int *x, int *y, int *w, int *h)
{
    s16 cx, cy;
    int k = t * t * 256 / (ZOOM_FRAMES * ZOOM_FRAMES);   // (slow start, quick end)
    k = 256 - (256 - k) * (256 - k) / 256;               // ease out on top
    CellRect(box, sG.scroll, &cx, &cy);
    *x = BOX_X + (cx - BOX_X) * k / 256;
    *y = BOX_Y + (cy - BOX_Y) * k / 256;
    *w = BOX_W + (CELL_W - BOX_W) * k / 256;
    *h = BOX_H + (CELL_H - BOX_H) * k / 256;
}

// The backdrop over the box's whole place (its title plate too: the box is hidden while this is kept)
static void SaveBack(const u16 *f)
{
    int y;
    for (y = 0; y < WALL_H; y++)
        memcpy(&sG.back[y * WALL_W], &f[(WALL_Y + y) * DISPLAY_WIDTH + WALL_X], WALL_W * 2);
}

bool8 MP_BoxGrid_HideBox(void)
{
    return sG.phase == PHASE_ZOOM_OUT || sG.phase == PHASE_GRID || sG.phase == PHASE_SCROLLING;
}

void MP_BoxGrid_Composite(u16 *frame)
{
    static u16 sLive[BOX_W * BOX_H];
    int x, y, w, h;
    s16 want;
    if (sG.phase == PHASE_NONE)
    {
        // (while L2 / R2 is held toward the grid: the PC as it is, for the zoom out to start from)
        if (sG.holdDir)
            memcpy(sG.shot, frame, sizeof(sG.shot));
        return;
    }
    // the hand's row eases into view
    want = WantScroll(sG.sel);
    if ((sG.phase == PHASE_GRID || sG.phase == PHASE_SCROLLING) && sG.scroll != want)
        sG.scroll += (want - sG.scroll) / 3 ? (want - sG.scroll) / 3 : (want > sG.scroll ? 1 : -1);
    switch (sG.phase)
    {
    case PHASE_ZOOM_OUT:
        // (the box is hidden: the frame has the PC's backdrop where it was)
        SaveBack(frame);
        DrawHead(frame);
        DrawCells(frame, sG.origin);
        ZoomRect(sG.origin, sG.t, &x, &y, &w, &h);
        Scaled(frame, &sG.shot[BOX_Y * DISPLAY_WIDTH + BOX_X], BOX_W, BOX_H, DISPLAY_WIDTH, x, y, w, h);
        if (++sG.t >= ZOOM_FRAMES)
            sG.phase = PHASE_GRID;
        break;
    case PHASE_GRID:
    case PHASE_SCROLLING:
        SaveBack(frame);
        DrawHead(frame);
        DrawCells(frame, -1);
        DrawHand(frame);
        if (sG.phase == PHASE_SCROLLING && StorageGetCurrentBox() == sG.target && MP_Storage_BoxSettled())
        {
            sG.t = 0;
            sG.phase = PHASE_ZOOM_IN;
        }
        break;
    case PHASE_ZOOM_IN:
        // the box as it is now (on the picked box, shown again), growing from its cell back to its place, over
        // the backdrop kept from the grid (its page in place). The whole place is covered, the title plate too
        // (drawn as the grid has it): the top row's icons reach above the box, and they'd show there at full size
        // before the box had grown back
        sG.scroll = WantScroll(sG.target);
        for (y = 0; y < BOX_H; y++)
            memcpy(&sLive[y * BOX_W], &frame[(BOX_Y + y) * DISPLAY_WIDTH + BOX_X], BOX_W * 2);
        for (y = 0; y < WALL_H; y++)
            memcpy(&frame[(WALL_Y + y) * DISPLAY_WIDTH + WALL_X], &sG.back[y * WALL_W], WALL_W * 2);
        DrawHead(frame);
        DrawCells(frame, sG.target);
        ZoomRect(sG.target, ZOOM_FRAMES - sG.t, &x, &y, &w, &h);
        Scaled(frame, sLive, BOX_W, BOX_H, BOX_W, x, y, w, h);
        if (++sG.t > ZOOM_FRAMES)
        {
            sG.phase = PHASE_NONE;
            MPNet_Log("box grid: closed on box %d", StorageGetCurrentBox() + 1);
        }
        break;
    }
}
