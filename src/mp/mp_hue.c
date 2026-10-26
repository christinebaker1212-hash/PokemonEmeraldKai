// Moemon Co-op: Pokémon Stadium's colors (D5). A NICKNAMED Moemon gets its own hue, worked out the way Pokémon
// Stadium 2 does it (the bluemoonfalls "Stadium hues" page: tools/moemon_hues): the OT name, the nickname and the
// Trainer ID add up to a number 0-255 that picks a hue shift within the species' limits. Kai's pictures keep their
// skin tones (src/data/mp_hues.h: the skin slots of each picture); the original POKéMON pictures (OPTIONS >
// CREATURES), the icons and the followers turn as a whole. Nothing is stored: the hue follows from the Pokémon.
//
// Where it shows: every Moemon picture that has its Pokémon (pokemon.c's palette getters: battle, summary, PC,
// trades, evolution, hatching, contests...), the party / PC icons (pokemon_icon.c), the followers (mp_follower.c,
// friends' too: their hue travels in the wire state).
#include <string.h>
#include "global.h"
#include "mp.h"
#include "data.h"
#include "decompress.h"
#include "palette.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "sprite.h"
#include "string_util.h"
#include "constants/characters.h"
#include "constants/species.h"
#include "constants/pokemon_icon.h"
#include "../data/mp_hues.h"

// ---------------------------------------------------------------------------
// Small math (the game links without libm)

static double Floor(double x)
{
    double t = (double)(long long)x;
    return t > x ? t - 1 : t;
}

static double Abs(double x)
{
    return x < 0 ? -x : x;
}

static double Fmod(double a, double b)   // JavaScript's %: the sign of a
{
    return a - b * (double)(long long)(a / b);
}

// ---------------------------------------------------------------------------
// The Stadium number

// A game character as Pokémon Stadium counts it (its names are ASCII; other letters by their Latin-1 code)
static u8 StadiumCode(u8 c)
{
    if (c >= CHAR_A && c <= CHAR_Z)
        return 'A' + (c - CHAR_A);
    if (c >= CHAR_a && c <= CHAR_z)
        return 'a' + (c - CHAR_a);
    if (c >= CHAR_0 && c <= CHAR_9)
        return '0' + (c - CHAR_0);
    switch (c)
    {
    case CHAR_SPACE:            return 0x20;
    case CHAR_LEFT_PAREN:       return 0x28;
    case CHAR_RIGHT_PAREN:      return 0x29;
    case CHAR_COLON:            return 0x3A;
    case CHAR_SEMICOLON:        return 0x3B;
    case CHAR_HYPHEN:           return 0x2D;
    case CHAR_QUESTION_MARK:    return 0x3F;
    case CHAR_EXCL_MARK:        return 0x21;
    case CHAR_PERIOD:           return 0x2E;
    case CHAR_SLASH:            return 0x2F;
    case CHAR_COMMA:            return 0x2C;
    case CHAR_MULT_SIGN:        return 0xD7;
    case CHAR_PK:               return 0xBC;
    case CHAR_MN:               return 0xBD;
    case CHAR_MALE:             return 0xA9;
    case CHAR_FEMALE:           return 0xBE;
    case CHAR_A_DIAERESIS:      return 0xC4;
    case CHAR_A_DIAERESIS + 1:  return 0xD6;   // Ö
    case CHAR_A_DIAERESIS + 2:  return 0xDC;   // Ü
    case CHAR_A_DIAERESIS + 3:  return 0xE4;   // ä
    case CHAR_o_DIAERESIS:      return 0xF6;
    case CHAR_o_DIAERESIS + 1:  return 0xFC;   // ü
    // not in Stadium's names: the nearest code
    case CHAR_SGL_QUOTE_LEFT:
    case CHAR_SGL_QUOTE_RIGHT:  return 0x27;
    case CHAR_DBL_QUOTE_LEFT:
    case CHAR_DBL_QUOTE_RIGHT:  return 0x22;
    case CHAR_AMPERSAND:        return 0x26;
    case CHAR_PLUS:             return 0x2B;
    case CHAR_EQUALS:           return 0x3D;
    case CHAR_PERCENT:          return 0x25;
    case CHAR_A_GRAVE:          return 0xC0;
    case CHAR_A_ACUTE:          return 0xC1;
    case CHAR_A_CIRCUMFLEX:     return 0xC2;
    case CHAR_C_CEDILLA:        return 0xC7;
    case CHAR_E_GRAVE:          return 0xC8;
    case CHAR_E_ACUTE:          return 0xC9;
    case CHAR_E_CIRCUMFLEX:     return 0xCA;
    case CHAR_E_DIAERESIS:      return 0xCB;
    case CHAR_I_GRAVE:          return 0xCC;
    case CHAR_I_ACUTE:          return 0xCD;
    case CHAR_I_CIRCUMFLEX:     return 0xCE;
    case CHAR_I_DIAERESIS:      return 0xCF;
    case CHAR_O_GRAVE:          return 0xD2;
    case CHAR_O_ACUTE:          return 0xD3;
    case CHAR_O_CIRCUMFLEX:     return 0xD4;
    case CHAR_U_GRAVE:          return 0xD9;
    case CHAR_U_ACUTE:          return 0xDA;
    case CHAR_U_CIRCUMFLEX:     return 0xDB;
    case CHAR_N_TILDE:          return 0xD1;
    case CHAR_ESZETT:           return 0xDF;
    case CHAR_a_GRAVE:          return 0xE0;
    case CHAR_a_ACUTE:          return 0xE1;
    case CHAR_a_CIRCUMFLEX:     return 0xE2;
    case CHAR_c_CEDILLA:        return 0xE7;
    case CHAR_e_GRAVE:          return 0xE8;
    case CHAR_e_ACUTE:          return 0xE9;
    case CHAR_e_CIRCUMFLEX:     return 0xEA;
    case CHAR_e_DIAERESIS:      return 0xEB;
    case CHAR_i_GRAVE:          return 0xEC;
    case CHAR_i_ACUTE:          return 0xED;
    case CHAR_i_CIRCUMFLEX:     return 0xEE;
    case CHAR_i_DIAERESIS:      return 0xEF;
    case CHAR_o_GRAVE:          return 0xF2;
    case CHAR_o_ACUTE:          return 0xF3;
    case CHAR_o_CIRCUMFLEX:     return 0xF4;
    case CHAR_u_GRAVE:          return 0xF9;
    case CHAR_u_ACUTE:          return 0xFA;
    case CHAR_u_CIRCUMFLEX:     return 0xFB;
    case CHAR_n_TILDE:          return 0xF1;
    }
    return 0;
}

