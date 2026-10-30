/*
 * mugen.cfg: configuration and options (original module 0x45DE80-0x45FC7F).
 *
 * Config_Load reads the whole file at start-up; Options_Load/Options_Save
 * handle the part the options screen edits ([Options], volumes, keys).
 */
#include "mugen.h"

#define CONFIG_PATH "data/mugen.cfg"

static const char *const ai_ramp_modes[3] = { "arcade", "team", "survival" };

/* 0x0045FA30: reads a [P<n> Keys] / [P<n> Joystick] group into keys (missing keys keep their value). */
int Config_ReadKeys(Ini *ini, const char *group, int *keys)
{
    static const struct {
        const char *name;
        int index;
    } map[] = {
        { "Jump", KEY_JUMP }, { "Crouch", KEY_CROUCH }, { "Left", KEY_LEFT }, { "Right", KEY_RIGHT },
        { "A", KEY_A }, { "B", KEY_B }, { "C", KEY_C }, { "X", KEY_X }, { "Y", KEY_Y }, { "Z", KEY_Z },
        { "Start", KEY_START },
    };
    int i;

    if (Ini_SelectGroup(ini, group) == -1) {
        return 0;
    }
    for (i = 0; i < (int)(sizeof(map) / sizeof(map[0])); i++) {
        const char *value = Ini_GetValue(ini, map[i].name);
        if (value != NULL) {
            keys[map[i].index] = atoi(value);
        }
    }
    return 1;
}

/* 0x0045FB90: writes keys into the selected group (Ini opened editable). */
void Config_WriteKeys(Ini *ini, const int *keys)
{
    Ini_SetValuef(ini, "Jump", "%i", keys[KEY_JUMP]);
    Ini_SetValuef(ini, "Crouch", "%i", keys[KEY_CROUCH]);
    Ini_SetValuef(ini, "Left", "%i", keys[KEY_LEFT]);
    Ini_SetValuef(ini, "Right", "%i", keys[KEY_RIGHT]);
    Ini_SetValuef(ini, "A", "%i", keys[KEY_A]);
    Ini_SetValuef(ini, "B", "%i", keys[KEY_B]);
    Ini_SetValuef(ini, "C", "%i", keys[KEY_C]);
    Ini_SetValuef(ini, "X", "%i", keys[KEY_X]);
    Ini_SetValuef(ini, "Y", "%i", keys[KEY_Y]);
    Ini_SetValuef(ini, "Z", "%i", keys[KEY_Z]);
    Ini_SetValuef(ini, "Start", "%i", keys[KEY_START]);
}

/* Unassigned entries (0) of a key map get the given "none" value. */
static void Config_FillUnassigned(int *keys, int none)
{
    int i;

    for (i = 1; i < KEY_COUNT; i++) {
        if (keys[i] == 0) {
            keys[i] = none;
        }
    }
}

/*
 * 0x0045DE80: loads data/mugen.cfg into the game variables. Returns 0 (with
 * an error message) on a missing mandatory group or an invalid value; as in
 * the original the Ini is then not freed.
 */
