// Moemon Kai: the POKéDEX's own side (the user, 2026-11-11).
//
// The co-op POKéDEX is one for the whole group: what a friend sees or catches is added to everyone's (mp_dex.c). This
// keeps what the player saw and caught THEMSELVES beside it, so the POKéDEX can show either: OWN, or SHARED (the
// save's flags, as before). Kept in "<save>.owndex" (the save's layout never changes): a bit a species for seen and
// for caught, the list's view (OWN / SHARED, the SHOW filter) and the entry it was left on.
//
// A save from before starts with what its POKéDEX holds, less the catches a friend is credited with (mp_dex.c
// MP_Dex_CaughtBy): what a friend only showed it can't be told from its own sightings, so those count as seen.
//
// The list screen's header is drawn over the frame (MP_DexHud_Composite, src/platform/sdl2.c): the game's own
// screen has no layer left for it (its list's layer scrolls).
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "main.h"
#include "mp.h"
#include "palette.h"
#include "pokedex.h"
#include "string_util.h"
#include "text.h"
#include "constants/characters.h"
#include "constants/rgb.h"
#include "aw_file.h"
#include "mp_dexplus.h"

extern char gPlatformSavePath[512];
extern bool8 gMpDexApplying;
u8 MP_RenderSmallGlyph(u16 glyphId, u8 fgColor, u8 bgColor, u8 shadowColor);
void MP_AsciiToGame(u8 *dst, const char *src, int maxLen);

#define DEX_BYTES ((NATIONAL_DEX_COUNT + 7) / 8)

static struct
{
    bool8 loaded;
    char path[512];
    u8 seen[DEX_BYTES], caught[DEX_BYTES];
    u8 shared, filter;
    u16 last;        // the entry the list was left on (a national number; 0 none)
} sOwn;

static void Path(char *out, size_t size)
{
    snprintf(out, size, "%s.owndex", gPlatformSavePath);
}

static void Write(void)
{
    char path[600];
    AwFile *f;
    u32 i;
    Path(path, sizeof(path));
    if ((f = AwFile_Open(path)) == NULL)
        return;
    AwFile_Printf(f, "owndex 1\nseen ");
    for (i = 0; i < DEX_BYTES; i++)
        AwFile_Printf(f, "%02x", sOwn.seen[i]);
    AwFile_Printf(f, "\ncaught ");
    for (i = 0; i < DEX_BYTES; i++)
        AwFile_Printf(f, "%02x", sOwn.caught[i]);
    AwFile_Printf(f, "\nview %u %u %u\n", sOwn.shared, sOwn.filter, sOwn.last);
    AwFile_Close(f);
}

static void Hex(const char *p, u8 *out)
{
    u32 i;
    for (i = 0; i < DEX_BYTES; i++)
    {
        unsigned v;
        if (sscanf(p + 2 * i, "%2x", &v) != 1)
            break;
        out[i] = (u8)v;
    }
}

static void Load(void)
{
    char path[600];
    static char line[DEX_BYTES * 2 + 64];
    FILE *f;
    if (sOwn.loaded && strcmp(sOwn.path, gPlatformSavePath) == 0)
        return;
    memset(&sOwn, 0, sizeof(sOwn));
    sOwn.loaded = TRUE;
    sOwn.shared = TRUE;
    snprintf(sOwn.path, sizeof(sOwn.path), "%s", gPlatformSavePath);
    Path(path, sizeof(path));
    if ((f = fopen(path, "r")) == NULL)
    {
        // (a save from before: what its POKéDEX holds, less a friend's catches)
        u16 nat;
        if (gSaveBlock2Ptr == NULL || gSaveBlock1Ptr == NULL)
        {
            sOwn.loaded = FALSE;
            return;
        }
        for (nat = 1; nat <= NATIONAL_DEX_COUNT; nat++)
        {
            if (GetSetPokedexFlag(nat, FLAG_GET_SEEN))
                sOwn.seen[(nat - 1) / 8] |= 1 << ((nat - 1) % 8);
            if (GetSetPokedexFlag(nat, FLAG_GET_CAUGHT) && MP_Dex_CaughtBy(nat) == NULL)
                sOwn.caught[(nat - 1) / 8] |= 1 << ((nat - 1) % 8);
        }
        return;
    }
    while (fgets(line, sizeof(line), f))
    {
        unsigned a, b, c;
        if (strncmp(line, "seen ", 5) == 0)
            Hex(line + 5, sOwn.seen);
        else if (strncmp(line, "caught ", 7) == 0)
            Hex(line + 7, sOwn.caught);
        else if (sscanf(line, "view %u %u %u", &a, &b, &c) == 3)
            sOwn.shared = a != 0, sOwn.filter = b < MP_DEXSHOW_COUNT ? b : 0, sOwn.last = (u16)c;
    }
    fclose(f);
}

