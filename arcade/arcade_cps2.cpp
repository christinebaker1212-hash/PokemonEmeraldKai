// Moemon Kai: the GAME CENTER's arcade games (include/arcade.h). A CPS-2 board from cps2recomp (arcade/cps2recomp:
// GPL-3.0, its machine, video, QSound and Z80; the 68000 always in its interpreter, since the games' own code is
// never translated or kept here) running the ROM set the player put in the "arcade" folder beside the game. A set is
// read from <set>.zip, with the files it shares with its parent set from <parent>.zip (as ROM collections keep them);
// files are found by checksum, whatever they are called. Nothing of the games is in the repository or the downloads.
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

#include "cps2recomp/machine/machine.h"
#include "cps2recomp/machine/romset.h"
#include "cps2recomp/runtime/state.h"
#include "arcade.h"

extern "C" const char *Updater_ExeFile(const char *name);

using namespace cps2recomp;

namespace {

struct ArcadeGame {
    const char *title, *set, *parent, *files;
    const char *program, *graphics, *soundProgram, *samples, *key;
};

// The sets these boards are made from (MAME's names and CRC32s; the same as the cps2recomp ports')
const ArcadeGame kGames[ARCADE_GAME_COUNT] = {
    {"VAMPIRE SAVIOR", "vsavj", "vsav", "vsav.zip and vsavj.zip",
     "vm3j.03d:2a2e74a4 vm3j.04d:1c2427bc vm3j.05a:95ce88d5 vm3j.06b:2c4297e0 vm3j.07b:a38aaae7 vm3j.08a:5773e5c9 "
     "vm3j.09b:d064f8b9 vm3j.10b:434518e9",
     "vm3.13m:fd8a11eb vm3.15m:dd1e7d4e vm3.17m:6b89445e vm3.19m:3830fdc7 vm3.14m:c1a28e6c vm3.16m:194a7304 "
     "vm3.18m:df9a9f47 vm3.20m:c22fc3d9",
     "vm3.01:f778769b vm3.02:cc09faa1", "vm3.11m:e80e956e vm3.12m:9cd71557", "vsavj.key:36d28ab8"},
    {"CYBERBOTS", "cybots", "cybots", "cybots.zip",
     "cybe.03:234381cd cybe.04:80691061 cyb.05:ec40408e cyb.06:1ad0bed2 cyb.07:6245a39a cyb.08:4b48e223 "
     "cyb.09:e15238f6 cyb.10:75f4003b",
     "cyb.13m:f0dce192 cyb.15m:187aa39c cyb.17m:8a0e4b12 cyb.19m:34b62612 cyb.14m:c1537957 cyb.16m:15349e86 "
     "cyb.18m:d83e977d cyb.20m:77cdad5c",
     "cyb.01:9c0fb079 cyb.02:51cb0c4e", "cyb.11m:362ccab2 cyb.12m:7066e9cc", "cybots.key:9bbcbef3"},
};

// The board keeps a reference to its set: g_roms lives as long as g_machine (and is freed after it)
std::unique_ptr<RomSet> g_roms;
std::unique_ptr<Machine> g_machine;
int g_game = -1;
size_t g_audioAt = 0;
std::vector<uint8_t> g_state;

std::string SetPath(int game)
{
    const char *dir = getenv("MOEMON_ARCADE_DIR");
    std::string folder = dir && dir[0] ? dir : Arcade_Folder();
    return folder + "/" + kGames[game].set + ".zip";
}

bool Load(int game, RomSet &roms, std::string &error)
{
    const ArcadeGame &g = kGames[game];
    const RomList list = ParseRomList(g.program, g.graphics, g.soundProgram, g.samples, g.key, g.set, g.parent);
    return LoadRomSet(SetPath(game), list, roms, error);
}

} // namespace

extern "C" {

const char *Arcade_Folder(void)
{
    return Updater_ExeFile("arcade");
}

const char *Arcade_Title(int game)
{
    return game >= 0 && game < ARCADE_GAME_COUNT ? kGames[game].title : "";
}

const char *Arcade_SetFiles(int game)
{
    return game >= 0 && game < ARCADE_GAME_COUNT ? kGames[game].files : "";
}

bool Arcade_Check(int game, char *why, size_t whySize)
{
    RomSet roms;
    std::string error;
    if (game < 0 || game >= ARCADE_GAME_COUNT)
        return false;
    if (Load(game, roms, error))
        return true;
    if (why && whySize)
        snprintf(why, whySize, "%s", error.c_str());
    return false;
}

bool Arcade_Open(int game)
{
    std::string error;
    Arcade_Close();
    if (game < 0 || game >= ARCADE_GAME_COUNT)
        return false;
    g_roms = std::make_unique<RomSet>();
    if (!Load(game, *g_roms, error)) {
        printf("[arcade] %s: %s\n", kGames[game].title, error.c_str());
        g_roms.reset();
        return false;
    }
    g_machine = std::make_unique<Machine>(*g_roms);
    g_machine->Reset();
    g_game = game;
    g_audioAt = 0;
    printf("[arcade] %s: the board is on\n", kGames[game].title);
    return true;
}

void Arcade_Close(void)
{
    g_machine.reset();
    g_roms.reset();
    g_game = -1;
}

bool Arcade_IsOpen(void)
{
    return g_machine != nullptr;
}

void Arcade_Frame(uint16_t p1, uint16_t p2)
{
    if (!g_machine)
        return;
    g_machine->players[0] = p1;
    g_machine->players[1] = p2;
    g_machine->RunFrame();
}

const uint32_t *Arcade_Picture(void)
{
    return g_machine ? g_machine->Picture() : nullptr;
}

int Arcade_SampleRate(void)
{
    return (int)QSound::kSampleRate;
}

int Arcade_TakeAudio(int16_t *out, int maxPairs)
{
    if (!g_machine)
        return 0;
    std::vector<int16_t> &a = g_machine->audio;
    size_t pairs = (a.size() - g_audioAt) / 2;
    if (pairs > (size_t)maxPairs)
        pairs = maxPairs;
    memcpy(out, &a[g_audioAt], pairs * 2 * sizeof(int16_t));
    g_audioAt += pairs * 2;
    if (g_audioAt >= a.size()) {
        a.clear();
        g_audioAt = 0;
    }
    return (int)pairs;
}

size_t Arcade_SaveState(uint8_t *buf, size_t cap)
{
    if (!g_machine)
        return 0;
    g_state.clear();
    Archive ar(g_state, true);
    g_machine->Serialize(ar);
    if (g_state.size() > cap)
        return 0;
    memcpy(buf, g_state.data(), g_state.size());
    return g_state.size();
}

bool Arcade_LoadState(const uint8_t *buf, size_t size)
{
    if (!g_machine)
        return false;
    g_state.assign(buf, buf + size);
    Archive ar(g_state, false);
    g_machine->Serialize(ar);
    g_machine->audio.clear();
    g_audioAt = 0;
    return ar.Ok();
}

uint32_t Arcade_Hash(void)
{
    uint32_t h = 2166136261u;
    if (!g_machine)
        return 0;
    for (uint8_t b : g_machine->ram)
        h = (h ^ b) * 16777619u;
    return h;
}

} // extern "C"
