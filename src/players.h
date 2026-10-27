// ghmetallica - the profile follows the controller that starts the game (src/players.cpp).

#pragma once

namespace ghmetallica {

// Each frame, whether the title screen is shown (src/quit_prompt.cpp's hook): while it is, the controller that presses
// a button first becomes the profile's.
void SetTitleScreen(bool shown);

}  // namespace ghmetallica
