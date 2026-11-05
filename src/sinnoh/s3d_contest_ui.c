// Moemon Kai: SUPER CONTESTS' screens (the rules: s3d_contest.c). Drawn at 480x320 like the BADGE CASE
// (mp_badgecase.c: IsActive / Step / Canvas / Frame, sdl2.c and display.c show it as the game picture), in the order
// Platinum runs them (src/contest.c FieldTask_RunContest): the VISUAL round (the player's dress-up against Platinum's
// time, then each entrant's look and hearts), the DANCE round (four songs' measures: the lead's steps, the backups'
// copies, each step judged), the ACTING round (four turns: a move and a judge, the performances, the judges'
// voltage), then the results. Every line said is Platinum's own (contest_text, contest_visual_competition,
// contest_dance_competition, contest_acting_competition, contest_results); the accessories and backdrops are its art
// (sinnoh/platinum_gfx.pak "CNTS"), the POKéMON this game's pictures, the music this game's contest songs.
#include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>
#include "global.h"
#include "data.h"
#include "decompress.h"
#include "easy_chat.h"
#include "event_data.h"
#include "main.h"
#include "m4a.h"
#include "pokemon.h"
#include "script.h"
#include "sound.h"
#include "string_util.h"
#include "strings.h"
#include "constants/characters.h"
#include "constants/songs.h"
#include "mp.h"
#include "s3d.h"
#include "s3d_contest.h"
#include "../tcg/tcg_gfx.h"

#define C gSinnohContest
#define CW 480
#define CH 320
#define DRESS_FRAMES (60 * 60)   // (Platinum's dress-up: a minute)
#define PHOTO_W 256              // (the dress-up's own space: the DS screen, the POKéMON in its middle)
#define PHOTO_H 192
#define MON_X 128
#define MON_Y 128

bool8 S3D_Gfx_ContestPicture(bool8 backdrop, u16 n, u16 *w, u16 *h, const u16 **px);
bool8 Sinnoh_HasBackdrop(u16 backdrop);
void Sinnoh_AccessoryName(u8 *dest, u16 accessory, bool8 article);

enum {
    SC_NONE, SC_INTRO_VISUAL, SC_DRESS, SC_VISUAL_SHOW, SC_INTRO_DANCE, SC_DANCE, SC_DANCE_END, SC_INTRO_ACTING,
    SC_ACT_MOVE, SC_ACT_JUDGE, SC_ACT_PERFORM, SC_ACT_TURN_END, SC_ACT_END, SC_RESULTS, SC_PHOTO, SC_DONE,
};

static struct
{
    bool8 active, enableScript, studio;
    u8 scene;
    u16 prevKeys;
    u32 t;
    // the text box: Platinum's message, page by page
    char pages[8][160];
    u8 pageCount, page;
    u16 typed;
    // dress-up
    u8 list[100], listCount, cursor, top, held, backdropPick;
    s16 hx, hy;
    u32 left;
    // the visual show
    u8 shown, step;
    // dance
    u8 round, part;
    u32 partStart;
    u16 beat;   // (60ths of a second, times 16)
    struct { s16 at; u8 dir, made, grade; } lead[16], copy[CONTEST_ENTRANTS][16];
    u8 leadCount, copyCount[CONTEST_ENTRANTS];
    s16 flash[CONTEST_ENTRANTS];
    u8 flashGrade[CONTEST_ENTRANTS], flashDir[CONTEST_ENTRANTS];
    u8 playerSteps;
    // acting
    u8 menu, move, judge, place, noteShown;
    bool8 concedeAsk;   // (START on the move list: CONCEDE?)
    // results
    u8 resultStep;
    // textures
    SDL_Renderer *owner;
    SDL_Texture *canvas, *acc[100], *bd[18], *mon[CONTEST_ENTRANTS];
    u16 monSpecies[CONTEST_ENTRANTS];
    u32 monKey[CONTEST_ENTRANTS];
    const struct ContestEntrant *photo;
    u16 photoTitle;
    u16 bigFrame[CW * CH];
    bool8 frameOk;
} U;

// ----------------------------------------------------------------------------------------------------- the words

static void Utf8(char *dst, int cap, const u8 *src, u8 pages[8], u8 *count)
{
    int n = 0;
    (void)pages;
    for (; *src != EOS && n < cap - 3; src++)
    {
        u8 c = *src;
        if (c == EXT_CTRL_CODE_BEGIN)
        {
            src += GetExtCtrlCodeLength(src[1]) - 1;
            continue;
        }
        if (c == CHAR_NEWLINE)
            dst[n++] = '\n';
        else if (c == CHAR_PROMPT_SCROLL || c == CHAR_PROMPT_CLEAR)
            dst[n++] = '\f';
        else if (c == CHAR_e_ACUTE)
            dst[n++] = (char)0xC3, dst[n++] = (char)0xA9;
        else if (c == CHAR_SGL_QUOTE_RIGHT || c == CHAR_SGL_QUOTE_LEFT)
            dst[n++] = '\'';
        else if (c == CHAR_ELLIPSIS)
            dst[n++] = '.', dst[n++] = '.', dst[n++] = '.';
        else
        {
            char a[2];
            u8 one[2] = {c, EOS};
            MP_GameToAscii(a, one, 1);
            dst[n++] = a[0];
        }
    }
    dst[n] = 0;
    (void)count;
}

// a message of Platinum's (its string slots set first), split into its pages
static void Say(const u8 *text)
{
    static u8 expanded[700];
    static char all[1400];
    char *p, *page;
    StringExpandPlaceholders(expanded, text);
    Utf8(all, sizeof(all), expanded, NULL, NULL);
    U.pageCount = 0;
    U.page = 0;
    U.typed = 0;
    for (page = all; page && *page && U.pageCount < 8; page = p)
    {
        p = strchr(page, '\f');
        if (p)
            *p++ = 0;
        while (*page == '\n')
            page++;
        if (*page)
        {
            snprintf(U.pages[U.pageCount], sizeof(U.pages[0]), "%s", page);
            MPNet_Log("sinnoh contest: \"%s\"", U.pages[U.pageCount]);
            U.pageCount++;
        }
    }
}

static void SayName(const char *name)
{
    Say(S3D_TextByName(name));
}

static bool8 Talking(void)
{
    return U.page < U.pageCount;
}

// A (or B) turns the page; FALSE once every page was read
static bool8 Read(u16 pressed)
{
    if (!Talking())
        return FALSE;
    if (U.typed < strlen(U.pages[U.page]))
    {
        U.typed += 2;
        if (pressed & (A_BUTTON | B_BUTTON))
            U.typed = strlen(U.pages[U.page]);
        return TRUE;
    }
    if (pressed & (A_BUTTON | B_BUTTON))
    {
        U.page++;
        U.typed = 0;
        PlaySE(SE_SELECT);
    }
    return Talking();
}

static void SetMonSlot(u8 slot, u8 id)
{
    StringCopy(Contest_Slot(slot), C.e[id].nick);
}

static void SetTrainerSlot(u8 slot, u8 id)
{
    StringCopy(Contest_Slot(slot), C.e[id].trainer);
}

static void SetJudgeSlot(u8 slot, u8 judge)
{
    StringCopy(Contest_Slot(slot), Contest_Text(CB_JUDGE_NAMES, Contest_JudgeName(judge)));
}

static void SetNumberSlot(u8 slot, s32 n)
{
    ConvertIntToDecimalStringN(Contest_Slot(slot), n < 0 ? -n : n, STR_CONV_MODE_LEFT_ALIGN, 3);
}

// ----------------------------------------------------------------------------------------------------- textures

