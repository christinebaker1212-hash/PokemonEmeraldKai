// Moemon Co-op: MOéMON GARDEN, talking to and doing things with Pokémon (docs/POKEMON_GARDEN.md 6).
//
// A in front of the follower does the fitting thing (comforts a frightened one, joins the game it's
// inviting you to, greets a calm one); holding A opens a wheel of verbs (mp_menu.c MP_OpenVerbWheel).
// How it lands depends on the moment (its needs and feelings), its mood and its bond with you. Repeats
// become ordinary, never negative. Each Pokémon has boundaries from its temperament: crossing one gets
// a sidestep (and the journal notes it); insisting brings a brief huff that fades, the bond is never
// marked. Only a good memory involving you raises Friendship (mp_life.c MP_Life_Remember).
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "mp.h"
#include "mp_life.h"
#include "mp_life_emote.h"
#include "mp_life_field.h"
#include "mp_garden.h"
#include "berry.h"
#include "event_object_lock.h"
#include "event_object_movement.h"
#include "field_message_box.h"
#include "field_player_avatar.h"
#include "item.h"
#include "main.h"
#include "map_name_popup.h"
#include "pokemon.h"
#include "random.h"
#include "script.h"
#include "sound.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "constants/berry.h"
#include "constants/items.h"
#include "constants/songs.h"
#include "constants/species.h"

void MP_OpenVerbWheel(void (*done)(s8 verb));   // mp_menu.c: -1 = nothing picked

#define HOLD_FOR_WHEEL 20   // frames A is held before the wheel opens

static const u8 sText_Talk_Happy[] = _("{STR_VAR_1} looks up at you happily.");
static const u8 sText_Talk_Shy[] = _("{STR_VAR_1} peeks at you shyly.");
static const u8 sText_Talk_Aloof[] = _("{STR_VAR_1} gives you a cool glance.");
static const u8 sText_Talk_Scared[] = _("{STR_VAR_1} is trembling a little.");
static const u8 sText_Talk_Asleep[] = _("{STR_VAR_1} is fast asleep.");
static const u8 sText_Talk_Hungry[] = _("{STR_VAR_1}'s tummy rumbles.");
static const u8 sText_Talk_Default[] = _("{STR_VAR_1} is happy to be with you!");
static const u8 sText_Talk_Inviting[] = _("{STR_VAR_1} wants to play!");
static const u8 sText_Pet_Warm[] = _("{STR_VAR_1} leans into your hand.");
static const u8 sText_Pet_Aloof[] = _("{STR_VAR_1} allows it, just this once.");
static const u8 sText_Pet_Boundary[] = _("{STR_VAR_1} slips out from under your hand… but stays close to you.");
static const u8 sText_Feed_None[] = _("You don't have any BERRIES to share.");
static const u8 sText_Feed_Hungry[] = _("You share your {STR_VAR_2}. {STR_VAR_1} gobbles it up!");
static const u8 sText_Feed_Favorite[] = _("You share your {STR_VAR_2}. {STR_VAR_1} gobbles it up! It's a favorite!");
static const u8 sText_Feed_Disliked[] = _("You share your {STR_VAR_2}. {STR_VAR_1} makes a face… but eats it anyway.");
static const u8 sText_Feed_Full[] = _("{STR_VAR_1} is full and politely turns the {STR_VAR_2} down.");
static const u8 sText_Feed_Snack[] = _("{STR_VAR_1} nibbles the {STR_VAR_2} as a snack.");
static const u8 sText_Play_Great[] = _("You play with {STR_VAR_1}! It's having a great time.");
static const u8 sText_Play_Sleepy[] = _("{STR_VAR_1} is too sleepy to play right now.");
static const u8 sText_Play_Default[] = _("You play with {STR_VAR_1} for a while.");
static const u8 sText_Praise_Glow[] = _("You praise {STR_VAR_1}. It beams with pride!");
static const u8 sText_Praise_Proud[] = _("{STR_VAR_1} acts like it's no big deal… but it's pleased.");
static const u8 sText_Praise_Default[] = _("{STR_VAR_1} looks pleased.");
static const u8 sText_Comfort_Calms[] = _("You comfort {STR_VAR_1}. It calms down and stays close to you.");
static const u8 sText_Comfort_NotNeeded[] = _("{STR_VAR_1} doesn't need comforting, but it doesn't mind.");
static const u8 sText_Groom_Boundary[] = _("{STR_VAR_1} wriggles away. It prefers to tidy itself.");
static const u8 sText_Groom_Neat[] = _("You groom {STR_VAR_1}. It looks very neat!");
static const u8 sText_Sit_Shy[] = _("You sit quietly with {STR_VAR_1}. It seems to like this.");
static const u8 sText_Sit_Default[] = _("You sit with {STR_VAR_1} for a while.");
static const u8 sText_Watch_Learn[] = _("You watch {STR_VAR_1} quietly… You learned something about it. (JOURNAL)");
static const u8 sText_Watch_Nothing[] = _("You watch {STR_VAR_1}. It goes about its business.");
static const u8 sText_Keepsake_Give[] = _("{STR_VAR_1} treasures the {STR_VAR_2} you gave it!");
static const u8 sText_Keepsake_Still[] = _("{STR_VAR_1} still treasures its {STR_VAR_2}.");
static const u8 sText_Keepsake_Nothing[] = _("Give {STR_VAR_1} something to hold first, then offer it as a keepsake.");
static const u8 sText_Content[] = _("{STR_VAR_1} seems content.");
static const u8 sText_Huff[] = _("{STR_VAR_1} huffs and turns away for a moment.");

