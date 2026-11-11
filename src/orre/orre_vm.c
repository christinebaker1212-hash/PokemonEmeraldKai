// Moemon Kai: ORRE's scripts are Pokémon Colosseum's own, run as Colosseum runs them (docs/ORRE_HANDOFF.md step 4,
// "Colosseum's scripts"). Its rooms' script banks (.scd, in ORRE's data pack) and the shared bank in common_rel are
// bytecode for its GS VM; this is that VM, opcode for opcode from the Colosseum decomp (src/game/gs_vm.c: the operand
// descriptors, the call and native frames, the int / float arithmetic), with each native the scripts call mapped to
// what Kai does for it. Nothing is translated ahead of time and nothing of ORRE's scripts or words is written here.
//
// A room (Colosseum's floor table record: + 0x28 its sound script, + 0x2C and + 0x30 its set-up, + 0x34 its main
// loop, + 0x38 what runs on leaving) gets its set-up run and its main loop started when the player arrives
// (Orre_Vm_RoomEntered); every overworld frame runs the threads until they wait (Orre_Vm_Frame, from O3D_Update).
// The main loop watches the player and the story flags and starts the room's scenes; a scene takes the player
// (heroMoveInitEvent: Kai's script Orre_EventScript_VmHold locks the field while it runs, and starts its battles)
// until it lets go (heroMoveTermEvent).
//
// Values are 32-bit words as in the game; an address a script takes (a variable's) is a tagged index here (VM_REF_*),
// the code's own offsets bank-relative, so nothing depends on the PC's pointer size.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "global.h"
#include "data.h"
#include "event_data.h"
#include "field_message_box.h"
#include "item.h"
#include "main.h"
#include "menu.h"
#include "naming_screen.h"
#include "task.h"
#include "o3d.h"
#include "orre_sound.h"
#include "orre.h"
#include "orre_data.h"
#include "palette.h"
#include "constants/rgb.h"
#include "battle_setup.h"
#include "script.h"
#include "string_util.h"
#include "text.h"
#include "constants/characters.h"
#include "constants/moves.h"
#include "constants/species.h"
#include "constants/songs.h"
#include "sound.h"
#include "money.h"
#include "pokemon.h"
#include "orre_shadow.h"
#include "overworld.h"
#include "fieldmap.h"
#include "map_name_popup.h"
#include "constants/maps.h"
#include "field_screen_effect.h"
#include "field_player_avatar.h"
#include "script_menu.h"
#include "constants/songs.h"
#include "script_pokemon_util.h"
#include "shop.h"
#include "constants/items.h"

void MPNet_Log(const char *fmt, ...);
extern const u8 Orre_EventScript_VmHold[];

#define VM_STACK 512
#define VM_THREADS 8
#define VM_STEPS_PER_FRAME 200000      // (a script that never waits stops here for the frame)
#define VM_REF 0x80000000u            // an address a script took: the variable's index, tagged
#define VM_REF_FRAME 0x40000000u

#define COMMON_BANK_ID 0x0596
#define COMMON_BANK_AT 0x89CFC        // common_rel .data (docs/recon/common_script_0596.md in the decomp)
#define COMMON_BANK_SIZE 8120
#define ROOM_BANK_ID 0x0100
#define FLOORS 0x8BCF4
#define FLOOR_COUNT 0x14A670
#define FLOOR_SIZE 0x4C
#define NAMES 0x1A34                 // the people's names: {u16 the flag that tells it's known, u32 its message}
#define NAME_COUNT 0x14A660

typedef union { u32 u; s32 s; f32 f; } V;

struct Bank
{
    u16 id;
    u8 *d;                 // a copy (its globals live in it)
    u32 size, globals, funcs;
};

enum { T_FREE, T_RUN, T_WAIT, T_DONE };
enum { W_NONE, W_FRAMES, W_MESSAGE, W_MESSAGE_KEY, W_YESNO, W_BATTLE, W_MOVE, W_SCREEN, W_FANFARE, W_CAMERA, W_THREAD, W_FADE, W_NEWS, W_CAM_MOVE };

// Kai's screens a scene opens (W_SCREEN): Orre_EventScript_VmHoldLoop goes to them while the scene waits
enum { SCREEN_NONE, SCREEN_SHOP, SCREEN_PC, SCREEN_WARP, SCREEN_CHOICE, SCREEN_RELIC, SCREEN_NAME };

// Colosseum's shops (menuShopOpen N): tools/orre/gen_shops.py
struct OrreShop
{
    u8 kind;
    u16 items[20];
};
#include "../data/orre_shops.h"

// The story's gates (scriptSetCol: a collision object a room's scripts switch on and off), the tiles each closes:
// tools/orre/gen_gates.py
struct OrreGate { u16 room; u8 gate; u16 first, count; bool8 closed; };   // (closed: at the room's start)
struct OrreDoor { u16 room; u8 index; s8 col; u16 open, lock; u8 type; bool8 touch; float x0, z0, x1, z1; };
#include "../data/orre_gates.h"

struct Thread
{
    u8 state, wait;
    bool8 holds;          // it took the player (heroMoveInitEvent): the field is locked while it runs
    bool8 room;           // the room's main loop (stopped when the room changes)
    struct Bank *bank;
    u32 ip, scriptId;
    s32 sp, frame;
    s32 frames;           // W_FRAMES: frames left
    u32 message;          // W_MESSAGE*: the line shown (shown again if the field hid it)
    u16 battle;           // W_BATTLE: the battle asked for
    u32 moveGroup, moveIndex;   // W_MOVE: the person whose walk it waits for
    u8 screen, shop;      // W_SCREEN: the screen asked for (SCREEN_*), Colosseum's shop number
    u32 choices[8];       // SCREEN_CHOICE: the choices' messages (std199_menuDisplayCustomMenu)
    u8 nchoices;
    u32 result;           // SCREEN_CHOICE: the row chosen
    struct Thread *child; // W_THREAD: the script it runs to its end (std209_threadRunScriptMode4)
    bool8 camHold;
    bool8 debug;          // started by Orre_Vm_Debug (tests: its end is logged)        // it holds the field for a shot it plays (a room's main loop: Colosseum's hero stands still)
    u32 handle;           // its task number (scriptExecTask's, for std006_threadWaitForCompletion)
    u32 childId;
    bool8 screenOpen;     // W_SCREEN: the hold script went to it
    u32 news[2];          // W_NEWS: std182_tvNewsEvent's lines still to show (NewsEvent)
    u8 newsLen, newsAt;
    bool8 newsHeld;       // ... it took the player for them
    bool8 heroLoop;       // it steps the player (heroMoveMain): a room's main loop
    bool8 loopHold;       // ... and went off into a scene without them: the player waits until it steps them again
    u8 sinceHero;         // frames since it last did
    bool8 stepped;        // it has stepped them at least once (its first pass is over)
    V st[VM_STACK];
};

static struct Bank sCommon, sRoomBank;
static struct Thread sThreads[VM_THREADS];
static u8 *sRel;
static const u8 *sData;
static u32 sDataSize;
static bool8 sDataTried;
static u16 sRoom;          // the Colosseum room the threads belong to (0: none)
static bool8 sHolding;     // Orre_EventScript_VmHold is waiting (special Orre_VmHold)
static struct Thread *sHolder;
static u8 sScreen, sShop;  // the screen the hold script goes to next (Orre_VmScreen), the shop it opens
static struct Thread *sScreenThread;
static u32 sHandles;   // ... the thread that asked (its choices)
static u16 sWarpMap;       // SCREEN_WARP: floorLink's destination
static s16 sWarpX, sWarpY;
static u16 sLastRoom, sPrevRoom;   // floorGetPrevFloorID: the room before this one
static u32 sEntry, sLinkEntry;     // floorGetNextPosIndex: the entry the player came in by (floorLink's, else found)
static bool8 sLinked;
static bool8 sLogged[256];

static u32 Be32(const u8 *p) { return (u32)p[0] << 24 | p[1] << 16 | p[2] << 8 | p[3]; }
static u16 Be16(const u8 *p) { return p[0] << 8 | p[1]; }

// ---- the data: common_rel's .data and the banks

static bool8 LoadData(void)
{
    u32 size, st, off;
    if (sDataTried)
        return sData != NULL;
    sDataTried = TRUE;
    if ((sRel = OrreData_Read("common/common_rel.rel", &size)) == NULL)
        return sDataTried = FALSE;   // (no data pack yet: looked for again next time)
    if (size < 0x40)
        return FALSE;
    st = Be32(sRel + 0x10);
    if (Be32(sRel + 0x0C) < 5 || st + 8 * 5 > size)
        return FALSE;
    off = Be32(sRel + st + 8 * 4) & ~3u;
    sDataSize = Be32(sRel + st + 8 * 4 + 4);
    if (off + sDataSize > size || sDataSize < FLOOR_COUNT + 4)
        return FALSE;
    sData = sRel + off;
    return TRUE;
}

static bool8 BankFrom(struct Bank *b, const u8 *src, u32 size)
{
    free(b->d);
    memset(b, 0, sizeof(*b));
    if (src == NULL || size < 0x18 || (b->d = malloc(size)) == NULL)
        return FALSE;
    memcpy(b->d, src, size);
    b->size = size;
    b->id = Be16(b->d);
    b->funcs = Be16(b->d + 4);
    b->globals = Be32(b->d + 0x10);
    if (0x18 + 4 * b->funcs > size || b->globals > size)
    {
        free(b->d);
        b->d = NULL;
        return FALSE;
    }
    return TRUE;
}

static struct Bank *BankOf(u16 id)
{
    if (id == COMMON_BANK_ID)
    {
        if (sCommon.d == NULL && LoadData() && COMMON_BANK_AT + COMMON_BANK_SIZE <= sDataSize)
            BankFrom(&sCommon, sData + COMMON_BANK_AT, COMMON_BANK_SIZE);
        return sCommon.d ? &sCommon : NULL;
    }
    if (id == ROOM_BANK_ID)
        return sRoomBank.d ? &sRoomBank : NULL;
    return NULL;
}

static void LoadRoomBank(const char *file)
{
    char name[80];
    u8 *d;
    u32 size;
    snprintf(name, sizeof(name), "%s/%s.scd", file, file);
    d = OrreData_Read(name, &size);
    BankFrom(&sRoomBank, d, d ? size : 0);
    free(d);
}

// The floor table's record of Colosseum room ROOM
static const u8 *FloorOf(u16 room)
{
    u32 i, n;
    if (!LoadData())
        return NULL;
    n = Be32(sData + FLOOR_COUNT);
    for (i = 0; i < n && FLOORS + (i + 1) * FLOOR_SIZE <= sDataSize; i++)
        if (Be32(sData + FLOORS + i * FLOOR_SIZE + 0xC) == room)
            return sData + FLOORS + i * FLOOR_SIZE;
    return NULL;
}

// ---- the stack and operands (gs_vm.h GSvmPush / GSvmPop / GSvmGetOperand)

static void Push(struct Thread *t, u32 v)
{
    if (t->sp < VM_STACK)
        t->st[t->sp++].u = v;
}

static V Pop(struct Thread *t)
{
    V v = {0};
    if (t->sp > 0)
        v = t->st[--t->sp];
    return v;
}

static V *Var(struct Thread *t, u16 desc, s32 idx)
{
    static V sNone;
    if (desc & 0x40)
    {
        s32 at = t->frame + idx;
        return at >= 0 && at < VM_STACK ? &t->st[at] : &sNone;
    }
    if (t->bank && t->bank->globals + 4 * (u32)(idx + 1) <= t->bank->size && idx >= 0)
        return (V *)(t->bank->d + t->bank->globals + 4 * idx);   // (big endian in the bank: see GVal)
    return &sNone;
}

// A global lives in the bank, big endian
static bool8 IsGlobal(struct Thread *t, V *p)
{
    return t->bank && (u8 *)p >= t->bank->d && (u8 *)p < t->bank->d + t->bank->size;
}

static V Load(struct Thread *t, V *p)
{
    V v;
    v.u = IsGlobal(t, p) ? Be32((const u8 *)p) : p->u;
    return v;
}

static void Store(struct Thread *t, V *p, V v)
{
    if (IsGlobal(t, p))
    {
        u8 *b = (u8 *)p;
        b[0] = v.u >> 24, b[1] = v.u >> 16, b[2] = v.u >> 8, b[3] = v.u;
    }
    else
        *p = v;
}

