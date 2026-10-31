/* What EDOPro's duel engine reads, from graphics/ygo/edo.pak (tools/ygo/edo/build_pack.py): the card database, the
 * card scripts, the system strings and the Forbidden & Limited lists. The pack is built in (ygo_edo_pak.c); its
 * chunks are inflated as they're needed and the last few kept. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ygo_edo.h"

char *stbi_zlib_decode_noheader_malloc(const char *buffer, int len, int *outlen);
extern const uint8_t gYgoEdoPak[], gYgoEdoPakEnd[];
extern const uint32_t gYgoEdoCodes[YGO_CARD_COUNT + 1];

#define ENTRY 52
#define CACHE 6

struct EdoList
{
    char name[32];
    int whitelist;
    uint32_t count;
    uint32_t *codes;
    uint8_t *limits;
};
static struct
{
    int ready, failed;
    uint32_t count, chunks;
    const uint8_t *entries, *chunkTable;
    struct { int chunk; uint8_t *data; uint32_t size, used; } cache[CACHE];
    uint32_t useClock;
    /* cards */
    uint8_t *cardBlob;
    struct YgoEdoRec *recs;
    uint32_t recCount;
    struct YgoCard *extra;          /* a card for each record, ids YGO_CARD_COUNT + 1 + index */
    uint16_t *catalogOfRec;         /* the catalog card a record is, 0 none */
    /* strings */
    uint8_t *stringBlob;
    struct { uint32_t id; const char *text; } *system;
    uint32_t systemCount;
    /* lists */
    struct EdoList lists[YGO_EDO_LISTS_MAX];
    unsigned listCount;
} E;

static uint32_t U32(const uint8_t *p) { return p[0] | p[1] << 8 | p[2] << 16 | (uint32_t)p[3] << 24; }
static uint64_t U64(const uint8_t *p) { return U32(p) | (uint64_t)U32(p + 4) << 32; }

static int Header(void)
{
    size_t size = (size_t)(gYgoEdoPakEnd - gYgoEdoPak);
    if (size < 12 || memcmp(gYgoEdoPak, "EDOP", 4))
        return 0;
    E.count = U32(gYgoEdoPak + 4);
    E.chunks = U32(gYgoEdoPak + 8);
    E.entries = gYgoEdoPak + 12;
    E.chunkTable = E.entries + (size_t)ENTRY * E.count;
    return (size_t)(E.chunkTable + 12 * (size_t)E.chunks - gYgoEdoPak) <= size;
}
static const uint8_t *Find(const char *name)
{
    uint32_t lo = 0, hi = E.count;
    while (lo < hi)
    {
        uint32_t mid = (lo + hi) / 2;
        int c = strncmp((const char *)E.entries + (size_t)ENTRY * mid, name, 40);
        if (!c)
            return E.entries + (size_t)ENTRY * mid;
        if (c < 0)
            lo = mid + 1;
        else
            hi = mid;
    }
    return NULL;
}
static const uint8_t *Chunk(uint32_t chunk, uint32_t *size)
{
    int i, slot = 0;
    const uint8_t *c;
    int len = 0;
    char *data;
    for (i = 0; i < CACHE; i++)
        if (E.cache[i].data && E.cache[i].chunk == (int)chunk)
        {
            E.cache[i].used = ++E.useClock;
            *size = E.cache[i].size;
            return E.cache[i].data;
        }
    if (chunk >= E.chunks)
        return NULL;
    c = E.chunkTable + 12 * (size_t)chunk;
    data = stbi_zlib_decode_noheader_malloc((const char *)gYgoEdoPak + U32(c), (int)U32(c + 4), &len);
    if (!data || (uint32_t)len != U32(c + 8))
    {
        free(data);
        printf("[ygo] edo: chunk %u of the pack won't inflate\n", (unsigned)chunk);
        return NULL;
    }
    for (i = 1; i < CACHE; i++)
        if (!E.cache[i].data || E.cache[i].used < E.cache[slot].used)
            slot = i;
    if (!E.cache[0].data)
        slot = 0;
    free(E.cache[slot].data);
    E.cache[slot].data = (uint8_t *)data;
    E.cache[slot].chunk = (int)chunk;
    E.cache[slot].size = (uint32_t)len;
    E.cache[slot].used = ++E.useClock;
    *size = (uint32_t)len;
    return (const uint8_t *)data;
}
int YgoEdo_HasFile(const char *name)
{
    return (E.ready || Header()) && Find(name) != NULL;
}
uint8_t *YgoEdo_File(const char *name, uint32_t *size)
{
    const uint8_t *e, *chunk;
    uint32_t csize = 0, off, len;
    uint8_t *out;
    if (!E.entries && !Header())
        return NULL;
    if (!(e = Find(name)) || !(chunk = Chunk(U32(e + 40), &csize)))
        return NULL;
    off = U32(e + 44);
    len = U32(e + 48);
    if (off + len > csize || !(out = malloc(len + 1)))
        return NULL;
    memcpy(out, chunk + off, len);
    out[len] = 0;
    if (size)
        *size = len;
    return out;
}

