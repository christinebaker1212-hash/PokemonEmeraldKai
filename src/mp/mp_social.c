// Moemon Co-op: START > CO-OP > SOCIAL.
//   * Chat log: everything said and emoted, and the news pop-ups
//   * Emote combos: two players doing the same emote together get a big one
//   * Scribbles: leave one of your phrases on a signpost for friends to read
//   * Pranks: spend a MoeGatcha pull to play a harmless joke on a friend
//     (they get a free revenge prank)
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "global.h"
#include "mp.h"
#include "easy_chat.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "main.h"
#include "sound.h"
#include "string_util.h"
#include "task.h"
#include "constants/characters.h"
#include "constants/easy_chat.h"
#include "constants/songs.h"
#include "constants/species.h"

void MP_AsciiToGame(u8 *dst, const char *src, int maxLen);
void MP_GameToAscii(char *dst, const u8 *src, int maxLen);
void MP_Gatcha_AddPulls(s32 n);
s32 MP_Gatcha_Pulls(void);
bool8 MP_FacingSign(s16 *x, s16 *y);   // field_control_avatar.c

enum {
    MPR_SOC_PRANK = MPR_SOCIAL_FIRST,   // u8 kind
    MPR_SOC_PRANK_REPLY,                // u8 kind, u8 applied
};

enum { P_MAGIKARP, P_BACKWARDS, P_CONFETTI, P_WHOOPEE, P_COUNT };

static const char *const sPrankNames[P_COUNT] = {
    "MAGIKARP FOLLOWER (1 min)",
    "BACKWARDS CONTROLS (15 sec)",
    "CONFETTI CANNON",
    "WHOOPEE CUSHION",
};

#define LOG_SIZE        40
#define COMBO_FRAMES    90
#define COMBO_RANGE     8
#define PRANK_IMMUNE    45     // seconds a victim is safe after a prank
#define REVENGE_TIME    120    // seconds to get a free revenge prank

struct ChatEntry
{
    char who[12];
    char text[56];
    time_t when;
};

static struct ChatEntry sLog[LOG_SIZE];
static u8 sLogHead, sLogCount;
static u32 sFrame;
static u32 sLastEmoteFrame[MP_MAX_PLAYERS];
static u8 sLastEmoteKind[MP_MAX_PLAYERS];
static time_t sMagikarpUntil, sBackwardsUntil, sImmuneUntil;
static time_t sRevengeUntil[MP_MAX_PLAYERS];
static s16 sSignX = -1, sSignY = -1;
static u8 sSignGroup, sSignNum;

// ---------------------------------------------------------------------------
// Chat log

void MP_Social_Log(const char *who, const char *text)
{
    struct ChatEntry *e = &sLog[(sLogHead + sLogCount) % LOG_SIZE];
    if (sLogCount == LOG_SIZE)
    {
        e = &sLog[sLogHead];
        sLogHead = (sLogHead + 1) % LOG_SIZE;
    }
    else
        sLogCount++;
    snprintf(e->who, sizeof(e->who), "%s", who);
    snprintf(e->text, sizeof(e->text), "%s", text);
    // One line: line breaks become spaces
    {
        char *c;
        for (c = e->text; *c; c++)
            if (*c == '\n')
                *c = ' ';
    }
    e->when = time(NULL);
}

static void PlayerName(u8 id, char *out)
{
    if (id == gMpLocalId)
        MP_GameToAscii(out, gSaveBlock2Ptr->playerName, PLAYER_NAME_LENGTH);
    else if (id < MP_MAX_PLAYERS)
        MP_GameToAscii(out, gMpPlayers[id].name, PLAYER_NAME_LENGTH);
    else
        snprintf(out, 12, "?");
}

void MP_Social_LogSay(u8 id, const u8 *gameText)
{
    char who[16], text[64];
    PlayerName(id, who);
    MP_GameToAscii(text, gameText, 60);
    MP_Social_Log(who, text);
}

void MP_Social_LogToast(const u8 *gameText)
{
    char text[96];
    MP_GameToAscii(text, gameText, 90);
    MP_Social_Log("NEWS", text);
}

// ---------------------------------------------------------------------------
// Emote combos

