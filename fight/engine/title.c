/*
 * Title screen, options screen, key configuration and training menu
 * (original module 0x428AF0-0x42E2EF).
 *
 * The module starts right after the storyboard player (Storyboard_PlayFile
 * ends at 0x428AEA) with the title screen callbacks; its first string,
 * "%i.%i.%i %s (%i.%02i.%02i) " (0x4F574C), follows the storyboard's last one
 * ("Finished playing storyboard", 0x4F5728). It ends before Lua_Init.
 *
 * The title screen is called "mode select" by the log: it is the menu of game
 * modes over TitleBG, with the info box (F1). The options screen and the
 * training menu are drawn with the built-in fonts and plain rectangles; the
 * key configuration runs its own frame loop.
 */
#include <SDL.h>

#include "mugen.h"

/* Gameflow states returned by Title_Run / Options_Run (see Gameflow_Run). */
#define GAMEFLOW_TITLE      4
#define GAMEFLOW_OPTIONS    5
#define GAMEFLOW_SELECT     6
#define GAMEFLOW_EXIT       20

/* Buttons of ScreenLoop.input that choose an item (wider than SCREEN_IN_BUTTONS). */
#define TITLE_IN_BUTTONS    0xFF0

#define KEYCONFIG_ENTRIES   11      /* up, down, left, right, a, b, c, x, y, z, start */
#define KEYCONFIG_ITEMS     14      /* "Config all", the entries, "Default", "Exit" */
#define KEYCONFIG_DEFAULT   12
#define KEYCONFIG_EXIT      13

/* .rdata 0x4F1EDC: key slot (KEY_*) of each key configuration entry. */
static const int keyconfig_slots[KEYCONFIG_ENTRIES] = {
    KEY_JUMP, KEY_CROUCH, KEY_LEFT, KEY_RIGHT, KEY_A, KEY_B, KEY_C, KEY_X, KEY_Y, KEY_Z, KEY_START,
};

/* .rdata 0x4F2038: default keys of each player (also the built-in keys of Input_Init). */
static const int keyconfig_default_keys[2][KEYCONFIG_ENTRIES] = {
    { SDLK_UP, SDLK_DOWN, SDLK_LEFT, SDLK_RIGHT, SDLK_COMMA, SDLK_PERIOD, SDLK_SLASH, SDLK_l, SDLK_SEMICOLON,
      SDLK_QUOTE, SDLK_RETURN },
    { SDLK_w, SDLK_s, SDLK_a, SDLK_d, SDLK_f, SDLK_g, SDLK_h, SDLK_r, SDLK_t, SDLK_y, SDLK_u },
};

/* .rdata 0x4F2090: default joystick buttons of each player (pad index << 5 | button). */
static const int keyconfig_default_joy[2][KEYCONFIG_ENTRIES] = {
    { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B },
    { 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B },
};

/* Sound of the motif ({group, number}), at full volume on any channel. */
static void Title_PlaySnd(const int *snd)
{
    Sound_PlaySystem(g_game->motif_snd, snd[0], -1, snd[1], 100.0);
}

/* Starts the screen fade out (unless one is running) and makes the screen loop end after it. */
static void Title_FadeOut(ScreenLoop *screen, int time)
{
    GameVars *g = g_game;

    if (screen->fade_out == 0 || screen->fade_ticks == -1) {
        g->fade.type = FADE_PALETTE;
        g->fade.ticks = 0;
        g->fade.time = time;
        g->fade.out = 1;
        screen->fade_out = 1;
        screen->fade_ticks = time;
    }
}

/* Brightness of the blinking cursor bar for a tick count: a triangle wave. */
static int Title_Pulse(int t, int period, int base)
{
    if (t % (period * 2) < period) {
        return t % period + base;
    }
    return base + period - 1 - t % period;
}

/* ---- title screen ---- */

/* 0x00428AF0 */
int Title_Init(ScreenLoop *screen, TitleState *st)
{
    GameVars *g = g_game;
    TitleInfo *ti = &g->title_info;
    SelectInfo *info;
    int count = 0;
    int i, s, k;
    size_t len;

    memset(st, 0, sizeof(*st));
    st->chosen = 0;
    st->chosen_side = 1;
    st->item = g->game_mode;
    if (g->demo_mode) {
        st->item = TITLE_ITEM_ARCADE;
    }
    g->demo_mode = 0;

    /* forget the cursors and team menu choices of the select screen */
    info = g_selectInfo;
    for (s = 0; s < 2; s++) {
        for (k = 0; k < 4; k++) {
            info->last_row[s][k] = -1;
            info->last_col[s][k] = -1;
        }
    }
    info->teammenu[0].saved_item = -1;
    info->teammenu[1].saved_item = -1;

    for (i = 0; i < TITLE_ITEM_COUNT; i++) {
        st->items[i] = ti->item_names[i];
        if (ti->item_names[i][0] != '\0') {
            count++;
        } else {
            st->items[i] = NULL;
        }
    }
    if (count == 0) {
        /* no menu: run the demo, or quit */
        g = g_game;
        if (g->char_count > 0 && g_keyState[SDLK_ESCAPE] == 0 && !g_quitRequested) {
            g->demo_mode = 1;
            g->demo_ticks = 0;
            st->item = TITLE_ITEM_VERSUS;
            st->chosen_side = 1;
        } else {
            st->item = TITLE_ITEM_EXIT;
        }
        screen->fade_out = 1;
        screen->fade_ticks = 0;
    }
    /* Quirk: with every item hidden this never ends. */
    while (st->items[st->item] == NULL) {
        st->item++;
        if (st->item >= TITLE_ITEM_COUNT) {
            st->item = 0;
        }
    }

    memset(&st->box_w, 0, sizeof(*st) - offsetof(TitleState, box_w));
    st->text[0] = '\0';
    strncat(st->text, "This game is powered by M.U.G.E.N version ", sizeof(st->text) - 1);
    len = strlen(st->text);
    _snprintf(st->text + len, sizeof(st->text) - len, "%i.%i.%i %s (%i.%02i.%02i) ", MUGEN_VERSION_MAJOR,
              MUGEN_VERSION_MINOR, MUGEN_VERSION_PATCH, MUGEN_VERSION_TAG, MUGEN_BUILD_YEAR, MUGEN_BUILD_MONTH,
              MUGEN_BUILD_DAY);
    len = strlen(st->text);
    if (sizeof(st->text) - len > 1) {
        strncat(st->text,
                " (c) Elecbyte 1999-2013\n\nThis version of M.U.G.E.N is a development build, but free for "
                "non-commercial use only.\nPlease read the license in readme.txt before using this software.\n",
                sizeof(st->text) - 1 - len);
    }
    st->box_x = st->box_x_hidden = st->box_x_shown = (g_targetWidth - 600) / 2;
    st->box_y = st->box_y_target = st->box_y_hidden = -405;
    st->box_y_shown = (g_targetHeight - 400) / 2;
    st->box_w = 600;
    st->box_h = 400;

    g = g_game;
    if (!g_titleInfoShown) {
        g_titleInfoShown = 1;
        st->show_info = g->first_run;
    }
    screen->bg = g->title_bg;
    screen->scale = (float)g->motif_scale;
    screen->bgm = &g->title_bgm;
    g->fade.type = FADE_PALETTE;
    g->fade.ticks = 0;
    g->fade.time = ti->fadein_time;
    g->fade.out = 0;
    Log_Printf("Mode select init\n");
    return 1;
}

/* 0x00428D90: the chosen item becomes the game mode (9 options, 10 exit). */
int Title_End(ScreenLoop *screen, TitleState *st)
{
    GameVars *g = g_game;

    g->game_mode = st->item;
    g->game_mode_side = st->chosen_side - 1;
    memset(st, 0, sizeof(*st));
    Log_Printf("End of mode select loop\n");
    return 1;
}

/* 0x00428DD0 */
int Title_Update(ScreenLoop *screen, TitleState *st)
{
    GameVars *g = g_game;
    const TitleInfo *ti = &g->title_info;
    int key = screen->key;
    int chosen = 0;

    if (screen->fade_out && screen->fade_ticks > 0) {
        /* escape skips the fade out */
        if (key == SDLK_ESCAPE || g_quitRequested) {
            screen->fade_out = 1;
            screen->fade_ticks = 0;
        }
        return 1;
    }
    if (!st->box_shown && !st->chosen) {
        if (screen->input & SCREEN_IN_UP) {
            do {
                if (--st->item < 0) {
                    st->item = TITLE_ITEM_COUNT - 1;
                }
            } while (st->items[st->item] == NULL);
            Title_PlaySnd(ti->move_snd);
        }
        if (screen->input & SCREEN_IN_DOWN) {
            do {
                if (++st->item >= TITLE_ITEM_COUNT) {
                    st->item = 0;
                }
            } while (st->items[st->item] == NULL);
            Title_PlaySnd(ti->move_snd);
        }
        if (screen->input & TITLE_IN_BUTTONS) {
            st->chosen = 1;
            st->chosen_side = screen->input_side + 1;
            Title_PlaySnd(ti->done_snd);
            chosen = 1;
        }
    }
    g = g_game;
    if (screen->input != 0 || st->box_moving) {
        screen->idle_ticks = 0;
    }
    if (screen->idle_ticks > g->demo_wait && g->demo_wait != -1 && g->char_count > 0) {
        /* idle: start the demo */
        g->demo_mode = 1;
        g->demo_ticks = 0;
        st->item = TITLE_ITEM_VERSUS;
        st->chosen = 1;
        st->chosen_side = 1;
        chosen = 1;
    }
    if (key == SDLK_F1 || st->show_info) {
        st->box_moving = 1;
        st->box_shown = st->box_shown == 0;
        if (st->box_shown == 1) {
            st->box_y_target = st->box_y_shown;
        } else if (st->box_shown == 0) {
            st->box_y_target = st->box_y_hidden;
        }
        st->show_info = 0;
    }
    if (key == SDLK_ESCAPE || g_quitRequested) {
        st->item = TITLE_ITEM_EXIT;
        st->chosen = 1;
    } else if (!chosen) {
        return 1;
    }
    Title_FadeOut(screen, ti->fadeout_time);
    return 1;
}

