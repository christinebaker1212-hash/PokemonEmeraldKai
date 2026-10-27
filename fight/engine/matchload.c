/*
 * Loading, starting and ending a match (original module 0x421E80-0x4259EF,
 * strings "Loading match assets..." to "Match RNG closed"): the stage
 * (stages/*.def), the players of both teams from the player cache, the
 * helper slots, the quick VS and demo teams, the match RNG, the start and
 * the reset of a round, the next round of a team match, the end of a match
 * (victory quote) and the shadows and reflections of the stage.
 *
 * Floating point follows the x87 code of the original: intermediate results
 * stay in double and are rounded to float where the original stores a float.
 */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <SDL_mixer.h>
#define HAVE_STRUCT_TIMESPEC 1      /* the CRT's time.h already has it */
#include <pthread.h>

#include "mugen.h"

/* Timer_GetTicks as the original inlines it (Timer_Count included). */
static int Ml_TimerTicks(void)
{
    __int64 now;

    if (g_timerPerformance) {
        LARGE_INTEGER c;
        QueryPerformanceCounter(&c);
        now = c.QuadPart - g_timerStart;
    } else {
        now = (__int64)timeGetTime() - g_timerStart;
    }
    return (int)((__int64)g_timerTicksPerSecond * now / g_timerFrequency);
}

/* The session RNG (Random_Next) as the original inlines it. */
static int Ml_SessionRandom(GameVars *g)
{
    g->session_rng = g->session_rng * g->session_rng_mul % g->session_rng_mod;
    return (int)(unsigned int)g->session_rng;
}

/* A state change to `state` as inlined by the original: warns when the player has no such state. */
static void Ml_ChangeState(Player *p, int state)
{
    StateDefs *states = p->custom_states != NULL ? p->custom_states : p->states;
    KeyedArray *list = states->list;
    int index = KeyedArray_Find(list, state, 0);
    int exists = index != -1 && list->data + list->elem_size * index != NULL;
    int prev = p->state_no;

    p->prev_state_no = prev;
    p->state_no = state;
    p->state_time = 0;
    if (!exists) {
        Player_Warnf(p, "changed to invalid state %i (from state %i)\n", state, prev);
    }
}

/* The player masks of the palette groups for player number n (bit n - 1). */
static void Ml_SetPlayerMask(Player *p, int n)
{
    int bit = n - 1;

    p->pal_groups[0] = 0;
    if (bit < 32) {
        p->pal_groups[1] = 1u << bit;
        p->pal_groups[2] = 0;
    } else {
        p->pal_groups[1] = 0;
        p->pal_groups[2] = 1u << (n - 33);
    }
    p->pal_groups[3] = 0;
    p->pal_groups[4] = 0;
}

/* The persist vars entry of team `team`, member `member`, slot `slot` (TeamSel.member_data[0]). */
static PersistVars *Ml_PersistEntry(GameVars *g, int team, int member, int slot)
{
    return &g->teams[team].member_data[member][slot].vars;
}

/* ------------------------------------------------------------------ match assets */

/*
 * 0x00421E80: loads a match: the stage (fatal error when it fails), its
 * camera and BGM, the turns mode faces, the helper slots, then starts it
 * (Match_Start). The time it took is kept in GameVars.match_load_ticks.
 */
void Match_LoadAssets(void)
{
    GameVars *g = g_game;

    g->player_id_next = 0;
    Log_Printf("Loading match assets...\n");
    g_game->match_load_ticks = (int)Timer_GetTicks();
    if (!Stage_Load(&g_game->stage)) {
        Error_Exit();
    }
    if (!Stage_Activate(&g_game->stage)) {
        Error_Exit();
    }
    if (!Match_LoadTeamFaces()) {
        return;
    }
    Helpers_Alloc();
    g = g_game;
    g->debug_player = g->debug != 0;
    g->teams[0].cur_member = 0;
    g->teams[1].cur_member = 0;
    memcpy(&g->match_members[0], &g->teams[0].members[g->teams[0].cur_member], sizeof(TeamMember));
    memcpy(&g->match_members[1], &g->teams[1].members[g->teams[1].cur_member], sizeof(TeamMember));
    Match_Start();
    g = g_game;
    g->match_load_ticks = Ml_TimerTicks() - g->match_load_ticks;
    Log_Printf("Match assets initialized OK");
    Log_Printf("\n");
    g_game->match_loaded = 1;
}

/*
 * 0x00422000: the demo mode teams: one random character with a random
 * palette (of the first 12) on each side, chosen with the session RNG.
 */
void Demo_SetupTeams(void)
{
    GameVars *g = g_game;
    int i;

    g->team_variant = 0;
    g->arcade_side = 0;
    g->teams[1].member_count = 1;
    g->teams[0].member_count = 1;
    g->teams[1].members[0].count = 0;
    g->teams[0].members[0].count = 0;
    g->teams[1].members[0].mode = TEAM_SINGLE;
    g->teams[0].members[0].mode = TEAM_SINGLE;
    for (i = 0; i < 2; i++) {
        int side = i % 2;
        TeamSel *t = &g->teams[side];
        TeamSlot *slot = &t->members[0].slots[i / 2];
        int r;

        slot->player = i + 1;
        slot->side = side;
        slot->char_index = Ml_SessionRandom(g) % g->char_count;
        r = Ml_SessionRandom(g);
        slot->pal = PalReserve_Request(slot->char_index, r % 12);
        g = g_game;
        t->members[0].count++;
        if (t->members[0].count > 1) {
            t->members[0].mode = TEAM_SIMUL;
            t->members[0].simul_mode = 1;
            g->team_variant = 0;
        }
        t->char_count++;
        g->unused_12378[i] = 1;
        g->player_sides[i] = i & 1;
    }
    memcpy(&g->match_members[0], &g->teams[0].members[0], sizeof(TeamMember));
    memcpy(&g->match_members[1], &g->teams[1].members[0], sizeof(TeamMember));
    g->select_stage[0] = '\0';
}

/*
 * 0x004221C0: game flow step 6: the persist vars are reset; the character
 * select comes next (7), or the stage pick (8) for quick VS and for demo
 * fights without the select screen (random demo teams).
 */
int Gameflow_SetupMatch(void)
{
    GameVars *g;
    int i;

    PersistVars_Reset(0);
    PersistVars_Reset(1);
    g = g_game;
    g->arcade_match = 0;
    g->continued = 0;
    for (i = 0; i < 6; i++) {
        g->unused_8718[i] = 0;
    }
    if (g->quick_vs == 0 && (g->demo_mode == 0 || g->demo_select_enabled != 0)) {
        return 7;
    }
    if (g->demo_mode != 0) {
        Demo_SetupTeams();
    }
    return 8;
}

/*
 * 0x00422240: the quick VS teams from the command line (-p1..-p4): each
 * character gets a GameVars.char_info entry ("chars/<name>/<name>.def", or
 * the given path under chars/), its [Info] pal.defaults (the palette of
 * player n is pal.defaults[n] unless -p<n>.color is given); -rounds sets
 * [Round] match.wins and -s the stage (under stages/ unless it starts with
 * a slash, ".def" added). A missing character is a fatal error.
 *
 * Quirks kept: a player naming the same character as an earlier player j
 * takes the palette default from char_info[j] (j is the player, not its
 * character index), and the directory and file names are used as sprintf
 * formats.
 */