/* ------------------------------------------------------------------ */
/* Cards                                                                */
/* ------------------------------------------------------------------ */
static int LoadCards(void)
{
    uint32_t size = 0, i, n;
    const uint8_t *p, *text;
    if (!(E.cardBlob = YgoEdo_File("cards.bin", &size)) || size < 8 || memcmp(E.cardBlob, "EDOC", 4))
        return 0;
    n = U32(E.cardBlob + 4);
    if (8 + (size_t)n * 52 > size)
        return 0;
    E.recs = calloc(n, sizeof(*E.recs));
    E.extra = calloc(n, sizeof(*E.extra));
    E.catalogOfRec = calloc(n, sizeof(*E.catalogOfRec));
    if (!E.recs || !E.extra || !E.catalogOfRec)
        return 0;
    text = E.cardBlob + 8 + (size_t)n * 52;
    for (i = 0, p = E.cardBlob + 8; i < n; i++, p += 52)
    {
        struct YgoEdoRec *r = &E.recs[i];
        const char *s;
        unsigned k;
        r->code = U32(p);
        r->alias = U32(p + 4);
        r->setcode = U64(p + 8);
        r->type = U32(p + 16);
        r->level = U32(p + 20);
        r->attribute = U32(p + 24);
        r->race = U64(p + 28);
        r->atk = (int32_t)U32(p + 36);
        r->def = (int32_t)U32(p + 40);
        r->ot = U32(p + 44);
        s = (const char *)text + U32(p + 48);
        r->name = s;
        s += strlen(s) + 1;
        r->desc = s;
        for (k = 0; k < 16; k++)
        {
            s += strlen(s) + 1;
            r->str[k] = s;
        }
    }
    E.recCount = n;
    return 1;
}
static int RecIndex(uint32_t code)
{
    uint32_t lo = 0, hi = E.recCount;
    while (lo < hi)
    {
        uint32_t mid = (lo + hi) / 2;
        if (E.recs[mid].code < code)
            lo = mid + 1;
        else
            hi = mid;
    }
    return lo < E.recCount && E.recs[lo].code == code ? (int)lo : -1;
}
const struct YgoEdoRec *YgoEdo_Rec(uint32_t code)
{
    int i;
    if (!YgoEdo_DataInit() || (i = RecIndex(code)) < 0)
        return NULL;
    return &E.recs[i];
}
unsigned YgoEdo_RecCount(void) { return YgoEdo_DataInit() ? E.recCount : 0; }
const struct YgoEdoRec *YgoEdo_RecAt(unsigned index)
{
    return YgoEdo_DataInit() && index < E.recCount ? &E.recs[index] : NULL;
}