/* Moves the scroll target by one item when the cursor left the visible window. */
static void Title_ScrollToItem(const TitleInfo *ti, TitleState *st, int row)
{
    int rel = row * ti->item_spacing[1] - st->scroll_target;

    if (rel > (ti->window_visible_items - 1) * ti->item_spacing[1]) {
        st->scroll_target += ti->item_spacing[1];
    }
    if (rel < 0) {
        st->scroll_target -= ti->item_spacing[1];
    }
}

/* 0x00429020: scrolls the menu and slides the info box. */
int Title_Tick(ScreenLoop *screen, TitleState *st)
{
    const TitleInfo *ti = &g_game->title_info;
    int rows = 0;
    int i, cur, target;
    float dist;

    if (st->item >= 0) {
        for (i = 0; i <= st->item; i++) {
            if (st->items[i] != NULL) {
                rows++;
            }
        }
    }
    Title_ScrollToItem(ti, st, rows - 1);
    Title_ScrollToItem(ti, st, rows - 1);
    target = st->scroll_target;
    cur = st->scroll;
    if (cur < target) {
        st->scroll = (target - cur) / 5 + cur + 1;
    } else if (cur > target) {
        st->scroll = (target - cur) / 5 + cur - 1;
    }
    dist = (float)fabs((double)(target - st->scroll));
    if (dist <= 2.0f || screen->ticks == 0) {
        st->scroll = target;
    }

    if (st->box_moving) {
        int to = st->box_y_target;
        int y = st->box_y;
        if (y != to) {
            int dir = to > y ? 1 : -1;
            y = (to - y) / 12 + y + dir * 5;
            st->box_dir = dir;
            st->box_y = y;
            if ((to - y) * dir <= 0) {
                st->box_y = to;
                if (!st->box_shown) {
                    st->box_moving = 0;
                }
            }
        }
        st->box_progress = (float)((double)(st->box_y - st->box_y_hidden) /
                                   (double)(st->box_y_shown - st->box_y_hidden));
    }
    return 1;
}

/* 0x0042E260: text of the info box in the [Infobox] font (word wrapped), white. */
void Infobox_DrawText(const char *text, int x, int y)
{
    GameVars *g = g_game;
    int wrap = g_fontWrapMode;
    struct Font *const *fonts = (struct Font *const *)g->motif_fonts;
    Font *font = NULL;
    int bank;

    if (g->infobox_font.fight_font == 1) {
        fonts = g->fight.fonts;
    }
    if (g->infobox_font.font >= 0 && g->infobox_font.font < 10) {
        font = fonts[g->infobox_font.font];
    }
    bank = g->infobox_font.bank;
    if (font == NULL) {
        font = g->font_medium;
        bank = 0;
    }
    g_fontWrapMode = FONT_WRAP_WORD;
    Font_DrawText(text, font, x, font->size_h + y, g->infobox_font.align, bank, 0x100, 0x100, 0x100);
    g_fontWrapMode = wrap;
}

/* The info box: a framed panel with the motif's [Infobox Text] and the version text. */
static void Title_DrawInfoBox(TitleState *st)
{
    GameVars *g = g_game;
    int x1 = st->box_x;
    int y1 = st->box_y;
    int x2 = st->box_w + x1;
    int y2 = st->box_h + y1;
    unsigned int light, mid;
    int wrap;
    GfxLineFn line = (GfxLineFn)g_gfxDrawLine;
    GfxPixelFn pixel = (GfxPixelFn)g_gfxPutPixel;
    GfxRectFn frame = (GfxRectFn)g_gfxDrawRect;

    light = Video_MapRGB(0x56, 0x76, 0xA8);
    mid = Video_MapRGB(0x40, 0x60, 0x94);
    wrap = g_fontWrapMode;
    Gfx_SetClipRect(x1, y1, x2, y2);
    g_fontWrapMode = FONT_WRAP_WORD;
    g_gfxBlendRect(x1 + 2, y1 + 2, x2 - 2, y2 - 2, Video_MapRGB(0, 0, 0x40), 0x100, 0x80);
    frame(x1 + 1, y1 + 1, x2 - 1, y2 - 1, light);
    frame(x1 + 2, y1 + 2, x2 - 2, y2 - 2, mid);
    line((float)(x1 + 1), (float)y1, (float)(x2 - 1), (float)y1, mid);
    line((float)(x1 + 1), (float)y2, (float)(x2 - 1), (float)y2, mid);
    line((float)x1, (float)(y1 + 1), (float)x1, (float)(y2 - 1), mid);
    line((float)x2, (float)(y1 + 1), (float)x2, (float)(y2 - 1), mid);
    /* rounded corners */
    pixel(x1 + 1, y1 + 1, mid);
    pixel(x2 - 1, y1 + 1, mid);
    pixel(x1 + 1, y2 - 1, mid);
    pixel(x2 - 1, y2 - 1, mid);
    pixel(x1 + 2, y1 + 2, light);
    pixel(x2 - 2, y1 + 2, light);
    pixel(x1 + 2, y2 - 2, light);
    pixel(x2 - 2, y2 - 2, light);
    pixel(x1 + 3, y1 + 3, mid);
    pixel(x2 - 3, y1 + 3, mid);
    pixel(x1 + 3, y2 - 3, mid);
    pixel(x2 - 3, y2 - 3, mid);
    if (g->infobox_text[0] == '\0') {
        Infobox_DrawText(st->text, st->box_x + 10, st->box_y + 6);
    } else {
        /* the motif's text in the top three quarters, a line, then the version text */
        int ym = st->box_h * 3 / 4 + y1;
        Gfx_SetClipRect(x1, y1, x2 - 9, ym - 1);
        Infobox_DrawText(g->infobox_text, st->box_x + 10, st->box_y + 6);
        Gfx_SetClipRect(x1, y1, x2, y2);
        g_gfxFillRect(x1 + 10, ym, x2 - 10, ym + 1, Video_MapRGB(0xFF, 0xFF, 0xFF));
        Infobox_DrawText(st->text, st->box_x + 10, ym + 6);
    }
    Infobox_DrawText("Press F1 to continue.", st->box_x + 10, st->box_y - 21 + st->box_h);
    Gfx_SetClipRect(0, 0, g_targetWidth, g_targetHeight);
    g_fontWrapMode = wrap;
}

/* 0x004291D0 */
int Title_Draw(ScreenLoop *screen, TitleState *st)
{
    GameVars *g = g_game;
    const TitleInfo *ti = &g->title_info;
    int ticks = screen->ticks;
    int row = 0;
    int i, y_bar;

    if (g->skip_draw) {
        return 1;
    }

    /* the menu, clipped to its window; items slide out sideways while the info box shows */
    {
        int y2 = (int)(((ti->window_visible_items - 1) * ti->item_spacing[1] + ti->window_margins_y[1] + ti->pos_y) *
                       g->motif_scale);
        int y1 = (int)((ti->pos_y - ti->window_margins_y[0]) * g->motif_scale);
        Gfx_SetClipRect(0, y1, g_targetWidth, y2);
    }
    for (i = 0; i < TITLE_ITEM_COUNT; i++) {
        const char *name = st->items[i];
        float x0, away;
        int x, y;
        if (name == NULL) {
            continue;
        }
        x0 = (float)ti->pos_x;
        if (i & 1) {
            away = (float)((int)g_game->game_coord[0] * 409.0 / 320.0);
        } else {
            away = (float)((int)g_game->game_coord[0] * -91.0 / 320.0);
        }
        x = (int)(x0 + (away - x0) * st->box_progress);
        y = ti->item_spacing[1] * row - st->scroll + ti->pos_y;
        if (i == st->item && !g_game->demo_mode) {
            int c = Title_Pulse(ticks * 2, 32, 16);
            unsigned int color = Video_MapRGB(c / 2, c, c);
            if (ti->boxcursor_visible && g_game->fade.type == FADE_NONE) {
                GameVars *gg = g_game;
                g_gfxFillRectColor((int)((ti->boxcursor_coords[0] + x) * gg->motif_scale),
                                   (int)((ti->boxcursor_coords[1] + y) * gg->motif_scale),
                                   (int)((ti->boxcursor_coords[2] + x) * gg->motif_scale),
                                   (int)((ti->boxcursor_coords[3] + y) * gg->motif_scale), color);
            }
            FontSpec_DrawText(&ti->item_active_font, st->items[i], x, y);
        } else {
            FontSpec_DrawText(&ti->item_font, name, x, y);
        }
        row++;
    }
    Gfx_SetClipRect(0, 0, g_targetWidth, g_targetHeight);

    if (st->box_moving) {
        Title_DrawInfoBox(st);
    }

    /* bottom bar: version and copyright, or "Loading demo" */
    y_bar = g_targetHeight - 2;
    if (g_game->fade.type != FADE_NONE) {
        y_bar = (int)(g_game->fade.progress * 10.0 + y_bar);
    }
    g_gfxBlendRect(0, y_bar - 5, g_targetWidth - 1, y_bar + 1, Video_MapRGB(0, 0, 0x40), 0x100, 0x80);
    if (st->chosen && g_game->demo_mode) {
        Font_DrawText("Loading demo", g_game->font_small, g_targetWidth - 4, y_bar, FONT_ALIGN_RIGHT, 0, 0xC0, 0xC0,
                      0xC0);
        return 1;
    }
    Font_Printf(g_game->font_small, 2, y_bar, FONT_ALIGN_LEFT, 0, 0xC0, 0xC0, 0xC0, "M.U.G.E.N %i.%i.%i %s",
                MUGEN_VERSION_MAJOR, MUGEN_VERSION_MINOR, MUGEN_VERSION_PATCH, MUGEN_VERSION_TAG);
    Font_DrawText("(c) Elecbyte 1999-2013", g_game->font_small, g_targetWidth - 3, y_bar, FONT_ALIGN_RIGHT, 0, 0xC0,
                  0xC0, 0xC0);
    Font_DrawText("Press F1 for info", g_game->font_small, g_targetWidth / 2 - 1,
                  (int)(st->box_progress * 18.0 + y_bar), FONT_ALIGN_CENTER, 0, 0xC0, 0xC0, 0xC0);
    Font_DrawText("Press F1 to continue", g_game->font_small, g_targetWidth / 2 - 1,
                  (int)((y_bar + 18) - st->box_progress * 18.0), FONT_ALIGN_CENTER, 0, 0xC0, 0xC0, 0xC0);
    return 1;
}