static V Operand(struct Thread *t, u16 desc)
{
    V v = {0}, idx;
    if (desc == 0)
        return v;
    if (desc & 0x80)
        return Pop(t);
    idx = Pop(t);
    if (desc & (0x20 | 0x100))
        v.u = VM_REF | ((desc & 0x40) ? VM_REF_FRAME | (u32)(t->frame + idx.s) : (u32)idx.s);
    else
        v = Load(t, Var(t, desc, idx.s));
    return v;
}

// What a tagged address names (natives that write through one)
static V *Deref(struct Thread *t, u32 ref)
{
    static V sNone;
    if (!(ref & VM_REF))
        return &sNone;
    if (ref & VM_REF_FRAME)
        return (ref & 0xFFFF) < VM_STACK ? &t->st[ref & 0xFFFF] : &sNone;
    return Var(t, 0, (s32)(ref & 0xFFFFFF));
}

static u8 Code8(struct Thread *t) { return t->ip < t->bank->size ? t->bank->d[t->ip++] : 1; }
static u16 Code16(struct Thread *t) { u16 v = t->ip + 2 <= t->bank->size ? Be16(t->bank->d + t->ip) : 0; t->ip += 2; return v; }
static u32 Code32(struct Thread *t) { u32 v = t->ip + 4 <= t->bank->size ? Be32(t->bank->d + t->ip) : 0; t->ip += 4; return v; }

static bool8 IsInt(u8 desc) { return (desc & 0x3F) == 2; }
static f32 AsF(V v, u8 desc) { return IsInt(desc) ? (f32)v.s : v.f; }

// ---- threads

// The function steps the player itself (it calls heroMoveMain, native 31, with no arguments: push 31, native of one
// word): a room's main loop
static bool8 StepsHero(const struct Bank *b, u32 func)
{
    static const u8 sCall[] = {0x08, 0x02, 0x00, 0x00, 0x00, 0x1F, 0x25, 0x00, 0x01, 0x00, 0x01};
    u32 start = Be32(b->d + 0x18 + 4 * func), end = b->size, i;
    for (i = 0; i < b->funcs; i++)
    {
        u32 o = Be32(b->d + 0x18 + 4 * i);
        if (o > start && o < end)
            end = o;
    }
    for (i = start; i + sizeof(sCall) <= end; i++)
        if (memcmp(b->d + i, sCall, sizeof(sCall)) == 0)
            return TRUE;
    return FALSE;
}

static struct Thread *Start(u32 scriptId, u32 argc, const u32 *args, bool8 room)
{
    struct Bank *b = BankOf(scriptId >> 16);
    u32 func = scriptId & 0xFFFF, i;
    struct Thread *t = NULL;
    if (b == NULL || func >= b->funcs)
    {
        MPNet_Log("orre vm: no script %08X", scriptId);
        return NULL;
    }
    for (i = 0; i < VM_THREADS; i++)
        if (sThreads[i].state == T_FREE || sThreads[i].state == T_DONE)
        {
            t = &sThreads[i];
            break;
        }
    if (t == NULL)
        return NULL;
    memset(t, 0, sizeof(*t));
    t->bank = b;
    t->scriptId = scriptId;
    t->room = room;
    t->heroLoop = room && StepsHero(b, func);   // (from its first pass: a scene it starts with has the player wait too)
    t->ip = Be32(b->d + 0x18 + 4 * func);
    t->state = T_RUN;
    // (cms_ScriptCreateThread: the result's slot, the arguments, a null return address, the frame, the count)
    Push(t, 0);
    for (i = 0; i < argc; i++)
        Push(t, args[i]);
    Push(t, 0);
    Push(t, 0);
    Push(t, argc);
    return t;
}

static void FadeBack(void);
static bool8 sScriptBlack;

// The scene that held the player lets go or ends while another still holds them (a room's loop that went back to
// stepping the player while the scene it started plays on): the hold passes to that one. The player walked about in
// the middle of such scenes (the user, 2026-11-11). TRUE: someone else holds on
static bool8 PassHold(struct Thread *from)
{
    u32 i;
    for (i = 0; i < VM_THREADS; i++)
        if (&sThreads[i] != from && sThreads[i].state == T_RUN && sThreads[i].holds)
        {
            sHolder = &sThreads[i];
            MPNet_Log("orre vm: %08X lets go, %08X still holds the player", from->scriptId, sHolder->scriptId);
            return TRUE;
        }
    return FALSE;
}

static void Finish(struct Thread *t)
{
    t->state = T_DONE;
    if (t->holds)
        FadeBack();   // (a scene that faded out and ended without fading in)
    if (t->debug)
        MPNet_Log("orre vm: debug scene %08X ends", t->scriptId);
    if (t->holds && sHolder == t && !PassHold(t))
    {
        // (the scene ended without letting go: the field is given back)
        sHolder = NULL;
        if (sHolding)
        {
            sHolding = FALSE;
            gSpecialVar_Result = 0;
            ScriptContext_Enable();
        }
    }
}

// ---- the natives (lbl_802E1CF0's indices: tools/orre/natives.json)

enum
{
    N_CHAR_SET_ANIM = 51, N_PEOPLE_WAIT_SYNC = 53, N_TALK = 54, N_END_SPEECH = 55, N_IN_BOUNDS = 58,
    N_WIN_OPEN = 7, N_WIN_CLOSE = 8, N_YESNO = 11, N_FADE_SET = 28, N_FADE_CHECK = 29, N_HERO_MAIN = 31,
    N_HERO_INIT_EVENT = 32, N_HERO_TERM_EVENT = 33, N_PEOPLE_MOVE_CHECK = 62, N_FLAG_SET1 = 78, N_FLAG_SET0 = 79,
    N_FLAG_SET = 80, N_FLAG_CHECK = 81, N_FLAG_GET = 82, N_ROOM_ID = 92, N_GROUP_ID = 93, N_FIGHT_START = 102,
    N_BATTLE_RESULT = 103, N_SET_VALUE = 132, N_FIELD_START = 152, N_SLEEP = 157, N_CAMERA_END_CHECK = 160,
    N_PEOPLE_VISIBLE = 50, N_PEOPLE_OPEN = 56, N_PEOPLE_CLOSE = 57, N_SET_POSITION = 59, N_SET_ROTATION = 60,
    N_MOVE_TO = 61, N_LOOK_AT_PLAYER = 65, N_TURN_TO_ANGLE = 68, N_FLOOR_CHAR_DISP = 100, N_FACE_CHARACTER = 165,
    N_MOVE_TO_CHARACTER = 177, N_TURN_HEAD_TO = 181, N_APPROACH_PLAYER = 232,
    N_SHOP_OPEN = 25, N_POKEMON_CENTER = 83, N_WARP_MENU_ROOM = 98, N_HAVE_ITEM = 120, N_ADD_MONEY = 133,
    N_GET_MONEY = 134, N_ADD_ITEM = 219, N_CAMERA_PLAY = 42, N_CAMERA_STOP = 43, N_CAMERA_STOP2 = 44,
    N_CAMERA_WAIT = 46, N_CAMERA_RETURN = 179, N_FLOOR_LINK = 95, N_FLOOR_LINK_SE = 125, N_PREV_FLOOR = 94,
    N_NEXT_FLOOR = 180, N_NEXT_POS = 195, N_CHOICE = 199, N_SUB_YESNO = 245, N_CUSTOM43 = 158, N_SURPRISE = 131,
    N_WALK_RESERVE = 72, N_WALK_ADD = 73, N_WALK_START = 74, N_GET_POSITION = 172, N_GAZE_HERO = 169,
    N_THREAD_YIELD = 71, N_GET_ITEM = 119, N_MODEL_RES = 112, N_SAVE_COLOSSEUM = 223, N_EXEC_TASK = 3,
    N_RUN_SCRIPT = 209, N_THREAD_WAIT = 6, N_SCENE_MODE = 36, N_CAMERA_TARGET = 37, N_PARTY_STATUS = 216,
    N_GET_PREMIUM = 187, N_SET_PREMIUM = 188, N_CHECK_TEMOCHI = 226, N_WIN_OPEN_SE = 224, N_SHADOW_COUNT = 124,
    N_PARTY_COUNT = 123, N_NAME_ENTRY = 22, N_GIFT_POKEMON = 217, N_RELIVE = 194, N_SHADOW_OPEN_COUNT = 242,
    N_MT_BATTLE_LEAVE = 229, N_PARTNER_FOLLOW = 35, N_TV_NEWS = 182, N_CAM_SET_EYE = 40, N_CAM_SET_ROT = 41,
    N_CAM_MOVE_EYE = 140, N_CAM_MOVE_ROT = 141, N_CAM_SET_FOV = 150, N_WALK_SPEED = 69, N_TURN_TO_POINT = 164, N_SET_COL = 118,
};

// Natives that change nothing Kai shows yet, with what Colosseum's scripts expect back: sounds and music (until the
// music is converted), the scene's own camera moves and modes (the field camera stays; shots play: o3d_cam.c),
// models' and map objects' animations, lights, people's internal flags and walking speed, story gates (left open),
// doors and elevators (the maps' own links), the P★DA's mail, the TV's channel
static const struct { u16 n; u8 ret; } sQuiet[] = {
    {130, 0}, {218, 0},
    {38, 0}, {39, 0}, {48, 0}, {127, 0}, {147, 0},
    {148, 0}, {149, 0}, {156, 0}, {170, 1}, {126, 0},
    {113, 0}, {114, 0}, {115, 0}, {116, 0}, {117, 1}, {154, 0}, {178, 1}, {190, 0}, {191, 0},
    {193, 1}, {221, 0}, {244, 0}, {70, 0}, {168, 0}, {231, 0},
    {63, 0}, {64, 0}, {66, 0}, {67, 0}, {76, 0}, {77, 0}, {151, 0}, {155, 1},
    {97, 0}, {197, 0}, {161, 0}, {162, 0}, {136, 0}, {144, 0},
    {173, 0}, {17, 0},
    {2, 0}, {243, 0}, {111, 0}, {49, 0}, {189, 0}, {214, 0}, {186, 0}, {234, 0},
    {196, 0}, {166, 0},
    {225, 0}, {246, 0},   // (the party noted for a challenge's check; the save's continue point: Kai's save)
    {223, 0}, {236, 0}, {239, 0},   // (MT. BATTLE's rest stops: save and quit (Kai saves any time); POKe COUPONS: not yet)
};

// Colosseum's own items past Gen 3's (its key items: the story's keys, DNA samples...) as Kai-side ORRE flags: what a
// script asks (scriptHaveItem) and gives (scriptAddItem / scriptGetItem); Kai's bag has Gen 3's (to 348)
#define ORRE_KEY_ITEM_FIRST 349
#define ORRE_KEY_ITEM_FLAG(item) (ORRE_KAI_FLAG_FIRST + 0x10 + ((item) - ORRE_KEY_ITEM_FIRST))
#define IS_ORRE_KEY_ITEM(item) ((item) >= ORRE_KEY_ITEM_FIRST && ORRE_KEY_ITEM_FLAG(item) < ORRE_FLAG_SNAG_MACHINE)

#define ROOM_MT_BATTLE_PC 75   // (D2_pc_1F: MT. BATTLE's reception)
#define ROOM_PCBOX_MENU 913    // (Colosseum's menu rooms: the PC's boxes)
#define LAST_COLO_ITEM 348     // (Colosseum's items to here are Gen 3's numbers, Kai's own)

