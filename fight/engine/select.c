/*
 * Character select screen (original module 0x4036A0-0x40B06F): select.def
 * (characters, stages, arcade options), the [Select Info] layout of the
 * motif, the select screen callbacks run by Screen_Run, the arcade opponent
 * order and the palette reservation of selected characters.
 *
 * The module starts right after the CNS expression code (which ends at
 * 0x40369F) with Select_OpenDef, whose message is the first string of the
 * select screen's .rdata block (0x4F0670), and ends before CmdLine_Parse.
 */
#include <float.h>

#include "mugen.h"

#define SELECT_MAX_MATCHES      30      /* entries of [Options] arcade/team.maxmatches */
#define SELECT_PAL_COUNT        12

static const char *const mode_titles[] = {
    "Arcade", "Versus Mode", "Team Arcade", "Team Versus", "Team Cooperative",
    "Survival", "Survival Cooperative", "Training Mode", "Watch Mode",
};

/* "Out of memory" without a line break, inlined at each failed allocation of the loaders. */
static void Select_OutOfMemory(void)
{
    TextRing *ring = g_game->error_text;

    if (ring != NULL) {
        TextRing_Printf(ring, "Out of memory");
    } else {
        fprintf(stderr, "Out of memory");
    }
}

/* 0x004036A0: finds and parses select.def; the grid size comes from the layout loaded before. */
int Select_OpenDef(const char *path)
{
    GameVars *g = g_game;
    char found[0x200];
    Ini *ini;
    Stream *s;

    g_selectRows = g->select_info.rows;
    g_selectInfo = &g->select_info;
    g_selectCells = g->select_info.cells;
    g_selectColumns = g_selectInfo->columns;
    if (!File_Find(path, found, NULL, NULL)) {
        Error_Printf("Couldn't find file for selection screen: %s\n", path);
        return 0;
    }
    ini = NULL;
    s = Stream_OpenFile(found, "rt");
    if (s != NULL) {
        ini = Ini_Parse(s, INI_MODE_READ);
        Stream_Close(s);
    }
    g_selectIni = ini;
    return ini != NULL;
}

/* 0x00403760: character info, then select.def unless a quick VS match was requested. */
int Select_Init(void)
{
    Log_Printf("Initializing character info...");
    if (!Select_InitCharInfo()) {
        Error_Exit();
    }
    Log_Printf("OK\n");
    if (!g_game->quick_vs) {
        Log_Printf("Initializing select screen...");
        if (!Select_OpenDef(g_game->select_info.def_file)) {
            Error_Fatal("Cannot load char select data in %s\n", g_game->select_info.def_file);
        }
        Log_Printf("finding characters...");
        if (!Select_LoadDef()) {
            Error_Fatal("Error finding characters/stages in %s\n", g_game->select_info.def_file);
        }
        Log_Printf("OK");
        Log_Printf("\n");
    }
    return 1;
}

/* 0x00403820 */
int Select_InitCharInfo(void)
{
    GameVars *g = g_game;

    g_selectInfo = &g->select_info;
    g->char_capacity = 32;
    g->char_info = calloc(32, sizeof(CharInfo));
    if (g->char_info == NULL) {
        Select_OutOfMemory();
        return 0;
    }
    g->char_info_2 = calloc(g->char_capacity + 90, sizeof(int));
    if (g->char_info_2 == NULL) {
        Error_OutOfMemory();
        return 0;
    }
    return Precache_Init(g->precache) != 0;
}

/* Comma-separated list of up to 30 ints ("arcade.maxmatches"); missing entries keep their value. */
static void Select_ReadIntList(const char *s, int *out)
{
    int n, i = 0;

    for (n = 0; n < SELECT_MAX_MATCHES; n++) {
        if (s[i] == '\0') {
            break;
        }
        out[n] = atoi(s + i);
        do {
            i++;
        } while (s[i] != '\0' && s[i] != ',');
        if (s[i] != '\0') {
            i++;
        }
        while (Char_IsSpace(s[i]) && s[i] != '\0') {
            i++;
        }
    }
}

/* Index of the first non-blank char from i. */
static int Select_SkipSpaces(const char *s, int i)
{
    while (Char_IsSpace(s[i]) && s[i] != '\0') {
        i++;
    }
    return i;
}

/* Skips the current char, then blanks. */
static int Select_SkipCharAndSpaces(const char *s, int i)
{
    do {
        i++;
    } while (Char_IsSpace(s[i]) && s[i] != '\0');
    return i;
}

/*
 * Copies a lower-cased word of a [Characters] line up to stop, a blank or the
 * end; returns the index after it. As in the original the copy is unbounded.
 */
static int Select_ReadWord(const char *line, int i, char stop, char *out)
{
    int n = 0;
    char c = line[i];

    while (c != stop && !Char_IsSpace(c) && line[i] != '\0') {
        out[n++] = (char)Char_ToLower(line[i]);
        i++;
        c = line[i];
    }
    out[n] = '\0';
    return i;
}

/*
 * [Info] name copy: keeps a closing quote that the length limit cut off, then
 * resolves quotes and escapes.
 */
static void Select_CopyName(char *dst, const char *value, size_t max)
{
    int quoted = 0;
    int len = (int)strlen(value);

    if (len > 0 && value[len - 1] == '"') {
        quoted = 1;
    }
    dst[0] = '\0';
    strncat(dst, value, max);
    if (quoted) {
        len = (int)strlen(dst);
        if (len > 0) {
            dst[len - 1] = '"';
        }
    }
    Str_Unquote(dst);
}

/*
 * Reads the [Info] name of a stage into stage slot n. Returns 0 on the
 * (unreachable) error of the value disappearing between two lookups.
 */
static int Select_ReadStageName(Ini *def, int n)
{
    const char *value;

    if (Ini_SelectGroup(def, "Info") == -1) {
        return 1;
    }
    if (Ini_GetValue(def, "displayname") != NULL) {
        char *name = g_game->select_screen[n].name;
        value = Ini_GetValue(def, "displayname");
        if (value == NULL) {
            Error_Printf("Cannot find parameter: %s", "displayname");
            Error_Printf("Error getting stage displayname for %s\n", g_game->select_screen[n].path);
            return 0;
        }
        Select_CopyName(name, value, 0x4E);
    } else if (Ini_GetValue(def, "name") != NULL) {
        char *name = g_game->select_screen[n].name;
        value = Ini_GetValue(def, "name");
        if (value == NULL) {
            Error_Printf("Cannot find parameter: %s", "name");
            Error_Printf("Error getting stage name for %s\n", g_game->select_screen[n].path);
            return 0;
        }
        Select_CopyName(name, value, 0x4E);
    }
    return 1;
}

/* 1 if the stage list already has this path. */
static int Select_HasStage(const char *path)
{
    int n = g_game->stage_count;
    int j;

    for (j = 0; j < n; j++) {
        if (strcmp(g_game->select_screen[j].path, path) == 0) {
            break;
        }
    }
    return j < n;
}

/* Opens and parses a text file through a file stream (inlined in the original). */
static Ini *Select_OpenIni(const char *path)
{
    Ini *ini = NULL;
    Stream *s = Stream_OpenFile(path, "rt");

    if (s != NULL) {
        ini = Ini_Parse(s, INI_MODE_READ);
        Stream_Close(s);
    }
    return ini;
}

/*
 * Reads the stage file of the next free slot n (path already set) and counts
 * it. Returns 0 when the file could not be read, -1 on error, 1 when added.
 */
static int Select_LoadStage(int n)
{
    Ini *def = Select_OpenIni(g_game->select_screen[n].path);
    StageInfo *stage;

    if (def == NULL) {
        return 0;
    }
    if (!Select_ReadStageName(def, n)) {
        return -1;
    }
    Ini_Free(def);
    stage = &g_game->select_screen[n];
    if (stage->name[0] == '\0') {
        stage->name[0] = '\0';
        strncat(stage->name, stage->path, 0x4F);
    }
    g_game->stage_count++;
    return 1;
}

/*
 * 0x004038D0: reads select.def: [Options], [Characters] into the grid and
 * GameVars.char_info, then the stage list from the characters' stages and
 * [ExtraStages]. Frees select.def (g_selectIni is left dangling).
 *
 * Quirks kept: the name and parameter words of a [Characters] line are copied
 * without a length check; parameter values are lower-cased (music paths too);
 * the zip stream of the last character is not closed when the grid fills up;
 * error returns leak the character's Ini; a failed realloc leaves
 * char_capacity raised.
 */
