// Moemon Co-op: the ball effect editor (from the summary's POKé BALL page, SELECT).
//
// A battle scene with the Moemon standing on its platform and a panel on the
// right: PRESET, MATERIALIZATION, ENERGY COLOR, ENTRANCE PARTICLES, RECALL
// EFFECT and SOUND ACCENT, then SAVE AS (MY PRESET 1-3, which PRESET then offers)
// and APPLY TO PARTY. The preview loops: the ball drops in and opens, the
// entrance plays, a short pause, then the recall. SELECT shows it from the
// other side (as other players see it), START saves, B leaves.
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "mp.h"
#include "bg.h"
#include "data.h"
#include "decompress.h"
#include "gpu_regs.h"
#include "graphics.h"
#include "main.h"
#include "malloc.h"
#include "menu.h"
#include "palette.h"
#include "pokeball.h"
#include "pokemon.h"
#include "scanline_effect.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "trainer_pokemon_sprites.h"
#include "trig.h"
#include "util.h"
#include "window.h"
#include "constants/items.h"
#include "constants/rgb.h"
#include "constants/songs.h"

u8 AnimateBallOpenParticles(u8 x, u8 y, u8 priority, u8 subpriority, u8 ballId);
u8 ItemIdToBallId(u16 ballItem);
extern const u16 gBallOpenFadeColors[];

enum { ROW_PRESET, ROW_FORM, ROW_COLOR, ROW_PART, ROW_RECALL, ROW_SOUND, ROW_SAVEAS, ROW_PARTY, ROW_COUNT };
#define ROW_SETTINGS ROW_SAVEAS   // rows before this have a value line under the label

enum { ASK_NONE, ASK_SAVE, ASK_REPLACE, ASK_PARTY };

#define WIN_PANEL 0
#define WIN_INFO  1
#define TEXT_PAL  15
#define PANEL_X   112   // the panel covers x 112-239

// Text palette: 1 white, 2 shadow, 3 grey, 4 panel, 5 selected band, 6 gold, 7 dim label, 8 edge
static const u16 sTextPal[16] = {
    0, RGB(31, 31, 31), RGB(2, 3, 7), RGB(20, 21, 25), RGB(4, 6, 12), RGB(10, 14, 26), RGB(31, 25, 6),
    RGB(15, 17, 23), RGB(16, 20, 30), 0, 0, 0, 0, 0, 0, 0,
};
enum { TC_WHITE = 1, TC_SHADOW, TC_GREY, TC_PANEL, TC_BAND, TC_GOLD, TC_DIM, TC_EDGE };

static const struct BgTemplate sBgs[] = {
    {.bg = 0, .charBaseIndex = 0, .mapBaseIndex = 31, .screenSize = 0, .paletteMode = 0, .priority = 0, .baseTile = 0},
    {.bg = 3, .charBaseIndex = 2, .mapBaseIndex = 26, .screenSize = 1, .paletteMode = 0, .priority = 3, .baseTile = 0},
};

static const struct WindowTemplate sWindows[] = {
    [WIN_PANEL] = {.bg = 0, .tilemapLeft = PANEL_X / 8, .tilemapTop = 0, .width = 16, .height = 20, .paletteNum = TEXT_PAL, .baseBlock = 1},
    [WIN_INFO]  = {.bg = 0, .tilemapLeft = 0, .tilemapTop = 14, .width = 14, .height = 6, .paletteNum = TEXT_PAL, .baseBlock = 1 + 320},
    DUMMY_WIN_TEMPLATE
};

enum { PH_DROP, PH_LAND, PH_OPEN, PH_HOLD, PH_RECALL, PH_REST };

struct Editor
{
    struct MpBallFx fx, orig;
    struct BoxPokemon *target;
    MainCallback exitCb;
    u8 row;
    bool8 front;      // as other players see it
    u8 asking;        // ASK_*: a YES / NO question in the info box
    u8 slot;          // SAVE AS: MY PRESET slot 0-2
    char note[64];    // a line for the info box ("Saved as MY PRESET 1.")
    bool8 leaving;
    u16 species;
    u32 personality, otId;
    u16 ballItem, ballStyle;
    u8 ballId;
    u16 monSprite;
    u8 ballSprite;
    u8 phase;
    u16 timer;
    s16 monX, monY, ballX, ballY;
    bool8 defaultForm, defaultRecall;
};

static EWRAM_DATA struct Editor *sEd = NULL;

static void CB2_Editor(void);
static void VBlankCB_Editor(void);
static void Task_Editor(u8 taskId);

