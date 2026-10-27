#ifndef MUGEN_AI_H
#define MUGEN_AI_H

/*
 * The computer players (ai.c): moves of the characters are recorded (AiMove)
 * with statistics of their timing and positions, kept in AiData.tables by
 * state number and in "ElecbyteAIB" files, and the computer players choose
 * counter moves from them.
 */

/* Running statistics of a value: average, range, spread. */
typedef struct AiStat {
    float avg;                  /* 0x00 (box ranges: -FLT_MAX = none yet) */
    int min;                    /* 0x04 */
    int max;                    /* 0x08 */
    float dev;                  /* 0x0C average distance from avg */
} AiStat;
MUGEN_ASSERT_SIZE(AiStat, 0x10);

/* A state the character went through on the way to the move. */
typedef struct AiMoveStep {
    int state_no;               /* 0x00 */
    int statetype;              /* 0x04 */
    int movetype;               /* 0x08 */
    int ctrl;                   /* 0x0C */
} AiMoveStep;
MUGEN_ASSERT_SIZE(AiMoveStep, 0x10);

/*
 * A learned move (0x4B4 bytes; AiData.tables entries keyed by (0, state_no)).
 * The timing statistics are triples ticks / x / y, with the position relative
 * to the start of the move (x towards the facing at the start); the
 * per-contact ones are indexed by the contact of the move: 0 none, 1
 * guarded, 2 hit.
 */
typedef struct AiMove {
    AiMoveStep steps[32];       /* 0x000 */
    int step_count;             /* 0x200 */
    int state_no;               /* 0x204 the move's state (key) */
    int statetype;              /* 0x208 */
    int cmds[10];               /* 0x20C command labels that led to it, -1 terminated */
    AiStat hit_at[3];           /* 0x234 when the HitDef (or projectile) became active */
    AiStat contact_at[3][3];    /* 0x264 when it was active, by contact */
    AiStat bounds[2][4];        /* 0x2F4 Clsn1 / Clsn2 ranges: x1, y1, x2, y2 (avg only) */
    unsigned int attr[4];       /* 0x374 HitDef hitflag, guardflag, attr, reversal attr (-1 when they differ) */
    AiStat damage;              /* 0x384 */
    AiStat ctrl_at[3][3];       /* 0x394 when the player got control back, by contact */
    AiStat end_at[3][3];        /* 0x424 when the move ended, by contact */
} AiMove;
MUGEN_ASSERT_OFFSET(AiMove, step_count, 0x200);
MUGEN_ASSERT_OFFSET(AiMove, cmds, 0x20C);
MUGEN_ASSERT_OFFSET(AiMove, hit_at, 0x234);
MUGEN_ASSERT_OFFSET(AiMove, contact_at, 0x264);
MUGEN_ASSERT_OFFSET(AiMove, bounds, 0x2F4);
MUGEN_ASSERT_OFFSET(AiMove, attr, 0x374);
MUGEN_ASSERT_OFFSET(AiMove, damage, 0x384);
MUGEN_ASSERT_OFFSET(AiMove, ctrl_at, 0x394);
MUGEN_ASSERT_OFFSET(AiMove, end_at, 0x424);
MUGEN_ASSERT_SIZE(AiMove, 0x4B4);

/* The "ElecbyteAIB" file header (0x40 bytes). */
typedef struct AiFileHeader {
    char signature[12];         /* 0x00 "ElecbyteAIB" */
    int version;                /* 0x0C 0x100 */
    unsigned short year;        /* 0x10 build date of the MUGEN that wrote it: 2013 */
    unsigned char month;        /* 0x12 8 */
    unsigned char day;          /* 0x13 11 */
    unsigned char reserved[0x2C]; /* 0x14 zero in the files the game writes, not checked when reading */
} AiFileHeader;
MUGEN_ASSERT_SIZE(AiFileHeader, 0x40);

#define AI_FILE_VERSION 0x100

/* A message between computer players (AiState.msgs; Ai_Broadcast). */
typedef struct AiMsg {
    struct Player *from;        /* 0x00 */
    int type;                   /* 0x04 0 state change, 1 move, 2 taken, -1 none */
    int value;                  /* 0x08 state number */
    KeyedArray *table;          /* 0x0C learned moves of the state type */
} AiMsg;
MUGEN_ASSERT_SIZE(AiMsg, 0x10);

/* A step of a way to a position (AiState.path). */
typedef struct AiPathStep {
    int kind;                   /* 0x00 1 walk, 2 jump, 0 none */
    int x;                      /* 0x04 */
    int y;                      /* 0x08 */
} AiPathStep;
MUGEN_ASSERT_SIZE(AiPathStep, 0x0C);

