// Moemon Kai: UNOVA's own screens (the user's approved mock-ups, docs/unova/mockups/, tools/unova/mockups.py), drawn at
// the GBA's 240x160 with the game's own font and window frame (src/tcg/tcg_gfx.c's G_*) into a texture the display
// shows at twice its size, sharp (sdl2.c / display.c: Unova_Screen_Canvas / _Frame / _Step, as SINNOH's digging and
// ORRE's world map); the game waits while one is open. Here: the screen host, and the C-GEAR (Black's device, its
// buttons as co-op: IR the friend in front (TRADE / BATTLE), WIRELESS the CO-OP menu, ENTRALINK there; the friends in
// the room on its panel as Black shows passers-by), opened from START in UNOVA once Black's script has given it.
// The ENTRALINK's and the MUSICAL's screens: u3d_entralink.c, u3d_musical.c (through Unova_Screen_Open).
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include "global.h"
#include "main.h"
#include "rtc.h"
#include "sound.h"
#include "script.h"
#include "overworld.h"
#include "field_player_avatar.h"
#include "constants/songs.h"
#include "mp.h"
#include "u3d.h"
#include "u3d_screen.h"
#include "../tcg/tcg_gfx.h"

#define SW 240
#define SH 160
#define CW 480
#define CH 320

const u16 *Platform_GetFrame(void);   // (sdl2.c: the game picture as it is)
void MP_GameToAscii(char *dst, const u8 *src, int maxLen);

static struct
{
    const struct UnovaScreen *screen;   // (NULL: none open)
    SDL_Renderer *owner;
    SDL_Texture *gba, *canvas, *field, *hexes;
    u16 fieldPic[SW * SH];
    bool8 fieldNew;
    u16 frame[SW * SH];
    bool8 frameOk;
    u16 prevKeys;
    u32 t;
    void (*pending)(void);   // (run on the next field frame once the screen is closed)
} S;

// ------------------------------------------------------------------------------------------------- the host
void Unova_Screen_Open(const struct UnovaScreen *screen)
{
    const u16 *f = Platform_GetFrame();
    if (f)
        memcpy(S.fieldPic, f, sizeof(S.fieldPic));
    S.fieldNew = TRUE;
    S.screen = screen;
    S.prevKeys = 0xFFFF;   // (the button that opened it isn't a press here)
    S.t = 0;
    S.frameOk = FALSE;
    if (screen->open)
        screen->open();
    MPNet_Log("unova screen: %s", screen->name);
}

void Unova_Screen_Close(void (*then)(void))
{
    if (S.screen)
        MPNet_Log("unova screen: %s closed", S.screen->name);
    S.screen = NULL;
    S.pending = then;
}

bool8 Unova_Screen_IsActive(void)
{
    return S.screen != NULL;
}

u32 Unova_Screen_Time(void)
{
    return S.t;
}

// u3d_vm.c U3D_Vm_FieldFrame: what a closed screen left to do on the field (a warp, the CO-OP menu...)
void Unova_Screen_FieldFrame(void)
{
    void Unova_Entralink_FieldFrame(void);
    Unova_Entralink_FieldFrame();   // (the PASS POWERS' minutes, the room's mission: u3d_entralink.c)
    {
        void Unova_Musical_FieldFrame(void);
        Unova_Musical_FieldFrame();   // (a MUSICAL's next step: u3d_musical.c)
    }
    {
        void Unova_Phenom_FieldFrame(void);
        Unova_Phenom_FieldFrame();   // (rustling grass, dust, ripples, a bird's shadow: u3d_phenom.c)
    }
    if (S.pending && !S.screen)
    {
        void (*f)(void) = S.pending;
        S.pending = NULL;
        f();
    }
}

bool8 Unova_Screen_Step(u16 keys)
{
    u16 pressed;
    if (!S.screen)
        return FALSE;
    pressed = keys & ~S.prevKeys;
    S.prevKeys = keys;
    S.t++;
    S.screen->step(pressed, keys);
    return TRUE;
}

