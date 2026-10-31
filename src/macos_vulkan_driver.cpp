// ghmetallica - macOS: which Vulkan driver draws (src/macos_vulkan_driver.h).
//
// The SDK draws with Vulkan, which macOS runs through a driver on top of Metal. Two are bundled
// (installer/macos/make_dmg.sh), in Contents/Resources/vulkan with the Vulkan loader:
// - MoltenVK, the SDK's own, which it selects when nothing else is chosen (src/ui/vulkan/vulkan_moltenvk.cpp there:
//   VK_DRIVER_FILES is left alone when already set): the default;
// - KosmicKrisp (LunarG, Mesa; Vulkan 1.4 conformant on Apple Silicon with macOS 26 or later), with
//   ghmetallica_vulkan_driver = "kosmickrisp": lighter on the GPU (Xenos path at 3x: 57-81 % against MoltenVK's
//   95-98 %), but its device was lost in a song on the Xenos path (2026-10-31, Vulkan SDK 1.4.363.0), which the SDK
//   ends the game on.
// A VK_DRIVER_FILES set by the player wins over both.

#include "macos_vulkan_driver.h"

#include "startup.h"

#include <rex/cvar.h>
#include <rex/filesystem.h>
#include <rex/logging.h>

#include <cstdlib>
#include <filesystem>
#include <string>

REXCVAR_DEFINE_STRING(ghmetallica_vulkan_driver, "moltenvk", "GHMetallica",
                      "macOS: the Vulkan driver on top of Metal: \"kosmickrisp\" (LunarG's, when bundled) or "
                      "\"moltenvk\"");

namespace ghmetallica::macos {

void SelectVulkanDriver() {
  if (const char* preset = std::getenv("VK_DRIVER_FILES"); preset && *preset) {
    REXLOG_INFO("[vulkan] VK_DRIVER_FILES set outside the game: {}", preset);
    return;
  }
  const std::string driver = REXCVAR_GET(ghmetallica_vulkan_driver);
  if (driver != "kosmickrisp") {
    REXLOG_INFO("[vulkan] driver: MoltenVK (ghmetallica_vulkan_driver = \"{}\")", driver);
    return;
  }
  // Contents/MacOS/../Resources/vulkan, the bundle's layout; next to the executable in a development build.
  const std::filesystem::path exe_dir = rex::filesystem::GetExecutableFolder();
  for (const std::filesystem::path& root : {exe_dir.parent_path() / "Resources" / "vulkan", exe_dir / "vulkan"}) {
    const std::filesystem::path icd = root / "share" / "vulkan" / "icd.d" / "libkosmickrisp_icd.json";
    std::error_code ec;
    if (std::filesystem::exists(icd, ec) && std::filesystem::exists(root / "lib" / "libvulkan_kosmickrisp.dylib", ec)) {
      setenv("VK_DRIVER_FILES", icd.c_str(), 1);
      setenv("VK_ICD_FILENAMES", icd.c_str(), 1);
      // Mesa's shader cache (KosmicKrisp's shaders as Metal code), which it keeps nowhere by default here: with the
      // SDK's shader storage (the pipelines built before the game starts) and Metal's own cache of compiled code, a
      // shader is converted and compiled once, not at every start. A MESA_SHADER_CACHE_DIR set by the player wins.
      const std::filesystem::path mesa_cache = ghmetallica::DataFolder() / "userdata" / "cache" / "mesa";
      std::filesystem::create_directories(mesa_cache, ec);
      setenv("MESA_SHADER_CACHE_DIR", mesa_cache.c_str(), 0);
      REXLOG_INFO("[vulkan] driver: KosmicKrisp ({}); shader cache {}", icd.string(), mesa_cache.string());
      return;
    }
  }
  REXLOG_INFO("[vulkan] driver: MoltenVK (KosmicKrisp is not bundled)");
}

}  // namespace ghmetallica::macos
