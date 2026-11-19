// Moemon Kai: CHATOT's CHATTER as it was in Platinum (the user, 2026-11-19: "Chatter will function as it did in
// Platinum, playing back exactly the sound that was recorded, and all players can hear what it sounds like online";
// docs/COOP_NOTES.md "CHATTER as it was in Platinum"). From pret/pokeplatinum's chatot_cry.c and sound_chatot.c:
// - the saying: one second heard by the microphone (src/platform/mic.c: 2000 signed 8-bit samples), kept as Platinum
//   keeps it, 1000 bytes of 4-bit samples (ChatotCry_StoreAudio), one for the save ("<save>.chatter"); CHATTER in the
//   party menu records a new one (data/scripts/chatter.inc: Platinum's own words);
// - every CHATOT cry of the normal kinds (sound_chatot.c Sound_CanPlayChatotCry: normal, short, fainting, unwell) is
//   the saying instead, played at 2000 samples a second a random bit faster each time (CHATOT_CRY_SPEED_VARIANCE);
//   outside battle the player's own; in battle the trainer's whose CHATOT it is (BattleSystem_GetChatotCry): ours,
//   a co-op partner's or a versus opponent's (theirs reach us online: MP_BLOB_CHATTER, kept by the room's server for
//   whoever joins later), never a wild one's or an NPC trainer's (Chatot's own cry);
// - CHATTER confuses by the saying (BtlCmd_CheckChatterActivation, Sound_GetChatterActivationParameter): its byte 15
//   below -30 a 11% chance, 30 and up 31%, else (or no saying) 1%.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "battle.h"
#include "battle_util.h"
#include "battle_anim.h"
#include "main.h"
#include "pokemon.h"
#include "aw_file.h"
#include "mic.h"
#include "mp.h"
#include "constants/battle.h"
#include "constants/moves.h"
#include "constants/sound.h"
#include "constants/species.h"

extern char gPlatformSavePath[512];
void MP_SendBlob(u8 kind, const void *data, u32 len);

#define SAYING_BYTES  1000                 // Platinum's CHATOT_CRY_SIZE: two 4-bit samples a byte
#define FILE_MAGIC    0x54414843           // "CHAT"
#define FILE_VERSION  1
#define OUT_RATE      42048                // the game's mix (music_player.c), as the STADIUM announcer's
#define BLOB_CHATTER  1                    // (src/mp/mp_net.h MP_BLOB_CHATTER)

struct Saying
{
    bool8 valid;
    s8 data[SAYING_BYTES];
};

static struct Saying sOwn;                         // the save's (Platinum: SaveData_GetChatotCry)
static struct Saying sFriend[MP_MAX_PLAYERS];      // friends' in the room
static char sLoadedFor[600];
static u32 sLoadedOwner;
static s16 sNextBattler = -1;                      // the battler whose cry comes next (MP_Chatter_ForBattler)

// ---------------------------------------------------------------------------------------------------------------
// Platinum's format (chatot_cry.c)

// ChatotCry_StoreAudio: 2000 samples of 8 bits into 1000 bytes of two 4-bit samples (the first in the low half)
static void Store(struct Saying *s, const s8 *in)
{
    int i;
    s->valid = TRUE;
    for (i = 0; i < SAYING_BYTES; i++)
    {
        u8 lo = (u8)((s8)(in[i * 2] / 16) + 8), hi = (u8)((s8)(in[i * 2 + 1] / 16) + 8);
        s->data[i] = (s8)((lo & 0xF) | (hi << 4));
    }
}

// ChatotCry_GetUpsampledAudio: back to 2000 samples of 8 bits
static void Unpack(const struct Saying *s, s8 *out)
{
    int i;
    for (i = 0; i < SAYING_BYTES; i++)
    {
        out[i * 2] = (s8)(((s->data[i] & 0xF) - 8) * 16);
        out[i * 2 + 1] = (s8)((((u8)s->data[i] >> 4) - 8) * 16);
    }
}

// ---------------------------------------------------------------------------------------------------------------
// "<save>.chatter"

static u32 Owner(void)
{
    const u8 *id = gSaveBlock2Ptr->playerTrainerId;
    return id[0] | (id[1] << 8) | (id[2] << 16) | ((u32)id[3] << 24);
}

static void Path(char *out, size_t size)
{
    snprintf(out, size, "%s.chatter", gPlatformSavePath);
}

