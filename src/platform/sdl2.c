#ifdef PLATFORM_SDL2
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
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
#include "platform/display.h"
#include "platform/display_sdl.h"
#include "platform/framedraw.h"
#include "platform/system.h"
#include "platform/gamepad.h"

extern void (*const gIntrTable[])(void);

SDL_Thread *mainLoopThread;
// Moemon Kai PC: the window, renderer and scaling live in platform/display.c
SDL_sem *vBlankSemaphore;
SDL_atomic_t isFrameAvailable;
bool speedUp = false;
#define SPEED_UP_SCALE 5.0
static void UpdateTimeScale(void);
// Moemon Kai PC: game controllers (src/platform/gamepad.c)
void Gamepad_Init(void);
int Gamepad_HandleEvent(const SDL_Event *e);
u16 Gamepad_Poll(int *fastForward);
static bool sPadFastForward;
bool isRunning = true;
bool paused = false;
double simTime = 0;
double lastGameTime = 0;
double curGameTime = 0;
double fixedTimestep = 1.0 / 60.0; // 16.666667ms
double timeScale = 1.0;
bool8 MP_Qol_SkipActive(void);
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
// waitfor / pressuntil: wait for a named game state (see MP_TestCondition)
static char sUntilName[32];
static int sUntilFrames = 0;
static u16 sUntilKeys = 0;
extern int MP_TestCondition(const char *name);
static u16 sMashKeys = 0;
static unsigned long sFrameCounter = 0;
static char sShotDir[400] = ".";
static u16 ParseTestKeys(const char *s);
static void TestHarnessStep(void);
static bool SpectateStep(void);
void Platform_SaveScreenshot(const char *name);

extern void AgbMain(void);
extern void MP_Init(void);
extern void MP_FrameUpdate(void);
extern void MainLoop(void);
extern void DoSoftReset(void);

