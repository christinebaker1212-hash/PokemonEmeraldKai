// STADIUM's themes (OPTIONS > BATTLE > STADIUM SETTINGS > STYLE): POKESTADIUM 2 and POKESTADIUM, the screens redone in
// POKéMON STADIUM 2's and the first STADIUM's looks (docs/POKESTADIUM_2_THEME_HANDOFF.md). Included by
// mp_stadium_ui.c (its state, U, and its pictures). Every still part comes from graphics/stadium/themes.bin, drawn by
// tools/stadium/restyle/build_themes.py with the mock-ups' own materials; the game puts its own things over them (the
// names, POKéMON, the cursor) at the places that script writes into src/data/stadium_themes.h. Coordinates here are
// real pixels of the 480x320 canvas (the original screens' are its 240x160 units).
#include "../data/stadium_themes.h"

extern const u8 gMpStadiumThemes[];
int uncompress(unsigned char *dest, unsigned long *destLen, const unsigned char *source, unsigned long sourceLen);

static struct
{
    bool8 indexed;
    u32 at[TH_COUNT];
    s16 x[TH_COUNT], y[TH_COUNT];
    u16 w[TH_COUNT], h[TH_COUNT];
    SDL_Texture *tex[TH_COUNT];
} sTh;

static u8 Theme(void)
{
    return gMpConfig.stadiumStyle % MP_STADIUM_STYLE_COUNT;
}

// (a new renderer: the old one's textures went with it)
static void ThReset(void)
{
    memset(sTh.tex, 0, sizeof(sTh.tex));
}

static void ThIndex(void)
{
    const u8 *p = gMpStadiumThemes;
    u16 n, i;
    u32 o = 6;
    sTh.indexed = TRUE;
    if (memcmp(p, "STTH", 4) != 0)
        return;
    n = p[4] | p[5] << 8;
    for (i = 0; i < n && i < TH_COUNT; i++)
    {
        const u8 *e = p + o;
        u32 z = e[8] | e[9] << 8 | e[10] << 16 | (u32)e[11] << 24;
        sTh.x[i] = (s16)(e[0] | e[1] << 8), sTh.y[i] = (s16)(e[2] | e[3] << 8);
        sTh.w[i] = e[4] | e[5] << 8, sTh.h[i] = e[6] | e[7] << 8;
        sTh.at[i] = o + 12;
        o += 12 + z;
    }
}

static SDL_Texture *Th(u16 id)
{
    if (id >= TH_COUNT)
        return NULL;
    if (!sTh.indexed)
        ThIndex();
    if (!sTh.tex[id] && sTh.w[id])
    {
        const u8 *e = gMpStadiumThemes + sTh.at[id] - 12;
        unsigned long size = (unsigned long)sTh.w[id] * sTh.h[id] * 4, zsize = e[8] | e[9] << 8 | e[10] << 16 | (u32)e[11] << 24;
        u8 *px = malloc(size);
        if (px && uncompress(px, &size, gMpStadiumThemes + sTh.at[id], zsize) == 0)
        {
            sTh.tex[id] = SDL_CreateTexture(U.owner, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, sTh.w[id], sTh.h[id]);
            if (sTh.tex[id])
            {
                SDL_UpdateTexture(sTh.tex[id], NULL, px, sTh.w[id] * 4);
                SDL_SetTextureBlendMode(sTh.tex[id], SDL_BLENDMODE_BLEND);
            }
        }
        free(px);
    }
    return sTh.tex[id];
}

// A piece where the script put it (DX, DY: moved by that many real pixels)
static void ThPutAt(u16 id, float dx, float dy, u8 alpha)
{
    SDL_Texture *t = Th(id);
    SDL_FRect d;
    if (!t)
        return;
    d.x = sTh.x[id] + dx, d.y = sTh.y[id] + dy, d.w = sTh.w[id], d.h = sTh.h[id];
    SDL_RenderSetScale(U.owner, 1, 1);
    SDL_SetTextureAlphaMod(t, alpha);
    SDL_RenderCopyF(U.owner, t, NULL, &d);
    SDL_SetTextureAlphaMod(t, 255);
    SDL_RenderSetScale(U.owner, 2, 2);
}

static void ThPut(u16 id)
{
    ThPutAt(id, 0, 0, 255);
}

// A piece centred on (CX, CY)
static void ThPutCentred(u16 id, float cx, float cy, u8 alpha)
{
    ThPutAt(id, cx - sTh.x[id] - sTh.w[id] / 2.0f, cy - sTh.y[id] - sTh.h[id] / 2.0f, alpha);
}

// A texture into a box (real pixels)
static void ThBlit(SDL_Texture *t, float x, float y, float w, float h, u8 alpha)
{
    Blit(t, x / 2, y / 2, w / 2, h / 2, alpha);
}

// The words' hard shadow, real pixels down and right
static int ThShadow(void)
{
    static int sPx;
    if (!sPx)
    {
        const char *e = getenv("MOEMON_TH_SHADOW");   // (1 or 2: to compare them)
        sPx = e && atoi(e) > 0 ? atoi(e) : 2;
    }
    return sPx;
}

// Writing at real pixels (X, Y the line's top), with a hard shadow in OUTLINE's colour (PX 0: none), BOLD, the
// face falling from COLOR to GRAD (G_NO_GRAD: none). ALIGN TA_LEFT / TA_CENTER / TA_RIGHT. Returns the width in pixels
enum { THF_OUTLINE_INK = 0x282c38 };
static int ThText(float x, float y, const char *s, int font, unsigned color, unsigned grad, unsigned outline, int px,
                  bool8 bold, int align)
{
    int w = G_Measure(s, font) * 2;
    float l = align == TA_CENTER ? x - w / 2.0f : align == TA_RIGHT ? x - w : x;
    int ux = (int)SDL_floorf(SDL_roundf(l) / 2), uy = (int)SDL_floorf(SDL_roundf(y) / 2);
    G_TextStyle(outline, px ? -ThShadow() : 0, bold, grad);
    G_TextNudge((SDL_roundf(l) - ux * 2) / 2, (SDL_roundf(y) - uy * 2) / 2);
    G_Text(ux, uy, color, G_NO_SHADOW, s, 0, font);
    G_TextNudge(0, 0);
    G_TextStyle(0, 0, 0, G_NO_GRAD);
    return w;
}


