// Moemon Co-op: MOéMON GARDEN among friends (docs/POKEMON_GARDEN.md 6.4, 7, 10).
//
// Each save keeps its own side: our Pokémon's feelings about friends and their Pokémon live in our .life
// file, and feelings can be one-sided. Solo is complete; friends add Trainers and Pokémon.
//   - Friends see what our follower is doing on its own (the wire state carries its activity, pose, its
//     own tile and its emote bubbles; mp_follower.c walks it there on their screen).
//   - When Trainers pause together, followers meet: they greet, play, or squabble according to their bonds
//     (mp_life_field.c LIFE_ACT_MINGLE), and a Pokémon that's fond of a friend runs to greet them.
//   - Being around friends slowly makes them familiar; a favorite Trainer can be someone else (shown
//     warmly: it runs to greet them), but only the owner's moments ever raise Friendship.
//   - Anyone can talk to, pet, feed (from their own bag)... a friend's follower. Its own game decides how it
//     answers, through its bond with that person (MPR_LIFE_VERB / REPLY). Never Friendship.
//   - A new LOOK gets a double take: surprise, a sniff, then happy recognition (shy ones hesitate longer;
//     shorter for friends who change often).
//   - Records travel with their Pokémon: a trade sends the outgoing one's record and bonds (it greets its old
//     Trainer warmly ever after), and a Pokémon from the shared PC asks the room for its story.
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "mp.h"
#include "mp_life.h"
#include "mp_life_emote.h"
#include "mp_life_field.h"
#include "mp_life_social.h"
#include "mp_garden.h"
#include "data.h"
#include "event_object_movement.h"
#include "field_message_box.h"
#include "field_player_avatar.h"
#include "item.h"
#include "main.h"
#include "pokemon.h"
#include "random.h"
#include "script.h"
#include "sound.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "constants/items.h"
#include "constants/songs.h"

void MP_OpenVerbWheel(void (*done)(s8 verb));   // mp_menu.c: -1 = nothing picked

#define MPR_LIFE_VERB    (MPR_LIFE_FIRST + 0)   // u8 verb, u16 item: to a follower's Trainer
#define MPR_LIFE_REPLY   (MPR_LIFE_FIRST + 1)   // u8 verb, u8 answer, u16 item
#define MPR_LIFE_RECORD  (MPR_LIFE_FIRST + 2)   // u8 kind (0 ask: u32 key; 1 answer, 2 trade: a packed record)

enum
{
    ANS_NONE, ANS_HAPPY, ANS_POLITE, ANS_SHY, ANS_SIDESTEP, ANS_ATE, ANS_FULL, ANS_PLAYED, ANS_PROUD,
    ANS_COMFORTED, ANS_TALK, ANS_LOVES, ANS_ASLEEP, ANS_COUNT
};

static struct
{
    // friends nearby
    s16 lastX[MP_MAX_PLAYERS], lastY[MP_MAX_PLAYERS];
    u16 still[MP_MAX_PLAYERS];          // frames their Trainer has stood still
    u8 emoteSeq[MP_MAX_PLAYERS];
    bool8 emoteSeen[MP_MAX_PLAYERS];
    u32 lookVersion[MP_MAX_PLAYERS];
    bool8 lookSeen[MP_MAX_PLAYERS];
    u8 lookChanges[MP_MAX_PLAYERS];     // LOOK changes lately (fewer double takes for frequent changers)
    u32 metMin[MP_MAX_PLAYERS];         // play minute our follower last met theirs (or them)
    u32 lastVerbFrame[MP_MAX_PLAYERS];  // the last thing they did with our follower
    u16 nearTimer;
    u32 frame;
    // the double take: surprise, a sniff, recognition
    u8 takeStage;
    u16 takeTimer;
    // records asked for this session (each once)
    u32 asked[16];
    u8 askedCount;
    // talking to a friend's follower
    u8 taskId;
    u8 friendId;
    s8 verb;
    u16 item;
    u8 answer;
    u32 targetKey;          // which of their Pokémon (their follower, or one in the MOéMON GARDEN)
    u16 targetDex;
} sS = {.taskId = TASK_NONE};

static bool8 SameMap(u8 id)
{
    return gMpPlayers[id].present && gMpPlayers[id].hasState && gSaveBlock1Ptr != NULL
        && gMpPlayers[id].state.mapGroup == gSaveBlock1Ptr->location.mapGroup
        && gMpPlayers[id].state.mapNum == gSaveBlock1Ptr->location.mapNum;
}

static u32 FriendKey(u8 id)
{
    u32 key = MP_Life_TrainerKey(gMpPlayers[id].otId, gMpPlayers[id].name);
    MP_Life_NoteName(key, gMpPlayers[id].name);
    return key;
}

// "ALICE's SKITTY" for a friend's follower (we know its species, not its nickname)
static void FriendMonNameOf(u8 id, u16 dex, u8 *out)
{
    u16 species = NationalPokedexNumToSpecies(dex);
    u8 *end = StringCopy(out, gMpPlayers[id].name);
    static const u8 sS_[] = _("'s ");
    end = StringCopy(end, sS_);
    StringCopy(end, gSpeciesNames[species]);
}

