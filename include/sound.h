#ifndef GUARD_SOUND_H
#define GUARD_SOUND_H

#include "constants/sound.h"

void InitMapMusic(void);
void MapMusicMain(void);
void ResetMapMusic(void);
u16 GetCurrentMapMusic(void);
void PlayNewMapMusic(u16 songNum);
void StopMapMusic(void);
void FadeOutMapMusic(u8 speed);
void FadeOutAndPlayNewMapMusic(u16 songNum, u8 speed);
void FadeOutAndFadeInNewMapMusic(u16 songNum, u8 fadeOutSpeed, u8 fadeInSpeed);
bool8 IsNotWaitingForBGMStop(void);
void PlayFanfareByFanfareNum(u8 fanfareNum);
bool8 WaitFanfare(bool8 stop);
void StopFanfareByFanfareNum(u8 fanfareNum);
void PlayFanfare(u16 songNum);
bool8 IsFanfareTaskInactive(void);
void MP_Sound_TickFanfare(void);
void MP_Sound_EndFanfare(void);
void FadeInNewBGM(u16 songNum, u8 speed);
void FadeOutBGMTemporarily(u8 speed);
bool8 IsBGMPausedOrStopped(void);
void FadeInBGM(u8 speed);
void FadeOutBGM(u8 speed);
bool8 IsBGMStopped(void);
void PlayCry_Normal(u16 species, s8 pan);
void PlayCry_NormalNoDucking(u16 species, s8 pan, s8 volume, u8 priority);
void PlayCry_ByMode(u16 species, s8 pan, u8 mode);
void PlayCry_ReleaseDouble(u16 species, s8 pan, u8 mode);
void PlayCry_Script(u16 species, u8 mode);
void PlayCry_DuckNoRestore(u16 species, s8 pan, u8 mode);
void PlayCryInternal(u16 species, s8 pan, s8 volume, u8 priority, u8 mode);
bool8 IsCryFinished(void);
void StopCryAndClearCrySongs(void);
void StopCry(void);
bool8 IsCryPlayingOrClearCrySongs(void);
bool8 IsCryPlaying(void);
void PlayBGM(u16 songNum);
void PlaySE(u16 songNum);
void PlaySE12WithPanning(u16 songNum, s8 pan);
void PlaySE1WithPanning(u16 songNum, s8 pan);
void PlaySE2WithPanning(u16 songNum, s8 pan);
void SE12PanpotControl(s8 pan);
bool8 IsSEPlaying(void);
bool8 IsBGMPlaying(void);
bool8 IsSpecialSEPlaying(void);

// Moemon Kai: songs remember which scene asked for them (custom music: include/mp_kaisong_scene.h)
#include "mp_kaisong_scene.h"
#ifndef MP_KAISONG_NO_SCENE
#define PlayBGM(n) MP_KAISONG_WRAP(PlayBGM(n))
#define PlayNewMapMusic(n) MP_KAISONG_WRAP(PlayNewMapMusic(n))
#define FadeOutAndPlayNewMapMusic(n, s) MP_KAISONG_WRAP(FadeOutAndPlayNewMapMusic(n, s))
#define FadeOutAndFadeInNewMapMusic(n, o, i) MP_KAISONG_WRAP(FadeOutAndFadeInNewMapMusic(n, o, i))
#define FadeInNewBGM(n, s) MP_KAISONG_WRAP(FadeInNewBGM(n, s))
#define PlayFanfare(n) MP_KAISONG_WRAP(PlayFanfare(n))
#define PlayFanfareByFanfareNum(n) MP_KAISONG_WRAP(PlayFanfareByFanfareNum(n))
#endif

#endif // GUARD_SOUND_H
