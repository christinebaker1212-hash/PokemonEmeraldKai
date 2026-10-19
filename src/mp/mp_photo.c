// Moemon Co-op: TEAM PHOTO (CO-OP > SOCIAL). Friends on the same map line up
// on PROF. BIRCH's stage for a snapshot: everyone's trainer picture with their look, names, the place and the date, framed like
// an instant photo. The shot is saved as photos/team-YYYYMMDD-HHMMSS.png (3x)
// and the friends in it get a note.
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>
#endif
#include "global.h"
#include "mp.h"
#include "bg.h"
#include "event_data.h"
#include "data.h"
#include "field_screen_effect.h"
#include "field_weather.h"
#include "script.h"
#include "gpu_regs.h"
#include "main.h"
#include "menu.h"
#include "overworld.h"
#include "palette.h"
#include "region_map.h"
#include "scanline_effect.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "window.h"
#include "constants/characters.h"
#include "constants/rgb.h"
#include "constants/songs.h"

#define PHOTO_DIR  "photos"
#define PHOTO_SCALE 3
#define TAG_PIC    0x7310
#define BORDER     8
#define STRIP_TOP  120    // the caption strip under the picture
#define PIC_Y      72     // trainer picture centers

enum { WIN_FRAME, WIN_NOTE };
enum { PH_FADE_IN, PH_CHEESE, PH_CAPTION, PH_SHOOT, PH_FLASH, PH_DONE, PH_EXIT };

void NewGameBirchSpeech_LoadBackdrop(u8 bg);   // main_menu.c
void MP_AsciiToGame(u8 *dst, const char *src, int maxLen);
const u16 *Platform_GetFrame(void);
bool8 MP_Avatar_PlayerFront(s8 id, u8 *tiles, u16 *pal);   // mp_avatar.c
void MP_FieldClearVBlankHBlankCallbacks(void);
bool8 MP_WritePng(const char *path, const u8 *rgba, int w, int h, const char *text);   // mp_pack.c

static const struct BgTemplate sBgTemplates[] = {
    {.bg = 0, .charBaseIndex = 2, .mapBaseIndex = 31, .screenSize = 0, .paletteMode = 0, .priority = 0, .baseTile = 0},
    {.bg = 1, .charBaseIndex = 0, .mapBaseIndex = 7, .screenSize = 0, .paletteMode = 0, .priority = 3, .baseTile = 0},
};

static const struct WindowTemplate sWindows[] = {
    [WIN_FRAME] = {.bg = 0, .tilemapLeft = 0, .tilemapTop = 0, .width = 30, .height = 20, .paletteNum = 15, .baseBlock = 1},
    [WIN_NOTE]  = {.bg = 0, .tilemapLeft = 0, .tilemapTop = 0, .width = 30, .height = 2, .paletteNum = 14, .baseBlock = 1 + 600},
    DUMMY_WIN_TEMPLATE,
};

static const struct OamData sOamPic = {
    .shape = SPRITE_SHAPE(64x64), .size = SPRITE_SIZE(64x64), .priority = 1,
};

static EWRAM_DATA struct
{
    u8 state;
    u16 timer;
    u8 count;
    s8 ids[MP_MAX_PLAYERS];        // -1 = me
    u8 spriteIds[MP_MAX_PLAYERS];
    u8 names[MP_MAX_PLAYERS][PLAYER_NAME_LENGTH + 1];
    u8 place[32];
    char path[96];
    bool8 saved;
    time_t when;
} sPhoto = {0};

static u8 sPicTiles[MP_MAX_PLAYERS][TRAINER_PIC_SIZE];
static u16 sPicPals[MP_MAX_PLAYERS][16];

// ---------------------------------------------------------------------------
// Who is in it

// Friends on this map, in player order (everyone sees the same line-up)
static u8 GatherFriends(s8 *ids)
{
    u8 n = 0, i;
    for (i = 0; i < MP_MAX_PLAYERS; i++)
    {
        if (i == gMpLocalId)
            ids[n++] = -1;
        else if (MP_IsConnected() && gMpPlayers[i].present
              && gMpPlayers[i].state.mapGroup == gSaveBlock1Ptr->location.mapGroup
              && gMpPlayers[i].state.mapNum == gSaveBlock1Ptr->location.mapNum)
            ids[n++] = i;
    }
    if (gMpLocalId < 0)
        ids[n++] = -1;
    return n;
}

// ---------------------------------------------------------------------------
// The scene

