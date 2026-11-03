// Moemon Co-op: the phone link's protocol core (docs/COMPANION_APP.md, docs/PHONE_LINK.md). Plain C with no game
// code, so the game (src/mp/mp_phone.c), the test client (tools/phone/phonecli.c) and the app (android/, through the
// NDK) all build the same file. Crypto is Monocypher's (src/mp/monocypher, public domain / BSD-2).
#ifndef GUARD_PHONE_PROTO_H
#define GUARD_PHONE_PROTO_H

#include <stddef.h>
#include <stdint.h>

#define PHONE_LINK_VERSION 1        // the link's own protocol number (not the game's version)
#define PHONE_TCP_PORT     47610    // the game listens here on the local network (the next free one if taken)
#define PHONE_UDP_PORT     47611    // "where are you?" broadcasts: the game answers with its TCP port
#define PHONE_KEY_SIZE     32
#define PHONE_ID_SIZE      16
#define PHONE_SAVEID_SIZE  8
#define PHONE_NONCE_SIZE   24
#define PHONE_MAC_SIZE     16
#define PHONE_SEAL_EXTRA   (PHONE_NONCE_SIZE + PHONE_MAC_SIZE + 4)   // a sealed message's bytes past its own
#define PHONE_CODE_LEN     6        // the letters to type when the camera can't read the QR code
#define PHONE_FRAME_MAX    (1 << 20)

// A frame on the wire: a 4-byte big-endian length, then the body. A body's first byte says what it is:
#define PHONE_BODY_PLAIN   'P'      // JSON, only before the session: "pair", "paired", "hello", "welcome", "bye"
#define PHONE_BODY_SEALED  'S'      // nonce | mac | sealed (sequence number | JSON)

void PhoneProto_Random(void *out, size_t n);   // the system's random source (never the game's RNG)

// A pairing secret from the letters typed (PHONE_CODE_LEN of PHONE_CODE_ALPHABET)
#define PHONE_CODE_ALPHABET "ABCDEFGHJKLMNPQRSTUVWXYZ23456789"
void PhoneProto_CodeSecret(const char *code, uint8_t out[PHONE_KEY_SIZE]);
void PhoneProto_NewCode(char out[PHONE_CODE_LEN + 1]);

// The QR code's secret: PHONE_QR_SECRET_SIZE random bytes (short, so the code stays small), stretched to a key
#define PHONE_QR_SECRET_SIZE 16
void PhoneProto_QrSecret(const uint8_t raw[PHONE_QR_SECRET_SIZE], uint8_t out[PHONE_KEY_SIZE]);

// The phone proves it holds the pairing secret (the QR code's, or the typed code's)
void PhoneProto_PairProof(const uint8_t secret[PHONE_KEY_SIZE], const uint8_t phoneId[PHONE_ID_SIZE],
                          const uint8_t saveId[PHONE_SAVEID_SIZE], uint8_t out[PHONE_KEY_SIZE]);

// A session's key from the pair's long-term key and both sides' fresh nonces
void PhoneProto_SessionKey(const uint8_t key[PHONE_KEY_SIZE], const uint8_t phoneNonce[PHONE_NONCE_SIZE],
                           const uint8_t gameNonce[PHONE_NONCE_SIZE], uint8_t out[PHONE_KEY_SIZE]);

// Seal: out gets nonce | mac | ciphertext of (seq, big-endian 4 bytes | msg); returns its size (len + PHONE_SEAL_EXTRA)
size_t PhoneProto_Seal(const uint8_t key[PHONE_KEY_SIZE], uint32_t seq, const void *msg, size_t len, uint8_t *out);
// Open: the message's size (out gets len - PHONE_SEAL_EXTRA bytes), -1 if it was forged or damaged
int PhoneProto_Open(const uint8_t key[PHONE_KEY_SIZE], const uint8_t *in, size_t len, uint32_t *seq, uint8_t *out);

// 16 bytes from a 32-byte key and a word (BLAKE2b keyed): a pipe's ID ("pipe"), a friend's presence record ("presence"),
// their inbox ("inbox")... secret to everyone without the key, the same on the game, the app and the test client
void PhoneProto_Derive(const uint8_t key[PHONE_KEY_SIZE], const char *word, uint8_t out[16]);
void PhoneProto_Derive32(const uint8_t key[PHONE_KEY_SIZE], const char *word, uint8_t out[PHONE_KEY_SIZE]);
// Two saves' shared key (X25519): only those two can work it out, each from its own secret and the other's public key
void PhoneProto_PublicKey(const uint8_t secret[PHONE_KEY_SIZE], uint8_t pub[PHONE_KEY_SIZE]);
void PhoneProto_PairKey(const uint8_t mySecret[PHONE_KEY_SIZE], const uint8_t theirPublic[PHONE_KEY_SIZE],
                        uint8_t out[PHONE_KEY_SIZE]);
void PhoneProto_ToHex(const uint8_t *in, size_t n, char *out);   // out: 2n + 1 chars, upper case
int PhoneProto_FromHex(const char *in, uint8_t *out, size_t n);  // 0 if in isn't 2n hex digits

#endif // GUARD_PHONE_PROTO_H