static u32 SumName(const u8 *s, u8 max)
{
    u32 sum = 0;
    u8 i;
    for (i = 0; i < max && s[i] != EOS; i++)
        sum += StadiumCode(s[i]);
    return sum;
}

// The page's NV: OT name + nickname + the Trainer ID's two bytes, last two hex digits
u8 MP_Hue_Number(const u8 *otName, const u8 *nickname, u16 trainerId)
{
    return (SumName(otName, PLAYER_NAME_LENGTH) + SumName(nickname, POKEMON_NAME_LENGTH) + (trainerId >> 8) + (trainerId & 0xFF)) & 0xFF;
}

// The hue the page's slider ends on: (NV - 255/range*min) * range/255, an <input type=range step=1> from -min to
// max (rounded to the nearest whole degree, halves up)
s16 MP_Hue_Degrees(u16 species, u8 nv)
{
    double def, inc, hue;
    int min, max, range, deg;
    if (species == SPECIES_NONE || species >= NUM_SPECIES)
        return 0;
    min = sMpHueMin[species];
    max = sMpHueMax[species];
    range = min + max;
    if (range == 0)
        return 0;
    def = (255.0 / range) * min;
    inc = range / 255.0;
    hue = (nv - def) * inc;
    deg = (int)Floor(hue + 0.5);
    if (deg < -min)
        deg = -min;
    if (deg > max)
        deg = max;
    return deg;
}

void MP_Hue_Limits(u16 species, u8 *min, u8 *max)
{
    *min = species < NUM_SPECIES ? sMpHueMin[species] : 0;
    *max = species < NUM_SPECIES ? sMpHueMax[species] : 0;
}

static bool8 IsNicknamed(u16 species, const u8 *nickname)
{
    const u8 *name = gSpeciesNames[species];
    u8 i;
    for (i = 0; i < POKEMON_NAME_LENGTH; i++)
    {
        if (nickname[i] != name[i])
            return TRUE;
        if (name[i] == EOS)
            return FALSE;
    }
    return FALSE;
}

