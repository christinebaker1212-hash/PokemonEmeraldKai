#ifndef MUGEN_MOTIF_H
#define MUGEN_MOTIF_H

/*
 * The motif's system.def (motif.c): layouts of the screens read by
 * System_LoadMotif into GameVars that have no module of their own yet
 * (versus, victory, continue, game over, win and survival results screens,
 * endings). [Title Info], [Option Info] and [Select Info] are in title.h and
 * select.h.
 */

/* [End Credits], [Default Ending], [Game Over Screen]: enabled + storyboard (0x204 bytes). */
typedef struct MotifStoryboard {
    int enabled;                    /* 0x000 enabled */
    char storyboard[0x200];         /* 0x004 storyboard */
} MotifStoryboard;
MUGEN_ASSERT_SIZE(MotifStoryboard, 0x204);

/* [Win Screen] (0xBC bytes at GameVars + 0x3F8C). */
typedef struct WinScreenInfo {
    int enabled;                    /* 0x00 enabled */
    SysElem wintext;                /* 0x04 wintext.* */
    int pose_time;                  /* 0xB0 pose.time */
    int fadein_time;                /* 0xB4 fadein.time */
    int fadeout_time;               /* 0xB8 fadeout.time */
} WinScreenInfo;
MUGEN_ASSERT_OFFSET(WinScreenInfo, pose_time, 0xB0);
MUGEN_ASSERT_SIZE(WinScreenInfo, 0xBC);

/* [Survival Results Screen] (0xC0 bytes at GameVars + 0x4048). */
typedef struct SurvivalResultsInfo {
    int enabled;                    /* 0x00 enabled */
    SysElem winstext;               /* 0x04 winstext.* */
    int show_time;                  /* 0xB0 show.time */
    int fadein_time;                /* 0xB4 fadein.time */
    int fadeout_time;               /* 0xB8 fadeout.time */
    int roundstowin;                /* 0xBC roundstowin */
} SurvivalResultsInfo;
MUGEN_ASSERT_OFFSET(SurvivalResultsInfo, show_time, 0xB0);
MUGEN_ASSERT_SIZE(SurvivalResultsInfo, 0xC0);

/* [Continue Screen] (0x568 bytes at GameVars + 0x4108). */
typedef struct ContinueInfo {
    int enabled;                    /* 0x000 enabled */
    char storyboard[0x200];         /* 0x004 storyboard */
    int pos[2];                     /* 0x204 pos (default 159, 40) */
    SysElem continue_text;          /* 0x20C continue.* */
    SysElem yes;                    /* 0x2B8 yes.* */
    SysElem yes_active;             /* 0x364 yes.active.* */
    SysElem no;                     /* 0x410 no.* */
    SysElem no_active;              /* 0x4BC no.active.* */
} ContinueInfo;
MUGEN_ASSERT_OFFSET(ContinueInfo, pos, 0x204);
MUGEN_ASSERT_OFFSET(ContinueInfo, continue_text, 0x20C);
MUGEN_ASSERT_OFFSET(ContinueInfo, no_active, 0x4BC);
MUGEN_ASSERT_SIZE(ContinueInfo, 0x568);

/* One side of the versus screen: "p<n>.name.*" and the face "p<n>.*" (0xD8 bytes). */
typedef struct VsSideLayout {
    FontSpec name_font;             /* 0x00 p<n>.name.font */
    int name_pos[2];                /* 0x1C p<n>.name.offset, then p<n>.name.pos */
    int name_spacing[2];            /* 0x24 p<n>.name.spacing */
    SysElem face;                   /* 0x2C p<n>.*; default sprite 9000,1 facing p1 1, p2 -1, read with p<n>.scale, p<n>.pos */
} VsSideLayout;
MUGEN_ASSERT_OFFSET(VsSideLayout, face, 0x2C);
MUGEN_ASSERT_SIZE(VsSideLayout, 0xD8);

/* [VS Screen] (0x268 bytes at GameVars + 0x4874). */
typedef struct VsScreenInfo {
    int time;                       /* 0x000 time (default 150) */
    int fadein_time;                /* 0x004 fadein.time */
    int fadeout_time;               /* 0x008 fadeout.time */
    VsSideLayout sides[2];          /* 0x00C */
    SysElem match;                  /* 0x1BC match.* */
} VsScreenInfo;
MUGEN_ASSERT_OFFSET(VsScreenInfo, sides, 0x00C);
MUGEN_ASSERT_OFFSET(VsScreenInfo, match, 0x1BC);
MUGEN_ASSERT_SIZE(VsScreenInfo, 0x268);

/* [Victory Screen] (0x194 bytes at GameVars + 0x4ADC). */
typedef struct VictoryInfo {
    int enabled;                    /* 0x000 enabled */
    int time;                       /* 0x004 time (default 150) */
    int fadein_time;                /* 0x008 fadein.time */
    int fadeout_time;               /* 0x00C fadeout.time */
    FontSpec name_font;             /* 0x010 p1.name.font */
    int name_offset[2];             /* 0x02C p1.name.offset */
    int name_spacing[2];            /* 0x034 p1.name.spacing */
    SysElem face;                   /* 0x03C p1.* */
    SysElem winquote;               /* 0x0E8 winquote.* */
} VictoryInfo;
MUGEN_ASSERT_OFFSET(VictoryInfo, name_font, 0x010);
MUGEN_ASSERT_OFFSET(VictoryInfo, face, 0x03C);
MUGEN_ASSERT_OFFSET(VictoryInfo, winquote, 0x0E8);
MUGEN_ASSERT_SIZE(VictoryInfo, 0x194);

#endif /* MUGEN_MOTIF_H */
