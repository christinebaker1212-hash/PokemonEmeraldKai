// Moemon VMU manager: the system's own file windows (Windows Explorer; zenity on Linux).
#pragma once
#include <string>
#include <vector>

namespace filedlg {

enum class Kind { Open, Save, Folder };
enum class Result { Picked, Cancelled, Unavailable };

// exts: lowercase with the dot (".bin"); the first is a Save's default. Blocks until the window closes.
Result show(Kind kind, const std::string& title, const std::vector<std::string>& exts, const std::string& name,
            const std::string& startDir, std::string& out);

}  // namespace filedlg
