// Moemon Co-op: ball entrance and recall effects.
//
// Each Moemon keeps its own effect with its ball (like the ball's color and
// decoration): how its body forms (materialization), the particles around the
// entrance, the energy color, how it's recalled and a sound accent. They're
// stored in the Pokémon itself (MON_DATA_MP_BALLFX: substruct 0's spare u16;
// MON_DATA_MP_BALLFX_SOUND: the 4 spare flag bits), so they travel with trades
// and every game in a link battle shows them.
//
// Timing: a body effect never takes longer than the game's own send-out (the
// Moemon fully formed with its own colors 16 frames after the ball opens) or
// recall (22 frames); particles may drift on afterwards without holding
// anything up. Several entrances at once (doubles, triples) share a budget so
// the screen stays readable, and the spotlight dims the background only once.
//
// Graphics: tools/moemon_ballfx/gen.py -> mp_ballfx_gfx.h. All particles use
// one 16-color palette (the overworld has a single free one), each set is only
// in VRAM while it's on screen, and the "energy" art is recolored when loaded.
#include <string.h>
#include "global.h"
#include "mp.h"
#include "decompress.h"
#include "gpu_regs.h"
#include "main.h"
#include "malloc.h"
#include "palette.h"
#include "pokemon.h"
#include "random.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "task.h"
#include "trig.h"
#include "util.h"
#include "constants/rgb.h"
#include "constants/songs.h"

#include "mp_ballfx_gfx.h"

#define TAG_FX_PAL        0x5F00
#define TAG_FX_SET(s)     (0x5F10 + (s))
#define TAG_FX_ENERGY(c)  (0x5F30 + (c))

#define FORM_LEN    16   // the game's own send-out: formed and in its colors 16 frames after the ball opens
#define RECALL_LEN  22   // the game's own recall

// ---------------------------------------------------------------------------
// Data: names, packing, presets

static const char *const sFormNames[MPFX_FORM_COUNT] = {
    "DEFAULT", "ANIME BEAM", "SMOKE FORM", "BUBBLE FORM", "LIGHTNING", "HAUNTED FADE",
    "DIGITAL", "INK", "CRYSTAL", "METEOR DROP", "POP-IN", "RISE UP",
};
static const char *const sPartNames[MPFX_PART_COUNT] = {
    "DEFAULT", "NONE", "CONFETTI", "EXPLOSION", "SMOKE PUFF", "FIREWORKS", "FLOWER BLOOM", "BUBBLES",
    "LIGHTNING BOLT", "WISPS + EYES", "PIXEL BITS", "INK SPLASH", "CRYSTAL SHARDS", "SPOTLIGHT", "DUST RING",
    "ROYAL SPARKLES",
};
static const char *const sRecallNames[MPFX_RECALL_COUNT] = {
    "DEFAULT", "CLASSIC BEAM", "DISSOLVE", "REVERSE CONFETTI", "SMOKE RETREAT", "DIGITAL SHUTDOWN", "CURTAIN CALL",
};
static const char *const sSoundNames[MPFX_SOUND_COUNT] = {
    "AUTO", "NONE", "SOFT CHIME", "POP", "BOOM", "FANFARE", "THUNDER", "SPOOKY",
};

struct FxPreset { const char *name; struct MpBallFx fx; };
static const struct FxPreset sPresets[] = {
    {"DEFAULT",              {MPFX_FORM_DEFAULT,   MPFX_PART_DEFAULT,   MP_BALL_RED,    MPFX_RECALL_DEFAULT,  MPFX_SOUND_AUTO}},
    {"HAUNTED",              {MPFX_FORM_HAUNTED,   MPFX_PART_WISPS,     MP_BALL_PURPLE, MPFX_RECALL_SMOKE,    MPFX_SOUND_SPOOKY}},
    {"SUPERSTAR",            {MPFX_FORM_POPIN,     MPFX_PART_SPOTLIGHT, MP_BALL_GOLD,   MPFX_RECALL_CURTAIN,  MPFX_SOUND_FANFARE}},
    {"FIRECRACKER",          {MPFX_FORM_ANIME_BEAM, MPFX_PART_FIREWORKS, MP_BALL_ORANGE, MPFX_RECALL_CONFETTI, MPFX_SOUND_BOOM}},
    {"TEAM ROCKET'S BUDGET", {MPFX_FORM_SMOKE,     MPFX_PART_EXPLOSION, MP_BALL_BLACK,  MPFX_RECALL_SMOKE,    MPFX_SOUND_POP}},
    {"GLITCH",               {MPFX_FORM_DIGITAL,   MPFX_PART_PIXELS,    MP_BALL_GREEN,  MPFX_RECALL_DIGITAL,  MPFX_SOUND_AUTO}},
    {"BLOOM",                {MPFX_FORM_ANIME_BEAM, MPFX_PART_FLOWER,   MP_BALL_PINK,   MPFX_RECALL_DISSOLVE, MPFX_SOUND_CHIME}},
    {"CLASSIC ANIME",        {MPFX_FORM_ANIME_BEAM, MPFX_PART_NONE,     MP_BALL_RED,    MPFX_RECALL_BEAM,     MPFX_SOUND_AUTO}},
    {"ROYALTY",              {MPFX_FORM_RISE,      MPFX_PART_ROYAL,     MP_BALL_GOLD,   MPFX_RECALL_CURTAIN,  MPFX_SOUND_FANFARE}},
    {"STORM",                {MPFX_FORM_LIGHTNING, MPFX_PART_LIGHTNING, MP_BALL_YELLOW, MPFX_RECALL_DISSOLVE, MPFX_SOUND_THUNDER}},
    {"DEEP SEA",             {MPFX_FORM_BUBBLE,    MPFX_PART_BUBBLES,   MP_BALL_CYAN,   MPFX_RECALL_DISSOLVE, MPFX_SOUND_AUTO}},
    {"SHOOTING STAR",        {MPFX_FORM_METEOR,    MPFX_PART_DUST,      MP_BALL_BLUE,   MPFX_RECALL_BEAM,     MPFX_SOUND_AUTO}},
};

const char *MP_BallFx_FormName(u8 i)   { return sFormNames[i < MPFX_FORM_COUNT ? i : 0]; }
const char *MP_BallFx_PartName(u8 i)   { return sPartNames[i < MPFX_PART_COUNT ? i : 0]; }
const char *MP_BallFx_RecallName(u8 i) { return sRecallNames[i < MPFX_RECALL_COUNT ? i : 0]; }
const char *MP_BallFx_SoundName(u8 i)  { return sSoundNames[i < MPFX_SOUND_COUNT ? i : 0]; }
u8 MP_BallFx_PresetCount(void)         { return ARRAY_COUNT(sPresets); }
const char *MP_BallFx_PresetName(u8 i) { return sPresets[i < ARRAY_COUNT(sPresets) ? i : 0].name; }

void MP_BallFx_Preset(u8 i, struct MpBallFx *out)
{
    *out = sPresets[i < ARRAY_COUNT(sPresets) ? i : 0].fx;
}

// The preset this effect is, or -1
s8 MP_BallFx_WhichPreset(const struct MpBallFx *fx)
{
    u8 i;
    for (i = 0; i < ARRAY_COUNT(sPresets); i++)
        if (memcmp(&sPresets[i].fx, fx, sizeof(*fx)) == 0)
            return i;
    return -1;
}

// MY PRESET 1-3: the player's own mixes, kept with the save (<save>.coop, "fxpreset1" =
// used bit 30 | sound << 16 | packed effect)
#define USER_PRESET_USED (1 << 30)

bool8 MP_BallFx_GetUserPreset(u8 slot, struct MpBallFx *out)
{
    char key[16];
    s32 v;
    if (slot >= MPFX_USER_PRESETS)
        return FALSE;
    snprintf(key, sizeof(key), "fxpreset%d", slot + 1);
    v = MP_SaveData_Get(key);
    if (!(v & USER_PRESET_USED))
        return FALSE;
    MP_BallFx_Unpack(v & 0xFFFF, (v >> 16) & 0x7, out);
    return TRUE;
}

void MP_BallFx_SaveUserPreset(u8 slot, const struct MpBallFx *fx)
{
    char key[16];
    if (slot >= MPFX_USER_PRESETS)
        return;
    snprintf(key, sizeof(key), "fxpreset%d", slot + 1);
    MP_SaveData_Set(key, USER_PRESET_USED | ((fx->sound & 0x7) << 16) | MP_BallFx_Pack(fx));
}

s8 MP_BallFx_WhichUserPreset(const struct MpBallFx *fx)
{
    struct MpBallFx p;
    u8 i;
    for (i = 0; i < MPFX_USER_PRESETS; i++)
        if (MP_BallFx_GetUserPreset(i, &p) && memcmp(&p, fx, sizeof(p)) == 0)
            return i;
    return -1;
}

void MP_BallFx_EffectName(const struct MpBallFx *fx, char *out, u32 size)
{
    s8 i = MP_BallFx_WhichPreset(fx);
    if (i >= 0)
        snprintf(out, size, "%s", sPresets[i].name);
    else if ((i = MP_BallFx_WhichUserPreset(fx)) >= 0)
        snprintf(out, size, "MY PRESET %d", i + 1);
    else
        snprintf(out, size, "CUSTOM");
}

// filler: form 0-3, particles 4-7, energy color 8-11, recall 12-14; sound in the spare flag bits
void MP_BallFx_OfBoxMon(const struct BoxPokemon *boxMon, struct MpBallFx *out)
{
    u16 v = GetBoxMonData((struct BoxPokemon *)boxMon, MON_DATA_MP_BALLFX, NULL);
    out->form = v & 0xF;
    out->part = (v >> 4) & 0xF;
    out->color = (v >> 8) & 0xF;
    out->recall = (v >> 12) & 0x7;
    out->sound = GetBoxMonData((struct BoxPokemon *)boxMon, MON_DATA_MP_BALLFX_SOUND, NULL) & 0x7;
    if (out->form >= MPFX_FORM_COUNT)
        out->form = 0;
    if (out->color >= MP_BALL_COLORS)
        out->color = 0;
    if (out->recall >= MPFX_RECALL_COUNT)
        out->recall = 0;
}

void MP_BallFx_OfMon(struct Pokemon *mon, struct MpBallFx *out)
{
    MP_BallFx_OfBoxMon(&mon->box, out);
}

u16 MP_BallFx_Pack(const struct MpBallFx *fx)
{
    return (fx->form & 0xF) | ((fx->part & 0xF) << 4) | ((fx->color & 0xF) << 8) | ((fx->recall & 0x7) << 12);
}

void MP_BallFx_Unpack(u16 v, u8 sound, struct MpBallFx *out)
{
    out->form = (v & 0xF) < MPFX_FORM_COUNT ? (v & 0xF) : 0;
    out->part = (v >> 4) & 0xF;
    out->color = ((v >> 8) & 0xF) < MP_BALL_COLORS ? ((v >> 8) & 0xF) : 0;
    out->recall = ((v >> 12) & 0x7) < MPFX_RECALL_COUNT ? ((v >> 12) & 0x7) : 0;
    out->sound = sound & 0x7;
}

void MP_BallFx_SetBoxMon(struct BoxPokemon *boxMon, const struct MpBallFx *fx)
{
    u16 v = MP_BallFx_Pack(fx);
    u8 s = fx->sound & 0x7;
    SetBoxMonData(boxMon, MON_DATA_MP_BALLFX, &v);
    SetBoxMonData(boxMon, MON_DATA_MP_BALLFX_SOUND, &s);
}

bool8 MP_BallFx_IsDefaultEntrance(const struct MpBallFx *fx)
{
    return fx->form == MPFX_FORM_DEFAULT && fx->part == MPFX_PART_DEFAULT
        && (fx->sound == MPFX_SOUND_AUTO || fx->sound == MPFX_SOUND_NONE);
}

// ---------------------------------------------------------------------------
// Colors

