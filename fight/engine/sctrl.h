#ifndef MUGEN_SCTRL_H
#define MUGEN_SCTRL_H

/*
 * State controllers (sctrl.c): the controller types (StateCtrl.type), the
 * parameter blocks of the types with many parameters (StateCtrl.params) and
 * what some of them evaluate to. StateCtrl itself is in player.h.
 */

/* StateCtrl.type: the CNS "type" names map to these codes (StateCtrl_ParseParams). */
enum {
    SCTRL_NULL = 0,                 /* null, forcefeedback */
    SCTRL_CHANGESTATE = 1,
    SCTRL_SELFSTATE = 2,
    SCTRL_TURN = 3,
    SCTRL_VARSET = 4,
    SCTRL_VARADD = 5,
    SCTRL_VARRANDOM = 6,
    SCTRL_VARRANGESET = 7,
    SCTRL_LIFESET = 10,
    SCTRL_LIFEADD = 11,
    SCTRL_POWERSET = 12,
    SCTRL_POWERADD = 13,
    SCTRL_CTRLSET = 20,
    SCTRL_POSSET = 21,
    SCTRL_POSADD = 22,
    SCTRL_POSFREEZE = 23,
    SCTRL_VELSET = 24,
    SCTRL_VELADD = 25,
    SCTRL_VELMUL = 26,
    SCTRL_GRAVITY = 27,
    SCTRL_OFFSET = 28,
    SCTRL_HITBY = 30,               /* hitby, nothitby (the attributes inverted) */
    SCTRL_CHANGEANIM = 32,
    SCTRL_CHANGEANIM2 = 33,
    SCTRL_PLAYSND = 34,
    SCTRL_STOPSND = 35,
    SCTRL_SNDPAN = 36,
    SCTRL_HITDEF = 37,
    SCTRL_REVERSALDEF = 38,
    SCTRL_PROJECTILE = 39,
    SCTRL_WIDTH = 40,
    SCTRL_SCREENBOUND = 50,
    SCTRL_SPRPRIORITY = 51,
    SCTRL_ANGLEDRAW = 60,
    SCTRL_ANGLESET = 61,
    SCTRL_ANGLEADD = 62,
    SCTRL_ANGLEMUL = 63,
    SCTRL_TRANS = 65,
    SCTRL_TARGETBIND = 70,
    SCTRL_BINDTOTARGET = 71,
    SCTRL_TARGETSTATE = 72,
    SCTRL_TARGETPOWERADD = 73,
    SCTRL_TARGETLIFEADD = 74,
    SCTRL_TARGETVELSET = 75,
    SCTRL_TARGETVELADD = 76,
    SCTRL_TARGETFACING = 77,
    SCTRL_TARGETDROP = 78,
    SCTRL_ATTACKDIST = 90,
    SCTRL_PLAYERPUSH = 91,
    SCTRL_HITFALLSET = 100,
    SCTRL_HITVELSET = 101,
    SCTRL_HITFALLDAMAGE = 102,
    SCTRL_HITFALLVEL = 103,
    SCTRL_HITADD = 104,
    SCTRL_ATTACKMULSET = 105,
    SCTRL_DEFENCEMULSET = 106,
    SCTRL_STATETYPESET = 110,
    SCTRL_HITOVERRIDE = 120,
    SCTRL_MOVEHITRESET = 130,
    SCTRL_VICTORYQUOTE = 140,       /* mugenversion 1.0 */
    SCTRL_ASSERTSPECIAL = 150,
    SCTRL_ZOOM = 151,               /* mugenversion 1.1 */
    SCTRL_ENVSHAKE = 200,
    SCTRL_FALLENVSHAKE = 201,
    SCTRL_ENVCOLOR = 202,
    SCTRL_GAMEMAKEANIM = 203,
    SCTRL_AFTERIMAGE = 210,
    SCTRL_AFTERIMAGETIME = 211,
    SCTRL_PAUSE = 212,
    SCTRL_SUPERPAUSE = 214,
    SCTRL_MAKEDUST = 215,
    SCTRL_PALFX = 216,              /* palfx, bgpalfx, allpalfx (PalFxParams.kind) */
    SCTRL_REMAPPAL = 217,
    SCTRL_EXPLOD = 220,
    SCTRL_REMOVEEXPLOD = 221,
    SCTRL_EXPLODBINDTIME = 222,
    SCTRL_MODIFYEXPLOD = 223,
    SCTRL_HELPER = 300,
    SCTRL_DESTROYSELF = 301,
    SCTRL_PARENTVARSET = 304,
    SCTRL_PARENTVARADD = 305,
    SCTRL_DISPLAYTOCLIPBOARD = 310,
    SCTRL_APPENDTOCLIPBOARD = 311,
    SCTRL_CLEARCLIPBOARD = 312,
    SCTRL_BINDTOPARENT = 320,
    SCTRL_BINDTOROOT = 321,
    SCTRL_TAG = 550,                /* parsed, but fatal when run */
    SCTRL_TAGIN = 551,              /* run only: never produced by the parser */
    SCTRL_TAGOUT = 552,             /* run only */
    SCTRL_NOP = 1000                /* run only: does nothing */
};

