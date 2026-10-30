/*
 * Error reporting (original module around 0x40BAA0-0x40C7CA).
 *
 * Errors go to the error clipboard once the game variables exist, to stderr
 * before that. Error_Exit shows them in a dialog together with the tail of
 * the debug clipboard (or of mugen.log) and quits. Player warnings go to the
 * debug clipboard.
 */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#ifdef MF_ORIGINAL_DIALOG
#include <richedit.h>
#endif
#include <SDL.h>
#include <SDL_syswm.h>

#include "mugen.h"

#define ERROR_TEXT_SIZE      0x4000
#define ERROR_DIALOG_ID      200     /* dialog resource in mugen.exe */
#define ERROR_DIALOG_EDIT_ID 0xC9
#define ERROR_DIALOG_W       480
#define ERROR_DIALOG_H       384
#define ERROR_TAIL_LINES     20

typedef struct ErrorDialogText {
    const char *text;
    const char *title;
} ErrorDialogText;

/* 0x0040C0F0 */
void Error_Fatal(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    if (g_game != NULL && g_game->error_text != NULL) {
        TextRing_VPrintf(g_game->error_text, ap, fmt);
    } else {
        vfprintf(stderr, fmt, ap);
    }
    va_end(ap);
    Error_Exit();
}

/* 0x0040C140 */
void Error_Printf(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    if (g_game != NULL && g_game->error_text != NULL) {
        TextRing_VPrintf(g_game->error_text, ap, fmt);
    } else {
        vfprintf(stderr, fmt, ap);
    }
    va_end(ap);
}

/* 0x0040C190 */
void Error_OutOfMemory(void)
{
    if (g_game != NULL && g_game->error_text != NULL) {
        TextRing_Printf(g_game->error_text, "Out of memory");
    } else {
        fprintf(stderr, "Out of memory");
    }
}

/* 0x0040BD90 */
void Error_Exit(void)
{
    char tmp[0x1000];
    char text[ERROR_TEXT_SIZE];
    TextRing *tail = g_game->clipboard;
    size_t len;

    if (tail == NULL) {
        tail = g_log != NULL ? g_log->mirror : NULL;
    }
    text[0] = '\0';
    if (g_libraryError != 0) {
        const char *msg;
        switch (g_libraryError) {
        case LIBERR_FONT:
            msg = "Can't load fonts";
            break;
        case LIBERR_SOUND:
            msg = "Can't init sound";
            break;
        case LIBERR_GRAPHICS:
            msg = "Can't init graphics";
            break;
        case LIBERR_VIDEO_MODE:
            msg = "Can't change video mode";
            break;
        default:
            msg = "Error not defined";
            break;
        }
        _snprintf(tmp, sizeof(tmp), "Library (built in) message: %s.\n\n", msg);
        Str_Append(text, tmp, sizeof(text));
    }
    if (g_libraryErrorText[0] != '\0') {
        _snprintf(tmp, sizeof(tmp), "Library error message: %s\n\n", g_libraryErrorText);
        Str_Append(text, tmp, sizeof(text));
    }
    if (g_outOfMemory) {
        _snprintf(tmp, sizeof(tmp), "Out of memory error.\n\n");
        Str_Append(text, tmp, sizeof(text));
    }
    Str_Append(text, "Error detected.\n\n", sizeof(text));
    if (g_game != NULL && g_game->error_text != NULL) {
        len = strlen(text);
        TextRing_Dump(g_game->error_text, text + len, -1, (unsigned int)(sizeof(text) - len));
        Str_Append(text, "\n", sizeof(text));
    }
    Str_Append(text, "Clipboard tail:\n", sizeof(text));
    len = strlen(text);
    TextRing_Dump(tail, text + len, ERROR_TAIL_LINES, (unsigned int)(sizeof(text) - len));

    if (g_game != NULL && g_game->sound_enabled) {
        Sound_Deinit();
    }
    g_onActivate = NULL;
    if (g_game != NULL && g_game->game_dir[0] != '\0') {
        _chdir(g_game->game_dir);
    }
    text[sizeof(text) - 1] = '\0';
    Error_ShowDialog(text, "M.U.G.E.N Error");
    Log_Printf("\n%s", text);
    Log_Printf("Exit with an error.\n");
    if (g_log != NULL) {
        Log_Printf("\nLog file successfully closed.\n");
        Log_Close();
        g_log = NULL;
    }
    GameVars_Free();
    SDL_Quit();
    exit(1);
}