// Words wrapped to W pixels from (X, Y), LINE apart (ThText's style); returns the lines written
static int ThWrap(float x, float y, float w, float line, const char *s, int font, unsigned color, unsigned grad,
                  unsigned outline, int px, bool8 bold, int align)
{
    char buf[160], *words[40], cur[160];
    int n = 0, lines = 0, i;
    char *tok;
    snprintf(buf, sizeof(buf), "%s", s);
    for (tok = strtok(buf, " "); tok && n < 40; tok = strtok(NULL, " "))
        words[n++] = tok;
    cur[0] = 0;
    for (i = 0; i <= n; i++)
    {
        char next[160];
        if (i < n)
            snprintf(next, sizeof(next), "%s%s%s", cur, cur[0] ? " " : "", words[i]);
        if (i == n || (cur[0] && G_Measure(next, font) * 2 > w))
        {
            if (cur[0])
                ThText(x, y + lines++ * line, cur, font, color, grad, outline, px, bold, align);
            if (i == n)
                break;
            snprintf(cur, sizeof(cur), "%s", words[i]);
        }
        else
            snprintf(cur, sizeof(cur), "%s", next);
    }
    return lines;
}

// A box piece cut into nine and stretched to (X, Y, W, H) (real pixels): its corners EDGE wide kept as drawn, its
// shadow round it (PAD: how far the piece's box reaches past the drawn box)
static void ThNine(u16 id, float x, float y, float w, float h, int edge, int pad, u8 alpha)
{
    SDL_Texture *t = Th(id);
    int sw, sh, k, j, e = edge + pad;
    float dx[3], dw[3], dy[3], dh[3];
    int sx[3], sww[3], sy[3], shh[3];
    if (!t)
        return;
    sw = sTh.w[id], sh = sTh.h[id];
    sx[0] = 0, sww[0] = e, sx[1] = e, sww[1] = sw - 2 * e, sx[2] = sw - e, sww[2] = e;
    sy[0] = 0, shh[0] = e, sy[1] = e, shh[1] = sh - 2 * e, sy[2] = sh - e, shh[2] = e;
    dx[0] = x - pad, dw[0] = e, dx[1] = x - pad + e, dw[1] = w + 2 * pad - 2 * e, dx[2] = x + w + pad - e, dw[2] = e;
    dy[0] = y - pad, dh[0] = e, dy[1] = y - pad + e, dh[1] = h + 2 * pad - 2 * e, dy[2] = y + h + pad - e, dh[2] = e;
    SDL_RenderSetScale(U.owner, 1, 1);
    SDL_SetTextureAlphaMod(t, alpha);
    for (j = 0; j < 3; j++)
        for (k = 0; k < 3; k++)
        {
            SDL_Rect src = {sx[k], sy[j], sww[k], shh[j]};
            SDL_FRect d = {dx[k], dy[j], dw[k], dh[j]};
            SDL_RenderCopyF(U.owner, t, &src, &d);
        }
    SDL_SetTextureAlphaMod(t, 255);
    SDL_RenderSetScale(U.owner, 2, 2);
}

// A POKéMON's front picture at its own pixels in a window (X, Y, W, H real pixels), clipped to it
static void ThFrontIn(SDL_Texture *t, float x, float y, float w, float h, u8 alpha)
{
    SDL_Rect clip = {(int)(x / 2), (int)(y / 2), (int)(w / 2), (int)(h / 2)};
    SDL_RenderSetClipRect(U.owner, &clip);
    ThBlit(t, x + (w - 64) / 2, y + (h - 64) / 2, 64, 64, alpha);
    SDL_RenderSetClipRect(U.owner, NULL);
}

// ---------------------------------------------------------------------------------------------------------------
// POKESTADIUM 2

// A rule chip (screens.py chip): the capsule of its colour cut to the word, the word in the narrow font. Returns its
// width in pixels
static int S2Chip(float x, float y, const char *s, u16 chip)
{
    SDL_Texture *t = Th(chip);
    int w = (G_Measure(s, TF_NARROW) + 7) * 2;
    const int capL = 16, capR = 18;   // (the capsule's ends in the piece: 4 px of shadow round it)
    if (t)
    {
        SDL_Rect src;
        SDL_FRect d;
        SDL_RenderSetScale(U.owner, 1, 1);
        src = (SDL_Rect){0, 0, capL, sTh.h[chip]}, d = (SDL_FRect){x - 4, y - 4, capL, sTh.h[chip]};
        SDL_RenderCopyF(U.owner, t, &src, &d);
        src = (SDL_Rect){sTh.w[chip] / 2, 0, 2, sTh.h[chip]}, d = (SDL_FRect){x - 4 + capL, y - 4, w + 8 - capL - capR, sTh.h[chip]};
        SDL_RenderCopyF(U.owner, t, &src, &d);
        src = (SDL_Rect){sTh.w[chip] - capR, 0, capR, sTh.h[chip]}, d = (SDL_FRect){x + w + 4 - capR, y - 4, capR, sTh.h[chip]};
        SDL_RenderCopyF(U.owner, t, &src, &d);
        SDL_RenderSetScale(U.owner, 2, 2);
    }
    ThText(x + w / 2.0f + 1, y - 2, s, TF_NARROW, G_WHITE, G_NO_GRAD, 0x101420, 1, FALSE, TA_CENTER);
    return w;
}

// The words in a glass box (screens.py glass_text): white falling to grey, a black line round them, bold
static void S2GlassText(int x0, int y0, int x1, int y1, const char *s)
{
    int font = G_Measure(s, TF_SMALL) * 2 <= (x1 - x0) - 18 ? TF_SMALL : TF_NARROW;
    ThText((x0 + x1) / 2.0f, (y0 + y1) / 2.0f - 13, s, font, 0xffffff, 0xd0d4de, 0x000000, 1, TRUE, TA_CENTER);
}

// The foot: the hints with the controller's own buttons, or a note in their place (why not, what happened)
static void S2Foot(const char *hint)
{
    if (U.note[0])
    {
        ThText(8, 296, U.note, TF_SMALL, 0xf8e8a0, G_NO_GRAD, 0x000000, 1, FALSE, TA_LEFT);
        return;
    }
    G_TextStyle(0x000000, -ThShadow(), TRUE, 0xd0d4de);
    G_Hint(4, 148, 0xffffff, G_NO_SHADOW, hint, TF_SMALL, 1);
    G_TextStyle(0, 0, 0, G_NO_GRAD);
}

