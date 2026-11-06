// Moemon Maker, M3: a POKéMON's battle pictures from the user's own PNGs (docs/MOEMON_MAKER.md "The database",
// decision 22). MoemonMaker/art/SPECIES_X_front.png (64x64, or 64x128 for its two frames) and SPECIES_X_back.png (64x64;
// any colors, transparency or the top-left pixel's color for the background) take the place of the game's pictures in battle, the summary and the
// POKéDEX, in both picture sets (OPTIONS > CREATURES). The GBA draws a POKéMON in 15 colors and a clear one, so the
// pictures are fit to that here: the front and the back share one palette (a picture left as the game's is fit
// together with the new one so the two still match). SPECIES_X_shiny.png (the front, in its shiny colors) gives the
// shiny palette; without it the shiny one is the new palette turned a sixth of the way round the color wheel.
//
// The game reads its pictures and palettes through gMonFrontPicTable and friends (include/data.h MP_MONART_ART), so a
// patched copy of each table (its entries pointing at these pictures, stored as uncompressed LZ77 the way the game's
// own decompression reads them) is all it takes.
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STBI_ONLY_PNG
#define STBI_NO_HDR
#define STBI_NO_LINEAR
#define STBI_NO_SIMD
#include "../mp/third_party/stb_image.h"

#include "global.h"
#include "data.h"
#include "decompress.h"
#include "constants/species.h"
#include "maker.h"
#include "maker_api.h"

const char *Maker_ReadDir(void);
extern const u32 gMonArtCounts_Moe[4], gMonArtCounts_Pokemon[4];
#include "pokemon_icon.h"

const struct CompressedSpriteSheet *gMakerArtFront[2], *gMakerArtBack[2];
const struct CompressedSpritePalette *gMakerArtPal[2], *gMakerArtShiny[2];
const struct MonCoords *gMakerCoordsFront[2], *gMakerCoordsBack[2];
const u8 *gMakerElevation[2];
const union AnimCmd *const *const *gMakerFrontAnims[2];
static struct CompressedSpriteSheet *sFront[2], *sBack[2];
static struct CompressedSpritePalette *sPal[2], *sShiny[2];
static struct MonCoords *sCoordsF[2], *sCoordsB[2];
static u8 *sElev[2];
static const union AnimCmd *const **sAnims[2];
static const u8 **sIcons;
static u8 *sIconPal;
static int sNewCount;   // (M3b: the new POKéMON given their base's pictures)
static void *sBlocks[NUM_SPECIES * 8];   // (every buffer made, to free them on a reload)
static int sBlockCount, sArtCount;

static void *Keep(void *p)
{
    if (p && sBlockCount < (int)ARRAY_COUNT(sBlocks))
        sBlocks[sBlockCount++] = p;
    return p;
}

// Data as the game's LZ77 reader takes it: every block literal
static u32 *LzLiteral(const u8 *data, u32 size)
{
    u32 blocks = (size + 7) / 8, out = 4 + blocks * 9, i, n = 4;
    u8 *buf = calloc(1, (out + 3) & ~3u);
    buf[0] = 0x10;
    buf[1] = size & 0xFF;
    buf[2] = (size >> 8) & 0xFF;
    buf[3] = (size >> 16) & 0xFF;
    for (i = 0; i < size; i += 8)
    {
        u32 k;
        buf[n++] = 0;
        for (k = 0; k < 8; k++)
            buf[n++] = i + k < size ? data[i + k] : 0;
    }
    return Keep(buf);
}

static const struct CompressedSpriteSheet *const sSetFront[2] = {gMonFrontPicTable_Moe, gMonFrontPicTable_Pokemon};
static const struct CompressedSpriteSheet *const sSetBack[2] = {gMonBackPicTable_Moe, gMonBackPicTable_Pokemon};
static const struct CompressedSpritePalette *const sSetPal[2] = {gMonPaletteTable_Moe, gMonPaletteTable_Pokemon};
static const struct CompressedSpritePalette *const sSetShiny[2] = {gMonShinyPaletteTable_Moe, gMonShinyPaletteTable_Pokemon};

