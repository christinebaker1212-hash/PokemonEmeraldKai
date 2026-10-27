/*
 * The motif (original module 0x45C4D0-0x45DE7F; string literals from
 * "  Load system file %s..." to "sprite data..."): System_LoadMotif reads
 * the motif's system.def (sprites, sounds, fonts, animations, music, every
 * screen layout and background) and then its fight.def; the backgrounds and
 * the sprites can be unloaded during matches ([Misc] UnloadSystem) and
 * reloaded afterwards.
 *
 * Quirks kept from the original:
 * - Errors after system.def is parsed return without freeing the Ini, and
 *   most of them (missing files, a missing "fight" or "select" entry, a
 *   failed [Select Info]) without a message.
 * - [Title Info] fadein.time/fadeout.time and the [Demo Mode], [Continue
 *   Screen]... values have no default: what was there is kept (0 at start-up).
 * - A missing VictoryBG only logs a line and clears the library error text.
 * - System_FreeMotif frees the select cells and the animations without
 *   clearing their pointers; System_ReloadMotif does not check VictoryBG to
 *   decide whether the backgrounds need reloading.
 */
#include <SDL_ttf.h>
#include "mugen.h"

/* [Title Info] menu.itemname.<key> and the default names, by TITLE_ITEM_*. */
static const char *const title_item_keys[TITLE_ITEM_COUNT] = {
    "arcade", "versus", "teamarcade", "teamversus", "teamcoop", "survival",
    "survivalcoop", "training", "watch", "options", "exit",
};
static const char *const title_item_defaults[TITLE_ITEM_COUNT] = {
    "Arcade", "VS Mode", "Team Arcade", "Team Versus", "Team Co-op", "Survival",
    "Survival Co-op", "Training", "Watch", "Options", "Exit",
};

/* [Title Info] */
static void load_title_info(Ini *ini, TitleInfo *ti)
{
    char key[0x100];
    int i;

    ti->pos_x = 159;
    ti->pos_y = 158;
    ti->item_spacing[1] = 13;
    ti->window_margins_y[0] = 12;
    ti->window_margins_y[1] = 8;
    ti->window_visible_items = 5;
    ti->boxcursor_visible = 1;
    ti->boxcursor_coords[0] = -32;
    ti->boxcursor_coords[1] = -10;
    ti->boxcursor_coords[2] = 32;
    ti->boxcursor_coords[3] = 2;
    Ini_GetInt(ini, "fadein.time", &ti->fadein_time);
    Ini_GetInt(ini, "fadeout.time", &ti->fadeout_time);
    if (ti->fadein_time < 1) {
        ti->fadein_time = 1;
    }
    if (ti->fadeout_time < 1) {
        ti->fadeout_time = 1;
    }
    Ini_Scanf(ini, "menu.pos", "%i,%i", &ti->pos_x, &ti->pos_y);
    FontSpec_Read(&ti->item_font, ini, "menu.item.font");
    FontSpec_Read(&ti->item_active_font, ini, "menu.item.active.font");
    Ini_Scanf(ini, "menu.item.spacing", "%i,%i", &ti->item_spacing[0], &ti->item_spacing[1]);
    for (i = 0; i < TITLE_ITEM_COUNT; i++) {
        if (title_item_keys[i][0] == '\0') {
            continue;
        }
        sprintf(key, "menu.itemname.%s", title_item_keys[i]);
        if (Ini_GetValue(ini, key) != NULL) {
            Ini_ReadString(ini, key, sizeof(ti->item_names[i]) - 1, ti->item_names[i]);
        } else {
            strcpy(ti->item_names[i], title_item_defaults[i]);
        }
    }
    Ini_Scanf(ini, "menu.window.margins.y", "%i,%i", &ti->window_margins_y[0], &ti->window_margins_y[1]);
    Ini_GetInt(ini, "menu.window.visibleitems", &ti->window_visible_items);
    Ini_GetInt(ini, "menu.boxcursor.visible", &ti->boxcursor_visible);
    Ini_Scanf(ini, "menu.boxcursor.coords", "%i,%i,%i,%i", &ti->boxcursor_coords[0], &ti->boxcursor_coords[1],
              &ti->boxcursor_coords[2], &ti->boxcursor_coords[3]);
    Sound_ReadRef(&ti->move_snd[0], &ti->move_snd[1], ini, "cursor.move.snd");
    Sound_ReadRef(&ti->done_snd[0], &ti->done_snd[1], ini, "cursor.done.snd");
    Sound_ReadRef(&ti->cancel_snd[0], &ti->cancel_snd[1], ini, "cancel.snd");
}