static void S2Select(void)
{
    u8 i, t;
    char line[96];
    static const u16 sGold[7] = {TH_S2_CUPROW_GOLD0, TH_S2_CUPROW_GOLD1, TH_S2_CUPROW_GOLD2, TH_S2_CUPROW_GOLD3,
                                 TH_S2_CUPROW_GOLD4, TH_S2_CUPROW_GOLD5, TH_S2_CUPROW_GOLD6};
    static const u16 sGrey[7] = {TH_S2_CUPROW_GREY0, TH_S2_CUPROW_GREY1, TH_S2_CUPROW_GREY2, TH_S2_CUPROW_GREY3,
                                 TH_S2_CUPROW_GREY4, TH_S2_CUPROW_GREY5, TH_S2_CUPROW_GREY6};
    static const u16 sRing[7] = {TH_S2_CUPRING0, TH_S2_CUPRING1, TH_S2_CUPRING2, TH_S2_CUPRING3, TH_S2_CUPRING4,
                                 TH_S2_CUPRING5, TH_S2_CUPRING6};
    static const s16 sRowY[7][2] = {{SY_S2_CUP0_Y0, SY_S2_CUP0_Y1}, {SY_S2_CUP1_Y0, SY_S2_CUP1_Y1}, {SY_S2_CUP2_Y0, SY_S2_CUP2_Y1},
                                    {SY_S2_CUP3_Y0, SY_S2_CUP3_Y1}, {SY_S2_CUP4_Y0, SY_S2_CUP4_Y1}, {SY_S2_CUP5_Y0, SY_S2_CUP5_Y1},
                                    {SY_S2_CUP6_Y0, SY_S2_CUP6_Y1}};
    ThPut(TH_S2_SELECT);
    snprintf(line, sizeof(line), "ROUND %d", U.round + 1);
    S2GlassText(SX_S2_ROUND_X0, SX_S2_ROUND_Y0, SX_S2_ROUND_X1, SX_S2_ROUND_Y1, line);
    // the cups: the row under the cursor in gold, one not open in grey, the classes won as spheres at the right
    for (i = 0; i < STADIUM_CUP_COUNT && i < 7; i++)
    {
        bool8 open = StadiumCup_Open(U.round, i, 0);
        u8 tiers = StadiumCup_Info(i)->tiers;
        float cy = (sRowY[i][0] + sRowY[i][1]) / 2.0f;
        if (U.cup == i)
            ThPut(sGold[i]);
        else if (!open)
            ThPut(sGrey[i]);
        ThText(SX_S2_CUPS_X0 + 8, cy - 13, CupLabel(i), TF_SMALL, open ? 0xffffff : 0xc4c8d2, open ? 0xdfe4f0 : G_NO_GRAD,
               THF_OUTLINE_INK, 2, FALSE, TA_LEFT);
        for (t = 0; t < tiers; t++)
        {
            u16 c = StadiumCup_Save()->cleared[U.round][i][t];
            ThPutCentred((c & (1 << STADIUM_HARD_ALL)) ? TH_S2_MARK_GOLD : c ? TH_S2_MARK_SILVER : TH_S2_MARK_EMPTY,
                         SX_S2_CUPS_X1 - 49 + (4 - tiers + t) * 13, cy, 255);
        }
    }
    if (U.cup < 7)
        ThPut(sRing[U.cup]);
    // the cup's picture in the portrait's frame, a light passing over it
    {
        SDL_Texture *pic = CupPic(U.cup);
        bool8 open = StadiumCup_Open(U.round, U.cup, 0);
        const int px = SX_S2_PIC_X0, py = SX_S2_PIC_Y0, pw = SX_S2_PIC_X1 - SX_S2_PIC_X0, ph = SX_S2_PIC_Y1 - SX_S2_PIC_Y0;
        int sweep = (U.age * 6) % 520 - 120, k;
        if (pic)
            ThBlit(pic, px, py, pw, ph, open ? 255 : 90);
        else
            CupScene(U.cup, px / 2, py / 2, pw / 2, ph / 2, open);
        if (open && sweep > -40 && sweep < pw)
            for (k = 0; k < ph; k += 2)
            {
                int sx = px + sweep + (ph - k) * 45 / 100;
                if (sx >= px && sx < px + pw - 12)
                    G_FillAdd(sx / 2, (py + k) / 2, 6, 1, 0xffffff, 30);
            }
        if (!open)
        {
            G_FillA(px / 2, py / 2 + ph / 4 - 9, pw / 2, 18, 0x000000, 160);
            ThText(px + pw / 2.0f, py + ph / 2.0f - 15, "LOCKED", TF_NORMAL, 0xffffff, 0xd0d4de, 0x000000, 2, FALSE, TA_CENTER);
        }
    }
    // its rules: chips and a line under them
    {
        const struct StadiumCupInfo *c = StadiumCup_Info(U.cup);
        float x = SX_S2_RULES_X0 + 4, y = SX_S2_RULES_Y0 + 4;
        const float ly = SX_S2_RULES_Y0 + 27;
        if (!StadiumCup_Open(U.round, U.cup, 0))
            ThText(SX_S2_RULES_X0 + 6, SX_S2_RULES_Y0 + 12, U.round ? "Beat MEWTWO in Round 1." : "Win every cup first.",
                   TF_SMALL, 0xffd0c0, G_NO_GRAD, 0x1c2c20, 2, FALSE, TA_LEFT);
        else
        {
            if (U.cup == STADIUM_CUP_CHALLENGE)
            {
                x += S2Chip(x, y, "6 LENT", TH_S2_CHIP_BLUE) + 4;
                x += S2Chip(x, y, "Lv 30-75", TH_S2_CHIP_GREEN) + 4;
                S2Chip(x, y, "PICK 3", TH_S2_CHIP_ORANGE);
                snprintf(line, sizeof(line), "Random teams on both sides.");
            }
            else
            {
                if (c->levelSum)
                    snprintf(line, sizeof(line), "Lv %d-%d", c->levelMin, c->levelMax);
                else
                    snprintf(line, sizeof(line), "ANY LEVEL");
                x += S2Chip(x, y, line, TH_S2_CHIP_GREEN) + 4;
                if (c->levelSum)
                {
                    snprintf(line, sizeof(line), "SUM %d", c->levelSum);
                    x += S2Chip(x, y, line, TH_S2_CHIP_BLUE) + 4;
                }
                if (U.cup != STADIUM_CUP_MEWTWO)
                    S2Chip(x, y, U.cup == STADIUM_CUP_CASTLE ? "8 GYMS" : "8 FOES", TH_S2_CHIP_ORANGE);
                snprintf(line, sizeof(line), "%s", c->petit ? "2.0 m, 20 kg or less, lowest stage."
                         : c->allowsMewtwo ? "MEWTWO and MEW allowed." : "No MEWTWO or MEW.");
            }
            ThText(SX_S2_RULES_X0 + 6, ly, line, TF_NARROW, 0xffffff, G_NO_GRAD, 0x1c2c20, 2, FALSE, TA_LEFT);
        }
    }
    S2Foot("{A} ENTER  {L}{R} ROUND  {B} EXIT");
}