static u32 Argb(u16 c)
{
    return 0xFF000000 | ((c & 31) * 255 / 31) << 16 | (((c >> 5) & 31) * 255 / 31) << 8 | (((c >> 10) & 31) * 255 / 31);
}

static SDL_Texture *ArtTexture(bool8 backdrop, u16 n)
{
    SDL_Texture **slot = backdrop ? &U.bd[n % 18] : &U.acc[n % 100];
    static u32 px[256 * 256];
    const u16 *src;
    u16 w, h;
    u32 i;
    if (*slot)
        return *slot;
    if (!S3D_Gfx_ContestPicture(backdrop, n, &w, &h, &src) || (u32)w * h > sizeof(px) / 4)
        return NULL;
    for (i = 0; i < (u32)w * h; i++)
        px[i] = (src[i] & 0x8000) || backdrop ? Argb(src[i]) : 0;
    *slot = SDL_CreateTexture(U.owner, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, w, h);
    if (*slot)
    {
        SDL_UpdateTexture(*slot, NULL, px, w * 4);
        SDL_SetTextureBlendMode(*slot, SDL_BLENDMODE_BLEND);
        SDL_SetTextureScaleMode(*slot, SDL_ScaleModeNearest);
    }
    return *slot;
}

static SDL_Texture *MonTexture(u8 k, const struct ContestEntrant *e)
{
    static u8 tiles[MON_PIC_SIZE * 4];
    static u16 pal[64];
    static u32 px[64 * 64];
    u32 key = e->species * 2654435761u ^ e->personality ^ e->otId * 40503u;
    int x, y;
    if (e->species == SPECIES_NONE || e->species >= NUM_SPECIES)
        return NULL;
    if (U.mon[k] && U.monKey[k] == key)
        return U.mon[k];
    if (U.mon[k])
        SDL_DestroyTexture(U.mon[k]);
    U.mon[k] = NULL;
    U.monKey[k] = key;
    HandleLoadSpecialPokePic(&gMonFrontPicTable[e->species], tiles, e->species, e->personality);
    LZ77UnCompWram(GetMonSpritePalFromSpeciesAndPersonality(e->species, e->otId, e->personality), pal);
    for (y = 0; y < 64; y++)
        for (x = 0; x < 64; x++)
        {
            u8 b = tiles[((y / 8) * 8 + x / 8) * 32 + (y % 8) * 4 + (x % 8) / 2];
            u8 v = (x & 1) ? b >> 4 : b & 15;
            px[y * 64 + x] = v ? Argb(pal[v]) : 0;
        }
    U.mon[k] = SDL_CreateTexture(U.owner, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, 64, 64);
    if (U.mon[k])
    {
        SDL_UpdateTexture(U.mon[k], NULL, px, 64 * 4);
        SDL_SetTextureBlendMode(U.mon[k], SDL_BLENDMODE_BLEND);
        SDL_SetTextureScaleMode(U.mon[k], SDL_ScaleModeNearest);
    }
    return U.mon[k];
}

static void Forget(void)
{
    memset(U.acc, 0, sizeof(U.acc));
    memset(U.bd, 0, sizeof(U.bd));
    memset(U.mon, 0, sizeof(U.mon));
    memset(U.monKey, 0, sizeof(U.monKey));
    U.canvas = NULL;
}

// ----------------------------------------------------------------------------------------------------- drawing

// a dressed POKéMON in a photo frame at x, y (hi-res pixels), w wide (the DS screen's 256 x 192 scaled)
static void DrawPhoto(u8 k, const struct ContestEntrant *e, int x, int y, int w, bool8 held)
{
    SDL_Renderer *r = G_R();
    float s = (float)w / PHOTO_W;
    int h = (int)(PHOTO_H * s);
    SDL_Rect dst = {x, y, w, h};
    SDL_Texture *t = ArtTexture(TRUE, e->backdrop >= 0 && e->backdrop < 18 ? e->backdrop : 0);
    u8 i;
    if (t)
        SDL_RenderCopy(r, t, NULL, &dst);
    else
    {
        SDL_SetRenderDrawColor(r, 0xf0, 0xe0, 0xc8, 255);
        SDL_RenderFillRect(r, &dst);
    }
    t = MonTexture(k, e);
    if (t)
    {
        SDL_Rect m = {x + (int)((MON_X - 40) * s), y + (int)((MON_Y - 40) * s), (int)(80 * s), (int)(80 * s)};
        SDL_RenderCopy(r, t, NULL, &m);
    }
    for (i = 0; i < e->wornCount; i++)
    {
        u16 aw, ah;
        const u16 *px;
        t = ArtTexture(FALSE, e->worn[i].accessory);
        if (t && S3D_Gfx_ContestPicture(FALSE, e->worn[i].accessory, &aw, &ah, &px))
        {
            SDL_Rect a = {x + (int)((e->worn[i].x - aw / 2) * s), y + (int)((e->worn[i].y - ah / 2) * s),
                          (int)(aw * s + 0.5f), (int)(ah * s + 0.5f)};
            SDL_RenderCopy(r, t, NULL, &a);
        }
    }
    if (held)
    {
        u16 aw, ah;
        const u16 *px;
        t = ArtTexture(FALSE, U.held);
        if (t && S3D_Gfx_ContestPicture(FALSE, U.held, &aw, &ah, &px))
        {
            SDL_Rect a = {x + (int)((U.hx - aw / 2) * s), y + (int)((U.hy - ah / 2) * s), (int)(aw * s + 0.5f),
                          (int)(ah * s + 0.5f)};
            SDL_SetTextureAlphaMod(t, (U.t / 8) & 1 ? 255 : 170);
            SDL_RenderCopy(r, t, NULL, &a);
            SDL_SetTextureAlphaMod(t, 255);
        }
    }
    SDL_SetRenderDrawColor(r, 0x40, 0x30, 0x28, 255);
    SDL_RenderDrawRect(r, &dst);
}

static void Hearts(int x, int y, u8 n, unsigned color)
{
    u8 i;
    for (i = 0; i < n; i++)
    {
        G_Circle(x + i * 7 + 2, y + 2, 2.2f, color, 255);
        G_Circle(x + i * 7 + 4, y + 2, 2.2f, color, 255);
        G_Fill(x + i * 7 + 1, y + 3, 5, 2, color);
        G_Fill(x + i * 7 + 2, y + 5, 3, 1, color);
    }
}

static void TextBox(void)
{
    char shown[160];
    if (!Talking())
        return;
    G_RoundFill(4, 120, 232, 36, 0x383848, 255);
    G_RoundFill(5, 121, 230, 34, 0xf8f8f0, 255);
    snprintf(shown, sizeof(shown), "%.*s", U.typed, U.pages[U.page]);
    G_Text(10, 123, 0x303030, G_NO_SHADOW, shown, 220, TF_NORMAL);
    if (U.typed >= strlen(U.pages[U.page]) && (U.t / 16) & 1)
        G_Arrow(224, 148, 1, 0x303030);
}

static const char *const sDirNames[4] = {"DanceCompetition_Text_Up", "DanceCompetition_Text_Front",
                                          "DanceCompetition_Text_Left", "DanceCompetition_Text_Right"};

static void Word(int x, int y, int align, unsigned color, const char *name)
{
    static u8 expanded[120];
    char s[120];
    StringExpandPlaceholders(expanded, S3D_TextByName(name));
    Utf8(s, sizeof(s), expanded, NULL, NULL);
    G_TextAlign(x, y, align, color, G_NO_SHADOW, s, TF_SMALL);
}

static void WordGame(int x, int y, int align, unsigned color, const u8 *text)
{
    char s[80];
    Utf8(s, sizeof(s), text, NULL, NULL);
    G_TextAlign(x, y, align, color, G_NO_SHADOW, s, TF_SMALL);
}