/* Kind of variable of VarSet/VarAdd/ParentVarSet/ParentVarAdd (StateCtrl.arg[2]). */
enum {
    SCTRL_VAR = 0,
    SCTRL_FVAR = 1,
    SCTRL_SYSVAR = 3,
    SCTRL_SYSFVAR = 4
};

/* PalFxParams.kind */
enum {
    PALFX_PLAYER = 0,               /* PalFX */
    PALFX_BG = 1,                   /* BGPalFX */
    PALFX_ALL = 2                   /* AllPalFX */
};

/* PlaySnd (0x74 bytes). */
typedef struct PlaySndParams {
    Expr channel;                   /* 0x00 default -1 */
    Expr group;                     /* 0x0C value (group, number) */
    Expr number;                    /* 0x18 */
    Expr volume;                    /* 0x24 evaluated but not used */
    Expr volumescale;               /* 0x30 default 100.0 */
    int fight;                      /* 0x3C F prefix: a fight.def common sound */
    Expr lowpriority;               /* 0x40 */
    Expr freqmul;                   /* 0x4C default 1.0; only checked */
    Expr loop;                      /* 0x58 */
    int abspan;                     /* 0x64 pan is abspan */
    Expr pan;                       /* 0x68 pan or abspan */
} PlaySndParams;
MUGEN_ASSERT_OFFSET(PlaySndParams, fight, 0x3C);
MUGEN_ASSERT_OFFSET(PlaySndParams, abspan, 0x64);
MUGEN_ASSERT_SIZE(PlaySndParams, 0x74);

/* AfterImage, and the "afterimage." parameters of a projectile (0x118 bytes). */
typedef struct AfterImageParams {
    Expr time;                      /* 0x000 default 1 */
    Expr length;                    /* 0x00C default 20 */
    Expr timegap;                   /* 0x018 default 1 */
    Expr framegap;                  /* 0x024 default 4 */
    Expr palcolor;                  /* 0x030 default 256 */
    Expr palinvertall;              /* 0x03C default 0 */
    Expr palbright[3];              /* 0x048 default 30, 30, 30 */
    Expr palcontrast[3];            /* 0x06C default 120, 120, 220 */
    Expr palpostbright[3];          /* 0x090 default 0 */
    Expr paladd[3];                 /* 0x0B4 default 10, 10, 25 */
    Expr palmul[3];                 /* 0x0D8 default 0.65, 0.65, 0.75 */
    int trans;                      /* 0x0FC Trans_Parse, 0 = none */
    Expr alpha[2];                  /* 0x100 source, dest (default 256, 256) */
} AfterImageParams;
MUGEN_ASSERT_OFFSET(AfterImageParams, palbright, 0x048);
MUGEN_ASSERT_OFFSET(AfterImageParams, palmul, 0x0D8);
MUGEN_ASSERT_OFFSET(AfterImageParams, trans, 0x0FC);
MUGEN_ASSERT_SIZE(AfterImageParams, 0x118);

