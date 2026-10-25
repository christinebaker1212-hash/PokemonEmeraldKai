// Moemon Co-op: PMD Collab overworld sprites (include/mp_pmd.h; tools/moemon_pmd/import.py has the data layout).
// Sheets are inflated when someone first shows that Pokémon and shared; a sheet nobody has shown for a few seconds
// is freed (the big sprites' lists keep pointing at a frame for a frame or two after it goes).
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "palette.h"
#include "sprite.h"
#include "mp.h"
#include "mp_pmd.h"
#include "mp_life_field.h"

#include "../data/mp_pmd.h"

extern const u8 gMpPmdData[];
char *stbi_zlib_decode_noheader_malloc(const char *buffer, int len, int *outlen);

#define CACHE_MAX   48
#define FREE_AFTER  (60 * 5)   // frames unshown before a sheet is freed

struct MpPmdChunk
{
    const struct MpPmdIndex *index;
    u8 *raw;
    u16 refs;
    u16 idle;          // frames since the last view let go
    u8 bpp, colors;
    const u8 *pal, *shiny;   // u16 colors each (little-endian)
    const u8 *anims;         // u32 offsets, PMD_ANIM_COUNT of them
    u8 animCount;
};

static struct MpPmdChunk sCache[CACHE_MAX];

// What an animation a sheet hasn't got becomes: the nearest one it has (WALK last: every sheet has it)
static const u8 sFallbacks[PMD_ANIM_COUNT][4] = {
    [PMD_ANIM_WALK]        = {PMD_ANIM_IDLE},
    [PMD_ANIM_IDLE]        = {PMD_ANIM_WALK},
    [PMD_ANIM_SLEEP]       = {PMD_ANIM_LAYING, PMD_ANIM_EVENTSLEEP, PMD_ANIM_SIT, PMD_ANIM_IDLE},
    [PMD_ANIM_EAT]         = {PMD_ANIM_NOD, PMD_ANIM_IDLE},
    [PMD_ANIM_HOP]         = {PMD_ANIM_POSE, PMD_ANIM_IDLE},
    [PMD_ANIM_ROTATE]      = {PMD_ANIM_HOP, PMD_ANIM_POSE, PMD_ANIM_IDLE},
    [PMD_ANIM_ATTACK]      = {PMD_ANIM_STRIKE, PMD_ANIM_SHOOT, PMD_ANIM_QUICKSTRIKE, PMD_ANIM_IDLE},
    [PMD_ANIM_SHOOT]       = {PMD_ANIM_ATTACK, PMD_ANIM_STRIKE, PMD_ANIM_IDLE},
    [PMD_ANIM_STRIKE]      = {PMD_ANIM_ATTACK, PMD_ANIM_QUICKSTRIKE, PMD_ANIM_IDLE},
    [PMD_ANIM_LOOKUP]      = {PMD_ANIM_IDLE},
    [PMD_ANIM_CHARGE]      = {PMD_ANIM_POSE, PMD_ANIM_IDLE},
    [PMD_ANIM_SHOCK]       = {PMD_ANIM_HURT, PMD_ANIM_IDLE},
    [PMD_ANIM_TRIP]        = {PMD_ANIM_TUMBLE, PMD_ANIM_HURT, PMD_ANIM_IDLE},
    [PMD_ANIM_TUMBLE]      = {PMD_ANIM_TRIP, PMD_ANIM_HURT, PMD_ANIM_IDLE},
    [PMD_ANIM_HURT]        = {PMD_ANIM_CRINGE, PMD_ANIM_IDLE},
    [PMD_ANIM_CRINGE]      = {PMD_ANIM_HURT, PMD_ANIM_IDLE},
    [PMD_ANIM_NOD]         = {PMD_ANIM_IDLE},
    [PMD_ANIM_POSE]        = {PMD_ANIM_HOP, PMD_ANIM_IDLE},
    [PMD_ANIM_SIT]         = {PMD_ANIM_IDLE},
    [PMD_ANIM_DEEPBREATH]  = {PMD_ANIM_IDLE},
    [PMD_ANIM_LAYING]      = {PMD_ANIM_SLEEP, PMD_ANIM_SIT, PMD_ANIM_IDLE},
    [PMD_ANIM_FLOAT]       = {PMD_ANIM_IDLE},
    [PMD_ANIM_LEAPFORTH]   = {PMD_ANIM_HOP, PMD_ANIM_POSE, PMD_ANIM_IDLE},
    [PMD_ANIM_PULL]        = {PMD_ANIM_NOD, PMD_ANIM_IDLE},
    [PMD_ANIM_WAKE]        = {PMD_ANIM_IDLE},
    [PMD_ANIM_EVENTSLEEP]  = {PMD_ANIM_SLEEP, PMD_ANIM_LAYING, PMD_ANIM_IDLE},
    [PMD_ANIM_SWING]       = {PMD_ANIM_ATTACK, PMD_ANIM_IDLE},
    [PMD_ANIM_DOUBLE]      = {PMD_ANIM_ATTACK, PMD_ANIM_IDLE},
    [PMD_ANIM_QUICKSTRIKE] = {PMD_ANIM_STRIKE, PMD_ANIM_ATTACK, PMD_ANIM_IDLE},
};