static void DrawDress(void)
{
    SDL_Renderer *r = G_R();
    u8 i;
    char s[48];
    G_Fill(0, 0, 240, 160, 0xd8c8e8);
    SDL_RenderSetScale(r, 1, 1);
    DrawPhoto(0, &C.e[0], 8, 8, 288, U.held != 0xFF);
    SDL_RenderSetScale(r, 2, 2);
    G_RoundFill(152, 4, 84, 112, 0x504060, 255);
    G_RoundFill(153, 5, 82, 110, 0xf8f0f8, 255);
    for (i = 0; i < 9 && U.top + i < U.listCount; i++)
    {
        u8 a = U.list[U.top + i];
        u8 name[40];
        Sinnoh_AccessoryName(name, a, FALSE);
        Utf8(s, 24, name, NULL, NULL);
        if (U.top + i == U.cursor)
            G_Fill(155, 7 + i * 12, 78, 11, 0xe0c8f0);
        G_Text(158, 7 + i * 12, 0x303030, G_NO_SHADOW, s, 0, TF_SMALL);
        snprintf(s, sizeof(s), "%d", Sinnoh_AccessoryCount(a));
        G_TextAlign(231, 7 + i * 12, TA_RIGHT, 0x605060, G_NO_SHADOW, s, TF_SMALL);
    }
    if (U.listCount == 0)
        G_Text(158, 9, 0x605060, G_NO_SHADOW, "-", 60, TF_SMALL);
    if (!U.studio)
    {
        snprintf(s, sizeof(s), "%u", (unsigned)(U.left + 59) / 60);
        G_RoundFill(4, 120, 60, 16, 0x504060, 255);
        G_TextAlign(34, 122, TA_CENTER, G_WHITE, G_NO_SHADOW, s, TF_NORMAL);
    }
    if (!Talking())
    {
        G_RoundFill(68, 120, 168, 36, 0x504060, 255);
        G_Text(72, 122, G_WHITE, G_NO_SHADOW, U.held != 0xFF ? "+  A" : "A  +  B  SELECT  L/R  START", 160, TF_SMALL);
    }
}

static void DrawVisualShow(void)
{
    SDL_Renderer *r = G_R();
    u8 k = U.shown < CONTEST_ENTRANTS ? CONTEST_ENTRANTS - 1 - U.shown : 0;
    const struct ContestEntrant *e = &C.e[k];
    G_Fill(0, 0, 240, 160, 0x302838);
    SDL_RenderSetScale(r, 1, 1);
    DrawPhoto(k, e, 112, 12, 256, FALSE);
    SDL_RenderSetScale(r, 2, 2);
    if (U.step >= 1)
        Hearts(60, 108, e->condHearts, 0xf06080);
    if (U.step >= 2)
        Hearts(60 + e->condHearts * 7 + 4, 108, e->dressHearts, 0xf0c040);
}

static void DrawStage(u8 highlight)
{
    SDL_Renderer *r = G_R();
    u8 i;
    G_Fill(0, 0, 240, 100, 0x283058);
    G_Fill(0, 72, 240, 48, 0x8a6a48);
    SDL_RenderSetScale(r, 1, 1);
    for (i = 0; i < CONTEST_ENTRANTS; i++)
    {
        SDL_Texture *t = MonTexture(i, &C.e[i]);
        int x = 20 + i * 112, y = 60;
        if (highlight == i)
            y -= 8;
        if (U.flash[i] > 0 && U.flashDir[i] == 0)
            y -= 6;
        if (U.flash[i] > 0 && U.flashDir[i] == 2)
            x -= 6;
        if (U.flash[i] > 0 && U.flashDir[i] == 3)
            x += 6;
        if (t)
        {
            SDL_Rect d = {x, y, 96, 96};
            SDL_RenderCopy(r, t, NULL, &d);
        }
    }
    SDL_RenderSetScale(r, 2, 2);
    for (i = 0; i < CONTEST_ENTRANTS; i++)
    {
        WordGame(34 + i * 56, 82, TA_CENTER, highlight == i ? 0xf8e070 : G_WHITE, C.e[i].nick);
        if (U.flash[i] > 0)
            Word(34 + i * 56, 6, TA_CENTER, U.flashGrade[i] <= 1 ? 0xf8e070 : U.flashGrade[i] <= 3 ? 0x90f090 : 0xc0c0c0,
                 U.flashGrade[i] <= 1 ? "DanceCompetition_Text_Excellent"
                 : U.flashGrade[i] <= 3 ? "DanceCompetition_Text_Good" : "DanceCompetition_Text_Miss");
    }
}

static void DrawDance(void)
{
    u8 i, lead = C.order[U.round];
    u32 now = U.t - U.partStart, measure = U.beat * 8 / 16;
    char s[16];
    DrawStage(lead);
    // the measure: eight beats, the lead's steps on it (the backups follow them in the second half)
    G_RoundFill(8, 100, 224, 18, 0x181828, 255);
    for (i = 0; i <= 16; i++)
        G_Fill(14 + i * 13, i % 2 ? 106 : 102, 1, i % 2 ? 6 : 14, 0x606080);
    for (i = 0; i < U.leadCount; i++)
        if (U.lead[i].made)
        {
            int x = 14 + U.lead[i].at * 13;
            G_Arrow(x - 3, 107, U.lead[i].dir == 0 ? 0 : U.lead[i].dir == 1 ? 1 : U.lead[i].dir, 0xf8d050);
        }
    G_Fill(14 + (int)(now * 208 / (measure ? measure : 1)), 100, 2, 18, U.part ? 0x70e070 : 0xf07070);
    snprintf(s, sizeof(s), "%d/%d", U.round < CONTEST_ENTRANTS ? U.round + 1 : CONTEST_ENTRANTS, CONTEST_ENTRANTS);
    G_TextAlign(232, 90, TA_RIGHT, G_WHITE, G_NO_SHADOW, s, TF_SMALL);
    if (!Talking())
    {
        u8 pattern;
        SetNumberSlot(0, Contest_Song(&pattern)->steps);
        if (lead == 0 && U.part == 0)
            Word(120, 122, TA_CENTER, 0xf8a0a0, "DanceCompetition_Text_PerformStepsAsMainDancer");
        else if (lead != 0 && U.part == 1)
            Word(120, 122, TA_CENTER, 0xa0c0f8, "DanceCompetition_Text_CopyPokemonBeforeYou");
        for (i = 0; i < 4; i++)
            Word(40 + i * 54, 140, TA_CENTER, G_WHITE, sDirNames[i]);
    }
}

