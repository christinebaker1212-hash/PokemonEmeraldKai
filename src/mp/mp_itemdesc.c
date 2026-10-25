// Moemon Co-op: item descriptions (the Gen 4 games' texts, tools/moemon_items/gen_desc.py) fitted to the window
// that shows them. The texts have no line breaks of their own: they're wrapped here to the window's width, in the
// normal font if they fit its lines, else the narrow one; one that still doesn't fit (the narrow boxes of the shops,
// the PC's item storage, the BATTLE PYRAMID bag) shows a page of lines at a time, turning every couple of seconds.
// The pager only turns a page while the window still shows what it drew last (a message drawn there instead, another
// screen's window, a new item: it stops).
#include "global.h"
#include "main.h"
#include "text.h"
#include "window.h"
#include "string_util.h"
#include "strings.h"
#include "menu.h"
#include "mp.h"

#define DESC_MAX       320
#define DESC_LINES     12
#define LINE_PITCH     16
#define FIRST_PAGE     210   // frames the first page stays up
#define PAGE_FRAMES    150   // ... and each one after it

static struct
{
    bool8 on;
    u8 windowId;
    u8 font;
    u8 x, y;
    u8 linesShown, lineCount, top;
    u8 fill;
    bool8 hasColors;
    u8 colors[3];
    u16 timer;
    u32 sum;
    u8 width, height;   // the window's, in tiles (another window in its place: the pager stops)
    u8 line[DESC_LINES][DESC_MAX / 2];
} sDesc;

static u32 WindowSum(u8 windowId)
{
    const u32 *p = (const u32 *)gWindows[windowId].tileData;
    u32 n = gWindows[windowId].window.width * gWindows[windowId].window.height * TILE_SIZE_4BPP / 4, i, sum = 0;
    if (p == NULL)
        return 0;
    for (i = 0; i < n; i++)
        sum = (sum * 31) ^ p[i];
    return sum;
}

// Wrapped into lines no wider than width in this font (a line break in the text starts a line too). FALSE: more lines
// than DESC_LINES (or a word wider than the window)
static bool8 WrapLines(const u8 *src, u8 font, u16 width, u8 *count)
{
    u8 word[DESC_MAX / 2], cur[DESC_MAX / 2];
    u16 wl = 0, cl = 0;
    u8 n = 0;
    bool8 end = FALSE;
    cur[0] = EOS;
    while (!end)
    {
        u8 c = *src;
        if (c == EOS || c == CHAR_SPACE || c == CHAR_NEWLINE)
        {
            word[wl] = EOS;
            if (wl)
            {
                u8 cand[DESC_MAX / 2];
                u16 len = cl;
                memcpy(cand, cur, cl);
                if (cl)
                    cand[len++] = CHAR_SPACE;
                if (len + wl + 1 > sizeof(cand))
                    return FALSE;
                memcpy(cand + len, word, wl + 1);
                if (GetStringWidth(font, cand, 0) <= width)
                {
                    memcpy(cur, cand, len + wl + 1);
                    cl = len + wl;
                }
                else
                {
                    if (cl == 0 || n >= DESC_LINES)
                        return FALSE;
                    memcpy(sDesc.line[n++], cur, cl + 1);
                    memcpy(cur, word, wl + 1);
                    cl = wl;
                    if (GetStringWidth(font, cur, 0) > width)
                        return FALSE;
                }
            }
            wl = 0;
            if (c == CHAR_NEWLINE || c == EOS)
            {
                if (n >= DESC_LINES)
                    return FALSE;
                memcpy(sDesc.line[n++], cur, cl + 1);
                cl = 0;
                cur[0] = EOS;
            }
            end = (c == EOS);
        }
        else if (wl + 2 < sizeof(word))
        {
            word[wl++] = c;
        }
        src++;
    }
    *count = n;
    return TRUE;
}

