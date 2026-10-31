// Moemon Kai: the RUINS OF ALPH's sliding-panel puzzles (QzStartRuinsOfAlphPuzzle, Johto_RuinsOfAlphChambers).
// A 6x6 board: the picture's 16 pieces (the chamber's POKéMON, its MOéMON picture cut in 4x4) start scattered on the
// outer ring; A picks a piece up and puts it down (or swaps it with the one there), B leaves. When every piece is in
// its place the panel opens: VAR_RESULT 1. VAR_0x8004 = the chamber (0 KABUTO, 1 OMANYTE, 2 AERODACTYL, 3 HO-OH),
// VAR_0x8005 = 1: already solved (the finished picture is shown).
#include "global.h"
#include "bg.h"
#include "data.h"
#include "decompress.h"
#include "event_data.h"
#include "gpu_regs.h"
#include "main.h"
#include "malloc.h"
#include "menu.h"
#include "mp.h"
#include "overworld.h"
#include "palette.h"
#include "pokemon.h"
#include "qz_specials.h"
#include "random.h"
#include "scanline_effect.h"
#include "script.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "text_window.h"
#include "window.h"
#include "constants/rgb.h"
#include "constants/songs.h"
#include "constants/species.h"

#define CELLS     6          // the board is 6x6 cells of 16x16
#define BOARD_X   72
#define BOARD_Y   16
#define PIECES    16
#define TAG_PIECE 0x5A00     // + piece
#define TAG_CURSOR 0x5A20
#define TAG_BACKING 0x5A21
#define NO_PIECE  0xFF

enum { WIN_BOARD, WIN_HELP };

static const struct BgTemplate sBgs[] = {
    {.bg = 0, .charBaseIndex = 2, .mapBaseIndex = 31, .screenSize = 0, .paletteMode = 0, .priority = 1, .baseTile = 0},
};

static const struct WindowTemplate sWindows[] = {
    [WIN_BOARD] = {.bg = 0, .tilemapLeft = BOARD_X / 8, .tilemapTop = BOARD_Y / 8, .width = CELLS * 2, .height = CELLS * 2,
                   .paletteNum = 15, .baseBlock = 1},
    [WIN_HELP] = {.bg = 0, .tilemapLeft = 2, .tilemapTop = 17, .width = 26, .height = 2, .paletteNum = 15,
                  .baseBlock = 1 + CELLS * 2 * CELLS * 2},
    DUMMY_WIN_TEMPLATE,
};

static const struct OamData sOam16 = {.shape = SPRITE_SHAPE(16x16), .size = SPRITE_SIZE(16x16), .priority = 0};

static EWRAM_DATA struct
{
    u8 board[CELLS * CELLS];   // the piece in each cell (NO_PIECE: empty)
    u8 sprite[PIECES];
    u8 backing[PIECES];        // a stone tile under each piece (its see-through parts show it)
    u8 same[PIECES];           // the first piece that looks the same (blank corners can trade places)
    u8 cursorSprite;
    u8 cursor;                 // the cell under the cursor
    u8 held;                   // the piece held (NO_PIECE: none)
    u8 chamber;
    bool8 solvedBefore;
    bool8 leaving;
    u8 doneTimer;
    u8 *pic;
} *sAlph = NULL;

static const u16 sChamberSpecies[4] = {SPECIES_KABUTO, SPECIES_OMANYTE, SPECIES_AERODACTYL, SPECIES_HO_OH};

static u8 Inner(u8 piece)   // the cell a piece belongs in
{
    return (1 + piece / 4) * CELLS + 1 + piece % 4;
}

static bool8 Solved(void)
{
    u8 i;
    for (i = 0; i < PIECES; i++)
        if (sAlph->board[Inner(i)] == NO_PIECE || sAlph->same[sAlph->board[Inner(i)]] != sAlph->same[i])
            return FALSE;
    return TRUE;
}

static void CellXY(u8 cell, s16 *x, s16 *y)
{
    *x = BOARD_X + (cell % CELLS) * 16 + 8;
    *y = BOARD_Y + (cell / CELLS) * 16 + 8;
}

