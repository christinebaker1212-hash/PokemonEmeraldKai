// Moemon Co-op: the auto-updater. Runs when the game starts, before anything else
// (sdl2.c main, right after the window opens).
//
// 1. Fetches update.txt from the latest GitHub release of MOEMON_UPDATE_REPO
//    (https://github.com/<repo>/releases/latest/download/update.txt). No release,
//    offline, a firewall in the way, anything wrong: the game just starts.
// 2. If that release's version is higher than this build's (mp_version.h), downloads
//    this platform's zip with a progress screen (ESC skips it), checks its size and
//    SHA-256 against update.txt, and checks every file's CRC as it unpacks.
// 3. Installs next to the exe: every file is first written as <name>.upd_new, then
//    swapped in (the old one is kept as <name>.upd_old until the next start, so a
//    failure halfway puts everything back). Save files and .ini files that are
//    already there are never replaced, and nothing that isn't in the zip is touched.
// 4. Starts the new version and quits.
//
// It only runs in a folder unpacked from a release (moemon_release.txt next to the
// exe), never in a source checkout. AutoUpdate = 0 in moemon_coop.ini, --no-update or
// MOEMON_NO_UPDATE=1 turn it off. Downloads use the system's own HTTPS: WinHTTP on
// Windows, libcurl (loaded only if present) on Linux. Nothing about the firewall is
// changed: these are ordinary outgoing HTTPS requests.
//
// Tests (tools/test/update_test.sh): MOEMON_UPDATE_URL=<base> takes update.txt and the zip
// from <base> instead of GitHub. MOEMON_UPDATE_FETCH=<url> fetches one URL, prints its size
// and SHA-256 and exits. MOEMON_UPDATE_TEST_FAIL_SWAP=N fails file swap N (0 = the first; rollback),
// MOEMON_UPDATE_TEST_ESC=1 presses ESC once the download is going, MOEMON_UPDATE_TEST_SHOT=
// <prefix> saves the download and result screens as <prefix>_download.bmp / _result.bmp.
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>

#ifdef _WIN32
#include <windows.h>
#include <winhttp.h>
#include <direct.h>
#ifndef WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY
#define WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY 4
#endif
#else
#include <dlfcn.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include "mp_version.h"

#ifdef _WIN32
#define UPDATE_PLATFORM "windows"
#else
#define UPDATE_PLATFORM "linux"
#endif

#define RELEASE_MARKER "moemon_release.txt"
#define CLEANUP_LIST "update_cleanup.txt"
#define DOWNLOAD_FILE "update_download.zip"
#define MAX_MANIFEST 16384
#define MAX_ZIP (512u * 1024u * 1024u)

int Display_Init(const char *title);
void Display_Shutdown(void);
int Display_HandleEvent(const SDL_Event *e);
int Display_SaveOutputBMP(const char *path);
void Display_UploadFrame(const void *pixels);
void Display_Present(void);

// stb_image's raw deflate decoder (compiled in mp_avatar.c)
char *stbi_zlib_decode_noheader_malloc(const char *buffer, int len, int *outlen);

static char sExeDir[1024];
static char sExePath[1024];

static void Log(const char *fmt, ...)
{
    va_list ap;
    printf("[update] ");
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
    printf("\n");
    fflush(stdout);
}

// ---------------------------------------------------------------------------
// Files, with UTF-8 paths on Windows too

#ifdef _WIN32
static wchar_t *Wide(const char *s)
{
    int n = MultiByteToWideChar(CP_UTF8, 0, s, -1, NULL, 0);
    wchar_t *w = malloc(sizeof(wchar_t) * (n > 0 ? n : 1));
    if (n <= 0 || !w)
    {
        if (w)
            w[0] = 0;
        return w;
    }
    MultiByteToWideChar(CP_UTF8, 0, s, -1, w, n);
    return w;
}
#endif

static FILE *UOpen(const char *path, const char *mode)
{
#ifdef _WIN32
    wchar_t *wp = Wide(path), *wm = Wide(mode);
    FILE *f = (wp && wm) ? _wfopen(wp, wm) : NULL;
    free(wp);
    free(wm);
    return f;
#else
    return fopen(path, mode);
#endif
}

static int UExists(const char *path)
{
    FILE *f = UOpen(path, "rb");
    if (!f)
        return 0;
    fclose(f);
    return 1;
}

static int URemove(const char *path)
{
#ifdef _WIN32
    wchar_t *wp = Wide(path);
    int ok = wp && DeleteFileW(wp);
    free(wp);
    return ok ? 0 : -1;
#else
    return remove(path);
#endif
}

// Moves a file onto a new name (replacing it). Works on a running exe on both systems.
static int URename(const char *from, const char *to)
{
#ifdef _WIN32
    wchar_t *wf = Wide(from), *wt = Wide(to);
    int ok = wf && wt && MoveFileExW(wf, wt, MOVEFILE_REPLACE_EXISTING);
    free(wf);
    free(wt);
    return ok ? 0 : -1;
#else
    return rename(from, to);
#endif
}

static void UMkdir(const char *path)
{
#ifdef _WIN32
    wchar_t *wp = Wide(path);
    if (wp)
        CreateDirectoryW(wp, NULL);
    free(wp);
#else
    mkdir(path, 0755);
#endif
}

// Makes every folder above a file
static void MakeParents(const char *path)
{
    char tmp[1024];
    size_t i;
    snprintf(tmp, sizeof(tmp), "%s", path);
    for (i = 1; tmp[i]; i++)
    {
        if (tmp[i] == '/' || tmp[i] == '\\')
        {
            char c = tmp[i];
            tmp[i] = 0;
            UMkdir(tmp);
            tmp[i] = c;
        }
    }
}

static void JoinPath(char *out, size_t size, const char *dir, const char *name)
{
    snprintf(out, size, "%s/%s", dir, name);
}

static int FindExe(void)
{
#ifdef _WIN32
    wchar_t w[1024];
    DWORD n = GetModuleFileNameW(NULL, w, 1024);
    if (n == 0 || n >= 1024)
        return 0;
    if (!WideCharToMultiByte(CP_UTF8, 0, w, -1, sExePath, sizeof(sExePath), NULL, NULL))
        return 0;
#else
    ssize_t n = readlink("/proc/self/exe", sExePath, sizeof(sExePath) - 1);
    if (n <= 0)
        return 0;
    sExePath[n] = 0;
#endif
    snprintf(sExeDir, sizeof(sExeDir), "%s", sExePath);
    {
        char *a = strrchr(sExeDir, '/'), *b = strrchr(sExeDir, '\\');
        char *s = a > b ? a : b;
        if (!s)
            return 0;
        *s = 0;
    }
    return 1;
}

