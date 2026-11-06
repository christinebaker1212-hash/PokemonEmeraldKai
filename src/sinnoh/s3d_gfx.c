// Moemon Kai: SINNOH's people as Platinum draws them (sinnoh/platinum_gfx.pak, tools/sinnoh/build_gfx.py): overworld
// sprites (event_object_movement.c builds their graphics info) and trainers' battle pictures (drawn whole, 80x80, by
// the big-sprite renderer over the stand-in's place). Without the file everything keeps the game's own stand-ins.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "sinnoh_tower.h"
#include "palette.h"
#include "sprite.h"
#include "s3d.h"
#include "mp_bigsprite.h"
#include "constants/sinnoh_gfx.h"
#include "constants/trainers.h"
#include "constants/battle.h"
#include "battle.h"
#include "data.h"
#include "mp.h"
#include "battle_anim.h"
#include "battle_setup.h"

const char *Updater_ExeFile(const char *name);

#define SINNOH_BACK_SIZE (2 + 32 + 4 * 2048)
struct SinnohSprite { u8 kind; const u16 *pal[2]; const u8 *frames[2]; };   // ([1]: MIXED's smaller copy)
struct SinnohPic { const u16 *pal; struct MpBigSprite big; };

static struct
{
    bool8 tried, ok;
    u8 *data;
    struct SinnohSprite sprites[SINNOH_SPRITE_COUNT];
    struct SinnohPic pics[SINNOH_PIC_COUNT];
    const u16 *town[2];    // the TOWN MAP's pages (tools/sinnoh/gen_townmap.py picture), RGB555
    u16 townW, townH;
    struct S3dStarterGfx starter;   // the briefcase (tools/sinnoh/build_starter.py: "STRT"; frames NULL: none)
    const u8 *backs;       // the partners' backs (build_gfx.py backs(): "BACK"), each u16 picture, palette, 4 frames
    u16 backCount;
    const u8 *accessories[100];   // SUPER CONTESTS' art (tools/sinnoh/build_contest.py: "CNTS"): u16 w, h, RGB555
    const u8 *backdrops2[18];
    const u8 *stages[8];    // Platinum's contest stages (build_contest.py stages(): "CSTG"): u16 w, h, RGB555
    const u8 *mining[128];  // the Underground's digging (tools/sinnoh/build_mining.py: "MINE"): u16 w, h, RGB555
    const u8 *unownWidths;  // Platinum's UNOWN font (tools/sinnoh/build_unown.py: "UNWN"): widths, glyphs
    const u16 *unownGlyphs;
    u16 unownCount;
    const u8 *actions[2][3];   // DAWN's and LUCAS's action sheets (build_gfx.py player_actions(): "PACT"): u8 frames,
                               // pad, 32x32 frames
    const u8 *actionPal[2];    // ... and each one's palette
    const u8 *journal;     // the JOURNAL's notebook (tools/sinnoh/build_journal.py: "JRNL"): u16 w, h, then the boy's and
                           // the girl's w*h color numbers and 256 colors
    const u8 *poketch;     // the POKéTCH's art (tools/sinnoh/build_poketch.py: "PKCH")
    const u8 *poketchEnd;
} sGfx;

