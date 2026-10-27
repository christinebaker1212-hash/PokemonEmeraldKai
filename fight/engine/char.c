/*
 * Characters (original module 0x434B20-0x43BF3F): the player position
 * helpers and per-frame input processing at the start of the module, the
 * player cache, the character loader (.def, constants, .cmd, states, sprites,
 * animations, sounds, palettes, learned AI), player creation, reset and
 * freeing, the missing sprite/animation report, the persistent variables,
 * turns member rotation and the active player range. The player state
 * machine follows at 0x43BF40.
 *
 * The constants are loaded twice: into the Lua table of the character
 * (CharData.lua_ref, field "const", with defaults and limits) and into
 * CharData.consts, which every player copies.
 */
#include "mugen.h"

#define LUA_REGISTRYINDEX   (-10000)
#define LUA_TTABLE          5

/* FLT_MAX as a double, the "no limit" of the constants. */
#define CONST_MAX           ((double)3.402823466e+38F)

/* The coordinate unit of the loaded stage. */
#define STAGE_UNIT(g)       ((g)->stage.unit)

/* ------------------------------------------------------------------ player runtime helpers */

/*
 * 0x00434B20: origin of a "postype" position (explods, helpers, projectiles)
 * relative to p. space 1 is screen space, 0 game space (other values: the
 * left/right edges are not divided by GameVars.camera_zoom). bind, when given,
 * receives a reference to the player for p1/p2.
 */
void Player_GetPosOrigin(PlayerRef *bind, Player *p, int postype, Player **p2, int space, float *x, float *y,
                         float *offset_x, float *offset_y, int *facing)
{
    GameVars *g = g_game;
    float ox = 0.0f, px = 0.0f, py = 0.0f, oy = 0.0f;
    float width;
    int dir = 1;
    int right;
    Coord c;

    if (bind != NULL) {
        bind->valid = 0;
    }
    switch (postype) {
    case POSTYPE_P1:
    case POSTYPE_P2:
        if (postype == POSTYPE_P2) {
            if (*p2 != NULL) {
                p = *p2;
            } else {
                Player_Warn(p, "tried to use relative pos of non-existent p2");
            }
        }
        dir = p->facing;
        if (bind != NULL) {
            PlayerRef_Set(bind, p, 1);
        }
        if (space == 1) {
            ox = p->draw.x;
            oy = p->draw.y;
        } else {
            px = (float)p->pos.x;
            py = (float)p->pos.y;
        }
        break;
    case POSTYPE_FRONT:
    case POSTYPE_BACK:
        dir = p->facing;
        /* fall through */
    case POSTYPE_LEFT:
    case POSTYPE_RIGHT:
        width = (float)(int)g->game_coord[0];
        if (space == 0) {
            width = width / g->camera_zoom;
        }
        if (postype == POSTYPE_BACK) {
            right = p->facing == 1;
        } else if (postype == POSTYPE_FRONT) {
            right = p->facing == -1;
        } else {
            right = postype == POSTYPE_LEFT;
        }
        if (space == 1) {
            py = 0.0f;
            px = 0.0f;
        } else {
            float cam = (float)(g_gameCoord[0] / STAGE_UNIT(g) * g->camera_x + g->camera_offset_x);
            Coord_MakeUnit(&c, cam, 0.0);
            px = (float)c.x;
            py = (float)c.y;
            ox = (float)(-width * 0.5);
        }
        if (!right) {
            ox = (float)((width - 1.0) + ox);
        }
        break;
    default:
        /* POSTYPE_NONE and unknown values: everything 0 */
        break;
    }
    if (x != NULL) {
        *x = px;
    }
    if (y != NULL) {
        *y = py;
    }
    if (offset_x != NULL) {
        *offset_x = ox;
    }
    if (offset_y != NULL) {
        *offset_y = oy;
    }
    if (facing != NULL) {
        *facing = dir;
    }
}

/* 0x00434D50: flips the input directions to the facing (InputDirMode 1: to the side of the nearest enemy). */
void Player_UpdateInputFacing(Player *p)
{
    int flipped;

    if (g_game->input_dir_mode == 1 && p->enemy != NULL) {
        flipped = p->pos.x > p->enemy->pos.x;
    } else {
        flipped = p->facing == -1;
    }
    Input_SetFlipped(&p->input, flipped);
}

/*
 * 0x00434DB0: per-frame input of a player: buttons that skip the round intro
 * and the end of a round, the command buffers, and the automatic state
 * changes from held directions (walk, crouch, jump, air jump, guard).
 */
void Player_UpdateInput(Player *p)
{
    InputBuf *in = &p->input;
    GameVars *g;
    unsigned int key;
    int keep_crouch = 0;
    int change;
    int floor, i;

    if (in->read == in->write) {
        key = 0;
    } else {
        key = in->ring[in->read];
        in->read++;
        if (in->read >= PLAYER_INPUT_RING_SIZE) {
            in->read = 0;
        }
    }
    g = g_game;
    if (!p->ai) {
        if (g->screen_kind != 6 && g->screen_kind != 7 && g->screen_kind != 8 &&
            (g->round_state == 1 || (g->round_state == 0 && g->round_state_time > 2)) && key - 5 <= 5 &&
            g->intro_time <= g->fight.round.round_time + 2 && g->intro_skipped == 0) {
            g->intro_skipped = 1;
            g->fade.out = 1;
            g->fade.type = 2;
            g->fade.ticks = 0;
            g->fade.time = g->round_no <= 1 ? 12 : 6;
            g->fade.bar_size = g_targetHeight / 8;
        }
        if ((g->screen_kind == 6 || g->screen_kind == 7) && g->skip_pressed == 1 && key - 5 <= 5) {
            g->skip_pressed = 2;
            g->unused_1281C = 0;
        }
        if (g->screen_kind != 6 && g->screen_kind != 7 && g->screen_kind != 8 && g->round_state == 4 &&
            g->round_state_time > g->fight.round.over_wintime) {
            int skip_to = g->over_end_time - 17;
            if (g->round_state_time < skip_to && key - 5 <= 5) {
                g->round_state = 4;
                g->round_state_time = skip_to;
                g->over_skipped = 1;
            }
        }
    }
    if (p->suspended != 0) {
        return;
    }
    if (p->root != NULL && p->no_keyctrl != 0) {
        return;
    }
    if (p->no_keyctrl == 0 && g->input_blocked == 0) {
        if (p->input_cleared != 0) {
            Input_Resync(&p->input, p->input_keys);
            p->input_cleared = 0;
        }
    } else {
        key = 0;
        Input_Clear(&p->input);
        p->input_cleared = 1;
    }
    if (g->input_enabled == 0) {
        return;
    }

    change = -1;
    if (p->hitpause_time > 0) {
        p->cmd_state.frozen = 1;
    }
    if (p->no_keyctrl == 0) {
        p->cmd_state.held = Input_GetHeld(&p->input);
    } else {
        p->cmd_state.held = 0;
    }
    floor = 0;
    if (g->pause_kind != -1 && p->pause_movetime[g->pause_kind] == 0 &&
        g->pauses[g->pause_kind][6] < g->pauses[g->pause_kind][8]) {
        floor = 2;
    }
    if (p->cmd_state.frozen == 0) {
        for (i = 0; i < CMD_MAX_LABELS; i++) {
            unsigned char t = p->cmd_active[i];
            if (t != 0) {
                t--;
                p->cmd_active[i] = t;
                if ((int)t < floor) {
                    p->cmd_active[i] = (unsigned char)floor;
                }
            }
        }
    }
    if (p->root != NULL && p->root->ai != 0) {
        int frozen = p->cmd_state.frozen;
        p->cmd_state = p->root->cmd_state;
        p->cmd_state.frozen = frozen;
    } else {
        CmdState_AddInput(key, &p->cmd_state, &p->input, g_game->frames);
        g = g_game;
    }
    {
        /* both passes start from the same history position */
        int pos = p->cmd_state.pos;
        CmdState_Update(p->cmds, p->cmd_active, &p->cmd_state, g->frames, CMD_TYPE_NORMAL);
        p->cmd_state.pos = pos;
        CmdState_Update(&p->data->cmds, p->cmd_active, &p->cmd_state, g->frames, CMD_TYPE_RECOVERY);
    }
    Ai_UpdatePlayer(p);
    if (p->hitpause_time == 0 && g->player_tick[p->index] != 0 && p->cmd_state.frozen == 1) {
        p->cmd_state.frozen = 0;
    }

    /* held directions: 1 walk, 7 jump, 10 air jump, 11 crouch, 40 guard */
    if (p->statetype == STATETYPE_STAND) {
        int fwd = p->input.keys[KEY_RIGHT], back = p->input.keys[KEY_LEFT];
        if (fwd != 0 && back == 0) {
            change = 1;
        }
        if (back != 0 && fwd == 0) {
            change = 1;
        }
        if (p->input.keys[KEY_CROUCH] != 0) {
            change = 11;
        }
        if (p->input.keys[KEY_JUMP] != 0) {
            change = 7;
        }
    }
    if (p->statetype == STATETYPE_CROUCH && p->input.keys[KEY_CROUCH] != 0) {
        keep_crouch = 1;
    }
    if (p->statetype == STATETYPE_AIR && key == 1) {
        change = 10;
    }
    p->in_guard_dist = 0;
    if (g->game_type_m == 0 && p->input.keys[KEY_LEFT] != 0 &&
        ((p->movetype == MOVETYPE_IDLE && (p->ctrl != 0 || p->state_no == 52)) || p->state_no == 151 ||
         p->state_no == 153 || p->state_no == 155) &&
        !(p->statetype == STATETYPE_STAND && p->nostandguard) &&
        !(p->statetype == STATETYPE_CROUCH && p->nocrouchguard) &&
        !(p->statetype == STATETYPE_AIR && p->noairguard) && p->input.keys[KEY_JUMP] == 0 &&
        g_game->players_first <= g_game->players_last) {
        GameVars *gv = g_game;
        for (i = gv->players_first; i <= g_game->players_last; i++) {
            Player *e = gv->players[i];
            if (p == e || e == NULL || !e->active || e->team_side == p->team_side || e->team_side == 0) {
                continue;
            }
            if (e->movetype == MOVETYPE_ATTACK || (*(KeyedArray **)e->projectiles)->count > 0) {
                p->in_guard_dist = 1;
                if (Player_InGuardDist(e, p)) {
                    change = 40;
                }
            }
        }
    }
    if (p->input.keys[KEY_RIGHT] != 0 && p->input.keys[KEY_LEFT] != 0) {
        change = -1;
    }
    if (p->no_input_states != 0) {
        memset(p->cmd_active, 0, sizeof(p->cmd_active));
        change = -1;
        keep_crouch = 0;
    }
    switch (change) {
    case 1:
        if (p->ctrl != 0 && !p->nowalk && p->state_no != 20) {
            Player_ChangeState(20, p);
        }
        break;
    case 7:
        if (p->ctrl != 0 && g_game->round_state != 3) {
            Player_ChangeState(40, p);
        }
        break;
    case 10:
        if (p->ctrl != 0 && p->air_jumps > 0 && (double)p->consts.airjump_height < -p->pos.y) {
            Player_ChangeState(45, p);
            p->air_jumps--;
        }
        break;
    case 11:
        if (p->ctrl != 0) {
            if (p->state_no != 100) {
                p->vel.x = 0.0;
            }
            if (p->state_no != 10 && p->state_no != 11) {
                Player_ChangeState(10, p);
            }
        }
        break;
    case 40:
        if (p->movetype == MOVETYPE_IDLE && p->ctrl != 0) {
            int s = p->state_no;
            if (s != 130 && s != 131 && s != 132 && s != 120 && s != 140) {
                if (p->statetype != STATETYPE_AIR && s != 100) {
                    p->vel.x = 0.0;
                }
                Player_ChangeState(120, p);
            }
            p->in_guard_dist = 1;
        }
        break;
    case 900:
        /* never selected above */
        Player_Turn(p);
        if (p->anim->anim->actions->slots[p->anim->action].key2 != 5) {
            Player_ChangeAnimSelf(p, 5);
        }
        break;
    default:
        if (p->state_no == 20) {
            p->prev_state_no = 20;
            p->state_no = 0;
            p->state_time = 0;
        }
        if (p->statetype == STATETYPE_CROUCH && p->ctrl != 0 && !keep_crouch) {
            Player_ChangeState(12, p);
        }
        break;
    }
    if (key != 0 && p->movetype == MOVETYPE_HIT) {
        if (p->statetype == STATETYPE_LIEDOWN) {
            p->gethit_recovertime = (int)(p->gethit_recovertime * 0.75);
        } else if (p->gethit_fallcount > 0) {
            p->gethit_recovertime = (int)(p->gethit_recovertime * 0.9);
        }
    }
}

/* ------------------------------------------------------------------ player cache */

