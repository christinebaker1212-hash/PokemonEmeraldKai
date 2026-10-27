#ifndef MUGEN_HASHTABLE_H
#define MUGEN_HASHTABLE_H

/*
 * Hash tables with callbacks and the command tables of the debug console
 * (hashtable.c).
 */

/* Callbacks; ud1 / ud2 are the user data pointers of the keys or values (always NULL here). */
typedef unsigned int (__cdecl *HashFn)(const void *key, void *ud1, void *ud2);
typedef int (__cdecl *HashCompareFn)(const void *a, const void *b, void *ud1, void *ud2);   /* 0 = equal */
typedef void *(__cdecl *HashCopyFn)(const void *p, void *ud1, void *ud2);
typedef void (__cdecl *HashFreeFn)(void *p, void *ud1, void *ud2);

typedef struct HashEntry {
    void *key;                  /* 0x00 */
    void *value;                /* 0x04 */
    unsigned int hash;          /* 0x08 */
    struct HashEntry *next;     /* 0x0C next in the bucket */
} HashEntry;
MUGEN_ASSERT_SIZE(HashEntry, 0x10);

typedef struct HashTable {
    double max_load;            /* 0x00 entries per bucket before growing (0.7) */
    void *key_ud[2];            /* 0x08 */
    void *value_ud[2];          /* 0x10 */
    HashEntry **buckets;        /* 0x18 */
    unsigned int bucket_count;  /* 0x1C a prime */
    unsigned int count;         /* 0x20 entries */
    HashCompareFn compare;      /* 0x24 */
    HashFn hash;                /* 0x28 */
    HashCopyFn key_copy;        /* 0x2C NULL: the key pointer itself is stored */
    HashFreeFn key_free;        /* 0x30 */
    HashCopyFn value_copy;      /* 0x34 */
    HashFreeFn value_free;      /* 0x38 */
    int prime_index;            /* 0x3C index of bucket_count in the prime table */
    int key_inline;             /* 0x40 keys are hashed and compared by address of the argument */
    int value_inline;           /* 0x44 */
} HashTable;
MUGEN_ASSERT_OFFSET(HashTable, buckets, 0x18);
MUGEN_ASSERT_OFFSET(HashTable, prime_index, 0x3C);
MUGEN_ASSERT_SIZE(HashTable, 0x48);

/* Parameters of HashTable_Create. */
typedef struct HashParams {
    unsigned int size;          /* 0x00 entries expected */
    int pad_04;                 /* 0x04 never accessed by the game */
    double max_load;            /* 0x08 <= 0: 0.7 */
    void *key_ud[2];            /* 0x10 not used by HashTable_Create */
    int key_inline;             /* 0x18 */
    HashCopyFn key_copy;        /* 0x1C */
    HashFreeFn key_free;        /* 0x20 */
    HashFn hash;                /* 0x24 */
    HashCompareFn compare;      /* 0x28 */
    int pad_2c;                 /* 0x2C never accessed by the game */
    void *value_ud[2];          /* 0x30 not used by HashTable_Create */
    int value_inline;           /* 0x38 */
    HashCopyFn value_copy;      /* 0x3C */
    HashFreeFn value_free;      /* 0x40 */
} HashParams;
MUGEN_ASSERT_OFFSET(HashParams, max_load, 0x08);
MUGEN_ASSERT_OFFSET(HashParams, hash, 0x24);
MUGEN_ASSERT_OFFSET(HashParams, value_free, 0x40);
MUGEN_ASSERT_SIZE(HashParams, 0x48);

/* A string with its length and no terminator (the keys of command tables). */
typedef struct CountedStr {
    unsigned int len;           /* 0x00 */
    char data[4];               /* 0x04 len bytes (the allocation is len + 4 bytes) */
} CountedStr;

struct CmdTable;

/* A command handler: the table, the command word and the rest of the line (NULL if none); returns 0 or a negative error, which CmdTable_Exec ignores. */
typedef int (__cdecl *CmdHandlerFn)(struct CmdTable *t, char *word, char *rest);

/* A command of a command table (the values of its hash table). */
typedef struct CmdTableEntry {
    const char *name;           /* 0x00 */
    CmdHandlerFn fn;            /* 0x04 */
    const char *help;           /* 0x08 help text (listed by the console's help command) */
} CmdTableEntry;
MUGEN_ASSERT_SIZE(CmdTableEntry, 0x0C);

typedef struct CmdTable {
    int case_sensitive;         /* 0x00 0: command words are lowered first */
    HashTable *table;           /* 0x04 CountedStr -> CmdTableEntry */
} CmdTable;
MUGEN_ASSERT_SIZE(CmdTable, 0x08);

#endif /* MUGEN_HASHTABLE_H */