static const u16 sEnergyRgb[MP_BALL_COLORS] = {
    [MP_BALL_RED]    = RGB(31, 6, 5),
    [MP_BALL_BLUE]   = RGB(8, 14, 31),
    [MP_BALL_GREEN]  = RGB(8, 28, 10),
    [MP_BALL_YELLOW] = RGB(31, 29, 6),
    [MP_BALL_PINK]   = RGB(31, 14, 24),
    [MP_BALL_PURPLE] = RGB(21, 10, 31),
    [MP_BALL_ORANGE] = RGB(31, 17, 3),
    [MP_BALL_CYAN]   = RGB(8, 28, 31),
    [MP_BALL_BLACK]  = RGB(6, 6, 8),
    [MP_BALL_SILVER] = RGB(25, 26, 28),
    [MP_BALL_GOLD]   = RGB(31, 24, 6),
};
// the particle palette's entry closest to each energy color
static const u8 sEnergyIndex[MP_BALL_COLORS] = {
    [MP_BALL_RED] = 8, [MP_BALL_BLUE] = 11, [MP_BALL_GREEN] = 13, [MP_BALL_YELLOW] = 6, [MP_BALL_PINK] = 9,
    [MP_BALL_PURPLE] = 10, [MP_BALL_ORANGE] = 7, [MP_BALL_CYAN] = 12, [MP_BALL_BLACK] = 4, [MP_BALL_SILVER] = 2,
    [MP_BALL_GOLD] = 15,
};
static const u8 sConfettiColors[] = {8, 11, 13, 6, 9, 12};

static u16 MixRgb(u16 a, u16 b, u8 bWeight16)
{
    s32 ar = a & 0x1F, ag = (a >> 5) & 0x1F, ab = (a >> 10) & 0x1F;
    s32 br = b & 0x1F, bg = (b >> 5) & 0x1F, bb = (b >> 10) & 0x1F;
    ar += (br - ar) * bWeight16 / 16;
    ag += (bg - ag) * bWeight16 / 16;
    ab += (bb - ab) * bWeight16 / 16;
    return RGB(ar, ag, ab);
}

u16 MP_BallFx_EnergyRgb(u8 color)
{
    return sEnergyRgb[color < MP_BALL_COLORS ? color : 0];
}

// ---------------------------------------------------------------------------
// Sounds

static const u16 sAccentSongs[MPFX_SOUND_COUNT] = {
    [MPFX_SOUND_AUTO] = 0, [MPFX_SOUND_NONE] = 0,
    [MPFX_SOUND_CHIME] = SE_SHINY, [MPFX_SOUND_POP] = SE_BALLOON_RED, [MPFX_SOUND_BOOM] = SE_M_SELF_DESTRUCT,
    [MPFX_SOUND_FANFARE] = SE_SUCCESS, [MPFX_SOUND_THUNDER] = SE_M_THUNDERBOLT, [MPFX_SOUND_SPOOKY] = SE_M_CONFUSE_RAY,
};

static u16 AutoSong(const struct MpBallFx *fx)
{
    switch (fx->part)
    {
    case MPFX_PART_CONFETTI:  return SE_BALLOON_RED;
    case MPFX_PART_EXPLOSION: return SE_M_SELF_DESTRUCT;
    case MPFX_PART_FIREWORKS: return SE_M_COMET_PUNCH;
    case MPFX_PART_FLOWER:    return SE_M_SWEET_SCENT;
    case MPFX_PART_BUBBLES:   return SE_M_BUBBLE;
    case MPFX_PART_LIGHTNING: return SE_M_THUNDERBOLT;
    case MPFX_PART_WISPS:     return SE_M_CONFUSE_RAY;
    case MPFX_PART_PIXELS:    return SE_M_LOCK_ON;
    case MPFX_PART_INK:       return SE_M_BUBBLE3;
    case MPFX_PART_CRYSTAL:   return SE_ICE_BREAK;
    case MPFX_PART_SPOTLIGHT: return SE_M_DETECT;
    case MPFX_PART_DUST:      return SE_M_ROCK_THROW;
    case MPFX_PART_ROYAL:     return SE_SUCCESS;
    }
    switch (fx->form)
    {
    case MPFX_FORM_HAUNTED:   return SE_M_CONFUSE_RAY;
    case MPFX_FORM_LIGHTNING: return SE_M_THUNDER_WAVE;
    case MPFX_FORM_BUBBLE:    return SE_M_BUBBLE;
    case MPFX_FORM_CRYSTAL:   return SE_ICE_CRACK;
    case MPFX_FORM_DIGITAL:   return SE_M_LOCK_ON;
    case MPFX_FORM_POPIN:     return SE_BALLOON_RED;
    case MPFX_FORM_METEOR:    return SE_M_ROCK_THROW;
    }
    return 0;
}

static void PlayAccent(const struct MpBallFx *fx, s8 pan)
{
    u16 song = fx->sound == MPFX_SOUND_AUTO ? AutoSong(fx) : sAccentSongs[fx->sound < MPFX_SOUND_COUNT ? fx->sound : 0];
    if (song)
        PlaySE12WithPanning(song, pan);
}

// ---------------------------------------------------------------------------
// Particles: drawn by the effects layer over the finished frame
// (MP_BallFx_Composite, from the platform's VDraw), so they aren't bound by the
// GBA's sprites, VRAM or palettes: hundreds at once, any color, 16 levels of
// see-through or additive light, scaled and rotated - always nearest-neighbor
// on the 240x160 frame, so they still look like the game.

enum
{
    MOVE_FREE,     // velocity + gravity + drag
    MOVE_FLUTTER,  // falls slowly, swaying (after it starts going down)
    MOVE_HOME,     // eases into (tx, ty) by the end of its life
    MOVE_SPIRAL,   // circles inward to (tx, ty)
    MOVE_ORBIT,    // circles (tx, ty) on a flat ellipse
    MOVE_RISE,     // floats up swaying, pops at the end
};

enum { BLEND_NORMAL, BLEND_ADD, BLEND_GLOW };   // GLOW: adds light on dark pixels, paints on bright ones

#define PF_ROCKET     0x01   // bursts into sparks when it ends
#define PF_FAR_DIM    0x02   // orbiting: fainter on the far side (behind the Moemon)
#define PF_POPPED     0x04
#define PF_NOCLIP     0x08   // may cover the battle text box
#define PF_STICK      0x10   // follows a body's sprite (tx/ty = offset)
#define PF_UNDER      0x20   // behind the Moemon (underSprite): skips the pixels it covers

#define MAX_PARTS 1024
#define ART_NONE  0xFF

struct FxPart
{
    bool8 used;
    u8 art, nFrames, frameTime;
    u8 color;        // energy color
    u8 blend;
    u8 alpha;        // 16 = solid
    u8 fadeIn, fadeOut;
    u8 delay;        // hidden and still until then
    u8 move, sway, popArt, flags;
    u8 rot;
    s8 dRot;
    u8 trailEvery, trailArt;
    u16 solid;       // nonzero: every pixel this color (a piece of the Moemon itself)
    u8 underSprite;
    u8 stickSprite;
    u16 scale;       // 256 = 1
    s16 dScale;
    u16 stretchX, stretchY;   // extra squash/stretch (256 = none)
    u16 age, life;
    s32 x, y;        // 8 fractional bits
    s16 vx, vy, ay;
    u8 drag;
    s16 tx, ty;
    u16 angle;
    s16 radius, dAngle, dRadius;
};

static struct FxPart sParts[MAX_PARTS];
static u16 sPartHigh;               // parts in use are below this
static MainCallback sPartScreen;    // the screen they belong to (drawn only there)

static bool8 sFieldMode;   // smaller and fewer, on the overworld
static s16 sClipX0, sClipY0, sClipX1 = DISPLAY_WIDTH, sClipY1 = DISPLAY_HEIGHT;
static bool8 sClipSet;
static u8 sCurMon = MAX_SPRITES;   // the Moemon the effect being started belongs to

static struct FxPart *Spawn(u8 art, u8 color, s16 x, s16 y)
{
    u16 i;
    struct FxPart *p;
    for (i = 0; i < MAX_PARTS; i++)
        if (!sParts[i].used)
            break;
    if (i == MAX_PARTS)
        return NULL;
    p = &sParts[i];
    memset(p, 0, sizeof(*p));
    p->used = TRUE;
    p->art = art;
    p->nFrames = 1;
    p->color = color < MP_BALL_COLORS ? color : 0;
    p->alpha = 16;
    p->scale = 256;
    p->stretchX = p->stretchY = 256;
    p->popArt = ART_NONE;
    p->underSprite = sCurMon;
    p->x = x << 8;
    p->y = y << 8;
    p->life = 30;
    if (i + 1 > sPartHigh)
        sPartHigh = i + 1;
    sPartScreen = gMain.callback2;
    return p;
}

static u16 PartIndex(struct FxPart *p)
{
    return p ? p - sParts : 0xFFFF;
}

static struct FxPart *PartAt(u16 i)
{
    return (i < MAX_PARTS && sParts[i].used) ? &sParts[i] : NULL;
}

static void Kill(u16 *i)
{
    if (*i < MAX_PARTS)
        sParts[*i].used = FALSE;
    *i = 0xFFFF;
}

// random in [-r, r]
static s16 Rnd(s16 r)
{
    if (r <= 0)
        return 0;
    return (s16)(Random() % (2 * r + 1)) - r;
}

static u16 RndU(u16 n)
{
    return n ? Random() % n : 0;
}

static void FireworkBurst(s16 x, s16 y);

static void UpdatePart(struct FxPart *p)
{
    s32 dx, dy;

    if (p->delay)
    {
        p->delay--;
        return;
    }
    p->age++;
    if (p->age >= p->life)
    {
        if (p->popArt != ART_NONE && !(p->flags & PF_POPPED))
        {
            p->flags |= PF_POPPED;
            p->art = p->popArt;
            p->nFrames = 1;
            p->life += 5;
            p->vx = p->vy = 0;
            p->fadeOut = 3;
            return;
        }
        if (p->flags & PF_ROCKET)
            FireworkBurst(p->x >> 8, p->y >> 8);
        p->used = FALSE;
        return;
    }
    switch (p->move)
    {
    case MOVE_FREE:
    case MOVE_FLUTTER:
        p->vy += p->ay;
        if (p->drag)
        {
            p->vx = p->vx * (256 - p->drag) / 256;
            p->vy = p->vy * (256 - p->drag) / 256;
        }
        if (p->move == MOVE_FLUTTER && p->vy > 0x70)
            p->vy = 0x70;   // paper and petals fall slowly
        p->x += p->vx;
        p->y += p->vy;
        if (p->sway && (p->move != MOVE_FLUTTER || p->vy > 0))
            p->x += Sin((p->age * 6 + p->angle) & 0xFF, p->sway * 24);
        break;
    case MOVE_RISE:
        if (p->flags & PF_POPPED)
            break;
        p->y += p->vy;
        p->x += Sin((p->age * 8 + p->angle) & 0xFF, p->sway * 20);
        break;
    case MOVE_HOME:
        dx = (p->tx << 8) - p->x;
        dy = (p->ty << 8) - p->y;
        {
            s32 left = p->life - p->age;
            if (left < 1)
                left = 1;
            p->x += dx / left;
            p->y += dy / left;
        }
        break;
    case MOVE_SPIRAL:
    case MOVE_ORBIT:
        p->angle += p->dAngle;
        p->radius += p->dRadius;
        if (p->radius < 0)
            p->radius = 0;
        p->x = (p->tx << 8) + Cos((p->angle >> 8) & 0xFF, p->radius >> 8) * 256;
        p->y = (p->ty << 8) + Sin((p->angle >> 8) & 0xFF, p->radius >> 8) * 256 / (p->move == MOVE_ORBIT ? 3 : 1);
        if (p->flags & PF_FAR_DIM)
        {
            if (((p->angle >> 8) & 0xFF) >= 128)
                p->flags |= PF_UNDER;
            else
                p->flags &= ~PF_UNDER;
        }
        break;
    }
    if (p->flags & PF_STICK && p->stickSprite < MAX_SPRITES && gSprites[p->stickSprite].inUse)
    {
        struct Sprite *s = &gSprites[p->stickSprite];
        p->x = (s->x + s->x2 + p->tx) << 8;
        p->y = (s->y + s->y2 + p->ty) << 8;
    }
    p->rot += p->dRot;
    if (p->dScale)
    {
        s32 sc = p->scale + p->dScale;
        p->scale = sc < 16 ? 16 : (sc > 0x1000 ? 0x1000 : sc);
    }
    if (p->trailEvery && p->age % p->trailEvery == 0)
    {
        struct FxPart *t = Spawn(p->trailArt, p->color, p->x >> 8, p->y >> 8);
        if (t)
        {
            t->blend = BLEND_GLOW;
            t->life = 10;
            t->fadeOut = 10;
            t->alpha = 12;
            t->dScale = -12;
            t->flags = p->flags & PF_NOCLIP;
        }
    }
}

