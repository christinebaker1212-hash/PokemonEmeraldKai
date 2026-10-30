#ifndef MUGEN_FIGHT_H
#define MUGEN_FIGHT_H

/*
 * fight.def: the fight screen (lifebars, powerbars, faces, names, timer,
 * combo counters, round announcements and win icons), loaded by Fight_Load
 * into GameVars.fight.
 */

struct Anim;

/* A life or power bar of one player ([Lifebar], [Simul Lifebar], [Turns Lifebar], [Powerbar]). */
typedef struct FightBar {
    int pos[2];             /* 0x000 p<n>.pos */
    int range[2];           /* 0x008 p<n>.range.x */
    int pad_010;            /* 0x010 never accessed by the game */
    SysElem bg[3];          /* 0x014 p<n>.bg0..bg2 */
    SysElem mid;            /* 0x218 */
    SysElem front;          /* 0x2C4 */
} FightBar;
MUGEN_ASSERT_OFFSET(FightBar, bg, 0x014);
MUGEN_ASSERT_OFFSET(FightBar, front, 0x2C4);
MUGEN_ASSERT_SIZE(FightBar, 0x370);

typedef struct FightFacePlayer {
    SysElem bg[3];          /* 0x000 p<n>.bg0..bg2 (p<n>.bg goes to bg[0]) */
    SysElem face;           /* 0x204 */
} FightFacePlayer;
MUGEN_ASSERT_SIZE(FightFacePlayer, 0x2B0);

/* [Face], [Simul Face], [Turns Face] (room for 4 players in each). */
typedef struct FightFace {
    FightFacePlayer p[4];   /* 0x000 */
    int pos_x[4];           /* 0xAC0 p<n>.pos */
    int pos_y[4];           /* 0xAD0 */
    int facing[4];          /* 0xAE0 */
} FightFace;
MUGEN_ASSERT_OFFSET(FightFace, pos_x, 0xAC0);
MUGEN_ASSERT_OFFSET(FightFace, facing, 0xAE0);
MUGEN_ASSERT_SIZE(FightFace, 0xAF0);

/* [Turns Face] p<n>.teammate.* */
typedef struct FightTeammate {
    SysElem bg[3];          /* 0x000 */
    SysElem face;           /* 0x204 */
    SysElem ko;             /* 0x2B0 */
    int pos[2];             /* 0x35C */
    int pad_364;            /* 0x364 never accessed by the game */
    int spacing[2];         /* 0x368 */
} FightTeammate;
MUGEN_ASSERT_OFFSET(FightTeammate, pos, 0x35C);
MUGEN_ASSERT_OFFSET(FightTeammate, spacing, 0x368);
MUGEN_ASSERT_SIZE(FightTeammate, 0x370);

/* [Name], [Simul Name], [Turns Name] */
typedef struct FightName {
    SysElem bg[4];          /* 0x000 p<n>.bg */
    SysElem name[4];        /* 0x2B0 p<n>.name */
    int pos_x[4];           /* 0x560 p<n>.pos */
    int pos_y[4];           /* 0x570 */
} FightName;
MUGEN_ASSERT_OFFSET(FightName, pos_x, 0x560);
MUGEN_ASSERT_SIZE(FightName, 0x580);

/* [Time] */
typedef struct FightTime {
    SysElem bg;             /* 0x000 */
    SysElem counter;        /* 0x0AC */
    int pos[2];             /* 0x158 */
    int frames_per_count;   /* 0x160 */
} FightTime;
MUGEN_ASSERT_OFFSET(FightTime, pos, 0x158);
MUGEN_ASSERT_SIZE(FightTime, 0x164);

/* [Combo], one per team. */
typedef struct FightCombo {
    int pos[2];             /* 0x000 */
    int start_x;            /* 0x008 */
    int displaytime;        /* 0x00C */
    int shake;              /* 0x010 counter.shake */
    SysElem counter;        /* 0x014 */
    char counter_fmt[0x40]; /* 0x0C0 counter text ("%i" if empty) */
    SysElem text;           /* 0x100 */
    char text_fmt[0x40];    /* 0x1AC */
} FightCombo;
MUGEN_ASSERT_OFFSET(FightCombo, counter, 0x014);
MUGEN_ASSERT_OFFSET(FightCombo, counter_fmt, 0x0C0);
MUGEN_ASSERT_OFFSET(FightCombo, text_fmt, 0x1AC);
MUGEN_ASSERT_SIZE(FightCombo, 0x1EC);

