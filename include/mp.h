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
    MP_BASE_CREATED,      // made in LOOK > CREATE (MpAppearance.created / recipe)
    MP_BASE_COUNT
};

#define MP_PIC_NONE 0xFFFF

struct MpSpriteEntry
{
    u16 gfxId;      // OBJ_EVENT_GFX_*, or MP_GFX_EXT_FIRST + n for FireRed/LeafGreen sprites
    const char *name;
    u16 frontPic;   // trainer card / battle front pic choice (see MP_Avatar_FrontChoiceCount), MP_PIC_NONE = default
    u16 backPic;    // battle back pic choice, MP_PIC_NONE = default
    u8 gender;
};

#define MP_GFX_EXT_FIRST 0x8000   // overworld sprites that only exist in the CO-OP catalog

#define MP_SKIN_COUNT   7
#define MP_COLOR_COUNT  11
#define MP_HAIR_COUNT   15

// A trainer made in LOOK > CREATE (parts of the trainer customization kit)
#define MP_CREATED_SLOTS 3
struct MpCreated
{
    u8 made;        // 0 = this slot was never saved
    u8 girl;        // body: 0 boy, 1 girl
    u8 skin;        // 0-3
    u8 hair, hairColor;
    u8 top, topColor;
    u8 bottom, bottomColor;
    u8 hat, hatColor;   // hat 0 = none, else style + 1
    u8 bag, bagColor;   // bag 0 = none, else style + 1
    u8 fieldHair;       // field sprite hairstyle: 0 = AUTO (matches HAIR), else 1 + a Gen 3 kit hairstyle
};
#define MP_CREATED_BYTES 14   // 13 before FIELD HAIR (still read)

