#ifndef MUGEN_GAME_H
#define MUGEN_GAME_H

/*
 * The "game variables" block, allocated once by System_Init (calloc 0x13108).
 * Only the fields understood so far are named; the rest is padding named after
 * its offset. Field sizes marked "?" are not verified yet.
 */
/* Indices into the key/button arrays (index 0 and 12 unused). */
enum {
    KEY_JUMP = 1,
    KEY_RIGHT = 2,
    KEY_CROUCH = 3,
    KEY_LEFT = 4,
    KEY_A = 5,
    KEY_B = 6,
    KEY_C = 7,
    KEY_X = 8,
    KEY_Y = 9,
    KEY_Z = 10,
    KEY_START = 11,
    KEY_COUNT = 13
};

/* Key value meaning "unassigned" in keyboard (KEY_NONE) and joystick (JOY_NONE) maps. */
#define KEY_NONE 0x200
#define JOY_NONE 0x80

/* GameVars.render_mode ([Video] RenderMode). */
enum {
    RENDER_SYSTEM = 0,          /* also DirectX (with VIDEO_DIRECTX) */
    RENDER_OPENGL_SCREEN = 1,
    RENDER_OPENGL = 2
};

/* GameVars.video_flags. */
#define VIDEO_DIRECTX    0x01
#define VIDEO_FULLSCREEN 0x02
#define VIDEO_HWBLIT     0x04   /* BlitMode = PageFlip, or OpenGL */
#define VIDEO_RESIZABLE  0x10

/* AI ramp of a game mode ([Arcade] <mode>.AIramp.start/end = match, offset). */
typedef struct AiRamp {
    int start_match;
    int start_offset;
    int end_match;
    int end_offset;
} AiRamp;

/* GameVars.sfx_background_mode / g_bgmBackgroundMode values. */
enum {
    SFX_BG_MUTE = 0,
    SFX_BG_PLAY = 1
};
enum {
    BGM_BG_PAUSE = 0,
    BGM_BG_MUTE = 1,
    BGM_BG_PLAY = 2
};

/* Int array (IntArray_Init, 0x46B770), also used as a ring buffer (g_keyQueue). */
typedef struct IntArray {
    int *data;              /* 0x00 */
    int read;               /* 0x04 ring buffer: next entry to take */
    int write;              /* 0x08 ring buffer: next entry to fill */
    int capacity;           /* 0x0C */
} IntArray;
MUGEN_ASSERT_SIZE(IntArray, 0x10);

/* What an AfterImage controller sets (AfterImage.settings, AfterImage_Apply). */
typedef struct AfterImageSettings {
    int time;                       /* 0x00 */
    int length;                     /* 0x04 */
    int timegap;                    /* 0x08 */
    int framegap;                   /* 0x0C */
    int palcolor;                   /* 0x10 */
    int palinvertall;               /* 0x14 */
    int palbright[3];               /* 0x18 */
    int palcontrast[3];             /* 0x24 */
    int palpostbright[3];           /* 0x30 */
    int paladd[3];                  /* 0x3C */
    float palmul[3];                /* 0x48 */
    int trans;                      /* 0x54 */
    int alpha[2];                   /* 0x58 */
} AfterImageSettings;
MUGEN_ASSERT_OFFSET(AfterImageSettings, palmul, 0x48);
MUGEN_ASSERT_SIZE(AfterImageSettings, 0x60);

/*
 * An afterimage buffer (AfterImage_Init, 0x401000); GameVars.afterimages holds
 * them by (0, index). Player.afterimage and Projectile.afterimage point to the one
 * they use; running and anim are cleared when they let go of it.
 */
typedef struct AfterImage {
    void *frames;       /* 0x00 calloc'ed 0x40B0 bytes */
    int max_length;     /* 0x04 60: frames the buffer holds */
    struct AnimState *anim; /* 0x08 animation followed, NULL when the buffer is free (AfterImages_FindFree) */
    int oldest;         /* 0x0C oldest recorded frame kept (ring index) */
    int next;           /* 0x10 next frame written (ring index) */
    int ticks;          /* 0x14 ticks since the afterimage started (AfterImageTime clears it) */
    int running;        /* 0x18 1 once started by AfterImage_Apply, 0 when it ends */
    AfterImageSettings settings; /* 0x1C */
} AfterImage;
MUGEN_ASSERT_OFFSET(AfterImage, settings, 0x1C);
MUGEN_ASSERT_SIZE(AfterImage, 0x7C);

/* Sprite decompression buffer (DecompBuffer_Create, 0x48C7B0), g_spriteDecompBuffer. */
typedef struct DecompBuffer {
    unsigned int size;      /* 0x00 rounded up to 1 KB */
    void *data;             /* 0x04 DecompBlock list (spritecodec.h) */
    int clock;              /* 0x08 base of the blocks' expire values (the evicted blocks' total) */
    int hits;               /* 0x0C DecompBuffer_Get found the block */
    int misses;             /* 0x10 */
    int block_count;        /* 0x14 blocks, free and used (not the end marker) */
    int used_count;         /* 0x18 */
    int free_count;         /* 0x1C */
    int evictions;          /* 0x20 */
    int used_bytes;         /* 0x24 sizes of the used blocks */
} DecompBuffer;
MUGEN_ASSERT_SIZE(DecompBuffer, 0x28);

/* A combo counter of one team (GameVars.combo). */
typedef struct ComboState {
    int timer;              /* 0x00 ticks the counter stays shown ([Combo] displaytime), 0 = hidden */
    int hits;               /* 0x04 hits of the current combo */
    int pad_08;             /* 0x08 never accessed by the game */
    int shown;              /* 0x0C hits shown, at most 999 */
    int x;                  /* 0x10 x offset of the counter ([Combo] start.x when it appears) */
    int ongoing;            /* 0x14 a target of the team is still being hit this tick: the combo goes on (else hits restart) */
} ComboState;
MUGEN_ASSERT_SIZE(ComboState, 0x18);

/* The face sprites of a team's members, as Match_LoadAssets sets them up (GameVars.team_faces). */
typedef struct TeamFaces {
    SpriteSet *sprites[4];          /* 0x00 by member */
    float scale[4];                 /* 0x10 [Select Info] cell scale of each */
} TeamFaces;
MUGEN_ASSERT_SIZE(TeamFaces, 0x20);

