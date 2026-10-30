#ifndef MUGEN_CHAR_H
#define MUGEN_CHAR_H

/*
 * Character data types (char.c): the constants block shared by CharData and
 * Player, the command set read from the .cmd file and the small player
 * sub-structures the character code touches. CharData and Player themselves
 * are in player.h.
 */

/* Number of commands with distinct names a character may have (CmdSet.label_count). */
#define CMD_MAX_LABELS      0x80
/* Steps of one command ("command = ..."). */
#define CMD_MAX_STEPS       0x100
/* Victory quotes read from [Quotes] (victory0..victory99). */
#define CHAR_QUOTE_COUNT    100

/* CmdStep.keys: directions and buttons (the buttons go through the [Remap] table). */
#define CMD_KEY_A           0x00010
#define CMD_KEY_B           0x00020
#define CMD_KEY_C           0x00040
#define CMD_KEY_X           0x00080
#define CMD_KEY_Y           0x00100
#define CMD_KEY_Z           0x00200
#define CMD_KEY_S           0x00400
#define CMD_KEY_U           0x01000
#define CMD_KEY_F           0x02000
#define CMD_KEY_D           0x04000
#define CMD_KEY_B_DIR       0x08000
#define CMD_KEY_UF          0x03000
#define CMD_KEY_UB          0x09000
#define CMD_KEY_DF          0x06000
#define CMD_KEY_DB          0x0C000
#define CMD_KEY_N           0x80000     /* "N" */
#define CMD_KEY_DIRS        0x8F000     /* directions: a repeated direction gets an implicit release step */
#define CMD_RELEASE         0x10000     /* "~": released (with the charge time in bits 22..31) */
#define CMD_ANY_DIR         0x20000     /* "/": held */
#define CMD_4WAY            0x40000     /* "$" */
#define CMD_CHARGE_SHIFT    20          /* "~30": (30 & ~3) << 20 */

/* CmdStep.flags. */
#define CMD_STEP_REPEATED   0x01        /* the step repeats the previous direction (release step inserted) */
#define CMD_STEP_STRICT     0x01        /* ">": no other key in between (shares bit 0) */

/* CmdEntry.type. */
#define CMD_TYPE_NORMAL     1
#define CMD_TYPE_RECOVERY   2           /* the "recovery" command: label 0 */

/* One step of a command. */
typedef struct CmdStep {
    unsigned int keys;          /* 0x00 CMD_KEY_* | CMD_RELEASE | CMD_ANY_DIR | CMD_4WAY | charge time */
    unsigned int flags;         /* 0x04 CMD_STEP_* */
} CmdStep;
MUGEN_ASSERT_SIZE(CmdStep, 0x08);

/* One [Command] group of the .cmd file. */
typedef struct CmdEntry {
    CmdStep *steps;             /* 0x00 */
    int step_count;             /* 0x04 */
    int time;                   /* 0x08 time (default [Defaults] command.time) */
    int buffer_time;            /* 0x0C buffer.time, at most 30 */
    int label;                  /* 0x10 command number; commands with the same name share it */
    int type;                   /* 0x14 CMD_TYPE_* */
} CmdEntry;
MUGEN_ASSERT_SIZE(CmdEntry, 0x18);

/* The commands of a character (CharData.cmds). */
typedef struct CmdSet {
    CmdEntry *entries;          /* 0x00 count entries */
    char **names;               /* 0x04 name of each entry, without the quotes */
    int count;                  /* 0x08 [Command] groups */
    int label_count;            /* 0x0C distinct names + 1 */
} CmdSet;
MUGEN_ASSERT_SIZE(CmdSet, 0x10);

/* [Remap] of the .cmd file: CMD_KEY_* value of a, b, c, x, y, z, s (Cmd_Parse). */
typedef struct CmdRemap {
    unsigned int keys[7];       /* 0x00 */
} CmdRemap;
MUGEN_ASSERT_SIZE(CmdRemap, 0x1C);

/* States of a character (StateDefs_Load, state module): Statedef by state number. */
typedef struct StateDefs {
    KeyedArray *list;           /* 0x00 */
} StateDefs;
MUGEN_ASSERT_SIZE(StateDefs, 0x04);