void QuickVs_SetupTeams(void)
{
    CmdLineArgs *cl = g_cmdline;
    GameVars *g = g_game;
    char dir[0x200], path[0x200], file[0x204];
    int i;

    g->teams[1].member_count = 1;
    g->teams[0].member_count = 1;
    g->teams[1].members[0].count = 0;
    g->teams[0].members[0].count = 0;
    g->teams[1].members[0].mode = TEAM_SINGLE;
    g->teams[0].members[0].mode = TEAM_SINGLE;
    for (i = 0; i < 4; i++) {
        int side = i % 2;
        TeamSel *t = &g->teams[side];
        TeamSlot *slot;
        int info_index, j, k;

        if (cl->player[i][0] == '\0') {
            continue;
        }
        slot = &t->members[0].slots[i / 2];
        slot->player = i + 1;
        slot->side = side;
        for (j = 0; j < i; j++) {
            if (strcmp(cl->player[i], cl->player[j]) == 0) {
                break;
            }
        }
        if (j < i) {
            slot->char_index = g->teams[j % 2].members[0].slots[j / 2].char_index;
            info_index = j;
        } else {
            int ci = g->char_count;
            CharInfo *info = &g->char_info[ci];
            Ini *ini;

            slot->char_index = ci;
            Path_Split(cl->player[i], dir, file);
            if (dir[0] == '\0') {
                sprintf(dir, "chars/%s/", cl->player[i]);
                sprintf(file, "%s.def", cl->player[i]);
            } else {
                static const char chars[] = "chars";
                for (k = 0; k < 5; k++) {
                    if (dir[k] != chars[k] || dir[k] == '\0' || chars[k] == '\0') {
                        break;
                    }
                }
                if (k < 5) {
                    sprintf(path, "chars/%s", dir);
                    strcpy(dir, path);
                }
            }
            sprintf(path, "%s%s", dir, file);
            ini = Ini_Open(path);
            if (ini == NULL) {
                Error_Fatal("Cannot find character: %s\nPlease specify a valid character def file for QuickVS.\n",
                            path);
            }
            Ini_SelectGroup(ini, "Info");
            Ini_Scanf(ini, "pal.defaults", "%i,%i,%i,%i", &info->pal_defaults[0], &info->pal_defaults[1],
                      &info->pal_defaults[2], &info->pal_defaults[3]);
            for (k = 0; k < 4; k++) {
                if (info->pal_defaults[k] < 1 || info->pal_defaults[k] > 12) {
                    info->pal_defaults[k] = k + 1;
                }
            }
            Ini_Free(ini);
            sprintf(g_game->char_info[ci].dir, dir);
            sprintf(g_game->char_info[ci].def_file, file);
            g = g_game;
            g->char_count++;
            info_index = ci;
        }
        slot->pal = g->char_info[info_index].pal_defaults[i] - 1;
        if (g->override_color[i] != -1) {
            slot->pal = g->override_color[i];
        }
        t->members[0].count++;
        if (t->members[0].count > 1) {
            t->members[0].mode = TEAM_SIMUL;
            t->members[0].simul_mode = 1;
            g->team_variant = 0;
        }
        t->char_count++;
        g->player_sides[i] = i & 1;
        g->unused_12378[i] = 1;
    }
    memcpy(&g->match_members[0], &g->teams[0].members[0], sizeof(TeamMember));
    memcpy(&g->match_members[1], &g->teams[1].members[0], sizeof(TeamMember));
    g->fight.round.match_wins = cl->rounds;
    if (cl->stage[0] != '\0') {
        int len;
        _snprintf(g->select_stage, sizeof(g->select_stage),
                  cl->stage[0] == '\\' || cl->stage[0] == '/' ? "%s" : "stages/%s", cl->stage);
        len = (int)strlen(g_game->select_stage);
        if (len > 4 && _stricmp(g_game->select_stage + len - 4, ".def") != 0) {
            strncat(g_game->select_stage, ".def", 0x200 - len);
        }
    }
}

/*
 * 0x004227D0: game flow step 8: counts the match, sets up the quick VS
 * teams, then the stage: the select screen's, the arcade opponent's (with
 * its music), [Debug] StartStage for quick VS, or a random one of the stage
 * list for "random" or none.
 */
void Gameflow_PickStage(void)
{
    GameVars *g = g_game;
    int quick;

    if (g->continued == 0) {
        g->arcade_match++;
    }
    g->matches_picked++;
    quick = g->quick_vs != 0;
    if (quick) {
        QuickVs_SetupTeams();
        g = g_game;
        quick = g->quick_vs != 0;
    }
    if (quick && g->select_stage[0] == '\0') {
        strncpy(g->select_stage, g->start_stage, 0x200);
        g = g_game;
    }
    g->arcade_music[0] = '\0';
    if (g->arcade_side != 0) {
        int idx = Arcade_NextTeam();
        if (idx != -1) {
            CharInfo *info = &g_game->char_info[idx];
            g = g_game;
            if (g->continued == 0 || g->select_stage[0] == '\0') {
                strncpy(g->select_stage, info->stage, 0x200);
                g = g_game;
            }
            strncpy(g->arcade_music, info->music, 0x200);
        }
        g = g_game;
    }
    if (g->select_stage[0] == '\0' || _stricmp(g->select_stage, "random") == 0) {
        const char *src;
        g = g_game;
        if (g->stage_count > 0) {
            src = g->select_screen[Ml_SessionRandom(g) % g->stage_count].path;
        } else {
            src = g->start_stage;
        }
        strncpy(g->select_stage, src, 0x200);
    }
}

/* ------------------------------------------------------------------ the stage */

/*
 * 0x00422960: loads the stage GameVars.select_stage into st (nothing when it
 * is already loaded): [Info], [Camera], [PlayerInfo], [Bound], [StageInfo],
 * [Shadow], [Reflection], [Music] and the backgrounds. Returns 0 on errors
 * (logged with Error_Printf; the parsed file is not freed then, nor when
 * displayname or author cannot be read).
 */
