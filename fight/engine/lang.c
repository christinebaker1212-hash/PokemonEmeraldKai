/*
 * The language code (original function 0x479580, the last of the block
 * before the collision boxes).
 */
#include "mugen.h"

/*
 * 0x00479580: sets g_language to the first three characters of code ("en"
 * with its terminator). Returns 0 and clears it for NULL or "".
 */
int Lang_SetCode(const char *code)
{
    if (code == NULL || code[0] == '\0') {
        g_language[0] = '\0';
        return 0;
    }
    g_language[0] = code[0];
    g_language[1] = code[1];
    g_language[2] = code[2];
    return 1;
}
