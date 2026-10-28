// Moemon Rhythm: the DANCE machine on the GAME CENTER's 3F, the map's side (the game itself: src/rhythm/,
// docs/RHYTHM_DECISIONS.md #12-#16).
//
// The pads: a player steps up from the row in front of the machine (cells 0-2, row 7) and stands on the left pad
// (1P) or the right one (2P), drawn up on it (their sprite moved by x2 / y2) and turning to the arrows they step on.
// Everyone hears where each player stands (MPR_RH_PAD, repeated): "Let's dance!" over someone waiting, the pads'
// dancers for everyone in the room on the map.
//
// Two players: a friend stepping up to the other pad while someone waits is asked "Dance with NAME?"; YES asks the
// waiting player's game (MPR_RH_ASK), which answers at once, and both start a credit together (the waiting player
// picks the songs). Then the machine's own messages go back and forth (MPR_RH_SESSION, rh_screen.c).
//
// Records: the server keeps them (mp_scores.c); the player's own bests, the songs they've cleared and their
// options are in "<save>.dance" (offline the bests stand in for the server's).
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "mp.h"
#include "mp_net.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "fieldmap.h"
#include "main.h"
#include "random.h"
#include "script.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "constants/event_objects.h"
#include "constants/maps.h"
#include "constants/songs.h"
#include "mp_version.h"
#include "rhythm.h"

void MPNet_Log(const char *fmt, ...);
void MP_Achieve_Count(const char *name, s32 n);
void MP_Achieve_Unlock(const char *id);
extern char gPlatformSavePath[512];

enum
{
    MPR_RH_PAD = MPR_RHYTHM_FIRST,  // u8 side, state (PAD_*), mapGroup, mapNum, arrows held
    MPR_RH_ASK,                     // u32 session, u8 side (the asker's)
    MPR_RH_ANSWER,                  // u32 session, u8 yes
    MPR_RH_SESSION,                 // the machine's own messages (rh_screen.c RN_*)
};

enum { PAD_EMPTY, PAD_WAITING, PAD_SOLO, PAD_VERSUS };
enum { RESULT_NONE, RESULT_PLAYED, RESULT_REFUSED };

#define PAD_RESEND  60
#define PAD_TTL     150
#define ASK_FRAMES  (60 * 6)
#define MACHINE_ROW 7               // where the players stand
#define PAD_LEFT_X  16              // the pads' middles in the map's pixels
#define PAD_RIGHT_X 32
#define PAD_FEET_Y  108

static const u8 sText_LetsDance[] = _("Let's dance!");
static const u8 sText_Dancing[] = _("Dancing!");
static const u8 sText_YourFriend[] = _("Your friend");

static struct
{
    u8 side, state, arrows, arrowsSent;
    u16 timer;
    u8 result;
    bool8 onPad;                // our sprite is drawn on the pad
    // asking / answering
    u32 session;
    s8 peer;
    u16 askTimer;
    bool8 asking, answered, answerYes;
} sMe;

static struct
{
    u8 side, state, mapGroup, mapNum, arrows;
    u16 ttl;
    bool8 posed;
} sFriends[MP_MAX_PLAYERS];

static bool8 OnMachineMap(u8 mapGroup, u8 mapNum)
{
    return mapGroup == MAP_GROUP(MAP_RUSTBORO_CITY_GAME_CENTER_3F) && mapNum == MAP_NUM(MAP_RUSTBORO_CITY_GAME_CENTER_3F);
}

static bool8 HereNow(void)
{
    return OnMachineMap(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum);
}

static bool8 FriendValid(s8 id)
{
    return id >= 0 && id < MP_MAX_PLAYERS && id != gMpLocalId && gMpPlayers[id].present;
}

static void Put32(u8 *p, u32 v)
{
    p[0] = v;
    p[1] = v >> 8;
    p[2] = v >> 16;
    p[3] = v >> 24;
}

static u32 Get32(const u8 *p)
{
    return p[0] | (p[1] << 8) | (p[2] << 16) | ((u32)p[3] << 24);
}

