// Moemon Maker, M2c: trainers (docs/MOEMON_MAKER.md "Build order" M2, "The database" Trainers). The user changes a
// trainer's name, class, picture, items, AI, double battle and party in the editor; the edits are MoemonMaker/trainers.txt,
// one block a trainer, each with a hash of the game's own trainer (was=) so a later update that changes the same trainer
// wins and the editor shows the clash:
//
//   trainer TRAINER_CALVIN_1 was=0x1A2B3C4D
//     name CALVIN
//     class 5
//     pic 12
//     music 1
//     ai 7
//     double 0
//     items ITEM_POTION ITEM_NONE ITEM_NONE ITEM_NONE
//     mon SPECIES_ZIGZAGOON 5 iv=0 item=ITEM_NONE moves=MOVE_TACKLE,MOVE_GROWL,MOVE_NONE,MOVE_NONE
//
// The game reads its trainers through gTrainers (include/data.h: the table gTrainersTable points at), so a patched copy
// of the table is all it takes: battles, level scaling (mp_levelscale.c), TEAM PADDING and co-op joining read the
// edited party like any other. The trainer's "already beaten" flag stays the game's own.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "data.h"
#include "constants/opponents.h"
#include "constants/trainers.h"
#include "constants/characters.h"
#include "maker.h"
#include "maker_api.h"

void MP_AsciiToGame(u8 *dst, const char *src, int maxLen);
const char *Maker_ReadDir(void);
extern const struct Trainer gTrainersBase[];
extern const u32 gTrainersBaseCount;

static struct Trainer *sTable;   // the patched copy (NULL: the game's own)
static u16 sEdited;

// A hash of a trainer as the game has it (its fields and party): an edit applies only while it's the same
u32 Maker_TrainerHash(u16 id)
{
    const struct Trainer *t = &gTrainersBase[id];
    u32 h = 2166136261u, i;
    const u8 *p = (const u8 *)t;
    size_t partyBytes;
    for (i = 0; i < offsetof(struct Trainer, party); i++)
        h = (h ^ p[i]) * 16777619u;
    switch (t->partyFlags & 3)
    {
    case 0: partyBytes = sizeof(struct TrainerMonNoItemDefaultMoves); break;
    case F_TRAINER_PARTY_CUSTOM_MOVESET: partyBytes = sizeof(struct TrainerMonNoItemCustomMoves); break;
    case F_TRAINER_PARTY_HELD_ITEM: partyBytes = sizeof(struct TrainerMonItemDefaultMoves); break;
    default: partyBytes = sizeof(struct TrainerMonItemCustomMoves); break;
    }
    p = (const u8 *)t->party.ItemCustomMoves;
    for (i = 0; p && i < partyBytes * t->partySize; i++)
        h = (h ^ p[i]) * 16777619u;
    return h;
}

static bool8 Named(int table, const char *s, u32 *out)
{
    unsigned v;
    char *end;
    if (MakerApi_ValueOf(table, s, &v))
        return *out = v, TRUE;
    *out = strtoul(s, &end, 0);
    return end != s;
}

