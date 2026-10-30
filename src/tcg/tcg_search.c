/* SEARCH & SORT for the card games' binders and deck builders (see tcg_search.h).
 *
 * The menu lists SEARCH (the name: any part of it, any case; typed on the screen keyboard or the real one), SORT BY,
 * ORDER, OWNED ONLY, then the game's own filters, CLEAR ALL and DONE. UP / DOWN choose a row, LEFT / RIGHT change it,
 * A types the search or clears, B or {SEARCH} closes it. Every change shows at once in the list behind it and in the
 * count of cards it leaves. Nothing here changes a card, a deck or what is owned: TcgSearch_Apply only drops and
 * reorders the ids the screen hands it. */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>
#include "global.h"
#include "sound.h"
#include "constants/songs.h"
#include "tcg_gfx.h"
#include "tcg_ui_kit.h"
#include "tcg_search.h"

extern char gPlatformSavePath[];

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
    KEY_L = 512,
    KEY_SEARCH = 0x1000
};

struct Settings
{
    char text[SEARCH_TEXT_MAX + 1];
    uint8_t sort, desc, ownedOnly;
    int8_t f[SEARCH_FILTERS_MAX];
};
static const struct SearchGame *sGame[SEARCH_GAMES];
static struct Settings sSet[SEARCH_GAMES];
static unsigned sGen[SEARCH_GAMES];
static int sLoaded[SEARCH_GAMES], sDirty;   /* (each game's line is read once, when it's first known) */
static struct
{
    int on, game, row, top, typing;
    char before[SEARCH_TEXT_MAX + 1];
    unsigned frame;
} M;

/* ------------------------------------------------------------------ */
/* Remembered settings: "<save>.search", a line a game                  */
/* ------------------------------------------------------------------ */
static void Path(char *out, size_t size) { snprintf(out, size, "%s.search", gPlatformSavePath); }
static void Load(void)
{
    char path[512], line[256];
    FILE *f;
    unsigned want = 0, g;
    for (g = 0; g < SEARCH_GAMES; g++)
        if (sGame[g] && !sLoaded[g])
            want |= 1u << g, sLoaded[g] = 1;
    if (!want)
        return;
    Path(path, sizeof(path));
    if (!(f = fopen(path, "r")))
        return;
    while (fgets(line, sizeof(line), f))
    {
        char *tab = strchr(line, '\t'), *p;
        if (!tab)
            continue;
        *tab = 0;
        for (g = 0; g < SEARCH_GAMES; g++)
            if (want >> g & 1 && !strcmp(line, sGame[g]->id))
            {
                struct Settings *s = &sSet[g];
                int i, v;
                char *text = tab + 1, *rest = strchr(text, '\t');
                if (!rest)
                    break;
                *rest++ = 0;
                snprintf(s->text, sizeof(s->text), "%s", text);
                p = rest;
                if (sscanf(p, "%d", &v) == 1 && v >= 0 && v < sGame[g]->sortCount)
                    s->sort = (uint8_t)v;
                for (i = 0; i < 2 + SEARCH_FILTERS_MAX && (p = strchr(p, ' ')); i++)
                {
                    p++;
                    if (sscanf(p, "%d", &v) != 1)
                        break;
                    if (i == 0)
                        s->desc = v != 0;
                    else if (i == 1)
                        s->ownedOnly = v != 0;
                    else if (i - 2 < sGame[g]->filterCount)
                    {
                        const struct SearchFilter *fl = &sGame[g]->filters[i - 2];
                        s->f[i - 2] = (int8_t)(v >= 0 && v <= fl->count ? v : 0);
                    }
                }
                break;
            }
    }
    fclose(f);
}
static void Save(void)
{
    char path[512], tmp[520], line[256];
    FILE *f, *old;
    unsigned g;
    int i;
    if (!sDirty || !gPlatformSavePath[0])
        return;
    sDirty = 0;
    Path(path, sizeof(path));
    snprintf(tmp, sizeof(tmp), "%s.tmp", path);
    if (!(f = fopen(tmp, "w")))
        return;
    if ((old = fopen(path, "r")))
    {
        /* the lines of games not opened since the game started stay as they were */
        while (fgets(line, sizeof(line), old))
        {
            size_t n = strcspn(line, "\t");
            int known = 0;
            for (g = 0; g < SEARCH_GAMES; g++)
                known |= sGame[g] && strlen(sGame[g]->id) == n && !strncmp(line, sGame[g]->id, n);
            if (!known && line[n] == '\t')
                fputs(line, f);
        }
        fclose(old);
    }
    for (g = 0; g < SEARCH_GAMES; g++)
    {
        if (!sGame[g])
            continue;
        fprintf(f, "%s\t%s\t%u %u %u", sGame[g]->id, sSet[g].text, sSet[g].sort, sSet[g].desc, sSet[g].ownedOnly);
        for (i = 0; i < sGame[g]->filterCount; i++)
            fprintf(f, " %d", sSet[g].f[i]);
        fputc('\n', f);
    }
    fclose(f);
    remove(path);
    rename(tmp, path);
}
void TcgSearch_Register(unsigned game, const struct SearchGame *g)
{
    if (game < SEARCH_GAMES && !sGame[game])
    {
        sGame[game] = g;   /* (its line in the file is read now that it's known: Load) */
    }
}
static void Changed(int game)
{
    sGen[game]++;
    sDirty = 1;
}

