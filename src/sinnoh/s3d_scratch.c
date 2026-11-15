// Moemon Kai: the BATTLE FRONTIER's Scratch-Off Cards, as Platinum has them (pret/pokeplatinum
// src/applications/scratch_off_cards/main.c), on one 240x160 screen as the user approved (2026-11-15, the mock-ups in
// docs/sinnoh/mockups/, tools/sinnoh/mockups_scratch.py). Three cards are picked from Platinum's four, then each is
// scratched: nine cells hide icons of the card's four prize POKéMON (three of one, two of each of the others: the
// first of them three times, every second one after; PlaceRemainingValuesInCells) and two DITTO that stand for any;
// three cells may be scratched, and three alike win that POKéMON's prize: three of its BERRY, one of the four a
// NUGGET (one). Platinum's touch screen is a coin here: the + Control Pad moves it from cell to cell and A held
// scratches the film off where it stands (Platinum's 6 x 6 strokes; a cell shows once 380 of the 500 pixels in its
// middle are bare, CheckIfCellIsRevealed). Platinum's two screens: the prize card (its top screen) as each card
// begins and while R is held; B is Platinum's NEXT CARD / CANCEL button ("Stop scratching this card?").
// Art: sinnoh/platinum_scratch.bin from the user's ROM (tools/sinnoh/build_scratch.py; never committed), the very
// pictures Platinum's screens show, never scaled but for Platinum's own card pull-out; words: Platinum's
// (scratch_off_cards_app). The script: Platinum's (scripts_scratch_off_cards.s through tools/sinnoh/gen_scripts.py:
// OpenScratchOffCardsApp is special Sinnoh_ScratchOffOpen + waitstate, GetScratchOffCardsWonItem
// Sinnoh_ScratchOffWonItem). Drawn as UNOVA's screens are (u3d_screen.c), in SINNOH's own code: a 240x160 picture the
// display shows at twice its size, sharp (sdl2.c / display.c Sinnoh_Scratch_*).
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <SDL2/SDL.h>
#include "global.h"
#include "event_data.h"
#include "item.h"
#include "main.h"
#include "random.h"
#include "script.h"
#include "sound.h"
#include "string_util.h"
#include "strings.h"
#include "constants/characters.h"
#include "constants/items.h"
#include "constants/songs.h"
#include "mp.h"
#include "s3d.h"
#include "../tcg/tcg_gfx.h"

const char *Updater_ExeFile(const char *name);
void MP_GameToAscii(char *dst, const u8 *src, int maxLen);

#define SW 240
#define SH 160
#define CW 480
#define CH 320
#define CELLS 9
#define PICK 4                // (NUM_CARDS_TO_PICK)
#define PLAY 3                // (NUM_CARDS_TO_PLAY)
#define PRIZES 4              // (NUM_WINNABLE_ITEMS)
#define TO_SCRATCH 3          // (NUM_CELLS_TO_SCRATCH)
#define DITTO PRIZES          // (WILDCARD_CELL_VALUE)
#define EMPTY 176

// build_scratch.py's pictures
#define PIC_TOP 0
#define PIC_PRIZE 1
#define PIC_CARD 5
#define PIC_FILM 9
#define PIC_CARDS 10
#define PIC_MONS 14
#define PIC_BOX 19
#define PIC_FLASH 27
#define PIC_POPUP 35
#define PICS 44
#define BOX_CENTRE 7

// where each of Platinum's screens lies on this one (never scaled): the top screen's picture while choosing, the card,
// the prize card
#define CHOOSE_DY (-30)
#define CARD_DX (-8)
#define CARD_DY (-8)
#define PRIZE_DY (-27)
static const s16 sCardX[PICK] = {0, 50, 101, 152};   // (the DS's 56 apart runs past 240: all four whole here)
#define CARD_Y 6
#define PULL_X 54   // (the card grown half again from its corner, as Platinum's affine sprite: 132 wide in the middle)

// Platinum's places (DS pixels)
static const struct { u8 top, bottom, left, right; } sCellRect[CELLS] = {   // (sScratchCellScreenPixelRegions)
    {21, 54, 32, 89}, {21, 54, 102, 157}, {21, 54, 170, 227}, {68, 100, 32, 89}, {68, 100, 102, 157},
    {68, 100, 170, 227}, {114, 148, 32, 89}, {114, 148, 102, 157}, {114, 148, 170, 227}};
static const u8 sCountAt[CELLS][2] = {{42, 27}, {117, 27}, {189, 27}, {42, 74}, {117, 74}, {189, 74},   // (25 x 20)
                                      {42, 120}, {117, 120}, {189, 120}};
static const u8 sMonAt[CELLS][2] = {{57, 18}, {124, 18}, {191, 18}, {57, 66}, {124, 66}, {191, 66}, {57, 114},
                                    {124, 114}, {191, 114}};   // (sScratchMonPositions; drawn 16 to the left)
static const u8 sWinMonX[TO_SCRATCH] = {52, 108, 164};   // (sWinningPopupMonSpritePositions, y 68)
static const u8 sNameY[PRIZES] = {10, 12, 6, 8};          // (sItemNameYOffsets)
static const u8 sNameTop[PRIZES] = {3, 8, 14, 19};       // (the item name windows' tile rows)
static const u16 sItems[] = {   // (sAvailableItems)
    ITEM_POMEG_BERRY, ITEM_KELPSY_BERRY, ITEM_QUALOT_BERRY, ITEM_HONDEW_BERRY, ITEM_GREPA_BERRY, ITEM_TAMATO_BERRY,
    ITEM_OCCA_BERRY, ITEM_PASSHO_BERRY, ITEM_WACAN_BERRY, ITEM_RINDO_BERRY, ITEM_YACHE_BERRY, ITEM_CHOPLE_BERRY,
    ITEM_KEBIA_BERRY, ITEM_SHUCA_BERRY, ITEM_COBA_BERRY, ITEM_PAYAPA_BERRY, ITEM_TANGA_BERRY, ITEM_CHARTI_BERRY,
    ITEM_KASIB_BERRY, ITEM_HABAN_BERRY, ITEM_COLBUR_BERRY, ITEM_BABIRI_BERRY, ITEM_CHILAN_BERRY};
