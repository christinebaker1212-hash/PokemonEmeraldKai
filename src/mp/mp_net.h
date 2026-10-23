// Moemon Co-op: low level networking (TCP framing, client, lobby + relay server).
// This header is plain C and must not depend on the game's headers so that the
// standalone lobby server (tools/mp_server) can be built from the same sources.
#ifndef GUARD_MP_NET_H
#define GUARD_MP_NET_H

#include <stdint.h>

#ifndef MP_PROTOCOL_VERSION   // -D only to build an "other version" server for tests
#define MP_PROTOCOL_VERSION 34   // 34: joining a battle in progress (mp_join.c); 33: SPECTATE panel (screens any time, scroll shifts, palette tiles); 32: held HMs never travel (trades, shared PC); 31: a follower's ball effect in 24 bits (state byte 23); 30: vanilla gender ratios, light level
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
#define MP_WIRESTATE_SIZE  24

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

// Room storage (mp_store.c): the shared PC and counters, saved by the server
#define MP_STORE_MONS        60
#define MP_STORE_MON_BYTES   112   // opaque to the server (the game stores a Pokemon + who put it in)
#define MP_STORE_ITEMS       64
#define MP_STORE_COUNTERS    512
#define MP_STORE_KEY_MAX     31
enum {
    MP_ST_PC_LIST = 1,     // -> u8 count, (u8 slot, data)*
    MP_ST_PC_PUT,          // data -> u8 ok, u8 slot
    MP_ST_PC_TAKE,         // u8 slot -> u8 ok, u8 slot, data
    MP_ST_PC_CHANGED,      // (to the room) someone put in or took a Pokemon
    MP_ST_ITEM_LIST,       // -> u8 count, (u16 item, u16 qty)*
    MP_ST_ITEM_PUT,        // u16 item, u16 qty -> u16 item, u16 stored
    MP_ST_ITEM_TAKE,       // u16 item, u16 qty -> u16 item, u16 taken
    MP_ST_ITEMS_CHANGED,   // (to the room)
    MP_ST_CTR_ADD,         // s32 delta, str key -> (to the room) CTR_VALUE
    MP_ST_CTR_MAX,         // s32 value, str key: keep the larger -> (to the room) CTR_VALUE
    MP_ST_CTR_GET,         // str prefix -> CTR_VALUE* then CTR_END
    MP_ST_CTR_VALUE,       // s32 value, str key
    MP_ST_CTR_END,         // str prefix
};
typedef void (*MPStoreSendFunc)(void *ctx, int toAll, const uint8_t *data, uint16_t len);
void MPStore_Handle(const uint8_t *roomKey, const uint8_t *data, uint16_t len, MPStoreSendFunc send, void *ctx);

// Logging hook (printf style). Defaults to stdout.
void MPNet_Log(const char *fmt, ...);

#endif // GUARD_MP_NET_H
