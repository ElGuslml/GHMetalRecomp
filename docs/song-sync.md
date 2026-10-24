# The audio drifts out of sync during a song

**Status: fixed (2026-10-24).** After calibrating again (75 ms audio, 0 ms video on the developer's machine), whole
songs stay in sync without pausing.

## Symptom

A song starts in sync with the notes (after calibrating the game in its options), then the audio slowly falls behind
until it is clearly out of sync. Pausing and resuming puts it back. The same happens on the later Xbox 360 models
(the Corona and Waitsburg boards).

## Cause

The game counts its time with the processor's timebase, divided by the timer frequency the system reports. The
original Xbox 360 reports 50,000,000 Hz while its timebase really runs at 49,875,000 Hz, and the game corrects it:
when the system says 50,000,000 it uses 49,875,000. On a console whose timebase does run at 50 MHz, and in this
recompilation, that correction makes the game's clock run 0.25 % fast: the notes move ahead of the music, which
plays at the sound card's real rate, by about 150 ms a minute. On resuming a song the game seeks the music to its
clock, which is why a pause puts them back together.

The music itself plays without gaps (measured over a whole song).

## Fix

`src/timer.cpp` with a `[[midasm_hook]]` in `ghmetallica_functions.toml`: the game keeps the 50,000,000 Hz the
system reports, the rate its timebase runs at. No change to the song files is needed.

Also, `src/startup.cpp` sets `audio_maxqframes = 4` in `ghmetallica.toml`: the sound waits 21 ms less before
reaching the speakers (the SDK's default queue is 8 frames of 5.3 ms). Calibrate again after this change.

## Turning it off

Set `ghmetallica_timer_fix = false` in `ghmetallica.toml`, or start the game with `--ghmetallica_timer_fix=false`
(the original code). The queue: `audio_maxqframes = 8`.
