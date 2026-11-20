#ifndef GUARD_MP_UISTYLE_H
#define GUARD_MP_UISTYLE_H

// Moemon Kai: OPTIONS > DISPLAY > UI STYLE (docs/ui_styles/UI_STYLES.md): the battle's boxes and panels, the field's
// message box and START drawn in RED's, CRYSTAL's, FIRERED's, PLATINUM's or BLACK's own look, from the style's pack
// (tools/ui_styles/build_pack.py), over the frame the game draws (its own boxes hidden there). src/mp/mp_uistyle.c

enum
{
    UISTYLE_KAI,
    UISTYLE_RED,
    UISTYLE_CRYSTAL,
    UISTYLE_FIRERED,
    UISTYLE_PLATINUM,
    UISTYLE_BLACK,
    UISTYLE_COUNT
};

struct Pokemon;

u8 MP_UiStyle_Active(void);              // the style in effect: the one chosen when its pack is there, else KAI
bool8 MP_UiStyle_Available(u8 style);    // its pack is there (KAI always)
const char *MP_UiStyle_Name(u8 style);   // "KAI", "RED"...
const char *MP_UiStyle_Key(u8 style);    // its moemon_coop.ini value: "kai", "red"...
u8 MP_UiStyle_FromKey(const char *key);
void MP_UiStyle_Changed(void);           // OPTIONS changed it (its pack loaded now)

// the game's text printers (text.c AddTextPrinter, window.c FillWindowPixelBuffer): what each window prints
void MP_UiStyle_TextPrinted(u8 windowId, const u8 *str);
void MP_UiStyle_WindowFilled(u8 windowId);
void MP_UiStyle_WindowPicture(u8 windowId);   // (a picture blitted into the window)
extern u8 gMpUiTextBlit;   // (non-zero: the text printer's own blit, a ▼ or a button: not a picture)
void MP_UiStyle_WindowGone(u8 windowId);  // (window.c RemoveWindow)
void MP_UiStyle_TextRun(u8 windowId, const u8 *str, u8 x, u8 y, u8 fontId, u8 fg, u8 letterSpacing, u8 lineSpacing);   // (text.c)
void MP_UiStyle_WindowRectFilled(u8 windowId, u16 x, u16 y, u16 w, u16 h);   // (window.c FillWindowPixelRect)
void MP_UiStyle_WindowShown(u8 windowId, bool8 shown);   // (window.c Put / ClearWindowTilemap)
void MP_UiStyle_Framed(u8 windowId, u8 kind);   // menu.c: its frame drawn (1 dialogue, 2 standard) or cleared (0)
bool8 MP_UiStyle_JournalNext(void);   // the summary's JOURNAL: A turns the style's own pages
void MP_UiStyle_JournalReset(void);
enum { MP_UINAV_COMMAND, MP_UINAV_MOVES, MP_UINAV_PARTY };
// a menu the style lays out other than KAI: its D-pad (NavOwn TRUE), the entry the D-pad in KEYS goes to from CUR
// among the USABLE ones (a bit each), round the far end in WRAP
bool8 MP_UiStyle_NavOwn(u8 kind);
int MP_UiStyle_NavMove(u8 kind, int cur, u16 keys, u32 usable, bool8 wrap);
void MP_UiStyle_BattleBox(u8 x0, u8 y0, u8 x1, u8 y1, bool8 bg1, bool8 clear);   // battle_script_commands.c HandleBattleWindow
const u8 *MP_TextPrinter_Cur(u8 windowId, bool8 *active, u8 *state);   // text.c: where its printer is

// the battle's boxes (battle_interface.c), for every battle
void MP_UiStyle_HudUpdate(u8 battler, struct Pokemon *mon, bool8 hpToo);
void MP_UiStyle_HudBar(u8 battler, s32 hp, s32 maxHp);
void MP_UiStyle_HudExp(u8 battler, s32 exp, s32 expMax);
bool8 MP_UiStyle_DrawsHealthboxes(void);  // the game's healthbox sprites aren't drawn (CreateBattlerHealthboxSprites)
void MP_UiStyle_SetChooser(u8 battler);   // the player's battler choosing an action or move now

