// Moemon Kai: the healing machine in SINNOH's POKeMON CENTERs as Platinum plays it (its
// src/overlay006/healing_machine_animation/pokecenter.c): one of its little 3D POKe BALLs set into the machine for
// each POKeMON, 16 frames apart, two rows of three 4.5 units apart, 12 above the machine; then they flash while the
// healing tune plays (Platinum's flash is a material animation the 3D engine doesn't play: here they blink), and go.
// field_effect.c FldEff_PokecenterHeal calls S3D_Heal_Start instead of drawing Emerald's balls and monitor
// (1.63.8: those showed over Platinum's machine). The machine stands beside the nurse in every CENTER (the same
// offset in all but the LEAGUE's north one: tools/sinnoh checked against Platinum's map props).
#include "global.h"
#include "s3d.h"
#include "event_object_movement.h"
#include "field_effect.h"
#include "sound.h"
#include "task.h"
#include "constants/field_effects.h"
#include "constants/songs.h"
#include "constants/sinnoh_gfx.h"
#include "fieldmap.h"

u16 S3D_Gyms_HealBallModel(void);   // (s3d_gyms.c: the pack's number for it, src/data/sinnoh_gyms.h)

#define FIRST_SLOT 30   // the dynamic props' slots it uses (no gym in a CENTER)
#define BALL_TICKS 16   // (Platinum: 15 ticks waited, then the next)
#define BLINK 8

static const s8 sOffsets[PARTY_SIZE][2] = {{-1, -1}, {1, -1}, {-1, 0}, {1, 0}, {-1, 1}, {1, 1}};   // (x, z) in 4.5s

#define tCount  data[0]
#define tPlaced data[1]
#define tTimer  data[2]
#define tState  data[3]

static float sX, sZ;

static void Show(u8 n, bool8 on)
{
    u8 i;
    for (i = 0; i < n; i++)
    {
        if (on)
            S3D_DynProp(FIRST_SLOT + i, S3D_Gyms_HealBallModel(),
                        sX + sOffsets[i][0] * 4.5f, 12.0f, sZ + sOffsets[i][1] * 4.5f, 0);
        else
            S3D_DynPropHide(FIRST_SLOT + i);
    }
}

static void Task_Heal(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    switch (tState)
    {
    case 0:   // a ball a time
        if (tTimer-- > 0)
            break;
        if (tPlaced < tCount)
        {
            tPlaced++;
            Show(tPlaced, TRUE);
            PlaySE(SE_BALL);
            tTimer = BALL_TICKS;
            break;
        }
        PlayFanfare(MUS_HEAL);
        tTimer = 0;
        tState++;
        break;
    case 1:   // they flash with the tune
        tTimer++;
        Show(tCount, (tTimer / BLINK) % 2 == 0);
        if (tTimer > 20 && IsFanfareTaskInactive())
        {
            Show(tCount, FALSE);
            FieldEffectActiveListRemove(FLDEFF_POKECENTER_HEAL);
            DestroyTask(taskId);
        }
        break;
    }
}

bool8 S3D_Heal_Start(u8 count)
{
    u32 i;
    float ox, oz;
    const struct ObjectEvent *nurse = NULL;
    if (!S3D_Active() || !S3D_MapOrigin(&ox, &oz))
        return FALSE;
    for (i = 0; i < OBJECT_EVENTS_COUNT; i++)
        if (gObjectEvents[i].active && OBJ_GFX_ID(&gObjectEvents[i]) == OBJ_EVENT_GFX_SINNOH_POKECENTER_NURSE)
            nurse = &gObjectEvents[i];
    if (nurse == NULL)
        return FALSE;
    {
        s32 tx = nurse->currentCoords.x - MAP_OFFSET, tz = nurse->currentCoords.y - MAP_OFFSET;
        // the machine against the middle of the nurse's tile (Platinum's map props: (-22.2, -14.5); the LEAGUE's
        // north CENTER, its nurse at (4, 1): (-24, -12))
        bool8 north = tx == 4 && tz == 1;
        sX = ox + tx * 16 + 8 + (north ? -24.0f : -22.2f);
        sZ = oz + tz * 16 + 8 + (north ? -12.0f : -14.5f);
    }
    {
        u8 taskId = CreateTask(Task_Heal, 0xFF);
        gTasks[taskId].tCount = count > PARTY_SIZE ? PARTY_SIZE : count;
        gTasks[taskId].tTimer = 0;
    }
    return TRUE;
}

// `mp healballs`: the balls set right now (tests)
u8 S3D_Heal_Shown(void)
{
    u8 taskId = FindTaskIdByFunc(Task_Heal);
    return taskId == TASK_NONE ? 0 : gTasks[taskId].tPlaced;
}