static void SetNameVar(s8 id)
{
    if (id >= 0 && id < MP_MAX_PLAYERS && gMpPlayers[id].present)
        StringCopyN(gStringVar1, gMpPlayers[id].name, MP_NAME_LEN);
    else
        StringCopy(gStringVar1, sText_YourFriend);
    gStringVar1[MP_NAME_LEN] = EOS;
}

// ---------------------------------------------------------------------------
// The pads

static void SendPad(void)
{
    u8 p[6];
    if (!MP_IsConnected())
        return;
    p[0] = MPR_RH_PAD;
    p[1] = sMe.side;
    p[2] = sMe.state;
    p[3] = gSaveBlock1Ptr->location.mapGroup;
    p[4] = gSaveBlock1Ptr->location.mapNum;
    p[5] = sMe.arrows;
    MP_SendRelay(0xFF, p, sizeof(p));
    sMe.arrowsSent = sMe.arrows;
}

static void SetPad(u8 state)
{
    if (sMe.state == state)
        return;
    sMe.state = state;
    sMe.timer = 0;
    SendPad();
    if (state == PAD_EMPTY && gMpLocalId >= 0)
        MP_Emote_ClearBubbleFor(gMpLocalId);
}

// The friend on a pad of the machine on our map (-1 nobody)
static s8 FriendOnPad(u8 side)
{
    s8 i;
    for (i = 0; i < MP_MAX_PLAYERS; i++)
        if (FriendValid(i) && sFriends[i].ttl && sFriends[i].state != PAD_EMPTY && sFriends[i].side == side
            && OnMachineMap(sFriends[i].mapGroup, sFriends[i].mapNum) && HereNow())
            return i;
    return -1;
}

// Where a sprite at (x, MACHINE_ROW) is drawn to stand on its pad
static void PadOffset(u8 side, s16 cellX, s16 *dx, s16 *dy)
{
    *dx = (side ? PAD_RIGHT_X : PAD_LEFT_X) - (cellX * 16 + 8);
    *dy = PAD_FEET_Y - (MACHINE_ROW * 16 + 16);
}

static u8 FacingFor(u8 arrows)
{
    if (arrows & 1)
        return DIR_WEST;
    if (arrows & 8)
        return DIR_EAST;
    if (arrows & 2)
        return DIR_SOUTH;
    return DIR_NORTH;
}

// A friend's sprite (mp_overworld.c, each frame): up on their pad while they're there
void MP_Rhythm_AvatarPose(u8 id, struct ObjectEvent *obj)
{
    struct Sprite *sprite;
    if (id >= MP_MAX_PLAYERS || obj == NULL || obj->spriteId >= MAX_SPRITES)
        return;
    sprite = &gSprites[obj->spriteId];
    if (sFriends[id].ttl && sFriends[id].state != PAD_EMPTY && OnMachineMap(sFriends[id].mapGroup, sFriends[id].mapNum)
        && obj->currentCoords.y - MAP_OFFSET == MACHINE_ROW && obj->currentCoords.x - MAP_OFFSET <= 2)
    {
        s16 dx, dy;
        u8 dir = FacingFor(sFriends[id].arrows);
        PadOffset(sFriends[id].side, obj->currentCoords.x - MAP_OFFSET, &dx, &dy);
        sprite->x2 = dx;
        sprite->y2 = dy - (sFriends[id].arrows ? 1 : 0);
        if (obj->facingDirection != dir)
            ObjectEventTurn(obj, dir);
        sFriends[id].posed = TRUE;
    }
    else if (sFriends[id].posed)
    {
        sprite->x2 = 0;
        sprite->y2 = 0;
        sFriends[id].posed = FALSE;
    }
}