// std217's gifts as Colosseum makes them (heroPokemonGetPrasle / GetHouou): species, level, its OT's name (a message)
// and number; to the party, else the PC
static void Orre_GiveGift(u32 kind)
{
    static const struct { u16 species; u8 level; u16 otMsg; u32 otId; } sGifts[] = {
        [3] = {SPECIES_PLUSLE, 13, 0x12AC, 0x911D},
        [4] = {SPECIES_HO_OH, 70, 0x12AD, 0x2740},
    };
    struct Pokemon mon;
    u8 name[PLAYER_NAME_LENGTH + 1];
    if (kind >= ARRAY_COUNT(sGifts) || sGifts[kind].species == 0)
    {
        MPNet_Log("orre vm: gift %u (Colosseum's hero's own: not given)", kind);
        return;
    }
    CreateMon(&mon, sGifts[kind].species, sGifts[kind].level, USE_RANDOM_IVS, FALSE, 0, OT_ID_PRESET, sGifts[kind].otId);
    StringExpandPlaceholders(gStringVar4, Orre_Text(sGifts[kind].otMsg));
    memcpy(name, gStringVar4, PLAYER_NAME_LENGTH);
    name[PLAYER_NAME_LENGTH] = EOS;
    SetMonData(&mon, MON_DATA_OT_NAME, name);
    GiveMonToPlayer(&mon);
    MPNet_Log("orre vm: gift %u: species %u", kind, sGifts[kind].species);
}

static u16 KaiItem(u32 coloItem)
{
    return coloItem <= LAST_COLO_ITEM ? coloItem : ITEM_NONE;
}

static void WordsOf(u8 *out, u8 ctl, u32 value)
{
    switch (ctl)
    {
    case 0x59: case 0x31: case 0x37: case 0x51: case 0x55: case 0x56:   // (npc, msg_id, menu_msg...: a message)
        StringCopy(out, Orre_Text(value));
        break;
    case 0x2D: case 0x2E: case 0x29:                                       // (item, item2, item_name)
        CopyItemName(value <= 348 ? value : ITEM_NONE, out);
        break;
    case 0x32: case 0x33: case 0x4E: case 0x58: case 0x10: case 0x0F:    // (pokemon, pokemon2, pokemon_id, tribe...)
        StringCopy(out, gSpeciesNames[value < NUM_SPECIES ? value : 0]);
        break;
    case 0x39: case 0x28:                                                  // (move, move_name)
        StringCopy(out, gMoveNames[value < MOVES_COUNT ? value : 0]);
        break;
    default:                                                                // (digits, money, values)
        ConvertIntToDecimalStringN(out, value, STR_CONV_MODE_LEFT_ALIGN, 10);
        break;
    }
}

u32 MP_TextPrinterDebug(u8 id);
static void ShowMessage(struct Thread *t, u32 msg, bool8 key)
{
    static u8 sPlaceholder[6];
    // (the place's name banner shares BG0's scroll with the message box: it goes first, or its slide carries the box)
    HideMapNamePopUpWindow();
    if (!IsFieldMessageBoxHidden())
        HideFieldMessageBox();
    Orre_TextPlaceholder(sPlaceholder, msg);
    ShowFieldMessage(sPlaceholder);
    t->message = msg;
    t->wait = key ? W_MESSAGE_KEY : W_MESSAGE;
    MPNet_Log("orre vm: message %u", msg);
}

// The person a talk names: their name's message in {npc}, as Colosseum's std054 puts it: the name table's row (common_rel
// .data 0x1A34, 8 bytes: u16 the story flag that tells it's known, u32 its message), "???" (message 0xFA3) until then
static void SpeakerName(u32 group, u32 index)
{
    u16 row = Orre_Npc_Name(index);
    u32 msg = 0xFA3;
    (void)group;
    if (row != 0xFFFF && LoadData() && row < Be32(sData + NAME_COUNT) && NAMES + 8 * (row + 1) <= sDataSize)
    {
        u16 hear = Be16(sData + NAMES + 8 * row);
        if (hear == 0 || Orre_Flag(hear))
            msg = Be32(sData + NAMES + 8 * row + 4);
    }
    Orre_SetStrVar(0x59, Orre_Text(msg));
}

static bool8 HeroInBox(f32 x0, f32 z0, f32 x1, f32 z1)
{
    float cx, cz;
    if (!O3D_TileToColo(gSaveBlock1Ptr->pos.x, gSaveBlock1Ptr->pos.y, &cx, &cz))
        return FALSE;
    return cx >= min(x0, x1) && cx <= max(x0, x1) && cz >= min(z0, z1) && cz <= max(z0, z1);
}

// The hold script starts as soon as Kai's script engine is free (Orre_Vm_Frame: a map's own arrival script may still
// be running when a scene takes the player)
static bool8 sHoldPending;

static void TryHold(void)
{
    if (sHolder == NULL)
        sHoldPending = FALSE;   // (the scene that asked ended before Kai's script engine was free: nothing left to hold.
                                // Left set, the room's touch spots and main loop boxes stayed off until the player left:
                                // PHENAC after RUI's rescue, the MAYOR's house never reached)
    if (sHoldPending && sHolder && !sHolding && !ScriptContext_IsEnabled())
    {
        sHoldPending = FALSE;
        ScriptContext_SetupScript(Orre_EventScript_VmHold);
        MPNet_Log("orre vm: %08X takes the player", sHolder->scriptId);
    }
}

// Kai's whiteout comes back to the save's last heal place (overworld.c SetWarpDestinationToLastHealLocation)
static void HealAt(s8 group, s8 num, s8 warpId, s16 x, s16 y)
{
    struct WarpData *w = &gSaveBlock1Ptr->lastHealLocation;
    w->mapGroup = group, w->mapNum = num, w->warpId = warpId, w->x = x, w->y = y;
}

// The story as a scene found it (Colosseum's "a loss ends the game": back to the save, the scene to play again): the
// flags kept when a scene takes the player, put back when one of its battles is lost that way (Orre_Vm_LossWhiteOut)
static u8 *sSceneStory;
static u32 sSceneStorySize;
static bool8 sLossPending;

static void KeepStory(void)
{
    u32 size;
    u8 *st = Orre_StoryState(&size);
    if (sSceneStory == NULL || sSceneStorySize != size)
    {
        free(sSceneStory);
        sSceneStory = malloc(size);
        sSceneStorySize = sSceneStory ? size : 0;
    }
    if (sSceneStory)
        memcpy(sSceneStory, st, size);
}

static void Hold(struct Thread *t)
{
    t->holds = TRUE;
    if (sHolder == NULL)
    {
        KeepStory();
        sHolder = t;
        sHoldPending = TRUE;
        TryHold();
    }
}

static void LetGo(struct Thread *t)
{
    t->holds = FALSE;
    if (sHolder == t && !PassHold(t))
    {
        sHolder = NULL;
        // (the player walks again: a scene camera left where the scene put it goes back to the field's, as the user
        // asked, 2026-11-08: PHENAC's sack scene leaves it on the sack)
        if (O3D_FreeActive() && !O3D_FreeMoving())
            O3D_CamEaseBack(0.6f);
        if (sHolding)
        {
            sHolding = FALSE;
            gSpecialVar_Result = 0;
            ScriptContext_Enable();
        }
    }
}

// Call native N with ARGS (A[0] the first): its result; the thread may be left waiting
// fadeSet(MODE, SECONDS): Colosseum's whole-screen fade (its scripts use 3, to black, and 2, back from it), as Kai's
// palette fade (the 3D picture follows it: o3d.c FindFade) in about the same time: 8 steps of DELAY + 1 frames, a
// whole-screen fade 30% quicker (palette.c sQuickFade). A scene that ends black is given back (FadeBack)

static void Fade(u32 mode, f32 seconds)
{
    s32 delay = (s32)(seconds * 60.0f / 5.6f + 0.5f) - 1;
    if (mode != 2 && mode != 3)
        return;
    delay = delay < 0 ? 0 : delay > 15 ? 15 : delay;
    BeginNormalPaletteFade(PALETTES_ALL, delay, mode == 3 ? 0 : 16, mode == 3 ? 16 : 0, RGB_BLACK);
    sScriptBlack = mode == 3;
    MPNet_Log("orre vm: fade %s (%d)", mode == 3 ? "out" : "in", (int)delay);
}

static void FadeBack(void)
{
    if (sScriptBlack)
    {
        sScriptBlack = FALSE;
        BeginNormalPaletteFade(PALETTES_ALL, 4, 16, 0, RGB_BLACK);
        MPNet_Log("orre vm: fade in (the scene ended black)");
    }
}

// std182_tvNewsEvent (the decomp's gs_range_801C766C.c): the TV's news by the story's flags, after 3 frames, each
// line waiting for A; flags it marks as seen (std078 sets: orre_story.c). The TV's own picture (std171 / std211's
// texture change) isn't drawn yet
// std211_tvSetChannelByStory: the room's TV on the channel the story is at (the news not yet watched, or after it)
static void TvByStory(void)
{
    s32 ch;
    if (Orre_Flag(0xADA))
        ch = Orre_Flag(0xA0F) ? 12 : 11;
    else if (Orre_Flag(0xADB))
        ch = Orre_Flag(0x483) ? 12 : 11;
    else if (Orre_Flag(0x42C))
        ch = Orre_Flag(0x42D) ? 4 : 3;
    else if (Orre_Flag(0xAD3))
        ch = Orre_Flag(0xA08) ? 4 : 3;
    else if (Orre_Flag(0xAD0))
        ch = Orre_Flag(0xA07) ? 4 : 3;
    else if (Orre_Flag(0xACF))
        ch = Orre_Flag(0xA06) ? 2 : 1;
    else
        ch = 0;
    O3D_TvSet(sRoom, ch);
}

static void NewsEvent(struct Thread *t)
{
    u8 mode;
    t->newsLen = t->newsAt = 0;
    if (Orre_Flag(0xADA))
    {
        Orre_SetFlag(0xA0F, TRUE);
        t->news[0] = 0x439F, t->news[1] = 0x43A0, t->newsLen = 2;
        O3D_TvSet(sRoom, 12);
    }
    else if (Orre_Flag(0xADB))
    {
        Orre_SetFlag(0x483, TRUE);
        t->news[0] = 0x439D, t->news[1] = 0x439E, t->newsLen = 2;
        O3D_TvSet(sRoom, 12);
    }
    else
    {
        // (darkPokemonCheckFlag12(0x43), a late Shadow POKeMON's mark, isn't kept by Kai's records: the story's flag
        // 0x42C comes with it)
        if (Orre_Flag(0x42C))
            Orre_SetFlag(0x42D, TRUE), mode = 4;
        else if (Orre_Flag(0xAD3))
            Orre_SetFlag(0xA08, TRUE), mode = 3;
        else if (Orre_Flag(0xAD0))
            Orre_SetFlag(0xA07, TRUE), mode = 2;
        else if (Orre_Flag(0xACF))
            Orre_SetFlag(0xA06, TRUE), mode = 1;
        else
            Orre_SetFlag(0xA05, TRUE), mode = 0;
        t->news[0] = 0x426E + mode, t->newsLen = 1;
        TvByStory();
    }
    t->newsHeld = !t->holds;
    if (t->newsHeld)
        Hold(t);
    t->frames = 3;
    t->wait = W_NEWS;
    MPNet_Log("orre vm: the TV's news (%u lines)", t->newsLen);
}

// scriptSetCol(GATE, ON): GScolsys2SetObjEnable: the gate's tiles impassable while it's on (the maps are built with
// every gate open: tools/orre/maps.py). Gates overlap (PHENAC's 42 is the bottom row of 40, RUI's pen, and the place's
// way out): a tile stays shut while any closed gate covers it, or opening 42 let the player out of the pen before
// RUI was free (the user, 2026-11-08). Closed gates are laid again every frame: a battle's return rebuilds the map
static bool8 sGateClosed[ORRE_GATE_COUNT];

static void LayGate(u32 i, bool8 on)
{
    u32 k, j, m;
    for (k = 0; k < sOrreGates[i].count; k++)
    {
        const u8 *tile = sOrreGateTiles[sOrreGates[i].first + k];
        bool8 shut = on;
        for (j = 0; j < ORRE_GATE_COUNT && !shut; j++)
            if (j != i && sGateClosed[j] && sOrreGates[j].room == sRoom)
                for (m = 0; m < sOrreGates[j].count && !shut; m++)
                {
                    const u8 *o = sOrreGateTiles[sOrreGates[j].first + m];
                    shut = o[0] == tile[0] && o[1] == tile[1];
                }
        MapGridSetMetatileImpassabilityAt(tile[0] + MAP_OFFSET, tile[1] + MAP_OFFSET, shut);
    }
}