static void PlaceSprites(void)
{
    u8 c, i;
    s16 x, y;
    for (i = 0; i < PIECES; i++)
        gSprites[sAlph->sprite[i]].invisible = gSprites[sAlph->backing[i]].invisible = TRUE;
    for (c = 0; c < CELLS * CELLS; c++)
        if (sAlph->board[c] != NO_PIECE)
        {
            struct Sprite *s = &gSprites[sAlph->sprite[sAlph->board[c]]];
            CellXY(c, &x, &y);
            s->x = x, s->y = y, s->invisible = FALSE;
            s->oam.priority = 1;
            s = &gSprites[sAlph->backing[sAlph->board[c]]];
            s->x = x, s->y = y, s->invisible = FALSE;
            s->oam.priority = 1;
        }
    CellXY(sAlph->cursor, &x, &y);
    gSprites[sAlph->cursorSprite].x = x;
    gSprites[sAlph->cursorSprite].y = y;
    if (sAlph->held != NO_PIECE)
    {
        struct Sprite *s = &gSprites[sAlph->sprite[sAlph->held]];
        s->x = x - 3, s->y = y - 4, s->invisible = FALSE;   // lifted
        s->oam.priority = 0;
        s = &gSprites[sAlph->backing[sAlph->held]];
        s->x = x - 3, s->y = y - 4, s->invisible = FALSE;
        s->oam.priority = 0;
    }
}

static void DrawBoard(void)
{
    u8 c;
    FillWindowPixelBuffer(WIN_BOARD, PIXEL_FILL(0));
    for (c = 0; c < CELLS * CELLS; c++)
    {
        u8 cx = c % CELLS, cy = c / CELLS;
        bool8 inner = cx >= 1 && cx <= 4 && cy >= 1 && cy <= 4;
        FillWindowPixelRect(WIN_BOARD, PIXEL_FILL(inner ? 13 : 14), cx * 16 + 1, cy * 16 + 1, 14, 14);
    }
    CopyWindowToVram(WIN_BOARD, COPYWIN_FULL);
}

static void Help(const u8 *text)
{
    static const u8 sColors[] = {TEXT_COLOR_WHITE, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_LIGHT_GRAY};
    FillWindowPixelBuffer(WIN_HELP, PIXEL_FILL(1));
    AddTextPrinterParameterized3(WIN_HELP, FONT_NORMAL, 0, 1, sColors, TEXT_SKIP_DRAW, text);
    CopyWindowToVram(WIN_HELP, COPYWIN_FULL);
}

static void VBlankCB_Alph(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void CB2_Alph(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    RunTextPrinters();
    UpdatePaletteFade();
    DoScheduledBgTilemapCopiesToVram();
}

static void Leave(u8 taskId, bool8 solved)
{
    gSpecialVar_Result = solved;
    sAlph->leaving = TRUE;
    BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
    gTasks[taskId].data[0] = 1;
    MPNet_Log("alph puzzle %d: %s", sAlph->chamber, solved ? "solved" : "left");
}

static void Task_Alph(u8 taskId)
{
    static const u8 sHelp[] = _("{A_BUTTON} PICK UP / PUT DOWN  {B_BUTTON} LEAVE");
    static const u8 sDone[] = _("The picture is complete!");
    u8 c;
    if (gTasks[taskId].data[0])   // leaving
    {
        if (!gPaletteFade.active)
        {
            Free(sAlph->pic);
            FREE_AND_SET_NULL(sAlph);
            FreeAllWindowBuffers();
            DestroyTask(taskId);
            SetMainCallback2(CB2_ReturnToFieldContinueScript);
        }
        return;
    }
    if (gPaletteFade.active)
        return;
    if (sAlph->doneTimer)
    {
        if (--sAlph->doneTimer == 0)
            Leave(taskId, !sAlph->solvedBefore);
        return;
    }
    if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        if (sAlph->held != NO_PIECE)
        {
            // back where it came from: the first empty cell (it was lifted from one)
            for (c = 0; c < CELLS * CELLS && sAlph->board[c] != NO_PIECE; c++)
                ;
            sAlph->board[c] = sAlph->held;
            sAlph->held = NO_PIECE;
        }
        Leave(taskId, FALSE);
        return;
    }
    if (sAlph->solvedBefore)
        return;
    if (JOY_NEW(DPAD_LEFT) && sAlph->cursor % CELLS)
        sAlph->cursor--, PlaySE(SE_SELECT);
    else if (JOY_NEW(DPAD_RIGHT) && sAlph->cursor % CELLS < CELLS - 1)
        sAlph->cursor++, PlaySE(SE_SELECT);
    else if (JOY_NEW(DPAD_UP) && sAlph->cursor >= CELLS)
        sAlph->cursor -= CELLS, PlaySE(SE_SELECT);
    else if (JOY_NEW(DPAD_DOWN) && sAlph->cursor < CELLS * (CELLS - 1))
        sAlph->cursor += CELLS, PlaySE(SE_SELECT);
    else if (JOY_NEW(A_BUTTON))
    {
        u8 here = sAlph->board[sAlph->cursor];
        if (sAlph->held == NO_PIECE && here != NO_PIECE)
        {
            sAlph->held = here, sAlph->board[sAlph->cursor] = NO_PIECE;
            PlaySE(SE_BALL);
        }
        else if (sAlph->held != NO_PIECE)
        {
            sAlph->board[sAlph->cursor] = sAlph->held;
            sAlph->held = here;   // (a piece there is picked up in its place)
            PlaySE(SE_CLICK);
            if (sAlph->held == NO_PIECE && Solved())
            {
                PlaceSprites();
                gSprites[sAlph->cursorSprite].invisible = TRUE;
                PlaySE(SE_SUCCESS);
                Help(sDone);
                sAlph->doneTimer = 90;
                return;
            }
        }
    }
    (void)sHelp;
    PlaceSprites();
}