// the winning pop-up's animation (NANR 18: cell, frames), played once
static const u8 sPopup[][2] = {{8, 4}, {0, 3}, {1, 3}, {2, 3}, {3, 3}, {4, 6}, {5, 2}, {6, 2}, {8, 8}, {6, 2}, {7, 2},
                               {4, 25}};
// the card pulled out (CenterSelectedCardOnScreen's steps, per card)
static const s8 sCentreSteps[PICK][8] = {{9, 9, 9, 8, 7, 6, 5, 4}, {2, 2, 2, 1, 1, 1, 1, 1},
                                         {-8, -8, -8, -7, -7, -6, -5, -4}, {-18, -16, -15, -14, -13, -12, -10, -8}};

enum { P_NONE, P_FADE_IN, P_BRING_IN, P_SELECT, P_PULL_OUT, P_PRIZES, P_CARD_IN, P_START, P_SCRATCH, P_WIN, P_TOO_BAD,
       P_ASK, P_SLIDE_AWAY, P_FADE_OUT };

static struct
{
    bool8 active;
    u8 phase, sub;
    u16 timer;
    u32 t;
    u16 prevKeys;
    // the cards picked
    u8 picks[PLAY], picked, sel;
    s16 cardX[PICK], cardY[PICK];
    u8 front;        // (the card pulled out, drawn in front and grown)
    u16 scale;       // (0x1000: as it is)
    u8 step;
    s16 fromX;       // (where it was)
    s16 slide;       // (the card's way in or out, pixels)
    u8 bringing;     // (the card coming in now)
    // the card being scratched
    u8 card;         // (0-2)
    u8 cell[CELLS];
    u16 prize[PRIZES];
    bool8 scratched[CELLS], revealed[CELLS];
    u8 shown[TO_SCRATCH], shownCount;
    bool8 pair;
    u8 winValue;
    u8 flash, flashT;
    u8 coin;         // (the cell the coin stands on)
    s16 coinX, coinY;
    u16 stroke[CELLS];   // (how far each cell's scribble has gone)
    bool8 rubbing;
    u8 rubT;
    u8 film[256 * 192];   // (1: scratched off; Platinum's scratchedPixels and the film's cleared pixels at once)
    bool8 filmDirty;
    bool8 prizeView;
    u8 yes;          // (the YES / NO cursor)
    u8 popFrame, popT;
    bool8 popOn, popDone;
    u8 mosaic;
    bool8 dittoDone;
    u8 fade;         // (0-16 black)
    // what was won (GetScratchOffCardsWonItem)
    u16 won[PLAY], wonCount[PLAY];
    char words[96], button[24];
    // the art
    u16 *data;
    const u16 *pic[PICS];
    u16 picW[PICS], picH[PICS];
    bool8 tried, ok;
    SDL_Renderer *owner;
    SDL_Texture *gba, *canvas, *tex[PICS], *filmTex;
    u16 frame[SW * SH];
    bool8 frameOk;
} S;

// ----------------------------------------------------------------------------------------------------- the art
static void Load(void)
{
    const char *path = Updater_ExeFile("sinnoh/platinum_scratch.bin");
    FILE *f;
    long size;
    u32 o = 3, i;
    S.tried = TRUE;
    if ((f = fopen(path, "rb")) == NULL)
        return;
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);
    S.data = malloc(size + 2);
    if (S.data == NULL || fread(S.data, 1, size, f) != (size_t)size || memcmp(S.data, "SSCR", 4) != 0
     || S.data[2] != PICS)
    {
        fclose(f);
        free(S.data);
        S.data = NULL;
        MPNet_Log("sinnoh scratch: %s isn't the Scratch-Off Cards", path);
        return;
    }
    fclose(f);
    for (i = 0; i < PICS; i++)
    {
        if ((o + 2) * 2 > (u32)size)
            return;
        S.picW[i] = S.data[o];
        S.picH[i] = S.data[o + 1];
        S.pic[i] = S.data + o + 2;
        o += 2 + S.picW[i] * S.picH[i];
        if (o * 2 > (u32)size)
            return;
    }
    S.ok = TRUE;
}

static SDL_Texture *Tex(u16 n)
{
    static u32 px[256 * 192];
    u32 i;
    if (n >= PICS || !S.ok)
        return NULL;
    if (S.tex[n])
        return S.tex[n];
    for (i = 0; i < (u32)S.picW[n] * S.picH[n]; i++)
        px[i] = (S.pic[n][i] & 0x8000) ? 0xFF000000u | G_Rgb15(S.pic[n][i]) : 0;
    S.tex[n] = SDL_CreateTexture(G_R(), SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, S.picW[n], S.picH[n]);
    if (S.tex[n])
    {
        SDL_UpdateTexture(S.tex[n], NULL, px, S.picW[n] * 4);
        SDL_SetTextureBlendMode(S.tex[n], SDL_BLENDMODE_BLEND);
        SDL_SetTextureScaleMode(S.tex[n], SDL_ScaleModeNearest);
    }
    return S.tex[n];
}

