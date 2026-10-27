/*
 * File lookup helpers (original module around 0x40C1D0-0x40C3E8, next to the
 * error reporting code).
 */
#include "mugen.h"

/* 0x0040C1D0 */
int File_Exists(const char *path)
{
    FILE *fp = fopen(path, "rb");

    if (fp != NULL) {
        fclose(fp);
    }
    return fp != NULL;
}

/*
 * 0x0040C200: looks for path under base_dir, the motif directory, "./" and
 * "data/" in that order (base_dir is skipped for absolute paths). On success
 * out receives the full path and dir_out, if given, the directory part.
 *
 * A drive letter is cut off the path and never used, as in the original.
 */
int File_Find(const char *path, char *out, char *dir_out, const char *base_dir)
{
    char drive[4];
    const char *prefixes[4];
    char dir[0x200];
    char full[0x200];
    char file[0x1F0];
    int i;

    if (path[0] != '\0' && path[1] == ':') {
        strncpy(drive, path, 2);
        drive[2] = '\0';
        path += 2;
    } else {
        drive[0] = '\0';
    }
    prefixes[0] = base_dir;
    prefixes[1] = g_game->motif_dir;
    prefixes[2] = "./";
    prefixes[3] = "data/";
    Path_Split(path, dir, file);
    if (dir[0] == '/' || dir[0] == '\\') {
        prefixes[0] = "";
    }
    for (i = 0; i < 4; i++) {
        FILE *fp;
        if (prefixes[i] == NULL) {
            continue;
        }
        sprintf(full, "%s%s%s", prefixes[i], dir, file);
        fp = fopen(full, "rb");
        if (fp != NULL) {
            fclose(fp);
            if (dir_out != NULL) {
                sprintf(dir_out, "%s%s", prefixes[i], dir);
            }
            strcpy(out, full);
            return 1;
        }
    }
    Error_Printf("Can't find file %s", path);
    return 0;
}

/*
 * 0x0040C360: copies the value of key (at most size - 1 chars) and unquotes
 * it, keeping the closing quote when truncation cut it off. Reports a missing
 * key as an error.
 */
int Ini_ReadString(Ini *ini, const char *key, unsigned int size, char *out)
{
    const char *value = Ini_GetValue(ini, key);
    int quoted = 0;
    int len;

    if (value == NULL) {
        Error_Printf("Cannot find parameter: %s", key);
        return 0;
    }
    len = (int)strlen(value);
    if (len > 0 && value[len - 1] == '"') {
        quoted = 1;
    }
    out[0] = '\0';
    if (size > 1) {
        strncat(out, value, size - 1);
    }
    if (quoted) {
        len = (int)strlen(out);
        if (len > 0) {
            out[len - 1] = '"';
        }
    }
    Str_Unquote(out);
    return 1;
}
