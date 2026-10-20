// Moemon Co-op: emotes and phrases above players' heads.
//
//   !  ?  heart   the game's own field-effect icons, sent in the player state
//   note ...      and phrases: a speech bubble, sent as a relay message
//
// Each player has 16 custom phrases made in the game's word editor (the one
// used for the TRAINER profile), plus a list of quick phrases. Use them from
// CO-OP menu > EMOTES, the number keys 1-8 or a controller's LT / RT hold menu.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "mp.h"
#include "battle.h"
#include "battle_anim.h"
#include "main.h"
#include "easy_chat.h"
#include "event_object_lock.h"
#include "event_object_movement.h"
#include "field_effect.h"
#include "field_player_avatar.h"
#include "field_screen_effect.h"
#include "field_weather.h"
#include "overworld.h"
#include "palette.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "constants/easy_chat.h"
#include "constants/field_effects.h"
#include "constants/songs.h"
#include "constants/characters.h"
#include "constants/rgb.h"
#include "constants/maps.h"
#include "fieldmap.h"
#include "script.h"

void MP_AsciiToGame(u8 *dst, const char *src, int maxLen);
u8 MP_RenderSmallGlyph(u16 glyphId, u8 fgColor, u8 bgColor, u8 shadowColor);
void MP_FieldClearVBlankHBlankCallbacks(void);


#define BUBBLE_W      128
#define BUBBLE_H      32
#define BUBBLE_TIME   (60 * 3)
#define BUBBLE_TEXT   96
#define LINE_H        12
#define NUM_BUBBLES   (2 + MP_MAX_PLAYERS * 3) // 0 = you, 1 + id = remote players, door labels, pings, toast
#define TOAST_SLOT    (NUM_BUBBLES - 1)
#define TOAST_TIME    (60 * 4)
#define LABEL_SLOT(id) (1 + MP_MAX_PLAYERS + (id))
#define PING_SLOT(id)  (1 + MP_MAX_PLAYERS * 2 + (id))
#define TAG_BUBBLE    0x53A0                 // + slot * 2 + half
#define TAG_BUBBLE_PAL MP_TAG_UI_PAL

enum { COL_CLEAR, COL_FILL, COL_TEXT, COL_SHADOW, COL_EDGE };

struct Bubble
{
    u8 text[BUBBLE_TEXT];   // game text, CHAR_NEWLINE between lines
    u8 note;                // MP_BUBBLE_ICON_*: a picture instead of text
    u8 battler;             // in battle: over this battler (MAX_BATTLERS_COUNT = not a battle bubble)
    u16 timer;              // frames left; 0 = no bubble
    bool8 dirty;            // text changed, redraw the tiles
    u8 spriteId[2];         // left and right 64x32 halves
    bool8 atTile;           // door label: shown over a map tile, not a person
    bool8 toast;            // a note at the top of the screen (no tail)
    s16 tileX, tileY;       // that tile (map coords with MAP_OFFSET)
    s8 boxShift;            // the box's offset from the tail (a battle bubble near a screen edge)
    u8 boxW;                // the box's width, from BuildBubble
};

static EWRAM_DATA struct Bubble sBubbles[NUM_BUBBLES];
static bool8 sInit;
static u8 sPixels[BUBBLE_W * BUBBLE_H];
static u8 sTiles[2][64 * 32 / 2];

// One sprite palette for all co-op overlays (bubbles, toasts and the compass
// arrows), so they don't take palette slots the map's people need
static const u16 sBubblePal[16] = {
    RGB(0, 0, 0), RGB(31, 31, 31), RGB(7, 7, 8), RGB(20, 20, 21), RGB(9, 9, 11),
    // Toasts, in the co-op menu style (filled in from the player's window frame)
    RGB(4, 4, 12), RGB(31, 31, 31), RGB(1, 1, 5), RGB(31, 25, 6), RGB(11, 4, 19),
    // Compass arrows: red, blue, green, yellow players, outline
    RGB(31, 8, 8), RGB(8, 16, 31), RGB(6, 26, 6), RGB(31, 27, 4), RGB(2, 2, 4),
    // The emote icons' pink heart (trainer_see.c MpIconLook: white 1, black 14, pink 15)
    RGB(31, 20, 22),
};

static const struct OamData sOam_Bubble = {
    .shape = SPRITE_SHAPE(64x32),
    .size = SPRITE_SIZE(64x32),
    .priority = 1,
};

// 9x11 music note (1 = ink)
static const char *const sNoteBitmap[] = {
    "...######",
    "...######",
    "...#....#",
    "...#....#",
    "...#....#",
    "...#....#",
    ".###..###",
    "####.####",
    "####.####",
    ".##...##.",
};

// 9x8 heart (battle bubbles; the field shows the heart emote icon instead)
static const char *const sHeartBitmap[] = {
    ".##...##.",
    "####.####",
    "#########",
    "#########",
    ".#######.",
    "..#####..",
    "...###...",
    "....#....",
};

// Quick phrases (any player can use them; they're sent as text)
// Ready-made phrases, 4 per group (the controller's R2 quick pick shows them by group)
static const char *const sQuickPhrases[] = {
    "Hello!", "See you!", "Thank you!", "Good game!",          // GREET
    "Follow me!", "Wait for me!", "Over here!", "Let's go!",   // MOVE
    "Let's battle!", "Let's team up!", "Nice!", "Help!",       // BATTLE
    "Yes!", "No!", "Be right back!", "Hurry up!",              // MORE
};

// A ready-made phrase as ASCII ("" when out of range)
const char *MP_Emote_StockPhrase(u8 i)
{
    return i < ARRAY_COUNT(sQuickPhrases) ? sQuickPhrases[i] : "";
}

static void SpriteCB_Bubble(struct Sprite *sprite)
{
    // Moved by MP_Emote_Update
}

