// Moemon Kai: the ENTRALINK's missions and PASS POWERS (the user's approved mock-ups: docs/unova/mockups/
// entralink_missions.png, pass_powers.png), co-op friends in place of Black's wireless (docs/UNOVA_DECISIONS.md).
// Every word on these screens is Black's own (system banks 45, 67, 69, 160: tools/unova/text.py); the PASS POWERS are
// Black's table (a/0/3/8, gpower.c: a level to receive one, its cost in Pass Orbs, its value, its kind and minutes;
// one active a kind, the encounter and EXP kinds each other's opposite) and its effects (gpower.c GPOWER_Apply*),
// hooked where the game counts encounters, egg steps, friendship, prices, EXP, prize money and catches.
// The ENTREE (Black's menu, bank 69): ACCEPT A MISSION (one for the room, of Black's own kinds: SUPPORT, talk to a
// friend and give them a power; BATTLE, challenge a friend; Black's words for it, its time limit and its Pass Orbs),
// RECEIVE PASS POWER (the Black Lv. its table asks, its Pass Orbs), CHECK STATUS. A received power is used from the
// C-GEAR (u3d_screen.c), for the whole room if SHARE is on (Black's "received Pass Power from <name>'s Entralink").
// Kept in UNOVA's saved work (u3d_vm.c U3D_Vm_EntralinkSave).
#include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>
#include "global.h"
#include "main.h"
#include "pokemon.h"
#include "random.h"
#include "sound.h"
#include "string_util.h"
#include "script.h"
#include "script_pokemon_util.h"
#include "field_player_avatar.h"
#include "constants/songs.h"
#include "constants/characters.h"
#include "mp.h"
#include "u3d.h"
#include "u3d_screen.h"
#include "../tcg/tcg_gfx.h"

#define POWERS 48
#define KINDS 13
#define NO_POWER 0x30
enum { EL_MISSION_START, EL_MISSION_DONE, EL_POWER_SHARE };
enum { PK_ENC_UP, PK_ENC_DOWN, PK_HATCH, PK_FRIEND, PK_BARGAIN, PK_HP, PK_PP, PK_EXP_UP, PK_EXP_DOWN, PK_MONEY,
       PK_CAPTURE };

void MP_GameToAscii(char *dst, const u8 *src, int maxLen);
void MP_AsciiToGame(u8 *dst, const char *src, int maxLen);
const u16 *U3D_Vm_GPower(u8 id);
struct UnovaEntralinkSave *U3D_Vm_EntralinkSave(void);

struct UnovaEntralinkSave
{
    u16 orbs, missions;
    u8 owned[6];       // (a bit a PASS POWER received)
    u8 shareOff;
    u8 activeId[KINDS];
    u32 framesLeft[KINDS];
};

STATIC_ASSERT(sizeof(struct UnovaEntralinkSave) <= 96, EntralinkSaveFits);   // (u3d_vm.c sWork.entralink)

static struct UnovaEntralinkSave *Save(void)
{
    struct UnovaEntralinkSave *s = U3D_Vm_EntralinkSave();
    static bool8 sInit;
    if (!sInit)
    {
        // (an old save's zeros: no power active is NO_POWER, not power 0)
        u32 k;
        bool8 fresh = TRUE;
        for (k = 0; k < KINDS; k++)
            if (s->activeId[k] || s->framesLeft[k])
                fresh = FALSE;
        if (fresh)
            for (k = 0; k < KINDS; k++)
                s->activeId[k] = NO_POWER;
        sInit = TRUE;
    }
    return s;
}

// The Black Lv. (Black's ENTREE level): one, and one more a mission completed, to Black's highest need, 30
static u8 BlackLevel(void)
{
    u32 lv = 1 + Save()->missions;
    return lv > 30 ? 30 : lv;
}

static bool8 Owned(u8 id)
{
    return id < POWERS && (Save()->owned[id / 8] >> (id % 8)) & 1;
}

