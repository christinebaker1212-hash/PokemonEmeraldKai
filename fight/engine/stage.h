#ifndef MUGEN_STAGE_H
#define MUGEN_STAGE_H

/*
 * The loaded stage (GameVars.stage, 0x1AE0). Only the fields known so far
 * are named; the stage module itself is not decompiled yet.
 */
typedef struct Stage {
    int loaded;                 /* 0x000 a stage is loaded */
    char name[0x30];            /* 0x004 [Info] name ("Undefined" if missing) */
    char displayname[0x30];     /* 0x034 [Info] displayname (default: name) */
    char author[0x30];          /* 0x064 [Info] author */
    char path[0x200];           /* 0x094 the .def file the stage was loaded from (Stage_Load) */
    int pad_294;                /* 0x294 */
    double unit;                /* 0x298 coordinate unit of the stage ([StageInfo] localcoord width) */
    double height;              /* 0x2A0 height of the stage in stage units (localcoord), cropped by Camera_UpdateCrop */
    double half_width;          /* 0x2A8 (localcoord width - 1) / 2 (int division) */
    double half_height;         /* 0x2B0 (localcoord height - 1) / 2 */
    unsigned int versiondate;   /* 0x2B8 [Info] versiondate (Version_ParseDate) */
    unsigned int mugenversion;  /* 0x2BC [Info] mugenversion (Version_Parse) */
    int resetbg;                /* 0x2C0 [StageInfo] resetbg, default 1 */
    int camera_startx;          /* 0x2C4 [Camera] startx */
    int camera_starty;          /* 0x2C8 [Camera] starty */
    int boundleft;              /* 0x2CC [Camera] boundleft (also the left stage bound of projectiles, times the stage unit) */
    int boundright;             /* 0x2D0 [Camera] boundright */
    int boundhigh;              /* 0x2D4 [Camera] boundhigh, default -25 */
    int boundlow;               /* 0x2D8 [Camera] boundlow */
    int cuthigh;                /* 0x2DC [Camera] cuthigh */
    int cutlow;                 /* 0x2E0 [Camera] cutlow */
    int overdrawhigh;           /* 0x2E4 [Camera] overdrawhigh */
    int overdrawlow;            /* 0x2E8 [Camera] overdrawlow */
    int overdrawleft;           /* 0x2EC [Camera] overdrawleft */
    int overdrawright;          /* 0x2F0 [Camera] overdrawright */
    float verticalfollow;       /* 0x2F4 [Camera] verticalfollow (negative: zoom to keep the players on the screen) */
    int tension;                /* 0x2F8 [Camera] tension, default 50 */
    int tensionhigh;            /* 0x2FC [Camera] tensionhigh */
    int tensionlow;             /* 0x300 [Camera] tensionlow */
    int floortension;           /* 0x304 [Camera] floortension */
    float zoomout;              /* 0x308 [Camera] zoomout, 1.0 (the projectile stage bounds widen by half the game width over this) */
    float zoomin;               /* 0x30C [Camera] zoomin, 1.0 */
    float startzoom;            /* 0x310 [Camera] startzoom, 1.0 */
    int zoffset;                /* 0x314 [StageInfo] zoffset, default 190 */
    int zoffsetlink;            /* 0x318 [StageInfo] zoffsetlink, default -1 */
    int startx[4];              /* 0x31C [PlayerInfo] p1startx..p4startx (p3, p4 default to p1, p2 -/+ 25) */
    int starty[4];              /* 0x32C [PlayerInfo] p1starty..p4starty */
    int startz[5];              /* 0x33C [PlayerInfo] p1startz..p4startz; [4] is p1facing. The pos z trigger of
                                   pre-1.0 characters reads [player number - 1] (player 5 gets p1facing) */
    int facing[3];              /* 0x350 [PlayerInfo] p2facing..p4facing (p1facing is startz[4]) */
    int leftbound;              /* 0x35C left bound of the players (Player_ClampToStage), [PlayerInfo] leftbound */
    int rightbound;             /* 0x360 right bound, [PlayerInfo] rightbound */
    int autoturn;               /* 0x364 ChangeState turns a standing / crouching player with control to face the enemy
                                   ([StageInfo] autoturn) */
    float xscale;               /* 0x368 [StageInfo] xscale (0.5 with hires) */
    float yscale;               /* 0x36C [StageInfo] yscale */
    int hires;                  /* 0x370 [StageInfo] hires */
    int screenright;            /* 0x374 distance of the players from the right screen edge (ScreenBound), [Bound] screenright */
    int screenleft;             /* 0x378 distance from the left screen edge, [Bound] screenleft */
    int shadow[6];              /* 0x37C shadow parameters (Shadow_Draw): [0] [Shadow] intensity (128), [1] color (mapped),
                                   [2] yscale (a float, 0.4), [3] xshear (a float), [4..5] fade.range */
    int reflection_intensity;   /* 0x394 [Reflection] intensity (Shadow_Draw) */
    struct Bg *bg;              /* 0x398 the stage's "BG" backgrounds (Bg_Load, freed by Stage_Unload) */
    int pad_39C;                /* 0x39C */
    BgmInfo bgm;                /* 0x3A0 [Music] bgmusic, bgmvolume, bgmloop, bgmloopstart, bgmloopend */
} Stage;
MUGEN_ASSERT_OFFSET(Stage, name, 0x004);
MUGEN_ASSERT_OFFSET(Stage, displayname, 0x034);
MUGEN_ASSERT_OFFSET(Stage, author, 0x064);
MUGEN_ASSERT_OFFSET(Stage, unit, 0x298);
MUGEN_ASSERT_OFFSET(Stage, boundleft, 0x2CC);
MUGEN_ASSERT_OFFSET(Stage, zoomout, 0x308);
MUGEN_ASSERT_OFFSET(Stage, height, 0x2A0);
MUGEN_ASSERT_OFFSET(Stage, camera_startx, 0x2C4);
MUGEN_ASSERT_OFFSET(Stage, boundhigh, 0x2D4);
MUGEN_ASSERT_OFFSET(Stage, verticalfollow, 0x2F4);
MUGEN_ASSERT_OFFSET(Stage, floortension, 0x304);
MUGEN_ASSERT_OFFSET(Stage, startzoom, 0x310);
MUGEN_ASSERT_OFFSET(Stage, startz, 0x33C);
MUGEN_ASSERT_OFFSET(Stage, autoturn, 0x364);
MUGEN_ASSERT_OFFSET(Stage, bg, 0x398);
MUGEN_ASSERT_OFFSET(Stage, path, 0x094);
MUGEN_ASSERT_OFFSET(Stage, versiondate, 0x2B8);
MUGEN_ASSERT_OFFSET(Stage, resetbg, 0x2C0);
MUGEN_ASSERT_OFFSET(Stage, zoffset, 0x314);
MUGEN_ASSERT_OFFSET(Stage, startx, 0x31C);
MUGEN_ASSERT_OFFSET(Stage, starty, 0x32C);
MUGEN_ASSERT_OFFSET(Stage, facing, 0x350);
MUGEN_ASSERT_OFFSET(Stage, xscale, 0x368);
MUGEN_ASSERT_OFFSET(Stage, hires, 0x370);
MUGEN_ASSERT_OFFSET(Stage, bgm, 0x3A0);
MUGEN_ASSERT_SIZE(Stage, 0x5C0);

#endif /* MUGEN_STAGE_H */