static void InitIfNeeded(void)
{
    u8 i;
    if (sInit)
        return;
    memset(sBubbles, 0, sizeof(sBubbles));
    for (i = 0; i < NUM_BUBBLES; i++)
    {
        sBubbles[i].spriteId[0] = sBubbles[i].spriteId[1] = MAX_SPRITES;
        sBubbles[i].battler = MAX_BATTLERS_COUNT;
    }
    sInit = TRUE;
}

// ---------------------------------------------------------------------------
// Drawing

static void Put(int x, int y, u8 c)
{
    if (x >= 0 && x < BUBBLE_W && y >= 0 && y < BUBBLE_H)
        sPixels[y * BUBBLE_W + x] = c;
}

static int GlyphWidth(u16 glyph)
{
    if (glyph == CHAR_SPACE)
        return 3;
    return MP_RenderSmallGlyph(glyph, COL_TEXT, COL_CLEAR, COL_SHADOW);
}

static int DrawGlyph(int x, int y, u16 glyph)
{
    int w, gx, gy;
    if (glyph == CHAR_SPACE)
        return 3;
    w = MP_RenderSmallGlyph(glyph, COL_TEXT, COL_CLEAR, COL_SHADOW);
    for (gy = 0; gy < 16; gy++)
    {
        const u32 *rows = (gy < 8) ? gCurGlyph.gfxBufferTop : gCurGlyph.gfxBufferBottom;
        for (gx = 0; gx < w && gx < 16; gx++)
        {
            u32 row = rows[(gx < 8 ? 0 : 8) + (gy % 8)];
            u8 c = (row >> ((gx % 8) * 4)) & 0xF;
            if (c)
                Put(x + gx, y + gy, c);
        }
    }
    return w;
}

// Splits the text into at most 2 lines; returns the number of lines
static int SplitLines(const u8 *text, const u8 *lines[2], int lens[2])
{
    int n = 0, i = 0;
    lines[0] = text;
    lens[0] = lens[1] = 0;
    while (text[i] != EOS && n < 2)
    {
        if (text[i] == CHAR_NEWLINE)
        {
            if (lens[n] > 0)
                n++;
            if (n < 2)
            {
                lines[n] = text + i + 1;
                lens[n] = 0;
            }
        }
        else
            lens[n]++;
        i++;
    }
    if (n < 2 && lens[n] > 0)
        n++;
    return n;
}

static int LineWidth(const u8 *s, int len)
{
    int i, w = 0;
    for (i = 0; i < len; i++)
        w += GlyphWidth(s[i]);
    return w;
}

#define TEXT_MAX_W (BUBBLE_W - 8)

// Wraps the text into 2 lines that fit in the bubble, breaking at spaces;
// text that still doesn't fit ends in ".."
static void FitText(const u8 *in, u8 *out, int outSize)
{
    int n = 0, line = 0, lineW = 0, i = 0;
    while (in[i] != EOS && n < outSize - 4)
    {
        // Next word (up to a space or line break)
        int j = i, wordW = 0, k;
        if (in[i] == CHAR_NEWLINE)
        {
            if (++line >= 2)
                break;
            out[n++] = CHAR_NEWLINE;
            lineW = 0;
            i++;
            continue;
        }
        if (in[i] == CHAR_SPACE)
        {
            if (lineW > 0 && lineW + 3 <= TEXT_MAX_W)
            {
                out[n++] = CHAR_SPACE;
                lineW += 3;
            }
            i++;
            continue;
        }
        while (in[j] != EOS && in[j] != CHAR_SPACE && in[j] != CHAR_NEWLINE)
            wordW += GlyphWidth(in[j++]);
        if (lineW + wordW > TEXT_MAX_W && lineW > 0)
        {
            // Start a new line (drop the space we just added)
            if (n > 0 && out[n - 1] == CHAR_SPACE)
                n--;
            if (++line >= 2)
                goto cut;
            out[n++] = CHAR_NEWLINE;
            lineW = 0;
        }
        for (k = i; k < j && n < outSize - 4; k++)
        {
            int gw = GlyphWidth(in[k]);
            if (lineW + gw > TEXT_MAX_W)
            {
                if (++line >= 2)
                    goto cut;
                out[n++] = CHAR_NEWLINE;
                lineW = 0;
            }
            out[n++] = in[k];
            lineW += gw;
        }
        i = j;
    }
    out[n] = EOS;
    return;
cut:
    // Make room for ".." at the end of the last line
    while (n > 0 && out[n - 1] != CHAR_NEWLINE && lineW + 2 * GlyphWidth(CHAR_PERIOD) > TEXT_MAX_W)
        lineW -= GlyphWidth(out[--n]);
    out[n++] = CHAR_PERIOD;
    out[n++] = CHAR_PERIOD;
    out[n] = EOS;
}

