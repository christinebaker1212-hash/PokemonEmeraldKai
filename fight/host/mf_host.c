/*
 * Moemon Fight: the engine inside the game (include/mf_engine.h).
 *
 * The engine runs on its own thread, as the original ran on its main thread: it starts up once
 * (System_Init, the select list), then waits for sessions. A session is the part of the original
 * game flow from the match setup to the victory screen (character select, versus screen,
 * match), started without the title screen in VS mode (two players) or arcade mode (player 1
 * against the CPU, one match). Every frame of the screen loops ends in MfHost_FrameEnd, where
 * the engine waits for the game to step it with both players' buttons; its clock moves one frame
 * per step. Files are read under the mugen/ folder (MOEMON_MUGEN_DIR), and a fatal error ends the
 * engine's thread instead of the program.
 */
#include <setjmp.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>
#ifndef _WIN32
#include <dirent.h>
#include <strings.h>
#include <sys/stat.h>
#endif

#include "mugen.h"
#include "mf_compat.h"
#include "../../include/mf_engine.h"

/* the real C library calls, which mf_port.h redirects for the engine */
#undef fopen
#undef remove
#undef exit

/* ------------------------------------------------------------------ state */

enum {
    HOST_OFF,           /* the thread hasn't started */
    HOST_LOADING,       /* System_Init */
    HOST_READY,         /* waiting for a session */
    HOST_RUNNING,       /* a session runs */
    HOST_DEAD           /* stopped on an error */
};

static pthread_mutex_t sLock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t sCond = PTHREAD_COND_INITIALIZER;
static pthread_t sThread;
static pthread_t sEngineThread;     /* set by the thread itself (sThread may not be written yet) */
static volatile int sState = HOST_OFF;
static int sSessionPending;
static int sActive;                 /* a session was asked for and hasn't ended */
static struct MfSession sSession;
static int sResult = MF_RESULT_NONE;
static int sCancel;
static unsigned long sGranted, sDone; /* frames the engine may complete / has completed */
static unsigned short sButtons[2];
static unsigned short sHeld[2];     /* the buttons the engine's keys are down for */
static char sError[0x1000];

static jmp_buf sExitJump;
static int sExitJumpReady;

static unsigned short sFrames[2][MF_SCREEN_W * MF_SCREEN_H];
static int sFront;                  /* index of the frame the game shows */
static int sHaveFrame;
static int sPresented;              /* the back frame has a new picture */
static volatile unsigned int sFrameSerial;

static char sRoot[0x200];           /* "<mugen folder>/" */
static int sSyncSteps;              /* MOEMON_FIGHT_SYNC: MfEngine_Step waits for the frame (tests) */

/* The engine's clock: MfTime_* ticks per second, one frame per step. */
#define HOST_CLOCK_FREQ 60000
static long long sClock;

/* ------------------------------------------------------------------ files */

static void Host_InitRoot(void)
{
    const char *env = getenv("MOEMON_MUGEN_DIR");
    size_t n;

    snprintf(sRoot, sizeof(sRoot), "%s", env != NULL && env[0] != '\0' ? env : "mugen");
    n = strlen(sRoot);
    if (n > 0 && sRoot[n - 1] != '/' && sRoot[n - 1] != '\\' && n + 1 < sizeof(sRoot)) {
        sRoot[n] = '/';
        sRoot[n + 1] = '\0';
    }
}

static int Path_IsAbsolute(const char *path)
{
    if (path[0] == '/' || path[0] == '\\') {
        return 1;
    }
    return path[0] != '\0' && path[1] == ':';
}

#ifndef _WIN32
/* Linux test builds: the pack's paths assume Windows (backslashes, any letter case). Finds each part
 * of path case-insensitively; returns 1 with the real path in out. */
