// Moemon Kai: OPTIONS > DISPLAY > GB OPTIONS > MOVES (RED / CRYSTAL): every move animated and heard as Pokémon Red or
// Crystal show it.
//
// tools/moemon_legacy/move_bake.py builds both games from their pret disassemblies, plays every move's animation
// from each side in an emulator and records each frame: the sprites (their tiles), the background palette and the
// sprites' (BGP / OBP0 / OBP1; Crystal's colours too), the scroll on every line and the window's place (shakes,
// waves), what became of the two pictures (as they were, gone, or cut into its 8x8 cells moved about, with tiles of
// the move's own such as SUBSTITUTE's doll) and the sounds started (each rendered once from the emulator).
// tools/moemon_legacy/move_pack.py packs it into graphics/moemon/legacy/gbmoves.bin (mp_gb_moves_data.c).
//
// KAI's own animation runs on underneath, unseen and unheard (its sounds are muted: sound.c), so the battle takes
// the same time as for a friend without the option and co-op never waits: the Game Boy's timeline is fitted to the
// length KAI's takes (sKaiMoveFrames, measured: tools note in mp_gb_moves_kailen.h). The picture is the battle as it
// was the frame before (sStill), the battlers taken out where the timeline moves or hides them (the backdrop under
// them drawn once without sprites: sPlate), and the Game Boy's frame drawn over it: the Game Boy's two pictures'
// places mapped onto the attacker's and the target's (so Doubles and Triples aim where KAI's would).
// A move the game doesn't have takes the nearest one it does (MP_GbMoves_StandIn).
#include "global.h"
#include "mp.h"
#include "battle.h"
#include "battle_anim.h"
#include "main.h"
#include "mp_uistyle.h"
#include "data.h"
#include "pokemon.h"
#include "constants/battle_anim.h"
#include "constants/moves.h"
#include "constants/rgb.h"
#include <string.h>

#include "mp_gb_moves_kailen.h"

extern const u8 gMpGbMovesData[];
extern u32 gMpAnimSteps;
void MPNet_Log(const char *fmt, ...);

#define W 240
#define H 160

struct GbGame {
    const u8 *base;
    u16 moves, tiles, pictiles, picmaps, lines, sounds, cgbpals;
    u8 pbox[4], ebox[4];
    const u8 *anims, *tileData, *picTileData, *picMaps, *lineData, *soundData, *cgbPalData;
};

struct GbSprite { u8 x, y, attr; u16 tile; };

struct GbFrame {
    u8 bgp, obp[2];
    s16 scx, scy;           // a value, or a line table (flags)
    u8 flags;
    s8 winDx;
    u8 winDy, winOn;
    u8 picKind[2];
    u16 picMap[2];
    u8 picBgp[2];           // each picture's own (Crystal's palettes 0 / 1; Red's: BGP)
    u16 cgbPal;
    u8 nSpr;
    struct GbSprite spr[40];
};

static struct {
    bool8 active;
    bool8 wantPlate, havePlate, haveStill;
    u8 game;                // 0 Red, 1 Crystal
    u8 turn;                // the Game Boy's side: 0 the player's
    u8 battler[2];          // KAI's user and target
    u16 gbLen, fit;
    u32 start;
    const u8 *cursor;       // the next frame's bytes
    s32 decoded;            // the frame in sF
    struct GbFrame f;
    // the map from the Game Boy's screen to ours (the pictures' centres)
    s32 gux, guy, kux, kuy;
    s32 sx256, sy256;
    // ... and the target's, and each side's picture's size over the Game Boy's (256ths: a sprite's place is mapped
    // from the side it's nearer, MapPoint)
    s32 gtx, gty, ktx, kty;
    s32 su256, st256;
    s32 boxX[2], boxY[2];   // the user's and the target's pictures' centres as the move started (KaiBox)
} sM;
static u16 sPrev[W * H], sStill[W * H], sPlate[W * H], sRaw[W * H];
static struct GbGame sGames[2];
static bool8 sLoaded;

static u16 R16(const u8 *p) { return p[0] | (p[1] << 8); }
static u32 R32(const u8 *p) { return p[0] | (p[1] << 8) | (p[2] << 16) | ((u32)p[3] << 24); }