/* Learned AI data (ai.c, "ElecbyteAIB" files). */
typedef struct AiData {
    KeyedArray *tables[2];      /* 0x000 AiMove by (0, state_no): stand and crouch moves (80), air moves (40) */
    AiStat bounds[4][4];        /* 0x008 Clsn2 ranges (x1, y1, x2, y2; avg only) in states 0, 20, 11 and 50 */
} AiData;
MUGEN_ASSERT_SIZE(AiData, 0x108);

/*
 * The [Data], [Size], [Velocity] and [Movement] constants of a character
 * (Char_LoadConstants). CharData holds the loaded values and every Player
 * gets a copy (Player.consts). Sizes and velocities are in character units.
 */
typedef struct CharConsts {
    int life;                       /* 0x000 */
    int power;                      /* 0x004 default 3000 */
    int attack;                     /* 0x008 */
    int attack_z_width[2];          /* 0x00C [Size] attack.z.width: front, back */
    int airjuggle;                  /* 0x014 */
    int attack_dist;                /* 0x018 */
    int proj_attack_dist;           /* 0x01C */
    int defence;                    /* 0x020 */
    float fall_defence_mul;         /* 0x024 (fall.defence_up + 100) / 100 */
    int liedown_time;               /* 0x028 */
    float xscale;                   /* 0x02C */
    float yscale;                   /* 0x030 */
    int ground_back;                /* 0x034 */
    int ground_front;               /* 0x038 */
    int air_back;                   /* 0x03C */
    int air_front;                  /* 0x040 */
    int height;                     /* 0x044 */
    int proj_doscale;               /* 0x048 */
    int head_pos[2];                /* 0x04C */
    int mid_pos[2];                 /* 0x054 */
    int shadowoffset;               /* 0x05C */
    float walk_fwd;                 /* 0x060 */
    float walk_back;                /* 0x064 */
    float walk_up;                  /* 0x068 */
    float walk_down;                /* 0x06C */
    float run_fwd[2];               /* 0x070 */
    float run_back[2];              /* 0x078 */
    float run_down[2];              /* 0x080 */
    float run_up[2];                /* 0x088 */
    float jump_y;                   /* 0x090 second value of jump.neu */
    float jump_neu_x;               /* 0x094 */
    float jump_fwd_x;               /* 0x098 */
    float jump_back_x;              /* 0x09C */
    float jump_up_x;                /* 0x0A0 */
    float jump_down_x;              /* 0x0A4 */
    float runjump_y;                /* 0x0A8 second value of runjump.fwd */
    float runjump_fwd_x;            /* 0x0AC default jump.fwd */
    float runjump_back_x;           /* 0x0B0 */
    float runjump_up_x;             /* 0x0B4 */
    float runjump_down_x;           /* 0x0B8 */
    int airjump_num;                /* 0x0BC */
    int airjump_height;             /* 0x0C0 */
    float airjump_y;                /* 0x0C4 second value of airjump.neu (default jump y) */
    float airjump_neu_x;            /* 0x0C8 */
    float airjump_fwd_x;            /* 0x0CC */
    float airjump_back_x;           /* 0x0D0 */
    float airjump_up_x;             /* 0x0D4 */
    float airjump_down_x;           /* 0x0D8 */
    float yaccel;                   /* 0x0DC */
    float stand_friction;           /* 0x0E0 */
    float crouch_friction;          /* 0x0E4 */
    int sparkno;                    /* 0x0E8 */
    int guard_sparkno;              /* 0x0EC */
    int ko_echo;                    /* 0x0F0 default 1 */
    int draw_offset[2];             /* 0x0F4 */
    int int_persist_index;          /* 0x0FC 0..60 */
    int float_persist_index;        /* 0x100 0..40 */
    float air_gethit_groundrecover[2];      /* 0x104 */
    float air_gethit_airrecover_mul[2];     /* 0x10C */
    float air_gethit_airrecover_add[2];     /* 0x114 */
    float air_gethit_airrecover_back;       /* 0x11C */
    float air_gethit_airrecover_fwd;        /* 0x120 */
    float air_gethit_airrecover_up;         /* 0x124 */
    float air_gethit_airrecover_down;       /* 0x128 */
    float stand_friction_threshold;         /* 0x12C */
    float crouch_friction_threshold;        /* 0x130 */
    float air_gethit_groundlevel;           /* 0x134 */
    float air_gethit_groundrecover_ground_threshold; /* 0x138 */
    float air_gethit_groundrecover_groundlevel;      /* 0x13C */
    float air_gethit_airrecover_threshold;  /* 0x140 */
    float air_gethit_airrecover_yaccel;     /* 0x144 */
    float air_gethit_trip_groundlevel;      /* 0x148 */
    float down_bounce_offset[2];            /* 0x14C */
    float down_bounce_yaccel;               /* 0x154 */
    float down_bounce_groundlevel;          /* 0x158 */
    float down_friction_threshold;          /* 0x15C */
} CharConsts;
MUGEN_ASSERT_OFFSET(CharConsts, defence, 0x020);
MUGEN_ASSERT_OFFSET(CharConsts, walk_fwd, 0x060);
MUGEN_ASSERT_OFFSET(CharConsts, jump_y, 0x090);
MUGEN_ASSERT_OFFSET(CharConsts, airjump_num, 0x0BC);
MUGEN_ASSERT_OFFSET(CharConsts, yaccel, 0x0DC);
MUGEN_ASSERT_OFFSET(CharConsts, int_persist_index, 0x0FC);
MUGEN_ASSERT_OFFSET(CharConsts, air_gethit_groundrecover, 0x104);
MUGEN_ASSERT_OFFSET(CharConsts, stand_friction_threshold, 0x12C);
MUGEN_ASSERT_SIZE(CharConsts, 0x160);

