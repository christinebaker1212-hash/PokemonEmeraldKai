/*
 * Keyed arrays (original module 0x466550-0x466A21): elements of a fixed
 * size in slots, found by a (key1, key2) pair through a small cache of
 * recent lookups and a linear search between the first and last used slots.
 * Sprites, palettes, animation actions and many game tables live in them.
 */
#include "mugen.h"

/* Keys of an empty cache entry; no element is expected to use them. */
#define KEYED_CACHE_EMPTY_KEY1  0x7FFFFFFF
#define KEYED_CACHE_EMPTY_KEY2  (-0x7FFFFFFF - 1)

/*
 * 0x00466550: an empty array of capacity elements of elem_size bytes,
 * growing one slot at a time.
 */
KeyedArray *KeyedArray_Create(int capacity, int elem_size)
{
    KeyedArray *list = calloc(1, sizeof(KeyedArray));

    if (list == NULL) {
        return NULL;
    }
    list->unused_00 = 0;
    list->elem_size = elem_size;
    list->count = 0;
    list->capacity = capacity;
    list->serial = 0;
    list->grow_step = 1;
    list->data = calloc(capacity, elem_size);
    if (list->data == NULL) {
        /* slots is always NULL here. */
        if (list->slots != NULL) {
            free(list->slots);
        }
        free(list);
        return NULL;
    }
    list->slots = calloc(capacity + 1, sizeof(KeyedArraySlot));
    if (list->slots == NULL) {
        KeyedArray_Free(list);
        return NULL;
    }
    list->last = 0;
    list->first = 0;
    list->cache_size = KEYED_CACHE_SIZE;
    KeyedArray_ClearCache(list);
    return list;
}

/* 0x00466600: frees the array (not what its elements point to). */
void KeyedArray_Free(KeyedArray *list)
{
    if (list == NULL) {
        return;
    }
    if (list->data != NULL) {
        free(list->data);
    }
    if (list->slots != NULL) {
        free(list->slots);
    }
    free(list);
}

/*
 * 0x00466630: copies elem into a free slot under (key1, key2) and returns the
 * slot, or -1 when the array is full. Keys are not checked for duplicates.
 */
int KeyedArray_Add(KeyedArray *list, const void *elem, int key1, int key2)
{
    int index = KeyedArray_FreeSlot(list);

    if (index == -1) {
        return -1;
    }
    memcpy(list->data + list->elem_size * index, elem, list->elem_size);
    list->count++;
    list->slots[index].used = 1;
    list->slots[index].serial = list->serial;
    list->slots[index].key1 = key1;
    list->slots[index].key2 = key2;
    list->serial++;
    if (index < list->first) {
        list->first = index;
    }
    if (index > list->last) {
        list->last = index;
    }
    KeyedArray_ClearCache(list);
    return index;
}

/*
 * 0x004666B0: frees slot index. The slot is not checked: removing an unused
 * slot still decrements count.
 */
void KeyedArray_Remove(int index, KeyedArray *list)
{
    KeyedArray_ClearCache(list);
    list->slots[index].used = 0;
    if (--list->count == 0) {
        list->last = 0;
        list->first = 0;
        return;
    }
    if (index == list->first) {
        while (!list->slots[list->first].used) {
            list->first++;
        }
    } else if (index == list->last) {
        while (!list->slots[list->last].used) {
            list->last--;
        }
    }
}

/*
 * 0x00466740: grows the array to capacity slots (new slots are zeroed).
 * Returns 0 when capacity is smaller than the current one or memory runs
 * out; if only the slot table cannot grow, data keeps its new size while
 * capacity stays the same.
 */
int KeyedArray_Grow(KeyedArray *list, int capacity)
{
    int old = list->capacity;
    void *p;

    if (capacity < old) {
        return 0;
    }
    if (capacity == old) {
        return 1;
    }
    p = realloc(list->data, list->elem_size * capacity);
    if (p == NULL) {
        return 0;
    }
    list->data = p;
    p = realloc(list->slots, (capacity + 1) * sizeof(KeyedArraySlot));
    if (p == NULL) {
        return 0;
    }
    list->slots = p;
    memset(list->data + list->elem_size * old, 0, (capacity - old) * list->elem_size);
    /* From the old sentinel slot to the new one. */
    memset(&list->slots[old], 0, (capacity - old + 1) * sizeof(KeyedArraySlot));
    list->capacity = capacity;
    return 1;
}