// The save's saying, read when the save (or its player) changed
static void EnsureLoaded(void)
{
    char path[600];
    FILE *f;
    u32 head[3];
    Path(path, sizeof(path));
    if (strcmp(path, sLoadedFor) == 0 && sLoadedOwner == Owner())
        return;
    snprintf(sLoadedFor, sizeof(sLoadedFor), "%s", path);
    sLoadedOwner = Owner();
    memset(&sOwn, 0, sizeof(sOwn));
    f = fopen(path, "rb");
    if (f == NULL)
        return;
    if (fread(head, 4, 3, f) == 3 && head[0] == FILE_MAGIC && head[1] == FILE_VERSION && head[2] == sLoadedOwner
     && fread(sOwn.data, 1, SAYING_BYTES, f) == SAYING_BYTES)
        sOwn.valid = TRUE;
    fclose(f);
}

static void SaveOwn(void)
{
    char path[600];
    u32 head[3] = {FILE_MAGIC, FILE_VERSION, Owner()};
    AwFile *f;
    Path(path, sizeof(path));
    f = AwFile_Open(path);
    if (f == NULL)
        return;
    AwFile_Write(head, 4, 3, f);
    AwFile_Write(sOwn.data, 1, SAYING_BYTES, f);
    AwFile_Close(f);
}

bool8 MP_Chatter_HasSaying(void)
{
    EnsureLoaded();
    return sOwn.valid;
}

// Friends hear our saying: sent to the room (and kept by its server for whoever joins later)
void MP_Chatter_SendOwn(void)
{
    u8 b[1 + SAYING_BYTES];
    EnsureLoaded();
    if (!sOwn.valid || !MP_IsConnected())
        return;
    b[0] = 1;
    memcpy(b + 1, sOwn.data, SAYING_BYTES);
    MP_SendBlob(BLOB_CHATTER, b, sizeof(b));
}

// A new saying, heard by the microphone (MIC_RECORD_SAMPLES samples): kept, saved and sent to friends
void MP_Chatter_SetSaying(const s8 *samples)
{
    EnsureLoaded();
    Store(&sOwn, samples);
    SaveOwn();
    MPNet_Log("chatter: a new saying (byte 15 = %d)", sOwn.data[15]);
    MP_Chatter_SendOwn();
}

void MP_Chatter_OnBlob(u8 id, const u8 *data, u32 len)
{
    if (id >= MP_MAX_PLAYERS || len < 1 + SAYING_BYTES)
        return;
    sFriend[id].valid = data[0] != 0;
    memcpy(sFriend[id].data, data + 1, SAYING_BYTES);
    MPNet_Log("chatter: player %d's saying heard", id + 1);
}

void MP_Chatter_OnPlayerLeft(u8 id)
{
    if (id < MP_MAX_PLAYERS)
        sFriend[id].valid = FALSE;
}

// ---------------------------------------------------------------------------------------------------------------
// Whose saying a CHATOT has

void MP_Chatter_ForBattler(u8 battler)
{
    sNextBattler = battler;
}

// The trainer of a battler: ours, a co-op partner's, a versus opponent's; NULL: a wild one's or an NPC trainer's
static const struct Saying *SayingOfBattler(u8 b)
{
    s8 p;
    if (b >= gBattlersCount)
        return NULL;
    if (GetBattlerSide(b) == B_SIDE_PLAYER)
    {
        if (gMpCoopBattle && (p = MP_Coop_BattlerPlayer(b)) >= 0 && p != gMpLocalId)
            return p < MP_MAX_PLAYERS ? &sFriend[p] : NULL;
        if (!gMpCoopBattle && (gBattleTypeFlags & BATTLE_TYPE_INGAME_PARTNER)
         && GetBattlerPosition(b) == B_POSITION_PLAYER_RIGHT)
            return NULL;   // (STEVEN and the like: an NPC's CHATOT)
        EnsureLoaded();
        return &sOwn;
    }
    if (!gMpCoopBattle && (p = MP_Battle_PeerId()) >= 0 && p < MP_MAX_PLAYERS)
        return &sFriend[p];
    return NULL;
}

static const struct Saying *SayingForCry(u8 *whoBattler)
{
    s16 b = sNextBattler;
    u8 i;
    sNextBattler = -1;
    *whoBattler = 0xFF;
    if (!gMain.inBattle)
    {
        EnsureLoaded();
        return &sOwn;
    }
    if (b < 0)
    {
        // (a cry with no battler named: the CHATOT out in the battle)
        for (i = 0; i < gBattlersCount && b < 0; i++)
            if (gBattleMons[i].species == SPECIES_CHATOT)
                b = i;
        if (b < 0)
        {
            EnsureLoaded();
            return &sOwn;   // (a summary or the party opened in battle: as outside it)
        }
    }
    *whoBattler = (u8)b;
    return SayingOfBattler((u8)b);
}

