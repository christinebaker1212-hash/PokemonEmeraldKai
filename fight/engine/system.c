/*
 * Start-up and shutdown of the whole program (original module
 * 0x41A500-0x41E46F): SDL, game variables, configuration, timer, input,
 * sound, BGM, video, fonts, options, motif, Lua and console; the input key
 * tables, the fight.def loader and the window focus handlers.
 */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <shlobj.h>
#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_syswm.h>
#include <SDL_ttf.h>
#define HAVE_STRUCT_TIMESPEC 1      /* the CRT's time.h already has it */
#include <pthread.h>

#include "mugen.h"

#define WINDOW_ICON_ID 101     /* icon resource in mugen.exe */

/* 0x0041A500: gives the SDL window the program icon. */
void System_SetWindowIcon(void)
{
    /* Moemon Fight: the game's window keeps its own icon. */
}

/* 0x0041E260: g_onActivate callback. */
void System_OnActivate(int gained)
{
    if (gained) {
        System_OnFocusGained(gained);
    } else {
        System_OnFocusLost(gained);
    }
}

/*
 * 0x0041AE10: creates the debug console. mugen.cfg is parsed and dropped
 * again (nothing is read from it), then the console visibility is toggled.
 */
int Console_Init(void)
{
    Stream *s;
    Ini *ini;

    if (!Console_Create()) {
        Error_Printf("Could not init console\n");
        return 0;
    }
    s = Stream_OpenFile("data/mugen.cfg", "rt");
    if (s != NULL) {
        ini = Ini_Parse(s, INI_MODE_READ);
        Stream_Close(s);
        if (ini != NULL) {
            Ini_Free(ini);
            if (g_console != NULL) {
                g_console->lua_mode = g_console->lua_mode == 0;
            }
            return 1;
        }
    }
    Error_Printf("Can't open %s\n", "data/mugen.cfg");
    return 0;
}

/* Makes the start-up directory end in exactly one '/' (a trailing '\\' is dropped). */
static void System_NormalizeCwd(char *cwd)
{
    if (cwd[(int)strlen(cwd) - 1] == '\\') {
        cwd[(int)strlen(cwd) - 1] = '\0';
    }
    if (cwd[(int)strlen(cwd) - 1] != '/') {
        strcat(cwd, "/");
    }
}

/* 0x0041A560: any failure ends in Error_Exit (or exit(1) before the error clipboard exists). */
void System_Init(void)
{
    CmdLineArgs *args = g_cmdline;
    char fonts[0x200] = { 0 };
    GameVars *g;
    int mode;

    SDL_putenv("SDL_DISABLE_LOCK_KEYS=1");
    SDL_Init(0xFFFF);
    g_libraryError = LIBERR_NONE;
    g_unused505200 = 0;
    g_libraryErrorText[0] = '\0';
    SDL_WM_SetCaption("M.U.G.E.N", "MUGEN");
    System_SetWindowIcon();

    Log_Printf("Allocating game variables\n");
    g = calloc(1, sizeof(GameVars));
    if (g == NULL) {
        g_outOfMemory = 1;
        printf("Can't allocate game variables.\n\n");
        exit(1);
    }
    g_game = g;
    g->error_text = TextRing_Create(0x400, 32);
    g = g_game;
    if (g->error_text == NULL) {
        printf("Can't allocate error clipboard.\n\n");
        exit(1);
    }
    if (_getcwd(g->cwd, sizeof(g->cwd)) != NULL) {
        System_NormalizeCwd(g_game->cwd);
    }
    g = g_game;
    if (args->game_dir[0] != '\0') {
        g->game_dir[0] = '\0';
        strncat(g->game_dir, g->cwd, 0x1FF);
        if (_chdir(args->game_dir) != 0) {
            Error_Printf("Could not change to gamedir %s\n", args->game_dir);
            Error_Exit();
        }
    }

    Log_Printf("Reading configuration file...");
    if (!Config_Load()) {
        Error_Printf("Error reading %s\n", "data/mugen.cfg");
        Error_Exit();
    }
    MfHost_ConfigLoaded();      /* Moemon Fight: the game's settings over the pack's */
    Log_Printf("OK\n");
    g = g_game;
    if (args->nomusic) {
        g->bgm_disabled = 1;
    }
    if (args->log_enabled) {
        FILE *fp;
        g->match_log = 1;
        strcpy(g->match_log_file, args->log_file);
        fp = fopen(g->match_log_file, "wt");     /* truncate */
        if (fp != NULL) {
            fclose(fp);
        }
        g = g_game;
    }
    memcpy(g->override_ai, args->ai, 19 * sizeof(int));    /* ai..power and the 3 ints after */
    g->speedtest = args->speedtest;
    g->stresstest = args->stresstest;
    g->nojoy = args->nojoy;
    g->match_seed_arg = args->match_seed_arg;
    g->session_seed_arg = args->session_seed;
    g->quick_vs = args->player[1][0] != '\0';

    Log_Printf("Initializing timer...");
    if (!g_timerInitialized) {
        Timer_Init();
    }
    if (g_timerPerformance) {
        Log_Printf("performance timer enabled...");
    }
    Log_Printf("frequency %lli...", g_timerFrequency);
    Log_Printf("OK\n");

    Log_Printf("Initializing keyboard...");
    Keyboard_Init();
    Log_Printf("configuring...");
    g_unused5044D8 = 2;
    g_keyboardQueueWrite = 0;
    g_keyboardQueueRead = 0;
    g_keyboardKeepRepeat = 0;
    SDL_EnableKeyRepeat(0, 0);
    g_keyboardNoReleases = 0;
    g_unused503468 = 1;
    Log_Printf("OK\n");

    Log_Printf("Initializing input engine...");
    Input_Init();
    if (!Events_Init()) {
        Error_Fatal("Can't init input\n");
    }
    Log_Printf("OK\n");

    Log_Printf("Initializing sound...");
    if (args->nosound) {
        g_game->sound_enabled = 0;
        g_sndActive = 0;
        Log_Printf("sound is disabled\n");
    }
    g = g_game;
    if (g->sound_enabled) {
        if (!Sound_Init(g->buffer_size, g->channels, g->bit_depth, g->wav_channels, g->sample_rate,
                        g->sfx_resample_method, g->sfx_resample_quality)) {
            g_game->sound_enabled = 0;
            g_sndActive = 0;
            Log_Printf("Could not initialize sound mixer.\n");
        } else {
            g_sndActive = 1;
            Log_Printf("OK\n");
        }
        g_libraryError = LIBERR_NONE;
        g = g_game;
        if (g->sound_enabled && !g->bgm_disabled) {
            Log_Printf("Initializing BGM...  ");
            g = g_game;
            if (!Bgm_Init(g->music_plugins, g->bgm_resample_method, g->bgm_resample_quality,
                          g->bgm_volume * g->master_volume / 100.0)) {
                Log_Printf("Could not initialize BGM.\n");
                g_game->bgm_disabled = 1;
            } else {
                Log_Printf("OK\n");
                g_bgmBackgroundMode = g_game->bgm_background_mode;
            }
        }
    }

    Log_Printf("Initializing graphics...");
    g = g_game;
    Log_Printf("gameCoord %ix%i...", (int)g->game_coord[0], (int)g->game_coord[1]);
    if (!Video_PreInit()) {
        Error_Exit();
    }
    Log_Printf("render mode %i_%i...", g_game->render_mode, g_game->video_flags);
    g = g_game;
    mode = g->render_mode;
    if (mode < RENDER_SYSTEM || mode > RENDER_OPENGL) {
        Error_Exit();
    }
    g_renderMode = mode;
    Gfx_SelectRenderer(mode);
    Log_Printf("trying %ix%ix%i mode 0x%X...", g->video_width, g->video_height, g->depth, 0);
    Video_SetVRetrace(g_game->vretrace);
    g = g_game;
    if (!Video_SetMode((int)g->game_coord[0], (int)g->game_coord[1], g->video_width, g->video_height, g->depth,
                       g->video_flags)) {
        Log_Printf("failed. ");
        Error_Exit();
    }
    Log_Printf("success...");
    if (g_bgmInitialized && (g_game->video_flags & VIDEO_FULLSCREEN)) {
        g_bgmBackgroundMode = 0;
    }
    Log_Printf("OK\n");
    g_game->window_active = 1;

    Log_Printf("Setting callbacks...");
    g_onActivate = System_OnActivate;
    Log_Printf("OK\n");

    Log_Printf("Initializing font...");
    if (TTF_Init() != 0) {
        Log_Printf("failed. ");
        Error_Exit();
    }
    if (SHGetFolderPathA(NULL, CSIDL_FONTS, NULL, 0, fonts) == S_OK) {
        strncpy(g_systemFontDir, fonts, 0x1FF);
        g_systemFontDirPtr = g_systemFontDir;
    }
    Log_Printf("OK\n");

    Log_Printf("Initializing game variables...");
    if (!GameVars_Init()) {
        Error_Exit();
    }
    Log_Printf("OK\n");
    Log_Printf("Session RNG seed is %u\n", g_game->session_seed);
    Log_Printf("Loading system fonts...");
    if (!System_LoadFonts()) {
        Error_Exit();
    }
    Log_Printf("OK\n");

    Log_Printf("Loading options...\n");
    Options_SetDefaults();
    Options_Load();
    Log_Printf("Options loaded OK\n");
    g = g_game;
    if (args->ai_level > 0) {
        g->difficulty = args->ai_level;
    }
    if (args->speed >= -9 && args->speed <= 9) {
        g->option_game_speed = args->speed;
    }

    Log_Printf("Loading system...\n");
    if (args->motif[0] != '\0') {
        g = g_game;
        strcpy(g->motif_dir, "data/");
        Path_Split(args->motif, g->motif_dir + 5, g->motif_file);
    }
    if (!System_LoadMotif()) {
        Error_Printf("Error loading system data: %s%s\n", g_game->motif_dir, g_game->motif_file);
        Error_Exit();
    }
    if (!Lua_Init()) {
        Error_Printf("Error initializing lua state");
        Error_Exit();
    }
    if (!Console_Init()) {
        Error_Printf("Error initializing console");
        Error_Exit();
    }
    Sound_SetVolume(g_game->master_volume / 100.0 * g_game->wav_volume);
    Log_Printf("System loaded OK\n");
    g = g_game;
    Gfx_SetCoordSpace((int)g->motif_coord[0], (int)g->motif_coord[1], g->system_fit);
}