typedef struct GameVars {
    char game_dir[0x200];                  /* 0x00000 directory restored on exit (set with -gamedir) */
    char cwd[0x200];                       /* 0x00200 start-up working directory, ending in '/' */
    int cwd_writable;                      /* 0x00400 a file could be created in the working directory */
    unsigned int compat_version[3];        /* 0x00404 mugenversion thresholds (GameVars_Init): [0] 1.1 (constants), [1] 1.0 (most
                                              behaviour), [2] 1.1 (ScreenBound, Explod postype) */
    char select_stage[0x200];              /* 0x00410 stage chosen on the select screen */
    char arcade_music[0x200];              /* 0x00610 music= of the arcade opponent's select.def line, played instead of
                                              the stage BGM (Gameflow_PickStage, Stage_Activate) */
    FontSpec infobox_font;                 /* 0x00810 system.def [Infobox] text.font */
    char infobox_text[0x1000];             /* 0x0082C [Infobox Text] lines, shown above the version text */
    TitleInfo title_info;                  /* 0x0182C system.def [Title Info] */
    OptionInfo option_info;                /* 0x01A14 system.def [Option Info] */
    Stage stage;                           /* 0x01AE0 the loaded stage */
    CharInfo *char_info;                   /* 0x020A0 select.def characters (freed at exit) */
    int char_count;                        /* 0x020A4 */
    int chars_cached;                      /* 0x020A8 characters loaded in the player cache (PlayerCache_Load) */
    int char_capacity;                     /* 0x020AC entries allocated in char_info */
    int *char_info_2;                      /* 0x020B0 arcade order (ArcadeOrder.chars), freed with char_info */
    int arcade_order_count;                /* 0x020B4 ArcadeOrder.count */
    int arcade_order_next;                 /* 0x020B8 ArcadeOrder.next */
    int arcade_order_wins;                  /* 0x020BC ArcadeOrder.wins */
    int arcade_maxmatches[30];             /* 0x020C0 select.def [Options] arcade.maxmatches, by order */
    int team_maxmatches[30];               /* 0x02138 [Options] team.maxmatches */
    int precache_enabled;                  /* 0x021B0 first field of the precache thread state */
    pthread_mutex_t precache_mutex;        /* 0x021B4 pthread_mutex_t (a pointer in pthreads-win32; Moemon Fight: the real type) */
    int precache_running;                  /* 0x021B8 started by Precache_Start, cleared by Precache_Stop */
    int precache_state;                    /* 0x021BC 0 no thread, 1 thread started, 2 go on, 3 loading */
    int precache_done;                     /* 0x021C0 the thread went through its queue */
    int precache_stop;                     /* 0x021C4 stop after the current character */
    int precache_queue[5];                 /* 0x021C8 characters to load, -1 terminated */
    SelectInfo select_info;                /* 0x021DC system.def [Select Info] */
    char logo_storyboard[0x200];           /* 0x03580 system.def [Files] logo.storyboard */
    char intro_storyboard[0x200];          /* 0x03780 system.def [Files] intro.storyboard */
    int intro_played;                      /* 0x03980 the intro storyboard was played (Gameflow_Run; cleared after a game over or the credits) */
    MotifStoryboard end_credits;           /* 0x03984 system.def [End Credits] */
    MotifStoryboard default_ending;        /* 0x03B88 system.def [Default Ending] */
    char ending_storyboard[0x200];         /* 0x03D8C ending storyboard of the arcade character (Match_Run), played by Gameflow_Run */
    WinScreenInfo win_screen;              /* 0x03F8C system.def [Win Screen] */
    SurvivalResultsInfo survival_results;  /* 0x04048 system.def [Survival Results Screen] */
    ContinueInfo continue_screen;          /* 0x04108 system.def [Continue Screen] */
    MotifStoryboard game_over;             /* 0x04670 system.def [Game Over Screen] */
    VsScreenInfo vs_screen;                /* 0x04874 system.def [VS Screen] */
    VictoryInfo victory_screen;            /* 0x04ADC system.def [Victory Screen] */
    char victory_quote[0x400];             /* 0x04C70 victory quote of the match winner (Match_Unload), cleared by Match_Start, shown by the victory screen; size not verified */
    StageInfo *select_screen;              /* 0x05070 stage list (freed at exit unless quick VS) */
    int stage_count;                       /* 0x05074 */
    int team_variant;                      /* 0x05078 1 survival modes, 2 watch, 3 training (Charsel_SetupMode); 0 at start-up */
    int player_sides[4];                   /* 0x0507C side of player 1..4 (i & 1, QuickVs_SetupTeams, Demo_SetupTeams);
                                              1, 2 at start-up; [0] == [1]: anyone may answer the continue screen */
    int arcade_side;                       /* 0x0508C arcade modes: side of the human team + 1, else 0 */
    int survival_side;                     /* 0x05090 survival modes: side of the human team + 1, else 0 */
    int game_mode;                         /* 0x05094 GAME_MODE_* */
    int game_mode_side;                    /* 0x05098 side that chose the mode */
    int screen_kind;                       /* 0x0509C ScreenLoop.kind of the screen being run (Screen_Run); 6, 7, 8: no round intro / KO skipping by the players */
    TeamSel teams[2];                      /* 0x050A0 teams chosen on the select screen */
    TeamMember match_members[2];           /* 0x08658 the member of each team in the match (TeamSel.members entry);
                                              mode: value of the TeamMode trigger (0 single, 1 simul, 3 turns) */
    int continued;                         /* 0x08710 the continue screen was answered yes: the select screen keeps the teams (no input, no sounds) */
    int continue_reselect;                 /* 0x08714 1 at start-up: after a continue, back to the select screen (step 7)
                                              instead of straight to the match (10) */
    int unused_8718[6];                    /* 0x08718 cleared by Gameflow_SetupMatch; [1] / [4] get the rounds won at the end
                                              of a match, never read */
    SpriteSet *screen;                     /* 0x08730 main render target (Screen_Create; none in OpenGL safe mode) */
    SpriteSet *render_target;              /* 0x08734 current target, the screen at start-up */
    struct Font *font_small;               /* 0x08738 font/f-4x6.def */
    struct Font *font_medium;              /* 0x0873C font/f-6x9.def */
    struct Font *options_font;             /* 0x08740 font of the options screen (Options_GetFont), freed unless it is font_medium */
    struct AnimSet *anims;                 /* 0x08744 "persistent animation array" */
    struct SpriteQueue *sprite_layer1;     /* 0x08748 "priority sprites layer 1" (explods and the select screen draw here) */
    void *palgroups;                       /* 0x0874C "palgroup array" (calloc 0xC) */
    KeyedArray *afterimages;               /* 0x08750 AfterImage by (0, index), [Config] AfterImageMax */
    int draw_offset_y;                     /* 0x08754 vertical screen offset of the stage (draw positions) */
    int stage_zoffset;                     /* 0x08758 the same, set together by Stage_Activate ([StageInfo] zoffset) */
    int vretrace_on;                       /* 0x0875C vretrace in use (copy of vretrace at start-up, toggled by a debug key) */
    int envcolor_under;                    /* 0x08760 EnvColor under */
    unsigned int envcolor_color;           /* 0x08764 EnvColor colour (Video_MapRGB) */
    int envcolor_time;                     /* 0x08768 EnvColor time + 1 (-1 stays -1) */
    SpriteSet *fightfx_sff;                /* 0x0876C fight.def [Files] fightfx.sff */
    struct Anim *fightfx_anim;             /* 0x08770 fight.def [Files] fightfx.air */
    float fightfx_scale;                   /* 0x08774 fight.def [FightFx] scale */
    FightDef fight;                        /* 0x08778 the rest of fight.def */
    TeamFaces team_faces[2];               /* 0x10BDC face sprites of the members (Team_RotateMembers rotates them) */
    int hud_enabled;                       /* 0x10C1C the fight HUD is drawn (1 at start-up; toggled by the console / Lua) */
    SysDrawList *explods;                  /* 0x10C20 Explods_Create([Config] ExplodMax) */
    SysDrawList *sysexplods;               /* 0x10C24 Explods_Create([Config] SysExplodMax); system.def elements being drawn */
    char motif_file[0x200];                /* 0x10C28 file part of the motif path */
    char motif_dir[0x200];                 /* 0x10E28 directory of the motif's system.def */
    char pad_11028[0x11088 - 0x11028];
    double motif_coord[4];                 /* 0x11088 [Info] localcoord: width, height, (width-1)/2, (height-1)/2 (int halves) */
    double motif_scale;                    /* 0x110A8 game width / motif width */
    char pad_110B0[0x110B8 - 0x110B0];
    int motif_loaded;                      /* 0x110B8 the motif backgrounds are loaded (System_UnloadMotif / System_ReloadMotif) */
    SpriteSet *motif_sprites;              /* 0x110BC system.sff */
    char motif_sff_path[0x200];            /* 0x110C0 its path, for System_ReloadMotif */
    Snd *motif_snd;                        /* 0x112C0 system.snd */
    char motif_snd_path[0x200];            /* 0x112C4 its path */
    Anim *motif_anims;                     /* 0x114C4 system.def animations */
    void *motif_fonts[10];                 /* 0x114C8 system.def fonts (Font *) */
    struct Bg *versus_bg;                  /* 0x114F0 VersusBG background */
    struct Bg *victory_bg;                 /* 0x114F4 VictoryBG background (optional) */
    struct Bg *title_bg;                   /* 0x114F8 TitleBG background (ScreenLoop.bg of the title screen) */
    struct Bg *select_bg;                  /* 0x114FC SelectBG background */
    struct Bg *option_bg;                  /* 0x11500 OptionBG background */
    char pad_11504[0x11508 - 0x11504];
    BgmInfo title_bgm;                     /* 0x11508 [Music] title.bgm, given to ScreenLoop.bgm by the title screen */
    BgmInfo vs_bgm;                        /* 0x11728 [Music] vs.bgm */
    BgmInfo victory_bgm;                   /* 0x11948 [Music] victory.bgm */
    BgmInfo select_bgm;                    /* 0x11B68 [Music] select.bgm, given to ScreenLoop.bgm by the select screen */
    struct Bg *stage_bg;                   /* 0x11D88 the stage background drawn behind the fight (Match_DrawStageBack) */
    PalFxDef bg_palfx_def;                 /* 0x11D8C BGPalFX definition (PalFx_Apply) */
    PalFxDef all_palfx_def;                /* 0x11DC8 AllPalFX definition */
    PalFx all_palfx;                       /* 0x11E04 PalFX of the whole screen (AllPalFX) */
    PalFx stage_palfx;                     /* 0x11E2C PalFX of the stage (palette group 0x100) */
    PalFx palgroup_200_palfx;              /* 0x11E54 palette group 0x200 */
    int sound_enabled;                     /* 0x11E7C [Sound] Sound */
    double master_volume;                  /* 0x11E80 [Sound] MasterVolume (percent) */
    double wav_volume;                     /* 0x11E88 [Sound] WavVolume */
    double bgm_volume;                     /* 0x11E90 [Sound] BGMVolume */
    unsigned int frames;                   /* 0x11E98 frames run, logged at exit */
    int frame_skip;                        /* 0x11E9C frames skipped between drawn ones, -1 none ('f' debug key: 1, 2, then 2000 =
                                              no drawing and no sound; -stresstest) */
    int frame_skip_count;                  /* 0x11EA0 frames skipped so far (also 1 when the frame is late) */
    int skip_draw;                         /* 0x11EA4 nothing is drawn this frame; 0 at start-up */
    int no_sounds;                         /* 0x11EA8 no sound is started while set (Sound_PlaySystem, PlaySnd) */
    int frame_step;                        /* 0x11EAC set by Scroll Lock while paused: the pause comes back after the frame (Screen_Run) */
    int defocus_paused;                    /* 0x11EB0 paused by [Misc] PauseOnDefocus */
    int debug_full_power;                  /* 0x11EB4 debug: power stays full */
    unsigned int time_seed;                /* 0x11EB8 time at start-up, given to srand */
    int session_seed_arg;                  /* 0x11EBC -sessionseed */
    int match_seed_arg;                    /* 0x11EC0 CmdLineArgs.match_seed_arg, given to Match_SeedRng */
    char pad_11EC4[0x11EC8 - 0x11EC4];
    unsigned int session_seed;             /* 0x11EC8 session RNG seed (Rng_Seed) */
    int session_seed_hi;                   /* 0x11ECC high half of the seed, cleared by Rng_Seed */
    unsigned long long session_rng;        /* 0x11ED0 session RNG state (Random_Next: state * mul % mod) */
    unsigned int session_rng_mod;          /* 0x11ED8 0x7FFFFFFF */
    unsigned int session_rng_mul;          /* 0x11EDC 16807 */
    int debug_no_ko;                       /* 0x11EE0 debug: life does not drop below 1 */
    Fade fade;                             /* 0x11EE4 screen fade (Fade_Update) */
    float camera_offset_x;                 /* 0x11F00 0.0 at start-up; camera x offset (Camera_GetX adds it, the pos trigger subtracts it) */
    float camera_offset_y;                 /* 0x11F04 0.0 at start-up; camera y offset, scaled by camera_zoom (Camera_GetY) */
    float screen_left;                     /* 0x11F08 left edge of the screen (camera, moves towards screen_left_target) */
    float screen_right;                    /* 0x11F0C right edge of the screen (moves towards screen_right_target) */
    float screen_left_target;              /* 0x11F10 left edge the camera moves to (Camera_Update) */
    float screen_right_target;             /* 0x11F14 right edge the camera moves to */
    float camera_y_target;                 /* 0x11F18 camera y that camera_offset_y follows (Camera_Update) */
    float camera_x;                        /* 0x11F1C 0.0 at start-up; camera x in stage units (Camera_GetX) */
    float camera_y;                        /* 0x11F20 0.0 at start-up; camera y in stage units (Camera_GetY) */
    int camera_x_base;                     /* 0x11F24 camera x (int) subtracted from camera_x by Camera_GetX */
    int camera_y_base;                     /* 0x11F28 0 at start-up; subtracted from camera_y by Camera_GetY */
    int crop_top;                          /* 0x11F2C stage rows cut at the top (Camera_UpdateCrop) */
    int crop_bottom;                       /* 0x11F30 stage rows cut at the bottom */
    float camera_zoom;                     /* 0x11F34 1.0 at start-up; camera zoom (CameraZoom trigger) */
    float zoom_active;                     /* 0x11F38 set by the Zoom controller for a tick (draw positions use the zoom and shake_x / _y) */
    float zoom_x;                          /* 0x11F3C Zoom pos x (game units) */
    float zoom_y;                          /* 0x11F40 Zoom pos y */
    float zoom_scale;                      /* 0x11F44 Zoom scale */
    float stage_scale;                     /* 0x11F48 draw scale of the stage */
    float zoom_left;                       /* 0x11F4C left edge of the zoomed view (Camera_UpdateZoom) */
    float zoom_right;                      /* 0x11F50 right edge of the zoomed view */
    float shake_x;                         /* 0x11F54 screen shake offset x */
    float shake_y;                         /* 0x11F58 screen shake offset y */
    float zoom_lag;                        /* 0x11F5C Zoom lag */
    int envshake[6];                       /* 0x11F60 the screen's EnvShake (an EnvShake: HitDef, EnvShake, FallEnvShake; [1] time) */
    int game_type_m;                       /* 0x11F78 [Rules] GameType starts with 'm' (else "VS") */
    float attack_life_to_power_mul;        /* 0x11F7C [Rules] Default.Attack.LifeToPowerMul */
    float gethit_life_to_power_mul;        /* 0x11F80 [Rules] Default.GetHit.LifeToPowerMul */
    float super_target_defence_mul;        /* 0x11F84 [Rules] Super.TargetDefenceMul */
    int keys[2][KEY_COUNT];                /* 0x11F88 [P1 Keys], [P2 Keys]: SDL key codes */
    int input_keys[2][KEY_COUNT];          /* 0x11FF0 keys in use, built-in ones from Input_Init (0x200 flags a joystick button) */
    int joy_keys_active[2][KEY_COUNT];     /* 0x12058 joy_keys, cleared for unused joysticks */
    int joy_keys[2][KEY_COUNT];            /* 0x120C0 [P1 Joystick], [P2 Joystick] */
    int console_open;                      /* 0x12128 the console is open: Screen_Run skips the update callback and the fight keys */
    int input_blocked;                     /* 0x1212C player input is cleared while set */
    int use_keyboard[2];                   /* 0x12130 [Input] P<n>.UseKeyboard */
    int use_joystick[2];                   /* 0x12138 joystick_type != 0 */
    int joystick_type[2];                  /* 0x12140 [Input] P<n>.Joystick.type */
    int force_feedback;                    /* 0x12148 [Input] ForceFeedback */
    int psx_delay;                         /* 0x1214C [Input] PSXDelay */
    int input_dir_mode;                    /* 0x12150 [Input] InputDirMode */
    IntArray key_codes;                    /* 0x12154 keyboard codes typed this frame (Screen_Run), 32 entries */
    int player_slots;                      /* 0x12164 helper_max + 4: highest player index */
    int helper_slots;                      /* 0x12168 helper_max */
    int unused_1216C;                      /* 0x1216C 4 at start-up, never read */
    int players_active;                    /* 0x12170 players with Player.active set (Players_SetActive) */
    struct CharData *player_chars[64];     /* 0x12174 by player number: character data from the player cache (Team_CreatePlayers) */
    struct Player *players[65];            /* 0x12274 players by Player.index */
    int unused_12378[4];                   /* 0x12378 1, 1, 0, 0 at start-up and in quick VS / demo set-up, never read */
    char pad_12388[0x12474 - 0x12388];
    int players_first;                     /* 0x12474 lowest index of an active player (players[]) */
    int players_last;                      /* 0x12478 highest index of an active player, -1 none */
    struct Player *hit_players[65];        /* 0x1247C players that can collide this tick (Players_CheckHits), NULL terminated */
    int hit_player_count;                  /* 0x12580 entries in hit_players */
    int player_id_next;                    /* 0x12584 next Player.id */
    int player_tick[65];                   /* 0x12588 by Player.index: commands of the player are cleared while set; the player
                                              runs this tick (Round_Tick); [64] (0x12688): a helper was created this
                                              tick, Round_Tick runs Player_Update again */
    int input_enabled;                     /* 0x1268C player input is processed only while set */
    int hits_enabled;                      /* 0x12690 Players_CheckHits runs (set by Match_Start and the fight states of Round_RunState) */
    int hitdef_serial;                     /* 0x12694 next HitDef hit ID (HitDef.serial) */
    int screenshot_requested;              /* 0x12698 screenshot requested (F12), taken by Screen_Present */
    unsigned char assert_intro;            /* 0x1269C AssertSpecial global flags, indexed from here (11 bytes) */
    unsigned char assert_roundnotover;     /* 0x1269D */
    signed char assert_noko;               /* 0x1269E read into g_nokoLatch when a player is KOed */
    unsigned char assert_nokosnd;          /* 0x1269F */
    unsigned char assert_nokoslow;         /* 0x126A0 */
    unsigned char assert_nomusic;          /* 0x126A1 */
    unsigned char assert_globalnoshadow;   /* 0x126A2 */
    unsigned char assert_timerfreeze;      /* 0x126A3 */
    unsigned char assert_nobardisplay;     /* 0x126A4 */
    unsigned char assert_nobg;             /* 0x126A5 */
    unsigned char assert_nofg;             /* 0x126A6 */
    char pad_126A7[0x126A8 - 0x126A7];
    int window_active;                     /* 0x126A8 the game window has the focus */
    int demo_mode;                         /* 0x126AC attract mode: the computer plays the menus */
    int demo_intro_waitcycles;             /* 0x126B0 [Demo Mode] intro.waitcycles */
    int demo_wait;                         /* 0x126B4 [Demo Mode] title.waittime: idle ticks before the demo, -1 never */
    int demo_select_enabled;               /* 0x126B8 [Demo Mode] select.enabled */
    int demo_vsscreen_enabled;             /* 0x126BC [Demo Mode] vsscreen.enabled */
    int demo_fight_endtime;                /* 0x126C0 [Demo Mode] fight.endtime */
    int demo_fight_playbgm;                /* 0x126C4 [Demo Mode] fight.playbgm */
    int demo_fight_stopbgm;                /* 0x126C8 [Demo Mode] fight.stopbgm */
    int demo_fight_bars_display;           /* 0x126CC [Demo Mode] fight.bars.display */
    int demo_debuginfo;                    /* 0x126D0 [Demo Mode] debuginfo */
    int demo_ticks;                        /* 0x126D4 ticks of the running demo, cleared when one starts */
    int demo_cycles;                       /* 0x126D8 demo fights since the intro storyboard played (Demo_TickFight) */
    int pause_kind;                        /* 0x126DC the pause running: 0 Pause, 1 SuperPause, -1 none */
    int pauses[2][9];                      /* 0x126E0 [0] Pause, [1] SuperPause (Pause_Apply): [0..4] palette group masks
                                              darkened, [5] darken, [6] time, [7] pausebg, [8] endcmdbuftime; [6] < [8]:
                                              command timers stop at 2 */
    int round_no;                          /* 0x12728 round number (survival: match number) */
    int rounds_won[2];                     /* 0x1272C rounds won by team 1, 2 (index of the next Round_SetWinType entry) */
    int round_not_counted;                 /* 0x12734 the round result is not counted (Match_CheckOver), cleared by Round_Reset */
    int draws;                             /* 0x12738 draw games so far (Match_CheckOver) */
    int slow_time;                         /* 0x1273C slow motion ticks left after the KO ([Round] slow.time) */
    int slow_step;                         /* 0x12740 slow motion: added to slow_counter every tick while that is <= 0 */
    int slow_counter;                      /* 0x12744 slow motion counter (-100 a tick): the match only advances while it is 0 */
    int intro_time;                        /* 0x12748 round intro time */
    int ko_state_time;                     /* 0x1274C round_state_time when the KO state (3) ended; the over state (4) counts on from it */
    int over_end_time;                     /* 0x12750 round_state_time at which the over state ends (pushed back by roundnotover) */
    int round_state;                       /* 0x12754 round state: 0/1 intro, 3, 4 over */
    int round_winner;                      /* 0x12758 team side that won the round, 0 = none (draw) */
    int win_type;                          /* 0x1275C how the round was won: TRIGGER_WIN_KO, TRIGGER_WIN_TIME..., 0 while not over */
    int round_state_time;                  /* 0x12760 round state time */
    int camera_mode;                       /* 0x12764 camera mode: <= 1 no movement, 2 the edges jump to their targets */
    int round_time;                        /* 0x12768 round time left in ticks ((Time + 1) * [Time] framespercount - 1), -1 = none (mugen.roundtimeleft) */
    int round_ticks;                       /* 0x1276C ticks of round time used (the timer bar) */
    int intro_skipped;                     /* 0x12770 round intro skipped by a button */
    int over_skipped;                      /* 0x12774 round over skipped by a button */
    int win_icons[2][10];                  /* 0x12778 per team and won round: win icon (WIN_ICON_*) */
    int win_perfect[2][10];                /* 0x127C8 the same: WIN_ICON_PERFECT, or -1 */
    int skip_pressed;                      /* 0x12818 1 -> 2 when a button is pressed (screen_kind 6, 7) */
    int unused_1281C;                      /* 0x1281C cleared by the input processing, never read */
    int arcade_match;                      /* 0x12820 arcade match number + 1 */
    int matches_picked;                    /* 0x12824 matches picked (Gameflow_PickStage) */
    int match_winner;                      /* 0x12828 team side that won the match (Match_Run: the arcade team lost otherwise) */
    int winner_member;                     /* 0x1282C team member of the winner whose victory quote is used (Match_Unload) */
    int match_has_result;                  /* 0x12830 the match has a result (Match_Run: arcade / survival progress) */
    int pad_12834;                         /* 0x12834 never accessed by the game */
    unsigned int match_rng_seed;           /* 0x12838 match RNG (Match_SeedRng): seed */
    int match_rng_seed_hi;                 /* 0x1283C high half of the seed, cleared by Match_SeedRng */
    unsigned long long match_rng;          /* 0x12840 state (Match_Random: state * mul % mod; also mugen.random / the AI) */
    unsigned int match_rng_mod;            /* 0x12848 0x7FFFFFFF */
    unsigned int match_rng_mul;            /* 0x1284C 16807 */
    int match_rng_open;                    /* 0x12850 set by Match_SeedRng, cleared when the match ends */
    int match_loaded;                      /* 0x12854 a match is loaded (Match_LoadAssets .. unload): debug info and fight drawing */
    ComboState combo[2];                   /* 0x12858 combo counters of team 1, 2 (Combo_AddHits) */
    double game_coord[4];                  /* 0x12888 game width, height, (width-1)/2, (height-1)/2 */
    int game_width;                        /* 0x128A8 [Config] GameWidth */
    int game_height;                       /* 0x128AC [Config] GameHeight */
    int video_width;                       /* 0x128B0 [Video] Width */
    int video_height;                      /* 0x128B4 [Video] Height */
    int depth;                             /* 0x128B8 [Video] Depth */
    int render_mode;                       /* 0x128BC RENDER_* */
    int safe_mode;                         /* 0x128C0 [Video] SafeMode (OpenGL only) */
    int safe_mode_opengl;                  /* 0x128C4 safe mode with OpenGL */
    int stage_fit;                         /* 0x128C8 [Video] StageFit */
    int system_fit;                        /* 0x128CC [Video] SystemFit */
    unsigned int video_flags;              /* 0x128D0 VIDEO_* */
    int fullscreen;                        /* 0x128D4 [Video] FullScreen */
    int hide_dev_banner;                   /* 0x128D8 [Debug] HideDevelopmentBuildBanner */
    int vretrace;                          /* 0x128DC [Video] VRetrace */
    int keep_aspect;                       /* 0x128E0 [Video] KeepAspect */
    int filter_indexed;                    /* 0x128E4 [Video] FilterIndexed */
    int unused_128E8;                      /* 0x128E8 set to -1 when [Sound] is read, never read */
    int unused_128EC;                      /* 0x128EC set to -1 when [Sound] is read, never read */
    int wav_channels;                      /* 0x128F0 [Sound] WavChannels */
    int pad_128F4;                         /* 0x128F4 never accessed by the game */
    int bgm_disabled;                      /* 0x128F8 -nomusic, or BGM init failed */
    int sample_rate;                       /* 0x128FC [Sound] SampleRate */
    int bit_depth;                         /* 0x12900 [Sound] BitDepth */
    int buffer_size;                       /* 0x12904 [Sound] BufferSize */
    int channels;                          /* 0x12908 [Sound] Channels */
    DynArray *music_plugins;               /* 0x1290C [Music] plugins, BgmPluginEntry * (BgmPlugins_Load) */
    int stereo_effects;                    /* 0x12910 [Sound] StereoEffects */
    int panning_width;                     /* 0x12914 [Sound] PanningWidth */
    int sfx_background_mode;               /* 0x12918 [Misc] SFXBackgroundMode: 0 mute, 1 play */
    int bgm_background_mode;               /* 0x1291C [Misc] BGMBackgroundMode: 0 pause, 1 mute, 2 play */
    int sfx_resample_method;               /* 0x12920 [Sound] SFXResampleMethod: 1 libresample, 0 SDL */
    int sfx_resample_quality;              /* 0x12924 */
    int bgm_resample_method;               /* 0x12928 */
    int bgm_resample_quality;              /* 0x1292C */
    int game_speed;                        /* 0x12930 [Config] GameSpeed (ticks per second) */
    int draw_shadows;                      /* 0x12934 */
    int afterimage_max;                    /* 0x12938 */
    int layered_sprite_max;                /* 0x1293C */
    int sprite_decompression_buffer_size;  /* 0x12940 */
    int explod_max;                        /* 0x12944 */
    int sysexplod_max;                     /* 0x12948 */
    int helper_max;                        /* 0x1294C */
    int player_projectile_max;             /* 0x12950 */
    int player_pal_max;                    /* 0x12954 */
    int first_run;                         /* 0x12958 [Config] FirstRun */
    int player_cache;                      /* 0x1295C [Misc] PlayerCache + 2 */
    int precache;                          /* 0x12960 [Misc] Precache (off with OpenGL) */
    int buffered_read;                     /* 0x12964 [Misc] BufferedRead */
    int unload_system;                     /* 0x12968 [Misc] UnloadSystem */
    int pause_on_defocus;                  /* 0x1296C [Misc] PauseOnDefocus */
    float force_stage_zoomout;             /* 0x12970 [Misc] ForceStageZoomout */
    int any_key_unpauses;                  /* 0x12974 [Input] AnyKeyUnpauses */
    int ai_random_color;                   /* 0x12978 [Arcade] AI.RandomColor */
    int ai_cheat;                          /* 0x1297C [Arcade] AI.Cheat */
    AiRamp ai_ramp[3];                     /* 0x12980 arcade, team, survival */
    int match_log;                         /* 0x129B0 -log given (see CmdLineArgs.log_enabled) */
    char match_log_file[0x200];            /* 0x129B4 */
    int override_ai[4];                    /* 0x12BB4 CmdLineArgs.ai..power copied as one block */
    int override_color[4];                 /* 0x12BC4 */
    int override_life[4];                  /* 0x12BD4 */
    int override_power[4];                 /* 0x12BE4 */
    int nojoy;                             /* 0x12BF4 -nojoy */
    int exit_frame;                        /* 0x12BF8 CmdLineArgs.exit_frame: quit once GameVars.frames reaches it (> 0) */
    int no_frame_stats;                    /* 0x12BFC CmdLineArgs.no_frame_stats: no frame statistics at exit */
    int difficulty;                        /* 0x12C00 [Options] Difficulty */
    int life;                              /* 0x12C04 [Options] Life */
    int time;                              /* 0x12C08 [Options] Time */
    int option_game_speed;                 /* 0x12C0C [Options] GameSpeed */
    int master_volume_pct;                 /* 0x12C10 options menu value; 101 = 100%, -1 = 0% */
    int wav_volume_pct;                    /* 0x12C14 */
    int bgm_volume_pct;                    /* 0x12C18 */
    int option_item7;                      /* 0x12C1C options menu slots of "Input Config", "Team Mode Config" (kept, unused) */
    int option_item8;                      /* 0x12C20 */
    int option_save;                       /* 0x12C24 options menu "Load/Save": 0 load, 1 save */
    int option_item10;                     /* 0x12C28 slot of "Default Values" (kept, unused) */
    char pad_12C2C[0x12C30 - 0x12C2C];
    int guard_ko;                          /* 0x12C30 1 once the options are loaded: a guarded hit that would kill KOs instead */
    int option_input[2][5];                /* 0x12C34 input menu values by side and item; [side][1]: [Input]
                                              P<n>.Joystick.type, the other items have no value */
    int team_1vs2_life;                    /* 0x12C5C [Options] Team.1VS2Life */
    int team_lose_on_ko;                   /* 0x12C60 [Options] Team.LoseOnKO */
    int option_team_unused[2];             /* 0x12C64 team menu items 2 and 3 (Options_TeamValue), no options behind them */
    char motif[0x200];                     /* 0x12C6C [Options] motif */
    int training_cursor;                   /* 0x12E6C training menu: item under the cursor */
    int training_value;                    /* 0x12E70 value of that item */
    int training_menu;                     /* 0x12E74 the training menu is shown (pause) */
    int training_page;                     /* 0x12E78 TRAINING_* page (its "Dummy control" value) */
    int training_coop[5];                  /* 0x12E7C page values: control, guard, mode, distance, button jam */
    int training_ai[2];                    /* 0x12E90 control, AI level - 1 */
    int training_manual[1];                /* 0x12E98 control */
    char start_stage[0x200];               /* 0x12E9C [Debug] StartStage (not terminated if 512 long) */
    int load_ticks[5];                     /* 0x1309C loading times in ticks: [0] the stage backgrounds, [1..4] players 1..4 */
    int match_load_ticks;                  /* 0x130B0 ticks Match_LoadAssets took */
    int start_ticks;                       /* 0x130B4 Timer_GetTicks() after start-up */
    int end_ticks;                         /* 0x130B8 Timer_GetTicks() at shutdown */
    char pad_130BC[0x130C0 - 0x130BC];
    int stage_palgroups;                   /* 0x130C0 palette groups with only the stage bit (PalGroups_Add) */
    int debug_player;                      /* 0x130C4 debug info shown: index of the player it shows (cycled by a debug key), 0 off */
    int quick_vs;                          /* 0x130C8 a quick VS match was requested */
    int debug;                             /* 0x130CC [Debug] Debug */
    int keep_sources;                      /* 0x130D0 1 at start-up: state controllers and statedefs remember their CNS line */
    int allow_debug_mode;                  /* 0x130D4 [Debug] AllowDebugMode (forced on by Debug) */
    int allow_debug_keys;                  /* 0x130D8 [Debug] AllowDebugKeys */
    int clsn_draw_mode;                    /* 0x130DC 2: debug collision boxes drawn filled (Clsn_Draw flag) */
    int show_fps;                          /* 0x130E0 the frame rate is drawn in the corner (Game_PrePresent) */
    TextRing *clipboard;                   /* 0x130E4 debug clipboard (the mugen.log mirror) */
    int clipboard_lines;                   /* 0x130E8 lines of the debug clipboard drawn (16) */
    TextRing *error_text;                  /* 0x130EC error clipboard */
    int speedup;                           /* 0x130F0 [Debug] Speedup */
    int speedtest;                         /* 0x130F4 -speedtest */
    int round_reset_pending;               /* 0x130F8 Screen_Run resets the round (Round_Reset) before the next frame */
    int match_reload;                      /* 0x130FC the match is reloaded (Game_LoopInit) */
    int stresstest;                        /* 0x13100 -stresstest */
    char pad_13104[0x13108 - 0x13104];
} GameVars;
MUGEN_ASSERT_OFFSET(GameVars, cwd, 0x00200);
MUGEN_ASSERT_OFFSET(GameVars, stage, 0x01AE0);
MUGEN_ASSERT_OFFSET(GameVars, select_stage, 0x00410);
MUGEN_ASSERT_OFFSET(GameVars, char_info, 0x020A0);
MUGEN_ASSERT_OFFSET(GameVars, char_info_2, 0x020B0);
MUGEN_ASSERT_OFFSET(GameVars, arcade_maxmatches, 0x020C0);
MUGEN_ASSERT_OFFSET(GameVars, team_maxmatches, 0x02138);
MUGEN_ASSERT_OFFSET(GameVars, precache_enabled, 0x021B0);
MUGEN_ASSERT_OFFSET(GameVars, select_info, 0x021DC);
MUGEN_ASSERT_OFFSET(GameVars, select_screen, 0x05070);
MUGEN_ASSERT_OFFSET(GameVars, arcade_side, 0x0508C);
MUGEN_ASSERT_OFFSET(GameVars, teams, 0x050A0);
MUGEN_ASSERT_OFFSET(GameVars, continued, 0x08710);
MUGEN_ASSERT_OFFSET(GameVars, sprite_layer1, 0x08748);
MUGEN_ASSERT_OFFSET(GameVars, sysexplods, 0x10C24);
MUGEN_ASSERT_OFFSET(GameVars, motif_scale, 0x110A8);
MUGEN_ASSERT_OFFSET(GameVars, motif_sprites, 0x110BC);
MUGEN_ASSERT_OFFSET(GameVars, motif_snd, 0x112C0);
MUGEN_ASSERT_OFFSET(GameVars, motif_fonts, 0x114C8);
MUGEN_ASSERT_OFFSET(GameVars, infobox_font, 0x00810);
MUGEN_ASSERT_OFFSET(GameVars, title_info, 0x0182C);
MUGEN_ASSERT_OFFSET(GameVars, option_info, 0x01A14);
MUGEN_ASSERT_OFFSET(GameVars, title_bg, 0x114F8);
MUGEN_ASSERT_OFFSET(GameVars, select_bg, 0x114FC);
MUGEN_ASSERT_OFFSET(GameVars, title_bgm, 0x11508);
MUGEN_ASSERT_OFFSET(GameVars, demo_wait, 0x126B4);
MUGEN_ASSERT_OFFSET(GameVars, demo_ticks, 0x126D4);
MUGEN_ASSERT_OFFSET(GameVars, option_save, 0x12C24);
MUGEN_ASSERT_OFFSET(GameVars, option_input, 0x12C34);
MUGEN_ASSERT_OFFSET(GameVars, option_team_unused, 0x12C64);
MUGEN_ASSERT_OFFSET(GameVars, training_cursor, 0x12E6C);
MUGEN_ASSERT_OFFSET(GameVars, training_manual, 0x12E98);
MUGEN_ASSERT_OFFSET(GameVars, select_bgm, 0x11B68);
MUGEN_ASSERT_OFFSET(GameVars, skip_draw, 0x11EA4);
MUGEN_ASSERT_OFFSET(GameVars, fade, 0x11EE4);
MUGEN_ASSERT_OFFSET(GameVars, input_keys, 0x11FF0);
MUGEN_ASSERT_OFFSET(GameVars, demo_mode, 0x126AC);
MUGEN_ASSERT_OFFSET(GameVars, screen, 0x08730);
MUGEN_ASSERT_OFFSET(GameVars, motif_coord, 0x11088);
MUGEN_ASSERT_OFFSET(GameVars, session_seed, 0x11EC8);
MUGEN_ASSERT_OFFSET(GameVars, player_sides, 0x0507C);
MUGEN_ASSERT_OFFSET(GameVars, match_members, 0x08658);
MUGEN_ASSERT_OFFSET(GameVars, team_faces, 0x10BDC);
MUGEN_ASSERT_OFFSET(GameVars, bg_palfx_def, 0x11D8C);
MUGEN_ASSERT_OFFSET(GameVars, all_palfx_def, 0x11DC8);
MUGEN_ASSERT_OFFSET(GameVars, camera_zoom, 0x11F34);
MUGEN_ASSERT_OFFSET(GameVars, envshake, 0x11F60);
MUGEN_ASSERT_OFFSET(GameVars, players_last, 0x12478);
MUGEN_ASSERT_OFFSET(GameVars, assert_intro, 0x1269C);
MUGEN_ASSERT_OFFSET(GameVars, assert_nofg, 0x126A6);
MUGEN_ASSERT_OFFSET(GameVars, pause_kind, 0x126DC);
MUGEN_ASSERT_OFFSET(GameVars, round_state, 0x12754);
MUGEN_ASSERT_OFFSET(GameVars, match_loaded, 0x12854);
MUGEN_ASSERT_OFFSET(GameVars, keep_sources, 0x130D0);
MUGEN_ASSERT_OFFSET(GameVars, window_active, 0x126A8);
MUGEN_ASSERT_OFFSET(GameVars, bgm_disabled, 0x128F8);
MUGEN_ASSERT_OFFSET(GameVars, match_log, 0x129B0);
MUGEN_ASSERT_OFFSET(GameVars, override_ai, 0x12BB4);
MUGEN_ASSERT_OFFSET(GameVars, nojoy, 0x12BF4);
MUGEN_ASSERT_OFFSET(GameVars, stresstest, 0x13100);
MUGEN_ASSERT_OFFSET(GameVars, motif_file, 0x10C28);
MUGEN_ASSERT_OFFSET(GameVars, motif_dir, 0x10E28);
MUGEN_ASSERT_OFFSET(GameVars, sound_enabled, 0x11E7C);
MUGEN_ASSERT_OFFSET(GameVars, game_type_m, 0x11F78);
MUGEN_ASSERT_OFFSET(GameVars, keys, 0x11F88);
MUGEN_ASSERT_OFFSET(GameVars, joy_keys_active, 0x12058);
MUGEN_ASSERT_OFFSET(GameVars, joy_keys, 0x120C0);
MUGEN_ASSERT_OFFSET(GameVars, use_keyboard, 0x12130);
MUGEN_ASSERT_OFFSET(GameVars, input_dir_mode, 0x12150);
MUGEN_ASSERT_OFFSET(GameVars, game_coord, 0x12888);
MUGEN_ASSERT_OFFSET(GameVars, video_flags, 0x128D0);
MUGEN_ASSERT_OFFSET(GameVars, wav_channels, 0x128F0);
MUGEN_ASSERT_OFFSET(GameVars, sample_rate, 0x128FC);
MUGEN_ASSERT_OFFSET(GameVars, bgm_resample_quality, 0x1292C);
MUGEN_ASSERT_OFFSET(GameVars, first_run, 0x12958);
MUGEN_ASSERT_OFFSET(GameVars, ai_ramp, 0x12980);
MUGEN_ASSERT_OFFSET(GameVars, difficulty, 0x12C00);
MUGEN_ASSERT_OFFSET(GameVars, bgm_volume_pct, 0x12C18);
MUGEN_ASSERT_OFFSET(GameVars, team_1vs2_life, 0x12C5C);
MUGEN_ASSERT_OFFSET(GameVars, motif, 0x12C6C);
MUGEN_ASSERT_OFFSET(GameVars, start_stage, 0x12E9C);
MUGEN_ASSERT_OFFSET(GameVars, debug, 0x130CC);
MUGEN_ASSERT_OFFSET(GameVars, allow_debug_keys, 0x130D8);
MUGEN_ASSERT_OFFSET(GameVars, speedup, 0x130F0);
MUGEN_ASSERT_OFFSET(GameVars, start_ticks, 0x130B4);
MUGEN_ASSERT_OFFSET(GameVars, end_ticks, 0x130B8);
MUGEN_ASSERT_OFFSET(GameVars, quick_vs, 0x130C8);
MUGEN_ASSERT_OFFSET(GameVars, clipboard, 0x130E4);
MUGEN_ASSERT_OFFSET(GameVars, error_text, 0x130EC);
MUGEN_ASSERT_OFFSET(GameVars, cwd_writable, 0x00400);
MUGEN_ASSERT_OFFSET(GameVars, team_variant, 0x05078);
MUGEN_ASSERT_OFFSET(GameVars, continue_reselect, 0x08714);
MUGEN_ASSERT_OFFSET(GameVars, render_target, 0x08734);
MUGEN_ASSERT_OFFSET(GameVars, font_small, 0x08738);
MUGEN_ASSERT_OFFSET(GameVars, afterimages, 0x08750);
MUGEN_ASSERT_OFFSET(GameVars, vretrace_on, 0x0875C);
MUGEN_ASSERT_OFFSET(GameVars, fightfx_sff, 0x0876C);
MUGEN_ASSERT_OFFSET(GameVars, fight, 0x08778);
MUGEN_ASSERT_OFFSET(GameVars, hud_enabled, 0x10C1C);
MUGEN_ASSERT_OFFSET(GameVars, explods, 0x10C20);
MUGEN_ASSERT_OFFSET(GameVars, frame_skip, 0x11E9C);
MUGEN_ASSERT_OFFSET(GameVars, defocus_paused, 0x11EB0);
MUGEN_ASSERT_OFFSET(GameVars, time_seed, 0x11EB8);
MUGEN_ASSERT_OFFSET(GameVars, camera_offset_x, 0x11F00);
MUGEN_ASSERT_OFFSET(GameVars, camera_x, 0x11F1C);
MUGEN_ASSERT_OFFSET(GameVars, camera_y_base, 0x11F28);
MUGEN_ASSERT_OFFSET(GameVars, camera_zoom, 0x11F34);
MUGEN_ASSERT_OFFSET(GameVars, key_codes, 0x12154);
MUGEN_ASSERT_OFFSET(GameVars, unused_1216C, 0x1216C);
MUGEN_ASSERT_OFFSET(GameVars, unused_12378, 0x12378);
MUGEN_ASSERT_OFFSET(GameVars, players_last, 0x12478);
MUGEN_ASSERT_OFFSET(GameVars, keep_sources, 0x130D0);
MUGEN_ASSERT_OFFSET(GameVars, clipboard_lines, 0x130E8);
MUGEN_ASSERT_OFFSET(GameVars, chars_cached, 0x020A8);
MUGEN_ASSERT_OFFSET(GameVars, screen_kind, 0x0509C);
MUGEN_ASSERT_OFFSET(GameVars, input_blocked, 0x1212C);
MUGEN_ASSERT_OFFSET(GameVars, players, 0x12274);
MUGEN_ASSERT_OFFSET(GameVars, players_active, 0x12170);
MUGEN_ASSERT_OFFSET(GameVars, team_faces, 0x10BDC);
MUGEN_ASSERT_OFFSET(GameVars, players_first, 0x12474);
MUGEN_ASSERT_OFFSET(GameVars, player_id_next, 0x12584);
MUGEN_ASSERT_OFFSET(GameVars, input_enabled, 0x1268C);
MUGEN_ASSERT_OFFSET(GameVars, pauses, 0x126E0);
MUGEN_ASSERT_OFFSET(GameVars, envcolor_time, 0x08768);
MUGEN_ASSERT_OFFSET(GameVars, bg_palfx_def, 0x11D8C);
MUGEN_ASSERT_OFFSET(GameVars, all_palfx_def, 0x11DC8);
MUGEN_ASSERT_OFFSET(GameVars, zoom_scale, 0x11F44);
MUGEN_ASSERT_OFFSET(GameVars, zoom_lag, 0x11F5C);
MUGEN_ASSERT_OFFSET(GameVars, hitdef_serial, 0x12694);
MUGEN_ASSERT_OFFSET(GameVars, assert_intro, 0x1269C);
MUGEN_ASSERT_OFFSET(GameVars, round_no, 0x12728);
MUGEN_ASSERT_OFFSET(GameVars, round_state, 0x12754);
MUGEN_ASSERT_OFFSET(GameVars, intro_skipped, 0x12770);
MUGEN_ASSERT_OFFSET(GameVars, arcade_match, 0x12820);
MUGEN_ASSERT_OFFSET(GameVars, match_members, 0x08658);
MUGEN_ASSERT_OFFSET(GameVars, camera_x_base, 0x11F24);
MUGEN_ASSERT_OFFSET(GameVars, round_winner, 0x12758);
MUGEN_ASSERT_OFFSET(GameVars, win_type, 0x1275C);
MUGEN_ASSERT_OFFSET(GameVars, logo_storyboard, 0x03580);
MUGEN_ASSERT_OFFSET(GameVars, end_credits, 0x03984);
MUGEN_ASSERT_OFFSET(GameVars, default_ending, 0x03B88);
MUGEN_ASSERT_OFFSET(GameVars, win_screen, 0x03F8C);
MUGEN_ASSERT_OFFSET(GameVars, survival_results, 0x04048);
MUGEN_ASSERT_OFFSET(GameVars, continue_screen, 0x04108);
MUGEN_ASSERT_OFFSET(GameVars, game_over, 0x04670);
MUGEN_ASSERT_OFFSET(GameVars, vs_screen, 0x04874);
MUGEN_ASSERT_OFFSET(GameVars, victory_screen, 0x04ADC);
MUGEN_ASSERT_OFFSET(GameVars, motif_loaded, 0x110B8);
MUGEN_ASSERT_OFFSET(GameVars, motif_sff_path, 0x110C0);
MUGEN_ASSERT_OFFSET(GameVars, motif_snd_path, 0x112C4);
MUGEN_ASSERT_OFFSET(GameVars, versus_bg, 0x114F0);
MUGEN_ASSERT_OFFSET(GameVars, vs_bgm, 0x11728);
MUGEN_ASSERT_OFFSET(GameVars, victory_bgm, 0x11948);
MUGEN_ASSERT_OFFSET(GameVars, demo_intro_waitcycles, 0x126B0);
MUGEN_ASSERT_OFFSET(GameVars, demo_debuginfo, 0x126D0);
MUGEN_ASSERT_OFFSET(GameVars, screen_left, 0x11F08);
MUGEN_ASSERT_OFFSET(GameVars, camera_y_target, 0x11F18);
MUGEN_ASSERT_OFFSET(GameVars, crop_top, 0x11F2C);
MUGEN_ASSERT_OFFSET(GameVars, rounds_won, 0x1272C);
MUGEN_ASSERT_OFFSET(GameVars, camera_mode, 0x12764);
MUGEN_ASSERT_OFFSET(GameVars, win_icons, 0x12778);
MUGEN_ASSERT_OFFSET(GameVars, win_perfect, 0x127C8);
MUGEN_ASSERT_OFFSET(GameVars, combo, 0x12858);
MUGEN_ASSERT_OFFSET(GameVars, frame_step, 0x11EAC);
MUGEN_ASSERT_OFFSET(GameVars, console_open, 0x12128);
MUGEN_ASSERT_OFFSET(GameVars, screenshot_requested, 0x12698);
MUGEN_ASSERT_OFFSET(GameVars, assert_nokoslow, 0x126A0);
MUGEN_ASSERT_OFFSET(GameVars, match_reload, 0x130FC);
MUGEN_ASSERT_OFFSET(GameVars, debug_player, 0x130C4);
MUGEN_ASSERT_OFFSET(GameVars, show_fps, 0x130E0);
MUGEN_ASSERT_OFFSET(GameVars, intro_played, 0x03980);
MUGEN_ASSERT_OFFSET(GameVars, ending_storyboard, 0x03D8C);
MUGEN_ASSERT_OFFSET(GameVars, slow_step, 0x12740);
MUGEN_ASSERT_OFFSET(GameVars, slow_counter, 0x12744);
MUGEN_ASSERT_OFFSET(GameVars, match_winner, 0x12828);
MUGEN_ASSERT_OFFSET(GameVars, match_has_result, 0x12830);
MUGEN_ASSERT_OFFSET(GameVars, round_time, 0x12768);
MUGEN_ASSERT_OFFSET(GameVars, match_rng, 0x12840);
MUGEN_ASSERT_OFFSET(GameVars, match_rng_mul, 0x1284C);
MUGEN_ASSERT_OFFSET(GameVars, winner_member, 0x1282C);
MUGEN_ASSERT_OFFSET(GameVars, victory_quote, 0x04C70);
MUGEN_ASSERT_OFFSET(GameVars, demo_cycles, 0x126D8);
MUGEN_ASSERT_OFFSET(GameVars, stage_bg, 0x11D88);
MUGEN_ASSERT_OFFSET(GameVars, zoom_left, 0x11F4C);
MUGEN_ASSERT_OFFSET(GameVars, arcade_music, 0x00610);
MUGEN_ASSERT_OFFSET(GameVars, unused_8718, 0x08718);
MUGEN_ASSERT_OFFSET(GameVars, stage_zoffset, 0x08758);
MUGEN_ASSERT_OFFSET(GameVars, session_rng, 0x11ED0);
MUGEN_ASSERT_OFFSET(GameVars, session_rng_mul, 0x11EDC);
MUGEN_ASSERT_OFFSET(GameVars, player_chars, 0x12174);
MUGEN_ASSERT_OFFSET(GameVars, hits_enabled, 0x12690);
MUGEN_ASSERT_OFFSET(GameVars, round_not_counted, 0x12734);
MUGEN_ASSERT_OFFSET(GameVars, ko_state_time, 0x1274C);
MUGEN_ASSERT_OFFSET(GameVars, matches_picked, 0x12824);
MUGEN_ASSERT_OFFSET(GameVars, match_rng_seed, 0x12838);
MUGEN_ASSERT_OFFSET(GameVars, match_rng_open, 0x12850);
MUGEN_ASSERT_OFFSET(GameVars, load_ticks, 0x1309C);
MUGEN_ASSERT_OFFSET(GameVars, match_load_ticks, 0x130B0);
MUGEN_ASSERT_SIZE(GameVars, 0x13108);

