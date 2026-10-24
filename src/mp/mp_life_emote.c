// Moemon Co-op: MOéMON GARDEN, Pokémon emotes (docs/POKEMON_GARDEN.md 2.2, 9).
//
// A Pokémon's feelings show in a round, soft pink bubble over it, with icons of its own (a note, a
// heart, Zzz, a sweat drop...). They never look like the Trainers' emotes (the game's square field
// icons), so nobody mistakes one for the other. The icons are drawn here from small pixel maps.
// Slots: 0 = the local follower, 1 + id = remote players' followers, then Garden Pokémon (mp_garden.c).
#include <string.h>
#include "global.h"
#include "mp.h"
#include "mp_life.h"
#include "mp_life_emote.h"
#include "main.h"
#include "palette.h"
#include "sprite.h"
#include "constants/rgb.h"

#define TAG_LIFE_EMOTE_PAL 0x5720
#define EMOTE_TIME 90

// 0 clear, 1 outline, 2 fill, 3 dark, 4 red, 5 blue, 6 yellow, 7 white
static const u16 sPalette[16] = {
    RGB(31, 0, 31), RGB(22, 8, 14), RGB(31, 29, 30), RGB(8, 4, 6), RGB(28, 8, 12),
    RGB(8, 16, 28), RGB(30, 24, 6), RGB(31, 31, 31), RGB(14, 22, 10), RGB(31, 20, 24),
};

// 9x9 icons ('.' clear, '#' dark, 'r' red, 'b' blue, 'y' yellow, 'w' white, 'g' green, 'p' pink)
static const char *const sIcons[LIFE_EMOTE_COUNT][9] = {
    [LIFE_EMOTE_NOTE] = {
        "....##...", "....#.#..", "....#..#.", "....#....", "....#....",
        "..###....", ".####....", ".###.....", "........."},
    [LIFE_EMOTE_HEART] = {
        ".........", ".rr...rr.", "rrrr.rrrr", "rrrrrrrrr", "rrrrrrrrr",
        ".rrrrrrr.", "..rrrrr..", "...rrr...", "....r...."},
    [LIFE_EMOTE_ZZZ] = {
        "....#####", ".......#.", "......#..", ".....#...", "....#####",
        "####.....", "..#......", ".#.......", "####....."},
    [LIFE_EMOTE_EXCLAIM] = {
        "...###...", "...###...", "...###...", "...###...", "....#....",
        "....#....", ".........", "...###...", "...###..."},
    [LIFE_EMOTE_QUESTION] = {
        "..####...", ".##..##..", ".....##..", "....##...", "...##....",
        "...##....", ".........", "...##....", "...##...."},
    [LIFE_EMOTE_SWEAT] = {
        "....b....", "...bbb...", "..bbbbb..", "..bbwbb..", ".bbbwbbb.",
        ".bbbbbbb.", ".bbbbbbb.", "..bbbbb..", "...bbb..."},
    [LIFE_EMOTE_ANGRY] = {
        ".rr...rr.", "r..r.r..r", "r.......r", ".r.....r.", ".........",
        ".r.....r.", "r.......r", "r..r.r..r", ".rr...rr."},
    [LIFE_EMOTE_TEAR] = {
        ".........", ".##...##.", ".........", ".b.......", ".b.......",
        "bbb.###..", ".b.#...#.", ".........", "........."},
    [LIFE_EMOTE_SPARKLE] = {
        "....y....", "....y....", "...yyy...", "yyyyyyyyy", ".yyyyyyy.",
        "..yyyyy..", "..yy.yy..", ".yy...yy.", "........."},
    [LIFE_EMOTE_FOOD] = {
        "....g....", "...gg....", "..rrrrr..", ".rrrrrrr.", ".rrwrrrr.",
        ".rrrrrrr.", ".rrrrrrr.", "..rrrrr..", "...rrr..."},
    [LIFE_EMOTE_DOTS] = {
        ".........", ".........", ".........", ".........", "##.##.##.",
        "##.##.##.", ".........", ".........", "........."},
    [LIFE_EMOTE_CALM] = {
        ".........", ".........", ".##...##.", "#..#.#..#", "....#....",
        ".........", ".##...##.", "#..#.#..#", "....#...."},
    [LIFE_EMOTE_SUN] = {
        "....y....", ".y..y..y.", "..yyyyy..", "..yyyyy..", "yyyyyyyyy",
        "..yyyyy..", "..yyyyy..", ".y..y..y.", "....y...."},
    [LIFE_EMOTE_WATER] = {
        ".b.....b.", ".b..b..b.", "bbb.b.bbb", "....b....", "...bbb...",
        "..bbbbb..", "..bbwbb..", "..bbbbb..", "...bbb..."},
    [LIFE_EMOTE_BALL] = {
        "..#####..", ".#rrrrr#.", "#rrrrrrr#", "#rr###rr#", "####w####",
        "#ww###ww#", "#wwwwwww#", ".#wwwww#.", "..#####.."},
    [LIFE_EMOTE_LEAF] = {
        "......gg.", "....gggg.", "...ggggg.", "..ggggg#.", ".gggg#g..",
        ".ggg#gg..", ".g##gg...", "##.......", "#........"},
};