/* 0x004355D0: the loaded data of select.def character index, or NULL. */
CharData *CharCache_Get(int index)
{
    if (index >= 0 && index < g_game->char_count) {
        return g_game->char_info[index].data;
    }
    return NULL;
}

/* Saves the learned AI of a cached character next to its .def (when the game directory is writable). */
static void PlayerCache_SaveAi(CharInfo *entry, char *path)
{
    CharData *c = entry->data;

    if (g_game->cwd_writable && c->ai_file[0] != '\0' && c->ai.tables[0] != NULL) {
        Path_Join(c->ai_file, entry->dir, path);
        if (!Ai_Save(path, &entry->data->ai)) {
            TextRing_Printf(g_game->clipboard, "Cannot save AI info for %s", entry->def_file);
        }
    }
}

/*
 * 0x00435600: the data of character index, loaded if needed. When the cache
 * is full, the least recently released unused character is freed; if there is
 * none the cache grows by one.
 */
CharData *PlayerCache_Load(int index)
{
    GameVars *g = g_game;
    CharInfo *info = g->char_info;
    CharInfo *entry = &info[index];
    char path[0x200];
    long long start;
    double ms;
    int best, best_serial, i, elapsed;

    if (entry->data != NULL) {
        entry->cache_count++;
        Log_Printf("Player %s cache count increased: %i\n", entry->data->name, entry->cache_count);
        return entry->data;
    }
    if (g->chars_cached >= g->player_cache) {
        best = -1;
        best_serial = -1;
        for (i = 0; i < g->char_count; i++) {
            CharInfo *e = &info[i];
            if (e->cache_count == 0 && e->data != NULL && (best == -1 || e->cache_serial < best_serial)) {
                best_serial = e->cache_serial;
                best = i;
            }
        }
        if (best != -1) {
            CharInfo *e = &g->char_info[best];
            Log_Printf("Player %s purged from cache: %i\n", e->data->name, e->cache_serial);
            PlayerCache_SaveAi(&g->char_info[best], path);
            Char_Free(g->char_info[best].data);
            g->char_info[best].data = NULL;
            g->chars_cached--;
        } else {
            g_game->player_cache++;
        }
    }
    start = Timer_GetTicks();
    entry->data = Char_Load(entry->dir, entry->def_file);
    if (entry->data == NULL) {
        Path_Join(entry->def_file, entry->dir, path);
        Error_Printf("Error loading %s\n", path);
        return NULL;
    }
    elapsed = (int)Timer_GetTicks() - (int)start;
    ms = (double)elapsed * 1000.0 / g_timerTicksPerSecond;
    g->char_info[index].data->char_index = index;
    g->char_info[index].cache_count = 1;
    g->chars_cached++;
    Log_Printf("New char %s loaded into cache: (%i/%i cached) Load time: %.3fms\n", g->char_info[index].data->name,
               g->chars_cached, g_game->player_cache, ms);
    return g->char_info[index].data;
}

/* 0x00435870: one user less; an unused character remembers when it was released (serial + counter). */
void PlayerCache_Release(int index, int serial)
{
    CharInfo *entry = &g_game->char_info[index];

    if (entry->cache_count > 0) {
        entry->cache_count--;
        Log_Printf("Player %s cache count reduced: %i\n", entry->data->name, entry->cache_count);
        entry = &g_game->char_info[index];
        if (entry->cache_count == 0) {
            entry->cache_serial = serial + g_playerCacheSerial;
            g_playerCacheSerial++;
        }
    }
}

/* 0x004358E0: frees every cached character (saving learned AI data first). */
void PlayerCache_Clear(void)
{
    GameVars *g = g_game;
    char path[0x200];
    int i;

    for (i = 0; i < g->char_count; i++) {
        if (g->char_info[i].data != NULL) {
            PlayerCache_SaveAi(&g->char_info[i], path);
            Char_Free(g->char_info[i].data);
            g->char_info[i].data = NULL;
            g->char_info[i].cache_count = 0;
            g->chars_cached--;
        }
    }
}

/* 0x004359E0: 1 when a character can be loaded without growing the cache. */
int PlayerCache_HasRoom(void)
{
    GameVars *g = g_game;
    int i;

    if (g->chars_cached < g->player_cache) {
        return 1;
    }
    for (i = 0; i < g->char_count; i++) {
        if (g->char_info[i].cache_count == 0 && g->char_info[i].data != NULL) {
            return 1;
        }
    }
    return 0;
}

/* ------------------------------------------------------------------ character loader */

/* 0x00435A30: loads def (in directory dir, or in the zip file dir). */
CharData *Char_Load(const char *dir, const char *def)
{
    char path[0x200];
    CharData *c;
    Stream *s;

    Path_Join(def, dir, path);
    if (File_GetType(dir) == FILETYPE_ZIP) {
        s = Stream_OpenZip(dir, def);
        if (s == NULL) {
            return NULL;
        }
        Log_Printf("Loading zipped character %s:%s...\n", dir, def);
        c = Char_LoadDef(dir, s);
    } else {
        s = Stream_OpenFile(path, "rt");
        if (s == NULL) {
            Error_Printf("Can't open %s\n", def);
            return NULL;
        }
        Log_Printf("Loading character %s...\n", path);
        c = Char_LoadDef(dir, s);
    }
    Stream_Close(s);
    if (c != NULL) {
        Log_Printf("  %i expressions (%i on trigger lines)\n", c->expr_count, c->trigger_expr_count);
    }
    Log_Printf("Character %s %s", def, c != NULL ? "loaded OK" : "failed to load");
    Log_Printf("\n");
    return c;
}

/*
 * Opens a file of the character for parsing: from the zip stream (which is
 * reused for every file) or from dir. Returns the stream, NULL when the file
 * cannot be opened (a zip stream is returned even when the entry is missing).
 */
static Stream *Char_OpenFile(int is_zip, Stream *zs, const char *dir, const char *name, char *path,
                             const char *mode)
{
    if (is_zip) {
        Stream_ZipUnload(zs);
        Stream_ZipLoad(zs, name);
        return zs;
    }
    Path_Join(name, dir, path);
    return Stream_OpenFile(path, mode);
}

/* "Error in <file>[:<line>]\n" after a parse error in ini. */
static void Char_ReportError(const char *file, Ini *ini, int debug)
{
    Error_Printf("Error in %s", file);
    if (debug) {
        Error_Printf(":%i", Ini_SourceLine(ini->cursor, ini));
    }
    Error_Printf("\n");
}

/* After a failed state file: says why when the mugenversion differs from the engine's. */
static void Char_ReportVersion(const CharData *c)
{
    if (c->mugenversion < g_game->compat_version[0]) {
        Error_Printf("Character mugenversion is older than this version of M.U.G.E.N.\n");
    } else if (c->mugenversion > g_game->compat_version[0]) {
        Error_Printf("Character mugenversion is newer than this version of M.U.G.E.N.\n");
    }
}

/*
 * 0x00435B30: loads a character from its .def stream s. Returns NULL on
 * errors; as in the original most error paths leak what was loaded so far
 * (only a failed first state file frees the character).
 */
