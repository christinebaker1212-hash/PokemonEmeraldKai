/* Yu-Gi-Oh! duels on EDOPro's engine (edopro/ocgcore, edo9300's ygopro-core): the game's duel API (include/ygo/duel.h)
 * on top of it.
 *
 * The engine runs until it needs an answer and leaves a stream of messages: cards moving, summons, chains, damage,
 * then the question. Each message becomes presentation events for the duel screen, and moves the game's objects:
 * every physical card is one object for the whole duel (a card moving keeps its number), tracked here in the
 * engine's own lists (each Deck, hand, Graveyard... in its order; the zones by sequence). When the engine stops,
 * the state is read back from it (every card's code, position, ATK / DEF...) into YgoState, and the question is
 * put into YgoQuery: what the screen and the AI answer. Answers become the engine's responses.
 *
 * The engine's player 0 goes first: when the game's player 1 goes first the two are swapped here (GP / CP).
 * Some questions are answered here without asking (no card can be chained, the zone the Main Phase answer named,
 * the attack target the "attack directly?" question already picked): always from the duel and the answers alone,
 * so a duel rebuilt from its answers (YgoDuel_Copy) and the other side of an online duel run the same. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ygo/duel.h"
#include "ygo_edo.h"
#include "../../edopro/ocgcore/ocgapi.h"
#include "../../edopro/ocgcore/ocgapi_constants.h"

#define NONE 0xFF
#define SEL_MAX 16384
#define PENDING_MAX 24

enum { LI_DECK, LI_HAND, LI_GRAVE, LI_REMOVED, LI_EXTRA, LISTS };
struct List
{
    uint16_t n;
    uint8_t o[YGO_OBJS];
};
struct Loc
{
    uint8_t con, loc;
    uint32_t seq, pos;
};
/* What a query item answers with */
enum
{
    K_IDLE = 1, K_BATTLE, K_CHAIN, K_EFFECTYN, K_YESNO, K_OPTION, K_CARD, K_TRIBUTE, K_UNSELECT, K_SUM, K_POSITION,
    K_PLACE, K_COUNTER, K_SORT, K_RACE, K_ATTRIB, K_NAME, K_NUMBER, K_RPS, K_ATTACK_SYNTH
};
struct YgoEdoDuel
{
    OCG_Duel duel;
    uint8_t swap;   /* the engine's player 0 is the game's player `swap` */
    uint16_t decks[2][YGO_DECK_MAX], extras[2][YGO_FUSION_MAX];
    uint32_t seed;
    uint8_t first;
    /* objects where the engine has them */
    struct List lists[2][LISTS];
    uint8_t mz[2][7], sz[2][8];      /* object + 1 */
    struct List overlay[2][7];
    /* the question */
    uint8_t sel[SEL_MAX];
    uint32_t selLen;
    uint8_t kind;
    int32_t resp[YGO_QUERY_MAX];
    uint8_t mustCount;               /* SUM: items before the choices are the musts */
    uint8_t rpsPlayer;
    uint64_t hint;                   /* the HINT_SELECTMSG for the next question */
    /* answers so far (YgoDuel_Copy rebuilds from them) */
    struct YgoAnswer *log;
    uint32_t logCount, logCap;
    /* what's going on */
    uint8_t inDamage, tributes, quiet, needRebuild, declaring;
    uint8_t wantZone, wantZonePlayer;    /* the Main Phase answer's zone (seq + 1, bit 7 a Spell & Trap Zone; 0 the first) */
    uint8_t wantZoneSet;                 /* ... for the next zone question, when it comes right after */
    uint8_t wantTarget;                  /* the attack target picked at "attack directly?": object + 1 */
    uint8_t pending[PENDING_MAX], pendingFrom[PENDING_MAX], pendingCount;   /* moved onto the field, nothing shown yet */
    int32_t shownAtk[YGO_OBJS];
    uint8_t onField[YGO_OBJS];
    int failed;
};

static unsigned GP(const struct YgoEdoDuel *e, unsigned core) { return core > 1 ? core : core ^ e->swap; }

/* ------------------------------------------------------------------ */
/* Reading messages                                                     */
/* ------------------------------------------------------------------ */
struct Rd
{
    const uint8_t *p, *end;
    int bad;
};
static uint8_t R8(struct Rd *r)
{
    if (r->p + 1 > r->end)
        return r->bad = 1, 0;
    return *r->p++;
}
static uint16_t R16(struct Rd *r)
{
    uint16_t v;
    if (r->p + 2 > r->end)
        return r->bad = 1, 0;
    v = (uint16_t)(r->p[0] | r->p[1] << 8);
    r->p += 2;
    return v;
}
static uint32_t R32(struct Rd *r)
{
    uint32_t v;
    if (r->p + 4 > r->end)
        return r->bad = 1, 0;
    v = r->p[0] | r->p[1] << 8 | r->p[2] << 16 | (uint32_t)r->p[3] << 24;
    r->p += 4;
    return v;
}
static uint64_t R64(struct Rd *r)
{
    uint64_t lo = R32(r);
    return lo | (uint64_t)R32(r) << 32;
}
static struct Loc RLoc(struct Rd *r)
{
    struct Loc l;
    l.con = R8(r);
    l.loc = R8(r);
    l.seq = R32(r);
    l.pos = R32(r);
    return l;
}
/* (con u8, loc u8, seq u32) or (con u8, loc u8, seq u8) */
static struct Loc RLoc3(struct Rd *r, int seq8)
{
    struct Loc l;
    l.con = R8(r);
    l.loc = R8(r);
    l.seq = seq8 ? R8(r) : R32(r);
    l.pos = 0;
    return l;
}

/* ------------------------------------------------------------------ */
/* Objects                                                              */
/* ------------------------------------------------------------------ */
static int ListOf(unsigned loc)
{
    switch (loc)
    {
    case EDO_LOC_DECK: return LI_DECK;
    case EDO_LOC_HAND: return LI_HAND;
    case EDO_LOC_GRAVE: return LI_GRAVE;
    case EDO_LOC_REMOVED: return LI_REMOVED;
    case EDO_LOC_EXTRA: return LI_EXTRA;
    default: return -1;
    }
}
static unsigned YgoLoc(unsigned loc)
{
    switch (loc)
    {
    case EDO_LOC_DECK: return YGO_LOC_DECK;
    case EDO_LOC_HAND: return YGO_LOC_HAND;
    case EDO_LOC_MZONE: return YGO_LOC_MZONE;
    case EDO_LOC_SZONE: return YGO_LOC_SZONE;
    case EDO_LOC_GRAVE: return YGO_LOC_GRAVE;
    case EDO_LOC_REMOVED: return YGO_LOC_BANISHED;
    case EDO_LOC_EXTRA: return YGO_LOC_EXTRA;
    default: return YGO_LOC_NONE;
    }
}
static int ObjAt(const struct YgoEdoDuel *e, struct Loc l)
{
    int li;
    if (l.con > 1)
        return -1;
    if (l.loc & EDO_LOC_OVERLAY)
        return l.seq < 7 && l.pos < e->overlay[l.con][l.seq].n ? e->overlay[l.con][l.seq].o[l.pos] : -1;
    if (l.loc == EDO_LOC_MZONE)
        return l.seq < 7 && e->mz[l.con][l.seq] ? e->mz[l.con][l.seq] - 1 : -1;
    if (l.loc == EDO_LOC_SZONE)
        return l.seq < 8 && e->sz[l.con][l.seq] ? e->sz[l.con][l.seq] - 1 : -1;
    if ((li = ListOf(l.loc)) < 0)
        return -1;
    return l.seq < e->lists[l.con][li].n ? e->lists[l.con][li].o[l.seq] : -1;
}
static void ListRemove(struct List *list, unsigned seq)
{
    if (seq >= list->n)
        return;
    memmove(list->o + seq, list->o + seq + 1, list->n - seq - 1);
    list->n--;
}
static void ListInsert(struct List *list, unsigned seq, unsigned o)
{
    if (list->n >= YGO_OBJS)
        return;
    if (seq > list->n)
        seq = list->n;
    memmove(list->o + seq + 1, list->o + seq, list->n - seq);
    list->o[seq] = (uint8_t)o;
    list->n++;
}
static void Unplace(struct YgoEdoDuel *e, struct Loc l)
{
    int li;
    if (l.con > 1)
        return;
    if (l.loc & EDO_LOC_OVERLAY)
    {
        if (l.seq < 7)
            ListRemove(&e->overlay[l.con][l.seq], l.pos);
    }
    else if (l.loc == EDO_LOC_MZONE)
    {
        if (l.seq < 7)
            e->mz[l.con][l.seq] = 0;
    }
    else if (l.loc == EDO_LOC_SZONE)
    {
        if (l.seq < 8)
            e->sz[l.con][l.seq] = 0;
    }
    else if ((li = ListOf(l.loc)) >= 0)
        ListRemove(&e->lists[l.con][li], l.seq);
}
static unsigned PosFlags(unsigned pos, uint8_t *faceUp)
{
    *faceUp = (pos & (EDO_POS_FACEUP_ATTACK | EDO_POS_FACEUP_DEFENSE)) != 0;
    return (pos & (EDO_POS_FACEUP_ATTACK | EDO_POS_FACEDOWN_ATTACK)) != 0;
}
static void Place(struct YgoDuel *d, unsigned o, struct Loc l)
{
    struct YgoEdoDuel *e = d->edo;
    struct YgoObj *ob = &d->s.objs[o];
    int li;
    if (l.con > 1 || !l.loc)
    {
        ob->location = YGO_LOC_NONE;
        return;
    }
    if (l.loc & EDO_LOC_OVERLAY)
    {
        if (l.seq < 7)
            ListInsert(&e->overlay[l.con][l.seq], l.pos, o);
        ob->location = YGO_LOC_NONE;
        ob->controller = (uint8_t)GP(e, l.con);
        return;
    }
    if (l.loc == EDO_LOC_MZONE && l.seq < 7)
        e->mz[l.con][l.seq] = (uint8_t)(o + 1);
    else if (l.loc == EDO_LOC_SZONE && l.seq < 8)
        e->sz[l.con][l.seq] = (uint8_t)(o + 1);
    else if ((li = ListOf(l.loc)) >= 0)
        ListInsert(&e->lists[l.con][li], l.seq, o);
    if (ob->location != YgoLoc(l.loc))
        ob->enteredTurn = d->s.turn;
    ob->location = (uint8_t)YgoLoc(l.loc);
    ob->controller = (uint8_t)GP(e, l.con);
    ob->seq = (uint8_t)l.seq;
    if (l.loc & (EDO_LOC_MZONE | EDO_LOC_SZONE))
        ob->attackPos = (uint8_t)PosFlags(l.pos, &ob->faceUp);
    else
    {
        ob->faceUp = l.loc != EDO_LOC_DECK && l.loc != EDO_LOC_EXTRA && l.loc != EDO_LOC_HAND;
        ob->attackPos = 1;
    }
    if (!(l.loc & (EDO_LOC_MZONE | EDO_LOC_SZONE)))
        ob->equipTo = 0, ob->counters = 0, ob->counterType = 0, ob->negated = 0, ob->summonType = 0;
}
static int NewObj(struct YgoDuel *d, uint32_t code, unsigned owner)
{
    unsigned o;
    if (d->s.objCount < YGO_OBJS)
        o = d->s.objCount++;
    else
    {
        /* every object used: a token that left the field takes the place of its kind */
        for (o = 0; o < YGO_OBJS; o++)
            if (d->s.objs[o].location == YGO_LOC_NONE && Ygo_Card(d->s.objs[o].card) &&
                Ygo_Card(d->s.objs[o].card)->frame == YGO_TOKEN)
                break;
        if (o == YGO_OBJS)
            return -1;
    }
    memset(&d->s.objs[o], 0, sizeof(d->s.objs[o]));
    d->s.objs[o].card = (uint16_t)YgoEdo_IdOfCode(code);
    d->s.objs[o].owner = (uint8_t)owner;
    d->s.objs[o].controller = (uint8_t)owner;
    d->edo->onField[o] = 0;
    return (int)o;
}
static void SetCode(struct YgoDuel *d, unsigned o, uint32_t code)
{
    unsigned id;
    if (code && (id = YgoEdo_IdOfCode(code)))
        d->s.objs[o].card = (uint16_t)id;
}

