// Moemon Co-op: the TRAINER CARD's JOHTO and SINNOH cards, HeartGold's and Platinum's own (sinnoh/cards.pak from
// tools/sinnoh/build_cards.py: front, back and their six colors by stars), drawn at 480x320 over the game's card
// (trainer_card.c opens them for those regions; HOENN keeps Emerald's card, KANTO FireRed's). The card's text is the
// game's font; the picture is the one on the game's card (its window's pixels: every LOOK as it's drawn there). The
// back shows the region's badges as the BADGE CASE does and the card's team. A flips it, L / R the region, SELECT the
// BADGE CASE, B closes the card. Also MY CARD (START on the player's own card): the card's team, its title and
// whether the title shows.
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include "global.h"
#include "main.h"
#include "sound.h"
#include "constants/songs.h"
#include "pokemon_icon.h"
#include "graphics.h"
#include "mp.h"
#include "mp_region.h"
#include "mp_tcard.h"
#include "../tcg/tcg_gfx.h"

#define RC_W 480
#define RC_H 320

const char *Updater_ExeFile(const char *name);
void MP_BadgeCase_DrawBadgeFor(SDL_Renderer *r, const struct MpCardView *v, u8 region, u8 badge, int x, int y, int size);
void MP_BadgeCase_OpenView(const struct MpCardView *view, u8 region);
bool8 MP_BadgeCase_IsActive(void);
bool8 Platform_PoketchMouse(s32 *x, s32 *y, bool8 *down);
bool8 MP_TrainerCard_RegionShown(u8 region, const struct MpCardView *friendView);

enum { KIND_HGSS, KIND_PLATINUM, KIND_COUNT };

struct CardArt
{
    u16 w, h;
    u8 npal;
    const u8 *side[2];
    const u32 *pal;             // npal * 256
};

// Where the details go on each card (its own pixels: the stripes of the art)
struct CardLayout
{
    s16 idX, idY, nameX, nameY;           // front: the top row
    s16 rowX, rowW, rowY[3];              // front: MONEY, the dex, BADGES
    s16 picX, picY;                       // front: the picture's top left (64x64)
    s16 lowX, lowW, lowY[2];              // front: TIME, the title
    s16 backX, backW, backY[5];           // back: its rows
    s16 panelX, panelY, panelW;           // back: the white panel (badges, then the team)
};

static const struct CardLayout sLayouts[KIND_COUNT] = {
    [KIND_HGSS] = {
        .idX = 8, .idY = 24, .nameX = 128, .nameY = 24,
        .rowX = 8, .rowW = 150, .rowY = {48, 72, 96},
        .picX = 168, .picY = 49,
        .lowX = 8, .lowW = 238, .lowY = {128, 144},
        .backX = 8, .backW = 238, .backY = {8, 24, 48, 64, 80},
        .panelX = 32, .panelY = 98, .panelW = 188,
    },
    [KIND_PLATINUM] = {
        .idX = 8, .idY = 25, .nameX = 8, .nameY = 41,
        .rowX = 8, .rowW = 136, .rowY = {65, 89, 113},
        .picX = 160, .picY = 48,
        .lowX = 8, .lowW = 226, .lowY = {137, 153},
        .backX = 8, .backW = 226, .backY = {9, 25, 49, 65, 81},
        .panelX = 28, .panelY = 108, .panelW = 184,
    },
};

static const char *const sRegionNames[REGION_COUNT] = {"HOENN", "JOHTO", "KANTO", "SINNOH"};

static struct
{
    bool8 active, back, myCard;
    bool8 draining;             // closed: the game waits until the buttons are let go (they were this card's)
    u8 region, result;
    u8 flip;                    // frames left of the flip (0: none)
    u8 myRow;
    u32 age;
    u16 prevKeys;
    bool8 mouseDown;
    struct MpCardView view;
    bool8 loaded, tried;
    u8 *pak;
    struct CardArt art[KIND_COUNT];
    SDL_Renderer *owner;
    SDL_Texture *canvas, *side, *pic;
    SDL_Texture *card[KIND_COUNT][2][6];
    SDL_Texture *icon[PARTY_SIZE];
    u16 iconSpecies[PARTY_SIZE];
} R;

