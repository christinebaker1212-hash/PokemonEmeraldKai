// Moemon Kai: UNOVA's POKeMON MUSICALS (the user's approved mock-ups, docs/unova/mockups/musical_*.png; "use Black's
// real props when available", 2026-11-14). Black's own data from the user's ROM (tools/unova/build_musical.py ->
// unova/black_musical.pak, never committed): its 100 props (pictures, the body places each may be worn at, its
// category and offset), its four shows (STARDOM, FOREST STROLL, A SWEET SOIREE, EXCITING NIMBASA: the judges' share
// for each category, six NPC performers each with their props, the fans' reward row) and the fans' tables.
//
// Black's flow (ov010 musical_event.c MUSICAL_EVENT_Main), run here as UNOVA's own: StartMusical ends the receptionist's
// script; the theater's zone script 3 introduces the show, the DRESS UP screen, script 4, the STAGE (three NPC
// performers Black's show picks by the fan points, or friends in the room who dressed up lately), the RESULTS, Black's
// scoring (musical_program.c SplitBonus: each category's share split over every prop of it on stage, then
// ScorePerformers: a prop the performer used during the show earns its share again; capped at 255), the save's records
// and the ten fans waiting with props (RollProgramRewards), script 5, then the shared closing script 0x28CC. The
// zone's scripts read the show through MUSICAL_GetValue 30-39 and MUSICAL_BufferString (u3d_vm.c).
//
// Kai's own: any POKeMON can perform (Black allowed only some); props sit at points found from the POKeMON's own picture
// (its outline: ears, head, eyes, face, body, waist, hands) with Black's offsets; friends' latest dress-ups reach the
// room (relay MPR_MUSICAL) and take NPC places on stage. Its own code (u3d_screen.c's host draws it).
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>
#include "global.h"
#include "main.h"
#include "data.h"
#include "decompress.h"
#include "pokemon.h"
#include "random.h"
#include "sound.h"
#include "string_util.h"
#include "script.h"
#include "constants/songs.h"
#include "constants/species.h"
#include "constants/characters.h"
#include "mp.h"
#include "u3d.h"
#include "u3d_screen.h"
#include "../tcg/tcg_gfx.h"

const char *Updater_ExeFile(const char *name);
void *U3D_Vm_MusicalSave(void);
bool8 U3D_Vm_RunScene(u16 scriptId);
bool8 U3D_Vm_Idle(void);
u8 *U3D_Vm_WordSlot(u8 slot);

#define PROPS 100
#define SLOTS 9        // Black's body slots: 0 / 1 ears, 2 head, 3 eyes, 4 face, 5 body, 6 waist, 7 / 8 hands
#define NO_PROP 0xFF
#define SHOWS 4
#define FANS 10
#define STAGE_FRAMES (60 * 25)
#define APPEALS 2      // (Black's: two props used during a show count, STA_SLOT_GetSub)

#define BANK_CATEGORY 75   // ("cool", "cute", "elegant", "unique", "quirky"...)
#define BANK_PROP 79       // (a prop's name: prop + 3)
#define BANK_PROP_ART 282  // ("a Pink Barrette")
#define BANK_NPC 76        // (an NPC performer's trainer name)
#define BANK_NPC_MON 77    // (... and its POKeMON's)

// ------------------------------------------------------------------------------------------------- the pack
struct Prop
{
    u8 w, h, pos, cat;
    u16 bits;
    s8 ox, oy;
    const u8 *px;   // (w * h ARGB, little-endian)
};
struct Rival
{
    u16 national;
    u8 portrait, nameId;
    u16 appeal, level;
    u16 prop[8];
    u8 slot[8];
};
struct Show
{
    u8 bonus[4], total, row;
    char title[48];
    u8 rivals;
    struct Rival r[8];
};
static struct
{
    bool8 tried, ok;
    u8 *data;
    struct Prop p[PROPS];
    struct Show s[SHOWS];
    u16 threshold[10];
    u8 row[7][40];
} P;

static u16 Le16(const u8 *p) { return p[0] | p[1] << 8; }

static bool8 Load(void)
{
    FILE *f;
    long size;
    const u8 *p, *end;
    u32 i, k, props, shows;
    if (P.tried)
        return P.ok;
    P.tried = TRUE;
    f = fopen(Updater_ExeFile("unova/black_musical.pak"), "rb");
    if (f == NULL)
    {
        MPNet_Log("musical: no unova/black_musical.pak (tools/unova/make_pack.sh)");
        return FALSE;
    }
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);
    P.data = malloc(size);
    if (P.data == NULL || fread(P.data, 1, size, f) != (size_t)size || size < 10 || memcmp(P.data, "UMUS", 4) != 0)
    {
        fclose(f);
        free(P.data);
        P.data = NULL;
        return FALSE;
    }
    fclose(f);
    p = P.data + 6;
    end = P.data + size;
    props = Le16(p), shows = Le16(p + 2);
    p += 4;
    for (i = 0; i < props && p + 8 <= end; i++)
    {
        struct Prop *q = &P.p[i < PROPS ? i : PROPS - 1];
        q->w = p[0], q->h = p[1], q->pos = p[2], q->cat = p[3] & 3, q->bits = Le16(p + 4), q->ox = (s8)p[6], q->oy = (s8)p[7];
        q->px = p + 8;
        p += 8 + q->w * q->h * 4;
    }
    for (i = 0; i < shows && i < SHOWS && p + 7 <= end; i++)
    {
        struct Show *s = &P.s[i];
        u8 n;
        memcpy(s->bonus, p, 4);
        s->total = p[4], s->row = p[5];
        n = p[6];
        memcpy(s->title, p + 7, n < sizeof(s->title) - 1 ? n : sizeof(s->title) - 1);
        p += 7 + n;
        s->rivals = *p++;
        for (k = 0; k < s->rivals && p + 8 + 24 <= end; k++)
        {
            struct Rival *r = &s->r[k < 8 ? k : 7];
            u32 j;
            r->national = Le16(p), r->portrait = p[2], r->nameId = p[3], r->appeal = Le16(p + 4), r->level = Le16(p + 6);
            for (j = 0; j < 8; j++)
                r->prop[j] = Le16(p + 8 + 3 * j), r->slot[j] = p[10 + 3 * j];
            p += 8 + 24;
        }
        if (s->rivals > 8)
            s->rivals = 8;
    }
    if (p + 20 + 280 <= end)
    {
        for (i = 0; i < 10; i++)
            P.threshold[i] = Le16(p + 2 * i);
        memcpy(P.row, p + 20, sizeof(P.row));
    }
    P.ok = TRUE;
    MPNet_Log("musical: %u props, %u shows from Black's pack", props, shows);
    return TRUE;
}