/* ------------------------------------------------------------------ */
/* Events                                                               */
/* ------------------------------------------------------------------ */
static void Event(struct YgoDuel *d, unsigned type, unsigned player, int obj, int target, int32_t value,
                  unsigned flags)
{
    struct YgoEvent *ev = &d->s.events[d->s.eventSeq % YGO_EVENT_RING];
    memset(ev, 0, sizeof(*ev));
    ev->type = (uint8_t)type;
    ev->player = (uint8_t)player;
    ev->obj = obj < 0 ? NONE : (uint8_t)obj;
    ev->target = target < 0 ? NONE : (uint8_t)target;
    ev->value = value;
    ev->flags = (uint16_t)flags;
    ev->card = obj >= 0 && obj < d->s.objCount ? d->s.objs[obj].card : 0;
    d->s.eventSeq++;
}
static unsigned Reason(uint32_t r)
{
    return (r & 0x20 ? YGO_REASON_BATTLE : 0) | (r & 0x40 ? YGO_REASON_EFFECT : 0) | (r & 0x80 ? YGO_REASON_COST : 0) |
           (r & 0x2 ? YGO_REASON_TRIBUTE : 0) | (r & 0x8 ? YGO_REASON_MATERIAL : 0) |
           (r & 0x1 ? YGO_REASON_DESTROY : 0) | (r & 0x400 ? YGO_REASON_RULE : 0) |
           (r & 0x4000 ? YGO_REASON_DISCARD : 0);
}
static int TakePending(struct YgoEdoDuel *e, int o, uint8_t *from)
{
    unsigned i;
    for (i = 0; i < e->pendingCount; i++)
        if (e->pending[i] == o)
        {
            if (from)
                *from = e->pendingFrom[i];
            memmove(e->pending + i, e->pending + i + 1, e->pendingCount - i - 1);
            memmove(e->pendingFrom + i, e->pendingFrom + i + 1, e->pendingCount - i - 1);
            e->pendingCount--;
            return 1;
        }
    return 0;
}
static void FlushPending(struct YgoDuel *d)
{
    struct YgoEdoDuel *e = d->edo;
    unsigned i;
    for (i = 0; i < e->pendingCount; i++)
        Event(d, YGO_EV_MOVE, d->s.objs[e->pending[i]].controller, e->pending[i], -1, 0, 0);
    e->pendingCount = 0;
}

/* ------------------------------------------------------------------ */
/* The state, read back from the engine                                 */
/* ------------------------------------------------------------------ */
struct QCard
{
    int present;
    uint32_t code, pos, owner, status, level, type;
    int32_t atk, def;
    struct Loc equip;
    int hasEquip;
    uint32_t counters, counterType;
};
/* One card of a location query; returns the bytes it took */
static const uint8_t *QueryCard(const uint8_t *p, const uint8_t *end, struct QCard *c)
{
    memset(c, 0, sizeof(*c));
    if (p + 2 > end)
        return end;
    if (!(p[0] | p[1] << 8))
        return p + 2;   /* an empty zone */
    c->present = 1;
    while (p + 6 <= end)
    {
        unsigned size = p[0] | p[1] << 8;
        uint32_t flag = p[2] | p[3] << 8 | p[4] << 16 | (uint32_t)p[5] << 24;
        const uint8_t *v = p + 6;
        struct Rd r;
        r.p = v;
        r.end = p + 2 + size;
        r.bad = 0;
        if (size < 4 || p + 2 + size > end)
            return end;
        switch (flag)
        {
        case QUERY_CODE: c->code = R32(&r); break;
        case QUERY_POSITION: c->pos = R32(&r); break;
        case QUERY_LEVEL: c->level = R32(&r); break;
        case QUERY_TYPE: c->type = R32(&r); break;
        case QUERY_ATTACK: c->atk = (int32_t)R32(&r); break;
        case QUERY_DEFENSE: c->def = (int32_t)R32(&r); break;
        case QUERY_OWNER: c->owner = R8(&r); break;
        case QUERY_STATUS: c->status = R32(&r); break;
        case QUERY_EQUIP_CARD:
            if (size >= 4 + 10)
            {
                c->equip = RLoc(&r);
                c->hasEquip = c->equip.loc != 0;
            }
            break;
        case QUERY_COUNTERS:
        {
            uint32_t n = R32(&r), i;
            for (i = 0; i < n && !r.bad; i++)
            {
                uint32_t v = R32(&r);
                c->counters += v >> 16;
                if ((v >> 16) && !c->counterType)
                    c->counterType = v & 0xffff;   /* (the first kind it holds) */
            }
            break;
        }
        default:
            break;
        }
        p += 2 + size;
        if (flag == QUERY_END)
            break;
    }
    return p;
}
#define QFLAGS (QUERY_CODE | QUERY_POSITION | QUERY_LEVEL | QUERY_TYPE | QUERY_ATTACK | QUERY_DEFENSE | QUERY_OWNER | \
                QUERY_STATUS | QUERY_EQUIP_CARD | QUERY_COUNTERS)

static void Rebuild(struct YgoDuel *d);
unsigned gYgoEdoRebuilds;   /* (tests: times the objects had to be put back from the engine) */
unsigned gYgoEdoRetries;     /* (tests: answers the engine refused: none should be) */
int gYgoEdoNoShuffle;        /* (tests: the Decks stay in the order given, the last card on top) */
/* Reads every location back; 0 when what the engine has isn't what the messages said (then Rebuild) */
static int Sync(struct YgoDuel *d, int fix)
{
    static const uint8_t sLocs[] = {EDO_LOC_DECK, EDO_LOC_HAND, EDO_LOC_MZONE, EDO_LOC_SZONE, EDO_LOC_GRAVE,
                                    EDO_LOC_REMOVED, EDO_LOC_EXTRA};
    struct YgoEdoDuel *e = d->edo;
    unsigned cp, li, i;
    int ok = 1;
    for (cp = 0; cp < 2; cp++)
        for (li = 0; li < sizeof(sLocs); li++)
        {
            OCG_QueryInfo q;
            uint32_t len = 0, total;
            const uint8_t *p, *end;
            unsigned seq = 0;
            memset(&q, 0, sizeof(q));
            q.flags = QFLAGS;
            q.con = (uint8_t)cp;
            q.loc = sLocs[li];
            p = OCG_DuelQueryLocation(e->duel, &len, &q);
            if (!p || len < 4)
                continue;
            total = p[0] | p[1] << 8 | p[2] << 16 | (uint32_t)p[3] << 24;
            end = p + 4 + total;
            if (end > p + len)
                end = p + len;
            p += 4;
            while (p < end)
            {
                struct QCard c;
                struct Loc l;
                int o;
                p = QueryCard(p, end, &c);
                l.con = (uint8_t)cp;
                l.loc = sLocs[li];
                l.seq = seq++;
                l.pos = c.pos;
                o = ObjAt(e, l);
                if (!c.present)
                {
                    if (o >= 0)
                        ok = 0;
                    continue;
                }
                if (o < 0)
                {
                    ok = 0;
                    continue;
                }
                if (!fix)
                    continue;
                {
                    struct YgoObj *ob = &d->s.objs[o];
                    SetCode(d, (unsigned)o, c.code);
                    ob->owner = (uint8_t)GP(e, c.owner);
                    ob->controller = (uint8_t)GP(e, cp);
                    ob->location = (uint8_t)YgoLoc(l.loc);
                    ob->seq = (uint8_t)l.seq;
                    if (l.loc & (EDO_LOC_MZONE | EDO_LOC_SZONE))
                        ob->attackPos = (uint8_t)PosFlags(c.pos, &ob->faceUp);
                    ob->atk = c.atk;
                    ob->def = c.def;
                    ob->level = (uint8_t)c.level;
                    ob->negated = (c.status & STATUS_DISABLED) != 0;
                    ob->counters = (uint8_t)(c.counters > 255 ? 255 : c.counters);
                    ob->counterType = (uint16_t)(c.counters ? c.counterType : 0);
                    ob->equipTo = 0;
                    if (c.hasEquip)
                    {
                        int t = ObjAt(e, c.equip);
                        ob->equipTo = t >= 0 ? (uint8_t)(t + 1) : 0;
                    }
                }
            }
            if (ListOf(sLocs[li]) >= 0 && seq != e->lists[cp][ListOf(sLocs[li])].n)
                ok = 0;
        }
    /* LP */
    {
        uint32_t len = 0;
        const uint8_t *f = OCG_DuelQueryField(e->duel, &len);
        if (f && len >= 8 && fix)
        {
            /* options u32, then per player lp u32 and its zones */
            struct Rd r;
            unsigned p, z;
            r.p = f;
            r.end = f + len;
            r.bad = 0;
            R32(&r);
            for (p = 0; p < 2 && !r.bad; p++)
            {
                d->s.players[GP(e, p)].lp = (int32_t)R32(&r);
                for (z = 0; z < 7 + 8 && !r.bad; z++)
                    if (R8(&r))
                    {
                        R8(&r);
                        R32(&r);
                    }
                for (z = 0; z < 6; z++)
                    R32(&r);
            }
        }
    }
    (void)i;
    return ok;
}
/* Every object put where the engine has a card, keeping each where it was when the code matches */
static void Rebuild(struct YgoDuel *d)
{
    static const uint8_t sLocs[] = {EDO_LOC_DECK, EDO_LOC_HAND, EDO_LOC_MZONE, EDO_LOC_SZONE, EDO_LOC_GRAVE,
                                    EDO_LOC_REMOVED, EDO_LOC_EXTRA};
    struct YgoEdoDuel *e = d->edo;
    uint8_t used[YGO_OBJS];
    struct { uint8_t con, loc; uint16_t seq; uint32_t code; } want[YGO_OBJS * 2];
    unsigned wantCount = 0, cp, li, i, o;
    struct YgoState old = d->s;
    gYgoEdoRebuilds++;
    printf("[ygo] edo: lost track of the cards, rebuilding\n");
    for (cp = 0; cp < 2; cp++)
        for (li = 0; li < sizeof(sLocs); li++)
        {
            OCG_QueryInfo q;
            uint32_t len = 0, total, seq = 0;
            const uint8_t *p, *end;
            memset(&q, 0, sizeof(q));
            q.flags = QUERY_CODE;
            q.con = (uint8_t)cp;
            q.loc = sLocs[li];
            p = OCG_DuelQueryLocation(e->duel, &len, &q);
            if (!p || len < 4)
                continue;
            total = p[0] | p[1] << 8 | p[2] << 16 | (uint32_t)p[3] << 24;
            end = p + 4 + total;
            if (end > p + len)
                end = p + len;
            p += 4;
            while (p < end)
            {
                struct QCard c;
                p = QueryCard(p, end, &c);
                if (c.present && wantCount < sizeof(want) / sizeof(want[0]))
                {
                    want[wantCount].con = (uint8_t)cp;
                    want[wantCount].loc = sLocs[li];
                    want[wantCount].seq = (uint16_t)seq;
                    want[wantCount].code = c.code;
                    wantCount++;
                }
                seq++;
            }
        }
    memset(e->lists, 0, sizeof(e->lists));
    memset(e->mz, 0, sizeof(e->mz));
    memset(e->sz, 0, sizeof(e->sz));
    memset(used, 0, sizeof(used));
    for (o = 0; o < d->s.objCount; o++)
        d->s.objs[o].location = YGO_LOC_NONE;
    /* (overlays are kept) */
    for (cp = 0; cp < 2; cp++)
        for (i = 0; i < 7; i++)
            for (o = 0; o < e->overlay[cp][i].n; o++)
                used[e->overlay[cp][i].o[o]] = 1;
    for (i = 0; i < wantCount; i++)
    {
        struct Loc l;
        unsigned id = YgoEdo_IdOfCode(want[i].code), pick = YGO_OBJS;
        l.con = want[i].con;
        l.loc = want[i].loc;
        l.seq = want[i].seq;
        l.pos = EDO_POS_FACEDOWN_DEFENSE;
        /* the same place, else the same card, else any free object */
        for (o = 0; o < old.objCount && pick == YGO_OBJS; o++)
            if (!used[o] && old.objs[o].location == YgoLoc(l.loc) && old.objs[o].controller == GP(e, l.con) &&
                old.objs[o].seq == l.seq && old.objs[o].card == id)
                pick = o;
        for (o = 0; o < old.objCount && pick == YGO_OBJS; o++)
            if (!used[o] && old.objs[o].card == id)
                pick = o;
        for (o = 0; o < old.objCount && pick == YGO_OBJS; o++)
            if (!used[o])
                pick = o;
        if (pick == YGO_OBJS)
        {
            int n = NewObj(d, want[i].code, GP(e, l.con));
            if (n < 0)
                continue;
            pick = (unsigned)n;
        }
        used[pick] = 1;
        d->s.objs[pick].card = (uint16_t)id;
        Place(d, pick, l);
        if (ListOf(l.loc) >= 0)
        {
            /* (ListInsert at seq: in order already) */
        }
    }
    Sync(d, 1);
}
/* The game's per-player lists from the objects' places */
static void Mirror(struct YgoDuel *d)
{
    struct YgoEdoDuel *e = d->edo;
    unsigned cp, i;
    for (cp = 0; cp < 2; cp++)
    {
        struct YgoPlayer *pl = &d->s.players[GP(e, cp)];
        const struct List *l = e->lists[cp];
        pl->deckCount = (uint8_t)(l[LI_DECK].n < sizeof(pl->deck) ? l[LI_DECK].n : sizeof(pl->deck));
        memcpy(pl->deck, l[LI_DECK].o, pl->deckCount);
        pl->handCount = (uint8_t)(l[LI_HAND].n < sizeof(pl->hand) ? l[LI_HAND].n : sizeof(pl->hand));
        memcpy(pl->hand, l[LI_HAND].o, pl->handCount);
        pl->graveCount = (uint8_t)(l[LI_GRAVE].n < sizeof(pl->grave) ? l[LI_GRAVE].n : sizeof(pl->grave));
        memcpy(pl->grave, l[LI_GRAVE].o, pl->graveCount);
        pl->banishedCount = (uint8_t)(l[LI_REMOVED].n < sizeof(pl->banished) ? l[LI_REMOVED].n : sizeof(pl->banished));
        memcpy(pl->banished, l[LI_REMOVED].o, pl->banishedCount);
        pl->extraCount = (uint8_t)(l[LI_EXTRA].n < sizeof(pl->extra) ? l[LI_EXTRA].n : sizeof(pl->extra));
        memcpy(pl->extra, l[LI_EXTRA].o, pl->extraCount);
        for (i = 0; i < YGO_ZONES; i++)
            pl->monsters[i] = e->mz[cp][i];
        for (i = 0; i < YGO_ZONES + 1; i++)
            pl->spells[i] = e->sz[cp][i];
    }
}
/* After the engine stopped: the state, and the changes to ATK the messages don't tell */
static void Settle(struct YgoDuel *d)
{
    struct YgoEdoDuel *e = d->edo;
    unsigned o;
    FlushPending(d);
    if (!Sync(d, 1) || e->needRebuild)
        Rebuild(d);
    e->needRebuild = 0;
    Mirror(d);
    for (o = 0; o < d->s.objCount; o++)
    {
        const struct YgoObj *ob = &d->s.objs[o];
        int on = ob->location == YGO_LOC_MZONE && ob->faceUp;
        if (on && e->onField[o] && ob->atk != e->shownAtk[o])
            Event(d, YGO_EV_STAT, ob->controller, (int)o, -1, ob->atk - e->shownAtk[o], 0);
        e->onField[o] = (uint8_t)on;
        e->shownAtk[o] = ob->atk;
    }
}

