#ifndef MOEMON_TCG_INTEGRATION_H
#define MOEMON_TCG_INTEGRATION_H
#include "tcg/tcg.h"
#include "tcg/link.h"
struct SDL_Renderer;
struct SDL_Texture;
union SDL_Event;
struct TcgCollection *Tcg_GetCollection(void);
/* The CARD CASE's button on the map: ask the person (or friend) in front for a
 * card duel; 0 when nobody's there (the case opens instead). */
int Tcg_FieldChallenge(void);
int Tcg_FieldAskFriend(void);
/* Card duels and trades with a friend (tcg_online.c): why not, or NULL; then ask. */
const char *Tcg_CanAskFriend(unsigned id);
void Tcg_AskFriend(unsigned id, unsigned kind); /* kind 0 duel, 1 trade */
/* Shown over this player's name to friends: 0 none, 1 a card duel. */
unsigned Tcg_DuelingNow(void);
unsigned Tcg_Today(void);
/* Every card collected: a star on the TRAINER CARD. */
int Tcg_CardMasterStar(void);
/* The printed card pictures are next to the game (the updater downloads them once). */
int Tcg_PrintedArtInstalled(void);
struct TcgLink *Tcg_Online(void);
void Tcg_OnlineRelay(unsigned from, const unsigned char *data, unsigned len);
void Tcg_OnlineTick(void);
int Tcg_OnlineWilling(void);
void Tcg_OpenCase(void (*returnToGame)(void));
int Tcg_IsActive(void);
int Tcg_Step(unsigned keys);
int Tcg_HandleEvent(const union SDL_Event *event);
/* Draws the open card screen over the game picture and returns it (NULL when
 * closed): the display scales it like the game, spectators get Tcg_Frame. */
struct SDL_Texture *Tcg_Canvas(struct SDL_Renderer *renderer, struct SDL_Texture *game);
const unsigned short *Tcg_Frame(void);
/* Players who mustn't see this screen (the online opponent: your hand). */
unsigned Tcg_HiddenFrom(void);
void Tcg_DestroyTextures(void);
void Tcg_OpenChallenge(const char *name, const char *deck, unsigned pack);
void Tcg_Debug(const char *command);
/* Right after the game was saved: the collection goes in "<save>.cards". */
void Tcg_WriteSidecar(void);
#endif
