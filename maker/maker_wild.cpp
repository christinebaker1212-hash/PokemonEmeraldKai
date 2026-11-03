// Moemon Maker: the ENCOUNTERS panel (docs/MOEMON_MAKER.md, decision 10): each of the map's wild tables as a simple list
// (each Pokémon once, with its total chance and levels) and, in a tab, the exact slots with their fixed chances; both
// edit the same slots. Changing a chance in the list spreads the Pokémon over the slots as close to the chances asked
// for as the fixed slots allow, and shows what it came to. Edits: "wild AREA SLOT PROP WAS NOW" (maker_edits.cpp),
// applied by the game over its GENERATION's pools (src/maker/maker_overrides.c Maker_WildInfo).
#include <stdio.h>
#include <string.h>
#include <algorithm>
#include <map>
#include <string>
#include <vector>
#include "imgui.h"
#include "maker_ui.h"

namespace maker {

static const char *const kAreaKeys[MAKER_WILD_AREAS] = {"land", "water", "rock", "fish"};
static const char *const kAreaNames[MAKER_WILD_AREAS] = {"GRASS / CAVE", "SURFING", "ROCK SMASH", "FISHING"};
static const int kLand[12] = {20, 20, 10, 10, 10, 10, 5, 5, 4, 4, 1, 1};
static const int kWater[5] = {60, 30, 5, 4, 1};
static const int kFish[10] = {70, 30, 60, 20, 20, 40, 40, 15, 4, 1};

static int Chance(int area, int slot)
{
    switch (area)
    {
    case MAKER_WILD_LAND: return kLand[slot];
    case MAKER_WILD_FISH: return kFish[slot];
    default: return kWater[slot];
    }
}

// The groups of slots chances add up within: one, or for fishing each rod's
struct Group { const char *name; int first, count; };
static std::vector<Group> Groups(int area)
{
    if (area == MAKER_WILD_FISH)
        return {{"OLD ROD", 0, 2}, {"GOOD ROD", 2, 3}, {"SUPER ROD", 5, 5}};
    return {{"", 0, MakerApi_WildSlotCount(area)}};
}

struct Table { int count = 0, rate = 0; MakerWildSlot base[12], cur[12]; };

static std::string SlotKey(int area, int slot)
{
    return std::string(kAreaKeys[area]) + " " + std::to_string(slot);
}

static Table Load(int map, int area)
{
    Table t;
    t.count = MakerApi_WildArea(map, area, t.base, &t.rate);
    for (int i = 0; i < t.count; i++)
    {
        std::string k = SlotKey(area, i);
        t.cur[i].species = (int)CurrentProp(map, "wild", k, "species", t.base[i].species);
        t.cur[i].minLevel = (int)CurrentProp(map, "wild", k, "min", t.base[i].minLevel);
        t.cur[i].maxLevel = (int)CurrentProp(map, "wild", k, "max", t.base[i].maxLevel);
    }
    return t;
}

static void SetSlot(int map, int area, const Table &t, int slot, const MakerWildSlot &v)
{
    std::string k = SlotKey(area, slot);
    SetProp(map, "wild", k, "species", t.base[slot].species, v.species);
    SetProp(map, "wild", k, "min", t.base[slot].minLevel, v.minLevel);
    SetProp(map, "wild", k, "max", t.base[slot].maxLevel, v.maxLevel);
}

static const char *SpeciesName(int species)
{
    const char *n = MakerApi_NameOf(MAKER_NAMES_SPECIES, (unsigned)species);
    return n && n[0] ? (strncmp(n, "SPECIES_", 8) == 0 ? n + 8 : n) : "?";
}

static bool SpeciesPicker(const char *id, int *species)
{
    bool picked = false;
    ImGui::SetNextItemWidth(-FLT_MIN);
    if (ImGui::BeginCombo(id, SpeciesName(*species), ImGuiComboFlags_HeightLarge))
    {
        static char filter[48];
        if (ImGui::IsWindowAppearing())
        {
            filter[0] = 0;
            ImGui::SetKeyboardFocusHere();
        }
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::InputTextWithHint("##f", "Search...", filter, sizeof(filter));
        int n = MakerApi_NameCount(MAKER_NAMES_SPECIES);
        for (int i = 0; i < n; i++)
        {
            unsigned v;
            const char *name = MakerApi_NameAt(MAKER_NAMES_SPECIES, i, &v);
            if (v == 0 || strncmp(name, "SPECIES_", 8) != 0)
                continue;
            if (filter[0])
            {
                bool hit = false;
                for (const char *p = name; *p && !hit; p++)
                    hit = strncasecmp(p, filter, strlen(filter)) == 0;
                if (!hit)
                    continue;
            }
            ImGui::PushID(i);
            if (ImGui::Selectable(name + 8, (int)v == *species))
                *species = v, picked = true;
            ImGui::PopID();
        }
        ImGui::EndCombo();
    }
    return picked;
}

// ---------------------------------------------------------------------------------------------------------------------
// The list: each Pokémon once

struct Row { int species, chance, minLevel, maxLevel; };

static std::vector<Row> Rows(const Table &t, int area, const Group &g)
{
    std::vector<Row> rows;
    for (int i = g.first; i < g.first + g.count; i++)
    {
        const MakerWildSlot &s = t.cur[i];
        auto it = std::find_if(rows.begin(), rows.end(), [&](const Row &r) { return r.species == s.species; });
        if (it == rows.end())
            rows.push_back(Row{s.species, Chance(area, i), s.minLevel, s.maxLevel});
        else
        {
            it->chance += Chance(area, i);
            it->minLevel = std::min(it->minLevel, s.minLevel);
            it->maxLevel = std::max(it->maxLevel, s.maxLevel);
        }
    }
    std::sort(rows.begin(), rows.end(), [](const Row &a, const Row &b) { return a.chance > b.chance; });
    return rows;
}

// One row's chance set to `pct`: the others keep their share of what's left (so 50% is 50%, the rest scaled to fit)
static std::vector<Row> WithChance(std::vector<Row> rows, int r, int pct)
{
    int others = 0;
    for (size_t i = 0; i < rows.size(); i++)
        if ((int)i != r)
            others += rows[i].chance;
    for (size_t i = 0; i < rows.size(); i++)
        if ((int)i == r)
            rows[i].chance = pct;
        else if (others > 0)
            rows[i].chance = std::max(1, (int)(rows[i].chance * (100.0 - pct) / others + 0.5));
    return rows;
}

// The rows' Pokémon over the group's slots, as close to each one's chance as the fixed slots allow: the biggest slot
// first, each to the Pokémon furthest below what it asked for
static void Spread(int map, int area, const Table &t, const Group &g, const std::vector<Row> &rows)
{
    std::vector<int> order;
    for (int i = g.first; i < g.first + g.count; i++)
        order.push_back(i);
    std::stable_sort(order.begin(), order.end(), [&](int a, int b) { return Chance(area, a) > Chance(area, b); });
    std::vector<int> got(rows.size(), 0);
    int total = 0;
    for (const Row &r : rows)
        total += r.chance;
    if (total <= 0 || rows.empty())
        return;
    for (int slot : order)
    {
        int best = 0;
        double bestGap = -1e9;
        for (size_t r = 0; r < rows.size(); r++)
        {
            double want = rows[r].chance * 100.0 / total, gap = want - got[r];
            if (gap > bestGap)
                bestGap = gap, best = (int)r;
        }
        got[best] += Chance(area, slot);
        SetSlot(map, area, t, slot, MakerWildSlot{rows[best].species, rows[best].minLevel, rows[best].maxLevel});
    }
}

static void ListView(int map, int area, const Table &t)
{
    for (const Group &g : Groups(area))
    {
        if (g.name[0])
            ImGui::SeparatorText(g.name);
        std::vector<Row> rows = Rows(t, area, g);
        ImGui::PushID(g.first);
        if (ImGui::BeginTable("list", 5, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_SizingStretchProp))
        {
            ImGui::TableSetupColumn("POKéMON", ImGuiTableColumnFlags_WidthStretch, 3);
            ImGui::TableSetupColumn("CHANCE %", ImGuiTableColumnFlags_WidthStretch, 1.4f);
            ImGui::TableSetupColumn("FROM Lv", ImGuiTableColumnFlags_WidthStretch, 1.2f);
            ImGui::TableSetupColumn("TO Lv", ImGuiTableColumnFlags_WidthStretch, 1.2f);
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 24);
            ImGui::TableHeadersRow();
            int remove = -1;
            for (size_t r = 0; r < rows.size(); r++)
            {
                Row row = rows[r];
                ImGui::PushID((int)r);
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                bool changed = false, respread = false;
                if (SpeciesPicker("##sp", &row.species))
                    changed = true;
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-FLT_MIN);
                if (ImGui::InputInt("##ch", &row.chance, 1, 5) && row.chance >= 1 && row.chance <= 100)
                    respread = true;
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-FLT_MIN);
                if (ImGui::InputInt("##lo", &row.minLevel, 0, 0) && row.minLevel >= 1 && row.minLevel <= 100)
                    changed = true;
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-FLT_MIN);
                if (ImGui::InputInt("##hi", &row.maxLevel, 0, 0) && row.maxLevel >= 1 && row.maxLevel <= 100)
                    changed = true;
                ImGui::TableNextColumn();
                if (rows.size() > 1 && ImGui::SmallButton("x"))
                    remove = (int)r;
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Take this Pokémon out (its chance goes to the others)");
                if (changed)
                {
                    // (its slots take the new species / levels)
                    if (row.maxLevel < row.minLevel)
                        row.maxLevel = row.minLevel;
                    BeginChange(map);
                    for (int i = g.first; i < g.first + g.count; i++)
                        if (t.cur[i].species == rows[r].species)
                            SetSlot(map, area, t, i, MakerWildSlot{row.species, row.minLevel, row.maxLevel});
                    EndChange();
                }
                if (respread)
                {
                    std::vector<Row> want = WithChance(rows, (int)r, row.chance);
                    BeginChange(map);
                    Spread(map, area, t, g, want);
                    EndChange();
                }
                ImGui::PopID();
            }
            ImGui::EndTable();
            if (remove >= 0)
            {
                std::vector<Row> want = rows;
                want.erase(want.begin() + remove);
                BeginChange(map);
                Spread(map, area, t, g, want);
                EndChange();
            }
        }
        if (ImGui::SmallButton("+ Add a POKéMON"))
        {
            // (a new one at 10%, the rest spread round it)
            std::vector<Row> want = rows;
            Row n = want.empty() ? Row{1, 10, 2, 3} : want.back();
            n.species = 1;
            n.chance = 10;
            want.push_back(n);
            BeginChange(map);
            Spread(map, area, t, g, want);
            EndChange();
        }
        ImGui::SameLine();
        ImGui::TextDisabled("The chances are the slots' (fixed by the game); a changed chance is spread as close as they allow.");
        ImGui::PopID();
    }
}

