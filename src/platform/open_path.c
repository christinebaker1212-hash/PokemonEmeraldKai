// Moemon Co-op: open a file or folder next to the game with the system's
// default program (browser for the Sprite Editor, file manager for folders).
#ifdef PLATFORM_SDL2
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>
#ifndef _WIN32
#include <limits.h>
#endif

int Platform_OpenPath(const char *relPath)
{
    char full[1024], url[2048];
    const char *p;
    int n = 0;

#ifdef _WIN32
    if (!_fullpath(full, relPath, sizeof(full)))
        return -1;
#else
    if (!realpath(relPath, full))
        return -1;
#endif
    n = snprintf(url, sizeof(url), "file://%s", full[0] == '/' ? "" : "/");
    for (p = full; *p && n < (int)sizeof(url) - 4; p++)
    {
        unsigned char c = (unsigned char)*p;
        if (c == '\\')
            url[n++] = '/';
        else if (c == ' ' || c == '#' || c == '%' || c >= 0x80)
            n += snprintf(url + n, sizeof(url) - n, "%%%02X", c);
        else
            url[n++] = c;
    }
    url[n] = 0;
    printf("[mp] opening %s\n", url);
    if (getenv("MOEMON_TEST"))
        return 0; // never launch programs from automated tests
    return SDL_OpenURL(url);
}
#endif