static void DrawActing(void)
{
    u8 i;
    char s[48];
    DrawStage(U.scene == SC_ACT_PERFORM ? C.act.order[U.place] : 0xFF);
    for (i = 0; i < 3; i++)
    {
        G_RoundFill(8 + i * 78, 4, 70, 16, 0x181828, 255);
        G_Fill(10 + i * 78, 15, C.act.voltage[i] * 66 / 50, 3, i == 1 ? 0xf8a040 : 0x60c0f8);
        {
            u8 name[24];
            StringCopy(name, Contest_Text(CB_JUDGE_NAMES, Contest_JudgeName(i)));
            WordGame(43 + i * 78, 4, TA_CENTER, G_WHITE, name);
        }
    }
    if (U.scene == SC_ACT_MOVE || U.scene == SC_ACT_JUDGE)
    {
        G_RoundFill(8, 100, 224, 52, 0x303048, 255);
        for (i = 0; i < MAX_MON_MOVES; i++)
        {
            u16 m = C.e[0].moves[i];
            bool8 ok = Contest_MoveUsable(0, m) && Contest_MoveType(m) <= 4;
            if (m == MOVE_NONE)
                continue;
            if (U.scene == SC_ACT_MOVE && U.move == i)
                G_Fill(10, 102 + i * 12, 110, 11, 0x605890);
            WordGame(14, 102 + i * 12, TA_LEFT, ok ? G_WHITE : 0x808080, gMoveNames[m]);
            Hearts(78, 105 + i * 12, Contest_EffectAppeal(Contest_MoveEffect(m)) / 10, 0xf06080);
        }
        if (U.scene == SC_ACT_MOVE)
        {
            u16 l1, l2;
            u16 m = C.e[0].moves[U.move];
            Contest_EffectDescription(Contest_MoveEffect(m), &l1, &l2);
            {
                u8 both[200];
                StringCopy(both, Contest_Text(CB_EFFECTS, l1));
                StringAppend(both, gText_Space);
                StringAppend(both, Contest_Text(CB_EFFECTS, l2));
                Utf8(s, sizeof(s), both, NULL, NULL);
            }
            G_Text(124, 102, 0xe0e0f0, G_NO_SHADOW, s, 104, TF_SMALL);
        }
        else
            for (i = 0; i < 3; i++)
                G_Focus(8 + U.judge * 78, 4, 70, 16, 0xf8d038, U.t);
    }
    if (U.scene == SC_ACT_MOVE)
    {
        if (U.concedeAsk)
        {
            G_RoundFill(48, 60, 144, 30, 0x181828, 240);
            G_TextAlign(120, 62, TA_CENTER, G_WHITE, G_NO_SHADOW, "CONCEDE? Last place.", TF_SMALL);
            G_TextAlign(120, 75, TA_CENTER, 0xf8d038, G_NO_SHADOW, "A: YES     B: NO", TF_SMALL);
        }
        else
            G_TextAlign(8, 90, TA_LEFT, 0xc0c0d0, G_NO_SHADOW, "START: CONCEDE", TF_SMALL);
    }
    snprintf(s, sizeof(s), "%d/%d", C.act.turn < CONTEST_TURNS ? C.act.turn + 1 : CONTEST_TURNS, CONTEST_TURNS);
    G_TextAlign(232, 90, TA_RIGHT, G_WHITE, G_NO_SHADOW, s, TF_SMALL);
}

static void DrawResults(void)
{
    u8 i, r;
    static const unsigned sColor[3] = {0xf06080, 0x60c0f8, 0xf8c040};
    G_Fill(0, 0, 240, 160, 0x302838);
    for (i = 0; i < CONTEST_ENTRANTS; i++)
    {
        u8 id = CONTEST_ENTRANTS - 1 - i;
        int x = 52, w = 0;
        WordGame(6, 8 + i * 26, TA_LEFT, C.e[id].place == 0 && U.resultStep >= 4 ? 0xf8e070 : G_WHITE, C.e[id].nick);
        for (r = 0; r < ROUND_COUNT; r++)
        {
            if (!Contest_HasRound(r) || U.resultStep < r + 1)
                continue;
            w = C.e[id].round[r] * 180 / 100;
            G_Fill(x, 9 + i * 26, w, 8, sColor[r]);
            x += w;
        }
        if (U.resultStep >= 4)
        {
            char s[8];
            snprintf(s, sizeof(s), "%d", C.e[id].place + 1);
            G_TextAlign(234, 8 + i * 26, TA_RIGHT, G_WHITE, G_NO_SHADOW, s, TF_NORMAL);
        }
    }
}

static void Draw(void)
{
    SDL_Renderer *r = G_R();
    SDL_RenderSetScale(r, 2, 2);
    switch (U.scene)
    {
    case SC_DRESS:
        DrawDress();
        break;
    case SC_VISUAL_SHOW:
    case SC_INTRO_VISUAL:
        if (U.scene == SC_INTRO_VISUAL)
            G_Fill(0, 0, 240, 160, 0x302838);
        else
            DrawVisualShow();
        break;
    case SC_INTRO_DANCE:
    case SC_DANCE:
    case SC_DANCE_END:
        DrawDance();
        break;
    case SC_INTRO_ACTING:
    case SC_ACT_MOVE:
    case SC_ACT_JUDGE:
    case SC_ACT_PERFORM:
    case SC_ACT_TURN_END:
    case SC_ACT_END:
        DrawActing();
        break;
    case SC_RESULTS:
        DrawResults();
        break;
    case SC_PHOTO:
        G_Fill(0, 0, 240, 160, 0x201820);
        if (U.photo)
        {
            SDL_RenderSetScale(r, 1, 1);
            DrawPhoto(0, U.photo, 48, 16, 384, FALSE);
            SDL_RenderSetScale(r, 2, 2);
            if (U.photoTitle != 0xFFFF && U.photoTitle != 0)
            {
                u8 word[32];
                CopyEasyChatWord(word, U.photoTitle);
                WordGame(120, 140, TA_CENTER, G_WHITE, word);
            }
            else
                WordGame(120, 140, TA_CENTER, G_WHITE, U.photo->nick);
        }
        break;
    }
    TextBox();
    SDL_RenderSetScale(r, 1, 1);
}

SDL_Texture *Sinnoh_Contest_Canvas(SDL_Renderer *r)
{
    SDL_Texture *previous;
    SDL_Rect oldViewport, oldClip;
    SDL_bool clipped;
    float oldX, oldY;
    if (!U.active)
        return NULL;
    G_Begin(r);
    if (U.owner != r)
    {
        Forget();
        U.owner = r;
    }
    if (!U.canvas)
    {
        U.canvas = SDL_CreateTexture(r, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, CW, CH);
        if (!U.canvas)
            return NULL;
        SDL_SetTextureBlendMode(U.canvas, SDL_BLENDMODE_NONE);
        SDL_SetTextureScaleMode(U.canvas, SDL_ScaleModeLinear);
    }
    previous = SDL_GetRenderTarget(r);
    SDL_RenderGetViewport(r, &oldViewport);
    clipped = SDL_RenderIsClipEnabled(r);
    SDL_RenderGetClipRect(r, &oldClip);
    SDL_RenderGetScale(r, &oldX, &oldY);
    SDL_SetRenderTarget(r, U.canvas);
    SDL_RenderSetViewport(r, NULL);
    SDL_RenderSetClipRect(r, NULL);
    SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
    SDL_RenderClear(r);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    Draw();
    U.frameOk = SDL_RenderReadPixels(r, NULL, SDL_PIXELFORMAT_ABGR1555, U.bigFrame, CW * 2) == 0;
    SDL_SetRenderTarget(r, previous);
    SDL_RenderSetViewport(r, &oldViewport);
    SDL_RenderSetClipRect(r, clipped ? &oldClip : NULL);
    SDL_RenderSetScale(r, oldX, oldY);
    return U.canvas;
}

// the game picture while a contest runs (spectators, test shots): the canvas halved
const u16 *Sinnoh_Contest_Frame(void)
{
    static u16 small[240 * 160];
    int x, y;
    if (!U.active || !U.frameOk)
        return NULL;
    for (y = 0; y < 160; y++)
        for (x = 0; x < 240; x++)
            small[y * 240 + x] = U.bigFrame[(y * 2) * CW + x * 2] & 0x7FFF;
    return small;
}

// ----------------------------------------------------------------------------------------------------- the rounds

static void BuildList(void)
{
    u8 a;
    U.listCount = 0;
    for (a = 0; a < 100; a++)
        if (Sinnoh_AccessoryCount(a) > 0)
            U.list[U.listCount++] = a;
    U.cursor = U.top = 0;
}