static void Load(void)
{
    const char *path = getenv("MOEMON_S3D_GFX");
    FILE *f;
    long size;
    u8 *p, *end;
    u32 i, n;
    sGfx.tried = TRUE;
    if (path == NULL)
        path = Updater_ExeFile("sinnoh/platinum_gfx.pak");
    f = fopen(path, "rb");
    if (f == NULL)
        return;
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);
    sGfx.data = malloc(size);
    if (sGfx.data == NULL || fread(sGfx.data, 1, size, f) != (size_t)size || size < 10
     || memcmp(sGfx.data, "S3DG", 4) != 0 || sGfx.data[4] != 2)
    {
        fclose(f);
        free(sGfx.data);
        sGfx.data = NULL;
        printf("[s3d] %s isn't a SINNOH graphics pack\n", path);
        return;
    }
    fclose(f);
    p = sGfx.data + 8;
    end = sGfx.data + size;
    n = p[0] | p[1] << 8;
    p += 2;
    if (n != SINNOH_SPRITE_COUNT)
        goto bad;   // (a pack from another commit: names and numbers wouldn't match)
    for (i = 0; i < n; i++)
    {
        u8 kind;
        u32 frames;
        if (p + 26 + 32 > end)
            goto bad;
        kind = p[24];
        frames = kind == 0 ? 9 * 512 : kind == 3 ? 10 * 512 : kind == 1 ? 512 : kind == 4 ? 2048 : 128;
        if (p + 26 + 2 * (32 + frames) > end)
            goto bad;
        sGfx.sprites[i].kind = kind;
        sGfx.sprites[i].pal[0] = (const u16 *)(p + 26);
        sGfx.sprites[i].frames[0] = p + 26 + 32;
        sGfx.sprites[i].pal[1] = (const u16 *)(p + 26 + 32 + frames);
        sGfx.sprites[i].frames[1] = p + 26 + 32 + frames + 32;
        p += 26 + 2 * (32 + frames);
    }
    if (p + 2 > end)
        goto bad;
    n = p[0] | p[1] << 8;
    p += 2;
    if (n != SINNOH_PIC_COUNT)
        goto bad;
    for (i = 0; i < n; i++)
    {
        if (p + 24 + 32 + 8 + 80 * 40 > end)
            goto bad;
        sGfx.pics[i].pal = (const u16 *)(p + 24);
        sGfx.pics[i].big.frame = p + 24 + 32;
        sGfx.pics[i].big.bpp = 4;
        sGfx.pics[i].big.colorCount = 16;
        p += 24 + 32 + 8 + 80 * 40;
    }
    // the partners' backs: "BACK", count, each u16 picture, u16 palette[16], four 64x64 frames of tiles
    if (p + 6 <= end && memcmp(p, "BACK", 4) == 0)
    {
        n = p[4] | p[5] << 8;
        if (p + 6 + n * SINNOH_BACK_SIZE <= end)
        {
            sGfx.backs = p + 6;
            sGfx.backCount = n;
            p += 6 + n * SINNOH_BACK_SIZE;
        }
    }
    // then the TOWN MAP: "TOWN", width, height, pages (u16 each), the pages' RGB555 pixels (a pack without it: none)
    if (p + 10 <= end && memcmp(p, "TOWN", 4) == 0)
    {
        u32 w = p[4] | p[5] << 8, h = p[6] | p[7] << 8, pages = p[8] | p[9] << 8;
        if (pages >= 2 && p + 10 + pages * w * h * 2 <= end)
        {
            sGfx.townW = w;
            sGfx.townH = h;
            sGfx.town[0] = (const u16 *)(p + 10);
            sGfx.town[1] = (const u16 *)(p + 10 + w * h * 2);
            p += 10 + pages * w * h * 2;
        }
    }
    if (p + 8 <= end && memcmp(p, "STRT", 4) == 0)
    {
        u32 size = p[4] | p[5] << 8 | p[6] << 16 | (u32)p[7] << 24;
        const u8 *q = p + 8, *qend = p + 8 + size;
        struct S3dStarterGfx *g = &sGfx.starter;
        if (qend <= end && q + 8 <= qend)
        {
            u32 k, n;
            g->w = q[0] | q[1] << 8;
            g->h = q[2] | q[3] << 8;
            g->openFrames = q[4] | q[5] << 8;
            g->camFrames = q[6] | q[7] << 8;
            q += 8;
            n = (u32)g->w * g->h * 2 * (g->openFrames + g->camFrames);
            if (q + n + 2 <= qend)
            {
                g->frames = (const u16 *)q;
                q += n;
                g->bounceFrames = q[0] | q[1] << 8;
                q += 2;
                for (k = 0; k < 3 && q + 8 <= qend; k++)
                {
                    g->ball[k].x = q[0] | q[1] << 8;
                    g->ball[k].y = q[2] | q[3] << 8;
                    g->ball[k].w = q[4] | q[5] << 8;
                    g->ball[k].h = q[6] | q[7] << 8;
                    g->ball[k].px = (const u16 *)(q + 8);
                    q += 8 + (u32)g->ball[k].w * g->ball[k].h * 2 * g->bounceFrames;
                }
                if (k == 3 && q + 4 + 2048 + 12 <= qend)
                {
                    g->cursor = (const u16 *)(q + 4);
                    q += 4 + 2048;
                    for (k = 0; k < 3; k++)
                    {
                        g->cursorPos[k][0] = (s16)(q[0] | q[1] << 8);
                        g->cursorPos[k][1] = (s16)(q[2] | q[3] << 8);
                        q += 4;
                    }
                }
                else
                    g->frames = NULL;   // (cut short: none)
            }
        }
        p = qend <= end ? qend : end;
    }
    // SUPER CONTESTS' art: "CNTS", u16 accessories (u16 w, h, RGB555), u16 backdrops (the same)
    if (p + 6 <= end && memcmp(p, "CNTS", 4) == 0)
    {
        u32 k, list;
        p += 4;
        for (list = 0; list < 2 && p + 2 <= end; list++)
        {
            n = p[0] | p[1] << 8;
            p += 2;
            for (k = 0; k < n && p + 4 <= end; k++)
            {
                u32 w = p[0] | p[1] << 8, h = p[2] | p[3] << 8;
                if (p + 4 + w * h * 2 > end)
                    break;
                if (list == 0 && k < ARRAY_COUNT(sGfx.accessories))
                    sGfx.accessories[k] = p;
                if (list == 1 && k < ARRAY_COUNT(sGfx.backdrops2))
                    sGfx.backdrops2[k] = p;
                p += 4 + w * h * 2;
            }
        }
    }
    // the contests' stages: "CSTG", u16 count, pictures (u16 w, h, RGB555)
    if (p + 6 <= end && memcmp(p, "CSTG", 4) == 0)
    {
        u32 k;
        n = p[4] | p[5] << 8;
        p += 6;
        for (k = 0; k < n && p + 4 <= end; k++)
        {
            u32 w = p[0] | p[1] << 8, h = p[2] | p[3] << 8;
            if (p + 4 + w * h * 2 > end)
                break;
            if (k < ARRAY_COUNT(sGfx.stages))
                sGfx.stages[k] = p;
            p += 4 + w * h * 2;
        }
    }
    // the Underground's digging screen: "MINE", u16 count, pictures (u16 w, h, RGB555)
    if (p + 6 <= end && memcmp(p, "MINE", 4) == 0)
    {
        u32 k;
        n = p[4] | p[5] << 8;
        p += 6;
        for (k = 0; k < n && p + 4 <= end; k++)
        {
            u32 w = p[0] | p[1] << 8, h = p[2] | p[3] << 8;
            if (p + 4 + w * h * 2 > end)
                break;
            if (k < ARRAY_COUNT(sGfx.mining))
                sGfx.mining[k] = p;
            p += 4 + w * h * 2;
        }
    }
    // Platinum's UNOWN font: "UNWN", u16 count, u8 widths[count], count x 32 u16 glyphs
    if (p + 6 <= end && memcmp(p, "UNWN", 4) == 0)
    {
        n = p[4] | p[5] << 8;
        if (p + 6 + n + n * 64 <= end)
        {
            sGfx.unownCount = n;
            sGfx.unownWidths = p + 6;
            sGfx.unownGlyphs = (const u16 *)(p + 6 + n);
            p += 6 + n + n * 64;
        }
    }
    // DAWN's and LUCAS's action sheets: "PACT", u8 players, u8 sheets, per player u16 palette[16] then per sheet u8
    // frames, u8 pad, the 32x32 frames
    if (p + 6 <= end && memcmp(p, "PACT", 4) == 0)
    {
        u32 who, sheet, players = p[4], sheets = p[5];
        p += 6;
        for (who = 0; who < players && p + 32 <= end; who++)
        {
            if (who < 2)
                sGfx.actionPal[who] = p;
            p += 32;
            for (sheet = 0; sheet < sheets && p + 2 <= end; sheet++)
            {
                u32 frames = p[0];
                if (p + 2 + frames * 512 > end)
                    break;
                if (who < 2 && sheet < 3 && frames)
                    sGfx.actions[who][sheet] = p;
                p += 2 + frames * 512;
            }
        }
    }
    // the JOURNAL's notebook: "JRNL", u16 w, h, then twice (boy, girl) w*h color numbers and 256 u16 colors
    if (p + 8 <= end && memcmp(p, "JRNL", 4) == 0)
    {
        u32 w = p[4] | p[5] << 8, h = p[6] | p[7] << 8;
        if (p + 8 + 2 * (w * h + 512) <= end)
        {
            sGfx.journal = p + 4;
            p += 8 + 2 * (w * h + 512);
        }
        else
            p = end;
    }
    if (p + 4 <= end && memcmp(p, "PKCH", 4) == 0)
    {
        sGfx.poketch = p + 4;
        sGfx.poketchEnd = end;
    }
    sGfx.ok = TRUE;
    return;