/* 0x00429B80: the title screen loop; returns 0 when the state cannot be allocated. */
int Title_Setup(ScreenLoop *screen)
{
    memset(screen, 0, sizeof(*screen));
    screen->scale = 1.0f;
    screen->fadein_time = 10;
    screen->fadeout_time = 10;
    screen->kind = 1;
    screen->fps = 60;
    screen->reload_motif = 1;
    screen->start_frame = g_game->frames;
    screen->state = calloc(1, sizeof(TitleState));
    if (screen->state == NULL) {
        return 0;
    }
    screen->init = (ScreenFn)Title_Init;
    screen->end = (ScreenFn)Title_End;
    screen->update = (ScreenFn)Title_Update;
    screen->tick = (ScreenFn)Title_Tick;
    screen->draw = (ScreenFn)Title_Draw;
    screen->overlay = NULL;
    return 1;
}

/* 0x00429C20: returns the next gameflow state: options, select screen or exit. */
int Title_Run(void)
{
    ScreenLoop screen;
    int mode;

    Log_Printf("Entering mode select.\n");
    Title_Setup(&screen);       /* the result is not checked */
    if (!Screen_Run(&screen)) {
        return GAMEFLOW_EXIT;   /* the state is not freed */
    }
    screen.result = screen.done;
    if (screen.state != NULL) {
        free(screen.state);
    }
    memset(&screen, 0, sizeof(screen));
    mode = g_game->game_mode;
    if (mode == TITLE_ITEM_EXIT) {
        return GAMEFLOW_EXIT;
    }
    return mode != TITLE_ITEM_OPTIONS ? GAMEFLOW_SELECT : GAMEFLOW_OPTIONS;
}

/* ---- options screen ---- */

/* Value of main menu item n (GameVars.difficulty, life, ... option_item10). */
static int *Options_Value(GameVars *g, int n)
{
    return (int *)((char *)g + offsetof(GameVars, difficulty)) + n;
}

/* Value of item n of the input menu for a side (item 1: joystick type). */
static int *Options_InputValue(GameVars *g, int side, int n)
{
    return &g->option_input[side][n];
}

/* Value of item n of the team menu (team_1vs2_life, team_lose_on_ko, then option_team_unused). */
static int *Options_TeamValue(GameVars *g, int n)
{
    return (int *)((char *)g + offsetof(GameVars, team_1vs2_life)) + n;
}

/* 0x00429CB0: font/options.def, loaded once at a size that fits the game height; else the medium font. */
Font *Options_GetFont(void)
{
    GameVars *g = g_game;
    Font *font;
    int height;

    if (g->options_font != NULL) {
        return g->options_font;
    }
    height = (int)g->game_coord[1];
    font = Font_Load("font/options.def", height > 240 ? height * 10 / 240 : 10);
    g = g_game;
    g->options_font = font;
    if (font == NULL) {
        g->options_font = g->font_medium;
        return g->options_font;
    }
    return font;
}

/* 0x00429D30 */
int Options_Init(ScreenLoop *screen, OptionsState *st)
{
    const OptionInfo *oi = &g_game->option_info;
    GameVars *g;
    Font *font;

    memset(st, 0, sizeof(*st));
    font = Options_GetFont();
    st->font = font;
    st->line_h = font->spacing_y + font->size_h;
    st->char_w = font->spacing_x + font->size_w;
    st->line_gap = st->line_h / 6;
    st->margin = 15;
    st->bg_color = Video_MapRGB(0x80, 0x60, 0x40);
    st->border_color = Video_MapRGB(0x40, 0x30, 0x20);
    g = g_game;
    st->columns = 1;
    st->accept = 0;
    st->menu = OPTIONS_MENU_MAIN;
    st->main_items[OPTIONS_DIFFICULTY] = "Difficulty";
    st->main_items[OPTIONS_LIFE] = "Life";
    st->main_items[OPTIONS_TIME] = "Time Limit";
    st->main_items[OPTIONS_GAME_SPEED] = "Game Speed";
    st->main_items[OPTIONS_MASTER_VOLUME] = "Master Volume";
    st->main_items[OPTIONS_WAV_VOLUME] = "Wave/SFX Volume";
    st->main_items[OPTIONS_BGM_VOLUME] = "BGM Volume";
    st->main_items[OPTIONS_INPUT_CONFIG] = "Input Config";
    st->main_items[OPTIONS_TEAM_CONFIG] = "Team Mode Config";
    st->main_items[OPTIONS_LOAD_SAVE] = "Load/Save";
    st->main_items[OPTIONS_RETURN] = "Return to Main Menu";
    st->main_items[OPTIONS_DEFAULTS] = "Default Values";
    g->option_save = 0;
    st->input_items[3] = "Default Values";
    st->input_items[0] = "Key Config";
    st->input_items[1] = "Joystick";
    st->input_items[2] = "Joystick Config";
    st->input_items[4] = "Return to Options";
    st->team_items[0] = "Single VS Simul Advantage";
    st->team_items[1] = "If player KOed";
    st->team_items[2] = "Default Values";
    st->team_items[3] = "Return to Options";
    st->menus[OPTIONS_MENU_MAIN] = st->main_items;
    st->menus[OPTIONS_MENU_INPUT] = st->input_items;
    st->menus[OPTIONS_MENU_TEAM] = st->team_items;
    screen->bg = g->option_bg;
    screen->scale = (float)g->motif_scale;
    screen->bgm = NULL;
    g->fade.type = FADE_PALETTE;
    g->fade.ticks = 0;
    g->fade.time = oi->fadein_time;
    g->fade.out = 0;
    Log_Printf("Options screen init\n");
    return 1;
}

/* 0x00429F60 */
int Options_End(ScreenLoop *screen, OptionsState *st)
{
    memset(st, 0, sizeof(*st));
    Log_Printf("End of options screen loop\n");
    return 1;
}

/* Left / right on a main menu item. prompt is OptionsState.prompt before this frame's input. */
static int Options_ChangeMain(OptionsState *st, int item, int v, int left, int right, int prompt)
{
    switch (item) {
    case OPTIONS_DIFFICULTY:
        if (left && v > 1) {
            v--;
        }
        if (right && v < 8) {
            v++;
        }
        break;
    case OPTIONS_LIFE:
        if (left && v > 30) {
            v -= 10;
        }
        if (right && v < 300) {
            v += 10;
        }
        break;
    case OPTIONS_TIME:
        /* 20, 40, 60, 80, 99, none (-1) */
        if (left) {
            if (v > 20) {
                v = v == 99 ? 80 : v - 20;
            } else if (v == 99) {
                v = 80;
            } else if (v == -1) {
                v = 99;
            }
        }
        if (right) {
            if (v < 80 && v != -1) {
                v += 20;
            } else if (v == 80) {
                v = 99;
            } else if (v == 99) {
                v = -1;
            }
        }
        break;
    case OPTIONS_GAME_SPEED:
        if (left && v > -9) {
            v--;
        }
        if (right && v < 9) {
            v++;
        }
        break;
    case OPTIONS_MASTER_VOLUME:
    case OPTIONS_WAV_VOLUME:
    case OPTIONS_BGM_VOLUME:
        /* -1 off .. 101 max */
        if (left && v > -1) {
            v--;
        }
        if (right && v < 101) {
            v++;
        }
        break;
    case OPTIONS_LOAD_SAVE:
        if (left) {
            v = 0;
        }
        if (right) {
            v = 1;
        }
        if (prompt == 1 && st->prompt_done) {
            /* the "Options Loaded/Saved" message was confirmed */
            st->prompt = 0;
            st->prompt_done = 0;
        }
        break;
    case OPTIONS_DEFAULTS:
        if (prompt == 1 && st->prompt_done) {
            /* Quirk: the slot's old value is written back after Options_SetDefaults. */
            if (st->prompt_yes == 1) {
                Options_SetDefaults();
            }
            st->prompt = 0;
            st->prompt_done = 0;
        }
        break;
    }
    return v;
}

