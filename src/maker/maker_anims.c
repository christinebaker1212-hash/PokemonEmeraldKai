// Moemon Maker, M5: move animations on a timeline (the editor's ANIMATIONS window, maker/maker_anims_ui.cpp). One file a
// move in MoemonMaker/anims/MOVE_X.txt (pictures beside it as PNGs), a header and one element a line, times in frames:
//
//   anim MOVE_TACKLE frames=48 replace=1      (replace=1: the Classic animation becomes a wait of that many frames;
//                                               0: drawn over it, like BATTLE ANIMS > MODERN)
//   sound START SE_NAME
//   flash START LEN RRGGBB STRENGTH           (the picture tinted toward a color, rising and falling, STRENGTH 0-100)
//   shake START LEN attacker|target AMOUNT    (the Pokemon shaken AMOUNT pixels)
//   ring START LEN WHERE RRGGBB R0 R1         (a ring growing from radius R0 to R1)
//   burst START LEN WHERE RRGGBB COUNT        (COUNT sparks flying out)
//   beam START LEN RRGGBB WIDTH               (a beam from the attacker to the target, drawn out over its first half)
//   image START LEN FILE.png FROM TO [SCALE]  (a picture moving from FROM to TO, SCALE in percent)
//
// WHERE / FROM / TO: attacker, target, center, or X,Y. Drawn by an effects layer over the finished picture (as
// mp_modernanim.c does), never bound by the GBA's sprites or palettes; the same frame count on every game of a link
// (the pack is the same for all). src/battle_anim.c asks MP_MakerAnim_Begin when a move's animation starts; sdl2.c
// draws each frame (MP_MakerAnim_Composite). The editor's preview draws the same elements on a stand-in scene
// (MakerApi_AnimPreview).
#include <dirent.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "battle.h"
#include "battle_anim.h"
#include "main.h"
#include "sound.h"
#include "maker.h"
#include "maker_api.h"
#include "constants/battle_anim.h"

unsigned char *stbi_load(const char *filename, int *x, int *y, int *comp, int req_comp);
void stbi_image_free(void *retval_from_stbi_load);
const char *Maker_ReadDir(void);
void MPNet_Log(const char *fmt, ...);
extern const u8 *const gBattleAnims_Moves[];

#define W DISPLAY_WIDTH
#define H DISPLAY_HEIGHT
#define MAX_ELEMS 48

enum { E_SOUND, E_FLASH, E_SHAKE, E_RING, E_BURST, E_BEAM, E_IMAGE };
struct Elem
{
    u8 kind;
    u16 start, len;
    u32 color;          // 0xRRGGBB
    s16 a, b;           // strength / amount / radii / count / width / scale
    char where[24], to[24];
    u16 sound;
    u8 *rgba;           // an image's pixels
    int iw, ih;
};
struct Anim
{
    u16 move;
    u16 frames;
    bool8 replace;
    struct Elem elems[MAX_ELEMS];
    int count;
};

static struct Anim *sAnims;
static int sAnimCount;
static bool8 sLoaded;
static u8 sWaitScript[2 * 32 + 1];

// the playing one
static struct { const struct Anim *anim; int frame; u8 attacker, target; s16 shakeSprite[2], shakeX[2]; } sPlay;

static int Clamp(int v, int lo, int hi) { return v < lo ? lo : v > hi ? hi : v; }

