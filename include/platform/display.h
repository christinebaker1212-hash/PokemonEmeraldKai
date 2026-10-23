// Moemon Kai PC: display settings (fullscreen, scaling, filters, vsync).
// The state lives in the platform layer (src/platform/display.c). Game code
// (e.g. src/display_menu.c) uses these plain C functions; they apply live.
#ifndef GUARD_PLATFORM_DISPLAY_H
#define GUARD_PLATFORM_DISPLAY_H

enum {
    DISPLAY_SCALE_PIXEL_PERFECT, // largest whole-number scale, black bars
    DISPLAY_SCALE_SHARP_FIT,     // fill keeping 3:2, sharp-bilinear
    DISPLAY_SCALE_STRETCH,       // fill the window, sharp-bilinear
    DISPLAY_SCALE_COUNT
};

enum {
    DISPLAY_FILTER_OFF,
    DISPLAY_FILTER_SCANLINES,
    DISPLAY_FILTER_LCD_GRID,
    DISPLAY_FILTER_COUNT
};

#define DISPLAY_MAX_WINDOW_SCALE 10

int Display_GetFullscreen(void);
void Display_SetFullscreen(int on);
void Display_ToggleFullscreen(void);
int Display_GetWindowScale(void);
int Display_GetMaxWindowScale(void);   // largest preset that fits the desktop
void Display_SetWindowScale(int scale);
int Display_GetScaleMode(void);
void Display_SetScaleMode(int mode);
int Display_GetFilter(void);
void Display_SetFilter(int filter);
int Display_IsFilterShown(void);       // 0 if the window is too small for the filter
int Display_GetVSync(void);
void Display_SetVSync(int on);
void Display_SaveSettings(void);

// Moemon Co-op spectating panel (src/mp/mp_spectate.c): up to DISPLAY_PANEL_MAX friends' screens in a row at
// the bottom of the window, each `scale` (1-4) times 240x160 with a label under it. The game fits in the space
// above; a window gets bigger (or the game smaller) so every screen stays in view.
#define DISPLAY_PANEL_MAX 3
#define DISPLAY_PANEL_LABEL_W 240
#define DISPLAY_PANEL_LABEL_H 12
struct DisplayPanelCard
{
    const void *frame;      // 240x160 ABGR1555, like the game's frame
    const void *label;      // DISPLAY_PANEL_LABEL_W x DISPLAY_PANEL_LABEL_H ARGB8888
    unsigned accent;        // 0xRRGGBB: the card's outline
    int frameChanged, labelChanged;
};
void Display_SetPanel(int count, int scale, const struct DisplayPanelCard *cards);   // count 0: no panel
int Display_GetPanelScale(void);   // the scale it's shown at now (less than asked when the screen is too small)

// In-game DISPLAY SETTINGS screen (src/display_menu.c).
// Display_OpenSettingsMenu only creates the menu task; onClose (may be NULL)
// runs after its windows are removed.
void Display_OpenSettingsMenu(void (*onClose)(void));
// Overworld version: locks field controls and unlocks them on close.
void Display_OpenFieldSettingsMenu(void);

#endif // GUARD_PLATFORM_DISPLAY_H