static bool8 LoadPack(void)
{
    const u8 *d = gMpGbMovesData;
    u32 g;
    if (sLoaded)
        return sGames[0].base != NULL || sGames[1].base != NULL;
    sLoaded = TRUE;
    if (memcmp(d, "GBMV", 4) != 0)
        return FALSE;
    for (g = 0; g < 2; g++)
    {
        u32 off = R32(d + 8 + g * 4);
        struct GbGame *gm = &sGames[g];
        const u8 *b;
        if (off == 0)
            continue;
        b = d + off;
        gm->base = b;
        gm->moves = R16(b), gm->tiles = R16(b + 2), gm->pictiles = R16(b + 4), gm->picmaps = R16(b + 6);
        gm->lines = R16(b + 8), gm->sounds = R16(b + 10), gm->cgbpals = R16(b + 12);
        memcpy(gm->pbox, b + 14, 4);
        memcpy(gm->ebox, b + 18, 4);
        gm->anims = b + R32(b + 22), gm->tileData = b + R32(b + 26), gm->picTileData = b + R32(b + 30);
        gm->picMaps = b + R32(b + 34), gm->lineData = b + R32(b + 38), gm->soundData = b + R32(b + 42);
        gm->cgbPalData = b + R32(b + 46);
    }
    return sGames[0].base != NULL || sGames[1].base != NULL;
}

// GB OPTIONS > MOVES: RED / CRYSTAL showing, the Game Boy's (MP_GB_KAI_MOVES unset), and its game in the pack
static s8 GbGame(void)
{
    s8 g = MP_UiStyle_Active() == UISTYLE_RED ? 0 : MP_UiStyle_Active() == UISTYLE_CRYSTAL ? 1 : -1;
    if (g < 0 || MP_Gb_On(MP_GB_KAI_MOVES) || !LoadPack() || sGames[g].base == NULL)
        return -1;
    return g;
}

// The move's name in capitals (the stand-in's keywords)
static void MoveName(u16 move, char *out, u32 size)
{
    MP_GameToAscii(out, gMoveNames[move], size);
}

static bool8 Has(const char *name, const char *word)
{
    return strstr(name, word) != NULL;
}

