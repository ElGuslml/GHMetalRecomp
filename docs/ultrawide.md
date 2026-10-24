# Ultrawide displays

**Status: working, one known issue (2026-10-24).** Checked in a 2560×1080 window: the title screen, the main menu,
the setlist and a song. Known issue: a dark band, the shape of the note highway but a little wider, shows around it
during songs (below).

## Symptom

On a screen wider than 16:9 (21:9, 32:9) the game shows its 16:9 image with black bars at the sides. Stretching the
image over the whole screen distorts everything horizontally.

## Cause

The game renders a 1280×720 frame. Its cameras take the screen's aspect ratio from a setting its scripts give
(`SetScreen { aspect = ... }`, 16:9), and its 2D layer (menus, HUD, and the note highway, which is made of screen
elements) is laid out on a 1280×720 canvas under one root element, `root_window`.

## Fix

`src/widescreen.cpp` with two `[[midasm_hook]]`s in `ghmetallica_functions.toml`, active only with
`ghmetallica_ultrawide` (GH Metallica Launcher's image format on a screen or window wider than 16:9). The 1280×720
frame is stretched over the screen (anamorphic), and the game is corrected by the same factor (screen aspect ratio
÷ 16:9, for example 1.34 on 3440×1440):

- **The stage and the band are seen wider (Hor+).** The screen aspect ratio the scripts set is widened; the
  engine's cameras already widen their field of view for it, keeping the vertical one.
- **The menus, the HUD and the note highway stay 16:9 in the center** (`ghmetallica_ultrawide_hud`):
  `root_window`'s scale is narrowed by the factor around the center, and every element under it follows. In the
  menus the 3D room behind them fills the sides.

**Known issue:** during songs, a dark band shaped like the note highway, a little wider than it, shows at its sides:
something drawn behind the highway is not narrowed with it. It does not change the notes or the gameplay.

## With NativeXRenderer

When NativeXRenderer draws the game (GH Metallica Launcher's renderer row), it renders at the screen's aspect ratio
itself with its image format "fill the screen" (`native_x_renderer_fill_screen`), and the same hooks widen the
cameras and keep the 2D layer at 16:9 by its factor: the image is the same, without the stretched 1280×720 frame.

## Turning it off

Choose 16:9 in GH Metallica Launcher's image format (it writes `ghmetallica_ultrawide = false`), or start the game with
`--ghmetallica_ultrawide=false`. With `ghmetallica_ultrawide_hud = false` the 2D layer is stretched with the image.
