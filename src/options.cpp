// ghmetallica - GH Metallica Launcher options (see options.h).

#include "options.h"

#include <rex/cvar.h>
#include <rex/filesystem.h>
#include <rex/logging.h>

#include <cstdlib>
#include <string>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

#include "project.h"
#ifdef __APPLE__
#include "macos_vulkan_driver.h"
#endif

#ifdef __APPLE__
namespace {
// Before main (and SDL_Init): fullscreen in its own Space (options.cpp, LoadLauncherOptions). SDL reads hints from
// the environment; one already set (by the player) is kept.
const bool g_fullscreen_spaces = [] { return setenv("SDL_VIDEO_MAC_FULLSCREEN_SPACES", "1", 0) == 0; }();
// CoreAudio's buffer: one console frame (256 samples, 5.3 ms) instead of SDL's default, so the device asks for sound
// in steps as small as the game makes it.
const bool g_audio_frames = [] { return setenv("SDL_AUDIO_DEVICE_SAMPLE_FRAMES", "256", 0) == 0; }();
}  // namespace
#endif

namespace ghmetallica {
namespace {

// The optional features (src/project.h): a cvar name, or "" when the game does not have the feature.
constexpr bool kHasFrameRate = sizeof(GAME_FRAME_RATE_CVAR) > 1;
constexpr bool kHasUltrawide = sizeof(GAME_ULTRAWIDE_CVAR) > 1;

}  // namespace

void LoadLauncherOptions(const std::filesystem::path& options_path,
                         const std::filesystem::path& gpu_plugin_path) {
#ifdef _WIN32
  // The render scale cvars (draw_resolution_scale_x/y, resolution_scale) live in the GPU DLL,
  // which the SDK loads later (SetupPresentation). Loading it now registers them, so the log line
  // below shows the effective values (otherwise LoadConfig keeps them pending and applies them
  // when they register). It stays loaded: LoadGpuPlugin opens the same DLL later.
  if (!LoadLibraryW(gpu_plugin_path.c_str())) {
    REXLOG_WARN("[options] Could not load {} ({})", gpu_plugin_path.string(), GetLastError());
  }
#endif

  // Loaded after ghmetallica.toml, so it takes precedence; the command line (for example Steam launch
  // options) wins over both.
  if (std::filesystem::exists(options_path)) {
    rex::cvar::LoadConfig(options_path);
  } else {
    REXLOG_INFO("[options] {} does not exist: using " GAME_CONFIG " (" GAME_LAUNCHER " creates it)",
                options_path.string());
  }

  // The frame rate (src/framerate.cpp): above 60 the emulated vertical blank runs at that rate (the SDK's
  // video_mode_refresh_rate, read when the graphics system starts, after this). At 60 it is left alone, so a
  // video_mode_refresh_rate given on its own still counts.
  if (kHasFrameRate) {
    if (int32_t frame_rate = rex::cvar::Query<int32_t>(GAME_FRAME_RATE_CVAR); frame_rate > 60) {
      rex::cvar::SetFlagByName("video_mode_refresh_rate", std::to_string(frame_rate));
    }
  }

#ifdef __APPLE__
  // macOS: MoltenVK reports what Metal lacks (primitive restart cannot be disabled) through the debug messenger at
  // every pipeline, thousands of lines a minute; logging them on the GPU thread costs frames. Off unless Vulkan
  // validation is on (vulkan_validation_enabled).
  if (!rex::cvar::Query<bool>("vulkan_validation_enabled")) {
    rex::cvar::SetFlagByName("vulkan_log_debug_messages", "false");
  }
  // Fullscreen as macOS has it: the game in a Space of its own (SDL's desktop fullscreen with
  // SDL_VIDEO_MAC_FULLSCREEN_SPACES, set below before SDL starts). An exclusive mode switch would cover the current
  // desktop instead; the Retina display is drawn at its own pixels either way.
  rex::cvar::SetFlagByName("fullscreen_exclusive", "false");
  macos::SelectVulkanDriver();
  // ghmetallica.toml's 4-frame (21 ms) queue is made for Windows' low-latency output; macOS's shared output takes
  // its sound in larger bursts and drained it about ten times a second (audio_keep_time's skips): 6 frames (32 ms).
  if (rex::cvar::Query<int32_t>("audio_maxqframes") < 6) {
    rex::cvar::SetFlagByName("audio_maxqframes", "6");
  }
#endif

  // NativeXRenderer draws through its own Vulkan device (DiligentCore), and the SDK's backend only keeps the
  // GPU in sync then. Its Vulkan backend keeps the process on one graphics API: no idle D3D12 device next
  // to NativeXRenderer's, and overlays such as RTSS report the API that presents. (The native_x_renderer cvar
  // exists once the game's NativeXRenderer side references the library; until then Query gives false.)
  if (rex::cvar::Query<bool>("native_x_renderer") && rex::cvar::Query<std::string>("gpu_backend") != "vulkan") {
    rex::cvar::SetFlagByName("gpu_backend", "vulkan");
    REXLOG_INFO("[options] gpu_backend set to vulkan: NativeXRenderer draws");
  }

  // Ultrawide on the Xenos path (src/widescreen.cpp): the cameras see with the window's aspect ratio, so the
  // presenter must stretch the frame over the window instead of showing it 16:9 with bars. The launcher
  // writes both; this covers the ultrawide cvar given alone (the command line). NativeXRenderer fills the
  // window itself (native_x_renderer_fill_screen).
  if (kHasUltrawide && rex::cvar::Query<bool>(GAME_ULTRAWIDE_CVAR) &&
      !rex::cvar::Query<bool>("native_x_renderer") && rex::cvar::Query<bool>("present_letterbox")) {
    rex::cvar::SetFlagByName("present_letterbox", "false");
    REXLOG_INFO("[options] present_letterbox set to false: " GAME_ULTRAWIDE_CVAR " stretches the image");
  }

  bool fullscreen = rex::cvar::Query<bool>("fullscreen");
  std::string features;
  if (kHasFrameRate) {
    features += std::string(" ") + GAME_FRAME_RATE_CVAR + "=" + std::to_string(rex::cvar::Query<int32_t>(GAME_FRAME_RATE_CVAR)) +
                " refresh=" + std::to_string(int(rex::cvar::Query<double>("video_mode_refresh_rate")));
  }
  if (kHasUltrawide) {
    features += std::string(" ") + GAME_ULTRAWIDE_CVAR + "=" + (rex::cvar::Query<bool>(GAME_ULTRAWIDE_CVAR) ? "true" : "false");
  }
  REXLOG_INFO("[options] monitor={} scale={}x{} display={} window={}x{} video_mode={}x{} letterbox={} "
              "gpu_backend={} anisotropic={} native_x_renderer={} language={}{}",
              rex::cvar::Query<int32_t>("monitor"),
              rex::cvar::Query<int32_t>("draw_resolution_scale_x"),
              rex::cvar::Query<int32_t>("draw_resolution_scale_y"),
              fullscreen ? (rex::cvar::Query<bool>("fullscreen_exclusive") ? "fullscreen" : "borderless")
                         : "window",
              rex::cvar::Query<int32_t>("window_width"), rex::cvar::Query<int32_t>("window_height"),
              rex::cvar::Query<int32_t>("video_mode_width"),
              rex::cvar::Query<int32_t>("video_mode_height"),
              rex::cvar::Query<bool>("present_letterbox"),
              rex::cvar::Query<std::string>("gpu_backend"), rex::cvar::Query<int32_t>("anisotropic_override"),
              rex::cvar::Query<bool>("native_x_renderer"),
              rex::cvar::Query<uint32_t>("user_language"), features);
}

std::filesystem::path GpuPluginPath() {
  std::filesystem::path exe_dir = rex::filesystem::GetExecutableFolder();
#if defined(_WIN32)
  const char* names[] = {"rexgpu-xenos.dll", "rexgpu-xenosrd.dll", "rexgpu-xenosd.dll"};
#elif defined(__APPLE__)
  const char* names[] = {"librexgpu-xenos.dylib", "librexgpu-xenosrd.dylib",
                         "librexgpu-xenosd.dylib"};
#else
  const char* names[] = {"librexgpu-xenos.so", "librexgpu-xenosrd.so", "librexgpu-xenosd.so"};
#endif
  for (const char* name : names) {
    if (std::filesystem::exists(exe_dir / name)) {
      return exe_dir / name;
    }
  }
  return exe_dir / names[0];
}

}  // namespace ghmetallica