// The classes: STADIUM's ball banners, the one under the cursor moved out in a gold ring; on the right its trainers and
// the best won there
static void S2Class(void)
{
    static const u16 sLit[4] = {TH_S2_BALL0_LIT, TH_S2_BALL1_LIT, TH_S2_BALL2_LIT, TH_S2_BALL3_LIT};
    static const u16 sGreyB[4] = {TH_S2_BALL0_GREY, TH_S2_BALL1_GREY, TH_S2_BALL2_GREY, TH_S2_BALL3_GREY};
    static const u16 sRingB[4] = {TH_S2_BALLRING0, TH_S2_BALLRING1, TH_S2_BALLRING2, TH_S2_BALLRING3};
    static const s16 sBallY[4] = {SY_S2_BALL0, SY_S2_BALL1, SY_S2_BALL2, SY_S2_BALL3};
    u8 t, i;
    char line[64];
    const float ux = SX_S2_CLASS_X0 + 6;
    ThPut(TH_S2_CLASS);
    S2GlassText(SX_S2_CUPNAME_X0, SX_S2_CUPNAME_Y0, SX_S2_CUPNAME_X1, SX_S2_CUPNAME_Y1, StadiumCup_Info(U.cup)->name);
    S2GlassText(SX_S2_ROUND_X0, SX_S2_ROUND_Y0, SX_S2_ROUND_X1, SX_S2_ROUND_Y1, U.round ? "ROUND 2" : "ROUND 1");
    for (t = 0; t < 4; t++)
    {
        bool8 open = StadiumCup_Open(U.round, U.cup, t), focus = U.tier == t;
        ThPutAt(open ? sLit[t] : sGreyB[t], focus ? 12 : 0, 0, 255);
        if (focus)
            ThPut(sRingB[t]);
        if (open && StadiumCup_Cleared(U.round, U.cup, t))
        {
            int w = G_Measure("WON", TF_NARROW) + 9;
            ThPutAt(TH_S2_WON, 248 - 10, sBallY[t] + 21 - 10, 255);
            ThText(248 + w + 1, sBallY[t] + 19, "WON", TF_NARROW, G_WHITE, G_NO_GRAD, 0x201400, 1, FALSE, TA_CENTER);
        }
    }
    // the class under the cursor
    ThText(ux, SX_S2_CLASS_Y0 + 2, sClassNames[U.tier], TF_SMALL, 0xffffff, 0xdfe4f0, THF_OUTLINE_INK, 2, FALSE, TA_LEFT);
    if (U.cup == STADIUM_CUP_CHALLENGE)
    {
        static const u8 sLevels[4] = {30, 45, 60, 75};
        snprintf(line, sizeof(line), "Every POKéMON Lv %d", sLevels[U.tier]);
        ThText(ux, SX_S2_CLASS_Y0 + 28, line, TF_NARROW, 0xf4f6fa, G_NO_GRAD, THF_OUTLINE_INK, 2, FALSE, TA_LEFT);
    }
    else
    {
        u8 n = StadiumCup_TrainerCount(U.round, U.cup, U.tier);
        snprintf(line, sizeof(line), "%d TRAINERS", n);
        ThText(ux, SX_S2_CLASS_Y0 + 28, line, TF_NARROW, 0xf4f6fa, G_NO_GRAD, THF_OUTLINE_INK, 2, FALSE, TA_LEFT);
        if (n >= 8)
            ThPut(TH_S2_FACE_GOLD);
        for (i = 0; i < 8 && i < n; i++)
        {
            const struct StadiumTrainer *tr = Stadium_Trainer(U.round, StadiumCup_Info(U.cup)->division[U.tier], i);
            int x = SX_S2_FACES_X + (i % 4) * SX_S2_FACE_W, y = SX_S2_FACES_Y + (i / 4) * SX_S2_FACE_H;
            int w = SX_S2_FACE_W - 2, h = SX_S2_FACE_H - 4;
            if (tr && tr->pic)
                BlitPart(TrainerTex(tr->pic), 8, 0, 48, 48 * h / w, (x + 1) / 2.0f, y / 2.0f, w / 2.0f, h / 2.0f,
                         StadiumCup_Open(U.round, U.cup, U.tier) ? 255 : 80);
        }
    }
    // the best won here
    {
        u16 c = StadiumCup_Save()->cleared[U.round][U.cup][U.tier];
        u8 best = 0, k;
        const int by = SX_S2_FACES_Y + SX_S2_FACE_H * 2 + 6;
        for (k = 0; k < 16; k++)
            if ((c & (1 << k)) && __builtin_popcount(k) > best)
                best = __builtin_popcount(k);
        ThText(ux, by, "BEST", TF_NARROW, 0xffffff, G_NO_GRAD, THF_OUTLINE_INK, 2, FALSE, TA_LEFT);
        if (!c)
            ThText(ux, by + 30, "Not won yet.", TF_NARROW, 0xf4f6fa, G_NO_GRAD, THF_OUTLINE_INK, 2, FALSE, TA_LEFT);
        else
        {
            for (k = 0; k < 4; k++)
                ThPutCentred(k < best ? (best == 4 ? TH_S2_BEST_GOLD : TH_S2_BEST_LIT) : TH_S2_BEST_EMPTY,
                             SX_S2_CLASS_X0 + 14 + k * 20, by + 34, 255);
            snprintf(line, sizeof(line), "%d of 4", best);
            ThText(ux, by + 50, line, TF_NARROW, 0xf4f6fa, G_NO_GRAD, THF_OUTLINE_INK, 2, FALSE, TA_LEFT);
            ThText(ux, by + 70, "restrictions", TF_NARROW, 0xf4f6fa, G_NO_GRAD, THF_OUTLINE_INK, 2, FALSE, TA_LEFT);
        }
    }
    S2Foot("{A} CHOOSE  {B} BACK");
}

