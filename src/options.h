// ghmetallica - GH Metallica Launcher options.
//
// The launcher is a separate executable ("GH Metallica Launcher.exe", launcher/) that writes
// ghmetallica_options.toml next to GHMetallicaRecomp.exe. The game executable only loads it, after ghmetallica.toml and before the
// window and the GPU are created (which is when those options are read), and starts right away without showing
// anything, so it can be launched from Steam Big Picture without a mouse.

#pragma once

#include <filesystem>

namespace ghmetallica {

// Loads options_path (if it exists) and logs the effective values.
// gpu_plugin_path: the GPU DLL (rexgpu-xenos.dll), which holds the render scale cvars.
void LoadLauncherOptions(const std::filesystem::path& options_path,
                         const std::filesystem::path& gpu_plugin_path);

// The GPU DLL next to the executable (rexgpu-xenos.dll, or its RelWithDebInfo/Debug variant).
std::filesystem::path GpuPluginPath();

}  // namespace ghmetallica
