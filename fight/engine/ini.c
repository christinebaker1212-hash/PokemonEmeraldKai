/*
 * Text file parser for MUGEN's .def/.cfg/.cns/.air files (original module
 * 0x469A50-0x46B20D).
 *
 * Key lookups compare case-insensitively unless g_iniCaseSensitive is set.
 * Blank characters are ' ', '\t' and '\r' (only for chars 1..127, as in the
 * original).
 */
#include "mugen.h"

#define INI_READ_SIZE 0x4000

static int Ini_IsBlank(char c)
{
    return c > 0 && (c == ' ' || c == '\t' || c == '\r');
}

/* Marks the cursor as past the group when it reaches the end or a header. */
static void Ini_CheckEnd(Ini *ini)
{
    ini->at_end = 0;
    if (ini->cursor >= ini->line_count || ini->lines[ini->cursor][0] == '[') {
        ini->at_end = 1;
    }
}

/* Selects group index (no range check) and rewinds the cursor to its first line. */
static void Ini_EnterGroup(Ini *ini, int index)
{
    ini->group = index;
    ini->group_line = ini->group_lines[index] + 1;
    ini->cursor = ini->group_line;
    ini->group_offset = (int)(ini->lines[ini->group_line] - ini->text);
    Ini_CheckEnd(ini);
}

/* Key at the start of line followed by a blank or '='. */
static int Ini_LineHasKey(const char *line, const char *key, int key_len)
{
    if (g_iniCaseSensitive) {
        if (!Str_PrefixMatch(line, key, key_len)) {
            return 0;
        }
    } else if (!Str_PrefixMatchI(line, key, key_len)) {
        return 0;
    }
    return Ini_IsBlank(line[key_len]) || line[key_len] == '=';
}

/* Value of a "key = value" line: after '=' and the blanks that follow it. */
static char *Ini_LineValue(char *line, int key_len)
{
    int i = key_len;

    while (line[i] != '=' && line[i] != '\0') {
        i++;
    }
    if (line[i] != '\0') {
        i++;
    }
    while (Ini_IsBlank(line[i])) {
        i++;
    }
    return line + i;
}

/* 0x00469A50 */
void Ini_Free(Ini *ini)
{
    IniSource *src;
    int i;

    if (ini == NULL) {
        return;
    }
    src = ini->src;
    if (*ini->refs != 1) {
        (*ini->refs)--;
        free(ini);
        return;
    }
    if (ini->text != NULL) {
        free(ini->text);
    }
    if (ini->lines != NULL) {
        free(ini->lines);
    }
    if (ini->group_lines != NULL) {
        free(ini->group_lines);
    }
    if (src != NULL) {
        if (src->line_map != NULL) {
            free(src->line_map);
        }
        if (src->src_lines != NULL) {
            for (i = 0; i < src->src_count; i++) {
                if (src->src_lines[i] != NULL) {
                    free(src->src_lines[i]);
                }
            }
            free(src->src_lines);
        }
        free(src);
    }
    free(ini->refs);
    free(ini);
}

/* 0x00469B00 */
Ini *Ini_Open(const char *path)
{
    return Ini_OpenMode(path, INI_MODE_READ);
}

/* 0x00469B40 */
Ini *Ini_OpenMode(const char *path, int mode)
{
    Stream *s = Stream_OpenFile(path, "rt");
    Ini *ini;

    if (s == NULL) {
        return NULL;
    }
    ini = Ini_Parse(s, mode);
    Stream_Close(s);
    return ini;
}

/* 0x00469B80 */
Ini *Ini_OpenEditable(const char *path)
{
    return Ini_OpenMode(path, INI_MODE_EDIT);
}

