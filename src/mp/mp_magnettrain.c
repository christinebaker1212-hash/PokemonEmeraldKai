// Moemon Kai: the MAGNET TRAIN's ride between GOLDENROD and SAFFRON, as Crystal shows it (pokecrystal
// engine/events/magnet_train.asm, art from tools/quetzal_port/gen_magnet_train.py): the scenery's band scrolls by four
// pixels a frame (BG1), the train's band (BG0: Crystal's train over the rails) waits off screen, slides in, holds
// while the trees rush past, then leaves the other side, with the player's head in a window; the waits are Crystal's
// (128 frames each). Crystal's screen is 160 wide: here the train comes on from off the 240-wide screen and stops in
// its middle. Then the game warps to the other station (qz_specials.c TravelNow, where DoWarp's fade would be).
#include "global.h"
#include "bg.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "gpu_regs.h"
#include "main.h"
#include "malloc.h"
#include "overworld.h"
#include "palette.h"
#include "rtc.h"
#include "script.h"
#include "sound.h"
#include "sprite.h"
#include "task.h"
#include "constants/event_object_movement.h"
#include "constants/rgb.h"
#include "constants/songs.h"
#include "../data/qz_magnet_train.h"

#define TRAIN_W      160    // Crystal's train, 20 tiles
#define TRAIN_TOP    7      // its band on the GBA screen: rows 7-12 (Crystal's 6-11, a row lower: 20 rows, not 18)
#define WAIT         128
#define BLANK_TILE   96     // (sMagnetTrainTiles holds 96)
#define PAL_GRAY     0
#define PAL_GREEN    1
#define PAL_YELLOW   2

static EWRAM_DATA struct
{
    u16 trainMap[32 * 64];      // BG0, 64 x 32 tiles (screen size 1): the train's band
    u16 sceneryMap[32 * 32];    // BG1
    s16 trainX;                 // the train's left edge on the screen
    u16 scroll;
    s8 dir;                     // 1: to SAFFRON (left to right, as Crystal's forward), -1: to GOLDENROD
    u8 state, wait;
    u8 playerSprite;
    u16 gfx;
} *sMT = NULL;

static const struct BgTemplate sBgTemplates[] = {
    {.bg = 0, .charBaseIndex = 0, .mapBaseIndex = 28, .screenSize = 1, .paletteMode = 0, .priority = 1, .baseTile = 0},
    {.bg = 1, .charBaseIndex = 0, .mapBaseIndex = 31, .screenSize = 0, .paletteMode = 0, .priority = 2, .baseTile = 0},
};

static u16 Entry(u8 tile, u8 pal)
{
    return tile | (pal << 12);
}

// Crystal's rows by GBA row: its 18 rows a row lower, and the GBA's extra row at the top and the bottom carries on
// the trees' two-row pattern (Crystal's rows 0-3 and 14-17 are two rows of trees each: the row above row 0 is a
// tree's lower half, row 1's; the row below row 17 a tree's upper half, row 16's), so the screen's edges show whole
// rows of trees scrolling with the rest instead of a repeated half row
static u8 CrystalRow(u8 gbaRow)
{
    if (gbaRow == 0)
        return 1;
    if (gbaRow > 18)
        return 16;
    return gbaRow - 1;
}