// ------------------------------------------------------------------------------------------------- the save
struct UnovaMusicalSave
{
    u8 owned[13];
    u8 equip[4];        // the last show's props by category (MUSICAL_SAVE_SetEquippedProp)
    u8 setting;         // the script's setting (GetValue 4 / 5: the show picked at the counter)
    u8 shot;            // (Black's musical photo: none kept here)
    u8 selected;        // the last show's score (SetSelectedProgram)
    u16 shows, tops, fanPoints;
    struct { u8 state, rank, prop, pad; } fans[FANS];
    u8 outfit[SLOTS];   // the last dress-up's props, a body slot each (NO_PROP + 1 = 0: none)
};
STATIC_ASSERT(sizeof(struct UnovaMusicalSave) <= 96, MusicalSaveFits);

static struct UnovaMusicalSave *Save(void)
{
    return U3D_Vm_MusicalSave();
}

static bool8 Owned(u8 prop)
{
    return prop < PROPS && (Save()->owned[prop / 8] >> (prop % 8) & 1);
}

// ------------------------------------------------------------------------------------------------- performers
enum { PK_ME, PK_FRIEND, PK_NPC };
struct Performer
{
    u16 species;
    u32 pid, otId;
    u8 kind, rival, player;
    char trainer[24], mon[24];
    u8 prop[SLOTS], used[SLOTS], appealsLeft;
    u16 score, nextAppeal;
    u8 appeal[4];
    u16 hop, hearts;
};

// Friends' latest dress-ups (relay MPR_MUSICAL): their POKeMON and props
struct FriendEntry
{
    bool8 valid;
    u16 species;
    u32 pid, otId;
    u8 prop[SLOTS];
    u8 nick[POKEMON_NAME_LENGTH + 1];
};
static struct FriendEntry sFriend[MP_MAX_PLAYERS];

enum { MS_NONE, MS_INTRO, MS_INTRO_WAIT, MS_DRESS, MS_PRESTAGE, MS_PRESTAGE_WAIT, MS_STAGE, MS_RESULTS, MS_AFTER,
       MS_AFTER_WAIT, MS_THANKS, MS_THANKS_WAIT };
static struct
{
    bool8 active, practice, dressDone;
    u8 state, partySlot, show, local;
    u8 rank[4], bonus[4], share[4], picks[3];
    struct Performer p[4];
    u16 t;
} M;

static void Ascii(char *out, int n, const u8 *game)
{
    MP_GameToAscii(out, game, n);
}

static void BlackAscii(char *out, int n, u16 bank, u16 msg)
{
    MP_GameToAscii(out, U3D_Text(bank, msg), n);
}

static void SetPerformerFromMon(struct Performer *e, struct Pokemon *mon)
{
    u8 nick[POKEMON_NAME_LENGTH + 2];
    e->species = GetMonData(mon, MON_DATA_SPECIES);
    e->pid = GetMonData(mon, MON_DATA_PERSONALITY);
    e->otId = GetMonData(mon, MON_DATA_OT_ID);
    GetMonData(mon, MON_DATA_NICKNAME, nick);
    nick[POKEMON_NAME_LENGTH] = EOS;
    StringGet_Nickname(nick);
    Ascii(e->mon, sizeof(e->mon), nick);
    Ascii(e->trainer, sizeof(e->trainer), gSaveBlock2Ptr->playerName);
}

// ------------------------------------------------------------------------------------------------- textures
static struct
{
    SDL_Renderer *owner;
    SDL_Texture *prop[PROPS];
    SDL_Texture *mon[5];
    u32 monKey[5];
    u8 box[5][4];   // (the picture's outline: x0, y0, x1, y1)
} T;

static void Forget(SDL_Renderer *r)
{
    if (T.owner == r)
        return;
    memset(&T, 0, sizeof(T));
    T.owner = r;
}

static SDL_Texture *PropTexture(u8 id)
{
    struct Prop *q;
    Forget(G_R());
    if (id >= PROPS || !Load() || P.p[id].px == NULL)
        return NULL;
    if (T.prop[id])
        return T.prop[id];
    q = &P.p[id];
    T.prop[id] = SDL_CreateTexture(G_R(), SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, q->w, q->h);
    if (T.prop[id])
    {
        SDL_UpdateTexture(T.prop[id], NULL, q->px, q->w * 4);
        SDL_SetTextureBlendMode(T.prop[id], SDL_BLENDMODE_BLEND);
        SDL_SetTextureScaleMode(T.prop[id], SDL_ScaleModeNearest);
    }
    return T.prop[id];
}

static u32 Argb(u16 c)
{
    u32 r = (c & 31) * 255 / 31, g = (c >> 5 & 31) * 255 / 31, b = (c >> 10 & 31) * 255 / 31;
    return 0xFF000000u | r << 16 | g << 8 | b;
}

// The POKeMON's front picture (the game's own, its shiny / form palette) and its outline, cached by slot K
static SDL_Texture *MonTexture(u8 k, u16 species, u32 pid, u32 otId)
{
    static u8 tiles[MON_PIC_SIZE * 4];
    static u16 pal[64];
    static u32 px[64 * 64];
    u32 key = species * 2654435761u ^ pid ^ otId * 40503u;
    int x, y, x0 = 64, y0 = 64, x1 = 0, y1 = 0;
    Forget(G_R());
    if (species == SPECIES_NONE || species >= NUM_SPECIES)
        return NULL;
    if (T.mon[k] && T.monKey[k] == key)
        return T.mon[k];
    if (T.mon[k])
        SDL_DestroyTexture(T.mon[k]);
    T.mon[k] = NULL;
    T.monKey[k] = key;
    HandleLoadSpecialPokePic(&gMonFrontPicTable[species], tiles, species, pid);
    LZ77UnCompWram(GetMonSpritePalFromSpeciesAndPersonality(species, otId, pid), pal);
    for (y = 0; y < 64; y++)
        for (x = 0; x < 64; x++)
        {
            u8 b = tiles[((y / 8) * 8 + x / 8) * 32 + (y % 8) * 4 + (x % 8) / 2];
            u8 v = (x & 1) ? b >> 4 : b & 15;
            px[y * 64 + x] = v ? Argb(pal[v]) : 0;
            if (v)
            {
                x0 = x < x0 ? x : x0, y0 = y < y0 ? y : y0;
                x1 = x > x1 ? x : x1, y1 = y > y1 ? y : y1;
            }
        }
    if (x1 < x0)
        x0 = 16, y0 = 8, x1 = 48, y1 = 63;
    T.box[k][0] = x0, T.box[k][1] = y0, T.box[k][2] = x1, T.box[k][3] = y1;
    T.mon[k] = SDL_CreateTexture(G_R(), SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, 64, 64);
    if (T.mon[k])
    {
        SDL_UpdateTexture(T.mon[k], NULL, px, 64 * 4);
        SDL_SetTextureBlendMode(T.mon[k], SDL_BLENDMODE_BLEND);
        SDL_SetTextureScaleMode(T.mon[k], SDL_ScaleModeNearest);
    }
    return T.mon[k];
}

