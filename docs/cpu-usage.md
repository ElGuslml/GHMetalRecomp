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

## Also: registers as local variables in the generated code (2026-10-24)

Not a bug, an optimization of the recompiled code, as in KameoRecomp, GRAWRecomp and POTCAWERecomp: with the SDK's
codegen options `non_volatile_as_local`, `cr_as_local`, `ctr_as_local`, `xer_as_local` and `reserved_as_local`
([`ghmetallica_functions.toml`](../ghmetallica_functions.toml)), the PowerPC registers that no call passes on are
local variables of each function instead of fields of a structure in memory, so the compiler keeps them in host
registers. The C runtime's `setjmp`/`longjmp` (used by the JPEG and PNG decoders linked into the game) are named for
the SDK, and the functions that call `setjmp` keep their registers in the structure.

Unlike those games, the vector registers stay in the structure (`non_volatile_vectors_as_local = false`, an SDK
option added for this game): the compiler that built Guitar Hero: Metallica passes values from a function to the
functions it calls in vector registers that the calling convention says are not used for that (a constant in `v92`
read three calls further down), and as local variables those values would be lost.

Title screen: 0.44 → 0.40 cores (mostly idle, as the menus are since the waits above). A song in Quickplay: 0.84-0.89
cores at 60 FPS.

## Turning it off

Set `ghmetallica_wait_sleep_us = 0` in `ghmetallica.toml`, or start the game with `--ghmetallica_wait_sleep_us=0`:
the threads wait as on the console again.