static u16 Rd16(const u8 *p)
{
    return p[0] | (p[1] << 8);
}

static u32 Rd32(const u8 *p)
{
    return p[0] | (p[1] << 8) | (p[2] << 16) | ((u32)p[3] << 24);
}

static const struct MpPmdIndex *FindIndex(u16 dex, u8 variant)
{
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sMpPmdIndex); i++)
        if (sMpPmdIndex[i].dex == dex && sMpPmdIndex[i].variant == variant)
            return &sMpPmdIndex[i];
    return NULL;
}

bool8 MP_Pmd_Has(u16 dex)
{
    return FindIndex(dex, 0) != NULL;
}

static struct MpPmdChunk *Load(const struct MpPmdIndex *ix)
{
    struct MpPmdChunk *c, *freeSlot = NULL;
    int len = 0, i;
    u32 palBytes;

    for (i = 0; i < CACHE_MAX; i++)
    {
        if (sCache[i].index == ix && sCache[i].raw)
            return &sCache[i];
        if (!sCache[i].raw && !freeSlot)
            freeSlot = &sCache[i];
    }
    if (!freeSlot)   // full: the sheet unshown the longest goes now
    {
        for (i = 0; i < CACHE_MAX; i++)
            if (sCache[i].refs == 0 && (!freeSlot || sCache[i].idle > freeSlot->idle))
                freeSlot = &sCache[i];
        if (!freeSlot)
            return NULL;
        free(freeSlot->raw);
        memset(freeSlot, 0, sizeof(*freeSlot));
    }
    c = freeSlot;
    c->raw = (u8 *)stbi_zlib_decode_noheader_malloc((const char *)gMpPmdData + ix->offset, ix->size, &len);
    if (!c->raw || (u32)len != ix->raw || memcmp(c->raw, "PMD2", 4) != 0)
    {
        MPNet_Log("pmd: sheet %d/%d didn't inflate", ix->dex, ix->variant);
        free(c->raw);
        c->raw = NULL;
        return NULL;
    }
    c->index = ix;
    c->bpp = c->raw[4];
    c->colors = c->raw[5];
    c->animCount = c->raw[7] < PMD_ANIM_COUNT ? c->raw[7] : PMD_ANIM_COUNT;
    c->pal = c->raw + 8;
    c->shiny = c->pal + 2 * c->colors;
    palBytes = 4 * c->colors;
    c->anims = c->raw + 8 + palBytes + ((4 - palBytes % 4) % 4);
    c->refs = 0;
    c->idle = 0;
    return c;
}

static const u8 *AnimRecord(const struct MpPmdChunk *c, u8 anim)
{
    u32 off;
    if (!c || anim >= c->animCount)
        return NULL;
    off = Rd32(c->anims + 4 * anim);
    return off ? c->raw + off : NULL;
}

