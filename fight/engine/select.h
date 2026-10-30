#ifndef MUGEN_SELECT_H
#define MUGEN_SELECT_H

/*
 * Character select screen: select.def (characters, stages, arcade order),
 * the [Select Info] layout of the motif, the select screen state machine and
 * the palette reservation of selected characters.
 */

/* ---- system.def elements and the motif draw list (module around 0x40FD00-0x412500) ---- */

/* A text font reference ("x.font = font, bank, align, r, g, b"), read by FontSpec_Read. */
typedef struct FontSpec {
    int font;               /* 0x00 0..9, -1 = none */
    int fight_font;         /* 0x04 1: use the fight.def fonts (always 0 here) */
    int bank;               /* 0x08 */
    int align;              /* 0x0C */
    int r;                  /* 0x10 256 = unchanged */
    int g;                  /* 0x14 */
    int b;                  /* 0x18 */
} FontSpec;
MUGEN_ASSERT_SIZE(FontSpec, 0x1C);

/* SysDrawItem.kind */
enum {
    SYSDRAW_ANIM = 1,           /* anim: an animation (explods, sparks, "x.anim" elements) */
    SYSDRAW_SPRITE = 2,         /* sprite, draw_sprites: a sprite ("x.spr" elements) */
    SYSDRAW_TEXT = 3            /* font, text: a text ("x.font" elements) */
};

/*
 * An item of a draw list (explod.c): an explod, a hit spark or dust, or a
 * system.def / fight.def element being drawn.
 */
