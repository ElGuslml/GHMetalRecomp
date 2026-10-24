# CPU usage: five cores at 100 %

**Status: fixed (2026-10-24).** Measured with 1280x720 in a window: the main menu went from 5 cores to 0.3, a song in
Quickplay uses about 1.2, at 60 FPS; songs played without stutter.

## Symptom

With the game open the processor works hard and gets hot (70-75 °C), even in the menus: five of its cores are busy
all the time.

## Cause

Several of the game's threads wait by checking a condition again and again. On the Xbox 360 each of those threads
had a hardware thread of its own, so the checking cost nothing else; on a PC each one keeps a core at 100 %:

- the three threads of the game's job system, while they have no job;
- the main thread, at the start of each frame and while it waits for the render thread to finish one;
- the render thread, while it waits for the screen's vertical blank before showing a frame.

## Fix

`src/thread_waits.cpp` with five `[[midasm_hook]]`s in `ghmetallica_functions.toml`, one in each waiting loop. The
game's code is unchanged: a wait first checks for 100 µs as before (short waits are not slowed down), and then the
thread sleeps 200 µs between checks. The settings `ghmetallica_wait_spin_us` and `ghmetallica_wait_sleep_us` set
those times.

## Turning it off

Set `ghmetallica_wait_sleep_us = 0` in `ghmetallica.toml`, or start the game with `--ghmetallica_wait_sleep_us=0`:
the threads wait as on the console again.