// A move Red (1-165) or Crystal (1-251) doesn't have: the nearest one it does, by its name's words, else its type
// and whether it hits (KAI's own move data)
u16 MP_GbMoves_StandIn(u16 move, bool8 crystal)
{
    char n[32];
    u8 type = gBattleMoves[move].type;
    bool8 hits = gBattleMoves[move].power != 0;
    if (IS_MAKER_MOVE(move))   // (Moemon Maker's: its base's)
    {
        u16 Maker_MoveBase(u16 move);
        move = Maker_MoveBase(move) ? Maker_MoveBase(move) : MOVE_POUND;
        type = gBattleMoves[move].type, hits = gBattleMoves[move].power != 0;
    }
    if (move <= (crystal ? 251 : 165))
        return move;
    MoveName(move, n, sizeof(n));
    if (Has(n, "PUNCH"))   return type == TYPE_FIRE ? MOVE_FIRE_PUNCH : type == TYPE_ICE ? MOVE_ICE_PUNCH
                                : type == TYPE_ELECTRIC ? MOVE_THUNDER_PUNCH : MOVE_MEGA_PUNCH;
    if (Has(n, "KICK"))    return MOVE_MEGA_KICK;
    if (Has(n, "FANG") || Has(n, "BITE") || Has(n, "CRUNCH") || Has(n, "JAW"))
        return crystal ? MOVE_CRUNCH : MOVE_HYPER_FANG;
    if (Has(n, "CLAW") || Has(n, "SLASH") || Has(n, "CUT") || Has(n, "BLADE"))
        return crystal && type == TYPE_STEEL ? MOVE_METAL_CLAW : MOVE_SLASH;
    if (Has(n, "BEAM") || Has(n, "RAY"))
        return type == TYPE_ICE ? MOVE_ICE_BEAM : type == TYPE_PSYCHIC ? MOVE_PSYBEAM : type == TYPE_GRASS ? MOVE_SOLAR_BEAM
             : type == TYPE_BUG || type == TYPE_POISON ? MOVE_AURORA_BEAM : MOVE_HYPER_BEAM;
    if (Has(n, "WAVE") && type == TYPE_WATER) return MOVE_SURF;
    if (Has(n, "PULSE") || Has(n, "SPHERE") || Has(n, "BALL") || (Has(n, "BLAST") && type != TYPE_FIRE))
        return type == TYPE_ELECTRIC ? (crystal ? MOVE_ZAP_CANNON : MOVE_THUNDERBOLT)
             : type == TYPE_GHOST ? (crystal ? MOVE_SHADOW_BALL : MOVE_NIGHT_SHADE) : type == TYPE_WATER ? MOVE_BUBBLE_BEAM
             : type == TYPE_PSYCHIC ? MOVE_PSYCHIC : type == TYPE_FIGHTING ? (crystal ? MOVE_DYNAMIC_PUNCH : MOVE_SUBMISSION)
             : MOVE_SWIFT;
    if (Has(n, "DANCE") || Has(n, "BULK") || Has(n, "PLOT") || Has(n, "SHIFT") || Has(n, "CALM"))
        return MOVE_SWORDS_DANCE;
    if (!hits)
    {
        switch (type)
        {
        case TYPE_PSYCHIC: return MOVE_AMNESIA;
        case TYPE_GRASS:   return MOVE_SLEEP_POWDER;
        case TYPE_POISON:  return MOVE_TOXIC;
        case TYPE_ELECTRIC: return MOVE_THUNDER_WAVE;
        case TYPE_FIRE:    return crystal ? MOVE_SUNNY_DAY : MOVE_AGILITY;
        case TYPE_WATER:   return crystal ? MOVE_RAIN_DANCE : MOVE_WITHDRAW;
        case TYPE_ROCK:    return crystal ? MOVE_SANDSTORM : MOVE_HARDEN;
        case TYPE_ICE:     return MOVE_HAZE;
        case TYPE_GHOST:   return MOVE_CONFUSE_RAY;
        case TYPE_STEEL:   return MOVE_HARDEN;
        case TYPE_DARK:    return crystal ? MOVE_MEAN_LOOK : MOVE_LEER;
        case TYPE_FAIRY:   return crystal ? MOVE_SWEET_KISS : MOVE_GROWL;
        case TYPE_FLYING:  return MOVE_AGILITY;
        case TYPE_BUG:     return MOVE_STRING_SHOT;
        default:           return MOVE_GROWTH;
        }
    }
    switch (type)
    {
    case TYPE_FIRE:     return gBattleMoves[move].power >= 100 ? MOVE_FIRE_BLAST : MOVE_FLAMETHROWER;
    case TYPE_WATER:    return gBattleMoves[move].power >= 100 ? MOVE_HYDRO_PUMP : MOVE_WATER_GUN;
    case TYPE_ELECTRIC: return gBattleMoves[move].power >= 100 ? MOVE_THUNDER : MOVE_THUNDERBOLT;
    case TYPE_GRASS:    return gBattleMoves[move].power >= 100 ? MOVE_SOLAR_BEAM : MOVE_RAZOR_LEAF;
    case TYPE_ICE:      return gBattleMoves[move].power >= 100 ? MOVE_BLIZZARD : MOVE_ICE_BEAM;
    case TYPE_FIGHTING: return gBattleMoves[move].power >= 100 ? (crystal ? MOVE_CROSS_CHOP : MOVE_HI_JUMP_KICK) : MOVE_KARATE_CHOP;
    case TYPE_POISON:   return gBattleMoves[move].power >= 80 ? MOVE_SLUDGE : MOVE_ACID;
    case TYPE_GROUND:   return gBattleMoves[move].power >= 90 ? MOVE_EARTHQUAKE : MOVE_BONE_CLUB;
    case TYPE_FLYING:   return gBattleMoves[move].power >= 90 ? MOVE_SKY_ATTACK : MOVE_WING_ATTACK;
    case TYPE_PSYCHIC:  return gBattleMoves[move].power >= 80 ? MOVE_PSYCHIC : MOVE_CONFUSION;
    case TYPE_BUG:      return crystal ? (gBattleMoves[move].power >= 80 ? MOVE_MEGAHORN : MOVE_FURY_CUTTER) : MOVE_TWINEEDLE;
    case TYPE_ROCK:     return crystal ? (gBattleMoves[move].power >= 60 ? MOVE_ROCK_SLIDE : MOVE_ROLLOUT) : MOVE_ROCK_SLIDE;
    case TYPE_GHOST:    return crystal ? MOVE_SHADOW_BALL : MOVE_LICK;
    case TYPE_DRAGON:   return crystal && gBattleMoves[move].power >= 100 ? MOVE_OUTRAGE : MOVE_DRAGON_RAGE;
    case TYPE_DARK:     return crystal ? (gBattleMoves[move].power >= 70 ? MOVE_CRUNCH : MOVE_FAINT_ATTACK) : MOVE_BITE;
    case TYPE_STEEL:    return crystal ? (gBattleMoves[move].power >= 70 ? MOVE_IRON_TAIL : MOVE_STEEL_WING) : MOVE_SLAM;
    case TYPE_FAIRY:    return crystal ? MOVE_SWEET_KISS : MOVE_DIZZY_PUNCH;
    default:            return gBattleMoves[move].power >= 100 ? MOVE_HYPER_BEAM : gBattleMoves[move].power >= 70
                             ? MOVE_TAKE_DOWN : MOVE_TACKLE;
    }
}

// The Game Boy's picture box's centre for a side (its pixels)
static void BoxCentre(const struct GbGame *gm, bool8 player, s32 *x, s32 *y)
{
    const u8 *b = player ? gm->pbox : gm->ebox;
    *x = b[0] + b[2] / 2, *y = b[1] + b[3] / 2;
}