CharData *Char_LoadDef(const char *dir, Stream *s)
{
    char buf[0x200];
    char path[0x200];
    char key[0x100];
    Palette pal;
    unsigned char colors[0x400];
    long tell;
    int debug, mode, is_zip;
    Stream *zs;
    Ini *def, *ini;
    CharData *c;
    struct lua_State *L;
    int w, h, i, status;

    tell = Stream_Tell(s);
    debug = g_game->allow_debug_mode;
    mode = debug ? INI_MODE_EDIT : INI_MODE_READ;
    is_zip = s->type == STREAM_ZIP;
    zs = is_zip ? s : NULL;
    def = Ini_Parse(s, INI_MODE_READ);
    if (def == NULL) {
        return NULL;
    }
    c = calloc(1, sizeof(CharData));
    if (c == NULL) {
        g_outOfMemory = 1;
        return NULL;
    }
    L = g_luaState;
    lua_createtable(L, 0, 0);
    c->lua_ref = luaL_ref(L, LUA_REGISTRYINDEX);

    /* [Info] */
    Log_Printf("  Loading info...");
    if (Ini_SelectGroup(def, "Info") == -1) {
        Error_Printf("Can't access [Info]\n");
        return NULL;
    }
    if (!Ini_ReadString(def, "name", sizeof(c->name), c->name)) {
        return NULL;
    }
    if (Ini_GetValue(def, "displayname") != NULL) {
        if (!Ini_ReadString(def, "displayname", sizeof(c->displayname), c->displayname)) {
            return NULL;
        }
    } else {
        c->displayname[0] = '\0';
        strncat(c->displayname, c->name, sizeof(c->displayname) - 1);
    }
    if (Ini_GetValue(def, "author") != NULL && !Ini_ReadString(def, "author", sizeof(c->author), c->author)) {
        return NULL;
    }
    c->versiondate = Version_ParseDate(Ini_GetValue(def, "versiondate"));
    c->mugenversion = Version_Parse(Ini_GetValue(def, "mugenversion"));
    if (c->mugenversion < g_game->compat_version[1]) {
        Log_Printf("Info: %s loading in pre-1.0 compatible mode\n", c->displayname);
    }
    Log_Printf("OK\n");
    w = 320;
    h = 240;
    Ini_Scanf(def, "localcoord", "%i,%i", &w, &h);
    if (w < 1) {
        w = 1;
    }
    if (h < 1) {
        h = 1;
    }
    c->localcoord[0] = w;
    c->localcoord[1] = h;
    c->localcoord[2] = (w - 1) / 2;
    c->localcoord[3] = (h - 1) / 2;

    /* [Files]: commands */
    if (Ini_SelectGroup(def, "Files") == -1) {
        Error_Printf("Can't access [Files]\n");
        return NULL;
    }
    if (Ini_GetString(def, "cmd", buf, sizeof(buf)) != 1 || buf[0] == '\0') {
        return NULL;
    }
    Log_Printf("  Loading cmd command set %s...", buf);
    zs = Char_OpenFile(is_zip, zs, dir, buf, path, "rt");
    ini = Ini_Parse(zs, mode);
    if (ini == NULL) {
        Error_Printf("Can't read file %s\n", buf);
        return NULL;
    }
    if (!Char_LoadCmd(ini, &c->cmds)) {
        Char_ReportError(buf, ini, debug);
        return NULL;
    }
    Ini_Free(ini);
    if (!is_zip) {
        Stream_Close(zs);
        zs = NULL;
    }
    Log_Printf("OK\n");

    /* constants */
    if (Ini_GetString(def, "cns", buf, sizeof(buf)) != 1 || buf[0] == '\0') {
        return NULL;
    }
    Log_Printf("  Loading cns %s...", buf);
    zs = Char_OpenFile(is_zip, zs, dir, buf, path, "rt");
    ini = Ini_Parse(zs, mode);
    if (ini == NULL) {
        Error_Printf("Can't read file: %s\n", buf);
        return NULL;
    }
    if (!is_zip) {
        Stream_Close(zs);
        zs = NULL;
    }
    if (!Char_LoadConstants(c, ini)) {
        Char_ReportError(buf, ini, debug);
        return NULL;
    }

    /* states: from the constants file unless "st" names another one */
    {
        const char *st = Ini_GetValue(def, "st");
        if (st == NULL) {
            return NULL;
        }
        if (strcmp(st, buf) != 0) {
            Ini_Free(ini);
            strcpy(buf, st);
            zs = Char_OpenFile(is_zip, zs, dir, buf, path, "rt");
            if (zs == NULL) {
                Error_Printf("\nCould not open file: %s\n", buf);
                return NULL;
            }
            ini = Ini_Parse(zs, mode);
            if (ini == NULL) {
                Error_Printf("Can't read file %s\n", buf);
                return NULL;
            }
            if (!is_zip) {
                Stream_Close(zs);
                zs = NULL;
            }
        }
    }
    c->states = StateDefs_Load(ini, c);
    if (c->states == NULL) {
        Char_ReportError(buf, ini, debug);
        Char_ReportVersion(c);
        Char_Free(c);
        return NULL;
    }
    Ini_Free(ini);
    Log_Printf("OK\n");
    for (i = 0; i <= 9; i++) {
        sprintf(key, "st%i", i);
        if (Ini_GetString(def, key, buf, sizeof(buf)) != 1 || buf[0] == '\0') {
            continue;
        }
        Log_Printf("  Loading cns %s...", buf);
        zs = Char_OpenFile(is_zip, zs, dir, buf, path, "rt");
        if (zs == NULL) {
            Error_Printf("\nCould not open file: %s\n", buf);
            return NULL;
        }
        ini = Ini_Parse(zs, mode);
        if (ini == NULL) {
            Error_Printf("\nError reading in file: %s\n", buf);
            return NULL;
        }
        if (!StateDefs_LoadMore(ini, c->states, c)) {
            Char_ReportError(buf, ini, debug);
            Char_ReportVersion(c);
            return NULL;
        }
        if (!is_zip) {
            Stream_Close(zs);
            zs = NULL;
        }
        Ini_Free(ini);
        Log_Printf("OK\n");
    }

    /* states in the command file */
    if (Ini_GetString(def, "cmd", buf, sizeof(buf)) != 1 || buf[0] == '\0') {
        return NULL;
    }
    Log_Printf("  Loading cmd state entry %s...", buf);
    zs = Char_OpenFile(is_zip, zs, dir, buf, path, "rt");
    if (zs == NULL) {
        Error_Printf("Could not open file: %s\n", buf);
        return NULL;
    }
    ini = Ini_Parse(zs, mode);
    if (ini == NULL) {
        Error_Printf("Error reading %s\n", buf);
        return NULL;
    }
    if (!is_zip) {
        Stream_Close(zs);
        zs = NULL;
    }
    if (!StateDefs_LoadMore(ini, c->states, c)) {
        Char_ReportError(buf, ini, debug);
        return NULL;
    }
    Ini_Free(ini);
    Log_Printf("OK\n");

    /* common states: next to the character, else wherever File_Find finds them */
    if (Ini_GetString(def, "stcommon", buf, sizeof(buf)) == 1 && buf[0] != '\0') {
        int opened;
        Log_Printf("  Loading common states %s...", buf);
        if (is_zip) {
            Stream_ZipUnload(zs);
            opened = Stream_ZipLoad(zs, buf);
        } else {
            Path_Join(buf, dir, path);
            zs = Stream_OpenFile(path, "rt");
            opened = zs != NULL;
        }
        if (opened == 1) {
            ini = Ini_Parse(zs, mode);
            if (ini == NULL) {
                Error_Printf("Error reading %s\n", buf);
                return NULL;
            }
            /* the message names path, which a zip character never set (stack garbage in the original) */
            if (!StateDefs_LoadMore(ini, c->states, c)) {
                Char_ReportError(path, ini, debug);
                return NULL;
            }
        } else {
            if (!File_Find(buf, path, NULL, NULL)) {
                return NULL;
            }
            ini = Ini_OpenMode(path, mode);
            if (ini == NULL) {
                Error_Printf("Error reading %s\n", path);
                return NULL;
            }
            if (!StateDefs_LoadMore(ini, c->states, c)) {
                Char_ReportError(path, ini, debug);
                return NULL;
            }
        }
        if (!is_zip) {
            Stream_Close(zs);       /* NULL after the File_Find fallback */
            zs = NULL;
        }
        Ini_Free(ini);
        Log_Printf("OK\n");
    }

    /* sprites */
    if (Ini_GetString(def, "sprite", buf, sizeof(buf)) != 1 || buf[0] == '\0') {
        return NULL;
    }
    Log_Printf("  Loading sff %s...", buf);
    if (is_zip) {
        Stream_ZipUnload(zs);
        Stream_ZipLoad(zs, buf);
        /* the original then tests a stack slot that holds the st<n> loop counter (10): never NULL */
        c->sff = Sff_LoadStream(zs);
    } else {
        Path_Join(buf, dir, path);
        c->sff = Sff_Load(path);
    }
    if (c->sff == NULL) {
        Error_Printf("Can't load %s\n", buf);
        return NULL;
    }
    Log_Printf("OK\n");

    /* animations; collision boxes are scaled from the character's localcoord */
    if (Ini_GetString(def, "anim", buf, sizeof(buf)) != 1 || buf[0] == '\0') {
        return NULL;
    }
    Log_Printf("  Loading anim %s...", buf);
    zs = Char_OpenFile(is_zip, zs, dir, buf, path, "rt");
    if (zs == NULL) {
        Error_Printf("Could not open file: %s\n", buf);
        return NULL;
    }
    ini = Ini_Parse(zs, mode);
    if (ini == NULL) {
        Error_Printf("Error reading in file: %s\n", buf);
        return NULL;
    }
    {
        double q = g_gameCoord[0] / c->localcoord[0];
        g_clsnScaleX = (float)(c->consts.xscale * q);
        g_clsnScaleY = (float)(q * c->consts.yscale);
    }
    c->anim = Anim_Parse(ini);
    if (c->anim == NULL) {
        /* g_clsnScaleX/Y stay set */
        Char_ReportError(buf, ini, debug);
        return NULL;
    }
    g_clsnScaleY = 1.0f;
    g_clsnScaleX = 1.0f;
    Ini_Free(ini);
    if (!is_zip) {
        Stream_Close(zs);
        zs = NULL;
    }
    Log_Printf("OK\n");

    /* sounds */
    if (g_sndActive == 1) {
        status = 0;
        if (Ini_GetString(def, "sound", buf, sizeof(buf)) == 1 && buf[0] != '\0') {
            Log_Printf("  Loading snd %s...", buf);
            if (is_zip) {
                Stream_ZipUnload(zs);
                Stream_ZipLoad(zs, buf);
                c->snd = Snd_Parse(zs, &status);
                if (c->snd == NULL) {
                    Error_Printf("Can't load %s\n", buf);
                    return NULL;
                }
                if (status == 2) {
                    Log_Printf("SND file loaded with errors: %s\n", buf);
                } else if (status != 0) {
                    Log_Printf("Invalid status code %d returned while loading SND file (engine's fault): %s\n",
                               status, buf);
                }
            } else {
                Stream *ss;
                Path_Join(buf, dir, path);
                ss = Stream_OpenFile(path, "rb");
                if (ss == NULL) {
                    Error_Printf("Could not open file: %s\n", buf);
                    return NULL;
                }
                c->snd = Snd_Parse(ss, &status);
                if (c->snd == NULL) {
                    Error_Printf("Can't load %s\n", buf);
                    return NULL;
                }
                if (status == 2) {
                    Log_Printf("SND file loaded with errors: %s\n", buf);
                } else if (status != 0) {
                    Log_Printf("Invalid status code %d returned while loading SND file (engine's fault): %s\n",
                               status, buf);
                }
                Stream_Close(ss);
                zs = NULL;
            }
            Log_Printf("OK\n");
        }
    }

    /* palettes 1..12 missing from the sprite file: pal1..pal12 .act files */
    Log_Printf("  Loading pals...");
    for (i = 1; i <= 12; i++) {
        if (SpriteSet_FindPalette(c->sff, 1, i) != NULL) {
            continue;
        }
        sprintf(key, "pal%i", i);
        if (Ini_GetString(def, key, buf, sizeof(buf)) != 1 || buf[0] == '\0') {
            continue;
        }
        zs = Char_OpenFile(is_zip, zs, dir, buf, path, "rb");
        if (zs == NULL) {
            Error_Printf("Can't open %s\n", buf);
            return NULL;
        }
        if (!Pal_ReadAct(zs, colors)) {
            Error_Printf("Can't load %s\n", buf);
            return NULL;
        }
        if (!is_zip) {
            Stream_Close(zs);
            zs = NULL;
        }
        if (!Pal_Init(&pal, 0x100, colors, PAL_COPY)) {
            return NULL;
        }
        if (!SpriteSet_AddPalette(c->sff, &pal, 1, i)) {
            return NULL;
        }
        Log_Printf("(1,%i)", i);
    }
    Log_Printf("OK\n");

    /*
     * SFF v1.01: sprites using the palette of 9000,0 or 0,0 are switched to
     * palette 1,1 (the first one), so that palette selection works.
     */
    if (c->sff->version == 0x01000100 && SpriteSet_FindPalette(c->sff, 1, 1) != NULL) {
        int pal_9000 = -1, pal_0, index;
        Sprite *spr = SpriteSet_FindSprite(c->sff, 9000, 0);
        if (spr != NULL) {
            pal_9000 = spr->pal_index;
        }
        spr = SpriteSet_FindSprite(c->sff, 0, 0);
        if (spr != NULL) {
            pal_0 = spr->pal_index;
            Log_Printf("  SFF v1.01 palette hack engaged.\n");
            index = c->sff->palettes != NULL ? KeyedArray_Find(c->sff->palettes, 1, 1) : -1;
            for (i = 0; i < (c->sff->sprites != NULL ? c->sff->sprites->count : 0); i++) {
                KeyedArray *list = c->sff->sprites;
                Sprite *sp = (Sprite *)(list->data + list->elem_size * i);
                if ((sp->pal_index == pal_9000 || sp->pal_index == pal_0) && sp->pal_index != -1) {
                    sp->pal_index = index;
                }
            }
        }
    }

    /* learned AI */
    if (Ini_GetString(def, "ai", buf, sizeof(buf)) == 1 && buf[0] != '\0') {
        Log_Printf("  Loading learned AI %s...", buf);
        Path_Join(buf, dir, path);
        if (!Ai_Load(path, &c->ai)) {
            FILE *fp = fopen(path, "rb");
            if (fp != NULL) {
                Log_Printf("Ignoring invalid AI file: %s\n", buf);
                fclose(fp);
            }
        }
        strncpy(c->ai_file, buf, sizeof(c->ai_file) - 1);
        Log_Printf("OK\n");
    }
    Ini_Free(def);
    Stream_Seek(s, tell, SEEK_SET);
    return c;
}

/* ------------------------------------------------------------------ constants */

/* 0x00436D60: const(name) = the int key of ini (def when missing), limited to min..max. */
int Const_ReadInt(Ini *ini, const char *key, int min, int max, CharData *c, int def, const char *name)
{
    const char *s = Ini_GetValue(ini, key);
    int v;

    if (s == NULL || sscanf(s, "%d", &v) != 1) {
        v = def;
    }
    if (v < min) {
        v = min;
    }
    if (v > max) {
        v = max;
    }
    return CharLua_SetConstInt(c, g_luaState, name, v);
}

/* 0x00436DD0: const(name) = the float key of ini (def when missing), limited to min..max. */
int Const_ReadFloat(Ini *ini, const char *key, CharData *c, const char *name, double def, double min, double max)
{
    const char *s = Ini_GetValue(ini, key);
    double v;

    if (s == NULL || sscanf(s, "%lf", &v) != 1) {
        v = def;
    }
    if (min > v) {
        v = min;
    }
    if (max < v) {
        v = max;
    }
    return CharLua_SetConstFloat(c, g_luaState, name, v);
}

/*
 * 0x00436E60: const(name) = a distance: a coordinate whose x is the key of
 * ini, in the character's units (def is in 320-wide units and is converted),
 * limited to min..max.
 */
int Const_ReadDist(CharData *c, const char *key, Ini *ini, const char *name, double def, double min, double max)
{
    double q = g_gameCoord[0] / c->localcoord[0];
    double f;
    const char *s;
    Coord w, v;

    /* {def, 0, 0, unit 1} converted to unit q */
    f = 1.0 / q;
    w.x = def * f;
    f = 1.0 / q;
    w.y = 0.0 * f;
    w.z = f * 0.0;
    w.unit = q;
    w.type = 1;
    w.pad_24 = 0;       /* not set in the original (stack garbage) */
    v = w;
    s = Ini_GetValue(ini, key);
    if (s == NULL || sscanf(s, "%lf", &v.x) != 1) {
        v.x = w.x;
    }
    if (min > v.x) {
        v.x = min;
    }
    if (max < v.x) {
        v.x = max;
    }
    return CharLua_SetConstCoord(c, g_luaState, name, v.x, v.y, v.z, v.unit, v.type, v.pad_24);
}

/* 0x00436F70: const(velocity.air.gethit.airrecover.mul) from air.gethit.airrecover.mul (factors, no units). */
int Const_ReadAirRecoverMul(CharData *c, Ini *ini, double dx, double dy, double dz, double dunit, int dtype,
                            int dpad, double minx, double miny, double minz, double minunit, int mintype,
                            int minpad, double maxx, double maxy, double maxz, double maxunit, int maxtype,
                            int maxpad)
{
    Coord v;

    v.x = dx;
    v.y = dy;
    v.z = dz;
    v.unit = dunit;
    v.type = dtype;
    v.pad_24 = dpad;
    Ini_Scanf(ini, "air.gethit.airrecover.mul", "%lf,%lf", &v.x, &v.y);
    if (v.x < minx) {
        v.x = minx;
    }
    if (maxx < v.x) {
        v.x = maxx;
    }
    if (v.y < miny) {
        v.y = miny;
    }
    if (maxy < v.y) {
        v.y = maxy;
    }
    return CharLua_SetConstCoord(c, g_luaState, "velocity.air.gethit.airrecover.mul", v.x, v.y, v.z, v.unit,
                                 v.type, v.pad_24);
}

/*
 * 0x00437040: const(name) = the "x,y" key of ini as a coordinate in game
 * units: def is converted to the character's units, the values read are
 * limited to min..max there and the result is converted back.
 */