static u8 sTiles[LIFE_EMOTE_COUNT][4 * 32];
static struct SpriteFrameImage sImages[LIFE_EMOTE_COUNT];
static bool8 sDrawn;

struct EmoteSlot
{
    u8 spriteId;
    u8 icon;
    u16 timer;
};
static struct EmoteSlot sSlots[LIFE_EMOTE_SLOTS];
static bool8 sInit;

static const struct OamData sOam = {.shape = SPRITE_SHAPE(16x16), .size = SPRITE_SIZE(16x16), .priority = 1};

#define ANIM(n) static const union AnimCmd sAnim##n[] = {ANIMCMD_FRAME(n, 1), ANIMCMD_END}
ANIM(0); ANIM(1); ANIM(2); ANIM(3); ANIM(4); ANIM(5); ANIM(6); ANIM(7);
ANIM(8); ANIM(9); ANIM(10); ANIM(11); ANIM(12); ANIM(13); ANIM(14); ANIM(15);
static const union AnimCmd *const sAnims[LIFE_EMOTE_COUNT] = {
    sAnim0, sAnim1, sAnim2, sAnim3, sAnim4, sAnim5, sAnim6, sAnim7,
    sAnim8, sAnim9, sAnim10, sAnim11, sAnim12, sAnim13, sAnim14, sAnim15,
};

static void SpriteCB_Emote(struct Sprite *sprite)
{
}

static void PutPixel(u8 *tiles, int x, int y, u8 c)
{
    // 16x16 sprite, 1D mapping: tiles TL, TR, BL, BR; 4 bits a pixel, low nibble first
    int t = (y / 8) * 2 + (x / 8);
    u8 *b = &tiles[t * 32 + (y % 8) * 4 + (x % 8) / 2];
    if (x & 1)
        *b = (*b & 0x0F) | (c << 4);
    else
        *b = (*b & 0xF0) | c;
}

// A round bubble (an ellipse with a little tail at the bottom) and the icon in it
static void DrawIcons(void)
{
    u8 i;
    int x, y;
    for (i = 0; i < LIFE_EMOTE_COUNT; i++)
    {
        u8 *t = sTiles[i];
        memset(t, 0, sizeof(sTiles[i]));
        for (y = 0; y < 14; y++)
            for (x = 0; x < 16; x++)
            {
                // ellipse centered at (7.5, 6.5), radii 7.5 x 6.5, in units of 1/2 pixel
                int dx = 2 * x - 15, dy = 2 * y - 13;
                int d = dx * dx * 169 + dy * dy * 225;   // (dx/15)^2 + (dy/13)^2 scaled by 15^2 * 13^2
                int r = 15 * 15 * 13 * 13;
                if (d <= r * 72 / 100)
                    PutPixel(t, x, y, 2);
                else if (d <= r)
                    PutPixel(t, x, y, 1);
            }
        // the tail
        PutPixel(t, 5, 13, 1); PutPixel(t, 6, 13, 2); PutPixel(t, 7, 13, 1);
        PutPixel(t, 5, 14, 1); PutPixel(t, 6, 14, 1);
        PutPixel(t, 5, 15, 1);
        // the icon, centered
        if (sIcons[i][0] == NULL)
            continue;
        for (y = 0; y < 9; y++)
            for (x = 0; x < 9; x++)
            {
                u8 c = 0;
                switch (sIcons[i][y][x])
                {
                case '#': c = 3; break;
                case 'r': c = 4; break;
                case 'b': c = 5; break;
                case 'y': c = 6; break;
                case 'w': c = 7; break;
                case 'g': c = 8; break;
                case 'p': c = 9; break;
                }
                if (c)
                    PutPixel(t, x + 4, y + 2, c);
            }
        sImages[i].data = sTiles[i];
        sImages[i].size = sizeof(sTiles[i]);
    }
    sDrawn = TRUE;
}

