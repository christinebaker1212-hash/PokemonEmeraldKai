#ifndef MOEMON_TCG_INTEGRATION_H
#define MOEMON_TCG_INTEGRATION_H
#include "tcg/tcg.h"
#include "tcg/link.h"
struct SDL_Renderer;
struct SDL_Texture;
union SDL_Event;
struct TcgCollection *Tcg_GetCollection(void);
/* Your record against a card player, in any of the three games (QUOTE_GAME_*: kept in "<save>.cards"), and what they
 * say: before a game (as the game's own text, NULL to keep `usual`) and after it (ASCII, "NAME: ..." on one line). voice
 * QUOTE_VOICE_*, who the special player's name or NULL. */
const struct TcgRecord *CardTable_Record(uint32_t key, unsigned game);
void CardTable_Result(uint32_t key, unsigned game, int won);
const unsigned char *CardTable_Hello(uint32_t key, unsigned game, const char *who, unsigned voice, const unsigned char *usual,
                                     uint32_t pick);
int CardTable_Goodbye(char *out, unsigned long size, const char *name, uint32_t key, unsigned game, const char *who,
                      unsigned voice, int theyWon, uint32_t pick);
/* A map person's kind of talk (QUOTE_VOICE_*), by how they look (tcg_field.c's people) */
unsigned Tcg_VoiceOf(unsigned graphicsId);
struct ObjectEvent;
unsigned Tcg_LookGfx(const struct ObjectEvent *npc);   // Moemon Kai: a Johto / Kanto sprite's Kai look-alike
int Tcg_IsTrainerId(unsigned id);
/* The CARD CASE's button on the map: ask the person (or friend) in front for a
 * card duel; 0 when nobody's there (the case opens instead). */
int Tcg_FieldChallenge(void);
int Tcg_FieldAskFriend(void);
/* Card duels and trades with a friend (tcg_online.c): why not, or NULL; then ask. */
const char *Tcg_CanAskFriend(unsigned id, unsigned kind);   /* kind: TCG_ASK_* (tcg/online.h) */
void Tcg_AskFriend(unsigned id, unsigned kind); /* kind 0 duel, 1 trade */
/* Shown over this player's name to friends: 0 none, 1 a card duel. */
unsigned Tcg_DuelingNow(void);
unsigned Tcg_Today(void);
/* A new duel's seed (all three card games): the collection's generator stirred with the clock, so a save loaded again
 * never deals the same hands (the scripted tests keep theirs) */
uint32_t Tcg_DuelSeed(uint32_t *rng);
/* Every card collected: a star on the TRAINER CARD. */
int Tcg_CardMasterStar(void);
struct TcgLink *Tcg_Online(void);
void Tcg_OnlineRelay(unsigned from, const unsigned char *data, unsigned len);
void Tcg_OnlineTick(void);
int Tcg_OnlineWilling(void);
void Tcg_OpenCase(void (*returnToGame)(void));
void Tcg_OpenGameCenterShop(void);   /* the GAME CENTER CARD SHOP (tcg_ui.c UI_GCSHOP; a script waits) */
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
/* Someone on the map as the card screens show them (their sprite, up to 32x32 ARGB) */
void Tcg_PortraitOfObject(unsigned objectId, uint32_t *pixels, uint8_t *w, uint8_t *h);
void Tcg_Debug(const char *command);
/* Right after the game was saved: the collection goes in "<save>.cards". */
void Tcg_WriteSidecar(void);
#endif