static int Path_ResolveCase(const char *path, char *out, size_t size)
{
    char work[0x400];
    char *part, *save = NULL;
    size_t len;

    snprintf(work, sizeof(work), "%s", path);
    out[0] = '\0';
    if (work[0] == '/') {
        snprintf(out, size, "/");
    }
    for (part = strtok_r(work, "/", &save); part != NULL; part = strtok_r(NULL, "/", &save)) {
        char candidate[0x400];
        DIR *dir;
        struct dirent *e;
        int found = 0;

        len = strlen(out);
        snprintf(candidate, sizeof(candidate), "%s%s", out, part);
        if (access(candidate, F_OK) == 0) {
            snprintf(out + len, size - len, "%s/", part);
            continue;
        }
        dir = opendir(len > 0 ? out : ".");
        if (dir == NULL) {
            return 0;
        }
        while ((e = readdir(dir)) != NULL) {
            if (strcasecmp(e->d_name, part) == 0) {
                snprintf(out + len, size - len, "%s/", e->d_name);
                found = 1;
                break;
            }
        }
        closedir(dir);
        if (!found) {
            return 0;
        }
    }
    len = strlen(out);
    if (len > 1 && out[len - 1] == '/') {
        out[len - 1] = '\0';
    }
    return 1;
}
#endif

/* The real path of an engine path: under sRoot unless absolute. */
static void Path_Full(const char *path, char *out, size_t size)
{
    char *p;

    if (Path_IsAbsolute(path)) {
        snprintf(out, size, "%s", path);
    } else {
        snprintf(out, size, "%s%s", sRoot, path);
    }
#ifndef _WIN32
    for (p = out; *p != '\0'; p++) {
        if (*p == '\\') {
            *p = '/';
        }
    }
#else
    (void)p;
#endif
}

FILE *MfFile_Open(const char *path, const char *mode)
{
    char full[0x400];
    FILE *fp;

    if (path == NULL || path[0] == '\0') {
        return NULL;
    }
    Path_Full(path, full, sizeof(full));
#ifndef _WIN32
    if (mode[0] == 'r') {
        /* as on Windows: no folders, and any letter case */
        char real[0x400];
        struct stat st;
        if (stat(full, &st) != 0) {
            if (!Path_ResolveCase(full, real, sizeof(real)) || stat(real, &st) != 0) {
                return NULL;
            }
            snprintf(full, sizeof(full), "%s", real);
        }
        if (S_ISDIR(st.st_mode)) {
            return NULL;
        }
    }
#endif
    fp = fopen(full, mode);
    return fp;
}

int MfFile_Remove(const char *path)
{
    char full[0x400];

    Path_Full(path, full, sizeof(full));
    return remove(full);
}

int MfFile_Chdir(const char *path)
{
    return 0;   /* the game's working directory stays */
}

char *MfFile_Getcwd(char *buf, int size)
{
    if (buf == NULL || size <= 0) {
        return NULL;
    }
    snprintf(buf, (size_t)size, "%s", sRoot);
    return buf;
}

/* ------------------------------------------------------------------ the clock */

int MfTime_Counter(LARGE_INTEGER *count)
{
    count->QuadPart = sClock;
    return 1;
}

int MfTime_Frequency(LARGE_INTEGER *freq)
{
    freq->QuadPart = HOST_CLOCK_FREQ;
    return 1;
}

unsigned int MfTime_Ms(void)
{
    return (unsigned int)(sClock * 1000 / HOST_CLOCK_FREQ);
}

/* ------------------------------------------------------------------ errors */

void MfHost_ErrorDialog(const char *text, const char *title)
{
    snprintf(sError, sizeof(sError), "%s", text != NULL ? text : "");
    fprintf(stderr, "[fight] %s\n%s\n", title != NULL ? title : "", sError);
    /* Error_Exit frees the game variables next: the BGM thread must not outlive them */
    if (g_bgmInitialized) {
        Bgm_Deinit();
    }
}

void MfHost_Exit(int code)
{
    if (sExitJumpReady && pthread_equal(pthread_self(), sEngineThread)) {
        longjmp(sExitJump, code != 0 ? code : 1);
    }
    fprintf(stderr, "[fight] the engine exited (%d) outside its thread\n", code);
    abort();
}

/* ------------------------------------------------------------------ settings */

