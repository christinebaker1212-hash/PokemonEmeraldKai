#ifndef GUARD_GBA_DEFINES_H
#define GUARD_GBA_DEFINES_H

#include <stddef.h>

#define TRUE  1
#define FALSE 0

#ifdef PORTABLE
#define IWRAM_DATA
#define EWRAM_DATA
#define COMMON_DATA
#else
#define IWRAM_DATA __attribute__((section("iwram_data")))
#define EWRAM_DATA __attribute__((section("ewram_data")))
#define COMMON_DATA __attribute__((section("common_data")))
#endif
#define UNUSED __attribute__((unused))

#if MODERN
#define NOINLINE __attribute__((noinline))
#else
#define NOINLINE
#endif

#define ALIGNED(n) __attribute__((aligned(n)))

#define BG_PLTT_SIZE  0x200
#define OBJ_PLTT      (PLTT + BG_PLTT_SIZE)
#define OBJ_PLTT_SIZE 0x200   // the GBA's 16 sprite palettes

// Moemon PC: graphics memory past the GBA's limits. Sprite palettes 16-63 follow
// the GBA's 16 in PLTT and the palette buffers (so OBJ_PLTT_ID(n) just works),
// sprite tiles go up to 4096 (128 KB), OAM has 256 entries, 128 sprites.
#ifdef PORTABLE
#define OBJ_PAL_SLOTS       64
#define OBJ_TILE_COUNT_EXT  4096
#define OAM_ENTRY_COUNT     256
#else
#define OBJ_PAL_SLOTS       16
#define OBJ_TILE_COUNT_EXT  1024
#define OAM_ENTRY_COUNT     128
#endif
#define OBJ_PLTT_EXT_SIZE   ((OBJ_PAL_SLOTS - 16) * 0x20)   // palettes 16+ (PC only)
#define OAM_MANUAL_BASE     (OAM_ENTRY_COUNT - 64)          // OAM entries code fills by hand (digits, confetti...)
#define PLTT_SIZE     (BG_PLTT_SIZE + OBJ_PLTT_SIZE + OBJ_PLTT_EXT_SIZE)

#ifndef PORTABLE
#define SOUND_INFO_PTR (*(struct SoundInfo **)0x3007FF0)
#define INTR_CHECK     (*(u16 *)0x3007FF8)
#define INTR_VECTOR    (*(void **)0x3007FFC)

#define EWRAM_START 0x02000000
#define EWRAM_END   (EWRAM_START + 0x40000)
#define IWRAM_START 0x03000000
#define IWRAM_END   (IWRAM_START + 0x8000)

#define PLTT      0x5000000
#else
extern struct SoundInfo * SOUND_INFO_PTR;
extern unsigned short INTR_CHECK;
extern void * INTR_VECTOR;

extern unsigned char PLTT[PLTT_SIZE] __attribute__ ((aligned (4)));
#endif

#define BG_PLTT       PLTT

#define VRAM_SIZE (0x10000 + OBJ_TILE_COUNT_EXT * 0x20)
#ifndef PORTABLE
#define VRAM      0x6000000
#else
extern unsigned char VRAM_[VRAM_SIZE] __attribute__ ((aligned (4)));
#define VRAM (uintptr_t)VRAM_
#endif //PORTABLE

#define BG_VRAM           VRAM
#define BG_VRAM_SIZE      0x10000
#define BG_CHAR_SIZE      0x4000
#define BG_SCREEN_SIZE    0x800
#define BG_CHAR_ADDR(n)   (BG_VRAM + (BG_CHAR_SIZE * (n)))
#define BG_SCREEN_ADDR(n) (BG_VRAM + (BG_SCREEN_SIZE * (n)))

#define BG_TILE_H_FLIP(n) (0x400 + (n))
#define BG_TILE_V_FLIP(n) (0x800 + (n))

#define NUM_BACKGROUNDS 4

// text-mode BG
#define OBJ_VRAM0      (VRAM + 0x10000)
#define OBJ_VRAM0_SIZE (OBJ_TILE_COUNT_EXT * 0x20)

// bitmap-mode BG
#define OBJ_VRAM1      (VRAM + 0x14000)
#define OBJ_VRAM1_SIZE 0x4000

#ifdef PORTABLE
#define OAM_SIZE (OAM_ENTRY_COUNT * 12)   // struct OamData is 12 bytes on PC (wider tile / palette numbers)
#else
#define OAM_SIZE 0x400
#endif
#ifndef PORTABLE
#define OAM      0x7000000
#else
extern unsigned char OAM[OAM_SIZE] __attribute__ ((aligned (4)));
#endif

#define ROM_HEADER_SIZE   0xC0

// Dimensions of a tile in pixels
#define TILE_WIDTH  8
#define TILE_HEIGHT 8

// Dimensions of the GBA screen in pixels
#define DISPLAY_WIDTH  240
#define DISPLAY_HEIGHT 160

// Dimensions of the GBA screen in tiles
#define DISPLAY_TILE_WIDTH  (DISPLAY_WIDTH / TILE_WIDTH)
#define DISPLAY_TILE_HEIGHT (DISPLAY_HEIGHT / TILE_HEIGHT)

// Size of different tile formats in bytes
#define TILE_SIZE(bpp) ((bpp) * TILE_WIDTH * TILE_HEIGHT / 8)
#define TILE_SIZE_1BPP TILE_SIZE(1) // 8
#define TILE_SIZE_4BPP TILE_SIZE(4) // 32
#define TILE_SIZE_8BPP TILE_SIZE(8) // 64

#define TILE_OFFSET_4BPP(n) ((n) * TILE_SIZE_4BPP)
#define TILE_OFFSET_8BPP(n) ((n) * TILE_SIZE_8BPP)

#define TOTAL_OBJ_TILE_COUNT OBJ_TILE_COUNT_EXT

#define PLTT_SIZEOF(n) ((n) * sizeof(u16))
#define PLTT_SIZE_4BPP PLTT_SIZEOF(16)
#define PLTT_SIZE_8BPP PLTT_SIZEOF(256)

#define PLTT_OFFSET_4BPP(n) ((n) * PLTT_SIZE_4BPP)

#endif // GUARD_GBA_DEFINES_H