int Stage_Load(Stage *st)
{
    const char *path = g_game->select_stage;
    int color[3] = { 0, 0, 0 };     /* r, g, b */
    int zoom_given = 0;
    int w, h, ok;
    const char *v;
    Ini *ini;
    Stream *s;

    if (st->loaded != 0 && strcmp(st->path, path) == 0) {
        Log_Printf("Stage already loaded: %s\n", path);
        return 1;
    }
    Log_Printf("Loading stage...\n");
    if (st->loaded != 0) {
        Stage_Unload(st);
    }
    memset(st, 0, sizeof(Stage));
    s = Stream_OpenFile(path, "rt");
    ini = NULL;
    if (s != NULL) {
        ini = Ini_Parse(s, INI_MODE_READ);
        Stream_Close(s);
    }
    if (ini == NULL) {
        Error_Printf("Can't open stage: %s\n", path);
        return 0;
    }

    if (Ini_SelectGroup(ini, "Info") != -1) {
        if (!Ini_ReadString(ini, "name", sizeof(st->name), st->name)) {
            st->name[0] = '\0';
            strncat(st->name, "Undefined", sizeof(st->name) - 1);
        }
        if (Ini_GetValue(ini, "displayname") != NULL) {
            if (!Ini_ReadString(ini, "displayname", sizeof(st->displayname), st->displayname)) {
                return 0;
            }
        } else {
            st->displayname[0] = '\0';
            strncat(st->displayname, st->name, sizeof(st->displayname) - 1);
        }
        if (Ini_GetValue(ini, "author") != NULL &&
            !Ini_ReadString(ini, "author", sizeof(st->author), st->author)) {
            return 0;
        }
        st->versiondate = Version_ParseDate(Ini_GetValue(ini, "versiondate"));
        st->mugenversion = Version_Parse(Ini_GetValue(ini, "mugenversion"));
        if (st->mugenversion < g_game->compat_version[1]) {
            TextRing_Printf(g_game->clipboard, "Info: stage %s loading in pre-1.0 compatible mode\n",
                            st->displayname);
        }
    }

    if (Ini_SelectGroup(ini, "Camera") == -1) {
        Error_Printf("Missing camera info: %s\n", path);
        return 0;
    }
    st->verticalfollow = 0.2f;
    st->startzoom = 1.0f;
    st->floortension = 0;
    st->zoomout = 1.0f;
    st->tensionlow = 0;
    st->zoomin = 1.0f;
    st->tensionhigh = 0;
    st->tension = 50;
    st->boundhigh = -25;
    st->boundlow = 0;
    st->camera_starty = 0;
    st->camera_startx = 0;
    st->boundleft = -75;
    st->boundright = 75;
    st->cuthigh = 0;
    st->cutlow = -1;
    st->overdrawlow = 0;
    st->overdrawhigh = 0;
    if ((v = Ini_GetValue(ini, "boundleft")) != NULL) {
        st->boundleft = atoi(v);
    }
    if ((v = Ini_GetValue(ini, "boundright")) != NULL) {
        st->boundright = atoi(v);
    }
    if ((v = Ini_GetValue(ini, "startx")) != NULL) {
        st->camera_startx = atoi(v);
    }
    if ((v = Ini_GetValue(ini, "starty")) != NULL) {
        st->camera_starty = atoi(v);
    }
    if ((v = Ini_GetValue(ini, "verticalfollow")) != NULL) {
        st->verticalfollow = (float)atof(v);
    }
    if ((v = Ini_GetValue(ini, "tension")) != NULL) {
        st->tension = atoi(v);
    }
    if ((v = Ini_GetValue(ini, "floortension")) != NULL) {
        st->floortension = atoi(v);
    }
    if ((v = Ini_GetValue(ini, "tensionhigh")) != NULL) {
        st->tensionhigh = atoi(v);
    }
    if ((v = Ini_GetValue(ini, "tensionlow")) != NULL) {
        /* a tensionlow switches verticalfollow to the zooming mode */
        st->tensionlow = atoi(v);
        st->verticalfollow = -1.0f;
    }
    if ((v = Ini_GetValue(ini, "boundhigh")) != NULL) {
        st->boundhigh = atoi(v);
    }
    if ((v = Ini_GetValue(ini, "boundlow")) != NULL) {
        st->boundlow = atoi(v);
    }
    if ((v = Ini_GetValue(ini, "overdrawhigh")) != NULL) {
        st->overdrawhigh = atoi(v);
    }
    if ((v = Ini_GetValue(ini, "overdrawlow")) != NULL) {
        st->overdrawlow = atoi(v);
    }
    if ((v = Ini_GetValue(ini, "overdrawleft")) != NULL) {
        st->overdrawleft = atoi(v);
    }
    if ((v = Ini_GetValue(ini, "overdrawright")) != NULL) {
        st->overdrawright = atoi(v);
    }
    if ((v = Ini_GetValue(ini, "cuthigh")) != NULL) {
        st->cuthigh = atoi(v);
    }
    if ((v = Ini_GetValue(ini, "cutlow")) != NULL) {
        st->cutlow = atoi(v);
    }
    if ((v = Ini_GetValue(ini, "zoomout")) != NULL) {
        st->zoomout = (float)atof(v);
        zoom_given = 1;
    }
    if ((v = Ini_GetValue(ini, "zoomin")) != NULL) {
        st->zoomin = (float)atof(v);
        zoom_given = 1;
    }
    if (st->zoomout < 0.001) {
        st->zoomout = 0.001f;
    }
    if (st->zoomout > st->zoomin) {
        st->zoomin = st->zoomout;
    }
    if ((v = Ini_GetValue(ini, "startzoom")) != NULL) {
        st->startzoom = (float)atof(v);
    }

    if (Ini_SelectGroup(ini, "PlayerInfo") == -1) {
        Error_Printf("Missing player info: %s\n", path);
        return 0;
    }
    ok = Ini_GetInt(ini, "rightbound", &st->rightbound);
    ok &= Ini_GetInt(ini, "leftbound", &st->leftbound);
    ok = Ini_GetInt(ini, "p2facing", &st->facing[0]) & ok;
    ok = Ini_GetInt(ini, "p1facing", &st->startz[4]) & ok;
    ok = Ini_GetInt(ini, "p2starty", &st->starty[1]) & ok;
    ok = Ini_GetInt(ini, "p1starty", &st->starty[0]) & ok;
    ok = Ini_GetInt(ini, "p2startx", &st->startx[1]) & ok;
    ok = Ini_GetInt(ini, "p1startx", &st->startx[0]) & ok;
    if (ok != 1) {
        Error_Printf("Can't read player info: %s\n", path);
        return 0;
    }
    st->starty[2] = st->starty[0];
    st->startx[2] = st->startx[0] - 25;
    st->startx[3] = st->startx[1] + 25;
    st->facing[1] = st->startz[4];
    st->facing[2] = st->facing[0];
    st->starty[3] = st->starty[1];
    Ini_GetInt(ini, "p3startx", &st->startx[2]);
    Ini_GetInt(ini, "p3starty", &st->starty[2]);
    Ini_GetInt(ini, "p3facing", &st->facing[1]);
    Ini_GetInt(ini, "p4startx", &st->startx[3]);
    Ini_GetInt(ini, "p4starty", &st->starty[3]);
    Ini_GetInt(ini, "p4facing", &st->facing[2]);
    Ini_GetInt(ini, "p1startz", &st->startz[0]);
    Ini_GetInt(ini, "p2startz", &st->startz[1]);
    Ini_GetInt(ini, "p3startz", &st->startz[2]);
    Ini_GetInt(ini, "p4startz", &st->startz[3]);

    if (Ini_SelectGroup(ini, "Bound") == -1) {
        Error_Printf("Missing bound info: %s\n", path);
        return 0;
    }
    ok = Ini_GetInt(ini, "screenleft", &st->screenleft);
    ok &= Ini_GetInt(ini, "screenright", &st->screenright);
    if (ok != 1) {
        Error_Printf("Can't read bound info: %s\n", path);
        return 0;
    }

    if (Ini_SelectGroup(ini, "StageInfo") == -1) {
        Error_Printf("Missing stage info: %s\n", path);
        return 0;
    }
    Ini_GetInt(ini, "hires", &st->hires);
    w = 320;
    h = 240;
    Ini_Scanf(ini, "localcoord", "%i,%i", &w, &h);
    if (w < 1) {
        w = 1;
    }
    if (h < 1) {
        h = 1;
    }
    st->unit = (double)w;
    st->height = (double)h;
    st->half_width = (double)((w - 1) / 2);
    st->half_height = (double)((h - 1) / 2);
    {
        GameVars *g = g_game;
        float fz = g->force_stage_zoomout;
        if (fz != 0.0f && !zoom_given) {
            int d = Ftol((1.0 / fz - 1.0) * st->unit * 0.5);
            st->boundleft += d;
            st->zoomout = g->force_stage_zoomout;
            st->boundright -= d;
        }
    }
    st->zoffset = 190;
    st->zoffsetlink = -1;
    Ini_GetInt(ini, "zoffset", &st->zoffset);
    Ini_GetInt(ini, "zoffsetlink", &st->zoffsetlink);
    if (st->cutlow < 0) {
        if (st->zoffsetlink == -1) {
            st->cutlow = Ftol((st->height - st->zoffset) - st->height / 20.0);
        }
        if (st->cutlow < 0) {
            st->cutlow = 0;
        }
    }
    st->autoturn = 0;
    Ini_GetInt(ini, "autoturn", &st->autoturn);
    st->resetbg = 1;
    Ini_GetInt(ini, "resetbg", &st->resetbg);
    st->yscale = 1.0f;
    st->xscale = 1.0f;
    if (st->hires != 0) {
        st->yscale = 0.5f;
        st->xscale = 0.5f;
    }
    Ini_GetFloat(ini, "xscale", &st->xscale);
    Ini_GetFloat(ini, "yscale", &st->yscale);

    *(float *)&st->shadow[2] = 0.4f;
    st->shadow[0] = 0x80;
    if (Ini_SelectGroup(ini, "Shadow") != -1) {
        Ini_Scanf(ini, "color", "%i,%i,%i", &color[0], &color[1], &color[2]);
        Ini_GetInt(ini, "intensity", &st->shadow[0]);
        Ini_GetFloat(ini, "yscale", (float *)&st->shadow[2]);
        Ini_GetFloat(ini, "xshear", (float *)&st->shadow[3]);
        Ini_Scanf(ini, "fade.range", "%i,%i", &st->shadow[4], &st->shadow[5]);
    }
    st->shadow[1] = (int)Video_MapRGB(color[0], color[1], color[2]);
    if (Ini_SelectGroup(ini, "Reflection") != -1) {
        Ini_GetInt(ini, "intensity", &st->reflection_intensity);
    }
    if (Ini_SelectGroup(ini, "Music") != -1) {
        Bgm_ReadInfoKeys(&st->bgm, "bgmusic", ini, "bgmvolume", "bgmloop", "bgmloopstart", "bgmloopend");
    }

    Log_Printf("  Loading BG...");
    g_game->load_ticks[0] = (int)Timer_GetTicks();
    if (!Stage_LoadBg(st, ini)) {
        Error_Printf("BG error reading %s\n", path);
        return 0;
    }
    g_game->load_ticks[0] = (int)Timer_GetTicks() - g_game->load_ticks[0];
    Log_Printf("OK\n");
    Ini_Free(ini);
    st->path[0] = '\0';
    strncat(st->path, path, sizeof(st->path) - 1);
    st->loaded = 1;
    Log_Printf("Stage loaded OK\n");
    return 1;
}

