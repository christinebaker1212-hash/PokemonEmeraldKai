// Moemon Co-op: game side multiplayer API
#ifndef GUARD_MP_H
#define GUARD_MP_H

#include "global.h"
#include "constants/event_objects.h"

#define MP_MAX_PLAYERS      4
#define MP_NAME_LEN         8
#define MP_LOCALID_BASE     0xE0   // local ids used by remote player object events

// Unused "doll" graphics ids are repurposed for multiplayer avatars
#define OBJ_EVENT_GFX_MP_LOCAL      OBJ_EVENT_GFX_UNUSED_NATU_DOLL      // 76
#define OBJ_EVENT_GFX_MP_REMOTE(id) (OBJ_EVENT_GFX_UNUSED_MAGNEMITE_DOLL + (id)) // 77..80
#define OBJ_EVENT_GFX_MP_FIRST      OBJ_EVENT_GFX_UNUSED_NATU_DOLL
#define OBJ_EVENT_GFX_MP_LAST       (OBJ_EVENT_GFX_UNUSED_MAGNEMITE_DOLL + MP_MAX_PLAYERS - 1)

// Base character sprites
enum {
    MP_BASE_DEFAULT,      // follows the save file's gender
    MP_BASE_BRENDAN,
    MP_BASE_MAY,
    MP_BASE_RS_BRENDAN,
    MP_BASE_RS_MAY,
    MP_BASE_CUSTOM,       // custom PNG sprite sheet
    MP_BASE_COUNT
};

#define MP_SKIN_COUNT   7
#define MP_COLOR_COUNT  11

struct MpAppearance
{
    u8 base;
    u8 skin;     // 0 = sprite default
    u8 outfit;   // 0 = sprite default
    u8 accent;   // 0 = sprite default
    u8 gender;   // save file gender, used for bike/surf sprites
    u8 hasCustom;
    u16 gfxId;      // any overworld sprite (MP_BASE_NPC)
    u8 trainerPic;  // trainer card / battle front pic (0xFF = automatic)
};

// Avatar modes sent over the network
enum {
    MP_AVATAR_FOOT,
    MP_AVATAR_MACH_BIKE,
    MP_AVATAR_ACRO_BIKE,
    MP_AVATAR_SURF,
    MP_AVATAR_UNDERWATER,
};

enum {
    MP_SPEED_WALK,
    MP_SPEED_RUN,
    MP_SPEED_FAST,
    MP_SPEED_FASTER,
};

// State flags
#define MPF_IN_OVERWORLD   (1 << 0)
#define MPF_FREE           (1 << 1)   // controllable: no script/menu running
#define MPF_TRAINER_MODE   (1 << 2)
#define MPF_IN_BATTLE      (1 << 3)
#define MPF_ENGAGED        (1 << 4)   // spotting/being spotted/battling

// Emote kinds (low nibble of the emote byte, high nibble is a counter)
#define MP_EMOTE_NONE         0
#define MP_EMOTE_EXCLAMATION  1

struct MpState
{
    u8 mapGroup;
    u8 mapNum;
    s16 x;          // object event coords (include MAP_OFFSET)
    s16 y;
    u8 facing;
    u8 elevation;
    u8 avatarMode;
    u8 speed;
    u8 flags;
    u8 topLevel;
    u8 partyCount;
    u8 emote;
    u16 seq;
};

#define MP_CUSTOM_FRAMES     18
#define MP_CUSTOM_TILES_SIZE (MP_CUSTOM_FRAMES * 256)
#define MP_CUSTOM_BLOB_SIZE  (MP_CUSTOM_TILES_SIZE + 32)

struct MpPlayer
{
    bool8 present;
    bool8 hasInfo;
    bool8 hasState;
    u8 name[MP_NAME_LEN + 1];
    u32 otId;
    struct MpAppearance app;
    struct MpState state;
    u8 *customGfx;            // custom sprite data or NULL
    u32 customGfxSize;
    u32 appearanceVersion;    // bumped whenever appearance/custom gfx changes
    u8 *blobBuf[1];           // big data being received (MP_BLOB_KINDS)
    u32 blobTotal[1];
    u32 blobHave[1];
};

