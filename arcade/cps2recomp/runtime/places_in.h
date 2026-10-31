// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy
//
// Sources written for one game that other games made from the same program build too (game.cfg's "sources"):
// they name the first game's places, and each game's family.h lists where those are in it ({first, here} pairs,
// in the first's order; tools/places.py makes the list). The games' own places.h give At() and AtA5() from this.
//
// Worked out when the program is compiled (consteval): a place the list does not have stops the build there. An
// empty list is the first game's own: a place is where it is said to be.
#pragma once

#include <cstddef>
#include <cstdint>

namespace cps2recomp {

// Where a place of the first game's program or memory is in the game being built, from that game's list
template <typename List>
consteval uint32_t PlaceIn(const List &places, uint32_t place) {
    if (places.size() == 0) {
        return place;
    }
    size_t low = 0, high = places.size();
    while (low < high) {
        const size_t middle = (low + high) / 2;
        const auto [first, here] = places[middle];
        if (first < place) {
            low = middle + 1;
        } else {
            high = middle;
        }
    }
    if (low == places.size()) {
        throw "this game's family.h does not say where this place is (tools/places.py)";
    }
    const auto [first, here] = places[low];
    if (first != place) {
        throw "this game's family.h does not say where this place is (tools/places.py)";
    }
    return here;
}

// The same for a variable of the game's, as a distance from A5 (FF8000)
template <typename List>
consteval int32_t VariableIn(const List &places, int32_t offset) {
    return static_cast<int32_t>(PlaceIn(places, (0xFF8000u + static_cast<uint32_t>(offset)) & 0xFFFFFF)) - 0xFF8000;
}

} // namespace cps2recomp