/*
 * 0x00423580: makes the loaded stage the one of the match: its coordinate
 * space, the 3D projection, the screen offset of its zoffset, its
 * background behind the fight, and its BGM (or the arcade opponent's music)
 * unless a demo fight keeps the BGM. Returns 0 when there is no stage or the
 * BGM cannot be loaded.
 */
int Stage_Activate(Stage *st)
{
    GameVars *g;
    BgmInfo music;
    const BgmInfo *bgm;
    int y;

    if (st->loaded == 0) {
        return 0;
    }
    Gfx_SetCoordSpace(Ftol(st->unit), Ftol(st->height), g_game->stage_fit);
    g_glNear = 1000.0;
    g_glFar = 10000.0;
    g_glProjection = 1;
    g_glZScale = 1.0;
    if (g_renderMode == 2) {
        Gl_SetupView();
    }
    g = g_game;
    y = Ftol(g_gameCoord[0] / g->stage.unit * st->zoffset);
    g->draw_offset_y = y;
    g->stage_zoffset = y;
    g->stage_bg = st->bg;
    if (g->sound_enabled == 0) {
        return 1;
    }
    if (g->arcade_music[0] != '\0') {
        /* the original leaves unused_21c of this BgmInfo uninitialised */
        strcpy(music.path, g->arcade_music);
        music.volume = 100.0;
        music.loop = 1;
        music.loop_start = 0;
        music.loop_end = -1;
        music.unused_21c = 0;
        bgm = &music;
    } else {
        bgm = &st->bgm;
    }
    if (g->demo_mode != 0 && g->demo_fight_playbgm == 0) {
        return 1;
    }
    Bgm_Stop();
    if (!Bgm_Load(bgm)) {
        return 0;
    }
    return 1;
}

/*
 * 0x004236E0: loads the stage backgrounds ("BG" groups), with the sprites of
 * [BGdef] spr relative to the stage's directory (a "stages/" prefix of spr
 * is dropped from that directory). Legacy (pre-1.0) stages get the old
 * background behaviour. Returns 0 when the backgrounds cannot be loaded.
 */
int Stage_LoadBg(Stage *st, Ini *ini)
{
    char dir[0x200];
    const char *spr;
    Bg *bg;
    int n;

    dir[0] = '\0';
    if (Ini_SelectGroup(ini, "BGdef") != -1) {
        const char *path = g_game->select_stage;
        if (path[0] != '\0') {
            n = (int)strlen(path) - 1;
            while (n >= 0 && path[n] != '\\' && path[n] != '/') {
                n--;
            }
            n++;
            if (n > 0) {
                strncpy(dir, path, n);
            }
            dir[n] = '\0';
        } else {
            dir[0] = '\0';
        }
        spr = Ini_GetValue(ini, "spr");
        if (spr != NULL && (Str_PrefixMatchI(spr, "stages/", 7) || Str_PrefixMatchI(spr, "stages\\", 7))) {
            int len = (int)strlen(dir);
            char *tail = NULL;
            /*
             * The original tests dir[len - 8] for a backslash whatever the
             * length: below 8 it reads uninitialised stack bytes in front
             * of dir, taken here as not a backslash.
             */
            if (len > 8 && dir[len - 8] == '/') {
                tail = dir + len;
            } else if (len >= 8 && dir[len - 8] == '\\') {
                tail = dir + len;
            } else if (len == 7) {
                tail = dir + len;
            }
            if (tail != NULL) {
                char *t = tail - 7;
                if (_stricmp(t, "stages/") == 0 || _stricmp(t, "stages\\") == 0) {
                    *t = '\0';
                }
            }
        }
    }
    bg = Bg_Load("BG", ini, dir, NULL, NULL);
    st->bg = bg;
    if (bg == NULL) {
        return 0;
    }
    {
        GameVars *g = g_game;
        int half = Ftol(g->stage.unit) / 2;
        int dx = Ftol((double)half / st->xscale);
        Bg_ShiftWindows(st->bg, dx);
        if (st->mugenversion < g->compat_version[1]) {
            Bg_SetLegacy(st->bg);
        }
    }
    return 1;
}

/*
 * 0x00423FC0: frees the loaded stage (its backgrounds; the fight's back layer
 * is cleared when it is this stage's) and clears it.
 */
void Stage_Unload(Stage *st)
{
    GameVars *g;

    if (st->loaded == 0) {
        return;
    }
    Log_Printf("Unloading stage...");
    g = g_game;
    if (g->stage_bg == st->bg) {
        g->stage_bg = NULL;
    }
    Bg_Free(st->bg);
    memset(st, 0, sizeof(Stage));
    Log_Printf("OK\n");
}

/* ------------------------------------------------------------------ the players */

/*
 * 0x00423890: the helper slots: players 5 and up (at most 63), one per
 * [Config] HelperMax, created without character data or emptied when they
 * exist. A failure is fatal.
 */
void Helpers_Alloc(void)
{
    GameVars *g;
    int k;

    Log_Printf("  Allocating helpers...");
    g = g_game;
    for (k = 0; k < g->helper_slots; k++) {
        int idx = 5 + k;
        Player *p;

        if (idx > 63) {
            break;
        }
        p = g->players[idx];
        if (p != NULL) {
            Players_SetActive(p->index, 0);
            g->players[idx]->data = NULL;
            continue;
        }
        p = Player_Create(NULL);
        g = g_game;
        g->players[idx] = p;
        if (p == NULL) {
            Error_Printf("Can't create helper characters\n");
            Error_Exit();
        }
        p->index = idx;
        if (idx - 1 < 32) {
            p->pal_groups[0] = 0;
            p->pal_groups[1] = 1u << (idx - 1);
            p->pal_groups[2] = 0;
            p->pal_groups[3] = 0;
            p->pal_groups[4] = 0;
        } else if (idx - 1 < 64) {
            p->pal_groups[0] = 0;
            p->pal_groups[1] = 0;
            p->pal_groups[2] = 1u << (idx - 1 - 32);
            p->pal_groups[3] = 0;
            p->pal_groups[4] = 0;
        } else {
            memset(p->pal_groups, 0, sizeof(p->pal_groups));
        }
    }
    Log_Printf("OK\n");
}

/*
 * 0x004239C0: replaces the players of team `team` by its current member
 * (turns and survival): the old ones are freed (their persist vars saved),
 * the new ones created; the leader keeps its power. A failure is fatal.
 */
void Team_ReloadMember(int team)
{
    GameVars *g = g_game;
    TeamMember *m = &g->teams[team].members[g->teams[team].cur_member];
    int power = -1;
    Player *p;

    Match_FreeAssets();
    p = g_game->players[team + 1];
    if (p != NULL) {
        power = p->power;
    }
    if (!Team_FreePlayers(team)) {
        Error_Exit();
    }
    if (!Team_CreatePlayers(team)) {
        Error_Exit();
    }
    if (power != -1) {
        g = g_game;
        if (g->players[team + 1] != NULL) {
            /* the power level shown gets the power itself */
            g->players[team + 1]->powerbar_level = power;
            g->players[team + 1]->powerbar_power = power;
            g->players[team + 1]->power = power;
        }
    }
    PalGroups_Build();
    memcpy(&g_game->match_members[team], m, sizeof(TeamMember));
    Players_ForceAi();
}

/*
 * 0x00423A90: creates the players of the current member of team `team` from
 * the player cache: team, persist vars, input device, AI, palette; they are
 * put in the match. Returns 0 (logged) when a character cannot be loaded or
 * a player created.
 */