static u8 KindOf(u8 region) { return region == REGION_SINNOH ? KIND_PLATINUM : KIND_HGSS; }

// ----------------------------------------------------------------------------------------------------- the art

static void LoadPak(void)
{
    const char *path = getenv("MOEMON_CARDS_PAK");
    FILE *f;
    long size;
    const u8 *p, *end;
    u8 k;
    R.tried = TRUE;
    if (path == NULL)
        path = Updater_ExeFile("sinnoh/cards.pak");
    f = fopen(path, "rb");
    if (f == NULL)
        return;
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);
    R.pak = malloc(size);
    if (R.pak == NULL || fread(R.pak, 1, size, f) != (size_t)size || size < 6 || memcmp(R.pak, "MCRD", 4) != 0
     || R.pak[4] != 1 || R.pak[5] < KIND_COUNT)
    {
        fclose(f);
        free(R.pak);
        R.pak = NULL;
        return;
    }
    fclose(f);
    p = R.pak + 6;
    end = R.pak + size;
    for (k = 0; k < KIND_COUNT; k++)
    {
        struct CardArt *a = &R.art[k];
        if (p + 5 > end)
            return;
        a->w = p[0] | (p[1] << 8);
        a->h = p[2] | (p[3] << 8);
        a->npal = p[4];
        p += 5;
        if (p + 2 * a->w * a->h + a->npal * 1024 > end || a->npal == 0)
            return;
        a->side[0] = p;
        a->side[1] = p + a->w * a->h;
        p += 2 * a->w * a->h;
        a->pal = (const u32 *)p;
        p += a->npal * 1024;
    }
    R.loaded = TRUE;
}

// a side of a card in a color, at twice its pixels (kept sharp when it's drawn smaller or larger)
static SDL_Texture *CardTexture(SDL_Renderer *r, u8 kind, u8 back, u8 color)
{
    const struct CardArt *a = &R.art[kind];
    SDL_Texture *t;
    u32 *px;
    int x, y;
    if (color >= a->npal)
        color = a->npal - 1;
    if (color > 5)
        color = 5;
    if (R.card[kind][back][color])
        return R.card[kind][back][color];
    px = malloc(a->w * a->h * 16);
    if (px == NULL)
        return NULL;
    for (y = 0; y < a->h * 2; y++)
        for (x = 0; x < a->w * 2; x++)
            px[y * a->w * 2 + x] = a->pal[color * 256 + a->side[back][(y / 2) * a->w + x / 2]];
    t = SDL_CreateTexture(r, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, a->w * 2, a->h * 2);
    if (t)
    {
        SDL_UpdateTexture(t, NULL, px, a->w * 8);
        SDL_SetTextureBlendMode(t, SDL_BLENDMODE_BLEND);
        SDL_SetTextureScaleMode(t, SDL_ScaleModeLinear);
    }
    free(px);
    R.card[kind][back][color] = t;
    return t;
}

static u32 Rgb555(u16 c)
{
    return 0xFF000000 | ((c & 31) * 255 / 31) << 16 | (((c >> 5) & 31) * 255 / 31) << 8 | (((c >> 10) & 31) * 255 / 31);
}

