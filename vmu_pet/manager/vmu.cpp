// Moemon VMU manager: VMU images and the formats around them (vmu.h).
#include "vmu.h"

#include <cstring>
#include <ctime>

namespace vmu {

static int fromBcd(uint8_t v) { return (v >> 4) * 10 + (v & 15); }
static uint8_t toBcd(int v) { return (uint8_t)(((v / 10) << 4) | (v % 10)); }
static uint16_t rd16(const uint8_t* p) { return (uint16_t)(p[0] | (p[1] << 8)); }
static uint32_t rd32(const uint8_t* p) { return (uint32_t)(p[0] | (p[1] << 8) | (p[2] << 16) | ((uint32_t)p[3] << 24)); }
static void wr16(uint8_t* p, uint16_t v) { p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); }
static void wr32(uint8_t* p, uint32_t v) { for(int i = 0; i < 4; ++i) p[i] = (uint8_t)(v >> (8 * i)); }

static void putText(uint8_t* p, size_t n, const std::string& s, char pad) {
    for(size_t i = 0; i < n; ++i) p[i] = i < s.size() ? (uint8_t)s[i] : (uint8_t)pad;
}

std::string trimText(const uint8_t* p, size_t n) {
    std::string s;
    for(size_t i = 0; i < n && p[i]; ++i) s += (p[i] >= 32 && p[i] < 127) ? (char)p[i] : '?';
    while(!s.empty() && s.back() == ' ') s.pop_back();
    return s;
}

Stamp Stamp::now() {
    time_t t = time(nullptr);
    struct tm tmv;
#ifdef _WIN32
    localtime_s(&tmv, &t);
#else
    localtime_r(&t, &tmv);
#endif
    Stamp s;
    s.year = tmv.tm_year + 1900; s.month = tmv.tm_mon + 1; s.day = tmv.tm_mday;
    s.hour = tmv.tm_hour; s.minute = tmv.tm_min; s.second = tmv.tm_sec;
    s.weekday = (tmv.tm_wday + 6) % 7;  // the VMU counts from Monday
    return s;
}

void Stamp::toBcd(uint8_t out[8]) const {
    out[0] = vmu::toBcd(year / 100); out[1] = vmu::toBcd(year % 100); out[2] = vmu::toBcd(month);
    out[3] = vmu::toBcd(day); out[4] = vmu::toBcd(hour); out[5] = vmu::toBcd(minute); out[6] = vmu::toBcd(second);
    out[7] = (uint8_t)weekday;
}

Stamp Stamp::fromBcd(const uint8_t in[8]) {
    Stamp s;
    s.year = vmu::fromBcd(in[0]) * 100 + vmu::fromBcd(in[1]);
    s.month = vmu::fromBcd(in[2]); s.day = vmu::fromBcd(in[3]); s.hour = vmu::fromBcd(in[4]);
    s.minute = vmu::fromBcd(in[5]); s.second = vmu::fromBcd(in[6]); s.weekday = in[7];
    return s;
}

std::string Stamp::text() const {
    char b[32];
    snprintf(b, sizeof b, "%04d-%02d-%02d %02d:%02d", year, month, day, hour, minute);
    return b;
}

uint32_t argb4444ToRgba(uint16_t c) {
    uint32_t a = ((c >> 12) & 15) * 17, r = ((c >> 8) & 15) * 17, g = ((c >> 4) & 15) * 17, b = (c & 15) * 17;
    return r | (g << 8) | (b << 16) | (a << 24);
}

uint16_t rgbaToArgb4444(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    auto q = [](uint8_t v) { return (uint16_t)((v + 8) / 17); };
    return (uint16_t)((q(a) << 12) | (q(r) << 8) | (q(g) << 4) | q(b));
}

uint16_t crc16(const uint8_t* data, size_t n, uint16_t crc) {
    for(size_t i = 0; i < n; ++i) {
        crc ^= (uint16_t)(data[i] << 8);
        for(int k = 0; k < 8; ++k) crc = (crc & 0x8000) ? (uint16_t)((crc << 1) ^ 0x1021) : (uint16_t)(crc << 1);
    }
    return crc;
}

// ---- the image ----

Image::Image() : raw(kImageSize, 0) { format(); }

