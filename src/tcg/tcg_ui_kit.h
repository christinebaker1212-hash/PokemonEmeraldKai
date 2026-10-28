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
    KIT_GAME_YUGIOH
};
unsigned TcgUi_Frame(void);   /* counts up while the card screens are open (the row frame's pulse) */
unsigned TcgUi_Theme(void);
void TcgUi_Background(void);
void TcgUi_SidePanel(const char *title, unsigned color);
/* The CARD CASE's two sides on the side panel's tab: POKeMON | YU-GI-OH! */
void TcgUi_CaseTabs(unsigned game, unsigned color);
void TcgUi_ListBox(int x, int y, int w, int h);
void TcgUi_Row(int x, int y, int w, int on, int enabled, const char *label, const char *right);
/* The bottom strip; card: a card face drawn at its left by draw (G_Card, Ygo_DrawCard), 0 none */
void TcgUi_Strip(void (*draw)(unsigned, float, float, float, float, double, unsigned, unsigned), unsigned card,
                 const char *text);
void TcgUi_Hints(const char *hint);
void TcgUi_TopBar(const char *title);
void TcgUi_MessageBox(const char *message, int asking, int yes);
/* The CARD CASE on its POKeMON side again (from the YU-GI-OH! side's tabs) */
void Tcg_OpenCaseSide(void (*returnToGame)(void), int fromScript);
/* The Pokemon promo card of a special trainer who duels with Yu-Gi-Oh! cards now (the ELITE FOUR), once */
unsigned Tcg_GivePromoOf(unsigned special);
/* Back to the map (or the script that opened the screens) */
void Tcg_CloseScreens(void (*returnToGame)(void), int fromScript);
#endif