static void UpdateParts(void)
{
    u16 i, high = 0;
    for (i = 0; i < sPartHigh; i++)
    {
        if (!sParts[i].used)
            continue;
        UpdatePart(&sParts[i]);
        if (sParts[i].used)
            high = i + 1;
    }
    // bursts may have added parts past the old end
    for (i = sPartHigh; i < MAX_PARTS; i++)
        if (sParts[i].used)
            high = i + 1;
    sPartHigh = high;
}

static bool8 AnyParts(void)
{
    return sPartHigh != 0;
}

// ---------------------------------------------------------------------------
// Screen moments: spotlights (the rest of the frame dims, a lit cone stays),
// flashes, and the GBA blend / mosaic registers the Moemon's own sprite needs.

#define MAX_SPOTS 3

static EWRAM_DATA struct
{
    bool8 saved;
    u16 bldcnt, bldalpha, mosaic;
    u8 flashTimer, flashLevel;
    u8 mosaicWant;
    u8 recent;           // entrances in the last moments (for the budget)
    u8 recentTimer[6];
    struct { s16 x, feet, halfTop, halfBottom; u8 timer, len; } spot[MAX_SPOTS];
} sScreen = {0};

static void ScreenSave(void)
{
    if (!sScreen.saved)
    {
        sScreen.bldcnt = GetGpuReg(REG_OFFSET_BLDCNT);
        sScreen.bldalpha = GetGpuReg(REG_OFFSET_BLDALPHA);
        sScreen.mosaic = GetGpuReg(REG_OFFSET_MOSAIC);
        sScreen.saved = TRUE;
    }
}

static void ScreenRestore(void)
{
    if (sScreen.saved)
    {
        SetGpuReg(REG_OFFSET_BLDCNT, sScreen.bldcnt);
        SetGpuReg(REG_OFFSET_BLDALPHA, sScreen.bldalpha);
        SetGpuReg(REG_OFFSET_MOSAIC, sScreen.mosaic);
        sScreen.saved = FALSE;
    }
}

static void ScreenFlash(u8 frames, u8 level)
{
    if (sScreen.flashTimer < frames)
        sScreen.flashTimer = frames;
    if (sScreen.flashLevel < level)
        sScreen.flashLevel = level;
}

static void ScreenSpot(s16 x, s16 feet, s16 halfBottom, u8 frames)
{
    u8 i;
    for (i = 0; i < MAX_SPOTS; i++)
    {
        if (sScreen.spot[i].timer == 0)
        {
            sScreen.spot[i].x = x;
            sScreen.spot[i].feet = feet;
            sScreen.spot[i].halfTop = halfBottom / 5;
            sScreen.spot[i].halfBottom = halfBottom;
            sScreen.spot[i].timer = sScreen.spot[i].len = frames;
            return;
        }
    }
}

// ---------------------------------------------------------------------------
// Bodies: the Moemon sprite forming (entrance) or going back (recall)

#define MAX_BODIES 8
#define FLAG_FRONT   0x01   // a front sprite (opponent side): its feet are higher in the picture
#define FLAG_FIELD   0x02   // a follower on the overworld

struct FxBody
{
    bool8 active;
    bool8 recall;
    u8 mon;
    u8 id;           // form or recall
    u8 color;
    u8 flags;
    u8 t;
    u8 hopT;         // explosion: a little bounce from this frame (0 = none)
    s16 bx, by;      // where the ball is
    s16 baseX2, baseY2;
    u8 origAffineMode, origMatrixNum, origObjMode;
    bool8 origMosaic, origPaused, ownMatrix;
    u16 helper[3];   // our own particles (bubble, crystal, trail, scanline)
    u8 alpha;        // see-through level wanted for the sprite (16 = solid)
};

static EWRAM_DATA struct FxBody sBodies[MAX_BODIES] = {0};

static u8 BodyHeight(struct FxBody *b)
{
    return gSprites[b->mon].oam.size == SPRITE_SIZE(32x32) ? 32 : 64;
}

static void BodyScale(struct FxBody *b, s32 sx, s32 sy, bool8 anchorBottom)
{
    struct Sprite *s = &gSprites[b->mon];
    u8 n = s->oam.matrixNum;
    s32 h = BodyHeight(b);
    if (!(s->oam.affineMode & ST_OAM_AFFINE_ON_MASK))
        return;
    if (sx < 6)
        sx = 6;
    if (sy < 6)
        sy = 6;
    gOamMatrices[n].a = 0x10000 / sx;
    gOamMatrices[n].b = 0;
    gOamMatrices[n].c = 0;
    gOamMatrices[n].d = 0x10000 / sy;
    if (anchorBottom)
    {
        // keep the feet on the ground: back sprites stand on the picture's bottom edge
        s32 feet = (b->flags & FLAG_FRONT) ? h * 3 / 8 : h / 2;
        s->y2 = b->baseY2 + feet * (256 - sy) / 256;
    }
    else
    {
        s->y2 = b->baseY2;
    }
}

static void BodyTint(struct FxBody *b, s32 coeff, u16 color)
{
    u16 off = OBJ_PLTT_ID(gSprites[b->mon].oam.paletteNum);
    if (coeff < 0)
        coeff = 0;
    BlendPalette(off, 16, coeff > 16 ? 16 : coeff, color);
}

static void BodyBegin(struct FxBody *b)
{
    struct Sprite *s = &gSprites[b->mon];
    b->origAffineMode = s->oam.affineMode;
    b->origMatrixNum = s->oam.matrixNum;
    b->origObjMode = s->oam.objMode;
    b->origMosaic = s->oam.mosaic;
    b->origPaused = s->affineAnimPaused;
    b->baseX2 = s->x2;
    b->baseY2 = s->y2;
    b->ownMatrix = FALSE;
    b->alpha = 16;
    b->helper[0] = b->helper[1] = b->helper[2] = 0xFFFF;
    if (!(s->oam.affineMode & ST_OAM_AFFINE_ON_MASK))
    {
        u8 m = AllocOamMatrix();
        if (m != 0xFF)
        {
            s->oam.matrixNum = m;
            b->ownMatrix = TRUE;
        }
    }
    s->affineAnimPaused = TRUE;
    if (b->ownMatrix || (s->oam.affineMode & ST_OAM_AFFINE_ON_MASK))
    {
        s->oam.affineMode = ST_OAM_AFFINE_DOUBLE;
        CalcCenterToCornerVec(s, s->oam.shape, s->oam.size, s->oam.affineMode);
        gOamMatrices[s->oam.matrixNum].a = gOamMatrices[s->oam.matrixNum].d = 0x100;
        gOamMatrices[s->oam.matrixNum].b = gOamMatrices[s->oam.matrixNum].c = 0;
    }
}

// Puts the sprite back the way it was (visible after an entrance, hidden after a recall)
static void BodyEnd(struct FxBody *b)
{
    struct Sprite *s = &gSprites[b->mon];
    u8 n = s->oam.matrixNum, i;

    for (i = 0; i < ARRAY_COUNT(b->helper); i++)
        Kill(&b->helper[i]);
    if (s->oam.affineMode & ST_OAM_AFFINE_ON_MASK)
    {
        gOamMatrices[n].a = gOamMatrices[n].d = 0x100;
        gOamMatrices[n].b = gOamMatrices[n].c = 0;
    }
    BodyTint(b, 0, 0);
    s->x2 = b->baseX2;
    s->y2 = b->baseY2;
    s->oam.objMode = b->origObjMode;
    s->oam.mosaic = b->origMosaic;
    if (b->ownMatrix)
    {
        FreeOamMatrix(n);
        s->oam.matrixNum = b->origMatrixNum;
    }
    s->oam.affineMode = b->origAffineMode;
    CalcCenterToCornerVec(s, s->oam.shape, s->oam.size, s->oam.affineMode);
    s->affineAnimPaused = b->origPaused;
    s->invisible = b->recall;
    b->active = FALSE;
}

static s16 OffX(const struct Sprite *s) { return s->coordOffsetEnabled ? gSpriteCoordOffsetX : 0; }
static s16 OffY(const struct Sprite *s) { return s->coordOffsetEnabled ? gSpriteCoordOffsetY : 0; }
static s16 MonX(struct FxBody *b) { return gSprites[b->mon].x + b->baseX2 + OffX(&gSprites[b->mon]); }
static s16 MonY(struct FxBody *b) { return gSprites[b->mon].y + b->baseY2 + OffY(&gSprites[b->mon]); }

static s16 Dist(s16 v)
{
    return sFieldMode ? v / 2 : v;
}

// Fewer particles when several Moemon come out together (so it stays readable)
static u8 Budget(u8 n)
{
    u8 k = sScreen.recent;
    if (sFieldMode)
        n = (n + 1) / 2;
    if (k >= 3)
        return (n * 5 + 9) / 10;
    if (k == 2)
        return (n * 7 + 9) / 10;
    return n;
}

static void NoteEntrance(void)
{
    u8 i;
    for (i = 0; i < ARRAY_COUNT(sScreen.recentTimer); i++)
    {
        if (sScreen.recentTimer[i] == 0)
        {
            sScreen.recentTimer[i] = 40;
            break;
        }
    }
    sScreen.recent = 0;
    for (i = 0; i < ARRAY_COUNT(sScreen.recentTimer); i++)
        if (sScreen.recentTimer[i])
            sScreen.recent++;
}

// Ramp 0..256 over [t0, t1]
static s32 Ramp(s32 t, s32 t0, s32 t1)
{
    if (t <= t0)
        return 0;
    if (t >= t1)
        return 256;
    return (t - t0) * 256 / (t1 - t0);
}

// A glow (additive) at (x, y)
static struct FxPart *Glow(u8 color, s16 x, s16 y, bool8 big, u8 life, u16 scale, s16 dScale)
{
    struct FxPart *p = Spawn(big ? FXA_EXTRA_GLOW32 : FXA_EXTRA_GLOW16, color, x, y);
    if (p)
    {
        p->blend = BLEND_ADD;
        p->alpha = 12;
        p->life = life;
        p->fadeOut = life / 2;
        p->scale = scale;
        p->dScale = dScale;
    }
    return p;
}

// An expanding ring (shockwave)
static struct FxPart *Ring(u8 color, bool8 white, s16 x, s16 y, u8 life, u16 scale, s16 dScale, u16 flat)
{
    struct FxPart *p = Spawn(white ? FXA_EXTRA_WRING : FXA_EXTRA_RING, color, x, y);
    if (p)
    {
        p->blend = BLEND_ADD;
        p->alpha = 14;
        p->life = life;
        p->fadeOut = life;
        p->scale = scale;
        p->dScale = dScale;
        p->stretchY = flat;
    }
    return p;
}

// ---------------------------------------------------------------------------
// The particle effects (around an entrance)

static void Confetti(s16 cx, s16 cy, s16 dir)
{
    u8 i, n = Budget(22);
    for (i = 0; i < n; i++)
    {
        u8 col = RndU(6);
        struct FxPart *p = Spawn(FXA_CONFETTI_CONFETTI8_0 + col * 3, 0, cx + Rnd(3), cy + Rnd(3));
        if (!p)
            return;
        p->move = MOVE_FLUTTER;
        p->vx = dir * (0xA0 + RndU(0x200));
        p->vy = -(0x260 + RndU(0x220));
        if (sFieldMode)
            p->vx /= 2, p->vy = p->vy * 2 / 3;
        p->ay = 0x22;
        p->sway = 3 + RndU(3);
        p->angle = RndU(256);
        p->nFrames = 3;
        p->frameTime = 3 + RndU(4);
        p->life = 60 + RndU(30);
        p->fadeOut = 14;
        p->delay = RndU(3);
    }
    // the cannon's puff
    {
        struct FxPart *p = Spawn(FXA_EXTRA_SOFT, 0, cx, cy);
        if (p)
        {
            p->alpha = 10;
            p->life = 12;
            p->fadeOut = 12;
            p->scale = 128;
            p->dScale = 18;
            p->vx = dir * 0x80;
            p->vy = -0x80;
        }
    }
}

static void PartConfetti(s16 mx, s16 my, s16 feet)
{
    Confetti(mx - Dist(24), feet, -1);
    Confetti(mx + Dist(24), feet, 1);
}