int Const_ReadVector(CharData *c, Ini *ini, const char *key, const char *name, double dx, double dy, double dz,
                     double dunit, int dtype, int dpad, double minx, double miny, double minz, double minunit,
                     int mintype, int minpad, double maxx, double maxy, double maxz, double maxunit,
                     int maxtype, int maxpad)
{
    double q = g_gameCoord[0] / c->localcoord[0];
    double f;
    Coord v;

    f = dunit / q;
    v.x = f * dx;
    v.y = dy * f;
    v.z = f * dz;
    v.unit = q;
    v.type = dtype;
    v.pad_24 = dpad;
    Ini_Scanf(ini, key, "%lf,%lf", &v.x, &v.y);
    if (v.x < minx) {
        v.x = minx;
    }
    if (maxx < v.x) {
        v.x = maxx;
    }
    if (v.y < miny) {
        v.y = miny;
    }
    if (maxy < v.y) {
        v.y = maxy;
    }
    f = v.unit;
    v.x = v.x * f;
    v.y = v.y * f;
    v.z = f * v.z;
    v.unit = 1.0;
    return CharLua_SetConstCoord(c, g_luaState, name, v.x, v.y, v.z, v.unit, v.type, v.pad_24);
}

/* 0x004371A0: const(name) as a coordinate ({0, 0, 0, 1} when it is not one). */
Coord *Const_GetCoord(Coord *out, const char *name, CharData *c)
{
    Coord v;

    v.x = 0.0;
    v.y = 0.0;
    v.z = 0.0;
    v.unit = 1.0;
    v.type = 2;
    v.pad_24 = 0;       /* not set in the original (stack garbage) */
    *out = v;
    CharLua_GetConstCoord(c, g_luaState, out, name);
    return out;
}

/* 0x004371F0: const(alias) = const(name). */
int Const_Alias(CharData *c, const char *name, const char *alias)
{
    struct lua_State *L = g_luaState;

    lua_rawgeti(L, LUA_REGISTRYINDEX, c->lua_ref);
    lua_pushstring(L, "const");
    lua_gettable(L, -2);
    if (lua_type(L, -1) != LUA_TTABLE) {
        return luaL_error(L, "Table expected");
    }
    lua_pushstring(L, name);
    lua_gettable(L, -2);
    lua_pushstring(L, alias);
    lua_pushvalue(L, -2);
    lua_settable(L, -4);
    lua_settop(L, -2);
    lua_settop(L, -3);
    return 0;
}

/* Const_ReadVector with Coord arguments. */
static int Const_ReadVec(CharData *c, Ini *ini, const char *key, const char *name, const Coord *def,
                         const Coord *min, const Coord *max)
{
    return Const_ReadVector(c, ini, key, name, def->x, def->y, def->z, def->unit, def->type, def->pad_24, min->x,
                            min->y, min->z, min->unit, min->type, min->pad_24, max->x, max->y, max->z, max->unit,
                            max->type, max->pad_24);
}

/* Const_ReadVector without limits. */
static int Const_ReadVecFree(CharData *c, Ini *ini, const char *key, const char *name, const Coord *def)
{
    Coord min, max;

    Coord_Make(&max, CONST_MAX, CONST_MAX);
    Coord_Make(&min, -CONST_MAX, -CONST_MAX);
    return Const_ReadVec(c, ini, key, name, def, &min, &max);
}

/* 0/1 flag of ini (0 when missing; values outside 0..1 are limited). */
static int Const_ReadFlag(Ini *ini, const char *key)
{
    const char *s = Ini_GetValue(ini, key);
    int v;

    if (s == NULL || sscanf(s, "%d", &v) != 1) {
        return 0;
    }
    if (v < 0) {
        return 0;
    }
    if (v > 1) {
        return 1;
    }
    return v;
}

/*
 * 0x00437290: [Data], [Size], [Velocity], [Movement] and [Quotes] of the
 * constants file: first into the Lua constants of the character, then into
 * c->consts. Returns 0 when a group or a required key is missing.
 */