bad:
    printf("[s3d] %s: another graphics pack (rebuild it: tools/sinnoh/make_pack.sh)\n", path);
}

// Platinum's sprite INDEX (SMALL: MIXED's smaller copy)
bool8 S3D_Gfx_Sprite(u16 index, bool8 small, u8 *kind, const u16 **pal, const u8 **frames)
{
    if (!sGfx.tried)
        Load();
    if (!sGfx.ok || index >= SINNOH_SPRITE_COUNT)
        return FALSE;
    *kind = sGfx.sprites[index].kind;
    *pal = sGfx.sprites[index].pal[small != 0];
    *frames = sGfx.sprites[index].frames[small != 0];
    return TRUE;
}

#include "../data/sinnoh_trainer_pics.h"

// A SINNOH trainer's own battle picture on SPRITE (made with the stand-in's 64x64 picture STANDIN and its palette, at
// the stand-in's height): Platinum's 80x80 picture drawn whole in its place, feet where the game's pictures have
// theirs, its colors in the sprite's palette slot. Nothing changes for any other trainer or without the pack.
void S3D_Gfx_TrainerPic(struct Sprite *sprite, u16 trainerId, u16 standIn)
{
    u8 pic;
    if (IS_SINNOH_FRONTIER_TRAINER(trainerId))   // (Platinum's frontier trainers: s3d_tower.c)
    {
        if (Sinnoh_Frontier_Pic(trainerId) != standIn)
            return;
        pic = Sinnoh_Frontier_SinnohPic(trainerId);
    }
    else if (trainerId < FIRST_SINNOH_TRAINER || trainerId >= FIRST_SINNOH_TRAINER + SINNOH_TRAINERS_COUNT
          || gTrainers[trainerId].trainerPic != standIn)
        return;
    else
        pic = sSinnohTrainerPics[trainerId - FIRST_SINNOH_TRAINER];
    if (!sGfx.tried)
        Load();
    if (!sGfx.ok || pic >= SINNOH_PIC_COUNT)
        return;
    LoadPalette(sGfx.pics[pic].pal, OBJ_PLTT_ID(sprite->oam.paletteNum), PLTT_SIZE_4BPP);
    sprite->y -= (8 - gTrainerFrontPicCoords[standIn].size) * 4;   // (the stand-in's own height undone)
    sprite->mpBig = &sGfx.pics[pic].big;
    printf("[test] sinnoh battle picture: trainer %d picture %d\n", trainerId, pic);
}

