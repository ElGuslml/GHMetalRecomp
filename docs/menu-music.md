# Noise instead of music from the title screen on

**Status: fixed (2026-10-24).** Confirmed by ear: the title screen and the menus play their music.

## Symptom

The videos at startup sound right, but from the "PRESS ANY BUTTON TO ROCK" screen on, the music and the menus
play only a strange noise.

## Cause

The game plays its music and sounds through FMOD, which mixes into a tiny ring of two blocks of 256 samples.
FMOD's mixer thread checks every millisecond how far the console's audio output has read that ring, and mixes a
block when the read position has moved to the other block. On the console the output reads one block every
5.3 ms, so FMOD sees every move.

The SDK calls the game's audio output when the PC's sound device has played a frame, and Windows takes them two
at a time every 10 ms. The two blocks were read 0.1 ms apart, the position came back to the same block before
FMOD checked again, and FMOD mixed only about 13 blocks a second out of 188. The rest of the time the output
played the old contents of the ring again: the noise. The videos sound right because Bink does not go through
FMOD.

The music data itself was fine: the songs are decrypted correctly and the XMA decoder decodes them without
errors. It was read 15 times too slowly.

## Fix

A change in the SDK (`audio_callback_min_interval_us`, change 39 of its `recomp` branch): the game's audio output is called at least 3 ms apart,
as on the console, instead of twice in a row. The number of calls still follows the sound device, so the
latency is the same. On the title screen FMOD now mixes 188 blocks a second and reads each song stream 94 times a
second (48,000 samples a second), up from 13 and 7.

`src/startup.cpp` sets `audio_callback_min_interval_us = 3000` in `ghmetallica.toml`.

## Turning it off

Set `audio_callback_min_interval_us = 0` in `ghmetallica.toml`, or start the game with
`--audio_callback_min_interval_us=0`.
