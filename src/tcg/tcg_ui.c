#include "mp_version.h"
#if MOEMON_DEV_BUILD
#include <SDL2/SDL.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "fonts.h"
#include "mp.h"
#include "text_window.h"
#include "constants/characters.h"
#include "tcg/art_pack.h"
#include "tcg/card_ids.h"
#include "tcg/duel.h"
#include "tcg/integration.h"
unsigned char *stbi_load(const char *, int *, int *, int *, int);
void stbi_image_free(void *);
int Tcg_GrantCase(void);
void Tcg_ResumeGame(void (*callback)(void));

enum
{
    UI_CLOSED,
    UI_INVITE,
    UI_STARTER,
    UI_CASE,
    UI_COLLECTION,
    UI_INSPECT,
    UI_DECKS,
    UI_EDIT,
    UI_PACKS,
    UI_REVEAL,
    UI_DUEL,
    UI_HAND,
    UI_TARGET,
    UI_ATTACKS,
    UI_CONFIRM_QUIT,
    UI_ART,
    UI_DECK_ACTIONS,
    UI_COPY_DECK,
    UI_COPY_CONFIRM,
    UI_CHECK,
    UI_FIELD,
    UI_DISCARD,
    UI_POWERS,
    UI_ONLINE,
    UI_ONLINE_INVITE,
    UI_ONLINE_WAIT
};
enum
{
    KEY_A = 1,
    KEY_B = 2,
    KEY_SELECT = 4,
    KEY_START = 8,
    KEY_RIGHT = 16,
    KEY_LEFT = 32,
    KEY_UP = 64,
    KEY_DOWN = 128,
    KEY_R = 256,
    KEY_L = 512
};
static struct
{
    int screen, back, cursor, scroll, card, deck, pack, editing, showAll, selectedSlot, frame;
    int inspectScroll, inspectMax, zoom, panX, panY, copyTarget, filter, viewWho, online;
    unsigned previousKeys, age, aiDelay;
    const char *opponent, *opponentDeck;
    unsigned rewardPack;
    void (*returnToGame)(void);
    struct TcgDuel duel;
    struct TcgAction action;
    uint16_t revealed[10];
    char message[256];
} U;
struct ArtTexture
{
    unsigned id, full, age, fallback;
    SDL_Texture *texture;
    int w, h;
};
static struct ArtTexture sArt[24];
static unsigned sArtAge;
static SDL_Renderer *sRenderer;
static SDL_Texture *sFontFace[2], *sFontShadow[2], *sCanvas, *sWindowFrame;
static unsigned sWindowFrameId;
static int sFontReady;

static void Message(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    vsnprintf(U.message, sizeof(U.message), format, ap);
    va_end(ap);
}
static void Screen(int screen)
{
    U.screen = screen;
    U.cursor = 0;
    U.scroll = 0;
    U.age = 0;
    U.message[0] = 0;
}
int Tcg_IsActive(void) { return U.screen != UI_CLOSED; }
int Tcg_OnlineWilling(void) { return U.screen == UI_ONLINE && Tcg_GetCollection()->optedIn; }
static void OnlineView(void)
{
    struct TcgLink *l = Tcg_Online();
    U.duel = l->game;
    if (l->seat)
    {
        struct TcgPlayer temp = U.duel.players[0];
        U.duel.players[0] = U.duel.players[1];
        U.duel.players[1] = temp;
        U.duel.current ^= 1;
        U.duel.first ^= 1;
        if (U.duel.promotePlayer < 2)
            U.duel.promotePlayer ^= 1;
        if (U.duel.winner < 2)
            U.duel.winner ^= 1;
    }
}
static void Close(void)
{
    void (*callback)(void) = U.returnToGame;
    if (U.online)
        TcgLink_Cancel(Tcg_Online(), "Card session closed.");
    U.online = 0;
    Screen(UI_CLOSED);
    U.returnToGame = NULL;
    Tcg_ResumeGame(callback);
}
void Tcg_OpenChallenge(const char *name, const char *deck, unsigned pack)
{
    struct TcgCollection *c = Tcg_GetCollection();
    U.opponent = name;
    U.opponentDeck = deck;
    U.rewardPack = pack;
    U.returnToGame = NULL;
    Screen(c->optedIn ? UI_CASE : UI_INVITE);
    printf("[tcg] explicit challenge: %s (%s), opted in %u\n", name, deck, c->optedIn);
}
void Tcg_OpenCase(void (*returnToGame)(void))
{
    struct TcgCollection *c = Tcg_GetCollection();
    U.returnToGame = returnToGame;
    U.opponent = NULL;
    U.opponentDeck = NULL;
    U.rewardPack = 0;
    if (!c->optedIn)
    {
        U.opponent = "Practice Collector";
        U.opponentDeck = "PracticePlayer";
        U.rewardPack = 0;
    }
    Screen(c->optedIn ? UI_CASE : UI_INVITE);
}
void Tcg_DestroyTextures(void)
{
    unsigned i;
    for (i = 0; i < 24; i++)
    {
        if (sArt[i].texture)
            SDL_DestroyTexture(sArt[i].texture);
        memset(&sArt[i], 0, sizeof(sArt[i]));
    }
    for (i = 0; i < 2; i++)
    {
        if (sFontFace[i])
            SDL_DestroyTexture(sFontFace[i]);
        if (sFontShadow[i])
            SDL_DestroyTexture(sFontShadow[i]);
        sFontFace[i] = sFontShadow[i] = NULL;
    }
    if (sCanvas)
        SDL_DestroyTexture(sCanvas);
    if (sWindowFrame)
        SDL_DestroyTexture(sWindowFrame);
    sCanvas = sWindowFrame = NULL;
    sRenderer = NULL;
    sFontReady = 0;
}
static unsigned GameChar(unsigned char c)
{
    if (c >= 'A' && c <= 'Z')
        return CHAR_A + c - 'A';
    if (c >= 'a' && c <= 'z')
        return CHAR_a + c - 'a';
    if (c >= '0' && c <= '9')
        return CHAR_0 + c - '0';
    switch (c)
    {
    case ' ':
        return CHAR_SPACE;
    case '!':
        return CHAR_EXCL_MARK;
    case '?':
        return CHAR_QUESTION_MARK;
    case '.':
        return CHAR_PERIOD;
    case '-':
        return CHAR_HYPHEN;
    case ',':
        return CHAR_COMMA;
    case '/':
        return CHAR_SLASH;
    case ':':
        return CHAR_COLON;
    case '\'':
        return CHAR_SGL_QUOTE_RIGHT;
    case '(':
        return CHAR_LEFT_PAREN;
    case ')':
        return CHAR_RIGHT_PAREN;
    case '+':
        return CHAR_PLUS;
    case '%':
        return CHAR_PERCENT;
    default:
        return CHAR_QUESTION_MARK;
    }
}
static void Font(SDL_Renderer *renderer)
{
    unsigned font, code, x, y;
    uint32_t *face, *shadow;
    if (sFontReady)
        return;
    sFontReady = 1;
    face = calloc(256 * 256, sizeof(*face));
    shadow = calloc(256 * 256, sizeof(*shadow));
    if (!face || !shadow)
    {
        free(face);
        free(shadow);
        return;
    }
    for (font = 0; font < 2; font++)
    {
        const u16 *glyphs = font ? gFontSmallLatinGlyphs : gFontNormalLatinGlyphs;
        memset(face, 0, 256 * 256 * 4);
        memset(shadow, 0, 256 * 256 * 4);
        for (code = 0; code < 256; code++)
            for (y = 0; y < 16; y++)
                for (x = 0; x < 16; x++)
                {
                    unsigned tile = (y / 8) * 2 + x / 8;
                    unsigned value = (glyphs[code * 32 + tile * 8 + y % 8] >> (14 - 2 * (x % 8))) & 3;
                    unsigned dst = ((code / 16) * 16 + y) * 256 + (code % 16) * 16 + x;
                    if (value == 1)
                        face[dst] = 0xffffffffu;
                    if (value == 2)
                        shadow[dst] = 0xffffffffu;
                }
        sFontFace[font] = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, 256, 256);
        sFontShadow[font] = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, 256, 256);
        if (sFontFace[font])
        {
            SDL_UpdateTexture(sFontFace[font], NULL, face, 256 * 4);
            SDL_SetTextureBlendMode(sFontFace[font], SDL_BLENDMODE_BLEND);
            SDL_SetTextureScaleMode(sFontFace[font], SDL_ScaleModeNearest);
        }
        if (sFontShadow[font])
        {
            SDL_UpdateTexture(sFontShadow[font], NULL, shadow, 256 * 4);
            SDL_SetTextureBlendMode(sFontShadow[font], SDL_BLENDMODE_BLEND);
            SDL_SetTextureScaleMode(sFontShadow[font], SDL_ScaleModeNearest);
        }
    }
    free(face);
    free(shadow);
}
static SDL_Rect Rect(float x, float y, float w, float h)
{
    SDL_Rect r = {(int)x, (int)y, (int)w, (int)h};
    return r;
}
static void Color(SDL_Renderer *r, unsigned color)
{
    SDL_SetRenderDrawColor(r, (color >> 16) & 255, (color >> 8) & 255, color & 255, 255);
}
static void Fill(SDL_Renderer *r, int x, int y, int w, int h, unsigned color)
{
    SDL_Rect box = Rect(x, y, w, h);
    Color(r, color);
    SDL_RenderFillRect(r, &box);
}
static int FontWidth(unsigned ch, int small)
{
    return small ? gFontSmallLatinGlyphWidths[ch] : gFontNormalLatinGlyphWidths[ch];
}
static int Measure(const char *s, int small)
{
    int w = 0;
    while (*s)
        w += FontWidth(GameChar((unsigned char)*s++), small);
    return w;
}
static void Glyph(SDL_Renderer *r, unsigned code, int x, int y, int small, unsigned color)
{
    int w = FontWidth(code, small), h = small ? 12 : 15;
    SDL_Rect src = {(code & 15) * 16, (code >> 4) * 16, w, h}, dst = {x, y, w, h};
    SDL_Texture *shadow = sFontShadow[small], *face = sFontFace[small];
    if (!shadow || !face)
        return;
    SDL_SetTextureColorMod(shadow, 216, 216, 216);
    SDL_SetTextureColorMod(face, (color >> 16) & 255, (color >> 8) & 255, color & 255);
    SDL_RenderCopy(r, shadow, &src, &dst);
    SDL_RenderCopy(r, face, &src, &dst);
}
static int NativeText(SDL_Renderer *r, int x, int y, unsigned color, const char *text, int width, int small)
{
    int px = x, py = y, start = 1, line = small ? 12 : 16;
    while (*text)
    {
        unsigned char ch = (unsigned char)*text;
        if (start && width > 0 && ch != ' ' && ch != '\n')
        {
            const char *next = text;
            int word = 0;
            while (*next && *next != ' ' && *next != '\n')
                word += FontWidth(GameChar((unsigned char)*next++), small);
            if (px > x && px + word > x + width)
            {
                px = x;
                py += line;
            }
        }
        text++;
        start = ch == ' ' || ch == '\n';
        if (ch == '\n')
        {
            px = x;
            py += line;
            continue;
        }
        unsigned code = GameChar(ch);
        int w = FontWidth(code, small);
        if (width > 0 && px + w > x + width)
        {
            px = x;
            py += line;
        }
        Glyph(r, code, px, py, small, color);
        px += w;
    }
    return py + line;
}
static int Text(SDL_Renderer *r, int x, int y, unsigned color, const char *text, int width)
{
    return NativeText(r, x, y, color, text, width, 1);
}
static unsigned Rgb15(unsigned c)
{
    return ((c & 31) * 255 / 31) << 16 | (((c >> 5) & 31) * 255 / 31) << 8 | (((c >> 10) & 31) * 255 / 31);
}
static void Window(SDL_Renderer *r, int x, int y, int w, int h)
{
    unsigned frame = gSaveBlock2Ptr ? gSaveBlock2Ptr->optionsWindowFrameType : 0;
    const struct TilesPal *assets = GetWindowFrameTilesPal(frame < WINDOW_FRAMES_COUNT ? frame : 0);
    SDL_Texture *texture;
    uint32_t pixels[24 * 24];
    unsigned t, px, py;
    if (!sWindowFrame || sWindowFrameId != frame)
    {
        if (sWindowFrame)
            SDL_DestroyTexture(sWindowFrame);
        sWindowFrameId = frame;
        for (t = 0; t < 9; t++)
            for (py = 0; py < 8; py++)
                for (px = 0; px < 8; px++)
                {
                    unsigned index = (assets->tiles[t * 32 + py * 4 + px / 2] >> ((px & 1) * 4)) & 15;
                    unsigned c = Rgb15(assets->pal[index]);
                    unsigned dst = ((t / 3) * 8 + py) * 24 + (t % 3) * 8 + px;
                    pixels[dst] = index ? (0xff000000u | ((c & 255) << 16) | (c & 0xff00) | (c >> 16)) : 0;
                }
        sWindowFrame = SDL_CreateTexture(r, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, 24, 24);
        if (sWindowFrame)
        {
            SDL_UpdateTexture(sWindowFrame, NULL, pixels, 24 * 4);
            SDL_SetTextureBlendMode(sWindowFrame, SDL_BLENDMODE_BLEND);
            SDL_SetTextureScaleMode(sWindowFrame, SDL_ScaleModeNearest);
        }
    }
    Fill(r, x + 4, y + 4, w - 8, h - 8, 0xf8f8f0);
    texture = sWindowFrame;
    if (!texture)
        return;
    for (py = 0; py < (unsigned)h; py += 8)
        for (px = 0; px < (unsigned)w; px += 8)
        {
            unsigned tx = px == 0 ? 0 : px + 8 >= (unsigned)w ? 2 : 1, ty = py == 0 ? 0 : py + 8 >= (unsigned)h ? 2 : 1;
            SDL_Rect src = {(int)tx * 8, (int)ty * 8, 8, 8},
                     dst = {x + (int)px, y + (int)py, w - (int)px < 8 ? w - (int)px : 8,
                            h - (int)py < 8 ? h - (int)py : 8};
            src.w = dst.w;
            src.h = dst.h;
            SDL_RenderCopy(r, texture, &src, &dst);
        }
}
static void Cursor(SDL_Renderer *r, int x, int y)
{
    int row;
    for (row = 0; row < 7; row++)
    {
        int w = row < 4 ? row + 1 : 7 - row;
        Fill(r, x, y + row, w, 1, 0x383838);
    }
}
static void Header(SDL_Renderer *r, const char *title, const char *right)
{
    Window(r, 0, 0, 240, 32);
    NativeText(r, 8, 7, 0x383838, title, 170, 0);
    if (right)
        Text(r, 232 - Measure(right, 1), 9, 0x586850, right, 80);
}
static void Footer(SDL_Renderer *r, const char *text)
{
    Window(r, 0, 136, 240, 24);
    Text(r, 8, 142, 0x383838, text, 224);
}
static void Option(SDL_Renderer *r, int index, int x, int y, const char *text)
{
    if (index == U.cursor)
        Cursor(r, x - 8, y + 4);
    NativeText(r, x, y, 0x383838, text, 125, 0);
}
static void Mat(SDL_Renderer *r)
{
    int x, y;
    Fill(r, 0, 0, 240, 160, 0x688878);
    for (y = 0; y < 160; y += 8)
        for (x = 0; x < 240; x += 8)
            if (((x + y) / 8) & 1)
                Fill(r, x, y, 8, 8, 0x708e78);
}