static bool8 NearEachOther(u8 id)
{
    s16 rx, ry;
    struct ObjectEvent *me = &gObjectEvents[gPlayerAvatar.objectEventId];
    if (id >= MP_MAX_PLAYERS || id == gMpLocalId || !gMpPlayers[id].present || !gMpPlayers[id].hasState)
        return FALSE;
    if (gMpPlayers[id].state.mapGroup != gSaveBlock1Ptr->location.mapGroup
     || gMpPlayers[id].state.mapNum != gSaveBlock1Ptr->location.mapNum)
        return FALSE;
    if (!MP_GetRemoteMapPos(id, &rx, &ry))
        return FALSE;
    return abs(rx - me->currentCoords.x) <= COMBO_RANGE && abs(ry - me->currentCoords.y) <= COMBO_RANGE;
}

static void Combo(u8 other, u8 kind)
{
    static const char *const sTexts[] = {
        [MP_EMOTE_EXCLAMATION] = "DOUBLE SHOCK!!",
        [MP_EMOTE_QUESTION]    = "HUH?! HUH?!",
        [MP_EMOTE_HEART]       = "TRUE FRIENDS!",
        [MP_EMOTE_NOTE]        = "DUET!",
    };
    u8 text[32];
    char who[16], line[64];
    if (kind >= ARRAY_COUNT(sTexts) || !sTexts[kind])
        return;
    MP_AsciiToGame(text, sTexts[kind], 30);
    MP_Emote_ShowBubbleFor(gMpLocalId, text);
    MP_Emote_ShowBubbleFor(other, text);
    PlaySE(kind == MP_EMOTE_HEART ? SE_M_CHARM : kind == MP_EMOTE_NOTE ? SE_M_HEAL_BELL : SE_APPLAUSE);
    PlayerName(other, who);
    snprintf(line, sizeof(line), "Combo with %s: %s", who, sTexts[kind]);
    MP_Social_Log("COMBO", line);
    // Nobody combos twice off the same emotes
    sLastEmoteFrame[other] = 0;
    if (gMpLocalId >= 0)
        sLastEmoteFrame[gMpLocalId] = 0;
}

// An emote, yours or a friend's (kind: MP_EMOTE_*)
void MP_Social_OnEmote(u8 id, u8 kind)
{
    char who[16];
    static const char *const sNames[] = {"", "!", "?", "(heart)", "(music)"};
    u8 i;
    if (id >= MP_MAX_PLAYERS || kind == MP_EMOTE_NONE)
        return;
    PlayerName(id, who);
    if (kind < ARRAY_COUNT(sNames))
        MP_Social_Log(who, sNames[kind]);
    sLastEmoteFrame[id] = sFrame;
    sLastEmoteKind[id] = kind;
    if (gMpLocalId < 0)
        return;
    // Same emote at nearly the same time as someone close by
    if (id == gMpLocalId)
    {
        for (i = 0; i < MP_MAX_PLAYERS; i++)
            if (i != gMpLocalId && sLastEmoteFrame[i] && sLastEmoteKind[i] == kind
             && sFrame - sLastEmoteFrame[i] <= COMBO_FRAMES && NearEachOther(i))
            {
                Combo(i, kind);
                return;
            }
    }
    else if (sLastEmoteFrame[gMpLocalId] && sLastEmoteKind[gMpLocalId] == kind
          && sFrame - sLastEmoteFrame[gMpLocalId] <= COMBO_FRAMES && NearEachOther(id))
    {
        Combo(id, kind);
    }
}

// ---------------------------------------------------------------------------
// Scribbles on signs (kept by the server: 4 words and the writer's name)

static void SignKey(char *out, int size, u8 group, u8 num, s16 x, s16 y, const char *part)
{
    snprintf(out, size, "n:%d.%d.%d.%d:%s", group, num, x, y, part);
}

static void SetCounter(const char *key, s32 value)
{
    s32 old = MP_Counter_Get(key);
    if (old != value)
        MP_Counter_Add(key, value - old);
}

static void RequestMapSigns(void)
{
    char prefix[24];
    snprintf(prefix, sizeof(prefix), "n:%d.%d.", gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum);
    MP_Counter_Request(prefix);
}

