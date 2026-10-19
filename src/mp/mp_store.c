// Moemon Co-op: storage the server keeps for each server/room, saved to disk.
//
//   * the shared PC: Pokémon and items anyone in the room can put in and take
//     out (taking is decided by the server, so two players can never both take
//     the same Pokémon)
//   * counters: named numbers for team goals, achievements, streaks and the
//     weekly race ("d:20261019:trainers", "w:2026-42:caught:ALICE", ...)
//
// A room's storage is found by its key (a hash of server name + password), so
// hosting again with the same name and password brings everything back. Files
// are "coop_store_<key>.dat" in the folder the server runs in.
//
// Wire format (MPMSG_C_STORE / MPMSG_S_STORE): u8 op, then the op's data.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#include "mp_net.h"

#define MAX_STORES 64

struct StoreItem
{
    uint16_t item;
    uint16_t qty;
};

struct StoreCounter
{
    char key[MP_STORE_KEY_MAX + 1];
    int32_t value;
    uint32_t touched;
};

struct Store
{
    int used;
    uint8_t key[MP_ROOM_KEY_SIZE];
    uint8_t monUsed[MP_STORE_MONS];
    uint8_t mons[MP_STORE_MONS][MP_STORE_MON_BYTES];
    struct StoreItem items[MP_STORE_ITEMS];
    struct StoreCounter counters[MP_STORE_COUNTERS];
    uint32_t touch;
    time_t lastUse;
};

static struct Store *sStores[MAX_STORES];
static uint8_t sReply[MP_MAX_MSG];

#define FILE_MAGIC "MOEST1\n"

static void FileName(const uint8_t *key, char *out, size_t size)
{
    const char *dir = getenv("MOEMON_STORE_DIR");
    char hex[17];
    int i;
    for (i = 0; i < 8; i++)
        snprintf(hex + i * 2, 3, "%02x", key[i]);
    snprintf(out, size, "%s%scoop_store_%s.dat", dir ? dir : "", dir ? "/" : "", hex);
}

static void Save(struct Store *s)
{
    char path[512], tmp[520];
    FILE *f;
    FileName(s->key, path, sizeof(path));
    snprintf(tmp, sizeof(tmp), "%s.tmp", path);
    f = fopen(tmp, "wb");
    if (!f)
        return;
    fwrite(FILE_MAGIC, 1, strlen(FILE_MAGIC), f);
    fwrite(s->key, 1, MP_ROOM_KEY_SIZE, f);
    fwrite(s->monUsed, 1, sizeof(s->monUsed), f);
    fwrite(s->mons, 1, sizeof(s->mons), f);
    fwrite(s->items, 1, sizeof(s->items), f);
    fwrite(s->counters, 1, sizeof(s->counters), f);
    if (fclose(f) == 0)
    {
        remove(path);
        rename(tmp, path);
    }
}

static void Load(struct Store *s)
{
    char path[512], magic[8] = {0};
    FILE *f;
    uint8_t key[MP_ROOM_KEY_SIZE];
    FileName(s->key, path, sizeof(path));
    f = fopen(path, "rb");
    if (!f)
        return;
    if (fread(magic, 1, strlen(FILE_MAGIC), f) == strlen(FILE_MAGIC) && memcmp(magic, FILE_MAGIC, strlen(FILE_MAGIC)) == 0
     && fread(key, 1, sizeof(key), f) == sizeof(key) && memcmp(key, s->key, sizeof(key)) == 0)
    {
        if (fread(s->monUsed, 1, sizeof(s->monUsed), f) != sizeof(s->monUsed)
         || fread(s->mons, 1, sizeof(s->mons), f) != sizeof(s->mons)
         || fread(s->items, 1, sizeof(s->items), f) != sizeof(s->items)
         || fread(s->counters, 1, sizeof(s->counters), f) != sizeof(s->counters))
        {
            MPNet_Log("shared storage file %s is damaged; starting empty", path);
            memset(s->monUsed, 0, sizeof(s->monUsed));
            memset(s->items, 0, sizeof(s->items));
            memset(s->counters, 0, sizeof(s->counters));
        }
    }
    fclose(f);
}

