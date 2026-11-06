// Moemon Maker, M4b: "Start from the game's script". Every person's, sign's and trigger's script of every map as the
// body of a Maker event (tools/maker/decompile.py --pack: its branches, movements and the texts it reaches in its own
// scripts file copied in as `label` / `raw` lines, texts as quoted strings where the Maker's text says them exactly),
// zlib-compressed into include/constants/maker_scripts.h when the game is built (never committed), unpacked the first
// time the editor asks. An event made from one runs the same commands as the game's own script (the round trip:
// tools/test/maker_decompile.sh), and the user changes it from there.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "maker.h"
#include "maker_api.h"
#include "constants/maker_scripts.h"

char *stbi_zlib_decode_malloc(const char *buffer, int len, int *outlen);

struct GameScript { const char *label, *cond, *lines; };
static struct GameScript *sScripts;
static int sScriptCount;
static char *sText;

static int CmpScript(const void *a, const void *b)
{
    return strcmp(((const struct GameScript *)a)->label, ((const struct GameScript *)b)->label);
}

static void Load(void)
{
    int len = 0, n = 0, cap = 0;
    char *p, *end;
    if (sText)
        return;
    sText = stbi_zlib_decode_malloc((const char *)sMakerScriptsZ, sizeof(sMakerScriptsZ), &len);
    if (!sText || len != sMakerScriptsLen)
    {
        free(sText);
        sText = calloc(1, 1);
        return;
    }
    // entries "LABEL COND\nlines...\n\0", sorted by label
    for (p = sText, end = sText + len; p < end; p += strlen(p) + 1)
    {
        char *nl = strchr(p, '\n'), *sp;
        if (!nl)
            continue;
        *nl = 0;
        sp = strchr(p, ' ');
        if (sp)
            *sp = 0;
        if (n == cap)
        {
            cap = cap * 2 + 256;
            sScripts = realloc(sScripts, cap * sizeof(*sScripts));
        }
        sScripts[n].label = p;
        sScripts[n].cond = sp ? sp + 1 : "";
        sScripts[n].lines = nl + 1;
        n++;
        p = nl + 1;
    }
    sScriptCount = n;
}

static const struct GameScript *Find(const char *label)
{
    struct GameScript key;
    Load();
    key.label = label;
    return sScriptCount ? bsearch(&key, sScripts, sScriptCount, sizeof(sScripts[0]), CmpScript) : NULL;
}

// The body of a Maker event that does what the game's script SCRIPTNAME does (KIND: MAKER_EVENT_ON_PERSON / SIGN /
// TRIGGER), or NULL when there is none (a script the Maker's lines can't say: kept as the game has it)
const char *MakerApi_GameScript(const char *scriptName, int kind)
{
    static char *sOut;
    const struct GameScript *g = scriptName && scriptName[0] ? Find(scriptName) : NULL;
    const char *l;
    size_t cap, n;
    if (!g)
        return NULL;
    cap = strlen(g->lines) * 2 + strlen(g->cond) + 64;
    free(sOut);
    sOut = malloc(cap);
    n = snprintf(sOut, cap, "page trigger=%s%s%s\n", kind == MAKER_EVENT_ON_TRIGGER ? "step" : "talk",
                 g->cond[0] ? " " : "", g->cond);
    for (l = g->lines; *l;)   // each command under the page, indented by two
    {
        const char *nl = strchr(l, '\n');
        size_t len = nl ? (size_t)(nl - l) : strlen(l);
        if (len)
            n += snprintf(sOut + n, cap - n, "  %.*s\n", (int)len, l);
        l += len + (nl ? 1 : 0);
    }
    return sOut;
}

int Maker_GameScriptCount(void)
{
    Load();
    return sScriptCount;
}

const char *Maker_GameScriptLabel(int i)
{
    Load();
    return i >= 0 && i < sScriptCount ? sScripts[i].label : NULL;
}
