// Moemon Kai: the GAME CENTER's arcade games (include/arcade.h), each on its own board behind one bridge (1.64.0: the CPS-2
// games, and PUZZLE BOBBLE's Taito B board, arcade/taitob/taitob.h). A CPS-2 board from cps2recomp (arcade/cps2recomp:
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
#include <sys/stat.h>

#include "cps2recomp/machine/machine.h"
#include "cps2recomp/machine/romset.h"
#include "cps2recomp/runtime/state.h"
#include "taitob/taitob.h"
#include "psikyosh/ps5.h"
#include "arcade.h"

extern "C" const char *Updater_ExeFile(const char *name);

using namespace cps2recomp;

namespace {

struct ArcadeGame {
    const char *title, *set, *parent, *files;
    const char *program, *graphics, *soundProgram, *samples, *key;
};

// The sets these boards are made from (MAME's names and CRC32s; the same as the cps2recomp ports')
const ArcadeGame kGames[ARCADE_PBOBBLE] = {
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
    // (the graphics: three banks of four, 2 MB then 1 MB files, appended in order as the board maps them)
    {"SUPER STREET FIGHTER II X", "ssf2xj", "ssf2t", "ssf2t.zip and ssf2xj.zip",
     "sfxj.03d:50b52b37 sfxj.04a:af7767b4 sfxj.05:f4ff18f5 sfxj.06b:413477c2 sfxj.07a:a18b3d83 sfxj.08:2de76f10 "
     "sfx.09:642fae3f",
     "sfx.13m:cf94d275 sfx.15m:5eb703af sfx.17m:ffa60e0f sfx.19m:34e825c5 sfx.14m:b7cc32e7 sfx.16m:8376ad18 "
     "sfx.18m:f5b1b336 sfx.20m:459d5c6b sfx.21m:e32854af sfx.23m:760f2927 sfx.25m:1ee90208 sfx.27m:f814400f",
     "sfx.01:b47b8835 sfx.02:0022633f", "sfx.11m:9bdbd476 sfx.12m:a05e3aab", "ssf2xj.key:160d1424"},
    {"STREET FIGHTER ALPHA 3", "sfa3", "sfa3", "sfa3.zip",
     "sz3e.03c:9762b206 sz3e.04c:5ad3f721 sz3.05c:57fd0a40 sz3.06c:f6305f8b sz3.07c:6eab0f6f sz3.08c:910c4a3b "
     "sz3.09c:b29e5199 sz3.10b:deb2ff52",
     "sz3.13m:0f7a60d9 sz3.15m:8e933741 sz3.17m:d6e98147 sz3.19m:f31a728a sz3.14m:5ff98297 sz3.16m:52b5bdee "
     "sz3.18m:40631ed5 sz3.20m:763409b4",
     "sz3.01:de810084 sz3.02:72445dc4", "sz3.11m:1c89eed1 sz3.12m:f392b13a", "sfa3.key:54fa39c6"},
};

// (1.64.0) The other boards' sets: MAME's names and CRC32s, each file's part in order (the zips: the set's, then its
// parent's)
struct OtherGame {
    const char *title, *files;
    std::vector<std::string> zips;
    std::vector<RomFile> roms;
};
const OtherGame &Other(int game)
{
    static const OtherGame kPbobble = {
        "PUZZLE BOBBLE", "pbobble.zip", {"pbobble"},
        {{"pb-ic18.ic18", 0x5de14f49}, {"pb-ic2.ic2", 0x2abe07d1},   // 68000 program: even / odd bytes
         {"pb-ic27.ic27", 0x26efa4c4},                               // Z80
         {"pb-ic14.ic14", 0x55f90ea4}, {"pb-ic9.ic9", 0x3253aac9},    // TC0180VCU
         {"pb-ic15.ic15", 0x0840cbc4}}};                             // ADPCM
    // TGM2 PLUS: its program and EEPROM from tgm2p.zip, the rest from its parent tgm2.zip
    static const OtherGame kTgm2 = {
        "TETRIS THE GRAND MASTER 2 PLUS", "tgm2p.zip and tgm2.zip", {"tgm2p", "tgm2"},
        {{"2b.u21", 0x38bc626c}, {"1b.u22", 0x7599fb19}, {"tgm2p.default.nv", 0xb2328b40},   // SH-2 program, EEPROM
         {"81ts_3l.u6", 0xd77cff9c}, {"82ts_3h.u14", 0xf012b583}, {"83ts_4l.u7", 0x078cafc3},
         {"84ts_4h.u15", 0x1f91446b}, {"85ts_5l.u8", 0x40fbd259}, {"86ts_5h.u16", 0x186c935f},
         {"87ts_6l.u1", 0xc17dc48a}, {"88ts_6h.u2", 0xe4dba5da}, {"89ts_7l.u19", 0xdab1b2c5},
         {"90ts_7h.u20", 0xaae696b3}, {"91ts_8l.u28", 0xe953ace1}, {"92ts_8h.u29", 0x9da3b976},
         {"93ts_9l.u41", 0x233087fe}, {"94ts_9h.u42", 0x9da831c7}, {"95ts_10l.u58", 0x303a5240},
         {"96ts_10h.u59", 0x2240ebf6},                                                           // the l / h pairs
         {"97ts_snd.u52", 0x9155eca6}}};                                                         // YMF278B samples
    static const OtherGame kNone = {"", "", {}, {}};
    return game == ARCADE_PBOBBLE ? kPbobble : game == ARCADE_TGM2 ? kTgm2 : kNone;
}
bool IsCps2(int game)
{
    return game >= 0 && game < ARCADE_PBOBBLE;
}

// A running board, whichever it is
struct Board {
    virtual ~Board() = default;
    virtual void Frame(uint16_t p1, uint16_t p2) = 0;
    virtual const uint32_t *Picture() = 0;
    virtual int Width() const = 0;
    virtual int Height() const = 0;
    virtual std::vector<int16_t> &Audio() = 0;
    virtual int SampleRate() const = 0;
    virtual void Serialize(Archive &ar) = 0;
    virtual uint32_t Hash() const = 0;
};

struct Cps2Board : Board {
    // (the board keeps a reference to its set: the set lives as long as the machine, and is freed after it)
    std::unique_ptr<RomSet> roms;
    std::unique_ptr<Machine> machine;
    void Frame(uint16_t p1, uint16_t p2) override {
        machine->players[0] = p1;
        machine->players[1] = p2;
        machine->RunFrame();
    }
    const uint32_t *Picture() override { return machine->Picture(); }
    int Width() const override { return ARCADE_WIDTH; }
    int Height() const override { return ARCADE_HEIGHT; }
    std::vector<int16_t> &Audio() override { return machine->audio; }
    int SampleRate() const override { return (int)QSound::kSampleRate; }
    void Serialize(Archive &ar) override { machine->Serialize(ar); }
    uint32_t Hash() const override {
        uint32_t h = 2166136261u;
        for (uint8_t b : machine->ram)
            h = (h ^ b) * 16777619u;
        return h;
    }
    ~Cps2Board() override {
        machine.reset();
        roms.reset();
    }
};

struct TaitoBBoard : Board {
    std::unique_ptr<taitob::Board> board;
    void Frame(uint16_t p1, uint16_t p2) override { board->RunFrame(p1, p2); }
    const uint32_t *Picture() override { return board->Picture(); }
    int Width() const override { return taitob::Board::kWidth; }
    int Height() const override { return taitob::Board::kHeight; }
    std::vector<int16_t> &Audio() override { return board->audio; }
    int SampleRate() const override { return (int)board->SampleRate(); }
    void Serialize(Archive &ar) override { board->Serialize(ar); }
    uint32_t Hash() const override { return board->Hash(); }
};

struct Ps5Board : Board {
    std::unique_ptr<psikyosh::Board> board;
    void Frame(uint16_t p1, uint16_t p2) override { board->RunFrame(p1, p2); }
    const uint32_t *Picture() override { return board->Picture(); }
    int Width() const override { return psikyosh::Board::kWidth; }
    int Height() const override { return psikyosh::Board::kHeight; }
    std::vector<int16_t> &Audio() override { return board->audio; }
    int SampleRate() const override { return (int)board->SampleRate(); }
    void Serialize(Archive &ar) override { board->Serialize(ar); }
    uint32_t Hash() const override { return board->Hash(); }
};

std::unique_ptr<Board> g_board;
int g_game = -1;
size_t g_audioAt = 0;
std::vector<uint8_t> g_state;

std::string Folder()
{
    const char *dir = getenv("MOEMON_ARCADE_DIR");
    return dir && dir[0] ? dir : Arcade_Folder();
}

std::string SetPath(int game)
{
    return Folder() + "/" + kGames[game].set + ".zip";
}

bool Load(int game, RomSet &roms, std::string &error)
{
    const ArcadeGame &g = kGames[game];
    const RomList list = ParseRomList(g.program, g.graphics, g.soundProgram, g.samples, g.key, g.set, g.parent);
    return LoadRomSet(SetPath(game), list, roms, error);
}

// PUZZLE BOBBLE's parts from its files
std::unique_ptr<Board> OpenTaitoB(int game, std::string &error)
{
    const OtherGame &g = Other(game);
    std::vector<std::vector<uint8_t>> f;
    if (!ReadZipFiles(Folder(), g.zips, g.roms, &f, error))
        return nullptr;
    taitob::Roms roms;
    roms.program.assign(0x80000, 0xFF);
    for (size_t i = 0; i < 0x40000 && i < f[0].size() && i < f[1].size(); i++) {
        roms.program[i * 2] = f[0][i];       // (ROM_LOAD16_BYTE: ic18 the even bytes, the words' tops)
        roms.program[i * 2 + 1] = f[1][i];
    }
    roms.sound = f[2];
    roms.graphics = f[3];
    roms.graphics.insert(roms.graphics.end(), f[4].begin(), f[4].end());
    roms.samples = f[5];
    auto b = std::make_unique<TaitoBBoard>();
    b->board = std::make_unique<taitob::Board>(std::move(roms));
    return b;
}

// TGM2 PLUS's parts from its files (the program as the SH-2 sees it: MAME's ROM_LOAD32_WORD_SWAP; the graphics as
// MAME's "gfx" region: ROM_LOAD32_WORD, from 0xc00000)
std::unique_ptr<Board> OpenPs5(int game, std::string &error)
{
    const OtherGame &g = Other(game);
    std::vector<std::vector<uint8_t>> f;
    if (!ReadZipFiles(Folder(), g.zips, g.roms, &f, error))
        return nullptr;
    psikyosh::Roms roms;
    roms.program.assign(0x100000, 0);
    for (size_t k = 0; k < 0x40000 && 2 * k + 1 < f[0].size() && 2 * k + 1 < f[1].size(); k++) {
        roms.program[4 * k + 0] = f[0][2 * k + 1];
        roms.program[4 * k + 1] = f[0][2 * k];
        roms.program[4 * k + 2] = f[1][2 * k + 1];
        roms.program[4 * k + 3] = f[1][2 * k];
    }
    roms.eeprom = f[2];
    roms.graphics.assign(0x3000000, 0);
    for (int pair = 0; pair < 8; pair++) {
        const std::vector<uint8_t> &l = f[3 + pair * 2], &h = f[4 + pair * 2];
        const size_t base = 0xc00000 + static_cast<size_t>(pair) * 0x400000;
        for (size_t k = 0; 2 * k + 1 < l.size() && 2 * k + 1 < h.size(); k++) {
            roms.graphics[base + 4 * k + 0] = l[2 * k];
            roms.graphics[base + 4 * k + 1] = l[2 * k + 1];
            roms.graphics[base + 4 * k + 2] = h[2 * k];
            roms.graphics[base + 4 * k + 3] = h[2 * k + 1];
        }
    }
    roms.samples = f[19];
    auto b = std::make_unique<Ps5Board>();
    b->board = std::make_unique<psikyosh::Board>(std::move(roms));
    return b;
}

} // namespace

