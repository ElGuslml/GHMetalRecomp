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

## Songs: smarter waits (2026-10-24, later)

A song still used about 1.4 cores. The waiting threads slept, but in slices of 200 µs: each one woke thousands of
times a second, and each wake costs the kernel a timer and a context switch (the waiting threads spent more time in
the kernel than in the game). And the main thread, waiting for the job system's workers to finish a group of its
jobs, checked the queue again and again under the queue's lock, the lock the workers need to finish them; the
slower the workers woke, the longer it spun (with 1 ms slices the main thread reached 92 % of a core). So:

- a worker with an empty queue blocks on the event the game pulses when it queues a job: it wakes as soon as a job
  comes, and otherwise once a millisecond;
- the main thread waiting for its jobs watches their count between its passes, without the lock;
- the render thread sleeps in one go until shortly before the vertical blank it waits for (one comes every 1/60 s,
  or at the higher rate of [frame-rate.md](frame-rate.md); the first version assumed 1/60 s and held 120 at 67 FPS).

Measured in the career's first song (1280x720 window, NativeXRenderer, the same build with and without): 1.42 →
1.11 cores, the main thread 43 % → 13 % of a core; frame pacing unchanged (16.67 ms on average, the same longest
frames). Menus unchanged (about 0.4). `ghmetallica_wait_smart = false` goes back to the slices. Gustavo played
with it: no stutter, the songs in sync.

Left for later:
- **The job workers**, still about 10 % of a core each in a song. They now block on the job event 75 % of the time;
  the rest is each wake's own cost: the game's WaitForSingleObject(event, 0) after the wait (in the SDK a
  SwitchToThread and the object table's lock), and the queue's critical section, contended. If the event wakes all
  three workers for each job, one waiting worker would be enough.
- **NativeXRenderer's thread**, about 27 %: besides drawing, it protects the guest memory of each upload again every
  frame (VirtualProtect) to see the game's next writes there, and those writes then fault on the game's render
  thread; the dynamic data the game rewrites every frame could be compared instead of watched. A NativeXRenderer
  topic (its `docs/plan.md`).

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
the threads wait as on the console again. `ghmetallica_wait_smart = false` keeps the slices but not the smarter waits
above.
