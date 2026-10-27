#ifndef MUGEN_PLAYER_H
#define MUGEN_PLAYER_H

#define PLAYER_VAR_COUNT    60
#define PLAYER_FVAR_COUNT   40

/* Player.statetype values. */
enum {
    STATETYPE_STAND = 1,
    STATETYPE_CROUCH = 2,
    STATETYPE_AIR = 3,
    STATETYPE_LIEDOWN = 4
};

/* Player.movetype values. */
enum {
    MOVETYPE_IDLE = 0,
    MOVETYPE_ATTACK = 1,
    MOVETYPE_HIT = 2
};

/* HitDef attribute bits (HitDef.attr, Player.hitby_attr / nothitby_attr, HitOverride.attr). */
#define HITATTR_STAND       0x001   /* S */
#define HITATTR_CROUCH      0x002   /* C */
#define HITATTR_AIR         0x004   /* A */
#define HITATTR_STATES      0x007   /* the state type bits */
#define HITATTR_THROWS      0x1C0   /* NT, ST, HT: both players become unhittable for the pause time */

/*
 * Where a player's states come from (Player_GetCustomState / Player_SetCustomState):
 * the states, commands and animations of another player (p2stateno).
 */
typedef struct CustomStateInfo {
    int index;                  /* 0x00 Player.index of the owner of the states */
    StateDefs *states;          /* 0x04 */
    CmdSet *cmds;               /* 0x08 */
    Anim *anim;                 /* 0x0C */
} CustomStateInfo;
MUGEN_ASSERT_SIZE(CustomStateInfo, 0x10);

/*
 * A HitDef (or ReversalDef) as the state controllers set it up (Player.hitdef,
 * Projectile.hitdef). Only the fields the player runtime reads are named; the
 * names follow the HitDef parameters where the use makes them clear.
 */
typedef struct HitDef {
    int active;                 /* 0x000 the HitDef can hit */
    int hitonce_used;           /* 0x004 a hitonce ReversalDef already reversed a hit (cleared at set-up) */
    unsigned int affectteam;    /* 0x008 team bits as seen from team side 1 */
    unsigned int attr;          /* 0x00C HITATTR_* */
    int serial;                 /* 0x010 hit ID: GameVars.hitdef_serial counter at set-up (-1 for projectiles),
                                   kept by the targets (TargetEntry.hit_id, Player.hit_ids) */
    int id;                     /* 0x014 becomes the target's gethit_id */
    int chainid;                /* 0x018 -1 = none */
    int nochainid[2];           /* 0x01C -1 = none */
    int kill;                   /* 0x024 */
    int guard_kill;             /* 0x028 */
    int fall_kill;              /* 0x02C */
    int hitonce;                /* 0x030 */
    int juggle;                 /* 0x034 juggle points, -1 = none */
    int damage;                 /* 0x038 */
    int guard_damage;           /* 0x03C */
    int getpower;               /* 0x040 */
    int guard_getpower;         /* 0x044 */
    int givepower;              /* 0x048 */
    int guard_givepower;        /* 0x04C */
    int p2_pausetime;           /* 0x050 shake time of the target */
    int p1_pausetime;           /* 0x054 */
    int numhits;                /* 0x058 */
    int hitsound[3];            /* 0x05C group, number, nonzero: fight.def common sounds */
    int guardsound[3];          /* 0x068 */
    unsigned int guardflag;     /* 0x074 Player_HitFlagMatches flags */
    unsigned int hitflag;       /* 0x078 bit 0: always fall, 0x80: hits projectiles (P) */
    int priority;               /* 0x07C */
    int priority_type;          /* 0x080 0 hit, 1 miss, 2 dodge */
    int p1stateno;              /* 0x084 -1 = none */
    int p2stateno;              /* 0x088 -1 = none */
    int p2getp1state;           /* 0x08C */
    CustomStateInfo custom;     /* 0x090 the attacker's states, for p2getp1state */
    int pad_0A0;                /* 0x0A0 never accessed by the game */
    int snap_time;              /* 0x0A4 p2bindtime: TargetEntry.bind_time of the target */
    int p1sprpriority;          /* 0x0A8 */
    int p2sprpriority;          /* 0x0AC */
    int animtype;               /* 0x0B0 */
    int forcestand;             /* 0x0B4 the target stands up (ground hits) */
    int forcenofall;            /* 0x0B8 clears the target's fall */
    unsigned int reversal_attr; /* 0x0BC ReversalDef reversal.attr (nonzero: a ReversalDef, not a HitDef) */
    int fall_damage;            /* 0x0C0 */
    int fall_animtype;          /* 0x0C4 */
    float fall_xvel;            /* 0x0C8 -FLT_MAX = none */
    float fall_yvel;            /* 0x0CC */
    int fall_recover;           /* 0x0D0 */
    int fall_recovertime;       /* 0x0D4 */
    int fall_envshake[6];       /* 0x0D8 */
    int sparkno;                /* 0x0F0 -2: the character's default, -1 none */
    int spark_own;              /* 0x0F4 the spark is from the character's sprites (S prefix) */
    int guard_sparkno;          /* 0x0F8 */
    int guard_spark_own;        /* 0x0FC */
    int sparkxy[2];             /* 0x100 */
    int p1facing;               /* 0x108 */
    int p1getp2facing;          /* 0x10C 1: turn the attacker when facing the target's way, -1 when not, 0 never */
    int mindist[2];             /* 0x110 -1025 = none */
    int maxdist[2];             /* 0x118 -1025 = none; also the TargetEntry bind offset */
    int p2facing;               /* 0x120 target facing rule */
    int ground_type;            /* 0x124 0 = no get hit state on the ground */
    int ground_hittime;         /* 0x128 */
    int ground_slidetime;       /* 0x12C */
    float ground_velocity[2];   /* 0x130 -FLT_MAX = keep */
    float ground_cornerpush;    /* 0x138 */
    int air_type;               /* 0x13C */
    int air_animtype;           /* 0x140 */
    int air_hittime;            /* 0x144 */
    int air_fall;               /* 0x148 */
    float air_velocity[2];      /* 0x14C -FLT_MAX = keep */
    float air_cornerpush;       /* 0x154 */
    int down_bounce;            /* 0x158 */
    int down_hittime;           /* 0x15C */
    float down_velocity[2];     /* 0x160 */
    float down_cornerpush;      /* 0x168 */
    float guard_velocity;       /* 0x16C */
    int guard_hittime;          /* 0x170 */
    int guard_slidetime;        /* 0x174 */
    int guard_ctrltime;         /* 0x178 */
    int guard_dist;             /* 0x17C guard.dist (AttackDist; -1: [Size] attack.dist) */
    int guard_p2_pausetime;     /* 0x180 */
    int guard_p1_pausetime;     /* 0x184 */
    float guard_cornerpush;     /* 0x188 */
    float airguard_velocity[2]; /* 0x18C */
    int airguard_ctrltime;      /* 0x194 */
    float airguard_cornerpush;  /* 0x198 */
    float yaccel;               /* 0x19C -FLT_MAX: 0.35 in 320 units */
    int palfx[15];              /* 0x1A0 copied to the target's Player.palfx_def; [1] time */
    int envshake[6];            /* 0x1DC copied to GameVars 0x11F60 when [1] (time) is set */
} HitDef;
MUGEN_ASSERT_OFFSET(HitDef, chainid, 0x018);
MUGEN_ASSERT_OFFSET(HitDef, numhits, 0x058);
MUGEN_ASSERT_OFFSET(HitDef, custom, 0x090);
MUGEN_ASSERT_OFFSET(HitDef, sparkno, 0x0F0);
MUGEN_ASSERT_OFFSET(HitDef, ground_type, 0x124);
MUGEN_ASSERT_OFFSET(HitDef, yaccel, 0x19C);
MUGEN_ASSERT_OFFSET(HitDef, envshake, 0x1DC);
MUGEN_ASSERT_SIZE(HitDef, 0x1F4);