/* ------------------------------------------------------------------ */
/* What the list shows                                                  */
/* ------------------------------------------------------------------ */
static int ContainsCase(const char *hay, const char *needle)
{
    size_t n = strlen(needle), i;
    if (!n)
        return 1;
    for (; *hay; hay++)
    {
        for (i = 0; i < n && hay[i] && tolower((unsigned char)hay[i]) == tolower((unsigned char)needle[i]); i++)
            ;
        if (i == n)
            return 1;
    }
    return 0;
}
static int Passes(const struct SearchGame *g, const struct Settings *s, unsigned id)
{
    int i;
    if (s->text[0] && !ContainsCase(g->name(id), s->text))
        return 0;
    if (s->ownedOnly && !g->owned(id))
        return 0;
    for (i = 0; i < g->filterCount; i++)
    {
        const struct SearchFilter *f = &g->filters[i];
        int choice = s->f[i], v;
        if (!choice)
            continue;
        if (f->kind == SF_CHOICE && f->values && f->values[choice - 1] == SEARCH_ALL)
            continue;
        v = g->value(id, f->key);
        if (v == SEARCH_NA)
            return 0;
        if (f->kind == SF_CHOICE)
        {
            int want = f->values ? f->values[choice - 1] : choice - 1;
            if (want != SEARCH_ALL && v != want)
                return 0;
        }
        else
        {
            int edge = f->lo + (choice - 1) * f->step;
            if (f->kind == SF_MIN ? v < edge : v > edge)
                return 0;
        }
    }
    return 1;
}
/* (qsort has no context: the one sort running) */
static const struct SearchGame *sSortGame;
static const struct Settings *sSortSet;
static uint16_t sOrder[4096];   /* each id's place in the list as handed over (ties keep it) */
static int SortValue(unsigned id, int key)
{
    if (key == -2)
        return (int)sSortGame->owned(id);
    return sSortGame->value(id, key);
}
static int Compare(const void *pa, const void *pb)
{
    unsigned a = *(const uint16_t *)pa, b = *(const uint16_t *)pb;
    int key = sSortGame->sorts[sSortSet->sort].key, r = 0;
    if (key == -1)
    {
        const char *x = sSortGame->name(a), *y = sSortGame->name(b);
        if (!*x != !*y)
            return !*x ? 1 : -1;   /* (a card not met yet has no name to show: last, either way) */
        for (; *x && *y && !r; x++, y++)
            r = tolower((unsigned char)*x) - tolower((unsigned char)*y);
        if (!r)
            r = (unsigned char)*x - (unsigned char)*y;
        if (sSortSet->desc)
            r = -r;
    }
    else
    {
        int va = SortValue(a, key), vb = SortValue(b, key);
        if (va == SEARCH_NA || vb == SEARCH_NA)
            r = (va == SEARCH_NA) - (vb == SEARCH_NA);   /* (cards without it last, either way) */
        else
            r = sSortSet->desc ? (vb > va) - (vb < va) : (va > vb) - (va < vb);
    }
    if (!r)
        r = (int)sOrder[a] - (int)sOrder[b];
    return r;
}
unsigned TcgSearch_Apply(unsigned game, uint16_t *ids, unsigned n)
{
    const struct SearchGame *g = game < SEARCH_GAMES ? sGame[game] : NULL;
    const struct Settings *s;
    unsigned i, k = 0;
    if (!g)
        return n;
    Load();
    s = &sSet[game];
    for (i = 0; i < n; i++)
        if (Passes(g, s, ids[i]))
            ids[k++] = ids[i];
    if (s->sort != 0 || s->desc)
    {
        for (i = 0; i < k; i++)
            if (ids[i] < sizeof(sOrder) / sizeof(sOrder[0]))
                sOrder[ids[i]] = (uint16_t)i;
        sSortGame = g;
        sSortSet = s;
        qsort(ids, k, sizeof(ids[0]), Compare);
    }
    return k;
}
int TcgSearch_Narrowed(unsigned game)
{
    const struct Settings *s;
    int i;
    if (game >= SEARCH_GAMES || !sGame[game])
        return 0;
    Load();
    s = &sSet[game];
    if (s->text[0] || s->ownedOnly)
        return 1;
    for (i = 0; i < sGame[game]->filterCount; i++)
        if (s->f[i])
            return 1;
    return 0;
}
static int Sorted(unsigned game) { return sSet[game].sort != 0 || sSet[game].desc; }
unsigned TcgSearch_Generation(unsigned game) { return game < SEARCH_GAMES ? sGen[game] : 0; }
int TcgSearch_Choice(unsigned game, int filter)
{
    if (game >= SEARCH_GAMES || !sGame[game] || filter < 0 || filter >= sGame[game]->filterCount)
        return 0;
    Load();
    return sSet[game].f[filter];
}
void TcgSearch_SetChoice(unsigned game, int filter, int choice)
{
    if (game >= SEARCH_GAMES || !sGame[game] || filter < 0 || filter >= sGame[game]->filterCount)
        return;
    Load();
    if (sSet[game].f[filter] != choice)
    {
        sSet[game].f[filter] = (int8_t)choice;
        Changed((int)game);
        Save();
    }
}
void TcgSearch_ClearAll(unsigned game)
{
    if (game >= SEARCH_GAMES)
        return;
    memset(&sSet[game], 0, sizeof(sSet[game]));
    Changed((int)game);
}

