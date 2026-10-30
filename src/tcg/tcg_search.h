/* SEARCH & SORT (src/tcg/tcg_search.c): one menu for the three card games' binders and deck builders, opened with
 * {SEARCH} (L2 / LT / ZL, or F on the keyboard). A name search (any part, any case), a sort and its order, OWNED ONLY,
 * and each game's own filters, all at once; they only change what the list shows and in which order (never a deck, an
 * owned count or a card). Each game's settings are kept apart and remembered ("<save>.search"). */
#ifndef MOEMON_TCG_SEARCH_H
#define MOEMON_TCG_SEARCH_H
#include <limits.h>
#include <stdint.h>

enum { SEARCH_POKEMON, SEARCH_YUGIOH, SEARCH_TRIAD, SEARCH_GAMES };
#define SEARCH_NA INT_MIN        /* a value the card doesn't have (a Trainer's HP): no filter on it matches */
#define SEARCH_ALL (-32768)      /* a choice's value that lets every card through (Triple Triad's ALL STARS) */
#define SEARCH_FILTERS_MAX 12
#define SEARCH_TEXT_MAX 16
/* The keys every game's value() answers */
enum { SEARCH_KEY_NUMBER = 0, SEARCH_KEY_RARITY = 1, SEARCH_KEY_GAME = 2 /* (its own from here) */ };

enum
{
    SF_CHOICE,   /* ANY, then each of names[] (matching values[i], or i when values is NULL) */
    SF_MIN,      /* ANY, then "at least" lo, lo + step... (count of them; names[], when given, are what they say) */
    SF_MAX       /* ANY, then "at most" */
};
struct SearchFilter
{
    const char *label;
    uint8_t kind, key, count;
    const char *const *names;
    const int16_t *values;
    int16_t lo, step;
    const char *anyName;         /* what choice 0 says (NULL: "ANY") */
};
struct SearchSort
{
    const char *name;
    int8_t key;                  /* -1: by name, -2: by copies owned, else value()'s key */
};
struct SearchGame
{
    const char *id;              /* in the settings file */
    const struct SearchFilter *filters;
    uint8_t filterCount;
    const struct SearchSort *sorts;   /* the first is the default (the catalog's order) */
    uint8_t sortCount;
    const char *(*name)(unsigned id);
    int (*value)(unsigned id, int key);
    unsigned (*owned)(unsigned id);
};

void TcgSearch_Register(unsigned game, const struct SearchGame *g);
void TcgSearch_Open(unsigned game);
int TcgSearch_Active(void);
/* The menu's buttons; 1 when what the list shows changed (keep the cursor on its card) */
int TcgSearch_Step(unsigned pressed);
/* The menu over the list; count: the cards it shows now */
void TcgSearch_Draw(unsigned count);
/* ids (the list as the screen would show it): what passes the search and filters, in the chosen order; returns how many */
unsigned TcgSearch_Apply(unsigned game, uint16_t *ids, unsigned n);
/* Anything narrowing the list (search, OWNED ONLY, a filter): the active-filter mark */
int TcgSearch_Narrowed(unsigned game);
/* Changes whenever the settings do (for a screen's cached list) */
unsigned TcgSearch_Generation(unsigned game);
/* A filter's choice (0 = its ANY / BY TAB) */
int TcgSearch_Choice(unsigned game, int filter);
/* The mark on a list screen: what's narrowing it and how many cards are left (drawn only when narrowed or sorted);
 * a negative x puts its right edge there */
void TcgSearch_Badge(unsigned game, int x, int y, unsigned count);
/* An empty list's words: how to get cards back */
void TcgSearch_Empty(unsigned game, int cx, int y);
void TcgSearch_SetChoice(unsigned game, int filter, int choice);
void TcgSearch_ClearAll(unsigned game);
/* The test harness: "mp cards search TEXT" and the like */
void TcgSearch_Debug(unsigned game, const char *what);
#endif
