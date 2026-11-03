// Moemon Maker: the MAP panel and the TILES panel (docs/MOEMON_MAKER.md "Modes").
#include "imgui.h"
#include "maker_ui.h"

namespace maker {

void MapView_Opened() {}
void MapView_Closed() {}

void MapView_Panel()
{
    if (!ImGui::Begin("MAP"))
    {
        ImGui::End();
        return;
    }
    if (gEd.map < 0)
        ImGui::TextDisabled("Pick a map in MAPS.");
    else
        ImGui::Text("%s", gEd.maps[gEd.map].title);
    ImGui::End();
}

void TilesPanel()
{
    if (!ImGui::Begin("TILES"))
    {
        ImGui::End();
        return;
    }
    ImGui::TextDisabled("Open a map to see its tiles.");
    ImGui::End();
}

void MapView_TestCommand(const char *, const char *) {}

} // namespace maker