/* 0x0041AEA0: shuts everything down in reverse order and ends with SDL_Quit. */
void System_Deinit(void)
{
    GameVars *g;
    int elapsed;

    g = g_game;
    g->end_ticks = (int)Timer_GetTicks();
    elapsed = g->end_ticks - g->start_ticks;
    if (!g->no_frame_stats) {
        if (g_timerTicksPerSecond > 0) {
            Log_Printf("Frames      : %u\nTime elapsed: %.3f sec\n", g->frames,
                       (double)(unsigned int)elapsed / g_timerTicksPerSecond);
            Log_Printf("ticks: %lu, tps: %lu, start %lu, end %lu\n", elapsed, g_timerTicksPerSecond,
                       g_game->start_ticks, g_game->end_ticks);
        }
        Log_Printf("\n");
    }

    Log_Printf("Unloading match assets\n");
    Match_FreeAssets();
    Players_ReleaseSlots();
    Players_Free();
    Stage_Unload(&g_game->stage);
    g_game->match_loaded = 0;
    Log_Printf("Clearing player cache...");
    PlayerCache_Clear();
    Log_Printf("OK\n");

    Log_Printf("Freeing character info...");
    g = g_game;
    if (g->char_info != NULL) {
        free(g->char_info);
    }
    if (g->char_info_2 != NULL) {
        free(g->char_info_2);
    }
    Log_Printf("OK\n");
    if (!g_game->quick_vs) {
        Log_Printf("Freeing select screen...");
        if (g_game->select_screen != NULL) {
            free(g_game->select_screen);
        }
        Log_Printf("OK\n");
    }
    Log_Printf("Freeing fight data...");
    Fight_Free();
    Log_Printf("OK\n");
    Console_Free();
    if (g_luaState != NULL) {
        lua_close(g_luaState);
        g_luaState = NULL;
        g_luaHook1 = (void *)Stub_ReturnZero;
        g_luaHook2 = (void *)Stub_Nop;
    }
    Log_Printf("Freeing system data...");
    System_FreeMotif();
    Log_Printf("OK\n");
    Log_Printf("Freeing game variables...");
    GameVars_Free();
    Log_Printf("OK\n");

    Log_Printf("Deinitializing input...");
    Events_Deinit();
    Log_Printf("pad...");
    Input_DeinitPads();
    Log_Printf("keyboard...");
    if (g_keyboardActive) {
        g_keyHandler = 0;
        memset(g_keyState, 0, sizeof(g_keyState));
        g_keyboardQueueWrite = 0;
        g_keyboardQueueRead = 0;
        g_keyboardActive = 0;
    }
    Log_Printf("OK\n");
    TTF_Quit();
    Log_Printf("Removing callbacks...");
    g_onActivate = NULL;
    Log_Printf("OK\n");
    Log_Printf("Deintializing timer...");
    if (g_timerInitialized) {
        if (!g_timerPerformance) {
            timeEndPeriod(1);
        }
        g_timerInitialized = 0;
    }
    Log_Printf("OK\n");
    Log_Printf("Deinitializing graphics...");
    Video_Deinit();
    Log_Printf("OK\n");
    Log_Printf("Deinitializing BGM...");
    Bgm_Deinit();
    Log_Printf("OK\n");
    Log_Printf("Deinitializing sound...");
    Sound_Deinit();
    Log_Printf("OK\n");
    SDL_Quit();
}

/* ------------------------------------------------------------------ input */

/* Entries of g_keyMap (input codes); KEYMAP_NONE is where unassigned keys go. */
#define KEYMAP_SIZE 0x280
#define KEYMAP_NONE 0x280
#define KEYMAP_KEY_SHIFT 0x200      /* in a built-in key: the code is ORed with KEYMAP_SHIFTED */
#define KEYMAP_SHIFTED 0x1000