struct InteractState
{
    u32 personality;
    u32 lastVerbFrame[LIFE_VERBS];
    u32 boundaryFrames[3];      // the last three boundary crossings (insisting)
    u32 lastOrdinaryBond;       // play minute of the last small bond step from an ordinary interaction
    u32 frame;
};
static struct InteractState sI;

// Who this is with: the follower, or a Pokémon in the MOéMON GARDEN (mp_garden_life.c)
static struct
{
    bool8 active;
    struct Pokemon *mon;
    s8 actor;           // the Garden's actor index (-1: the follower)
} sTarget = {.actor = -1};

static struct Pokemon *TheFollower(void);

static struct Pokemon *FollowerMon(void)
{
    if (sTarget.active)
        return sTarget.mon;
    return TheFollower();
}

static void React(u8 emote, u8 pose, u16 frames)
{
    if (sTarget.active && sTarget.actor >= 0)
        MP_Garden_ActorReaction(sTarget.actor, emote, pose, frames);
    else
        MP_LifeField_Reaction(emote, pose, frames);
}

static u8 TargetActivity(void)
{
    return sTarget.active && sTarget.actor >= 0 ? MP_Garden_ActorActivity(sTarget.actor) : MP_LifeField_Activity();
}

static bool8 TargetInviting(void)
{
    return sTarget.active && sTarget.actor >= 0 ? MP_Garden_ActorActivity(sTarget.actor) == LIFE_ACT_PLAY : MP_LifeField_Inviting();
}

static void EndTarget(void)
{
    sTarget.active = FALSE;
    sTarget.actor = -1;
}

static struct Pokemon *TheFollower(void)
{
    u8 i;
    u32 p = gMpConfig.followerPersonality;
    for (i = 0; i < PARTY_SIZE && p; i++)
        if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES) != SPECIES_NONE && !GetMonData(&gPlayerParty[i], MON_DATA_IS_EGG)
         && GetMonData(&gPlayerParty[i], MON_DATA_PERSONALITY) == p)
            return &gPlayerParty[i];
    return NULL;
}

