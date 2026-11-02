// Moemon Kai: SINNOH's words (docs/SINNOH_HANDOFF.md step 4). Platinum's message banks in the game's character set come
// in sinnoh/platinum_text.pak beside the game (tools/sinnoh/build_text.py, made from the decomp when the release is
// built; nothing of it is in this repository). A SINNOH script's text is a placeholder naming a bank and a message
// (PLACEHOLDER_ID_SINNOH_TEXT, 4 bytes of 7 bits); StringExpandPlaceholders asks here for the message.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "s3d.h"
#include "constants/characters.h"

const char *Updater_ExeFile(const char *name);
int uncompress(unsigned char *dest, unsigned long *destLen, const unsigned char *source, unsigned long sourceLen);

u8 gSinnohStrVar[3][64];   // SINNOH's text's string slots 3-5 (PLACEHOLDER_ID_SINNOH_VAR4-6; slots 0-2 are gStringVar1-3)

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
    const char *path = getenv("MOEMON_S3D_TEXT");
    FILE *f;
    long size;
    u8 *data;
    sTried = TRUE;
    if (path == NULL)
        path = Updater_ExeFile("sinnoh/platinum_text.pak");
    if ((f = fopen(path, "rb")) == NULL)
        return;
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);
    data = malloc(size > 0 ? size : 1);
    if (data == NULL || fread(data, 1, size, f) != (size_t)size || size < 8 || memcmp(data, "S3TZ", 4) != 0)
    {
        fclose(f);
        free(data);
        printf("[s3d] %s isn't SINNOH's text\n", path);
        return;
    }
    fclose(f);
    sTextSize = data[4] | data[5] << 8 | data[6] << 16 | (unsigned long)data[7] << 24;
    sText = malloc(sTextSize ? sTextSize : 1);
    if (sText == NULL || uncompress(sText, &sTextSize, data + 8, (unsigned long)(size - 8)) != 0
     || sTextSize < 12 || memcmp(sText, "S3DT", 4) != 0 || Get32(4) != 1)
    {
        printf("[s3d] %s: can't read SINNOH's text\n", path);
        free(sText);
        sText = NULL;
    }
    free(data);
    if (sText)
        printf("[s3d] %s: %u banks of SINNOH's text\n", path, Get32(8));
}

// How many banks the pack has (its last: Platinum's frontier trainers, build_text.py frontier_bank())
u16 S3D_TextBanks(void)
{
    if (!sTried)
        Load();
    return sText ? Get32(8) : 0;
}

const u8 *S3D_Text(u16 bank, u16 msg)
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

// A SINNOH message by Platinum's own name (its id in the decomp's message bank), for the code that names them (the TV:
// s3d_tv.c): gSinnohTextNames from tools/sinnoh/tv_data.py, sorted by the name's FNV-1a hash. FALSE: no such message
// (or a build without the decomp)
extern const u32 gSinnohTextNames[];
bool8 S3D_TextNamed(const char *name, u16 *bank, u16 *msg)
{
    u32 h = 0x811C9DC5, lo = 0, hi = gSinnohTextNames[0];
    const u8 *p;
    for (p = (const u8 *)name; *p; p++)
        h = (h ^ *p) * 0x01000193;
    while (lo < hi)
    {
        u32 mid = (lo + hi) / 2;
        const u32 *row = &gSinnohTextNames[1 + mid * 2];
        if (row[0] == h)
        {
            *bank = row[1] & 0xFFFF;
            *msg = row[1] >> 16;
            return TRUE;
        }
        if (row[0] < h)
            lo = mid + 1;
        else
            hi = mid;
    }
    return FALSE;
}

// The message itself (Message missing when there's none)
const u8 *S3D_TextByName(const char *name)
{
    u16 bank, msg;
    if (!S3D_TextNamed(name, &bank, &msg))
        return S3D_Text(0xFFFF, 0);
    return S3D_Text(bank, msg);
}
