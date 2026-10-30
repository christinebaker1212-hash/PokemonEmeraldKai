// Moemon Co-op: WATCH mode in the MOéMON GARDEN (docs/POKEMON_GARDEN.md 11.4).
//
// From the GARDEN BOARD: the Trainer sits down and the view drifts to whatever interesting thing is happening
// (a game starting, a squabble, a pile of nappers, someone using a decoration). No text unless asked: A tells
// what's going on there, B stands up. What you see fills in journals (a fact about the one watched).
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "mp.h"
#include "mp_garden.h"
#include "mp_life.h"
#include "mp_life_field.h"
#include "data.h"
#include "event_object_lock.h"
#include "event_object_movement.h"
#include "field_message_box.h"
#include "field_player_avatar.h"
#include "field_specials.h"
#include "main.h"
#include "pokemon.h"
#include "random.h"
#include "script.h"
#include "sound.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "constants/event_objects.h"
#include "constants/songs.h"

void SpawnCameraObject(void);
void RemoveCameraObject(void);
u8 MP_GardenLife_Count(void);
bool8 MP_GardenLife_Info(u8 i, s16 *x, s16 *y, u8 *act, u8 *phase, s8 *partner, u16 *dex, s8 *owner, u32 *key);
u8 MP_GardenLife_Detail(u8 i);
u8 MP_GardenLife_Doing(u8 i);

// (the Garden's activity ids, mp_garden_life.c)

static struct
{
    bool8 active;
    bool8 leaving;
    u8 camObj;
    s8 focus;           // actor watched
    u16 timer;          // frames on this one
    u16 leaveTimer;
    bool8 talking;      // the A description is up
    u8 msgState;
} sW = {.focus = -1};

bool8 MP_GardenWatch_Active(void)
{
    return sW.active;
}

// Where the view is while watching (the camera): what's on screen around it is what the Trainer sees
bool8 MP_GardenWatch_View(s16 *x, s16 *y)
{
    if (!sW.active || sW.camObj >= OBJECT_EVENTS_COUNT || !gObjectEvents[sW.camObj].active)
        return FALSE;
    *x = gObjectEvents[sW.camObj].currentCoords.x;
    *y = gObjectEvents[sW.camObj].currentCoords.y;
    return TRUE;
}

static int Interest(u8 i)
{
    s16 x, y;
    u8 act, phase, j, nappers = 0;
    s8 partner, owner;
    u16 dex;
    u32 key;
    if (!MP_GardenLife_Info(i, &x, &y, &act, &phase, &partner, &dex, &owner, &key))
        return -1;
    switch (act)
    {
    case LIFE_ACT_PLAY:
    case GA_CHASE:
    case GA_SQUABBLE:
        return 50 + (phase == 1 ? 20 : 0);
    case GA_DECOR:
        return 40;
    case GA_RARE:
        return 95;   // something special: always worth watching
    case GA_SONG:
        return 55;
    case GA_REACT:
        return 45;
    case GA_FARM:
        return 42;
    case GA_PURSUIT:
        return 35;
    case LIFE_ACT_NAP:
        // a pile of nappers is worth a look
        for (j = 0; j < MP_GardenLife_Count(); j++)
        {
            s16 x2, y2;
            u8 a2, p2;
            s8 pa2, o2;
            u16 d2;
            u32 k2;
            if (j != i && MP_GardenLife_Info(j, &x2, &y2, &a2, &p2, &pa2, &d2, &o2, &k2) && a2 == LIFE_ACT_NAP && abs(x2 - x) + abs(y2 - y) <= 2)
                nappers++;
        }
        return 20 + nappers * 25;
    case LIFE_ACT_TYPE:
        return 25;
    case LIFE_ACT_FORAGE:
    case LIFE_ACT_COMPANY:
        return 15;
    case LIFE_ACT_QUIET:
        return 12;
    }
    return 3;
}