// KAI's battler's picture's centre: where its sprite is drawn as the move starts (a UI STYLE's layout, the species'
// own offsets: the coordinate tables don't know them all), else the tables'
static void KaiCentre(u8 battler, s32 *x, s32 *y)
{
    u8 id = battler < MAX_BATTLERS_COUNT ? gBattlerSpriteIds[battler] : MAX_SPRITES;
    if (id < MAX_SPRITES && gSprites[id].inUse)
    {
        *x = gSprites[id].x + gSprites[id].x2;
        *y = gSprites[id].y + gSprites[id].y2;
        return;
    }
    *x = GetBattlerSpriteCoord(battler, BATTLER_COORD_X_2);
    *y = GetBattlerSpriteCoord(battler, BATTLER_COORD_Y_PIC_OFFSET);
}

// One axis of the map: KAI's distance over the Game Boy's (256ths), kept between half and twice, 1 when the user is
// the target or they're level on it
static s32 Scale(s32 gd, s32 kd)
{
    s32 s;
    if (gd == 0 || (kd > -12 && kd < 12))
        return 256;
    s = kd * 256 / gd;
    if (s > 512) s = 512;
    if (s < -512) s = -512;
    if (s > -128 && s < 128) s = s < 0 ? -128 : 128;
    return s;
}

// A point of the Game Boy's screen on ours: where it is from the user's picture (that picture's size over the Game Boy's)
// near the user, from the target's near the target, and between the two as far along as it is on the Game Boy, so
// what sits on a POKeMON (Ember's flames at its feet, Thunderbolt's ball on it) sits on it here
static void MapPoint(s32 gx, s32 gy, s32 *kx, s32 *ky)
{
    s32 dx = sM.gtx - sM.gux, dy = sM.gty - sM.guy, len = dx * dx + dy * dy, t = 0;
    s32 ux = sM.kux + (gx - sM.gux) * sM.su256 / 256, uy = sM.kuy + (gy - sM.guy) * sM.su256 / 256;
    s32 tx = sM.ktx + (gx - sM.gtx) * sM.st256 / 256, ty = sM.kty + (gy - sM.gty) * sM.st256 / 256;
    if (len > 0)
        t = ((gx - sM.gux) * dx + (gy - sM.guy) * dy) * 256 / len;
    if (t < 0) t = 0;
    if (t > 256) t = 256;
    *kx = ux + (tx - ux) * t / 256;
    *ky = uy + (ty - uy) * t / 256;
}

// A side's picture's size here over the Game Boy's (KAI's are 64)
static s32 PicScale(const u8 *box)
{
    s32 s = box[2] > box[3] ? box[2] : box[3];
    s = s ? 64 * 256 / s : 256;
    return s < 192 ? 192 : s > 384 ? 384 : s;
}

// battle_anim.c LaunchBattleAnimation, a move's: the Game Boy's to show over it?
void MP_GbMoves_Begin(u16 move, u8 attacker, u8 target)
{
    s8 g = GbGame();
    const struct GbGame *gm;
    u16 gbMove;
    u32 off;
    s32 gtx, gty, ktx, kty;
    sM.active = FALSE;
    if (g < 0 || move >= MOVES_COUNT || attacker >= MAX_BATTLERS_COUNT)
        return;
    gm = &sGames[g];
    gbMove = MP_GbMoves_StandIn(move, g == 1);
    sM.turn = GetBattlerSide(attacker) == B_SIDE_PLAYER ? 0 : 1;
    if (gbMove == 0 || gbMove >= gm->moves || (off = R32(gm->anims + 4 * (gbMove * 2 + sM.turn))) == 0)
        return;
    sM.game = g;
    sM.battler[0] = attacker;
    sM.battler[1] = target < MAX_BATTLERS_COUNT ? target : attacker;
    sM.cursor = gm->base + off + 2;
    sM.gbLen = R16(gm->base + off);
    sM.fit = move < ARRAY_COUNT(sKaiMoveFrames) ? sKaiMoveFrames[move][sM.turn] : 0;
    if (sM.fit == 0)
        sM.fit = sM.gbLen;
    sM.decoded = -1;
    memset(&sM.f, 0, sizeof(sM.f));
    sM.f.bgp = 0xE4, sM.f.obp[0] = 0xD0, sM.f.obp[1] = 0xE0;
    sM.start = gMpAnimSteps;
    BoxCentre(gm, sM.turn == 0, &sM.gux, &sM.guy);
    BoxCentre(gm, sM.turn != 0, &gtx, &gty);
    KaiCentre(sM.battler[0], &sM.kux, &sM.kuy);
    KaiCentre(sM.battler[1], &ktx, &kty);
    sM.boxX[0] = sM.kux, sM.boxY[0] = sM.kuy, sM.boxX[1] = ktx, sM.boxY[1] = kty;
    sM.sx256 = Scale(gtx - sM.gux, ktx - sM.kux);
    sM.sy256 = Scale(gty - sM.guy, kty - sM.kuy);
    if (sM.battler[1] == sM.battler[0])   // (the move on itself: the Game Boy's other side kept where it'd be)
    {
        sM.sx256 = sM.sy256 = 256;
        ktx = sM.kux + (gtx - sM.gux), kty = sM.kuy + (gty - sM.guy);
    }
    sM.gtx = gtx, sM.gty = gty, sM.ktx = ktx, sM.kty = kty;
    sM.su256 = PicScale(sM.turn == 0 ? gm->pbox : gm->ebox);
    sM.st256 = PicScale(sM.turn == 0 ? gm->ebox : gm->pbox);
    sM.haveStill = sM.havePlate = FALSE;
    sM.wantPlate = TRUE;
    sM.active = TRUE;
    MPNet_Log("gb moves: %s move %d (as %d), side %d, %d frames fitted to %d (user %d,%d target %d,%d)",
              g ? "crystal" : "red", move, gbMove, sM.turn, sM.gbLen, sM.fit, sM.kux, sM.kuy, ktx, kty);
}

