// This game's profile: everything the launcher knows about the game it belongs to. The rest of launcher\ is the same
// for every game (recomp-tools' template copies it); game.h, game.cpp (the game's own texts) and palette.h (its
// colors) are each game's, with the art (launcher\art), the icon (res\icon.ico) and the version
// (res\version.h). The names and the options the game executable shares are src/project.h's.

#pragma once

#include <array>
#include <cstdint>
#include <vector>

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

// The title the art and the sidebar show: a small line above, the name below.
inline constexpr const char* kArtKicker = "GUITAR HERO";
inline constexpr const char* kArtTitle = "Metallica";
// The mark in the logo beside it (the sidebar, the first run): an icon's SVG paths in a 24-unit box, stroked as the
// launcher's icons are (ui.cpp). A guitar pick with a lightning bolt (the mockup's, local_docs/launcher_design).
inline constexpr const char* kBrandIcon =
    "<path d='M12 21c-4-4-8-9-8-13 0-3 4-5 8-5s8 2 8 5c0 4-4 9-8 13z'/><path d='M13 6l-3 5h4l-3 5'/>";
// The sidebar's name beside the logo, and the words before "LAUNCHER" under it ("METALLICA · LAUNCHER"); null: the
// art's title (kArtTitle) and "LAUNCHER" alone.
inline constexpr const char* kSidebarTitle = "Guitar Hero";
inline constexpr const char* kSidebarSubtitle = "Metallica";
// Where the Play page crops its art (launcher\art\hero.svg) in a window of another shape: the point kept in view,
// as fractions of the art's width and height. The guitar's head, near the top.
inline constexpr float kHeroFocusX = 0.5f;
inline constexpr float kHeroFocusY = 0.1f;
// The logo's mark on a disc instead of a rounded square.
inline constexpr bool kBrandRound = false;
// The first run's art panel: the game's name in a dark card over the art, with the tagline, instead of straight on
// the art with the tagline at its foot.
inline constexpr bool kInstallTitleCard = true;
// The Play page's name and buttons in a black bar across the foot of the art, under a line of accent (the first run's
// title card gets the same line), instead of straight on the art.
inline constexpr bool kHeroTitleBar = true;
// The titles in capitals, as the mockup draws them: the pages', the Play page's name, the sidebar's and the first
// run's.
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
// does not look for updates (a private repository), and its Updates page only shows the version. Once the repository
// is public: "https://api.github.com/repos/<user>/GHMetalRecomp/releases?per_page=20" and
// "https://github.com/<user>/GHMetalRecomp/releases".
inline constexpr const char* kReleasesApi = "";
inline constexpr const char* kReleasesPage = "";

// --- The game's own options (a cvar each; empty: the game has no such option, and the launcher no row) ---

// NativeXRenderer draws this game (src/native_x_renderer_ghmetallica.cpp): the Graphics page's renderer cards, its
// internal resolution, antialiasing and image format. False: the game is drawn by Xenos only, and the launcher says
// nothing of NativeXRenderer.
inline constexpr bool kHasNativeXRenderer = GAME_HAS_NATIVE_X_RENDERER;
// The renderer chosen until the player picks one (an options file without native_x_renderer): NativeXRenderer once a
// full play test has gone through it (as in POTCAWE, its D56), the Xenos path until then.
inline constexpr bool kNativeXRendererDefault = false;

// The Graphics page's shadow quality row: NativeXRenderer draws the game's shadow maps at a scale of their own
// (native_x_renderer_shadow_scale, its D92): at least twice or four times the game's resolution. Only where it
// finds them: the depth passes with no color target that are not of the game's output size.
// Turned on once the log shows NativeXRenderer finding the game's shadow maps ("shadow map: ...") with
// native_x_renderer_shadow_scale = 4, and their image is right.
inline constexpr bool kShadowQuality = kHasNativeXRenderer && false;

// The Graphics page's frame rate row: kFrameRateCvar's choices, each a value of the cvar and the frame rate it
// gives, and the value without the key. A boolean cvar (kFrameRateIsBool: the values are 0 and 1, as
// src/framerate.cpp's 60 FPS) or a number (the emulated vblank's rate, video_mode_refresh_rate, which some games
// follow). A rate above 60 is offered only where the monitor reaches it, and marked experimental.
struct FrameRate {
  int value;
  int fps;
};
// Here src/framerate.cpp's ghmetallica_frame_rate, the frame rate itself: 60 as on the console, above it the emulated
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