// ---------------------------------------------------------------------------

static void PrintOn(u8 win, const char *ascii, u8 x, u8 y, u8 fg, u8 font)
{
    u8 text[96];
    u8 colors[3] = {0, fg, TC_SHADOW};
    MP_AsciiToGame(text, ascii, sizeof(text) - 1);
    AddTextPrinterParameterized4(win, font, x, y, 0, 0, colors, TEXT_SKIP_DRAW, text);
}

static void Print(const char *ascii, u8 x, u8 y, u8 fg, u8 font)
{
    PrintOn(WIN_PANEL, ascii, x, y, fg, font);
}

// Prints ASCII text wrapped to `width` pixels (small font), up to 3 lines
static void PrintWrapped(u8 win, const char *ascii, u8 x, u8 y, u16 width)
{
    char line[64], test[64];
    const char *p = ascii;
    u8 game[96], row = 0;
    line[0] = 0;
    while (*p && row < 3)
    {
        const char *w = p;
        u32 len;
        while (*p && *p != ' ')
            p++;
        len = p - w;
        snprintf(test, sizeof(test), "%s%s%.*s", line, line[0] ? " " : "", (int)len, w);
        MP_AsciiToGame(game, test, sizeof(game) - 1);
        if (line[0] && GetStringWidth(FONT_SMALL, game, 0) > width)
        {
            PrintOn(win, line, x, y + row * 13, TC_WHITE, FONT_SMALL);
            row++;
            snprintf(line, sizeof(line), "%.*s", (int)len, w);
        }
        else
        {
            snprintf(line, sizeof(line), "%s", test);
        }
        while (*p == ' ')
            p++;
    }
    if (line[0] && row < 3)
        PrintOn(win, line, x, y + row * 13, TC_WHITE, FONT_SMALL);
}

// What the selected choice looks like (in the box under the battle field, like a battle message)
static const char *const sFormInfo[MPFX_FORM_COUNT] = {
    "The game's own: a white flash as it grows.",
    "Energy pours from the ball, a glowing silhouette grows into it, then its colors.",
    "Its silhouette appears in a billowing cloud that clears.",
    "It forms inside a big floating bubble that pops.",
    "Its silhouette flickers with sparks, then it's solid.",
    "It fades in from nothing, wavering like a ghost.",
    "Pixels gather, it sharpens out of a mosaic and a scanline passes over it.",
    "It's painted on as an ink silhouette, then its colors come through.",
    "A crystal forms around it, then breaks into glittering fragments.",
    "It falls in a blazing streak and lands with a squash and a shockwave.",
    "It's oversized for a moment, then settles with a bounce.",
    "It rises from the ground.",
    "A beam of light strikes its spot and it materializes inside.",
    "It springs out of the ball, flips once and lands.",
    "A vortex opens and it spins out of it, slowing down.",
    "It zooms in from the side with speed lines and skids to a stop.",
    "It drops in as see-through jelly and wobbles into shape.",
    "A card in its energy color flips over three times and shows it.",
    "It's projected as a flickering, glitching hologram that firms up.",
    "Stars swirl in and gather into its shape, then it shines.",
    "Sand pours down and piles into its shape, then falls away.",
    "A pillar of fire erupts and it steps out of the flames.",
    "A geyser shoots it up, then it drops into place in a splash.",
    "A whirl of leaves wraps it, then blows away.",
    "Its shadow slides over from the ball and it rises out of it.",
    "It grows in as its colors run through the rainbow.",
    "It bounces out of the ball to its spot, three hops, each smaller.",
};
static const char *const sPartInfo[MPFX_PART_COUNT] = {
    "The game's own: the ball's sparkles.",
    "No particles at all.",
    "Two bursts of colorful paper. A few pieces flutter down after.",
    "A flash, an orange blast, embers and soot - and a little bounce.",
    "A puff of smoke rolls out around it.",
    "Rockets shoot up and burst into colored sparks.",
    "A flower opens beneath it, scattering petals.",
    "Bubbles rise around it and pop.",
    "A bolt strikes its spot with a flash and sparks.",
    "Wisps circle it and two eyes glow for a moment.",
    "Square pixels scatter and blink out.",
    "A splash of ink slaps down, droplets flying.",
    "Glittering crystal shards burst outward.",
    "The field dims and a spotlight shines on it.",
    "A dust ring and a small impact flash at its feet.",
    "Golden sparkles, glitter and a little crown.",
    "Hearts pop up around it and float away.",
    "Music notes rise on both sides, as if it were singing.",
    "Snow drifts down over it, ice glinting here and there.",
    "A flurry of white feathers, drifting down.",
    "A gust blows tumbling leaves past it.",
    "Shooting stars streak by, it twinkles, a big star spins in.",
    "A rainbow arcs over it from two little clouds.",
    "Flames spring up in a ring around its feet.",
    "A burst of water, a splash crown and flying drops.",
    "A storm of cherry petals blows across the field.",
    "Coins fly out, spinning, and bounce on the ground.",
    "A bunch of balloons floats up from behind it.",
    "Butterflies flutter out and away.",
    "Glittering gems burst out and bounce on the ground.",
    "A flaring aura rises around it in its energy color.",
};
static const char *const sRecallInfo[MPFX_RECALL_COUNT] = {
    "The game's own: it shrinks into the ball.",
    "It turns into a glowing silhouette and shrinks back into the ball.",
    "Its body breaks into glowing motes that stream toward the ball.",
    "It becomes paper that swirls back inside.",
    "A puff hides it; the smoke is sucked into the ball.",
    "It becomes a grid of squares that blink out.",
    "A quick bow, a sparkle, then back to the ball.",
};
static const char *const sSoundInfo[MPFX_SOUND_COUNT] = {
    "A sound that suits the effect.",
    "No extra sound.",
    "A soft chime.",
    "A pop.",
    "A boom.",
    "A short fanfare.",
    "A thunderclap.",
    "Something spooky.",
};