/* [VS Screen] */
static void load_vs_screen(Ini *ini, VsScreenInfo *vs)
{
    char key[0x100];
    int i;

    vs->time = 150;
    Ini_GetInt(ini, "time", &vs->time);
    Ini_GetInt(ini, "fadein.time", &vs->fadein_time);
    Ini_GetInt(ini, "fadeout.time", &vs->fadeout_time);
    for (i = 1; i <= 2; i++) {
        VsSideLayout *side = &vs->sides[i - 1];

        sprintf(key, "p%i", i);
        SysElem_Read(&side->face, key, ini);
        if (side->face.type == SYSELEM_NONE) {
            /* the big portrait of the character by default */
            side->face.scale_y = 1.0f;
            side->face.scale_x = 1.0f;
            side->face.type = SYSELEM_SPRITE;
            side->face.group = 9000;
            side->face.number = 1;
            side->face.facing = i == 1 ? 1 : -1;
            sprintf(key, "p%i.scale", i);
            Ini_Scanf(ini, key, "%f,%f", &side->face.scale_x, &side->face.scale_y);
            sprintf(key, "p%i.pos", i);
            Ini_Scanf(ini, key, "%f,%f", &side->face.x, &side->face.y);
        }
        sprintf(key, "p%i.name.offset", i);
        Ini_Scanf(ini, key, "%i,%i", &side->name_pos[0], &side->name_pos[1]);
        sprintf(key, "p%i.name.pos", i);
        Ini_Scanf(ini, key, "%i,%i", &side->name_pos[0], &side->name_pos[1]);
        sprintf(key, "p%i.name.font", i);
        FontSpec_Read(&side->name_font, ini, key);
        sprintf(key, "p%i.name.spacing", i);
        Ini_Scanf(ini, key, "%i,%i", &side->name_spacing[0], &side->name_spacing[1]);
    }
    SysElem_Read(&vs->match, "match", ini);
}

/* [Victory Screen] */
static void load_victory_screen(Ini *ini, VictoryInfo *vi)
{
    char key[0x100];

    vi->time = 150;
    Ini_GetInt(ini, "enabled", &vi->enabled);
    Ini_GetInt(ini, "time", &vi->time);
    Ini_GetInt(ini, "fadein.time", &vi->fadein_time);
    Ini_GetInt(ini, "fadeout.time", &vi->fadeout_time);
    sprintf(key, "p%i", 1);
    SysElem_Read(&vi->face, key, ini);
    sprintf(key, "p%i.name.offset", 1);
    Ini_Scanf(ini, key, "%i,%i", &vi->name_offset[0], &vi->name_offset[1]);
    sprintf(key, "p%i.name.font", 1);
    FontSpec_Read(&vi->name_font, ini, key);
    sprintf(key, "p%i.name.spacing", 1);
    Ini_Scanf(ini, key, "%i,%i", &vi->name_spacing[0], &vi->name_spacing[1]);
    SysElem_Read(&vi->winquote, "winquote", ini);
}

/* [Demo Mode] */
static void load_demo_mode(Ini *ini, GameVars *g)
{
    int enabled = 0;

    Ini_GetInt(ini, "enabled", &enabled);
    if (enabled != 0) {
        Ini_GetInt(ini, "title.waittime", &g->demo_wait);
    }
    Ini_GetInt(ini, "select.enabled", &g->demo_select_enabled);
    Ini_GetInt(ini, "vsscreen.enabled", &g->demo_vsscreen_enabled);
    Ini_GetInt(ini, "fight.endtime", &g->demo_fight_endtime);
    Ini_GetInt(ini, "fight.playbgm", &g->demo_fight_playbgm);
    Ini_GetInt(ini, "fight.stopbgm", &g->demo_fight_stopbgm);
    Ini_GetInt(ini, "fight.bars.display", &g->demo_fight_bars_display);
    Ini_GetInt(ini, "intro.waitcycles", &g->demo_intro_waitcycles);
    Ini_GetInt(ini, "debuginfo", &g->demo_debuginfo);
}