/* ------------------------------------------------------------------ */
/* Messages                                                             */
/* ------------------------------------------------------------------ */
static unsigned PhaseOf(unsigned p)
{
    if (p & 0x1)
        return YGO_PHASE_DRAW;
    if (p & 0x2)
        return YGO_PHASE_STANDBY;
    if (p & 0x4)
        return YGO_PHASE_MAIN1;
    if (p & 0xF8)
        return YGO_PHASE_BATTLE;
    if (p & 0x100)
        return YGO_PHASE_MAIN2;
    return YGO_PHASE_END;
}
static void MsgMove(struct YgoDuel *d, struct Rd *r)
{
    struct YgoEdoDuel *e = d->edo;
    uint32_t code = R32(r), reason;
    struct Loc from = RLoc(r), to = RLoc(r);
    int o;
    unsigned toLoc = YgoLoc(to.loc), wasLoc, wasCon;
    reason = R32(r);
    if (r->bad)
        return;
    o = from.loc ? ObjAt(e, from) : -1;
    if (o < 0)
    {
        if (from.loc)
            e->needRebuild = 1;
        o = NewObj(d, code, GP(e, to.con <= 1 ? to.con : from.con));
        if (o < 0)
            return;
    }
    else
        Unplace(e, from);
    wasLoc = d->s.objs[o].location;
    wasCon = d->s.objs[o].controller;
    SetCode(d, (unsigned)o, code);
    if (!to.loc)
    {
        /* a token gone */
        if (reason & 0x1)
            Event(d, YGO_EV_DESTROY, wasCon, o, -1, 0, Reason(reason));
        d->s.objs[o].location = YGO_LOC_NONE;
        Event(d, YGO_EV_MOVE, wasCon, o, -1, 0, 0);
        return;
    }
    Place(d, (unsigned)o, to);
    d->s.objs[o].reason = (uint8_t)Reason(reason);
    if ((reason & 0x12) == 0x12)
        e->tributes++;
    if (to.loc & EDO_LOC_OVERLAY)
    {
        Event(d, YGO_EV_MOVE, d->s.objs[o].controller, o, -1, 0, 0);
        return;
    }
    if ((reason & 0x1) && toLoc != YGO_LOC_MZONE && toLoc != YGO_LOC_SZONE)
        Event(d, YGO_EV_DESTROY, wasCon, o, -1, 0, Reason(reason));
    switch (toLoc)
    {
    case YGO_LOC_GRAVE:
        Event(d, YGO_EV_TO_GRAVE, d->s.objs[o].owner, o, -1, 0, Reason(reason));
        break;
    case YGO_LOC_BANISHED:
        Event(d, YGO_EV_BANISH, d->s.objs[o].owner, o, -1, 0, Reason(reason));
        break;
    case YGO_LOC_HAND:
        Event(d, YGO_EV_TO_HAND, d->s.objs[o].controller, o, -1, 0, Reason(reason));
        break;
    case YGO_LOC_DECK:
    case YGO_LOC_EXTRA:
        Event(d, YGO_EV_TO_DECK, d->s.objs[o].owner, o, -1, 0, Reason(reason));
        break;
    default:
        if (wasLoc == toLoc && wasCon != d->s.objs[o].controller)
            Event(d, YGO_EV_CONTROL, d->s.objs[o].controller, o, -1, 0, 0);
        else if (e->pendingCount < PENDING_MAX)
        {
            e->pending[e->pendingCount] = (uint8_t)o;
            e->pendingFrom[e->pendingCount++] = (uint8_t)wasLoc;
        }
        else
            Event(d, YGO_EV_MOVE, d->s.objs[o].controller, o, -1, 0, 0);
        break;
    }
}
static void Reorder(struct YgoDuel *d, struct List *list, struct Rd *r, unsigned count)
{
    /* the list's cards are now these codes, in this order: the objects follow their cards */
    uint8_t old[YGO_OBJS], taken[YGO_OBJS];
    unsigned n = list->n, i, j;
    memcpy(old, list->o, n);
    memset(taken, 0, sizeof(taken));
    for (i = 0; i < count && i < n; i++)
    {
        uint32_t code = R32(r);
        unsigned id = YgoEdo_IdOfCode(code), pick = n;
        for (j = 0; j < n && pick == n; j++)
            if (!taken[j] && d->s.objs[old[j]].card == id)
                pick = j;
        for (j = 0; j < n && pick == n; j++)
            if (!taken[j])
                pick = j;
        taken[pick] = 1;
        list->o[i] = old[pick];
        d->s.objs[old[pick]].seq = (uint8_t)i;
        SetCode(d, old[pick], code);
    }
}
/* One message from the engine; a question is kept for Ask */
static void Message(struct YgoDuel *d, unsigned type, const uint8_t *data, uint32_t len)
{
    struct YgoEdoDuel *e = d->edo;
    struct YgoState *s = &d->s;
    struct Rd r;
    r.p = data;
    r.end = data + len;
    r.bad = 0;
    /* a move onto the field shows by itself unless what comes next is its summon, set or activation */
    switch (type)
    {
    case MSG_MOVE: case MSG_HINT: case MSG_CARD_HINT: case MSG_SUMMONING: case MSG_SPSUMMONING: case MSG_FLIPSUMMONING:
    case MSG_SET: case MSG_CHAINING: case MSG_EQUIP: case MSG_POS_CHANGE: case MSG_SHUFFLE_SET_CARD: case MSG_FIELD_DISABLED:
        break;
    default:
        FlushPending(d);
        break;
    }
    if (type >= MSG_SELECT_BATTLECMD && type <= MSG_SELECT_UNSELECT_CARD)
    {
        if (len + 1 <= SEL_MAX)
        {
            e->sel[0] = (uint8_t)type;
            memcpy(e->sel + 1, data, len);
            e->selLen = len + 1;
        }
        return;
    }
    switch (type)
    {
    case MSG_ANNOUNCE_RACE: case MSG_ANNOUNCE_ATTRIB: case MSG_ANNOUNCE_CARD: case MSG_ANNOUNCE_NUMBER:
    case MSG_ROCK_PAPER_SCISSORS:
        if (len + 1 <= SEL_MAX)
        {
            e->sel[0] = (uint8_t)type;
            memcpy(e->sel + 1, data, len);
            e->selLen = len + 1;
        }
        return;
    case MSG_RETRY:
        gYgoEdoRetries++;
        printf("[ygo] edo: the engine refused an answer (question %u, kind %u, prompt %u, %u items, min %u max %u)\n",
               d->query.type, e->kind, d->query.prompt, d->query.count, d->query.min, d->query.max);
        return;
    case MSG_HINT:
    {
        unsigned ht = R8(&r), pl = R8(&r);
        uint64_t desc = R64(&r);
        if (ht == 3)        /* HINT_SELECTMSG */
            e->hint = desc;
        else if (ht == 8)   /* HINT_CODE: a declared card */
        {
            Event(d, YGO_EV_DECLARE, GP(e, pl), -1, -1, 0, 1);
            s->events[(s->eventSeq - 1) % YGO_EVENT_RING].card = (uint16_t)YgoEdo_IdOfCode((uint32_t)desc);
        }
        else if (ht == 9)
            Event(d, YGO_EV_DECLARE, GP(e, pl), -1, -1, (int32_t)desc, 2);
        else if (ht == 6)
            Event(d, YGO_EV_DECLARE, GP(e, pl), -1, -1, (int32_t)desc, 3);
        else if (ht == 7)
            Event(d, YGO_EV_DECLARE, GP(e, pl), -1, -1, (int32_t)desc, 4);
        return;
    }
    case MSG_WIN:
    {
        unsigned w = R8(&r), why = R8(&r);
        s->over = 1;
        s->winner = (uint8_t)(w > 1 ? NONE : GP(e, w));
        s->winReason = why == 0x1 ? YGO_WIN_LP : why == 0x2 ? YGO_WIN_DECK_OUT : why == 0x10 ? YGO_WIN_EXODIA
                     : why == 0x0 ? YGO_WIN_CONCEDE : YGO_WIN_EFFECT;
        Event(d, YGO_EV_WIN, s->winner, -1, -1, s->winReason, 0);
        return;
    }
    case MSG_NEW_TURN:
    {
        unsigned pl = R8(&r);
        s->turn++;
        s->current = (uint8_t)GP(e, pl);
        s->phase = YGO_PHASE_DRAW;
        s->players[0].normalSummoned = s->players[1].normalSummoned = 0;
        {
            unsigned o;
            for (o = 0; o < s->objCount; o++)
                s->objs[o].attacked = 0;
        }
        e->quiet = 1;
        Event(d, YGO_EV_TURN, s->current, -1, -1, s->turn, 0);
        return;
    }
    case MSG_NEW_PHASE:
    {
        unsigned ph = PhaseOf(R16(&r));
        e->quiet = 1;
        s->attacker = s->attackTarget = 0;
        if (ph != s->phase || ph != YGO_PHASE_BATTLE)
        {
            s->phase = (uint8_t)ph;
            Event(d, YGO_EV_PHASE, s->current, -1, -1, ph, 0);
        }
        return;
    }
    case MSG_DRAW:
    {
        unsigned cp = R8(&r), n = R32(&r), i;
        for (i = 0; i < n && !r.bad && cp <= 1; i++)
        {
            uint32_t code = R32(&r);
            struct List *deck = &e->lists[cp][LI_DECK];
            struct Loc to;
            int o;
            R32(&r);   /* (position) */
            if (!deck->n)
            {
                e->needRebuild = 1;
                break;
            }
            o = deck->o[deck->n - 1];
            deck->n--;
            to.con = (uint8_t)cp;
            to.loc = EDO_LOC_HAND;
            to.seq = e->lists[cp][LI_HAND].n;
            to.pos = 0;
            SetCode(d, (unsigned)o, code);
            Place(d, (unsigned)o, to);
            Event(d, YGO_EV_DRAW, GP(e, cp), o, -1, 0, 0);
        }
        e->quiet = 0;
        return;
    }
    case MSG_MOVE:
        MsgMove(d, &r);
        e->quiet = 0;
        return;
    case MSG_SHUFFLE_HAND:
    case MSG_SHUFFLE_EXTRA:
    {
        unsigned cp = R8(&r), n = R32(&r);
        if (cp <= 1)
            Reorder(d, &e->lists[cp][type == MSG_SHUFFLE_HAND ? LI_HAND : LI_EXTRA], &r, n);
        return;
    }
    case MSG_SHUFFLE_DECK:
    {
        unsigned cp = R8(&r);
        Event(d, YGO_EV_SHUFFLE, GP(e, cp), -1, -1, 0, 0);
        return;
    }
    case MSG_SWAP_GRAVE_DECK: case MSG_REVERSE_DECK: case MSG_RELOAD_FIELD: case MSG_TAG_SWAP:
        e->needRebuild = 1;
        return;
    case MSG_SHUFFLE_SET_CARD:
        /* face-down cards trade places: their objects stay, their codes are read back */
        return;
    case MSG_SWAP:
    {
        uint32_t c1 = R32(&r);
        struct Loc l1 = RLoc(&r);
        uint32_t c2 = R32(&r);
        struct Loc l2 = RLoc(&r);
        int o1 = ObjAt(e, l1), o2 = ObjAt(e, l2);
        if (o1 < 0 || o2 < 0)
        {
            e->needRebuild = 1;
            return;
        }
        Unplace(e, l1);
        Unplace(e, l2);
        l1.pos = l2.pos = EDO_POS_FACEUP_ATTACK;
        SetCode(d, (unsigned)o1, c1);
        SetCode(d, (unsigned)o2, c2);
        {
            uint8_t up1 = s->objs[o1].faceUp, ap1 = s->objs[o1].attackPos;
            uint8_t up2 = s->objs[o2].faceUp, ap2 = s->objs[o2].attackPos;
            Place(d, (unsigned)o1, l2);
            Place(d, (unsigned)o2, l1);
            s->objs[o1].faceUp = up1, s->objs[o1].attackPos = ap1;
            s->objs[o2].faceUp = up2, s->objs[o2].attackPos = ap2;
        }
        Event(d, YGO_EV_CONTROL, s->objs[o1].controller, o1, -1, 0, 0);
        Event(d, YGO_EV_CONTROL, s->objs[o2].controller, o2, -1, 0, 0);
        e->quiet = 0;
        return;
    }
    case MSG_POS_CHANGE:
    {
        uint32_t code = R32(&r);
        struct Loc l = RLoc3(&r, 1);
        unsigned prev = R8(&r), cur = R8(&r);
        int o = ObjAt(e, l);
        uint8_t up, wasUp;
        unsigned atk, wasAtk;
        if (o < 0)
            return;
        SetCode(d, (unsigned)o, code);
        atk = PosFlags(cur, &up);
        wasAtk = PosFlags(prev, &wasUp);
        s->objs[o].faceUp = up;
        s->objs[o].attackPos = (uint8_t)atk;
        if (TakePending(e, o, NULL))
        {
            Event(d, YGO_EV_MOVE, s->objs[o].controller, o, -1, 0, 0);
            return;
        }
        if (!wasUp && up)
            Event(d, YGO_EV_FLIP, s->objs[o].controller, o, -1, 0, 0);
        if (atk != wasAtk)
            Event(d, YGO_EV_POSITION, s->objs[o].controller, o, -1, (int32_t)atk, 0);
        e->quiet = 0;
        return;
    }
    case MSG_SET:
    {
        uint32_t code = R32(&r);
        struct Loc l = RLoc(&r);
        int o = ObjAt(e, l);
        if (o < 0)
            return;
        SetCode(d, (unsigned)o, code);
        TakePending(e, o, NULL);
        Event(d, YGO_EV_SET, s->objs[o].controller, o, -1, 0, 0);
        e->quiet = 0;
        return;
    }
    case MSG_SUMMONING:
    case MSG_SPSUMMONING:
    case MSG_FLIPSUMMONING:
    {
        uint32_t code = R32(&r);
        struct Loc l = RLoc(&r);
        int o = ObjAt(e, l);
        uint8_t from = YGO_LOC_NONE;
        unsigned how;
        const struct YgoCard *c;
        if (o < 0)
            return;
        SetCode(d, (unsigned)o, code);
        TakePending(e, o, &from);
        c = Ygo_Card(s->objs[o].card);
        if (type == MSG_SUMMONING)
        {
            how = e->tributes ? YGO_SUMMON_TRIBUTE : YGO_SUMMON_NORMAL;
            s->players[s->objs[o].controller].normalSummoned = 1;
        }
        else if (type == MSG_FLIPSUMMONING)
            how = YGO_SUMMON_FLIP;
        else if (c && c->frame == YGO_FUSION && from == YGO_LOC_EXTRA)
            how = YGO_SUMMON_FUSION;
        else if (c && c->frame == YGO_RITUAL && from == YGO_LOC_HAND)
            how = YGO_SUMMON_RITUAL;
        else
            how = YGO_SUMMON_SPECIAL;
        e->tributes = 0;
        s->objs[o].summonType = (uint8_t)how;
        if (type == MSG_FLIPSUMMONING)
            s->objs[o].faceUp = 1, s->objs[o].attackPos = 1;
        else
            s->objs[o].faceUp = (l.pos & (EDO_POS_FACEUP_ATTACK | EDO_POS_FACEUP_DEFENSE)) != 0;
        Event(d, YGO_EV_SUMMON, s->objs[o].controller, o, -1, (int32_t)how, 0);
        e->quiet = 0;
        return;
    }
    case MSG_CHAINING:
    {
        uint32_t code = R32(&r);
        struct Loc l = RLoc(&r);
        uint64_t desc;
        unsigned tcon, n;
        int o;
        tcon = R8(&r);
        R8(&r);
        R32(&r);
        desc = R64(&r);
        n = R32(&r);
        o = ObjAt(e, l);
        if (o >= 0)
        {
            SetCode(d, (unsigned)o, code);
            TakePending(e, o, NULL);
            if (l.loc & (EDO_LOC_MZONE | EDO_LOC_SZONE))
                PosFlags(l.pos, &s->objs[o].faceUp);
        }
        if (n >= 1 && n <= YGO_CHAIN_MAX)
        {
            struct YgoLink *k = &s->chain[n - 1];
            k->obj = o < 0 ? NONE : (uint8_t)o;
            k->player = (uint8_t)GP(e, tcon);
            k->card = (uint16_t)YgoEdo_IdOfCode(code);
            k->desc = desc;
            k->negated = 0;
            s->chainCount = (uint8_t)n;
        }
        Event(d, YGO_EV_ACTIVATE, GP(e, tcon), o, -1, (int32_t)n, 0);
        if (o < 0)
            s->events[(s->eventSeq - 1) % YGO_EVENT_RING].card = (uint16_t)YgoEdo_IdOfCode(code);
        e->quiet = 0;
        return;
    }
    case MSG_CHAIN_SOLVING:
    {
        unsigned n = R8(&r);
        if (n >= 1 && n <= s->chainCount)
            Event(d, YGO_EV_RESOLVE, s->chain[n - 1].player, s->chain[n - 1].obj == NONE ? -1 : s->chain[n - 1].obj,
                  -1, (int32_t)n, 0);
        return;
    }
    case MSG_CHAIN_NEGATED:
    case MSG_CHAIN_DISABLED:
    {
        unsigned n = R8(&r);
        if (n >= 1 && n <= s->chainCount)
        {
            s->chain[n - 1].negated = 1;
            Event(d, YGO_EV_NEGATE, s->chain[n - 1].player, s->chain[n - 1].obj == NONE ? -1 : s->chain[n - 1].obj,
                  -1, (int32_t)n, 0);
        }
        return;
    }
    case MSG_CHAIN_END:
        s->chainCount = 0;
        return;
    case MSG_ATTACK:
    {
        struct Loc a = RLoc(&r), t = RLoc(&r);
        int o = ObjAt(e, a), to = t.loc ? ObjAt(e, t) : -1;
        if (o < 0)
            return;
        s->attacker = (uint8_t)(o + 1);
        s->attackTarget = to < 0 ? 0 : (uint8_t)(to + 1);
        s->objs[o].attacked = 1;
        Event(d, YGO_EV_ATTACK, s->objs[o].controller, o, to, 0, 0);
        e->quiet = 0;
        return;
    }
    case MSG_BATTLE:
        Event(d, YGO_EV_BATTLE, s->current, s->attacker ? s->attacker - 1 : -1,
              s->attackTarget ? s->attackTarget - 1 : -1, 0, 0);
        return;
    case MSG_ATTACK_DISABLED:
        s->attacker = s->attackTarget = 0;
        return;
    case MSG_DAMAGE_STEP_START:
        e->inDamage = 1;
        return;
    case MSG_DAMAGE_STEP_END:
        e->inDamage = 0;
        return;
    case MSG_DAMAGE:
    case MSG_PAY_LPCOST:
    {
        unsigned cp = R8(&r);
        int32_t n = (int32_t)R32(&r);
        if (cp > 1)
            return;
        s->players[GP(e, cp)].lp -= n;
        if (s->players[GP(e, cp)].lp < 0)
            s->players[GP(e, cp)].lp = 0;
        Event(d, YGO_EV_DAMAGE, GP(e, cp), -1, -1, n, type == MSG_DAMAGE && e->inDamage);
        e->quiet = 0;
        return;
    }
    case MSG_RECOVER:
    {
        unsigned cp = R8(&r);
        int32_t n = (int32_t)R32(&r);
        if (cp > 1)
            return;
        s->players[GP(e, cp)].lp += n;
        Event(d, YGO_EV_RECOVER, GP(e, cp), -1, -1, n, 0);
        e->quiet = 0;
        return;
    }
    case MSG_LPUPDATE:
    {
        unsigned cp = R8(&r);
        int32_t lp = (int32_t)R32(&r), was;
        if (cp > 1)
            return;
        was = s->players[GP(e, cp)].lp;
        s->players[GP(e, cp)].lp = lp;
        if (lp < was)
            Event(d, YGO_EV_DAMAGE, GP(e, cp), -1, -1, was - lp, 0);
        else if (lp > was)
            Event(d, YGO_EV_RECOVER, GP(e, cp), -1, -1, lp - was, 0);
        return;
    }
    case MSG_EQUIP:
    {
        struct Loc a = RLoc(&r), t = RLoc(&r);
        int o = ObjAt(e, a), to = ObjAt(e, t);
        if (o < 0)
            return;
        TakePending(e, o, NULL);
        s->objs[o].equipTo = to < 0 ? 0 : (uint8_t)(to + 1);
        Event(d, YGO_EV_EQUIP, s->objs[o].controller, o, to, 0, 0);
        return;
    }
    case MSG_UNEQUIP:
    {
        int o = ObjAt(e, RLoc(&r));
        if (o >= 0)
            s->objs[o].equipTo = 0;
        return;
    }
    case MSG_CARD_TARGET:
    {
        struct Loc a = RLoc(&r), t = RLoc(&r);
        int o = ObjAt(e, a), to = ObjAt(e, t);
        if (to >= 0)
            Event(d, YGO_EV_TARGET, o >= 0 ? s->objs[o].controller : s->current, o, to, 0, 0);
        return;
    }
    case MSG_BECOME_TARGET:
    {
        unsigned n = R32(&r), i;
        int by = s->chainCount && s->chain[s->chainCount - 1].obj != NONE ? s->chain[s->chainCount - 1].obj : -1;
        for (i = 0; i < n && !r.bad; i++)
        {
            int to = ObjAt(e, RLoc(&r));
            if (to >= 0)
                Event(d, YGO_EV_TARGET, by >= 0 ? s->objs[by].controller : s->current, by, to, 0, 0);
        }
        return;
    }
    case MSG_TOSS_COIN:
    case MSG_TOSS_DICE:
    {
        unsigned cp = R8(&r), n = R8(&r), i;
        for (i = 0; i < n && !r.bad; i++)
            Event(d, type == MSG_TOSS_COIN ? YGO_EV_COIN : YGO_EV_DICE, GP(e, cp), -1, -1, R8(&r), 0);
        return;
    }
    case MSG_CONFIRM_CARDS:
    case MSG_CONFIRM_DECKTOP:
    case MSG_CONFIRM_EXTRATOP:
    {
        unsigned viewer = R8(&r), n = R32(&r), i;
        for (i = 0; i < n && !r.bad; i++)
        {
            uint32_t code = R32(&r);
            struct Loc l = RLoc3(&r, 0);
            int o = ObjAt(e, l);
            if (o < 0)
                continue;
            SetCode(d, (unsigned)o, code);
            if (type != MSG_CONFIRM_CARDS || s->objs[o].controller != GP(e, viewer))
                Event(d, YGO_EV_REVEAL, s->objs[o].controller, o, -1, 0, 0);
            else
                Event(d, YGO_EV_LOOK, GP(e, viewer), o, -1, 0, 0);
        }
        return;
    }
    case MSG_ADD_COUNTER:
    case MSG_REMOVE_COUNTER:
    {
        struct Loc l;
        unsigned n, kind;
        int o;
        kind = R16(&r);
        l = RLoc3(&r, 1);
        n = R16(&r);
        o = ObjAt(e, l);
        if (o < 0)
            return;
        if (type == MSG_ADD_COUNTER)
        {
            s->objs[o].counters = (uint8_t)(s->objs[o].counters + n > 255 ? 255 : s->objs[o].counters + n);
            s->objs[o].counterType = (uint16_t)kind;
        }
        else
            s->objs[o].counters = (uint8_t)(s->objs[o].counters > n ? s->objs[o].counters - n : 0);
        if (!s->objs[o].counters)
            s->objs[o].counterType = 0;
        /* (flags: the kind of counter) */
        Event(d, YGO_EV_COUNTER, s->objs[o].controller, o, -1, s->objs[o].counters, s->objs[o].counterType);
        return;
    }
    case MSG_REMOVE_CARDS:
    {
        unsigned n = R32(&r), i;
        for (i = 0; i < n && !r.bad; i++)
        {
            struct Loc l = RLoc(&r);
            int o = ObjAt(e, l);
            if (o < 0)
                continue;
            Unplace(e, l);
            s->objs[o].location = YGO_LOC_NONE;
            Event(d, YGO_EV_MOVE, s->objs[o].controller, o, -1, 0, 0);
        }
        return;
    }
    default:
        return;
    }
}

