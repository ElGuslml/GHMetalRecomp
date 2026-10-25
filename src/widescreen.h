// ghmetallica - ultrawide displays.
//
// Anamorphic rendering: the game's 3D views see with the window's aspect ratio (e.g. 3440x1440) but still draw into
// its 1280x720 image, its 2D layer narrowed to 16:9 in the middle of it; the SDK presenter stretches it over the whole
// window (present_letterbox = false). Enabled with the ghmetallica_ultrawide cvar; with NativeXRenderer, which renders
// at the screen's aspect ratio itself, by its native_x_renderer_fill_screen.

#pragma once

#include <cstdint>

namespace ghmetallica {

// Size of the game window in pixels (from ReXApp::OnWindowPixelSizeChanged).
void SetOutputSize(uint32_t width, uint32_t height);

}  // namespace ghmetallica
