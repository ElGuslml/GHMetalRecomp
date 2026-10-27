# An empty screen after pressing a button on a second controller

**Status: fixed (2026-10-25), and since 2026-10-26 the profile follows the controller that starts the game.** The
controller that presses a button at the title screen plays with the profile; the others are its guests.

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

## One profile for the computer, the other controllers its guests (2026-10-26)

Whichever controller presses the button at the title screen now plays with the profile and its saves, as on the
console, where the controller that starts the game signs in with its own profile:
- **The profile follows that controller.** While the title screen is shown (`src/players.cpp`, from the frame hook
  that finds the title's text, `src/quit_prompt.cpp`), the SDK swaps a controller that presses a button with the first
  one before the game sees the press: the game sees it on player 1's controller, the profile's, and goes straight to
  the main menu. The log says so: "The controller in slot 1 pressed first: it is now slot 0, the profile's". The
  keyboard always goes with the profile.
- **The other controllers are its guests** (`signin_guests = true` in `ghmetallica.toml`), as when guests sign in on
  the Xbox 360: signed in, named after the profile ("Gus (1)", "Gus (2)"), with the game's default settings, which
  are not kept, and no saves of their own (the SDK refuses to create or delete content for them, as every user's
  content here is the profile's). The game knows them as guests (it reads the guest flag of `XUserGetSigninInfo`).

Checked with the keyboard's pad made a second controller (a temporary SDK change, removed) and the guitar as the
first: Enter on the keyboard at the title screen -> the main menu with the profile, no sign-in question. With the
swap off, a guest who starts the game is offered the sign-in screen, which closes as cancelled, and goes on to the
main menu as a guest (without the swap and without guests: the "not signed in" message, as before).

## Turning it off

- `--ghmetallica_profile_follows_press=false`: the first controller connected plays with the profile, as before.
- `signin_guests = false` in `ghmetallica.toml` (or `--signin_guests=false`): the other controllers have no profile,
  and the game shows its "not signed in" message when one starts the game.
- `signin_ui_cancel = false`: the SDK's old notifications, and the empty screen when a second controller presses
  the button.
