// Moemon Kai: a catch in a battle that goes on (a co-op wild battle with the other POKeMON still out, ORRE's snags in
// trainer battles). The battle's song (Kai's, or Colosseum's in ORRE) keeps playing, quieter, under the catch's jingle,
// and comes back up where it has got to: never stopped, never started again (the user, 2026-11-11: it used to play the
// caught song over the battle's, then start HOENN's trainer battle song from the top)
#include "global.h"
#include "battle.h"
#include "main.h"
#include "m4a.h"
#include "sound.h"
#include "task.h"
#include "mp.h"
#include "orre_sound.h"
#include "constants/songs.h"

#define DUCK_VOLUME 72      // the battle's song under the jingle (of 256)
#define JINGLE_MAX 420      // frames at most (the jingle's own end comes first)

static void Task_CatchJingle(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    // (the jingle: FireRed's catch fanfare on the SE2 player, or a GENESIS / EPOCH BALL's own on the effects)
    bool8 sounding = (gMPlayInfo_SE2.status & MUSICPLAYER_STATUS_TRACK) && !(gMPlayInfo_SE2.status & MUSICPLAYER_STATUS_PAUSE);
    if (++data[0] < 20 || (sounding && data[0] < JINGLE_MAX))
    {
        m4aMPlayVolumeControl(&gMPlayInfo_BGM, TRACKS_ALL, DUCK_VOLUME);   // (a cry's ducking ends at full: kept down)
        return;
    }
    m4aMPlayVolumeControl(&gMPlayInfo_BGM, TRACKS_ALL, 256);
    OrreSound_Duck(1.0f);
    MPNet_Log("catch jingle: the song back up after %d frames (song %p, playing %d)", data[0], (void *)gMPlayInfo_BGM.songHeader,
              (gMPlayInfo_BGM.status & MUSICPLAYER_STATUS_TRACK) && !(gMPlayInfo_BGM.status & MUSICPLAYER_STATUS_PAUSE));
    DestroyTask(taskId);
}

// whether the battle goes on after TARGET is caught: a co-op wild battle with another wild POKeMON still out, or a
// trainer battle (ORRE's snags). A co-op wild battle is a link battle flagged as a trainer one, so that flag alone
// isn't one that goes on (its last catch ends it, with the caught song)
bool8 MP_CatchJingle_GoesOn(u8 target)
{
    if (gMpCoopWild)
        return MP_Coop_OtherWildOut(target);
    return (gBattleTypeFlags & BATTLE_TYPE_TRAINER) != 0;
}

// OWN_JINGLE: the ball's own jingle already plays (a GENESIS / EPOCH BALL): only the song goes down
static u32 sStartedAt;   // (the frame the last jingle started: the ball's click starts it, the "Gotcha!" after it
                         // doesn't start it again)

void MP_CatchJingle_Start(bool8 ownJingle)
{
    if (FuncIsActiveTask(Task_CatchJingle))
        return;
    if (!ownJingle && sStartedAt != 0 && gMain.vblankCounter1 - sStartedAt < 900)
        return;
    if (!ownJingle)
        sStartedAt = gMain.vblankCounter1 | 1;
    m4aMPlayVolumeControl(&gMPlayInfo_BGM, TRACKS_ALL, DUCK_VOLUME);
    OrreSound_Duck((float)DUCK_VOLUME / 256);
    if (!ownJingle)
        m4aSongNumStart(MUS_RG_CAUGHT_INTRO);
    CreateTask(Task_CatchJingle, 2);
    MPNet_Log("catch jingle: the battle's song down (song %p)%s", (void *)gMPlayInfo_BGM.songHeader, ownJingle ? ", the ball's own jingle" : "");
}