static u8 PartyMonCount(void)
{
    u8 i, n = 0;
    for (i = 0; i < PARTY_SIZE; i++)
        if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES) != SPECIES_NONE && !GetMonData(&gPlayerParty[i], MON_DATA_IS_EGG))
            n++;
    return n;
}

static void DrawInfo(void)
{
    const char *text;
    char buf[112];
    s8 preset;
    struct MpBallFx saved;
    FillWindowPixelBuffer(WIN_INFO, PIXEL_FILL(TC_PANEL));
    FillWindowPixelRect(WIN_INFO, PIXEL_FILL(TC_EDGE), 0, 0, 112, 1);
    if (sEd->asking != ASK_NONE)
    {
        if (sEd->asking == ASK_SAVE)
            text = "Save the changes?";
        else if (sEd->asking == ASK_REPLACE)
            snprintf(buf, sizeof(buf), "Replace MY PRESET %d with this mix?", sEd->slot + 1), text = buf;
        else
            snprintf(buf, sizeof(buf), "Give this mix to all %d Moemon in your party?", PartyMonCount()), text = buf;
        PrintWrapped(WIN_INFO, text, 4, 3, 104);
        PrintOn(WIN_INFO, "{A} YES   {B} NO", 4, 33, TC_GOLD, FONT_SMALL);
        PutWindowTilemap(WIN_INFO);
        CopyWindowToVram(WIN_INFO, COPYWIN_FULL);
        return;
    }
    if (sEd->note[0])
    {
        PrintWrapped(WIN_INFO, sEd->note, 4, 3, 104);
        PutWindowTilemap(WIN_INFO);
        CopyWindowToVram(WIN_INFO, COPYWIN_FULL);
        return;
    }
    switch (sEd->row)
    {
    case ROW_PRESET:
        preset = MP_BallFx_WhichPreset(&sEd->fx);
        if (preset < 0 && MP_BallFx_WhichUserPreset(&sEd->fx) >= 0)
            text = "One of your own presets. SAVE AS keeps up to three.";
        else
            text = preset < 0 ? "Your own mix. Pick a preset with {LEFTRIGHT}, or change any part below." :
                   preset == 0 ? "Everything the way the game does it." : "A ready-made mix. Change any part below to make it your own.";
        break;
    case ROW_SAVEAS:
        snprintf(buf, sizeof(buf), MP_BallFx_GetUserPreset(sEd->slot, &saved)
                 ? "{A} keeps this mix as MY PRESET %d, replacing the one there."
                 : "{A} keeps this mix as MY PRESET %d (empty) for any Moemon.", sEd->slot + 1);
        text = buf;
        break;
    case ROW_PARTY:
        text = "{A}: give this mix to every Moemon in your party.";
        break;
    case ROW_FORM:   text = sFormInfo[sEd->fx.form]; break;
    case ROW_COLOR:  text = "The color of its energy: beams, glows, motes, pixels and ink."; break;
    case ROW_PART:   text = sPartInfo[sEd->fx.part]; break;
    case ROW_RECALL: text = sRecallInfo[sEd->fx.recall]; break;
    default:         text = sSoundInfo[sEd->fx.sound]; break;
    }
    PrintWrapped(WIN_INFO, text, 4, 3, 104);
    PutWindowTilemap(WIN_INFO);
    CopyWindowToVram(WIN_INFO, COPYWIN_FULL);
}

