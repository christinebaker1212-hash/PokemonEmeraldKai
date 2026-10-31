// Moemon Fight: which engine a VS cabinet runs. The GAME CENTER's cabinets run either the MUGEN engine (fight/,
// include/mf_engine.h) or a CPS-2 board (include/arcade.h through src/platform/arcade_fight.c, which gives the board
// the same calls as the MUGEN engine: sessions, frames, rollback, desync recovery, picture and sound). mp_fight.c
// and fight_platform.c call FightEngine_*: gFightKind says which one answers (COOP_NOTES "The GAME CENTER's
// arcade cabinets").
#ifndef GUARD_FIGHT_ENGINE_H
#define GUARD_FIGHT_ENGINE_H

#include "mf_engine.h"

enum
{
    FIGHT_KIND_MUGEN,
    FIGHT_KIND_ARCADE,      // + the arcade game (ARCADE_*): FIGHT_KIND_ARCADE + ARCADE_CYBERBOTS...
};

extern unsigned char gFightKind;

// The arcade board as a fight engine (src/platform/arcade_fight.c); the same meanings as MfEngine_*
int ArcFight_PackSummary(int game, struct MfPack *out);
int ArcFight_Begin(int game, const struct MfSession *session);
int ArcFight_IsActive(void);
int ArcFight_Step(const unsigned short buttons[2]);
unsigned int ArcFight_FrameCount(void);
int ArcFight_Kept(unsigned int *oldest);
int ArcFight_RollBack(unsigned int frame);
void ArcFight_StepQuiet(const unsigned short buttons[2]);
unsigned int ArcFight_Hash(void);
int ArcFight_StateInfo(struct MfStateInfo *out);
int ArcFight_StateHashes(int region, unsigned int first, unsigned int count, unsigned int *out);
int ArcFight_StatePage(int region, unsigned int index, unsigned char *out);
int ArcFight_StateBegin(void);
int ArcFight_StatePut(int region, unsigned int index, const unsigned char *page);
int ArcFight_StateEnd(const struct MfStateInfo *from);
void ArcFight_TestPoke(int delta);
const unsigned short *ArcFight_Frame(void);
unsigned int ArcFight_FrameSerial(void);
int ArcFight_Result(void);
void ArcFight_Cancel(void);
const char *ArcFight_Error(void);
void ArcFight_MixAudio(short *stereo, int frames, int rate);

#define FE_ARCADE (gFightKind >= FIGHT_KIND_ARCADE)
#define FE_GAME ((int)gFightKind - FIGHT_KIND_ARCADE)

static inline int FightEngine_PackSummary(const char *cabinet, struct MfPack *out)
{
    return FE_ARCADE ? ArcFight_PackSummary(FE_GAME, out) : MfEngine_PackSummary(cabinet, out);
}
static inline int FightEngine_CabinetText(const char *cabinet, char *out, int size)
{
    if (FE_ARCADE)
    {
        if (size > 0)
            out[0] = 0;
        return 0;
    }
    return MfEngine_CabinetText(cabinet, out, size);
}
static inline int FightEngine_Begin(const struct MfSession *s)
{
    return FE_ARCADE ? ArcFight_Begin(FE_GAME, s) : MfEngine_Begin(s);
}
static inline int FightEngine_IsActive(void) { return FE_ARCADE ? ArcFight_IsActive() : MfEngine_IsActive(); }
static inline int FightEngine_Step(const unsigned short b[2]) { return FE_ARCADE ? ArcFight_Step(b) : MfEngine_Step(b); }
static inline int FightEngine_Ready(void) { return FE_ARCADE ? ArcFight_IsActive() : MfEngine_Ready(); }
static inline unsigned int FightEngine_FrameCount(void)
{
    return FE_ARCADE ? ArcFight_FrameCount() : MfEngine_FrameCount();
}
static inline int FightEngine_Kept(unsigned int *oldest)
{
    return FE_ARCADE ? ArcFight_Kept(oldest) : MfEngine_Kept(oldest);
}
static inline int FightEngine_RollBack(unsigned int f) { return FE_ARCADE ? ArcFight_RollBack(f) : MfEngine_RollBack(f); }
static inline void FightEngine_StepQuiet(const unsigned short b[2])
{
    if (FE_ARCADE)
        ArcFight_StepQuiet(b);
    else
        MfEngine_StepQuiet(b);
}
static inline unsigned int FightEngine_Hash(void) { return FE_ARCADE ? ArcFight_Hash() : MfEngine_Hash(); }
static inline int FightEngine_StateInfo(struct MfStateInfo *out)
{
    return FE_ARCADE ? ArcFight_StateInfo(out) : MfEngine_StateInfo(out);
}
static inline int FightEngine_StateHashes(int region, unsigned int first, unsigned int count, unsigned int *out)
{
    return FE_ARCADE ? ArcFight_StateHashes(region, first, count, out) : MfEngine_StateHashes(region, first, count, out);
}
static inline int FightEngine_StatePage(int region, unsigned int index, unsigned char *out)
{
    return FE_ARCADE ? ArcFight_StatePage(region, index, out) : MfEngine_StatePage(region, index, out);
}
static inline int FightEngine_StateBegin(void) { return FE_ARCADE ? ArcFight_StateBegin() : MfEngine_StateBegin(); }
static inline int FightEngine_StatePut(int region, unsigned int index, const unsigned char *page)
{
    return FE_ARCADE ? ArcFight_StatePut(region, index, page) : MfEngine_StatePut(region, index, page);
}
static inline int FightEngine_StateEnd(const struct MfStateInfo *from)
{
    return FE_ARCADE ? ArcFight_StateEnd(from) : MfEngine_StateEnd(from);
}
static inline void FightEngine_TestPoke(int delta)
{
    if (FE_ARCADE)
        ArcFight_TestPoke(delta);
    else
        MfEngine_TestPoke(delta);
}
static inline const unsigned short *FightEngine_Frame(void) { return FE_ARCADE ? ArcFight_Frame() : MfEngine_Frame(); }
static inline unsigned int FightEngine_FrameSerial(void)
{
    return FE_ARCADE ? ArcFight_FrameSerial() : MfEngine_FrameSerial();
}
static inline int FightEngine_Result(void) { return FE_ARCADE ? ArcFight_Result() : MfEngine_Result(); }
static inline void FightEngine_Cancel(void)
{
    if (FE_ARCADE)
        ArcFight_Cancel();
    else
        MfEngine_Cancel();
}
static inline const char *FightEngine_Error(void) { return FE_ARCADE ? ArcFight_Error() : MfEngine_Error(); }
static inline void FightEngine_MixAudio(short *stereo, int frames, int rate)
{
    if (FE_ARCADE)
        ArcFight_MixAudio(stereo, frames, rate);
    else
        MfEngine_MixAudio(stereo, frames, rate);
}

#endif // GUARD_FIGHT_ENGINE_H