int Team_CreatePlayers(int team)
{
    GameVars *g = g_game;
    TeamSel *t = &g->teams[team];
    TeamMember *m = &t->members[t->cur_member];
    int k;

    for (k = 0; k < m->count; k++) {
        TeamSlot *slot = &m->slots[k];
        int n;
        CharData *c;

        if (slot->char_index == -1) {
            continue;
        }
        n = slot->player;
        if (n - 1 < 4) {
            g_game->load_ticks[n] = (int)Timer_GetTicks();
        }
        c = PlayerCache_Load(slot->char_index);
        g = g_game;
        g->player_chars[n] = c;
        if (c == NULL) {
            Error_Printf("Error loading p%i\n", n);
            return 0;
        }
        if (n - 1 < 4) {
            int *ticks = &g->load_ticks[n];
            *ticks = (int)Timer_GetTicks() - *ticks;
            g = g_game;
        }
    }
    m = &t->members[t->cur_member];
    for (k = 0; k < m->count; k++) {
        TeamSlot *slot = &m->slots[k];
        int n = slot->player;
        Player *p = Player_Create(g->player_chars[n]);
        PersistVars *pv;
        int index;

        g = g_game;
        g->players[n] = p;
        if (p == NULL) {
            Error_Printf("Can't create player %i\n", n);
            return 0;
        }
        p->team_index = team;
        p->team_side = (team != 0) + 1;
        p->team_member = t->cur_member;
        p->team_slot = k;
        pv = Ml_PersistEntry(g, team, t->cur_member, k);
        index = p->consts.int_persist_index;
        memcpy(&p->var[index], &pv->var[index], (PERSIST_INT_COUNT - index) * sizeof(int));
        index = p->consts.float_persist_index;
        memcpy(&p->fvar[index], &pv->fvar[index], (PERSIST_FLOAT_COUNT - index) * sizeof(float));
        p->input_keys = g->input_keys[slot->side];
        if (slot->ai != 0) {
            p->ai = 1;
        }
        p->pal_no = slot->pal;
        if (g->players[n]->active != 1) {
            if (g->players_active == 0) {
                g->players_first = n;
                g->players_last = n;
            } else {
                if (n < g->players_first) {
                    g->players_first = n;
                }
                if (n > g->players_last) {
                    g->players_last = n;
                }
            }
            g->players[n]->active = 1;
            g->players_active++;
        }
        Player_CheckRequired(p);
        p->index = n;
        Ml_SetPlayerMask(p, n);
    }
    return 1;
}

/*
 * 0x00423D70: frees the players of team `team` (GameVars.match_members): out of
 * the match, persist vars saved, unlinked, freed and released from the
 * player cache. Returns 1.
 */
int Team_FreePlayers(int team)
{
    GameVars *g = g_game;
    TeamMember *m = &g->match_members[team];
    int k;

    for (k = 0; k < m->count; k++) {
        int n = m->slots[k].player;
        Player *p = g->players[n];
        int ci;

        if (p == NULL) {
            continue;
        }
        if (p->active) {
            Players_SetActive(n, 0);
        }
        p = g->players[n];
        if (p->team_index != -1) {
            PersistVars_Save(p, Ml_PersistEntry(g, p->team_index, p->team_member, p->team_slot));
            g = g_game;
        }
        p = g->players[n];
        ci = p->data != NULL ? p->data->char_index : -1;
        Player_Unlink(p);
        Player_Free(g_game->players[n]);
        if (ci != -1) {
            PlayerCache_Release(ci, 0);
        }
        g = g_game;
        g->player_chars[n] = NULL;
        g->players[n] = NULL;
    }
    return 1;
}

/*
 * 0x00423E90: frees every player (helpers included): out of the match,
 * persist vars saved, freed and released from the player cache (with the
 * characters selected as serial for the arcade team).
 */
void Players_Free(void)
{
    GameVars *g;
    int i;

    Log_Printf("Freeing players\n");
    g = g_game;
    for (i = 1; i <= g->player_slots; i++) {
        Player *p = g->players[i];
        int ci = -1, serial = 0;

        if (p == NULL) {
            continue;
        }
        if (p->active) {
            Players_SetActive(i, 0);
        }
        if (p->team_index != -1) {
            Player *q = g->players[i];
            PersistVars_Save(p, Ml_PersistEntry(g, q->team_index, q->team_member, q->team_slot));
            g = g_game;
        }
        if (p->data != NULL && p->helper == 0) {
            ci = p->data->char_index;
            if (p->team_side != 0 && p->team_side == g->arcade_side) {
                serial = g->teams[1].char_count + g->teams[0].char_count;
            } else {
                serial = 0;
            }
        }
        Player_Free(p);
        if (ci != -1) {
            PlayerCache_Release(ci, serial);
        }
        g = g_game;
        g->player_chars[i] = NULL;
        g->players[i] = NULL;
    }
}

/*
 * 0x00424020: the faces of the team members for [Turns Face] (spr
 * elements): sprite group / number of the face from every member's sprite
 * file, read with buffering off, and the member's select cell scale.
 * Returns 1.
 */
int Match_LoadTeamFaces(void)
{
    GameVars *g;
    int side;

    Players_ReleaseSlots();
    g = g_game;
    for (side = 0; side < 2; side++) {
        TeamSel *t = &g->teams[side];
        const SysElem *face = &g->fight.turns_teammate[side].face;
        int select[3];
        int m;

        if (t->member_count < 2 || g->survival_side == 2 - side || face->type != SYSELEM_SPRITE) {
            continue;
        }
        select[0] = face->group;
        select[1] = face->number;
        select[2] = -1;
        for (m = 0; m < t->member_count; m++) {
            CharInfo *info = &g->char_info[t->members[m].slots[0].char_index];
            char path[0x200];
            Ini *ini;

            Path_Join(info->def_file, info->dir, path);
            ini = Ini_Open(path);
            g = g_game;
            if (ini != NULL) {
                const char *spr;
                path[0] = '\0';
                if (Ini_SelectGroup(ini, "Files") != -1 && (spr = Ini_GetValue(ini, "sprite")) != NULL) {
                    Path_Join(spr, g_game->char_info[t->members[m].slots[0].char_index].dir, path);
                }
                Ini_Free(ini);
                if (path[0] != '\0') {
                    SpriteSet *set = NULL;
                    Stream *s;
                    g_bufferedRead = 0;
                    s = Stream_OpenFile(path, "rb");
                    if (s != NULL) {
                        set = Sff_LoadSelected(s, select);
                        Stream_Close(s);
                    }
                    g = g_game;
                    g->team_faces[side].sprites[m] = set;
                    g_bufferedRead = g->buffered_read;
                }
                g = g_game;
            }
            info = &g->char_info[t->members[m].slots[0].char_index];
            g->team_faces[side].scale[m] = g->select_info.cells[info->cell].scale;
        }
    }
    return 1;
}

/* 0x004242C0: frees the turns face sprites of both teams (Match_LoadTeamFaces). */
void Players_ReleaseSlots(void)
{
    GameVars *g = g_game;
    int side, m;

    for (side = 0; side < 2; side++) {
        for (m = 0; m < 4; m++) {
            SpriteSet *set = g->team_faces[side].sprites[m];
            if (set != NULL) {
                SpriteSet_Free(set);
                g->team_faces[side].sprites[m] = NULL;
            }
        }
    }
}

/* ------------------------------------------------------------------ rounds */

/*
 * 0x00424310: after a round of a team match: the round counter, the losing
 * teams bring their next member in (turns, the screen goes black meanwhile)
 * and the life of the members is reset; survival gets a new opponent team
 * and match RNG; then the round is reset and the frame timer restarted.
 */