// A new game: nothing seen
void MP_DexOwn_Reset(void)
{
    Load();
    if (!sOwn.loaded)
        return;
    memset(sOwn.seen, 0, sizeof(sOwn.seen));
    memset(sOwn.caught, 0, sizeof(sOwn.caught));
    sOwn.last = 0;
    Write();
}

// pokedex.c GetSetPokedexFlag: the game marks a species seen or caught (not while a friend's entries are added)
void MP_DexOwn_Mark(u16 nat, bool8 caught)
{
    u8 mask, *bits;
    if (gMpDexApplying || nat < 1 || nat > NATIONAL_DEX_COUNT)
        return;
    Load();
    if (!sOwn.loaded)
        return;
    mask = 1 << ((nat - 1) % 8);
    bits = caught ? sOwn.caught : sOwn.seen;
    if ((bits[(nat - 1) / 8] & mask) && (sOwn.seen[(nat - 1) / 8] & mask))
        return;
    bits[(nat - 1) / 8] |= mask;
    sOwn.seen[(nat - 1) / 8] |= mask;   // (caught is seen)
    Write();
}

// The player saw (caught) it themselves; never more than the save's own POKéDEX says
bool8 MP_DexOwn_Has(u16 nat, bool8 caught)
{
    if (nat < 1 || nat > NATIONAL_DEX_COUNT)
        return FALSE;
    Load();
    if (!sOwn.loaded)
        return GetSetPokedexFlag(nat, caught ? FLAG_GET_CAUGHT : FLAG_GET_SEEN);
    if (!((caught ? sOwn.caught : sOwn.seen)[(nat - 1) / 8] >> ((nat - 1) % 8) & 1))
        return FALSE;
    return GetSetPokedexFlag(nat, caught ? FLAG_GET_CAUGHT : FLAG_GET_SEEN);
}

bool8 MP_DexView_Shared(void)
{
    Load();
    return sOwn.shared;
}

u8 MP_DexView_Filter(void)
{
    Load();
    return sOwn.filter;
}

u16 MP_DexView_Last(void)
{
    Load();
    return sOwn.last;
}

void MP_DexView_Set(bool8 shared, u8 filter, u16 last)
{
    Load();
    if (sOwn.shared == shared && sOwn.filter == filter && sOwn.last == last)
        return;
    sOwn.shared = shared, sOwn.filter = filter, sOwn.last = last;
    if (sOwn.loaded)
        Write();
}

// ------------------------------------------------------------------------------------------------ the list's header
static struct MpDexHud sHud;
static u32 sHudFrame;
static bool8 sHudOn;

void MP_DexHud_Show(const struct MpDexHud *hud)
{
    sHud = *hud;
    sHudFrame = gMain.vblankCounter1;
    sHudOn = TRUE;
}

static u16 Mix(u16 a, u16 b, s32 t)   // t of 16 toward b
{
    s32 r = (a & 31) + (((b & 31) - (a & 31)) * t) / 16;
    s32 g = ((a >> 5) & 31) + ((((b >> 5) & 31) - ((a >> 5) & 31)) * t) / 16;
    s32 bl = ((a >> 10) & 31) + ((((b >> 10) & 31) - ((a >> 10) & 31)) * t) / 16;
    return (u16)(r | g << 5 | bl << 10 | 0x8000);   // (the frame's pixels: bit 15 set)
}

static void Box(u16 *frame, s32 x0, s32 y0, s32 x1, s32 y1, u16 color, s32 t)
{
    s32 x, y;
    for (y = y0; y <= y1; y++)
        for (x = x0; x <= x1; x++)
        {
            bool8 corner = (x == x0 || x == x1) && (y == y0 || y == y1);
            if (!corner && x >= 0 && y >= 0 && x < DISPLAY_WIDTH && y < DISPLAY_HEIGHT)
                frame[y * DISPLAY_WIDTH + x] = Mix(frame[y * DISPLAY_WIDTH + x], color, t);
        }
}

