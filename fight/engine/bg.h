#ifndef MUGEN_BG_H
#define MUGEN_BG_H

/*
 * Backgrounds (bg.c): the "[<name>def]" / "[<name> ...]" / "[<name>Ctrl ...]"
 * groups of stage files ("BG"), of system.def ("TitleBG", "SelectBG", ...)
 * and of storyboard scenes ("BG0", ...), their per-tick update, the camera
 * and the drawing of the elements.
 */

/* BgElem.type ("type" parameter, by its first letter). */
#define BG_TYPE_NORMAL      0       /* "normal" */
#define BG_TYPE_PARALLAX    1       /* "parallax" */
#define BG_TYPE_H           2       /* any type starting with 'h': loaded and updated, never drawn */
#define BG_TYPE_ANIM        3       /* "anim" */
#define BG_TYPE_DUMMY       10      /* "dummy" */

/* BgCtrl.type */
#define BGCTRL_NULL         0
#define BGCTRL_VISIBLE      1
#define BGCTRL_ENABLE       2
#define BGCTRL_POSSET       3
#define BGCTRL_VELSET       4
#define BGCTRL_POSADD       5
#define BGCTRL_VELADD       6
#define BGCTRL_ANIM         8
#define BGCTRL_SINX         11
#define BGCTRL_SINY         12
#define BGCTRL_DUMMY        20
#define BGCTRL_TYPE_LIMIT   100     /* controllers of this type or above never run */

/* Room in BgCtrl.ids: up to 10 ids and the -1 that ends them. */
#define BGCTRL_MAX_IDS      10

/* BgCtrl.x / .y of PosSet, VelSet, PosAdd, VelAdd when "x" / "y" is not given (-FLT_MAX). */
#define BGCTRL_UNSET        (-3.4028234663852886e+38)

/* Bg_Load bit flags of the "tile" parameter passed to BgElem_InitParallax. */
#define BG_TILE_X           1
#define BG_TILE_Y           2

/* One "[BGCtrl ...]" controller (0x54 bytes). */
typedef struct BgCtrl {
    int ids[BGCTRL_MAX_IDS + 1];    /* 0x00 "ctrlid": elements it acts on, ended by -1; -1 first = all */
    int event_id;                   /* 0x2C [BGCtrlDef] "eventID" (read, unused) */
    int type;                       /* 0x30 BGCTRL_* */
    int start;                      /* 0x34 "time" = start, end, looptime */
    int end;                        /* 0x38 start when "time" has one value */
    int looptime;                   /* 0x3C -1: the timer never wraps ([BGCtrlDef] "looptime" by default) */
    int value;                      /* 0x40 Visible, Enable: flag; Anim: action number */
    float x;                        /* 0x44 Pos* / Vel*: "x" (BGCTRL_UNSET if missing); SinX/SinY: amplitude */
    float y;                        /* 0x48 Pos* / Vel*: "y"; SinX/SinY: period */
    float z;                        /* 0x4C SinX/SinY: offset */
    int timer;                      /* 0x50 ticks since the start or the last loop */
} BgCtrl;
MUGEN_ASSERT_OFFSET(BgCtrl, event_id, 0x2C);
MUGEN_ASSERT_OFFSET(BgCtrl, value, 0x40);
MUGEN_ASSERT_OFFSET(BgCtrl, timer, 0x50);
MUGEN_ASSERT_SIZE(BgCtrl, 0x54);

