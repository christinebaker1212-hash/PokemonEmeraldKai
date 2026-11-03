// Moemon Co-op: the phone link's sockets (docs/PHONE_LINK.md). No game headers here (winsock and the game's headers
// don't mix): src/mp/mp_phone.c does the game's side through these few functions.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mp_sock.h"
#include "mp_phone_net.h"
#include "phone_proto.h"

struct PhoneConn
{
    mp_sock sock;
    uint8_t *in;
    size_t inLen, inCap;
    uint8_t *out;
    size_t outLen, outCap;
    size_t frameDone;      // bytes of the frame handed out last, dropped at the next Recv
    int fresh;
    uint16_t pipe;         // a pipe through the co-op server (its channel; 0: a socket)
};

void (*gPhonePipeOut)(uint16_t channel, const uint8_t *body, size_t len);
void (*gPhonePipeClose)(uint16_t channel);

static mp_sock sListen = MP_BADSOCK, sUdp = MP_BADSOCK;
static int sPort;
static struct PhoneConn sConns[PHONE_NET_CONNS];

static void Drop(struct PhoneConn *c)
{
    if (c->pipe && gPhonePipeClose)
        gPhonePipeClose(c->pipe);
    c->pipe = 0;
    if (c->sock != MP_BADSOCK)
        mp_closesock(c->sock);
    c->sock = MP_BADSOCK;
    c->inLen = c->outLen = c->frameDone = 0;
    c->fresh = 0;
}

int PhoneNet_Listen(int port)
{
    struct sockaddr_in a;
    int tries, one = 1, i;
    if (sListen != MP_BADSOCK)
        return sPort;
    if (MPNet_Init() != 0)
        return -1;
    for (i = 0; i < PHONE_NET_CONNS; i++)
        sConns[i].sock = MP_BADSOCK;
    for (tries = 0; tries < 16; tries++)
    {
        mp_sock s = socket(AF_INET, SOCK_STREAM, 0);
        if (s == MP_BADSOCK)
            return -1;
#ifndef _WIN32
        setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char *)&one, sizeof(one));
#endif
        memset(&a, 0, sizeof(a));
        a.sin_family = AF_INET;
        a.sin_addr.s_addr = htonl(INADDR_ANY);
        a.sin_port = htons((uint16_t)(port + tries));
        if (bind(s, (struct sockaddr *)&a, sizeof(a)) == 0 && listen(s, 4) == 0)
        {
            MPSock_SetNonBlocking(s);
            sListen = s;
            sPort = port + tries;
            break;
        }
        mp_closesock(s);
    }
    if (sListen == MP_BADSOCK)
        return -1;
    // The "where are you?" answers (the phone broadcasts on PHONE_UDP_PORT; a second game on this PC can't have it too)
    {
        mp_sock u = socket(AF_INET, SOCK_DGRAM, 0);
        if (u != MP_BADSOCK)
        {
            memset(&a, 0, sizeof(a));
            a.sin_family = AF_INET;
            a.sin_addr.s_addr = htonl(INADDR_ANY);
            a.sin_port = htons(PHONE_UDP_PORT);
            if (bind(u, (struct sockaddr *)&a, sizeof(a)) == 0)
            {
                MPSock_SetNonBlocking(u);
                sUdp = u;
            }
            else
                mp_closesock(u);
        }
    }
    return sPort;
}

void PhoneNet_Stop(void)
{
    int i;
    for (i = 0; i < PHONE_NET_CONNS; i++)
        Drop(&sConns[i]);
    if (sListen != MP_BADSOCK)
        mp_closesock(sListen);
    if (sUdp != MP_BADSOCK)
        mp_closesock(sUdp);
    sListen = sUdp = MP_BADSOCK;
}

static int Grow(uint8_t **buf, size_t *cap, size_t need)
{
    if (need <= *cap)
        return 1;
    {
        size_t n = *cap ? *cap : 4096;
        uint8_t *p;
        while (n < need)
            n *= 2;
        p = (uint8_t *)realloc(*buf, n);
        if (p == NULL)
            return 0;
        *buf = p;
        *cap = n;
    }
    return 1;
}

