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
#include "o3d.h"
#include "orre.h"
#include "orre_data.h"
#include "palette.h"
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
enum { W_NONE, W_FRAMES, W_MESSAGE, W_MESSAGE_KEY, W_YESNO, W_BATTLE, W_MOVE, W_SCREEN, W_FANFARE, W_CAMERA };

// Kai's screens a scene opens (W_SCREEN): Orre_EventScript_VmHoldLoop goes to them while the scene waits
enum { SCREEN_NONE, SCREEN_SHOP, SCREEN_PC };

// Colosseum's shops (menuShopOpen N): tools/orre/gen_shops.py
struct OrreShop
{
    u8 kind;
    u16 items[20];
};
#include "../data/orre_shops.h"

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
    bool8 screenOpen;     // W_SCREEN: the hold script went to it
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
    if ((sRel = OrreData_Read("common/common_rel.rel", &size)) == NULL || size < 0x40)
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

static void Finish(struct Thread *t)
{
    t->state = T_DONE;
    if (t->holds && sHolder == t)
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
    N_CAMERA_WAIT = 46, N_CAMERA_RETURN = 179,
};

#define ROOM_PCBOX_MENU 913    // (Colosseum's menu rooms: the PC's boxes)
#define LAST_COLO_ITEM 348     // (Colosseum's items to here are Gen 3's numbers, Kai's own)

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

static void ShowMessage(struct Thread *t, u32 msg, bool8 key)
{
    static u8 sPlaceholder[6];
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
    if (sHoldPending && sHolder && !sHolding && !ScriptContext_IsEnabled())
    {
        sHoldPending = FALSE;
        ScriptContext_SetupScript(Orre_EventScript_VmHold);
        MPNet_Log("orre vm: %08X takes the player", sHolder->scriptId);
    }
}

static void Hold(struct Thread *t)
{
    t->holds = TRUE;
    if (sHolder == NULL)
    {
        sHolder = t;
        sHoldPending = TRUE;
        TryHold();
    }
}

static void LetGo(struct Thread *t)
{
    t->holds = FALSE;
    if (sHolder == t)
    {
        sHolder = NULL;
        if (sHolding)
        {
            sHolding = FALSE;
            gSpecialVar_Result = 0;
            ScriptContext_Enable();
        }
    }
}

// Call native N with ARGS (A[0] the first): its result; the thread may be left waiting
static u32 Native(struct Thread *t, u32 n, const V *a, s32 count)
{
    V z = {0};
    static s8 sTrace = -1;
    if (sTrace < 0)
        sTrace = getenv("MOEMON_ORRE_VM_TRACE") != NULL;
    if (sTrace && n != N_HERO_MAIN)
        MPNet_Log("orre vm: %08X native %u (%d args: %u %u %u)", t->scriptId, n, count, count > 0 ? a[0].u : 0,
                  count > 1 ? a[1].u : 0, count > 2 ? a[2].u : 0);
#define ARG(i) ((i) < count ? a[i] : z)
    switch (n)
    {
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
    case N_SLEEP:
        t->frames = (s32)ARG(0).f;
        t->wait = W_FRAMES;
        return 0;
    case N_HERO_INIT_EVENT: Hold(t); return 0;
    case N_HERO_TERM_EVENT: LetGo(t); return 0;
    case N_HERO_MAIN: return 0;          // (Kai's field moves the player)
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
        MPNet_Log("orre vm: party healed");
        return 0;
    case N_CAMERA_PLAY:   // cameraPlayAnime(group, shot, frame, loop): Colosseum's shot in ORRE's view (o3d_cam.c)
        O3D_CamPlayId(ARG(1).u, (float)ARG(2).s, ARG(3).u != 0);
        return 0;
    case N_CAMERA_STOP: case N_CAMERA_STOP2: case N_CAMERA_RETURN:   // (back to the field's camera)
        O3D_CamStop();
        return 0;
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
        return KaiItem(ARG(0).u) && CheckBagHasItem(KaiItem(ARG(0).u), 1);
    case N_ADD_ITEM:
        if (!KaiItem(ARG(0).u))
            return 0;
        if (ARG(1).s > 0)
            return AddBagItem(KaiItem(ARG(0).u), ARG(1).s);
        if (ARG(1).s < 0)
            return RemoveBagItem(KaiItem(ARG(0).u), -ARG(1).s);
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
    case N_FADE_SET: case N_FADE_CHECK: return 0;
    case N_PEOPLE_WAIT_SYNC: case N_CAMERA_END_CHECK: case N_CHAR_SET_ANIM: return 1;
    // ---- the room's people (orre_npcs.c)
    case N_PEOPLE_VISIBLE: Orre_Npc_SetShown(ARG(1).u, ARG(2).u != 0); return 0;
    case N_FLOOR_CHAR_DISP: Orre_Npc_SetDisp(ARG(0).u, ARG(1).u != 0); return 0;
    case N_PEOPLE_OPEN: Orre_Npc_SetShown(ARG(1).u, TRUE); return 0;
    case N_PEOPLE_CLOSE: Orre_Npc_SetShown(ARG(1).u, FALSE); return 0;
    case N_SET_POSITION: Orre_Npc_SetPosition(ARG(0).u, ARG(1).u, ARG(2).f, ARG(4).f); return 0;
    case N_SET_ROTATION: Orre_Npc_SetAngle(ARG(0).u, ARG(1).u, ARG(3).f); return 0;
    case N_TURN_TO_ANGLE: Orre_Npc_SetAngle(ARG(0).u, ARG(1).u, ARG(2).f); return 0;
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
        if (!IsTextPrinterActive(0))
            t->wait = W_NONE;
        break;
    case W_MESSAGE_KEY:
        if (!IsTextPrinterActive(0) && JOY_NEW(A_BUTTON | B_BUTTON))
        {
            PlaySE(SE_SELECT);
            t->wait = W_NONE;
        }
        break;
    case W_YESNO:
    {
        s8 c = Menu_ProcessInputNoWrapClearOnChoose();
        if (c != MENU_NOTHING_CHOSEN)
            Resume(t, c == 0 ? 0 : 1);
        break;
    }
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
            sScreen = t->screen, sShop = t->shop;
            t->screenOpen = TRUE;
            sHolding = FALSE;
            ScriptContext_Enable();
        }
        else if (t->screenOpen && sHolding)
            t->wait = W_NONE;
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
    if (O3D_RoomFile(&room) == NULL)
    {
        if (sRoom)
            Orre_Vm_Stop();
        return;
    }
    if (room != sRoom)
        Orre_Vm_RoomEntered();
    if (getenv("MOEMON_ORRE_VM_TRACE"))
    {
        static u32 sTick;
        if (++sTick % 120 == 0)
            for (i = 0; i < VM_THREADS; i++)
                if (sThreads[i].state == T_RUN)
                    MPNet_Log("orre vm: thread %u %08X wait %u holds %u (holding %u, printer %u, box hidden %u, fade %u, script %u)", i,
                              sThreads[i].scriptId, sThreads[i].wait, sThreads[i].holds, sHolding,
                              IsTextPrinterActive(0), IsFieldMessageBoxHidden(), gPaletteFade.active, ScriptContext_IsEnabled());
    }
    if (gPaletteFade.active)
        return;   // (the field comes back: the scripts wait until it's there, as Colosseum's fades do)
    TryHold();
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
    }
}

