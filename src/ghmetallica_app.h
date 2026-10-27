// ghmetallica - the app: the SDK's virtual hooks (rex::ReXApp).

#pragma once

#include <rex/rex_app.h>

#include <rex/cvar.h>
#include <rex/filesystem.h>
#include <rex/kernel/xam/module.h>
#include <rex/logging.h>
#include <rex/system/kernel_state.h>
#include <rex/system/xthread.h>
#include <rex/ui/imgui_dialog.h>

#include <native_x_renderer/native_x_renderer.h>

#include "native_x_renderer_ghmetallica.h"
#include "keyboard.h"
#include "options.h"
#include "project.h"
#include "startup.h"
#include "widescreen.h"

class GhmetallicaApp : public rex::ReXApp {
 public:
  using rex::ReXApp::ReXApp;

  static std::unique_ptr<rex::ui::WindowedApp> Create(
      rex::ui::WindowedAppContext& ctx) {
    return std::unique_ptr<GhmetallicaApp>(new GhmetallicaApp(ctx, "ghmetallica",
        PPCImageConfig));
  }

  // Other hooks the SDK offers (rex/rex_app.h): OnPostInitLogging, OnLoadXexImage, OnPostLoadXexImage,
  // OnCreateDialogs, OnShutdown.
  // Use the "xenos" GPU emulation (rexgpu-xenos.dll) unless another one was given.
  void OnPreSetup(rex::RuntimeConfig& config) override {
    if (config.gpu_plugin.empty()) {
      config.gpu_plugin = "xenos";
    }
  }

  // Game files: without --game_data_root, assets/ next to GHMetallicaRecomp.exe or the repository's assets/. User data
  // (shader cache, downloadable content; the saves and achievements before the profiles): userdata/ next to the
  // executable; the saves, settings and achievements in the launchers' current profile (src/startup.cpp).
  void OnConfigurePaths(rex::PathConfig& paths) override {
    if (paths.game_data_root.empty()) {
      paths.game_data_root = ghmetallica::DefaultGameRoot();
    }
    ghmetallica::ConfigureUserData(paths.user_data_root, paths.cache_root);
    ghmetallica::ConfigureProfile(paths.user_data_root);
  }

  // ghmetallica.toml with the built-in defaults if it is missing, then (after the SDK loads it) the game files check
  // and GH Metallica Launcher's options (separate executable, launcher/): ghmetallica_options.toml, before the window and the GPU
  // are created. The game shows no window of its own. No game files check when the launcher asks for the
  // achievements' list (--achievements_cache_only: no window, a question would wait unseen).
  bool SetupEnvironment() override {
    auto exe_dir = rex::filesystem::GetExecutableFolder();
    ghmetallica::EnsureDefaultConfig(exe_dir / GAME_CONFIG);
    if (!rex::ReXApp::SetupEnvironment()) return false;
    REXLOG_INFO("[startup] User data: {}", user_data_root().string());
    REXLOG_INFO("[startup] {}", native_x_renderer::Describe());
    if (!rex::cvar::Query<bool>("achievements_cache_only") && !ghmetallica::CheckGameFiles(game_data_root())) return false;
    ghmetallica::LoadLauncherOptions(exe_dir / GAME_OPTIONS, ghmetallica::GpuPluginPath());
    return true;
  }

  // Window title (the SDK sets "<app name> <SDK build stamp>"), then NativeXRenderer, before the guest starts: its
  // hooks (src/native_x_renderer_ghmetallica.cpp) are pass-throughs until it is initialized; then ultrawide
  // (src/widescreen.cpp): the cameras' aspect ratio follows the window.
  void OnPostSetup() override {
    if (window()) {
      window()->SetTitle(GAME_TITLE);
    }
    native_x_renderer::Initialize(ghmetallica::NativeXRendererProfile(),
                                  runtime() ? runtime()->graphics_system() : nullptr, window(),
                                  runtime() ? runtime()->cache_root() : std::filesystem::path());
    if (window()) {
      ghmetallica::SetOutputSize(window()->GetActualPhysicalWidth(), window()->GetActualPhysicalHeight());
    }
    ghmetallica::InitializeKeyboard();
  }
  void OnWindowPixelSizeChanged(uint32_t pixel_width, uint32_t pixel_height) override {
    ghmetallica::SetOutputSize(pixel_width, pixel_height);
  }

  // Ultrawide's black bars outside a song (src/widescreen.cpp): a dialog of the SDK's ImGui layer, gone before it.
  void OnCreateDialogs(rex::ui::ImGuiDrawer* drawer) override { menu_bars_ = ghmetallica::CreateMenuBars(drawer); }
  void OnShutdown() override { menu_bars_.reset(); }

  // On exit, log whether the game asked to launch another executable.
  void OnGuestThreadExit(rex::system::XThread* thread) override {
    (void)thread;
    auto* rt = runtime();
    if (!rt || !rt->kernel_state()) return;
    auto xam = rt->kernel_state()->GetKernelModule<rex::kernel::xam::XamModule>("xam.xex");
    if (!xam) return;
    auto& ld = xam->loader_data();
    REXLOG_INFO("[ghmetallica] Title exited. launch_path='{}' flags=0x{:X} launch_data={} bytes",
                ld.launch_path, ld.launch_flags, ld.launch_data.size());
  }

 private:
  std::unique_ptr<rex::ui::ImGuiDialog> menu_bars_;
};