int Char_LoadConstants(CharData *c, Ini *ini)
{
    struct lua_State *L = g_luaState;
    CharConsts *k = &c->consts;
    char key[0x40] = { 0 };
    Coord def, saved;
    const char *s;
    double q, d, front, back, jump_x, jump_y;
    float w;
    int i, count;

    lua_rawgeti(L, LUA_REGISTRYINDEX, c->lua_ref);
    lua_pushstring(L, "const");
    lua_createtable(L, 0, 0);
    lua_settable(L, -3);
    lua_settop(L, -2);

    /* [Data] */
    if (Ini_SelectGroup(ini, "Data") == -1) {
        Error_Printf("Can't access [Data]\n");
        return 0;
    }
    Const_ReadInt(ini, "life", 1, 0x7FFFFFFF, c, 1000, "data.life");
    Const_ReadInt(ini, "power", 0, 0x7FFFFFFF, c, 3000, "data.power");
    Const_ReadInt(ini, "attack", 0, 0x7FFFFFFF, c, 100, "data.attack");
    w = (float)((int)g_game->game_coord[0] * 4.0 / 320.0);
    front = back = w / (g_gameCoord[0] / c->localcoord[0]);
    Ini_Scanf(ini, "attack.z.width", "%lf,%lf", &front, &back);
    CharLua_SetConstFloat(c, g_luaState, "size.attack.z.width.front", front);
    CharLua_SetConstFloat(c, g_luaState, "size.attack.z.width.back", back);
    Const_ReadInt(ini, "airjuggle", 0, 0x7FFFFFFF, c, 15, "data.airjuggle");
    Const_ReadInt(ini, "defence", 0, 0x7FFFFFFF, c, 100, "data.defence");
    d = 0.0;
    s = Ini_GetValue(ini, "fall.defence_up");
    if (s != NULL) {
        d = atof(s);
    }
    CharLua_SetConstFloat(c, g_luaState, "data.fall.defence_mul", 100.0 / (d + 100.0));
    Const_ReadInt(ini, "liedown.time", 0, 0x7FFFFFFF, c, 60, "data.liedown.time");
    Const_ReadInt(ini, "sparkno", (int)0x80000000, 0x7FFFFFFF, c, 0, "data.sparkno");
    Const_ReadInt(ini, "guard.sparkno", (int)0x80000000, 0x7FFFFFFF, c, 0, "data.guard.sparkno");
    CharLua_SetConstInt(c, g_luaState, "data.ko.echo", Const_ReadFlag(ini, "ko.echo"));
    Const_ReadInt(ini, "IntPersistIndex", 0, PLAYER_VAR_COUNT, c, PLAYER_VAR_COUNT, "data.intpersistindex");
    Const_ReadInt(ini, "FloatPersistIndex", 0, PLAYER_FVAR_COUNT, c, PLAYER_FVAR_COUNT, "data.floatpersistindex");

    s = Ini_GetValue(ini, "life");
    if (s == NULL) {
        goto data_error;
    }
    k->life = atoi(s);
    s = Ini_GetValue(ini, "attack");
    if (s == NULL) {
        goto data_error;
    }
    k->attack = atoi(s);
    s = Ini_GetValue(ini, "defence");
    if (s == NULL) {
        goto data_error;
    }
    k->defence = atoi(s);
    if (!Ini_GetFloat(ini, "fall.defence_up", &k->fall_defence_mul) ||
        !Ini_GetInt(ini, "liedown.time", &k->liedown_time) || !Ini_GetInt(ini, "airjuggle", &k->airjuggle)) {
        goto data_error;
    }
    Ini_GetDoubleDef(ini, "volumescale", &c->volumescale, 100.0);
    k->power = 3000;
    k->ko_echo = 1;
    k->int_persist_index = PLAYER_VAR_COUNT;
    k->float_persist_index = PLAYER_FVAR_COUNT;
    Ini_GetInt(ini, "power", &k->power);
    k->fall_defence_mul = (float)(100.0 / (k->fall_defence_mul + 100.0));
    Ini_GetInt(ini, "sparkno", &k->sparkno);
    Ini_GetInt(ini, "guard.sparkno", &k->guard_sparkno);
    Ini_GetInt(ini, "ko.echo", &k->ko_echo);
    Ini_GetInt(ini, "IntPersistIndex", &k->int_persist_index);
    Ini_GetInt(ini, "FloatPersistIndex", &k->float_persist_index);
    if (k->int_persist_index > PLAYER_VAR_COUNT) {
        k->int_persist_index = PLAYER_VAR_COUNT;
    }
    if (k->int_persist_index < 0) {
        k->int_persist_index = 0;
    }
    if (k->float_persist_index > PLAYER_FVAR_COUNT) {
        k->float_persist_index = PLAYER_FVAR_COUNT;
    }
    if (k->float_persist_index < 0) {
        k->float_persist_index = 0;
    }

    /* [Size] */
    if (Ini_SelectGroup(ini, "Size") == -1) {
        Error_Printf("Can't access [Size]\n");
        return 0;
    }
    Const_ReadDist(c, "attack.dist", ini, "size.attack.dist", Units_From320(160.0f), 0.0, CONST_MAX);
    Const_ReadDist(c, "proj.attack.dist", ini, "size.proj.attack.dist", 0.0, 0.0, CONST_MAX);
    Const_ReadFloat(ini, "xscale", c, "size.xscale", 1.0, 0.0, CONST_MAX);
    Const_ReadFloat(ini, "yscale", c, "size.yscale", 1.0, 0.0, CONST_MAX);
    Const_ReadDist(c, "ground.back", ini, "size.ground.back", Units_From320(15.0f), 0.0, CONST_MAX);
    Const_ReadDist(c, "ground.front", ini, "size.ground.front", Units_From320(15.0f), 0.0, CONST_MAX);
    Const_ReadDist(c, "air.back", ini, "size.air.back", Units_From320(15.0f), 0.0, CONST_MAX);
    Const_ReadDist(c, "air.front", ini, "size.air.front", Units_From320(15.0f), 0.0, CONST_MAX);
    Const_ReadDist(c, "height", ini, "size.height", Units_From320(60.0f), 0.0, CONST_MAX);
    CharLua_SetConstInt(c, g_luaState, "size.proj.doscale", Const_ReadFlag(ini, "proj.doscale"));
    Coord_MakeUnit(&def, Units_From320(-5.0f), Units_From320(-90.0f));
    Const_ReadVecFree(c, ini, "head.pos", "size.head.pos", &def);
    Coord_MakeUnit(&def, Units_From320(-5.0f), Units_From320(-60.0f));
    Const_ReadVecFree(c, ini, "mid.pos", "size.mid.pos", &def);
    Const_ReadDist(c, "shadowoffset", ini, "size.shadowoffset", 0.0, -CONST_MAX, CONST_MAX);
    Coord_MakeUnit(&def, 0.0, 0.0);
    Const_ReadVecFree(c, ini, "draw.offset", "size.draw.offset", &def);

    k->yscale = 1.0f;
    k->xscale = 1.0f;
    q = g_gameCoord[0] / c->localcoord[0];
    k->attack_dist = (int)(Units_From320(160.0f) / q);
    k->proj_attack_dist = (int)(Units_From320(90.0f) / q);
    k->attack_z_width[1] = k->attack_z_width[0] = (int)(Units_From320(4.0f) / q);
    k->height = (int)(Units_From320(60.0f) / q);
    k->mid_pos[0] = k->head_pos[0] = (int)(Units_From320(-5.0f) / q);
    k->mid_pos[1] = (int)(Units_From320(-60.0f) / q);
    k->head_pos[1] = (int)(Units_From320(-90.0f) / q);
    if (!Ini_GetInt(ini, "ground.back", &k->ground_back) || !Ini_GetInt(ini, "ground.front", &k->ground_front) ||
        !Ini_GetInt(ini, "air.back", &k->air_back) || !Ini_GetInt(ini, "air.front", &k->air_front)) {
        Error_Printf("Can't load Size constants\n");
        return 0;
    }
    Ini_GetInt(ini, "height", &k->height);
    Ini_GetFloat(ini, "xscale", &k->xscale);
    Ini_GetFloat(ini, "yscale", &k->yscale);
    Ini_Scanf(ini, "draw.offset", "%i,%i", &k->draw_offset[0], &k->draw_offset[1]);
    Ini_Scanf(ini, "attack.z.width", "%i,%i", &k->attack_z_width[0], &k->attack_z_width[1]);
    Ini_GetInt(ini, "attack.dist", &k->attack_dist);
    Ini_GetInt(ini, "proj.attack.dist", &k->proj_attack_dist);
    Ini_GetInt(ini, "proj.doscale", &k->proj_doscale);
    Ini_Scanf(ini, "head.pos", "%i,%i", &k->head_pos[0], &k->head_pos[1]);
    Ini_Scanf(ini, "mid.pos", "%i,%i", &k->mid_pos[0], &k->mid_pos[1]);
    Ini_GetInt(ini, "shadowoffset", &k->shadowoffset);

    /* [Velocity] */
    if (Ini_SelectGroup(ini, "Velocity") == -1) {
        Error_Printf("Can't access [Velocity]\n");
        return 0;
    }
    {
        static const char *const keys[8][2] = {
            { "walk.fwd", "velocity.walk.fwd" }, { "walk.back", "velocity.walk.back" },
            { "walk.up", "velocity.walk.up" },   { "walk.down", "velocity.walk.down" },
            { "run.fwd", "velocity.run.fwd" },   { "run.back", "velocity.run.back" },
            { "run.down", "velocity.run.down" }, { "run.up", "velocity.run.up" },
        };
        for (i = 0; i < 8; i++) {
            Coord_MakeUnit(&def, 0.0, 0.0);
            Const_ReadVecFree(c, ini, keys[i][0], keys[i][1], &def);
        }
    }
    /* the jump velocities default to x = 0 and the y of jump.neu; up and down read each other's key */
    jump_x = 0.0;
    jump_y = 0.0;
    Ini_Scanf(ini, "jump.neu", "%lf,%lf", &jump_x, &jump_y);
    {
        static const char *const keys[5][2] = {
            { "jump.neu", "velocity.jump.neu" },  { "jump.fwd", "velocity.jump.fwd" },
            { "jump.back", "velocity.jump.back" }, { "jump.down", "velocity.jump.up" },
            { "jump.up", "velocity.jump.down" },
        };
        for (i = 0; i < 5; i++) {
            Coord_MakeScaled(&def, c, 0.0, jump_y);
            Const_ReadVecFree(c, ini, keys[i][0], keys[i][1], &def);
        }
    }
    Const_Alias(c, "velocity.jump.neu", "velocity.jump");
    Const_GetCoord(&def, "velocity.jump.neu", c);
    Const_ReadVecFree(c, ini, "airjump.neu", "velocity.airjump.neu", &def);
    {
        static const char *const keys[4][5] = {
            { "velocity.jump.fwd", "runjump.fwd", "velocity.runjump.fwd", "airjump.fwd", "velocity.airjump.fwd" },
            { "velocity.jump.back", "runjump.back", "velocity.runjump.back", "airjump.back",
              "velocity.airjump.back" },
            { "velocity.jump.up", "runjump.up", "velocity.runjump.up", "airjump.up", "velocity.airjump.up" },
            { "velocity.jump.down", "runjump.down", "velocity.runjump.down", "airjump.down",
              "velocity.airjump.down" },
        };
        for (i = 0; i < 4; i++) {
            Const_GetCoord(&saved, keys[i][0], c);
            Const_ReadVecFree(c, ini, keys[i][1], keys[i][2], &saved);
            Const_ReadVecFree(c, ini, keys[i][3], keys[i][4], &saved);
        }
    }
    Const_Alias(c, "velocity.runjump.fwd", "velocity.runjump");
    Const_Alias(c, "velocity.airjump.neu", "velocity.airjump");
    Coord_MakeUnit(&def, Units_From320(-0.15f), Units_From320(-3.5f));
    Const_ReadVecFree(c, ini, "air.gethit.groundrecover", "velocity.air.gethit.groundrecover", &def);
    {
        Coord min, max;
        Coord_Make(&max, CONST_MAX, CONST_MAX);
        Coord_Make(&min, -CONST_MAX, -CONST_MAX);
        Coord_Make(&def, 0.5, 0.2);
        Const_ReadAirRecoverMul(c, ini, def.x, def.y, def.z, def.unit, def.type, def.pad_24, min.x, min.y, min.z,
                                min.unit, min.type, min.pad_24, max.x, max.y, max.z, max.unit, max.type,
                                max.pad_24);
    }
    Coord_MakeUnit(&def, Units_From320(0.0f), Units_From320(-4.5f));
    Const_ReadVecFree(c, ini, "air.gethit.airrecover.add", "velocity.air.gethit.airrecover.add", &def);
    Const_ReadDist(c, "air.gethit.airrecover.back", ini, "velocity.air.gethit.airrecover.back",
                   Units_From320(-1.0f), -CONST_MAX, CONST_MAX);
    Const_ReadDist(c, "air.gethit.airrecover.fwd", ini, "velocity.air.gethit.airrecover.fwd", Units_From320(0.0f),
                   -CONST_MAX, CONST_MAX);
    Const_ReadDist(c, "air.gethit.airrecover.up", ini, "velocity.air.gethit.airrecover.up", Units_From320(-2.0f),
                   -CONST_MAX, CONST_MAX);
    Const_ReadDist(c, "air.gethit.airrecover.down", ini, "velocity.air.gethit.airrecover.down",
                   Units_From320(1.5f), -CONST_MAX, CONST_MAX);

    Ini_GetFloat(ini, "walk.fwd", &k->walk_fwd);
    Ini_GetFloat(ini, "walk.back", &k->walk_back);
    Ini_GetFloat(ini, "walk.up", &k->walk_up);
    Ini_GetFloat(ini, "walk.down", &k->walk_down);
    Ini_Scanf(ini, "run.fwd", "%f,%f", &k->run_fwd[0], &k->run_fwd[1]);
    Ini_Scanf(ini, "run.back", "%f,%f", &k->run_back[0], &k->run_back[1]);
    Ini_Scanf(ini, "run.up", "%f,%f", &k->run_up[0], &k->run_up[1]);
    Ini_Scanf(ini, "run.down", "%f,%f", &k->run_down[0], &k->run_down[1]);
    Ini_Scanf(ini, "jump.neu", "%f,%f", &k->jump_neu_x, &k->jump_y);
    Ini_Scanf(ini, "jump.fwd", "%f", &k->jump_fwd_x);
    Ini_Scanf(ini, "jump.back", "%f", &k->jump_back_x);
    Ini_Scanf(ini, "jump.down", "%f", &k->jump_down_x);
    Ini_Scanf(ini, "jump.up", "%f", &k->jump_up_x);
    /* run and air jumps default to the jump velocities */
    k->airjump_neu_x = k->jump_neu_x;
    k->airjump_y = k->jump_y;
    k->airjump_fwd_x = k->runjump_fwd_x = k->jump_fwd_x;
    k->airjump_back_x = k->runjump_back_x = k->jump_back_x;
    k->airjump_up_x = k->runjump_up_x = k->jump_up_x;
    k->airjump_down_x = k->runjump_down_x = k->jump_down_x;
    Ini_Scanf(ini, "runjump.fwd", "%f,%f", &k->runjump_fwd_x, &k->runjump_y);
    Ini_Scanf(ini, "runjump.back", "%f", &k->runjump_back_x);
    Ini_Scanf(ini, "runjump.down", "%f", &k->runjump_down_x);
    Ini_Scanf(ini, "runjump.up", "%f", &k->runjump_up_x);
    Ini_Scanf(ini, "airjump.neu", "%f,%f", &k->airjump_neu_x, &k->airjump_y);
    Ini_Scanf(ini, "airjump.fwd", "%f", &k->airjump_fwd_x);
    Ini_Scanf(ini, "airjump.back", "%f", &k->airjump_back_x);
    Ini_Scanf(ini, "airjump.down", "%f", &k->airjump_down_x);
    Ini_Scanf(ini, "airjump.up", "%f", &k->airjump_up_x);
    q = g_gameCoord[0] / c->localcoord[0];
    k->air_gethit_groundrecover[0] = (float)(Units_From320(-0.15f) / q);
    k->air_gethit_groundrecover[1] = (float)(Units_From320(-3.5f) / q);
    k->air_gethit_airrecover_mul[0] = 0.5f;
    k->air_gethit_airrecover_mul[1] = 0.2f;
    k->air_gethit_airrecover_add[0] = (float)(Units_From320(0.0f) / q);
    k->air_gethit_airrecover_add[1] = (float)(Units_From320(-4.5f) / q);
    k->air_gethit_airrecover_back = (float)(Units_From320(-1.0f) / q);
    k->air_gethit_airrecover_fwd = (float)(Units_From320(0.0f) / q);
    k->air_gethit_airrecover_up = (float)(Units_From320(-2.0f) / q);
    k->air_gethit_airrecover_down = (float)(Units_From320(1.5f) / q);
    Ini_Scanf(ini, "air.gethit.groundrecover", "%f,%f", &k->air_gethit_groundrecover[0],
              &k->air_gethit_groundrecover[1]);
    Ini_Scanf(ini, "air.gethit.airrecover.mul", "%f,%f", &k->air_gethit_airrecover_mul[0],
              &k->air_gethit_airrecover_mul[1]);
    Ini_Scanf(ini, "air.gethit.airrecover.add", "%f,%f", &k->air_gethit_airrecover_add[0],
              &k->air_gethit_airrecover_add[1]);
    Ini_Scanf(ini, "air.gethit.airrecover.back", "%f", &k->air_gethit_airrecover_back);
    Ini_Scanf(ini, "air.gethit.airrecover.fwd", "%f", &k->air_gethit_airrecover_fwd);
    Ini_Scanf(ini, "air.gethit.airrecover.up", "%f", &k->air_gethit_airrecover_up);
    Ini_Scanf(ini, "air.gethit.airrecover.down", "%f", &k->air_gethit_airrecover_down);

    /* [Movement] */
    if (Ini_SelectGroup(ini, "Movement") == -1) {
        Error_Printf("Can't access [Movement]\n");
        return 0;
    }
    Const_ReadInt(ini, "airjump.num", 0, 0x7FFFFFFF, c, 0, "movement.airjump.num");
    Const_ReadDist(c, "airjump.height", ini, "movement.airjump.height", 0.0, 0.0, CONST_MAX);
    Const_ReadDist(c, "yaccel", ini, "movement.yaccel", 0.0, -CONST_MAX, CONST_MAX);
    Const_ReadFloat(ini, "stand.friction", c, "movement.stand.friction", 0.0, 0.0, 1.0);
    Const_ReadFloat(ini, "crouch.friction", c, "movement.crouch.friction", 0.0, 0.0, 1.0);
    Const_ReadDist(c, "stand.friction.threshold", ini, "movement.stand.friction.threshold", Units_From320(2.0f),
                   -CONST_MAX, CONST_MAX);
    Const_ReadDist(c, "crouch.friction.threshold", ini, "movement.crouch.friction.threshold",
                   Units_From320(0.0f), -CONST_MAX, CONST_MAX);
    Const_ReadDist(c, "air.gethit.groundlevel", ini, "movement.air.gethit.groundlevel", Units_From320(10.0f),
                   -CONST_MAX, CONST_MAX);
    Const_ReadDist(c, "air.gethit.groundrecover.ground.threshold", ini,
                   "movement.air.gethit.groundrecover.ground.threshold", Units_From320(-20.0f), -CONST_MAX,
                   CONST_MAX);
    Const_ReadDist(c, "air.gethit.groundrecover.groundlevel", ini, "movement.air.gethit.groundrecover.groundlevel",
                   Units_From320(10.0f), -CONST_MAX, CONST_MAX);
    Const_ReadDist(c, "air.gethit.airrecover.threshold", ini, "movement.air.gethit.airrecover.threshold",
                   Units_From320(-1.0f), -CONST_MAX, CONST_MAX);
    Const_ReadDist(c, "air.gethit.airrecover.yaccel", ini, "movement.air.gethit.airrecover.yaccel",
                   Units_From320(0.35f), -CONST_MAX, CONST_MAX);
    Const_ReadDist(c, "air.gethit.trip.groundlevel", ini, "movement.air.gethit.trip.groundlevel",
                   Units_From320(15.0f), -CONST_MAX, CONST_MAX);
    Coord_MakeUnit(&def, Units_From320(0.0f), Units_From320(20.0f));
    Const_ReadVecFree(c, ini, "down.bounce.offset", "movement.down.bounce.offset", &def);
    Const_ReadDist(c, "down.bounce.yaccel", ini, "movement.down.bounce.yaccel", Units_From320(0.4f), -CONST_MAX,
                   CONST_MAX);
    Const_ReadDist(c, "down.bounce.groundlevel", ini, "movement.down.bounce.groundlevel", Units_From320(12.0f),
                   -CONST_MAX, CONST_MAX);
    Const_ReadDist(c, "down.friction.threshold", ini, "movement.down.friction.threshold", Units_From320(0.05f),
                   -CONST_MAX, CONST_MAX);

    if (!Ini_GetFloat(ini, "yaccel", &k->yaccel) || !Ini_GetFloat(ini, "stand.friction", &k->stand_friction) ||
        !Ini_GetFloat(ini, "crouch.friction", &k->crouch_friction)) {
        Error_Printf("Can't load movement constants\n");
        return 0;
    }
    Ini_GetInt(ini, "airjump.num", &k->airjump_num);
    Ini_GetInt(ini, "airjump.height", &k->airjump_height);
    q = g_gameCoord[0] / c->localcoord[0];
    k->stand_friction_threshold = (float)(Units_From320(2.0f) / q);
    k->crouch_friction_threshold = (float)(Units_From320(0.0f) / q);
    k->air_gethit_groundlevel = (float)(Units_From320(10.0f) / q);
    k->air_gethit_groundrecover_ground_threshold = (float)(Units_From320(-20.0f) / q);
    k->air_gethit_groundrecover_groundlevel = (float)(Units_From320(10.0f) / q);
    k->air_gethit_airrecover_threshold = (float)(Units_From320(-1.0f) / q);
    k->air_gethit_airrecover_yaccel = (float)(Units_From320(0.35f) / q);
    k->air_gethit_trip_groundlevel = (float)(Units_From320(15.0f) / q);
    k->down_bounce_offset[0] = (float)(Units_From320(0.0f) / q);
    k->down_bounce_offset[1] = (float)(Units_From320(20.0f) / q);
    k->down_bounce_yaccel = (float)(Units_From320(0.4f) / q);
    k->down_bounce_groundlevel = (float)(Units_From320(12.0f) / q);
    k->down_friction_threshold = (float)(Units_From320(0.05f) / q);
    Ini_Scanf(ini, "stand.friction.threshold", "%f", &k->stand_friction_threshold);
    Ini_Scanf(ini, "crouch.friction.threshold", "%f", &k->crouch_friction_threshold);
    Ini_Scanf(ini, "air.gethit.groundlevel", "%f", &k->air_gethit_groundlevel);
    Ini_Scanf(ini, "air.gethit.groundrecover.ground.threshold", "%f",
              &k->air_gethit_groundrecover_ground_threshold);
    Ini_Scanf(ini, "air.gethit.groundrecover.groundlevel", "%f", &k->air_gethit_groundrecover_groundlevel);
    Ini_Scanf(ini, "air.gethit.airrecover.threshold", "%f", &k->air_gethit_airrecover_threshold);
    Ini_Scanf(ini, "air.gethit.airrecover.yaccel", "%f", &k->air_gethit_airrecover_yaccel);
    Ini_Scanf(ini, "air.gethit.trip.groundlevel", "%f", &k->air_gethit_trip_groundlevel);
    Ini_Scanf(ini, "down.bounce.offset", "%f,%f", &k->down_bounce_offset[0], &k->down_bounce_offset[1]);
    Ini_Scanf(ini, "down.bounce.yaccel", "%f", &k->down_bounce_yaccel);
    Ini_Scanf(ini, "down.bounce.groundlevel", "%f", &k->down_bounce_groundlevel);
    Ini_Scanf(ini, "down.friction.threshold", "%f", &k->down_friction_threshold);

    /* [Quotes] (optional) */
    if (Ini_SelectGroup(ini, "Quotes") != -1) {
        count = 0;
        for (i = 0; i < CHAR_QUOTE_COUNT; i++) {
            sprintf(key, "victory%i", i);
            c->quotes[i] = Ini_GetValueDup(ini, key);
            if (c->quotes[i] != NULL) {
                count++;
            }
        }
        c->quote_count = count;
    }
    return 1;

data_error:
    Error_Printf("Can't load Data constants\n");
    return 0;
}