static void PoseMe(bool8 on)
{
    struct ObjectEvent *me = &gObjectEvents[gPlayerAvatar.objectEventId];
    struct Sprite *sprite = &gSprites[gPlayerAvatar.spriteId];
    if (on)
    {
        s16 dx, dy;
        PadOffset(sMe.side, me->currentCoords.x - MAP_OFFSET, &dx, &dy);
        sprite->x2 = dx;
        sprite->y2 = dy;
        sMe.onPad = TRUE;
    }
    else if (sMe.onPad)
    {
        sprite->x2 = 0;
        sprite->y2 = 0;
        sMe.onPad = FALSE;
    }
}

// Every frame (mp_main.c): our pad repeated, friends' pads aging, the bubbles, our sprite on the pad
void MP_Rhythm_Update(void)
{
    u8 i;
    if (sMe.state != PAD_EMPTY && ++sMe.timer >= PAD_RESEND)
    {
        sMe.timer = 0;
        SendPad();
    }
    if (sMe.state == PAD_WAITING && gMpLocalId >= 0)
        MP_Emote_KeepBubbleFor(gMpLocalId, sText_LetsDance);
    if (MP_IsInOverworld())
        PoseMe(sMe.state != PAD_EMPTY && HereNow());
    for (i = 0; i < MP_MAX_PLAYERS; i++)
    {
        if (!sFriends[i].ttl)
            continue;
        if (--sFriends[i].ttl == 0 || !FriendValid(i) || sFriends[i].state == PAD_EMPTY)
        {
            sFriends[i].ttl = 0;
            MP_Emote_ClearBubbleFor(i);
        }
        else if (OnMachineMap(sFriends[i].mapGroup, sFriends[i].mapNum) && HereNow())
        {
            if (sFriends[i].state == PAD_WAITING)
                MP_Emote_KeepBubbleFor(i, sText_LetsDance);
            else
                MP_Emote_KeepBubbleFor(i, sText_Dancing);
        }
    }
}

void MP_Rhythm_OnPlayerLeft(u8 id)
{
    if (id >= MP_MAX_PLAYERS)
        return;
    sFriends[id].ttl = 0;
    MPNet_Log("dance: player %d left the room", id);
    Rhythm_PeerGone(id);
}

// ---------------------------------------------------------------------------
// The machine's side of a credit (include/rhythm.h)

static void BeginCredit(s8 peer, bool8 picker)
{
    struct RhythmBegin b;
    memset(&b, 0, sizeof(b));
    b.side = sMe.side;
    b.peer = peer;
    b.picker = picker;
    MP_GameToAscii(b.myName, gSaveBlock2Ptr->playerName, PLAYER_NAME_LENGTH);
    if (peer >= 0)
        MP_GameToAscii(b.peerName, gMpPlayers[peer].name, MP_NAME_LEN);
    SetPad(peer >= 0 ? PAD_VERSUS : PAD_SOLO);
    sMe.result = RESULT_PLAYED;
    PlaySE(SE_SUCCESS);
    Rhythm_Begin(&b);
}

void MP_Rhythm_Send(int8_t to, const uint8_t *data, uint16_t len)
{
    static u8 p[1 + 4096];
    if (!MP_IsConnected() || to < 0 || len > sizeof(p) - 1)
        return;
    p[0] = MPR_RH_SESSION;
    memcpy(p + 1, data, len);
    MP_SendRelay((u8)to, p, len + 1);
}

void MP_Rhythm_Ended(void)
{
    sMe.arrows = 0;
    SetPad(PAD_EMPTY);
    ScriptContext_Enable();
}

void MP_Rhythm_Dancing(uint8_t arrows)
{
    static u8 sThrottle;
    sMe.arrows = arrows;
    if (sThrottle)
        sThrottle--;
    if (arrows != sMe.arrowsSent && sThrottle == 0)
    {
        sThrottle = 4;   // at most 15 a second
        SendPad();
    }
}

// ---------------------------------------------------------------------------
// The records: the server's (cached), and our own bests in "<save>.dance"

#define CACHE 64
static struct
{
    u32 chart;
    u8 count;
    char names[MP_RH_TOP][MP_RH_NAME];
    u32 scores[MP_RH_TOP];
    u8 grades[MP_RH_TOP];
} sCache[CACHE];
static u8 sCacheNext;

