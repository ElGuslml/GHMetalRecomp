# The crowd claps out of time on star power

**Status: fixed (2026-10-27).** Checked by Gustavo with 104 ms of audio calibration: the claps on the beat of the music.
The same happens on the Xbox 360.

## Symptom

While a player's star power is on, the crowd claps on every beat. After calibrating the audio in the game's options,
the songs' instruments and vocals move by the calibration, but the claps do not: they come late by the calibration
(104 ms with Gustavo's).

## Cause

The game plays the claps from a script run on each of the song's fretbar events: it waits 0.18 s, then starts the
crowd's beat script, which plays a clap when any player has star power on. The fretbar events come as late as the
player's audio and video calibration, and the fixed 0.18 s does not take that back.

## Fix

`src/crowd.cpp` with a `[[midasm_hook]]` in the game's script `Wait` (`ghmetallica_functions.toml`): the fretbar
script's wait is shortened by the calibration, read from the game's own settings (the global that holds the
calibrated offset, less its base), so the claps follow the calibration as the music does, whatever its value.

## Turning it off

`--ghmetallica_crowd_claps_fix=false` (or `ghmetallica_crowd_claps_fix = false` in `ghmetallica.toml`): the claps as on
the console.
