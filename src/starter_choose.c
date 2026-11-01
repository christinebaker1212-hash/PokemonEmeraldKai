#include "global.h"
#include "bg.h"
#include "data.h"
#include "decompress.h"
#include "event_data.h"
#include "gpu_regs.h"
#include "international_string_util.h"
#include "main.h"
#include "menu.h"
#include "palette.h"
#include "pokedex.h"
#include "pokemon.h"
#include "scanline_effect.h"
#include "sound.h"
#include "sprite.h"
#include "starter_choose.h"
#include "strings.h"
#include "task.h"
#include "text.h"
#include "text_window.h"
#include "trainer_pokemon_sprites.h"
#include "trig.h"
#include "window.h"
#include "constants/songs.h"
#include "constants/rgb.h"

#define STARTER_MON_COUNT   3

// Position of the sprite of the selected starter Pokémon
#define STARTER_PKMN_POS_X (DISPLAY_WIDTH / 2)
#define STARTER_PKMN_POS_Y 64

#define TAG_POKEBALL_SELECT 0x1000
#define TAG_STARTER_CIRCLE  0x1001

static void CB2_StarterChoose(void);
static void ClearStarterLabel(void);
static void Task_StarterChoose(u8 taskId);
static void Task_HandleStarterChooseInput(u8 taskId);
static void Task_WaitForStarterSprite(u8 taskId);
static void Task_AskConfirmStarter(u8 taskId);
static void Task_HandleConfirmStarterInput(u8 taskId);
static void Task_DeclineStarter(u8 taskId);
static void Task_LeaveTable(u8 taskId);
static void Task_MoveStarterChooseCursor(u8 taskId);
static void Task_CreateStarterLabel(u8 taskId);
static void CreateStarterPokemonLabel(u8 selection);
static u8 CreatePokemonFrontSprite(u16 species, u8 x, u8 y);
static void SpriteCB_SelectionHand(struct Sprite *sprite);
static void SpriteCB_Pokeball(struct Sprite *sprite);
static void SpriteCB_StarterPokemon(struct Sprite *sprite);

static u16 sStarterLabelWindowId;

// Moemon Kai: the KANTO / JOHTO starter table (the user's mock-up, docs/user_images/57.png): the same screen with the
// lab's table drawn in full color under the layers (MP_StarterTable_Composite), the balls in a row, the region's
// three POKéMON. MP_StarterTable_Open starts it; the result is the ball's index (0-2) in gSpecialVar_Result (0xFFFF:
// SINNOH's briefcase put down with B). The table can't be left without a POKéMON (1.63.11: B stepped back from it
// and the player could walk off).
#include "mp/mp_startable_gfx.h"
static bool8 sTableMode;
static u16 sTableSpecies[STARTER_MON_COUNT];
static const u8 sText_TableChoose[] = _("Which MOéMON will you choose?");
// Moemon Kai: SINNOH's briefcase (Platinum's starter screen, drawn ahead of time from its own models and animations:
// tools/sinnoh/build_starter.py, s3d_gfx.c S3D_Gfx_Starter): the case opens, the camera comes down on it, Platinum's
// glove points at the balls and the one under it rocks; the rest is the table mode's (circle, cry, YES / NO).
#include "s3d.h"
static bool8 sBriefcase;
static u8 sChooseTaskId;
static u16 sBcFrame;     // the picture shown: the opening's frames, then the camera's
static u16 sBcBounce;    // the ball under the glove: its rocking, counted in frames
static bool8 sBcChoosing;
static const u8 sText_BriefcaseChoose[] = _("Which MOéMON will you choose?\nB: close the briefcase");
static void Task_BriefcaseIntro(u8 taskId);
void MP_StarterTable_Open(const u16 *species, MainCallback back);
#define TABLE_HAND_Y 26
#define TABLE_LABEL_TOP 10

const u16 gBirchBagGrass_Pal[] = INCGFX_U16("graphics/starter_choose/tiles.png", ".gbapal");
static const u16 sPokeballSelection_Pal[] = INCGFX_U16("graphics/starter_choose/pokeball_selection.png", ".gbapal");
static const u16 sStarterCircle_Pal[] = INCGFX_U16("graphics/starter_choose/starter_circle.png", ".gbapal");
const u32 gBirchBagTilemap[] = INCGFX_U32("graphics/starter_choose/birch_bag.bin", ".lz");
const u32 gBirchGrassTilemap[] = INCGFX_U32("graphics/starter_choose/birch_grass.bin", ".lz");
const u32 gBirchBagGrass_Gfx[] = INCGFX_U32("graphics/starter_choose/tiles.png", ".4bpp.lz");
const u32 gPokeballSelection_Gfx[] = INCGFX_U32("graphics/starter_choose/pokeball_selection.png", ".4bpp.lz");
static const u32 sStarterCircle_Gfx[] = INCGFX_U32("graphics/starter_choose/starter_circle.png", ".4bpp.lz");