// ... its end (KAI's ended)
void MP_GbMoves_End(void)
{
    void MP_GbMoves_UnmuteKai(void);
    sM.active = FALSE;
    sM.wantPlate = FALSE;
    MP_GbMoves_UnmuteKai();
}

bool8 MP_GbMoves_Active(void)
{
    return sM.active;
}

// sound.c: KAI's animation's own sounds are played silent while the Game Boy's play (still played: an animation
// that waits for its sounds takes as long as it always does)
bool8 MP_GbMoves_MuteKai(void)
{
    return sM.active;
}

// sdl2.c VDraw: the backdrop without sprites, once, as a move starts
bool8 MP_GbMoves_WantPlate(void)
{
    return sM.active && sM.wantPlate;
}

void MP_GbMoves_TakePlate(const u16 *frame)
{
    memcpy(sPlate, frame, sizeof(sPlate));
    sM.wantPlate = FALSE;
    sM.havePlate = TRUE;
}

// ... and the same frame drawn with them, before anything is drawn over it (UI STYLE's boxes...): a battler's pixels
// are where the two differ
void MP_GbMoves_TakeRaw(const u16 *frame)
{
    memcpy(sRaw, frame, sizeof(sRaw));
}

// The next frame from the pack into sM.f; its sounds started
static void DecodeNext(void)
{
    const struct GbGame *gm = &sGames[sM.game];
    const u8 *p = sM.cursor;
    struct GbFrame *f = &sM.f;
    u8 flags = *p++, k, n;
    sM.decoded++;
    if (flags & 0x80)
    {
        sM.cursor = p;
        return;
    }
    f->flags = flags;
    f->bgp = *p++, f->obp[0] = *p++, f->obp[1] = *p++;
    if (flags & 1) f->scx = R16(p), p += 2; else f->scx = (s8)*p++;
    if (flags & 2) f->scy = R16(p), p += 2; else f->scy = (s8)*p++;
    f->winDx = (s8)*p++, f->winDy = *p++, f->winOn = *p++;
    for (k = 0; k < 2; k++)
    {
        f->picKind[k] = *p++;
        if (f->picKind[k] == 3)
            f->picMap[k] = R16(p), p += 2;
    }
    if (flags & 0x10)
        f->cgbPal = R16(p), p += 2;
    if (flags & 0x20)
        f->picBgp[0] = *p++, f->picBgp[1] = *p++;
    else
        f->picBgp[0] = f->picBgp[1] = f->bgp;
    f->nSpr = 0;
    if (flags & 0x08)
    {
        n = *p++;
        for (k = 0; k < n; k++, p += 5)
            if (f->nSpr < ARRAY_COUNT(f->spr))
            {
                struct GbSprite *s = &f->spr[f->nSpr++];
                s->x = p[0], s->y = p[1], s->tile = R16(p + 2), s->attr = p[4];
            }
    }
    if (flags & 0x04)
    {
        void MP_LegacySfx_PlayMove(const u8 *sound);
        n = *p++;
        for (k = 0; k < n; k++, p += 2)
        {
            u16 id = R16(p);
            if (id < gm->sounds)
                MP_LegacySfx_PlayMove(gm->base + R32(gm->soundData + 4 * id));
        }
    }
    sM.cursor = p;
}

// A colour's Game Boy shade (0 white .. 3 black)
static u8 ShadeOf(u16 c)
{
    s32 l = ((c & 31) * 77 + ((c >> 5) & 31) * 151 + ((c >> 10) & 31) * 28) >> 8;
    return l >= 24 ? 0 : l >= 16 ? 1 : l >= 8 ? 2 : 3;
}