static void PickFocus(void)
{
    u8 i;
    int best = -1;
    s8 pick = -1;
    for (i = 0; i < MP_GardenLife_Count(); i++)
    {
        int v = Interest(i);
        if (v < 0)
            continue;
        v += Random() % 12;
        if ((s8)i == sW.focus)
            v -= 25;   // something else for a change
        if (v > best)
            best = v, pick = i;
    }
    if (pick != sW.focus && pick >= 0)
    {
        s16 x, y;
        u8 act, phase;
        s8 partner, owner;
        u16 dex;
        u32 key;
        sW.focus = pick;
        sW.timer = 0;
        if (MP_GardenLife_Info(pick, &x, &y, &act, &phase, &partner, &dex, &owner, &key) && owner < 0)
        {
            // what you see fills in journals (now and then: MP_Life_Observe); what it's doing right there, at once
            struct MpLifeRec *rec = MP_Life_FindKey(key);
            if (rec)
            {
                struct BoxPokemon *box = MP_Life_PartyBox(key);
                if (box)
                    MP_Life_Observe(box);
                if ((rec->favDecor >> 8) >= 5 && (act == GA_DECOR || act == LIFE_ACT_NAP))
                    MP_Life_Reveal(rec, LIFE_FACT_DECOR);
                if (rec->pursuit && act == GA_PURSUIT)
                    MP_Life_Reveal(rec, LIFE_FACT_PURSUIT);
            }
        }
        MPNet_Log("garden watch: focus %d (act %u)", pick, act);
    }
}

static bool8 sWatchPending;   // asked for on the board: starts once its script is over

void MP_Garden_StartWatch(void)
{
    sWatchPending = TRUE;
}

static void Start(void)
{
    if (sW.active || !MP_Garden_IsHere())
        return;
    memset(&sW, 0, sizeof(sW));
    sW.focus = -1;
    sW.active = TRUE;
    LockPlayerFieldControls();
    SpawnCameraObject();
    sW.camObj = GetObjectEventIdByLocalIdAndMap(LOCALID_CAMERA, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup);
    PlaySE(SE_SELECT);
    MPNet_Log("garden watch: start");
}

static void Describe(void);

static void StepCameraToward(s16 tx, s16 ty)
{
    struct ObjectEvent *cam;
    s16 dx, dy;
    u8 dir;
    if (sW.camObj >= OBJECT_EVENTS_COUNT || !gObjectEvents[sW.camObj].active)
        return;
    cam = &gObjectEvents[sW.camObj];
    if (ObjectEventIsHeldMovementActive(cam) && !ObjectEventClearHeldMovementIfFinished(cam))
        return;
    dx = tx - cam->currentCoords.x;
    dy = ty - cam->currentCoords.y;
    if (abs(dx) + abs(dy) <= 1)
        return;
    if (abs(dx) >= abs(dy))
        dir = dx > 0 ? DIR_EAST : DIR_WEST;
    else
        dir = dy > 0 ? DIR_SOUTH : DIR_NORTH;
    ObjectEventSetHeldMovement(cam, GetWalkSlowMovementAction(dir));
}

static void End(void)
{
    if (sW.camObj < OBJECT_EVENTS_COUNT && gObjectEvents[sW.camObj].active)
        RemoveCameraObject();
    sW.active = FALSE;
    sW.focus = -1;
    UnlockPlayerFieldControls();
    MPNet_Log("garden watch: end");
}

void MP_GardenWatch_Update(void)
{
    struct ObjectEvent *me = &gObjectEvents[gPlayerAvatar.objectEventId];
    if (sWatchPending && MP_IsLocalFree())
    {
        sWatchPending = FALSE;
        Start();
    }
    if (!sW.active)
        return;
    if (!MP_Garden_IsHere())
    {
        sW.active = FALSE;
        return;
    }
    if (sW.talking)
    {
        switch (sW.msgState)
        {
        case 0:
            if (!IsTextPrinterActive(0) && JOY_NEW(A_BUTTON | B_BUTTON))
            {
                HideFieldMessageBox();
                sW.msgState = 1;
            }
            break;
        case 1:
            if (IsFieldMessageBoxHidden())
                sW.talking = FALSE;
            break;
        }
        return;
    }
    if (sW.leaving)
    {
        // the view comes back to the Trainer, then they stand up
        StepCameraToward(me->currentCoords.x, me->currentCoords.y);
        if (++sW.leaveTimer > 600)
            End();
        else if (sW.camObj < OBJECT_EVENTS_COUNT)
        {
            struct ObjectEvent *cam = &gObjectEvents[sW.camObj];
            bool8 still = !ObjectEventIsHeldMovementActive(cam) || ObjectEventClearHeldMovementIfFinished(cam);
            if (still && abs(cam->currentCoords.x - me->currentCoords.x) + abs(cam->currentCoords.y - me->currentCoords.y) <= 1)
                End();
        }
        return;
    }
    if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        sW.leaving = TRUE;
        return;
    }
    if (JOY_NEW(A_BUTTON) && sW.focus >= 0)
    {
        Describe();
        return;
    }
    if (sW.focus < 0 || ++sW.timer > 60 * 8 || Interest(sW.focus) < 10)
        PickFocus();
    if (sW.focus >= 0)
    {
        s16 x, y;
        u8 act, phase;
        s8 partner, owner;
        u16 dex;
        u32 key;
        if (MP_GardenLife_Info(sW.focus, &x, &y, &act, &phase, &partner, &dex, &owner, &key))
            StepCameraToward(x, y);
    }
}

