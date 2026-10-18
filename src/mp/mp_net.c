// Moemon Co-op: TCP transport, message framing, client, LAN discovery.
// Frame format: u16 length (little endian, counts type + payload), u8 type, payload.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>

#include "mp_sock.h"
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

void MPSock_SetNonBlocking(mp_sock s)
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
        setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, (const char *)&one, sizeof(one));
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

void MPConn_Reset(struct MpConn *c)
{
    if (c->sock != MP_BADSOCK)
        mp_closesock(c->sock);
    c->sock = MP_BADSOCK;
    c->connecting = 0;
    c->recvLen = 0;
    c->sendLen = 0;
}

void MPConn_Queue(struct MpConn *c, uint8_t type, const void *data, uint16_t len)
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
int MPConn_Flush(struct MpConn *c)
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
int MPConn_Fill(struct MpConn *c)
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
int MPConn_Pop(struct MpConn *c, uint8_t *type, uint8_t *buf, uint16_t *len)
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
        MPSock_SetNonBlocking(s);
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
    MPConn_Queue(&sClient, type, data, len);
}

void MPNet_ClientClose(void)
{
    MPConn_Reset(&sClient);
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

    if (MPConn_Flush(&sClient) < 0)
    {
        Client_Fail("send failed");
        return 0;
    }
    rc = MPConn_Pop(&sClient, type, buf, len);
    if (rc == 1)
        return 1;
    if (rc < 0)
    {
        Client_Fail("bad data from server");
        return 0;
    }
    if (MPConn_Fill(&sClient) < 0)
    {
        // Deliver what we still have buffered before reporting the close
        rc = MPConn_Pop(&sClient, type, buf, len);
        if (rc == 1)
            return 1;
        Client_Fail("server closed the connection");
        return 0;
    }
    rc = MPConn_Pop(&sClient, type, buf, len);
    if (rc < 0)
    {
        Client_Fail("bad data from server");
        return 0;
    }
    return rc;
}


// ---------------------------------------------------------------------------
// Addresses

void MPNet_ParseAddress(const char *addr, char *host, int hostLen, int *port)
{
    const char *colon;
    int n;

    while (*addr == ' ')
        addr++;
    if (addr[0] == '[')
    {
        const char *end = strchr(addr, ']');
        if (end)
        {
            n = (int)(end - addr - 1);
            if (n >= hostLen)
                n = hostLen - 1;
            memcpy(host, addr + 1, n);
            host[n] = 0;
            if (end[1] == ':' && atoi(end + 2) > 0)
                *port = atoi(end + 2);
            return;
        }
    }
    colon = strchr(addr, ':');
    // More than one ':' means a bare IPv6 address without a port
    if (colon && strchr(colon + 1, ':') == NULL)
    {
        n = (int)(colon - addr);
        if (n >= hostLen)
            n = hostLen - 1;
        memcpy(host, addr, n);
        host[n] = 0;
        if (atoi(colon + 1) > 0)
            *port = atoi(colon + 1);
        return;
    }
    snprintf(host, hostLen, "%s", addr);
}

// ---------------------------------------------------------------------------
// SHA-256 (public domain style implementation, FIPS 180-4)

static const uint32_t sK[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2,
};

#define ROR(x, n) (((x) >> (n)) | ((x) << (32 - (n))))

static void Sha256Block(uint32_t h[8], const uint8_t *p)
{
    uint32_t w[64], a, b, c, d, e, f, g, hh;
    int i;
    for (i = 0; i < 16; i++)
        w[i] = ((uint32_t)p[i * 4] << 24) | ((uint32_t)p[i * 4 + 1] << 16) | ((uint32_t)p[i * 4 + 2] << 8) | p[i * 4 + 3];
    for (i = 16; i < 64; i++)
    {
        uint32_t s0 = ROR(w[i - 15], 7) ^ ROR(w[i - 15], 18) ^ (w[i - 15] >> 3);
        uint32_t s1 = ROR(w[i - 2], 17) ^ ROR(w[i - 2], 19) ^ (w[i - 2] >> 10);
        w[i] = w[i - 16] + s0 + w[i - 7] + s1;
    }
    a = h[0]; b = h[1]; c = h[2]; d = h[3]; e = h[4]; f = h[5]; g = h[6]; hh = h[7];
    for (i = 0; i < 64; i++)
    {
        uint32_t S1 = ROR(e, 6) ^ ROR(e, 11) ^ ROR(e, 25);
        uint32_t ch = (e & f) ^ (~e & g);
        uint32_t t1 = hh + S1 + ch + sK[i] + w[i];
        uint32_t S0 = ROR(a, 2) ^ ROR(a, 13) ^ ROR(a, 22);
        uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
        uint32_t t2 = S0 + maj;
        hh = g; g = f; f = e; e = d + t1; d = c; c = b; b = a; a = t1 + t2;
    }
    h[0] += a; h[1] += b; h[2] += c; h[3] += d; h[4] += e; h[5] += f; h[6] += g; h[7] += hh;
}