// ---------------------------------------------------------------------------
// SHA-256 and CRC-32

struct Sha256 { uint32_t h[8]; uint8_t buf[64]; uint64_t len; size_t fill; };

static const uint32_t sK[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2,
};

#define ROR(x, n) (((x) >> (n)) | ((x) << (32 - (n))))

static void ShaBlock(struct Sha256 *s, const uint8_t *p)
{
    uint32_t w[64], a, b, c, d, e, f, g, h;
    int i;
    for (i = 0; i < 16; i++)
        w[i] = ((uint32_t)p[i * 4] << 24) | ((uint32_t)p[i * 4 + 1] << 16) | ((uint32_t)p[i * 4 + 2] << 8) | p[i * 4 + 3];
    for (i = 16; i < 64; i++)
    {
        uint32_t s0 = ROR(w[i - 15], 7) ^ ROR(w[i - 15], 18) ^ (w[i - 15] >> 3);
        uint32_t s1 = ROR(w[i - 2], 17) ^ ROR(w[i - 2], 19) ^ (w[i - 2] >> 10);
        w[i] = w[i - 16] + s0 + w[i - 7] + s1;
    }
    a = s->h[0]; b = s->h[1]; c = s->h[2]; d = s->h[3]; e = s->h[4]; f = s->h[5]; g = s->h[6]; h = s->h[7];
    for (i = 0; i < 64; i++)
    {
        uint32_t t1 = h + (ROR(e, 6) ^ ROR(e, 11) ^ ROR(e, 25)) + ((e & f) ^ (~e & g)) + sK[i] + w[i];
        uint32_t t2 = (ROR(a, 2) ^ ROR(a, 13) ^ ROR(a, 22)) + ((a & b) ^ (a & c) ^ (b & c));
        h = g; g = f; f = e; e = d + t1; d = c; c = b; b = a; a = t1 + t2;
    }
    s->h[0] += a; s->h[1] += b; s->h[2] += c; s->h[3] += d; s->h[4] += e; s->h[5] += f; s->h[6] += g; s->h[7] += h;
}

static void ShaInit(struct Sha256 *s)
{
    static const uint32_t sH0[8] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
    memcpy(s->h, sH0, sizeof(sH0));
    s->len = 0;
    s->fill = 0;
}

static void ShaUpdate(struct Sha256 *s, const void *data, size_t n)
{
    const uint8_t *p = data;
    s->len += n;
    while (n)
    {
        size_t take = 64 - s->fill;
        if (take > n)
            take = n;
        memcpy(s->buf + s->fill, p, take);
        s->fill += take;
        p += take;
        n -= take;
        if (s->fill == 64)
        {
            ShaBlock(s, s->buf);
            s->fill = 0;
        }
    }
}

static void ShaHex(struct Sha256 *s, char out[65])
{
    uint64_t bits = s->len * 8;
    uint8_t pad = 0x80, zero = 0, lenBytes[8];
    int i;
    ShaUpdate(s, &pad, 1);
    while (s->fill != 56)
        ShaUpdate(s, &zero, 1);
    for (i = 0; i < 8; i++)
        lenBytes[i] = (uint8_t)(bits >> (56 - 8 * i));
    ShaUpdate(s, lenBytes, 8);
    for (i = 0; i < 8; i++)
        sprintf(out + i * 8, "%08x", s->h[i]);
    out[64] = 0;
}

static uint32_t Crc32(const uint8_t *p, size_t n)
{
    static uint32_t table[256];
    static int ready;
    uint32_t c = 0xFFFFFFFFu;
    size_t i;
    if (!ready)
    {
        uint32_t k, v;
        int j;
        for (k = 0; k < 256; k++)
        {
            v = k;
            for (j = 0; j < 8; j++)
                v = (v & 1) ? 0xEDB88320u ^ (v >> 1) : v >> 1;
            table[k] = v;
        }
        ready = 1;
    }
    for (i = 0; i < n; i++)
        c = table[(c ^ p[i]) & 0xFF] ^ (c >> 8);
    return c ^ 0xFFFFFFFFu;
}

// ---------------------------------------------------------------------------
// The update's state, shared by the screen (main thread) and the download (worker)

enum {
    PH_CHECK, PH_DOWNLOAD, PH_VERIFY, PH_INSTALL,
    PH_NONE,        // nothing to install: start the game
    PH_INSTALLED,   // restart into the new version
    PH_FAILED,      // show why for a moment, then start the game
    PH_SKIPPED,     // the player pressed ESC
};

static struct
{
    SDL_atomic_t phase;
    SDL_atomic_t cancel;
    SDL_atomic_t got;        // bytes downloaded
    long total;              // the zip's size from update.txt
    char version[32];
    char tag[48];
    char zip[128];
    char sha[65];
    char exe[128];
    char error[96];
} U;

static void Fail(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(U.error, sizeof(U.error), fmt, ap);
    va_end(ap);
    Log("failed: %s", U.error);
    SDL_AtomicSet(&U.phase, PH_FAILED);
}

// ---------------------------------------------------------------------------
// HTTP(S) GET into a sink. Returns 1 when the whole body arrived with status 200.

struct Sink
{
    FILE *file;        // to a file, or
    char *mem;         // to memory (up to cap bytes)
    long cap, len;
    struct Sha256 sha;
    int progress;      // count into U.got
};

static int SinkWrite(struct Sink *k, const void *data, size_t n)
{
    if (SDL_AtomicGet(&U.cancel))
        return 0;
    if (k->len + (long)n > k->cap)
        return 0;
    if (k->file)
    {
        if (fwrite(data, 1, n, k->file) != n)
            return 0;
    }
    else
    {
        memcpy(k->mem + k->len, data, n);
    }
    ShaUpdate(&k->sha, data, n);
    k->len += (long)n;
    if (k->progress)
        SDL_AtomicSet(&U.got, (int)k->len);
    return 1;
}

