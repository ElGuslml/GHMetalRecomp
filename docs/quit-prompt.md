# Quitting from the title screen and the main menu

**Status: working (2026-10-24).** Checked: B on "PRESS ANY BUTTON TO ROCK" and in the main menu opens the dialog, B
cancels, A quits; in Quickplay B goes back as before.

## Symptom

The game has no way out of itself (on the console the dashboard was): the window had to be closed.

## Cause

Once past the title screen, the game never goes back to it, and B does nothing in the main menu.

## Fix

On the title screen and in the main menu, B asks "Are you sure you want to quit the game?" in the game's language:
A (green fret) quits, B (red fret) cancels. The game does not see those presses. Everywhere else B works as before,
including a message box over the main menu.

`src/quit_prompt.cpp` (a `[[midasm_hook]]` in `ghmetallica_functions.toml`) looks up, once per frame, two of the
game's screen elements in its element table: the "PRESS ANY BUTTON TO ROCK" text, which only the title screen has,
and the container of the main menu (found in the game's scripts). While either exists, and no message box does,
the SDK's quit prompt is offered.

## Turning it off

Set `quit_prompt = false` in `ghmetallica.toml`, or start the game with `--quit_prompt=false`.