int DoMain(void *param);
void ProcessEvents(void);
void VDraw(void);

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

    Display_PreInit(); // loads moemon_display.ini, sets hints that must precede SDL_Init

    if(SDL_Init(SDL_INIT_VIDEO | (sTestMode ? 0 : SDL_INIT_AUDIO)) < 0)
    {
        DBGPRINTF("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    if (Display_Init("Moemon Emerald Kai Co-op") != 0)
        return 1;
    {
        void Gamepad_Init(void);
        if (!sTestMode)
            Gamepad_Init();
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

    while (isRunning)
    {
        double deltaTime;
        bool lockstep = false;

        ProcessEvents();

        curGameTime = SDL_GetPerformanceCounter();
        deltaTime = (double)((curGameTime - lastGameTime) / (double)SDL_GetPerformanceFrequency());
        // Moemon Co-op QoL: holding START in a script seen before plays it at 4x
        // Small tweak (AllowSmallTweaks = 1): typing "speed" plays at 5x until "slow"
        timeScale = MP_Qol_SkipActive() ? 4.0 : (speedUp ? SPEED_UP_SCALE : 1.0);
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
            if (SpectateStep())
            {
                AudioUpdate();
                sFrameCounter++;
                lastGameTime = curGameTime;
                continue;
            }
            ENTER_VBLANK();
            MainLoop();
            VDraw();
            RunDMAsAndVBlank();
            AudioUpdate();
            sFrameCounter++;
            lastGameTime = curGameTime;
            continue;
        }

        if (!paused)
        {
            // Frame pacing. With vsync on a ~60/120/144Hz monitor the loop runs once per
            // refresh; then exactly one game frame every N refreshes is perfectly smooth
            // (an accumulator would sometimes run 0 or 2 frames and stutter). If vsync
            // doesn't really wait, or the game hitched, fall back to the accumulator.
            static double sAvgDelta = 1.0 / 60.0;
            static int sRefresh, sRefreshCheck, sRefreshPhase;
            double realDelta = deltaTime / timeScale;
            int steps, perGameFrame = 0, i;

            sAvgDelta = sAvgDelta * 0.95 + (realDelta < 0.1 ? realDelta : 0.1) * 0.05;
            if (sRefreshCheck-- <= 0)
            {
                sRefresh = Display_GetRefreshRate();
                sRefreshCheck = 120;
            }
            if (Display_GetVSync() && timeScale == 1.0 && sRefresh >= 55)
            {
                int n = (sRefresh + 30) / 60;
                if (n >= 1 && n <= 4 && abs(sRefresh - 60 * n) <= 2
                 && sAvgDelta > 0.8 / sRefresh && sAvgDelta < 1.25 / sRefresh)
                    perGameFrame = n;
            }

            if (perGameFrame && realDelta < 2.5 / 60.0)
            {
                lockstep = true;
                steps = (++sRefreshPhase >= perGameFrame) ? 1 : 0;
                if (steps)
                    sRefreshPhase = 0;
                accumulator = 0;
            }
            else
            {
                accumulator += deltaTime;
                steps = (int)(accumulator / fixedTimestep);
                if (steps > 8)   // way behind (window dragged, PC busy): don't try to catch up
                {
                    steps = 8;
                    accumulator = steps * fixedTimestep;
                }
                accumulator -= steps * fixedTimestep;
            }

            // MOEMON_FPS_LOG=1: print game frames per second (pacing check)
            {
                static int sLog = -1, sCount;
                static Uint64 sLast;
                if (sLog < 0)
                    sLog = getenv("MOEMON_FPS_LOG") != NULL;
                if (sLog)
                {
                    Uint64 now = SDL_GetPerformanceCounter();
                    sCount += steps;
                    if (sLast == 0)
                        sLast = now;
                    if (now - sLast >= SDL_GetPerformanceFrequency())
                    {
                        printf("[fps] %d game frames/s (refresh %d, lockstep %d)\n", sCount, sRefresh, perGameFrame);
                        sCount = 0;
                        sLast = now;
                    }
                }
            }
            for (i = 0; i < steps; i++)
            {
                //run game logic, draw frame and process DMAs and vblank
                MP_FrameUpdate();
                if (SpectateStep())
                    continue;
                ENTER_VBLANK(); //you must be in VBlank before running a game tick
                MainLoop();
                if (i == steps - 1)   // only the frame that will be shown is drawn
                    VDraw();
                RunDMAsAndVBlank();
            }

            //samples per frame is 701, that gets multipled by two when being queued and then multipled by four because samples are float32 which are 4 bytes long hence the divide by 8
            //this number is then checked against samples per frame multipled by three rounded down to 2000 to give it enough margin of error while not desyncing
            //this is all done to sync audio to gameplay
            if (SDL_GetQueuedAudioSize(1)/8 < 2000)
            {
                AudioUpdate();
            }

        }

        lastGameTime = curGameTime;

        // Present only when there is something new (a game frame, a resize or a
        // settings change). With vsync on, presenting waits for the monitor;
        // otherwise sleep briefly while the next game frame is still >2ms away.
        if (Display_NeedsPresent() || lockstep)   // lockstep: every refresh is presented, so vsync paces the loop
            Display_Present();
        else if (paused || (fixedTimestep - accumulator) / timeScale > 0.002)
            SDL_Delay(1);
    }

    CloseSaveFile();

    Display_SaveSettings();
    Display_Shutdown();
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

// Moemon Co-op: keyboard typing (src/platform/text_input.c)
void Platform_TextInputEvent(const char *utf8);
int Platform_TextInputKeyDown(int sym, int mod);

void ProcessEvents(void)
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        if (Display_HandleEvent(&event)) // F11 / Alt+Enter fullscreen, resizes
            continue;
        if (Gamepad_HandleEvent(&event)) // controllers plugged in / out
            continue;
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
            }
            break;
        case SDL_TEXTINPUT:
            Platform_TextInputEvent(event.text.text);
            break;
        case SDL_DROPFILE:
            // Moemon Co-op: drop a sprite sheet onto the window to wear it
            {
                extern void MP_OnFileDropped(const char *path);
                MP_OnFileDropped(event.drop.file);
                SDL_free(event.drop.file);
            }
            break;
        case SDL_KEYDOWN:
            if (Platform_TextInputKeyDown(event.key.keysym.sym, event.key.keysym.mod))
                break;
            if (event.key.keysym.sym >= SDLK_a && event.key.keysym.sym <= SDLK_z && !event.key.repeat
             && !(event.key.keysym.mod & (KMOD_CTRL | KMOD_ALT | KMOD_GUI)))
            {
                void Platform_PushLetter(char c);
                Platform_PushLetter('a' + (event.key.keysym.sym - SDLK_a));
            }
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
            case SDLK_c:
                // Moemon Co-op: the CO-OP menu (Y/Triangle on a controller)
                if (!(event.key.keysym.mod & (KMOD_CTRL | KMOD_ALT)) && !event.key.repeat
                 && Platform_InputContext() == INPUT_CONTEXT_OVERWORLD)
                    Platform_ContextAction(INPUT_ACTION_COOP_MENU);
                break;
            case SDLK_1: case SDLK_2: case SDLK_3: case SDLK_4:
            case SDLK_5: case SDLK_6: case SDLK_7: case SDLK_8:
                // Moemon Co-op: emotes (1-4) and your phrases (5-8)
                if (!(event.key.keysym.mod & (KMOD_CTRL | KMOD_ALT)) && !event.key.repeat
                 && Platform_InputContext() == INPUT_CONTEXT_OVERWORLD)
                    Platform_ContextAction(INPUT_ACTION_EMOTE_KEY1 + (event.key.keysym.sym - SDLK_1));
                break;
            }
            break;
        }
    }
}