/* EnvShake, and the "envshake." / "fall.envshake." parameters of a HitDef (0x40 bytes). */
typedef struct EnvShakeParams {
    Expr time;                      /* 0x00 default 0 */
    Expr dir;                       /* 0x0C default 0; never evaluated */
    Expr freq;                      /* 0x18 default 60.0 */
    Expr ampl;                      /* 0x24 default -4 */
    Expr phase;                     /* 0x30 default 0.0 */
    int no_phase;                   /* 0x3C phase not given (90 when freq >= 90) */
} EnvShakeParams;
MUGEN_ASSERT_SIZE(EnvShakeParams, 0x40);

/* An evaluated EnvShake (HitDef.envshake / fall_envshake, GameVars.envshake, Player.gethit_fall_envshake_unused). */
typedef struct EnvShake {
    int unused_00;                  /* 0x00 cleared when the EnvShake is evaluated, never read */
    int time;                       /* 0x04 */
    int dir;                        /* 0x08 left 0 */
    float freq;                     /* 0x0C */
    float ampl;                     /* 0x10 in game units */
    float phase;                    /* 0x14 */
} EnvShake;
MUGEN_ASSERT_SIZE(EnvShake, 0x18);

/* Explod / ModifyExplod (0x1B4 bytes); ModifyExplod leaves INT_MIN / -FLT_MAX for unchanged values. */
typedef struct ExplodParams {
    Expr id;                        /* 0x000 */
    Expr anim;                      /* 0x00C */
    int anim_fight;                 /* 0x018 F prefix */
    Expr pos[2];                    /* 0x01C */
    Expr vel[2];                    /* 0x034 vel or velocity */
    Expr accel[2];                  /* 0x04C */
    Expr random[2];                 /* 0x064 */
    Expr supermove;                 /* 0x07C */
    Expr supermovetime;             /* 0x088 */
    Expr pausemovetime;             /* 0x094 */
    Expr removetime;                /* 0x0A0 default -2 */
    Expr scale[2];                  /* 0x0AC default 1.0 */
    Expr angle;                     /* 0x0C4 */
    Expr yangle;                    /* 0x0D0 */
    Expr xangle;                    /* 0x0DC */
    int space;                      /* 0x0E8 1 screen, 0 stage, -1 auto */
    int postype;                    /* 0x0EC POSTYPE_*, -1 */
    Expr bindid;                    /* 0x0F0 default -2 */
    Expr bindtime;                  /* 0x0FC default 1 */
    Expr sprpriority;               /* 0x108 */
    Expr ontop;                     /* 0x114 */
    Expr facing;                    /* 0x120 default 1 */
    Expr vfacing;                   /* 0x12C default 1 */
    Expr shadow[3];                 /* 0x138 */
    Expr ownpal;                    /* 0x15C default: anim_fight */
    Expr remappal[2];               /* 0x168 default -1, 0 */
    Expr removeongethit;            /* 0x180 */
    Expr ignorehitpause;            /* 0x18C default 1 */
    int trans;                      /* 0x198 Trans_Parse, default -1 */
    Expr alpha[2];                  /* 0x19C */
} ExplodParams;
MUGEN_ASSERT_OFFSET(ExplodParams, supermove, 0x07C);
MUGEN_ASSERT_OFFSET(ExplodParams, space, 0x0E8);
MUGEN_ASSERT_OFFSET(ExplodParams, shadow, 0x138);
MUGEN_ASSERT_OFFSET(ExplodParams, trans, 0x198);
MUGEN_ASSERT_SIZE(ExplodParams, 0x1B4);

/* PalFX / BGPalFX / AllPalFX, and the "palfx." parameters of a HitDef (0xA0 bytes). */
typedef struct PalFxParams {
    int kind;                       /* 0x00 PALFX_* */
    Expr time;                      /* 0x04 */
    Expr color;                     /* 0x10 default 256 */
    Expr add[3];                    /* 0x1C */
    Expr mul[3];                    /* 0x40 default 256 */
    Expr sinadd[3];                 /* 0x64 */
    Expr invertall;                 /* 0x88 */
    Expr period;                    /* 0x94 fourth sinadd value */
} PalFxParams;
MUGEN_ASSERT_OFFSET(PalFxParams, mul, 0x40);
MUGEN_ASSERT_OFFSET(PalFxParams, invertall, 0x88);
MUGEN_ASSERT_SIZE(PalFxParams, 0xA0);

