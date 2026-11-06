// Moemon Maker: Publish (docs/MOEMON_MAKER.md decisions 8, 24, 26). The working copy's files (MoemonMaker/base/*.txt,
// MoemonMaker/music/*.kaisong) go to the game's repository as one commit on its default branch, through GitHub's API
// with the token the user pasted into the editor (kept in moemon_maker.ini on this PC only; never logged). Only files
// that differ are uploaded (Git's own blob ids, SHA-1 of "blob <size>\0<data>", are compared), files removed here are
// removed there. The commit carries a "Moemon-Maker:" trailer (CLAUDE.md: the user's work, sessions ask first). The
// repository's maker-pack workflow then builds the pack players' games download.
//
// The publish history lists the commits that changed MoemonMaker/; "Roll back to this" publishes that commit's
// MoemonMaker/ as a new commit (the old tree put back) and puts the same files in the working copy.
//
// Everything runs on a thread of its own; the editor asks how it's going (MakerApi_PublishState).
#include <SDL2/SDL.h>
#include <ctype.h>
#include <dirent.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "mp_version.h"
#include "maker.h"
#include "maker_api.h"

int Updater_Request(const char *method, const char *url, const char *headers, const void *body, long bodyLen,
                    char **out, long *outLen, int *status);

// ---------------------------------------------------------------------------------------------------------------------
// SHA-1 (Git's blob ids) and base64

struct Sha1 { u32 h[5]; u64 len; u8 buf[64]; u32 n; };

static u32 Rol(u32 x, int n) { return (x << n) | (x >> (32 - n)); }

