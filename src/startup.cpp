// ghmetallica - startup (see startup.h).

#include "startup.h"

#include <rex/cvar.h>
#include <rex/filesystem.h>
#include <rex/logging.h>
#include <rex/system/flags.h>

#include <algorithm>
#include <cwctype>
#include <array>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

#include "profile_store.h"
#include "project.h"

namespace ghmetallica {
namespace {

// Whether `folder` is inside the repository the build was made from (the parent of its assets\ folder): a
// development build run from out\build\. A copy anywhere else (a release package, an installation) is not, even
// on the machine that built it.
bool InsideRepository(const std::filesystem::path& folder, const std::filesystem::path& repository_assets) {
  std::error_code ec;
  auto normal = [&ec](const std::filesystem::path& p) {
    std::wstring s = std::filesystem::weakly_canonical(p, ec).generic_wstring();
    std::transform(s.begin(), s.end(), s.begin(), [](wchar_t c) { return wchar_t(std::towlower(c)); });
    if (!s.empty() && s.back() != L'/') s += L'/';
    return s;
  };
  std::wstring root = normal(repository_assets.parent_path());
  std::wstring inside = normal(folder);
  return root.size() > 1 && inside.compare(0, root.size(), root) == 0;
}

// CRC32 of the default.xex this code was recompiled from (10,366,976 bytes, title ID 41560830, version 0.0.0.1,
// media ID 2E0C8E5F).
constexpr uint32_t kGameExecutableCrc32 = 0xA7C93B87;

// The GPU settings KameoRecomp, GRAWRecomp and POTCAWERecomp share, none shown to be needed by this game yet:
// when something draws wrong, try the SDK default of each one from the command line first. Each comment says what
// the setting does and what the other value gives back.
constexpr const char kDefaultConfig[] =
    R"(# GHMetallicaRecomp.exe runtime configuration, loaded at startup. The game recreates this file with these
# defaults if it is missing, and adds the settings an existing file lacks. GH Metallica Launcher saves its choices to
# ghmetallica_options.toml, which is loaded after this file; command line options (--name=value) win over both.
log_level = "info"
# Do not wait at the end of the frame for new D3D12 pipelines (a stutter the first time an effect
# appears): the object appears a few frames later. false = wait.
gpu_async_pipelines_no_frame_wait = true
# Translate the shaders' control flow with if blocks instead of a switch. In POTCAWERecomp the
# switch (the SDK's default) made the pixel shader body re-execute under GPU load and lit objects
# turned into flat saturated colors. true = the SDK default.
dxbc_switch = false
# Clears drawn through a surface with another MSAA mode or pitch than the scene: without these
# settings the SDK copies the render target into that alias and back around each clear. Run the
# vertex shader of unclipped draws on the CPU to know how much EDRAM they use (the Xenia default;
# the SDK default is false)...
execute_unclipped_draw_vs_on_cpu = true
# ...and skip the copy into a render target when the draw replaces every sample of it.
render_target_skip_transfers_on_overwrite = true
# Write constant full-coverage draws (clears) straight into the render targets that hold that
# memory. Off: in KameoRecomp it removed all the geometry from a cinematic (a bug not found yet).
render_target_draw_clears = false
# Keep the GPU copies of memory the CPU uploaded valid across frames. With the SDK default (true)
# every page uploaded by the CPU is re-uploaded the next frame even if it did not change. false is
# the original Xenia behavior (true is a hack for one game).
clear_memory_page_state = false
# Anisotropic filtering for textures with mipmaps, sharper at oblique angles: 5 = 16x, 4 = 8x,
# 3 = 4x (the SDK default), 2 = 2x, 0 = off, -1 = only what the game asks. GH Metallica Launcher's
# "Anisotropic filtering" row sets it.
anisotropic_override = 5
# Call the game's audio callback at least 3 ms apart (the console calls it every 5.3 ms), not in the
# bursts of two the host device frees frames in. FMOD's mixer polls how far its two-block ring has
# been read and missed most blocks: from the title screen on, the music played as noise. 0 = the SDK
# default (no spacing).
audio_callback_min_interval_us = 3000
# Audio frames queued for the sound device (5.3 ms each): 4 is the least the SDK allows, 21 ms less
# delay between a strum and its sound than the SDK's 8. With the calls spaced the queue stays full
# (no gap in a whole song, measured); the game's calibration covers what is left. 8 = the SDK default.
audio_maxqframes = 4
# The SDK's audio threads (its audio worker runs the game's sound callback and, with ghmetallica_fmod_on_audio_thread,
# FMOD's mixing) in Windows' multimedia class "Pro Audio", as the sound device's thread: a busy PC holds them up less.
# false = normal priority (the SDK default).
audio_thread_priority = true
# When the game's sound still comes late (its audio thread held up longer than the 21 ms queue), the silence played
# meanwhile is made up by dropping as many of its next frames: a short skip, and the music stays with the notes (the
# SDK default, false, plays the rest of the song that much late, until a pause).
audio_keep_time = true
# V-Sync (GH Metallica Launcher's row writes both; docs/vsync.md): NativeXRenderer presents with FIFO, each frame
# shown once and never torn, and the game's vertical blank follows the monitor's where the frame rate is its refresh
# rate or a whole fraction of it, so the game never runs ahead of the display. Measured at 160 FPS on a 160 Hz
# display: 4 ms from present to display, no frame dropped (MAILBOX with the SDK's timer: 16-19 ms, NVIDIA queuing
# three frames). "immediate" and false = V-Sync off (tearing, or a G-Sync / FreeSync display following the game).
native_x_renderer_present_mode = "vsync"
vblank_follows_display = true
# Report a controller (and the keyboard's emulated one) as a guitar: the game gives a plain gamepad
# the vocals, which need a microphone. Frets on A (green), B (red), Y (yellow), X (blue) and LB
# (orange), strumming on the D-pad, the whammy bar on the right stick, Star Power on Back. A real
# guitar or drum kit keeps its own kind. 8 = drum kit, 0 = the SDK default (a gamepad).
gamepad_subtype = 6
# An Xbox 360 guitar reads its whammy bar half pressed until the bar first moves: at rest until then.
guitar_whammy_rest = true
# A controller or the keyboard playing guitar: the whammy bar at rest while the right stick is centred (or its keys
# are up), and pressed as far as the stick goes from the centre, on either side. The stick's own value, 0 at rest,
# is the bar held about half way for the game (its bar rests at -0.76 of the axis).
gamepad_whammy_rest = true
# A PC microphone 12 dB quieter than Windows records it (usb_microphone_1, off until GH Metallica
# Launcher's Microphone chooses one): the game turns its own USB microphone down to the bottom of its
# range for a song; a PC microphone at full level also hears the room and the speakers (the first test,
# a condenser microphone at 0 dB: the pitch line jumped). The launcher's Gain row changes it.
usb_microphone_1_gain = -12
usb_microphone_2_gain = -12
# Start pressed on a second controller: the game asks that controller to sign in, and only the first
# has a profile. The sign-in UI closes as when the player cancels it, and the game shows its "not
# signed in" message (sign in / continue without saving); the SDK default reported a sign-in change
# and the game waited for that controller's profile forever on an empty screen.
signin_ui_cancel = true
# The other controllers are the profile's guests, as on the console (Name (1)): signed in, so the game
# does not ask them to sign in, with no saves or settings of their own. The controller that presses a
# button at the title screen is the profile's (ghmetallica_profile_follows_press, src/players.cpp).
signin_guests = true
# The keyboard as a guitar (with mnk_mode; GH Metallica Launcher's Keyboard page changes them): the frets
# on D, F, J, K and L (green, red, yellow, blue, orange: A, B, Y, X, LB), strumming on the up and down
# arrows (the D-pad), Star Power on Space or Backspace (Back), Start on Enter or X, the whammy bar on the
# right arrow (the right stick). The sticks' other keys are off: W A S D, F and K would move them with the
# frets.
keybind_a = "D"
keybind_b = "F"
keybind_y = "J"
keybind_x = "K"
keybind_left_shoulder = "L"
keybind_dpad_up = "Up"
keybind_dpad_down = "Down"
keybind_back = "Space,Backspace"
keybind_start = "Return,X"
keybind_rstick_right = "Right"
keybind_rstick_up = ""
keybind_rstick_down = ""
keybind_rstick_left = ""
keybind_rstick_press = ""
keybind_lstick_up = ""
keybind_lstick_down = ""
keybind_lstick_left = ""
keybind_lstick_right = ""
keybind_lstick_press = ""
# The keyboard's frets strum as they are pressed with mnk_tap_target = "dpad_down" (the launcher's switch),
# frets pressed within mnk_tap_window_ms of each other making one chord: one strum. Empty: off, the
# arrows strum.
mnk_tap_sources = "a,b,y,x,lb"
mnk_tap_target = ""
# The same for a controller playing guitar (its frets A, B, Y, X and LB) with gamepad_tap_target = "dpad_down" (the
# launcher's switch on the controller's page), only in a song as the keyboard's (src/keyboard.cpp's gate).
gamepad_tap_sources = "a,b,y,x,lb"
gamepad_tap_target = ""
# The game's screen effects over the 3D image (src/screen_fx.cpp; the note highway and the HUD are drawn after
# them): false turns one off. Bloom: the glow around lights. Depth of field: the background blurred in some camera
# shots and behind the menus and the pause menu. Grain: the film grain. Color: the color correction (each venue's
# look, fades to black, the image greyed while the rock meter is red); with it off the grain is off too (without the
# color pass it is far stronger). Vignette: darkened corners, where a venue turns it on. Ink: the ink blotches a song
# adds. true = as on the console.
ghmetallica_fx_bloom = true
ghmetallica_fx_depth_of_field = true
ghmetallica_fx_grain = true
ghmetallica_fx_color = true
ghmetallica_fx_vignette = true
ghmetallica_fx_ink = true
)";

uint32_t Crc32(const std::vector<char>& data) {
  static const std::array<uint32_t, 256> table = [] {
    std::array<uint32_t, 256> t{};
    for (uint32_t i = 0; i < 256; ++i) {
      uint32_t c = i;
      for (int k = 0; k < 8; ++k) {
        c = (c & 1) ? 0xEDB88320u ^ (c >> 1) : c >> 1;
      }
      t[i] = c;
    }
    return t;
  }();
  uint32_t crc = 0xFFFFFFFFu;
  for (char byte : data) {
    crc = table[(crc ^ uint8_t(byte)) & 0xFF] ^ (crc >> 8);
  }
  return crc ^ 0xFFFFFFFFu;
}

#ifdef _WIN32
// MB_* flags; returns the button pressed.
int ShowMessage(const std::wstring& text, UINT flags) {
  return MessageBoxW(nullptr, text.c_str(), L"" GAME_TITLE, flags);
}

std::wstring Hex32(uint32_t value) {
  wchar_t buffer[16];
  swprintf(buffer, 16, L"%08X", value);
  return buffer;
}
#endif

}  // namespace

