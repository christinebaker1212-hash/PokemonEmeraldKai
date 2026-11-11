// Moemon Kai: ORRE's words (docs/ORRE_HANDOFF.md step 2). Every line of Pokemon Colosseum's text in Kai's character set
// comes in orre/orre_text.pak beside ORRE's other packs (tools/orre/build_text.py, ripped from the disc by
// tools/orre/make_pack.sh; nothing of it is in this repository). A script's text is a placeholder naming Colosseum's
// own message id (PLACEHOLDER_ID_ORRE_TEXT, 3 bytes of 7 bits); StringExpandPlaceholders asks here for the line.
// The words the game fills in ({npc} the speaker, {item}, {pokemon}, {digit}...) are ORRE's string slots, one per
// Colosseum control (PLACEHOLDER_ID_ORRE_VAR + its id: tools/orre/msg.py CONTROLS), set as Colosseum's scripts set
// them (Orre_SetStrVar). Never ORRE's words written here: only "Message missing" when a line isn't in the pack.
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "orre.h"
#include "string_util.h"
#include "constants/characters.h"

const char *Updater_OrreRoot(void);
int uncompress(unsigned char *dest, unsigned long *destLen, const unsigned char *source, unsigned long sourceLen);

static u8 sStrVar[ORRE_STR_VARS][ORRE_STR_VAR_SIZE];
static u8 *sText;
static unsigned long sTextSize;
static bool8 sTried;
static const u8 sMissing[] = {0xC7, 0xD9, 0xE7, 0xE7, 0xD5, 0xDB, 0xD9, 0x00, 0xE1, 0xDD, 0xE7, 0xE7, 0xDD, 0xE2, 0xDB, EOS};   // "Message missing"
static const u8 sEmpty[] = {EOS};

static u32 Get32(unsigned long at)
{
    if (at + 4 > sTextSize)
        return 0;
    return sText[at] | sText[at + 1] << 8 | sText[at + 2] << 16 | (u32)sText[at + 3] << 24;
}

static void Load(void)
{
    char path[1100];
    FILE *f;
    long size;
    u8 *data;
    sTried = TRUE;
    if (getenv("MOEMON_ORRE_TEXT"))
        snprintf(path, sizeof(path), "%s", getenv("MOEMON_ORRE_TEXT"));
    else
        snprintf(path, sizeof(path), "%s/orre_text.pak", Updater_OrreRoot());
    if ((f = fopen(path, "rb")) == NULL)
    {
        printf("[orre] no text pack at %s\n", path);
        return;
    }
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);
    data = malloc(size > 0 ? size : 1);
    if (data == NULL || fread(data, 1, size, f) != (size_t)size || size < 8 || memcmp(data, "O3TZ", 4) != 0)
    {
        fclose(f);
        free(data);
        printf("[orre] %s isn't ORRE's text\n", path);
        return;
    }
    fclose(f);
    sTextSize = data[4] | data[5] << 8 | data[6] << 16 | (unsigned long)data[7] << 24;
    sText = malloc(sTextSize ? sTextSize : 1);
    if (sText == NULL || uncompress(sText, &sTextSize, data + 8, (unsigned long)(size - 8)) != 0
     || sTextSize < 12 || memcmp(sText, "ORTX", 4) != 0 || Get32(4) != 1 || 12 + 8 * (unsigned long)Get32(8) > sTextSize)
    {
        printf("[orre] %s: can't read ORRE's text\n", path);
        free(sText);
        sText = NULL;
    }
    free(data);
    if (sText)
        printf("[orre] %s: %u lines of ORRE's text\n", path, Get32(8));
}

u32 Orre_TextCount(void)
{
    static time_t sLast;
    // (not there or not readable: looked for again every few seconds at most, so a pack put there while the game runs
    // is read then: "Message missing" never stays for the session)
    if (sText == NULL && (!sTried || time(NULL) - sLast >= 3))
    {
        sLast = time(NULL);
        Load();
    }
    return sText ? Get32(8) : 0;
}

bool8 Orre_HasText(u32 id)
{
    u32 lo = 0, hi = Orre_TextCount();
    while (lo < hi)
    {
        u32 mid = (lo + hi) / 2, key = Get32(12 + 8 * mid);
        if (key == id)
            return Get32(16 + 8 * mid) < sTextSize;
        if (key < id)
            lo = mid + 1;
        else
            hi = mid;
    }
    return FALSE;
}

