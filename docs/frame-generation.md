# Frame generation (experimental)

**Status: added (2026-11-03), off by default; in the launcher since 2026-11-04 (Settings > Video > Frame generation,
experimental). NativeXRenderer only, with DLSS or FSR 3.1.**

## What it does

AMD FSR 3.1's frame generation (NativeXRenderer D123, D124) draws a frame between each two of the game's and shows it
first: the game runs at half the display's refresh rate and the screen shows the full rate (80 -> 160 FPS on a 160 Hz
display). It works on NVIDIA and AMD cards (AMD lists RTX 20 and later, Radeon RX 5000 and later). V-Sync is turned on
while it is: the two frames are shown a refresh apart.

The note highway is interpolated as the stage is: its notes, beat lines and board move between the game's frames in
the generated one (the highway is drawn by the game in screen pixels; its movement is taken from where each piece was
in the last frame, the board's from its beat lines). The HUD stays where it is.

It adds delay to what is shown: each of the game's frames waits one refresh behind the generated one (6.25 ms at
160 Hz), plus the generation itself (about 2 ms of the graphics card's time); against the game running at the full
rate without it, about one game frame (12.5 ms at 80 FPS), as the game also reads the controls half as often. The game's
own timing does not change; the video calibration takes it. Not measured end to end (no latency meter here). NVIDIA
Reflex is not used: AMD's frame generation does not need it, and V-Sync following the display already keeps frames
from queueing (4.2 ms from present to display at 160 Hz, [vsync.md](vsync.md)).

## Measured

A song at 3440x1440 with DLAA on an RTX 4070 Ti: 159 FPS shown from the game's 80, the graphics card 54 % busy at
134 W, against 66 % and 156 W for DLAA at 160 FPS without it; the renderer's thread has twice the time per game frame.

## Turning it on

GH Metallica Launcher: Settings > Video > Upscaling: AMD FSR 3.1 or NVIDIA DLSS, then **Frame generation: AMD FSR
3.1**, and in Graphics the frame rate to see (120 FPS or more; the display's refresh rate is best). V-Sync stays on.
The frame rate chosen is the one shown: the game runs at half of it (`src/options.cpp`; 160 -> the game at 80) and the
log says `frame generation: 160 FPS shown, the game at 80`. Below 120 it stays off (the game would run under 60).

In `ghmetallica_options.toml`:

```toml
native_x_renderer_antialiasing = "dlaa"   # or fsr_native, a DLSS or FSR 3.1 mode
native_x_renderer_frame_generation = true
ghmetallica_frame_rate = 160              # shown; the game at 80
```

Without DLSS or FSR 3.1 it stays off (the log says so).