/* ------------------------------------------------------------------ commands (.cmd) */

/*
 * 0x00439920: [Defaults], [Remap] and the [Command] groups of a .cmd file.
 * Commands with the same name share a label; "recovery" has label 0.
 */
int Char_LoadCmd(Ini *ini, CmdSet *cmds)
{
    static const char *const remap_keys[7] = { "a", "b", "c", "x", "y", "z", "s" };
    CmdRemap remap = { { CMD_KEY_A, CMD_KEY_B, CMD_KEY_C, CMD_KEY_X, CMD_KEY_Y, CMD_KEY_Z, CMD_KEY_S } };
    int time = 15, buffer_time = 1, label = 1;
    const char *header, *value;
    int i, j, len;

    if (Ini_SelectGroup(ini, "defaults") != -1) {
        value = Ini_GetValue(ini, "command.time");
        if (value != NULL) {
            time = atoi(value);
            if (time < 1) {
                time = 1;
            }
        }
        value = Ini_GetValue(ini, "command.buffer.time");
        if (value != NULL) {
            buffer_time = atoi(value);
            if (buffer_time < 1) {
                buffer_time = 1;
            } else if (buffer_time > 30) {
                buffer_time = 30;
            }
        }
    }
    if (Ini_SelectGroup(ini, "remap") != -1) {
        for (i = 0; i < 7; i++) {
            value = Ini_GetValue(ini, remap_keys[i]);
            if (value == NULL) {
                continue;
            }
            switch (value[0]) {
            case 'a':
                remap.keys[i] = CMD_KEY_A;
                break;
            case 'b':
                remap.keys[i] = CMD_KEY_B;
                break;
            case 'c':
                remap.keys[i] = CMD_KEY_C;
                break;
            case 'x':
                remap.keys[i] = CMD_KEY_X;
                break;
            case 'y':
                remap.keys[i] = CMD_KEY_Y;
                break;
            case 'z':
                remap.keys[i] = CMD_KEY_Z;
                break;
            case 's':
                remap.keys[i] = CMD_KEY_S;
                break;
            default:
                remap.keys[i] = 1;
                break;
            }
        }
    }
    cmds->count = Ini_CountGroups(ini, "command");
    if (cmds->count == 0) {
        Error_Printf("Need at least 1 command\n");
        return 0;
    }
    cmds->entries = calloc(1, cmds->count * sizeof(CmdEntry));
    if (cmds->entries == NULL) {
        g_outOfMemory = 1;
        return 0;
    }
    cmds->names = malloc(cmds->count * sizeof(char *));
    if (cmds->names == NULL) {
        return 0;
    }
    memset(cmds->names, 0, cmds->count * sizeof(char *));
    ini->group = -1;
    header = Ini_NextGroup(ini);
    for (i = 0; i < cmds->count; i++) {
        CmdEntry *e;
        while (!Str_StartsWithI(header + 1, "command")) {
            header = Ini_NextGroup(ini);
        }
        value = Ini_GetValue(ini, "name");
        if (value == NULL) {
            Error_Printf("Command %i missing name parameter\n", i + 1);
            return 0;
        }
        if (value[0] != '"' || value[strlen(value) - 1] != '"') {
            Error_Printf("Command %i name not in quotes\n", i + 1);
            return 0;
        }
        cmds->names[i] = malloc(strlen(value) - 1);
        if (cmds->names[i] == NULL) {
            return 0;
        }
        strncpy(cmds->names[i], value + 1, strlen(value) - 2);
        cmds->names[i][strlen(value) - 2] = '\0';
        e = &cmds->entries[i];
        e->label = label++;
        e->type = CMD_TYPE_NORMAL;
        if (strcmp(value, "\"recovery\"") == 0) {
            e->label = 0;
            e->type = CMD_TYPE_RECOVERY;    /* label was incremented anyway */
        } else {
            for (j = i - 1; j >= 0; j--) {
                len = (int)strlen(value) - 2;
                if (strncmp(value + 1, cmds->names[j], len) == 0 && (int)strlen(cmds->names[j]) == len) {
                    label--;
                    e->label = cmds->entries[j].label;
                    break;
                }
            }
        }
        if (label >= CMD_MAX_LABELS) {
            Error_Printf("Player has too many commands. Only %i unique commands labels allowed.\n", CMD_MAX_LABELS);
            return 0;
        }
        value = Ini_GetValue(ini, "command");
        if (value == NULL) {
            Error_Printf("Command %i \"%s\" missing command parameter\n", i + 1, cmds->names[i]);
            return 0;
        }
        if (!Cmd_Parse(value, &cmds->entries[i], &remap)) {
            Error_Printf("Error in command: %s\n", cmds->names[i]);
            return 0;
        }
        cmds->entries[i].time = time;
        Ini_GetInt(ini, "time", &cmds->entries[i].time);
        cmds->entries[i].buffer_time = buffer_time;
        Ini_GetInt(ini, "buffer.time", &cmds->entries[i].buffer_time);
        if (cmds->entries[i].buffer_time > 30) {
            cmds->entries[i].buffer_time = 30;
        }
        header = Ini_NextGroup(ini);
    }
    cmds->label_count = label;
    return 1;
}

/*
 * 0x00439F20: parses "command = ..." into entry. A direction repeated by the
 * next step gets an implicit release step in between. The original keeps 256
 * steps on the stack; a 257th overwrites its return address (here the extra
 * entry is only scratch and the command is rejected).
 */
int Cmd_Parse(const char *s, CmdEntry *entry, const CmdRemap *remap)
{
    CmdStep steps[CMD_MAX_STEPS + 1];
    unsigned int keys, mods, key;
    int n = 0, pos = 0, strict, charge, len, i;

    memset(steps, 0, CMD_MAX_STEPS * sizeof(CmdStep));
    if (s[0] != '\0') {
        do {
            unsigned int flags;
            keys = 0;
            mods = 0;
            strict = 0;
            charge = 0;
            while (s[pos] >= 1 && s[pos] <= 127 && isspace(s[pos]) && s[pos] != '\0') {
                pos++;
            }
            if (s[pos] == '\0') {
                return 0;
            }
            if (s[pos] == '>') {
                strict = 1;
                pos++;
            }
            if (s[pos] == '/') {
                mods = CMD_ANY_DIR;
                pos++;
            }
            if (s[pos] == '~') {
                char ch = s[pos + 1];
                mods |= CMD_RELEASE;
                pos++;
                while (ch >= '0' && ch <= '9') {
                    pos++;
                    charge = charge * 10 + (ch - '0');
                    ch = s[pos];
                }
                mods |= (unsigned int)(charge & ~3) << CMD_CHARGE_SHIFT;
            }
            if (s[pos] == '$') {
                mods |= CMD_4WAY;
                pos++;
            }
            if (s[pos] == '+') {
                pos++;
            }
            for (;;) {
                len = Cmd_ParseKey(&s[pos], &key, remap);
                if (len == 0) {
                    return 0;
                }
                keys |= key;
                pos += len;
                if (s[pos] != '+') {
                    break;
                }
                pos++;
            }
            keys |= mods;
            flags = (unsigned int)strict;
            if (n > 0 && keys == steps[n - 1].keys && (keys & CMD_KEY_DIRS)) {
                steps[n] = steps[n - 1];
                steps[n].keys |= CMD_RELEASE;
                steps[n].flags |= CMD_STEP_STRICT;
                flags |= CMD_STEP_STRICT;
                n++;
            }
            steps[n].keys = keys;
            steps[n].flags = flags;
            n++;
            if (n > CMD_MAX_STEPS) {
                return 0;
            }
            while (s[pos] != ',' && s[pos] != '\0') {
                pos++;
            }
            if (s[pos] == '\0') {
                break;
            }
            pos++;
        } while (s[pos] != '\0');
    }
    entry->steps = calloc(1, n * sizeof(CmdStep));
    if (entry->steps == NULL) {
        g_outOfMemory = 1;
        return 0;
    }
    for (i = 0; i < n; i++) {
        entry->steps[i] = steps[i];
    }
    entry->step_count = n;
    return 1;
}

/* 0x0043A150: one key of a command step; returns its length (1 or 2), 0 when unknown. */
int Cmd_ParseKey(const char *s, unsigned int *out, const CmdRemap *remap)
{
    switch (s[0]) {
    case 'B':
        *out = CMD_KEY_B_DIR;
        return 1;
    case 'D':
        if (s[1] == 'B') {
            *out = CMD_KEY_DB;
            return 2;
        }
        if (s[1] == 'F') {
            *out = CMD_KEY_DF;
            return 2;
        }
        *out = CMD_KEY_D;
        return 1;
    case 'F':
        *out = CMD_KEY_F;
        return 1;
    case 'N':
        *out = CMD_KEY_N;
        return 1;
    case 'U':
        if (s[1] == 'B') {
            *out = CMD_KEY_UB;
            return 2;
        }
        if (s[1] == 'F') {
            *out = CMD_KEY_UF;
            return 2;
        }
        *out = CMD_KEY_U;
        return 1;
    case 'a':
        *out = remap->keys[0];
        return 1;
    case 'b':
        *out = remap->keys[1];
        return 1;
    case 'c':
        *out = remap->keys[2];
        return 1;
    case 'x':
        *out = remap->keys[3];
        return 1;
    case 'y':
        *out = remap->keys[4];
        return 1;
    case 'z':
        *out = remap->keys[5];
        return 1;
    case 's':
        *out = remap->keys[6];
        return 1;
    }
    return 0;
}

/* 0x0043A290: frees the command names. */
void CmdSet_FreeNames(CmdSet *cmds)
{
    char **names = cmds->names;
    int count = cmds->count;
    int i;

    if (names != NULL) {
        for (i = 0; i < count; i++) {
            if (names[i] != NULL) {
                free(names[i]);
            }
        }
        free(names);
    }
    cmds->names = NULL;
}

/* 0x0043A2E0: frees the commands (entries is left dangling). */
void CmdSet_Free(CmdSet *cmds)
{
    int i;

    if (cmds == NULL) {
        return;
    }
    if (cmds->entries != NULL) {
        for (i = 0; i < cmds->count; i++) {
            if (cmds->entries[i].steps != NULL) {
                free(cmds->entries[i].steps);
            }
        }
        free(cmds->entries);
    }
    CmdSet_FreeNames(cmds);
}

