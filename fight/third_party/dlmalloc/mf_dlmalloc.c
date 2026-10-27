/* Moemon Fight: dlmalloc 2.8.6 (Doug Lea, MIT-0) as the fight's heap: one mspace in the region fight/host/mf_mem.c
 * reserves, with everything it keeps inside that region (no locks, no system memory of its own), so the region
 * alone is the heap's whole state. */
#define ONLY_MSPACES 1
#define MSPACES 1
#define USE_LOCKS 0
#define HAVE_MMAP 0
#define HAVE_MORECORE 0
#define USE_DL_PREFIX 1
#define FOOTERS 0
#define MALLOC_FAILURE_ACTION
#define DEFAULT_TRIM_THRESHOLD MAX_SIZE_T
#ifdef _WIN32
/* mingw's FORCEINLINE is extern inline: dlmalloc's static helpers need a plain one */
#include <windows.h>
#undef FORCEINLINE
#define FORCEINLINE __inline __attribute__((always_inline))
#endif
#include "dlmalloc.c"
