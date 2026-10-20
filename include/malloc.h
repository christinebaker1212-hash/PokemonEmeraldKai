#ifndef GUARD_ALLOC_H
#define GUARD_ALLOC_H


#define FREE_AND_SET_NULL(ptr)          \
{                                       \
    Free(ptr);                          \
    ptr = NULL;                         \
}

#define TRY_FREE_AND_SET_NULL(ptr) if (ptr != NULL) FREE_AND_SET_NULL(ptr)

#ifdef PORTABLE
// Moemon Co-op: the PC build's heap is 4.5x the GBA's. 64-bit block headers and
// pointers, and six battlers' sprite buffers, filled the GBA-sized one in battle:
// the summary screen (forgetting a move to learn a new one) then crashed
#define HEAP_SIZE 0x80000
#else
#define HEAP_SIZE 0x1C000
#endif
extern u8 gHeap[HEAP_SIZE];

void *Alloc(u32 size);
void *AllocZeroed(u32 size);
void Free(void *pointer);
void InitHeap(void *heapStart, u32 heapSize);

#endif // GUARD_ALLOC_H