static void PartExplosion(s16 mx, s16 my, struct FxBody *b)
{
    u8 i;
    struct FxPart *p;
    // a bright flash, a fireball, a shockwave, embers, then soot
    ScreenFlash(3, 10);
    if ((p = Spawn(FXA_EXPLOSION_FLASH, 0, mx, my)))
    {
        p->blend = BLEND_ADD;
        p->life = 6;
        p->fadeOut = 4;
        p->scale = 200;
        p->dScale = 40;
    }
    if ((p = Spawn(FXA_EXPLOSION_BLAST0, 0, mx, my)))
    {
        p->nFrames = 3;
        p->frameTime = 4;
        p->life = 14;
        p->fadeOut = 4;
        p->delay = 1;
        p->scale = 300;
        p->dScale = 10;
    }
    Ring(MP_BALL_ORANGE, FALSE, mx, my + Dist(8), 14, 120, 40, 140);
    for (i = 0; i < Budget(16); i++)
    {
        u8 ang = RndU(256);
        if (!(p = Spawn(FXA_EXTRA_EMBER, 0, mx, my)))
            break;
        p->blend = BLEND_GLOW;
        p->vx = Cos(ang, Dist(0x260 + RndU(0x180)));
        p->vy = Sin(ang, Dist(0x200 + RndU(0x140))) - 0x80;
        p->ay = 0x10;
        p->drag = 8;
        p->life = 20 + RndU(14);
        p->fadeOut = 10;
    }
    for (i = 0; i < Budget(7); i++)
    {
        u8 ang = RndU(256);
        if (!(p = Spawn(FXA_EXPLOSION_SOOT0, 0, mx + Rnd(Dist(14)), my + Rnd(Dist(10)))))
            break;
        p->vx = Cos(ang, Dist(0x240));
        p->vy = Sin(ang, Dist(0x120)) - 0x80;
        p->drag = 7;
        p->life = 28 + RndU(10);
        p->fadeOut = 16;
        p->alpha = 13;
        p->delay = 7;
        p->scale = 180 + RndU(80);
        p->dScale = 3;
        p->nFrames = 2;
        p->frameTime = 30;
    }
    if (b != NULL)
        b->hopT = 6;
}

static void PartSmoke(s16 mx, s16 my, u8 n, u8 life, s16 spread)
{
    u8 i;
    for (i = 0; i < Budget(n); i++)
    {
        u8 ang = RndU(256);
        struct FxPart *p = Spawn(FXA_SMOKE_SMOKE0 + RndU(2), 0, mx + Rnd(Dist(spread)), my + Rnd(Dist(spread)));
        if (!p)
            return;
        p->vx = Cos(ang, Dist(0x1A0)) + Rnd(0x30);
        p->vy = Sin(ang, Dist(0xC0)) - 0x30;
        p->drag = 10;
        p->scale = 200 + RndU(100);
        p->dScale = 4;   // billows as it goes
        p->rot = RndU(256);
        p->dRot = Rnd(2);
        p->life = life + RndU(10);
        p->fadeOut = life / 2;
        p->alpha = 14;
    }
}

static void PartFireworks(s16 mx, s16 my, s16 feet)
{
    u8 i, n = Budget(5);
    for (i = 0; i < n; i++)
    {
        struct FxPart *p = Spawn(FXA_FIREWORKS_ROCKET, 0, mx + Dist((i - n / 2) * 14), feet);
        if (!p)
            return;
        p->vy = -Dist(0x480) - RndU(0xA0);
        p->vx = Dist((i - n / 2) * 0x20) + Rnd(0x18);
        p->ay = 0x14;
        p->life = 15 + RndU(5);
        p->delay = i * 3;
        p->flags = PF_ROCKET;
        p->trailEvery = 1;
        p->trailArt = FXA_EXTRA_EMBER;
    }
}

static void FireworkBurst(s16 x, s16 y)
{
    u8 i, col = RndU(6), n = Budget(22);
    struct FxPart *p;
    if ((p = Glow(MP_BALL_YELLOW, x, y, FALSE, 8, 160, 30)))
        p->alpha = 8;
    for (i = 0; i < n; i++)
    {
        u8 ang = i * 256 / n + RndU(8);
        s16 speed = (i & 1) ? 0x1C0 : 0x120;
        if (!(p = Spawn(FXA_FIREWORKS_SPARK8_0 + col * 2, 0, x, y)))
            return;
        p->blend = BLEND_GLOW;
        p->vx = Cos(ang, Dist(speed));
        p->vy = Sin(ang, Dist(speed));
        p->drag = 9;
        p->ay = 0x07;
        p->nFrames = 2;
        p->frameTime = 12;
        p->life = 30 + RndU(10);
        p->fadeOut = 14;
    }
}

static void PartFlower(s16 mx, s16 feet)
{
    struct FxPart *p = Spawn(FXA_FLOWER_BLOOM0, 0, mx, feet);
    u8 i;
    if (p)
    {
        p->nFrames = 3;
        p->frameTime = 3;
        p->life = 36;
        p->fadeOut = 12;
        p->scale = 200;
        p->dScale = 4;
        p->flags = PF_UNDER;
    }
    if ((p = Glow(MP_BALL_PINK, mx, feet, TRUE, 16, 160, 12)))
        p->flags = PF_UNDER;
    for (i = 0; i < Budget(24); i++)
    {
        u8 ang = i * 256 / 24 + RndU(10);
        if (!(p = Spawn(FXA_FLOWER_PETAL0, 0, mx, feet - 2)))
            return;
        p->move = MOVE_FLUTTER;
        p->vx = Cos(ang, Dist(0x140 + RndU(0x100)));
        p->vy = -0x120 - RndU(0x140);
        p->ay = 0x10;
        p->sway = 3 + RndU(3);
        p->angle = RndU(256);
        p->nFrames = 2;
        p->frameTime = 5 + RndU(3);
        p->rot = RndU(256);
        p->dRot = Rnd(6);
        p->life = 56 + RndU(24);
        p->fadeOut = 14;
        p->delay = 5 + RndU(4);
    }
}

static void PartBubbles(s16 mx, s16 my, s16 feet, u8 n)
{
    u8 i;
    for (i = 0; i < Budget(n); i++)
    {
        u8 r = RndU(4);
        struct FxPart *p = Spawn(r == 0 ? FXA_BUBBLES_BUBBLE16 : FXA_BUBBLES_BUBBLE8, 0, mx + Rnd(Dist(30)), feet - RndU(Dist(34)));
        if (!p)
            return;
        p->move = MOVE_RISE;
        p->vy = -(0x70 + RndU(0xA0));
        p->sway = 2 + RndU(3);
        p->angle = RndU(256);
        p->life = 18 + RndU(36);
        p->popArt = FXA_BUBBLES_POP;
        p->alpha = 13;
        p->scale = r == 1 ? 180 : 256;
        p->delay = RndU(6);
    }
}

static void PartLightning(s16 mx, s16 my)
{
    struct FxPart *p;
    u8 i;
    ScreenFlash(4, 12);
    for (i = 0; i < 2; i++)
    {
        // the bolt, from off the top of the screen down to it
        if ((p = Spawn(FXA_LIGHTNING_BOLT, 0, mx + 10 + (i ? 3 : 0), my - 26 - (i ? 64 : 0))))
        {
            p->blend = i ? BLEND_ADD : BLEND_NORMAL;
            p->life = 9;
            p->fadeOut = 5;
            p->stretchX = i ? 220 : 256;
            p->flags = PF_NOCLIP;
        }
    }
    Glow(MP_BALL_YELLOW, mx, my + Dist(14), TRUE, 14, 220, 16);
    Ring(MP_BALL_YELLOW, TRUE, mx, my + Dist(24), 12, 100, 36, 110);
    for (i = 0; i < Budget(14); i++)
    {
        u8 ang = RndU(256);
        if (!(p = Spawn(FXA_LIGHTNING_ZAP0, 0, mx, my + Dist(12))))
            return;
        p->blend = BLEND_GLOW;
        p->vx = Cos(ang, Dist(0x240));
        p->vy = Sin(ang, Dist(0x140)) - 0x40;
        p->drag = 18;
        p->nFrames = 2;
        p->frameTime = 2;
        p->life = 14 + RndU(8);
        p->fadeOut = 8;
    }
}

static void PartWisps(s16 mx, s16 my)
{
    u8 i;
    struct FxPart *p;
    if ((p = Spawn(FXA_WISPS_EYES, 0, mx, my - Dist(6))))
    {
        p->blend = BLEND_ADD;
        p->life = 14;
        p->fadeIn = 3;
        p->fadeOut = 5;
    }
    if ((p = Spawn(FXA_WISPS_BLINK, 0, mx, my - Dist(6))))
    {
        p->blend = BLEND_ADD;
        p->life = 3;
        p->delay = 7;   // blinks once
    }
    for (i = 0; i < Budget(6); i++)
    {
        if (!(p = Spawn(FXA_WISPS_WISP0, 0, mx, my)))
            return;
        p->move = MOVE_ORBIT;
        p->tx = mx;
        p->ty = my + Dist(10);
        p->angle = (i * 256 / 6) << 8;
        p->dAngle = 0x500;
        p->radius = Dist(34) << 8;
        p->dRadius = 0x18;
        p->nFrames = 3;
        p->frameTime = 4;
        p->life = 46;
        p->fadeIn = 6;
        p->fadeOut = 14;
        p->flags = PF_FAR_DIM;
        p->trailEvery = 3;
        p->trailArt = FXA_ENERGY_MOTE;
        p->color = MP_BALL_PURPLE;
    }
}

static void PartPixels(s16 mx, s16 my, u8 color)
{
    u8 i;
    struct FxPart *p;
    for (i = 0; i < Budget(36); i++)
    {
        u8 ang = RndU(256);
        if (!(p = Spawn(RndU(3) ? FXA_ENERGY_SQ2 : FXA_ENERGY_SQ4, color, mx + Rnd(Dist(10)), my + Rnd(Dist(10)))))
            return;
        p->vx = Cos(ang, Dist(0xC0 + RndU(0x200)));
        p->vy = Sin(ang, Dist(0xC0 + RndU(0x200)));
        p->drag = 16;
        p->life = 22 + RndU(18);
        p->fadeOut = 10;
        p->blend = RndU(2) ? BLEND_GLOW : BLEND_NORMAL;
    }
}

static void PartInk(s16 mx, s16 my, s16 feet, u8 color)
{
    u8 i;
    struct FxPart *p = Spawn(FXA_ENERGY_SPLAT, color, mx, feet - Dist(3));
    if (p)
    {
        p->life = 44;
        p->fadeOut = 16;
        p->scale = 60;
        p->stretchX = 360;   // flat on the ground, wider than the Moemon
        p->stretchY = 110;
        p->rot = RndU(256);
        p->flags = PF_UNDER;
    }
    // the splat slaps open in a few frames (UpdateInkSplat below)
    for (i = 0; i < Budget(16); i++)
    {
        u8 ang = 128 + 10 + RndU(108);   // upward
        if (!(p = Spawn(FXA_ENERGY_DROP, color, mx + Rnd(Dist(16)), feet - Dist(4))))
            return;
        p->vx = Cos(ang, Dist(0x220));
        p->vy = Sin(ang, Dist(0x2A0));
        p->ay = 0x22;
        p->life = 26 + RndU(10);
        p->fadeOut = 6;
        p->scale = 200 + RndU(120);
    }
}

static void PartCrystal(s16 mx, s16 my, u8 n)
{
    u8 i;
    struct FxPart *p;
    Glow(MP_BALL_CYAN, mx, my, TRUE, 12, 220, 14);
    for (i = 0; i < Budget(n); i++)
    {
        u8 ang = i * 256 / n + RndU(12);
        if (!(p = Spawn(FXA_CRYSTAL_SHARD0, 0, mx + Rnd(Dist(6)), my + Rnd(Dist(10)))))
            return;
        p->vx = Cos(ang, Dist(0x160 + RndU(0x180)));
        p->vy = Sin(ang, Dist(0x120 + RndU(0x140)));
        p->drag = 10;
        p->ay = 0x0C;
        p->nFrames = 3;
        p->frameTime = 3;
        p->rot = RndU(256);
        p->dRot = Rnd(10);
        p->scale = 200 + RndU(160);
        p->life = 28 + RndU(14);
        p->fadeOut = 12;
        if ((i % 3) == 0 && (p = Spawn(FXA_ROYAL_SPARKLE1, 0, mx + Rnd(Dist(28)), my + Rnd(Dist(24)))))
        {
            p->blend = BLEND_ADD;
            p->life = 10;
            p->delay = 4 + RndU(14);
            p->fadeOut = 6;
        }
    }
}