static const struct WindowTemplate sWindowTemplates[] =
{
    {
        .bg = 0,
        .tilemapLeft = 3,
        .tilemapTop = 15,
        .width = 24,
        .height = 4,
        .paletteNum = 14,
        .baseBlock = 0x0200
    },
    DUMMY_WIN_TEMPLATE,
};

static const struct WindowTemplate sWindowTemplate_ConfirmStarter =
{
    .bg = 0,
    .tilemapLeft = 24,
    .tilemapTop = 9,
    .width = 5,
    .height = 4,
    .paletteNum = 14,
    .baseBlock = 0x0260
};

static const struct WindowTemplate sWindowTemplate_StarterLabel =
{
    .bg = 0,
    .tilemapLeft = 0,
    .tilemapTop = 0,
    .width = 13,
    .height = 4,
    .paletteNum = 14,
    .baseBlock = 0x0274
};

static const u8 sPokeballCoords[STARTER_MON_COUNT][2] =
{
    {60, 64},
    {120, 88},
    {180, 64},
};

static const u8 sStarterLabelCoords[STARTER_MON_COUNT][2] =
{
    {0, 9},
    {16, 10},
    {8, 4},
};

static const u16 sStarterMon[STARTER_MON_COUNT] =
{
    SPECIES_TREECKO,
    SPECIES_TORCHIC,
    SPECIES_MUDKIP,
};

static const struct BgTemplate sBgTemplates[3] =
{
    {
        .bg = 0,
        .charBaseIndex = 2,
        .mapBaseIndex = 31,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 0,
        .baseTile = 0
    },
    {
        .bg = 2,
        .charBaseIndex = 0,
        .mapBaseIndex = 7,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 3,
        .baseTile = 0
    },
    {
        .bg = 3,
        .charBaseIndex = 0,
        .mapBaseIndex = 6,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 1,
        .baseTile = 0
    },
};

static const u8 sTextColors[] = {TEXT_COLOR_TRANSPARENT, TEXT_COLOR_WHITE, TEXT_COLOR_LIGHT_GRAY};

static const struct OamData sOam_Hand =
{
    .y = DISPLAY_HEIGHT,
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .mosaic = FALSE,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(32x32),
    .x = 0,
    .matrixNum = 0,
    .size = SPRITE_SIZE(32x32),
    .tileNum = 0,
    .priority = 1,
    .paletteNum = 0,
    .affineParam = 0,
};

static const struct OamData sOam_Pokeball =
{
    .y = DISPLAY_HEIGHT,
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .mosaic = FALSE,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(32x32),
    .x = 0,
    .matrixNum = 0,
    .size = SPRITE_SIZE(32x32),
    .tileNum = 0,
    .priority = 1,
    .paletteNum = 0,
    .affineParam = 0,
};

static const struct OamData sOam_StarterCircle =
{
    .y = DISPLAY_HEIGHT,
    .affineMode = ST_OAM_AFFINE_DOUBLE,
    .objMode = ST_OAM_OBJ_NORMAL,
    .mosaic = FALSE,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(64x64),
    .x = 0,
    .matrixNum = 0,
    .size = SPRITE_SIZE(64x64),
    .tileNum = 0,
    .priority = 1,
    .paletteNum = 0,
    .affineParam = 0,
};

static const u8 sCursorCoords[][2] =
{
    {60, 32},
    {120, 56},
    {180, 32},
};

static const union AnimCmd sAnim_Hand[] =
{
    ANIMCMD_FRAME(48, 30),
    ANIMCMD_END,
};

static const union AnimCmd sAnim_Pokeball_Still[] =
{
    ANIMCMD_FRAME(0, 30),
    ANIMCMD_END,
};

static const union AnimCmd sAnim_Pokeball_Moving[] =
{
    ANIMCMD_FRAME(16, 4),
    ANIMCMD_FRAME(0, 4),
    ANIMCMD_FRAME(32, 4),
    ANIMCMD_FRAME(0, 4),
    ANIMCMD_FRAME(16, 4),
    ANIMCMD_FRAME(0, 4),
    ANIMCMD_FRAME(32, 4),
    ANIMCMD_FRAME(0, 4),
    ANIMCMD_FRAME(0, 32),
    ANIMCMD_FRAME(16, 8),
    ANIMCMD_FRAME(0, 8),
    ANIMCMD_FRAME(32, 8),
    ANIMCMD_FRAME(0, 8),
    ANIMCMD_FRAME(16, 8),
    ANIMCMD_FRAME(0, 8),
    ANIMCMD_FRAME(32, 8),
    ANIMCMD_FRAME(0, 8),
    ANIMCMD_JUMP(0),
};

static const union AnimCmd sAnim_StarterCircle[] =
{
    ANIMCMD_FRAME(0, 8),
    ANIMCMD_END,
};

