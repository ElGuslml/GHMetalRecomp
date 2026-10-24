# Playing with a controller or the keyboard as a guitar

**Status: working (2026-10-24).** Tested in Quickplay with a DualSense and with the keyboard's emulated controller:
Guitar and Bass are offered and the songs play. Not checked yet: the whammy bar with a controller's stick, and the
keyboard's default keys (below).

## Symptom

With a controller or the keyboard, Quickplay stops at "You must connect a microphone or Xbox 360 Headset to
continue", so no song can be played.

## Cause

The game asks each controller what kind it is and gives each kind its part: guitars play the guitar or the bass,
drum kits the drums, and an ordinary controller the vocals, which need a microphone.

## Fix

A change in the SDK (change 41 of its `recomp` branch, `gamepad_subtype`): a controller, and the keyboard's emulated
one, are reported as a guitar. A guitar uses the same controls as an Xbox 360 controller, so the game reads it as
it is:

| Guitar | Xbox controller | PlayStation controller |
|---|---|---|
| Green, Red, Yellow, Blue frets | A, B, Y, X | Cross, Circle, Triangle, Square |
| Orange fret | LB | L1 |
| Strum | D-pad up / down | D-pad up / down |
| Whammy bar | Right stick, sideways | Right stick, sideways |
| Star Power | Back (View) | Create |
| Start | Start (Menu) | Options |

An Xbox controller's buttons have the frets' colors. A real guitar or drum kit keeps its own kind.

`src/startup.cpp` sets `gamepad_subtype = 6` in `ghmetallica.toml`. 8 makes the controller a drum kit.

The keyboard's default keys are the SDK's, made for a gamepad (A on Semicolon or Space, the D-pad on Shift and the
arrows): they can be changed in GH Metallica Launcher's Keyboard page; keys made for the guitar are planned.

## Turning it off

Set `gamepad_subtype = 0` in `ghmetallica.toml`, or start the game with `--gamepad_subtype=0`: a controller is then
the singer again.
