// Moemon Co-op: TCP transport, message framing, client and relay server.
// Frame format: u16 length (little endian, counts type + payload), u8 type, payload.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
typedef SOCKET mp_sock;
#define MP_BADSOCK INVALID_SOCKET
#define mp_closesock closesocket
#define MP_WOULDBLOCK(e) ((e) == WSAEWOULDBLOCK || (e) == WSAEINPROGRESS || (e) == WSAEALREADY)
static int mp_lasterr(void) { return WSAGetLastError(); }
#else
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
typedef int mp_sock;
#define MP_BADSOCK (-1)
#define mp_closesock close
#define MP_WOULDBLOCK(e) ((e) == EWOULDBLOCK || (e) == EAGAIN || (e) == EINPROGRESS)
static int mp_lasterr(void) { return errno; }
#endif

#include "mp_net.h"

static char sLastError[160];

void MPNet_Log(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    printf("[mp] ");
    vprintf(fmt, ap);
    printf("\n");
    fflush(stdout);
    va_end(ap);
}

const char *MPNet_LastError(void)
{
    return sLastError;
}

static void SetNonBlocking(mp_sock s)
{
#ifdef _WIN32
    u_long mode = 1;
    ioctlsocket(s, FIONBIO, &mode);
#else
    int flags = fcntl(s, F_GETFL, 0);
    fcntl(s, F_SETFL, flags | O_NONBLOCK);
#endif
    {
        int one = 1;
        setsockopt(s, IPPROTO_TCP, TCP_NODELAY, (const char *)&one, sizeof(one));
    }
}

int MPNet_Init(void)
{
    static int sInit = 0;
    if (sInit)
        return 0;
#ifdef _WIN32
    {
        WSADATA wsa;
        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
            return -1;
    }
#endif
    sInit = 1;
    return 0;
}

// ---------------------------------------------------------------------------
// Connection buffers (shared by client and server)

#define RECVBUF_SIZE (MP_MAX_MSG * 8)

struct MpConn {
    mp_sock sock;
    int connecting;
    uint8_t recv[RECVBUF_SIZE];
    int recvLen;
    uint8_t *send;
    int sendLen;
    int sendCap;
};

static void Conn_Reset(struct MpConn *c)
{
    if (c->sock != MP_BADSOCK)
        mp_closesock(c->sock);
    c->sock = MP_BADSOCK;
    c->connecting = 0;
    c->recvLen = 0;
    c->sendLen = 0;
}

static void Conn_Queue(struct MpConn *c, uint8_t type, const void *data, uint16_t len)
{
    int need = c->sendLen + 3 + len;
    uint16_t frameLen = (uint16_t)(len + 1);
    if (need > c->sendCap)
    {
        int cap = c->sendCap ? c->sendCap : 4096;
        while (cap < need)
            cap *= 2;
        if (cap > 4 * 1024 * 1024)
            return; // peer is not reading; drop
        c->send = (uint8_t *)realloc(c->send, cap);
        c->sendCap = cap;
    }
    c->send[c->sendLen++] = frameLen & 0xFF;
    c->send[c->sendLen++] = frameLen >> 8;
    c->send[c->sendLen++] = type;
    if (len)
        memcpy(c->send + c->sendLen, data, len);
    c->sendLen += len;
}

// Returns -1 on a fatal error
static int Conn_Flush(struct MpConn *c)
{
    while (c->sendLen > 0)
    {
        int n = (int)send(c->sock, (const char *)c->send, c->sendLen, 0);
        if (n < 0)
        {
            int e = mp_lasterr();
            if (MP_WOULDBLOCK(e))
                return 0;
            return -1;
        }
        if (n == 0)
            return 0;
        memmove(c->send, c->send + n, c->sendLen - n);
        c->sendLen -= n;
    }
    return 0;
}

// Returns -1 if the connection closed
static int Conn_Fill(struct MpConn *c)
{
    for (;;)
    {
        int space = RECVBUF_SIZE - c->recvLen;
        int n;
        if (space <= 0)
            return 0;
        n = (int)recv(c->sock, (char *)c->recv + c->recvLen, space, 0);
        if (n > 0)
        {
            c->recvLen += n;
            continue;
        }
        if (n == 0)
            return -1;
        {
            int e = mp_lasterr();
            if (MP_WOULDBLOCK(e))
                return 0;
            return -1;
        }
    }
}

