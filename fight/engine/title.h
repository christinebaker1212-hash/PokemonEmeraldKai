#ifndef MUGEN_TITLE_H
#define MUGEN_TITLE_H

/*
 * Title screen (the "mode select" menu of the log), the options screen, the
 * key / joystick configuration loop and the training menu (title.c).
 */

/* ---- system.def [Title Info] and [Option Info] (GameVars.title_info / option_info) ---- */

/* Menu items of the title screen, in the order of TitleInfo.item_names. */
enum {
    TITLE_ITEM_ARCADE = 0,
    TITLE_ITEM_VERSUS = 1,
    TITLE_ITEM_TEAM_ARCADE = 2,
    TITLE_ITEM_TEAM_VERSUS = 3,
    TITLE_ITEM_TEAM_COOP = 4,
    TITLE_ITEM_SURVIVAL = 5,
    TITLE_ITEM_SURVIVAL_COOP = 6,
    TITLE_ITEM_TRAINING = 7,
    TITLE_ITEM_WATCH = 8,
    TITLE_ITEM_OPTIONS = 9,
    TITLE_ITEM_EXIT = 10,
    TITLE_ITEM_COUNT = 11
};

/* [Title Info] (0x1E8 bytes at GameVars + 0x182C). */
typedef struct TitleInfo {
    int fadein_time;                /* 0x000 fadein.time */
    int fadeout_time;               /* 0x004 fadeout.time */
    int move_snd[2];                /* 0x008 cursor.move.snd: group, number */
    int done_snd[2];                /* 0x010 cursor.done.snd */
    int cancel_snd[2];              /* 0x018 cancel.snd */
    int pos_x;                      /* 0x020 menu.pos */
    int pos_y;                      /* 0x024 */
    FontSpec item_font;             /* 0x028 menu.item.font */
    FontSpec item_active_font;      /* 0x044 menu.item.active.font */
    int item_spacing[2];            /* 0x060 menu.item.spacing */
    char item_names[TITLE_ITEM_COUNT][0x20]; /* 0x068 menu.itemname.*; empty: item hidden */
    int window_margins_y[2];        /* 0x1C8 menu.window.margins.y: above, below */
    int window_visible_items;       /* 0x1D0 menu.window.visibleitems */
    int boxcursor_visible;          /* 0x1D4 menu.boxcursor.visible */
    int boxcursor_coords[4];        /* 0x1D8 menu.boxcursor.coords: x1, y1, x2, y2 */
} TitleInfo;
MUGEN_ASSERT_OFFSET(TitleInfo, item_font, 0x028);
MUGEN_ASSERT_OFFSET(TitleInfo, item_names, 0x068);
MUGEN_ASSERT_OFFSET(TitleInfo, window_margins_y, 0x1C8);
MUGEN_ASSERT_OFFSET(TitleInfo, boxcursor_coords, 0x1D8);
MUGEN_ASSERT_SIZE(TitleInfo, 0x1E8);

/* [Option Info] (0xCC bytes at GameVars + 0x1A14). */
typedef struct OptionInfo {
    int fadein_time;                /* 0x00 fadein.time */
    int fadeout_time;               /* 0x04 fadeout.time */
    int move_snd[2];                /* 0x08 cursor.move.snd: group, number */
    int done_snd[2];                /* 0x10 cursor.done.snd */
    int cancel_snd[2];              /* 0x18 cancel.snd */
    SysElem title;                  /* 0x20 title.*; its text_ptr receives the title text (Options_Draw) */
} OptionInfo;
MUGEN_ASSERT_OFFSET(OptionInfo, title, 0x20);
MUGEN_ASSERT_SIZE(OptionInfo, 0xCC);

/* ---- title screen state (0x1078 bytes, calloc'ed by Title_Setup) ---- */

typedef struct TitleState {
    const char *items[TITLE_ITEM_COUNT];    /* 0x0000 TitleInfo.item_names, NULL when empty */
    int show_info;                  /* 0x002C toggle the info box once (first run) */
    int item;                       /* 0x0030 TITLE_ITEM_* under the cursor */
    int chosen;                     /* 0x0034 an item was chosen (or exit / demo) */
    int chosen_side;                /* 0x0038 side that chose it + 1 */
    int scroll;                     /* 0x003C menu scroll offset (drawn) */
    int scroll_target;              /* 0x0040 */
    int box_w;                      /* 0x0044 info box: 600 x 400 */
    int box_h;                      /* 0x0048 */
    char text[0x1000];              /* 0x004C info box text */
    int box_x;                      /* 0x104C */
    int box_y;                      /* 0x1050 current, slides between the two below */
    int box_x_hidden;               /* 0x1054 */
    int box_y_hidden;               /* 0x1058 -405: above the screen */
    int box_x_shown;                /* 0x105C */
    int box_y_shown;                /* 0x1060 centred */
    int box_moving;                 /* 0x1064 the box is visible or moving */
    int box_dir;                    /* 0x1068 1 down, -1 up */
    float box_progress;             /* 0x106C 0 hidden .. 1 shown */
    int box_shown;                  /* 0x1070 */
    int box_y_target;               /* 0x1074 */
} TitleState;
MUGEN_ASSERT_OFFSET(TitleState, show_info, 0x002C);
MUGEN_ASSERT_OFFSET(TitleState, box_w, 0x0044);
MUGEN_ASSERT_OFFSET(TitleState, text, 0x004C);
MUGEN_ASSERT_OFFSET(TitleState, box_x, 0x104C);
MUGEN_ASSERT_OFFSET(TitleState, box_progress, 0x106C);
MUGEN_ASSERT_SIZE(TitleState, 0x1078);