/* HitOverride (0x28 bytes). */
typedef struct HitOverrideParams {
    unsigned int attr;              /* 0x00 HITATTR_* */
    Expr stateno;                   /* 0x04 default -1 */
    Expr time;                      /* 0x10 default 1 */
    Expr forceair;                  /* 0x1C */
} HitOverrideParams;
MUGEN_ASSERT_SIZE(HitOverrideParams, 0x28);

/* HitDef / ReversalDef, and the HitDef of a projectile (0x4F0 bytes). See HitDef_SetFromParams. */
typedef struct HitDefParams {
    int affectteam;                 /* 0x000 1 F, 2 E (default), 3 B */
    int attr;                       /* 0x004 HitDefAttr_Parse */
    Expr id;                        /* 0x008 */
    Expr chainid;                   /* 0x014 default -1 */
    Expr nochainid[2];              /* 0x020 default -1 */
    Expr kill;                      /* 0x038 default 1 */
    Expr guard_kill;                /* 0x044 */
    Expr fall_kill;                 /* 0x050 */
    Expr hitonce;                   /* 0x05C default: a throw attribute */
    Expr air_juggle;                /* 0x068 default -1 */
    Expr damage[2];                 /* 0x074 hit, guard */
    Expr getpower[2];               /* 0x08C */
    Expr givepower[2];              /* 0x0A4 */
    Expr p2_pausetime;              /* 0x0BC second pausetime value (shake time) */
    Expr p1_pausetime;              /* 0x0C8 */
    Expr numhits;                   /* 0x0D4 default 1 */
    Expr hitsound[2];               /* 0x0E0 group (default -1), number */
    int hitsound_fight;             /* 0x0F8 default 1 (F) */
    Expr guardsound[2];             /* 0x0FC */
    int guardsound_fight;           /* 0x114 */
    unsigned int guardflag;         /* 0x118 HitFlags_Parse */
    unsigned int hitflag;           /* 0x11C default 0x3E */
    Expr priority;                  /* 0x120 default 4 */
    int priority_type;              /* 0x12C 0 hit, 1 miss, 2 dodge */
    Expr p1stateno;                 /* 0x130 default -1 */
    Expr p2stateno;                 /* 0x13C default -1 */
    Expr p2getp1state;              /* 0x148 default 1 with p2stateno */
    Expr snap_time;                 /* 0x154 fourth (third from 1.0) snap value */
    Expr p1sprpriority;             /* 0x160 p1sprpriority or sprpriority, default 1 */
    Expr p2sprpriority;             /* 0x16C */
    int animtype;                   /* 0x178 0 light .. 5 diagup */
    Expr forcestand;                /* 0x17C */
    Expr forcenofall;               /* 0x188 */
    int forcestand_given;           /* 0x194 */
    unsigned int reversal_attr;     /* 0x198 ReversalDef reversal.attr */
    Expr fall_damage;               /* 0x19C */
    int fall_animtype;              /* 0x1A8 */
    Expr fall_xvelocity;            /* 0x1AC default -FLT_MAX */
    Expr fall_yvelocity;            /* 0x1B8 default -4.5 in the character's units */
    Expr fall_recover;              /* 0x1C4 default 1 */
    Expr fall_recovertime;          /* 0x1D0 default 4 */
    EnvShakeParams fall_envshake;   /* 0x1DC "fall.envshake." */
    Expr fall;                      /* 0x21C */
    Expr sparkno;                   /* 0x228 default -2 */
    int spark_own;                  /* 0x234 S prefix */
    Expr guard_sparkno;             /* 0x238 default -2 */
    int guard_spark_own;            /* 0x244 */
    Expr sparkxy[2];                /* 0x248 */
    Expr p1facing;                  /* 0x260 */
    Expr p1getp2facing;             /* 0x26C */
    Expr mindist[2];                /* 0x278 default -1025 */
    Expr maxdist[2];                /* 0x290 default -1025 */
    Expr p2facing;                  /* 0x2A8 */
    int ground_type;                /* 0x2B4 0 none, 1 high (default), 2 low, 3 trip */
    Expr ground_hittime;            /* 0x2B8 */
    Expr ground_slidetime;          /* 0x2C4 */
    Expr ground_velocity[2];        /* 0x2D0 */
    Expr ground_cornerpush;         /* 0x2E8 */
    int air_type;                   /* 0x2F4 default ground_type */
    int air_animtype;               /* 0x2F8 default animtype */
    Expr air_hittime;               /* 0x2FC default 20 */
    Expr air_fall;                  /* 0x308 default fall */
    Expr air_velocity[2];           /* 0x314 */
    Expr air_cornerpush;            /* 0x32C */
    Expr down_bounce;               /* 0x338 */
    Expr down_hittime;              /* 0x344 default 20 */
    Expr down_velocity[2];          /* 0x350 */
    Expr down_cornerpush;           /* 0x368 */
    Expr guard_velocity;            /* 0x374 */
    Expr guard_hittime;             /* 0x380 */
    Expr guard_slidetime;           /* 0x38C */
    Expr guard_ctrltime;            /* 0x398 */
    Expr guard_dist;                /* 0x3A4 default -1 */
    Expr guard_p2_pausetime;        /* 0x3B0 */
    Expr guard_p1_pausetime;        /* 0x3BC */
    Expr guard_cornerpush;          /* 0x3C8 */
    Expr airguard_velocity[2];      /* 0x3D4 */
    Expr airguard_ctrltime;         /* 0x3EC */
    Expr airguard_cornerpush;       /* 0x3F8 */
    Expr yaccel;                    /* 0x404 default -FLT_MAX */
    PalFxParams palfx;              /* 0x410 "palfx." */
    EnvShakeParams envshake;        /* 0x4B0 "envshake." */
} HitDefParams;
MUGEN_ASSERT_OFFSET(HitDefParams, p2_pausetime, 0x0BC);
MUGEN_ASSERT_OFFSET(HitDefParams, hitsound_fight, 0x0F8);
MUGEN_ASSERT_OFFSET(HitDefParams, priority_type, 0x12C);
MUGEN_ASSERT_OFFSET(HitDefParams, reversal_attr, 0x198);
MUGEN_ASSERT_OFFSET(HitDefParams, fall_envshake, 0x1DC);
MUGEN_ASSERT_OFFSET(HitDefParams, spark_own, 0x234);
MUGEN_ASSERT_OFFSET(HitDefParams, ground_type, 0x2B4);
MUGEN_ASSERT_OFFSET(HitDefParams, air_type, 0x2F4);
MUGEN_ASSERT_OFFSET(HitDefParams, guard_dist, 0x3A4);
MUGEN_ASSERT_OFFSET(HitDefParams, yaccel, 0x404);
MUGEN_ASSERT_OFFSET(HitDefParams, palfx, 0x410);
MUGEN_ASSERT_OFFSET(HitDefParams, envshake, 0x4B0);
MUGEN_ASSERT_SIZE(HitDefParams, 0x4F0);