static void VBlankCB_Photo(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void CB2_Photo(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    RunTextPrinters();
    UpdatePaletteFade();
    DoScheduledBgTilemapCopiesToVram();
}

static u8 NameWidth(const u8 *s)
{
    return GetStringWidth(FONT_SMALL, s, 0);
}

static s16 PicX(u8 i)
{
    s16 spacing = sPhoto.count >= 4 ? 52 : 60;
    return 120 + (2 * i - (sPhoto.count - 1)) * spacing / 2;
}

// The white frame, with "Say cheese!" or the caption in the strip
static void DrawFrame(bool8 caption)
{
    static const u8 sInk[] = {1, 2, 3};
    u8 win = WIN_FRAME, i, text[64];
    FillWindowPixelBuffer(win, PIXEL_FILL(0));
    FillWindowPixelRect(win, PIXEL_FILL(1), 0, 0, 240, BORDER);
    FillWindowPixelRect(win, PIXEL_FILL(1), 0, 0, BORDER, 160);
    FillWindowPixelRect(win, PIXEL_FILL(1), 240 - BORDER, 0, BORDER, 160);
    FillWindowPixelRect(win, PIXEL_FILL(1), 0, STRIP_TOP, 240, 160 - STRIP_TOP);
    if (!caption)
    {
        MP_AsciiToGame(text, "Everyone, say cheese!", sizeof(text));
        AddTextPrinterParameterized3(win, FONT_NORMAL, (240 - GetStringWidth(FONT_NORMAL, text, 0)) / 2, STRIP_TOP + 12, sInk, TEXT_SKIP_DRAW, text);
    }
    else
    {
        char date[32];
        struct tm *t = localtime(&sPhoto.when);
        static const char *const sMonths[] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};
        u8 line[64];
        for (i = 0; i < sPhoto.count; i++)
        {
            s16 x = PicX(i) - NameWidth(sPhoto.names[i]) / 2;
            if (x < BORDER)
                x = BORDER;
            AddTextPrinterParameterized3(win, FONT_SMALL, x, STRIP_TOP + 2, sInk, TEXT_SKIP_DRAW, sPhoto.names[i]);
        }
        snprintf(date, sizeof(date), "  %s %d, %d", t ? sMonths[t->tm_mon] : "", t ? t->tm_mday : 0, t ? t->tm_year + 1900 : 0);
        StringCopy(line, sPhoto.place);
        MP_AsciiToGame(text, date, sizeof(text));
        StringAppend(line, text);
        AddTextPrinterParameterized3(win, FONT_NORMAL, (240 - GetStringWidth(FONT_NORMAL, line, 0)) / 2, STRIP_TOP + 20, sInk, TEXT_SKIP_DRAW, line);
    }
    PutWindowTilemap(win);
    CopyWindowToVram(win, COPYWIN_FULL);
}

static void ShowNote(const char *ascii)
{
    static const u8 sInk[] = {1, 2, 3};
    u8 text[96];
    FillWindowPixelBuffer(WIN_NOTE, PIXEL_FILL(1));
    MP_AsciiToGame(text, ascii, sizeof(text));
    AddTextPrinterParameterized3(WIN_NOTE, FONT_SMALL, 4, 2, sInk, TEXT_SKIP_DRAW, text);
    PutWindowTilemap(WIN_NOTE);
    CopyWindowToVram(WIN_NOTE, COPYWIN_FULL);
}

static void CreatePics(void)
{
    u8 i;
    for (i = 0; i < sPhoto.count; i++)
    {
        struct SpriteSheet sheet = {sPicTiles[i], TRAINER_PIC_SIZE, TAG_PIC + i};
        struct SpritePalette pal = {sPicPals[i], TAG_PIC + i};
        struct SpriteTemplate tmpl = {
            .tileTag = TAG_PIC + i, .paletteTag = TAG_PIC + i, .oam = &sOamPic,
            .anims = gDummySpriteAnimTable, .images = NULL, .affineAnims = gDummySpriteAffineAnimTable,
            .callback = SpriteCallbackDummy,
        };
        MP_Avatar_PlayerFront(sPhoto.ids[i], sPicTiles[i], sPicPals[i]);
        LoadSpriteSheet(&sheet);
        LoadSpritePalette(&pal);
        sPhoto.spriteIds[i] = CreateSprite(&tmpl, PicX(i), PIC_Y, 0);
    }
}

// ---------------------------------------------------------------------------
// Saving

static void EnsureDir(void)
{
#ifdef _WIN32
    _mkdir(PHOTO_DIR);
#else
    mkdir(PHOTO_DIR, 0755);
#endif
}

