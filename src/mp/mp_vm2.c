// Moemon Kai: the VMU pet's MOéMON and EGGS through a VM2 plugged into the PC (docs/VMU_PET_DECISIONS.md "Kai's side";
// the user's mock-ups docs/vmu_pet/kai_vm2_mockups.png).
//
// The PC menu has VM2 only while one answers (MP_Vm2_Present: a CH340 port that says ACK; src/platform/vm2.c). Choosing
// it reads the VM2's internal memory on a thread (the pet program's SAVE page: vmu_pet/pet/pet.s, at its file's $BB80:
// "MOEMONVMUPET", then the pet's RAM $40-$7b), then the box screen opens over a box of its own ("VM2", one place:
// VM2_SLOT, the VMU's screen) holding what the pet has: its EGG, or its grown MOéMON as a POKéMON of this game.
// The menus are the PC's own; when they close the box is compared with what the VM2 held:
//   taken out          the VM2 holds nothing now (the pet starts a new Egg); the MOéMON's transfer ID is remembered
//                      ("<save>.vmu": each comes in once, as the pet's SWAP can leave one on two units)
//   an EGG put there   it becomes the VM2's (an EGG of the KANTO EDITION: national 1-151)
//   anything else      anywhere else in those boxes, or not an EGG it can raise: back to the player's own PC
// The SAVE page is written back with the VM2 mark (vmu_pet/pet/link.inc vm2_check: then its LINK menu is there), and
// the VM2 restarts out of GUI-mode (the software disconnect: "It's safe to unplug it").
#include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>
#include "global.h"
#include "mp.h"
#include "aw_file.h"
#include "daycare.h"
#include "event_data.h"
#include "item.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "string_util.h"
#include "vm2.h"
#include "random.h"
#include "constants/items.h"

#define MP_VM2_WALLPAPER 15       // PLAIN under the VM2's own (pokemon_storage_system.c draws the VMU on box 0)

extern char gPlatformSavePath[];
void MP_AsciiToGame(u8 *dst, const char *src, int maxLen);

#define VM2_SLOT      8          // the box's place on the VMU's screen (row 1, column 2)
#define SAVE_PAGE     0xBB80     // in the pet's file
#define REC           16         // the record's place in the SAVE page (RAM $40)
#define REC_BYTES     39         // $40-$66 (link.inc's PET_BYTES)
#define TRAY          0x4c       // the SAVE page's VM2 tray, then its mark
#define MARK          0x78
#define EDITION_KANTO 1

// the record (pet.s's RAM names, from $40)
enum { R_MAGIC, R_STAGE, R_SPECIES, R_LEVEL, R_EXP, R_HUNGER, R_STRENGTH, R_POOP, R_FLAGS, R_MISTAKES, R_DAYS, R_FED,
       R_TRAINED, R_FEDALL, R_TRAINALL, R_EGGLEFT, R_PID, R_IVS = 20, R_IVUPS = 26, R_TID = 27, R_EDITION = 31,
       R_MASTER, R_ASTRAL, R_WINDAY, R_WINALL, R_BATTLES };

enum { JOB_NONE, JOB_READ, JOB_WRITE };

static SDL_Thread *sThread;
static SDL_atomic_t sBusy;
static int sJob, sJobOk;
static u8 sImage[VM2_IMAGE_SIZE];
static int sPageAt = -1;              // the SAVE page's place in the image (-1: no pet program)
static u8 sRecord[REC_BYTES];         // what the VM2 held
static u8 sResult;                    // VM2_RESULT_*
static u8 sOpen;                      // the box screen is the VM2's
static u8 sReturned;                  // POKéMON that went back to the player's PC
static u32 sProbeFailAt;              // when a CH340 last didn't answer (asked again after 20 s)
static EWRAM_DATA struct PokemonStorage sBoxes = {0};

enum { VM2_RESULT_NO_ANSWER, VM2_RESULT_NO_PET, VM2_RESULT_EMPTY, VM2_RESULT_EGG, VM2_RESULT_MON };

// ---- the thread

static int Worker(void *unused)
{
    (void)unused;
    if (sJob == JOB_READ)
        sJobOk = Vm2_ReadInternal(sImage);
    else if (sJob == JOB_WRITE)
    {
        int block = sPageAt / VM2_BLOCK_SIZE;
        sJobOk = Vm2_WriteBlock(block, sImage + block * VM2_BLOCK_SIZE);
        Vm2_Restart();
    }
    SDL_AtomicSet(&sBusy, 0);
    return 0;
}