// ---------------------------------------------------------------------------------------------------------------------
// The slots: exactly as the game has them

static void SlotsView(int map, int area, const Table &t)
{
    for (const Group &g : Groups(area))
    {
        if (g.name[0])
            ImGui::SeparatorText(g.name);
        ImGui::PushID(g.first + 100);
        if (ImGui::BeginTable("slots", 5, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_SizingStretchProp))
        {
            ImGui::TableSetupColumn("SLOT", ImGuiTableColumnFlags_WidthFixed, 40);
            ImGui::TableSetupColumn("CHANCE", ImGuiTableColumnFlags_WidthFixed, 60);
            ImGui::TableSetupColumn("POKéMON", ImGuiTableColumnFlags_WidthStretch, 3);
            ImGui::TableSetupColumn("FROM Lv", ImGuiTableColumnFlags_WidthStretch, 1);
            ImGui::TableSetupColumn("TO Lv", ImGuiTableColumnFlags_WidthStretch, 1);
            ImGui::TableHeadersRow();
            for (int i = g.first; i < g.first + g.count; i++)
            {
                MakerWildSlot v = t.cur[i];
                bool edited = v.species != t.base[i].species || v.minLevel != t.base[i].minLevel || v.maxLevel != t.base[i].maxLevel;
                ImGui::PushID(i);
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text(edited ? "%d *" : "%d", i + 1);
                ImGui::TableNextColumn();
                ImGui::Text("%d%%", Chance(area, i));
                ImGui::TableNextColumn();
                bool changed = SpeciesPicker("##sp", &v.species);
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-FLT_MIN);
                changed |= ImGui::InputInt("##lo", &v.minLevel, 0, 0) && v.minLevel >= 1 && v.minLevel <= 100;
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-FLT_MIN);
                changed |= ImGui::InputInt("##hi", &v.maxLevel, 0, 0) && v.maxLevel >= 1 && v.maxLevel <= 100;
                if (changed)
                {
                    BeginChange(map);
                    SetSlot(map, area, t, i, v);
                    EndChange();
                }
                ImGui::PopID();
            }
            ImGui::EndTable();
        }
        ImGui::PopID();
    }
}