static void SetGate(u32 gate, bool8 on)
{
    u32 i;
    for (i = 0; i < ORRE_GATE_COUNT; i++)
        if (sOrreGates[i].room == sRoom && sOrreGates[i].gate == gate)
        {
            sGateClosed[i] = on;
            LayGate(i, on);
            MPNet_Log("orre vm: gate %u %s (%u tiles)", gate, on ? "closed" : "open", sOrreGates[i].count);
            return;
        }
}

// Colosseum's doors (the decomp's floor_event.c floorEventCtrlDoor; the table: tools/orre/colo_rooms.py Room.doors).
// A door's leaf is a collision object, off while the door is open: its tiles are a gate here. MODE 0 opens it (unless
// it's open already, or locked: its lock flag not yet set), 1 has it open at once, 2 closes it (if open), 3 has it
// closed at once; its flag remembers. The rooms' maps are built with every door open (tools/orre/maps.py): before
// this, a closed leaf was a wall for good and what lay behind it (the police station's cells, the labs' rooms, the
// bookshelf in front of PYRITE's hideout) couldn't be reached (2026-11-11).
static const struct OrreDoor *DoorOf(u32 index)
{
    u32 i;
    for (i = 0; i < ORRE_DOOR_COUNT; i++)
        if (sOrreDoors[i].index == index)
            return &sOrreDoors[i];
    return NULL;
}

static u32 CtrlDoor(u32 index, u32 mode)
{
    const struct OrreDoor *d = DoorOf(index);
    if (d == NULL)
        return (u32)-1;
    switch (mode)
    {
    case 0:
        if (d->open && Orre_Flag(d->open))
            return 0;
        if (d->lock && !Orre_Flag(d->lock))
            return 1;
        OrreSound_Play(d->type == 3 ? 0x4BE : 0x44, 100, FALSE);
        // fallthrough
    case 1:
        if (d->open && !Orre_Flag(d->open))
            Orre_SetFlag(d->open, TRUE);
        if (d->col >= 0 && d->room == sRoom)
            SetGate(d->col, FALSE);
        O3D_DoorMode(d->index, mode);
        break;
    case 2:
        if (d->open && !Orre_Flag(d->open))
            return 0;
        OrreSound_Play(d->type == 3 ? 0x4BE : 0x44, 100, FALSE);
        // fallthrough
    case 3:
        if (d->open && Orre_Flag(d->open))
            Orre_SetFlag(d->open, FALSE);
        if (d->col >= 0 && d->room == sRoom)
            SetGate(d->col, TRUE);
        O3D_DoorMode(d->index, mode);
        break;
    }
    return 0;
}

// A room's start (the decomp's floorInitDoors): each door open or closed by its flag. A door that opens when walked
// up to starts shut every time (its flag is one of Colosseum's first 150, the ones a room's stay lasts)
static void InitDoors(u32 room)
{
    u32 i;
    for (i = 0; i < ORRE_DOOR_COUNT; i++)
    {
        const struct OrreDoor *d = &sOrreDoors[i];
        if (d->room == room)
            CtrlDoor(d->index, d->type != 2 && !d->touch && d->open && Orre_Flag(d->open) ? 1 : 3);
    }
}

// The closed gate that covers tile X, Y in this room: its row + 1, 0 none
static u32 ClosedGateAt(s16 x, s16 y, s16 reach, u8 col)
{
    u32 i, k;
    for (i = 0; i < ORRE_GATE_COUNT; i++)
        if (sGateClosed[i] && sOrreGates[i].room == sRoom && sOrreGates[i].gate == col)
            for (k = 0; k < sOrreGates[i].count; k++)
            {
                const u8 *tile = sOrreGateTiles[sOrreGates[i].first + k];
                if (abs(tile[0] - x) <= reach && abs(tile[1] - y) <= reach)
                    return i + 1;
            }
    return 0;
}

// Doors that open when someone walks up to them (Colosseum's touch triggers in front of a door run the shared script
// 0x05960005: door mode 0): the player beside a closed leaf's tiles opens it
static void TouchDoors(void)
{
    struct ObjectEvent *p = &gObjectEvents[gPlayerAvatar.objectEventId];
    s16 x = p->currentCoords.x - MAP_OFFSET, y = p->currentCoords.y - MAP_OFFSET;
    float cx = 0, cz = 0;
    bool8 at = O3D_TileToColo(x, y, &cx, &cz);
    u32 i;
    for (i = 0; i < ORRE_DOOR_COUNT; i++)
    {
        const struct OrreDoor *d = &sOrreDoors[i];
        if (d->room != sRoom || !d->touch || (d->open && Orre_Flag(d->open)))
            continue;
        // (on its touch trigger, half a tile's slack: a tile's centre seldom lies in a thin trigger; or beside its
        // shut leaf's tiles)
        if ((at && d->x1 > d->x0 - 0.01f && (d->x0 != 0 || d->x1 != 0 || d->z0 != 0 || d->z1 != 0)
             && cx >= d->x0 - 4.0f && cx <= d->x1 + 4.0f && cz >= d->z0 - 4.0f && cz <= d->z1 + 4.0f)
         || (d->col >= 0 && ClosedGateAt(x, y, 1, d->col)))
            CtrlDoor(d->index, 0);
    }
}

// Tile X, Y is shut by a closed gate or door of this room
bool8 Orre_Vm_GateShutAt(s16 x, s16 y)
{
    u32 i, k;
    for (i = 0; i < ORRE_GATE_COUNT; i++)
        if (sGateClosed[i] && sOrreGates[i].room == sRoom)
            for (k = 0; k < sOrreGates[i].count; k++)
                if (sOrreGateTiles[sOrreGates[i].first + k][0] == x && sOrreGateTiles[sOrreGates[i].first + k][1] == y)
                    return TRUE;
    return FALSE;
}

// Tests' walker: tile X, Y is shut only by a door that opens when the player walks up to it
bool8 Orre_Vm_DoorWillOpen(s16 x, s16 y)
{
    u32 i;
    if (!sRoom)
        return FALSE;
    for (i = 0; i < ORRE_DOOR_COUNT; i++)
    {
        const struct OrreDoor *d = &sOrreDoors[i];
        if (d->room == sRoom && d->touch && d->col >= 0 && !(d->lock && !Orre_Flag(d->lock)) && ClosedGateAt(x, y, 0, d->col))
            return TRUE;
    }
    return FALSE;
}

static void KeepGates(void)
{
    u32 i, k;
    for (i = 0; i < ORRE_GATE_COUNT; i++)
        if (sGateClosed[i] && sOrreGates[i].room == sRoom)
            for (k = 0; k < sOrreGates[i].count; k++)
            {
                const u8 *tile = sOrreGateTiles[sOrreGates[i].first + k];
                MapGridSetMetatileImpassabilityAt(tile[0] + MAP_OFFSET, tile[1] + MAP_OFFSET, TRUE);
            }
}