static u8 Worn(u8 a)
{
    u8 i, n = 0;
    for (i = 0; i < C.e[0].wornCount; i++)
        n += C.e[0].worn[i].accessory == a;
    return n;
}

static void NextBackdrop(s8 d)
{
    u8 i;
    for (i = 0; i < 18; i++)
    {
        C.e[0].backdrop = (C.e[0].backdrop + d + 18) % 18;
        if (C.e[0].backdrop == 0 || Sinnoh_HasBackdrop(C.e[0].backdrop))
            return;
    }
}

static void StepDress(u16 keys, u16 pressed)
{
    if (Read(pressed))
        return;
    if (U.left && !U.studio)
        U.left--;
    if (U.held != 0xFF)
    {
        if (keys & DPAD_LEFT && U.hx > 8)
            U.hx -= 2;
        if (keys & DPAD_RIGHT && U.hx < PHOTO_W - 8)
            U.hx += 2;
        if (keys & DPAD_UP && U.hy > 8)
            U.hy -= 2;
        if (keys & DPAD_DOWN && U.hy < PHOTO_H - 8)
            U.hy += 2;
        if (pressed & A_BUTTON && C.e[0].wornCount < CONTEST_MAX_WORN)
        {
            struct ContestWorn *w = &C.e[0].worn[C.e[0].wornCount++];
            w->accessory = U.held;
            w->x = U.hx;
            w->y = U.hy;
            U.held = 0xFF;
            PlaySE(SE_SELECT);
        }
        else if (pressed & B_BUTTON)
            U.held = 0xFF;
    }
    else
    {
        if (pressed & DPAD_UP && U.cursor > 0)
            U.cursor--;
        if (pressed & DPAD_DOWN && U.cursor + 1 < U.listCount)
            U.cursor++;
        if (U.cursor < U.top)
            U.top = U.cursor;
        if (U.cursor >= U.top + 9)
            U.top = U.cursor - 8;
        if (pressed & A_BUTTON && U.listCount && Worn(U.list[U.cursor]) < Sinnoh_AccessoryCount(U.list[U.cursor]))
        {
            U.held = U.list[U.cursor];
            U.hx = MON_X;
            U.hy = MON_Y - 24;
            PlaySE(SE_SELECT);
        }
        if (pressed & (B_BUTTON | SELECT_BUTTON) && C.e[0].wornCount)
            C.e[0].wornCount--;
        else if (pressed & B_BUTTON && U.studio)
        {
            // (the studio: out again without a photo)
            Contest_StudioDone(FALSE);
            U.scene = SC_DONE;
            return;
        }
        if (pressed & L_BUTTON)
            NextBackdrop(-1);
        if (pressed & R_BUTTON)
            NextBackdrop(1);
    }
    if (U.studio && pressed & START_BUTTON)
    {
        // (the studio's photo: kept for JUBILIFE TV's gallery, its title asked for next by Platinum's script)
        U.held = 0xFF;
        Sinnoh_ContestKeepPhoto(PHOTO_STUDIO, &C.e[0]);
        Contest_StudioDone(TRUE);
        PlaySE(SE_SUCCESS);
        U.scene = SC_DONE;
        return;
    }
    if (!U.studio && (U.left == 0 || pressed & START_BUTTON))
    {
        U.held = 0xFF;
        Contest_ScoreVisual();
        U.scene = SC_VISUAL_SHOW;
        U.shown = 0;
        U.step = 0;
    }
}

static void StepVisualShow(u16 pressed)
{
    static const char *const sEntry[4] = {"VisualCompetition_Text_EntryNumberOne", "VisualCompetition_Text_EntryNumberTwo",
                                          "VisualCompetition_Text_EntryNumberThree", "VisualCompetition_Text_EntryNumberFour"};
    u8 k = CONTEST_ENTRANTS - 1 - U.shown;
    if (Read(pressed))
        return;
    if (U.step == 0)
    {
        SetTrainerSlot(0, k);
        SetMonSlot(1, k);
        SayName(sEntry[U.shown]);
        U.step = 1;
        return;
    }
    if (U.step == 1)
    {
        U.step = 2;
        U.t = 0;
        PlaySE(SE_SUCCESS);
        return;
    }
    if (U.step == 2 && U.t > 40)
    {
        U.shown++;
        U.step = 0;
        if (U.shown >= CONTEST_ENTRANTS)
        {
            u8 next = Contest_HasRound(ROUND_DANCE) ? SC_INTRO_DANCE : Contest_HasRound(ROUND_ACTING) ? SC_INTRO_ACTING : SC_RESULTS;
            if (next == SC_INTRO_DANCE)
                SayName("VisualCompetition_Text_DanceCompetitionIsNext");
            U.scene = next;
            U.step = 0;
        }
    }
}

// ---- DANCE

static u32 Seeded(void)
{
    return Contest_Random();
}

// the lead's steps (ov17_0224E990): places on the measure, nudged by its skill, directions
static void LeadSteps(u8 lead)
{
    u8 pattern, slots, taken[16], n = 0, i, k;
    const struct ContestSong *song = Contest_Song(&pattern);
    bool8 halves = C.rank >= 2;
    static const u8 sSkill[4] = {1, 2, 3, 4}, sSame[4] = {90, 40, 0, 0};
    u8 skill = C.e[lead].ai < 4 ? sSkill[C.e[lead].ai] : 1;
    memset(taken, 0, sizeof(taken));
    slots = halves ? 16 : 8;
    while (n < song->steps)
    {
        u8 v = Seeded() % slots;
        if (v == 0 || taken[v])
            continue;
        if (halves)
        {
            if ((v == slots - 1 && taken[v - 1]) || taken[v - 1] || (v + 1 < slots && taken[v + 1]))
                continue;
            if ((v & 1) && (Seeded() & 0xFF) < 128)
                continue;
        }
        taken[v] = 1;
        n++;
    }
    skill = C.rank == 0 ? skill * 2 : C.rank == 2 ? skill / 2 : C.rank == 3 ? skill / 3 : skill;
    U.leadCount = 0;
    for (k = 0; k < slots; k++)
        if (taken[k])
        {
            s16 wobble = (s16)(Seeded() % (2 + skill)) - (1 + skill / 2);   // (in 30ths of a second)
            U.lead[U.leadCount].at = halves ? k : k * 2;   // (the timeline's units: half beats)
            U.lead[U.leadCount].made = FALSE;
            U.lead[U.leadCount].grade = wobble < 0 ? -wobble : wobble;   // (how far off the beat it will step)
            U.lead[U.leadCount].dir = 0;
            U.leadCount++;
        }
    U.lead[0].dir = Seeded() % 4;
    for (i = 1; i < U.leadCount; i++)
        U.lead[i].dir = i < U.leadCount - 1 && sSame[C.rank] > Seeded() % 100 ? U.lead[i - 1].dir : Seeded() % 4;
}

static u32 HalfBeatFrames(u8 n)
{
    return (u32)U.beat * n / 32;   // (U.beat: a beat in 60ths, times 16)
}

static void Dancer(u8 who, u8 dir, u8 grade)
{
    U.flash[who] = 24;
    U.flashDir[who] = dir;
    U.flashGrade[who] = grade;
    C.e[who].dance += Contest_StepPoints(grade);
    if (grade <= 1)
        PlaySE(SE_SUCCESS);
    else if (grade <= 3)
        PlaySE(SE_SELECT);
}

static void StartPart(void)
{
    U.partStart = U.t;
    memset(U.copyCount, 0, sizeof(U.copyCount));
    U.playerSteps = 0;
}