static unsigned AttributeOf(uint32_t a)
{
    return a & 0x10 ? YGO_LIGHT : a & 0x20 ? YGO_DARK : a & 0x2 ? YGO_WATER : a & 0x4 ? YGO_FIRE : a & 0x1 ? YGO_EARTH
         : a & 0x8 ? YGO_WIND : a & 0x40 ? YGO_DIVINE : YGO_ATTR_NONE;
}
static unsigned RaceOf(uint64_t r)
{
    static const struct { uint64_t bit; uint8_t race; } sRaces[] = {
        {0x1, YGO_WARRIOR}, {0x2, YGO_SPELLCASTER}, {0x4, YGO_FAIRY}, {0x8, YGO_FIEND}, {0x10, YGO_ZOMBIE},
        {0x20, YGO_MACHINE}, {0x40, YGO_AQUA}, {0x80, YGO_PYRO}, {0x100, YGO_ROCK}, {0x200, YGO_WINGED_BEAST},
        {0x400, YGO_PLANT}, {0x800, YGO_INSECT}, {0x1000, YGO_THUNDER}, {0x2000, YGO_DRAGON}, {0x4000, YGO_BEAST},
        {0x8000, YGO_BEAST_WARRIOR}, {0x10000, YGO_DINOSAUR}, {0x20000, YGO_FISH}, {0x40000, YGO_SEA_SERPENT},
        {0x80000, YGO_REPTILE}};
    unsigned i;
    for (i = 0; i < sizeof(sRaces) / sizeof(sRaces[0]); i++)
        if (r & sRaces[i].bit)
            return sRaces[i].race;
    return YGO_RACE_NONE;
}
static void MakeExtra(unsigned i)
{
    const struct YgoEdoRec *r = &E.recs[i];
    struct YgoCard *c = &E.extra[i];
    uint32_t t = r->type;
    c->name = r->name;
    c->text = r->desc;
    c->password = r->code;
    c->id = (uint16_t)(YGO_CARD_COUNT + 1 + i);
    c->atk = r->atk < 0 ? YGO_STAT_UNKNOWN : (uint16_t)r->atk;
    c->def = r->def < 0 ? YGO_STAT_UNKNOWN : (uint16_t)r->def;
    c->level = (uint8_t)(r->level & 0xff);
    c->attribute = (uint8_t)AttributeOf(r->attribute);
    c->race = (uint8_t)RaceOf(r->race);
    c->frame = t & EDO_TYPE_TOKEN ? YGO_TOKEN : t & EDO_TYPE_SPELL ? YGO_SPELL : t & EDO_TYPE_TRAP ? YGO_TRAP
             : t & (EDO_TYPE_FUSION | EDO_TYPE_SYNCHRO | EDO_TYPE_XYZ | EDO_TYPE_LINK) ? YGO_FUSION
             : t & EDO_TYPE_RITUAL ? YGO_RITUAL : t & EDO_TYPE_EFFECT ? YGO_EFFECT : YGO_NORMAL;
    c->property = t & EDO_TYPE_CONTINUOUS ? YGO_PROP_CONTINUOUS : t & EDO_TYPE_EQUIP ? YGO_PROP_EQUIP
                : t & EDO_TYPE_FIELD ? YGO_PROP_FIELD : t & EDO_TYPE_QUICKPLAY ? YGO_PROP_QUICK
                : t & EDO_TYPE_RITUAL && !(t & EDO_TYPE_MONSTER) ? YGO_PROP_RITUAL
                : t & EDO_TYPE_COUNTER ? YGO_PROP_COUNTER : YGO_PROP_NORMAL;
    c->abilities = (uint8_t)((t & EDO_TYPE_FLIP ? YGO_ABILITY_FLIP : 0) | (t & EDO_TYPE_TOON ? YGO_ABILITY_TOON : 0) |
                             (t & EDO_TYPE_SPIRIT ? YGO_ABILITY_SPIRIT : 0) | (t & EDO_TYPE_UNION ? YGO_ABILITY_UNION : 0) |
                             (t & EDO_TYPE_GEMINI ? YGO_ABILITY_GEMINI : 0));
    c->limit = 3;
}

