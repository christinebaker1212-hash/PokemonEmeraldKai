// Moemon Co-op: low level networking (TCP framing, client, lobby + relay server).
// This header is plain C and must not depend on the game's headers so that the
// standalone lobby server (tools/mp_server) can be built from the same sources.
#ifndef GUARD_MP_NET_H
#define GUARD_MP_NET_H

#include <stdint.h>

#ifndef MP_PROTOCOL_VERSION   // -D only to build an "other version" server for tests
#define MP_PROTOCOL_VERSION 67   // 67: Gen 5-9 (species 519-1050 with the EGG at 1051 and Unown's letters after, moves 468-847, abilities 125-311 as u16 in the battle structures, items 508-552, FAIRY type 18; the Hall of Fame team block 126 bytes); 66: Johto and Kanto (map groups 34-39, MPR_RULES last byte = the host's region, a room is in its host's region); 65: Gen 4 (species 412-518 with the EGG at 519 and Unown's letters after, moves 355-467, abilities 78-124, items 400-474 (CARD CASE stays 399); MPR_RULES byte 4 = MOVE KIND); 64: SPECTATE frames carry the custom song's place in the song number's bits 10-15 (music/*.kaisong, mp_kaisong.c); 63: joining a battle against two trainers (MpJoinPlan trainerIdB, the snapshot's second lose text; MPR_COOP_BATTLING bytes 8-10: the second trainer's object and id), story battles can be joined; 62: Triple Triad games and trades with friends (relay extension 255 version 8: request kinds 4-5, type 14 src/tt/tt_net.c); 61: the DANCE machine (relay 32-35: pads, asks, its own messages; store ops MP_ST_RH_* 40-42: the server's records); 60: Yu-Gi-Oh! answers carry the zone a card goes to (relay extension 255 version 7); TEAM PADDING pads a Triple Battle to three, grunts can be joined; 59: Yu-Gi-Oh! duels and trades with friends (relay extension 255 version 6: request kinds 2-3, types 8-13 src/ygo/ygo_net.c); 58: CO-OP > TOURNAMENT of card duels (MPR_T_INVITE carries the kind) and online concessions with their win event (relay extension 255); 57: VS cabinet challenges carry the fight files' hashes and the answering player's cabinet rules (MPR_FIGHT_ASK / ANSWER), desync recovery (MPR_FIGHT_RESYNC 31); 56: the GAME CENTER (RUSTBORO indoor maps 17-20: older games have no such maps); 55: the VS cabinet online (relay messages 25-29: seats, challenges, buttons; src/mp/mp_fight.c); 54: co-op levels: MPR_COOP_RESOLVE boxes carry the moves and PP (25 bytes each: moves learned without forgetting one reach their owner in the battle); 53: card rules 2 (relay extension 255 version 5: every Pokemon Power, the effects audit; TCG_DISCARD_DOLL; new field state in the duel hash); 52: card duels 2.0 (relay extension 255 version 4: requests / answers / card trades; the wire state light byte bit 7 = in a card duel); 51: item 399 (CARD CASE) and card duels (relay extension 255, src/tcg/tcg_link.c); 50: the battle weather in the battle link (B_COMM_MP_WEATHER blocks from the master); 49: LEVEL SCALING settings (MPR_RULES bytes 4-8: MP_SCALE_WILD..PAST8); 48: the MOéMON GARDEN BERRY farm (MPR_FARM_STATE / ASK / ANSWER 252-254, GA_FARM in the actor relay); 47: PMD Collab followers and Garden Pokémon (life poses 7-19: the PMD actions; followerFlags bit 7 = shiny; Garden actors: pose in 6 bits of byte 11, byte 6 bit 1 = shiny); 46: level scaling (wire state 40 bytes: top3, the team's level; MPR_RULES byte 3 = LEVEL SCALING); 45: the MOéMON GARDEN with a place per type (a new 56x40 layout: Garden Pokémon positions mean other tiles); 44: the MOéMON GARDEN is every POKéMON CENTER's 2F (a new layout, one open meadow: no hedge); 43: followers on their own and Garden Pokémon replay their owner's exact steps and facing (wire state 39 bytes: lifePx / lifePy / lifeFace; Garden actors 18 bytes); 42: MOéMON GARDEN long-term life (Garden activities GA_PURSUIT / RARE / REACT / SONG in the actor relay); 41: MOéMON GARDEN (wire state 36 bytes: follower key, its activity, pose, own tile and emote; MPR_LIFE_VERB / REPLY / RECORD / ASK); 40: LOOK > BIKE moves to appearance byte 27 (26 is the CREATE recipe's FIELD HAIR), BATTLE FLAG phrases (MPR_ARRIVED / BATTLE_GO carry 3 x 6 easy chat words, MPR_PHRASE_READY), the loser keeps their money, catch EXP (co-op: a catch evolves like a win); 39: characters from Moemon's other branch (graphics ids 0x9000+, appearance byte 26 = LOOK > BIKE), 127 ID / 107 BACK pictures more; 38: co-op against two trainers (ACCEPT: second trainer + lose text; WAIT: a second NPC), held trainers spot friends, waiting players merge; 37: a follower's Stadium hue in the wire state (26 bytes); 36: shared PC as 14 boxes of 30 (BOX_GET / TXN / CURSOR); 35: items 389-398 (QUICK POWDER, EVIOLITE, treats, CANDY JAR); 34: joining a battle in progress (mp_join.c); 33: SPECTATE panel (screens any time, scroll shifts, palette tiles); 32: held HMs never travel (trades, shared PC); 31: a follower's ball effect in 24 bits (state byte 23); 30: vanilla gender ratios, light level
#endif
#define MP_DEFAULT_PORT     27015      // TCP: lobby server
#define MP_DISCOVERY_PORT   27016      // UDP: finding games on the local network
#define MP_MAX_PLAYERS      4
#define MP_MAX_MSG          8192

