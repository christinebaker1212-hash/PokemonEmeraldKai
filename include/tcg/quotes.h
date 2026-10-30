#ifndef MOEMON_TCG_QUOTES_H
#define MOEMON_TCG_QUOTES_H
/* What card players say before and after a game, in all three card games (src/tcg/card_quotes.c): by their kind
 * (or as themselves, for the special players: by name), and by your record against them. Friendly, competitive,
 * never unkind. */
#include <stddef.h>
#include <stdint.h>
enum
{
    QUOTE_VOICE_KID,
    QUOTE_VOICE_STYLISH,
    QUOTE_VOICE_TOUGH,
    QUOTE_VOICE_SEA,
    QUOTE_VOICE_REFINED,
    QUOTE_VOICE_BRAINY,
    QUOTE_VOICE_TEAM,
    QUOTE_VOICE_STAFF,
    QUOTE_VOICE_COUNT
};
enum
{
    QUOTE_GAME_POKEMON,
    QUOTE_GAME_YGO,
    QUOTE_GAME_TT
};
enum
{
    QUOTE_HELLO,     /* before the game */
    QUOTE_THEY_WON,  /* after it: they won */
    QUOTE_THEY_LOST  /* after it: you won */
};
/* The line, in ASCII ('\n' a new line, '\f' a new page; at most 36 letters a line). special: the special player's name
 * as the game shows it ("ROXANNE", "TATE & LIZA", "CARD QUEEN"...), NULL for anyone else. wins / losses: your record
 * against them, before the game for QUOTE_HELLO, with this game counted for the others. pick: a number of theirs
 * (the same number, the same line). Returns 0 (and "") when there's nothing to say. */
int CardQuote(char *out, size_t size, const char *special, unsigned voice, unsigned game, unsigned when, unsigned wins,
              unsigned losses, uint32_t pick);
/* The same line on one row, for the end-of-game boxes ('\n' and '\f' as spaces) */
void CardQuote_OneLine(char *text);
#endif