// Someone else around (an NPC, a friend): for proud ones who'd rather not be fussed over in front of others
static bool8 OthersAround(void)
{
    struct ObjectEvent *me = &gObjectEvents[gPlayerAvatar.objectEventId];
    u8 i;
    for (i = 0; i < OBJECT_EVENTS_COUNT; i++)
    {
        struct ObjectEvent *o = &gObjectEvents[i];
        if (!o->active || i == gPlayerAvatar.objectEventId)
            continue;
        if (abs(o->currentCoords.x - me->currentCoords.x) <= 4 && abs(o->currentCoords.y - me->currentCoords.y) <= 3)
            return TRUE;
    }
    return FALSE;
}

static bool8 IsRepeat(u8 verb)
{
    bool8 repeat = sI.lastVerbFrame[verb] && sI.frame - sI.lastVerbFrame[verb] < 60 * 60;
    sI.lastVerbFrame[verb] = sI.frame ? sI.frame : 1;
    return repeat;
}

static void OwnerBond(struct MpLifeRec *rec, s8 trust, s8 affection, bool8 meaningful)
{
    u32 me = MP_Life_MonKey(rec->personality, rec->otId);
    u32 now = MP_Life_PlayMinutes();
    if (meaningful)
    {
        MP_Life_BondShare(me, MP_Life_LocalTrainerKey(), 1, trust, affection, 0, 0);
    }
    else if (now - sI.lastOrdinaryBond >= 10 || sI.lastOrdinaryBond == 0)
    {
        // ordinary moments only add a little familiarity, at most every ten minutes
        sI.lastOrdinaryBond = now ? now : 1;
        MP_Life_BondShare(me, MP_Life_LocalTrainerKey(), 1, 0, 0, 0, 0);
    }
}

// A boundary crossed: a sidestep; insisting three times in a short while brings a brief huff
static const u8 *CrossBoundary(struct MpLifeRec *rec, const u8 *sidestep, u8 verb)
{
    u8 i, recent = 0;
    MP_Life_Reveal(rec, LIFE_FACT_BOUNDARY);
    sI.boundaryFrames[0] = sI.boundaryFrames[1];
    sI.boundaryFrames[1] = sI.boundaryFrames[2];
    sI.boundaryFrames[2] = sI.frame ? sI.frame : 1;
    for (i = 0; i < 3; i++)
        if (sI.boundaryFrames[i] && sI.frame - sI.boundaryFrames[i] < 60 * 15)
            recent++;
    MPNet_Log("life interact: boundary %u (%u in a row)", verb, recent);
    if (recent >= 3)
    {
        MP_Life_Feel(rec, EMO_FRUSTRATION, 30);   // fades within minutes; the bond is never marked
        React(LIFE_EMOTE_ANGRY, LIFE_POSE_HURT, 90);
        return sText_Huff;
    }
    React(rec->cur[LIFE_WARM] > 110 ? LIFE_EMOTE_HEART : LIFE_EMOTE_DOTS, LIFE_POSE_NONE, 0);
    return sidestep;
}

// The berry to share: the one it'd like most among the common ones in the BAG
static u16 PickBerry(struct MpLifeRec *rec, s8 *taste)
{
    u16 item, best = ITEM_NONE;
    s32 bestScore = -10000;
    for (item = FIRST_BERRY_INDEX; item < FIRST_BERRY_INDEX + 20; item++)
    {
        const struct Berry *b;
        s32 score;
        u8 type = ItemIdToBerryType(item);
        if (!CheckBagHasItem(item, 1) || type == 0)
            continue;
        b = GetBerryInfo(type);
        score = b->spicy * rec->taste[FLAVOR_SPICY] + b->dry * rec->taste[FLAVOR_DRY] + b->sweet * rec->taste[FLAVOR_SWEET]
              + b->bitter * rec->taste[FLAVOR_BITTER] + b->sour * rec->taste[FLAVOR_SOUR];
        if (score > bestScore)
            bestScore = score, best = item;
    }
    *taste = bestScore > 300 ? 1 : bestScore < -300 ? -1 : 0;
    return best;
}

