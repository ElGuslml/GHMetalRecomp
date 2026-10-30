# The music falls behind the notes after stutters

**Status: fixed (2026-10-30).**

## Symptom

Players reported that when the game stutters during a song, the song drifts out of sync, a little more at each
stutter, until a pause puts it back.

## Cause

The notes follow real time: a stutter shorter than 1.7 s costs them nothing. The music can lose time in two places,
measured by holding up one thread at a time on purpose during a song (`local_docs/22`):

- FMOD, the game's sound engine, mixes 256-sample blocks into a ring of two, from a thread that polls about every
  millisecond how far the sound output has read. When that thread comes more than one block (5.3 ms) late, the reader
  has gone round the ring and the blocks in between are never mixed: the song's tracks stay behind by them. Held up
  8 ms once a second: 0.35 s behind after 54 s.
- The SDK's audio thread, which runs the game's sound callback for each frame the device plays: held up longer than
  its 21 ms queue, the device plays silence, and the song plays that much later from then on. 30 ms once a second:
  0.8 s behind.

The main and render threads, the usual causes of a visible stutter, cost the music nothing. Something that holds up the
audio threads on a player's PC (a busy CPU, a driver) makes both a stutter and the drift.

## Fix

- FMOD mixes each block on the SDK's audio thread right after the device's frame reads the previous one, so its own
  thread no longer has to come in time (`ghmetallica_fmod_on_audio_thread`, `src/fmod_mix.cpp`).
- The silence played while the game's sound came late is made up by dropping as many of its next frames: a short skip
  instead of a lasting delay (`audio_keep_time`, the SDK's recomp change 62).
- The SDK's audio threads run in Windows' "Pro Audio" multimedia class, as the sound device's own thread does
  (`audio_thread_priority`, recomp change 63).

The same song, 54 s, a thread held up on purpose (how far the music ended behind the notes):

| Held up | Before | After |
|---|---|---|
| Nothing | -0.2 ms | +1.0 ms |
| FMOD's thread, 150 ms every 5 s | +1495 ms | +1.2 ms |
| FMOD's thread, 8 ms every second | +354 ms | +1.5 ms |
| The audio thread, 150 ms every 5 s | +1350 ms | +70 ms |
| The audio thread, 30 ms every second | +819 ms | +0.6 ms |

## Turning it off

In `ghmetallica.toml`: `ghmetallica_fmod_on_audio_thread = false`, `audio_keep_time = false`,
`audio_thread_priority = false`.