#ifdef _WIN32
static int HttpGet(const char *url, struct Sink *k, int *status)
{
    URL_COMPONENTS uc;
    wchar_t *wurl = Wide(url), host[256];
    HINTERNET s = NULL, c = NULL, r = NULL;
    DWORD code = 0, len = sizeof(code), got;
    char buf[16384];
    int ok = 0;

    *status = 0;
    if (!wurl)
        return 0;
    memset(&uc, 0, sizeof(uc));
    uc.dwStructSize = sizeof(uc);
    uc.dwSchemeLength = (DWORD)-1;
    uc.dwHostNameLength = (DWORD)-1;
    uc.dwUrlPathLength = (DWORD)-1;
    uc.dwExtraInfoLength = (DWORD)-1;
    if (!WinHttpCrackUrl(wurl, 0, 0, &uc) || uc.dwHostNameLength >= 256)
        goto done;
    memcpy(host, uc.lpszHostName, uc.dwHostNameLength * sizeof(wchar_t));
    host[uc.dwHostNameLength] = 0;
    s = WinHttpOpen(L"MoemonEmeraldKaiCoop-Updater/" MOEMON_VERSION, WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY, NULL, NULL, 0);
    if (!s)   // older Windows (and Wine): the system's proxy settings
        s = WinHttpOpen(L"MoemonEmeraldKaiCoop-Updater/" MOEMON_VERSION, WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, NULL, NULL, 0);
    if (!s)
        goto done;
    WinHttpSetTimeouts(s, 6000, 6000, 15000, 30000);
    c = WinHttpConnect(s, host, uc.nPort, 0);
    if (!c)
        goto done;
    r = WinHttpOpenRequest(c, L"GET", uc.lpszUrlPath, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
                           uc.nScheme == INTERNET_SCHEME_HTTPS ? WINHTTP_FLAG_SECURE : 0);
    if (!r)
        goto done;
    if (!WinHttpSendRequest(r, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0)
     || !WinHttpReceiveResponse(r, NULL))
        goto done;
    if (!WinHttpQueryHeaders(r, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BY_INDEX, &code, &len, WINHTTP_NO_HEADER_INDEX))
        goto done;
    *status = (int)code;
    if (code != 200)
        goto done;
    for (;;)
    {
        got = 0;
        if (!WinHttpReadData(r, buf, sizeof(buf), &got))
            goto done;
        if (got == 0)
            break;
        if (!SinkWrite(k, buf, got))
            goto done;
    }
    ok = 1;
done:
    if (r)
        WinHttpCloseHandle(r);
    if (c)
        WinHttpCloseHandle(c);
    if (s)
        WinHttpCloseHandle(s);
    free(wurl);
    return ok;
}
#else
// libcurl, loaded at run time: the game doesn't need it to start
typedef void CURL;
static struct
{
    void *lib;
    int (*global_init)(long);
    CURL *(*easy_init)(void);
    int (*easy_setopt)(CURL *, int, ...);
    int (*easy_perform)(CURL *);
    int (*easy_getinfo)(CURL *, int, ...);
    void (*easy_cleanup)(CURL *);
    const char *(*easy_strerror)(int);
} sCurl;

#define CURLOPT_WRITEDATA 10001
#define CURLOPT_URL 10002
#define CURLOPT_USERAGENT 10018
#define CURLOPT_CAINFO 10065
#define CURLOPT_WRITEFUNCTION 20011
#define CURLOPT_TIMEOUT 13
#define CURLOPT_LOW_SPEED_LIMIT 19
#define CURLOPT_LOW_SPEED_TIME 20
#define CURLOPT_FOLLOWLOCATION 52
#define CURLOPT_MAXREDIRS 68
#define CURLOPT_CONNECTTIMEOUT 78
#define CURLOPT_NOSIGNAL 99
#define CURLINFO_RESPONSE_CODE 0x200002

static int LoadCurl(void)
{
    static const char *const sNames[] = {"libcurl.so.4", "libcurl-gnutls.so.4", "libcurl.so"};
    size_t i;
    if (sCurl.lib)
        return 1;
    for (i = 0; i < sizeof(sNames) / sizeof(sNames[0]) && !sCurl.lib; i++)
        sCurl.lib = dlopen(sNames[i], RTLD_NOW);
    if (!sCurl.lib)
        return 0;
    sCurl.global_init = (int (*)(long))dlsym(sCurl.lib, "curl_global_init");
    sCurl.easy_init = (CURL *(*)(void))dlsym(sCurl.lib, "curl_easy_init");
    sCurl.easy_setopt = (int (*)(CURL *, int, ...))dlsym(sCurl.lib, "curl_easy_setopt");
    sCurl.easy_perform = (int (*)(CURL *))dlsym(sCurl.lib, "curl_easy_perform");
    sCurl.easy_getinfo = (int (*)(CURL *, int, ...))dlsym(sCurl.lib, "curl_easy_getinfo");
    sCurl.easy_cleanup = (void (*)(CURL *))dlsym(sCurl.lib, "curl_easy_cleanup");
    sCurl.easy_strerror = (const char *(*)(int))dlsym(sCurl.lib, "curl_easy_strerror");
    if (!sCurl.global_init || !sCurl.easy_init || !sCurl.easy_setopt || !sCurl.easy_perform || !sCurl.easy_getinfo
     || !sCurl.easy_cleanup || !sCurl.easy_strerror)
    {
        dlclose(sCurl.lib);
        sCurl.lib = NULL;
        return 0;
    }
    sCurl.global_init(3);   // CURL_GLOBAL_DEFAULT, once, on the main thread
    return 1;
}

static size_t CurlWrite(char *data, size_t size, size_t n, void *ctx)
{
    return SinkWrite(ctx, data, size * n) ? size * n : 0;
}

static int HttpGet(const char *url, struct Sink *k, int *status)
{
    CURL *c;
    long code = 0;
    int rc;
    const char *ca = getenv("CURL_CA_BUNDLE") ? getenv("CURL_CA_BUNDLE") : getenv("SSL_CERT_FILE");

    *status = 0;
    if (!sCurl.lib || !(c = sCurl.easy_init()))
        return 0;
    sCurl.easy_setopt(c, CURLOPT_URL, url);
    sCurl.easy_setopt(c, CURLOPT_USERAGENT, "MoemonEmeraldKaiCoop-Updater/" MOEMON_VERSION);
    sCurl.easy_setopt(c, CURLOPT_FOLLOWLOCATION, 1L);
    sCurl.easy_setopt(c, CURLOPT_MAXREDIRS, 8L);
    sCurl.easy_setopt(c, CURLOPT_NOSIGNAL, 1L);
    sCurl.easy_setopt(c, CURLOPT_CONNECTTIMEOUT, 6L);
    sCurl.easy_setopt(c, CURLOPT_LOW_SPEED_LIMIT, 512L);   // under 0.5 KB/s for 30 s: give up
    sCurl.easy_setopt(c, CURLOPT_LOW_SPEED_TIME, 30L);
    sCurl.easy_setopt(c, CURLOPT_WRITEFUNCTION, CurlWrite);
    sCurl.easy_setopt(c, CURLOPT_WRITEDATA, k);
    if (ca && ca[0])
        sCurl.easy_setopt(c, CURLOPT_CAINFO, ca);
    rc = sCurl.easy_perform(c);
    sCurl.easy_getinfo(c, CURLINFO_RESPONSE_CODE, &code);
    *status = (int)code;
    if (rc != 0 && !SDL_AtomicGet(&U.cancel))
        Log("%s: %s", url, sCurl.easy_strerror(rc));
    sCurl.easy_cleanup(c);
    return rc == 0 && code == 200;
}
#endif