struct MpConfig
{
    bool8 enabled;
    char lobby[128];          // override of the built-in online lobby address
    char lastRoom[24];
    char lastRoomKey[65];     // hex SHA-256 room key (not the password)
    char lastHostName[24];
    char name[32];
    struct MpAppearance app;
    char customSprite[256];
    u8 trainerRange;
};

extern struct MpConfig gMpConfig;
extern struct MpPlayer gMpPlayers[MP_MAX_PLAYERS];
extern s8 gMpLocalId;   // -1 when not connected

// Lobby session (mp_main.c)
enum {
    MP_LOBBY_OFF,
    MP_LOBBY_CONNECTING,
    MP_LOBBY_READY,           // connected, not in a room
    MP_LOBBY_WAITING,         // create/join sent
    MP_LOBBY_IN_ROOM,
    MP_LOBBY_RECONNECT_WAIT,  // lost the connection, will rejoin the same room
    MP_LOBBY_ERROR,
};

enum {
    MP_LOBBY_ERR_NONE,
    MP_LOBBY_ERR_UNREACHABLE,
    MP_LOBBY_ERR_NAME_TAKEN,
    MP_LOBBY_ERR_NOT_FOUND,
    MP_LOBBY_ERR_BAD_PASSWORD,
    MP_LOBBY_ERR_FULL,
    MP_LOBBY_ERR_VERSION,
    MP_LOBBY_ERR_BAD_NAME,
    MP_LOBBY_ERR_SLOW_DOWN,
    MP_LOBBY_ERR_SERVER_FULL,
};

#define MP_LIST_MAX 24
struct MpLanRoom;
void MP_Lobby_Connect(const char *address, bool8 isLocal);
void MP_Lobby_Disconnect(void);
u8 MP_Lobby_State(void);
u8 MP_Lobby_Error(void);
void MP_Lobby_RequestList(void);
bool8 MP_Lobby_ListReady(void);
int MP_Lobby_GetList(struct MpLanRoom *out, int max);
void MP_Lobby_CreateRoom(const char *room, const char *password, bool8 hidden);
void MP_Lobby_JoinRoom(const char *room, const char *password);
void MP_Lobby_JoinRoomWithKey(const char *room, const u8 *key);
const u8 *MP_Lobby_RoomKey(void);
const char *MP_Lobby_RoomName(void);
bool8 MP_Lobby_IsLocalHost(void);
bool8 MP_Lobby_InRoom(void);
const char *MP_OnlineLobbyAddress(void);
bool8 MP_HasOnlineLobby(void);

// mp_setup.c (title screen HOST / JOIN / SOLO screen)
void CB2_InitCoopSetup(void);
void CB2_InitCoopSetupFromField(void);
void MP_AutoSession_Update(void);

// mp_main.c
void MP_AsciiToGame(u8 *dst, const char *src, int maxLen);
void MP_GameToAscii(char *dst, const u8 *src, int maxLen);
void MP_SendBlob(u8 kind, const void *data, u32 len);
void MP_DispatchRelay(u8 from, const u8 *data, u16 len);
void MP_Init(void);
void MP_FrameUpdate(void);
bool8 MP_IsConnected(void);
const char *MP_StatusText(void);
void MP_SaveConfig(void);
void MP_SendRelay(u8 target, const void *data, u16 len);
void MP_SendAppearance(void);
bool8 MP_IsInOverworld(void);
bool8 MP_IsLocalFree(void);
u8 MP_GetPartyTopLevel(void);
void MP_GetLocalState(struct MpState *state);
void MP_TriggerLocalEmote(u8 kind);
u8 MP_ConnectedCount(void);