static const char *RowLabel(u8 row)
{
    static const char *const sLabels[ROW_COUNT] = {
        "PRESET", "MATERIALIZATION", "ENERGY COLOR", "ENTRANCE PARTICLES", "RECALL EFFECT", "SOUND ACCENT",
        "SAVE AS", "APPLY TO PARTY",
    };
    return sLabels[row];
}

static void RowValue(u8 row, char *out, u32 size)
{
    s8 preset;
    switch (row)
    {
    case ROW_PRESET:
        (void)preset;
        MP_BallFx_EffectName(&sEd->fx, out, size);
        break;
    case ROW_SAVEAS:
    {
        struct MpBallFx saved;
        (void)saved;
        snprintf(out, size, "MY PRESET %d", sEd->slot + 1);
        break;
    }
    case ROW_PARTY:
        out[0] = 0;
        break;
    case ROW_FORM:   snprintf(out, size, "%s", MP_BallFx_FormName(sEd->fx.form)); break;
    case ROW_COLOR:
    {
        char c[16];
        MP_GameToAscii(c, MP_BallStyle_ColorName(sEd->fx.color), sizeof(c) - 1);
        snprintf(out, size, "%s", c);
        break;
    }
    case ROW_PART:   snprintf(out, size, "%s", MP_BallFx_PartName(sEd->fx.part)); break;
    case ROW_RECALL: snprintf(out, size, "%s", MP_BallFx_RecallName(sEd->fx.recall)); break;
    case ROW_SOUND:  snprintf(out, size, "%s", MP_BallFx_SoundName(sEd->fx.sound)); break;
    }
}

static void DrawPanel(void)
{
    u8 row;
    char line[48], value[32];

    FillWindowPixelBuffer(WIN_PANEL, PIXEL_FILL(TC_PANEL));
    FillWindowPixelRect(WIN_PANEL, PIXEL_FILL(TC_EDGE), 0, 0, 1, 160);
    Print("BALL EFFECTS", 6, 1, TC_GOLD, FONT_NORMAL);
    for (row = 0; row < ROW_COUNT; row++)
    {
        bool8 on = (row == sEd->row) && !sEd->asking;
        RowValue(row, value, sizeof(value));
        if (row < ROW_SETTINGS)
        {
            // label, and the value under it
            u8 y = 17 + row * 17;
            if (on)
                FillWindowPixelRect(WIN_PANEL, PIXEL_FILL(TC_BAND), 2, y - 1, 124, 17);
            Print(RowLabel(row), 6, y - 2, on ? TC_GREY : TC_DIM, FONT_SMALL);
            snprintf(line, sizeof(line), on ? "< %s >" : "%s", value);
            Print(line, on ? 6 : 12, y + 6, TC_WHITE, FONT_SMALL);
        }
        else
        {
            // one line: SAVE AS < MY PRESET 1 >, APPLY TO PARTY
            u8 y = 17 + ROW_SETTINGS * 17 + 2 + (row - ROW_SETTINGS) * 13;
            if (on)
                FillWindowPixelRect(WIN_PANEL, PIXEL_FILL(TC_BAND), 2, y - 1, 124, 13);
            Print(RowLabel(row), 6, y - 2, on ? TC_GOLD : TC_DIM, FONT_SMALL);
            if (value[0])
            {
                snprintf(line, sizeof(line), on ? "< %s >" : "%s", value);
                Print(line, 44, y - 2, TC_WHITE, FONT_SMALL);
            }
        }
    }
    snprintf(line, sizeof(line), "{SELECT} %s  {START} SAVE", sEd->front ? "BACK" : "FRONT");
    Print(line, 6, 146, TC_GREY, FONT_SMALL);
    PutWindowTilemap(WIN_PANEL);
    CopyWindowToVram(WIN_PANEL, COPYWIN_FULL);
    DrawInfo();
}

// ---------------------------------------------------------------------------
// The preview

static void DestroyPreview(void)
{
    MP_BallFx_StopAll();
    if (sEd->monSprite != 0xFFFF)
    {
        FreeAndDestroyMonPicSprite(sEd->monSprite);
        sEd->monSprite = 0xFFFF;
    }
    if (sEd->ballSprite < MAX_SPRITES)
    {
        DestroySprite(&gSprites[sEd->ballSprite]);
        sEd->ballSprite = MAX_SPRITES;
    }
}

