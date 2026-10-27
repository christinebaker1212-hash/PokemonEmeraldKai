/*
 * Moemon Fight: what a player's fight files are (include/mf_engine.h MfEngine_PackSummary), compared before an
 * online fight (src/mp/mp_fight.c): characters or stages that aren't the same on both sides are greyed out on
 * the select screen and never picked at random; a different engine, screenpack or select list means no fight.
 *
 * Each part's hash covers every file it uses: a character's whole folder, a stage's .def with its sprites and
 * music, and for the core the data/ and font/ folders and the screenpack's folder. File names count in lower
 * case (a pack copied between Windows and Linux keeps its hashes); junk the OS leaves (Thumbs.db, .DS_Store,
 * logs) doesn't count. Runs on the game's thread, not the engine's.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>
#ifndef _WIN32
#include <strings.h>
#endif

#include "../../include/mf_engine.h"

int MfHost_RealPath(const char *path, char *out, size_t size);

typedef unsigned long long u64h;

#define MIX(h, v) ((h) = ((h) ^ (v)) * 0x9E3779B97F4A7C15ULL, (h) ^= (h) >> 29)

/* The stages of the last summary, as engine paths (lower case, '/'): the host looks up the off list by path */
static char sStagePaths[MF_PACK_MAX][0x80];
static int sStagePathCount;

static void Pack_Norm(const char *in, char *out, size_t size)
{
    size_t i, n = 0;

    while (in[0] == '.' && (in[1] == '/' || in[1] == '\\')) {
        in += 2;
    }
    for (i = 0; in[i] != '\0' && n + 1 < size; i++) {
        char c = in[i] == '\\' ? '/' : (char)tolower((unsigned char)in[i]);
        if (c == '/' && n > 0 && out[n - 1] == '/') {
            continue;
        }
        out[n++] = c;
    }
    out[n] = '\0';
}

static void Pack_HashName(u64h *h, const char *name)
{
    for (; *name != '\0'; name++) {
        MIX(*h, (u64h)(unsigned char)tolower((unsigned char)*name));
    }
    MIX(*h, 0x2F);
}

/* A file's bytes (into h); 0 when it isn't there */
static int Pack_HashFile(u64h *h, const char *real)
{
    static unsigned char buf[1 << 16];
    FILE *f = fopen(real, "rb");
    size_t n, i;
    u64h total = 0;

    if (f == NULL) {
        return 0;
    }
    while ((n = fread(buf, 1, sizeof(buf), f)) > 0) {
        for (i = 0; i + 8 <= n; i += 8) {
            u64h w;
            memcpy(&w, buf + i, 8);
            MIX(*h, w);
        }
        for (; i < n; i++) {
            MIX(*h, (u64h)buf[i] | 0x100);
        }
        total += n;
    }
    fclose(f);
    MIX(*h, total);
    return 1;
}

static int Pack_Junk(const char *name)
{
    size_t n = strlen(name);
    return name[0] == '.' || strcasecmp(name, "thumbs.db") == 0 || strcasecmp(name, "desktop.ini") == 0
        || (n > 4 && strcasecmp(name + n - 4, ".log") == 0);
}

static int Pack_CompareNames(const void *a, const void *b)
{
    return strcasecmp(*(char *const *)a, *(char *const *)b);
}

/* Every file under a real folder, in name order (into h) */
static void Pack_HashDir(u64h *h, const char *real, int depth)
{
    DIR *dir = depth < 8 ? opendir(real) : NULL;
    struct dirent *e;
    char **names = NULL;
    int count = 0, cap = 0, i;

    if (dir == NULL) {
        MIX(*h, 0xD1D1);
        return;
    }
    while ((e = readdir(dir)) != NULL) {
        if (Pack_Junk(e->d_name)) {
            continue;
        }
        if (count == cap) {
            char **more = realloc(names, (cap = cap ? cap * 2 : 32) * sizeof(char *));
            if (more == NULL) {
                break;
            }
            names = more;
        }
        names[count++] = strdup(e->d_name);
    }
    closedir(dir);
    if (count > 1) {
        qsort(names, count, sizeof(char *), Pack_CompareNames);
    }
    for (i = 0; i < count; i++) {
        char path[0x400];
        struct stat st;
        snprintf(path, sizeof(path), "%s/%s", real, names[i]);
        if (stat(path, &st) == 0) {
            Pack_HashName(h, names[i]);
            if (S_ISDIR(st.st_mode)) {
                Pack_HashDir(h, path, depth + 1);
            } else {
                Pack_HashFile(h, path);
            }
        }
        free(names[i]);
    }
    free(names);
}

