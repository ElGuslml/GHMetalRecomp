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

### A guitar whose frets cannot be assigned, or whose whammy bar is always pressed (2026-10-29)

Reported with CRKD guitars, whose Xbox 360 mode is an XInput controller (vendor 0x3651 or 0x0351), not yet confirmed
with one:

- **Assigning frets in GH Metallica Launcher.** While it waited for a button the launcher also took a trigger pressed
  past half. An Xbox 360 Guitar Hero guitar's accelerometer is on its triggers, at half when still, so moving the
  guitar assigned a trigger before the fret was pressed. Now a trigger counts only if it was released when the wait
  began and is then pressed past 90 %, and only the controller being edited is listened to (RecompLauncher 3.4).
- **Assignments per controller.** They were kept per type of controller, so a guitar and an Xbox 360 controller (both
  SDL's `xbox360`) shared one layout: swapping a guitar's yellow and blue swapped the controller's Y and X. They are
  now each controller's own, by its serial number or GUID (the SDK's `gamepad_device_remap`, change 59).
- **The whammy bar always pressed.** A guitar that reports itself as a gamepad rests its bar at -32768, which change 57
  read as the stick pushed all the way. The SDK now judges where the right stick's X rests (still for a second, the
  first time): at -32768 it is a bar and keeps its value; the log says which (`right stick's X rests at`). And the
  launcher's Controls page has a Whammy bar row on each guitar and each controller playing guitar: how far the game
  reads it now, and a calibration (push the bar all the way and hold it, then let it go) for a bar that rests or moves
  elsewhere: the axis, its value at rest and all the way (`gamepad_whammy_calibration`, change 60).

### CRKD guitars, and a whammy bar on another axis (2026-11-05)

A player of a CRKD Gibson Les Paul (Multi Platform) reported no whammy bar, or yellow and blue swapped, and a launcher
that did not react to the guitar on its P3 setting. Not tried with a CRKD guitar yet: the fixes were checked with
SDL's virtual controllers, and which USB IDs the guitar reports on each setting is expected, not read from a log.

- **Any controller in GH Metallica Launcher.** The launcher kept only the first controller open, and SDL reports only
  an open controller's input: on any other the Controls page assigned nothing and the whammy bar read 0, so calibrating
  it always ended in "no movement". SDL lists HIDAPI controllers first, so with a DualSense connected a guitar was never
  the first. Every controller is open now, and the last one that pressed a button navigates (RecompLauncher 3.12).
- **A guitar locking the launcher.** After a button was assigned the launcher waited for all of them to be up, and
  counted the triggers: an Xbox 360 guitar's tilt rests its right trigger at half, so the controller stayed locked until
  the launcher closed. The triggers no longer count; they press nothing there.
- **The P3 setting.** A PS3 Guitar Hero guitar is USB 12BA:0100, the ID that console's games look for, so the guitar
  should report it there. That protocol has yellow on Square's bit and blue on Triangle's: SDL reads them as a
  DualShock 3's, so the game saw them swapped. Its bar rests at the middle of the axis, which the game reads as half
  pressed. The SDK puts both right (`guitar_ps3_protocol`, change 71, set in `src/startup.cpp`), and the log says so.
  Tilt is not on that protocol's axes, only on its accelerometer: Star Power is on Back (View) unless Tilt is assigned
  (below).
- **The whammy bar's row.** It now says Assign and shows the axis in use: push the bar all the way, hold it, let it go,
  and any axis of the controller is the bar, which a guitar with sticks of its own needs if its right stick is not the
  bar (CRKD's Les Paul has both). The launcher's log says what it found.

### Tilt that does nothing (2026-11-05)

A player reported that tilting a Wii guitar gave no Star Power. Not tried with one yet: checked with SDL's virtual
controllers. The game reads tilt from the right stick's Y (an Xbox 360 guitar's accelerometer) and asks for its very end:
`star_tilt_threshold` in its scripts is -1.0, which is the axis at 98 % (32256 of 32767). An Xbox 360 guitar's sensor
saturates well before that. A Wii guitar through WiitarThing reaches the PC as an Xbox 360 controller whose tilt axis
ends with the neck straight up, so at the angle anyone plays it never gets there. On a PS3 guitar tilt is on no axis at
all.

GH Metallica Launcher's Controls page has a Tilt row under the Whammy bar's, on each guitar and each controller playing
guitar: how far the game reads it, and Assign: tilt the neck up as for Star Power and hold it, then go back to your
playing position. Any axis of the controller, or its accelerometer, becomes the right stick's Y: 0 at rest and the end of
the axis a fifth short of the position held, so Star Power comes before you reach it (`gamepad_tilt_calibration`, change
72 of the SDK; the launcher's log says what it found). A guitar whose tilt is a switch shows as a button: assign that
button to Star Power instead.

## Turning it off

Set `gamepad_subtype = 0` in `ghmetallica.toml`, or start the game with `--gamepad_subtype=0`: a controller is then
the singer again. The keyboard's strum on each fret: the Keyboard page's switch, or `mnk_tap_target = ""`; a
controller's: its switch on the Controls page, or `gamepad_tap_target = ""`. The whammy bar at rest on a controller's
stick: `gamepad_whammy_rest = false`. A PS3 guitar's frets and bar: `guitar_ps3_protocol = false`. An assigned
whammy bar or tilt: X (or Delete) on its row's Assign, or Reset.