static const union AnimCmd *const sAnims_Hand[] =
{
    sAnim_Hand,
};

static const union AnimCmd *const sAnims_Pokeball[] =
{
    sAnim_Pokeball_Still,
    sAnim_Pokeball_Moving,
};

static const union AnimCmd *const sAnims_StarterCircle[] =
{
    sAnim_StarterCircle,
};

static const union AffineAnimCmd sAffineAnim_StarterPokemon[] =
{
    AFFINEANIMCMD_FRAME(16, 16, 0, 0),
    AFFINEANIMCMD_FRAME(16, 16, 0, 15),
    AFFINEANIMCMD_END,
};

static const union AffineAnimCmd sAffineAnim_StarterCircle[] =
{
    AFFINEANIMCMD_FRAME(20, 20, 0, 0),
    AFFINEANIMCMD_FRAME(20, 20, 0, 15),
    AFFINEANIMCMD_END,
};

static const union AffineAnimCmd *const sAffineAnims_StarterPokemon = {sAffineAnim_StarterPokemon};
static const union AffineAnimCmd *const sAffineAnims_StarterCircle[] = {sAffineAnim_StarterCircle};

static const struct CompressedSpriteSheet sSpriteSheet_PokeballSelect[] =
{
    {
        .data = gPokeballSelection_Gfx,
        .size = 0x0800,
        .tag = TAG_POKEBALL_SELECT
    },
    {}
};

static const struct CompressedSpriteSheet sSpriteSheet_StarterCircle[] =
{
    {
        .data = sStarterCircle_Gfx,
        .size = 0x0800,
        .tag = TAG_STARTER_CIRCLE
    },
    {}
};

static const struct SpritePalette sSpritePalettes_StarterChoose[] =
{
    {
        .data = sPokeballSelection_Pal,
        .tag = TAG_POKEBALL_SELECT
    },
    {
        .data = sStarterCircle_Pal,
        .tag = TAG_STARTER_CIRCLE
    },
    {},
};

static const struct SpriteTemplate sSpriteTemplate_Hand =
{
    .tileTag = TAG_POKEBALL_SELECT,
    .paletteTag = TAG_POKEBALL_SELECT,
    .oam = &sOam_Hand,
    .anims = sAnims_Hand,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCB_SelectionHand
};

static const struct SpriteTemplate sSpriteTemplate_Pokeball =
{
    .tileTag = TAG_POKEBALL_SELECT,
    .paletteTag = TAG_POKEBALL_SELECT,
    .oam = &sOam_Pokeball,
    .anims = sAnims_Pokeball,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCB_Pokeball
};

static const struct SpriteTemplate sSpriteTemplate_StarterCircle =
{
    .tileTag = TAG_STARTER_CIRCLE,
    .paletteTag = TAG_STARTER_CIRCLE,
    .oam = &sOam_StarterCircle,
    .anims = sAnims_StarterCircle,
    .images = NULL,
    .affineAnims = sAffineAnims_StarterCircle,
    .callback = SpriteCB_StarterPokemon
};

// .text
static u16 StarterSpecies(u16 selection)
{
    extern u16 GetStarterPokemon(u16 chosenStarterId);
    if (sTableMode)
        return sTableSpecies[selection < STARTER_MON_COUNT ? selection : 0];
    return GetStarterPokemon(selection);
}

static u8 BallX(u8 selection)
{
    const struct S3dStarterGfx *g = sBriefcase ? S3D_Gfx_Starter() : NULL;
    if (g != NULL)
        return g->ball[selection].x + g->ball[selection].w / 2;
    return sTableMode ? sStarterTableBallX[selection] : sPokeballCoords[selection][0];
}

static u8 BallY(u8 selection)
{
    const struct S3dStarterGfx *g = sBriefcase ? S3D_Gfx_Starter() : NULL;
    if (g != NULL)
        return g->ball[selection].y + g->ball[selection].h / 2;
    return sTableMode ? STARTER_TABLE_BALL_Y : sPokeballCoords[selection][1];
}

// SINNOH's briefcase: the same as the table, with Platinum's picture (FALSE: the pack has none, nothing opened)
bool8 MP_StarterBriefcase_Open(const u16 *species, MainCallback back)
{
    if (S3D_Gfx_Starter() == NULL)
        return FALSE;
    sBriefcase = TRUE;
    sBcFrame = 0;
    sBcBounce = 0;
    sBcChoosing = FALSE;
    MP_StarterTable_Open(species, back);
    return TRUE;
}