static void Pic(u16 n, int x, int y)
{
    SDL_Texture *t = Tex(n);
    SDL_Rect d = {x, y, S.picW[n], S.picH[n]};
    if (t)
        SDL_RenderCopy(G_R(), t, NULL, &d);
}

// (Platinum's card pull-out: the card grown from its corner)
static void PicScaled(u16 n, int x, int y, u16 scale)
{
    SDL_Texture *t = Tex(n);
    SDL_Rect d = {x, y, S.picW[n] * scale / 0x1000, S.picH[n] * scale / 0x1000};
    if (t)
        SDL_RenderCopy(G_R(), t, NULL, &d);
}

// (the DITTO's transformation: G2_SetOBJMosaicSize, blocks of SIZE + 1)
static void PicMosaic(u16 n, int x, int y, u8 size)
{
    int bx, by, b = size + 1;
    if (size == 0)
    {
        Pic(n, x, y);
        return;
    }
    for (by = 0; by < S.picH[n]; by += b)
        for (bx = 0; bx < S.picW[n]; bx += b)
        {
            u16 c = S.pic[n][by * S.picW[n] + bx];
            if (c & 0x8000)
                G_Fill(x + bx, y + by, b < S.picW[n] - bx ? b : S.picW[n] - bx, b < S.picH[n] - by ? b : S.picH[n] - by,
                       G_Rgb15(c));
        }
}

// The film: Platinum's picture with every scratched pixel taken off
static void Film(int x, int y)
{
    static u32 px[256 * 192];
    SDL_Rect d = {x, y, 256, 192};
    u32 i;
    if (!S.ok)
        return;
    if (!S.filmTex)
    {
        S.filmTex = SDL_CreateTexture(G_R(), SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 256, 192);
        if (!S.filmTex)
            return;
        SDL_SetTextureBlendMode(S.filmTex, SDL_BLENDMODE_BLEND);
        SDL_SetTextureScaleMode(S.filmTex, SDL_ScaleModeNearest);
        S.filmDirty = TRUE;
    }
    if (S.filmDirty)
    {
        const u16 *f = S.pic[PIC_FILM];
        for (i = 0; i < 256 * 192; i++)
            px[i] = ((f[i] & 0x8000) && !S.film[i]) ? 0xFF000000u | G_Rgb15(f[i]) : 0;
        SDL_UpdateTexture(S.filmTex, NULL, px, 256 * 4);
        S.filmDirty = FALSE;
    }
    SDL_RenderCopy(G_R(), S.filmTex, NULL, &d);
}

// ----------------------------------------------------------------------------------------------------- the words
static void Words(const char *name, u32 number)
{
    u8 expanded[200];
    const u8 *src;
    int n = 0;
    ConvertIntToDecimalStringN(gStringVar1, number, STR_CONV_MODE_LEFT_ALIGN, 1);
    StringExpandPlaceholders(expanded, S3D_TextByName(name));
    for (src = expanded; *src != EOS && n < (int)sizeof(S.words) - 3; src++)
    {
        u8 c = *src;
        if (c == EXT_CTRL_CODE_BEGIN)
        {
            src += GetExtCtrlCodeLength(src[1]) - 1;
            continue;
        }
        if (c == CHAR_NEWLINE || c == CHAR_PROMPT_SCROLL || c == CHAR_PROMPT_CLEAR || c == PLACEHOLDER_BEGIN)
            break;
        if (c == CHAR_e_ACUTE)
            S.words[n++] = (char)0xC3, S.words[n++] = (char)0xA9;
        else
        {
            char a[2];
            u8 one[2] = {c, EOS};
            MP_GameToAscii(a, one, 1);
            S.words[n++] = a[0];
        }
    }
    S.words[n] = 0;
    MPNet_Log("sinnoh scratch: \"%s\"", S.words);
}

static void ItemWords(char *out, int cap, u16 item)
{
    u8 name[40];
    CopyItemName(item, name);
    MP_GameToAscii(out, name, cap - 1);
}

// ----------------------------------------------------------------------------------------------------- the card
static u32 Rand(u32 n)
{
    return n ? Random() % n : 0;
}

// (PlaceDittosInCells, PlaceRemainingValuesInCells, SelectItemsWinnableOnCard)
static void Deal(void)
{
    int i, j, k, value, tries;
    for (i = 0; i < CELLS; i++)
        S.cell[i] = EMPTY;
    for (i = 0; i < 2; i++)
        for (;;)
        {
            k = Rand(CELLS);
            if (S.cell[k] == EMPTY)
            {
                S.cell[k] = DITTO;
                break;
            }
        }
    value = Rand(PRIZES);
    for (i = 0; i < CELLS; i++)
    {
        for (tries = 0; tries < 30; tries++)
        {
            k = Rand(CELLS);
            if (S.cell[k] == EMPTY)
                break;
        }
        if (tries == 30)
            for (k = 0; k < CELLS && S.cell[k] != EMPTY; k++)
                ;
        if (k >= CELLS)
            break;
        S.cell[k] = value;
        if (i == 2 || i == 4 || i == 6)
            value = (value + 1) % PRIZES;
    }
    k = Rand(PRIZES);
    for (i = 0; i < PRIZES; i++)
    {
        if (i == k)
        {
            S.prize[i] = ITEM_NUGGET;
            continue;
        }
        for (;;)
        {
            u16 item = sItems[Rand(ARRAY_COUNT(sItems))];
            for (j = 0; j < i && S.prize[j] != item; j++)
                ;
            if (j == i)
            {
                S.prize[i] = item;
                break;
            }
        }
    }
    memset(S.scratched, 0, sizeof(S.scratched));
    memset(S.revealed, 0, sizeof(S.revealed));
    memset(S.stroke, 0, sizeof(S.stroke));
    memset(S.film, 0, sizeof(S.film));
    S.filmDirty = TRUE;
    S.shownCount = 0;
    S.pair = FALSE;
    S.flash = S.flashT = 0;
    S.coin = 4;
    S.rubbing = FALSE;
    S.popOn = FALSE;
    S.mosaic = 0;
    MPNet_Log("sinnoh scratch: card %u (%u): %u %u %u / %u %u %u / %u %u %u, prizes %u %u %u %u", S.card + 1,
              S.picks[S.card], S.cell[0], S.cell[1], S.cell[2], S.cell[3], S.cell[4], S.cell[5], S.cell[6], S.cell[7],
              S.cell[8], S.prize[0], S.prize[1], S.prize[2], S.prize[3]);
}