// A picture as 64 x ROWS RGBA (alpha 0 = clear; a front's second frame the first again unless the file has one), from
// the user's PNG; NULL if there's none or it can't be read
static u8 *ReadPng(const char *name, const char *kind, int rows)
{
    char path[1300];
    int w, h, n, x, y;
    u8 *img, *out;
    FILE *f;
    long size;
    u8 *file;
    snprintf(path, sizeof(path), "%s/art/%s_%s.png", Maker_ReadDir(), name, kind);
    if (!(f = fopen(path, "rb")))
        return NULL;
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);
    file = malloc(size);
    if (!file || fread(file, 1, size, f) != (size_t)size)
    {
        free(file);
        fclose(f);
        return NULL;
    }
    fclose(f);
    img = stbi_load_from_memory(file, size, &w, &h, &n, 4);
    free(file);
    if (!img)
    {
        printf("[maker] art: %s can't be read (left out)\n", path);
        return NULL;
    }
    if (w != 64 || (h != 64 && h != 128))
        printf("[maker] art: %s is %dx%d; pictures are 64x64 (fit into it)\n", path, w, h);
    out = calloc(64 * rows, 4);
    for (y = 0; y < rows; y++)
        for (x = 0; x < 64; x++)
        {
            int fy = (w == 64 && h == 128 && rows == 128) ? y : y % 64;   // (the frame's row)
            int sx = w == 64 ? x : x * w / 64, sy = (w == 64 && (h == 64 || h == 128)) ? fy : (fy % 64) * h / 64;
            const u8 *p = img + (sy * w + sx) * 4;
            u8 *q = out + (y * 64 + x) * 4;
            if (sx >= w || sy >= h)
                continue;
            memcpy(q, p, 4);
            // (no transparency in the file: the top-left pixel's color is the background)
            if (img[3] == 255 && p[0] == img[0] && p[1] == img[1] && p[2] == img[2])
                q[3] = 0;
        }
    stbi_image_free(img);
    return out;
}

// The game's own picture as 64 x ROWS RGBA (a front: 128, its two frames), from a set's table and palette
static u8 *GamePicture(const struct CompressedSpriteSheet *sheet, const struct CompressedSpritePalette *pal, int rows)
{
    u8 *tiles = calloc(1, MON_PIC_SIZE * 4), *out = calloc(64 * rows, 4);
    u16 colors[16];
    int x, y;
    LZ77UnCompWram(sheet->data, tiles);
    LZ77UnCompWram(pal->data, colors);
    for (y = 0; y < rows; y++)
        for (x = 0; x < 64; x++)
        {
            int tile = (y / 8) * 8 + x / 8, i = tiles[tile * 32 + (y % 8) * 4 + (x % 8) / 2];
            u8 *q = out + (y * 64 + x) * 4;
            i = (x & 1) ? i >> 4 : i & 15;
            if (!i)
                continue;
            q[0] = (colors[i] & 31) << 3;
            q[1] = ((colors[i] >> 5) & 31) << 3;
            q[2] = ((colors[i] >> 10) & 31) << 3;
            q[3] = 255;
        }
    free(tiles);
    return out;
}

// Median cut over the opaque pixels of one or two pictures: up to 15 colors (RGB555) in pal[1..15]
struct Box { int lo, hi; };
static u32 sPix[64 * 128 + 64 * 64];

static int Channel(u32 c, int ch) { return (c >> (ch * 5)) & 31; }

static int SortCh;
static int CmpCh(const void *a, const void *b)
{
    return Channel(*(const u32 *)a, SortCh) - Channel(*(const u32 *)b, SortCh);
}

static int MakePalette(u8 *const pics[2], u16 pal[16])
{
    struct Box boxes[15];
    int n = 0, count = 1, i, k;
    for (k = 0; k < 2; k++)
        for (i = 0; pics[k] && i < 64 * (k ? 64 : 128); i++)
            if (pics[k][i * 4 + 3] >= 128)
                sPix[n++] = (pics[k][i * 4] >> 3) | ((pics[k][i * 4 + 1] >> 3) << 5) | ((pics[k][i * 4 + 2] >> 3) << 10);
    if (!n)
        return 0;
    boxes[0].lo = 0, boxes[0].hi = n;
    while (count < 15)
    {
        int best = -1, bestRange = 0, ch = 0, b;
        for (b = 0; b < count; b++)
        {
            int c;
            if (boxes[b].hi - boxes[b].lo < 2)
                continue;
            for (c = 0; c < 3; c++)
            {
                int lo = 31, hi = 0, j;
                for (j = boxes[b].lo; j < boxes[b].hi; j++)
                {
                    int v = Channel(sPix[j], c);
                    lo = v < lo ? v : lo, hi = v > hi ? v : hi;
                }
                if (hi - lo > bestRange)
                    bestRange = hi - lo, best = b, ch = c;
            }
        }
        if (best < 0)
            break;
        SortCh = ch;
        qsort(sPix + boxes[best].lo, boxes[best].hi - boxes[best].lo, sizeof(u32), CmpCh);
        boxes[count].lo = (boxes[best].lo + boxes[best].hi) / 2;
        boxes[count].hi = boxes[best].hi;
        boxes[best].hi = boxes[count].lo;
        count++;
    }
    for (k = 0; k < count; k++)
    {
        u32 s[3] = {0, 0, 0}, m = boxes[k].hi - boxes[k].lo, j, c;
        for (j = boxes[k].lo; j < (u32)boxes[k].hi; j++)
            for (c = 0; c < 3; c++)
                s[c] += Channel(sPix[j], c);
        pal[1 + k] = m ? (s[0] / m) | ((s[1] / m) << 5) | ((s[2] / m) << 10) : 0;
    }
    for (k = count; k < 15; k++)
        pal[1 + k] = pal[count];
    return count;
}

