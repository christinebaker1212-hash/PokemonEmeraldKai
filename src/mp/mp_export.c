// Moemon Co-op: the phone app's art and tables, exported by the game itself so the app always shows exactly what
// the game does (docs/PHONE_LINK.md "The app's art"). MOEMON_EXPORT_APP=DIR runs it at the first frame and quits:
//   DIR/icons/SPECIES.png   the party icon's two frames (32x64), DIR/front/SPECIES.png the front picture (64x64, as
//                           palette indexes: gray = index x 17; data.json's species "pal" colors it),
//   DIR/items/ITEM.png      the BAG icon (24x24), DIR/font.png the normal font (16 glyphs a row, 16x16 cells),
//   DIR/wallpapers/N.png    a PC box's wallpaper as the PC draws it (160x144: 20x18 tiles, the name's frame on top),
//   DIR/types.png           the type icons (32x16 each, by type, then the contest categories),
//   DIR/frames/N.png        the window frames of OPTIONS > FRAME (24x24: 3x3 tiles, corners, sides and middle),
//   DIR/data.json           species, moves, items, abilities, natures, types (names as the game's text bytes in hex),
//                           the font's widths.
// android/build_assets.sh runs it before building the APK.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "global.h"
#include "mp.h"
#include "battle_main.h"
#include "data.h"
#include "decompress.h"
#include "item.h"
#include "item_icon.h"
#include "malloc.h"
#include "menu.h"
#include "pokemon.h"
#include "pokemon_icon.h"
#include "pokemon_summary_screen.h"
#include "strings.h"
#include "constants/region_map_sections.h"
#include "battle_anim.h"
#include "graphics.h"
#include "region_map.h"
#include "pokedex.h"
#include "text_window.h"
#include "constants/characters.h"
#include "constants/abilities.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/species.h"
#include "constants/trainers.h"
#include "constants/pokemon.h"

#ifdef _WIN32
#include <direct.h>
#define MakeDir(p) _mkdir(p)
#else
#define MakeDir(p) mkdir(p, 0755)
#endif

void MP_ExportGlyph(u16 glyphId, u8 out[16][16], u8 *width);   // text.c
const u32 *MP_SpeciesFrontPal(u16 species);   // pokemon.c
u8 MP_PhoneItemUse(u16 item);                  // party_menu.c
u8 MP_TypeIconPal(u8 type);                    // pokemon_summary_screen.c
void MP_DexTexts(u32 num, const u8 *out[4]);   // pokedex.c
void MP_DexInfoScreen(u16 *out);
void MP_DexListScreen(u16 *out);
void MP_DexFootprint(u16 species, u8 out[16][16]);
const struct PokedexEntry *MP_DexEntry(u32 num);
u32 MP_TownMap_Pages(void);   // region_map.c
const u8 *MP_TownMap_Title(u8 page);
bool8 MP_TownMap_Picture(u8 page, u16 *out, u32 *w, u32 *h);
mapsec_u16_t MP_TownMap_Cell(u8 page, u8 x, u8 y);
u32 MP_Ribbons_Count(void);   // pokenav_ribbons_summary.c
void MP_Ribbons_Icon(u32 id, u16 *out);
const u8 *MP_Ribbons_Desc(u32 id, u32 line);
extern const u8 *const gGiftRibbonDescriptionPointers[][2];
bool8 MP_WallpaperArt(u8 id, const u32 **tiles, const u32 **tilemap, const u16 **palettes);   // pokemon_storage_system.c

// ------------------------------------------------------------------------------------------- a tiny PNG writer

static u32 sCrcTable[256];

static u32 Crc(u32 crc, const u8 *p, size_t n)
{
    size_t i;
    if (sCrcTable[1] == 0)
    {
        u32 k, c;
        for (k = 0; k < 256; k++)
        {
            c = k;
            for (i = 0; i < 8; i++)
                c = c & 1 ? 0xEDB88320 ^ (c >> 1) : c >> 1;
            sCrcTable[k] = c;
        }
    }
    crc = ~crc;
    for (i = 0; i < n; i++)
        crc = sCrcTable[(crc ^ p[i]) & 0xFF] ^ (crc >> 8);
    return ~crc;
}

