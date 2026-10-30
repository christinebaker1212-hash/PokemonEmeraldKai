/* Moemon Fight: saving and restoring the fight frame by frame (fight/host/mf_rollback.c) */
#ifndef MF_ROLLBACK_H
#define MF_ROLLBACK_H
#include <stddef.h>

#define MF_RB_FRAMES 10         /* frames kept: going back up to 9 */
#define MF_RB_HOST_SIZE 64      /* the host's own values kept with each frame */

typedef struct MfRbStats {
    unsigned long frames, pages, most_pages, rewinds, frames_back;
    size_t globals, shadow;
} MfRbStats;

/* Starts keeping frames, `frame` being the one just done (the oldest that can be gone back to). 0: can't */
int MfRollback_Begin(unsigned int frame, const void *host);
void MfRollback_End(void);
int MfRollback_On(void);
/* Keeps `frame`, just done (the one after the last kept) */
int MfRollback_Save(unsigned int frame, const void *host);
/* Puts everything back as it was after `frame` (kept, not older than MfRollback_Oldest): 1 when done */
int MfRollback_Restore(unsigned int frame, void *host);
unsigned int MfRollback_Oldest(void);
/* On the engine thread, before it runs the engine (Linux: a stack for the page-write fault handler) */
void MfRollback_ThreadInit(void);
/* The engine as it is now (started up, between sessions), and putting it back so (0: none kept) */
int MfRollback_KeepStart(const void *host);
int MfRollback_RestoreStart(void *host);
void MfRollback_Stats(MfRbStats *out);
/* The engine's globals section */
unsigned char *MfRollback_Globals(size_t *size);

#endif