/* The keys of the engine's built-in key maps (system.c input_builtin_keys), in MF_BTN_* order. */
static const int sPlayerKeys[2][11] = {
    { SDLK_UP, SDLK_DOWN, SDLK_LEFT, SDLK_RIGHT, SDLK_COMMA, SDLK_PERIOD, SDLK_SLASH, SDLK_l, SDLK_SEMICOLON,
      SDLK_QUOTE, SDLK_RETURN },
    { SDLK_w, SDLK_s, SDLK_a, SDLK_d, SDLK_f, SDLK_g, SDLK_h, SDLK_r, SDLK_t, SDLK_y, SDLK_u },
};
static const int sKeySlots[11] = {
    KEY_JUMP, KEY_CROUCH, KEY_LEFT, KEY_RIGHT, KEY_A, KEY_B, KEY_C, KEY_X, KEY_Y, KEY_Z, KEY_START,
};

void MfHost_ConfigLoaded(void)
{
    GameVars *g = g_game;
    BgmPluginEntry *plugin;
    DynArray *plugins;

    /* the picture: the software renderer, 480x320 */
    g->render_mode = RENDER_SYSTEM;
    g->depth = 16;
    g->video_flags = 0;
    g->video_width = MF_SCREEN_W;
    g->video_height = MF_SCREEN_H;
    g->stage_fit = 0;
    g->system_fit = 0;
    g->safe_mode = 0;
    g->game_width = MF_SCREEN_W;
    g->game_height = MF_SCREEN_H;
    g->game_coord[0] = MF_SCREEN_W;
    g->game_coord[1] = MF_SCREEN_H;
    g->game_coord[2] = (MF_SCREEN_W - 1) / 2;
    g->game_coord[3] = (MF_SCREEN_H - 1) / 2;
    memcpy(g_gameCoord, g->game_coord, sizeof(g_gameCoord));
    g->game_speed = 60;

    /* the sound: GBA rate, mono sound effects, every song through the "moemon" plugin */
    g->sound_enabled = 1;
    g->sample_rate = MF_MIX_RATE;
    g->channels = 2;
    g->bit_depth = 16;
    g->stereo_effects = 0;
    g->sfx_resample_method = SND_RESAMPLE_LIBRESAMPLE;
    g->bgm_resample_method = SND_RESAMPLE_LIBRESAMPLE;
    g->bgm_disabled = 0;
    plugins = calloc(1, sizeof(DynArray));
    plugin = calloc(1, sizeof(BgmPluginEntry));
    if (plugins != NULL && plugin != NULL) {
        plugins->data = calloc(BGM_ARRAY_CAPACITY, sizeof(void *));
        plugins->elem_size = sizeof(void *);
        plugins->capacity = BGM_ARRAY_CAPACITY;
        plugin->name = strdup("moemon");
        if (plugins->data != NULL && plugin->name != NULL) {
            ((BgmPluginEntry **)plugins->data)[0] = plugin;
            plugins->count = 1;
            g->music_plugins = plugins;    /* the pack's list is left as it was read */
        }
    }

    g->nojoy = 1;
    g->precache = 0;            /* characters load on the engine's thread, where errors are caught */

    /* no debug keys, no first-run box */
    g->debug = 0;
    g->allow_debug_mode = 0;
    g->first_run = 0;
}

void MfHost_KeysLoaded(void)
{
    GameVars *g = g_game;
    int p, i;

    /* the players' buttons arrive as the built-in keys, whatever the pack's [P1 Keys] say; no joysticks */
    for (p = 0; p < 2; p++) {
        for (i = 0; i < 11; i++) {
            g->keys[p][sKeySlots[i]] = sPlayerKeys[p][i];
        }
        g->option_input[p][1] = 0;
        g->use_joystick[p] = 0;
        g->joystick_type[p] = 0;
    }
}

/* ------------------------------------------------------------------ the screenpack's place */

int g_mfMotifOffsetX;
int g_mfHudShift;