void WildPanel()
{
    if (!ImGui::Begin("ENCOUNTERS"))
    {
        ImGui::End();
        return;
    }
    int map = gEd.map;
    if (map < 0)
    {
        ImGui::TextDisabled("Pick a map in MAPS.");
        ImGui::End();
        return;
    }
    bool any = false;
    if (ImGui::BeginTabBar("areas"))
    {
        for (int a = 0; a < MAKER_WILD_AREAS; a++)
        {
            Table t = Load(map, a);
            if (!t.count)
                continue;
            any = true;
            if (!ImGui::BeginTabItem(kAreaNames[a]))
                continue;
            ImGui::TextDisabled("Encounter rate %d", t.rate);
            if (ImGui::BeginTabBar("view"))
            {
                if (ImGui::BeginTabItem("List"))
                {
                    ListView(map, a, t);
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Slots"))
                {
                    SlotsView(map, a, t);
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    if (!any)
        ImGui::TextDisabled("%s has no wild POKéMON.", gEd.maps[map].title);
    ImGui::End();
}

// Tests: "maker wild AREA SLOT SPECIES MIN MAX" (a slot), "maker wildchance AREA SPECIES PERCENT" (the list's chance)
void Wild_TestCommand(const char *cmd, const char *arg)
{
    std::string c = cmd;
    int map = gEd.map;
    char areaName[16], species[64];
    int area;
    if (map < 0)
        return;
    if (c == "wild")
    {
        int slot, lo, hi;
        unsigned sp;
        if (sscanf(arg, "%15s %d %63s %d %d", areaName, &slot, species, &lo, &hi) != 5)
            return;
        for (area = 0; area < MAKER_WILD_AREAS && strcmp(areaName, kAreaKeys[area]); area++)
            ;
        Table t = Load(map, area);
        if (area == MAKER_WILD_AREAS || slot >= t.count || !MakerApi_ValueOf(MAKER_NAMES_SPECIES, species, &sp))
            return;
        BeginChange(map);
        SetSlot(map, area, t, slot, MakerWildSlot{(int)sp, lo, hi});
        EndChange();
    }
    else if (c == "wildchance")
    {
        int pct;
        unsigned sp;
        if (sscanf(arg, "%15s %63s %d", areaName, species, &pct) != 3)
            return;
        for (area = 0; area < MAKER_WILD_AREAS && strcmp(areaName, kAreaKeys[area]); area++)
            ;
        Table t = Load(map, area);
        if (area == MAKER_WILD_AREAS || !t.count || !MakerApi_ValueOf(MAKER_NAMES_SPECIES, species, &sp))
            return;
        Group g = Groups(area)[0];
        std::vector<Row> rows = Rows(t, area, g);
        int at = -1;
        for (size_t i = 0; i < rows.size(); i++)
            if (rows[i].species == (int)sp)
                at = (int)i;
        if (at < 0)
        {
            rows.push_back(Row{(int)sp, 0, rows.empty() ? 2 : rows[0].minLevel, rows.empty() ? 3 : rows[0].maxLevel});
            at = (int)rows.size() - 1;
        }
        rows = WithChance(rows, at, pct);
        BeginChange(map);
        Spread(map, area, t, g, rows);
        EndChange();
        rows = Rows(Load(map, area), area, g);
        printf("[maker] wild %s list:", areaName);
        for (const Row &r : rows)
            printf(" %s=%d%%", SpeciesName(r.species), r.chance);
        printf("\n");
    }
}

} // namespace maker