// Pops one frame. Returns 1 if a frame was popped, 0 if none, -1 on garbage.
static int Conn_Pop(struct MpConn *c, uint8_t *type, uint8_t *buf, uint16_t *len)
{
    uint16_t frameLen;
    if (c->recvLen < 3)
        return 0;
    frameLen = c->recv[0] | (c->recv[1] << 8);
    if (frameLen == 0 || frameLen > MP_MAX_MSG + 1)
        return -1;
    if (c->recvLen < 2 + frameLen)
        return 0;
    *type = c->recv[2];
    *len = frameLen - 1;
    if (*len)
        memcpy(buf, c->recv + 3, *len);
    memmove(c->recv, c->recv + 2 + frameLen, c->recvLen - 2 - frameLen);
    c->recvLen -= 2 + frameLen;
    return 1;
}

// ---------------------------------------------------------------------------
// Client

static struct MpConn sClient = { MP_BADSOCK };
static int sClientStatus = MPNET_DISCONNECTED;

int MPNet_Connect(const char *host, int port)
{
    struct addrinfo hints, *res = NULL, *ai;
    char portStr[16];
    int rc;

    MPNet_Init();
    MPNet_ClientClose();
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    snprintf(portStr, sizeof(portStr), "%d", port);
    rc = getaddrinfo(host, portStr, &hints, &res);
    if (rc != 0 || res == NULL)
    {
        snprintf(sLastError, sizeof(sLastError), "could not resolve %s", host);
        return -1;
    }
    for (ai = res; ai; ai = ai->ai_next)
    {
        mp_sock s = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
        if (s == MP_BADSOCK)
            continue;
        SetNonBlocking(s);
        rc = connect(s, ai->ai_addr, (int)ai->ai_addrlen);
        if (rc == 0 || MP_WOULDBLOCK(mp_lasterr()))
        {
            sClient.sock = s;
            sClient.connecting = (rc != 0);
            sClient.recvLen = 0;
            sClient.sendLen = 0;
            sClientStatus = sClient.connecting ? MPNET_CONNECTING : MPNET_CONNECTED;
            freeaddrinfo(res);
            return 0;
        }
        mp_closesock(s);
    }
    freeaddrinfo(res);
    snprintf(sLastError, sizeof(sLastError), "could not connect to %s:%d", host, port);
    return -1;
}

int MPNet_ClientStatus(void)
{
    return sClientStatus;
}

void MPNet_ClientSend(uint8_t type, const void *data, uint16_t len)
{
    if (sClientStatus == MPNET_DISCONNECTED)
        return;
    Conn_Queue(&sClient, type, data, len);
}

void MPNet_ClientClose(void)
{
    Conn_Reset(&sClient);
    sClientStatus = MPNET_DISCONNECTED;
}

static void Client_Fail(const char *why)
{
    snprintf(sLastError, sizeof(sLastError), "%s", why);
    MPNet_Log("connection lost: %s", why);
    MPNet_ClientClose();
}

int MPNet_ClientPoll(uint8_t *type, uint8_t *buf, uint16_t *len)
{
    int rc;

    if (sClientStatus == MPNET_DISCONNECTED)
        return 0;

    if (sClientStatus == MPNET_CONNECTING)
    {
        fd_set wset, eset;
        struct timeval tv = {0, 0};
        int err = 0;
        socklen_t elen = sizeof(err);
        FD_ZERO(&wset);
        FD_ZERO(&eset);
        FD_SET(sClient.sock, &wset);
        FD_SET(sClient.sock, &eset);
        rc = select((int)sClient.sock + 1, NULL, &wset, &eset, &tv);
        if (rc <= 0)
            return 0;
        getsockopt(sClient.sock, SOL_SOCKET, SO_ERROR, (char *)&err, &elen);
        if (err != 0 || FD_ISSET(sClient.sock, &eset))
        {
            Client_Fail("server refused the connection");
            return 0;
        }
        sClientStatus = MPNET_CONNECTED;
        sClient.connecting = 0;
    }

    if (Conn_Flush(&sClient) < 0)
    {
        Client_Fail("send failed");
        return 0;
    }
    rc = Conn_Pop(&sClient, type, buf, len);
    if (rc == 1)
        return 1;
    if (rc < 0)
    {
        Client_Fail("bad data from server");
        return 0;
    }
    if (Conn_Fill(&sClient) < 0)
    {
        // Deliver what we still have buffered before reporting the close
        rc = Conn_Pop(&sClient, type, buf, len);
        if (rc == 1)
            return 1;
        Client_Fail("server closed the connection");
        return 0;
    }
    rc = Conn_Pop(&sClient, type, buf, len);
    if (rc < 0)
    {
        Client_Fail("bad data from server");
        return 0;
    }
    return rc;
}

