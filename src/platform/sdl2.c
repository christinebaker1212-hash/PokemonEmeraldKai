#ifdef PLATFORM_SDL2
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#include <xinput.h>
#endif

#include <SDL2/SDL.h>

#include "global.h"
#include "platform.h"
#include "rtc.h"
#include "gba/defines.h"
#include "gba/m4a_internal.h"
#include "cgb_audio.h"
#include "gba/flash_internal.h"
#include "platform/dma.h"
#include "platform/framedraw.h"
#include "platform/system.h"

extern void (*const gIntrTable[])(void);

SDL_Thread *mainLoopThread;
SDL_Window *sdlWindow;
SDL_Renderer *sdlRenderer;
SDL_Texture *sdlTexture;
SDL_sem *vBlankSemaphore;
SDL_atomic_t isFrameAvailable;
bool speedUp = false;
unsigned int videoScale = 3;
bool videoScaleChanged = false;
bool isRunning = true;
bool paused = false;
double simTime = 0;
double lastGameTime = 0;
double curGameTime = 0;
double fixedTimestep = 1.0 / 60.0; // 16.666667ms
double timeScale = 1.0;
struct SiiRtcInfo internalClock;

static FILE *sSaveFile = NULL;
char gPlatformSavePath[512] = "pokeemerald.sav";
static uint16_t sFrameImage[DISPLAY_WIDTH * DISPLAY_HEIGHT];

// ---- Automated test harness (MOEMON_TEST=<script>) ----
// Script lines: "wait N", "press KEYS N" (KEYS like A, B, START, UP, A+UP), "shot NAME", "quit",
// "log TEXT". Lines starting with '#' are comments. In test mode the game runs unthrottled.
static FILE *sTestScript = NULL;
static bool sTestMode = false;
static u16 sTestKeys = 0;
static int sTestWait = 0;
static int sMashCount = 0, sMashInterval = 0;
static u16 sMashKeys = 0;
static unsigned long sFrameCounter = 0;
static char sShotDir[400] = ".";
static u16 ParseTestKeys(const char *s);
static void TestHarnessStep(void);
void Platform_SaveScreenshot(const char *name);

extern void AgbMain(void);
extern void MP_Init(void);
extern void MP_FrameUpdate(void);
extern void MainLoop(void);
extern void DoSoftReset(void);

int DoMain(void *param);
void ProcessEvents(void);
void VDraw(SDL_Texture *texture);

static void ReadSaveFile(char *path);
static void StoreSaveFile(void);
static void CloseSaveFile(void);

static void UpdateInternalClock(void);