// The hue of a Pokémon (0: none - not nicknamed, an Egg, nothing there)
s16 MP_Hue_OfBoxMon(const struct BoxPokemon *b)
{
    u8 nick[POKEMON_NAME_LENGTH + 2], ot[PLAYER_NAME_LENGTH + 2];
    u16 species;
    struct BoxPokemon *m = (struct BoxPokemon *)b;
    if (b == NULL || !GetBoxMonData(m, MON_DATA_SANITY_HAS_SPECIES) || GetBoxMonData(m, MON_DATA_SANITY_IS_EGG)
     || GetBoxMonData(m, MON_DATA_SANITY_IS_BAD_EGG))
        return 0;
    species = GetBoxMonData(m, MON_DATA_SPECIES);
    memset(nick, EOS, sizeof(nick));
    memset(ot, EOS, sizeof(ot));
    GetBoxMonData(m, MON_DATA_NICKNAME, nick);
    nick[POKEMON_NAME_LENGTH] = EOS;
    if (species == SPECIES_NONE || species >= NUM_SPECIES || !IsNicknamed(species, nick))
        return 0;
    GetBoxMonData(m, MON_DATA_OT_NAME, ot);
    ot[PLAYER_NAME_LENGTH] = EOS;
    return MP_Hue_Degrees(species, MP_Hue_Number(ot, nick, GetBoxMonData(m, MON_DATA_OT_ID) & 0xFFFF));
}

// The Pokémon a picture is of, when only its IDs are known: the party, the other side (battles, trades), the
// boxes (ours, and the shared PC's while it's shown), the DAY CARE
static const struct BoxPokemon *FindMon(u32 otId, u32 personality, bool8 anyOt)
{
    struct PokemonStorage *stores[2];
    u32 i, j, k;
#define MATCHES(b) ((b)->personality == personality && (anyOt || (b)->otId == otId) && ((b)->personality || (b)->otId))
    for (i = 0; i < PARTY_SIZE; i++)
    {
        if (MATCHES(&gPlayerParty[i].box))
            return &gPlayerParty[i].box;
        if (MATCHES(&gEnemyParty[i].box))
            return &gEnemyParty[i].box;
    }
    stores[0] = gPokemonStoragePtr;
    stores[1] = MP_PersonalStorage();
    for (k = 0; k < 2; k++)
    {
        if (stores[k] == NULL || (k == 1 && stores[1] == stores[0]))
            continue;
        for (i = 0; i < TOTAL_BOXES_COUNT; i++)
            for (j = 0; j < IN_BOX_COUNT; j++)
                if (MATCHES(&stores[k]->boxes[i][j]))
                    return &stores[k]->boxes[i][j];
    }
    if (gSaveBlock1Ptr != NULL)
        for (i = 0; i < DAYCARE_MON_COUNT; i++)
            if (MATCHES(&gSaveBlock1Ptr->daycare.mons[i].mon))
                return &gSaveBlock1Ptr->daycare.mons[i].mon;
#undef MATCHES
    return NULL;
}

s16 MP_Hue_Find(u32 otId, u32 personality)
{
    return MP_Hue_OfBoxMon(FindMon(otId, personality, FALSE));
}

// Icons only know the personality
s16 MP_Hue_FindByPersonality(u32 personality)
{
    if (personality == 0)
        return 0;
    return MP_Hue_OfBoxMon(FindMon(0, personality, TRUE));
}

// ---------------------------------------------------------------------------
// The color turn (hue-shift.js: RGB -> HSL with whole degrees and percents, back with rounding)

static double JsRound(double x)
{
    return Floor(x + 0.5);
}

static void ShiftColor(u16 *color, int hueDiff)
{
    u16 c = *color;
    int r5 = c & 31, g5 = (c >> 5) & 31, b5 = (c >> 10) & 31;
    double r = ((r5 << 3) | (r5 >> 2)) / 255.0, g = ((g5 << 3) | (g5 >> 2)) / 255.0, b = ((b5 << 3) | (b5 >> 2)) / 255.0;
    double cmin = r < g ? (r < b ? r : b) : (g < b ? g : b), cmax = r > g ? (r > b ? r : b) : (g > b ? g : b), delta = cmax - cmin;
    double h = 0, s, l, cc, x, m, rr = 0, gg = 0, bb = 0;
    int hi, si, li, R, G, B;

    if (delta == 0)
        h = 0;
    else if (cmax == r)
        h = Fmod((g - b) / delta, 6);
    else if (cmax == g)
        h = (b - r) / delta + 2;
    else
        h = (r - g) / delta + 4;
    hi = (int)JsRound(h * 60);
    if (hi < 0)
        hi += 360;
    l = (cmax + cmin) / 2;
    s = delta == 0 ? 0 : delta / (1 - Abs(2 * l - 1));
    si = (int)(s * 100);   // parseInt: whole percents, cut
    li = (int)(l * 100);

    hi = (hi + hueDiff) % 360;
    s = si / 100.0;
    l = li / 100.0;
    cc = (1 - Abs(2 * l - 1)) * s;
    x = cc * (1 - Abs(Fmod(hi / 60.0, 2) - 1));
    m = l - cc / 2;
    if (hi < 60)
        rr = cc, gg = x, bb = 0;
    else if (hi < 120)
        rr = x, gg = cc, bb = 0;
    else if (hi < 180)
        rr = 0, gg = cc, bb = x;
    else if (hi < 240)
        rr = 0, gg = x, bb = cc;
    else if (hi < 300)
        rr = x, gg = 0, bb = cc;
    else
        rr = cc, gg = 0, bb = x;
    R = (int)JsRound((rr + m) * 255);
    G = (int)JsRound((gg + m) * 255);
    B = (int)JsRound((bb + m) * 255);
    R = R < 0 ? 0 : R > 255 ? 255 : R;
    G = G < 0 ? 0 : G > 255 ? 255 : G;
    B = B < 0 ? 0 : B > 255 ? 255 : B;
    *color = ((R * 31 + 127) / 255) | (((G * 31 + 127) / 255) << 5) | (((B * 31 + 127) / 255) << 10);
}