static void WriteScribble(s16 x, s16 y, const u16 *words, const u8 *author)
{
    char key[40];
    u8 g = gSaveBlock1Ptr->location.mapGroup, n = gSaveBlock1Ptr->location.mapNum;
    u8 name[8] = {0};
    int i;
    for (i = 0; i < 7 && author && author[i] != EOS; i++)
        name[i] = author[i];
    SignKey(key, sizeof(key), g, n, x, y, "w0");
    SetCounter(key, words ? (s32)(words[0] | ((u32)words[1] << 16)) : 0);
    SignKey(key, sizeof(key), g, n, x, y, "w1");
    SetCounter(key, words ? (s32)(words[2] | ((u32)words[3] << 16)) : 0);
    SignKey(key, sizeof(key), g, n, x, y, "a0");
    SetCounter(key, (s32)(name[0] | (name[1] << 8) | (name[2] << 16) | ((u32)name[3] << 24)));
    SignKey(key, sizeof(key), g, n, x, y, "a1");
    SetCounter(key, (s32)(name[4] | (name[5] << 8) | (name[6] << 16) | ((u32)name[7] << 24)));
}

static bool8 ReadScribble(u8 group, u8 num, s16 x, s16 y, u16 *words, u8 *author)
{
    char key[40];
    u32 w0, w1, a0, a1;
    SignKey(key, sizeof(key), group, num, x, y, "w0");
    w0 = MP_Counter_Get(key);
    SignKey(key, sizeof(key), group, num, x, y, "w1");
    w1 = MP_Counter_Get(key);
    if (w0 == 0 && w1 == 0)
        return FALSE;
    SignKey(key, sizeof(key), group, num, x, y, "a0");
    a0 = MP_Counter_Get(key);
    SignKey(key, sizeof(key), group, num, x, y, "a1");
    a1 = MP_Counter_Get(key);
    words[0] = w0 & 0xFFFF;
    words[1] = w0 >> 16;
    words[2] = w1 & 0xFFFF;
    words[3] = w1 >> 16;
    author[0] = a0; author[1] = a0 >> 8; author[2] = a0 >> 16; author[3] = a0 >> 24;
    author[4] = a1; author[5] = a1 >> 8; author[6] = a1 >> 16; author[7] = EOS;
    return TRUE;
}

// A sign was read (its tile, from field_control_avatar.c)
void MP_Social_OnSignRead(s16 x, s16 y)
{
    sSignX = x;
    sSignY = y;
    sSignGroup = gSaveBlock1Ptr->location.mapGroup;
    sSignNum = gSaveBlock1Ptr->location.mapNum;
}

// The sign's own text is done: show a friend's scribble, if there is one
void MP_Social_AfterSign(void)
{
    u16 words[4];
    u8 author[8], text[96];
    char name[12], line[160], ascii[96];
    if (sSignX < 0 || !MP_IsConnected())
        return;
    if (ReadScribble(sSignGroup, sSignNum, sSignX, sSignY, words, author))
    {
        ConvertEasyChatWordsToString(text, words, 2, 2);
        MP_GameToAscii(name, author, 7);
        MP_GameToAscii(ascii, text, 90);
        snprintf(line, sizeof(line), "Someone scribbled here...\n%s: %s", name, ascii);
        MP_ShowNotice(line);
    }
    sSignX = -1;
}

// ---------------------------------------------------------------------------
// Pranks

bool8 MP_Prank_MagikarpActive(void)
{
    return sMagikarpUntil && time(NULL) < sMagikarpUntil;
}

bool8 MP_Prank_BackwardsActive(void)
{
    return sBackwardsUntil && time(NULL) < sBackwardsUntil && MP_IsInOverworld();
}

static bool8 CanBePranked(void)
{
    return MP_IsInOverworld() && !gMain.inBattle && time(NULL) >= sImmuneUntil;
}

static void ApplyPrank(u8 from, u8 kind)
{
    char who[16], msg[96];
    u8 text[32];
    PlayerName(from, who);
    sImmuneUntil = time(NULL) + PRANK_IMMUNE;
    sRevengeUntil[from] = time(NULL) + REVENGE_TIME;
    switch (kind)
    {
    case P_MAGIKARP:
        sMagikarpUntil = time(NULL) + 60;
        PlaySE(SE_M_SURF);
        snprintf(msg, sizeof(msg), "%s turned your follower\ninto a MAGIKARP!", who);
        break;
    case P_BACKWARDS:
        sBackwardsUntil = time(NULL) + 15;
        PlaySE(SE_M_CONFUSE_RAY);
        snprintf(msg, sizeof(msg), "%s turned your controls\nBACKWARDS!", who);
        break;
    case P_CONFETTI:
        PlaySE(SE_BALLOON_RED);
        MP_AsciiToGame(text, "SURPRISE!!", 20);
        MP_Emote_ShowBubbleFor(gMpLocalId, text);
        snprintf(msg, sizeof(msg), "%s hit you with\nthe CONFETTI CANNON!", who);
        break;
    default:
        PlaySE(SE_M_TOXIC);
        MP_AsciiToGame(text, "...pfffft!", 20);
        MP_Emote_ShowBubbleFor(gMpLocalId, text);
        snprintf(msg, sizeof(msg), "%s put a WHOOPEE\nCUSHION under you!", who);
        break;
    }
    MP_Events_QueueToast(msg);
    snprintf(msg, sizeof(msg), "pranked you: %s (free REVENGE for 2 min!)", sPrankNames[kind]);
    MP_Social_Log(who, msg);
}