static u8 StrongestFlavor(u16 item)
{
    const struct Berry *b = GetBerryInfo(ItemIdToBerryType(item));
    u8 v[FLAVOR_COUNT], i, best = 0;
    v[FLAVOR_SPICY] = b->spicy; v[FLAVOR_DRY] = b->dry; v[FLAVOR_SWEET] = b->sweet; v[FLAVOR_BITTER] = b->bitter; v[FLAVOR_SOUR] = b->sour;
    for (i = 1; i < FLAVOR_COUNT; i++)
        if (v[i] > v[best])
            best = i;
    return best;
}

static u8 NegativeFeeling(struct MpLifeRec *rec)
{
    u8 m = rec->emo[EMO_FEAR];
    if (rec->emo[EMO_SADNESS] > m) m = rec->emo[EMO_SADNESS];
    if (rec->emo[EMO_FRUSTRATION] > m) m = rec->emo[EMO_FRUSTRATION];
    return m;
}

// One verb: what happens, and the line that says so (game text)
static const u8 *DoVerb(struct Pokemon *mon, struct MpLifeRec *rec, u8 verb)
{
    u32 owner = MP_Life_LocalTrainerKey();
    u8 act = TargetActivity();
    bool8 repeat = IsRepeat(verb);
    struct MpLifeBond *bond = MP_Life_Bond(MP_Life_MonKey(rec->personality, rec->otId), owner, FALSE);
    u8 familiar = bond ? bond->v[BOND_FAMILIAR] : 0;

    switch (verb)
    {
    case LIFE_VERB_TALK:
        OwnerBond(rec, 0, 0, FALSE);
        if (act == LIFE_ACT_NAP)
            return sText_Talk_Asleep;
        if (TargetInviting())
        {
            React(LIFE_EMOTE_BALL, LIFE_POSE_HOP, 60);
            return sText_Talk_Inviting;
        }
        if (rec->emo[EMO_FEAR] >= 40)
        {
            React(LIFE_EMOTE_SWEAT, LIFE_POSE_NONE, 0);
            return sText_Talk_Scared;
        }
        if (rec->need[NEED_HUNGER] >= 210)
        {
            React(LIFE_EMOTE_FOOD, LIFE_POSE_NONE, 0);
            return sText_Talk_Hungry;
        }
        if (rec->cur[LIFE_BOLD] < 90 && familiar < 120)
        {
            React(LIFE_EMOTE_DOTS, LIFE_POSE_NONE, 0);
            return sText_Talk_Shy;
        }
        if (rec->cur[LIFE_WARM] < 80)
        {
            React(LIFE_EMOTE_NONE, LIFE_POSE_SIT, 40);
            return sText_Talk_Aloof;
        }
        React(rec->mood > 30 ? LIFE_EMOTE_NOTE : LIFE_EMOTE_CALM, LIFE_POSE_NOD, 60);   // nods along
        return rec->mood > 30 ? sText_Talk_Happy : sText_Talk_Default;

    case LIFE_VERB_PET:
        if ((rec->boundaries & (1 << LIFE_VERB_PET)) && OthersAround())
            return CrossBoundary(rec, sText_Pet_Boundary, verb);
        if (repeat)
        {
            OwnerBond(rec, 0, 0, FALSE);
            React(LIFE_EMOTE_CALM, LIFE_POSE_NONE, 0);
            return sText_Content;
        }
        MP_Life_Feel(rec, rec->cur[LIFE_WARM] >= 100 ? EMO_AFFECTION : EMO_CALM, 15);
        OwnerBond(rec, 1, rec->cur[LIFE_WARM] >= 100 ? 1 : 0, FALSE);
        if (rec->cur[LIFE_WARM] < 80)
        {
            React(LIFE_EMOTE_DOTS, LIFE_POSE_SIT, 40);
            return sText_Pet_Aloof;
        }
        React(LIFE_EMOTE_HEART, LIFE_POSE_JOY, 45);
        return sText_Pet_Warm;

    case LIFE_VERB_FEED:
    {
        s8 taste;
        u16 berry = PickBerry(rec, &taste);
        if (berry == ITEM_NONE)
            return sText_Feed_None;
        CopyItemName(berry, gStringVar2);
        // just ate: politely declines (the berry stays in the BAG)
        if (rec->need[NEED_HUNGER] < 60)
        {
            React(LIFE_EMOTE_DOTS, LIFE_POSE_NONE, 0);
            return sText_Feed_Full;
        }
        RemoveBagItem(berry, 1);
        // whether it likes the flavor: the Trainer learns its tastes this way
        MP_Life_Reveal(rec, taste > 0 ? LIFE_FACT_LIKES : taste < 0 ? LIFE_FACT_DISLIKES : 0xFF);
        if (rec->need[NEED_HUNGER] >= 180 || act == LIFE_ACT_FORAGE)
        {
            // it was actually looking for food: this matters (a memory; a moment with its Trainer)
            u8 strength = 100 + (rec->need[NEED_HUNGER] - 150) / 2 + (taste > 0 ? 30 : 0);
            MP_Life_Satisfy(rec, NEED_HUNGER, 200);
            MP_Life_Feel(rec, EMO_JOY, taste < 0 ? 10 : 30);
            if (taste >= 0)
                MP_Life_Remember(rec, MEM_FED, strength, TIER_PART, owner, berry);
            if (taste > 0 && rec->emo[EMO_SADNESS] + rec->emo[EMO_FEAR] >= 40)
                MP_Life_AddTrait(rec, TRAIT_COMFORT_FOOD, StrongestFlavor(berry), 50);   // it cheered up on this
            OwnerBond(rec, 2, 2, TRUE);
            React(taste < 0 ? LIFE_EMOTE_SWEAT : LIFE_EMOTE_NOTE, taste < 0 ? LIFE_POSE_HURT : LIFE_POSE_EAT, 50);
            return taste > 0 ? sText_Feed_Favorite : taste < 0 ? sText_Feed_Disliked : sText_Feed_Hungry;
        }
        MP_Life_Satisfy(rec, NEED_HUNGER, 80);
        MP_Life_Feel(rec, EMO_JOY, taste < 0 ? 0 : 10);
        OwnerBond(rec, 0, 0, FALSE);
        React(taste < 0 ? LIFE_EMOTE_SWEAT : LIFE_EMOTE_FOOD, LIFE_POSE_NONE, 0);
        return taste < 0 ? sText_Feed_Disliked : sText_Feed_Snack;
    }

    case LIFE_VERB_PLAY:
        if (rec->need[NEED_REST] >= 210 || act == LIFE_ACT_NAP)
        {
            React(LIFE_EMOTE_ZZZ, LIFE_POSE_NONE, 0);
            return sText_Play_Sleepy;
        }
        if (TargetInviting() || rec->need[NEED_PLAY] >= 150)
        {
            // it wanted to play: a game together it'll remember
            MP_Life_Remember(rec, MEM_PLAYED, 100 + rec->need[NEED_PLAY] / 4, TIER_PART, owner, 0);
            MP_Life_Satisfy(rec, NEED_PLAY, 220);
            MP_Life_Feel(rec, EMO_JOY, 35);
            OwnerBond(rec, 1, 2, TRUE);
            React(LIFE_EMOTE_NOTE, LIFE_POSE_HOP, 90);
            return sText_Play_Great;
        }
        MP_Life_Satisfy(rec, NEED_PLAY, 60);
        MP_Life_Feel(rec, EMO_JOY, repeat ? 5 : 12);
        OwnerBond(rec, 0, 0, FALSE);
        React(LIFE_EMOTE_NOTE, LIFE_POSE_HOP, 40);
        return repeat ? sText_Content : sText_Play_Default;

    case LIFE_VERB_PRAISE:
        if (rec->emo[EMO_PRIDE] >= 40 && !repeat)
        {
            MP_Life_Remember(rec, MEM_PRAISED, 90 + rec->emo[EMO_PRIDE], TIER_PART, owner, 0);
            MP_Life_Feel(rec, EMO_PRIDE, 20);
            OwnerBond(rec, 1, 2, TRUE);
            React(LIFE_EMOTE_SPARKLE, LIFE_POSE_JOY, 50);
            return rec->cur[LIFE_PROUD] >= 180 ? sText_Praise_Proud : sText_Praise_Glow;
        }
        OwnerBond(rec, 0, 0, FALSE);
        MP_Life_Feel(rec, EMO_PRIDE, repeat ? 0 : 8);
        React(repeat ? LIFE_EMOTE_CALM : LIFE_EMOTE_SPARKLE, LIFE_POSE_NONE, 0);
        return repeat ? sText_Content : (rec->cur[LIFE_PROUD] >= 180 ? sText_Praise_Proud : sText_Praise_Default);

    case LIFE_VERB_COMFORT:
    {
        u8 neg = NegativeFeeling(rec);
        if (neg >= 40)
        {
            // it was upset: being comforted is a memory (and softens a fear right then)
            MP_Life_Remember(rec, MEM_COMFORTED, 90 + neg, TIER_PART, owner, 0);
            MP_Life_Feel(rec, EMO_CALM, 40);
            MP_Life_Feel(rec, EMO_AFFECTION, 20);
            rec->emo[EMO_FEAR] /= 3;
            rec->emo[EMO_SADNESS] /= 3;
            rec->emo[EMO_FRUSTRATION] /= 3;
            MP_Life_SoftenTrait(rec, TRAIT_AFRAID, LIFE_TRIG_THUNDER, 12);
            MP_Life_SoftenTrait(rec, TRAIT_AFRAID, LIFE_TRIG_DARK, 12);
            OwnerBond(rec, 3, 2, TRUE);
            React(LIFE_EMOTE_HEART, LIFE_POSE_SIT, 90);
            return sText_Comfort_Calms;
        }
        MP_Life_Feel(rec, EMO_AFFECTION, repeat ? 0 : 6);
        OwnerBond(rec, 0, 0, FALSE);
        React(LIFE_EMOTE_CALM, LIFE_POSE_NONE, 0);
        return repeat ? sText_Content : sText_Comfort_NotNeeded;
    }

    case LIFE_VERB_GROOM:
        if (rec->boundaries & (1 << LIFE_VERB_GROOM))
            return CrossBoundary(rec, sText_Groom_Boundary, verb);
        if (repeat)
        {
            React(LIFE_EMOTE_CALM, LIFE_POSE_NONE, 0);
            return sText_Content;
        }
        MP_Life_Feel(rec, EMO_CALM, 15);
        MP_Life_Feel(rec, EMO_JOY, 8);
        if (MP_Life_TypeNeed(rec, FALSE) == TNEED_COZY)
            MP_Life_Satisfy(rec, NEED_TYPE_A, 80);
        OwnerBond(rec, 1, 1, FALSE);
        React(LIFE_EMOTE_SPARKLE, LIFE_POSE_NONE, 0);
        return sText_Groom_Neat;

    case LIFE_VERB_WATCH:
        // watching quietly: now and then the Trainer notices something about it (mp_life_journal.c)
        if (MP_Life_Observe(&mon->box))
        {
            MPNet_Log("life interact: watched, learned something");
            React(LIFE_EMOTE_QUESTION, LIFE_POSE_NONE, 0);
            OwnerBond(rec, 0, 0, FALSE);
            return sText_Watch_Learn;
        }
        MPNet_Log("life interact: watched, nothing new");
        return sText_Watch_Nothing;
    }
    return sText_Talk_Default;
}