static int Nearest(const u16 pal[16], int count, int r, int g, int b)
{
    int best = 1, bestD = 1 << 30, i;
    for (i = 1; i <= count; i++)
    {
        int dr = r - (pal[i] & 31), dg = g - ((pal[i] >> 5) & 31), db = b - ((pal[i] >> 10) & 31);
        int d = dr * dr * 3 + dg * dg * 4 + db * db * 2;
        if (d < bestD)
            bestD = d, best = i;
    }
    return best;
}

// A picture in the palette, as the game's 4bpp tiles (8x8 tiles in rows; a front's second frame after the first)
static u8 *Tiles(const u8 *rgba, int rows, const u16 pal[16], int count)
{
    u8 *t = calloc(1, MON_PIC_SIZE * rows / 64);
    int x, y;
    for (y = 0; y < rows; y++)
        for (x = 0; x < 64; x++)
        {
            const u8 *p = rgba + (y * 64 + x) * 4;
            int i = p[3] < 128 ? 0 : Nearest(pal, count, p[0] >> 3, p[1] >> 3, p[2] >> 3);
            u8 *b = &t[((y / 8) * 8 + x / 8) * 32 + (y % 8) * 4 + (x % 8) / 2];
            *b |= (x & 1) ? i << 4 : i;
        }
    return t;
}

// The shiny palette: from the shiny PNG (each color's average where the front has it), else the hue turned 60 degrees
static void ShinyPalette(const u16 pal[16], int count, const u8 *front, const u8 *shiny, u16 out[16])
{
    int i;
    out[0] = pal[0];
    for (i = 1; i < 16; i++)
    {
        int r = pal[i] & 31, g = (pal[i] >> 5) & 31, b = (pal[i] >> 10) & 31;
        if (shiny)
        {
            long s[3] = {0, 0, 0}, n = 0, p;
            for (p = 0; p < 64 * 64; p++)
                if (front[p * 4 + 3] >= 128 && shiny[p * 4 + 3] >= 128
                 && Nearest(pal, count, front[p * 4] >> 3, front[p * 4 + 1] >> 3, front[p * 4 + 2] >> 3) == i)
                    s[0] += shiny[p * 4] >> 3, s[1] += shiny[p * 4 + 1] >> 3, s[2] += shiny[p * 4 + 2] >> 3, n++;
            if (n)
                r = s[0] / n, g = s[1] / n, b = s[2] / n;
        }
        else
        {
            // (r, g, b) -> a sixth of a turn: the mean of each channel with the next
            int nr = (r + b) / 2, ng = (g + r) / 2, nb = (b + g) / 2;
            r = nr, g = ng, b = nb;
        }
        out[i] = r | (g << 5) | (b << 10);
    }
}

static void Reset(void)
{
    int i;
    for (i = 0; i < sBlockCount; i++)
        free(sBlocks[i]);
    sBlockCount = 0;
    for (i = 0; i < 2; i++)
    {
        free(sFront[i]), free(sBack[i]), free(sPal[i]), free(sShiny[i]);
        sFront[i] = NULL, sBack[i] = NULL, sPal[i] = NULL, sShiny[i] = NULL;
        gMakerArtFront[i] = NULL, gMakerArtBack[i] = NULL, gMakerArtPal[i] = NULL, gMakerArtShiny[i] = NULL;
        free(sCoordsF[i]), free(sCoordsB[i]), free(sElev[i]), free(sAnims[i]);
        sCoordsF[i] = NULL, sCoordsB[i] = NULL, sElev[i] = NULL, sAnims[i] = NULL;
        gMakerCoordsFront[i] = NULL, gMakerCoordsBack[i] = NULL, gMakerElevation[i] = NULL, gMakerFrontAnims[i] = NULL;
    }
    free(sIcons), sIcons = NULL;
    free(sIconPal), sIconPal = NULL;
    gMonIconTableTable = gMonIconTableBase;
    gMonIconPaletteIndicesTable = gMonIconPaletteIndicesBase;
    sArtCount = 0;
    sNewCount = 0;
}

