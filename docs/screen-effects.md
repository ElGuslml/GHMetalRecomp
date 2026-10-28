# Turning off the game's screen effects

**Status: options in `ghmetallica.toml` (2026-10-28), all on by default.** Not in GH Metallica Launcher yet.

## What the game applies

The engine's screen effects (its scripts' `ScreenFX_*` functions) work on the 3D image; the note highway and the HUD
are drawn after them. At boot and at each venue the game makes the same list and its scripts change their parameters
as the scene goes on:

| Option | Effect |
|---|---|
| `ghmetallica_fx_bloom` | The glow around lights and bright surfaces |
| `ghmetallica_fx_depth_of_field` | The background blurred in some camera shots, behind the menus and the pause menu |
| `ghmetallica_fx_grain` | The film grain over the image |
| `ghmetallica_fx_color` | The color correction: brightness, contrast, hue and saturation, each venue's look, fades to black, the image greyed while the rock meter is red |
| `ghmetallica_fx_vignette` | Darkened corners, where a venue or a moment turns it on |
| `ghmetallica_fx_ink` | The ink blotches a song adds over the image |

The list also has a radial blur (off by default) and one effect not identified yet; they have no option.

## How

The six effects are applied together by one final pass (the engine's `sub_8251EC80`), which picks a variant of its
shader by a mask of the effects that are on and sets each effect's constants by the same bits. `src/screen_fx.cpp`
(a `[[midasm_hook]]` in `ghmetallica_functions.toml`) takes the bits of the effects turned off out of that mask. The
passes that prepare the bloom and the blurred image still run.

## Turning one off

`ghmetallica_fx_<name> = false` in `ghmetallica.toml` (or `--ghmetallica_fx_<name>=false`); they also show, and apply
at once, in the F4 settings overlay (the GHMetallica category).

The grain depends on the color correction: with the color correction on, the grain can be on or off; with the color
correction off, the grain is off too, whatever `ghmetallica_fx_grain` says, because that pass also tones the grain down
and without it the grain is far stronger. A launcher's switches follow the same rule (the grain's switch off and
disabled while the color's is off).