int Config_Load(void)
{
    GameVars *g;
    char text[0x104];
    const char *value;
    int width, height;
    int flag;
    Stream *s;
    Ini *ini = NULL;
    int c, i;

    s = Stream_OpenFile(CONFIG_PATH, "rt");
    if (s != NULL) {
        ini = Ini_Parse(s, INI_MODE_READ);
        Stream_Close(s);
    }
    if (ini == NULL) {
        Error_Printf("Can't open config file %s.\n", CONFIG_PATH);
        return 0;
    }

    /* [Rules] */
    if (Ini_SelectGroup(ini, "Rules") == -1) {
        Error_Printf("Can't access [Rules]\n");
        return 0;
    }
    value = Ini_GetValue(ini, "gametype");
    if (value == NULL) {
        Error_Printf("Can't get gametype.\n");
        return 0;
    }
    c = Char_ToLower(*value);
    g = g_game;
    if (c == 'm') {
        g->game_type_m = 1;
    } else if (c == 'v') {
        g->game_type_m = 0;
    } else {
        Error_Printf("Invalid gametype: %s\n", value);
        return 0;
    }
    g->attack_life_to_power_mul = 3.0f;
    g->gethit_life_to_power_mul = 2.0f;
    g->super_target_defence_mul = 1.0f;
    value = Ini_GetValue(ini, "default.attack.lifetopowermul");
    if (value != NULL) {
        g->attack_life_to_power_mul = (float)atof(value);
    }
    value = Ini_GetValue(ini, "default.gethit.lifetopowermul");
    if (value != NULL) {
        g_game->gethit_life_to_power_mul = (float)atof(value);
    }
    value = Ini_GetValue(ini, "super.targetdefencemul");
    if (value != NULL) {
        g_game->super_target_defence_mul = (float)atof(value);
    }

    /* [Config] */
    if (Ini_SelectGroup(ini, "config") == -1) {
        Error_Printf("Can't access [Config]\n");
        return 0;
    }
    g = g_game;
    g->game_speed = 60;
    g->draw_shadows = 1;
    width = 640;
    height = 480;
    text[0] = '\0';
    Ini_ReadString(ini, "language", 0x100, text);
    if (text[0] == '\0') {
        const char *lang;
        if (GetLocaleInfoA(LOCALE_USER_DEFAULT, LOCALE_SISO639LANGNAME, g_systemLanguage, 3) != 0) {
            Log_Printf("Autodetected system language. ");
            lang = g_systemLanguage;
        } else {
            Log_Printf("Unable to detect system language. ");
            lang = "en";
        }
        text[0] = '\0';
        strncat(text, lang, 0xFF);
    }
    Log_Printf("Setting language \"%s\".\n", text);
    Lang_SetCode(text);
    Ini_GetInt(ini, "gamewidth", &width);
    Ini_GetInt(ini, "gameheight", &height);
    g = g_game;
    g->game_width = width;
    g->game_height = height;
    g->game_coord[0] = (double)width;
    g->game_coord[1] = (double)height;
    g->game_coord[2] = (double)((width - 1) / 2);
    g->game_coord[3] = (double)((height - 1) / 2);
    memcpy(g_gameCoord, g->game_coord, sizeof(g_gameCoord));
    Ini_GetInt(ini, "gamespeed", &g->game_speed);
    Ini_GetInt(ini, "drawshadows", &g_game->draw_shadows);
    Ini_GetInt(ini, "afterimagemax", &g_game->afterimage_max);
    if (g_game->afterimage_max < 1) {
        g_game->afterimage_max = 1;
    }
    Ini_GetInt(ini, "layeredspritemax", &g_game->layered_sprite_max);
    if (g_game->layered_sprite_max < 32) {
        g_game->layered_sprite_max = 32;
    }
    Ini_GetInt(ini, "spritedecompressionbuffersize", &g_game->sprite_decompression_buffer_size);
    if (g_game->sprite_decompression_buffer_size < 128) {
        g_game->sprite_decompression_buffer_size = 128;
    }
    Ini_GetInt(ini, "explodmax", &g_game->explod_max);
    if (g_game->explod_max < 8) {
        g_game->explod_max = 8;
    }
    Ini_GetInt(ini, "sysexplodmax", &g_game->sysexplod_max);
    if (g_game->sysexplod_max < 8) {
        g_game->sysexplod_max = 8;
    }
    Ini_GetInt(ini, "helpermax", &g_game->helper_max);
    if (g_game->helper_max < 4) {
        g_game->helper_max = 4;
    }
    if (g_game->helper_max > 56) {
        g_game->helper_max = 56;
    }
    Ini_GetInt(ini, "playerprojectilemax", &g_game->player_projectile_max);
    if (g_game->player_projectile_max < 5) {
        g_game->player_projectile_max = 5;
    }
    Ini_GetInt(ini, "playerpalmax", &g_game->player_pal_max);
    if (g_game->player_pal_max < 12) {
        g_game->player_pal_max = 12;
    }
    Ini_GetInt(ini, "firstrun", &g_game->first_run);

    /* [Debug] */
    if (Ini_SelectGroup(ini, "Debug") != -1) {
        Ini_GetInt(ini, "debug", &g_game->debug);
        Ini_GetInt(ini, "allowdebugmode", &g_game->allow_debug_mode);
        Ini_GetInt(ini, "allowdebugkeys", &g_game->allow_debug_keys);
        if (g_game->debug) {
            g_game->allow_debug_mode = 1;
        }
        Ini_GetInt(ini, "speedup", &g_game->speedup);
        value = Ini_GetValue(ini, "startstage");
        if (value != NULL) {
            strncpy(g_game->start_stage, value, sizeof(g_game->start_stage));
        }
        Ini_GetInt(ini, "HideDevelopmentBuildBanner", &g_game->hide_dev_banner);
    }

    /* [Video] */
    if (Ini_SelectGroup(ini, "video") == -1) {
        Error_Printf("Can't access [Video]\n");
        return 0;
    }
    g = g_game;
    g->video_width = width;
    g->video_flags = 0;
    g->video_height = height;
    g->keep_aspect = 1;
    g->filter_indexed = 1;
    Ini_GetInt(ini, "width", &g->video_width);
    Ini_GetInt(ini, "height", &g_game->video_height);
    Ini_GetInt(ini, "depth", &g_game->depth);
    Ini_GetInt(ini, "vretrace", &g_game->vretrace);
    Ini_GetInt(ini, "keepaspect", &g_game->keep_aspect);
    Ini_GetInt(ini, "filterindexed", &g_game->filter_indexed);
    Ini_GetInt(ini, "fullscreen", &g_game->fullscreen);
    g = g_game;
    if (g->fullscreen == 0) {
        g->video_flags &= ~VIDEO_FULLSCREEN;
    } else {
        g->video_flags |= VIDEO_FULLSCREEN;
    }
    g->render_mode = RENDER_SYSTEM;
    value = Ini_GetValue(ini, "rendermode");
    if (value != NULL) {
        if (_stricmp(value, "system") == 0) {
            g_game->render_mode = RENDER_SYSTEM;
        } else if (_stricmp(value, "opengl") == 0) {
            g_game->render_mode = RENDER_OPENGL;
        } else if (_stricmp(value, "openglscreen") == 0) {
            g_game->render_mode = RENDER_OPENGL_SCREEN;
        } else if (_stricmp(value, "directx") == 0) {
            g_game->video_flags |= VIDEO_DIRECTX;
            g_game->render_mode = RENDER_SYSTEM;
        } else {
            Error_Printf("Invalid RenderMode: %s\n", value);
            return 0;
        }
    }
    g = g_game;
    if (g->render_mode == RENDER_OPENGL) {
        Ini_GetInt(ini, "safemode", &g->safe_mode);
    }
    value = Ini_GetValue(ini, "blitmode");
    if (value != NULL && (c = Char_ToLower(*value)) != 'n') {
        if (c != 'p') {
            Error_Printf("Invalid BlitMode: %s\n", value);
            return 0;
        }
        g_game->video_flags |= VIDEO_HWBLIT;
    }
    flag = 0;
    Ini_GetInt(ini, "resizable", &flag);
    if (flag) {
        g_game->video_flags |= VIDEO_RESIZABLE;
    }
    Ini_GetInt(ini, "stagefit", &g_game->stage_fit);
    Ini_GetInt(ini, "systemfit", &g_game->system_fit);
    g = g_game;
    i = g->render_mode;
    if (i == RENDER_SYSTEM || i == RENDER_OPENGL_SCREEN) {
        g->depth = 16;
        g->stage_fit = 0;
        g->system_fit = 0;
    }
    if (i == RENDER_OPENGL) {
        g->video_flags |= VIDEO_HWBLIT;
        g->depth = 32;
    }
    if (g->safe_mode) {
        g->video_flags &= ~VIDEO_RESIZABLE;
        g->stage_fit = 0;
        g->system_fit = 0;
        if (i == RENDER_OPENGL) {
            g->safe_mode_opengl = 1;
        }
    }

    /* [Sound] */
    if (Ini_SelectGroup(ini, "sound") == -1) {
        Error_Printf("Can't access [Sound]\n");
        return 0;
    }
    g = g_game;
    g->wav_volume = 900.0;      /* overwritten below: only the default argument counts */
    g->sound_enabled = 1;
    g->master_volume = 1000.0;
    g->stereo_effects = 1;
    g->panning_width = 160;
    g->bgm_volume = 800.0;
    Ini_GetInt(ini, "sound", &g->sound_enabled);
    Ini_GetInt(ini, "stereoeffects", &g_game->stereo_effects);
    Ini_GetInt(ini, "panningwidth", &g_game->panning_width);
    if (Ini_GetInt(ini, "reversestereo", &flag)) {
        g_stereoDirection = flag == 0 ? 1 : -1;
    }
    g_game->sfx_resample_method = 1;
    value = Ini_GetValue(ini, "SFXResampleMethod");
    if (value != NULL) {
        if (_stricmp(value, "libresample") == 0) {
            g_game->sfx_resample_method = 1;
        } else if (_stricmp(value, "SDL") == 0) {
            g_game->sfx_resample_method = 0;
        }
    }
    if (g_game->sfx_resample_method == 1) {
        Ini_GetIntDef(ini, "SFXResampleQuality", &g_game->sfx_resample_quality, 0);
    }
    g_game->bgm_resample_method = 1;
    value = Ini_GetValue(ini, "BGMResampleMethod");
    if (value != NULL) {
        if (_stricmp(value, "libresample") == 0) {
            g_game->bgm_resample_method = 1;
        } else if (_stricmp(value, "SDL") == 0) {
            g_game->bgm_resample_method = 0;
        }
    }
    if (g_game->bgm_resample_method == 1) {
        Ini_GetIntDef(ini, "BGMResampleQuality", &g_game->bgm_resample_quality, 0);
    }
    Ini_GetDoubleDef(ini, "mastervolume", &g_game->master_volume, 100.0);
    Ini_GetDoubleDef(ini, "wavvolume", &g_game->wav_volume, 100.0);
    Ini_GetDoubleDef(ini, "bgmvolume", &g_game->bgm_volume, 100.0);
    Ini_GetIntDef(ini, "samplerate", &g_game->sample_rate, 44100);
    Ini_GetIntDef(ini, "channels", &g_game->channels, 2);
    Ini_GetIntDef(ini, "bitdepth", &g_game->bit_depth, 16);
    Ini_GetIntDef(ini, "buffersize", &g_game->buffer_size, 11);
    g = g_game;
    g->unused_128E8 = -1;
    g->wav_channels = 8;
    Ini_GetInt(ini, "wavchannels", &g->wav_channels);
    g_game->unused_128EC = -1;
    if (Ini_SelectGroup(ini, "music") == -1) {
        g_game->music_plugins = NULL;
    } else {
        g_game->music_plugins = BgmPlugins_Load(ini);
    }

    /* [Misc] */
    if (Ini_SelectGroup(ini, "Misc") == -1) {
        Error_Printf("Can't access [Misc]\n");
        return 0;
    }
    g_game->player_cache = 0;
    Ini_GetInt(ini, "playercache", &g_game->player_cache);
    g_game->player_cache += 2;
    g_game->precache = 0;
    Ini_GetInt(ini, "precache", &g_game->precache);
    if (g_game->render_mode == RENDER_OPENGL) {
        g_game->precache = 0;
    }
    if (Ini_GetInt(ini, "bufferedread", &flag)) {
        g_bufferedRead = flag;
        g_sndBufferedRead = flag;
        g_game->buffered_read = flag;
    }
    Ini_GetInt(ini, "unloadsystem", &g_game->unload_system);
    Ini_GetInt(ini, "pauseondefocus", &g_game->pause_on_defocus);
    g_game->sfx_background_mode = 1;
    value = Ini_GetValue(ini, "sfxbackgroundmode");
    if (value != NULL) {
        if (_stricmp(value, "mute") == 0) {
            g_game->sfx_background_mode = 0;
        } else if (_stricmp(value, "play") == 0) {
            g_game->sfx_background_mode = 1;
        }
    }
    g_game->bgm_background_mode = 2;
    value = Ini_GetValue(ini, "bgmbackgroundmode");
    if (value != NULL) {
        if (_stricmp(value, "pause") == 0) {
            g_game->bgm_background_mode = 0;
        } else if (_stricmp(value, "play") == 0) {
            g_game->bgm_background_mode = 2;
        } else if (_stricmp(value, "mute") == 0) {
            g_game->bgm_background_mode = 1;
        }
    }
    Ini_GetFloat(ini, "forcestagezoomout", &g_game->force_stage_zoomout);

    /* [Arcade] */
    if (Ini_SelectGroup(ini, "Arcade") == -1) {
        Error_Printf("Can't access [Arcade]\n");
        return 0;
    }
    Ini_GetInt(ini, "ai.randomcolor", &g_game->ai_random_color);
    Ini_GetInt(ini, "ai.cheat", &g_game->ai_cheat);
    for (i = 0; i < 3; i++) {
        AiRamp *ramp = &g_game->ai_ramp[i];
        sprintf(text, "%s.AIramp.start", ai_ramp_modes[i]);
        Ini_Scanf(ini, text, "%i,%i", &ramp->start_match, &ramp->start_offset);
        sprintf(text, "%s.AIramp.end", ai_ramp_modes[i]);
        Ini_Scanf(ini, text, "%i,%i", &ramp->end_match, &ramp->end_offset);
    }

    /* [Input] */
    if (Ini_SelectGroup(ini, "Input") == -1) {
        Error_Printf("Can't access [Input]\n");
        return 0;
    }
    Ini_GetInt(ini, "p1.usekeyboard", &g_game->use_keyboard[0]);
    Ini_GetInt(ini, "p2.usekeyboard", &g_game->use_keyboard[1]);
    Ini_GetInt(ini, "p1.joystick.type", &g_game->joystick_type[0]);
    Ini_GetInt(ini, "p2.joystick.type", &g_game->joystick_type[1]);
    g = g_game;
    g->use_joystick[0] = g->joystick_type[0] != 0;
    g->use_joystick[1] = g->joystick_type[1] != 0;
    g->force_feedback = g->use_joystick[0] || g->use_joystick[1];
    Ini_GetInt(ini, "forcefeedback", &g->force_feedback);
    Ini_GetInt(ini, "psxdelay", &g_game->psx_delay);
    Ini_GetInt(ini, "inputdirmode", &g_game->input_dir_mode);
    Ini_GetInt(ini, "anykeyunpauses", &g_game->any_key_unpauses);
    if (g_game->use_keyboard[0] && !Config_ReadKeys(ini, "p1 keys", g_game->keys[0])) {
        Error_Printf("Can't read p1 key config.\n");
        return 0;
    }
    g = g_game;
    Config_FillUnassigned(g->keys[0], KEY_NONE);
    if (g->use_keyboard[1]) {
        int ok = Config_ReadKeys(ini, "p2 keys", g_game->keys[1]);
        g = g_game;
        if (!ok) {
            Error_Printf("Can't read p2 key config.\n");
            return 0;
        }
    }
    Config_FillUnassigned(g->keys[1], KEY_NONE);
    g->joy_keys[0][KEY_JUMP] = 1;
    g->joy_keys[0][KEY_CROUCH] = 2;
    g->joy_keys[0][KEY_LEFT] = 3;
    g->joy_keys[0][KEY_RIGHT] = 4;
    memcpy(g->joy_keys[1], g->joy_keys[0], sizeof(g->joy_keys[1]));
    sprintf(text, "P1 joystick");
    Config_ReadKeys(ini, text, g_game->joy_keys[0]);
    sprintf(text, "P2 joystick");
    Config_ReadKeys(ini, text, g_game->joy_keys[1]);
    g = g_game;
    memcpy(g->joy_keys_active[0], g->joy_keys[0], sizeof(g->joy_keys_active[0]));
    memcpy(g->joy_keys_active[1], g->joy_keys[1], sizeof(g->joy_keys_active[1]));
    if (!g->use_joystick[0]) {
        memset(g->joy_keys_active[0], 0, sizeof(g->joy_keys_active[0]));
    }
    if (!g->use_joystick[1]) {
        memset(g->joy_keys_active[1], 0, sizeof(g->joy_keys_active[1]));
    }
    Config_FillUnassigned(g->joy_keys_active[0], JOY_NONE);
    Config_FillUnassigned(g->joy_keys_active[1], JOY_NONE);
    Config_FillUnassigned(g->joy_keys[0], JOY_NONE);
    Config_FillUnassigned(g->joy_keys[1], JOY_NONE);
    Ini_Free(ini);

    /* Clear FirstRun once it has been seen. */
    if (g_game->first_run) {
        ini = Ini_OpenEditable(CONFIG_PATH);
        if (ini != NULL) {
            Ini_SelectGroup(ini, "Config");
            Ini_SetValuef(ini, "FirstRun", "0");
            Ini_Save(ini, CONFIG_PATH);
            Ini_Free(ini);
        }
    }
    Ini_SetLanguage(g_language);
    return 1;
}

