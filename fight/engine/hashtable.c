/*
 * Hash tables and the command tables built on them (original block
 * 0x478CC0-0x479573, strings "\r\n"): separate chaining with prime bucket
 * counts, keys and values copied and freed through callbacks, and a table
 * of named commands (the debug console's) keyed by counted strings that
 * dispatches a command line to the handler of its first word.
 */
#include "mugen.h"

#define HASH_PRIME_COUNT    28
#define HASH_DEFAULT_LOAD   0.7

/* 0x004F1970: bucket counts, each about twice the previous one. */
static const unsigned int hash_primes[HASH_PRIME_COUNT] = {
    11, 23, 53, 97, 193, 389, 769, 1543, 3079, 6151, 12289, 24593, 49157, 98317, 196613, 393241, 786433,
    1572869, 3145739, 6291469, 12582917, 25165843, 50331653, 100663319, 201326611, 402653189, 805306457,
    1610612741
};

/*
 * 0x00478CC0: moves the entries to the next prime bucket count. Returns 1
 * when the table cannot grow (or the buckets cannot be allocated), else 0.
 */
int HashTable_Grow(HashTable *ht)
{
    unsigned int count, i;
    HashEntry **buckets;

    if (ht->prime_index == HASH_PRIME_COUNT - 1) {
        return 1;
    }
    count = hash_primes[ht->prime_index + 1];
    buckets = calloc(count, sizeof(HashEntry *));
    if (buckets == NULL) {
        g_outOfMemory = 1;
        return 1;
    }
    for (i = 0; i < ht->bucket_count; i++) {
        HashEntry *e = ht->buckets[i];
        while (e != NULL) {
            HashEntry *next = e->next;
            unsigned int b = e->hash % count;
            e->next = buckets[b];
            buckets[b] = e;
            e = next;
        }
    }
    free(ht->buckets);
    ht->prime_index++;
    ht->bucket_count = count;
    ht->buckets = buckets;
    return 0;
}

/*
 * 0x00478D60: a table for at least params->size entries (NULL when more
 * than the largest prime). Keys and values stored inline (key_inline /
 * value_inline) get no copy or free callbacks. The load factor defaults to
 * 0.7.
 */
HashTable *HashTable_Create(const HashParams *params)
{
    HashTable *ht;
    int i;

    for (i = 0; hash_primes[i] < params->size; i++) {
        if (i + 1 >= HASH_PRIME_COUNT) {
            return NULL;
        }
    }
    ht = calloc(1, sizeof(HashTable));
    if (ht == NULL) {
        g_outOfMemory = 1;
        return NULL;
    }
    ht->key_inline = params->key_inline;
    ht->value_inline = params->value_inline;
    ht->prime_index = i;
    ht->bucket_count = hash_primes[i];
    ht->buckets = calloc(ht->bucket_count, sizeof(HashEntry *));
    if (ht->buckets == NULL) {
        g_outOfMemory = 1;
    }
    if (ht->buckets == NULL) {
        free(ht);
        return NULL;
    }
    ht->hash = params->hash;
    ht->compare = params->compare;
    if (ht->key_inline == 0) {
        ht->key_copy = params->key_copy;
        ht->key_free = params->key_free;
    }
    if (ht->value_inline == 0) {
        ht->value_copy = params->value_copy;
        ht->value_free = params->value_free;
    }
    ht->max_load = 0.0 < params->max_load ? params->max_load : HASH_DEFAULT_LOAD;
    return ht;
}

/*
 * 0x00478E40: the entry of key (hashed to `hash`) in bucket b, or NULL.
 * Inline keys are compared by address of the key argument.
 */
HashEntry *HashTable_FindEntry(unsigned int b, unsigned int hash, const HashTable *ht, void *key)
{
    HashEntry *e = ht->buckets[b];

    if (ht->key_inline == 0) {
        for (; e != NULL; e = e->next) {
            if (hash == e->hash && ht->compare(key, e->key, ht->key_ud[0], ht->key_ud[1]) == 0) {
                break;
            }
        }
    } else {
        for (; e != NULL; e = e->next) {
            if (hash == e->hash && ht->compare(&key, e, ht->key_ud[0], ht->key_ud[1]) == 0) {
                break;
            }
        }
    }
    return e;
}

/*
 * 0x00478EB0: sets table[key] = value (copies through the callbacks; the
 * old value is freed). Returns 0, or 1 when out of memory or a copy
 * failed.
 */