static void Be32(u8 *p, u32 v)
{
    p[0] = v >> 24, p[1] = v >> 16, p[2] = v >> 8, p[3] = v;
}

static void Chunk(FILE *f, const char *type, const u8 *data, u32 len)
{
    u8 head[8], tail[4];
    u32 crc;
    Be32(head, len);
    memcpy(head + 4, type, 4);
    fwrite(head, 1, 8, f);
    fwrite(data, 1, len, f);
    crc = Crc(0, (const u8 *)type, 4);
    crc = Crc(crc, data, len);
    Be32(tail, crc);
    fwrite(tail, 1, 4, f);
}

// RGBA pixels, w x h, as a PNG (zlib "stored" blocks: no compression; the APK zips them anyway)
static bool8 WritePng(const char *path, const u8 *rgba, u32 w, u32 h)
{
    static const u8 sSig[8] = {0x89, 'P', 'N', 'G', 13, 10, 26, 10};
    u32 raw = (w * 4 + 1) * h, blocks = (raw + 65534) / 65535, zlen = 2 + raw + blocks * 5 + 4, i, a = 1, b = 0;
    u8 ihdr[13], *z, *q;
    FILE *f = fopen(path, "wb");
    if (f == NULL)
        return FALSE;
    z = malloc(zlen + raw);
    if (z == NULL)
    {
        fclose(f);
        return FALSE;
    }
    // the filtered scanlines first (filter 0), then into stored blocks
    {
        u8 *lines = z + zlen, *p = lines;
        for (i = 0; i < h; i++)
        {
            *p++ = 0;
            memcpy(p, rgba + i * w * 4, w * 4);
            p += w * 4;
        }
        q = z;
        *q++ = 0x78, *q++ = 0x01;
        for (i = 0; i < raw; i += 65535)
        {
            u32 n = raw - i < 65535 ? raw - i : 65535;
            *q++ = i + n >= raw;
            *q++ = n & 0xFF, *q++ = n >> 8, *q++ = ~n & 0xFF, *q++ = (~n >> 8) & 0xFF;
            memcpy(q, lines + i, n);
            q += n;
        }
        for (i = 0; i < raw; i++)
        {
            a = (a + lines[i]) % 65521;
            b = (b + a) % 65521;
        }
        Be32(q, (b << 16) | a);
        q += 4;
    }
    Be32(ihdr, w);
    Be32(ihdr + 4, h);
    ihdr[8] = 8, ihdr[9] = 6, ihdr[10] = 0, ihdr[11] = 0, ihdr[12] = 0;
    fwrite(sSig, 1, 8, f);
    Chunk(f, "IHDR", ihdr, 13);
    Chunk(f, "IDAT", z, q - z);
    Chunk(f, "IEND", NULL, 0);
    fclose(f);
    free(z);
    return TRUE;
}

// 4bpp GBA tiles (tilesW x tilesH tiles, in rows) with a 16-color palette (index 0 clear) as RGBA
static void Tiles(const u8 *tiles, u32 tilesW, u32 tilesH, const u16 *pal, u8 *rgba)
{
    u32 tx, ty, y, x, w = tilesW * 8;
    for (ty = 0; ty < tilesH; ty++)
        for (tx = 0; tx < tilesW; tx++)
        {
            const u8 *t = tiles + (ty * tilesW + tx) * 32;
            for (y = 0; y < 8; y++)
                for (x = 0; x < 8; x++)
                {
                    u8 idx = (t[y * 4 + x / 2] >> ((x & 1) * 4)) & 0xF;
                    u16 c = pal[idx];
                    u8 *o = rgba + ((ty * 8 + y) * w + tx * 8 + x) * 4;
                    o[0] = (c & 31) * 255 / 31;
                    o[1] = ((c >> 5) & 31) * 255 / 31;
                    o[2] = ((c >> 10) & 31) * 255 / 31;
                    o[3] = idx ? 255 : 0;
                }
        }
}

