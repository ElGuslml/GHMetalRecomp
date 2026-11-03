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
  card. (AMD FSR 3.1 and NVIDIA DLSS appear only in games that mark their 3D view for NativeXRenderer; this one does
  not.)
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

## Turning it off

Upscaling: Off and Sharpness: Off in the launcher, or in `ghmetallica_options.toml`:

```toml
native_x_renderer_scaling = "bilinear"
native_x_renderer_resolution_scale = 0
native_x_renderer_sharpness = 0
```
