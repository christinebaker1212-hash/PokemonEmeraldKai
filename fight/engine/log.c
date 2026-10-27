/*
 * Status log (mugen.log) and match log (original modules around 0x40C3F0 and
 * 0x46C270).
 */
#include "mugen.h"

/* Number of mugen.log lines kept in memory for the error dialog. */
#define LOG_MIRROR_LINES 64

/* 0x0040C3F0 */
int Log_Init(void)
{
    FILE *fp;
    Log *log;
    TextRing *mirror;

    fp = fopen("mugen.log", "wt");
    if (fp == NULL) {
        return 0;
    }
    log = Mem_Calloc(1, sizeof(Log));
    if (log == NULL) {
        fclose(fp);
    } else {
        log->level = 9;
        log->fp = fp;
        log->autoflush = 1;
        log->owns_fp = 1;
    }
    g_log = log;
    if (log == NULL) {
        return 0;
    }
    mirror = TextRing_Create(LOG_LINE_SIZE, LOG_MIRROR_LINES);
    if (mirror == NULL) {
        return 0;
    }
    Log_SetMirror(mirror);
    Log_Printf("---------------------------------------------------\n"
               "M.U.G.E.N ver %i.%i.%i %s (%i.%02i.%02i) status log\n"
               "---------------------------------------------------\n",
               MUGEN_VERSION_MAJOR, MUGEN_VERSION_MINOR, MUGEN_VERSION_PATCH, MUGEN_VERSION_TAG,
               MUGEN_BUILD_YEAR, MUGEN_BUILD_MONTH, MUGEN_BUILD_DAY);
    return 1;
}

/* 0x0040C4A0 */
void Log_Printf(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    Log_VWrite(g_log, ap, 1, fmt);
    va_end(ap);
}

/* 0x0046C270 */
void Log_Close(void)
{
    Log *log = g_log;

    if (log == NULL) {
        return;
    }
    if (log->fp != NULL) {
        if (log->mirror != NULL && log->owns_mirror) {
            TextRing_Free(log->mirror);
            log->mirror = NULL;
            log->owns_mirror = 0;
        }
        fflush(log->fp);
        if (log->owns_fp) {
            fclose(log->fp);
        }
    }
    free(log);
}

/* 0x0046C2E0: the log takes ownership of the ring. */
void Log_SetMirror(TextRing *ring)
{
    Log *log = g_log;

    if (log == NULL) {
        return;
    }
    if (log->mirror != NULL && log->owns_mirror) {
        TextRing_Free(log->mirror);
    }
    log->mirror = ring;
    log->owns_mirror = 1;
}

/* 0x0046C320 */
void Log_VWrite(Log *log, va_list ap, int level, const char *fmt)
{
    char buf[LOG_LINE_SIZE];

    if (log == NULL || level > log->level || log->fp == NULL) {
        return;
    }
    _vsnprintf(buf, LOG_LINE_SIZE - 1, fmt, ap);
    buf[LOG_LINE_SIZE - 1] = '\0';
    fprintf(log->fp, "%s", buf);
    if (log->autoflush) {
        fflush(log->fp);
    }
    if (log->mirror != NULL) {
        TextRing_Append(log->mirror, buf);
    }
}

/* 0x0046C3C0: writes to the match log given with -log, if any. */
void MatchLog_Printf(int level, const char *fmt, ...)
{
    va_list ap;

    if (g_matchLog == NULL) {
        return;
    }
    va_start(ap, fmt);
    Log_VWrite(g_matchLog, ap, level, fmt);
    va_end(ap);
}