void MPNet_Sha256(const void *data, int len, uint8_t out[32])
{
    uint32_t h[8] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
    const uint8_t *p = (const uint8_t *)data;
    uint8_t tail[128];
    uint64_t bits = (uint64_t)len * 8;
    int i, rem, tailLen;

    while (len >= 64)
    {
        Sha256Block(h, p);
        p += 64;
        len -= 64;
    }
    rem = len;
    memset(tail, 0, sizeof(tail));
    memcpy(tail, p, rem);
    tail[rem] = 0x80;
    tailLen = (rem < 56) ? 64 : 128;
    for (i = 0; i < 8; i++)
        tail[tailLen - 1 - i] = (uint8_t)(bits >> (i * 8));
    Sha256Block(h, tail);
    if (tailLen == 128)
        Sha256Block(h, tail + 64);
    for (i = 0; i < 8; i++)
    {
        out[i * 4] = h[i] >> 24;
        out[i * 4 + 1] = h[i] >> 16;
        out[i * 4 + 2] = h[i] >> 8;
        out[i * 4 + 3] = h[i];
    }
}

// ---------------------------------------------------------------------------
// Rooms

static int IsRoomChar(char c)
{
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')
        || c == ' ' || c == '-' || c == '.' || c == '!' || c == '?' || c == '\'' || c == '&';
}

int MPNet_IsValidRoomName(const char *name)
{
    int i, len = (int)strlen(name), letters = 0;
    if (len < 1 || len > MP_ROOM_NAME_MAX || name[0] == ' ' || name[len - 1] == ' ')
        return 0;
    for (i = 0; i < len; i++)
    {
        if (!IsRoomChar(name[i]))
            return 0;
        if (name[i] != ' ')
            letters++;
    }
    return letters > 0;
}

void MPNet_RoomKey(const char *roomName, const char *password, uint8_t out[MP_ROOM_KEY_SIZE])
{
    char buf[MP_ROOM_NAME_MAX + 2 + 128];
    int i, n = 0;
    for (i = 0; roomName[i] && n < MP_ROOM_NAME_MAX; i++)
    {
        char c = roomName[i];
        buf[n++] = (c >= 'A' && c <= 'Z') ? (char)(c - 'A' + 'a') : c;
    }
    buf[n++] = '\n';
    for (i = 0; password[i] && i < 120; i++)
        buf[n++] = password[i];
    MPNet_Sha256(buf, n, out);
}

// ---------------------------------------------------------------------------
// Local network discovery (client side).
// Probe:  "MOEMON?" u16 version
// Answer: "MOEMON!" u16 version, u16 tcpPort, u8 count, rooms (see MPMSG_S_ROOM_LIST)

#ifdef _WIN32
#include <iphlpapi.h>
#else
#include <ifaddrs.h>
#include <net/if.h>
#endif

static mp_sock sLanSock = MP_BADSOCK;
static struct MpLanRoom sLanRooms[MP_LAN_MAX_GAMES];
static int sLanCount;

static void LanSendTo(uint32_t addrHostOrder, const uint8_t *probe, int len)
{
    struct sockaddr_in to;
    memset(&to, 0, sizeof(to));
    to.sin_family = AF_INET;
    to.sin_port = htons(MP_DISCOVERY_PORT);
    to.sin_addr.s_addr = htonl(addrHostOrder);
    sendto(sLanSock, (const char *)probe, len, 0, (struct sockaddr *)&to, sizeof(to));
}