// Lobby address built into the game. Players never see or type it.
// Override at build time with  make ... LOBBY=my.server.com  (or LOBBY=1.2.3.4:27015),
// or per player with  lobby = ...  in moemon_coop.ini.
#ifndef MP_DEFAULT_LOBBY
#define MP_DEFAULT_LOBBY ""
#endif

// ---------------------------------------------------------------------------
// Connection flow
//   1. Client connects to a lobby server (the online one, or a game hosting on the LAN).
//   2. Lobby phase: LIST rooms, CREATE a room (name + password) or JOIN one.
//   3. Once in a room, the client sends HELLO when its save file is loaded and
//      plays as before. The server relays everything only within the room.

// Message types, client -> server
enum {
    MPMSG_C_HELLO = 1,        // MpHello
    MPMSG_C_STATE = 2,        // MpWireState
    MPMSG_C_BLOB = 3,         // u8 kind, u32 totalLen, u32 offset, data (chunked big data, e.g. sprites)
    MPMSG_C_RELAY = 4,        // u8 targetId (0xFF = everyone in the room), then opaque peer payload
    MPMSG_C_INFO = 5,         // MpHello (appearance/name update)
    MPMSG_C_PING = 6,
    MPMSG_C_ROOM_LIST = 7,    // (nothing)
    MPMSG_C_ROOM_CREATE = 8,  // u16 version, str name, u8 hasPassword, u8[32] key, u8 hidden
    MPMSG_C_ROOM_JOIN = 9,    // u16 version, str name, u8[32] key
    MPMSG_C_ROOM_LEAVE = 10,
    MPMSG_C_STORE = 11,       // u8 op, ... (room storage, see mp_store.c)
};