// SIT WITH and KEEPSAKE (the wheel's other verbs)
static const u8 *DoSitWith(struct MpLifeRec *rec)
{
    bool8 shy = rec->cur[LIFE_BOLD] < 100 || rec->cur[LIFE_SOCIAL] < 110;
    MP_Life_Feel(rec, EMO_CALM, 25);
    MP_Life_Satisfy(rec, rec->cur[LIFE_SOCIAL] >= 128 ? NEED_COMPANY : NEED_QUIET, 120);
    OwnerBond(rec, shy ? 2 : 1, 1, shy);
    MP_Life_Reveal(rec, LIFE_FACT_TEMPER(LIFE_SOCIAL));
    React(LIFE_EMOTE_CALM, LIFE_POSE_SIT, 120);
    return shy ? sText_Sit_Shy : sText_Sit_Default;
}

static const u8 *DoKeepsake(struct Pokemon *mon, struct MpLifeRec *rec)
{
    u16 item = GetMonData(mon, MON_DATA_HELD_ITEM);
    if (item == ITEM_NONE)
        return sText_Keepsake_Nothing;
    CopyItemName(item, gStringVar2);
    if (rec->keepsake == item)
    {
        React(LIFE_EMOTE_HEART, LIFE_POSE_NONE, 0);
        return sText_Keepsake_Still;
    }
    rec->keepsake = item;
    MP_Life_Reveal(rec, LIFE_FACT_KEEPSAKE);
    MP_Life_Remember(rec, MEM_KEEPSAKE, 150, TIER_PART, MP_Life_LocalTrainerKey(), item);
    MP_Life_Feel(rec, EMO_AFFECTION, 30);
    OwnerBond(rec, 1, 3, TRUE);
    React(LIFE_EMOTE_HEART, LIFE_POSE_JOY, 60);
    return sText_Keepsake_Give;
}