// ------------------------------------------------------------------------------------------------- Black's words
// A line of Black's system bank BANK, its word slots filled first, as text the screens draw (UTF-8: é; line breaks
// and Black's waits as spaces, the screens wrap)
static void BlackLine(u16 bank, u16 msg, char *out, int size)
{
    static u8 buf[512];
    char *o = out;
    const u8 *s;
    StringExpandPlaceholders(buf, U3D_Text(bank, msg));
    for (s = buf; *s != EOS && o < out + size - 3; s++)
    {
        if (*s == CHAR_e_ACUTE)
            *o++ = (char)0xC3, *o++ = (char)0xA9;
        else if (*s == CHAR_NEWLINE || *s == CHAR_PROMPT_SCROLL || *s == CHAR_PROMPT_CLEAR)
        {
            if (o > out && o[-1] != ' ')
                *o++ = ' ';
        }
        else
        {
            u8 one[2] = {*s, EOS};
            char c[2];
            MP_GameToAscii(c, one, 1);
            *o++ = c[0] == '~' ? 'e' : c[0];
        }
    }
    while (o > out && o[-1] == ' ')
        o--;
    *o = 0;
    {
        char *r = out, *w = out;   // (Black's diamond marks, ◇ / ◆, have no letter here: left off)
        for (; *r; r++)
            if (!(*r == '?' && (r == out || r[-1] == ' ')))   // a word's first letter: the mark; a word's last: a question
                *w++ = *r;
        for (r = out; *r == ' '; r++)
            ;
        if (r != out)
            memmove(out, r, strlen(r) + 1);
        *w = 0;
    }
}

static void Slot(u8 slot, const char *ascii)
{
    MP_AsciiToGame(gUnovaStrVar[slot % UNOVA_STR_VARS], ascii, 60);
}

static void SlotNum(u8 slot, u32 n)
{
    ConvertIntToDecimalStringN(gUnovaStrVar[slot % UNOVA_STR_VARS], n, STR_CONV_MODE_LEFT_ALIGN, 5);
}

// A PASS POWER's name (bank 160), Black's arrows left off: the screens draw them as dots
static void PowerName(u8 id, char *out, int size)
{
    int n;
    BlackLine(160, id, out, size);
    for (n = strlen(out); n > 0 && (out[n - 1] == ' ' || out[n - 1] == '?'); n--)
        out[n - 1] = 0;
}

// How strong a power is: 1-3 (Black's arrows), 0 for S / MAX / FULL RECOVERY
static u8 PowerStrength(u8 id)
{
    return id < 33 ? id % 3 + 1 : 0;
}

static bool8 PowerDown(u8 id)
{
    u8 kind = U3D_Vm_GPower(id)[4];
    return kind == PK_ENC_DOWN || kind == PK_EXP_DOWN;
}

// ------------------------------------------------------------------------------------------------- PASS POWERS
// gpower.c GPOWER_Activate: one a kind, for its minutes; ENCOUNTER / EXP up and down each other's opposite. HP / PP
// restoring powers work at once (GPOWER_UseHpRestore / PpRestore)
static void Activate(u8 id)
{
    const u16 *p = U3D_Vm_GPower(id);
    struct UnovaEntralinkSave *s = Save();
    u8 kind;
    if (id >= POWERS || !p)
        return;
    kind = p[4];
    if (kind == PK_HP)
    {
        if (p[3] == 999)
            HealPlayerParty();
        else
        {
            u32 i;
            for (i = 0; i < PARTY_SIZE; i++)
            {
                struct Pokemon *mon = &gPlayerParty[i];
                u16 hp = GetMonData(mon, MON_DATA_HP), max = GetMonData(mon, MON_DATA_MAX_HP);
                if (GetMonData(mon, MON_DATA_SPECIES) && !GetMonData(mon, MON_DATA_IS_EGG) && hp)
                {
                    hp = hp + p[3] > max ? max : hp + p[3];
                    SetMonData(mon, MON_DATA_HP, &hp);
                    break;
                }
            }
        }
        return;
    }
    if (kind == PK_PP)
    {
        u32 i, m;
        for (i = 0; i < PARTY_SIZE; i++)
        {
            struct Pokemon *mon = &gPlayerParty[i];
            if (GetMonData(mon, MON_DATA_SPECIES) && !GetMonData(mon, MON_DATA_IS_EGG) && GetMonData(mon, MON_DATA_HP))
            {
                for (m = 0; m < MAX_MON_MOVES; m++)
                {
                    u16 move = GetMonData(mon, MON_DATA_MOVE1 + m);
                    u8 pp = GetMonData(mon, MON_DATA_PP1 + m), max;
                    if (!move)
                        continue;
                    max = CalculatePPWithBonus(move, GetMonData(mon, MON_DATA_PP_BONUSES), m);
                    pp = pp + p[3] > max ? max : pp + p[3];
                    SetMonData(mon, MON_DATA_PP1 + m, &pp);
                }
                break;
            }
        }
        return;
    }
    if (kind >= KINDS)
        return;
    s->activeId[kind] = id;
    s->framesLeft[kind] = (u32)p[5] * 60 * 60;   // (Black's minutes; the game's 60 frames a second)
    if (kind == PK_ENC_UP || kind == PK_ENC_DOWN)
        s->activeId[kind ^ 1] = NO_POWER, s->framesLeft[kind ^ 1] = 0;
    if (kind == PK_EXP_UP || kind == PK_EXP_DOWN)
        s->activeId[kind == PK_EXP_UP ? PK_EXP_DOWN : PK_EXP_UP] = NO_POWER,
        s->framesLeft[kind == PK_EXP_UP ? PK_EXP_DOWN : PK_EXP_UP] = 0;
    printf("[test] pass power %u on (kind %u, value %u, %u min)\n", id, kind, p[3], p[5]);
}