typedef struct SysDrawItem {
    int used;                   /* 0x000 */
    int kind;                   /* 0x004 SYSDRAW_* */
    int drawn;                  /* 0x008 set when drawn this frame */
    int owner_id;               /* 0x00C explods: Player.id of the owner (NumExplod); elements: SysElem.owner_id */
    int id;                     /* 0x010 explods: explod ID; elements: SysElem.id */
    int pad_014;                /* 0x014 never accessed by the game */
    Coord pos;                  /* 0x018 */
    Coord vel;                  /* 0x040 added to pos every tick (SysDraw_Start) */
    Coord accel;                /* 0x068 added to vel every tick */
    float offset_x;             /* 0x090 x offset (from the bound player, or the element position) */
    float offset_y;             /* 0x094 y offset */
    int pause_movetime[2];      /* 0x098 pausemovetime, supermovetime: ticks it keeps moving in a Pause /
                                   SuperPause (indexed by GameVars.pause_kind) */
    int started;                /* 0x0A0 ticks since it was added */
    int time;                   /* 0x0A4 -1 = forever; freed once it runs out */
    Coord scale;                /* 0x0A8 x, y */
    double angle;               /* 0x0D0 */
    double yangle;              /* 0x0D8 */
    double xangle;              /* 0x0E0 */
    float scale_mul;            /* 0x0E8 scale factor of the scale parameter (fightfx scale for F explods), 1.0 */
    int bind[4];                /* 0x0EC a PlayerRef (char.h): the player the item is bound to */
    int bindtime;               /* 0x0FC ticks it stays bound, -1 forever */
    float unused_100;           /* 0x100 cleared on unbinding with unused_104..10c, never read */
    float unused_104;           /* 0x104 */
    int unused_108;             /* 0x108 */
    int unused_10c;             /* 0x10C */
    int postype;                /* 0x110 POSTYPE_*, 3 for elements */
    int layer;                  /* 0x114 layer: 0 drawn by sprpriority (anim->queue), else on top (ontop: 2) */
    int flip;                   /* 0x118 DRAW_FLIP_* */
    int space;                  /* 0x11C space: 1 screen, 0 stage (moved by the camera) */
    int shadow;                 /* 0x120 0 = no shadow, else its colour (Video_MapRGB) or -1 */
    int removeongethit;         /* 0x124 */
    int ignorehitpause;         /* 0x128 */
    struct SpriteSet *sprites;  /* 0x12C owned sprite set (palettes), freed with the item */
    PalFx *palfx;               /* 0x130 PalFX followed (the owner's), NULL = palfx_own */
    PalFx palfx_own;            /* 0x134 */
    struct PalRemap *remap;     /* 0x15C &remap_own, the owner's, or NULL */
    PalRemap remap_own;         /* 0x160 */
    struct AnimState *anim;     /* 0x1E4 */
    int trans;                  /* 0x1E8 TRANS_* (DrawParams_SetBlend) */
    int alpha_src;              /* 0x1EC 0..256 */
    int alpha_dst;              /* 0x1F0 0..256 */
    struct SpriteSet *draw_sprites; /* 0x1F4 sprites drawn (not owned; sprites holds the owned set) */
    int sprite;                 /* 0x1F8 SYSDRAW_SPRITE: slot of the sprite in draw_sprites, -1 */
    struct Font *font;          /* 0x1FC SYSDRAW_TEXT */
    int font_bank;              /* 0x200 */
    int font_align;             /* 0x204 */
    int font_r;                 /* 0x208 256 = unchanged */
    int font_g;                 /* 0x20C */
    int font_b;                 /* 0x210 */
    char text[0x40];            /* 0x214 */
    const char *text_ptr;       /* 0x254 text drawn instead of text when set (SysElem.text_ptr) */
    int window_x1;              /* 0x258 clip window, -1 = none (window_y2 decides) */
    int window_x2;              /* 0x25C */
    int window_y1;              /* 0x260 */
    int window_y2;              /* 0x264 */
    int textwrap;               /* 0x268 g_fontWrapMode while the text is drawn */
    int pad_26c;                /* 0x26C never accessed by the game */
} SysDrawItem;
MUGEN_ASSERT_OFFSET(SysDrawItem, vel, 0x040);
MUGEN_ASSERT_OFFSET(SysDrawItem, time, 0x0A4);
MUGEN_ASSERT_OFFSET(SysDrawItem, angle, 0x0D0);
MUGEN_ASSERT_OFFSET(SysDrawItem, bind, 0x0EC);
MUGEN_ASSERT_OFFSET(SysDrawItem, postype, 0x110);
MUGEN_ASSERT_OFFSET(SysDrawItem, sprites, 0x12C);
MUGEN_ASSERT_OFFSET(SysDrawItem, palfx_own, 0x134);
MUGEN_ASSERT_OFFSET(SysDrawItem, remap, 0x15C);
MUGEN_ASSERT_OFFSET(SysDrawItem, trans, 0x1E8);
MUGEN_ASSERT_OFFSET(SysDrawItem, font, 0x1FC);
MUGEN_ASSERT_OFFSET(SysDrawItem, text_ptr, 0x254);
MUGEN_ASSERT_OFFSET(SysDrawItem, textwrap, 0x268);
MUGEN_ASSERT_SIZE(SysDrawItem, 0x270);

typedef struct SysDrawList {
    SysDrawItem *items;     /* 0x00 */
    int capacity;           /* 0x04 */
    int count;              /* 0x08 items in use */
} SysDrawList;

/* ---- select.def ---- */

/* GameVars.char_info: one per [Characters] entry with a character (0xE64 bytes). */
typedef struct CharInfo {
    int cache_count;                /* 0x000 players using the cached data (player cache, char.c) */
    int cache_serial;               /* 0x004 when the data became unused (PlayerCache_Release) */
    struct CharData *data;          /* 0x008 loaded character, NULL when not cached */
    char pad_00C[0x20C - 0x00C];    /* 0x00C never accessed by the game */
    char def_file[0x200];           /* 0x20C */
    char dir[0x200];                /* 0x40C */
    char stage[0x200];              /* 0x60C stage of the line ("random" or empty: none) */
    char music[0x200];              /* 0x80C music= */
    int boss;                       /* 0xA0C boss= minus 1, -1 = none */
    int include_stage;              /* 0xA10 3 when the stage goes to the stage list, else 0 */
    int cell;                       /* 0xA14 row * columns + column */
    unsigned int pal_mask;          /* 0xA18 palettes that exist (bit n: pal n+1) */
    int pal_defaults[4];            /* 0xA1C [Info] pal.defaults, 1..12 */
    int pal_keymap[12];             /* 0xA2C [Palette Keymap] a..z2, 1..12 */
    int order;                      /* 0xA5C order= minus 1, -1..29 */
    char intro_storyboard[0x200];   /* 0xA60 [Arcade] intro.storyboard */
    char ending_storyboard[0x200];  /* 0xC60 [Arcade] ending.storyboard */
    unsigned int pal_free;          /* 0xE60 palettes not reserved by a selected copy */
} CharInfo;
MUGEN_ASSERT_OFFSET(CharInfo, def_file, 0x20C);
MUGEN_ASSERT_OFFSET(CharInfo, boss, 0xA0C);
MUGEN_ASSERT_OFFSET(CharInfo, pal_keymap, 0xA2C);
MUGEN_ASSERT_OFFSET(CharInfo, order, 0xA5C);
MUGEN_ASSERT_OFFSET(CharInfo, pal_free, 0xE60);
MUGEN_ASSERT_SIZE(CharInfo, 0xE64);