// the party screen (party_menu.c MP_PartyMenu_GetState): what UI STYLE draws its party from
struct MpPartyState
{
    u8 layout, menuType, action, msgWin;
    s8 slot, slot2;
    bool8 chooseHalf;
    u8 cancelBall, confirmBall;
    u8 monSprite[6], itemSprite[6], ballSprite[6], statusSprite[6], window[6];
    const u8 *desc[6];   // (a slot's description in place of its HP: ABLE, NOT ABLE...)
    u8 owner[6];         // whose POKéMON: 0 the player's, 1 / 2 a co-op partner's (KAI draws those in the partner colour)
};
bool8 MP_PartyMenu_GetState(struct MpPartyState *st);
bool8 MP_UiStyle_PartyStyled(void);   // (a style draws the party screen now, whatever its layout: it moves as the style's)
bool8 MP_UiStyle_PartyOwnLayout(void);   // (... and has its own Double / Triple Battle party: it moves as KAI's layout)

// OPTIONS (option_menu.c): its tabs or a page's title, its rows, the line under them
struct MpOptionsView
{
    u8 tab, nTabs, rows, sel;
    const u8 *tabs[5];
    bool8 page;           // a page of a row's own (title in TITLE) in place of the tabs
    bool8 plainHint;      // the line under the rows is the screen's own (L / R: TABS ...), nothing about the row
    u8 title[40], hint[64];
};
bool8 MP_Options_View(struct MpOptionsView *v);

// the BAG (item_menu.c): the pocket shown, its rows, the cursor, what the description box says
struct MpBagView
{
    u8 view, location;
    u16 rows, top, cursor;   // (rows: CLOSE BAG's too when it's there; cursor: the row, counted from the first)
    s16 swapFrom;            // a row being moved, or -1
    bool8 closeRow;          // the last row is CLOSE BAG
    const u8 *pocketName, *desc;
    u8 itemSprite;           // the item's icon shown
    u8 own[16], nOwn;        // the BAG's own sprites (its bag, icons, the moving row's line)
    u8 listWin, descWin, nameWin, tmWin[2];
    u8 arrowTask;            // the list's scroll arrows (TASK_NONE: none)
};
bool8 MP_Bag_View(struct MpBagView *v);
const u8 *MP_Bag_RowView(u16 row, s32 *qty, u8 *tag);   // (TAG: 8 bytes)