/* 0x0045F100: option defaults; the volume percentages come from the configured volumes. */
void Options_SetDefaults(void)
{
    GameVars *g = g_game;

    g->difficulty = 4;
    g->life = 100;
    g->time = 99;
    g->option_game_speed = 0;
    g->master_volume_pct = (int)(g->master_volume + 0.5);
    g->wav_volume_pct = (int)(g->wav_volume + 0.5);
    g->bgm_volume_pct = (int)(g->bgm_volume + 0.5);
}

/* Options menu encoding of a volume: 101 for 100%, -1 for 0%. */
static int Options_VolumeToPct(double volume)
{
    if (volume == 100.0) {
        return 101;
    }
    if (volume == 0.0) {
        return -1;
    }
    return (int)(volume + 0.5);
}

/* Inverse of Options_VolumeToPct, keeping the fractional part of the current volume. */
static double Options_PctToVolume(int pct, double volume)
{
    if (pct == -1) {
        return 0.0;
    }
    if (pct == 101) {
        return 100.0;
    }
    return (volume - floor(volume + 0.5)) + pct;
}

/* 0x0045F170: reloads the options screen settings from mugen.cfg and reinitialises input. */
void Options_Load(void)
{
    GameVars *g;
    const char *value;
    Stream *s;
    Ini *ini = NULL;

    s = Stream_OpenFile(CONFIG_PATH, "rt");
    if (s != NULL) {
        ini = Ini_Parse(s, INI_MODE_READ);
        Stream_Close(s);
    }
    if (ini == NULL) {
        Error_Fatal("Error reading %s\n", CONFIG_PATH);
    }
    if (Ini_SelectGroup(ini, "Options") != -1) {
        if ((value = Ini_GetValue(ini, "Difficulty")) != NULL) {
            g_game->difficulty = atoi(value);
        }
        if ((value = Ini_GetValue(ini, "Life")) != NULL) {
            g_game->life = atoi(value);
        }
        if ((value = Ini_GetValue(ini, "Time")) != NULL) {
            g_game->time = atoi(value);
        }
        if ((value = Ini_GetValue(ini, "GameSpeed")) != NULL) {
            g_game->option_game_speed = atoi(value);
        }
        Ini_ReadString(ini, "motif", sizeof(g_game->motif), g_game->motif);
        g = g_game;
        if (g->motif[0] == '\0') {
            strcpy(g->motif, "data/system.def");
        }
        Path_Split(g->motif, g->motif_dir, g->motif_file);
        if ((value = Ini_GetValue(ini, "Team.1VS2Life")) != NULL) {
            g_game->team_1vs2_life = atoi(value);
        }
        if ((value = Ini_GetValue(ini, "Team.LoseOnKO")) != NULL) {
            g_game->team_lose_on_ko = atoi(value);
        }
    }
    g = g_game;
    if (Ini_SelectGroup(ini, "Sound") != -1) {
        g->master_volume_pct = Options_VolumeToPct(g->master_volume);
        g->wav_volume_pct = Options_VolumeToPct(g->wav_volume);
        g->bgm_volume_pct = Options_VolumeToPct(g->bgm_volume);
    }
    g->guard_ko = 1;
    Config_ReadKeys(ini, "p1 keys", g->keys[0]);
    Config_ReadKeys(ini, "p2 keys", g_game->keys[1]);
    g = g_game;
    Config_FillUnassigned(g->keys[0], KEY_NONE);
    Config_FillUnassigned(g->keys[1], KEY_NONE);
    if (Ini_SelectGroup(ini, "Input") != -1) {
        if ((value = Ini_GetValue(ini, "p1.joystick.type")) != NULL) {
            g_game->option_input[0][1] = atoi(value);
        }
        if ((value = Ini_GetValue(ini, "p2.joystick.type")) != NULL) {
            g_game->option_input[1][1] = atoi(value);
        }
    }
    g = g_game;
    g->joystick_type[0] = g->option_input[0][1];
    g->joystick_type[1] = g->option_input[1][1];
    g->use_joystick[0] = g->option_input[0][1] != 0;
    g->use_joystick[1] = g->option_input[1][1] != 0;
    Config_ReadKeys(ini, "P1 joystick", g->joy_keys[0]);
    Config_ReadKeys(ini, "P2 joystick", g_game->joy_keys[1]);
    g = g_game;
    Config_FillUnassigned(g->joy_keys[0], JOY_NONE);
    Config_FillUnassigned(g->joy_keys[1], JOY_NONE);
    Ini_Free(ini);
    MfHost_KeysLoaded();        /* Moemon Fight: the players' buttons arrive as fixed keys */
    Input_InitPads();
    if (!Input_Remap()) {
        Error_Fatal("Cannot init input\n");
    }
}