/* SelectCell.char_index besides an index into GameVars.char_info. */
#define SELECT_CELL_EMPTY   (-1)
#define SELECT_CELL_RANDOM  (-2)

/* One box of the select grid (0x440 bytes). */
typedef struct SelectCell {
    int char_index;                 /* 0x000 SELECT_CELL_* or index into GameVars.char_info */
    int is_zip;                     /* 0x004 the character is in a zip file */
    char name[0x30];                /* 0x008 [Info] displayname / name */
    char def_file[0x200];           /* 0x038 */
    char dir[0x200];                /* 0x238 */
    struct SpriteSet *sprites;      /* 0x438 sprites 9000,0..2 */
    float scale;                    /* 0x43C motif width / character localcoord width */
} SelectCell;
MUGEN_ASSERT_OFFSET(SelectCell, def_file, 0x038);
MUGEN_ASSERT_OFFSET(SelectCell, sprites, 0x438);
MUGEN_ASSERT_SIZE(SelectCell, 0x440);

/* GameVars.select_screen: the stage list ([ExtraStages] and stages included by characters). */
typedef struct StageInfo {
    char path[0x200];               /* 0x000 */
    char name[0x50];                /* 0x200 [Info] displayname / name, else the path */
    int flags;                      /* 0x250 CharInfo.include_stage, 0 for extra stages */
} StageInfo;
MUGEN_ASSERT_SIZE(StageInfo, 0x254);

/* GameVars.char_info_2 and the fields after it: arcade opponents in fight order. */
typedef struct ArcadeOrder {
    int *chars;                     /* 0x00 character indices, ended by -1 */
    int count;                      /* 0x04 */
    int next;                       /* 0x08 next opponent */
    int wins;                       /* 0x0C cleared; counts the matches won (Gameflow_Run), never read */
} ArcadeOrder;
MUGEN_ASSERT_SIZE(ArcadeOrder, 0x10);

/* ---- [Select Info] of system.def (GameVars.select_info) ---- */

/* Team menu of one side ("p1.teammenu.*"), 0x49C bytes. */
typedef struct TeamMenuLayout {
    int pos_x;                      /* 0x000 p<n>.teammenu.pos */
    int pos_y;                      /* 0x004 */
    SysElem bg;                     /* 0x008 */
    SysElem self_title;             /* 0x0B4 */
    SysElem enemy_title;            /* 0x160 */
    int move_snd[2];                /* 0x20C group, number */
    int value_snd[2];               /* 0x214 */
    int done_snd[2];                /* 0x21C */
    int item_offset[2];             /* 0x224 */
    FontSpec item_font;             /* 0x22C */
    FontSpec item_active_font[2];   /* 0x248 active, active2 (blinking) */
    int item_spacing[2];            /* 0x280 */
    SysElem item_cursor;            /* 0x288 */
    SysElem value_icon;             /* 0x334 */
    int value_spacing[2];           /* 0x3E0 */
    SysElem value_empty_icon;       /* 0x3E8 */
    int saved_item;                 /* 0x494 menu item chosen last time, -1 = none */
    int saved_value;                /* 0x498 its value */
} TeamMenuLayout;
MUGEN_ASSERT_OFFSET(TeamMenuLayout, move_snd, 0x20C);
MUGEN_ASSERT_OFFSET(TeamMenuLayout, item_font, 0x22C);
MUGEN_ASSERT_OFFSET(TeamMenuLayout, item_cursor, 0x288);
MUGEN_ASSERT_OFFSET(TeamMenuLayout, value_empty_icon, 0x3E8);
MUGEN_ASSERT_SIZE(TeamMenuLayout, 0x49C);