// A SINNOH trainer's Platinum picture for other screens (the card games' challenge screen): 80x80, rows of 40 bytes
// (4 bits a pixel, the low nibble first), and its 16 colors; FALSE: none (another trainer, or no pack)
bool8 S3D_Gfx_TrainerPicPixels(u16 trainerId, const u8 **pixels, const u16 **pal)
{
    u8 pic;
    if (trainerId < FIRST_SINNOH_TRAINER || trainerId >= FIRST_SINNOH_TRAINER + SINNOH_TRAINERS_COUNT)
        return FALSE;
    pic = sSinnohTrainerPics[trainerId - FIRST_SINNOH_TRAINER];
    if (!sGfx.tried)
        Load();
    if (!sGfx.ok || pic >= SINNOH_PIC_COUNT)
        return FALSE;
    *pixels = sGfx.pics[pic].big.frame + 8;
    *pal = sGfx.pics[pic].pal;
    return TRUE;
}

// A SINNOH partner's back (a tag or partner battle beside them: battle_tower.c SetQzPartner): Platinum's, four 64x64
// frames as the game's trainer back pictures are laid out, and its 16 colors; FALSE: none (no back of its own, or no
// pack). DAWN's and LUCAS's are Kai's LOOKs' (*look: "kai_dawn" / "kai_lucas", tiles NULL)
bool8 S3D_Gfx_PartnerBack(u16 trainerId, const u8 **tiles, const u16 **pal, const char **look)
{
    u8 pic;
    u32 i;
    *look = NULL;
    *tiles = NULL;
    if (trainerId < FIRST_SINNOH_TRAINER || trainerId >= FIRST_SINNOH_TRAINER + SINNOH_TRAINERS_COUNT)
        return FALSE;
    pic = sSinnohTrainerPics[trainerId - FIRST_SINNOH_TRAINER];
    if (pic == SINNOH_PIC_PLAYER_FEMALE || pic == SINNOH_PIC_DP_PLAYER_FEMALE || pic == SINNOH_PIC_DP_PLAYER_FEMALE_2)
        *look = "kai_dawn";
    else if (pic == SINNOH_PIC_PLAYER_MALE || pic == SINNOH_PIC_DP_PLAYER_MALE || pic == SINNOH_PIC_DP_PLAYER_MALE_2)
        *look = "kai_lucas";
    if (*look)
        return TRUE;
    if (!sGfx.tried)
        Load();
    if (!sGfx.ok)
        return FALSE;
    for (i = 0; i < sGfx.backCount; i++)
    {
        const u8 *b = sGfx.backs + i * SINNOH_BACK_SIZE;
        if ((b[0] | b[1] << 8) == pic)
        {
            *pal = (const u16 *)(b + 2);
            *tiles = b + 2 + 32;
            return TRUE;
        }
    }
    return FALSE;
}

