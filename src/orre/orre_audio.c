// ORRE's sound in the game's audio frame (music_player.c): Colosseum's own songs and effects (include/orre_sound.h,
// musyx/orre_musyx.c) play while the player is in ORRE, through its menus too; a battle has Kai's own music, the
// song waiting where it is; leaving ORRE ends everything
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "main.h"
#include "o3d.h"
#include "orre_sound.h"

bool8 Orre_BattleSongOn(void);   // orre_battle.c

// music/orre_gains.txt (tools/kaisong/normalize.py, at build time): "<sound id>\t<dB>" a line, each of Colosseum's songs
// as loud as the game's other music; "fx" the gain while no song plays
const char *MP_Kaisong_MusicDir(void);

static void ReadGains(void)
{
    char path[600], line[100];
    FILE *fp;
    snprintf(path, sizeof(path), "%s/orre_gains.txt", MP_Kaisong_MusicDir());
    if ((fp = fopen(path, "r")) == NULL)
        return;
    while (fgets(line, sizeof(line), fp))
    {
        char *tab = strchr(line, '\t');
        if (line[0] == '#' || tab == NULL)
            continue;
        OrreSound_SetGain(strncmp(line, "fx", 2) == 0 ? 0 : (u32)atoi(line), (float)atof(tab + 1));
    }
    fclose(fp);
}

void Orre_AudioFrame(float *buffer, s32 count)
{
    static bool8 sWasInOrre, sGains;
    if (!sGains)
        sGains = TRUE, ReadGains();
    bool8 inOrre = gSaveBlock1Ptr != NULL && O3D_IsOrreGroup(gSaveBlock1Ptr->location.mapGroup)
                && gMain.callback2 != NULL;
    if (sWasInOrre && !inOrre)
        OrreSound_Silence();
    sWasInOrre = inOrre;
    // (a battle: Colosseum's own song for it when it has one, orre_battle.c; else Kai's, the room's song waiting)
    OrreSound_Mix(buffer, count, 42048, inOrre && (!gMain.inBattle || Orre_BattleSongOn()));
}
