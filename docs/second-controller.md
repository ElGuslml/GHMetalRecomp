# An empty screen after pressing a button on a second controller

**Status: fixed (2026-10-25).** The game shows its "not signed in" message, as on the console.

## Symptom

With two or more controllers connected (a guitar and a gamepad, say), pressing a button at the title screen ("PRESS
ANY BUTTON TO ROCK") on any controller but the first leaves an empty screen: the menus' concrete wall and nothing on
it. The game never goes on.

## Cause

The recompilation's system software has one player profile, signed in on the first controller. Each other
controller is another player with no profile. The controller that presses the button becomes the one that plays,
so the game asks that controller's player to sign in, through the console's sign-in screen.

There is no sign-in screen here. Instead, the SDK reported at once that the first player's sign-in had changed and
that the screen had closed (an old workaround for other games, kept from Xenia). The game waited for the second
controller's profile, which never came.

## Fix

`src/startup.cpp` sets the SDK option `signin_ui_cancel = true` in `ghmetallica.toml`: the sign-in screen opens and
closes as when the player cancels it on the console, without a sign-in. The game then shows its own message: "You
are not signed in to a player profile. You will not be able to save any game content until you sign in", with
**Sign in** (it asks again, and comes back to the message) and **Continue without saving** (the main menu,
playing with that controller and saving nothing).

To play with your profile and its saves, press the button at the title screen on the first controller. The first
controller is the one Windows connected first; the log says which one is in which slot ("New controller connected
to slot 0").

## Planned: one profile for the computer, the other controllers its guests

Not done yet. The aim is to always play with the same profile, whichever controller presses the button:
- the other controllers signed in as guests of the profile, as on the Xbox 360 ("Gus (1)", "Gus (2)"), so the game
  does not ask them to sign in;
- the profile follows the controller that presses the button at the title screen: that controller becomes player 1,
  with the profile and its saves, and the others its guests. Guests alone would not do it: on the console a guest
  who starts the game plays as a guest and does not save to the profile.

Both are changes to the SDK's system software (its single profile and the controllers' slots), not to the game.

## Turning it off

Set `signin_ui_cancel = false` in `ghmetallica.toml`, or start the game with `--signin_ui_cancel=false`: the SDK's
old notifications, and the empty screen when a second controller presses the button.
