// Moemon Kai: UNOVA's words (docs/UNOVA_HANDOFF.md). Black's own message banks in the game's character set come in
// unova/black_text.pak beside the game (tools/unova/build_text.py, made from the user's Black ROM when the release is
// built; nothing of it is in this repository). An UNOVA script's text is a placeholder naming a bank and a message
// (PLACEHOLDER_ID_UNOVA_TEXT, 4 bytes of 7 bits); Black's word slots 3 and up are PLACEHOLDER_ID_UNOVA_VAR + slot.
// StringExpandPlaceholders asks here for both. Its own code, never SINNOH's (s3d_text.c) or ORRE's (orre_text.c).
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "string_util.h"
#include "mp.h"
#include "u3d.h"
#include "constants/characters.h"

const char *Updater_ExeFile(const char *name);
int uncompress(unsigned char *dest, unsigned long *destLen, const unsigned char *source, unsigned long sourceLen);

u8 gUnovaStrVar[UNOVA_STR_VARS][64] = {[0 ... UNOVA_STR_VARS - 1] = {EOS}};   // (empty: an EOS, never the 0 that is a space)

static u8 *sText;
static unsigned long sTextSize;
static bool8 sTried;
static const u8 sMissing[] = {0xC7, 0xD9, 0xE7, 0xE7, 0xD5, 0xDB, 0xD9, 0x00, 0xE1, 0xDD, 0xE7, 0xE7, 0xDD, 0xE2, 0xDB, EOS};   // "Message missing"

static u32 Get32(unsigned long at)
{
    if (at + 4 > sTextSize)
        return 0;
    return sText[at] | sText[at + 1] << 8 | sText[at + 2] << 16 | (u32)sText[at + 3] << 24;
}

static void Load(void)
{
    const char *path = getenv("MOEMON_U3D_TEXT");
    FILE *f;
    long size;
    u8 *data;
    sTried = TRUE;
    if (path == NULL)
        path = Updater_ExeFile("unova/black_text.pak");
    if ((f = fopen(path, "rb")) == NULL)
        return;
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);
    data = malloc(size > 0 ? size : 1);
    if (data == NULL || fread(data, 1, size, f) != (size_t)size || size < 8 || memcmp(data, "U3TZ", 4) != 0)
    {
        fclose(f);
        free(data);
        printf("[u3d] %s isn't UNOVA's text\n", path);
        return;
    }
    fclose(f);
    sTextSize = data[4] | data[5] << 8 | data[6] << 16 | (unsigned long)data[7] << 24;
    sText = malloc(sTextSize ? sTextSize : 1);
    if (sText == NULL || uncompress(sText, &sTextSize, data + 8, (unsigned long)(size - 8)) != 0
     || sTextSize < 12 || memcmp(sText, "U3DT", 4) != 0 || Get32(4) != 1)
    {
        printf("[u3d] %s: can't read UNOVA's text\n", path);
        free(sText);
        sText = NULL;
    }
    free(data);
    if (sText)
        printf("[u3d] %s: %u banks of UNOVA's text\n", path, Get32(8));
}

u16 U3D_TextBanks(void)
{
    if (!sTried)
        Load();
    return sText ? Get32(8) : 0;
}

const u8 *U3D_Text(u16 bank, u16 msg)
{
    u32 bankAt, count, at;
    if (!sTried)
        Load();
    if (sText == NULL || bank >= Get32(8))
        return sMissing;
    bankAt = Get32(12 + 4 * bank);
    count = Get32(bankAt);
    if (msg >= count || (at = Get32(bankAt + 4 + 4 * msg)) == 0 || at >= sTextSize)
        return sMissing;
    return sText + at;
}

// StringExpandPlaceholders: ID (after PLACEHOLDER_BEGIN), SRC just past it; writes at *DEST, returns where SRC goes on
const u8 *U3D_ExpandPlaceholder(u8 id, const u8 *src, u8 **dest)
{
    static u8 depth;   // (a word slot holding a text that names that slot again stops here, never loops)
    if (id == PLACEHOLDER_ID_UNOVA_TEXT)
    {
        u16 bank = (src[0] & 0x7F) << 7 | (src[1] & 0x7F), msg = (src[2] & 0x7F) << 7 | (src[3] & 0x7F);
        if (depth < 4)
        {
            depth++;
            *dest = StringExpandPlaceholders(*dest, U3D_Text(bank, msg));
            depth--;
        }
        return src + 4;
    }
    if (depth < 4)
    {
        depth++;
        *dest = StringExpandPlaceholders(*dest, gUnovaStrVar[(id - PLACEHOLDER_ID_UNOVA_VAR) % UNOVA_STR_VARS]);
        depth--;
    }
    else
        MPNet_Log("u3d text: word slot %u names itself", id - PLACEHOLDER_ID_UNOVA_VAR);
    return src;
}