// The cup's menu: its picture, the restrictions' switches, the help in glass, the team, REGISTER / SAVED TEAMS / START!
static void S2Options(void)
{
    static const u16 sGold[4] = {TH_S2_OROW_GOLD0, TH_S2_OROW_GOLD1, TH_S2_OROW_GOLD2, TH_S2_OROW_GOLD3};
    static const u16 sRing[4] = {TH_S2_OROWRING0, TH_S2_OROWRING1, TH_S2_OROWRING2, TH_S2_OROWRING3};
    static const u16 sBtn[3] = {TH_S2_OBTNRING0, TH_S2_OBTNRING1, TH_S2_OBTNRING2};
    static const char *const sNames[4] = {"LEVEL CAP", "SPECIES", "HELD ITEMS", "RENTALS ONLY"};
    char line[96];
    u8 i, rows = U.cup == STADIUM_CUP_CHALLENGE ? 0 : 4, hard = HardCount();
    const struct StadiumTeam *t = &StadiumCup_Save()->team[U.cup];
    const float ox = SX_S2_OPTS_X0 + 8;
    ThPut(TH_S2_OPTIONS);
    snprintf(line, sizeof(line), "%s%s%s", StadiumCup_Info(U.cup)->name, StadiumCup_Info(U.cup)->tiers > 1 ? "  " : "",
             StadiumCup_Info(U.cup)->tiers > 1 ? sClassNames[U.tier] : "");
    S2GlassText(SX_S2_OHEAD_X0, SX_S2_OHEAD_Y0, SX_S2_OHEAD_X1, SX_S2_OHEAD_Y1, line);
    S2GlassText(SX_S2_ROUND_X0, SX_S2_ROUND_Y0, SX_S2_ROUND_X1, SX_S2_ROUND_Y1, U.round ? "ROUND 2" : "ROUND 1");
    // the cup's picture and its class's banner
    {
        SDL_Texture *pic = CupPic(U.cup);
        const int px = SX_S2_OPIC_X0, py = SX_S2_OPIC_Y0, pw = SX_S2_OPIC_X1 - SX_S2_OPIC_X0, ph = SX_S2_OPIC_Y1 - SX_S2_OPIC_Y0;
        if (pic)
            ThBlit(pic, px, py, pw, ph, 255);
        else
            CupScene(U.cup, px / 2, py / 2, pw / 2, ph / 2, TRUE);
        if (StadiumCup_Info(U.cup)->tiers > 1)
            ThBlit(U.pics[PIC_BALL(U.tier)], px + (pw - 170) / 2.0f, py + ph - 54, 170, 48, 255);
    }
    // the restrictions: a star each in place, the rows with their switches, a sphere where it counts as hard
    ThText(ox, SX_S2_OPTS_Y0 - 1, "RESTRICTIONS", TF_SMALL, 0xffffff, 0xdfe4f0, THF_OUTLINE_INK, 2, FALSE, TA_LEFT);
    for (i = 0; i < 4; i++)
        ThPutCentred(i < hard ? TH_S2_STAR_LIT : TH_S2_STAR_EMPTY, SX_S2_OPTS_X1 - 62 + i * 16, SX_S2_OPTS_Y0 + 11, 255);
    if (rows)
        for (i = 0; i < 4; i++)
        {
            float y = SY_S2_OROW0 + i * SY_S2_OROW_H, sx = SX_S2_OPTS_X1 - 92;
            bool8 on = (U.opt & (1 << i)) != 0, held = i == 0 && (U.opt & STADIUM_OPT_RENTALS);
            bool8 isHard = i == 1 ? on : i == 3 ? on : !on;
            if (U.optCursor == i)
                ThPut(sGold[i]);
            ThText(ox, y - 1, sNames[i], G_Measure(sNames[i], TF_SMALL) * 2 <= sx - ox - 6 ? TF_SMALL : TF_NARROW,
                   held ? 0xb0b4c0 : 0xffffff, held ? G_NO_GRAD : 0xdfe4f0, THF_OUTLINE_INK, 2, FALSE, TA_LEFT);
            ThPutAt(on ? TH_S2_SWITCH_ON : TH_S2_SWITCH_OFF, sx - 10, y + 2 - 11, held ? 110 : 255);
            ThText(sx + (on ? 26 : 42), y - 2, on ? "ON" : "OFF", TF_NARROW, held ? 0xc0c0c0 : G_WHITE, G_NO_GRAD, 0x181818, 1,
                   FALSE, TA_CENTER);
            if (isHard)
                ThPutCentred(TH_S2_MARK_GOLD, SX_S2_OPTS_X1 - 10, y + 12, 255);
            if (U.optCursor == i)
                ThPut(sRing[i]);
        }
    else
        ThWrap(ox, SY_S2_OROW0, SX_S2_OPTS_X1 - ox - 6, 22, "The CHALLENGE CUP lends you six random POKéMON for every "
               "challenge. Pick three before each battle.", TF_SMALL, 0xffffff, 0xdfe4f0, THF_OUTLINE_INK, 2, FALSE, TA_LEFT);
    // the help in glass
    if (rows)
    {
        u8 k = U.optCursor < 4 ? U.optCursor : 3;
        bool8 on = (U.opt & (1 << k)) != 0;
        const char *help = hard == 4 ? "Every restriction in place: GREAT REWARDS!"
                         : k == 0 && (U.opt & STADIUM_OPT_RENTALS) ? "Off: rentals are already the cup's level." : sOptHelp[k][on ? 0 : 1];
        float cx = (SX_S2_OHELP_X0 + SX_S2_OHELP_X1) / 2.0f, w = SX_S2_OHELP_X1 - SX_S2_OHELP_X0 - 28;
        int n = G_Measure(help, TF_SMALL) * 2 <= w ? 1 : 2;
        ThWrap(cx, (SX_S2_OHELP_Y0 + SX_S2_OHELP_Y1) / 2.0f - 13 - (n - 1) * 10, w, 20, help, TF_SMALL,
               hard == 4 ? 0xfff0a0 : 0xffffff, 0xd0d4de, 0x000000, 1, TRUE, TA_CENTER);
    }
    // the team (the lent six's sockets in the CHALLENGE CUP)
    for (i = 0; i < STADIUM_TEAM_SIZE; i++)
    {
        float x = SX_S2_OTEAM_X0 + 2 + i * 30;
        if (i < t->count && rows)
            ThBlit(IconTex(GetBoxMonData((struct BoxPokemon *)&t->slot[i].mon, MON_DATA_SPECIES)), x - 16, SX_S2_OTEAM_Y0 - 20,
                   64, 64, 255);
        else
            ThPutCentred(TH_S2_SOCKET_GREEN, x + 16, (SX_S2_OTEAM_Y0 + SX_S2_OTEAM_Y1) / 2.0f, 255);
    }
    // the buttons
    for (i = 0; i < 3; i++)
    {
        int x0 = SX_S2_OBTN_X + i * SX_S2_OBTN_STEP;
        bool8 dim = i < 2 && !rows;
        if (i == 0)
            snprintf(line, sizeof(line), "REGISTER %d / 6", t->count);
        ThText(x0 + SX_S2_OBTN_W / 2.0f, (SY_S2_OBTN_Y0 + SY_S2_OBTN_Y1) / 2.0f - 13, i == 0 && rows ? line : i == 0 ? "REGISTER"
               : i == 1 ? "SAVED TEAMS" : "START!", TF_SMALL, dim ? 0x707480 : 0xffffff, dim ? G_NO_GRAD : 0xd0d4de, 0x000000, 1,
               TRUE, TA_CENTER);
        if (U.optCursor == rows + i)
            ThPut(sBtn[i]);
    }
    S2Foot("{A} CHANGE / CHOOSE  {B} BACK");
}