/* 0x00469BC0 */
Ini *Ini_Parse(Stream *s, int mode)
{
    char buf[INI_READ_SIZE + 8];
    int keep_raw = mode == INI_MODE_EDIT;
    int keep_src = mode == INI_MODE_EDIT || mode == INI_MODE_TRACK;
    IniSource *src = NULL;
    int lines_cap = 0x100;
    int lines_bytes;
    int groups_cap = 0x40;
    int groups_bytes;
    long start;
    char *out;
    int *refs;
    Ini *ini;

    if (s == NULL) {
        return NULL;
    }
    start = Stream_Tell(s);
    ini = Mem_Calloc(1, sizeof(Ini));
    if (ini == NULL) {
        return NULL;
    }
    refs = Mem_Calloc(1, sizeof(int));
    ini->refs = refs;
    if (refs == NULL) {
        return NULL;    /* leaks ini, as the original */
    }
    *refs = 1;
    if (keep_src) {
        src = Mem_Calloc(1, sizeof(IniSource));
        ini->src = src;
        if (src == NULL) {
            return NULL;
        }
        if (keep_raw) {
            src->src_cap = 0x100;
            src->src_lines = Mem_Calloc(0x100, sizeof(char *));    /* Moemon Fight: 0x400 bytes on 32-bit */
            if (src->src_lines == NULL) {
                Ini_Free(ini);
                return NULL;
            }
        }
    }
    ini->text_size = Stream_GetSize(s);
    ini->text = Mem_Calloc(1, ini->text_size + 0x10);
    if (ini->text == NULL) {
        goto fail;
    }
    ini->lines = Mem_Calloc(0x101, sizeof(char *));
    if (ini->lines == NULL) {
        goto fail;
    }
    ini->group_lines = Mem_Calloc(0x41, sizeof(int));
    if (ini->group_lines == NULL) {
        goto fail;
    }
    if (keep_src) {
        src->line_map = Mem_Calloc(0x101, sizeof(int));
        if (src->line_map == NULL) {
            goto fail;
        }
    }

    out = ini->text;
    memset(buf, 0, sizeof(buf));
    /* Drop a UTF-8 byte order mark (remembered for Ini_Save), otherwise rewind. */
    if (Stream_Gets(s, buf, 4) == NULL) {
        goto done;
    }
    if ((unsigned char)buf[0] == 0xEF && (unsigned char)buf[1] == 0xBB && (unsigned char)buf[2] == 0xBF) {
        if (keep_src) {
            src->bom[0] = (char)0xEF;
            src->bom[1] = buf[1];
            src->bom[2] = buf[2];
            src->bom[3] = '\0';
        }
        buf[0] = '\0';
    } else {
        Stream_Seek(s, start, SEEK_SET);
    }
    if (Stream_Gets(s, buf, INI_READ_SIZE) == NULL) {
        goto done;
    }
    groups_bytes = 0x200;
    lines_bytes = 0x800;
    do {
        int len, first, i;
        char *line;

        if (keep_raw) {
            char *raw;
            if (src->src_count + 1 >= src->src_cap) {
                char **grown = realloc(src->src_lines, (src->src_cap + 0x100) * sizeof(char *));    /* Moemon Fight: + 0x400 bytes on 32-bit */
                if (grown == NULL) {
                    goto fail;
                }
                src->src_cap += 0x100;
                src->src_lines = grown;
            }
            for (len = 0; buf[len] != '\0' && buf[len] != '\n'; len++) {
            }
            src->src_lines[src->src_count] = malloc(len + 1);
            raw = src->src_lines[src->src_count];
            if (raw == NULL) {
                Ini_Free(ini);
                return NULL;
            }
            for (i = 0; i < len; i++) {
                raw[i] = buf[i];
            }
            raw[i] = '\0';
        }
        if (keep_src) {
            src->src_count++;
        }

        /* Strip the comment, the newline and trailing blanks. */
        for (i = 0; buf[i] != '\0'; i++) {
            if (buf[i] == ';') {
                buf[i] = '\0';
                break;
            }
        }
        len = (int)strlen(buf);
        if (buf[0] != '\0') {
            if (buf[len - 1] == '\n') {
                buf[len - 1] = '\0';
                len--;
            }
            if (buf[0] != '\0') {
                for (i = len - 1; i >= 0 && Ini_IsBlank(buf[i]); i--) {
                    buf[i] = '\0';
                    len--;
                }
            }
        }
        for (first = 0; Ini_IsBlank(buf[first]); first++) {
        }
        line = buf + first;

        if (*line != '\0') {
            strcpy(out, line);
            if (keep_src) {
                src->line_map[ini->line_count] = src->src_count - 1;
            }
            ini->lines[ini->line_count] = out;
            ini->line_count++;
            if (ini->line_count >= lines_cap) {
                char **grown = realloc(ini->lines, (size_t)(lines_cap + 0x100) * sizeof(char *));   /* Moemon Fight: lines_bytes counts 4-byte pointers */
                if (grown == NULL) {
                    goto fail;
                }
                ini->lines = grown;
                if (keep_src) {
                    int *map = realloc(src->line_map, lines_bytes);
                    if (map == NULL) {
                        goto fail;
                    }
                    src->line_map = map;
                }
                lines_cap += 0x100;
                lines_bytes += 0x400;
            }
            if (*line == '[' && buf[len - 1] == ']') {
                ini->group_lines[ini->group_count] = ini->line_count - 1;
                ini->group_count++;
                if (ini->group_count >= groups_cap) {
                    int *grown = realloc(ini->group_lines, groups_bytes);
                    if (grown == NULL) {
                        goto fail;
                    }
                    groups_cap += 0x40;
                    groups_bytes += 0x100;
                    ini->group_lines = grown;
                }
            }
            out += strlen(line) + 1;
        }
    } while (Stream_Gets(s, buf, INI_READ_SIZE) != NULL);

done:
    memcpy(out, "[TPEnd]", 8);
    ini->lang[0] = g_iniLang[0];
    ini->lang[1] = g_iniLang[1];
    ini->lang[2] = g_iniLang[2];
    return ini;

fail:
    Ini_Free(ini);
    return NULL;
}