static void Pack_HashEnginePath(u64h *h, const char *path, int folder)
{
    char real[0x400];

    Pack_HashName(h, path);
    if (!MfHost_RealPath(path, real, sizeof(real))) {
        MIX(*h, 0xAB5E);
        return;
    }
    if (folder) {
        Pack_HashDir(h, real, 0);
    } else {
        Pack_HashFile(h, real);
    }
}

static unsigned int Pack_Fold(u64h h)
{
    return (unsigned int)(h ^ (h >> 32));
}

/* ------------------------------------------------------------------ reading the .def files (just enough) */

/* "key = value ; comment" in [group] of an engine file (any case); value without quotes */
static int Pack_DefValue(const char *path, const char *group, const char *key, char *out, size_t size)
{
    char real[0x400], line[0x400];
    FILE *f;
    int in = 0;

    out[0] = '\0';
    if (!MfHost_RealPath(path, real, sizeof(real)) || (f = fopen(real, "rb")) == NULL) {
        return 0;
    }
    while (fgets(line, sizeof(line), f) != NULL) {
        char *p = line, *eq, *end;
        while (*p == ' ' || *p == '\t') {
            p++;
        }
        if (*p == '[') {
            char *close = strchr(p, ']');
            in = close != NULL && (size_t)(close - p - 1) == strlen(group) && strncasecmp(p + 1, group, strlen(group)) == 0;
            continue;
        }
        if (!in || (eq = strchr(p, '=')) == NULL) {
            continue;
        }
        end = eq;
        while (end > p && (end[-1] == ' ' || end[-1] == '\t')) {
            end--;
        }
        if ((size_t)(end - p) != strlen(key) || strncasecmp(p, key, strlen(key)) != 0) {
            continue;
        }
        p = eq + 1;
        end = strchr(p, ';');
        if (end == NULL) {
            end = p + strlen(p);
        }
        while (*p == ' ' || *p == '\t' || *p == '"') {
            p++;
        }
        while (end > p && (end[-1] == ' ' || end[-1] == '\t' || end[-1] == '\r' || end[-1] == '\n' || end[-1] == '"')) {
            end--;
        }
        snprintf(out, size, "%.*s", (int)(end - p), p);
        fclose(f);
        return 1;
    }
    fclose(f);
    return 0;
}

/* A path a .def names: next to the .def, else from the pack's top folder */
static void Pack_Beside(const char *def, const char *name, char *out, size_t size)
{
    char real[0x400];
    const char *slash = strrchr(def, '/');

    if (slash != NULL) {
        snprintf(out, size, "%.*s/%s", (int)(slash - def), def, name);
        if (MfHost_RealPath(out, real, sizeof(real))) {
            return;
        }
    }
    snprintf(out, size, "%s", name);
}

static void Pack_AddStage(const char *path)
{
    char norm[0x80];
    int i;

    Pack_Norm(path, norm, sizeof(norm));
    if (norm[0] == '\0' || strcmp(norm, "random") == 0) {
        return;
    }
    for (i = 0; i < sStagePathCount; i++) {
        if (strcmp(sStagePaths[i], norm) == 0) {
            return;
        }
    }
    if (sStagePathCount < MF_PACK_MAX) {
        snprintf(sStagePaths[sStagePathCount++], sizeof(sStagePaths[0]), "%s", norm);
    }
}