static u32 Native(struct Thread *t, u32 n, const V *a, s32 count)
{
    V z = {0};
    static s8 sTrace = -1;
    if (sTrace < 0)
        sTrace = getenv("MOEMON_ORRE_VM_TRACE") != NULL;
    if (sTrace && n != N_HERO_MAIN)
        MPNet_Log("orre vm: %08X native %u (%d args: %u %u %u %u %u %u)", t->scriptId, n, count, count > 0 ? a[0].u : 0,
                  count > 1 ? a[1].u : 0, count > 2 ? a[2].u : 0, count > 3 ? a[3].u : 0, count > 4 ? a[4].u : 0,
                  count > 5 ? a[5].u : 0);
#define ARG(i) ((i) < count ? a[i] : z)
    switch (n)
    {
    // Colosseum's sounds, its own (include/orre_sound.h: the MusyX runtime on the disc's data). std104 the music
    // (id, fade, volume), std105 / 138 an effect or a second song (the wind), 106 / 153 stops, 137 a sound's volume,
    // 109 the master volume, 228 the music playing, 108 whether a sound plays
    case 104: return OrreSound_Play(ARG(0).u, ARG(2).u, TRUE);
    case 105: return OrreSound_Play(ARG(0).u, ARG(2).u, FALSE);
    case 138: return OrreSound_Play(ARG(0).u, ARG(2).u, FALSE);
    case 106: OrreSound_Stop(ARG(0).u, ARG(1).u); return 0;
    case 153: OrreSound_StopBgm(ARG(0).u); return 0;
    case 137: OrreSound_Volume(ARG(0).u, ARG(1).u, ARG(2).u); return 0;
    case 109: OrreSound_MasterVolume(ARG(0).u, ARG(1).u); return 1;
    case 228: return OrreSound_Bgm();
    case N_FLAG_CHECK: return Orre_Flag(ARG(0).u);
    case N_FLAG_GET: return Orre_FlagValue(ARG(0).u);
    case N_FLAG_SET1: Orre_SetFlag(ARG(0).u, TRUE); return 0;   // (the decomp's names are swapped: orre_story.c)
    case N_FLAG_SET0: Orre_SetFlag(ARG(0).u, FALSE); return 0;
    case N_FLAG_SET: Orre_SetFlagValue(ARG(0).u, ARG(1).u); return 0;
    case N_TALK:
        Orre_Npc_TalkStart(ARG(0).u, ARG(1).u);
        SpeakerName(ARG(0).u, ARG(1).u);
        ShowMessage(t, ARG(2).u, TRUE);
        return 0;
    case N_WIN_OPEN:
        ShowMessage(t, ARG(0).u, ARG(1).u != 0);
        return 0;
    case N_END_SPEECH:
        Orre_Npc_TalkEnd(ARG(0).u, ARG(1).u);
        // fall through
    case N_WIN_CLOSE:
        if (!IsFieldMessageBoxHidden())
            HideFieldMessageBox();
        return 0;
    case N_YESNO:
        DisplayYesNoMenuDefaultYes();
        t->wait = W_YESNO;
        return 0;
    case N_SLEEP:   // std157_sleep(SECONDS): its time over the refresh rate (gs_range_801C766C.c)
        t->frames = (s32)(ARG(0).f * 60.0f + 0.5f);
        t->wait = W_FRAMES;
        return 0;
    case N_HERO_INIT_EVENT: Hold(t); return 0;
    case N_HERO_TERM_EVENT: LetGo(t); return 0;
    case N_HERO_MAIN:   // (Kai's field moves the player; the loop is back from a scene it ran itself: they walk again)
        t->heroLoop = TRUE, t->stepped = TRUE, t->sinceHero = 0;
        if (t->loopHold)
        {
            t->loopHold = FALSE;
            LetGo(t);
        }
        return 0;
    case N_SHOP_OPEN:   // menuShopOpen(N): Kai's POKeMART with Colosseum's list N (the coupon counters: not yet)
        if (ARG(0).u < ORRE_SHOP_COUNT && sOrreShops[ARG(0).u].kind <= 1 && sOrreShops[ARG(0).u].items[0])
        {
            t->wait = W_SCREEN, t->screen = SCREEN_SHOP, t->shop = ARG(0).u, t->screenOpen = FALSE;
            MPNet_Log("orre vm: shop %u", ARG(0).u);
        }
        else
            MPNet_Log("orre vm: shop %u not opened (POKe COUPONS)", ARG(0).u);
        return 0;
    case N_WARP_MENU_ROOM:   // std098_mapWarpToMenuRoom: the PC's boxes are Kai's PC; the other menus aren't yet
        if (ARG(0).u == ROOM_PCBOX_MENU)
        {
            t->wait = W_SCREEN, t->screen = SCREEN_PC, t->screenOpen = FALSE;
            MPNet_Log("orre vm: the PC");
        }
        else
            MPNet_Log("orre vm: menu room %u not converted yet", ARG(0).u);
        return 0;
    case N_POKEMON_CENTER:   // std083: the POKeMON CENTER's ball machine (mode 0) or its heal (1-3): the party healed
        HealPlayerParty();
        PlayFanfare(MUS_HEAL);
        t->wait = W_FANFARE;
        // (a whiteout comes back here, as Kai's POKeMON CENTERs: the player at the counter)
        HealAt( gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum,
                    WARP_ID_NONE, gSaveBlock1Ptr->pos.x, gSaveBlock1Ptr->pos.y);
        MPNet_Log("orre vm: party healed");
        return 0;
    case N_CAMERA_PLAY:   // cameraPlayAnime(group, shot, frame, loop): Colosseum's shot in ORRE's view (o3d_cam.c)
        if (O3D_CamPlayId(ARG(1).u, (float)ARG(2).s, ARG(3).u != 0) && !t->holds)
            Hold(t), t->camHold = TRUE;
        return 0;
    case N_SCENE_MODE:   // GSscene_SetMode: 0 the field's (its camera back), 3 the scene's own (o3d_cam.c sFree)
        if (ARG(0).u == 3)
            O3D_FreeMode(TRUE);
        if (ARG(0).u != 0)
            return 0;
        O3D_CamStop();
        if (t->camHold)
            t->camHold = FALSE, LetGo(t);
        return 0;
    case N_CAMERA_RETURN:   // cameraReturn(SECONDS, WAIT) (its natives.json order: 0x0201): back to the field's camera
        O3D_CamStopShot();
        O3D_FreeReturn(ARG(0).f);
        if (ARG(1).u && O3D_FreeMoving())
            t->wait = W_CAM_MOVE;
        if (t->camHold)
            t->camHold = FALSE, LetGo(t);
        return 0;
    case N_CAMERA_TARGET:   // cameraSetTarget(group, person): the field's camera on the hero again (after a shot)
    case N_CAMERA_STOP: case N_CAMERA_STOP2:   // (a shot ends: the scene's own camera, if any, stays)
        O3D_CamStopShot();
        if (t->camHold)
            t->camHold = FALSE, LetGo(t);
        return 0;
    case N_CAM_SET_EYE: case N_CAM_SET_ROT: case N_CAM_MOVE_EYE: case N_CAM_MOVE_ROT:
    {
        // std040_cameraSetPosition / cameraSetRotation (X Y Z), cameraMovePositionXYZ / cameraMoveRotationXYZ (an int its
        // C function never reads (natives.json: 0x01020202 0x02), X Y Z, SECONDS)
        bool8 move = n == N_CAM_MOVE_EYE || n == N_CAM_MOVE_ROT;
        float v[3] = {ARG(move).f, ARG(move + 1).f, ARG(move + 2).f};
        O3D_FreeSet(n == N_CAM_SET_ROT || n == N_CAM_MOVE_ROT, v, move ? ARG(4).f : 0.0f);
        MPNet_Log("orre vm: camera %s %s %.2f %.2f %.2f (%.2f s)", n == N_CAM_SET_ROT || n == N_CAM_MOVE_ROT ? "rot" : "eye",
                  move ? "to" : "at", v[0], v[1], v[2], move ? ARG(4).f : 0.0f);
        return 0;
    }
    case N_CAM_SET_FOV: O3D_FreeFov(ARG(0).f); return 0;
    case N_CAMERA_END_CHECK:   // cameraMoveEndCheck(WAIT): whether the scene's camera still moves; WAIT: until it stops
        if (ARG(0).u && O3D_FreeMoving())
            t->wait = W_CAM_MOVE;
        return O3D_FreeMoving();
    case N_CAMERA_WAIT:   // cameraWaitSyncAnime: until the shot has played
        if (ARG(0).u)
            t->wait = W_CAMERA;
        return 0;
    case N_GET_MONEY: return GetMoney(&gSaveBlock1Ptr->money);
    case N_ADD_MONEY:
        if (ARG(0).s >= 0)
            AddMoney(&gSaveBlock1Ptr->money, ARG(0).s);
        else
            RemoveMoney(&gSaveBlock1Ptr->money, -ARG(0).s);
        return GetMoney(&gSaveBlock1Ptr->money);
    case N_HAVE_ITEM:
        if (IS_ORRE_KEY_ITEM(ARG(0).u))
            return Orre_Flag(ORRE_KEY_ITEM_FLAG(ARG(0).u));
        return KaiItem(ARG(0).u) && CheckBagHasItem(KaiItem(ARG(0).u), 1);
    case N_ADD_ITEM:
        if (IS_ORRE_KEY_ITEM(ARG(0).u))
        {
            Orre_SetFlag(ORRE_KEY_ITEM_FLAG(ARG(0).u), ARG(1).s > 0);
            return 0;
        }
        if (!KaiItem(ARG(0).u))
            return 0;
        if (ARG(1).s > 0)
            return AddBagItem(KaiItem(ARG(0).u), ARG(1).s);
        if (ARG(1).s < 0)
            return RemoveBagItem(KaiItem(ARG(0).u), -ARG(1).s);
        return 0;
    case N_GET_ITEM:   // scriptGetItem(item, count): floorEventGetTresure(4): the fanfare, the item, its line
        {
            u32 item = ARG(0).u, count = ARG(1).s > 0 ? ARG(1).u : 1, msg = count == 1 ? 0x3CB8 : 0x3CBD;
            u8 words[ORRE_STR_VAR_SIZE];
            WordsOf(words, 0x2D, item);
            Orre_SetStrVar(0x2D, words);
            WordsOf(words, 0x2F, count);
            Orre_SetStrVar(0x2F, words);
            if (IS_ORRE_KEY_ITEM(item))
                Orre_SetFlag(ORRE_KEY_ITEM_FLAG(item), TRUE);
            else if (KaiItem(item) && !AddBagItem(KaiItem(item), count))
                AddPCItem(KaiItem(item), count), msg = count == 1 ? 0x3CBA : 0x3CBB;   // (a full bag: the PC)
            PlayFanfare(MUS_OBTAIN_ITEM);
            ShowMessage(t, msg, TRUE);
            MPNet_Log("orre vm: item %u x%u", item, count);
            return 0;
        }
    case N_FLOOR_LINK:      // floorLink(room, entry): to Colosseum's room ROOM, landing on its entry ENTRY
    case N_FLOOR_LINK_SE:   // floorLinkWithSE(room, entry, sound)
        if (ARG(0).u && O3D_RoomEntry(ARG(0).u, ARG(1).u, &sWarpMap, &sWarpX, &sWarpY))
        {
            sLinked = TRUE, sLinkEntry = ARG(1).u;
            t->wait = W_SCREEN, t->screen = SCREEN_WARP, t->screenOpen = FALSE;
            if (!t->holds)
                Hold(t);
            MPNet_Log("orre vm: floorLink %u entry %u", ARG(0).u, ARG(1).u);
        }
        else if (ARG(0).u)
            MPNet_Log("orre vm: floorLink %u: not a map", ARG(0).u);
        return 0;
    case N_MT_BATTLE_LEAVE:   // std229_mapLoad395: leaving MT. BATTLE's course (its menu room 917 sends the player back
        if (O3D_RoomEntry(ROOM_MT_BATTLE_PC, 0, &sWarpMap, &sWarpX, &sWarpY))   // to the reception)
        {
            sLinked = TRUE, sLinkEntry = 0;
            t->wait = W_SCREEN, t->screen = SCREEN_WARP, t->screenOpen = FALSE;
            if (!t->holds)
                Hold(t);
        }
        return 0;
    case N_PREV_FLOOR: return sPrevRoom;
    case N_NEXT_FLOOR: return sRoom;
    case N_NEXT_POS: return sEntry;
    case N_CHOICE:   // std199_menuDisplayCustomMenu(the choices' messages, count, x, y, flags): the row chosen
        {
            u32 i;
            t->nchoices = ARG(1).u < ARRAY_COUNT(t->choices) ? ARG(1).u : ARRAY_COUNT(t->choices);
            for (i = 0; i < t->nchoices; i++)
                t->choices[i] = Load(t, Deref(t, ARG(0).u + i)).u;
            t->wait = W_SCREEN, t->screen = SCREEN_CHOICE, t->screenOpen = FALSE;
            if (!t->holds)
                Hold(t);
            return 0;   // (its result comes when the menu closes: Orre_VmChoiceDone)
        }
    case N_SUB_YESNO:   // menuSubOpenYesNo(type, x, y, the cursor's start): 0 YES, as std011
        DisplayYesNoMenuDefaultYes();
        t->wait = W_YESNO;
        return 0;
    case N_CUSTOM43: return 0;
    case N_SURPRISE: Orre_Npc_Surprise(ARG(0).u, ARG(1).u); return 0;
    case N_WALK_RESERVE: Orre_Npc_WalkListReset(ARG(1).u); return 1;
    case N_WALK_ADD: Orre_Npc_WalkListAdd(ARG(1).u, ARG(2).f, ARG(4).f); return 1;
    case N_WALK_START: Orre_Npc_WalkListStart(ARG(1).u, ARG(2).u != 0); return 1;
    case N_GET_POSITION:   // std172_characterGetPosition(group, index, &vec): x, y, z written through the address
        {
            float cx = 0, cz = 0;
            V v;
            Orre_Npc_GetColo(ARG(0).u, ARG(1).u, &cx, &cz);
            v.f = cx, Store(t, Deref(t, ARG(2).u), v);
            v.f = 0, Store(t, Deref(t, ARG(2).u + 1), v);
            v.f = cz, Store(t, Deref(t, ARG(2).u + 2), v);
            return 0;
        }
    case N_GAZE_HERO: return Orre_Npc_SeesHero(ARG(1).u);
    case N_THREAD_YIELD:
        t->frames = 1, t->wait = W_FRAMES;
        return 0;
    case N_MODEL_RES: return ARG(0).u;
    case N_PARTY_STATUS:   // std216_partyStatusUpdate_8ae: mode 0 the party healed and counted (between a colosseum's
        if (ARG(0).u == 0)   // rounds); 1 the battle's own states cleared
        {
            HealPlayerParty();
            return CalculatePlayerPartyCount();
        }
        return 0;
    case N_GET_PREMIUM: return Orre_FlagValue(0xA9E);   // (the colosseum's prize: flag 0xA9E)
    case N_SET_PREMIUM: Orre_SetFlagValue(0xA9E, ARG(0).u); return 0;
    case N_CHECK_TEMOCHI: return 1;   // (the party fits a challenge's rules: Kai's own rules apply)
    case N_WIN_OPEN_SE:
        ShowMessage(t, ARG(0).u, ARG(1).u != 0);
        return 0;
    case N_SHADOW_COUNT:   // scriptGetDarkPokemonNum: Shadow Pokémon in the party
        {
            u32 i, c = 0;
            for (i = 0; i < PARTY_SIZE; i++)
                if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES) && Orre_Shadow_Is(&gPlayerParty[i]))
                    c++;
            return c;
        }
    case N_PARTY_COUNT: return CalculatePlayerPartyCount();
    case N_PARTNER_FOLLOW:   // std035_setPartnerFollowing: RUI walks behind the player (orre_npcs.c, character 101)
        Orre_Npc_PartnerFollow(ARG(0).u != 0);
        return 0;
    case N_SHADOW_OPEN_COUNT:   // scriptGetDarkPointZeroPokemonNum: the party's Shadow Pokémon whose heart is open
        {
            u32 i, c = 0;
            for (i = 0; i < PARTY_SIZE; i++)
                if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES) && Orre_Shadow_Level(&gPlayerParty[i]) == ORRE_SHADOW_OPEN)
                    c++;
            return c;
        }
    case N_RELIVE:   // preReliveSetParameter(0, 0): the RELIC STONE's purification: Kai's (party_menu.c Orre_RelicStone)
        if (ARG(0).u == 0)
            t->wait = W_SCREEN, t->screen = SCREEN_RELIC, t->screenOpen = FALSE;
        else
            MPNet_Log("orre vm: relive mode %u (%u) not converted yet", ARG(0).u, ARG(1).u);
        return 0;
    case N_NAME_ENTRY:   // menuNameEntryOpen(MODE, N): 0 the hero's name (Kai's player has theirs), 1 the partner's:
        // PHENAC's scene, once she's out of the sack: Kai's naming screen under Colosseum's "Her name is..."
        // (it was skipped, taken for the hero's: the user, 2026-11-11)
        if (ARG(0).u == 1 && sHolder == t)
            t->wait = W_SCREEN, t->screen = SCREEN_NAME, t->screenOpen = FALSE;
        return 1;
    case N_GIFT_POKEMON:   // std217_playerReceiveGiftPokemon: 3 DUKING's PLUSLE, 4 HO-OH (1, 2: Colosseum's hero's own)
        Orre_GiveGift(ARG(0).u);
        return 0;
    case N_EXEC_TASK:   // scriptExecTask(script, priority, 4 arguments): a script alongside this one
    case N_RUN_SCRIPT:  // std209_threadRunScriptMode4(script, 4 arguments): one run to its end before this goes on
        {
            u32 args[4], i, first = n == N_EXEC_TASK ? 2 : 1;
            struct Thread *c;
            for (i = 0; i < 4; i++)
                args[i] = ARG(first + i).u;
            if ((c = Start(ARG(0).u, 4, args, t->room)) == NULL)
                return 0;
            if (n == N_RUN_SCRIPT)
                t->child = c, t->childId = c->scriptId, t->wait = W_THREAD;
            c->handle = ++sHandles ? sHandles : ++sHandles;
            return c->handle;   // (std006_threadWaitForCompletion's)
        }
    case N_THREAD_WAIT:   // std006_threadWaitForCompletion(the task scriptExecTask gave): until it has run
        {
            u32 i;
            for (i = 0; i < VM_THREADS; i++)
                if (sThreads[i].state == T_RUN && sThreads[i].handle == ARG(0).u && ARG(0).u)
                    t->child = &sThreads[i], t->childId = sThreads[i].scriptId, t->wait = W_THREAD;
            return 0;
        }
    case N_SAVE_COLOSSEUM:   // std223_askYesNoThenWarp395: save, then a colosseum's challenge (menu room 917)
        MPNet_Log("orre vm: a colosseum's challenge (not converted yet)");
        return 0;
    case N_IN_BOUNDS:
        if (ARG(0).u == 0 && ARG(1).u == 100)   // (the hero)
            return HeroInBox(ARG(2).f, ARG(3).f, ARG(4).f, ARG(5).f);
        return 0;
    case N_ROOM_ID: return sRoom;
    case N_GROUP_ID: return 1;
    case N_SET_VALUE:
        {
            u8 words[ORRE_STR_VAR_SIZE];
            WordsOf(words, ARG(0).u, ARG(1).u);
            Orre_SetStrVar(ARG(0).u, words);
        }
        return 0;
    case N_FIGHT_START:
    case N_FIELD_START:
        t->battle = ARG(0).u;
        t->wait = W_BATTLE;
        if (!t->holds)
            Hold(t);
        return 0;
    case N_BATTLE_RESULT: return Orre_BattleResult();
    case N_FADE_SET: Fade(ARG(0).u, ARG(1).f); return 0;
    case N_FADE_CHECK:   // fadeCheck(WAIT): whether a fade runs; WAIT: until it's done
        if (ARG(0).u && gPaletteFade.active)
            t->wait = W_FADE;
        return gPaletteFade.active;
    case N_TV_NEWS: NewsEvent(t); return 1;
    case 171: O3D_TvSet(sRoom, ARG(0).s); return 1;   // std171_mapSetTextureChange(CHANNEL): the room's TV
    case 211: TvByStory(); return 1;                 // std211_tvSetChannelByStory
    case N_PEOPLE_WAIT_SYNC: case N_CHAR_SET_ANIM: return 1;
    // ---- the room's people (orre_npcs.c)
    case N_PEOPLE_VISIBLE: Orre_Npc_SetShown(ARG(1).u, ARG(2).u != 0); return 0;
    case N_FLOOR_CHAR_DISP: Orre_Npc_SetDisp(ARG(0).u, ARG(1).u != 0); return 0;
    case N_PEOPLE_OPEN: Orre_Npc_SetShown(ARG(1).u, TRUE); return 0;
    case N_PEOPLE_CLOSE: Orre_Npc_SetShown(ARG(1).u, FALSE); return 0;
    case N_SET_POSITION: Orre_Npc_SetPosition(ARG(0).u, ARG(1).u, ARG(2).f, ARG(4).f); return 0;
    case N_SET_ROTATION: Orre_Npc_SetAngle(ARG(0).u, ARG(1).u, ARG(3).f); return 0;
    case N_TURN_TO_ANGLE: Orre_Npc_SetAngle(ARG(0).u, ARG(1).u, ARG(2).f); return 0;
    case N_SET_COL: SetGate(ARG(0).u, ARG(1).u != 0); return ARG(0).u;
    case 96: return CtrlDoor(ARG(1).u, ARG(2).u);   // floorEventCtrlDoor(room, door, mode)
    case N_WALK_SPEED: Orre_Npc_SetSpeed(ARG(0).u, ARG(1).u, ARG(2).f); return 0;   // peopleSetWalkSpeed
    case N_TURN_TO_POINT: Orre_Npc_FacePoint(ARG(0).u, ARG(1).u, ARG(2).f, ARG(4).f); return 0;   // peopleTurnToPoint
    case N_MOVE_TO: Orre_Npc_MoveTo(ARG(0).u, ARG(1).u, ARG(3).f, ARG(5).f); return 0;
    case N_MOVE_TO_CHARACTER: Orre_Npc_MoveToOther(ARG(0).u, ARG(1).u, ARG(2).u, ARG(3).u); return 0;
    case N_APPROACH_PLAYER:
        Orre_Npc_MoveToOther(ARG(0).u, ARG(1).u, 0, 100);
        t->moveGroup = ARG(0).u, t->moveIndex = ARG(1).u;
        t->wait = W_MOVE;
        return 0;
    case N_PEOPLE_MOVE_CHECK:
        if (ARG(2).u == 0)
            return Orre_Npc_Done(ARG(0).u, ARG(1).u);
        t->moveGroup = ARG(0).u, t->moveIndex = ARG(1).u;
        t->wait = W_MOVE;
        return 1;
    case N_FACE_CHARACTER: case N_TURN_HEAD_TO: Orre_Npc_FaceOther(ARG(0).u, ARG(1).u, ARG(2).u, ARG(3).u); return 0;
    case N_LOOK_AT_PLAYER: Orre_Npc_FaceOther(ARG(0).u, ARG(1).u, 0, 100); return 0;
    }
