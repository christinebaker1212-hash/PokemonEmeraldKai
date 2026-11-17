// Moemon Co-op: the ball effect editor (from the summary's POKé BALL page, SELECT).
//
// A battle scene with the Moemon standing on its platform and a panel on the
// right: PRESET, MATERIALIZATION, ENERGY COLOR, ENTRANCE PARTICLES, RECALL
// EFFECT and SOUND ACCENT, then SAVE AS (MY PRESET 1-6, which PRESET then offers)
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

// A on a row opens every piece of its kind at once (MODE_LIST: a list, the colors as a grid; MODE_PRESETS)
enum { MODE_ROWS, MODE_LIST, MODE_PRESETS };
#define KIND_SOUND MPFX_PIECE_KINDS   // the five kinds in the list: MPFX_PIECE_* and the sound
#define LIST_KINDS (MPFX_PIECE_KINDS + 1)
enum { FILTER_ALL, FILTER_FAV, FILTER_NEW, FILTER_COUNT };
#define LIST_ROWS     9    // rows a list shows at once
#define LIST_Y        27   // the first row
#define LIST_H        13
#define GRID_COLS     8
#define PRESET_ROWS   5
#define PRESET_H      25
#define MAX_LIST      256

#define WIN_PANEL 0
#define WIN_INFO  1
#define TEXT_PAL  15
#define PANEL_X   112   // the panel covers x 112-239

