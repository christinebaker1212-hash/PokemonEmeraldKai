/* The card screens' look (src/tcg/tcg_ui.c), shared with the Yu-Gi-Oh! screens (src/ygo/ygo_ui.c): the green
 * background, the coloured side panel, the white list with the red row frame, the description strip, the button
 * hints, the message window. */
#ifndef MOEMON_TCG_UI_KIT_H
#define MOEMON_TCG_UI_KIT_H
#define KIT_PANEL_W 104
#define KIT_STRIP_Y 112
#define KIT_LIST_X 108
#define KIT_LIST_W 128
#define KIT_ROW_H 16
enum
{
    KIT_GAME_POKEMON,
    KIT_GAME_YUGIOH,
    KIT_GAME_TRIAD
};
unsigned TcgUi_Frame(void);   /* counts up while the card screens are open (the row frame's pulse) */
unsigned TcgUi_Theme(void);
void TcgUi_Background(void);
void TcgUi_SidePanel(const char *title, unsigned color);
/* The CARD CASE's three sides on the side panel's tabs: POKeMON | YU-GI-OH! | TRIPLE TRIAD */
void TcgUi_CaseTabs(unsigned game, unsigned color);
void TcgUi_ListBox(int x, int y, int w, int h);
void TcgUi_Row(int x, int y, int w, int on, int enabled, const char *label, const char *right);
/* The bottom strip; card: a card face drawn at its left by draw (G_Card, Ygo_DrawCard), 0 none */
void TcgUi_Strip(void (*draw)(unsigned, float, float, float, float, double, unsigned, unsigned), unsigned card,
                 const char *text);
void TcgUi_Hints(const char *hint);
void TcgUi_TopBar(const char *title);
/* A counter's tabs down the side panel (5 fit when short), tab selected, and what you spend (balance, what) */
void TcgUi_ShopTabs(const char *const *names, int count, int tab, unsigned fill, unsigned edge, unsigned ink,
                    const char *balance, const char *what, unsigned color);
void TcgUi_MessageBox(const char *message, int asking, int yes);
/* How many lines text wraps to at width in font; text cut to fit width with a "." */
int TcgUi_WrapLines(const char *text, int width, int font);
void TcgUi_FitText(char *text, int width, int font);
/* A small menu (the game's window and arrow) at x, y; rows whose bit is set in disabled are grey */
void TcgUi_PopMenu(int x, int y, int w, const char *const *items, int count, int cursor, unsigned disabled);
int TcgUi_PopMenuMove(unsigned pressed, int cursor, int count, unsigned disabled);
/* The name editor (RENAME): a keyboard on the screen, the real one too. NameStep: 0 typing, 1 done, -1 left */
void TcgUi_NameOpen(const char *title, const char *name);
int TcgUi_NameActive(void);
int TcgUi_NameStep(unsigned pressed);
const char *TcgUi_NameText(void);
void TcgUi_NameDraw(unsigned color);
/* The CARD CASE on its POKeMON side again (from the YU-GI-OH! side's tabs) */
void Tcg_OpenCaseSide(void (*returnToGame)(void), int fromScript);
/* The CARD CASE on any side (KIT_GAME_*): L and R on a side's first screen go round them */
void Tcg_OpenCaseGame(unsigned game, void (*returnToGame)(void), int fromScript);
/* The Pokemon promo card of a special trainer who duels with Yu-Gi-Oh! cards now (the ELITE FOUR), once */
unsigned Tcg_GivePromoOf(unsigned special);
/* Back to the map (or the script that opened the screens) */
void Tcg_CloseScreens(void (*returnToGame)(void), int fromScript);
#endif