static void StepDance(u16 pressed)
{
    u8 pattern, i, lead = C.order[U.round];
    const struct ContestSong *song = Contest_Song(&pattern);
    u32 now = U.t - U.partStart, measure = HalfBeatFrames(16);
    s8 dir = pressed & DPAD_UP ? 0 : pressed & DPAD_DOWN ? 1 : pressed & DPAD_LEFT ? 2 : pressed & DPAD_RIGHT ? 3 : -1;
    for (i = 0; i < CONTEST_ENTRANTS; i++)
        if (U.flash[i] > 0)
            U.flash[i]--;
    if (Read(pressed))
        return;
    if (U.part == 0)
    {
        // the lead dances: the computer's steps when their time comes, the player's when pressed
        if (lead == 0)
        {
            if (dir >= 0 && U.leadCount < song->steps && now < measure)
            {
                u32 half = HalfBeatFrames(1), at = (now + half / 2) / half;
                u32 dist = now > at * half ? now - at * half : at * half - now;
                u8 grade = Contest_JudgeStep(dist / 2, song->near, FALSE, FALSE, dir, 0);
                if (at >= 16)
                    at = 15;
                U.lead[U.leadCount].at = at;
                U.lead[U.leadCount].dir = dir;
                U.lead[U.leadCount].made = TRUE;
                U.leadCount++;
                Dancer(0, dir, grade);
            }
        }
        else
            for (i = 0; i < U.leadCount; i++)
                if (!U.lead[i].made && now >= HalfBeatFrames(U.lead[i].at))
                {
                    U.lead[i].made = TRUE;
                    Dancer(lead, U.lead[i].dir, Contest_JudgeStep(U.lead[i].grade, song->near, FALSE, FALSE, 0, 0));
                }
        if (now >= measure)
        {
            U.part = 1;
            StartPart();
        }
        return;
    }
    // the backups copy: the computer's with Platinum's wobble and missed directions, the player's when pressed
    for (i = 0; i < CONTEST_ENTRANTS; i++)
    {
        u8 c = U.copyCount[i];
        if (i == lead || c >= U.leadCount || !U.lead[c].made)
            continue;
        if (i == 0)
            continue;
        if (now >= HalfBeatFrames(U.lead[c].at))
        {
            static const u8 sSkill[4] = {1, 2, 3, 4};
            u8 skill = sSkill[C.e[i].ai & 3], wob, miss, d, grade;
            wob = C.rank <= 1 ? skill * 2 : C.rank == 2 ? skill + skill / 2 : skill;
            miss = wob + (c > 0 && U.lead[c].dir != U.lead[c - 1].dir ? 8 : 0);
            miss = C.rank <= 1 ? miss * 2 : C.rank == 2 ? miss + miss / 2 : miss;
            d = U.lead[c].dir;
            if (Seeded() % 100 < miss)
                while (d == U.lead[c].dir)
                    d = Seeded() % 4;
            grade = Contest_JudgeStep(Seeded() % (2 + wob) / 2, song->near, TRUE, TRUE, d, U.lead[c].dir);
            Dancer(i, d, grade);
            U.copyCount[i]++;
        }
    }
    if (lead != 0 && dir >= 0 && now < measure)
    {
        u32 half = HalfBeatFrames(1), at = (now + half / 2) / half;
        u32 dist = now > at * half ? now - at * half : at * half - now;
        u8 k, leadDir = 0;
        bool8 there = FALSE;
        for (k = 0; k < U.leadCount; k++)
            if (U.lead[k].made && U.lead[k].at == at)
                there = TRUE, leadDir = U.lead[k].dir;
        Dancer(0, dir, Contest_JudgeStep(dist / 2, song->near, TRUE, there, dir, leadDir));
    }
    if (now >= measure)
    {
        U.round++;
        U.part = 0;
        if (U.round >= CONTEST_ENTRANTS)
        {
            u8 best = 0;
            for (i = 1; i < CONTEST_ENTRANTS; i++)
                if (C.e[i].dance > C.e[best].dance)
                    best = i;
            SetMonSlot(0, best);
            SayName("DanceCompetition_Text_AnnounceLeadingMon");
            U.scene = SC_DANCE_END;
            m4aMPlayAllStop();
            return;
        }
        if (C.order[U.round] != 0)
            LeadSteps(C.order[U.round]);
        else
            U.leadCount = 0;
        StartPart();
    }
}

static void BeginDance(void)
{
    u8 pattern;
    const struct ContestSong *song = Contest_Song(&pattern);
    U.beat = 3600 * 16 / song->bpm;
    U.round = 0;
    U.part = 0;
    if (C.order[0] != 0)
        LeadSteps(C.order[0]);
    else
        U.leadCount = 0;
    StartPart();
    // Platinum's dance songs: the Master and Ultra ranks' by category (the user's Platinum rip: COOL, BEAUTY, CUTE,
    // SMART, TOUGH, the order of Contest_Song's songs 2-6); the lower ranks' aren't made yet (Gen 3's stands in)
    {
        static const u16 sCategorySongs[5] = {MUS_SINNOH_CONTEST_COOL, MUS_SINNOH_CONTEST_BEAUTY, MUS_SINNOH_CONTEST_CUTE,
                                              MUS_SINNOH_CONTEST_SMART, MUS_SINNOH_CONTEST_TOUGH};
        PlayBGM(C.rank >= 2 && C.type < 5 ? sCategorySongs[C.type] : MUS_CONTEST);
    }
    U.scene = SC_DANCE;
}

// ---- ACTING

// an effect message's string slots by Platinum's format (ov17_022413D8.c ov17_0224290C): the POKeMON it's about
// (who), another (other), its move, a number, its judge
static void EffectSlots(const struct ContestNote *n)
{
    u8 judge = C.act.judgeOf[n->who];
    u16 move = C.act.p[n->who].move;
    switch (n->format)
    {
    case 1: case 2: SetMonSlot(0, n->who); break;
    case 3: SetMonSlot(0, n->other); break;
    case 4: StringCopy(Contest_Slot(0), gMoveNames[move]); break;
    case 5: SetNumberSlot(0, n->number); break;
    case 6: SetJudgeSlot(0, judge); break;
    case 7: SetMonSlot(0, n->who); SetMonSlot(1, n->other); break;
    case 8: SetMonSlot(0, n->who); StringCopy(Contest_Slot(1), gMoveNames[move]); break;
    case 9: SetMonSlot(0, n->who); SetNumberSlot(1, n->number); break;
    case 10: SetMonSlot(0, n->who); SetJudgeSlot(1, judge); break;
    case 11: case 14: SetMonSlot(0, n->other); SetMonSlot(1, n->who); break;
    case 12: SetMonSlot(0, n->other); StringCopy(Contest_Slot(1), gMoveNames[move]); break;
    case 13: SetMonSlot(0, n->other); SetNumberSlot(1, n->number); break;
    case 15: case 18: StringCopy(Contest_Slot(0), gMoveNames[move]); SetMonSlot(1, n->who); break;
    case 16: StringCopy(Contest_Slot(0), gMoveNames[move]); SetMonSlot(1, n->other); break;
    case 17: StringCopy(Contest_Slot(0), gMoveNames[move]); SetNumberSlot(1, n->number); break;
    case 19: case 22: SetNumberSlot(0, n->number); SetMonSlot(1, n->who); break;
    case 20: SetNumberSlot(0, n->number); SetMonSlot(1, n->other); break;
    case 21: SetNumberSlot(0, n->number); StringCopy(Contest_Slot(1), gMoveNames[move]); break;
    case 23: SetJudgeSlot(0, judge); SetMonSlot(1, n->who); break;
    case 24: SetJudgeSlot(0, judge); SetMonSlot(1, n->other); break;
    case 25: SetJudgeSlot(0, judge); StringCopy(Contest_Slot(1), gMoveNames[move]); break;
    case 26: SetJudgeSlot(0, judge); SetNumberSlot(1, n->number); break;
    case 27: SetMonSlot(0, n->who); SetJudgeSlot(1, judge); StringCopy(Contest_Slot(2), gMoveNames[move]); break;
    default: SetMonSlot(0, n->who); break;
    }
}