struct MpAppearance
{
    u8 base;
    u8 skin;     // 0 = sprite default
    u8 outfit;   // 0 = sprite default
    u8 accent;   // 0 = sprite default
    u8 gender;   // save file gender, used for bike/surf sprites
    u8 hasCustom;
    u16 gfxId;      // any overworld sprite (MP_BASE_NPC)
    u16 frontSel;   // trainer card / battle front pic: 0 = matches the sprite, else 1 + front choice
    u8 backSel;     // battle back pic: 0 = matches the sprite, else 1 + back choice
    u8 hair;        // 0 = sprite default
    u8 created;     // MP_BASE_CREATED: which CUSTOM slot (1-3), for the name
    struct MpCreated recipe;   // MP_BASE_CREATED: the parts (everyone builds the look from them)
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
#define MPF_RIDING         (1 << 6)   // riding piggyback on another player

// Emote kinds (low nibble of the emote byte, high nibble is a counter)
#define MP_EMOTE_NONE         0
#define MP_EMOTE_EXCLAMATION  1
#define MP_EMOTE_QUESTION     2
#define MP_EMOTE_HEART        3
#define MP_EMOTE_NOTE         4   // the music note bubble (combos, chat log)

#define MP_NUM_PHRASES   4    // custom phrases per player
#define MP_PHRASE_WORDS  4    // easy chat words per phrase

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
    u8 badges;         // how many gym badges (catch-up bonus)
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
    u16 phrases[MP_NUM_PHRASES][MP_PHRASE_WORDS];  // easy chat words, 0xFFFF = empty
    u8 quickStock[MP_NUM_PHRASES];   // R2 quick pick: the stock phrase an empty phrase slot offers
    u8 smallTweaks;           // AllowSmallTweaks (only from the file): MOEGATCHA typed codes
    struct MpCreated custom[MP_CREATED_SLOTS];   // LOOK > CREATE presets (CUSTOM 1-3)
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
void MP_Avatar_RestoreSprite(const struct MpAppearance *saved, const char *savedPack);
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
const char *MP_Avatar_HairName(u8 hair);
// Separate trainer card / battle front pic and back pic choices (0 = MATCH)
u16 MP_Avatar_FrontChoiceCount(void);          // choices + 1 (MATCH)
const char *MP_Avatar_FrontSelName(u16 sel);   // menu name
const char *MP_Avatar_FrontSelArtist(u16 sel); // "" unless it's credited fan art
const char *MP_Avatar_FrontSelConfigName(u16 sel);
u16 MP_Avatar_ParseFrontSel(const char *name);
u8 MP_Avatar_BackChoiceCount(void);
const char *MP_Avatar_BackSelName(u8 sel);
const char *MP_Avatar_BackSelConfigName(u8 sel);
u8 MP_Avatar_ParseBackSel(const char *name);
bool8 MP_Avatar_FrontSelRecolors(u16 sel);
void MP_Avatar_MatchPictures(struct MpAppearance *app, bool8 front, bool8 back); // MATCH, or DEFAULT (ID) / NONE (back) if there's nothing to match
// The local look's pictures as the battles show them (recolored); FALSE = none
bool8 MP_Avatar_PreviewFront(u8 *tiles, u16 *pal);
bool8 MP_Avatar_PreviewBack(u8 *tiles, u16 *pal);   // first frame
// Overworld graphics of any catalog sprite (Emerald or FireRed/LeafGreen)
const struct ObjectEventGraphicsInfo *MP_Avatar_NpcInfo(u16 gfxId);
const u16 *MP_Avatar_NpcPalette(u16 gfxId);
bool8 MP_Avatar_GetFrontTiles(const struct MpAppearance *app, u8 *tiles, u16 *pal);  // not recolored
bool8 MP_Avatar_GetBackTiles(const struct MpAppearance *app, u8 *tiles, u16 *pal);   // 4 frames, not recolored

// mp_creator.c: trainers made from the parts kit (LOOK > CREATE)
enum { MP_CR_HAIR, MP_CR_TOP, MP_CR_BOTTOM, MP_CR_HAT, MP_CR_BAG, MP_CR_CAT_COUNT };
struct MpCreatedGfx
{
    u8 ow[27][32 * 32 / 2];   // 32x32 frames: walk 0-8, run 9-17, Mach Bike 18-26 (Emerald's order)
    u16 owPal[16];
    u8 front[64 * 64 / 2];
    u16 frontPal[16];
    u8 back[4][64 * 64 / 2];   // Emerald's 4 throw frames
    u16 backPal[16];
};
#define MP_CR_CACHE_LOCAL MP_MAX_PLAYERS   // cache slots: player ids, then the local player
void MP_Created_Pack(const struct MpCreated *r, u8 *p);       // MP_CREATED_BYTES
void MP_Created_Unpack(const u8 *p, struct MpCreated *r);
void MP_Created_Default(struct MpCreated *r, bool8 girl);
void MP_Created_Clamp(struct MpCreated *r);
const struct MpCreatedGfx *MP_Created_Gfx(u8 cacheSlot, const struct MpCreated *r);
u8 MP_Created_StyleCount(bool8 girl, u8 cat);
const char *MP_Created_StyleName(bool8 girl, u8 cat, u8 style);
u8 MP_Created_ColorCount(bool8 girl, u8 cat, u8 style);
const char *MP_Created_ColorName(bool8 girl, u8 cat, u8 style, u8 color);
const char *MP_Created_SkinName(u8 skin);
#define MP_CR_SKIN_COUNT 4
void MP_Created_ToHex(const struct MpCreated *r, char *out);   // 2 * MP_CREATED_BYTES + 1 chars
u8 MP_Created_FieldHairCount(void);              // AUTO + the Gen 3 kit hairstyles
const char *MP_Created_FieldHairName(u8 fieldHair);
bool8 MP_Created_FromHex(const char *s, struct MpCreated *r);

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
enum { MP_MENU_PAGE_MAIN, MP_MENU_PAGE_LOOK, MP_MENU_PAGE_EMOTES, MP_MENU_PAGE_FRIENDS, MP_MENU_PAGE_CREATE };
void MP_OpenMenuPage(u8 page);
// Quick pick (emotes and phrases around the player): hold 0 LT/Q, 1 RT/E (let go to send), -1 stays open
enum { MP_QUICK_PAGE_EMOTES, MP_QUICK_PAGE_MINE, MP_QUICK_PAGE_PHRASES = 0xFF };
void MP_OpenQuickPick(u8 page, s8 hold);
void MP_OpenQuickPickAt(u8 page, s8 hold, s8 sel);
bool8 MP_QuickPickOpen(void);
void MP_Emote_EditPhraseFromField(u8 slot);   // word editor, then back to the field (screen already faded)
void MP_OpenMenuPageAt(u8 page, u8 cursor);

// Emotes and phrases (mp_emote.c)
struct ObjectEvent;
void MP_Emote_Update(void);
void MP_Emote_Icon(u8 kind);
void MP_Emote_Note(void);
void MP_Emote_SayText(const u8 *text);
void MP_Emote_SayQuick(u8 i);
u8 MP_Emote_QuickPhraseCount(void);
const char *MP_Emote_QuickPhrase(u8 i);
bool8 MP_Emote_PhraseIsEmpty(u8 slot);
void MP_Emote_PhraseText(u8 slot, u8 *dst);
#define MP_NUM_STOCK_PHRASES 16
const char *MP_Emote_StockPhrase(u8 i);
void MP_Emote_SayPhrase(u8 slot);
void MP_Emote_EditPhrase(u8 slot);
void MP_Emote_Hotkey(u8 n);
void MP_Emote_OnRemoteIcon(u8 id, struct ObjectEvent *obj, u8 kind);
void MP_Emote_OnRelay(u8 from, const u8 *data, u16 len);
bool8 MP_Emote_RemoteBubbleShown(u8 id);
void MP_Emote_OnPlayerLeft(u8 id);

// Friends' whereabouts, GO TO FRIEND (mp_friends.c)
u8 MP_Friends_List(u8 *ids);
u16 MP_Friends_Mapsec(u8 id);
void MP_Friends_LocationName(u8 id, u8 *dst);
u16 MP_Friends_Destination(u8 id);
bool8 MP_Friends_GoTo(u8 id);
bool8 MP_IsValidMap(u8 mapGroup, u8 mapNum);
u16 MP_BuddyBonusPercent(void);
void MP_Toast(const u8 *text);
void MP_Emote_ShowBubbleFor(u8 playerId, const u8 *text);
// mp_social.c
void MP_Social_Open(void);
void MP_Compass_Update(void);   // mp_compass.c
bool8 MP_Compass_AnyShown(void);
bool8 MP_Emote_LoadUiPalette(void);   // the sprite palette shared by bubbles, toasts and compass arrows
// mp_piggy.c
bool8 MP_Piggy_IsRiding(void);
void MP_Piggy_FieldInput(u16 newKeys);
void MP_Piggy_Update(void);
void MP_Piggy_OnRelay(u8 from, const u8 *data, u16 len);
void MP_Piggy_OnPlayerLeft(u8 id);
bool8 MP_Piggy_TryInteract(void);
// mp_together.c
void MP_Together_Update(void);
u8 MP_Together_Level(void);
u8 MP_Together_Meter(void);
u16 MP_Together_ExpPercent(void);
u8 MP_Together_ShinyRerolls(void);
u16 MP_CatchUpPercent(void);
u16 MP_ExpBonusPercent(void);
u8 MP_Badges(void);
void MP_Social_Update(void);
void MP_Social_OnRelay(u8 from, const u8 *data, u16 len);
void MP_Social_Log(const char *who, const char *text);
void MP_Social_LogSay(u8 id, const u8 *gameText);
void MP_Social_LogToast(const u8 *gameText);
void MP_Social_OnEmote(u8 id, u8 kind);
void MP_Social_OnSignRead(s16 x, s16 y);
void MP_Social_AfterSign(void);
bool8 MP_Prank_MagikarpActive(void);
bool8 MP_Prank_BackwardsActive(void);

// Shared Pokédex (mp_dex.c)
extern bool8 gMpDexApplying;
void MP_Dex_OnLocalFlag(u16 nationalNum, bool8 caught);
void MP_Dex_OnRelay(u8 from, const u8 *data, u16 len);
void MP_Dex_Update(void);
const u8 *MP_Dex_CaughtBy(u16 nationalNum);

// Co-op list windows (mp_ui.c)
#define MPUI_ROWS 7
// Co-op menu theme (mp_ui.c): colors of the swapped-in text palette
enum { MPT_BG = 1, MPT_TEXT, MPT_SHADOW, MPT_RED, MPT_GREEN = 6, MPT_BLUE = 8, MPT_BAND = 10, MPT_GOLD, MPT_SEL, MPT_GREY, MPT_BORDER, MPT_BAND2 };
void MpTheme_BuildPalette(u16 *pal16);   // from the player's window frame (OPTIONS > FRAME)
void MpTheme_Push(void);
void MpTheme_Pop(void);
void MpTheme_Panel(u8 windowId, u8 bandHeight);
void MpTheme_SelBar(u8 windowId, int y, int h);
void MpTheme_Print(u8 windowId, u8 font, const u8 *str, int x, int y, u8 fg, u8 bg);
bool8 MpUi_IsOpen(void);
void MpUi_Open(void);
void MpUi_Close(void);
void MpUi_Begin(const char *title);
void MpUi_TitleRight(const u8 *text);
void MpUi_Row(u8 row, bool8 selected, const u8 *left, const u8 *right);
void MpUi_RowAscii(u8 row, bool8 selected, const char *left, const char *right);
void MpUi_TextAscii(u8 row, const char *text);
void MpUi_End(void);
void MpUi_Help(const u8 *text);
void MpUi_HelpAscii(const char *ascii);
void MpUi_HelpItem(u16 item);   // the item's description on the help line
void MpUi_PrintWrapped(u8 windowId, const u8 *text, int x, int y, int width, int maxLines, u8 fg, u8 bg);
void MP_ShowFieldMessageFitted(const u8 *text);   // ShowFieldMessage, wrapped to the box with pages
bool8 MpUi_MoveCursor(u16 *cursor, u16 count);
u16 MpUi_ScrollFor(u16 cursor, u16 count, u16 *scroll);
void MpUi_Qty(u8 *dst, u32 qty);

// Data kept next to the save file (mp_savedata.c)
s32 MP_SaveData_Get(const char *key);
u16 MP_RegisteredItem(u8 slot);            // item_menu.c: key items on L (0) / R (1)
bool8 MP_UseRegisteredItemSlot(u8 slot);
bool8 MP_Qol_OpenFollowerPicker(void);     // mp_qol.c
void MP_Qol_OnScriptStart(const u8 *script);
void MP_Qol_SignStarted(void);
void MP_Qol_Update(void);
bool8 MP_Qol_SkipActive(void);
u8 MP_Qol_FieldTextSpeed(u8 normalSpeed);
void MP_SaveData_Set(const char *key, s32 value);
void MP_SaveData_Add(const char *key, s32 delta);
s32 MP_Today(void);              // YYYYMMDD, local time
s32 MP_DayNumber(s32 yyyymmdd);  // days since 2000-01-01
s32 MP_WeekNumber(void);

// Shared PC and room counters (mp_sharedpc.c)
void MP_SharedPc_Open(void);
void MP_Store_OnMessage(const u8 *data, u16 len);
s32 MP_Counter_Get(const char *key);
u32 MP_Counter_Version(void);
void MP_Counter_Add(const char *key, s32 delta);
void MP_Counter_Max(const char *key, s32 value);
void MP_Counter_Request(const char *prefix);
void MP_Counter_ForEach(const char *prefix, void (*fn)(const char *key, s32 value, void *arg), void *arg);

// Trading (mp_trade.c) and daily gifts (mp_gift.c)
void MP_Trade_OpenMenu(void);
void MP_Trade_OnRelay(u8 from, const u8 *data, u16 len);
void MP_Trade_Update(void);
void MP_Gift_OpenMenu(void);
void MP_Gift_OnRelay(u8 from, const u8 *data, u16 len);

// Daily goals, streak, outbreak, achievements, weekly race (mp_events.c)
void MP_Events_Update(void);
void MP_Events_OnTrainerWin(void);
void MP_Events_OnCoopWin(u8 partner);
void MP_Events_OnWildEnd(u8 outcome, u8 level);
void MP_Events_QueueToast(const char *ascii);   // shown on the field, one after another
void MP_Events_OnTrade(void);
void MP_Events_OnGiftSent(void);
u16 MP_Events_MaybeOutbreak(u16 species);
s32 MP_Events_StreakDays(void);
void MP_Daily_OpenMenu(void);

// Team rematches with gym leaders (mp_rematch.c)
const u8 *MP_Rematch_ScriptForObject(const u8 *script);
bool8 MP_Rematch_IsRematchTrainer(u16 trainer);
void MP_Rematch_Reward(void);
void MP_Rematch_IsEndgame(void);

// Battle tournament (mp_tourney.c)
extern bool8 gMpFriendlyBattle;
void MP_Tourney_OpenMenu(void);
void MP_Tourney_OnRelay(u8 from, const u8 *data, u16 len);
void MP_Tourney_Update(void);
void MP_Tourney_OnBattleResult(u8 outcome);
void MP_Battle_Start(u8 opponentId, bool8 isMaster, u32 opponentMoney);
void MP_AsciiToGame(u8 *dst, const char *src, int maxLen);

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
extern bool8 gMpCoopWild;            // TRUE while a co-op wild double runs (on both games)
bool8 MP_Coop_TryWildTeamUp(void);   // a land wild encounter: team up with the closest friend in the grass
void MP_Coop_SetWildEnemyParty(void);
void MP_Coop_OnWildCaught(u8 throwerPartyIndex, struct Pokemon *mon);
void (*MP_EndWildBattleCallback(void))(void);        // battle_setup.c
void MP_StartNormalWildBattle(void);                 // battle_setup.c
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
    MPR_SAY = 80,              // speech bubble: u8 kind, text (mp_emote.c)
    MPR_DEX_FIRST = 84,        // shared Pokedex (mp_dex.c)
    MPR_SOCIAL_END = 100,
    MPR_TRADE_FIRST = 100,     // trading (mp_trade.c)
    MPR_GIFT_FIRST = 110,      // daily gifts (mp_gift.c)
    MPR_EVENT_FIRST = 120,     // goals, race, achievements (mp_events.c)
    MPR_TOURNEY_FIRST = 140,   // battle tournament (mp_tourney.c)
    MPR_SOCIAL_FIRST = 160,    // pranks, piggyback... (mp_social.c, mp_piggy.c)
    MPR_LINKX_FIRST = 180,     // record mixing, secret bases, union room... (mp_linkx.c)
    MPR_MINIGAME_FIRST = 200,  // berry blender / berry crush with friends (mp_minigame.c)
};