static struct ArtTexture *Art(SDL_Renderer *r, unsigned id, unsigned full)
{
    unsigned i, best = 0;
    struct ArtTexture *a;
    char path[1200];
    const char *env = getenv("MOEMON_TCG_ART_DIR");
    char *base;
    unsigned char *pixels;
    int channels;
    for (i = 0; i < 24; i++)
        if (sArt[i].texture && sArt[i].id == id && sArt[i].full == full)
        {
            sArt[i].age = ++sArtAge;
            return &sArt[i];
        }
    for (i = 0; i < 24; i++)
        if (!sArt[i].texture || sArt[i].age < sArt[best].age)
        {
            best = i;
            if (!sArt[i].texture)
                break;
        }
    a = &sArt[best];
    if (a->texture)
        SDL_DestroyTexture(a->texture);
    memset(a, 0, sizeof(*a));
    base = SDL_GetBasePath();
    if (env)
        snprintf(path, sizeof(path), "%s/%s/%03u.jpg", env, full ? "cards" : "thumbnails", id);
    else
        snprintf(path, sizeof(path), "%stcg/art/%s/%s/%03u.jpg", base ? base : "", TCG_ART_VERSION,
                 full ? "cards" : "thumbnails", id);
    pixels = stbi_load(path, &a->w, &a->h, &channels, 4);
    if (!pixels)
    {
        snprintf(path, sizeof(path), "dev/artwork/tcg-art/%s/%03u.jpg", full ? "cards" : "thumbnails", id);
        pixels = stbi_load(path, &a->w, &a->h, &channels, 4);
    }
    if (pixels)
    {
        a->full = full;
    }
    else
    {
        snprintf(path, sizeof(path), "%stcg/original/%03u.png", base ? base : "", id);
        pixels = stbi_load(path, &a->w, &a->h, &channels, 4);
        if (!pixels)
        {
            snprintf(path, sizeof(path), "graphics/tcg/original/%03u.png", id);
            pixels = stbi_load(path, &a->w, &a->h, &channels, 4);
        }
        a->full = full;
        a->fallback = 1;
    }
    SDL_free(base);
    if (!pixels)
        return NULL;
    a->texture = SDL_CreateTexture(r, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, a->w, a->h);
    if (a->texture)
    {
        SDL_UpdateTexture(a->texture, NULL, pixels, a->w * 4);
        SDL_SetTextureScaleMode(a->texture, SDL_ScaleModeNearest);
    }
    stbi_image_free(pixels);
    a->id = id;
    a->age = ++sArtAge;
    return a;
}
static void Card(SDL_Renderer *r, unsigned id, int x, int y, int w, int h, int full)
{
    struct ArtTexture *a;
    const struct TcgCard *c = Tcg_Card(id);
    SDL_Rect dst;
    Fill(r, x, y, w, h, id ? 0xe1c16d : 0x263e70);
    if (!id)
    {
        Text(r, x + 12, y + h / 2 - 12, 0xffffff, "MOEMON", w - 20);
        return;
    }
    a = Art(r, id, full);
    if (a && a->texture && !a->fallback)
    {
        /* Fit the printed scan within the caller's padded content rectangle.
         * Keep its aspect ratio even when a menu uses a short preview area. */
        int fitW = w, fitH = (int)((long)w * a->h / a->w);
        if (fitH > h)
        {
            fitH = h;
            fitW = (int)((long)h * a->w / a->h);
        }
        dst = Rect(x + (w - fitW) / 2, y + (h - fitH) / 2, fitW, fitH);
        SDL_RenderCopy(r, a->texture, NULL, &dst);
        return;
    }
    Fill(r, x + 3, y + 3, w - 6, h - 6, 0xf5f0db);
    if (a && a->texture)
    {
        dst = Rect(x + 6, y + h / 5, w - 12, h * 2 / 5);
        SDL_RenderCopy(r, a->texture, NULL, &dst);
    }
    if (c && h > 70)
        Text(r, x + 2, y + h - 13, 0x383838, "GB", w - 4);
}
/* Compact deck-builder preview: illustration only, with a centered frame.
 * Coordinates are relative to the original 600x825 printed-card layout. */