void PhoneNet_Poll(const char *udpAnswer)
{
    int i;
    if (sListen == MP_BADSOCK)
        return;
    for (;;)
    {
        mp_sock s = accept(sListen, NULL, NULL);
        if (s == MP_BADSOCK)
            break;
        for (i = 0; i < PHONE_NET_TCP && sConns[i].sock != MP_BADSOCK; i++)
            ;
        if (i == PHONE_NET_TCP)
        {
            mp_closesock(s);   // (two phones at most)
            continue;
        }
        MPSock_SetNonBlocking(s);
        {
            int one = 1;
            setsockopt(s, IPPROTO_TCP, TCP_NODELAY, (const char *)&one, sizeof(one));
        }
        sConns[i].sock = s;
        sConns[i].inLen = sConns[i].outLen = sConns[i].frameDone = 0;
        sConns[i].fresh = 1;
    }
    for (i = 0; i < PHONE_NET_CONNS; i++)
    {
        struct PhoneConn *c = &sConns[i];
        if (c->sock == MP_BADSOCK)
            continue;
        if (c->outLen)
        {
            int n = send(c->sock, (const char *)c->out, (int)c->outLen, 0);
            if (n > 0)
            {
                memmove(c->out, c->out + n, c->outLen - n);
                c->outLen -= n;
            }
            else if (n < 0 && !MP_WOULDBLOCK(mp_lasterr()))
            {
                Drop(c);
                continue;
            }
        }
        for (;;)
        {
            int n;
            if (!Grow(&c->in, &c->inCap, c->inLen + 65536))
            {
                Drop(c);
                break;
            }
            n = recv(c->sock, (char *)c->in + c->inLen, 65536, 0);
            if (n > 0)
                c->inLen += n;
            else
            {
                if (n == 0 || !MP_WOULDBLOCK(mp_lasterr()))
                    Drop(c);
                break;
            }
        }
    }
    if (sUdp != MP_BADSOCK && udpAnswer != NULL)
    {
        uint8_t buf[256];
        struct sockaddr_in from;
        for (;;)
        {
            mp_socklen fl = sizeof(from);
            int n = recvfrom(sUdp, (char *)buf, sizeof(buf) - 1, 0, (struct sockaddr *)&from, &fl);
            if (n <= 0)
                break;
            buf[n] = '\0';
            if (n >= 9 && memcmp(buf, "MOEPHONE?", 9) == 0)
                sendto(sUdp, udpAnswer, (int)strlen(udpAnswer), 0, (struct sockaddr *)&from, fl);
        }
    }
}

int PhoneNet_NewConn(void)
{
    int i;
    for (i = 0; i < PHONE_NET_CONNS; i++)
        if ((sConns[i].sock != MP_BADSOCK || sConns[i].pipe) && sConns[i].fresh)
        {
            sConns[i].fresh = 0;
            return i;
        }
    return -1;
}

int PhoneNet_IsOpen(int c)
{
    return c >= 0 && c < PHONE_NET_CONNS && (sConns[c].sock != MP_BADSOCK || sConns[c].pipe != 0);
}

int PhoneNet_Recv(int ci, const uint8_t **frame, size_t *len)
{
    struct PhoneConn *c = &sConns[ci];
    size_t n;
    if (!PhoneNet_IsOpen(ci))
        return -1;
    if (c->frameDone)
    {
        memmove(c->in, c->in + c->frameDone, c->inLen - c->frameDone);
        c->inLen -= c->frameDone;
        c->frameDone = 0;
    }
    if (c->inLen < 4)
        return 0;
    n = ((size_t)c->in[0] << 24) | ((size_t)c->in[1] << 16) | ((size_t)c->in[2] << 8) | c->in[3];
    if (n == 0 || n > PHONE_FRAME_MAX)
    {
        Drop(c);
        return -1;
    }
    if (c->inLen < 4 + n)
        return 0;
    *frame = c->in + 4;
    *len = n;
    c->frameDone = 4 + n;
    return 1;
}