#define BESTS_MAGIC 0x31534E44   // "DNS1"
#define MAX_BESTS 4096
#define OPTION_BYTES 16
struct Best
{
    u32 chart, song, score;
    u8 grade, fullCombo, diff, sent;
    u16 maxCombo, pad;
};
static struct Best *sBests;
static int sBestCount;
static u8 sOptionBytes[OPTION_BYTES];
static bool8 sBestsLoaded;
static char sBestsFor[512];

static void BestsPath(char *out, size_t size)
{
    snprintf(out, size, "%s.dance", gPlatformSavePath);
}

static void LoadBests(void)
{
    char path[600];
    FILE *f;
    u32 magic = 0, n = 0;
    if (sBestsLoaded && !strcmp(sBestsFor, gPlatformSavePath))
        return;
    sBestsLoaded = TRUE;
    snprintf(sBestsFor, sizeof(sBestsFor), "%s", gPlatformSavePath);
    free(sBests);
    sBests = calloc(MAX_BESTS, sizeof(*sBests));
    sBestCount = 0;
    memset(sOptionBytes, 0, sizeof(sOptionBytes));
    BestsPath(path, sizeof(path));
    if (!(f = fopen(path, "rb")))
        return;
    if (fread(&magic, 4, 1, f) == 1 && magic == BESTS_MAGIC && fread(sOptionBytes, OPTION_BYTES, 1, f) == 1
        && fread(&n, 4, 1, f) == 1 && n <= MAX_BESTS)
        sBestCount = (int)fread(sBests, sizeof(*sBests), n, f);
    fclose(f);
}

static void SaveBests(void)
{
#if !MOEMON_DEV_BUILD
    char path[600];
    FILE *f;
    u32 magic = BESTS_MAGIC, n = (u32)sBestCount;
    BestsPath(path, sizeof(path));
    if (!(f = fopen(path, "wb")))
        return;
    fwrite(&magic, 4, 1, f);
    fwrite(sOptionBytes, OPTION_BYTES, 1, f);
    fwrite(&n, 4, 1, f);
    fwrite(sBests, sizeof(*sBests), n, f);
    fclose(f);
#endif
}

static struct Best *BestFor(u32 chart, bool8 make)
{
    int i;
    LoadBests();
    for (i = 0; i < sBestCount; i++)
        if (sBests[i].chart == chart)
            return &sBests[i];
    if (!make || sBestCount >= MAX_BESTS)
        return NULL;
    memset(&sBests[sBestCount], 0, sizeof(sBests[0]));
    sBests[sBestCount].chart = chart;
    return &sBests[sBestCount++];
}

void MP_Rhythm_LoadOptions(uint8_t *opts, int size)
{
    LoadBests();
    memcpy(opts, sOptionBytes, size < OPTION_BYTES ? size : OPTION_BYTES);
}

void MP_Rhythm_SaveOptions(const uint8_t *opts, int size)
{
    LoadBests();
    memcpy(sOptionBytes, opts, size < OPTION_BYTES ? size : OPTION_BYTES);
    SaveBests();
}

static void MyName(char *out)
{
    memset(out, 0, MP_RH_NAME);
    MP_GameToAscii(out, gSaveBlock2Ptr->playerName, PLAYER_NAME_LENGTH);
    out[MP_RH_NAME - 1] = 0;
}

static void SendSubmit(u32 chart, const char *title, int diff, int meter, u32 score, int grade, int maxCombo, int fc)
{
    u8 p[1 + 14 + MP_RH_NAME + 1 + 48];
    int n;
    if (!MP_IsConnected() || gMpLocalId < 0)
        return;
    p[0] = MP_ST_RH_SUBMIT;
    Put32(p + 1, chart);
    p[5] = diff;
    p[6] = meter;
    Put32(p + 7, score);
    p[11] = grade;
    p[12] = fc;
    p[13] = maxCombo;
    p[14] = maxCombo >> 8;
    MyName((char *)p + 15);
    n = title ? strlen(title) : 0;
    if (n > 47)
        n = 47;
    p[15 + MP_RH_NAME] = n;
    memcpy(p + 16 + MP_RH_NAME, title, n);
    MPNet_ClientSend(MPMSG_C_STORE, p, 16 + MP_RH_NAME + n);
}