// The same, as indexes: gray (index x 17) and clear for index 0
static void TilesIndex(const u8 *tiles, u32 tilesW, u32 tilesH, u8 *rgba)
{
    u32 tx, ty, y, x, w = tilesW * 8;
    for (ty = 0; ty < tilesH; ty++)
        for (tx = 0; tx < tilesW; tx++)
        {
            const u8 *t = tiles + (ty * tilesW + tx) * 32;
            for (y = 0; y < 8; y++)
                for (x = 0; x < 8; x++)
                {
                    u8 idx = (t[y * 4 + x / 2] >> ((x & 1) * 4)) & 0xF;
                    u8 *o = rgba + ((ty * 8 + y) * w + tx * 8 + x) * 4;
                    o[0] = o[1] = o[2] = idx * 17;
                    o[3] = idx ? 255 : 0;
                }
        }
}

static void Json(FILE *f, const u8 *text, size_t max)
{
    size_t i;
    fputc('"', f);
    for (i = 0; i < max && text[i] != EOS; i++)
        fprintf(f, "%02X", text[i]);
    fputc('"', f);
}

// ------------------------------------------------------------------------------------------------------ export

static u32 AllSpecies(void)
{
    return SPECIES_UNOWN_QMARK + 1;   // (the EGG and UNOWN's letters after the 1,050)
}

static void ExportMons(const char *dir)
{
    static u8 sPic[64 * 64 / 2 * 4], sRgba[64 * 64 * 4];
    char path[600];
    u32 s, n = 0;
    snprintf(path, sizeof(path), "%s/icons", dir);
    MakeDir(path);
    snprintf(path, sizeof(path), "%s/front", dir);
    MakeDir(path);
    for (s = 1; s < AllSpecies(); s++)
    {
        // (UNOWN's letters: UNOWN with a personality that makes the letter, as the game draws them)
        u32 letter = s > SPECIES_EGG ? s - SPECIES_UNOWN_B + 1 : 0;
        u32 pid = (letter & 3) | (((letter >> 2) & 3) << 8) | (((letter >> 4) & 3) << 16) | (((letter >> 6) & 3) << 24);
        const u8 *icon = GetMonIconPtr(s > SPECIES_EGG ? SPECIES_UNOWN : s, pid, TRUE);
        u8 pal = GetValidMonIconPalIndex(s > SPECIES_EGG ? SPECIES_UNOWN : s);
        if (icon != NULL)
        {
            Tiles(icon, 4, 8, (const u16 *)gMonIconPaletteTable[pal].data, sRgba);
            snprintf(path, sizeof(path), "%s/icons/%u.png", dir, s);
            WritePng(path, sRgba, 32, 64);
        }
        if (s < NUM_SPECIES || s > SPECIES_EGG)
        {
            u16 colors[16];
            memset(sPic, 0, sizeof(sPic));
            LoadSpecialPokePic(&gMonFrontPicTable[s > SPECIES_EGG ? SPECIES_UNOWN : s], sPic,
                               s > SPECIES_EGG ? SPECIES_UNOWN : s, pid, TRUE);
            // (as palette indexes: the app colors it with the species' palette, or a POKeMON's own: shiny, hue)
            (void)colors;
            TilesIndex(sPic, 8, 8, sRgba);
            snprintf(path, sizeof(path), "%s/front/%u.png", dir, s);
            WritePng(path, sRgba, 64, 64);
        }
        n++;
    }
    MPNet_Log("export: %u POKeMON's icons and pictures", n);
}

static void ExportItems(const char *dir)
{
    static u8 sTiles[0x200], sRgba[24 * 24 * 4];
    u16 colors[16];
    char path[600];
    u32 i;
    snprintf(path, sizeof(path), "%s/items", dir);
    MakeDir(path);
    for (i = 1; i < ITEMS_COUNT; i++)
    {
        const void *pic = GetItemIconPicOrPalette(i, 0), *pal = GetItemIconPicOrPalette(i, 1);
        if (pic == NULL || pal == NULL)
            continue;
        LZDecompressWram(pic, sTiles);
        LZDecompressWram(pal, colors);
        Tiles(sTiles, 3, 3, colors, sRgba);
        snprintf(path, sizeof(path), "%s/items/%u.png", dir, i);
        WritePng(path, sRgba, 24, 24);
    }
}