static void SetMonScale(s32 scale)
{
    struct Sprite *s = &gSprites[sEd->monSprite];
    u8 n = s->oam.matrixNum;
    if (scale < 8)
        scale = 8;
    gOamMatrices[n].a = gOamMatrices[n].d = 0x10000 / scale;
    gOamMatrices[n].b = gOamMatrices[n].c = 0;
}

static void StartPreview(void)
{
    struct Sprite *mon;
    s16 yOff;

    DestroyPreview();
    sEd->defaultForm = sEd->fx.form == MPFX_FORM_DEFAULT;
    sEd->defaultRecall = sEd->fx.recall == MPFX_RECALL_DEFAULT;
    // standing on the player's platform, from the back (or from the front, as others see it)
    sEd->monX = 64;
    if (sEd->front)
        yOff = 40 + gMonFrontPicCoords[sEd->species].y_offset + 32;
    else
        yOff = 80 + gMonBackPicCoords[sEd->species].y_offset;
    sEd->monY = yOff;
    sEd->monSprite = CreateMonPicSprite_Affine(sEd->species, sEd->otId, sEd->personality,
                                               sEd->front ? MON_PIC_AFFINE_FRONT : MON_PIC_AFFINE_BACK,
                                               sEd->monX, sEd->monY, 1, TAG_NONE);
    if (sEd->monSprite == 0xFFFF)
        return;
    mon = &gSprites[sEd->monSprite];
    mon->oam.priority = 2;
    mon->invisible = TRUE;
    mon->callback = SpriteCallbackDummy;
    mon->affineAnimPaused = TRUE;
    mon->oam.affineMode = ST_OAM_AFFINE_DOUBLE;
    CalcCenterToCornerVec(mon, mon->oam.shape, mon->oam.size, mon->oam.affineMode);
    SetMonScale(256);

    // the ball drops in at its feet
    sEd->ballX = sEd->monX;
    sEd->ballY = sEd->monY + (sEd->front ? 24 : 28);
    LoadBallGfx(sEd->ballId);
    sEd->ballSprite = CreateSprite(&gBallSpriteTemplates[sEd->ballId], sEd->ballX, sEd->ballY - 60, 1);
    if (sEd->ballSprite < MAX_SPRITES)
    {
        gSprites[sEd->ballSprite].callback = SpriteCallbackDummy;
        gSprites[sEd->ballSprite].oam.priority = 2;
        MP_BallStyle_ApplyToBallSprite(sEd->ballSprite, sEd->ballId, sEd->ballStyle);
    }
    sEd->phase = PH_DROP;
    sEd->timer = 0;
}

// The game's own send-out / recall, for the parts left on DEFAULT
static void DefaultEntranceFrame(u16 t)
{
    u16 off = OBJ_PLTT_ID(gSprites[sEd->monSprite].oam.paletteNum);
    gSprites[sEd->monSprite].invisible = FALSE;
    SetMonScale(t < 13 ? 0x28 + 0x12 * t : 256);
    BlendPalette(off, 16, t < 16 ? 16 - t : 0, RGB_WHITE);
}

static void DefaultRecallFrame(u16 t)
{
    u16 off = OBJ_PLTT_ID(gSprites[sEd->monSprite].oam.paletteNum);
    BlendPalette(off, 16, t < 10 ? t * 16 / 10 : 16, gBallOpenFadeColors[sEd->ballId]);
    if (t >= 10)
        SetMonScale(0x10000 / (0x100 + (t - 10) * 0x30));
    if (t >= 21)
        gSprites[sEd->monSprite].invisible = TRUE;
}