int HashTable_Insert(HashTable *ht, void *key, void *value)
{
    double load;
    unsigned int hash, b;
    HashEntry *e;
    int added = 0;
    void *k;
    void *v;

    load = (double)(unsigned int)(ht->count + 1) / (double)ht->bucket_count;
    if (ht->key_inline == 0) {
        hash = ht->hash(key, ht->key_ud[0], ht->key_ud[1]);
    } else {
        hash = ht->hash(&key, ht->key_ud[0], ht->key_ud[1]);
    }
    b = hash % ht->bucket_count;
    e = HashTable_FindEntry(b, hash, ht, key);
    if (e == NULL) {
        added = 1;
        if (ht->max_load < load) {
            HashTable_Grow(ht);    /* the result is not checked */
            b = hash % ht->bucket_count;
        }
        e = calloc(1, sizeof(HashEntry));
        if (e == NULL) {
            g_outOfMemory = 1;
            return 1;
        }
        if (ht->key_copy != NULL) {
            k = ht->key_copy(key, ht->key_ud[0], ht->key_ud[1]);
            if (k == NULL) {
                goto fail;
            }
        } else {
            k = key;
        }
        e->key = k;
        e->hash = hash;
    } else {
        k = NULL;   /* not used: the existing entry keeps its key */
    }
    v = value;
    if (ht->value_copy != NULL) {
        v = ht->value_copy(value, ht->value_ud[0], ht->value_ud[1]);
        if (v == NULL) {
            if (!added) {
                return 1;
            }
            goto fail;
        }
    }
    if (!added && ht->value_free != NULL) {
        ht->value_free(e->value, ht->value_ud[0], ht->value_ud[1]);
    }
    e->value = v;
    if (added) {
        e->next = ht->buckets[b];
        ht->buckets[b] = e;
        ht->count++;
    }
    return 0;

fail:
    if (ht->key_copy != NULL && k != NULL && ht->key_free != NULL) {
        ht->key_free(k, ht->key_ud[0], ht->key_ud[1]);
    }
    free(e);
    return 1;
}

/* 0x00479070: the value of key, or NULL. */
void *HashTable_Lookup(const HashTable *ht, void *key)
{
    unsigned int hash = ht->hash(key, ht->key_ud[0], ht->key_ud[1]);
    HashEntry *e = HashTable_FindEntry(hash % ht->bucket_count, hash, ht, key);

    return e != NULL ? e->value : NULL;
}

/* 0x004790B0: frees the entries (keys and values through the callbacks) and the table. */
void HashTable_Free(HashTable *ht)
{
    unsigned int i;

    if (ht->key_free != NULL || ht->value_free != NULL) {
        for (i = 0; i < ht->bucket_count && ht->count != 0; i++) {
            HashEntry *e = ht->buckets[i];
            while (e != NULL) {
                HashEntry *next;
                if (ht->key_free != NULL) {
                    ht->key_free(e->key, ht->key_ud[0], ht->key_ud[1]);
                }
                if (ht->value_free != NULL) {
                    ht->value_free(e->value, ht->value_ud[0], ht->value_ud[1]);
                }
                ht->count--;
                next = e->next;
                free(e);
                e = next;
            }
        }
    }
    free(ht->buckets);
    free(ht);
}

/* ---- counted string keys ---- */

/* 0x00479140: a counted string (length, then the bytes without a terminator) copy of s, or NULL. */
CountedStr *CountedStr_Make(const char *s)
{
    size_t len;
    CountedStr *cs;

    if (s == NULL) {
        return NULL;
    }
    len = strlen(s);
    cs = malloc(len + 4);
    if (cs == NULL) {
        return NULL;
    }
    cs->len = len;
    memcpy(cs->data, s, len);
    return cs;
}

/* 0x00479190: key copy callback: a copy of the counted string. */
void *CountedStr_Copy(const void *key, void *ud1, void *ud2)
{
    const CountedStr *src = key;
    CountedStr *cs = malloc(src->len + 4);

    if (cs != NULL) {
        cs->len = src->len;
        memcpy(cs->data, src->data, src->len);
    }
    return cs;
}

/* 0x004791D0: hash callback of counted strings. */
unsigned int CountedStr_Hash(const void *key, void *ud1, void *ud2)
{
    const CountedStr *cs = key;

    return Hash_Lookup3(cs->data, cs->len);
}

/* 0x004791F0: free callback of keys and values. */
void Hash_FreeCallback(void *p, void *ud1, void *ud2)
{
    free(p);
}