// Message types, server -> client
enum {
    MPMSG_S_WELCOME = 20,     // u8 yourId, u8 maxPlayers
    MPMSG_S_REJECT = 21,      // u8 reason, u16 server version
    MPMSG_S_PLAYER_INFO = 22, // u8 id, MpHello
    MPMSG_S_PLAYER_LEAVE = 23,// u8 id
    MPMSG_S_STATE = 24,       // u8 id, MpWireState
    MPMSG_S_BLOB = 25,        // u8 id, then the same payload as MPMSG_C_BLOB
    MPMSG_S_RELAY = 26,       // u8 fromId, opaque peer payload
    MPMSG_S_PONG = 27,
    MPMSG_S_ROOM_LIST = 28,   // u8 count, then per room: str name, u8 players, u8 max, u8 flags, u16 version
    MPMSG_S_ROOM_OK = 29,     // str name, u8 players, u8 created
    MPMSG_S_ROOM_ERROR = 30,  // u8 reason, u16 server version (protocol 20+ servers)
    MPMSG_S_STORE = 31,       // u8 op, ... (room storage replies and updates)
};

// "str" on the wire = u8 length + that many ASCII bytes (no terminator)
#define MP_ROOM_NAME_MAX   16
#define MP_ROOM_KEY_SIZE   32
#define MP_ROOM_FLAG_LOCKED  1   // needs a password

enum {
    MP_REJECT_FULL = 1,
    MP_REJECT_VERSION = 2,
    MP_REJECT_NO_ROOM = 3,
};

enum {
    MP_ROOM_ERR_NONE,
    MP_ROOM_ERR_NAME_TAKEN,
    MP_ROOM_ERR_NOT_FOUND,
    MP_ROOM_ERR_BAD_PASSWORD,
    MP_ROOM_ERR_FULL,
    MP_ROOM_ERR_VERSION,
    MP_ROOM_ERR_BAD_NAME,
    MP_ROOM_ERR_SERVER_FULL,
    MP_ROOM_ERR_SLOW_DOWN,
};

// Big data kinds (MPMSG_C_BLOB)
#define MP_BLOB_SPRITES    0
#define MP_BLOB_KINDS      1
#define MP_BLOB_MAX_SIZE   (192 * 1024)
#define MP_BLOB_CHUNK      (MP_MAX_MSG - 64)

// Wire structs are serialized by hand (little endian), these are just sizes.
#define MP_NAME_LEN        8   // game charset, EOS (0xFF) padded
#define MP_HELLO_SIZE      (2 + MP_NAME_LEN + 4 + 32)
#define MP_WIRESTATE_SIZE  40

// ---- Client ----
enum {
    MPNET_DISCONNECTED = 0,
    MPNET_CONNECTING,
    MPNET_CONNECTED,
};

int MPNet_Init(void);
int MPNet_Connect(const char *host, int port);
int MPNet_ClientStatus(void);
void MPNet_ClientSend(uint8_t type, const void *data, uint16_t len);
// Returns 1 and fills type/buf/len when a full message is available.
int MPNet_ClientPoll(uint8_t *type, uint8_t *buf, uint16_t *len);
void MPNet_ClientClose(void);
const char *MPNet_LastError(void);
// Splits "host:port" (or "[v6]:port"); port stays as given when absent.
void MPNet_ParseAddress(const char *addr, char *host, int hostLen, int *port);

// Room key: SHA-256 of "<lowercase room name>\n<password>". The server never sees passwords.
void MPNet_RoomKey(const char *roomName, const char *password, uint8_t out[MP_ROOM_KEY_SIZE]);
void MPNet_Sha256(const void *data, int len, uint8_t out[32]);
int MPNet_IsValidRoomName(const char *name);

// ---- Lobby + relay server (runs standalone, or inside a game hosting on the LAN) ----
int MPServer_Start(int port, int lanDiscovery);
void MPServer_Poll(void);
void MPNet_ClientFlush(void);
int MPNet_ClientQueued(void);   // bytes waiting to go out (the connection is slower than what we send)
void MPServer_Stop(void);
int MPServer_Port(void);
int MPServer_IsRunning(void);
int MPServer_PlayerCount(void);
int MPServer_RoomCount(void);
// Standalone server: wait up to `ms` for network activity (0 = just poll)
void MPServer_Wait(int ms);

// ---- Local network discovery (client side) ----
#define MP_LAN_MAX_GAMES 8
struct MpLanRoom {
    char name[MP_ROOM_NAME_MAX + 1];
    uint8_t players, maxPlayers, flags;
    uint16_t version;
    char host[64];
    int port;
};
void MPLan_StartSearch(void);         // sends a broadcast probe; call again to refresh
int MPLan_Poll(struct MpLanRoom *out, int max); // returns the rooms found so far
void MPLan_StopSearch(void);