// ---------------------------------------------------------------------------
// Versions and update.txt

// 1 when a is a higher version than b ("1.2.10" > "1.2.9")
static int VersionNewer(const char *a, const char *b)
{
    while (*a || *b)
    {
        long x = strtol(a, (char **)&a, 10), y = strtol(b, (char **)&b, 10);
        if (x != y)
            return x > y;
        if (*a == '.')
            a++;
        if (*b == '.')
            b++;
        if ((*a && !isdigit((unsigned char)*a)) || (*b && !isdigit((unsigned char)*b)))
            break;
    }
    return 0;
}

static int ValidName(const char *s)
{
    if (!*s || strlen(s) >= 127)
        return 0;
    for (; *s; s++)
        if (!(isalnum((unsigned char)*s) || *s == '.' || *s == '-' || *s == '_'))
            return 0;
    return 1;
}

// version X / tag vX / <platform> <zip> <bytes> <sha256> <exe>
static int ParseManifest(char *text)
{
    char *line = strtok(text, "\r\n");
    int haveFile = 0;
    U.version[0] = U.tag[0] = 0;
    for (; line; line = strtok(NULL, "\r\n"))
    {
        char key[32], a[128], b[32], c[80], d[128];
        if (line[0] == '#')
            continue;
        if (sscanf(line, "%31s %127s %31s %79s %127s", key, a, b, c, d) == 5 && strcmp(key, UPDATE_PLATFORM) == 0)
        {
            char *end;
            long size = strtol(b, &end, 10);
            int i, hex = strlen(c) == 64;
            for (i = 0; hex && i < 64; i++)
                hex = isxdigit((unsigned char)c[i]);
            if (*end || size <= 0 || (unsigned long)size > MAX_ZIP || !hex || !ValidName(a) || !ValidName(d))
                return 0;
            snprintf(U.zip, sizeof(U.zip), "%s", a);
            U.total = size;
            for (i = 0; i < 64; i++)
                U.sha[i] = (char)tolower((unsigned char)c[i]);
            U.sha[64] = 0;
            snprintf(U.exe, sizeof(U.exe), "%s", d);
            haveFile = 1;
        }
        else if (sscanf(line, "%31s %127s", key, a) == 2)
        {
            if (strcmp(key, "version") == 0)
                snprintf(U.version, sizeof(U.version), "%s", a);
            else if (strcmp(key, "tag") == 0 && ValidName(a))
                snprintf(U.tag, sizeof(U.tag), "%s", a);
        }
    }
    return U.version[0] && isdigit((unsigned char)U.version[0]) && haveFile && (U.tag[0] || getenv("MOEMON_UPDATE_URL"));
}

static void AssetUrl(char *out, size_t size, const char *name, int latest)
{
    const char *base = getenv("MOEMON_UPDATE_URL");
    if (base && base[0])
        snprintf(out, size, "%s%s%s", base, base[strlen(base) - 1] == '/' ? "" : "/", name);
    else if (latest)
        snprintf(out, size, "https://github.com/%s/releases/latest/download/%s", MOEMON_UPDATE_REPO, name);
    else
        snprintf(out, size, "https://github.com/%s/releases/download/%s/%s", MOEMON_UPDATE_REPO, U.tag, name);
}

// ---------------------------------------------------------------------------
// The zip: read, check, unpack next to the exe, swap in

static uint32_t Rd16(const uint8_t *p) { return p[0] | (p[1] << 8); }
static uint32_t Rd32(const uint8_t *p) { return p[0] | (p[1] << 8) | (p[2] << 16) | ((uint32_t)p[3] << 24); }

struct Entry
{
    char name[256];       // relative to the exe's folder, '/' separated
    uint32_t method, crc, csize, usize, local, mode;
};

// A name inside the zip that can only land inside the game's folder
static int SafeRelative(const char *s)
{
    const char *p = s;
    if (!*s || *s == '/' || strchr(s, ':') || strchr(s, '\\'))
        return 0;
    while (*p)
    {
        const char *slash = strchr(p, '/');
        size_t n = slash ? (size_t)(slash - p) : strlen(p);
        if (n == 0 || (n == 1 && p[0] == '.') || (n == 2 && p[0] == '.' && p[1] == '.'))
            return 0;
        p += n;
        if (*p == '/')
            p++;
    }
    return 1;
}

// Player files already there stay as they are
static int Protected(const char *name, const char *path)
{
    size_t n = strlen(name);
    if (!UExists(path))
        return 0;
    if (n >= 4 && strcasecmp(name + n - 4, ".ini") == 0)
        return 1;
    if (strstr(name, ".sav") != NULL)
        return 1;
    return 0;
}

static int ReadEntries(const uint8_t *z, long size, struct Entry **out, int *count)
{
    long eocd = -1, i, off;
    uint32_t n, cdOff, k;
    struct Entry *e;
    char prefix[256] = "";
    int strip = 1;

    for (i = size - 22; i >= 0 && i >= size - 22 - 65535; i--)
        if (Rd32(z + i) == 0x06054b50)
        {
            eocd = i;
            break;
        }
    if (eocd < 0)
        return 0;
    n = Rd16(z + eocd + 10);
    cdOff = Rd32(z + eocd + 16);
    if (n == 0 || n > 20000 || cdOff >= (uint32_t)size)
        return 0;
    e = calloc(n, sizeof(*e));
    if (!e)
        return 0;
    off = cdOff;
    for (k = 0; k < n; k++)
    {
        uint32_t nameLen, extraLen, commentLen, flags, made;
        if (off + 46 > size || Rd32(z + off) != 0x02014b50)
            goto bad;
        made = Rd16(z + off + 4);
        flags = Rd16(z + off + 8);
        e[k].method = Rd16(z + off + 10);
        e[k].crc = Rd32(z + off + 16);
        e[k].csize = Rd32(z + off + 20);
        e[k].usize = Rd32(z + off + 24);
        nameLen = Rd16(z + off + 28);
        extraLen = Rd16(z + off + 30);
        commentLen = Rd16(z + off + 32);
        e[k].mode = (made >> 8) == 3 ? (Rd32(z + off + 38) >> 16) : 0;
        e[k].local = Rd32(z + off + 42);
        if (nameLen == 0 || nameLen >= sizeof(e[k].name) || off + 46 + nameLen > size || (flags & 1))
            goto bad;
        memcpy(e[k].name, z + off + 46, nameLen);
        e[k].name[nameLen] = 0;
        off += 46 + nameLen + extraLen + commentLen;
    }
    // the release zip has everything in one top folder: drop it
    for (k = 0; k < n && strip; k++)
    {
        char *slash = strchr(e[k].name, '/');
        if (!slash)
            strip = 0;
        else if (k == 0)
            snprintf(prefix, sizeof(prefix), "%.*s", (int)(slash - e[k].name + 1), e[k].name);
        else if (strncmp(e[k].name, prefix, strlen(prefix)) != 0)
            strip = 0;
    }
    for (k = 0; k < n; k++)
    {
        if (strip)
            memmove(e[k].name, e[k].name + strlen(prefix), strlen(e[k].name) - strlen(prefix) + 1);
        if (!e[k].name[0] || e[k].name[strlen(e[k].name) - 1] == '/')
        {
            e[k].name[0] = 0;   // a folder (or the top folder itself): made as files need it
            continue;
        }
        if (!SafeRelative(e[k].name) || (e[k].method != 0 && e[k].method != 8))
            goto bad;
    }
    *out = e;
    *count = (int)n;
    return 1;
bad:
    free(e);
    return 0;
}