#ifdef MF_ORIGINAL_DIALOG   /* Moemon Fight: the game shows the error instead (MfHost_ErrorDialog) */
/* 0x0040C4C0: read-only rich edit with the text, sized to the dialog, above the OK button. */
static INT_PTR CALLBACK ErrorDialog_Proc(HWND dlg, UINT msg, WPARAM wparam, LPARAM lparam)
{
    if (msg == WM_INITDIALOG) {
        const ErrorDialogText *data = (const ErrorDialogText *)lparam;
        int screen_w = GetSystemMetrics(SM_CXSCREEN);
        int screen_h = GetSystemMetrics(SM_CYSCREEN);
        SETTEXTEX settext;
        CHARFORMATA format;
        RECT client, ok_rect;
        HWND ok, edit;
        int ok_w, ok_h;

        CreateSolidBrush(0xFFFFFF);     /* unused, as in the original */
        settext.flags = ST_DEFAULT;
        settext.codepage = CP_UTF8;
        memset(&format, 0, sizeof(format));
        format.cbSize = sizeof(format);
        format.dwMask = CFM_FACE | CFM_SIZE;
        format.yHeight = 180;           /* twips: 9 pt */
        strcpy(format.szFaceName, "Courier New");
        ok = GetDlgItem(dlg, IDOK);
        if (g_richEditLib == NULL) {
            g_richEditLib = LoadLibraryA("Riched20.dll");
            if (g_richEditLib == NULL) {
                return FALSE;
            }
        }
        edit = CreateWindowExA(0, "RichEdit20A", "",
                               WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_BORDER | WS_TABSTOP |
                                   ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_READONLY,
                               0, 0, 100, 100, dlg, (HMENU)ERROR_DIALOG_EDIT_ID, GetModuleHandleA(NULL), NULL);
        if (edit == NULL || ok == NULL) {
            return FALSE;
        }
        SendMessageA(dlg, WM_SETTEXT, 0, (LPARAM)data->title);
        SendMessageA(edit, EM_SETTEXTMODE, TM_PLAINTEXT, 0);
        SendMessageA(edit, EM_SETCHARFORMAT, SCF_DEFAULT, (LPARAM)&format);
        SendMessageA(edit, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELONG(10, 10));
        SendMessageA(edit, EM_SETTEXTEX, (WPARAM)&settext, (LPARAM)data->text);
        MoveWindow(dlg, (screen_w - ERROR_DIALOG_W) / 2, (int)((screen_h - ERROR_DIALOG_H) * 0.45),
                   ERROR_DIALOG_W, ERROR_DIALOG_H, FALSE);
        GetClientRect(dlg, &client);
        GetClientRect(ok, &ok_rect);
        ok_h = ok_rect.bottom - ok_rect.top;
        ok_w = ok_rect.right - ok_rect.left;
        MoveWindow(edit, 0, 0, client.right, client.bottom - ok_h - 22, FALSE);
        MoveWindow(ok, client.right - ok_w - 9, client.bottom - ok_h - 11, ok_w, ok_h, FALSE);
        return TRUE;
    }
    if (msg == WM_COMMAND) {
        char unused[0x100];
        switch (LOWORD(wparam)) {
        case IDOK:
            GetDlgItemTextA(dlg, ERROR_DIALOG_EDIT_ID, unused, sizeof(unused));
            EndDialog(dlg, IDOK);
            break;
        case IDCANCEL:
            EndDialog(dlg, IDCANCEL);
            break;
        }
        return TRUE;
    }
    return FALSE;
}

/* 0x0040C770: modal, owned by the game window when there is one. */
int Error_ShowDialog(const char *text, const char *title)
{
    SDL_SysWMinfo info;
    ErrorDialogText data;
    HWND parent;

    memset(&info, 0, sizeof(info));     /* the version is left at 0.0.0 */
    parent = SDL_GetWMInfo(&info) ? info.window : NULL;
    data.text = text;
    data.title = title;
    DialogBoxParamA(GetModuleHandleA(NULL), MAKEINTRESOURCEA(ERROR_DIALOG_ID), parent, ErrorDialog_Proc,
                    (LPARAM)&data);
    return 0;
}
#else
/* 0x0040C770. Moemon Fight: the text goes to the game, which ends the fight and says why. */
int Error_ShowDialog(const char *text, const char *title)
{
    MfHost_ErrorDialog(text, title);
    return 0;
}
#endif

#define PLAYER_WARNING "%-4i: Warning: Player %s (%i) in state %i: %s\n"

