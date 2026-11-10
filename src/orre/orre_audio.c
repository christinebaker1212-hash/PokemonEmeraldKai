// ORRE's sound in the game's audio frame (music_player.c): Colosseum's own songs and effects (include/orre_sound.h,
// musyx/orre_musyx.c) play while the player is in ORRE, through its menus too; a battle has Kai's own music, the
// song waiting where it is; leaving ORRE ends everything
#include "global.h"
#include "main.h"
#include "o3d.h"
#include "orre_sound.h"

bool8 Orre_BattleSongOn(void);   // orre_battle.c

void Orre_AudioFrame(float *buffer, s32 count)
{
    static bool8 sWasInOrre;
    bool8 inOrre = gSaveBlock1Ptr != NULL && O3D_IsOrreGroup(gSaveBlock1Ptr->location.mapGroup)
                && gMain.callback2 != NULL;
    if (sWasInOrre && !inOrre)
        OrreSound_Silence();
    sWasInOrre = inOrre;
    // (a battle: Colosseum's own song for it when it has one, orre_battle.c; else Kai's, the room's song waiting)
    OrreSound_Mix(buffer, count, 42048, inOrre && (!gMain.inBattle || Orre_BattleSongOn()));
}