/* 0x00479200: 0 when the counted strings are equal, else 1. */
int CountedStr_Differ(const CountedStr *a, const CountedStr *b)
{
    size_t n = a->len;
    size_t i;

    if (n != b->len) {
        return 1;
    }
    for (i = 0; i < n; i++) {
        if (a->data[i] != b->data[i]) {
            return 1;
        }
    }
    return 0;
}

/* 0x00479260: compare callback of counted strings (CountedStr_Differ). */
int CountedStr_Compare(const void *a, const void *b, void *ud1, void *ud2)
{
    return CountedStr_Differ(a, b);
}

/* ---- command tables ---- */

/* 0x00479550: value copy callback of command tables: a malloc'ed copy of the CmdTableEntry. */
void *CmdTableEntry_Copy(const void *value, void *ud1, void *ud2)
{
    CmdTableEntry *e = malloc(sizeof(CmdTableEntry));

    if (e != NULL) {
        *e = *(const CmdTableEntry *)value;
    }
    return e;
}

/* 0x00479270: an empty case-insensitive command table, or NULL. */
CmdTable *CmdTable_Create(void)
{
    HashParams params;
    CmdTable *t;

    params.max_load = 0.0;
    params.size = 10;
    params.key_ud[0] = NULL;
    params.key_ud[1] = NULL;
    params.key_inline = 0;
    params.key_copy = CountedStr_Copy;
    params.key_free = Hash_FreeCallback;
    params.hash = CountedStr_Hash;
    params.compare = CountedStr_Compare;
    params.value_ud[0] = NULL;
    params.value_ud[1] = NULL;
    params.value_inline = 0;
    params.value_copy = CmdTableEntry_Copy;
    params.value_free = Hash_FreeCallback;
    t = calloc(1, sizeof(CmdTable));
    if (t == NULL) {
        g_outOfMemory = 1;
        return NULL;
    }
    t->table = HashTable_Create(&params);
    if (t->table == NULL) {
        free(t);
        return NULL;
    }
    t->case_sensitive = 0;
    return t;
}

/*
 * 0x00479320: registers handler fn (with arg) under name. Returns 1, or 0
 * when it cannot be added (the key copy leaks then).
 */
int CmdTable_Add(const char *name, CmdHandlerFn fn, void *arg, CmdTable *t)
{
    CmdTableEntry entry;
    CountedStr *key;

    entry.name = name;
    entry.fn = fn;
    entry.help = arg;
    if (name == NULL) {
        return 0;
    }
    key = CountedStr_Make(name);
    if (key == NULL) {
        return 0;
    }
    if (HashTable_Insert(t->table, key, &entry) != 0) {
        return 0;
    }
    free(key);
    return 1;
}

/* The rest of the line after the command word, leading spaces skipped (strtok state continues). */
static char *CmdTable_Rest(void)
{
    char *rest = strtok(NULL, "\r\n");

    if (rest != NULL) {
        while (isspace(*rest)) {
            rest++;
        }
    }
    return rest;
}

/*
 * 0x004793B0: runs a command line: the handler of its first word (lower
 * case unless the table is case sensitive) gets the word and the rest of
 * the line; unknown words go to the handler registered under "" (with the
 * rest of the line). Returns 0 when out of memory, else 1.
 */
int CmdTable_Exec(const char *line, CmdTable *t)
{
    char *buf = malloc(strlen(line) + 1);
    char *word;
    CountedStr *key;
    CmdTableEntry *e;

    if (buf == NULL) {
        return 0;
    }
    strcpy(buf, line);
    word = strtok(buf, " \r\n");
    if (word == NULL) {
        free(buf);
        return 1;
    }
    if (t->case_sensitive == 0) {
        Str_ToLowerCopy(word, word);
    }
    key = CountedStr_Make(word);
    if (key == NULL) {
        free(buf);
        return 0;
    }
    e = HashTable_Lookup(t->table, key);
    free(key);
    if (e != NULL) {
        char *rest = CmdTable_Rest();
        if (e->fn != NULL) {
            e->fn(t, word, rest);
        }
    } else {
        key = CountedStr_Make("");
        if (key != NULL) {
            e = HashTable_Lookup(t->table, key);
            if (e != NULL && e->fn != NULL) {
                char *rest = CmdTable_Rest();
                e->fn(t, word, rest);
            }
            free(key);
        }
    }
    free(buf);
    return 1;
}
