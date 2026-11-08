// Moemon Kai: ORRE's data pack (orre/orre_data.pak, tools/orre/build_pack.py): "ORDT" u32 version, u32 files, each
// char name[48], u32 offset, u32 bytes, u32 deflated bytes (0: stored); offsets from the data start
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include "orre_data.h"

const char *Updater_OrreRoot(void);
int uncompress(unsigned char *dest, unsigned long *destLen, const unsigned char *source, unsigned long sourceLen);   // (zlib)

typedef struct
{
    char name[48];
    uint32_t off, size, zsize;
} Entry;

static struct
{
    char path[1024];
    int tried;
    Entry *e;
    uint32_t n, dataStart;
    char *ids;   // index/ids.txt
} sPak;

static uint32_t Le32(const uint8_t *p)
{
    return p[0] | p[1] << 8 | p[2] << 16 | (uint32_t)p[3] << 24;
}

const char *OrreData_Path(void)
{
    if (getenv("MOEMON_ORRE_DATA"))
        snprintf(sPak.path, sizeof(sPak.path), "%s", getenv("MOEMON_ORRE_DATA"));
    else
        snprintf(sPak.path, sizeof(sPak.path), "%s/orre_data.pak", Updater_OrreRoot());
    return sPak.path;
}

static int Open(void)
{
    FILE *f;
    uint8_t head[12], *table;
    uint32_t i;
    static time_t sLast;
    if (sPak.e)
        return 1;
    // (not there or not readable: looked for again, every few seconds at most: a pack put there while the game runs,
    // the boat's download, is read then)
    if (sPak.tried && time(NULL) - sLast < 3)
        return 0;
    sPak.tried = 1;
    sLast = time(NULL);
    if ((f = fopen(OrreData_Path(), "rb")) == NULL)
    {
        printf("[orre] no data pack at %s\n", sPak.path);
        return 0;
    }
    if (fread(head, 1, 12, f) != 12 || memcmp(head, "ORDT", 4) != 0)
    {
        fclose(f);
        printf("[orre] %s isn't ORRE's data pack\n", sPak.path);
        return 0;
    }
    sPak.n = Le32(head + 8);
    table = malloc(sPak.n * 60u);
    sPak.e = calloc(sPak.n, sizeof(Entry));
    if (table == NULL || sPak.e == NULL || fread(table, 60, sPak.n, f) != sPak.n)
    {
        fclose(f);
        free(table);
        free(sPak.e);
        sPak.e = NULL;
        return 0;
    }
    fclose(f);
    for (i = 0; i < sPak.n; i++)
    {
        memcpy(sPak.e[i].name, table + i * 60, 47);
        sPak.e[i].off = Le32(table + i * 60 + 48);
        sPak.e[i].size = Le32(table + i * 60 + 52);
        sPak.e[i].zsize = Le32(table + i * 60 + 56);
    }
    free(table);
    sPak.dataStart = 12 + sPak.n * 60;
    return 1;
}

uint8_t *OrreData_Read(const char *name, uint32_t *size)
{
    uint32_t i, stored;
    FILE *f;
    uint8_t *raw, *out;
    unsigned long got;
    if (!Open())
        return NULL;
    for (i = 0; i < sPak.n; i++)
        if (strcmp(sPak.e[i].name, name) == 0)
            break;
    if (i == sPak.n)
        return NULL;
    stored = sPak.e[i].zsize ? sPak.e[i].zsize : sPak.e[i].size;
    if ((f = fopen(sPak.path, "rb")) == NULL)
        return NULL;
    raw = malloc(stored + 1);
    if (raw == NULL || fseek(f, (long)(sPak.dataStart + sPak.e[i].off), SEEK_SET) != 0 || fread(raw, 1, stored, f) != stored)
    {
        fclose(f);
        free(raw);
        return NULL;
    }
    fclose(f);
    if (!sPak.e[i].zsize)
        out = raw;
    else
    {
        out = malloc(sPak.e[i].size + 1);
        got = sPak.e[i].size;
        if (out == NULL || uncompress(out, &got, raw, stored) != 0 || got != sPak.e[i].size)
        {
            free(out);
            free(raw);
            printf("[orre] %s won't inflate\n", name);
            return NULL;
        }
        free(raw);
    }
    out[sPak.e[i].size] = 0;   // (text files end there)
    if (size)
        *size = sPak.e[i].size;
    return out;
}

int OrreData_ResourceName(uint32_t id, char *out, int outSize)
{
    char key[12];
    const char *p;
    int n;
    if (sPak.ids == NULL && (sPak.ids = (char *)OrreData_Read("index/ids.txt", NULL)) == NULL)
        return 0;
    snprintf(key, sizeof(key), "%08x ", id);
    for (p = sPak.ids; p && *p; p = strchr(p, '\n'), p = p ? p + 1 : NULL)
        if (strncmp(p, key, 9) == 0)
        {
            p += 9;
            for (n = 0; p[n] && p[n] != '\n' && n < outSize - 1; n++)
                out[n] = p[n];
            out[n] = 0;
            return 1;
        }
    return 0;
}