// A warp (overworld.c LoadMapFromWarp): the next frame on an ORRE map enters its room anew, even the same room
// (a return from a battle isn't a warp: the scene that started it goes on)
void Orre_Vm_Warped(void)
{
    if (sRoom)
        MPNet_Log("orre vm: warped: room %u's scripts stop", sRoom);
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
void Orre_Vm_RoomEntered(void)
{
    u16 room = 0;
    const char *file = O3D_RoomFile(&room);
    const u8 *floor;
    u32 k;
    Orre_Vm_Stop();
    // (off in players' games until the rooms' people walk and the cameras move: ORRE_HANDOFF step 5; on for tests and
    // development with MOEMON_ORRE_SCRIPTS=1)
    if (file == NULL || !getenv("MOEMON_ORRE_SCRIPTS"))
        return;
    sRoom = room;
    LoadRoomBank(file);
    Orre_Npcs_RoomEntered(room);
    if ((floor = FloorOf(room)) == NULL || sRoomBank.d == NULL)
        return;
    for (k = 0x2C; k <= 0x34; k += 4)
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
        return;
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

// (special, waitstate) Colosseum's shop sShop as Kai's POKeMART (it lets the script go on when it closes)
void Orre_VmShop(void)
{
    CreatePokemartMenu(sOrreShops[sShop < ORRE_SHOP_COUNT ? sShop : 0].items);
}

// (special) Orre_EventScript_Check: the player checked the spot VAR_0x8004 (a row of src/data/orre_room_checks.h,
// orre_npcs.c): its script runs with its four arguments as the scene holding the field (floor_character.c
// heroDispatchEvent, cmsRunSync)
void Orre_VmCheck(void)
{
    u32 args[4], id = Orre_Check_Script(gSpecialVar_0x8004, args);
    struct Thread *t;
    if (id == 0 || sHolder != NULL || (t = Start(id, 4, args, FALSE)) == NULL)
        return;
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

// After the battle the hold script started: the thread goes on with its result
void Orre_VmBattleDone(void)
{
    MPNet_Log("orre vm: battle done (%s, result %u)", sHolder ? "the scene goes on" : "no scene", Orre_BattleResult());
    if (sHolder && sHolder->wait == W_BATTLE)
        Resume(sHolder, Orre_BattleResult());
}

// Tests and debugging: run function FUNC of bank BANK (0x100 the room's, 0x596 the shared one) as a scene
bool8 Orre_Vm_Debug(u16 bank, u16 func)
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
    t = Start((u32)bank << 16 | func, 0, NULL, FALSE);
    return t != NULL;
}