/* ------------------------------------------------------------------ */
/* Questions                                                            */
/* ------------------------------------------------------------------ */
static void Q(struct YgoDuel *d, unsigned type, unsigned corePlayer, unsigned prompt)
{
    memset(&d->query, 0, sizeof(d->query));
    d->query.type = (uint8_t)type;
    d->query.player = (uint8_t)GP(d->edo, corePlayer);
    d->query.prompt = (uint8_t)prompt;
    d->query.min = d->query.max = 1;
    d->query.desc = d->edo->hint;
}
static struct YgoChoice *Add(struct YgoDuel *d, unsigned kind, int obj, int32_t resp)
{
    struct YgoChoice *c;
    if (d->query.count >= YGO_QUERY_MAX)
        return NULL;
    d->edo->resp[d->query.count] = resp;
    c = &d->query.items[d->query.count++];
    memset(c, 0, sizeof(*c));
    c->kind = (uint8_t)kind;
    c->obj = obj < 0 ? NONE : (uint8_t)obj;
    c->card = obj >= 0 ? d->s.objs[obj].card : 0;
    return c;
}
static int ObjOf(struct YgoDuel *d, uint32_t code, struct Loc l)
{
    int o = ObjAt(d->edo, l);
    if (o >= 0)
        SetCode(d, (unsigned)o, code);
    return o;
}
static unsigned PromptOf(uint64_t hint)
{
    switch (hint)
    {
    case 500: case 531: return YGO_P_TRIBUTE;
    case 501: return YGO_P_DISCARD;
    case 502: return YGO_P_DESTROY;
    case 503: return YGO_P_BANISH;
    case 504: case 574: return YGO_P_TO_GRAVE;
    case 505: return YGO_P_TO_HAND;
    case 506: case 573: return YGO_P_ADD_TO_HAND;
    case 507: return YGO_P_TO_DECK;
    case 508: case 509: case 510: case 527: return YGO_P_SUMMON;
    case 511: case 512: case 513: case 533: return YGO_P_MATERIAL;
    case 518: return YGO_P_EQUIP;
    case 520: return YGO_P_CONTROL;
    case 528: return YGO_P_POSITION;
    case 549: return YGO_P_ATTACK_TARGET;
    case 551: case 575: case 576: case 577: return YGO_P_TARGET;
    case 552: return YGO_P_COIN_CALL;
    case 562: case 563: case 564: case 565: case 567: return YGO_P_DECLARE;
    default: return YGO_P_NONE;
    }
}
static int RespondInt(struct YgoDuel *d, int32_t v)
{
    OCG_DuelSetResponse(d->edo->duel, &v, 4);
    return 1;
}
/* RPN filters of a card name to declare (playerop.cpp is_declarable) */
static int Declarable(const struct YgoEdoRec *r, const uint64_t *ops, unsigned n)
{
    int64_t st[64];
    unsigned sp = 0, i;
    int alias = 0, token = 0;
    uint64_t race = r->race;
#define POP() (sp ? st[--sp] : 0)
#define PUSH(v) do { if (sp < 64) st[sp++] = (v); } while (0)
    for (i = 0; i < n; i++)
    {
        uint64_t op = ops[i];
        int64_t a, b;
        switch (op)
        {
        case 0x4000000000000000ULL: b = POP(); a = POP(); PUSH(a + b); break;
        case 0x4000000100000000ULL: b = POP(); a = POP(); PUSH(a - b); break;
        case 0x4000000200000000ULL: b = POP(); a = POP(); PUSH(a * b); break;
        case 0x4000000300000000ULL: b = POP(); a = POP(); PUSH(b ? a / b : 0); break;
        case 0x4000000400000000ULL: b = POP(); a = POP(); PUSH(a && b); break;
        case 0x4000000500000000ULL: b = POP(); a = POP(); PUSH(a || b); break;
        case 0x4000000600000000ULL: a = POP(); PUSH(-a); break;
        case 0x4000000700000000ULL: a = POP(); PUSH(!a); break;
        case 0x4000000800000000ULL: b = POP(); a = POP(); PUSH(a & b); break;
        case 0x4000000900000000ULL: b = POP(); a = POP(); PUSH(a | b); break;
        case 0x4000001000000000ULL: a = POP(); PUSH(~a); break;
        case 0x4000001100000000ULL: b = POP(); a = POP(); PUSH(a ^ b); break;
        case 0x4000001200000000ULL: b = POP(); a = POP(); PUSH(a << b); break;
        case 0x4000001300000000ULL: b = POP(); a = POP(); PUSH(a >> b); break;
        case 0x4000001400000000ULL: alias = 1; break;
        case 0x4000001500000000ULL: token = 1; break;
        case 0x4000010000000000ULL: a = POP(); PUSH(r->code == (uint32_t)a); break;
        case 0x4000010100000000ULL:
        {
            /* ISSETCARD */
            uint16_t set, sub;
            uint64_t sc = r->setcode;
            int res = 0;
            a = POP();
            set = (uint16_t)(a & 0xfff);
            sub = (uint16_t)(a & 0xf000);
            while (sc)
            {
                uint16_t c = (uint16_t)(sc & 0xffff);
                if ((c & 0xfff) == set && (c & 0xf000 & sub) == sub)
                    res = 1;
                sc >>= 16;
            }
            PUSH(res);
            break;
        }
        case 0x4000010200000000ULL: a = POP(); PUSH((int64_t)(r->type & (uint32_t)a)); break;
        case 0x4000010300000000ULL: a = POP(); PUSH((int64_t)(race & (uint64_t)a)); break;
        case 0x4000010400000000ULL: a = POP(); PUSH((int64_t)(r->attribute & (uint32_t)a)); break;
        case 0x4000010500000000ULL: PUSH(r->code); break;
        case 0x4000010700000000ULL: PUSH(r->type); break;
        case 0x4000010800000000ULL: PUSH((int64_t)race); break;
        case 0x4000010900000000ULL: PUSH(r->attribute); break;
        default: PUSH((int64_t)op); break;
        }
    }
#undef POP
#undef PUSH
    if (sp != 1 || !st[0])
        return 0;
    return r->code == 78734254 || r->code == 13857930 ||
           ((alias || !r->alias) && (token || (r->type & (EDO_TYPE_MONSTER | EDO_TYPE_TOKEN)) !=
                                                  (EDO_TYPE_MONSTER | EDO_TYPE_TOKEN)));
}
/* Puts the question from the engine's last select message into d->query; answers it here when there's nothing to
 * ask (returns 0 then: the engine runs on) */