// A set's four tables copied whole (Unown's other letters live past NUM_SPECIES)
static void Table(int set)
{
    const u32 *n = set ? gMonArtCounts_Pokemon : gMonArtCounts_Moe;
    if (sFront[set])
        return;
    sFront[set] = malloc(sizeof(**sFront) * n[0]);
    sBack[set] = malloc(sizeof(**sBack) * n[1]);
    sPal[set] = malloc(sizeof(**sPal) * n[2]);
    sShiny[set] = malloc(sizeof(**sShiny) * n[3]);
    memcpy(sFront[set], sSetFront[set], sizeof(**sFront) * n[0]);
    memcpy(sBack[set], sSetBack[set], sizeof(**sBack) * n[1]);
    memcpy(sPal[set], sSetPal[set], sizeof(**sPal) * n[2]);
    memcpy(sShiny[set], sSetShiny[set], sizeof(**sShiny) * n[3]);
    {
        // (and, for the new POKéMON, the coordinates, elevation and front animations)
        const u32 *c = set ? gMonArtCounts2_Pokemon : gMonArtCounts2_Moe;
        sCoordsF[set] = malloc(sizeof(**sCoordsF) * c[0]);
        sCoordsB[set] = malloc(sizeof(**sCoordsB) * c[1]);
        sAnims[set] = malloc(sizeof(**sAnims) * c[2]);
        sElev[set] = malloc(NUM_SPECIES);
        memcpy(sCoordsF[set], set ? gMonFrontPicCoords_Pokemon : gMonFrontPicCoords_Moe, sizeof(**sCoordsF) * c[0]);
        memcpy(sCoordsB[set], set ? gMonBackPicCoords_Pokemon : gMonBackPicCoords_Moe, sizeof(**sCoordsB) * c[1]);
        memcpy(sAnims[set], set ? gMonFrontAnimsPtrTable_Pokemon : gMonFrontAnimsPtrTable_Moe, sizeof(**sAnims) * c[2]);
        memcpy(sElev[set], set ? gEnemyMonElevation_Pokemon : gEnemyMonElevation_Moe, NUM_SPECIES);
    }
}

// M3b: each new POKéMON (MoemonMaker/database.txt `new species`) drawn as the POKéMON it's based on until its own
// pictures (NEW_<id>_front.png...) replace them: the rows of its base in every table, under its own tags
static void FillNewSpecies(void)
{
    int s, set;
    for (s = SPECIES_MAKER_FIRST; s <= SPECIES_MAKER_LAST; s++)
    {
        u16 base = Maker_SpeciesBase(s);
        if (base == SPECIES_NONE)
            continue;
        for (set = 0; set < 2; set++)
        {
            Table(set);
            sFront[set][s] = sSetFront[set][base], sFront[set][s].tag = s;
            sBack[set][s] = sSetBack[set][base], sBack[set][s].tag = s;
            sPal[set][s] = sSetPal[set][base], sPal[set][s].tag = s;
            sShiny[set][s] = sSetShiny[set][base], sShiny[set][s].tag = s + SPECIES_SHINY_TAG;
            sCoordsF[set][s] = sCoordsF[set][base];
            sCoordsB[set][s] = sCoordsB[set][base];
            sAnims[set][s] = sAnims[set][base];
            sElev[set][s] = sElev[set][base];
        }
        if (!sIcons)
        {
            u32 n = SPECIES_UNOWN_QMARK + 1;
            sIcons = malloc(sizeof(*sIcons) * n);
            sIconPal = malloc(n);
            memcpy(sIcons, gMonIconTableBase, sizeof(*sIcons) * n);
            memcpy(sIconPal, gMonIconPaletteIndicesBase, n);
            gMonIconTableTable = (const u8 *const *)sIcons;
            gMonIconPaletteIndicesTable = sIconPal;
        }
        sIcons[s] = gMonIconTableBase[base];
        sIconPal[s] = gMonIconPaletteIndicesBase[base];
        sNewCount++;
    }
}

static void UseCopies(void)
{
    int set;
    for (set = 0; set < 2; set++)
    {
        if (!sFront[set])
            continue;
        gMakerArtFront[set] = sFront[set];
        gMakerArtBack[set] = sBack[set];
        gMakerArtPal[set] = sPal[set];
        gMakerArtShiny[set] = sShiny[set];
        gMakerCoordsFront[set] = sCoordsF[set];
        gMakerCoordsBack[set] = sCoordsB[set];
        gMakerElevation[set] = sElev[set];
        gMakerFrontAnims[set] = (const union AnimCmd *const *const *)sAnims[set];
    }
}