static void PreviewFrame(void)
{
    struct Sprite *ball = sEd->ballSprite < MAX_SPRITES ? &gSprites[sEd->ballSprite] : NULL;
    u8 flags = sEd->front ? MPFX_FRONT : 0;

    if (sEd->monSprite == 0xFFFF)
        return;
    sEd->timer++;
    switch (sEd->phase)
    {
    case PH_DROP:
        if (ball)
            ball->y = sEd->ballY - 60 + 60 * sEd->timer * sEd->timer / 144;
        if (sEd->timer >= 12)
        {
            if (ball)
                ball->y = sEd->ballY;
            PlaySE(SE_BALL_BOUNCE_1);
            sEd->phase = PH_LAND;
            sEd->timer = 0;
        }
        break;
    case PH_LAND:
        if (ball)
            ball->y2 = sEd->timer < 6 ? -Sin(sEd->timer * 21, 5) : 0;
        if (sEd->timer >= 10)
        {
            sEd->phase = PH_OPEN;
            sEd->timer = 0;
            if (ball)
                StartSpriteAnim(ball, 1);
            if (sEd->fx.part == MPFX_PART_DEFAULT)
                AnimateBallOpenParticles(sEd->ballX, sEd->ballY - 5, 1, 28, sEd->ballId);
            else
                PlaySE(SE_BALL_OPEN);
            MP_BallFx_StartEntrance(sEd->monSprite, sEd->ballX, sEd->ballY - 5, flags, &sEd->fx);
            if (sEd->defaultForm)
                DefaultEntranceFrame(0);
        }
        break;
    case PH_OPEN:
        if (sEd->defaultForm)
            DefaultEntranceFrame(sEd->timer);
        if (ball && sEd->timer == 8)
            ball->invisible = TRUE;
        if (sEd->timer >= 16 && !MP_BallFx_BodyRunning(sEd->monSprite))
        {
            gSprites[sEd->monSprite].invisible = FALSE;
            sEd->phase = PH_HOLD;
            sEd->timer = 0;
        }
        break;
    case PH_HOLD:
        if (sEd->timer >= 56)   // a brief pause to look at it
        {
            sEd->phase = PH_RECALL;
            sEd->timer = 0;
            PlaySE(SE_BALL_OPEN);
            if (!sEd->defaultRecall)
                MP_BallFx_StartRecall(sEd->monSprite, sEd->monX - 56, sEd->monY + 24, flags, &sEd->fx);
            else
                DefaultRecallFrame(0);
        }
        break;
    case PH_RECALL:
        if (sEd->defaultRecall)
            DefaultRecallFrame(sEd->timer);
        if (sEd->timer >= 22 && !MP_BallFx_BodyRunning(sEd->monSprite))
        {
            gSprites[sEd->monSprite].invisible = TRUE;
            sEd->phase = PH_REST;
            sEd->timer = 0;
        }
        break;
    case PH_REST:
        if (sEd->timer >= 34 && !MP_BallFx_Active())
            StartPreview();
        else if (sEd->timer >= 90)
            StartPreview();
        break;
    }
}

// ---------------------------------------------------------------------------
// Input

static void ChangeValue(s8 d)
{
    struct MpBallFx *fx = &sEd->fx;
    switch (sEd->row)
    {
    case ROW_PRESET:
    {
        // the ready-made presets, then MY PRESET 1-3 (the saved ones)
        u8 builtIn = MP_BallFx_PresetCount(), total = builtIn + MPFX_USER_PRESETS, tries;
        struct MpBallFx user;
        s16 at = MP_BallFx_WhichPreset(fx);
        if (at < 0 && (at = MP_BallFx_WhichUserPreset(fx)) >= 0)
            at += builtIn;
        if (at < 0)
            at = d > 0 ? -1 : 0;
        for (tries = 0; tries < total; tries++)
        {
            at = (at + total + d) % total;
            if (at < builtIn)
            {
                MP_BallFx_Preset(at, fx);
                break;
            }
            if (MP_BallFx_GetUserPreset(at - builtIn, &user))
            {
                *fx = user;
                break;
            }
        }
        break;
    }
    case ROW_SAVEAS:
        sEd->slot = (sEd->slot + MPFX_USER_PRESETS + d) % MPFX_USER_PRESETS;
        PlaySE(SE_SELECT);
        DrawPanel();
        return;   // the preview keeps playing
    case ROW_PARTY:
        return;
    // only the pieces this save has (mp_ballunlock.c)
    case ROW_FORM:   fx->form = MP_BallUnlock_Step(MPFX_PIECE_FORM, fx->form, d); break;
    case ROW_COLOR:  fx->color = MP_BallUnlock_Step(MPFX_PIECE_COLOR, fx->color, d); break;
    case ROW_PART:   fx->part = MP_BallUnlock_Step(MPFX_PIECE_PART, fx->part, d); break;
    case ROW_RECALL: fx->recall = MP_BallUnlock_Step(MPFX_PIECE_RECALL, fx->recall, d); break;
    case ROW_SOUND:  fx->sound = (fx->sound + MPFX_SOUND_COUNT + d) % MPFX_SOUND_COUNT; break;
    }
    PlaySE(SE_SELECT);
    DrawPanel();
    StartPreview();
}

