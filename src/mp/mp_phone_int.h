// Moemon Co-op: the phone link's insides shared by its files (mp_phone.c: the link, the snapshot and orders;
// mp_phone_dev.c: the device's apps: the live map, MOENAV calls, the battle feed, the POKéGEAR)
#ifndef GUARD_MP_PHONE_INT_H
#define GUARD_MP_PHONE_INT_H
#include <stddef.h>

struct Buf
{
    char *p;
    size_t len, cap;
};

extern struct Buf sOut;   // (the one message being written)

void BufReset(struct Buf *b);
void BufAdd(struct Buf *b, const char *fmt, ...) __attribute__((format(printf, 2, 3)));
void BufText(struct Buf *b, const u8 *s, size_t max);
void BufBoxMon(struct Buf *b, struct BoxPokemon *box);
void BufPartyMon(struct Buf *b, struct Pokemon *mon);
bool8 JsonNum(const char *json, const char *key, long long *out);
bool8 JsonStr(const char *json, const char *key, char *out, size_t size);
void SendBuf(int c, struct Buf *b);
struct Pokemon *OwnParty(void);
bool8 Safe(void);

// mp_phone_dev.c
void PhoneDev_Place(struct Buf *b);                     // the snapshot's place: the town map spot, friends on it
bool8 PhoneDev_OnSealed(int c, const char *t, const char *json);   // TRUE: the message was the device's
void PhoneDev_Update(int c, bool8 live);                // every frame, per connection: the battle feed
void PhoneDev_Closed(int c);
#endif