static void ShowNote(void)
{
    const struct ContestNote *n = &C.act.note[U.noteShown];
    static const char *const sUp[5] = {"Contest_Text_CoolMovePerformedVoltageUp", "Contest_Text_BeautyMovePerformedVoltageUp",
                                       "Contest_Text_CuteMovePerformedVoltageUp", "Contest_Text_SmartMovePerformedVoltageUp",
                                       "Contest_Text_ToughMovePerformedVoltageUp"};
    static const char *const sDown[5] = {"Contest_Text_CoolMovePerformedVoltageDown",
                                         "Contest_Text_BeautyMovePerformedVoltageDown",
                                         "Contest_Text_CuteMovePerformedVoltageDown",
                                         "Contest_Text_SmartMovePerformedVoltageDown",
                                         "Contest_Text_ToughMovePerformedVoltageDown"};
    static const char *const sImpressed[5] = {"", "Contest_Text_JudgeWasVeryImpressed", "Contest_Text_JudgeWasStronglyImpressed",
                                              "Contest_Text_JudgeWasImpressed", "Contest_Text_EveryonePerformedForSameJudge"};
    static const char *const sAttention[5] = {"Contest_Text_MonDidntStandOut", "Contest_Text_MonAttractedALittleAttention",
                                              "Contest_Text_MonAttractedDecentAttention",
                                              "Contest_Text_MonAttractedALotOfAttention",
                                              "Contest_Text_MonCommandedTotalAttention"};
    switch (n->kind)
    {
    case NOTE_PERFORMED:
        // ("To {judge, slot 1}, {POKeMON, slot 0} performed {move, slot 2}!")
        SetMonSlot(0, n->who);
        SetJudgeSlot(1, n->other);
        StringCopy(Contest_Slot(2), gMoveNames[C.act.p[n->who].move]);
        SayName("Contest_Text_ToJudgeMonPerformedMove");
        break;
    case NOTE_SAME_JUDGE:
        SayName("Contest_Text_SomeoneAlreadyPerformedForJudge");
        break;
    case NOTE_APPEAL:
        SetMonSlot(0, n->who);
        SayName(sAttention[n->number > 4 ? 4 : n->number < 0 ? 0 : n->number]);
        break;
    case NOTE_EFFECT:
        EffectSlots(n);
        Say(Contest_Text(CB_ACTING, n->msg));
        break;
    case NOTE_VOLTAGE_UP:
        SetJudgeSlot(0, n->other);
        SayName(sUp[n->number % 5]);
        break;
    case NOTE_VOLTAGE_DOWN:
        SetJudgeSlot(0, n->other);
        SayName(sDown[n->number % 5]);
        break;
    case NOTE_EXCITED:
        SayName("Contest_Text_AudienceIsGettingPrettyExcited");
        break;
    case NOTE_WILD:
        SetNumberSlot(0, n->number);
        SayName("Contest_Text_AudienceIsGoingWild");
        break;
    case NOTE_IMPRESSED:
    {
        u8 k, slot = 1;
        SetJudgeSlot(0, n->other);
        for (k = 0; k < CONTEST_ENTRANTS && slot < 5; k++)
            if (C.act.judgeOf[C.act.order[k]] == n->other)
                SetMonSlot(slot++, C.act.order[k]);
        SayName(sImpressed[n->number > 4 ? 4 : n->number]);
        break;
    }
    }
    U.noteShown++;
}

static void ChooseAll(void)
{
    u8 i;
    for (i = 1; i < CONTEST_ENTRANTS; i++)
    {
        u16 move;
        u8 judge;
        Contest_AiChoose(i, &move, &judge);
        Contest_Choose(i, move, judge);
    }
    Contest_Choose(0, C.e[0].moves[U.move], U.judge);
    U.place = 0;
    U.noteShown = 0;
    C.act.notes = 0;
    U.scene = SC_ACT_PERFORM;
    U.step = 0;
}

static void StepActing(u16 pressed)
{
    if (Read(pressed))
        return;
    switch (U.scene)
    {
    case SC_ACT_MOVE:
        // Moemon Co-op: CONCEDE (START): the rest is skipped and the player takes last place
        if (U.concedeAsk)
        {
            if (pressed & A_BUTTON)
            {
                PlaySE(SE_SELECT);
                U.concedeAsk = FALSE;
                C.conceded = TRUE;
                MPNet_Log("sinnoh contest: the player conceded (turn %d)", C.act.turn + 1);
                C.act.turn = CONTEST_TURNS;
                for (pressed = 0; pressed < CONTEST_ENTRANTS; pressed++)
                    C.e[pressed].acting = C.act.total[pressed];
                U.scene = SC_RESULTS;
                U.resultStep = 0;
            }
            else if (pressed & (B_BUTTON | START_BUTTON))
            {
                PlaySE(SE_SELECT);
                U.concedeAsk = FALSE;
            }
            break;
        }
        if (pressed & START_BUTTON)
        {
            PlaySE(SE_SELECT);
            U.concedeAsk = TRUE;
            MPNet_Log("sinnoh contest: CONCEDE?");
            break;
        }
        if (pressed & DPAD_UP && U.move > 0)
            U.move--;
        if (pressed & DPAD_DOWN && U.move < MAX_MON_MOVES - 1 && C.e[0].moves[U.move + 1] != MOVE_NONE)
            U.move++;
        if (pressed & A_BUTTON && Contest_MoveUsable(0, C.e[0].moves[U.move]) && Contest_MoveType(C.e[0].moves[U.move]) <= 4)
        {
            PlaySE(SE_SELECT);
            SayName("Contest_Text_ChooseJudge");
            U.scene = SC_ACT_JUDGE;
        }
        break;
    case SC_ACT_JUDGE:
        if (pressed & DPAD_LEFT && U.judge > 0)
            U.judge--;
        if (pressed & DPAD_RIGHT && U.judge < 2)
            U.judge++;
        if (pressed & B_BUTTON)
            U.scene = SC_ACT_MOVE;
        else if (pressed & A_BUTTON)
        {
            PlaySE(SE_SELECT);
            ChooseAll();
        }
        break;
    case SC_ACT_PERFORM:
        if (U.noteShown < C.act.notes)
        {
            ShowNote();
            break;
        }
        if (U.step == 0)
        {
            Contest_Perform(U.place);
            U.noteShown = 0;
            U.step = 1;
            break;
        }
        U.step = 0;
        if (++U.place >= CONTEST_ENTRANTS)
        {
            Contest_EndTurn();
            U.noteShown = 0;
            U.scene = SC_ACT_TURN_END;
        }
        break;
    case SC_ACT_TURN_END:
        if (U.noteShown < C.act.notes)
        {
            ShowNote();
            break;
        }
        if (C.act.turn < CONTEST_TURNS - 1)
            SayName("Contest_Text_NextTurnLowestScoringPerformFirst");
        Contest_NextTurn();
        if (C.act.turn >= CONTEST_TURNS)
        {
            u8 i;
            for (i = 0; i < CONTEST_ENTRANTS; i++)
                C.e[i].acting = C.act.total[i];
            SayName("Contest_Text_JudgingIsOver");
            U.scene = SC_ACT_END;
        }
        else
        {
            SetNumberSlot(0, C.act.turn + 1);
            SayName(C.act.turn == CONTEST_TURNS - 1 ? "Contest_Text_LastPerformanceChooseMove" : "Contest_Text_WhichMoveIsYourChoice");
            U.move = 0;
            while (U.move < MAX_MON_MOVES - 1 && !Contest_MoveUsable(0, C.e[0].moves[U.move]))
                U.move++;
            U.scene = SC_ACT_MOVE;
        }
        break;
    case SC_ACT_END:
        U.scene = SC_RESULTS;
        U.resultStep = 0;
        break;
    }
}