/* Player input buffer (input module, Player.input): last directions and the held keys. */
#define PLAYER_INPUT_RING_SIZE 30
typedef struct InputBuf {
    unsigned int ring[PLAYER_INPUT_RING_SIZE]; /* 0x00 recent inputs */
    int keys[KEY_COUNT];        /* 0x78 held state of each KEY_* (after the facing flip) */
    int read;                   /* 0xAC next ring entry to take */
    int write;                  /* 0xB0 next ring entry to fill */
    int locked;                 /* 0xB4 codes are not queued */
    int flipped;                /* 0xB8 left/right swapped (facing left) */
} InputBuf;
MUGEN_ASSERT_OFFSET(InputBuf, keys, 0x78);
MUGEN_ASSERT_OFFSET(InputBuf, read, 0xAC);
MUGEN_ASSERT_SIZE(InputBuf, 0xBC);

/* Command recognition state of a player (cmdinput.c, Player.cmd_state): a ring of 128 input entries. */
typedef struct CmdState {
    unsigned int keys[128];     /* 0x000 CMD_KEY_* of each entry (CmdState_AddInput) */
    unsigned int frames[128];   /* 0x200 GameVars.frames of each entry */
    int charge[128];            /* 0x400 ticks a released key was held */
    int first;                  /* 0x600 oldest entry */
    int write;                  /* 0x604 next entry to fill */
    int pos;                    /* 0x608 input history position advanced by CmdState_Update */
    unsigned int press_frame[16]; /* 0x60C frame each CMD_KEY_* bit was last pressed */
    int frozen;                 /* 0x64C commands are not updated (set while Player.hitpause_time > 0) */
    unsigned int held;          /* 0x650 keys held this frame (Input_GetHeld) */
    unsigned int dirs;          /* 0x654 directions held at the last direction change */
    unsigned int pending;       /* 0x658 buttons waiting for the next flush (CmdState_AddInput(0)) */
} CmdState;
MUGEN_ASSERT_OFFSET(CmdState, pos, 0x608);
MUGEN_ASSERT_OFFSET(CmdState, frozen, 0x64C);
MUGEN_ASSERT_SIZE(CmdState, 0x65C);

/* Reference to a player by id (PlayerRef_Set, explods/helpers "bind" targets). */
typedef struct PlayerRef {
    int valid;                  /* 0x00 */
    int id;                     /* 0x04 Player.id */
    int index;                  /* 0x08 Player.index */
    int flag;                   /* 0x0C */
} PlayerRef;
MUGEN_ASSERT_SIZE(PlayerRef, 0x10);

/* Player_GetPosOrigin postypes. */
enum {
    POSTYPE_NONE = -1,
    POSTYPE_P1 = 0,
    POSTYPE_FRONT = 1,
    POSTYPE_BACK = 2,
    POSTYPE_LEFT = 3,
    POSTYPE_RIGHT = 4,
    POSTYPE_P2 = 5
};

#endif /* MUGEN_CHAR_H */
