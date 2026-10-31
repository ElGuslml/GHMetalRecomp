// ghmetallica - the shell build's screen (src/shell_screen.h).

#include "shell_screen.h"

#include <imgui.h>

#include <algorithm>
#include <cfloat>

#include "project.h"

namespace ghmetallica {
namespace {

class ShellScreen : public rex::ui::ImGuiDialog {
 public:
  explicit ShellScreen(rex::ui::ImGuiDrawer* drawer) : ImGuiDialog(drawer) {}

 protected:
  void OnDraw(ImGuiIO& io) override {
    const ImVec2 size = io.DisplaySize;
    ImDrawList* draw = ImGui::GetBackgroundDrawList();
    draw->AddRectFilled(ImVec2(0, 0), size, IM_COL32(12, 12, 14, 255));
    const float scale = std::max(1.0f, size.y / 540.0f);
    auto centered = [&](const char* text, float font_scale, float y, ImU32 color) {
      ImFont* font = ImGui::GetFont();
      const float font_size = ImGui::GetFontSize() * font_scale;
      const ImVec2 extent = font->CalcTextSizeA(font_size, FLT_MAX, 0.0f, text);
      draw->AddText(font, font_size, ImVec2((size.x - extent.x) * 0.5f, y), color, text);
    };
    centered(GAME_TITLE, 2.4f * scale, size.y * 0.42f, IM_COL32(197, 220, 58, 255));
    centered("Game files installed. Gameplay is not available in this preview build yet.", 1.1f * scale,
             size.y * 0.42f + 48.0f * scale, IM_COL32(200, 200, 200, 255));
  }
};

}  // namespace

std::unique_ptr<rex::ui::ImGuiDialog> CreateShellScreen(rex::ui::ImGuiDrawer* drawer) {
  return std::make_unique<ShellScreen>(drawer);
}

}  // namespace ghmetallica