/* Sound references are (group, number) pairs; a group of -1 means none. */

/* [Round] */
typedef struct FightRound {
    int match_wins;             /* 0x000 */
    int match_maxdrawgames;     /* 0x004 */
    int start_waittime;         /* 0x008 */
    int intro_time;             /* 0x00C */
    int pos[2];                 /* 0x010 */
    SysElem round_default;      /* 0x018 round.default ("%%" collapsed) */
    SysElem unused_0c4;         /* 0x0C4 an element Fight_Load never reads (never accessed) */
    SysElem round[9];           /* 0x170 round1..round9 */
    int round_time;             /* 0x77C */
    int round_sndtime;          /* 0x780 */
    int round_snd_group[10];    /* 0x784 round<n>.snd, n = 1..9 */
    int round_snd_number[10];   /* 0x7AC */
    SysElem fight;              /* 0x7D4 */
    int fight_time;             /* 0x880 */
    int fight_sndtime;          /* 0x884 */
    int fight_snd[2];           /* 0x888 */
    SysElem ko;                 /* 0x890 KO */
    SysElem dko;                /* 0x93C DKO */
    SysElem to;                 /* 0x9E8 TO */
    int ko_time;                /* 0xA94 */
    int ko_sndtime;             /* 0xA98 */
    int ko_snd[2];              /* 0xA9C */
    int dko_snd[2];             /* 0xAA4 */
    int to_snd[2];              /* 0xAAC */
    SysElem win;                /* 0xAB4 */
    SysElem win2;               /* 0xB60 */
    SysElem draw;               /* 0xC0C */
    int win_snd[2];             /* 0xCB8 */
    int lose_snd[2];            /* 0xCC0 */
    int draw_snd[2];            /* 0xCC8 */
    int win_time;               /* 0xCD0 */
    int ctrl_time;              /* 0xCD4 */
    int slow_time;              /* 0xCD8 */
    int over_waittime;          /* 0xCDC */
    int over_hittime;           /* 0xCE0 */
    int over_wintime;           /* 0xCE4 */
    int over_time;              /* 0xCE8 */
} FightRound;
MUGEN_ASSERT_OFFSET(FightRound, round, 0x170);
MUGEN_ASSERT_OFFSET(FightRound, round_time, 0x77C);
MUGEN_ASSERT_OFFSET(FightRound, fight, 0x7D4);
MUGEN_ASSERT_OFFSET(FightRound, ko, 0x890);
MUGEN_ASSERT_OFFSET(FightRound, ko_time, 0xA94);
MUGEN_ASSERT_OFFSET(FightRound, win, 0xAB4);
MUGEN_ASSERT_OFFSET(FightRound, win_snd, 0xCB8);
MUGEN_ASSERT_SIZE(FightRound, 0xCEC);

/* Win types, index into FightWinIcon.icon. */
enum {
    WIN_ICON_N = 0,         /* normal */
    WIN_ICON_S = 1,         /* special */
    WIN_ICON_H = 2,         /* hyper */
    WIN_ICON_C = 3,         /* cheese */
    WIN_ICON_T = 4,         /* time over */
    WIN_ICON_THROW = 5,
    WIN_ICON_SUICIDE = 6,
    WIN_ICON_TEAMMATE = 7,
    WIN_ICON_PERFECT = 8,
    WIN_ICON_COUNT = 9
};