static void Start(int job)
{
    if (sThread)
    {
        SDL_WaitThread(sThread, NULL);
        sThread = NULL;
    }
    sJob = job;
    sJobOk = 0;
    SDL_AtomicSet(&sBusy, 1);
    sThread = SDL_CreateThread(Worker, "moemon_vm2", NULL);
    if (!sThread)
        Worker(NULL);
}

// MP_Vm2_Present: a VM2 is plugged in and answers (the PC menu's VM2 row). Quick without one; a CH340 that doesn't
// answer is asked again only after 20 seconds.
bool8 MP_Vm2_Present(void)
{
    char port[256];
    if (SDL_AtomicGet(&sBusy))
        return TRUE;
    if (!Vm2_FindPort(port, sizeof(port)))
    {
        Vm2_Close();
        return FALSE;
    }
    if (Vm2_IsOpen())
        return TRUE;
    if (sProbeFailAt && SDL_GetTicks() - sProbeFailAt < 20000)
        return FALSE;
    if (Vm2_Open(port))
    {
        MPNet_Log("vm2: %s answered (%s)", port, Vm2_Info());
        return TRUE;
    }
    sProbeFailAt = SDL_GetTicks();
    return FALSE;
}

u16 MP_Vm2_Busy(void)
{
    return SDL_AtomicGet(&sBusy) != 0;
}

// ---- the pet in the image

// the pet's SAVE page: the GAME file's $BB80 holding "MOEMONVMUPET" (-1: none)
static int FindPage(const u8 *img)
{
    int blk, e;
    for (blk = 253; blk >= 241; blk--)
        for (e = 0; e < 16; e++)
        {
            const u8 *ent = img + blk * 512 + e * 32;
            int at;
            if (ent[0] != 0xCC)
                continue;
            at = (ent[2] | ent[3] << 8) * 512 + SAVE_PAGE;
            if (at + 128 <= VM2_IMAGE_SIZE && memcmp(img + at, "MOEMONVMUPET", 12) == 0)
                return at;
        }
    return -1;
}

static u32 PlayerOtId(void)
{
    return gSaveBlock2Ptr->playerTrainerId[0] | (gSaveBlock2Ptr->playerTrainerId[1] << 8)
         | (gSaveBlock2Ptr->playerTrainerId[2] << 16) | ((u32)gSaveBlock2Ptr->playerTrainerId[3] << 24);
}

static u32 Get32(const u8 *p)
{
    return p[0] | (p[1] << 8) | (p[2] << 16) | ((u32)p[3] << 24);
}

// a personality that's shiny for our OT ID and keeps the nature and gender (the pet's SHINY, bought with SPARKS)
static u32 ShinyKeeping(u16 species, u32 p)
{
    u32 otId = PlayerOtId(), i, k, np;
    for (i = 0; i < 0x10000; i++)
    {
        u16 lo = LOHALF(p) ^ i;
        for (k = 0; k < SHINY_ODDS; k++)
        {
            np = ((u32)(HIHALF(otId) ^ LOHALF(otId) ^ lo ^ k) << 16) | lo;
            if (GetNatureFromPersonality(np) == GetNatureFromPersonality(p)
             && GetGenderFromSpeciesAndPersonality(species, np) == GetGenderFromSpeciesAndPersonality(species, p))
                return np;
        }
    }
    return p;
}

// the record as this game's POKéMON: an EGG keeps its personality and IVs; a grown one is its level with them, the
// IV UPs bought (5 to the lowest each), shiny if bought, and its care (the user: "good care should benefit"):
// friendship 255 less 10 a care mistake (70 at least), and EVs from its trainings (4 each, spread evenly)
static bool8 RecordToMon(const u8 *r, struct Pokemon *mon)
{
    u16 species;
    u32 pid = Get32(r + R_PID);
    u8 ivs[6], i, n;
    if (r[R_MAGIC] != 'P' || r[R_EDITION] != EDITION_KANTO || r[R_SPECIES] >= 151 || r[R_STAGE] < 1 || r[R_STAGE] > 2)
        return FALSE;
    species = NationalPokedexNumToSpecies(r[R_SPECIES] + 1);
    for (i = 0; i < 6; i++)
        ivs[i] = min(r[R_IVS + i], 31);
    for (n = 0; n < r[R_IVUPS]; n++)
    {
        u8 lo = 0;
        for (i = 1; i < 6; i++)
            if (ivs[i] < ivs[lo])
                lo = i;
        ivs[lo] = min(ivs[lo] + 5, 31);
    }
    if (r[R_STAGE] == 1)
    {
        CreateEgg(mon, species, FALSE);
        SetMonData(mon, MON_DATA_PERSONALITY, &pid);
    }
    else
    {
        u8 level = min(max(r[R_LEVEL], 1), MAX_LEVEL);
        u8 friendship = 255 - min(r[R_MISTAKES] * 10, 185);
        u16 evs = min(r[R_TRAINALL] * 4, 510) / 6;
        u16 ball = ITEM_POKE_BALL;
        u8 metLevel = 0;
        if (r[R_FLAGS] & 2)
            pid = ShinyKeeping(species, pid);
        CreateMon(mon, species, level, USE_RANDOM_IVS, TRUE, pid, OT_ID_PLAYER_ID, 0);
        SetMonData(mon, MON_DATA_FRIENDSHIP, &friendship);
        SetMonData(mon, MON_DATA_POKEBALL, &ball);
        SetMonData(mon, MON_DATA_MET_LEVEL, &metLevel);   // (hatched, on the VMU)
        for (i = 0; i < 6; i++)
            SetMonData(mon, MON_DATA_HP_EV + i, &evs);
    }
    for (i = 0; i < 6; i++)
        SetMonData(mon, MON_DATA_HP_IV + i, &ivs[i]);
    CalculateMonStats(mon);
    return TRUE;
}