/* Back from a sub-menu to the main menu. */
static void Options_ReturnToMain(OptionsState *st)
{
    st->cursor[0] = st->main_cursor0;
    st->cursor[1] = st->main_cursor1;
    st->menu = OPTIONS_MENU_MAIN;
    st->accept = 0;
    st->skip_draw = 1;
}

/* Enters a sub-menu. */
static void Options_EnterMenu(OptionsState *st, int menu)
{
    st->main_cursor0 = st->cursor[0];
    st->main_cursor1 = st->cursor[1];
    st->menu = menu;
    st->cursor[1] = 0;
    st->cursor[0] = 0;
    st->accept = 0;
    st->skip_draw = 1;
}

/*
 * The joystick type of one side changed to v: every side that has a
 * joystick type gets its joystick map (and, quirk, the type v), the others
 * lose it; then the pads are closed or (re)opened.
 */
static void Options_SetJoystick(int v)
{
    GameVars *g = g_game;
    int p, k;

    for (p = 0; p < 2; p++) {
        g->use_joystick[p] = *Options_InputValue(g, p, 1) != 0;
        if (g->use_joystick[p]) {
            memcpy(g->joy_keys_active[p], g->joy_keys[p], sizeof(g->joy_keys_active[p]));
            g->joystick_type[p] = v;
        } else {
            memset(g->joy_keys_active[p], 0, sizeof(g->joy_keys_active[p]));
            g->joystick_type[p] = 0;
            for (k = 1; k < KEY_COUNT; k++) {
                if (g->joy_keys_active[p][k] == 0) {
                    g->joy_keys_active[p][k] = JOY_NONE;
                }
            }
        }
    }
    g = g_game;
    if (!g->use_joystick[0] && !g->use_joystick[1] && g_padsActive) {
        Input_DeinitPads();
    } else if (!g->nojoy && !g_padsActive) {
        if (!Pads_Init()) {
            g_game->use_joystick[1] = 0;
            g_game->use_joystick[0] = 0;
        } else {
            Log_Printf("Setting up input\n");
            Input_Remap();
        }
    }
}

/* Runs the key or joystick configuration from the input menu. */
static void Options_RunKeyConfig(ScreenLoop *screen, const OptionInfo *oi, int joystick)
{
    Title_PlaySnd(oi->done_snd);
    Log_Printf(joystick ? "Config pads menu\n" : "Config keys menu\n");
    screen->ticks = KeyConfig_Run(joystick, screen->ticks);
    Log_Printf(joystick ? "Return from config pads menu\n" : "Return from config keys menu\n");
    Title_PlaySnd(oi->cancel_snd);
}

/*
 * 0x00429F90. Quirk kept: in the input and team menus, a button pressed on
 * an item without an action ("Joystick", the team values) leaves accept set,
 * which fades the options screen out like "Return to Main Menu".
 */
int Options_Update(ScreenLoop *screen, OptionsState *st)
{
    const OptionInfo *oi = &g_game->option_info;
    int key = screen->key;
    int side = screen->input_side;
    int old_accept = st->accept;
    int *cursor = st->cursor;
    int left, right, prompt, v;

    if (st->lock_frames > 0) {
        screen->key = 0;
        screen->input2 = 0;
        screen->input = 0;
        screen->input_side = 0;
        st->lock_frames--;
        key = 0;
    }
    switch (st->menu) {
    case OPTIONS_MENU_MAIN:
        st->row_h = st->line_gap + st->line_h;
        st->item_count = OPTIONS_MAIN_COUNT;
        st->box_h = st->line_h + st->row_h * 14;
        st->columns = 1;
        st->box_w = st->char_w * 26;
        st->value_x = st->box_w;
        break;
    case OPTIONS_MENU_TEAM:
        st->item_count = 4;
        st->row_h = st->line_gap + st->line_h;
        st->box_h = st->line_h + st->row_h * 5;
        st->columns = 1;
        st->box_w = st->char_w * 32;
        st->value_x = st->box_w;
        break;
    case OPTIONS_MENU_INPUT:
        st->box_w = st->char_w * 40;
        st->box_h = st->line_h + (st->line_gap + st->line_h) * 8;
        st->item_count = 5;
        st->row_h = st->line_gap + st->line_h;
        st->columns = 2;
        st->value_x = st->box_w / 2;
        break;
    }
    st->box_x = (g_targetWidth - st->box_w) / 2;
    st->box_y = st->line_h + 40;

    if (!st->prompt || (cursor[side] == OPTIONS_LOAD_SAVE && st->menu == OPTIONS_MENU_MAIN)) {
        if (screen->input & SCREEN_IN_UP) {
            if (--cursor[side] < 0) {
                cursor[side] = st->item_count - 1;
            }
            st->prompt_done = 0;
            st->prompt = 0;
            Title_PlaySnd(oi->move_snd);
        }
        if (screen->input & SCREEN_IN_DOWN) {
            if (++cursor[side] >= st->item_count) {
                cursor[side] = 0;
            }
            st->prompt_done = 0;
            st->prompt = 0;
            Title_PlaySnd(oi->move_snd);
        }
    }
    if (st->columns == 1) {
        cursor[1 - side] = cursor[side];
    }
    if (screen->input & TITLE_IN_BUTTONS) {
        st->accept = 1;
        st->accept_side = side + 1;
    } else {
        st->accept = 0;
    }
    right = (screen->input & SCREEN_IN_RIGHT) != 0;
    left = (screen->input & SCREEN_IN_LEFT) != 0;
    prompt = st->prompt;
    if (prompt) {
        /* left / right choose No / Yes, a button or escape answers */
        if (left || key == SDLK_ESCAPE || g_quitRequested) {
            st->prompt_yes = 0;
        }
        if (right) {
            st->prompt_yes = 1;
        }
        if (st->accept || key == SDLK_ESCAPE || g_quitRequested) {
            st->prompt_done = 1;
        }
        right = 0;
        left = 0;
        st->accept = 0;
        if (key == SDLK_ESCAPE) {
            key = 0;
        }
    }

    switch (st->menu) {
    case OPTIONS_MENU_MAIN:
        v = *Options_Value(g_game, cursor[side]);
        v = Options_ChangeMain(st, cursor[side], v, left, right, prompt);
        *Options_Value(g_game, cursor[side]) = v;
        if (!st->prompt) {
            if (key == SDLK_ESCAPE || g_quitRequested) {
                cursor[side] = OPTIONS_RETURN;
                st->accept = 1;
            }
            if (key == SDLK_F1) {
                cursor[side] = OPTIONS_INPUT_CONFIG;
                st->accept = 1;
            } else if (key == SDLK_F2) {
                cursor[side] = OPTIONS_TEAM_CONFIG;
                st->accept = 1;
            }
        }
        if (!st->accept) {
            break;
        }
        switch (cursor[side]) {
        case OPTIONS_INPUT_CONFIG:
            Title_PlaySnd(oi->done_snd);
            Options_EnterMenu(st, OPTIONS_MENU_INPUT);
            break;
        case OPTIONS_TEAM_CONFIG:
            Title_PlaySnd(oi->done_snd);
            Options_EnterMenu(st, OPTIONS_MENU_TEAM);
            break;
        case OPTIONS_LOAD_SAVE:
            Title_PlaySnd(oi->done_snd);
            if (*Options_Value(g_game, cursor[side]) == 0) {
                Options_Load();
            } else {
                Options_Save();
            }
            st->prompt = 1;         /* shows "Options Loaded/Saved" */
            st->lock_frames = 1;
            st->accept = 0;
            break;
        case OPTIONS_DEFAULTS:
            Title_PlaySnd(oi->done_snd);
            st->prompt = 1;         /* "Reset default values?" */
            st->prompt_yes = 0;
            st->accept = 0;
            break;
        case OPTIONS_RETURN:
            break;                  /* fades out below */
        default:
            st->accept = 0;
            break;
        }
        break;

    case OPTIONS_MENU_INPUT: {
        int item = cursor[side];
        int old;
        v = *Options_InputValue(g_game, side, item);
        old = v;
        if (item == 1) {
            /* joystick on / off */
            if (left && v != 0) {
                v = 0;
            }
            if (right && v == 0) {
                v = 1;
            }
            if (v != old) {
                *Options_InputValue(g_game, side, item) = v;
                Options_SetJoystick(v);
            }
        }
        *Options_InputValue(g_game, side, cursor[side]) = v;
        if (key == SDLK_ESCAPE || g_quitRequested) {
            cursor[1] = 4;
            cursor[0] = 4;
            st->accept = 1;
        }
        if (key == SDLK_F1) {
            cursor[1] = 0;
            cursor[0] = 0;
            st->accept = 1;
        } else if (key == SDLK_F2) {
            cursor[1] = 2;
            cursor[0] = 2;
            st->accept = 1;
        }
        if (!st->accept) {
            break;
        }
        switch (cursor[side]) {
        case 0:
            Options_RunKeyConfig(screen, oi, 0);
            st->accept = 0;
            break;
        case 2:
            if (g_game->use_joystick[0] || g_game->use_joystick[1]) {
                Options_RunKeyConfig(screen, oi, 1);
            }
            st->accept = 0;
            break;
        case 3:
            Title_PlaySnd(oi->done_snd);
            *Options_InputValue(g_game, side, 1) = 0;
            st->accept = 0;
            break;
        case 4:
            Title_PlaySnd(oi->cancel_snd);
            Options_ReturnToMain(st);
            break;
        }
        break;
    }

    case OPTIONS_MENU_TEAM: {
        int item = cursor[side];
        v = *Options_TeamValue(g_game, item);
        if (item == 0) {
            if (left && v > 100) {
                v -= 10;
            }
            if (right && v < 300) {
                v += 10;
            }
        } else if (item == 1) {
            if (left && v > 0) {
                v--;
            }
            if (right && v < 1) {
                v++;
            }
        }
        *Options_TeamValue(g_game, item) = v;
        if (key == SDLK_ESCAPE || g_quitRequested) {
            cursor[side] = 3;
            st->accept = 1;
        }
        if (!st->accept) {
            break;
        }
        if (cursor[side] == 2) {
            Title_PlaySnd(oi->done_snd);
            g_game->team_1vs2_life = 200;
            g_game->team_lose_on_ko = 0;
            st->accept = 0;
        } else if (cursor[side] == 3) {
            Title_PlaySnd(oi->cancel_snd);
            Options_ReturnToMain(st);
        }
        break;
    }
    }

    if (st->accept && old_accept != st->accept && (!screen->fade_out || screen->fade_ticks == -1)) {
        Title_FadeOut(screen, oi->fadeout_time);
        Title_PlaySnd(oi->cancel_snd);
    }
    return 1;
}