// The briefcase's picture under the layers (from MP_StarterTable_Composite): this frame's, the ball under the glove
// rocking and the glove over it while choosing
static void BriefcaseComposite(u16 *frame, s32 coeff)
{
    const struct S3dStarterGfx *g = S3D_Gfx_Starter();
    const u16 *pic;
    s32 x, y, sel = -1, bx = 0, by = 0, bw = 0, bh = 0, cx = 0, cy = 0;
    const u16 *patch = NULL;
    if (g == NULL)
        return;
    pic = g->frames + (u32)g->w * g->h * (sBcFrame < g->openFrames + g->camFrames ? sBcFrame : g->openFrames + g->camFrames - 1);
    if (sBcChoosing)
    {
        sel = gTasks[sChooseTaskId].data[0];   // (tStarterSelection)
        if (sel > 2)
            sel = 2;
        bx = g->ball[sel].x, by = g->ball[sel].y, bw = g->ball[sel].w, bh = g->ball[sel].h;
        patch = g->ball[sel].px + (u32)bw * bh * ((sBcBounce / 2) % g->bounceFrames);
        cx = g->cursorPos[sel][0] - 16;
        cy = g->cursorPos[sel][1] - 16 + (Sin((sBcBounce * 4) & 0xFF, 3));
    }
    for (y = 0; y < DISPLAY_HEIGHT && y < g->h; y++)
    {
        u16 *row = &frame[y * DISPLAY_WIDTH];
        for (x = 0; x < DISPLAY_WIDTH && x < g->w; x++)
        {
            u16 c;
            s32 r, gr, b;
            if (row[x] & 0x8000)
                continue;
            c = pic[y * g->w + x];
            if (patch != NULL && x >= bx && x < bx + bw && y >= by && y < by + bh)
                c = patch[(y - by) * bw + x - bx];
            if (sel >= 0 && x >= cx && x < cx + 32 && y >= cy && y < cy + 32 && (g->cursor[(y - cy) * 32 + x - cx] & 0x8000))
                c = g->cursor[(y - cy) * 32 + x - cx];
            r = c & 0x1F, gr = (c >> 5) & 0x1F, b = (c >> 10) & 0x1F;
            if (coeff)
            {
                r += (-r * coeff) >> 4;
                gr += (-gr * coeff) >> 4;
                b += (-b * coeff) >> 4;
            }
            row[x] = r | (gr << 5) | (b << 10) | 0x8000;
        }
    }
}

void MP_StarterTable_Open(const u16 *species, MainCallback back)
{
    u8 i;
    for (i = 0; i < STARTER_MON_COUNT; i++)
        sTableSpecies[i] = species[i];
    sTableMode = TRUE;
    gMain.savedCallback = back;
    SetMainCallback2(CB2_ChooseStarter);
}

bool8 MP_StarterTable_Active(void)   // (tests: harness condition "startertable": choosing, no fade)
{
    return sTableMode && gMain.callback2 != NULL && FuncIsActiveTask(Task_HandleStarterChooseInput) && !gPaletteFade.active;
}

// The table under everything drawn (sdl2.c, as the BAG's picture): the backdrop is white, its faded color says how
// far the fade to black has gone; inside the label's window it is darkened as the GBA's blend does there (BLDY 7).
void MP_StarterTable_Composite(u16 *frame)
{
    s8 fadeOf[32];
    u16 bd = 0x7FFF, winH, winV;
    int i, x, y, coeff;
    if (!sTableMode || gMain.callback2 != CB2_StarterChoose)
        return;
    for (i = 0; i < DISPLAY_WIDTH * DISPLAY_HEIGHT; i++)
        if (!(frame[i] & 0x8000))
        {
            bd = frame[i];
            break;
        }
    memset(fadeOf, -1, sizeof(fadeOf));
    for (coeff = 16; coeff >= 0; coeff--)
        fadeOf[31 + ((-31 * coeff) >> 4)] = coeff;
    coeff = fadeOf[bd & 0x1F];
    if (coeff < 0)
        coeff = 0;
    if (sBriefcase)
    {
        BriefcaseComposite(frame, coeff);
        return;
    }
    winH = GetGpuReg(REG_OFFSET_WIN0H);
    winV = GetGpuReg(REG_OFFSET_WIN0V);
    for (y = 0; y < DISPLAY_HEIGHT; y++)
    {
        u16 *row = &frame[y * DISPLAY_WIDTH];
        bool8 inV = y >= (winV >> 8) && y < (winV & 0xFF);
        for (x = 0; x < DISPLAY_WIDTH; x++)
        {
            u16 c;
            s32 r, g, b;
            if (row[x] & 0x8000)
                continue;
            c = sStarterTable_Colors[sStarterTable_Bg[y][x]];
            r = c & 0x1F, g = (c >> 5) & 0x1F, b = (c >> 10) & 0x1F;
            if (inV && x >= (winH >> 8) && x < (winH & 0xFF))
            {
                r -= (r * 7) >> 4;
                g -= (g * 7) >> 4;
                b -= (b * 7) >> 4;
            }
            if (coeff)
            {
                r += (-r * coeff) >> 4;
                g += (-g * coeff) >> 4;
                b += (-b * coeff) >> 4;
            }
            row[x] = r | (g << 5) | (b << 10) | 0x8000;
        }
    }
}