// an EGG of this game as the pet's record (a new transfer ID); FALSE: not one the KANTO EDITION hatches
static bool8 EggToRecord(struct BoxPokemon *box, u8 *r)
{
    u16 species = GetBoxMonData(box, MON_DATA_SPECIES);
    u16 dex = SpeciesToNationalPokedexNum(species);
    u32 pid = GetBoxMonData(box, MON_DATA_PERSONALITY), tid = Random32();
    u8 i;
    if (!GetBoxMonData(box, MON_DATA_IS_EGG) || GetBoxMonData(box, MON_DATA_SANITY_IS_BAD_EGG) || dex < 1 || dex > 151)
        return FALSE;
    memset(r, 0, REC_BYTES);
    r[R_MAGIC] = 'P';
    r[R_STAGE] = 1;
    r[R_SPECIES] = dex - 1;
    r[R_LEVEL] = 1;
    r[R_EGGLEFT] = 3;                     // it hatches half an hour after the pet next sees it
    for (i = 0; i < 4; i++)
    {
        r[R_PID + i] = pid >> (8 * i);
        r[R_TID + i] = tid >> (8 * i);
    }
    for (i = 0; i < 6; i++)
        r[R_IVS + i] = GetBoxMonData(box, MON_DATA_HP_IV + i);
    r[R_EDITION] = EDITION_KANTO;
    return TRUE;
}

// ---- the transfer IDs already taken in ("<save>.vmu": u32s)

static bool8 Taken(u32 tid)
{
    char path[512];
    u32 v;
    FILE *f;
    snprintf(path, sizeof(path), "%s.vmu", gPlatformSavePath);
    f = fopen(path, "rb");
    if (!f)
        return FALSE;
    while (fread(&v, 4, 1, f) == 1)
        if (v == tid)
        {
            fclose(f);
            return TRUE;
        }
    fclose(f);
    return FALSE;
}

static void Take(u32 tid)
{
    char path[512];
    static u32 list[4096];
    size_t n = 0;
    FILE *f;
    snprintf(path, sizeof(path), "%s.vmu", gPlatformSavePath);
    f = fopen(path, "rb");
    if (f)
    {
        n = fread(list, 4, ARRAY_COUNT(list) - 1, f);
        fclose(f);
    }
    list[n++] = tid;
    AwFile_WriteNow(path, list, n * 4);
}

// ---- the PC's specials

// MP_Vm2_StartRead (PC > VM2): read the VM2 on the thread (MP_Vm2_Busy until done)
void MP_Vm2_StartRead(void)
{
    Start(JOB_READ);
}

// MP_Vm2_Result: VM2_RESULT_* of the read: 0 no answer, 1 no pet program, 2 nothing on it, 3 an EGG, 4 a MOéMON
// (also its name in gStringVar1)
u16 MP_Vm2_Result(void)
{
    struct Pokemon mon;
    sResult = VM2_RESULT_NO_ANSWER;
    sPageAt = -1;
    if (!sJobOk)
        return sResult;
    sPageAt = FindPage(sImage);
    if (sPageAt < 0)
        return sResult = VM2_RESULT_NO_PET;
    memcpy(sRecord, sImage + sPageAt + REC, REC_BYTES);
    if (!RecordToMon(sRecord, &mon) || Taken(Get32(sRecord + R_TID)))
        return sResult = VM2_RESULT_EMPTY;
    GetMonData(&mon, MON_DATA_NICKNAME, gStringVar1);
    StringGet_Nickname(gStringVar1);
    return sResult = sRecord[R_STAGE] == 1 ? VM2_RESULT_EGG : VM2_RESULT_MON;
}