// a POKéMON's menu icon (its first frame: 4x4 tiles of 4 bits, the icon palette it names)
static SDL_Texture *IconTexture(SDL_Renderer *r, u8 slot, u16 species)
{
    static u32 px[32 * 32];
    const u8 *tiles;
    const u16 *pal;
    int t, i;
    if (R.icon[slot] && R.iconSpecies[slot] == species)
        return R.icon[slot];
    if (R.icon[slot])
        SDL_DestroyTexture(R.icon[slot]);
    R.icon[slot] = NULL;
    tiles = GetMonIconTiles(species, 0);
    pal = gMonIconPalettes[GetMonIconPaletteIndexFromSpecies(species)];
    if (tiles == NULL || pal == NULL)
        return NULL;
    for (t = 0; t < 16; t++)
        for (i = 0; i < 64; i++)
        {
            u8 b = tiles[t * 32 + i / 2], v = (i & 1) ? b >> 4 : b & 15;
            px[((t / 4) * 8 + i / 8) * 32 + (t % 4) * 8 + i % 8] = v ? Rgb555(pal[v]) : 0;
        }
    R.icon[slot] = SDL_CreateTexture(r, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, 32, 32);
    if (R.icon[slot])
    {
        SDL_UpdateTexture(R.icon[slot], NULL, px, 32 * 4);
        SDL_SetTextureBlendMode(R.icon[slot], SDL_BLENDMODE_BLEND);
    }
    R.iconSpecies[slot] = species;
    return R.icon[slot];
}

static void ForgetTextures(void)
{
    memset(R.card, 0, sizeof(R.card));
    memset(R.icon, 0, sizeof(R.icon));
    memset(R.iconSpecies, 0, sizeof(R.iconSpecies));
    R.canvas = R.side = R.pic = NULL;
}

// ----------------------------------------------------------------------------------------------------- drawing

static float sScale;   // the card's pixels -> the picture's
static int sOx, sOy;

// text at a card position (the game's font, in the GBA's pixels: half the picture's)
static void CardText(int cx, int cy, int align, const char *text, unsigned color)
{
    int gx = (int)((sOx + cx * sScale) / 2), gy = (int)((sOy + cy * sScale) / 2);
    G_TextAlign(gx, gy, align, color, 0xd8d8d0, text, TF_NORMAL);
}

static void Row(int x, int y, int w, const char *label, const char *value)
{
    CardText(x + 4, y, TA_LEFT, label, G_INK);
    if (value)
        CardText(x + w - 4, y, TA_RIGHT, value, G_INK);
}

static void DrawFront(SDL_Renderer *r, const struct CardLayout *L)
{
    const struct MpCardView *v = &R.view;
    char buf[48];
    u8 b, got = 0;
    snprintf(buf, sizeof(buf), "IDNo. %05u", v->trainerId);
    CardText(L->idX + 4, L->idY, TA_LEFT, buf, G_INK);
    snprintf(buf, sizeof(buf), "NAME: %s", v->name);
    CardText(L->nameX + 4, L->nameY, TA_LEFT, buf, G_INK);
    snprintf(buf, sizeof(buf), "\xA5%u", (unsigned)v->money);
    Row(L->rowX, L->rowY[0], L->rowW, "MONEY", buf);
    snprintf(buf, sizeof(buf), "%u", v->dex);
    Row(L->rowX, L->rowY[1], L->rowW, "MO\xc3\xa9" "DEX", buf);
    for (b = 0; b < TCARD_BADGES_PER_REGION; b++)
        got += MP_CardView_HasBadge(v, R.region, b);
    snprintf(buf, sizeof(buf), "%u", got);
    Row(L->rowX, L->rowY[2], L->rowW, "BADGES", buf);
    snprintf(buf, sizeof(buf), "%u:%02u", v->hours, v->minutes);
    Row(L->lowX, L->lowY[0], L->lowW, "TIME", buf);
    if (v->title[0])
        CardText(L->lowX + L->lowW / 2, L->lowY[1], TA_CENTER, v->title, 0x305898);
    // the picture
    if (v->hasPic)
    {
        SDL_Rect dst;
        if (!R.pic)
        {
            R.pic = SDL_CreateTexture(r, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, 64, 64);
            if (R.pic)
            {
                SDL_UpdateTexture(R.pic, NULL, v->pic, 64 * 4);
                SDL_SetTextureBlendMode(R.pic, SDL_BLENDMODE_BLEND);
                SDL_SetTextureScaleMode(R.pic, SDL_ScaleModeNearest);
            }
        }
        if (R.pic)
        {
            SDL_RenderSetScale(r, 1, 1);
            dst.x = sOx + (int)(L->picX * sScale);
            dst.y = sOy + (int)(L->picY * sScale);
            dst.w = dst.h = 128;   // (two of the picture's pixels a GBA pixel, like the game's card)
            SDL_RenderCopy(r, R.pic, NULL, &dst);
            SDL_RenderSetScale(r, 2, 2);
        }
    }
}