// Unpacks one entry into memory and checks it. The caller frees it.
static uint8_t *Unpack(const uint8_t *z, long size, const struct Entry *e)
{
    uint32_t nameLen, extraLen;
    const uint8_t *data;
    uint8_t *out;
    long start;
    if ((long)e->local + 30 > size || Rd32(z + e->local) != 0x04034b50)
        return NULL;
    nameLen = Rd16(z + e->local + 26);
    extraLen = Rd16(z + e->local + 28);
    start = (long)e->local + 30 + nameLen + extraLen;
    if (start + (long)e->csize > size)
        return NULL;
    data = z + start;
    if (e->method == 0)
    {
        if (e->csize != e->usize)
            return NULL;
        out = malloc(e->usize ? e->usize : 1);
        if (!out)
            return NULL;
        memcpy(out, data, e->usize);
    }
    else
    {
        int len = 0;
        out = (uint8_t *)stbi_zlib_decode_noheader_malloc((const char *)data, (int)e->csize, &len);
        if (!out)
            return NULL;
        if ((uint32_t)len != e->usize)
        {
            free(out);
            return NULL;
        }
    }
    if (Crc32(out, e->usize) != e->crc)
    {
        free(out);
        return NULL;
    }
    return out;
}

static int WriteFileAll(const char *path, const uint8_t *data, uint32_t n, int exec)
{
    FILE *f;
    MakeParents(path);
    f = UOpen(path, "wb");
    if (!f)
        return 0;
    if (n && fwrite(data, 1, n, f) != n)
    {
        fclose(f);
        URemove(path);
        return 0;
    }
    if (fclose(f) != 0)
    {
        URemove(path);
        return 0;
    }
#ifndef _WIN32
    chmod(path, exec ? 0755 : 0644);
#else
    (void)exec;
#endif
    return 1;
}

static int Install(const uint8_t *z, long size)
{
    struct Entry *e = NULL;
    int n = 0, i, ok = 0, swapped = 0, swaps = 0;
    char *oldNames = NULL;   // each file's .upd_old name, 1024 apiece
    FILE *list;

    if (!ReadEntries(z, size, &e, &n))
    {
        Fail("THE DOWNLOAD IS NOT A GOOD ZIP");
        return 0;
    }
    oldNames = calloc(n, 1024);
    if (!oldNames)
    {
        free(e);
        Fail("OUT OF MEMORY");
        return 0;
    }
    // 1. every file as <name>.upd_new
    for (i = 0; i < n; i++)
    {
        char path[1024], tmp[1100];
        uint8_t *data;
        if (!e[i].name[0])
            continue;
        JoinPath(path, sizeof(path), sExeDir, e[i].name);
        if (Protected(e[i].name, path))
        {
            Log("keeping %s", e[i].name);
            e[i].name[0] = 0;
            continue;
        }
        data = Unpack(z, size, &e[i]);
        if (!data)
        {
            Fail("BAD FILE IN THE ZIP: %.16s", e[i].name);
            goto cleanup_new;
        }
        snprintf(tmp, sizeof(tmp), "%s.upd_new", path);
        if (!WriteFileAll(tmp, data, e[i].usize, (e[i].mode & 0111) || strcmp(e[i].name, U.exe) == 0))
        {
            free(data);
            Fail("COULD NOT WRITE %.20s", e[i].name);
            goto cleanup_new;
        }
        free(data);
    }
    // 2. swap them in, keeping the old ones until the next start
    for (i = 0; i < n; i++)
    {
        char path[1024], tmp[1100], *old = oldNames + (size_t)i * 1024;
        int k;
        if (!e[i].name[0])
            continue;
        if (getenv("MOEMON_UPDATE_TEST_FAIL_SWAP") && atoi(getenv("MOEMON_UPDATE_TEST_FAIL_SWAP")) == swaps++)
        {
            Fail("COULD NOT REPLACE %.12s (TEST)", e[i].name);
            goto rollback;
        }
        JoinPath(path, sizeof(path), sExeDir, e[i].name);
        snprintf(tmp, sizeof(tmp), "%s.upd_new", path);
        if (UExists(path))
        {
            for (k = 0; k < 10; k++)
            {
                snprintf(old, 1024, k ? "%s.upd_old%d" : "%s.upd_old", path, k);
                URemove(old);
                if (!UExists(old) && URename(path, old) == 0)
                    break;
                old[0] = 0;
            }
            if (!old[0])
            {
                Fail("COULD NOT REPLACE %.20s", e[i].name);
                goto rollback;
            }
        }
        if (URename(tmp, path) != 0)
        {
            if (old[0])
                URename(old, path);
            old[0] = 0;
            Fail("COULD NOT REPLACE %.20s", e[i].name);
            goto rollback;
        }
        swapped = i + 1;
    }
    // the old files go at the next start (a running exe can't be deleted on Windows)
    {
        char path[1024];
        JoinPath(path, sizeof(path), sExeDir, CLEANUP_LIST);
        list = UOpen(path, "a");
        if (list)
        {
            for (i = 0; i < n; i++)
                if (oldNames[(size_t)i * 1024])
                    fprintf(list, "%s\n", oldNames + (size_t)i * 1024);
            fclose(list);
        }
    }
    ok = 1;
    goto done;

rollback:
    for (i = swapped - 1; i >= 0; i--)
    {
        char path[1024], *old = oldNames + (size_t)i * 1024;
        if (!e[i].name[0])
            continue;
        JoinPath(path, sizeof(path), sExeDir, e[i].name);
        if (old[0])
            URename(old, path);
        else
            URemove(path);   // it was new in this update
    }
cleanup_new:
    for (i = 0; i < n; i++)
    {
        char path[1024], tmp[1100];
        if (!e[i].name[0])
            continue;
        JoinPath(path, sizeof(path), sExeDir, e[i].name);
        snprintf(tmp, sizeof(tmp), "%s.upd_new", path);
        URemove(tmp);
    }
done:
    free(oldNames);
    free(e);
    return ok;
}