// Our bests the server hasn't heard of (played offline): sent when we're next at the machine online
static void SendUnsent(void)
{
    int i;
    if (!MP_IsConnected() || gMpLocalId < 0)
        return;
    LoadBests();
    for (i = 0; i < sBestCount; i++)
        if (!sBests[i].sent && sBests[i].score)
        {
            SendSubmit(sBests[i].chart, "", sBests[i].diff, 0, sBests[i].score, sBests[i].grade, sBests[i].maxCombo,
                       sBests[i].fullCombo);
            sBests[i].sent = 1;
        }
    SaveBests();
}

void MP_Rhythm_SubmitScore(uint32_t chart, const char *title, int diff, int meter, uint32_t score, int grade,
                           int maxCombo, int fullCombo, const int *judges)
{
    struct Best *b = BestFor(chart, TRUE);
    (void)judges;
    if (b && score > b->score)
    {
        b->score = score;
        b->grade = grade;
        b->fullCombo = fullCombo;
        b->maxCombo = maxCombo;
        b->diff = diff;
        b->sent = MP_IsConnected() && gMpLocalId >= 0;
        SaveBests();
    }
    SendSubmit(chart, title, diff, meter, score, grade, maxCombo, fullCombo);
    MPNet_Log("dance: %s [%d] %u points, grade %d, full combo %d", title, diff, score, grade, fullCombo);
}

void MP_Rhythm_AskRecords(uint32_t chart)
{
    u8 p[5];
    if (!MP_IsConnected() || gMpLocalId < 0)
        return;
    p[0] = MP_ST_RH_GET;
    Put32(p + 1, chart);
    MPNet_ClientSend(MPMSG_C_STORE, p, sizeof(p));
}

int MP_Rhythm_Records(uint32_t chart, char names[][16], uint32_t *scores, uint8_t *grades, int max)
{
    int i, k;
    for (i = 0; i < CACHE; i++)
        if (sCache[i].chart == chart && sCache[i].chart != 0)
        {
            int n = sCache[i].count < max ? sCache[i].count : max;
            for (k = 0; k < n; k++)
            {
                memcpy(names[k], sCache[i].names[k], MP_RH_NAME);
                names[k][MP_RH_NAME < 16 ? MP_RH_NAME : 15] = 0;
                scores[k] = sCache[i].scores[k];
                grades[k] = sCache[i].grades[k];
            }
            if (n > 0)
                return n;
        }
    // offline (or the server has none): our own best
    {
        struct Best *b = BestFor(chart, FALSE);
        if (b && b->score && max > 0)
        {
            char me[MP_RH_NAME];
            MyName(me);
            memcpy(names[0], me, MP_RH_NAME);
            names[0][MP_RH_NAME < 16 ? MP_RH_NAME : 15] = 0;
            scores[0] = b->score;
            grades[0] = b->grade;
            return 1;
        }
    }
    return 0;
}

// MP_ST_RH_TOP from the server (mp_sharedpc.c passes it on)
void MP_Rhythm_OnStore(const u8 *data, u16 len)
{
    u32 chart;
    int i, n, slot = -1;
    if (len < 6 || data[0] != MP_ST_RH_TOP)
        return;
    chart = Get32(data + 1);
    n = data[5] < MP_RH_TOP ? data[5] : MP_RH_TOP;
    if (len < 6 + n * (MP_RH_NAME + 8))
        return;
    for (i = 0; i < CACHE; i++)
        if (sCache[i].chart == chart)
            slot = i;
    if (slot < 0)
    {
        slot = sCacheNext;
        sCacheNext = (sCacheNext + 1) % CACHE;
    }
    sCache[slot].chart = chart;
    sCache[slot].count = n;
    for (i = 0; i < n; i++)
    {
        const u8 *e = data + 6 + i * (MP_RH_NAME + 8);
        memcpy(sCache[slot].names[i], e, MP_RH_NAME);
        sCache[slot].names[i][MP_RH_NAME - 1] = 0;
        sCache[slot].scores[i] = Get32(e + MP_RH_NAME);
        sCache[slot].grades[i] = e[MP_RH_NAME + 4];
    }
}

