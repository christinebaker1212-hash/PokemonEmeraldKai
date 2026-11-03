// Moemon Maker: the toolbar's buttons, their icons drawn as shapes (no picture files to ship or keep in step).
#include <math.h>
#include "imgui.h"
#include "imgui_internal.h"
#include "maker_ui.h"

namespace maker {

static void DrawIcon(ImDrawList *dl, int icon, ImVec2 p, float s, ImU32 col, ImU32 accent)
{
    // p: the icon's top-left, s: its size (an 18x18 design scaled)
    auto P = [&](float x, float y) { return ImVec2(p.x + x * s / 18.0f, p.y + y * s / 18.0f); };
    float t = s / 18.0f * 1.5f;   // line thickness
    switch (icon)
    {
    case ICON_SAVE:
        dl->AddRect(P(2, 2), P(16, 16), col, 1, 0, t);
        dl->AddRectFilled(P(5, 2), P(13, 7), col);
        dl->AddRect(P(5, 10), P(13, 16), col, 0, 0, t);
        break;
    case ICON_UNDO:
    case ICON_REDO:
    {
        bool undo = icon == ICON_UNDO;
        float dir = undo ? 1.0f : -1.0f, cx = 9;
        dl->PathArcTo(P(cx, 10), 5 * s / 18.0f, undo ? -IM_PI * 0.9f : -IM_PI * 0.1f, undo ? IM_PI * 0.5f : IM_PI * 0.5f - IM_PI * 1.4f, 12);
        dl->PathStroke(col, 0, t);
        ImVec2 a = undo ? P(4.2f, 9) : P(13.8f, 9);
        dl->AddTriangleFilled(ImVec2(a.x - 3 * dir * s / 18, a.y - 3 * s / 18), ImVec2(a.x + 3 * dir * s / 18, a.y - 3 * s / 18),
                              ImVec2(a.x, a.y + 2 * s / 18), col);
        break;
    }
    case ICON_MODE_MAP:
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                dl->AddRectFilled(P(2 + i * 5, 2 + j * 5), P(6 + i * 5, 6 + j * 5), (i + j) & 1 ? accent : col);
        break;
    case ICON_MODE_COLLISION:
        dl->AddRect(P(2, 2), P(16, 16), col, 0, 0, t);
        dl->AddLine(P(5, 5), P(13, 13), IM_COL32(210, 40, 40, 255), t * 1.3f);
        dl->AddLine(P(13, 5), P(5, 13), IM_COL32(210, 40, 40, 255), t * 1.3f);
        break;
    case ICON_MODE_EVENTS:
        dl->AddCircleFilled(P(9, 5), 3 * s / 18, col);
        dl->AddRectFilled(P(5, 9), P(13, 16), col, 2 * s / 18);
        break;
    case ICON_MODE_ENCOUNTERS:
        for (int i = 0; i < 4; i++)
        {
            float x = 3 + i * 4;
            dl->AddTriangleFilled(P(x - 2, 16), P(x + 2, 16), P(x + (i & 1 ? 1 : -1), 4 + (i & 1) * 3), IM_COL32(60, 160, 60, 255));
        }
        break;
    case ICON_PENCIL:
        dl->AddLine(P(4, 14), P(14, 4), col, t * 2.2f);
        dl->AddTriangleFilled(P(2, 16), P(3, 12), P(6, 15), col);
        break;
    case ICON_RECT:
        dl->AddRect(P(3, 4), P(15, 14), col, 0, 0, t);
        dl->AddRectFilled(P(5, 6), P(13, 12), accent);
        break;
    case ICON_FILL:
        dl->AddQuadFilled(P(3, 8), P(9, 2), P(15, 8), P(9, 14), col);
        dl->AddCircleFilled(P(15, 13), 2.2f * s / 18, accent);
        break;
    case ICON_PICKER:
        dl->AddLine(P(4, 14), P(11, 7), col, t * 1.6f);
        dl->AddCircleFilled(P(13, 5), 3 * s / 18, col);
        dl->AddCircleFilled(P(3.5f, 14.5f), 1.6f * s / 18, accent);
        break;
    case ICON_ZOOM_OUT:
    case ICON_ZOOM_IN:
        dl->AddCircle(P(8, 8), 5 * s / 18, col, 16, t);
        dl->AddLine(P(11.5f, 11.5f), P(16, 16), col, t * 1.6f);
        dl->AddLine(P(5.5f, 8), P(10.5f, 8), col, t);
        if (icon == ICON_ZOOM_IN)
            dl->AddLine(P(8, 5.5f), P(8, 10.5f), col, t);
        break;
    case ICON_PLAY:
        dl->AddTriangleFilled(P(4, 2), P(4, 16), P(16, 9), IM_COL32(40, 170, 70, 255));
        break;
    }
}

bool IconButton(const char *id, int icon, const char *tooltip, bool selected, bool enabled)
{
    float s = 30.0f * gEd.uiScale;
    ImGui::PushID(id);
    if (!enabled)
        ImGui::BeginDisabled();
    if (selected)
        ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_HeaderActive));
    ImVec2 pos = ImGui::GetCursorScreenPos();
    bool pressed = ImGui::Button("##b", ImVec2(s, s));
    if (selected)
        ImGui::PopStyleColor();
    ImU32 col = ImGui::GetColorU32(ImGuiCol_Text);
    ImU32 accent = ImGui::GetColorU32(ImGuiCol_CheckMark);
    float pad = s * 0.2f;
    DrawIcon(ImGui::GetWindowDrawList(), icon, ImVec2(pos.x + pad, pos.y + pad), s - 2 * pad, col, accent);
    if (!enabled)
        ImGui::EndDisabled();
    if (tooltip && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled | ImGuiHoveredFlags_DelayShort))
        ImGui::SetTooltip("%s", tooltip);
    ImGui::PopID();
    return pressed && enabled;
}

void ToolbarSeparator()
{
    ImGui::SameLine();
    ImVec2 p = ImGui::GetCursorScreenPos();
    float h = 30.0f * gEd.uiScale;
    ImGui::GetWindowDrawList()->AddLine(ImVec2(p.x + 3, p.y + 3), ImVec2(p.x + 3, p.y + h - 3), ImGui::GetColorU32(ImGuiCol_Separator), 1);
    ImGui::Dummy(ImVec2(7, h));
    ImGui::SameLine();
}

} // namespace maker