u16 GetStarterPokemon(u16 chosenStarterId)
{
    if (chosenStarterId > STARTER_MON_COUNT)
        chosenStarterId = 0;
    return sStarterMon[chosenStarterId];
}

static void VblankCB_StarterChoose(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

// Data for Task_StarterChoose
#define tStarterSelection   data[0]
#define tPkmnSpriteId       data[1]
#define tCircleSpriteId     data[2]

// Data for sSpriteTemplate_Pokeball
#define sTaskId data[0]
#define sBallId data[1]

void CB2_ChooseStarter(void)
{
    u8 taskId;
    u8 spriteId;

    SetVBlankCallback(NULL);

    SetGpuReg(REG_OFFSET_DISPCNT, 0);
    SetGpuReg(REG_OFFSET_BG3CNT, 0);
    SetGpuReg(REG_OFFSET_BG2CNT, 0);
    SetGpuReg(REG_OFFSET_BG1CNT, 0);
    SetGpuReg(REG_OFFSET_BG0CNT, 0);

    ChangeBgX(0, 0, BG_COORD_SET);
    ChangeBgY(0, 0, BG_COORD_SET);
    ChangeBgX(1, 0, BG_COORD_SET);
    ChangeBgY(1, 0, BG_COORD_SET);
    ChangeBgX(2, 0, BG_COORD_SET);
    ChangeBgY(2, 0, BG_COORD_SET);
    ChangeBgX(3, 0, BG_COORD_SET);
    ChangeBgY(3, 0, BG_COORD_SET);

    DmaFill16(3, 0, VRAM, VRAM_SIZE);
    DmaFill32(3, 0, OAM, OAM_SIZE);
    DmaFill16(3, 0, PLTT, PLTT_SIZE);

    if (!sTableMode)   // (Moemon Kai: the table is drawn by MP_StarterTable_Composite)
    {
        LZ77UnCompVram(gBirchBagGrass_Gfx, (void *)VRAM);
        LZ77UnCompVram(gBirchBagTilemap, (void *)(BG_SCREEN_ADDR(6)));
        LZ77UnCompVram(gBirchGrassTilemap, (void *)(BG_SCREEN_ADDR(7)));
    }

    ResetBgsAndClearDma3BusyFlags(0);
    InitBgsFromTemplates(0, sBgTemplates, ARRAY_COUNT(sBgTemplates));
    InitWindows(sWindowTemplates);

    DeactivateAllTextPrinters();
    LoadUserWindowBorderGfx(0, 0x2A8, BG_PLTT_ID(13));
    ClearScheduledBgCopiesToVram();
    ScanlineEffect_Stop();
    ResetTasks();
    ResetSpriteData();
    ResetPaletteFade();
    FreeAllSpritePalettes();
    ResetAllPicSprites();

    LoadPalette(GetOverworldTextboxPalettePtr(), BG_PLTT_ID(14), PLTT_SIZE_4BPP);
    LoadPalette(gBirchBagGrass_Pal, BG_PLTT_ID(0), sizeof(gBirchBagGrass_Pal));
    if (sTableMode)
    {
        static const u16 white = RGB_WHITE;
        LoadPalette(&white, BG_PLTT_ID(0), PLTT_SIZEOF(1));   // (the backdrop: MP_StarterTable_Composite reads the fade from it)
    }
    LoadCompressedSpriteSheet(&sSpriteSheet_PokeballSelect[0]);
    LoadCompressedSpriteSheet(&sSpriteSheet_StarterCircle[0]);
    LoadSpritePalettes(sSpritePalettes_StarterChoose);
    BeginNormalPaletteFade(PALETTES_ALL, 0, 0x10, 0, RGB_BLACK);

    EnableInterrupts(DISPSTAT_VBLANK);
    SetVBlankCallback(VblankCB_StarterChoose);
    SetMainCallback2(CB2_StarterChoose);

    SetGpuReg(REG_OFFSET_WININ, WININ_WIN0_BG_ALL | WININ_WIN0_OBJ | WININ_WIN0_CLR);
    SetGpuReg(REG_OFFSET_WINOUT, WINOUT_WIN01_BG_ALL | WINOUT_WIN01_OBJ);
    SetGpuReg(REG_OFFSET_WIN0H, 0);
    SetGpuReg(REG_OFFSET_WIN0V, 0);
    SetGpuReg(REG_OFFSET_BLDCNT, BLDCNT_TGT1_BG1 | BLDCNT_TGT1_BG2 | BLDCNT_TGT1_BG3 | BLDCNT_TGT1_OBJ | BLDCNT_TGT1_BD | BLDCNT_EFFECT_DARKEN);
    SetGpuReg(REG_OFFSET_BLDALPHA, 0);
    SetGpuReg(REG_OFFSET_BLDY, 7);
    if (sTableMode)   // (Moemon Kai: no blend: MP_StarterTable_Composite darkens the label's window itself)
    {
        SetGpuReg(REG_OFFSET_BLDCNT, 0);
        SetGpuReg(REG_OFFSET_BLDY, 0);
    }
    SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_WIN0_ON | DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);

    ShowBg(0);
    if (!sTableMode)
    {
        ShowBg(2);
        ShowBg(3);
    }

    taskId = CreateTask(sBriefcase ? Task_BriefcaseIntro : Task_StarterChoose, 0);
    gTasks[taskId].tStarterSelection = 1;
    sChooseTaskId = taskId;

    // Create hand sprite
    spriteId = CreateSprite(&sSpriteTemplate_Hand, 120, sTableMode ? TABLE_HAND_Y : 56, 2);
    gSprites[spriteId].data[0] = taskId;
    gSprites[spriteId].invisible = sBriefcase;   // (Moemon Kai: the briefcase's picture has Platinum's glove and balls)

    // Create three Poké Ball sprites
    spriteId = CreateSprite(&sSpriteTemplate_Pokeball, BallX(0), BallY(0), 2);
    gSprites[spriteId].sTaskId = taskId;
    gSprites[spriteId].invisible = sBriefcase;
    gSprites[spriteId].sBallId = 0;

    spriteId = CreateSprite(&sSpriteTemplate_Pokeball, BallX(1), BallY(1), 2);
    gSprites[spriteId].sTaskId = taskId;
    gSprites[spriteId].invisible = sBriefcase;
    gSprites[spriteId].sBallId = 1;

    spriteId = CreateSprite(&sSpriteTemplate_Pokeball, BallX(2), BallY(2), 2);
    gSprites[spriteId].sTaskId = taskId;
    gSprites[spriteId].invisible = sBriefcase;
    gSprites[spriteId].sBallId = 2;

    sStarterLabelWindowId = WINDOW_NONE;
}

