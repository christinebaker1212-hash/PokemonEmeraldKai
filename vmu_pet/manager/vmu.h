// Moemon VMU manager: VMU images (128 KB), their files, VMS headers, icons, ICONDATA_VMS, .VMI and .DCI files.
// The layout is the same as vmu_pet/tools/vmufs.py describes (and libevmu's evmu_fat.h / evmu_vms.h name).
#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace vmu {

constexpr int kBlock = 512;
constexpr int kBlocks = 256;
constexpr int kImageSize = kBlock * kBlocks;
constexpr int kRoot = 255, kFat = 254, kDir = 253, kDirBlocks = 13, kUserBlocks = 200;
constexpr uint16_t kFree = 0xFFFC, kLast = 0xFFFA;
constexpr uint8_t kTypeData = 0x33, kTypeGame = 0xCC;
constexpr const char* kIconDataName = "ICONDATA_VMS";

using Bytes = std::vector<uint8_t>;

struct Stamp {  // BCD in the file system; plain numbers here
    int year = 2000, month = 1, day = 1, hour = 0, minute = 0, second = 0, weekday = 0;
    static Stamp now();
    void toBcd(uint8_t out[8]) const;
    static Stamp fromBcd(const uint8_t in[8]);
    std::string text() const;
};

struct File {
    uint8_t type = 0;           // kTypeData / kTypeGame
    bool copyProtected = false;
    uint16_t first = 0;
    std::string name;           // up to 12 characters
    Stamp stamp;
    uint16_t blocks = 0;
    uint16_t headerOffset = 0;  // in blocks: 1 for a GAME
    int entry = -1;             // byte offset of its directory entry in the image
    bool isGame() const { return type == kTypeGame; }
    bool isIconData() const { return name == kIconDataName; }
};

// A colour icon: 32x32, 4 bits a pixel (high nibble = left pixel), palette ARGB4444
struct Icon {
    std::array<uint8_t, 512> pixels{};
    uint8_t at(int x, int y) const { uint8_t b = pixels[y * 16 + x / 2]; return (x & 1) ? (b & 15) : (b >> 4); }
    void set(int x, int y, uint8_t c) {
        uint8_t& b = pixels[y * 16 + x / 2];
        b = (x & 1) ? ((b & 0xF0) | (c & 15)) : ((b & 0x0F) | (uint8_t)(c << 4));
    }
};

// The 1-bit VMU icon of ICONDATA_VMS: 32x32, bit 7 = left pixel, 1 = dark
struct MonoIcon {
    std::array<uint8_t, 128> bits{};
    bool at(int x, int y) const { return (bits[y * 4 + x / 8] >> (7 - (x & 7))) & 1; }
    void set(int x, int y, bool on) {
        uint8_t& b = bits[y * 4 + x / 8];
        uint8_t m = (uint8_t)(0x80 >> (x & 7));
        b = on ? (b | m) : (b & ~m);
    }
};

// The VMS header of a data or GAME file (128 bytes, then icons, then the eyecatch)
struct VmsHeader {
    std::string vmuDesc;   // 16
    std::string dcDesc;    // 32
    std::string creator;   // 16
    uint16_t iconCount = 1, animSpeed = 8, eyecatchType = 0, crc = 0;
    uint32_t dataBytes = 0;
    std::array<uint16_t, 16> palette{};
    std::vector<Icon> icons;
    int eyecatchBytes() const;  // by eyecatchType
    int totalHeaderBytes() const { return 128 + 512 * (int)icons.size() + eyecatchBytes(); }
};

// ICONDATA_VMS: the VMU's own icon in the Dreamcast's menus (a 1-bit one and a colour one)
struct IconData {
    std::string desc;  // 16
    MonoIcon mono;
    std::array<uint16_t, 16> palette{};
    Icon color;
    bool hasColor = true;
};

uint32_t argb4444ToRgba(uint16_t c);  // 0xAABBGGRR for SDL_PIXELFORMAT_ABGR8888
uint16_t rgbaToArgb4444(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
uint16_t crc16(const uint8_t* data, size_t n, uint16_t crc = 0);

class Image {
public:
    Bytes raw;

    Image();                                   // a freshly formatted VMU
    bool load(const Bytes& data, std::string* err);
    void format();
    bool formatted() const;

    uint16_t fat(int b) const;
    void setFat(int b, uint16_t v);
    std::vector<File> files() const;
    const File* find(const std::string& name) const;
    int freeBlocks() const;
    int usedBlocks() const { return kUserBlocks - freeBlocks(); }
    std::vector<int> chain(uint16_t first) const;

    Bytes read(const File& f) const;           // every block of the file
    bool remove(const File& f, std::string* err);
    // A file into the image: a GAME at block 0, data from the top down. Fails without room.
    bool add(const std::string& name, uint8_t type, const Bytes& data, bool copyProtected, const Stamp& stamp,
             std::string* err);
    // Replace a file's contents (it may change size); keeps its name, type, protection, date.
    bool replace(const File& f, const Bytes& data, std::string* err);
    bool rename(const File& f, const std::string& name, std::string* err);
    bool setCopyProtected(const File& f, bool on);

    // The volume colour (the root block): shown for this VMU in the Dreamcast's menus
    bool customColor() const;
    void colorRgba(uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a) const;
    void setColor(bool custom, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
};

// VMS headers in a file's bytes (at headerOffset blocks in)
bool parseVms(const Bytes& file, int headerOffset, VmsHeader& out);
// Write a header back. Same icon count and eyecatch as the file had: in place. For a data file the icons may change in
// number: the data after them moves (rebuildData). Data files get a fresh CRC.
bool writeVmsInPlace(Bytes& file, int headerOffset, const VmsHeader& h, bool isData);
Bytes rebuildData(const Bytes& file, const VmsHeader& oldH, const VmsHeader& newH);
uint16_t vmsCrc(const Bytes& file, int headerOffset, const VmsHeader& h);

bool parseIconData(const Bytes& file, IconData& out);
Bytes buildIconData(const IconData& d);

// .VMI (the description that goes with a .VMS) and .DCI (Nexus: a directory entry, then the file, words swapped)
struct Vmi {
    std::string desc, copyright, resource, vmuName;
    bool game = false, copyProtected = false;
    uint32_t size = 0;
    Stamp stamp;
};
bool parseVmi(const Bytes& data, Vmi& out);
Bytes buildVmi(const File& f, const std::string& resource, const std::string& desc, uint32_t size);
bool parseDci(const Bytes& data, File& meta, Bytes& body, std::string* err);
Bytes buildDci(const File& f, const Bytes& body);

std::string trimText(const uint8_t* p, size_t n);  // a fixed-width text field without its padding

}  // namespace vmu
