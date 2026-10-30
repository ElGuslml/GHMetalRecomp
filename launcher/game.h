// This game's profile: everything the launcher knows about the game it belongs to. The launcher's code is the same for
// every game (the RecompLauncher submodule, recomp_launcher\, with game_types.h for the types below); game.h, game.cpp
// (the game's own texts) and palette.h (its colors) are each game's, with the art (launcher\art), the icon
// (res\icon.ico), the resource script (res\launcher.rc) and the version (res\version.h). The names and the options the
// game executable shares are src/project.h's.

#pragma once

#include <array>
#include <cstdint>
#include <vector>

#include "game_types.h"
#include "project.h"
#include "version.h"

namespace launcher::game {

// --- Names ---

// The game, as the texts name it ("the .iso file of %s").
inline constexpr const char* kName = GAME_TITLE;
// The recompiled executable and the launcher, side by side; the launcher's name is its window's title.
inline constexpr const char* kRecompExecutable = GAME_EXE;
inline constexpr const char* kLauncherExecutable = GAME_LAUNCHER ".exe";
inline constexpr const char* kLauncherName = GAME_LAUNCHER;
// The game's settings next to it: its base file (src/startup.cpp writes the defaults) and what the launcher saves.
inline constexpr const char* kConfigFile = GAME_CONFIG;
inline constexpr const char* kOptionsFile = GAME_OPTIONS;
// The game's user data when its folder cannot be written: Documents\<this> (the SDK's default, the app's name).
inline constexpr const char* kDocumentsFolder = "ghmetallica";
// res\version.h.
inline constexpr const char* kVersion = RECOMP_VERSION_STR;

// --- The look ---

// The title the Play page shows over the art: a small line above, the name below.
inline constexpr const char* kArtKicker = "GUITAR HERO";
inline constexpr const char* kArtTitle = "Metallica";
// The mark in the header's logo while the game's icon is unknown: an icon's SVG paths in a 24-unit box, stroked as the
// launcher's icons are (ui.cpp). A guitar pick with a lightning bolt (the mockup's, local_docs/launcher_design).
inline constexpr const char* kBrandIcon =
    "<path d='M12 21c-4-4-8-9-8-13 0-3 4-5 8-5s8 2 8 5c0 4-4 9-8 13z'/><path d='M13 6l-3 5h4l-3 5'/>";
// The header's name beside the logo, on two lines (in capitals).
inline constexpr const char* kBrandTitle = "Guitar Hero";
inline constexpr const char* kBrandSubtitle = "Metallica";
// Where the art under the pages (launcher\art\background.svg, 1280 x 760) is cropped in a window of another shape: the
// point kept in view, as fractions of its width and height. The pile of skulls, at the bottom right.
inline constexpr float kArtFocusX = 0.8f;
inline constexpr float kArtFocusY = 1.0f;
// The logo's mark on a disc instead of a rounded square.
inline constexpr bool kBrandRound = false;
// The titles in capitals, as the mockup draws them (the game's own texts that the launcher shows as titles).
inline constexpr bool kUppercaseTitles = true;

// --- The disc ---

// The disc's executable, at its root and in assets\ once extracted; extracted last, so that its presence marks a
// complete installation.
inline constexpr const char* kDiscExecutable = GAME_XEX;
// The disc this build was recompiled from (src/startup.cpp checks the same CRC at the game's start).
inline constexpr uint32_t kTitleId = 0x41560830;
inline constexpr uint32_t kExecutableCrc32 = 0xA7C93B87;
inline constexpr const char* kRegion = "USA";
// The game's files on that disc (1470 files): the space the first run asks for before an ISO is chosen.
inline constexpr uint64_t kInstalledBytes = 7220319995;
// The game's frame: the Xenos path's scales and NativeXRenderer's internal resolutions are multiples of it.
inline constexpr int kFrameWidth = GAME_FRAME_WIDTH;
inline constexpr int kFrameHeight = GAME_FRAME_HEIGHT;

// --- Updates ---

// This project's releases (updater.cpp): GitHub's API for them, newest first, and their page. Empty: the launcher
// does not look for updates, and its Updates page only shows the version. Set from the first release on, while the
// repository is still private (the check fails until it is public, and the releases made then are found).
inline constexpr const char* kReleasesApi = "https://api.github.com/repos/ElGuslml/GHMetalRecomp/releases?per_page=20";
inline constexpr const char* kReleasesPage = "https://github.com/ElGuslml/GHMetalRecomp/releases";

// --- The game's own options (a cvar each; empty: the game has no such option, and the launcher no row) ---

// NativeXRenderer draws this game (src/native_x_renderer_ghmetallica.cpp): the Graphics page's renderer cards, its
// internal resolution, antialiasing and image format. False: the game is drawn by Xenos only, and the launcher says
// nothing of NativeXRenderer.
inline constexpr bool kHasNativeXRenderer = GAME_HAS_NATIVE_X_RENDERER;
// The renderer chosen until the player picks one (an options file without native_x_renderer): NativeXRenderer, as in
// Kameo, since Gustavo's play at 3440x1440 up to 160 FPS went through it (the first release, 0.1.0).
inline constexpr bool kNativeXRendererDefault = true;

// The Graphics page's shadow quality row: NativeXRenderer draws the game's shadow maps at a scale of their own
// (native_x_renderer_shadow_scale, its D92): at least twice or four times the game's resolution. Only where it
// finds them: the depth passes with no color target that are not of the game's output size.
// Turned on once the log shows NativeXRenderer finding the game's shadow maps ("shadow map: ...") with
// native_x_renderer_shadow_scale = 4, and their image is right.
inline constexpr bool kShadowQuality = kHasNativeXRenderer && false;

// The Graphics page's frame rate row: kFrameRateCvar's choices (game_types.h's FrameRate) and the value without the
// key. Here src/framerate.cpp's ghmetallica_frame_rate, the frame rate itself: 60 as on the console, above it the emulated
// vblank's rate (src/options.cpp), which the game follows at its own speed.
inline constexpr const char* kFrameRateCvar = GAME_FRAME_RATE_CVAR;
inline constexpr bool kFrameRateIsBool = false;
inline constexpr std::array<FrameRate, 6> kFrameRates{{{60, 60}, {120, 120}, {144, 144}, {160, 160}, {165, 165}, {240, 240}}};
inline constexpr int kFrameRateDefault = 60;

// The Xenos path's MSAA turned off (src/msaa.cpp): the Graphics page's antialiasing row with Xenos, the game's own
// antialiasing (its label, "2× MSAA") or none.
inline constexpr const char* kNoMsaaCvar = "";
inline constexpr const char* kMsaaLabel = "";

// The game's cameras widened for a screen wider than 16:9 (src/widescreen.cpp), set with the Video page's image
// format; the Xenos path then stretches its image over the screen (present_letterbox = false). Empty: the format
// applies to NativeXRenderer only.
inline constexpr const char* kUltrawideCvar = GAME_ULTRAWIDE_CVAR;

// The languages the game has (game_types.h's Language): the Game page's language row, in this order. Empty: the game
// asks in its own menu, and the launcher has no row.
// The disc's text packs (DATA/COMPRESSED/PAK/qs_*.pak.xen): English and French, German, Italian, Spanish (Spain's);
// the game picks one from XGetLanguage at start (checked: Spanish's main menu, German's title screen).
inline constexpr std::array<Language, 5> kLanguages{{
    {3, "Deutsch", nullptr, nullptr},
    {1, "English", nullptr, nullptr},
    {5, "Español", nullptr, nullptr},
    {4, "Français", nullptr, nullptr},
    {6, "Italiano", nullptr, nullptr},
}};

// Switches of the game's own (src/), each a boolean cvar with a row in the Settings tab (game_types.h's Toggle); their
// titles, hints and help are game.cpp's (GameTexts::toggles and toggle_help, in this order).
// The screen effects (src/screen_fx.cpp, docs/screen-effects.md), all on as on the console: the grain only with the
// color correction (without that pass the grain is far stronger, Gustavo's rule).
inline constexpr std::array<Toggle, 6> kToggles{{
    {"ghmetallica_fx_bloom", true, ToggleGroup::kScreenEffects},
    {"ghmetallica_fx_depth_of_field", true, ToggleGroup::kScreenEffects},
    {"ghmetallica_fx_color", true, ToggleGroup::kScreenEffects},
    {"ghmetallica_fx_grain", true, ToggleGroup::kScreenEffects, "ghmetallica_fx_color"},
    {"ghmetallica_fx_vignette", true, ToggleGroup::kScreenEffects},
    {"ghmetallica_fx_ink", true, ToggleGroup::kScreenEffects},
}};

// Whether some switch is in `group`.
constexpr bool HasToggles(ToggleGroup group) {
  for (const Toggle& toggle : kToggles) {
    if (toggle.group == group) return true;
  }
  return false;
}

// The Settings tab has a Game category (page_settings.cpp) when the game has a language row or switches of its own.
inline constexpr bool kHasGamePage = !kLanguages.empty() || HasToggles(ToggleGroup::kGame);

// --- Downloadable content ---

// The DLC page (page_dlc.cpp, dlc.h): Xbox 360 content packages, each installed by the game's --install_content tool
// mode into its user data's content folder, where its system software lists them. On when the game takes
// downloadable content: checked with Guitar Hero III's "Death Magnetic" album (title 415607F7), whose 10 songs the
// game lists in Quickplay (docs/dlc.md).
inline constexpr bool kHasDlcPage = true;

// --- The keyboard ---

// The Keyboard page's defaults where they are not the SDK's (its "Reset to defaults", and a control the files do not
// set): keybind_* cvars and their keys, as src/startup.cpp writes them for the game. The keyboard as a guitar
// (Gustavo's frets): D, F, J, K, L; the arrows strum; Space or Backspace Star Power; the sticks' other keys off, as
// W A S D, F and K would move them with the frets. Empty: the SDK's (game_types.h's KeyDefault).
inline constexpr std::array<KeyDefault, 19> kKeyboardDefaults{{
    {"keybind_a", "D"},
    {"keybind_b", "F"},
    {"keybind_y", "J"},
    {"keybind_x", "K"},
    {"keybind_left_shoulder", "L"},
    {"keybind_dpad_up", "Up"},
    {"keybind_dpad_down", "Down"},
    {"keybind_back", "Space,Backspace"},
    {"keybind_start", "Return,X"},
    {"keybind_rstick_right", "Right"},
    {"keybind_rstick_up", ""},
    {"keybind_rstick_down", ""},
    {"keybind_rstick_left", ""},
    {"keybind_rstick_press", ""},
    {"keybind_lstick_up", ""},
    {"keybind_lstick_down", ""},
    {"keybind_lstick_left", ""},
    {"keybind_lstick_right", ""},
    {"keybind_lstick_press", ""},
}};
// The Keyboard page's switch that makes the keyboard's frets strum as they are pressed (the SDK's mnk_tap_target with
// src/startup.cpp's mnk_tap_sources, the frets): the control it presses when on, the D-pad (strumming); "": no switch.
// The same on each controller's page for the controllers playing guitar (gamepad_tap_target, gamepad_tap_sources).
// Its title and hint are game.cpp's (GameTexts::keyboard_tap_title, keyboard_tap_hint).
inline constexpr const char* kKeyboardTapTarget = "dpad_down";
// The whammy bar's row on each controller playing guitar (page_controls.cpp): how far the game reads the bar, and its
// calibration (the SDK's gamepad_whammy_calibration, recomp change 60: the axis it is on, at rest and all the way).
inline constexpr bool kHasWhammy = true;

// --- Singing ---

// The Controls tab's microphone and each controller's part (page_controls.cpp): a recording device of the PC as each of
// the game's two USB microphones (the SDK's usb_microphone_1 and _2, recomp change 55), and a controller playing the
// game's instrument (gamepad_subtype's guitar, src/startup.cpp) or singing (a plain gamepad, XInput subtype 1, which the
// game gives the vocals; the SDK's gamepad_device_subtypes, change 56). Real guitars and drum kits keep their own.
// Their names are GameTexts::part_instrument and part_vocals.
inline constexpr bool kHasMicrophones = true;
// The microphone's gain (usb_microphone_1_gain) until the player changes it, and the Microphone's Reset: src/startup.cpp's.
inline constexpr int kMicrophoneGainDefault = -12;

// --- The controls in the game's words ---

// The game's functions (game_types.h's GameControl) in groups, in the launcher's language (game.cpp); empty: the pages show the Xbox 360 controls
// (texts.cpp). The Keyboard page lists after them, under "Other controls", the ones no function is on.
const std::vector<GameControlGroup>& GameControlsFor(uint32_t language);

// --- Texts of the game's own (game.cpp) ---

struct GameTexts {
  // The first run's welcome, in the game's spirit; null: the launcher's own (texts.cpp).
  const char* welcome_title;
  const char* welcome_text;
  // kToggles' rows: a title and a hint each.
  std::array<const char*, kToggles.size() * 2> toggles;
  // kToggles' help, the Settings tab's panel for the row in focus; null: the hint.
  std::array<const char*, kToggles.size()> toggle_help{};
  // The first run's last screen (the game installed), in the game's spirit; null: the launcher's own.
  const char* done_title = nullptr;
  const char* done_text = nullptr;
  // The DLC page: where the game shows what was installed; null: nothing said.
  const char* dlc_where = nullptr;
  // The first run's extraction screen (while it runs), in the game's spirit; null: the launcher's own.
  const char* extracting_title = nullptr;
  const char* extracting_text = nullptr;
  // The Controls tab's keyboard switch for kKeyboardTapTarget, and its tip in the help.
  const char* keyboard_tap_title = nullptr;
  const char* keyboard_tap_hint = nullptr;
  // kHasMicrophones: a controller's two parts, the game's instrument and the singer's; null: the launcher's words.
  const char* part_instrument = nullptr;
  const char* part_vocals = nullptr;
};
// In the language of the launcher's texts (an XLanguage); missing ones fall back to English.
const GameTexts& GameTextsFor(uint32_t language);

}  // namespace launcher::game