// Turns a 16-color palette by `hue` degrees, except the slots in keepMask (bit n = slot n) and slot 0
void MP_Hue_ShiftPalette(u16 *pal, u16 keepMask, s16 hue)
{
    int diff = hue < 0 ? hue + 360 : hue, i;
    if (hue == 0)
        return;
    for (i = 1; i < 16; i++)
        if (!(keepMask & (1 << i)))
            ShiftColor(&pal[i], diff);
}

// The skin slots a species' picture keeps (none for the original POKéMON pictures)
static u16 SkinMask(u16 species)
{
    if (MP_MonArtPokemon() || species >= NUM_SPECIES)
        return 0;
    return sMpHueSkin[species];
}

// ---------------------------------------------------------------------------
// Turned palettes for the game's palette getters (compressed like the originals)

#define LZ_RING 16
// A palette can hold several 16-color palettes: CASTFORM's has its four forms (128 bytes), the rest one (32 bytes)
#define LZ_MAX_COLORS 64
// 4-byte header + (flag byte + 8 literal bytes) per 8 bytes of colors
static u32 sLzRing[LZ_RING][(4 + (LZ_MAX_COLORS * 2 / 8) * 9 + 3) / 4];
static struct CompressedSpritePalette sStructRing[LZ_RING];
static u8 sLzNext;

// The size an LZ77 palette unpacks to (bytes), 0 if it isn't one this can turn
static u32 LzPaletteSize(const u32 *lz)
{
    const u8 *b = (const u8 *)lz;
    u32 size = b[1] | (b[2] << 8) | (b[3] << 16);
    if (b[0] != 0x10 || size == 0 || size % 32 != 0 || size > LZ_MAX_COLORS * 2)
        return 0;
    return size;
}

static const u32 *MakeLz(const u32 *lz, u16 species, s16 hue, u8 *slotOut)
{
    u16 pal[LZ_MAX_COLORS];
    u32 size = LzPaletteSize(lz), i, k;
    u8 *out, slot;
    if (size == 0)
        return lz;   // (not a palette this knows: as it is)
    LZ77UnCompWram(lz, pal);
    for (i = 0; i < size / 32; i++)
        MP_Hue_ShiftPalette(&pal[i * 16], SkinMask(species), hue);
    slot = sLzNext;
    sLzNext = (sLzNext + 1) % LZ_RING;
    out = (u8 *)sLzRing[slot];
    out[0] = 0x10;
    out[1] = size & 0xFF;
    out[2] = size >> 8;
    out[3] = 0;
    for (i = 0; i < size / 8; i++)
    {
        out[4 + i * 9] = 0;   // eight literal bytes follow
        for (k = 0; k < 4; k++)
        {
            out[4 + i * 9 + 1 + k * 2] = pal[i * 4 + k] & 0xFF;
            out[4 + i * 9 + 2 + k * 2] = pal[i * 4 + k] >> 8;
        }
    }
    if (slotOut)
        *slotOut = slot;
    return sLzRing[slot];
}

const u32 *MP_Hue_Lz(const u32 *lz, u16 species, s16 hue)
{
    if (hue == 0 || lz == NULL)
        return lz;
    return MakeLz(lz, species, hue, NULL);
}

// Sprite palette tags for turned icon palettes: one per (icon palette, hue); a new pair takes the next tag that isn't
// loaded, in turn, so a tag isn't reused soon after
#define HUE_TAGS     128
static struct { u16 orig; s16 hue; } sTags[HUE_TAGS];
static u8 sTagNext;