// One species' pictures into both sets
static void Apply(int species, u8 *front, u8 *back, u8 *shiny)
{
    int set;
    for (set = 0; set < 2; set++)
    {
        u8 *pics[2], *ownFront = NULL, *ownBack = NULL;
        u16 pal[16], shinyPal[16];
        int count;
        int src = IS_MAKER_SPECIES(species) && Maker_SpeciesBase(species) ? Maker_SpeciesBase(species) : species;   // (M3b)
        pics[0] = front ? front : (ownFront = GamePicture(&sSetFront[set][src], &sSetPal[set][src], 128));
        pics[1] = back ? back : (ownBack = GamePicture(&sSetBack[set][src], &sSetPal[set][src], 64));
        LZ77UnCompWram(sSetPal[set][src].data, pal);
        count = MakePalette(pics, pal);
        if (count)
        {
            u8 *ft = Tiles(pics[0], 128, pal, count), *bt = Tiles(pics[1], 64, pal, count);
            Table(set);
            ShinyPalette(pal, count, pics[0], shiny, shinyPal);
            sFront[set][species].data = LzLiteral(ft, MON_PIC_SIZE * 2);
            sBack[set][species].data = LzLiteral(bt, MON_PIC_SIZE);
            sPal[set][species].data = LzLiteral((const u8 *)pal, 32);
            sShiny[set][species].data = LzLiteral((const u8 *)shinyPal, 32);
            free(ft);
            free(bt);
        }
        free(ownFront);
        free(ownBack);
    }
}

// When the game starts and when the edits are read again: every SPECIES_X_front / _back / _shiny.png in MoemonMaker/art
void Maker_LoadArt(void)
{
    char dir[1100];
    DIR *d;
    struct dirent *de;
    Reset();
    FillNewSpecies();
    UseCopies();
    snprintf(dir, sizeof(dir), "%s/art", Maker_ReadDir());
    if (!(d = opendir(dir)))
        return;
    while ((de = readdir(d)) != NULL)
    {
        char name[96];
        size_t len = strlen(de->d_name);
        unsigned v;
        u8 *front, *back, *shiny;
        // (one pass per species: its _front file, else its _back file when it has no front)
        if (len > 10 && strcmp(de->d_name + len - 10, "_front.png") == 0)
            snprintf(name, sizeof(name), "%.*s", (int)(len - 10), de->d_name);
        else if (len > 9 && strcmp(de->d_name + len - 9, "_back.png") == 0)
        {
            char fpath[1300];
            FILE *f;
            snprintf(name, sizeof(name), "%.*s", (int)(len - 9), de->d_name);
            snprintf(fpath, sizeof(fpath), "%s/%s_front.png", dir, name);
            if ((f = fopen(fpath, "rb")))
            {
                fclose(f);
                continue;   // (done with its front)
            }
        }
        else
            continue;
        if (sscanf(name, "NEW_%u", &v) == 1 && IS_MAKER_SPECIES(v))
            ;   // (M3b: a new POKéMON's own pictures, by its id)
        else if (!MakerApi_ValueOf(MAKER_NAMES_SPECIES, name, &v) || v == 0 || v >= NUM_SPECIES)
        {
            printf("[maker] art: no %s (left out)\n", name);
            continue;
        }
        front = ReadPng(name, "front", 128);
        back = ReadPng(name, "back", 64);
        shiny = ReadPng(name, "shiny", 64);
        if (front || back)
        {
            Apply(v, front, back, front ? shiny : NULL);
            sArtCount++;
        }
        free(front);
        free(back);
        free(shiny);
    }
    closedir(d);
    if (sArtCount)
    {
        UseCopies();
        printf("[maker] art: %d POKeMON with pictures of the user's own\n", sArtCount);
    }
}

void MakerApi_ReloadArt(void)
{
    Maker_LoadArt();
}

// A POKéMON's picture as the game shows it now (the active set, the user's art included), as 64x64 RGBA, for the editor
int MakerApi_SpeciesPicture(int species, int back, int shiny, unsigned char *rgba)
{
    u8 *p;
    if (species <= 0 || species >= NUM_SPECIES)
        return 0;
    p = GamePicture(back ? &gMonBackPicTable[species] : &gMonFrontPicTable[species],
                    shiny ? &gMonShinyPaletteTable[species] : &gMonPaletteTable[species], 64);
    memcpy(rgba, p, 64 * 64 * 4);
    free(p);
    return 1;
}