// Applies the edits' text (the whole file); returns how many trainers it changed
int Maker_SetTrainers(const char *text)
{
    char line[512];
    const char *p = text;
    struct Trainer *cur = NULL;
    struct TrainerMonItemCustomMoves party[PARTY_SIZE];
    int count = 0, partyCount = 0;
    // (the whole table: JOHTO's, KANTO's, SINNOH's and ORRE's trainers come after TRAINERS_COUNT, and the game reads
    // them through this copy too)
    if (!sTable)
        sTable = malloc(sizeof(struct Trainer) * gTrainersBaseCount);
    memcpy(sTable, gTrainersBase, sizeof(struct Trainer) * gTrainersBaseCount);
    sEdited = 0;
#define FINISH()                                                                                                       \
    if (cur && partyCount)                                                                                             \
    {                                                                                                                  \
        struct TrainerMonItemCustomMoves *copy = malloc(sizeof(party[0]) * partyCount);                               \
        memcpy(copy, party, sizeof(party[0]) * partyCount);                                                            \
        cur->party.ItemCustomMoves = copy;                                                                             \
        cur->partySize = partyCount;                                                                                   \
        cur->partyFlags = F_TRAINER_PARTY_CUSTOM_MOVESET | F_TRAINER_PARTY_HELD_ITEM;                                  \
    }
    while (*p)
    {
        const char *nl = strchr(p, '\n');
        int len = nl ? (int)(nl - p) : (int)strlen(p);
        char a[96] = "", b[96] = "";
        u32 v;
        if (len >= (int)sizeof(line))
            len = sizeof(line) - 1;
        memcpy(line, p, len);
        line[len] = 0;
        p = nl ? nl + 1 : p + strlen(p);
        if (sscanf(line, "trainer %95s was=%95s", a, b) >= 1 && strncmp(line, "trainer ", 8) == 0)
        {
            FINISH();
            cur = NULL;
            partyCount = 0;
            if (!Named(MAKER_NAMES_TRAINER, a, &v) || v == 0 || v >= TRAINERS_COUNT)
            {
                printf("[maker] trainers: no %s (left out)\n", a);
                continue;
            }
            if (b[0] && strtoul(b, NULL, 0) != Maker_TrainerHash(v))
            {
                printf("[maker] trainers: %s changed in the game since the edit (left out)\n", a);
                continue;
            }
            cur = &sTable[v];
            count++;
            continue;
        }
        if (!cur)
            continue;
        if (sscanf(line, " name %95[^\n]", a) == 1)
        {
            u8 name[TRAINER_NAME_LENGTH + 2];
            MP_AsciiToGame(name, a, TRAINER_NAME_LENGTH);
            memset(cur->trainerName, EOS, sizeof(cur->trainerName));
            for (v = 0; v < TRAINER_NAME_LENGTH && name[v] != EOS; v++)
                cur->trainerName[v] = name[v];
            cur->trainerName[v] = EOS;
        }
        else if (sscanf(line, " class %95s", a) == 1 && Named(-1, a, &v))
            cur->trainerClass = v;
        else if (sscanf(line, " pic %95s", a) == 1 && Named(-1, a, &v))
            cur->trainerPic = v;
        else if (sscanf(line, " music %95s", a) == 1 && Named(-1, a, &v))
            cur->encounterMusic_gender = (cur->encounterMusic_gender & 0x80) | (v & 0x7F);
        else if (sscanf(line, " ai %95s", a) == 1 && Named(-1, a, &v))
            cur->aiFlags = v;
        else if (sscanf(line, " double %95s", a) == 1)
            cur->doubleBattle = atoi(a) != 0;
        else if (strncmp(line, "  items ", 8) == 0 || strncmp(line, " items ", 7) == 0)
        {
            char *q = strstr(line, "items ") + 6, *tok, *save = NULL;
            int k = 0;
            for (tok = strtok_r(q, " ", &save); tok && k < MAX_TRAINER_ITEMS; tok = strtok_r(NULL, " ", &save), k++)
                cur->items[k] = Named(MAKER_NAMES_ITEM, tok, &v) ? v : 0;
        }
        else if (sscanf(line, " mon %95s %95s", a, b) == 2 && partyCount < PARTY_SIZE)
        {
            struct TrainerMonItemCustomMoves *m = &party[partyCount];
            char *q;
            memset(m, 0, sizeof(*m));
            if (!Named(MAKER_NAMES_SPECIES, a, &v))
                continue;
            m->species = v;
            m->lvl = atoi(b);
            if ((q = strstr(line, "iv=")))
                m->iv = atoi(q + 3);
            if ((q = strstr(line, "item=")))
            {
                char it[64];
                sscanf(q + 5, "%63s", it);
                m->heldItem = Named(MAKER_NAMES_ITEM, it, &v) ? v : 0;
            }
            if ((q = strstr(line, "moves=")))
            {
                char mv[300], *tok, *save = NULL;
                int k = 0;
                sscanf(q + 6, "%299s", mv);
                for (tok = strtok_r(mv, ",", &save); tok && k < MAX_MON_MOVES; tok = strtok_r(NULL, ",", &save), k++)
                    m->moves[k] = Named(MAKER_NAMES_MOVE, tok, &v) ? v : 0;
            }
            partyCount++;
        }
    }
    FINISH();
#undef FINISH
    sEdited = count;
    gTrainersTable = count ? sTable : gTrainersBase;
    if (count)
        printf("[maker] %d edited trainer%s\n", count, count == 1 ? "" : "s");
    return count;
}

