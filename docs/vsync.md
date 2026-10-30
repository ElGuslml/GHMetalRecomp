# V-Sync

**Status: added (2026-10-29), on by default; in step with the display since 2026-10-30.**

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

Measured afterwards with PresentMon (2026-10-30, 30 s of a song, fullscreen at 3440x1440, RTX 4070 Ti): the emulated
vertical blank is a timer at exactly the frame rate asked for, a hair away from the display's (160.000 for a
159.962 Hz monitor). At 160 FPS the game ran ahead of the display and NVIDIA's MAILBOX queued three frames: every
frame on screen 16-19 ms after the game presented it. At 60 FPS on a 60 Hz monitor the timer's phase slid against the
display's: from present to display anywhere between 7 and 15 ms, changing over a minute.

## Fix

GH Metallica Launcher, Settings > Graphics > V-Sync (RecompLauncher 3.4 and 3.5):

- **On** (default): each frame shown once, never torn. NativeXRenderer presents with FIFO
  (`native_x_renderer_present_mode = "vsync"`, NativeXRenderer's D107), and the game's vertical blank follows the
  monitor's own (`vblank_follows_display`, the SDK's recomp change 61) where the frame rate is the monitor's refresh
  rate or a whole fraction of it. The Xenos path presents without its tearing modes
  (`d3d12_allow_variable_refresh_rate_and_tearing`, `vulkan_allow_present_mode_immediate`,
  `vulkan_allow_present_mode_fifo_relaxed` false).
- **Off**: each frame shown as soon as it is ready (`"immediate"`, the Xenos options true, the vertical blank by the
  timer): tearing, or a G-Sync or FreeSync display following the game.

| Fullscreen, PresentMon | 160 FPS, 160 Hz: before | after | 60 FPS, 60 Hz: before | after |
|---|---|---|---|---|
| Frames shown once | 4799 of 4799 | 4786 of 4788 | 1800 of 1800 | 1800 of 1800 |
| Presents within 0.5 ms of the period | 90 % | 91 % | 52 % | 99.8 % |
| Present to display | 17.8 ms | 4.2 ms | 11.2 ms (7-15) | 16.5 ms (steady) |

The game's notes are judged against the song, not the screen, so a steady delay to the screen is what the video
calibration takes off; one that slides cannot be calibrated. Recalibrate the video after updating.

A frame rate the display does not divide (60 or 120 FPS on a 144 or 160 Hz monitor) keeps the timer and judders by
arithmetic (frames shown for 1 and 2, or 2 and 3 refreshes): choose the monitor's refresh rate in the launcher, or use
a G-Sync or FreeSync monitor with V-Sync off.

The log says what was chosen: `[vblank] following the display's vertical blank: \\.\DISPLAY1 at 160 Hz` (or why the
timer is kept) and `[native_x_renderer] present mode: vsync (FIFO)`.

## Turning it off

The launcher's V-Sync row. For the vertical blank alone, `vblank_follows_display = false` in
`ghmetallica_options.toml`; `native_x_renderer_present_mode = "mailbox"` for the earlier presentation.
