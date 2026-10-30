/*
 * Small string, path and math helpers (original module 0x4612E0-0x4618B5).
 *
 * The character helpers only call the CRT ctype functions for 1..127, as the
 * original does (chars are signed).
 */
#include "mugen.h"

/* 0x004612E0: calloc that records allocation failures for the error dialog. */
void *Mem_Calloc(size_t count, size_t size)
{
    void *p = calloc(count, size);

    if (p == NULL) {
        g_outOfMemory = 1;
    }
    return p;
}

/* 0x00461300 */
int File_GetType(const char *path)
{
    int len = (int)strlen(path);

    if (len > 4 && path[len - 4] == '.') {
        const char *ext = path + len - 3;
        if (_stricmp(ext, "pcx") == 0) {
            return FILETYPE_PCX;
        }
        if (_stricmp(ext, "png") == 0) {
            return FILETYPE_PNG;
        }
        if (_stricmp(ext, "sff") == 0) {
            return FILETYPE_SFF;
        }
        if (_stricmp(ext, "wav") == 0) {
            return FILETYPE_WAV;
        }
        if (_stricmp(ext, "snd") == 0) {
            return FILETYPE_SND;
        }
        if (_stricmp(ext, "ogg") == 0) {
            return FILETYPE_OGG;
        }
        if (_stricmp(ext, "mp3") == 0) {
            return FILETYPE_MP3;
        }
        if (_stricmp(ext, "mid") == 0) {
            return FILETYPE_MIDI;
        }
        if (_stricmp(ext, "mod") == 0) {
            return FILETYPE_MOD;
        }
        if (_stricmp(ext, "s3m") == 0) {
            return FILETYPE_MOD;
        }
        if (_stricmp(ext, "zip") == 0) {
            return FILETYPE_ZIP;
        }
    }
    if (len > 3 && path[len - 3] == '.') {
        if (_stricmp(path + len - 2, "xm") == 0) {
            return FILETYPE_MOD;
        }
        if (_stricmp(path + len - 2, "da") == 0) {
            return FILETYPE_DA;
        }
    }
    return FILETYPE_UNKNOWN;
}

/* 0x00461470 */
int Char_IsSpace(int c)
{
    return (unsigned int)(c - 1) < 0x7F && isspace(c);
}

/* 0x00461490 */
int Char_IsDigit(int c)
{
    return (unsigned int)(c - 1) < 0x7F && isdigit(c);
}

/* 0x004614B0 */
int Char_ToLower(int c)
{
    if ((unsigned int)(c - 1) < 0x7F) {
        c = tolower(c);
    }
    return c;
}

/* 0x004614D0: 1 if the strings match (ignoring case) up to the end of the shorter one. */
int Str_StartsWithI(const char *s, const char *prefix)
{
    for (; *s != '\0'; s++, prefix++) {
        if (*prefix == '\0') {
            return 1;
        }
        if (Char_ToLower(*s) != Char_ToLower(*prefix)) {
            return 0;
        }
    }
    return 1;
}

/* 0x00461530: 1 if the first n chars match ignoring case, with no terminator among them. */
int Str_PrefixMatchI(const char *a, const char *b, int n)
{
    int i;

    for (i = 0; i < n; i++) {
        if (Char_ToLower(a[i]) != Char_ToLower(b[i]) || a[i] == '\0' || b[i] == '\0') {
            return 0;
        }
    }
    return 1;
}

/* 0x004615A0: case-sensitive Str_PrefixMatchI. */
int Str_PrefixMatch(const char *a, const char *b, int n)
{
    int i;

    for (i = 0; i < n; i++) {
        if (a[i] != b[i] || a[i] == '\0' || b[i] == '\0') {
            return 0;
        }
    }
    return 1;
}

/*
 * 0x004615E0: skips leading blanks, then copies an identifier ([A-Za-z0-9_])
 * into dst (up to 63 chars). Returns the number of source chars consumed.
 */
int Str_ReadIdent(const char *src, char *dst)
{
    int room = 64;
    int i = 0;

    while (Char_IsSpace(src[i])) {
        i++;
    }
    while (isalnum(src[i]) || src[i] == '_') {
        if (room > 1) {
            *dst++ = src[i];
            room--;
        }
        i++;
    }
    if (room > 0) {
        *dst = '\0';
    }
    return i;
}

/* 0x00461650: lower-cases src into dst. Does not write the terminator. */
void Str_ToLowerCopy(const char *src, char *dst)
{
    for (; *src != '\0'; src++) {
        *dst++ = (char)tolower(*src);
    }
}

/* 0x00461680: splits path into directory (with trailing slash) and file name; either may be NULL. */
void Path_Split(const char *path, char *dir, char *file)
{
    int i;

    if (path[0] == '\0') {
        if (dir != NULL) {
            dir[0] = '\0';
        }
        if (file != NULL) {
            file[0] = '\0';
        }
        return;
    }
    for (i = (int)strlen(path) - 1; i >= 0; i--) {
        if (path[i] == '\\' || path[i] == '/') {
            break;
        }
    }
    i++;
    if (file != NULL) {
        strcpy(file, path + i);
    }
    if (dir != NULL) {
        if (i > 0) {
            strncpy(dir, path, i);
        }
        dir[i] = '\0';
    }
}

/* 0x00461700: out = dir + "/" + file, unless file is already absolute. */
void Path_Join(const char *file, const char *dir, char *out)
{
    size_t len;

    if (file[0] == '/' || file[0] == '\\' || (file[0] != '\0' && file[1] == ':')) {
        strcpy(out, file);
        return;
    }
    strcpy(out, dir);
    len = strlen(out);
    if (len > 0 && out[len - 1] != '/' && out[len - 1] != '\\') {
        strcat(out, "/");
    }
    strcat(out, file);
}

/* 0x004617A0: bounded strcat; size is the capacity of dst. */
void Str_Append(char *dst, const char *src, unsigned int size)
{
    unsigned int room = size - (unsigned int)strlen(dst);

    if (room > 1) {
        strncat(dst, src, room - 1);
    }
}

/*
 * 0x004617D0: moves cur towards target by step, then blends with target by t;
 * snaps to target when closer than snap.
 */
float Math_Approach(float cur, float target, float step, float t, float snap)
{
    float diff;
    float sign;
    float next;
    float result;

    if (fabsf(target - cur) < step) {
        cur = target;
    }
    diff = target - cur;
    if (diff < 0.0f) {
        sign = -1.0f;
    } else if (diff > 0.0f) {
        sign = 1.0f;
    } else {
        sign = 0.0f;
    }
    next = cur + step * sign;
    result = next * t + (1.0f - t) * target;
    if (fabsf(target - result) < snap) {
        return target;
    }
    return result;
}

/* 0x00461890 */
int Math_Round(float x)
{
    float r = (float)floor(x + 0.5);

    return (int)r;
}