// the summary (pokemon_summary_screen.c): the POKéMON, the page, choosing a move, the KAI pages' own words
struct MpSummaryView
{
    struct Pokemon *mon;
    u8 page, statsView;              // (PSS_PAGE_*: INFO, SKILLS, BATTLE MOVES, CONTEST MOVES, POKé BALL, JOURNAL)
    bool8 isEgg, choosing, swapping, learning, canRename, canClearEvs, clearAsk;
    u16 newMove;
    u8 moveCursor, moveSwap;         // (the move picked, 4 the new one or CANCEL; the one it moves to)
    u8 monSprite, ballSprite, statusSprite;
    u8 own[32], nOwn;                // the summary's own sprites a style draws its own of (type icons, the move frame)
    struct Sprite *markings;
    u8 ballBox;                      // the POKé BALL page's lines (MP_Summary_BallLine)
    u8 ballIcon;                     // ... its ball's picture (Kai's place for it: from its box's corner, 88, 32)
    const u8 *journal[8];            // the JOURNAL's rows on its page, the sentence each belongs to
    u8 journalEntry[8], journalRows, journalPage, journalPages;
};
bool8 MP_Summary_View(struct MpSummaryView *v);
struct MpSummaryWords
{
    u8 nick[24], species[16], dexNo[8], level[4], ot[12], idNo[8], item[24], nature[16], metPlace[32], metLevel[4];
    u8 memo[160];                    // (KAI's own trainer memo, with its colours)
    bool8 memoPlain;                 // met there at that level by its own trainer (a style may say it its own way)
    bool8 sealed, shiny, hideOT, otFemale;
    const u8 *ability, *abilityDesc;
    u8 type1, type2;
    u8 hp[4], maxHp[4], stat[5][4], statNature[5];   // (ATK DEF SP.ATK SP.DEF SPEED; 1 raised, 2 lowered)
    u16 hpNow, hpMax;
    u8 ivStars[6], ev[6][4];         // (HP first; stars 0-3, 4: a perfect 31)
    u8 exp[8], nextLv[8], ribbons[4];
    s32 expFrac;
    u16 move[5];
    u8 moveName[5][16], pp[5][4], maxPp[5][4], moveType[5], contestType[5];
    u8 power[4], accuracy[4], appeal, jam;
    const u8 *moveDesc, *contestDesc;
};
void MP_Summary_Words(struct MpSummaryWords *w);
const u8 *MP_Summary_BallLine(u8 k, u8 *x, u8 *y);
u8 MP_Summary_BallState(const u8 **effect);
u8 MP_Options_RowView(u8 row, const u8 **label, const u8 **names, u8 *cur, u8 *buf, u8 *lbuf);   // (NAMES: 8)

// the renderer (gba_easy_draw.c): BG0's pixels a style draws in place of, this frame (y0 <= line < y1, x0 <= x < x1)
extern s16 gMpUiHideBg0[4];
// ... and in a battle's transition, only the screen's 8x8 cells where BG0 still shows Kai's message box (NULL: all of
// the rectangle): the transition draws its own pictures on BG0 over the box, and those are never hidden
extern const u8 *gMpUiHideBg0Cells;   // [20][30], 1 = hide
#define MP_UI_HIDE_RECTS 8
extern s16 gMpUiHideRects[MP_UI_HIDE_RECTS][4];   // (and more: a style's pop-ups where Kai's are)
extern u8 gMpUiHideCount;
void MP_UiStyle_PreFrame(void);          // VDraw, before the frame is drawn: what's hidden
void MP_UiStyle_Composite(u16 *frame);   // VDraw, after: the style's pieces over it

bool8 MP_UiStyle_HasIconSet(u8 style, u8 set);   // OPTIONS > DISPLAY > MENU POKéMON: 0 ORIGINAL, 1 EXTENDED
bool8 MP_BattleMoveSwapping(u8 battler, u8 *to);
bool8 MP_BattleChoosingMove(u8 battler);           // battle_controller_player.c: its move menu is up   // battle_controller_player.c: SELECT's move swap, the move it's on

bool8 MP_BattlePic_SceneTint(u16 *tint);   // CRYSTAL's battles, WHITE BG ON: the time of day's light (mp_battlepic.c)

bool8 MP_UiStyle_GbSwitch(void);          // RED / CRYSTAL: the party's SWITCH as theirs (party_menu.c)
void MP_UiStyle_PartyBlank(u8 mask);      // party slots shown blank (bits), the SWITCH's rows cleared
bool8 MP_UiStyle_HoldLine(u8 windowId, const u8 *cur);   // RED / CRYSTAL: the printer waits for A here (text.c)
void MP_UiStyle_HoldDone(u8 windowId);
void MP_UiStyle_PartySlide(u8 slot, u8 step);   // FIRERED / PLATINUM / BLACK: the SWITCH's slide, STEP 8 px out
u8 MP_UiStyle_PartySlideSteps(u8 slot);         // (the steps until the slot has left the screen)
void MP_LegacySfx_PlaySwap(bool8 crystal);

#endif // GUARD_MP_UISTYLE_H