int Select_LoadDef(void)
{
    static const int sprite_select[] = { 9000, 0, 9000, 1, 9000, 2, -1 };
    char name[0x200];
    char word[0x200];
    char zip[0x200];
    char file[0x200];
    char tmp[0x200];
    char dir[0x200];
    char sprite_path[0x200];
    int select[7];
    Stream *zs = NULL;
    GameVars *g;
    const char *line;
    const char *value;
    int row = 0, col = 0, extra = 0;
    int i, n, k;

    n = g_selectColumns * g_selectRows;
    for (i = 0; i < n; i++) {
        g_selectCells[i].char_index = SELECT_CELL_EMPTY;
    }

    if (Ini_SelectGroup(g_selectIni, "Options") == -1) {
        Error_Printf("Could not locate Options group.\n");
        return 0;
    }
    value = Ini_GetValue(g_selectIni, "arcade.maxmatches");
    if (value != NULL) {
        Select_ReadIntList(value, g_game->arcade_maxmatches);
    }
    value = Ini_GetValue(g_selectIni, "team.maxmatches");
    if (value != NULL) {
        Select_ReadIntList(value, g_game->team_maxmatches);
    }
    if (Ini_SelectGroup(g_selectIni, "Characters") == -1) {
        Error_Printf("Could not locate Characters group.\n");
        return 0;
    }

    while ((line = Ini_NextLine(g_selectIni)) != NULL) {
        SelectCell *cell;
        CharInfo *ci;
        Ini *def = NULL;
        int is_zip = 0;
        int param;

        zip[0] = '\0';
        g = g_game;
        if (g->char_count >= g->char_capacity) {
            CharInfo *info;
            int *order;
            g->char_capacity += 8;
            info = realloc(g->char_info, g->char_capacity * sizeof(CharInfo));
            if (info == NULL) {
                Select_OutOfMemory();
                return 0;
            }
            memset(&info[g->char_capacity - 8], 0, 8 * sizeof(CharInfo));
            g->char_info = info;
            order = realloc(g->char_info_2, g->char_capacity * sizeof(int) + 90 * sizeof(int));
            if (order == NULL) {
                Select_OutOfMemory();
                return 0;
            }
            memset(&order[g->char_capacity - 8], 0, 8 * sizeof(int));
            g->char_info_2 = order;
        }
        cell = &g_selectCells[g_selectColumns * row + col];
        ci = &g->char_info[g->char_count];
        memset(ci, 0, sizeof(CharInfo));
        ci->boss = -1;

        /* "name, stage, param=value, ..." */
        i = 0;
        if (line[0] != ',') {
            char c = line[0];
            while (c != '\0') {
                name[i] = c;
                c = line[i + 1];
                i++;
                if (c == ',') {
                    break;
                }
            }
        }
        name[i] = '\0';
        if (line[i] != '\0') {
            i++;
        }
        i = Select_SkipSpaces(line, i);
        n = 0;
        while (!Char_IsSpace(line[i]) && line[i] != ',' && line[i] != '\0') {
            if (n < 0x1FF) {
                ci->stage[n++] = line[i];
            }
            i++;
        }
        ci->stage[n] = '\0';
        if (n > 0 && _stricmp(ci->stage, "random") != 0) {
            ci->include_stage = 3;
        }
        i = Select_SkipSpaces(line, i);
        while (line[i] == ',') {
            i = Select_SkipCharAndSpaces(line, i);
            i = Select_ReadWord(line, i, '=', word);
            if (_stricmp(word, "includestage") == 0) {
                param = 0;
            } else if (_stricmp(word, "music") == 0) {
                param = 1;
            } else if (_stricmp(word, "order") == 0) {
                param = 2;
            } else if (_stricmp(word, "boss") == 0) {
                param = 3;
            } else {
                Error_Printf("\n[Characters]: On line: %s\n-- Unknown flag: %s", line, word);
                return 0;
            }
            i = Select_SkipSpaces(line, i);
            if (line[i] != '=') {
                Error_Printf("\n[Characters]: On line: %s\n-- Param %s missing '='", line, word);
                return 0;
            }
            i = Select_SkipCharAndSpaces(line, i);
            i = Select_ReadWord(line, i, ',', word);
            switch (param) {
            case 0:
                ci->include_stage = atoi(word) != 0 ? 3 : 0;
                break;
            case 1:
                strncpy(ci->music, word, 0x1FF);
                break;
            case 2:
                if (!Char_IsDigit(word[0])) {
                    Error_Printf("Order parameter must be a numeric value: %s\n", line);
                    return 0;
                }
                ci->order = atoi(word) - 1;
                if (ci->order < -1) {
                    ci->order = -1;
                }
                if (ci->order > SELECT_MAX_MATCHES - 1) {
                    ci->order = SELECT_MAX_MATCHES - 1;
                }
                break;
            case 3:
                if (!Char_IsDigit(word[0])) {
                    Error_Printf("Boss parameter must be a numeric value: %s\n", line);
                    return 0;
                }
                ci->boss = atoi(word) - 1;
                break;
            }
            i = Select_SkipSpaces(line, i);
        }

        if (_stricmp(name, "randomselect") == 0) {
            cell->name[0] = '\0';
            cell->sprites = NULL;
            cell->char_index = SELECT_CELL_RANDOM;
            goto next;
        }

        /* "file.zip:dir/char.def" */
        if (name[0] == '\0') {
            zip[0] = '\0';
            tmp[0] = '\0';
        } else {
            for (k = (int)strlen(name) - 1; k >= 0 && name[k] != ':'; k--) {
            }
            k++;
            strcpy(tmp, name + k);
            if (k > 0 && name[k - 1] == ':') {
                k--;
            }
            if (k > 0) {
                strncpy(zip, name, k);
            }
            zip[k] = '\0';
        }
        strncpy(name, tmp, 0x1FF);
        Path_Split(name, dir, file);

        if (zip[0] != '\0' && File_GetType(zip) == FILETYPE_ZIP) {
            is_zip = 1;
        } else if (File_GetType(name) == FILETYPE_ZIP) {
            /* "chars/kfm.zip": the def file inside is "chars/kfm.def" */
            is_zip = 1;
            strncpy(zip, name, 0x1FF);
            strcpy(name + strlen(name) - 3, "def");
        }
        if (is_zip) {
            if (zip[0] != '/' && zip[0] != '\\' && !(zip[0] != '\0' && zip[1] == ':')) {
                strncpy(tmp, zip, 0x1FF);
                sprintf(zip, "chars/%s", tmp);
            }
            strncpy(file, name, 0x1FF);
            zs = Mem_Calloc(1, sizeof(Stream));
            if (zs == NULL) {
                g_streamError = STREAM_ERR_NOMEM;
                Log_Printf("\n  Error opening zip file: %s\n", zip);
                goto failed;
            }
            zs->type = STREAM_ZIP;
            zs->zip = unzOpen(zip);
            if (zs->zip == NULL) {
                g_streamError = STREAM_ERR_OPEN;
                free(zs);
                zs = NULL;
                Log_Printf("\n  Error opening zip file: %s\n", zip);
                goto failed;
            }
            if (!Stream_ZipLoad(zs, file)) {
                free(zs);           /* the zip file is not closed */
                zs = NULL;
                if (g_streamError != STREAM_ERR_ZIP) {
                    Log_Printf("\n  Error opening zip file: %s\n", zip);
                } else {
                    Log_Printf("\n  Could not open %s in zip file: %s\n", file, zip);
                }
                goto failed;
            }
            def = Ini_Parse(zs, INI_MODE_READ);
            if (def == NULL) {
                Log_Printf("\n  Error reading from zip MFstream: %s:%s\n", zip, file);
            }
            Stream_ZipUnload(zs);   /* the zip stays open for the sprites */
            if (def == NULL) {
                goto failed;
            }
        } else {
            if (dir[0] == '\0') {
                sprintf(dir, "chars/%s/", name);
                strncpy(file, name, 0x1FF);
            } else if (dir[0] != '/' && dir[0] != '\\' && dir[1] != ':') {
                strncpy(tmp, dir, 0x1FF);
                sprintf(dir, "chars/%s", tmp);
            }
            if (strlen(file) <= 4 || _stricmp(file + strlen(file) - 4, ".def") != 0) {
                strcat(file, ".def");
            }
            Path_Join(file, dir, tmp);
            def = Select_OpenIni(tmp);
            if (def == NULL) {
                Log_Printf("\n  Error reading character file: %s\n", tmp);
                goto failed;
            }
        }

        /* the character's def file */
        Ini_SelectGroup(def, "Info");
        if (Ini_GetValue(def, "displayname") != NULL) {
            value = Ini_GetValue(def, "displayname");
            if (value == NULL) {
                Error_Printf("Cannot find parameter: %s", "displayname");
                Error_Printf("Error reading %s\n", file);
                return 0;
            }
            Select_CopyName(cell->name, value, sizeof(cell->name) - 2);
        } else if (Ini_GetValue(def, "name") != NULL) {
            value = Ini_GetValue(def, "name");
            if (value == NULL) {
                Error_Printf("Cannot find parameter: %s", "name");
                Error_Printf("Error reading %s\n", file);
                return 0;
            }
            Select_CopyName(cell->name, value, sizeof(cell->name) - 2);
        }
        cell->def_file[0] = '\0';
        strncat(cell->def_file, file, 0x1FF);
        cell->dir[0] = '\0';
        strncat(cell->dir, is_zip ? zip : dir, 0x1FF);
        cell->is_zip = is_zip;
        {
            int lc_width, lc_height;

            Ini_SelectGroup(def, "Info");
            Ini_Scanf(def, "pal.defaults", "%i,%i,%i,%i", &ci->pal_defaults[0], &ci->pal_defaults[1],
                      &ci->pal_defaults[2], &ci->pal_defaults[3]);
            for (k = 0; k < 4; k++) {
                if (ci->pal_defaults[k] <= 0 || ci->pal_defaults[k] > SELECT_PAL_COUNT) {
                    ci->pal_defaults[k] = k + 1;
                }
            }
            lc_width = 320;
            lc_height = 240;
            Ini_Scanf(def, "localcoord", "%i,%i", &lc_width, &lc_height);
            cell->scale = 1.0f;
            if (lc_width > 0) {
                cell->scale = (float)(g_game->motif_coord[0] / lc_width);
            }
        }
        if (Ini_SelectGroup(def, "Palette keymap") != -1) {
            for (k = 0; k < SELECT_PAL_COUNT; k++) {
                ci->pal_keymap[k] = 1;
                value = Ini_GetValue(def, g_palKeymapKeys[k]);
                if (value != NULL) {
                    ci->pal_keymap[k] = atoi(value);
                    if (ci->pal_keymap[k] < 1) {
                        ci->pal_keymap[k] = 1;
                    } else if (ci->pal_keymap[k] > SELECT_PAL_COUNT) {
                        ci->pal_keymap[k] = SELECT_PAL_COUNT;
                    }
                    ci->pal_mask |= 1u << ((ci->pal_keymap[k] - 1) & 31);
                }
            }
            ci->pal_free = ci->pal_mask;
        } else {
            for (k = 0; k < SELECT_PAL_COUNT; k++) {
                ci->pal_keymap[k] = k + 1;
            }
        }
        if (Ini_SelectGroup(def, "Arcade") != -1) {
            value = Ini_GetValue(def, "intro.storyboard");
            if (value != NULL) {
                ci->intro_storyboard[0] = '\0';
                strncat(ci->intro_storyboard, value, 0x1FF);
            }
            value = Ini_GetValue(def, "ending.storyboard");
            if (value != NULL) {
                ci->ending_storyboard[0] = '\0';
                strncat(ci->ending_storyboard, value, 0x1FF);
            }
        }
        if (Ini_SelectGroup(def, "Files") == -1) {
            Error_Printf("Missing [Files] section in %s\n", file);
            return 0;
        }
        value = Ini_GetValue(def, "sprite");
        if (value == NULL) {
            Error_Printf("Missing sprite parameter in %s\n", file);
            return 0;
        }
        tmp[0] = '\0';
        strncat(tmp, value, 0x1FF);
        Path_Join(tmp, dir, sprite_path);
        cell->char_index = g_game->char_count;
        if (ci->pal_mask == 0) {
            for (k = 0; k < SELECT_PAL_COUNT; k++) {
                sprintf(word, "pal%i", k + 1);
                if (Ini_GetValue(def, word) != NULL) {
                    ci->pal_mask |= 1u << k;
                }
            }
            ci->pal_free = ci->pal_mask;
        }
        ci->def_file[0] = '\0';
        strncat(ci->def_file, cell->def_file, 0x1FF);
        ci->dir[0] = '\0';
        strncat(ci->dir, cell->dir, 0x1FF);
        ci->cell = g_selectColumns * row + col;
        g_game->char_count++;
        memcpy(select, sprite_select, sizeof(select));
        cell->sprites = NULL;
        if (!is_zip) {
            FILE *fp = fopen(sprite_path, "rb");
            if (fp != NULL) {
                SpriteSet *set = NULL;
                Stream *s;
                fclose(fp);
                g_bufferedRead = 0;
                s = Stream_OpenFile(sprite_path, "rb");
                if (s != NULL) {
                    set = Sff_LoadSelected(s, select);
                    Stream_Close(s);
                }
                cell->sprites = set;
                g_bufferedRead = g_game->buffered_read;
            }
        } else {
            Stream_ZipLoad(zs, tmp);
            cell->sprites = Sff_LoadSelected(zs, select);
        }
        Ini_Free(def);
        goto next;

    failed:
        cell->name[0] = '\0';
        cell->sprites = NULL;
        cell->char_index = SELECT_CELL_EMPTY;

    next:
        col++;
        name[0] = '\0';
        if (col >= g_selectColumns) {
            row++;
            col = 0;
        }
        if (row >= g_selectRows) {
            break;
        }
        if (is_zip && zs != NULL) {
            Stream_Close(zs);
        }
    }

    /* the stage list */
    if (Ini_SelectGroup(g_selectIni, "extrastages") != -1) {
        while ((line = Ini_NextLine(g_selectIni)) != NULL) {
            if (line[0] != '\0') {
                extra++;
            }
        }
    }
    g = g_game;
    g->select_screen = calloc(g->char_count + extra, sizeof(StageInfo));
    if (g->select_screen == NULL) {
        Select_OutOfMemory();
        return 0;
    }
    for (k = 0; k < g_game->char_count; k++) {
        StageInfo *stage;
        int r;
        if (g_game->char_info[k].include_stage == 0 || Select_HasStage(g_game->char_info[k].stage)) {
            continue;
        }
        n = g_game->stage_count;
        stage = &g_game->select_screen[n];
        stage->path[0] = '\0';
        strncat(stage->path, g_game->char_info[k].stage, 0x1FF);
        stage->flags = g_game->char_info[k].include_stage;
        r = Select_LoadStage(n);
        if (r < 0) {
            return 0;
        }
        if (r == 0) {
            g_game->char_info[k].stage[0] = '\0';
        }
    }
    if (extra > 0) {
        /* [ExtraStages] again; flags stay as calloc'ed (or as left by a stage that failed to load) */
        Ini_Rewind(g_selectIni);
        while ((line = Ini_NextLine(g_selectIni)) != NULL) {
            StageInfo *stage;
            if (Select_HasStage(line)) {
                continue;
            }
            n = g_game->stage_count;
            stage = &g_game->select_screen[n];
            stage->path[0] = '\0';
            strncat(stage->path, line, 0x1FF);
            if (Select_LoadStage(n) < 0) {
                return 0;
            }
        }
    }
    Ini_Free(g_selectIni);
    return 1;
}