bool Image::load(const Bytes& data, std::string* err) {
    if(data.size() != (size_t)kImageSize) {
        if(err) *err = "a VMU image is 131072 bytes; this is " + std::to_string(data.size());
        return false;
    }
    raw = data;
    return true;
}

bool Image::formatted() const {
    for(int i = 0; i < 16; ++i) if(raw[kRoot * kBlock + i] != 0x55) return false;
    return true;
}

void Image::format() {
    std::fill(raw.begin(), raw.end(), 0);
    uint8_t* r = &raw[kRoot * kBlock];
    memset(r, 0x55, 16);
    Stamp::now().toBcd(r + 0x30);
    const uint16_t f[] = {255, 0, kRoot, kFat, 1, kDir, kDirBlocks};
    for(int i = 0; i < 7; ++i) wr16(r + 0x40 + i * 2, f[i]);
    wr16(r + 0x50, kUserBlocks); wr16(r + 0x52, 41); wr16(r + 0x54, 0); wr16(r + 0x56, 128);
    for(int b = 0; b < kBlocks; ++b) setFat(b, kFree);
    setFat(kRoot, kLast);
    setFat(kFat, kLast);
    for(int b = kDir; b > kDir - kDirBlocks; --b) setFat(b, b > kDir - kDirBlocks + 1 ? (uint16_t)(b - 1) : kLast);
}

uint16_t Image::fat(int b) const { return rd16(&raw[kFat * kBlock + b * 2]); }
void Image::setFat(int b, uint16_t v) { wr16(&raw[kFat * kBlock + b * 2], v); }

std::vector<File> Image::files() const {
    std::vector<File> out;
    for(int d = kDir; d > kDir - kDirBlocks; --d)
        for(int i = 0; i < kBlock / 32; ++i) {
            int off = d * kBlock + i * 32;
            const uint8_t* e = &raw[off];
            if(e[0] != kTypeData && e[0] != kTypeGame) continue;
            File f;
            f.type = e[0];
            f.copyProtected = e[1] == 0xFF;
            f.first = rd16(e + 2);
            f.name = trimText(e + 4, 12);
            f.stamp = Stamp::fromBcd(e + 0x10);
            f.blocks = rd16(e + 0x18);
            f.headerOffset = rd16(e + 0x1A);
            f.entry = off;
            out.push_back(f);
        }
    return out;
}

const File* Image::find(const std::string& name) const {
    static thread_local std::vector<File> cache;
    cache = files();
    for(auto& f : cache) if(f.name == name) return &f;
    return nullptr;
}

int Image::freeBlocks() const {
    int n = 0;
    for(int b = 0; b < kUserBlocks; ++b) if(fat(b) == kFree) ++n;
    return n;
}

std::vector<int> Image::chain(uint16_t first) const {
    std::vector<int> out;
    uint16_t b = first;
    while(b != kLast && b < kBlocks && out.size() <= (size_t)kBlocks) {
        out.push_back(b);
        b = fat(b);
    }
    return out;
}

Bytes Image::read(const File& f) const {
    Bytes out;
    for(int b : chain(f.first)) out.insert(out.end(), raw.begin() + b * kBlock, raw.begin() + (b + 1) * kBlock);
    return out;
}

bool Image::remove(const File& f, std::string* err) {
    if(f.entry < 0) { if(err) *err = "no such file"; return false; }
    for(int b : chain(f.first)) setFat(b, kFree);
    memset(&raw[f.entry], 0, 32);
    return true;
}