// BGP on a colour of the battle: its shade taken where BGP sends it, the colour made that much lighter or darker
static u16 ApplyBgp(u16 c, u8 bgp)
{
    static const s32 sBright[4] = {256, 170, 85, 0};
    u8 from = ShadeOf(c), to = (bgp >> (from * 2)) & 3;
    s32 r = c & 31, g = (c >> 5) & 31, b = (c >> 10) & 31, k;
    if (to == from)
        return c;
    if (to == 3)
        return RGB(0, 0, 0);
    if (to > from)
    {
        k = sBright[to] * 256 / sBright[from];
        return RGB(r * k >> 8, g * k >> 8, b * k >> 8);
    }
    k = (sBright[to] - sBright[from]) * 256 / (256 - sBright[from]);
    return RGB(r + ((31 - r) * k >> 8), g + ((31 - g) * k >> 8), b + ((31 - b) * k >> 8));
}

// A sprite's shade's colour: GB OPTIONS > COLORS' when they're on, else greys
static u16 ObjColor(u8 shade, u8 obp)
{
    u16 MP_GbPal_ObjShade(u8 shade, u8 obp);
    return MP_GbPal_ObjShade(shade, obp);
}

// KAI's battler's 64x64 box
static void KaiBox(u8 battler, s32 *x0, s32 *y0)
{
    s32 cx, cy;
    if (battler == sM.battler[0] || battler == sM.battler[1])   // (as the move started: KAI's shakes don't move it)
    {
        u8 k = battler == sM.battler[0] ? 0 : 1;
        cx = sM.boxX[k], cy = sM.boxY[k];
    }
    else
        KaiCentre(battler, &cx, &cy);
    *x0 = cx - 32, *y0 = cy - 32;
}

// Is (x, y) the battler's (a sprite drew it: the frame differs from the backdrop there, and nothing was drawn over it
// since)?
static bool8 IsMon(s32 x, s32 y)
{
    u32 i = y * W + x;
    return x >= 0 && x < W && y >= 0 && y < H && sRaw[i] != sPlate[i] && sStill[i] == sRaw[i];
}

static u16 Shade(u16 c, u8 bgp)
{
    return bgp == 0xE4 ? c : ApplyBgp(c, bgp);
}

// Take the battler out of the picture (the backdrop under it, in BGP's shades)
static void Erase(u16 *img, u8 battler)
{
    s32 x0, y0, x, y;
    KaiBox(battler, &x0, &y0);
    for (y = max(0, y0); y < min(H, y0 + 64); y++)
        for (x = max(0, x0); x < min(W, x0 + 64); x++)
            if (IsMon(x, y))
                img[y * W + x] = Shade(sPlate[y * W + x], sM.f.bgp);
}

// The battler as it is, in its picture's shades
static void ShadeMon(u16 *img, u8 battler, u8 bgp)
{
    s32 x0, y0, x, y;
    if (bgp == 0xE4)
        return;
    KaiBox(battler, &x0, &y0);
    for (y = max(0, y0); y < min(H, y0 + 64); y++)
        for (x = max(0, x0); x < min(W, x0 + 64); x++)
            if (IsMon(x, y))
                img[y * W + x] = ApplyBgp(sStill[y * W + x], bgp);
}

// A picture as the Game Boy's map of cells says: each of its 7x7 cells the battler's own cell (64/7 pixels here)
// moved, or a tile of the move's own
static void DrawMap(u16 *img, u8 battler, u16 map, u8 bgp, u8 side)
{
    const struct GbGame *gm = &sGames[sM.game];
    const u8 *m;
    s32 x0, y0, n, i;
    if (map >= gm->picmaps)
        return;
    m = gm->base + R32(gm->picMaps + 4 * map);
    KaiBox(battler, &x0, &y0);
    n = *m++;
    for (i = 0; i < n; i++, m += 4)
    {
        s32 cx = (s8)m[0], cy = (s8)m[1], src = R16(m + 2), dx0, dy0, dx1, dy1, x, y;
        dx0 = x0 + cx * 64 / 7, dx1 = x0 + (cx + 1) * 64 / 7;
        dy0 = y0 + cy * 64 / 7, dy1 = y0 + (cy + 1) * 64 / 7;
        for (y = max(0, dy0); y < min(H, dy1); y++)
            for (x = max(0, dx0); x < min(W, dx1); x++)
            {
                if (src < 49)
                {
                    s32 sx = x0 + (src % 7) * 64 / 7 + (x - dx0), sy = y0 + (src / 7) * 64 / 7 + (y - dy0);
                    if (IsMon(sx, sy))
                        img[y * W + x] = Shade(sStill[sy * W + sx], bgp);
                }
                else if (src >= 256 && src - 256 < gm->pictiles)
                {
                    const u8 *t = gm->picTileData + (src - 256) * 16;
                    s32 tx = (x - dx0) * 8 / (dx1 - dx0), ty = (y - dy0) * 8 / (dy1 - dy0);
                    u8 v = ((t[ty * 2] >> (7 - tx)) & 1) | (((t[ty * 2 + 1] >> (7 - tx)) & 1) << 1);
                    if (v && sM.game == 1 && (sM.f.flags & 0x10) && sM.f.cgbPal < gm->cgbpals)
                    {
                        void MP_GbPal_AllowColor(u16 c);
                        img[y * W + x] = R16(gm->cgbPalData + sM.f.cgbPal * 128 + ((side ^ sM.turn) * 4 + v) * 2);
                        MP_GbPal_AllowColor(img[y * W + x]);
                    }
                    else if (v)
                        img[y * W + x] = ObjColor((bgp >> (v * 2)) & 3, 2);
                }
            }
    }
}