/*
 * 0x004667D0: the slot the next element goes to: the first hole after
 * first, else the slot before first or after last; -1 when full.
 */
int KeyedArray_FreeSlot(KeyedArray *list)
{
    int count = list->count;
    int i;

    if (count >= list->capacity) {
        return -1;
    }
    if (count == 0) {
        return 0;
    }
    if (list->last - list->first + 1 > count) {
        for (i = list->first + 1; list->slots[i].used; i++) {
        }
        return i;
    }
    if (list->first > 0) {
        return list->first - 1;
    }
    if (list->last < list->capacity - 1) {
        return list->last + 1;
    }
    return -1;
}

/*
 * 0x00466840: slot of the element under (key1, key2), or -1. The lookup
 * cache is searched from the most recent entry; on a cache miss the result
 * (even -1) is added to it.
 *
 * The linear search stores the keys in the slot after last as a sentinel
 * (the key fields of that slot change). When there are holes between first
 * and last the sentinel is not enough, since unused slots are skipped and
 * the slot after last is unused: a missing key makes the search run past
 * the slot table until it meets a matching "used" slot in memory, as in the
 * original.
 */
int KeyedArray_Find(KeyedArray *list, int key1, int key2)
{
    int last = list->last;
    KeyedArraySlot *slots;
    KeyedArrayCacheEntry *e;
    int i;

    if (list->count == 0) {
        return -1;
    }
    for (i = list->cache_pos; i < list->cache_size; i++) {
        if (list->cache[i].key1 == key1 && list->cache[i].key2 == key2) {
            return list->cache[i].index;
        }
    }
    for (i = 0; i < list->cache_pos; i++) {
        if (list->cache[i].key1 == key1 && list->cache[i].key2 == key2) {
            return list->cache[i].index;
        }
    }

    slots = list->slots;
    i = list->first - 1;
    slots[last + 1].key1 = key1;
    slots[last + 1].key2 = key2;
    if (last - list->first + 1 == list->count) {
        do {
            i++;
        } while (slots[i].key1 != key1 || slots[i].key2 != key2);
    } else {
        do {
            i++;
        } while (slots[i].key1 != key1 || slots[i].key2 != key2 || !slots[i].used);
    }
    if (i > last) {
        i = -1;
    }

    if (--list->cache_pos < 0) {
        list->cache_pos = list->cache_size - 1;
    }
    e = &list->cache[list->cache_pos];
    e->key2 = key2;
    e->key1 = key1;
    e->index = i;
    return i;
}

/* 0x00466970: the element under (key1, key2), or NULL. */
void *KeyedArray_Get(KeyedArray *list, int key1, int key2)
{
    int index = KeyedArray_Find(list, key1, key2);

    if (index == -1) {
        return NULL;
    }
    return list->data + list->elem_size * index;
}

/* 0x00466990: starts an iteration: the element in the first used slot, or NULL. */
void *KeyedArray_First(KeyedArray *list)
{
    if (list->count == 0) {
        return NULL;
    }
    list->iter = list->first;
    return list->data + list->elem_size * list->first;
}

/* 0x004669B0: the element in the next used slot, or NULL after last. */
void *KeyedArray_Next(KeyedArray *list)
{
    int i = list->iter;

    if (i >= list->last) {
        return NULL;
    }
    do {
        i++;
    } while (!list->slots[i].used);
    list->iter = i;
    return list->data + list->elem_size * i;
}

/* 0x004669E0: empties the lookup cache. */
void KeyedArray_ClearCache(KeyedArray *list)
{
    int i;

    for (i = 0; i < list->cache_size; i++) {
        list->cache[i].key1 = KEYED_CACHE_EMPTY_KEY1;
        list->cache[i].key2 = KEYED_CACHE_EMPTY_KEY2;
        list->cache[i].index = -1;
    }
    list->cache_pos = 0;
}