/* AiState.mode */
enum {
    AI_MODE_IDLE = 0,
    AI_MODE_MOVE = 1,           /* walk / jump along the path */
    AI_MODE_ATTACK = 2,         /* do AiState.move */
    AI_MODE_GUARD = 3,          /* guard AiState.threat */
    AI_MODE_FIGHT = 4           /* random walking, jumping and buttons */
};

/*
 * The decision state of a computer player (Player.ai_state, 0x165C: the AI
 * block of a Player starts with Player.ai at 0x1658), up to Player.dummy_control.
 */
typedef struct AiState {
    int mode;                   /* 0x165C AI_MODE_* */
    int prev_mode;              /* 0x1660 */
    int mode_ticks;             /* 0x1664 */
    int crouch_idle;            /* 0x1668 1: crouch while idle */
    int idle_ticks;             /* 0x166C ticks idle */
    int path_kind;              /* 0x1670 kind of the current path step */
    int step_ticks;             /* 0x1674 ticks on the current path step */
    int start_x;                /* 0x1678 position when the step started */
    int start_y;                /* 0x167C */
    AiPathStep path[2];         /* 0x1680 */
    int path_index;             /* 0x1698 */
    AiMove *move;               /* 0x169C move being done (AI_MODE_ATTACK) */
    int move_statetype;         /* 0x16A0 state type the move starts from */
    int move_started;           /* 0x16A4 its command was given */
    AiMove *current;            /* 0x16A8 learned move of the player's current state, or NULL */
    int pad_16AC[3];            /* 0x16AC never accessed by the game */
    struct Player *threat_owner;/* 0x16B8 enemy whose move was copied into threat */
    AiMove threat;              /* 0x16BC the enemy's current move */
    int threat_valid;           /* 0x1B70 */
    int threat_ticks;           /* 0x1B74 ticks since threat was copied */
    int no_threat_ticks;        /* 0x1B78 ticks without an attacking threat */
    int pad_1B7C;               /* 0x1B7C never accessed by the game */
    int cooldown;               /* 0x1B80 ticks before guarding again */
    int guard_crouch;           /* 0x1B84 guard crouching */
    AiMove *candidates[50];     /* 0x1B88 counter moves found by Ai_FindCounters, NULL terminated */
    int candidate_count;        /* 0x1C50 */
    AiMsg msgs[10];             /* 0x1C54 received messages */
    int msg_count;              /* 0x1CF4 */
    int state_msg_sent;         /* 0x1CF8 a state message was sent (Round_Tick notifies the other AI players when the move ends) */
} AiState;
MUGEN_ASSERT_OFFSET(AiState, path, 0x1680 - 0x165C);
MUGEN_ASSERT_OFFSET(AiState, path_index, 0x1698 - 0x165C);
MUGEN_ASSERT_OFFSET(AiState, current, 0x16A8 - 0x165C);
MUGEN_ASSERT_OFFSET(AiState, threat, 0x16BC - 0x165C);
MUGEN_ASSERT_OFFSET(AiState, threat_valid, 0x1B70 - 0x165C);
MUGEN_ASSERT_OFFSET(AiState, candidates, 0x1B88 - 0x165C);
MUGEN_ASSERT_OFFSET(AiState, msgs, 0x1C54 - 0x165C);
MUGEN_ASSERT_OFFSET(AiState, state_msg_sent, 0x1CF8 - 0x165C);
MUGEN_ASSERT_SIZE(AiState, 0x1CFC - 0x165C);

/* A move being recorded (Player.ai_record, 0x1EE4; Ai_RecordMove). */
typedef struct AiLearn {
    int active;                 /* 0x1EE4 1 recording, 2 it hit (or fired) something */
    int ticks;                  /* 0x1EE8 */
    int statetype;              /* 0x1EEC at the start */
    int start_x;                /* 0x1EF0 */
    int start_y;                /* 0x1EF4 */
    int facing;                 /* 0x1EF8 */
    int hit_serial;             /* 0x1EFC HitDef hit ID (HitDef.serial) whose damage was counted */
    int proj_id;                /* 0x1F00 ProjID of the projectile followed */
    int contact;                /* 0x1F04 0 none, 1 guarded, 2 hit (first one seen) */
    AiMove move;                /* 0x1F08 */
} AiLearn;
MUGEN_ASSERT_OFFSET(AiLearn, move, 0x24);
MUGEN_ASSERT_SIZE(AiLearn, 0x4D8);

#endif /* MUGEN_AI_H */