/* 0x0046A150: writes the source lines back (INI_MODE_EDIT). */
int Ini_Save(Ini *ini, const char *path)
{
    IniSource *src = ini->src;
    FILE *fp;
    int i;

    if (src == NULL) {
        return 0;
    }
    fp = fopen(path, "wt");
    if (fp == NULL) {
        return 0;
    }
    if (src->bom[0] != '\0') {
        fprintf(fp, "%s", src->bom);
    }
    for (i = 0; i < src->src_count; i++) {
        fprintf(fp, "%s\n", src->src_lines[i]);
    }
    fclose(fp);
    return 1;
}

/* 0x0046A1D0: selects "[name]"; returns the group's text offset or -1. */
int Ini_FindGroup(Ini *ini, const char *name)
{
    int *group_lines = ini->group_lines;
    char **lines = ini->lines;
    char header[256];
    int len, i;

    header[0] = '[';
    strncpy(header + 1, name, 0xFF);
    strcat(header, "]");
    len = (int)strlen(header);
    for (i = 0; i < ini->group_count; i++) {
        if (Str_PrefixMatchI(lines[group_lines[i]], header, len)) {
            if (i < 0 || i >= ini->group_count) {
                break;
            }
            Ini_EnterGroup(ini, i);
            return ini->group_offset;
        }
    }
    return -1;
}

/* 0x0046A2F0: prefers "[lang.name]" when a language is set. */
int Ini_SelectGroup(Ini *ini, const char *name)
{
    char localized[256];
    int r;

    if (ini->lang != NULL && ini->lang[0] != '\0') {
        _snprintf(localized, sizeof(localized), "%s.%s", ini->lang, name);
        r = Ini_FindGroup(ini, localized);
        if (r != -1) {
            return r;
        }
    }
    return Ini_FindGroup(ini, name);
}

/* 0x0046A360 */
int Ini_SelectGroupIndex(Ini *ini, int index)
{
    if (index < 0 || index >= ini->group_count) {
        return -1;
    }
    Ini_EnterGroup(ini, index);
    return ini->group_offset;
}

/* 0x0046A3B0 */
void Ini_Rewind(Ini *ini)
{
    Ini_EnterGroup(ini, ini->group);
}

/* 0x0046A3F0: selects the next group; returns its "[header]" line or NULL. */
const char *Ini_NextGroup(Ini *ini)
{
    ini->group++;
    if (ini->group >= ini->group_count) {
        return NULL;
    }
    Ini_EnterGroup(ini, ini->group);
    return ini->lines[ini->group_lines[ini->group]];
}

/* 0x0046A450: counts groups whose name starts with prefix; keeps the selected group index. */
int Ini_CountGroups(Ini *ini, const char *prefix)
{
    int saved = ini->group;
    int count = 0;
    const char *header;

    ini->group = -1;
    for (header = Ini_NextGroup(ini); header != NULL; header = Ini_NextGroup(ini)) {
        if (Str_StartsWithI(header + 1, prefix)) {
            count++;
        }
    }
    ini->group = saved;
    return count;
}