static u16 Value(u8 kind)
{
    struct UnovaEntralinkSave *s = Save();
    if (kind >= KINDS || s->activeId[kind] >= POWERS || !s->framesLeft[kind])
        return 0;
    return U3D_Vm_GPower(s->activeId[kind])[3];
}

// The game's counts through the PASS POWER on (gpower.c GPOWER_Apply*): KIND as Black's
u32 Unova_PassPower_Apply(u8 kind, u32 v)
{
    u16 x;
    switch (kind)
    {
    case PK_ENC_UP:   // (an encounter rate: up or down, the active one)
        if ((x = Value(PK_ENC_UP)) || (x = Value(PK_ENC_DOWN)))
            return v * x / 100;
        return v;
    case PK_HATCH:
    case PK_BARGAIN:
    case PK_MONEY:
    case PK_CAPTURE:
        return (x = Value(kind)) ? v * x / 100 : v;
    case PK_FRIEND:   // (a gain only)
        return v + Value(PK_FRIEND);
    case PK_EXP_UP:
        if ((x = Value(PK_EXP_UP)) || (x = Value(PK_EXP_DOWN)))
            return v * x / 100;
        return v;
    }
    return v;
}

// Egg steps through HATCHING POWER: each step counts for VALUE / 100 (the remainder kept)
u32 Unova_PassPower_EggSteps(void)
{
    static u32 sFrac;
    u16 x = Value(PK_HATCH);
    if (!x)
        return 1;
    sFrac += x;
    x = sFrac / 100;
    sFrac %= 100;
    return x;
}

// ------------------------------------------------------------------------------------------------- missions
enum { MK_SUPPORT, MK_BATTLE };
struct Mission
{
    u8 kind, target, power, orbs;   // (target: the friend's player id; power: the S / MAX power given)
    u16 text, seconds;
};
#define MISSION_CHOICES 5
static struct
{
    struct Mission choice[MISSION_CHOICES];
    u8 count;
    struct Mission now;   // (the room's mission while it runs)
    bool8 running;
    u8 starter;
    u32 framesLeft;
} M;

// Black's SUPPORT mission words (bank 67 #21-33, "Talk to <friend> and give <power>!") and the power each names
static const u8 sSupportPower[13] = {45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33};

static int Friends(u8 *ids)
{
    int i, n = 0;
    if (!MP_IsConnected())
        return 0;
    for (i = 0; i < MP_MAX_PLAYERS; i++)
        if (i != gMpLocalId && gMpPlayers[i].present)
            ids[n++] = i;
    return n;
}

static void MakeChoices(void)
{
    u8 ids[MP_MAX_PLAYERS];
    int n = Friends(ids), i;
    M.count = 0;
    if (!n)
        return;
    for (i = 0; i < MISSION_CHOICES; i++)
    {
        struct Mission *m = &M.choice[M.count++];
        m->target = ids[Random() % n];
        if (i % 3 == 2)
            m->kind = MK_BATTLE, m->text = 34 + Random() % 8, m->seconds = 240, m->orbs = 3, m->power = 0;
        else
        {
            u8 k = Random() % 13;
            m->kind = MK_SUPPORT, m->text = 21 + k, m->power = sSupportPower[k], m->seconds = 180, m->orbs = 2;
        }
    }
}

static void FriendName(u8 id, char *out, int n)
{
    MP_GameToAscii(out, gMpPlayers[id].name, n);
}

// Black's words for a mission, its slots filled: 0 the friend, 1 the power, 2 the level / seconds, 3 the orbs
static void MissionText(const struct Mission *m, char *out, int size)
{
    char name[16], power[40];
    FriendName(m->target, name, sizeof(name));
    Slot(0, name);
    PowerName(m->power, power, sizeof(power));
    Slot(1, power);
    SlotNum(2, m->seconds);
    SlotNum(3, m->orbs);
    BlackLine(67, m->text, out, size);
}