static void ExportWallpapers(const char *dir)
{
    static u8 sRgba[160 * 144 * 4];
    const u32 *tiles, *tilemap;
    const u16 *pals;
    char path[600];
    u8 id;
    snprintf(path, sizeof(path), "%s/wallpapers", dir);
    MakeDir(path);
    for (id = 0; MP_WallpaperArt(id, &tiles, &tilemap, &pals); id++)
    {
        u32 gfxSize, mapSize, x, y;
        // (the game heap's: Free, never free)
        u8 *gfx = malloc_and_decompress(tiles, &gfxSize);
        u16 *map = malloc_and_decompress(tilemap, &mapSize);
        if (gfx == NULL || map == NULL || mapSize < 20 * 18 * 2)
        {
            if (gfx) Free(gfx);
            if (map) Free(map);
            continue;
        }
        // (pokemon_storage_system.c DrawWallpaper: the map's palettes 1-2 are the wallpaper's two, frame then
        // background; color 0 is clear: the PC's backdrop shows there)
        for (y = 0; y < 144; y++)
            for (x = 0; x < 160; x++)
            {
                u16 e = map[(y / 8) * 20 + x / 8];
                u32 t = e & 0x3FF, px = x & 7, py = y & 7, pal = (e >> 12) & 0xF, idx;
                u16 c;
                u8 *o = sRgba + (y * 160 + x) * 4;
                if (e & 0x400)
                    px = 7 - px;
                if (e & 0x800)
                    py = 7 - py;
                idx = t * 32 + py * 4 + px / 2 < gfxSize ? (gfx[t * 32 + py * 4 + px / 2] >> ((px & 1) * 4)) & 0xF : 0;
                pal = pal >= 1 && pal <= 2 ? pal - 1 : 0;
                c = pals[pal * 16 + idx];
                o[0] = (c & 31) * 255 / 31;
                o[1] = ((c >> 5) & 31) * 255 / 31;
                o[2] = ((c >> 10) & 31) * 255 / 31;
                o[3] = idx ? 255 : 0;   // (the PC's own backdrop shows there)
            }
        snprintf(path, sizeof(path), "%s/wallpapers/%u.png", dir, id);
        WritePng(path, sRgba, 160, 144);
        if (gfx) Free(gfx);
        if (map) Free(map);
    }
}

static void ExportFrames(const char *dir)
{
    static u8 sRgba[24 * 24 * 4];
    char path[600];
    u8 id;
    snprintf(path, sizeof(path), "%s/frames", dir);
    MakeDir(path);
    for (id = 0; id < WINDOW_FRAMES_COUNT; id++)
    {
        const struct TilesPal *f = GetWindowFrameTilesPal(id);
        Tiles((const u8 *)f->tiles, 3, 3, f->pal, sRgba);
        snprintf(path, sizeof(path), "%s/frames/%u.png", dir, id);
        WritePng(path, sRgba, 24, 24);
    }
}

// The type icons as the summary draws them (32x16 each, in type order, then the contest categories)
static void ExportTypeIcons(const char *dir)
{
    static u8 sTiles[(NUMBER_OF_MON_TYPES + CONTEST_CATEGORIES_COUNT) * 0x100], sRgba[32 * 16 * 4];
    static u8 sSheet[32 * 16 * (NUMBER_OF_MON_TYPES + CONTEST_CATEGORIES_COUNT) * 4];
    u16 pals[16 * 3];
    char path[600];
    u32 t, y;
    LZDecompressWram(gMoveTypes_Gfx, sTiles);
    LZDecompressWram(gMoveTypes_Pal, pals);
    for (t = 0; t < NUMBER_OF_MON_TYPES + CONTEST_CATEGORIES_COUNT; t++)
    {
        Tiles(sTiles + t * 0x100, 4, 2, pals + 16 * (t < NUMBER_OF_MON_TYPES ? MP_TypeIconPal(t) : 0), sRgba);
        for (y = 0; y < 16; y++)
            memcpy(sSheet + ((t * 16 + y) * 32) * 4, sRgba + y * 32 * 4, 32 * 4);
    }
    snprintf(path, sizeof(path), "%s/types.png", dir);
    WritePng(path, sSheet, 32, 16 * (NUMBER_OF_MON_TYPES + CONTEST_CATEGORIES_COUNT));
}

