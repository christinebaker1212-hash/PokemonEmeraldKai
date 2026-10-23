// Moemon Co-op: storage the server keeps for each server/room, saved to disk.
//
//   * the shared PC: 14 boxes of 30 Pokémon and a list of items anyone in the
//     room can put in and take out. The server decides every move (a player
//     picks a Pokémon up into their "hand" here), so two players can never both
//     take the same Pokémon, and one in a hand when its player leaves goes back
//     into a free slot.
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

struct Hand
{
    uint8_t count;
    uint8_t mons[MP_STORE_HAND][MP_STORE_MON_BYTES];
};

struct Store
{
    int used;
    uint8_t key[MP_ROOM_KEY_SIZE];
    uint8_t monUsed[MP_STORE_MONS];
    uint8_t mons[MP_STORE_MONS][MP_STORE_MON_BYTES];
    uint8_t boxNames[MP_SHARED_BOXES][MP_STORE_BOXNAME];
    uint8_t wallpapers[MP_SHARED_BOXES];
    struct Hand hands[MP_MAX_PLAYERS];
    struct StoreItem items[MP_STORE_ITEMS];
    struct StoreCounter counters[MP_STORE_COUNTERS];
    uint32_t touch;
    time_t lastUse;
};

static struct Store *sStores[MAX_STORES];
static uint8_t sReply[MP_MAX_MSG];

#define FILE_MAGIC_V1 "MOEST1\n"   // 60 Pokemon in a list
#define FILE_MAGIC "MOEST2\n"      // 14 boxes of 30, box names and wallpapers, the hands
#define MONS_V1 60

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
    fwrite(s->boxNames, 1, sizeof(s->boxNames), f);
    fwrite(s->wallpapers, 1, sizeof(s->wallpapers), f);
    fwrite(s->hands, 1, sizeof(s->hands), f);
    fwrite(s->items, 1, sizeof(s->items), f);
    fwrite(s->counters, 1, sizeof(s->counters), f);
    if (fclose(f) == 0)
    {
        remove(path);
        rename(tmp, path);
    }
}

// "SHARED1".."SHARED14" in the game's characters (A-Z 0xBB.., 0-9 0xA1..), and the stock wallpapers in turn
static void DefaultBoxes(struct Store *s)
{
    static const char sName[] = "SHARED";
    int b, i;
    for (b = 0; b < MP_SHARED_BOXES; b++)
    {
        int n = 0, num = b + 1;
        for (i = 0; sName[i]; i++)
            s->boxNames[b][n++] = (uint8_t)(0xBB + (sName[i] - 'A'));
        if (num >= 10)
            s->boxNames[b][n++] = (uint8_t)(0xA1 + num / 10);
        s->boxNames[b][n++] = (uint8_t)(0xA1 + num % 10);
        s->boxNames[b][n] = 0xFF;
        s->wallpapers[b] = (uint8_t)(b % 12);
    }
}

static int FirstFree(struct Store *s, int from)
{
    int i;
    for (i = 0; i < MP_STORE_MONS; i++)
    {
        int j = (from + i) % MP_STORE_MONS;
        if (!s->monUsed[j])
            return j;
    }
    return -1;
}

// Whatever was in a hand when the server last stopped goes back into the boxes
static void EmptyHandsIntoBoxes(struct Store *s)
{
    int p, i;
    for (p = 0; p < MP_MAX_PLAYERS; p++)
    {
        for (i = 0; i < s->hands[p].count && i < MP_STORE_HAND; i++)
        {
            int slot = FirstFree(s, 0);
            if (slot < 0)
                break;
            s->monUsed[slot] = 1;
            memcpy(s->mons[slot], s->hands[p].mons[i], MP_STORE_MON_BYTES);
        }
        s->hands[p].count = 0;
    }
}

