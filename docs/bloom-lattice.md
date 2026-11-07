# A lattice in the bloom at high resolutions

**Status: fixed (2026-11-07, NativeXRenderer D127), on by default.**

## Symptom

With NativeXRenderer above 720 lines (most visible at 1440p and ultrawide), the glow around bright things on the stage
(the lights, the neon snake) came out doubled or as a grid of copies instead of a smooth halo; the blurred background of
the menus and the pause menu had the same pattern. Reported in a GitHub issue.

## Cause

The game builds its bloom and its depth of field from reduced copies of the scene: a 640x360 downsample, then 320x180
targets where it draws the glowing objects and blurs them with a 16-tap filter. The distance between the taps is
computed by the game for 320x180 texels and passed to the shader as constants. NativeXRenderer drew everything at the
render scale, those targets too (860x360 at 3440x1440), so the taps landed two or three texels apart and skipped the
texels between them.

## Fix

NativeXRenderer keeps the reduced copies of the screen (the 1280x720 output divided by a whole number: 640x360,
320x180) at the game's resolution, as on the console; the 640x360 copy averages every pixel of the high resolution
scene it covers. The rest of the image (the scene, the HUD, the note highway) stays at the screen's resolution. It also
costs a little less GPU time. Set in `src/native_x_renderer_ghmetallica.cpp` (`unscaled_screen_fractions`).

The Xenos path (D3D12, the other renderer) still has the lattice when its resolution scale is above 1.

## Turning it off

`native_x_renderer_unscaled_screen_fractions = false` in `ghmetallica.toml` (read at startup).