// ---------------------------------------------------------------------------
// Relay server
//
// The server knows very little about the game: it assigns player ids, caches
// each player's HELLO/STATE/custom sprite so late joiners see everyone, and
// forwards peer-to-peer RELAY messages (used for challenges and battles).

struct MpServerSlot {
    struct MpConn conn;
    int used;
    int helloDone;
    uint8_t hello[MP_HELLO_SIZE];
    uint8_t state[MP_WIRESTATE_SIZE];
    int hasState;
    uint8_t *gfx;
    uint16_t gfxLen;
};

static mp_sock sListen = MP_BADSOCK;
static struct MpServerSlot sSlots[MP_MAX_PLAYERS];
static uint8_t sSrvBuf[MP_MAX_MSG + 8];
static uint8_t sMsgBuf[MP_MAX_MSG + 8];

int MPServer_IsRunning(void)
{
    return sListen != MP_BADSOCK;
}

int MPServer_PlayerCount(void)
{
    int i, n = 0;
    for (i = 0; i < MP_MAX_PLAYERS; i++)
        if (sSlots[i].used && sSlots[i].helloDone)
            n++;
    return n;
}

int MPServer_Start(int port)
{
    struct sockaddr_in6 addr6;
    struct sockaddr_in addr4;
    int one = 1, i;

    MPNet_Init();
    if (sListen != MP_BADSOCK)
        return 0;
    for (i = 0; i < MP_MAX_PLAYERS; i++)
    {
        memset(&sSlots[i], 0, sizeof(sSlots[i]));
        sSlots[i].conn.sock = MP_BADSOCK;
    }

    // Prefer a dual stack IPv6 socket, fall back to IPv4
    sListen = socket(AF_INET6, SOCK_STREAM, 0);
    if (sListen != MP_BADSOCK)
    {
        int zero = 0;
        setsockopt(sListen, SOL_SOCKET, SO_REUSEADDR, (const char *)&one, sizeof(one));
        setsockopt(sListen, IPPROTO_IPV6, IPV6_V6ONLY, (const char *)&zero, sizeof(zero));
        memset(&addr6, 0, sizeof(addr6));
        addr6.sin6_family = AF_INET6;
        addr6.sin6_addr = in6addr_any;
        addr6.sin6_port = htons((uint16_t)port);
        if (bind(sListen, (struct sockaddr *)&addr6, sizeof(addr6)) != 0)
        {
            mp_closesock(sListen);
            sListen = MP_BADSOCK;
        }
    }
    if (sListen == MP_BADSOCK)
    {
        sListen = socket(AF_INET, SOCK_STREAM, 0);
        if (sListen == MP_BADSOCK)
            return -1;
        setsockopt(sListen, SOL_SOCKET, SO_REUSEADDR, (const char *)&one, sizeof(one));
        memset(&addr4, 0, sizeof(addr4));
        addr4.sin_family = AF_INET;
        addr4.sin_addr.s_addr = htonl(INADDR_ANY);
        addr4.sin_port = htons((uint16_t)port);
        if (bind(sListen, (struct sockaddr *)&addr4, sizeof(addr4)) != 0)
        {
            snprintf(sLastError, sizeof(sLastError), "port %d is already in use", port);
            mp_closesock(sListen);
            sListen = MP_BADSOCK;
            return -1;
        }
    }
    if (listen(sListen, 8) != 0)
    {
        mp_closesock(sListen);
        sListen = MP_BADSOCK;
        return -1;
    }
    SetNonBlocking(sListen);
    MPNet_Log("server listening on port %d (up to %d players)", port, MP_MAX_PLAYERS);
    return 0;
}