static void DeckIllustration(SDL_Renderer *r, unsigned id)
{
    struct ArtTexture *art = id ? Art(r, id, 1) : NULL;
    const struct TcgCard *card = Tcg_Card(id);
    SDL_Rect src, dst;
    int width = 56, height, contentH, panelY;
    if (!art || !art->texture || !card)
        return;
    src = Rect(0, 0, art->w, art->h);
    if (!art->fallback)
    {
        int x = 64, y = 99, w = 474, h = 328;
        if (card->kind == TCG_TRAINER)
        {
            x = 64;
            y = 199;
            w = 480;
            h = 255;
        }
        else if (card->kind == TCG_ENERGY)
        {
            x = 136;
            y = 410;
            w = h = 330;
        }
        src = Rect(x * art->w / 600, y * art->h / 825, w * art->w / 600, h * art->h / 825);
    }
    height = (width * src.h + src.w / 2) / src.w;
    contentH = (height + 7) / 8 * 8;
    panelY = 96 - (contentH + 16) / 2;
    Window(r, 168, panelY, 72, contentH + 16);
    dst = Rect(176, panelY + 8 + (contentH - height) / 2, width, height);
    SDL_RenderCopy(r, art->texture, &src, &dst);
}
static unsigned DeckCount(unsigned deck, unsigned card)
{
    struct TcgCollection *c = Tcg_GetCollection();
    unsigned i, n = 0;
    for (i = 0; i < TCG_DECK_SIZE; i++)
        n += card ? c->decks[deck].cards[i] == card : c->decks[deck].cards[i] != 0;
    return n;
}
/* Original deck_configuration.asm groups Pokemon by type, then Trainers/Energy. */
static int CardFilter(unsigned id, unsigned filter)
{
    static const unsigned types[] = {TCG_GRASS,    TCG_FIRE,    TCG_WATER,    TCG_LIGHTNING,
                                     TCG_FIGHTING, TCG_PSYCHIC, TCG_COLORLESS};
    const struct TcgCard *card = Tcg_Card(id);
    if (!card)
        return 0;
    if (filter < 7)
        return card->kind == TCG_POKEMON && card->type == types[filter];
    return card->kind == (filter == 7 ? TCG_TRAINER : TCG_ENERGY);
}
static int VisibleCard(unsigned id)
{
    struct TcgCollection *c = Tcg_GetCollection();
    if (U.screen == UI_EDIT)
        return CardFilter(id, U.filter) && (U.editing ? DeckCount(U.deck, id) != 0 : id <= 6 || c->owned[id] != 0);
    return U.showAll || c->owned[id];
}
static unsigned CollectionCard(unsigned index)
{
    unsigned id;
    for (id = 1; id <= TCG_CARD_COUNT; id++)
        if (VisibleCard(id))
        {
            if (!index--)
                return id;
        }
    return 0;
}
static unsigned CollectionSize(void)
{
    unsigned id, n = 0;
    for (id = 1; id <= TCG_CARD_COUNT; id++)
        n += VisibleCard(id);
    return n;
}
static void Inspect(unsigned id, int back)
{
    U.card = id;
    U.back = back;
    U.inspectScroll = U.inspectMax = U.zoom = U.panX = U.panY = 0;
    Screen(UI_INSPECT);
}
static void Apply(const struct TcgAction *a)
{
    int result;
    if (U.online)
    {
        if (!TcgLink_Submit(Tcg_Online(), a, SDL_GetTicks()))
            Message("%s", Tcg_Online()->error[0] ? Tcg_Online()->error
                                                 : "Wait for your turn and the other player's acknowledgement.");
        else
        {
            OnlineView();
            Screen(UI_DUEL);
        }
        return;
    }
    result = TcgDuel_Apply(&U.duel, a);
    if (result != TCG_OK)
        Message("%s", U.duel.error[0] ? U.duel.error : "Choose the required cards and targets.");
    else
    {
        Screen(UI_DUEL);
        U.aiDelay = 20;
    }
}
static void StartDuel(int practice)
{
    struct TcgCollection *c = Tcg_GetCollection();
    unsigned i;
    char reason[160];
    const char *name = practice ? "PracticePlayer" : U.opponentDeck;
    if (!Tcg_DeckValidate(c->decks[c->activeDeck].cards, c->owned, reason, sizeof(reason)))
    {
        Message("%s", reason);
        return;
    }
    for (i = 0; i < gTcgDeckDefinitionCount; i++)
        if (name && !strcmp(name, gTcgDeckDefinitions[i].name))
            break;
    if (i == gTcgDeckDefinitionCount)
    {
        Message("The opponent's deck is unavailable.");
        return;
    }
    if (!TcgDuel_Init(&U.duel, c->decks[c->activeDeck].cards, gTcgDeckDefinitions[i].cards, Tcg_Random(&c->rng), 6))
    {
        Message("Unable to set up this duel.");
        return;
    }
    if (practice)
        U.opponent = "Practice Collector";
    Screen(UI_DUEL);
    U.aiDelay = 20;
}
static int MenuSize(void)
{
    switch (U.screen)
    {
    case UI_INVITE:
        return 2;
    case UI_STARTER:
        return 3;
    case UI_CASE:
        return 9;
    case UI_ONLINE:
        return MP_MAX_PLAYERS;
    case UI_ONLINE_INVITE:
        return 2;
    case UI_COLLECTION:
    case UI_EDIT:
        return CollectionSize();
    case UI_DECK_ACTIONS:
        return 3;
    case UI_COPY_CONFIRM:
        return 2;
    case UI_DECKS:
    case UI_COPY_DECK:
        return TCG_SAVED_DECKS;
    case UI_PACKS:
        return 4;
    case UI_REVEAL:
        return 10;
    case UI_CHECK:
        return 5;
    case UI_FIELD:
    case UI_POWERS:
        return TCG_FIELD_SIZE;
    case UI_DISCARD:
        return U.duel.players[U.viewWho].discardSize;
    case UI_ATTACKS:
        return 2;
    case UI_CONFIRM_QUIT:
        return 2;
    case UI_DUEL:
        return 6;
    case UI_HAND:
        return U.duel.players[0].handSize;
    case UI_TARGET:
        return TCG_FIELD_SIZE;
    default:
        return 1;
    }
}
static void Choose(void)
{
    struct TcgCollection *c = Tcg_GetCollection();
    struct TcgAction a;
    unsigned id, i;
    char reason[160];
    switch (U.screen)
    {
    case UI_INVITE:
        if (U.cursor)
            Close();
        else
            Screen(UI_STARTER);
        break;
    case UI_STARTER:
        if (!Tcg_GrantCase())
            Message("Your Key Items pocket needs space for the Card Case.");
        else if (Tcg_ChooseStarter(c, U.cursor))
            Screen(UI_CASE);
        break;
    case UI_CASE:
        switch (U.cursor)
        {
        case 0:
            if (U.opponentDeck)
                StartDuel(0);
            else
                Message("Face a card-playing NPC and use the challenge action to request a duel.");
            break;
        case 1:
            Screen(UI_COLLECTION);
            break;
        case 2:
            Screen(UI_DECKS);
            break;
        case 3:
            Screen(UI_PACKS);
            break;
        case 4:
            StartDuel(1);
            break;
        case 5:
            U.showAll = !U.showAll;
            Message("Collection view: %s", U.showAll ? "all cards" : "owned cards");
            break;
        case 6:
            c->fastAnimations = !c->fastAnimations;
            Message("Animations: %s", c->fastAnimations ? "fast" : "normal");
            break;
        case 7:
            Screen(UI_ONLINE);
            break;
        case 8:
            Close();
            break;
        }
        break;
    case UI_ONLINE:
    {
        struct TcgLink *l = Tcg_Online();
        if (!MP_IsConnected() || U.cursor == (int)gMpLocalId || !gMpPlayers[U.cursor].present)
            Message("Choose a connected friend. Both players must open ONLINE CARDS.");
        else if (!Tcg_DeckValidate(c->decks[c->activeDeck].cards, c->owned, reason, sizeof(reason)))
            Message("%s", reason);
        else if (TcgLink_Invite(l, U.cursor, Tcg_Random(&c->rng), c->decks[c->activeDeck].cards, SDL_GetTicks()))
        {
            U.online = 1;
            Screen(UI_ONLINE_WAIT);
        }
        break;
    }
    case UI_ONLINE_INVITE:
        if (U.cursor)
        {
            TcgLink_Cancel(Tcg_Online(), "Invitation declined.");
            U.online = 0;
            Screen(UI_ONLINE);
        }
        else if (!Tcg_DeckValidate(c->decks[c->activeDeck].cards, c->owned, reason, sizeof(reason)))
            Message("%s", reason);
        else if (TcgLink_Accept(Tcg_Online(), c->decks[c->activeDeck].cards, SDL_GetTicks()))
        {
            U.online = 1;
            Screen(UI_ONLINE_WAIT);
        }
        break;
    case UI_INSPECT:
        Screen(UI_ART);
        break;
    case UI_ART:
        U.zoom = !U.zoom;
        U.panX = U.panY = 0;
        break;
    case UI_COLLECTION:
        Inspect(CollectionCard(U.cursor), UI_COLLECTION);
        break;
    case UI_DECKS:
        U.deck = U.cursor;
        Screen(UI_DECK_ACTIONS);
        break;
    case UI_DECK_ACTIONS:
        if (U.cursor == 0)
        {
            if (Tcg_DeckValidate(c->decks[U.deck].cards, c->owned, reason, sizeof(reason)))
            {
                c->activeDeck = U.deck;
                Message("Active deck: %s", c->decks[U.deck].name);
            }
            else
                Message("%s", reason);
        }
        else if (U.cursor == 1)
        {
            if (!c->decks[U.deck].name[0])
                snprintf(c->decks[U.deck].name, TCG_NAME_LEN, "Deck %d", U.deck + 1);
            U.editing = 0;
            Screen(UI_EDIT);
        }
        else
            Screen(UI_COPY_DECK);
        break;
    case UI_COPY_DECK:
        if (U.cursor == U.deck)
            Message("Choose a different deck slot.");
        else
        {
            U.copyTarget = U.cursor;
            if (DeckCount(U.copyTarget, 0))
                Screen(UI_COPY_CONFIRM);
            else
            {
                Tcg_DeckCopy(c, U.deck, U.copyTarget);
                Screen(UI_DECK_ACTIONS);
                Message("Deck copied to slot %u.", U.copyTarget + 1);
            }
        }
        break;
    case UI_COPY_CONFIRM:
        if (U.cursor == 1)
        {
            Tcg_DeckCopy(c, U.deck, U.copyTarget);
            Screen(UI_DECK_ACTIONS);
            Message("Deck copied to slot %u.", U.copyTarget + 1);
        }
        else
            Screen(UI_COPY_DECK);
        break;
    case UI_EDIT:
        id = CollectionCard(U.cursor);
        if (id)
            Inspect(id, UI_EDIT);
        break;
    case UI_PACKS:
        U.pack = U.cursor;
        if (Tcg_OpenPack(c, U.pack, U.revealed))
        {
            Screen(UI_REVEAL);
            printf("[tcg] opened pack %d\n", U.pack);
        }
        else
            Message("No unopened packs of this type.");
        break;
    case UI_REVEAL:
        Inspect(U.revealed[U.cursor], UI_REVEAL);
        break;
    case UI_DUEL:
        if (U.duel.phase == TCG_FINISHED)
        {
            if (U.online)
            {
                TcgLink_Cancel(Tcg_Online(), "Duel complete.");
                U.online = 0;
            }
            Screen(UI_CASE);
            break;
        }
        if (U.duel.phase == TCG_PROMOTION)
        {
            memset(&a, 0, sizeof(a));
            a.type = TCG_PROMOTE;
            a.slot = U.cursor;
            Apply(&a);
            break;
        }
        if (U.duel.phase == TCG_SETUP || U.cursor == 0)
        {
            Screen(UI_HAND);
            break;
        }
        if (U.cursor == 1)
            Screen(UI_ATTACKS);
        else if (U.cursor == 2)
            Screen(UI_CHECK);
        else if (U.cursor == 3)
            Screen(UI_POWERS);
        else if (U.cursor == 4)
        {
            memset(&U.action, 0, sizeof(U.action));
            U.action.type = TCG_RETREAT;
            Screen(UI_TARGET);
        }
        else
        {
            memset(&a, 0, sizeof(a));
            a.type = TCG_PASS;
            Apply(&a);
        }
        break;
    case UI_CHECK:
        U.viewWho = U.cursor == 1 || U.cursor == 3;
        if (U.cursor < 2)
            Screen(UI_FIELD);
        else if (U.cursor < 4)
            Screen(UI_DISCARD);
        else
            Screen(UI_HAND);
        break;
    case UI_FIELD:
    {
        const struct TcgCard *card = TcgDuel_Field(&U.duel, U.viewWho, U.cursor);
        if (card)
            Inspect(card->id, UI_FIELD);
        break;
    }
    case UI_DISCARD:
    {
        const struct TcgPlayer *p = &U.duel.players[U.viewWho];
        if ((unsigned)U.cursor < p->discardSize)
            Inspect(p->cards[p->discard[U.cursor]], UI_DISCARD);
        break;
    }
    case UI_POWERS:
    {
        const struct TcgCard *card = TcgDuel_Field(&U.duel, 0, U.cursor);
        if (!card)
            break;
        for (i = 0; i < 2; i++)
            if (card->attacks[i].category == TCG_POKEMON_POWER && card->attacks[i].name[0])
            {
                memset(&a, 0, sizeof(a));
                a.type = TCG_USE_POWER;
                a.slot = U.cursor;
                a.attack = i;
                Apply(&a);
                return;
            }
        Message("This Pokemon has no Pokemon Power.");
        break;
    }
    case UI_HAND:
        if (U.online && U.duel.current != 0)
        {
            Message("Wait for your turn.");
            break;
        }
        if ((unsigned)U.cursor >= U.duel.players[0].handSize)
            break;
        id = U.duel.players[0].cards[U.duel.players[0].hand[U.cursor]];
        memset(&U.action, 0, sizeof(U.action));
        U.action.hand = U.cursor;
        U.action.type = Tcg_Card(id)->kind == TCG_ENERGY    ? TCG_ATTACH
                        : Tcg_Card(id)->kind == TCG_TRAINER ? TCG_PLAY_TRAINER
                        : Tcg_Card(id)->stage               ? TCG_EVOLVE
                                                            : TCG_PLAY_BASIC;
        if (U.action.type == TCG_PLAY_TRAINER)
            Apply(&U.action);
        else
            Screen(UI_TARGET);
        break;
    case UI_TARGET:
        U.action.slot = U.cursor;
        if (U.action.type == TCG_RETREAT)
        {
            const struct TcgCard *active = TcgDuel_Field(&U.duel, 0, 0);
            struct TcgFieldCard *f = &U.duel.players[0].field[0];
            unsigned energy = 0;
            U.action.selectionCount = 0;
            for (i = 0; active && i < f->attachedSize && energy < active->retreat; i++)
            {
                unsigned physical = f->attached[i];
                const struct TcgCard *e = Tcg_Card(U.duel.players[0].cards[physical]);
                if (e->kind == TCG_ENERGY)
                {
                    U.action.selections[U.action.selectionCount++] = physical;
                    energy += e->id == TCG_CARD_DOUBLE_COLORLESS_ENERGY ? 2 : 1;
                }
            }
        }
        Apply(&U.action);
        break;
    case UI_ATTACKS:
        memset(&a, 0, sizeof(a));
        a.type = TCG_ATTACK;
        a.attack = U.cursor;
        Apply(&a);
        break;
    case UI_CONFIRM_QUIT:
        if (!U.cursor)
        {
            memset(&a, 0, sizeof(a));
            a.type = TCG_CONCEDE;
            Apply(&a);
        }
        else
            Screen(UI_DUEL);
        break;
    default:
        break;
    }
}
int Tcg_Step(unsigned keys)
{
    unsigned pressed = keys & ~U.previousKeys;
    U.previousKeys = keys;
    if (!Tcg_IsActive())
        return 0;
    U.age++;
    U.frame++;
    {
        struct TcgLink *l = Tcg_Online();
        if (U.screen == UI_ONLINE && l->state == TCG_LINK_INVITED)
        {
            U.online = 1;
            Screen(UI_ONLINE_INVITE);
        }
        if (U.online && l->state == TCG_LINK_PLAYING)
        {
            OnlineView();
            if (U.screen == UI_ONLINE_WAIT)
                Screen(UI_DUEL);
        }
        if (U.online && l->state == TCG_LINK_ENDED)
        {
            U.online = 0;
            Screen(UI_CASE);
            Message("%s", l->error);
        }
    }
    if ((U.screen == UI_DUEL || U.screen == UI_HAND || U.screen == UI_TARGET || U.screen == UI_ATTACKS) && !U.online &&
        U.duel.phase == TCG_FINISHED && !U.duel.rewardClaimed)
    {
        U.duel.rewardClaimed = 1;
        if (U.duel.winner == 0)
        {
            Tcg_AwardPack(Tcg_GetCollection(), U.rewardPack);
            printf("[tcg] victory: one booster awarded\n");
            Message("Victory! One booster pack was added to your Card Case.");
        }
        else
            Message("The duel is over. No cards or ordinary progress were lost.");
    }
    if (!U.online && U.screen == UI_DUEL && U.duel.phase != TCG_FINISHED &&
        (U.duel.phase == TCG_PROMOTION ? U.duel.promotePlayer : U.duel.current) == 1)
    {
        struct TcgAction a;
        if (U.aiDelay)
        {
            U.aiDelay--;
            return 1;
        }
        if (TcgDuel_AiAction(&U.duel, &a) && TcgDuel_Apply(&U.duel, &a) == TCG_OK)
            U.aiDelay = Tcg_GetCollection()->fastAnimations ? 2 : 20;
        else
        {
            Message("Opponent could not resolve an action: %s", U.duel.error);
            U.aiDelay = 120;
        }
        return 1;
    }
    if (U.message[0] && (pressed & (KEY_A | KEY_B)))
    {
        U.message[0] = 0;
        return 1;
    }
    if (pressed & KEY_B)
    {
        switch (U.screen)
        {
        case UI_INVITE:
        case UI_STARTER:
        case UI_CASE:
            Close();
            break;
        case UI_ART:
            Screen(UI_INSPECT);
            break;
        case UI_INSPECT:
            Screen(U.back);
            break;
        case UI_EDIT:
        case UI_COPY_DECK:
            Screen(UI_DECK_ACTIONS);
            break;
        case UI_COPY_CONFIRM:
            Screen(UI_COPY_DECK);
            break;
        case UI_DECK_ACTIONS:
            Screen(UI_DECKS);
            U.cursor = U.deck;
            break;
        case UI_ONLINE_WAIT:
        case UI_ONLINE_INVITE:
            TcgLink_Cancel(Tcg_Online(), "Card session cancelled.");
            U.online = 0;
            Screen(UI_ONLINE);
            break;
        case UI_FIELD:
        case UI_DISCARD:
            Screen(UI_CHECK);
            break;
        case UI_CHECK:
        case UI_POWERS:
        case UI_HAND:
        case UI_TARGET:
        case UI_ATTACKS:
            Screen(UI_DUEL);
            break;
        case UI_DUEL:
            Screen(UI_CONFIRM_QUIT);
            break;
        default:
            Screen(UI_CASE);
            break;
        }
        return 1;
    }
    if (U.screen == UI_INSPECT)
    {
        if (pressed & KEY_UP)
            U.inspectScroll -= 12;
        if (pressed & KEY_DOWN)
            U.inspectScroll += 12;
        if (U.inspectScroll < 0)
            U.inspectScroll = 0;
        if (U.inspectScroll > U.inspectMax)
            U.inspectScroll = U.inspectMax;
        if (pressed & KEY_A)
            Choose();
        return 1;
    }
    if (U.screen == UI_ART)
    {
        if (pressed & KEY_UP)
            U.panY -= 8;
        if (pressed & KEY_DOWN)
            U.panY += 8;
        if (pressed & KEY_LEFT)
            U.panX -= 8;
        if (pressed & KEY_RIGHT)
            U.panX += 8;
        if (U.panY < 0)
            U.panY = 0;
        if (U.panY > 80)
            U.panY = 80;
        if (U.panX < -40)
            U.panX = -40;
        if (U.panX > 40)
            U.panX = 40;
        if (pressed & KEY_A)
            Choose();
        return 1;
    }
    if (U.screen == UI_EDIT)
    {
        char reason[160];
        unsigned id;
        if (pressed & KEY_SELECT)
        {
            U.editing = !U.editing;
            U.cursor = 0;
            U.message[0] = 0;
        }
        if (pressed & (KEY_L | KEY_R))
        {
            U.filter = (U.filter + (pressed & KEY_R ? 1 : 8)) % 9;
            U.cursor = 0;
            U.message[0] = 0;
        }
        if (pressed & KEY_START)
        {
            if (Tcg_DeckValidate(Tcg_GetCollection()->decks[U.deck].cards, Tcg_GetCollection()->owned, reason,
                                 sizeof(reason)))
                Message("This deck is ready to play.");
            else
                Message("%s", reason);
        }
        if (pressed & KEY_UP && U.cursor > 0)
            U.cursor--;
        if (pressed & KEY_DOWN && U.cursor + 1 < (int)CollectionSize())
            U.cursor++;
        id = CollectionCard(U.cursor);
        if (id && (pressed & KEY_LEFT))
            Tcg_DeckRemove(Tcg_GetCollection(), U.deck, id);
        if (id && (pressed & KEY_RIGHT))
            if (!Tcg_DeckAdd(Tcg_GetCollection(), U.deck, id, reason, sizeof(reason)))
                Message("%s", reason);
        if (U.cursor >= (int)CollectionSize() && U.cursor)
            U.cursor--;
        if (pressed & KEY_A)
            Choose();
        return 1;
    }
    if (pressed & KEY_UP)
        U.cursor--;
    if (pressed & KEY_DOWN)
        U.cursor++;
    if (pressed & KEY_LEFT)
        U.cursor -= U.screen == UI_DUEL && U.duel.phase == TCG_TURN ? 2 : 1;
    if (pressed & KEY_RIGHT)
        U.cursor += U.screen == UI_DUEL && U.duel.phase == TCG_TURN ? 2 : 1;
    if (pressed & KEY_L)
        U.cursor -= 10;
    if (pressed & KEY_R && U.screen != UI_EDIT)
        U.cursor += 10;
    {
        int count = MenuSize();
        if (count > 0)
        {
            while (U.cursor < 0)
                U.cursor += count;
            U.cursor %= count;
        }
        else
            U.cursor = 0;
    }
    if (pressed & KEY_SELECT)
    {
        if (U.screen == UI_HAND && U.duel.players[0].handSize)
            Inspect(U.duel.players[0].cards[U.duel.players[0].hand[U.cursor]], UI_HAND);
        else if (U.screen == UI_COLLECTION)
            Inspect(CollectionCard(U.cursor), UI_COLLECTION);
        else if (U.screen == UI_REVEAL)
            Inspect(U.revealed[U.cursor], UI_REVEAL);
        else if (U.screen == UI_DUEL && TcgDuel_Field(&U.duel, 1, 0))
            Inspect(TcgDuel_Field(&U.duel, 1, 0)->id, UI_DUEL);
    }
    if (pressed & KEY_START && U.screen == UI_DUEL && U.duel.phase == TCG_SETUP)
    {
        struct TcgAction a = {0};
        a.type = TCG_FINISH_SETUP;
        Apply(&a);
    }
    if (pressed & KEY_START && U.screen == UI_DUEL && U.duel.phase == TCG_TURN && TcgDuel_Field(&U.duel, 0, 0))
        Inspect(TcgDuel_Field(&U.duel, 0, 0)->id, UI_DUEL);
    if (pressed & KEY_A)
        Choose();
    return 1;
}
int Tcg_HandleEvent(const union SDL_Event *event)
{
    (void)event;
    return 0;
}
static void Board(SDL_Renderer *r)
{
    unsigned who, s;
    char line[80];
    Mat(r);
    for (who = 0; who < 2; who++)
    {
        const struct TcgPlayer *p = &U.duel.players[who];
        int y = who ? 3 : 54;
        Text(r, 7, y, 0x183820,
             who ? (U.duel.current == 1 ? "THEIR TURN" : "OPPONENT") :
                   (U.duel.current == 0 ? "YOUR TURN" : "YOU"), 85);
        for (s = 1; s < TCG_FIELD_SIZE; s++)
        {
            const struct TcgCard *c = TcgDuel_Field(&U.duel, who, s);
            if (c)
                Card(r, c->id, 7 + (s - 1) * 18, y + 13, 15, 21, 0);
            else
                Fill(r, 7 + (s - 1) * 18, y + 13, 15, 21, 0x507058);
        }
        const struct TcgCard *c = TcgDuel_Field(&U.duel, who, 0);
        if (c)
        {
            unsigned hp = c->hp > p->field[0].damage ? c->hp - p->field[0].damage : 0;
            Card(r, c->id, 106, y + 2, 28, 39, 0);
            snprintf(line, sizeof(line), "%u/%u HP", hp, c->hp);
            Text(r, 96, y + 41, 0x183820, line, 57);
            Text(r, 159, y + 2, 0x183820, c->name, 75);
        }
        snprintf(line, sizeof(line), "DECK %u", p->deckSize);
        Text(r, 159, y + 16, 0x183820, line, 75);
        snprintf(line, sizeof(line), "HAND %u", p->handSize);
        Text(r, 159, y + 28, 0x183820, line, 75);
        snprintf(line, sizeof(line), "PRIZES %u", p->prizeSize);
        Text(r, 159, y + 40, 0x183820, line, 75);
    }
    Window(r, 0, 108, 240, 52);
    if (U.duel.phase == TCG_FINISHED)
    {
        Text(r, 8, 117, 0x383838, U.duel.winner == 0 ? "YOU WIN! A: RETURN" : "DUEL OVER. A: RETURN", 224);
    }
    else if (U.duel.phase == TCG_PROMOTION)
    {
        Text(r, 8, 117, 0x383838, "Choose your next Active Pokemon.", 224);
        for (s = 1; s < TCG_FIELD_SIZE; s++)
        {
            snprintf(line, sizeof(line), "%u", s);
            Option(r, s, 20 + (s - 1) * 42, 135, line);
        }
    }
    else if (U.duel.phase == TCG_SETUP)
    {
        Option(r, 0, 16, 117, "HAND");
        Text(r, 96, 117, 0x383838, "START: READY", 132);
        Text(r, 8, 135, 0x383838, "Choose an Active Pokemon and Bench.", 224);
    }
    else
    {
        /* Same column-major layout as DuelMenuData in the original core.asm. */
        static const char *const labels[] = {"HAND", "ATTACK", "CHECK", "PKMN POWER", "RETREAT", "DONE"};
        for (s = 0; s < 6; s++)
            Option(r, s, 16 + (s / 2) * 76, 117 + (s % 2) * 18, labels[s]);
    }
    if (U.message[0])
    {
        Window(r, 0, 104, 240, 56);
        Text(r, 8, 111, 0x383838, U.message, 224);
    }
}
void Tcg_Render(struct SDL_Renderer *r, int width, int height)
{
    struct TcgCollection *c;
    SDL_Texture *previous;
    SDL_Rect oldViewport, oldClip;
    SDL_bool clipped;
    float oldX, oldY;
    char line[160];
    unsigned i;
    if (!Tcg_IsActive())
        return;
    if (sRenderer && sRenderer != r)
        Tcg_DestroyTextures();
    sRenderer = r;
    Font(r);
    if (!sCanvas)
    {
        sCanvas = SDL_CreateTexture(r, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 240, 160);
        if (!sCanvas)
            return;
        SDL_SetTextureBlendMode(sCanvas, SDL_BLENDMODE_BLEND);
        SDL_SetTextureScaleMode(sCanvas, SDL_ScaleModeNearest);
    }
    previous = SDL_GetRenderTarget(r);
    SDL_RenderGetViewport(r, &oldViewport);
    clipped = SDL_RenderIsClipEnabled(r);
    SDL_RenderGetClipRect(r, &oldClip);
    SDL_RenderGetScale(r, &oldX, &oldY);
    SDL_SetRenderTarget(r, sCanvas);
    SDL_RenderSetViewport(r, NULL);
    SDL_RenderSetClipRect(r, NULL);
    SDL_RenderSetScale(r, 1, 1);
    SDL_SetRenderDrawColor(r, 0, 0, 0, 0);
    SDL_RenderClear(r);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    c = Tcg_GetCollection();
    switch (U.screen)
    {
    case UI_INVITE:
        Window(r, 176, 48, 64, 48);
        Option(r, 0, 192, 55, "YES");
        Option(r, 1, 192, 72, "NO");
        Window(r, 0, 104, 240, 56);
        Text(r, 8, 112, 0x383838,
             "Would you like a CARD CASE and\na free starter deck? Card duels\nare an entirely optional pastime.", 224);
        break;
    case UI_STARTER:
    {
        static const unsigned ids[] = {TCG_CARD_CHARMANDER, TCG_CARD_SQUIRTLE, TCG_CARD_BULBASAUR};
        static const char *const names[] = {"FIRE", "WATER", "GRASS"};
        Mat(r);
        Header(r, "CHOOSE A DECK", NULL);
        for (i = 0; i < 3; i++)
        {
            int x = i * 80;
            Window(r, x, 32, 80, 104);
            Card(r, ids[i], x + 16, 42, 48, 66, 0);
            if ((int)i == U.cursor)
                Cursor(r, x + 9, 117);
            NativeText(r, x + 21, 112, 0x383838, names[i], 54, 0);
        }
        Footer(r, "A: CHOOSE   B: CANCEL");
        break;
    }
    case UI_CASE:
    {
        static const char *const names[] = {"CHALLENGE",       "COLLECTION",   "DECKS",
                                            "BOOSTER PACKS",   "PRACTICE",     "OWNED / ALL",
                                            "ANIMATION SPEED", "ONLINE CARDS", "PUT CASE AWAY"};
        unsigned selected = c->decks[c->activeDeck].cards[0];
        for (i = 0; i < TCG_DECK_SIZE; i++)
        {
            unsigned id = c->decks[c->activeDeck].cards[i];
            if (id && Tcg_Card(id)->kind == TCG_POKEMON)
            {
                selected = id;
                break;
            }
        }
        Mat(r);
        Header(r, "CARD CASE", "DEV");
        Window(r, 0, 32, 144, 104);
        Window(r, 144, 32, 96, 104);
        U.scroll = U.cursor / 5 * 5;
        for (i = 0; i < 5 && U.scroll + (int)i < 9; i++)
            Option(r, U.scroll + i, 16, 39 + 18 * i, names[U.scroll + i]);
        if (selected)
            Card(r, selected, 172, 42, 40, 55, 0);
        Text(r, 152, 100, 0x383838, c->decks[c->activeDeck].name, 80);
        snprintf(line, sizeof(line), "WINS %u", c->wins);
        Text(r, 152, 117, 0x383838, line, 80);
        Footer(r, "A: SELECT   B: CLOSE");
        break;
    }
    case UI_COLLECTION:
    {
        Mat(r);
        if (U.screen == UI_EDIT)
        {
            snprintf(line, sizeof(line), "%s %u/60", U.editing ? "IN DECK" : "ADD CARDS", DeckCount(U.deck, 0));
            Header(r, line, NULL);
        }
        else
            Header(r, "COLLECTION", NULL);
        Window(r, 0, 32, 152, 104);
        Window(r, 152, 32, 88, 104);
        U.scroll = U.cursor / 6 * 6;
        for (i = 0; i < 6; i++)
        {
            unsigned id = CollectionCard(U.scroll + i);
            if (!id)
                break;
            if ((int)(U.scroll + i) == U.cursor)
                Cursor(r, 8, 41 + i * 15);
            NativeText(r, 16, 36 + i * 15, 0x383838, Tcg_Card(id)->name, 112, 0);
            snprintf(line, sizeof(line), "%u", U.screen == UI_EDIT ? DeckCount(U.deck, id) : c->owned[id]);
            Text(r, 139 - Measure(line, 1), 39 + i * 15, 0x383838, line, 18);
        }
        unsigned id = CollectionCard(U.cursor);
        if (id)
        {
            Card(r, id, 164, 40, 64, 88, 0);
        }
        Footer(r, U.screen == UI_EDIT
                      ? (U.editing ? "A: REMOVE  SELECT: ADD  R: INFO" : "A: ADD  SELECT: DECK  R: INFO")
                      : "A: DETAILS   L/R: PAGE");
        break;
    }
    case UI_EDIT:
    {
        static const char *const labels[] = {"GR", "FI", "WA", "LI", "FT", "PS", "CO", "TR", "EN"};
        Mat(r);
        snprintf(line, sizeof(line), "%s %u/60", U.editing ? "IN DECK" : "BUILD DECK", DeckCount(U.deck, 0));
        Header(r, line, NULL);
        Window(r, 0, 32, 240, 24);
        for (i = 0; i < 9; i++)
        {
            unsigned count = 0, j;
            for (j = 0; j < TCG_DECK_SIZE; j++)
                count += CardFilter(c->decks[U.deck].cards[j], i);
            if ((int)i == U.filter)
                Fill(r, 6 + i * 25, 37, 25, 14, 0xc8d8e8);
            Text(r, 8 + i * 25, 37, 0x383838, labels[i], 24);
            snprintf(line, sizeof(line), "%u", count);
            Text(r, 29 + i * 25 - Measure(line, 1), 37, 0x385060, line, 14);
        }
        Window(r, 0, 56, 168, 80);
        U.scroll = U.cursor / 4 * 4;
        for (i = 0; i < 4; i++)
        {
            unsigned id = CollectionCard(U.scroll + i);
            if (!id)
                break;
            const struct TcgCard *card = Tcg_Card(id);
            if ((int)(U.scroll + i) == U.cursor)
                Cursor(r, 8, 65 + i * 17);
            if (card->kind == TCG_POKEMON)
                snprintf(line, sizeof(line), "%s Lv%u", card->name, card->level);
            else
                snprintf(line, sizeof(line), "%s", card->name);
            Text(r, 16, 61 + i * 17, 0x383838, line, 105);
            if (id <= 6)
                snprintf(line, sizeof(line), "%u/--", DeckCount(U.deck, id));
            else
                snprintf(line, sizeof(line), "%u/%u", DeckCount(U.deck, id), c->owned[id]);
            Text(r, 158 - Measure(line, 1), 61 + i * 17, 0x383838, line, 40);
        }
        unsigned id = CollectionCard(U.cursor);
        if (id)
            DeckIllustration(r, id);
        else
            Text(r, 16, 64, 0x383838, "No cards in this category.", 145);
        Footer(r, "LEFT:- RIGHT:+ A:INFO L/R:TYPE");
        break;
    }
    case UI_INSPECT:
    {
        const struct TcgCard *card = Tcg_Card(U.card);
        Mat(r);
        Header(r, card ? card->name : "CARD", NULL);
        Window(r, 0, 32, 80, 104);
        Window(r, 80, 32, 160, 104);
        Card(r, U.card, 8, 40, 64, 88, 0);
        SDL_Rect clip = {88, 38, 144, 90};
        SDL_RenderSetClipRect(r, &clip);
        int y = 38 - U.inspectScroll;
        if (card)
        {
            if (card->kind == TCG_POKEMON)
            {
                snprintf(line, sizeof(line), "HP %u  RETREAT %u", card->hp, card->retreat);
                y = Text(r, 88, y, 0x383838, line, 142) + 3;
                for (i = 0; i < 2; i++)
                    if (card->attacks[i].name[0])
                    {
                        y = NativeText(r, 88, y, 0x385060, card->attacks[i].name, 142, 0);
                        y = Text(r, 88, y, 0x383838, card->attacks[i].description, 142) + 5;
                    }
            }
            else
                y = Text(r, 88, y, 0x383838, card->description, 142);
        }
        U.inspectMax = y + U.inspectScroll - 128;
        if (U.inspectMax < 0)
            U.inspectMax = 0;
        SDL_RenderSetClipRect(r, NULL);
        Footer(r, "A: LARGE CARD   UP/DOWN: TEXT");
        break;
    }
    case UI_ART:
    {
        Mat(r);
        Window(r, 0, 0, 240, 160);
        Footer(r, "A: ZOOM  ARROWS: PAN  B: BACK");
        break;
    }
    case UI_DECKS:
    case UI_COPY_DECK:
        Mat(r);
        Header(r, U.screen == UI_COPY_DECK ? "COPY TO WHICH SLOT?" : "SAVED DECKS", NULL);
        Window(r, 0, 32, 240, 104);
        U.scroll = U.cursor / 5 * 5;
        for (i = 0; i < 5 && U.scroll + (int)i < TCG_SAVED_DECKS; i++)
        {
            unsigned d = U.scroll + i;
            snprintf(line, sizeof(line), "%s%s", d == c->activeDeck ? "* " : "",
                     c->decks[d].name[0] ? c->decks[d].name : "EMPTY DECK");
            Option(r, d, 16, 39 + 18 * i, line);
        }
        Footer(r, "A: SELECT   B: BACK");
        break;
    case UI_DECK_ACTIONS:
        Mat(r);
        Header(r, c->decks[U.deck].name[0] ? c->decks[U.deck].name : "EMPTY DECK", NULL);
        Window(r, 0, 32, 240, 104);
        Option(r, 0, 20, 41, "SET ACTIVE");
        Option(r, 1, 20, 64, "EDIT CARDS");
        Option(r, 2, 20, 87, "COPY DECK");
        snprintf(line, sizeof(line), "%u/60 CARDS", DeckCount(U.deck, 0));
        Text(r, 147, 44, 0x383838, line, 85);
        Footer(r, "A: SELECT   B: BACK");
        break;
    case UI_COPY_CONFIRM:
        Mat(r);
        Header(r, "REPLACE DECK?", NULL);
        Window(r, 0, 32, 240, 104);
        Text(r, 8, 39, 0x383838, "Replace this saved deck list?\nYour owned cards stay available.", 224);
        Option(r, 0, 20, 83, "CANCEL");
        Option(r, 1, 20, 106, "REPLACE");
        Footer(r, "A: SELECT   B: CANCEL");
        break;
    case UI_ONLINE:
        Mat(r);
        Header(r, "ONLINE CARDS", NULL);
        Window(r, 0, 32, 240, 104);
        for (i = 0; i < MP_MAX_PLAYERS; i++)
        {
            snprintf(line, sizeof(line), "%s%s", i == (unsigned)gMpLocalId ? "YOU: " : "",
                     gMpPlayers[i].present ? (const char *)gMpPlayers[i].name : "---");
            Option(r, i, 16, 39 + i * 20, line);
        }
        Footer(r, "A: INVITE   B: BACK");
        break;
    case UI_ONLINE_INVITE:
        Mat(r);
        Header(r, "CARD DUEL INVITATION", NULL);
        Window(r, 0, 32, 240, 104);
        snprintf(line, sizeof(line), "%s would like a card duel.", gMpPlayers[Tcg_Online()->peer].name);
        Text(r, 8, 40, 0x383838, line, 224);
        Text(r, 8, 65, 0x383838, "No cards or money are wagered.", 224);
        Option(r, 0, 20, 87, "ACCEPT");
        Option(r, 1, 20, 108, "DECLINE");
        Footer(r, "A: SELECT   B: DECLINE");
        break;
    case UI_ONLINE_WAIT:
        Mat(r);
        Header(r, "ONLINE CARDS", NULL);
        Window(r, 0, 32, 240, 104);
        Text(r, 8, 42, 0x383838,
             "Waiting for the other player...\nBoth players need this dev build\nand ONLINE CARDS open.", 224);
        Footer(r, "B: CANCEL");
        break;
    case UI_CHECK:
    {
        static const char *const labels[] = {"YOUR POKEMON", "OPPONENT'S POKEMON", "YOUR DISCARD PILE",
                                             "OPPONENT'S DISCARD", "YOUR HAND"};
        Mat(r);
        Header(r, "CHECK PLAY AREA", NULL);
        Window(r, 0, 32, 240, 104);
        for (i = 0; i < 5; i++)
            Option(r, i, 16, 39 + i * 18, labels[i]);
        Footer(r, "A: CHECK   B: DUEL");
        break;
    }
    case UI_FIELD:
    case UI_POWERS:
    {
        unsigned who = U.screen == UI_POWERS ? 0 : U.viewWho;
        Mat(r);
        Header(r, U.screen == UI_POWERS ? "POKEMON POWERS" : who ? "OPPONENT'S POKEMON" : "YOUR POKEMON", NULL);
        Window(r, 0, 32, 160, 104);
        Window(r, 160, 32, 80, 104);
        for (i = 0; i < TCG_FIELD_SIZE; i++)
        {
            const struct TcgCard *card = TcgDuel_Field(&U.duel, who, i);
            snprintf(line, sizeof(line), "%s %s", i ? "B" : "A", card ? card->name : "---");
            Option(r, i, 16, 37 + i * 15, line);
        }
        const struct TcgCard *card = TcgDuel_Field(&U.duel, who, U.cursor);
        if (card)
        {
            const struct TcgFieldCard *f = &U.duel.players[who].field[U.cursor];
            Card(r, card->id, 177, 40, 45, 62, 0);
            snprintf(line, sizeof(line), "HP %u/%u", card->hp > f->damage ? card->hp - f->damage : 0, card->hp);
            Text(r, 167, 104, 0x383838, line, 65);
            snprintf(line, sizeof(line), "ENERGY %u", TcgDuel_Energy(&U.duel, who, U.cursor, TCG_NONE));
            Text(r, 167, 116, 0x383838, line, 65);
        }
        Footer(r, U.screen == UI_POWERS ? "A: USE POWER   B: DUEL" : "A: DETAILS   B: CHECK MENU");
        break;
    }
    case UI_DISCARD:
    {
        const struct TcgPlayer *p = &U.duel.players[U.viewWho];
        Mat(r);
        Header(r, U.viewWho ? "OPPONENT'S DISCARD" : "YOUR DISCARD PILE", NULL);
        Window(r, 0, 32, 152, 104);
        Window(r, 152, 32, 88, 104);
        U.scroll = U.cursor / 6 * 6;
        for (i = 0; i < 6 && U.scroll + i < p->discardSize; i++)
        {
            unsigned id = p->cards[p->discard[U.scroll + i]];
            Option(r, U.scroll + i, 16, 39 + i * 15, Tcg_Card(id)->name);
        }
        if (p->discardSize)
            Card(r, p->cards[p->discard[U.cursor]], 164, 40, 64, 88, 0);
        else
            Text(r, 16, 40, 0x383838, "No discarded cards.", 128);
        Footer(r, "A: DETAILS   B: CHECK MENU");
        break;
    }
    case UI_PACKS:
    {
        static const char *const names[] = {"COLOSSEUM", "EVOLUTION", "MYSTERY", "LABORATORY"};
        Mat(r);
        Header(r, "BOOSTER PACKS", NULL);
        Window(r, 0, 32, 240, 104);
        for (i = 0; i < 4; i++)
        {
            Option(r, i, 20, 42 + i * 21, names[i]);
            snprintf(line, sizeof(line), "x%u", c->packs[i]);
            Text(r, 196, 45 + i * 21, 0x383838, line, 32);
        }
        Footer(r, "A: OPEN PACK   B: BACK");
        break;
    }
    case UI_REVEAL:
        Mat(r);
        Header(r, "NEW CARDS", NULL);
        for (i = 0; i < 10; i++)
        {
            int x = 10 + (i % 5) * 46, y = 34 + (i / 5) * 49;
            float t = (float)(U.age - (int)i * 2) / (c->fastAnimations ? 4 : 16);
            if ((int)U.age < (int)i * 2)
                t = 0;
            if (t > 1)
                t = 1;
            t = t * t * (3 - 2 * t);
            if ((int)i == U.cursor)
                Fill(r, x - 1, y - 1, 34, 46, 0xf8d858);
            Card(r, U.revealed[i], x, y + (int)((1 - t) * 12), 32, 44, 0);
        }
        Footer(r, "A: DETAILS   B: CARD CASE");
        break;
    case UI_DUEL:
        Board(r);
        break;
    case UI_HAND:
        Mat(r);
        Header(r, "YOUR HAND", NULL);
        Window(r, 0, 32, 152, 104);
        Window(r, 152, 32, 88, 104);
        U.scroll = U.cursor / 6 * 6;
        for (i = 0; i < 6; i++)
        {
            unsigned at = U.scroll + i;
            if (at >= U.duel.players[0].handSize)
                break;
            unsigned id = U.duel.players[0].cards[U.duel.players[0].hand[at]];
            Option(r, at, 16, 36 + i * 15, Tcg_Card(id)->name);
        }
        if (U.duel.players[0].handSize)
            Card(r, U.duel.players[0].cards[U.duel.players[0].hand[U.cursor]], 164, 40, 64, 88, 0);
        Footer(r, "A: PLAY   SELECT: DETAILS");
        break;
    case UI_TARGET:
        Mat(r);
        Header(r, "CHOOSE A POKEMON", NULL);
        Window(r, 0, 32, 240, 104);
        for (i = 0; i < TCG_FIELD_SIZE; i++)
        {
            const struct TcgCard *card = TcgDuel_Field(&U.duel, 0, i);
            snprintf(line, sizeof(line), "%s %s", i ? "BENCH" : "ACTIVE", card ? card->name : "---");
            Option(r, i, 16, 35 + 15 * i, line);
        }
        Footer(r, "A: SELECT   B: CANCEL");
        break;
    case UI_ATTACKS:
    {
        const struct TcgCard *card = TcgDuel_Field(&U.duel, 0, 0);
        Mat(r);
        Header(r, "CHOOSE AN ATTACK", NULL);
        Window(r, 0, 32, 240, 104);
        if (card)
            for (i = 0; i < 2; i++)
            {
                Option(r, i, 16, 42 + i * 42, card->attacks[i].name);
                snprintf(line, sizeof(line), "DAMAGE %u", card->attacks[i].damage);
                Text(r, 24, 59 + i * 42, 0x586850, line, 190);
            }
        Footer(r, "A: ATTACK   B: CANCEL");
        break;
    }
    case UI_CONFIRM_QUIT:
        Mat(r);
        Header(r, "CONCEDE?", NULL);
        Window(r, 0, 32, 240, 104);
        Text(r, 8, 40, 0x383838, "You will not lose any cards,\nmoney or ordinary progress.", 224);
        Option(r, 0, 24, 82, "CONCEDE");
        Option(r, 1, 24, 104, "KEEP PLAYING");
        Footer(r, "A: SELECT   B: BACK");
        break;
    default:
        break;
    }
    if (U.message[0] && U.screen != UI_DUEL)
    {
        Window(r, 0, 112, 240, 48);
        Text(r, 8, 119, 0x383838, U.message, 224);
    }
    SDL_SetRenderTarget(r, previous);
    SDL_RenderSetViewport(r, &oldViewport);
    SDL_RenderSetClipRect(r, clipped ? &oldClip : NULL);
    SDL_RenderSetScale(r, oldX, oldY);
    int scale = width / 240;
    if (height / 160 < scale)
        scale = height / 160;
    if (scale < 1)
        scale = 1;
    SDL_Rect dest = {(width - 240 * scale) / 2, (height - 160 * scale) / 2, 240 * scale, 160 * scale};
    SDL_RenderCopy(r, sCanvas, NULL, &dest);
    if (U.screen == UI_ART)
    {
        struct ArtTexture *a = Art(r, U.card, 1);
        if (a && a->texture && !a->fallback)
        {
            SDL_Rect clip = {dest.x + 8 * scale, dest.y + 4 * scale, 224 * scale, 132 * scale};
            float fit = (float)(132 * scale) / a->h * (U.zoom ? 1.6f : 1.0f);
            SDL_Rect image = {dest.x + 120 * scale - (int)(a->w * fit) / 2 - U.panX * scale,
                              dest.y + 4 * scale - U.panY * scale, (int)(a->w * fit), (int)(a->h * fit)};
            SDL_RenderSetClipRect(r, &clip);
            SDL_SetTextureScaleMode(a->texture, SDL_ScaleModeLinear);
            SDL_RenderCopy(r, a->texture, NULL, &image);
            SDL_SetTextureScaleMode(a->texture, SDL_ScaleModeNearest);
            SDL_RenderSetClipRect(r, clipped ? &oldClip : NULL);
        }
    }
}