static void BuildBubble(struct Bubble *b)
{
    const u8 *lines[2];
    int lens[2], nLines, i, x, y, w = 0, h, left, top, bottom;
    u8 fitted[BUBBLE_TEXT + 8];

    memset(sPixels, 0, sizeof(sPixels));
    if (b->note)
    {
        nLines = 1;
        w = 9;
        lens[0] = 0;
    }
    else
    {
        FitText(b->text, fitted, sizeof(fitted));
        nLines = SplitLines(fitted, lines, lens);
        if (nLines == 0)
            return;
        for (i = 0; i < nLines; i++)
        {
            int lw = LineWidth(lines[i], lens[i]);
            if (lw > w)
                w = lw;
        }
    }
    if (w > BUBBLE_W - 8)
        w = BUBBLE_W - 8;
    h = nLines * LINE_H + 2;
    bottom = BUBBLE_H - 4;          // the tail goes below the box
    top = bottom - h;
    left = (BUBBLE_W - (w + 6)) / 2;
    // Moemon Co-op: a battle bubble near the screen edge slides its box over, the tail
    // stays on the speaker (never past the box's rounded end)
    if (b->boxShift)
    {
        int maxShift = (w + 6) / 2 - 6;
        int shift = b->boxShift;
        if (maxShift < 0)
            maxShift = 0;
        if (shift > maxShift)
            shift = maxShift;
        if (shift < -maxShift)
            shift = -maxShift;
        left += shift;
        if (left < 0)
            left = 0;
        if (left + w + 5 > BUBBLE_W - 1)
            left = BUBBLE_W - 1 - (w + 5);
    }
    b->boxW = w + 6;

    // Rounded box with an outline
    for (y = top; y <= bottom; y++)
        for (x = left; x <= left + w + 5; x++)
        {
            bool8 edge = (y == top || y == bottom || x == left || x == left + w + 5);
            bool8 corner = ((y == top || y == bottom) && (x == left || x == left + w + 5));
            if (!corner)
                Put(x, y, edge ? COL_EDGE : COL_FILL);
        }
    // Tail
    for (i = 0; i < 3 && !b->toast; i++)
        for (x = BUBBLE_W / 2 - 3 + i; x <= BUBBLE_W / 2 + 2 - i; x++)
            Put(x, bottom + i, (x == BUBBLE_W / 2 - 3 + i || x == BUBBLE_W / 2 + 2 - i) ? COL_EDGE : COL_FILL);
    if (!b->toast)
    {
        for (x = BUBBLE_W / 2 - 2; x <= BUBBLE_W / 2 + 1; x++)
            Put(x, bottom, COL_FILL);
        Put(BUBBLE_W / 2 - 1, bottom + 3, COL_EDGE);
        Put(BUBBLE_W / 2, bottom + 3, COL_EDGE);
    }

    if (b->note)
    {
        const char *const *bitmap = b->note == MP_BUBBLE_ICON_HEART ? sHeartBitmap : sNoteBitmap;
        int rows = b->note == MP_BUBBLE_ICON_HEART ? (int)ARRAY_COUNT(sHeartBitmap) : (int)ARRAY_COUNT(sNoteBitmap);
        for (y = 0; y < rows; y++)
            for (x = 0; bitmap[y][x]; x++)
                if (bitmap[y][x] == '#')
                    Put(left + 3 + x, top + 2 + y, b->note == MP_BUBBLE_ICON_HEART ? 15 : COL_TEXT);
    }
    else
    {
        for (i = 0; i < nLines; i++)
        {
            int lw = LineWidth(lines[i], lens[i]), k;
            int cx = left + 3 + (lw < w ? (w - lw) / 2 : 0);
            for (k = 0; k < lens[i] && cx < left + w + 3; k++)
                cx += DrawGlyph(cx, top - 1 + i * LINE_H, lines[i][k]);
        }
        // Text that ran past the box: tidy the right edge again
        for (y = top; y <= bottom; y++)
            for (x = left + w + 5; x < BUBBLE_W; x++)
                if (x == left + w + 5)
                    Put(x, y, (y == top || y == bottom) ? COL_CLEAR : COL_EDGE);
                else
                    Put(x, y, COL_CLEAR);
    }

    // Toasts use the co-op menu colors, with a purple band along the top
    if (b->toast)
    {
        static const u8 sToastColor[] = {[COL_CLEAR] = 0, [COL_FILL] = 5, [COL_TEXT] = 6, [COL_SHADOW] = 7, [COL_EDGE] = 8};
        int topRow = -1;
        for (i = 0; i < BUBBLE_W * BUBBLE_H; i++)
        {
            if (sPixels[i] <= COL_EDGE)
                sPixels[i] = sToastColor[sPixels[i]];
            if (topRow < 0 && sPixels[i] != 0)
                topRow = i / BUBBLE_W;
        }
        for (i = 0; topRow >= 0 && i < BUBBLE_W; i++)
            if (sPixels[(topRow + 1) * BUBBLE_W + i] == 5)
                sPixels[(topRow + 1) * BUBBLE_W + i] = 9;
    }

    // Two 64x32 halves, 4bpp 1D tiles
    memset(sTiles, 0, sizeof(sTiles));
    for (i = 0; i < 2; i++)
    {
        int tx, ty, px, py;
        for (ty = 0; ty < 4; ty++)
            for (tx = 0; tx < 8; tx++)
            {
                u8 *tile = sTiles[i] + (ty * 8 + tx) * 32;
                for (py = 0; py < 8; py++)
                    for (px = 0; px < 8; px++)
                    {
                        u8 c = sPixels[(ty * 8 + py) * BUBBLE_W + i * 64 + tx * 8 + px];
                        int k = py * 8 + px;
                        tile[k / 2] |= (k & 1) ? (c << 4) : c;
                    }
            }
    }
}

// ---------------------------------------------------------------------------
// Sprites

static bool8 HalfIsOurs(struct Bubble *b, u8 half)
{
    u8 s = b->spriteId[half];
    return s < MAX_SPRITES && gSprites[s].inUse && gSprites[s].callback == SpriteCB_Bubble
        && gSprites[s].data[0] == (s16)((b - sBubbles) * 2 + half + 1);
}

static void DestroyBubbleSprites(struct Bubble *b)
{
    u8 half;
    for (half = 0; half < 2; half++)
    {
        u16 tag = TAG_BUBBLE + (b - sBubbles) * 2 + half;
        if (HalfIsOurs(b, half))
            DestroySprite(&gSprites[b->spriteId[half]]);
        b->spriteId[half] = MAX_SPRITES;
        FreeSpriteTilesByTag(tag);
    }
}

static bool8 AnyBubbleShown(void)
{
    u8 i;
    for (i = 0; i < NUM_BUBBLES; i++)
        if (HalfIsOurs(&sBubbles[i], 0))
            return TRUE;
    return FALSE;
}

