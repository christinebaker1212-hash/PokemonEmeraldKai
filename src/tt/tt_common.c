// Moemon Kai: Triple Triad's words and limits: the rules' names and what they do, the trade rules, the card types,
// what a deck may hold, and the packs and prices.
#include <stdio.h>
#include "tt/tt.h"

static const char *const sRuleNames[TT_RULE_COUNT] = {
    "OPEN", "SAME", "PLUS", "RANDOM", "SUDDEN DEATH", "SAME WALL", "ELEMENTAL", "THREE OPEN", "REVERSE",
    "FALLEN ACE", "ASCENSION", "DESCENSION", "ORDER", "CHAOS", "SWAP",
};
static const char *const sRuleHelp[TT_RULE_COUNT] = {
    "Both hands are played face up.",
    "Place a card whose numbers match two or more neighbors' facing numbers, and you take them. Taken cards "
    "start a COMBO.",
    "Place a card whose numbers add up to the same total with two or more neighbors' facing numbers, and you take "
    "them. Taken cards start a COMBO.",
    "Your hand is dealt at random from all the cards you own.",
    "A draw is played again, each side holding the cards it ended with.",
    "With SAME, an A facing the edge of the board counts as a match.",
    "Some squares carry a type. A card of that type gets +1 there; any other card gets -1.",
    "Three cards of each hand are played face up.",
    "The lower number takes the card.",
    "A 1 can take an A.",
    "Each card of a type on the board gives that type's cards +1.",
    "Each card of a type on the board gives that type's cards -1.",
    "Cards must be played in the order they are held.",
    "The card to play is chosen at random each turn.",
    "One card of each hand changes sides for the game.",
};
// How a rule leans: +1 rewards the better hand and planning, -1 leaves more to luck
static const signed char sRuleLean[TT_RULE_COUNT] = {1, 1, 1, -1, 0, 1, -1, 1, 0, 0, 0, 0, -1, -1, -1};

const char *Tt_RuleName(unsigned rule) { return rule < TT_RULE_COUNT ? sRuleNames[rule] : "?"; }
const char *Tt_RuleHelp(unsigned rule) { return rule < TT_RULE_COUNT ? sRuleHelp[rule] : ""; }
int Tt_RuleLean(unsigned rule) { return rule < TT_RULE_COUNT ? sRuleLean[rule] : 0; }

static const char *const sTradeNames[TT_TRADE_COUNT] = {"ONE", "DIFF", "DIRECT", "ALL"};
static const char *const sTradeHelp[TT_TRADE_COUNT] = {
    "Win, and take one card of your choice from their hand.",
    "Win, and take as many of their cards as you won by.",
    "Win, and keep every one of their cards you turned to your color.",
    "Win, and take their whole hand.",
};
const char *Tt_TradeName(unsigned trade) { return trade < TT_TRADE_COUNT ? sTradeNames[trade] : "?"; }
const char *Tt_TradeHelp(unsigned trade) { return trade < TT_TRADE_COUNT ? sTradeHelp[trade] : ""; }

const char *Tt_TypeName(unsigned type)
{
    static const char *const sTypes[TT_TYPE_COUNT] = {"", "PRIMAL", "SCION", "SOCIETY", "GARLEAN"};
    return type < TT_TYPE_COUNT ? sTypes[type] : "";
}

int Tt_DeckFits(const uint16_t *cards, unsigned slot, unsigned id, char *reason, size_t reasonLen)
{
    const struct TtCard *c = Tt_Card(id);
    unsigned i, four = 0, five = 0;
    if (!c)
        return 0;
    for (i = 0; i < TT_HAND; i++)
    {
        const struct TtCard *o = i == slot ? NULL : Tt_Card(cards[i]);
        if (!o)
            continue;
        if (cards[i] == id)
        {
            snprintf(reason, reasonLen, "A deck holds only one of each card.");
            return 0;
        }
        four += o->stars >= 4;
        five += o->stars == 5;
    }
    if (c->stars == 5 && five)
    {
        snprintf(reason, reasonLen, "A deck holds only one five-star card.");
        return 0;
    }
    if (c->stars >= 4 && four >= 2)
    {
        snprintf(reason, reasonLen, "A deck holds at most two cards of four stars or more.");
        return 0;
    }
    return 1;
}

int Tt_DeckValid(const uint16_t *cards, char *reason, size_t reasonLen)
{
    unsigned i;
    for (i = 0; i < TT_HAND; i++)
    {
        if (!Tt_Card(cards[i]))
        {
            snprintf(reason, reasonLen, "A deck needs five cards.");
            return 0;
        }
        if (!Tt_DeckFits(cards, i, cards[i], reason, reasonLen))
            return 0;
    }
    return 1;
}

const char *Tt_PackName(unsigned pack)
{
    static const char *const sPacks[TT_PACK_TYPES] = {"BRONZE", "SILVER", "GOLD", "MYTHRIL", "IMPERIAL"};
    return pack < TT_PACK_TYPES ? sPacks[pack] : "?";
}
unsigned Tt_PackPrice(unsigned pack)
{
    static const unsigned sPrices[TT_PACK_SOLD] = {600, 1600, 4000};
    return pack < TT_PACK_SOLD ? sPrices[pack] : 0;
}
unsigned Tt_SinglePrice(unsigned id)
{
    static const unsigned sPrices[TT_MAX_STARS + 1] = {0, 400, 1000, 2600, 0, 0};
    const struct TtCard *c = Tt_Card(id);
    return c && c->stars <= 3 ? sPrices[c->stars] : 0;
}