static void FriendMonName(u8 id, u8 *out)
{
    FriendMonNameOf(id, gMpPlayers[id].state.followerDex, out);
}

static void NoteFriendFollower(u8 id)
{
    u8 name[40];
    if (gMpPlayers[id].state.followerKey == 0 || gMpPlayers[id].state.followerDex == 0)
        return;
    FriendMonName(id, name);
    name[23] = EOS;
    MP_Life_NoteName(gMpPlayers[id].state.followerKey, name);
}

static bool8 RemoteOwnerTile(u8 id, s16 *x, s16 *y)
{
    u8 obj = MP_GetRemoteObjectEventId(id);
    if (obj >= OBJECT_EVENTS_COUNT || !gObjectEvents[obj].active)
        return FALSE;
    *x = gObjectEvents[obj].currentCoords.x;
    *y = gObjectEvents[obj].currentCoords.y;
    return TRUE;
}

static u32 LocalFollowerKey(void)
{
    struct MpLifeRec *rec = MP_LifeField_FollowerRec();
    return rec ? MP_Life_MonKey(rec->personality, rec->otId) : 0;
}

// ---------------------------------------------------------------------------
// The wire state

void MP_LifeSocial_CollectState(struct MpState *s)
{
    struct MpLifeRec *rec = s->followerDex ? MP_LifeField_FollowerRec() : NULL;
    s16 x, y;
    s->followerKey = rec ? MP_Life_MonKey(rec->personality, rec->otId) : 0;
    s->lifeAct = rec ? MP_LifeField_Activity() : 0;
    s->lifePose = rec ? MP_Follower_SlotPose(0) : 0;
    s->lifeDx = s->lifeDy = MP_LIFE_NO_AUTO;
    if (rec && MP_Follower_AutoTargetTile(&x, &y) && gPlayerAvatar.objectEventId < OBJECT_EVENTS_COUNT)
    {
        struct ObjectEvent *me = &gObjectEvents[gPlayerAvatar.objectEventId];
        s16 dx = x - me->currentCoords.x, dy = y - me->currentCoords.y;
        if (dx > -20 && dx < 20 && dy > -20 && dy < 20)
        {
            s->lifeDx = dx;
            s->lifeDy = dy;
        }
    }
    s->lifeEmote = MP_LifeEmote_Current(LIFE_EMOTE_SLOT_LOCAL);
    s->lifeEmoteSeq = MP_LifeEmote_Seq(LIFE_EMOTE_SLOT_LOCAL);
}

// ---------------------------------------------------------------------------
// Meeting friends' followers (mp_life_field.c)

static u8 MeetingEmote(struct MpLifeRec *rec, struct MpLifeBond *bond, u8 theirAct, u8 *pose)
{
    *pose = LIFE_POSE_SIT;
    if (bond == NULL || bond->v[BOND_FAMILIAR] < 20)
    {
        // a first meeting: its opening stance (docs 5.3)
        if (rec->cur[LIFE_BOLD] < 90)
            return LIFE_EMOTE_SWEAT;
        return rec->cur[LIFE_WARM] >= 150 ? LIFE_EMOTE_HEART : LIFE_EMOTE_QUESTION;
    }
    if (bond->v[BOND_FRICTION] >= 60 && bond->v[BOND_FRICTION] > bond->v[BOND_AFFECTION] / 2)
    {
        *pose = LIFE_POSE_SHAKE;   // a squabble
        return bond->v[BOND_RESPECT] >= 80 ? LIFE_EMOTE_SPARKLE : LIFE_EMOTE_ANGRY;
    }
    if (rec->cur[LIFE_LIVELY] >= 150 || theirAct == LIFE_ACT_PLAY)
    {
        *pose = LIFE_POSE_HOP;     // tag
        return LIFE_EMOTE_BALL;
    }
    return bond->v[BOND_AFFECTION] >= 110 ? LIFE_EMOTE_HEART : LIFE_EMOTE_NOTE;
}

