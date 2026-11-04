# Crackling, or the sound muting with low latency audio, on slower PCs

**Status: fixed (2026-11-04), for 0.2.3.** Reproduced on the developer's PC by making the game's audio thread slower;
the player's own test is pending.

## Symptom

A player (Discord, 2026-11-04): the sound crackled all the time; with **Low latency audio** on, the crackling went
away but the sound muted itself every 15 to 30 seconds.

## Cause

The game's sound is made a frame (5.3 ms) at a time on the SDK's audio thread, which also runs FMOD's mixing
(`ghmetallica_fmod_on_audio_thread`, see [song-stutters.md](song-stutters.md)). Since the [menu music fix](menu-music.md)
each call was started at least 3 ms after the last one returned (`audio_callback_min_interval_us = 3000`), so a frame
took its work plus 3 ms. On a PC where the work takes more than about 2.2 ms, the sound can never keep up:
- the normal output takes two frames every 10 ms, so a missing one is a 5 ms gap: hundreds a second, the crackling
  (measured here with 2.2 ms of work added: 3401 gaps in 40 s, more than half the sound);
- the low latency output reads steadily, so there was no crackling, but the SDK's catch-up after a late frame
  (`audio_keep_time`) dropped the very frame the device was waiting for, played more silence for it, and owed that
  too: a silence that fed itself for seconds.

The spacing protected FMOD's own mixing thread, which no longer mixes since FMOD mixes on the audio thread.

## Fix

- `audio_callback_min_interval_us = 0`: the calls are no longer spaced. A `ghmetallica.toml` from an older version that
  still has `3000` is changed once at startup (a value chosen by hand is left alone). With 2.2 or even 4 ms of work
  added to each frame: no gap in 40 s.
- In the SDK (change 69): the catch-up drops a frame only while the device still has as many queued as it takes at
  once, so it never adds silence. A 150 ms hold-up of the audio thread now costs 138 ms of sound, once (it cost
  400 ms plus a series of small gaps).

## Turning it off

`audio_callback_min_interval_us = 3000` in `ghmetallica.toml` brings the spacing back (needed only with
`ghmetallica_fmod_on_audio_thread = false`); `audio_keep_time = false` turns the catch-up off (the sound then stays
late after a hold-up, until a pause).