/* [Continue Screen] */
static void load_continue_screen(Ini *ini, ContinueInfo *cs)
{
    Ini_GetInt(ini, "enabled", &cs->enabled);
    Ini_GetString(ini, "storyboard", cs->storyboard, sizeof(cs->storyboard));
    cs->pos[0] = 159;
    cs->pos[1] = 40;
    Ini_Scanf(ini, "pos", "%i,%i", &cs->pos[0], &cs->pos[1]);
    SysElem_Read(&cs->continue_text, "continue", ini);
    SysElem_Read(&cs->yes, "yes", ini);
    SysElem_Read(&cs->no, "no", ini);
    SysElem_Read(&cs->yes_active, "yes.active", ini);
    SysElem_Read(&cs->no_active, "no.active", ini);
}

/* "enabled" and "storyboard" of the selected group. */
static void load_storyboard(Ini *ini, MotifStoryboard *sb)
{
    Ini_GetInt(ini, "enabled", &sb->enabled);
    Ini_GetString(ini, "storyboard", sb->storyboard, sizeof(sb->storyboard));
}

/*
 * 0x0045C4D0: loads GameVars.motif_dir + motif_file (system.def) and the
 * fight.def it names. Returns 0 on errors.
 */
int System_LoadMotif(void)
{
    char path[0x200];
    char fight[0x200];
    const char *dirs[5];
    GameVars *g = g_game;
    Stream *s;
    Ini *ini;
    const char *value;
    const char *line;
    int w, h;

    Log_Printf("  Load system file %s...", g->motif_file);
    Path_Join(g->motif_file, g->motif_dir, path);
    s = Stream_OpenFile(path, "rt");
    ini = NULL;
    if (s != NULL) {
        ini = Ini_Parse(s, INI_MODE_READ);
        Stream_Close(s);
    }
    if (ini == NULL) {
        Error_Printf("Can't open %s%s\n", g->motif_dir, g->motif_file);
        return 0;
    }
    Log_Printf("OK\n");

    if (Ini_SelectGroup(ini, "Info") != -1) {
        w = 320;
        h = 240;
        Ini_Scanf(ini, "localcoord", "%i,%i", &w, &h);
        if (w < 1) {
            w = 1;
        }
        if (h < 1) {
            h = 1;
        }
        g->motif_coord[0] = w;
        g->motif_coord[1] = h;
        g->motif_coord[2] = (w - 1) / 2;
        g->motif_coord[3] = (h - 1) / 2;
        g->motif_scale = g->game_coord[0] / g->motif_coord[0];
    }

    /* [Files] */
    if (Ini_SelectGroup(ini, "Files") == -1) {
        Error_Printf("Can't access [Files]\n");
        return 0;
    }
    Log_Printf("  Load system spr...");
    value = Ini_GetValue(ini, "spr");
    if (value == NULL) {
        Error_Printf("Missing spr specification in [Files]\n");
        return 0;
    }
    if (!File_Find(value, path, NULL, NULL)) {
        return 0;
    }
    g->motif_sprites = Sff_Load(path);
    if (g->motif_sprites == NULL) {
        Error_Printf("Can't load %s\n", value);
        return 0;
    }
    strcpy(g->motif_sff_path, path);
    Log_Printf("OK\n");
    if (g_sndActive == 1) {
        Log_Printf("  Load system snd...");
        value = Ini_GetValue(ini, "snd");
        if (value != NULL) {
            if (!File_Find(value, path, NULL, NULL)) {
                return 0;
            }
            g->motif_snd = Snd_Load(path);
            if (g->motif_snd == NULL) {
                Error_Printf("Can't load %s\n", value);
                return 0;
            }
            strcpy(g->motif_snd_path, path);
        }
        Log_Printf("OK\n");
    } else {
        g->motif_snd = NULL;
    }
    value = Ini_GetValue(ini, "fight");
    if (value == NULL) {
        return 0;
    }
    strcpy(fight, value);
    if (Ini_GetString(ini, "select", g->select_info.def_file, sizeof(g->select_info.def_file)) < 1 ||
        g->select_info.def_file[0] == '\0') {
        return 0;
    }
    Ini_GetString(ini, "logo.storyboard", g->logo_storyboard, sizeof(g->logo_storyboard));
    Ini_GetString(ini, "intro.storyboard", g->intro_storyboard, sizeof(g->intro_storyboard));
    Log_Printf("  Load system fonts...");
    dirs[0] = g->motif_dir;
    dirs[1] = "./";
    dirs[2] = "data/";
    dirs[3] = "font/";
    dirs[4] = NULL;
    Fonts_Load(ini, (Font **)g->motif_fonts, dirs);
    Log_Printf("OK\n");

    /* the animations: the [Begin Action n] groups, if any */
    ini->group = -1;
    line = Ini_NextGroup(ini);
    while (!Str_StartsWithI(line + 1, "begin action ")) {
        line = Ini_NextGroup(ini);
        if (line == NULL) {
            break;
        }
    }
    if (line != NULL) {
        Log_Printf("  Load system anim...");
        g->motif_anims = Anim_Parse(ini);
        if (g->motif_anims == NULL) {
            Error_Printf("Error loading anim\n");
            return 0;
        }
        Log_Printf("OK\n");
    }

    if (Ini_SelectGroup(ini, "Music") != -1) {
        Bgm_ReadInfo(ini, "title.bgm", &g->title_bgm);
        Bgm_ReadInfo(ini, "select.bgm", &g->select_bgm);
        Bgm_ReadInfo(ini, "vs.bgm", &g->vs_bgm);
        Bgm_ReadInfo(ini, "victory.bgm", &g->victory_bgm);
    }
    if (Ini_SelectGroup(ini, "Infobox") != -1) {
        FontSpec_Read(&g->infobox_font, ini, "text.font");
    }
    if (Ini_SelectGroup(ini, "Infobox Text") != -1) {
        /* the lines of the group, a lone "_" standing for an empty line */
        g->infobox_text[0] = '\0';
        for (line = Ini_FirstLine(ini); line != NULL; line = Ini_NextLine(ini)) {
            if (line[0] != '_' || line[1] != '\0') {
                Str_Append(g->infobox_text, line, sizeof(g->infobox_text));
            }
            Str_Append(g->infobox_text, "\n", sizeof(g->infobox_text));
        }
    }

    Log_Printf("  Load [Title Info]...");
    if (Ini_SelectGroup(ini, "title info") == -1) {
        Error_Printf("Can't access [Title Info]\n");
        return 0;
    }
    load_title_info(ini, &g->title_info);
    Log_Printf("OK\n");

    Log_Printf("  Load [Option Info]...");
    if (Ini_SelectGroup(ini, "option info") == -1) {
        Error_Printf("Can't access [Option Info]\n");
        return 0;
    }
    g->option_info.fadein_time = 10;
    g->option_info.fadeout_time = 10;
    Ini_GetInt(ini, "fadein.time", &g->option_info.fadein_time);
    Ini_GetInt(ini, "fadeout.time", &g->option_info.fadeout_time);
    Sound_ReadRef(&g->option_info.move_snd[0], &g->option_info.move_snd[1], ini, "cursor.move.snd");
    Sound_ReadRef(&g->option_info.done_snd[0], &g->option_info.done_snd[1], ini, "cursor.done.snd");
    Sound_ReadRef(&g->option_info.cancel_snd[0], &g->option_info.cancel_snd[1], ini, "cancel.snd");
    SysElem_Read(&g->option_info.title, "title", ini);
    Log_Printf("OK\n");

    Log_Printf("  Load [Select Info]...");
    if (Ini_SelectGroup(ini, "select info") == -1) {
        Error_Printf("Can't access [Select Info]\n");
        return 0;
    }
    if (!Select_LoadLayout(ini)) {
        return 0;
    }
    Log_Printf("OK\n");

    Log_Printf("  Load [VS Screen]...");
    if (Ini_SelectGroup(ini, "vs screen") == -1) {
        Error_Printf("Can't access [VS Screen]\n");
        return 0;
    }
    load_vs_screen(ini, &g->vs_screen);
    Log_Printf("OK\n");

    Log_Printf("  Load [Victory Screen]...");
    if (Ini_SelectGroup(ini, "victory screen") != -1) {
        load_victory_screen(ini, &g->victory_screen);
        Log_Printf("OK\n");
    } else {
        Log_Printf("Couldn't load\n");
    }

    g->demo_wait = -1;
    if (Ini_SelectGroup(ini, "demo mode") != -1) {
        Log_Printf("  Load [Demo Mode]...");
        load_demo_mode(ini, g);
        Log_Printf("OK\n");
    }
    if (Ini_SelectGroup(ini, "continue screen") != -1) {
        Log_Printf("  Load [Continue Screen]...");
        load_continue_screen(ini, &g->continue_screen);
        Log_Printf("OK\n");
    }
    if (Ini_SelectGroup(ini, "game over screen") != -1) {
        Log_Printf("  Load [Game Over Screen]...");
        load_storyboard(ini, &g->game_over);
        Log_Printf("OK\n");
    }
    if (Ini_SelectGroup(ini, "win screen") != -1) {
        Log_Printf("  Load [Win Screen]...");
        Ini_GetInt(ini, "enabled", &g->win_screen.enabled);
        SysElem_Read(&g->win_screen.wintext, "wintext", ini);
        Ini_GetInt(ini, "fadein.time", &g->win_screen.fadein_time);
        Ini_GetInt(ini, "pose.time", &g->win_screen.pose_time);
        Ini_GetInt(ini, "fadeout.time", &g->win_screen.fadeout_time);
        Log_Printf("OK\n");
    }
    if (Ini_SelectGroup(ini, "survival results screen") != -1) {
        Log_Printf("  Load [Survival Results Screen]...");
        Ini_GetInt(ini, "enabled", &g->survival_results.enabled);
        SysElem_Read(&g->survival_results.winstext, "winstext", ini);
        Ini_GetInt(ini, "fadein.time", &g->survival_results.fadein_time);
        Ini_GetInt(ini, "show.time", &g->survival_results.show_time);
        Ini_GetInt(ini, "fadeout.time", &g->survival_results.fadeout_time);
        Ini_GetInt(ini, "roundstowin", &g->survival_results.roundstowin);
        Log_Printf("OK\n");
    }
    if (Ini_SelectGroup(ini, "default ending") != -1) {
        Log_Printf("  Load [Default Ending]...");
        load_storyboard(ini, &g->default_ending);
        Log_Printf("OK\n");
    }
    if (Ini_SelectGroup(ini, "end credits") != -1) {
        Log_Printf("  Load [End Credits]...");
        load_storyboard(ini, &g->end_credits);
        Log_Printf("OK\n");
    }

    /* backgrounds */
    Log_Printf("  Load TitleBG...");
    g->title_bg = Bg_Load("TitleBG", ini, g->motif_dir, g->motif_sprites, g->motif_anims);
    if (g->title_bg == NULL) {
        Error_Printf("Error loading TitleBG\n");
        return 0;
    }
    Log_Printf("VersusBG...");
    g->versus_bg = Bg_Load("VersusBG", ini, g->motif_dir, g->motif_sprites, g->motif_anims);
    if (g->versus_bg == NULL) {
        Error_Printf("Error loading VersusBG\n");
        return 0;
    }
    Log_Printf("VictoryBG...");
    g->victory_bg = Bg_Load("VictoryBG", ini, g->motif_dir, g->motif_sprites, g->motif_anims);
    if (g->victory_bg == NULL) {
        Log_Printf("Error loading VictoryBG\n");
        g_libraryErrorText[0] = '\0';
    }
    Log_Printf("SelectBG...");
    g->select_bg = Bg_Load("SelectBG", ini, g->motif_dir, g->motif_sprites, g->motif_anims);
    if (g->select_bg == NULL) {
        Error_Printf("Error loading SelectBG\n");
        return 0;
    }
    Log_Printf("OptionBG...");
    g->option_bg = Bg_Load("OptionBG", ini, g->motif_dir, g->motif_sprites, g->motif_anims);
    if (g->option_bg == NULL) {
        Error_Printf("Error loading OptionBG\n");
        return 0;
    }
    Log_Printf("OK\n");
    Ini_Free(ini);
    g->motif_loaded = 1;

    Log_Printf("  Loading fight data\n");
    if (!File_Find(fight, path, NULL, NULL)) {
        return 0;
    }
    if (!Fight_Load(path)) {
        Error_Printf("Error loading fight data: %s\n", fight);
        return 0;
    }
    Log_Printf("  Fight data loaded OK\n");
    return 1;
}