/* 0x0040BAA0 */
void Player_Warn(Player *p, const char *msg)
{
    if (p != NULL) {
        TextRing_Printf(g_game->clipboard, PLAYER_WARNING, g_game->frames, p->name, p->id, p->state_no, msg);
    }
}

/* 0x0040BAE0: the message is not terminated when it fills the buffer. */
/* Moemon Fight: tests (the character audit) see the players' warnings: MOEMON_FIGHT_WARN=1 prints them */
static void Error_TestWarn(Player *p, int line, const char *msg)
{
    static int on = -1;
    if (on < 0) {
        on = getenv("MOEMON_FIGHT_WARN") != NULL;
    }
    if (on && p != NULL) {
        printf("fightwarn: %s in state %d%s%.0d: %s%s", p->name, p->state_no, line != -1 ? " line " : "",
               line != -1 ? line : 0, msg, msg[0] != '\0' && msg[strlen(msg) - 1] == '\n' ? "" : "\n");
    }
}

void Player_Warnf(Player *p, const char *fmt, ...)
{
    char msg[0x800];
    va_list ap;

    if (p != NULL) {
        va_start(ap, fmt);
        _vsnprintf(msg, sizeof(msg), fmt, ap);
        TextRing_Printf(g_game->clipboard, PLAYER_WARNING, g_game->frames, p->name, p->id, p->state_no, msg);
        Error_TestWarn(p, -1, msg);
    }
}

/* 0x0040BB50: names the CNS line of the state controller when known. */
void Player_WarnAt(Player *p, const StateCtrl *sc, const char *fmt, ...)
{
    char msg[0x800];
    int line = -1;
    va_list ap;

    if (p == NULL) {
        return;
    }
    if (sc != NULL && sc->source != NULL) {
        line = sc->source->line;
    }
    va_start(ap, fmt);
    _vsnprintf(msg, sizeof(msg), fmt, ap);
    Error_TestWarn(p, line, msg);
    if (line != -1) {
        TextRing_Printf(g_game->clipboard, "%-4i: Warning: Player %s (%i) in state %i (line %i): %s\n",
                        g_game->frames, p->name, p->id, p->state_no, line, msg);
    } else {
        TextRing_Printf(g_game->clipboard, PLAYER_WARNING, g_game->frames, p->name, p->id, p->state_no, msg);
    }
}

/* 0x0040BC20: a trigger redirected from player "from" to p failed. */
void Player_WarnRedirected(Player *p, Player *from, const char *msg)
{
    if (p != NULL) {
        Error_TestWarn(p, -1, msg);
        TextRing_Printf(g_game->clipboard,
                        "%-4i: Warning: Player %s (%i) in state %i: in redirected trigger from player %s (%i) in state %i: %s\n",
                        g_game->frames, p->name, p->id, p->state_no, from->name, from->id, from->state_no, msg);
    }
}

/* 0x0040BC70 */
void Clipboard_Printf(const char *fmt, ...)
{
    TextRing *ring = g_game->clipboard;
    char text[LOG_LINE_SIZE];
    va_list ap;

    va_start(ap, fmt);
    _vsnprintf(text, sizeof(text), fmt, ap);
    TextRing_Append(ring, text);
}
/*
 * 0x0040BCC0: draws the last max_lines lines of ring (all of them with -1)
 * from y downwards, then the line being written when it is not empty.
 */
void Clipboard_Draw(TextRing *ring, int max_lines, Font *font, int y)
{
    int saved_wrap;
    int i;
    const char *text;

    if (ring == NULL) {
        return;
    }
    saved_wrap = g_fontWrapMode;
    i = ring->head;
    g_fontWrapMode = FONT_WRAP_WORD;
    if (max_lines != -1) {
        int last = ring->cur;
        if (last < i) {
            last += ring->count;
        }
        if (last - i + 1 > max_lines) {
            i = ring->cur - max_lines;
            if (i < 0) {
                i += ring->count;
            }
        }
    }
    while (i != ring->cur) {
        Font_DrawText(ring->lines[i], font, 10, y, FONT_ALIGN_LEFT, 0, 0x100, 0x100, 0x100);
        y += (font->size_h + font->spacing_y) * g_fontLineCount;
        if (++i >= ring->count) {
            i = 0;
        }
    }
    text = ring->lines[i];
    if (*text != '\0') {
        Font_DrawText(text, font, 10, y, FONT_ALIGN_LEFT, 0, 0x100, 0x100, 0x100);
    }
    g_fontWrapMode = saved_wrap;
}