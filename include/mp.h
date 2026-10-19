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
    MP_BASE_NPC,          // any overworld sprite from the catalog (MpAppearance.gfxId)
    MP_BASE_COUNT
};

#define MP_PIC_NONE 0xFF

struct MpSpriteEntry
{
    u16 gfxId;
    const char *name;
    u8 frontPic;    // trainer card / battle front pic, MP_PIC_NONE = default
    u8 backPic;     // battle back pic, MP_PIC_NONE = default
    u8 gender;
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
#define MPF_CHALLENGED     (1 << 5)   // an NPC trainer challenged them: others can walk up and join

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
    u16 followerDex;   // national dex number of the follower Pokémon (0 = none)
    u8 followerFlags;  // bit 0: female
};


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
    u32 followerPersonality;  // the party Pokémon that follows you (0 = none)
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
const char *MP_InviteCode(void);   // for a game hosted on this PC, "" if none
const char *MP_VpnName(void);      // "Radmin VPN" / "Hamachi" when the code uses one
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
void MP_Emote_FromStick(u8 dir);
u8 MP_ConnectedCount(void);

// mp_avatar.c
void MP_Avatar_Init(void);
bool8 MP_LocalUsesCustomAvatar(void);
bool8 MP_IsMpGraphicsId(u8 graphicsId);
const struct ObjectEventGraphicsInfo *MP_GetGraphicsInfo(u8 graphicsId);
void MP_Avatar_ApplyLocal(void);     // recolor + refresh the local player sprite
void MP_Avatar_BuildRemote(u8 id);   // rebuild a remote player's gfx info + palette
void MP_Avatar_GetRemotePalette(u8 id, u8 avatarMode, u16 *dst);
const u16 *MP_Avatar_OriginalPalette(u16 tag);
const struct ObjectEventGraphicsInfo *MP_Avatar_LocalStateInfo(u8 graphicsId); // sprite pack bike/surf/...
u16 MP_Avatar_RemoteFrontPic(u8 id);
void MP_Avatar_RefreshPackList(void);
bool8 MP_Avatar_ExportLocal(char *pathOut, int len);
bool8 MP_Avatar_InstallPack(const char *src, char *err, int errLen);
const u8 *MP_Avatar_LocalCustomBlob(void);
u32 MP_Avatar_LocalCustomBlobSize(void);
bool8 MP_Avatar_IsValidBlob(const u8 *data, u32 size);
const char *MP_Avatar_BaseName(u8 base);
// Sprite choices shown in START > CO-OP: the bases, then every catalog sprite
u16 MP_Avatar_ChoiceCount(void);
u16 MP_Avatar_GetChoice(const struct MpAppearance *app);
void MP_Avatar_SetChoice(struct MpAppearance *app, u16 choice);
const char *MP_Avatar_ChoiceName(u16 choice);
int MP_Avatar_ParseSpriteName(const char *name, struct MpAppearance *app); // TRUE when found
void MP_Avatar_SpriteConfigName(const struct MpAppearance *app, char *out, int len);
const struct MpSpriteEntry *MP_Avatar_FindCatalog(u16 gfxId);
// Trainer pics for the trainer card and battles (0xFFFF = game default)
u16 MP_Avatar_LocalFrontPic(void);
u16 MP_Avatar_LocalBackPic(void);
u16 MP_Avatar_RemoteBackPic(u8 id);      // co-op partner
bool8 MP_Avatar_RemoteHidesBack(u8 id);
u16 MP_Avatar_FrontPicFor(const struct MpAppearance *app);
u16 MP_LocalFrontPicOr(u16 defaultPic);
u16 MP_LocalBackPicOr(u16 defaultPic);
bool8 MP_LocalHidesTrainerBack(void);   // picked character has no battle back sprite
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
u16 MP_Battle_OpponentFrontPic(void);   // 0xFFFF = default
void MP_Battle_FrameUpdate(void);
void MP_Battle_OpenLink(u8 peer, bool8 isMaster);  // co-op battles: link layer only
void MP_Battle_CloseLink(void);
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

// mp_spectate.c (watching other players' battles)
bool8 MP_Spectate_IsWatching(void);
bool8 MP_Spectate_IsStreaming(void);
const u16 *MP_Spectate_WatchFrame(u16 keys);
void MP_Spectate_OnRelay(u8 from, const u8 *data, u16 len);
void MP_Spectate_OnPlayerLeft(u8 id);
void MP_Spectate_FrameUpdate(void);
bool8 MP_Spectate_TryInteract(void);

// Follower Pokémon (mp_follower.c)
struct Pokemon;
void MP_Follower_Update(void);
bool8 MP_Follower_TryInteract(void);
bool8 MP_Follower_Toggle(struct Pokemon *mon);
bool8 MP_Follower_IsFollowing(struct Pokemon *mon);
bool8 MP_Follower_HasGraphics(struct Pokemon *mon);
u16 MP_Follower_LocalDex(bool8 *female);
bool8 MP_Follower_LocalTile(s16 *x, s16 *y);

// mp_coop.c (walk up to a challenged player to team up for a Double Battle)
extern bool8 gMpCoopBattle;          // TRUE while a co-op battle runs (on both games)
void MP_Coop_Update(void);
void MP_Coop_OnRelay(u8 from, const u8 *data, u16 len);
void MP_Coop_OnPlayerLeft(u8 id);
bool8 MP_Coop_IsChallenged(void);
bool8 MP_Coop_IsBusy(void);          // asking / being asked / in a co-op battle
void MP_Coop_OnTrainerConfigured(void);
void MP_Coop_OnStartTrainerBattle(void);
void MP_Coop_OnBattleAborted(void);
bool8 MP_Coop_IsMaster(void);
void MP_Coop_ArrangeEnemyParty(void);
u16 MP_Coop_TrainerPic(void);
u16 MP_Coop_PartnerBackPicOr(u16 defaultPic);
bool8 MP_Coop_PartnerHidesBack(void);
const u8 *MP_Coop_LoseText(void);    // NULL = use the game's
void MP_Coop_RecordLearnLater(u8 partyIndex, u16 move);
void MP_Coop_RecordMoney(u32 amount);
bool8 MP_Coop_IsTradedMon(u8 partyIndex);
u16 MP_Coop_VictorySong(void);
void MP_Coop_MirrorExp(u8 battler, u8 monId, s16 gainedExp);
void CB2_MpCoopEndBattle(void);
void MP_Coop_JoinCheck(void);        // special, after the trainer's intro speech
void MP_Coop_PrepareLearnMove(void); // special, post-battle move learning

// mp_notify.c
void MP_ShowNotice(const char *ascii);
void MP_Notice_Update(void);
void MP_OnFileDropped(const char *path);

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