/* 0x0045D8F0: frees everything System_LoadMotif loaded (not fight.def). */
void System_FreeMotif(void)
{
    GameVars *g = g_game;
    int i;

    Bg_Free(g->option_bg);
    g->option_bg = NULL;
    Bg_Free(g->select_bg);
    g->select_bg = NULL;
    Bg_Free(g->victory_bg);
    g->victory_bg = NULL;
    Bg_Free(g->versus_bg);
    g->versus_bg = NULL;
    Bg_Free(g->title_bg);
    g->title_bg = NULL;
    if (g->select_info.cells != NULL) {
        for (i = 0; i < g->select_info.rows * g->select_info.columns; i++) {
            if (g->select_info.cells[i].sprites != NULL) {
                SpriteSet_Free(g->select_info.cells[i].sprites);
            }
        }
        free(g->select_info.cells);
    }
    for (i = 0; i < 10; i++) {
        Font *font = (Font *)g->motif_fonts[i];
        if (font == g->font_small || font == NULL) {
            continue;
        }
        if (font->sprites != NULL) {
            SpriteSet_Free(font->sprites);
        }
        if (font->ttf != NULL) {
            TTF_CloseFont(font->ttf);
        }
        free(font);
    }
    Anim_Free(g->motif_anims);
    Snd_Free(g->motif_snd);
    g->motif_snd = NULL;
    g->motif_snd_path[0] = '\0';
    SpriteSet_Free(g->motif_sprites);
    g->motif_sprites = NULL;
    g->motif_sff_path[0] = '\0';
}