static void PartSpotlight(s16 mx, s16 my, s16 feet)
{
    u8 i;
    struct FxPart *p;
    ScreenSpot(mx, feet, Dist(36), 44);
    // a pool of light at its feet and dust floating in the beam
    if ((p = Spawn(FXA_EXTRA_GLOW32, MP_BALL_YELLOW, mx, feet - 2)))
    {
        p->blend = BLEND_ADD;
        p->alpha = 8;
        p->life = 44;
        p->fadeIn = 6;
        p->fadeOut = 10;
        p->stretchX = 512;
        p->stretchY = 120;
        p->flags = PF_UNDER;
    }
    for (i = 0; i < Budget(10); i++)
    {
        if (!(p = Spawn(FXA_ENERGY_SQ2, MP_BALL_SILVER, mx + Rnd(Dist(26)), feet - RndU(Dist(90)))))
            return;
        p->blend = BLEND_ADD;
        p->vy = 0x18 + RndU(0x18);
        p->sway = 1;
        p->angle = RndU(256);
        p->life = 40;
        p->fadeIn = 8;
        p->fadeOut = 10;
        p->alpha = 9;
    }
}

static void PartDust(s16 mx, s16 feet, u8 n)
{
    u8 i;
    struct FxPart *p;
    if ((p = Spawn(FXA_DUST_IMPACT, 0, mx, feet - 4)))
    {
        p->blend = BLEND_ADD;
        p->life = 8;
        p->fadeOut = 6;
        p->flags = PF_UNDER;
    }
    if ((p = Ring(MP_BALL_SILVER, TRUE, mx, feet - 2, 14, 90, 34, 70)))
        p->flags = PF_UNDER;
    for (i = 0; i < Budget(n); i++)
    {
        u8 ang = i * 256 / n;
        if (!(p = Spawn(FXA_DUST_DUST0, 0, mx, feet - 2)))
            return;
        p->vx = Cos(ang, Dist(0x1E0));
        p->vy = Sin(ang, Dist(0x50)) - 0x14;
        p->drag = 14;
        p->nFrames = 2;
        p->frameTime = 14;
        p->scale = 200 + RndU(100);
        p->dScale = 3;
        p->life = 30 + RndU(8);
        p->fadeOut = 14;
        p->alpha = 13;
    }
}

static void PartRoyal(s16 mx, s16 my, s16 head)
{
    u8 i;
    struct FxPart *p;
    for (i = 0; i < Budget(18); i++)
    {
        if (!(p = Spawn(FXA_ROYAL_SPARKLE0, 0, mx + Rnd(Dist(34)), my + Rnd(Dist(30)))))
            break;
        p->blend = BLEND_GLOW;
        p->nFrames = 3;
        p->frameTime = 4;
        p->life = 12;
        p->delay = i * 2;
        p->vy = -0x18;
        p->fadeOut = 4;
    }
    // golden glitter drifting down
    for (i = 0; i < Budget(14); i++)
    {
        if (!(p = Spawn(FXA_ENERGY_SQ2, MP_BALL_GOLD, mx + Rnd(Dist(40)), head - RndU(Dist(20)))))
            break;
        p->blend = BLEND_GLOW;
        p->move = MOVE_FLUTTER;
        p->vy = 0x30;
        p->sway = 2;
        p->angle = RndU(256);
        p->life = 40 + RndU(10);
        p->fadeOut = 12;
        p->delay = 4 + RndU(10);
    }
    if ((p = Spawn(FXA_ROYAL_CROWN, 0, mx, head)))
    {
        p->move = MOVE_RISE;
        p->vy = -0x0C;
        p->life = 38;
        p->delay = 6;
        p->fadeIn = 3;
        p->fadeOut = 10;
        p->scale = 120;
        p->dScale = 30;   // grows in, then stays (clamped in StartParticles' follow-up)
    }
    if ((p = Glow(MP_BALL_GOLD, mx, head, FALSE, 12, 160, 20)))
        p->delay = 6;
}

// Starts the particle effect for an entrance. feet/head: the Moemon's ground line and top.
static void StartParticles(u8 part, u8 color, s16 mx, s16 my, s16 feet, s16 head, struct FxBody *b)
{
    switch (part)
    {
    case MPFX_PART_CONFETTI:  PartConfetti(mx, my, feet - 4); break;
    case MPFX_PART_EXPLOSION: PartExplosion(mx, my, b); break;
    case MPFX_PART_SMOKE:     PartSmoke(mx, my, 12, 40, 12); break;
    case MPFX_PART_FIREWORKS: PartFireworks(mx, my, feet); break;
    case MPFX_PART_FLOWER:    PartFlower(mx, feet); break;
    case MPFX_PART_BUBBLES:   PartBubbles(mx, my, feet, 22); break;
    case MPFX_PART_LIGHTNING: PartLightning(mx, my); break;
    case MPFX_PART_WISPS:     PartWisps(mx, my); break;
    case MPFX_PART_PIXELS:    PartPixels(mx, my, color); break;
    case MPFX_PART_INK:       PartInk(mx, my, feet, color); break;
    case MPFX_PART_CRYSTAL:   PartCrystal(mx, my, 20); break;
    case MPFX_PART_SPOTLIGHT: PartSpotlight(mx, my, feet); break;
    case MPFX_PART_DUST:      PartDust(mx, feet, 14); break;
    case MPFX_PART_ROYAL:     PartRoyal(mx, my, head); break;
    }
}

// Per-frame tweaks some particles need (scale curves that aren't linear)
static void ShapeParts(void)
{
    u16 i;
    for (i = 0; i < sPartHigh; i++)
    {
        struct FxPart *p = &sParts[i];
        if (!p->used || p->delay)
            continue;
        if (p->art == FXA_ENERGY_SPLAT && p->age <= 5)
            p->scale = 60 + 196 * p->age / 5;   // slaps open
        else if (p->art == FXA_ROYAL_CROWN && p->scale > 256)
            p->scale = 256, p->dScale = 0;
        else if (p->art == FXA_EXPLOSION_BLAST0 && p->scale > 340)
            p->dScale = 0;
    }
}

// ---------------------------------------------------------------------------
// The manager: one task while anything runs. Moves the particles, keeps the
// GBA blend / mosaic registers right for the Moemon sprites, ends itself.

static void Task_FxManager(u8 taskId);

static void EnsureManager(void)
{
    if (!FuncIsActiveTask(Task_FxManager))
        CreateTask(Task_FxManager, 1);
}

static void Task_FxManager(u8 taskId)
{
    u8 i, alpha = 0;
    bool8 bodies = FALSE, mosaic = FALSE, spots = FALSE;

    for (i = 0; i < ARRAY_COUNT(sScreen.recentTimer); i++)
        if (sScreen.recentTimer[i])
            sScreen.recentTimer[i]--;
    UpdateParts();
    ShapeParts();
    if (sScreen.flashTimer && --sScreen.flashTimer == 0)
        sScreen.flashLevel = 0;
    for (i = 0; i < MAX_SPOTS; i++)
    {
        if (sScreen.spot[i].timer)
        {
            sScreen.spot[i].timer--;
            spots = TRUE;
        }
    }
    for (i = 0; i < MAX_BODIES; i++)
    {
        if (!sBodies[i].active)
            continue;
        bodies = TRUE;
        if (sBodies[i].alpha < 16 && (alpha == 0 || sBodies[i].alpha < alpha))
            alpha = sBodies[i].alpha;
        if (gSprites[sBodies[i].mon].oam.mosaic)
            mosaic = TRUE;
    }

    // a fading Moemon sprite and a mosaic one need the blend / mosaic registers for a moment
    if (alpha || mosaic)
        ScreenSave();
    if (sScreen.saved)
    {
        if (alpha)
        {
            SetGpuReg(REG_OFFSET_BLDCNT, BLDCNT_TGT2_BG0 | BLDCNT_TGT2_BG1 | BLDCNT_TGT2_BG2 | BLDCNT_TGT2_BG3
                                         | BLDCNT_TGT2_OBJ | BLDCNT_TGT2_BD);
            SetGpuReg(REG_OFFSET_BLDALPHA, BLDALPHA_BLEND(alpha, 16 - alpha));
        }
        else
        {
            SetGpuReg(REG_OFFSET_BLDCNT, sScreen.bldcnt);
            SetGpuReg(REG_OFFSET_BLDALPHA, sScreen.bldalpha);
        }
        SetGpuReg(REG_OFFSET_MOSAIC, (sScreen.mosaic & 0x00FF) | (sScreen.mosaicWant << 8) | (sScreen.mosaicWant << 12));
        if (!alpha && !mosaic)
            ScreenRestore();
    }

    if (!AnyParts() && !bodies && !spots && !sScreen.flashTimer)
    {
        ScreenRestore();
        sScreen.mosaicWant = 0;
        DestroyTask(taskId);
    }
}

// ---------------------------------------------------------------------------
// Entrances and recalls

#define tSlot data[0]

static void Task_FxBody(u8 taskId);

static struct FxBody *NewBody(u8 mon)
{
    u8 i;
    for (i = 0; i < MAX_BODIES; i++)
        if (sBodies[i].active && sBodies[i].mon == mon)
            BodyEnd(&sBodies[i]);   // a new effect on the same sprite replaces the old one
    for (i = 0; i < MAX_BODIES; i++)
    {
        if (!sBodies[i].active)
        {
            memset(&sBodies[i], 0, sizeof(sBodies[i]));
            sBodies[i].active = TRUE;
            sBodies[i].mon = mon;
            return &sBodies[i];
        }
    }
    return NULL;
}

static s16 Feet(struct FxBody *b)
{
    s16 h = BodyHeight(b);
    return MonY(b) + ((b->flags & FLAG_FRONT) ? h * 3 / 8 : h / 2);
}

static s16 Head(struct FxBody *b)
{
    s16 h = BodyHeight(b);
    return MonY(b) - ((b->flags & FLAG_FRONT) ? h / 4 : h / 8);
}

static struct FxPart *Helper(struct FxBody *b, u8 slot, u8 art, u8 color, s16 x, s16 y)
{
    struct FxPart *p;
    Kill(&b->helper[slot]);
    p = Spawn(art, color, x, y);
    if (p)
    {
        p->life = 0xFFFF;   // ends with the body
        b->helper[slot] = PartIndex(p);
    }
    return p;
}

static bool8 MonPixel(u8 spriteId, s16 sx, s16 sy, u16 *color);

// The Moemon as a grid of 4x4 squares in its own colors: calls back for each square
static u8 MonSquares(struct FxBody *b, u8 step, void (*each)(struct FxBody *, s16, s16, u16))
{
    s16 x, y, cx = MonX(b), cy = MonY(b), half = BodyHeight(b) / 2;
    u8 n = 0;
    u16 c;
    for (y = cy - half; y < cy + half; y += step)
        for (x = cx - half; x < cx + half; x += step)
            if (MonPixel(b->mon, x + step / 2, y + step / 2, &c))
            {
                each(b, x + step / 2, y + step / 2, c);
                n++;
            }
    return n;
}

static void SquareIn(struct FxBody *b, s16 x, s16 y, u16 c)
{
    // flies in from somewhere around it to where it belongs
    struct FxPart *p = Spawn(FXA_ENERGY_SQ4, b->color, x + Rnd(Dist(70)), y + Rnd(Dist(56)));
    if (!p)
        return;
    p->solid = c;
    p->move = MOVE_HOME;
    p->tx = x;
    p->ty = y;
    p->life = 6 + RndU(5);
    p->fadeIn = 3;
}

static void SquareOut(struct FxBody *b, s16 x, s16 y, u16 c)
{
    // stays where it was, then blinks out (a few float up a little)
    struct FxPart *p = Spawn(FXA_ENERGY_SQ4, b->color, x, y);
    if (!p)
        return;
    p->solid = c;
    p->life = 2 + RndU(12);
    if (RndU(4) == 0)
        p->vy = -0x30;
}

static void MoteFromBody(struct FxBody *b, s16 x, s16 y, u16 c)
{
    struct FxPart *p;
    if (RndU(5) != 0)
        return;
    if (!(p = Spawn(FXA_ENERGY_MOTE, b->color, x, y)))
        return;
    p->move = MOVE_HOME;
    p->blend = BLEND_GLOW;
    p->tx = b->bx;
    p->ty = b->by;
    p->life = 9 + RndU(10);
    p->delay = RndU(10);
    p->fadeIn = 3;
}