/* A HitOverride slot (Player.hit_override). */
typedef struct HitOverride {
    int active;                 /* 0x00 */
    unsigned int attr;          /* 0x04 HITATTR_* */
    int stateno;                /* 0x08 -1: the hit is only ignored */
    int time;                   /* 0x0C ticks left, -1 forever */
    int forceair;               /* 0x10 */
} HitOverride;
MUGEN_ASSERT_SIZE(HitOverride, 0x14);

/* An entry of Player.targets (players hit by the player's HitDefs or projectiles). */
typedef struct TargetEntry {
    struct Player *player;      /* 0x00 */
    int juggle;                 /* 0x04 juggle points left */
    int hit_id;                 /* 0x08 ID of the HitDef that hit (target(ID)) */
    int bind_x;                 /* 0x0C TargetBind offset (times the facing) */
    int bind_y;                 /* 0x10 */
    int bind_time;              /* 0x14 TargetBind ticks left, -1 forever */
    int bind_active;            /* 0x18 bind_time at the last Player_Update of the owner */
} TargetEntry;
MUGEN_ASSERT_SIZE(TargetEntry, 0x1C);

/* A player (character, helper or projectile owner), 0x2490 bytes (Player_Create). */
typedef struct Player {
    struct CharData *data;      /* 0x000 the loaded character, NULL for a helper being created */
    int id;                     /* 0x004 PlayerID, unique (GameVars.player_id_next) */
    int index;                  /* 0x008 player number (index into per-player GameVars arrays) */
    int team_side;              /* 0x00C team, 0 = none */
    int team_index;             /* 0x010 team 0 / 1 (GameVars.teams, match_members: TeamMode trigger), -1 at creation */
    int team_member;            /* 0x014 member of the team the player belongs to (TeamSel.cur_member when created);
                                   with team_slot, the entry team_member * 4 + team_slot of TeamSel.member_data */
    int team_slot;              /* 0x018 slot of the player in that member (simul: 0..3) */
    int helper;                 /* 0x01C created without character data (helpers) */
    char name[0x30];            /* 0x020 CharData.name */
    CharConsts consts;          /* 0x050 copy of CharData.consts */
    int active;                 /* 0x1B0 in the match (Players_SetActive) */
    int suspended;              /* 0x1B4 input, AI and afterimages are skipped while set (only ever cleared) */
    int life;                   /* 0x1B8 [Options] Life * consts.life / 100 at reset (Life trigger) */
    int life_max;               /* 0x1BC LifeMax trigger */
    int lifebar_life;           /* 0x1C0 life shown by the life bar */
    int lifebar_red;            /* 0x1C4 its red part (Lifebar_TickLife) */
    int *power_ref;             /* 0x1C8 &power of the team leader (players[team_side]) or of the player */
    int *power_max_ref;         /* 0x1CC &power_max of the team leader or of the player */
    int power;                  /* 0x1D0 power (Power trigger, read through power_ref) */
    int power_max;              /* 0x1D4 consts.power at reset (PowerMax trigger, through power_max_ref) */
    int powerbar_power;         /* 0x1D8 power shown by the power bar (Lifebar_TickPower) */
    int powerbar_level;         /* 0x1DC power level (thousands) shown, a level sound plays when it rises */
    float attack_mul;           /* 0x1E0 consts.attack / 100 */
    float defence_mul;          /* 0x1E4 100 / consts.defence */
    int facing;                 /* 0x1E8 1 right, -1 left */
    int next_facing;            /* 0x1EC facing to take at the next Player_Update (0 = none; HitDef p1facing / p2facing) */
    int visible;                /* 0x1F0 1 at reset and every tick, 0 with AssertSpecial invisible, KO blinking */
    char pad_1F4[0x1F8 - 0x1F4];
    Coord pos;                  /* 0x1F8 position */
    Coord prev_pos;             /* 0x220 position at the start of the tick */
    Coord vel;                  /* 0x248 velocity */
    float unused_270;           /* 0x270 cleared at reset, never read */
    int sprpriority;            /* 0x274 sprite priority (statedef, SprPriority, HitDef p1/p2sprpriority; AnimState.layer) */
    int air_jumps;              /* 0x278 air jumps left */
    float screen_left;          /* 0x27C left edge of the screen in stage units, for ScreenBound (Player_ScreenBound) */
    float screen_right;         /* 0x280 right edge */
    int pause_movetime[2];      /* 0x284 Pause / SuperPause movetime, indexed by GameVars.pause_kind (the pause kind) */
    int screenbound;            /* 0x28C ScreenBound value (set every tick) */
    int movecamera_x;           /* 0x290 ScreenBound movecamera x */
    int movecamera_y;           /* 0x294 ScreenBound movecamera y */
    int playerpush;             /* 0x298 PlayerPush */
    int width_edge_front;       /* 0x29C Width edge (front, back) */
    int width_edge_back;        /* 0x2A0 */
    int width_player_front;     /* 0x2A4 Width player (front, back), added to the size widths */
    int width_player_back;      /* 0x2A8 */
    unsigned char nostandguard; /* 0x2AC AssertSpecial flags, indexed from here (0x2AC..0x2B4 cleared at reset) */
    unsigned char nocrouchguard; /* 0x2AD */
    unsigned char noairguard;   /* 0x2AE */
    unsigned char noautoturn;   /* 0x2AF */
    unsigned char noshadow;     /* 0x2B0 */
    unsigned char nojugglecheck; /* 0x2B1 */
    unsigned char nowalk;       /* 0x2B2 */
    unsigned char unguardable;  /* 0x2B3 */
    unsigned char invisible;    /* 0x2B4 */
    char pad_2B5[0x2B8 - 0x2B5];
    int proj_contact_id;        /* 0x2B8 ID of the projectile of the last projectile contact (ProjHit... triggers) */
    int proj_contact;           /* 0x2BC that contact: 0 none, 1 guarded, 2 hit, 3 cancelled */
    int proj_contact_time;      /* 0x2C0 ticks since that contact (ProjHitTime...) */
    void *projectiles;          /* 0x2C4 a ProjectileList (Projectiles_Create) */
    KeyedArray *targets;        /* 0x2C8 */
    char pad_2CC[0x2D0 - 0x2CC];
    Coord push_vel;             /* 0x2D0 corner push: velocity added every tick (start of a 0x38-byte block cleared at reset) */
    float push_decay_x;         /* 0x2F8 push_vel.x is multiplied by this every tick (1.0 at reset, 0.7 for a hit) */
    float push_decay_y;         /* 0x2FC push_vel.y is multiplied by this every tick (1.0 at reset) */
    float push_min_speed;       /* 0x300 the push stops when the x speed drops below this */
    int push_time;              /* 0x304 ticks the push is applied, -1 = until it is slow enough */
    int no_keyctrl;             /* 0x308 input ignored (commands cleared): helper keyctrl = 0, versus and result screens */
    int input_cleared;          /* 0x30C input was cleared */
    int no_input_states;        /* 0x310 no automatic state changes from held keys (end of the round) */
    InputBuf input;             /* 0x314 */
    CmdState cmd_state;         /* 0x3D0 */
    const int *input_keys;      /* 0xA2C keys of the player's input device (Input_Resync) */
    CmdSet *cmds;               /* 0xA30 &data->cmds, or the commands of a custom state owner */
    unsigned char cmd_active[CMD_MAX_LABELS]; /* 0xA34 ticks each command label stays active */
    unsigned char unused_AB4[0x200]; /* 0xAB4 cleared at reset, never read */
    StateDefs *states;          /* 0xCB4 data->states; start of a 0x420 block cleared at creation */
    int custom_owner;           /* 0xCB8 index of the player whose states are used (custom state owner), -1 */
    StateDefs *custom_states;   /* 0xCBC states of another player, or NULL */
    CmdSet *custom_cmds;        /* 0xCC0 commands of the custom state owner */
    struct Anim *custom_anim;   /* 0xCC4 animations of the custom state owner (CustomStateInfo.anim, Anim2), or NULL */
    int anim_from_owner;        /* 0xCC8 the animation comes from the custom state owner (ChangeAnim2) */
    int state_no;               /* 0xCCC current state number */
    int prev_state_no;          /* 0xCD0 */
    unsigned char sctrl_wait[0x200]; /* 0xCD4 per controller of the current state: ticks it stays inactive (persistent) */
    int state_time;             /* 0xED4 ticks in the current state (Time trigger) */
    int statetype;              /* 0xED8 STATETYPE_* */
    int movetype;               /* 0xEDC MOVETYPE_* */
    int physics;                /* 0xEE0 PHYSICS_*: 1 stand, 2 crouch, 3 air, 0 none */
    int ctrl;                   /* 0xEE4 */
    int in_guard_dist;          /* 0xEE8 */
    int idle_time;              /* 0xEEC ticks with movetype idle (training: life and power refill) */
    int hitpause_time;          /* 0xEF0 hit pause ticks left, commands frozen (HitPauseTime trigger) */
    int hitpause_next;          /* 0xEF4 hit pause to start (moved to hitpause_time) */
    int paused;                 /* 0xEF8 paused this tick (hit pause or Pause / SuperPause) */
    int pos_freeze;             /* 0xEFC the velocity is not applied (PosFreeze, bound by a target) */
    int alive;                  /* 0xF00 Alive trigger */
    int ko_time;                /* 0xF04 ticks since the KO */
    int update_frame;           /* 0xF08 GameVars.frames of the last Player_Update */
    int move_contact;           /* 0xF0C 1 guarded, 2 hit, 4 reversed (MoveContact, MoveHit, MoveGuarded, MoveReversed) */
    int move_contact_time;      /* 0xF10 its time - 1 */
    int hit_frame;              /* 0xF14 GameVars.frames of the last hit given (HitCount counts once per frame) */
    int state_exists;           /* 0xF18 the current state exists */
    int var[PLAYER_VAR_COUNT];  /* 0xF1C var(n) */
    float fvar[PLAYER_FVAR_COUNT]; /* 0x100C fvar(n) */
    int sysvar[5];              /* 0x10AC sysvar(n) */
    float sysfvar[5];           /* 0x10C0 sysfvar(n) */
    int nohit_time;             /* 0x10D4 start of a 0x158-byte block cleared at reset (up to hit_ids); ticks the
                                   player cannot be hit (after a hit or a reversal) */
    int hitby_attr;             /* 0x10D8 0xFFF at reset; HitBy attributes (HITATTR_*) */
    int nothitby_attr;          /* 0x10DC 0xFFF at reset; NotHitBy attributes */
    int hitby_time;             /* 0x10E0 ticks hitby_attr stays (back to 0xFFF at 0) */
    int nothitby_time;          /* 0x10E4 ticks nothitby_attr stays */
    int gethit_xveladd;         /* 0x10E8 GetHitVar(xveladd), read as an int (cleared by the training KO recovery) */
    int gethit_yveladd;         /* 0x10EC GetHitVar(yveladd), read as an int */
    struct Player *gethit_attacker; /* 0x10F0 the player whose HitDef hit this one (KO win type) */
    int gethit_type;            /* 0x10F4 GetHitVar(type) */
    int gethit_animtype;        /* 0x10F8 GetHitVar(animtype) */
    int gethit_airtype;         /* 0x10FC GetHitVar(airtype) */
    int gethit_groundtype;      /* 0x1100 GetHitVar(groundtype) */
    int gethit_damage;          /* 0x1104 GetHitVar(damage): damage to take at the next Player_Update */
    int gethit_hitcount;        /* 0x1108 GetHitVar(hitcount): hits taken in the current combo */
    int gethit_fallcount;       /* 0x110C GetHitVar(fallcount): falls in the current combo (down: liedown time halved) */
    int gethit_shaketime;       /* 0x1110 hit shake ticks left (HitShakeOver: <= 0) */
    int gethit_hittime;         /* 0x1114 hit ticks left (HitOver: < 0) */
    int gethit_slidetime;       /* 0x1118 GetHitVar(slidetime) */
    int gethit_ctrltime;        /* 0x111C GetHitVar(ctrltime) */
    int gethit_recovertime;     /* 0x1120 consts.liedown_time at reset; scaled down by the automatic state changes; GetHitVar(recovertime) */
    int gethit_attr;            /* 0x1124 attributes of the HitDef that hit the player (KO win type) */
    float gethit_xoff;          /* 0x1128 GetHitVar(xoff): x added to the position at the next Player_Update (snap) */
    float gethit_yoff;          /* 0x112C GetHitVar(yoff): y added to the position at the next Player_Update */
    float gethit_vel_x;         /* 0x1130 HitVel X (times facing) */
    float gethit_vel_y;         /* 0x1134 HitVel Y */
    float gethit_yaccel;        /* 0x1138 get hit y acceleration; GetHitVar(yaccel) */
    int gethit_id;              /* 0x113C hit ID of the HitDef that hit the player (target(ID) redirection, NumTarget(ID)); GetHitVar(chainid) */
    int gethit_guarded;         /* 0x1140 GetHitVar(guarded) */
    struct Player *gethit_isbound; /* 0x1144 GetHitVar(isbound): the player that binds this one as its target, or NULL */
    int gethit_fall;            /* 0x1148 HitFall */
    int gethit_fall_damage;     /* 0x114C GetHitVar(fall.damage) */
    float gethit_fall_xvel;     /* 0x1150 -FLT_MAX at reset; GetHitVar(fall.xvel) */
    float gethit_fall_yvel;     /* 0x1154 GetHitVar(fall.yvel) */
    int gethit_fall_recover;    /* 0x1158 CanRecover: the fall can be recovered from (with gethit_fall_time >=
                                   gethit_fall_recovertime); GetHitVar(fall.recover) */
    int gethit_fall_time;       /* 0x115C GetHitVar(fall.time) */
    int gethit_fall_recovertime; /* 0x1160 GetHitVar(fall.recovertime) */
    int gethit_fall_kill;       /* 0x1164 GetHitVar(fall.kill) (1 when not hit) */
    int gethit_fall_envshake_unused; /* 0x1168 HitDef.fall_envshake[0] (EnvShake.unused_00, always 0), copied with the
                                   fall.envshake values, never read */
    int gethit_fall_envshake_time; /* 0x116C GetHitVar(fall.envshake.time) */
    int gethit_fall_envshake_dir; /* 0x1170 GetHitVar(fall.envshake.dir) */
    float gethit_fall_envshake_freq; /* 0x1174 GetHitVar(fall.envshake.freq) */
    float gethit_fall_envshake_ampl; /* 0x1178 GetHitVar(fall.envshake.ampl), truncated to an int */
    float gethit_fall_envshake_phase; /* 0x117C GetHitVar(fall.envshake.phase) */
    HitOverride hit_override[8]; /* 0x1180 HitOverride slots */
    int *hit_ids;               /* 0x1220 by attacker index: hit ID (HitDef.serial) of its last HitDef that hit this
                                   player (GameVars.player_slots + 1 ints) */
    float gethit_cornerpush;    /* 0x1224 corner push velocity the attacker gets from this hit */
    int hit_free_time;          /* 0x1228 ticks since the player was last in movetype hit */
    int *hit_pending;           /* 0x122C by attacker index: its HitDef connects this tick (GameVars.player_slots + 1 ints) */
    HitDef hitdef;              /* 0x1230 the current HitDef (or ReversalDef) */
    int hitdef_juggle;          /* 0x1424 juggle points the current HitDef needs, reset every tick */
    float draw_zoom;            /* 0x1428 start of a 0xC4-byte block cleared at reset; draw scale of the tick (stage zoom) */
    DrawParams draw;            /* 0x142C x, y: position in screen space */
    DrawParams draw2;           /* 0x148C */
    int pad_14EC;               /* 0x14EC never accessed by the game */
    Coord draw_offset;          /* 0x14F0 drawing offset: [Size] draw.offset, Offset controller */
    AfterImage *afterimage;     /* 0x1518 an element of GameVars.afterimages, taken by the AfterImage controller */
    int combo_hits;             /* 0x151C hits given in the current combo of the team (combo counter) */
    int hitcount;               /* 0x1520 HitCount */
    int uniqhitcount;           /* 0x1524 UniqHitCount */
    int unused_1528[2];         /* 0x1528 cleared at reset, never read */
    SpriteSet *sprites;         /* 0x1530 data->sff */
    AnimState *anim;            /* 0x1534 */
    int anim_owner;             /* 0x1538 index of the player whose animations are used (custom state owner), -1 */
    int pal_no;                 /* 0x153C 0-based palette */
    PalFx *palfx;               /* 0x1540 &palfx_own */
    PalFx palfx_own;            /* 0x1544 */
    PalRemap *remap;            /* 0x156C &remap_own */
    PalRemap remap_own;         /* 0x1570 */
    unsigned int pal_groups[5]; /* 0x15F4 palette group masks of the player (PalGroups_Build) */
    PalFxDef palfx_def;         /* 0x1608 the player's PalFX (PalFX controller, HitDef palfx) */
    int helper_id;              /* 0x1644 helper ID (helper(ID) redirection, IsHelper(ID), NumHelper(ID)) */
    int parent_id;              /* 0x1648 Player.id of the parent when the helper was created */
    struct Player *parent;      /* 0x164C helper: the player that created it (may have been destroyed: check parent_id) */
    struct Player *root;        /* 0x1650 player whose commands this one shares, or NULL; root of a helper */
    int helper_player;          /* 0x1654 helper of type "player" (gets the player-only behaviour) */
    int ai;                     /* 0x1658 computer controlled (training dummy: cooperative / AI); start of the AI block (0xDF8 bytes) */
    AiState ai_state;           /* 0x165C */
    int dummy_control;          /* 0x1CFC training dummy: TRAINING_COOPERATIVE or TRAINING_AI */
    int dummy_mode;             /* 0x1D00 stand, crouch, jump, W jump */
    int dummy_guard;            /* 0x1D04 none, auto */
    int dummy_dist_max;         /* 0x1D08 distance: 50 close, 150 medium, 0 any / far */
    int dummy_dist_min;         /* 0x1D0C 100 medium, 280 far, 0 any / close */
    int dummy_button_jam;       /* 0x1D10 none, a, b, c, x, y, z, start */
    int dummy_timer;            /* 0x1D14 training dummy: ticks before the next key change */
    InputBuf ai_queue;          /* 0x1D18 inputs played before deciding again */
    int ai_threatened;          /* 0x1DD4 an active player has this one in its guard distance */
    int ai_learn;               /* 0x1DD8 the AI has learned data (Ai_InitPlayer) */
    AiData ai_data;             /* 0x1DDC */
    AiLearn ai_record;          /* 0x1EE4 the move being recorded */
    int ai_state_cmds[10];      /* 0x23BC ai_cmds of the last state change by a controller (Ai_CopyCommands) */
    unsigned int ai_state_frame; /* 0x23E4 GameVars.frames of that state change */
    int ai_cmds[10];            /* 0x23E8 command labels used by the CNS this tick, -1 terminated (Ai_AddCommand) */
    int last_proj_id;           /* 0x2410 ProjID of the last projectile fired by a Projectile controller */
    int ctrl_state_no;          /* 0x2414 state_no when the last controller was run (StateCtrl_Run) */
    int ctrl_statetype;         /* 0x2418 statetype then */
    int ctrl_movetype;          /* 0x241C movetype then */
    int ctrl_ctrl;              /* 0x2420 ctrl then */
    int ai_level;               /* 0x2424 Player_GetAiLevel */
    int ai_cmds_out[10];        /* 0x2428 commands chosen by the AI this tick, -1 terminated */
    struct Player *enemy;       /* 0x2450 nearest enemy */
    struct Player *last_enemy;  /* 0x2454 the last active non-helper enemy (when enemy is gone) */
    int rounds_existed;         /* 0x2458 RoundsExisted */
    PlayerRef bind_to;          /* 0x245C BindToParent / BindToRoot / BindToTarget: the other player (start of a
                                   0x24-byte block cleared at reset) */
    int bind_time;              /* 0x246C ticks the player stays bound, -1 forever */
    float bind_x;               /* 0x2470 bind offset x (times the facing) */
    float bind_y;               /* 0x2474 bind offset y */
    int bind_facing;            /* 0x2478 bind facing (times the facing of the other player), 0 = keep */
    int unused_247C;            /* 0x247C cleared with the binding, never read */
    int action_failed;          /* 0x2480 the last action change failed (invalid action or element) */
    TextRing *warnings;         /* 0x2484 3 lines of 0x9D */
    int standby;                /* 0x2488 StandBy trigger */
    char pad_248C[0x2490 - 0x248C];
} Player;
MUGEN_ASSERT_OFFSET(Player, id, 0x004);
MUGEN_ASSERT_OFFSET(Player, team_member, 0x014);
MUGEN_ASSERT_OFFSET(Player, name, 0x020);
MUGEN_ASSERT_OFFSET(Player, consts, 0x050);
MUGEN_ASSERT_OFFSET(Player, active, 0x1B0);
MUGEN_ASSERT_OFFSET(Player, life, 0x1B8);
MUGEN_ASSERT_OFFSET(Player, power_ref, 0x1C8);
MUGEN_ASSERT_OFFSET(Player, powerbar_level, 0x1DC);
MUGEN_ASSERT_OFFSET(Player, attack_mul, 0x1E0);
MUGEN_ASSERT_OFFSET(Player, facing, 0x1E8);
MUGEN_ASSERT_OFFSET(Player, pos, 0x1F8);
MUGEN_ASSERT_OFFSET(Player, vel, 0x248);
MUGEN_ASSERT_OFFSET(Player, unused_270, 0x270);
MUGEN_ASSERT_OFFSET(Player, air_jumps, 0x278);
MUGEN_ASSERT_OFFSET(Player, pause_movetime, 0x284);
MUGEN_ASSERT_OFFSET(Player, playerpush, 0x298);
MUGEN_ASSERT_OFFSET(Player, width_player_back, 0x2A8);
MUGEN_ASSERT_OFFSET(Player, nowalk, 0x2B2);
MUGEN_ASSERT_OFFSET(Player, invisible, 0x2B4);
MUGEN_ASSERT_OFFSET(Player, proj_contact_id, 0x2B8);
MUGEN_ASSERT_OFFSET(Player, projectiles, 0x2C4);
MUGEN_ASSERT_OFFSET(Player, push_vel, 0x2D0);
MUGEN_ASSERT_OFFSET(Player, push_decay_y, 0x2FC);
MUGEN_ASSERT_OFFSET(Player, input, 0x314);
MUGEN_ASSERT_OFFSET(Player, cmd_state, 0x3D0);
MUGEN_ASSERT_OFFSET(Player, input_keys, 0xA2C);
MUGEN_ASSERT_OFFSET(Player, cmd_active, 0xA34);
MUGEN_ASSERT_OFFSET(Player, unused_AB4, 0xAB4);
MUGEN_ASSERT_OFFSET(Player, states, 0xCB4);
MUGEN_ASSERT_OFFSET(Player, custom_anim, 0xCC4);
MUGEN_ASSERT_OFFSET(Player, anim_from_owner, 0xCC8);
MUGEN_ASSERT_OFFSET(Player, state_no, 0xCCC);
MUGEN_ASSERT_OFFSET(Player, sctrl_wait, 0xCD4);
MUGEN_ASSERT_OFFSET(Player, statetype, 0xED8);
MUGEN_ASSERT_OFFSET(Player, hitpause_time, 0xEF0);
MUGEN_ASSERT_OFFSET(Player, hitpause_next, 0xEF4);
MUGEN_ASSERT_OFFSET(Player, alive, 0xF00);
MUGEN_ASSERT_OFFSET(Player, state_exists, 0xF18);
MUGEN_ASSERT_OFFSET(Player, var, 0xF1C);
MUGEN_ASSERT_OFFSET(Player, fvar, 0x100C);
MUGEN_ASSERT_OFFSET(Player, sysvar, 0x10AC);
MUGEN_ASSERT_OFFSET(Player, sysfvar, 0x10C0);
MUGEN_ASSERT_OFFSET(Player, nothitby_attr, 0x10DC);
MUGEN_ASSERT_OFFSET(Player, gethit_xveladd, 0x10E8);
MUGEN_ASSERT_OFFSET(Player, gethit_type, 0x10F4);
MUGEN_ASSERT_OFFSET(Player, gethit_shaketime, 0x1110);
MUGEN_ASSERT_OFFSET(Player, gethit_recovertime, 0x1120);
MUGEN_ASSERT_OFFSET(Player, gethit_xoff, 0x1128);
MUGEN_ASSERT_OFFSET(Player, gethit_vel_x, 0x1130);
MUGEN_ASSERT_OFFSET(Player, gethit_id, 0x113C);
MUGEN_ASSERT_OFFSET(Player, gethit_guarded, 0x1140);
MUGEN_ASSERT_OFFSET(Player, gethit_fall, 0x1148);
MUGEN_ASSERT_OFFSET(Player, gethit_fall_yvel, 0x1154);
MUGEN_ASSERT_OFFSET(Player, gethit_fall_recover, 0x1158);
MUGEN_ASSERT_OFFSET(Player, gethit_fall_recovertime, 0x1160);
MUGEN_ASSERT_OFFSET(Player, gethit_fall_kill, 0x1164);
MUGEN_ASSERT_OFFSET(Player, gethit_fall_envshake_time, 0x116C);
MUGEN_ASSERT_OFFSET(Player, gethit_fall_envshake_phase, 0x117C);
MUGEN_ASSERT_OFFSET(Player, hit_pending, 0x122C);
MUGEN_ASSERT_OFFSET(Player, hitdef, 0x1230);
MUGEN_ASSERT_OFFSET(Player, hitdef_juggle, 0x1424);
MUGEN_ASSERT_OFFSET(Player, draw, 0x142C);
MUGEN_ASSERT_OFFSET(Player, draw2, 0x148C);
MUGEN_ASSERT_OFFSET(Player, draw_offset, 0x14F0);
MUGEN_ASSERT_OFFSET(Player, afterimage, 0x1518);
MUGEN_ASSERT_OFFSET(Player, hitcount, 0x1520);
MUGEN_ASSERT_OFFSET(Player, sprites, 0x1530);
MUGEN_ASSERT_OFFSET(Player, pal_no, 0x153C);
MUGEN_ASSERT_OFFSET(Player, palfx_own, 0x1544);
MUGEN_ASSERT_OFFSET(Player, remap_own, 0x1570);
MUGEN_ASSERT_OFFSET(Player, pal_groups, 0x15F4);
MUGEN_ASSERT_OFFSET(Player, palfx_def, 0x1608);
MUGEN_ASSERT_OFFSET(Player, helper_id, 0x1644);
MUGEN_ASSERT_OFFSET(Player, parent, 0x164C);
MUGEN_ASSERT_OFFSET(Player, root, 0x1650);
MUGEN_ASSERT_OFFSET(Player, ai, 0x1658);
MUGEN_ASSERT_OFFSET(Player, ai_state, 0x165C);
MUGEN_ASSERT_OFFSET(Player, dummy_control, 0x1CFC);
MUGEN_ASSERT_OFFSET(Player, dummy_button_jam, 0x1D10);
MUGEN_ASSERT_OFFSET(Player, dummy_timer, 0x1D14);
MUGEN_ASSERT_OFFSET(Player, ai_queue, 0x1D18);
MUGEN_ASSERT_OFFSET(Player, ai_threatened, 0x1DD4);
MUGEN_ASSERT_OFFSET(Player, ai_data, 0x1DDC);
MUGEN_ASSERT_OFFSET(Player, ai_record, 0x1EE4);
MUGEN_ASSERT_OFFSET(Player, ai_state_cmds, 0x23BC);
MUGEN_ASSERT_OFFSET(Player, ai_cmds, 0x23E8);
MUGEN_ASSERT_OFFSET(Player, last_proj_id, 0x2410);
MUGEN_ASSERT_OFFSET(Player, ctrl_ctrl, 0x2420);
MUGEN_ASSERT_OFFSET(Player, ai_level, 0x2424);
MUGEN_ASSERT_OFFSET(Player, ai_cmds_out, 0x2428);
MUGEN_ASSERT_OFFSET(Player, enemy, 0x2450);
MUGEN_ASSERT_OFFSET(Player, rounds_existed, 0x2458);
MUGEN_ASSERT_OFFSET(Player, bind_to, 0x245C);
MUGEN_ASSERT_OFFSET(Player, bind_facing, 0x2478);
MUGEN_ASSERT_OFFSET(Player, action_failed, 0x2480);
MUGEN_ASSERT_OFFSET(Player, warnings, 0x2484);
MUGEN_ASSERT_OFFSET(Player, standby, 0x2488);
MUGEN_ASSERT_SIZE(Player, 0x2490);
/* Bytes of the AI block of a Player cleared by Player_Free (from Player.ai). */
#define PLAYER_AI_BLOCK_SIZE 0xDF8