#define INPUT_KEYS 11

/* .rdata 0x4F1EDC: key slot of each built-in key below. */
static const int input_key_slots[INPUT_KEYS] = {
    KEY_JUMP, KEY_CROUCH, KEY_LEFT, KEY_RIGHT, KEY_A, KEY_B, KEY_C, KEY_X, KEY_Y, KEY_Z, KEY_START,
};

/* .rdata 0x4F2038: built-in keys of each player (SDL key codes). */
static const int input_builtin_keys[2][INPUT_KEYS] = {
    { SDLK_UP, SDLK_DOWN, SDLK_LEFT, SDLK_RIGHT, SDLK_COMMA, SDLK_PERIOD, SDLK_SLASH, SDLK_l, SDLK_SEMICOLON,
      SDLK_QUOTE, SDLK_RETURN },
    { SDLK_w, SDLK_s, SDLK_a, SDLK_d, SDLK_f, SDLK_g, SDLK_h, SDLK_r, SDLK_t, SDLK_y, SDLK_u },
};

/* Gives the unassigned (0) entries 1..12 of a key map the "none" value. */
static void Input_FillUnassigned(int *keys, int none)
{
    int i;

    for (i = 1; i < KEY_COUNT; i++) {
        if (keys[i] == 0) {
            keys[i] = none;
        }
    }
}

/*
 * 0x0041B1E0: activates the joystick maps of the players that use a
 * joystick, reopens the pads and reinstalls the input callbacks.
 */
void Input_InitPads(void)
{
    GameVars *g = g_game;

    memcpy(g->joy_keys_active[0], g->joy_keys[0], sizeof(g->joy_keys_active[0]));
    memcpy(g->joy_keys_active[1], g->joy_keys[1], sizeof(g->joy_keys_active[1]));
    if (!g->use_joystick[0]) {
        memset(g->joy_keys_active[0], 0, sizeof(g->joy_keys_active[0]));
        Input_FillUnassigned(g->joy_keys_active[0], JOY_NONE);
    }
    if (!g->use_joystick[1]) {
        memset(g->joy_keys_active[1], 0, sizeof(g->joy_keys_active[1]));
        Input_FillUnassigned(g->joy_keys_active[1], JOY_NONE);
    }
    if (g_padsActive) {
        Log_Printf("Deinitializing pads\n");
        Input_DeinitPads();
        g = g_game;
    }
    if ((g->use_joystick[0] || g->use_joystick[1]) && !g->nojoy) {
        Log_Printf("Initializing pads...");
        if (!g_padsActive && Pads_Init()) {
            Log_Printf("OK\n");
        } else {
            g_game->use_joystick[1] = 0;
            g_game->use_joystick[0] = 0;
            Log_Printf("failed\n");
        }
    }
    Log_Printf("Reinitializing input engine...");
    g_keyHandler = 0;
    g_padHandler = 0;
    if (!Events_Init()) {
        Error_Fatal("Can't init input\n");
    }
    Log_Printf("OK\n");
}

/* 0x0041B340: fills GameVars.input_keys with the built-in keys. */
void Input_Init(void)
{
    int (*keys)[KEY_COUNT] = g_game->input_keys;
    int p, i;

    for (p = 0; p < 2; p++) {
        for (i = 0; i < INPUT_KEYS; i++) {
            keys[p][input_key_slots[i]] = input_builtin_keys[p][i];
        }
    }
}

/*
 * 0x0041B380: rebuilds g_keyMap, which turns the configured keys and
 * joystick buttons (as input codes) into the built-in keys, then reinstalls
 * the input callbacks. Returns 0 if the map can't be allocated or the input
 * engine fails.
 */
int Input_Remap(void)
{
    /* One spare entry: unassigned keys land on map[KEYMAP_NONE], which is never copied. */
    int map[KEYMAP_SIZE + 1];
    int pass, p, i;

    memset(map, 0, KEYMAP_SIZE * sizeof(int));
    for (pass = 0; pass < 2; pass++) {
        InputTranslateFn translate = Input_TranslateKey;

        if (pass != 0) {
            translate = Input_TranslateJoy;
        }
        for (p = 0; p < 2; p++) {
            const int *keys = pass == 0 ? g_game->keys[p] : g_game->joy_keys_active[p];

            for (i = 0; i < INPUT_KEYS; i++) {
                /* The translation of the built-in key is Input_TranslateKey, inlined. */
                int builtin = input_builtin_keys[p][i];
                int code = builtin & ~KEYMAP_KEY_SHIFT;

                if (code == KEY_NONE || code == 0) {
                    code = KEYMAP_NONE;
                } else if (builtin & KEYMAP_KEY_SHIFT) {
                    code |= KEYMAP_SHIFTED;
                }
                map[translate(keys[input_key_slots[i]])] = code;
            }
        }
    }
    Log_Printf("Remapping keys...");
    if (g_keyMap == NULL) {
        g_keyMap = calloc(4, KEYMAP_SIZE + 1);  /* sic: count and size swapped, 0xA04 bytes */
        if (g_keyMap == NULL) {
            return 0;
        }
    }
    memcpy(g_keyMap, map, KEYMAP_SIZE * sizeof(int));
    Log_Printf("OK\n");
    Log_Printf("Reinitializing input engine...");
    g_keyHandler = 0;
    g_padHandler = 0;
    if (!Events_Init()) {
        return 0;
    }
    Log_Printf("OK\n");
    return 1;
}

/* ------------------------------------------------------------- fight.def */

/*
 * The fight.def loader repeats the same sequences for every bar, face and
 * name block (inlined or copied in the original, which has no helpers for
 * them); they are factored out below without changing the order of calls.
 * Group lookups the original inlined use Ini_SelectGroup, which does the
 * same ("[lang.name]" first, then "[name]").
 */

/* value of key as an int, if present. */
static void Fight_ReadInt(Ini *ini, const char *key, int *out)
{
    const char *value = Ini_GetValue(ini, key);

    if (value != NULL) {
        *out = atoi(value);
    }
}

static int Fight_SelectGroup(Ini *ini, const char *name, const char *missing)
{
    if (Ini_SelectGroup(ini, name) == -1) {
        Error_Printf(missing);
        return 0;
    }
    return 1;
}

/* p<n>.pos, .range.x, .bg0..bg2, .mid and .front of a life or power bar. */
static void Fight_LoadBar(Ini *ini, FightBar *bar, int player, char *key)
{
    int i;

    bar->bg[0].displaytime = 1;
    bar->bg[1].displaytime = 1;
    bar->bg[2].displaytime = 1;
    bar->mid.displaytime = 1;
    bar->front.displaytime = 1;
    sprintf(key, "p%i.pos", player);
    Ini_Scanf(ini, key, "%i,%i", &bar->pos[0], &bar->pos[1]);
    sprintf(key, "p%i.range.x", player);
    Ini_Scanf(ini, key, "%i,%i", &bar->range[0], &bar->range[1]);
    for (i = 0; i < 3; i++) {
        sprintf(key, "p%i.bg%i", player, i);
        SysElem_Read(&bar->bg[i], key, ini);
    }
    sprintf(key, "p%i.mid", player);
    SysElem_Read(&bar->mid, key, ini);
    sprintf(key, "p%i.front", player);
    SysElem_Read(&bar->front, key, ini);
}