// The GYM LEADER CASTLE: the stages as rows (a leader's face, the gym, its leader, the badge), the one under the cursor
// on the green plate with its trainers and the badges won
static void S2Castle(void)
{
    static const s16 sRowY[9][2] = {{SY_S2_CROW0_Y0, SY_S2_CROW0_Y1}, {SY_S2_CROW1_Y0, SY_S2_CROW1_Y1}, {SY_S2_CROW2_Y0, SY_S2_CROW2_Y1},
                                    {SY_S2_CROW3_Y0, SY_S2_CROW3_Y1}, {SY_S2_CROW4_Y0, SY_S2_CROW4_Y1}, {SY_S2_CROW5_Y0, SY_S2_CROW5_Y1},
                                    {SY_S2_CROW6_Y0, SY_S2_CROW6_Y1}, {SY_S2_CROW7_Y0, SY_S2_CROW7_Y1}, {SY_S2_CROW8_Y0, SY_S2_CROW8_Y1}};
    static const u16 sGold[9] = {TH_S2_CROW_GOLD0, TH_S2_CROW_GOLD1, TH_S2_CROW_GOLD2, TH_S2_CROW_GOLD3, TH_S2_CROW_GOLD4,
                                 TH_S2_CROW_GOLD5, TH_S2_CROW_GOLD6, TH_S2_CROW_GOLD7, TH_S2_CROW_GOLD8};
    static const u16 sGrey[9] = {TH_S2_CROW_GREY0, TH_S2_CROW_GREY1, TH_S2_CROW_GREY2, TH_S2_CROW_GREY3, TH_S2_CROW_GREY4,
                                 TH_S2_CROW_GREY5, TH_S2_CROW_GREY6, TH_S2_CROW_GREY7, TH_S2_CROW_GREY8};
    static const u16 sRing[9] = {TH_S2_CROWRING0, TH_S2_CROWRING1, TH_S2_CROWRING2, TH_S2_CROWRING3, TH_S2_CROWRING4,
                                 TH_S2_CROWRING5, TH_S2_CROWRING6, TH_S2_CROWRING7, TH_S2_CROWRING8};
    static const u16 sBadge[8] = {TH_S2_BADGE0, TH_S2_BADGE1, TH_S2_BADGE2, TH_S2_BADGE3, TH_S2_BADGE4, TH_S2_BADGE5,
                                  TH_S2_BADGE6, TH_S2_BADGE7};
    u8 i, badges = StadiumCastle_Badges(U.round), n;
    char line[64];
    const float gx = SX_S2_CSIDE_X0 + 6;
    ThPut(TH_S2_CASTLE);
    S2GlassText(6, 6, 300, 38, StadiumCup_Info(STADIUM_CUP_CASTLE)->name);
    S2GlassText(SX_S2_ROUND_X0, SX_S2_ROUND_Y0, SX_S2_ROUND_X1, SX_S2_ROUND_Y1, U.round ? "ROUND 2" : "ROUND 1");
    for (i = 0; i < STADIUM_CASTLE_STAGES; i++)
    {
        int y0 = sRowY[i][0], y1 = sRowY[i][1];
        float ty = (y0 + y1) / 2.0f - 13;
        bool8 open = StadiumCastle_StageOpen(U.round, i);
        const struct StadiumTrainer *face = CastleFace(i);
        const char *name = StadiumCastle_StageName(i);
        unsigned ink = open ? 0xffffff : 0xc4c8d2, grad = open ? 0xdfe4f0 : G_NO_GRAD;
        if (U.stage == i)
            ThPut(sGold[i]);
        else if (!open)
            ThPut(sGrey[i]);
        if (face && face->pic)
            BlitPart(TrainerTex(face->pic), 14, 4, 36, 26, (SX_S2_CASTLE_X0 + 2) / 2.0f, (y0 + 1) / 2.0f, 17, (y1 - y0 - 2) / 2.0f,
                     open ? 255 : 90);
        ThText(SX_S2_CASTLE_X0 + 40, ty, name, G_Measure(name, TF_SMALL) * 2 <= 124 ? TF_SMALL : TF_NARROW, ink, grad,
               THF_OUTLINE_INK, 2, FALSE, TA_LEFT);
        if (i < STADIUM_CASTLE_GYMS)
        {
            char who[16] = "";
            if (face)
                Ascii(who, face->name, sizeof(who));
            ThText(SX_S2_CASTLE_X0 + 170, ty, who, G_Measure(who, TF_SMALL) * 2 <= 92 ? TF_SMALL : TF_NARROW, 0xe8ecf8, G_NO_GRAD,
                   THF_OUTLINE_INK, 2, FALSE, TA_LEFT);
            ThPutCentred((badges >> i) & 1 ? sBadge[i] : TH_S2_BEST_EMPTY, SX_S2_CASTLE_X1 - 14, (y0 + y1) / 2.0f, 255);
        }
        else if (!open)
            ThText(SX_S2_CASTLE_X0 + 170, ty, "LOCKED", TF_SMALL, 0xc4c8d2, G_NO_GRAD, THF_OUTLINE_INK, 2, FALSE, TA_LEFT);
        else if (StadiumCup_Cleared(U.round, STADIUM_CUP_CASTLE, 0))
        {
            int w = G_Measure("WON", TF_NARROW) + 9;
            ThPutAt(TH_S2_WON, SX_S2_CASTLE_X1 - 20 - w * 2 - 10, (y0 + y1) / 2.0f - 10 - 10, 255);
            ThText(SX_S2_CASTLE_X1 - 20 - w, (y0 + y1) / 2.0f - 12, "WON", TF_NARROW, G_WHITE, G_NO_GRAD, 0x201400, 1, FALSE, TA_CENTER);
        }
    }
    if (U.stage < STADIUM_CASTLE_STAGES)
        ThPut(sRing[U.stage]);
    // the stage under the cursor
    ThText(gx, SX_S2_CSIDE_Y0 + 2, StadiumCastle_StageName(U.stage), TF_SMALL, 0xffffff, 0xdfe4f0, 0x1c2c20, 2, FALSE, TA_LEFT);
    n = U.stage < STADIUM_CASTLE_GYMS ? 4 : 5;
    ThText(gx, SX_S2_CSIDE_Y0 + 26, n == 4 ? "4 BATTLES" : "5 BATTLES", TF_NARROW, 0xf4f6fa, G_NO_GRAD, 0x1c2c20, 2, FALSE, TA_LEFT);
    ThPut(n == 4 ? TH_S2_CCELL_GOLD4 : TH_S2_CCELL_GOLD5);
    for (i = 0; i < n; i++)
    {
        const struct StadiumTrainer *tr = U.stage < STADIUM_CASTLE_GYMS ? Stadium_Trainer(U.round, STADIUM_GYM_1 + U.stage, i)
                                        : i < 4 ? Stadium_Trainer(U.round, STADIUM_ELITE_FOUR, i)
                                                : Stadium_Trainer(U.round, STADIUM_RIVAL, 0);
        int w = (SX_S2_CSIDE_X1 - SX_S2_CSIDE_X0) / n, x = SX_S2_CSIDE_X0 + i * w;
        if (tr && tr->pic)
            BlitPart(TrainerTex(tr->pic), 32 - 56 * w / SY_S2_CCELL_H / 2, 0, 56 * w / SY_S2_CCELL_H, 56, x / 2.0f, SY_S2_CCELLS / 2.0f,
                     w / 2.0f, SY_S2_CCELL_H / 2.0f, StadiumCastle_StageOpen(U.round, U.stage) ? 255 : 90);
        if (i == n - 1 && U.stage == STADIUM_CASTLE_E4)   // (the RIVAL's team is picked against the player's six)
            ThText(x + w / 2.0f, SY_S2_CCELLS + 8, "?", TF_NORMAL, 0xffffff, 0xdfe4f0, 0x1c2c20, 2, FALSE, TA_CENTER);
    }
    // the badges won
    snprintf(line, sizeof(line), "BADGES %d / 8", StadiumCastle_BadgeCount(U.round));
    ThText(gx, SY_S2_CBADGES, line, TF_SMALL, 0xffffff, 0xdfe4f0, 0x1c2c20, 2, FALSE, TA_LEFT);
    for (i = 0; i < STADIUM_CASTLE_GYMS; i++)
        ThPutCentred((badges >> i) & 1 ? sBadge[i] : TH_S2_SOCKET_GREEN, SX_S2_CSIDE_X0 + 20 + (i % 4) * 40,
                     SY_S2_CBADGES + 44 + (i / 4) * 26, 255);
    if (U.stage < STADIUM_CASTLE_GYMS)
        snprintf(line, sizeof(line), "%s", (badges >> U.stage) & 1 ? "Won. Try it again?" : StadiumCastle_BadgeName(U.stage));
    else
        snprintf(line, sizeof(line), "%s", StadiumCastle_StageOpen(U.round, U.stage) ? "Then, the castle!" : "Needs 8 badges.");
    ThWrap(gx, SY_S2_CBADGES + 88, SX_S2_CSIDE_X1 - gx - 4, 22, line, TF_SMALL, 0xffffff, 0xdfe4f0, 0x1c2c20, 2, FALSE, TA_LEFT);
    S2Foot("{A} CHOOSE  {B} BACK");
}

