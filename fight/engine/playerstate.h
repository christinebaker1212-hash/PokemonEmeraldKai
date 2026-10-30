#ifndef MUGEN_PLAYERSTATE_H
#define MUGEN_PLAYERSTATE_H

/*
 * Types of the player runtime (playerstate.c): projectiles, the palette
 * groups of AllPalFX / BGPalFX and their definitions. HitDef, TargetEntry
 * and HitOverride are in player.h.
 */

/* Projectile.state */
enum {
    PROJ_STATE_LIVE = 1,        /* moving, can hit */
    PROJ_STATE_REMOVED = 2,     /* out of bounds, out of time or hit by a HitDef: remove animation */
    PROJ_STATE_CANCELLED = 3,   /* cancelled by another projectile: cancel animation */
    PROJ_STATE_HIT = 4          /* hit a player: hit animation */
};

/* A projectile (Projectile controller), 0x508 bytes, in ProjectileList.list. */
typedef struct Projectile {
    int id;                     /* 0x000 ProjID */
    int state;                  /* 0x004 PROJ_STATE_* */
    int team_side;              /* 0x008 team of the owner */
    int priority;               /* 0x00C projpriority */
    int anim_hit;               /* 0x010 projhitanim (state 4), -1 = remove at once */
    int anim_remove;            /* 0x014 projremanim (state 2) */
    int anim_cancel;            /* 0x018 projcancelanim (state 3) */
    int hits;                   /* 0x01C projhits left */
    int ticks;                  /* 0x020 ticks alive (not counted while paused) */
    int time;                   /* 0x024 ticks in the current state */
    int hitpause_time;          /* 0x028 hit pause ticks left */
    int miss_time;              /* 0x02C projmisstime ticks left */
    int projmisstime;           /* 0x030 projmisstime */
    int facing;                 /* 0x034 */
    int removeonhit;            /* 0x038 projremove */
    int removetime;             /* 0x03C projremovetime, -1 = never */
    int shadow;                 /* 0x040 */
    int pad_44;                 /* 0x044 never accessed by the game */
    Coord pos;                  /* 0x048 */
    double draw_scale_x;        /* 0x070 projscale */
    double draw_scale_y;        /* 0x078 */
    char pad_080[0x098 - 0x080];
    float scale;                /* 0x098 screen scale of the last Projectiles_UpdateDraw */
    DrawParams draw;            /* 0x09C */
    PalFx *palfx;               /* 0x0FC &palfx_own */
    PalFx palfx_own;            /* 0x100 */
    PalRemap *remap;            /* 0x128 &remap_own */
    PalRemap remap_own;         /* 0x12C */
    Coord vel;                  /* 0x1B0 velocity */
    Coord remvel;               /* 0x1D8 velocity after it is removed */
    Coord velmul;               /* 0x200 velocity multiplier (x, y) */
    Coord accel;                /* 0x228 */
    int bound[5];               /* 0x250 projedgebound, projstagebound (left), projheightbound low, stage right,
                                   height high */
    HitDef hitdef;              /* 0x264 */
    AnimState anim;             /* 0x458 */
    int sprpriority;            /* 0x4E8 */
    int paused;                 /* 0x4EC paused this tick (pause_movetime ran out) */
    int hit_pending;            /* 0x4F0 hit a player this tick (applied by Projectiles_ApplyHits) */
    int hitpause_next;          /* 0x4F4 hit pause to start */
    int miss_time_next;         /* 0x4F8 miss time to start */
    int pause_movetime[2];      /* 0x4FC pausemovetime, supermovetime: ticks it keeps moving in a Pause / SuperPause (by GameVars.pause_kind) */
    AfterImage *afterimage;     /* 0x504 an element of GameVars.afterimages, taken by the afterimage parameters;
                                   its anim and running are cleared on removal (as Player_Reset does) */
} Projectile;
MUGEN_ASSERT_OFFSET(Projectile, pos, 0x048);
MUGEN_ASSERT_OFFSET(Projectile, scale, 0x098);
MUGEN_ASSERT_OFFSET(Projectile, remap_own, 0x12C);
MUGEN_ASSERT_OFFSET(Projectile, vel, 0x1B0);
MUGEN_ASSERT_OFFSET(Projectile, accel, 0x228);
MUGEN_ASSERT_OFFSET(Projectile, bound, 0x250);
MUGEN_ASSERT_OFFSET(Projectile, hitdef, 0x264);
MUGEN_ASSERT_OFFSET(Projectile, anim, 0x458);
MUGEN_ASSERT_OFFSET(Projectile, pause_movetime, 0x4FC);
MUGEN_ASSERT_SIZE(Projectile, 0x508);

/* The projectiles of a player (Player.projectiles, Projectiles_Create). */
typedef struct ProjectileList {
    KeyedArray *list;           /* 0x00 Projectile by (0, 0) */
    Anim *anim;                 /* 0x04 the player's animations */
    SpriteSet *sprites;         /* 0x08 */
} ProjectileList;
MUGEN_ASSERT_SIZE(ProjectileList, 0x0C);

/* A palette group (GameVars.palgroups): a PalFx and the group masks it belongs to. */
typedef struct PalGroupEntry {
    PalFx *fx;                  /* 0x00 */
    unsigned int mask[5];       /* 0x04 */
} PalGroupEntry;
MUGEN_ASSERT_SIZE(PalGroupEntry, 0x18);

typedef struct PalGroups {
    PalGroupEntry *entries;     /* 0x00 */
    int capacity;               /* 0x04 */
    int count;                  /* 0x08 */
} PalGroups;
MUGEN_ASSERT_SIZE(PalGroups, 0x0C);

#endif /* MUGEN_PLAYERSTATE_H */