static bool8 SaveShot(void)
{
    const u16 *frame = Platform_GetFrame();
    static u8 sRgba[DISPLAY_WIDTH * PHOTO_SCALE * DISPLAY_HEIGHT * PHOTO_SCALE * 4];
    struct tm *t = localtime(&sPhoto.when);
    int x, y, w = DISPLAY_WIDTH * PHOTO_SCALE;
    bool8 ok;

    if (!frame)
        return FALSE;
    for (y = 0; y < DISPLAY_HEIGHT * PHOTO_SCALE; y++)
    {
        for (x = 0; x < w; x++)
        {
            u16 c = frame[(y / PHOTO_SCALE) * DISPLAY_WIDTH + x / PHOTO_SCALE];
            u8 *p = &sRgba[(y * w + x) * 4];
            p[0] = ((c & 31) << 3) | ((c & 31) >> 2);
            p[1] = (((c >> 5) & 31) << 3) | (((c >> 5) & 31) >> 2);
            p[2] = (((c >> 10) & 31) << 3) | (((c >> 10) & 31) >> 2);
            p[3] = 255;
        }
    }
    EnsureDir();
    snprintf(sPhoto.path, sizeof(sPhoto.path), PHOTO_DIR "/team-%04d%02d%02d-%02d%02d%02d.png",
             t ? t->tm_year + 1900 : 0, t ? t->tm_mon + 1 : 0, t ? t->tm_mday : 0,
             t ? t->tm_hour : 0, t ? t->tm_min : 0, t ? t->tm_sec : 0);
    ok = MP_WritePng(sPhoto.path, sRgba, w, DISPLAY_HEIGHT * PHOTO_SCALE, "Moemon Emerald Kai Co-op team photo");
    printf("[photo] %s %s\n", ok ? "saved" : "could not save", sPhoto.path);
    fflush(stdout);
    return ok;
}

// The friends in the picture hear about it
static void TellFriends(void)
{
    u8 i, buf[2] = {MPR_PHOTO, 0};
    buf[1] = sPhoto.count;
    for (i = 0; i < sPhoto.count; i++)
        if (sPhoto.ids[i] >= 0)
            MP_SendRelay(sPhoto.ids[i], buf, sizeof(buf));
}

void MP_Photo_OnRelay(u8 from, const u8 *data, u16 len)
{
    u8 text[64], name[PLAYER_NAME_LENGTH + 1];
    static const u8 sTook[] = _(" took a team photo with you!");
    if (from >= MP_MAX_PLAYERS || len < 2 || !gMpPlayers[from].present)
        return;
    StringCopyN(name, gMpPlayers[from].name, PLAYER_NAME_LENGTH);
    name[PLAYER_NAME_LENGTH] = EOS;
    StringCopy(text, name);
    StringAppend(text, sTook);
    MP_Toast(text);
    printf("[mp] team photo by player %d\n", from);
}

// ---------------------------------------------------------------------------

static void FreeScene(void)
{
    u8 i;
    for (i = 0; i < sPhoto.count; i++)
    {
        if (sPhoto.spriteIds[i] < MAX_SPRITES)
            DestroySprite(&gSprites[sPhoto.spriteIds[i]]);
        FreeSpriteTilesByTag(TAG_PIC + i);
        FreeSpritePaletteByTag(TAG_PIC + i);
    }
    FreeAllWindowBuffers();
}

static void Task_UnlockAfterFade(u8 taskId)
{
    if (IsWeatherNotFadingIn() == TRUE)
    {
        DestroyTask(taskId);
        UnlockPlayerFieldControls();
    }
}

static void FieldCB_ReturnFromPhoto(void)
{
    LockPlayerFieldControls();
    FadeInFromBlack();
    CreateTask(Task_UnlockAfterFade, 10);
}

static void Task_Photo(u8 taskId)
{
    switch (sPhoto.state)
    {
    case PH_FADE_IN:
        if (!gPaletteFade.active)
        {
            sPhoto.timer = 0;
            sPhoto.state = PH_CHEESE;
        }
        break;
    case PH_CHEESE:
        if (++sPhoto.timer >= 75)
        {
            DrawFrame(TRUE);
            sPhoto.timer = 0;
            sPhoto.state = PH_CAPTION;
        }
        break;
    case PH_CAPTION:
        // A few frames so the caption is on the screen being saved
        if (++sPhoto.timer >= 4)
            sPhoto.state = PH_SHOOT;
        break;
    case PH_SHOOT:
        sPhoto.saved = SaveShot();
        PlaySE(MUS_RG_PHOTO);
        BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_WHITE);
        TellFriends();
        sPhoto.state = PH_FLASH;
        break;
    case PH_FLASH:
        if (!gPaletteFade.active)
        {
            char note[128];
            if (sPhoto.saved)
                snprintf(note, sizeof(note), "Saved to %s   {A} OK", sPhoto.path);
            else
                snprintf(note, sizeof(note), "Couldn't save the photo.   {A} OK");
            ShowNote(note);
            sPhoto.state = PH_DONE;
        }
        break;
    case PH_DONE:
        if (JOY_NEW(A_BUTTON | B_BUTTON))
        {
            PlaySE(SE_SELECT);
            BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
            sPhoto.state = PH_EXIT;
        }
        break;
    case PH_EXIT:
        if (!gPaletteFade.active)
        {
            DestroyTask(taskId);
            FreeScene();
            SetGpuReg(REG_OFFSET_DISPCNT, 0);
            Overworld_ResetMapMusic();
            Overworld_PlaySpecialMapMusic();
            MP_FieldClearVBlankHBlankCallbacks();
            gFieldCallback = FieldCB_ReturnFromPhoto;
            SetMainCallback2(CB2_ReturnToField);
        }
        break;
    }
}

