# V-Sync

**Status: added (2026-10-29), on by default.**

## Symptom

A player asked for a V-Sync option; the image can tear on some PCs although `vsync = true` is in `ghmetallica.toml`.

## Cause

The SDK's `vsync` is not the display's: it is the console's vertical blank the SDK emulates, which paces the game at
the launcher's frame rate (false: no pacing, about 660 FPS in the menus). How each frame reaches the display is the
renderer's choice:

- NativeXRenderer (the default) presented without waiting: MAILBOX where the driver offers it (no tearing; NVIDIA
  does), else IMMEDIATE (tearing).
- The Xenos path tears by design, as Xenia does: Direct3D 12 presents with the tearing flag, Vulkan prefers
  IMMEDIATE.

Without tearing, a frame rate other than the display's refresh rate (60 FPS on a 144 Hz display) still shows each
frame for two or three refreshes in turn: the launcher's frame rate matching the display is what looks smoothest.

## Fix

GH Metallica Launcher, Settings > Graphics > V-Sync (RecompLauncher 3.4):

- **On** (default): never torn. NativeXRenderer `native_x_renderer_present_mode = "vsync"` (MAILBOX, else FIFO:
  NativeXRenderer's D106, with DiligentCore patch 0009); the Xenos path without its tearing modes
  (`d3d12_allow_variable_refresh_rate_and_tearing`, `vulkan_allow_present_mode_immediate` and
  `vulkan_allow_present_mode_fifo_relaxed` false).
- **Off**: each frame shown as soon as it is ready (`"immediate"`, the Xenos options true): tearing, or a G-Sync or
  FreeSync display following the game.

The log says which mode the driver gave: `Using VK_PRESENT_MODE_..._KHR swap chain present mode` (NativeXRenderer).

## Turning it off

The launcher's V-Sync row, or `native_x_renderer_present_mode = ""` in `ghmetallica_options.toml` for the earlier
behavior.