/* Face and name of one side ("p1.face", "p1.name.*"), 0xD8 bytes. */
typedef struct FaceLayout {
    SysElem face;                   /* 0x00 */
    FontSpec name_font;             /* 0xAC */
    int name_offset[2];             /* 0xC8 */
    int name_spacing[2];            /* 0xD0 */
} FaceLayout;
MUGEN_ASSERT_SIZE(FaceLayout, 0xD8);

typedef struct SelectInfo {
    char def_file[0x200];           /* 0x000 select.def ([Files] select), resolved by Select_OpenDef */
    int fadein_time;                /* 0x200 */
    int fadeout_time;               /* 0x204 */
    int pos_x;                      /* 0x208 pos */
    int pos_y;                      /* 0x20C */
    int rows;                       /* 0x210 */
    int columns;                    /* 0x214 */
    int cell_spacing;               /* 0x218 */
    int cell_size[2];               /* 0x21C */
    int wrapping;                   /* 0x224 */
    int show_empty_boxes;           /* 0x228 */
    int move_over_empty_boxes;      /* 0x22C */
    SysElem portrait;               /* 0x230 */
    int start_row[2];               /* 0x2DC p<n>.cursor.startcell */
    int start_col[2];               /* 0x2E4 */
    int last_row[2][4];             /* 0x2EC cursor of each team member when last selected, -1 = none */
    int last_col[2][4];             /* 0x30C */
    int p2_cursor_blink;            /* 0x32C */
    int cursor_move_snd[2][2];      /* 0x330 [group p1, p2], [number p1, p2] */
    int cursor_done_snd[2][2];      /* 0x340 */
    int random_move_snd[2][2];      /* 0x350 */
    int random_move_snd_cancel;     /* 0x360 */
    int stage_move_snd[2];          /* 0x364 */
    int stage_done_snd[2];          /* 0x36C */
    int cancel_snd[2];              /* 0x374 */
    int title_offset[2];            /* 0x37C */
    FontSpec title_font;            /* 0x384 */
    SysElem cell_bg;                /* 0x3A0 */
    SysElem cell_random;            /* 0x44C */
    int cell_random_switchtime;     /* 0x4F8 */
    SysElem cursor_active[2];       /* 0x4FC */
    SysElem cursor_done[2];         /* 0x654 */
    int stage_pos[2];               /* 0x7AC */
    FontSpec stage_active_font[2];  /* 0x7B4 active, active2 (blinking) */
    FontSpec stage_done_font;       /* 0x7EC */
    FaceLayout faces[2];            /* 0x808 */
    int teammenu_wrapping;          /* 0x9B8 teammenu.move.wrapping */
    TeamMenuLayout teammenu[2];     /* 0x9BC */
    SysElem sysexplod0;             /* 0x12F4 */
    SelectCell *cells;              /* 0x13A0 rows * columns */
} SelectInfo;
MUGEN_ASSERT_OFFSET(SelectInfo, fadein_time, 0x200);
MUGEN_ASSERT_OFFSET(SelectInfo, wrapping, 0x224);
MUGEN_ASSERT_OFFSET(SelectInfo, portrait, 0x230);
MUGEN_ASSERT_OFFSET(SelectInfo, start_row, 0x2DC);
MUGEN_ASSERT_OFFSET(SelectInfo, last_col, 0x30C);
MUGEN_ASSERT_OFFSET(SelectInfo, cursor_move_snd, 0x330);
MUGEN_ASSERT_OFFSET(SelectInfo, random_move_snd_cancel, 0x360);
MUGEN_ASSERT_OFFSET(SelectInfo, title_font, 0x384);
MUGEN_ASSERT_OFFSET(SelectInfo, cell_bg, 0x3A0);
MUGEN_ASSERT_OFFSET(SelectInfo, cell_random_switchtime, 0x4F8);
MUGEN_ASSERT_OFFSET(SelectInfo, cursor_done, 0x654);
MUGEN_ASSERT_OFFSET(SelectInfo, stage_pos, 0x7AC);
MUGEN_ASSERT_OFFSET(SelectInfo, faces, 0x808);
MUGEN_ASSERT_OFFSET(SelectInfo, teammenu, 0x9BC);
MUGEN_ASSERT_OFFSET(SelectInfo, sysexplod0, 0x12F4);
MUGEN_ASSERT_SIZE(SelectInfo, 0x13A4);

