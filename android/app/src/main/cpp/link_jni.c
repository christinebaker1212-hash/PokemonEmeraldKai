// Moemon Companion: the game's phone link protocol core (src/mp/phone_proto.c) for the app's Java (Link.java)
#include <jni.h>
#include <string.h>
#include <stdlib.h>
#include "phone_proto.h"

#define FN(name) Java_com_moemon_companion_Native_##name

static jbyteArray Bytes(JNIEnv *env, const uint8_t *p, size_t n)
{
    jbyteArray a = (*env)->NewByteArray(env, (jsize)n);
    (*env)->SetByteArrayRegion(env, a, 0, (jsize)n, (const jbyte *)p);
    return a;
}

JNIEXPORT jbyteArray JNICALL FN(random)(JNIEnv *env, jclass c, jint n)
{
    uint8_t *p = malloc(n);
    jbyteArray a;
    PhoneProto_Random(p, n);
    a = Bytes(env, p, n);
    free(p);
    return a;
}

JNIEXPORT jbyteArray JNICALL FN(qrSecret)(JNIEnv *env, jclass c, jbyteArray raw)
{
    uint8_t in[PHONE_QR_SECRET_SIZE], out[PHONE_KEY_SIZE];
    (*env)->GetByteArrayRegion(env, raw, 0, PHONE_QR_SECRET_SIZE, (jbyte *)in);
    PhoneProto_QrSecret(in, out);
    return Bytes(env, out, sizeof(out));
}

JNIEXPORT jbyteArray JNICALL FN(codeSecret)(JNIEnv *env, jclass c, jstring code)
{
    uint8_t out[PHONE_KEY_SIZE];
    const char *s = (*env)->GetStringUTFChars(env, code, NULL);
    char buf[PHONE_CODE_LEN + 1] = {0};
    strncpy(buf, s, PHONE_CODE_LEN);
    (*env)->ReleaseStringUTFChars(env, code, s);
    PhoneProto_CodeSecret(buf, out);
    return Bytes(env, out, sizeof(out));
}

JNIEXPORT jbyteArray JNICALL FN(pairProof)(JNIEnv *env, jclass c, jbyteArray secret, jbyteArray phone, jbyteArray save)
{
    uint8_t s[PHONE_KEY_SIZE], p[PHONE_ID_SIZE], v[PHONE_SAVEID_SIZE], out[PHONE_KEY_SIZE];
    (*env)->GetByteArrayRegion(env, secret, 0, sizeof(s), (jbyte *)s);
    (*env)->GetByteArrayRegion(env, phone, 0, sizeof(p), (jbyte *)p);
    (*env)->GetByteArrayRegion(env, save, 0, sizeof(v), (jbyte *)v);
    PhoneProto_PairProof(s, p, v, out);
    return Bytes(env, out, sizeof(out));
}

JNIEXPORT jbyteArray JNICALL FN(sessionKey)(JNIEnv *env, jclass c, jbyteArray key, jbyteArray pn, jbyteArray gn)
{
    uint8_t k[PHONE_KEY_SIZE], a[PHONE_NONCE_SIZE], b[PHONE_NONCE_SIZE], out[PHONE_KEY_SIZE];
    (*env)->GetByteArrayRegion(env, key, 0, sizeof(k), (jbyte *)k);
    (*env)->GetByteArrayRegion(env, pn, 0, sizeof(a), (jbyte *)a);
    (*env)->GetByteArrayRegion(env, gn, 0, sizeof(b), (jbyte *)b);
    PhoneProto_SessionKey(k, a, b, out);
    return Bytes(env, out, sizeof(out));
}

// msg sealed with key and seq: nonce | mac | sealed
JNIEXPORT jbyteArray JNICALL FN(seal)(JNIEnv *env, jclass c, jbyteArray key, jint seq, jbyteArray msg)
{
    uint8_t k[PHONE_KEY_SIZE];
    jsize n = (*env)->GetArrayLength(env, msg);
    uint8_t *in = malloc(n + 1), *out = malloc(n + PHONE_SEAL_EXTRA);
    jbyteArray a;
    (*env)->GetByteArrayRegion(env, key, 0, sizeof(k), (jbyte *)k);
    (*env)->GetByteArrayRegion(env, msg, 0, n, (jbyte *)in);
    PhoneProto_Seal(k, (uint32_t)seq, in, n, out);
    a = Bytes(env, out, n + PHONE_SEAL_EXTRA);
    free(in);
    free(out);
    return a;
}

// null if forged or damaged; else 4 bytes of sequence number (big-endian), then the message
JNIEXPORT jbyteArray JNICALL FN(open)(JNIEnv *env, jclass c, jbyteArray key, jbyteArray sealed)
{
    uint8_t k[PHONE_KEY_SIZE];
    jsize n = (*env)->GetArrayLength(env, sealed);
    uint8_t *in = malloc(n + 1), *out = malloc(n + 4);
    uint32_t seq;
    int m;
    jbyteArray a = NULL;
    (*env)->GetByteArrayRegion(env, key, 0, sizeof(k), (jbyte *)k);
    (*env)->GetByteArrayRegion(env, sealed, 0, n, (jbyte *)in);
    m = PhoneProto_Open(k, in, n, &seq, out + 4);
    if (m >= 0)
    {
        out[0] = seq >> 24, out[1] = seq >> 16, out[2] = seq >> 8, out[3] = seq;
        a = Bytes(env, out, m + 4);
    }
    free(in);
    free(out);
    return a;
}

JNIEXPORT jint JNICALL FN(linkVersion)(JNIEnv *env, jclass c)
{
    return PHONE_LINK_VERSION;
}