// When the game starts and when the edits are read again: MoemonMaker/trainers.txt
void Maker_LoadTrainers(void)
{
    char path[1200];
    FILE *f;
    long size;
    char *text;
    snprintf(path, sizeof(path), "%s/trainers.txt", Maker_ReadDir());
    if (!(f = fopen(path, "rb")))
    {
        gTrainersTable = gTrainersBase;
        return;
    }
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);
    text = malloc(size + 1);
    if (text && fread(text, 1, size, f) == (size_t)size)
    {
        text[size] = 0;
        Maker_SetTrainers(text);
    }
    free(text);
    fclose(f);
}

// The editor: the trainers' edits as it has them now (sent as they change, as the maps' are)
int MakerApi_SetTrainers(const char *text)
{
    return Maker_SetTrainers(text ? text : "");
}

// A trainer as the game has it (its own, never edited), for the editor
int MakerApi_TrainerInfo(int id, struct MakerTrainerInfo *info)
{
    const struct Trainer *t;
    int i, k;
    if (id <= 0 || id >= TRAINERS_COUNT)
        return 0;
    t = &gTrainersBase[id];
    memset(info, 0, sizeof(*info));
    for (i = 0; i < TRAINER_NAME_LENGTH && t->trainerName[i] != EOS; i++)
    {
        u8 c = t->trainerName[i];
        info->name[i] = c >= CHAR_A && c <= CHAR_Z ? 'A' + (c - CHAR_A) : c >= CHAR_a && c <= CHAR_z ? 'a' + (c - CHAR_a)
                      : c >= CHAR_0 && c <= CHAR_9 ? '0' + (c - CHAR_0) : c == CHAR_PERIOD ? '.' : c == CHAR_HYPHEN ? '-' : ' ';
    }
    info->trainerClass = t->trainerClass;
    info->pic = t->trainerPic;
    info->music = t->encounterMusic_gender & 0x7F;
    info->ai = t->aiFlags;
    info->doubleBattle = t->doubleBattle;
    for (i = 0; i < MAX_TRAINER_ITEMS; i++)
        info->items[i] = t->items[i];
    info->partySize = t->partySize > 6 ? 6 : t->partySize;
    for (i = 0; i < info->partySize; i++)
    {
        switch (t->partyFlags & 3)
        {
        case 0:
            info->party[i].species = t->party.NoItemDefaultMoves[i].species;
            info->party[i].level = t->party.NoItemDefaultMoves[i].lvl;
            info->party[i].iv = t->party.NoItemDefaultMoves[i].iv;
            break;
        case F_TRAINER_PARTY_CUSTOM_MOVESET:
            info->party[i].species = t->party.NoItemCustomMoves[i].species;
            info->party[i].level = t->party.NoItemCustomMoves[i].lvl;
            info->party[i].iv = t->party.NoItemCustomMoves[i].iv;
            for (k = 0; k < MAX_MON_MOVES; k++)
                info->party[i].moves[k] = t->party.NoItemCustomMoves[i].moves[k];
            break;
        case F_TRAINER_PARTY_HELD_ITEM:
            info->party[i].species = t->party.ItemDefaultMoves[i].species;
            info->party[i].level = t->party.ItemDefaultMoves[i].lvl;
            info->party[i].iv = t->party.ItemDefaultMoves[i].iv;
            info->party[i].item = t->party.ItemDefaultMoves[i].heldItem;
            break;
        default:
            info->party[i].species = t->party.ItemCustomMoves[i].species;
            info->party[i].level = t->party.ItemCustomMoves[i].lvl;
            info->party[i].iv = t->party.ItemCustomMoves[i].iv;
            info->party[i].item = t->party.ItemCustomMoves[i].heldItem;
            for (k = 0; k < MAX_MON_MOVES; k++)
                info->party[i].moves[k] = t->party.ItemCustomMoves[i].moves[k];
            break;
        }
    }
    info->hash = Maker_TrainerHash(id);
    return 1;
}