/* Value text of a main menu item. */
static void Options_MainValueText(int item, int v, char *text)
{
    switch (item) {
    case OPTIONS_DIFFICULTY:
        strcpy(text, v < 3 ? "Easy" : v > 5 ? "Hard" : "Medium");
        sprintf(text + strlen(text), " %i", v);
        break;
    case OPTIONS_LIFE:
        sprintf(text, "%i%%", v);
        break;
    case OPTIONS_TIME:
        if (v != -1) {
            sprintf(text, "%i", v);
        } else {
            strcpy(text, "None");
        }
        break;
    case OPTIONS_GAME_SPEED:
        if (v == 0) {
            strcpy(text, "Normal");
        } else if (v > 0) {
            sprintf(text, "Fast %i", v);
        } else {
            sprintf(text, "Slow %i", -v);
        }
        break;
    case OPTIONS_MASTER_VOLUME:
    case OPTIONS_WAV_VOLUME:
    case OPTIONS_BGM_VOLUME:
        if (v == -1) {
            strcpy(text, "Off");
        } else if (v == 101) {
            strcpy(text, "Max");
        } else {
            sprintf(text, "%i", v);
        }
        break;
    case OPTIONS_LOAD_SAVE:
        strcpy(text, v == 0 ? "Load" : "Save");
        break;
    }
}

/* Value text of the joystick type ("Joystick" item of the input menu). */
static void Options_JoystickText(int v, char *text)
{
    if (v == 0) {
        strcpy(text, "Disabled");
    } else if (v == 1) {
        strcpy(text, "Enabled");
    } else if (v >= 4 && v <= 8) {
        sprintf(text, "%i button", v);
    } else if (v == 20) {
        strcpy(text, "Sidewinder");
    } else if (v == 21) {
        strcpy(text, "SNESKey");
    } else if (v == 22) {
        strcpy(text, "PSX Pad");
    } else if (v == 23) {
        strcpy(text, "N64 Pad");
    }
}

/* Key hint drawn right-aligned after an item, or NULL. */
static const char *Options_Hint(int menu, int col, int item)
{
    if (menu == OPTIONS_MENU_MAIN) {
        if (item == OPTIONS_INPUT_CONFIG) {
            return "(F1)";
        }
        if (item == OPTIONS_TEAM_CONFIG) {
            return "(F2)";
        }
        if (item == OPTIONS_RETURN) {
            return "(Esc)";
        }
    } else if (menu == OPTIONS_MENU_INPUT) {
        if (col == 1) {
            if (item == 0) {
                return "(F1)";
            }
            if (item == 2) {
                return "(F2)";
            }
            if (item == 4) {
                return "(Esc)";
            }
        }
    } else if (menu == OPTIONS_MENU_TEAM && item == 3) {
        return "(Esc)";
    }
    return NULL;
}

/* 1 if a blank row comes before the item. */
static int Options_HasGap(int menu, int item)
{
    switch (menu) {
    case OPTIONS_MENU_MAIN:
        return item == OPTIONS_RETURN || item == OPTIONS_LOAD_SAVE;
    case OPTIONS_MENU_INPUT:
        return item == 0 || item == 3 || item == 1;
    case OPTIONS_MENU_TEAM:
        return item == 3;
    }
    return 0;
}

/* 0x0042AB00 */
int Options_Draw(ScreenLoop *screen, OptionsState *st)
{
    GameVars *g = g_game;
    OptionInfo *oi = &g->option_info;
    Font *font = st->font;
    int line_h = st->line_h;
    int char_w = st->char_w;
    int line_gap = st->line_gap;
    int box_x = st->box_x;
    int box_y = st->box_y;
    int box_w = st->box_w;
    int box_h = st->box_h;
    int row_h = st->row_h;
    int value_x = st->value_x;
    int margin = st->margin;
    unsigned int border = st->border_color;
    int menu = st->menu;
    GfxRectFn rect = (GfxRectFn)g_gfxFillRectSub;
    unsigned int last_color;
    const char *title = NULL;
    int col;

    if (st->skip_draw) {
        st->skip_draw = 0;
        return 1;
    }
    if (g->skip_draw) {
        return 1;
    }

    /* the panel */
    {
        int x1 = box_x - margin;
        int x2 = box_w + margin + box_x;
        int y2 = box_h + box_y;
        rect(x1, box_y - 14, x2 - 1, y2 - 1, st->bg_color);
        rect(x1 - 1, box_y - 13, x1 - 1, y2 - 2, border);
        rect(x2, box_y - 13, x2, y2 - 2, border);
        rect(x1 + 1, box_y - 15, x2 - 2, box_y - 15, border);
        rect(x1 + 1, y2, x2 - 2, y2, border);
    }
    /* Uninitialised in the original before the first cursor bar: it holds box_y - 13 there. */
    last_color = (unsigned int)(box_y - 13);

    /* title */
    if (menu == OPTIONS_MENU_MAIN) {
        title = "OPTIONS";
    } else if (menu == OPTIONS_MENU_INPUT) {
        int y = box_y - line_gap + line_h;
        title = "INPUT CONFIG";
        Font_DrawText("PLAYER 1", font, box_w / 4 + box_x, y, FONT_ALIGN_CENTER, 0, 0x80, 0x100, 0x100);
        Font_DrawText("PLAYER 2", font, box_w * 3 / 4 + box_x, y, FONT_ALIGN_CENTER, 0, 0x100, 0x80, 0x80);
    } else if (menu == OPTIONS_MENU_TEAM) {
        title = "TEAM MODE CONFIG";
    }
    oi->title.text_ptr = (intptr_t)title;
    if (oi->title.type != SYSELEM_NONE) {
        SysElem_DrawMotif(&oi->title, 0.0f, 0.0f);
    } else {
        Font_DrawText(title, font, g_targetWidth / 2, box_y - 30, FONT_ALIGN_CENTER, 0, 0x100, 0x100, 0x100);
    }

    /* items: one column, or one per player in the input menu */
    for (col = 0;;) {
        int x0 = col * box_w / 2 + box_x;
        int width_div = (st->columns != 1) + 1;
        int gaps = 0;
        int i;
        for (i = 0; i < st->item_count; i++) {
            char text[0x20];
            int x = x0;
            int y, r, gr, b;
            const char *hint;
            if (st->columns == 2) {
                int half = margin / 2;
                if (col == 0) {
                    half = -half;
                }
                x += half;
            }
            r = gr = b = 0xC0;
            if (Options_HasGap(menu, i)) {
                gaps++;
            }
            y = (i + gaps) * row_h + box_y + line_h;
            if (i == st->cursor[col]) {
                int c = Title_Pulse(screen->ticks * 4, 64, 32);
                last_color = Video_MapRGB(c, c, c);
                g_gfxFillRectColor(x - 5, y - line_h, box_w / width_div + x + 4, y + line_gap, last_color);
                r = 0xF8;
                gr = b = 0x100;
            }
            if (menu == OPTIONS_MENU_INPUT && i == 2 && !g_game->use_joystick[0] && !g_game->use_joystick[1]) {
                /* "Joystick Config" without a joystick */
                gr = b = 0x80;
                r = 0x90;
            }
            Font_DrawText(st->menus[menu][i], font, x, y, FONT_ALIGN_LEFT, 0, r, gr, b);
            text[0] = '\0';
            if (menu == OPTIONS_MENU_MAIN) {
                Options_MainValueText(i, *Options_Value(g_game, i), text);
            } else if (menu == OPTIONS_MENU_INPUT) {
                int v = *Options_InputValue(g_game, col, i);
                if (i == 1) {
                    Options_JoystickText(v, text);
                }
                if (!g_game->use_joystick[col] && v != 0) {
                    gr = b = 0x80;
                    r = 0x90;
                }
            } else if (menu == OPTIONS_MENU_TEAM) {
                int v = *Options_TeamValue(g_game, i);
                if (i == 0) {
                    sprintf(text, "%i%%", v);
                } else if (i == 1) {
                    if (v == 0) {
                        sprintf(text, "Team loses");
                    } else if (v == 1) {
                        sprintf(text, "AI keeps fighting");
                    } else if (v == 2) {
                        sprintf(text, "Control AI");
                    }
                }
            }
            Font_DrawText(text, font, x + value_x, y, FONT_ALIGN_RIGHT, 0, r, gr, b);
            hint = Options_Hint(menu, col, i);
            if (hint != NULL) {
                Font_DrawText(hint, font, x + value_x, y, FONT_ALIGN_RIGHT, 0, 0x100, 0x100, 0x80);
            }
        }
        if (st->columns == 1 || ++col >= 2) {
            break;
        }
    }

    /* prompt: "Reset default values?" or "Options Loaded/Saved" */
    if (st->prompt) {
        GfxLineFn line = (GfxLineFn)g_gfxDrawLine;
        int pw = char_w * 32;
        int ph = line_h * 3;
        int px = (g_targetWidth - pw) / 2;
        int cy = box_h / 2 + box_y;
        int half = ph / 2;
        int ty = line_h / 2 + half + cy - 1;
        int cx = pw / 2 + px;
        int item;
        line((float)px, (float)(cy - 1), (float)(px + pw), (float)(cy - 1), Video_MapRGB(0x38, 0x38, 0x60));
        line((float)px, (float)(cy + ph + 1), (float)(px + pw), (float)(cy + ph + 1), Video_MapRGB(0x38, 0x38, 0x60));
        line((float)(px - 1), (float)cy, (float)(px - 1), (float)(cy + ph), Video_MapRGB(0x38, 0x38, 0x60));
        line((float)(px + pw + 1), (float)cy, (float)(px + pw + 1), (float)(cy + ph), Video_MapRGB(0x38, 0x38, 0x60));
        g_gfxFillRect(px, cy, px + pw, cy + ph, Video_MapRGB(0x54, 0x54, 0x88));
        item = st->cursor[screen->input_side];
        if (item == OPTIONS_DEFAULTS) {
            int no[3] = { 0x100, 0x100, 0x100 };
            int yes[3] = { 0x100, 0x100, 0x100 };
            /* the bar goes from right to left (x1 > x2), as in the original */
            if (st->prompt_yes == 0) {
                yes[0] = yes[1] = yes[2] = 0xC0;
                g_gfxFillRectColor(px - char_w * 5 + pw, ty - line_h, px - char_w * 9 + pw, ty + line_gap,
                                   last_color);
            } else {
                no[0] = no[1] = no[2] = 0xC0;
                g_gfxFillRectColor(px - char_w + pw, ty - line_h, px - char_w * 4 + pw, ty + line_gap, last_color);
            }
            Font_DrawText("Reset default values?", font, char_w * 2 + px, ty, FONT_ALIGN_LEFT, 0, 0x100, 0x100, 0x100);
            Font_DrawText("No", font, px - char_w * 6 + pw, ty, FONT_ALIGN_RIGHT, 0, no[0], no[1], no[2]);
            Font_DrawText("Yes", font, px - char_w * 2 + pw, ty, FONT_ALIGN_RIGHT, 0, yes[0], yes[1], yes[2]);
            return 1;
        }
        if (item == OPTIONS_LOAD_SAVE) {
            Font_DrawText(g_game->option_save == 0 ? "Options Loaded" : "Options Saved", font, cx, ty,
                          FONT_ALIGN_CENTER, 0, 0x100, 0x100, 0x100);
        }
    }
    return 1;
}