// ---------------------------------------------------------------------------
// A: what's going on there (in plain words, no numbers)

static const u8 sTxt_Play[] = _("{STR_VAR_1} and {STR_VAR_2} are playing together.");
static const u8 sTxt_Tag[] = _("{STR_VAR_1} and {STR_VAR_2} are playing tag!");
static const u8 sTxt_Squabble[] = _("{STR_VAR_1} and {STR_VAR_2} are squaring up to each other.");
static const u8 sTxt_Decor[] = _("{STR_VAR_1} is playing with a decoration.");
static const u8 sTxt_Nap[] = _("{STR_VAR_1} is napping peacefully.");
static const u8 sTxt_NapPile[] = _("{STR_VAR_1} is napping in a pile of friends.");
static const u8 sTxt_Forage[] = _("{STR_VAR_1} is rummaging for something to eat.");
static const u8 sTxt_Type[] = _("{STR_VAR_1} is doing what it loves best.");
static const u8 sTxt_Company[] = _("{STR_VAR_1} is keeping you company.");
static const u8 sTxt_Quiet[] = _("{STR_VAR_1} is enjoying some time alone.");
static const u8 sTxt_Wander[] = _("{STR_VAR_1} is strolling around.");
static const u8 sTxt_RareAny[] = _("{STR_VAR_1} and {STR_VAR_2} are sharing a special moment.");
static const u8 sTxt_RareStars[] = _("{STR_VAR_1} and {STR_VAR_2} are gazing up at the stars together.");
static const u8 sTxt_RarePaddle[] = _("{STR_VAR_1} is coaxing {STR_VAR_2} into the water!");
static const u8 sTxt_RarePaddled[] = _("{STR_VAR_2} is coaxing {STR_VAR_1} into the water!");
static const u8 sTxt_RareNap[] = _("Old rivals {STR_VAR_1} and {STR_VAR_2} are napping side by side.");
static const u8 sTxt_RareFamily[] = _("{STR_VAR_1} and {STR_VAR_2} are grooming each other.");
static const u8 sTxt_Song[] = _("{STR_VAR_1} and {STR_VAR_2} are singing their family's song.");
static const u8 sTxt_React[] = _("{STR_VAR_1} is taking a good look at {STR_VAR_2}'s new form.");
static const u8 sTxt_PursuitAny[] = _("{STR_VAR_1} is busy with something it loves.");
static const u8 *const sTxt_Pursuit[LIFE_PURSUIT_COUNT] = {
    [LIFE_PURSUIT_NONE]    = sTxt_PursuitAny,
    [LIFE_PURSUIT_CLIMB]   = (const u8[])_("{STR_VAR_1} is practicing its climbing."),
    [LIFE_PURSUIT_SWIM]    = (const u8[])_("{STR_VAR_1} is swimming laps in the pond."),
    [LIFE_PURSUIT_SING]    = (const u8[])_("{STR_VAR_1} is singing its heart out."),
    [LIFE_PURSUIT_COLLECT] = (const u8[])_("{STR_VAR_1} is looking for pretty stones."),
    [LIFE_PURSUIT_DIG]     = (const u8[])_("{STR_VAR_1} is digging a tunnel."),
    [LIFE_PURSUIT_TEACH]   = (const u8[])_("{STR_VAR_1} is showing {STR_VAR_2} how it's done."),
};
// what it's visibly doing, when it's that (MP_GardenLife_Doing)
static const u8 *const sTxt_Doing[] = {
    [1] = (const u8[])_("{STR_VAR_1} is swimming in the pond."),
    [2] = (const u8[])_("{STR_VAR_1} is up in a tree!"),
    [3] = (const u8[])_("{STR_VAR_1} is digging in the dirt."),
};
static const u8 sTxt_FarmAny[] = _("{STR_VAR_1} is looking after the BERRY plants.");
static const u8 *const sTxt_Farm[] = {
    (const u8[])_("{STR_VAR_1} is watering the BERRY plants."),
    (const u8[])_("{STR_VAR_1} is helping a BERRY plant grow."),
    (const u8[])_("{STR_VAR_1} is sneaking a bite of a BERRY!"),
};
static const u8 sTxt_Taught[] = _("{STR_VAR_2} is showing {STR_VAR_1} how it's done.");