// ASCII in the game's small font; returns where it ends
static s32 Text(u16 *frame, s32 x, s32 y, const char *ascii, u16 color, u16 shadow)
{
    u8 text[48];
    u32 i;
    MP_AsciiToGame(text, ascii, sizeof(text) - 1);
    for (i = 0; text[i] != EOS; i++)
    {
        s32 w, gx, gy;
        if (text[i] == CHAR_SPACE)
        {
            x += 3;
            continue;
        }
        w = MP_RenderSmallGlyph(text[i], 1, 0, 2);
        for (gy = 0; gy < 16; gy++)
        {
            const u32 *rows = gy < 8 ? gCurGlyph.gfxBufferTop : gCurGlyph.gfxBufferBottom;
            for (gx = 0; gx < w && gx < 16; gx++)
            {
                u8 c = (rows[(gx < 8 ? 0 : 8) + (gy % 8)] >> ((gx % 8) * 4)) & 0xF;
                s32 px = x + gx, py = y + gy - 1;
                if ((c == 1 || c == 2) && px >= 0 && py >= 0 && px < DISPLAY_WIDTH && py < DISPLAY_HEIGHT)
                    frame[py * DISPLAY_WIDTH + px] = c == 1 ? (u16)(color | 0x8000) : Mix(frame[py * DISPLAY_WIDTH + px], shadow, 10);
            }
        }
        x += w;
    }
    return x;
}

static s32 Width(const char *ascii)
{
    u8 text[48];
    u32 i;
    s32 w = 0;
    MP_AsciiToGame(text, ascii, sizeof(text) - 1);
    for (i = 0; text[i] != EOS; i++)
        w += text[i] == CHAR_SPACE ? 3 : MP_RenderSmallGlyph(text[i], 1, 0, 2);
    return w;
}

// A key's name in a small dark tab, then its label
static s32 Key(u16 *frame, s32 x, s32 y, const char *key, const char *label, u16 ink)
{
    s32 w = Width(key);
    Box(frame, x, y, x + w + 3, y + 9, RGB(6, 7, 10), 14);
    Text(frame, x + 2, y, key, RGB(31, 31, 31), 0);
    return Text(frame, x + w + 6, y, label, ink, RGB(31, 31, 31)) + 5;
}

static const char *const sShowNames[MP_DEXSHOW_COUNT] = {"ALL", "NOT SEEN", "NOT CAUGHT", "CAUGHT", "FRIENDS'"};

void MP_DexHud_Composite(u16 *frame)
{
    char buf[40];
    s32 x, y, w, own, all;
    if (!sHudOn || gMain.vblankCounter1 - sHudFrame > 2 || gPaletteFade.active)
        return;
    // the title's strip: which POKéDEX, whose
    Box(frame, 8, 0, 127, 15, RGB(31, 31, 31), 16);
    x = 12;
    if (sHud.nationalOn)
        x = Key(frame, x, 3, "L", sHud.national ? "NATIONAL" : "REGION", RGB(4, 6, 12));
    else
        x = Text(frame, x, 3, "REGION", RGB(4, 6, 12), RGB(31, 31, 31)) + 6;
    Key(frame, x, 3, "R", sHud.shared ? "SHARED" : "OWN", sHud.shared ? RGB(3, 12, 22) : RGB(20, 6, 4));
    // under the picture: what the list shows, how much of it is caught
    Box(frame, 64, 138, 128, 158, RGB(2, 6, 4), 11);
    snprintf(buf, sizeof(buf), "%s", sShowNames[sHud.filter < MP_DEXSHOW_COUNT ? sHud.filter : 0]);
    Key(frame, 66, 139, "LT", buf, RGB(31, 31, 31));
    own = sHud.total ? sHud.ownCaught * 100 / sHud.total : 0;
    all = sHud.total ? sHud.allCaught * 100 / sHud.total : 0;
    snprintf(buf, sizeof(buf), "%d%%", sHud.shared ? all : own);
    Text(frame, 66, 149, buf, RGB(31, 31, 31), 0);
    w = 38;
    y = 152;
    Box(frame, 88, y, 88 + w + 1, y + 4, RGB(0, 0, 0), 12);
    if (sHud.shared)   // (the friends' part behind the player's own)
        for (x = 0; x < w * sHud.allCaught / (sHud.total ? sHud.total : 1); x++)
            frame[(y + 1) * DISPLAY_WIDTH + 89 + x] = frame[(y + 2) * DISPLAY_WIDTH + 89 + x]
                = frame[(y + 3) * DISPLAY_WIDTH + 89 + x] = RGB(9, 20, 31) | 0x8000;
    for (x = 0; x < w * sHud.ownCaught / (sHud.total ? sHud.total : 1); x++)
        frame[(y + 1) * DISPLAY_WIDTH + 89 + x] = frame[(y + 2) * DISPLAY_WIDTH + 89 + x]
            = frame[(y + 3) * DISPLAY_WIDTH + 89 + x] = RGB(31, 26, 6) | 0x8000;
}