/* ---- select screen state ---- */

/* GameVars.game_mode */
enum {
    GAME_MODE_ARCADE = 0,
    GAME_MODE_VERSUS = 1,
    GAME_MODE_TEAM_ARCADE = 2,
    GAME_MODE_TEAM_VERSUS = 3,
    GAME_MODE_TEAM_COOP = 4,
    GAME_MODE_SURVIVAL = 5,
    GAME_MODE_SURVIVAL_COOP = 6,
    GAME_MODE_TRAINING = 7,
    GAME_MODE_WATCH = 8
};

/* CharselStep.type */
enum {
    CHARSEL_STEP_NONE = 0,
    CHARSEL_STEP_TEAM_MENU = 1,     /* choose single / simul / tag / turns */
    CHARSEL_STEP_CHARS = 2,         /* select the characters of a team */
    CHARSEL_STEP_AUTO_TEAM = 3,     /* fill a team without input */
    CHARSEL_STEP_STAGE = 4,         /* select the stage */
    CHARSEL_STEP_DONE = 5,
    CHARSEL_STEP_WAIT = 6
};

/* TeamMember.mode, TeamMenuItem.mode */
enum {
    TEAM_SINGLE = 0,
    TEAM_SIMUL = 1,
    TEAM_TAG = 2,                   /* "Not yet implemented" */
    TEAM_TURNS = 3
};

/* One step of a side's select script (0x28 bytes). */
typedef struct CharselStep {
    int type;                       /* 0x00 CHARSEL_STEP_* */
    int team;                       /* 0x04 team filled by the step */
    int mode;                       /* 0x08 TEAM_* */
    int count;                      /* 0x0C characters to select */
    int simul_mode;                 /* 0x10 simul with 2: 0/2 both by one player, 1 swapped sides */
    int menu_type;                  /* 0x14 team menu: must be 0 */
    unsigned int menu_flags;        /* 0x18 team menu: 1 single, 2 simul, 4 tag, 8 turns */
    int menu_item;                  /* 0x1C team menu: initial item */
    int player;                     /* 0x20 team menu: side controlling it */
    struct CharselStep *target;     /* 0x24 team menu: step that receives the choice */
} CharselStep;
MUGEN_ASSERT_SIZE(CharselStep, 0x28);

/* Select script of one side (0x1A0 bytes). */
typedef struct CharselScript {
    CharselStep steps[10];          /* 0x000 */
    int count;                      /* 0x190 */
    int current;                    /* 0x194 */
    int entered;                    /* 0x198 the current step was just entered */
    int ticks;                      /* 0x19C ticks in the current step */
} CharselScript;
MUGEN_ASSERT_SIZE(CharselScript, 0x1A0);

typedef struct TeamSlot {
    int player;                     /* 0x00 player number: side + 1 + 2 * slot */
    int char_index;                 /* 0x04 -1 = not selected yet */
    int pal;                        /* 0x08 0-based palette, -1 = none */
    int side;                       /* 0x0C side that selected it */
    int ai;                         /* 0x10 1 = computer controlled */
} TeamSlot;
MUGEN_ASSERT_SIZE(TeamSlot, 0x14);

/* One member of a team: one character, or several for simul (0x5C bytes). */
typedef struct TeamMember {
    int mode;                       /* 0x00 TEAM_* */
    int simul_mode;                 /* 0x04 */
    int count;                      /* 0x08 slots in use */
    TeamSlot slots[4];              /* 0x0C */
} TeamMember;
MUGEN_ASSERT_SIZE(TeamMember, 0x5C);

