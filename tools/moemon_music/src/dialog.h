// Moemon Music Studio: the system's file dialogs (Windows common dialogs, zenity / kdialog on
// Linux). Each returns 1 with a path, 0 when cancelled, -1 when there is no dialog to show (the
// window then asks for the path itself).
#ifndef DIALOG_H
#define DIALOG_H

int dlg_open(char *out, int n, const char *title, const char *filterName, const char *patterns);
int dlg_save(char *out, int n, const char *title, const char *filterName, const char *pattern, const char *defName);
int dlg_folder(char *out, int n, const char *title);

#endif
