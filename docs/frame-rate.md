# Frame rates above 60

**Status: working, experimental (2026-10-25).** The game runs at 120, 144, 160, 165 or 240 FPS, chosen in GH Metallica
Launcher's frame rate row (offered up to the monitor's refresh rate). Checked: 120 FPS through the career's first song
at the console's speed, 160 FPS in the menus; Gustavo's play test at 120 and 160: songs in sync and smooth, the boot's
star and both calibrations as at 60, the whammy on star power sustains as at 60.

## Symptom

The game runs at 60 FPS, as on the console, also on a monitor that shows more.

## Cause

The game paces its frames by the console's vertical blank, 60 times a second: it shows a frame when the previous
one has been shown and on the next vertical blank. No patch for a higher frame rate exists for any Guitar Hero
(Xenia's game patches, RPCS3's patches): they already run at 60 on the consoles.

## Fix

The vertical blank the SDK emulates runs at the chosen rate (`ghmetallica_frame_rate`, which sets the SDK's
`video_mode_refresh_rate`; `src/framerate.cpp`, `src/options.cpp`), and the game shows a frame on each one. Its clock
measures real time, so it keeps its speed: at 120 FPS the career's first song took as long from its start to the
failure of a player who presses nothing as at 60 (75.8 s and 74.5 s). With the vertical blank emulated every
millisecond (`vsync = false`) the menus ran at about 660 FPS: the game has no limit of its own.

The threads' waits follow the vertical blank's rate (`src/thread_waits.cpp`; their first version assumed 60 Hz and
held 120 at about 67 FPS).

Some of the game's timing counts frames instead of time, and ran twice as fast at 120 FPS and faster still at 160
(Gustavo: the boot's spinning star, the audio calibration's beeps after the first four, the video calibration's
targets). `ghmetallica_frame_rate_fix` (on) covers:
- **script waits in game frames** (`Wait N gameframes`, about 300 in the game's scripts, used by loops that animate a
  little each frame): above 60 FPS each lasts what it lasts on the console, N / 60 s, and a loop of them keeps the
  console's pace (the audio calibration's beeps: every 1.066 s at 60, 120 and 160 FPS, measured). Except the one
  loop that measures its own time: star power's drain, which takes the frame's time (`current_deltatime`) off the
  meter each frame. Held to 60 passes a second it took 60 frames' time a second, and star power lasted 160 / 60 times
  as long at 160 FPS (Gustavo, 2026-10-27). It runs every frame again; no other script of the game's 7822 waits in game
  frames and reads the frame's time;
- **button presses polled by those scripts**: a press shows for one frame, and a script held to the console's pace
  skips frames, so the calibrations missed strums (3 of 7 at 160 FPS in a test). Each script is given, once, the
  presses of the frames it skipped (7 of 7);
- **the whammy on a sustain**, told by the frame: the bar counted as moving when it moved more than 3 % of its
  travel since the previous frame, for the next 5 frames, and a star power sustain turns blue or back after 5 frames
  in a row. At 120 a frame's movement is half as big and 5 frames half as long, so the blue came and went (Gustavo).
  The bar is compared with its position a 60th of a second before, and the 5 frames last 5 / 60 s;
- **the loading screen**, which turns its star by a fixed angle each frame: the angle is scaled to the frame rate
  (3.60 radians a second at 60, 3.52 at 160 while the boot ran at 156 FPS; 9.34 without this);
- **clocks counted in vertical blanks**, which divide by the game's vertical blanks per second (60, or 50 for PAL):
  they divide by the real rate. The game's value itself stays 60, as other code compares it with 60.

## Turning it off

Choose 60 FPS in GH Metallica Launcher's frame rate row, or start the game with `--ghmetallica_frame_rate=60`.
`--ghmetallica_frame_rate_fix=false` keeps the higher rate without those fixes (to compare). A calibration made at a
higher frame rate before these fixes may be off: calibrate again.