static void Sha1Block(struct Sha1 *s, const u8 *p)
{
    u32 w[80], a, b, c, d, e, t;
    int i;
    for (i = 0; i < 16; i++)
        w[i] = (u32)p[i * 4] << 24 | (u32)p[i * 4 + 1] << 16 | (u32)p[i * 4 + 2] << 8 | p[i * 4 + 3];
    for (i = 16; i < 80; i++)
        w[i] = Rol(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
    a = s->h[0], b = s->h[1], c = s->h[2], d = s->h[3], e = s->h[4];
    for (i = 0; i < 80; i++)
    {
        u32 f, k;
        if (i < 20) f = (b & c) | (~b & d), k = 0x5A827999;
        else if (i < 40) f = b ^ c ^ d, k = 0x6ED9EBA1;
        else if (i < 60) f = (b & c) | (b & d) | (c & d), k = 0x8F1BBCDC;
        else f = b ^ c ^ d, k = 0xCA62C1D6;
        t = Rol(a, 5) + f + e + k + w[i];
        e = d, d = c, c = Rol(b, 30), b = a, a = t;
    }
    s->h[0] += a, s->h[1] += b, s->h[2] += c, s->h[3] += d, s->h[4] += e;
}

static void Sha1Init(struct Sha1 *s)
{
    memset(s, 0, sizeof(*s));
    s->h[0] = 0x67452301, s->h[1] = 0xEFCDAB89, s->h[2] = 0x98BADCFE, s->h[3] = 0x10325476, s->h[4] = 0xC3D2E1F0;
}

static void Sha1Update(struct Sha1 *s, const u8 *p, size_t n)
{
    s->len += n;
    while (n--)
    {
        s->buf[s->n++] = *p++;
        if (s->n == 64)
        {
            Sha1Block(s, s->buf);
            s->n = 0;
        }
    }
}

static void Sha1Hex(struct Sha1 *s, char *out)
{
    u64 bits = s->len * 8;
    u8 pad = 0x80, z = 0, lenb[8];
    int i;
    Sha1Update(s, &pad, 1);
    while (s->n != 56)
        Sha1Update(s, &z, 1);
    for (i = 0; i < 8; i++)
        lenb[i] = (u8)(bits >> (56 - 8 * i));
    Sha1Update(s, lenb, 8);
    for (i = 0; i < 20; i++)
        sprintf(out + i * 2, "%02x", (s->h[i / 4] >> (24 - 8 * (i % 4))) & 0xFF);
}

static void BlobId(const u8 *data, long n, char *out)
{
    struct Sha1 s;
    char head[32];
    int hl = snprintf(head, sizeof(head), "blob %ld", n) + 1;   // (with its NUL)
    Sha1Init(&s);
    Sha1Update(&s, (const u8 *)head, hl);
    Sha1Update(&s, data, n);
    Sha1Hex(&s, out);
}

static char *Base64(const u8 *p, long n)
{
    static const char t[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    char *out = malloc((n + 2) / 3 * 4 + 1), *o = out;
    long i;
    if (!out)
        return NULL;
    for (i = 0; i < n; i += 3)
    {
        u32 v = (u32)p[i] << 16 | (i + 1 < n ? (u32)p[i + 1] << 8 : 0) | (i + 2 < n ? p[i + 2] : 0);
        *o++ = t[v >> 18];
        *o++ = t[(v >> 12) & 63];
        *o++ = i + 1 < n ? t[(v >> 6) & 63] : '=';
        *o++ = i + 2 < n ? t[v & 63] : '=';
    }
    *o = 0;
    return out;
}

// ---------------------------------------------------------------------------------------------------------------------
// A little of GitHub's JSON: the string after "key": from p on

static const char *JsonString(const char *p, const char *key, char *out, int size)
{
    char pat[64];
    const char *at;
    int n = 0;
    snprintf(pat, sizeof(pat), "\"%s\"", key);
    at = p ? strstr(p, pat) : NULL;
    out[0] = 0;
    if (!at)
        return NULL;
    at += strlen(pat);
    while (*at == ' ' || *at == ':' || *at == '\n' || *at == '\r' || *at == '\t')
        at++;
    if (*at != '"')
        return at;
    for (at++; *at && *at != '"' && n < size - 1; at++)
    {
        if (*at == '\\' && at[1])
        {
            at++;
            out[n++] = *at == 'n' ? ' ' : *at;
        }
        else
            out[n++] = *at;
    }
    out[n] = 0;
    return at;
}

// A JSON string's text, escaped for a request's body
static void JsonEscape(char *out, int size, const char *s)
{
    int n = 0;
    for (; *s && n < size - 3; s++)
    {
        if (*s == '"' || *s == '\\')
            out[n++] = '\\', out[n++] = *s;
        else if (*s == '\n')
            out[n++] = '\\', out[n++] = 'n';
        else if ((u8)*s >= 0x20)
            out[n++] = *s;
    }
    out[n] = 0;
}

// ---------------------------------------------------------------------------------------------------------------------
// The job (one at a time, on its own thread)

enum { JOB_PUBLISH, JOB_HISTORY, JOB_ROLLBACK };

static struct
{
    SDL_atomic_t state;          // MAKER_PUBLISH_*
    SDL_Thread *thread;
    int job;
    char token[256];
    char message[512];
    char rollTo[48];
    char result[256];
    int historyCount;
    struct { char sha[48], date[32], message[160]; } history[40];
} P;

static char sApi[256], sRepo[160], sBranch[160];

static void Fail(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(P.result, sizeof(P.result), fmt, ap);
    va_end(ap);
    printf("[maker] publish: %s\n", P.result);
    SDL_AtomicSet(&P.state, MAKER_PUBLISH_FAILED);
}

static char *Api(const char *method, const char *path, const char *body, int *status, const char *accept)
{
    char url[600], headers[640];
    char *out = NULL;
    snprintf(url, sizeof(url), "%s%s", sApi, path);
    snprintf(headers, sizeof(headers), "Authorization: Bearer %s\r\nAccept: %s\r\nX-GitHub-Api-Version: 2022-11-28\r\n%s",
             P.token, accept ? accept : "application/vnd.github+json", body ? "Content-Type: application/json\r\n" : "");
    Updater_Request(method, url, headers, body, body ? (long)strlen(body) : 0, &out, NULL, status);
    return out;
}

static bool8 ApiOk(int status)
{
    return status >= 200 && status < 300;
}

// The repository's default branch and its head commit's sha and tree
static bool8 Head(char *commit, char *tree)
{
    int status;
    char path[400], *r;
    if (!sBranch[0])
    {
        snprintf(path, sizeof(path), "/repos/%s", sRepo);
        r = Api("GET", path, NULL, &status, NULL);
        if (!ApiOk(status) || !JsonString(r, "default_branch", sBranch, sizeof(sBranch)) || !sBranch[0])
        {
            free(r);
            Fail(status == 401 ? "GitHub didn't accept the token (check it in Publish settings)" : "Couldn't reach the repository (HTTP %d)", status);
            return FALSE;
        }
        free(r);
    }
    snprintf(path, sizeof(path), "/repos/%s/git/ref/heads/%s", sRepo, sBranch);
    r = Api("GET", path, NULL, &status, NULL);
    if (!ApiOk(status) || !JsonString(strstr(r ? r : "", "\"object\""), "sha", commit, 48) || !commit[0])
    {
        free(r);
        Fail("Couldn't read the branch %s (HTTP %d)", sBranch, status);
        return FALSE;
    }
    free(r);
    snprintf(path, sizeof(path), "/repos/%s/git/commits/%s", sRepo, commit);
    r = Api("GET", path, NULL, &status, NULL);
    if (!ApiOk(status) || !JsonString(strstr(r ? r : "", "\"tree\""), "sha", tree, 48) || !tree[0])
    {
        free(r);
        Fail("Couldn't read the latest commit (HTTP %d)", status);
        return FALSE;
    }
    free(r);
    return TRUE;
}

// What the repository has in MoemonMaker/DIR: names and blob ids
struct Remote { char name[128], sha[48]; };

static int RemoteDir(const char *dir, const char *ref, struct Remote *out, int max)
{
    int status, n = 0;
    char path[400], *r;
    const char *p;
    snprintf(path, sizeof(path), "/repos/%s/contents/MoemonMaker/%s?ref=%s", sRepo, dir, ref);
    r = Api("GET", path, NULL, &status, NULL);
    if (!ApiOk(status))
    {
        free(r);
        return status == 404 ? 0 : -1;   // (none yet)
    }
    for (p = r; p && n < max; )
    {
        p = JsonString(p, "name", out[n].name, sizeof(out[n].name));
        if (!p)
            break;
        p = JsonString(p, "sha", out[n].sha, sizeof(out[n].sha));
        if (!p)
            break;
        n++;
    }
    free(r);
    return n;
}

static u8 *ReadAll(const char *path, long *n)
{
    FILE *f = fopen(path, "rb");
    u8 *data;
    if (!f)
        return NULL;
    fseek(f, 0, SEEK_END);
    *n = ftell(f);
    fseek(f, 0, SEEK_SET);
    data = malloc(*n + 1);
    if (data && fread(data, 1, *n, f) != (size_t)*n)
    {
        free(data);
        data = NULL;
    }
    fclose(f);
    return data;
}

// The tree entries for one folder: changed / new files uploaded as blobs, files gone from here removed
static bool8 TreeEntriesFor(const char *dir, const char *suffix, const char *ref, char **entries, int *changes)
{
    static struct Remote remote[1024];
    char localDir[1100];
    int nr = RemoteDir(dir, ref, remote, 1024), i;
    DIR *d;
    struct dirent *de;
    u8 seen[1024] = {0};
    if (nr < 0)
    {
        Fail("Couldn't list MoemonMaker/%s on GitHub", dir);
        return FALSE;
    }
    snprintf(localDir, sizeof(localDir), "%s/%s", MakerApi_EditsDir(), dir);
    if ((d = opendir(localDir)) != NULL)
    {
        while ((de = readdir(d)) != NULL)
        {
            size_t nl = strlen(de->d_name), sl = strlen(suffix);
            char path[1300], id[48], *b64, *body, *r, entry[400], blob[48];
            long size;
            u8 *data;
            int status;
            if (nl <= sl || strcmp(de->d_name + nl - sl, suffix) != 0)
                continue;
            snprintf(path, sizeof(path), "%s/%s", localDir, de->d_name);
            data = ReadAll(path, &size);
            if (!data)
                continue;
            BlobId(data, size, id);
            for (i = 0; i < nr; i++)
                if (strcmp(remote[i].name, de->d_name) == 0)
                {
                    seen[i] = 1;
                    break;
                }
            if (i < nr && strcmp(remote[i].sha, id) == 0)
            {
                free(data);
                continue;   // (the same there)
            }
            b64 = Base64(data, size);
            free(data);
            body = malloc(strlen(b64) + 64);
            sprintf(body, "{\"content\":\"%s\",\"encoding\":\"base64\"}", b64);
            free(b64);
            snprintf(path, sizeof(path), "/repos/%s/git/blobs", sRepo);
            r = Api("POST", path, body, &status, NULL);
            free(body);
            if (!ApiOk(status) || !JsonString(r, "sha", blob, sizeof(blob)) || !blob[0])
            {
                free(r);
                closedir(d);
                Fail("Couldn't upload %s (HTTP %d)", de->d_name, status);
                return FALSE;
            }
            free(r);
            snprintf(entry, sizeof(entry), "%s{\"path\":\"MoemonMaker/%s/%s\",\"mode\":\"100644\",\"type\":\"blob\",\"sha\":\"%s\"}",
                     **entries ? "," : "", dir, de->d_name, blob);
            *entries = realloc(*entries, strlen(*entries) + strlen(entry) + 1);
            strcat(*entries, entry);
            (*changes)++;
        }
        closedir(d);
    }
    for (i = 0; i < nr; i++)
        if (!seen[i])
        {
            // (gone from here: gone there)
            char entry[400];
            snprintf(entry, sizeof(entry), "%s{\"path\":\"MoemonMaker/%s/%s\",\"mode\":\"100644\",\"type\":\"blob\",\"sha\":null}",
                     **entries ? "," : "", dir, remote[i].name);
            *entries = realloc(*entries, strlen(*entries) + strlen(entry) + 1);
            strcat(*entries, entry);
            (*changes)++;
        }
    return TRUE;
}

// A tree and a commit on the head, and the branch moved to it
static bool8 CommitTree(const char *headCommit, const char *headTree, const char *entries, const char *message, char *newCommit)
{
    int status;
    char path[400], tree[48], *body, *r, msg[700];
    size_t len = strlen(entries) + 200;
    body = malloc(len);
    snprintf(body, len, "{\"base_tree\":\"%s\",\"tree\":[%s]}", headTree, entries);
    snprintf(path, sizeof(path), "/repos/%s/git/trees", sRepo);
    r = Api("POST", path, body, &status, NULL);
    free(body);
    if (!ApiOk(status) || !JsonString(r, "sha", tree, sizeof(tree)) || !tree[0])
    {
        free(r);
        Fail("Couldn't make the commit's tree (HTTP %d)", status);
        return FALSE;
    }
    free(r);
    JsonEscape(msg, sizeof(msg), message);
    len = strlen(msg) + 300;
    body = malloc(len);
    snprintf(body, len, "{\"message\":\"%s\\n\\nMoemon-Maker: %s\",\"tree\":\"%s\",\"parents\":[\"%s\"]}", msg, MOEMON_VERSION,
             tree, headCommit);
    snprintf(path, sizeof(path), "/repos/%s/git/commits", sRepo);
    r = Api("POST", path, body, &status, NULL);
    free(body);
    if (!ApiOk(status) || !JsonString(r, "sha", newCommit, 48) || !newCommit[0])
    {
        free(r);
        Fail("Couldn't make the commit (HTTP %d)", status);
        return FALSE;
    }
    free(r);
    snprintf(path, sizeof(path), "/repos/%s/git/refs/heads/%s", sRepo, sBranch);
    {
        char b[160];
        snprintf(b, sizeof(b), "{\"sha\":\"%s\",\"force\":false}", newCommit);
        r = Api("PATCH", path, b, &status, NULL);
    }
    free(r);
    if (!ApiOk(status))
    {
        Fail(status == 422 ? "The branch moved while publishing: publish again" : "Couldn't move the branch (HTTP %d)", status);
        return FALSE;
    }
    return TRUE;
}

static void DoPublish(void)
{
    char commit[48], tree[48], newCommit[48];
    char *entries = calloc(1, 1);
    int changes = 0;
    if (!Head(commit, tree) || !TreeEntriesFor("base", ".txt", sBranch, &entries, &changes)
     || !TreeEntriesFor("music", ".kaisong", sBranch, &entries, &changes)
     || !TreeEntriesFor("lua", ".lua", sBranch, &entries, &changes)
     || !TreeEntriesFor("tilesets", ".txt", sBranch, &entries, &changes)
     || !TreeEntriesFor("anims", ".txt", sBranch, &entries, &changes)
     || !TreeEntriesFor("anims", ".png", sBranch, &entries, &changes)
     || !TreeEntriesFor("models", ".txt", sBranch, &entries, &changes)
     || !TreeEntriesFor("models", ".glb", sBranch, &entries, &changes))
    {
        free(entries);
        return;
    }
    if (!changes)
    {
        free(entries);
        snprintf(P.result, sizeof(P.result), "Nothing to publish: GitHub has the same files.");
        SDL_AtomicSet(&P.state, MAKER_PUBLISH_DONE);
        return;
    }
    if (!CommitTree(commit, tree, entries, P.message, newCommit))
    {
        free(entries);
        return;
    }
    free(entries);
    snprintf(P.result, sizeof(P.result), "Published %d %s (%.7s). Players get it at their next start.", changes,
             changes == 1 ? "file" : "files", newCommit);
    printf("[maker] publish: %s\n", P.result);
    SDL_AtomicSet(&P.state, MAKER_PUBLISH_DONE);
}

static void DoHistory(void)
{
    int status;
    char path[400], *r;
    const char *p;
    char commit[48], tree[48];
    if (!Head(commit, tree))
        return;
    snprintf(path, sizeof(path), "/repos/%s/commits?sha=%s&path=MoemonMaker&per_page=40", sRepo, sBranch);
    r = Api("GET", path, NULL, &status, NULL);
    if (!ApiOk(status))
    {
        free(r);
        Fail("Couldn't read the publish history (HTTP %d)", status);
        return;
    }
    P.historyCount = 0;
    for (p = r; p && P.historyCount < (int)ARRAY_COUNT(P.history); )
    {
        // each commit: "sha", then its "commit": {"author": {"date"}, "message"}
        __typeof__(&P.history[0]) h = &P.history[P.historyCount];
        const char *next = JsonString(p, "sha", h->sha, sizeof(h->sha));
        if (!next)
            break;
        p = JsonString(next, "date", h->date, sizeof(h->date));
        if (!p)
            break;
        p = JsonString(p, "message", h->message, sizeof(h->message));
        if (!p)
            break;
        {
            // (to the next commit: past this one's "parents")
            const char *par = strstr(p, "\"parents\"");
            p = par ? strchr(par, ']') : NULL;
        }
        P.historyCount++;
    }
    free(r);
    snprintf(P.result, sizeof(P.result), "%d publishes.", P.historyCount);
    SDL_AtomicSet(&P.state, MAKER_PUBLISH_DONE);
}

// Roll back: that commit's MoemonMaker/ put back as a new commit, and its files in the working copy
static void DoRollback(void)
{
    int status, i, n;
    char path[600], commit[48], tree[48], oldTree[48], sub[48], entry[200], newCommit[48], *r;
    const char *p;
    static struct Remote files[1024];
    if (!Head(commit, tree))
        return;
    snprintf(path, sizeof(path), "/repos/%s/git/commits/%s", sRepo, P.rollTo);
    r = Api("GET", path, NULL, &status, NULL);
    if (!ApiOk(status) || !JsonString(strstr(r ? r : "", "\"tree\""), "sha", oldTree, sizeof(oldTree)))
    {
        free(r);
        Fail("Couldn't read that publish (HTTP %d)", status);
        return;
    }
    free(r);
    snprintf(path, sizeof(path), "/repos/%s/git/trees/%s", sRepo, oldTree);
    r = Api("GET", path, NULL, &status, NULL);
    sub[0] = 0;
    // (its entries, one by one, to MoemonMaker's: then its "sha")
    {
        char name[160];
        p = r;
        while (p && (p = JsonString(p, "path", name, sizeof(name))) != NULL && strcmp(name, "MoemonMaker") != 0)
            ;
    }
    if (!ApiOk(status) || !p || !JsonString(p, "sha", sub, sizeof(sub)) || !sub[0])
    {
        free(r);
        Fail("That publish has no MoemonMaker folder");
        return;
    }
    free(r);
    snprintf(entry, sizeof(entry), "{\"path\":\"MoemonMaker\",\"mode\":\"040000\",\"type\":\"tree\",\"sha\":\"%s\"}", sub);
    {
        char msg[200];
        snprintf(msg, sizeof(msg), "Moemon Maker: back to the publish %.7s", P.rollTo);
        if (!CommitTree(commit, tree, entry, msg, newCommit))
            return;
    }
    // the working copy: the same files
    {
        static const char *const sDirs[] = {"base", "music", "lua", "tilesets", "anims", "models"};
        int d;
        for (d = 0; d < (int)(sizeof(sDirs) / sizeof(sDirs[0])); d++)
        {
            char localDir[1100];
            DIR *dd;
            struct dirent *de;
            n = RemoteDir(sDirs[d], newCommit, files, 1024);
            snprintf(localDir, sizeof(localDir), "%s/%s", MakerApi_EditsDir(), sDirs[d]);
            // (what isn't there now goes from here too)
            if ((dd = opendir(localDir)) != NULL)
            {
                while ((de = readdir(dd)) != NULL)
                {
                    bool8 keep = de->d_name[0] == '.';
                    for (i = 0; i < n && !keep; i++)
                        keep = strcmp(files[i].name, de->d_name) == 0;
                    if (!keep)
                    {
                        snprintf(path, sizeof(path), "%s/%s", localDir, de->d_name);
                        remove(path);
                    }
                }
                closedir(dd);
            }
            for (i = 0; i < n; i++)
            {
                long len = 0;
                FILE *f;
                char *data;
                snprintf(path, sizeof(path), "%s/repos/%s/contents/MoemonMaker/%s/%s?ref=%s", sApi, sRepo, sDirs[d], files[i].name, newCommit);
                {
                    char headers[400];
                    snprintf(headers, sizeof(headers), "Authorization: Bearer %s\r\nAccept: application/vnd.github.raw\r\n", P.token);
                    data = NULL;
                    if (!Updater_Request("GET", path, headers, NULL, 0, &data, &len, &status))
                    {
                        free(data);
                        Fail("Couldn't download %s (HTTP %d)", files[i].name, status);
                        return;
                    }
                }
                snprintf(path, sizeof(path), "%s/%s", localDir, files[i].name);
                if ((f = fopen(path, "wb")) != NULL)
                {
                    fwrite(data, 1, len, f);
                    fclose(f);
                }
                free(data);
            }
        }
    }
    snprintf(P.result, sizeof(P.result), "Back to the publish %.7s (published again as %.7s).", P.rollTo, newCommit);
    printf("[maker] publish: %s\n", P.result);
    SDL_AtomicSet(&P.state, MAKER_PUBLISH_ROLLED_BACK);
}

static int Worker(void *arg)
{
    (void)arg;
    switch (P.job)
    {
    case JOB_PUBLISH: DoPublish(); break;
    case JOB_HISTORY: DoHistory(); break;
    case JOB_ROLLBACK: DoRollback(); break;
    }
    return 0;
}

static int Start(int job, const char *token)
{
    const char *api = getenv("MOEMON_GITHUB_API"), *repo = getenv("MOEMON_GITHUB_REPO");
    int st = SDL_AtomicGet(&P.state);
    if (st == MAKER_PUBLISH_WORKING || !token || !token[0])
        return 0;
    if (P.thread)
    {
        SDL_WaitThread(P.thread, NULL);
        P.thread = NULL;
    }
    snprintf(sApi, sizeof(sApi), "%s", api && api[0] ? api : "https://api.github.com");
    snprintf(sRepo, sizeof(sRepo), "%s", repo && repo[0] ? repo : MOEMON_UPDATE_REPO);
    snprintf(P.token, sizeof(P.token), "%s", token);
    P.job = job;
    P.result[0] = 0;
    SDL_AtomicSet(&P.state, MAKER_PUBLISH_WORKING);
    P.thread = SDL_CreateThread(Worker, "maker-publish", NULL);
    if (!P.thread)
    {
        Fail("Couldn't start");
        return 0;
    }
    return 1;
}

int MakerApi_Publish(const char *token, const char *message)
{
    snprintf(P.message, sizeof(P.message), "%s", message && message[0] ? message : "Moemon Maker: edits");
    return Start(JOB_PUBLISH, token);
}

int MakerApi_PublishHistory(const char *token)
{
    return Start(JOB_HISTORY, token);
}

int MakerApi_PublishRollback(const char *token, const char *sha)
{
    snprintf(P.rollTo, sizeof(P.rollTo), "%s", sha);
    return Start(JOB_ROLLBACK, token);
}

int MakerApi_PublishState(const char **result)
{
    if (result)
        *result = P.result;
    return SDL_AtomicGet(&P.state);
}

int MakerApi_PublishHistoryCount(void)
{
    return P.historyCount;
}

int MakerApi_PublishHistoryItem(int i, const char **sha, const char **date, const char **message)
{
    if (i < 0 || i >= P.historyCount)
        return 0;
    *sha = P.history[i].sha;
    *date = P.history[i].date;
    *message = P.history[i].message;
    return 1;
}

// The state seen (the editor read the result)
void MakerApi_PublishAcknowledge(void)
{
    int st = SDL_AtomicGet(&P.state);
    if (st != MAKER_PUBLISH_WORKING)
        SDL_AtomicSet(&P.state, MAKER_PUBLISH_IDLE);
}