// The battle's opponent trainer picture for BATTLER (battle controllers): the trainer it belongs to
void S3D_Gfx_BattleTrainerPic(struct Sprite *sprite, u8 battler, u16 standIn)
{
    bool8 second = GetBattlerPosition(battler) == B_POSITION_OPPONENT_RIGHT
                && ((gBattleTypeFlags & BATTLE_TYPE_TWO_OPPONENTS) || (gMpCoopBattle && MP_Coop_TwoTrainers()));
    if (!(gBattleTypeFlags & BATTLE_TYPE_TRAINER) || ((gBattleTypeFlags & BATTLE_TYPE_LINK) && !gMpCoopBattle))
        return;
    S3D_Gfx_TrainerPic(sprite, second ? gTrainerBattleOpponent_B : gTrainerBattleOpponent_A, standIn);
}

// The TOWN MAP's page PAGE (0 NORTH, 1 SOUTH): W x H RGB555 pixels; FALSE: none (no pack, or one without it)
bool8 S3D_Gfx_TownMap(u8 page, const u16 **pixels, u16 *w, u16 *h)
{
    if (!sGfx.tried)
        Load();
    if (!sGfx.ok || page > 1 || sGfx.town[page] == NULL)
        return FALSE;
    *pixels = sGfx.town[page];
    *w = sGfx.townW;
    *h = sGfx.townH;
    return TRUE;
}

// The POKéTCH's app screen NAME: 192x160 shade bytes (0 lightest .. 3 darkest); NULL: none
const u8 *S3D_Gfx_PoketchScreen(const char *name)
{
    const u8 *p;
    u32 i, n;
    if (!sGfx.tried)
        Load();
    if (!sGfx.ok || (p = sGfx.poketch) == NULL)
        return NULL;
    n = p[0] | p[1] << 8;
    p += 2;
    for (i = 0; i < n && p + 16 + 192 * 160 <= sGfx.poketchEnd; i++, p += 16 + 192 * 160)
        if (strncmp((const char *)p, name, 16) == 0)
            return p + 16;
    return NULL;
}