/* The old single "<key>" background, loaded only when <key>.spr or <key>.anim exists. */
static void Fight_LoadOldBg(Ini *ini, SysElem *e, char *key, char *spr_key, char *anim_key)
{
    sprintf(spr_key, "%s.spr", key);
    sprintf(anim_key, "%s.anim", key);
    if (Ini_GetValue(ini, spr_key) != NULL || Ini_GetValue(ini, anim_key) != NULL) {
        SysElem_Read(e, key, ini);
    }
}

/* p<n>.pos, .bg0..bg2, .bg, .face and .facing of player index i. */
static void Fight_LoadFace(Ini *ini, FightFace *face, int i, char *key, char *spr_key, char *anim_key)
{
    FightFacePlayer *p = &face->p[i];
    int player = i + 1;
    int j;

    sprintf(key, "p%i.pos", player);
    Ini_Scanf(ini, key, "%i,%i", &face->pos_x[i], &face->pos_y[i]);
    for (j = 0; j < 3; j++) {
        sprintf(key, "p%i.bg%i", player, j);
        SysElem_Read(&p->bg[j], key, ini);
    }
    sprintf(key, "p%i.bg", player);
    Fight_LoadOldBg(ini, &p->bg[0], key, spr_key, anim_key);
    sprintf(key, "p%i.face", player);
    SysElem_Read(&p->face, key, ini);
    sprintf(key, "p%i.facing", player);
    Fight_ReadInt(ini, key, &face->facing[i]);
}

/* p<n>.pos, .bg and .name of player index i. */
static void Fight_LoadName(Ini *ini, FightName *name, int i, char *key)
{
    int player = i + 1;

    sprintf(key, "p%i.pos", player);
    Ini_Scanf(ini, key, "%i,%i", &name->pos_x[i], &name->pos_y[i]);
    sprintf(key, "p%i.bg", player);
    SysElem_Read(&name->bg[i], key, ini);
    sprintf(key, "p%i.name", player);
    SysElem_Read(&name->name[i], key, ini);
}

/* Copies the texts of a combo counter's font elements into its format strings. */
static void Fight_SetComboFormats(FightCombo *c)
{
    strncpy(c->text_fmt, c->text.text, sizeof(c->text_fmt));
    c->text_fmt[sizeof(c->text_fmt) - 1] = '\0';
    strncpy(c->counter_fmt, c->counter.text, sizeof(c->counter_fmt));
    /* Bug kept: terminates text_fmt again, so a 64-character counter text leaves counter_fmt unterminated. */
    c->text_fmt[sizeof(c->text_fmt) - 1] = '\0';
    if (c->counter_fmt[0] == '\0') {
        strcpy(c->counter_fmt, "%i");
    }
}

/* Turns each "%%" into "%" in place. */
static void Fight_CollapsePercents(char *s)
{
    int i = 0, j = 0;

    if (s[0] != '\0') {
        do {
            if (s[i] != '%' || s[i + 1] != '%') {
                s[j++] = s[i];
            }
            i++;
        } while (s[i] != '\0');
    }
    s[j] = '\0';
}

/*
 * 0x0041B500: loads the fight screen definition (fight.def) into
 * GameVars.fight / fightfx_*, then allocates the explod lists. Returns 0 on
 * error, as in the original without freeing the parsed file or anything
 * loaded so far.
 */