void MP_Social_OnRelay(u8 from, const u8 *data, u16 len)
{
    if (from >= MP_MAX_PLAYERS || len < 2)
        return;
    switch (data[0])
    {
    case MPR_SOC_PRANK:
    {
        u8 reply[3] = {MPR_SOC_PRANK_REPLY, data[1], 0};
        if (data[1] < P_COUNT && CanBePranked())
        {
            ApplyPrank(from, data[1]);
            reply[2] = 1;
        }
        MP_SendRelay(from, reply, 3);
        break;
    }
    case MPR_SOC_PRANK_REPLY:
        if (len >= 3)
        {
            char who[16], msg[96];
            PlayerName(from, who);
            if (data[2])
                snprintf(msg, sizeof(msg), "Your prank got %s!", who);
            else
            {
                // Busy or just pranked: no prank, pull refunded
                snprintf(msg, sizeof(msg), "%s dodged it (busy or just\npranked). Pull refunded.", who);
                if (data[1] != 0xFF)
                    MP_Gatcha_AddPulls(1);
            }
            MP_Events_QueueToast(msg);
        }
        break;
    }
}

// ---------------------------------------------------------------------------
// Screens

enum { SC_HUB, SC_LOG, SC_SCRIBBLE, SC_PRANK_WHO, SC_PRANK_WHAT };
enum { HUB_EMOTES, HUB_LOG, HUB_SCRIBBLE, HUB_PRANK, HUB_CLOSE, HUB_COUNT };

static EWRAM_DATA struct
{
    u8 taskId;
    u8 screen;
    u16 cursor, scroll;
    u8 friends[MP_MAX_PLAYERS];
    u8 friendCount;
    u8 target;
    s16 signX, signY;
    char note[80];
} sUi = {0};

static bool8 RevengeOn(u8 id)
{
    return id < MP_MAX_PLAYERS && sRevengeUntil[id] && time(NULL) < sRevengeUntil[id];
}