// The field as it was when the screen opened (the C-GEAR is drawn over it, as the POKeTCH is)
void Unova_Screen_DrawField(unsigned darken)
{
    SDL_Renderer *r = G_R();
    SDL_Rect d = {0, 0, SW, SH};
    if (!S.field)
    {
        S.field = SDL_CreateTexture(r, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, SW, SH);
        S.fieldNew = TRUE;
    }
    if (!S.field)
        return;
    if (S.fieldNew)
    {
        static uint32_t px[SW * SH];
        u32 i;
        for (i = 0; i < SW * SH; i++)
            px[i] = 0xFF000000u | G_Rgb15(S.fieldPic[i]);
        SDL_UpdateTexture(S.field, NULL, px, SW * 4);
        S.fieldNew = FALSE;
    }
    SDL_RenderCopy(r, S.field, NULL, &d);
    if (darken)
        G_FillA(0, 0, SW, SH, 0x000000, darken);
}

SDL_Texture *Unova_Screen_Canvas(SDL_Renderer *r)
{
    SDL_Texture *previous;
    SDL_Rect oldViewport, oldClip, all = {0, 0, CW, CH};
    SDL_bool clipped;
    float oldX, oldY;
    if (!S.screen)
        return NULL;
    G_Begin(r);
    if (S.owner != r)
    {
        S.gba = S.canvas = S.field = S.hexes = NULL;
        S.owner = r;
    }
    if (!S.gba)
    {
        S.gba = SDL_CreateTexture(r, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, SW, SH);
        S.canvas = SDL_CreateTexture(r, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, CW, CH);
        if (!S.gba || !S.canvas)
            return NULL;
        SDL_SetTextureBlendMode(S.gba, SDL_BLENDMODE_NONE);
        SDL_SetTextureScaleMode(S.gba, SDL_ScaleModeNearest);
        SDL_SetTextureBlendMode(S.canvas, SDL_BLENDMODE_NONE);
    }
    previous = SDL_GetRenderTarget(r);
    SDL_RenderGetViewport(r, &oldViewport);
    clipped = SDL_RenderIsClipEnabled(r);
    SDL_RenderGetClipRect(r, &oldClip);
    SDL_RenderGetScale(r, &oldX, &oldY);
    SDL_SetRenderTarget(r, S.gba);
    SDL_RenderSetViewport(r, NULL);
    SDL_RenderSetClipRect(r, NULL);
    SDL_RenderSetScale(r, 1, 1);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
    SDL_RenderClear(r);
    S.screen->draw();
    SDL_SetRenderTarget(r, S.gba);
    S.frameOk = SDL_RenderReadPixels(r, NULL, SDL_PIXELFORMAT_ABGR1555, S.frame, SW * 2) == 0;
    if (getenv("MOEMON_SCREEN_TRACE"))
        printf("[test] unova screen drawn %u ok %d err %s\n", S.t, S.frameOk, SDL_GetError());
    SDL_SetRenderTarget(r, S.canvas);
    SDL_RenderCopy(r, S.gba, NULL, &all);
    SDL_SetRenderTarget(r, previous);
    SDL_RenderSetViewport(r, &oldViewport);
    SDL_RenderSetClipRect(r, clipped ? &oldClip : NULL);
    SDL_RenderSetScale(r, oldX, oldY);
    return S.canvas;
}

// the game picture while a screen is open (spectators, test shots)
const u16 *Unova_Screen_Frame(void)
{
    static u16 out[SW * SH];
    u32 i;
    if (!S.screen || !S.frameOk)
        return NULL;
    for (i = 0; i < SW * SH; i++)
        out[i] = S.frame[i] & 0x7FFF;
    return out;
}

// ------------------------------------------------------------------------------------------------- drawing helpers
void Unova_Screen_Text(int x, int y, unsigned color, unsigned shadow, const char *s, int font, int align)
{
    G_TextAlign(x, y, align, color, shadow, s, font);
}

// The message box at the foot of the screen, in the game's window frame (two lines)
void Unova_Screen_Message(const char *line1, const char *line2)
{
    G_Window(0, 112, 240, 48);
    G_Text(10, 120, G_INK, G_INK_SHADOW, line1, 220, TF_NORMAL);
    if (line2)
        G_Text(10, 136, G_INK, G_INK_SHADOW, line2, 220, TF_NORMAL);
}