void MPServer_Stop(void)
{
    int i;
    for (i = 0; i < MP_MAX_PLAYERS; i++)
    {
        Conn_Reset(&sSlots[i].conn);
        free(sSlots[i].conn.send);
        free(sSlots[i].gfx);
        memset(&sSlots[i], 0, sizeof(sSlots[i]));
        sSlots[i].conn.sock = MP_BADSOCK;
    }
    if (sListen != MP_BADSOCK)
        mp_closesock(sListen);
    sListen = MP_BADSOCK;
}

static void Srv_SendTo(int id, uint8_t type, const void *data, uint16_t len)
{
    if (sSlots[id].used)
        Conn_Queue(&sSlots[id].conn, type, data, len);
}

// Sends "u8 fromId + data" to everyone except `except`
static void Srv_BroadcastWithId(int except, uint8_t type, uint8_t fromId, const void *data, uint16_t len)
{
    int i;
    sSrvBuf[0] = fromId;
    memcpy(sSrvBuf + 1, data, len);
    for (i = 0; i < MP_MAX_PLAYERS; i++)
        if (i != except && sSlots[i].used && sSlots[i].helloDone)
            Srv_SendTo(i, type, sSrvBuf, (uint16_t)(len + 1));
}

static void Srv_Drop(int id)
{
    uint8_t b = (uint8_t)id;
    int wasJoined = sSlots[id].helloDone;
    MPNet_Log("player %d left", id);
    Conn_Reset(&sSlots[id].conn);
    free(sSlots[id].gfx);
    sSlots[id].gfx = NULL;
    sSlots[id].gfxLen = 0;
    sSlots[id].used = 0;
    sSlots[id].helloDone = 0;
    sSlots[id].hasState = 0;
    if (wasJoined)
    {
        int i;
        for (i = 0; i < MP_MAX_PLAYERS; i++)
            if (sSlots[i].used && sSlots[i].helloDone)
                Srv_SendTo(i, MPMSG_S_PLAYER_LEAVE, &b, 1);
    }
}

static void Srv_HandleMessage(int id, uint8_t type, uint8_t *data, uint16_t len)
{
    struct MpServerSlot *slot = &sSlots[id];
    int i;

    switch (type)
    {
    case MPMSG_C_HELLO:
    case MPMSG_C_INFO:
        if (len < MP_HELLO_SIZE)
            return;
        if (type == MPMSG_C_HELLO)
        {
            uint16_t ver = data[0] | (data[1] << 8);
            uint8_t welcome[2];
            if (ver != MP_PROTOCOL_VERSION)
            {
                uint8_t reason = MP_REJECT_VERSION;
                Srv_SendTo(id, MPMSG_S_REJECT, &reason, 1);
                Conn_Flush(&slot->conn);
                Srv_Drop(id);
                return;
            }
            welcome[0] = (uint8_t)id;
            welcome[1] = MP_MAX_PLAYERS;
            Srv_SendTo(id, MPMSG_S_WELCOME, welcome, 2);
            // Tell the newcomer about everyone already here
            for (i = 0; i < MP_MAX_PLAYERS; i++)
            {
                if (i == id || !sSlots[i].used || !sSlots[i].helloDone)
                    continue;
                sSrvBuf[0] = (uint8_t)i;
                memcpy(sSrvBuf + 1, sSlots[i].hello, MP_HELLO_SIZE);
                Srv_SendTo(id, MPMSG_S_PLAYER_INFO, sSrvBuf, MP_HELLO_SIZE + 1);
                if (sSlots[i].gfx)
                {
                    sSrvBuf[0] = (uint8_t)i;
                    memcpy(sSrvBuf + 1, sSlots[i].gfx, sSlots[i].gfxLen);
                    Srv_SendTo(id, MPMSG_S_CUSTOM_GFX, sSrvBuf, (uint16_t)(sSlots[i].gfxLen + 1));
                }
                if (sSlots[i].hasState)
                {
                    sSrvBuf[0] = (uint8_t)i;
                    memcpy(sSrvBuf + 1, sSlots[i].state, MP_WIRESTATE_SIZE);
                    Srv_SendTo(id, MPMSG_S_STATE, sSrvBuf, MP_WIRESTATE_SIZE + 1);
                }
            }
            slot->helloDone = 1;
            MPNet_Log("player %d joined", id);
        }
        if (!slot->helloDone)
            return;
        memcpy(slot->hello, data, MP_HELLO_SIZE);
        Srv_BroadcastWithId(id, MPMSG_S_PLAYER_INFO, (uint8_t)id, slot->hello, MP_HELLO_SIZE);
        break;
    case MPMSG_C_STATE:
        if (!slot->helloDone || len < MP_WIRESTATE_SIZE)
            return;
        memcpy(slot->state, data, MP_WIRESTATE_SIZE);
        slot->hasState = 1;
        Srv_BroadcastWithId(id, MPMSG_S_STATE, (uint8_t)id, data, MP_WIRESTATE_SIZE);
        break;
    case MPMSG_C_CUSTOM_GFX:
        if (!slot->helloDone || len == 0 || len > MP_MAX_MSG - 8)
            return;
        free(slot->gfx);
        slot->gfx = (uint8_t *)malloc(len);
        memcpy(slot->gfx, data, len);
        slot->gfxLen = len;
        Srv_BroadcastWithId(id, MPMSG_S_CUSTOM_GFX, (uint8_t)id, data, len);
        break;
    case MPMSG_C_RELAY:
    {
        uint8_t target;
        if (!slot->helloDone || len < 2)
            return;
        target = data[0];
        if (target >= MP_MAX_PLAYERS || !sSlots[target].used || !sSlots[target].helloDone)
            return;
        sSrvBuf[0] = (uint8_t)id;
        memcpy(sSrvBuf + 1, data + 1, len - 1);
        Srv_SendTo(target, MPMSG_S_RELAY, sSrvBuf, len);
        break;
    }
    case MPMSG_C_PING:
        Srv_SendTo(id, MPMSG_S_PONG, data, len);
        break;
    }
}