/* ---- select screen callbacks (run by Screen_Run) ---- */

/* 0x00405D20 */
int Charsel_Init(ScreenLoop *screen, CharselState *st)
{
    GameVars *g = g_game;
    SelectInfo *info = &g->select_info;

    memset(st, 0, sizeof(*st));
    st->stage = -1;
    Charsel_SetupMode(st, g->game_mode, g->game_mode_side);
    Charsel_ResetBoxes(&st->phase);
    PalReserve_ResetAll();
    g = g_game;
    screen->bg = g->select_bg;
    screen->scale = (float)g->motif_scale;
    screen->bgm = &g->select_bgm;
    g->fade.type = 1;
    g->fade.ticks = 0;
    g->fade.time = info->fadein_time;
    g->fade.out = 0;
    Log_Printf("Charsel init\n");
    return 1;
}

/*
 * 0x00405DD0: on success copies the teams to GameVars, picks the stage and
 * prepares the arcade order / survival opponents.
 */
int Charsel_End(ScreenLoop *screen, CharselState *st)
{
    GameVars *g = g_game;
    int most[2];
    int t;

    if (screen->done) {
        if (!g->demo_mode) {
            Sound_PlaySystem(g->motif_snd, g->select_info.cancel_snd[0], -1, g->select_info.cancel_snd[1], 100.0);
        }
        Precache_Stop();
        PalFx_ResetGlobal();
        return 1;
    }
    memcpy(g->teams, st->teams, sizeof(g->teams));
    for (t = 0; t < 2; t++) {
        TeamSel *team = &g->teams[t];
        int m;
        most[t] = 0;
        if (team->member_count > 0) {
            int n = 0;
            for (m = 0; m < team->member_count; m++) {
                if (n < team->members[m].count) {
                    n = team->members[m].count;
                }
            }
            most[t] = n;
        }
    }
    if (g->player_cache < most[0] + most[1]) {
        g->player_cache = most[0] + most[1];
    }
    if (st->stage_done) {
        const char *path;
        if (st->stage == -1) {
            if (g->stage_count > 0) {
                path = g->select_screen[Random_Next() % g->stage_count].path;
            } else {
                path = g->start_stage;
            }
        } else {
            path = g->select_screen[st->stage].path;
        }
        g->select_stage[0] = '\0';
        strncat(g->select_stage, path, 0x1FF);
        g = g_game;
    }
    if (g->arcade_side) {
        if (!g->continued) {
            Arcade_BuildOrder();
            g = g_game;
        }
        if (g->precache_enabled) {
            Precache_QueueArcade(0);
            Precache_Start();
            g = g_game;
        }
    }
    if (g->survival_side) {
        Team_Randomize(&g->teams[2 - g->survival_side], 2 - g->survival_side, 1);
    }
    memset(st, 0, sizeof(*st));
    Log_Printf("End of charsel loop\n");
    return 1;
}

/* Sound of the motif, at full volume. */
static void Select_PlaySnd(const int *group, const int *number, int channel)
{
    Sound_PlaySystem(g_game->motif_snd, *group, channel, *number, 100.0);
}

/* 0x00406020: runs the select script of each side. */
int Charsel_Update(ScreenLoop *screen, CharselState *st)
{
    GameVars *g = g_game;
    SelectInfo *info = &g->select_info;
    unsigned int key = screen->key;
    unsigned int input2 = screen->input2;
    int finished = 0;
    int auto_play;
    int side;

    auto_play = g->stresstest != 0 || g->demo_mode != 0;
    if (g->demo_mode && ((key != 0 && !(key & 0x200)) || (input2 != 0 && !(input2 & 0x100)) ||
                         screen->input != 0)) {
        /* any key ends the demo */
        screen->input = 0;
        screen->done = 1;
        finished = 1;
    } else if (key == 27 || g_quitRequested) {
        screen->done = 1;
        finished = 1;
    }

    for (side = 0; side < 2; side++) {
        CharselScript *sc = &st->scripts[side];
        CharselCursor *cur = &st->cursors[side];

        for (;;) {
            CharselStep *step;
            int entered;
            int sel = -1;
            int advanced;

            if (sc->current >= sc->count) {
                st->side_done[side] = 1;
                break;
            }
            entered = sc->entered;
            step = &sc->steps[sc->current];
            sc->entered = 0;

            switch (step->type) {
            case CHARSEL_STEP_NONE:
            case CHARSEL_STEP_DONE:
            case CHARSEL_STEP_WAIT:
                advanced = CharselScript_Next(sc) != NULL;
                break;

            case CHARSEL_STEP_TEAM_MENU: {
                TeamMenu *menu = &st->menus[side];
                TeamMenuLayout *tl;
                if (entered) {
                    TeamMenu_Init(side, st, step);
                }
                if (g_game->continued == 0) {
                    if (screen->input_side == side) {
                        int old_item = menu->item;
                        int old_value = menu->items[old_item].value;
                        sel = TeamMenu_Move(menu, screen->input, screen->input_side, menu->player == 0 ? 1 : -1);
                        tl = &info->teammenu[menu->player];
                        if (old_item != menu->item) {
                            Select_PlaySnd(&tl->move_snd[0], &tl->move_snd[1], -1);
                        } else if (old_value != menu->items[menu->item].value) {
                            Select_PlaySnd(&tl->value_snd[0], &tl->value_snd[1], -1);
                        }
                    }
                } else {
                    sel = menu->item;
                }
                if (auto_play) {
                    if (sc->ticks < 60 && (sc->ticks + side * 4) % 8 == 0) {
                        int n = menu->count;
                        TeamMenuItem *it;
                        menu->item = (Random_Next() % (n - 1) + 1 + menu->item) % n;
                        it = &menu->items[menu->item];
                        it->value = it->max;
                        if (it->max > it->min) {
                            int lo = it->min;
                            it->value = Random_Next() % (it->max - lo + 1) + lo;
                        }
                        tl = &info->teammenu[menu->player];
                        Select_PlaySnd(&tl->move_snd[0], &tl->move_snd[1], -1);
                    }
                    if (sc->ticks >= 300) {
                        sel = menu->item;
                    }
                }
                if (sel == -1) {
                    goto stay;
                }
                if (menu->type == 0) {
                    CharselStep *target = step->target;
                    TeamMenuItem *it;
                    tl = &info->teammenu[menu->player];
                    if (g_game->continued == 0) {
                        Select_PlaySnd(&tl->done_snd[0], &tl->done_snd[1], -1);
                    }
                    it = &menu->items[menu->item];
                    switch (it->mode) {
                    case TEAM_SINGLE:
                        target->mode = TEAM_SINGLE;
                        target->count = 1;
                        break;
                    case TEAM_SIMUL:
                        target->mode = TEAM_SIMUL;
                        target->simul_mode = 0;
                        target->count = 2;
                        break;
                    case TEAM_TAG:
                        Error_Fatal("Not yet implemented");
                    case TEAM_TURNS:
                        target->mode = TEAM_TURNS;
                        target->count = it->value;
                        break;
                    default:
                        Error_Fatal("Programmer's fault");
                    }
                    tl->saved_item = menu->item;
                    tl->saved_value = menu->items[menu->item].value;
                }
                menu->active = 0;
                advanced = CharselScript_Next(sc) != NULL;
                break;
            }

            case CHARSEL_STEP_CHARS: {
                int swap = 0;
                int team, old_col, old_row, col;
                SelectCell *cell;
                CharselBox *box;
                if (entered) {
                    Charsel_StartChars(side, st, step);
                }
                if (step->mode == TEAM_SIMUL && step->count == 2 && cur->slot == 1 && step->simul_mode == 1) {
                    swap = 1;
                }
                if (cur->slot == -1) {
                    goto stay;
                }
                team = cur->team;
                old_col = cur->col;
                old_row = cur->row;
                sel = Charsel_MoveCursor(screen->input, screen->input_side, &cur->col, &cur->row,
                                         swap ? 1 - side : side);
                if (sel == -1) {
                    if (cur->auto_timer == 0) {
                        sel = Random_Next() % 12;
                        cur->auto_timer = Random_Next() % 10 + 10;
                    } else if (cur->auto_timer > 0 && screen->ticks % info->cell_random_switchtime == 0) {
                        cur->auto_timer--;
                    }
                }
                if (old_col != cur->col || old_row != cur->row) {
                    Select_PlaySnd(&info->cursor_move_snd[0][side], &info->cursor_move_snd[1][side], -1);
                }
                cell = NULL;
                col = cur->col;
                if (col != -1 && cur->row != -1 && &g_selectCells[cur->row * g_selectColumns + col] != NULL) {
                    int row = cur->row;
                    GameVars *g2 = g_game;
                    if (g2->char_count > 0) {
                        if (!auto_play && g_selectCells[row * g_selectColumns + col].char_index != SELECT_CELL_RANDOM) {
                            cur->sel_col = col;
                            cur->sel_row = row;
                        } else {
                            int ci = g_selectCells[cur->sel_row * g_selectColumns + cur->sel_col].char_index;
                            int t = info->cell_random_switchtime;
                            if (ci < 0) {
                                ci = Random_Next() % g2->char_count;
                                Select_FindCell(ci, &cur->sel_col, &cur->sel_row);
                            }
                            if (screen->ticks % t == t / 2 * side) {
                                if (g2->char_count != 1) {
                                    int n = g2->char_count;
                                    ci = (Random_Next() % (n - 1) + ci + 1) % n;
                                }
                                Select_FindCell(ci, &cur->sel_col, &cur->sel_row);
                                Select_PlaySnd(&info->random_move_snd[0][side], &info->random_move_snd[1][side],
                                               info->random_move_snd_cancel ? side : -1);
                            }
                            /* the cell under the random cursor (writes ci into it if FindCell failed) */
                            g_selectCells[cur->sel_row * g_selectColumns + cur->sel_col].char_index = ci;
                            if (auto_play) {
                                cur->col = cur->sel_col;
                                cur->row = cur->sel_row;
                            }
                        }
                    }
                    if (cur->sel_col != -1 && cur->sel_row != -1) {
                        cell = &g_selectCells[cur->sel_row * g_selectColumns + cur->sel_col];
                    }
                }
                box = &st->boxes[team][cur->slot];
                box->col = cur->col;
                box->row = cur->row;
                box->sel_col = cur->sel_col;
                box->sel_row = cur->sel_row;
                box->state = CHARSEL_BOX_ACTIVE;
                box->side = swap ? 1 - side : side;
                if (sel == -1 || cell == NULL || cell->char_index == SELECT_CELL_EMPTY) {
                    goto stay;
                }
                Select_PlaySnd(&info->cursor_done_snd[0][side], &info->cursor_done_snd[1][side], -1);
                st->boxes[team][cur->slot].state = CHARSEL_BOX_DONE;
                {
                    CursorMember *cm = &cur->members[cur->slot];
                    int m = cm->team;
                    TeamSel *ts = &st->teams[m];
                    TeamMember *member = &ts->members[ts->member_count];
                    TeamSlot *slot;
                    int k, r, c;
                    member->mode = cur->step->mode;
                    member->simul_mode = cur->step->simul_mode;
                    slot = &member->slots[member->count];
                    slot->char_index = cell->char_index;
                    slot->pal = g_game->char_info[cell->char_index].pal_keymap[sel] - 1;
                    slot->player = m + member->count * 2 + 1;
                    slot->ai = cur->members[cur->slot].ai;
                    slot->side = cur->members[cur->slot].side;
                    Log_Printf("Selected char %i on teamslot %i.%i\n", cell->char_index, m, member->count);
                    slot->pal = PalReserve_Request(slot->char_index, slot->pal);
                    k = cur->slot + step->team * 4;
                    if (g_game->arcade_side == 0) {
                        r = cur->row;
                        c = cur->col;
                    } else {
                        r = cur->sel_row;
                        c = cur->sel_col;
                    }
                    g_selectInfo->last_col[0][k] = c;
                    g_selectInfo->last_row[0][k] = r;
                    Charsel_FlashCell(&st->phase, cur->sel_col, cur->sel_row);
                    if (g_game->precache_enabled && ts->member_count == 0) {
                        Precache_QueueChar(member->slots[member->count].char_index);
                        Precache_Start();
                    }
                    member->count++;
                    cur->slot++;
                    cm = &cur->members[cur->slot];      /* may read past the 4 members */
                    if (cm->start_col != -1) {
                        cur->col = cm->start_col;
                        cur->sel_col = cm->start_col;
                    }
                    if (cm->start_row != -1) {
                        cur->sel_row = cm->start_row;
                        cur->row = cm->start_row;
                    }
                    ts->char_count++;
                    if (member->mode == TEAM_TURNS) {
                        if (cur->slot < cur->count) {
                            ts->member_count++;
                        }
                        if (member->mode == TEAM_TURNS) {
                            member->mode = TEAM_SINGLE;
                        }
                    }
                    if (cur->slot < cur->count) {
                        goto stay;
                    }
                    ts->member_count++;
                    st->team_done[m] = 1;
                }
                advanced = CharselScript_Next(sc) != NULL;
                break;
            }

            case CHARSEL_STEP_AUTO_TEAM: {
                TeamSel *ts = &st->teams[step->team];
                if (step->mode == TEAM_TURNS) {
                    TeamSel_Fill(step->count, ts, step->team, TEAM_SINGLE, 1);
                } else {
                    TeamSel_Fill(1, ts, step->team, step->mode, step->count);
                }
                if (step->mode == TEAM_SIMUL) {
                    ts->members[0].simul_mode = step->simul_mode;
                }
                st->team_done[step->team] = 1;
                advanced = CharselScript_Next(sc) != NULL;
                break;
            }

            case CHARSEL_STEP_STAGE: {
                int was_done, old_stage;
                if (entered) {
                    st->stage_select_shown = 1;
                }
                if (auto_play) {
                    st->stage_done = 1;
                    st->stage = -1;
                }
                was_done = st->stage_done;
                old_stage = st->stage;
                if (!was_done) {
                    st->stage_done = StageSelect_Move(&st->stage, screen->input, screen->input_side, side);
                }
                if (old_stage != st->stage) {
                    Select_PlaySnd(&info->stage_move_snd[0], &info->stage_move_snd[1], -1);
                }
                if (!was_done) {
                    if (!st->stage_done) {
                        goto stay;
                    }
                    Select_PlaySnd(&info->stage_done_snd[0], &info->stage_done_snd[1], -1);
                }
                if (!st->stage_done) {
                    goto stay;
                }
                advanced = CharselScript_Next(sc) != NULL;
                break;
            }

            default:
                goto stay;
            }
            if (!advanced) {
                break;
            }
            screen->input = 0;
        }
    stay:
        sc->ticks++;
    }

    if (st->side_done[0] && st->side_done[1] && st->phase != 1) {
        if (st->phase == 2) {
            goto fade_out;
        }
        st->phase = 1;
        st->phase_ticks = 0;
    }
    if (!finished) {
        return 1;
    }
fade_out:
    if (screen->fade_out == 0 || screen->fade_ticks == -1) {
        g = g_game;
        g->fade.type = 1;
        g->fade.ticks = 0;
        g->fade.time = info->fadeout_time;
        g->fade.out = 1;
        screen->fade_out = 1;
        screen->fade_ticks = info->fadeout_time;
    }
    return 1;
}