// The files the last update replaced
static void CleanUpLastUpdate(void)
{
    char path[1024], line[1100], left[8192] = "";
    FILE *f;
    int any = 0;
    JoinPath(path, sizeof(path), sExeDir, CLEANUP_LIST);
    f = UOpen(path, "r");
    if (!f)
        return;
    while (fgets(line, sizeof(line), f))
    {
        line[strcspn(line, "\r\n")] = 0;
        if (!line[0] || !strstr(line, ".upd_old"))
            continue;
        if (URemove(line) != 0 && UExists(line))
        {
            if (strlen(left) + strlen(line) + 2 < sizeof(left))
            {
                strcat(left, line);
                strcat(left, "\n");
            }
            any = 1;
        }
    }
    fclose(f);
    if (any)
    {
        f = UOpen(path, "w");
        if (f)
        {
            fputs(left, f);
            fclose(f);
        }
    }
    else
    {
        URemove(path);
    }
}

// ---------------------------------------------------------------------------
// The worker: check, download, verify, install

static int Worker(void *arg)
{
    struct Sink k;
    char url[512], path[1024], hex[65];
    int status;
    uint8_t *zip;
    FILE *f;
    (void)arg;

    // update.txt from the latest release
    memset(&k, 0, sizeof(k));
    k.mem = malloc(MAX_MANIFEST + 1);
    k.cap = MAX_MANIFEST;
    ShaInit(&k.sha);
    if (!k.mem)
    {
        SDL_AtomicSet(&U.phase, PH_NONE);
        return 0;
    }
    AssetUrl(url, sizeof(url), "update.txt", 1);
    if (!HttpGet(url, &k, &status))
    {
        Log("no update information (%s, status %d)", url, status);
        free(k.mem);
        SDL_AtomicSet(&U.phase, PH_NONE);
        return 0;
    }
    k.mem[k.len] = 0;
    if (!ParseManifest(k.mem))
    {
        Log("update.txt has no %s download", UPDATE_PLATFORM);
        free(k.mem);
        SDL_AtomicSet(&U.phase, PH_NONE);
        return 0;
    }
    free(k.mem);
    if (!VersionNewer(U.version, MOEMON_VERSION))
    {
        Log("up to date (this is %s, the latest release is %s)", MOEMON_VERSION, U.version);
        SDL_AtomicSet(&U.phase, PH_NONE);
        return 0;
    }
    Log("version %s is out (this is %s): downloading %s, %ld bytes", U.version, MOEMON_VERSION, U.zip, U.total);

    // the zip, to a file next to the exe
    SDL_AtomicSet(&U.phase, PH_DOWNLOAD);
    JoinPath(path, sizeof(path), sExeDir, DOWNLOAD_FILE);
    memset(&k, 0, sizeof(k));
    k.file = UOpen(path, "wb");
    k.cap = U.total;
    k.progress = 1;
    ShaInit(&k.sha);
    if (!k.file)
    {
        Fail("CANNOT WRITE IN THE GAME FOLDER");
        return 0;
    }
    AssetUrl(url, sizeof(url), U.zip, 0);
    if (!HttpGet(url, &k, &status))
    {
        fclose(k.file);
        URemove(path);
        if (SDL_AtomicGet(&U.cancel))
        {
            Log("skipped by the player");
            SDL_AtomicSet(&U.phase, PH_SKIPPED);
        }
        else
        {
            Fail(status ? "DOWNLOAD FAILED (HTTP %d)" : "DOWNLOAD FAILED (NO CONNECTION)", status);
        }
        return 0;
    }
    if (fclose(k.file) != 0)
    {
        URemove(path);
        Fail("CANNOT WRITE IN THE GAME FOLDER");
        return 0;
    }

    // exactly the file update.txt describes
    SDL_AtomicSet(&U.phase, PH_VERIFY);
    ShaHex(&k.sha, hex);
    if (k.len != U.total || strcmp(hex, U.sha) != 0)
    {
        URemove(path);
        Fail("THE DOWNLOAD IS DAMAGED (CHECKSUM)");
        Log("got %ld bytes, sha256 %s; expected %ld, %s", k.len, hex, U.total, U.sha);
        return 0;
    }
    zip = malloc(U.total);
    f = UOpen(path, "rb");
    if (!zip || !f || fread(zip, 1, U.total, f) != (size_t)U.total)
    {
        if (f)
            fclose(f);
        free(zip);
        URemove(path);
        Fail("COULD NOT READ THE DOWNLOAD");
        return 0;
    }
    fclose(f);
    // read back from the disk: check it again
    {
        struct Sha256 s;
        ShaInit(&s);
        ShaUpdate(&s, zip, U.total);
        ShaHex(&s, hex);
        if (strcmp(hex, U.sha) != 0)
        {
            free(zip);
            URemove(path);
            Fail("THE DOWNLOAD IS DAMAGED (DISK)");
            return 0;
        }
    }

    SDL_AtomicSet(&U.phase, PH_INSTALL);
    if (!Install(zip, U.total))
    {
        free(zip);
        URemove(path);
        return 0;   // Install says why
    }
    free(zip);
    URemove(path);
    Log("installed version %s", U.version);
    SDL_AtomicSet(&U.phase, PH_INSTALLED);
    return 0;
}

// ---------------------------------------------------------------------------
// The screen: a 240x160 frame with a small 5x7 font

static uint16_t sFrame[240 * 160];

#define RGB15(r, g, b) ((r) | ((g) << 5) | ((b) << 10) | 0x8000)
#define C_BG RGB15(3, 4, 7)
#define C_PANEL RGB15(6, 8, 13)
#define C_EDGE RGB15(12, 15, 22)
#define C_TEXT RGB15(31, 31, 31)
#define C_DIM RGB15(19, 21, 26)
#define C_BAR RGB15(14, 31, 21)
#define C_BAR_BG RGB15(4, 5, 9)
#define C_RED RGB15(31, 14, 12)