// One animation's text into A (the editor's preview parses its own text too); FALSE when it isn't one
static bool8 Parse(struct Anim *a, const char *text, const char *dir)
{
    const char *p = text;
    memset(a, 0, sizeof(*a));
    while (p && *p)
    {
        char line[200], w[8][40];
        const char *nl = strchr(p, '\n');
        int len = nl ? (int)(nl - p) : (int)strlen(p), n;
        unsigned v;
        if (len >= (int)sizeof(line))
            len = sizeof(line) - 1;
        memcpy(line, p, len);
        line[len] = 0;
        p = nl ? nl + 1 : NULL;
        memset(w, 0, sizeof(w));
        n = sscanf(line, "%39s %39s %39s %39s %39s %39s %39s %39s", w[0], w[1], w[2], w[3], w[4], w[5], w[6], w[7]);
        if (n <= 0 || w[0][0] == '#')
            continue;
        if (!strcmp(w[0], "anim") && n >= 2)
        {
            int i;
            if (!MakerApi_ValueOf(MAKER_NAMES_MOVE, w[1], &v))
                return FALSE;
            a->move = v;
            a->frames = 60;
            for (i = 2; i < n; i++)
            {
                if (!strncmp(w[i], "frames=", 7))
                    a->frames = Clamp(atoi(w[i] + 7), 1, 600);
                else if (!strncmp(w[i], "replace=", 8))
                    a->replace = atoi(w[i] + 8) != 0;
            }
            continue;
        }
        if (a->count >= MAX_ELEMS || n < 3)
            continue;
        {
            struct Elem *e = &a->elems[a->count];
            memset(e, 0, sizeof(*e));
            e->start = Clamp(atoi(w[1]), 0, 600);
            if (!strcmp(w[0], "sound"))
            {
                e->kind = E_SOUND;
                if (!MakerApi_ValueOf(MAKER_NAMES_SONG, w[2], &v))
                    continue;
                e->sound = v;
            }
            else
            {
                e->len = Clamp(atoi(w[2]), 1, 600);
                if (!strcmp(w[0], "flash") && n >= 5)
                    e->kind = E_FLASH, e->color = strtoul(w[3], NULL, 16), e->a = Clamp(atoi(w[4]), 0, 100);
                else if (!strcmp(w[0], "shake") && n >= 5)
                    e->kind = E_SHAKE, snprintf(e->where, sizeof(e->where), "%s", w[3]), e->a = Clamp(atoi(w[4]), 0, 8);
                else if (!strcmp(w[0], "ring") && n >= 7)
                    e->kind = E_RING, snprintf(e->where, sizeof(e->where), "%s", w[3]), e->color = strtoul(w[4], NULL, 16),
                    e->a = Clamp(atoi(w[5]), 0, 200), e->b = Clamp(atoi(w[6]), 0, 200);
                else if (!strcmp(w[0], "burst") && n >= 6)
                    e->kind = E_BURST, snprintf(e->where, sizeof(e->where), "%s", w[3]), e->color = strtoul(w[4], NULL, 16),
                    e->a = Clamp(atoi(w[5]), 1, 64);
                else if (!strcmp(w[0], "beam") && n >= 5)
                    e->kind = E_BEAM, e->color = strtoul(w[3], NULL, 16), e->a = Clamp(atoi(w[4]), 1, 24);
                else if (!strcmp(w[0], "image") && n >= 6)
                {
                    char path[1300];
                    e->kind = E_IMAGE;
                    snprintf(e->where, sizeof(e->where), "%s", w[4]);
                    snprintf(e->to, sizeof(e->to), "%s", w[5]);
                    e->a = n >= 7 ? Clamp(atoi(w[6]), 10, 800) : 100;
                    snprintf(path, sizeof(path), "%s/%s", dir, w[3]);
                    if (strchr(w[3], '/') || strchr(w[3], '\\') || strstr(w[3], ".."))
                        continue;
                    e->rgba = stbi_load(path, &e->iw, &e->ih, &len, 4);
                    if (!e->rgba)
                        continue;
                }
                else
                    continue;
            }
            a->count++;
        }
    }
    return a->move != 0;
}

static void LoadAll(void)
{
    char dir[1100], path[1400];
    DIR *d;
    struct dirent *de;
    sLoaded = TRUE;
    snprintf(dir, sizeof(dir), "%s/anims", Maker_ReadDir());
    if ((d = opendir(dir)) == NULL)
        return;
    while ((de = readdir(d)) != NULL)
    {
        size_t n = strlen(de->d_name);
        FILE *f;
        long size;
        char *text;
        if (n < 5 || strcmp(de->d_name + n - 4, ".txt") != 0)
            continue;
        snprintf(path, sizeof(path), "%s/%s", dir, de->d_name);
        if ((f = fopen(path, "rb")) == NULL)
            continue;
        fseek(f, 0, SEEK_END);
        size = ftell(f);
        fseek(f, 0, SEEK_SET);
        text = calloc(1, size + 1);
        if (fread(text, 1, size, f) != (size_t)size)
            size = 0;
        fclose(f);
        sAnims = realloc(sAnims, (sAnimCount + 1) * sizeof(*sAnims));
        if (Parse(&sAnims[sAnimCount], text, dir))
            sAnimCount++;
        else
            printf("[maker] anims: %s isn't an animation (left out)\n", de->d_name);
        free(text);
    }
    closedir(d);
    if (sAnimCount)
        printf("[maker] %d move animations\n", sAnimCount);
}

