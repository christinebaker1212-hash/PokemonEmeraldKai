#ifndef MUGEN_ANIM_H
#define MUGEN_ANIM_H

/*
 * Animations (AIR files and the "[Begin Action n]" groups of other text
 * files), their playback state, the per-frame sprite draw queue and the
 * one-shot animation sets.
 */

/* ---- sprite draw parameters ----
 * Built by the render helpers at 0x462F70-0x4635C0 (video module). */

/* DrawParams.flags: which fields are set. */
#define DRAW_POS        0x0001  /* x, y */
#define DRAW_FLIP       0x0002  /* flip */
#define DRAW_FLIP2      0x0004  /* flip2 */
#define DRAW_SCALE      0x0008  /* scale_x, scale_y */
#define DRAW_SCALE2     0x0010  /* scale2_x, scale2_y */
#define DRAW_ANGLE      0x0020  /* angle, angle_y, angle_z */
#define DRAW_SHEAR_X    0x0040  /* shear_x */
#define DRAW_SHEAR_Y    0x0080  /* shear_y */
#define DRAW_BLEND      0x0100  /* alpha_src, alpha_dst, blend */
#define DRAW_STRETCH    0x0200  /* top_stretch..bottom_offset */
#define DRAW_UNUSED_400 0x0400  /* only passed on to the children (DrawParams_Combine) */
#define DRAW_UNUSED_1000 0x1000 /* set on every AIR element and BG, only passed on to the children */

/* DrawParams.flip bits ("H" and "V" in AIR element flags). */
#define DRAW_FLIP_H     1
#define DRAW_FLIP_V     2

/* DrawParams.blend */
#define BLEND_NONE      0
#define BLEND_ADD       1
#define BLEND_SUB       2

typedef struct DrawParams {
    float width_ratio;      /* 0x00 1.0; parallax quads: top / bottom width (OpenGL squeezes the texture by it) */
    float width_anchor;     /* 0x04 0.5; x of the sprite (0..1) the width_ratio squeeze keeps in place */
    float x;                /* 0x08 offset */
    float y;                /* 0x0C */
    float scale_x;          /* 0x10 */
    float scale_y;          /* 0x14 */
    unsigned int flip;      /* 0x18 DRAW_FLIP_* */
    unsigned int flip2;     /* 0x1C DRAW_FLIP_* */
    float scale2_x;         /* 0x20 */
    float scale2_y;         /* 0x24 */
    float shear_x;          /* 0x28 x shear (shadow xshear, parallax), added to the parent's */
    float shear_y;          /* 0x2C y shear, added to the parent's */
    float angle;            /* 0x30 degrees */
    float angle_y;          /* 0x34 ? */
    float angle_z;          /* 0x38 ? */
    float top_stretch;      /* 0x3C 1.0; texture stretch at the top of the quad (OpenGL parallax) */
    float bottom_stretch;   /* 0x40 1.0; at the bottom */
    float top_offset;       /* 0x44 texture offset at the top of the quad */
    float bottom_offset;    /* 0x48 at the bottom */
    int masked;             /* 0x4C 1: colour 0 is transparent (BG mask; the software drawers mask flipped / rotated sprites anyway) */
    float alpha_src;        /* 0x50 source alpha, 1.0 = 256 */
    float alpha_dst;        /* 0x54 destination alpha */
    int blend;              /* 0x58 BLEND_* */
    unsigned int flags;     /* 0x5C DRAW_* */
} DrawParams;
MUGEN_ASSERT_OFFSET(DrawParams, flip, 0x18);
MUGEN_ASSERT_OFFSET(DrawParams, angle, 0x30);
MUGEN_ASSERT_OFFSET(DrawParams, alpha_src, 0x50);
MUGEN_ASSERT_OFFSET(DrawParams, flags, 0x5C);
MUGEN_ASSERT_SIZE(DrawParams, 0x60);

/* ---- animations ---- */

/* AnimElem.flags */
#define ANIM_ELEM_CLSN1_DEFAULT 0x04    /* clsn1 came from "Clsn1Default" */
#define ANIM_ELEM_CLSN2_DEFAULT 0x08    /* clsn2 came from "Clsn2Default" */

/* One frame of an action. */
typedef struct AnimElem {
    int start;              /* 0x00 action time at which the element starts */
    int time;               /* 0x04 duration, -1 = forever */
    int group;              /* 0x08 sprite */
    int number;             /* 0x0C */
    int interp_offset;      /* 0x10 "Interpolate Offset" towards this element */
    int interp_blend;       /* 0x14 "Interpolate Blend" */
    int interp_scale;       /* 0x18 "Interpolate Scale" */
    int interp_angle;       /* 0x1C "Interpolate Angle" */
    DrawParams dp;          /* 0x20 offset, flip, trans, scale, angle */
    unsigned int flags;     /* 0x80 ANIM_ELEM_* */
    Clsn *clsn1;            /* 0x84 */
    Clsn *clsn2;            /* 0x88 */
} AnimElem;
MUGEN_ASSERT_OFFSET(AnimElem, dp, 0x20);
MUGEN_ASSERT_OFFSET(AnimElem, flags, 0x80);
MUGEN_ASSERT_SIZE(AnimElem, 0x8C);

