#include "mp_version.h"
#include "tcg/link.h"
#include <stdio.h>
#include <string.h>
enum
{
    INVITE = 1,
    ACCEPT,
    START,
    ACTION,
    ACK,
    CANCEL
};
static uint32_t Read(const uint8_t *p)
{
    return p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
}
static void Write(uint8_t *p, uint32_t n)
{
    unsigned i;
    for (i = 0; i < 4; i++)
        p[i] = n >> (8 * i);
}
static void Header(struct TcgLink *l, uint8_t *p, unsigned type)
{
    p[0] = TCG_LINK_RELAY;
    p[1] = TCG_LINK_VERSION;
    p[2] = type;
    Write(p + 3, l->session);
}
static void Send(struct TcgLink *l, unsigned type)
{
    uint8_t p[7];
    Header(l, p, type);
    l->send(l->context, l->peer, p, 7);
}
static void SendDeck(struct TcgLink *l, unsigned type, const uint16_t *deck)
{
    uint8_t p[127];
    unsigned i;
    Header(l, p, type);
    for (i = 0; i < 60; i++)
    {
        p[7 + i * 2] = deck[i];
        p[8 + i * 2] = deck[i] >> 8;
    }
    l->send(l->context, l->peer, p, sizeof(p));
}
static int ReadDeck(const uint8_t *p, uint16_t *deck)
{
    unsigned i;
    char reason[128];
    for (i = 0; i < 60; i++)
        deck[i] = p[7 + i * 2] | p[8 + i * 2] << 8;
    return Tcg_DeckValidate(deck, NULL, reason, sizeof(reason));
}
/* Hash fields explicitly so padding, byte order and compiler ABI cannot diverge. */
uint32_t TcgLink_Hash(const struct TcgDuel *g)
{
    uint32_t h = 2166136261u;
    unsigned i, j, k;
#define H(v)                                                                                                           \
    do                                                                                                                 \
    {                                                                                                                  \
        uint32_t n = (v);                                                                                              \
        unsigned b;                                                                                                    \
        for (b = 0; b < 4; b++)                                                                                        \
        {                                                                                                              \
            h = (h ^ (n & 255)) * 16777619u;                                                                           \
            n >>= 8;                                                                                                   \
        }                                                                                                              \
    } while (0)
    H(g->rng);
    H(g->turn);
    H(g->current);
    H(g->first);
    H(g->phase);
    H(g->winner);
    H(g->prizeCount);
    H(g->promotePlayer);
    H(g->needsTurnStart);
    H(g->switchPending);
    H(g->endAfterSwitch);
    for (i = 0; i < 2; i++)
    {
        const struct TcgPlayer *p = &g->players[i];
        H(p->deckSize);
        H(p->handSize);
        H(p->discardSize);
        H(p->prizeSize);
        H(p->energyPlayed);
        H(p->confusionRetreatFailed);
        H(p->setupDone);
        H(p->retreated);
        H(p->noTrainers);
        H(p->turnsTaken);
        for (j = 0; j < 60; j++)
        {
            H(p->cards[j]);
            H(p->attachmentExpires[j]);
        }
        for (j = 0; j < p->deckSize; j++)
            H(p->deck[j]);
        for (j = 0; j < p->handSize; j++)
            H(p->hand[j]);
        for (j = 0; j < p->discardSize; j++)
            H(p->discard[j]);
        for (j = 0; j < p->prizeSize; j++)
            H(p->prizes[j]);
        for (j = 0; j < 6; j++)
        {
            const struct TcgFieldCard *f = &p->field[j];
            H(f->stackSize);
            H(f->attachedSize);
            H(f->damage);
            H(f->enteredTurn);
            H(f->condition);
            H(f->poison);
            H(f->noPrize);
            H(f->barrier);
            H(f->noRetreat);
            H(f->noAttack);
            H(f->plusPower);
            H(f->defender);
            H(f->powerUsed);
            H(f->guard);
            H(f->weaken);
            H(f->charged);
            H(f->usedOnce);
            H(f->effectBy);
            H(f->burn);
            H(f->shift);
            for (k = 0; k < f->stackSize; k++)
                H(f->stack[k]);
            for (k = 0; k < f->attachedSize; k++)
                H(f->attached[k]);
        }
    }
#undef H
    return h;
}
void TcgLink_Init(struct TcgLink *l, void (*send)(void *, unsigned, const uint8_t *, unsigned), void *ctx)
{
    memset(l, 0, sizeof(*l));
    l->send = send;
    l->context = ctx;
}
void TcgLink_Cancel(struct TcgLink *l, const char *reason)
{
    if (l->state != TCG_LINK_IDLE && l->state != TCG_LINK_ENDED)
        Send(l, CANCEL);
    l->state = TCG_LINK_ENDED;
    l->pending = 0;
    snprintf(l->error, sizeof(l->error), "%s", reason);
}
int TcgLink_Invite(struct TcgLink *l, unsigned peer, uint32_t session, const uint16_t *deck, uint32_t now)
{
    char reason[128];
    if ((l->state != TCG_LINK_IDLE && l->state != TCG_LINK_ENDED) || !session ||
        !Tcg_DeckValidate(deck, NULL, reason, sizeof(reason)))
        return 0;
    l->session = session;
    l->peer = peer;
    l->seat = 0;
    l->sequence = l->pending = 0;
    l->error[0] = 0;
    memcpy(l->decks[0], deck, 120);
    l->state = TCG_LINK_SENT;
    l->deadline = now + 120000;
    SendDeck(l, INVITE, deck);
    return 1;
}
int TcgLink_Accept(struct TcgLink *l, const uint16_t *deck, uint32_t now)
{
    char reason[128];
    if (l->state != TCG_LINK_INVITED || !Tcg_DeckValidate(deck, NULL, reason, sizeof(reason)))
        return 0;
    memcpy(l->decks[1], deck, 120);
    l->state = TCG_LINK_STARTING;
    l->deadline = now + 30000;
    SendDeck(l, ACCEPT, deck);
    return 1;
}
static unsigned Actor(const struct TcgDuel *g) { return g->phase == TCG_PROMOTION ? g->promotePlayer : g->current; }
int TcgLink_Submit(struct TcgLink *l, const struct TcgAction *a, uint32_t now)
{
    uint8_t p[85];
    uint32_t before;
    unsigned i;
    if (l->state != TCG_LINK_PLAYING || l->pending || (Actor(&l->game) != l->seat && a->type != TCG_CONCEDE) ||
        a->selectionCount > 60 || l->game.phase == TCG_FINISHED)
        return 0;
    before = TcgLink_Hash(&l->game);
    l->beforePending = l->game;
    l->sequenceBefore = l->sequence;
    if (a->type == TCG_CONCEDE)
    {
        l->game.winner = l->seat ^ 1;
        l->game.phase = TCG_FINISHED;
    }
    else if (TcgDuel_Apply(&l->game, a) != TCG_OK)
    {
        snprintf(l->error, sizeof(l->error), "%s", l->game.error);
        return 0;
    }
    Header(l, p, ACTION);
    Write(p + 7, l->sequence++);
    Write(p + 11, before);
    Write(p + 15, TcgLink_Hash(&l->game));
    p[19] = a->type;
    p[20] = a->hand;
    p[21] = a->slot;
    p[22] = a->attack;
    p[23] = a->target;
    p[24] = a->selectionCount;
    for (i = 0; i < a->selectionCount; i++)
        p[25 + i] = a->selections[i];
    l->pending = 1;
    l->deadline = now + 30000;
    l->error[0] = 0;
    l->send(l->context, l->peer, p, 25 + a->selectionCount);
    return 1;
}
void TcgLink_Receive(struct TcgLink *l, unsigned from, const uint8_t *p, unsigned len, int willing, uint32_t now)
{
    unsigned type;
    if (len < 7 || p[0] != TCG_LINK_RELAY || p[1] != TCG_LINK_VERSION)
        return;
    type = p[2];
    if (type == INVITE)
    {
        uint16_t deck[60];
        if (len != 127 || !willing || (l->state != TCG_LINK_IDLE && l->state != TCG_LINK_ENDED) || !Read(p + 3) ||
            !ReadDeck(p, deck))
            return;
        l->session = Read(p + 3);
        l->peer = from;
        l->seat = 1;
        l->sequence = l->pending = 0;
        memcpy(l->decks[0], deck, 120);
        l->state = TCG_LINK_INVITED;
        l->deadline = now + 120000;
        l->error[0] = 0;
        return;
    }
    if (from != l->peer || Read(p + 3) != l->session || l->state == TCG_LINK_IDLE || l->state == TCG_LINK_ENDED)
        return;
    if (type == CANCEL && len == 7)
    {
        l->state = TCG_LINK_ENDED;
        l->pending = 0;
        snprintf(l->error, sizeof(l->error), "Other player ended the card session.");
        return;
    }
    if (type == ACCEPT && len == 127 && l->state == TCG_LINK_SENT)
    {
        if (!ReadDeck(p, l->decks[1]) || !TcgDuel_Init(&l->game, l->decks[0], l->decks[1], l->session, 6))
        {
            TcgLink_Cancel(l, "Invalid opponent deck.");
            return;
        }
        l->state = TCG_LINK_PLAYING;
        Send(l, START);
        return;
    }
    if (type == START && len == 7 && l->state == TCG_LINK_STARTING)
    {
        if (!TcgDuel_Init(&l->game, l->decks[0], l->decks[1], l->session, 6))
        {
            TcgLink_Cancel(l, "Could not initialize duel.");
            return;
        }
        l->state = TCG_LINK_PLAYING;
        return;
    }
    if (type == ACK && len == 15 && l->state == TCG_LINK_PLAYING)
    {
        if (Read(p + 7) == l->sequence && Read(p + 11) == TcgLink_Hash(&l->game))
            l->pending = 0;
        else
            TcgLink_Cancel(l, "Card duel lost synchronization.");
        return;
    }
    if (type == ACTION && len >= 25 && l->state == TCG_LINK_PLAYING)
    {
        struct TcgAction a = {0};
        uint8_t ack[15];
        struct TcgDuel next;
        if (p[19] == TCG_CONCEDE && l->pending)
        {
            /* A concession crossed our own action: ours never reached the
             * other side, so drop it (both seats then hold the same duel).
             * Two concessions crossing: seat 0's stands. */
            if (l->game.phase == TCG_FINISHED && l->game.winner == (l->seat ^ 1) && l->seat == 0)
                return;
            l->game = l->beforePending;
            l->sequence = l->sequenceBefore;
            l->pending = 0;
        }
        else if (l->pending && l->game.phase == TCG_FINISHED && l->game.winner == (l->seat ^ 1))
            return; /* we conceded first: the other side rolls this action back */
        if (l->pending || Read(p + 7) != l->sequence || Read(p + 11) != TcgLink_Hash(&l->game) ||
            (Actor(&l->game) == l->seat && p[19] != TCG_CONCEDE) || p[24] > 60 || len != 25u + p[24])
        {
            TcgLink_Cancel(l, "Invalid or out-of-order card action.");
            return;
        }
        a.type = p[19];
        a.hand = p[20];
        a.slot = p[21];
        a.attack = p[22];
        a.target = p[23];
        a.selectionCount = p[24];
        memcpy(a.selections, p + 25, a.selectionCount);
        next = l->game;
        int result = TCG_OK;
        if (a.type == TCG_CONCEDE && next.phase != TCG_FINISHED)
        {
            next.winner = l->seat;
            next.phase = TCG_FINISHED;
        }
        else
            result = TcgDuel_Apply(&next, &a);
        if (result != TCG_OK || !TcgDuel_Conserved(&next) || TcgLink_Hash(&next) != Read(p + 15))
        {
            TcgLink_Cancel(l, "Card duel rules mismatch.");
            return;
        }
        l->game = next;
        l->sequence++;
        Header(l, ack, ACK);
        Write(ack + 7, l->sequence);
        Write(ack + 11, TcgLink_Hash(&l->game));
        l->send(l->context, l->peer, ack, 15);
    }
}
void TcgLink_Tick(struct TcgLink *l, int connected, uint32_t now)
{
    if (l->state == TCG_LINK_IDLE || l->state == TCG_LINK_ENDED)
        return;
    if (!connected)
        TcgLink_Cancel(l, "Connection lost. No cards or progress were lost.");
    else if ((l->state != TCG_LINK_PLAYING || l->pending) && (int32_t)(now - l->deadline) >= 0)
        TcgLink_Cancel(l, "Card session timed out.");
}