static void Notice(u16 bank, u16 msg)
{
    char line[160];
    BlackLine(bank, msg, line, sizeof(line));
    MP_ShowNotice(line);
}

static void SendMission(void)
{
    u8 b[12];
    b[0] = MPR_ENTRALINK, b[1] = EL_MISSION_START, b[2] = M.now.kind, b[3] = M.now.target, b[4] = M.now.power;
    b[5] = M.now.orbs, b[6] = M.now.text & 0xFF, b[7] = M.now.text >> 8, b[8] = M.now.seconds & 0xFF,
    b[9] = M.now.seconds >> 8;
    MP_SendRelay(0xFF, b, 10);
}

static void StartMission(const struct Mission *m, u8 starter)
{
    M.now = *m;
    M.running = TRUE;
    M.starter = starter;
    M.framesLeft = (u32)m->seconds * 60;
    printf("[test] entralink mission %u (kind %u) for player %u, %u s\n", m->text, m->kind, m->target + 1, m->seconds);
    Notice(69, 16);   // "The mission is starting now!"
}

// The mission done (here or by a friend): everyone in it gets its Pass Orbs and a mission more
static void EndMission(bool8 done)
{
    struct UnovaEntralinkSave *s = Save();
    if (!M.running)
        return;
    M.running = FALSE;
    if (done)
    {
        s->orbs += M.now.orbs;
        if (s->missions < 0xFFFF)
            s->missions++;
        PlaySE(SE_SUCCESS);
        Notice(45, 36);   // "The mission is complete."
    }
    else
    {
        PlaySE(SE_FAILURE);
        Notice(45, 37);   // "The mission failed..."
    }
    printf("[test] entralink mission %s (orbs %u, missions %u)\n", done ? "complete" : "failed", s->orbs, s->missions);
}

static void SendDone(void)
{
    u8 b[3] = {MPR_ENTRALINK, EL_MISSION_DONE, 1};
    MP_SendRelay(0xFF, b, 3);
}

// mp_main.c: a friend's mission or power
void Unova_Entralink_OnRelay(u8 from, const u8 *data, u16 len)
{
    if (len < 2)
        return;
    switch (data[1])
    {
    case EL_MISSION_START:
        if (len >= 10 && !M.running)
        {
            struct Mission m = {data[2], data[3], data[4], data[5], data[6] | data[7] << 8, data[8] | data[9] << 8};
            StartMission(&m, from);
        }
        break;
    case EL_MISSION_DONE:
        EndMission(TRUE);
        break;
    case EL_POWER_SHARE:
        if (len >= 3 && data[2] < POWERS)
        {
            char me[16], friendName[16];
            Activate(data[2]);
            MP_GameToAscii(me, gSaveBlock2Ptr->playerName, 8);
            FriendName(from, friendName, sizeof(friendName));
            Slot(0, me);
            Slot(1, friendName);
            Notice(45, 38);   // "<me> received Pass Power from <friend>'s Entralink."
        }
        break;
    }
}

// The player talking to (facing, A) the friend a SUPPORT mission names: done, and the friend gets its power
static s8 FriendInFront(void)
{
    s16 x, y, rx, ry;
    int i;
    GetXYCoordsOneStepInFrontOfPlayer(&x, &y);
    for (i = 0; i < MP_MAX_PLAYERS; i++)
        if (i != gMpLocalId && gMpPlayers[i].present && MP_GetRemoteMapPos(i, &rx, &ry) && rx == x && ry == y)
            return i;
    return -1;
}

// mp_battle.c MP_Battle_Start: a PvP battle with the friend a BATTLE mission names completes it
void Unova_Entralink_OnPvpBattle(u8 opponent)
{
    if (M.running && M.now.kind == MK_BATTLE && opponent == M.now.target)
    {
        SendDone();
        EndMission(TRUE);
    }
}

// Each field frame (u3d_screen.c): the powers' minutes, the mission's time and its talk
void Unova_Entralink_FieldFrame(void)
{
    struct UnovaEntralinkSave *s = Save();
    static bool8 sFreeBefore;
    bool8 freeNow = !ArePlayerFieldControlsLocked() && !Unova_Screen_IsActive();
    u32 k;
    for (k = 0; k < KINDS; k++)
        if (s->framesLeft[k] && --s->framesLeft[k] == 0)
            s->activeId[k] = NO_POWER;
    if (!M.running || (M.framesLeft && --M.framesLeft == 0))
    {
        if (M.running)
            EndMission(FALSE);
        sFreeBefore = freeNow;
        return;
    }
    // (free on the frame before: the field's own A on a friend locks the controls before this hook runs)
    if (M.now.kind == MK_SUPPORT && JOY_NEW(A_BUTTON) && sFreeBefore && FriendInFront() == M.now.target)
    {
        u8 b[3] = {MPR_ENTRALINK, EL_POWER_SHARE, M.now.power};
        MP_SendRelay(M.now.target, b, 3);   // (the friend talked to gets the power)
        SendDone();
        EndMission(TRUE);
    }
    sFreeBefore = freeNow;
}