#undef ARG
    {
        u32 i;
        for (i = 0; i < ARRAY_COUNT(sQuiet); i++)
            if (sQuiet[i].n == n)
                return sQuiet[i].ret;
    }
    if (n < ARRAY_COUNT(sLogged) && !sLogged[n])
    {
        sLogged[n] = TRUE;
        MPNet_Log("orre vm: native %u not converted yet (does nothing)", n);
    }
    return 0;
}

// ---- the interpreter (gs_vm.c cms_ScriptExecute and the cms_op* handlers)

static void Binary(struct Thread *t, u8 op)
{
    u8 rd = Code8(t), ld = Code8(t);
    V r = Operand(t, rd), l = Operand(t, ld), out;
    bool8 ints = IsInt(ld) && IsInt(rd);
    f32 lf = AsF(l, ld), rf = AsF(r, rd);
    out.u = 0;
    switch (op)
    {
    case 13: out.s = (IsInt(ld) ? l.s : (s32)l.f) & (IsInt(rd) ? r.s : (s32)r.f); break;
    case 14: out.s = (IsInt(ld) ? l.s : (s32)l.f) | (IsInt(rd) ? r.s : (s32)r.f); break;
    case 20: if (ints) out.s = l.s * r.s; else out.f = lf * rf; break;
    case 21: if (ints) out.s = r.s ? l.s / r.s : 0; else out.f = lf / rf; break;
    case 22: if (ints) out.s = r.s ? l.s % r.s : 0; else out.f = fmodf(lf, rf); break;
    case 23: if (ints) out.s = l.s + r.s; else out.f = lf + rf; break;
    case 24: if (ints) out.s = l.s - r.s; else out.f = lf - rf; break;
    case 25: out.s = ints ? l.s < r.s : lf < rf; break;
    case 26: out.s = ints ? l.s <= r.s : lf <= rf; break;
    case 27: out.s = ints ? l.s > r.s : lf > rf; break;
    case 28: out.s = ints ? l.s >= r.s : lf >= rf; break;
    case 29: out.s = ints ? l.s == r.s : lf == rf; break;
    case 30: out.s = ints ? l.s != r.s : lf != rf; break;
    case 33: out.s = (IsInt(ld) ? l.s != 0 : l.f != 0) && (IsInt(rd) ? r.s != 0 : r.f != 0); break;
    case 34: out.s = (IsInt(ld) ? l.s != 0 : l.f != 0) || (IsInt(rd) ? r.s != 0 : r.f != 0); break;
    }
    Push(t, out.u);
}

// Run T until it waits or ends
static void Run(struct Thread *t)
{
    u32 steps = 0;
    while (t->state == T_RUN && t->wait == W_NONE && steps++ < VM_STEPS_PER_FRAME)
    {
        u8 op = Code8(t);
        switch (op)
        {
        case 0: break;
        case 1: Finish(t); return;
        case 2: Push(t, 0xFFFFFFFF); break;
        case 3: { u16 n; t->ip++; n = Code16(t); while (n--) Pop(t); break; }
        case 4:
        {
            u32 func = Code32(t);
            u16 argc = Code16(t), locals = Code16(t);
            Push(t, t->ip);
            Push(t, t->frame);
            Push(t, locals);
            t->frame = t->sp - (argc + locals + 3);
            if (func < t->bank->funcs)
                t->ip = Be32(t->bank->d + 0x18 + 4 * func);
            break;
        }
        case 5:
        {
            V argc = Pop(t), frame = Pop(t), ret = Pop(t);
            s32 i;
            t->frame = frame.s;
            for (i = 0; i < argc.s; i++)
                Pop(t);
            if (ret.u == 0)
            {
                Finish(t);
                return;
            }
            t->ip = ret.u;
            break;
        }
        case 6: t->ip = Code32(t); break;
        case 7: { u32 to = Code32(t); if (Pop(t).s == 0) t->ip = to; break; }
        case 8: t->ip++; Push(t, Code32(t)); break;
        case 13: case 14: case 20: case 21: case 22: case 23: case 24: case 25: case 26: case 27: case 28: case 29:
        case 30: case 33: case 34:
            Binary(t, op);
            break;
        case 15: { u8 d = Code8(t); V idx = Pop(t); Push(t, idx.u); Push(t, Operand(t, d | 0x100).u); break; }
        case 16:
        {
            u8 d = Code8(t);
            u16 n = Code16(t), i;
            V start = Pop(t);
            for (i = 0; i < n; i++)
            {
                Push(t, start.u + i);
                Push(t, Operand(t, d).u);
            }
            break;
        }
        case 17: { u8 d = Code8(t); V v = Operand(t, d); V r; if (IsInt(d)) r.s = !v.s; else r.f = !v.f; Push(t, r.u); break; }
        case 18: { u8 d = Code8(t); V v = Operand(t, d); V r; if (IsInt(d)) r.s = -v.s; else r.f = -v.f; Push(t, r.u); break; }
        case 19: { u8 d = Code8(t); Push(t, Operand(t, d).u); break; }
        case 31:
        {
            u8 sd = Code8(t), dd = Code8(t);
            V v = Operand(t, sd), idx;
            if ((dd & 0x3F) != (sd & 0x3F))
            {
                if (IsInt(sd))
                    v.f = (f32)v.s;
                else
                    v.s = (s32)v.f;
            }
            if (dd != 0 && !(dd & 0x80))
            {
                idx = Pop(t);
                Store(t, Var(t, dd, idx.s), v);
            }
            Push(t, v.u);
            break;
        }
        case 32:
        {
            u8 sd = Code8(t), dd = Code8(t);
            u16 n = Code16(t), i;
            V tmp[64];
            V dst;
            if (dd & 0x80 || n > 64)
                break;
            if (sd & 0x80)
            {
                t->sp -= n;
                for (i = 0; i < n; i++)
                    tmp[i] = t->st[t->sp + i];
            }
            else
            {
                V si = Pop(t);
                for (i = 0; i < n; i++)
                    tmp[i] = Load(t, Var(t, sd, si.s + i));
            }
            dst = Pop(t);
            for (i = 0; i < n; i++)
                Store(t, Var(t, dd, dst.s + i), tmp[i]);
            for (i = 0; i < n; i++)
                Push(t, tmp[i].u);
            break;
        }
        case 35:   // (printf: the developers' log)
        {
            u16 argc;
            t->ip += 2;
            argc = Code16(t);
            Push(t, t->frame);
            Push(t, argc);
            t->frame = t->sp - (argc + 2);
            {
                V a = Pop(t), f = Pop(t);
                s32 i;
                t->frame = f.s;
                for (i = 0; i < a.s; i++)
                    Pop(t);
            }
            break;
        }
        case 36:   // (a wait of N frames: the yield)
        case 37:   // (a native: its index first, then its arguments)
        {
            u16 argc;
            s32 base;
            t->ip += 2;
            argc = Code16(t);
            Push(t, t->frame);
            Push(t, argc);
            t->frame = t->sp - (argc + 2);
            base = t->frame;
            if (op == 36)
            {
                t->frames = t->st[base].s;
                t->wait = W_FRAMES;
            }
            else if (base >= 1)
            {
                s32 count = t->sp - base - 3;
                t->st[base - 1].u = Native(t, t->st[base].u, &t->st[base + 1], count > 0 ? count : 0);
            }
            {
                V a = Pop(t), f = Pop(t);
                s32 i;
                t->frame = f.s;
                for (i = 0; i < a.s; i++)
                    Pop(t);
            }
            break;
        }
        default:
            MPNet_Log("orre vm: script %08X: unknown code %02X at %05X", t->scriptId, op, t->ip - 1);
            Finish(t);
            return;
        }
    }
}