static void Draw(void)
{
    u16 i, first, count;
    char line[80], who[16];
    switch (sUi.screen)
    {
    case SC_HUB:
    {
        static const char *const sRows[] = {"EMOTES & PHRASES...", "CHAT LOG...", "SCRIBBLE ON A SIGN...", "PRANK A FRIEND...", "CLOSE"};
        MpUi_Begin("SOCIAL");
        for (i = 0; i < HUB_COUNT; i++)
            MpUi_RowAscii(i, i == sUi.cursor, sRows[i], NULL);
        MpUi_End();
        MpUi_HelpAscii(sUi.note[0] ? sUi.note : "A: SELECT   B: CLOSE");
        break;
    }
    case SC_LOG:
        MpUi_Begin("CHAT LOG");
        count = sLogCount ? sLogCount : 1;
        first = MpUi_ScrollFor(sUi.cursor, count, &sUi.scroll);
        if (sLogCount == 0)
            MpUi_TextAscii(1, "Nothing said yet. Say hi!");
        for (i = first; i < sLogCount && i < first + MPUI_ROWS; i++)
        {
            // Newest first
            struct ChatEntry *e = &sLog[(sLogHead + sLogCount - 1 - i) % LOG_SIZE];
            long ago = (long)(time(NULL) - e->when);
            char when[12];
            if (ago < 60)
                snprintf(when, sizeof(when), "now");
            else if (ago < 3600)
                snprintf(when, sizeof(when), "%ldm", ago / 60);
            else
                snprintf(when, sizeof(when), "%ldh", ago / 3600);
            snprintf(line, sizeof(line), "%s: %s", e->who, e->text);
            MpUi_RowAscii(i - first, i == sUi.cursor, line, when);
        }
        MpUi_End();
        MpUi_HelpAscii("UP/DOWN: SCROLL   B: BACK");
        break;
    case SC_SCRIBBLE:
        MpUi_Begin("SCRIBBLE WHAT?");
        for (i = 0; i < MP_NUM_PHRASES; i++)
        {
            u8 text[96], left[100];
            if (MP_Emote_PhraseIsEmpty(i))
            {
                snprintf(line, sizeof(line), "%d (empty phrase)", i + 1);
                MpUi_RowAscii(i, i == sUi.cursor, line, NULL);
                continue;
            }
            MP_Emote_PhraseText(i, text);
            for (count = 0; text[count] != EOS; count++)
                if (text[count] == CHAR_NEWLINE)
                    text[count] = CHAR_SPACE;
            left[0] = CHAR_1 + i;
            left[1] = CHAR_SPACE;
            StringCopy(left + 2, text);
            MpUi_Row(i, i == sUi.cursor, left, NULL);
        }
        MpUi_RowAscii(MP_NUM_PHRASES, sUi.cursor == MP_NUM_PHRASES, "ERASE THE SCRIBBLE", NULL);
        MpUi_RowAscii(MP_NUM_PHRASES + 1, sUi.cursor == MP_NUM_PHRASES + 1, "BACK", NULL);
        MpUi_End();
        MpUi_HelpAscii(sUi.note[0] ? sUi.note : "Friends see it when they read this sign.");
        break;
    case SC_PRANK_WHO:
        MpUi_Begin("PRANK WHO?");
        snprintf(line, sizeof(line), "PULLS: %d", (int)MP_Gatcha_Pulls());
        {
            u8 r[24];
            MP_AsciiToGame(r, line, 20);
            MpUi_TitleRight(r);
        }
        for (i = 0; i < sUi.friendCount; i++)
        {
            PlayerName(sUi.friends[i], who);
            MpUi_RowAscii(i, i == sUi.cursor, who, RevengeOn(sUi.friends[i]) ? "REVENGE: free!" : "1 pull");
        }
        MpUi_RowAscii(sUi.friendCount, sUi.cursor == sUi.friendCount, "BACK", NULL);
        if (sUi.friendCount == 0)
            MpUi_TextAscii(2, "Nobody else is online right now.");
        MpUi_End();
        MpUi_HelpAscii(sUi.note[0] ? sUi.note : "Harmless jokes. Each costs a MOEGATCHA pull.");
        break;
    case SC_PRANK_WHAT:
        PlayerName(sUi.target, who);
        snprintf(line, sizeof(line), "PRANK %s WITH...", who);
        MpUi_Begin(line);
        for (i = 0; i < P_COUNT; i++)
            MpUi_RowAscii(i, i == sUi.cursor, sPrankNames[i], NULL);
        MpUi_RowAscii(P_COUNT, sUi.cursor == P_COUNT, "BACK", NULL);
        MpUi_End();
        MpUi_HelpAscii(RevengeOn(sUi.target) ? "This one's free: REVENGE!" : "Uses 1 MOEGATCHA pull.");
        break;
    }
}

static void Close(u8 taskId)
{
    MpUi_Close();
    DestroyTask(taskId);
}

static u16 CountFor(void)
{
    switch (sUi.screen)
    {
    case SC_HUB:        return HUB_COUNT;
    case SC_LOG:        return sLogCount ? sLogCount : 1;
    case SC_SCRIBBLE:   return MP_NUM_PHRASES + 2;
    case SC_PRANK_WHO:  return sUi.friendCount + 1;
    default:            return P_COUNT + 1;
    }
}

static void Go(u8 screen)
{
    sUi.screen = screen;
    sUi.cursor = 0;
    sUi.scroll = 0;
    sUi.note[0] = 0;
}