// Colosseum's message ID in Kai's character set ("Message missing" when the pack hasn't got it)
const u8 *Orre_Text(u32 id)
{
    u32 lo = 0, hi = Orre_TextCount();
    while (lo < hi)
    {
        u32 mid = (lo + hi) / 2, key = Get32(12 + 8 * mid);
        if (key == id)
        {
            u32 at = Get32(16 + 8 * mid);
            return at < sTextSize ? sText + at : sMissing;
        }
        if (key < id)
            lo = mid + 1;
        else
            hi = mid;
    }
    return sMissing;
}

// The placeholder a script's text is (tools/orre/build_text.py orre_text_bytes): OUT holds 6 bytes
void Orre_TextPlaceholder(u8 *out, u32 id)
{
    out[0] = PLACEHOLDER_BEGIN;
    out[1] = PLACEHOLDER_ID_ORRE_TEXT;
    out[2] = (id >> 14) & 0x7F;
    out[3] = (id >> 7) & 0x7F;
    out[4] = id & 0x7F;
    out[5] = EOS;
}

// ORRE's string slots: CTL is Colosseum's control id (tools/orre/msg.py CONTROLS: 89 npc, 45 item, 50 pokemon...)
void Orre_SetStrVar(u8 ctl, const u8 *str)
{
    if (ctl >= ORRE_STR_VARS)
        return;
    if (str == NULL)
        sStrVar[ctl][0] = EOS;
    else
        *StringCopyN(sStrVar[ctl], str, ORRE_STR_VAR_SIZE - 1) = EOS;
}

// The partner's name ({rui}, Colosseum's control 44): the player's to give at PHENAC (orre_vm.c Orre_VmName), RUI
// until then or left empty. Kept with the story, as 56 of Kai's own flags (each letter's bits turned over, so none
// set is no name): no side file changes its layout
#define RUI_CTL 44
static const u8 sRui[] = _("RUI");

const u8 *Orre_PartnerName(void)
{
    static u8 name[PLAYER_NAME_LENGTH + 1];
    u32 i, b;
    for (i = 0; i < PLAYER_NAME_LENGTH; i++)
    {
        u8 c = 0;
        for (b = 0; b < 8; b++)
            if (Orre_Flag(ORRE_FLAG_RUI_NAME + i * 8 + b))
                c |= 1 << b;
        name[i] = c ^ 0xFF;
    }
    name[PLAYER_NAME_LENGTH] = EOS;
    return name[0] == EOS ? sRui : name;
}

void Orre_SetPartnerName(const u8 *name)
{
    u32 i, b;
    bool8 end = FALSE;
    for (i = 0; i < PLAYER_NAME_LENGTH; i++)
    {
        u8 c;
        if (name[i] == EOS)
            end = TRUE;
        c = (end ? EOS : name[i]) ^ 0xFF;
        for (b = 0; b < 8; b++)
            Orre_SetFlag(ORRE_FLAG_RUI_NAME + i * 8 + b, c >> b & 1);
    }
}

const u8 *Orre_StrVar(u8 ctl)
{
    if (ctl == RUI_CTL)
        return Orre_PartnerName();
    return ctl < ORRE_STR_VARS ? sStrVar[ctl] : sEmpty;
}

void Orre_ClearStrVars(void)
{
    memset(sStrVar, EOS, sizeof(sStrVar));
}

// StringExpandPlaceholders' ORRE branch: SRC points past the placeholder's id; returns past its bytes
const u8 *Orre_ExpandPlaceholder(u8 id, const u8 *src, u8 **dest)
{
    if (id == PLACEHOLDER_ID_ORRE_TEXT)
    {
        u32 msg = (u32)(src[0] & 0x7F) << 14 | (src[1] & 0x7F) << 7 | (src[2] & 0x7F);
        *dest = StringExpandPlaceholders(*dest, Orre_Text(msg));
        return src + 3;
    }
    *dest = StringExpandPlaceholders(*dest, Orre_StrVar(id - PLACEHOLDER_ID_ORRE_VAR));
    return src;
}
