// Moemon Co-op: the phone link's protocol core (phone_proto.h). No game code: shared with the test client and the app.
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "phone_proto.h"
#include "monocypher/monocypher.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
// (RtlGenRandom: advapi32's SystemFunction036, the OS's random source on every Windows the game runs on)
BOOLEAN NTAPI SystemFunction036(PVOID buffer, ULONG length);
#endif

void PhoneProto_Random(void *out, size_t n)
{
#ifdef _WIN32
    if (SystemFunction036(out, (ULONG)n))
        return;
#else
    FILE *f = fopen("/dev/urandom", "rb");
    if (f != NULL)
    {
        size_t got = fread(out, 1, n, f);
        fclose(f);
        if (got == n)
            return;
    }
#endif
    {
        // (never expected: a hash of the time and the address of a stack variable, still never the game's RNG)
        uint8_t seed[64], h[64];
        size_t i, k = 0;
        uint64_t t = (uint64_t)time(NULL), c = (uint64_t)clock(), a = (uint64_t)(uintptr_t)&seed;
        memset(seed, 0, sizeof(seed));
        memcpy(seed, &t, 8);
        memcpy(seed + 8, &c, 8);
        memcpy(seed + 16, &a, 8);
        for (i = 0; i < n; i++)
        {
            if (k == 0)
            {
                crypto_blake2b(h, sizeof(h), seed, sizeof(seed));
                memcpy(seed + 32, h, 32);
            }
            ((uint8_t *)out)[i] = h[k];
            k = (k + 1) % 32;
        }
    }
}

void PhoneProto_CodeSecret(const char *code, uint8_t out[PHONE_KEY_SIZE])
{
    static const char sLabel[] = "moemon phone code v1:";
    uint8_t buf[sizeof(sLabel) + PHONE_CODE_LEN];
    size_t i;
    memcpy(buf, sLabel, sizeof(sLabel) - 1);
    for (i = 0; i < PHONE_CODE_LEN; i++)
    {
        char c = code[i];
        if (c >= 'a' && c <= 'z')
            c -= 'a' - 'A';
        buf[sizeof(sLabel) - 1 + i] = (uint8_t)c;
    }
    crypto_blake2b(out, PHONE_KEY_SIZE, buf, sizeof(sLabel) - 1 + PHONE_CODE_LEN);
}

void PhoneProto_NewCode(char out[PHONE_CODE_LEN + 1])
{
    static const char sAlphabet[] = PHONE_CODE_ALPHABET;
    uint8_t r[PHONE_CODE_LEN];
    size_t i;
    PhoneProto_Random(r, sizeof(r));
    for (i = 0; i < PHONE_CODE_LEN; i++)
        out[i] = sAlphabet[r[i] % 32];
    out[PHONE_CODE_LEN] = '\0';
}

void PhoneProto_QrSecret(const uint8_t raw[PHONE_QR_SECRET_SIZE], uint8_t out[PHONE_KEY_SIZE])
{
    static const char sLabel[] = "moemon phone qr v1:";
    uint8_t buf[sizeof(sLabel) + PHONE_QR_SECRET_SIZE];
    memcpy(buf, sLabel, sizeof(sLabel) - 1);
    memcpy(buf + sizeof(sLabel) - 1, raw, PHONE_QR_SECRET_SIZE);
    crypto_blake2b(out, PHONE_KEY_SIZE, buf, sizeof(sLabel) - 1 + PHONE_QR_SECRET_SIZE);
}

void PhoneProto_PairProof(const uint8_t secret[PHONE_KEY_SIZE], const uint8_t phoneId[PHONE_ID_SIZE],
                          const uint8_t saveId[PHONE_SAVEID_SIZE], uint8_t out[PHONE_KEY_SIZE])
{
    uint8_t msg[5 + PHONE_ID_SIZE + PHONE_SAVEID_SIZE];
    memcpy(msg, "pair:", 5);
    memcpy(msg + 5, phoneId, PHONE_ID_SIZE);
    memcpy(msg + 5 + PHONE_ID_SIZE, saveId, PHONE_SAVEID_SIZE);
    crypto_blake2b_keyed(out, PHONE_KEY_SIZE, secret, PHONE_KEY_SIZE, msg, sizeof(msg));
}

void PhoneProto_Derive(const uint8_t key[PHONE_KEY_SIZE], const char *word, uint8_t out[16])
{
    crypto_blake2b_keyed(out, 16, key, PHONE_KEY_SIZE, (const uint8_t *)word, strlen(word));
}

