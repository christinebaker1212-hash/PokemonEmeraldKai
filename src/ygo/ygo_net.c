/* Yu-Gi-Oh! duels with a friend: the decks once, then the answers (include/ygo/net.h). */
#include <stdio.h>
#include <string.h>
#include "tcg/link.h"
#include "ygo/collection.h"
#include "ygo/net.h"

enum
{
    INVITE = YGO_NET_FIRST_TYPE,
    ACCEPT,
    START,
    ANSWER,
    ACK,
    CANCEL
};
#define DECK_BYTES (2 * (YGO_DECK_MAX + YGO_FUSION_MAX))
#define DECK_LEN (7 + DECK_BYTES)
#define ANSWER_LEN 21   /* + the picks */
#define FLAG_CONCEDE 1

static uint32_t Read(const uint8_t *p) { return p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24; }
static void Write(uint8_t *p, uint32_t n)
{
    unsigned i;
    for (i = 0; i < 4; i++)
        p[i] = (uint8_t)(n >> (8 * i));
}
static void Header(const struct YgoNet *l, uint8_t *p, unsigned type)
{
    p[0] = TCG_LINK_RELAY;
    p[1] = TCG_LINK_VERSION;
    p[2] = (uint8_t)type;
    Write(p + 3, l->session);
}
static void Send(struct YgoNet *l, unsigned type)
{
    uint8_t p[7];
    Header(l, p, type);
    l->send(l->context, l->peer, p, 7);
}
static int Valid(const uint16_t *main, const uint16_t *extra)
{
    char reason[160];
    return Ygo_DeckValidate(main, extra, NULL, reason, sizeof(reason));
}
static void SendDeck(struct YgoNet *l, unsigned type, const uint16_t *main, const uint16_t *extra)
{
    uint8_t p[DECK_LEN];
    unsigned i;
    Header(l, p, type);
    for (i = 0; i < YGO_DECK_MAX + YGO_FUSION_MAX; i++)
    {
        uint16_t card = i < YGO_DECK_MAX ? main[i] : extra[i - YGO_DECK_MAX];
        p[7 + i * 2] = (uint8_t)card;
        p[8 + i * 2] = (uint8_t)(card >> 8);
    }
    l->send(l->context, l->peer, p, sizeof(p));
}
static int ReadDeck(const uint8_t *p, uint16_t *main, uint16_t *extra)
{
    unsigned i;
    for (i = 0; i < YGO_DECK_MAX + YGO_FUSION_MAX; i++)
    {
        uint16_t card = (uint16_t)(p[7 + i * 2] | p[8 + i * 2] << 8);
        if (card > YGO_CARD_COUNT)
            return 0;
        if (i < YGO_DECK_MAX)
            main[i] = card;
        else
            extra[i - YGO_DECK_MAX] = card;
    }
    return Valid(main, extra);
}

uint32_t YgoNet_Hash(const struct YgoDuel *d)
{
    uint32_t h = YgoDuel_Hash(d);
    unsigned i, k;
    h = (h ^ d->answerCount) * 16777619u;
    for (i = 0; i < d->answerCount; i++)
    {
        h = (h ^ d->answers[i].count) * 16777619u;
        for (k = 0; k < d->answers[i].count && k < (unsigned)sizeof(d->answers[i].picks); k++)
            h = (h ^ d->answers[i].picks[k]) * 16777619u;
    }
    h = (h ^ d->query.type) * 16777619u;
    h = (h ^ d->query.player) * 16777619u;
    return h;
}
unsigned YgoNet_First(uint32_t session) { return (session >> 7) & 1; }

