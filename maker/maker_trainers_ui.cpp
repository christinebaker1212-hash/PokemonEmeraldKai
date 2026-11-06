// Moemon Maker, M2c: the TRAINERS window (Tools > Trainers). Any trainer's name, class, picture, items, AI, double battle
// and party (species, level, IVs, held item, four moves), kept as MoemonMaker/trainers.txt (src/maker/maker_trainers.c's
// format, each with a hash of the game's own trainer) and sent to the game as they change, so F5 battles the edited
// party at once. Ctrl+S (or the window's Save) writes the file.
#include <SDL.h>
#include <stdio.h>
#include <string.h>
#include <algorithm>
#include <map>
#include <string>
#include <vector>
#include "maker_ui.h"

namespace maker {

struct TrainerEdit
{
    unsigned int was = 0;
    MakerTrainerInfo t{};
};

static std::map<int, TrainerEdit> sTrainers;   // by trainer id
static bool sOpen, sLoaded, sDirty;
static int sSel = -1;
static char sFilter[64];

static const char *Name(int table, int v)
{
    const char *n = MakerApi_NameOf(table, (unsigned)v);
    return n && n[0] ? n : "?";
}

static std::string TrainersText()
{
    std::string out = "# Moemon Maker: the user's trainers (written by the editor; was= is the game's own trainer's hash)\n";
    char line[512];
    for (auto &kv : sTrainers)
    {
        const MakerTrainerInfo &t = kv.second.t;
        snprintf(line, sizeof(line), "trainer %s was=0x%08X\n", Name(MAKER_NAMES_TRAINER, kv.first), kv.second.was);
        out += line;
        snprintf(line, sizeof(line), "  name %s\n  class %d\n  pic %d\n  music %d\n  ai %u\n  double %d\n", t.name,
                 t.trainerClass, t.pic, t.music, t.ai, t.doubleBattle);
        out += line;
        out += "  items";
        for (int i = 0; i < 4; i++)
            out += std::string(" ") + (t.items[i] ? Name(MAKER_NAMES_ITEM, t.items[i]) : "ITEM_NONE");
        out += "\n";
        for (int i = 0; i < t.partySize; i++)
        {
            const MakerTrainerMon &m = t.party[i];
            snprintf(line, sizeof(line), "  mon %s %d iv=%d item=%s moves=%s,%s,%s,%s\n", Name(MAKER_NAMES_SPECIES, m.species),
                     m.level, m.iv, m.item ? Name(MAKER_NAMES_ITEM, m.item) : "ITEM_NONE",
                     m.moves[0] ? Name(MAKER_NAMES_MOVE, m.moves[0]) : "MOVE_NONE", m.moves[1] ? Name(MAKER_NAMES_MOVE, m.moves[1]) : "MOVE_NONE",
                     m.moves[2] ? Name(MAKER_NAMES_MOVE, m.moves[2]) : "MOVE_NONE", m.moves[3] ? Name(MAKER_NAMES_MOVE, m.moves[3]) : "MOVE_NONE");
            out += line;
        }
    }
    return out;
}

static unsigned Value(int table, const char *s)
{
    unsigned v = 0;
    if (!MakerApi_ValueOf(table, s, &v))
        v = strtoul(s, NULL, 0);
    return v;
}

// Reads MoemonMaker/trainers.txt back into the editor
static void Load()
{
    sLoaded = true;
    sTrainers.clear();
    FILE *f = fopen((EditsDir() + "/trainers.txt").c_str(), "rb");
    if (!f)
        return;
    char line[512];
    TrainerEdit *cur = nullptr;
    while (fgets(line, sizeof(line), f))
    {
        char a[128] = "", b[128] = "";
        line[strcspn(line, "\r\n")] = 0;
        if (sscanf(line, "trainer %127s was=%127s", a, b) >= 1 && !strncmp(line, "trainer ", 8))
        {
            int id = (int)Value(MAKER_NAMES_TRAINER, a);
            cur = &sTrainers[id];
            cur->was = strtoul(b, NULL, 0);
            MakerApi_TrainerInfo(id, &cur->t);
            cur->t.partySize = 0;
            continue;
        }
        if (!cur)
            continue;
        if (sscanf(line, " name %127[^\n]", a) == 1)
            snprintf(cur->t.name, sizeof(cur->t.name), "%s", a);
        else if (sscanf(line, " class %127s", a) == 1)
            cur->t.trainerClass = atoi(a);
        else if (sscanf(line, " pic %127s", a) == 1)
            cur->t.pic = atoi(a);
        else if (sscanf(line, " music %127s", a) == 1)
            cur->t.music = atoi(a);
        else if (sscanf(line, " ai %127s", a) == 1)
            cur->t.ai = strtoul(a, NULL, 0);
        else if (sscanf(line, " double %127s", a) == 1)
            cur->t.doubleBattle = atoi(a);
        else if (strstr(line, " items "))
        {
            char it[4][64] = {"", "", "", ""};
            sscanf(strstr(line, "items ") + 6, "%63s %63s %63s %63s", it[0], it[1], it[2], it[3]);
            for (int i = 0; i < 4; i++)
                cur->t.items[i] = it[i][0] ? (int)Value(MAKER_NAMES_ITEM, it[i]) : 0;
        }
        else if (sscanf(line, " mon %127s %127s", a, b) == 2 && cur->t.partySize < 6)
        {
            MakerTrainerMon &m = cur->t.party[cur->t.partySize++];
            m = MakerTrainerMon{};
            m.species = (int)Value(MAKER_NAMES_SPECIES, a);
            m.level = atoi(b);
            if (const char *q = strstr(line, "iv="))
                m.iv = atoi(q + 3);
            if (const char *q = strstr(line, "item="))
            {
                char it[64];
                sscanf(q + 5, "%63s", it);
                m.item = (int)Value(MAKER_NAMES_ITEM, it);
            }
            if (const char *q = strstr(line, "moves="))
            {
                char mv[300];
                sscanf(q + 6, "%299s", mv);
                int k = 0;
                for (char *tok = strtok(mv, ","); tok && k < 4; tok = strtok(NULL, ","), k++)
                    m.moves[k] = (int)Value(MAKER_NAMES_MOVE, tok);
            }
        }
    }
    fclose(f);
    MakerApi_SetTrainers(TrainersText().c_str());
}

static void Changed()
{
    sDirty = true;
    MakerApi_SetTrainers(TrainersText().c_str());
}

int Trainers_Save()
{
    if (!sDirty)
        return 0;
    std::string path = EditsDir() + "/trainers.txt";
    if (sTrainers.empty())
        remove(path.c_str());
    else if (FILE *f = fopen(path.c_str(), "wb"))
    {
        std::string text = TrainersText();
        fwrite(text.data(), 1, text.size(), f);
        fclose(f);
    }
    sDirty = false;
    return 1;
}

bool Trainers_Unsaved()
{
    return sDirty;
}

static bool Picker(const char *id, int table, int *value, const char *none)
{
    bool picked = false;
    char preview[96];
    snprintf(preview, sizeof(preview), "%s", *value == 0 && none ? none : Name(table, *value));
    ImGui::SetNextItemWidth(-FLT_MIN);
    if (ImGui::BeginCombo(id, preview, ImGuiComboFlags_HeightLarge))
    {
        static char filter[64];
        if (ImGui::IsWindowAppearing())
        {
            filter[0] = 0;
            ImGui::SetKeyboardFocusHere();
        }
        ImGui::InputTextWithHint("##f", "Search...", filter, sizeof(filter));
        std::string f = filter;
        std::transform(f.begin(), f.end(), f.begin(), ::toupper);
        if (none && ImGui::Selectable(none, *value == 0))
            *value = 0, picked = true;
        int n = MakerApi_NameCount(table);
        for (int i = 0; i < n; i++)
        {
            unsigned v;
            const char *nm = MakerApi_NameAt(table, i, &v);
            if (!f.empty() && !strstr(nm, f.c_str()))
                continue;
            ImGui::PushID(i);
            if (ImGui::Selectable(nm, (int)v == *value))
                *value = (int)v, picked = true;
            ImGui::PopID();
        }
        ImGui::EndCombo();
    }
    return picked;
}

// The battle bench (M3): battle this trainer at once in a test game, and a damage estimate both ways
static int sBenchKind = MAKER_PLAY_BADGES4, sBenchSpecies = 280, sBenchLevel = 30, sBenchMoves[4] = {33, 0, 0, 0}, sBenchFoe;

static void DamageLine(const char *who, int atkSp, int atkLv, int defSp, int defLv, int move)
{
    MakerDamage d;
    if (!move)
        return;
    if (!MakerApi_Damage(atkSp, atkLv, defSp, defLv, move, &d))
    {
        ImGui::TextDisabled("%s %s: no set damage", who, Name(MAKER_NAMES_MOVE, move) + 5);
        return;
    }
    int lo = d.defenderHp ? d.min * 100 / d.defenderHp : 0, hi = d.defenderHp ? d.max * 100 / d.defenderHp : 0;
    ImVec4 col = d.effectiveness == 0 ? ImVec4(0.5f, 0.5f, 0.5f, 1) : d.effectiveness > 100 ? ImVec4(0.2f, 0.7f, 0.2f, 1)
               : d.effectiveness < 100 ? ImVec4(0.8f, 0.4f, 0.2f, 1) : ImGui::GetStyleColorVec4(ImGuiCol_Text);
    ImGui::TextColored(col, "%s %-14s %3d-%3d HP  (%d-%d%% of %d)%s%s", who, Name(MAKER_NAMES_MOVE, move) + 5, d.min, d.max,
                       lo, hi, d.defenderHp, d.stab ? "  STAB" : "", d.effectiveness == 0 ? "  no effect"
                       : d.effectiveness > 100 ? "  super effective" : d.effectiveness < 100 ? "  not very effective" : "");
}

static void BenchPanel(int trainer, const MakerTrainerInfo &t)
{
    ImGui::SeparatorText("BATTLE BENCH");
    ImGui::TextDisabled("Battle this trainer in a test game (never your real save), as it is now with your edits.");
    ImGui::RadioButton("Start of the game", &sBenchKind, MAKER_PLAY_START);
    ImGui::SameLine();
    ImGui::RadioButton("Four badges", &sBenchKind, MAKER_PLAY_BADGES4);
    ImGui::SameLine();
    ImGui::RadioButton("Champion", &sBenchKind, MAKER_PLAY_CHAMPION);
    ImGui::SameLine();
    ImGui::RadioButton("Testing team", &sBenchKind, MAKER_PLAY_TEST);
    ImGui::SameLine();
    ImGui::RadioButton("A copy of save file 1", &sBenchKind, MAKER_PLAY_SAVE);
    bool playing = MakerApi_IsPlaying();
    ImGui::BeginDisabled(playing);
    if (ImGui::Button("Battle this trainer now"))
    {
        int map = gEd.map >= 0 ? gEd.map : MakerApi_FindMap("MAP_LITTLEROOT_TOWN");
        MakerMapInfo info;
        if (map >= 0 && MakerApi_MapInfo(map, &info))
        {
            Trainers_Save();
            if (MakerApi_PlayBattle(map, info.width / 2, info.height / 2, sBenchKind, 1, trainer))
                Message("Battling %s. F12 or closing the game's window comes back.", Name(MAKER_NAMES_TRAINER, trainer));
        }
    }
    ImGui::EndDisabled();
    if (playing)
        ImGui::SameLine(), ImGui::TextDisabled("(a test game is running)");
    if (t.partySize <= 0)
        return;
    ImGui::Spacing();
    ImGui::TextDisabled("Damage estimate (perfect IVs, no EVs, neutral natures; before abilities, items and weather):");
    ImGui::SetNextItemWidth(200 * gEd.uiScale);
    ImGui::BeginGroup();
    ImGui::Text("Your POKeMON");
    ImGui::SetNextItemWidth(200 * gEd.uiScale);
    Picker("##bsp", MAKER_NAMES_SPECIES, &sBenchSpecies, NULL);
    ImGui::SetNextItemWidth(200 * gEd.uiScale);
    ImGui::InputInt("Level##blv", &sBenchLevel);
    sBenchLevel = std::max(1, std::min(sBenchLevel, 100));
    for (int k = 0; k < 4; k++)
    {
        ImGui::PushID(200 + k);
        ImGui::SetNextItemWidth(200 * gEd.uiScale);
        Picker("##bmv", MAKER_NAMES_MOVE, &sBenchMoves[k], "(none)");
        ImGui::PopID();
    }
    ImGui::EndGroup();
    ImGui::SameLine();
    ImGui::BeginGroup();
    ImGui::Text("Against");
    sBenchFoe = std::max(0, std::min(sBenchFoe, t.partySize - 1));
    for (int i = 0; i < t.partySize; i++)
    {
        char label[64];
        snprintf(label, sizeof(label), "%s Lv%d##foe%d", Name(MAKER_NAMES_SPECIES, t.party[i].species) + 8, t.party[i].level, i);
        ImGui::RadioButton(label, &sBenchFoe, i);
        if (i % 3 != 2 && i + 1 < t.partySize)
            ImGui::SameLine();
    }
    const MakerTrainerMon &foe = t.party[sBenchFoe];
    for (int k = 0; k < 4; k++)
        DamageLine("You:", sBenchSpecies, sBenchLevel, foe.species, foe.level, sBenchMoves[k]);
    bool any = false;
    for (int k = 0; k < 4; k++)
        if (foe.moves[k])
            any = true, DamageLine("Foe:", foe.species, foe.level, sBenchSpecies, sBenchLevel, foe.moves[k]);
    if (!any)
        ImGui::TextDisabled("(its moves are its own at that level: set them above to see their damage)");
    ImGui::EndGroup();
}

static const char *const kAiFlags[] = {"Checks bad moves", "Tries to KO", "Checks viability", "Setup first turn",
                                       "Risky", "Prefers strongest", "Prefers baton pass", "Double battle"};

void TrainersWindow()
{
    if (!sOpen)
        return;
    if (!sLoaded)
        Load();
    ImGui::SetNextWindowSize(ImVec2(900 * gEd.uiScale, 560 * gEd.uiScale), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("TRAINERS", &sOpen))
    {
        ImGui::End();
        return;
    }
    ImGui::BeginChild("list", ImVec2(260 * gEd.uiScale, 0), ImGuiChildFlags_Borders);
    ImGui::SetNextItemWidth(-FLT_MIN);
    ImGui::InputTextWithHint("##tf", "Find a trainer...", sFilter, sizeof(sFilter));
    std::string f = sFilter;
    std::transform(f.begin(), f.end(), f.begin(), ::toupper);
    int n = MakerApi_NameCount(MAKER_NAMES_TRAINER);
    for (int i = 0; i < n; i++)
    {
        unsigned v;
        const char *nm = MakerApi_NameAt(MAKER_NAMES_TRAINER, i, &v);
        if (v == 0 || (!f.empty() && !strstr(nm, f.c_str())))
            continue;
        ImGui::PushID(i);
        bool edited = sTrainers.count((int)v) > 0;
        if (edited)
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.6f, 0.1f, 1));
        if (ImGui::Selectable(nm + 8, sSel == (int)v))
            sSel = (int)v;
        if (edited)
            ImGui::PopStyleColor();
        ImGui::PopID();
    }
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("detail", ImVec2(0, 0));
    MakerTrainerInfo base;
    if (sSel <= 0 || !MakerApi_TrainerInfo(sSel, &base))
    {
        ImGui::TextDisabled("Pick a trainer. Edited ones are orange.");
        ImGui::EndChild();
        ImGui::End();
        return;
    }
    bool edited = sTrainers.count(sSel) > 0;
    TrainerEdit tmp;
    tmp.t = base;
    tmp.was = base.hash;
    TrainerEdit &e = edited ? sTrainers[sSel] : tmp;
    ImGui::PushFont(gEd.fontBold);
    ImGui::Text("%s", Name(MAKER_NAMES_TRAINER, sSel));
    ImGui::PopFont();
    if (edited && e.was != base.hash)
        ImGui::TextColored(ImVec4(0.85f, 0.2f, 0.2f, 1), "The game changed this trainer since your edit: it battles as the game has it until you save again.");
    bool ch = false;
    char name[16];
    snprintf(name, sizeof(name), "%s", e.t.name);
    ImGui::SetNextItemWidth(200 * gEd.uiScale);
    if (ImGui::InputText("Name", name, 11))
        snprintf(e.t.name, sizeof(e.t.name), "%s", name), ch = true;
    ImGui::SetNextItemWidth(120 * gEd.uiScale);
    ch |= ImGui::InputInt("Class", &e.t.trainerClass);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(120 * gEd.uiScale);
    ch |= ImGui::InputInt("Picture", &e.t.pic);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(120 * gEd.uiScale);
    ch |= ImGui::InputInt("Music", &e.t.music);
    bool dbl = e.t.doubleBattle != 0;
    if (ImGui::Checkbox("Double battle", &dbl))
        e.t.doubleBattle = dbl, ch = true;
    ImGui::TextDisabled("AI:");
    for (int b = 0; b < 8; b++)
    {
        ImGui::SameLine();
        bool on = (e.t.ai >> b) & 1;
        if (ImGui::Checkbox(kAiFlags[b], &on))
            e.t.ai = on ? (e.t.ai | (1u << b)) : (e.t.ai & ~(1u << b)), ch = true;
        if (b == 3)
            ImGui::NewLine(), ImGui::Dummy(ImVec2(18 * gEd.uiScale, 0));
    }
    ImGui::TextDisabled("Items the trainer uses in battle:");
    for (int i = 0; i < 4; i++)
    {
        ImGui::PushID(100 + i);
        ImGui::SetNextItemWidth(170 * gEd.uiScale);
        ImGui::BeginGroup();
        ch |= Picker("##it", MAKER_NAMES_ITEM, &e.t.items[i], "(none)");
        ImGui::EndGroup();
        ImGui::PopID();
        if (i < 3)
            ImGui::SameLine();
    }
    ImGui::SeparatorText("PARTY");
    if (ImGui::BeginTable("party", 8, ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchProp))
    {
        for (const char *h : {"Species", "Lv", "IVs", "Holds", "Move 1", "Move 2", "Move 3", "Move 4"})
            ImGui::TableSetupColumn(h);
        ImGui::TableHeadersRow();
        for (int i = 0; i < e.t.partySize; i++)
        {
            MakerTrainerMon &m = e.t.party[i];
            ImGui::PushID(i);
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ch |= Picker("##sp", MAKER_NAMES_SPECIES, &m.species, NULL);
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-FLT_MIN);
            ch |= ImGui::InputInt("##lv", &m.level, 0);
            m.level = std::max(1, std::min(m.level, 100));
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-FLT_MIN);
            ch |= ImGui::InputInt("##iv", &m.iv, 0);
            m.iv = std::max(0, std::min(m.iv, 255));
            ImGui::TableNextColumn();
            ch |= Picker("##hi", MAKER_NAMES_ITEM, &m.item, "(none)");
            for (int k = 0; k < 4; k++)
            {
                ImGui::TableNextColumn();
                ImGui::PushID(k);
                ch |= Picker("##mv", MAKER_NAMES_MOVE, &m.moves[k], "(its own)");
                ImGui::PopID();
            }
            ImGui::PopID();
        }
        ImGui::EndTable();
    }
    if (e.t.partySize < 6 && ImGui::Button("Add a POKeMON"))
    {
        e.t.party[e.t.partySize] = e.t.partySize ? e.t.party[e.t.partySize - 1] : MakerTrainerMon{1, 5, 0, 0, {0, 0, 0, 0}};
        e.t.partySize++;
        ch = true;
    }
    if (e.t.partySize > 1)
    {
        ImGui::SameLine();
        if (ImGui::Button("Remove the last"))
            e.t.partySize--, ch = true;
    }
    ImGui::TextDisabled("Moves left as \"(its own)\" are the ones it knows at that level. IVs: 0-255 as the game counts them.");
    BenchPanel(sSel, e.t);
    if (ch)
    {
        if (!edited)
            sTrainers[sSel] = e;
        Changed();
    }
    if (edited)
    {
        if (ImGui::Button("Back to the game's own"))
        {
            sTrainers.erase(sSel);
            Changed();
        }
        ImGui::SameLine();
    }
    if (ImGui::Button("Save (Ctrl+S)") && Trainers_Save())
        Message("Saved the trainers");
    ImGui::EndChild();
    ImGui::End();
}

