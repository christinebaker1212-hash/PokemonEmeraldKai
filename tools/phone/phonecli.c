// Moemon Co-op: a test client for the phone link (docs/PHONE_LINK.md), the same protocol core as the app
// (src/mp/phone_proto.c). It pairs or says hello, then passes messages along: each JSON line on stdin goes to the
// game sealed, each message from the game comes out as one JSON line on stdout. tools/phone/gate.py drives it.
//
//   phonecli HOST PORT STATE pair SAVEID SECRET|CODE [NAME]   pair (the QR code's 32-digit secret, or the 6 letters typed)
//   phonecli HOST PORT STATE hello                           link with the phone and key kept in STATE
//
// Build: make -C tools/phone
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include "phone_proto.h"

static int sSock = -1;
static uint8_t sKey[PHONE_KEY_SIZE], sPhone[PHONE_ID_SIZE], sSave[PHONE_SAVEID_SIZE], sSess[PHONE_KEY_SIZE];
static uint32_t sSeqOut, sSeqIn;

static void Die(const char *why)
{
    fprintf(stderr, "phonecli: %s\n", why);
    exit(1);
}

static void SendFrame(const uint8_t *body, size_t len)
{
    uint8_t head[4] = {(uint8_t)(len >> 24), (uint8_t)(len >> 16), (uint8_t)(len >> 8), (uint8_t)len};
    if (send(sSock, head, 4, 0) != 4 || send(sSock, body, len, 0) != (ssize_t)len)
        Die("send failed");
}

static void SendPlain(const char *json)
{
    size_t n = strlen(json);
    uint8_t *b = malloc(n + 1);
    b[0] = PHONE_BODY_PLAIN;
    memcpy(b + 1, json, n);
    SendFrame(b, n + 1);
    free(b);
}

static void SendSealed(const char *json)
{
    size_t n = strlen(json);
    uint8_t *b = malloc(n + 1 + PHONE_SEAL_EXTRA);
    b[0] = PHONE_BODY_SEALED;
    PhoneProto_Seal(sSess, ++sSeqOut, json, n, b + 1);
    SendFrame(b, n + 1 + PHONE_SEAL_EXTRA);
    free(b);
}

static int RecvAll(uint8_t *p, size_t n)
{
    size_t got = 0;
    while (got < n)
    {
        ssize_t r = recv(sSock, p + got, n - got, 0);
        if (r <= 0)
            return 0;
        got += r;
    }
    return 1;
}

// One frame from the game: its JSON (sealed ones opened), NULL when the link closed
static char *RecvJson(void)
{
    uint8_t head[4], *body;
    size_t n;
    if (!RecvAll(head, 4))
        return NULL;
    n = ((size_t)head[0] << 24) | ((size_t)head[1] << 16) | ((size_t)head[2] << 8) | head[3];
    if (n == 0 || n > PHONE_FRAME_MAX)
        Die("bad frame");
    body = malloc(n + 1);
    if (!RecvAll(body, n))
        return NULL;
    if (body[0] == PHONE_BODY_PLAIN)
    {
        memmove(body, body + 1, n - 1);
        body[n - 1] = '\0';
        return (char *)body;
    }
    if (body[0] == PHONE_BODY_SEALED)
    {
        uint32_t seq;
        char *out = malloc(n);
        int m = PhoneProto_Open(sSess, body + 1, n - 1, &seq, (uint8_t *)out);
        free(body);
        if (m < 0 || seq <= sSeqIn)
            Die("a forged or replayed message from the game");
        sSeqIn = seq;
        out[m] = '\0';
        return out;
    }
    Die("unknown frame");
    return NULL;
}

static const char *Field(const char *json, const char *key, char *out, size_t size)
{
    char pat[32];
    const char *p;
    size_t n = 0;
    snprintf(pat, sizeof(pat), "\"%s\":\"", key);
    p = strstr(json, pat);
    if (p == NULL)
        return NULL;
    p += strlen(pat);
    while (*p && *p != '"' && n + 1 < size)
        out[n++] = *p++;
    out[n] = '\0';
    return out;
}

static void Connect(const char *host, const char *port)
{
    struct addrinfo hints = {0}, *res;
    int one = 1;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(host, port, &hints, &res) != 0)
        Die("no such host");
    sSock = socket(res->ai_family, res->ai_socktype, 0);
    if (sSock < 0 || connect(sSock, res->ai_addr, res->ai_addrlen) != 0)
        Die("can't connect");
    setsockopt(sSock, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));
    freeaddrinfo(res);
}

static void SaveState(const char *path)
{
    FILE *f = fopen(path, "wb");
    if (f == NULL)
        Die("can't write the state");
    fwrite(sPhone, 1, sizeof(sPhone), f);
    fwrite(sKey, 1, sizeof(sKey), f);
    fwrite(sSave, 1, sizeof(sSave), f);
    fclose(f);
}

static void LoadState(const char *path)
{
    FILE *f = fopen(path, "rb");
    if (f == NULL || fread(sPhone, 1, sizeof(sPhone), f) != sizeof(sPhone) || fread(sKey, 1, sizeof(sKey), f) != sizeof(sKey)
     || fread(sSave, 1, sizeof(sSave), f) != sizeof(sSave))
        Die("no state: pair first");
    fclose(f);
}