// ---------------------------------------------------------------------------
// Achievements (docs/RHYTHM_DECISIONS.md #16)

static bool8 TitleIs(const char *title, const char *name)
{
#ifdef _WIN32
    return _stricmp(title, name) == 0;
#else
    return strcasecmp(title, name) == 0;
#endif
}

// EXTREME's bosses and the hardest of the older mixes: each one cleared on HEAVY or CHALLENGE counts once
static const char *const sBosses[] = {
    "MAX 300", "MAXX UNLIMITED", "The legend of MAX", "PARANOiA survivor MAX", "Kakumei", "DEAD END",
    "PARANOiA survivor", "TRIP MACHINE survivor", "Pink Rose", "V(for EXTREME)",
};

void MP_Rhythm_StageDone(const struct RhythmStage *s)
{
    struct Best *b;
    bool8 firstClear;
    int i;
    if (!s->cleared)
        return;
    LoadBests();
    // a song counts once for SONG HUNTER, whatever its chart (the song's key in a best with chart 0 + song)
    b = BestFor(s->song ^ 0x5A5A0000, FALSE);
    firstClear = b == NULL;
    if (firstClear && (b = BestFor(s->song ^ 0x5A5A0000, TRUE)) != NULL)
    {
        b->song = s->song;
        b->sent = 1;   // (not a chart: never sent)
        SaveBests();
        MP_Achieve_Count("ddrsongs", 1);
    }
    MP_Achieve_Count("ddrclear", 1);
    if (s->diff == 4)
        MP_Achieve_Unlock("ddrchallenge");
    if (s->fullCombo >= 1)
        MP_Achieve_Count("ddrfc", 1);
    if (s->fullCombo >= 2)
        MP_Achieve_Count("ddrpfc", 1);
    if (s->fullCombo >= 3 && s->marvelous)
    {
        MP_Achieve_Unlock("ddrmfc");
        if (s->meter >= 10)
            MP_Achieve_Unlock("ddrmfcten");
    }
    if (s->grade <= 1 && s->meter >= 9)
        MP_Achieve_Unlock("ddraaanine");
    if (s->grade <= 1 && s->meter >= 10)
        MP_Achieve_Unlock("ddraaaten");
    if (s->maxCombo >= 1000)
        MP_Achieve_Unlock("ddrcombo");
    if (s->speed == 5)
        MP_Achieve_Unlock("ddrspeed");
    if (s->appearance == 3 && s->meter >= 9)
        MP_Achieve_Unlock("ddrstealth");
    if (s->versus == 1)
        MP_Achieve_Count("ddrvs", 1);
    if (s->diff >= 3 && TitleIs(s->title, "MAX 300"))
    {
        MP_Achieve_Unlock("ddrmaxthreehundred");
        if (s->grade <= 2)
            MP_Achieve_Unlock("ddrmaxaa");
    }
    if (s->diff >= 3 && TitleIs(s->title, "PARANOiA survivor MAX"))
        MP_Achieve_Unlock("ddrsurvivor");
    for (i = 0; i < (int)ARRAY_COUNT(sBosses); i++)
        if (s->diff >= 3 && TitleIs(s->title, sBosses[i]))
        {
            // once per boss
            struct Best *mark = BestFor(s->song ^ 0xB0550000, FALSE);
            if (!mark && (mark = BestFor(s->song ^ 0xB0550000, TRUE)) != NULL)
            {
                mark->sent = 1;
                SaveBests();
                MP_Achieve_Count("ddrboss", 1);
            }
        }
    if (s->record && MP_IsConnected())
        MP_Achieve_Unlock("ddrrecord");
}