void OpenTrainers()
{
    sOpen = true;
}

// Tests: "maker trainer TRAINER_X SPECIES LEVEL" gives a trainer a one-POKeMON party; "maker trainers" opens the window
void Trainers_TestCommand(const char *cmd, const char *arg)
{
    std::string c = cmd;
    if (!sLoaded)
        Load();
    if (c == "trainers")
        sOpen = true, sSel = arg[0] ? (int)Value(MAKER_NAMES_TRAINER, arg) : sSel;
    else if (c == "trainer")
    {
        char t[64] = "", sp[64] = "";
        int lv = 5;
        sscanf(arg, "%63s %63s %d", t, sp, &lv);
        int id = (int)Value(MAKER_NAMES_TRAINER, t);
        TrainerEdit e;
        if (!MakerApi_TrainerInfo(id, &e.t))
            return;
        e.was = e.t.hash;
        e.t.partySize = 1;
        e.t.party[0] = MakerTrainerMon{(int)Value(MAKER_NAMES_SPECIES, sp), lv, 0, 0, {0, 0, 0, 0}};
        sTrainers[id] = e;
        sSel = id;
        Changed();
        Trainers_Save();
    }
    else if (c == "bench")
    {
        // bench TRAINER KIND: battle it in a test game at once (KIND: MAKER_PLAY_*)
        char t[64] = "";
        int kind = MAKER_PLAY_TEST;
        sscanf(arg, "%63s %d", t, &kind);
        int id = (int)Value(MAKER_NAMES_TRAINER, t), map = gEd.map >= 0 ? gEd.map : MakerApi_FindMap("MAP_LITTLEROOT_TOWN");
        MakerMapInfo info;
        if (map >= 0 && MakerApi_MapInfo(map, &info))
            MakerApi_PlayBattle(map, info.width / 2, info.height / 2, kind, 1, id);
    }
    else if (c == "damage")
    {
        // damage SPECIES LEVEL SPECIES LEVEL MOVE: the bench's estimate, printed
        char a[64], d[64], m[64];
        int la = 50, ld = 50;
        MakerDamage r;
        if (sscanf(arg, "%63s %d %63s %d %63s", a, &la, d, &ld, m) == 5
         && MakerApi_Damage((int)Value(MAKER_NAMES_SPECIES, a), la, (int)Value(MAKER_NAMES_SPECIES, d), ld, (int)Value(MAKER_NAMES_MOVE, m), &r))
            printf("[maker] damage %s -> %s with %s: %d-%d of %d HP, x%d%%%s\n", a, d, m, r.min, r.max, r.defenderHp,
                   r.effectiveness, r.stab ? " STAB" : "");
    }
}

} // namespace maker
