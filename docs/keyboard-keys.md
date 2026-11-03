# Shift, Ctrl, Alt and the F keys on the keyboard's controls

**Status: added (2026-11-02), waiting for a player's test.** Players asked for Alt as Star Power and for F1-F12 on
their controls.

## Before

- Shift, Ctrl and Alt only worked held with another key ("Shift+Up"). The launcher's Keyboard page refused one alone:
  a bind made of a modifier alone could never fire, and holding one silenced every other key ("Up" stays silent while
  Shift is held, which is how the D-pad shares the arrow keys).
- F1-F12 could be assigned, but F3 (the debug overlay), F4 (the settings window) and ` (the console) kept opening the
  SDK's overlays: a fret on F4 opened the settings window mid-song, which then took the keyboard.

## Now

- **Modifiers as keys.** On the Keyboard page, press and release Shift, Ctrl or Alt alone: that side's key is
  assigned ("Left Alt", saved as `LAlt`; also `RAlt`, `LShift`, `RShift`, `LCtrl`, `RCtrl`). The SDK (recomp change
  67) then takes it for that control's key and not a modifier: Left Alt held for Star Power leaves the frets as they
  are. The other side's key is still a modifier, so a bind such as "Shift+Left" keeps working with it (the page says
  so when such a bind exists).
- While a Shift key is assigned alone and the game has focus, Windows' Sticky Keys and Filter Keys shortcuts (Shift
  five times, the right Shift held eight seconds) are held off, unless you use those features, and given back when
  the game loses focus or closes. While an Alt key is assigned alone, Alt+F4 does not close the game.
- **Debug keys off.** `debug_hotkeys = false` (`src/startup.cpp`; the launcher's Settings > Game > Debug keys, off by
  default): F3, F4 and ` do nothing in the game. F7 still shows the achievements.
- **A key on a control only presses it** (recomp change 68), with the debug keys on too, F7 included. Turning the
  debug keys on names the keys that are on controls ("F4 (Yellow fret)").

Checked in the game (keys sent as a keyboard sends them): Left Alt on a control, held, with Down in the main menu, the
cursor moved; Right Alt held, it did not (still a modifier). F4 on the orange fret opened nothing; F3 opened the debug
overlay with the debug keys on. The log: "Keyboard: RShift, LAlt bound alone ...", the shortcuts held off, and given
back at the exit. Unit tests in the SDK (`tests/unit/input/mnk_binds_test.cpp`).

## Turning it off

Nothing changes until a modifier is assigned alone. `debug_hotkeys = true` in `ghmetallica_options.toml` (or the
launcher's row) brings back F3, F4 and `.