void PhoneNet_Send(int ci, const uint8_t *body, size_t len)
{
    struct PhoneConn *c = &sConns[ci];
    if (PhoneNet_IsOpen(ci) && c->pipe)
    {
        if (gPhonePipeOut)
            gPhonePipeOut(c->pipe, body, len);
        return;
    }
    if (!PhoneNet_IsOpen(ci) || !Grow(&c->out, &c->outCap, c->outLen + 4 + len))
        return;
    c->out[c->outLen++] = (uint8_t)(len >> 24);
    c->out[c->outLen++] = (uint8_t)(len >> 16);
    c->out[c->outLen++] = (uint8_t)(len >> 8);
    c->out[c->outLen++] = (uint8_t)len;
    memcpy(c->out + c->outLen, body, len);
    c->outLen += len;
}

size_t PhoneNet_Pending(int ci)
{
    return PhoneNet_IsOpen(ci) ? sConns[ci].outLen : 0;
}

void PhoneNet_Close(int ci)
{
    if (ci >= 0 && ci < PHONE_NET_CONNS)
        Drop(&sConns[ci]);
}

// This PC's address on the local network, for the QR code: the route a packet to the internet would take (nothing is
// sent: a UDP socket's connect only picks the route), else 127.0.0.1
void PhoneNet_LocalAddress(char *out, size_t size)
{
    struct sockaddr_in a, me;
    mp_socklen ml = sizeof(me);
    mp_sock s;
    snprintf(out, size, "127.0.0.1");
    if (MPNet_Init() != 0)
        return;
    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s == MP_BADSOCK)
        return;
    memset(&a, 0, sizeof(a));
    a.sin_family = AF_INET;
    a.sin_port = htons(53);
    a.sin_addr.s_addr = htonl(0x08080808);
    if (connect(s, (struct sockaddr *)&a, sizeof(a)) == 0 && getsockname(s, (struct sockaddr *)&me, &ml) == 0)
    {
        uint32_t ip = ntohl(me.sin_addr.s_addr);
        if (ip != 0)
            snprintf(out, size, "%u.%u.%u.%u", (ip >> 24) & 255, (ip >> 16) & 255, (ip >> 8) & 255, ip & 255);
    }
    mp_closesock(s);
}

// ------------------------------------------------------------------------------------------------------- pipes

int PhoneNet_PipeOpen(uint16_t channel)
{
    int i;
    for (i = PHONE_NET_TCP; i < PHONE_NET_CONNS; i++)
        if (sConns[i].sock == MP_BADSOCK && !sConns[i].pipe)
        {
            sConns[i].sock = MP_BADSOCK;
            sConns[i].pipe = channel;
            sConns[i].inLen = sConns[i].outLen = sConns[i].frameDone = 0;
            sConns[i].fresh = 1;
            return i;
        }
    return -1;
}

void PhoneNet_PipeIn(uint16_t channel, const uint8_t *body, size_t len)
{
    int i;
    for (i = PHONE_NET_TCP; i < PHONE_NET_CONNS; i++)
    {
        struct PhoneConn *c = &sConns[i];
        if (c->pipe != channel || len == 0 || len > PHONE_FRAME_MAX || !Grow(&c->in, &c->inCap, c->inLen + 4 + len))
            continue;
        // (as a socket's: the 4-byte length, then the frame, so PhoneNet_Recv takes it the same way)
        c->in[c->inLen++] = (uint8_t)(len >> 24);
        c->in[c->inLen++] = (uint8_t)(len >> 16);
        c->in[c->inLen++] = (uint8_t)(len >> 8);
        c->in[c->inLen++] = (uint8_t)len;
        memcpy(c->in + c->inLen, body, len);
        c->inLen += len;
    }
}

void PhoneNet_PipeClosed(uint16_t channel)
{
    int i;
    for (i = PHONE_NET_TCP; i < PHONE_NET_CONNS; i++)
        if (sConns[i].pipe == channel)
        {
            sConns[i].pipe = 0;   // (closed by the server: nothing to tell it)
            Drop(&sConns[i]);
        }
}