/* ------------------------------------------------------------------ */
/* The menu                                                             */
/* ------------------------------------------------------------------ */
enum { ROW_SEARCH, ROW_SORT, ROW_ORDER, ROW_OWNED, ROW_FILTERS };
static int Rows(void) { return ROW_FILTERS + sGame[M.game]->filterCount + 2; }   /* + CLEAR ALL, DONE */
static int RowClear(void) { return ROW_FILTERS + sGame[M.game]->filterCount; }
void TcgSearch_Open(unsigned game)
{
    if (game >= SEARCH_GAMES || !sGame[game])
        return;
    Load();
    memset(&M, 0, sizeof(M));
    M.on = 1;
    M.game = (int)game;
    PlaySE(SE_RG_BAG_POCKET);
    printf("[search] open (%s)\n", sGame[game]->id);
}
int TcgSearch_Active(void) { return M.on; }
static void Close(void)
{
    M.on = 0;
    Save();
    PlaySE(SE_SELECT);
    printf("[search] closed\n");
}
static const char *FilterValue(const struct SearchFilter *f, int choice, char *buf, size_t size)
{
    if (!choice)
        return f->anyName ? f->anyName : "ANY";
    if (f->kind == SF_CHOICE || f->names)
        return f->names[choice - 1];   /* (a MIN / MAX with names: Triple Triad's A for 10) */
    snprintf(buf, size, "%d", f->lo + (choice - 1) * f->step);
    return buf;
}
static void Cycle(int dir)
{
    struct Settings *s = &sSet[M.game];
    const struct SearchGame *g = sGame[M.game];
    if (M.row == ROW_SORT)
        s->sort = (uint8_t)((s->sort + g->sortCount + dir) % g->sortCount);
    else if (M.row == ROW_ORDER)
        s->desc ^= 1;
    else if (M.row == ROW_OWNED)
        s->ownedOnly ^= 1;
    else if (M.row >= ROW_FILTERS && M.row < RowClear())
    {
        const struct SearchFilter *f = &g->filters[M.row - ROW_FILTERS];
        int n = f->count + 1, v = s->f[M.row - ROW_FILTERS];
        s->f[M.row - ROW_FILTERS] = (int8_t)((v + n + dir) % n);
    }
    else
        return;
    PlaySE(SE_RG_BAG_CURSOR);
    Changed(M.game);
}
int TcgSearch_Step(unsigned pressed)
{
    struct Settings *s;
    int rows, changed = 0;
    if (!M.on)
        return 0;
    M.frame++;
    s = &sSet[M.game];
    rows = Rows();
    if (M.typing)
    {
        /* the screen keyboard (and the real one): the list follows every letter */
        int r = TcgUi_NameStep(pressed);
        if (strcmp(s->text, TcgUi_NameText()))
        {
            snprintf(s->text, sizeof(s->text), "%s", TcgUi_NameText());
            Changed(M.game);
            changed = 1;
        }
        if (r)
        {
            M.typing = 0;
            if (r < 0 && (pressed & KEY_B))
            {
                snprintf(s->text, sizeof(s->text), "%s", M.before);   /* B: as it was */
                Changed(M.game);
                changed = 1;
            }
            printf("[search] text \"%s\"\n", s->text);
        }
        return changed;
    }
    if (pressed & (KEY_B | KEY_SEARCH))
    {
        Close();
        return 0;
    }
    if (pressed & KEY_UP)
        M.row = (M.row + rows - 1) % rows, PlaySE(SE_SELECT);
    if (pressed & KEY_DOWN)
        M.row = (M.row + 1) % rows, PlaySE(SE_SELECT);
    if (pressed & KEY_LEFT)
        Cycle(-1), changed = 1;
    else if (pressed & KEY_RIGHT)
        Cycle(1), changed = 1;
    if (pressed & KEY_A)
    {
        if (M.row == ROW_SEARCH)
        {
            snprintf(M.before, sizeof(M.before), "%s", s->text);
            TcgUi_NameOpen("SEARCH BY NAME", s->text);
            M.typing = 1;
            PlaySE(SE_SELECT);
        }
        else if (M.row == RowClear())
        {
            TcgSearch_ClearAll((unsigned)M.game);
            PlaySE(SE_SUCCESS);
            printf("[search] clear all\n");
            changed = 1;
        }
        else if (M.row == rows - 1)
            Close();
        else
            Cycle(1), changed = 1;
    }
    if (pressed & KEY_START)
    {
        /* START: clear the row under the cursor (a filter back to ANY, the search emptied) */
        if (M.row == ROW_SEARCH && s->text[0])
            s->text[0] = 0, Changed(M.game), changed = 1, PlaySE(SE_SELECT);
        else if (M.row >= ROW_FILTERS && M.row < RowClear() && s->f[M.row - ROW_FILTERS])
            s->f[M.row - ROW_FILTERS] = 0, Changed(M.game), changed = 1, PlaySE(SE_SELECT);
    }
    return changed;
}
void TcgSearch_Draw(unsigned count)
{
    const struct SearchGame *g;
    const struct Settings *s;
    int rows, i, x = 22, y = 6, w = 196, h = 148, rowH = 11, visible = 10, top;
    char line[64], buf[32];
    if (!M.on)
        return;
    g = sGame[M.game];
    s = &sSet[M.game];
    rows = Rows();
    /* the list stays in view behind it, dimmed */
    G_FillA(0, 0, 240, 160, 0x000000, 110);
    G_FillA(x + 3, y + 4, w, h, 0x000000, 80);
    G_RoundFill(x, y, w, h, 0xf4f4f0, 250);
    G_RoundFrame(x, y, w, h, 0x405060, 255);
    G_RoundFill(x + 1, y + 1, w - 2, 15, 0x384868, 255);
    G_Text(x + 6, y, G_WHITE, 0x182030, "SEARCH & SORT", 0, TF_NORMAL);
    snprintf(line, sizeof(line), "%u CARD%s", count, count == 1 ? "" : "S");
    G_TextAlign(x + w - 6, y + 3, TA_RIGHT, count ? 0xc8e8ff : 0xffa0a0, G_NO_SHADOW, line, TF_NARROW);
    top = M.row - visible / 2;
    if (top > rows - visible)
        top = rows - visible;
    if (top < 0)
        top = 0;
    for (i = 0; i < visible && top + i < rows; i++)
    {
        int r = top + i, ry = y + 19 + i * rowH, on = r == M.row, set = 0;
        const char *label = "", *value = "";
        if (r == ROW_SEARCH)
        {
            label = "SEARCH";
            value = s->text[0] ? s->text : "(any name)";
            set = s->text[0] != 0;
        }
        else if (r == ROW_SORT)
            label = "SORT BY", value = g->sorts[s->sort].name, set = s->sort != 0;
        else if (r == ROW_ORDER)
            label = "ORDER", value = s->desc ? "DESCENDING" : "ASCENDING", set = s->desc;
        else if (r == ROW_OWNED)
            label = "OWNED ONLY", value = s->ownedOnly ? "ON" : "OFF", set = s->ownedOnly;
        else if (r < RowClear())
        {
            const struct SearchFilter *f = &g->filters[r - ROW_FILTERS];
            label = f->label;
            value = FilterValue(f, s->f[r - ROW_FILTERS], buf, sizeof(buf));
            set = s->f[r - ROW_FILTERS] != 0;
        }
        else if (r == RowClear())
            label = "CLEAR ALL";
        else
            label = "DONE";
        if (on)
        {
            G_RoundFill(x + 3, ry - 1, w - 6, rowH, 0xf8d0c8, 255);
            G_RoundFrame(x + 3, ry - 1, w - 6, rowH, 0xe82820, 255);
        }
        G_Text(x + 8, ry - 2, r >= RowClear() ? 0x2858a8 : G_INK, G_NO_SHADOW, label, 0, TF_NARROW);
        if (*value)
        {
            int vx = x + w - 10;
            if (on && r != ROW_SEARCH)
            {
                G_Arrow(vx - 1, ry + 2, 3, 0xe82820);
                vx -= 7;
            }
            snprintf(line, sizeof(line), "%s", value);
            TcgUi_FitText(line, 96, TF_NARROW);
            G_TextAlign(vx, ry - 2, TA_RIGHT, set ? 0xc03020 : 0x707880, G_NO_SHADOW, line, TF_NARROW);
            if (on && r != ROW_SEARCH)
                G_Arrow(vx - G_Measure(line, TF_NARROW) - 6, ry + 2, 2, 0xe82820);
        }
    }
    if (top > 0)
        G_Arrow(x + w / 2 - 3, y + 17, 0, 0x8090a0);
    if (top + visible < rows)
        G_Arrow(x + w / 2 - 3, y + 19 + visible * rowH, 1, 0x8090a0);
    G_FillA(x + 4, y + h - 16, w - 8, 1, 0xc0c4c8, 255);
    G_Hint(x + 6, y + h - 14, 0x505860, G_NO_SHADOW,
           M.row == ROW_SEARCH ? "{A} TYPE  {START} EMPTY  {B} DONE"
           : M.row >= RowClear() ? "{A} CHOOSE  {B} DONE"
                                 : "{LEFTRIGHT} CHANGE  {START} ANY  {B} DONE",
           TF_NARROW, 1);
    if (M.typing)
        TcgUi_NameDraw(0x384868);
}
void TcgSearch_Badge(unsigned game, int x, int y, unsigned count)
{
    char line[64];
    int narrowed, w;
    if (game >= SEARCH_GAMES || !sGame[game])
        return;
    narrowed = TcgSearch_Narrowed(game);
    if (!narrowed && !Sorted(game))
        return;
    if (narrowed)
    {
        int i, n = sSet[game].ownedOnly;
        for (i = 0; i < sGame[game]->filterCount; i++)
            n += sSet[game].f[i] != 0;
        if (sSet[game].text[0] && n)
            snprintf(line, sizeof(line), "\"%s\" +%d: %u", sSet[game].text, n, count);
        else if (sSet[game].text[0])
            snprintf(line, sizeof(line), "\"%s\": %u", sSet[game].text, count);
        else
            snprintf(line, sizeof(line), "%d FILTER%s: %u", n, n == 1 ? "" : "S", count);
    }
    else
        snprintf(line, sizeof(line), "BY %s", sGame[game]->sorts[sSet[game].sort].name);
    TcgUi_FitText(line, 110, TF_NARROW);
    w = G_Measure(line, TF_NARROW) + 8;
    if (x < 0)
        x = -x - w;   /* (a negative x: the right edge) */
    G_RoundFill(x, y, w, 10, narrowed ? 0xd85830 : 0x4878a8, 235);
    G_Text(x + 4, y - 2, G_WHITE, G_NO_SHADOW, line, 0, TF_NARROW);
}
void TcgSearch_Empty(unsigned game, int cx, int y)
{
    (void)game;
    G_TextAlign(cx, y, TA_CENTER, G_WHITE, 0x204030, "No cards match.", TF_SMALL);
    G_Hint(cx - G_Hint(0, 0, 0, 0, "{SEARCH} SEARCH & SORT: CLEAR ALL", TF_NARROW, 0) / 2, y + 14, G_WHITE, 0x204030,
           "{SEARCH} SEARCH & SORT: CLEAR ALL", TF_NARROW, 1);
}
/* mp <game> search TEXT | sort N | desc | owned | filter I V | clear */
void TcgSearch_Debug(unsigned game, const char *what)
{
    struct Settings *s;
    int a = 0, b = 0;
    if (game >= SEARCH_GAMES || !sGame[game])
        return;
    Load();
    s = &sSet[game];
    if (!strncmp(what, "text ", 5))
        snprintf(s->text, sizeof(s->text), "%s", what + 5);
    else if (sscanf(what, "sort %d", &a) == 1 && a >= 0 && a < sGame[game]->sortCount)
        s->sort = (uint8_t)a;
    else if (!strcmp(what, "desc"))
        s->desc ^= 1;
    else if (!strcmp(what, "owned"))
        s->ownedOnly ^= 1;
    else if (sscanf(what, "filter %d %d", &a, &b) == 2 && a >= 0 && a < sGame[game]->filterCount && b >= 0 &&
             b <= sGame[game]->filters[a].count)
        s->f[a] = (int8_t)b;
    else if (!strcmp(what, "clear"))
        memset(s, 0, sizeof(*s));
    Changed((int)game);
    printf("[search] %s: text \"%s\" sort %s %s owned %u narrowed %d\n", sGame[game]->id, s->text,
           sGame[game]->sorts[s->sort].name, s->desc ? "desc" : "asc", s->ownedOnly, TcgSearch_Narrowed(game));
}
