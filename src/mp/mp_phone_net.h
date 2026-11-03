// Moemon Co-op: the phone link's sockets (mp_phone_net.c), plain types only: mp_phone.c can't include socket headers.
#ifndef GUARD_MP_PHONE_NET_H
#define GUARD_MP_PHONE_NET_H

#include <stddef.h>
#include <stdint.h>

#define PHONE_NET_CONNS 2   // a save remembers two phones

int PhoneNet_Listen(int port);              // the TCP port listened on, -1 if none could be
void PhoneNet_Stop(void);
void PhoneNet_Poll(const char *udpAnswer);  // accept, send, receive; udpAnswer: what "where are you?" gets back
int PhoneNet_NewConn(void);                 // a connection just accepted, -1 if none
int PhoneNet_IsOpen(int c);
int PhoneNet_Recv(int c, const uint8_t **frame, size_t *len);   // 1: a whole frame (good until the next call)
void PhoneNet_Send(int c, const uint8_t *body, size_t len);
size_t PhoneNet_Pending(int c);
void PhoneNet_Close(int c);
void PhoneNet_LocalAddress(char *out, size_t size);

#endif // GUARD_MP_PHONE_NET_H