/* The motif fits the picture whole (by height for 4:3 packs in 3:2), centred: nothing of it is cut off or
 * squished (docs/FIGHT_DECISIONS.md #5). */
void MfHost_FitMotif(void)
{
    GameVars *g = g_game;
    double sx = g->game_coord[0] / g->motif_coord[0];
    double sy = g->game_coord[1] / g->motif_coord[1];
    double s = sx < sy ? sx : sy;

    g->motif_scale = s;
    g_mfMotifOffsetX = (int)floor((g->game_coord[0] - g->motif_coord[0] * s) / 2.0 + 0.5);
    g_mfHudShift = 0;
}

/* ------------------------------------------------------------------ frames */

void MfHost_Present(const SDL_Surface *surface)
{
    unsigned short *dst = sFrames[sFront ^ 1];
    int w, h, y;

    if (surface == NULL || surface->format->BytesPerPixel != 2) {
        return;
    }
    w = surface->w < MF_SCREEN_W ? surface->w : MF_SCREEN_W;
    h = surface->h < MF_SCREEN_H ? surface->h : MF_SCREEN_H;
    if (w < MF_SCREEN_W || h < MF_SCREEN_H) {
        memset(dst, 0, sizeof(sFrames[0]));
    }
    for (y = 0; y < h; y++) {
        memcpy(dst + y * MF_SCREEN_W, (const unsigned char *)surface->pixels + y * surface->pitch, (size_t)w * 2);
    }
    sPresented = 1;
}

/* Key events for the buttons that changed (engine thread). */
static void Host_PushButtons(const unsigned short buttons[2])
{
    int p, i;

    for (p = 0; p < 2; p++) {
        unsigned short changed = (unsigned short)(buttons[p] ^ sHeld[p]);
        for (i = 0; i < 11; i++) {
            if (changed & (1 << i)) {
                MfSdl_PushKey(sPlayerKeys[p][i], (buttons[p] >> i) & 1);
            }
        }
        sHeld[p] = buttons[p];
    }
}

void MfHost_FrameEnd(void)
{
    unsigned short buttons[2];
    int cancel;

    pthread_mutex_lock(&sLock);
    if (sPresented) {
        sFront ^= 1;
        sHaveFrame = 1;
        sPresented = 0;
        sFrameSerial++;
    }
    sDone++;
    pthread_cond_broadcast(&sCond);
    while (sDone >= sGranted && !sCancel) {
        pthread_cond_wait(&sCond, &sLock);
    }
    buttons[0] = sButtons[0];
    buttons[1] = sButtons[1];
    cancel = sCancel;
    pthread_mutex_unlock(&sLock);

    sClock += g_timerFrameTicks > 0 ? g_timerFrameTicks : HOST_CLOCK_FREQ / 60;
    if (cancel) {
        /* escape ends any screen; nothing else is held */
        buttons[0] = buttons[1] = 0;
        Host_PushButtons(buttons);
        MfSdl_PushKey(SDLK_ESCAPE, 1);
        MfSdl_PushKey(SDLK_ESCAPE, 0);
    } else {
        Host_PushButtons(buttons);
    }
    g_game->skip_draw = 0;
}

/* ------------------------------------------------------------------ sessions */

/* The cabinet's rules from mugen/cabinets.ini (missing values keep the pack's). */
static void Host_ApplyCabinet(const char *cabinet)
{
    GameVars *g = g_game;
    Ini *ini = Ini_Open("cabinets.ini");
    int v;

    if (ini == NULL) {
        return;
    }
    if (Ini_SelectGroup(ini, cabinet) != -1) {
        if (Ini_GetInt(ini, "rounds", &v) && v >= 1 && v <= 9) {
            g->fight.round.match_wins = v;
        }
        if (Ini_GetInt(ini, "life", &v) && v >= 30 && v <= 300) {
            g->life = v;
        }
        if (Ini_GetInt(ini, "time", &v) && (v == -1 || (v >= 20 && v <= 99))) {
            g->time = v;
        }
        if (Ini_GetInt(ini, "speed", &v) && v >= -9 && v <= 9) {
            g->option_game_speed = v;
        }
        if (Ini_GetInt(ini, "cpu.level", &v) && v >= 1 && v <= 8) {
            g->difficulty = v;
        }
    }
    Ini_Free(ini);
}