/* Persistent variables of a player between rounds (TeamSel.member_data entries). */
#define PERSIST_INT_COUNT   60
#define PERSIST_FLOAT_COUNT 40
typedef struct PersistVars {
    int var[PERSIST_INT_COUNT];         /* 0x000 */
    float fvar[PERSIST_FLOAT_COUNT];    /* 0x0F0 */
} PersistVars;
MUGEN_ASSERT_SIZE(PersistVars, 0x190);
/* What a team keeps for each player between rounds and matches (TeamSel.member_data). */
typedef struct TeamMemberData {
    PersistVars vars;                   /* 0x000 */
    int victory_quote;                  /* 0x190 VictoryQuote index, -1 random */
} TeamMemberData;
MUGEN_ASSERT_SIZE(TeamMemberData, 0x194);
/* Stride of the persist var entries in TeamSel. */
#define PERSIST_ENTRY_SIZE  0x194

/* A team (0x1ADC bytes); GameVars.teams holds the result of the select screen. */
typedef struct TeamSel {
    int cur_member;                 /* 0x000 index into members of the member fighting (turns order) */
    int member_count;               /* 0x004 members (current member while selecting) */
    int char_count;                 /* 0x008 characters selected */
    TeamMember members[4];          /* 0x00C */
    TeamMemberData member_data[4][4];   /* 0x17C by member and slot (Player.team_member, team_slot); survival moves
                                           member 1 to 0 for the next match */
    int recovery_base;              /* 0x1ABC turns: life recovered after a won round, in 1/1000 (Team_LifeRecovery; 125 in team matches) */
    float recovery_rate;            /* 0x1AC0 plus this times the round time left (275.0) */
    float recovery_weight;          /* 0x1AC4 weight of the opponents' remaining life in it (0.6) */
    int life_reset;                 /* 0x1AC8 life is reset every round (else it carries over, Round_Reset) */
    float life_mul;                 /* 0x1ACC multiplier of the life of the members (Team_ResetLife) */
    int wins_needed;                /* 0x1AD0 rounds to win the match ([Round] match.wins, members of the other team) */
    int draws_max;                  /* 0x1AD4 [Round] match.maxdrawgames */
    float life_ratio;               /* 0x1AD8 mean life / lifemax of the members at the round start (Round_Reset) */
} TeamSel;
MUGEN_ASSERT_OFFSET(TeamSel, recovery_base, 0x1ABC);
MUGEN_ASSERT_OFFSET(TeamSel, life_ratio, 0x1AD8);
MUGEN_ASSERT_OFFSET(TeamSel, members, 0x00C);
MUGEN_ASSERT_OFFSET(TeamSel, member_data, 0x17C);
MUGEN_ASSERT_SIZE(TeamSel, 0x1ADC);

typedef struct CursorMember {
    int team;                       /* 0x00 */
    int ai;                         /* 0x04 */
    int side;                       /* 0x08 */
    int start_col;                  /* 0x0C cursor position for this member, -1 = keep */
    int start_row;                  /* 0x10 */
} CursorMember;
MUGEN_ASSERT_SIZE(CursorMember, 0x14);

/* Character cursor of one side (0x74 bytes). */
typedef struct CharselCursor {
    int team;                       /* 0x00 */
    int col;                        /* 0x04 */
    int row;                        /* 0x08 */
    int sel_col;                    /* 0x0C cell actually selected (differs on random cells) */
    int sel_row;                    /* 0x10 */
    int slot;                       /* 0x14 characters selected so far */
    int count;                      /* 0x18 characters to select */
    int auto_timer;                 /* 0x1C demo mode: ticks before the computer selects, -1 = none */
    CharselStep *step;              /* 0x20 */
    CursorMember members[4];        /* 0x24 */
} CharselCursor;
MUGEN_ASSERT_SIZE(CharselCursor, 0x74);