// The languages the game has: the Game page's language row, in this order. Empty: the game asks in its own menu,
// and the launcher has no row. Each is an XLanguage id (user_language: 1 English, 2 Japanese, 3 German, 4 French,
// 5 Spanish, 6 Italian, 7 Korean, 8 Traditional Chinese, 9 Portuguese, 10 Simplified Chinese, 11 Polish,
// 12 Russian) with its name in its own words (UTF-8; the launcher draws other scripts with Windows' fonts). A game
// with two versions of a language lists it twice, the second with a boolean cvar set when it is chosen (Kameo's
// Latin American Spanish). Until one is chosen, the Windows display language's: the entry whose `locales` (a
// locale name prefix, "es-ES") matches the user's locale best, or the first with that id.
struct Language {
  uint32_t id;
  const char* name;
  const char* variant_cvar;  // null: none
  const char* locales;       // null: any locale of the language
};
// The disc's text packs (DATA/COMPRESSED/PAK/qs_*.pak.xen): English and French, German, Italian, Spanish (Spain's);
// the game picks one from XGetLanguage at start (checked: Spanish's main menu, German's title screen).
inline constexpr std::array<Language, 5> kLanguages{{
    {3, "Deutsch", nullptr, nullptr},
    {1, "English", nullptr, nullptr},
    {5, "Español", nullptr, nullptr},
    {4, "Français", nullptr, nullptr},
    {6, "Italiano", nullptr, nullptr},
}};

// Switches of the game's own (src/), each a boolean cvar with a row in the Game page; their titles and hints are
// game.cpp's (GameTexts::toggles, in this order).
struct Toggle {
  const char* cvar;
  bool default_value;
};
inline constexpr std::array<Toggle, 0> kToggles{};

// The Settings group has a Game page (page_game.cpp) when the game has a language row or switches.
inline constexpr bool kHasGamePage = !kLanguages.empty() || !kToggles.empty();

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
// W A S D, F and K would move them with the frets.
struct KeyDefault {
  const char* cvar;
  const char* keys;
};
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
// Its title and hint are game.cpp's (GameTexts::keyboard_tap_title, keyboard_tap_hint).
inline constexpr const char* kKeyboardTapTarget = "dpad_down";

// --- The controls in the game's words ---

// A function of the game, as the Keyboard and Gamepad pages show it in place of the Xbox 360 control it is on (GH
// Metallica's mockup: "Green fret" where the launcher said "A button"): its chip, a square of a fret's color or a
// short text on grey, and its names.
struct GameControl {
  int bind;              // the Xbox 360 control: a Bind of settings.h
  const char* chip;      // the chip's text (UTF-8; an up or down arrow draws one), or "": color only
  uint32_t chip_color;   // 0xRRGGBB
  uint32_t chip_text;    // 0xRRGGBB
  const char* name;      // the function, in the launcher's language
  // The Gamepad page's value for a control the controllers cannot move (the D-pad, a stick): on the game's guitar
  // ("Whammy bar") and on any other controller ("Right stick"). On a guitar the moved buttons are named after the
  // function they press by default ("Green fret").
  const char* on_guitar = nullptr;
  const char* on_pad = nullptr;
};
struct GameControlGroup {
  const char* title;
  std::vector<GameControl> controls;
};
// The game's functions in groups, in the launcher's language (game.cpp); empty: the pages show the Xbox 360 controls
// (texts.cpp). The Keyboard page lists after them, under "Other controls", the ones no function is on.
const std::vector<GameControlGroup>& GameControlsFor(uint32_t language);

// --- Texts of the game's own (game.cpp) ---

struct GameTexts {
  // The first run's welcome, in the game's spirit; null: the launcher's own (texts.cpp).
  const char* welcome_title;
  const char* welcome_text;
  // kToggles' rows: a title and a hint each.
  std::array<const char*, kToggles.size() * 2> toggles;
  // The first run's last screen (the game installed), in the game's spirit; null: the launcher's own.
  const char* done_title = nullptr;
  const char* done_text = nullptr;
  // The DLC page: where the game shows what was installed; null: nothing said.
  const char* dlc_where = nullptr;
  // The first run's extraction screen (while it runs), in the game's spirit; null: the launcher's own.
  const char* extracting_title = nullptr;
  const char* extracting_text = nullptr;
  // The Gamepad page's name in the sidebar, its title and subtitle; null: the launcher's own ("Gamepad").
  const char* gamepad_nav = nullptr;
  const char* gamepad_title = nullptr;
  const char* gamepad_subtitle = nullptr;
  // The Keyboard and Gamepad pages' first column with GameControlsFor's functions ("Game function"); null: the
  // launcher's own (the Xbox 360 control).
  const char* functions_column = nullptr;
  // The Keyboard page's switch for kKeyboardTapTarget.
  const char* keyboard_tap_title = nullptr;
  const char* keyboard_tap_hint = nullptr;
};
// In the language of the launcher's texts (an XLanguage); missing ones fall back to English.
const GameTexts& GameTextsFor(uint32_t language);

}  // namespace launcher::game