static u8 ScratchedCount(void)
{
    int i, n = 0;
    for (i = 0; i < CELLS; i++)
        n += S.scratched[i];
    return n;
}

// (MarkScratchedPixelsAtPoint / RemoveScratchedOffPixels: 6 x 6 about the point, the film only in the cell's own)
static void Stamp(u8 c, int px, int py)
{
    int x, y;
    for (y = -3; y < 3; y++)
        for (x = -3; x < 3; x++)
        {
            int xx = px + x, yy = py + y;
            if (xx >= sCellRect[c].left && xx <= sCellRect[c].right && yy >= sCellRect[c].top && yy <= sCellRect[c].bottom)
                S.film[yy * 256 + xx] = 1;
        }
    S.filmDirty = TRUE;
}

// (CheckIfCellIsRevealed)
static void CheckRevealed(u8 c)
{
    int x, y, n = 0;
    for (y = sCountAt[c][1]; y < sCountAt[c][1] + 20; y++)
        for (x = sCountAt[c][0]; x < sCountAt[c][0] + 25; x++)
            n += S.film[y * 256 + x];
    if (n >= 380 && !S.revealed[c])
    {
        PlaySE(SE_PIN);
        S.shown[S.shownCount++] = c;
        S.revealed[c] = TRUE;
        MPNet_Log("sinnoh scratch: cell %u shows %u", c, S.cell[c]);
    }
}

// (CheckIfThreeOfAKindFound)
static bool8 ThreeAlike(void)
{
    u8 a, b, c;
    int i;
    bool8 three = FALSE;
    if (S.shownCount <= 1)
        return FALSE;
    a = S.cell[S.shown[0]];
    b = S.cell[S.shown[1]];
    if (a == b || a == DITTO || b == DITTO)
    {
        if (!S.pair)
            PlaySE(SE_SELECT);
        S.pair = TRUE;
    }
    if (S.shownCount <= 2)
        return FALSE;
    c = S.cell[S.shown[2]];
    if ((a == b && a == c) || (a == DITTO && b == DITTO) || (a == DITTO && c == DITTO) || (b == DITTO && c == DITTO)
     || (a == DITTO && b == c) || (b == DITTO && a == c) || (c == DITTO && a == b))
        three = TRUE;
    if (!three)
        return FALSE;
    for (i = 0; i < TO_SCRATCH; i++)
        if (S.cell[S.shown[i]] != DITTO)
        {
            S.winValue = S.cell[S.shown[i]];
            break;
        }
    return TRUE;
}

// The coin at work: A held scribbles across the cell, row by row (Platinum's stylus strokes, two 6 x 6 stamps 3 apart a
// frame, a row every 4 pixels)
static void Rub(void)
{
    u8 c = S.coin;
    int l = sCellRect[c].left + 2, r = sCellRect[c].right - 2, t = sCellRect[c].top + 2, b = sCellRect[c].bottom - 1;
    int width = r - l, rows = (b - t) / 4 + 1, k;
    for (k = 0; k < 2; k++)
    {
        int s = S.stroke[c]++, row = (s / (width / 3 + 1)) % rows, along = (s % (width / 3 + 1)) * 3;
        int x = (row & 1) ? r - along : l + along, y = t + row * 4 + ((s * 7) % 3) - 1;
        S.coinX = x;
        S.coinY = y;
        Stamp(c, x, y);
    }
    CheckRevealed(c);
}

static void CoinHome(void)
{
    S.coinX = (sCellRect[S.coin].left + sCellRect[S.coin].right) / 2 + 9;
    S.coinY = (sCellRect[S.coin].top + sCellRect[S.coin].bottom) / 2 + 5;
}

// ----------------------------------------------------------------------------------------------------- drawing
static void Coin(int x, int y)
{
    G_Circle(x, y, 7.5f, 0x885818, 255);
    G_Circle(x, y, 6.5f, 0xf8c840, 255);
    G_Circle(x, y, 4.5f, 0xc88820, 255);
    G_Circle(x, y, 3.5f, 0xf8c840, 255);
    G_Fill(x - 4, y - 4, 1, 1, 0xfff8c8);
    G_Fill(x - 3, y - 5, 1, 1, 0xfff8c8);
    G_Fill(x - 5, y - 3, 1, 1, 0xfff8c8);
}

static void Message(void)
{
    G_Window(0, 112, 240, 48);
    G_Text(16, 120, G_INK, G_INK_SHADOW, S.words, 216, TF_NORMAL);
}