// The result a native that waited leaves for the script (in its slot: the value left on the stack after its frame)
static void Resume(struct Thread *t, u32 result)
{
    if (t->sp > 0)
        t->st[t->sp - 1].u = result;
    t->wait = W_NONE;
}

static void Poll(struct Thread *t)
{
    switch (t->wait)
    {
    case W_FRAMES:
        if (--t->frames <= 0)
            t->wait = W_NONE;
        break;
    case W_MOVE:
        if (Orre_Npc_Done(t->moveGroup, t->moveIndex))
            t->wait = W_NONE;
        break;
    case W_MESSAGE:
    case W_MESSAGE_KEY:
        // (the field closed the box under the line: a scene that lets the player go and says its next line at once
        // has Kai's hold script end a frame later, its releaseall hiding the box. The line again, or the thread waited
        // for good on words nobody saw: the PRE GYM's first trainer never battled)
        // (the box's task gone with its text still printing: nothing runs the printer any more. A finished line
        // reads "hidden" too, its printer done)
        if (IsFieldMessageBoxHidden() && IsTextPrinterActive(0) && t->message && !ScriptContext_IsEnabled())
        {
            ShowMessage(t, t->message, t->wait == W_MESSAGE_KEY);
            break;
        }
        if (t->wait == W_MESSAGE)
        {
            if (!IsTextPrinterActive(0))
                t->wait = W_NONE;
            break;
        }
        if (!IsTextPrinterActive(0) && JOY_NEW(A_BUTTON | B_BUTTON))
        {
            PlaySE(SE_SELECT);
            t->wait = W_NONE;
            if (t->newsLen)   // (the TV's news: its next line, or its end)
                t->wait = W_NEWS;
        }
        break;
    case W_FADE:
        if (!gPaletteFade.active)
            t->wait = W_NONE;
        break;
    case W_CAM_MOVE:
        if (!O3D_FreeMoving())
            t->wait = W_NONE;
        break;
    case W_NEWS:
        if (t->frames > 0 && --t->frames > 0)
            break;
        if (t->newsAt < t->newsLen)
        {
            if (t->newsAt == 1 && t->newsLen == 2)   // (the two-line reports' second picture)
                O3D_TvSet(sRoom, t->news[1] == 0x43A0 ? 15 : 14);
            ShowMessage(t, t->news[t->newsAt++], TRUE);
        }
        else
        {
            if (t->newsLen == 2)
                TvByStory();
            if (!IsFieldMessageBoxHidden())
                HideFieldMessageBox();
            t->newsLen = t->newsAt = 0;
            t->wait = W_NONE;
            if (t->newsHeld)
                t->newsHeld = FALSE, LetGo(t);
        }
        break;
    case W_YESNO:
    {
        s8 c = Menu_ProcessInputNoWrapClearOnChoose();
        if (c != MENU_NOTHING_CHOSEN)
        {
            MPNet_Log("orre vm: yes/no %s", c == 0 ? "YES" : "NO");
            Resume(t, c == 0 ? 0 : 1);
        }
        break;
    }
    case W_THREAD:
        if (t->child == NULL || t->child->state != T_RUN || t->child->scriptId != t->childId)
            t->wait = W_NONE, t->child = NULL;
        break;
    case W_CAMERA:
        if (O3D_CamDone())
            t->wait = W_NONE;
        break;
    case W_FANFARE:
        if (WaitFanfare(FALSE))
            t->wait = W_NONE;
        break;
    case W_SCREEN:
        // (the hold script goes to the screen, Orre_VmScreen tells it which; back from it, it holds again)
        if (sHolder != t)
            t->wait = W_NONE;
        else if (!t->screenOpen && sHolding)
        {
            sScreen = t->screen, sShop = t->shop, sScreenThread = t;
            t->screenOpen = TRUE;
            sHolding = FALSE;
            ScriptContext_Enable();
        }
        else if (t->screenOpen && sHolding)
        {
            if (t->screen == SCREEN_CHOICE)
                Resume(t, t->result);
            else
                t->wait = W_NONE;
        }
        break;
    case W_BATTLE:
        if (sHolding && sHolder == t)
        {
            // (while Orre_EventScript_VmHold waits: the battle; after it the hold goes on, VAR_RESULT 2, and the
            // battle's end gives the thread its result: Orre_VmBattleDone)
            if (Orre_PrepareBattle(t->battle))
            {
                sHolding = FALSE;
                BattleSetup_StartOrreBattle();
            }
            else
                Resume(t, ORRE_BATTLE_LOST);
        }
        break;
    }
}

// ---- the game's side

// Every overworld frame (O3D_Update): the room's threads run until they wait
void Orre_Vm_Frame(void)
{
    u16 room = 0;
    u32 i;
    bool8 away = FALSE;
    if (O3D_RoomFile(&room) == NULL)
    {
        if (sRoom)
            Orre_Vm_Stop();
        return;
    }
    if (room != sRoom)
    {
        // (a warp's last field frame: WarpIntoMap has put the save on the next map already, in the warp's task, and
        // this runs after the tasks in that same frame. The room isn't loaded yet: entered then, its set-up and the
        // first frame of its main loop ran, the map's load (Orre_Vm_Warped) stopped them and the room started again.
        // A scene that starts as its room is entered had set its first flag by then and was never seen: the Mayor's
        // house, with FOLLY's and TRUDLY's battles and the first Shadow Pokémon (2026-11-11))
        if (gMain.callback2 != CB2_Overworld && gMain.callback2 != CB2_OverworldBasic)
            return;
        Orre_Vm_RoomEntered();
    }
    if (getenv("MOEMON_ORRE_VM_TRACE"))
    {
        static u32 sTick;
        if (++sTick % 120 == 0)
            for (i = 0; i < VM_THREADS; i++)
                if (sThreads[i].state == T_RUN)
                    MPNet_Log("orre vm: thread %u %08X wait %u holds %u (holding %u, printer %u, box hidden %u, fade %u, script %u, state %08X)", i,
                              sThreads[i].scriptId, sThreads[i].wait, sThreads[i].holds, sHolding,
                              IsTextPrinterActive(0), IsFieldMessageBoxHidden(), gPaletteFade.active, ScriptContext_IsEnabled(), MP_TextPrinterDebug(0));
    }
    if (getenv("MOEMON_ORRE_TOUCH_DEBUG"))
    {
        static u32 sTick2;
        if (++sTick2 % 120 == 0)
            MPNet_Log("orre vm: state: holder %08X pending %u holding %u script %u fade %u locked %u", sHolder ? sHolder->scriptId : 0,
                      sHoldPending, sHolding, ScriptContext_IsEnabled(), gPaletteFade.active, ArePlayerFieldControlsLocked());
    }
    if (gPaletteFade.active)
        return;   // (the field comes back: the scripts wait until it's there, as Colosseum's fades do)
    TryHold();
    KeepGates();
    if (sRoom)
        TouchDoors();
    Orre_Npcs_Frame();   // (people's looped walk lists)
    if (sHolder == NULL && !sHoldPending && !ScriptContext_IsEnabled() && sRoom)
        Orre_Touch_Frame();   // (the touch spots: orre_npcs.c)
    for (i = 0; i < VM_THREADS; i++)
    {
        struct Thread *t = &sThreads[i];
        if (t->state != T_RUN)
            continue;
        if (t->wait == W_BATTLE && !(sHolding && sHolder == t))
        {
            Poll(t);
            continue;
        }
        if (t->wait != W_NONE)
            Poll(t);
        if (t->wait == W_NONE)
            Run(t);
        // (Colosseum's player only moves while a room's loop steps them, heroMoveMain every frame: a scene the loop
        // runs itself, with no heroMoveInitEvent, has them stand still all the same. PHENAC's three from TEAM SNAGEM:
        // the player walked off while they talked)
        if (t->heroLoop && t->state == T_RUN && !t->holds)
        {
            if (t->sinceHero < 255)
                t->sinceHero++;
            if (t->sinceHero > 3)
                away = TRUE;   // (about to wait: the scene's camera stays the scene's)
            // (a scene on its first pass, before it ever stepped them: at once, or the field took the scene's camera
            // back; later a few frames' grace, the loop's own wait between steps)
            if (t->sinceHero > (t->stepped ? 6 : 0) && !getenv("MOEMON_ORRE_NO_LOOP_HOLD"))
            {
                t->loopHold = TRUE;
                Hold(t);
                MPNet_Log("orre vm: %08X is off in a scene: the player waits", t->scriptId);
            }
        }
    }
    // (the player walks: no scene holds them and nothing locks the field. A scene's camera left where the scene put
    // it (PHENAC's kidnappers end by finishing, never letting go) eases back to the field's, the user 2026-11-08)
    if (sHolder == NULL && !sHoldPending && !sHolding && !away && !ScriptContext_IsEnabled() && !ArePlayerFieldControlsLocked()
     && O3D_CamActive() && !O3D_FreeBack() && !O3D_FreeMoving())
    {
        MPNet_Log("orre vm: the player walks: the scene's camera goes back to the field's");
        O3D_CamEaseBack(0.6f);
    }
}

// A warp (overworld.c LoadMapFromWarp): the next frame on an ORRE map enters its room anew, even the same room
// (a return from a battle isn't a warp: the scene that started it goes on)
void Orre_Vm_Warped(void)
{
    if (sRoom)
    {
        // (the room's script on leaving first: its floor record's + 0x38, run through at once)
        const u8 *floor = FloorOf(sRoom);
        struct Thread *t;
        if (floor && Be32(floor + 0x38) && (t = Start(Be32(floor + 0x38), 0, NULL, TRUE)) != NULL)
            Run(t);
        MPNet_Log("orre vm: warped: room %u's scripts stop", sRoom);
        // (the room's own sounds end with it, as Colosseum's floor change ends them: its looping effects and its wind
        // went on into the next room, which has its own or none (the user, 2026-11-11). Its song stays: the next
        // room's sound script keeps it if it's the same, or fades it into its own)
        OrreSound_StopEffects();
        OrreSound_StopAmbient(400);
    }
    sScriptBlack = FALSE;   // (the warp's own fade brings the next room in)
    if (sLossPending)
    {
        // (the whiteout's warp: the story as the scene found it, so it plays again)
        u32 size;
        u8 *st = Orre_StoryState(&size);
        sLossPending = FALSE;
        if (sSceneStory && sSceneStorySize == size)
            memcpy(st, sSceneStory, size);
        MPNet_Log("orre vm: the scene's battle lost: the story as it was before it");
    }
    Orre_Vm_Stop();
}

void Orre_Vm_Stop(void)
{
    u32 i;
    if (sRoom && O3D_CamActive() && !O3D_CamKeep())
        O3D_CamStop();   // (a scene's shot ends with its room's scripts)
    for (i = 0; i < VM_THREADS; i++)
        sThreads[i].state = T_FREE;
    sHolder = NULL;
    sHoldPending = FALSE;
    sRoom = 0;
    Orre_Npcs_Clear();
}

// The player arrived in an ORRE room: its bank, its set-up, its main loop (Colosseum's floor record)
// Colosseum's scripts run in ORRE's rooms: the story (MOEMON_ORRE_SCRIPTS=0 leaves them off: tests of the maps alone)
bool8 Orre_Vm_ScriptsOn(void)
{
    const char *e = getenv("MOEMON_ORRE_SCRIPTS");
    return e == NULL || strcmp(e, "0") != 0;
}

