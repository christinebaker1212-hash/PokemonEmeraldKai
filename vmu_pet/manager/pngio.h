// Moemon VMU manager: PNG files for icons.
#pragma once
#include <cstdint>
#include <vector>

namespace png {

struct Rgba {
    int w = 0, h = 0;
    std::vector<uint32_t> px;  // 0xAABBGGRR (bytes R, G, B, A in memory)
};

bool load(const std::vector<uint8_t>& file, Rgba& out);
std::vector<uint8_t> save(const Rgba& img);

}  // namespace png
