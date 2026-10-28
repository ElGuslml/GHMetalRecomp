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

`src/widescreen.cpp` with eight `[[midasm_hook]]`s in `ghmetallica_functions.toml`, active only with
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

## The hit sparks (2026-10-26)

Gustavo: the sparks of a hit note sat outside their lane at 21:9 (the green one's to the left). They are 2D particles,
quads drawn by `sub_821B79F8` (DrawVerticesUP at 0x821B7B44, 4 vertices of 20 bytes, x in pixels of the 1280×720
canvas) only when notes are hit, which no hook narrowed. `GHMetallicaParticleHook` narrows them like the sprites.
Found with temporary probes on the engine's 2D draws during a song played on Easy with the keyboard (every fret press
strums): the particles' quads came at up to 98 a second with x from 137 to 1011, unnarrowed.

## Black bars outside a song (2026-10-26)

The menus, the loading screens and the boot videos were made for 16:9: at their sides the 3D room behind the menus
showed, or the loading screen was stretched over the whole screen (it is drawn by another element's quad,
`sub_82465228`, which the 2D layer's hooks did not narrow). Now:
- that quad is narrowed too (`GHMetallicaElementQuadHook`, 0x82465718), so the loading screen is 16:9 in the center;
- outside a song black bars cover the sides (`ghmetallica_menu_bars`, on by default), drawn over the image by a dialog
  of the SDK's ImGui layer, so with both renderers. A song is told by the note highway's board, drawn in every frame
  of a song from the band's intro on and while it is paused, and never in the menus; while a picture element (the
  loading screen) is drawn it does not count, as the song's first frames run under the end of its loading screen. In a
  song, its intro and its pause menu the stage fills the screen as before.

The first version (earlier the same day) narrowed only the screen elements under `root_window` and widened the
`SetScreen` aspect ratio, which the cameras turned out not to use: the scene was stretched, and the highway's notes
and a dark band around it (the depth quad, left at full width) did not match its board.

## With NativeXRenderer

When NativeXRenderer draws the game (GH Metallica Launcher's renderer row), it renders at the screen's aspect ratio
itself with its image format "fill the screen" (`native_x_renderer_fill_screen`), and the same hooks widen the 3D
views and keep the 2D layer at 16:9 by its factor: the image is the same, without the stretched 1280×720 frame.

It can also render another aspect ratio than the screen's (2026-10-28, NativeXRenderer D104,
`native_x_renderer_aspect`): the launcher's image format offers 16:9, 21:9, 32:9 and the screen's own on any screen,
so 21:9 on a 16:9 screen (bars above and below) or 32:9 on a 21:9 one. The menus' black bars are drawn at the sides
of the image, not of the screen.

## Turning it off

Choose 16:9 in GH Metallica Launcher's image format (it writes `ghmetallica_ultrawide = false`), or start the game with
`--ghmetallica_ultrawide=false`. With `ghmetallica_ultrawide_hud = false` the 2D layer is stretched with the image.
With `ghmetallica_menu_bars = false` the menus show the room at their sides as before.
