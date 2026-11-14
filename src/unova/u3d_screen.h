#ifndef GUARD_U3D_SCREEN_H
#define GUARD_U3D_SCREEN_H

// UNOVA's own screens (src/unova/u3d_screen.c): each draws at 240x160 with G_* and takes the buttons while open
struct UnovaScreen
{
    const char *name;
    void (*open)(void);
    void (*draw)(void);
    void (*step)(u16 pressed, u16 keys);
};

void Unova_Screen_Open(const struct UnovaScreen *screen);
void Unova_Screen_Close(void (*then)(void));   // THEN: run on the next field frame (NULL: nothing)
bool8 Unova_Screen_IsActive(void);
u32 Unova_Screen_Time(void);
void Unova_Screen_FieldFrame(void);
void Unova_Screen_DrawField(unsigned darken);
void Unova_Screen_Message(const char *line1, const char *line2);
void Unova_Screen_List(int x, int y, int wTiles, const char *const *items, int count, int sel);

void Unova_CGear_Open(void);

#endif // GUARD_U3D_SCREEN_H