/* Data of a loaded character (0x8F8 bytes), shared by its players (states, commands...). */
typedef struct CharData {
    char name[0x30];            /* 0x000 [Info] name */
    char displayname[0x30];     /* 0x030 [Info] displayname (default: name) */
    char author[0x30];          /* 0x060 [Info] author */
    double localcoord[4];       /* 0x090 width, height, (width - 1) / 2, (height - 1) / 2 */
    unsigned int versiondate;   /* 0x0B0 [Info] versiondate (Version_ParseDate) */
    unsigned int mugenversion;  /* 0x0B4 [Info] mugenversion (Version_Parse) */
    char pad_0B8[0x2B8 - 0x0B8];
    int char_index;             /* 0x2B8 index into GameVars.char_info (PlayerCache_Load) */
    CharConsts consts;          /* 0x2BC [Data], [Size], [Velocity], [Movement] */
    CmdSet cmds;                /* 0x41C commands (.cmd) */
    StateDefs *states;          /* 0x42C */
    SpriteSet *sff;             /* 0x430 */
    Anim *anim;                 /* 0x434 */
    char pad_438[0x440 - 0x438];
    double volumescale;         /* 0x440 [Data] volumescale (default 100) */
    Snd *snd;                   /* 0x448 NULL without sound */
    char ai_file[0x200];        /* 0x44C [Files] ai, saved back by the player cache */
    AiData ai;                  /* 0x64C learned AI */
    char *quotes[CHAR_QUOTE_COUNT]; /* 0x754 [Quotes] victory0..victory99 */
    int quote_count;            /* 0x8E4 */
    int trigger_expr_count;     /* 0x8E8 expressions on trigger lines */
    int pad_8EC;                /* 0x8EC never accessed by the game */
    int expr_count;             /* 0x8F0 expressions compiled by Expr_Compile */
    int lua_ref;                /* 0x8F4 Lua registry reference of the character table (constants in "const") */
} CharData;
MUGEN_ASSERT_OFFSET(CharData, localcoord, 0x090);
MUGEN_ASSERT_OFFSET(CharData, mugenversion, 0x0B4);
MUGEN_ASSERT_OFFSET(CharData, char_index, 0x2B8);
MUGEN_ASSERT_OFFSET(CharData, consts, 0x2BC);
MUGEN_ASSERT_OFFSET(CharData, cmds, 0x41C);
MUGEN_ASSERT_OFFSET(CharData, sff, 0x430);
MUGEN_ASSERT_OFFSET(CharData, volumescale, 0x440);
MUGEN_ASSERT_OFFSET(CharData, ai_file, 0x44C);
MUGEN_ASSERT_OFFSET(CharData, ai, 0x64C);
MUGEN_ASSERT_OFFSET(CharData, quotes, 0x754);
MUGEN_ASSERT_OFFSET(CharData, quote_count, 0x8E4);
MUGEN_ASSERT_OFFSET(CharData, expr_count, 0x8F0);
MUGEN_ASSERT_SIZE(CharData, 0x8F8);