static void CB2_StarterChoose(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    DoScheduledBgTilemapCopiesToVram();
    UpdatePaletteFade();
}

static void Task_StarterChoose(u8 taskId)
{
    CreateStarterPokemonLabel(gTasks[taskId].tStarterSelection);
    DrawStdFrameWithCustomTileAndPalette(0, FALSE, 0x2A8, 0xD);
    AddTextPrinterParameterized(0, FONT_NORMAL, sBriefcase ? sText_BriefcaseChoose : sTableMode ? sText_TableChoose : gText_BirchInTrouble, 0, 1, 0, NULL);
    PutWindowTilemap(0);
    ScheduleBgCopyTilemapToVram(0);
    gTasks[taskId].func = Task_HandleStarterChooseInput;
}

static void Task_HandleStarterChooseInput(u8 taskId)
{
    u8 selection = gTasks[taskId].tStarterSelection;

    if (sTableMode && gPaletteFade.active)   // (Moemon Kai: no press from the lab's text picks a ball while it fades in)
        return;
    if (sBriefcase)   // the glove shows and the ball under it rocks (BriefcaseComposite)
    {
        sBcChoosing = TRUE;
        sBcBounce++;
    }
    if (JOY_NEW(A_BUTTON))
    {
        u8 spriteId;

        sBcChoosing = FALSE;   // (Platinum hides the glove and stops the ball for the preview)
        ClearStarterLabel();

        // Create white circle background
        spriteId = CreateSprite(&sSpriteTemplate_StarterCircle, BallX(selection), BallY(selection), 1);
        gTasks[taskId].tCircleSpriteId = spriteId;

        // Create Pokémon sprite
        spriteId = CreatePokemonFrontSprite(StarterSpecies(gTasks[taskId].tStarterSelection), BallX(selection), BallY(selection));
        gSprites[spriteId].affineAnims = &sAffineAnims_StarterPokemon;
        gSprites[spriteId].callback = SpriteCB_StarterPokemon;

        gTasks[taskId].tPkmnSpriteId = spriteId;
        gTasks[taskId].func = Task_WaitForStarterSprite;
    }
    else if (sTableMode && sBriefcase && JOY_NEW(B_BUTTON))   // Moemon Kai: put SINNOH's briefcase down
    {
        PlaySE(SE_SELECT);
        ClearStarterLabel();
        gSpecialVar_Result = 0xFFFF;
        gTasks[taskId].func = Task_LeaveTable;
    }
    else if (JOY_NEW(DPAD_LEFT) && selection > 0)
    {
        sBcBounce = 0;
        if (sBriefcase)
            PlaySE(SE_SELECT);
        gTasks[taskId].tStarterSelection--;
        gTasks[taskId].func = Task_MoveStarterChooseCursor;
    }
    else if (JOY_NEW(DPAD_RIGHT) && selection < STARTER_MON_COUNT - 1)
    {
        sBcBounce = 0;
        if (sBriefcase)
            PlaySE(SE_SELECT);
        gTasks[taskId].tStarterSelection++;
        gTasks[taskId].func = Task_MoveStarterChooseCursor;
    }
}