// The POKéTCH's sprite sheet SHEET: its first cell's bytes (and the cell count); NULL: none
static const u8 *PoketchSheet(const char *sheet, u32 *cellCount)
{
    const u8 *p;
    u32 i, n, k, cells, seqs;
    if (!sGfx.tried)
        Load();
    if (!sGfx.ok || (p = sGfx.poketch) == NULL)
        return NULL;
    n = p[0] | p[1] << 8;
    p += 2 + n * (16 + 192 * 160);
    if (p + 2 > sGfx.poketchEnd)
        return NULL;
    n = p[0] | p[1] << 8;
    p += 2;
    for (i = 0; i < n && p + 18 <= sGfx.poketchEnd; i++)
    {
        bool8 want = strncmp((const char *)p, sheet, 16) == 0;
        cells = p[16] | p[17] << 8;
        p += 18;
        if (want)
        {
            *cellCount = cells;
            return p;
        }
        for (k = 0; k < cells && p + 8 <= sGfx.poketchEnd; k++)
            p += 8 + (p[4] | p[5] << 8) * (p[6] | p[7] << 8);
        seqs = p[0] | p[1] << 8;
        p += 2;
        for (k = 0; k < seqs && p + 3 <= sGfx.poketchEnd; k++)
            p += 3 + p[0] * 2;
    }
    return NULL;
}

// Cell CELL of the POKéTCH's sprite sheet SHEET: its top-left from the sprite's point, size and shade bytes (255 clear)
bool8 S3D_Gfx_PoketchCell(const char *sheet, u16 cell, s16 *x, s16 *y, u16 *w, u16 *h, const u8 **px)
{
    u32 cells, k;
    const u8 *p = PoketchSheet(sheet, &cells);
    if (p == NULL || cell >= cells)
        return FALSE;
    for (k = 0; k < cell; k++)
        p += 8 + (p[4] | p[5] << 8) * (p[6] | p[7] << 8);
    if (p + 8 > sGfx.poketchEnd)
        return FALSE;
    *x = (s16)(p[0] | p[1] << 8);
    *y = (s16)(p[2] | p[3] << 8);
    *w = p[4] | p[5] << 8;
    *h = p[6] | p[7] << 8;
    *px = p + 8;
    return p + 8 + *w * *h <= sGfx.poketchEnd;
}

// Animation SEQ of SHEET at frame T (Platinum's playback: 1 once, holding the last frame; 2 looping from its loop
// start; 3 there and back once; 4 there and back): the cell it shows; 0xFFFF none
u16 S3D_Gfx_PoketchAnimCell(const char *sheet, u16 seq, u32 t)
{
    u32 cells, k, total = 0, nf, loop, mode, i;
    const u8 *p = PoketchSheet(sheet, &cells), *f;
    if (p == NULL)
        return 0xFFFF;
    for (k = 0; k < cells; k++)
        p += 8 + (p[4] | p[5] << 8) * (p[6] | p[7] << 8);
    if (seq >= (u32)(p[0] | p[1] << 8))
        return seq < cells ? seq : 0xFFFF;   // (a sheet without animations: the cell of that number)
    p += 2;
    for (k = 0; k < seq; k++)
        p += 3 + p[0] * 2;
    nf = p[0], loop = p[1], mode = p[2], f = p + 3;
    if (nf == 0)
        return 0xFFFF;
    for (i = 0; i < nf; i++)
        total += f[i * 2 + 1] ? f[i * 2 + 1] : 1;
    if (mode == 3 || mode == 4)   // (there and back: the frames forward then backward)
    {
        u32 round = total * 2;
        t = mode == 3 && t >= round ? round - 1 : t % round;
        if (t >= total)
            t = round - 1 - t;
    }
    else if (mode == 2)
    {
        u32 head = 0;
        for (i = 0; i < loop && i < nf; i++)
            head += f[i * 2 + 1] ? f[i * 2 + 1] : 1;
        if (t >= total)
            t = head + (t - head) % (total - head ? total - head : 1);
    }
    else if (t >= total)
        t = total - 1;
    for (i = 0; i < nf; i++)
    {
        u32 d = f[i * 2 + 1] ? f[i * 2 + 1] : 1;
        if (t < d)
            return f[i * 2];
        t -= d;
    }
    return f[(nf - 1) * 2];
}

// The briefcase's starter screen (NULL: no pack, or one without it)
const struct S3dStarterGfx *S3D_Gfx_Starter(void)
{
    if (!sGfx.tried)
        Load();
    return sGfx.ok && sGfx.starter.frames != NULL ? &sGfx.starter : NULL;
}