/* 0x00406DB0: fades the cell flashes; phase 2 once both sides are done and the flashes are over. */
int Charsel_Tick(ScreenLoop *screen, CharselState *st)
{
    if (st->flash[0].ticks > 0) {
        st->flash[0].ticks--;
    }
    if (st->flash[1].ticks > 0) {
        st->flash[1].ticks--;
    }
    st->phase_ticks++;
    if (st->phase == 1 && st->flash[0].ticks <= 0 && st->flash[1].ticks <= 0) {
        st->phase = 2;
    }
    return 1;
}

/*
 * Adds a system.def element to the motif draw list at (x, y) and draws it
 * this frame (inlined at every element of Charsel_Draw). Elements of other
 * layers are skipped.
 */
static void Select_DrawElem(const SysElem *elem, SpriteSet *sprites, Anim *anims, void **fonts, float x, float y,
                            float scale)
{
    SysDrawList *list;
    SysDrawItem *item;
    int idx;

    if (elem->layerno != 0) {
        return;
    }
    list = g_game->sysexplods;
    idx = SysElem_Add(elem, list, sprites, anims, fonts, x, y);
    if (idx >= 0) {
        SysElem_SetScale(idx, (float)g_game->motif_scale, scale);
    }
    if (idx == -1) {
        return;
    }
    item = &list->items[idx];
    if (item->started == 0 && item->kind == 1) {
        SysDraw_Start(list, idx);
    }
    if (g_game->skip_draw == 0) {
        SysDraw_Update(item);
    }
    item->drawn = 1;
    if (item->time <= 1 && item->time != -1) {
        item = &list->items[idx];
        if (item->used) {
            if (item->sprites != NULL) {
                SpriteSet_Free(item->sprites);
            }
            item->sprites = NULL;
            item->draw_sprites = 0;
            item->used = 0;
            list->count--;
        }
    }
}

/* Select_DrawElem with the motif's sprites, animations and fonts. */
static void Select_DrawMotifElem(const SysElem *elem, float x, float y)
{
    GameVars *g = g_game;

    Select_DrawElem(elem, g->motif_sprites, g->motif_anims, g->motif_fonts, x, y, 1.0f);
}

/* A cursor box to draw (list 0: done, list 1: active). */
typedef struct CharselBoxRef {
    int team;
    int side;
    int slot;
} CharselBoxRef;