void MPServer_Poll(void)
{
    int i;
    uint8_t type;
    uint16_t len;

    if (sListen == MP_BADSOCK)
        return;

    // Accept new players
    for (;;)
    {
        mp_sock s = accept(sListen, NULL, NULL);
        if (s == MP_BADSOCK)
            break;
        for (i = 0; i < MP_MAX_PLAYERS; i++)
            if (!sSlots[i].used)
                break;
        SetNonBlocking(s);
        if (i == MP_MAX_PLAYERS)
        {
            struct MpConn tmp;
            uint8_t reason = MP_REJECT_FULL;
            memset(&tmp, 0, sizeof(tmp));
            tmp.sock = s;
            Conn_Queue(&tmp, MPMSG_S_REJECT, &reason, 1);
            Conn_Flush(&tmp);
            mp_closesock(s);
            free(tmp.send);
            MPNet_Log("rejected a player: server full");
            continue;
        }
        sSlots[i].used = 1;
        sSlots[i].helloDone = 0;
        sSlots[i].hasState = 0;
        sSlots[i].conn.sock = s;
        sSlots[i].conn.recvLen = 0;
        sSlots[i].conn.sendLen = 0;
        MPNet_Log("connection accepted into slot %d", i);
    }

    for (i = 0; i < MP_MAX_PLAYERS; i++)
    {
        int rc;
        if (!sSlots[i].used)
            continue;
        if (Conn_Fill(&sSlots[i].conn) < 0)
        {
            while (sSlots[i].used && Conn_Pop(&sSlots[i].conn, &type, sMsgBuf, &len) == 1)
                Srv_HandleMessage(i, type, sMsgBuf, len);
            Srv_Drop(i);
            continue;
        }
        for (;;)
        {
            rc = Conn_Pop(&sSlots[i].conn, &type, sMsgBuf, &len);
            if (rc != 1)
                break;
            Srv_HandleMessage(i, type, sMsgBuf, len);
            if (!sSlots[i].used)
                break;
        }
        if (rc < 0 && sSlots[i].used)
            Srv_Drop(i);
    }

    for (i = 0; i < MP_MAX_PLAYERS; i++)
        if (sSlots[i].used && Conn_Flush(&sSlots[i].conn) < 0)
            Srv_Drop(i);
}
