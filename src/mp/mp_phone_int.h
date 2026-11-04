// Moemon Co-op: the phone link's insides shared by its files (mp_phone.c: the link, the snapshot and orders;
// orders' places
struct Buf;
enum { LOC_NONE, LOC_PARTY, LOC_BOX };
struct Loc { u8 kind, box, slot; };
bool8 ParseLoc(const char *json, const char *key, struct Loc *loc);
struct BoxPokemon *BoxAt(const struct Loc *loc);
bool8 Empty(const struct Loc *loc);
bool8 Is(const struct Loc *loc, u32 pid, u32 ot);
u32 NonEggsInParty(void);
void Changed(const struct Loc *loc);
void PartyChanged(void);
extern bool8 sBagChanged;
struct PokemonStorage *Storage(void);
u32 Rev(void);
extern struct Buf sDoneExtra;                 // more for the next fresh "done" (a walk, a homecoming, gains)
const u8 *PhoneOrderPhone(void);              // the phone whose order is being carried out (its 16-byte ID)

// mp_phone_walk.c: the walk, training, the Garden's kind acts
const char *PhoneWalk_Apply(const char *op, const char *json);   // "walk", "home", "train", "untrain", "collect", "kind"
void PhoneWalk_Snapshot(struct Buf *b, const u8 *phone);         // the snapshot's "walk", "train" (for that phone)
void PhoneWalk_Load(void);                                       // a save loaded
void PhoneWalk_Update(void);                                     // every frame
void PhoneWalk_ForgetAll(void);                                  // FORGET PHONES: a walking POKéMON comes home as it left
bool8 PhoneWalk_OnSealed(int c, const char *t, const char *json);

// mp_phone_friends.c: friends' keys, presence, the phones' pipes through the co-op server, invites
void PhoneFriends_Load(void);
void PhoneFriends_Update(void);
void PhoneFriends_Snapshot(struct Buf *b);
const char *PhoneFriends_Apply(const char *op, const char *json);   // "share", "join"
const u8 *MP_Phone_SaveId(void);
void PhoneFriends_Mail(u8 op, const u8 key[16], const u8 *secret16, u32 ttl, const u8 *data, u16 n);
bool8 PhoneFriends_PairKey(const u8 id[8], u8 out[32]);
const u8 *PhoneFriends_Name(const u8 id[8]);
void PhoneFriends_Inbox(u8 key[16], u8 owner[16]);
void PhoneFriends_TradeKey(u8 out[32]);
bool8 PhoneFriends_Loaded(void);

// mp_phone_trade.c: trades by mail (offers, deposits in the server's escrow, the trade scene)
void PhoneTrade_Load(void);                                      // a save loaded
void PhoneTrade_Update(void);                                    // every frame
void PhoneTrade_OnServer(const u8 *data, u16 len);               // MPMSG_S_MAIL answers (the escrow's, the inbox's)
void PhoneTrade_Snapshot(struct Buf *b);                         // the snapshot's "trades"
const char *PhoneTrade_Apply(const char *op, const char *json);  // "offer", "accept", "untrade"
bool8 MP_Phone_OnOffer(u32 pid, u32 ot);
u32 PhoneTrade_Hash(void);                                       // (the snapshot's rev: the trades' states)

// mp_phone_cards.c: the card games on the phone (TRIPLE TRIAD's collection, decks, prizes, packs)
void PhoneCards_Load(void);
void PhoneCards_Snapshot(struct Buf *b);                         // the snapshot's "tt" and "tcg"
u32 PhoneCards_Hash(void);                                       // (the snapshot's rev: what "tt" shows)
const char *PhoneCards_Apply(const char *op, const char *json);  // "ttdeck", "ttwin", "ttopen", "tcgdeck", "tcgduel", "tcgopen"                         // (the PC: ON OFFER; no release, walk or training)

// mp_phone_ball.c: a Moemon's POKé BALL (ball, color, decoration, entrance / recall effect) and MY PRESETs
void PhoneBall_Snapshot(struct Buf *b);                          // the snapshot's "fxu", "fxp"
u32 PhoneBall_Hash(void);
const char *PhoneBall_Apply(const char *op, const char *json);   // "ballstyle", "ballfx", "fxparty", "fxpreset"; -1: not these

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