/* 0x00406E10 */
int Charsel_Draw(ScreenLoop *screen, CharselState *st)
{
    GameVars *g = g_game;
    SelectInfo *info = &g->select_info;
    int active_slot[2] = { -1, -1 };
    int counts[2] = { 0, 0 };
    CharselBoxRef lists[2][8];
    char text[0x60];
    int list = 0;   /* uninitialised in the original; only used when a box has state 0 */
    int t, s, r, c, x, y;

    for (t = 0; t < 2; t++) {
        for (s = 0; s < 4; s++) {
            CharselBox *b = &st->boxes[t][s];
            if (b->col == -1 || b->row == -1) {
                b->cell = -1;
            } else {
                b->cell = b->row * g_selectColumns + b->col;
                if (b->state != CHARSEL_BOX_NONE) {
                    active_slot[t] = s;
                }
            }
            if (b->sel_col == -1 || b->sel_row == -1) {
                b->sel_cell = -1;
            } else {
                b->sel_cell = b->sel_row * g_selectColumns + b->sel_col;
            }
        }
    }
    for (t = 0; t < 2; t++) {
        for (s = 0; s < 4; s++) {
            CharselBox *b = &st->boxes[t][s];
            CharselBoxRef *ref;
            if (b->col == -1 || b->row == -1) {
                continue;
            }
            if (b->state == CHARSEL_BOX_DONE) {
                list = 0;
            } else if (b->state == CHARSEL_BOX_ACTIVE) {
                list = 1;
            }
            ref = &lists[list][counts[list]];
            ref->side = b->side;
            ref->team = t;
            ref->slot = s;
            counts[list]++;
        }
    }

    /* faces of the characters under the cursors */
    if (g->skip_draw == 0) {
        for (t = 0; t < 2; t++) {
            SelectCell *cell;
            if (active_slot[t] == -1) {
                continue;
            }
            cell = &g_selectCells[st->boxes[t][active_slot[t]].sel_cell];
            if (cell->sprites == NULL || KeyedArray_Find(cell->sprites->sprites, 9000, 1) == -1) {
                continue;
            }
            Select_DrawElem(&info->faces[t].face, cell->sprites, NULL, NULL, 0.0f, 0.0f, cell->scale);
        }
    }

    /* the grid */
    x = info->pos_x;
    y = info->pos_y;
    for (r = 0; r < g_selectRows; r++) {
        for (c = 0; c < g_selectColumns; c++) {
            SelectCell *cell = &g_selectCells[g_selectColumns * r + c];
            if (cell->char_index != SELECT_CELL_EMPTY || info->show_empty_boxes) {
                Select_DrawMotifElem(&info->cell_bg, (float)x, (float)y);
            }
            if (cell->char_index == SELECT_CELL_RANDOM) {
                float rx = info->portrait.x + x;
                float ry = info->portrait.y + y;
                Select_DrawMotifElem(&info->cell_random, rx, ry);
            }
            if (g_game->skip_draw == 0 && cell->sprites != NULL) {
                Select_DrawElem(&info->portrait, cell->sprites, NULL, NULL, (float)x, (float)y, cell->scale);
            }
            x += info->cell_spacing + info->cell_size[0];
        }
        x = info->pos_x;
        y += info->cell_size[1] + info->cell_spacing;
    }

    /* flashes of selected cells */
    g = g_game;
    if (g->skip_draw == 0) {
        for (t = 0; t < 2; t++) {
            CharselFlash *f = &st->flash[t];
            unsigned int color;
            int v, x1, y1, x2, y2;
            if (f->ticks <= 0) {
                continue;
            }
            v = f->ticks * 255 / f->length;
            if (g_renderMode == 2 || g_renderMode == 3) {
                color = ((v << 8 | v) << 8) | v;
            } else {
                KeyedArray *sprites = g_renderTarget->sprites;
                Sprite *target = (Sprite *)(sprites->data + sprites->elem_size * g_renderTargetSprite);
                SDL_Surface *surface;
                if (target == NULL || target->format != SPRITE_FORMAT_SURFACE) {
                    surface = NULL;     /* crashes below, as in the original */
                } else {
                    surface = target->pixels;
                }
                color = SDL_MapRGB(surface->format, (Uint8)v, (Uint8)v, (Uint8)v);
            }
            x1 = (info->cell_spacing + info->cell_size[0]) * f->col + info->pos_x;
            y1 = (info->cell_spacing + info->cell_size[1]) * f->row + info->pos_y;
            y2 = info->cell_size[1] + y1 - 2;
            x2 = info->cell_size[0] + x1 - 2;
            g = g_game;
            g_gfxFillRectColor((int)(x1 * g->motif_scale), (int)(y1 * g->motif_scale), (int)(x2 * g->motif_scale),
                          (int)(y2 * g->motif_scale), color);
        }
    }

    /* cursor boxes glide towards their cell */
    for (t = 0; t < 2; t++) {
        for (s = 0; s < counts[t]; s++) {
            CharselBoxRef *ref = &lists[t][s];
            CharselBox *b = &st->boxes[ref->team][ref->slot];
            float d;
            b->target_x = (float)((info->cell_spacing + info->cell_size[0]) * b->col + info->pos_x);
            b->target_y = (float)((info->cell_size[1] + info->cell_spacing) * b->row + info->pos_y);
            if (t == 0 || b->x == -FLT_MAX) {
                b->x = b->target_x;
                b->y = b->target_y;
                continue;
            }
            b->x = (float)(b->target_x * 0.5 + b->x * 0.5);
            d = (float)fabs(b->x - b->target_x);
            if (d < 1.0) {
                b->x = b->target_x;
            }
            b->y = (float)(b->target_y * 0.5 + b->y * 0.5);
            d = (float)fabs(b->y - b->target_y);
            if (d < 1.0) {
                b->y = b->target_y;
            }
        }
    }

    /* cursors: done ones first, then active ones (blinking when both share a cell) */
    for (t = 0; t < 2; t++) {
        for (s = 0; s < counts[t]; s++) {
            CharselBoxRef *ref = &lists[t][s];
            CharselBox *b = &st->boxes[ref->team][ref->slot];
            int bx = (int)b->x;
            int by = (int)b->y;
            if (t == 0) {
                Select_DrawMotifElem(&info->cursor_done[ref->side], (float)bx, (float)by);
            } else if (t == 1) {
                if (counts[1] == 2 && info->p2_cursor_blink) {
                    CharselBox *b1 = &st->boxes[lists[1][1].team][lists[1][1].slot];
                    CharselBox *b0 = &st->boxes[lists[1][0].team][lists[1][0].slot];
                    if (b0->col == b1->col && b0->row == b1->row) {
                        if (((screen->ticks & 7) < 4) == s) {
                            Select_DrawMotifElem(&info->cursor_active[ref->side], (float)bx, (float)by);
                        }
                        continue;
                    }
                }
                Select_DrawMotifElem(&info->cursor_active[ref->side], (float)bx, (float)by);
            }
        }
    }

    /* names, then the sprites queued by the elements */
    g = g_game;
    if (g->skip_draw == 0) {
        for (t = 0; t < 2; t++) {
            FaceLayout *face = &info->faces[t];
            x = face->name_offset[0];
            y = face->name_offset[1];
            for (s = 0; s < 4; s++) {
                int idx = st->boxes[t][s].cell;
                const SelectCell *cell;
                const char *name;
                if (idx == -1) {
                    continue;
                }
                cell = &g_selectCells[idx];
                name = cell->name;
                if (cell->name[0] == '\0') {
                    if (cell->char_index != SELECT_CELL_RANDOM) {
                        continue;
                    }
                    name = "Random";
                }
                FontSpec_DrawText(&face->name_font, name, x, y);
                x += face->name_spacing[0];
                y += face->name_spacing[1];
            }
        }
        g = g_game;
        if (g->skip_draw == 0) {
            SpriteQueue *queue = g->sprite_layer1;
            KeyedArray *items = queue->items;
            PalFx *fx = g_palFxCur;
            PalRemap *remap = g_palRemapCur;
            int i;
            for (i = 0; i < items->count; i++) {
                SpriteQueueItem *it = (SpriteQueueItem *)(queue->order[i] * items->elem_size + items->data);
                if (it == NULL) {
                    continue;
                }
                g_palFxCur = it->palfx;
                g_palRemapCur = it->remap;
                if (it->sprites != NULL) {
                    if (g_renderMode == 2) {
                        Sprite_DrawGL(it->sprites, it->sprite, &it->dp);
                    } else {
                        Sprite_DrawSoft(it->sprites, it->sprite, &it->dp);
                    }
                }
                g_palFxCur = fx;
                g_palRemapCur = remap;
            }
        }
    }

    /* stage */
    g = g_game;
    if (st->stage_select_shown && g->skip_draw == 0) {
        if (st->stage == -1) {
            sprintf(text, "Stage: Random");
        } else {
            sprintf(text, "Stage %i: %s", st->stage + 1, g->select_screen[st->stage].name);
        }
        if (st->stage_done == 0) {
            FontSpec_DrawText(&info->stage_active_font[screen->ticks / 2 & 1], text, info->stage_pos[0],
                              info->stage_pos[1]);
        } else {
            FontSpec_DrawText(&info->stage_done_font, text, info->stage_pos[0], info->stage_pos[1]);
        }
    }

    /* team menus */
    for (t = 0; t < 2; t++) {
        TeamMenu *menu = &st->menus[t];
        TeamMenuLayout *tl;
        int i;
        if (!menu->active) {
            continue;
        }
        tl = &info->teammenu[menu->player];
        x = tl->pos_x;
        y = tl->pos_y;
        Select_DrawMotifElem(&tl->bg, (float)x, (float)y);
        if (t == menu->player) {
            Select_DrawMotifElem(&tl->self_title, (float)x, (float)y);
        } else {
            Select_DrawMotifElem(&tl->enemy_title, (float)x, (float)y);
        }
        x += tl->item_offset[0];
        y += tl->item_offset[1];
        for (i = 0; i < menu->count; i++) {
            const TeamMenuItem *it = &menu->items[i];
            if (i == menu->item) {
                Select_DrawMotifElem(&tl->item_cursor, (float)x, (float)y);
                FontSpec_DrawText(&tl->item_active_font[screen->ticks / 2 & 1], it->name, x, y);
            } else {
                FontSpec_DrawText(&tl->item_font, it->name, x, y);
            }
            if (it->min <= it->max) {
                int sx = x, sy = y;
                int k;
                for (k = 0; k < it->max; k++) {
                    if (k < it->value) {
                        Select_DrawMotifElem(&tl->value_icon, (float)x, (float)y);
                    } else {
                        Select_DrawMotifElem(&tl->value_empty_icon, (float)x, (float)y);
                    }
                    x += tl->value_spacing[0];
                    y += tl->value_spacing[1];
                }
                x = sx;
                y = sy;
            }
            x += tl->item_spacing[0];
            y += tl->item_spacing[1];
        }
    }

    Select_DrawMotifElem(&info->sysexplod0, 0.0f, 0.0f);

    g = g_game;
    if (g->skip_draw == 0 && g->game_mode >= GAME_MODE_ARCADE && g->game_mode <= GAME_MODE_WATCH) {
        FontSpec_DrawText(&info->title_font, mode_titles[g->game_mode], info->title_offset[0], info->title_offset[1]);
    }
    return 1;
}

/* 0x004089E0: the select screen loop; returns 0 when the state cannot be allocated. */
int Charsel_Setup(ScreenLoop *screen)
{
    memset(screen, 0, sizeof(*screen));
    screen->scale = 1.0f;
    screen->fadein_time = 10;
    screen->fadeout_time = 10;
    screen->kind = 2;
    screen->fps = 60;
    screen->reload_motif = 1;
    screen->start_frame = g_game->frames;
    screen->state = calloc(1, sizeof(CharselState));
    if (screen->state == NULL) {
        return 0;
    }
    screen->init = (ScreenFn)Charsel_Init;
    screen->end = (ScreenFn)Charsel_End;
    screen->update = (ScreenFn)Charsel_Update;
    screen->tick = (ScreenFn)Charsel_Tick;
    screen->draw = (ScreenFn)Charsel_Draw;
    screen->overlay = NULL;
    return 1;
}

/* 0x00408A80: returns the next gameflow state: 8 to go on, 4 when cancelled or failed. */
int Charsel_Run(void)
{
    ScreenLoop screen;
    int result = 8;
    int cancelled;

    Log_Printf("Entering character select.\n");
    Charsel_Setup(&screen);     /* the result is not checked */
    if (!Screen_Run(&screen)) {
        result = 4;
    }
    cancelled = screen.done;
    if (screen.state != NULL) {
        free(screen.state);
    }
    memset(&screen, 0, sizeof(screen));
    if (cancelled) {
        Log_Printf("Canceled character select.\n");
        return 4;
    }
    return result;
}

/* Fills step k of a script. */
static void Charsel_SetStep(CharselScript *sc, int k, int type, int team, int mode, int count)
{
    CharselStep *step = &sc->steps[k];

    step->type = type;
    step->team = team;
    step->mode = mode;
    step->count = count;
}

/* Makes step k of a script a team menu for player that fills step k + 1. */
static void Charsel_SetMenuStep(CharselScript *sc, int k, int player, unsigned int flags, int item)
{
    CharselStep *step = &sc->steps[k];

    step->type = CHARSEL_STEP_TEAM_MENU;
    step->menu_type = 0;
    step->menu_flags = flags;
    step->menu_item = item;
    step->player = player;
    step->target = &sc->steps[k + 1];
}

#define MENU_ALL    0x0B    /* single, simul, turns */
#define MENU_NO_TURNS 0x03  /* single, simul */

/*
 * 0x00408B20: builds the select scripts of both sides for a game mode chosen
 * by side. Steps not set stay as they are (CHARSEL_STEP_NONE after Charsel_Init).
 */
void Charsel_SetupMode(CharselState *st, int mode, int side)
{
    GameVars *g = g_game;
    CharselScript *sc = &st->scripts[side];
    CharselScript *p1 = &st->scripts[0];
    CharselScript *p2 = &st->scripts[1];

    sc->steps[0].type = CHARSEL_STEP_NONE;
    g->arcade_side = 0;
    g->survival_side = 0;
    g->team_variant = 0;
    switch (mode) {
    case GAME_MODE_ARCADE:
        Charsel_SetStep(sc, 1, CHARSEL_STEP_CHARS, side, TEAM_SINGLE, 1);
        Charsel_SetStep(sc, 2, CHARSEL_STEP_AUTO_TEAM, 1 - side, TEAM_SINGLE, 1);
        sc->steps[3].type = CHARSEL_STEP_DONE;
        sc->count = 4;
        g->arcade_side = side + 1;
        break;
    case GAME_MODE_VERSUS:
        Charsel_SetStep(p1, 1, CHARSEL_STEP_CHARS, 0, TEAM_SINGLE, 1);
        p1->steps[2].type = CHARSEL_STEP_STAGE;
        p1->steps[3].type = CHARSEL_STEP_DONE;
        p1->count = 4;
        Charsel_SetStep(p2, 4, CHARSEL_STEP_CHARS, 1, TEAM_SINGLE, 1);
        p2->steps[5].type = CHARSEL_STEP_STAGE;
        p2->steps[6].type = CHARSEL_STEP_DONE;
        p2->count = 7;
        break;
    case GAME_MODE_TEAM_ARCADE:
        Charsel_SetMenuStep(sc, 1, side, MENU_ALL, 1);
        Charsel_SetStep(sc, 2, CHARSEL_STEP_CHARS, side, TEAM_SIMUL, 2);
        sc->steps[2].simul_mode = 0;
        Charsel_SetMenuStep(sc, 3, 1 - side, MENU_ALL, 1);
        Charsel_SetStep(sc, 4, CHARSEL_STEP_AUTO_TEAM, 1 - side, TEAM_SIMUL, 2);
        sc->steps[4].simul_mode = 2;
        sc->steps[5].type = CHARSEL_STEP_DONE;
        sc->count = 6;
        g->arcade_side = side + 1;
        break;
    case GAME_MODE_TEAM_VERSUS:
        Charsel_SetMenuStep(p1, 1, 0, MENU_ALL, 1);
        Charsel_SetStep(p1, 2, CHARSEL_STEP_CHARS, 0, TEAM_SIMUL, 2);
        p1->steps[2].simul_mode = 0;
        p1->steps[3].type = CHARSEL_STEP_STAGE;
        p1->steps[4].type = CHARSEL_STEP_DONE;
        p1->count = 5;
        Charsel_SetMenuStep(p2, 5, 1, MENU_ALL, 1);
        Charsel_SetStep(p2, 6, CHARSEL_STEP_CHARS, 1, TEAM_SIMUL, 2);
        p2->steps[6].simul_mode = 0;
        p2->steps[7].type = CHARSEL_STEP_STAGE;
        p2->steps[8].type = CHARSEL_STEP_DONE;
        p2->count = 9;
        break;
    case GAME_MODE_TEAM_COOP:
        Charsel_SetStep(sc, 1, CHARSEL_STEP_CHARS, side, TEAM_SIMUL, 2);
        sc->steps[1].simul_mode = 1;
        Charsel_SetMenuStep(sc, 2, 1 - side, MENU_ALL, 1);
        Charsel_SetStep(sc, 3, CHARSEL_STEP_AUTO_TEAM, 1 - side, TEAM_SIMUL, 2);
        sc->steps[3].simul_mode = 2;
        sc->steps[4].type = CHARSEL_STEP_DONE;
        sc->count = 5;
        g->arcade_side = side + 1;
        break;
    case GAME_MODE_SURVIVAL:
        Charsel_SetMenuStep(sc, 1, side, MENU_ALL, 0);
        Charsel_SetStep(sc, 2, CHARSEL_STEP_CHARS, side, TEAM_SIMUL, 2);
        sc->steps[2].simul_mode = 0;
        Charsel_SetMenuStep(sc, 3, 1 - side, MENU_NO_TURNS, 0);
        Charsel_SetStep(sc, 4, CHARSEL_STEP_AUTO_TEAM, 1 - side, TEAM_SIMUL, 2);
        sc->steps[4].simul_mode = 2;
        sc->steps[5].type = CHARSEL_STEP_STAGE;
        sc->steps[6].type = CHARSEL_STEP_DONE;
        sc->count = 7;
        goto survival;
    case GAME_MODE_SURVIVAL_COOP:
        Charsel_SetStep(sc, 1, CHARSEL_STEP_CHARS, side, TEAM_SIMUL, 2);
        sc->steps[1].simul_mode = 1;
        Charsel_SetMenuStep(sc, 2, 1 - side, MENU_NO_TURNS, 0);
        Charsel_SetStep(sc, 3, CHARSEL_STEP_AUTO_TEAM, 1 - side, TEAM_SIMUL, 2);
        sc->steps[3].simul_mode = 2;
        sc->steps[4].type = CHARSEL_STEP_STAGE;
        sc->steps[5].type = CHARSEL_STEP_DONE;
        sc->count = 6;
    survival:
        g = g_game;
        g->team_variant = 1;
        g->survival_side = side + 1;
        g->arcade_side = 0;
        break;
    case GAME_MODE_TRAINING:
        Charsel_SetStep(sc, 1, CHARSEL_STEP_CHARS, side, TEAM_SINGLE, 1);
        Charsel_SetStep(sc, 2, CHARSEL_STEP_CHARS, 1 - side, TEAM_SINGLE, 1);
        sc->steps[3].type = CHARSEL_STEP_STAGE;
        sc->steps[4].type = CHARSEL_STEP_DONE;
        sc->count = 5;
        g->team_variant = 3;
        break;
    case GAME_MODE_WATCH:
        Charsel_SetMenuStep(sc, 1, side, MENU_ALL, 0);
        Charsel_SetStep(sc, 2, CHARSEL_STEP_CHARS, side, TEAM_SINGLE, 1);
        Charsel_SetMenuStep(sc, 3, 1 - side, MENU_ALL, 0);
        Charsel_SetStep(sc, 4, CHARSEL_STEP_CHARS, 1 - side, TEAM_SINGLE, 1);
        sc->steps[5].type = CHARSEL_STEP_STAGE;
        sc->steps[6].type = CHARSEL_STEP_DONE;
        sc->count = 7;
        g_game->team_variant = 2;
        break;
    }
}

