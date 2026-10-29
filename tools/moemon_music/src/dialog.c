#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dialog.h"

#ifdef _WIN32
#include <windows.h>
#include <commdlg.h>
#include <shlobj.h>

// patterns: "*.mid;*.midi"
static void make_filter(char *buf, int n, const char *name, const char *patterns)
{
    int k = snprintf(buf, n, "%s (%s)", name, patterns) + 1;
    k += snprintf(buf + k, n - k, "%s", patterns) + 1;
    k += snprintf(buf + k, n - k, "All files") + 1;
    k += snprintf(buf + k, n - k, "*.*") + 1;
    buf[k] = 0;
}

int dlg_open(char *out, int n, const char *title, const char *filterName, const char *patterns)
{
    char filter[512];
    make_filter(filter, sizeof(filter), filterName, patterns);
    OPENFILENAMEA o;
    memset(&o, 0, sizeof(o));
    out[0] = 0;
    o.lStructSize = sizeof(o);
    o.lpstrFilter = filter;
    o.lpstrFile = out;
    o.nMaxFile = (DWORD)n;
    o.lpstrTitle = title;
    o.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
    return GetOpenFileNameA(&o) ? 1 : 0;
}

int dlg_save(char *out, int n, const char *title, const char *filterName, const char *pattern, const char *defName)
{
    char filter[512];
    make_filter(filter, sizeof(filter), filterName, pattern);
    OPENFILENAMEA o;
    memset(&o, 0, sizeof(o));
    snprintf(out, n, "%s", defName ? defName : "");
    o.lStructSize = sizeof(o);
    o.lpstrFilter = filter;
    o.lpstrFile = out;
    o.nMaxFile = (DWORD)n;
    o.lpstrTitle = title;
    o.lpstrDefExt = pattern[0] == '*' && pattern[1] == '.' ? pattern + 2 : NULL;
    o.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
    return GetSaveFileNameA(&o) ? 1 : 0;
}

int dlg_folder(char *out, int n, const char *title)
{
    BROWSEINFOA b;
    memset(&b, 0, sizeof(b));
    b.lpszTitle = title;
    b.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    LPITEMIDLIST id = SHBrowseForFolderA(&b);
    if (!id) return 0;
    char path[MAX_PATH];
    int ok = SHGetPathFromIDListA(id, path);
    CoTaskMemFree(id);
    if (!ok) return 0;
    snprintf(out, n, "%s", path);
    return 1;
}

#else

static int have(const char *prog)
{
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "command -v %s >/dev/null 2>&1", prog);
    return system(cmd) == 0;
}

static void shell_quote(char *buf, int n, const char *s)
{
    int k = 0;
    if (k < n - 1) buf[k++] = '\'';
    for (; *s && k < n - 5; s++) {
        if (*s == '\'') { buf[k++] = '\''; buf[k++] = '\\'; buf[k++] = '\''; buf[k++] = '\''; }
        else buf[k++] = *s;
    }
    buf[k++] = '\'';
    buf[k] = 0;
}

static int run(const char *cmd, char *out, int n)
{
    FILE *p = popen(cmd, "r");
    if (!p) return -1;
    out[0] = 0;
    if (!fgets(out, n, p)) out[0] = 0;
    int rc = pclose(p);
    size_t l = strlen(out);
    while (l && (out[l - 1] == '\n' || out[l - 1] == '\r')) out[--l] = 0;
    return rc == 0 && out[0] ? 1 : 0;
}

// "*.mid;*.midi" -> "*.mid *.midi"
static void spaced(char *buf, int n, const char *patterns)
{
    snprintf(buf, n, "%s", patterns);
    for (char *c = buf; *c; c++) if (*c == ';') *c = ' ';
}

int dlg_open(char *out, int n, const char *title, const char *filterName, const char *patterns)
{
    char cmd[1024], t[256], f[256], pats[128];
    spaced(pats, sizeof(pats), patterns);
    shell_quote(t, sizeof(t), title);
    if (have("zenity")) {
        char filt[200];
        snprintf(filt, sizeof(filt), "%s | %s", filterName, pats);
        shell_quote(f, sizeof(f), filt);
        snprintf(cmd, sizeof(cmd), "zenity --file-selection --title=%s --file-filter=%s 2>/dev/null", t, f);
        return run(cmd, out, n);
    }
    if (have("kdialog")) {
        char filt[200];
        snprintf(filt, sizeof(filt), "%s (%s)", pats, filterName);
        shell_quote(f, sizeof(f), filt);
        snprintf(cmd, sizeof(cmd), "kdialog --title %s --getopenfilename . %s 2>/dev/null", t, f);
        return run(cmd, out, n);
    }
    return -1;
}

int dlg_save(char *out, int n, const char *title, const char *filterName, const char *pattern, const char *defName)
{
    char cmd[1400], t[256], f[256], d[600];
    shell_quote(t, sizeof(t), title);
    shell_quote(d, sizeof(d), defName ? defName : "");
    if (have("zenity")) {
        char filt[200];
        snprintf(filt, sizeof(filt), "%s | %s", filterName, pattern);
        shell_quote(f, sizeof(f), filt);
        snprintf(cmd, sizeof(cmd), "zenity --file-selection --save --confirm-overwrite --title=%s --filename=%s --file-filter=%s 2>/dev/null",
                 t, d, f);
        return run(cmd, out, n);
    }
    if (have("kdialog")) {
        char filt[200];
        snprintf(filt, sizeof(filt), "%s (%s)", pattern, filterName);
        shell_quote(f, sizeof(f), filt);
        snprintf(cmd, sizeof(cmd), "kdialog --title %s --getsavefilename %s %s 2>/dev/null", t, d, f);
        return run(cmd, out, n);
    }
    return -1;
}

int dlg_folder(char *out, int n, const char *title)
{
    char cmd[600], t[256];
    shell_quote(t, sizeof(t), title);
    if (have("zenity")) {
        snprintf(cmd, sizeof(cmd), "zenity --file-selection --directory --title=%s 2>/dev/null", t);
        return run(cmd, out, n);
    }
    if (have("kdialog")) {
        snprintf(cmd, sizeof(cmd), "kdialog --title %s --getexistingdirectory . 2>/dev/null", t);
        return run(cmd, out, n);
    }
    return -1;
}

#endif