void Tcg_Debug(const char *command)
{
    struct TcgCollection *c = Tcg_GetCollection();
    unsigned n = 0;
    if (!strcmp(command, "challenge"))
    {
        printf("[tcg] field challenge accepted: %d\n", Tcg_TryChallenge());
        return;
    }
    if (!strcmp(command, "open"))
    {
        Tcg_OpenChallenge("Development Collector", "PracticePlayer", 0);
        return;
    }
    if (sscanf(command, "starter %u", &n) == 1)
    {
        Tcg_ChooseStarter(c, n % 3);
        Tcg_GrantCase();
        return;
    }
    if (sscanf(command, "inspect %u", &n) == 1)
    {
        Inspect(n, UI_CASE);
        return;
    }
    if (!strcmp(command, "collection"))
    {
        U.showAll = 1;
        Screen(UI_COLLECTION);
        return;
    }
    if (!strcmp(command, "packs"))
    {
        if (c->optedIn)
        {
            Tcg_AwardPack(c, 0);
            Screen(UI_PACKS);
        }
        return;
    }
    if (!strcmp(command, "online"))
    {
        Screen(UI_ONLINE);
        return;
    }
    if (sscanf(command, "invite %u", &n) == 1)
    {
        U.cursor = n % MP_MAX_PLAYERS;
        Screen(UI_ONLINE);
        U.cursor = n % MP_MAX_PLAYERS;
        Choose();
        return;
    }
    if (!strcmp(command, "netconcede"))
    {
        struct TcgAction a = {0};
        a.type = TCG_CONCEDE;
        TcgLink_Submit(Tcg_Online(), &a, SDL_GetTicks());
        return;
    }
    if (!strcmp(command, "netstep"))
    {
        struct TcgAction a;
        struct TcgLink *l = Tcg_Online();
        if (l->state == TCG_LINK_PLAYING && TcgDuel_AiAction(&l->game, &a))
            TcgLink_Submit(l, &a, SDL_GetTicks());
        printf("[tcg-net] state %u seq %u phase %u hash %08x winner %u\n", l->state, l->sequence, l->game.phase,
               TcgLink_Hash(&l->game), l->game.winner);
        return;
    }
    if (!strcmp(command, "ready"))
    {
        /* Harness helper: use legal AI actions for setup, then wait for the
         * player's first turn. It never relocates or reveals hidden cards. */
        struct TcgAction a;
        unsigned steps = 0;
        while (steps++ < 128 && U.duel.phase != TCG_FINISHED && (U.duel.phase == TCG_SETUP || U.duel.current == 1))
            if (!TcgDuel_AiAction(&U.duel, &a) || TcgDuel_Apply(&U.duel, &a) != TCG_OK)
                break;
        printf("[tcg] ready phase %u current %u conserved %d\n", U.duel.phase, U.duel.current,
               TcgDuel_Conserved(&U.duel));
        return;
    }
    if (!strcmp(command, "snapshot"))
    {
        const unsigned char *bytes = (const unsigned char *)&U.duel;
        uint32_t hash = 2166136261u;
        unsigned i;
        for (i = 0; i < sizeof(U.duel); i++)
            hash = (hash ^ bytes[i]) * 16777619u;
        printf("[tcg] rules snapshot %08x conserved %d\n", hash, TcgDuel_Conserved(&U.duel));
        return;
    }
    if (!strcmp(command, "ai"))
    {
        unsigned steps = 0;
        struct TcgAction a;
        while (U.duel.phase != TCG_FINISHED && steps++ < 2000)
        {
            if (!TcgDuel_AiAction(&U.duel, &a) || TcgDuel_Apply(&U.duel, &a) != TCG_OK)
                break;
        }
        printf("[tcg] AI test: %u actions, phase %u, winner %u\n", steps, U.duel.phase, U.duel.winner);
        return;
    }
    if (sscanf(command, "deck %u", &n) == 1 && n < TCG_SAVED_DECKS)
    {
        char reason[160];
        int valid = Tcg_DeckValidate(c->decks[n].cards, c->owned, reason, sizeof(reason));
        printf("[tcg] deck %u count %u valid %d active %u\n", n, DeckCount(n, 0), valid, c->activeDeck);
        return;
    }
    if (!strcmp(command, "state"))
        printf("[tcg] screen %d opted %u phase %u current %u wins %u\n", U.screen, c->optedIn, U.duel.phase,
               U.duel.current, c->wins);
}
#endif
