// Moemon Co-op: socket portability helpers shared by the client and the server.
// Internal to src/mp/mp_net.c and src/mp/mp_server.c.
#ifndef GUARD_MP_SOCK_H
#define GUARD_MP_SOCK_H

#ifdef _WIN32
#ifndef FD_SETSIZE
#define FD_SETSIZE 1024
#endif
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
typedef SOCKET mp_sock;
#define MP_BADSOCK INVALID_SOCKET
#define mp_closesock closesocket
#define MP_WOULDBLOCK(e) ((e) == WSAEWOULDBLOCK || (e) == WSAEINPROGRESS || (e) == WSAEALREADY)
static inline int mp_lasterr(void) { return WSAGetLastError(); }
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
static inline int mp_lasterr(void) { return errno; }
#endif

#include <stdint.h>
#include "mp_net.h"

#ifdef _WIN32
typedef int mp_socklen;
#else
typedef socklen_t mp_socklen;
#endif

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


void MPSock_SetNonBlocking(mp_sock s);
void MPConn_Reset(struct MpConn *c);
void MPConn_Queue(struct MpConn *c, uint8_t type, const void *data, uint16_t len);
int MPConn_Flush(struct MpConn *c);   // -1 on a fatal error
int MPConn_Fill(struct MpConn *c);    // -1 when the connection closed
int MPConn_Pop(struct MpConn *c, uint8_t *type, uint8_t *buf, uint16_t *len); // 1 popped, 0 none, -1 garbage

#endif // GUARD_MP_SOCK_H