// Text palette: 1 white, 2 shadow, 3 grey, 4 panel, 5 selected band, 6 gold, 7 dim label, 8 edge, 9 NEW,
// 10-14 the presets' color chips (set as the list is drawn)
static const u16 sTextPal[16] = {
    0, RGB(31, 31, 31), RGB(2, 3, 7), RGB(20, 21, 25), RGB(4, 6, 12), RGB(10, 14, 26), RGB(31, 25, 6),
    RGB(15, 17, 23), RGB(16, 20, 30), RGB(31, 12, 10), 0, 0, 0, 0, 0, 0,
};
enum { TC_WHITE = 1, TC_SHADOW, TC_GREY, TC_PANEL, TC_BAND, TC_GOLD, TC_DIM, TC_EDGE, TC_NEW, TC_CHIP0 };
// ENERGY COLOR's grid: each swatch in its own 2 x 2 tiles, whose palette is the text palette with seven colors
// in 9-15 (BG palettes 5-14: 70 colors)
#define SWATCH_PAL0    5
#define SWATCH_PALS    10
#define SWATCH_PER_PAL 7
#define TC_SWATCH0     9

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
    u8 slot;          // SAVE AS: MY PRESET slot 0-5
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
    // the list
    u8 mode;
    u8 filter;
    u16 cursor, top, shown;
    struct MpBallFx before;    // B in the list puts this back
    u8 items[MAX_LIST];        // the pieces shown (indexes), or presets: 0-5 MY PRESET, 6+ the ready-made ones
    bool8 owned[MAX_LIST];
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
    "Rocks pile into a boulder around it, then crack and fall away.",
    "Water pours down on its spot and it forms in the falling stream.",
    "Sparks circle and join into a crackling cage, then it's free.",
    "Petals close into a bud around it, and the bud blooms open.",
    "A doll stands in its place, then turns into it in a puff.",
    "An eye opens above it and psychic rings ripple it into being.",
    "The ground cracks, lava bursts up and it cools out of the glow.",
    "The ground shakes, dirt flies and it drops in with a thud.",
    "Birds of ice, thunder and fire swoop past and leave it there.",
    "Walls of light box it in, glow, then shatter.",
    "A pink light loops around playfully and bursts into it.",
    "A warp panel lights up and it's drawn in from its feet up.",
    "Rainbow flames rise in a ring and it steps out of them.",
    "A silver wind spirals down and it forms in the eye of it.",
    "Aurora light ripples down like a curtain and it appears.",
    "Green rings warp time around its spot; it fades in from the past.",
    "Glyphs gather and spell it out, then it fills in.",
    "A bell rings above; with each chime it grows clearer.",
    "A little tree wobbles on its spot, then it springs out.",
    "A dragon of light coils up and it appears at its head.",
    "Radio waves pulse, the static clears and it comes in clear.",
    "An APRICORN drops, splits open and it pops out.",
    "A milk bottle tips over and it forms in the white splash.",
    "Glowing bugs swarm in and light up its shape.",
    "The ground warps and it rises out of a dark, twisting hole.",
    "Space tears open in pink light and it steps through.",
    "Three lights rise from a ripple and become it.",
    "A crescent moon shines down and it forms in its light.",
    "Black mist swirls, red eyes glint, then it clears to show it.",
    "Seeds burst into flowers and it springs up among them.",
    "Rays of light rain down from above and it shines.",
    "Vines climb up around it and pull back to show it.",
    "Ghostly blue flames circle it, then fade and leave it there.",
    "Ice builds up into its shape, then melts away.",
    "Ribbons twirl around it and untie with a flourish.",
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
    "Tiny balls pop out and bounce all around it.",
    "Purple and pink sparkles burst, like a MASTER BALL opening.",
    "Rings of water ripple out from its feet.",
    "BERRIES tumble out and bounce around it.",
    "Paper streamers shoot out and curl down, as when a ship sails.",
    "Crackling electric orbs circle it and zap.",
    "APRICORNS of every color tumble out around it.",
    "Green crosses and soft sparkles rise around it.",
    "Rings of sound pulse out from it.",
    "Glinting scales swirl up around it.",
    "Glowing pollen drifts up in gentle spirals.",
    "Signal arcs pulse out, like a call coming in.",
    "Tiny ice glints sparkle in the cold air.",
    "Glittering spheres pop up out of the ground.",
    "Colored plates spin in a ring around it.",
    "Camera flashes pop from all around.",
    "Fluffy seeds float off on the breeze.",
    "Paper lanterns float up, glowing.",
    "Hit sparks flare all around, as in a clash.",
    "Contest seals pop out: hearts, stars and notes.",
};
static const char *const sRecallInfo[MPFX_RECALL_COUNT] = {
    "The game's own: it shrinks into the ball.",
    "It turns into a glowing silhouette and shrinks back into the ball.",
    "Its body breaks into glowing motes that stream toward the ball.",
    "It becomes paper that swirls back inside.",
    "A puff hides it; the smoke is sucked into the ball.",
    "It becomes a grid of squares that blink out.",
    "A quick bow, a sparkle, then back to the ball.",
    "A beam of light falls on it, it narrows to a line and is taken up.",
    "It crouches, then backflips into the ball.",
    "A vortex opens and it spins into it, faster and faster.",
    "It leans in and dashes off the screen with speed lines.",
    "It wobbles and melts into a puddle that slides back to the ball.",
    "It flips over to its card side and the card spins into the ball.",
    "It switches off like an old TV: a line, a dot, gone.",
    "It turns to starlight and streams into the ball as stars.",
    "It turns to sand and crumbles into a heap.",
    "It bursts into flame and is gone in a puff of smoke.",
    "A geyser bursts up under it and carries it away.",
    "Leaves whirl around it, then fly into the ball.",
    "It darkens and sinks into its shadow; the shadow slides away.",
    "Its colors run through the rainbow as it twists into a sparkle.",
    "It hops back to the ball, three hops, each bigger.",
    "Its heart beats twice, then it bursts into hearts.",
    "A bubble forms around it and floats it away, then pops.",
    "It crackles, a bolt strikes, and it's gone in the flash.",
    "Ice closes around it, it shakes, and it shatters.",
    "Three balloons lift it off the top of the screen.",
    "It crouches and leaps clean off the top of the screen.",
    "It hops, turns head-down and dives into the ground with a splash.",
    "It turns white and scatters into feathers.",
    "A black hole opens at the ball and pulls it in, stretching.",
    "It fades away like a ghost, drifting up.",
    "It pixelates, then its pixels rain down and fade.",
    "It shoots up like a rocket and bursts into fireworks.",
    "It dozes off, Zs floating up, and fades away asleep.",
    "It blushes pink and blows away in cherry petals.",
    "It pales and its spirit, a glowing orb, flies into the ball.",
    "Rocks pile up over it and sink into the ground.",
    "A wave rolls in and carries it off to the left.",
    "It turns into a ball of lightning and zips back to the ball.",
    "Sleep powder falls; it nods off and fades in drifting spores.",
    "It swaps with a doll, which flops over and vanishes.",
    "A spoon bends, it floats up and is drawn back to the ball.",
    "It turns to glowing embers that drift back to the ball.",
    "It digs down into the ground, dirt flying, and is gone.",
    "It takes off on wings and flies away to the upper left.",
    "It turns to glass, cracks, and the shards fly to the ball.",
    "It melts into a pink blob that bounces back to the ball.",
    "A white sheet falls over it, flutters and lies flat, empty.",
    "A silver gust lifts it, spinning, away to the left.",
    "A rainbow feather circles it and it rises on rainbow light.",
    "It blurs, afterimages trailing, and it's gone to the left.",
    "Green rings spin around it and it's pulled into the past.",
    "It breaks into glyphs that float off and fade.",
    "A bell tolls; with each toll it fades a little more.",
    "An eggshell closes around it and the EGG rolls to the ball.",
    "It coils into a dragon of light that flies up and away.",
    "It breaks up into static and the signal fades out.",
    "It flops and splashes, then hops away in a big arc.",
    "A net swoops over it and is pulled away to the left.",
    "A blizzard sweeps in and it's gone with the snow.",
    "It sinks into a rift in the ground that closes after it.",
    "A pink slash cuts the air and it falls through the tear.",
    "Mist rolls in, three lights circle and it's gone with them.",
    "It twirls in moonlight and becomes a crescent that sets.",
    "A dark hole spreads under it and it sinks in, asleep.",
    "Petals lift it up into the sky, flowers trailing.",
    "Golden rings lift it up into the light.",
    "Vines grab it and pull it back to the left.",
    "Blue flames circle it and it fades into one that drifts away.",
    "It is sealed in a snow globe that shrinks into the ball.",
    "The light flickers out; when it's back, it's gone.",
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

static void DrawListInfo(void);

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
    if (sEd->mode != MODE_ROWS)
    {
        DrawListInfo();
        return;
    }
    switch (sEd->row)
    {
    case ROW_PRESET:
        preset = MP_BallFx_WhichPreset(&sEd->fx);
        if (preset < 0 && MP_BallFx_WhichUserPreset(&sEd->fx) >= 0)
            text = "One of your own presets. SAVE AS keeps up to six.";
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

// A piece's description, as the editor's info box shows it (the phone app's POKé BALL page: mp_export.c)
const char *MP_BallFxEditor_Info(u8 kind, u8 i)
{
    switch (kind)
    {
    case MPFX_PIECE_FORM:   return i < MPFX_FORM_COUNT ? sFormInfo[i] : "";
    case MPFX_PIECE_PART:   return i < MPFX_PART_COUNT ? sPartInfo[i] : "";
    case MPFX_PIECE_RECALL: return i < MPFX_RECALL_COUNT ? sRecallInfo[i] : "";
    default:                return i < MPFX_SOUND_COUNT ? sSoundInfo[i] : "";
    }
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

// ---------------------------------------------------------------------------
// The five kinds a list goes round (L / R): MATERIALIZATION, ENERGY COLOR, ENTRANCE PARTICLES, RECALL EFFECT, SOUND
// ACCENT. Favourites and the pieces already looked at (no longer NEW) are kept with the save, 32 to a key
// ("fxfav:f:0", "fxseen:r:1").

static const u8 sRowKind[ROW_COUNT] = {
    [ROW_PRESET] = 0xFF, [ROW_FORM] = MPFX_PIECE_FORM, [ROW_COLOR] = MPFX_PIECE_COLOR, [ROW_PART] = MPFX_PIECE_PART,
    [ROW_RECALL] = MPFX_PIECE_RECALL, [ROW_SOUND] = KIND_SOUND, [ROW_SAVEAS] = 0xFF, [ROW_PARTY] = 0xFF,
};
static const char sKindLetter[LIST_KINDS] = {'c', 'f', 'p', 'r', 's'};

static u16 KindCount(u8 kind)
{
    return kind == KIND_SOUND ? MPFX_SOUND_COUNT : MP_BallUnlock_Count(kind);
}

static bool8 KindHas(u8 kind, u16 i)
{
    return kind == KIND_SOUND ? i < MPFX_SOUND_COUNT : MP_BallUnlock_Has(kind, i);
}

static u8 *KindField(struct MpBallFx *fx, u8 kind)
{
    switch (kind)
    {
    case MPFX_PIECE_COLOR:  return &fx->color;
    case MPFX_PIECE_FORM:   return &fx->form;
    case MPFX_PIECE_PART:   return &fx->part;
    case MPFX_PIECE_RECALL: return &fx->recall;
    default:                return &fx->sound;
    }
}

static const char *KindTitle(u8 kind)
{
    static const char *const sTitles[LIST_KINDS] = {"ENERGY COLOR", "MATERIALIZATION", "ENTRANCE PARTICLES",
                                                     "RECALL EFFECT", "SOUND ACCENT"};
    return sTitles[kind];
}

static const char *KindPieceName(u8 kind, u16 i, char *buf, u32 size)
{
    switch (kind)
    {
    case MPFX_PIECE_COLOR:  MP_GameToAscii(buf, MP_BallStyle_ColorName(i), size - 1); return buf;
    case MPFX_PIECE_FORM:   return MP_BallFx_FormName(i);
    case MPFX_PIECE_PART:   return MP_BallFx_PartName(i);
    case MPFX_PIECE_RECALL: return MP_BallFx_RecallName(i);
    default:                return MP_BallFx_SoundName(i);
    }
}

// The order a kind's dots and L / R go in: as the rows are
static const u8 sKindOrder[LIST_KINDS] = {MPFX_PIECE_FORM, MPFX_PIECE_COLOR, MPFX_PIECE_PART, MPFX_PIECE_RECALL, KIND_SOUND};

static u8 KindTab(u8 kind)
{
    u8 i;
    for (i = 0; i < LIST_KINDS; i++)
        if (sKindOrder[i] == kind)
            return i;
    return 0;
}

static u8 RowOfKind(u8 kind)
{
    u8 r;
    for (r = 0; r < ROW_COUNT; r++)
        if (sRowKind[r] == kind)
            return r;
    return ROW_FORM;
}

static bool8 FlagBit(const char *what, u8 kind, u16 i)
{
    char key[24];
    snprintf(key, sizeof(key), "%s:%c:%d", what, sKindLetter[kind], i / 32);
    return ((u32)MP_SaveData_Get(key) >> (i % 32)) & 1;
}

static void SetFlagBit(const char *what, u8 kind, u16 i, bool8 on)
{
    char key[24];
    u32 v;
    snprintf(key, sizeof(key), "%s:%c:%d", what, sKindLetter[kind], i / 32);
    v = (u32)MP_SaveData_Get(key);
    v = on ? (v | (1u << (i % 32))) : (v & ~(1u << (i % 32)));
    MP_SaveData_Set(key, (s32)v);
}

static bool8 IsFavorite(u8 kind, u16 i)
{
    return FlagBit("fxfav", kind, i);
}

// NEW: earned (not one of everyone's first pieces) and not yet looked at in a list
static bool8 IsNew(u8 kind, u16 i)
{
    return kind != KIND_SOUND && i >= MP_BallUnlock_First(kind) && KindHas(kind, i) && !FlagBit("fxseen", kind, i);
}

static u16 KindOwnedCount(u8 kind)
{
    u16 i, n = 0, count = KindCount(kind);
    for (i = 0; i < count; i++)
        n += KindHas(kind, i);
    return n;
}

// From cur, the next (dir 1) or previous (-1) favourite this save has; cur when there's none
static u16 NextFavorite(u8 kind, u16 cur, s8 dir)
{
    u16 count = KindCount(kind), i, v = cur;
    for (i = 0; i < count; i++)
    {
        v = (v + count + dir) % count;
        if (IsFavorite(kind, v) && KindHas(kind, v))
            return v;
    }
    return cur;
}

// Little pictures drawn into the panel ('#' = a pixel)
static const char *const sStar[] = {"...#...", "..###..", "#######", ".#####.", "..###..", ".##.##.", "##...##"};
static const char *const sLock[] = {"..###..", ".#...#.", ".#...#.", "#######", "###.###", "###.###", "#######"};
static const char *const sArrowL[] = {"..#", ".##", "###", ".##", "..#"};
static const char *const sArrowR[] = {"#..", "##.", "###", "##.", "#.."};

static void Glyph(const char *const *pic, u8 rows, u8 x, u8 y, u8 color)
{
    u8 r, c;
    for (r = 0; r < rows; r++)
        for (c = 0; pic[r][c]; c++)
            if (pic[r][c] == '#')
                FillWindowPixelRect(WIN_PANEL, PIXEL_FILL(color), x + c, y + r, 1, 1);
}
#define GLYPH(pic, x, y, color) Glyph(pic, ARRAY_COUNT(pic), x, y, color)

static u16 TextWidth(const char *ascii)
{
    u8 game[96];
    MP_AsciiToGame(game, ascii, sizeof(game) - 1);
    return GetStringWidth(FONT_SMALL, game, 0);
}

static u8 ListKind(void)
{
    return sRowKind[sEd->row];
}

// The pieces the list shows under its filter (presets: MY PRESET 1-6, then the ready-made ones)
static void BuildList(void)
{
    u16 i, count;
    u8 kind = ListKind();
    sEd->shown = 0;
    if (sEd->mode == MODE_PRESETS)
    {
        count = MPFX_USER_PRESETS + MP_BallFx_PresetCount();
        for (i = 0; i < count && i < MAX_LIST; i++)
        {
            sEd->items[sEd->shown] = i;
            sEd->owned[sEd->shown] = i < MPFX_USER_PRESETS || MP_BallFx_PresetUnlocked(i - MPFX_USER_PRESETS);
            sEd->shown++;
        }
        return;
    }
    count = KindCount(kind);
    for (i = 0; i < count && i < MAX_LIST; i++)
    {
        bool8 has = KindHas(kind, i);
        if (sEd->filter == FILTER_FAV && !(has && IsFavorite(kind, i)))
            continue;
        if (sEd->filter == FILTER_NEW && !IsNew(kind, i))
            continue;
        sEd->items[sEd->shown] = i;
        sEd->owned[sEd->shown] = has;
        sEd->shown++;
    }
}

static bool8 IsGrid(void)
{
    return sEd->mode == MODE_LIST && ListKind() == MPFX_PIECE_COLOR;
}

static void ListScroll(void)
{
    u16 vis = sEd->mode == MODE_PRESETS ? PRESET_ROWS : IsGrid() ? 7 * 8 : LIST_ROWS;
    u16 step = IsGrid() ? 8 : 1, cur = sEd->cursor - sEd->cursor % step;
    if (cur < sEd->top)
        sEd->top = cur;
    if (sEd->cursor >= sEd->top + vis)
        sEd->top = cur + step - vis;
    if (sEd->shown <= vis)
        sEd->top = 0;
    else if (sEd->top > (sEd->shown + step - 1) / step * step - vis)
        sEd->top = (sEd->shown + step - 1) / step * step - vis;
}

// Puts the cursor on the piece the mix has now (or on the nearest one shown)
static void ListCursorToCurrent(void)
{
    u16 i, want;
    sEd->cursor = 0;
    if (sEd->mode == MODE_PRESETS)
    {
        s8 user = MP_BallFx_WhichUserPreset(&sEd->fx), built = MP_BallFx_WhichPreset(&sEd->fx);
        want = user >= 0 ? (u16)user : built >= 0 ? MPFX_USER_PRESETS + built : 0;
    }
    else
    {
        want = *KindField(&sEd->fx, ListKind());
    }
    for (i = 0; i < sEd->shown; i++)
        if (sEd->items[i] >= want)
        {
            sEd->cursor = i;
            break;
        }
    ListScroll();
}

static void ScrollBar(u8 y0, u16 height, u16 vis)
{
    u16 h, y;
    if (sEd->shown <= vis)
        return;
    FillWindowPixelRect(WIN_PANEL, PIXEL_FILL(TC_BAND), 122, y0, 3, height);
    h = height * vis / sEd->shown;
    if (h < 6)
        h = 6;
    y = y0 + (height - h) * sEd->top / (sEd->shown - vis);
    FillWindowPixelRect(WIN_PANEL, PIXEL_FILL(TC_GREY), 122, y, 3, h);
}

static void ListHeader(u8 kind)
{
    u8 t;
    const char *title = KindTitle(kind);
    Print("{L}", 4, 0, TC_WHITE, FONT_SMALL);
    Print(title, 65 - TextWidth(title) / 2, 0, TC_GOLD, FONT_SMALL);
    Print("{R}", 116, 0, TC_WHITE, FONT_SMALL);
    for (t = 0; t < LIST_KINDS; t++)
        FillWindowPixelRect(WIN_PANEL, PIXEL_FILL(t == KindTab(kind) ? TC_GOLD : TC_DIM), 50 + t * 6, 22, 3, 2);
    FillWindowPixelRect(WIN_PANEL, PIXEL_FILL(TC_EDGE), 2, 25, 124, 1);
}

static void ListFooter(void)
{
    Print("{A}PICK {SELECT}", 6, 146, TC_GREY, FONT_SMALL);
    GLYPH(sStar, 6 + TextWidth("{A}PICK {SELECT}") + 1, 149, TC_GOLD);
    Print("{START}", 6 + TextWidth("{A}PICK {SELECT}") + 11, 146, TC_GREY, FONT_SMALL);
}

static const char *const sFilterNames[FILTER_COUNT] = {"SHOW ALL", "SHOW FAV", "SHOW NEW"};

static void DrawList(void)
{
    u8 kind = ListKind(), r;
    char line[48], buf[24];
    u16 have = 0, i;

    ListHeader(kind);
    for (i = 0; i < sEd->shown; i++)
        have += sEd->owned[i];
    snprintf(line, sizeof(line), "%d OF %d", have, sEd->shown);
    Print(line, 6, 12, TC_GREY, FONT_SMALL);
    Print(sFilterNames[sEd->filter], 124 - TextWidth(sFilterNames[sEd->filter]), 12, TC_GOLD, FONT_SMALL);
    if (sEd->shown == 0)
    {
        Print(sEd->filter == FILTER_FAV ? "No favorites yet:" : "Nothing new.", 8, LIST_Y + 2, TC_DIM, FONT_SMALL);
        if (sEd->filter == FILTER_FAV)
        {
            Print("{SELECT} on a piece marks it", 8, LIST_Y + 15, TC_DIM, FONT_SMALL);
            Print("as one.", 8, LIST_Y + 28, TC_DIM, FONT_SMALL);
        }
    }
    for (r = 0; r < LIST_ROWS && sEd->top + r < sEd->shown; r++)
    {
        u16 at = sEd->top + r, piece = sEd->items[at];
        u8 y = LIST_Y + r * LIST_H;
        if (at == sEd->cursor)
            FillWindowPixelRect(WIN_PANEL, PIXEL_FILL(TC_BAND), 2, y, 118, LIST_H);
        if (sEd->owned[at])
        {
            if (IsFavorite(kind, piece))
                GLYPH(sStar, 5, y + 3, TC_GOLD);
            Print(KindPieceName(kind, piece, buf, sizeof(buf)), 14, y - 1, TC_WHITE, FONT_SMALL);
            if (IsNew(kind, piece))
                Print("NEW", 119 - TextWidth("NEW"), y - 1, TC_NEW, FONT_SMALL);
        }
        else
        {
            GLYPH(sLock, 5, y + 3, TC_DIM);
            Print("- - - - -", 14, y - 1, TC_DIM, FONT_SMALL);
        }
    }
    ScrollBar(LIST_Y, LIST_ROWS * LIST_H, LIST_ROWS);
    ListFooter();
}

// ENERGY COLOR: every color at once, as swatches (a 16 x 16 cell each, 8 to a row, from y 32)
#define GRID_Y    32
#define GRID_ROWS 7
static void DrawColorGrid(void)
{
    u16 i, have = 0;
    char line[24], name[24];
    ListHeader(MPFX_PIECE_COLOR);
    if (sEd->shown)
    {
        u16 c = sEd->items[sEd->cursor];
        Print(sEd->owned[sEd->cursor] ? KindPieceName(MPFX_PIECE_COLOR, c, name, sizeof(name)) : "- - - - -", 6, 12,
              sEd->owned[sEd->cursor] ? TC_WHITE : TC_DIM, FONT_SMALL);
    }
    else
    {
        Print(sEd->filter == FILTER_FAV ? "No favorites yet." : "Nothing new.", 6, 12, TC_DIM, FONT_SMALL);
    }
    for (i = 0; i < sEd->shown; i++)
        have += sEd->owned[i];
    if (sEd->filter == FILTER_ALL)
        snprintf(line, sizeof(line), "%d OF %d", have, sEd->shown);
    else
        snprintf(line, sizeof(line), "%s", sFilterNames[sEd->filter]);
    Print(line, 124 - TextWidth(line), 12, sEd->filter == FILTER_ALL ? TC_GREY : TC_GOLD, FONT_SMALL);
    for (i = sEd->top; i < sEd->shown && i < sEd->top + GRID_COLS * GRID_ROWS; i++)
    {
        u8 x = ((i - sEd->top) % GRID_COLS) * 16, y = GRID_Y + ((i - sEd->top) / GRID_COLS) * 16;
        u16 c = sEd->items[i];
        if (sEd->owned[i])
        {
            FillWindowPixelRect(WIN_PANEL, PIXEL_FILL(TC_SHADOW), x + 3, y + 3, 10, 10);
            FillWindowPixelRect(WIN_PANEL, PIXEL_FILL(TC_SWATCH0 + c % SWATCH_PER_PAL), x + 4, y + 4, 8, 8);
            if (IsFavorite(MPFX_PIECE_COLOR, c))
                GLYPH(sStar, x + 5, y + 5, TC_WHITE);
        }
        else
        {
            FillWindowPixelRect(WIN_PANEL, PIXEL_FILL(TC_BAND), x + 3, y + 3, 10, 10);
            GLYPH(sLock, x + 5, y + 5, TC_DIM);
        }
        if (i == sEd->cursor)
        {
            FillWindowPixelRect(WIN_PANEL, PIXEL_FILL(TC_GOLD), x + 1, y + 1, 14, 1);
            FillWindowPixelRect(WIN_PANEL, PIXEL_FILL(TC_GOLD), x + 1, y + 14, 14, 1);
            FillWindowPixelRect(WIN_PANEL, PIXEL_FILL(TC_GOLD), x + 1, y + 1, 1, 14);
            FillWindowPixelRect(WIN_PANEL, PIXEL_FILL(TC_GOLD), x + 14, y + 1, 1, 14);
        }
    }
    {
        u16 vis = GRID_COLS * GRID_ROWS;
        if (sEd->shown > vis)
        {
            u16 rows = (sEd->shown + GRID_COLS - 1) / GRID_COLS;
            u16 h = GRID_ROWS * 16 * GRID_ROWS / rows;
            u16 y = GRID_Y + (GRID_ROWS * 16 - h) * (sEd->top / GRID_COLS) / (rows - GRID_ROWS);
            FillWindowPixelRect(WIN_PANEL, PIXEL_FILL(TC_BAND), 125, GRID_Y, 2, GRID_ROWS * 16);
            FillWindowPixelRect(WIN_PANEL, PIXEL_FILL(TC_GREY), 125, y, 2, h);
        }
    }
    ListFooter();
}

// The grid's swatches take their colors' palettes (after PutWindowTilemap)
static void ColorGridTilemap(void)
{
    u16 *map = GetBgTilemapBuffer(0);
    u16 i;
    for (i = sEd->top; i < sEd->shown && i < sEd->top + GRID_COLS * GRID_ROWS; i++)
    {
        u8 tx = PANEL_X / 8 + ((i - sEd->top) % GRID_COLS) * 2, ty = GRID_Y / 8 + ((i - sEd->top) / GRID_COLS) * 2, d;
        u16 pal = SWATCH_PAL0 + sEd->items[i] / SWATCH_PER_PAL;
        if (pal >= SWATCH_PAL0 + SWATCH_PALS)
            continue;
        for (d = 0; d < 4; d++)
        {
            u16 *e = &map[(ty + d / 2) * 32 + tx + d % 2];
            *e = (*e & 0x0FFF) | (pal << 12);
        }
    }
}

static void LoadSwatchPalettes(void)
{
    u16 pal[16];
    u8 p, k;
    for (p = 0; p < SWATCH_PALS; p++)
    {
        memcpy(pal, sTextPal, sizeof(pal));
        for (k = 0; k < SWATCH_PER_PAL; k++)
        {
            u16 c = p * SWATCH_PER_PAL + k;
            pal[TC_SWATCH0 + k] = c < MP_BALL_COLORS ? MP_BallFx_EnergyRgb(c) : 0;
        }
        LoadPalette(pal, BG_PLTT_ID(SWATCH_PAL0 + p), sizeof(pal));
    }
}

// PRESET: every preset with what's in it
static void DrawPresets(void)
{
    u8 r;
    struct MpBallFx fx;
    char name[24];
    Print("PRESETS", 6, 1, TC_GOLD, FONT_NORMAL);
    FillWindowPixelRect(WIN_PANEL, PIXEL_FILL(TC_EDGE), 2, 15, 124, 1);
    for (r = 0; r < PRESET_ROWS && sEd->top + r < sEd->shown; r++)
    {
        u16 at = sEd->top + r, p = sEd->items[at];
        u8 y = 17 + r * PRESET_H;
        bool8 filled;
        bool8 on = at == sEd->cursor;
        if (on)
            FillWindowPixelRect(WIN_PANEL, PIXEL_FILL(TC_BAND), 2, y, 118, PRESET_H);
        if (p < MPFX_USER_PRESETS)
        {
            filled = MP_BallFx_GetUserPreset(p, &fx);
            snprintf(name, sizeof(name), "MY PRESET %d", p + 1);
        }
        else
        {
            filled = TRUE;
            MP_BallFx_Preset(p - MPFX_USER_PRESETS, &fx);
            snprintf(name, sizeof(name), "%s", MP_BallFx_PresetName(p - MPFX_USER_PRESETS));
        }
        if (!sEd->owned[at])
        {
            GLYPH(sLock, 5, y + 3, TC_DIM);
            Print(name, 14, y - 1, TC_DIM, FONT_SMALL);
            Print("- - - - -", 14, y + 10, TC_DIM, FONT_SMALL);
            continue;
        }
        if (filled)
        {
            u16 chip = MP_BallFx_EnergyRgb(fx.color);
            LoadPalette(&chip, BG_PLTT_ID(TEXT_PAL) + TC_CHIP0 + r, sizeof(chip));
            FillWindowPixelRect(WIN_PANEL, PIXEL_FILL(TC_SHADOW), 5, y + 3, 6, 6);
            FillWindowPixelRect(WIN_PANEL, PIXEL_FILL(TC_CHIP0 + r), 6, y + 4, 4, 4);
        }
        Print(name, 14, y - 1, on ? TC_GOLD : TC_WHITE, FONT_SMALL);
        Print(filled ? MP_BallFx_FormName(fx.form) : "- empty -", 14, y + 10, filled ? TC_GREY : TC_DIM, FONT_SMALL);
    }
    ScrollBar(17, PRESET_ROWS * PRESET_H, PRESET_ROWS);
    Print("{A}USE", 6, 146, TC_GREY, FONT_SMALL);
    if (sEd->shown && sEd->items[sEd->cursor] < MPFX_USER_PRESETS)
        Print("{START}SAVE HERE", 6 + TextWidth("{A}USE") + 5, 146, TC_GREY, FONT_SMALL);
}

static void DrawRows(void)
{
    u8 row;
    char line[48], value[32];

    Print("BALL EFFECTS", 6, 1, TC_GOLD, FONT_NORMAL);
    for (row = 0; row < ROW_COUNT; row++)
    {
        bool8 on = (row == sEd->row) && !sEd->asking;
        u8 kind = sRowKind[row];
        RowValue(row, value, sizeof(value));
        if (row < ROW_SETTINGS)
        {
            // label (with how many of its kind this save has), and the value under it
            u8 y = 17 + row * 17, x;
            if (on)
                FillWindowPixelRect(WIN_PANEL, PIXEL_FILL(TC_BAND), 2, y - 1, 124, 17);
            Print(RowLabel(row), 6, y - 2, on ? TC_GREY : TC_DIM, FONT_SMALL);
            if (kind != 0xFF)
            {
                snprintf(line, sizeof(line), "%d/%d", KindOwnedCount(kind), KindCount(kind));
                Print(line, 124 - TextWidth(line), y - 2, on ? TC_GREY : TC_DIM, FONT_SMALL);
            }
            Print(value, 12, y + 6, TC_WHITE, FONT_SMALL);
            x = 12 + TextWidth(value);
            if (on)
            {
                GLYPH(sArrowL, 6, y + 10, TC_WHITE);
                GLYPH(sArrowR, x + 2, y + 10, TC_WHITE);
                x += 6;
            }
            if (kind != 0xFF && IsFavorite(kind, *KindField(&sEd->fx, kind)))
                GLYPH(sStar, x + 3, y + 9, TC_GOLD);
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
    Print(sEd->row < ROW_SETTINGS ? "{A}LIST {L}{R}FAVORITE" : "{SELECT}FRONT {START}SAVE", 6, 146, TC_GREY, FONT_SMALL);
}

static void DrawPanel(void)
{
    FillWindowPixelBuffer(WIN_PANEL, PIXEL_FILL(TC_PANEL));
    FillWindowPixelRect(WIN_PANEL, PIXEL_FILL(TC_EDGE), 0, 0, 1, 160);
    if (sEd->mode == MODE_PRESETS)
        DrawPresets();
    else if (IsGrid())
        DrawColorGrid();
    else if (sEd->mode == MODE_LIST)
        DrawList();
    else
        DrawRows();
    PutWindowTilemap(WIN_PANEL);
    if (IsGrid())
        ColorGridTilemap();
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

bool8 MP_FxClip_Active(void);
bool8 MP_FxClip_Next(struct MpBallFx *fx);

static void StartPreview(void)
{
    struct Sprite *mon;
    s16 yOff;

    DestroyPreview();
    if (MP_FxClip_Active())   // (recording for the phone app: mp_fxclip.c sets the next piece)
    {
        if (!MP_FxClip_Next(&sEd->fx))
            return;
        sEd->front = TRUE;
    }
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
        StartBallSpin(&gSprites[sEd->ballSprite]);
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
            {
                ball->y = sEd->ballY;
                StartSpriteAnim(ball, 0);   // landed: done spinning
            }
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
        if (sEd->timer >= (MP_FxClip_Active() ? 30 : 56))   // a brief pause to look at it
        {
            sEd->phase = PH_RECALL;
            sEd->timer = 0;
            PlaySE(SE_BALL_OPEN);
            if (!sEd->defaultRecall)
                MP_BallFx_StartRecall(sEd->monSprite, sEd->monX - 56,   // always to the left, as in battle
                                      
                                      sEd->front ? sEd->monY - 24 : sEd->monY + 24, flags, &sEd->fx);   // as in battle
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
        // the ready-made presets (those whose pieces this save has), then MY PRESET 1-6 (the saved ones)
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
                if (!MP_BallFx_PresetUnlocked(at))
                    continue;
                MP_BallFx_Preset(at, fx);
                break;
            }
            if (MP_BallFx_GetUserPreset(at - builtIn, &user))
            {
                *fx = user;
                break;
            }
        }
        {
            char name[32];
            MP_BallFx_EffectName(fx, name, sizeof(name));
            MPNet_Log("ball fx: preset %s", name);
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

// ---------------------------------------------------------------------------
// The list: what's under the cursor plays in the preview; A keeps it, B puts back what was there

static void DrawListInfo(void)
{
    char buf[112];
    const char *text;
    u16 piece;
    if (sEd->shown == 0)
    {
        text = sEd->filter == FILTER_FAV ? "{START} shows them all again." : "Nothing new to see: {START} shows them all.";
    }
    else if (sEd->mode == MODE_PRESETS)
    {
        piece = sEd->items[sEd->cursor];
        if (!sEd->owned[sEd->cursor])
            text = "Not yet yours: earn its parts first.";
        else if (piece < MPFX_USER_PRESETS)
        {
            struct MpBallFx saved;
            text = MP_BallFx_GetUserPreset(piece, &saved) ? "Your own mix. {START} keeps the one you had here instead."
                                                          : "Empty. {START} keeps the mix you had here.";
        }
        else
            text = piece == MPFX_USER_PRESETS ? "Everything the way the game does it." : "A ready-made mix.";
    }
    else
    {
        u8 kind = ListKind();
        piece = sEd->items[sEd->cursor];
        if (!sEd->owned[sEd->cursor])
        {
            char how[80];
            MP_BallUnlock_HowTo(kind, piece, how, sizeof(how));
            snprintf(buf, sizeof(buf), "Not yet earned. %s", how);
            text = buf;
        }
        else switch (kind)
        {
        case MPFX_PIECE_FORM:   text = sFormInfo[piece]; break;
        case MPFX_PIECE_PART:   text = sPartInfo[piece]; break;
        case MPFX_PIECE_RECALL: text = sRecallInfo[piece]; break;
        case KIND_SOUND:        text = sSoundInfo[piece]; break;
        default:                text = "The color of its energy: beams, glows, motes, pixels and ink."; break;
        }
    }
    PrintWrapped(WIN_INFO, text, 4, 3, 104);
    PutWindowTilemap(WIN_INFO);
    CopyWindowToVram(WIN_INFO, COPYWIN_FULL);
}

// The mix with what's under the cursor in it, playing in the preview (a piece not yet earned isn't shown)
static void ListHover(void)
{
    u16 piece;
    if (sEd->shown == 0 || !sEd->owned[sEd->cursor])
        return;
    piece = sEd->items[sEd->cursor];
    if (sEd->mode == MODE_PRESETS)
    {
        struct MpBallFx fx;
        if (piece < MPFX_USER_PRESETS)
        {
            if (!MP_BallFx_GetUserPreset(piece, &fx))
                fx = sEd->before;
        }
        else
            MP_BallFx_Preset(piece - MPFX_USER_PRESETS, &fx);
        if (memcmp(&fx, &sEd->fx, sizeof(fx)) == 0)
            return;
        sEd->fx = fx;
    }
    else
    {
        u8 kind = ListKind();
        if (IsNew(kind, piece))
            SetFlagBit("fxseen", kind, piece, TRUE);   // looked at: no longer NEW (the list keeps showing it till it's left)
        if (*KindField(&sEd->fx, kind) == piece)
            return;
        *KindField(&sEd->fx, kind) = piece;
    }
    StartPreview();
}

static void OpenList(void)
{
    sEd->before = sEd->fx;
    sEd->mode = sEd->row == ROW_PRESET ? MODE_PRESETS : MODE_LIST;
    if (sEd->mode == MODE_PRESETS)
        sEd->filter = FILTER_ALL;
    sEd->top = 0;
    BuildList();
    ListCursorToCurrent();
    PlaySE(SE_SELECT);
    DrawPanel();
}

static void CloseList(bool8 keep)
{
    if (!keep)
    {
        if (memcmp(&sEd->fx, &sEd->before, sizeof(sEd->fx)) != 0)
        {
            sEd->fx = sEd->before;
            StartPreview();
        }
    }
    sEd->mode = MODE_ROWS;
    PlaySE(SE_SELECT);
    DrawPanel();
}

static void ListMove(s16 d)
{
    s16 c = sEd->cursor;
    if (sEd->shown == 0)
        return;
    if (d == 1 || d == -1 || IsGrid())
        c = (c + sEd->shown * 8 + d) % sEd->shown;   // round and round
    else
    {
        c += d;   // a page: stops at the ends
        if (c < 0)
            c = 0;
        if (c >= sEd->shown)
            c = sEd->shown - 1;
    }
    if (c == sEd->cursor)
        return;
    sEd->cursor = c;
    ListScroll();
    PlaySE(SE_SELECT);
    ListHover();
    DrawPanel();
}

// L / R: the kind before / after (the mix as it is now is kept: B still puts back what was there before the list)
static void ListKindStep(s8 d)
{
    u8 tab = (KindTab(ListKind()) + LIST_KINDS + d) % LIST_KINDS;
    sEd->row = RowOfKind(sKindOrder[tab]);
    sEd->top = 0;
    BuildList();
    ListCursorToCurrent();
    PlaySE(SE_SELECT);
    DrawPanel();
}

static void Task_List(void)
{
    if (JOY_REPEAT(DPAD_UP))
        ListMove(IsGrid() ? -GRID_COLS : -1);
    else if (JOY_REPEAT(DPAD_DOWN))
        ListMove(IsGrid() ? GRID_COLS : 1);
    else if (JOY_REPEAT(DPAD_LEFT))
        ListMove(IsGrid() ? -1 : -(sEd->mode == MODE_PRESETS ? PRESET_ROWS : LIST_ROWS));
    else if (JOY_REPEAT(DPAD_RIGHT))
        ListMove(IsGrid() ? 1 : (sEd->mode == MODE_PRESETS ? PRESET_ROWS : LIST_ROWS));
    else if (JOY_NEW(L_BUTTON) && sEd->mode == MODE_LIST)
        ListKindStep(-1);
    else if (JOY_NEW(R_BUTTON) && sEd->mode == MODE_LIST)
        ListKindStep(1);
    else if (JOY_NEW(A_BUTTON))
    {
        if (sEd->shown && !sEd->owned[sEd->cursor])
        {
            PlaySE(SE_FAILURE);
            return;
        }
        CloseList(TRUE);
    }
    else if (JOY_NEW(B_BUTTON))
        CloseList(FALSE);
    else if (JOY_NEW(SELECT_BUTTON) && sEd->mode == MODE_LIST && sEd->shown)
    {
        u8 kind = ListKind();
        u16 piece = sEd->items[sEd->cursor];
        if (!sEd->owned[sEd->cursor])
        {
            PlaySE(SE_FAILURE);
            return;
        }
        SetFlagBit("fxfav", kind, piece, !IsFavorite(kind, piece));
        PlaySE(SE_SELECT);
        DrawPanel();
    }
    else if (JOY_NEW(START_BUTTON) && sEd->mode == MODE_LIST)
    {
        // ALL / FAV / NEW: the cursor stays on the same piece where it can
        u16 piece = sEd->shown ? sEd->items[sEd->cursor] : 0, i;
        sEd->filter = (sEd->filter + 1) % FILTER_COUNT;
        BuildList();
        sEd->cursor = 0;
        for (i = 0; i < sEd->shown; i++)
            if (sEd->items[i] >= piece)
            {
                sEd->cursor = i;
                break;
            }
        sEd->top = 0;
        ListScroll();
        PlaySE(SE_SELECT);
        ListHover();
        DrawPanel();
    }
    else if (JOY_NEW(START_BUTTON) && sEd->mode == MODE_PRESETS && sEd->shown && sEd->items[sEd->cursor] < MPFX_USER_PRESETS)
    {
        // keeps the mix from before the list in this slot
        u8 slot = sEd->items[sEd->cursor];
        sEd->slot = slot;
        sEd->fx = sEd->before;
        MP_BallFx_SaveUserPreset(slot, &sEd->fx);
        PlaySE(SE_SAVE);
        StartPreview();
        DrawPanel();
    }
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
        char name[24];
        void Sinnoh_TvBallEffect(struct BoxPokemon *box, const char *effectName);
        MP_BallFx_SetBoxMon(sEd->target, &sEd->fx);
        PlaySE(SE_EXP_MAX);
        MP_BallFx_EffectName(&sEd->fx, name, sizeof(name));
        Sinnoh_TvBallEffect(sEd->target, name);   // Moemon Kai: SINNOH's TV's SEAL CLUB SHOW (src/sinnoh/s3d_tv.c)
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
    if (sEd->note[0] && (gMain.newKeys & (A_BUTTON | B_BUTTON | DPAD_ANY | START_BUTTON | SELECT_BUTTON | L_BUTTON | R_BUTTON)))
    {
        sEd->note[0] = 0;   // any key clears the "saved" line (and still does its own thing)
        DrawInfo();
    }
    if (sEd->mode != MODE_ROWS)
    {
        Task_List();
        return;
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
        OpenList();
    else if ((JOY_NEW(L_BUTTON) || JOY_NEW(R_BUTTON)) && sRowKind[sEd->row] != 0xFF)
    {
        // the favourite before / after this one
        u8 kind = sRowKind[sEd->row], *field = KindField(&sEd->fx, kind);
        u16 next = NextFavorite(kind, *field, JOY_NEW(L_BUTTON) ? -1 : 1);
        if (next == *field)
        {
            PlaySE(SE_FAILURE);
        }
        else
        {
            *field = next;
            PlaySE(SE_SELECT);
            DrawPanel();
            StartPreview();
        }
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
    LoadSwatchPalettes();

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