// Loads the shared overlay palette; returns FALSE when no slot is free
bool8 MP_Emote_LoadUiPalette(void)
{
    static u16 sPal[16];
    if (IndexOfSpritePaletteTag(TAG_BUBBLE_PAL) != 0xFF)
        return TRUE;
    {
        // Toasts take the co-op menu colors (from the player's window frame)
        u16 theme[16];
        MpTheme_BuildPalette(theme);
        memcpy(sPal, sBubblePal, sizeof(sPal));
        sPal[5] = theme[MPT_BG];       // white
        sPal[6] = theme[MPT_TEXT];     // dark grey
        sPal[7] = theme[MPT_SHADOW];   // light grey
        sPal[8] = theme[MPT_BORDER];   // the frame's color
        sPal[9] = theme[MPT_BAND];
    }
    return MP_AllocCoopPalette(TAG_BUBBLE_PAL, sPal) != 0xFF;
}

static bool8 CreateBubbleSprites(struct Bubble *b)
{
    u8 half;

    if (!MP_Emote_LoadUiPalette())
        return FALSE;
    BuildBubble(b);
    for (half = 0; half < 2; half++)
    {
        u16 tag = TAG_BUBBLE + (b - sBubbles) * 2 + half;
        struct SpriteSheet sheet = {sTiles[half], sizeof(sTiles[half]), tag};
        struct SpriteTemplate tmpl = {
            .tileTag = tag, .paletteTag = TAG_BUBBLE_PAL, .oam = &sOam_Bubble,
            .anims = gDummySpriteAnimTable, .images = NULL,
            .affineAnims = gDummySpriteAffineAnimTable, .callback = SpriteCB_Bubble,
        };
        u8 id;
        FreeSpriteTilesByTag(tag);
        if (LoadSpriteSheet(&sheet) == 0)
        {
            DestroyBubbleSprites(b);
            return FALSE;
        }
        id = CreateSprite(&tmpl, 0, 0, 0);
        if (id == MAX_SPRITES)
        {
            DestroyBubbleSprites(b);
            return FALSE;
        }
        gSprites[id].data[0] = (b - sBubbles) * 2 + half + 1;
        gSprites[id].coordOffsetEnabled = TRUE;
        gSprites[id].invisible = TRUE;
        b->spriteId[half] = id;
    }
    b->dirty = FALSE;
    return TRUE;
}

static void RedrawBubble(struct Bubble *b)
{
    u8 half;
    BuildBubble(b);
    for (half = 0; half < 2; half++)
    {
        u16 start = GetSpriteTileStartByTag(TAG_BUBBLE + (b - sBubbles) * 2 + half);
        if (start != 0xFFFF)
            memcpy((void *)(OBJ_VRAM0 + start * TILE_SIZE_4BPP), sTiles[half], sizeof(sTiles[half]));
    }
    b->dirty = FALSE;
}

static void UpdateBubble(struct Bubble *b, u8 ownerObj)
{
    struct Sprite *os;
    s16 x, y;
    u8 half;

    if (b->timer == 0 || ownerObj >= OBJECT_EVENTS_COUNT || !gObjectEvents[ownerObj].active
     || gObjectEvents[ownerObj].spriteId >= MAX_SPRITES)
    {
        if (HalfIsOurs(b, 0) || HalfIsOurs(b, 1) || b->spriteId[0] != MAX_SPRITES)
            DestroyBubbleSprites(b);
        if (b->timer)
            b->timer--;
        return;
    }
    b->timer--;
    if (!HalfIsOurs(b, 0) || !HalfIsOurs(b, 1))
    {
        DestroyBubbleSprites(b);
        if (!CreateBubbleSprites(b))
            return;
    }
    else if (b->dirty)
        RedrawBubble(b);

    os = &gSprites[gObjectEvents[ownerObj].spriteId];
    if (b->toast)
    {
        x = DISPLAY_WIDTH / 2;
        y = 18;
    }
    else if (b->atTile)
    {
        // Over the door: the tail points at the top of the tile
        SetSpritePosToMapCoords(b->tileX, b->tileY, &x, &y);
        x += 8;
        y -= 12;
    }
    else
    {
        x = os->x + os->x2;
        y = os->y + os->y2 + os->centerToCornerVecY - 16 + 2;   // bubble bottom just above the head
    }
    for (half = 0; half < 2; half++)
    {
        struct Sprite *s = &gSprites[b->spriteId[half]];
        s->x = x - 32 + half * 64;
        s->y = y;
        if (b->toast)
            s->invisible = ArePlayerFieldControlsLocked();
        else
            s->invisible = b->atTile ? FALSE : os->invisible;
        s->subpriority = b->atTile ? 1 : 0;
        s->coordOffsetEnabled = !b->toast;
        if (b->toast)
            s->oam.priority = 0;
    }
    if (b->timer == 0)
        DestroyBubbleSprites(b);
}

// ---------------------------------------------------------------------------
// Showing and sending

static void ShowBubble(u8 slot, const u8 *text, u8 note)
{
    struct Bubble *b;
    InitIfNeeded();
    if (slot >= NUM_BUBBLES)
        return;
    b = &sBubbles[slot];
    memset(b->text, EOS, sizeof(b->text));
    if (text)
        StringCopyN(b->text, text, BUBBLE_TEXT - 1);
    b->note = note;
    b->battler = MAX_BATTLERS_COUNT;
    b->boxShift = 0;
    b->timer = BUBBLE_TIME;
    b->dirty = TRUE;
}

void MP_StartNoteIcon(void);
bool8 MP_EmoteIconShown(void);

// The music note icon over an object (trainer_see.c)
static void StartNoteIcon(struct ObjectEvent *obj)
{
    if (obj == NULL || !obj->active)
        return;
    gFieldEffectArguments[0] = obj->localId;
    gFieldEffectArguments[1] = obj->mapNum;
    gFieldEffectArguments[2] = obj->mapGroup;
    MP_StartNoteIcon();
}

// The !, ? and heart icons over an object
static void StartIcon(struct ObjectEvent *obj, u8 kind)
{
    static const u8 sEffects[] = {
        [MP_EMOTE_EXCLAMATION] = FLDEFF_EXCLAMATION_MARK_ICON,
        [MP_EMOTE_QUESTION]    = FLDEFF_QUESTION_MARK_ICON,
        [MP_EMOTE_HEART]       = FLDEFF_HEART_ICON,
    };
    if (kind == 0 || kind >= ARRAY_COUNT(sEffects) || !obj->active)
        return;
    gFieldEffectArguments[0] = obj->localId;
    gFieldEffectArguments[1] = obj->mapNum;
    gFieldEffectArguments[2] = obj->mapGroup;
    FieldEffectStart(sEffects[kind]);
}