bool8 MP_LifeSocial_MingleTarget(struct MpLifeRec *rec, u8 *friendId, s16 *x, s16 *y, s16 *fx, s16 *fy, u8 *emote,
                                 u8 *pose, bool8 *greetTrainer)
{
    static const s8 d[4][2] = {{0, 1}, {1, 0}, {-1, 0}, {0, -1}};
    u32 me = MP_Life_MonKey(rec->personality, rec->otId), now = MP_Life_PlayMinutes();
    u8 i, k;
    struct ObjectEvent *owner = &gObjectEvents[gPlayerAvatar.objectEventId];
    for (i = 0; i < MP_MAX_PLAYERS; i++)
    {
        s16 ox, oy, tx, ty;
        struct MpLifeBond *trainerBond;
        bool8 greet;
        if (i == gMpLocalId || !SameMap(i) || sS.still[i] < 90 || !RemoteOwnerTile(i, &ox, &oy))
            continue;
        if (abs(ox - owner->currentCoords.x) + abs(oy - owner->currentCoords.y) > 6)
            continue;
        if (sS.metMin[i] && now < sS.metMin[i] + 2)
            continue;   // met a moment ago
        trainerBond = MP_Life_Bond(me, FriendKey(i), FALSE);
        greet = trainerBond && trainerBond->v[BOND_AFFECTION] >= 120;   // it's fond of them: it runs to them
        if (greet || !MP_Follower_SlotTile(1 + i, &tx, &ty))
        {
            if (!greet)
                continue;
            tx = ox;
            ty = oy;
        }
        for (k = 0; k < 4; k++)
        {
            s16 cx = tx + d[k][0], cy = ty + d[k][1];
            if (MP_Follower_CanStandAt(cx, cy) && !(cx == ox && cy == oy))
            {
                *friendId = i;
                *x = cx;
                *y = cy;
                *fx = tx;
                *fy = ty;
                *greetTrainer = greet;
                if (greet)
                {
                    *pose = LIFE_POSE_HOP;
                    *emote = LIFE_EMOTE_HEART;
                }
                else
                {
                    *emote = MeetingEmote(rec, MP_Life_Bond(me, gMpPlayers[i].state.followerKey, FALSE),
                                          gMpPlayers[i].state.lifeAct, pose);
                }
                return TRUE;
            }
        }
    }
    return FALSE;
}

// They met: a shared moment (docs 5.3: bonds grow from shared life)
void MP_LifeSocial_Met(u8 id, bool8 trainer)
{
    struct MpLifeRec *rec = MP_LifeField_FollowerRec();
    u32 me, them;
    if (rec == NULL || id >= MP_MAX_PLAYERS)
        return;
    me = MP_Life_MonKey(rec->personality, rec->otId);
    sS.metMin[id] = MP_Life_PlayMinutes();
    if (trainer)
    {
        them = FriendKey(id);
        MP_Life_BondShare(me, them, 2, 1, 2, 0, 0);
        MPNet_Log("life social: greeted player %d", id + 1);
        return;
    }
    them = gMpPlayers[id].state.followerKey;
    if (them == 0)
        return;
    NoteFriendFollower(id);
    {
        struct MpLifeBond *b = MP_Life_Bond(me, them, FALSE);
        bool8 first = b == NULL || b->shared == 0;
        bool8 squabble = b && b->v[BOND_FRICTION] >= 60 && b->v[BOND_FRICTION] > b->v[BOND_AFFECTION] / 2;
        bool8 competitive = rec->cur[LIFE_PROUD] >= 170 || rec->cur[LIFE_BOLD] >= 180;
        MP_Life_BondShare(me, them, 3, squabble ? 0 : 1, squabble ? 0 : (rec->cur[LIFE_WARM] >= 140 ? 2 : 1), competitive ? 1 : 0,
                          competitive && squabble ? 1 : 0);
        b = MP_Life_Bond(me, them, FALSE);
        if (b && b->v[BOND_AFFECTION] >= 150 && b->v[BOND_TRUST] >= 120)
        {
            // a friend made (once)
            u8 m;
            bool8 known = FALSE;
            for (m = 0; m < LIFE_MEMORIES; m++)
                if (rec->mem[m].kind == MEM_FRIEND && rec->mem[m].who == them)
                    known = TRUE;
            if (!known)
                MP_Life_Remember(rec, MEM_FRIEND, 150, TIER_PART, them, 0);
        }
        MPNet_Log("life social: met player %d's follower%s", id + 1, first ? " (first time)" : "");
    }
}

// ---------------------------------------------------------------------------
// Friends nearby: familiarity, their followers' bubbles, a new LOOK

static void DoubleTake(void)
{
    struct MpLifeRec *rec;
    if (sS.takeStage == 0)
        return;
    if (sS.takeTimer)
    {
        sS.takeTimer--;
        return;
    }
    rec = MP_LifeField_FollowerRec();
    if (rec == NULL)
    {
        sS.takeStage = 0;
        return;
    }
    switch (sS.takeStage)
    {
    case 1:   // surprise
        MP_LifeField_Reaction(LIFE_EMOTE_EXCLAIM, LIFE_POSE_NONE, 0);
        sS.takeTimer = rec->cur[LIFE_BOLD] < 90 ? 110 : 70;   // shy ones hesitate a moment longer
        sS.takeStage = 2;
        break;
    case 2:   // a sniff
        MP_LifeField_Reaction(LIFE_EMOTE_QUESTION, LIFE_POSE_SHAKE, 50);
        sS.takeTimer = 80;
        sS.takeStage = 3;
        break;
    case 3:   // it's them!
        MP_LifeField_Reaction(LIFE_EMOTE_HEART, LIFE_POSE_HOP, 50);
        MPNet_Log("life social: double take done");
        sS.takeStage = 0;
        break;
    }
}

