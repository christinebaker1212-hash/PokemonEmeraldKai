// Moemon Rhythm: graphics/rhythm/rhythm.bin (tools/rhythm/gen_assets.py), built in with the assembler's .incbin
// like src/ygo/ygo_art_data.c: DDR EXTREME's art cut down to 240x160 and its sounds and announcer.
// json_data_rules.mk makes this file depend on it.
#include <stdlib.h>
#include <string.h>
#include "rh.h"

#ifdef _WIN32
#define RH_ART_SECTION ".section .rdata,\"dr\"\n"
#else
#define RH_ART_SECTION ".section .rodata\n"
#endif

__asm__(
    RH_ART_SECTION
    ".balign 16\n"
    ".global gRhythmAssets\n"
    "gRhythmAssets:\n"
    ".incbin \"graphics/rhythm/rhythm.bin\"\n"
    ".global gRhythmAssetsEnd\n"
    "gRhythmAssetsEnd:\n"
    ".byte 0\n"
    ".text\n");

extern const uint8_t gRhythmAssets[], gRhythmAssetsEnd[];

#define NAME_LEN 64
#define ENTRY 80

struct Entry
{
    char name[NAME_LEN];
    struct RhPic pic;
    uint16_t kind;
    uint32_t offset, size;
};

static struct Entry *sEntries;
static int sCount;

static uint32_t U32(const uint8_t *p)
{
    return p[0] | p[1] << 8 | p[2] << 16 | (uint32_t)p[3] << 24;
}

static uint16_t U16(const uint8_t *p)
{
    return (uint16_t)(p[0] | p[1] << 8);
}

static void Load(void)
{
    static int sTried;
    int i;
    if (sTried)
        return;
    sTried = 1;
    if (gRhythmAssetsEnd - gRhythmAssets < 8 || memcmp(gRhythmAssets, "RHY1", 4))
        return;
    sCount = (int)U32(gRhythmAssets + 4);
    sEntries = calloc(sCount, sizeof(*sEntries));
    for (i = 0; i < sCount; i++)
    {
        const uint8_t *e = gRhythmAssets + 8 + i * ENTRY;
        memcpy(sEntries[i].name, e, NAME_LEN);
        sEntries[i].name[NAME_LEN - 1] = 0;
        sEntries[i].kind = U16(e + 64);
        sEntries[i].pic.w = U16(e + 66);
        sEntries[i].pic.h = U16(e + 68);
        sEntries[i].pic.frames = U16(e + 70);
        sEntries[i].offset = U32(e + 72);
        sEntries[i].size = U32(e + 76);
        sEntries[i].pic.rgba = gRhythmAssets + sEntries[i].offset;
    }
}

static const struct Entry *Find(const char *name, int kind)
{
    int i;
    Load();
    for (i = 0; i < sCount; i++)
        if (sEntries[i].kind == kind && !strcmp(sEntries[i].name, name))
            return &sEntries[i];
    return NULL;
}

const struct RhPic *RhPic_Get(const char *name)
{
    const struct Entry *e = Find(name, 0);
    return e ? &e->pic : NULL;
}

const uint8_t *RhSound_Get(const char *name, uint32_t *size)
{
    const struct Entry *e = Find(name, 1);
    if (!e)
        return NULL;
    *size = e->size;
    return gRhythmAssets + e->offset;
}

int RhSound_Count(const char *prefix)
{
    int i, n = 0;
    size_t len = strlen(prefix);
    Load();
    for (i = 0; i < sCount; i++)
        if (sEntries[i].kind == 1 && !strncmp(sEntries[i].name, prefix, len))
            n++;
    return n;
}