static void DrawPage(void)
{
    u8 i;
    FillWindowPixelBuffer(sDesc.windowId, sDesc.fill);
    for (i = 0; i < sDesc.linesShown && sDesc.top + i < sDesc.lineCount; i++)
    {
        const u8 *text = sDesc.line[sDesc.top + i];
        u8 y = sDesc.y + LINE_PITCH * i;
        if (sDesc.hasColors)
            AddTextPrinterParameterized4(sDesc.windowId, sDesc.font, sDesc.x, y, 0, 0, sDesc.colors, TEXT_SKIP_DRAW, text);
        else
            AddTextPrinterParameterized(sDesc.windowId, sDesc.font, text, sDesc.x, y, TEXT_SKIP_DRAW, NULL);
    }
    CopyWindowToVram(sDesc.windowId, COPYWIN_GFX);
    sDesc.sum = WindowSum(sDesc.windowId);
}

// An item's description in this window: from (x, y), width pixels wide, height pixels tall (lines of 16), in these
// text colors (NULL: the window's own), the window filled with fill (PIXEL_FILL(n)) first
void MP_ItemDesc_Print(u8 windowId, const u8 *desc, u8 x, u8 y, u16 width, u16 height, const u8 *colors, u8 fill)
{
    u8 expanded[DESC_MAX];
    u8 lines = 0;
    static const u8 sFonts[] = {FONT_NORMAL, FONT_NARROW};
    u8 i;

    sDesc.on = FALSE;
    if (StringLength(desc) >= DESC_MAX / 2)
        desc = gText_EmptyString2;
    StringExpandPlaceholders(expanded, desc);   // ({POKEBLOCK} and the like, before measuring)
    sDesc.windowId = windowId;
    sDesc.x = x;
    sDesc.y = y;
    sDesc.fill = fill;
    sDesc.hasColors = colors != NULL;
    if (colors)
        memcpy(sDesc.colors, colors, 3);
    sDesc.linesShown = height / LINE_PITCH ? height / LINE_PITCH : 1;
    sDesc.top = 0;
    sDesc.lineCount = 0;
    for (i = 0; i < ARRAY_COUNT(sFonts); i++)
    {
        sDesc.font = sFonts[i];
        if (WrapLines(expanded, sDesc.font, width, &lines) && lines <= sDesc.linesShown)
            break;
    }
    if (i == ARRAY_COUNT(sFonts) && !WrapLines(expanded, FONT_NARROW, width, &lines))
    {
        // (can't happen with the game's texts: shown as it is)
        sDesc.font = FONT_NORMAL;
        StringCopy(sDesc.line[0], desc);
        lines = 1;
    }
    sDesc.lineCount = lines;
    DrawPage();
    if (lines > sDesc.linesShown)
    {
        sDesc.on = TRUE;
        sDesc.timer = FIRST_PAGE;
        sDesc.width = gWindows[windowId].window.width;
        sDesc.height = gWindows[windowId].window.height;
    }
}

// mp_main.c MP_FrameUpdate: the next page of a description too long for its window
void MP_ItemDesc_Update(void)
{
    if (!sDesc.on)
        return;
    // (a screen opening calls this before its main loop takes over: only the window's own contents tell)
    if (sDesc.windowId >= WINDOWS_MAX || gWindows[sDesc.windowId].tileData == NULL
     || gWindows[sDesc.windowId].window.width != sDesc.width || gWindows[sDesc.windowId].window.height != sDesc.height
     || WindowSum(sDesc.windowId) != sDesc.sum)
    {
        sDesc.on = FALSE;   // (the window shows something else now)
        return;
    }
    if (--sDesc.timer)
        return;
    sDesc.top += sDesc.linesShown;
    if (sDesc.top >= sDesc.lineCount)
        sDesc.top = 0;
    DrawPage();
    sDesc.timer = sDesc.top == 0 ? FIRST_PAGE : PAGE_FRAMES;
}

// Tests: what the last description looks like ("[test] itemdesc: font F, N lines, M shown")
void MP_ItemDesc_Debug(void)
{
    printf("[test] itemdesc: font %d, %d lines, %d shown, top %d, paging %d\n", sDesc.font, sDesc.lineCount,
           sDesc.linesShown, sDesc.top, sDesc.on);
}