static struct Store *GetStore(const uint8_t *key)
{
    int i, freeSlot = -1, oldest = -1;
    for (i = 0; i < MAX_STORES; i++)
    {
        if (sStores[i] && memcmp(sStores[i]->key, key, MP_ROOM_KEY_SIZE) == 0)
        {
            sStores[i]->lastUse = time(NULL);
            return sStores[i];
        }
        if (!sStores[i] && freeSlot < 0)
            freeSlot = i;
        if (sStores[i] && (oldest < 0 || sStores[i]->lastUse < sStores[oldest]->lastUse))
            oldest = i;
    }
    if (freeSlot < 0)
    {
        // Everything's saved after each change, so the oldest can just go
        free(sStores[oldest]);
        sStores[oldest] = NULL;
        freeSlot = oldest;
    }
    sStores[freeSlot] = (struct Store *)calloc(1, sizeof(struct Store));
    if (!sStores[freeSlot])
        return NULL;
    memcpy(sStores[freeSlot]->key, key, MP_ROOM_KEY_SIZE);
    Load(sStores[freeSlot]);
    sStores[freeSlot]->lastUse = time(NULL);
    return sStores[freeSlot];
}

static struct StoreCounter *FindCounter(struct Store *s, const char *key, int create)
{
    int i, freeSlot = -1, oldest = -1;
    for (i = 0; i < MP_STORE_COUNTERS; i++)
    {
        struct StoreCounter *c = &s->counters[i];
        if (c->key[0] && strcmp(c->key, key) == 0)
        {
            c->touched = ++s->touch;
            return c;
        }
        if (!c->key[0] && freeSlot < 0)
            freeSlot = i;
        if (c->key[0] && (oldest < 0 || c->touched < s->counters[oldest].touched))
            oldest = i;
    }
    if (!create)
        return NULL;
    if (freeSlot < 0)
        freeSlot = oldest; // full: forget the counter nobody has used for longest
    memset(&s->counters[freeSlot], 0, sizeof(s->counters[freeSlot]));
    snprintf(s->counters[freeSlot].key, sizeof(s->counters[freeSlot].key), "%s", key);
    s->counters[freeSlot].touched = ++s->touch;
    return &s->counters[freeSlot];
}

static void Put32(uint8_t *p, int32_t v)
{
    p[0] = v; p[1] = v >> 8; p[2] = v >> 16; p[3] = v >> 24;
}

static int32_t Get32(const uint8_t *p)
{
    return (int32_t)(p[0] | (p[1] << 8) | (p[2] << 16) | ((uint32_t)p[3] << 24));
}

static uint16_t CounterMsg(uint8_t *out, const struct StoreCounter *c)
{
    int n = (int)strlen(c->key);
    out[0] = MP_ST_CTR_VALUE;
    Put32(out + 1, c->value);
    out[5] = (uint8_t)n;
    memcpy(out + 6, c->key, n);
    return (uint16_t)(6 + n);
}

static int ReadKey(const uint8_t *data, uint16_t len, int pos, char *out)
{
    int n;
    if (pos >= len)
        return 0;
    n = data[pos];
    if (n == 0 || n > MP_STORE_KEY_MAX || pos + 1 + n > len)
        return 0;
    memcpy(out, data + pos + 1, n);
    out[n] = 0;
    return 1;
}

