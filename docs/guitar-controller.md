# Playing with a controller or the keyboard as a guitar

**Status: working (2026-10-24).** Tested in Quickplay with a DualSense and with the keyboard's emulated controller:
Guitar and Bass are offered and the songs play; an Xbox 360 guitar and the keyboard's keys (below) played by Gustavo
(2026-10-25). The whammy bar at rest on a controller's stick and the keyboard's, and the strum on each fret on a
controller: confirmed by Gustavo (2026-10-28).

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
| Whammy bar | Right stick, to either side | Right stick, to either side |
| Star Power | Back (View) | Create |
| Start | Start (Menu) | Options |

An Xbox controller's buttons have the frets' colors. A real guitar or drum kit keeps its own kind.

`src/startup.cpp` sets `gamepad_subtype = 6` in `ghmetallica.toml`. 8 makes the controller a drum kit.

### The keyboard (2026-10-25)

The keyboard's default keys are made for the guitar (`src/startup.cpp`, and GH Metallica Launcher's Keyboard page,
whose "Reset to defaults" gives them back):

| Guitar | Keys |
|---|---|
| Green, Red, Yellow, Blue, Orange frets | D, F, J, K, L |
| Strum | Up / Down arrows |
| Star Power | Space or Backspace |
| Start | Enter or X |
| Whammy bar | Right arrow |

The sticks' other keys are off (W A S D, F and K would move them with the frets).

**Strum when a fret is pressed** (the Keyboard page's switch, off by default): each fret key strums as it is pressed,
so the arrows are not needed. Frets pressed within 20 ms of each other count as one chord, one strum, even when they
are not pressed at exactly the same time; a fret pressed later strums again. The strum comes about 20 ms after the
first fret (the time to wait for the rest of a chord). It works only during a song: in the menus the strum (the
D-pad) moves the cursor, and each fret pressed to choose or go back would move it too.

It is the SDK's `mnk_tap_*` options (change 45 of its `recomp` branch): `mnk_tap_target = "dpad_down"` turns it on
(the launcher's switch), `mnk_tap_sources` are the frets, `mnk_tap_window_ms` (20) the chord's window and
`mnk_tap_hold_ms` (30) how long the strum is held. `src/keyboard.cpp` tells the SDK when a song is running (a hook on
the players' update, `sub_82137F18`, which runs every frame of a song).

### The whammy bar half pressed until it moves (2026-10-25)

With an Xbox 360 guitar the whammy bar acted half pressed on every sustain from the start of a song until the player
moved it once. The bar is the right stick's X axis, -32768 at rest, and the game measures it from a rest at -0.76 of
the axis (`resting_whammy_position`): the axis at 0 is the bar at 43 %. The guitar reads 0 there until the bar first
moves, and SDL reports an axis only when it changes. A change in the SDK (changes 46 and 48 of its `recomp` branch,
`guitar_whammy_rest`, set in `src/startup.cpp`): a guitar's bar is at rest until the guitar reports a position; the
log says when (`whammy bar reported its first position`). Off in the SDK by default: in a game without guitars a
guitar's bar at rest is its right stick held left.

### The controller's frets strum as they are pressed (2026-10-28)

The keyboard's switch for a controller too: GH Metallica Launcher's Controls page has "Strum when a fret is pressed"
on each controller that plays guitar (not on a real guitar, a drum kit or a controller set to sing), one switch for
all of them, off by default. The same timing and the same rule: only during a song. The SDK's `gamepad_tap_target`
(the launcher writes `"dpad_down"` or `""`) and `gamepad_tap_sources` (`a,b,y,x,lb`, `src/startup.cpp`), change 57 of
its `recomp` branch; `src/keyboard.cpp`'s song gate serves both.

### The whammy bar half pressed with a controller or the keyboard (2026-10-28)

With a controller or the keyboard the whammy bar acted 43 % pressed on every sustain: the same rest at -0.76 of the
axis as above, and a centred stick is 0. A change in the SDK (change 57, `gamepad_whammy_rest`, set in
`src/startup.cpp`): a controller or the keyboard played as a guitar has its bar at rest while the right stick is
centred, and fully pressed with the stick pushed to either side (past a tenth of its travel, where a stick may drift).
The keyboard's Right arrow presses it fully. With a controller and a real guitar, or the keyboard, in one player's
hands, the bar pressed furthest counts.

## Turning it off

Set `gamepad_subtype = 0` in `ghmetallica.toml`, or start the game with `--gamepad_subtype=0`: a controller is then
the singer again. The keyboard's strum on each fret: the Keyboard page's switch, or `mnk_tap_target = ""`; a
controller's: its switch on the Controls page, or `gamepad_tap_target = ""`. The whammy bar at rest on a controller's
stick: `gamepad_whammy_rest = false`.