/* CharselBox.state */
enum {
    CHARSEL_BOX_NONE = 0,
    CHARSEL_BOX_ACTIVE = 1,
    CHARSEL_BOX_DONE = 2
};

/* Cursor box drawn on the grid for one slot of one side (0x30 bytes). */
typedef struct CharselBox {
    float x;                        /* 0x00 drawn position, -FLT_MAX = not drawn yet */
    float y;                        /* 0x04 */
    float target_x;                 /* 0x08 */
    float target_y;                 /* 0x0C */
    int col;                        /* 0x10 */
    int row;                        /* 0x14 */
    int cell;                       /* 0x18 row * columns + col, -1 */
    int sel_col;                    /* 0x1C */
    int sel_row;                    /* 0x20 */
    int sel_cell;                   /* 0x24 */
    int state;                      /* 0x28 CHARSEL_BOX_* */
    int side;                       /* 0x2C */
} CharselBox;
MUGEN_ASSERT_SIZE(CharselBox, 0x30);

/* Flash of a selected cell (0x10 bytes). */
typedef struct CharselFlash {
    int col;                        /* 0x00 */
    int row;                        /* 0x04 */
    int ticks;                      /* 0x08 counts down */
    int length;                     /* 0x0C */
} CharselFlash;

typedef struct TeamMenuItem {
    int mode;                       /* 0x00 TEAM_* */
    const char *name;               /* 0x04 */
    int value;                      /* 0x08 characters (simul, tag, turns) */
    int min;                        /* 0x0C */
    int max;                        /* 0x10 */
} TeamMenuItem;
MUGEN_ASSERT_SIZE(TeamMenuItem, 0x14);

/* Team menu of one side (0xB4 bytes). */
typedef struct TeamMenu {
    int active;                     /* 0x00 */
    int type;                       /* 0x04 must be 0 */
    int player;                     /* 0x08 side using the menu (layout) */
    int item;                       /* 0x0C */
    int count;                      /* 0x10 */
    TeamMenuItem items[8];          /* 0x14 */
} TeamMenu;
MUGEN_ASSERT_SIZE(TeamMenu, 0xB4);

/* State of the select screen (0x3D14 bytes, calloc'ed by Charsel_Setup). */
typedef struct CharselState {
    CharselScript scripts[2];       /* 0x0000 per side */
    int pad_340[2];                 /* 0x0340 never accessed by the game */
    TeamSel teams[2];               /* 0x0348 */
    int team_done[2];               /* 0x3900 */
    int stage;                      /* 0x3908 selected stage, -1 = random */
    int stage_done;                 /* 0x390C */
    CharselCursor cursors[2];       /* 0x3910 */
    int phase;                      /* 0x39F8 1 = both sides done, 2 = flashes over */
    int phase_ticks;                /* 0x39FC */
    CharselBox boxes[2][4];         /* 0x3A00 [team][slot] */
    CharselFlash flash[2];          /* 0x3B80 */
    int stage_select_shown;         /* 0x3BA0 */
    TeamMenu menus[2];              /* 0x3BA4 */
    int side_done[2];               /* 0x3D0C */
} CharselState;
MUGEN_ASSERT_OFFSET(CharselState, teams, 0x0348);
MUGEN_ASSERT_OFFSET(CharselState, team_done, 0x3900);
MUGEN_ASSERT_OFFSET(CharselState, cursors, 0x3910);
MUGEN_ASSERT_OFFSET(CharselState, phase, 0x39F8);
MUGEN_ASSERT_OFFSET(CharselState, boxes, 0x3A00);
MUGEN_ASSERT_OFFSET(CharselState, flash, 0x3B80);
MUGEN_ASSERT_OFFSET(CharselState, menus, 0x3BA4);
MUGEN_ASSERT_OFFSET(CharselState, side_done, 0x3D0C);
MUGEN_ASSERT_SIZE(CharselState, 0x3D14);

/* ---- generic screen loop (run by Screen_Run at 0x40E7C0) ---- */

struct ScreenLoop;
typedef int (__cdecl *ScreenFn)(struct ScreenLoop *screen, void *state);