int main(int argc, char **argv)
{
    // Open an output console on Windows
#ifdef _WIN32
    AllocConsole() ;
    AttachConsole( GetCurrentProcessId() ) ;
    freopen( "CON", "w", stdout ) ;
#endif

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--save") == 0 && i + 1 < argc)
            snprintf(gPlatformSavePath, sizeof(gPlatformSavePath), "%s", argv[++i]);
    }
    if (getenv("MOEMON_SAVE"))
        snprintf(gPlatformSavePath, sizeof(gPlatformSavePath), "%s", getenv("MOEMON_SAVE"));
    if (getenv("MOEMON_SHOTDIR"))
        snprintf(sShotDir, sizeof(sShotDir), "%s", getenv("MOEMON_SHOTDIR"));
    if (getenv("MOEMON_TEST"))
    {
        sTestScript = fopen(getenv("MOEMON_TEST"), "r");
        sTestMode = (sTestScript != NULL);
        if (!sTestMode)
            printf("Could not open test script %s\n", getenv("MOEMON_TEST"));
    }

    ReadSaveFile(gPlatformSavePath);

    if(SDL_Init(SDL_INIT_VIDEO | (sTestMode ? 0 : SDL_INIT_AUDIO)) < 0)
    {
        DBGPRINTF("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    sdlWindow = SDL_CreateWindow("Moemon Emerald Kai Co-op", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, DISPLAY_WIDTH * videoScale, DISPLAY_HEIGHT * videoScale, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (sdlWindow == NULL)
    {
        DBGPRINTF("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_PRESENTVSYNC);
    if (sdlRenderer == NULL)
    {
        DBGPRINTF("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 255, 255);
    SDL_RenderClear(sdlRenderer);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
    SDL_RenderSetLogicalSize(sdlRenderer, DISPLAY_WIDTH, DISPLAY_HEIGHT);

    sdlTexture = SDL_CreateTexture(sdlRenderer,
                                   SDL_PIXELFORMAT_ABGR1555,
                                   SDL_TEXTUREACCESS_STREAMING,
                                   DISPLAY_WIDTH, DISPLAY_HEIGHT);
    if (sdlTexture == NULL)
    {
        DBGPRINTF("Texture could not be created! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    simTime = curGameTime = lastGameTime = SDL_GetPerformanceCounter();

    isFrameAvailable.value = 0;
    vBlankSemaphore = SDL_CreateSemaphore(0);

    SDL_AudioSpec want;

    SDL_memset(&want, 0, sizeof(want)); /* or SDL_zero(want) */
    want.freq = 42048;
    want.format = AUDIO_F32;
    want.channels = 2;
    want.samples = 1024;
    cgb_audio_init(want.freq);


    if (sTestMode)
        ;
    else if (SDL_OpenAudio(&want, 0) < 0)
        SDL_Log("Failed to open audio: %s", SDL_GetError());
    else
    {
        if (want.format != AUDIO_F32) /* we let this one thing change. */
            SDL_Log("We didn't get Float32 audio format.");
        SDL_PauseAudio(0);
    }

    AgbMain();
    MP_Init();

    double accumulator = 0.0;

    memset(&internalClock, 0, sizeof(internalClock));
    internalClock.status = SIIRTCINFO_24HOUR;
    UpdateInternalClock();

    bool isGameStepDrawn = false;
    while (isRunning)
    {
        double deltaTime;

        ProcessEvents();

        curGameTime = SDL_GetPerformanceCounter();
        deltaTime = (double)((curGameTime - lastGameTime) / (double)SDL_GetPerformanceFrequency());
        deltaTime *= timeScale; //apply speedup

        if (sTestMode)
        {
            // Unthrottled deterministic stepping for automated tests
            {
                static double sTestFps = -1;
                static Uint64 sNextTick;
                if (sTestFps < 0)
                    sTestFps = getenv("MOEMON_TEST_FPS") ? atof(getenv("MOEMON_TEST_FPS")) : 0;
                if (sTestFps > 0)
                {
                    Uint64 now = SDL_GetPerformanceCounter();
                    Uint64 step = (Uint64)(SDL_GetPerformanceFrequency() / sTestFps);
                    if (sNextTick == 0)
                        sNextTick = now;
                    while (SDL_GetPerformanceCounter() < sNextTick)
                        SDL_Delay(1);
                    sNextTick += step;
                }
            }
            TestHarnessStep();
            MP_FrameUpdate();
            ENTER_VBLANK();
            MainLoop();
            VDraw(sdlTexture);
            RunDMAsAndVBlank();
            AudioUpdate();
            sFrameCounter++;
            lastGameTime = curGameTime;
            continue;
        }

        if (!paused)
        {
            accumulator += deltaTime;

            isGameStepDrawn = false;

            while (accumulator >= fixedTimestep)
            {
                //run game logic, draw frame and process DMAs and vblank
                MP_FrameUpdate();
                ENTER_VBLANK(); //you must be in VBlank before running a game tick
                MainLoop();
                if (!isGameStepDrawn)
                {
                    VDraw(sdlTexture);
                    //SDL_RenderClear(sdlRenderer);
                    isGameStepDrawn = true;
                }
                RunDMAsAndVBlank();

                accumulator -= fixedTimestep;
            }

            //samples per frame is 701, that gets multipled by two when being queued and then multipled by four because samples are float32 which are 4 bytes long hence the divide by 8
            //this number is then checked against samples per frame multipled by three rounded down to 2000 to give it enough margin of error while not desyncing
            //this is all done to sync audio to gameplay
            if (SDL_GetQueuedAudioSize(1)/8 < 2000)
            {
                AudioUpdate();
            }

            if (videoScaleChanged)
            {
                SDL_SetWindowSize(sdlWindow, DISPLAY_WIDTH * videoScale, DISPLAY_HEIGHT * videoScale);
                videoScaleChanged = false;
            }
        }

        lastGameTime = curGameTime;

        SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
        SDL_RenderPresent(sdlRenderer);
    }

    CloseSaveFile();

    SDL_DestroyWindow(sdlWindow);
    SDL_Quit();
    return 0;
}

static void ReadSaveFile(char *path)
{
    // Check whether the saveFile exists, and create it if not
    sSaveFile = fopen(path, "r+b");
    if (sSaveFile == NULL)
    {
        sSaveFile = fopen(path, "w+b");
    }

    fseek(sSaveFile, 0, SEEK_END);
    int fileSize = ftell(sSaveFile);
    fseek(sSaveFile, 0, SEEK_SET);

    // Only read as many bytes as fit inside the buffer
    // or as many bytes as are in the file
    int bytesToRead = (fileSize < sizeof(FLASH_BASE)) ? fileSize : sizeof(FLASH_BASE);

    int bytesRead = fread(FLASH_BASE, 1, bytesToRead, sSaveFile);

    // Fill the buffer if the savefile was just created or smaller than the buffer itself
    for (int i = bytesRead; i < sizeof(FLASH_BASE); i++)
    {
        FLASH_BASE[i] = 0xFF;
    }
}

static void StoreSaveFile()
{
    if (sSaveFile != NULL)
    {
        fseek(sSaveFile, 0, SEEK_SET);
        fwrite(FLASH_BASE, 1, sizeof(FLASH_BASE), sSaveFile);
    }
}

void Platform_StoreSaveFile(void)
{
    StoreSaveFile();
}

void Platform_ReadFlash(u16 sectorNum, u32 offset, u8 *dest, u32 size)
{
    DBGPRINTF("ReadFlash(sectorNum=0x%04X,offset=0x%08X,size=0x%02X)\n",sectorNum,offset,size);
    FILE * savefile = fopen(gPlatformSavePath, "r+b");
    if (savefile == NULL)
    {
        puts("Error opening save file.");
        return;
    }
    if (fseek(savefile, (sectorNum << gFlash->sector.shift) + offset, SEEK_SET))
    {
        fclose(savefile);
        return;
    }
    if (fread(dest, 1, size, savefile) != size)
    {
        fclose(savefile);
        return;
    }
    fclose(savefile);
}

void Platform_QueueAudio(float *audioBuffer, s32 samplesPerFrame)
{
    SDL_QueueAudio(1, audioBuffer, samplesPerFrame);
}


static void CloseSaveFile()
{
    if (sSaveFile != NULL)
    {
        fclose(sSaveFile);
    }
}

// Key mappings
#define KEY_A_BUTTON      SDLK_z
#define KEY_B_BUTTON      SDLK_x
#define KEY_START_BUTTON  SDLK_RETURN
#define KEY_SELECT_BUTTON SDLK_BACKSLASH
#define KEY_L_BUTTON      SDLK_a
#define KEY_R_BUTTON      SDLK_s
#define KEY_DPAD_UP       SDLK_UP
#define KEY_DPAD_DOWN     SDLK_DOWN
#define KEY_DPAD_LEFT     SDLK_LEFT
#define KEY_DPAD_RIGHT    SDLK_RIGHT

#define HANDLE_KEYUP(key) \
case KEY_##key:  keys &= ~key; break;

#define HANDLE_KEYDOWN(key) \
case KEY_##key:  keys |= key; break;

static u16 keys;

void ProcessEvents(void)
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT:
            isRunning = false;
            break;
        case SDL_KEYUP:
            switch (event.key.keysym.sym)
            {
            HANDLE_KEYUP(A_BUTTON)
            HANDLE_KEYUP(B_BUTTON)
            HANDLE_KEYUP(START_BUTTON)
            HANDLE_KEYUP(SELECT_BUTTON)
            HANDLE_KEYUP(L_BUTTON)
            HANDLE_KEYUP(R_BUTTON)
            HANDLE_KEYUP(DPAD_UP)
            HANDLE_KEYUP(DPAD_DOWN)
            HANDLE_KEYUP(DPAD_LEFT)
            HANDLE_KEYUP(DPAD_RIGHT)
            case SDLK_SPACE:
                if (speedUp)
                {
                    speedUp = false;
                    timeScale = 1.0;
                    //SDL_ClearQueuedAudio(1);
                    //SDL_PauseAudio(0);
                }
                break;
            }
            break;
        case SDL_KEYDOWN:
            switch (event.key.keysym.sym)
            {
            HANDLE_KEYDOWN(A_BUTTON)
            HANDLE_KEYDOWN(B_BUTTON)
            HANDLE_KEYDOWN(START_BUTTON)
            HANDLE_KEYDOWN(SELECT_BUTTON)
            HANDLE_KEYDOWN(L_BUTTON)
            HANDLE_KEYDOWN(R_BUTTON)
            HANDLE_KEYDOWN(DPAD_UP)
            HANDLE_KEYDOWN(DPAD_DOWN)
            HANDLE_KEYDOWN(DPAD_LEFT)
            HANDLE_KEYDOWN(DPAD_RIGHT)
            case SDLK_r:
                if (event.key.keysym.mod & (KMOD_LCTRL | KMOD_RCTRL))
                {
                    DoSoftReset();
                }
                break;
            case SDLK_p:
                if (event.key.keysym.mod & (KMOD_LCTRL | KMOD_RCTRL))
                {
                    paused = !paused;
                }
                break;
            case SDLK_SPACE:
                if (!speedUp)
                {
                    speedUp = true;
                    timeScale = 5.0;
                    //SDL_PauseAudio(1);
                }
                break;
            }
            break;
        case SDL_WINDOWEVENT:
            if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
            {
                unsigned int w = event.window.data1;
                unsigned int h = event.window.data2;
                
                videoScale = 0;
                if (w / DISPLAY_WIDTH > videoScale)
                    videoScale = w / DISPLAY_WIDTH;
                if (h / DISPLAY_HEIGHT > videoScale)
                    videoScale = h / DISPLAY_HEIGHT;
                if (videoScale < 1)
                    videoScale = 1;

                videoScaleChanged = true;
            }
            break;
        }
    }
}

#ifdef _WIN32
#define STICK_THRESHOLD 0.5f
u16 GetXInputKeys()
{
    XINPUT_STATE state;
    ZeroMemory(&state, sizeof(XINPUT_STATE));

    DWORD dwResult = XInputGetState(0, &state);
    u16 xinputKeys = 0;

    if (dwResult == ERROR_SUCCESS)
    {
        /* A */      xinputKeys |= (state.Gamepad.wButtons & XINPUT_GAMEPAD_A) >> 12;
        /* B */      xinputKeys |= (state.Gamepad.wButtons & XINPUT_GAMEPAD_X) >> 13;
        /* Start */  xinputKeys |= (state.Gamepad.wButtons & XINPUT_GAMEPAD_START) >> 1;
        /* Select */ xinputKeys |= (state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) >> 3;
        /* L */      xinputKeys |= (state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) << 1;
        /* R */      xinputKeys |= (state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) >> 1;
        /* Up */     xinputKeys |= (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) << 6;
        /* Down */   xinputKeys |= (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) << 6;
        /* Left */   xinputKeys |= (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) << 3;
        /* Right */  xinputKeys |= (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) << 1;


        /* Control Stick */
        float xAxis = (float)state.Gamepad.sThumbLX / (float)SHRT_MAX;
        float yAxis = (float)state.Gamepad.sThumbLY / (float)SHRT_MAX;

        if (xAxis < -STICK_THRESHOLD) xinputKeys |= DPAD_LEFT;
        if (xAxis >  STICK_THRESHOLD) xinputKeys |= DPAD_RIGHT;
        if (yAxis < -STICK_THRESHOLD) xinputKeys |= DPAD_DOWN;
        if (yAxis >  STICK_THRESHOLD) xinputKeys |= DPAD_UP;


        /* Speedup */
        // Note: 'speedup' variable is only (un)set on keyboard input
        double oldTimeScale = timeScale;
        timeScale = (state.Gamepad.bRightTrigger > 0x80 || speedUp) ? 5.0 : 1.0;

        if (oldTimeScale != timeScale)
        {
            if (timeScale > 1.0)
            {
                SDL_PauseAudio(1);
            }
            else
            {
                SDL_ClearQueuedAudio(1);
                SDL_PauseAudio(0);
            }
        }
    }

    return xinputKeys;
}
#endif // _WIN32

u16 Platform_GetKeyInput(void)
{
    if (sTestMode)
        return sTestKeys;
#ifdef _WIN32
    u16 gamepadKeys = GetXInputKeys();
    return (gamepadKeys != 0) ? gamepadKeys : keys;
#endif

    return keys;
}

void VDraw(SDL_Texture *texture)
{
    uint16_t *image = sFrameImage;

    memset(image, 0, sizeof(sFrameImage));
    DrawFrame(image);
    if (!sTestMode)
        SDL_UpdateTexture(texture, NULL, image, DISPLAY_WIDTH * sizeof (Uint16));
    REG_VCOUNT = 161; // prep for being in VBlank period
}

int DoMain(void *data)
{
    AgbMain();
}

void VBlankIntrWait(void)
{
    return;
}

u8 BinToBcd(u8 bin)
{
    int placeCounter = 1;
    u8 out = 0;
    do
    {
        out |= (bin % 10) * placeCounter;
        placeCounter *= 16;
    }
    while ((bin /= 10) > 0);

    return out;
}

void Platform_GetStatus(struct SiiRtcInfo *rtc)
{
    rtc->status = internalClock.status;
}

void Platform_SetStatus(struct SiiRtcInfo *rtc)
{
    internalClock.status = rtc->status;
}

static void UpdateInternalClock(void)
{
    time_t rawTime = time(NULL);
    struct tm *time = localtime(&rawTime);

    internalClock.year = BinToBcd(time->tm_year - 100);
    internalClock.month = BinToBcd(time->tm_mon + 1);
    internalClock.day = BinToBcd(time->tm_mday);
    internalClock.dayOfWeek = BinToBcd(time->tm_wday);
    internalClock.hour = BinToBcd(time->tm_hour);
    internalClock.minute = BinToBcd(time->tm_min);
    internalClock.second = BinToBcd(time->tm_sec);
}

void Platform_GetDateTime(struct SiiRtcInfo *rtc)
{
    UpdateInternalClock();

    rtc->year = internalClock.year;
    rtc->month = internalClock.month;
    rtc->day = internalClock.day;
    rtc->dayOfWeek = internalClock.dayOfWeek;
    rtc->hour = internalClock.hour;
    rtc->minute = internalClock.minute;
    rtc->second = internalClock.second;
    DBGPRINTF("GetDateTime: %d-%02d-%02d %02d:%02d:%02d\n", ConvertBcdToBinary(rtc->year),
                                                         ConvertBcdToBinary(rtc->month),
                                                         ConvertBcdToBinary(rtc->day),
                                                         ConvertBcdToBinary(rtc->hour),
                                                         ConvertBcdToBinary(rtc->minute),
                                                         ConvertBcdToBinary(rtc->second));
}

void Platform_SetDateTime(struct SiiRtcInfo *rtc)
{
    internalClock.month = rtc->month;
    internalClock.day = rtc->day;
    internalClock.dayOfWeek = rtc->dayOfWeek;
    internalClock.hour = rtc->hour;
    internalClock.minute = rtc->minute;
    internalClock.second = rtc->second;
}

void Platform_GetTime(struct SiiRtcInfo *rtc)
{
    UpdateInternalClock();

    rtc->hour = internalClock.hour;
    rtc->minute = internalClock.minute;
    rtc->second = internalClock.second;
    DBGPRINTF("GetTime: %02d:%02d:%02d\n", ConvertBcdToBinary(rtc->hour),
                                        ConvertBcdToBinary(rtc->minute),
                                        ConvertBcdToBinary(rtc->second));
}

void Platform_SetTime(struct SiiRtcInfo *rtc)
{
    internalClock.hour = rtc->hour;
    internalClock.minute = rtc->minute;
    internalClock.second = rtc->second;
}

void Platform_SetAlarm(u8 *alarmData)
{
    // TODO
}

static u16 ParseTestKeys(const char *s)
{
    static const struct { const char *name; u16 key; } names[] = {
        {"A", A_BUTTON}, {"B", B_BUTTON}, {"START", START_BUTTON}, {"SELECT", SELECT_BUTTON},
        {"L", L_BUTTON}, {"R", R_BUTTON}, {"UP", DPAD_UP}, {"DOWN", DPAD_DOWN},
        {"LEFT", DPAD_LEFT}, {"RIGHT", DPAD_RIGHT}, {"NONE", 0},
    };
    u16 keysOut = 0;
    char buf[64];
    snprintf(buf, sizeof(buf), "%s", s);
    for (char *tok = strtok(buf, "+"); tok; tok = strtok(NULL, "+"))
        for (unsigned i = 0; i < sizeof(names) / sizeof(names[0]); i++)
            if (strcmp(tok, names[i].name) == 0)
                keysOut |= names[i].key;
    return keysOut;
}

void Platform_SaveScreenshot(const char *name)
{
    char path[1024];
    SDL_Surface *surf = SDL_CreateRGBSurfaceWithFormatFrom(sFrameImage, DISPLAY_WIDTH, DISPLAY_HEIGHT, 16,
                                                           DISPLAY_WIDTH * 2, SDL_PIXELFORMAT_ABGR1555);
    snprintf(path, sizeof(path), "%s/%s.bmp", sShotDir, name);
    if (surf)
    {
        SDL_SaveBMP(surf, path);
        SDL_FreeSurface(surf);
    }
    printf("[test] frame %lu: screenshot %s\n", sFrameCounter, path);
    fflush(stdout);
}

bool8 Platform_IsTestMode(void)
{
    return sTestMode;
}

static void TestHarnessStep(void)
{
    char line[256];

    if (sTestWait > 0)
    {
        --sTestWait;
        if (sMashCount > 0 && sTestWait == sMashInterval - 2)
            sTestKeys = 0;
        if (sTestWait == 0 && sMashCount == 0)
            sTestKeys = 0;
        if (sTestWait > 0)
            return;
    }
    if (sMashCount > 0)
    {
        if (--sMashCount > 0)
        {
            sTestKeys = sMashKeys;
            sTestWait = sMashInterval;
            return;
        }
    }
    sTestKeys = 0;
    while (fgets(line, sizeof(line), sTestScript))
    {
        char cmd[32] = "", arg[200] = "";
        int n = 1;
        if (line[0] == '#' || line[0] == '\n')
            continue;
        if (sscanf(line, "%31s %199s %d", cmd, arg, &n) < 1)
            continue;
        if (strcmp(cmd, "wait") == 0)
        {
            sTestWait = atoi(arg);
            return;
        }
        else if (strcmp(cmd, "press") == 0)
        {
            sTestKeys = ParseTestKeys(arg);
            sTestWait = n > 0 ? n : 1;
            return;
        }
        else if (strcmp(cmd, "mash") == 0)
        {
            // mash KEYS COUNT [INTERVAL]
            int interval = 30;
            sscanf(line, "%*s %*s %d %d", &n, &interval);
            sMashKeys = ParseTestKeys(arg);
            sMashCount = n;
            sMashInterval = interval < 3 ? 3 : interval;
            sTestKeys = sMashKeys;
            sTestWait = sMashInterval;
            return;
        }
        else if (strcmp(cmd, "shot") == 0)
        {
            Platform_SaveScreenshot(arg);
        }
        else if (strcmp(cmd, "log") == 0)
        {
            printf("[test] frame %lu: %s", sFrameCounter, line + 4);
            fflush(stdout);
        }
        else if (strcmp(cmd, "quit") == 0)
        {
            isRunning = false;
            return;
        }
    }
    // End of script
    isRunning = false;
}

void SoftReset(u32 resetFlags)
{
    puts("Soft Reset called. Exiting.");
    exit(0);
}

#endif