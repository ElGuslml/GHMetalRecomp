# Low latency audio

**Status: added (2026-10-30), off by default: the launcher's Settings > Game > Low latency audio.**

## Symptom

The sound comes late: players set about 75 ms of audio calibration.

## Cause

The game's sound goes to the SDK's queue (4 frames of 5.3 ms, 21 ms), then SDL hands it to Windows' audio engine in
shared mode, which mixes every program's sound for the device. Measured on a Focusrite Scarlett 2i2 (a USB interface),
from the game submitting a frame to the driver playing it: 77 ms, 57 of them past the SDK's queue. A motherboard's
or a monitor's output takes about 20-25 ms less on the shared path (`local_docs/14`).

## Fix

**Low latency audio** in GH Metallica Launcher (Settings > Game; the SDK's `audio_wasapi_exclusive`, recomp change 64):
the game takes the output device in exclusive mode (WASAPI), past Windows' audio engine, keeps 10 ms in the device's
buffer (`audio_wasapi_exclusive_buffer_ms`) and queues 2 of its frames instead of 4 (`audio_wasapi_exclusive_queue`).
Other programs (music, Discord) cannot play on that device while the game runs; when the device is busy or refuses,
the usual shared output is used (the log says which).

| Scarlett 2i2, a song | Submit to played |
|---|---|
| Shared output (before) | 76.9 ms |
| Exclusive, 4 frames queued | 30.6 ms |
| Exclusive, 2 frames queued (the game's setting) | 20.1 ms |

No gap in whole sessions, the song in sync. Calibrate the audio again after turning it on or off: about 55 ms less on
this device.

The log: `audio_wasapi_exclusive: '<device>' in exclusive mode, 48 kHz stereo 24-bit, 10.0 ms kept in a 39.0 ms
buffer`. If it says the device's buffer ran empty (a click), raise `audio_wasapi_exclusive_buffer_ms` in
`ghmetallica.toml`.

## Turning it off

The launcher's row (off by default), or `audio_wasapi_exclusive = false` in `ghmetallica_options.toml`.
