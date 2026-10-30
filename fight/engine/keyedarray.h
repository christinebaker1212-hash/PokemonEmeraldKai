#ifndef MUGEN_KEYEDARRAY_H
#define MUGEN_KEYEDARRAY_H

/*
 * Keyed array: fixed-size elements in slots addressed by index and found by
 * a (key1, key2) pair, e.g. (group, number) for sprites. Used slots lie
 * between first and last; the slot table has one extra entry past capacity,
 * used as the sentinel of the linear key search. Recent lookups (including
 * misses) are kept in a small cache that every insertion or removal clears.
 */

typedef struct KeyedArraySlot {
    int used;               /* 0x00 slot holds an element */
    int serial;             /* 0x04 insertion counter */
    int key1;               /* 0x08 */
    int key2;               /* 0x0C */
} KeyedArraySlot;
MUGEN_ASSERT_SIZE(KeyedArraySlot, 0x10);

#define KEYED_CACHE_SIZE        16

typedef struct KeyedArrayCacheEntry {
    int key1;               /* 0x00 */
    int key2;               /* 0x04 */
    int index;              /* 0x08 slot, -1 for a miss */
} KeyedArrayCacheEntry;
MUGEN_ASSERT_SIZE(KeyedArrayCacheEntry, 0x0C);

typedef struct KeyedArray {
    int unused_00;                      /* 0x00 always 0, never read */
    int elem_size;                      /* 0x04 */
    int count;                          /* 0x08 used slots */
    int capacity;                       /* 0x0C */
    int grow_step;                      /* 0x10 capacity added when full (1) */
    int serial;                         /* 0x14 next insertion counter */
    unsigned char *data;                /* 0x18 capacity * elem_size */
    KeyedArraySlot *slots;              /* 0x1C capacity + 1 entries */
    int iter;                           /* 0x20 iteration cursor (KeyedArray_First/Next) */
    int first;                          /* 0x24 lowest used slot */
    int last;                           /* 0x28 highest used slot */
    KeyedArrayCacheEntry cache[KEYED_CACHE_SIZE]; /* 0x2C recent lookups */
    int cache_size;                     /* 0xEC entries of cache in use (16) */
    int cache_pos;                      /* 0xF0 most recent entry; entries are added downwards */
} KeyedArray;
MUGEN_ASSERT_OFFSET(KeyedArray, data, 0x18);
MUGEN_ASSERT_OFFSET(KeyedArray, cache, 0x2C);
MUGEN_ASSERT_OFFSET(KeyedArray, cache_size, 0xEC);
MUGEN_ASSERT_SIZE(KeyedArray, 0xF4);

#endif /* MUGEN_KEYEDARRAY_H */