/* Projectile (0x69C bytes). */
typedef struct ProjectileParams {
    Expr projid;                    /* 0x000 */
    Expr offset[2];                 /* 0x00C */
    int postype;                    /* 0x024 */
    Expr projremove;                /* 0x028 default 1 */
    Expr projremovetime;            /* 0x034 default -1 */
    Expr projmisstime;              /* 0x040 */
    Expr projedgebound;             /* 0x04C default 40 in the character's units */
    Expr projstagebound;            /* 0x058 default 40 */
    Expr projheightbound[2];        /* 0x064 default -240, 1 */
    Expr projhits;                  /* 0x07C default 1 */
    Expr projpriority;              /* 0x088 default 1 */
    Expr projanim;                  /* 0x094 */
    Expr projhitanim;               /* 0x0A0 default -1 */
    Expr projremanim;               /* 0x0AC default projhitanim */
    Expr projcancelanim;            /* 0x0B8 default projremanim */
    Expr projshadow[3];             /* 0x0C4 */
    Expr projsprpriority;           /* 0x0E8 default 3 */
    Expr velocity[2];               /* 0x0F4 */
    Expr velmul[2];                 /* 0x10C default 1.0 */
    Expr remvelocity[2];            /* 0x124 */
    Expr accel[2];                  /* 0x13C */
    Expr projscale[2];              /* 0x154 default 1.0 */
    Expr supermovetime;             /* 0x16C */
    Expr pausemovetime;             /* 0x178 */
    Expr ownpal;                    /* 0x184 */
    Expr remappal[2];               /* 0x190 default -1 */
    HitDefParams hitdef;            /* 0x1A8 */
    AfterImageParams *afterimage;   /* 0x698 "afterimage." parameters when afterimage.time is given */
} ProjectileParams;
MUGEN_ASSERT_OFFSET(ProjectileParams, postype, 0x024);
MUGEN_ASSERT_OFFSET(ProjectileParams, projshadow, 0x0C4);
MUGEN_ASSERT_OFFSET(ProjectileParams, hitdef, 0x1A8);
MUGEN_ASSERT_OFFSET(ProjectileParams, afterimage, 0x698);
MUGEN_ASSERT_SIZE(ProjectileParams, 0x69C);