bool Image::add(const std::string& name, uint8_t type, const Bytes& data, bool copyProtected, const Stamp& stamp,
                std::string* err) {
    for(auto& f : files()) if(f.name == name) { if(err) *err = name + " is already on this VMU"; return false; }
    int n = (int)((data.size() + kBlock - 1) / kBlock);
    if(n == 0) n = 1;
    std::vector<int> blocks;
    if(type == kTypeGame) {
        for(auto& f : files()) if(f.isGame()) { if(err) *err = "a VMU holds one GAME (" + f.name + " is here)"; return false; }
        for(int b = 0; b < n; ++b) {
            if(b >= kUserBlocks || fat(b) != kFree) {
                if(err) *err = "a GAME needs blocks 0-" + std::to_string(n - 1) + " free (it runs from block 0)";
                return false;
            }
            blocks.push_back(b);
        }
    } else {
        for(int b = kUserBlocks - 1; b >= 0 && (int)blocks.size() < n; --b) if(fat(b) == kFree) blocks.push_back(b);
        if((int)blocks.size() < n) {
            if(err) *err = "not enough room: " + std::to_string(n) + " blocks needed, " + std::to_string(freeBlocks()) + " free";
            return false;
        }
    }
    int entry = -1;
    for(int d = kDir; d > kDir - kDirBlocks && entry < 0; --d)
        for(int i = 0; i < kBlock / 32; ++i) {
            int off = d * kBlock + i * 32;
            if(raw[off] == 0) { entry = off; break; }
        }
    if(entry < 0) { if(err) *err = "the directory is full"; return false; }
    for(int i = 0; i < n; ++i) {
        uint8_t* dst = &raw[blocks[i] * kBlock];
        memset(dst, 0, kBlock);
        size_t from = (size_t)i * kBlock;
        if(from < data.size()) memcpy(dst, data.data() + from, std::min((size_t)kBlock, data.size() - from));
        setFat(blocks[i], i + 1 < n ? (uint16_t)blocks[i + 1] : kLast);
    }
    uint8_t* e = &raw[entry];
    memset(e, 0, 32);
    e[0] = type;
    e[1] = copyProtected ? 0xFF : 0x00;
    wr16(e + 2, (uint16_t)blocks[0]);
    putText(e + 4, 12, name, ' ');
    stamp.toBcd(e + 0x10);
    wr16(e + 0x18, (uint16_t)n);
    wr16(e + 0x1A, type == kTypeGame ? 1 : 0);
    return true;
}

bool Image::replace(const File& f, const Bytes& data, std::string* err) {
    Bytes old = read(f);
    File keep = f;
    if(!remove(f, err)) return false;
    if(add(keep.name, keep.type, data, keep.copyProtected, keep.stamp, err)) return true;
    std::string ignore;
    add(keep.name, keep.type, old, keep.copyProtected, keep.stamp, &ignore);  // put it back as it was
    return false;
}

bool Image::rename(const File& f, const std::string& name, std::string* err) {
    if(name.empty() || name.size() > 12) { if(err) *err = "a VMU file name is 1-12 characters"; return false; }
    for(auto& o : files()) if(o.name == name && o.entry != f.entry) { if(err) *err = name + " is already here"; return false; }
    putText(&raw[f.entry + 4], 12, name, ' ');
    return true;
}

bool Image::setCopyProtected(const File& f, bool on) {
    raw[f.entry + 1] = on ? 0xFF : 0x00;
    return true;
}

bool Image::customColor() const { return raw[kRoot * kBlock + 0x10] == 1; }

void Image::colorRgba(uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a) const {
    const uint8_t* p = &raw[kRoot * kBlock + 0x10];
    b = p[1]; g = p[2]; r = p[3]; a = p[4];
}

void Image::setColor(bool custom, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    uint8_t* p = &raw[kRoot * kBlock + 0x10];
    p[0] = custom ? 1 : 0; p[1] = b; p[2] = g; p[3] = r; p[4] = a;
}

// ---- VMS headers ----

int VmsHeader::eyecatchBytes() const {
    switch(eyecatchType) {
        case 1: return 8064;
        case 2: return 512 + 4032;
        case 3: return 32 + 2016;
        default: return 0;
    }
}

bool parseVms(const Bytes& file, int headerOffset, VmsHeader& h) {
    size_t at = (size_t)headerOffset * kBlock;
    if(file.size() < at + 128) return false;
    const uint8_t* p = &file[at];
    h.vmuDesc = trimText(p, 16);
    h.dcDesc = trimText(p + 16, 32);
    h.creator = trimText(p + 48, 16);
    h.iconCount = rd16(p + 0x40);
    h.animSpeed = rd16(p + 0x42);
    h.eyecatchType = rd16(p + 0x44);
    h.crc = rd16(p + 0x46);
    h.dataBytes = rd32(p + 0x48);
    for(int i = 0; i < 16; ++i) h.palette[i] = rd16(p + 0x60 + i * 2);
    if(h.iconCount > 3) return false;  // not a VMS header (the format allows 1-3)
    h.icons.assign(h.iconCount, Icon{});
    for(int i = 0; i < h.iconCount; ++i) {
        size_t o = at + 128 + (size_t)i * 512;
        if(file.size() < o + 512) return false;
        memcpy(h.icons[i].pixels.data(), &file[o], 512);
    }
    return true;
}