// Called by mp_overworld.c when a remote player's emote counter changes
void MP_Emote_OnRemoteIcon(u8 id, struct ObjectEvent *obj, u8 kind)
{
    InitIfNeeded();
    if (kind != MP_EMOTE_NONE && id < MP_MAX_PLAYERS)
        sBubbles[1 + id].timer = 0;   // the icon replaces their bubble
    StartIcon(obj, kind);
    MP_Social_OnEmote(id, kind);
}

enum { SAY_TEXT, SAY_NOTE, SAY_BATTLE };
static void ShowBattleBubble(u8 slot, u8 battler, const u8 *text, u8 icon);   // SAY_BATTLE: u8 icon, then the text

static void SendSay(u8 kind, const u8 *text)
{
    u8 buf[2 + BUBBLE_TEXT];
    u16 len = 0;
    if (!MP_IsConnected())
        return;
    buf[0] = MPR_SAY;
    buf[1] = kind;
    if (text)
    {
        while (len < BUBBLE_TEXT - 1 && text[len] != EOS)
        {
            buf[2 + len] = text[len];
            len++;
        }
    }
    buf[2 + len] = EOS;
    MP_SendRelay(0xFF, buf, 3 + len);
}

void MP_Emote_OnRelay(u8 from, const u8 *data, u16 len)
{
    u8 text[BUBBLE_TEXT];
    u16 i;
    if (from >= MP_MAX_PLAYERS || data[0] != MPR_SAY || len < 3)
        return;
    for (i = 0; i + 2 < len && i < BUBBLE_TEXT - 1 && data[2 + i] != EOS; i++)
    {
        u8 c = data[2 + i];
        // Only plain characters and line breaks; no control codes
        text[i] = (c >= 0xF7 && c != CHAR_NEWLINE) ? CHAR_SPACE : c;
    }
    text[i] = EOS;
    if (data[1] == SAY_BATTLE)
    {
        // A bubble over their Moemon, if they're in this battle with us
        u8 battler = MP_BattleChat_BattlerOf(from), icon = len > 2 ? data[2] : 0;
        for (i = 0; i + 3 < len && i < BUBBLE_TEXT - 1 && data[3 + i] != EOS; i++)
        {
            u8 c = data[3 + i];
            text[i] = (c >= 0xF7 && c != CHAR_NEWLINE) ? CHAR_SPACE : c;
        }
        text[i] = EOS;
        MPNet_Log("battle emote from player %d over battler %d", from + 1, battler);
        if (battler < MAX_BATTLERS_COUNT)
            ShowBattleBubble(1 + from, battler, text, icon);
        return;
    }
    if (data[1] == SAY_NOTE)
    {
        u8 obj = MP_GetRemoteObjectEventId(from);
        sBubbles[1 + from].timer = 0;   // the icon replaces their bubble
        if (obj < OBJECT_EVENTS_COUNT)
            StartNoteIcon(&gObjectEvents[obj]);
        MP_Social_OnEmote(from, MP_EMOTE_NOTE);
    }
    else
    {
        ShowBubble(1 + from, text, FALSE);
        MP_Social_LogSay(from, text);
    }
}

// ---------------------------------------------------------------------------
// Battle bubbles (mp_battlechat.c): over the battler's Moemon

static void ShowBattleBubble(u8 slot, u8 battler, const u8 *text, u8 icon)
{
    ShowBubble(slot, text, icon);
    sBubbles[slot].battler = battler;
}

static void UpdateBattleBubble(struct Bubble *b)
{
    u8 spriteId = gBattlerSpriteIds[b->battler], half;
    s16 x, y;
    struct Sprite *os;

    if (b->timer == 0 || spriteId >= MAX_SPRITES || !gSprites[spriteId].inUse)
    {
        if (HalfIsOurs(b, 0) || HalfIsOurs(b, 1))
            DestroyBubbleSprites(b);
        if (b->timer)
            b->timer--;
        return;
    }
    b->timer--;
    if (!HalfIsOurs(b, 0) || !HalfIsOurs(b, 1))
    {
        DestroyBubbleSprites(b);
        if (!CreateBubbleSprites(b))
            return;
    }
    else if (b->dirty)
        RedrawBubble(b);
    os = &gSprites[spriteId];
    x = os->x + os->x2;
    y = os->y + os->y2 - 32 + 4;   // the tail at the top of the 64x64 picture
    // The tail stays over the speaker's Moemon; near a screen edge the box slides
    // back on screen instead (moving the whole bubble pointed it at the neighbor)
    if (x < 8)
        x = 8;
    if (x > DISPLAY_WIDTH - 8)
        x = DISPLAY_WIDTH - 8;
    if (y < 16)
        y = 16;
    if (b->boxW)
    {
        s16 half = b->boxW / 2, shift = 0;
        if (x - half < 2)
            shift = 2 - (x - half);
        else if (x + half > DISPLAY_WIDTH - 2)
            shift = (DISPLAY_WIDTH - 2) - (x + half);
        if (shift != b->boxShift)
        {
            b->boxShift = shift;
            RedrawBubble(b);
        }
    }
    for (half = 0; half < 2; half++)
    {
        struct Sprite *s = &gSprites[b->spriteId[half]];
        s->x = x - 32 + half * 64;
        s->y = y;
        s->coordOffsetEnabled = FALSE;
        s->invisible = FALSE;
        s->oam.priority = 0;
        s->subpriority = 2;   // behind the battle emote pick
    }
    if (b->timer == 0)
        DestroyBubbleSprites(b);
}