bool8 Unova_Entralink_MissionRunning(void)
{
    return M.running;
}

// ------------------------------------------------------------------------------------------------- the screens
enum { ES_MENU, ES_MISSIONS, ES_MISSION_ASK, ES_RECEIVE, ES_RECEIVE_ASK, ES_STATUS, ES_POWERS, ES_POWER_ASK, ES_NOTE };
static struct
{
    u8 page, back, sel, top, ask;
    u8 list[POWERS], count;
    char note[2][160];
} E;

#define HEAD 0x407848
#define HEAD_SH 0x204828
#define BG_TOP 0xd8e8c8
#define DOT_ON 0x58b048
#define DOT_DOWN 0x5878c8
#define DOT_OFF 0xc8d0c0

static void Background(void)
{
    int y;
    for (y = 0; y < 160; y += 4)
        G_Fill(0, y, 240, 4, G_Mix(0xe8f0d8, 0xc8d8b8, y / 160.0f));
    G_Fill(0, 0, 240, 16, HEAD);
}

static void Header(const char *left, const char *right)
{
    G_Text(6, 0, G_WHITE, HEAD_SH, left, 160, TF_NORMAL);
    if (right)
        G_TextAlign(234, 2, TA_RIGHT, G_WHITE, HEAD_SH, right, TF_SMALL);
}

static void Dots(int x, int y, u8 id)
{
    int k, n = PowerStrength(id);
    for (k = 0; k < 3; k++)
        G_Circle(x + 4 + k * 12, y + 4, 3.5f, k < n ? (PowerDown(id) ? DOT_DOWN : DOT_ON) : DOT_OFF, 255);
}

static void Note(const char *a, const char *b)
{
    snprintf(E.note[0], sizeof(E.note[0]), "%s", a);
    snprintf(E.note[1], sizeof(E.note[1]), "%s", b ? b : "");
    E.back = E.page;
    E.page = ES_NOTE;
}

static void NoteBlack(u16 bank, u16 msg)
{
    char line[160];
    BlackLine(bank, msg, line, sizeof(line));
    Note(line, NULL);
}

static void BuildReceivable(void)
{
    u8 i;
    E.count = 0;
    for (i = 0; i < POWERS; i++)
    {
        const u16 *p = U3D_Vm_GPower(i);
        if (p[0] && p[0] < 0xFE && !Owned(i))
            E.list[E.count++] = i;
    }
}

static void BuildOwned(void)
{
    u8 i;
    E.count = 0;
    for (i = 0; i < POWERS; i++)
        if (Owned(i))
            E.list[E.count++] = i;
}

static void BigText(const char *s)
{
    G_Window(0, 40, 240, 120);
    G_Text(10, 48, G_INK, G_INK_SHADOW, s, 220, TF_NORMAL);
}

static void DrawMenu(void)
{
    char title[48], items[3][40];
    const char *list[3];
    char me[16];
    int i;
    MP_GameToAscii(me, gSaveBlock2Ptr->playerName, 8);
    Slot(0, me);
    BlackLine(69, 0, title, sizeof(title));   // "<name>'S ENTREE"
    for (i = 0; i < 3; i++)
        BlackLine(69, 1 + i, items[i], sizeof(items[i])), list[i] = items[i];
    Background();
    Header(title, NULL);
    Unova_Screen_List(56, 32, 14, list, 3, E.sel);
    if (M.running)
    {
        char l[80];
        BlackLine(69, 4, l, sizeof(l));   // "There is a mission in progress."
        Unova_Screen_Message(l, NULL);
    }
}