// The fitting thing for A (docs 6.1)
static s8 SmartVerb(struct MpLifeRec *rec)
{
    if (TargetActivity() == LIFE_ACT_NAP)
        return LIFE_VERB_WATCH + 64;             // don't wake it: just look (a TALK that says it's asleep)
    if (NegativeFeeling(rec) >= 40)
        return LIFE_VERB_COMFORT;
    if (TargetInviting())
        return LIFE_VERB_PLAY;
    if (rec->emo[EMO_PRIDE] >= 50)
        return LIFE_VERB_PRAISE;
    return LIFE_VERB_TALK;
}

// ---------------------------------------------------------------------------
// The task: tap A (the fitting thing) or hold it (the wheel)

#define tState data[0]
#define tTimer data[1]
#define tVerb  data[2]

static void Task_Interact(u8 taskId);

static void ShowResult(u8 taskId, s8 verb)
{
    struct Pokemon *mon = FollowerMon();
    struct MpLifeRec *rec = mon ? MP_Life_Get(&mon->box, TRUE) : NULL;
    const u8 *text;
    if (mon == NULL || rec == NULL || verb < 0)
    {
        UnlockPlayerFieldControls();
        DestroyTask(taskId);
        EndTarget();
        return;
    }
    GetMonData(mon, MON_DATA_NICKNAME, gStringVar1);
    StringGet_Nickname(gStringVar1);
    if (verb == LIFE_VERB_WATCH + 64)
        text = sText_Talk_Asleep, React(LIFE_EMOTE_ZZZ, LIFE_POSE_NONE, 0);
    else if (verb == LIFE_VERBS)          // SIT WITH
        text = DoSitWith(rec);
    else if (verb == LIFE_VERB_KEEPSAKE)
        text = DoKeepsake(mon, rec);
    else
        text = DoVerb(mon, rec, verb);
    MPNet_Log("life interact: verb %d", verb);
    StringExpandPlaceholders(gStringVar4, text);
    // a cry, softly, for the warm answers
    if (verb == LIFE_VERB_PLAY || verb == LIFE_VERB_PET || verb == LIFE_VERB_COMFORT || verb == LIFE_VERB_FEED)
        PlayCry_NormalNoDucking(GetMonData(mon, MON_DATA_SPECIES), 0, CRY_VOLUME_RS / 2, CRY_PRIORITY_NORMAL);
    HideMapNamePopUpWindow();
    MP_ShowFieldMessageFitted(gStringVar4);
    gTasks[taskId].tState = 3;
}