/* Where a state controller or statedef was read from (calloc'ed when GameVars.keep_sources is set). */
typedef struct StateCtrlSource {
    void *statedef;             /* 0x00 StateCtrl: its StateDef (StateDef sources: never set, freed if set) */
    int line;                   /* 0x04 line number in the CNS file */
} StateCtrlSource;
MUGEN_ASSERT_SIZE(StateCtrlSource, 0x08);

/*
 * A state controller (StateDef.ctrls element, key (type, number); StateCtrl_Parse).
 * The meaning of the six Exprs in arg depends on the type (StateCtrl_ParseParams);
 * the types with many parameters keep them in a block (sctrl.h).
 */
typedef struct StateCtrl {
    KeyedArray *triggers;       /* 0x00 trigger Exprs by (group, index); triggerall is group -1 (Trigger_TestList) */
    int trigger_groups;         /* 0x04 trigger1..N */
    int persistent;             /* 0x08 */
    int ignorehitpause;         /* 0x0C */
    int type;                   /* 0x10 SCTRL_* */
    int pad_14;                 /* 0x14 never accessed by the game */
    Expr arg[6];                /* 0x18 parameters, by type */
    void *params;               /* 0x60 parameter block (PlaySndParams, HitDefParams...; CnsText of the clipboard types) */
    StateCtrlSource *source;    /* 0x64 */
} StateCtrl;
MUGEN_ASSERT_OFFSET(StateCtrl, persistent, 0x08);
MUGEN_ASSERT_OFFSET(StateCtrl, type, 0x10);
MUGEN_ASSERT_OFFSET(StateCtrl, arg, 0x18);
MUGEN_ASSERT_OFFSET(StateCtrl, params, 0x60);
MUGEN_ASSERT_OFFSET(StateCtrl, source, 0x64);
MUGEN_ASSERT_SIZE(StateCtrl, 0x68);

#endif /* MUGEN_PLAYER_H */