// The line's shift (the Game Boy's line under KAI's line Y): the window's when it covers it, else the background's
// scroll
static void LineShift(s32 y, s32 *dx, s32 *dy)
{
    const struct GbGame *gm = &sGames[sM.game];
    const struct GbFrame *f = &sM.f;
    s32 gy = sM.guy + (y - sM.kuy) * 256 / sM.sy256;
    if (gy < 0) gy = 0;
    if (gy > 143) gy = 143;
    if (f->winOn && f->winDy < 144 && gy >= f->winDy)
    {
        *dx = f->winDx, *dy = f->winDy;
        return;
    }
    *dx = -((f->flags & 1) && f->scx < gm->lines ? (s8)gm->lineData[f->scx * 144 + gy] : f->scx);
    *dy = -((f->flags & 2) && f->scy < gm->lines ? (s8)gm->lineData[f->scy * 144 + gy] : f->scy);
}

// Sprites that touch are one object (a flame, a bolt, a ball: built of 8x8 tiles): the object's centre is mapped and
// its tiles keep their 8-pixel spacing, as the GENESIS / EPOCH BALLS' throws do (mapping each tile on its own opened
// gaps through it)
static u8 Root(u8 *up, u8 i)
{
    while (up[i] != i)
        i = up[i] = up[up[i]];
    return i;
}

static void DrawSprites(u16 *img)
{
    const struct GbGame *gm = &sGames[sM.game];
    const struct GbFrame *f = &sM.f;
    u8 up[ARRAY_COUNT(f->spr)];
    s16 bx0[ARRAY_COUNT(f->spr)], by0[ARRAY_COUNT(f->spr)], bx1[ARRAY_COUNT(f->spr)], by1[ARRAY_COUNT(f->spr)];
    s16 offx[ARRAY_COUNT(f->spr)], offy[ARRAY_COUNT(f->spr)];
    s32 i, j;
    for (i = 0; i < f->nSpr; i++)
        up[i] = i;
    for (i = 0; i < f->nSpr; i++)
    {
        s32 ax = f->spr[i].x - 8, ay = f->spr[i].y - 16, ah = (f->spr[i].tile & 0x8000) ? 16 : 8;
        for (j = i + 1; j < f->nSpr; j++)
        {
            s32 bx = f->spr[j].x - 8, by = f->spr[j].y - 16, bh = (f->spr[j].tile & 0x8000) ? 16 : 8;
            if (ax <= bx + 8 && bx <= ax + 8 && ay <= by + bh && by <= ay + ah)
                up[Root(up, j)] = Root(up, i);
        }
    }
    for (i = 0; i < f->nSpr; i++)
        bx0[i] = by0[i] = 0x7FFF, bx1[i] = by1[i] = -0x7FFF;
    for (i = 0; i < f->nSpr; i++)
    {
        u8 r = Root(up, i);
        s32 x = f->spr[i].x - 8, y = f->spr[i].y - 16, h = (f->spr[i].tile & 0x8000) ? 16 : 8;
        if (x < bx0[r]) bx0[r] = x;
        if (y < by0[r]) by0[r] = y;
        if (x + 8 > bx1[r]) bx1[r] = x + 8;
        if (y + h > by1[r]) by1[r] = y + h;
    }
    for (i = 0; i < f->nSpr; i++)
        if (Root(up, i) == i)
        {
            s32 cx = (bx0[i] + bx1[i]) / 2, cy = (by0[i] + by1[i]) / 2, kx, ky;
            MapPoint(cx, cy, &kx, &ky);
            offx[i] = kx - cx, offy[i] = ky - cy;
        }
    for (i = f->nSpr - 1; i >= 0; i--)   // (the first in OAM over the rest)
    {
        const struct GbSprite *s = &f->spr[i];
        bool8 tall = (s->tile & 0x8000) != 0;
        u16 tile = s->tile & 0x7FFF;
        s32 gx = s->x - 8, gy = s->y - 16, h = tall ? 16 : 8, kx, ky, x, y;
        u8 obp = (s->attr >> 4) & 1, r = Root(up, i);
        if (tile >= gm->tiles)
            continue;
        kx = gx + offx[r], ky = gy + offy[r];
        for (y = 0; y < h; y++)
        {
            s32 py = ky + y, ty = (s->attr & 0x40) ? h - 1 - y : y;
            const u8 *t = gm->tileData + (tile + ty / 8) * 16 + (ty & 7) * 2;
            if (py < 0 || py >= H || tile + ty / 8 >= gm->tiles)
                continue;
            for (x = 0; x < 8; x++)
            {
                s32 px = kx + x, tx = (s->attr & 0x20) ? 7 - x : x;
                u8 v = ((t[0] >> (7 - tx)) & 1) | (((t[1] >> (7 - tx)) & 1) << 1);
                if (v == 0 || px < 0 || px >= W)
                    continue;
                if ((s->attr & 0x80) && sRaw[py * W + px] != sPlate[py * W + px])
                    continue;   // (behind the background's colours 1-3: what's drawn over the backdrop here)
                if (sM.game == 1 && (f->flags & 0x10) && f->cgbPal < gm->cgbpals)   // (Crystal's own colours)
                {
                    void MP_GbPal_AllowColor(u16 c);
                    img[py * W + px] = R16(gm->cgbPalData + f->cgbPal * 128 + 64 + ((s->attr & 7) * 4 + v) * 2);
                    MP_GbPal_AllowColor(img[py * W + px]);
                }
                else
                    img[py * W + px] = ObjColor((f->obp[obp] >> (v * 2)) & 3, obp);
            }
        }
    }
}

