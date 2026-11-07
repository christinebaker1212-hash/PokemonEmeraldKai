// Moemon Kai: Colosseum's story flags in ORRE, kept as Colosseum keeps them (its GS flag module: the decomp's
// gs_range_8018FE30.c, gs_flag_exact_801908D4.c). A flag is a value of 1-32 bits in one of three levels' buffers,
// defined by common_rel's table (.data 0x111E04, the count at 0x14A6C0; 8 bytes each: type << 6 | width, the story
// jump's value index, an item swap, an event, the bit position (laid out at load as GSflagInitBitPos does), the
// next flag in the story's order). The table is read from ORRE's data pack (common/common_rel.rel); the values go in
// "<save>.orre" with the Shadow Pokémon (src/orre/orre_shadow.c): saves never change layout. Flag numbers past the
// table (ORRE_KAI_FLAG_FIRST on) are Kai's own (the SNAG MACHINE...), kept apart.
// The decomp's names of the setters are swapped: std078 ("flagSetFalse") sets 1 and std079 ("flagSetTrue") 0.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "orre.h"
#include "orre_data.h"

#define FLAG_TABLE   0x111E04   // common_rel .data
#define FLAG_COUNT   0x14A6C0   // ... {u32 count, s16 head}
#define RO_SWAPS     0x00       // common_rel .rodata: {u16 add, u16 remove} items (lbl_80478EF4)
#define RO_VALUES    0x08       // ... the story jump's values (lbl_80478ED4)
#define RO_VALUE_END 0x24
#define MAX_FLAGS    ORRE_KAI_FLAG_FIRST
#define KAI_FLAGS    (0x1000 - ORRE_KAI_FLAG_FIRST)

struct FlagDef
{
    u8 level, width, jump, swap, event;
    u16 bit;
    s16 next;
};

static struct FlagDef sDefs[MAX_FLAGS];
static u32 sJumpValues[(RO_VALUE_END - RO_VALUES) / 4];
static u16 sCount;
static s16 sHead;
static bool8 sTried, sOk;

// The values: Colosseum's three levels (the sidecar writes them as they are) and Kai's own flags
static struct
{
    u32 level[4][ORRE_FLAG_LEVEL_WORDS];   // [0] unused (levels 1-3)
    u8 kai[KAI_FLAGS / 8];
} sState;

// A format 1 sidecar's flags (one bit a number) wait here until the table is read (Colosseum's widths and levels)
static u16 sOld[64];
static u8 sOldCount;

static u32 Be32(const u8 *p) { return (u32)p[0] << 24 | p[1] << 16 | p[2] << 8 | p[3]; }
static u16 Be16(const u8 *p) { return p[0] << 8 | p[1]; }

static void ApplyOld(void);

static bool8 Parse(const u8 *rel, u32 size, u32 *next);

static bool8 Load(void)
{
    u8 *rel;
    u32 size, next[4] = {0, 0, 0, 0};
    if (sTried)
        return sOk;
    sTried = TRUE;
    if ((rel = OrreData_Read("common/common_rel.rel", &size)) == NULL)
        return FALSE;
    sOk = Parse(rel, size, next);
    free(rel);
    if (sOk)
    {
        printf("[orre] story flags: %u (level bits %u %u %u)\n", sCount, next[1], next[2], next[3]);
        ApplyOld();
    }
    return sOk;
}

static bool8 Parse(const u8 *rel, u32 size, u32 *next)
{
    u32 ns, st, dataOff, dataSize, roOff, roSize, i;
    if (size < 0x40)
        return FALSE;
    ns = Be32(rel + 0x0C);
    st = Be32(rel + 0x10);
    if (ns < 5 || st + 8 * 5 > size)
        return FALSE;
    roOff = Be32(rel + st + 8 * 3) & ~3u;
    roSize = Be32(rel + st + 8 * 3 + 4);
    dataOff = Be32(rel + st + 8 * 4) & ~3u;
    dataSize = Be32(rel + st + 8 * 4 + 4);
    if (dataOff + dataSize > size || roOff + roSize > size || FLAG_COUNT + 6 > dataSize || RO_VALUE_END > roSize)
        return FALSE;
    sCount = Be32(rel + dataOff + FLAG_COUNT);
    sHead = (s16)Be16(rel + dataOff + FLAG_COUNT + 4);
    if (sCount > MAX_FLAGS || FLAG_TABLE + 8 * sCount > dataSize)
    {
        printf("[orre] story flags: %u, more than room for\n", sCount);
        return FALSE;
    }
    for (i = 0; i < sCount; i++)
    {
        const u8 *d = rel + dataOff + FLAG_TABLE + 8 * i;
        struct FlagDef *f = &sDefs[i];
        f->level = d[0] >> 6;
        f->width = d[0] & 0x3F;
        if (f->width > 32)
            f->width = 32;
        else if (f->width == 0)
            f->width = 1;
        f->jump = d[1];
        f->swap = d[2];
        f->event = d[3];
        f->next = (s16)Be16(d + 6);
        f->bit = next[f->level];   // (level 0: none: such a flag holds nothing)
        if (f->level)
            next[f->level] += f->width;
        if (next[f->level] > ORRE_FLAG_LEVEL_WORDS * 32)
        {
            printf("[orre] story flags: level %u needs %u bits\n", f->level, next[f->level]);
            return FALSE;
        }
    }
    for (i = 0; i < ARRAY_COUNT(sJumpValues); i++)
        sJumpValues[i] = Be32(rel + roOff + RO_VALUES + 4 * i);
    return TRUE;
}