static void Pair(const char *state, const char *saveHex, const char *secretArg, const char *name)
{
    uint8_t secret[PHONE_KEY_SIZE], proof[PHONE_KEY_SIZE];
    char phoneHex[2 * PHONE_ID_SIZE + 1], proofHex[2 * PHONE_KEY_SIZE + 1], json[512], keyHex[256], *reply;
    if (!PhoneProto_FromHex(saveHex, sSave, PHONE_SAVEID_SIZE))
        Die("bad save ID");
    if (strlen(secretArg) == PHONE_CODE_LEN)
        PhoneProto_CodeSecret(secretArg, secret);
    else
    {
        uint8_t raw[PHONE_QR_SECRET_SIZE];
        if (!PhoneProto_FromHex(secretArg, raw, sizeof(raw)))
            Die("bad secret");
        PhoneProto_QrSecret(raw, secret);
    }
    PhoneProto_Random(sPhone, sizeof(sPhone));
    PhoneProto_PairProof(secret, sPhone, sSave, proof);
    PhoneProto_ToHex(sPhone, sizeof(sPhone), phoneHex);
    PhoneProto_ToHex(proof, sizeof(proof), proofHex);
    snprintf(json, sizeof(json), "{\"t\":\"pair\",\"v\":%d,\"phone\":\"%s\",\"save\":\"%s\",\"proof\":\"%s\",\"name\":\"%s\"}",
             PHONE_LINK_VERSION, phoneHex, saveHex, proofHex, name);
    SendPlain(json);
    reply = RecvJson();
    if (reply == NULL)
        Die("the game closed the link");
    printf("%s\n", reply);
    fflush(stdout);
    if (Field(reply, "key", keyHex, sizeof(keyHex)) == NULL)
        exit(2);
    {
        uint8_t sealed[PHONE_KEY_SIZE + PHONE_SEAL_EXTRA], key[PHONE_KEY_SIZE + PHONE_SEAL_EXTRA];
        uint32_t seq;
        if (!PhoneProto_FromHex(keyHex, sealed, sizeof(sealed))
         || PhoneProto_Open(secret, sealed, sizeof(sealed), &seq, key) != PHONE_KEY_SIZE)
            Die("the game's key doesn't open");
        memcpy(sKey, key, PHONE_KEY_SIZE);
    }
    SaveState(state);
}

static void Hello(void)
{
    uint8_t pn[PHONE_NONCE_SIZE], gn[PHONE_NONCE_SIZE];
    char phoneHex[2 * PHONE_ID_SIZE + 1], saveHex[2 * PHONE_SAVEID_SIZE + 1], pnHex[2 * PHONE_NONCE_SIZE + 1];
    char json[512], gnHex[2 * PHONE_NONCE_SIZE + 4], *reply;
    PhoneProto_Random(pn, sizeof(pn));
    PhoneProto_ToHex(sPhone, sizeof(sPhone), phoneHex);
    PhoneProto_ToHex(sSave, sizeof(sSave), saveHex);
    PhoneProto_ToHex(pn, sizeof(pn), pnHex);
    snprintf(json, sizeof(json), "{\"t\":\"hello\",\"v\":%d,\"phone\":\"%s\",\"save\":\"%s\",\"nonce\":\"%s\"}",
             PHONE_LINK_VERSION, phoneHex, saveHex, pnHex);
    SendPlain(json);
    reply = RecvJson();
    if (reply == NULL)
        Die("the game closed the link");
    printf("%s\n", reply);
    fflush(stdout);
    if (Field(reply, "nonce", gnHex, sizeof(gnHex)) == NULL || !PhoneProto_FromHex(gnHex, gn, sizeof(gn)))
        exit(2);
    PhoneProto_SessionKey(sKey, pn, gn, sSess);
    free(reply);
}

int main(int argc, char **argv)
{
    char *line = NULL;
    size_t cap = 0;
    if (argc < 5)
        Die("usage: phonecli HOST PORT STATE pair SAVEID SECRET|CODE [NAME] | hello");
    Connect(argv[1], argv[2]);
    if (strcmp(argv[4], "pair") == 0)
    {
        if (argc < 7)
            Die("pair needs SAVEID and SECRET");
        Pair(argv[3], argv[5], argv[6], argc > 7 ? argv[7] : "TESTPHONE");
        return 0;
    }
    LoadState(argv[3]);
    Hello();
    for (;;)
    {
        struct pollfd fds[2] = {{STDIN_FILENO, POLLIN, 0}, {sSock, POLLIN, 0}};
        if (poll(fds, 2, -1) < 0)
            break;
        if (fds[1].revents & (POLLIN | POLLHUP))
        {
            char *json = RecvJson();
            if (json == NULL)
            {
                printf("{\"t\":\"closed\"}\n");
                fflush(stdout);
                return 0;
            }
            printf("%s\n", json);
            fflush(stdout);
            free(json);
        }
        if (fds[0].revents & (POLLIN | POLLHUP))
        {
            ssize_t n = getline(&line, &cap, stdin);
            if (n <= 0)
                return 0;
            while (n > 0 && (line[n - 1] == '\n' || line[n - 1] == '\r'))
                line[--n] = '\0';
            if (n > 0)
                SendSealed(line);
        }
    }
    return 0;
}