// ---------------------------------------------------------------------------------------------------------------
// Playing a saying (mixed after the game's own sound: music_player.c, as the STADIUM announcer)

static struct
{
    bool8 on;
    s8 wave[MIC_RECORD_SAMPLES];
    u64 pos, step;                 // 16.16 in the saying's samples
    float left, right;
    u32 seed;
} sV = {.seed = 0x51C4A7};

// Platinum's own little random numbers (LCRNG_Next): never the game's, which a link battle shares
static u32 NextRand(void)
{
    sV.seed = sV.seed * 1103515245 + 24691;
    return sV.seed >> 16;
}

static bool8 PlaySaying(const struct Saying *s, s8 pan, s8 volume, const char *why)
{
    u32 speed;
    float vol, p;
    if (s == NULL || !s->valid)
        return FALSE;
    Unpack(s, sV.wave);
    speed = 32768 + NextRand() % 8192;   // WAVE_OUT_SPEED(1.0) + up to CHATOT_CRY_SPEED_VARIANCE
    sV.step = ((u64)MIC_RECORD_RATE * speed / 32768 << 16) / OUT_RATE;
    sV.pos = 0;
    vol = (volume > 0 ? volume : 1) / 127.0f;
    p = pan / 64.0f;   // (-64 left .. 63 right)
    sV.left = vol * (p > 0 ? 1.0f - p : 1.0f);
    sV.right = vol * (p < 0 ? 1.0f + p : 1.0f);
    sV.on = TRUE;
    MPNet_Log("chatter: CHATOT's saying (%s, speed %u.%03u)", why, speed / 32768, (speed % 32768) * 1000 / 32768);
    return TRUE;
}

// sound.c PlayCryInternal: a CHATOT cry of a normal kind is its saying when it has one (TRUE: nothing else to play)
bool8 MP_Chatter_TryCry(u8 mode, s8 pan, s8 volume)
{
    const struct Saying *s;
    u8 battler;
    char why[48];
    switch (mode)
    {
    case CRY_MODE_NORMAL: case CRY_MODE_DOUBLES: case CRY_MODE_ENCOUNTER: case CRY_MODE_FAINT:
    case CRY_MODE_WEAK: case CRY_MODE_WEAK_DOUBLES:
        break;
    default:
        sNextBattler = -1;
        return FALSE;   // (HOWL, HYPER VOICE, ROAR, GROWL: Chatot's own cry, as Platinum)
    }
    char whose[16];
    s = SayingForCry(&battler);
    if (s == &sOwn)
        snprintf(whose, sizeof(whose), "ours");
    else if (s != NULL)
        snprintf(whose, sizeof(whose), "player %d's", (int)(s - sFriend) + 1);
    else
        snprintf(whose, sizeof(whose), "none");
    if (battler == 0xFF)
        snprintf(why, sizeof(why), "%s, mode %d", whose, mode);
    else
        snprintf(why, sizeof(why), "%s, battler %d, mode %d", whose, battler, mode);
    return PlaySaying(s, pan, volume, why);
}

bool8 MP_Chatter_Playing(void)
{
    return sV.on;
}

void MP_Chatter_Stop(void)
{
    sV.on = FALSE;
}

void MP_Chatter_Mix(float *buffer, s32 count)
{
    s32 n;
    if (!sV.on)
        return;
    for (n = 0; n + 1 < count; n += 2)
    {
        u32 i = (u32)(sV.pos >> 16);
        float a, b, f, v;
        if (i + 1 >= MIC_RECORD_SAMPLES)
        {
            sV.on = FALSE;
            break;
        }
        f = (sV.pos & 0xFFFF) / 65536.0f;
        a = sV.wave[i], b = sV.wave[i + 1];
        v = (a + (b - a) * f) / 128.0f;
        buffer[n] += v * sV.left;
        buffer[n + 1] += v * sV.right;
        sV.pos += sV.step;
    }
}

// ---------------------------------------------------------------------------------------------------------------
// In battle: CHATTER's chance to confuse (battle_script_commands.c Cmd_seteffectwithchance)

// Sound_GetChatterActivationParameter: 0, 1 or 2 by the saying's byte 15
static u8 ChatterLevel(const struct Saying *s)
{
    s8 v;
    if (s == NULL || !s->valid)
        return 0;
    v = s->data[15];
    if (v < -30)
        return 1;
    if (v >= 30)
        return 2;
    return 0;
}

