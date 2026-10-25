// ghmetallica - the names, the frame size and the optional features the game executable (src/) and GH Metallica Launcher
// (launcher/, through launcher/game.h) share. Plain macros: the launcher is a separate program without the SDK's
// runtime, and the game uses them in string literals (L"" GAME_TITLE).

#pragma once

#define GAME_TITLE "Guitar Hero: Metallica"
#define GAME_EXE "GHMetallicaRecomp.exe"
#define GAME_LAUNCHER "GH Metallica Launcher"
// The game's executable in the game files (assets/): the code is recompiled from it.
#define GAME_XEX "default.xex"
// Next to GAME_EXE: the base settings (src/startup.cpp writes the defaults) and what the launcher saves.
#define GAME_CONFIG "ghmetallica.toml"
#define GAME_OPTIONS "ghmetallica_options.toml"

// The game's frame: the launcher's render scales and internal resolutions are multiples of it. Most Xbox 360
// games draw 1280x720; check it in a RenderDoc capture or the SDK's log before relying on it.
#define GAME_FRAME_WIDTH 1280
#define GAME_FRAME_HEIGHT 720

// The launcher rows and the options a game turns on once it has the feature (as KameoRecomp, GRAWRecomp and
// POTCAWERecomp did, in this order):
// - Frame rate: the cvar of src/framerate.cpp's option, here the frame rate itself (60 to 240, the emulated vblank's
//   rate above 60); "" = no row.
#define GAME_FRAME_RATE_CVAR "ghmetallica_frame_rate"
// - Ultrawide: the cvar of src/widescreen.cpp's option, the game's cameras widened for a screen wider than 16:9
//   (for example "ghmetallica_ultrawide"); "" = no row. On the Xenos path the image is then stretched over the screen
//   (present_letterbox = false, src/options.cpp).
#define GAME_ULTRAWIDE_CVAR "ghmetallica_ultrawide"
// - NativeXRenderer: 1 once src/native_x_renderer_ghmetallica.cpp (hooks and profile) is in and the observer matches
//   (native_x_renderer/docs/integrating-a-game.md): the "Renderer" row, its internal resolution and antialiasing.
#define GAME_HAS_NATIVE_X_RENDERER 1
