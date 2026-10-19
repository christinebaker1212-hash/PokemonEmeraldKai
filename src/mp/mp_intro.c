// Moemon Co-op: the new game's "let me get a look at you" step.
// After the boy/girl question PROF. BIRCH asks to see the player, and the game
// opens the LOOK menu (mp_menu.c, intro mode) on the intro's backdrop, with the
// field sprite standing on the platform turning around so changes show at once.
// When the player is done, the intro picks up again (main_menu.c).
#include <string.h>
#include "global.h"
#include "mp.h"
#include "bg.h"
#include "event_object_movement.h"
#include "gpu_regs.h"
#include "main.h"
#include "menu.h"
#include "palette.h"
#include "scanline_effect.h"
#include "sprite.h"
#include "task.h"
#include "text.h"
#include "text_window.h"
#include "window.h"
#include "constants/event_objects.h"
#include "constants/event_object_movement.h"
#include "constants/rgb.h"

void NewGameBirchSpeech_LoadBackdrop(u8 bg);   // main_menu.c
void CB2_NewGameBirchSpeech_ReturnFromLook(void);
void MP_OpenLookForIntro(void (*onDone)(void));

#define SPRITE_X 116
#define SPRITE_Y 88

static const struct BgTemplate sBgTemplates[] = {
    // Windows, laid out like the field's so the co-op menus draw the same
    {.bg = 0, .charBaseIndex = 2, .mapBaseIndex = 31, .screenSize = 0, .paletteMode = 0, .priority = 0, .baseTile = 0},
    // PROF. BIRCH's backdrop
    {.bg = 1, .charBaseIndex = 0, .mapBaseIndex = 7, .screenSize = 0, .paletteMode = 0, .priority = 3, .baseTile = 0},
};

static const struct WindowTemplate sWindows[] = {
    // Window 0 only gives the message box graphics a home (LoadMessageBoxAndBorderGfx)
    {.bg = 0, .tilemapLeft = 0, .tilemapTop = 0, .width = 1, .height = 1, .paletteNum = 15, .baseBlock = 0x300},
    DUMMY_WIN_TEMPLATE,
};

static EWRAM_DATA u8 sSpriteId = 0;
static EWRAM_DATA bool8 sLeaving = FALSE;
static EWRAM_DATA struct MpAppearance sShown = {0};
static EWRAM_DATA u16 sTurnTimer = 0;
static EWRAM_DATA u16 sSpriteSize = 0;

static void VBlankCB_IntroLook(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void CB2_IntroLook(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    RunTextPrinters();
    UpdatePaletteFade();
    DoScheduledBgTilemapCopiesToVram();
}

// The look's graphics are rebuilt in place, so the sprite's frame size can grow
// under it: free the tiles it was made with, not the new size (that would free
// the next sprite's tiles too)
static void DestroyFieldSprite(void)
{
    static struct SpriteFrameImage sMadeWith;
    if (sSpriteId == MAX_SPRITES)
        return;
    sMadeWith.data = NULL;
    sMadeWith.size = sSpriteSize;
    gSprites[sSpriteId].images = &sMadeWith;
    DestroySprite(&gSprites[sSpriteId]);
    sSpriteId = MAX_SPRITES;
}

// The field sprite, made again whenever the look changes
static void ShowFieldSprite(void)
{
    static const u8 sTurn[] = {ANIM_STD_FACE_SOUTH, ANIM_STD_FACE_WEST, ANIM_STD_FACE_NORTH, ANIM_STD_FACE_EAST};
    const struct ObjectEventGraphicsInfo *info;
    DestroyFieldSprite();
    info = GetObjectEventGraphicsInfo(OBJ_EVENT_GFX_MP_LOCAL);
    FreeSpritePaletteByTag(info->paletteTag);
    sSpriteId = CreateObjectGraphicsSprite(OBJ_EVENT_GFX_MP_LOCAL, SpriteCallbackDummy, SPRITE_X, SPRITE_Y - info->height / 2, 0);
    if (sSpriteId != MAX_SPRITES)
    {
        sSpriteSize = gSprites[sSpriteId].images->size;
        gSprites[sSpriteId].oam.priority = 1;
        StartSpriteAnim(&gSprites[sSpriteId], sTurn[(sTurnTimer / 48) % 4]);
    }
    sShown = gMpConfig.app;
}

static void Task_IntroLook(u8 taskId)
{
    if (sLeaving)
    {
        if (!gPaletteFade.active)
        {
            DestroyTask(taskId);
            DestroyFieldSprite();
            FreeAllWindowBuffers();
            SetMainCallback2(CB2_NewGameBirchSpeech_ReturnFromLook);
        }
        return;
    }
    // Turn around slowly; show changes right away
    if (++sTurnTimer % 48 == 0 || memcmp(&sShown, &gMpConfig.app, sizeof(sShown)) != 0)
        ShowFieldSprite();
}

static void IntroLookDone(void)
{
    sLeaving = TRUE;
    BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
}

void CB2_MpIntroLook(void)
{
    SetVBlankCallback(NULL);
    SetGpuReg(REG_OFFSET_DISPCNT, 0);
    ResetBgsAndClearDma3BusyFlags(0);
    InitBgsFromTemplates(0, sBgTemplates, ARRAY_COUNT(sBgTemplates));
    DmaFill16(3, 0, VRAM, VRAM_SIZE);
    DmaFill32(3, 0, OAM, OAM_SIZE);
    DmaFill16(3, 0, PLTT, PLTT_SIZE);
    ResetPaletteFade();
    ScanlineEffect_Stop();
    ResetSpriteData();
    FreeAllSpritePalettes();
    ResetTasks();
    NewGameBirchSpeech_LoadBackdrop(1);
    SetGpuReg(REG_OFFSET_BG1HOFS, 136 - SPRITE_X);   // the platform under the player
    SetGpuReg(REG_OFFSET_BG1VOFS, 0);
    InitWindows(sWindows);
    DeactivateAllTextPrinters();
    LoadMessageBoxAndBorderGfx();
    SetGpuReg(REG_OFFSET_BLDCNT, 0);
    SetGpuReg(REG_OFFSET_WININ, 0);
    SetGpuReg(REG_OFFSET_WINOUT, 0);
    SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
    ShowBg(0);
    ShowBg(1);
    sSpriteId = MAX_SPRITES;
    sLeaving = FALSE;
    sTurnTimer = 0;
    MP_Avatar_ApplyLocal();   // the look for the gender just picked
    ShowFieldSprite();
    // Runs after the menu's task (80): a look that changes size gets its new sprite
    // before the old one copies bigger frames over the next sprite's tiles
    CreateTask(Task_IntroLook, 200);
    MP_OpenLookForIntro(IntroLookDone);
    BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
    SetVBlankCallback(VBlankCB_IntroLook);
    SetMainCallback2(CB2_IntroLook);
}

// Test harness condition "introlook"
bool8 MP_IntroLook_Active(void)
{
    return FindTaskIdByFunc(Task_IntroLook) != TASK_NONE && !sLeaving && !gPaletteFade.active;
}