static int Ask(struct YgoDuel *d)
{
    struct YgoEdoDuel *e = d->edo;
    struct Rd r;
    unsigned type, cp, i, n;
    if (!e->selLen)
        return -1;
    type = e->sel[0];
    r.p = e->sel + 1;
    r.end = e->sel + e->selLen;
    r.bad = 0;
    cp = R8(&r);
    if (type == MSG_ROCK_PAPER_SCISSORS)
        e->rpsPlayer = (uint8_t)cp;
    if (type != MSG_SELECT_PLACE)
        e->wantZoneSet = 0;
    d->nameCount = 0;
    switch (type)
    {
    case MSG_SELECT_IDLECMD:
    {
        static const uint8_t sKinds[] = {YGO_ACT_SUMMON, YGO_ACT_SPECIAL, YGO_ACT_POSITION, YGO_ACT_SET_MONSTER,
                                         YGO_ACT_SET_SPELL};
        unsigned list;
        Q(d, YGO_Q_IDLE, cp, YGO_P_NONE);
        e->kind = K_IDLE;
        e->wantZone = 0;
        e->declaring = 0;
        for (list = 0; list < 5; list++)
        {
            n = R32(&r);
            for (i = 0; i < n && !r.bad; i++)
            {
                uint32_t code = R32(&r);
                struct Loc l = RLoc3(&r, list == 2);
                int o = ObjOf(d, code, l);
                unsigned kind = sKinds[list];
                if (list == 2 && o >= 0 && !d->s.objs[o].faceUp)
                    kind = YGO_ACT_FLIP_SUMMON;
                Add(d, kind, o, (int32_t)(i << 16 | list));
            }
        }
        n = R32(&r);
        for (i = 0; i < n && !r.bad; i++)
        {
            uint32_t code = R32(&r);
            struct Loc l = RLoc3(&r, 0);
            uint64_t desc = R64(&r);
            struct YgoChoice *c;
            int o;
            R8(&r);
            o = ObjOf(d, code, l);
            if ((c = Add(d, YGO_ACT_ACTIVATE, o, (int32_t)(i << 16 | 5))))
            {
                c->desc = desc;
                c->value = (uint8_t)i;
                if (o < 0)
                    c->card = (uint16_t)YgoEdo_IdOfCode(code);
            }
        }
        if (R8(&r))
            Add(d, YGO_ACT_TO_BATTLE, -1, 6);
        if (R8(&r))
            Add(d, YGO_ACT_END_TURN, -1, 7);
        e->hint = 0;
        return 1;
    }
    case MSG_SELECT_BATTLECMD:
    {
        Q(d, YGO_Q_BATTLE, cp, YGO_P_NONE);
        e->kind = K_BATTLE;
        e->declaring = 0;
        n = R32(&r);
        for (i = 0; i < n && !r.bad; i++)
        {
            uint32_t code = R32(&r);
            struct Loc l = RLoc3(&r, 0);
            uint64_t desc = R64(&r);
            struct YgoChoice *c;
            int o;
            R8(&r);
            o = ObjOf(d, code, l);
            if ((c = Add(d, YGO_ACT_ACTIVATE, o, (int32_t)(i << 16 | 0))))
                c->desc = desc, c->value = (uint8_t)i;
        }
        n = R32(&r);
        for (i = 0; i < n && !r.bad; i++)
        {
            uint32_t code = R32(&r);
            struct Loc l = RLoc3(&r, 1);
            int o = ObjOf(d, code, l);
            struct YgoChoice *c = Add(d, YGO_ACT_ATTACK, o, (int32_t)(i << 16 | 1));
            unsigned direct = R8(&r);
            if (c)
                c->flags = (uint8_t)direct;
        }
        if (R8(&r))
            Add(d, YGO_ACT_TO_MAIN2, -1, 2);
        if (R8(&r))
            Add(d, YGO_ACT_END_TURN, -1, 3);
        e->hint = 0;
        return 1;
    }
    case MSG_SELECT_CHAIN:
    {
        unsigned forced;
        R8(&r);   /* (spe_count) */
        forced = R8(&r);
        R32(&r);
        R32(&r);
        n = R32(&r);
        if (!n && !forced)
            return RespondInt(d, -1), 0;
        Q(d, YGO_Q_CHAIN, cp, YGO_P_CHAIN);
        e->kind = K_CHAIN;
        d->query.min = forced ? 1 : 0;
        for (i = 0; i < n && !r.bad; i++)
        {
            uint32_t code = R32(&r);
            struct Loc l = RLoc(&r);
            uint64_t desc = R64(&r);
            struct YgoChoice *c;
            int o;
            R8(&r);
            o = ObjOf(d, code, l);
            if ((c = Add(d, YGO_ACT_ACTIVATE, o, (int32_t)i)))
            {
                c->desc = desc;
                if (o < 0)
                    c->card = (uint16_t)YgoEdo_IdOfCode(code);
            }
        }
        return 1;
    }
    case MSG_SELECT_EFFECTYN:
    {
        uint32_t code = R32(&r);
        struct Loc l = RLoc(&r);
        uint64_t desc = R64(&r);
        int o = ObjOf(d, code, l);
        Q(d, YGO_Q_YESNO, cp, YGO_P_EFFECT_YESNO);
        e->kind = K_EFFECTYN;
        d->query.card = (uint16_t)YgoEdo_IdOfCode(code);
        d->query.desc = desc;
        Add(d, 0, o, 0);
        Add(d, 0, o, 1);
        return 1;
    }
    case MSG_SELECT_YESNO:
    {
        uint64_t desc = R64(&r);
        if (desc == 31)
        {
            /* "Attack directly?": asked as the attack's targets, the opponent's monsters or their Life Points */
            unsigned z, opp = cp ^ 1;
            Q(d, YGO_Q_CARDS, cp, YGO_P_ATTACK_TARGET);
            e->kind = K_ATTACK_SYNTH;
            d->query.card = e->declaring ? d->s.objs[e->declaring - 1].card : 0;
            d->s.attacker = e->declaring;
            for (z = 0; z < 7; z++)
                if (e->mz[opp][z])
                    Add(d, 0, e->mz[opp][z] - 1, 0);
            if (Add(d, 0, -1, 1))
                d->query.items[d->query.count - 1].value = 1;
            return 1;
        }
        Q(d, YGO_Q_YESNO, cp, YGO_P_ANY);
        e->kind = K_YESNO;
        d->query.desc = desc;
        Add(d, 0, -1, 0);
        Add(d, 0, -1, 1);
        return 1;
    }
    case MSG_SELECT_OPTION:
    {
        n = R8(&r);
        Q(d, YGO_Q_OPTION, cp, PromptOf(e->hint));
        e->kind = K_OPTION;
        for (i = 0; i < n && !r.bad; i++)
        {
            uint64_t desc = R64(&r);
            struct YgoChoice *c = Add(d, 0, -1, (int32_t)i);
            if (c)
                c->desc = desc, c->value = (uint8_t)i;
            if (desc == 60 || desc == 61)
                d->query.prompt = YGO_P_COIN_CALL;
        }
        return 1;
    }
    case MSG_SELECT_CARD:
    case MSG_SELECT_TRIBUTE:
    {
        unsigned cancel = R8(&r), mn = R32(&r), mx = R32(&r);
        n = R32(&r);
        Q(d, YGO_Q_CARDS, cp, type == MSG_SELECT_TRIBUTE ? YGO_P_TRIBUTE : PromptOf(e->hint));
        e->kind = type == MSG_SELECT_TRIBUTE ? K_TRIBUTE : K_CARD;
        d->query.min = (uint8_t)(mn > 255 ? 255 : mn);
        d->query.max = (uint8_t)(mx > 255 ? 255 : mx);
        d->query.cancelable = (uint8_t)cancel;
        d->query.sum = d->query.min;
        if (cancel)
            d->query.min = 0;
        if (d->query.prompt == YGO_P_ATTACK_TARGET)
            d->s.attacker = e->declaring;
        for (i = 0; i < n && !r.bad; i++)
        {
            uint32_t code = R32(&r);
            struct Loc l;
            struct YgoChoice *c;
            int o;
            if (type == MSG_SELECT_TRIBUTE)
                l = RLoc3(&r, 0), l.pos = 0;
            else
                l = RLoc(&r);
            o = l.loc ? ObjOf(d, code, l) : -1;
            c = Add(d, 0, o, (int32_t)i);
            if (c)
            {
                if (o < 0)
                    c->card = (uint16_t)YgoEdo_IdOfCode(code);
                c->param = type == MSG_SELECT_TRIBUTE ? R8(&r) : 1;
            }
            else if (type == MSG_SELECT_TRIBUTE)
                R8(&r);
        }
        /* the target "attack directly?" already picked */
        if (d->query.prompt == YGO_P_ATTACK_TARGET && e->wantTarget)
        {
            for (i = 0; i < d->query.count; i++)
                if (d->query.items[i].obj == e->wantTarget - 1)
                {
                    int32_t resp[3] = {0, 1, (int32_t)i};
                    e->wantTarget = 0;
                    OCG_DuelSetResponse(e->duel, resp, sizeof(resp));
                    return 0;
                }
            e->wantTarget = 0;
        }
        return 1;
    }
    case MSG_SELECT_UNSELECT_CARD:
    {
        unsigned finish = R8(&r), cancel = R8(&r), mn = R32(&r), mx = R32(&r), m;
        Q(d, YGO_Q_CARDS, cp, PromptOf(e->hint));
        e->kind = K_UNSELECT;
        /* one card at a time: with enough picked it can be finished (no pick); some can be backed out of too */
        d->query.min = finish ? 0 : 1;
        d->query.max = 1;
        d->query.cancelable = (uint8_t)cancel;
        d->query.sum = finish ? 0 : 1;
        (void)mn;
        (void)mx;
        for (m = 0; m < 2; m++)
        {
            n = R32(&r);
            for (i = 0; i < n && !r.bad; i++)
            {
                uint32_t code = R32(&r);
                struct Loc l = RLoc(&r);
                int o = ObjOf(d, code, l);
                struct YgoChoice *c = Add(d, 0, o, (int32_t)(d->query.count));
                if (c)
                {
                    if (o < 0)
                        c->card = (uint16_t)YgoEdo_IdOfCode(code);
                    if (m)
                        c->flags = YGO_IF_PICKED;
                }
            }
        }
        return 1;
    }
    case MSG_SELECT_SUM:
    {
        unsigned atLeast = R8(&r), m;
        Q(d, YGO_Q_SUM, cp, PromptOf(e->hint));
        e->kind = K_SUM;
        d->query.sumMode = atLeast ? YGO_SUM_AT_LEAST : YGO_SUM_EXACT;
        d->query.sum = R32(&r);
        d->query.min = (uint8_t)R32(&r);
        d->query.max = (uint8_t)R32(&r);
        e->mustCount = 0;
        for (m = 0; m < 2; m++)
        {
            n = R32(&r);
            for (i = 0; i < n && !r.bad; i++)
            {
                uint32_t code = R32(&r);
                struct Loc l = RLoc(&r);
                uint32_t param = R32(&r);
                int o = ObjOf(d, code, l);
                struct YgoChoice *c = Add(d, 0, o, (int32_t)i);
                if (c)
                {
                    c->param = (uint16_t)(param & 0xffff);
                    c->value = (uint8_t)(param >> 16);   /* (the other way it can count) */
                    if (!m)
                        c->flags = YGO_IF_MUST, e->mustCount++;
                    if (o < 0)
                        c->card = (uint16_t)YgoEdo_IdOfCode(code);
                }
            }
        }
        return 1;
    }
    case MSG_SELECT_POSITION:
    {
        uint32_t code = R32(&r);
        unsigned pos = R8(&r);
        static const uint8_t sOrder[] = {1, 4, 8, 2};
        Q(d, YGO_Q_POSITION, cp, YGO_P_POSITION);
        e->kind = K_POSITION;
        d->query.card = (uint16_t)YgoEdo_IdOfCode(code);
        for (i = 0; i < 4; i++)
            if (pos & sOrder[i])
            {
                struct YgoChoice *c = Add(d, 0, -1, sOrder[i]);
                if (c)
                    c->value = sOrder[i], c->card = d->query.card;
            }
        return 1;
    }
    case MSG_SELECT_PLACE:
    case MSG_SELECT_DISFIELD:
    {
        unsigned count = R8(&r), side, z;
        uint32_t flag = R32(&r);
        Q(d, YGO_Q_ZONE, cp, YGO_P_NONE);
        e->kind = K_PLACE;
        d->query.min = d->query.max = (uint8_t)count;
        for (side = 0; side < 2; side++)
            for (z = 0; z < 13; z++)
            {
                unsigned spell = z >= 5, seq = spell ? z - 5 : z;
                uint32_t bit = (spell ? 0x100u << seq : 1u << seq) << (side ? 16 : 0);
                struct YgoChoice *c;
                if (spell && seq > 5)
                    continue;
                if (flag & bit)
                    continue;
                if ((c = Add(d, 0, -1, (int32_t)((side ? cp ^ 1 : cp) | (spell ? EDO_LOC_SZONE : EDO_LOC_MZONE) << 8 |
                                                 seq << 16))))
                    c->value = (uint8_t)seq, c->flags = (uint8_t)((spell ? YGO_ZF_SPELL : 0) | (side ? YGO_ZF_OPPONENT : 0));
            }
        if (count == 1)
        {
            int pick = -1;
            int wanted = e->wantZoneSet;
            e->wantZoneSet = 0;
            (void)wanted;
            if (d->query.count == 1)
                pick = 0;
            else if (wanted && GP(e, cp) == e->wantZonePlayer && type == MSG_SELECT_PLACE)
                for (i = 0; i < d->query.count && pick < 0; i++)
                {
                    const struct YgoChoice *c = &d->query.items[i];
                    if (c->flags & YGO_ZF_OPPONENT)
                        continue;
                    if (!e->wantZone || (c->value == ((e->wantZone - 1) & 0x7f) &&
                                         !!(c->flags & YGO_ZF_SPELL) == !!(e->wantZone & 0x80)))
                        pick = (int)i;
                }
            if (pick >= 0)
            {
                uint8_t b[3];
                int32_t v = e->resp[pick];
                b[0] = (uint8_t)(v & 0xff);
                b[1] = (uint8_t)(v >> 8 & 0xff);
                b[2] = (uint8_t)(v >> 16 & 0xff);
                OCG_DuelSetResponse(e->duel, b, 3);
                return 0;
            }
        }
        return 1;
    }
    case MSG_SELECT_COUNTER:
    {
        R16(&r);
        Q(d, YGO_Q_COUNTER, cp, YGO_P_NONE);
        e->kind = K_COUNTER;
        d->query.sum = R16(&r);
        n = R32(&r);
        for (i = 0; i < n && !r.bad; i++)
        {
            uint32_t code = R32(&r);
            struct Loc l = RLoc3(&r, 1);
            unsigned have = R16(&r);
            int o = ObjOf(d, code, l);
            struct YgoChoice *c = Add(d, 0, o, (int32_t)i);
            if (c)
                c->param = (uint16_t)have;
        }
        d->query.min = d->query.max = d->query.count;
        return 1;
    }
    case MSG_SORT_CHAIN:
    {
        int8_t v = -1;
        OCG_DuelSetResponse(e->duel, &v, 1);
        return 0;
    }
    case MSG_SORT_CARD:
    {
        n = R32(&r);
        Q(d, YGO_Q_SORT, cp, YGO_P_ORDER);
        e->kind = K_SORT;
        for (i = 0; i < n && !r.bad; i++)
        {
            uint32_t code = R32(&r);
            struct Loc l;
            int o;
            l.con = R8(&r);
            l.loc = (uint8_t)R32(&r);
            l.seq = R32(&r);
            l.pos = 0;
            o = ObjOf(d, code, l);
            if (Add(d, 0, o, (int32_t)i) && o < 0)
                d->query.items[d->query.count - 1].card = (uint16_t)YgoEdo_IdOfCode(code);
        }
        d->query.min = d->query.max = d->query.count;
        return 1;
    }
    case MSG_ANNOUNCE_RACE:
    case MSG_ANNOUNCE_ATTRIB:
    {
        unsigned count = R8(&r), bits = type == MSG_ANNOUNCE_RACE ? 32 : 7;
        uint64_t avail = type == MSG_ANNOUNCE_RACE ? R64(&r) : R32(&r);
        Q(d, YGO_Q_OPTION, cp, YGO_P_DECLARE);
        e->kind = type == MSG_ANNOUNCE_RACE ? K_RACE : K_ATTRIB;
        d->query.desc = type == MSG_ANNOUNCE_RACE ? 563 : 562;
        d->query.min = d->query.max = (uint8_t)count;
        for (i = 0; i < bits; i++)
            if (avail >> i & 1)
            {
                struct YgoChoice *c = Add(d, 0, -1, (int32_t)i);
                if (c)
                    c->value = (uint8_t)i, c->desc = (type == MSG_ANNOUNCE_RACE ? 1020 : 1010) + i;
            }
        return 1;
    }
    case MSG_ANNOUNCE_NUMBER:
    {
        n = R8(&r);
        Q(d, YGO_Q_OPTION, cp, YGO_P_DECLARE);
        e->kind = K_NUMBER;
        d->query.desc = 565;
        for (i = 0; i < n && !r.bad; i++)
        {
            uint64_t v = R64(&r);
            struct YgoChoice *c = Add(d, 0, -1, (int32_t)i);
            if (c)
                c->param = (uint16_t)v, c->value = (uint8_t)i;
        }
        return 1;
    }
    case MSG_ANNOUNCE_CARD:
    {
        uint64_t ops[64];
        unsigned k, total;
        n = R8(&r);
        for (i = 0; i < n && i < 64 && !r.bad; i++)
            ops[i] = R64(&r);
        Q(d, YGO_Q_NAME, cp, YGO_P_DECLARE);
        e->kind = K_NAME;
        d->query.desc = 564;
        d->query.min = d->query.max = 2;
        /* the cards the game shows first (its catalog), then the rest of EDOPro's when none of them can be */
        total = YgoEdo_RecCount();
        for (k = 0; k < 2 && !d->nameCount; k++)
            for (i = 0; i < total && d->nameCount < YGO_NAMES_MAX; i++)
            {
                const struct YgoEdoRec *rec = YgoEdo_RecAt(i);
                unsigned id = YgoEdo_IdOfCode(rec->code);
                if ((k == 0) != (id <= YGO_CARD_COUNT))
                    continue;
                if (Declarable(rec, ops, n < 64 ? n : 64))
                    d->names[d->nameCount++] = (uint16_t)id;
            }
        return 1;
    }
    case MSG_ROCK_PAPER_SCISSORS:
    {
        Q(d, YGO_Q_OPTION, cp, YGO_P_ANY);
        e->kind = K_RPS;
        for (i = 1; i <= 3; i++)
        {
            struct YgoChoice *c = Add(d, 0, -1, (int32_t)i);
            if (c)
                c->value = (uint8_t)i;
        }
        return 1;
    }
    default:
        printf("[ygo] edo: a question the game doesn't know (%u)\n", type);
        return -1;
    }
}