static void DrawChoose(void)
{
    int i;
    Pic(PIC_TOP, 0 - 8, CHOOSE_DY);
    for (i = PICK - 1; i >= 0; i--)   // (the left card in front: Platinum's priorities 20 + i)
        if (!(S.phase == P_PULL_OUT && i == S.front))
            Pic(PIC_CARDS + i, S.cardX[i], S.cardY[i]);
    if (S.phase == P_PULL_OUT)
        PicScaled(PIC_CARDS + S.front, S.cardX[S.front], S.cardY[S.front], S.scale);
    if (S.phase == P_SELECT)
        Message();
}

static void DrawPrizes(int dx)
{
    int i;
    char name[24];
    Pic(PIC_PRIZE + S.picks[S.card], -8 + dx, PRIZE_DY);
    for (i = 0; i < PRIZES; i++)
    {
        Pic(PIC_MONS + i, 44 - 8 + dx, 26 + i * 42 + PRIZE_DY);
        ItemWords(name, sizeof(name), S.prize[i]);
        G_TextAlign(176 - 8 + dx, sNameTop[i] * 8 + sNameY[i] + PRIZE_DY, TA_CENTER, G_INK, G_INK_SHADOW, name, TF_NORMAL);
    }
}

static void DrawCard(int dx)
{
    int i;
    Pic(PIC_CARD + S.picks[S.card], CARD_DX + dx, CARD_DY);
    for (i = 0; i < CELLS; i++)   // (under the film: priority 2)
        Pic(PIC_MONS + S.cell[i], sMonAt[i][0] - 16 + CARD_DX + dx, sMonAt[i][1] + CARD_DY);
    if (S.pair && S.shownCount >= 2)   // (HighlightMatchingPair: the two in Platinum's flashing boxes)
        for (i = 0; i < 2; i++)
            Pic(PIC_FLASH + S.flash, sMonAt[S.shown[i]][0] - 36 + CARD_DX + dx, sMonAt[S.shown[i]][1] - 10 + CARD_DY);
    Film(CARD_DX + dx, CARD_DY);
    if (dx == 0 && (S.phase == P_SCRATCH || S.phase == P_ASK || S.phase == P_START))
    {
        // the card's foot: R the prize card, B Platinum's NEXT CARD / CANCEL
        char hint[48];
        G_Hint(6, 145, 0xf8f8f8, 0x2848a0, "{R}", TF_SMALL, 1);
        snprintf(hint, sizeof(hint), "{B} %s", S.button);
        G_Hint(234 - G_Hint(0, 0, 0, 0, hint, TF_SMALL, 0), 145, 0xf8f8f8, 0x2848a0, hint, TF_SMALL, 1);
    }
    if (S.phase == P_SCRATCH || S.phase == P_ASK)
        Coin(S.coinX + CARD_DX, S.coinY + CARD_DY);
}

static void Draw(void)
{
    int i;
    switch (S.phase)
    {
    case P_FADE_IN:
    case P_BRING_IN:
    case P_SELECT:
    case P_PULL_OUT:
        DrawChoose();
        break;
    case P_PRIZES:
        DrawPrizes(S.slide);
        break;
    case P_CARD_IN:
        DrawPrizes(S.slide + 256);
        DrawCard(S.slide);
        break;
    case P_SLIDE_AWAY:
        DrawCard(S.slide);
        break;
    default:
        if (S.prizeView)
            DrawPrizes(0);
        else
            DrawCard(0);
        break;
    }
    if (!S.prizeView)
    {
        if (S.phase == P_START || (S.phase == P_TOO_BAD && S.sub == 0))
        {
            Pic(PIC_BOX + BOX_CENTRE, 68 + CARD_DX, 68 + CARD_DY);
            G_TextAlign(120, 69, TA_CENTER, G_INK, G_INK_SHADOW, S.words, TF_NORMAL);
        }
        if (S.popOn)
        {
            Pic(PIC_POPUP + sPopup[S.popFrame][0], 38 + CARD_DX, 56 + CARD_DY);
            for (i = 0; i < TO_SCRATCH; i++)
            {
                u8 v = S.cell[S.shown[i]];
                if (v == DITTO && S.dittoDone)
                    v = S.winValue;
                PicMosaic(PIC_MONS + v, sWinMonX[i] + CARD_DX, 68 + CARD_DY, S.cell[S.shown[i]] == DITTO ? S.mosaic : 0);
            }
        }
    }
    if (S.phase == P_ASK)
    {
        Message();
        G_Window(176, 64, 64, 48);
        G_Text(192, 72, G_INK, G_INK_SHADOW, "YES", 40, TF_NORMAL);
        G_Text(192, 88, G_INK, G_INK_SHADOW, "NO", 40, TF_NORMAL);
        G_Cursor(184, 76 + (S.yes ? 0 : 16));
    }
    if (!S.ok)
    {
        G_Window(0, 112, 240, 48);
        G_Text(16, 120, G_INK, G_INK_SHADOW, "sinnoh/platinum_scratch.bin", 216, TF_NORMAL);
    }
    if (S.fade)
        G_FillA(0, 0, SW, SH, 0x000000, S.fade >= 16 ? 255 : S.fade * 16);
}

// ----------------------------------------------------------------------------------------------------- the steps
static void Phase(u8 p)
{
    S.phase = p;
    S.sub = 0;
    S.timer = 0;
}

static void BringIn(void)
{
    int i;
    for (i = 0; i < PICK; i++)
    {
        S.cardX[i] = sCardX[i] - 256;
        S.cardY[i] = CARD_Y;
    }
    S.bringing = PICK - 1;
    Words("ScratchOffCardsApp_Text_SelectCard", S.picked + 1);
    PlaySE(SE_RG_CARD_FLIPPING);
    Phase(P_BRING_IN);
}