static void SavePreset(void)
{
    MP_BallFx_SaveUserPreset(sEd->slot, &sEd->fx);
    snprintf(sEd->note, sizeof(sEd->note), "Saved as MY PRESET %d. Pick it in PRESET for any Moemon.", sEd->slot + 1);
    MPNet_Log("ball fx: saved MY PRESET %d (%06x)", sEd->slot + 1, (unsigned)MP_BallFx_Pack(&sEd->fx));
    PlaySE(SE_SAVE);
}

// Every Moemon in the party gets this mix (the one being edited too, when it's in the party)
static void ApplyToParty(void)
{
    u8 i, n = 0;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES) == SPECIES_NONE || GetMonData(&gPlayerParty[i], MON_DATA_IS_EGG))
            continue;
        MP_BallFx_SetBoxMon(&gPlayerParty[i].box, &sEd->fx);
        if (&gPlayerParty[i].box == sEd->target)
            sEd->orig = sEd->fx;   // already saved: no "save the changes?" on the way out
        n++;
    }
    snprintf(sEd->note, sizeof(sEd->note), "Done! %d Moemon in your party have this mix now.", n);
    MPNet_Log("ball fx: applied to %d party Moemon", n);
    PlaySE(SE_EXP_MAX);
}

static void Leave(bool8 save)
{
    if (save)
    {
        MP_BallFx_SetBoxMon(sEd->target, &sEd->fx);
        PlaySE(SE_EXP_MAX);
    }
    else
    {
        PlaySE(SE_SELECT);
    }
    sEd->leaving = TRUE;
    BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
}

static void Task_Editor(u8 taskId)
{
    if (sEd->leaving)
    {
        if (!gPaletteFade.active)
        {
            MainCallback cb = sEd->exitCb;
            DestroyPreview();
            MP_BallFx_SetClip(0, 0, 0, 0);
            FreeAllWindowBuffers();
            ResetSpriteData();
            FreeAllSpritePalettes();
            Free(sEd);
            sEd = NULL;
            DestroyTask(taskId);
            SetMainCallback2(cb);
        }
        return;
    }
    PreviewFrame();
    if (gPaletteFade.active)
        return;
    if (sEd->asking == ASK_SAVE)
    {
        if (JOY_NEW(A_BUTTON))
            Leave(TRUE);
        else if (JOY_NEW(B_BUTTON))
            Leave(FALSE);
        return;
    }
    if (sEd->asking != ASK_NONE)
    {
        if (JOY_NEW(A_BUTTON))
        {
            if (sEd->asking == ASK_REPLACE)
                SavePreset();
            else
                ApplyToParty();
            sEd->asking = ASK_NONE;
            DrawPanel();
        }
        else if (JOY_NEW(B_BUTTON))
        {
            PlaySE(SE_SELECT);
            sEd->asking = ASK_NONE;
            DrawPanel();
        }
        return;
    }
    if (sEd->note[0] && (gMain.newKeys & (A_BUTTON | B_BUTTON | DPAD_ANY | START_BUTTON | SELECT_BUTTON)))
    {
        sEd->note[0] = 0;   // any key clears the "saved" line (and still does its own thing)
        DrawInfo();
    }
    if (JOY_NEW(DPAD_UP))
    {
        sEd->row = (sEd->row + ROW_COUNT - 1) % ROW_COUNT;
        PlaySE(SE_SELECT);
        DrawPanel();
    }
    else if (JOY_NEW(DPAD_DOWN))
    {
        sEd->row = (sEd->row + 1) % ROW_COUNT;
        PlaySE(SE_SELECT);
        DrawPanel();
    }
    else if (JOY_REPEAT(DPAD_LEFT))
        ChangeValue(-1);
    else if (JOY_REPEAT(DPAD_RIGHT))
        ChangeValue(1);
    else if (JOY_NEW(A_BUTTON) && sEd->row == ROW_SAVEAS)
    {
        struct MpBallFx saved;
        if (MP_BallFx_GetUserPreset(sEd->slot, &saved) && memcmp(&saved, &sEd->fx, sizeof(saved)) != 0)
        {
            sEd->asking = ASK_REPLACE;
            PlaySE(SE_SELECT);
            DrawPanel();
        }
        else
        {
            SavePreset();
            DrawPanel();
        }
    }
    else if (JOY_NEW(A_BUTTON) && sEd->row == ROW_PARTY)
    {
        sEd->asking = ASK_PARTY;
        PlaySE(SE_SELECT);
        DrawPanel();
    }
    else if (JOY_NEW(A_BUTTON))
    {
        PlaySE(SE_SELECT);
        StartPreview();
    }
    else if (JOY_NEW(SELECT_BUTTON))
    {
        sEd->front = !sEd->front;
        PlaySE(SE_SELECT);
        DrawPanel();
        StartPreview();
    }
    else if (JOY_NEW(START_BUTTON))
        Leave(TRUE);
    else if (JOY_NEW(B_BUTTON))
    {
        if (memcmp(&sEd->fx, &sEd->orig, sizeof(sEd->fx)) != 0)
        {
            sEd->asking = ASK_SAVE;
            PlaySE(SE_SELECT);
            DrawPanel();
        }
        else
        {
            Leave(FALSE);
        }
    }
}