// Handles one request from a player in the room with this key.
// send(ctx, toAll, data, len): toAll = 0 back to the sender, 1 to the whole room.
void MPStore_Handle(const uint8_t *roomKey, const uint8_t *data, uint16_t len, MPStoreSendFunc send, void *ctx)
{
    struct Store *s;
    int i;
    if (len < 1 || !(s = GetStore(roomKey)))
        return;

    switch (data[0])
    {
    case MP_ST_PC_LIST:
    {
        // Every stored Pokémon: u8 count, then (u8 slot, data) each
        uint16_t n = 2;
        uint8_t count = 0;
        sReply[0] = MP_ST_PC_LIST;
        for (i = 0; i < MP_STORE_MONS && n + 1 + MP_STORE_MON_BYTES <= sizeof(sReply); i++)
        {
            if (!s->monUsed[i])
                continue;
            sReply[n++] = (uint8_t)i;
            memcpy(sReply + n, s->mons[i], MP_STORE_MON_BYTES);
            n += MP_STORE_MON_BYTES;
            count++;
        }
        sReply[1] = count;
        send(ctx, 0, sReply, n);
        break;
    }
    case MP_ST_PC_PUT:
        sReply[0] = MP_ST_PC_PUT;
        sReply[1] = 0;
        sReply[2] = 0xFF;
        if (len >= 1 + MP_STORE_MON_BYTES)
        {
            for (i = 0; i < MP_STORE_MONS; i++)
                if (!s->monUsed[i])
                    break;
            if (i < MP_STORE_MONS)
            {
                s->monUsed[i] = 1;
                memcpy(s->mons[i], data + 1, MP_STORE_MON_BYTES);
                Save(s);
                sReply[1] = 1;
                sReply[2] = (uint8_t)i;
            }
        }
        send(ctx, 0, sReply, 3);
        if (sReply[1])
        {
            uint8_t changed = MP_ST_PC_CHANGED;
            send(ctx, 1, &changed, 1);
        }
        break;
    case MP_ST_PC_TAKE:
        sReply[0] = MP_ST_PC_TAKE;
        sReply[1] = 0;
        sReply[2] = len >= 2 ? data[1] : 0xFF;
        if (len >= 2 && data[1] < MP_STORE_MONS && s->monUsed[data[1]])
        {
            s->monUsed[data[1]] = 0;
            memcpy(sReply + 3, s->mons[data[1]], MP_STORE_MON_BYTES);
            Save(s);
            sReply[1] = 1;
            send(ctx, 0, sReply, 3 + MP_STORE_MON_BYTES);
            {
                uint8_t changed = MP_ST_PC_CHANGED;
                send(ctx, 1, &changed, 1);
            }
        }
        else
            send(ctx, 0, sReply, 3);
        break;
    case MP_ST_ITEM_LIST:
    {
        uint16_t n = 2;
        uint8_t count = 0;
        sReply[0] = MP_ST_ITEM_LIST;
        for (i = 0; i < MP_STORE_ITEMS; i++)
        {
            if (!s->items[i].item || !s->items[i].qty)
                continue;
            sReply[n++] = s->items[i].item & 0xFF;
            sReply[n++] = s->items[i].item >> 8;
            sReply[n++] = s->items[i].qty & 0xFF;
            sReply[n++] = s->items[i].qty >> 8;
            count++;
        }
        sReply[1] = count;
        send(ctx, 0, sReply, n);
        break;
    }
    case MP_ST_ITEM_PUT:
    case MP_ST_ITEM_TAKE:
    {
        uint16_t item, qty, done = 0;
        if (len < 5)
            return;
        item = data[1] | (data[2] << 8);
        qty = data[3] | (data[4] << 8);
        for (i = 0; i < MP_STORE_ITEMS; i++)
            if (s->items[i].item == item && s->items[i].qty)
                break;
        if (data[0] == MP_ST_ITEM_PUT && item && qty)
        {
            if (i == MP_STORE_ITEMS)
                for (i = 0; i < MP_STORE_ITEMS; i++)
                    if (!s->items[i].item || !s->items[i].qty)
                        break;
            if (i < MP_STORE_ITEMS)
            {
                uint32_t total = (uint32_t)(s->items[i].item == item ? s->items[i].qty : 0) + qty;
                if (total > 999)
                    total = 999;
                done = (uint16_t)(total - (s->items[i].item == item ? s->items[i].qty : 0));
                s->items[i].item = item;
                s->items[i].qty = (uint16_t)total;
            }
        }
        else if (data[0] == MP_ST_ITEM_TAKE && i < MP_STORE_ITEMS)
        {
            done = qty < s->items[i].qty ? qty : s->items[i].qty;
            s->items[i].qty -= done;
            if (s->items[i].qty == 0)
                s->items[i].item = 0;
        }
        if (done)
            Save(s);
        sReply[0] = data[0];
        sReply[1] = item & 0xFF;
        sReply[2] = item >> 8;
        sReply[3] = done & 0xFF;
        sReply[4] = done >> 8;
        send(ctx, 0, sReply, 5);
        if (done)
        {
            uint8_t changed = MP_ST_ITEMS_CHANGED;
            send(ctx, 1, &changed, 1);
        }
        break;
    }
    case MP_ST_CTR_ADD:
    case MP_ST_CTR_MAX:
    {
        char key[MP_STORE_KEY_MAX + 1];
        struct StoreCounter *c;
        int32_t v;
        if (len < 6 || !ReadKey(data, len, 5, key))
            return;
        v = Get32(data + 1);
        c = FindCounter(s, key, 1);
        if (data[0] == MP_ST_CTR_ADD)
            c->value += v;
        else if (v > c->value)
            c->value = v;
        Save(s);
        send(ctx, 1, sReply, CounterMsg(sReply, c));
        break;
    }
    case MP_ST_CTR_GET:
    {
        // Every counter whose name starts with the prefix, then CTR_END
        char prefix[MP_STORE_KEY_MAX + 1];
        size_t pl;
        if (len < 2 || !ReadKey(data, len, 1, prefix))
            prefix[0] = 0;
        pl = strlen(prefix);
        for (i = 0; i < MP_STORE_COUNTERS; i++)
        {
            struct StoreCounter *c = &s->counters[i];
            if (c->key[0] && strncmp(c->key, prefix, pl) == 0)
                send(ctx, 0, sReply, CounterMsg(sReply, c));
        }
        sReply[0] = MP_ST_CTR_END;
        sReply[1] = (uint8_t)pl;
        memcpy(sReply + 2, prefix, pl);
        send(ctx, 0, sReply, (uint16_t)(2 + pl));
        break;
    }
    }
}