void MakerApi_AnimReload(void)
{
    int i, k;
    for (i = 0; i < sAnimCount; i++)
        for (k = 0; k < sAnims[i].count; k++)
            if (sAnims[i].elems[k].rgba)
                stbi_image_free(sAnims[i].elems[k].rgba);
    free(sAnims);
    sAnims = NULL;
    sAnimCount = 0;
    sLoaded = FALSE;
}

// battle_anim.c, as an animation starts: a move with a Maker animation plays it; with replace=1 the script to run
// instead of the Classic one (a wait of the animation's frames), else NULL
const u8 *MP_MakerAnim_Begin(const u8 *const table[], u16 id)
{
    int i, k, n = 0;
    sPlay.anim = NULL;
    if (table != gBattleAnims_Moves || !gMain.inBattle)
        return NULL;
    if (!sLoaded)
        LoadAll();
    for (i = 0; i < sAnimCount && sAnims[i].move != id; i++)
        ;
    if (i == sAnimCount)
        return NULL;
    sPlay.anim = &sAnims[i];
    sPlay.frame = 0;
    sPlay.attacker = gBattleAnimAttacker;
    sPlay.target = gBattleAnimTarget;
    sPlay.shakeSprite[0] = sPlay.shakeSprite[1] = -1;
    MPNet_Log("maker anim %s (%d frames%s)", MakerApi_NameOf(MAKER_NAMES_MOVE, id), sAnims[i].frames, sAnims[i].replace ? ", in place of the Classic one" : "");
    if (!sAnims[i].replace)
        return NULL;
    for (k = sAnims[i].frames; k > 0 && n < 32; k -= 255)
    {
        sWaitScript[2 * n] = 0x04;   // delay (battle_anim.c sScriptCmdTable)
        sWaitScript[2 * n + 1] = k > 255 ? 255 : k;
        n++;
    }
    sWaitScript[2 * n] = 0x08;       // end
    return sWaitScript;
}

// ---- drawing (onto a BGR555 frame in the game, onto RGBA in the editor's preview)

struct Canvas { u16 *f555; u32 *rgba; int ax, ay, tx, ty; };

static void Blend(struct Canvas *c, int x, int y, u32 rgb, int alpha)   // alpha 0-256
{
    int r = (rgb >> 16) & 255, g = (rgb >> 8) & 255, b = rgb & 255;
    if (x < 0 || y < 0 || x >= W || y >= H || alpha <= 0)
        return;
    if (alpha > 256)
        alpha = 256;
    if (c->f555)
    {
        u16 *px = &c->f555[y * W + x];
        int pr = (*px & 31) << 3, pg = ((*px >> 5) & 31) << 3, pb = ((*px >> 10) & 31) << 3;
        pr += (r - pr) * alpha / 256, pg += (g - pg) * alpha / 256, pb += (b - pb) * alpha / 256;
        *px = (pr >> 3) | ((pg >> 3) << 5) | ((pb >> 3) << 10);
    }
    else
    {
        u32 *px = &c->rgba[y * W + x];
        int pr = *px & 255, pg = (*px >> 8) & 255, pb = (*px >> 16) & 255;
        pr += (r - pr) * alpha / 256, pg += (g - pg) * alpha / 256, pb += (b - pb) * alpha / 256;
        *px = 0xFF000000u | (pb << 16) | (pg << 8) | pr;
    }
}

static void Where(struct Canvas *c, const char *w, int *x, int *y)
{
    if (!strcmp(w, "attacker"))
        *x = c->ax, *y = c->ay;
    else if (!strcmp(w, "target"))
        *x = c->tx, *y = c->ty;
    else if (!strcmp(w, "center"))
        *x = W / 2, *y = H / 2;
    else if (sscanf(w, "%d,%d", x, y) != 2)
        *x = W / 2, *y = H / 2;
}

static void Disc(struct Canvas *c, int cx, int cy, int r, u32 rgb, int alpha)
{
    int x, y;
    for (y = -r; y <= r; y++)
        for (x = -r; x <= r; x++)
            if (x * x + y * y <= r * r)
                Blend(c, cx + x, cy + y, rgb, alpha);
}