static void DrawMissions(void)
{
    char title[40], mins[8], line[60];
    u8 ids[MP_MAX_PLAYERS];
    int i, n;
    BlackLine(69, 14, title, sizeof(title));   // "BEGIN A MISSION"
    Background();
    Header(title, NULL);
    G_Window(0, 16, 160, 112);
    for (i = 0; i < M.count; i++)
    {
        const struct Mission *m = &M.choice[i];
        char kind[40];
        BlackLine(67, m->kind == MK_BATTLE ? 0 : 1, kind, sizeof(kind));   // "BATTLE MISSION" / "SUPPORT MISSION"
        G_Text(16, 24 + i * 16, G_INK, G_INK_SHADOW, kind, 110, TF_SMALL);
        snprintf(mins, sizeof(mins), "%u:%02u", m->seconds / 60, m->seconds % 60);
        G_TextAlign(150, 24 + i * 16, TA_RIGHT, G_INK, G_INK_SHADOW, mins, TF_SMALL);
        if (i == E.sel)
            G_Cursor(9, 27 + i * 16);
    }
    G_Window(160, 16, 80, 112);
    G_Text(168, 22, G_INK, G_INK_SHADOW, "JOINING", 64, TF_SMALL);
    {
        char me[16];
        MP_GameToAscii(me, gSaveBlock2Ptr->playerName, 8);
        G_Text(172, 38, G_INK, G_INK_SHADOW, me, 60, TF_SMALL);
    }
    n = Friends(ids);
    for (i = 0; i < n && i < 4; i++)
    {
        char name[16];
        FriendName(ids[i], name, sizeof(name));
        G_Text(172, 52 + i * 14, G_INK, G_INK_SHADOW, name, 60, TF_SMALL);
    }
    BlackLine(69, 19, line, sizeof(line));   // "Please select a mission."
    Unova_Screen_Message(line, NULL);
}

static void DrawPowerList(bool8 receive)
{
    char title[40], line[80], orbs[24];
    int i, rows = 6;
    BlackLine(69, receive ? 34 : 38, title, sizeof(title));   // "AVAILABLE PASS POWER" / "CURRENT PASS POWER"
    if (receive)
        snprintf(orbs, sizeof(orbs), "ORBS: %u", Save()->orbs);
    else
        snprintf(orbs, sizeof(orbs), "SHARE: %s", Save()->shareOff ? "OFF" : "ON");
    Background();
    Header(title, orbs);
    G_Window(0, 16, 240, 112);
    if (!E.count)
    {
        BlackLine(69, 39, line, sizeof(line));   // "No Pass Power has been received."
        G_Text(16, 24, G_INK, G_INK_SHADOW, receive ? "" : line, 210, TF_SMALL);
    }
    for (i = 0; i < rows && E.top + i < E.count; i++)
    {
        u8 id = E.list[E.top + i];
        char name[40], extra[24];
        int y = 24 + i * 15;
        const u16 *p = U3D_Vm_GPower(id);
        PowerName(id, name, sizeof(name));
        G_Text(16, y, G_INK, G_INK_SHADOW, name, 130, TF_SMALL);
        if (receive)
            snprintf(extra, sizeof(extra), "Lv.%u  %u", p[0], p[2]);
        else
            snprintf(extra, sizeof(extra), "%u min", p[5]);
        G_TextAlign(176, y, TA_RIGHT, G_INK, G_INK_SHADOW, extra, TF_SMALL);
        Dots(186, y + 3, id);
        if (E.top + i == E.sel)
            G_Cursor(9, y + 3);
    }
    if (E.count)
    {
        u8 id = E.list[E.sel];
        if (receive)
        {
            SlotNum(0, U3D_Vm_GPower(id)[2]);
            BlackLine(69, 40, line, sizeof(line));   // "Orbs needed: N"
        }
        else
            BlackLine(69, 37, line, sizeof(line));   // "Please select the Pass Power..."
        if (!receive)
            Unova_Screen_Message("Use it for the whole room?", NULL);
        else
            Unova_Screen_Message(line, NULL);
    }
    else
        Unova_Screen_Message(" ", NULL);
}

static void DrawStatus(void)
{
    char title[40], l[4][60];
    int i;
    BlackLine(69, 13, title, sizeof(title));   // "ENTREE STATUS"
    // (Black's number codes: {0202:0} is slot 0, the first number its kind)
    SlotNum(0, BlackLevel());
    BlackLine(69, 5, l[0], sizeof(l[0]));      // "Entree's Total Level: N"
    BlackLine(69, 9, l[1], sizeof(l[1]));      // "Black Lv.: N"
    BuildOwned();
    SlotNum(0, E.count);
    BlackLine(69, 10, l[2], sizeof(l[2]));     // "Available Pass Powers: N"
    SlotNum(0, Save()->missions);
    BlackLine(69, 12, l[3], sizeof(l[3]));     // "Completed missions: N"
    Background();
    Header(title, NULL);
    G_Window(0, 16, 240, 96);
    for (i = 0; i < 4; i++)
        G_Text(16, 26 + i * 18, G_INK, G_INK_SHADOW, l[i], 210, TF_NORMAL);
    {
        char orbs[24];
        snprintf(orbs, sizeof(orbs), "PASS ORBS: %u", Save()->orbs);
        Unova_Screen_Message(orbs, NULL);
    }
}

