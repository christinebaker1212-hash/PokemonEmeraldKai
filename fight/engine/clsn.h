#ifndef MUGEN_CLSN_H
#define MUGEN_CLSN_H

/*
 * Collision boxes ("Clsn1"/"Clsn2" lists of the AIR elements), module
 * 0x4795C0-0x479C9F.
 */

/* One box, corners sorted (x1 <= x2, y1 <= y2), relative to the player axis. */
typedef struct ClsnBox {
    int hidden;             /* 0x00 always 0: the debug display skips boxes where it is set */
    int x1;                 /* 0x04 */
    int y1;                 /* 0x08 */
    int x2;                 /* 0x0C */
    int y2;                 /* 0x10 */
} ClsnBox;
MUGEN_ASSERT_SIZE(ClsnBox, 0x14);

/* A box list, shared by reference between elements ("Clsn1Default"). */
typedef struct Clsn {
    int valid;              /* 0x00 1; the collision tests skip a list where it is 0 */
    int capacity;           /* 0x04 box count from the "Clsn1: n" line; the tests check this many */
    ClsnBox *boxes;         /* 0x08 */
    int count;              /* 0x0C boxes added so far */
    int refs;               /* 0x10 freed when it drops to 0 */
} Clsn;
MUGEN_ASSERT_SIZE(Clsn, 0x14);

#endif /* MUGEN_CLSN_H */
