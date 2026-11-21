// Moemon VMU manager: PNG in (stb_image, from the game's src/mp/third_party) and out (stored deflate: icons are small).
#include "pngio.h"

#include <cstdio>
#include <cstring>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_NO_STDIO
#include "stb_image.h"

namespace png {

bool load(const std::vector<uint8_t>& file, Rgba& out) {
    int w, h, n;
    unsigned char* p = stbi_load_from_memory(file.data(), (int)file.size(), &w, &h, &n, 4);
    if(!p) return false;
    out.w = w;
    out.h = h;
    out.px.assign((uint32_t*)p, (uint32_t*)p + (size_t)w * h);
    stbi_image_free(p);
    return true;
}

static uint32_t crcTable[256];
static void initCrc() {
    for(uint32_t n = 0; n < 256; ++n) {
        uint32_t c = n;
        for(int k = 0; k < 8; ++k) c = (c & 1) ? 0xEDB88320u ^ (c >> 1) : c >> 1;
        crcTable[n] = c;
    }
}
static uint32_t crc(const uint8_t* p, size_t n, uint32_t c = 0xFFFFFFFFu) {
    for(size_t i = 0; i < n; ++i) c = crcTable[(c ^ p[i]) & 0xFF] ^ (c >> 8);
    return c;
}
static void be32(std::vector<uint8_t>& o, uint32_t v) {
    for(int i = 3; i >= 0; --i) o.push_back((uint8_t)(v >> (8 * i)));
}
static void chunk(std::vector<uint8_t>& o, const char* type, const std::vector<uint8_t>& data) {
    be32(o, (uint32_t)data.size());
    size_t start = o.size();
    o.insert(o.end(), type, type + 4);
    o.insert(o.end(), data.begin(), data.end());
    be32(o, crc(&o[start], o.size() - start) ^ 0xFFFFFFFFu);
}

std::vector<uint8_t> save(const Rgba& img) {
    if(!crcTable[1]) initCrc();
    std::vector<uint8_t> raw;  // filter byte 0 + RGBA per row
    for(int y = 0; y < img.h; ++y) {
        raw.push_back(0);
        for(int x = 0; x < img.w; ++x) {
            uint32_t c = img.px[(size_t)y * img.w + x];
            raw.push_back((uint8_t)c); raw.push_back((uint8_t)(c >> 8)); raw.push_back((uint8_t)(c >> 16)); raw.push_back((uint8_t)(c >> 24));
        }
    }
    std::vector<uint8_t> z = {0x78, 0x01};
    size_t at = 0;
    do {
        size_t n = std::min<size_t>(65535, raw.size() - at);
        z.push_back(at + n == raw.size() ? 1 : 0);
        z.push_back((uint8_t)n); z.push_back((uint8_t)(n >> 8));
        z.push_back((uint8_t)~n); z.push_back((uint8_t)(~n >> 8));
        z.insert(z.end(), raw.begin() + at, raw.begin() + at + n);
        at += n;
    } while(at < raw.size());
    uint32_t a = 1, b = 0;
    for(uint8_t v : raw) { a = (a + v) % 65521; b = (b + a) % 65521; }
    be32(z, (b << 16) | a);
    std::vector<uint8_t> out = {0x89, 'P', 'N', 'G', '\r', '\n', 0x1A, '\n'};
    std::vector<uint8_t> ihdr;
    be32(ihdr, (uint32_t)img.w);
    be32(ihdr, (uint32_t)img.h);
    ihdr.insert(ihdr.end(), {8, 6, 0, 0, 0});
    chunk(out, "IHDR", ihdr);
    chunk(out, "IDAT", z);
    chunk(out, "IEND", {});
    return out;
}

}  // namespace png