static const char *const sGlyphs[] = {
    // 7 rows of 5 columns each
    "0.###.#...##..###.#.###..##...#.###.", "1..#...##....#....#....#....#...###.",
    "2.###.#...#....#...#...#...#...#####", "3#####...#...#.....#.....##...#.###.",
    "4...#...##..#.#.#..#.#####...#....#.", "5######....####.....#....##...#.###.",
    "6..##..#...#....####.#...##...#.###.", "7#####....#...#...#...#....#....#...",
    "8.###.#...##...#.###.#...##...#.###.", "9.###.#...##...#.####....#...#..##..",
    "A.###.#...##...#######...##...##...#", "B####.#...##...#####.#...##...#####.",
    "C.###.#...##....#....#....#...#.###.", "D###..#..#.#...##...##...##..#.###..",
    "E######....#....####.#....#....#####", "F######....#....####.#....#....#....",
    "G.###.#...##....#.####...##...#.####", "H#...##...##...#######...##...##...#",
    "I.###...#....#....#....#....#...###.", "J..###...#....#....#....##..#..##...",
    "K#...##..#.#.#..##...#.#..#..#.#...#", "L#....#....#....#....#....#....#####",
    "M#...###.###.#.##.#.##...##...##...#", "N#...##...###..##.#.##..###...##...#",
    "O.###.#...##...##...##...##...#.###.", "P####.#...##...#####.#....#....#....",
    "Q.###.#...##...##...##.#.##..#..##.#", "R####.#...##...#####.#.#..#..#.#...#",
    "S.#####....#.....###.....#....#####.", "T#####..#....#....#....#....#....#..",
    "U#...##...##...##...##...##...#.###.", "V#...##...##...##...##...#.#.#...#..",
    "W#...##...##...##.#.##.#.##.#.#.#.#.", "X#...##...#.#.#...#...#.#.#...##...#",
    "Y#...##...#.#.#...#....#....#....#..", "Z#####....#...#...#...#...#....#####",
    "...........................##...##..", "-................###................",
    "%##..###..#...#...#...#...#..###..##", "/....#....#...#...#...#...#....#....",
    ":......##...##.........##...##......", "(...#...#...#....#....#.....#.....#.",
    ")..#.....#.....#....#....#...#...#..", "!..#....#....#....#....#.........#..",
    ",....................##....#...#....",
};

static const char *Glyph(char ch)
{
    size_t i;
    ch = (char)toupper((unsigned char)ch);
    for (i = 0; i < sizeof(sGlyphs) / sizeof(sGlyphs[0]); i++)
        if (sGlyphs[i][0] == ch)
            return sGlyphs[i] + 1;
    return NULL;
}

static void Fill(int x0, int y0, int x1, int y1, uint16_t c)
{
    int x, y;
    for (y = y0; y <= y1; y++)
        for (x = x0; x <= x1; x++)
            if (x >= 0 && x < 240 && y >= 0 && y < 160)
                sFrame[y * 240 + x] = c;
}

static int TextWidth(const char *s)
{
    return (int)strlen(s) * 6 - 1;
}

static void Text(int x, int y, const char *s, uint16_t c)
{
    for (; *s; s++, x += 6)
    {
        const char *g = Glyph(*s);
        int gx, gy;
        if (!g || strlen(g) < 35)
            continue;
        for (gy = 0; gy < 7; gy++)
            for (gx = 0; gx < 5; gx++)
                if (g[gy * 5 + gx] == '#')
                    Fill(x + gx, y + gy, x + gx, y + gy, c);
    }
}

static void TextCentered(int y, const char *s, uint16_t c)
{
    Text((240 - TextWidth(s)) / 2, y, s, c);
}

static void DrawScreen(int phase)
{
    char line[64];
    Fill(0, 0, 239, 159, C_BG);
    Fill(4, 40, 235, 119, C_EDGE);   // 38 characters fit inside
    Fill(5, 41, 234, 118, C_PANEL);
    TextCentered(50, "MOEMON EMERALD KAI CO-OP", C_TEXT);
    switch (phase)
    {
    case PH_CHECK:
        TextCentered(76, "CHECKING FOR UPDATES...", C_DIM);
        break;
    case PH_DOWNLOAD:
    case PH_VERIFY:
    case PH_INSTALL:
    {
        long got = SDL_AtomicGet(&U.got), total = U.total > 0 ? U.total : 1;
        int w = (int)((long long)got * 176 / total);
        snprintf(line, sizeof(line), "UPDATING TO VERSION %s", U.version);
        TextCentered(66, line, C_TEXT);
        Fill(32, 80, 207, 87, C_EDGE);
        Fill(33, 81, 206, 86, C_BAR_BG);
        if (phase != PH_DOWNLOAD)
            w = 174;
        if (w > 174)
            w = 174;
        if (w > 0)
            Fill(33, 81, 32 + w, 86, C_BAR);
        if (phase == PH_DOWNLOAD)
        {
            snprintf(line, sizeof(line), "%ld.%ld / %ld.%ld MB", got / 1048576, (got % 1048576) * 10 / 1048576,
                     total / 1048576, (total % 1048576) * 10 / 1048576);
            TextCentered(94, line, C_DIM);
            TextCentered(106, "ESC: SKIP FOR NOW", C_DIM);
        }
        else
        {
            TextCentered(94, phase == PH_VERIFY ? "CHECKING THE DOWNLOAD..." : "INSTALLING...", C_DIM);
        }
        break;
    }
    case PH_INSTALLED:
        snprintf(line, sizeof(line), "UPDATED TO VERSION %s!", U.version);
        TextCentered(70, line, C_BAR);
        TextCentered(86, "STARTING THE GAME...", C_DIM);
        break;
    case PH_FAILED:
        TextCentered(66, "THE UPDATE DID NOT WORK:", C_RED);
        TextCentered(78, U.error, C_TEXT);
        TextCentered(96, "STARTING THIS VERSION.", C_DIM);
        TextCentered(106, "IT WILL TRY AGAIN NEXT TIME.", C_DIM);
        break;
    }
    Display_UploadFrame(sFrame);
    Display_Present();
}

// ---------------------------------------------------------------------------

static int TurnedOff(int argc, char **argv)
{
    const char *cfg = getenv("MOEMON_COOP_CONFIG") ? getenv("MOEMON_COOP_CONFIG") : "moemon_coop.ini";
    char line[256];
    FILE *f;
    int i, off = 0;
    for (i = 1; i < argc; i++)
        if (strcmp(argv[i], "--no-update") == 0)
            return 1;
    if (getenv("MOEMON_NO_UPDATE") || getenv("MOEMON_JUST_UPDATED"))
        return 1;
    // headless tests never update unless they point it somewhere
    if (getenv("MOEMON_TEST") && !getenv("MOEMON_UPDATE_URL"))
        return 1;
    f = fopen(cfg, "r");
    if (f)
    {
        while (fgets(line, sizeof(line), f))
        {
            char *p = line, *eq;
            while (*p == ' ' || *p == '\t')
                p++;
            if (strncasecmp(p, "AutoUpdate", 10) == 0 && (eq = strchr(p, '=')))
                off = atoi(eq + 1) == 0;
        }
        fclose(f);
    }
    return off;
}