static void EntreeDraw(void)
{
    switch (E.page == ES_NOTE ? E.back : E.page)
    {
    case ES_MENU: DrawMenu(); break;
    case ES_MISSIONS: case ES_MISSION_ASK: DrawMissions(); break;
    case ES_RECEIVE: case ES_RECEIVE_ASK: DrawPowerList(TRUE); break;
    case ES_POWERS: case ES_POWER_ASK: DrawPowerList(FALSE); break;
    case ES_STATUS: DrawStatus(); break;
    }
    if (E.page == ES_MISSION_ASK)
    {
        char t[200], yes[8], no[8];
        const char *yn[2];
        MissionText(&M.choice[E.sel], t, sizeof(t));
        BigText(t);
        BlackLine(69, 30, yes, sizeof(yes)), BlackLine(69, 31, no, sizeof(no));
        yn[0] = yes, yn[1] = no;
        Unova_Screen_List(176, 4, 6, yn, 2, E.ask);
    }
    else if (E.page == ES_RECEIVE_ASK || E.page == ES_POWER_ASK)
    {
        char t[120], yes[8], no[8];
        const char *yn[2];
        if (E.page == ES_RECEIVE_ASK)
            BlackLine(69, 32, t, sizeof(t));   // "Will you receive this Pass Power from the Entree?"
        else
            snprintf(t, sizeof(t), "%s", Save()->shareOff ? "Use this Pass Power?" : "Use it for the whole room?");
        Unova_Screen_Message(t, NULL);
        BlackLine(69, 30, yes, sizeof(yes)), BlackLine(69, 31, no, sizeof(no));
        yn[0] = yes, yn[1] = no;
        Unova_Screen_List(176, 64, 6, yn, 2, E.ask);
    }
    else if (E.page == ES_NOTE)
    {
        G_Window(0, 96, 240, 64);
        G_Text(10, 104, G_INK, G_INK_SHADOW, E.note[0], 220, TF_NORMAL);
        if (E.note[1][0])
            G_Text(10, 136, G_INK, G_INK_SHADOW, E.note[1], 220, TF_NORMAL);
    }
}

static bool8 MoveSel(u16 pressed, u8 count, u8 rows)
{
    if (!count)
        return FALSE;
    if (pressed & DPAD_UP)
        E.sel = (E.sel + count - 1) % count;
    else if (pressed & DPAD_DOWN)
        E.sel = (E.sel + 1) % count;
    else
        return FALSE;
    if (E.sel < E.top)
        E.top = E.sel;
    if (E.sel >= E.top + rows)
        E.top = E.sel - rows + 1;
    PlaySE(SE_SELECT);
    return TRUE;
}

