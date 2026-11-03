# The keyboard playing guitar taken for the singer

**Status: fixed (2026-11-02), waiting for a player's test.** A player's kind of controller (a guitar, the drums, the
singer) follows the controller they play with, outside a song.

## Symptom

A player reported the keyboard set to play guitar (GH Metallica Launcher, Controls -> Keyboard -> Instrument:
Guitar) and the microphone off, yet the game made the keyboard's player the singer.

## Cause

The game asks each controller for its kind once, when it connects (and again when a button is pressed at the title
screen): a guitar plays guitar or bass, the drums the drums, and a plain gamepad (or a dance pad) the vocals, which
need a microphone. The keyboard is not a player of its own: it shares player 1 with the first controller connected,
and the SDK answers for whichever of the two was used last. At the start nothing has been used, so the controller
answers. With that controller set to sing (Controls -> its page -> Instrument: Vocals), or of a kind the game gives
the vocals, player 1 stayed the singer when the keyboard played. A controller whose stick or trigger rests away
from its centre (pedals, a throttle) also counted as used at every read, so it kept answering for player 1 whatever
the keyboard did.

## Fix

- The SDK counts a controller as used when a button is pressed or a stick or trigger leaves its rest, not while it
  merely rests off centre; and it notes when player 1 is played on another kind of controller than the game was told
  of. The log says so: "Slot 0 is played on another kind of controller now".
- `src/players.cpp` (a hook on the game's update of each controller port) then makes the game ask that port for its
  kind again, outside a song (the note highway shown, its pause menu included); the port's input is not read for two
  frames. The log: "[players] slot 0: the game asks for its kind of controller again".
- The same keeps the kinds right when a button at the title screen moves a controller to player 1
  (docs/second-controller.md): the game takes no notice of the system's "controllers changed" notification.

Checked with a DualSense set to sing and the keyboard playing guitar: at the start the game was told player 1 is a
gamepad (the DualSense's kind, the singer); the keyboard's first key made it ask again and get a guitar. Unit tests in
the SDK (`tests/unit/input/input_assignment_test.cpp`) cover both rules.

## Turning it off

`ghmetallica_kind_follows_controller = false` in `ghmetallica.toml`: each player keeps the kind the game was told
when their controller connected.