/* 0x0042B960: the options screen loop; returns 0 when the state cannot be allocated. */
int Options_Setup(ScreenLoop *screen)
{
    memset(screen, 0, sizeof(*screen));
    screen->scale = 1.0f;
    screen->fadein_time = 10;
    screen->fadeout_time = 10;
    screen->kind = 3;
    screen->fps = 60;
    screen->reload_motif = 1;
    screen->start_frame = g_game->frames;
    screen->state = calloc(1, sizeof(OptionsState));
    if (screen->state == NULL) {
        return 0;
    }
    screen->init = (ScreenFn)Options_Init;
    screen->end = (ScreenFn)Options_End;
    screen->update = (ScreenFn)Options_Update;
    screen->tick = (ScreenFn)Screen_TickNop;
    screen->draw = (ScreenFn)Options_Draw;
    screen->overlay = NULL;
    return 1;
}

/* 0x0042BA00: returns to the title screen, or exits when the loop fails. */
int Options_Run(void)
{
    ScreenLoop screen;

    Log_Printf("Entering options screen.\n");
    Options_Setup(&screen);     /* the result is not checked */
    if (!Screen_Run(&screen)) {
        return GAMEFLOW_EXIT;
    }
    screen.result = screen.done;
    if (screen.state != NULL) {
        free(screen.state);
    }
    return GAMEFLOW_TITLE;
}

/* ---- key / joystick configuration ---- */

/* Text drawn with Font_DrawTextEx at (x, y) and neutral draw parameters. */
static void KeyConfig_Text(const char *text, Font *font, float x, float y, int align, int r, int g, int b)
{
    DrawParams dp;

    DrawParams_Init(&dp);
    dp.x = x;
    dp.y = y;
    Font_DrawTextEx(text, font, &dp, align, 0, r, g, b);
}

/* Name of a configured key or joystick button. */
static void KeyConfig_KeyName(int joystick, int v, int none, char *text)
{
    if (v == none) {
        strcpy(text, "Not used");
    } else if (!joystick) {
        sprintf(text, "%s", v < SDLK_LAST ? SDL_GetKeyName((SDLKey)v) : "");
    } else {
        sprintf(text, "%c-%02i", 'A' + (v >> 5 & 3), v & ~0x60);
    }
}

/*
 * 0x0042BA60: the key (joystick = 0) or joystick button (1) configuration of
 * both players, with its own frame loop at 60 fps over the options screen's
 * background. ticks counts the frames (blinking); the new count is returned.
 * Quirk: the original waits forever when a pad or key queue says a code is
 * pending but its indices are equal (cannot happen).
 */