uint16_t vmsCrc(const Bytes& file, int headerOffset, const VmsHeader& h) {
    size_t at = (size_t)headerOffset * kBlock;
    size_t total = (size_t)h.totalHeaderBytes() + h.dataBytes;
    if(at + total > file.size()) total = file.size() - at;
    Bytes copy(file.begin() + at, file.begin() + at + total);
    if(copy.size() >= 0x48) { copy[0x46] = 0; copy[0x47] = 0; }
    return crc16(copy.data(), copy.size());
}

static void writeHeaderBytes(uint8_t* p, const VmsHeader& h) {
    putText(p, 16, h.vmuDesc, ' ');
    putText(p + 16, 32, h.dcDesc, ' ');
    putText(p + 48, 16, h.creator, '\0');
    wr16(p + 0x40, (uint16_t)h.icons.size());
    wr16(p + 0x42, h.animSpeed);
    wr16(p + 0x44, h.eyecatchType);
    wr16(p + 0x46, h.crc);
    wr32(p + 0x48, h.dataBytes);
    for(int i = 0; i < 16; ++i) wr16(p + 0x60 + i * 2, h.palette[i]);
}

bool writeVmsInPlace(Bytes& file, int headerOffset, const VmsHeader& h, bool isData) {
    size_t at = (size_t)headerOffset * kBlock;
    if(file.size() < at + (size_t)h.totalHeaderBytes()) return false;
    VmsHeader w = h;
    w.crc = 0;
    writeHeaderBytes(&file[at], w);
    for(size_t i = 0; i < h.icons.size(); ++i) memcpy(&file[at + 128 + i * 512], h.icons[i].pixels.data(), 512);
    if(isData) {
        w.crc = vmsCrc(file, headerOffset, w);
        wr16(&file[at + 0x46], w.crc);
    }
    return true;
}

Bytes rebuildData(const Bytes& file, const VmsHeader& oldH, const VmsHeader& newH) {
    // header + icons + the old eyecatch + the data, laid out again with the new icons
    size_t oldHead = (size_t)oldH.totalHeaderBytes();
    size_t eyeAt = 128 + 512 * oldH.icons.size();
    Bytes eye(file.begin() + std::min(eyeAt, file.size()), file.begin() + std::min(oldHead, file.size()));
    size_t dataEnd = std::min(file.size(), oldHead + (size_t)oldH.dataBytes);
    Bytes data(file.begin() + std::min(oldHead, file.size()), file.begin() + dataEnd);
    Bytes out(128 + 512 * newH.icons.size(), 0);
    out.insert(out.end(), eye.begin(), eye.end());
    out.insert(out.end(), data.begin(), data.end());
    out.resize((out.size() + kBlock - 1) / kBlock * kBlock, 0);
    VmsHeader w = newH;
    w.dataBytes = (uint32_t)data.size();
    writeVmsInPlace(out, 0, w, true);
    return out;
}

// ---- ICONDATA_VMS ----

bool parseIconData(const Bytes& file, IconData& d) {
    if(file.size() < 24) return false;
    d.desc = trimText(&file[0], 16);
    uint32_t monoAt = rd32(&file[16]), colorAt = rd32(&file[20]);
    if(monoAt + 128 > file.size()) return false;
    memcpy(d.mono.bits.data(), &file[monoAt], 128);
    d.hasColor = colorAt != 0 && colorAt + 32 + 512 <= file.size();
    if(d.hasColor) {
        for(int i = 0; i < 16; ++i) d.palette[i] = rd16(&file[colorAt + i * 2]);
        memcpy(d.color.pixels.data(), &file[colorAt + 32], 512);
    }
    return true;
}

Bytes buildIconData(const IconData& d) {
    Bytes out(kBlock * 2, 0);  // 0x20 header, 0x20 mono (128), 0xA0 colour (32 + 512) = 0x2C0
    putText(&out[0], 16, d.desc, ' ');
    wr32(&out[16], 0x20);
    wr32(&out[20], d.hasColor ? 0xA0 : 0);
    memcpy(&out[0x20], d.mono.bits.data(), 128);
    if(d.hasColor) {
        for(int i = 0; i < 16; ++i) wr16(&out[0xA0 + i * 2], d.palette[i]);
        memcpy(&out[0xC0], d.color.pixels.data(), 512);
    }
    return out;
}