int Fight_Load(const char *path)
{
    static const struct {
        const char *key;
        int icon;
    } win_icons[] = {
        { "p%i.n", WIN_ICON_N }, { "p%i.s", WIN_ICON_S }, { "p%i.h", WIN_ICON_H },
        { "p%i.throw", WIN_ICON_THROW }, { "p%i.t", WIN_ICON_T }, { "p%i.c", WIN_ICON_C },
        { "p%i.suicide", WIN_ICON_SUICIDE }, { "p%i.teammate", WIN_ICON_TEAMMATE },
        { "p%i.perfect", WIN_ICON_PERFECT },
    };
    FightDef *fd = &g_game->fight;
    FightRound *r = &fd->round;
    FightWinIcon *w = &fd->winicon;
    char key[0xC0];
    char spr_key[0x40];
    char anim_key[0x40];
    char found[0x200];
    char dir[0x200];
    const char *dirs[6];
    const char *value;
    const char *line;
    Stream *s;
    Ini *ini;
    GameVars *g;
    float center;
    int i, j;

    Log_Printf("    Opening fight data file %s...", path);
    s = Stream_OpenFile(path, "rt");
    ini = NULL;
    if (s != NULL) {
        ini = Ini_Parse(s, INI_MODE_READ);
        Stream_Close(s);
    }
    if (ini == NULL) {
        Error_Printf("Can't open %s\n", path);
        return 0;
    }
    Log_Printf("OK\n");
    if (path[0] == '\0') {
        dir[0] = '\0';
    } else {
        for (i = (int)strlen(path) - 1; i >= 0 && path[i] != '\\' && path[i] != '/'; i--) {
        }
        i++;
        if (i > 0) {
            strncpy(dir, path, i);
        }
        dir[i] = '\0';
    }

    /* [Files] */
    Log_Printf("    Reading [Files]...");
    if (!Fight_SelectGroup(ini, "Files", "Missing files info\n")) {
        return 0;
    }
    value = Ini_GetValue(ini, "sff");
    if (value == NULL) {
        Error_Printf("Can't read Files|sff\n");
        return 0;
    }
    if (!File_Find(value, found, NULL, dir)) {
        return 0;
    }
    fd->sff = Sff_Load(found);
    if (fd->sff == NULL) {
        Error_Printf("Can't load %s\n", value);
        return 0;
    }
    value = Ini_GetValue(ini, "snd");
    if (value == NULL) {
        Error_Printf("Can't read Files|snd\n");
        return 0;
    }
    if (g_sndActive == 1) {
        if (!File_Find(value, found, NULL, dir)) {
            return 0;
        }
        fd->snd = Snd_Load(found);
        if (fd->snd == NULL) {
            Error_Printf("Can't load %s\n", value);
            return 0;
        }
    } else {
        fd->snd = NULL;
    }
    value = Ini_GetValue(ini, "fightfx.sff");
    if (value == NULL) {
        Error_Printf("Can't read Files|fightfx.sff\n");
        return 0;
    }
    if (!File_Find(value, found, NULL, dir)) {
        return 0;
    }
    g_game->fightfx_sff = Sff_Load(found);
    if (g_game->fightfx_sff == NULL) {
        Error_Printf("Can't load %s\n", value);
        return 0;
    }
    value = Ini_GetValue(ini, "fightfx.air");
    if (value == NULL) {
        Error_Printf("Can't read Files|fightfx.air\n");
        return 0;
    }
    if (!File_Find(value, found, NULL, dir)) {
        return 0;
    }
    g_game->fightfx_anim = Anim_Load(found);
    if (g_game->fightfx_anim == NULL) {
        Error_Printf("Can't load %s\n", value);
        return 0;
    }
    value = Ini_GetValue(ini, "common.snd");
    if (value == NULL) {
        Error_Printf("Can't read Files|common.snd\n");
        return 0;
    }
    if (g_sndActive == 1) {
        if (!File_Find(value, found, NULL, dir)) {
            return 0;
        }
        g_game->fight.common_snd = Snd_Load(found);
        if (g_game->fight.common_snd == NULL) {
            Error_Printf("Can't load %s\n", value);
            return 0;
        }
    } else {
        g_game->fight.common_snd = NULL;
    }
    Log_Printf("OK\n");
    Log_Printf("    Loading fonts...");
    dirs[0] = dir;
    dirs[1] = g_game->motif_dir;
    dirs[2] = "./";
    dirs[3] = "data/";
    dirs[4] = "font/";
    dirs[5] = NULL;
    Fonts_Load(ini, fd->fonts, dirs);
    Log_Printf("OK\n");

    /* The first "[Begin Action n]" group, if any, starts the fight animations. */
    ini->group = -1;
    line = Ini_NextGroup(ini);
    while (!Str_StartsWithI(line + 1, "begin action ")) {     /* no NULL check before the first test */
        line = Ini_NextGroup(ini);
        if (line == NULL) {
            break;
        }
    }
    if (line != NULL) {
        Log_Printf("    Load fight anim...");
        fd->anim = Anim_Parse(ini);
        if (fd->anim == NULL) {
            Error_Printf("Error loading anim in Files\n");
            return 0;
        }
        Log_Printf("OK\n");
    }

    g_game->fightfx_scale = 1.0f;
    if (Ini_SelectGroup(ini, "fightfx") != -1) {
        Ini_GetFloat(ini, "scale", &g_game->fightfx_scale);
    }

    /* Life and power bars. */
    Log_Printf("    Reading [Lifebar]...");
    if (!Fight_SelectGroup(ini, "Lifebar", "Missing lifebar info\n")) {
        return 0;
    }
    for (i = 0; i < 2; i++) {
        Fight_LoadBar(ini, &fd->lifebar[i], i + 1, key);
    }
    Log_Printf("[Turns Lifebar]...");
    if (!Fight_SelectGroup(ini, "Turns Lifebar", "Missing turns lifebar info\n")) {
        return 0;
    }
    for (i = 0; i < 2; i++) {
        Fight_LoadBar(ini, &fd->turns_lifebar[i], i + 1, key);
    }
    Log_Printf("[Simul Lifebar]...");
    if (!Fight_SelectGroup(ini, "Simul Lifebar", "Missing simul lifebar info\n")) {
        return 0;
    }
    for (i = 0; i < 4; i++) {
        Fight_LoadBar(ini, &fd->simul_lifebar[i], i + 1, key);
    }
    Log_Printf("[Powerbar]...");
    if (!Fight_SelectGroup(ini, "Powerbar", "Missing Powerbar info\n")) {
        return 0;
    }
    for (i = 0; i < FIGHT_LEVEL_SNDS; i++) {
        fd->level_snd_group[i] = -1;
    }
    for (i = 0; i < 2; i++) {
        Fight_LoadBar(ini, &fd->powerbar[i], i + 1, key);
        sprintf(key, "p%i.counter", i + 1);
        SysElem_Read(&fd->powerbar_counter[i], key, ini);
    }
    for (i = 0; i < FIGHT_LEVEL_SNDS; i++) {
        sprintf(spr_key, "level%i.snd", i);
        Ini_Scanf(ini, spr_key, "%i,%i", &fd->level_snd_group[i], &fd->level_snd_number[i]);
    }

    /* Faces. */
    Log_Printf("\n    [Face]...");
    if (!Fight_SelectGroup(ini, "face", "Missing face info\n")) {
        return 0;
    }
    fd->face.facing[0] = 1;
    fd->face.facing[1] = -1;
    for (i = 0; i < 2; i++) {
        Fight_LoadFace(ini, &fd->face, i, key, spr_key, anim_key);
    }
    Log_Printf("[Simul Face]...");
    if (!Fight_SelectGroup(ini, "simul face", "Missing simul face info\n")) {
        return 0;
    }
    fd->simul_face.facing[2] = 1;
    fd->simul_face.facing[0] = 1;
    fd->simul_face.facing[3] = -1;
    fd->simul_face.facing[1] = -1;
    for (i = 0; i < 4; i++) {
        Fight_LoadFace(ini, &fd->simul_face, i, key, spr_key, anim_key);
    }
    Log_Printf("[Turns Face]...");
    if (!Fight_SelectGroup(ini, "turns face", "Missing turns face info\n")) {
        return 0;
    }
    fd->turns_face.facing[0] = 1;
    fd->turns_face.facing[1] = -1;
    for (i = 0; i < 2; i++) {
        FightTeammate *tm = &fd->turns_teammate[i];
        int player = i + 1;

        Fight_LoadFace(ini, &fd->turns_face, i, key, spr_key, anim_key);
        sprintf(key, "p%i.teammate.pos", player);
        Ini_Scanf(ini, key, "%i,%i", &tm->pos[0], &tm->pos[1]);
        sprintf(key, "p%i.teammate.spacing", player);
        Ini_Scanf(ini, key, "%i,%i", &tm->spacing[0], &tm->spacing[1]);
        for (j = 0; j < 3; j++) {
            sprintf(key, "p%i.teammate.bg%i", player, j);
            SysElem_Read(&tm->bg[j], key, ini);
        }
        sprintf(key, "p%i.teammate.bg", player);
        Fight_LoadOldBg(ini, &tm->bg[0], key, spr_key, anim_key);
        sprintf(key, "p%i.teammate.face", player);
        SysElem_Read(&tm->face, key, ini);
        sprintf(key, "p%i.teammate.ko", player);
        SysElem_Read(&tm->ko, key, ini);
        sprintf(key, "p%i.teammate.facing", player);
        Fight_ReadInt(ini, key, &fd->turns_face.facing[i]);   /* sic: overwrites p<n>.facing */
    }

    /* Names. */
    Log_Printf("\n    [Name]...");
    if (!Fight_SelectGroup(ini, "name", "Missing name info\n")) {
        return 0;
    }
    for (i = 0; i < 2; i++) {
        Fight_LoadName(ini, &fd->name, i, key);
    }
    Log_Printf("[Simul Name]...");
    if (!Fight_SelectGroup(ini, "simul name", "Missing simul name info\n")) {
        return 0;
    }
    for (i = 0; i < 4; i++) {
        Fight_LoadName(ini, &fd->simul_name, i, key);
    }
    Log_Printf("[Turns Name]...");
    if (!Fight_SelectGroup(ini, "turns name", "Missing turns name info\n")) {
        return 0;
    }
    for (i = 0; i < 4; i++) {
        Fight_LoadName(ini, &fd->turns_name, i, key);
    }

    /* Timer. */
    Log_Printf("\n    [Time]...");
    if (!Fight_SelectGroup(ini, "time", "Missing time info\n")) {
        return 0;
    }
    fd->time.frames_per_count = 60;
    Ini_Scanf(ini, "pos", "%i,%i", &fd->time.pos[0], &fd->time.pos[1]);
    SysElem_Read(&fd->time.bg, "bg", ini);
    SysElem_Read(&fd->time.counter, "counter", ini);
    Fight_ReadInt(ini, "framespercount", &fd->time.frames_per_count);

    /* Combo counters: one setting for both teams ("pos" given, team 2 mirrored) or team1.* / team2.*. */
    Log_Printf("[Combo]...");
    if (!Fight_SelectGroup(ini, "combo", "Missing combo info\n")) {
        return 0;
    }
    if (Ini_GetValue(ini, "pos") != NULL) {
        for (i = 0; i < 2; i++) {
            FightCombo *c = &fd->combo[i];

            c->pos[0] = 10;
            c->pos[1] = 80;
            c->start_x = -40;
            c->displaytime = 90;
            Ini_Scanf(ini, "pos", "%i,%i", &c->pos[0], &c->pos[1]);
            Fight_ReadInt(ini, "start.x", &c->start_x);
            SysElem_Read(&c->counter, "counter", ini);
            Fight_ReadInt(ini, "counter.shake", &c->shake);
            SysElem_Read(&c->text, "text", ini);
            Fight_ReadInt(ini, "displaytime", &c->displaytime);
            Fight_SetComboFormats(c);
        }
        fd->combo[1].counter.font_align = SYSELEM_ALIGN_RIGHT;
        fd->combo[1].text.font_align = SYSELEM_ALIGN_RIGHT;
        fd->combo[0].counter.font_align = SYSELEM_ALIGN_LEFT;
        fd->combo[0].text.font_align = SYSELEM_ALIGN_LEFT;
        fd->combo[1].start_x = g_targetWidth - fd->combo[1].start_x - 1;
        fd->combo[1].pos[0] = 319 - fd->combo[1].pos[0];
    } else {
        for (i = 0; i < 2; i++) {
            FightCombo *c = &fd->combo[i];
            int team = i + 1;

            sprintf(key, "team%i.pos", team);
            Ini_Scanf(ini, key, "%i,%i", &c->pos[0], &c->pos[1]);
            sprintf(key, "team%i.start.x", team);
            Fight_ReadInt(ini, key, &c->start_x);
            sprintf(key, "team%i.counter", team);
            SysElem_Read(&c->counter, key, ini);
            sprintf(key, "team%i.counter.shake", team);
            Fight_ReadInt(ini, key, &c->shake);
            sprintf(key, "team%i.text", team);
            SysElem_Read(&c->text, key, ini);
            sprintf(key, "team%i.displaytime", team);
            Fight_ReadInt(ini, key, &c->displaytime);
            Fight_SetComboFormats(c);
        }
    }

    /* Round announcements. */
    Log_Printf("[Round]...");
    if (!Fight_SelectGroup(ini, "round", "Missing round info\n")) {
        return 0;
    }
    g = g_game;
    r->start_waittime = 30;
    r->intro_time = 70;
    center = (float)(g->video_width / 2);
    r->win.x = center;
    r->ko.x = center;
    r->fight.x = center;
    center = (float)(g->video_height / 2);
    r->win.y = center;
    r->ko.y = center;
    r->fight.y = center;
    strcpy(r->fight.text, "Fight!");
    strcpy(r->ko.text, "K.O");
    r->round_time = 0;
    r->fight_time = 0;
    r->ko_time = 0;
    r->win_time = 60;
    r->slow_time = 60;
    r->over_waittime = 0;
    r->over_hittime = 10;
    r->over_wintime = 45;
    r->over_time = 150;
    r->match_wins = 2;
    r->match_maxdrawgames = 1;
    Fight_ReadInt(ini, "match.wins", &r->match_wins);
    Fight_ReadInt(ini, "match.maxdrawgames", &r->match_maxdrawgames);
    Fight_ReadInt(ini, "start.waittime", &r->start_waittime);
    Fight_ReadInt(ini, "intro.time", &r->intro_time);
    Ini_Scanf(ini, "pos", "%i,%i", &r->pos[0], &r->pos[1]);
    SysElem_Read(&r->round_default, "round.default", ini);
    Fight_CollapsePercents(r->round_default.text);
    Fight_ReadInt(ini, "round.time", &r->round_time);
    r->round_sndtime = r->round_time;
    Fight_ReadInt(ini, "round.sndtime", &r->round_sndtime);
    for (i = 1; i <= 9; i++) {
        r->round_snd_group[i] = -1;
        sprintf(key, "round%i.snd", i);
        Ini_Scanf(ini, key, "%i,%i", &r->round_snd_group[i], &r->round_snd_number[i]);
        sprintf(key, "round%i", i);
        SysElem_Read(&r->round[i - 1], key, ini);
    }
    r->fight_snd[0] = -1;
    SysElem_Read(&r->fight, "fight", ini);
    Fight_ReadInt(ini, "fight.time", &r->fight_time);
    r->fight_sndtime = r->fight_time;
    Fight_ReadInt(ini, "fight.sndtime", &r->fight_sndtime);
    Ini_Scanf(ini, "fight.snd", "%i,%i", &r->fight_snd[0], &r->fight_snd[1]);
    Fight_ReadInt(ini, "ctrl.time", &r->ctrl_time);
    r->to_snd[0] = -1;
    r->dko_snd[0] = -1;
    r->ko_snd[0] = -1;
    SysElem_Read(&r->ko, "KO", ini);
    SysElem_Read(&r->dko, "DKO", ini);
    SysElem_Read(&r->to, "TO", ini);
    Fight_ReadInt(ini, "KO.time", &r->ko_time);
    r->ko_sndtime = r->ko_time;
    Fight_ReadInt(ini, "KO.sndtime", &r->ko_sndtime);
    Ini_Scanf(ini, "KO.snd", "%i,%i", &r->ko_snd[0], &r->ko_snd[1]);
    Ini_Scanf(ini, "DKO.snd", "%i,%i", &r->dko_snd[0], &r->dko_snd[1]);
    Ini_Scanf(ini, "TO.snd", "%i,%i", &r->to_snd[0], &r->to_snd[1]);
    /* Quirk kept: over.hittime is only read further down, so this adds its default (10). */
    r->ko_time += r->over_hittime;
    r->ko_sndtime += r->over_hittime;
    Fight_ReadInt(ini, "win.time", &r->win_time);
    SysElem_Read(&r->win, "win", ini);
    SysElem_Read(&r->win2, "win2", ini);
    SysElem_Read(&r->draw, "draw", ini);
    r->win_snd[0] = -1;
    r->draw_snd[0] = -1;
    r->lose_snd[0] = -1;
    Ini_Scanf(ini, "win.snd", "%i,%i", &r->win_snd[0], &r->win_snd[1]);
    Ini_Scanf(ini, "lose.snd", "%i,%i", &r->lose_snd[0], &r->lose_snd[1]);
    Ini_Scanf(ini, "draw.snd", "%i,%i", &r->draw_snd[0], &r->draw_snd[1]);
    Fight_ReadInt(ini, "slow.time", &r->slow_time);
    Fight_ReadInt(ini, "over.waittime", &r->over_waittime);
    Fight_ReadInt(ini, "over.hittime", &r->over_hittime);
    Fight_ReadInt(ini, "over.wintime", &r->over_wintime);
    Fight_ReadInt(ini, "over.time", &r->over_time);

    /* Win icons. */
    Log_Printf("[WinIcon]...");
    if (!Fight_SelectGroup(ini, "winicon", "Missing winicon info\n")) {
        return 0;
    }
    for (i = 0; i < 2; i++) {
        int player = i + 1;

        sprintf(key, "p%i.pos", player);
        Ini_Scanf(ini, key, "%i,%i", &w->pos_x[i], &w->pos_y[i]);
        sprintf(key, "p%i.iconoffset", player);
        Ini_Scanf(ini, key, "%i,%i", &w->iconoffset_x[i], &w->iconoffset_y[i]);
        sprintf(key, "p%i.bg0", player);
        SysElem_Read(&w->bg0[i], key, ini);
        sprintf(key, "p%i.counter", player);
        SysElem_Read(&w->counter[i], key, ini);
        for (j = 0; j < WIN_ICON_COUNT; j++) {
            sprintf(key, win_icons[j].key, player);
            SysElem_Read(&w->icon[i][win_icons[j].icon], key, ini);
        }
    }
    Fight_ReadInt(ini, "useiconupto", &w->useiconupto);
    if (w->useiconupto > 9) {
        w->useiconupto = 9;
    }
    Log_Printf("OK\n");
    Ini_Free(ini);

    Log_Printf("    Allocating explods...");
    g_game->explods = Explods_Create(g_game->explod_max, g_game->sprite_layer1);
    if (g_game->explods == NULL) {
        Error_Printf("Can't allocate explods\n");
        return 0;
    }
    g_game->sysexplods = Explods_Create(g_game->sysexplod_max, NULL);
    if (g_game->sysexplods == NULL) {
        Error_Printf("Can't allocate sysexplods\n");
        return 0;
    }
    Log_Printf("OK\n");
    return 1;
}