/* 0x00409040: phase points into the CharselState. */
void Charsel_ResetBoxes(int *phase)
{
    CharselState *st = (CharselState *)((char *)phase - offsetof(CharselState, phase));
    int t, s;

    for (t = 0; t < 2; t++) {
        for (s = 0; s < 4; s++) {
            CharselBox *b = &st->boxes[t][s];
            b->y = -FLT_MAX;
            b->row = -1;
            b->x = -FLT_MAX;
            b->col = -1;
            b->cell = -1;
        }
    }
    st->flash[0].length = 4;
    st->flash[1].length = 4;
}

/* 0x00409090: starts a flash on the cell, using the flash slot that is further along. */
void Charsel_FlashCell(int *phase, int col, int row)
{
    CharselState *st = (CharselState *)((char *)phase - offsetof(CharselState, phase));
    int limit = 999;
    int k;

    if (st->flash[0].ticks < limit) {
        limit = st->flash[0].ticks;
    }
    k = st->flash[1].ticks < limit ? 1 : 0;
    st->flash[k].ticks = st->flash[k].length + 1;
    st->flash[k].col = col;
    st->flash[k].row = row;
}

/* Cell at (col, row) has no character. */
#define CELL_EMPTY_AT(col, row) (g_selectCells[(row) * g_selectColumns + (col)].char_index == SELECT_CELL_EMPTY)

/*
 * 0x004090E0: moves a character cursor; returns the button pressed (0..5,
 * plus 6 when start is held) on a cell with a character, else -1. Empty
 * cells are skipped unless moveoveremptyboxes is set.
 */
int Charsel_MoveCursor(unsigned int input, int input_side, int *col, int *row, int side)
{
    int cols = g_selectColumns;
    int result = -1;
    int c, last, tries;
    unsigned int code, key;

    if (input_side != side || g_game->char_count == 0) {
        return -1;
    }
    if (input & SCREEN_IN_LEFT) {
        c = *col;
        last = c;
        for (tries = cols;;) {
            if (!CELL_EMPTY_AT(c, *row)) {
                last = c;
            }
            c--;
            if (c < 0) {
                if (!g_selectInfo->wrapping) {
                    c = last;
                    break;
                }
                c = cols - 1;
            }
            if (!CELL_EMPTY_AT(c, *row) || g_selectInfo->move_over_empty_boxes || --tries == 0) {
                break;
            }
        }
        *col = c;
    }
    if (input & SCREEN_IN_RIGHT) {
        c = *col;
        last = c;
        for (tries = cols;;) {
            if (!CELL_EMPTY_AT(c, *row)) {
                last = c;
            }
            c++;
            if (c >= cols) {
                if (!g_selectInfo->wrapping) {
                    c = last;
                    break;
                }
                c = 0;
            }
            if (!CELL_EMPTY_AT(c, *row) || g_selectInfo->move_over_empty_boxes || --tries == 0) {
                break;
            }
        }
        *col = c;
    }
    if (input & SCREEN_IN_UP) {
        c = *row;
        last = c;
        for (tries = g_selectRows;;) {
            if (!CELL_EMPTY_AT(*col, c)) {
                last = c;
            }
            c--;
            if (c < 0) {
                if (!g_selectInfo->wrapping) {
                    c = last;
                    break;
                }
                c = g_selectRows - 1;
            }
            if (!CELL_EMPTY_AT(*col, c) || g_selectInfo->move_over_empty_boxes || --tries == 0) {
                break;
            }
        }
        *row = c;
    }
    if (input & SCREEN_IN_DOWN) {
        c = *row;
        last = c;
        for (tries = g_selectRows;;) {
            if (!CELL_EMPTY_AT(*col, c)) {
                last = c;
            }
            c++;
            if (c >= g_selectRows) {
                if (!g_selectInfo->wrapping) {
                    c = last;
                    break;
                }
                c = 0;
            }
            if (!CELL_EMPTY_AT(*col, c) || g_selectInfo->move_over_empty_boxes || --tries == 0) {
                break;
            }
        }
        *row = c;
    }
    if (!CELL_EMPTY_AT(*col, *row)) {
        if (input & 0x010) {
            result = 0;
        }
        if (input & 0x020) {
            result = 1;
        }
        if (input & 0x040) {
            result = 2;
        }
        if (input & 0x080) {
            result = 3;
        }
        if (input & 0x100) {
            result = 4;
        }
        if (input & 0x200) {
            result = 5;
        }
    }
    /* start held: palettes 7..12 */
    code = (unsigned int)g_game->input_keys[side][KEY_START];
    key = code & ~0x200u;
    if (key == 0x200 || key == 0) {
        key = 0x280;
    } else if (code & 0x200) {
        key |= 0x1000;
    }
    if (g_inputState[key] && result != -1) {
        result += 6;
    }
    return result;
}

/* 0x004093C0: moves the stage cursor (-1 = random); returns 1 when a button was pressed. */
int StageSelect_Move(int *stage, unsigned int input, int input_side, int side)
{
    int s = *stage;
    GameVars *g;

    if (input_side != side) {
        return 0;
    }
    g = g_game;
    if (input & SCREEN_IN_LEFT) {
        s--;
        if (s < -1) {
            s = g->stage_count - 1;
        }
    }
    if (input & SCREEN_IN_UP) {
        s -= 10;
        if (s < -1) {
            s = -1;
        }
    }
    if (input & SCREEN_IN_RIGHT) {
        s++;
        if (s >= g->stage_count) {
            s = -1;
        }
    }
    if (input & SCREEN_IN_DOWN) {
        s += 10;
        if (s >= g->stage_count) {
            s = g->stage_count - 1;
        }
    }
    *stage = s;
    return (input & SCREEN_IN_BUTTONS) != 0;
}

/*
 * 0x00409430: up/down choose the item, the sides change its value (dir -1
 * swaps them for player 2). Returns the item when a button was pressed, else -1.
 */
int TeamMenu_Move(TeamMenu *menu, unsigned int input, int input_side, int dir)
{
    unsigned int inc = SCREEN_IN_RIGHT;
    unsigned int dec = SCREEN_IN_LEFT;
    SelectInfo *info = g_selectInfo;
    TeamMenuItem *it;

    if (dir == -1) {
        inc = SCREEN_IN_LEFT;
        dec = SCREEN_IN_RIGHT;
    }
    if (input & SCREEN_IN_UP) {
        menu->item--;
        if (menu->item < 0) {
            menu->item = info->teammenu_wrapping ? menu->count - 1 : 0;
        }
    }
    if (input & SCREEN_IN_DOWN) {
        menu->item++;
        if (menu->item >= menu->count) {
            menu->item = info->teammenu_wrapping ? 0 : menu->count - 1;
        }
    }
    it = &menu->items[menu->item];
    if (it->max > it->min) {
        if ((inc & input) && it->value < it->max) {
            it->value++;
        }
        if ((dec & input) && it->value > it->min) {
            it->value--;
        }
    }
    if (input & SCREEN_IN_BUTTONS) {
        return menu->item;
    }
    return -1;
}

/* "key = group, number" for a sound; the group is -1 when the key is missing. */
static void Select_ReadSnd(Ini *ini, const char *key, int *group, int *number)
{
    *group = -1;
    Ini_Scanf(ini, key, "%i,%i", group, number);
}

/* "key = n" */
static void Select_ReadInt(Ini *ini, const char *key, int *out)
{
    const char *value = Ini_GetValue(ini, key);

    if (value != NULL) {
        *out = atoi(value);
    }
}

/*
 * 0x00409500: reads [Select Info] (selected in ini) into GameVars.select_info
 * and allocates the grid. The fonts are read inline in the original (same
 * code as FontSpec_Read).
 */