static void StartCard(void)
{
    Deal();
    Words(S.card == PLAY - 1 ? "ScratchOffCardsApp_Text_Cancel" : "ScratchOffCardsApp_Text_NextCard", 0);
    snprintf(S.button, sizeof(S.button), "%s", S.words);
    S.slide = -256;
    S.prizeView = FALSE;
    PlaySE(SE_RG_CARD_FLIPPING);
    Phase(P_PRIZES);
}

static void NextCard(void)
{
    S.card++;
    S.rubbing = FALSE;
    if (S.card >= PLAY)
    {
        Phase(P_FADE_OUT);
        return;
    }
    S.slide = 0;
    PlaySE(SE_RG_CARD_FLIPPING);
    Phase(P_SLIDE_AWAY);
}

static void Close(void)
{
    int i;
    S.active = FALSE;
    S.phase = P_NONE;
    for (i = 0; i < PLAY; i++)
        MPNet_Log("sinnoh scratch: card %d won %u x %u", i + 1, S.won[i], S.wonCount[i]);
    ScriptContext_Enable();
}

static void StepScratch(u16 pressed, u16 keys)
{
    static const s8 sMove[CELLS][4] = {   // (up, down, left, right)
        {6, 3, 2, 1}, {7, 4, 0, 2}, {8, 5, 1, 0}, {0, 6, 5, 4}, {1, 7, 3, 5}, {2, 8, 4, 3}, {3, 0, 8, 7}, {4, 1, 6, 8},
        {5, 2, 7, 6}};
    S.prizeView = (keys & R_BUTTON) != 0;
    if (S.pair && ++S.flashT >= 1)   // (HighlightMatchingPair: a palette a frame)
    {
        S.flashT = 0;
        S.flash = (S.flash + 1) % 8;
    }
    if (S.prizeView)
    {
        S.rubbing = FALSE;
        return;
    }
    if (!(keys & A_BUTTON))
    {
        int d = (pressed & DPAD_UP) ? 0 : (pressed & DPAD_DOWN) ? 1 : (pressed & DPAD_LEFT) ? 2 : (pressed & DPAD_RIGHT) ? 3 : -1;
        S.rubbing = FALSE;
        if (d >= 0)
        {
            S.coin = sMove[S.coin][d];
            CoinHome();
            PlaySE(SE_SELECT);
        }
        if (pressed & B_BUTTON)
        {
            PlaySE(SE_SELECT);
            Words("ScratchOffCardsApp_Text_StopScratching", 0);
            S.yes = FALSE;
            Phase(P_ASK);
        }
        return;
    }
    // A held on a cell: one of the three cells allowed (cellIsScratched), its film off where the coin goes
    if (!S.scratched[S.coin] && ScratchedCount() < TO_SCRATCH)
        S.scratched[S.coin] = TRUE;
    if (!S.scratched[S.coin])
        return;
    if (!S.rubbing || ++S.rubT >= 12)
    {
        S.rubT = 0;
        PlaySE(SE_M_SCRATCH);
    }
    S.rubbing = TRUE;
    if (!S.revealed[S.coin])
        Rub();
    if (ThreeAlike())
    {
        u16 item = S.prize[S.winValue];
        S.won[S.card] = item;
        S.wonCount[S.card] = item == ITEM_NUGGET ? 1 : 3;
        S.rubbing = FALSE;
        S.popOn = TRUE;
        S.popFrame = S.popT = 0;
        S.popDone = FALSE;
        S.dittoDone = FALSE;
        S.mosaic = 0;
        S.timer = (S.cell[S.shown[0]] == DITTO || S.cell[S.shown[1]] == DITTO || S.cell[S.shown[2]] == DITTO) ? 30 : 0;
        PlaySE(SE_SUCCESS);
        MPNet_Log("sinnoh scratch: three alike (%u): %u x %u", S.winValue, item, S.wonCount[S.card]);
        S.phase = P_WIN;
        S.sub = 0;
        return;
    }
    if (S.shownCount >= TO_SCRATCH)
    {
        S.rubbing = FALSE;
        PlaySE(SE_FAILURE);
        Words("ScratchOffCardsApp_Text_TooBad", 0);
        Phase(P_TOO_BAD);
        S.timer = 60;
    }
}

