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
void MP_UiStyle_WindowGone(u8 windowId);  // (window.c RemoveWindow)
void MP_UiStyle_Framed(u8 windowId, u8 kind);   // menu.c: its frame drawn (1 dialogue, 2 standard) or cleared (0)
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
};
bool8 MP_PartyMenu_GetState(struct MpPartyState *st);

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
u8 MP_Options_RowView(u8 row, const u8 **label, const u8 **names, u8 *cur, u8 *buf, u8 *lbuf);   // (NAMES: 8)

// the renderer (gba_easy_draw.c): BG0's pixels a style draws in place of, this frame (y0 <= line < y1, x0 <= x < x1)
extern s16 gMpUiHideBg0[4];
void MP_UiStyle_PreFrame(void);          // VDraw, before the frame is drawn: what's hidden
void MP_UiStyle_Composite(u16 *frame);   // VDraw, after: the style's pieces over it

#endif // GUARD_MP_UISTYLE_H