static void FetchTest(const char *url)
{
    struct Sink k;
    char hex[65], path[1024];
    int status;
    memset(&k, 0, sizeof(k));
    snprintf(path, sizeof(path), "%s/update_fetchtest.bin", sExeDir);
    k.file = UOpen(path, "wb");
    k.cap = MAX_ZIP;
    ShaInit(&k.sha);
    if (!k.file)
        return;
    if (HttpGet(url, &k, &status))
    {
        ShaHex(&k.sha, hex);
        Log("fetch %s: status %d, %ld bytes, sha256 %s", url, status, k.len, hex);
    }
    else
    {
        Log("fetch %s: failed, status %d", url, status);
    }
    fclose(k.file);
    URemove(path);
}

// Starts the newly installed game in place of this one
static int Relaunch(int argc, char **argv)
{
    char path[1024];
    JoinPath(path, sizeof(path), sExeDir, U.exe);
    if (!UExists(path))
        snprintf(path, sizeof(path), "%s", sExePath);
    Log("starting %s", path);
#ifdef _WIN32
    {
        STARTUPINFOW si;
        PROCESS_INFORMATION pi;
        wchar_t *wp = Wide(path), *cmd;
        const wchar_t *orig = GetCommandLineW();
        const wchar_t *rest = orig;
        size_t n;
        int ok;
        (void)argc;
        (void)argv;
        // the same arguments, after the program's own (quoted or not) name
        if (*rest == L'"')
        {
            rest = wcschr(rest + 1, L'"');
            rest = rest ? rest + 1 : orig + wcslen(orig);
        }
        else
        {
            while (*rest && *rest != L' ' && *rest != L'\t')
                rest++;
        }
        n = wcslen(wp) + wcslen(rest) + 4;
        cmd = malloc(n * sizeof(wchar_t));
        if (!wp || !cmd)
            return 0;
        swprintf(cmd, n, L"\"%ls\"%ls", wp, rest);
        SetEnvironmentVariableW(L"MOEMON_JUST_UPDATED", L"1");
        memset(&si, 0, sizeof(si));
        si.cb = sizeof(si);
        ok = CreateProcessW(wp, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
        free(wp);
        free(cmd);
        if (!ok)
        {
            Log("could not start it (error %lu)", (unsigned long)GetLastError());
            return 0;
        }
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        Display_Shutdown();
        SDL_Quit();
        return 1;
    }
#else
    (void)argc;
    setenv("MOEMON_JUST_UPDATED", "1", 1);
    Display_Shutdown();
    SDL_Quit();
    fflush(stdout);
    fflush(stderr);
    execv(path, argv);
    // still here: the new exe didn't start; nothing is left to run this one with
    Log("could not start it: %s", strerror(errno));
    return 1;
#endif
}

// Returns 1 when this process should quit (the new version is starting, or the
// player closed the window), 0 to start the game.
int Updater_Run(int argc, char **argv)
{
    char marker[1024];
    SDL_Thread *thread;
    Uint32 start, shownAt = 0;
    int quit = 0, phase;

    if (!FindExe())
        return 0;
    if (getenv("MOEMON_JUST_UPDATED"))
        Log("running version %s (just updated)", MOEMON_VERSION);
    CleanUpLastUpdate();
#ifndef _WIN32
    if (!LoadCurl())
    {
        if (!TurnedOff(argc, argv))
            Log("no libcurl on this system: no automatic updates");
        return 0;
    }
#endif
    if (getenv("MOEMON_UPDATE_FETCH"))
    {
        FetchTest(getenv("MOEMON_UPDATE_FETCH"));
        return 1;
    }
    if (TurnedOff(argc, argv))
        return 0;
    JoinPath(marker, sizeof(marker), sExeDir, RELEASE_MARKER);
    if (!UExists(marker))
        return 0;   // a source build, not a release folder

    memset(&U, 0, sizeof(U));
    SDL_AtomicSet(&U.phase, PH_CHECK);
    thread = SDL_CreateThread(Worker, "updater", NULL);
    if (!thread)
        return 0;
    start = SDL_GetTicks();
    for (;;)
    {
        SDL_Event ev;
        static int shotDownload, shotResult;
        phase = SDL_AtomicGet(&U.phase);
        if (phase == PH_DOWNLOAD && SDL_AtomicGet(&U.got) > 0)
        {
            const char *shot = getenv("MOEMON_UPDATE_TEST_SHOT");
            if (shot && !shotDownload)
            {
                char bmp[1024];
                DrawScreen(phase);
                snprintf(bmp, sizeof(bmp), "%s_download.bmp", shot);
                Display_SaveOutputBMP(bmp);
                shotDownload = 1;
            }
            if (getenv("MOEMON_UPDATE_TEST_ESC"))
                SDL_AtomicSet(&U.cancel, 1);
        }
        while (SDL_PollEvent(&ev))
        {
            Display_HandleEvent(&ev);
            if (ev.type == SDL_QUIT)
            {
                quit = 1;
                SDL_AtomicSet(&U.cancel, 1);
            }
            else if (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_ESCAPE && phase <= PH_DOWNLOAD)
            {
                SDL_AtomicSet(&U.cancel, 1);
            }
        }
        if (phase == PH_NONE || phase == PH_SKIPPED)
            break;
        if (phase == PH_INSTALLED || phase == PH_FAILED)
        {
            if (!shownAt)
                shownAt = SDL_GetTicks();
            DrawScreen(phase);
            if (getenv("MOEMON_UPDATE_TEST_SHOT") && !shotResult)
            {
                char bmp[1024];
                snprintf(bmp, sizeof(bmp), "%s_result.bmp", getenv("MOEMON_UPDATE_TEST_SHOT"));
                Display_SaveOutputBMP(bmp);
                shotResult = 1;
            }
            if (SDL_GetTicks() - shownAt > (phase == PH_FAILED ? 2500u : 800u) || quit)
                break;
        }
        else if (phase != PH_CHECK || SDL_GetTicks() - start > 300)
        {
            // the check alone is quick: only show it when the connection is slow
            DrawScreen(phase);
        }
        SDL_Delay(16);
    }
    SDL_WaitThread(thread, NULL);
    phase = SDL_AtomicGet(&U.phase);
    if (phase == PH_INSTALLED)
        return Relaunch(argc, argv);
    return quit;
}