// A menu in the blue panel (ROWS of words, the pointer at the CURSOR): its box at (X, Y), W wide; RED: a row in red
static void S2Menu(float x, float y, float w, const char *const *rows, u8 n, u8 cursor, int red)
{
    u8 i;
    ThNine(TH_S2_PANEL, x, y, w, 16 + n * 26, 12, 4, 255);
    for (i = 0; i < n; i++)
    {
        float ry = y + 8 + i * 26;
        if (i == cursor)
            ThPutAt(TH_S2_POINTER, x + 10 - 10, ry + 13 - 20, 255);
        ThText(x + 28, ry, rows[i], TF_SMALL, i == red ? 0xff9090 : 0xffffff, i == red ? G_NO_GRAD : 0xdfe4f0, 0x0c0c40, 2, FALSE,
               TA_LEFT);
    }
}

// A question in the blue panel: two lines, centred on (CX, Y)
static void S2Ask(float cx, float y, float w, const char *a, const char *b)
{
    ThNine(TH_S2_PANEL, cx - w / 2, y, w, 72, 12, 4, 255);
    ThText(cx, y + 10, a, TF_SMALL, 0xffffff, 0xdfe4f0, 0x0c0c40, 2, FALSE, TA_CENTER);
    ThText(cx, y + 38, b, TF_NARROW, 0xf0f0ff, G_NO_GRAD, 0x0c0c40, 2, FALSE, TA_CENTER);
}

// The ribbon's words (the layer has the ribbon)
static void S2Ribbon(const char *left, const char *right)
{
    if (left)
        ThText(10, 8 - 7, left, TF_SMALL, 0xffffff, G_NO_GRAD, 0x000000, 2, TRUE, TA_LEFT);
    if (right)
        ThText(470, 8 - 7, right, TF_SMALL, 0xffffff, G_NO_GRAD, 0x000000, 2, TRUE, TA_RIGHT);
}