/* Runs the engine to its next question (or the end) */
static int Run(struct YgoDuel *d)
{
    struct YgoEdoDuel *e = d->edo;
    int guard = 0;
    for (;;)
    {
        int st = OCG_DuelProcess(e->duel), asked;
        uint32_t len = 0, off = 0;
        const uint8_t *buf = OCG_DuelGetMessage(e->duel, &len);
        while (buf && off + 4 <= len)
        {
            uint32_t mlen = buf[off] | buf[off + 1] << 8 | buf[off + 2] << 16 | (uint32_t)buf[off + 3] << 24;
            if (!mlen || off + 4 + mlen > len)
                break;
            Message(d, buf[off + 4], buf + off + 5, mlen - 1);
            off += 4 + mlen;
        }
        if (d->s.over)
        {
            Settle(d);
            memset(&d->query, 0, sizeof(d->query));
            return 1;
        }
        if (st == OCG_DUEL_STATUS_END)
        {
            Settle(d);
            memset(&d->query, 0, sizeof(d->query));
            if (!d->s.over)
            {
                d->s.over = 1;
                d->s.winner = NONE;
            }
            return 1;
        }
        if (st != OCG_DUEL_STATUS_AWAITING)
            continue;
        asked = Ask(d);
        if (asked < 0)
        {
            /* a question that can't be asked: the engine's first choice */
            int32_t zero = 0;
            OCG_DuelSetResponse(e->duel, &zero, 4);
            asked = 0;
        }
        if (asked)
        {
            Settle(d);
            return 1;
        }
        if (++guard > 100000)
        {
            snprintf(d->error, sizeof(d->error), "the duel engine stopped answering");
            e->failed = 1;
            memset(&d->query, 0, sizeof(d->query));
            return 0;
        }
    }
}