static void LoadPieces(u16 species)
{
    u8 i, t;
    struct SpriteTemplate tmpl = {.oam = &sOam16, .anims = gDummySpriteAnimTable, .images = NULL,
                                  .affineAnims = gDummySpriteAffineAnimTable, .callback = SpriteCallbackDummy};
    struct SpritePalette pal;
    sAlph->pic = Alloc(MON_PIC_SIZE * MAX_MON_PIC_FRAMES);
    HandleLoadSpecialPokePic(&gMonFrontPicTable[species], sAlph->pic, species, 0);
    LZDecompressWram(GetMonSpritePalFromSpeciesAndPersonality(species, 0, 0), gDecompressionBuffer);
    pal.data = (const u16 *)gDecompressionBuffer;
    pal.tag = TAG_PIECE;
    LoadSpritePalette(&pal);
    {
        // the stone tile behind a piece: color 3 of the cursor's palette, a darker edge (color 4)
        static u8 sBack[4 * 32];
        struct SpriteSheet sheet = {sBack, sizeof(sBack), TAG_BACKING};
        u8 x, y;
        for (y = 0; y < 16; y++)
            for (x = 0; x < 16; x++)
            {
                u8 v = (x == 0 || y == 0 || x == 15 || y == 15) ? 4 : 3;
                u8 *b = &sBack[((y / 8) * 2 + x / 8) * 32 + (y % 8) * 4 + (x % 8) / 2];
                *b = (x & 1) ? ((*b & 0x0F) | (v << 4)) : ((*b & 0xF0) | v);
            }
        LoadSpriteSheet(&sheet);
    }
    for (i = 0; i < PIECES; i++)
    {
        // a 16x16 piece: the 2x2 tiles at (px, py) of the 8x8-tile picture, in 1D order
        u8 tiles[4 * 32];
        struct SpriteSheet sheet = {tiles, sizeof(tiles), TAG_PIECE + i};
        u8 px = (i % 4) * 2, py = (i / 4) * 2;
        for (t = 0; t < 4; t++)
            memcpy(tiles + t * 32, sAlph->pic + ((py + t / 2) * 8 + px + t % 2) * 32, 32);
        LoadSpriteSheet(&sheet);
        tmpl.tileTag = TAG_PIECE + i;
        tmpl.paletteTag = TAG_PIECE;
        sAlph->sprite[i] = CreateSprite(&tmpl, 0, 0, 1);
        sAlph->same[i] = i;
        for (t = 0; t < i; t++)   // (a piece that looks like an earlier one: the same place is fine for both)
        {
            u8 k, other[4 * 32], tt;
            u8 ox = (t % 4) * 2, oy = (t / 4) * 2;
            for (k = 0; k < 4; k++)
                memcpy(other + k * 32, sAlph->pic + ((oy + k / 2) * 8 + ox + k % 2) * 32, 32);
            for (tt = 0; tt < sizeof(other) && other[tt] == tiles[tt]; tt++)
                ;
            if (tt == sizeof(other))
            {
                sAlph->same[i] = sAlph->same[t];
                break;
            }
        }
    }
    {
        // the cursor: a white frame
        u8 tiles[4 * 32];
        static const u16 sCursorPal[16] = {0, RGB_WHITE, RGB(31, 26, 8), RGB(24, 21, 15), RGB(14, 11, 7)};
        struct SpriteSheet sheet = {tiles, sizeof(tiles), TAG_CURSOR};
        struct SpritePalette cpal = {sCursorPal, TAG_CURSOR};
        u8 x, y;
        memset(tiles, 0, sizeof(tiles));
        for (y = 0; y < 16; y++)
            for (x = 0; x < 16; x++)
                if (x == 0 || y == 0 || x == 15 || y == 15 || x == 1 || y == 1 || x == 14 || y == 14)
                {
                    u8 *b = &tiles[((y / 8) * 2 + x / 8) * 32 + (y % 8) * 4 + (x % 8) / 2];
                    u8 v = (x <= 1 || y <= 1) ? 2 : 1;
                    *b |= (x & 1) ? v << 4 : v;
                }
        LoadSpriteSheet(&sheet);
        LoadSpritePalette(&cpal);
        tmpl.tileTag = TAG_CURSOR;
        tmpl.paletteTag = TAG_CURSOR;
        sAlph->cursorSprite = CreateSprite(&tmpl, 0, 0, 0);
        tmpl.tileTag = TAG_BACKING;
        for (i = 0; i < PIECES; i++)
            sAlph->backing[i] = CreateSprite(&tmpl, 0, 0, 2);   // (under its piece)
    }
}