/* 0x0041D9B0: frees what Fight_Load loaded (the font loop uses Font_Free, inlined in the original). */
void Fight_Free(void)
{
    int i;

    for (i = 0; i < FIGHT_FONT_COUNT; i++) {
        Font *font = g_game->fight.fonts[i];

        if (font != g_game->font_small && font != NULL) {
            Font_Free(font);
        }
    }
    Anim_Free(g_game->fight.anim);
    SpriteSet_Free(g_game->fight.sff);
    Snd_Free(g_game->fight.common_snd);
    Snd_Free(g_game->fight.snd);
    SpriteSet_Free(g_game->fightfx_sff);
    Anim_Free(g_game->fightfx_anim);
    Explods_Free(g_game->sysexplods);
    Explods_Free(g_game->explods);
}

/* ------------------------------------------------------- game variables */

/*
 * 0x0041DAB0: creates the screen buffer, sprite decompression buffer, draw
 * queues and other per-game state. On failure everything is freed with
 * GameVars_Free (g_game becomes NULL) and 0 is returned.
 */
int GameVars_Init(void)
{
    GameVars *g = g_game;
    __time64_t now;
    FILE *fp;
    void *palgroups;

    g->team_variant = 0;
    g->compat_version[0] = 0x01010000;
    g->compat_version[1] = 0x01000000;
    g->compat_version[2] = 0x01010000;
    fp = fopen("_mugen__.tmp", "wb");
    if (fp != NULL) {
        g_game->cwd_writable = 1;
        fclose(fp);
        remove("_mugen__.tmp");
    }
    g = g_game;
    g->continue_reselect = 1;
    if (!g->safe_mode_opengl) {
        g->screen = Screen_Create(g->depth, (int)g->game_coord[0], (int)g->game_coord[1]);
        g = g_game;
        if (g->screen == NULL) {
            GameVars_Free();
            Error_Printf("Can't create screen buffer.\n");
            return 0;
        }
    }
    Gfx_SetRenderTarget(g->screen, 0);
    g_gfxClear(0);
    g = g_game;
    g->render_target = g->screen;
    g_spriteDecompBuffer = DecompBuffer_Create(g->sprite_decompression_buffer_size << 10);
    if (g_spriteDecompBuffer == NULL) {
        GameVars_Free();
        Error_Printf("Can't allocate sprite decompression buffer.\n");
        return 0;
    }
    g_game->sprite_layer1 = SpriteQueue_Create(g_game->layered_sprite_max);
    if (g_game->sprite_layer1 == NULL) {
        GameVars_Free();
        Error_Printf("Can't create priority sprites layer 1.\n");
        return 0;
    }
    g_game->anims = AnimSet_Create();
    if (g_game->anims == NULL) {
        GameVars_Free();
        Error_Printf("Can't create persistent animation array.\n");
        return 0;
    }
    g_game->afterimages = AfterImages_Create(g_game->afterimage_max);
    g = g_game;
    if (g->afterimages == NULL) {
        GameVars_Free();
        Error_Printf("Can't init afterimages.\n");
        return 0;
    }
    palgroups = calloc(1, sizeof(PalGroups));     /* Moemon Fight: 0xC on 32-bit */
    if (palgroups == NULL) {
        g_outOfMemory = 1;
    }
    g->palgroups = palgroups;
    if (palgroups == NULL) {
        GameVars_Free();
        Error_Printf("Can't create palgroup array.\n");
        return 0;
    }
    if (!IntArray_Init(&g->key_codes, 0x20)) {
        GameVars_Free();
        return 0;
    }
    g = g_game;
    g->vretrace_on = g->vretrace;
    g->hud_enabled = 1;
    strcpy(g->motif_file, "system.def");
    strcpy(g->motif_dir, "data/");
    g->camera_offset_y = 0.0f;
    g->camera_offset_x = 0.0f;
    g->camera_y = 0.0f;
    g->frame_skip = -1;
    g->players_last = -1;
    g->camera_x = 0.0f;
    g->helper_slots = g->helper_max;
    g->camera_zoom = 1.0f;
    g->frames = 0;
    g->skip_draw = 0;
    g->frame_skip_count = 0;
    g->camera_y_base = 0;
    g->unused_12378[0] = 1;
    g->unused_12378[1] = 1;
    g->unused_12378[2] = 0;
    g->unused_12378[3] = 0;
    g->unused_1216C = 4;
    g->player_slots = g->helper_max + 4;
    g->player_sides[0] = 1;
    g->player_sides[1] = 2;
    now = time(NULL);
    g_game->time_seed = (unsigned int)now;
    srand((unsigned int)now);
    Rng_Seed(g_game->session_seed_arg);
    g = g_game;
    g->keep_sources = 1;
    g->clipboard_lines = 0x10;
    g->clipboard = g_log != NULL ? g_log->mirror : NULL;
    return 1;
}