static unsigned int Pack_StageHash(const char *def)
{
    u64h h = 0x57A6E;
    char value[0x200], path[0x400];

    Pack_HashEnginePath(&h, def, 0);
    if (Pack_DefValue(def, "BGdef", "spr", value, sizeof(value)) && value[0] != '\0') {
        Pack_Beside(def, value, path, sizeof(path));
        Pack_HashEnginePath(&h, path, 0);
    }
    if (Pack_DefValue(def, "Music", "bgmusic", value, sizeof(value)) && value[0] != '\0') {
        Pack_Beside(def, value, path, sizeof(path));
        Pack_HashEnginePath(&h, path, 0);
    }
    return Pack_Fold(h);
}

int MfPack_StageIndex(const char *path)
{
    char norm[0x80];
    int i;

    Pack_Norm(path, norm, sizeof(norm));
    for (i = 0; i < sStagePathCount; i++) {
        if (strcmp(sStagePaths[i], norm) == 0) {
            return i;
        }
    }
    return -1;
}

int MfEngine_PackSummary(const char *cabinet, struct MfPack *out)
{
    char motif[0x200], select[0x200], stages[0x400], real[0x400], line[0x400];
    static char extra[MF_PACK_MAX][0x80], own[MF_PACK_MAX][0x80];
    int extraCount = 0, ownCount = 0, inChars = 0, inExtra = 0, i;
    u64h core = 0xC0DE;
    FILE *f;

    memset(out, 0, sizeof(*out));
    sStagePathCount = 0;
    /* the screenpack: the cabinet's, else mugen.cfg's */
    if (!Pack_DefValue("cabinets.ini", cabinet, "motif", motif, sizeof(motif)) || motif[0] == '\0') {
        Pack_DefValue("data/mugen.cfg", "Options", "motif", motif, sizeof(motif));
    }
    if (motif[0] == '\0') {
        snprintf(motif, sizeof(motif), "data/system.def");
    }
    Pack_HashEnginePath(&core, "data", 1);
    Pack_HashEnginePath(&core, "font", 1);
    {
        char folder[0x200];
        const char *slash = strrchr(motif, '/');
        snprintf(folder, sizeof(folder), "%.*s", slash != NULL ? (int)(slash - motif) : 4, slash != NULL ? motif : "data");
        if (strcasecmp(folder, "data") != 0) {
            Pack_HashEnginePath(&core, folder, 1);
        }
    }
    if (!Pack_DefValue(motif, "Files", "select", select, sizeof(select)) || select[0] == '\0') {
        snprintf(select, sizeof(select), "select.def");
    }
    {
        char path[0x400];
        Pack_Beside(motif, select, path, sizeof(path));
        if (!MfHost_RealPath(path, real, sizeof(real))) {
            snprintf(path, sizeof(path), "data/%s", select);
            MfHost_RealPath(path, real, sizeof(real));
        }
        Pack_HashEnginePath(&core, path, 0);
    }
    out->core = Pack_Fold(core);

    /* the characters (and their own stages), then [ExtraStages] */
    f = fopen(real, "rb");
    while (f != NULL && fgets(line, sizeof(line), f) != NULL) {
        char *p = line, *end, name[0x200], dir[0x200];
        size_t n;
        while (*p == ' ' || *p == '\t') {
            p++;
        }
        end = strchr(p, ';');
        if (end != NULL) {
            *end = '\0';
        }
        n = strlen(p);
        while (n > 0 && (p[n - 1] == '\r' || p[n - 1] == '\n' || p[n - 1] == ' ' || p[n - 1] == '\t')) {
            p[--n] = '\0';
        }
        if (*p == '[') {
            inChars = strncasecmp(p, "[Characters]", 12) == 0;
            inExtra = strncasecmp(p, "[ExtraStages]", 13) == 0;
            continue;
        }
        if (n == 0) {
            continue;
        }
        if (inExtra && extraCount < MF_PACK_MAX) {
            snprintf(extra[extraCount++], sizeof(extra[0]), "%s", p);
            continue;
        }
        if (!inChars) {
            continue;
        }
        end = strchr(p, ',');
        snprintf(name, sizeof(name), "%.*s", end != NULL ? (int)(end - p) : (int)n, p);
        for (n = strlen(name); n > 0 && (name[n - 1] == ' ' || name[n - 1] == '\t'); n--) {
            name[n - 1] = '\0';
        }
        if (name[0] == '\0' || strcasecmp(name, "randomselect") == 0 || strchr(name, ':') != NULL) {
            continue;
        }
        if (out->chars < MF_PACK_MAX) {
            u64h h = 0xC4A2;
            const char *slash = strrchr(name, '/') != NULL ? strrchr(name, '/') : strrchr(name, '\\');
            if (slash != NULL) {
                snprintf(dir, sizeof(dir), "chars/%.*s", (int)(slash - name), name);
            } else {
                size_t k = strlen(name);
                snprintf(dir, sizeof(dir), "chars/%.*s", k > 4 && strcasecmp(name + k - 4, ".def") == 0 ? (int)k - 4 : (int)k,
                         name);
            }
            Pack_HashEnginePath(&h, dir, 1);
            out->charHash[out->chars++] = Pack_Fold(h);
        }
        /* its own stage: "name, stages/x.def, includestage=0" */
        if (end != NULL) {
            char stage[0x200];
            char *q = end + 1, *qe;
            while (*q == ' ' || *q == '\t') {
                q++;
            }
            qe = strchr(q, ',');
            snprintf(stage, sizeof(stage), "%.*s", qe != NULL ? (int)(qe - q) : (int)strlen(q), q);
            for (n = strlen(stage); n > 0 && (stage[n - 1] == ' ' || stage[n - 1] == '\t'); n--) {
                stage[n - 1] = '\0';
            }
            if (ownCount < MF_PACK_MAX) {
                snprintf(own[ownCount++], sizeof(own[0]), "%s", stage);
            }
        }
    }
    if (f != NULL) {
        fclose(f);
    }
    /* the stages the cabinet can pick: its list, else [ExtraStages] and each character's own */
    if (Pack_DefValue("cabinets.ini", cabinet, "stages", stages, sizeof(stages)) && stages[0] != '\0') {
        char *s = stages;
        while (s != NULL && *s != '\0') {
            char *e = strchr(s, ','), name[0x80], path[0x100];
            size_t k;
            if (e != NULL) {
                *e = '\0';
            }
            while (*s == ' ' || *s == '\t') {
                s++;
            }
            snprintf(name, sizeof(name), "%s", s);
            for (k = strlen(name); k > 0 && (name[k - 1] == ' ' || name[k - 1] == '\t'); k--) {
                name[k - 1] = '\0';
            }
            k = strlen(name);
            if (k > 0) {
                snprintf(path, sizeof(path), "%s%s%s", strchr(name, '/') || strchr(name, '\\') ? "" : "stages/", name,
                         k > 4 && strcasecmp(name + k - 4, ".def") == 0 ? "" : ".def");
                Pack_AddStage(path);
            }
            s = e != NULL ? e + 1 : NULL;
        }
    } else {
        for (i = 0; i < extraCount; i++) {
            Pack_AddStage(extra[i]);
        }
        for (i = 0; i < ownCount; i++) {
            Pack_AddStage(own[i]);
        }
    }
    for (i = 0; i < sStagePathCount; i++) {
        out->stageHash[i] = Pack_StageHash(sStagePaths[i]);
    }
    out->stages = (unsigned char)sStagePathCount;
    return 1;
}

int MfEngine_CabinetText(const char *cabinet, char *out, int size)
{
    char real[0x400], line[0x400];
    FILE *f;
    int in = 0, n = 0;

    out[0] = '\0';
    if (!MfHost_RealPath("cabinets.ini", real, sizeof(real)) || (f = fopen(real, "rb")) == NULL) {
        return 0;
    }
    while (fgets(line, sizeof(line), f) != NULL) {
        char *p = line;
        size_t len;
        while (*p == ' ' || *p == '\t') {
            p++;
        }
        if (*p == '[') {
            char *close = strchr(p, ']');
            if (in) {
                break;
            }
            in = close != NULL && (size_t)(close - p - 1) == strlen(cabinet) && strncasecmp(p + 1, cabinet, strlen(cabinet)) == 0;
        }
        if (!in || *p == ';') {
            continue;
        }
        len = strlen(p);
        if (n + (int)len + 1 >= size) {
            break;
        }
        memcpy(out + n, p, len + 1);
        n += (int)len;
    }
    fclose(f);
    return n > 0;
}