// The POKéDEX's entry screen (240x160) and every footprint (16x16 each, in species order, 32 a row)
static void ExportDexArt(const char *dir)
{
    static u16 sScreen[DISPLAY_WIDTH * DISPLAY_HEIGHT];
    static u8 sRgba[DISPLAY_WIDTH * DISPLAY_HEIGHT * 4];
    u32 i, s, n = AllSpecies(), rows = (n + 31) / 32;
    u8 *sheet = calloc(32 * 16 * rows * 16 * 4, 1);
    char path[600];
    MP_DexInfoScreen(sScreen);
    for (i = 0; i < DISPLAY_WIDTH * DISPLAY_HEIGHT; i++)
    {
        u16 c = sScreen[i];
        sRgba[i * 4] = (c & 31) * 255 / 31;
        sRgba[i * 4 + 1] = ((c >> 5) & 31) * 255 / 31;
        sRgba[i * 4 + 2] = ((c >> 10) & 31) * 255 / 31;
        sRgba[i * 4 + 3] = 255;
    }
    snprintf(path, sizeof(path), "%s/dex_info.png", dir);
    WritePng(path, sRgba, DISPLAY_WIDTH, DISPLAY_HEIGHT);
    MP_DexListScreen(sScreen);
    for (i = 0; i < DISPLAY_WIDTH * DISPLAY_HEIGHT; i++)
    {
        u16 c = sScreen[i];
        sRgba[i * 4] = (c & 31) * 255 / 31;
        sRgba[i * 4 + 1] = ((c >> 5) & 31) * 255 / 31;
        sRgba[i * 4 + 2] = ((c >> 10) & 31) * 255 / 31;
        sRgba[i * 4 + 3] = 255;
    }
    snprintf(path, sizeof(path), "%s/dex_list.png", dir);
    WritePng(path, sRgba, DISPLAY_WIDTH, DISPLAY_HEIGHT);
    if (sheet == NULL)
        return;
    for (s = 1; s < n && s < NUM_SPECIES; s++)
    {
        u8 fp[16][16];
        u32 x, y;
        MP_DexFootprint(s, fp);
        for (y = 0; y < 16; y++)
            for (x = 0; x < 16; x++)
            {
                u8 *o = sheet + ((((s / 32) * 16 + y) * 32 * 16) + (s % 32) * 16 + x) * 4;
                if (fp[y][x])
                    o[0] = 49, o[1] = 41, o[2] = 41, o[3] = 255;
            }
    }
    snprintf(path, sizeof(path), "%s/footprints.png", dir);
    WritePng(path, sheet, 32 * 16, rows * 16);
    free(sheet);
}

// BGR555 (bit 15 = shown) to RGBA
static void Rgba(const u16 *px, u32 n, u8 *rgba)
{
    u32 i;
    for (i = 0; i < n; i++)
    {
        u16 c = px[i];
        rgba[i * 4] = (c & 31) * 255 / 31;
        rgba[i * 4 + 1] = ((c >> 5) & 31) * 255 / 31;
        rgba[i * 4 + 2] = ((c >> 10) & 31) * 255 / 31;
        rgba[i * 4 + 3] = (c & 0x8000) ? 255 : 0;
    }
}

// The town maps for the live map: every page as its full view shows it (townmap_N.png; SINNOH's at Platinum's own
// size, twice the others'), their cells in data.json (ExportData)
static void ExportTownMaps(const char *dir)
{
    static u16 sPx[480 * 320];
    static u8 sRgba[480 * 320 * 4];
    char path[600];
    u32 page, w, h;
    for (page = 0; page < MP_TownMap_Pages(); page++)
        if (MP_TownMap_Picture(page, sPx, &w, &h))
        {
            Rgba(sPx, w * h, sRgba);
            snprintf(path, sizeof(path), "%s/townmap_%u.png", dir, page);
            WritePng(path, sRgba, w, h);
        }
}

// The POKéNAV's RIBBONS: every ribbon's big icon (32x32 each, one column, in ribbon order)
static void ExportRibbons(const char *dir)
{
    static u16 sPx[32 * 32];
    u32 n = MP_Ribbons_Count(), i;
    u8 *sheet = calloc(32 * 32 * 4 * n, 1);
    char path[600];
    if (sheet == NULL)
        return;
    for (i = 0; i < n; i++)
    {
        MP_Ribbons_Icon(i, sPx);
        Rgba(sPx, 32 * 32, sheet + i * 32 * 32 * 4);
    }
    snprintf(path, sizeof(path), "%s/ribbons.png", dir);
    WritePng(path, sheet, 32, 32 * n);
    free(sheet);
}

