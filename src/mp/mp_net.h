// Moemon Co-op: low level networking (TCP framing, client + relay server).
// This header is plain C and must not depend on the game's headers so that the
// standalone server (tools/mp_server) can be built from the same sources.
#ifndef GUARD_MP_NET_H
#define GUARD_MP_NET_H

#include <stdint.h>

#define MP_PROTOCOL_VERSION 3
#define MP_DEFAULT_PORT     27015
#define MP_MAX_PLAYERS      4
#define MP_MAX_MSG          8192

// Message types, client -> server
enum {
    MPMSG_C_HELLO = 1,        // MpHello
    MPMSG_C_STATE = 2,        // MpWireState
    MPMSG_C_CUSTOM_GFX = 3,   // raw custom sprite blob
    MPMSG_C_RELAY = 4,        // u8 targetId, then opaque peer payload
    MPMSG_C_INFO = 5,         // MpHello (appearance/name update)
    MPMSG_C_PING = 6,
};

// Message types, server -> client
enum {
    MPMSG_S_WELCOME = 20,     // u8 yourId, u8 maxPlayers
    MPMSG_S_REJECT = 21,      // u8 reason
    MPMSG_S_PLAYER_INFO = 22, // u8 id, MpHello
    MPMSG_S_PLAYER_LEAVE = 23,// u8 id
    MPMSG_S_STATE = 24,       // u8 id, MpWireState
    MPMSG_S_CUSTOM_GFX = 25,  // u8 id, blob
    MPMSG_S_RELAY = 26,       // u8 fromId, opaque peer payload
    MPMSG_S_PONG = 27,
};

enum {
    MP_REJECT_FULL = 1,
    MP_REJECT_VERSION = 2,
};

// Wire structs are serialized by hand (little endian), these are just sizes.
#define MP_NAME_LEN        8   // game charset, EOS (0xFF) padded
#define MP_HELLO_SIZE      (2 + MP_NAME_LEN + 4 + 8)
#define MP_WIRESTATE_SIZE  16

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

// ---- Relay server (can run inside the game or standalone) ----
int MPServer_Start(int port);
void MPServer_Poll(void);
void MPServer_Stop(void);
int MPServer_IsRunning(void);
int MPServer_PlayerCount(void);

// Logging hook (printf style). Defaults to stdout.
void MPNet_Log(const char *fmt, ...);

#endif // GUARD_MP_NET_H