// A body slot's point on the picture (Black's MCSS points, found here from the POKeMON's outline)
static void Point(u8 k, u8 slot, int *x, int *y)
{
    static const u8 sFx[SLOTS] = {75, 25, 50, 50, 50, 50, 50, 88, 12};   // (percent of the outline)
    static const u8 sFy[SLOTS] = {8, 8, 4, 22, 32, 56, 72, 62, 62};
    int x0 = T.box[k][0], y0 = T.box[k][1], w = T.box[k][2] - x0 + 1, h = T.box[k][3] - y0 + 1;
    *x = x0 + w * sFx[slot] / 100;
    *y = y0 + h * sFy[slot] / 100;
}

// The POKeMON with its props, its feet at (X, FEET): 64 x 64, as the game draws it
static void DrawDressed(u8 k, u16 species, u32 pid, u32 otId, const u8 *prop, int x, int feet, int hop)
{
    static const u8 sOrder[SLOTS] = {5, 6, 4, 3, 0, 1, 2, 7, 8};   // (back to front)
    SDL_Texture *t = MonTexture(k, species, pid, otId);
    int left = x - 32, top = feet - 64 - hop, i, pass;
    if (t == NULL)
        return;
    for (pass = 0; pass < 2; pass++)   // (capes and wings behind the POKeMON, the rest in front)
    {
        if (pass == 1)
        {
            SDL_Rect d = {left, top, 64, 64};
            SDL_RenderCopy(G_R(), t, NULL, &d);
        }
        for (i = 0; i < SLOTS; i++)
        {
            u8 s = sOrder[i], id = prop[s];
            struct Prop *q;
            SDL_Texture *pt;
            int px, py;
            bool8 behind;
            if (id >= PROPS || (pt = PropTexture(id)) == NULL)
                continue;
            q = &P.p[id];
            behind = s == 5 && (q->bits & 0x80);
            if (behind != (pass == 0))
                continue;
            Point(k, s, &px, &py);
            px = left + (s == 8 ? px - q->ox : px + q->ox);
            py = top + py - q->oy;
            {
                SDL_Rect d = {px - q->w / 2, py - q->h / 2, q->w, q->h};
                SDL_RenderCopyEx(G_R(), pt, NULL, &d, 0, NULL, s == 8 ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
            }
        }
    }
}

// ------------------------------------------------------------------------------------------------- scoring (Black's)
static void ProgramCreate(void)
{
    struct Show *s = &P.s[M.show];
    u8 buf[8], n = 0, i, left;
    for (i = 0; i < 4; i++)
        M.bonus[i] = s->bonus[i];
    left = s->total / 5;
    while (left)
    {
        u8 chunk = Random() % 3 + 1;
        if (chunk > left)
            chunk = left;
        M.bonus[Random() % 4] += chunk * 5;
        left -= chunk;
    }
    for (i = 0; i < s->rivals && i < 6; i++)
        if (s->r[i].level <= Save()->fanPoints)
            buf[n++] = i;
    if (n < 3)
        buf[0] = 0, buf[1] = 1, buf[2] = 2, n = 3;
    for (i = 0; i < 30; i++)
    {
        u8 a = Random() % n, b = Random() % n, t = buf[a];
        buf[a] = buf[b], buf[b] = t;
    }
    memcpy(M.picks, buf, 3);
}

static void Score(void)
{
    u8 inCat[4] = {0}, mine[4][4] = {{0}}, i, j;
    for (i = 0; i < 4; i++)
        for (j = 0; j < SLOTS; j++)
            if (M.p[i].prop[j] < PROPS)
                mine[i][P.p[M.p[i].prop[j]].cat]++, inCat[P.p[M.p[i].prop[j]].cat]++;
    for (j = 0; j < 4; j++)
        M.share[j] = inCat[j] ? M.bonus[j] / inCat[j] : 0;
    for (i = 0; i < 4; i++)
    {
        M.p[i].score = 0;
        for (j = 0; j < 4; j++)
        {
            M.p[i].appeal[j] = M.share[j] * mine[i][j];
            M.p[i].score += M.p[i].appeal[j];
        }
        for (j = 0; j < SLOTS; j++)   // (ScorePerformers: a prop used on stage earns its share again)
            if (M.p[i].used[j] && M.p[i].prop[j] < PROPS)
                M.p[i].score += M.share[P.p[M.p[i].prop[j]].cat];
    }
    for (i = 0; i < 4; i++)
        M.rank[i] = i;
    for (i = 0; i < 4; i++)
        for (j = i + 1; j < 4; j++)
            if (M.p[M.rank[j]].score > M.p[M.rank[i]].score)
            {
                u8 t = M.rank[i];
                M.rank[i] = M.rank[j], M.rank[j] = t;
            }
    for (i = 0; i < 4; i++)
        MPNet_Log("musical: %s's %s scored %u", M.p[i].trainer, M.p[i].mon, M.p[i].score);
}

static u8 ScoreCapped(u8 slot)
{
    return slot < 4 ? (M.p[slot].score < 0x100 ? M.p[slot].score : 0xFF) : 0;
}

// RollProgramRewards and ApplyResults: the fans waiting with props, the records
static void Results(void)
{
    struct UnovaMusicalSave *s = Save();
    u8 local = ScoreCapped(M.local), top = ScoreCapped(M.rank[0]), level = 0, cand[40], nc = 0, i, j, shuffle[FANS];
    u32 sum = s->fanPoints + local, best = 0, cat = 0;
    struct Show *show = &P.s[M.show];
    for (i = 0; i < FANS; i++)
        s->fans[i].state = 4, s->fans[i].rank = 0, s->fans[i].prop = 0;
    for (i = 0; i < 10; i++)
        if (P.threshold[i] <= sum)
        {
            level = 10 - i;
            break;
        }
    if (show->row < 7)
        for (j = 0; j < 40; j++)
            if (!Owned(P.row[show->row][j]))
                cand[nc++] = P.row[show->row][j];
    for (i = 0; i < 4; i++)
        if (best < M.bonus[i])
            best = M.bonus[i], cat = i;
    for (i = 0; i < FANS; i++)
        shuffle[i] = i;
    for (i = 0; i < 100; i++)
    {
        u8 a = i % FANS, b = Random() % FANS, t = shuffle[a];
        shuffle[a] = shuffle[b], shuffle[b] = t;
    }
    for (i = 0; i < level && i < FANS; i++)
    {
        u8 f = shuffle[i];
        s->fans[f].state = cat;
        if (Random() % 100 < local && nc)
        {
            u8 pick = Random() % nc;
            s->fans[f].rank = 1, s->fans[f].prop = cand[pick];
            memmove(cand + pick, cand + pick + 1, nc - pick - 1);
            nc--;
        }
    }
    if (s->shows < 0xFFFF)
        s->shows++;
    if (local == top && local != 0 && s->tops < 0xFFFF)
        s->tops++;
    s->selected = local;
    s->fanPoints = s->fanPoints + local > 0xFFFF ? 0xFFFF : s->fanPoints + local;
    memset(s->equip, 0, sizeof(s->equip));
    for (j = 0; j < SLOTS; j++)
        if (M.p[M.local].prop[j] < PROPS)
            s->equip[P.p[M.p[M.local].prop[j]].cat]++;
    MPNet_Log("musical: results: rank %u, score %u, fan points %u, %u fans", 0, local, s->fanPoints, level);
}

// ------------------------------------------------------------------------------------------------- DRESS UP
enum { TAB_HEAD, TAB_FACE, TAB_BODY, TAB_HAND, TABS };
static struct
{
    u8 tab, cursor, top;
    u8 list[PROPS], count;
    u16 frames;
} D;

static u8 TabOf(u8 prop)
{
    static const u8 sTab[7] = {TAB_HEAD, TAB_HEAD, TAB_FACE, TAB_FACE, TAB_BODY, TAB_BODY, TAB_HAND};
    return sTab[P.p[prop].pos < 7 ? P.p[prop].pos : 0];
}

static void BuildList(void)
{
    u8 i;
    D.count = 0;
    for (i = 0; i < PROPS; i++)
        if (Owned(i) && TabOf(i) == D.tab)
            D.list[D.count++] = i;
    if (D.cursor >= D.count)
        D.cursor = D.count ? D.count - 1 : 0;
}

static s8 WornAt(u8 prop)
{
    u8 i;
    for (i = 0; i < SLOTS; i++)
        if (M.p[M.local].prop[i] == prop)
            return i;
    return -1;
}

// The body slots a prop may go in (Black's position bits), its own place first
static u8 Slots(u8 prop, u8 *out)
{
    static const u8 sMain[7][2] = {{0, 1}, {2, 0xFF}, {3, 0xFF}, {4, 0xFF}, {5, 0xFF}, {6, 0xFF}, {7, 8}};
    static const u8 sBit[9][2] = {{0, 1}, {2, 0xFF}, {3, 0xFF}, {4, 0xFF}, {5, 0xFF}, {6, 0xFF}, {7, 8}, {5, 0xFF},
                                  {0xFF, 0xFF}};
    u8 n = 0, b, k;
    struct Prop *q = &P.p[prop];
    for (k = 0; k < 2; k++)
        if (q->pos < 7 && sMain[q->pos][k] != 0xFF)
            out[n++] = sMain[q->pos][k];
    for (b = 0; b < 9; b++)
        if (q->bits >> b & 1)
            for (k = 0; k < 2; k++)
            {
                u8 s = sBit[b][k], j;
                bool8 have = FALSE;
                if (s == 0xFF)
                    continue;
                for (j = 0; j < n; j++)
                    have |= out[j] == s;
                if (!have)
                    out[n++] = s;
            }
    return n;
}

static void Wear(u8 prop)
{
    u8 slots[12], n = Slots(prop, slots), i;
    s8 at = WornAt(prop);
    if (at >= 0)   // (worn: taken off)
    {
        M.p[M.local].prop[at] = NO_PROP;
        PlaySE(SE_BALL_TRAY_EXIT);
        return;
    }
    for (i = 0; i < n; i++)
        if (M.p[M.local].prop[slots[i]] == NO_PROP)
        {
            M.p[M.local].prop[slots[i]] = prop;
            PlaySE(SE_BALL_TRAY_ENTER);
            return;
        }
    if (n)   // (every place it fits taken: it takes its own place's)
    {
        M.p[M.local].prop[slots[0]] = prop;
        PlaySE(SE_BALL_TRAY_ENTER);
    }
}

static void DressOpen(void)
{
    memset(&D, 0, sizeof(D));
    BuildList();
}

#define DR_BG 0xf0e0d0
#define DR_HEAD 0xb04060
#define DR_HEAD_SH 0x601830
#define DR_PANEL 0xd8e4f0
#define DR_CELL 0xf8f8f8
#define DR_TAB_ON 0xb04060
#define DR_TAB_OFF 0xf0d8e0

static void DressDraw(void)
{
    static const char *const sTabs[TABS] = {"HEAD", "FACE", "BODY", "HAND"};
    struct Performer *e = &M.p[M.local];
    char line[96];
    int i, y;
    for (y = 0; y < 160; y += 4)
        G_Fill(0, y, 240, 4, G_Mix(0xf8ece0, 0xe8d4c4, y / 160.0f));
    G_Fill(0, 0, 240, 16, DR_HEAD);
    G_Text(6, 0, G_WHITE, DR_HEAD_SH, "DRESS UP", 160, TF_NORMAL);
    if (!M.practice && Load())
        G_TextAlign(234, 2, TA_RIGHT, G_WHITE, DR_HEAD_SH, P.s[M.show].title, TF_SMALL);
    // the POKeMON in its frame on the stage's floor
    G_RoundFill(8, 20, 92, 88, 0xf8f8f8, 255);
    G_RoundFrame(8, 20, 92, 88, 0x8890a0, 255);
    G_RoundFill(11, 23, 86, 82, DR_PANEL, 255);
    G_Fill(16, 86, 77, 6, 0xc0a088);
    G_Fill(18, 92, 73, 2, 0x987860);
    DrawDressed(0, e->species, e->pid, e->otId, e->prop, 54, 90, 0);
    G_TextAlign(54, 93, TA_CENTER, G_INK, G_INK_SHADOW, e->mon, TF_SMALL);
    // the props: four tabs, a grid of four by three
    G_Window(104, 20, 132, 90);
    for (i = 0; i < TABS; i++)
    {
        G_Fill(110 + i * 30, 26, 28, 11, i == D.tab ? DR_TAB_ON : DR_TAB_OFF);
        G_TextAlign(124 + i * 30, 26, TA_CENTER, i == D.tab ? G_WHITE : G_INK, i == D.tab ? DR_HEAD_SH : G_INK_SHADOW,
                    sTabs[i], TF_SMALL);
    }
    for (i = 0; i < 12 && D.top + i < D.count; i++)
    {
        u8 id = D.list[D.top + i];
        int cx = 112 + (i % 4) * 30, cy = 40 + (i / 4) * 22;
        SDL_Texture *t = PropTexture(id);
        G_RoundFill(cx, cy, 26, 21, WornAt(id) >= 0 ? 0xf8e0e8 : DR_CELL, 255);
        G_RoundFrame(cx, cy, 26, 21, D.top + i == D.cursor ? DR_TAB_ON : 0xc8c0c8, 255);
        if (t)
        {
            struct Prop *q = &P.p[id];
            int w = q->w / 2, h = q->h / 2;
            SDL_Rect d = {cx + 13 - w / 2, cy + 10 - h / 2, w, h};
            SDL_RenderCopy(G_R(), t, NULL, &d);
        }
        if (D.top + i == D.cursor)
            G_RoundFrame(cx - 1, cy - 1, 28, 23, DR_TAB_ON, 255);
    }
    if (D.count > 12)
    {
        if (D.top > 0)
            G_Arrow(167, 38, 0, DR_TAB_ON);
        if (D.top + 12 < D.count)
            G_Arrow(167, 105, 1, DR_TAB_ON);
    }
    if (D.count && D.cursor < D.count)
        BlackAscii(line, sizeof(line), BANK_PROP, D.list[D.cursor] + 3);
    else
        snprintf(line, sizeof(line), "-");
    Unova_Screen_Message(line, "A: PUT ON / TAKE OFF   START: DONE");
}

static void DressStep(u16 pressed, u16 keys)
{
    (void)keys;
    if (pressed & (L_BUTTON | R_BUTTON))
    {
        D.tab = (D.tab + (pressed & R_BUTTON ? 1 : TABS - 1)) % TABS;
        D.cursor = D.top = 0;
        BuildList();
        PlaySE(SE_SELECT);
    }
    else if (pressed & DPAD_LEFT && D.cursor % 4 > 0)
        D.cursor--, PlaySE(SE_SELECT);
    else if (pressed & DPAD_RIGHT && D.cursor % 4 < 3 && D.cursor + 1 < D.count)
        D.cursor++, PlaySE(SE_SELECT);
    else if (pressed & DPAD_UP && D.cursor >= 4)
        D.cursor -= 4, PlaySE(SE_SELECT);
    else if (pressed & DPAD_DOWN && D.cursor + 4 < D.count)
        D.cursor += 4, PlaySE(SE_SELECT);
    else if (pressed & A_BUTTON && D.cursor < D.count)
        Wear(D.list[D.cursor]);
    else if (pressed & (START_BUTTON | B_BUTTON))
    {
        struct UnovaMusicalSave *s = Save();
        u8 i;
        PlaySE(SE_SELECT);
        for (i = 0; i < SLOTS; i++)
            s->outfit[i] = M.p[M.local].prop[i] < PROPS ? M.p[M.local].prop[i] + 1 : 0;
        M.dressDone = TRUE;
        {
            // (friends hear our dress-up: they can have it on their stage, MPR_MUSICAL)
            u8 b[2 + 2 + 8 + SLOTS + POKEMON_NAME_LENGTH + 1], nick[POKEMON_NAME_LENGTH + 2];
            struct Performer *e = &M.p[M.local];
            b[0] = MPR_MUSICAL, b[1] = 0, b[2] = e->species, b[3] = e->species >> 8;
            memcpy(b + 4, &e->pid, 4), memcpy(b + 8, &e->otId, 4), memcpy(b + 12, e->prop, SLOTS);
            GetMonData(&gPlayerParty[M.partySlot < PARTY_SIZE ? M.partySlot : 0], MON_DATA_NICKNAME, nick);
            memcpy(b + 12 + SLOTS, nick, POKEMON_NAME_LENGTH);
            b[12 + SLOTS + POKEMON_NAME_LENGTH] = EOS;
            if (MP_IsConnected())
                MP_SendRelay(0xFF, b, sizeof(b));
        }
        Unova_Screen_Close(NULL);
    }
    if (D.cursor < D.top)
        D.top = D.cursor / 4 * 4;
    if (D.cursor >= D.top + 12)
        D.top = D.cursor / 4 * 4 - 8;
}

static const struct UnovaScreen sDress = {"MUSICAL DRESS UP", DressOpen, DressDraw, DressStep};

// ------------------------------------------------------------------------------------------------- the STAGE
static void StageOpen(void)
{
    u8 i;
    M.t = 0;
    for (i = 0; i < 4; i++)
    {
        M.p[i].appealsLeft = APPEALS;
        M.p[i].hop = M.p[i].hearts = 0;
        memset(M.p[i].used, 0, SLOTS);
        M.p[i].nextAppeal = 240 + Random() % 600;
    }
}

static void Appeal(u8 k)
{
    struct Performer *e = &M.p[k];
    u8 j;
    if (e->appealsLeft == 0)
        return;
    for (j = 0; j < SLOTS; j++)
        if (e->prop[j] < PROPS && !e->used[j])
        {
            e->used[j] = 1;
            e->appealsLeft--;
            e->hop = 16;
            e->hearts = 48;
            if (k == M.local)
                PlaySE(SE_SHINY);
            return;
        }
    e->appealsLeft = 0;
}

#define ST_CURTAIN 0xa82030
#define ST_CURTAIN_DARK 0x701420
#define ST_BACK 0x281830
#define ST_FLOOR 0xb08860
#define ST_FLOOR_EDGE 0x806040

static void StageDraw(void)
{
    int i, y;
    G_Fill(0, 0, 240, 160, ST_BACK);
    for (i = 0; i < 3; i++)   // (the lights, sweeping)
    {
        int cx = 40 + i * 80 + (int)(30 * SDL_sinf(M.t / 40.0f + i * 2.1f));
        for (y = 0; y < 120; y += 4)
            G_FillA(cx - 6 - y / 5, y, 12 + 2 * (y / 5), 4, 0xfff8d0, 40);
    }
    G_Fill(0, 112, 240, 24, ST_FLOOR);
    G_Fill(0, 112, 240, 2, ST_FLOOR_EDGE);
    for (i = 0; i < 4; i++)
    {
        struct Performer *e = &M.p[i];
        int x = 40 + i * 53, hop = e->hop > 8 ? 16 - e->hop : e->hop;
        DrawDressed(i, e->species, e->pid, e->otId, e->prop, x, 122, hop);
        if (e->hearts)
        {
            int hy = 40 - (48 - e->hearts) / 2;
            G_Circle(x - 2, hy, 2.5f, 0xf05890, 200);
            G_Circle(x + 2, hy, 2.5f, 0xf05890, 200);
            G_FillA(x - 3, hy, 6, 4, 0xf05890, 200);
        }
        G_Window(x - 25, 128, 50, 16);
        G_TextAlign(x, 129, TA_CENTER, G_INK, G_INK_SHADOW, e->trainer, TF_SMALL);
    }
    G_Fill(0, 0, 14, 160, ST_CURTAIN), G_Fill(226, 0, 14, 160, ST_CURTAIN);
    G_Fill(10, 0, 4, 160, ST_CURTAIN_DARK), G_Fill(226, 0, 4, 160, ST_CURTAIN_DARK);
    G_Fill(0, 0, 240, 10, ST_CURTAIN);
    if (Load())
        G_TextAlign(120, 10, TA_CENTER, 0xf8d878, 0x502818, P.s[M.show].title, TF_NORMAL);
    G_Fill(0, 146, 240, 14, 0x181018);
    if (M.p[M.local].appealsLeft)
        G_TextAlign(234, 147, TA_RIGHT, G_WHITE, 0x403040, "A: SHOW OFF!", TF_SMALL);
}

static void StageStep(u16 pressed, u16 keys)
{
    u8 i;
    (void)keys;
    M.t++;
    for (i = 0; i < 4; i++)
    {
        if (M.p[i].hop)
            M.p[i].hop--;
        if (M.p[i].hearts)
            M.p[i].hearts--;
        if (i != M.local && M.p[i].appealsLeft && M.t >= M.p[i].nextAppeal)
        {
            Appeal(i);
            M.p[i].nextAppeal = M.t + 300 + Random() % 500;
        }
    }
    if (pressed & A_BUTTON)
        Appeal(M.local);
    if (M.t >= STAGE_FRAMES)
        Unova_Screen_Close(NULL);
}

static const struct UnovaScreen sStage = {"MUSICAL STAGE", StageOpen, StageDraw, StageStep};

// ------------------------------------------------------------------------------------------------- RESULTS
#define RS_HEAD 0xb04060
#define RS_ROW 0xf0e8f0
#define RS_FIRST 0xf8e8e8
#define RS_HEART 0xe85888
#define RS_HEART_OFF 0xd0c8d0

static void ResultsOpen(void)
{
    M.t = 0;
}

static void ResultsDraw(void)
{
    u8 i, k;
    u16 top = M.p[M.rank[0]].score;
    G_Fill(0, 0, 240, 160, 0x483048);
    G_Fill(0, 0, 240, 16, RS_HEAD);
    G_TextAlign(120, 0, TA_CENTER, G_WHITE, 0x601830, "RESULTS", TF_NORMAL);
    for (i = 0; i < 4; i++)
    {
        struct Performer *e = &M.p[M.rank[i]];
        int y = 20 + i * 23, hearts = top ? (e->score * 5 + top / 2) / top : 0;
        char n[4];
        SDL_Texture *t = MonTexture(M.rank[i], e->species, e->pid, e->otId);
        if (M.t < 20 * (4 - i))   // (from the last place up)
            continue;
        G_RoundFill(6, y, 228, 21, i == 0 ? RS_FIRST : RS_ROW, 255);
        snprintf(n, sizeof(n), "%u", i + 1);
        G_Text(12, y + 3, i == 0 ? 0xc03040 : G_INK, G_INK_SHADOW, n, 12, TF_NORMAL);
        if (t)
        {
            SDL_Rect d = {24, y - 6, 32, 32};
            SDL_RenderCopy(G_R(), t, NULL, &d);
        }
        G_Text(58, y + 3, G_INK, G_INK_SHADOW, e->trainer, 64, TF_NORMAL);
        for (k = 0; k < 5; k++)
        {
            unsigned c = k < hearts ? RS_HEART : RS_HEART_OFF;
            int hx = 126 + k * 14, hy = y + 8;
            G_Circle(hx - 2, hy, 2.5f, c, 255), G_Circle(hx + 2, hy, 2.5f, c, 255);
            G_Fill(hx - 4, hy, 9, 2, c), G_Fill(hx - 3, hy + 2, 7, 2, c), G_Fill(hx - 1, hy + 4, 3, 2, c);
        }
        if (i == 0)
        {
            G_Fill(208, y + 6, 16, 9, 0xe8b830);
            G_Fill(208, y + 3, 3, 4, 0xe8b830), G_Fill(214, y + 3, 4, 4, 0xe8b830), G_Fill(221, y + 3, 3, 4, 0xe8b830);
        }
    }
    if (M.t >= 100)
        Unova_Screen_Message(Load() ? P.s[M.show].title : "", "A: CONTINUE");
}

static void ResultsStep(u16 pressed, u16 keys)
{
    (void)keys;
    M.t++;
    if (M.t >= 100 && pressed & (A_BUTTON | B_BUTTON))
    {
        PlaySE(SE_SELECT);
        Unova_Screen_Close(NULL);
    }
}

static const struct UnovaScreen sResults = {"MUSICAL RESULTS", ResultsOpen, ResultsDraw, ResultsStep};

// ------------------------------------------------------------------------------------------------- the show
static void SetUpPerformers(void)
{
    struct Show *s = &P.s[M.show];
    u8 i, k = 0, friends[MP_MAX_PLAYERS], nf = 0;
    memset(M.p, 0, sizeof(M.p));
    M.local = 0;
    SetPerformerFromMon(&M.p[0], &gPlayerParty[M.partySlot]);
    M.p[0].kind = PK_ME;
    for (i = 0; i < SLOTS; i++)
        M.p[0].prop[i] = Save()->outfit[i] && Owned(Save()->outfit[i] - 1) ? Save()->outfit[i] - 1 : NO_PROP;
    if (MP_IsConnected())
        for (i = 0; i < MP_MAX_PLAYERS; i++)
            if (i != gMpLocalId && gMpPlayers[i].present && sFriend[i].valid)
                friends[nf++] = i;
    for (i = 1; i < 4; i++)
    {
        struct Performer *e = &M.p[i];
        if (nf)
        {
            u8 id = friends[--nf];
            e->kind = PK_FRIEND, e->player = id;
            e->species = sFriend[id].species, e->pid = sFriend[id].pid, e->otId = sFriend[id].otId;
            memcpy(e->prop, sFriend[id].prop, SLOTS);
            MP_GameToAscii(e->trainer, gMpPlayers[id].name, sizeof(e->trainer));
            Ascii(e->mon, sizeof(e->mon), sFriend[id].nick);
        }
        else
        {
            const struct Rival *r = &s->r[M.picks[k] < s->rivals ? M.picks[k] : 0];
            u8 j;
            e->kind = PK_NPC, e->rival = M.picks[k++];
            e->species = r->national ? NationalPokedexNumToSpecies(r->national) : SPECIES_PATRAT;
            e->pid = Random32(), e->otId = Random32();
            memset(e->prop, NO_PROP, SLOTS);
            for (j = 0; j < 8; j++)
                if (r->prop[j] < PROPS && r->slot[j] < SLOTS)
                    e->prop[r->slot[j]] = r->prop[j];
            BlackAscii(e->trainer, sizeof(e->trainer), BANK_NPC, r->nameId);
            BlackAscii(e->mon, sizeof(e->mon), BANK_NPC_MON, r->nameId);
        }
    }
}

// u3d_vm.c MUSICAL_StartMusical (single; Black's link mode the same here, friends joining by their dress-ups): the
// POKeMON of party slot SLOT. The calling script ends; the show goes on from the field frames (Unova_Musical_FieldFrame)
bool8 Unova_Musical_Begin(u8 slot)
{
    if (!Load() || slot >= PARTY_SIZE || GetMonData(&gPlayerParty[slot], MON_DATA_SPECIES) == SPECIES_NONE)
        return FALSE;
    memset(&M, 0, sizeof(M));
    M.active = TRUE;
    M.partySlot = slot;
    M.show = Save()->setting % SHOWS;
    ProgramCreate();
    SetUpPerformers();
    M.state = MS_INTRO;
    MPNet_Log("musical: %s with %s (show %u, picks %u %u %u)", P.s[M.show].title, M.p[0].mon, M.show, M.picks[0],
              M.picks[1], M.picks[2]);
    return TRUE;
}

// u3d_vm.c MUSICAL_ShowPartyMonMusical: the dressing room for practice (no show)
bool8 Unova_Musical_Practice(u8 slot)
{
    if (!Load() || slot >= PARTY_SIZE || GetMonData(&gPlayerParty[slot], MON_DATA_SPECIES) == SPECIES_NONE)
        return FALSE;
    memset(&M, 0, sizeof(M));
    M.practice = TRUE;
    M.partySlot = slot;
    SetPerformerFromMon(&M.p[0], &gPlayerParty[slot]);
    {
        u8 i;
        for (i = 0; i < SLOTS; i++)
            M.p[0].prop[i] = Save()->outfit[i] && Owned(Save()->outfit[i] - 1) ? Save()->outfit[i] - 1 : NO_PROP;
    }
    Unova_Screen_Open(&sDress);
    return TRUE;
}

bool8 Unova_Musical_Busy(void)
{
    return M.active || (M.practice && Unova_Screen_IsActive());
}

// Each field frame (u3d_screen.c Unova_Screen_FieldFrame): the show's next step when the last is done
void Unova_Musical_FieldFrame(void)
{
    if (!M.active || Unova_Screen_IsActive())
        return;
    switch (M.state)
    {
    case MS_INTRO:   // (Black's MUSICAL_EVENT_ShowMessage(3): the zone's script introducing the show)
        if (U3D_Vm_Idle())
            M.state = U3D_Vm_RunScene(3) ? MS_INTRO_WAIT : MS_DRESS;
        break;
    case MS_INTRO_WAIT:
        if (U3D_Vm_Idle())
        {
            M.dressDone = FALSE;
            Unova_Screen_Open(&sDress);
            M.state = MS_DRESS;
        }
        break;
    case MS_DRESS:
        if (!M.dressDone)
        {
            Unova_Screen_Open(&sDress);
            break;
        }
        M.state = MS_PRESTAGE;
        break;
    case MS_PRESTAGE:   // (script 4: to the stage)
        if (U3D_Vm_Idle())
            M.state = U3D_Vm_RunScene(4) ? MS_PRESTAGE_WAIT : MS_STAGE;
        break;
    case MS_PRESTAGE_WAIT:
        if (U3D_Vm_Idle())
        {
            Unova_Screen_Open(&sStage);
            M.state = MS_STAGE;
        }
        break;
    case MS_STAGE:
        if (M.t < STAGE_FRAMES)
        {
            Unova_Screen_Open(&sStage);
            break;
        }
        Score();
        Unova_Screen_Open(&sResults);
        M.state = MS_RESULTS;
        break;
    case MS_RESULTS:
        Results();
        M.state = MS_AFTER;
        break;
    case MS_AFTER:   // (script 5: the show's end, its words reading the results)
        if (U3D_Vm_Idle())
            M.state = U3D_Vm_RunScene(5) ? MS_AFTER_WAIT : MS_THANKS;
        break;
    case MS_AFTER_WAIT:
        if (U3D_Vm_Idle())
            M.state = MS_THANKS;
        break;
    case MS_THANKS:   // (0x28CC: the shared closing words)
        if (U3D_Vm_Idle())
            M.state = U3D_Vm_RunScene(0x28CC) ? MS_THANKS_WAIT : MS_NONE;
        if (M.state == MS_NONE)
            M.active = FALSE;
        break;
    case MS_THANKS_WAIT:
        if (U3D_Vm_Idle())
        {
            M.active = FALSE;
            M.state = MS_NONE;
            MPNet_Log("musical: the show is over");
        }
        break;
    }
}

// ------------------------------------------------------------------------------------------------- the scripts' words
static u8 BestAppeal(u8 slot)
{
    u8 best = 0, score = 0, i;
    bool8 tie = FALSE;
    for (i = 0; i < 4; i++)
        if (score < M.p[slot].appeal[i])
            tie = FALSE, best = i, score = M.p[slot].appeal[i];
        else if (score == M.p[slot].appeal[i])
            tie = TRUE;
    return tie ? 4 : best;
}

// MUSICAL_GetValue (Black's SCRCMD_MUSICAL_GetValue): OP with ARG; *SET FALSE: op 5 sets, nothing to give back
u16 Unova_Musical_GetValue(u8 op, u16 arg, bool8 *set)
{
    struct UnovaMusicalSave *s = Save();
    u8 i;
    *set = TRUE;
    if (op >= 30 && !M.active)
        return 0;
    switch (op)
    {
    case 0: return s->shows;
    case 1: return s->tops;
    case 2: return s->selected;
    case 3: case 8: case 9: case 10:
    {
        u8 best = 0, at = 4, low = 0xFF, lowAt = 4, prev = 0, lead = 4, second = 4;
        bool8 tie = FALSE;
        for (i = 0; i < 4; i++)
        {
            u8 v = s->equip[i];
            if (v > best)
            {
                if (prev == best)
                    second = 4;
                else
                    prev = best, second = lead;
                best = v, at = i, lead = i, tie = FALSE;
            }
            else if (v == best)
                tie = TRUE;
            if (v <= low)
                low = v, lowAt = i;
        }
        if (op == 3)
            return tie ? 4 : at;
        if (op == 8)
            return lowAt;
        if (op == 9)
        {
            for (i = 0, best = 0, at = 4; i < 4; i++)
                if (s->equip[i] > best)
                    best = s->equip[i], at = i;
            return at;
        }
        return second;
    }
    case 4: return s->setting;
    case 5: s->setting = arg; *set = FALSE; return 0;
    case 6:
        for (i = 0; i < 4; i++)
            if (s->equip[i])
                return 1;
        return 0;
    case 7: return s->shot != 0;
    case 11:
    {
        u8 n = 0;
        for (i = 0; i < PARTY_SIZE; i++)
            if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES) != SPECIES_NONE && !GetMonData(&gPlayerParty[i], MON_DATA_IS_EGG))
                n++;
        return n;
    }
    case 12:
        return arg < PARTY_SIZE && GetMonData(&gPlayerParty[arg], MON_DATA_SPECIES) != SPECIES_NONE
            && !GetMonData(&gPlayerParty[arg], MON_DATA_IS_EGG);
    case 13:
        for (i = 0; i < SLOTS; i++)
            if (s->outfit[i])
                return 1;
        return 0;
    case 14: return 0;   // (a distributed show: none)
    case 15: return Owned(arg);
    case 30: return M.local;
    case 31:
    {
        u16 best = 0;
        for (i = 0; i < 4; i++)
            best = M.p[i].score > best ? M.p[i].score : best;
        return best;
    }
    case 32:
    {
        u16 low = 0xFF;
        for (i = 0; i < 4; i++)
            low = M.p[i].score < low ? M.p[i].score : low;
        return low;
    }
    case 33: return ScoreCapped(arg);
    case 35: return arg;
    case 36:
    {
        u8 best = 0, at = 0;
        for (i = 0; i < 4; i++)
            if (best < M.bonus[i])
                best = M.bonus[i], at = i;
        return at;
    }
    case 37: return arg < 4 ? M.rank[arg] : 0;
    case 38: return arg < 4 ? BestAppeal(arg) : 4;
    case 39:
        if (arg >= 4 || arg == M.local)
            return 0;
        if (M.p[arg].kind == PK_FRIEND)
            return 1;
        return P.s[M.show].r[M.p[arg].rival < P.s[M.show].rivals ? M.p[arg].rival : 0].portrait;
    }
    return 0;
}

