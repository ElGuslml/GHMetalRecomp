# A lower internal resolution looks soft: AMD FSR 1 and NVIDIA Image Scaling

**Status: added (2026-11-03), off by default (the bilinear stretch, as before). NativeXRenderer only.**

## Symptom

With an internal resolution below the screen's (the launcher's Settings > Display > Internal resolution, for a
slower graphics card), the image was stretched to the screen with a bilinear filter: the menus' text, the HUD and
the edges of the 3D image looked soft.

## Cause

That stretch is the plain one. The game composes its HUD into the same image as the stage, so everything is
enlarged together.

## Fix

NativeXRenderer's D115 (`native_x_renderer/docs/decisions.md`): two rows in the launcher's Settings > Display, with
NativeXRenderer (RecompLauncher 3.8):

- **Upscaling** (`native_x_renderer_scaling`): Bilinear (`bilinear`, as before), AMD FSR 1 (`fsr`) or NVIDIA NIS
  (`nis`). FSR 1 and NVIDIA Image Scaling enlarge the image keeping edges and text sharp. Both work on any graphics
  card.
- **Sharpness** (`native_x_renderer_sharpness`): Default (-1: AMD's and NVIDIA's recommendation for FSR 1 and NIS,
  none with the bilinear stretch), Off (0), Low (0.3), Medium (0.6), High (1). With the bilinear stretch, AMD CAS
  sharpens the image. It works at the screen's own resolution too.

Measured at 3440x1440 from a 1720x720 image (RTX 4070 Ti): the menu's text about twice as sharp with FSR 1 or NIS
as with the stretch; the GPU's time per frame +0.14 ms with FSR 1, +0.20 ms with NIS, +0.04 ms with CAS.

## Turning it off

Upscaling: Bilinear and Sharpness: Default in the launcher, or in `ghmetallica_options.toml`:

```toml
native_x_renderer_scaling = "bilinear"
native_x_renderer_sharpness = -1
```