/* [WinIcon] */
typedef struct FightWinIcon {
    SysElem counter[2];                 /* 0x000 */
    SysElem bg0[2];                     /* 0x158 */
    SysElem icon[2][WIN_ICON_COUNT];    /* 0x2B0 */
    int pos_x[2];                       /* 0xEC8 */
    int pos_y[2];                       /* 0xED0 */
    int iconoffset_x[2];                /* 0xED8 */
    int iconoffset_y[2];                /* 0xEE0 */
    int useiconupto;                    /* 0xEE8 at most 9 */
} FightWinIcon;
MUGEN_ASSERT_OFFSET(FightWinIcon, icon, 0x2B0);
MUGEN_ASSERT_OFFSET(FightWinIcon, pos_x, 0xEC8);
MUGEN_ASSERT_SIZE(FightWinIcon, 0xEEC);

#define FIGHT_FONT_COUNT 10
#define FIGHT_LEVEL_SNDS 10

/* Everything loaded from fight.def (GameVars.fight). */
typedef struct FightDef {
    SpriteSet *sff;                         /* 0x0000 [Files] sff */
    struct Anim *anim;                      /* 0x0004 the "[Begin Action n]" groups of fight.def */
    struct Font *fonts[FIGHT_FONT_COUNT];   /* 0x0008 [Files] font0..font9 */
    Snd *snd;                               /* 0x0030 [Files] snd (NULL without sound) */
    Snd *common_snd;                        /* 0x0034 [Files] common.snd (NULL without sound) */
    FightBar lifebar[2];                    /* 0x0038 */
    FightBar simul_lifebar[4];              /* 0x0718 */
    FightBar turns_lifebar[2];              /* 0x14D8 */
    FightBar unused_1bb8[2];                /* 0x1BB8 two bars Fight_Load never reads (never accessed) */
    FightBar powerbar[2];                   /* 0x2298 */
    SysElem powerbar_counter[2];            /* 0x2978 [Powerbar] p<n>.counter */
    int level_snd_group[FIGHT_LEVEL_SNDS];  /* 0x2AD0 [Powerbar] level<n>.snd */
    int level_snd_number[FIGHT_LEVEL_SNDS]; /* 0x2AF8 */
    FightFace face;                         /* 0x2B20 */
    FightFace simul_face;                   /* 0x3610 */
    FightFace turns_face;                   /* 0x4100 */
    FightTeammate turns_teammate[2];        /* 0x4BF0 */
    FightName name;                         /* 0x52D0 */
    FightName simul_name;                   /* 0x5850 */
    FightName turns_name;                   /* 0x5DD0 */
    FightTime time;                         /* 0x6350 */
    FightCombo combo[2];                    /* 0x64B4 */
    FightRound round;                       /* 0x688C */
    FightWinIcon winicon;                   /* 0x7578 */
} FightDef;
MUGEN_ASSERT_OFFSET(FightDef, snd, 0x0030);
MUGEN_ASSERT_OFFSET(FightDef, lifebar, 0x0038);
MUGEN_ASSERT_OFFSET(FightDef, simul_lifebar, 0x0718);
MUGEN_ASSERT_OFFSET(FightDef, turns_lifebar, 0x14D8);
MUGEN_ASSERT_OFFSET(FightDef, powerbar, 0x2298);
MUGEN_ASSERT_OFFSET(FightDef, powerbar_counter, 0x2978);
MUGEN_ASSERT_OFFSET(FightDef, level_snd_group, 0x2AD0);
MUGEN_ASSERT_OFFSET(FightDef, face, 0x2B20);
MUGEN_ASSERT_OFFSET(FightDef, simul_face, 0x3610);
MUGEN_ASSERT_OFFSET(FightDef, turns_face, 0x4100);
MUGEN_ASSERT_OFFSET(FightDef, turns_teammate, 0x4BF0);
MUGEN_ASSERT_OFFSET(FightDef, name, 0x52D0);
MUGEN_ASSERT_OFFSET(FightDef, turns_name, 0x5DD0);
MUGEN_ASSERT_OFFSET(FightDef, time, 0x6350);
MUGEN_ASSERT_OFFSET(FightDef, combo, 0x64B4);
MUGEN_ASSERT_OFFSET(FightDef, round, 0x688C);
MUGEN_ASSERT_OFFSET(FightDef, winicon, 0x7578);
MUGEN_ASSERT_SIZE(FightDef, 0x8464);

#endif /* MUGEN_FIGHT_H */