/* 0x0041DE60: the two built-in fonts; a failure is reported as a font library error. */
int System_LoadFonts(void)
{
    g_game->font_small = Font_Load("font/f-4x6.def", 0);
    if (g_game->font_small == NULL) {
        g_libraryError = LIBERR_FONT;
        Error_Printf("Error loading f-4x6.def\n");
        return 0;
    }
    g_game->font_medium = Font_Load("font/f-6x9.def", 0);
    if (g_game->font_medium == NULL) {
        g_libraryError = LIBERR_FONT;
        Error_Printf("Error loading f-6x9.def\n");
        return 0;
    }
    return 1;
}

/*
 * 0x0041DEE0: unloads the match and frees the game variables (g_game becomes
 * NULL). Font_Free, AnimSet_Free, SpriteQueue_Free and TextRing_Free were
 * inlined in the original; they free the same blocks in the same order.
 */
void GameVars_Free(void)
{
    GameVars *g;
    DecompBuffer *buf;

    if (g_game == NULL) {
        return;
    }
    Log_Printf("Unloading match assets\n");
    Match_FreeAssets();
    Players_ReleaseSlots();
    Players_Free();
    Stage_Unload(&g_game->stage);
    g = g_game;
    g->match_loaded = 0;
    if (g->options_font != g->font_medium && g->options_font != NULL) {
        Font_Free(g->options_font);
        g = g_game;
    }
    if (g->font_small != NULL) {
        Font_Free(g->font_small);
        g = g_game;
    }
    if (g->font_medium != NULL) {
        Font_Free(g->font_medium);
        g = g_game;
    }
    if (g->key_codes.data != NULL) {
        free(g->key_codes.data);
    }
    memset(&g->key_codes, 0, sizeof(g->key_codes));
    if (g->palgroups != NULL) {
        free(g->palgroups);
    }
    AfterImages_Free(g->afterimages);
    AnimSet_Free(g_game->anims);
    SpriteQueue_Free(g_game->sprite_layer1);
    buf = g_spriteDecompBuffer;
    if (buf != NULL) {
        if (buf->data != NULL) {
            free(buf->data);
            buf->data = NULL;
        }
        free(buf);
        g_spriteDecompBuffer = NULL;
    }
    if (g_game->screen != NULL) {
        SpriteSet *screen = g_game->screen;

        if (g_renderTarget == screen) {
            Gfx_SetRenderTarget(NULL, 0);
            g_renderTarget = NULL;
            g_renderTargetSprite = 0;
        }
        SpriteSet_Free(screen);
    }
    g = g_game;
    TextRing_Free(g->error_text);
    if (g->music_plugins != NULL) {
        BgmPlugins_Free(g->music_plugins);
        g = g_game;
    }
    free(g);
    g_game = NULL;
}