void Match_NextRound(void)
{
    GameVars *g = g_game;
    int t;

    g->round_no++;
    for (t = 0; t < 2; t++) {
        TeamSel *ts;

        if (g->round_winner == t + 1) {
            continue;
        }
        ts = &g->teams[t];
        if (ts->cur_member + 1 < ts->member_count) {
            g_gfxClear(0);
            g = g_game;
            g->frame_skip_count = 0;
            g->skip_draw = 0;
            Timer_ResetFrame();
            Screen_Present();
            ts->cur_member++;
            Team_RotateMembers(t);
            Team_ReloadMember(t);
            g = g_game;
        }
        Team_ResetLife(&g->teams[t], 1);
    }
    if (g->survival_side != 0) {
        int s;
        Log_Printf("Match RNG closed\n");
        g = g_game;
        s = g->survival_side;
        g->match_rng_open = 0;
        Team_Randomize(&g->teams[2 - s], 2 - s, 0);
        Match_SeedRng(g_game->match_seed_arg);
    }
    Round_Reset();
    g = g_game;
    g->frame_skip_count = 0;
    g->skip_draw = 0;
    {
        __int64 now;
        if (g_timerPerformance) {
            LARGE_INTEGER c;
            QueryPerformanceCounter(&c);
            now = c.QuadPart - g_timerStart;
        } else {
            now = (__int64)timeGetTime() - g_timerStart;
        }
        g_timerResyncs++;
        g_timerFrame = 0;
        g_timerFrameStart = now;
    }
}

/*
 * 0x00424480: starts a match: the match RNG, persist vars reset (only the
 * computer's team in arcade), the team members loaded, the round rules of
 * each team (wins needed, draws, life multipliers between unequal teams,
 * survival's endless wins and life recovery), the players' power cleared,
 * the round reset, the win icons cleared, the camera at the stage start and
 * the BGM prebuffered.
 */
void Match_Start(void)
{
    GameVars *g;
    float r0, r1, q;
    int t, weak;

    Match_SeedRng(g_game->match_seed_arg);
    g = g_game;
    if (g->arcade_side != 0) {
        PersistVars_Reset(2 - g->arcade_side);
    } else {
        PersistVars_Reset(0);
        PersistVars_Reset(1);
    }
    Team_RotateMembers(0);
    Team_RotateMembers(1);
    Team_ReloadMember(0);
    Team_ReloadMember(1);
    g = g_game;
    for (t = 0; t < 2; t++) {
        TeamSel *ts = &g->teams[t];
        ts->life_mul = 1.0f;
        ts->recovery_base = 0;
        ts->recovery_rate = 0.0f;
        ts->recovery_weight = 0.0f;
        ts->life_reset = 1;
        ts->wins_needed = g->fight.round.match_wins;
        ts->draws_max = g->fight.round.match_maxdrawgames;
    }
    if (g->teams[0].member_count >= 2 || g->teams[1].member_count >= 2) {
        g->teams[0].recovery_rate = 275.0f;
        g->teams[0].recovery_base = 125;
        g->teams[0].life_reset = 0;
        g->teams[0].recovery_weight = 0.6f;
        if (g->teams[1].member_count >= 2) {
            g->teams[0].wins_needed = g->teams[1].member_count;
        }
        g->teams[0].draws_max = 0;
        g->teams[1].recovery_base = 125;
        g->teams[1].recovery_rate = 275.0f;
        g->teams[1].recovery_weight = 0.6f;
        g->teams[1].life_reset = 0;
        if (g->teams[0].member_count >= 2) {
            g->teams[1].wins_needed = g->teams[0].member_count;
        }
        g->teams[1].draws_max = 0;
    }
    /* the strength of each team: characters per needed win, simul teams scaled by Team.1VS2Life */
    r0 = (float)((double)g->teams[0].members[0].count / g->teams[0].wins_needed);
    if (g->teams[0].members[0].mode == TEAM_SIMUL) {
        r0 = (float)((double)g->team_1vs2_life * r0 / 200.0);
    }
    r1 = (float)((double)g->teams[1].members[0].count / g->teams[1].wins_needed);
    if (g->teams[1].members[0].mode == TEAM_SIMUL) {
        r1 = (float)((double)g->team_1vs2_life * r1 / 200.0);
    }
    q = (float)(r0 / r1);
    weak = 0;
    if (q < 1.0f) {
        weak = 1;
        q = (float)(1.0 / q);
    }
    g->teams[weak].life_mul = (float)(1.0 / q);
    t = weak;
    if (g->survival_side != 0) {
        int s = g->survival_side - 1;
        int n = g->teams[s].member_count;
        t = s;
        g->teams[s].wins_needed = 999999;
        if (n < 2) {
            g->teams[1 - s].wins_needed = 1;
        } else {
            float nf;
            g->teams[s].life_mul = (float)(1.0 - (double)(n - 1) * 0.1);
            nf = (float)n;
            g->teams[s].recovery_base = Ftol(1.0 / nf * g->teams[s].recovery_base);
            g->teams[s].recovery_rate = (float)(1.0 / (nf - 0.5) * g->teams[s].recovery_rate);
        }
    }
    if (g->quick_vs != 0) {
        g->teams[t].life_mul = 1.0f;
    }
    {
        int i;
        for (i = 1; i <= g->player_slots; i++) {
            Player *p = g->players[i];
            if (p != NULL) {
                p->powerbar_power = 0;
                p->power = 0;
                p->rounds_existed = 0;
            }
        }
    }
    Round_Reset();
    g = g_game;
    g->match_has_result = 0;
    g->intro_skipped = 0;
    g->round_no = 1;
    g->rounds_won[1] = 0;
    g->rounds_won[0] = 0;
    g->draws = 0;
    {
        int k;
        for (k = 0; k < 10; k++) {
            g->win_icons[1][k] = -1;
            g->win_icons[0][k] = -1;
            g->win_perfect[1][k] = -1;
            g->win_perfect[0][k] = -1;
        }
        for (k = 0; k < 6; k++) {
            g->envshake[k] = 0;
        }
    }
    g->hitdef_serial = 0;
    Bg_Reset(g->stage_bg);
    g->victory_quote[0] = '\0';
    for (t = 0; t < 2; t++) {
        TeamSel *ts = &g->teams[t];
        int m, k;
        for (m = 0; m < ts->member_count; m++) {
            for (k = 0; k < ts->members[m].count; k++) {
                /* the victory quote of the member: random */
                g->teams[t].member_data[m][k].victory_quote = -1;
            }
        }
    }
    {
        /* Camera_Set at the stage start, inlined */
        float x = (float)g->stage.camera_startx;
        float y = (float)g->stage.camera_starty;
        double hw;
        float e;
        g->camera_offset_x = x;
        g->camera_offset_y = y;
        g->camera_y_target = y;
        g->hits_enabled = 1;
        g->input_enabled = 1;
        hw = (double)Ftol(g->game_coord[0]) / g->camera_zoom * 0.5;
        e = (float)(x - hw);
        g->screen_left = e;
        g->screen_left_target = e;
        e = (float)(hw + x);
        g->screen_right = e;
        g->screen_right_target = e;
    }
    if ((g->demo_mode == 0 || g->demo_fight_playbgm == 1) && g_bgmActive) {
        pthread_mutex_lock((pthread_mutex_t *)&g_bgmMutex);
        Bgm_QueueCommand(g_bgmGetHandle(), NULL, BGM_CMD_PREBUFFER, 0);
        pthread_mutex_unlock((pthread_mutex_t *)&g_bgmMutex);
        g_bgmBackgroundChanged = 1;
    }
}

/*
 * 0x00424A20: ends a match: the victory quote of the winning member (its
 * chosen quote, else a random one of the character's) is kept for the
 * victory screen, the players are freed and the match RNG closed.
 *
 * Quirk kept: the random walk over the quotes starts at quote 1 and its
 * first step does not wrap, so from quote 99 it reads "quote 100", which is
 * CharData.quote_count (taken as a string pointer by strncpy).
 */
void Match_Unload(void)
{
    GameVars *g;

    Log_Printf("Finishing match\n");
    g = g_game;
    if (g->screen_kind == 4 && g->match_has_result != 0) {
        g->continued = 0;
        if (g->victory_quote[0] == '\0' && g->match_winner != 0) {
            TeamSel *ts = &g->teams[g->match_winner - 1];
            int m = g->winner_member;
            int ci = ts->members[m].slots[0].char_index;
            CharData *c;

            if (ci != -1 && (c = CharCache_Get(ci)) != NULL && c->quote_count > 0) {
                char *const *quotes = c->quotes;    /* Moemon Fight: pointers, not ints */
                int q = ts->member_data[m][0].victory_quote;
                if ((unsigned int)q > 99 || quotes[q] == 0 || q == -1) {
                    int n = Match_Random() % c->quote_count + 1;
                    q = 0;
                    do {
                        q = (q + 1) % 100;      /* Moemon Fight: wraps (the original read quote 100) */
                        while (quotes[q] == 0) {
                            q = (q + 1) % 100;
                        }
                        n--;
                    } while (n > 0);
                }
                strncpy(g->victory_quote, quotes[q], 0x3FF);
            }
        }
    }
    Players_Free();
    Log_Printf("Match RNG closed\n");
    g_game->match_rng_open = 0;
}