static void Task_Social(u8 taskId)
{
    if (MpUi_MoveCursor(&sUi.cursor, CountFor()))
    {
        sUi.note[0] = 0;
        Draw();
        return;
    }
    if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        if (sUi.screen == SC_HUB)
        {
            Close(taskId);
            return;
        }
        Go(sUi.screen == SC_PRANK_WHAT ? SC_PRANK_WHO : SC_HUB);
        Draw();
        return;
    }
    if (!JOY_NEW(A_BUTTON))
        return;
    PlaySE(SE_SELECT);
    switch (sUi.screen)
    {
    case SC_HUB:
        switch (sUi.cursor)
        {
        case HUB_EMOTES:
            Close(taskId);
            MP_OpenMenuPageAt(MP_MENU_PAGE_EMOTES, 0);
            return;
        case HUB_LOG:
            Go(SC_LOG);
            break;
        case HUB_SCRIBBLE:
            if (!MP_IsConnected())
                snprintf(sUi.note, sizeof(sUi.note), "Go online first: scribbles are kept by the server.");
            else if (!MP_FacingSign(&sUi.signX, &sUi.signY))
                snprintf(sUi.note, sizeof(sUi.note), "Stand in front of a sign and face it first.");
            else
            {
                RequestMapSigns();
                Go(SC_SCRIBBLE);
            }
            break;
        case HUB_PRANK:
            if (!MP_IsConnected())
                snprintf(sUi.note, sizeof(sUi.note), "Go online to prank your friends.");
            else
            {
                sUi.friendCount = MP_Friends_List(sUi.friends);
                Go(SC_PRANK_WHO);
            }
            break;
        default:
            Close(taskId);
            return;
        }
        break;
    case SC_LOG:
        Go(SC_HUB);
        sUi.cursor = HUB_LOG;
        break;
    case SC_SCRIBBLE:
        if (sUi.cursor < MP_NUM_PHRASES)
        {
            u16 words[MP_PHRASE_WORDS];
            u8 k;
            if (MP_Emote_PhraseIsEmpty(sUi.cursor))
            {
                PlaySE(SE_FAILURE);
                snprintf(sUi.note, sizeof(sUi.note), "Make that phrase first (EMOTES & PHRASES).");
                break;
            }
            for (k = 0; k < MP_PHRASE_WORDS; k++)
                words[k] = gMpConfig.phrases[sUi.cursor][k];
            WriteScribble(sUi.signX, sUi.signY, words, gSaveBlock2Ptr->playerName);
            PlaySE(SE_M_SKETCH);
            Close(taskId);
            MP_ShowNotice("You scribbled on the sign. Friends will see it when they read it.");
            return;
        }
        if (sUi.cursor == MP_NUM_PHRASES)
        {
            WriteScribble(sUi.signX, sUi.signY, NULL, NULL);
            Close(taskId);
            MP_ShowNotice("You wiped the sign clean.");
            return;
        }
        Go(SC_HUB);
        sUi.cursor = HUB_SCRIBBLE;
        break;
    case SC_PRANK_WHO:
        if (sUi.cursor >= sUi.friendCount)
        {
            Go(SC_HUB);
            sUi.cursor = HUB_PRANK;
            break;
        }
        sUi.target = sUi.friends[sUi.cursor];
        Go(SC_PRANK_WHAT);
        break;
    case SC_PRANK_WHAT:
        if (sUi.cursor >= P_COUNT)
        {
            Go(SC_PRANK_WHO);
            break;
        }
        if (!gMpPlayers[sUi.target].present)
        {
            Go(SC_PRANK_WHO);
            snprintf(sUi.note, sizeof(sUi.note), "They left.");
            break;
        }
        {
            bool8 free = RevengeOn(sUi.target);
            u8 buf[2] = {MPR_SOC_PRANK, sUi.cursor};
            if (!free && MP_Gatcha_Pulls() < 1)
            {
                PlaySE(SE_FAILURE);
                snprintf(sUi.note, sizeof(sUi.note), "No MOEGATCHA pulls left!");
                Go(SC_PRANK_WHO);
                break;
            }
            if (free)
                sRevengeUntil[sUi.target] = 0;
            else
                MP_Gatcha_AddPulls(-1);
            MP_SendRelay(sUi.target, buf, 2);
            PlaySE(SE_M_SWAGGER);
            Close(taskId);
            return;
        }
    }
    Draw();
}

void MP_Social_Open(void)
{
    memset(&sUi, 0, sizeof(sUi));
    MpUi_Open();
    sUi.taskId = CreateTask(Task_Social, 80);
    Draw();
}

// Every frame
void MP_Social_Update(void)
{
    static u8 sLastGroup = 0xFF, sLastNum = 0xFF;
    sFrame++;
    if (sFrame == 0)
        sFrame = 1;
    // New map: fetch its signs' scribbles
    if (MP_IsConnected() && MP_IsInOverworld()
     && (gSaveBlock1Ptr->location.mapGroup != sLastGroup || gSaveBlock1Ptr->location.mapNum != sLastNum))
    {
        sLastGroup = gSaveBlock1Ptr->location.mapGroup;
        sLastNum = gSaveBlock1Ptr->location.mapNum;
        RequestMapSigns();
    }
}