static void Nearby(void)
{
    struct MpLifeRec *rec = MP_LifeField_FollowerRec();
    u32 me = rec ? MP_Life_MonKey(rec->personality, rec->otId) : 0;
    struct ObjectEvent *owner;
    u8 i, p;
    if (gPlayerAvatar.objectEventId >= OBJECT_EVENTS_COUNT)
        return;
    owner = &gObjectEvents[gPlayerAvatar.objectEventId];
    for (i = 0; i < MP_MAX_PLAYERS; i++)
    {
        s16 x, y;
        u32 tk;
        if (i == gMpLocalId || !SameMap(i) || !RemoteOwnerTile(i, &x, &y))
            continue;
        if (abs(x - owner->currentCoords.x) + abs(y - owner->currentCoords.y) > 8)
            continue;
        tk = FriendKey(i);
        // the follower gets to know them; the rest of the party a little, from their balls
        if (me)
        {
            MP_Life_BondNudge(me, tk, BOND_FAMILIAR, 2, 120);
            if (gMpPlayers[i].state.followerKey)
            {
                NoteFriendFollower(i);
                MP_Life_BondNudge(me, gMpPlayers[i].state.followerKey, BOND_FAMILIAR, 1, 80);
            }
        }
        for (p = 0; p < PARTY_SIZE; p++)
        {
            struct MpLifeRec *r = MP_Life_Get(&gPlayerParty[p].box, FALSE);
            if (r && r != rec)
                MP_Life_BondNudge(MP_Life_MonKey(r->personality, r->otId), tk, BOND_FAMILIAR, 1, 60);
        }
    }
}

void MP_LifeSocial_Update(void)
{
    u8 i;
    sS.frame++;
    if (!MP_IsInOverworld() || gSaveBlock1Ptr == NULL)
        return;
    for (i = 0; i < MP_MAX_PLAYERS; i++)
    {
        s16 x, y;
        struct MpPlayer *p = &gMpPlayers[i];
        if (i == gMpLocalId || !p->present || !p->hasState)
        {
            sS.emoteSeen[i] = sS.lookSeen[i] = FALSE;
            sS.still[i] = 0;
            continue;
        }
        // standing still?
        if (RemoteOwnerTile(i, &x, &y) && x == sS.lastX[i] && y == sS.lastY[i])
        {
            if (sS.still[i] < 0xFFFF)
                sS.still[i]++;
        }
        else
        {
            sS.still[i] = 0;
            sS.lastX[i] = x;
            sS.lastY[i] = y;
        }
        // their follower's bubbles
        if (!sS.emoteSeen[i])
        {
            sS.emoteSeen[i] = TRUE;
            sS.emoteSeq[i] = p->state.lifeEmoteSeq;
        }
        else if (sS.emoteSeq[i] != p->state.lifeEmoteSeq)
        {
            sS.emoteSeq[i] = p->state.lifeEmoteSeq;
            if (p->state.lifeEmote < LIFE_EMOTE_COUNT && SameMap(i))
                MP_LifeEmote_Show(LIFE_EMOTE_SLOT_REMOTE(i), p->state.lifeEmote);
        }
        // a new LOOK: a double take from our follower, if it knows them
        if (!sS.lookSeen[i])
        {
            sS.lookSeen[i] = TRUE;
            sS.lookVersion[i] = p->appearanceVersion;
        }
        else if (sS.lookVersion[i] != p->appearanceVersion)
        {
            struct MpLifeBond *b;
            u32 me = LocalFollowerKey();
            sS.lookVersion[i] = p->appearanceVersion;
            b = me ? MP_Life_Bond(me, FriendKey(i), FALSE) : NULL;
            if (b && b->v[BOND_FAMILIAR] >= 20 && SameMap(i) && sS.takeStage == 0)
            {
                if (sS.lookChanges[i] < 255)
                    sS.lookChanges[i]++;
                if (sS.lookChanges[i] > 3)
                {
                    MP_LifeField_Reaction(LIFE_EMOTE_QUESTION, LIFE_POSE_NONE, 0);   // used to it by now
                    MPNet_Log("life social: a quick look (player %d)", i + 1);
                }
                else
                {
                    sS.takeStage = 1;
                    sS.takeTimer = 20;
                    MPNet_Log("life social: double take (player %d)", i + 1);
                }
            }
        }
    }
    // frequent changers: the count fades (one every 5 minutes)
    if ((sS.frame % (60 * 60 * 5)) == 0)
        for (i = 0; i < MP_MAX_PLAYERS; i++)
            if (sS.lookChanges[i])
                sS.lookChanges[i]--;
    DoubleTake();
    if (++sS.nearTimer >= 600)
    {
        sS.nearTimer = 0;
        Nearby();
    }
}

// ---------------------------------------------------------------------------
// Doing something with a friend's follower: our side

static const u8 sAns_Happy[] = _("{STR_VAR_1} looks really happy!");
static const u8 sAns_Polite[] = _("{STR_VAR_1} lets you, politely.");
static const u8 sAns_Shy[] = _("{STR_VAR_1} hides behind its TRAINER...");
static const u8 sAns_Sidestep[] = _("{STR_VAR_1} steps aside. Not that, thanks.");
static const u8 sAns_Ate[] = _("{STR_VAR_1} munched your {STR_VAR_2}!");
static const u8 sAns_Full[] = _("{STR_VAR_1} sniffs the {STR_VAR_2}, but it isn't hungry.");
static const u8 sAns_Played[] = _("You and {STR_VAR_1} played for a while!");
static const u8 sAns_Proud[] = _("{STR_VAR_1} puffs up proudly!");
static const u8 sAns_Comforted[] = _("{STR_VAR_1} leans against you. It seems calmer.");
static const u8 sAns_Talk[] = _("{STR_VAR_1} listens, head tilted.");
static const u8 sAns_Loves[] = _("{STR_VAR_1} runs to you, overjoyed!");
static const u8 sAns_Asleep[] = _("{STR_VAR_1} is fast asleep.");
static const u8 sAns_None[] = _("{STR_VAR_1} looks at you curiously.");
static const u8 sAns_NoBerry[] = _("You don't have any BERRIES.");