// mp_avatar.c
void MP_Avatar_Init(void);
bool8 MP_LocalUsesCustomAvatar(void);
bool8 MP_IsMpGraphicsId(u8 graphicsId);
const struct ObjectEventGraphicsInfo *MP_GetGraphicsInfo(u8 graphicsId);
void MP_Avatar_ApplyLocal(void);     // recolor + refresh the local player sprite
void MP_Avatar_BuildRemote(u8 id);   // rebuild a remote player's gfx info + palette
void MP_Avatar_GetPalette(const struct MpAppearance *app, const u8 *customGfx, u8 avatarMode, u16 *dst);
bool8 MP_Avatar_LoadCustomPng(const char *path, u8 *blob, char *err, int errLen);
const u8 *MP_Avatar_LocalCustomBlob(void);
u32 MP_Avatar_LocalCustomBlobSize(void);
bool8 MP_Avatar_IsValidBlob(const u8 *data, u32 size);
const char *MP_Avatar_BaseName(u8 base);
const char *MP_Avatar_SkinName(u8 skin);
const char *MP_Avatar_ColorName(u8 color);

// mp_overworld.c
void MP_Overworld_Update(void);
void MP_Overworld_OnPlayerLeft(u8 id);
void MP_Overworld_OnAppearanceChanged(u8 id);
void MP_Overworld_OnRemoteState(u8 id, const struct MpState *oldState);
bool8 MP_IsMpObjectEvent(const struct ObjectEvent *objectEvent);
u8 MP_GetRemoteObjectEventId(u8 id);
bool8 MP_GetRemoteMapPos(u8 id, s16 *x, s16 *y); // position in the current map frame
void MP_Overworld_Reset(void);

// mp_trainer.c
void MP_Trainer_Update(void);
void MP_Trainer_OnRelay(u8 from, const u8 *data, u16 len);
bool8 MP_Trainer_IsActive(void);
bool8 MP_Trainer_IsEngaged(void);
void MP_Trainer_Toggle(void);
void MP_Trainer_Begin(void);
bool8 MP_Trainer_CanPlantFlag(void);
const u8 *MP_Trainer_FlagUseMessage(void);
void MP_Trainer_OnPlayerLeft(u8 id);
void ItemUseOutOfBattle_BattleFlag(u8 taskId);

// mp_battle.c (network link layer + PvP flow)
extern bool8 gNetLinkActive;
void MP_Battle_Start(u8 opponentId, bool8 isMaster, u32 opponentMoney);
void MP_Battle_OnRelay(u8 from, const u8 *data, u16 len);
void MP_Battle_OnPlayerLeft(u8 id);
bool8 MP_Battle_IsActive(void);
void MP_Battle_FrameUpdate(void);
void NetLink_Open(void);
void NetLink_Close(void);
u8 NetLink_GetMultiplayerId(void);
u8 NetLink_GetPlayerCount(void);
bool8 NetLink_SendBlock(const void *src, u16 size);
bool8 NetLink_IsTaskFinished(void);
u8 NetLink_GetBlockReceivedStatus(void);
void NetLink_ResetBlockReceivedFlag(u8 who);
void NetLink_SetStandbyCallback(void);
void NetLink_SetCloseCallback(void);
bool8 NetLink_IsMaster(void);

// mp_menu.c
void MP_OpenMenu(void);

// relay message ids (first byte of peer payloads)
enum {
    MPR_SPOT = 1,          // trainer -> target: you have been spotted
    MPR_SPOT_ACK,          // target -> trainer: frozen at x,y (s16,s16)
    MPR_SPOT_DENY,         // target -> trainer: busy
    MPR_ARRIVED,           // trainer -> target: walked up, start the battle (includes money)
    MPR_BATTLE_GO,         // target -> trainer: ready (includes money)
    MPR_CANCEL,            // either side aborts
    MPR_LINK_PLAYER = 20,  // struct LinkPlayer
    MPR_LINK_BLOCK,        // block data
    MPR_LINK_SYNC,         // standby / close barrier
    MPR_BATTLE_DONE,       // battle finished (u8 outcome from sender's view)
    MPR_SPECTATE_FIRST = 40,   // battle spectating (mp_spectate.c)
    MPR_COOP_FIRST = 60,       // co-op double battles (mp_coop.c)
};

#endif // GUARD_MP_H