static u8 Resolve(const struct MpPmdChunk *c, u8 anim)
{
    u8 i;
    if (AnimRecord(c, anim) || anim >= PMD_ANIM_COUNT)
        return anim < PMD_ANIM_COUNT ? anim : PMD_ANIM_WALK;
    for (i = 0; i < 4; i++)
    {
        u8 alt = sFallbacks[anim][i];
        if (i > 0 && alt == PMD_ANIM_WALK)   // the rest of the list is empty (WALK is the last resort anyway)
            break;
        if (AnimRecord(c, alt))
            return alt;
    }
    return PMD_ANIM_WALK;
}

bool8 MP_Pmd_HasAnim(const struct MpPmdView *v, u8 anim)
{
    return AnimRecord(v->chunk, anim) != NULL;
}

// The frame's record for the view's animation, direction and frame
static const u8 *FrameRecord(const struct MpPmdView *v)
{
    const u8 *a = AnimRecord(v->chunk, v->shown);
    u8 frames, dirs, d, f;
    if (!a)
        return NULL;
    frames = a[0];
    dirs = a[1];
    d = dirs >= 8 ? v->dir & 7 : 0;
    f = v->frame < frames ? v->frame : frames - 1;
    return v->chunk->raw + Rd32(a + 4 + ((frames + 3) & ~3) + 4 * (d * frames + f));
}

static void Colors(const struct MpPmdChunk *c, bool8 shiny, u16 *out, u16 count)
{
    const u8 *src = shiny ? c->shiny : c->pal;
    u16 i;
    for (i = 0; i < count; i++)
        out[i] = i < c->colors ? Rd16(src + 2 * i) : 0;
}

bool8 MP_Pmd_Attach(struct MpPmdView *v, struct Sprite *sprite, u16 dex, bool8 female, bool8 shiny, s16 hue, u16 *colors16)
{
    const struct MpPmdIndex *ix = NULL;
    struct MpPmdChunk *c;
    bool8 ownShinySheet = FALSE;
    u16 all[256];
    u16 i;

    if (shiny)   // a shiny sheet of its own (its colors didn't fit beside the normal ones)
    {
        ix = FindIndex(dex, female ? 3 : 2);
        if (!ix && female && !FindIndex(dex, 1))
            ix = FindIndex(dex, 2);
        ownShinySheet = ix != NULL;
    }
    if (!ix && female)
        ix = FindIndex(dex, 1);
    if (!ix)
        ix = FindIndex(dex, 0);
    if (!ix || !(c = Load(ix)))
        return FALSE;
    if (v->chunk)
        MP_Pmd_Detach(v, sprite);
    memset(v, 0, sizeof(*v));
    v->chunk = c;
    c->refs++;
    v->shiny = shiny;
    v->anim = v->shown = PMD_ANIM_IDLE;
    v->shown = Resolve(c, PMD_ANIM_IDLE);
    v->loop = TRUE;
    v->big.bpp = c->bpp;
    v->big.colorCount = c->colors;
    Colors(c, shiny && !ownShinySheet, all, 256);
    MP_Hue_ShiftPalette(all, 0, hue);   // colors 1-15 (the palette slot)
    for (i = 16; i < c->colors; i += 15)   // 8 bpp: the rest, 15 at a time
    {
        u16 part[16], j, n = c->colors - i < 15 ? c->colors - i : 15;
        part[0] = 0;
        for (j = 0; j < n; j++)
            part[1 + j] = all[i + j];
        MP_Hue_ShiftPalette(part, 0, hue);
        for (j = 0; j < n; j++)
            all[i + j] = part[1 + j];
    }
    for (i = 0; i < 16; i++)
        colors16[i] = all[i];
    for (i = 16; i < 256; i++)
        v->big.extraShown[i - 16] = all[i];
    memcpy(v->extraOwn, v->big.extraShown, sizeof(v->extraOwn));
    v->big.frame = FrameRecord(v);
    if (sprite)
    {
        sprite->mpBig = &v->big;
        sprite->centerToCornerVecX = sprite->centerToCornerVecY = 0;
    }
    return TRUE;
}