static const u8 *const sAnswers[ANS_COUNT] = {
    [ANS_NONE] = sAns_None, [ANS_HAPPY] = sAns_Happy, [ANS_POLITE] = sAns_Polite, [ANS_SHY] = sAns_Shy,
    [ANS_SIDESTEP] = sAns_Sidestep, [ANS_ATE] = sAns_Ate, [ANS_FULL] = sAns_Full, [ANS_PLAYED] = sAns_Played,
    [ANS_PROUD] = sAns_Proud, [ANS_COMFORTED] = sAns_Comforted, [ANS_TALK] = sAns_Talk, [ANS_LOVES] = sAns_Loves,
    [ANS_ASLEEP] = sAns_Asleep,
};

#define tState data[0]
#define tTimer data[1]

static void Task_Friend(u8 taskId);

static void WheelDone(s8 verb)
{
    if (sS.taskId != TASK_NONE && gTasks[sS.taskId].isActive && gTasks[sS.taskId].func == Task_Friend)
    {
        sS.verb = verb;
        gTasks[sS.taskId].tState = 2;
    }
}

static u16 FirstBerry(void)
{
    u16 item;
    for (item = FIRST_BERRY_INDEX; item <= LAST_BERRY_INDEX; item++)
        if (CheckBagHasItem(item, 1))
            return item;
    return ITEM_NONE;
}

static void ShowAnswer(u8 taskId, const u8 *text)
{
    FriendMonNameOf(sS.friendId, sS.targetDex, gStringVar1);
    if (sS.item)
        CopyItemName(sS.item, gStringVar2);
    StringExpandPlaceholders(gStringVar4, text);
    MP_ShowFieldMessageFitted(gStringVar4);
    gTasks[taskId].tState = 4;
}

static void Task_Friend(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    switch (tState)
    {
    case 0:   // tap: the fitting thing (as far as we can tell); hold: the wheel
        if (!JOY_HELD(A_BUTTON))
        {
            u8 act = sS.targetKey == gMpPlayers[sS.friendId].state.followerKey ? gMpPlayers[sS.friendId].state.lifeAct : LIFE_ACT_NONE;
            sS.verb = act == LIFE_ACT_PLAY ? LIFE_VERB_PLAY : LIFE_VERB_TALK;
            if (act == LIFE_ACT_NAP)
            {
                ShowAnswer(taskId, sAns_Asleep);
                break;
            }
            tState = 2;
        }
        else if (++tTimer >= 20)
        {
            tState = 1;
            MP_OpenVerbWheel(WheelDone);
        }
        break;
    case 1:
        break;
    case 2:   // ask its Trainer's game
    {
        u8 buf[8];
        if (sS.verb < 0)
        {
            UnlockPlayerFieldControls();
            DestroyTask(taskId);
            sS.taskId = TASK_NONE;
            break;
        }
        sS.item = ITEM_NONE;
        if (sS.verb == LIFE_VERB_FEED && (sS.item = FirstBerry()) == ITEM_NONE)
        {
            ShowAnswer(taskId, sAns_NoBerry);
            break;
        }
        buf[0] = MPR_LIFE_VERB;
        buf[1] = (u8)sS.verb;
        buf[2] = sS.item & 0xFF;
        buf[3] = sS.item >> 8;
        buf[4] = sS.targetKey;
        buf[5] = sS.targetKey >> 8;
        buf[6] = sS.targetKey >> 16;
        buf[7] = sS.targetKey >> 24;
        sS.answer = 0xFF;
        MP_SendRelay(sS.friendId, buf, sizeof(buf));
        MPNet_Log("life social: verb %d to player %d", sS.verb, sS.friendId + 1);
        tTimer = 0;
        tState = 3;
        break;
    }
    case 3:   // its answer
        if (sS.answer != 0xFF || ++tTimer > 360)
        {
            u8 a = sS.answer < ANS_COUNT ? sS.answer : ANS_NONE;
            if (a == ANS_ATE && sS.item)
                RemoveBagItem(sS.item, 1);   // food from a friend comes from their own bag (docs 6.4)
            if (a == ANS_HAPPY || a == ANS_PLAYED || a == ANS_LOVES || a == ANS_ATE)
                PlayCry_NormalNoDucking(NationalPokedexNumToSpecies(sS.targetDex), 0, CRY_VOLUME_RS / 2, CRY_PRIORITY_NORMAL);
            ShowAnswer(taskId, sAnswers[a]);
        }
        break;
    case 4:
        if (!IsTextPrinterActive(0) && JOY_NEW(A_BUTTON | B_BUTTON))
        {
            HideFieldMessageBox();
            tState = 5;
        }
        break;
    case 5:
        if (IsFieldMessageBoxHidden())
        {
            UnlockPlayerFieldControls();
            DestroyTask(taskId);
            sS.taskId = TASK_NONE;
        }
        break;
    }
}

