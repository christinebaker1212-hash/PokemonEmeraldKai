// Moemon Co-op: the phone link's sockets (mp_phone_net.c), plain types only: mp_phone.c can't include socket headers.
#ifndef GUARD_MP_PHONE_NET_H
#define GUARD_MP_PHONE_NET_H

#include <stddef.h>
#include <stdint.h>

#define PHONE_NET_TCP   2   // a save remembers two phones: on the local network...
#define PHONE_NET_CONNS 4   // ...and through the co-op server (pipes: mp_phone_friends.c)

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

// Pipes: a phone reaching the game through the co-op server (docs/PHONE_LINK.md "Phase 4"). The server's channel
// becomes a connection like the others; whole frames in, whole frames out (through PhoneNet_PipeOut)
extern void (*gPhonePipeOut)(uint16_t channel, const uint8_t *body, size_t len);
extern void (*gPhonePipeClose)(uint16_t channel);
int PhoneNet_PipeOpen(uint16_t channel);                                  // its connection, -1 if none is free
void PhoneNet_PipeIn(uint16_t channel, const uint8_t *body, size_t len);  // a whole frame from the phone
void PhoneNet_PipeClosed(uint16_t channel);

#endif // GUARD_MP_PHONE_NET_H