/* 0x0046A4E0: index of the key's line in the selected group, or -1. */
int Ini_FindKeyLine(Ini *ini, const char *key)
{
    int skip = 0;   /* always 0: the "n-th match" argument was folded by LTCG */
    int key_len = (int)strlen(key);
    int i;

    for (i = ini->group_line; i < ini->line_count; i++) {
        const char *line = ini->lines[i];
        if (line[0] == '[') {
            break;
        }
        if (Ini_LineHasKey(line, key, key_len)) {
            if (skip <= 0) {
                return i;
            }
            skip--;
        }
    }
    return -1;
}

/* 0x0046A5E0: value of key in the selected group, or NULL; moves the cursor after it. */
char *Ini_GetValue(Ini *ini, const char *key)
{
    int key_len = (int)strlen(key);
    int i;

    for (i = ini->group_line; i < ini->line_count; i++) {
        char *line = ini->lines[i];
        if (line[0] == '[') {
            break;
        }
        if (Ini_LineHasKey(line, key, key_len)) {
            ini->cursor = i + 1;
            Ini_CheckEnd(ini);
            return Ini_LineValue(line, key_len);
        }
    }
    return NULL;
}

/* 0x0046A740: malloc'ed copy of the value with quotes and escapes resolved. */
char *Ini_GetValueDup(Ini *ini, const char *key)
{
    char *value = Ini_GetValue(ini, key);
    char *copy;

    if (value == NULL) {
        return NULL;
    }
    copy = malloc(strlen(value) + 1);
    if (copy == NULL) {
        return NULL;
    }
    strcpy(copy, value);
    Str_Unquote(copy);
    return copy;
}

/* 0x0046A7A0: like Ini_GetValue, but searches from the cursor. */
char *Ini_GetNextValue(Ini *ini, const char *key)
{
    int key_len = (int)strlen(key);
    int i;

    if (ini->at_end) {
        return NULL;
    }
    for (i = ini->cursor; i < ini->line_count; i++) {
        char *line = ini->lines[i];
        if (line[0] == '[') {
            break;
        }
        if (Ini_LineHasKey(line, key, key_len)) {
            ini->cursor = i + 1;
            Ini_CheckEnd(ini);
            return Ini_LineValue(line, key_len);
        }
    }
    return NULL;
}

/* 0x0046A8A0 */
int Ini_GetFloat(Ini *ini, const char *key, float *out)
{
    char *value = Ini_GetValue(ini, key);

    if (value == NULL) {
        return 0;
    }
    *out = (float)atof(value);
    return 1;
}

/* 0x0046A8D0 */
void Ini_GetDoubleDef(Ini *ini, const char *key, double *out, double def)
{
    char *value = Ini_GetValue(ini, key);

    if (value == NULL || sscanf(value, "%lf", out) != 1) {
        *out = def;
    }
}

/* 0x0046A900 */
int Ini_GetInt(Ini *ini, const char *key, int *out)
{
    char *value = Ini_GetValue(ini, key);

    if (value == NULL) {
        return 0;
    }
    *out = atoi(value);
    return 1;
}

/* 0x0046A930 */
void Ini_GetIntDef(Ini *ini, const char *key, int *out, int def)
{
    char *value = Ini_GetValue(ini, key);

    if (value == NULL || sscanf(value, "%d", out) != 1) {
        *out = def;
    }
}

/* 0x0046A960: Str_VScanf on the value; returns the number of fields read. */
int Ini_Scanf(Ini *ini, const char *key, const char *fmt, ...)
{
    char *value = Ini_GetValue(ini, key);
    va_list ap;
    int n;

    if (value == NULL) {
        return 0;
    }
    va_start(ap, fmt);
    n = Str_VScanf(value, ap, fmt);
    va_end(ap);
    return n;
}

/* 0x0046A990 */
int Ini_GetString(Ini *ini, const char *key, char *out, int size)
{
    char *value = Ini_GetValue(ini, key);

    if (value == NULL) {
        return 0;
    }
    out[0] = '\0';
    strncat(out, value, size - 1);
    return 1;
}