static void NameOfActor(s8 i, u8 *out)
{
    s16 x, y;
    u8 act, phase;
    s8 partner, owner;
    u16 dex;
    u32 key;
    out[0] = EOS;
    if (i < 0 || !MP_GardenLife_Info(i, &x, &y, &act, &phase, &partner, &dex, &owner, &key))
        return;
    if (!MP_Life_NameOf(key, out))
        StringCopy(out, gSpeciesNames[NationalPokedexNumToSpecies(dex)]);
}

static void Describe(void)
{
    s16 x, y;
    u8 act, phase;
    s8 partner, owner;
    u16 dex;
    u32 key;
    const u8 *t = sTxt_Wander;
    if (!MP_GardenLife_Info(sW.focus, &x, &y, &act, &phase, &partner, &dex, &owner, &key))
        return;
    NameOfActor(sW.focus, gStringVar1);
    NameOfActor(partner, gStringVar2);
    switch (act)
    {
    case LIFE_ACT_PLAY: t = partner >= 0 ? sTxt_Play : sTxt_Decor; break;
    case GA_CHASE: t = sTxt_Tag; break;
    case GA_SQUABBLE: t = sTxt_Squabble; break;
    case GA_DECOR: t = sTxt_Decor; break;
    case LIFE_ACT_NAP: t = Interest(sW.focus) >= 45 ? sTxt_NapPile : sTxt_Nap; break;
    case LIFE_ACT_FORAGE: t = sTxt_Forage; break;
    case LIFE_ACT_TYPE: t = sTxt_Type; break;
    case LIFE_ACT_COMPANY: t = sTxt_Company; break;
    case LIFE_ACT_QUIET: t = sTxt_Quiet; break;
    case GA_SONG: t = sTxt_Song; break;
    case GA_REACT: t = sTxt_React; break;
    case GA_FARM:
    {
        u8 d = MP_GardenLife_Detail(sW.focus);
        t = (d & 0x80) && (d & 0x7F) < ARRAY_COUNT(sTxt_Farm) ? sTxt_Farm[d & 0x7F] : sTxt_FarmAny;
        break;
    }
    case GA_RARE:
    {
        u8 d = MP_GardenLife_Detail(sW.focus);
        switch (d & 0x7F)
        {
        case LIFE_RARE_STARGAZE:  t = sTxt_RareStars; break;
        case LIFE_RARE_PADDLE:    t = (d & 0x80) ? sTxt_RarePaddled : sTxt_RarePaddle; break;
        case LIFE_RARE_RIVAL_NAP: t = sTxt_RareNap; break;
        case LIFE_RARE_FAMILY:    t = sTxt_RareFamily; break;
        default:                  t = sTxt_RareAny; break;
        }
        break;
    }
    case GA_PURSUIT:
    {
        u8 d = MP_GardenLife_Detail(sW.focus);
        t = d == 0x80 ? sTxt_Taught : d < LIFE_PURSUIT_COUNT ? sTxt_Pursuit[d] : sTxt_PursuitAny;
        break;
    }
    }
    // swimming, up a tree, digging: said as it is (a pursuit says what it's practicing)
    if (act != GA_PURSUIT && act != GA_RARE)
    {
        u8 doing = MP_GardenLife_Doing(sW.focus);
        if (doing != 0 && doing < ARRAY_COUNT(sTxt_Doing))
            t = sTxt_Doing[doing];
    }
    StringExpandPlaceholders(gStringVar4, t);
    MP_ShowFieldMessageFitted(gStringVar4);
    sW.talking = TRUE;
    sW.msgState = 0;
    MPNet_Log("garden watch: told (act %u)", act);
}

void MP_GardenWatch_Debug(void)
{
    printf("[test] garden watch active %d focus %d leaving %d\n", sW.active, sW.focus, sW.leaving);
}