/*
 * 0x00424B60: the life of the current members of team ts: Life * the team's
 * life multiplier * [Options] Life / 100, rounded; it becomes their life
 * maximum, and their life (and the life bar) when it is lower or with
 * `force`.
 */
void Team_ResetLife(TeamSel *ts, int force)
{
    TeamMember *m = &ts->members[ts->cur_member];
    int k;

    for (k = 0; k < m->count; k++) {
        GameVars *g = g_game;
        Player *p = g->players[m->slots[k].player];
        int life = Ftol((double)p->consts.life * ts->life_mul * g->life / 100.0 + 0.5);

        p->life_max = life;
        if (p->life > life || force) {
            p->lifebar_red = life;
            p->lifebar_life = life;
            p->life = life;
        }
    }
}

/* Empties a KeyedArray with its lookup cache (inlined by Round_Reset, as Screen_Run does). */
static void Ml_ClearKeyedArray(KeyedArray *a)
{
    int i;

    memset(a->slots, 0, a->capacity << 4);
    a->count = 0;
    a->last = 0;
    a->first = 0;
    a->iter = 0;
    for (i = 0; i < a->cache_size; i++) {
        a->cache[i].key1 = 0x7FFFFFFF;
        a->cache[i].key2 = (int)0x80000000;
        a->cache[i].index = -1;
    }
    a->cache_pos = 0;
}

/*
 * 0x00424BF0: resets the round: sounds stopped, round state and timer, the
 * camera, every character reset to state 5900 (helpers leave the match)
 * keeping its power, AI and (when life carries over) life, the team life,
 * the start positions, the command-line overrides of players 1-4, the
 * one-shot animations, the global PalFX (black until the next frame),
 * AssertSpecial flags, the stage BG (resetbg), BGPalFX and AllPalFX,
 * afterimages, explods, pauses, EnvShake and combo counters.
 */
void Round_Reset(void)
{
    GameVars *g;
    TeamMember *members = g_game->match_members;
    int i, t;

    Log_Printf("Resetting round\n");
    if (g_sndActive == 1) {
        Mix_HaltChannel(-1);
    }
    g = g_game;
    g->round_not_counted = 0;
    g->over_skipped = 0;
    g->over_end_time = g->fight.round.over_time;
    g->slow_time = 0;
    g->slow_counter = 0;
    g->slow_step = 0;
    g->intro_time = 0;
    g->ko_state_time = 0;
    g->round_state = 0;
    g->round_winner = 0;
    g->win_type = 0;
    g->round_state_time = 0;
    g->camera_mode = 0;
    g->round_time = (g->time + 1) * g->fight.time.frames_per_count - 1;
    g->round_ticks = 0;
    if (g->team_variant == 3 || g->demo_mode != 0 || g->survival_side != 0) {
        g->round_time = -1;
    }
    Camera_Reset();
    g->zoom_active = 0.0f;
    g->zoom_x = 0.0f;
    g->shake_x = 0.0f;
    g->zoom_y = 0.0f;
    g->shake_y = 0.0f;
    g->zoom_scale = 1.0f;
    g->stage_scale = 1.0f;
    g->zoom_left = g->screen_left;
    g->zoom_right = g->screen_right;
    for (i = 1; i <= g->player_slots; i++) {
        Player *p = g->players[i];
        int power, ai, life;

        if (p == NULL) {
            continue;
        }
        if (p->helper != 0) {
            if (p->active) {
                Players_SetActive(p->index, 0);
            }
            continue;
        }
        power = p->power;
        ai = p->ai;
        life = p->life;
        Player_Reset(p);
        Ml_ChangeState(p, 5900);
        Input_Resync(&p->input, p->input_keys);
        g = g_game;
        p->ai = ai;
        p->powerbar_power = power;
        p->power = power;
        if (g->demo_mode != 0) {
            int v = *p->power_max_ref;
            p->powerbar_power = v;
            p->powerbar_level = v;
            p->power = v;
        }
        /* team_index may be -1: the original reads before teams[0] then */
        if (*(int *)((char *)g + offsetof(GameVars, teams) + p->team_index * (int)sizeof(TeamSel) +
                     offsetof(TeamSel, life_reset)) == 0 &&
            g->round_reset_pending == 0) {
            p->life = life;
        }
        p->lifebar_red = p->life;
        p->lifebar_life = p->life;
    }
    for (t = 0; t < 2; t++) {
        Team_ResetLife(&g->teams[t], 0);
    }
    Round_PlacePlayers();
    for (t = 0; t < 2; t++) {
        TeamMember *m = &members[t];
        TeamSel *ts = &g->teams[t];
        int k;
        ts->life_ratio = 0.0f;
        for (k = 0; k < m->count; k++) {
            Player *p = g->players[m->slots[k].player];
            if (p != NULL) {
                ts->life_ratio = (float)((double)p->life / p->life_max + ts->life_ratio);
            }
        }
        if (m->count > 0) {
            ts->life_ratio = (float)(ts->life_ratio / (double)m->count);
        }
    }
    for (i = 1; i <= g->player_slots; i++) {
        if (g->players[i] != NULL) {
            g->players[i]->ctrl = 0;
        }
    }
    for (i = 0; i < 4; i++) {
        Player *p = g->players[i + 1];
        int v;
        if (p == NULL) {
            continue;
        }
        if ((v = g->override_ai[i]) != -1) {
            p->ai = v;
        }
        if ((v = g->override_life[i]) != -1) {
            p->lifebar_red = v;
            p->lifebar_life = v;
            p->life = v;
            p->life_max = v;
        }
        if ((v = g->override_power[i]) != -1) {
            *p->power_ref = v;
            p->powerbar_power = v;
            p->powerbar_level = v;
        }
    }
    Ml_ClearKeyedArray(g->anims->list);
    g_palFx.serial = g_palFxSerial;
    g_palFx.mul[0] = 0;
    g_palFx.mul[1] = 0;
    g_palFx.mul[2] = 0;
    g_palFxSerial++;
    memset(&g->assert_intro, 0, 11);
    if (g->stage.resetbg != 0) {
        Bg_Reset(g->stage_bg);
    }
    {
        PalFxDef *def = &g->bg_palfx_def;
        memset(def, 0, sizeof(PalFxDef));
        def->mul[2] = 0x100;
        def->mul[1] = 0x100;
        def->mul[0] = 0x100;
        def->color = 0x100;
        def->reset = 1;
        def = &g->all_palfx_def;
        memset(def, 0, sizeof(PalFxDef));
        def->mul[2] = 0x100;
        def->mul[1] = 0x100;
        def->mul[0] = 0x100;
        def->color = 0x100;
        def->reset = 1;
    }
    PalGroups_ResetAll();
    {
        KeyedArray *list = g->afterimages;
        int k;
        for (k = 0; k < list->capacity; k++) {
            int slot = KeyedArray_Find(list, 0, k);
            AfterImage *ai = slot != -1 ? (AfterImage *)(list->data + list->elem_size * slot) : NULL;
            /* a missing buffer is not checked for (the original crashes) */
            ai->anim = NULL;
            ai->running = 0;
        }
    }
    SysDraw_Clear(g->explods);
    SysDraw_Clear(g_game->sysexplods);
    g = g_game;
    g->pause_kind = -1;
    g->pauses[1][6] = 0;
    g->pauses[0][6] = 0;
    for (i = 0; i < 6; i++) {
        g->envshake[i] = 0;
    }
    memset(g->combo, 0, sizeof(g->combo));
    g->defocus_paused = 0;
}

/*
 * 0x004252D0: players 1-4 stand still at the stage start positions, turned
 * to the stage facing (with the input flipped to match: by the side of the
 * enemy with [Input] InputDirMode 1, else by the facing).
 */