/* 0x0043A330: frees a loaded character. */
void Char_Free(CharData *c)
{
    int i;

    if (c == NULL) {
        return;
    }
    Log_Printf("Freeing player RC %s...", c->name);
    Ai_Free(&c->ai);
    Log_Printf("CMD...");
    CmdSet_Free(&c->cmds);
    Log_Printf("CNS...");
    StateDefs_Free(c->states);
    Log_Printf("SFF...");
    if (!SpriteSet_Free(c->sff)) {
        Log_Printf("failed!");
    }
    Log_Printf("AIR...");
    Anim_Free(c->anim);
    Log_Printf("SND...");
    Snd_Free(c->snd);
    Log_Printf("Misc...");
    for (i = 0; i < CHAR_QUOTE_COUNT; i++) {
        if (c->quotes[i] != NULL) {
            free(c->quotes[i]);
        }
    }
    luaL_unref(g_luaState, LUA_REGISTRYINDEX, c->lua_ref);
    free(c);
    Log_Printf("OK\n");
}

/* ------------------------------------------------------------------ players */

/* 0x0043A450: a player of character c (NULL: a blank player). Returns NULL on errors (leaking). */
Player *Player_Create(CharData *c)
{
    Player *p = calloc(1, sizeof(Player));
    AnimState *anim;

    if (p == NULL) {
        g_outOfMemory = 1;
        if (g_game != NULL && g_game->error_text != NULL) {
            TextRing_Printf(g_game->error_text, "Out of memory");
            return NULL;
        }
        fprintf(stderr, "Out of memory");
        return NULL;
    }
    p->data = c;
    p->helper = c == NULL;
    if (c != NULL) {
        p->cmds = &c->cmds;
    }
    p->id = g_game->player_id_next;
    g_game->player_id_next++;
    p->team_index = -1;
    if (c != NULL) {
        p->consts = c->consts;
        p->sprites = c->sff;
    }
    p->palfx = &p->palfx_own;
    p->remap = &p->remap_own;
    PalFx_Init(&p->palfx_own, NULL);        /* inlined in the original */
    if (c != NULL) {
        SpriteSet *sprites = p->sprites;
        Anim *a = c->anim;
        anim = calloc(1, sizeof(AnimState));
        if (anim == NULL) {
            g_outOfMemory = 1;
        } else {
            anim->anim = a;
            anim->sprites = sprites;
        }
    } else {
        anim = calloc(1, sizeof(AnimState));
        if (anim == NULL) {
            g_outOfMemory = 1;
        } else {
            anim->anim = NULL;
            anim->sprites = NULL;
        }
    }
    p->anim = anim;
    if (anim == NULL) {
        Error_OutOfMemory();
        return NULL;
    }
    p->anim->queue = g_game->sprite_layer1;
    if (c != NULL) {
        p->projectiles = Projectiles_Create(g_game->player_projectile_max, c->anim, p->sprites);
        if (p->projectiles == NULL) {
            Error_Printf("Can't create projectile array.\n");
            return NULL;
        }
        memset(&p->states, 0, offsetof(Player, nohit_time) - offsetof(Player, states));   /* Moemon Fight: the same fields on 64-bit */
        p->states = c->states;
        if (!Ai_InitPlayer(p)) {
            Error_Printf("Can't create state controller.\n");
            return NULL;
        }
        p->ai_level = Player_GetAiLevel();
        if (p->ai_learn) {
            Ai_Reset(&p->ai_data);
            Ai_Merge(&p->ai_data, &c->ai);
        }
    } else {
        p->projectiles = Projectiles_Create(g_game->player_projectile_max, NULL, NULL);
        if (p->projectiles == NULL) {
            Error_Printf("Can't create projectile array.\n");
            return NULL;
        }
        memset(&p->states, 0, offsetof(Player, nohit_time) - offsetof(Player, states));   /* Moemon Fight: the same fields on 64-bit */
        p->states = NULL;
    }
    p->targets = KeyedArray_Create(8, 0x1C);
    if (p->targets == NULL) {
        Error_Printf("Can't create target array.\n");
        return NULL;
    }
    p->hit_pending = Mem_Calloc(g_game->player_slots + 1, sizeof(int));
    if (p->hit_pending == NULL) {
        return NULL;
    }
    p->hit_ids = Mem_Calloc(g_game->player_slots + 1, sizeof(int));
    if (p->hit_ids == NULL) {
        return NULL;
    }
    if (p->data != NULL) {
        p->name[0] = '\0';
        strncat(p->name, p->data->name, sizeof(p->name) - 1);
    }
    p->warnings = TextRing_Create(0x9D, 3);
    if (p->warnings == NULL) {
        return NULL;
    }
    memset(&p->remap_own, 0, sizeof(p->remap_own));
    Player_Reset(p);
    return p;
}

/* 0x0043A7C0: AI level of a new player: the difficulty, moved along the AI ramp of the mode, 1..8. */
int Player_GetAiLevel(void)
{
    GameVars *g = g_game;
    int match = g->arcade_match - 1;
    int level = g->difficulty;
    const AiRamp *ramp = NULL;

    if (g->arcade_side != 0) {
        const TeamSel *team = &g->teams[2 - g->arcade_side];
        ramp = &g->ai_ramp[0];
        if (team->members[0].mode != 0 || team->member_count > 1) {
            ramp = &g->ai_ramp[1];
        }
    }
    if (g->survival_side != 0) {
        match = g->round_no;
        ramp = &g->ai_ramp[2];
        level = 4;
    }
    if (ramp == NULL) {
        return level;
    }
    if (match <= ramp->start_match) {
        level += ramp->start_offset;
    } else if (match >= ramp->end_match) {
        level += ramp->end_offset;
    } else if (ramp->start_match < ramp->end_match) {
        level += (ramp->end_offset - ramp->start_offset) * (match - ramp->start_match) /
                     (ramp->end_match - ramp->start_match) +
                 ramp->start_offset;
    }
    if (level < 1) {
        return 1;
    }
    if (level > 8) {
        return 8;
    }
    return level;
}

/* 0x0043A880: frees a player (its learned AI data goes back to the character). */
void Player_Free(Player *p)
{
    if (p == NULL) {
        return;
    }
    if (p->ai_learn) {
        Ai_Merge(&p->data->ai, &p->ai_data);
    }
    TextRing_Free(p->warnings);             /* inlined in the original */
    Ai_Free(&p->ai_data);
    memset(&p->ai, 0, PLAYER_AI_BLOCK_SIZE);
    KeyedArray_Free(p->targets);            /* inlined in the original */
    Projectiles_Free(p->projectiles);
    if (p->afterimage != NULL) {
        p->afterimage->running = 0;
        p->afterimage->anim = NULL;
        p->afterimage = NULL;
    }
    if (p->anim != NULL) {
        if (p->anim->owns_sprites != 0) {
            SpriteSet_Free(p->anim->sprites);
        }
        free(p->anim);
    }
    if (p->hit_pending != NULL) {
        free(p->hit_pending);
    }
    if (p->hit_ids != NULL) {
        free(p->hit_ids);
    }
    free(p);
}

/* Sprites every character must have (group, number), ended by -1. */
static const int char_required_sprites[][2] = {
    { 5000, 0 },  { 5001, 0 },  { 5002, 0 },  { 5000, 10 }, { 5001, 10 }, { 5002, 10 }, { 5000, 20 },
    { 5001, 20 }, { 5002, 20 }, { 5010, 0 },  { 5011, 0 },  { 5012, 0 },  { 5010, 10 }, { 5011, 10 },
    { 5012, 10 }, { 5010, 20 }, { 5011, 20 }, { 5012, 20 }, { 5020, 0 },  { 5020, 10 }, { 5020, 20 },
    { 5030, 0 },  { 5031, 0 },  { 5032, 0 },  { 5030, 10 }, { 5031, 10 }, { 5032, 10 }, { 5030, 20 },
    { 5031, 20 }, { 5032, 20 }, { 5030, 30 }, { 5031, 30 }, { 5032, 30 }, { 5030, 40 }, { 5031, 40 },
    { 5032, 40 }, { 5030, 50 }, { 5031, 50 }, { 5032, 50 }, { 5040, 0 },  { 5041, 0 },  { 5042, 0 },
    { 5040, 10 }, { 5041, 10 }, { 5042, 10 }, { 5040, 20 }, { 5041, 20 }, { 5042, 20 }, { 5060, 0 },
    { 5061, 0 },  { 5062, 0 },  { 5060, 10 }, { 5061, 10 }, { 5062, 10 }, { 5070, 0 },  { 5071, 0 },
    { 5072, 0 },  { 5070, 10 }, { 5071, 10 }, { 5072, 10 }, { 5070, 20 }, { 5071, 20 }, { 5072, 20 },
    { -1, 0 },
};

/* Animation actions every character must have, in the original's order (5160 and 5170 before 5110), ended by -1. */
static const int char_required_actions[] = {
    0,    10,   11,   12,   20,   21,   40,   41,   42,   43,   47,   100,  105,  120,  121,  122,
    130,  131,  132,  140,  141,  142,  150,  151,  152,  5000, 5001, 5002, 5005, 5006, 5007, 5010,
    5011, 5012, 5015, 5016, 5017, 5020, 5021, 5022, 5025, 5026, 5027, 5030, 5040, 5050, 5070, 5080,
    5090, 5100, 5160, 5170, 5110, 5120, 5200, 5210, 5300, -1,
};

/*
 * 0x0043A9A0: lists the missing required sprites and animation actions of p
 * in the debug clipboard (8 of each at most). Returns 1 when none is missing.
 */
int Player_CheckRequired(Player *p)
{
    TextRing *ring = g_game->clipboard;
    SpriteSet *sff = p->data->sff;
    int missing_sprites = 0, missing_actions = 0;
    int i;

    for (i = 0; char_required_sprites[i][0] != -1; i++) {
        int group = char_required_sprites[i][0], number = char_required_sprites[i][1];
        if (KeyedArray_Find(sff->sprites, group, number) != -1) {
            continue;
        }
        if (missing_sprites == 0) {
            TextRing_Printf(ring, "%s (%i) missing required sprites:\n", p->name, p->id);
        } else if (missing_sprites >= 8) {
            TextRing_Printf(ring, " (more)");
            break;
        } else {
            TextRing_Printf(ring, ", ");
        }
        TextRing_Printf(ring, "%i,%i", group, number);
        missing_sprites++;
    }
    if (missing_sprites != 0) {
        TextRing_Printf(ring, "\n");
    }
    for (i = 0; char_required_actions[i] != -1; i++) {
        int action = char_required_actions[i];
        if (KeyedArray_Find(p->anim->anim->actions, 0, action) != -1) {
            continue;
        }
        if (missing_actions == 0) {
            TextRing_Printf(ring, "%s (%i) missing required animation actions:\n", p->name, p->id);
        } else if (missing_actions >= 8) {
            TextRing_Printf(ring, " (more)");
            break;
        } else {
            TextRing_Printf(ring, ", ");
        }
        TextRing_Printf(ring, "%i", action);
        missing_actions++;
    }
    if (missing_actions != 0) {
        TextRing_Printf(ring, "\n");
    }
    return missing_sprites == 0 && missing_actions == 0;
}

/* 0x0043B1B0: clears the persistent variables of the 16 entries of a team. */
void PersistVars_Reset(int team)
{
    unsigned char *entry;
    int i, j;

    Log_Printf("Reset persist vars team %i\n", team);
    entry = (unsigned char *)&g_game->teams[team] + offsetof(TeamSel, member_data);
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            memset(entry, 0, sizeof(PersistVars));
            entry += PERSIST_ENTRY_SIZE;
        }
    }
}

/* 0x0043B210: saves the vars and fvars of p from its persist indices on (the rest is cleared). */
void PersistVars_Save(const Player *p, PersistVars *dst)
{
    int index;

    Log_Printf("Save persist vars P%02i %s\n", p->id, p->name);
    memset(dst, 0, sizeof(*dst));
    index = p->consts.int_persist_index;
    memcpy(&dst->var[index], &p->var[index], (PERSIST_INT_COUNT - index) * sizeof(int));
    index = p->consts.float_persist_index;
    memcpy(&dst->fvar[index], &p->fvar[index], (PERSIST_FLOAT_COUNT - index) * sizeof(float));
}

/* 0x0043B280: in stress test and demo mode every player of the current team members is computer controlled. */
void Players_ForceAi(void)
{
    GameVars *g = g_game;
    int force = g->stresstest != 0 || g->demo_mode != 0;
    int t, i;

    for (t = 0; t < 2; t++) {
        const TeamMember *m = &g->teams[t].members[g->teams[t].cur_member];
        for (i = 0; i < m->count; i++) {
            Player *p = g->players[m->slots[i].player];
            if (force && p != NULL) {
                p->ai = 1;
            }
        }
    }
}

/* ------------------------------------------------------------ player lifecycle */