// Every frame in a battle
void MP_Emote_BattleUpdate(void)
{
    u8 i;
    bool8 any = FALSE;
    InitIfNeeded();
    for (i = 0; i < 1 + MP_MAX_PLAYERS; i++)
    {
        if (sBubbles[i].battler < MAX_BATTLERS_COUNT)
        {
            UpdateBattleBubble(&sBubbles[i]);
            any |= sBubbles[i].timer > 0;
        }
    }
    if (!any && !MP_BattleChat_IsOpen() && IndexOfSpritePaletteTag(TAG_BUBBLE_PAL) != 0xFF && !AnyBubbleShown())
        FreeSpritePaletteByTag(TAG_BUBBLE_PAL);
}

// This game's player says something in battle (text, or an icon with text NULL)
void MP_Emote_BattleSay(const u8 *text, u8 icon)
{
    u8 buf[2 + BUBBLE_TEXT];
    u16 len = 0;
    InitIfNeeded();
    ShowBattleBubble(0, MP_BattleChat_LocalBattler(), text, icon);
    if (gMpLocalId >= 0 && text)
        MP_Social_LogSay(gMpLocalId, text);
    if (!MP_IsConnected())
        return;
    buf[0] = MPR_SAY;
    buf[1] = SAY_BATTLE;
    buf[2] = icon;
    while (text && len < BUBBLE_TEXT - 2 && text[len] != EOS)
    {
        buf[3 + len] = text[len];
        len++;
    }
    buf[3 + len] = EOS;
    MP_SendRelay(0xFF, buf, 4 + len);
}

// A bubble over a player (combos, pranks)
void MP_Emote_ShowBubbleFor(u8 playerId, const u8 *text)
{
    if (playerId == gMpLocalId)
        ShowBubble(0, text, FALSE);
    else if (playerId < MP_MAX_PLAYERS)
        ShowBubble(1 + playerId, text, FALSE);
}

// Keeps a bubble up over a player while something lasts (re-shown before it
// runs out); MP_Emote_ClearBubbleFor takes it down
void MP_Emote_KeepBubbleFor(u8 playerId, const u8 *text)
{
    u8 slot = playerId == gMpLocalId ? 0 : 1 + playerId;
    InitIfNeeded();
    if (playerId >= MP_MAX_PLAYERS)
        return;
    if (sBubbles[slot].timer < 60 || StringCompare(sBubbles[slot].text, text) != 0)
        ShowBubble(slot, text, FALSE);
}

void MP_Emote_ClearBubbleFor(u8 playerId)
{
    u8 slot = playerId == gMpLocalId ? 0 : 1 + playerId;
    InitIfNeeded();
    if (playerId < MP_MAX_PLAYERS)
        sBubbles[slot].timer = 0;
}

static bool8 CanEmote(void)
{
    return MP_IsInOverworld() && !gMain.inBattle;
}

void MP_Emote_Icon(u8 kind)
{
    if (!CanEmote())
        return;
    MP_TriggerLocalEmote(kind);
    InitIfNeeded();
    sBubbles[0].timer = 0;
    StartIcon(&gObjectEvents[gPlayerAvatar.objectEventId], kind);
    if (gMpLocalId >= 0)
        MP_Social_OnEmote(gMpLocalId, kind);
}

void MP_Emote_Note(void)
{
    if (!CanEmote())
        return;
    InitIfNeeded();
    sBubbles[0].timer = 0;
    StartNoteIcon(&gObjectEvents[gPlayerAvatar.objectEventId]);
    SendSay(SAY_NOTE, NULL);
    if (gMpLocalId >= 0)
        MP_Social_OnEmote(gMpLocalId, MP_EMOTE_NOTE);
}

void MP_Emote_SayText(const u8 *text)
{
    if (!CanEmote() || text[0] == EOS)
        return;
    ShowBubble(0, text, FALSE);
    SendSay(SAY_TEXT, text);
    if (gMpLocalId >= 0)
        MP_Social_LogSay(gMpLocalId, text);
}

u8 MP_Emote_QuickPhraseCount(void)
{
    return ARRAY_COUNT(sQuickPhrases);
}

const char *MP_Emote_QuickPhrase(u8 i)
{
    return sQuickPhrases[i % ARRAY_COUNT(sQuickPhrases)];
}

void MP_Emote_SayQuick(u8 i)
{
    u8 text[BUBBLE_TEXT];
    MP_AsciiToGame(text, MP_Emote_QuickPhrase(i), BUBBLE_TEXT - 1);
    MP_Emote_SayText(text);
}

// ---------------------------------------------------------------------------
// Custom phrases (16 of them, 4 words each, made in the word editor)

bool8 MP_Emote_PhraseIsEmpty(u8 slot)
{
    u8 i;
    if (slot >= MP_NUM_PHRASES)
        return TRUE;
    for (i = 0; i < MP_PHRASE_WORDS; i++)
        if (gMpConfig.phrases[slot][i] != EC_EMPTY_WORD)
            return FALSE;
    return TRUE;
}

// The phrase as game text, two lines (words 1-2 and 3-4)
void MP_Emote_PhraseText(u8 slot, u8 *dst)
{
    u16 words[MP_PHRASE_WORDS];
    u8 i;
    dst[0] = EOS;
    if (MP_Emote_PhraseIsEmpty(slot))
        return;
    for (i = 0; i < MP_PHRASE_WORDS; i++)
        words[i] = gMpConfig.phrases[slot][i];
    ConvertEasyChatWordsToString(dst, words, 2, 2);
}

void MP_Emote_SayPhrase(u8 slot)
{
    u8 text[BUBBLE_TEXT * 2];
    if (MP_Emote_PhraseIsEmpty(slot))
    {
        PlaySE(SE_FAILURE);
        return;
    }
    MP_Emote_PhraseText(slot, text);
    text[BUBBLE_TEXT - 1] = EOS;
    MP_Emote_SayText(text);
}

static EWRAM_DATA u16 sEditWords[MP_PHRASE_WORDS] = {0};
static EWRAM_DATA u8 sEditSlot = 0;

