# A lower internal resolution looks soft: AMD FSR 1 and NVIDIA Image Scaling

**Status: added (2026-11-03), off by default (the bilinear stretch, as before). NativeXRenderer only.**

## Symptom

With an internal resolution below the screen's (the launcher's Settings > Video > Internal resolution, for a slower
graphics card), the image was stretched to the screen with a bilinear filter: the menus' text, the HUD and the edges
of the 3D image looked soft.

## Cause

That stretch is the plain one. The game composes its HUD into the same image as the stage, so everything is
enlarged together.

## Fix

NativeXRenderer's D115 (`native_x_renderer/docs/decisions.md`), with the rows other PC games have in the launcher's
Settings > Video (RecompLauncher 3.9, NativeXRenderer only):

- **Upscaling**: Off (the image at the internal resolution you choose, stretched as before), AMD FSR 1 or NVIDIA NIS.
  FSR 1 and NVIDIA Image Scaling enlarge the finished image keeping edges and text sharp. Both work on any graphics
  card. AMD FSR 3.1 (any graphics card) and NVIDIA DLSS (RTX) rebuild the 3D stage from several frames, which also
  antialiases it: see "FSR 3.1 and DLSS" below.
- **Quality**: how much smaller the image is drawn: Ultra Quality 77 % of the screen's lines, Quality 67 %, Balanced
  59 %, Performance 50 %, Ultra Performance 33 %, never below the game's own 720 lines (a 1080p screen offers Ultra
  Quality and Quality). Each value shows the size it draws at; the Internal resolution row then shows it, locked.
- **Sharpness**: Off, Low, Medium, High; Medium is AMD's and NVIDIA's recommendation, and FSR 1 and NIS start there.
  With FSR 1 and NIS their own sharpening, otherwise AMD CAS; it works without upscaling too.

In `ghmetallica_options.toml`: `native_x_renderer_scaling` (`bilinear`, `fsr`, `nis`),
`native_x_renderer_resolution_scale` (the internal resolution over 720 lines: 960 lines is 1.333333),
`native_x_renderer_sharpness` (0 to 1; FSR 1's Medium 0.9, NIS's 0.5, CAS's 0.6) and the launcher's own
`launcher_upscaling_quality`.

Measured at 3440x1440 from a 1720x720 image (RTX 4070 Ti): the menu's text about twice as sharp with FSR 1 or NIS
as with the stretch; the GPU's time per frame +0.14 ms with FSR 1, +0.20 ms with NIS, +0.04 ms with CAS.

## FSR 3.1 and DLSS

**Status: added (2026-11-03), off by default. NativeXRenderer's D118-D121.**

The game does not tell NativeXRenderer where its 3D stage begins and ends, so NativeXRenderer finds it from the
draws: from the first 3D draw of the screen's size to the copy of the finished stage the game makes before its note
highway and HUD (`upscale_resolve_caller` in `src/native_x_renderer_ghmetallica.cpp`). The camera's movement comes
from a vote of those draws. FSR 3.1 and DLSS then work on the stage only: the highway, the notes and the HUD are drawn
after them, at the screen's resolution, never rebuilt from earlier frames.

- **Quality: Native** (FSR 3.1 Native AA, NVIDIA DLAA): the stage at the screen's resolution, antialiased; it replaces
  the Antialiasing row's SMAA or FXAA (locked while one of them is chosen).
- **Ultra Quality to Ultra Performance** (experimental): the stage drawn smaller and rebuilt at the screen's size
  (DLSS Quality at 3440x1440: 2293x960); the highway and HUD stay at the screen's.

Measured in a song at 3440x1440 and 160 FPS (RTX 4070 Ti): the GPU's time per frame 2.5-3.3 ms with SMAA, 3.95-4.23 with
DLAA, 3.67-3.92 with DLSS Quality, 3.22-3.56 with FSR 3.1 Quality. This game is light for the graphics card: DLSS and
FSR 3.1 cost more than drawing the stage smaller saves, so they are for the image, not for speed. In
`ghmetallica_options.toml`: `native_x_renderer_antialiasing` = `dlaa`, `fsr_native`, `dlss_quality`, `fsr_quality`
(and `_balanced`, `_performance`, `_ultra_performance`).

## Turning it off

Upscaling: Off and Sharpness: Off in the launcher, or in `ghmetallica_options.toml`:

```toml
native_x_renderer_scaling = "bilinear"
native_x_renderer_resolution_scale = 0
native_x_renderer_sharpness = 0
native_x_renderer_antialiasing = "smaa"
```