bool8 MP_LifeSocial_TryFriendFollower(void)
{
    struct ObjectEvent *me;
    s16 x, y;
    u8 i;
    if (!JOY_NEW(A_BUTTON) || !MP_IsLocalFree() || (sS.taskId != TASK_NONE && gTasks[sS.taskId].isActive))
        return FALSE;
    me = &gObjectEvents[gPlayerAvatar.objectEventId];
    x = me->currentCoords.x;
    y = me->currentCoords.y;
    MoveCoords(me->facingDirection, &x, &y);
    if (GetObjectEventIdByXY(x, y) != OBJECT_EVENTS_COUNT)
        return FALSE;
    for (i = 0; i < MP_MAX_PLAYERS; i++)
    {
        s16 fx, fy;
        if (i == gMpLocalId || !SameMap(i) || gMpPlayers[i].state.followerDex == 0 || !MP_Follower_SlotTile(1 + i, &fx, &fy))
            continue;
        if (fx != x || fy != y)
            continue;
        sS.friendId = i;
        sS.targetKey = gMpPlayers[i].state.followerKey;
        sS.targetDex = gMpPlayers[i].state.followerDex;
        LockPlayerFieldControls();
        sS.taskId = CreateTask(Task_Friend, 80);
        return TRUE;
    }
    return FALSE;
}

// A in front of a friend's Pokémon in the MOéMON GARDEN (mp_garden_life.c)
bool8 MP_LifeSocial_TryFriendPokemon(s8 owner, u32 key, u16 dex)
{
    if (owner < 0 || owner >= MP_MAX_PLAYERS || (sS.taskId != TASK_NONE && gTasks[sS.taskId].isActive))
        return FALSE;
    sS.friendId = owner;
    sS.targetKey = key;
    sS.targetDex = dex;
    LockPlayerFieldControls();
    sS.taskId = CreateTask(Task_Friend, 80);
    return TRUE;
}

// ---------------------------------------------------------------------------
// ... and its Trainer's side: our follower answers through its bond with them (never Friendship)

void MP_GardenLife_ReactKey(u32 key, u8 emote, u8 pose, u16 frames);

static u8 Answer(struct MpLifeRec *rec, u8 from, u8 verb, u16 item, bool8 asleep)
{
    u32 me = MP_Life_MonKey(rec->personality, rec->otId), tk = FriendKey(from);
    struct MpLifeBond *b = MP_Life_Bond(me, tk, TRUE);
    u32 fv, fr, rv, fa;
    bool8 repeat = sS.frame - sS.lastVerbFrame[from] < 60 * 60;
    bool8 shy = rec->cur[LIFE_BOLD] < 90 && b && b->v[BOND_FAMILIAR] < 40;
    u8 ans, aff0 = b ? b->v[BOND_AFFECTION] : 0;
    sS.lastVerbFrame[from] = sS.frame;
    MP_Life_Closest(me, &fr, &rv, &fa, &fv);
    if (asleep)
        return ANS_ASLEEP;
    if (verb < LIFE_VERBS && (rec->boundaries & (1 << verb)))
        return ANS_SIDESTEP;   // a boundary: no mark on the bond
    if (shy && (verb == LIFE_VERB_PET || verb == LIFE_VERB_GROOM || verb == LIFE_VERB_PLAY))
    {
        MP_Life_BondNudge(me, tk, BOND_FAMILIAR, 2, 255);
        return ANS_SHY;
    }
    switch (verb)
    {
    case LIFE_VERB_FEED:
        if (rec->need[NEED_HUNGER] < 120 || item == ITEM_NONE)
            return ANS_FULL;
        MP_Life_Satisfy(rec, NEED_HUNGER, 70);
        MP_Life_Feel(rec, EMO_JOY, 20);
        ans = ANS_ATE;
        break;
    case LIFE_VERB_PLAY:
        if (rec->cur[LIFE_LIVELY] < 110 && rec->need[NEED_PLAY] < 120)
            return ANS_POLITE;
        MP_Life_Satisfy(rec, NEED_PLAY, 60);
        MP_Life_Feel(rec, EMO_JOY, 20);
        ans = ANS_PLAYED;
        break;
    case LIFE_VERB_PET:
    case LIFE_VERB_GROOM:
        ans = rec->cur[LIFE_WARM] >= 110 ? ANS_HAPPY : ANS_POLITE;
        break;
    case LIFE_VERB_PRAISE:
        ans = rec->emo[EMO_PRIDE] >= 30 || rec->cur[LIFE_PROUD] >= 160 ? ANS_PROUD : ANS_POLITE;
        break;
    case LIFE_VERB_COMFORT:
        if (rec->emo[EMO_FEAR] + rec->emo[EMO_SADNESS] + rec->emo[EMO_FRUSTRATION] < 40)
            return ANS_POLITE;
        MP_Life_Feel(rec, EMO_CALM, 25);
        ans = ANS_COMFORTED;
        break;
    default:
        ans = ANS_TALK;
        break;
    }
    if (ans != ANS_POLITE && fv == tk)
        ans = ans == ANS_TALK || ans == ANS_HAPPY ? ANS_LOVES : ans;   // its favorite: shown warmly
    if (repeat)
        MP_Life_BondNudge(me, tk, BOND_FAMILIAR, 1, 255);   // again so soon: ordinary, never negative
    else
        MP_Life_BondShare(me, tk, 2, ans == ANS_POLITE ? 0 : 2, ans == ANS_POLITE ? 0 : 3, ans == ANS_PROUD ? 2 : 0, 0);
    b = MP_Life_Bond(me, tk, FALSE);
    if (b && aff0 < 120 && b->v[BOND_AFFECTION] >= 120)
        MP_Life_Remember(rec, MEM_FRIEND, 140, TIER_PART, tk, 1);   // took to them
    return ans;
}