/* 0x0046AA30 */
const char *Ini_NextLine(Ini *ini)
{
    const char *line;

    if (ini->at_end) {
        return NULL;
    }
    line = ini->lines[ini->cursor];
    ini->cursor++;
    Ini_CheckEnd(ini);
    return line;
}

/* 0x0046A9C0 */
const char *Ini_FirstLine(Ini *ini)
{
    Ini_Rewind(ini);
    return Ini_NextLine(ini);
}

/* 0x0046AA70: steps the cursor back; returns the line before the new cursor. */
const char *Ini_PrevLine(Ini *ini)
{
    int line = ini->cursor - 1;

    if (line == ini->group_line) {
        return NULL;
    }
    ini->cursor = line;
    Ini_CheckEnd(ini);
    return ini->lines[line - 1];
}

/* 0x0046AAB0: reads the next "key = value" line of the group (each up to 255 chars). */
int Ini_ReadKeyValue(Ini *ini, char *key, char *value)
{
    const char *line;
    int key_len, start, end, n;

    if (ini->at_end) {
        return 0;
    }
    line = ini->lines[ini->cursor];
    ini->cursor++;
    Ini_CheckEnd(ini);
    if (line == NULL) {
        return 0;
    }
    for (key_len = 0; !Ini_IsBlank(line[key_len]) && line[key_len] != '=' && line[key_len] != '\0'; key_len++) {
    }
    if (key_len == 0) {
        return 0;
    }
    start = key_len;
    while (Ini_IsBlank(line[start])) {
        start++;
    }
    if (line[start] != '=') {
        return 0;
    }
    do {
        start++;
    } while (Ini_IsBlank(line[start]));
    end = start;
    if (line[end] != '\0') {
        while (line[++end] != '\0') {
        }
    }
    n = key_len < 0xFF ? key_len : 0xFF;
    strncpy(key, line, n);
    key[n] = '\0';
    n = end - start < 0xFF ? end - start : 0xFF;
    strncpy(value, line + start, n);
    value[n] = '\0';
    return 1;
}

/*
 * 0x0046ABE0: source line of a parsed line. Past the last line of a group it
 * returns the first empty source line after the previous key, so new keys
 * can be inserted there.
 */
int Ini_SourceLine(int line, Ini *ini)
{
    IniSource *src = ini->src;
    char **raw;
    int r, end;

    if (line < 0 || src == NULL) {
        return -1;
    }
    raw = src->src_lines;
    if (raw == NULL && line >= ini->line_count && ini->at_end) {
        return src->line_map[ini->line_count - 1];
    }
    if (line <= 0 || !ini->at_end || raw == NULL) {
        return src->line_map[line];
    }
    r = src->line_map[line - 1] + 1;
    if (r >= src->src_count) {
        return r;
    }
    end = src->line_map[line];
    while (r < end && raw[r][0] != '\0') {
        r++;
        if (r >= src->src_count) {
            return r;
        }
    }
    return r;
}

/*
 * 0x0046AC60: replaces the value of key in the source lines (INI_MODE_EDIT),
 * keeping a trailing comment in its column. Returns 1, 0 if the key is
 * missing, -1 without source lines or when the line has no '='.
 */
int Ini_SetValuef(Ini *ini, const char *key, const char *fmt, ...)
{
    IniSource *src = ini->src;
    char out[0x400];
    char value[0x400];
    int comment = -1;
    int line, src_line, keep, i, n;
    const char *p;
    char *raw, *copy;
    va_list ap;

    if (src == NULL) {
        return -1;
    }
    va_start(ap, fmt);
    vsprintf(value, fmt, ap);
    va_end(ap);
    line = Ini_FindKeyLine(ini, key);
    if (line == -1) {
        return 0;
    }
    src_line = src->line_map[line];
    raw = src->src_lines[src_line];
    for (i = 0; raw[i] != '='; i++) {
        if (raw[i] == '\0') {
            return -1;
        }
    }
    do {
        i++;
    } while (Ini_IsBlank(raw[i]) && raw[i] != '\0');
    keep = i;
    for (; raw[i] != '\0'; i++) {
        if (raw[i] == ';') {
            comment = i;
            break;
        }
    }
    n = 0;
    if (keep > 0) {
        memcpy(out, raw, keep);
        n = keep;
    }
    for (p = value; *p != '\0'; p++) {
        out[n++] = *p;
    }
    if (comment != -1) {
        if (n < comment) {
            memset(out + n, ' ', comment - n);
            n = comment;
        }
        for (p = raw + comment; *p != '\0'; p++) {
            out[n++] = *p;
        }
    }
    out[n++] = '\0';
    copy = malloc(n);
    if (copy == NULL) {
        return 0;
    }
    strcpy(copy, out);
    free(raw);
    src->src_lines[src_line] = copy;
    return 1;
}

