// ghmetallica - the shell build's screen (GHMETALLICA_SHELL, src/shell_init.h): the game's window with its title, in
// place of the game, which this build does not have.

#pragma once

#include <memory>

#include <rex/ui/imgui_dialog.h>

namespace ghmetallica {

std::unique_ptr<rex::ui::ImGuiDialog> CreateShellScreen(rex::ui::ImGuiDrawer* drawer);

}  // namespace ghmetallica