/*
 * 0x0043B320: removes what refers to p before it goes away: its explods and
 * targets, its helpers, and the references other players keep to it. The
 * borrowed list of p is detached once per other player found. Returns 1.
 */
int Player_Unlink(Player *p)
{
    GameVars *g;
    int i;

    Explods_RemoveOwner(g_game->explods, p->id, -1);
    Targets_Release(p, NULL);
    g = g_game;
    for (i = g->players_first; i <= g->players_last; i++) {
        Player *o = g->players[i];
        if (o == p || o == NULL) {
            continue;
        }
        if (o->root == p) {
            Helper_Remove(o, 0, 1);
            g = g_game;
            continue;
        }
        if (p->afterimage != NULL) {
            p->afterimage->running = 0;
            p->afterimage->anim = NULL;
            p->afterimage = NULL;
        }
        if ((o->custom_states != NULL ? o->custom_owner : -1) == p->index) {
            Player_ClearCustomState(o);
        }
        if (o->enemy == p) {
            o->enemy = NULL;
        }
        if (o->last_enemy == p) {
            o->last_enemy = NULL;
        }
        if (PlayerRef_Is(&o->bind_to, p) && o->bind_time != 0) {
            o->bind_y = 0.0f;
            o->bind_time = 0;
            o->bind_x = 0.0f;
            o->bind_facing = 0;
            o->unused_247C = 0;
            o->bind_to.valid = 0;
        }
    }
    return 1;
}

/* A configured key (GameVars.input_keys, 0x200 marks a joystick button) is held. */
static int Team_KeyHeld(int key)
{
    int code = key & ~0x200;

    if (code == 0x200 || code == 0) {
        code = 0x280;
    } else if (key & 0x200) {
        code |= 0x1000;     /* reads past g_inputState for joystick buttons, as the original does */
    }
    return ((const unsigned char *)g_inputState)[code] != 0;
}

/*
 * 0x0043B460: before a turns match the human player of team can change the
 * order of the remaining members by holding back/forward (one step) or up
 * (two steps, four members): the members, their persist vars and
 * GameVars.team_faces are rotated.
 */
void Team_RotateMembers(int team)
{
    GameVars *g = g_game;
    TeamSel *t = &g->teams[team];
    int cur = t->cur_member;
    int n = t->member_count - cur;
    TeamMember *m = &t->members[cur];
    unsigned char *data = (unsigned char *)t + offsetof(TeamSel, member_data);
    unsigned char saved_data[4 * PERSIST_ENTRY_SIZE];
    TeamMember saved;
    const int *keys;
    int dir, shift, i;
    SpriteSet *saved_value;

    if (n <= 1 || m->slots[0].ai != 0) {
        return;
    }
    keys = g->input_keys[m->slots[0].side];
    dir = Team_KeyHeld(keys[KEY_LEFT]) ? -1 : 0;
    if (Team_KeyHeld(keys[KEY_RIGHT])) {
        dir = 1;
    }
    if (team == 1) {
        dir = -dir;
    }
    if (Team_KeyHeld(keys[KEY_JUMP]) && n == 4) {
        dir = 2;
    }
    shift = dir % n;
    if (shift < 0) {
        shift += n;
    }
    if (shift <= 0) {
        return;
    }
    do {
        saved = t->members[cur];
        memcpy(saved_data, data + cur * sizeof(saved_data), sizeof(saved_data));
        saved_value = g->team_faces[team].sprites[cur];
        for (i = cur; i < t->member_count - 1; i++) {
            t->members[i] = t->members[i + 1];
            memcpy(data + i * sizeof(saved_data), data + (i + 1) * sizeof(saved_data), sizeof(saved_data));
            g->team_faces[team].sprites[i] = g->team_faces[team].sprites[i + 1];
        }
        t->members[t->member_count - 1] = saved;
        memcpy(data + (t->member_count - 1) * sizeof(saved_data), saved_data, sizeof(saved_data));
        g_game->team_faces[team].sprites[t->member_count - 1] = saved_value;
    } while (--shift != 0);
}

/* 0x0043B6B0: -1, 0 or 1 (0 for NaN). */
float Math_Sign(float v)
{
    if (v < 0.0f) {
        return -1.0f;
    }
    if (v > 0.0f) {
        return 1.0f;
    }
    return 0.0f;
}

/* 0x0043B6F0: puts a player back to its start-of-round state (players without data are left alone). */
void Player_Reset(Player *p)
{
    GameVars *g = g_game;
    Player *leader = p;
    KeyedArray *list;
    Coord c;
    int life, i;

    if (p->data == NULL) {
        return;
    }
    if (p->team_side == 1) {
        leader = g->players[1];
    }
    if (p->team_side == 2) {
        leader = g->players[2];
    }
    list = *(KeyedArray **)p->projectiles;
    p->suspended = 0;
    p->facing = 1;
    p->visible = 1;
    p->powerbar_power = 0;
    p->power = 0;
    p->next_facing = 0;
    p->sprpriority = 0;
    p->air_jumps = 0;
    p->attack_mul = (float)(p->consts.attack / 100.0);
    p->defence_mul = (float)(100.0 / p->consts.defence);
    life = g->life * p->consts.life / 100;
    c.x = 0.0;
    c.y = 0.0;
    c.z = 0.0;
    c.unit = 1.0;
    c.type = COORD_TYPE_2D;
    c.pad_24 = 0;       /* not set in the original (stack garbage) */
    p->power_max_ref = &leader->power_max;
    p->power_ref = &leader->power;
    p->vel = c;
    p->prev_pos = c;
    p->pos = c;
    p->unused_270 = 0.0f;
    p->life_max = life;
    p->lifebar_red = life;
    p->lifebar_life = life;
    p->life = life;
    p->power_max = p->consts.power;
    p->draw_offset = c;
    p->pause_movetime[0] = 0;
    p->pause_movetime[1] = 0;
    p->screenbound = 1;
    p->width_player_back = 0;
    p->width_player_front = 0;
    p->width_edge_back = 0;
    p->width_edge_front = 0;
    memset(&p->nostandguard, 0, 8);
    p->invisible = 0;
    p->proj_contact = 0;
    p->proj_contact_time = 0;
    p->proj_contact_id = 0;

    /* projectiles: removed (each detaches its borrowed list first) */
    {
        void *e;
        for (e = KeyedArray_First(list); e != NULL; e = KeyedArray_Next(list)) {
            KeyedArray *cur = *(KeyedArray **)p->projectiles;
            unsigned char *proj = cur->data + cur->elem_size * list->iter;
            if (proj != NULL) {
                KeyedArray **borrowed = (KeyedArray **)(proj + 0x504);
                if (*borrowed != NULL) {
                    (*borrowed)->data = NULL;
                    (*borrowed)->count = 0;
                    *borrowed = NULL;
                }
                KeyedArray_Remove(list->iter, cur);
            }
        }
    }
    /* targets: emptied */
    {
        KeyedArray *t = p->targets;
        memset(t->slots, 0, t->capacity * sizeof(KeyedArraySlot));
        t->count = 0;
        t->last = 0;
        t->first = 0;
        t->iter = 0;
        KeyedArray_ClearCache(t);
    }
    memset(&p->push_vel, 0, offsetof(Player, no_keyctrl) - offsetof(Player, push_vel));   /* Moemon Fight: the same fields on 64-bit */
    p->push_vel = c;
    p->push_decay_y = 1.0f;
    p->push_decay_x = 1.0f;
    memset(&p->input, 0, sizeof(p->input));
    memset(&p->cmd_state, 0, sizeof(p->cmd_state));
    p->input_cleared = 0;
    p->no_keyctrl = 0;
    p->no_input_states = 0;
    p->cmds = &p->data->cmds;
    memset(p->cmd_active, 0, sizeof(p->cmd_active));
    memset(p->unused_AB4, 0, sizeof(p->unused_AB4));
    Input_SetFlipped(&p->input, 0);
    Player_ClearCustomState(p);
    p->state_no = 0;
    p->prev_state_no = 0;
    memset(p->sctrl_wait, 0, sizeof(p->sctrl_wait));
    p->state_time = 0;
    p->statetype = STATETYPE_STAND;
    p->movetype = MOVETYPE_IDLE;
    p->physics = 1;
    p->ctrl = 1;
    p->in_guard_dist = 0;
    p->idle_time = 0;
    p->hitpause_next = 0;
    p->hitpause_time = 0;
    p->paused = 0;
    p->pos_freeze = 0;
    p->alive = 1;
    p->ko_time = 0;
    p->update_frame = -1;
    p->move_contact = 0;
    p->move_contact_time = 0;
    p->state_exists = 1;
    /* the vars below the persist indices are cleared, the others persist */
    memset(p->var, 0, p->consts.int_persist_index * sizeof(int));
    memset(p->fvar, 0, p->consts.float_persist_index * sizeof(float));
    for (i = 0; i < 5; i++) {
        p->sysvar[i] = 0;
    }
    for (i = 0; i < 5; i++) {
        p->sysfvar[i] = 0.0f;
    }
    {
        int *keep = p->hit_ids;
        memset(&p->nohit_time, 0, offsetof(Player, hit_pending) - offsetof(Player, nohit_time));   /* Moemon Fight: the same fields on 64-bit */
        p->hit_ids = keep;
    }
    p->nothitby_attr = 0xFFF;
    p->hitby_attr = 0xFFF;
    g = g_game;
    p->gethit_recovertime = p->consts.liedown_time;
    p->gethit_yaccel = (float)((int)g->game_coord[0] * (double)0.35f / 320.0);
    p->gethit_fall_xvel = -3.402823466e+38F;
    p->gethit_fall_yvel = (float)((int)g->game_coord[0] * -4.5 / 320.0);
    for (i = 0; i <= g->player_slots; i++) {
        p->hit_ids[i] = -1;
    }
    memset(p->hit_pending, 0, g->player_slots * sizeof(int) + sizeof(int));
    memset(&p->hitdef, 0, sizeof(p->hitdef));
    memset(&p->draw_zoom, 0, offsetof(Player, pad_14EC) - offsetof(Player, draw_zoom));   /* Moemon Fight: the same fields on 64-bit */
    DrawParams_Init(&p->draw);          /* inlined in the original */
    DrawParams_Init(&p->draw2);
    p->draw2.flags = 0;
    if (p->afterimage != NULL) {
        p->afterimage->running = 0;
        p->afterimage->anim = NULL;
        p->afterimage = NULL;
    }
    p->combo_hits = 0;
    p->hitcount = 0;
    p->uniqhitcount = 0;
    p->unused_1528[0] = 0;
    p->unused_1528[1] = 0;
    p->anim->anim = p->data->anim;
    p->anim_from_owner = 0;
    Player_SetAction(p, 0);
    Anim_Reset(p->anim);                /* inlined in the original */
    memset(&p->palfx_def, 0, sizeof(p->palfx_def));
    p->palfx_def.mul[2] = 0x100;
    p->palfx_def.mul[1] = 0x100;
    p->palfx_def.mul[0] = 0x100;
    p->palfx_def.color = 0x100;
    p->palfx_def.reset = 1;
    PalFx_Init(&p->palfx_own, NULL);    /* inlined in the original */
    if (!p->helper && p->sprites->version == 0x01000100) {
        /* SFF v1.01: the selected palette replaces palette 1,1 */
        PalRemapEntry e;
        e.src_group = 1;
        e.src_index = 1;
        e.dst_group = 1;
        e.dst_index = p->pal_no + 1;
        memset(&p->remap_own, 0, sizeof(p->remap_own));
        PalRemap_Set(&p->remap_own, &e);
    }
    Ai_ResetPlayer(p);
    memset(&p->bind_to, 0, offsetof(Player, action_failed) - offsetof(Player, bind_to));   /* Moemon Fight: the same fields on 64-bit */
    p->last_enemy = NULL;
    p->enemy = NULL;
    p->action_failed = 0;
    p->warnings->cur = 0;
    p->warnings->head = 0;
    p->warnings->lines[0][0] = '\0';
}

/* 0x0043BE60: adds a player to the active ones or removes it, keeping players_first..players_last tight. */
void Players_SetActive(int index, int active)
{
    GameVars *g = g_game;

    if (g->players[index]->active == active) {
        return;
    }
    if (active) {
        if (g->players_active == 0) {
            g->players_last = index;
            g->players_first = index;
        } else {
            if (index < g->players_first) {
                g->players_first = index;
            }
            if (index > g->players_last) {
                g->players_last = index;
            }
        }
    } else if (g->players_active == 1) {
        g->players_first = 0;
        g->players_last = -1;
    } else if (index == g->players_first) {
        do {
            g->players_first++;
        } while (g->players[g->players_first] == NULL || !g->players[g->players_first]->active);
    } else if (index == g->players_last) {
        do {
            g->players_last--;
        } while (g->players[g->players_last] == NULL || !g->players[g->players_last]->active);
    }
    g->players[index]->active = active;
    g->players_active += (active != 0) * 2 - 1;
}