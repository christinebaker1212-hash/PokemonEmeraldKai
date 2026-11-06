// Moemon Maker: what the editor asks of the game (include/maker_api.h). The editor (C++, maker/) never includes the
// game's headers; everything it knows about maps, tiles, people and songs comes through here, read from this build's
// own data, so it's never behind the game (docs/MOEMON_MAKER.md "The Maker is part of the game").
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "global.fieldmap.h"
#include "overworld.h"
#include "region_map.h"
#include "constants/region_map_sections.h"
#include "constants/characters.h"
#include "maker.h"
#include "mp_version.h"
#include "maker_api.h"

// The game's text as UTF-8 (é for the font's é; the rest as GameCharToAscii)
static void GameToUtf8(char *dst, int size, const u8 *src)
{
    void MP_GameToAscii(char *dst, const u8 *src, int maxLen);
    int n = 0;
    for (; *src != EOS && n < size - 3; src++)
    {
        if (*src == CHAR_e_ACUTE || *src == CHAR_E_ACUTE)
        {
            dst[n++] = (char)0xC3;
            dst[n++] = (char)(*src == CHAR_e_ACUTE ? 0xA9 : 0x89);
        }
        else
        {
            u8 one[2] = {*src, EOS};
            char c[2];
            MP_GameToAscii(c, one, 1);
            if (c[0])
                dst[n++] = c[0];
        }
    }
    dst[n] = 0;
}

static int RegionOf(const char *name)
{
    if (strncmp(name, "MAP_JOHTO_", 10) == 0)
        return MAKER_REGION_JOHTO;
    if (strncmp(name, "MAP_KANTO_", 10) == 0)
        return MAKER_REGION_KANTO;
    if (strncmp(name, "MAP_SINNOH_", 11) == 0)
        return MAKER_REGION_SINNOH;
    return MAKER_REGION_HOENN;
}

// "MAP_SINNOH_VEILSTONE_STORE2F" -> "VEILSTONE STORE2F"; "MAP_ROUTE101" -> "ROUTE 101"
static void Title(char *dst, int size, const char *name)
{
    static const char *const sPrefixes[] = {"MAP_JOHTO_", "MAP_KANTO_", "MAP_SINNOH_", "MAP_"};
    const char *s = name;
    int i, n = 0;
    for (i = 0; i < 4; i++)
        if (strncmp(s, sPrefixes[i], strlen(sPrefixes[i])) == 0)
        {
            s += strlen(sPrefixes[i]);
            break;
        }
    for (; *s && n < size - 2; s++)
    {
        if (*s == '_')
            dst[n++] = ' ';
        else
        {
            // (a space before a route's number: ROUTE101 -> ROUTE 101)
            if (*s >= '0' && *s <= '9' && n >= 5 && strncmp(dst + n - 5, "ROUTE", 5) == 0)
                dst[n++] = ' ';
            dst[n++] = *s;
        }
    }
    dst[n] = 0;
}

int MakerApi_MapCount(void)
{
    return Maker_MapCount();
}

int MakerApi_MapInfo(int index, struct MakerMapInfo *info)
{
    const struct MakerMapName *m;
    const struct MapHeader *h;
    u8 buf[64];
    if (index < 0 || index >= Maker_MapCount())
        return 0;
    m = Maker_Map(index);
    h = Maker_BaseMapHeader(m->mapGroup, m->mapNum);
    memset(info, 0, sizeof(*info));
    info->name = m->name;
    info->groupName = m->group;
    info->region = index >= Maker_MapCount() - Maker_NewMapCount() ? Maker_NewMapRegion(m->mapNum) : RegionOf(m->name);
    info->mapGroup = m->mapGroup;
    info->mapNum = m->mapNum;
    Title(info->title, sizeof(info->title), m->name);
    if (h == NULL)
        return 1;
    if (h->mapLayout)
    {
        info->width = h->mapLayout->width;
        info->height = h->mapLayout->height;
    }
    info->music = h->music;
    if (h->regionMapSectionId != MAPSEC_SECRET_BASE && index < Maker_MapCount() - Maker_NewMapCount())   // (new maps: "New Maps")
    {
        buf[0] = EOS;
        GetMapName(buf, h->regionMapSectionId, 0);
        GameToUtf8(info->place, sizeof(info->place), buf);
    }
    return 1;
}

int MakerApi_FindMap(const char *name)
{
    int i;
    for (i = 0; i < Maker_MapCount(); i++)
        if (strcmp(Maker_Map(i)->name, name) == 0)
            return i;
    return -1;
}

const char *MakerApi_GameVersion(void)
{
    return MOEMON_VERSION;
}