static void Task_ReopenEmoteMenu(u8 taskId)
{
    if (IsWeatherNotFadingIn() == TRUE)
    {
        DestroyTask(taskId);
        MP_OpenQuickPickAt(MP_QUICK_PAGE_MINE + sEditSlot / 4, -1, sEditSlot % 4);   // back on the phrase you edited
    }
}

static void FieldCB_ReturnToEmoteMenu(void)
{
    LockPlayerFieldControls();
    FadeInFromBlack();
    CreateTask(Task_ReopenEmoteMenu, 80);
}

static void CB2_AfterPhraseEdit(void)
{
    u8 i;
    for (i = 0; i < MP_PHRASE_WORDS; i++)
        gMpConfig.phrases[sEditSlot][i] = sEditWords[i];
    MP_SaveConfig();
    MP_FieldClearVBlankHBlankCallbacks();
    gFieldCallback = FieldCB_ReturnToEmoteMenu;
    SetMainCallback2(CB2_ReturnToField);
}

// Opens the word editor for one phrase (the screen has already faded out)
void MP_Emote_EditPhrase(u8 slot)
{
    u8 i;
    sEditSlot = slot % MP_NUM_PHRASES;
    for (i = 0; i < MP_PHRASE_WORDS; i++)
        sEditWords[i] = gMpConfig.phrases[sEditSlot][i];
    CleanupOverworldWindowsAndTilemaps();
    DoEasyChatScreen(EASY_CHAT_TYPE_MP_PHRASE, sEditWords, CB2_AfterPhraseEdit, EASY_CHAT_PERSON_DISPLAY_NONE);
}

// From the quick pick (MINE page); the editor comes back to it
void MP_Emote_EditPhraseFromField(u8 slot)
{
    MP_Emote_EditPhrase(slot);
}

// Keys 1-8 and the right stick (see src/input_context.c)
void MP_Emote_Hotkey(u8 n)
{
    switch (n)
    {
    case 0: MP_Emote_Icon(MP_EMOTE_EXCLAMATION); break;
    case 1: MP_Emote_Icon(MP_EMOTE_QUESTION); break;
    case 2: MP_Emote_Icon(MP_EMOTE_HEART); break;
    case 3: MP_Emote_Note(); break;
    default:
        if (n - 4 < MP_NUM_PHRASES)
            MP_Emote_SayPhrase(n - 4);
        break;
    }
}

// ---------------------------------------------------------------------------
// Door labels: a friend's name over the door of the building they're in (or
// over the way out, when you're inside and they're outside).

static bool8 MapLeadsTo(const struct MapHeader *from, u8 group, u8 num)
{
    s32 i;
    if (from->connections)
        for (i = 0; i < from->connections->count; i++)
            if (from->connections->connections[i].mapGroup == group && from->connections->connections[i].mapNum == num)
                return TRUE;
    if (from->events)
        for (i = 0; i < from->events->warpCount; i++)
            if (from->events->warps[i].mapGroup == group && from->events->warps[i].mapNum == num)
                return TRUE;
    return FALSE;
}

// Finds the warp on the current map nearest to you that leads to (group, num),
// directly or through one more map
static bool8 FindDoorTo(u8 group, u8 num, s16 *outX, s16 *outY)
{
    const struct MapEvents *ev = gMapHeader.events;
    struct ObjectEvent *me = &gObjectEvents[gPlayerAvatar.objectEventId];
    s32 i, bestDist = 0x7FFFFFFF;
    bool8 found = FALSE;
    if (!ev)
        return FALSE;
    for (i = 0; i < ev->warpCount; i++)
    {
        const struct WarpEvent *w = &ev->warps[i];
        bool8 leads;
        s32 dx, dy;
        if (w->mapGroup == MAP_GROUP(MAP_DYNAMIC) && w->mapNum == MAP_NUM(MAP_DYNAMIC))
            continue;
        leads = (w->mapGroup == group && w->mapNum == num);
        if (!leads && MP_IsValidMap(w->mapGroup, w->mapNum))
            leads = MapLeadsTo(Overworld_GetMapHeaderByGroupAndId(w->mapGroup, w->mapNum), group, num);
        if (!leads)
            continue;
        dx = w->x + MAP_OFFSET - me->currentCoords.x;
        dy = w->y + MAP_OFFSET - me->currentCoords.y;
        if (dx * dx + dy * dy < bestDist)
        {
            bestDist = dx * dx + dy * dy;
            *outX = w->x + MAP_OFFSET;
            *outY = w->y + MAP_OFFSET;
            found = TRUE;
        }
    }
    return found;
}

static void UpdateDoorLabel(u8 id)
{
    struct Bubble *b = &sBubbles[LABEL_SLOT(id)];
    struct MpPlayer *p = &gMpPlayers[id];
    struct ObjectEvent *me = &gObjectEvents[gPlayerAvatar.objectEventId];
    s16 x, y, dx, dy;
    u8 name[PLAYER_NAME_LENGTH + 1];

    b->atTile = TRUE;
    b->timer = 0;
    if (id == gMpLocalId || !p->present || !p->hasInfo || !p->hasState
     || MP_GetRemoteObjectEventId(id) < OBJECT_EVENTS_COUNT       // you can see them already
     || !MP_IsValidMap(p->state.mapGroup, p->state.mapNum)
     || (p->state.mapGroup == gSaveBlock1Ptr->location.mapGroup && p->state.mapNum == gSaveBlock1Ptr->location.mapNum)
     || !FindDoorTo(p->state.mapGroup, p->state.mapNum, &x, &y))
        return;
    dx = x - me->currentCoords.x;
    dy = y - me->currentCoords.y;
    if (dx < -9 || dx > 9 || dy < -7 || dy > 7)
        return; // off screen
    StringCopyN(name, p->name, PLAYER_NAME_LENGTH);
    name[PLAYER_NAME_LENGTH] = EOS;
    // Two friends behind the same door: put the second one a line higher
    {
        u8 j;
        for (j = 0; j < id; j++)
        {
            struct Bubble *o = &sBubbles[LABEL_SLOT(j)];
            if (o->timer && o->tileX == x && o->tileY == y - 0)
                y--;
        }
    }
    if (b->tileX != x || b->tileY != y || StringCompare(b->text, name) != 0)
    {
        memset(b->text, EOS, sizeof(b->text));
        StringCopy(b->text, name);
        b->note = FALSE;
        b->dirty = TRUE;
    }
    b->tileX = x;
    b->tileY = y;
    b->timer = 2;   // kept alive every frame while it applies
}