static void Scramble(void)
{
    static const u8 sRing[] = {0, 1, 2, 3, 4, 5, 6, 11, 12, 17, 18, 23, 24, 29, 30, 31, 32, 33, 34, 35};   // 20 cells
    u8 order[ARRAY_COUNT(sRing)], i, j, t;
    memset(sAlph->board, NO_PIECE, sizeof(sAlph->board));
    if (sAlph->solvedBefore)
    {
        for (i = 0; i < PIECES; i++)
            sAlph->board[Inner(i)] = i;
        return;
    }
    {
        extern char *getenv(const char *name);
        if (getenv("MOEMON_ALPH_EASY"))   // tests: every piece in place but the first, on the corner
        {
            for (i = 1; i < PIECES; i++)
                sAlph->board[Inner(i)] = i;
            sAlph->board[0] = 0;
            return;
        }
    }
    for (i = 0; i < ARRAY_COUNT(sRing); i++)
        order[i] = sRing[i];
    for (i = ARRAY_COUNT(sRing) - 1; i > 0; i--)
    {
        j = Random() % (i + 1);
        t = order[i], order[i] = order[j], order[j] = t;
    }
    for (i = 0; i < PIECES; i++)
        sAlph->board[order[i]] = i;
}

static void CB2_OpenAlph(void)
{
    static const u16 sBackdrop = RGB(17, 13, 8);
    static const u16 sCells[3] = {RGB(23, 20, 14), RGB(9, 7, 5), RGB(26, 24, 18)};   // 13 inner, 14 ring, 15
    static const u8 sHelp[] = _("{A_BUTTON} PICK UP / PUT DOWN  {B_BUTTON} LEAVE");
    static const u8 sSolved[] = _("The picture is complete.  {B_BUTTON} LEAVE");
    SetVBlankCallback(NULL);
    SetGpuReg(REG_OFFSET_DISPCNT, 0);
    ResetBgsAndClearDma3BusyFlags(0);
    InitBgsFromTemplates(0, sBgs, ARRAY_COUNT(sBgs));
    ChangeBgX(0, 0, BG_COORD_SET);
    ChangeBgY(0, 0, BG_COORD_SET);
    DmaFill16(3, 0, VRAM, VRAM_SIZE);
    DmaFill32(3, 0, OAM, OAM_SIZE);
    DmaFill16(3, 0, PLTT, PLTT_SIZE);
    ResetPaletteFade();
    ScanlineEffect_Stop();
    ResetSpriteData();
    FreeAllSpritePalettes();
    ResetTasks();
    InitWindows(sWindows);
    DeactivateAllTextPrinters();
    LoadPalette(&sBackdrop, BG_PLTT_ID(0), 2);
    LoadPalette(gStandardMenuPalette, BG_PLTT_ID(15), PLTT_SIZE_4BPP);
    LoadPalette(sCells, BG_PLTT_ID(15) + 13, sizeof(sCells));
    LoadMessageBoxGfx(WIN_HELP, 0x200, BG_PLTT_ID(14));
    DrawDialogFrameWithCustomTileAndPalette(WIN_HELP, FALSE, 0x200, 14);
    PutWindowTilemap(WIN_BOARD);
    PutWindowTilemap(WIN_HELP);
    DrawBoard();
    LoadPieces(sChamberSpecies[sAlph->chamber]);
    Scramble();
    sAlph->held = NO_PIECE;
    sAlph->cursor = 0;
    if (sAlph->solvedBefore)
        gSprites[sAlph->cursorSprite].invisible = TRUE;
    PlaceSprites();
    Help(sAlph->solvedBefore ? sSolved : sHelp);
    CopyBgTilemapBufferToVram(0);
    SetGpuReg(REG_OFFSET_BLDCNT, 0);
    SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
    ShowBg(0);
    CreateTask(Task_Alph, 80);
    BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
    SetVBlankCallback(VBlankCB_Alph);
    SetMainCallback2(CB2_Alph);
    MPNet_Log("alph puzzle %d: open%s", sAlph->chamber, sAlph->solvedBefore ? " (solved before)" : "");
}

u16 QzStartRuinsOfAlphPuzzle(void)
{
    sAlph = AllocZeroed(sizeof(*sAlph));
    sAlph->chamber = gSpecialVar_0x8004 % 4;
    sAlph->solvedBefore = gSpecialVar_0x8005 == 1;
    gSpecialVar_Result = 0;
    SetMainCallback2(CB2_OpenAlph);
    return 0;
}