static void InitIfNeeded(void)
{
    u8 i;
    if (!sDrawn)
        DrawIcons();
    if (sInit)
        return;
    for (i = 0; i < LIFE_EMOTE_SLOTS; i++)
        sSlots[i].spriteId = MAX_SPRITES;
    sInit = TRUE;
}

static bool8 SpriteIsOurs(struct EmoteSlot *s)
{
    return s->spriteId < MAX_SPRITES && gSprites[s->spriteId].inUse && gSprites[s->spriteId].callback == SpriteCB_Emote
        && gSprites[s->spriteId].data[7] == (s16)(s - sSlots);
}

static void Destroy(struct EmoteSlot *s)
{
    if (SpriteIsOurs(s))
        DestroySprite(&gSprites[s->spriteId]);
    s->spriteId = MAX_SPRITES;
}

void MP_LifeEmote_Show(u8 slot, u8 icon)
{
    InitIfNeeded();
    if (slot >= LIFE_EMOTE_SLOTS || icon >= LIFE_EMOTE_COUNT)
        return;
    sSlots[slot].icon = icon;
    sSlots[slot].timer = EMOTE_TIME;
    if (SpriteIsOurs(&sSlots[slot]))
        StartSpriteAnim(&gSprites[sSlots[slot].spriteId], icon);
    MPNet_Log("life emote: slot %u icon %u", slot, icon);
}

u8 MP_LifeEmote_Current(u8 slot)
{
    if (slot >= LIFE_EMOTE_SLOTS || sSlots[slot].timer == 0)
        return LIFE_EMOTE_NONE;
    return sSlots[slot].icon;
}

// Every frame for each slot: where the Pokémon's sprite is (screen x, y of its top), or visible = FALSE
void MP_LifeEmote_Place(u8 slot, bool8 visible, s16 x, s16 top, u8 subpriority)
{
    struct EmoteSlot *s;
    struct Sprite *sp;
    static struct SpriteTemplate tmpl;
    InitIfNeeded();
    if (slot >= LIFE_EMOTE_SLOTS)
        return;
    s = &sSlots[slot];
    if (s->timer)
        s->timer--;
    if (!visible || s->timer == 0)
    {
        Destroy(s);
        return;
    }
    if (!SpriteIsOurs(s))
    {
        u8 pal = IndexOfSpritePaletteTag(TAG_LIFE_EMOTE_PAL);
        if (pal == 0xFF && MP_AllocCoopPalette(TAG_LIFE_EMOTE_PAL, sPalette) == 0xFF)
            return;
        tmpl.tileTag = TAG_NONE;
        tmpl.paletteTag = TAG_LIFE_EMOTE_PAL;
        tmpl.oam = &sOam;
        tmpl.anims = sAnims;
        tmpl.images = sImages;
        tmpl.affineAnims = gDummySpriteAffineAnimTable;
        tmpl.callback = SpriteCB_Emote;
        s->spriteId = CreateSprite(&tmpl, 0, 0, 0);
        if (s->spriteId >= MAX_SPRITES)
        {
            s->spriteId = MAX_SPRITES;
            return;
        }
        gSprites[s->spriteId].data[7] = slot;
        StartSpriteAnim(&gSprites[s->spriteId], s->icon);
    }
    sp = &gSprites[s->spriteId];
    {
        u8 pal = IndexOfSpritePaletteTag(TAG_LIFE_EMOTE_PAL);
        if (pal == 0xFF)
        {
            Destroy(s);
            return;
        }
        sp->oam.paletteNum = pal;
    }
    sp->oam.priority = gMain.inBattle ? 0 : 1;   // in battle, over the health boxes (it's small and brief)
    sp->x = x;
    sp->y = top - 6;
    // a little pop when it appears, then it bobs gently
    {
        u16 age = EMOTE_TIME - s->timer;
        sp->y2 = age < 6 ? -(s16)(6 - age) : ((age / 16) & 1 ? -1 : 0);
    }
    sp->subpriority = subpriority;
    sp->invisible = FALSE;
}

void MP_LifeEmote_HideAll(void)
{
    u8 i;
    InitIfNeeded();
    for (i = 0; i < LIFE_EMOTE_SLOTS; i++)
    {
        Destroy(&sSlots[i]);
        sSlots[i].timer = 0;
    }
}