// A list in the game's window frame: COUNT items, the cursor's at SEL
void Unova_Screen_List(int x, int y, int wTiles, const char *const *items, int count, int sel)
{
    int i;
    G_Window(x, y, wTiles * 8 + 16, count * 16 + 16);
    for (i = 0; i < count; i++)
    {
        G_Text(x + 16, y + 8 + i * 16, G_INK, G_INK_SHADOW, items[i], wTiles * 8 - 8, TF_NORMAL);
        if (i == sel)
            G_Cursor(x + 9, y + 12 + i * 16);
    }
}

// ------------------------------------------------------------------------------------------------- the C-GEAR
enum { CG_IR, CG_WIRELESS, CG_ENTRALINK, CG_BUTTONS };
enum { CGM_NONE, CGM_IR_MENU, CGM_NOTE, CGM_EL_MENU };
static struct
{
    u8 sel, mode, menuSel;
    s8 friendId;
    char note[2][40];
} C;

#define CGEAR_BODY 0x383840
#define CGEAR_EDGE 0x181820
#define CGEAR_TRIM 0x606070
#define CGEAR_TEXT 0xe8e8f0
#define CGEAR_TEXT_SH 0x585868
#define HEX_BASE 0x284070
#define HEX_LIGHT 0x5888c8
#define HEX_LINE 0x4060a0

// The panel's honeycomb (Black's C-GEAR skin, its plain blue), built once
static SDL_Texture *Hexes(void)
{
    struct GSurface s;
    int x, y;
    if (S.hexes)
        return S.hexes;
    s = G_SurfaceNew(200, 87);
    for (y = 0; y < s.h; y++)
        for (x = 0; x < s.w; x++)
        {
            // pointy-topped hexes 13 wide, rows 11 apart, odd rows shifted half a hex: the nearest centre, and how
            // near the pixel is to its hex's rim
            float best = 1e9f, second = 1e9f;
            int bestRow = 0, bestCol = 0, row, col;
            for (row = y / 11 - 1; row <= y / 11 + 1; row++)
                for (col = x / 13 - 1; col <= x / 13 + 1; col++)
                {
                    float cx = col * 13 + ((row & 1) ? 6.5f : 0), cy = row * 11;
                    float dx = x - cx, dy = (y - cy) * 1.18f, d = dx * dx + dy * dy;
                    if (d < best)
                        second = best, best = d, bestRow = row, bestCol = col;
                    else if (d < second)
                        second = d;
                }
            if (sqrtf(second) - sqrtf(best) < 1.2f)
                S_Px(&s, x, y, ARGB(HEX_LINE, 255));
            else
                S_Px(&s, x, y, ARGB(((bestRow * 7 + bestCol * 3) % 11 + 11) % 11 == 0 ? HEX_LIGHT : HEX_BASE, 255));
        }
    S.hexes = G_SurfaceTexture(&s);
    G_SurfaceFree(&s);
    return S.hexes;
}

// The friends the C-GEAR sees: co-op players in the room (as Black shows the people around)
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

// The friend right in front of the player (IR: Black's infrared reaches the DS face to face)
static s8 FriendInFront(void)
{
    s16 x, y, rx, ry;
    int i;
    if (!MP_IsConnected())
        return -1;
    GetXYCoordsOneStepInFrontOfPlayer(&x, &y);
    for (i = 0; i < MP_MAX_PLAYERS; i++)
        if (i != gMpLocalId && gMpPlayers[i].present && MP_GetRemoteMapPos(i, &rx, &ry) && rx == x && ry == y)
            return i;
    return -1;
}

static void FriendName(u8 id, char *out, int n)
{
    MP_GameToAscii(out, gMpPlayers[id].name, n);
}

static void CGearOpen(void)
{
    memset(&C, 0, sizeof(C));
    C.friendId = -1;
    PlaySE(SE_POKENAV_ON);
}

static void CGearButton(int i, const char *top, const char *sub)
{
    int x = 22 + i * 66, on = C.sel == i && C.mode == CGM_NONE;
    G_RoundFill(x, 116, 63, 30, CGEAR_EDGE, 255);
    G_RoundFill(x + 1, 117, 61, 28, on ? 0xf8e078 : 0xd8d8e0, 255);
    G_RoundFrame(x + 2, 118, 59, 26, on ? 0xfff8c8 : 0xf8f8f8, 255);
    G_TextAlign(x + 31, sub ? 118 : 123, TA_CENTER, G_INK, G_INK_SHADOW, top,
                G_Measure(top, TF_NORMAL) > 56 ? TF_SMALL : TF_NORMAL);
    if (sub)
        G_TextAlign(x + 31, 131, TA_CENTER, 0x606070, G_NO_SHADOW, sub, TF_SMALL);
}