static void BeginActing(void)
{
    static const char *const sTime[5] = {"Contest_Text_TimeForCoolActingCompetition",
                                         "Contest_Text_TimeForBeautyActingCompetition",
                                         "Contest_Text_TimeForCuteActingCompetition",
                                         "Contest_Text_TimeForSmartActingCompetition",
                                         "Contest_Text_TimeForToughActingCompetition"};
    Contest_BeginActing();
    MPNet_Log("sinnoh contest: acting begins");
    SayName(Contest_IsPractice() ? "Contest_Text_BeginPracticeActingCompetition" : sTime[C.type]);
    U.move = 0;
    U.judge = 1;
    U.scene = SC_ACT_MOVE;
}

static void StepResults(u16 pressed)
{
    static const char *const sRound[3] = {"ContestResults_Text_VisualCompetitionResults",
                                          "ContestResults_Text_DanceCompetitionResults",
                                          "ContestResults_Text_ActingCompetitionResults"};
    if (Read(pressed))
        return;
    if (U.resultStep == 0)
    {
        Contest_ScoreAll();
        PlayBGM(MUS_CONTEST_RESULTS);
        SayName("ContestResults_Text_JudgingCompleted");
        U.resultStep = 1;
        return;
    }
    while (U.resultStep <= 3 && !Contest_HasRound(U.resultStep - 1))
        U.resultStep++;
    if (U.resultStep <= 3)
    {
        SayName(sRound[U.resultStep - 1]);
        U.resultStep++;
        return;
    }
    if (U.resultStep == 4)
    {
        u8 w = C.ranking[0];
        SetNumberSlot(0, CONTEST_ENTRANTS - w);
        SetTrainerSlot(1, w);
        SetMonSlot(2, w);
        SayName("ContestResults_Text_TheWinnerIs");
        PlayFanfare(MUS_CONTEST_WINNER);
        U.resultStep = 5;
        return;
    }
    U.scene = SC_DONE;
}

// ----------------------------------------------------------------------------------------------------- running

static void Close(void)
{
    U.active = FALSE;
    U.scene = SC_NONE;
    if (U.enableScript)
        ScriptContext_Enable();
    U.enableScript = FALSE;
}

void Contest_RunScenes(void)
{
    u8 i;
    if (!C.made)
        return;
    U.studio = FALSE;
    memset(U.copyCount, 0, sizeof(U.copyCount));
    U.active = TRUE;
    U.enableScript = TRUE;
    U.prevKeys = 0xFFFF;
    U.concedeAsk = FALSE;
    U.t = 0;
    U.pageCount = U.page = 0;
    U.held = 0xFF;
    for (i = 0; i < CONTEST_ENTRANTS; i++)
        C.e[i].dance = C.e[i].acting = C.e[i].visual = 0;
    PlayBGM(MUS_CONTEST_LOBBY);
    if (Contest_HasRound(ROUND_VISUAL))
    {
        SayName(Contest_IsPractice() ? "VisualCompetition_Text_PracticeSession" : "VisualCompetition_Text_LetsBeginTheVisualCompetition");
        U.scene = SC_INTRO_VISUAL;
    }
    else if (Contest_HasRound(ROUND_DANCE))
    {
        SayName(Contest_IsPractice() ? "DanceCompetition_Text_ThanksForJoiningPracticeContest" : "DanceCompetition_Text_CompetitionIsReady");
        U.scene = SC_INTRO_DANCE;
    }
    else
        BeginActing();
    MPNet_Log("sinnoh contest: the screens open (scene %d)", U.scene);
}

bool8 Contest_RunStudio(u8 partySlot)
{
    if (partySlot >= PARTY_SIZE)
        return FALSE;
    Contest_StudioEntrant(partySlot);
    U.active = TRUE;
    U.enableScript = TRUE;
    U.studio = TRUE;
    U.prevKeys = 0xFFFF;
    U.t = 0;
    U.pageCount = U.page = 0;
    BuildList();
    U.held = 0xFF;
    U.left = 1;
    U.scene = SC_DRESS;
    MPNet_Log("sinnoh contest: the dress-up studio opens");
    return TRUE;
}

bool8 Contest_ShowPhoto(u8 type)
{
    U.studio = type == PHOTO_STUDIO;
    U.photoTitle = type == PHOTO_STUDIO ? *Sinnoh_PhotoTitle(PHOTO_STUDIO) : 0xFFFF;
    U.photo = Sinnoh_ContestPhoto(type);
    if (!U.photo)
        return FALSE;
    U.active = TRUE;
    U.enableScript = TRUE;
    U.prevKeys = 0xFFFF;
    U.scene = SC_PHOTO;
    U.pageCount = U.page = 0;
    return TRUE;
}

bool8 Sinnoh_Contest_IsActive(void)
{
    return U.active;
}

static void StepScene(u16 keys, u16 pressed);

// Moemon Co-op: the host's ANIM SPEED hurries the contest's own scenes (words, the acting, the results) by an extra
// step on some frames, with no buttons on it; the dance and the dressing keep the music's time
bool8 Sinnoh_Contest_Step(u16 keys)
{
    static u8 sAcc;
    u16 pressed;
    if (!U.active)
        return FALSE;
    pressed = keys & ~U.prevKeys;
    U.prevKeys = keys;
    StepScene(keys, pressed);
    if (U.active && U.scene != SC_DRESS && U.scene != SC_DANCE && U.scene != SC_PHOTO && U.scene != SC_DONE)
    {
        sAcc += MP_BattleAnimSpeed() / 5;   // (extra steps per 20 frames)
        if (sAcc >= 20)
        {
            sAcc -= 20;
            StepScene(keys, 0);
        }
    }
    return TRUE;
}

static void StepScene(u16 keys, u16 pressed)
{
    U.t++;
    switch (U.scene)
    {
    case SC_INTRO_VISUAL:
        if (!Read(pressed))
        {
            BuildList();
            U.left = DRESS_FRAMES;
            U.held = 0xFF;
            C.e[0].wornCount = 0;
            C.e[0].backdrop = 0;
            U.scene = SC_DRESS;
        }
        break;
    case SC_DRESS:
        StepDress(keys, pressed);
        break;
    case SC_VISUAL_SHOW:
        StepVisualShow(pressed);
        break;
    case SC_INTRO_DANCE:
        if (!Read(pressed))
            BeginDance();
        break;
    case SC_DANCE:
        StepDance(pressed);
        break;
    case SC_DANCE_END:
        if (!Read(pressed))
        {
            if (Contest_HasRound(ROUND_ACTING))
                BeginActing();
            else
                U.scene = SC_RESULTS, U.resultStep = 0;
        }
        break;
    case SC_INTRO_ACTING:
    case SC_ACT_MOVE:
    case SC_ACT_JUDGE:
    case SC_ACT_PERFORM:
    case SC_ACT_TURN_END:
    case SC_ACT_END:
        StepActing(pressed);
        break;
    case SC_RESULTS:
        StepResults(pressed);
        break;
    case SC_PHOTO:
        if (pressed & (A_BUTTON | B_BUTTON))
            Close();
        break;
    case SC_DONE:
        if (!C.scored && !U.studio)
            Contest_ScoreAll();
        Close();
        break;
    }
}
