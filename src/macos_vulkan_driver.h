// ghmetallica - macOS: which Vulkan driver draws (src/macos_vulkan_driver.cpp).

#pragma once

namespace ghmetallica::macos {

// Points the Vulkan loader at the driver ghmetallica_vulkan_driver names, from the app bundle, before the graphics
// system creates its Vulkan instance (called once the settings are loaded).
void SelectVulkanDriver();

}  // namespace ghmetallica::macos
