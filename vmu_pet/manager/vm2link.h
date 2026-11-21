// Moemon VMU manager: the VM2 over USB (a CH340 serial port). The protocol: docs/VM2_USB_PROTOCOL.md.
#pragma once
#include <cstdint>
#include <functional>
#include <mutex>
#include <string>
#include <vector>

namespace vm2 {

using Bytes = std::vector<uint8_t>;
using Progress = std::function<void(int done, int total)>;

struct PortInfo {
    std::string name;         // COM5, /dev/ttyUSB0
    std::string description;  // "USB-SERIAL CH340 (COM5)"
    bool ch340 = false;
};
std::vector<PortInfo> listPorts();

class Serial {
public:
    ~Serial() { close(); }
    bool open(const std::string& name, int baud, std::string* err);
    void close();
    bool isOpen() const;
    bool write(const void* data, size_t n);
    bool write(const std::string& s) { return write(s.data(), s.size()); }
    int readByte(int timeoutMs);                         // -1 on timeout
    bool readExact(uint8_t* out, size_t n, int timeoutMs);
    bool readLine(std::string& out, int timeoutMs);      // without the \r\n
    void flushInput();
private:
#ifdef _WIN32
    void* handle_ = nullptr;
#else
    int fd_ = -1;
#endif
    uint8_t buf_[4096];
    size_t have_ = 0, at_ = 0;
};

struct Entry {
    std::string name;
    uint32_t size = 0;
    std::string date;
    bool dir = false;
    std::string title;     // a folder's title.txt (the game's name)
    bool dreamEye = false;
};

class Device {
public:
    // connect: the port given, or (empty) the first CH340; 460800 baud, else 921600. Needs GUI-mode.
    bool connect(const std::string& port, std::string* err);
    void disconnect();
    bool connected() const { return serial_.isOpen() && ok_; }
    std::string port, hw, fw, id;
    int baud = 0;

    bool handshake();
    bool syncClock(std::string* err);
    bool readInternal(Bytes& out, Progress p, std::string* err);
    bool writeInternal(const Bytes& image, Progress p, std::string* err);

    // The SD card. A folder path is "\" (the root) or "\NAME"; a file "\NAME.BIN" or "\FOLDER\NAME.BIN".
    bool list(const std::string& folder, std::vector<Entry>& out, std::string* err);
    bool readFile(const std::string& path, uint32_t size, Bytes& out, Progress p, std::string* err);
    bool writeFile(const std::string& path, const Bytes& image, Progress p, std::string* err);  // 128 KB images
    bool copyFile(const std::string& from, const std::string& to, std::string* err);
    bool rename(const std::string& from, const std::string& to, std::string* err);
    bool removeFile(const std::string& path, std::string* err);
    bool makeFolder(const std::string& path, std::string* err);
    bool removeFolder(const std::string& path, std::string* err);
    bool writeTitle(const std::string& folder, const std::string& title, std::string* err);

    std::mutex lock;  // one operation at a time (the UI runs transfers on a thread)

private:
    bool tryBaud(const std::string& portName, int b);
    bool command(const char* cmd, int timeoutMs, std::string* err);  // sends it, expects ACK
    bool expectAck(int timeoutMs, std::string* err, const char* what);
    Serial serial_;
    bool ok_ = false;
};

}  // namespace vm2