static void UpdateTimeScale(void)
{
    // Moemon Co-op: no fast forward button (typing "speed" is a small tweak, see Platform_SetSpeedUp)
    timeScale = speedUp ? SPEED_UP_SCALE : 1.0;
}

// Typing "speed" (only with AllowSmallTweaks = 1 in moemon_coop.ini) / "slow"
static int sSpeedNotice;   // 1 on, 2 off: shown by the game (Platform_TakeSpeedNotice)
void Platform_SetSpeedUp(int on)
{
    bool8 MP_SmallTweaksAllowed(void);
    if (on && !MP_SmallTweaksAllowed())
        return;
    if ((bool)on == speedUp)
        return;
    speedUp = on != 0;
    UpdateTimeScale();
    sSpeedNotice = on ? 1 : 2;
}

int Platform_TakeSpeedNotice(void)
{
    int n = sSpeedNotice;
    sSpeedNotice = 0;
    return n;
}

// Q / E held on the keyboard: the quick emote / phrase pick (like LT / RT). It opens
// after a moment, so typing words with Q or E in them doesn't pop it up.
#define QUICK_KEY_DELAY 8
static void KeyboardQuickHolds(void)
{
    static int sHeldFrames[2];
    static const SDL_Scancode sKeys[2] = {SDL_SCANCODE_Q, SDL_SCANCODE_E};
    const Uint8 *state = SDL_GetKeyboardState(NULL);
    int i;
    for (i = 0; i < 2; i++)
    {
        int held = state[sKeys[i]] && !(SDL_GetModState() & (KMOD_CTRL | KMOD_ALT | KMOD_GUI));
        sHeldFrames[i] = held ? sHeldFrames[i] + 1 : 0;
        Gamepad_SetKeyHold(i, sHeldFrames[i] >= QUICK_KEY_DELAY);
        if (sHeldFrames[i] == QUICK_KEY_DELAY && Platform_InputContext() == INPUT_CONTEXT_OVERWORLD)
            Platform_ContextAction(i ? INPUT_ACTION_QUICK_PHRASES : INPUT_ACTION_QUICK_EMOTES);
    }
}

u16 Platform_GetKeyInput(void)
{
    int ff = 0;
    u16 padKeys;
    if (sTestMode)
    {
        int Gamepad_TestActive(void);
        if (!Gamepad_TestActive())
            return sTestKeys;
        SDL_JoystickUpdate();
        return sTestKeys | Gamepad_Poll(&ff);
    }
    padKeys = Gamepad_Poll(&ff);
    KeyboardQuickHolds();
    if ((bool)ff != sPadFastForward)
    {
        sPadFastForward = ff;
        UpdateTimeScale();
    }
    return keys | padKeys;
}