static void EntranceFrame(struct FxBody *b)
{
    struct Sprite *s = &gSprites[b->mon];
    u8 t = b->t;
    u16 energy = sEnergyRgb[b->color];
    u16 light = MixRgb(energy, RGB_WHITE, 6);
    s16 mx = MonX(b), my = MonY(b);
    struct FxPart *p;
    s32 r;
    u8 i;

    switch (b->id)
    {
    case MPFX_FORM_ANIME_BEAM:
        // energy pours out of the ball, a glowing silhouette grows into the Moemon, its colors come back
        if (t == 0)
        {
            for (i = 0; i < Budget(10); i++)
            {
                if (!(p = Spawn(FXA_ENERGY_STREAK, b->color, b->bx + Rnd(3), b->by + Rnd(3))))
                    break;
                p->move = MOVE_HOME;
                p->blend = BLEND_GLOW;
                p->tx = mx + Rnd(Dist(16));
                p->ty = my + Rnd(Dist(18));
                p->life = 4 + i;
                p->trailEvery = 1;
                p->trailArt = FXA_ENERGY_MOTE;
            }
            Glow(b->color, b->bx, b->by, FALSE, 10, 200, 24);
        }
        if (t == 2 && (p = Glow(b->color, mx, my, TRUE, 14, 120, 30)))
            p->alpha = 10;
        if (t == 9)
            Ring(b->color, FALSE, mx, my, 10, 160, 40, 256);
        r = t < 10 ? 26 + (300 - 26) * Ramp(t, 0, 9) / 256 : 300 - 44 * Ramp(t, 10, 12) / 256;
        BodyScale(b, r, r, TRUE);
        BodyTint(b, t < 9 ? 16 : 16 - 16 * Ramp(t, 9, 15) / 256, light);
        break;
    case MPFX_FORM_SMOKE:
        // its silhouette inside a billowing cloud that clears
        if (t == 0)
        {
            PartSmoke(mx, my, 10, 46, 14);
            PartSmoke(mx, my + Dist(14), 4, 36, 20);
        }
        BodyTint(b, t < 6 ? 16 : 16 - 16 * Ramp(t, 6, 15) / 256, RGB(4, 4, 5));
        break;
    case MPFX_FORM_BUBBLE:
        // forms inside a big floating bubble that pops
        if (t == 0 && (p = Helper(b, 0, FXA_BUBBLES_BUBBLE64, 0, mx, my)))
        {
            p->alpha = 13;
            p->scale = 128;
            p->flags = PF_STICK;
            p->stickSprite = b->mon;
        }
        if ((p = PartAt(b->helper[0])))
        {
            // wobbly: squash and stretch as it floats
            p->scale = 128 + 128 * Ramp(t, 0, 6) / 256;
            p->stretchX = 256 + Sin(t * 22, 18);
            p->stretchY = 256 - Sin(t * 22, 18);
            p->ty = -b->baseY2 + Sin(t * 16, 2);
        }
        r = 80 + 176 * Ramp(t, 0, 11) / 256;
        BodyScale(b, r, r, FALSE);
        s->y2 = b->baseY2 + Sin(t * 16, 2);
        BodyTint(b, t < 11 ? 7 : 7 - 7 * Ramp(t, 11, 15) / 256, RGB(16, 28, 31));
        if (t == 12)
        {
            Kill(&b->helper[0]);
            Ring(MP_BALL_CYAN, TRUE, mx, my, 8, 230, 30, 256);
            PartBubbles(mx, my, my + Dist(26), 14);
            for (i = 0; i < Budget(10); i++)
            {
                if ((p = Spawn(FXA_BUBBLES_POP, 0, mx + Cos(i * 25, Dist(30)), my + Sin(i * 25, Dist(30)))))
                {
                    p->life = 7;
                    p->fadeOut = 4;
                    p->vx = Cos(i * 25, 0x100);
                    p->vy = Sin(i * 25, 0x100);
                }
            }
        }
        break;
    case MPFX_FORM_LIGHTNING:
        // its silhouette flickers with sparks, then it's solid
        s->invisible = t < 11 && (t % 3) == 1;
        BodyTint(b, t < 11 ? 14 : 14 - 14 * Ramp(t, 11, 15) / 256, MixRgb(energy, RGB_WHITE, 9));
        if (t == 0 || t == 4 || t == 8)
        {
            Glow(b->color, mx, my, TRUE, 4, 240, 0);
            for (i = 0; i < Budget(3); i++)
            {
                if ((p = Spawn(FXA_LIGHTNING_ZAP0, 0, mx + Rnd(Dist(24)), my + Rnd(Dist(24)))))
                {
                    p->blend = BLEND_ADD;
                    p->nFrames = 2;
                    p->frameTime = 2;
                    p->life = 7;
                    p->fadeOut = 3;
                }
            }
        }
        break;
    case MPFX_FORM_HAUNTED:
        // fades in from nothing, wavering
        s->oam.objMode = ST_OAM_OBJ_BLEND;
        b->alpha = 1 + 15 * Ramp(t, 0, 15) / 256;
        s->x2 = b->baseX2 + Sin(t * 20, 2);
        BodyTint(b, 8 - 8 * Ramp(t, 4, 15) / 256, RGB(18, 10, 28));
        if (t == 0 && (p = Spawn(FXA_EXTRA_SOFT, MP_BALL_PURPLE, mx, my)))
        {
            // a faint haze where it will be (not a bright glow)
            p->alpha = 4;
            p->life = 18;
            p->fadeIn = 4;
            p->fadeOut = 10;
            p->scale = 600;
        }
        if (t == FORM_LEN - 1)
            b->alpha = 16;
        break;
    case MPFX_FORM_DIGITAL:
        // pixels gather, the picture sharpens out of a mosaic, a scanline passes
        if (t == 0)
            MonSquares(b, 4, SquareIn);   // its own pixels, in its own colors, assemble
        s->invisible = t < 4;
        s->oam.mosaic = t < 13;
        sScreen.mosaicWant = t < 13 ? (13 - t) * 12 / 9 : 0;
        if (sScreen.mosaicWant > 15)
            sScreen.mosaicWant = 15;
        if (t == 7 && (p = Helper(b, 0, FXA_ENERGY_SCAN, b->color, mx, Head(b) - 6)))
        {
            p->blend = BLEND_ADD;
            p->stretchX = 512;   // 64 px wide
            p->vy = (Feet(b) - Head(b) + 6) * 256 / 8;
        }
        BodyTint(b, t < 7 ? 12 : 12 - 12 * Ramp(t, 7, 15) / 256, light);
        break;
    case MPFX_FORM_INK:
        // painted on as an ink silhouette (upward), then its colors come through
        if (t == 0 && (p = Spawn(FXA_ENERGY_SPLAT, b->color, mx, Feet(b) - 4)))
        {
            p->life = 30;
            p->fadeOut = 12;
            p->stretchY = 110;
            p->scale = 300;
            p->flags = PF_UNDER;
        }
        r = 40 + 216 * Ramp(t, 0, 7) / 256;
        BodyScale(b, 256 + (256 - r) / 4, r, TRUE);
        BodyTint(b, t < 8 ? 16 : 16 - 16 * Ramp(t, 8, 15) / 256, MixRgb(energy, RGB_BLACK, 7));
        break;
    case MPFX_FORM_CRYSTAL:
        // a see-through crystal around it, then glittering fragments
        if (t == 0 && (p = Helper(b, 0, FXA_CRYSTAL_CASE, 0, mx, my)))
        {
            p->alpha = 9;
            p->flags = PF_STICK;
            p->stickSprite = b->mon;
            p->ty = -b->baseY2;
        }
        if ((p = PartAt(b->helper[0])))
            p->tx = (t >= 8 && t < 11) ? ((t & 1) ? 1 : -1) : 0;
        if (t == 3 && (p = Spawn(FXA_ROYAL_SPARKLE1, 0, mx - Dist(14), my - Dist(16))))
        {
            p->blend = BLEND_ADD;
            p->life = 8;
            p->nFrames = 1;
        }
        BodyTint(b, t < 11 ? 9 : 9 - 9 * Ramp(t, 11, 15) / 256, RGB(22, 30, 31));
        if (t == 11)
        {
            Kill(&b->helper[0]);
            ScreenFlash(2, 6);
            PartCrystal(mx, my, 16);
        }
        break;
    case MPFX_FORM_METEOR:
        // falls in a blazing streak, lands with a squash, a shockwave and a little bounce
        if (t == 0 && (p = Helper(b, 0, FXA_ENERGY_TRAIL, b->color, mx, my)))
        {
            p->blend = BLEND_ADD;
            p->scale = 320;
            p->rot = 236;   // along the fall
            p->trailEvery = 1;
            p->trailArt = FXA_ENERGY_MOTE;
        }
        if (t < 9)
        {
            s32 k = 256 - Ramp(t, 0, 9);   // 256 -> 0
            s32 fall = k * k / 256;        // accelerating
            BodyScale(b, 230, 280, FALSE);
            s->x2 = b->baseX2 + Dist(32) * fall / 256;
            s->y2 = b->baseY2 - Dist(110) * fall / 256;
            BodyTint(b, 16, light);
            if ((p = PartAt(b->helper[0])))
            {
                p->x = (mx + s->x2 - b->baseX2 + Dist(10)) << 8;
                p->y = (my + s->y2 - b->baseY2 - Dist(30)) << 8;
            }
        }
        else
        {
            if (t == 9)
            {
                Kill(&b->helper[0]);
                ScreenFlash(2, 8);
                PartDust(mx, Feet(b), 8);
                Glow(b->color, mx, Feet(b) - 6, TRUE, 10, 240, 10);
            }
            s->x2 = b->baseX2;
            r = t < 12 ? 70 - 70 * Ramp(t, 9, 12) / 256 : 0;   // squash, then back
            BodyScale(b, 256 + r / 2, 256 - r + (t >= 12 && t < 14 ? 16 : 0), TRUE);
            BodyTint(b, 16 - 16 * Ramp(t, 9, 15) / 256, light);
        }
        break;
    case MPFX_FORM_POPIN:
        // oversized for a moment, then it settles with a bounce
        if (t <= 5)
            r = 50 + (346 - 50) * Ramp(t, 0, 5) / 256;
        else if (t <= 8)
            r = 346 - (346 - 226) * Ramp(t, 5, 8) / 256;
        else if (t <= 11)
            r = 226 + (272 - 226) * Ramp(t, 8, 11) / 256;
        else
            r = 272 - 16 * Ramp(t, 11, 13) / 256;
        BodyScale(b, r, r, TRUE);
        BodyTint(b, 16 - 16 * Ramp(t, 2, 11) / 256, RGB_WHITE);
        if (t == 0)
            Glow(MP_BALL_SILVER, mx, my, TRUE, 8, 120, 40);
        if (t == 5)
            Ring(MP_BALL_SILVER, TRUE, mx, my, 9, 200, 36, 256);
        break;
    case MPFX_FORM_VANILLA:
        // the game's own: grows out of the ball in a white flash
        r = t < 13 ? 0x28 + 0x12 * t : 256;
        BodyScale(b, r, r, TRUE);
        BodyTint(b, 16 - t, RGB_WHITE);
        break;
    case MPFX_FORM_RISE:
        // rises from the ground
        if (t == 0)
        {
            PartDust(mx, Feet(b), 5);
            if ((p = Glow(b->color, mx, Feet(b) - 2, TRUE, 16, 200, 0)))
                p->stretchY = 100;
        }
        r = 10 + 246 * Ramp(t, 0, 11) / 256;
        if (t >= 12)
            r = 256 + (t < 14 ? 10 : 0);
        BodyScale(b, 216 + 40 * Ramp(t, 0, 11) / 256, r, TRUE);
        break;
    default:
        break;
    }

    // explosion: a little bounce while it forms
    if (b->hopT && t >= b->hopT && t < b->hopT + 8)
        s->y2 -= Sin((t - b->hopT) * 16, Dist(6));
}