// mp_minigame.c: BERRY BLENDER / BERRY CRUSH with friends (CO-OP > BERRY GAMES)
void MP_Minigame_Update(void);
void MP_Minigame_OnRelay(u8 from, const u8 *data, u16 len);
void MP_Minigame_OnPlayerLeft(u8 id);
void MP_Minigame_HostBlender(void);
bool8 MP_Blender_Active(void);
u8 MP_Blender_LocalSlot(void);
u8 MP_Blender_NumPlayers(void);
bool8 MP_Blender_IsLeader(void);
void MP_Blender_FillNames(void);
void MP_Blender_SendBerry(u16 item);
bool8 MP_Blender_Ready(u16 *items);
void MP_Blender_SendHit(u16 score);
u16 MP_Blender_TakeHit(u8 slot);
void MP_Blender_SendState(u16 arrow, s16 speed, u16 progress);
bool8 MP_Blender_TakeState(u16 *arrow, s16 *speed, u16 *progress);
void MP_Blender_SendEnd(u16 rpm, u32 time, const u16 *scores);
bool8 MP_Blender_TakeEnd(u16 *rpm, u32 *time, u16 *scores);
void MP_Blender_Finish(void);
void MP_Minigame_HostCrush(void);
void MP_Minigame_SendNews(const char *ascii);
bool8 MP_Crush_Active(void);
u8 MP_Crush_NumPlayers(void);
u8 MP_Crush_LocalSlot(void);
bool8 MP_Crush_Broken(void);
void MP_Crush_Standby(void);
bool8 MP_Crush_TaskFinished(void);
bool8 MP_Crush_SendBlock(const void *src, u16 size);
u8 MP_Crush_BlockStatus(void);
void MP_Crush_ResetBlocks(void);
void MP_Crush_SendPacket(void *data);
void MP_Crush_Finish(void);
void MP_StartBerryCrush(void);