static void EntreeStep(u16 pressed, u16 keys)
{
    struct UnovaEntralinkSave *s = Save();
    (void)keys;
    switch (E.page)
    {
    case ES_NOTE:
        if (pressed & (A_BUTTON | B_BUTTON))
            PlaySE(SE_SELECT), E.page = E.back;
        return;
    case ES_MENU:
        if (MoveSel(pressed, 3, 3))
            return;
        if (pressed & B_BUTTON)
        {
            PlaySE(SE_SELECT);
            Unova_Screen_Close(NULL);
        }
        else if (pressed & A_BUTTON)
        {
            PlaySE(SE_SELECT);
            if (E.sel == 0)
            {
                if (M.running)
                    NoteBlack(69, 17);   // "Another mission is already in progress."
                else if (!MP_IsConnected() || (MakeChoices(), M.count == 0))
                    NoteBlack(69, 22);   // "By selecting missions, you can help others..."
                else
                    E.page = ES_MISSIONS, E.sel = E.top = 0;
            }
            else if (E.sel == 1)
                BuildReceivable(), E.page = ES_RECEIVE, E.sel = E.top = 0;
            else
                E.page = ES_STATUS;
        }
        return;
    case ES_MISSIONS:
        if (MoveSel(pressed, M.count, MISSION_CHOICES))
            return;
        if (pressed & B_BUTTON)
            PlaySE(SE_SELECT), E.page = ES_MENU, E.sel = 0;
        else if (pressed & A_BUTTON)
            PlaySE(SE_SELECT), E.page = ES_MISSION_ASK, E.ask = 0;
        return;
    case ES_MISSION_ASK:
    case ES_RECEIVE_ASK:
    case ES_POWER_ASK:
        if (pressed & (DPAD_UP | DPAD_DOWN))
            E.ask ^= 1, PlaySE(SE_SELECT);
        else if (pressed & B_BUTTON || (pressed & A_BUTTON && E.ask == 1))
            PlaySE(SE_SELECT), E.page = E.page == ES_MISSION_ASK ? ES_MISSIONS : E.page == ES_RECEIVE_ASK ? ES_RECEIVE : ES_POWERS;
        else if (pressed & A_BUTTON)
        {
            PlaySE(SE_SELECT);
            if (E.page == ES_MISSION_ASK)
            {
                StartMission(&M.choice[E.sel], gMpLocalId);
                SendMission();
                Unova_Screen_Close(NULL);
            }
            else if (E.page == ES_RECEIVE_ASK)
            {
                u8 id = E.list[E.sel];
                const u16 *p = U3D_Vm_GPower(id);
                E.page = ES_RECEIVE;
                if (BlackLevel() < p[0])
                {
                    SlotNum(0, p[0]);
                    SlotNum(1, 0);
                    SlotNum(2, BlackLevel());
                    NoteBlack(69, 42);   // "The Black Level to receive it: N ... Current levels"
                }
                else if (s->orbs < p[2])
                {
                    SlotNum(0, p[2]);
                    NoteBlack(69, 40);   // "Orbs needed: N"
                }
                else
                {
                    char name[40];
                    s->orbs -= p[2];
                    s->owned[id / 8] |= 1 << (id % 8);
                    PowerName(id, name, sizeof(name));
                    Slot(0, name);
                    PlaySE(SE_SUCCESS);
                    NoteBlack(69, 35);   // "Received <power> from the Entree! ... registered to C-Gear!"
                    printf("[test] pass power %u received\n", id);
                    BuildReceivable();
                    if (E.sel >= E.count)
                        E.sel = E.count ? E.count - 1 : 0;
                }
            }
            else
            {
                u8 id = E.list[E.sel];
                Activate(id);
                if (!s->shareOff && MP_IsConnected())
                {
                    u8 b[3] = {MPR_ENTRALINK, EL_POWER_SHARE, id};
                    MP_SendRelay(0xFF, b, 3);
                }
                PlaySE(SE_SUCCESS);
                Unova_Screen_Close(NULL);
            }
        }
        return;
    case ES_RECEIVE:
    case ES_POWERS:
        if (MoveSel(pressed, E.count, 6))
            return;
        if (E.page == ES_POWERS && pressed & SELECT_BUTTON)
            s->shareOff ^= 1, PlaySE(SE_SELECT);
        else if (pressed & B_BUTTON)
        {
            PlaySE(SE_SELECT);
            if (E.page == ES_POWERS)
                Unova_Screen_Close(NULL);
            else
                E.page = ES_MENU, E.sel = 1;
        }
        else if (pressed & A_BUTTON && E.count)
            PlaySE(SE_SELECT), E.page = E.page == ES_RECEIVE ? ES_RECEIVE_ASK : ES_POWER_ASK, E.ask = 0;
        return;
    case ES_STATUS:
        if (pressed & (A_BUTTON | B_BUTTON))
            PlaySE(SE_SELECT), E.page = ES_MENU, E.sel = 2;
        return;
    }
}

static void EntreeOpen(void)
{
    memset(&E, 0, sizeof(E));
    E.page = ES_MENU;
}

static void PowersOpen(void)
{
    memset(&E, 0, sizeof(E));
    BuildOwned();
    E.page = ES_POWERS;
}

static const struct UnovaScreen sEntree = {"ENTREE", EntreeOpen, EntreeDraw, EntreeStep};
static const struct UnovaScreen sPowers = {"PASS POWER", PowersOpen, EntreeDraw, EntreeStep};

void Unova_Entree_Open(void)
{
    Unova_Screen_Open(&sEntree);
}

void Unova_PassPowers_Open(void)
{
    Unova_Screen_Open(&sPowers);
}

// (tests: `mp passpower ID ORBS MISSIONS`: a PASS POWER received, Pass Orbs and missions added)
void Unova_Entralink_DebugGive(u8 id, u16 orbs, u16 missions)
{
    struct UnovaEntralinkSave *s = Save();
    if (id < POWERS)
        s->owned[id / 8] |= 1 << (id % 8);
    s->orbs += orbs;
    s->missions += missions;
}

u8 Unova_PassPower_Active(u8 kind)
{
    return kind < KINDS ? Save()->activeId[kind] : NO_POWER;
}
