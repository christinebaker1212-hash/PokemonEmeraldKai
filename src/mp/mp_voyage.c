// Moemon Kai: the voyage between regions and KANTO's SEAGALLOP ferries, as FireRed's ferry scene (pokefirered
// src/seagallop.c, its art in graphics/moemon/voyage): the sea scrolls under the boat as it crosses the screen, its
// wake trailing, 140 frames; then the fade and the warp to wherever the trip was set to go (SetWarpDestination before
// MP_Voyage_Begin). Eastbound the boat faces right and the sea runs left, westbound the other way.
#include "global.h"
#include "bg.h"
#include "event_data.h"
#include "field_screen_effect.h"
#include "event_object_movement.h"
#include "field_weather.h"
#include "gpu_regs.h"
#include "main.h"
#include "malloc.h"
#include "overworld.h"
#include "palette.h"
#include "scanline_effect.h"
#include "script.h"
#include "sound.h"
#include "sprite.h"
#include "task.h"
#include "text_window.h"
#include "constants/rgb.h"
#include "constants/songs.h"

void MPNet_Log(const char *fmt, ...);

#define TAG_FERRY 0x5E10
#define TAG_WAKE  0x5E11

static const u16 sWaterTiles[] = INCGFX_U16("graphics/moemon/voyage/water.png", ".4bpp");
static const u16 sWaterPal[] = INCGFX_U16("graphics/moemon/voyage/water.png", ".gbapal");
static const u16 sWaterTilemapWest[] = INCBIN_U16("graphics/moemon/voyage/wb_tilemap.bin");
static const u16 sWaterTilemapEast[] = INCBIN_U16("graphics/moemon/voyage/eb_tilemap.bin");
static const u16 sFerryTiles[] = INCGFX_U16("graphics/moemon/voyage/ferry_sprite.png", ".4bpp");
static const u16 sFerryAndWakePal[] = INCGFX_U16("graphics/moemon/voyage/ferry_and_wake.pal", ".gbapal");
static const u16 sWakeTiles[] = INCGFX_U16("graphics/moemon/voyage/wake.png", ".4bpp");

static EWRAM_DATA void *sTilemap = NULL;
static bool8 sEastbound;

static const struct BgTemplate sBgTemplate[] = {
    {.bg = 3, .charBaseIndex = 3, .mapBaseIndex = 30, .screenSize = 0, .paletteMode = 0, .priority = 3, .baseTile = 0},
};

static const union AnimCmd sAnim_FerryWest[] = {ANIMCMD_FRAME(0, 10), ANIMCMD_END};
static const union AnimCmd sAnim_FerryEast[] = {ANIMCMD_FRAME(0, 10, .hFlip = TRUE), ANIMCMD_END};
static const union AnimCmd *const sAnims_Ferry[] = {sAnim_FerryWest, sAnim_FerryEast};
static const union AnimCmd sAnim_WakeWest[] = {ANIMCMD_FRAME(0x00, 0x14), ANIMCMD_FRAME(0x10, 0x14), ANIMCMD_FRAME(0x20, 0x0f), ANIMCMD_END};
static const union AnimCmd sAnim_WakeEast[] = {ANIMCMD_FRAME(0x00, 0x14, .hFlip = TRUE), ANIMCMD_FRAME(0x10, 0x14, .hFlip = TRUE),
                                               ANIMCMD_FRAME(0x20, 0x0f, .hFlip = TRUE), ANIMCMD_END};
static const union AnimCmd *const sAnims_Wake[] = {sAnim_WakeWest, sAnim_WakeEast};
static const struct OamData sOam_Ferry = {.size = 3};
static const struct OamData sOam_Wake = {.size = 2};

static void SpriteCB_Ferry(struct Sprite *sprite);
static void SpriteCB_Wake(struct Sprite *sprite);
static const struct SpriteTemplate sFerryTemplate = {TAG_FERRY, TAG_FERRY, &sOam_Ferry, sAnims_Ferry, NULL, gDummySpriteAffineAnimTable, SpriteCB_Ferry};
static const struct SpriteTemplate sWakeTemplate = {TAG_WAKE, TAG_FERRY, &sOam_Wake, sAnims_Wake, NULL, gDummySpriteAffineAnimTable, SpriteCB_Wake};

static void CreateWake(s16 x)
{
    u8 id = CreateSprite(&sWakeTemplate, x, 92, 8);
    if (id != MAX_SPRITES && sEastbound)
        StartSpriteAnim(&gSprites[id], 1);
}

static void SpriteCB_Ferry(struct Sprite *sprite)
{
    sprite->data[1] += sprite->data[0];
    sprite->x2 = sprite->data[1] >> 4;
    if (sprite->data[2] % 5 == 0)
        CreateWake(sprite->x + sprite->x2);
    sprite->data[2]++;
    if ((u16)(300 + sprite->x2) > 600)
        DestroySprite(sprite);
}

static void SpriteCB_Wake(struct Sprite *sprite)
{
    if (sprite->animEnded)
        DestroySprite(sprite);
}