// SUPER CONTESTS' art (s3d_contest.c): accessory N's / backdrop N's picture, W x H RGB555 (bit 15: drawn); FALSE: none
// Platinum's UNOWN font (src/text.c FONT_UNOWN): FALSE without the pack
bool8 S3D_Gfx_UnownFont(const u16 **glyphs, const u8 **widths, u16 *count)
{
    if (!sGfx.tried)
        Load();
    if (!sGfx.ok || sGfx.unownGlyphs == NULL)
        return FALSE;
    *glyphs = sGfx.unownGlyphs;
    *widths = sGfx.unownWidths;
    *count = sGfx.unownCount;
    return TRUE;
}

// the Underground's digging screen's picture N (s3d_mining.c; build_mining.py's order)
bool8 S3D_Gfx_MiningPicture(u16 n, u16 *w, u16 *h, const u16 **px)
{
    const u8 *p;
    if (!sGfx.tried)
        Load();
    if (!sGfx.ok || n >= ARRAY_COUNT(sGfx.mining) || (p = sGfx.mining[n]) == NULL || (p[0] | p[1] << 8) == 0)
        return FALSE;
    *w = p[0] | p[1] << 8;
    *h = p[2] | p[3] << 8;
    *px = (const u16 *)(p + 4);
    return TRUE;
}

// A contest stage (s3d_contest_ui.c STAGE_*): FALSE when the pack has none
bool8 S3D_Gfx_ContestStage(u16 n, u16 *w, u16 *h, const u16 **px)
{
    const u8 *p;
    if (!sGfx.tried)
        Load();
    if (!sGfx.ok || n >= ARRAY_COUNT(sGfx.stages) || (p = sGfx.stages[n]) == NULL)
        return FALSE;
    *w = p[0] | p[1] << 8;
    *h = p[2] | p[3] << 8;
    *px = (const u16 *)(p + 4);
    return TRUE;
}

bool8 S3D_Gfx_ContestPicture(bool8 backdrop, u16 n, u16 *w, u16 *h, const u16 **px)
{
    const u8 *p;
    if (!sGfx.tried)
        Load();
    if (!sGfx.ok)
        return FALSE;
    p = backdrop ? (n < ARRAY_COUNT(sGfx.backdrops2) ? sGfx.backdrops2[n] : NULL)
                 : (n < ARRAY_COUNT(sGfx.accessories) ? sGfx.accessories[n] : NULL);
    if (p == NULL || (p[0] | p[1] << 8) == 0)
        return FALSE;
    *w = p[0] | p[1] << 8;
    *h = p[2] | p[3] << 8;
    *px = (const u16 *)(p + 4);
    return TRUE;
}

// The JOURNAL's notebook (GIRL: the girl's): W x H color numbers and 16 palettes of 16 (FALSE: the pack has none)
bool8 S3D_Gfx_Journal(u8 girl, const u8 **px, const u16 **pal, u16 *w, u16 *h)
{
    const u8 *p;
    u32 n;
    if (!sGfx.tried)
        Load();
    if (!sGfx.ok || (p = sGfx.journal) == NULL)
        return FALSE;
    *w = p[0] | p[1] << 8;
    *h = p[2] | p[3] << 8;
    n = (u32)*w * *h;
    p += 4 + (girl ? n + 512 : 0);
    *px = p;
    *pal = (const u16 *)(p + n);
    return TRUE;
}

// DAWN's (WHO 0) or LUCAS's (1) Platinum action sheet (0 surfing, 1 a field move's POKé BALL, 2 fishing): frame FRAME's
// 32x32 tiles and the sheet's palette (FALSE: the pack has none)
bool8 S3D_Gfx_PlayerAction(u8 who, u8 sheet, u8 frame, const u8 **tiles, const u16 **pal)
{
    const u8 *a;
    if (!sGfx.tried)
        Load();
    if (!sGfx.ok || who >= 2 || sheet >= 3 || (a = sGfx.actions[who][sheet]) == NULL || frame >= a[0])
        return FALSE;
    if (tiles)
        *tiles = a + 2 + frame * 512;
    if (pal)
        *pal = (const u16 *)sGfx.actionPal[who];
    return TRUE;
}
