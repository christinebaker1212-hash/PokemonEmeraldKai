/* Triple Triad games with a friend: the rules and hands once, then the moves (include/tt/net.h). */
#include <stdio.h>
#include <string.h>
#include "tcg/link.h"
#include "tt/net.h"

enum { INVITE, ACCEPT, START, MOVE, ACK, CANCEL };
#define HEAD 8
#define INVITE_LEN (HEAD + 4 + 2 * TT_HAND)
#define ACCEPT_LEN (HEAD + 2 * TT_HAND)
#define MOVE_LEN (HEAD + 15)
#define ACK_LEN (HEAD + 8)
#define FLAG_CONCEDE 1

static uint32_t Read(const uint8_t *p) { return p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24; }
static void Write(uint8_t *p, uint32_t n)
{
    unsigned i;
    for (i = 0; i < 4; i++)
        p[i] = (uint8_t)(n >> (8 * i));
}
static void Header(const struct TtNet *l, uint8_t *p, unsigned kind)
{
    p[0] = TCG_LINK_RELAY;
    p[1] = TCG_LINK_VERSION;
    p[2] = TT_NET_TYPE;
    Write(p + 3, l->session);
    p[7] = (uint8_t)kind;
}
static void Send(struct TtNet *l, unsigned kind)
{
    uint8_t p[HEAD];
    Header(l, p, kind);
    l->send(l->context, l->peer, p, HEAD);
}
/* A hand: five different cards that exist (RANDOM deals past the deck limits, so only that) */
static int HandValid(const uint16_t *hand)
{
    unsigned i, k;
    for (i = 0; i < TT_HAND; i++)
    {
        if (!Tt_Card(hand[i]))
            return 0;
        for (k = 0; k < i; k++)
            if (hand[k] == hand[i])
                return 0;
    }
    return 1;
}
static void PutHand(uint8_t *p, const uint16_t *hand)
{
    unsigned i;
    for (i = 0; i < TT_HAND; i++)
    {
        p[i * 2] = (uint8_t)hand[i];
        p[i * 2 + 1] = (uint8_t)(hand[i] >> 8);
    }
}
static int GetHand(const uint8_t *p, uint16_t *hand)
{
    unsigned i;
    for (i = 0; i < TT_HAND; i++)
        hand[i] = (uint16_t)(p[i * 2] | p[i * 2 + 1] << 8);
    return HandValid(hand);
}

unsigned TtNet_First(uint32_t session) { return (session >> 7) & 1; }
void TtDuel_Concede(struct TtDuel *d, unsigned seat)
{
    if (d->over)
        return;
    d->over = 1;
    d->winner = (uint8_t)(seat ^ 1);
    d->evCount = 0;
}