static void DrawBack(SDL_Renderer *r, const struct CardLayout *L)
{
    const struct MpCardView *v = &R.view;
    char buf[48];
    u8 b, shiny = 0, i;
    Row(L->backX, L->backY[0], L->backW, "HALL OF FAME", (v->cleared >> R.region) & 1 ? "CHAMPION!" : "-");
    snprintf(buf, sizeof(buf), "%u WINS  %u LOSSES", v->linkWins, v->linkLosses);
    Row(L->backX, L->backY[1], L->backW, "LINK BATTLES", buf);
    snprintf(buf, sizeof(buf), "%u", v->trades);
    Row(L->backX, L->backY[2], L->backW, "TRADES", buf);
    for (b = 0; b < TCARD_BADGES_PER_REGION; b++)
        if (MP_CardView_HasBadge(v, R.region, b) && v->polish[R.region * TCARD_BADGES_PER_REGION + b] >= TCARD_POLISH_2_SPARKLES)
            shiny++;
    snprintf(buf, sizeof(buf), "%u / 8", shiny);
    Row(L->backX, L->backY[3], L->backW, "SHINING BADGES", buf);
    Row(L->backX, L->backY[4], L->backW, v->own ? "MY CARD TEAM" : "CARD TEAM", NULL);
    // the white panel: the region's badges, then the team
    SDL_RenderSetScale(r, 1, 1);
    {
        int px = sOx + (int)(L->panelX * sScale), py = sOy + (int)(L->panelY * sScale), pw = (int)(L->panelW * sScale);
        int size = pw / 8 - 4 > 44 ? 44 : pw / 8 - 4, step = pw / 8;
        for (b = 0; b < TCARD_BADGES_PER_REGION; b++)
            MP_BadgeCase_DrawBadgeFor(r, v, R.region, b, px + b * step + (step - size) / 2, py, size);
        G_Begin(r);
        step = pw / PARTY_SIZE;
        for (i = 0; i < v->teamCount && i < PARTY_SIZE; i++)
        {
            SDL_Texture *t = IconTexture(r, i, v->team[i]);
            SDL_Rect dst = {px + i * step + (step - 56) / 2, py + size + 4, 56, 56};
            if (t)
                SDL_RenderCopy(r, t, NULL, &dst);
            if ((v->teamShiny >> i) & 1)
                shiny = 1;
        }
    }
    SDL_RenderSetScale(r, 2, 2);
}

static void DrawSide(SDL_Renderer *r, bool8 back)
{
    u8 kind = KindOf(R.region);
    const struct CardArt *a = &R.art[kind];
    const struct CardLayout *L = &sLayouts[kind];
    SDL_Texture *t = CardTexture(r, kind, back, MP_CardView_Stars(&R.view, R.region));
    SDL_Rect dst;
    sScale = G_MinF((float)RC_W / a->w, (float)RC_H / a->h);
    dst.w = (int)(a->w * sScale);
    dst.h = (int)(a->h * sScale);
    dst.x = sOx = (RC_W - dst.w) / 2;
    dst.y = sOy = (RC_H - dst.h) / 2;
    SDL_RenderSetScale(r, 1, 1);
    if (t)
        SDL_RenderCopy(r, t, NULL, &dst);
    SDL_RenderSetScale(r, 2, 2);
    if (back)
        DrawBack(r, L);
    else
        DrawFront(r, L);
}