static void BuildMaps(void)
{
    u32 x, y;
    for (y = 0; y < 32; y++)
        for (x = 0; x < 32; x++)
        {
            u8 r = CrystalRow(y < 20 ? y : 19 - (y & 1));
            sMT->sceneryMap[y * 32 + x] = Entry(sMagnetTrainBgRows[r][x & 1], r >= 4 && r <= 13 ? PAL_GRAY : PAL_GREEN);
        }
    // the train's layer is clear above and below its band (tile BLANK_TILE: past the 96 loaded, zeros in VRAM, every
    // pixel transparent), or tile 0 (opaque: Crystal's shades are colours 1-4) covered the scenery there, standing still
    for (x = 0; x < ARRAY_COUNT(sMT->trainMap); x++)
        sMT->trainMap[x] = BLANK_TILE;
    // the train's band: the rails (Crystal's rows 6-11 of the scenery) across, the train at columns 0-19
    for (y = TRAIN_TOP; y < TRAIN_TOP + 6; y++)
        for (x = 0; x < 64; x++)
        {
            u8 r = CrystalRow(y);
            u16 *e = &sMT->trainMap[(x < 32 ? 0 : 32 * 32) + y * 32 + (x & 31)];
            *e = Entry(sMagnetTrainBgRows[r][x & 1], PAL_GRAY);
            if (x < 20 && y < TRAIN_TOP + 4)
                *e = Entry(sMagnetTrainFg[y - TRAIN_TOP][x], (y == TRAIN_TOP + 2 && x >= 7 && x <= 12) ? PAL_YELLOW : PAL_GRAY);
        }
}

static void LoadTimePalettes(void)
{
    u32 t;
    RtcCalcLocalTime();
    t = gLocalTime.hours >= 4 && gLocalTime.hours < 10 ? 0 : gLocalTime.hours >= 10 && gLocalTime.hours < 18 ? 1 : 2;
    LoadPalette(sMagnetTrainPals[t][0], BG_PLTT_ID(PAL_GRAY), PLTT_SIZE_4BPP);
    LoadPalette(sMagnetTrainPals[t][1], BG_PLTT_ID(PAL_GREEN), PLTT_SIZE_4BPP);
    LoadPalette(sMagnetTrainPals[t][2], BG_PLTT_ID(PAL_YELLOW), PLTT_SIZE_4BPP);
}