/* g_libraryError: failing subsystem, reported by Error_Exit. */
enum {
    LIBERR_NONE = 0,
    LIBERR_FONT = 1,
    LIBERR_SOUND = 2,
    LIBERR_GRAPHICS = 3,
    LIBERR_VIDEO_MODE = 4
};

/* Called with 1/0 when the game window gains/loses focus (see g_onActivate). */
typedef void (__cdecl *ActivateFn)(int gained);

/* Translates a configured key or joystick button to an input code (Input_Remap). */
typedef int (__cdecl *InputTranslateFn)(int key);

/* Value of the CmdLineArgs per-player overrides when not given. */
#define CMDLINE_UNSET (-1)

/* Parsed command line, allocated by SDL_main (malloc 0x640), filled by CmdLine_Parse. */
typedef struct CmdLineArgs {
    char player[4][0x40];                  /* 0x000 -p1..-p4 (first two plain arguments: p1, p2) */
    char stage[0x40];                      /* 0x100 -s / -stage */
    char motif[0x40];                      /* 0x140 -r: motif system.def ("/system.def" appended to a dir) */
    char pad_180[0x184 - 0x180];
    int log_enabled;                       /* 0x184 match log; never set (see CmdLine_Parse) */
    char log_file[0x200];                  /* 0x188 */
    char game_dir[0x200];                  /* 0x388 -gamedir */
    int nosound;                           /* 0x588 */
    int nomusic;                           /* 0x58C */
    int nojoy;                             /* 0x590 */
    int rounds;                            /* 0x594 -rounds: play this many then quit (default 1000000) */
    int ai[4];                             /* 0x598 -p<n>.ai, CMDLINE_UNSET if not given */
    int color[4];                          /* 0x5A8 -p<n>.color, 0-based */
    int life[4];                           /* 0x5B8 -p<n>.life */
    int power[4];                          /* 0x5C8 -p<n>.power */
    int unused_5D8;                        /* 0x5D8 copied to GameVars.nojoy, which nojoy then overwrites */
    int exit_frame;                        /* 0x5DC never set by CmdLine_Parse (GameVars.exit_frame) */
    int no_frame_stats;                    /* 0x5E0 never set by CmdLine_Parse (GameVars.no_frame_stats) */
    int ai_level;                          /* 0x5E4 -ailevel 1..9 (0 = not given) */
    int speed;                             /* 0x5E8 -speed -9..9 (default 999 = not given) */
    int session_seed;                      /* 0x5EC -sessionseed */
    int match_seed_arg;                    /* 0x5F0 match RNG seed for GameVars.match_seed_arg (never set by CmdLine_Parse) */
    int speedtest;                         /* 0x5F4 -speedtest */
    int stresstest;                        /* 0x5F8 -stresstest <n> */
    char pad_5FC[0x600 - 0x5FC];
    char storyboard[0x40];                 /* 0x600 -storyboard <file>: play it and quit */
} CmdLineArgs;
MUGEN_ASSERT_OFFSET(CmdLineArgs, stage, 0x100);
MUGEN_ASSERT_OFFSET(CmdLineArgs, game_dir, 0x388);
MUGEN_ASSERT_OFFSET(CmdLineArgs, nosound, 0x588);
MUGEN_ASSERT_OFFSET(CmdLineArgs, ai, 0x598);
MUGEN_ASSERT_OFFSET(CmdLineArgs, power, 0x5C8);
MUGEN_ASSERT_OFFSET(CmdLineArgs, ai_level, 0x5E4);
MUGEN_ASSERT_OFFSET(CmdLineArgs, stresstest, 0x5F8);
MUGEN_ASSERT_OFFSET(CmdLineArgs, storyboard, 0x600);
MUGEN_ASSERT_SIZE(CmdLineArgs, 0x640);

#endif /* MUGEN_GAME_H */