std::filesystem::path DefaultGameRoot() {
  std::filesystem::path next_to_exe = rex::filesystem::GetExecutableFolder() / "assets";
  if (std::filesystem::exists(next_to_exe / GAME_XEX)) {
    return next_to_exe;
  }
  // The repository's assets\, for a development build run from the repository only: a package or an installation
  // uses its own (the launcher extracts the game there), even on the machine that built it.
  std::filesystem::path repository = GHMETALLICA_DEFAULT_GAME_ROOT;
  if (InsideRepository(rex::filesystem::GetExecutableFolder(), repository) &&
      std::filesystem::exists(repository / GAME_XEX)) {
    return repository;
  }
  // Neither has the game: report the folder next to the executable (the release layout), not a path
  // of the machine the build was made on.
  return next_to_exe;
}

void ConfigureUserData(std::filesystem::path& user_data_root, std::filesystem::path& cache_root) {
  if (!rex::cvar::Query<std::string>("user_data_root").empty()) {
    return;  // chosen on the command line
  }
  std::filesystem::path portable = rex::filesystem::GetExecutableFolder() / "userdata";
  std::error_code ec;
  std::filesystem::create_directories(portable, ec);
  // Writable? Logging is not set up yet: the folder in use is logged later
  // (GhmetallicaApp::SetupEnvironment).
  std::filesystem::path probe = portable / ".write_test";
  {
    std::ofstream f(probe, std::ios::binary);
    if (!f) {
      return;
    }
  }
  std::filesystem::remove(probe, ec);
  user_data_root = portable;
  if (rex::cvar::Query<std::string>("cache_root").empty()) {
    cache_root = portable / "cache";
  }
}