// MUSICAL_GetProgramInfo: fan SLOT's KIND (0 its look by what it liked, 1 its object, 2 its message, 3 has a prop,
// 4 the prop)
u16 Unova_Musical_FanInfo(u8 slot, u8 kind)
{
    struct UnovaMusicalSave *s = Save();
    static const u8 sLook[5] = {0x18, 0x0F, 0x2D, 0x0C, 0};
    if (slot >= FANS)
        return 0;
    switch (kind)
    {
    case 0: return sLook[s->fans[slot].state < 5 ? s->fans[slot].state : 4];
    case 1: return slot + 5;
    case 2: return slot + 0x0F;
    case 3: return s->fans[slot].rank;
    case 4: return s->fans[slot].prop;
    }
    return 0;
}

void Unova_Musical_ResetFan(u8 slot)
{
    if (slot < FANS)
        Save()->fans[slot].rank = 0;
}

void Unova_Musical_AddProp(u8 prop)
{
    if (prop < PROPS)
        Save()->owned[prop / 8] |= 1 << (prop % 8);
    MPNet_Log("musical: prop %u owned", prop);
}

// MUSICAL_BufferString: KIND into word slot SLOT (5 / 6 the performer of ARG's trainer / POKeMON)
void Unova_Musical_BufferString(u8 kind, u8 slot, u16 arg)
{
    u8 *out = U3D_Vm_WordSlot(slot);
    switch (kind)
    {
    case 0: StringCopy(out, gSaveBlock2Ptr->playerName); break;   // (the stage's saved words: the player's here)
    case 1: StringCopy(out, U3D_Text(BANK_PROP, arg + 3)); break;
    case 8: StringCopy(out, U3D_Text(BANK_PROP_ART, arg + 3)); break;
    case 2:
        if (Load())
            MP_AsciiToGame(out, P.s[M.show].title, 40);
        break;
    case 3: StringCopy(out, U3D_Text(BANK_CATEGORY, arg)); break;
    case 4: StringCopy(out, U3D_Text(BANK_CATEGORY, arg + 4)); break;
    case 5:
    case 6:
        if (arg < 4 && M.active)
        {
            struct Performer *e = &M.p[arg];
            if (e->kind == PK_ME)
            {
                if (kind == 5)
                    StringCopy(out, gSaveBlock2Ptr->playerName);
                else
                {
                    GetMonData(&gPlayerParty[M.partySlot], MON_DATA_NICKNAME, out);
                    out[POKEMON_NAME_LENGTH] = EOS;
                }
            }
            else if (e->kind == PK_FRIEND)
                StringCopy(out, kind == 5 ? gMpPlayers[e->player].name : sFriend[e->player].nick);
            else
                StringCopy(out, U3D_Text(kind == 5 ? BANK_NPC : BANK_NPC_MON,
                                         P.s[M.show].r[e->rival < P.s[M.show].rivals ? e->rival : 0].nameId));
        }
        break;
    case 7:   // (the categories of the last show's props, most first: three slots)
    {
        struct UnovaMusicalSave *s = Save();
        u8 order[4] = {0, 1, 2, 3}, v[4], i, j, label[3];
        for (i = 0; i < 4; i++)
            v[i] = s->equip[i];
        for (i = 1; i < 4; i++)
            for (j = 0; j < 4 - i; j++)
                if (v[j] < v[j + 1])
                {
                    u8 t = v[j];
                    v[j] = v[j + 1], v[j + 1] = t, t = order[j], order[j] = order[j + 1], order[j + 1] = t;
                }
        if (v[0] == v[1] && v[0] == v[2])
            label[0] = 9, label[1] = 8;
        else
            label[0] = order[0] + 4, label[1] = v[1] == v[2] ? 8 : order[1] + 4;
        label[2] = v[1] == v[3] ? order[1] + 4 : v[2] == v[3] ? order[2] + 4 : order[3] + 4;
        for (i = 0; i < 3; i++)
            StringCopy(U3D_Vm_WordSlot(slot + i), U3D_Text(BANK_CATEGORY, label[i]));
        break;
    }
    }
}

// mp_main.c: a friend's dress-up
void Unova_Musical_OnRelay(u8 from, const u8 *data, u16 len)
{
    struct FriendEntry *f;
    if (from >= MP_MAX_PLAYERS || len < 12 + SLOTS + POKEMON_NAME_LENGTH + 1 || data[1] != 0)
        return;
    f = &sFriend[from];
    f->species = data[2] | data[3] << 8;
    memcpy(&f->pid, data + 4, 4), memcpy(&f->otId, data + 8, 4), memcpy(f->prop, data + 12, SLOTS);
    memcpy(f->nick, data + 12 + SLOTS, POKEMON_NAME_LENGTH);
    f->nick[POKEMON_NAME_LENGTH] = EOS;
    f->valid = f->species != SPECIES_NONE && f->species < NUM_SPECIES;
    MPNet_Log("musical: player %u dressed up species %u", from + 1, f->species);
}

// (tests: `mp musical PROPS`: the first PROPS props owned)
void Unova_Musical_DebugGive(u8 count)
{
    u8 i;
    for (i = 0; i < count && i < PROPS; i++)
        Unova_Musical_AddProp(i);
}