static void CB2_InitPhoto(void)
{
    u8 i;
    SetVBlankCallback(NULL);
    SetGpuReg(REG_OFFSET_DISPCNT, 0);
    ResetBgsAndClearDma3BusyFlags(0);
    InitBgsFromTemplates(0, sBgTemplates, ARRAY_COUNT(sBgTemplates));
    DmaClearLarge16(3, (void *)VRAM, VRAM_SIZE, 0x1000);
    DmaClear32(3, OAM, OAM_SIZE);
    DmaClear16(3, PLTT, PLTT_SIZE);
    ResetPaletteFade();
    ScanlineEffect_Stop();
    ResetSpriteData();
    FreeAllSpritePalettes();
    ResetTasks();
    NewGameBirchSpeech_LoadBackdrop(1);
    SetGpuReg(REG_OFFSET_BG1HOFS, 16);       // the stage in the middle
    SetGpuReg(REG_OFFSET_BG1VOFS, -12 & 0x1FF);
    InitWindows(sWindows);
    DeactivateAllTextPrinters();
    LoadPalette(gStandardMenuPalette, BG_PLTT_ID(15), PLTT_SIZE_4BPP);
    LoadPalette(gStandardMenuPalette, BG_PLTT_ID(14), PLTT_SIZE_4BPP);
    gPlttBufferUnfaded[BG_PLTT_ID(15) + 1] = gPlttBufferFaded[BG_PLTT_ID(15) + 1] = RGB(31, 31, 29);   // photo paper
    SetGpuReg(REG_OFFSET_BLDCNT, 0);
    SetGpuReg(REG_OFFSET_WININ, 0);
    SetGpuReg(REG_OFFSET_WINOUT, 0);
    SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
    ShowBg(0);
    ShowBg(1);
    for (i = 0; i < MP_MAX_PLAYERS; i++)
        sPhoto.spriteIds[i] = MAX_SPRITES;
    CreatePics();
    DrawFrame(FALSE);
    sPhoto.state = PH_FADE_IN;
    CreateTask(Task_Photo, 80);
    BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
    SetVBlankCallback(VBlankCB_Photo);
    SetMainCallback2(CB2_Photo);
}

static void Task_EnterPhoto(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        DestroyTask(taskId);
        CleanupOverworldWindowsAndTilemaps();
        SetMainCallback2(CB2_InitPhoto);
    }
}

// From SOCIAL: NULL = ready for a photo, otherwise why not (ASCII)
const char *MP_Photo_Check(void)
{
    s8 ids[MP_MAX_PLAYERS];
    if (GatherFriends(ids) < 2)
        return "Your friends need to be on this map too.";
    return NULL;
}

void MP_Photo_Start(void)
{
    u8 i;
    if (MP_Photo_Check())
        return;
    sPhoto.count = GatherFriends(sPhoto.ids);
    for (i = 0; i < sPhoto.count; i++)
    {
        if (sPhoto.ids[i] < 0)
            StringCopyN(sPhoto.names[i], gSaveBlock2Ptr->playerName, PLAYER_NAME_LENGTH);
        else
            StringCopyN(sPhoto.names[i], gMpPlayers[sPhoto.ids[i]].name, PLAYER_NAME_LENGTH);
        sPhoto.names[i][PLAYER_NAME_LENGTH] = EOS;
    }
    GetMapName(sPhoto.place, gMapHeader.regionMapSectionId, 0);
    sPhoto.when = time(NULL);
    LockPlayerFieldControls();
    FadeScreen(FADE_TO_BLACK, 0);
    CreateTask(Task_EnterPhoto, 80);
}

// Test harness condition "photo": waiting on the OK after the shot
bool8 MP_Photo_Done(void)
{
    return FindTaskIdByFunc(Task_Photo) != TASK_NONE && sPhoto.state == PH_DONE;
}