/* One "[BG ...]" element (0x134 bytes), kept in Bg.elems by (0, index). */
typedef struct BgElem {
    int type;                   /* 0x000 BG_TYPE_* */
    float start_x;              /* 0x004 "start" */
    float start_y;              /* 0x008 */
    float scalestart_x;         /* 0x00C "scalestart" */
    float scalestart_y;         /* 0x010 */
    float delta_x;              /* 0x014 "delta", negated */
    float delta_y;              /* 0x018 */
    float velocity_x;           /* 0x01C "velocity" (applied as velocity * Bg.ticks) */
    float velocity_y;           /* 0x020 */
    float scaledelta_x;         /* 0x024 "scaledelta", negated */
    float scaledelta_y;         /* 0x028 */
    float zoomdelta;            /* 0x02C "zoomdelta" */
    int tile_x;                 /* 0x030 "tile": 0 off, 1 infinite, n times (parallax: 0/1) */
    int tile_y;                 /* 0x034 (parallax: 0) */
    float tilespacing_x;        /* 0x038 "tilespacing" */
    float tilespacing_y;        /* 0x03C */
    float width;                /* 0x040 sprite size; parallax: top width */
    float height;               /* 0x044 */
    int window;                 /* 0x048 "window" or "maskwindow" given */
    int window_unshifted;       /* 0x04C "window" given and not yet moved by Bg_ShiftWindows */
    int window_x1;              /* 0x050 */
    int window_x2;              /* 0x054 "window" adds 1 (exclusive) */
    int window_y1;              /* 0x058 */
    int window_y2;              /* 0x05C */
    float windowdelta_x;        /* 0x060 "windowdelta" */
    float windowdelta_y;        /* 0x064 */
    int trans;                  /* 0x068 TRANS_* */
    int alpha_src;              /* 0x06C "alpha" (256 = 1.0) */
    int alpha_dst;              /* 0x070 */
    int mask;                   /* 0x074 "mask" (1 by default with add/sub) */
    int layer;                  /* 0x078 "layerno" */
    float scale_x;              /* 0x07C Bg_SetScale */
    float scale_y;              /* 0x080 */
    float x;                    /* 0x084 position computed by Bg_SetCamera */
    float y;                    /* 0x088 */
    float draw_scale_x;         /* 0x08C scale computed by Bg_SetCamera */
    float draw_scale_y;         /* 0x090 */
    float clip_x1;              /* 0x094 window computed by Bg_SetCamera */
    float clip_y1;              /* 0x098 */
    float clip_x2;              /* 0x09C */
    float clip_y2;              /* 0x0A0 */
    int by_width;               /* 0x0A4 parallax: "paralwidth" or "width" (rows scaled one by one) */
    float yscale_start;         /* 0x0A8 parallax: "yscalestart" / 100 */
    float yscale;               /* 0x0AC parallax: y scale computed by Bg_SetCamera */
    float yscale_delta;         /* 0x0B0 parallax: -"yscaledelta" / 100 */
    float width_top;            /* 0x0B4 parallax: top width */
    float width_bottom;         /* 0x0B8 parallax: bottom width */
    float width_ratio;          /* 0x0BC parallax: bottom / top width */
    int centered;               /* 0x0C0 parallax: "width" or "xscale" (axis in the middle), 0 for "paralwidth" */
    int *row_widths;            /* 0x0C4 parallax, software: width of each row */
    unsigned char **rows;       /* 0x0C8 parallax, software: the pixels of each row */
    int rows_filled;            /* 0x0CC rows holds allocated rows */
    unsigned char **rows2;      /* 0x0D0 freed like rows, never allocated */
    int rows2_filled;           /* 0x0D4 */
    unsigned char **rows3;      /* 0x0D8 freed like rows, never allocated */
    int id;                     /* 0x0DC "id" */
    int enabled;                /* 0x0E0 BGCtrl Enable */
    int visible;                /* 0x0E4 BGCtrl Visible */
    float pos_x;                /* 0x0E8 BGCtrl PosSet / PosAdd offset */
    float pos_y;                /* 0x0EC */
    float vel_x;                /* 0x0F0 BGCtrl VelSet / VelAdd, added to pos every tick */
    float vel_y;                /* 0x0F4 */
    int positionlink;           /* 0x0F8 "positionlink": follow the previous unlinked element */
    float sinx_amp;             /* 0x0FC "sin.x" = amplitude, period, offset */
    float siny_amp;             /* 0x100 "sin.y" */
    float sinx_period;          /* 0x104 */
    float siny_period;          /* 0x108 */
    float sinx_offset;          /* 0x10C */
    float siny_offset;          /* 0x110 */
    float sinx_time;            /* 0x114 ticks of the x wave */
    float siny_time;            /* 0x118 */
    AnimState *anim;            /* 0x11C "actionno" playback (calloc'ed), NULL for sprites */
    int action;                 /* 0x120 "actionno", -1 for sprites */
    int sprite;                 /* 0x124 slot of "spriteno" in Bg.sprites, -1 for animations */
    int pad_128;                /* 0x128 never accessed by the game */
    int pad_12c;                /* 0x12C never accessed by the game */
    int legacy;                 /* 0x130 set by Bg_SetLegacy (stages of older versions) */
} BgElem;
MUGEN_ASSERT_OFFSET(BgElem, tile_x, 0x30);
MUGEN_ASSERT_OFFSET(BgElem, window, 0x48);
MUGEN_ASSERT_OFFSET(BgElem, trans, 0x68);
MUGEN_ASSERT_OFFSET(BgElem, scale_x, 0x7C);
MUGEN_ASSERT_OFFSET(BgElem, by_width, 0xA4);
MUGEN_ASSERT_OFFSET(BgElem, row_widths, 0xC4);
MUGEN_ASSERT_OFFSET(BgElem, id, 0xDC);
MUGEN_ASSERT_OFFSET(BgElem, positionlink, 0xF8);
MUGEN_ASSERT_OFFSET(BgElem, anim, 0x11C);
MUGEN_ASSERT_OFFSET(BgElem, legacy, 0x130);
MUGEN_ASSERT_SIZE(BgElem, 0x134);

/* A loaded background (0x68 bytes, Bg_Create). */
typedef struct Bg {
    float cam_x;                /* 0x00 Bg_SetCamera */
    float cam_y;                /* 0x04 */
    float zoom_center_x;        /* 0x08 */
    float zoom_center_y;        /* 0x0C */
    float zoom;                 /* 0x10 1.0 */
    int unused_14;              /* 0x14 Storyboard_Draw writes its x position here (int), never read */
    int pad_18[5];              /* 0x18 never accessed by the game */
    int ticks;                  /* 0x2C Bg_Tick count, 0 after Bg_Reset */
    Anim *anim;                 /* 0x30 the file's "[Begin Action]" groups, or the caller's */
    int owns_anim;              /* 0x34 */
    SpriteSet *sprites;         /* 0x38 "spr" of [<name>def], or the caller's */
    int owns_sprites;           /* 0x3C */
    KeyedArray *elems;          /* 0x40 BgElem by (0, index) */
    BgCtrl *ctrls;              /* 0x44 */
    int ctrl_count;             /* 0x48 */
    int clear_color[3];         /* 0x4C "bgclearcolor" (r = -1: none; magenta with "debugbg") */
    int clear_cur[3];           /* 0x58 clear_color as of the last Bg_Tick / Bg_Reset (Bg_Draw) */
    int legacy;                 /* 0x64 Bg_SetLegacy */
} Bg;
MUGEN_ASSERT_OFFSET(Bg, ticks, 0x2C);
MUGEN_ASSERT_OFFSET(Bg, elems, 0x40);
MUGEN_ASSERT_OFFSET(Bg, clear_color, 0x4C);
MUGEN_ASSERT_OFFSET(Bg, clear_cur, 0x58);
MUGEN_ASSERT_SIZE(Bg, 0x68);

#endif /* MUGEN_BG_H */