void Orre_Vm_RoomEntered(void)
{
    u16 room = 0;
    const char *file = O3D_RoomFile(&room);
    const u8 *floor;
    u32 k;
    Orre_Vm_Stop();
    if (file == NULL || !Orre_Vm_ScriptsOn())
        return;
    sRoom = room;
    // (floorGetPrevFloorID, floorGetNextPosIndex: the room before, the entry the player came in by)
    if (sLastRoom != room)
        sPrevRoom = sLastRoom, sLastRoom = room;
    sEntry = sLinked ? sLinkEntry : O3D_EntryAt(gSaveBlock1Ptr->pos.x, gSaveBlock1Ptr->pos.y);
    sLinked = FALSE;
    if (room == 5 && Orre_Flag(1002) && !Orre_Flag(2481) && !Orre_Flag(1017))
    {
        // (the Mayor's house, its scene begun (1002) and TRUDLY never beaten (2481): the scene was cut off as it
        // started until 1.189.6 (Orre_Vm_Frame), so a save from before has never met the Mayor, FOLLY and TRUDLY
        // here, or the first Shadow Pokémon. Before PYRITE's story (1017) the scene plays again)
        Orre_SetFlag(1002, FALSE);
        MPNet_Log("orre vm: the Mayor's house scene never played: it plays again");
    }
    if (room == 3 && Orre_Flag(993) && !Orre_Flag(996))
    {
        // (PHENAC, the kidnappers gone and RUI still in the sack: a state Colosseum's set-up has no case for, its
        // pen never let the player out; a save that left it before 1.180.0, or saved in it, found no sack and the
        // pen shut for good (the user, 2026-11-08). The story goes back to before the kidnappers' scene, which
        // plays again)
        Orre_SetFlag(993, FALSE);
        Orre_SetFlag(994, FALSE);
        Orre_SetFlag(995, FALSE);
        MPNet_Log("orre vm: PHENAC with RUI still in the sack: the kidnappers' scene plays again");
    }
    LoadRoomBank(file);
    Orre_Npcs_RoomEntered(room);
    if ((floor = FloorOf(room)) == NULL || sRoomBank.d == NULL)
        return;
    {
        // (Colosseum's collision objects start on: a gate its scripts only ever open starts closed, as its set-up
        // finds it, the set-up opening what the story has opened: tools/orre/gen_gates.py)
        u32 i;
        memset(sGateClosed, 0, sizeof(sGateClosed));   // (a room entered anew: built with every gate open)
        for (i = 0; i < ORRE_GATE_COUNT; i++)
            if (sOrreGates[i].room == room && sOrreGates[i].closed)
                SetGate(sOrreGates[i].gate, TRUE);
        InitDoors(room);
    }
    for (k = 0x28; k <= 0x34; k += 4)   // (+ 0x28 the room's sound script: its song, its wind)
    {
        u32 id = Be32(floor + k);
        struct Thread *t;
        if (k == 0x30)
            Orre_Npcs_Place();   // (Colosseum's order: + 0x2C, the people, + 0x30)
        if (id == 0)
            continue;
        if ((t = Start(id, 0, NULL, TRUE)) != NULL && k < 0x34)
            Run(t);   // (the set-up runs through at once)
    }
    MPNet_Log("orre vm: room %u (%s): main %08X", room, file, Be32(floor + 0x34));
}

// (special, waitstate) Orre_EventScript_VmHold: the field stays locked while the scene that took the player runs;
// VAR_RESULT 1 when it asks for a battle (VAR_0x8004), 0 when it lets go
void Orre_VmHold(void)
{
    if (sHolder == NULL)
    {
        gSpecialVar_Result = 0;
        ScriptContext_Enable();
        return;
    }
    sHolding = TRUE;
}

// (special) Orre_EventScript_Talk: the person VAR_0x8004 was talked to: their talk script runs as the scene that holds
// the field (Colosseum's peopleStartScript: its arguments the group, the person, 0, 0)
void Orre_VmTalk(void)
{
    u32 args[4] = {1, gSpecialVar_0x8004, 0, 0};
    u32 id = Orre_Npc_TalkScriptId(gSpecialVar_0x8004);
    struct Thread *t;
    if (id == 0 || sHolder != NULL || (t = Start(id, 4, args, FALSE)) == NULL)
    {
        MPNet_Log("orre vm: talk to person %u: not now (%08X, %s)", gSpecialVar_0x8004, id,
                  sHolder ? "a scene holds the field" : "no script");
        return;
    }
    t->holds = TRUE;
    sHolder = t;
    MPNet_Log("orre vm: talk to person %u: %08X", gSpecialVar_0x8004, id);
}

// (specialvar) Orre_EventScript_VmHoldLoop: the Kai screen the scene asks for (SCREEN_*: 1 the shop, 2 the PC), once
u16 Orre_VmScreen(void)
{
    u16 s = sScreen;
    sScreen = SCREEN_NONE;
    return s;
}

// (special, waitstate) SCREEN_WARP: floorLink's warp (the scripts stop and the next room's start: Orre_Vm_Warped)
void Orre_VmWarp(void)
{
    SetWarpDestination(sWarpMap >> 8, sWarpMap & 0xFF, WARP_ID_NONE, sWarpX, sWarpY);
    DoWarp();
    ResetInitialPlayerAvatarState();
}

// (special, waitstate) SCREEN_NAME: the partner's name on Kai's naming screen (left empty: RUI, Colosseum's own)
static u8 sPartnerNameBuf[PLAYER_NAME_LENGTH + 1];

static void CB2_PartnerNamed(void)
{
    Orre_SetPartnerName(sPartnerNameBuf);
    SetMainCallback2(CB2_ReturnToFieldContinueScript);
}

static void Task_PartnerName(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        DestroyTask(taskId);
        NamingScreen_SetOrreTitle(Orre_Text(15688));   // ("Her name is...")
        *StringCopyN(sPartnerNameBuf, Orre_PartnerName(), PLAYER_NAME_LENGTH) = EOS;
        DoNamingScreen(NAMING_SCREEN_ORRE_PARTNER, sPartnerNameBuf, 0, 0, 0, CB2_PartnerNamed);
    }
}

void Orre_VmName(void)
{
    BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
    CreateTask(Task_PartnerName, 10);
}

// (special, waitstate) SCREEN_CHOICE: the choices as Kai's menu (B: the last, as Colosseum's cancel row is)
void Orre_VmChoice(void)
{
    static struct MenuAction sActions[8];
    struct Thread *t = sScreenThread;
    u32 i, n = t ? t->nchoices : 0;
    for (i = 0; i < n; i++)
        sActions[i].text = Orre_Text(t->choices[i]), sActions[i].func.void_u8 = NULL;
    if (n == 0 || !ScriptMenu_MultichoiceList(1, 1, sActions, n, FALSE))
    {
        gSpecialVar_Result = 0;
        ScriptContext_Enable();
    }
}

// (special) ... its row for the thread (B: the last row)
void Orre_VmChoiceDone(void)
{
    struct Thread *t = sScreenThread;
    if (t && t->state == T_RUN && t->wait == W_SCREEN && t->screen == SCREEN_CHOICE)
        t->result = gSpecialVar_Result == MULTI_B_PRESSED ? (t->nchoices ? t->nchoices - 1 : 0) : gSpecialVar_Result;
}

// (special, waitstate) Colosseum's shop sShop as Kai's POKeMART (it lets the script go on when it closes)
void Orre_VmShop(void)
{
    CreatePokemartMenu(sOrreShops[sShop < ORRE_SHOP_COUNT ? sShop : 0].items);
}

// (special) Orre_EventScript_Check: the player checked the spot VAR_0x8004 (a row of src/data/orre_room_checks.h,
// orre_npcs.c): its script runs with its four arguments as the scene holding the field (floor_character.c
// heroDispatchEvent, cmsRunSync)
void Orre_VmTouch(void)
{
    u32 args[4], id = Orre_Touch_Script(gSpecialVar_0x8004, args);
    struct Thread *t;
    if (id == 0 || sHolder != NULL || (t = Start(id, 4, args, FALSE)) == NULL)
    {
        MPNet_Log("orre vm: talk to person %u: not now (%08X, %s)", gSpecialVar_0x8004, id,
                  sHolder ? "a scene holds the field" : "no script");
        return;
    }
    t->holds = TRUE;
    sHolder = t;
    MPNet_Log("orre vm: touch %u: %08X", gSpecialVar_0x8004, id);
}

void Orre_VmCheck(void)
{
    u32 args[4], id = Orre_Check_Script(gSpecialVar_0x8004, args);
    struct Thread *t;
    if (id == 0 || sHolder != NULL || (t = Start(id, 4, args, FALSE)) == NULL)
    {
        MPNet_Log("orre vm: talk to person %u: not now (%08X, %s)", gSpecialVar_0x8004, id,
                  sHolder ? "a scene holds the field" : "no script");
        return;
    }
    t->holds = TRUE;
    sHolder = t;
    MPNet_Log("orre vm: check %u: %08X", gSpecialVar_0x8004, id);
}

bool8 Orre_Vm_Running(void)
{
    return sRoom != 0;
}

// (specialvar) a scene still holds the player
u16 Orre_VmHolding(void)
{
    return sHolder != NULL;
}

// The room's sound script again (its floor record's + 0x28), as Colosseum runs it after a battle
void Orre_Vm_RoomSound(void)
{
    u16 room = 0;
    const u8 *floor;
    struct Thread *t;
    if (O3D_RoomFile(&room) == NULL || (floor = FloorOf(room)) == NULL || Be32(floor + 0x28) == 0)
        return;
    if ((t = Start(Be32(floor + 0x28), 0, NULL, TRUE)) != NULL)
        Run(t);
}

// After the battle the hold script started: the thread goes on with its result
void Orre_VmBattleDone(void)
{
    MPNet_Log("orre vm: battle done (%s, result %u)", sHolder ? "the scene goes on" : "no scene", Orre_BattleResult());
    if (sHolder && sHolder->wait == W_BATTLE)
        Resume(sHolder, Orre_BattleResult());
}

// Tests and debugging: run function FUNC of bank BANK (0x100 the room's, 0x596 the shared one) as a scene
bool8 Orre_Vm_Debug(u16 bank, u16 func, const u32 *args)
{
    struct Thread *t;
    u16 room = 0;
    const char *file = O3D_RoomFile(&room);
    if (file == NULL)
        return FALSE;
    if (sRoom != room)
    {
        Orre_Vm_Stop();
        sRoom = room;
        LoadRoomBank(file);
    }
    {
        // (alone: the room's own threads stop, so its main loop doesn't start the same scene)
        u32 i;
        for (i = 0; i < VM_THREADS; i++)
            if (sThreads[i].state == T_RUN && sThreads[i].room && &sThreads[i] != sHolder)
                sThreads[i].state = T_FREE;
        if (sHolder != NULL)
            return FALSE;   // (a scene of the room's own holds the player: it runs, not a second one)
    }
    {
        static const u32 sNone[4] = {0, 0, 0, 0};   // (four as every scene gets: its return value has its slot)
        t = Start((u32)bank << 16 | func, 4, args ? args : sNone, FALSE);
    }
    if (t)
        t->debug = TRUE;
    return t != NULL;
}

// battle_setup.c CB2_EndOrreBattle: a battle lost where Colosseum's game ends (its zenmetu flag) whites out: the story
// goes back to before the scene (Colosseum goes back to the save), the whiteout to the last POKeMON CENTER healed at in
// ORRE (Orre_Vm's N_POKEMON_CENTER), else the OUTSKIRT STAND where the boat lands
void Orre_Vm_LossWhiteOut(void)
{
    sLossPending = TRUE;
    if (!O3D_IsOrreGroup(gSaveBlock1Ptr->lastHealLocation.mapGroup))
    {
        s16 x = 33, y = 40;
        Orre_MapStart(MAP_ORRE_OUTSKIRT_STAND, &x, &y);
        HealAt( MAP_GROUP(MAP_ORRE_OUTSKIRT_STAND),
                    MAP_NUM(MAP_ORRE_OUTSKIRT_STAND), WARP_ID_NONE, x, y);
    }
}