int Select_LoadLayout(Ini *ini)
{
    SelectInfo *info;
    char key[0x100];
    int p;

    g_selectInfo = &g_game->select_info;
    g_selectInfo->cell_random_switchtime = 4;
    info = g_selectInfo;
    Select_ReadInt(ini, "fadein.time", &info->fadein_time);
    Select_ReadInt(ini, "fadeout.time", &info->fadeout_time);
    if (info->fadein_time < 1) {
        info->fadein_time = 1;
    }
    if (info->fadeout_time < 1) {
        info->fadeout_time = 1;
    }
    Select_ReadInt(ini, "rows", &info->rows);
    Select_ReadInt(ini, "columns", &info->columns);
    Select_ReadInt(ini, "wrapping", &info->wrapping);
    Ini_Scanf(ini, "pos", "%i,%i", &info->pos_x, &info->pos_y);
    Select_ReadInt(ini, "showemptyboxes", &info->show_empty_boxes);
    Select_ReadInt(ini, "moveoveremptyboxes", &info->move_over_empty_boxes);
    Ini_Scanf(ini, "p1.cursor.startcell", "%i,%i", &info->start_row[0], &info->start_col[0]);
    Ini_Scanf(ini, "p2.cursor.startcell", "%i,%i", &info->start_row[1], &info->start_col[1]);
    Select_ReadInt(ini, "p2.cursor.blink", &info->p2_cursor_blink);
    Ini_Scanf(ini, "cell.size", "%i,%i", &info->cell_size[0], &info->cell_size[1]);
    Select_ReadInt(ini, "cell.spacing", &info->cell_spacing);
    SysElem_Read(&info->portrait, "portrait", ini);
    if (info->portrait.type == SYSELEM_NONE) {
        info->portrait.scale_y = 1.0f;
        info->portrait.scale_x = 1.0f;
        info->portrait.group = 9000;
        info->portrait.number = 0;
        info->portrait.type = SYSELEM_SPRITE;
        info->portrait.displaytime = 1;
        Ini_Scanf(ini, "portrait.scale", "%f,%f", &info->portrait.scale_x, &info->portrait.scale_y);
        Ini_Scanf(ini, "portrait.offset", "%f,%f", &info->portrait.x, &info->portrait.y);
    }
    Ini_Scanf(ini, "title.offset", "%i,%i", &info->title_offset[0], &info->title_offset[1]);
    FontSpec_Read(&info->title_font, ini, "title.font");
    Select_ReadSnd(ini, "p1.cursor.move.snd", &info->cursor_move_snd[0][0], &info->cursor_move_snd[1][0]);
    Select_ReadSnd(ini, "p2.cursor.move.snd", &info->cursor_move_snd[0][1], &info->cursor_move_snd[1][1]);
    Select_ReadSnd(ini, "p1.cursor.done.snd", &info->cursor_done_snd[0][0], &info->cursor_done_snd[1][0]);
    Select_ReadSnd(ini, "p2.cursor.done.snd", &info->cursor_done_snd[0][1], &info->cursor_done_snd[1][1]);
    Select_ReadSnd(ini, "p1.random.move.snd", &info->random_move_snd[0][0], &info->random_move_snd[1][0]);
    Select_ReadSnd(ini, "p2.random.move.snd", &info->random_move_snd[0][1], &info->random_move_snd[1][1]);
    Select_ReadInt(ini, "random.move.snd.cancel", &info->random_move_snd_cancel);
    Select_ReadSnd(ini, "stage.move.snd", &info->stage_move_snd[0], &info->stage_move_snd[1]);
    Select_ReadSnd(ini, "stage.done.snd", &info->stage_done_snd[0], &info->stage_done_snd[1]);
    Select_ReadSnd(ini, "cancel.snd", &info->cancel_snd[0], &info->cancel_snd[1]);
    SysElem_Read(&info->cell_bg, "cell.bg", ini);
    SysElem_Read(&info->cell_random, "cell.random", ini);
    Select_ReadInt(ini, "cell.random.switchtime", &info->cell_random_switchtime);
    if (info->cell_random_switchtime < 1) {
        info->cell_random_switchtime = 1;
    }
    for (p = 1; p <= 2; p++) {
        sprintf(key, "p%i.cursor.active", p);
        SysElem_Read(&info->cursor_active[p - 1], key, ini);
        sprintf(key, "p%i.cursor.done", p);
        SysElem_Read(&info->cursor_done[p - 1], key, ini);
    }
    for (p = 1; p <= 2; p++) {
        FaceLayout *f = &info->faces[p - 1];
        sprintf(key, "p%i.face", p);
        SysElem_Read(&f->face, key, ini);
        if (f->face.type == SYSELEM_NONE) {
            f->face.scale_y = 1.0f;
            f->face.scale_x = 1.0f;
            f->face.facing = p == 1 ? 1 : -1;
            sprintf(key, "p%i.face.offset", p);
            f->face.group = 9000;
            f->face.number = 1;
            f->face.type = SYSELEM_SPRITE;
            Ini_Scanf(ini, key, "%f,%f", &f->face.x, &f->face.y);
            sprintf(key, "p%i.face.scale", p);
            Ini_Scanf(ini, key, "%f,%f", &f->face.scale_x, &f->face.scale_y);
            sprintf(key, "p%i.face.facing", p);
            Ini_Scanf(ini, key, "%i", &f->face.facing);
        }
        f->name_spacing[0] = 0;
        f->name_spacing[1] = 14;
        sprintf(key, "p%i.name.offset", p);
        Ini_Scanf(ini, key, "%i,%i", &f->name_offset[0], &f->name_offset[1]);
        sprintf(key, "p%i.name.font", p);
        FontSpec_Read(&f->name_font, ini, key);
        sprintf(key, "p%i.name.spacing", p);
        Ini_Scanf(ini, key, "%i,%i", &f->name_spacing[0], &f->name_spacing[1]);
    }
    Ini_Scanf(ini, "stage.pos", "%i,%i", &info->stage_pos[0], &info->stage_pos[1]);
    FontSpec_Read(&info->stage_active_font[0], ini, "stage.active.font");
    FontSpec_Read(&info->stage_active_font[1], ini, "stage.active2.font");
    FontSpec_Read(&info->stage_done_font, ini, "stage.done.font");
    Select_ReadInt(ini, "teammenu.move.wrapping", &info->teammenu_wrapping);
    for (p = 1; p <= 2; p++) {
        TeamMenuLayout *t = &info->teammenu[p - 1];
        sprintf(key, "p%i.teammenu.pos", p);
        Ini_Scanf(ini, key, "%i,%i", &t->pos_x, &t->pos_y);
        sprintf(key, "p%i.teammenu.bg", p);
        SysElem_Read(&t->bg, key, ini);
        sprintf(key, "p%i.teammenu.selftitle", p);
        SysElem_Read(&t->self_title, key, ini);
        sprintf(key, "p%i.teammenu.enemytitle", p);
        SysElem_Read(&t->enemy_title, key, ini);
        sprintf(key, "p%i.teammenu.move.snd", p);
        Select_ReadSnd(ini, key, &t->move_snd[0], &t->move_snd[1]);
        sprintf(key, "p%i.teammenu.value.snd", p);
        Select_ReadSnd(ini, key, &t->value_snd[0], &t->value_snd[1]);
        sprintf(key, "p%i.teammenu.done.snd", p);
        Select_ReadSnd(ini, key, &t->done_snd[0], &t->done_snd[1]);
        sprintf(key, "p%i.teammenu.item.font", p);
        FontSpec_Read(&t->item_font, ini, key);
        sprintf(key, "p%i.teammenu.item.active.font", p);
        FontSpec_Read(&t->item_active_font[0], ini, key);
        sprintf(key, "p%i.teammenu.item.active2.font", p);
        FontSpec_Read(&t->item_active_font[1], ini, key);
        sprintf(key, "p%i.teammenu.item.offset", p);
        Ini_Scanf(ini, key, "%i,%i", &t->item_offset[0], &t->item_offset[1]);
        sprintf(key, "p%i.teammenu.item.spacing", p);
        Ini_Scanf(ini, key, "%i,%i", &t->item_spacing[0], &t->item_spacing[1]);
        sprintf(key, "p%i.teammenu.item.cursor", p);
        SysElem_Read(&t->item_cursor, key, ini);
        sprintf(key, "p%i.teammenu.value.icon", p);
        SysElem_Read(&t->value_icon, key, ini);
        sprintf(key, "p%i.teammenu.value.empty.icon", p);
        SysElem_Read(&t->value_empty_icon, key, ini);
        sprintf(key, "p%i.teammenu.value.spacing", p);
        Ini_Scanf(ini, key, "%i,%i", &t->value_spacing[0], &t->value_spacing[1]);
    }
    SysElem_Read(&info->sysexplod0, "sysexplod0", ini);
    info = g_selectInfo;
    info->cells = calloc(info->columns * info->rows, sizeof(SelectCell));
    if (info->cells == NULL) {
        Error_Printf("Out of memory\n");
        return 0;
    }
    return 1;
}

/* ---- arcade order ---- */

/* The computer's team in the arcade modes. */
static TeamSel *Arcade_CpuTeam(GameVars *g)
{
    return &g->teams[2 - g->arcade_side];
}

/* 0x0040A2B0: arcade order of opponents for the computer team's size. */
void Arcade_BuildOrder(void)
{
    GameVars *g = g_game;
    TeamSel *cpu;
    const int *max_matches;
    int side = g->arcade_side;
    int m, k, n;

    if (side == 0) {
        return;
    }
    cpu = Arcade_CpuTeam(g);
    /* counts up to 4 characters of the human team; the result is not used */
    n = 0;
    for (m = 0; m < g->teams[side - 1].member_count && n < 4; m++) {
        for (k = 0; k < g->teams[side - 1].members[m].count && n < 4; k++) {
            n++;
        }
    }
    max_matches = g->arcade_maxmatches;
    if (cpu->members[0].mode != TEAM_SINGLE || cpu->member_count > 1) {
        max_matches = g->team_maxmatches;
    }
    Arcade_ShuffleOrder((ArcadeOrder *)&g->char_info_2, cpu->char_count, max_matches);
}

/*
 * 0x0040A370: shuffles the characters, sorts them by order (stable bubble
 * sort), then takes max_matches[order] * team_size characters of each order
 * in turn, padding the last team with random characters of the same order.
 */
void Arcade_ShuffleOrder(ArcadeOrder *order, int team_size, const int *max_matches)
{
    GameVars *g = g_game;
    int n = g->char_count;
    int *idx = calloc(n + 1, sizeof(int));
    int out = 0;
    int i, pass, level, sorted;

    for (i = 0; i < n; i++) {
        idx[i] = i;
    }
    for (pass = 0; pass < 5; pass++) {
        for (i = 0; i < n; i++) {
            int r = Random_Next() % n;
            int t = idx[i];
            idx[i] = idx[r];
            idx[r] = t;
        }
    }
    sorted = 0;
    if (n >= 2) {
        int last = n - 1;
        do {
            CharInfo *ci;
            if (sorted) {
                break;
            }
            sorted = 1;
            ci = g->char_info;
            for (i = 0; i < last; i++) {
                int a = idx[i];
                if (ci[a].order > ci[idx[i + 1]].order) {
                    idx[i] = idx[i + 1];
                    idx[i + 1] = a;
                    sorted = 0;
                }
            }
            last--;
        } while (last + 1 >= 2);
    }
    level = 0;
    i = 0;
    while (i < n) {
        int start, k;
        while (g->char_info[idx[i]].order < level) {
            i++;
            if (i >= n) {
                goto done;
            }
        }
        start = i;
        k = 0;
        if (*max_matches * team_size > 0) {
            while (i < n && g->char_info[idx[i]].order == level) {
                order->chars[out++] = idx[i];
                k++;
                i++;
                if (k >= *max_matches * team_size) {
                    break;
                }
            }
        }
        while (k % team_size != 0 && i != start) {
            int r = Random_Next();
            k++;
            order->chars[out++] = idx[r % (i - start) + start];
        }
        if (i >= n) {
            break;
        }
        max_matches++;
        level++;
        if (level >= SELECT_MAX_MATCHES) {
            break;
        }
    }
done:
    order->chars[out] = -1;
    order->count = out;
    order->next = 0;
    order->wins = 0;
    free(idx);
}

/*
 * 0x0040A620: fills the computer team with the next characters of the arcade
 * order (the position is not advanced here). Returns the last character
 * placed, or -1.
 */