// ---------------------------------------------------------------------------
// Setup

static void VBlankCB_Editor(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void CB2_Editor(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    RunTextPrinters();
    UpdatePaletteFade();
}

static void CB2_InitEditor(void)
{
    u8 i;

    SetVBlankCallback(NULL);
    SetGpuReg(REG_OFFSET_DISPCNT, 0);
    DmaClearLarge16(3, (void *)VRAM, VRAM_SIZE, 0x1000);
    DmaClear32(3, OAM, OAM_SIZE);
    DmaClear16(3, PLTT, PLTT_SIZE);
    ResetBgsAndClearDma3BusyFlags(0);
    InitBgsFromTemplates(0, sBgs, ARRAY_COUNT(sBgs));
    for (i = 0; i < 4; i++)
    {
        ChangeBgX(i, 0, BG_COORD_SET);
        ChangeBgY(i, 0, BG_COORD_SET);
    }
    InitWindows(sWindows);
    DeactivateAllTextPrinters();
    ResetPaletteFade();
    ScanlineEffect_Stop();
    ResetTasks();
    ResetSpriteData();
    FreeAllSpritePalettes();
    SetGpuReg(REG_OFFSET_BLDCNT, 0);
    SetGpuReg(REG_OFFSET_BLDALPHA, 0);
    SetGpuReg(REG_OFFSET_MOSAIC, 0);

    // a grassy battle field, the player's platform under the Moemon
    LZDecompressVram(gBattleEnvironmentTiles_TallGrass, (void *)BG_CHAR_ADDR(2));
    LZDecompressVram(gBattleEnvironmentTilemap_TallGrass, (void *)BG_SCREEN_ADDR(26));
    LoadCompressedPalette(gBattleEnvironmentPalette_TallGrass, BG_PLTT_ID(2), 3 * PLTT_SIZE_4BPP);
    LoadPalette(sTextPal, BG_PLTT_ID(TEXT_PAL), sizeof(sTextPal));
    LoadPalette(sTextPal, BG_PLTT_ID(0), PLTT_SIZEOF(1));

    DrawPanel();
    CopyBgTilemapBufferToVram(0);
    MP_BallFx_SetClip(0, 0, PANEL_X, 112);   // effects stay on the battle field
    StartPreview();

    SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
    ShowBg(0);
    ShowBg(3);
    BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
    SetVBlankCallback(VBlankCB_Editor);
    SetMainCallback2(CB2_Editor);
    CreateTask(Task_Editor, 0);
}

// Opens the editor for this Moemon; exitCb when it's closed
void MP_BallFxEditor_Open(struct BoxPokemon *target, MainCallback exitCb)
{
    sEd = AllocZeroed(sizeof(*sEd));
    if (sEd == NULL)
    {
        SetMainCallback2(exitCb);
        return;
    }
    sEd->target = target;
    sEd->exitCb = exitCb;
    sEd->monSprite = 0xFFFF;
    sEd->ballSprite = MAX_SPRITES;
    MP_BallFx_OfBoxMon(target, &sEd->fx);
    sEd->orig = sEd->fx;
    sEd->species = GetBoxMonData(target, MON_DATA_SPECIES);
    sEd->personality = GetBoxMonData(target, MON_DATA_PERSONALITY);
    sEd->otId = GetBoxMonData(target, MON_DATA_OT_ID);
    sEd->ballStyle = GetBoxMonData(target, MON_DATA_ENCRYPT_SEPARATOR);
    {
        struct Pokemon tmp;
        BoxMonToMon(target, &tmp);
        sEd->ballItem = MP_BallStyle_DisplayItem(&tmp);
    }
    sEd->ballId = ItemIdToBallId(MP_SpecialBall_FromItem(sEd->ballItem) ? ITEM_POKE_BALL : sEd->ballItem);
    SetMainCallback2(CB2_InitEditor);
}
