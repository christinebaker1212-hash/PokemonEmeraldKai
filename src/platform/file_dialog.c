// Moemon Co-op: the system's own Open / Save As windows, for files the player picks themselves (Yu-Gi-Oh! decks as
// .ydk files). Windows only: elsewhere Platform_HasFileDialog() is 0 and the screens don't offer it.
#ifdef PLATFORM_SDL2
#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>
#endif

int Platform_HasFileDialog(void)
{
#ifdef _WIN32
    return 1;
#else
    return 0;
#endif
}

// save: Save As (else Open); filter: "Name\0*.ext\0" pairs ending in \0\0; defExt: "ydk". Returns 1 with path set.
int Platform_FileDialog(int save, const char *title, const char *filter, const char *defExt, char *path, int size)
{
#ifdef _WIN32
    OPENFILENAMEA ofn;
    BOOL ok;
    memset(&ofn, 0, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = GetActiveWindow();
    ofn.lpstrFilter = filter;
    ofn.lpstrFile = path;
    ofn.nMaxFile = (DWORD)size;
    ofn.lpstrTitle = title;
    ofn.lpstrDefExt = defExt;
    ofn.Flags = OFN_NOCHANGEDIR | (save ? OFN_OVERWRITEPROMPT : OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST);
    ok = save ? GetSaveFileNameA(&ofn) : GetOpenFileNameA(&ofn);
    printf("[mp] file window: %s\n", ok ? path : "closed");
    return ok ? 1 : 0;
#else
    (void)save;
    (void)title;
    (void)filter;
    (void)defExt;
    (void)path;
    (void)size;
    return 0;
#endif
}
#endif