static u8 sWheelTask = TASK_NONE;
static void WheelDone(s8 verb)
{
    if (sWheelTask != TASK_NONE && gTasks[sWheelTask].isActive && gTasks[sWheelTask].func == Task_Interact)
    {
        gTasks[sWheelTask].tVerb = verb;
        gTasks[sWheelTask].tState = 2;
    }
}

static void Task_Interact(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    switch (tState)
    {
    case 0:   // A held? tap = the fitting thing, hold = the wheel
        if (!JOY_HELD(A_BUTTON))
        {
            struct Pokemon *mon = FollowerMon();
            struct MpLifeRec *rec = mon ? MP_Life_Get(&mon->box, TRUE) : NULL;
            ShowResult(taskId, rec ? SmartVerb(rec) : -1);
        }
        else if (++tTimer >= HOLD_FOR_WHEEL)
        {
            tState = 1;
            sWheelTask = taskId;
            MP_OpenVerbWheel(WheelDone);
        }
        break;
    case 1:   // the wheel is open
        break;
    case 2:   // picked from the wheel
        ShowResult(taskId, (s8)tVerb);
        break;
    case 3:
        if (!IsTextPrinterActive(0) && JOY_NEW(A_BUTTON | B_BUTTON))
        {
            HideFieldMessageBox();
            tState = 4;
        }
        break;
    case 4:
        if (IsFieldMessageBoxHidden())
        {
            UnlockPlayerFieldControls();
            DestroyTask(taskId);
            EndTarget();
        }
        break;
    }
}