static void RecallFrame(struct FxBody *b)
{
    struct Sprite *s = &gSprites[b->mon];
    u8 t = b->t;
    u16 energy = sEnergyRgb[b->color];
    u16 light = MixRgb(energy, RGB_WHITE, 6);
    s16 mx = MonX(b), my = MonY(b);
    struct FxPart *p;
    s32 r;
    u8 i;

    switch (b->id)
    {
    case MPFX_RECALL_BEAM:
        // a glowing silhouette that shrinks back to the ball
        BodyTint(b, 16 * Ramp(t, 0, 5) / 256, light);
        r = 256 - 250 * Ramp(t, 6, 20) / 256;
        BodyScale(b, r, r, FALSE);
        s->x2 = b->baseX2 + (b->bx - mx) * Ramp(t, 8, 21) / 512;
        s->y2 = b->baseY2 + (b->by - my) * Ramp(t, 8, 21) / 512;
        if (t == 4)
            Glow(b->color, mx, my, TRUE, 16, 260, -12);
        if (t == 10)
        {
            for (i = 0; i < Budget(8); i++)
            {
                if (!(p = Spawn(FXA_ENERGY_STREAK, b->color, mx + Rnd(Dist(12)), my + Rnd(Dist(12)))))
                    break;
                p->move = MOVE_HOME;
                p->blend = BLEND_GLOW;
                p->tx = b->bx;
                p->ty = b->by;
                p->life = 7 + i;
                p->trailEvery = 1;
                p->trailArt = FXA_ENERGY_MOTE;
            }
        }
        break;
    case MPFX_RECALL_DISSOLVE:
        // its body breaks into glowing motes that stream toward the ball
        BodyTint(b, 14 * Ramp(t, 0, 6) / 256, light);
        if (t >= 4)
        {
            s->oam.objMode = ST_OAM_OBJ_BLEND;
            b->alpha = 16 - 15 * Ramp(t, 4, 18) / 256;
        }
        if (t == 3)
            MonSquares(b, 4, MoteFromBody);   // from all over its body
        s->invisible = t >= 19;
        break;
    case MPFX_RECALL_CONFETTI:
        // it becomes paper that swirls back inside
        if (t == 0)
        {
            for (i = 0; i < Budget(28); i++)
            {
                u8 col = RndU(6);
                if (!(p = Spawn(FXA_CONFETTI_CONFETTI8_0 + col * 3, 0, mx, my)))
                    break;
                p->move = MOVE_SPIRAL;
                p->tx = mx;
                p->ty = my;
                p->angle = (i * 256 / 28) << 8;
                p->dAngle = 0x900 + RndU(0x200);
                p->radius = (Dist(30) + RndU(Dist(18))) << 8;
                p->dRadius = -p->radius / 14;
                p->nFrames = 3;
                p->frameTime = 3;
                p->life = 15;
                p->fadeIn = 3;
            }
        }
        if (t == 14)
        {
            // and the whole swirl darts to the ball
            for (i = 0; i < Budget(6); i++)
            {
                if (!(p = Spawn(FXA_CONFETTI_CONFETTI8_0 + RndU(6) * 3, 0, mx, my)))
                    break;
                p->move = MOVE_HOME;
                p->tx = b->bx;
                p->ty = b->by;
                p->life = 6 + i;
                p->nFrames = 3;
                p->frameTime = 2;
            }
        }
        r = 256 - 246 * Ramp(t, 4, 15) / 256;
        BodyScale(b, r, r, FALSE);
        BodyTint(b, 12 * Ramp(t, 3, 11) / 256, RGB_WHITE);
        s->invisible = t >= 15;
        break;
    case MPFX_RECALL_SMOKE:
        // a puff hides it; the smoke is sucked into the ball
        if (t == 0)
        {
            for (i = 0; i < Budget(9); i++)
            {
                if (!(p = Spawn(FXA_SMOKE_SMOKE0, 0, mx + Rnd(Dist(14)), my + Rnd(Dist(16)))))
                    break;
                p->scale = 120;
                p->dScale = 24;   // puffs up...
                p->life = 9;
                p->rot = RndU(256);
                p->alpha = 15;
                // ...then a second puff takes its place and heads for the ball
                if ((p = Spawn(FXA_SMOKE_SMOKE0, 0, mx + Rnd(Dist(14)), my + Rnd(Dist(16)))))
                {
                    p->delay = 9;
                    p->move = MOVE_HOME;
                    p->tx = b->bx;
                    p->ty = b->by;
                    p->scale = 330;
                    p->dScale = -24;
                    p->life = 11 + RndU(3);
                    p->rot = RndU(256);
                    p->dRot = 8;
                }
            }
        }
        s->invisible = t >= 5;
        break;
    case MPFX_RECALL_DIGITAL:
        // its sprite becomes a grid of squares that blink out
        s->oam.mosaic = t < 9;
        sScreen.mosaicWant = t < 9 ? 1 + t * 14 / 8 : 0;
        if (sScreen.mosaicWant > 15)
            sScreen.mosaicWant = 15;
        BodyTint(b, 10 * Ramp(t, 0, 8) / 256, light);
        if (t == 9)
        {
            // the mosaic's colors... are its own: a grid of squares that blink out
            s->oam.mosaic = FALSE;
            BodyTint(b, 0, 0);
            MonSquares(b, 4, SquareOut);
            s->invisible = TRUE;
            if ((p = Helper(b, 0, FXA_ENERGY_SCAN, b->color, mx, my)))
            {
                p->blend = BLEND_ADD;
                p->stretchX = 512;
                p->stretchY = 128;
            }
        }
        if ((p = PartAt(b->helper[0])))
            p->stretchY = t < 21 ? 128 - 6 * (t - 9) : 16;
        break;
    case MPFX_RECALL_VANILLA:
        // the game's own: tinted red, it shrinks away
        BodyTint(b, 16 * Ramp(t, 0, 10) / 256, RGB(31, 8, 6));
        r = t < 10 ? 256 : 0x10000 / (0x100 + (t - 10) * 0x30);
        BodyScale(b, r, r, TRUE);
        s->invisible = t >= 21;
        break;
    case MPFX_RECALL_CURTAIN:
        // a quick bow, a sparkle, then back to the ball
        if (t < 8)
        {
            r = Ramp(t, 0, 4) - Ramp(t, 4, 8);   // 0 -> 256 -> 0
            BodyScale(b, 256, 256 - 36 * r / 256, TRUE);
            s->y2 += Dist(4) * r / 256;
        }
        if (t == 8)
        {
            if ((p = Spawn(FXA_ROYAL_SPARKLE1, 0, mx + Dist(12), Head(b))))
            {
                p->blend = BLEND_ADD;
                p->life = 12;
                p->scale = 160;
                p->dScale = 30;
                p->dRot = 12;
                p->fadeOut = 6;
            }
        }
        if (t >= 9)
        {
            r = 256 - 250 * Ramp(t, 10, 21) / 256;
            BodyScale(b, r, r, FALSE);
            BodyTint(b, 16 * Ramp(t, 9, 13) / 256, RGB_WHITE);
        }
        break;
    }
}

static void Task_FxBody(u8 taskId)
{
    struct FxBody *b = &sBodies[gTasks[taskId].tSlot];
    u8 len;

    if (!b->active || !gSprites[b->mon].inUse)
    {
        b->active = FALSE;
        DestroyTask(taskId);
        return;
    }
    len = b->recall ? RECALL_LEN : FORM_LEN;
    if (b->t >= len)
    {
        sScreen.mosaicWant = 0;
        BodyEnd(b);
        DestroyTask(taskId);
        return;
    }
    sCurMon = b->mon;
    if (b->recall)
        RecallFrame(b);
    else
        EntranceFrame(b);
    b->t++;
}

static u8 Flags(u8 flags)
{
    sFieldMode = (flags & MPFX_FIELD) != 0;
    return ((flags & MPFX_FRONT) ? FLAG_FRONT : 0) | (sFieldMode ? FLAG_FIELD : 0);
}

// The Moemon (monSpriteId, already in place) comes out of a ball at (bx, by):
// its form (unless DEFAULT: the caller runs the game's own), its particles
// (unless DEFAULT) and its sound accent. TRUE if a form started.
bool8 MP_BallFx_StartEntrance(u8 monSpriteId, s16 bx, s16 by, u8 flags, const struct MpBallFx *fx)
{
    struct FxBody *b = NULL;
    s8 pan = (flags & MPFX_FRONT) ? 63 : -64;
    s16 mx, my, feet, head, h;
    u8 bflags;

    if (monSpriteId >= MAX_SPRITES)
        return FALSE;
    bflags = Flags(flags);
    sCurMon = monSpriteId;
    MPNet_Log("ballfx: entrance form %d part %d color %d", fx->form, fx->part, fx->color);
    EnsureManager();
    NoteEntrance();
    PlayAccent(fx, pan);

    h = gSprites[monSpriteId].oam.size == SPRITE_SIZE(32x32) ? 32 : 64;
    mx = gSprites[monSpriteId].x + gSprites[monSpriteId].x2 + OffX(&gSprites[monSpriteId]);
    my = gSprites[monSpriteId].y + gSprites[monSpriteId].y2 + OffY(&gSprites[monSpriteId]);
    feet = my + ((flags & MPFX_FRONT) ? h * 3 / 8 : h / 2);
    head = my - ((flags & MPFX_FRONT) ? h / 4 : h / 8) - Dist(12);

    if (fx->form != MPFX_FORM_DEFAULT && fx->form <= MPFX_FORM_VANILLA)
    {
        b = NewBody(monSpriteId);
        if (b != NULL)
        {
            u8 taskId;
            b->recall = FALSE;
            b->id = fx->form;
            b->color = fx->color < MP_BALL_COLORS ? fx->color : 0;
            b->flags = bflags;
            b->bx = bx;
            b->by = by;
            BodyBegin(b);
            gSprites[monSpriteId].invisible = FALSE;
            taskId = CreateTask(Task_FxBody, 2);
            gTasks[taskId].tSlot = b - sBodies;
            Task_FxBody(taskId);   // the first frame right away, so it never shows unformed
        }
    }
    if (fx->part >= MPFX_PART_CONFETTI && fx->part < MPFX_PART_COUNT)
        StartParticles(fx->part, fx->color, mx, my, feet, head, b);
    return b != NULL;
}

bool8 MP_BallFx_StartRecall(u8 monSpriteId, s16 bx, s16 by, u8 flags, const struct MpBallFx *fx)
{
    struct FxBody *b;
    u8 taskId, bflags;

    if (monSpriteId >= MAX_SPRITES || fx->recall == MPFX_RECALL_DEFAULT || fx->recall > MPFX_RECALL_VANILLA)
        return FALSE;
    bflags = Flags(flags);
    sCurMon = monSpriteId;
    b = NewBody(monSpriteId);
    if (b == NULL)
        return FALSE;
    MPNet_Log("ballfx: recall %d color %d", fx->recall, fx->color);
    EnsureManager();
    b->recall = TRUE;
    b->id = fx->recall;
    b->color = fx->color < MP_BALL_COLORS ? fx->color : 0;
    b->flags = bflags;
    b->bx = bx;
    b->by = by;
    BodyBegin(b);
    taskId = CreateTask(Task_FxBody, 2);
    gTasks[taskId].tSlot = b - sBodies;
    Task_FxBody(taskId);
    return TRUE;
}

// TRUE while this sprite's entrance or recall is still going
bool8 MP_BallFx_BodyRunning(u8 monSpriteId)
{
    u8 i;
    for (i = 0; i < MAX_BODIES; i++)
        if (sBodies[i].active && sBodies[i].mon == monSpriteId)
            return TRUE;
    return FALSE;
}

// TRUE while anything of ours is on screen
bool8 MP_BallFx_Active(void)
{
    return FuncIsActiveTask(Task_FxManager);
}

// Ends everything at once (leaving a screen): bodies put back, particles gone
void MP_BallFx_StopAll(void)
{
    u8 i;
    for (i = 0; i < MAX_BODIES; i++)
    {
        if (sBodies[i].active)
        {
            if (gSprites[sBodies[i].mon].inUse)
                BodyEnd(&sBodies[i]);
            sBodies[i].active = FALSE;
        }
    }
    for (i = 0; i < NUM_TASKS; i++)
        if (gTasks[i].isActive && gTasks[i].func == Task_FxBody)
            DestroyTask(i);
    memset(sParts, 0, sizeof(sParts));
    sPartHigh = 0;
    sScreen.flashTimer = sScreen.flashLevel = 0;
    memset(sScreen.spot, 0, sizeof(sScreen.spot));
    sScreen.mosaicWant = 0;
    ScreenRestore();
    if (FuncIsActiveTask(Task_FxManager))
        DestroyTask(FindTaskIdByFunc(Task_FxManager));
    memset(sScreen.recentTimer, 0, sizeof(sScreen.recentTimer));
    sScreen.recent = 0;
}

// Where particles may draw (the editor's preview box); 0,0,0,0 = automatic
void MP_BallFx_SetClip(s16 x0, s16 y0, s16 x1, s16 y1)
{
    sClipX0 = x0;
    sClipY0 = y0;
    sClipX1 = x1;
    sClipY1 = y1;
    sClipSet = (x1 > x0 && y1 > y0);
}