// sdl2.c VDraw, after the battle's own drawing and UI STYLE's, before GB OPTIONS > COLORS
void MP_GbMoves_Composite(u16 *frame)
{
    static u16 base[W * H];
    u32 elapsed;
    s32 target, x, y;
    u8 k;
    if (sM.active && !gMain.inBattle)
        MP_GbMoves_End();   // (a battle that ended under an animation: a link lost...)
    if (!sM.active)
    {
        if (gMain.inBattle && GbGame() >= 0)
            memcpy(sPrev, frame, sizeof(sPrev));   // (the frame a move starts from)
        return;
    }
    if (!sM.haveStill)
    {
        memcpy(sStill, sPrev, sizeof(sStill));
        sM.haveStill = TRUE;
    }
    if (!sM.havePlate)
    {
        memcpy(sPlate, sStill, sizeof(sPlate));
        memcpy(sRaw, sStill, sizeof(sRaw));
    }
    elapsed = gMpAnimSteps - sM.start;   // (KAI's script's steps: as many as it measured, whatever ANIM SPEED does)
    target = sM.gbLen > sM.fit ? (s32)(elapsed * sM.gbLen / sM.fit) : (s32)elapsed;
    if (target >= sM.gbLen)
        target = sM.gbLen - 1;
    while (sM.decoded < target)
        DecodeNext();
    {
        char *getenv(const char *name);
        static s8 sTrace = -1;
        if (sTrace < 0)
            sTrace = getenv("MOEMON_GBMOVES_TRACE") != NULL;
        if (sTrace)
            MPNet_Log("gb moves frame %d (step %u): flags %x bgp %x scx %d scy %d win %d,%d,%d pics %d/%d spr %d",
                      sM.decoded, elapsed, sM.f.flags, sM.f.bgp, sM.f.scx, sM.f.scy, sM.f.winDx, sM.f.winDy, sM.f.winOn,
                      sM.f.picKind[0], sM.f.picKind[1], sM.f.nSpr);
    }
    // the battle in BGP's shades, its two pictures as the Game Boy's frame has them, each in its own
    if (sM.f.bgp == 0xE4)
        memcpy(base, sStill, sizeof(base));
    else
        for (x = 0; x < W * H; x++)
            base[x] = ApplyBgp(sStill[x], sM.f.bgp);
    for (k = 0; k < 2; k++)
    {
        if (k == 1 && sM.battler[1] == sM.battler[0])
            break;
        if (sM.f.picKind[k] == 2 || sM.f.picKind[k] == 3)
            Erase(base, sM.battler[k]);
        else
            ShadeMon(base, sM.battler[k], sM.f.picBgp[k]);
    }
    for (k = 0; k < 2; k++)
        if (sM.f.picKind[k] == 3 && !(k == 1 && sM.battler[1] == sM.battler[0]))
            DrawMap(base, sM.battler[k], sM.f.picMap[k], sM.f.picBgp[k], k);
    // shaken, waved and in BGP's shades
    for (y = 0; y < H; y++)
    {
        s32 dx, dy;
        LineShift(y, &dx, &dy);
        for (x = 0; x < W; x++)
        {
            s32 sx = x - dx, sy = y - dy;
            if (sx < 0) sx = 0;
            if (sx >= W) sx = W - 1;
            if (sy < 0) sy = 0;
            if (sy >= H) sy = H - 1;
            frame[y * W + x] = base[sy * W + sx];
        }
    }
    DrawSprites(frame);
}