static void CGearDraw(void)
{
    SDL_Rect hex = {20, 26, 200, 87};
    u8 ids[MP_MAX_PLAYERS];
    int n, i;
    char clock[8], line[48];
    Unova_Screen_DrawField(64);
    // the device
    G_RoundFill(12, 6, 216, 148, CGEAR_EDGE, 255);
    G_RoundFill(13, 7, 214, 146, CGEAR_BODY, 255);
    G_RoundFrame(14, 8, 212, 144, CGEAR_TRIM, 255);
    G_Fill(20, 12, 200, 12, 0x202028);
    G_Text(24, 10, CGEAR_TEXT, CGEAR_TEXT_SH, "C-GEAR", 80, TF_SMALL);
    RtcCalcLocalTime();
    snprintf(clock, sizeof(clock), "%2d:%02d", gLocalTime.hours, gLocalTime.minutes);
    G_TextAlign(196, 10, TA_RIGHT, CGEAR_TEXT, CGEAR_TEXT_SH, clock, TF_SMALL);
    G_Frame(201, 15, 14, 6, CGEAR_TEXT, 255);
    G_Fill(203, 17, 9, 2, 0x78d878);
    G_Fill(215, 16, 2, 4, CGEAR_TEXT);
    SDL_RenderCopy(G_R(), Hexes(), NULL, &hex);
    // the people around: the friends in the room
    n = Friends(ids);
    for (i = 0; i < n && i < 6; i++)
    {
        static const s16 sAt[6][2] = {{64, 50}, {150, 74}, {44, 84}, {170, 40}, {100, 36}, {112, 88}};
        char name[16];
        int w;
        FriendName(ids[i], name, sizeof(name));
        w = G_Measure(name, TF_SMALL) + 8;
        G_RoundFill(sAt[i][0] - w / 2, sAt[i][1] - 2, w, 14, CGEAR_EDGE, 255);
        G_RoundFill(sAt[i][0] - w / 2 + 1, sAt[i][1] - 1, w - 2, 12, 0xf8f8f8, 255);
        G_TextAlign(sAt[i][0], sAt[i][1] - 2, TA_CENTER, G_INK, G_INK_SHADOW, name, TF_SMALL);
    }
    if (!MP_IsConnected())
        snprintf(line, sizeof(line), "Not online");
    else
        snprintf(line, sizeof(line), "Friends in the room: %d", n);
    G_TextAlign(120, 96, TA_CENTER, 0xe8f0ff, 0x283860, line, TF_SMALL);
    CGearButton(CG_IR, "IR", "FRIEND");
    CGearButton(CG_WIRELESS, "WIRELESS", "CO-OP");
    CGearButton(CG_ENTRALINK, "ENTRALINK", NULL);
    if (C.mode == CGM_IR_MENU)
    {
        static const char *const sItems[] = {"TRADE", "BATTLE", "CANCEL"};
        char name[16], l1[40];
        Unova_Screen_List(120, 30, 10, sItems, 3, C.menuSel);
        FriendName(C.friendId, name, sizeof(name));
        snprintf(l1, sizeof(l1), "%s is in front of you.", name);
        Unova_Screen_Message(l1, "What would you like to do?");
    }
    else if (C.mode == CGM_NOTE)
        Unova_Screen_Message(C.note[0], C.note[1][0] ? C.note[1] : NULL);
    else if (C.mode == CGM_EL_MENU)
    {
        // (Black's C-GEAR: its ENTRALINK button there; the ENTREE once in it; the PASS POWERS received)
        static const char *const sEl[] = {"ENTRALINK", "PASS POWER", "CANCEL"};
        Unova_Screen_List(144, 46, 9, sEl, 3, C.menuSel);
    }
}

static void Note(const char *a, const char *b)
{
    snprintf(C.note[0], sizeof(C.note[0]), "%s", a);
    snprintf(C.note[1], sizeof(C.note[1]), "%s", b ? b : "");
    C.mode = CGM_NOTE;
}