int KeyConfig_Run(int joystick, int ticks)
{
    GameVars *g = g_game;
    int saved_key_flag = g_keyboardKeepRepeat;
    int saved_rate = g_frameRate;
    int none = joystick ? JOY_NONE : KEY_NONE;
    Font *font = g->options_font;
    int line_h = font->spacing_y + font->size_h;
    int char_w = font->spacing_x + font->size_w;
    int line_gap = line_h / 6;
    int row_h = line_gap + line_h;
    int box_h = row_h * 16 + line_h;
    int value_x = char_w * 14;
    int box_w = char_w * 32;
    int box_x, half_w, box_y, col_x;
    unsigned int bg_color, border_color;
    int *keys[2];
    int cur[2][KEYCONFIG_ENTRIES];
    int backup[2][KEYCONFIG_ENTRIES];
    const char *names[KEYCONFIG_ITEMS];
    char text[0x80];
    int state = 0;          /* 0 browsing, 1 or 2: assigning the entries of that player */
    int player = 0;         /* column of the cursor */
    int item = 0;
    int entry = 0;          /* next entry to assign */
    int p, i;

    bg_color = Video_MapRGB(0x80, 0x60, 0x40);
    border_color = Video_MapRGB(0x40, 0x30, 0x20);
    box_x = (g_targetWidth - box_w) / 2;
    half_w = box_w / 2;
    box_y = line_h + 40;
    col_x = half_w + 15;
    if (!joystick) {
        keys[0] = g_game->keys[0];
        keys[1] = g_game->keys[1];
    } else {
        keys[0] = g_game->joy_keys[0];
        keys[1] = g_game->joy_keys[1];
    }
    for (p = 0; p < 2; p++) {
        for (i = 0; i < KEYCONFIG_ENTRIES; i++) {
            cur[p][i] = keys[p][keyconfig_slots[i]];
        }
    }
    names[0] = "Config all";
    names[1] = "Up";
    names[2] = "Down";
    names[3] = "Left";
    names[4] = "Right";
    names[5] = "A";
    names[6] = "B";
    names[7] = "C";
    names[8] = "X";
    names[9] = "Y";
    names[10] = "Z";
    names[11] = "Start";
    names[12] = "Default";
    names[13] = "Exit";

    Gfx_SetRenderTarget(g_game->screen, 0);
    Timer_SetFps(60);
    Events_Poll();
    g_keyboardQueueWrite = 0;
    g_keyboardQueueRead = 0;
    g_padQueueWrite = 0;
    g_padQueueRead = 0;
    Input_ResetState();
    g_keyQueue.write = 0;
    g_keyQueue.read = 0;
    g_keyboardKeepRepeat = 1;
    SysDraw_Clear(g_game->sysexplods);
    g_game->frame_skip_count = 0;
    g_game->skip_draw = 0;
    Timer_ResetFrame();

    for (;;) {
        int key = 0, pad = 0, button;
        int up = 0, down = 0, left = 0, right = 0, accept = 0;
        void *bg;

        /* input: a key, a joystick button */
        Events_Poll();
        if (g_keyboardQueueRead != g_keyboardQueueWrite) {
            if (!g_keyboardActive) {
                key = 0;
            } else {
                key = g_keyboardQueue[g_keyboardQueueRead];
                if (++g_keyboardQueueRead >= 0x100) {
                    g_keyboardQueueRead = 0;
                }
            }
        }
        if (Pads_CodePending()) {
            if (!g_padsActive) {
                pad = 0;
            } else {
                if (g_padQueueRead == g_padQueueWrite) {
                    for (;;) {
                    }
                }
                pad = g_padQueue[g_padQueueRead];
                if (++g_padQueueRead >= 0x100) {
                    g_padQueueRead = 0;
                }
            }
        }
        g = g_game;
        if (!g->safe_mode) {
            Video_HandleFullscreenKey(&key);
            g = g_game;
        }
        if (Input_CodePending() && (g_padsActive || g_keyboardActive)) {
            /* the menu input queue is not used here: drop one code */
            if (g_keyQueue.read == g_keyQueue.write) {
                for (;;) {
                }
            }
            if (++g_keyQueue.read >= g_keyQueue.capacity) {
                g_keyQueue.read = 0;
            }
        }
        button = pad & ~0x60;
        if (key == SDLK_RETURN || (unsigned int)(button - 5) <= 26) {
            accept = 1;
        }
        if (key == SDLK_UP || button == 1) {
            up = 1;
        }
        if (key == SDLK_DOWN || button == 2) {
            down = 1;
        }
        if (key == SDLK_LEFT || button == 3) {
            left = 1;
        }
        if (key == SDLK_RIGHT || button == 4) {
            right = 1;
        }
        if (state != 0) {
            down = 0;
            up = 0;
            left = 0;
            right = 0;
            accept = 0;
        }
        if (key == SDLK_F1 || key == SDLK_F2) {
            player = key != SDLK_F1;
            item = 0;
            accept = 1;
        }
        if (key == SDLK_F3 || key == SDLK_F4) {
            player = key != SDLK_F3;
            item = KEYCONFIG_DEFAULT;
            accept = 1;
        }
        if ((key == SDLK_ESCAPE && state == 0) || g_quitRequested) {
            item = KEYCONFIG_EXIT;
            accept = 1;
        }
        if (key == SDLK_ESCAPE && state != 0) {
            /* cancel "Config all" */
            item = 0;
            accept = 0;
            state = 0;
            memcpy(cur[player], backup[player], sizeof(cur[player]));
        }
        if (up) {
            if (item <= 0) {
                item = KEYCONFIG_EXIT;
            } else {
                item--;
                if (item >= 1 && item < KEYCONFIG_DEFAULT) {
                    item = 0;
                }
            }
        }
        if (down) {
            if (item >= KEYCONFIG_EXIT) {
                item = 0;
            } else {
                item++;
                if ((unsigned int)(item - 1) <= 10) {
                    item = KEYCONFIG_DEFAULT;
                }
            }
        }
        if (left) {
            player = 0;
        }
        if (right) {
            player = 1;
        }
        if (accept && item == 0) {
            /* "Config all": the following key presses fill the entries */
            memcpy(backup[player], cur[player], sizeof(backup[player]));
            state = player + 1;
            entry = 0;
            g_keyboardKeepRepeat = 0;
            key = 0;
            pad = 0;
        } else if (item == KEYCONFIG_DEFAULT && accept) {
            for (i = 0; i < KEYCONFIG_ENTRIES; i++) {
                cur[player][i] = !joystick ? keyconfig_default_keys[player][i] : keyconfig_default_joy[player][i];
            }
        }
        if (state != 0 && key != SDLK_ESCAPE) {
            int assigned = 0;
            if (!joystick && key != 0 && !(key & 0x200)) {
                cur[state - 1][entry] = key == SDLK_SPACE ? none : key;
                assigned = 1;
            } else if (joystick == 1 && pad != 0 && !(pad & 0x100)) {
                cur[state - 1][entry] = key == SDLK_SPACE ? none : pad;
                assigned = 1;
            } else if (key == SDLK_SPACE) {
                cur[state - 1][entry] = none;
                assigned = 1;
            }
            if (assigned && ++entry >= KEYCONFIG_ENTRIES) {
                state = 0;
                g_keyboardKeepRepeat = 1;
            }
        }

        /* draw */
        bg = g->option_bg;
        Bg_SetCamera(bg, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
        Bg_Tick(g->option_bg);
        if (!g->skip_draw) {
            GfxRectFn rect = (GfxRectFn)g_gfxFillRectSub;
            float scale = (float)g->motif_scale;
            int gaps = 0;
            const char *hint;
            Bg_SetScale(g->option_bg, scale, scale);
            Bg_Draw(g->option_bg, 0);
            {
                int x2 = box_x + box_w + 15;
                int y2 = box_y + box_h;
                rect(box_x - 15, box_y - 14, x2 - 1, y2 - 1, bg_color);
                rect(box_x - 16, box_y - 13, box_x - 16, y2 - 2, border_color);
                rect(x2, box_y - 13, x2, y2 - 2, border_color);
                rect(box_x - 14, box_y - 15, x2 - 2, box_y - 15, border_color);
                rect(box_x - 14, y2, x2 - 2, y2, border_color);
            }
            KeyConfig_Text(joystick ? "JOYSTICK CONFIG" : "KEY CONFIG", font, (float)(g_targetWidth / 2),
                           (float)(box_y - 30), FONT_ALIGN_CENTER, 0x100, 0x100, 0x100);
            {
                float y = (float)(box_y - line_gap + line_h);
                KeyConfig_Text("PLAYER 1", font, (float)(box_w / 4 + box_x), y, FONT_ALIGN_CENTER, 0x80, 0x100,
                               0x100);
                KeyConfig_Text("PLAYER 2", font, (float)(box_w * 3 / 4 + box_x), y, FONT_ALIGN_CENTER, 0x100, 0x80,
                               0x80);
            }
            if (ticks % 450 < 150) {
                strcpy(text, "Press a key to assign to entry");
            } else if (ticks % 450 < 300) {
                strcpy(text, "Press spacebar to disable entry");
            } else {
                strcpy(text, "Press esc to cancel");
            }
            if (state == 1) {
                KeyConfig_Text(text, font, (float)box_x, (float)(row_h * 15 + box_y + line_h), FONT_ALIGN_LEFT,
                               0xC0, 0xC0, 0xC0);
            } else if (state == 2) {
                KeyConfig_Text(text, font, (float)(box_x + box_w), (float)(row_h * 15 + box_y + line_h),
                               FONT_ALIGN_RIGHT, 0xC0, 0xC0, 0xC0);
            }
            for (i = 0; i < KEYCONFIG_ITEMS; i++) {
                int lr = 0xC0, lg = 0xC0, lb = 0xC0;    /* label (player 1) */
                int vr = 0xC0, vg = 0xC0, vb = 0xC0;    /* player 2 */
                int y;
                if (i == 14 || i == KEYCONFIG_DEFAULT || i == 0) {
                    gaps++;
                }
                y = (i + gaps) * row_h + box_y + line_h;
                if (i == item) {
                    unsigned int color;
                    int c = Title_Pulse(ticks * 4, 64, 32);
                    if (state == 0) {
                        color = Video_MapRGB(c, c, c);
                    } else {
                        color = Video_MapRGB(0x60, 0x60, 0x60);
                    }
                    if (i < KEYCONFIG_EXIT) {
                        g_gfxFillRectColor(player * col_x + box_x - 5, y - line_h, player * col_x + box_x + half_w - 15,
                                           y + line_gap, color);
                    } else {
                        g_gfxFillRectColor(box_x - 5, y - line_h, box_x + half_w - 15, y + line_gap, color);
                    }
                    if (player != 0 && item != KEYCONFIG_EXIT) {
                        vr = 0xF8;
                        vg = vb = 0x100;
                    } else {
                        lr = 0xF8;
                        lg = lb = 0x100;
                    }
                }
                if (i > 11 && state != 0) {
                    break;      /* "Default" and "Exit" are hidden while assigning */
                }
                Font_DrawText(names[i], font, box_x, y, FONT_ALIGN_LEFT, 0, lr, lg, lb);
                if (i <= KEYCONFIG_DEFAULT) {
                    Font_DrawText(names[i], font, box_x + col_x, y, FONT_ALIGN_LEFT, 0, vr, vg, vb);
                }
                hint = NULL;
                if (i == 0) {
                    KeyConfig_Text("(F1)", font, (float)(box_x + value_x), (float)y, FONT_ALIGN_RIGHT, 0x100, 0x100,
                                   0x80);
                    hint = "(F2)";
                } else if (i == KEYCONFIG_DEFAULT) {
                    KeyConfig_Text("(F3)", font, (float)(box_x + value_x), (float)y, FONT_ALIGN_RIGHT, 0x100, 0x100,
                                   0x80);
                    hint = "(F4)";
                } else if (i == KEYCONFIG_EXIT) {
                    KeyConfig_Text("(Esc)", font, (float)(box_x + value_x), (float)y, FONT_ALIGN_RIGHT, 0x100, 0x100,
                                   0x80);
                } else {
                    KeyConfig_KeyName(joystick, cur[0][i - 1], none, text);
                    KeyConfig_Text(text, font, (float)(box_x + value_x), (float)y, FONT_ALIGN_RIGHT, lr, lg, lb);
                    KeyConfig_KeyName(joystick, cur[1][i - 1], none, text);
                    KeyConfig_Text(text, font, (float)(box_x + value_x + col_x), (float)y, FONT_ALIGN_RIGHT, vr, vg,
                                   vb);
                }
                if (hint != NULL) {
                    KeyConfig_Text(hint, font, (float)(box_x + value_x + col_x), (float)y, FONT_ALIGN_RIGHT, 0x100,
                                   0x100, 0x80);
                }
            }
            if (state != 0) {
                /* bar on the entry being assigned */
                int y = (entry + 2) * row_h + box_y + line_h;
                int c = Title_Pulse(ticks * 4, 64, 32);
                unsigned int color = Video_MapRGB(c, 0x60 - c, 0x60 - c / 2);
                if (state == 1) {
                    g_gfxFillRectColor(box_x - 5, y - line_h, box_x + half_w - 15, y + line_gap, color);
                } else {
                    g_gfxFillRectColor(box_x + col_x - 5, y - line_h, box_x + half_w + col_x - 15, y + line_gap,
                                       color);
                }
            }
            g = g_game;
            scale = (float)g->motif_scale;
            Bg_SetScale(g->option_bg, scale, scale);
            Bg_Draw(g->option_bg, 1);
        }
        Screen_EndFrame();
        ticks++;
        if (accept && item == KEYCONFIG_EXIT) {
            break;
        }
    }

    g = g_game;
    g->frame_skip_count = 0;
    g->skip_draw = 0;
    Timer_ResetFrame();
    for (p = 0; p < 2; p++) {
        for (i = 0; i < KEYCONFIG_ENTRIES; i++) {
            keys[p][keyconfig_slots[i]] = cur[p][i];
        }
    }
    if (joystick == 1) {
        g = g_game;
        if (g->use_joystick[0]) {
            memcpy(g->joy_keys_active[0], g->joy_keys[0], sizeof(g->joy_keys_active[0]));
        }
        if (g->use_joystick[1]) {
            memcpy(g->joy_keys_active[1], g->joy_keys[1], sizeof(g->joy_keys_active[1]));
        }
    }
    Log_Printf("Setting up input\n");
    Input_Remap();
    g_keyboardKeepRepeat = saved_key_flag;
    Timer_SetFps(saved_rate);
    return ticks;
}

/* ---- training menu (pause screen of training mode) ---- */

#define TRAINING_PAGES 3

/* Items of each page and the number of values of each item. */
static const int training_item_count[TRAINING_PAGES] = { 5, 2, 1 };
static const int training_value_count[TRAINING_PAGES][5] = {
    { 3, 2, 4, 4, 8 },
    { 3, 8 },
    { 3 },
};

/* Values of the page being shown, GameVars.training_coop / ai / manual. */
static int *Training_Values(GameVars *g, int page)
{
    switch (page) {
    case TRAINING_COOPERATIVE:
        return g->training_coop;
    case TRAINING_AI:
        return g->training_ai;
    default:
        return g->training_manual;
    }
}

/*
 * 0x0042D750: key handler of the training menu while the game is paused.
 * The dummy is the player that did not choose training mode, the menu uses
 * the keys of the other one. Z toggles the menu. Returns 1 when a button or
 * escape was pressed.
 */
int Training_HandleKey(int code)
{
    GameVars *g = g_game;
    int page = g->training_page;
    int *values = Training_Values(g, page);
    int result = 0;
    int changed = 0;
    Player *dummy;
    const int *keys;

    if (!g->defocus_paused) {
        return 0;
    }
    if (g->game_mode_side == 1) {
        dummy = g->players[1];          /* P1 is the dummy, P2 plays */
        keys = g->input_keys[1];
    } else {
        dummy = g->players[2];
        keys = g->input_keys[0];
    }
    if (code == Input_TranslateKey(keys[KEY_Z]) && g->team_variant == 3 && g->screen_kind == 4) {
        g->training_menu = g->training_menu == 0;
        return 0;
    }
    if (!g->training_menu) {
        return 0;
    }

    /* read the dummy's settings */
    values[0] = page;
    if (page == TRAINING_COOPERATIVE) {
        values[1] = dummy->dummy_guard;
        values[2] = dummy->dummy_mode;
        values[4] = dummy->dummy_button_jam;
        if (dummy->dummy_dist_max == 0 && dummy->dummy_dist_min == 0) {
            values[3] = 0;
        } else if (dummy->dummy_dist_max == 50) {
            values[3] = 1;
        } else {
            values[3] = (dummy->dummy_dist_max != 150) + 2;
        }
    } else if (page == TRAINING_AI) {
        values[1] = dummy->ai_level - 1;
    }

    if (code == Input_TranslateKey(keys[KEY_LEFT])) {
        if (--g->training_value < 0) {
            g->training_value = training_value_count[page][g->training_cursor] - 1;
        }
        changed = 1;
    } else if (code == Input_TranslateKey(keys[KEY_RIGHT])) {
        if (++g->training_value == training_value_count[page][g->training_cursor]) {
            g->training_value = 0;
        }
        changed = 1;
    } else if (code == Input_TranslateKey(keys[KEY_JUMP])) {
        if (--g->training_cursor < 0) {
            g->training_cursor = training_item_count[page] - 1;
        }
        g->training_value = values[g->training_cursor];
    } else if (code == Input_TranslateKey(keys[KEY_CROUCH])) {
        if (++g->training_cursor >= training_item_count[page]) {
            g->training_cursor = 0;
        }
        g->training_value = values[g->training_cursor];
    } else if (Keys_IsButtonCode(keys, code) || code == SDLK_ESCAPE) {
        result = 1;
    }
    values[g->training_cursor] = g->training_value;

    /* write them back (the page is the one shown when the key came) */
    if (page == TRAINING_MANUAL) {
        dummy->ai = 0;
    } else {
        dummy->ai = 1;
        dummy->dummy_control = page;
    }
    if (page == TRAINING_COOPERATIVE) {
        dummy->dummy_guard = values[1];
        dummy->dummy_mode = values[2];
        dummy->dummy_button_jam = values[4];
        switch (values[3]) {
        case 0:
            dummy->dummy_dist_min = 0;
            dummy->dummy_dist_max = 0;
            break;
        case 1:
            dummy->dummy_dist_max = 50;
            dummy->dummy_dist_min = 0;
            break;
        case 2:
            dummy->dummy_dist_max = 150;
            dummy->dummy_dist_min = 100;
            break;
        case 3:
            dummy->dummy_dist_min = 280;
            dummy->dummy_dist_max = 0;
            break;
        }
    } else if (page == TRAINING_AI) {
        dummy->ai_level = values[1] + 1;
    }
    g->training_page = values[0];

    if (changed) {
        /* release the dummy's held keys */
        InputBuf *in = &dummy->input;
        int k;
        for (k = 1; k <= KEY_START; k++) {
            unsigned int c;
            if (in->keys[k] == 0) {
                continue;
            }
            c = k | 0x10;
            if (in->flipped) {
                if ((c & 0xF) == KEY_LEFT) {
                    c = (c & ~0xDu) | KEY_RIGHT;
                } else if ((c & 0xF) == KEY_RIGHT) {
                    c = (c & ~0xBu) | KEY_LEFT;
                }
            }
            PlayerInput_Push(in, c);
        }
    }
    return result;
}

/* 0x0042DB50: the training menu over the paused game. */
void Training_Draw(void)
{
    static const char *const control[] = { "Cooperative", "AI", "Manual" };
    static const char *const guard[] = { "None", "Auto" };
    static const char *const mode[] = { "Stand", "Crouch", "Jump", "W Jump" };
    static const char *const distance[] = { "Any", "Close", "Medium", "Far" };
    static const char *const jam[] = { "None", "A", "B", "C", "X", "Y", "Z", "Start" };
    static const char *const level[] = { "1", "2", "3", "4", "5", "6", "7", "8" };
    static const char *const *const coop_values[] = { control, guard, mode, distance, jam };
    static const char *const *const ai_values[] = { control, level };
    static const char *const *const manual_values[] = { control };
    static const char *const *const *const page_values[TRAINING_PAGES] = { coop_values, ai_values, manual_values };
    static const char *const coop_labels[] = { "Dummy control", "Guard mode", "Dummy mode", "Distance", "Button jam" };
    static const char *const ai_labels[] = { "Dummy control", "AI Level" };
    static const char *const manual_labels[] = { "Dummy control" };
    static const char *const *const page_labels[TRAINING_PAGES] = { coop_labels, ai_labels, manual_labels };
    int cx = g_targetWidth / 2;
    int lx = cx - 70;
    int right = g_targetWidth - lx + 20;
    int left = lx - 20;
    GfxRectFn frame = (GfxRectFn)g_gfxDrawRect;
    int page = g_game->training_page;
    int *values = Training_Values(g_game, page);
    int n, i, y;

    g_gfxBlendRect(left, 30, right, 150, Video_MapRGB(0, 0, 0x18), 0x100, 0x80);
    frame(left - 1, 29, right + 1, 151, Video_MapRGB(0x50, 0x50, 0x80));
    frame(left, 30, right, 150, Video_MapRGB(0xC0, 0xC0, 0xF0));
    Font_DrawText("Training Menu", g_game->font_medium, cx, 41, FONT_ALIGN_CENTER, 0, 0, 0, 0);
    Font_DrawText("Training Menu", g_game->font_medium, cx - 1, 40, FONT_ALIGN_CENTER, 0, 0x100, 0x100, 0x100);
    n = training_item_count[page];
    values[0] = page;
    for (i = 0, y = 60; i < n; i++, y += 20) {
        const char *label = page_labels[page][i];
        int v;
        KeyConfig_Text(label, g_game->font_medium, (float)(lx + 1), (float)(y + 1), FONT_ALIGN_LEFT, 0, 0, 0);
        v = values[i];
        if (v == -1) {
            KeyConfig_Text(label, g_game->font_medium, (float)lx, (float)(y + 1), FONT_ALIGN_LEFT, 100, 100, 100);
            continue;
        }
        Font_DrawText(page_values[page][i][v], g_game->font_medium, cx + 21, y + 1, FONT_ALIGN_LEFT, 0, 0, 0, 0);
        if (g_game->training_cursor == i) {
            Font_DrawText(label, g_game->font_medium, lx, y, FONT_ALIGN_LEFT, 0, 0x100, 0x100, 0x100);
            Font_DrawText(page_values[page][i][v], g_game->font_medium, cx + 20, y, FONT_ALIGN_LEFT, 0, 0x100, 0x100,
                          0xC8);
        } else {
            Font_DrawText(label, g_game->font_medium, lx, y, FONT_ALIGN_LEFT, 0, 0x80, 0x80, 0x80);
            Font_DrawText(page_values[page][i][v], g_game->font_medium, cx + 20, y, FONT_ALIGN_LEFT, 0, 0x100, 0x100,
                          0);
        }
    }
}
