#ifndef MUGEN_SFF_H
#define MUGEN_SFF_H

/*
 * Sprite files (SFF v1.01 and v2.x, and single PCX images). They are loaded
 * into sprite sets (sprite.h).
 */

/* ---- SFF file format ---- */

#define SFF_VERSION_101     0x01000100u     /* 1.01 */
#define SFF_VERSION_200     0x02000000u     /* 2.00 */
#define SFF_VERSION_201     0x02000100u     /* 2.01 */

/* Size of the file header; offsets in the file count from its start. */
#define SFF_HEADER_SIZE     0x200

/* File header, v1.01 layout. */
typedef struct Sff1Header {
    char signature[12];             /* 0x000 "ElecbyteSpr" */
    unsigned int version;           /* 0x00C */
    int group_count;                /* 0x010 */
    int image_count;                /* 0x014 */
    unsigned int first_offset;      /* 0x018 first subfile */
    unsigned int subheader_size;    /* 0x01C */
    unsigned char palette_type;     /* 0x020 1 = shared palette */
    unsigned char reserved[3];      /* 0x021 */
    char comments[0x1DC];           /* 0x024 */
} Sff1Header;
MUGEN_ASSERT_OFFSET(Sff1Header, palette_type, 0x20);
MUGEN_ASSERT_SIZE(Sff1Header, 0x200);

/* File header, v2.x layout. */
typedef struct Sff2Header {
    char signature[12];             /* 0x000 "ElecbyteSpr" */
    unsigned int version;           /* 0x00C */
    unsigned int reserved1;         /* 0x010 */
    unsigned int reserved2;         /* 0x014 */
    unsigned int compat_version;    /* 0x018 */
    unsigned int reserved3;         /* 0x01C */
    unsigned int reserved4;         /* 0x020 */
    unsigned int sprite_offset;     /* 0x024 sprite node table */
    unsigned int sprite_count;      /* 0x028 */
    unsigned int palette_offset;    /* 0x02C palette node table */
    unsigned int palette_count;     /* 0x030 */
    unsigned int ldata_offset;      /* 0x034 literal data (palettes, sprites) */
    unsigned int ldata_size;        /* 0x038 */
    unsigned int tdata_offset;      /* 0x03C translated data (sprites flagged SFF2_SPRITE_TDATA) */
    unsigned int tdata_size;        /* 0x040 */
    unsigned int reserved5;         /* 0x044 */
    unsigned int reserved6;         /* 0x048 */
    char unused[0x1B4];             /* 0x04C */
} Sff2Header;
MUGEN_ASSERT_OFFSET(Sff2Header, sprite_offset, 0x24);
MUGEN_ASSERT_OFFSET(Sff2Header, tdata_offset, 0x3C);
MUGEN_ASSERT_SIZE(Sff2Header, 0x200);

typedef union SffHeader {
    Sff1Header v1;
    Sff2Header v2;
} SffHeader;
MUGEN_ASSERT_SIZE(SffHeader, 0x200);

/* v1.01 subfile header, before each PCX image. */
typedef struct Sff1SubHeader {
    unsigned int next_offset;       /* 0x00 next subfile */
    int size;                       /* 0x04 PCX data size, 0 for a linked sprite */
    short x;                        /* 0x08 */
    short y;                        /* 0x0A */
    short group;                    /* 0x0C */
    short number;                   /* 0x0E */
    short link;                     /* 0x10 linked sprite index */
    unsigned char same_palette;     /* 0x12 uses the previous sprite's palette */
    char comment[13];               /* 0x13 */
} Sff1SubHeader;
MUGEN_ASSERT_OFFSET(Sff1SubHeader, same_palette, 0x12);
MUGEN_ASSERT_SIZE(Sff1SubHeader, 0x20);

/* Sff2SpriteNode.flags */
#define SFF2_SPRITE_TDATA   0x0001      /* data is in the tdata block */

/* v2 sprite node. */
typedef struct Sff2SpriteNode {
    unsigned short group;           /* 0x00 */
    unsigned short number;          /* 0x02 */
    unsigned short w;               /* 0x04 */
    unsigned short h;               /* 0x06 */
    short x;                        /* 0x08 */
    short y;                        /* 0x0A */
    unsigned short link;            /* 0x0C linked sprite index */
    unsigned char format;           /* 0x0E compression */
    unsigned char depth;            /* 0x0F bits per pixel */
    unsigned int data_offset;       /* 0x10 */
    unsigned int data_size;         /* 0x14 0 for a linked sprite */
    unsigned short palette;         /* 0x18 palette index */
    unsigned short flags;           /* 0x1A SFF2_SPRITE_* */
} Sff2SpriteNode;
MUGEN_ASSERT_OFFSET(Sff2SpriteNode, data_offset, 0x10);
MUGEN_ASSERT_SIZE(Sff2SpriteNode, 0x1C);

/* v2 palette node. */
typedef struct Sff2PaletteNode {
    unsigned short group;           /* 0x00 */
    unsigned short number;          /* 0x02 */
    unsigned short ncolors;         /* 0x04 */
    unsigned short link;            /* 0x06 linked palette index */
    unsigned int data_offset;       /* 0x08 in the ldata block */
    unsigned int data_size;         /* 0x0C 0 for a linked palette */
} Sff2PaletteNode;
MUGEN_ASSERT_SIZE(Sff2PaletteNode, 0x10);

/*
 * Sprite selections (Sff_LoadSelected) are arrays of (group, number) int
 * pairs ended by a -1 group.
 */
#define SFF_SELECT_END      (-1)

#endif /* MUGEN_SFF_H */
