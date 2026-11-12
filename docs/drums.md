# Drums on the keyboard or a controller, five or four lanes

**Status: added (2026-11-11), launcher 3.19.1:** Controls → a controller or Keyboard and mouse → Instrument: Drums, then
Drum kit: 5 lanes (Guitar Hero) or 4 lanes (Rock Band). Not yet played through a song with them.

## How the game tells the kits apart

The game asks each controller's kind with `XInputGetCapabilities` (`sub_8239D688`): subtype 8 is a drum kit, its
type 11. At the controller's first state read (`sub_8239D890`) a kit becomes type 10, Guitar Hero's five-lane kit,
unless its capability flags have bit 0 (or RB is held): then it stays 11, a Rock Band kit. `UseFourLaneHighway`
(`sub_82189AA0` / `sub_82189930`) plays type 11 on the game's own four-lane highway (`four_lane_highway`, its 4-lane
gems and HUD), as a Rock Band kit did on the console. The pads come from B (red), Y (yellow), X (blue) and A (green),
the orange cymbal from RB on five lanes only (`sub_82148980`), the kick pedal from LB; Star Power on Back.

## What the port does

- The SDK's `gamepad_device_subtypes` (change 87) takes capability flags after the subtype: `keyboard=8` is a
  five-lane kit, `keyboard=8:1` a four-lane one; a controller by its serial number or GUID the same.
- The launcher's Instrument row writes it (Guitar, Drums, Vocals), and its Drum kit row the lanes. A real drum kit
  gets the Drum kit row too (4 lanes names it with `8:1`).
- The keyboard has drum keys of its own (the guitar's stay as they were): five lanes D red, F yellow, J blue,
  K orange, L green, Space the kick, Backspace Star Power; four lanes K green and no orange. The menus keep the
  guitar's keys (arrows, Enter).

## Turning it off

Instrument: Guitar (the entry leaves `gamepad_device_subtypes`).
