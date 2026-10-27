#ifndef MUGEN_SYSELEM_H
#define MUGEN_SYSELEM_H

/* SysElem.type: which of spr / anim / font was given. */
enum {
    SYSELEM_NONE = 0,
    SYSELEM_ANIM = 1,       /* "x.anim" */
    SYSELEM_SPRITE = 2,     /* "x.spr" */
    SYSELEM_FONT = 3        /* "x.font" + "x.text" */
};

/* Font element alignment (SysElem.font_align). */
#define SYSELEM_ALIGN_LEFT  1
#define SYSELEM_ALIGN_RIGHT (-1)

/*
 * A drawable element of system.def, fight.def and the other motif files:
 * "<prefix>.spr", ".anim" or ".font" (with ".text"), plus ".offset",
 * ".facing", ".scale" and so on. Read by SysElem_Read (0x4117D0), which only
 * reads the other parameters when one of spr / anim / font is present, and
 * drawn through the system element draw list (SysElem_Add).
 */
typedef struct SysElem {
    int type;               /* 0x00 SYSELEM_* */
    int group;              /* 0x04 spr: group; anim: action; font: font index */
    int number;             /* 0x08 spr: number; font: bank */
    int font_align;         /* 0x0C font: SYSELEM_ALIGN_*, 0 centered */
    int font_color[3];      /* 0x10 font: r, g, b (0x100: the font's own) */
    int pad_1c;             /* 0x1C never accessed by the game */
    int displaytime;        /* 0x20 -2: the length of the animation */
    float x;                /* 0x24 .offset */
    float y;                /* 0x28 */
    int layerno;            /* 0x2C the select screen only draws layer 0 elements */
    int owner_id;           /* 0x30 owner id given to the draw list item (SysDrawItem.owner_id) */
    int id;                 /* 0x34 serial number (g_sysElemSerial) */
    int facing;             /* 0x38 */
    int vfacing;            /* 0x3C */
    float scale_x;          /* 0x40 */
    float scale_y;          /* 0x44 */
    float angle;            /* 0x48 */
    float yangle;           /* 0x4C */
    float xangle;           /* 0x50 */
    char text[0x40];        /* 0x54 font elements */
    intptr_t text_ptr;      /* 0x94 a const char * drawn instead of text when set, as an int (Moemon Fight: pointer-sized) */
    int window_x[2];        /* 0x98 .window = x1, y1, x2, y2; -1: none */
    int window_y[2];        /* 0xA0 */
    int textwrap;           /* 0xA8 0 none, 1 'c', 2 otherwise */
} SysElem;
MUGEN_ASSERT_OFFSET(SysElem, font_color, 0x10);
MUGEN_ASSERT_OFFSET(SysElem, displaytime, 0x20);
MUGEN_ASSERT_OFFSET(SysElem, x, 0x24);
MUGEN_ASSERT_OFFSET(SysElem, layerno, 0x2C);
MUGEN_ASSERT_OFFSET(SysElem, id, 0x34);
MUGEN_ASSERT_OFFSET(SysElem, scale_x, 0x40);
MUGEN_ASSERT_OFFSET(SysElem, text, 0x54);
MUGEN_ASSERT_OFFSET(SysElem, window_x, 0x98);
MUGEN_ASSERT_SIZE(SysElem, 0xAC);

#endif /* MUGEN_SYSELEM_H */
