# Skipping the videos at the start

**Status: added, off by default (2026-11-11):** the launcher's Settings → Game → "Skip the intro videos" row, or
`ghmetallica_skip_intro = true` in `ghmetallica.toml` (or `--ghmetallica_skip_intro=true`).

## What it does

At each start the game shows its legal notice while it loads, waits 6 more seconds on it, then plays four videos:
Activision's, RedOctane's and Neversoft's logos and the game's intro. With the option on, the game goes from the
loaded legal notice straight to its title screen ("Press any button to rock"): about 2 seconds from the window opening
on our test PC.

## How

The boot is a chain of the game's menu states (`ui_boot.qb`): each video's state plays its video with the script
`PlayMovieAndWait`, then moves to the next state, as when a button skips the video. `src/intro.cpp` (a hook in
`ghmetallica_functions.toml`) makes the script function `PlayMovie` find no video for those four, the first time each
is asked for, so each state moves on at once; and the legal notice's wait of 6 seconds is 0 (the shared hook on the
script function `Wait`, `src/crowd.cpp`). The log says "[intro] boot video 'atvi' skipped" for each. The game's
development path (`skip_boot_menu`) is not used: it also chooses the primary controller and leaves the loading screen
up.

## Turning it off

The launcher's row off, or `ghmetallica_skip_intro = false` (the default): the start is as on the console.