/* ------------------------------------------------------------------ */
/* Strings                                                              */
/* ------------------------------------------------------------------ */
static int CmpSystem(const void *a, const void *b)
{
    uint32_t x = *(const uint32_t *)a, y = *(const uint32_t *)b;
    return x < y ? -1 : x > y;
}
static void LoadStrings(void)
{
    uint32_t size = 0, n = 0, cap = 1024;
    char *p, *line;
    if (!(E.stringBlob = YgoEdo_File("strings.conf", &size)))
        return;
    E.system = malloc(sizeof(*E.system) * cap);
    for (line = (char *)E.stringBlob; line && *line; line = p)
    {
        char *end;
        unsigned long id;
        p = strchr(line, '\n');
        if (p)
            *p++ = 0;
        if (!strncmp(line, "!counter ", 9))
        {
            /* the counters' names, kept apart from the system strings by the top bit */
            id = strtoul(line + 9, &end, 0);
            if (end == line + 9 || *end != ' ')
                continue;
            id |= 0x80000000u;
        }
        else if (strncmp(line, "!system ", 8))
            continue;
        else
            id = strtoul(line + 8, &end, 0);
        if (end == line + 8 || *end != ' ')
            continue;   /* (a "!system" line with no number; "!counter" ones were checked above) */
        end[strcspn(end, "\r")] = 0;
        if (n == cap)
            E.system = realloc(E.system, sizeof(*E.system) * (cap *= 2));
        E.system[n].id = (uint32_t)id;
        E.system[n].text = end + 1;
        n++;
    }
    qsort(E.system, n, sizeof(*E.system), CmpSystem);
    E.systemCount = n;
}
const char *YgoEdo_System(unsigned id)
{
    uint32_t lo = 0, hi;
    if (!YgoEdo_DataInit())
        return NULL;
    hi = E.systemCount;
    while (lo < hi)
    {
        uint32_t mid = (lo + hi) / 2;
        if (E.system[mid].id < id)
            lo = mid + 1;
        else
            hi = mid;
    }
    return lo < E.systemCount && E.system[lo].id == id ? E.system[lo].text : NULL;
}
/* A counter's name ("Spell Counter"), by its kind as the engine gives it (some carry flag bits: 0x1002 Wedge Counter) */
const char *YgoEdo_Counter(unsigned type)
{
    const char *t = type ? YgoEdo_System(0x80000000u | type) : NULL;
    if (!t && type)
        t = YgoEdo_System(0x80000000u | (type & 0xfff));
    return t;
}
const char *YgoEdo_Desc(uint64_t desc)
{
    if (desc >> 20)
    {
        const struct YgoEdoRec *r = YgoEdo_Rec((uint32_t)(desc >> 20));
        unsigned n = (unsigned)(desc & 0xfffff);
        return r && n < 16 && r->str[n][0] ? r->str[n] : NULL;
    }
    return YgoEdo_System((unsigned)desc);
}

/* ------------------------------------------------------------------ */
/* Forbidden & Limited lists                                            */
/* ------------------------------------------------------------------ */
static void LoadLists(void)
{
    uint32_t i;
    for (i = 0; i < E.count && E.listCount < YGO_EDO_LISTS_MAX; i++)
    {
        const char *name = (const char *)E.entries + (size_t)ENTRY * i;
        uint8_t *text;
        char *line, *p;
        uint32_t cap = 0;
        if (strncmp(name, "lflist/", 7) || !(text = YgoEdo_File(name, NULL)))
            continue;
        for (line = (char *)text; line && *line; line = p)
        {
            p = strchr(line, '\n');
            if (p)
                *p++ = 0;
            line[strcspn(line, "\r")] = 0;
            if (line[0] == '!')
            {
                /* a new list (a file holds one or more) */
                if (E.listCount && !E.lists[E.listCount - 1].count && !E.lists[E.listCount - 1].whitelist)
                    E.listCount--;   /* (an empty one) */
                if (E.listCount >= YGO_EDO_LISTS_MAX)
                    break;
                snprintf(E.lists[E.listCount].name, sizeof(E.lists[0].name), "%s", line + 1);
                E.lists[E.listCount].whitelist = 0;
                E.lists[E.listCount].count = 0;
                E.lists[E.listCount].codes = NULL;
                E.lists[E.listCount].limits = NULL;
                E.listCount++;
                cap = 0;
            }
            else if (line[0] == '$' && E.listCount && !strncmp(line, "$whitelist", 10))
                E.lists[E.listCount - 1].whitelist = 1;
            else if (line[0] >= '0' && line[0] <= '9' && E.listCount)
            {
                char *end;
                unsigned long code = strtoul(line, &end, 10), limit;
                struct EdoList *l = &E.lists[E.listCount - 1];
                if (*end != ' ')
                    continue;
                limit = strtoul(end + 1, NULL, 10);
                if (l->count == cap)
                {
                    cap = cap ? cap * 2 : 256;
                    l->codes = realloc(l->codes, cap * sizeof(*l->codes));
                    l->limits = realloc(l->limits, cap);
                }
                l->codes[l->count] = (uint32_t)code;
                l->limits[l->count] = (uint8_t)(limit > 3 ? 3 : limit);
                l->count++;
            }
        }
        free(text);
    }
}
unsigned YgoEdo_ListCount(void) { return YgoEdo_DataInit() ? E.listCount : 0; }
const char *YgoEdo_ListName(unsigned list) { return list < E.listCount ? E.lists[list].name : ""; }
unsigned YgoEdo_ListLimit(unsigned list, uint32_t code)
{
    uint32_t i;
    const struct YgoEdoRec *r;
    if (list >= E.listCount)
        return 3;
    /* alternate artworks count as the card they show */
    if ((r = YgoEdo_Rec(code)) && r->alias && r->alias < code + 20 && code < r->alias + 20)
        code = r->alias;
    for (i = 0; i < E.lists[list].count; i++)
        if (E.lists[list].codes[i] == code)
            return E.lists[list].limits[i];
    return E.lists[list].whitelist ? 0 : 3;
}