void TtNet_Init(struct TtNet *l, void (*send)(void *, unsigned, const uint8_t *, unsigned), void *context)
{
    memset(l, 0, sizeof(*l));
    l->send = send;
    l->context = context;
}
void TtNet_Cancel(struct TtNet *l, const char *reason)
{
    if (l->state != TT_NET_IDLE && l->state != TT_NET_ENDED)
        Send(l, CANCEL);
    l->state = TT_NET_ENDED;
    l->pending = 0;
    snprintf(l->error, sizeof(l->error), "%s", reason);
}
static void Begin(struct TtNet *l)
{
    TtDuel_Start(&l->game, l->rules, TT_TRADE_ONE, l->session, l->hand[0], l->hand[1], TtNet_First(l->session));
    l->game.noEvents = 1;
    l->state = TT_NET_PLAYING;
    l->sequence = 0;
    l->pending = 0;
    l->queued = 0;
}
int TtNet_Invite(struct TtNet *l, unsigned peer, uint32_t session, uint32_t rules, const uint16_t *hand, uint32_t now)
{
    uint8_t p[INVITE_LEN];
    if ((l->state != TT_NET_IDLE && l->state != TT_NET_ENDED) || !session || !HandValid(hand))
        return 0;
    l->session = session;
    l->peer = peer;
    l->seat = 0;
    l->rules = rules & ((1u << TT_RULE_COUNT) - 1);
    l->sequence = l->pending = 0;
    l->queued = 0;
    l->error[0] = 0;
    memset(&l->game, 0, sizeof(l->game));
    memcpy(l->hand[0], hand, sizeof(l->hand[0]));
    l->state = TT_NET_SENT;
    l->deadline = now + 120000;
    Header(l, p, INVITE);
    Write(p + HEAD, l->rules);
    PutHand(p + HEAD + 4, hand);
    l->send(l->context, l->peer, p, sizeof(p));
    return 1;
}
int TtNet_Accept(struct TtNet *l, const uint16_t *hand, uint32_t now)
{
    uint8_t p[ACCEPT_LEN];
    if (l->state != TT_NET_INVITED || !HandValid(hand))
        return 0;
    memcpy(l->hand[1], hand, sizeof(l->hand[1]));
    l->state = TT_NET_STARTING;
    l->deadline = now + 30000;
    Header(l, p, ACCEPT);
    PutHand(p + HEAD, hand);
    l->send(l->context, l->peer, p, sizeof(p));
    return 1;
}
static void SendMove(struct TtNet *l, uint32_t before, unsigned slot, unsigned cell, unsigned flags, uint32_t now)
{
    uint8_t p[MOVE_LEN];
    Header(l, p, MOVE);
    Write(p + HEAD, l->sequence++);
    Write(p + HEAD + 4, before);
    Write(p + HEAD + 8, TtDuel_Hash(&l->game));
    p[HEAD + 12] = (uint8_t)flags;
    p[HEAD + 13] = (uint8_t)slot;
    p[HEAD + 14] = (uint8_t)cell;
    l->pending = 1;
    l->deadline = now + 30000;
    l->error[0] = 0;
    l->send(l->context, l->peer, p, sizeof(p));
}
int TtNet_Move(struct TtNet *l, unsigned slot, unsigned cell, uint32_t now)
{
    uint32_t before;
    if (l->state != TT_NET_PLAYING || l->pending || l->game.over || l->game.toMove != l->seat ||
        !TtDuel_CanPlay(&l->game, slot, cell))
        return 0;
    before = TtDuel_Hash(&l->game);
    l->beforePending = l->game;
    l->sequenceBefore = l->sequence;
    TtDuel_Play(&l->game, slot, cell);
    SendMove(l, before, slot, cell, 0, now);
    return 1;
}
int TtNet_Concede(struct TtNet *l, uint32_t now)
{
    uint32_t before;
    if (l->state != TT_NET_PLAYING || l->pending || l->game.over)
        return 0;
    before = TtDuel_Hash(&l->game);
    l->beforePending = l->game;
    l->sequenceBefore = l->sequence;
    TtDuel_Concede(&l->game, l->seat);
    SendMove(l, before, 0, 0, FLAG_CONCEDE, now);
    return 1;
}
int TtNet_Take(struct TtNet *l, unsigned *slot, unsigned *cell)
{
    if (!l->queued)
        return 0;
    *slot = l->queue[0][0];
    *cell = l->queue[0][1];
    memmove(l->queue[0], l->queue[1], (TT_NET_QUEUE - 1) * 2);
    l->queued--;
    return 1;
}
void TtNet_Receive(struct TtNet *l, unsigned from, const uint8_t *p, unsigned len, int willing, uint32_t now)
{
    unsigned kind;
    if (len < HEAD || p[0] != TCG_LINK_RELAY || p[1] != TCG_LINK_VERSION || p[2] != TT_NET_TYPE)
        return;
    kind = p[7];
    if (kind == INVITE)
    {
        uint16_t hand[TT_HAND];
        if (len != INVITE_LEN || !willing || (l->state != TT_NET_IDLE && l->state != TT_NET_ENDED) || !Read(p + 3) ||
            !GetHand(p + HEAD + 4, hand))
            return;
        l->session = Read(p + 3);
        l->peer = from;
        l->seat = 1;
        l->rules = Read(p + HEAD) & ((1u << TT_RULE_COUNT) - 1);
        l->sequence = l->pending = 0;
        l->queued = 0;
        memset(&l->game, 0, sizeof(l->game));
        memcpy(l->hand[0], hand, sizeof(hand));
        l->state = TT_NET_INVITED;
        l->deadline = now + 120000;
        l->error[0] = 0;
        return;
    }
    if (from != l->peer || Read(p + 3) != l->session || l->state == TT_NET_IDLE || l->state == TT_NET_ENDED)
        return;
    if (kind == CANCEL && len == HEAD)
    {
        l->state = TT_NET_ENDED;
        l->pending = 0;
        snprintf(l->error, sizeof(l->error), "The other player left the game.");
        return;
    }
    if (kind == ACCEPT && len == ACCEPT_LEN && l->state == TT_NET_SENT)
    {
        if (!GetHand(p + HEAD, l->hand[1]))
        {
            TtNet_Cancel(l, "Their hand can't be played.");
            return;
        }
        Begin(l);
        Send(l, START);
        return;
    }
    if (kind == START && len == HEAD && l->state == TT_NET_STARTING)
    {
        Begin(l);
        return;
    }
    if (kind == ACK && len == ACK_LEN && l->state == TT_NET_PLAYING)
    {
        if (Read(p + HEAD) == l->sequence && Read(p + HEAD + 4) == TtDuel_Hash(&l->game))
            l->pending = 0;
        else
            TtNet_Cancel(l, "The game fell out of step.");
        return;
    }
    if (kind == MOVE && len == MOVE_LEN && l->state == TT_NET_PLAYING)
    {
        struct TtDuel next;
        uint8_t ack[ACK_LEN];
        unsigned concede = p[HEAD + 12] & FLAG_CONCEDE, theirs = l->seat ^ 1, slot = p[HEAD + 13], cell = p[HEAD + 14];
        if (concede && l->pending)
        {
            /* A concession crossed our own move: ours never counted on the other side, so it's taken back (two
             * concessions crossing: seat 0's stands) */
            if (l->game.over && l->game.winner == theirs && l->seat == 0)
                return;
            l->game = l->beforePending;
            l->sequence = l->sequenceBefore;
            l->pending = 0;
            if (l->queued < TT_NET_QUEUE)
            {
                l->queue[l->queued][0] = 254;   /* (the table takes our own move back) */
                l->queue[l->queued++][1] = 254;
            }
        }
        else if (l->pending && l->game.over && l->game.winner == theirs)
            return;   /* we conceded first: the other side rolls this move back */
        if (l->pending || Read(p + HEAD) != l->sequence || Read(p + HEAD + 4) != TtDuel_Hash(&l->game) ||
            (!concede && l->game.toMove != theirs))
        {
            TtNet_Cancel(l, "A move came out of order.");
            return;
        }
        next = l->game;
        if (concede)
            TtDuel_Concede(&next, theirs);
        else if (!TtDuel_Play(&next, slot, cell))
        {
            TtNet_Cancel(l, "The two games disagree about the game.");
            return;
        }
        if (TtDuel_Hash(&next) != Read(p + HEAD + 8))
        {
            TtNet_Cancel(l, "The two games disagree about the game.");
            return;
        }
        l->game = next;
        l->sequence++;
        if (l->queued < TT_NET_QUEUE)
        {
            l->queue[l->queued][0] = (uint8_t)(concede ? 255 : slot);
            l->queue[l->queued++][1] = (uint8_t)(concede ? 255 : cell);
        }
        Header(l, ack, ACK);
        Write(ack + HEAD, l->sequence);
        Write(ack + HEAD + 4, TtDuel_Hash(&l->game));
        l->send(l->context, l->peer, ack, sizeof(ack));
    }
}
void TtNet_Tick(struct TtNet *l, int connected, uint32_t now)
{
    if (l->state == TT_NET_IDLE || l->state == TT_NET_ENDED)
        return;
    if (!connected)
        TtNet_Cancel(l, "Connection lost.");
    else if ((l->state != TT_NET_PLAYING || l->pending) && (int32_t)(now - l->deadline) >= 0)
        TtNet_Cancel(l, "The game timed out.");
}