static void Task_WaitForStarterSprite(u8 taskId)
{
    if (gSprites[gTasks[taskId].tCircleSpriteId].affineAnimEnded &&
        gSprites[gTasks[taskId].tCircleSpriteId].x == STARTER_PKMN_POS_X &&
        gSprites[gTasks[taskId].tCircleSpriteId].y == STARTER_PKMN_POS_Y)
    {
        gTasks[taskId].func = Task_AskConfirmStarter;
    }
}

static void Task_AskConfirmStarter(u8 taskId)
{
    PlayCry_Normal(StarterSpecies(gTasks[taskId].tStarterSelection), 0);
    FillWindowPixelBuffer(0, PIXEL_FILL(1));
    AddTextPrinterParameterized(0, FONT_NORMAL, gText_ConfirmStarterChoice, 0, 1, 0, NULL);
    ScheduleBgCopyTilemapToVram(0);
    CreateYesNoMenu(&sWindowTemplate_ConfirmStarter, 0x2A8, 0xD, 0);
    gTasks[taskId].func = Task_HandleConfirmStarterInput;
}

static void Task_HandleConfirmStarterInput(u8 taskId)
{
    u8 spriteId;

    switch (Menu_ProcessInputNoWrapClearOnChoose())
    {
    case 0:  // YES
        // Return the starter choice and exit.
        gSpecialVar_Result = gTasks[taskId].tStarterSelection;
        if (sTableMode)   // Moemon Kai: fade out to the lab (the field fades back in)
        {
            BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
            gTasks[taskId].func = Task_LeaveTable;
            break;
        }
        ResetAllPicSprites();
        SetMainCallback2(gMain.savedCallback);
        break;
    case 1:  // NO
    case MENU_B_PRESSED:
        PlaySE(SE_SELECT);
        spriteId = gTasks[taskId].tPkmnSpriteId;
        FreeOamMatrix(gSprites[spriteId].oam.matrixNum);
        FreeAndDestroyMonPicSprite(spriteId);

        spriteId = gTasks[taskId].tCircleSpriteId;
        FreeOamMatrix(gSprites[spriteId].oam.matrixNum);
        DestroySprite(&gSprites[spriteId]);
        gTasks[taskId].func = Task_DeclineStarter;
        break;
    }
}

// The briefcase's opening as Platinum plays it at 30 frames a second: a moment on the closed case, the case opening
// (its opening frames), the camera coming down on it (its camera frames), then choosing
#define BC_WAIT 36
static void Task_BriefcaseIntro(u8 taskId)
{
    const struct S3dStarterGfx *g = S3D_Gfx_Starter();
    s16 *t = &gTasks[taskId].data[3];
    if (g == NULL || JOY_NEW(START_BUTTON))   // (START skips it)
    {
        sBcFrame = g ? g->openFrames + g->camFrames - 1 : 0;
        gTasks[taskId].func = Task_StarterChoose;
        return;
    }
    if (gPaletteFade.active)
        return;
    (*t)++;
    if (*t == BC_WAIT)
        PlaySE(SE_BALL_OPEN);
    if (*t >= BC_WAIT)
    {
        u16 f = (*t - BC_WAIT) / 2;
        if (f >= g->openFrames + g->camFrames + 6)   // (and a short look at it)
        {
            sBcFrame = g->openFrames + g->camFrames - 1;
            gTasks[taskId].func = Task_StarterChoose;
            return;
        }
        sBcFrame = f < g->openFrames + g->camFrames ? f : g->openFrames + g->camFrames - 1;
    }
}

static void Task_LeaveTable(u8 taskId)
{
    if (!gPaletteFade.active && gPaletteFade.y == 0 && gSpecialVar_Result == 0xFFFF)
    {
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);   // (B: start the fade)
        return;
    }
    if (gPaletteFade.active)
        return;
    sTableMode = FALSE;
    sBriefcase = FALSE;
    sBcChoosing = FALSE;
    ResetAllPicSprites();
    SetMainCallback2(gMain.savedCallback);
}

static void Task_DeclineStarter(u8 taskId)
{
    gTasks[taskId].func = Task_StarterChoose;
}