// Opening the host's router (src/mp/mp_portmap.c). Runs on a background thread.
enum { MP_PM_IDLE, MP_PM_WORKING, MP_PM_OPEN, MP_PM_FAILED };
enum { MP_PM_NONE, MP_PM_UPNP, MP_PM_NATPMP };
enum {
    MP_PM_ERR_NO_ROUTER,   // no UPnP / NAT-PMP answer (turned off in the router)
    MP_PM_ERR_REFUSED,     // the router said no
    MP_PM_ERR_CGNAT,       // the internet provider shares one address between homes
};
void MPPortMap_Open(int internalPort);
void MPPortMap_Close(void);            // removes the mapping
int MPPortMap_State(void);
int MPPortMap_Error(void);
int MPPortMap_Method(void);
const char *MPPortMap_InviteCode(void);    // "K7QM-3XPA" while open, else ""
const char *MPPortMap_PublicAddress(void);
int MPPortMap_ExternalPort(void);
// Invite codes: a public IPv4 address + one of 8 game ports in 8 characters
void MPNet_MakeInviteCode(uint32_t ip, int port, char *out, int len);
int MPNet_ParseInviteCode(const char *code, uint32_t *ip, int *port);
// Radmin VPN (26.x) / Hamachi (25.x) adapter address, the last resort
int MPNet_FindVpnAddress(char *out, int len, char *nameOut, int nameLen);

