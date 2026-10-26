// ghmetallica - the keyboard's strum on each fret, only in a song (src/keyboard.cpp).

#pragma once

namespace ghmetallica {

// Before the guest starts: the SDK's keyboard strums (mnk_tap_target) only while a song runs.
void InitializeKeyboard();

}  // namespace ghmetallica