static const u8 sAnswerEmote[ANS_COUNT] = {
    [ANS_NONE] = LIFE_EMOTE_QUESTION, [ANS_HAPPY] = LIFE_EMOTE_HEART, [ANS_POLITE] = LIFE_EMOTE_CALM,
    [ANS_SHY] = LIFE_EMOTE_SWEAT, [ANS_SIDESTEP] = LIFE_EMOTE_DOTS, [ANS_ATE] = LIFE_EMOTE_FOOD,
    [ANS_FULL] = LIFE_EMOTE_DOTS, [ANS_PLAYED] = LIFE_EMOTE_NOTE, [ANS_PROUD] = LIFE_EMOTE_SPARKLE,
    [ANS_COMFORTED] = LIFE_EMOTE_HEART, [ANS_TALK] = LIFE_EMOTE_NOTE, [ANS_LOVES] = LIFE_EMOTE_HEART,
    [ANS_ASLEEP] = LIFE_EMOTE_ZZZ,
};

void MP_LifeSocial_OnRelay(u8 from, const u8 *data, u16 len)
{
    if (from >= MP_MAX_PLAYERS)
        return;
    switch (data[0])
    {
    case MPR_LIFE_VERB:
    {
        struct MpLifeRec *rec = MP_LifeField_FollowerRec();
        u8 reply[5], ans, pose;
        u16 item;
        u32 key = 0;
        bool8 garden = FALSE;
        if (len < 4)
            return;
        item = data[2] | (data[3] << 8);
        if (len >= 8)
            key = data[4] | (data[5] << 8) | (data[6] << 16) | ((u32)data[7] << 24);
        if (key && MP_Garden_IsHere() && MP_Life_PartyBox(key) != NULL)
        {
            rec = MP_Life_FindKey(key);   // one of ours in the Garden
            garden = TRUE;
        }
        ans = rec ? Answer(rec, from, data[1], item, garden ? FALSE : MP_LifeField_Activity() == LIFE_ACT_NAP) : ANS_NONE;
        pose = ans == ANS_PLAYED || ans == ANS_LOVES ? LIFE_POSE_HOP : ans == ANS_SHY ? LIFE_POSE_SIT : LIFE_POSE_NONE;
        if (rec && ans != ANS_ASLEEP)
        {
            if (garden)
                MP_GardenLife_ReactKey(key, sAnswerEmote[ans], pose, 90);
            else
                MP_LifeField_Reaction(sAnswerEmote[ans], pose, 90);
        }
        reply[0] = MPR_LIFE_REPLY;
        reply[1] = data[1];
        reply[2] = ans;
        reply[3] = data[2];
        reply[4] = data[3];
        MP_SendRelay(from, reply, sizeof(reply));
        MPNet_Log("life social: player %d did verb %u: answer %u", from + 1, data[1], ans);
        break;
    }
    case MPR_LIFE_REPLY:
        if (len >= 3 && from == sS.friendId && sS.taskId != TASK_NONE)
        {
            sS.answer = data[2];
            MPNet_Log("life social: answer %u", data[2]);
        }
        break;
    case MPR_LIFE_RECORD:
        if (len < 2)
            return;
        if (data[1] == 0 && len >= 6)
        {
            // someone asks about a Pokémon: if we know it, its story goes to them
            u32 key = data[2] | (data[3] << 8) | (data[4] << 16) | ((u32)data[5] << 24);
            if (MP_Life_FindKey(key))
                MP_LifeSocial_SendRecord(from, key, FALSE);
        }
        else if (data[1] == 1 || data[1] == 2)
        {
            MP_Life_UnpackRecord(data + 2, len - 2, data[1] == 2, gMpPlayers[from].name, FriendKey(from));
        }
        break;
    }
}

// ---------------------------------------------------------------------------
// Records travel

void MP_LifeSocial_SendRecord(u8 target, u32 key, bool8 traded)
{
    static u8 buf[2 + 4096];
    u16 n;
    if (!MP_IsConnected())
        return;
    n = MP_Life_PackRecord(key, buf + 2, sizeof(buf) - 2);
    if (n == 0)
        return;
    buf[0] = MPR_LIFE_RECORD;
    buf[1] = traded ? 2 : 1;
    MP_SendRelay(target, buf, n + 2);
    MPNet_Log("life social: sent a record (%s, %u bytes)", traded ? "trade" : "answer", n);
}