static int Host_RunSession(const struct MfSession *s)
{
    GameVars *g = g_game;
    int state = 6;
    int result = MF_RESULT_CANCELLED;

    Host_ApplyCabinet(s->cabinet[0] != '\0' ? s->cabinet : "mauville_gc");
    g = g_game;
    g->game_mode = s->mode == MF_MODE_CPU ? GAME_MODE_ARCADE : GAME_MODE_VERSUS;
    g->game_mode_side = 0;
    g->demo_mode = 0;
    g->quick_vs = 0;
    Rng_Seed((int)(s->seed & 0x7FFFFFFE) + 1);
    while (!sCancel) {
        switch (state) {
        case 6:
            state = Gameflow_SetupMatch();
            break;
        case 7:
            state = Charsel_Run();
            if (state != 8) {
                return MF_RESULT_CANCELLED;
            }
            break;
        case 8:
            Gameflow_PickStage();
            state = 10;
            break;
        case 10:
            state = Versus_Run();
            if (state != 11) {
                return MF_RESULT_CANCELLED;
            }
            break;
        case 11:
            state = Match_Run();
            g = g_game;
            if (state == 11) {
                break;      /* the match is played again */
            }
            if (state == 6 || !g->match_has_result) {
                return MF_RESULT_CANCELLED;
            }
            result = g->match_winner == 1 ? MF_RESULT_P1 : g->match_winner == 2 ? MF_RESULT_P2 : MF_RESULT_DRAW;
            if (state != 12) {
                return result;      /* an arcade loss: no continue screen */
            }
            break;
        case 12:
            Victory_Run();
            return result;
        default:
            return result;
        }
    }
    return MF_RESULT_CANCELLED;
}

/* What the original main() does before the game flow, and the select list (game flow step 2). */
static void Host_StartEngine(void)
{
    static char arg0[] = "mugen";
    char *argv[] = { arg0, NULL };

    g_cmdline = calloc(1, sizeof(CmdLineArgs));
    if (g_cmdline == NULL) {
        MfHost_ErrorDialog("Out of memory", "M.U.G.E.N Error");
        longjmp(sExitJump, 1);
    }
    Log_Init();
    CmdLine_Parse(1, argv);
    System_Init();
    Gfx_SetRenderTarget(g_game->screen, 0);
    g_gfxClear(0);
    Gfx_SetClipRect(0, 0, g_targetWidth, g_targetHeight);
    Timer_Reset();
    Timer_SetFps(60);
    g_game->start_ticks = (int)Timer_GetTicks();
    Select_Init();
}

static void *Host_Thread(void *arg)
{
    sEngineThread = pthread_self();
    if (setjmp(sExitJump) != 0) {
        pthread_mutex_lock(&sLock);
        sState = HOST_DEAD;
        sActive = 0;
        sSessionPending = 0;
        sResult = MF_RESULT_ERROR;
        if (sError[0] == '\0') {
            snprintf(sError, sizeof(sError), "The fight engine stopped.");
        }
        pthread_cond_broadcast(&sCond);
        pthread_mutex_unlock(&sLock);
        return NULL;
    }
    sExitJumpReady = 1;
    Host_StartEngine();
    for (;;) {
        struct MfSession session;
        int result;

        pthread_mutex_lock(&sLock);
        sState = HOST_READY;
        pthread_cond_broadcast(&sCond);
        while (!sSessionPending) {
            pthread_cond_wait(&sCond, &sLock);
        }
        sSessionPending = 0;
        sState = HOST_RUNNING;
        session = sSession;
        pthread_mutex_unlock(&sLock);

        MfSdl_ClearEvents();
        sHeld[0] = sHeld[1] = 0;
        result = Host_RunSession(&session);
        MfMix_Reset();

        pthread_mutex_lock(&sLock);
        sResult = sCancel ? MF_RESULT_CANCELLED : result;
        sActive = 0;
        sCancel = 0;
        pthread_cond_broadcast(&sCond);
        pthread_mutex_unlock(&sLock);
    }
    return NULL;
}