/* ------------------------------------------------------------------ */
/* Answers                                                              */
/* ------------------------------------------------------------------ */
static int SumCheck(const uint16_t *a, const uint16_t *b, unsigned n, unsigned i, int acc)
{
    /* every card counted one of its two ways, adding up to exactly acc (playerop.cpp select_sum_check1) */
    if (acc == 0 || i == n)
        return 0;
    if (i == n - 1)
        return acc == a[i] || (b[i] && acc == b[i]);
    return (acc > a[i] && SumCheck(a, b, n, i + 1, acc - a[i])) || (b[i] > 0 && acc > b[i] && SumCheck(a, b, n, i + 1, acc - b[i]));
}
static int Refuse(const struct YgoDuel *d, const char *why)
{
    snprintf(((struct YgoDuel *)d)->error, sizeof(d->error), "%s", why);
    return 0;
}
int YgoDuel_Check(const struct YgoDuel *d, const struct YgoAnswer *a)
{
    const struct YgoQuery *q = &d->query;
    const struct YgoEdoDuel *e = d->edo;
    unsigned i, j;
    if (!e || q->type == YGO_Q_NONE)
        return Refuse(d, "there's no question");
    if (a->count > YGO_PICKS_MAX)
        return Refuse(d, "too many picks");
    if (q->type != YGO_Q_COUNTER && q->type != YGO_Q_NAME)
        for (i = 0; i < a->count; i++)
        {
            if (a->picks[i] >= q->count)
                return Refuse(d, "no such choice");
            for (j = 0; j < i; j++)
                if (a->picks[j] == a->picks[i])
                    return Refuse(d, "a choice picked twice");
        }
    switch (q->type)
    {
    case YGO_Q_IDLE: case YGO_Q_BATTLE: case YGO_Q_YESNO: case YGO_Q_POSITION:
        return a->count == 1 ? 1 : Refuse(d, "pick one");
    case YGO_Q_CHAIN:
        return a->count == 1 || (!q->min && !a->count) ? 1 : Refuse(d, "pick one or pass");
    case YGO_Q_OPTION:
        return a->count >= q->min && a->count <= q->max ? 1 : Refuse(d, "pick as many as asked");
    case YGO_Q_ZONE:
        return a->count == q->min ? 1 : Refuse(d, "pick as many zones as asked");
    case YGO_Q_CARDS:
        if (e->kind == K_ATTACK_SYNTH)
            return a->count == 1 ? 1 : Refuse(d, "pick a target");
        if (e->kind == K_TRIBUTE)
        {
            unsigned worth = 0;
            if (!a->count && q->cancelable)
                return 1;
            for (i = 0; i < a->count; i++)
                worth += q->items[a->picks[i]].param;
            return a->count <= q->max && worth >= (q->cancelable ? q->sum : q->min) ? 1 : Refuse(d, "not enough Tributes");
        }
        if (!a->count && q->cancelable)
            return 1;
        return a->count >= (q->cancelable ? q->sum : q->min) && a->count <= q->max ? 1 : Refuse(d, "pick as many as asked");
    case YGO_Q_SUM:
    {
        uint16_t lo[YGO_QUERY_MAX], hi[YGO_QUERY_MAX];
        unsigned n = 0, picked = 0;
        for (i = 0; i < q->count; i++)
            if (q->items[i].flags & YGO_IF_MUST)
                lo[n] = q->items[i].param, hi[n] = q->items[i].value, n++;
        for (i = 0; i < a->count; i++)
        {
            if (q->items[a->picks[i]].flags & YGO_IF_MUST)
                return Refuse(d, "that one counts already");
            lo[n] = q->items[a->picks[i]].param, hi[n] = q->items[a->picks[i]].value, n++, picked++;
        }
        if (q->sumMode == YGO_SUM_EXACT)
        {
            if (picked < q->min || picked > q->max)
                return Refuse(d, "pick as many as asked");
            return SumCheck(lo, hi, n, 0, (int)q->sum) ? 1 : Refuse(d, "they don't add up");
        }
        {
            int sum = 0, mx = 0, mn = 0x7fffffff;
            for (i = 0; i < n; i++)
            {
                int ms = hi[i] && hi[i] < lo[i] ? hi[i] : lo[i];
                sum += ms;
                mx += hi[i] > lo[i] ? hi[i] : lo[i];
                if (ms < mn)
                    mn = ms;
            }
            return mx >= (int)q->sum && sum - mn < (int)q->sum ? 1 : Refuse(d, "they don't add up");
        }
    }
    case YGO_Q_SORT:
        return a->count == q->count ? 1 : Refuse(d, "put every card in order");
    case YGO_Q_COUNTER:
    {
        unsigned total = 0;
        if (a->count != q->count)
            return Refuse(d, "one number per card");
        for (i = 0; i < a->count; i++)
        {
            if (a->picks[i] > q->items[i].param)
                return Refuse(d, "more than it has");
            total += a->picks[i];
        }
        return total == q->sum ? 1 : Refuse(d, "remove as many as asked");
    }
    case YGO_Q_NAME:
        return a->count == 2 && (unsigned)(a->picks[0] | a->picks[1] << 8) < d->nameCount ? 1 : Refuse(d, "no such card");
    default:
        return Refuse(d, "no question");
    }
}
static void Respond(struct YgoDuel *d, const struct YgoAnswer *a)
{
    struct YgoEdoDuel *e = d->edo;
    const struct YgoQuery *q = &d->query;
    uint8_t buf[512];
    unsigned len = 0, i;
#define PUT32(v) do { int32_t _v = (int32_t)(v); memcpy(buf + len, &_v, 4); len += 4; } while (0)
    switch (e->kind)
    {
    case K_IDLE:
        e->wantZone = a->zone;
        e->wantZonePlayer = q->player;
        e->wantZoneSet = 1;
        PUT32(e->resp[a->picks[0]]);
        break;
    case K_BATTLE:
        if (q->items[a->picks[0]].kind == YGO_ACT_ATTACK && q->items[a->picks[0]].obj != NONE)
            e->declaring = (uint8_t)(q->items[a->picks[0]].obj + 1);
        PUT32(e->resp[a->picks[0]]);
        break;
    case K_CHAIN:
        PUT32(a->count ? e->resp[a->picks[0]] : -1);
        break;
    case K_EFFECTYN: case K_YESNO: case K_POSITION:
        PUT32(e->resp[a->picks[0]]);
        break;
    case K_OPTION: case K_NUMBER: case K_RPS:
        PUT32(e->resp[a->picks[0]]);
        break;
    case K_RACE:
    {
        uint64_t mask = 0;
        for (i = 0; i < a->count; i++)
            mask |= 1ULL << q->items[a->picks[i]].value;
        memcpy(buf, &mask, 8);
        len = 8;
        break;
    }
    case K_ATTRIB:
    {
        uint32_t mask = 0;
        for (i = 0; i < a->count; i++)
            mask |= 1u << q->items[a->picks[i]].value;
        PUT32(mask);
        break;
    }
    case K_ATTACK_SYNTH:
        if (q->items[a->picks[0]].obj == NONE)
            PUT32(1);
        else
        {
            e->wantTarget = (uint8_t)(q->items[a->picks[0]].obj + 1);
            PUT32(0);
        }
        break;
    case K_CARD: case K_TRIBUTE: case K_SUM:
        if (!a->count && e->kind != K_SUM)
        {
            PUT32(-1);
            break;
        }
        PUT32(0);
        PUT32(a->count);
        for (i = 0; i < a->count; i++)
            PUT32(e->kind == K_SUM ? a->picks[i] - e->mustCount : e->resp[a->picks[i]]);
        break;
    case K_UNSELECT:
        if (!a->count)
            PUT32(-1);
        else
        {
            PUT32(1);
            PUT32(e->resp[a->picks[0]]);
        }
        break;
    case K_PLACE:
        for (i = 0; i < a->count; i++)
        {
            int32_t v = e->resp[a->picks[i]];
            buf[len++] = (uint8_t)(v & 0xff);
            buf[len++] = (uint8_t)(v >> 8 & 0xff);
            buf[len++] = (uint8_t)(v >> 16 & 0xff);
        }
        break;
    case K_COUNTER:
        for (i = 0; i < q->count; i++)
        {
            int16_t v = (int16_t)a->picks[i];
            memcpy(buf + len, &v, 2);
            len += 2;
        }
        break;
    case K_SORT:
    {
        int8_t at[YGO_QUERY_MAX];
        for (i = 0; i < a->count; i++)
            at[a->picks[i]] = (int8_t)i;
        memcpy(buf, at, q->count);
        len = q->count;
        break;
    }
    case K_NAME:
        PUT32(YgoEdo_CodeOfId(d->names[a->picks[0] | a->picks[1] << 8]));
        break;
    default:
        PUT32(0);
        break;
    }
#undef PUT32
    OCG_DuelSetResponse(e->duel, buf, len);
}

/* ------------------------------------------------------------------ */
/* The engine's readers                                                 */
/* ------------------------------------------------------------------ */
static uint16_t sSetcodes[8];
static void CardReader(void *payload, uint32_t code, OCG_CardData *out)
{
    const struct YgoEdoRec *r = YgoEdo_Rec(code);
    unsigned i;
    uint64_t sc;
    (void)payload;
    memset(out, 0, sizeof(*out));
    out->code = code;
    out->setcodes = sSetcodes;
    sSetcodes[0] = 0;
    if (!r)
        return;
    for (i = 0, sc = r->setcode; i < 4 && sc; i++, sc >>= 16)
        sSetcodes[i] = (uint16_t)(sc & 0xffff);
    sSetcodes[i] = 0;
    out->alias = r->alias;
    out->type = r->type;
    out->level = r->level & 0xff;
    out->lscale = r->level >> 24 & 0xff;
    out->rscale = r->level >> 16 & 0xff;
    out->attribute = r->attribute;
    out->race = r->race;
    out->attack = r->atk;
    out->defense = r->def;
    if (r->type & EDO_TYPE_LINK)
        out->link_marker = (uint32_t)r->def, out->defense = 0;
}
static int ScriptReader(void *payload, OCG_Duel duel, const char *name)
{
    uint32_t size = 0;
    uint8_t *text;
    const char *base = strrchr(name, '/');
    int ok;
    (void)payload;
    base = base ? base + 1 : name;
    if (!(text = YgoEdo_File(base, &size)))
        return 0;
    ok = OCG_LoadScript(duel, (const char *)text, size, base);
    free(text);
    return ok;
}
static void LogHandler(void *payload, const char *text, int type)
{
    (void)payload;
    /* (a Normal Monster has no script: not worth a line) */
    if (strstr(text, "Failed to load") || strstr(text, "not found"))
        return;
    printf("[ygo] edo %s: %s\n", type == OCG_LOG_TYPE_ERROR ? "error" : "script", text);
}