/* ------------------------------------------------------------------ */
int YgoEdo_DataInit(void)
{
    unsigned i;
    if (E.ready || E.failed)
        return E.ready;
    if (!Header() || !LoadCards())
    {
        E.failed = 1;
        printf("[ygo] edo: the card pack is missing or damaged\n");
        return 0;
    }
    E.ready = 1;
    for (i = 0; i < E.recCount; i++)
        MakeExtra(i);
    for (i = 1; i <= YGO_CARD_COUNT; i++)
    {
        int r = RecIndex(gYgoEdoCodes[i]);
        if (r >= 0 && !E.catalogOfRec[r])
            E.catalogOfRec[r] = (uint16_t)i;
    }
    LoadStrings();
    LoadLists();
    printf("[ygo] edo: %u cards, %u scripts and data files, %u system strings, %u Forbidden & Limited lists\n",
           (unsigned)E.recCount, (unsigned)E.count, (unsigned)E.systemCount, E.listCount);
    return 1;
}
unsigned YgoEdo_IdOfCode(uint32_t code)
{
    int i;
    if (!code || !YgoEdo_DataInit() || (i = RecIndex(code)) < 0)
        return 0;
    return E.catalogOfRec[i] ? E.catalogOfRec[i] : YGO_CARD_COUNT + 1 + (unsigned)i;
}
/* The card a code is as the game knows it: its catalog card, also when the code is another printing (an alternate
 * artwork) of it; 0 when the catalog doesn't have the card */
static uint32_t Canonical(uint32_t code)
{
    int i = RecIndex(code);
    const struct YgoEdoRec *r = i >= 0 ? &E.recs[i] : NULL;
    if (r && r->alias && r->alias < code + 20 && code < r->alias + 20)
        return r->alias;
    return code;
}
unsigned YgoEdo_CatalogOf(uint32_t code)
{
    unsigned id, i;
    uint32_t canon;
    if (!code || !YgoEdo_DataInit())
        return 0;
    id = YgoEdo_IdOfCode(code);
    if (id && id <= YGO_CARD_COUNT)
        return id;
    canon = Canonical(code);
    for (i = 1; i <= YGO_CARD_COUNT; i++)
        if (Canonical(gYgoEdoCodes[i]) == canon)
            return i;
    return 0;
}
uint32_t YgoEdo_CodeOfId(unsigned id)
{
    if (id >= 1 && id <= YGO_CARD_COUNT)
        return gYgoEdoCodes[id];
    if (id > YGO_CARD_COUNT && YgoEdo_DataInit() && id - YGO_CARD_COUNT - 1 < E.recCount)
        return E.recs[id - YGO_CARD_COUNT - 1].code;
    return 0;
}
/* The catalog's Ygo_Card for ids past its own cards */
const struct YgoCard *Ygo_EdoCard(unsigned id)
{
    if (id <= YGO_CARD_COUNT || !YgoEdo_DataInit() || id - YGO_CARD_COUNT - 1 >= E.recCount)
        return NULL;
    return &E.extra[id - YGO_CARD_COUNT - 1];
}
int YgoEdo_Playable(uint32_t code)
{
    const struct YgoEdoRec *r = YgoEdo_Rec(code);
    char name[24];
    if (!r)
        return 0;
    if ((r->type & EDO_TYPE_NORMAL) && !(r->type & (EDO_TYPE_FUSION | EDO_TYPE_RITUAL | EDO_TYPE_EFFECT)))
        return 1;
    snprintf(name, sizeof(name), "c%u.lua", (unsigned)code);
    if (YgoEdo_HasFile(name))
        return 1;
    if (r->alias && r->alias < code + 10 && code < r->alias + 10)
    {
        snprintf(name, sizeof(name), "c%u.lua", (unsigned)r->alias);
        return YgoEdo_HasFile(name);
    }
    return 0;
}