static void DrawMyCard(void)
{
    // START on the player's own card: its team, its title, whether the title shows
    static const char *const sRows[] = {"CARD TEAM", "TITLE", "SHOW TITLE"};
    char value[40];
    const char *name, *desc;
    char status[8], reward[8];
    u16 t = MP_TCard_Title();
    u8 i;
    G_FillA(0, 0, 240, 160, 0x101010, 150);
    G_RoundFill(24, 36, 192, 88, 0x404048, 255);
    G_RoundFill(25, 37, 190, 86, 0xf8f0d0, 255);
    G_TextAlign(120, 40, TA_CENTER, G_INK, G_NO_SHADOW, "MY CARD", TF_NORMAL);
    for (i = 0; i < 3; i++)
    {
        int y = 58 + i * 16;
        if (i == R.myRow)
            G_RoundFill(30, y - 1, 180, 14, 0xf0d870, 255);
        G_Text(34, y, G_INK, G_NO_SHADOW, sRows[i], 0, TF_SMALL);
        if (i == 0)
            snprintf(value, sizeof(value), "%s", MP_TCard_TeamCount() ? "< KEPT AS SET >" : "< MY PARTY >");
        else if (i == 1)
        {
            if (t == 0 || t > MP_Achieve_Total() || !MP_Achieve_IsDone(t - 1))
                snprintf(value, sizeof(value), "< NONE >");
            else
            {
                MP_Achieve_Info(t - 1, &name, &desc, status, sizeof(status), reward, sizeof(reward));
                snprintf(value, sizeof(value), "< %s >", name);
            }
        }
        else
            snprintf(value, sizeof(value), "< %s >", MP_TCard_ShowTitle() ? "ON" : "OFF");
        G_TextAlign(206, y, TA_RIGHT, G_INK, G_NO_SHADOW, value, TF_SMALL);
    }
    G_Text(34, 108, 0x707060, G_NO_SHADOW, R.myRow == 0 ? "LEFT/RIGHT: the party now, or this team kept"
                                         : R.myRow == 1 ? "LEFT/RIGHT: an achievement's name" : "LEFT/RIGHT: on or off", 180, TF_SMALL);
}

SDL_Texture *MP_RegionCard_Canvas(SDL_Renderer *r, SDL_Texture *game)
{
    SDL_Texture *previous;
    SDL_Rect oldViewport, oldClip;
    SDL_bool clipped;
    float oldX, oldY;
    if (!R.active)
        return NULL;
    G_Begin(r);
    if (R.owner != r)
    {
        ForgetTextures();
        R.owner = r;
    }
    if (!R.canvas)
    {
        R.canvas = SDL_CreateTexture(r, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, RC_W, RC_H);
        R.side = SDL_CreateTexture(r, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, RC_W, RC_H);
        if (!R.canvas || !R.side)
            return NULL;
        SDL_SetTextureBlendMode(R.canvas, SDL_BLENDMODE_NONE);
        SDL_SetTextureBlendMode(R.side, SDL_BLENDMODE_BLEND);
        SDL_SetTextureScaleMode(R.side, SDL_ScaleModeLinear);
    }
    previous = SDL_GetRenderTarget(r);
    SDL_RenderGetViewport(r, &oldViewport);
    clipped = SDL_RenderIsClipEnabled(r);
    SDL_RenderGetClipRect(r, &oldClip);
    SDL_RenderGetScale(r, &oldX, &oldY);
    // the side shown, drawn whole (then squashed while it flips)
    SDL_SetRenderTarget(r, R.side);
    SDL_RenderSetViewport(r, NULL);
    SDL_RenderSetClipRect(r, NULL);
    SDL_SetRenderDrawColor(r, 0, 0, 0, 0);
    SDL_RenderClear(r);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    if (R.region < REGION_COUNT)
    {
        // (halfway through the flip the other side shows)
        bool8 back = R.back;
        if (R.flip > 5)
            back = !back;
        DrawSide(r, back);
    }
    SDL_SetRenderTarget(r, R.canvas);
    SDL_RenderSetScale(r, 1, 1);
    SDL_SetRenderDrawColor(r, 24, 24, 32, 255);
    SDL_RenderClear(r);
    if (R.region >= REGION_COUNT && game)
        SDL_RenderCopy(r, game, NULL, NULL);   // (MY CARD over the game's own card)
    else
    {
        SDL_Rect dst = {0, 0, RC_W, RC_H};
        if (R.flip)
        {
            float k = (R.flip > 5 ? R.flip - 5 : 5 - R.flip) / 5.0f;
            dst.w = (int)(RC_W * k);
            dst.x = (RC_W - dst.w) / 2;
        }
        if (dst.w > 0)
            SDL_RenderCopy(r, R.side, NULL, &dst);
    }
    SDL_RenderSetScale(r, 2, 2);
    if (R.myCard)
        DrawMyCard();
    SDL_SetRenderTarget(r, previous);
    SDL_RenderSetViewport(r, &oldViewport);
    SDL_RenderSetClipRect(r, clipped ? &oldClip : NULL);
    SDL_RenderSetScale(r, oldX, oldY);
    return R.canvas;
}