static void Step(u16 pressed, u16 keys)
{
    int i;
    switch (S.phase)
    {
    case P_FADE_IN:
        if (S.fade > 0)
            S.fade -= S.fade >= 3 ? 3 : S.fade;
        else
            BringIn();
        break;
    case P_BRING_IN:   // (State_BringInSelectableCards: the right card first, 32 a frame)
        S.cardX[S.bringing] += 32;
        if (S.cardX[S.bringing] >= sCardX[S.bringing])
        {
            S.cardX[S.bringing] = sCardX[S.bringing];
            if (S.bringing == 0)
            {
                S.sel = 0;
                S.cardY[S.sel] = CARD_Y - 6;
                Phase(P_SELECT);
            }
            else
            {
                S.bringing--;
                PlaySE(SE_RG_CARD_FLIPPING);
            }
        }
        break;
    case P_SELECT:
        if (pressed & (DPAD_LEFT | DPAD_RIGHT))
        {
            S.cardY[S.sel] = CARD_Y;
            S.sel = (S.sel + ((pressed & DPAD_LEFT) ? PICK - 1 : 1)) % PICK;
            S.cardY[S.sel] = CARD_Y - 6;
            PlaySE(SE_SELECT);
        }
        if (pressed & A_BUTTON)
        {
            PlaySE(SE_SELECT);
            S.picks[S.picked++] = S.sel;
            S.front = S.sel;
            S.fromX = S.cardX[S.sel];
            S.scale = 0x1000;
            S.step = 0;
            S.slide = 0;
            MPNet_Log("sinnoh scratch: card no. %u is %u", S.picked, S.sel);
            Phase(P_PULL_OUT);
        }
        break;
    case P_PULL_OUT:   // (State_PullOutSelectedCard: to the middle and half again as big, then all slide away)
        if (S.sub == 0)
        {
            if (S.step < 8)
            {
                // (Platinum's steps carry each card to about the middle; here to where the card half again as big
                // stands in the middle, in the same proportions)
                int k, done = 0, all = 0;
                for (k = 0; k < 8; k++)
                    all += sCentreSteps[S.front][k], done += k <= S.step ? sCentreSteps[S.front][k] : 0;
                S.step++;
                S.cardX[S.front] = S.fromX + (PULL_X - S.fromX) * done / all;
                S.cardY[S.front] -= 4;
            }
            S.scale += 0x100;
            if (S.scale >= 0x1800)
            {
                S.scale = 0x1800;
                S.sub = 1;
                PlaySE(SE_RG_CARD_FLIPPING);
            }
        }
        else
        {
            S.slide += 16;
            for (i = 0; i < PICK; i++)
                S.cardX[i] += 16;
            if (S.slide >= 256)
            {
                S.cardY[S.front] = CARD_Y;
                if (S.picked >= PLAY)
                {
                    S.card = 0;
                    StartCard();
                }
                else
                    BringIn();
            }
        }
        break;
    case P_PRIZES:   // (the prize card, Platinum's top screen, in from the left; A or a moment on brings the card)
        if (S.slide < 0)
            S.slide += 16;
        else if ((pressed & (A_BUTTON | B_BUTTON)) || ++S.timer >= 90)
        {
            S.slide = -256;
            PlaySE(SE_RG_CARD_FLIPPING);
            Phase(P_CARD_IN);
        }
        break;
    case P_CARD_IN:   // (State_SetupScratchCard: the card in from the left, 16 a frame)
        S.slide += 16;
        if (S.slide >= 0)
        {
            S.slide = 0;
            CoinHome();
            Words("ScratchOffCardsApp_Text_StartCard", S.card + 1);
            PlaySE(SE_PIN);
            Phase(P_START);
            S.timer = 30;
        }
        break;
    case P_START:
        if (S.timer)
            S.timer--;
        else
            Phase(P_SCRATCH);
        break;
    case P_SCRATCH:
        StepScratch(pressed, keys);
        break;
    case P_WIN:   // (the pop-up, the DITTO turning into the winner, then on)
        S.prizeView = FALSE;
        if (++S.popT >= sPopup[S.popFrame][1])
        {
            S.popT = 0;
            if (S.popFrame + 1 < ARRAY_COUNT(sPopup))
                S.popFrame++;
            else
                S.popDone = TRUE;   // (the animation done: IsAnimated FALSE)
        }
        if (S.timer)
            S.timer--;
        else if (S.sub == 0)   // (UpdateTransformingDitto: the mosaic up)
        {
            bool8 any = FALSE;
            for (i = 0; i < TO_SCRATCH; i++)
                any |= S.cell[S.shown[i]] == DITTO;
            if (!any)
                S.sub = 2;
            else
            {
                if (S.mosaic == 0)
                    PlaySE(SE_M_TELEPORT);
                if (S.mosaic < 4)
                    S.mosaic++;
                else
                {
                    S.dittoDone = TRUE;
                    S.sub = 1;
                }
            }
        }
        else if (S.sub == 1)   // (... and down)
        {
            if (S.mosaic > 0)
                S.mosaic--;
            else
                S.sub = 2;
        }
        else if (S.popDone)
        {
            S.popOn = FALSE;
            NextCard();
        }
        break;
    case P_TOO_BAD:
        if (S.timer)
            S.timer--;
        else if (S.sub == 0)
        {
            S.sub = 1;   // (the box away, the card still there a moment)
            S.timer = 30;
        }
        else
            NextCard();
        break;
    case P_ASK:   // ("Stop scratching this card?" YES: the next card, nothing won on this one)
        if (pressed & (DPAD_UP | DPAD_DOWN))
        {
            S.yes = !S.yes;
            PlaySE(SE_SELECT);
        }
        if (pressed & A_BUTTON)
        {
            PlaySE(SE_SELECT);
            if (S.yes)
                NextCard();
            else
                Phase(P_SCRATCH);
        }
        else if (pressed & B_BUTTON)
        {
            PlaySE(SE_SELECT);
            Phase(P_SCRATCH);
        }
        break;
    case P_SLIDE_AWAY:   // (State_SlideAwayScratchCard: off to the right, 16 a frame)
        S.slide += 16;
        if (S.slide >= 256)
            StartCard();
        break;
    case P_FADE_OUT:   // (State_FadeOutApp: 10 frames, then the fade)
        if (S.timer < 10)
            S.timer++;
        else if (S.fade < 16)
            S.fade += 3;
        else
            Close();
        break;
    }
}

// ----------------------------------------------------------------------------------------------------- the host
// (special, waitstate: OpenScratchOffCardsApp) three cards picked and scratched
void Sinnoh_ScratchOffOpen(void)
{
    if (!S.tried)
        Load();
    memset(S.won, 0, sizeof(S.won));
    memset(S.wonCount, 0, sizeof(S.wonCount));
    S.picked = S.card = 0;
    S.fade = 16;
    S.prizeView = FALSE;
    S.popOn = FALSE;
    S.prevKeys = 0xFFFF;
    S.t = 0;
    S.frameOk = FALSE;
    S.active = TRUE;
    Phase(P_FADE_IN);
    MPNet_Log("sinnoh scratch: open%s", S.ok ? "" : " (no art: sinnoh/platinum_scratch.bin isn't there)");
}