/* Helper (0x158 bytes); the size.* values are INT_MIN / -FLT_MAX when not given. */
typedef struct HelperParams {
    char name[0x30];                /* 0x000 */
    Expr id;                        /* 0x030 */
    int helpertype;                 /* 0x03C 0 normal, 1 player, 2 proj */
    Expr pos[2];                    /* 0x040 */
    int postype;                    /* 0x058 */
    Expr facing;                    /* 0x05C default 1 */
    Expr stateno;                   /* 0x068 */
    Expr keyctrl;                   /* 0x074 */
    Expr ownpal;                    /* 0x080 */
    Expr remappal[2];               /* 0x08C default -1 */
    Expr supermovetime;             /* 0x0A4 */
    Expr pausemovetime;             /* 0x0B0 */
    Expr xscale;                    /* 0x0BC size.xscale */
    Expr yscale;                    /* 0x0C8 */
    Expr ground_back;               /* 0x0D4 size.ground.back */
    Expr ground_front;              /* 0x0E0 */
    Expr air_back;                  /* 0x0EC */
    Expr air_front;                 /* 0x0F8 */
    Expr height;                    /* 0x104 */
    Expr proj_doscale;              /* 0x110 */
    Expr head_pos[2];               /* 0x11C */
    Expr mid_pos[2];                /* 0x134 */
    Expr shadowoffset;              /* 0x14C */
} HelperParams;
MUGEN_ASSERT_OFFSET(HelperParams, helpertype, 0x03C);
MUGEN_ASSERT_OFFSET(HelperParams, postype, 0x058);
MUGEN_ASSERT_OFFSET(HelperParams, xscale, 0x0BC);
MUGEN_ASSERT_OFFSET(HelperParams, shadowoffset, 0x14C);
MUGEN_ASSERT_SIZE(HelperParams, 0x158);

/* Pause (0x30 bytes), also the start of SuperPauseParams. */
typedef struct PauseParams {
    Expr time;                      /* 0x00 */
    Expr movetime;                  /* 0x0C */
    Expr endcmdbuftime;             /* 0x18 */
    Expr pausebg;                   /* 0x24 default 1 */
} PauseParams;
MUGEN_ASSERT_SIZE(PauseParams, 0x30);

/* SuperPause (0xA4 bytes). */
typedef struct SuperPauseParams {
    PauseParams pause;              /* 0x00 time defaults to 30 */
    int anim_fight;                 /* 0x30 default 1 (F) */
    Expr anim;                      /* 0x34 default 100 */
    Expr pos[2];                    /* 0x40 */
    Expr darken;                    /* 0x58 default 1 */
    Expr p2defmul;                  /* 0x64 default 0.0 */
    Expr poweradd;                  /* 0x70 */
    Expr unhittable;                /* 0x7C default 1 */
    int sound_fight;                /* 0x88 default 1 (F) */
    Expr sound[2];                  /* 0x8C group (default -1), number */
} SuperPauseParams;
MUGEN_ASSERT_OFFSET(SuperPauseParams, anim_fight, 0x30);
MUGEN_ASSERT_OFFSET(SuperPauseParams, sound_fight, 0x88);
MUGEN_ASSERT_SIZE(SuperPauseParams, 0xA4);

#endif /* MUGEN_SCTRL_H */