/* 0x0045DAA0: reloads what System_UnloadMotif freed; failures are fatal. */
void System_ReloadMotif(void)
{
    char path[0x200];
    GameVars *g = g_game;
    Stream *s;
    Ini *ini;

    if (g->motif_loaded) {
        return;
    }
    if (g->motif_sprites == NULL && g->motif_sff_path[0] != '\0') {
        Log_Printf("Reloading system sprite data...");
        g->motif_sprites = Sff_Load(g->motif_sff_path);
        if (g->motif_sprites == NULL) {
            Error_Fatal("Out of memory loading system file %s\n", g->motif_sff_path);
        }
    }
    if (g->motif_snd == NULL && g->motif_snd_path[0] != '\0') {
        Log_Printf("Reloading system sound data...");
        g->motif_snd = Snd_Load(g->motif_snd_path);
        if (g->motif_snd == NULL) {
            Error_Fatal("Out of memory loading system file %s\n", g->motif_snd_path);
        }
    }
    if (g->title_bg == NULL || g->versus_bg == NULL || g->select_bg == NULL || g->option_bg == NULL) {
        Log_Printf("Reloading system background data...");
        Path_Join(g->motif_file, g->motif_dir, path);
        s = Stream_OpenFile(path, "rt");
        ini = NULL;
        if (s != NULL) {
            ini = Ini_Parse(s, INI_MODE_READ);
            Stream_Close(s);
        }
        if (ini == NULL) {
            Error_Fatal("Out of memory loading system file %s\n", g->motif_file);
        }
        if (g->title_bg == NULL) {
            g->title_bg = Bg_Load("TitleBG", ini, g->motif_dir, g->motif_sprites, g->motif_anims);
            if (g->title_bg == NULL) {
                Error_Fatal("Error reloading TitleBG\n");
            }
        }
        if (g->versus_bg == NULL) {
            g->versus_bg = Bg_Load("VersusBG", ini, g->motif_dir, g->motif_sprites, g->motif_anims);
            if (g->versus_bg == NULL) {
                Error_Fatal("Error reloading VersusBG\n");
            }
        }
        if (g->victory_bg == NULL) {
            g->victory_bg = Bg_Load("VictoryBG", ini, g->motif_dir, g->motif_sprites, g->motif_anims);
        }
        if (g->select_bg == NULL) {
            g->select_bg = Bg_Load("SelectBG", ini, g->motif_dir, g->motif_sprites, g->motif_anims);
            if (g->select_bg == NULL) {
                Error_Fatal("Error reloading SelectBG\n");
            }
        }
        if (g->option_bg == NULL) {
            g->option_bg = Bg_Load("OptionBG", ini, g->motif_dir, g->motif_sprites, g->motif_anims);
            if (g->option_bg == NULL) {
                Error_Fatal("Error reloading OptionBG\n");
            }
        }
        Ini_Free(ini);
        Log_Printf("OK\n");
    }
    g->motif_loaded = 1;
}

/* 0x0045DD80: frees the motif backgrounds and sprites (kept: sounds, fonts, animations). */
void System_UnloadMotif(void)
{
    GameVars *g = g_game;

    if (!g->motif_loaded) {
        return;
    }
    Log_Printf("Unloading system background data...");
    Bg_Free(g->option_bg);
    g->option_bg = NULL;
    Bg_Free(g->select_bg);
    g->select_bg = NULL;
    Bg_Free(g->victory_bg);
    g->victory_bg = NULL;
    Bg_Free(g->versus_bg);
    g->versus_bg = NULL;
    Bg_Free(g->title_bg);
    g->title_bg = NULL;
    Log_Printf("sprite data...");
    SpriteSet_Free(g->motif_sprites);
    g->motif_sprites = NULL;
    Log_Printf("OK\n");
    g->motif_loaded = 0;
}