// A ping's bubble over a tile (mp_ping.c keeps it alive every frame it applies)
void MP_Emote_PingBubble(u8 slot, const u8 *text, s16 tileX, s16 tileY)
{
    struct Bubble *b;
    InitIfNeeded();
    if (slot >= MP_MAX_PLAYERS)
        return;
    b = &sBubbles[PING_SLOT(slot)];
    b->atTile = TRUE;
    if (text == NULL)
    {
        b->timer = 0;
        return;
    }
    if (b->tileX != tileX || b->tileY != tileY || StringCompare(b->text, text) != 0)
    {
        memset(b->text, EOS, sizeof(b->text));
        StringCopyN(b->text, text, BUBBLE_TEXT - 1);
        b->note = FALSE;
        b->dirty = TRUE;
    }
    b->tileX = tileX;
    b->tileY = tileY;
    b->timer = 2;
}

// A short note at the top of the screen for a few seconds (game text, 2 lines max)
void MP_Toast(const u8 *text)
{
    MP_Social_LogToast(text);
    struct Bubble *b;
    InitIfNeeded();
    b = &sBubbles[TOAST_SLOT];
    memset(b->text, EOS, sizeof(b->text));
    StringCopyN(b->text, text, BUBBLE_TEXT - 1);
    b->note = FALSE;
    b->toast = TRUE;
    b->timer = TOAST_TIME;
    b->dirty = TRUE;
}

// Speed up on / off (typed "speed" / "slow", src/platform/sdl2.c)
static void ShowSpeedNotice(void)
{
    int Platform_TakeSpeedNotice(void);
    int n = Platform_TakeSpeedNotice();
    u8 text[64];
    if (!n)
        return;
    MP_AsciiToGame(text, n == 1 ? "Speed up ON (type SLOW to stop)" : "Normal speed", 60);
    MP_Toast(text);
}

// Every overworld frame, after the camera moved
void MP_Emote_Update(void)
{
    u8 i;
    InitIfNeeded();
    if (!MP_IsInOverworld())
        return;
    // Battle bubbles end with the battle
    for (i = 0; i < 1 + MP_MAX_PLAYERS; i++)
        if (sBubbles[i].battler != MAX_BATTLERS_COUNT)
        {
            sBubbles[i].battler = MAX_BATTLERS_COUNT;
            sBubbles[i].timer = 0;
        }
    ShowSpeedNotice();
    UpdateBubble(&sBubbles[TOAST_SLOT], gPlayerAvatar.objectEventId);
    MP_Ping_Update();
    MP_Lend_Update();
    for (i = 0; i < MP_MAX_PLAYERS; i++)
    {
        UpdateDoorLabel(i);
        UpdateBubble(&sBubbles[LABEL_SLOT(i)], gPlayerAvatar.objectEventId);
        UpdateBubble(&sBubbles[PING_SLOT(i)], gPlayerAvatar.objectEventId);
    }
    UpdateBubble(&sBubbles[0], gPlayerAvatar.objectEventId);
    for (i = 0; i < MP_MAX_PLAYERS; i++)
    {
        u8 obj = (i != gMpLocalId && gMpPlayers[i].present) ? MP_GetRemoteObjectEventId(i) : OBJECT_EVENTS_COUNT;
        UpdateBubble(&sBubbles[1 + i], obj);
    }
    if (!AnyBubbleShown() && !MP_Compass_AnyShown() && !MP_EmoteIconShown() && IndexOfSpritePaletteTag(TAG_BUBBLE_PAL) != 0xFF)
        FreeSpritePaletteByTag(TAG_BUBBLE_PAL);
}

// Hide the name tag while that player's bubble is up
bool8 MP_Emote_RemoteBubbleShown(u8 id)
{
    bool8 MP_EmoteIconOver(u8 localId, u8 mapNum, u8 mapGroup);
    u8 obj;
    InitIfNeeded();
    if (id >= MP_MAX_PLAYERS)
        return FALSE;
    if (sBubbles[1 + id].timer > 0)
        return TRUE;
    obj = MP_GetRemoteObjectEventId(id);
    return obj < OBJECT_EVENTS_COUNT
        && MP_EmoteIconOver(gObjectEvents[obj].localId, gObjectEvents[obj].mapNum, gObjectEvents[obj].mapGroup);
}

void MP_Emote_OnPlayerLeft(u8 id)
{
    InitIfNeeded();
    if (id < MP_MAX_PLAYERS)
    {
        sBubbles[1 + id].timer = 0;
        DestroyBubbleSprites(&sBubbles[1 + id]);
        sBubbles[LABEL_SLOT(id)].timer = 0;
        DestroyBubbleSprites(&sBubbles[LABEL_SLOT(id)]);
    }
}

// Moemon Co-op: the battle bubbles' sprites, which the Triple Battle cards (drawn over
// the finished frame) paint over again so the cards never hide them
u8 MP_Emote_BattleOverlaySprites(u8 *ids, u8 max)
{
    u8 i, half, n = 0;
    for (i = 0; i < NUM_BUBBLES; i++)
    {
        if (sBubbles[i].battler >= MAX_BATTLERS_COUNT || sBubbles[i].timer == 0)
            continue;
        for (half = 0; half < 2 && n < max; half++)
            if (sBubbles[i].spriteId[half] < MAX_SPRITES)
                ids[n++] = sBubbles[i].spriteId[half];
    }
    return n;
}
