#ifndef MUGEN_VIDEO_H
#define MUGEN_VIDEO_H

/*
 * SDL video mode and the window (video.c), and the coordinate values that the
 * small helpers at the start of that module work on.
 */

/* Coord.type of the values made by Coord_Make. */
#define COORD_TYPE_2D 2

/*
 * A position (or size) in a coordinate space whose unit is `unit` (e.g. the
 * localcoord scale of a character). Passed and returned by value.
 */
typedef struct Coord {
    double x;               /* 0x00 */
    double y;               /* 0x08 */
    double z;               /* 0x10 */
    double unit;            /* 0x18 Coord_Rescale scales x, y, z by old unit / new unit */
    int type;               /* 0x20 COORD_TYPE_* */
    int pad_24;             /* 0x24 copied along, never set */
} Coord;
MUGEN_ASSERT_OFFSET(Coord, unit, 0x18);
MUGEN_ASSERT_SIZE(Coord, 0x28);

/* SDL video mode flags that SDL.h only defines internally. */
#define VIDEO_SDL_HWACCEL 0x00000100

#endif /* MUGEN_VIDEO_H */