/* "[Begin Action n]" */
typedef struct AnimAction {
    KeyedArray *elems;      /* 0x00 AnimElem by (0, index) */
    int elem_count;         /* 0x04 */
    int total_time;         /* 0x08 sum of the element times, -1 if one lasts forever */
    int loop_time;          /* 0x0C start of the "Loopstart" element */
    int loop_elem;          /* 0x10 index of the "Loopstart" element */
} AnimAction;
MUGEN_ASSERT_SIZE(AnimAction, 0x14);

typedef struct Anim {
    KeyedArray *actions;    /* 0x00 AnimAction by (0, action number) */
    int action_count;       /* 0x04 "[Begin Action" groups in the file */
} Anim;
MUGEN_ASSERT_SIZE(Anim, 0x08);

struct SpriteSet;           /* loaded SFF; its first field is the sprite KeyedArray */
struct SpriteQueue;

/* Playback state of an animation. */
typedef struct AnimState {
    Anim *anim;                 /* 0x00 */
    struct SpriteSet *sprites;  /* 0x04 */
    int owns_sprites;           /* 0x08 the state owns sprites (freed with it) */
    int action;                 /* 0x0C slot of the action in anim->actions */
    AnimElem *elem;             /* 0x10 current element */
    AnimElem *next;             /* 0x14 element after it (the loop element after the last) */
    int elem_index;             /* 0x18 */
    int ticks;                  /* 0x1C ticks since the action was set */
    int time;                   /* 0x20 action time (goes back to loop_time) */
    int sprite;                 /* 0x24 slot of the sprite in the sprite set, -1 if missing */
    DrawParams dp;              /* 0x28 element parameters, interpolated */
    struct SpriteQueue *queue;  /* 0x88 queue the sprites go to, NULL to draw at once */
    int layer;                  /* 0x8C sort key in the queue */
} AnimState;
MUGEN_ASSERT_OFFSET(AnimState, sprite, 0x24);
MUGEN_ASSERT_OFFSET(AnimState, dp, 0x28);
MUGEN_ASSERT_OFFSET(AnimState, queue, 0x88);
MUGEN_ASSERT_SIZE(AnimState, 0x90);

/* ---- sprite draw queue ---- */

struct PalFx;               /* PalFX parameters (0x28 bytes, palette module) */
struct PalRemap;            /* palette remapping (0x84 bytes, palette module) */

/* A sprite waiting to be drawn. */
typedef struct SpriteQueueItem {
    int layer;                  /* 0x000 sort key */
    struct SpriteSet *sprites;  /* 0x004 */
    int sprite;                 /* 0x008 */
    struct PalFx *palfx;        /* 0x00C g_palFxCur, or palfx_copy */
    int palfx_copy[10];         /* 0x010 copy of the static PalFx */
    struct PalRemap *remap;     /* 0x038 g_palRemapCur, or remap_copy */
    int remap_copy[33];         /* 0x03C copy of the static PalRemap */
    DrawParams dp;              /* 0x0C0 */
} SpriteQueueItem;
MUGEN_ASSERT_OFFSET(SpriteQueueItem, remap, 0x38);
MUGEN_ASSERT_OFFSET(SpriteQueueItem, dp, 0xC0);
MUGEN_ASSERT_SIZE(SpriteQueueItem, 0x120);

typedef struct SpriteQueue {
    KeyedArray *items;      /* 0x00 SpriteQueueItem by (0, layer) */
    int *order;             /* 0x04 item slots sorted by layer */
} SpriteQueue;
MUGEN_ASSERT_SIZE(SpriteQueue, 0x08);

/* ---- animation sets: one-shot animations removed when they end ---- */

typedef struct AnimInstance {
    int x;                  /* 0x00 */
    int y;                  /* 0x04 */
    unsigned int flip;      /* 0x08 DRAW_FLIP_* */
    float scale_x;          /* 0x0C */
    float scale_y;          /* 0x10 */
    AnimState state;        /* 0x14 */
} AnimInstance;
MUGEN_ASSERT_OFFSET(AnimInstance, state, 0x14);
MUGEN_ASSERT_SIZE(AnimInstance, 0xA4);

typedef struct AnimSet {
    KeyedArray *list;       /* 0x00 AnimInstance by (flags, 0) */
} AnimSet;
MUGEN_ASSERT_SIZE(AnimSet, 0x04);

/* Trans_Parse results ("trans" parameter of controllers and backgrounds). */
#define TRANS_DEFAULT   (-1)    /* empty or "default" */
#define TRANS_INVALID   (-2)
#define TRANS_NONE      0
#define TRANS_SUB       2
#define TRANS_ADD       3       /* add, add1, addalpha: alpha given separately */

/* Entries in g_sinTable: 8 per degree. */
#define SIN_TABLE_SIZE  3600

#endif /* MUGEN_ANIM_H */