// Room storage (mp_store.c): the shared PC and counters, saved by the server.
// The shared PC is 14 boxes of 30 like a personal one. A stored Pokemon is MP_STORE_MON_BYTES opaque bytes to the
// server, except its first 8: personality and OT ID (little-endian u32s), which tell one Pokemon from another. A
// player picking one up moves it into their "hand" on the server (nobody else can take it); if they leave, whatever
// is still in their hand goes back into the first free slots.
#define MP_SHARED_BOXES      14
#define MP_SHARED_BOX_SLOTS  30
#define MP_STORE_MONS        (MP_SHARED_BOXES * MP_SHARED_BOX_SLOTS)
#define MP_STORE_MON_BYTES   112   // the game stores a Pokemon + who put it in
#define MP_STORE_HAND        36    // a box and a party
#define MP_STORE_BOXNAME     9     // 8 game characters + EOS
#define MP_STORE_ITEMS       64
#define MP_STORE_COUNTERS    512
#define MP_STORE_KEY_MAX     31
enum {
    MP_ST_BOX_GET = 1,     // u8 box (0xFF: all) -> BOX_DATA for each
    MP_ST_BOX_DATA,        // u8 box, name[9], u8 wallpaper, u8 count, (u8 pos, data)*   (also to the room after a change)
    MP_ST_TXN,             // u16 seq, then commands (MP_TXN_*) -> BOX_DATA (room) for changed boxes, then TXN_RESULT
    MP_ST_TXN_RESULT,      // u16 seq, u8 refusedIn, (u32 pers, u32 otid)*, u8 lostHolds, (u32, u32)*, u8 hand, data*
    MP_ST_CURSOR,          // payload -> (to the others) u8 player, payload
    MP_ST_ITEM_LIST,       // -> u8 count, (u16 item, u16 qty)*
    MP_ST_ITEM_PUT,        // u16 item, u16 qty -> u16 item, u16 stored
    MP_ST_ITEM_TAKE,       // u16 item, u16 qty -> u16 item, u16 taken
    MP_ST_ITEMS_CHANGED,   // (to the room)
    MP_ST_CTR_ADD,         // s32 delta, str key -> (to the room) CTR_VALUE
    MP_ST_CTR_MAX,         // s32 value, str key: keep the larger -> (to the room) CTR_VALUE
    MP_ST_CTR_GET,         // str prefix -> CTR_VALUE* then CTR_END
    MP_ST_CTR_VALUE,       // s32 value, str key
    MP_ST_CTR_END,         // str prefix
    // the DANCE machine's records, for the whole server (mp_scores.c)
    MP_ST_RH_SUBMIT = 40,  // u32 chart, u8 diff, u8 meter, u32 score, u8 grade, u8 fullCombo, u16 maxCombo,
                           //   name[MP_RH_NAME], u8 len, title -> RH_TOP (to the room when it's a new record)
    MP_ST_RH_GET,          // u32 chart -> RH_TOP
    MP_ST_RH_TOP,          // u32 chart, u8 count, (name[MP_RH_NAME], u32 score, u8 grade, u8 fullCombo, u16 maxCombo)*
    // the BATTLE SIMULATOR's digital trainers, for the whole server (mp_scores.c; the record itself: mp_sim.c)
    MP_ST_SIM_PUT = 43,    // a player's record (its first MP_SIM_KEY bytes: name and Trainer ID) -> nothing
    MP_ST_SIM_GET,         // -> SIM_ROSTER parts
    MP_ST_SIM_ROSTER,      // u8 part, u8 parts, u8 count, (u16 len, record)*  (part 0xFF: one record to merge in,
                           //   sent to the room when someone's record arrives)
};
#define MP_SIM_KEY 12          // a record's identity: name (8) + Trainer ID (4)
#define MP_SIM_REC_MAX 480     // the largest record the server keeps
#define MP_SIM_PLAYERS 256     // the most players the server remembers (the least recently heard from go first)
#define MP_RH_NAME 12          // ASCII, 0 padded
#define MP_RH_TOP 10
// MP_ST_TXN commands, applied in order (each one on its own: a failed one doesn't undo the others)
enum {
    MP_TXN_HOLD = 1,       // u8 box, u8 pos, u32 pers, u32 otid: slot -> hand (only if that very Pokemon is there)
    MP_TXN_PLACE,          // u8 box, u8 pos, data: the hand's Pokemon with that personality / OT ID -> slot, with this
                           //   data (taken: the next free slot)
    MP_TXN_IN,             // u8 box, u8 pos, data: from the player's own game -> slot (taken: the next free; full:
                           //   refused, the game gives it back)
    MP_TXN_OUT,            // u32 pers, u32 otid: hand -> the player's own game
    MP_TXN_RELEASE,        // u32 pers, u32 otid: hand -> gone
    MP_TXN_SET,            // u8 box, u8 pos, u32 pers, u32 otid, data: new data for the same Pokemon (markings, name)
    MP_TXN_BOXINFO,        // u8 box, name[9], u8 wallpaper
};
typedef void (*MPStoreSendFunc)(void *ctx, int toAll, const uint8_t *data, uint16_t len);
// player: the sender's seat in the room (whose hand)
void MPStore_Handle(const uint8_t *roomKey, int player, const uint8_t *data, uint16_t len, MPStoreSendFunc send, void *ctx);
// The DANCE machine's records (mp_scores.c): server-wide, whatever the room
void MPScores_Handle(const uint8_t *data, uint16_t len, MPStoreSendFunc send, void *ctx);
// A player left the room: their hand goes back into free slots (BOX_DATA to the room)
void MPStore_PlayerLeft(const uint8_t *roomKey, int player, MPStoreSendFunc send, void *ctx);

// UDP between two players (the VS cabinet's buttons). Addresses and ports in network order.
int MPUdp_Open(void);               // a socket on any port (once): 0 when open
void MPUdp_Close(void);
uint16_t MPUdp_Port(void);
uint32_t MPUdp_LocalIp(void);       // our address on the local network
void MPUdp_AskPublic(void);         // asks the lobby what our address is from outside (MPUdp_Public when it answers)
int MPUdp_Public(uint32_t *ip, uint16_t *port);
void MPUdp_SendTo(uint32_t ip, uint16_t port, const void *data, int len);
int MPUdp_Recv(void *buf, int max, uint32_t *ip, uint16_t *port);   // length, 0 none

// Logging hook (printf style). Defaults to stdout.
void MPNet_Log(const char *fmt, ...);

#endif // GUARD_MP_NET_H
