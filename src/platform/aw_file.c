// Moemon Co-op: files written in the background (include/aw_file.h). One worker thread; each job is a whole file's
// bytes, written to "<path>.tmp" and swapped in (Windows: MoveFileEx replacing; elsewhere rename), so a crash midway
// leaves the old file whole. Jobs for a path not started yet are replaced by the newer copy.
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>
#include "aw_file.h"
#ifdef _WIN32
#include <windows.h>
#endif

struct AwFile
{
    char path[600];
    unsigned char *data;
    size_t len, cap;
};

struct Job
{
    char path[600];
    unsigned char *data;
    size_t len;
    int remove;   // (AwFile_Remove: the file goes instead)
};

#define MAX_JOBS 128
static struct Job sJobs[MAX_JOBS];
static int sJobCount;
static int sBusy;   // (the worker is writing a job it took)
static char sBusyPath[600];   // (... this file)
static SDL_mutex *sLock;
static SDL_cond *sWake, *sDone;
static SDL_Thread *sThread;
static int sNoThread;   // (SDL threads unavailable: written at once)

static void WriteOut(const char *path, const unsigned char *data, size_t len)
{
    char tmp[620];
    FILE *f;
    snprintf(tmp, sizeof(tmp), "%s.tmp", path);
    f = fopen(tmp, "wb");
    if (!f)
        return;
    if (len && fwrite(data, 1, len, f) != len)
    {
        fclose(f);
        remove(tmp);
        return;
    }
    if (fclose(f))
    {
        remove(tmp);
        return;
    }
#ifdef _WIN32
    if (!MoveFileExA(tmp, path, MOVEFILE_REPLACE_EXISTING))
    {
        remove(path);
        rename(tmp, path);
    }
#else
    rename(tmp, path);
#endif
}

static int Worker(void *unused)
{
    (void)unused;
    for (;;)
    {
        struct Job job;
        SDL_LockMutex(sLock);
        while (sJobCount == 0)
            SDL_CondWait(sWake, sLock);
        job = sJobs[0];
        memmove(&sJobs[0], &sJobs[1], sizeof(sJobs[0]) * (sJobCount - 1));
        sJobCount--;
        sBusy = 1;
        snprintf(sBusyPath, sizeof(sBusyPath), "%s", job.path);
        SDL_UnlockMutex(sLock);
        if (job.remove)
            remove(job.path);
        else
            WriteOut(job.path, job.data, job.len);
        free(job.data);
        SDL_LockMutex(sLock);
        sBusy = 0;
        sBusyPath[0] = 0;
        SDL_CondBroadcast(sDone);
        SDL_UnlockMutex(sLock);
    }
    return 0;
}

static void Start(void)
{
    if (sThread || sNoThread)
        return;
    sLock = SDL_CreateMutex();
    sWake = SDL_CreateCond();
    sDone = SDL_CreateCond();
    if (sLock && sWake && sDone)
        sThread = SDL_CreateThread(Worker, "moemon_files", NULL);
    if (!sThread)
        sNoThread = 1;
}

static void Queue(const char *path, unsigned char *data, size_t len, int del)
{
    int i;
    Start();
    if (sNoThread)
    {
        if (del)
            remove(path);
        else
            WriteOut(path, data, len);
        free(data);
        return;
    }
    SDL_LockMutex(sLock);
    for (i = 0; i < sJobCount; i++)
        if (strcmp(sJobs[i].path, path) == 0)
        {
            free(sJobs[i].data);   // (the newer copy wins)
            sJobs[i].data = data;
            sJobs[i].len = len;
            sJobs[i].remove = del;
            SDL_UnlockMutex(sLock);
            return;
        }
    while (sJobCount >= MAX_JOBS)   // (full: wait for room; never in practice)
        SDL_CondWait(sDone, sLock);
    snprintf(sJobs[sJobCount].path, sizeof(sJobs[0].path), "%s", path);
    sJobs[sJobCount].data = data;
    sJobs[sJobCount].len = len;
    sJobs[sJobCount].remove = del;
    sJobCount++;
    SDL_CondSignal(sWake);
    SDL_UnlockMutex(sLock);
}

void AwFile_Flush(void)
{
    if (!sThread)
        return;
    SDL_LockMutex(sLock);
    while (sJobCount > 0 || sBusy)
        SDL_CondWait(sDone, sLock);
    SDL_UnlockMutex(sLock);
}

AwFile *AwFile_Open(const char *path)
{
    AwFile *f = calloc(1, sizeof(*f));
    snprintf(f->path, sizeof(f->path), "%s", path);
    return f;
}

static int Grow(AwFile *f, size_t more)
{
    if (f->len + more <= f->cap)
        return 1;
    {
        size_t cap = f->cap ? f->cap : 4096;
        unsigned char *p;
        while (cap < f->len + more)
            cap *= 2;
        p = realloc(f->data, cap);
        if (!p)
            return 0;
        f->data = p;
        f->cap = cap;
    }
    return 1;
}

size_t AwFile_Write(const void *data, size_t size, size_t count, AwFile *f)
{
    size_t n = size * count;
    if (!f || !n || !Grow(f, n))
        return 0;
    memcpy(f->data + f->len, data, n);
    f->len += n;
    return count;
}

int AwFile_Printf(AwFile *f, const char *fmt, ...)
{
    va_list ap;
    int n;
    char small[512];
    va_start(ap, fmt);
    n = vsnprintf(small, sizeof(small), fmt, ap);
    va_end(ap);
    if (n < 0)
        return n;
    if ((size_t)n < sizeof(small))
        return AwFile_Write(small, 1, n, f) == (size_t)n ? n : -1;
    if (!Grow(f, n + 1))
        return -1;
    va_start(ap, fmt);
    vsnprintf((char *)f->data + f->len, n + 1, fmt, ap);
    va_end(ap);
    f->len += n;
    return n;
}

int AwFile_Putc(int c, AwFile *f)
{
    unsigned char b = (unsigned char)c;
    return AwFile_Write(&b, 1, 1, f) ? c : EOF;
}

int AwFile_Puts(const char *s, AwFile *f)
{
    size_t n = strlen(s);
    return AwFile_Write(s, 1, n, f) == n ? (int)n : EOF;
}

int AwFile_Close(AwFile *f)
{
    if (!f)
        return EOF;
    Queue(f->path, f->data, f->len, 0);   // (the job owns the bytes now)
    free(f);
    return 0;
}

void AwFile_Discard(AwFile *f)
{
    if (!f)
        return;
    free(f->data);
    free(f);
}

// At once (a delete is quick): a write of it still queued is dropped, one being written is waited out
void AwFile_Remove(const char *path)
{
    int i;
    if (sThread)
    {
        SDL_LockMutex(sLock);
        for (i = 0; i < sJobCount; i++)
            if (strcmp(sJobs[i].path, path) == 0)
            {
                free(sJobs[i].data);
                memmove(&sJobs[i], &sJobs[i + 1], sizeof(sJobs[0]) * (sJobCount - i - 1));
                sJobCount--;
                i--;
            }
        while (sBusy && strcmp(sBusyPath, path) == 0)
            SDL_CondWait(sDone, sLock);
        SDL_UnlockMutex(sLock);
    }
    remove(path);
}

void AwFile_WriteNow(const char *path, const void *data, size_t size)
{
    AwFile *f = AwFile_Open(path);
    AwFile_Write(data, 1, size, f);
    AwFile_Close(f);
}