extern "C" {

const char *Arcade_Folder(void)
{
    return Updater_ExeFile("arcade");
}

const char *Arcade_Title(int game)
{
    if (IsCps2(game))
        return kGames[game].title;
    return game >= 0 && game < ARCADE_GAME_COUNT ? Other(game).title : "";
}

const char *Arcade_SetFiles(int game)
{
    if (IsCps2(game))
        return kGames[game].files;
    return game >= 0 && game < ARCADE_GAME_COUNT ? Other(game).files : "";
}

// From the zips' lists of files only (CheckRomSet / ReadZipFiles): the cabinet asks it while its message is on screen,
// and unpacking the whole set there held the game up. The set is read in full by Arcade_Open, once the game is chosen.
bool Arcade_Check(int game, char *why, size_t whySize)
{
    std::string error;
    bool ok;
    if (game < 0 || game >= ARCADE_GAME_COUNT)
        return false;
    if (IsCps2(game)) {
        const ArcadeGame &g = kGames[game];
        const RomList list = ParseRomList(g.program, g.graphics, g.soundProgram, g.samples, g.key, g.set, g.parent);
        ok = CheckRomSet(SetPath(game), list, error);
    } else {
        ok = ReadZipFiles(Folder(), Other(game).zips, Other(game).roms, nullptr, error);
    }
    if (!ok && why && whySize)
        snprintf(why, whySize, "%s", error.c_str());
    return ok;
}

// Whether the player has the set, as Arcade_Check, remembered until the zips change (their sizes and times): cheap
// enough for every frame (the SPECTATE panel asks it of the friends' arcade pictures)
bool Arcade_HasSet(int game)
{
    static struct { bool known, ok; long long key; } sKnown[ARCADE_GAME_COUNT];
    struct stat st;
    long long key = 1469598103934665603LL;
    std::vector<std::string> paths;
    if (game < 0 || game >= ARCADE_GAME_COUNT)
        return false;
    if (IsCps2(game)) {
        paths = {SetPath(game), Folder() + "/" + kGames[game].parent + ".zip"};
    } else {
        for (const std::string &z : Other(game).zips)
            paths.push_back(Folder() + "/" + z + ".zip");
    }
    for (const std::string &path : paths) {
        if (stat(path.c_str(), &st) == 0)
            key = (key ^ (long long)st.st_size ^ ((long long)st.st_mtime << 20)) * 1099511628211LL;
        else
            key = (key ^ 7) * 1099511628211LL;
    }
    if (!sKnown[game].known || sKnown[game].key != key) {
        sKnown[game].ok = Arcade_Check(game, nullptr, 0);
        sKnown[game].key = key;
        sKnown[game].known = true;
    }
    return sKnown[game].ok;
}

bool Arcade_Open(int game)
{
    std::string error;
    Arcade_Close();
    if (game < 0 || game >= ARCADE_GAME_COUNT)
        return false;
    if (IsCps2(game)) {
        auto b = std::make_unique<Cps2Board>();
        b->roms = std::make_unique<RomSet>();
        if (!Load(game, *b->roms, error)) {
            printf("[arcade] %s: %s\n", kGames[game].title, error.c_str());
            return false;
        }
        b->machine = std::make_unique<Machine>(*b->roms);
        b->machine->Reset();
        g_board = std::move(b);
    } else {
        g_board = game == ARCADE_TGM2 ? OpenPs5(game, error) : OpenTaitoB(game, error);
        if (!g_board) {
            printf("[arcade] %s: %s\n", Arcade_Title(game), error.c_str());
            return false;
        }
    }
    g_game = game;
    g_audioAt = 0;
    printf("[arcade] %s: the board is on\n", Arcade_Title(game));
    return true;
}

void Arcade_Close(void)
{
    g_board.reset();
    g_game = -1;
}

bool Arcade_IsOpen(void)
{
    return g_board != nullptr;
}

void Arcade_Frame(uint16_t p1, uint16_t p2)
{
    if (g_board)
        g_board->Frame(p1, p2);
}

const uint32_t *Arcade_Picture(void)
{
    return g_board ? g_board->Picture() : nullptr;
}

void Arcade_PictureSize(int *w, int *h)
{
    *w = g_board ? g_board->Width() : ARCADE_WIDTH;
    *h = g_board ? g_board->Height() : ARCADE_HEIGHT;
}

int Arcade_SampleRate(void)
{
    return g_board ? g_board->SampleRate() : (int)QSound::kSampleRate;
}

int Arcade_TakeAudio(int16_t *out, int maxPairs)
{
    if (!g_board)
        return 0;
    std::vector<int16_t> &a = g_board->Audio();
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
    if (!g_board)
        return 0;
    g_state.clear();
    Archive ar(g_state, true);
    g_board->Serialize(ar);
    if (g_state.size() > cap)
        return 0;
    memcpy(buf, g_state.data(), g_state.size());
    return g_state.size();
}

bool Arcade_LoadState(const uint8_t *buf, size_t size)
{
    if (!g_board)
        return false;
    g_state.assign(buf, buf + size);
    Archive ar(g_state, false);
    g_board->Serialize(ar);
    g_board->Audio().clear();
    g_audioAt = 0;
    return ar.Ok();
}

uint32_t Arcade_Hash(void)
{
    return g_board ? g_board->Hash() : 0;
}

} // extern "C"