static void VBlankCB_MagnetTrain(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void CB2_MagnetTrain(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    UpdatePaletteFade();
}

static void PlaceTrain(void)
{
    // BG0 scrolls so the train's map column 0 shows at trainX (64 tiles wide: it hides off screen on either side)
    SetGpuReg(REG_OFFSET_BG0HOFS, (u16)(-sMT->trainX) & 511);
    SetGpuReg(REG_OFFSET_BG1HOFS, sMT->scroll & 255);
    if (sMT->playerSprite < MAX_SPRITES)
    {
        struct Sprite *s = &gSprites[sMT->playerSprite];
        s->x = sMT->trainX + 84;   // (the train's middle window, Crystal's sprite on it)
        s->invisible = s->x < -16 || s->x > DISPLAY_WIDTH + 16;
    }
}

static void Task_MagnetTrain(u8 taskId)
{
    // the scenery: Crystal's wMagnetTrainOffset * 2, two a frame
    sMT->scroll += 4 * sMT->dir;
    switch (sMT->state)
    {
    case 0:   // the scenery alone (Crystal's first WaitScene)
        if (gPaletteFade.active)
            break;
        if (++sMT->wait >= WAIT)
            sMT->state++, sMT->wait = 0;
        break;
    case 1:   // the train comes on (Crystal's MoveTrain1)
        sMT->trainX += 2 * sMT->dir;
        if (sMT->trainX == (DISPLAY_WIDTH - TRAIN_W) / 2)
            sMT->state++;
        break;
    case 2:   // it holds (WaitScene)
        if (++sMT->wait >= WAIT)
            sMT->state++, sMT->wait = 0;
        break;
    case 3:   // and leaves (MoveTrain2)
        sMT->trainX += 2 * sMT->dir;
        if (sMT->trainX <= -TRAIN_W || sMT->trainX >= DISPLAY_WIDTH)
            sMT->state++;
        break;
    case 4:   // the last WaitScene, then it has arrived (SFX_TRAIN_ARRIVED)
        if (++sMT->wait >= WAIT / 2)
        {
            PlaySE(SE_DING_DONG);
            BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
            sMT->state++;
        }
        break;
    case 5:
        if (gPaletteFade.active)
            break;
        DestroyTask(taskId);
        if (sMT->playerSprite < MAX_SPRITES)
            DestroySprite(&gSprites[sMT->playerSprite]);
        Free(sMT);
        sMT = NULL;
        SetVBlankCallback(NULL);
        WarpIntoMap();
        SetMainCallback2(CB2_LoadMap);
        return;
    }
    PlaceTrain();
}

static void CB2_InitMagnetTrain(void)
{
    SetVBlankCallback(NULL);
    SetGpuReg(REG_OFFSET_DISPCNT, 0);
    ResetBgsAndClearDma3BusyFlags(0);
    InitBgsFromTemplates(0, sBgTemplates, ARRAY_COUNT(sBgTemplates));
    DmaFill16(3, 0, (void *)VRAM, VRAM_SIZE);
    ResetPaletteFade();
    ResetSpriteData();
    FreeAllSpritePalettes();
    ResetTasks();
    LoadBgTiles(0, sMagnetTrainTiles, sizeof(sMagnetTrainTiles), 0);
    BuildMaps();
    SetBgTilemapBuffer(0, sMT->trainMap);
    SetBgTilemapBuffer(1, sMT->sceneryMap);
    CopyBgTilemapBufferToVram(0);
    CopyBgTilemapBufferToVram(1);
    LoadTimePalettes();
    // the player: their own look, the head and shoulders in the window (cut off below it, as Crystal's 16x16 icon)
    sMT->playerSprite = CreateObjectGraphicsSprite(sMT->gfx, SpriteCallbackDummy, 0, TRAIN_TOP * 8 + 20, 0);
    if (sMT->playerSprite < MAX_SPRITES)
    {
        struct Sprite *s = &gSprites[sMT->playerSprite];
        s->oam.priority = 0;
        s->mpClipOn = TRUE;
        s->mpClipY = (TRAIN_TOP + 3) * 8;
        StartSpriteAnim(s, sMT->dir > 0 ? ANIM_STD_GO_EAST : ANIM_STD_GO_WEST);
    }
    // the field's camera leaves its vertical scroll in BG1-3's registers (the scenery stood 40 pixels up: the rails'
    // fence at the screen's top in the trees' place, and too many rows of trees at the bottom)
    SetGpuReg(REG_OFFSET_BG0VOFS, 0);
    SetGpuReg(REG_OFFSET_BG1VOFS, 0);
    sMT->trainX = sMT->dir > 0 ? -TRAIN_W : DISPLAY_WIDTH;
    PlaceTrain();
    ShowBg(0);
    ShowBg(1);
    SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP | DISPCNT_BG0_ON | DISPCNT_BG1_ON);
    BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
    SetVBlankCallback(VBlankCB_MagnetTrain);
    CreateTask(Task_MagnetTrain, 0);
    SetMainCallback2(CB2_MagnetTrain);
}

// The field fades out (DoWarp's, the music going on), then the ride
static void Task_MagnetTrainFadeOut(u8 taskId)
{
    if (gPaletteFade.active)
        return;
    DestroyTask(taskId);
    SetMainCallback2(CB2_InitMagnetTrain);
}

// After SetWarpDestination: TO_SAFFRON TRUE heads east (Crystal's forward). FALSE when there's no memory for it (the
// caller warps the usual way)
bool8 MP_MagnetTrain_Ride(bool8 toSaffron)
{
    if (sMT == NULL)
        sMT = AllocZeroed(sizeof(*sMT));
    if (sMT == NULL)
        return FALSE;
    sMT->dir = toSaffron ? 1 : -1;
    sMT->gfx = OBJ_GFX_ID(&gObjectEvents[gPlayerAvatar.objectEventId]);
    sMT->playerSprite = MAX_SPRITES;
    FreezeObjectEvents();
    LockPlayerFieldControls();
    BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
    CreateTask(Task_MagnetTrainFadeOut, 10);
    return TRUE;
}
