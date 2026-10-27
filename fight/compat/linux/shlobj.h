/* Moemon Fight: <shlobj.h> for the Linux test build (no system font folder). */
#ifndef MF_LINUX_SHLOBJ_H
#define MF_LINUX_SHLOBJ_H
#define CSIDL_FONTS 0x0014
#define S_OK 0
static inline long SHGetFolderPathA(HWND owner, int folder, HANDLE token, DWORD flags, char *path)
{
    return -1;
}
#endif