void MP_Pmd_Detach(struct MpPmdView *v, struct Sprite *sprite)
{
    if (sprite && sprite->mpBig == &v->big)
        sprite->mpBig = NULL;
    if (v->chunk && v->chunk->refs > 0)
    {
        v->chunk->refs--;
        v->chunk->idle = 0;
    }
    v->chunk = NULL;
    v->big.frame = NULL;
}

void MP_Pmd_SetAnim(struct MpPmdView *v, u8 anim, u8 dir, bool8 loop)
{
    if (!v->chunk)
        return;
    v->dir = dir & 7;
    if (anim != v->anim || loop != v->loop)
    {
        v->anim = anim;
        v->shown = Resolve(v->chunk, anim);
        v->loop = loop;
        v->frame = 0;
        v->timer = 0;
        v->done = FALSE;
    }
    v->big.frame = FrameRecord(v);
}

void MP_Pmd_Tick(struct MpPmdView *v, struct Sprite *sprite)
{
    const u8 *a;
    u8 frames;

    if (!v->chunk || !(a = AnimRecord(v->chunk, v->shown)))
        return;
    frames = a[0];
    if (!v->done && ++v->timer >= a[4 + v->frame])
    {
        v->timer = 0;
        if (v->frame + 1 < frames)
            v->frame++;
        else if (v->loop)
            v->frame = 0;
        else
            v->done = TRUE;
    }
    v->big.frame = FrameRecord(v);
    // 8 bpp: colors 16+ fade with the palette slot (the slot holds colors 0-15)
    if (v->big.bpp == 8 && sprite)
    {
        struct MpFadeFit fit;
        u16 slot = OBJ_PLTT_ID(sprite->oam.paletteNum), i;
        MP_FadeFit_FromColors(&fit, &gPlttBufferUnfaded[slot], &gPlttBufferFaded[slot]);
        for (i = 0; i + 16 < v->big.colorCount; i++)
            v->big.extraShown[i] = MP_FadeFit_Apply(&fit, v->extraOwn[i]);
    }
}

bool8 MP_Pmd_Done(const struct MpPmdView *v)
{
    return v->done;
}

u16 MP_Pmd_Height(const struct MpPmdView *v)
{
    const u8 *f = v->big.frame;
    s16 dy;
    if (!f)
        return 0;
    dy = (s16)Rd16(f + 2);
    return dy < 0 ? -dy : 0;
}

u8 MP_Pmd_DirFrom4(u8 fdir)
{
    static const u8 sMap[4] = {PMD_DIR_DOWN, PMD_DIR_UP, PMD_DIR_LEFT, PMD_DIR_RIGHT};
    return fdir < 4 ? sMap[fdir] : PMD_DIR_DOWN;
}

u8 MP_Pmd_DirToward(s16 dx, s16 dy, u8 fallback)
{
    // eight 45-degree sectors; a step that's mostly one way faces that way
    s32 ax = dx < 0 ? -dx : dx, ay = dy < 0 ? -dy : dy;
    if (dx == 0 && dy == 0)
        return fallback;
    if (ax * 5 < ay * 2)   // within about 22 degrees of vertical
        return dy > 0 ? PMD_DIR_DOWN : PMD_DIR_UP;
    if (ay * 5 < ax * 2)
        return dx > 0 ? PMD_DIR_RIGHT : PMD_DIR_LEFT;
    if (dx > 0)
        return dy > 0 ? PMD_DIR_DOWN_RIGHT : PMD_DIR_UP_RIGHT;
    return dy > 0 ? PMD_DIR_DOWN_LEFT : PMD_DIR_UP_LEFT;
}

void MP_Pmd_FrameUpdate(void)
{
    int i;
    for (i = 0; i < CACHE_MAX; i++)
    {
        struct MpPmdChunk *c = &sCache[i];
        if (!c->raw || c->refs > 0)
            continue;
        if (++c->idle >= FREE_AFTER)
        {
            free(c->raw);
            memset(c, 0, sizeof(*c));
        }
    }
}

