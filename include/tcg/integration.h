#ifndef MOEMON_TCG_INTEGRATION_H
#define MOEMON_TCG_INTEGRATION_H
#include "mp_version.h"
#if MOEMON_DEV_BUILD
#include "tcg/tcg.h"
#include "tcg/link.h"
struct SDL_Renderer;
union SDL_Event;
struct TcgCollection *Tcg_GetCollection(void);
int Tcg_TryChallenge(void);
struct TcgLink *Tcg_Online(void);
void Tcg_OnlineRelay(unsigned from, const unsigned char *data, unsigned len);
void Tcg_OnlineTick(void);
int Tcg_OnlineWilling(void);
void Tcg_OpenCase(void (*returnToGame)(void));
int Tcg_IsActive(void);
int Tcg_Step(unsigned keys);
int Tcg_HandleEvent(const union SDL_Event *event);
void Tcg_Render(struct SDL_Renderer *renderer, int width, int height);
void Tcg_DestroyTextures(void);
void Tcg_OpenChallenge(const char *name, const char *deck, unsigned pack);
void Tcg_Debug(const char *command);
#else
#define Tcg_TryChallenge() 0
#define Tcg_OnlineTick() ((void)0)
#define Tcg_OnlineRelay(from, data, len) ((void)0)
#define Tcg_IsActive() 0
#define Tcg_Step(keys) 0
#define Tcg_HandleEvent(event) 0
#define Tcg_Render(renderer, width, height) ((void)0)
#define Tcg_DestroyTextures() ((void)0)
#endif
#endif