// MP_ShowVm2Box: the PC's menus over the VM2's box (as the GLOBAL PC's: pokemon_storage_system.c)
void MP_ShowVm2Box(void)
{
    struct Pokemon mon;
    u8 i;
    memset(&sBoxes, 0, sizeof(sBoxes));
    for (i = 0; i < TOTAL_BOXES_COUNT; i++)
        sBoxes.boxWallpapers[i] = MP_VM2_WALLPAPER;
    MP_AsciiToGame(sBoxes.boxNames[0], "VM2", BOX_NAME_LENGTH);
    if ((sResult == VM2_RESULT_EGG || sResult == VM2_RESULT_MON) && RecordToMon(sRecord, &mon))
        CopyMon(&sBoxes.boxes[0][VM2_SLOT], &mon.box, sizeof(mon.box));
    sOpen = TRUE;
    MP_ShowOtherPC(&sBoxes);
}

bool8 MP_Vm2_BoxOpen(void)
{
    return sOpen;
}

static void ToOwnPC(struct BoxPokemon *box)
{
    struct Pokemon mon;
    BoxMonToMon(box, &mon);
    if (MP_SendMonToPC(&mon) == MON_CANT_GIVE)
        GiveMonToPlayer(&mon);
    sReturned++;
}

// MP_Vm2_BoxClosed (pokemon_storage_system.c, the menus closing): what changed goes to the VM2, the rest home
void MP_Vm2_BoxClosed(void)
{
    struct BoxPokemon *slot = &sBoxes.boxes[0][VM2_SLOT];
    bool8 had = sResult == VM2_RESULT_EGG || sResult == VM2_RESULT_MON;
    u8 rec[REC_BYTES], *page;
    int b, p;
    sOpen = FALSE;
    sReturned = 0;
    memcpy(rec, sRecord, REC_BYTES);
    if (had && !(GetBoxMonData(slot, MON_DATA_SPECIES) != SPECIES_NONE
                 && GetBoxMonData(slot, MON_DATA_PERSONALITY) == Get32(sRecord + R_PID)
                 && GetBoxMonData(slot, MON_DATA_IS_EGG) == (sRecord[R_STAGE] == 1)))
    {
        // taken: in once only; the VM2 has nothing now (unless an EGG went there)
        Take(Get32(sRecord + R_TID));
        memset(rec, 0, REC_BYTES);
        rec[R_MAGIC] = 'P';
        MPNet_Log("vm2: its %s came in", sRecord[R_STAGE] == 1 ? "EGG" : "MOEMON");
    }
    if (GetBoxMonData(slot, MON_DATA_SPECIES) != SPECIES_NONE
     && !(had && GetBoxMonData(slot, MON_DATA_PERSONALITY) == Get32(sRecord + R_PID)))
    {
        if (EggToRecord(slot, rec))
            MPNet_Log("vm2: an EGG (national %u) goes to it", rec[R_SPECIES] + 1);
        else
            ToOwnPC(slot);
    }
    ZeroBoxMonData(slot);
    for (b = 0; b < TOTAL_BOXES_COUNT; b++)          // anything else put in these boxes goes home
        for (p = 0; p < IN_BOX_COUNT; p++)
            if (GetBoxMonData(&sBoxes.boxes[b][p], MON_DATA_SPECIES) != SPECIES_NONE)
                ToOwnPC(&sBoxes.boxes[b][p]);
    if (sPageAt < 0)
        return;
    // the SAVE page: the record, and the VM2 mark ("VM2M", the image's own root block $33-$36)
    page = sImage + sPageAt;
    memcpy(page + REC, rec, REC_BYTES);
    memcpy(page + MARK, "VM2M", 4);
    memcpy(page + MARK + 4, sImage + 255 * 512 + 0x33, 4);
    Start(JOB_WRITE);
}

// MP_Vm2_Returned: how many POKéMON went back to the player's PC (in gStringVar1)
u16 MP_Vm2_Returned(void)
{
    ConvertIntToDecimalStringN(gStringVar1, sReturned, STR_CONV_MODE_LEFT_ALIGN, 2);
    return sReturned;
}

// MP_Vm2_Written: the write and the restart went through
u16 MP_Vm2_Written(void)
{
    return sJobOk != 0;
}

// MP_Vm2_Close: the VM2 restarted without changes (the PC menu's B, or nothing to write)
void MP_Vm2_Close(void)
{
    if (!SDL_AtomicGet(&sBusy))
        Vm2_Restart();
}