// What it's doing as an animation, and its facing turning 45 degrees at a time toward the way it wants to face (a
// turn shows the diagonals between)
#define TURN_FRAMES 3
void MP_Pmd_Animate(struct MpPmdView *v, u8 *pdir, u8 *pturn, bool8 walking, u8 face4, u8 pose, bool8 fast)
{
    u8 anim = PMD_ANIM_IDLE, want = MP_Pmd_DirFrom4(face4), turnFrames = TURN_FRAMES;

    u16 dex = v->chunk ? v->chunk->index->dex : 0;   // picks between a pose's animations (each Pokémon its own)

    if (walking || pose != LIFE_POSE_NONE)
        v->idle = 0;
    if (walking)
        anim = PMD_ANIM_WALK;
    else switch (pose)
    {
    case LIFE_POSE_NONE:   // standing: now and then a deep breath, and after a long while it sits down
        if (++v->idle >= 60 * 40)
            anim = PMD_ANIM_SIT;
        else if (v->idle % (60 * 12) >= 60 * 12 - 90)
            anim = PMD_ANIM_DEEPBREATH;
        break;
    case LIFE_POSE_SIT:    anim = PMD_ANIM_SIT; break;
    case LIFE_POSE_NAP:    anim = PMD_ANIM_SLEEP; want = PMD_DIR_DOWN; break;
    case LIFE_POSE_HOP:    anim = PMD_ANIM_HOP; break;
    case LIFE_POSE_SHAKE:  anim = PMD_ANIM_EAT; break;   // rummaging, digging, sniffing about
    case LIFE_POSE_LOOKUP: anim = PMD_ANIM_LOOKUP; break;
    case LIFE_POSE_SPIN:   // looking all around: keeps turning
        want = (*pdir + 1) & 7;
        turnFrames = 8;
        break;
    case LIFE_POSE_EAT:    anim = PMD_ANIM_EAT; break;
    case LIFE_POSE_SPAR:
    {
        static const u8 sSpar[3] = {PMD_ANIM_ATTACK, PMD_ANIM_SHOOT, PMD_ANIM_STRIKE};
        anim = sSpar[dex % 3];
        break;
    }
    case LIFE_POSE_NOD:    anim = PMD_ANIM_NOD; break;
    case LIFE_POSE_JOY:    anim = PMD_ANIM_POSE; break;
    case LIFE_POSE_TWIRL:  anim = PMD_ANIM_ROTATE; break;
    case LIFE_POSE_TRIP:   anim = (dex & 1) ? PMD_ANIM_TUMBLE : PMD_ANIM_TRIP; break;
    case LIFE_POSE_HURT:   anim = (dex & 1) ? PMD_ANIM_CRINGE : PMD_ANIM_HURT; break;
    case LIFE_POSE_CHARGE: anim = (dex & 1) ? PMD_ANIM_SHOCK : PMD_ANIM_CHARGE; break;
    case LIFE_POSE_FLOAT:  anim = PMD_ANIM_FLOAT; break;
    case LIFE_POSE_LEAP:   anim = PMD_ANIM_LEAPFORTH; break;
    case LIFE_POSE_PULL:   anim = PMD_ANIM_PULL; break;
    case LIFE_POSE_BREATH: anim = PMD_ANIM_DEEPBREATH; break;
    case LIFE_POSE_LAY:    anim = PMD_ANIM_LAYING; break;
    }
    if (*pdir != want)
    {
        if (*pturn > 0)
            (*pturn)--;
        else
        {
            u8 cw = (want - *pdir) & 7;   // steps clockwise (down -> down-right -> right ...)
            *pdir = (*pdir + (cw <= 4 ? 1 : 7)) & 7;
            *pturn = turnFrames;
        }
    }
    else
    {
        *pturn = 0;
    }
    MP_Pmd_SetAnim(v, anim, *pdir, TRUE);
    if (fast && walking)   // running or biking: the steps go by twice as fast
        MP_Pmd_Tick(v, NULL);
}
