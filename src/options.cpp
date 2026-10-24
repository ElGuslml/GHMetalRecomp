// ghmetallica - GH Metallica Launcher options (see options.h).

#include "options.h"

#include <rex/cvar.h>
#include <rex/filesystem.h>
#include <rex/logging.h>

#include <string>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

#include "project.h"

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
    features += std::string(" ") + GAME_FRAME_RATE_CVAR + "=" + (rex::cvar::Query<bool>(GAME_FRAME_RATE_CVAR) ? "true" : "false");
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