// ----------------------------------------------------------------------------------------------------- input

bool8 MP_RegionCard_IsActive(void) { return R.active; }
bool8 MP_RegionCard_Available(void)
{
    if (!R.tried)
        LoadPak();
    return R.loaded;
}

// The card for JOHTO or SINNOH over the game's: the player's own (pic: the game card's picture, NULL for none) or a
// friend's view
void MP_RegionCard_Open(u8 region, const struct MpCardView *friendView, const u32 *pic)
{
    if (!R.tried)
        LoadPak();
    if (!R.loaded)
        return;
    if (friendView)
        R.view = *friendView;
    else
        MP_CardView_FillOwn(&R.view);
    if (pic)
    {
        memcpy(R.view.pic, pic, sizeof(R.view.pic));
        R.view.hasPic = TRUE;
    }
    if (R.pic)
        SDL_DestroyTexture(R.pic);
    R.pic = NULL;
    R.region = region;
    R.active = TRUE;
    R.back = FALSE;
    R.flip = 0;
    R.myCard = FALSE;
    R.result = 0xFF;
    R.prevKeys = 0xFFFF;
    MPNet_Log("card: %s card shown%s", sRegionNames[region], friendView ? " (a friend's)" : "");
}

// what the game's card does next: 0xFF nothing, 0xFE close, a region (HOENN / KANTO: the game's own cards)
u8 MP_RegionCard_TakeResult(void)
{
    u8 r = R.result;
    R.result = 0xFF;
    return r;
}

static void MyCardStep(u16 pressed)
{
    if (pressed & (B_BUTTON | START_BUTTON))
    {
        R.myCard = FALSE;
        if (R.view.own)
            MP_CardView_FillOwn(&R.view), R.view.hasPic = R.view.hasPic || R.pic != NULL;
        PlaySE(SE_SELECT);
        return;
    }
    if (pressed & DPAD_UP)
        R.myRow = (R.myRow + 2) % 3;
    else if (pressed & DPAD_DOWN)
        R.myRow = (R.myRow + 1) % 3;
    else if (pressed & (DPAD_LEFT | DPAD_RIGHT | A_BUTTON))
    {
        s8 dir = (pressed & DPAD_LEFT) ? -1 : 1;
        if (R.myRow == 0)
        {
            if (MP_TCard_TeamCount())
                MP_TCard_SetTeamFromParty(NULL, 0);   // back to the party as it is
            else
            {
                static const u8 sAll[PARTY_SIZE] = {0, 1, 2, 3, 4, 5};
                MP_TCard_SetTeamFromParty(sAll, PARTY_SIZE);   // this team, kept
            }
        }
        else if (R.myRow == 1)
        {
            // the next achievement done (0: none)
            u16 n = MP_Achieve_Total(), t = MP_TCard_Title(), k;
            for (k = 0; k <= n; k++)
            {
                t = (t + n + 1 + dir) % (n + 1);
                if (t == 0 || MP_Achieve_IsDone(t - 1))
                    break;
            }
            MP_TCard_SetTitle(t);
        }
        else
            MP_TCard_SetShowTitle(!MP_TCard_ShowTitle());
        PlaySE(SE_SELECT);
    }
}