void Round_PlacePlayers(void)
{
    GameVars *g = g_game;
    double k;
    int i;

    for (i = 1; i <= 4; i++) {
        Player *p = g->players[i];
        Coord v;
        if (p == NULL) {
            continue;
        }
        /* the original copies pad_24 from its uninitialised stack */
        v.x = 0.0;
        v.y = 0.0;
        v.z = 0.0;
        v.unit = 1.0;
        v.type = COORD_TYPE_2D;
        v.pad_24 = 0;
        p->vel = v;
    }
    k = g_gameCoord[0];
    for (i = 0; i < 4; i++) {
        Player *p = g->players[i + 1];
        int facing = i == 0 ? g->stage.startz[4] : g->stage.facing[i - 1];

        if (p == NULL) {
            continue;
        }
        if (facing != p->facing) {
            int flip;
            p->facing = -p->facing;
            if (g->input_dir_mode == 1 && p->enemy != NULL) {
                flip = !(p->pos.x <= p->enemy->pos.x);
            } else {
                flip = p->facing == -1;
            }
            Input_SetFlipped(&p->input, flip);
            g = g_game;
        }
        p->pos.x = (double)g->stage.startx[i] * (k / g->stage.unit);
        p->pos.y = (double)g->stage.starty[i] * (k / g->stage.unit);
    }
}

/*
 * 0x00425480: whether the match is over once the round is (state 3 past
 * [Round] over.hittime, or later states): 0 not over, 1 team 1 won, 2 team
 * 2 won; when both teams reach their wins, 3 - arcade side (the computer's
 * team) in arcade, else 1.
 */
int Match_CheckOver(void)
{
    GameVars *g = g_game;
    int state = g->round_state;
    int r = 0, w;

    if (state == 3) {
        if (g->round_state_time < g->fight.round.over_hittime) {
            return 0;
        }
    } else if (state < 3) {
        return 0;
    }
    w = g->rounds_won[0];
    if (g->round_not_counted == 0) {
        int s = g->round_winner;
        if (s == 1 || (s == 0 && g->draws + 1 > g->teams[0].draws_max)) {
            w++;
        }
    }
    if (w >= g->teams[0].wins_needed) {
        r = 1;
    }
    w = g->rounds_won[1];
    if (g->round_not_counted == 0) {
        int s = g->round_winner;
        if (s == 2 || (s == 0 && g->draws + 1 > g->teams[1].draws_max)) {
            w++;
        }
    }
    if (w < g->teams[1].wins_needed) {
        return r;
    }
    if (r == 0) {
        return 2;
    }
    if (g->arcade_side != 0) {
        return 3 - g->arcade_side;
    }
    return 1;
}

/*
 * 0x00425540: opens the match RNG with `seed`, or with the next session RNG
 * value (+ 1, modulo its modulus) for 0; the seed is logged and returned,
 * the RNG starts from it (1 when it is 0 or >= 0x7FFFFFFF).
 */
unsigned int Match_SeedRng(unsigned int seed)
{
    GameVars *g = g_game;
    unsigned int s;

    if (seed == 0) {
        Ml_SessionRandom(g);
        seed = ((unsigned int)g->session_rng + 1) % g->session_rng_mod;
    } else {
        Ml_SessionRandom(g);
    }
    s = seed;
    if (s == 0 || s >= 0x7FFFFFFF) {
        s = 1;
    }
    g->match_rng_seed_hi = 0;
    g->match_rng = s;
    g->match_rng_seed = s;
    g->match_rng_mul = 16807;
    g->match_rng_mod = 0x7FFFFFFF;
    Log_Printf("Match RNG seed: %u\n", seed);
    g_game->match_rng_open = 1;
    return seed;
}

/* ------------------------------------------------------------------ shadows */

/*
 * 0x00425680: the reflection and the shadow of the current frame of st,
 * drawn with dp at ground level y (+ offset), the offsets scaled by sx, sy.
 * shadow is Stage.shadow (intensity, colour, yscale, xshear, fade.range),
 * reflection points at the reflection intensity. The reflection is drawn
 * mirrored and additively with alpha intensity / 256; the shadow is the
 * sprite flattened by yscale, sheared, through a black PalFX, with alpha
 * from the intensity and the sprite's own alpha, fading out between the
 * fade range heights (none above its start). Nothing without [Config]
 * DrawShadows, a frame or a sprite.
 */
void Shadow_Draw(AnimState *st, const DrawParams *dp, float y, float offset, float sx, float sy,
                 const int *shadow, const int *reflection)
{
    GameVars *g = g_game;
    SpriteSet *sprites = st->sprites;
    float xshear, top, ty, s;
    DrawParams a, refl, shad;
    const DrawParams *res;
    int sprite, vdir, hdir, intensity, lo, hi;

    xshear = *(const float *)&shadow[3];
    if (g->draw_shadows == 0 || st->elem == NULL) {
        return;
    }
    sprite = st->sprite;
    if (sprite == -1) {
        return;
    }
    a = st->dp;
    DrawParams_ScalePos(dp, &a);
    a.x = a.x * sx;
    a.y = a.y * sy;
    res = DrawParams_Combine(dp, &a);
    shad = *res;
    refl = *res;
    {
        float cam = (float)(g_gameCoord[0] / g->stage.unit * g->camera_y + (double)g->camera_zoom * g->camera_offset_y);
        top = (float)((double)g->draw_offset_y - cam + offset);
    }
    if (g->zoom_active != 0.0f) {
        top = (float)(top - (double)g->shake_y * g->stage_scale);
    }
    vdir = (a.flip2 & 2) == 0 ? 1 : -1;
    ty = (float)(top - (double)a.y);
    intensity = *reflection;
    refl.y = (float)((double)ty + top);
    refl.scale2_y = (float)(refl.scale2_y * -1.0);
    shad.x = (float)(a.x - (double)vdir * xshear * ty);
    shad.y = (float)((double)ty * *(const float *)&shadow[2] + top);
    shad.scale2_y = (float)(-(double)*(const float *)&shadow[2] * shad.scale2_y);
    if (intensity > 0) {
        float as;
        refl.flags |= 0x100;
        refl.blend = 1;
        refl.alpha_src = (float)(intensity * 0.00390625);
        as = refl.alpha_src;
        refl.alpha_dst = (float)(1.0 - as);
        if (a.blend == 1) {
            float bd;
            refl.alpha_src = (float)(as * (double)a.alpha_src);
            bd = refl.alpha_dst;
            refl.alpha_dst = (float)(bd + (1.0 - bd) * a.alpha_dst);
        }
        Sprite_Draw(sprites, sprite, &refl);
    }
    lo = shadow[4];
    hi = shadow[5];
    s = (float)(1.0 - (1.0 - a.alpha_dst) * (shadow[0] * 0.00390625));
    if (lo < hi) {
        if ((double)lo >= y) {
            return;
        }
        if ((double)hi > y) {
            s = (float)(1.0 - (1.0 - s) * ((double)y - lo) / (hi - lo));
        }
    }
    if (!(s < 1.0f)) {
        return;
    }
    if (g_shadowPalFxReady == 0) {
        PalFx_Init(&g_shadowPalFx, NULL);
        g_shadowPalFx.serial = g_palFxSerial;
        g_shadowPalFx.mul[0] = 0;
        g_shadowPalFx.mul[1] = 0;
        g_shadowPalFx.mul[2] = 0;
        g_palFxSerial++;
    }
    shad.flags |= 0x100;
    hdir = (shad.flip2 & 1) == 0 ? 1 : -1;
    shad.alpha_src = (float)(1.0 - s);
    shad.alpha_dst = s;
    g_palFxCur = &g_shadowPalFx;
    shad.blend = 1;
    shad.shear_x = (float)(hdir * (double)xshear);
    Sprite_Draw(sprites, sprite, &shad);
    g_palFxCur = NULL;
}

/* 0x00425630: the next value of the match RNG (Park-Miller, the low 32 bits of the state). */
int Match_Random(void)
{
    GameVars *g = g_game;

    g->match_rng = g->match_rng * g->match_rng_mul % g->match_rng_mod;
    return (int)(unsigned int)g->match_rng;
}