void VDraw(void)
{
    uint16_t *image = sFrameImage;

    memset(image, 0, sizeof(sFrameImage));
    DrawFrame(image);
    if (!sTestMode)
        Display_UploadFrame(image);
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

const u16 *Platform_GetFrame(void)
{
    return sFrameImage;
}

// Moemon Co-op: while watching another player's battle the game is paused and
// the received feed is shown instead. Returns TRUE when this frame was handled.
static bool SpectateStep(void)
{
    extern bool8 MP_Spectate_IsWatching(void);
    extern const u16 *MP_Spectate_WatchFrame(u16 keys);
    const u16 *feed;
    if (!MP_Spectate_IsWatching())
        return false;
    feed = MP_Spectate_WatchFrame(Platform_GetKeyInput());
    if (!feed)
        return false;   // stopped watching: run the game this frame
    memcpy(sFrameImage, feed, sizeof(sFrameImage));
    if (!sTestMode)
        Display_UploadFrame(sFrameImage);
    return true;
}

bool8 Platform_IsTestMode(void)
{
    return sTestMode;
}

static void TestHarnessStep(void)
{
    char line[256];

    if (sUntilName[0])
    {
        if (MP_TestCondition(sUntilName))
        {
            sUntilName[0] = 0;
            sTestKeys = 0;
            printf("[test] frame %lu: reached state\n", sFrameCounter);
            fflush(stdout);
        }
        else
        {
            if (--sUntilFrames <= 0)
            {
                printf("[test] frame %lu: TIMEOUT waiting for %s\n", sFrameCounter, sUntilName);
                fflush(stdout);
                sUntilName[0] = 0;
            }
            else if (sUntilKeys)
            {
                // tap the keys every 45 frames
                int phase = sUntilFrames % 45;
                sTestKeys = (phase < 3) ? sUntilKeys : 0;
            }
            return;
        }
    }
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
        else if (strcmp(cmd, "waitfor") == 0 || strcmp(cmd, "pressuntil") == 0)
        {
            // waitfor NAME [MAXFRAMES]   /   pressuntil KEYS NAME [MAXFRAMES]
            char name[32] = "";
            int maxFrames = 3000;
            if (strcmp(cmd, "waitfor") == 0)
            {
                sscanf(line, "%*s %31s %d", name, &maxFrames);
                sUntilKeys = 0;
            }
            else
            {
                sscanf(line, "%*s %*s %31s %d", name, &maxFrames);
                sUntilKeys = ParseTestKeys(arg);
            }
            snprintf(sUntilName, sizeof(sUntilName), "%s", name);
            sUntilFrames = maxFrames;
            return;
        }
        else if (strcmp(cmd, "letters") == 0)
        {
            // letters WORD - as if those letter keys were pressed
            void Platform_PushLetter(char c);
            const char *src = line + 8;
            for (; *src >= 'a' && *src <= 'z'; src++)
                Platform_PushLetter(*src);
        }
        else if (strcmp(cmd, "type") == 0)
        {
            // type TEXT   (\n = Enter, \b = Backspace)
            void Platform_QueueTypedText(const char *text);
            char text[200];
            const char *src = line + 5;
            int n = 0;
            while (*src && *src != '\n' && *src != '\r' && n < 199)
            {
                if (src[0] == '\\' && src[1] == 'n') { text[n++] = '\n'; src += 2; }
                else if (src[0] == '\\' && src[1] == 'b') { text[n++] = '\b'; src += 2; }
                else text[n++] = *src++;
            }
            text[n] = 0;
            Platform_QueueTypedText(text);
        }
        else if (strcmp(cmd, "mp") == 0)
        {
            // mp COMMAND [ARG]: game-side test commands (src/mp_debug.c)
            extern void MP_TestCommand(const char *cmd, const char *arg);
            char a2[200] = "";
            sscanf(line, "%*s %*s %199[^\n]", a2);   // the rest of the line
            MP_TestCommand(arg, a2);
        }
        else if (strcmp(cmd, "rendershot") == 0)
        {
            // The real window output (scaling + filter), not just the 240x160 frame
            char path[1024];
            snprintf(path, sizeof(path), "%s/%s.bmp", sShotDir, arg);
            Display_UploadFrame(sFrameImage);
            printf("[test] frame %lu: rendershot %s %s\n", sFrameCounter, path,
                   Display_SaveOutputBMP(path) == 0 ? "ok" : "FAILED");
            fflush(stdout);
        }
        else if (strcmp(cmd, "winsize") == 0)
        {
            int w = atoi(arg), h = n;
            Display_TestSetWindowSize(w, h);
        }
        else if (strcmp(cmd, "display") == 0)
        {
            // display fullscreen|scale|mode|filter|vsync|toggle|save VALUE
            Display_TestCommand(arg, n);
            printf("[test] frame %lu: display %s %d\n", sFrameCounter, arg, n);
            fflush(stdout);
        }
        else if (strcmp(cmd, "vpad") == 0 || strcmp(cmd, "vbtn") == 0 || strcmp(cmd, "vaxis") == 0 || strcmp(cmd, "vuse") == 0)
        {
            // virtual controller: vpad [nintendo] / vbtn south|east|west|north|start|lb|... 0|1 / vaxis lx|ly|rx|ry|lt|rt -100..100
            void Gamepad_TestCommand(const char *cmd, const char *arg, int value);
            int v = 0;
            sscanf(line, "%*s %*s %d", &v);
            Gamepad_TestCommand(cmd, arg, v);
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