u16 MP_Hue_Tag(u16 orig, s16 hue)
{
    u32 i, n;
    if (hue == 0)
        return orig;
    // the same colors keep their tag (a caller often asks twice: once for the tag, once to load it)
    for (i = 0; i < HUE_TAGS; i++)
        if (sTags[i].orig == orig && sTags[i].hue == hue)
            return MP_HUE_TAG_BASE + i;
    for (n = 0; n < HUE_TAGS; n++)
    {
        i = (sTagNext + n) % HUE_TAGS;
        if (IndexOfSpritePaletteTag(MP_HUE_TAG_BASE + i) == 0xFF)
        {
            sTags[i].orig = orig;
            sTags[i].hue = hue;
            sTagNext = (i + 1) % HUE_TAGS;
            return MP_HUE_TAG_BASE + i;
        }
    }
    return orig;   // all in use (can't happen with 64 palette slots): the plain colors
}

// The struct getters keep the species' own tag: the game uses it as the species too (SetMultiuseSpriteTemplateToPokemon,
// sprites made with the species as their palette tag). A palette already loaded under that tag (the same species shown
// just before, maybe in other colors) gets this Pokémon's colors, since loading it again would keep the old ones.
const struct CompressedSpritePalette *MP_Hue_LzStruct(const struct CompressedSpritePalette *pal, u16 species, s16 hue)
{
    u8 slot, loaded;
    if (pal == NULL || pal->data == NULL)
        return pal;
    if (LzPaletteSize(pal->data) == 0)
        return pal;
    loaded = IndexOfSpritePaletteTag(pal->tag);
    if (loaded != 0xFF)
    {
        u16 colors[LZ_MAX_COLORS];   // (the loaded palette is the first 16 of them)
        LZ77UnCompWram(pal->data, colors);
        MP_Hue_ShiftPalette(colors, SkinMask(species), hue);
        if (memcmp(colors, &gPlttBufferUnfaded[OBJ_PLTT_ID(loaded)], PLTT_SIZE_4BPP) != 0)
        {
            CpuCopy16(colors, &gPlttBufferUnfaded[OBJ_PLTT_ID(loaded)], PLTT_SIZE_4BPP);
            if (!gPaletteFade.active)
                CpuCopy16(colors, &gPlttBufferFaded[OBJ_PLTT_ID(loaded)], PLTT_SIZE_4BPP);
        }
    }
    if (hue == 0)
        return pal;
    if (MakeLz(pal->data, species, hue, &slot) == pal->data)
        return pal;
    sStructRing[slot].data = sLzRing[slot];
    sStructRing[slot].tag = pal->tag;
    return &sStructRing[slot];
}

// ---------------------------------------------------------------------------
// Icons: the icon palettes are shared by many species, so a turned icon gets a copy of its own

// The palette tag a turned icon uses, loading its colors (the plain tag if there's no room)
u16 MP_Hue_IconPaletteTag(u16 baseTag, const u16 *colors, s16 hue)
{
    struct SpritePalette sp;
    u16 pal[16], tag;
    if (hue == 0 || colors == NULL)
        return baseTag;
    tag = MP_Hue_Tag(baseTag, hue);
    if (tag == baseTag || IndexOfSpritePaletteTag(tag) != 0xFF)
        return tag;
    memcpy(pal, colors, sizeof(pal));
    MP_Hue_ShiftPalette(pal, 0, hue);
    sp.data = pal;
    sp.tag = tag;
    if (LoadSpritePalette(&sp) == 0xFF)
        return baseTag;
    return tag;
}

// A turned icon palette nobody shows any more is let go (icons are freed one by one, their palettes by base tag)
void MP_Hue_FreeUnusedIconPalettes(void)
{
    u32 i, s;
    for (i = 0; i < HUE_TAGS; i++)
    {
        u8 slot = IndexOfSpritePaletteTag(MP_HUE_TAG_BASE + i);
        bool8 used = FALSE;
        if (slot == 0xFF || sTags[i].orig < POKE_ICON_BASE_PAL_TAG || sTags[i].orig >= POKE_ICON_BASE_PAL_TAG + 6)
            continue;
        for (s = 0; s < MAX_SPRITES && !used; s++)
            if (gSprites[s].inUse && gSprites[s].oam.paletteNum == slot)
                used = TRUE;
        if (!used)
            FreeSpritePaletteByTag(MP_HUE_TAG_BASE + i);
    }
}

bool8 MP_Hue_IsHueTag(u16 tag)
{
    return tag >= MP_HUE_TAG_BASE && tag < MP_HUE_TAG_BASE + HUE_TAGS;
}