// ---- .VMI and .DCI ----

bool parseVmi(const Bytes& v, Vmi& out) {
    if(v.size() < 108) return false;
    out.desc = trimText(&v[4], 32);
    out.copyright = trimText(&v[36], 32);
    out.stamp.year = rd16(&v[0x44]); out.stamp.month = v[0x46]; out.stamp.day = v[0x47];
    out.stamp.hour = v[0x48]; out.stamp.minute = v[0x49]; out.stamp.second = v[0x4A];
    out.stamp.weekday = (v[0x4B] + 6) % 7;
    out.resource = trimText(&v[0x50], 8);
    out.vmuName = trimText(&v[0x58], 12);
    uint16_t mode = rd16(&v[0x64]);
    out.game = (mode & 2) != 0;
    out.copyProtected = (mode & 1) != 0;
    out.size = rd32(&v[0x68]);
    return true;
}

Bytes buildVmi(const File& f, const std::string& resource, const std::string& desc, uint32_t size) {
    Bytes v(108, 0);
    uint8_t res[8] = {};
    for(size_t i = 0; i < 8 && i < resource.size(); ++i) res[i] = (uint8_t)resource[i];
    const char* sega = "SEGA";
    for(int i = 0; i < 4; ++i) v[i] = res[i] & (uint8_t)sega[i];
    putText(&v[4], 32, desc, ' ');
    putText(&v[36], 32, "Moemon VMU Manager", ' ');
    wr16(&v[0x44], (uint16_t)f.stamp.year);
    v[0x46] = (uint8_t)f.stamp.month; v[0x47] = (uint8_t)f.stamp.day; v[0x48] = (uint8_t)f.stamp.hour;
    v[0x49] = (uint8_t)f.stamp.minute; v[0x4A] = (uint8_t)f.stamp.second; v[0x4B] = (uint8_t)((f.stamp.weekday + 1) % 7);
    wr16(&v[0x4C], 0);
    wr16(&v[0x4E], 1);
    memcpy(&v[0x50], res, 8);
    putText(&v[0x58], 12, f.name, ' ');
    wr16(&v[0x64], (uint16_t)((f.isGame() ? 2 : 0) | (f.copyProtected ? 1 : 0)));
    wr32(&v[0x68], size);
    return v;
}

bool parseDci(const Bytes& data, File& meta, Bytes& body, std::string* err) {
    if(data.size() < 32 + kBlock || (data.size() - 32) % 4) { if(err) *err = "not a .DCI file"; return false; }
    const uint8_t* e = data.data();
    if(e[0] != kTypeData && e[0] != kTypeGame) { if(err) *err = "not a .DCI file (unknown file type)"; return false; }
    meta.type = e[0];
    meta.copyProtected = e[1] == 0xFF;
    meta.name = trimText(e + 4, 12);
    meta.stamp = Stamp::fromBcd(e + 0x10);
    meta.blocks = rd16(e + 0x18);
    meta.headerOffset = rd16(e + 0x1A);
    body.assign(data.begin() + 32, data.end());
    for(size_t i = 0; i + 3 < body.size(); i += 4) {
        std::swap(body[i], body[i + 3]);
        std::swap(body[i + 1], body[i + 2]);
    }
    return true;
}

Bytes buildDci(const File& f, const Bytes& body) {
    Bytes out(32, 0);
    out[0] = f.type;
    out[1] = f.copyProtected ? 0xFF : 0;
    wr16(&out[2], f.first);
    putText(&out[4], 12, f.name, ' ');
    f.stamp.toBcd(&out[0x10]);
    wr16(&out[0x18], f.blocks);
    wr16(&out[0x1A], f.headerOffset);
    Bytes b = body;
    for(size_t i = 0; i + 3 < b.size(); i += 4) {
        std::swap(b[i], b[i + 3]);
        std::swap(b[i + 1], b[i + 2]);
    }
    out.insert(out.end(), b.begin(), b.end());
    return out;
}

}  // namespace vmu