// SAVED TEAMS: the slots on the blue list (brackets on the one under the cursor), its team in the plate's windows
static void S2Teams(void)
{
    static const u16 sBr[8] = {TH_S2_TBRACKETS0, TH_S2_TBRACKETS1, TH_S2_TBRACKETS2, TH_S2_TBRACKETS3, TH_S2_TBRACKETS4,
                               TH_S2_TBRACKETS5, TH_S2_TBRACKETS6, TH_S2_TBRACKETS7};
    static u8 sTop;
    const struct StadiumSavedTeam *sel = StadiumTeams_Get(U.teamCursor);
    const float rh = (SX_S2_TLIST_Y1 - SX_S2_TLIST_Y0) / 8.0f, lx = SX_S2_TLIST_X0;
    char line[64];
    u8 i;
    if (U.teamCursor < sTop)
        sTop = U.teamCursor;
    if (U.teamCursor >= sTop + 8)
        sTop = U.teamCursor - 7;
    ThPut(TH_S2_TEAMS);
    snprintf(line, sizeof(line), "%s %d / 99", StadiumTeams_IsServer() ? "SERVER" : "THIS SAVE", StadiumTeams_Count());
    S2Ribbon("SAVED TEAMS", line);
    for (i = 0; i < 8 && sTop + i < STADIUM_SAVED_TEAMS; i++)
    {
        u8 n = sTop + i;
        const struct StadiumSavedTeam *t = StadiumTeams_Get(n);
        float ty = SX_S2_TLIST_Y0 + i * rh + rh / 2 - 13;
        snprintf(line, sizeof(line), "%02d", n + 1);
        ThText(lx + 6, ty, line, TF_SMALL, 0xc8d0ff, G_NO_GRAD, 0x0c0c40, 2, FALSE, TA_LEFT);
        if (t)
        {
            char owner[16];
            Ascii(owner, t->owner, sizeof(owner));
            ThText(lx + 38, ty, owner, TF_SMALL, 0xffffff, G_NO_GRAD, 0x0c0c40, 2, FALSE, TA_LEFT);
            if (t->cup < STADIUM_CUP_COUNT)
            {
                const char *cup = CupLabel(t->cup);
                ThText(SX_S2_TLIST_X1 - 6, ty, cup, G_Measure(cup, TF_SMALL) * 2 <= 100 ? TF_SMALL : TF_NARROW, 0xffe060, G_NO_GRAD,
                       0x0c0c40, 2, FALSE, TA_RIGHT);
            }
        }
        else
            ThText(lx + 38, ty, "- - - - -", TF_SMALL, 0x8890e0, G_NO_GRAD, 0x0c0c40, 2, FALSE, TA_LEFT);
        if (n == U.teamCursor)
            ThPut(sBr[i]);
    }
    // where in the 99: the thumb down the track
    ThPutAt(TH_S2_THUMB, 0, (SX_S2_TLIST_Y1 - SX_S2_TLIST_Y0 - 28) * sTop / (float)(STADIUM_SAVED_TEAMS - 8), 255);
    // the slot under the cursor
    {
        const float px = SX_S2_TPLATE_X0 + 8;
        snprintf(line, sizeof(line), "SLOT %02d", U.teamCursor + 1);
        ThText(px, SX_S2_TPLATE_Y0 + 4, line, TF_SMALL, 0xffffff, G_NO_GRAD, 0x202020, 2, TRUE, TA_LEFT);
        if (sel)
        {
            char owner[16];
            Ascii(owner, sel->owner, sizeof(owner));
            if (sel->cup < STADIUM_CUP_COUNT)
                ThText(SX_S2_TPLATE_X1 - 8, SX_S2_TPLATE_Y0 + 4, CupLabel(sel->cup), TF_SMALL, 0xffffff, G_NO_GRAD, 0x202020, 2, TRUE,
                       TA_RIGHT);
            snprintf(line, sizeof(line), "by %s", owner);
            ThText(px, SX_S2_TPLATE_Y0 + 26, line, TF_SMALL, 0xffffff, G_NO_GRAD, 0x202020, 2, TRUE, TA_LEFT);
        }
        for (i = 0; i < STADIUM_TEAM_SIZE; i++)
        {
            float wx = SX_S2_TWIN_X + (i % 3) * SX_S2_TWIN_DX, wy = SX_S2_TWIN_Y + (i / 3) * SX_S2_TWIN_DY;
            if (sel && i < sel->count)
            {
                struct BoxPokemon *b = (struct BoxPokemon *)&sel->slot[i].mon;
                ThFrontIn(FrontTex(GetBoxMonData(b, MON_DATA_SPECIES), GetBoxMonData(b, MON_DATA_PERSONALITY),
                                   GetBoxMonData(b, MON_DATA_OT_ID)), wx, wy, SX_S2_TWIN_W, SX_S2_TWIN_H, 255);
                snprintf(line, sizeof(line), "L%d", GetLevelFromBoxMonExp(b));
                ThText(wx + SX_S2_TWIN_W / 2.0f, wy + SX_S2_TWIN_H + 4, line, TF_SMALL, 0xffffff, G_NO_GRAD, 0x202020, 2, TRUE,
                       TA_CENTER);
            }
        }
        if (!sel)
            ThText(px, SX_S2_TPLATE_Y0 + 26, "Empty.", TF_SMALL, 0xffffff, G_NO_GRAD, 0x202020, 2, TRUE, TA_LEFT);
    }
    if (U.menu)
        S2Menu(300, 100, 140, sel ? sTeamMenuFull : sTeamMenuEmpty, sel ? 5 : 2, U.menuCursor, sel ? 3 : -1);
    if (U.confirm)
    {
        snprintf(line, sizeof(line), "Erase slot %02d's team?", U.teamCursor + 1);
        S2Ask(240, 120, 340, line, StadiumTeams_IsServer() ? "Everyone on the server loses it." : "It's gone for good.");
    }
    S2Foot(U.confirm ? "{A} ERASE  {B} KEEP" : U.menu ? "{A} CHOOSE  {B} BACK" : "{A} MENU  {L}{R} PAGE  {B} BACK");
}

// ---------------------------------------------------------------------------------------------------------------
// The screens in the chosen theme; FALSE: this one isn't drawn in it (the game's own then)
static bool8 ThemedDraw(void)
{
    switch (Theme())
    {
    case MP_STADIUM_STYLE_STADIUM2:
        switch (U.screen)
        {
        case SCR_SELECT: S2Select(); return TRUE;
        case SCR_CLASS: if (U.cup == STADIUM_CUP_CASTLE) S2Castle(); else S2Class(); return TRUE;
        case SCR_OPTIONS: S2Options(); return TRUE;
        case SCR_TEAMS: S2Teams(); return TRUE;
        }
        break;
    }
    return FALSE;
}