// The trainers' pictures (64x64 each, one column, by TRAINER_PIC_): MATCH CALL's people
static void ExportTrainerPics(const char *dir)
{
    static u8 sTiles[64 * 64 / 2 + 64];
    static u16 sPal[16 * 16];
    u32 n = TRAINER_PIC_MP_LOCAL, i;
    u8 *sheet = calloc(64 * 64 * 4 * n, 1);
    char path[600];
    if (sheet == NULL)
        return;
    for (i = 0; i < n; i++)
    {
        LZDecompressWram(gTrainerFrontPicTable[i].data, sTiles);
        LZDecompressWram(gTrainerFrontPicPaletteTable[i].data, sPal);
        Tiles(sTiles, 8, 8, sPal, sheet + i * 64 * 64 * 4);
    }
    snprintf(path, sizeof(path), "%s/trainers.png", dir);
    WritePng(path, sheet, 64, 64 * n);
    free(sheet);
}

static void ExportFont(const char *dir, FILE *json)
{
    static u8 sRgba[256 * 256 * 4];
    static const u8 sColors[4][4] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {64, 64, 64, 255}, {216, 216, 216, 255}};
    char path[600];
    u32 g, r, c;
    fprintf(json, "\"fontWidths\":[");
    for (g = 0; g < 256; g++)
    {
        u8 cell[16][16], width;
        MP_ExportGlyph(g, cell, &width);
        fprintf(json, "%s%u", g ? "," : "", width);
        for (r = 0; r < 16; r++)
            for (c = 0; c < 16; c++)
                memcpy(sRgba + (((g / 16) * 16 + r) * 256 + (g % 16) * 16 + c) * 4, sColors[cell[r][c] & 3], 4);
    }
    fprintf(json, "]");
    snprintf(path, sizeof(path), "%s/font.png", dir);
    WritePng(path, sRgba, 256, 256);
}