void PhoneProto_Derive32(const uint8_t key[PHONE_KEY_SIZE], const char *word, uint8_t out[PHONE_KEY_SIZE])
{
    crypto_blake2b_keyed(out, PHONE_KEY_SIZE, key, PHONE_KEY_SIZE, (const uint8_t *)word, strlen(word));
}

void PhoneProto_PublicKey(const uint8_t secret[PHONE_KEY_SIZE], uint8_t pub[PHONE_KEY_SIZE])
{
    crypto_x25519_public_key(pub, secret);
}

void PhoneProto_PairKey(const uint8_t mySecret[PHONE_KEY_SIZE], const uint8_t theirPublic[PHONE_KEY_SIZE],
                        uint8_t out[PHONE_KEY_SIZE])
{
    uint8_t shared[32];
    crypto_x25519(shared, mySecret, theirPublic);
    crypto_blake2b_keyed(out, PHONE_KEY_SIZE, shared, sizeof(shared), (const uint8_t *)"moemon pair", 11);
    crypto_wipe(shared, sizeof(shared));
}

void PhoneProto_SessionKey(const uint8_t key[PHONE_KEY_SIZE], const uint8_t phoneNonce[PHONE_NONCE_SIZE],
                           const uint8_t gameNonce[PHONE_NONCE_SIZE], uint8_t out[PHONE_KEY_SIZE])
{
    uint8_t msg[5 + 2 * PHONE_NONCE_SIZE];
    memcpy(msg, "sess:", 5);
    memcpy(msg + 5, phoneNonce, PHONE_NONCE_SIZE);
    memcpy(msg + 5 + PHONE_NONCE_SIZE, gameNonce, PHONE_NONCE_SIZE);
    crypto_blake2b_keyed(out, PHONE_KEY_SIZE, key, PHONE_KEY_SIZE, msg, sizeof(msg));
}

size_t PhoneProto_Seal(const uint8_t key[PHONE_KEY_SIZE], uint32_t seq, const void *msg, size_t len, uint8_t *out)
{
    uint8_t *nonce = out, *mac = out + PHONE_NONCE_SIZE, *text = out + PHONE_NONCE_SIZE + PHONE_MAC_SIZE;
    PhoneProto_Random(nonce, PHONE_NONCE_SIZE);
    text[0] = (uint8_t)(seq >> 24);
    text[1] = (uint8_t)(seq >> 16);
    text[2] = (uint8_t)(seq >> 8);
    text[3] = (uint8_t)seq;
    memcpy(text + 4, msg, len);
    crypto_aead_lock(text, mac, key, nonce, NULL, 0, text, len + 4);
    return len + PHONE_SEAL_EXTRA;
}

int PhoneProto_Open(const uint8_t key[PHONE_KEY_SIZE], const uint8_t *in, size_t len, uint32_t *seq, uint8_t *out)
{
    const uint8_t *nonce = in, *mac = in + PHONE_NONCE_SIZE, *text = in + PHONE_NONCE_SIZE + PHONE_MAC_SIZE;
    size_t n;
    uint8_t head[4];
    if (len < PHONE_SEAL_EXTRA)
        return -1;
    n = len - PHONE_NONCE_SIZE - PHONE_MAC_SIZE;
    // (unlocked in place into out: the caller's buffer holds the 4 bytes of sequence first)
    {
        uint8_t *tmp = out;   // out must hold n bytes
        if (crypto_aead_unlock(tmp, mac, key, nonce, NULL, 0, text, n) != 0)
            return -1;
        memcpy(head, tmp, 4);
        memmove(tmp, tmp + 4, n - 4);
    }
    *seq = ((uint32_t)head[0] << 24) | ((uint32_t)head[1] << 16) | ((uint32_t)head[2] << 8) | head[3];
    return (int)(n - 4);
}

void PhoneProto_ToHex(const uint8_t *in, size_t n, char *out)
{
    static const char sHex[] = "0123456789ABCDEF";
    size_t i;
    for (i = 0; i < n; i++)
    {
        out[2 * i] = sHex[in[i] >> 4];
        out[2 * i + 1] = sHex[in[i] & 15];
    }
    out[2 * n] = '\0';
}

static int HexDigit(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    return -1;
}

int PhoneProto_FromHex(const char *in, uint8_t *out, size_t n)
{
    size_t i;
    for (i = 0; i < n; i++)
    {
        int hi = HexDigit(in[2 * i]), lo = hi < 0 ? -1 : HexDigit(in[2 * i + 1]);
        if (hi < 0 || lo < 0)
            return 0;
        out[i] = (uint8_t)(hi * 16 + lo);
    }
    return 1;
}