/* ------------------------------------------------------------------ the game's side */

int MfEngine_Begin(const struct MfSession *session)
{
    pthread_mutex_lock(&sLock);
    if (sState == HOST_DEAD || sActive) {
        pthread_mutex_unlock(&sLock);
        return 0;
    }
    if (sState == HOST_OFF) {
        pthread_attr_t attr;
        Host_InitRoot();
        sSyncSteps = getenv("MOEMON_FIGHT_SYNC") != NULL;
        pthread_attr_init(&attr);
        pthread_attr_setstacksize(&attr, 16 << 20);
        sState = HOST_LOADING;
        if (pthread_create(&sThread, &attr, Host_Thread, NULL) != 0) {
            sState = HOST_DEAD;
            snprintf(sError, sizeof(sError), "Couldn't start the fight engine.");
            pthread_attr_destroy(&attr);
            pthread_mutex_unlock(&sLock);
            return 0;
        }
        pthread_attr_destroy(&attr);
    }
    sSession = *session;
    sSessionPending = 1;
    sActive = 1;
    sCancel = 0;
    sResult = MF_RESULT_NONE;
    sGranted = sDone;
    sButtons[0] = sButtons[1] = 0;
    sHaveFrame = 0;
    pthread_cond_broadcast(&sCond);
    pthread_mutex_unlock(&sLock);
    return 1;
}

int MfEngine_IsActive(void)
{
    return sActive;
}

void MfEngine_Step(const unsigned short buttons[2])
{
    struct timespec until;

    pthread_mutex_lock(&sLock);
    if (!sActive) {
        pthread_mutex_unlock(&sLock);
        return;
    }
    sButtons[0] = buttons[0];
    sButtons[1] = buttons[1];
    if (sGranted > sDone && !sSyncSteps) {
        /* the engine is still busy with the last step (loading) */
        pthread_mutex_unlock(&sLock);
        return;
    }
    if (sGranted <= sDone) {
        sGranted = sDone + 1;
        pthread_cond_broadcast(&sCond);
    }
    if (sSyncSteps) {
        /* tests: every step is a whole frame, however long the engine takes */
        while (sActive && sDone < sGranted) {
            pthread_cond_wait(&sCond, &sLock);
        }
        pthread_mutex_unlock(&sLock);
        return;
    }
    clock_gettime(CLOCK_REALTIME, &until);
    until.tv_nsec += 100 * 1000000L;
    if (until.tv_nsec >= 1000000000L) {
        until.tv_sec++;
        until.tv_nsec -= 1000000000L;
    }
    while (sActive && sDone < sGranted) {
        if (pthread_cond_timedwait(&sCond, &sLock, &until) == ETIMEDOUT) {
            break;
        }
    }
    pthread_mutex_unlock(&sLock);
}

const unsigned short *MfEngine_Frame(void)
{
    return sHaveFrame ? sFrames[sFront] : NULL;
}

unsigned int MfEngine_FrameSerial(void)
{
    return sFrameSerial;
}

int MfEngine_Result(void)
{
    return sActive ? MF_RESULT_NONE : sResult;
}

void MfEngine_Cancel(void)
{
    pthread_mutex_lock(&sLock);
    if (sActive) {
        sCancel = 1;
        if (sSessionPending) {
            /* not started yet */
            sSessionPending = 0;
            sActive = 0;
            sResult = MF_RESULT_CANCELLED;
            sCancel = 0;
        }
        pthread_cond_broadcast(&sCond);
    }
    pthread_mutex_unlock(&sLock);
}

const char *MfEngine_Error(void)
{
    return sError;
}

void MfEngine_MixAudio(short *stereo, int frames, int rate)
{
    if (sState == HOST_RUNNING || sState == HOST_READY) {
        MfMix_Render(stereo, frames, rate);
    }
}