/* 0x0046AE10: removes surrounding double quotes and resolves C escapes in place. */
int Str_Unquote(char *s)
{
    int len = (int)strlen(s);
    const char *end;
    const char *in;
    char *dst = s;

    if (len < 2 || s[0] != '"' || s[len - 1] != '"') {
        return 0;
    }
    end = s + len - 1;
    in = s + 1;
    while (in < end) {
        char c = *in++;
        if (c == '\\') {
            c = *in++;
            if (c == '\0') {
                break;
            }
            switch (c) {
            case '"':  c = '"'; break;
            case '\'': c = '\''; break;
            case '?':  c = '?'; break;
            case '\\': c = '\\'; break;
            case 'a':  c = '\a'; break;
            case 'b':  c = '\b'; break;
            case 'f':  c = '\f'; break;
            case 'n':  c = '\n'; break;
            case 'r':  c = '\r'; break;
            case 't':  c = '\t'; break;
            case 'v':  c = '\v'; break;
            default:
                continue;   /* unknown escapes are dropped */
            }
        }
        *dst++ = c;
    }
    *dst = '\0';
    return 1;
}

/*
 * 0x0046AF30: minimal scanf: conversions %d %i %o %u %x %e %f %g %c %ld %lu
 * %le %lf %lg %s, blanks match any amount of blanks and ',' must match.
 * Returns the sum of the sscanf results.
 *
 * Like the original, any other character in fmt (or an unknown conversion)
 * makes this loop forever.
 */
int Str_VScanf(const char *str, va_list ap, const char *fmt)
{
    int total = 0;

    if (*str == '\0' || *fmt == '\0') {
        return 0;
    }
    for (;;) {
        const char *conv = NULL;
        int advance = 2;
        int used;

        if (*fmt != '%') {
            if (Char_IsSpace(*fmt)) {
                while (Char_IsSpace(*str)) {
                    str++;
                }
                fmt++;
            } else if (*fmt == ',') {
                if (*str != ',') {
                    return total;
                }
                str++;
                fmt++;
            } else if (*fmt == '\0') {
                return total;
            }
            continue;
        }
        switch (fmt[1]) {
        case 'd':
        case 'i':
            conv = "%d%n";
            break;
        case 'o':
            conv = "%o%n";
            break;
        case 'u':
            conv = "%u%n";
            break;
        case 'x':
            conv = "%x%n";
            break;
        case 'e':
        case 'f':
        case 'g':
            conv = "%f%n";
            break;
        case 'c':
            conv = "%c%n";
            break;
        case 'l':
            advance = 3;
            if (fmt[2] == 'd') {
                conv = "%ld%n";
            } else if (fmt[2] == 'u') {
                conv = "%lu%n";
            } else if (fmt[2] == 'e' || fmt[2] == 'f' || fmt[2] == 'g') {
                conv = "%lg%n";
            }
            break;
        case 's':
            conv = "%s%n";
            break;
        }
        if (conv == NULL) {
            continue;
        }
        used = 0;
        total += sscanf(str, conv, va_arg(ap, void *), &used);
        str += used;
        fmt += advance;
    }
}

/* 0x0046B1D0: language prefix copied into files parsed afterwards (NULL clears it). */
int Ini_SetLanguage(const char *lang)
{
    if (lang == NULL) {
        g_iniLang[2] = '\0';
        g_iniLang[1] = '\0';
        g_iniLang[0] = '\0';
        return 1;
    }
    g_iniLang[0] = lang[0];
    g_iniLang[1] = lang[1];
    g_iniLang[2] = lang[2];
    return 1;
}