// A in front of the follower (mp_overworld.c, instead of the old "happy to be with you")
bool8 MP_LifeInteract_TryFollower(void)
{
    struct ObjectEvent *me;
    s16 x, y, fx, fy;
    u8 dir;
    if (!JOY_NEW(A_BUTTON) || !MP_IsLocalFree() || FuncIsActiveTask(Task_Interact))
        return FALSE;
    if (!MP_Follower_LocalPos(&fx, &fy, &dir) || FollowerMon() == NULL)
        return FALSE;
    me = &gObjectEvents[gPlayerAvatar.objectEventId];
    x = me->currentCoords.x;
    y = me->currentCoords.y;
    MoveCoords(me->facingDirection, &x, &y);
    if (x != fx || y != fy || GetObjectEventIdByXY(x, y) != OBJECT_EVENTS_COUNT)
        return FALSE;
    LockPlayerFieldControls();
    CreateTask(Task_Interact, 80);
    return TRUE;
}

// A in front of a Garden Pokémon (mp_garden_life.c): the same tap / hold, with that Pokémon
bool8 MP_LifeInteract_StartFor(struct Pokemon *mon, s8 actor)
{
    if (FuncIsActiveTask(Task_Interact) || mon == NULL)
        return FALSE;
    sTarget.active = TRUE;
    sTarget.mon = mon;
    sTarget.actor = actor;
    LockPlayerFieldControls();
    CreateTask(Task_Interact, 80);
    return TRUE;
}

void MP_LifeInteract_Update(void)
{
    struct Pokemon *mon = FollowerMon();
    sI.frame++;
    if (mon == NULL)
        return;
    if (GetMonData(mon, MON_DATA_PERSONALITY) != sI.personality)
    {
        memset(&sI, 0, sizeof(sI));
        sI.personality = GetMonData(mon, MON_DATA_PERSONALITY);
        sI.frame = 1;
    }
}

// Debug (mp life verb V): as if V was picked on the wheel in front of the follower
void MP_LifeInteract_Debug(s8 verb)
{
    u8 taskId = CreateTask(Task_Interact, 80);
    LockPlayerFieldControls();
    gTasks[taskId].data[2] = verb;
    gTasks[taskId].data[0] = 2;
}
