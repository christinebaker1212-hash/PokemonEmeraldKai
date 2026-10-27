#ifndef MUGEN_STATEDEF_H
#define MUGEN_STATEDEF_H

/*
 * State definitions (statedef.c): the [Statedef n] groups of a character's
 * CNS files with their state controllers (StateDefs, see char.h), and the
 * state machine that enters a state and runs its controllers.
 */

/* Most controllers a state may have (Player.sctrl_wait holds one persistence counter per controller). */
#define STATEDEF_MAX_CTRLS  0x200

/* StateDef.type / movetype / physics value for "U" (unchanged). */
#define STATEDEF_UNCHANGED  (-1)

/* StateDef.physics values. */
enum {
    PHYSICS_NONE = 0,
    PHYSICS_STAND = 1,
    PHYSICS_CROUCH = 2,
    PHYSICS_AIR = 3
};

/* Player_RunSpecialState flags. */
#define RUNSTATE_OWN        0x01    /* use the player's own states even in a custom state */
#define RUNSTATE_ALWAYS     0x02    /* run when not in a custom state too */
#define RUNSTATE_CUSTOM     0x04    /* run in a custom state too */

/* StateCtrl_Run result bits. */
#define STATECTRL_STOP      0x03    /* stop running the state's controllers (state changed...) */
#define STATECTRL_CHANGED   0x04    /* the state changed: Player_RunState returns 1 */

/* One [Statedef n] (StateDefs.list element, key (n, 0)). States below 0 only have controllers. */
typedef struct StateDef {
    KeyedArray *ctrls;          /* 0x00 StateCtrl in file order */
    int type;                   /* 0x04 STATETYPE_* (default S), STATEDEF_UNCHANGED */
    int movetype;               /* 0x08 MOVETYPE_* (default I), STATEDEF_UNCHANGED */
    int physics;                /* 0x0C PHYSICS_* (default N), STATEDEF_UNCHANGED */
    Expr juggle;                /* 0x10 default -1 (keep) */
    Expr facep2;                /* 0x1C default 0 */
    Expr hitdefpersist;         /* 0x28 default 0 */
    Expr hitcountpersist;       /* 0x34 default 0 */
    Expr movehitpersist;        /* 0x40 default 0 */
    int has_sprpriority;        /* 0x4C sprpriority given */
    Expr sprpriority;           /* 0x50 */
    Expr velset_x;              /* 0x5C default -FLT_MAX (keep) */
    Expr velset_y;              /* 0x68 default -FLT_MAX (keep) */
    Expr ctrl;                  /* 0x74 default -1 (keep) */
    Expr anim;                  /* 0x80 default -1 (keep) */
    Expr poweradd;              /* 0x8C default 0 */
    StateCtrlSource *source;    /* 0x98 line of the group, with GameVars.keep_sources */
} StateDef;
MUGEN_ASSERT_OFFSET(StateDef, juggle, 0x10);
MUGEN_ASSERT_OFFSET(StateDef, has_sprpriority, 0x4C);
MUGEN_ASSERT_OFFSET(StateDef, velset_x, 0x5C);
MUGEN_ASSERT_OFFSET(StateDef, poweradd, 0x8C);
MUGEN_ASSERT_OFFSET(StateDef, source, 0x98);
MUGEN_ASSERT_SIZE(StateDef, 0x9C);

/* Players a running state refers to (StateRunCtx_Init), passed to StateCtrl_Run. */
typedef struct StateRunCtx {
    struct Player *enemy;       /* 0x00 nearest enemy; NULL unless GameType is "VS" */
    struct Player *leader;      /* 0x04 owner of the life and power: the team leader in "VS", else the player */
    struct Player *partner;     /* 0x08 the other member of the team (players 1/3, 2/4) */
    struct Player *parent;      /* 0x0C a helper's parent while it exists and is active */
    struct Player *root;        /* 0x10 */
} StateRunCtx;
MUGEN_ASSERT_SIZE(StateRunCtx, 0x14);

/* CustomStateInfo is defined in player.h (HitDef embeds it). */

#endif /* MUGEN_STATEDEF_H */