static void ExportData(const char *dir)
{
    char path[600];
    u32 i;
    FILE *f;
    snprintf(path, sizeof(path), "%s/data.json", dir);
    f = fopen(path, "w");
    if (f == NULL)
        return;
    fprintf(f, "{\"version\":1,\"egg\":%u,\"species\":[", SPECIES_EGG);
    for (i = 0; i < AllSpecies(); i++)
    {
        u16 base = i > SPECIES_EGG ? SPECIES_UNOWN : i;
        fprintf(f, "%s{\"n\":", i ? "," : "");
        if (i == SPECIES_EGG)
            Json(f, gText_EggNickname, POKEMON_NAME_LENGTH);
        else
            Json(f, gSpeciesNames[base], POKEMON_NAME_LENGTH);
        fprintf(f, ",\"dex\":%u", i < NUM_SPECIES ? SpeciesToNationalPokedexNum(i) : base == SPECIES_UNOWN ? 201 : 0);
        if (base < NUM_SPECIES)
        {
            static u16 pal[512];   // (room for any palette: Kai's hold up to 64 colors)
            u32 k;
            fprintf(f, ",\"t\":[%u,%u],\"g\":%u,\"bs\":[%u,%u,%u,%u,%u,%u]", gSpeciesInfo[base].types[0],
                    gSpeciesInfo[base].types[1], gSpeciesInfo[base].genderRatio, gSpeciesInfo[base].baseHP,
                    gSpeciesInfo[base].baseAttack, gSpeciesInfo[base].baseDefense, gSpeciesInfo[base].baseSpAttack,
                    gSpeciesInfo[base].baseSpDefense, gSpeciesInfo[base].baseSpeed);
            LZDecompressWram(MP_SpeciesFrontPal(base), pal);
            fprintf(f, ",\"pal\":\"");
            for (k = 0; k < 16; k++)
                fprintf(f, "%04X", pal[k]);
            fprintf(f, "\"");
        }
        fprintf(f, "}");
    }
    fprintf(f, "],\"moves\":[");
    for (i = 0; i < MOVES_COUNT; i++)
    {
        fprintf(f, "%s{\"n\":", i ? "," : "");
        Json(f, gMoveNames[i], MOVE_NAME_LENGTH);
        fprintf(f, ",\"t\":%u,\"p\":%u,\"a\":%u,\"pp\":%u,\"c\":%u", gBattleMoves[i].type, gBattleMoves[i].power,
                gBattleMoves[i].accuracy, gBattleMoves[i].pp, gMoveCategories[i]);
        if (i > 0)
        {
            fprintf(f, ",\"d\":");
            Json(f, gMoveDescriptionPointers[i - 1], 200);
        }
        fprintf(f, "}");
    }
    fprintf(f, "],\"items\":[");
    for (i = 0; i < ITEMS_COUNT; i++)
    {
        u8 name[32];
        CopyItemName(i, name);
        fprintf(f, "%s{\"n\":", i ? "," : "");
        Json(f, name, sizeof(name));
        fprintf(f, ",\"pocket\":%u,\"hold\":%u,\"use\":%u,\"d\":", GetItemPocket(i), MP_ItemCanBeHeld(i),
                MP_PhoneItemUse(i));
        Json(f, GetItemDescription(i), 300);
        fprintf(f, "}");
    }
    fprintf(f, "],\"abilities\":[");
    for (i = 0; i < ABILITIES_COUNT; i++)
    {
        fprintf(f, "%s", i ? "," : "");
        Json(f, gAbilityNames[i], ABILITY_NAME_LENGTH);
    }
    fprintf(f, "],\"abilityDesc\":[");
    for (i = 0; i < ABILITIES_COUNT; i++)
    {
        fprintf(f, "%s", i ? "," : "");
        Json(f, gAbilityDescriptionPointers[i], 200);
    }
    // a nature's stat changes: ATTACK, DEFENSE, SPEED, SP. ATK, SP. DEF (+1 / -1)
    fprintf(f, "],\"natureStats\":[");
    for (i = 0; i < NUM_NATURES; i++)
        fprintf(f, "%s[%d,%d,%d,%d,%d]", i ? "," : "", gNatureStatTable[i][0], gNatureStatTable[i][1],
                gNatureStatTable[i][2], gNatureStatTable[i][3], gNatureStatTable[i][4]);
    // the places a POKeMON was met (MON_DATA_MET_LOCATION: a map section)
    fprintf(f, "],\"mapsecs\":[");
    for (i = 0; i < MAPSEC_SINNOH_END; i++)
    {
        u8 name[40];
        name[0] = EOS;
        if (i < MAPSEC_NONE || (i >= MAPSEC_JOHTO_FIRST && i < MAPSEC_JOHTO_END) || i >= MAPSEC_SINNOH_FIRST)
            GetMapNameGeneric(name, i);
        fprintf(f, "%s", i ? "," : "");
        Json(f, name, sizeof(name) - 1);
    }
    // POKe BALL numbers (MON_DATA_POKEBALL) to their items, for the ball's icon
    fprintf(f, "],\"ballItems\":{");
    {
        u32 n = 0;
        for (i = 1; i < ITEMS_COUNT; i++)
            if (GetItemPocket(i) == POCKET_POKE_BALLS && ItemIdToBallId(i) != 0)
                fprintf(f, "%s\"%u\":%u", n++ ? "," : "", ItemIdToBallId(i), i);
            else if (i == ITEM_POKE_BALL)
                fprintf(f, "%s\"%u\":%u", n++ ? "," : "", ItemIdToBallId(i), i);
    }
    // the POKeDEX's entries by national number: category, height (dm), weight (hg), the four games' texts
    fprintf(f, "},\"dex\":[0");
    for (i = 1; i <= NATIONAL_DEX_COUNT; i++)
    {
        const u8 *texts[4];
        u32 k;
        MP_DexTexts(i, texts);
        fprintf(f, ",{\"cat\":");
        Json(f, MP_DexEntry(i)->categoryName, sizeof(MP_DexEntry(i)->categoryName));
        fprintf(f, ",\"h\":%u,\"w\":%u,\"tx\":[", MP_DexEntry(i)->height, MP_DexEntry(i)->weight);
        for (k = 0; k < 4; k++)
        {
            fprintf(f, "%s", k ? "," : "");
            if (texts[k] != NULL)
                Json(f, texts[k], 400);
            else
                fprintf(f, "null");
        }
        fprintf(f, "]}");
    }
    fprintf(f, "],\"natures\":[");
    for (i = 0; i < NUM_NATURES; i++)
    {
        fprintf(f, "%s", i ? "," : "");
        Json(f, gNatureNamePointers[i], 16);
    }
    fprintf(f, "],\"types\":[");
    for (i = 0; i < NUMBER_OF_MON_TYPES; i++)
    {
        fprintf(f, "%s", i ? "," : "");
        Json(f, gTypeNames[i], TYPE_NAME_LENGTH);
    }
    // the type chart as the battles read it: [attacking, defending, x10 (20 super effective, 5 not very, 0 none)];
    // the entries after the FORESIGHT mark (0xFE) are the ones FORESIGHT and ODOR SLEUTH lift
    fprintf(f, "],\"typeChart\":[");
    for (i = 0; TYPE_EFFECT_ATK_TYPE(i) != TYPE_ENDTABLE; i += 3)
        fprintf(f, "%s[%u,%u,%u]", i ? "," : "", TYPE_EFFECT_ATK_TYPE(i), TYPE_EFFECT_DEF_TYPE(i),
                TYPE_EFFECT_MULTIPLIER(i));
    // the town maps' pages: name, picture size, and the map section of each grid cell (MP_TownMap_Cell; a cell (x, y)
    // is drawn at ((x + 1) * 8, (y + 2) * 8) of the full view's 240x160)
    fprintf(f, "],\"townmaps\":[");
    for (i = 0; i < MP_TownMap_Pages(); i++)
    {
        u32 x, y;
        fprintf(f, "%s{\"n\":", i ? "," : "");
        Json(f, MP_TownMap_Title(i), 20);
        fprintf(f, ",\"x0\":1,\"y0\":2,\"cells\":[");
        for (y = 0; y < 15; y++)
            for (x = 0; x < 28; x++)
                fprintf(f, "%s%u", x || y ? "," : "", MP_TownMap_Cell(i, x, y));
        fprintf(f, "]}");
    }
    // the ribbons' words (two lines each; a gift ribbon's are the save's: "giftRibbons" by its kind, 1 first)
    fprintf(f, "],\"ribbons\":[");
    for (i = 0; i < MP_Ribbons_Count(); i++)
    {
        fprintf(f, "%s[", i ? "," : "");
        Json(f, i < FIRST_GIFT_RIBBON ? MP_Ribbons_Desc(i, 0) : (const u8 *)"\xFF", 60);
        fprintf(f, ",");
        Json(f, i < FIRST_GIFT_RIBBON ? MP_Ribbons_Desc(i, 1) : (const u8 *)"\xFF", 60);
        fprintf(f, "]");
    }
    fprintf(f, "],\"giftRibbons\":[");
    for (i = 0; i < MAX_GIFT_RIBBON; i++)
    {
        fprintf(f, "%s[", i ? "," : "");
        Json(f, gGiftRibbonDescriptionPointers[i][0] ? gGiftRibbonDescriptionPointers[i][0] : (const u8 *)"\xFF", 60);
        fprintf(f, ",");
        Json(f, gGiftRibbonDescriptionPointers[i][1] ? gGiftRibbonDescriptionPointers[i][1] : (const u8 *)"\xFF", 60);
        fprintf(f, "]");
    }
    fprintf(f, "],");
    ExportFont(dir, f);
    fprintf(f, "}\n");
    fclose(f);
}

// mp_main.c MP_FrameUpdate, at the first frame when MOEMON_EXPORT_APP is set: everything, then the game quits
void MP_Export_IfAsked(void)
{
    static bool8 sDone;
    const char *dir = getenv("MOEMON_EXPORT_APP");
    if (sDone || dir == NULL || dir[0] == '\0')
        return;
    sDone = TRUE;
    MakeDir(dir);
    ExportMons(dir);
    ExportItems(dir);
    ExportWallpapers(dir);
    ExportFrames(dir);
    ExportTypeIcons(dir);
    ExportDexArt(dir);
    ExportTownMaps(dir);
    ExportRibbons(dir);
    ExportTrainerPics(dir);
    ExportData(dir);
    printf("[export] the app's art and tables are in %s\n", dir);
    fflush(stdout);
    exit(0);
}
