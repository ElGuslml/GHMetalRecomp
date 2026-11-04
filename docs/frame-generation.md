# Frame generation (experimental)

**Status: added (2026-11-03), off by default, not in the launcher yet. NativeXRenderer only, with DLSS or FSR 3.1.**

## What it does

AMD FSR 3.1's frame generation (NativeXRenderer D123, D124) draws a frame between each two of the game's and shows it
first: the game runs at half the display's refresh rate and the screen shows the full rate (80 -> 160 FPS on a 160 Hz
display). It works on NVIDIA and AMD cards (AMD lists RTX 20 and later, Radeon RX 5000 and later). V-Sync is turned on
while it is: the two frames are shown a refresh apart.

The note highway is interpolated as the stage is: its notes, beat lines and board move between the game's frames in
the generated one (the highway is drawn by the game in screen pixels; its movement is taken from where each piece was
in the last frame, the board's from its beat lines). The HUD stays where it is.

It adds about one game frame of delay to what is shown (12.5 ms at 80 FPS). The game's own timing does not change; the
video calibration can take the rest.

## Measured

A song at 3440x1440 with DLAA on an RTX 4070 Ti: 159 FPS shown from the game's 80, the graphics card 54 % busy at
134 W, against 66 % and 156 W for DLAA at 160 FPS without it; the renderer's thread has twice the time per game frame.

## Turning it on

In `ghmetallica_options.toml`, with DLSS or FSR 3.1 chosen (Settings > Video > Upscaling):

```toml
native_x_renderer_frame_generation = true
ghmetallica_frame_rate = 80
```

`ghmetallica_frame_rate` should be half the display's refresh rate (60 for 120 Hz, 72 for 144 Hz, 80 for 160 Hz).
Without DLSS or FSR 3.1 it stays off (the log says so).
