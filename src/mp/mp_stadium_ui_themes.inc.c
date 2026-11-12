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

// Writing at real pixels (X, Y the line's top), with STADIUM's outline: OUTLINE's colour PX wide (0 none), BOLD, the
// face falling from COLOR to GRAD (G_NO_GRAD: none). ALIGN TA_LEFT / TA_CENTER / TA_RIGHT. Returns the width in pixels
enum { THF_OUTLINE_INK = 0x282c38 };
static int ThText(float x, float y, const char *s, int font, unsigned color, unsigned grad, unsigned outline, int px,
                  bool8 bold, int align)
{
    int w = G_Measure(s, font) * 2;
    float l = align == TA_CENTER ? x - w / 2.0f : align == TA_RIGHT ? x - w : x;
    int ux = (int)SDL_floorf(SDL_roundf(l) / 2), uy = (int)SDL_floorf(SDL_roundf(y) / 2);
    G_TextStyle(outline, px, bold, grad);
    G_TextNudge((SDL_roundf(l) - ux * 2) / 2, (SDL_roundf(y) - uy * 2) / 2);
    G_Text(ux, uy, color, G_NO_SHADOW, s, 0, font);
    G_TextNudge(0, 0);
    G_TextStyle(0, 0, 0, G_NO_GRAD);
    return w;
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

// A note in the hints' place (why not, what happened)
static void S2Note(void)
{
    if (!U.note[0])
        return;
    ThPut(TH_S2_STRIP);
    ThText(8, 296, U.note, TF_SMALL, 0xf8e8a0, G_NO_GRAD, 0x000000, 1, FALSE, TA_LEFT);
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
    S2Note();
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
    S2Note();
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
        case SCR_CLASS: if (U.cup == STADIUM_CUP_CASTLE) break; S2Class(); return TRUE;
        }
        break;
    }
    return FALSE;
}