void MP_LifeSocial_AskRecord(u32 key)
{
    u8 buf[6], i;
    if (!MP_IsConnected() || key == 0)
        return;
    for (i = 0; i < sS.askedCount; i++)
        if (sS.asked[i] == key)
            return;
    if (sS.askedCount < ARRAY_COUNT(sS.asked))
        sS.asked[sS.askedCount++] = key;
    buf[0] = MPR_LIFE_RECORD;
    buf[1] = 0;
    buf[2] = key;
    buf[3] = key >> 8;
    buf[4] = key >> 16;
    buf[5] = key >> 24;
    MP_SendRelay(0xFF, buf, sizeof(buf));
}

void MP_LifeSocial_Debug(const char *args)
{
    char cmd[32] = "";
    int a = 0, b = 0;
    sscanf(args, "%31s %d %d", cmd, &a, &b);
    if (strcmp(cmd, "bonds") == 0)
    {
        // bonds: our follower's bonds with each friend and their follower
        u32 me = LocalFollowerKey();
        u8 i;
        for (i = 0; i < MP_MAX_PLAYERS; i++)
        {
            struct MpLifeBond *t, *f;
            if (i == gMpLocalId || !gMpPlayers[i].present)
                continue;
            t = MP_Life_Bond(me, FriendKey(i), FALSE);
            f = gMpPlayers[i].state.followerKey ? MP_Life_Bond(me, gMpPlayers[i].state.followerKey, FALSE) : NULL;
            printf("[test] life social player %d: trainer fam %d aff %d | follower %08x fam %d aff %d shared %d | still %u act %u dx %d\n",
                   i + 1, t ? t->v[BOND_FAMILIAR] : -1, t ? t->v[BOND_AFFECTION] : -1, (unsigned)gMpPlayers[i].state.followerKey,
                   f ? f->v[BOND_FAMILIAR] : -1, f ? f->v[BOND_AFFECTION] : -1, f ? f->shared : -1, sS.still[i],
                   gMpPlayers[i].state.lifeAct, gMpPlayers[i].state.lifeDx);
        }
    }
    else if (strcmp(cmd, "mingle") == 0)
    {
        // mingle: could our follower go and meet someone now? (and why not)
        struct MpLifeRec *rec = MP_LifeField_FollowerRec();
        u8 i;
        struct ObjectEvent *owner = &gObjectEvents[gPlayerAvatar.objectEventId];
        for (i = 0; i < MP_MAX_PLAYERS; i++)
        {
            s16 ox = 0, oy = 0, tx = 0, ty = 0;
            bool8 tile, ownerTile;
            if (i == gMpLocalId || !gMpPlayers[i].present)
                continue;
            ownerTile = RemoteOwnerTile(i, &ox, &oy);
            tile = MP_Follower_SlotTile(1 + i, &tx, &ty);
            printf("[test] life social mingle player %d: same map %d still %u owner %d (%d,%d) me (%d,%d) follower tile %d (%d,%d) can %d%d%d%d\n",
                   i + 1, SameMap(i), sS.still[i], ownerTile, ox, oy, owner->currentCoords.x, owner->currentCoords.y, tile, tx, ty,
                   MP_Follower_CanStandAt(tx, ty + 1), MP_Follower_CanStandAt(tx + 1, ty), MP_Follower_CanStandAt(tx - 1, ty),
                   MP_Follower_CanStandAt(tx, ty - 1));
        }
        if (rec)
        {
            u8 f, e, p;
            bool8 g;
            s16 x, y, fx, fy;
            printf("[test] life social mingle: %d\n", MP_LifeSocial_MingleTarget(rec, &f, &x, &y, &fx, &fy, &e, &p, &g));
        }
    }
    else if (strcmp(cmd, "verb") == 0 && a >= 1 && a <= MP_MAX_PLAYERS)
    {
        // verb PLAYER V: as if we picked V in front of that friend's follower
        sS.friendId = a - 1;
        sS.verb = b;
        sS.targetKey = gMpPlayers[a - 1].state.followerKey;
        sS.targetDex = gMpPlayers[a - 1].state.followerDex;
        LockPlayerFieldControls();
        sS.taskId = CreateTask(Task_Friend, 80);
        gTasks[sS.taskId].tState = 2;
        printf("[test] life social verb %d to player %d\n", b, a);
    }
    else if (strcmp(cmd, "trainerbond") == 0 && a >= 1 && a <= MP_MAX_PLAYERS)
    {
        // trainerbond PLAYER AFFECTION: our follower's fondness for that friend
        u32 me = LocalFollowerKey();
        struct MpLifeBond *t = me ? MP_Life_Bond(me, FriendKey(a - 1), TRUE) : NULL;
        if (t)
            t->v[BOND_AFFECTION] = t->v[BOND_FAMILIAR] = b;
        printf("[test] life social trainerbond %d = %d\n", a, b);
    }
}