void ConfigureProfile(const std::filesystem::path& user_data_root) {
  if (!REXCVAR_GET(profile_root).empty()) {
    return;  // chosen on the command line
  }
  if (std::optional<launcher::profiles::GameProfile> profile = launcher::profiles::ForGame(user_data_root)) {
    std::u8string folder = profile->folder.u8string();
    REXCVAR_SET(profile_root, std::string(folder.begin(), folder.end()));
    if (REXCVAR_GET(profile_name).empty()) {
      REXCVAR_SET(profile_name, profile->name);
    }
  }
}

// Creates ghmetallica.toml with the defaults, or appends to an existing one the settings (with their
// comments) it does not mention, so that a setting added in a newer build takes effect without
// deleting the user's file. A setting the file has, with any value, is left alone.
void EnsureDefaultConfig(const std::filesystem::path& config_path) {
  // Logging is not initialized yet (the config sets it up): nothing to log here.
  std::string existing;
  if (std::filesystem::exists(config_path)) {
    std::ifstream in(config_path, std::ios::binary);
    existing.assign((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
  } else {
    std::ofstream f(config_path, std::ios::binary);
    f << kDefaultConfig;
    return;
  }
  auto has_key = [&existing](const std::string& key) {
    size_t pos = 0;
    while ((pos = existing.find(key, pos)) != std::string::npos) {
      size_t line_start = existing.rfind('\n', pos);
      line_start = line_start == std::string::npos ? 0 : line_start + 1;
      size_t after = pos + key.size();
      bool only_spaces_before = existing.find_first_not_of(" \t", line_start) == pos;
      size_t next = existing.find_first_not_of(" \t", after);
      if (only_spaces_before && next != std::string::npos && existing[next] == '=') {
        return true;
      }
      pos = after;
    }
    return false;
  };
  std::string missing;
  std::string comments;
  std::istringstream defaults(kDefaultConfig);
  std::string line;
  while (std::getline(defaults, line)) {
    if (line.empty()) {
      comments.clear();
      continue;
    }
    if (line[0] == '#') {
      comments += line + '\n';
      continue;
    }
    size_t eq = line.find('=');
    if (eq == std::string::npos) {
      comments.clear();
      continue;
    }
    std::string key = line.substr(0, eq);
    key.erase(key.find_last_not_of(" \t") + 1);
    if (!has_key(key)) {
      missing += comments + line + '\n';
    }
    comments.clear();
  }
  if (missing.empty()) {
    return;
  }
  std::ofstream f(config_path, std::ios::binary | std::ios::app);
  if (!existing.empty() && existing.back() != '\n') {
    f << '\n';
  }
  f << missing;
}

bool CheckGameFiles(const std::filesystem::path& game_root) {
  std::filesystem::path exe = game_root / GAME_XEX;
  std::ifstream f(exe, std::ios::binary);
  if (!f) {
    REXLOG_ERROR("[startup] Game files not found: {} does not exist", exe.string());
#ifdef _WIN32
    ShowMessage(L"Game files not found.\n\n"
                L"Copy the files of your " GAME_TITLE " (Xbox 360) disc into:\n" +
                    game_root.wstring() + L"\n\nso that this file exists:\n" + exe.wstring(),
                MB_OK | MB_ICONERROR);
#endif
    return false;
  }
  std::vector<char> data((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
  uint32_t crc = Crc32(data);
  if (crc == kGameExecutableCrc32) {
    REXLOG_INFO("[startup] {}: CRC32 {:08X} (expected version)", exe.string(), crc);
    return true;
  }
  REXLOG_WARN("[startup] {}: CRC32 {:08X}, expected {:08X}: a different version of the game",
              exe.string(), crc, kGameExecutableCrc32);
#ifdef _WIN32
  int answer = ShowMessage(L"" GAME_XEX " is not the version this build was recompiled from "
                           L"(CRC32 " + Hex32(crc) + L", expected " + Hex32(kGameExecutableCrc32) +
                               L").\nThe game will most likely crash.\n\nStart anyway?",
                           MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2);
  return answer == IDYES;
#else
  return true;
#endif
}

}  // namespace ghmetallica