void MP_Rhythm_CreditDone(int stages, int allFullCombo, int allAaa)
{
    MP_Achieve_Count("ddrcredit", 1);
    if (stages >= 4 && allFullCombo)
        MP_Achieve_Unlock("ddrfourfc");
    if (stages >= 4 && allAaa)
        MP_Achieve_Unlock("ddrfouraaa");
}

// ---------------------------------------------------------------------------
// Relay

static void Answer(u8 to, u32 session, u8 yes)
{
    u8 p[6];
    p[0] = MPR_RH_ANSWER;
    Put32(p + 1, session);
    p[5] = yes;
    MP_SendRelay(to, p, sizeof(p));
}

void MP_Rhythm_OnRelay(u8 from, const u8 *data, u16 len)
{
    if (from >= MP_MAX_PLAYERS || len < 1)
        return;
    switch (data[0])
    {
    case MPR_RH_PAD:
        if (len >= 6)
        {
            sFriends[from].side = data[1] & 1;
            sFriends[from].state = data[2];
            sFriends[from].mapGroup = data[3];
            sFriends[from].mapNum = data[4];
            sFriends[from].arrows = data[5];
            sFriends[from].ttl = data[2] == PAD_EMPTY ? 1 : PAD_TTL;
        }
        break;
    case MPR_RH_ASK:
        if (len >= 6)
        {
            u32 session = Get32(data + 1);
            // only while we wait, for the other pad
            if (sMe.state != PAD_WAITING || (data[5] & 1) == sMe.side || Rhythm_IsActive())
            {
                Answer(from, session, 0);
                break;
            }
            Answer(from, session, 1);
            MPNet_Log("dance: player %d joins us", from);
            BeginCredit(from, TRUE);
        }
        break;
    case MPR_RH_ANSWER:
        if (len >= 6 && sMe.asking && from == sMe.peer && Get32(data + 1) == sMe.session)
        {
            sMe.answered = TRUE;
            sMe.answerYes = data[5] != 0;
        }
        break;
    case MPR_RH_SESSION:
        Rhythm_OnNet(from, data + 1, len - 1);
        break;
    }
}

// ---------------------------------------------------------------------------
// Specials (data/scripts/dance_machine.inc)

// Which pad: from where the player stands (x 0 the left, 2 the right, 1 the free one)
static u8 SideHere(void)
{
    struct ObjectEvent *me = &gObjectEvents[gPlayerAvatar.objectEventId];
    s16 x = me->currentCoords.x - MAP_OFFSET;
    if (x <= 0)
        return 0;
    if (x >= 2)
        return 1;
    return FriendOnPad(0) >= 0 ? 1 : 0;
}

// VAR_RESULT: 0 free, 1 a friend waits at the other pad (STR_VAR_1), 2 a friend is on this pad, 3 friends are
// dancing together, 4 a friend is dancing alone
void MP_Rhythm_StepUp(void)
{
    s8 here, other;
    sMe.side = SideHere();
    sMe.result = RESULT_NONE;
    gSpecialVar_Result = 0;
    if (!MP_IsConnected())
        return;
    SendUnsent();
    here = FriendOnPad(sMe.side);
    other = FriendOnPad(sMe.side ^ 1);
    MPNet_Log("dance: stepping up to pad %d (on this pad: %d, the other: %d)", sMe.side, here, other);

    if (here >= 0 && other < 0)
    {
        // they're on "our" pad: the other one is free
        sMe.side ^= 1;
        other = here;
        here = -1;
    }
    if (here >= 0)
    {
        SetNameVar(here);
        gSpecialVar_Result = sFriends[here].state == PAD_VERSUS ? 3 : sFriends[here].state == PAD_SOLO ? 4 : 2;
    }
    else if (other >= 0)
    {
        SetNameVar(other);
        gSpecialVar_Result = sFriends[other].state == PAD_WAITING ? 1 : sFriends[other].state == PAD_SOLO ? 4 : 3;
    }
}