// BtlCmd_CheckChatterActivation: TRUE to confuse. A CHATOT that isn't transformed, a target still standing; the
// chance 0 / 10 / 30 and "random % 100 > chance" skips it (so 1%, 11%, 31%)
bool8 MP_Chatter_Confuses(u8 attacker, u8 target, u16 random)
{
    static const u8 sChance[3] = {0, 10, 30};
    u8 level;
    if (gBattleMons[attacker].species != SPECIES_CHATOT || gBattleMons[target].hp == 0
     || (gBattleMons[attacker].status2 & STATUS2_TRANSFORMED))
        return FALSE;
    level = ChatterLevel(SayingOfBattler(attacker));
    MPNet_Log("chatter: CHATTER's level %d (%d%%)", level, sChance[level] + 1);
    return random % 100 <= sChance[level];
}

// Tests (mp chatter): what the saying is
void MP_Chatter_Debug(void)
{
    u8 i;
    EnsureLoaded();
    printf("[test] chatter: own %s, byte 15 = %d, level %d\n", sOwn.valid ? "yes" : "none", sOwn.data[15], ChatterLevel(&sOwn));
    for (i = 0; i < MP_MAX_PLAYERS; i++)
        if (sFriend[i].valid)
            printf("[test] chatter: player %d's saying, byte 15 = %d\n", i + 1, sFriend[i].data[15]);
}

// ---------------------------------------------------------------------------------------------------------------
// CHATTER in the party menu: Platinum's RecordChatotCry_Chatter (data/scripts/chatter.inc)
#include "event_data.h"
#include "field_effect.h"
#include "m4a.h"
#include "party_menu.h"
#include "script.h"
#include "task.h"

extern const u8 EventScript_RecordChatotCry[];

static void FieldCallback_Chatter(void)
{
    gSpecialVar_0x8004 = GetCursorSelectionMonId();
    ScriptContext_SetupScript(EventScript_RecordChatotCry);
}

// party_menu.c's field moves: CHATTER can always be used (Platinum's party menu, ACTION_CHATTER)
bool8 SetUpFieldMove_Chatter(void)
{
    gFieldCallback2 = FieldCallback_PrepareFadeInFromMenu;
    gPostMenuFieldCallback = FieldCallback_Chatter;
    return TRUE;
}

// (special) the CHATOT's species for its picture (0x8005), whether a saying would be forgotten (VAR_RESULT)
void MP_ChatterField_Prepare(void)
{
    gSpecialVar_0x8005 = GetMonData(&gPlayerParty[gSpecialVar_0x8004 < PARTY_SIZE ? gSpecialVar_0x8004 : 0], MON_DATA_SPECIES);
    gSpecialVar_Result = MP_Chatter_HasSaying();
}

// (specialvar) the microphone starts listening, the music lowered as Platinum's FadeOutBGM 42 (FALSE: no microphone)
u16 MP_ChatterField_Start(void)
{
    if (!Mic_StartRecording())
        return FALSE;
    m4aMPlayVolumeControl(&gMPlayInfo_BGM, TRACKS_ALL, 85);
    return TRUE;
}

static void Task_ChatterRecord(u8 taskId)
{
    if (Mic_RecordingDone())
    {
        s8 samples[MIC_RECORD_SAMPLES];
        Mic_TakeRecording(samples);
        MP_Chatter_SetSaying(samples);
        gSpecialVar_Result = TRUE;
    }
    else if (!Mic_RecordingStalled())
        return;
    else
    {
        // (the microphone stopped sending: no saying)
        Mic_StopRecording();
        MPNet_Log("chatter: the microphone went quiet, no saying");
        gSpecialVar_Result = FALSE;
    }
    m4aMPlayVolumeControl(&gMPlayInfo_BGM, TRACKS_ALL, 256);
    DestroyTask(taskId);
    ScriptContext_Enable();
}

// (special + waitstate) the second heard, kept as the saying (VAR_RESULT: TRUE learned)
void MP_ChatterField_Wait(void)
{
    CreateTask(Task_ChatterRecord, 80);
}

// Tests (mp chatter record): the microphone's next second becomes the saying, no script
static void Task_DebugRecord(u8 taskId)
{
    if (!Mic_RecordingDone())
        return;
    {
        s8 samples[MIC_RECORD_SAMPLES];
        Mic_TakeRecording(samples);
        MP_Chatter_SetSaying(samples);
    }
    DestroyTask(taskId);
}

void MP_Chatter_DebugRecord(void)
{
    if (Mic_StartRecording())
        CreateTask(Task_DebugRecord, 80);
}