static void Draw(struct Canvas *c, const struct Anim *a, int frame)
{
    int i;
    for (i = 0; i < a->count; i++)
    {
        const struct Elem *e = &a->elems[i];
        int t = frame - e->start, x, y, x2, y2, k;
        float u;
        if (e->kind == E_SOUND || e->kind == E_SHAKE || t < 0 || t >= e->len)
            continue;
        u = e->len > 1 ? (float)t / (e->len - 1) : 1.0f;
        switch (e->kind)
        {
        case E_FLASH:
        {
            int alpha = (int)(e->a * 2.56f * (u < 0.5f ? u * 2 : (1 - u) * 2));
            for (y = 0; y < H; y++)
                for (x = 0; x < W; x++)
                    Blend(c, x, y, e->color, alpha);
            break;
        }
        case E_RING:
        {
            int r = e->a + (int)((e->b - e->a) * u), alpha = (int)(256 * (1 - u * 0.8f));
            Where(c, e->where, &x, &y);
            for (k = 0; k < 360; k += 2)
            {
                float rad = k * 3.14159f / 180;
                Blend(c, x + (int)(r * cosf(rad)), y + (int)(r * sinf(rad)), e->color, alpha);
                Blend(c, x + (int)((r + 1) * cosf(rad)), y + (int)((r + 1) * sinf(rad)), e->color, alpha / 2);
            }
            break;
        }
        case E_BURST:
            Where(c, e->where, &x, &y);
            for (k = 0; k < e->a; k++)
            {
                float ang = (k * 137.5f + e->start * 7) * 3.14159f / 180, dist = 4 + u * (30 + (k * 13) % 20);
                Disc(c, x + (int)(dist * cosf(ang)), y + (int)(dist * sinf(ang)), u < 0.7f ? 2 : 1, e->color,
                     (int)(256 * (1 - u)));
            }
            break;
        case E_BEAM:
        {
            float reach = u < 0.5f ? u * 2 : 1.0f;
            int steps, alpha = (int)(256 * (u < 0.5f ? 1.0f : (1 - u) * 2));
            x = c->ax, y = c->ay, x2 = c->tx, y2 = c->ty;
            steps = abs(x2 - x) > abs(y2 - y) ? abs(x2 - x) : abs(y2 - y);
            for (k = 0; k <= steps * reach; k++)
            {
                int px = x + (x2 - x) * k / (steps ? steps : 1), py = y + (y2 - y) * k / (steps ? steps : 1);
                Disc(c, px, py, e->a / 2, e->color, alpha * 3 / 4);
                Disc(c, px, py, e->a / 4 > 0 ? e->a / 4 : 1, 0xFFFFFF, alpha / 2);
            }
            break;
        }
        case E_IMAGE:
        {
            int sw, sh;
            Where(c, e->where, &x, &y);
            Where(c, e->to, &x2, &y2);
            x += (int)((x2 - x) * u), y += (int)((y2 - y) * u);
            sw = e->iw * e->a / 100, sh = e->ih * e->a / 100;
            for (k = 0; k < sw * sh; k++)
            {
                int dx = k % sw, dy = k / sw, sx = dx * 100 / e->a, sy = dy * 100 / e->a;
                const u8 *p = &e->rgba[(sy * e->iw + sx) * 4];
                if (p[3])
                    Blend(c, x - sw / 2 + dx, y - sh / 2 + dy, (p[0] << 16) | (p[1] << 8) | p[2], p[3] + 1);
            }
            break;
        }
        }
    }
}

// The shake: the shaken Pokemon's sprite moved for the element's frames (and put back)
static void Shake(const struct Anim *a, int frame, u8 attacker, u8 target)
{
    int i, s;
    for (s = 0; s < 2; s++)
        if (sPlay.shakeSprite[s] >= 0)
            gSprites[sPlay.shakeSprite[s]].x2 -= sPlay.shakeX[s], sPlay.shakeSprite[s] = -1;
    for (i = 0; i < a->count; i++)
    {
        const struct Elem *e = &a->elems[i];
        int t = frame - e->start;
        u8 who, sprite;
        if (e->kind != E_SHAKE || t < 0 || t >= e->len)
            continue;
        who = !strcmp(e->where, "attacker") ? attacker : target;
        sprite = gBattlerSpriteIds[who];
        s = !strcmp(e->where, "attacker") ? 0 : 1;
        if (sprite >= MAX_SPRITES || sPlay.shakeSprite[s] >= 0)
            continue;
        sPlay.shakeX[s] = (t & 2) ? e->a : -e->a;
        gSprites[sprite].x2 += sPlay.shakeX[s];
        sPlay.shakeSprite[s] = sprite;
    }
}