int Arcade_NextTeam(void)
{
    GameVars *g = g_game;
    int **order = &g->char_info_2;
    int pos = g->arcade_order_next;
    TeamSel *cpu = Arcade_CpuTeam(g);
    int last = -1;
    int m, k;

    for (m = 0; m < cpu->member_count; m++) {
        TeamMember *member = &cpu->members[m];
        for (k = 0; k < member->count; k++) {
            TeamSlot *slot = &member->slots[k];
            int ci;
            if ((*order)[pos] == -1) {
                continue;
            }
            if (slot->pal != -1) {
                PalReserve_Release(slot->char_index, slot->pal);
            }
            g = g_game;
            ci = (*order)[pos];
            slot->char_index = ci;
            last = ci;
            if (g->ai_random_color) {
                slot->pal = Random_Next() % SELECT_PAL_COUNT;
            } else {
                slot->pal = g->char_info[ci].pal_defaults[0] - 1;
            }
            slot->pal = PalReserve_Request(ci, slot->pal);
            slot->ai = 1;
            pos++;
        }
    }
    return last;
}

/* 0x0040A780: characters of the arcade order the computer team takes next. */
int Arcade_CountNext(void)
{
    GameVars *g = g_game;
    int pos = g->arcade_order_next;
    TeamSel *cpu = Arcade_CpuTeam(g);
    int members = cpu->member_count;
    int n = 0;
    int m, k;

    for (m = 0; m < members; m++) {
        const int *p = &g->char_info_2[pos];
        for (k = cpu->members[m].count; k > 0; k--) {
            if (*p != -1) {     /* an entry of -1 is checked again, not skipped */
                pos++;
                p++;
                n++;
            }
        }
    }
    return n;
}

/*
 * 0x0040A7F0: copies the characters of the computer's next team (the one
 * after it with skip_current) to out; returns how many, 0 past the end.
 */
int Arcade_PeekNext(int *out, int skip_current)
{
    GameVars *g = g_game;
    TeamSel *cpu = Arcade_CpuTeam(g);
    int pos = g->arcade_order_next;
    int n = 0;
    int members, m, k;

    if (skip_current) {
        pos += Arcade_CountNext();
        g = g_game;
    }
    if (pos >= g->arcade_order_count) {
        return 0;
    }
    members = cpu->member_count;
    for (m = 0; m < members; m++) {
        TeamMember *member = &cpu->members[m];
        const int *p;
        if (member->count <= 0) {
            continue;
        }
        p = &g->char_info_2[pos];
        for (k = member->count; k > 0; k--) {
            if (*p != -1) {
                out[n] = *p;
                pos++;
                p++;
                n++;
            }
        }
    }
    return n;
}

/*
 * 0x0040A8A0: survival: random characters for a team of two members. With
 * keep == 0 the second member moves to the first and only the second is
 * drawn anew.
 */
void Team_Randomize(TeamSel *team, int side, int keep)
{
    int count = team->members[0].count;
    int m, k;

    team->member_count = 2;
    team->char_count = count * 2;
    team->cur_member = 0;
    if (keep == 0) {
        TeamMember *first = &team->members[0];
        for (k = 0; k < first->count; k++) {
            TeamSlot *slot = &first->slots[k];
            if (slot->pal != -1) {
                PalReserve_Release(slot->char_index, slot->pal);
            }
        }
        team->members[0] = team->members[1];
        memcpy(team->member_data[0], team->member_data[1], sizeof(team->member_data[0]));
        for (k = 0; k < 4; k++) {
            memset(&team->member_data[1][k].vars, 0, sizeof(PersistVars));
        }
    }
    for (m = keep == 0; m < 2; m++) {
        TeamMember *member = &team->members[m];
        int player = side + 1;
        if (keep != 0 || m == 1) {
            member->slots[1].pal = -1;
            member->slots[0].pal = -1;
            member->slots[1].char_index = -1;
            member->slots[0].char_index = -1;
        }
        member->mode = count != 1;
        if (count == 2) {
            member->simul_mode = 2;
        }
        member->count = count;
        for (k = 0; k < count; k++) {
            TeamSlot *slot = &member->slots[k];
            GameVars *g = g_game;
            int ci;
            slot->side = side;
            slot->player = player;
            ci = Random_Next() % g->char_count;
            slot->char_index = ci;
            if (g->ai_random_color) {
                slot->pal = Random_Next() % SELECT_PAL_COUNT;
            } else {
                slot->pal = g->char_info[ci].pal_defaults[0] - 1;
            }
            slot->pal = PalReserve_Request(ci, slot->pal);
            slot->ai = 1;
            player += 2;
        }
    }
}

/* 0x0040AAE0: grid position of a character's cell; returns 0 when it has none. */
int Select_FindCell(int char_index, int *col, int *row)
{
    int n = g_selectRows * g_selectColumns;
    int i;

    for (i = 0; i < n; i++) {
        if (g_selectCells[i].char_index == char_index) {
            *col = i % g_selectColumns;
            *row = i / g_selectColumns;
            return 1;
        }
    }
    return 0;
}

/* 0x0040AB30: enters the next step of a script; NULL at the end. */
CharselStep *CharselScript_Next(CharselScript *sc)
{
    if (sc->current >= sc->count) {
        return NULL;
    }
    sc->current++;
    sc->entered = 1;
    sc->ticks = 0;
    return &sc->steps[sc->current];
}

/* 0x0040AB70: sets up the cursor of side for a CHARSEL_STEP_CHARS step. */
void Charsel_StartChars(int side, CharselState *st, const CharselStep *step)
{
    GameVars *g = g_game;
    int auto_play = g->stresstest != 0 || g->demo_mode != 0;
    CharselCursor *cur = &st->cursors[side];
    SelectInfo *info;
    int k;

    cur->team = step->team;
    cur->slot = 0;
    cur->count = step->count;
    cur->step = (CharselStep *)step;
    cur->auto_timer = -1;
    if (auto_play) {
        cur->auto_timer = Random_Next() % 20 + 20;
        g_selectInfo->cell_random_switchtime = 6;
    }
    for (k = 0; k < cur->count; k++) {
        CursorMember *m = &cur->members[k];
        m->side = step->team;
        m->team = cur->team;
        m->start_col = -1;
        m->start_row = -1;
        if (g->team_variant == 2) {
            m->ai = 1;
        }
    }
    info = g_selectInfo;
    for (k = 0; k < 4; k++) {
        if (info->last_col[step->team][k] != -1) {
            cur->members[k].start_col = info->last_col[step->team][k];
        }
        if (info->last_row[step->team][k] != -1) {
            cur->members[k].start_row = info->last_row[step->team][k];
        }
    }
    cur->col = cur->members[0].start_col;
    if (cur->col == -1) {
        cur->col = g_selectInfo->start_col[step->team];
    }
    cur->row = cur->members[0].start_row;
    if (cur->row == -1) {
        cur->row = g_selectInfo->start_row[step->team];
    }
    cur->sel_row = cur->row;
    cur->sel_col = cur->col;
    if (step->mode == TEAM_SIMUL && step->count == 2) {
        if (step->simul_mode == 0 || step->simul_mode == 2) {
            cur->members[1].ai = 1;
        }
        if (step->simul_mode == 2) {
            cur->members[0].ai = 1;
        }
        if (step->simul_mode == 1) {
            cur->members[1].side = 1 - cur->members[1].side;
        }
    }
}

/* 0x0040AD00: a team chosen without input: count members of per_member computer-controlled slots. */
void TeamSel_Fill(int count, TeamSel *team, int side, int mode, int per_member)
{
    int m, k;

    team->member_count = count;
    for (m = 0; m < team->member_count; m++) {
        TeamMember *member = &team->members[m];
        member->count = per_member;
        member->mode = mode;
        for (k = 0; k < member->count; k++) {
            TeamSlot *slot = &member->slots[k];
            slot->player = side + 1 + 2 * k;
            slot->ai = 1;
            slot->char_index = -1;
            slot->side = side;
            slot->pal = -1;
            team->char_count++;
        }
    }
}

/* 0x0040AD80: opens the team menu of side with the modes allowed by the step. */
void TeamMenu_Init(int side, CharselState *st, const CharselStep *step)
{
    TeamMenu *menu = &st->menus[side];
    unsigned int flags;
    TeamMenuLayout *tl;
    int n = 0;

    menu->active = 1;
    menu->player = step->player;
    menu->type = step->menu_type;
    menu->item = step->menu_item;
    flags = step->menu_flags;
    if (menu->type != 0) {
        Error_Fatal("Menu - programmer's fault");
    }
    if (flags & 1) {
        menu->items[0].value = -1;
        menu->items[0].min = 0;
        menu->items[0].mode = TEAM_SINGLE;
        menu->items[0].name = "Single";
        menu->items[0].max = -1;
        n = 1;
    }
    if (flags & 2) {
        TeamMenuItem *it = &menu->items[n++];
        it->value = 2;
        it->min = 2;
        it->mode = TEAM_SIMUL;
        it->name = "Simul";
        it->max = 2;
    }
    if (flags & 4) {
        TeamMenuItem *it = &menu->items[n++];
        it->value = 2;
        it->min = 2;
        it->mode = TEAM_TAG;
        it->name = "Tag";
        it->max = 2;
    }
    if (flags & 8) {
        TeamMenuItem *it = &menu->items[n++];
        it->value = 2;
        it->min = 2;
        it->mode = TEAM_TURNS;
        it->name = "Turns";
        it->max = 4;
    }
    menu->count = n;
    tl = &g_selectInfo->teammenu[menu->player];
    if (tl->saved_item != -1) {
        menu->item = tl->saved_item;
        menu->items[tl->saved_item].value = tl->saved_value;
    }
}

/* ---- palette reservation: two copies of a character never get the same palette ---- */

/*
 * 0x0040AEA0: reserves palette pal (0-based) of a character, or the next
 * free one: first among 0..5 (pal 7..12 fall back to 1..6), then 6..11;
 * palette 0 when all are taken.
 */
int PalReserve_Request(int char_index, int pal)
{
    CharInfo *ci = &g_game->char_info[char_index];
    unsigned int free_mask = ci->pal_free;
    int tries;

    Log_Printf("Char %s (%i) request pal %i %X (%X) ->", ci->def_file, char_index, pal, ci->pal_mask, free_mask);
    if (!(free_mask & (1u << (pal & 31))) && pal >= 6) {
        pal -= 6;
    }
    if ((free_mask & (1u << (pal & 31))) && pal < 12) {
        goto reserve;
    }
    tries = 0;
    do {
        pal++;
        if (pal >= 6) {
            pal = 0;
        }
        tries++;
        if (free_mask & (1u << (pal & 31))) {
            if (tries < 6) {
                goto found;
            }
            break;
        }
    } while (tries < 6);
    do {
        pal++;
        if (pal >= 12) {
            pal = 6;
        }
        tries++;
        if (free_mask & (1u << (pal & 31))) {
            goto found;
        }
    } while (tries < 12);
    pal = 0;
    goto reserve;
found:
    if (tries >= 12) {
        pal = 0;
    }
reserve:
    free_mask &= ~(1u << (pal & 31));
    Log_Printf(" reserved %i (%X)\n", pal, free_mask);
    g_game->char_info[char_index].pal_free = free_mask;
    return pal;
}

/* 0x0040AF90: gives a palette back (only if the character has it). */
void PalReserve_Release(int char_index, int pal)
{
    CharInfo *ci = &g_game->char_info[char_index];
    unsigned int mask = ci->pal_mask;

    Log_Printf("Char %s (%i) unreserve pal %i %X %X -> ", ci->def_file, char_index, pal, mask, ci->pal_free);
    g_game->char_info[char_index].pal_free |= (1u << (pal & 31)) & mask;
    Log_Printf("%X\n", g_game->char_info[char_index].pal_free);
}

/* 0x0040B010 */
void PalReserve_ResetAll(void)
{
    GameVars *g = g_game;
    int i;

    for (i = 0; i < g->char_count; i++) {
        g->char_info[i].pal_free = g->char_info[i].pal_mask;
    }
    Log_Printf("Unreserved all palettes\n");
}