// A little ball (overworld) tossed from (x, y) to (tx, ty) in `life` frames; it pops open there
void MP_BallFx_MiniBall(s16 x, s16 y, s16 tx, s16 ty, u8 ballColor, u8 life)
{
    struct FxPart *p;
    sFieldMode = TRUE;
    sCurMon = MAX_SPRITES;
    EnsureManager();
    if (life < 2)
        life = 2;
    if ((p = Spawn(FXA_ENERGY_MINIBALL, ballColor, x, y)))
    {
        p->life = life;
        p->vx = (tx - x) * 256 / life;
        p->ay = 0x50;
        p->vy = (ty - y) * 256 / life - p->ay * life / 2;
        p->dRot = 24;
        p->flags = PF_NOCLIP;
    }
    if ((p = Spawn(FXA_EXTRA_GLOW16, MP_BALL_SILVER, tx, ty)))
    {
        p->blend = BLEND_ADD;
        p->alpha = 10;
        p->delay = life;
        p->life = 8;
        p->fadeOut = 6;
        p->scale = 120;
        p->dScale = 30;
    }
}

// ---------------------------------------------------------------------------
// The effects layer: drawn over the finished frame (platform VDraw)

static inline u16 Add555(u16 a, u16 b, u8 k)
{
    s32 r = (a & 0x1F) + (b & 0x1F) * k / 16, g = ((a >> 5) & 0x1F) + ((b >> 5) & 0x1F) * k / 16,
        bl = ((a >> 10) & 0x1F) + ((b >> 10) & 0x1F) * k / 16;
    return (r > 31 ? 31 : r) | ((g > 31 ? 31 : g) << 5) | ((bl > 31 ? 31 : bl) << 10);
}

static inline u16 Mix555(u16 a, u16 b, u8 k)   // k/16 of b over a
{
    s32 ar = a & 0x1F, ag = (a >> 5) & 0x1F, ab = (a >> 10) & 0x1F;
    s32 br = b & 0x1F, bg = (b >> 5) & 0x1F, bb = (b >> 10) & 0x1F;
    return (ar + (br - ar) * k / 16) | ((ag + (bg - ag) * k / 16) << 5) | ((ab + (bb - ab) * k / 16) << 10);
}

static const u8 sBayer4[4][4] = {{0, 8, 2, 10}, {12, 4, 14, 6}, {3, 11, 1, 9}, {15, 7, 13, 5}};

// The game's own whole-screen fade, applied to our layer too
static u16 Faded(u16 c)
{
    // only a fade over most of the sprites (a screen fade), not a flash aimed at the background
    if ((gPaletteFade.active || gPaletteFade.y) && MP_MaskTakesExtraPalettes(gPaletteFade.multipurpose1))
    {
        u8 y = gPaletteFade.y > 16 ? 16 : gPaletteFade.y;
        if (y)
            return Mix555(c, gPaletteFade.blendColor, y);
    }
    return c;
}

// The Moemon sprite's own pixel at screen (sx, sy) (scaled or not): TRUE and its color if it's drawn there
static bool8 MonPixel(u8 spriteId, s16 sx, s16 sy, u16 *color)
{
    struct Sprite *s = &gSprites[spriteId];
    static const u8 sDims[3][4][2] = {
        {{8, 8}, {16, 16}, {32, 32}, {64, 64}},
        {{16, 8}, {32, 8}, {32, 16}, {64, 32}},
        {{8, 16}, {8, 32}, {16, 32}, {32, 64}},
    };
    s32 w, h, lx, ly, dx, dy;
    const u8 *tiles;
    u8 v;
    if (spriteId >= MAX_SPRITES || !s->inUse || s->invisible || s->oam.shape > 2 || s->oam.bpp != ST_OAM_4BPP)
        return FALSE;
    w = sDims[s->oam.shape][s->oam.size][0];
    h = sDims[s->oam.shape][s->oam.size][1];
    dx = sx - (s->x + s->x2 + (s->coordOffsetEnabled ? gSpriteCoordOffsetX : 0));
    dy = sy - (s->y + s->y2 + (s->coordOffsetEnabled ? gSpriteCoordOffsetY : 0));
    if (s->oam.affineMode & ST_OAM_AFFINE_ON_MASK)
    {
        const struct OamMatrix *m = &gOamMatrices[s->oam.matrixNum];
        s32 rx = (m->a * dx + m->b * dy) >> 8;
        s32 ry = (m->c * dx + m->d * dy) >> 8;
        dx = rx;
        dy = ry;
    }
    lx = dx + w / 2;
    ly = dy + h / 2;
    if (lx < 0 || ly < 0 || lx >= w || ly >= h)
        return FALSE;
    tiles = (const u8 *)OBJ_VRAM0 + s->oam.tileNum * TILE_SIZE_4BPP;
    v = tiles[((ly / 8) * (w / 8) + lx / 8) * TILE_SIZE_4BPP + (ly % 8) * 4 + (lx % 8) / 2];
    v = (lx & 1) ? (v >> 4) : (v & 0xF);
    if (v == 0)
        return FALSE;
    if (color)
        *color = gPlttBufferUnfaded[OBJ_PLTT_ID(s->oam.paletteNum) + v] | 0x8000;
    return TRUE;
}

static void DrawPart(u16 *frame, const struct FxPart *p, s16 cx0, s16 cy0, s16 cx1, s16 cy1)
{
    const struct FxArt *art = &sFxArt[p->art];
    const u8 *px = sFxPixels + art->offset;
    u16 colors[19];
    u8 alpha = p->alpha, i;
    s32 sx, sy, cosv, sinv, halfW, halfH, x0, y0, x1, y1, x, y;
    s16 px0 = p->x >> 8, py0 = p->y >> 8;
    u16 energy = sEnergyRgb[p->color];

    // fades
    if (p->fadeIn && p->age < p->fadeIn)
        alpha = alpha * (p->age + 1) / (p->fadeIn + 1);
    if (p->fadeOut && p->life != 0xFFFF && p->life - p->age < p->fadeOut)
        alpha = alpha * (p->life - p->age) / p->fadeOut;
    if ((p->flags & PF_FAR_DIM) && ((p->angle >> 8) & 0xFF) >= 128)
        alpha = alpha * 12 / 16;   // the far side of the orbit (and behind the Moemon: PF_UNDER)
    if (alpha == 0)
        return;
    for (i = 1; i < 16; i++)
        colors[i] = Faded(sFxPalette[i]);
    colors[16] = Faded(energy);
    colors[17] = Faded(MixRgb(energy, RGB_WHITE, 8));
    colors[18] = Faded(MixRgb(energy, RGB_BLACK, 8));

    sx = p->scale * p->stretchX / 256;
    sy = p->scale * p->stretchY / 256;
    if (sx < 4 || sy < 4)
        return;
    cosv = gSineTable[(p->rot + 64) & 0xFF];
    sinv = gSineTable[p->rot & 0xFF];
    // bounding box (rotated)
    halfW = (art->w * sx / 256) / 2 + 1;
    halfH = (art->h * sy / 256) / 2 + 1;
    if (p->rot)
    {
        s32 m = halfW > halfH ? halfW : halfH;
        halfW = halfH = m * 3 / 2;
    }
    x0 = px0 - halfW; x1 = px0 + halfW;
    y0 = py0 - halfH; y1 = py0 + halfH;
    if (x0 < cx0) x0 = cx0;
    if (y0 < cy0) y0 = cy0;
    if (x1 > cx1) x1 = cx1;
    if (y1 > cy1) y1 = cy1;
    for (y = y0; y < y1; y++)
    {
        for (x = x0; x < x1; x++)
        {
            // back to the art's pixel (nearest)
            s32 dx = (x - px0) * 256 + 128, dy = (y - py0) * 256 + 128, ux, uy;
            u8 v;
            u16 *dst, c;
            if (p->rot)
            {
                s32 rx = (dx * cosv + dy * sinv) >> 8;
                s32 ry = (-dx * sinv + dy * cosv) >> 8;
                dx = rx;
                dy = ry;
            }
            ux = dx / sx + art->w / 2;
            uy = dy / sy + art->h / 2;
            if (dx < 0 && dx % sx)
                ux--;
            if (dy < 0 && dy % sy)
                uy--;
            if (ux < 0 || uy < 0 || ux >= art->w || uy >= art->h)
                continue;
            v = px[uy * art->w + ux];
            if (v == 0 || v > 18)
                continue;
            if ((p->flags & PF_UNDER) && MonPixel(p->underSprite, x, y, NULL))
                continue;   // the Moemon stands in front of it
            c = p->solid ? Faded(p->solid & 0x7FFF) : colors[v];
            dst = &frame[y * DISPLAY_WIDTH + x];
            if (p->blend == BLEND_ADD)
                *dst = Add555(*dst, c, alpha) | 0x8000;
            else if (p->blend == BLEND_GLOW)
            {
                u16 d = *dst;
                u8 lum = ((d & 0x1F) + ((d >> 5) & 0x1F) + ((d >> 10) & 0x1F)) / 6;   // 0-15
                *dst = Mix555(Add555(d, c, alpha), Mix555(d, c, alpha), lum + 1) | 0x8000;
            }
            else if (alpha >= 16)
                *dst = c | 0x8000;
            else
                *dst = Mix555(*dst, c, alpha) | 0x8000;
        }
    }
}

static bool8 InSpot(s16 x, s16 y, u8 *edge)
{
    u8 i;
    for (i = 0; i < MAX_SPOTS; i++)
    {
        s32 half, d;
        if (sScreen.spot[i].timer == 0 || y > sScreen.spot[i].feet + 6)
            continue;
        // a cone from the top of the screen down to its feet
        half = sScreen.spot[i].halfTop + (sScreen.spot[i].halfBottom - sScreen.spot[i].halfTop) * (y < 0 ? 0 : y) / (sScreen.spot[i].feet > 1 ? sScreen.spot[i].feet : 1);
        d = x - sScreen.spot[i].x;
        if (d < 0)
            d = -d;
        if (d <= half)
        {
            *edge = (half - d) < 4 ? (u8)(half - d) : 4;
            return TRUE;
        }
    }
    return FALSE;
}

void MP_BallFx_Composite(u16 *frame)
{
    s16 cx0 = 0, cy0 = 0, cx1 = DISPLAY_WIDTH, cy1 = DISPLAY_HEIGHT;
    u16 i;
    bool8 battle;

    if (!MP_BallFx_Active() || gMain.callback2 != sPartScreen || !MP_OBJ_LAYER_SHOWN())
        return;
    battle = gMain.inBattle;
    if (sClipSet)
    {
        cx0 = sClipX0; cy0 = sClipY0; cx1 = sClipX1; cy1 = sClipY1;
    }
    else if (battle)
    {
        cy1 = 112;   // the battle text box stays readable
    }

    // a spotlight: everything but its cone dims (dithered edge, like the GBA would)
    {
        u8 dim = 0, k;
        for (k = 0; k < MAX_SPOTS; k++)
        {
            u8 t = sScreen.spot[k].timer, len = sScreen.spot[k].len, d;
            if (!t)
                continue;
            d = (len - t) < 5 ? (len - t) * 2 : (t < 8 ? t : 9);
            if (d > dim)
                dim = d;
        }
        if (dim)
        {
            s16 x, y;
            for (y = cy0; y < cy1; y++)
            {
                for (x = cx0; x < cx1; x++)
                {
                    u16 *dst = &frame[y * DISPLAY_WIDTH + x];
                    u8 edge;
                    if (InSpot(x, y, &edge))
                    {
                        // lit: a touch brighter, softened at the edge with a dither
                        if (edge >= 4 || sBayer4[y & 3][x & 3] < edge * 4)
                            *dst = Add555(*dst, RGB(31, 30, 24), dim / 4) | 0x8000;
                        else
                            *dst = Mix555(*dst, 0, dim / 2) | 0x8000;
                    }
                    else
                    {
                        *dst = Mix555(*dst, 0, dim) | 0x8000;
                    }
                }
            }
        }
    }
    if (sScreen.flashTimer && sScreen.flashLevel)
    {
        s16 x, y;
        for (y = cy0; y < cy1; y++)
            for (x = cx0; x < cx1; x++)
                frame[y * DISPLAY_WIDTH + x] = Mix555(frame[y * DISPLAY_WIDTH + x], RGB_WHITE, sScreen.flashLevel * sScreen.flashTimer / 4 > 14 ? 14 : sScreen.flashLevel * sScreen.flashTimer / 4) | 0x8000;
    }

    for (i = 0; i < sPartHigh; i++)
    {
        const struct FxPart *p = &sParts[i];
        if (!p->used || p->delay)
            continue;
        DrawPart(frame, p, cx0, cy0, (p->flags & PF_NOCLIP) && !sClipSet ? DISPLAY_WIDTH : cx1,
                 (p->flags & PF_NOCLIP) && !sClipSet ? DISPLAY_HEIGHT : cy1);
    }
}