// the player's own card is open (TRUE) or a friend's
bool8 MP_RegionCard_MyCardOpen(void) { return R.myCard; }

void MP_RegionCard_OpenMyCard(void)
{
    // (also over the game's own cards: HOENN's and KANTO's)
    if (!R.active)
    {
        MP_CardView_FillOwn(&R.view);
        R.region = 0xFF;   // (no card art: the panel alone over the game's card)
        R.active = TRUE;
        R.result = 0xFF;
    }
    R.myCard = TRUE;
    R.myRow = 0;
    R.prevKeys = 0xFFFF;
    PlaySE(SE_WIN_OPEN);
}

bool8 MP_RegionCard_Step(u16 keys)
{
    u16 pressed;
    if (R.draining)
    {
        if (keys & (A_BUTTON | B_BUTTON | SELECT_BUTTON | START_BUTTON | L_BUTTON | R_BUTTON))
            return TRUE;
        R.draining = FALSE;
    }
    if (!R.active)
        return FALSE;
    if (MP_BadgeCase_IsActive())
        return FALSE;   // (the case is over it)
    R.age++;
    if (R.flip)
        R.flip--;
    pressed = keys & ~R.prevKeys;
    R.prevKeys = keys;
    if (R.myCard)
    {
        MyCardStep(pressed);
        if (!R.myCard && R.region == 0xFF)
        {
            R.active = FALSE;   // (the panel alone: back to the game's card)
            R.draining = TRUE;
            R.result = 0xFD;    // (the game's card draws itself again: the team or title may show)
        }
        return TRUE;
    }
    if (R.flip)
        return TRUE;
    if (pressed & A_BUTTON)
    {
        R.back = !R.back;
        R.flip = 10;
        PlaySE(SE_RG_CARD_FLIP);
    }
    else if (pressed & B_BUTTON)
    {
        R.active = FALSE;
        R.draining = TRUE;
        R.result = 0xFE;
    }
    else if ((pressed & START_BUTTON) && R.view.own)
        MP_RegionCard_OpenMyCard();
    else if (pressed & SELECT_BUTTON)
        MP_BadgeCase_OpenView(R.view.own ? NULL : &R.view, R.region);
    else if (pressed & (L_BUTTON | R_BUTTON))
    {
        // the next region's card: the game's own for HOENN and KANTO
        static const u8 sOrder[REGION_COUNT] = {REGION_HOENN, REGION_KANTO, REGION_JOHTO, REGION_SINNOH};
        u8 at = 0, k, next;
        for (k = 0; k < REGION_COUNT; k++)
            if (sOrder[k] == R.region)
                at = k;
        for (k = 1; k < REGION_COUNT; k++)
        {
            next = sOrder[(at + ((pressed & R_BUTTON) ? k : REGION_COUNT - k)) % REGION_COUNT];
            if (MP_TrainerCard_RegionShown(next, R.view.own ? NULL : &R.view))
                break;
        }
        PlaySE(SE_RG_CARD_OPEN);
        if (next == REGION_JOHTO || next == REGION_SINNOH)
        {
            R.region = next;
            R.back = FALSE;
        }
        else
        {
            R.active = FALSE;
            R.draining = TRUE;
            R.result = next;
        }
    }
    return TRUE;
}