// (special: GetScratchOffCardsWonItem VAR_0x8004) the item card VAR_0x8004 won in VAR_0x8005, how many in VAR_0x8006
void Sinnoh_ScratchOffWonItem(void)
{
    u16 i = gSpecialVar_0x8004 < PLAY ? gSpecialVar_0x8004 : 0;
    gSpecialVar_0x8005 = S.won[i];
    gSpecialVar_0x8006 = S.wonCount[i];
}

bool8 Sinnoh_Scratch_IsActive(void)
{
    return S.active;
}

bool8 Sinnoh_Scratch_Step(u16 keys)
{
    u16 pressed;
    if (!S.active)
        return FALSE;
    pressed = keys & ~S.prevKeys;
    S.prevKeys = keys;
    S.t++;
    Step(pressed, keys);
    return TRUE;
}

SDL_Texture *Sinnoh_Scratch_Canvas(SDL_Renderer *r)
{
    SDL_Texture *previous;
    SDL_Rect oldViewport, oldClip, all = {0, 0, CW, CH};
    SDL_bool clipped;
    float oldX, oldY;
    if (!S.active)
        return NULL;
    G_Begin(r);
    if (S.owner != r)
    {
        memset(S.tex, 0, sizeof(S.tex));
        S.gba = S.canvas = S.filmTex = NULL;
        S.owner = r;
    }
    if (!S.gba)
    {
        S.gba = SDL_CreateTexture(r, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, SW, SH);
        S.canvas = SDL_CreateTexture(r, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, CW, CH);
        if (!S.gba || !S.canvas)
            return NULL;
        SDL_SetTextureBlendMode(S.gba, SDL_BLENDMODE_NONE);
        SDL_SetTextureScaleMode(S.gba, SDL_ScaleModeNearest);
        SDL_SetTextureBlendMode(S.canvas, SDL_BLENDMODE_NONE);
    }
    previous = SDL_GetRenderTarget(r);
    SDL_RenderGetViewport(r, &oldViewport);
    clipped = SDL_RenderIsClipEnabled(r);
    SDL_RenderGetClipRect(r, &oldClip);
    SDL_RenderGetScale(r, &oldX, &oldY);
    SDL_SetRenderTarget(r, S.gba);
    SDL_RenderSetViewport(r, NULL);
    SDL_RenderSetClipRect(r, NULL);
    SDL_RenderSetScale(r, 1, 1);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
    SDL_RenderClear(r);
    Draw();
    SDL_SetRenderTarget(r, S.gba);
    S.frameOk = SDL_RenderReadPixels(r, NULL, SDL_PIXELFORMAT_ABGR1555, S.frame, SW * 2) == 0;
    SDL_SetRenderTarget(r, S.canvas);
    SDL_RenderCopy(r, S.gba, NULL, &all);
    SDL_SetRenderTarget(r, previous);
    SDL_RenderSetViewport(r, &oldViewport);
    SDL_RenderSetClipRect(r, clipped ? &oldClip : NULL);
    SDL_RenderSetScale(r, oldX, oldY);
    return S.canvas;
}

// the game picture while scratching (spectators, test shots)
const u16 *Sinnoh_Scratch_Frame(void)
{
    static u16 out[SW * SH];
    u32 i;
    if (!S.active || !S.frameOk)
        return NULL;
    for (i = 0; i < SW * SH; i++)
        out[i] = S.frame[i] & 0x7FFF;
    return out;
}

// ----------------------------------------------------------------------------------------------------- tests
// (mp scratch) where the screen is
void Sinnoh_ScratchDebug(void)
{
    int i, inBag = 0, wins = 0;
    for (i = 0; i < PLAY; i++)
        if (S.won[i])
            wins++, inBag += CheckBagHasItem(S.won[i], S.wonCount[i]);
    if (!S.active && wins)
        printf("[test] scratch: %d of %d prizes in the bag\n", inBag, wins);
    printf("[test] scratch: active %d phase %u card %u picks %u %u %u coin %u shown %u pair %d won %u/%u %u/%u %u/%u\n",
           S.active, S.phase, S.card, S.picks[0], S.picks[1], S.picks[2], S.coin, S.shownCount, S.pair, S.won[0],
           S.wonCount[0], S.won[1], S.wonCount[1], S.won[2], S.wonCount[2]);
}

// (mp scratchcell [VALUE]) the coin to an unscratched cell (holding VALUE, or a DITTO, when one is left; any
// otherwise), so A held scratches it
bool8 Sinnoh_ScratchTestCell(int value)
{
    int i, pick = -1;
    if (!S.active || S.phase != P_SCRATCH)
        return FALSE;
    for (i = 0; i < CELLS; i++)
        if (!S.revealed[i] && (value < 0 || S.cell[i] == value || S.cell[i] == DITTO))
        {
            pick = i;
            break;
        }
    for (i = 0; pick < 0 && i < CELLS; i++)
        if (!S.revealed[i])
            pick = i;
    if (pick < 0)
        return FALSE;
    S.coin = pick;
    CoinHome();
    return TRUE;
}

// (mp scratchwin) the value three of which lie on this card (the one to scratch for a win)
int Sinnoh_ScratchTestWinner(void)
{
    int n[PRIZES + 1] = {0}, i;
    for (i = 0; i < CELLS; i++)
        n[S.cell[i] < PRIZES + 1 ? S.cell[i] : 0]++;
    for (i = 0; i < PRIZES; i++)
        if (n[i] >= 3)
            return i;
    return 0;
}
