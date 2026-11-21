// Moemon VMU manager: the VM2 over USB (vm2link.h; the protocol in docs/VM2_USB_PROTOCOL.md).
#include "vm2link.h"

#include <chrono>
#include <cstring>
#include <ctime>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <setupapi.h>
#include <initguid.h>
#include <devguid.h>
#else
#include <dirent.h>
#include <fcntl.h>
#include <poll.h>
#include <termios.h>
#include <unistd.h>
#include <fstream>
#endif

namespace vm2 {

static const int kTimeout = 500;     // the GUI's
static const int kSdTimeout = 5000;  // its SD card commands'

static bool contains(const std::string& s, const char* what) { return s.find(what) != std::string::npos; }
static bool isNack(const std::string& s) { return contains(s, "NACK"); }
static bool isAck(const std::string& s) { return contains(s, "ACK") && !isNack(s); }

// ---- the serial port ----

#ifdef _WIN32

bool Serial::open(const std::string& name, int baud, std::string* err) {
    close();
    std::string path = "\\\\.\\" + name;
    HANDLE h = CreateFileA(path.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
    if(h == INVALID_HANDLE_VALUE) {
        if(err) *err = "can't open " + name + " (in use by another program, e.g. the VM2 GUI?)";
        return false;
    }
    DCB dcb;
    memset(&dcb, 0, sizeof dcb);
    dcb.DCBlength = sizeof dcb;
    GetCommState(h, &dcb);
    dcb.BaudRate = (DWORD)baud;
    dcb.ByteSize = 8;
    dcb.Parity = NOPARITY;
    dcb.StopBits = ONESTOPBIT;
    dcb.fBinary = TRUE;
    dcb.fParity = FALSE;
    dcb.fOutxCtsFlow = FALSE;
    dcb.fOutxDsrFlow = FALSE;
    dcb.fDtrControl = DTR_CONTROL_DISABLE;  // .NET's SerialPort default
    dcb.fRtsControl = RTS_CONTROL_DISABLE;
    dcb.fOutX = FALSE;
    dcb.fInX = FALSE;
    if(!SetCommState(h, &dcb)) {
        CloseHandle(h);
        if(err) *err = "can't set " + name + " to " + std::to_string(baud) + " baud";
        return false;
    }
    SetupComm(h, 1 << 16, 1 << 16);
    PurgeComm(h, PURGE_RXCLEAR | PURGE_TXCLEAR);
    handle_ = h;
    have_ = at_ = 0;
    return true;
}

void Serial::close() {
    if(handle_) CloseHandle((HANDLE)handle_);
    handle_ = nullptr;
    have_ = at_ = 0;
}

bool Serial::isOpen() const { return handle_ != nullptr; }

bool Serial::write(const void* data, size_t n) {
    if(!handle_) return false;
    const uint8_t* p = (const uint8_t*)data;
    while(n) {
        DWORD done = 0;
        if(!WriteFile((HANDLE)handle_, p, (DWORD)n, &done, nullptr)) return false;
        p += done;
        n -= done;
    }
    return true;
}

int Serial::readByte(int timeoutMs) {
    if(at_ < have_) return buf_[at_++];
    if(!handle_) return -1;
    COMMTIMEOUTS t;
    t.ReadIntervalTimeout = MAXDWORD;          // with the two below: return as soon as anything is there
    t.ReadTotalTimeoutMultiplier = MAXDWORD;
    t.ReadTotalTimeoutConstant = (DWORD)timeoutMs;
    t.WriteTotalTimeoutMultiplier = 0;
    t.WriteTotalTimeoutConstant = 2000;
    SetCommTimeouts((HANDLE)handle_, &t);
    DWORD got = 0;
    if(!ReadFile((HANDLE)handle_, buf_, sizeof buf_, &got, nullptr) || got == 0) return -1;
    have_ = got;
    at_ = 1;
    return buf_[0];
}

void Serial::flushInput() {
    have_ = at_ = 0;
    if(handle_) PurgeComm((HANDLE)handle_, PURGE_RXCLEAR);
}

std::vector<PortInfo> listPorts() {
    std::vector<PortInfo> out;
    HDEVINFO set = SetupDiGetClassDevsA(&GUID_DEVCLASS_PORTS, nullptr, nullptr, DIGCF_PRESENT);
    if(set != INVALID_HANDLE_VALUE) {
        SP_DEVINFO_DATA info;
        info.cbSize = sizeof info;
        for(DWORD i = 0; SetupDiEnumDeviceInfo(set, i, &info); ++i) {
            char friendly[256] = "";
            SetupDiGetDeviceRegistryPropertyA(set, &info, SPDRP_FRIENDLYNAME, nullptr, (PBYTE)friendly, sizeof friendly, nullptr);
            std::string f = friendly;
            size_t a = f.rfind("(COM"), b = f.rfind(')');
            if(a == std::string::npos || b == std::string::npos || b < a) continue;
            PortInfo p;
            p.name = f.substr(a + 1, b - a - 1);
            p.description = f;
            p.ch340 = contains(f, "CH340");
            out.push_back(p);
        }
        SetupDiDestroyDeviceInfoList(set);
    }
    return out;
}

#else  // POSIX

static speed_t speedFor(int baud) {
    switch(baud) {
#ifdef B921600
        case 921600: return B921600;
#endif
#ifdef B460800
        case 460800: return B460800;
#endif
        case 230400: return B230400;
        case 115200: return B115200;
        default: return B115200;
    }
}

bool Serial::open(const std::string& name, int baud, std::string* err) {
    close();
    int fd = ::open(name.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
    if(fd < 0) { if(err) *err = "can't open " + name; return false; }
    struct termios t;
    if(tcgetattr(fd, &t) == 0) {
        cfmakeraw(&t);
        t.c_cflag |= CLOCAL | CREAD;
        t.c_cflag &= ~(CRTSCTS | CSTOPB | PARENB);
        cfsetispeed(&t, speedFor(baud));
        cfsetospeed(&t, speedFor(baud));
        tcsetattr(fd, TCSANOW, &t);
    }
    tcflush(fd, TCIOFLUSH);
    fd_ = fd;
    have_ = at_ = 0;
    return true;
}

void Serial::close() {
    if(fd_ >= 0) ::close(fd_);
    fd_ = -1;
    have_ = at_ = 0;
}

bool Serial::isOpen() const { return fd_ >= 0; }

bool Serial::write(const void* data, size_t n) {
    const uint8_t* p = (const uint8_t*)data;
    while(n) {
        ssize_t w = ::write(fd_, p, n);
        if(w < 0) {
            struct pollfd pf = {fd_, POLLOUT, 0};
            if(poll(&pf, 1, 2000) <= 0) return false;
            continue;
        }
        p += w;
        n -= (size_t)w;
    }
    return true;
}

int Serial::readByte(int timeoutMs) {
    if(at_ < have_) return buf_[at_++];
    if(fd_ < 0) return -1;
    struct pollfd pf = {fd_, POLLIN, 0};
    if(poll(&pf, 1, timeoutMs) <= 0) return -1;
    ssize_t got = ::read(fd_, buf_, sizeof buf_);
    if(got <= 0) return -1;
    have_ = (size_t)got;
    at_ = 1;
    return buf_[0];
}

void Serial::flushInput() {
    have_ = at_ = 0;
    if(fd_ >= 0) tcflush(fd_, TCIFLUSH);
}

static std::string readSmall(const std::string& path) {
    std::ifstream f(path);
    std::string s;
    std::getline(f, s);
    return s;
}

std::vector<PortInfo> listPorts() {
    std::vector<PortInfo> out;
    DIR* d = opendir("/sys/class/tty");
    if(!d) return out;
    while(dirent* e = readdir(d)) {
        std::string n = e->d_name;
        if(n.rfind("ttyUSB", 0) != 0 && n.rfind("ttyACM", 0) != 0) continue;
        PortInfo p;
        p.name = "/dev/" + n;
        std::string base = "/sys/class/tty/" + n + "/device/";
        std::string vid = readSmall(base + "../idVendor");
        if(vid.empty()) vid = readSmall(base + "../../idVendor");
        std::string prod = readSmall(base + "../product");
        if(prod.empty()) prod = readSmall(base + "../../product");
        p.ch340 = vid == "1a86";
        p.description = p.name + (prod.empty() ? "" : " (" + prod + ")") + (p.ch340 ? " CH340" : "");
        out.push_back(p);
    }
    closedir(d);
    return out;
}

#endif

bool Serial::readExact(uint8_t* out, size_t n, int timeoutMs) {
    for(size_t i = 0; i < n; ++i) {
        int c = readByte(timeoutMs);
        if(c < 0) return false;
        out[i] = (uint8_t)c;
    }
    return true;
}

bool Serial::readLine(std::string& out, int timeoutMs) {
    out.clear();
    auto until = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeoutMs);
    for(;;) {
        int left = (int)std::chrono::duration_cast<std::chrono::milliseconds>(until - std::chrono::steady_clock::now()).count();
        if(left < 0) left = 0;
        int c = readByte(left);
        if(c < 0) return false;
        if(c == '\n') break;
        if(c != '\r') out += (char)c;
        if(out.size() > 4096) return false;
    }
    return true;
}

// ---- the protocol ----

bool Device::tryBaud(const std::string& portName, int b) {
    std::string err;
    if(!serial_.open(portName, b, &err)) return false;
    serial_.write("@HAND00");
    std::string line;
    if(serial_.readLine(line, kTimeout) && isAck(line)) return true;
    serial_.close();
    return false;
}

bool Device::connect(const std::string& portWanted, std::string* err) {
    std::lock_guard<std::mutex> g(lock);
    serial_.close();
    ok_ = false;
    std::vector<std::string> candidates;
    if(!portWanted.empty()) candidates.push_back(portWanted);
    else for(auto& p : listPorts()) if(p.ch340) candidates.push_back(p.name);
    if(candidates.empty()) {
        if(err) *err = "no VM2 found (no CH340 port). Is it plugged in, and its USB driver installed?";
        return false;
    }
    for(auto& c : candidates) {
        // the port may only be openable at all (e.g. busy): report that if nothing answers
        std::string openErr;
        Serial probe;
        if(!probe.open(c, 460800, &openErr)) {
            if(err) *err = openErr;
            continue;
        }
        probe.close();
        for(int b : {460800, 921600}) {
            if(!tryBaud(c, b)) continue;
            port = c;
            baud = b;
            ok_ = true;
            serial_.write("@INFO00");
            std::string line;
            if(serial_.readLine(line, kTimeout)) {
                std::vector<std::string> w;
                size_t s = 0;
                while(s <= line.size()) {
                    size_t e = line.find(' ', s);
                    if(e == std::string::npos) e = line.size();
                    w.push_back(line.substr(s, e - s));
                    s = e + 1;
                }
                if(w.size() > 1) hw = w[1];
                if(w.size() > 2) fw = w[2];
                if(w.size() > 3) id = w[3];
            }
            return true;
        }
        if(err) *err = c + " didn't answer. Put the VM2 in GUI-mode (Settings > System > GUI-mode) and close the VM2 GUI.";
    }
    return false;
}

void Device::disconnect() {
    std::lock_guard<std::mutex> g(lock);
    serial_.close();
    ok_ = false;
}

bool Device::handshake() {
    std::lock_guard<std::mutex> g(lock);
    if(!serial_.isOpen()) return false;
    serial_.flushInput();
    serial_.write("@HAND00");
    std::string line;
    ok_ = serial_.readLine(line, kTimeout) && isAck(line);
    return ok_;
}

bool Device::expectAck(int timeoutMs, std::string* err, const char* what) {
    std::string line;
    if(!serial_.readLine(line, timeoutMs)) { if(err) *err = std::string("no answer from the VM2 (") + what + ")"; return false; }
    if(contains(line, "NOSD")) { if(err) *err = "the VM2 has no SD card in"; return false; }
    if(isNack(line) || !isAck(line)) { if(err) *err = std::string("the VM2 refused: ") + what + " (" + line + ")"; return false; }
    return true;
}

bool Device::command(const char* cmd, int timeoutMs, std::string* err) {
    if(!connected()) { if(err) *err = "the VM2 isn't connected"; return false; }
    serial_.flushInput();
    serial_.write("@HAND00");
    if(!expectAck(kTimeout, err, "handshake")) return false;
    serial_.write(cmd);
    return expectAck(timeoutMs, err, cmd);
}

bool Device::syncClock(std::string* err) {
    std::lock_guard<std::mutex> g(lock);
    if(!connected()) { if(err) *err = "the VM2 isn't connected"; return false; }
    time_t t = time(nullptr);
    struct tm tmv;
#ifdef _WIN32
    localtime_s(&tmv, &t);
#else
    localtime_r(&t, &tmv);
#endif
    uint8_t b[10] = {(uint8_t)tmv.tm_sec, (uint8_t)tmv.tm_min, (uint8_t)tmv.tm_hour, (uint8_t)tmv.tm_wday,
                     (uint8_t)tmv.tm_mday, (uint8_t)(tmv.tm_mon + 1), (uint8_t)(tmv.tm_year + 1900 - 1980), 0, 0, 0};
    serial_.flushInput();
    serial_.write("@WRTC00");
    serial_.write(b, 10);
    return expectAck(kTimeout, err, "set the clock");
}

bool Device::readInternal(Bytes& out, Progress p, std::string* err) {
    std::lock_guard<std::mutex> g(lock);
    if(!connected()) { if(err) *err = "the VM2 isn't connected"; return false; }
    serial_.flushInput();
    serial_.write("@HAND00");
    if(!expectAck(kTimeout, err, "handshake")) return false;
    out.assign(131072, 0);
    for(int blk = 0; blk < 256; ++blk) {
        uint8_t n = (uint8_t)blk;
        serial_.write("@RMEM1");
        serial_.write(&n, 1);
        if(!serial_.readExact(&out[blk * 512], 512, kTimeout * 4)) {
            if(err) *err = "the VM2 stopped sending at block " + std::to_string(blk);
            return false;
        }
        if(p) p(blk + 1, 256);
    }
    return true;
}

bool Device::writeInternal(const Bytes& image, Progress p, std::string* err) {
    std::lock_guard<std::mutex> g(lock);
    if(image.size() != 131072) { if(err) *err = "a VMU image is 131072 bytes"; return false; }
    if(!connected()) { if(err) *err = "the VM2 isn't connected"; return false; }
    serial_.flushInput();
    serial_.write("@HAND00");
    if(!expectAck(kTimeout, err, "handshake")) return false;
    for(int blk = 0; blk < 256; ++blk) {
        uint8_t x = 0;
        for(int i = 0; i < 512; ++i) x ^= image[blk * 512 + i];
        bool done = false;
        for(int tries = 0; tries < 5 && !done; ++tries) {
            uint8_t n = (uint8_t)blk;
            serial_.write("@WMEM1");
            serial_.write(&n, 1);
            serial_.write(&image[blk * 512], 512);
            serial_.write(&x, 1);
            std::string line;
            done = serial_.readLine(line, kTimeout * 4) && isAck(line);
        }
        if(!done) { if(err) *err = "the VM2 didn't take block " + std::to_string(blk); return false; }
        if(p) p(blk + 1, 256);
    }
    return true;
}

static std::vector<std::string> split(const std::string& s, char c) {
    std::vector<std::string> out;
    size_t a = 0;
    for(;;) {
        size_t b = s.find(c, a);
        out.push_back(s.substr(a, b == std::string::npos ? std::string::npos : b - a));
        if(b == std::string::npos) break;
        a = b + 1;
    }
    return out;
}

static std::string trim(std::string s) {
    while(!s.empty() && (s.back() == ' ' || s.back() == '\r')) s.pop_back();
    size_t a = 0;
    while(a < s.size() && s[a] == ' ') ++a;
    return s.substr(a);
}

bool Device::list(const std::string& folder, std::vector<Entry>& out, std::string* err) {
    std::lock_guard<std::mutex> g(lock);
    out.clear();
    if(!command("@SDEXPL", kSdTimeout, err)) return false;
    // the GUI sends "\" for the root and a folder's bare name otherwise
    std::string arg = folder;
    if(arg != "\\" && !arg.empty() && arg[0] == '\\') arg = arg.substr(1);
    if(arg.empty()) arg = "\\";
    serial_.write(arg + "\r");
    std::string line;
    if(!serial_.readLine(line, kSdTimeout) || !contains(line, ">START_DIR")) {
        if(err) *err = "the VM2 didn't list " + folder;
        return false;
    }
    for(;;) {
        if(!serial_.readLine(line, kSdTimeout)) { if(err) *err = "the listing stopped"; return false; }
        if(contains(line, ">END_DIR")) break;
        auto f = split(line, '|');
        if(f.size() < 4) continue;
        Entry e;
        e.name = trim(f[0]);
        e.size = (uint32_t)strtoul(trim(f[1]).c_str(), nullptr, 10);
        e.date = trim(f[2]);
        e.dir = trim(f[3]) == "1";
        if(f.size() > 4) e.title = trim(f[4]);
        if(f.size() > 5) e.dreamEye = trim(f[5]) == "Yes";
        if(e.name == "." || e.name == "..") continue;
        out.push_back(e);
    }
    return true;
}

bool Device::readFile(const std::string& path, uint32_t size, Bytes& out, Progress p, std::string* err) {
    std::lock_guard<std::mutex> g(lock);
    if(!command("@SDFLRD", kSdTimeout, err)) return false;
    serial_.write(path + "\r");
    Bytes all(131072, 0);
    for(int blk = 0; blk < 256; ++blk) {
        if(!serial_.readExact(&all[blk * 512], 512, kSdTimeout)) {
            if(err) *err = "the VM2 stopped sending " + path + " at block " + std::to_string(blk);
            return false;
        }
        serial_.write("O");
        if(p) p(blk + 1, 256);
    }
    if(!expectAck(kSdTimeout, err, "read the file")) return false;
    if(size == 0 || size > all.size()) size = (uint32_t)all.size();
    out.assign(all.begin(), all.begin() + size);
    return true;
}

bool Device::writeFile(const std::string& path, const Bytes& image, Progress p, std::string* err) {
    std::lock_guard<std::mutex> g(lock);
    if(image.size() != 131072) { if(err) *err = "the VM2 only takes whole 128 KB VMU images"; return false; }
    if(!command("@SDFLWR", kSdTimeout, err)) return false;
    // as the GUI: in a folder "FOLDER\NAME.BIN" (no leading backslash), at the root "\NAME.BIN"
    std::string arg = path;
    if(arg.size() > 1 && arg[0] == '\\' && arg.find('\\', 1) != std::string::npos) arg = arg.substr(1);
    serial_.write(arg + "\r");
    std::string line;
    if(!serial_.readLine(line, kSdTimeout) || !contains(line, ">START")) {
        if(err) *err = "the VM2 didn't start writing " + path + (line.empty() ? "" : " (" + line + ")");
        return false;
    }
    for(int blk = 0; blk < 256; ++blk) {
        serial_.write(&image[blk * 512], 512);
        if(!serial_.readLine(line, kSdTimeout) || !contains(line, ">NXTBLK")) {
            if(err) *err = "the VM2 stopped taking " + path + " at block " + std::to_string(blk);
            return false;
        }
        if(p) p(blk + 1, 256);
    }
    return expectAck(kSdTimeout, err, "write the file");
}

#define SIMPLE2(fn, cmd, what)                                                                              \
    bool Device::fn(const std::string& a, const std::string& b, std::string* err) {                         \
        std::lock_guard<std::mutex> g(lock);                                                                \
        if(!command(cmd, kSdTimeout, err)) return false;                                                    \
        serial_.write(a + "\r");                                                                            \
        serial_.write(b + "\r");                                                                            \
        return expectAck(kSdTimeout, err, what);                                                            \
    }
#define SIMPLE1(fn, cmd, what)                                                                              \
    bool Device::fn(const std::string& a, std::string* err) {                                               \
        std::lock_guard<std::mutex> g(lock);                                                                \
        if(!command(cmd, kSdTimeout, err)) return false;                                                    \
        serial_.write(a + "\r");                                                                            \
        return expectAck(kSdTimeout, err, what);                                                            \
    }
SIMPLE2(copyFile, "@SDFCPY", "copy")
SIMPLE2(rename, "@SDRENM", "rename")
SIMPLE1(removeFile, "@SDFDEL", "delete the file")
SIMPLE1(makeFolder, "@SDDCRT", "make the folder")
SIMPLE1(removeFolder, "@SDDDEL", "delete the folder")

bool Device::writeTitle(const std::string& folder, const std::string& title, std::string* err) {
    std::lock_guard<std::mutex> g(lock);
    if(!command("@SDFTTL", kSdTimeout, err)) return false;
    std::string f = folder;
    if(!f.empty() && f[0] == '\\') f = f.substr(1);
    serial_.write(f + "\\title.txt\r");
    std::string line;
    if(!serial_.readLine(line, kSdTimeout) || !contains(line, ">START")) { if(err) *err = "the VM2 didn't take the title"; return false; }
    uint8_t b[128] = {};
    memcpy(b, title.data(), std::min<size_t>(title.size(), 127));
    serial_.write(b, 128);
    return expectAck(kSdTimeout, err, "write the title");
}

}  // namespace vm2