/* Writes a key map, with the "none" value written as 0. */
static void Options_SaveKeys(Ini *ini, const char *group, const int *keys, int none)
{
    int copy[KEY_COUNT];
    int i;

    memcpy(copy, keys, sizeof(copy));
    for (i = 1; i < KEY_COUNT; i++) {
        if (copy[i] == none) {
            copy[i] = 0;
        }
    }
    if (Ini_SelectGroup(ini, group) != -1) {
        Config_WriteKeys(ini, copy);
    }
}

/* 0x0045F5B0: writes the options screen settings back to mugen.cfg. */
void Options_Save(void)
{
    GameVars *g;
    Stream *s;
    Ini *ini = NULL;

    s = Stream_OpenFile(CONFIG_PATH, "rt");
    if (s != NULL) {
        ini = Ini_Parse(s, INI_MODE_EDIT);
        Stream_Close(s);
    }
    if (ini == NULL) {
        Error_Fatal("Error saving %s\n", CONFIG_PATH);
    }
    if (Ini_SelectGroup(ini, "Options") != -1) {
        Ini_SetValuef(ini, "Difficulty", "%i", g_game->difficulty);
        Ini_SetValuef(ini, "Life", "%i", g_game->life);
        Ini_SetValuef(ini, "Time", "%i", g_game->time);
        Ini_SetValuef(ini, "GameSpeed", "%i", g_game->option_game_speed);
        Ini_SetValuef(ini, "Team.1VS2Life", "%i", g_game->team_1vs2_life);
        Ini_SetValuef(ini, "Team.LoseOnKO", "%i", g_game->team_lose_on_ko);
    }
    if (Ini_SelectGroup(ini, "Sound") != -1) {
        double old_bgm, new_bgm;
        g = g_game;
        old_bgm = g->bgm_volume * g->master_volume / 100.0;
        g->master_volume = Options_PctToVolume(g->master_volume_pct, g->master_volume);
        Ini_SetValuef(ini, "MasterVolume", "%g", g->master_volume);
        g = g_game;
        g->wav_volume = Options_PctToVolume(g->wav_volume_pct, g->wav_volume);
        Ini_SetValuef(ini, "WavVolume", "%g", g->wav_volume);
        g = g_game;
        g->bgm_volume = Options_PctToVolume(g->bgm_volume_pct, g->bgm_volume);
        new_bgm = g->master_volume * g->bgm_volume / 100.0;
        if (new_bgm != old_bgm) {
            Bgm_SetVolume(new_bgm);
            g = g_game;
        }
        Ini_SetValuef(ini, "BGMVolume", "%g", g->bgm_volume);
    }
    if (Ini_SelectGroup(ini, "Input") != -1) {
        Ini_SetValuef(ini, "p1.joystick.type", "%i", g_game->option_input[0][1]);
        Ini_SetValuef(ini, "p2.joystick.type", "%i", g_game->option_input[1][1]);
    }
    Options_SaveKeys(ini, "p1 keys", g_game->keys[0], KEY_NONE);
    Options_SaveKeys(ini, "p2 keys", g_game->keys[1], KEY_NONE);
    Options_SaveKeys(ini, "p1 joystick", g_game->joy_keys[0], JOY_NONE);
    Options_SaveKeys(ini, "p2 joystick", g_game->joy_keys[1], JOY_NONE);
    if (!Ini_Save(ini, CONFIG_PATH)) {
        Error_Fatal("Error saving %s\n", CONFIG_PATH);
    }
    Ini_Free(ini);
}