static void Load(struct Store *s)
{
    char path[512], magic[8] = {0};
    FILE *f;
    uint8_t key[MP_ROOM_KEY_SIZE];
    DefaultBoxes(s);
    FileName(s->key, path, sizeof(path));
    f = fopen(path, "rb");
    if (!f)
        return;
    if (fread(magic, 1, strlen(FILE_MAGIC), f) != strlen(FILE_MAGIC) || fread(key, 1, sizeof(key), f) != sizeof(key)
     || memcmp(key, s->key, sizeof(key)) != 0)
    {
        fclose(f);
        return;
    }
    if (memcmp(magic, FILE_MAGIC, strlen(FILE_MAGIC)) == 0)
    {
        if (fread(s->monUsed, 1, sizeof(s->monUsed), f) != sizeof(s->monUsed)
         || fread(s->mons, 1, sizeof(s->mons), f) != sizeof(s->mons)
         || fread(s->boxNames, 1, sizeof(s->boxNames), f) != sizeof(s->boxNames)
         || fread(s->wallpapers, 1, sizeof(s->wallpapers), f) != sizeof(s->wallpapers)
         || fread(s->hands, 1, sizeof(s->hands), f) != sizeof(s->hands)
         || fread(s->items, 1, sizeof(s->items), f) != sizeof(s->items)
         || fread(s->counters, 1, sizeof(s->counters), f) != sizeof(s->counters))
        {
            MPNet_Log("shared storage file %s is damaged; starting empty", path);
            memset(s->monUsed, 0, sizeof(s->monUsed));
            memset(s->hands, 0, sizeof(s->hands));
            memset(s->items, 0, sizeof(s->items));
            memset(s->counters, 0, sizeof(s->counters));
            DefaultBoxes(s);
        }
        EmptyHandsIntoBoxes(s);
    }
    else if (memcmp(magic, FILE_MAGIC_V1, strlen(FILE_MAGIC_V1)) == 0)
    {
        // The old list of 60: in order into SHARED1 and SHARED2
        static uint8_t used[MONS_V1], mons[MONS_V1][MP_STORE_MON_BYTES];
        if (fread(used, 1, sizeof(used), f) == sizeof(used) && fread(mons, 1, sizeof(mons), f) == sizeof(mons)
         && fread(s->items, 1, sizeof(s->items), f) == sizeof(s->items)
         && fread(s->counters, 1, sizeof(s->counters), f) == sizeof(s->counters))
        {
            int i, n = 0;
            for (i = 0; i < MONS_V1; i++)
                if (used[i])
                {
                    s->monUsed[n] = 1;
                    memcpy(s->mons[n], mons[i], MP_STORE_MON_BYTES);
                    n++;
                }
            MPNet_Log("shared storage file %s: %d Pokemon moved into boxes", path, n);
        }
        else
        {
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

static uint32_t Ident32(const uint8_t *p)
{
    return (uint32_t)(p[0] | (p[1] << 8) | (p[2] << 16) | ((uint32_t)p[3] << 24));
}

// Is this the Pokemon with that personality and OT ID?
static int SameMon(const uint8_t *data, uint32_t pers, uint32_t otid)
{
    return Ident32(data) == pers && Ident32(data + 4) == otid;
}

// One box's contents: u8 box, name[9], u8 wallpaper, u8 count, (u8 pos, data)*
static uint16_t BoxMsg(struct Store *s, int box, uint8_t *out)
{
    uint16_t n = 0;
    int i, count = 0, countAt;
    out[n++] = MP_ST_BOX_DATA;
    out[n++] = (uint8_t)box;
    memcpy(out + n, s->boxNames[box], MP_STORE_BOXNAME);
    n += MP_STORE_BOXNAME;
    out[n++] = s->wallpapers[box];
    countAt = n++;
    for (i = 0; i < MP_SHARED_BOX_SLOTS; i++)
    {
        int slot = box * MP_SHARED_BOX_SLOTS + i;
        if (!s->monUsed[slot])
            continue;
        out[n++] = (uint8_t)i;
        memcpy(out + n, s->mons[slot], MP_STORE_MON_BYTES);
        n += MP_STORE_MON_BYTES;
        count++;
    }
    out[countAt] = (uint8_t)count;
    return n;
}

static int HandFind(struct Hand *h, uint32_t pers, uint32_t otid)
{
    int i;
    for (i = 0; i < h->count; i++)
        if (SameMon(h->mons[i], pers, otid))
            return i;
    return -1;
}

static void HandRemove(struct Hand *h, int i)
{
    memmove(h->mons[i], h->mons[i + 1], (size_t)(h->count - i - 1) * MP_STORE_MON_BYTES);
    h->count--;
}

// A Pokemon into a slot, or the next free one after it if that's taken (-1: the boxes are full)
static int PutInSlot(struct Store *s, int slot, const uint8_t *data, uint8_t *changedBoxes)
{
    if (slot < 0 || slot >= MP_STORE_MONS || s->monUsed[slot])
        slot = FirstFree(s, slot < 0 || slot >= MP_STORE_MONS ? 0 : slot);
    if (slot < 0)
        return -1;
    s->monUsed[slot] = 1;
    memcpy(s->mons[slot], data, MP_STORE_MON_BYTES);
    changedBoxes[slot / MP_SHARED_BOX_SLOTS] = 1;
    return slot;
}

static void SendChangedBoxes(struct Store *s, const uint8_t *changedBoxes, MPStoreSendFunc send, void *ctx)
{
    int b;
    for (b = 0; b < MP_SHARED_BOXES; b++)
        if (changedBoxes[b])
            send(ctx, 1, sReply, BoxMsg(s, b, sReply));
}

// A transaction: the commands in order (see MP_TXN_* in mp_net.h)
static void HandleTxn(struct Store *s, int player, const uint8_t *data, uint16_t len, MPStoreSendFunc send, void *ctx)
{
    struct Hand *h = &s->hands[player];
    uint8_t changedBoxes[MP_SHARED_BOXES] = {0};
    uint32_t refused[MP_STORE_HAND][2], lost[MP_STORE_HAND][2];
    int nRefused = 0, nLost = 0, changed = 0, i;
    uint16_t pos = 3, seq = data[1] | (data[2] << 8), n;

    while (pos < len)
    {
        uint8_t cmd = data[pos++];
        if (cmd == MP_TXN_HOLD)
        {
            int slot;
            uint32_t pers, otid;
            if (pos + 10 > len)
                break;
            slot = data[pos] * MP_SHARED_BOX_SLOTS + data[pos + 1];
            pers = Ident32(data + pos + 2);
            otid = Ident32(data + pos + 6);
            pos += 10;
            if (data[pos - 10] >= MP_SHARED_BOXES || data[pos - 9] >= MP_SHARED_BOX_SLOTS)
                continue;
            if (s->monUsed[slot] && SameMon(s->mons[slot], pers, otid) && h->count < MP_STORE_HAND)
            {
                memcpy(h->mons[h->count++], s->mons[slot], MP_STORE_MON_BYTES);
                s->monUsed[slot] = 0;
                changedBoxes[slot / MP_SHARED_BOX_SLOTS] = 1;
                changed = 1;
            }
            else if (nLost < MP_STORE_HAND)
            {
                // someone else got to it first
                lost[nLost][0] = pers;
                lost[nLost][1] = otid;
                nLost++;
            }
        }
        else if (cmd == MP_TXN_PLACE)
        {
            // the Pokemon in the hand with this data's personality and OT ID, put down with this data (the game
            // restores PP and such when a Pokemon goes into a box)
            int slot, k;
            if (pos + 2 + MP_STORE_MON_BYTES > len)
                break;
            slot = (data[pos] < MP_SHARED_BOXES && data[pos + 1] < MP_SHARED_BOX_SLOTS)
                 ? data[pos] * MP_SHARED_BOX_SLOTS + data[pos + 1] : 0;
            k = HandFind(h, Ident32(data + pos + 2), Ident32(data + pos + 6));
            if (k >= 0 && PutInSlot(s, slot, data + pos + 2, changedBoxes) >= 0)
            {
                HandRemove(h, k);
                changed = 1;
            }
            pos += 2 + MP_STORE_MON_BYTES;
        }
        else if (cmd == MP_TXN_IN)
        {
            int slot;
            if (pos + 2 + MP_STORE_MON_BYTES > len)
                break;
            slot = (data[pos] < MP_SHARED_BOXES && data[pos + 1] < MP_SHARED_BOX_SLOTS)
                 ? data[pos] * MP_SHARED_BOX_SLOTS + data[pos + 1] : 0;
            if (PutInSlot(s, slot, data + pos + 2, changedBoxes) >= 0)
                changed = 1;
            else if (nRefused < MP_STORE_HAND)
            {
                refused[nRefused][0] = Ident32(data + pos + 2);
                refused[nRefused][1] = Ident32(data + pos + 6);
                nRefused++;
            }
            pos += 2 + MP_STORE_MON_BYTES;
        }
        else if (cmd == MP_TXN_OUT || cmd == MP_TXN_RELEASE)
        {
            int k;
            if (pos + 8 > len)
                break;
            k = HandFind(h, Ident32(data + pos), Ident32(data + pos + 4));
            pos += 8;
            if (k >= 0)
            {
                HandRemove(h, k);
                changed = 1;
            }
        }
        else if (cmd == MP_TXN_SET)
        {
            int slot;
            if (pos + 10 + MP_STORE_MON_BYTES > len)
                break;
            if (data[pos] < MP_SHARED_BOXES && data[pos + 1] < MP_SHARED_BOX_SLOTS)
            {
                slot = data[pos] * MP_SHARED_BOX_SLOTS + data[pos + 1];
                if (s->monUsed[slot] && SameMon(s->mons[slot], Ident32(data + pos + 2), Ident32(data + pos + 6)))
                {
                    memcpy(s->mons[slot], data + pos + 10, MP_STORE_MON_BYTES);
                    changedBoxes[data[pos]] = 1;
                    changed = 1;
                }
            }
            pos += 10 + MP_STORE_MON_BYTES;
        }
        else if (cmd == MP_TXN_BOXINFO)
        {
            if (pos + 2 + MP_STORE_BOXNAME > len)
                break;
            if (data[pos] < MP_SHARED_BOXES)
            {
                memcpy(s->boxNames[data[pos]], data + pos + 1, MP_STORE_BOXNAME);
                s->boxNames[data[pos]][MP_STORE_BOXNAME - 1] = 0xFF;
                s->wallpapers[data[pos]] = data[pos + 1 + MP_STORE_BOXNAME];
                changedBoxes[data[pos]] = 1;
                changed = 1;
            }
            pos += 2 + MP_STORE_BOXNAME;
        }
        else
            break;   // unknown: the rest can't be read
    }
    if (changed)
        Save(s);
    SendChangedBoxes(s, changedBoxes, send, ctx);
    // the answer: what didn't work, and what's in the hand now
    n = 0;
    sReply[n++] = MP_ST_TXN_RESULT;
    sReply[n++] = seq & 0xFF;
    sReply[n++] = seq >> 8;
    sReply[n++] = (uint8_t)nRefused;
    for (i = 0; i < nRefused; i++)
    {
        Put32(sReply + n, (int32_t)refused[i][0]);
        Put32(sReply + n + 4, (int32_t)refused[i][1]);
        n += 8;
    }
    sReply[n++] = (uint8_t)nLost;
    for (i = 0; i < nLost; i++)
    {
        Put32(sReply + n, (int32_t)lost[i][0]);
        Put32(sReply + n + 4, (int32_t)lost[i][1]);
        n += 8;
    }
    sReply[n++] = h->count;
    for (i = 0; i < h->count && n + MP_STORE_MON_BYTES <= sizeof(sReply); i++)
    {
        memcpy(sReply + n, h->mons[i], MP_STORE_MON_BYTES);
        n += MP_STORE_MON_BYTES;
    }
    send(ctx, 0, sReply, n);
}

void MPStore_PlayerLeft(const uint8_t *roomKey, int player, MPStoreSendFunc send, void *ctx)
{
    struct Store *s;
    uint8_t changedBoxes[MP_SHARED_BOXES] = {0};
    int i;
    if (player < 0 || player >= MP_MAX_PLAYERS || !(s = GetStore(roomKey)) || s->hands[player].count == 0)
        return;
    for (i = 0; i < s->hands[player].count; i++)
        if (PutInSlot(s, 0, s->hands[player].mons[i], changedBoxes) < 0)
            break;   // full (can't be: they came out of these boxes)
    MPNet_Log("shared PC: player %d left holding %d Pokemon; back in the boxes", player, s->hands[player].count);
    s->hands[player].count = 0;
    Save(s);
    SendChangedBoxes(s, changedBoxes, send, ctx);
}

// Handles one request from a player in the room with this key.
// send(ctx, toAll, data, len): toAll = 0 back to the sender, 1 to the whole room, 2 to the room but the sender.
void MPStore_Handle(const uint8_t *roomKey, int player, const uint8_t *data, uint16_t len, MPStoreSendFunc send, void *ctx)
{
    struct Store *s;
    int i;
    if (len < 1 || player < 0 || player >= MP_MAX_PLAYERS || !(s = GetStore(roomKey)))
        return;

    switch (data[0])
    {
    case MP_ST_BOX_GET:
        for (i = 0; i < MP_SHARED_BOXES; i++)
            if (len < 2 || data[1] == 0xFF || data[1] == i)
                send(ctx, 0, sReply, BoxMsg(s, i, sReply));
        break;
    case MP_ST_TXN:
        if (len >= 3)
            HandleTxn(s, player, data, len, send, ctx);
        break;
    case MP_ST_CURSOR:
        // who's where in the shared PC: to everyone else, with the sender's seat
        if (len >= 2 && len < sizeof(sReply) - 1)
        {
            sReply[0] = MP_ST_CURSOR;
            sReply[1] = (uint8_t)player;
            memcpy(sReply + 2, data + 1, len - 1);
            send(ctx, 2, sReply, (uint16_t)(len + 1));
        }
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