// What the buttons do once the C-GEAR is closed (the field running again)
static void DoTrade(void)
{
    if (C.friendId >= 0)
        MP_Trade_AskFriend(C.friendId);
}

static void DoBattle(void)
{
    if (C.friendId >= 0 && !MP_Trainer_ChallengeFriend(C.friendId))
        MP_ShowNotice("You can't challenge your friend right now.");
}

static void DoCoopMenu(void)
{
    MP_OpenMenu();
}

static void DoEntralink(void)
{
    U3D_Vm_GoToEntralink();
}

static void DoEntree(void)
{
    void Unova_Entree_Open(void);
    Unova_Entree_Open();
}

static void DoPowers(void)
{
    void Unova_PassPowers_Open(void);
    Unova_PassPowers_Open();
}

static void CGearStep(u16 pressed, u16 keys)
{
    (void)keys;
    switch (C.mode)
    {
    case CGM_NOTE:
        if (pressed & (A_BUTTON | B_BUTTON))
        {
            PlaySE(SE_SELECT);
            C.mode = CGM_NONE;
        }
        return;
    case CGM_EL_MENU:
        if (pressed & DPAD_UP)
            C.menuSel = (C.menuSel + 2) % 3, PlaySE(SE_SELECT);
        else if (pressed & DPAD_DOWN)
            C.menuSel = (C.menuSel + 1) % 3, PlaySE(SE_SELECT);
        else if (pressed & B_BUTTON)
            C.mode = CGM_NONE, PlaySE(SE_SELECT);
        else if (pressed & A_BUTTON)
        {
            bool8 U3D_Vm_InEntralink(void);
            PlaySE(SE_SELECT);
            if (C.menuSel == 2)
                C.mode = CGM_NONE;
            else if (C.menuSel == 1)
                Unova_Screen_Close(DoPowers);
            else
                Unova_Screen_Close(U3D_Vm_InEntralink() ? DoEntree : DoEntralink);
        }
        return;
    case CGM_IR_MENU:
        if (pressed & DPAD_UP)
            C.menuSel = (C.menuSel + 2) % 3, PlaySE(SE_SELECT);
        else if (pressed & DPAD_DOWN)
            C.menuSel = (C.menuSel + 1) % 3, PlaySE(SE_SELECT);
        else if (pressed & B_BUTTON)
            C.mode = CGM_NONE, PlaySE(SE_SELECT);
        else if (pressed & A_BUTTON)
        {
            PlaySE(SE_SELECT);
            if (C.menuSel == 2)
                C.mode = CGM_NONE;
            else
                Unova_Screen_Close(C.menuSel == 0 ? DoTrade : DoBattle);
        }
        return;
    }
    if (pressed & DPAD_LEFT)
        C.sel = (C.sel + CG_BUTTONS - 1) % CG_BUTTONS, PlaySE(SE_SELECT);
    else if (pressed & DPAD_RIGHT)
        C.sel = (C.sel + 1) % CG_BUTTONS, PlaySE(SE_SELECT);
    else if (pressed & B_BUTTON)
    {
        PlaySE(SE_POKENAV_OFF);
        Unova_Screen_Close(NULL);
    }
    else if (pressed & A_BUTTON)
    {
        PlaySE(SE_SELECT);
        switch (C.sel)
        {
        case CG_IR:
            if (!MP_IsConnected())
                Note("Go online with the CO-OP menu", "to reach a friend.");
            else if ((C.friendId = FriendInFront()) < 0)
                Note("Face a friend to trade or battle", "with them.");
            else
                C.mode = CGM_IR_MENU, C.menuSel = 0;
            break;
        case CG_WIRELESS:
            Unova_Screen_Close(DoCoopMenu);
            break;
        case CG_ENTRALINK:
            C.mode = CGM_EL_MENU, C.menuSel = 0;
            break;
        }
    }
}

static const struct UnovaScreen sCGear = {"C-GEAR", CGearOpen, CGearDraw, CGearStep};

void Unova_CGear_Open(void)
{
    Unova_Screen_Open(&sCGear);
}

// (tests: the C-GEAR's state)
u8 Unova_CGear_Selected(void)
{
    return C.sel;
}