u32 Orre_FlagValue(u16 flag)
{
    const struct FlagDef *f;
    const u32 *buf;
    u32 v, w, b;
    if (flag >= ORRE_KAI_FLAG_FIRST)
        return flag < 0x1000 && (sState.kai[(flag - ORRE_KAI_FLAG_FIRST) >> 3] >> (flag & 7) & 1);
    if (!Load() || flag >= sCount || (f = &sDefs[flag])->level == 0)
        return 0;
    buf = sState.level[f->level];
    w = f->bit >> 5;
    b = f->bit & 31;
    if (f->width == 1)
        return buf[w] >> b & 1;
    v = buf[w] >> b;
    if (b + f->width > 32)
        v |= buf[w + 1] << (32 - b);
    return f->width == 32 ? v : v & ((1u << f->width) - 1);
}

void Orre_SetFlagValue(u16 flag, u32 value)
{
    const struct FlagDef *f;
    u32 *buf, w, b, mask;
    if (flag >= ORRE_KAI_FLAG_FIRST)
    {
        if (flag < 0x1000)
        {
            if (value)
                sState.kai[(flag - ORRE_KAI_FLAG_FIRST) >> 3] |= 1 << (flag & 7);
            else
                sState.kai[(flag - ORRE_KAI_FLAG_FIRST) >> 3] &= ~(1 << (flag & 7));
        }
        return;
    }
    if (!Load() || flag >= sCount || (f = &sDefs[flag])->level == 0)
        return;
    buf = sState.level[f->level];
    w = f->bit >> 5;
    b = f->bit & 31;
    mask = f->width == 32 ? 0xFFFFFFFF : (1u << f->width) - 1;
    value &= mask;   // (Colosseum logs a value too wide and keeps its low bits)
    buf[w] = (buf[w] & ~(mask << b)) | value << b;
    if (b + f->width > 32)
    {
        u32 spill = b + f->width - 32;
        buf[w + 1] = (buf[w + 1] & ~((1u << spill) - 1)) | value >> (f->width - spill);
    }
}

bool8 Orre_Flag(u16 flag)
{
    return Orre_FlagValue(flag) != 0;
}

void Orre_SetFlag(u16 flag, bool8 on)
{
    Orre_SetFlagValue(flag, on ? 1 : 0);
}

u16 Orre_FlagCount(void)
{
    return Load() ? sCount : 0;
}

u8 Orre_FlagWidth(u16 flag)
{
    return Load() && flag < sCount ? sDefs[flag].width : (flag >= ORRE_KAI_FLAG_FIRST ? 1 : 0);
}

// Colosseum's story jump (fn_8018FE30): the flags from FLAG on in the story's order cleared, the ones before it set
// to their values. Its item swaps and scene moves are only logged (tests reach a story point by it)
void Orre_StoryJump(u16 flag)
{
    s16 cur;
    u32 guard = 0;
    if (!Load() || flag >= sCount)
        return;
    for (cur = flag; cur >= 0 && cur < sCount && guard++ < MAX_FLAGS; cur = sDefs[cur].next)
        if (sDefs[cur].jump)
            Orre_SetFlagValue(cur, 0);
    guard = 0;
    for (cur = sHead; cur >= 0 && cur < sCount && cur != flag && guard++ < MAX_FLAGS; cur = sDefs[cur].next)
    {
        if (sDefs[cur].jump && sDefs[cur].jump < ARRAY_COUNT(sJumpValues))
            Orre_SetFlagValue(cur, sJumpValues[sDefs[cur].jump]);
        if (sDefs[cur].swap)
            printf("[orre] story jump: flag 0x%X swaps items (%u)\n", cur, sDefs[cur].swap);
    }
    if (sDefs[flag].event)
        printf("[orre] story jump: flag 0x%X has scene %u\n", flag, sDefs[flag].event);
    printf("[test] orre story jump 0x%X\n", flag);
}

// ---- "<save>.orre" (src/orre/orre_shadow.c)

u8 *Orre_StoryState(u32 *size)
{
    *size = sizeof(sState);
    return (u8 *)&sState;
}

void Orre_StoryReset(void)
{
    memset(&sState, 0, sizeof(sState));
    sOldCount = 0;
}

// A format 1 sidecar's flag bytes (one bit a number, 0x1000 of them): Kai's own kept, Colosseum's set as 1
void Orre_StoryFromOldBits(const u8 *bits, u32 size)
{
    u32 i;
    for (i = 0; i < size * 8 && i < 0x1000; i++)
        if (bits[i >> 3] >> (i & 7) & 1)
        {
            if (i >= ORRE_KAI_FLAG_FIRST)
                Orre_SetFlagValue(i, 1);
            else if (sOldCount < ARRAY_COUNT(sOld))
                sOld[sOldCount++] = i;
        }
    if (sTried)
        ApplyOld();
}

static void ApplyOld(void)
{
    u8 i, n = sOldCount;
    sOldCount = 0;   // (Orre_SetFlagValue reads the table: no recursion through Load)
    for (i = 0; i < n; i++)
        Orre_SetFlagValue(sOld[i], 1);
}