void MP_Rhythm_PlayAlone(void)
{
    BeginCredit(-1, TRUE);
}

#define tTimer data[0]

static void Task_Wait(u8 taskId)
{
    s8 other;
    if (Rhythm_IsActive())
    {
        DestroyTask(taskId);   // a friend joined: the machine is the picture now
        return;
    }
    // two waiting at both pads: the one who joined the room first asks
    other = FriendOnPad(sMe.side ^ 1);
    if (other >= 0 && sFriends[other].state == PAD_WAITING && gMpLocalId >= 0 && gMpLocalId < other
        && ++gTasks[taskId].tTimer >= 30 && !sMe.asking)
    {
        u8 p[6];
        gTasks[taskId].tTimer = 0;
        sMe.peer = other;
        sMe.session = ((u32)Random() << 16) ^ Random();
        sMe.asking = TRUE;
        sMe.answered = FALSE;
        sMe.askTimer = 0;
        p[0] = MPR_RH_ASK;
        Put32(p + 1, sMe.session);
        p[5] = sMe.side;
        MP_SendRelay(other, p, sizeof(p));
    }
    if (sMe.asking)
    {
        if (sMe.answered)
        {
            sMe.asking = FALSE;
            if (sMe.answerYes && FriendValid(sMe.peer))
            {
                DestroyTask(taskId);
                BeginCredit(sMe.peer, FALSE);
                return;
            }
        }
        else if (++sMe.askTimer >= ASK_FRAMES)
            sMe.asking = FALSE;
    }
    if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        sMe.result = RESULT_NONE;
        SetPad(PAD_EMPTY);
        DestroyTask(taskId);
        ScriptContext_Enable();
    }
}

void MP_Rhythm_Wait(void)
{
    sMe.side = SideHere();
    sMe.result = RESULT_NONE;
    sMe.asking = FALSE;
    SetPad(PAD_WAITING);
    CreateTask(Task_Wait, 80);
}

static void Task_Join(u8 taskId)
{
    if (sMe.answered)
    {
        DestroyTask(taskId);
        sMe.asking = FALSE;
        if (sMe.answerYes && FriendValid(sMe.peer))
        {
            BeginCredit(sMe.peer, FALSE);
            return;
        }
        sMe.result = RESULT_REFUSED;
        ScriptContext_Enable();
        return;
    }
    if (++sMe.askTimer >= ASK_FRAMES || !FriendValid(sMe.peer))
    {
        MPNet_Log("dance: no answer from player %d", sMe.peer);
        DestroyTask(taskId);
        sMe.asking = FALSE;
        sMe.result = RESULT_REFUSED;
        ScriptContext_Enable();
    }
}

// Asks the friend waiting at the other pad (MP_Rhythm_StepUp said 1)
void MP_Rhythm_Join(void)
{
    s8 other = FriendOnPad(sMe.side ^ 1);
    u8 p[6];
    sMe.result = RESULT_NONE;
    if (other < 0 || sFriends[other].state != PAD_WAITING)
    {
        sMe.result = RESULT_REFUSED;
        ScriptContext_Enable();
        return;
    }
    sMe.peer = other;
    sMe.session = ((u32)Random() << 16) ^ Random();
    sMe.asking = TRUE;
    sMe.answered = FALSE;
    sMe.askTimer = 0;
    p[0] = MPR_RH_ASK;
    Put32(p + 1, sMe.session);
    p[5] = sMe.side;
    MP_SendRelay(other, p, sizeof(p));
    MPNet_Log("dance: asking player %d", other);
    CreateTask(Task_Join, 80);
}

// VAR_RESULT: 0 nothing to say, 1 played, 2 the friend couldn't (STR_VAR_1)
void MP_Rhythm_Result(void)
{
    gSpecialVar_Result = sMe.result;
    if (sMe.result == RESULT_REFUSED)
        SetNameVar(sMe.peer);
    sMe.result = RESULT_NONE;
    if (!Rhythm_IsActive())
        SetPad(PAD_EMPTY);
}