// sdl2.c, each frame: the playing animation drawn over the picture, its sounds played, its shakes done
void MP_MakerAnim_Composite(u16 *frame)
{
    const struct Anim *a = sPlay.anim;
    struct Canvas c;
    int i;
    if (!a)
        return;
    if (!gMain.inBattle || sPlay.frame >= a->frames || (!a->replace && !gAnimScriptActive))
    {
        Shake(a, a->frames + 1, sPlay.attacker, sPlay.target);   // (puts a shaken Pokemon back)
        MPNet_Log("maker anim done");
        sPlay.anim = NULL;
        return;
    }
    for (i = 0; i < a->count; i++)
        if (a->elems[i].kind == E_SOUND && a->elems[i].start == sPlay.frame)
            PlaySE(a->elems[i].sound);
    Shake(a, sPlay.frame, sPlay.attacker, sPlay.target);
    memset(&c, 0, sizeof(c));
    c.f555 = frame;
    c.ax = GetBattlerSpriteCoord(sPlay.attacker, BATTLER_COORD_X);
    c.ay = GetBattlerSpriteCoord(sPlay.attacker, BATTLER_COORD_Y_PIC_OFFSET);
    c.tx = GetBattlerSpriteCoord(sPlay.target, BATTLER_COORD_X);
    c.ty = GetBattlerSpriteCoord(sPlay.target, BATTLER_COORD_Y_PIC_OFFSET);
    Draw(&c, a, sPlay.frame);
    sPlay.frame++;
}

// The editor's preview: frame FRAME of the animation TEXT on a stand-in scene (the player's Pokemon low left, the foe's
// high right); 240x160 RGBA. Returns the animation's frames (0 when the text isn't one) and its problem in WHY
int MakerApi_AnimPreview(const char *text, int frame, unsigned int *rgba)
{
    static struct Anim a;
    static char *sLast;
    static bool8 sOk;
    struct Canvas c;
    char dir[1100];
    int x, y, k, shakeA = 0, shakeT = 0;
    if (!sLast || strcmp(sLast, text) != 0)   // (parsed again only when the text changed: pictures are files)
    {
        for (k = 0; k < a.count; k++)
            if (a.elems[k].rgba)
                stbi_image_free(a.elems[k].rgba);
        snprintf(dir, sizeof(dir), "%s/anims", MakerApi_EditsDir());
        sOk = Parse(&a, text, dir);
        free(sLast);
        sLast = strdup(text);
    }
    if (!sOk)
        return 0;
    for (k = 0; k < a.count; k++)
    {
        const struct Elem *e = &a.elems[k];
        int t = frame - e->start;
        if (e->kind == E_SHAKE && t >= 0 && t < e->len)
            *(!strcmp(e->where, "attacker") ? &shakeA : &shakeT) = (t & 2) ? e->a : -e->a;
    }
    for (y = 0; y < H; y++)   // a sky, a field, and the two Pokemon as shadows on their platforms
        for (x = 0; x < W; x++)
        {
            u32 col = y < 70 ? 0xFFF0D8B0u - ((y / 8) << 8) : 0xFF78C090u;
            rgba[y * W + x] = col;
        }
    memset(&c, 0, sizeof(c));
    c.rgba = rgba;
    c.ax = 72, c.ay = 88, c.tx = 176, c.ty = 48;
    for (y = -10; y <= 10; y++)
        for (x = -36; x <= 36; x++)
            if (x * x * 100 / (36 * 36) + y * y * 100 / 100 <= 100)
                Blend(&c, c.ax + x, c.ay + 26 + y, 0x507040, 200), Blend(&c, c.tx + x, c.ty + 22 + y, 0x507040, 200);
    Disc(&c, c.ax + shakeA, c.ay, 22, 0x6070C0, 230);
    Disc(&c, c.tx + shakeT, c.ty, 18, 0xC06060, 230);
    Draw(&c, &a, frame);
    return a.frames;
}