void MPLan_StartSearch(void)
{
    uint8_t probe[9] = {'M', 'O', 'E', 'M', 'O', 'N', '?', MP_PROTOCOL_VERSION & 0xFF, MP_PROTOCOL_VERSION >> 8};
    int one = 1;

    MPNet_Init();
    if (sLanSock == MP_BADSOCK)
    {
        sLanSock = socket(AF_INET, SOCK_DGRAM, 0);
        if (sLanSock == MP_BADSOCK)
            return;
        setsockopt(sLanSock, SOL_SOCKET, SO_BROADCAST, (const char *)&one, sizeof(one));
        MPSock_SetNonBlocking(sLanSock);
    }
    sLanCount = 0;
    LanSendTo(0xFFFFFFFF, probe, sizeof(probe));       // limited broadcast
    LanSendTo(0x7F000001, probe, sizeof(probe));       // a game hosting on this same PC

    // Directed broadcast on every network adapter (Windows only sends the
    // limited broadcast through one adapter)
#ifdef _WIN32
    {
        ULONG size = 16 * 1024;
        IP_ADAPTER_ADDRESSES *list = (IP_ADAPTER_ADDRESSES *)malloc(size);
        if (list && GetAdaptersAddresses(AF_INET, GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER, NULL, list, &size) == NO_ERROR)
        {
            IP_ADAPTER_ADDRESSES *a;
            for (a = list; a; a = a->Next)
            {
                IP_ADAPTER_UNICAST_ADDRESS *u;
                if (a->OperStatus != IfOperStatusUp)
                    continue;
                for (u = a->FirstUnicastAddress; u; u = u->Next)
                {
                    struct sockaddr_in *sin = (struct sockaddr_in *)u->Address.lpSockaddr;
                    uint32_t ip, mask;
                    if (sin->sin_family != AF_INET || u->OnLinkPrefixLength == 0 || u->OnLinkPrefixLength > 30)
                        continue;
                    ip = ntohl(sin->sin_addr.s_addr);
                    mask = 0xFFFFFFFFu << (32 - u->OnLinkPrefixLength);
                    LanSendTo(ip | ~mask, probe, sizeof(probe));
                }
            }
        }
        free(list);
    }
#else
    {
        struct ifaddrs *ifs = NULL, *it;
        if (getifaddrs(&ifs) == 0)
        {
            for (it = ifs; it; it = it->ifa_next)
            {
                if (!it->ifa_addr || it->ifa_addr->sa_family != AF_INET || !(it->ifa_flags & IFF_BROADCAST) || !it->ifa_broadaddr)
                    continue;
                LanSendTo(ntohl(((struct sockaddr_in *)it->ifa_broadaddr)->sin_addr.s_addr), probe, sizeof(probe));
            }
            freeifaddrs(ifs);
        }
    }
#endif
}

static void LanAddRoom(const char *host, int port, const char *name, uint8_t players, uint8_t maxPlayers, uint8_t flags, uint16_t version)
{
    int i;
    struct MpLanRoom *r;
    for (i = 0; i < sLanCount; i++)
    {
        // Same game seen through two adapters (or loopback + LAN address)
        if (strcmp(sLanRooms[i].name, name) == 0 && sLanRooms[i].port == port)
        {
            if (strcmp(host, "127.0.0.1") == 0)
                snprintf(sLanRooms[i].host, sizeof(sLanRooms[i].host), "%s", host);
            return;
        }
    }
    if (sLanCount >= MP_LAN_MAX_GAMES)
        return;
    r = &sLanRooms[sLanCount++];
    snprintf(r->name, sizeof(r->name), "%s", name);
    snprintf(r->host, sizeof(r->host), "%s", host);
    r->port = port;
    r->players = players;
    r->maxPlayers = maxPlayers;
    r->flags = flags;
    r->version = version;
}

int MPLan_Poll(struct MpLanRoom *out, int max)
{
    uint8_t buf[1500];
    int guard, i;

    for (guard = 0; sLanSock != MP_BADSOCK && guard < 64; guard++)
    {
        struct sockaddr_in from;
        mp_socklen fromLen = sizeof(from);
        int n = (int)recvfrom(sLanSock, (char *)buf, sizeof(buf), 0, (struct sockaddr *)&from, &fromLen);
        int pos, count, port;
        char host[64];
        if (n <= 0)
            break;
        if (n < 12 || memcmp(buf, "MOEMON!", 7) != 0)
            continue;
        port = buf[9] | (buf[10] << 8);
        count = buf[11];
        inet_ntop(AF_INET, &from.sin_addr, host, sizeof(host));
        pos = 12;
        for (i = 0; i < count; i++)
        {
            char name[MP_ROOM_NAME_MAX + 1];
            int len;
            if (pos >= n)
                break;
            len = buf[pos++];
            if (len > MP_ROOM_NAME_MAX || pos + len + 5 > n)
                break;
            memcpy(name, buf + pos, len);
            name[len] = 0;
            pos += len;
            LanAddRoom(host, port, name, buf[pos], buf[pos + 1], buf[pos + 2], buf[pos + 3] | (buf[pos + 4] << 8));
            pos += 5;
        }
    }
    for (i = 0; i < sLanCount && i < max; i++)
        out[i] = sLanRooms[i];
    return i;
}

void MPLan_StopSearch(void)
{
    if (sLanSock != MP_BADSOCK)
        mp_closesock(sLanSock);
    sLanSock = MP_BADSOCK;
    sLanCount = 0;
}
