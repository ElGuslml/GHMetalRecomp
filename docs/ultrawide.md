# Ultrawide displays

**Status: working (2026-10-24).** Checked at 3440×1440 on the Xenos path and with NativeXRenderer: the title screen,
the main menu, the career's opening cinematic and its first song (the note highway, its notes and their tails, the
band).

## Symptom

On a screen wider than 16:9 (21:9, 32:9) the game shows its 16:9 image with black bars at the sides. Stretching the
image over the whole screen distorts everything horizontally.

## Cause

The game renders a 1280×720 frame. Its 3D views build their projection from the camera's horizontal field of view and
the view's own size (1280×720, 16:9), and its 2D layer (menus, HUD, text, and the note highway with its notes) is
drawn in pixels of a 1280×720 canvas. Part of the note highway is 3D: a quad drawn into the depth buffer before the
scene, so that nothing is drawn behind the highway; at 16:9 it lies exactly under the highway's board.

## Fix

`src/widescreen.cpp` with six `[[midasm_hook]]`s in `ghmetallica_functions.toml`, active only with
`ghmetallica_ultrawide` (GH Metallica Launcher's image format on a screen or window wider than 16:9) or, with
NativeXRenderer, when it fills the screen. The 1280×720 frame is stretched over the screen (anamorphic), and the game
is corrected by the same factor (screen aspect ratio ÷ 16:9, for example 1.34 on 3440×1440):

- **The 3D views see more at the sides (Hor+).** Every 16:9 perspective projection is widened by the factor when the
  engine builds it, keeping the vertical field of view: the stage and the band keep their proportions. The highway's
  depth quad is narrowed with it.
- **The 2D layer stays 16:9 in the center** (`ghmetallica_ultrawide_hud`): the vertices of everything 2D drawn to the
  screen (sprites, text, the highway's board) are narrowed around the center just before they are drawn, whatever code
  placed them, so the highway's board, its notes and its depth quad still line up. A sustained note's tail is placed
  by its shader from a constant (the lane's position) and vertices in the lane's own space: both are narrowed the same
  way. The clip windows that elements draw through (a scissor rectangle around rolling digits, such as the star
  rating's number in the score box) are narrowed with them; left as they were, they cut the number away. In the menus
  the 3D room behind them fills the sides.

The first version (earlier the same day) narrowed only the screen elements under `root_window` and widened the
`SetScreen` aspect ratio, which the cameras turned out not to use: the scene was stretched, and the highway's notes
and a dark band around it (the depth quad, left at full width) did not match its board.

## With NativeXRenderer

When NativeXRenderer draws the game (GH Metallica Launcher's renderer row), it renders at the screen's aspect ratio
itself with its image format "fill the screen" (`native_x_renderer_fill_screen`), and the same hooks widen the 3D
views and keep the 2D layer at 16:9 by its factor: the image is the same, without the stretched 1280×720 frame.

## Turning it off

Choose 16:9 in GH Metallica Launcher's image format (it writes `ghmetallica_ultrawide = false`), or start the game with
`--ghmetallica_ultrawide=false`. With `ghmetallica_ultrawide_hud = false` the 2D layer is stretched with the image.
