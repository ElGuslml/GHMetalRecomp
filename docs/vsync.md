# V-Sync

**Status: added (2026-10-29), on by default; in step with the display since 2026-10-30. Recommended with G-Sync and
FreeSync too; V-Sync off at the display's exact rate since 2026-11-04.**

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
arithmetic (frames shown for 1 and 2, or 2 and 3 refreshes): choose the monitor's refresh rate in the launcher.

## G-Sync and FreeSync (2026-11-04)

Gustavo's G-Sync monitor (159.962 Hz; NVIDIA's G-SYNC indicator on) tore with V-Sync off at 160 FPS: the timer at
160.000 put the game a hair above the panel's ceiling, and a variable refresh rate display can only follow frames that
come slower than its maximum. Up to 0.2.1 there was no V-Sync off: NativeXRenderer presented with MAILBOX, which never
tears (and queued frames).

- **V-Sync on is the one recommended, also with G-Sync or FreeSync**: it never tears, and with the game's vertical
  blank following the display it adds none of V-Sync's usual delay (no queue: 4.2 ms from present to display at 160 Hz,
  above). The launcher's help says so since RecompLauncher 3.11.3 (3.4-3.11.2 said Off was for G-Sync and FreeSync).
- **V-Sync on with G-Sync** (found later the same day: Gustavo's frame time graph showed a saw tooth for a few seconds,
  then even, over and over, at 120 Hz on a 4K TV and at 160 on the monitor). With V-Sync, the SDK finds a variable
  refresh rate display by its vertical blanks, which follow the game's frames, and keeps its timer for it
  (`[vblank] the display's vertical blanks come every 13.65 ms, not 8.33: a variable refresh rate`). That timer had two
  faults: it slept a millisecond at a time, so its ticks came 6 to 10 ms apart at 120 Hz, which a fixed refresh display
  evens out and a G-Sync display shows as they come; and it ran at the panel's maximum, so now and then frames came
  faster than the panel could show them and waited in the swap chain's queue. SDK change 70: a high resolution timer
  and `vblank_timer_spin_us = 700` (set in `src/startup.cpp`; ticks 8.33-8.38 ms apart, +0.02 cores), and on such a
  display the timer 0.25 % under its exact refresh rate (119.70 at 120, 159.56 at 159.962; the log:
  `[vblank] the timer at 119.70 Hz: the display follows the game's frames up to its 120.000 Hz`). The render thread
  wakes on an event the game's vertical blank callback sets (`src/thread_waits.cpp`), 4 us after the tick instead of
  up to half a millisecond. Gustavo: the saw tooth gone.
- **V-Sync off** runs the game just under the display's exact refresh rate when the frame rate chosen is that rate,
  within 2 % (`src/options.cpp`: the current mode's rate from `QueryDisplayConfig`, 159.962 for "160 Hz", less 0.25 %;
  the log says `V-Sync off: the game at 159.56 Hz, just under the display's 159.962 (160 chosen)`). Gustavo's test at
  the exact 159.96, before the precise timer: no tearing, the motion less even than with V-Sync on. NVIDIA's own way (a
  frame rate cap well under the maximum, about 153 at 160 Hz) was not wanted; 0.25 % under is enough with ticks this
  even.

The log says what was chosen: `[vblank] following the display's vertical blank: \\.\DISPLAY1 at 160 Hz` (or why the
timer is kept) and `[native_x_renderer] present mode: vsync (FIFO)`.

## Turning it off

The launcher's V-Sync row. For the vertical blank alone, `vblank_follows_display = false` in
`ghmetallica_options.toml`; `native_x_renderer_present_mode = "mailbox"` for the earlier presentation. With V-Sync off,
`video_mode_refresh_rate` on the command line sets the game's rate itself (with the launcher's frame rate at 60).
`vblank_timer_spin_us = 0` keeps the timer's ticks without the spin (up to half a millisecond late).
