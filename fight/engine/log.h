#ifndef MUGEN_LOG_H
#define MUGEN_LOG_H

/* Size of the formatting buffer used by the log and text ring printers. */
#define LOG_LINE_SIZE 0x1FA0

/* A log file (mugen.log, or the match log given with -log). */
typedef struct Log {
    FILE *fp;          /* 0x00 */
    int autoflush;     /* 0x04 fflush after every write */
    int level;         /* 0x08 messages above this level are dropped */
    int owns_fp;       /* 0x0C fclose on Log_Close */
    TextRing *mirror;  /* 0x10 receives a copy of every message */
    int owns_mirror;   /* 0x14 */
} Log;
MUGEN_ASSERT_SIZE(Log, 0x18);

#endif /* MUGEN_LOG_H */