void YgoNet_Init(struct YgoNet *l, void (*send)(void *, unsigned, const uint8_t *, unsigned), void *context)
{
    memset(l, 0, sizeof(*l));
    l->send = send;
    l->context = context;
}
void YgoNet_Cancel(struct YgoNet *l, const char *reason)
{
    if (l->state != YGO_NET_IDLE && l->state != YGO_NET_ENDED)
        Send(l, CANCEL);
    l->state = YGO_NET_ENDED;
    l->pending = 0;
    snprintf(l->error, sizeof(l->error), "%s", reason);
}
static void Begin(struct YgoNet *l)
{
    if (!YgoDuel_Init(&l->game, l->main[0], l->extra[0], l->main[1], l->extra[1], l->session,
                      YgoNet_First(l->session)))
    {
        YgoNet_Cancel(l, "The duel couldn't be set up.");
        return;
    }
    l->state = YGO_NET_PLAYING;
    l->sequence = 0;
    l->pending = 0;
}
int YgoNet_Invite(struct YgoNet *l, unsigned peer, uint32_t session, const uint16_t *main, const uint16_t *extra,
                   uint32_t now)
{
    if ((l->state != YGO_NET_IDLE && l->state != YGO_NET_ENDED) || !session || !Valid(main, extra))
        return 0;
    l->session = session;
    l->peer = peer;
    l->seat = 0;
    l->sequence = l->pending = 0;
    l->error[0] = 0;
    memcpy(l->main[0], main, sizeof(l->main[0]));
    memcpy(l->extra[0], extra, sizeof(l->extra[0]));
    l->state = YGO_NET_SENT;
    l->deadline = now + 120000;
    SendDeck(l, INVITE, main, extra);
    return 1;
}
int YgoNet_Accept(struct YgoNet *l, const uint16_t *main, const uint16_t *extra, uint32_t now)
{
    if (l->state != YGO_NET_INVITED || !Valid(main, extra))
        return 0;
    memcpy(l->main[1], main, sizeof(l->main[1]));
    memcpy(l->extra[1], extra, sizeof(l->extra[1]));
    l->state = YGO_NET_STARTING;
    l->deadline = now + 30000;
    SendDeck(l, ACCEPT, main, extra);
    return 1;
}
/* Sends what was just done to l->game (from beforeHash): an answer, or a concession */
static void SendAnswer(struct YgoNet *l, uint32_t beforeHash, const struct YgoAnswer *a, unsigned flags, uint32_t now)
{
    uint8_t p[ANSWER_LEN + sizeof(a->picks)];
    unsigned count = flags & FLAG_CONCEDE ? 0 : a->count;
    Header(l, p, ANSWER);
    Write(p + 7, l->sequence++);
    Write(p + 11, beforeHash);
    Write(p + 15, YgoNet_Hash(&l->game));
    p[19] = (uint8_t)flags;
    p[20] = (uint8_t)count;
    if (count)
        memcpy(p + ANSWER_LEN, a->picks, count);
    l->pending = 1;
    l->deadline = now + 30000;
    l->error[0] = 0;
    l->send(l->context, l->peer, p, ANSWER_LEN + count);
}
int YgoNet_Submit(struct YgoNet *l, const struct YgoAnswer *a, uint32_t now)
{
    uint32_t before;
    if (l->state != YGO_NET_PLAYING || l->pending || l->game.s.over || l->game.query.type == YGO_Q_NONE ||
        l->game.query.player != l->seat || a->count > sizeof(a->picks))
        return 0;
    before = YgoNet_Hash(&l->game);
    l->beforePending = l->game;
    l->sequenceBefore = l->sequence;
    if (!YgoDuel_Answer(&l->game, a))
    {
        snprintf(l->error, sizeof(l->error), "%s", l->game.error);
        l->game = l->beforePending;
        return 0;
    }
    SendAnswer(l, before, a, 0, now);
    return 1;
}
int YgoNet_Concede(struct YgoNet *l, uint32_t now)
{
    struct YgoAnswer none;
    uint32_t before;
    if (l->state != YGO_NET_PLAYING || l->pending || l->game.s.over)
        return 0;
    memset(&none, 0, sizeof(none));
    before = YgoNet_Hash(&l->game);
    l->beforePending = l->game;
    l->sequenceBefore = l->sequence;
    YgoDuel_Concede(&l->game, l->seat);
    SendAnswer(l, before, &none, FLAG_CONCEDE, now);
    return 1;
}
void YgoNet_Receive(struct YgoNet *l, unsigned from, const uint8_t *p, unsigned len, int willing, uint32_t now)
{
    unsigned type;
    if (len < 7 || p[0] != TCG_LINK_RELAY || p[1] != TCG_LINK_VERSION)
        return;
    type = p[2];
    if (type == INVITE)
    {
        uint16_t main[YGO_DECK_MAX], extra[YGO_FUSION_MAX];
        if (len != DECK_LEN || !willing || (l->state != YGO_NET_IDLE && l->state != YGO_NET_ENDED) || !Read(p + 3) ||
            !ReadDeck(p, main, extra))
            return;
        l->session = Read(p + 3);
        l->peer = from;
        l->seat = 1;
        l->sequence = l->pending = 0;
        memcpy(l->main[0], main, sizeof(main));
        memcpy(l->extra[0], extra, sizeof(extra));
        l->state = YGO_NET_INVITED;
        l->deadline = now + 120000;
        l->error[0] = 0;
        return;
    }
    if (from != l->peer || Read(p + 3) != l->session || l->state == YGO_NET_IDLE || l->state == YGO_NET_ENDED)
        return;
    if (type == CANCEL && len == 7)
    {
        l->state = YGO_NET_ENDED;
        l->pending = 0;
        snprintf(l->error, sizeof(l->error), "The other player ended the duel.");
        return;
    }
    if (type == ACCEPT && len == DECK_LEN && l->state == YGO_NET_SENT)
    {
        if (!ReadDeck(p, l->main[1], l->extra[1]))
        {
            YgoNet_Cancel(l, "Their deck can't be played.");
            return;
        }
        Begin(l);
        if (l->state == YGO_NET_PLAYING)
            Send(l, START);
        return;
    }
    if (type == START && len == 7 && l->state == YGO_NET_STARTING)
    {
        Begin(l);
        return;
    }
    if (type == ACK && len == 15 && l->state == YGO_NET_PLAYING)
    {
        if (Read(p + 7) == l->sequence && Read(p + 11) == YgoNet_Hash(&l->game))
            l->pending = 0;
        else
            YgoNet_Cancel(l, "The duel fell out of step.");
        return;
    }
    if (type == ANSWER && len >= ANSWER_LEN && l->state == YGO_NET_PLAYING)
    {
        struct YgoAnswer a;
        struct YgoDuel next;
        uint8_t ack[15];
        unsigned concede = p[19] & FLAG_CONCEDE, theirs = l->seat ^ 1;
        int ok;
        if (concede && l->pending)
        {
            /* A concession crossed our own answer: ours never counted on the other side, so drop it (both seats then
             * hold the same duel). Two concessions crossing: seat 0's stands. */
            uint32_t shown = l->game.s.eventSeq;
            if (l->game.s.over && l->game.s.winner == theirs && l->seat == 0)
                return;
            l->game = l->beforePending;
            /* (events go on from what the table already showed; not part of the hash) */
            if (shown > l->game.s.eventSeq)
                l->game.s.eventSeq = shown;
            l->sequence = l->sequenceBefore;
            l->pending = 0;
        }
        else if (l->pending && l->game.s.over && l->game.s.winner == theirs)
            return; /* we conceded first: the other side rolls this answer back */
        if (l->pending || Read(p + 7) != l->sequence || Read(p + 11) != YgoNet_Hash(&l->game) || p[20] > sizeof(a.picks) ||
            len != (unsigned)(ANSWER_LEN + p[20]) || (!concede && (l->game.query.type == YGO_Q_NONE || l->game.query.player != theirs)))
        {
            YgoNet_Cancel(l, "An answer came out of order.");
            return;
        }
        memset(&a, 0, sizeof(a));
        a.count = p[20];
        memcpy(a.picks, p + ANSWER_LEN, a.count);
        next = l->game;
        ok = concede ? (next.s.over || YgoDuel_Concede(&next, theirs)) : YgoDuel_Answer(&next, &a);
        if (!ok || !YgoDuel_Conserved(&next) || YgoNet_Hash(&next) != Read(p + 15))
        {
            YgoNet_Cancel(l, "The two games disagree about the duel.");
            return;
        }
        l->game = next;
        l->sequence++;
        Header(l, ack, ACK);
        Write(ack + 7, l->sequence);
        Write(ack + 11, YgoNet_Hash(&l->game));
        l->send(l->context, l->peer, ack, 15);
    }
}
void YgoNet_Tick(struct YgoNet *l, int connected, uint32_t now)
{
    if (l->state == YGO_NET_IDLE || l->state == YGO_NET_ENDED)
        return;
    if (!connected)
        YgoNet_Cancel(l, "Connection lost. No cards or progress were lost.");
    else if ((l->state != YGO_NET_PLAYING || l->pending) && (int32_t)(now - l->deadline) >= 0)
        YgoNet_Cancel(l, "The duel timed out.");
}