/* Input bits of ScreenLoop.input (menu directions and buttons). */
#define SCREEN_IN_UP        0x001
#define SCREEN_IN_RIGHT     0x002
#define SCREEN_IN_DOWN      0x004
#define SCREEN_IN_LEFT      0x008
#define SCREEN_IN_BUTTONS   0x7F0   /* a, b, c, x, y, z, start */
#define SCREEN_IN_START     0x200

typedef struct ScreenLoop {
    int kind;                       /* 0x00 SCREEN_KIND_* (2 for the select screen), copied to GameVars.screen_kind */
    void *state;                    /* 0x04 */
    int palgroup_masks[5];          /* 0x08 palette group masks the AllPalFX darkens (PalGroups_Apply) */
    int fps;                        /* 0x1C frames per second of the screen (60; the match: game speed) */
    int inner;                      /* 0x20 never set: an inner screen would keep the game state (no Game_LoopInit) and the music */
    int reload_motif;               /* 0x24 Screen_Init reloads the motif (System_ReloadMotif) and stops there */
    int loads_match;                /* 0x28 the screen loads the match assets (Game_LoopInit) and unloads them */
    int fadein_time;                /* 0x2C 10 */
    int fadeout_time;               /* 0x30 10 */
    void *bg;                       /* 0x34 background drawn behind and over the screen (system.def *BG) */
    float scale;                    /* 0x38 its scale */
    void *bgm;                      /* 0x3C BGM to play (Screen_Run: "Could not load BGM"), or NULL */
    intptr_t cutscene;              /* 0x40 a Cutscene * (storyboard screens), as an int (Moemon Fight: pointer-sized) */
    int fade_out;                   /* 0x44 fade out started */
    int fade_ticks;                 /* 0x48 */
    intptr_t key_table;             /* 0x4C menu key table of the frame (g_screenKeys, as an int; Input_FindKey; Moemon Fight: pointer-sized) */
    int key_count;                  /* 0x50 its entries (2) */
    int key;                        /* 0x54 key pressed this frame (27 = escape) */
    unsigned int input2;            /* 0x58 */
    int key_code;                   /* 0x5C keyboard code of the frame (Screen_Run) */
    unsigned int input;             /* 0x60 SCREEN_IN_* of this frame */
    int input_side;                 /* 0x64 side the input came from */
    int done;                       /* 0x68 the screen ends (escape / quit) */
    int ticks;                      /* 0x6C */
    int idle_ticks;                 /* 0x70 ticks since the last input (counted by Screen_Run) */
    int start_ticks;                /* 0x74 Timer_GetTicks() when the loop started (frame stats) */
    int end_ticks;                  /* 0x78 Timer_GetTicks() when it ended */
    unsigned int start_frame;       /* 0x7C GameVars.frames when set up */
    int end_frame;                  /* 0x80 GameVars.frames when it ended */
    int skipped_frames;             /* 0x84 frames not drawn */
    int result;                     /* 0x88 the title / options screens copy done here before freeing */
    int pad_8c;                     /* 0x8C never accessed by the game */
    ScreenFn init;                  /* 0x90 */
    ScreenFn end;                   /* 0x94 */
    ScreenFn update;                /* 0x98 */
    ScreenFn tick;                  /* 0x9C */
    ScreenFn draw;                  /* 0xA0 */
    ScreenFn overlay;               /* 0xA4 called after drawing a frame (fade bars included), with state */
} ScreenLoop;
MUGEN_ASSERT_OFFSET(ScreenLoop, bg, 0x34);
MUGEN_ASSERT_OFFSET(ScreenLoop, key, 0x54);
MUGEN_ASSERT_OFFSET(ScreenLoop, done, 0x68);
MUGEN_ASSERT_OFFSET(ScreenLoop, start_frame, 0x7C);
MUGEN_ASSERT_OFFSET(ScreenLoop, init, 0x90);
MUGEN_ASSERT_SIZE(ScreenLoop, 0xA8);

/* Value of g_gfxFillRectColor (renderer dependent). */
typedef void (__cdecl *GfxFillRectFn)(int x1, int y1, int x2, int y2, unsigned int color);

#endif /* MUGEN_SELECT_H */