/* 0x0041E170: count afterimage buffers keyed (0, index), or NULL. */
KeyedArray *AfterImages_Create(int count)
{
    AfterImage ai;
    KeyedArray *list;
    int i;

    list = KeyedArray_Create(count, sizeof(AfterImage));
    if (list == NULL) {
        return NULL;
    }
    for (i = 0; i < count; i++) {
        memset(&ai, 0, sizeof(ai));
        if (!AfterImage_Init(&ai)) {
            AfterImages_Free(list);
            return NULL;
        }
        KeyedArray_Add(list, &ai, 0, i);
    }
    return list;
}

/* 0x0041E1E0: frees the buffers and the list (KeyedArray_Free inlined in the original). */
void AfterImages_Free(KeyedArray *list)
{
    int i;

    if (list == NULL) {
        return;
    }
    for (i = 0; i < list->capacity; i++) {
        int slot = KeyedArray_Find(list, 0, i);

        if (slot != -1) {
            AfterImage *ai = (AfterImage *)(list->data + list->elem_size * slot);

            if (ai != NULL) {
                if (ai->frames != NULL) {
                    free(ai->frames);
                }
                memset(ai, 0, sizeof(*ai));
            }
        }
    }
    KeyedArray_Free(list);
}

/* ------------------------------------------------------------ window focus */

/*
 * BGM commands carry two words of data; the focus handlers only fill the
 * first one, so the second is whatever was on the stack (as in the original).
 */

/* 0x0041E280: mutes sound effects and pauses or mutes the BGM as configured ([Misc] *BackgroundMode). */
void System_OnFocusLost(int gained)
{
    GameVars *g = g_game;

    g->window_active = 0;
    if (g->sfx_background_mode == SFX_BG_MUTE) {
        int was_muted = g_sndMuted;

        g_sndMuted = 1;
        if (!was_muted) {
            Mix_Volume(-1, 0);
            g = g_game;
        }
    }
    if (g_bgmInitialized) {
        int time = g_bgmGetHandle();
        intptr_t data[2];

        pthread_mutex_lock((pthread_mutex_t *)&g_bgmMutex);
        switch (g_bgmBackgroundMode) {
        case BGM_BG_PAUSE:
            if (!(g_bgmStream->flags & BGM_STREAM_PAUSED)) {
                data[0] = BGM_STREAM_PAUSED;
                Bgm_QueueCommand(time, data, BGM_CMD_PAUSE, 0);
                g_bgmBackgroundChanged = 1;
            }
            break;
        case BGM_BG_MUTE:
            if (g_bgmMuted == 0) {
                data[0] = 1;
                Bgm_QueueCommand(time, data, BGM_CMD_MUTE, 0);
                g_bgmBackgroundChanged = 1;
            }
            break;
        }
        pthread_mutex_unlock((pthread_mutex_t *)&g_bgmMutex);
        g = g_game;
    }
    if (g->pause_on_defocus) {
        g->defocus_paused = 1;
    }
}

/* 0x0041E370: undoes System_OnFocusLost and resynchronises the keyboard state. */
void System_OnFocusGained(int gained)
{
    g_game->window_active = 1;
    Keyboard_SyncState();
    Input_ResetState();
    if (g_game->sfx_background_mode == SFX_BG_MUTE) {
        int was_muted = g_sndMuted;

        g_sndMuted = 0;
        if (was_muted && g_sndActive == 1) {
            Sound_RestoreVolumes();
        }
    }
    if (g_bgmInitialized) {
        int time = g_bgmGetHandle();
        intptr_t data[2];

        pthread_mutex_lock((pthread_mutex_t *)&g_bgmMutex);
        switch (g_bgmBackgroundMode) {
        case BGM_BG_PAUSE:
            if (g_bgmStream->flags & BGM_STREAM_PAUSED) {
                data[0] = BGM_STREAM_PAUSED;
                Bgm_QueueCommand(time, data, BGM_CMD_RESUME, 0);
                g_bgmBackgroundChanged = 1;
            }
            break;
        case BGM_BG_MUTE:
            if (g_bgmMuted != 0) {
                Bgm_QueueMute(0, time);
                g_bgmBackgroundChanged = 1;
            }
            break;
        }
        pthread_mutex_unlock((pthread_mutex_t *)&g_bgmMutex);
    }
    if (g_game->pause_on_defocus && g_pauseToggled == 0) {
        g_game->defocus_paused = 0;
    }
}

/* 0x0041E470: the number of frames run (the BGM thread's clock, g_bgmGetHandle). */
int Game_GetFrameCount(void)
{
    return (int)g_game->frames;
}
