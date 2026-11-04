// ghmetallica - GH Metallica Launcher options (see options.h).

#include "options.h"

#include <rex/cvar.h>
#include <rex/filesystem.h>
#include <rex/logging.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cwchar>
#include <string>
#include <vector>

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

// A refresh rate for the log: whole, or with its decimals (159.962).
std::string RefreshText(double hz) {
  char text[32];
  std::snprintf(text, sizeof(text), hz == std::floor(hz) ? "%.0f" : "%.3f", hz);
  return text;
}

#ifdef _WIN32
// The refresh rate of the display the game opens on (the monitor cvar: 1 = the primary, then the others in
// EnumDisplayMonitors order, as SDL numbers them), exact as Windows keeps its current mode (159.96 for a "160 Hz"
// monitor); 0 when unknown.
double DisplayRefresh(int32_t monitor) {
  std::vector<HMONITOR> monitors;
  EnumDisplayMonitors(
      nullptr, nullptr,
      [](HMONITOR handle, HDC, LPRECT, LPARAM list) -> BOOL {
        reinterpret_cast<std::vector<HMONITOR>*>(list)->push_back(handle);
        return TRUE;
      },
      LPARAM(&monitors));
  std::stable_partition(monitors.begin(), monitors.end(), [](HMONITOR handle) {
    MONITORINFO info = {sizeof(info)};
    return GetMonitorInfoW(handle, &info) && (info.dwFlags & MONITORINFOF_PRIMARY);
  });
  if (monitors.empty()) {
    return 0.0;
  }
  MONITORINFOEXW info = {};
  info.cbSize = sizeof(info);
  size_t index = monitor >= 1 && size_t(monitor) <= monitors.size() ? size_t(monitor) - 1 : 0;
  if (!GetMonitorInfoW(monitors[index], &info)) {
    return 0.0;
  }
  UINT32 path_count = 0;
  UINT32 mode_count = 0;
  if (GetDisplayConfigBufferSizes(QDC_ONLY_ACTIVE_PATHS, &path_count, &mode_count) != ERROR_SUCCESS) {
    return 0.0;
  }
  std::vector<DISPLAYCONFIG_PATH_INFO> paths(path_count);
  std::vector<DISPLAYCONFIG_MODE_INFO> modes(mode_count);
  if (QueryDisplayConfig(QDC_ONLY_ACTIVE_PATHS, &path_count, paths.data(), &mode_count, modes.data(), nullptr) !=
      ERROR_SUCCESS) {
    return 0.0;
  }
  for (UINT32 i = 0; i < path_count; ++i) {
    DISPLAYCONFIG_SOURCE_DEVICE_NAME source = {};
    source.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_SOURCE_NAME;
    source.header.size = sizeof(source);
    source.header.adapterId = paths[i].sourceInfo.adapterId;
    source.header.id = paths[i].sourceInfo.id;
    if (DisplayConfigGetDeviceInfo(&source.header) != ERROR_SUCCESS ||
        std::wcscmp(source.viewGdiDeviceName, info.szDevice) != 0) {
      continue;
    }
    const DISPLAYCONFIG_RATIONAL& rate = paths[i].targetInfo.refreshRate;
    return rate.Denominator ? double(rate.Numerator) / double(rate.Denominator) : 0.0;
  }
  return 0.0;
}
#endif

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
    int32_t frame_rate = rex::cvar::Query<int32_t>(GAME_FRAME_RATE_CVAR);
    // Frame generation (NativeXRenderer D123; GH Metallica Launcher's Video > Frame generation): the frame rate chosen
    // is the one shown, so the game runs at half of it and AMD FSR 3.1 draws a frame between each two of its own. It
    // needs DLSS or FSR 3.1 (NativeXRenderer turns it off otherwise and says so) and 120 FPS or more: the game at 60 or
    // more, never below the console's.
    if (rex::cvar::Query<bool>("native_x_renderer_frame_generation") && rex::cvar::Query<bool>("native_x_renderer")) {
      std::string aa = rex::cvar::Query<std::string>("native_x_renderer_antialiasing");
      bool temporal = aa == "dlaa" || aa.rfind("dlss_", 0) == 0 || aa.rfind("fsr_", 0) == 0;
      if (temporal && frame_rate < 120) {
        rex::cvar::SetFlagByName("native_x_renderer_frame_generation", "false");
        REXLOG_INFO("[options] frame generation off: it needs " GAME_FRAME_RATE_CVAR " at 120 or more (the game would "
                    "run at {} FPS)",
                    frame_rate / 2);
      } else if (temporal) {
        REXLOG_INFO("[options] frame generation: {} FPS shown, the game at {}", frame_rate, frame_rate / 2);
        frame_rate /= 2;
      }
    }
    // V-Sync off (the game's vertical blank on its own timer: vblank_follows_display false): at the display's own rate
    // when the frame rate chosen is that rate, within 2 % (159.96 for 160 on a "160 Hz" monitor). At the round number
    // the timer ran a hair faster than the panel, and a G-Sync monitor tore at its ceiling; at the display's rate it did
    // not (Gustavo, 2026-11-04). With V-Sync the vertical blank follows the display's already.
    double rate = frame_rate;
#ifdef _WIN32
    if (frame_rate > 60 && !rex::cvar::Query<bool>("vblank_follows_display")) {
      double display = DisplayRefresh(rex::cvar::Query<int32_t>("monitor"));
      if (display > 0.0 && std::abs(display - frame_rate) <= frame_rate * 0.02) {
        rate = display;
        REXLOG_INFO("[options] V-Sync off: the game at the display's {:.3f} Hz ({} chosen)", display, frame_rate);
      }
    }
#endif
    if (frame_rate > 60) {
      rex::cvar::SetFlagByName("video_mode_refresh_rate", std::to_string(rate));
    }
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
    features += std::string(" ") + GAME_FRAME_RATE_CVAR + "=" + std::to_string(rex::cvar::Query<int32_t>(GAME_FRAME_RATE_CVAR)) +
                " refresh=" + RefreshText(rex::cvar::Query<double>("video_mode_refresh_rate"));
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
