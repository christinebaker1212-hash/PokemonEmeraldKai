// Moemon VMU manager: the system's own file windows (Windows Explorer's Open / Save / pick a folder; zenity on Linux).
#include "filedlg.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shobjidl.h>
#include <string>
#include <vector>

namespace filedlg {

static std::wstring widen(const std::string& s) {
    if(s.empty()) return std::wstring();
    int n = MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), nullptr, 0);
    std::wstring w(n, 0);
    MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), &w[0], n);
    return w;
}

static std::string narrow(const wchar_t* w) {
    int n = WideCharToMultiByte(CP_UTF8, 0, w, -1, nullptr, 0, nullptr, nullptr);
    std::string s(n > 0 ? n - 1 : 0, 0);
    if(n > 1) WideCharToMultiByte(CP_UTF8, 0, w, -1, &s[0], n, nullptr, nullptr);
    return s;
}

static Result run(Kind kind, const std::string& title, const std::vector<std::string>& exts, const std::string& name,
                  const std::string& startDir, std::string& out) {
    HRESULT init = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    IFileDialog* dlg = nullptr;
    HRESULT hr = CoCreateInstance(kind == Kind::Save ? CLSID_FileSaveDialog : CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER,
                                  kind == Kind::Save ? IID_IFileSaveDialog : IID_IFileOpenDialog, (void**)&dlg);
    if(FAILED(hr) || !dlg) {
        if(SUCCEEDED(init)) CoUninitialize();
        return Result::Unavailable;
    }
    DWORD opts = 0;
    dlg->GetOptions(&opts);
    opts |= FOS_FORCEFILESYSTEM | FOS_NOCHANGEDIR;
    if(kind == Kind::Folder) opts |= FOS_PICKFOLDERS;
    if(kind == Kind::Open) opts |= FOS_FILEMUSTEXIST;
    if(kind == Kind::Save) opts |= FOS_OVERWRITEPROMPT;
    dlg->SetOptions(opts);
    std::wstring wt = widen(title);
    dlg->SetTitle(wt.c_str());
    // the filter: "VMU files (*.bin;*.vmu)" and "All files"
    std::wstring spec, label;
    for(auto& e : exts) {
        if(!spec.empty()) spec += L";";
        spec += L"*" + widen(e);
    }
    label = (spec.empty() ? L"" : spec);
    COMDLG_FILTERSPEC specs[2] = {{label.c_str(), spec.c_str()}, {L"All files", L"*.*"}};
    if(kind != Kind::Folder && !spec.empty()) {
        dlg->SetFileTypes(2, specs);
        dlg->SetFileTypeIndex(1);
        if(kind == Kind::Save && !exts.empty()) {
            std::wstring def = widen(exts[0].size() > 1 ? exts[0].substr(1) : exts[0]);
            dlg->SetDefaultExtension(def.c_str());
        }
    }
    if(!name.empty()) {
        std::wstring wn = widen(name);
        dlg->SetFileName(wn.c_str());
    }
    if(!startDir.empty()) {
        IShellItem* folder = nullptr;
        std::wstring wd = widen(startDir);
        if(SUCCEEDED(SHCreateItemFromParsingName(wd.c_str(), nullptr, IID_IShellItem, (void**)&folder)) && folder) {
            dlg->SetFolder(folder);
            folder->Release();
        }
    }
    Result r = Result::Cancelled;
    if(SUCCEEDED(dlg->Show(GetActiveWindow()))) {
        IShellItem* item = nullptr;
        if(SUCCEEDED(dlg->GetResult(&item)) && item) {
            PWSTR path = nullptr;
            if(SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &path)) && path) {
                out = narrow(path);
                CoTaskMemFree(path);
                r = Result::Picked;
            }
            item->Release();
        }
    }
    dlg->Release();
    if(SUCCEEDED(init)) CoUninitialize();
    return r;
}

Result show(Kind kind, const std::string& title, const std::vector<std::string>& exts, const std::string& name,
            const std::string& startDir, std::string& out) {
    return run(kind, title, exts, name, startDir, out);
}

}  // namespace filedlg

#else  // Linux: zenity when it's there, else the manager's own browser

#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

namespace filedlg {

static std::string quote(const std::string& s) {
    std::string q = "'";
    for(char c : s) {
        if(c == '\'') q += "'\\''";
        else q += c;
    }
    return q + "'";
}

Result show(Kind kind, const std::string& title, const std::vector<std::string>& exts, const std::string& name,
            const std::string& startDir, std::string& out) {
    if(getenv("VMU_MANAGER_NO_NATIVE") || system("command -v zenity >/dev/null 2>&1") != 0 || !getenv("DISPLAY"))
        return Result::Unavailable;
    std::string cmd = "zenity --file-selection --title=" + quote(title);
    if(kind == Kind::Folder) cmd += " --directory";
    if(kind == Kind::Save) cmd += " --save --confirm-overwrite";
    std::string start = startDir.empty() ? "" : startDir + "/";
    if(!name.empty() || !start.empty()) cmd += " --filename=" + quote(start + name);
    if(!exts.empty() && kind != Kind::Folder) {
        std::string f;
        for(auto& e : exts) f += " *" + e;
        cmd += " --file-filter=" + quote("VMU files |" + f) + " --file-filter=" + quote("All files | *");
    }
    cmd += " 2>/dev/null";
    FILE* p = popen(cmd.c_str(), "r");
    if(!p) return Result::Unavailable;
    char buf[4096];
    std::string got;
    while(fgets(buf, sizeof buf, p)) got += buf;
    int rc = pclose(p);
    while(!got.empty() && (got.back() == '\n' || got.back() == '\r')) got.pop_back();
    if(rc != 0 || got.empty()) return Result::Cancelled;
    out = got;
    return Result::Picked;
}

}  // namespace filedlg

#endif