/* ---- options screen state (0xDC bytes, calloc'ed by Options_Setup) ---- */

/* OptionsState.menu */
enum {
    OPTIONS_MENU_MAIN = 0,
    OPTIONS_MENU_TEAM = 1,
    OPTIONS_MENU_INPUT = 2
};

/* Items of the main options menu (index into GameVars.difficulty...). */
enum {
    OPTIONS_DIFFICULTY = 0,
    OPTIONS_LIFE = 1,
    OPTIONS_TIME = 2,
    OPTIONS_GAME_SPEED = 3,
    OPTIONS_MASTER_VOLUME = 4,
    OPTIONS_WAV_VOLUME = 5,
    OPTIONS_BGM_VOLUME = 6,
    OPTIONS_INPUT_CONFIG = 7,
    OPTIONS_TEAM_CONFIG = 8,
    OPTIONS_LOAD_SAVE = 9,
    OPTIONS_DEFAULTS = 10,
    OPTIONS_RETURN = 11,
    OPTIONS_MAIN_COUNT = 12
};

typedef struct OptionsState {
    struct Font *font;              /* 0x00 Options_GetFont */
    int line_h;                     /* 0x04 font height + line spacing */
    int char_w;                     /* 0x08 font width + spacing */
    int line_gap;                   /* 0x0C line_h / 6 */
    int box_x;                      /* 0x10 */
    int box_y;                      /* 0x14 line_h + 40 */
    int box_w;                      /* 0x18 */
    int box_h;                      /* 0x1C */
    int row_h;                      /* 0x20 line_gap + line_h */
    int value_x;                    /* 0x24 value column, from the left of a column */
    int margin;                     /* 0x28 15 */
    unsigned int bg_color;          /* 0x2C */
    unsigned int border_color;      /* 0x30 */
    int skip_draw;                  /* 0x34 the next Options_Draw draws nothing (menu changed) */
    int cursor[2];                  /* 0x38 per side */
    int accept;                     /* 0x40 a button was pressed this frame */
    int accept_side;                /* 0x44 its side + 1 */
    int lock_frames;                /* 0x48 frames whose input is dropped */
    int prompt;                     /* 0x4C a Yes/No prompt or a message is shown */
    int prompt_yes;                 /* 0x50 Yes selected */
    int prompt_done;                /* 0x54 the prompt was answered */
    int item_count;                 /* 0x58 */
    int columns;                    /* 0x5C 1, 2 in the input menu (one per player) */
    int menu;                       /* 0x60 OPTIONS_MENU_* */
    int main_cursor0;               /* 0x64 main menu cursor while in a sub-menu */
    int pad_68[2];                  /* 0x68 never accessed by the game */
    int main_cursor1;               /* 0x70 */
    int pad_74[2];                  /* 0x74 never accessed by the game */
    const char *main_items[OPTIONS_MAIN_COUNT]; /* 0x7C */
    const char *input_items[5];     /* 0xAC */
    const char *team_items[4];      /* 0xC0 */
    const char **menus[3];          /* 0xD0 item names by OPTIONS_MENU_* */
} OptionsState;
MUGEN_ASSERT_OFFSET(OptionsState, skip_draw, 0x34);
MUGEN_ASSERT_OFFSET(OptionsState, item_count, 0x58);
MUGEN_ASSERT_OFFSET(OptionsState, main_cursor1, 0x70);
MUGEN_ASSERT_OFFSET(OptionsState, main_items, 0x7C);
MUGEN_ASSERT_OFFSET(OptionsState, input_items, 0xAC);
MUGEN_ASSERT_OFFSET(OptionsState, team_items, 0xC0);
MUGEN_ASSERT_SIZE(OptionsState, 0xDC);

/* ---- training menu (pause screen of training mode) ---- */

/* Pages of the training menu: GameVars.training_page, also "Dummy control" (item 0 of every page). */
enum {
    TRAINING_COOPERATIVE = 0,
    TRAINING_AI = 1,
    TRAINING_MANUAL = 2
};

/* ---- renderer entry points with known parameters ---- */

/* g_gfxDrawLine: a line between two points. */
typedef void (__cdecl *GfxLineFn)(float x1, float y1, float x2, float y2, unsigned int color);

/* g_gfxPutPixel: one pixel. */
typedef void (__cdecl *GfxPixelFn)(int x, int y, unsigned int color);

#endif /* MUGEN_TITLE_H */