static void VBlankCB_Voyage(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void CB2_Voyage(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    UpdatePaletteFade();
}

static void Scroll(void)
{
    ChangeBgX(3, 0x600, sEastbound ? 1 : 2);
}

static void Task_VoyageEnd(u8 taskId)
{
    Scroll();
    if (BGMusicStopped() && !gPaletteFade.active)
    {
        DestroyTask(taskId);
        FreeSpriteTilesByTag(TAG_FERRY);
        FreeSpriteTilesByTag(TAG_WAKE);
        FreeSpritePaletteByTag(TAG_FERRY);
        Free(sTilemap);
        sTilemap = NULL;
        SetVBlankCallback(NULL);
        PlaySE(SE_EXIT);
        gFieldCallback = FieldCB_DefaultWarpExit;
        WarpIntoMap();
        SetMainCallback2(CB2_LoadMap);
        ResetInitialPlayerAvatarState();
        MPNet_Log("voyage: arrived");
    }
}

static void Task_Voyage(u8 taskId)
{
    Scroll();
    if (++gTasks[taskId].data[1] == 140)
    {
        Overworld_FadeOutMapMusic();
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
        gTasks[taskId].func = Task_VoyageEnd;
    }
}

static void CB2_SetUpVoyage(void)
{
    struct SpriteSheet sheets[] = {{sWakeTiles, sizeof(sWakeTiles), TAG_WAKE}, {sFerryTiles, sizeof(sFerryTiles), TAG_FERRY}, {}};
    struct SpritePalette pals[] = {{sFerryAndWakePal, TAG_FERRY}, {}};
    u8 id;
    SetVBlankCallback(NULL);
    SetGpuReg(REG_OFFSET_DISPCNT, 0);
    DmaClearLarge16(3, (void *)VRAM, VRAM_SIZE, 0x1000);
    ScanlineEffect_Stop();
    ResetTasks();
    ResetSpriteData();
    ResetPaletteFade();
    FreeAllSpritePalettes();
    sTilemap = AllocZeroed(0x800);
    ResetBgsAndClearDma3BusyFlags(0);
    InitBgsFromTemplates(0, sBgTemplate, ARRAY_COUNT(sBgTemplate));
    SetBgTilemapBuffer(3, sTilemap);
    ChangeBgX(3, 0, 0);
    ChangeBgY(3, 0, 0);
    SetGpuReg(REG_OFFSET_BG3VOFS, 0);
    LoadBgTiles(3, sWaterTiles, sizeof(sWaterTiles), 0);
    CopyToBgTilemapBufferRect(3, sEastbound ? sWaterTilemapEast : sWaterTilemapWest, 0, 0, 32, 32);
    LoadPalette(sWaterPal, BG_PLTT_ID(4), PLTT_SIZE_4BPP);
    {
        static const u16 sBlack = RGB_BLACK;
        LoadPalette(&sBlack, 0, sizeof(sBlack));   // (the backdrop: black above and below the letterbox)
    }
    CopyBgTilemapBufferToVram(3);
    ShowBg(3);
    LoadSpriteSheets(sheets);
    LoadSpritePalettes(pals);
    BlendPalettes(PALETTES_ALL, 16, RGB_BLACK);
    BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
    SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_MODE_0 | DISPCNT_OBJ_1D_MAP | DISPCNT_BG3_ON | DISPCNT_OBJ_ON | DISPCNT_WIN0_ON);
    SetGpuReg(REG_OFFSET_WININ, 0x3F);   // (FireRed's letterbox: the scene between rows 24 and 136)
    SetGpuReg(REG_OFFSET_WINOUT, 0x00);
    SetGpuReg(REG_OFFSET_WIN0H, 0x00F0);
    SetGpuReg(REG_OFFSET_WIN0V, 0x1888);
    SetVBlankCallback(VBlankCB_Voyage);
    PlaySE(SE_SHIP);
    id = CreateSprite(&sFerryTemplate, 0, 92, 0);
    if (id != MAX_SPRITES)
    {
        gSprites[id].data[0] = 48;
        if (sEastbound)
            StartSpriteAnim(&gSprites[id], 1);
        else
        {
            gSprites[id].x = 240;
            gSprites[id].data[0] = -48;
        }
    }
    CreateTask(Task_Voyage, 8);
    SetMainCallback2(CB2_Voyage);
    MPNet_Log("voyage: %s", sEastbound ? "east" : "west");
}

static void Task_FieldFadeOut(u8 taskId)
{
    if (gPaletteFade.active)
        return;
    DestroyTask(taskId);
    SetMainCallback2(CB2_SetUpVoyage);
}

// After SetWarpDestination: the field fades out (unless a script faded it already), the voyage, then the warp (in
// DoWarp's place)
void MP_Voyage_Begin(bool8 eastbound, bool8 faded)
{
    sEastbound = eastbound;
    LockPlayerFieldControls();
    FreezeObjectEvents();
    if (!faded)   // (the port's music plays on through the voyage, as FireRed's)
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
    CreateTask(Task_FieldFadeOut, 10);
}