// mp_grouplink.c: 2-4 player link over the relay (link CONTESTS)
bool8 MP_Group_Active(void);
u8 MP_Group_GetMultiplayerId(void);
u8 MP_Group_GetPlayerCount(void);
bool8 MP_Group_IsMaster(void);
bool8 MP_Group_SendBlock(const void *src, u16 size);
u8 MP_Group_GetBlockReceivedStatus(void);
void MP_Group_ResetBlockReceivedFlag(u8 who);
bool8 MP_Group_IsTaskFinished(void);
void MP_Group_SetStandbyCallback(void);
void MP_Group_SetCloseCallback(void);
void MP_Group_Close(void);
void MP_Group_OnRelay(u8 from, const u8 *data, u16 len);
void MP_Group_OnPlayerLeft(u8 id);
void MP_Group_Update(void);
bool8 MP_Contest_TryLinkup(void);

// mp_linkx.c: record mixing, Secret Base visits, Trainer Cards, Day Care eggs,
// shared roamers and weather, Hall of Fame together, the FRIENDS / BERRY GAMES pages
void MP_Linkx_Update(void);
void MP_Linkx_OnRelay(u8 from, const u8 *data, u16 len);
void MP_Linkx_OnPlayerLeft(u8 id);
void MP_Linkx_OpenUnion(void);   // CO-OP > BERRY GAMES
void MP_Linkx_OpenFriend(u8 id);  // CO-OP > FRIENDS > a friend
u16 MP_Linkx_CardPicOr(u16 defaultPic);
bool8 MP_Linkx_FriendDaycareMon(u8 id, struct BoxPokemon *out);
void MP_Linkx_SetPendingEgg(const struct Pokemon *egg, u8 friendId);
bool8 MP_Linkx_TakePendingEgg(struct Pokemon *egg);
void MP_Daycare_TryCoopEgg(struct DayCare *daycare);
void MP_Linkx_HofBegin(void);
u8 MP_Linkx_HofFriendCount(void);
bool8 MP_Linkx_HofFriend(u8 i, u8 *team, u8 *name, u16 *pic);

#endif // GUARD_MP_H