/* ------------------------------------------------------------------ */
/* The API                                                              */
/* ------------------------------------------------------------------ */
void YgoDuel_Free(struct YgoDuel *d)
{
    if (!d->edo)
        return;
    if (d->edo->duel)
        OCG_DestroyDuel(d->edo->duel);
    free(d->edo->log);
    free(d->edo);
    d->edo = NULL;
}
static unsigned DeckLen(const uint16_t *cards, unsigned max)
{
    unsigned n = 0;
    while (n < max && cards[n])
        n++;
    return n;
}
static int Start(struct YgoDuel *d)
{
    struct YgoEdoDuel *e = d->edo;
    OCG_DuelOptions o;
    unsigned t, i;
    memset(&o, 0, sizeof(o));
    o.seed[0] = e->seed ^ 0x9E3779B97F4A7C15ULL;
    o.seed[1] = (uint64_t)e->seed * 0xBF58476D1CE4E5B9ULL + 1;
    o.seed[2] = (uint64_t)e->seed * 0x94D049BB133111EBULL + 2;
    o.seed[3] = (uint64_t)e->seed << 17 ^ 0x2545F4914F6CDD1DULL;
    /* today's rules (Master Rule 5) without the Extra Monster Zones: the table has five zones a side */
    o.flags = (DUEL_MODE_MR5) & ~(uint64_t)(DUEL_EMZONE | DUEL_FSX_MMZONE);
    if (gYgoEdoNoShuffle)
        o.flags |= DUEL_PSEUDO_SHUFFLE;
    o.team1.startingLP = o.team2.startingLP = YGO_LP;
    o.team1.startingDrawCount = o.team2.startingDrawCount = 5;
    o.team1.drawCountPerTurn = o.team2.drawCountPerTurn = 1;
    o.cardReader = CardReader;
    o.scriptReader = ScriptReader;
    o.logHandler = LogHandler;
    if (OCG_CreateDuel(&e->duel, &o) != OCG_DUEL_CREATION_SUCCESS)
    {
        e->duel = NULL;
        return 0;
    }
    if (!ScriptReader(NULL, e->duel, "constant.lua") || !ScriptReader(NULL, e->duel, "utility.lua"))
        return 0;
    /* the objects, as the engine gets the cards. The engine never shuffles a Deck itself (EDOPro's program does it
     * before handing the cards over): each Deck is shuffled here, from the seed, so both games of a duel online deal
     * alike (the tests' unshuffled Decks keep their order) */
    for (t = 0; t < 2; t++)
    {
        unsigned gp = t ^ e->swap, nm = DeckLen(e->decks[gp], YGO_DECK_MAX), nx = DeckLen(e->extras[gp], YGO_FUSION_MAX);
        uint16_t deck[YGO_DECK_MAX];
        memcpy(deck, e->decks[gp], sizeof(deck));
        if (!gYgoEdoNoShuffle)
        {
            uint64_t x = (uint64_t)e->seed * 0x9E3779B97F4A7C15ULL + 0xD1B54A32D192ED03ULL * (gp + 1);
            for (i = nm; i > 1; i--)
            {
                unsigned j;
                uint16_t k;
                x ^= x >> 30;
                x *= 0xBF58476D1CE4E5B9ULL;
                x ^= x >> 27;
                x *= 0x94D049BB133111EBULL;
                x ^= x >> 31;
                j = (unsigned)(x % i);
                k = deck[i - 1];
                deck[i - 1] = deck[j];
                deck[j] = k;
            }
        }
        for (i = 0; i < nm + nx; i++)
        {
            OCG_NewCardInfo c;
            unsigned id = i < nm ? deck[i] : e->extras[gp][i - nm];
            struct Loc l;
            int ob;
            memset(&c, 0, sizeof(c));
            c.team = (uint8_t)t;
            c.con = (uint8_t)t;
            c.code = YgoEdo_CodeOfId(id);
            c.loc = i < nm ? EDO_LOC_DECK : EDO_LOC_EXTRA;
            c.pos = EDO_POS_FACEDOWN_DEFENSE;
            if (!c.code)
                continue;
            /* (the engine sends an Extra Deck monster in a main deck to the Extra Deck) */
            {
                const struct YgoEdoRec *rec = YgoEdo_Rec(c.code);
                if (rec && (rec->type & (EDO_TYPE_FUSION | EDO_TYPE_SYNCHRO | EDO_TYPE_XYZ | EDO_TYPE_LINK)))
                    c.loc = EDO_LOC_EXTRA;
            }
            OCG_DuelNewCard(e->duel, &c);
            if ((ob = NewObj(d, c.code, gp)) < 0)
                continue;
            l.con = (uint8_t)t;
            l.loc = (uint8_t)c.loc;
            l.seq = c.loc == EDO_LOC_DECK ? e->lists[t][LI_DECK].n : e->lists[t][LI_EXTRA].n;
            l.pos = c.pos;
            Place(d, (unsigned)ob, l);
        }
    }
    OCG_StartDuel(e->duel);
    return 1;
}
int YgoDuel_Init(struct YgoDuel *d, const uint16_t main0[YGO_DECK_MAX], const uint16_t extra0[YGO_FUSION_MAX],
                 const uint16_t main1[YGO_DECK_MAX], const uint16_t extra1[YGO_FUSION_MAX], uint32_t seed,
                 unsigned first)
{
    struct YgoEdoDuel *e;
    YgoDuel_Free(d);
    memset(d, 0, sizeof(*d));
    if (!YgoEdo_DataInit())
    {
        snprintf(d->error, sizeof(d->error), "the card data is missing");
        return 0;
    }
    if (!(e = calloc(1, sizeof(*e))))
    {
        snprintf(d->error, sizeof(d->error), "out of memory");
        return 0;
    }
    d->edo = e;
    e->seed = seed;
    e->first = (uint8_t)(first & 1);
    e->swap = e->first;
    memcpy(e->decks[0], main0, sizeof(e->decks[0]));
    memcpy(e->decks[1], main1, sizeof(e->decks[1]));
    if (extra0)
        memcpy(e->extras[0], extra0, sizeof(e->extras[0]));
    if (extra1)
        memcpy(e->extras[1], extra1, sizeof(e->extras[1]));
    d->s.first = e->first;
    d->s.current = e->first;
    d->s.players[0].lp = d->s.players[1].lp = YGO_LP;
    if (!Start(d))
    {
        snprintf(d->error, sizeof(d->error), "the duel engine couldn't start");
        YgoDuel_Free(d);
        return 0;
    }
    return Run(d);
}
int YgoDuel_Copy(struct YgoDuel *dst, const struct YgoDuel *src)
{
    const struct YgoEdoDuel *e = src->edo;
    uint32_t i;
    if (dst == src)
        return 1;
    if (!e)
        return 0;
    if (!YgoDuel_Init(dst, e->decks[0], e->extras[0], e->decks[1], e->extras[1], e->seed, e->first))
        return 0;
    for (i = 0; i < e->logCount; i++)
        if (!YgoDuel_Answer(dst, &e->log[i]))
            return 0;
    if (src->s.over && !dst->s.over)
    {
        /* a concession (not an answer) */
        dst->s.over = 1;
        dst->s.winner = src->s.winner;
        dst->s.winReason = src->s.winReason;
        memset(&dst->query, 0, sizeof(dst->query));
    }
    dst->s.eventSeq = src->s.eventSeq;
    memcpy(dst->s.events, src->s.events, sizeof(dst->s.events));
    return 1;
}
int YgoDuel_Rewind(struct YgoDuel *d, unsigned answers)
{
    struct YgoEdoDuel *e = d->edo;
    struct YgoAnswer *log;
    uint32_t shown = d->s.eventSeq, i;
    uint16_t decks[2][YGO_DECK_MAX], extras[2][YGO_FUSION_MAX];
    uint32_t seed;
    unsigned first;
    if (!e || answers > e->logCount)
        return 0;
    if (!(log = malloc((answers + 1) * sizeof(*log))))
        return 0;
    memcpy(log, e->log, answers * sizeof(*log));
    memcpy(decks, e->decks, sizeof(decks));
    memcpy(extras, e->extras, sizeof(extras));
    seed = e->seed;
    first = e->first;
    if (!YgoDuel_Init(d, decks[0], extras[0], decks[1], extras[1], seed, first))
    {
        free(log);
        return 0;
    }
    for (i = 0; i < answers; i++)
        if (!YgoDuel_Answer(d, &log[i]))
        {
            free(log);
            return 0;
        }
    free(log);
    if (d->s.eventSeq < shown)
        d->s.eventSeq = shown;
    return 1;
}
int YgoDuel_Answer(struct YgoDuel *d, const struct YgoAnswer *a)
{
    struct YgoEdoDuel *e = d->edo;
    if (!e || d->s.over || e->failed)
        return Refuse(d, "the duel is over");
    if (!YgoDuel_Check(d, a))
        return 0;
    if (e->logCount == e->logCap)
    {
        uint32_t cap = e->logCap ? e->logCap * 2 : 256;
        struct YgoAnswer *n = realloc(e->log, cap * sizeof(*n));
        if (!n)
            return Refuse(d, "out of memory");
        e->log = n;
        e->logCap = cap;
    }
    e->log[e->logCount++] = *a;
    Respond(d, a);
    d->error[0] = 0;
    return Run(d);
}
int YgoDuel_Concede(struct YgoDuel *d, unsigned player)
{
    if (d->s.over)
        return 0;
    d->s.over = 1;
    d->s.winner = (uint8_t)((player & 1) ^ 1);
    d->s.winReason = YGO_WIN_CONCEDE;
    Event(d, YGO_EV_WIN, d->s.winner, -1, -1, YGO_WIN_CONCEDE, 0);
    memset(&d->query, 0, sizeof(d->query));
    return 1;
}
int YgoDuel_Refresh(struct YgoDuel *d)
{
    if (!d->edo || d->s.over)
        return 0;
    return Ask(d) > 0;
}
static uint32_t Mix(uint32_t h, uint32_t v)
{
    h ^= v;
    return h * 16777619u;
}
uint32_t YgoDuel_Hash(const struct YgoDuel *d)
{
    const struct YgoState *s = &d->s;
    uint32_t h = 2166136261u;
    unsigned o, i;
    for (o = 0; o < s->objCount; o++)
    {
        const struct YgoObj *ob = &s->objs[o];
        h = Mix(h, ob->card | ob->owner << 16 | ob->controller << 18 | ob->location << 20 | (uint32_t)ob->seq << 24);
        h = Mix(h, ob->faceUp | ob->attackPos << 1 | ob->counters << 8 | (uint32_t)ob->equipTo << 16);
        h = Mix(h, (uint32_t)ob->atk);
        h = Mix(h, (uint32_t)ob->def);
    }
    for (i = 0; i < 2; i++)
        h = Mix(h, (uint32_t)s->players[i].lp);
    h = Mix(h, s->turn | s->phase << 16 | s->current << 24);
    h = Mix(h, s->over | s->winner << 8 | d->query.type << 16 | d->query.player << 24);
    h = Mix(h, d->query.count | d->query.min << 8 | d->query.max << 16);
    for (i = 0; i < d->query.count; i++)
        h = Mix(h, d->query.items[i].kind | d->query.items[i].obj << 8 | d->query.items[i].value << 16);
    return h;
}
int YgoDuel_Conserved(const struct YgoDuel *d)
{
    const struct YgoEdoDuel *e = d->edo;
    uint8_t seen[YGO_OBJS];
    unsigned cp, li, i, o;
    if (!e)
        return 0;
    memset(seen, 0, sizeof(seen));
    for (cp = 0; cp < 2; cp++)
    {
        for (li = 0; li < LISTS; li++)
            for (i = 0; i < e->lists[cp][li].n; i++)
                if (seen[e->lists[cp][li].o[i]]++)
                    return 0;
        for (i = 0; i < 7; i++)
        {
            if (e->mz[cp][i] && seen[e->mz[cp][i] - 1]++)
                return 0;
            for (o = 0; o < e->overlay[cp][i].n; o++)
                if (seen[e->overlay[cp][i].o[o]]++)
                    return 0;
        }
        for (i = 0; i < 8; i++)
            if (e->sz[cp][i] && seen[e->sz[cp][i] - 1]++)
                return 0;
    }
    for (o = 0; o < d->s.objCount; o++)
        if (!seen[o] && d->s.objs[o].location != YGO_LOC_NONE)
            return 0;
    return 1;
}
int YgoDuel_QuietChance(const struct YgoDuel *d)
{
    return d->edo && d->query.type == YGO_Q_CHAIN && !d->query.min && !d->s.chainCount && d->edo->quiet;
}
int YgoDuel_Atk(const struct YgoState *s, unsigned obj) { return obj < s->objCount ? s->objs[obj].atk : 0; }
int YgoDuel_Def(const struct YgoState *s, unsigned obj) { return obj < s->objCount ? s->objs[obj].def : 0; }
int YgoDuel_Level(const struct YgoState *s, unsigned obj)
{
    const struct YgoCard *c;
    if (obj >= s->objCount)
        return 0;
    if (s->objs[obj].level)
        return s->objs[obj].level;
    c = Ygo_Card(s->objs[obj].card);
    return c ? c->level : 0;
}
const struct YgoEvent *YgoDuel_GetEvent(const struct YgoState *s, uint32_t seq)
{
    if (seq >= s->eventSeq || s->eventSeq - seq > YGO_EVENT_RING)
        return NULL;
    return &s->events[seq % YGO_EVENT_RING];
}
unsigned YgoDuel_AnswerCount(const struct YgoDuel *d) { return d->edo ? d->edo->logCount : 0; }

/* Words: EDOPro's strings with the card's name for its %ls */
static const char *Fill(const char *fmt, const char *name, char *buf, unsigned size)
{
    const char *p = strstr(fmt, "%ls");
    if (!p)
        return fmt;
    snprintf(buf, size, "%.*s%s%s", (int)(p - fmt), fmt, name ? name : "", p + 3);
    p = strstr(buf, "%ls");
    if (p)
        buf[p - buf] = 0;   /* (a second one: cut there) */
    return buf;
}
const char *YgoDuel_QueryText(const struct YgoDuel *d, char *buf, unsigned size)
{
    const struct YgoQuery *q = &d->query;
    const struct YgoCard *c = Ygo_Card(q->card);
    const char *t;
    if (q->type == YGO_Q_YESNO && q->prompt == YGO_P_EFFECT_YESNO && !q->desc)
        t = YgoEdo_System(95);
    else
        t = q->desc ? YgoEdo_Desc(q->desc) : NULL;
    if (!t)
        return "";
    return Fill(t, c ? c->name : NULL, buf, size);
}
const char *YgoDuel_ItemText(const struct YgoDuel *d, unsigned item, char *buf, unsigned size)
{
    const struct YgoQuery *q = &d->query;
    const struct YgoChoice *c;
    const struct YgoCard *card;
    const char *t;
    if (item >= q->count)
        return "";
    c = &q->items[item];
    card = Ygo_Card(c->card);
    if (q->type == YGO_Q_OPTION && d->edo && d->edo->kind == K_NUMBER)
    {
        snprintf(buf, size, "%u", (unsigned)c->param);
        return buf;
    }
    if (q->type == YGO_Q_OPTION && d->edo && d->edo->kind == K_RPS)
        return c->value == 1 ? "Scissors" : c->value == 2 ? "Rock" : "Paper";
    if (!c->desc || !(t = YgoEdo_Desc(c->desc)))
        return "";
    return Fill(t, card ? card->name : NULL, buf, size);
}
int Ygo_CardReady(unsigned card)
{
    uint32_t code = YgoEdo_CodeOfId(card);
    return code && YgoEdo_Playable(code);
}