static void CreateStarterPokemonLabel(u8 selection)
{
    u8 categoryText[32];
    struct WindowTemplate winTemplate;
    const u8 *speciesName;
    s32 width;
    u8 labelLeft, labelRight, labelTop, labelBottom;

    u16 species = StarterSpecies(selection);
    if (sBriefcase)   // (no labels: Platinum's picture says enough)
        return;
    CopyMonCategoryText(SpeciesToNationalPokedexNum(species), categoryText);
    speciesName = gSpeciesNames[species];

    winTemplate = sWindowTemplate_StarterLabel;
    winTemplate.tilemapLeft = sStarterLabelCoords[selection][0];
    winTemplate.tilemapTop = sStarterLabelCoords[selection][1];
    if (sTableMode)   // under the ball (13 tiles wide, centered on it, kept on screen)
    {
        s32 left = (BallX(selection) - 52) / 8;
        winTemplate.tilemapLeft = left < 0 ? 0 : left > 30 - 13 ? 30 - 13 : left;
        winTemplate.tilemapTop = TABLE_LABEL_TOP;
    }

    sStarterLabelWindowId = AddWindow(&winTemplate);
    FillWindowPixelBuffer(sStarterLabelWindowId, PIXEL_FILL(0));

    width = GetStringCenterAlignXOffset(FONT_NARROW, categoryText, 0x68);
    AddTextPrinterParameterized3(sStarterLabelWindowId, FONT_NARROW, width, 1, sTextColors, 0, categoryText);

    width = GetStringCenterAlignXOffset(FONT_NORMAL, speciesName, 0x68);
    AddTextPrinterParameterized3(sStarterLabelWindowId, FONT_NORMAL, width, 17, sTextColors, 0, speciesName);

    PutWindowTilemap(sStarterLabelWindowId);
    ScheduleBgCopyTilemapToVram(0);

    labelLeft = winTemplate.tilemapLeft * 8 - 4;
    labelRight = (winTemplate.tilemapLeft + 13) * 8 + 4;
    labelTop = winTemplate.tilemapTop * 8;
    labelBottom = (winTemplate.tilemapTop + 4) * 8;
    if (winTemplate.tilemapLeft == 0)
        labelLeft = 0;
    SetGpuReg(REG_OFFSET_WIN0H, WIN_RANGE(labelLeft, labelRight));
    SetGpuReg(REG_OFFSET_WIN0V, WIN_RANGE(labelTop, labelBottom));
}

static void ClearStarterLabel(void)
{
    if (sStarterLabelWindowId == WINDOW_NONE)   // (Moemon Kai: the briefcase has none)
        return;
    FillWindowPixelBuffer(sStarterLabelWindowId, PIXEL_FILL(0));
    ClearWindowTilemap(sStarterLabelWindowId);
    RemoveWindow(sStarterLabelWindowId);
    sStarterLabelWindowId = WINDOW_NONE;
    SetGpuReg(REG_OFFSET_WIN0H, 0);
    SetGpuReg(REG_OFFSET_WIN0V, 0);
    ScheduleBgCopyTilemapToVram(0);
}

static void Task_MoveStarterChooseCursor(u8 taskId)
{
    ClearStarterLabel();
    gTasks[taskId].func = Task_CreateStarterLabel;
}

static void Task_CreateStarterLabel(u8 taskId)
{
    CreateStarterPokemonLabel(gTasks[taskId].tStarterSelection);
    gTasks[taskId].func = Task_HandleStarterChooseInput;
}

static u8 CreatePokemonFrontSprite(u16 species, u8 x, u8 y)
{
    u8 spriteId;

    spriteId = CreateMonPicSprite_Affine(species, SHINY_ODDS, 0, MON_PIC_AFFINE_FRONT, x, y, 14, TAG_NONE);
    gSprites[spriteId].oam.priority = 0;
    return spriteId;
}

static void SpriteCB_SelectionHand(struct Sprite *sprite)
{
    // Float up and down above selected Poké Ball
    sprite->x = sTableMode ? BallX(gTasks[sprite->data[0]].tStarterSelection) : sCursorCoords[gTasks[sprite->data[0]].tStarterSelection][0];
    sprite->y = sTableMode ? TABLE_HAND_Y : sCursorCoords[gTasks[sprite->data[0]].tStarterSelection][1];
    sprite->y2 = Sin(sprite->data[1], 8);
    sprite->data[1] = (u8)(sprite->data[1]) + 4;
}

static void SpriteCB_Pokeball(struct Sprite *sprite)
{
    // Animate Poké Ball if currently selected
    if (gTasks[sprite->sTaskId].tStarterSelection == sprite->sBallId)
        StartSpriteAnimIfDifferent(sprite, 1);
    else
        StartSpriteAnimIfDifferent(sprite, 0);
}

static void SpriteCB_StarterPokemon(struct Sprite *sprite)
{
    // Move sprite to upper center of screen
    // (Moemon Kai: steps of at most the distance left, so it lands from anywhere: the briefcase's balls)
    if (sprite->x > STARTER_PKMN_POS_X)
        sprite->x -= min(4, sprite->x - STARTER_PKMN_POS_X);
    if (sprite->x < STARTER_PKMN_POS_X)
        sprite->x += min(4, STARTER_PKMN_POS_X - sprite->x);
    if (sprite->y > STARTER_PKMN_POS_Y)
        sprite->y -= min(2, sprite->y - STARTER_PKMN_POS_Y);
    if (sprite->y < STARTER_PKMN_POS_Y)
        sprite->y += min(2, STARTER_PKMN_POS_Y - sprite->y);
}
