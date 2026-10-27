/*
 * Small helpers between the player input buffer and the Lua utilities
 * (original block 0x477EC0-0x478040, no strings): the decoded sprite cache
 * glue (compressed SFF v2 sprites are decoded into g_spriteDecompBuffer and
 * kept there until the space is needed), growable arrays and an empty
 * callback.
 */
#include "mugen.h"

/* DecompBlock and DecompRequest are defined in spritecodec.h. */

/*
 * 0x00477EC0: drops the decoded copy of the compressed sprite data `data`
 * from the sprite cache (blocks are chained by their size field).
 */
void SpriteCache_Remove(void *data)
{
    DecompBuffer *buf = g_spriteDecompBuffer;
    DecompBlock *block, *prev = NULL;

    if (buf == NULL) {
        return;
    }
    block = (DecompBlock *)buf->data;
    while (block->size != 0) {
        if (block->key == data) {
            DecompBuffer_FreeBlock(block, buf, prev);
            return;
        }
        prev = block;
        block = (DecompBlock *)((char *)block + block->size);
    }
}

/*
 * 0x00477EF0: the decoded pixels of the compressed sprite data `data`
 * (SFF v2 compression `format`, size bytes) in *out: the cached copy, or a
 * new block decoded with g_spriteDecoders. Returns 0 without a cache, when
 * no block can be had or for an unknown format (the new block stays
 * allocated then).
 */
int SpriteCache_Get(void *data, int format, int size, void **out)
{
    DecompRequest key;
    int found;
    void *pixels;

    if (g_spriteDecompBuffer == NULL) {
        return 0;
    }
    key.pool = g_spriteDecompBuffer;
    key.size = *(const unsigned int *)data;
    key.key = data;
    key.cost = *(const int *)data;
    pixels = DecompBuffer_Get(&key, &found);
    if (pixels == NULL) {
        return 0;
    }
    if (found) {
        *out = pixels;
        return 1;
    }
    if ((unsigned int)(format - 2) > 10) {
        return 0;
    }
    g_spriteDecoders[format](data, size, pixels);
    *out = pixels;
    return 1;
}

/*
 * 0x00477F80: an empty array of capacity elements (8 when capacity < 1).
 * Quirk: the parts already allocated leak when an allocation fails.
 */
DynArray *DynArray_Create(int capacity, int elem_size)
{
    DynArray *a = calloc(1, sizeof(DynArray));
    void *data;

    if (a == NULL) {
        g_outOfMemory = 1;
    }
    if (capacity < 1) {
        capacity = 8;
    }
    data = calloc(capacity, elem_size);
    if (data == NULL) {
        g_outOfMemory = 1;
    }
    if (a == NULL || data == NULL) {
        return NULL;
    }
    a->capacity = capacity;
    a->data = data;
    a->elem_size = elem_size;
    return a;
}

/* 0x00477FF0: appends a copy of elem (doubling the capacity when full); elem, or NULL. */
void *DynArray_Push(DynArray *a, const void *elem)
{
    if (a->count >= a->capacity) {
        void *data = realloc(a->data, a->elem_size * a->capacity * 2);
        if (data == NULL) {
            return NULL;
        }
        a->data = data;
        a->capacity *= 2;
    }
    memcpy((char *)a->data + a->count * a->elem_size, elem, a->elem_size);
    a->count++;
    return (void *)elem;
}

/* 0x00478040: does nothing (a default callback). */
void Stub_Nop(void)
{
}
