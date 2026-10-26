# Profiles shared by the launchers

**Status: working (2026-10-26), GH Metallica Launcher 2.2.0.** The player's profile, its saves, settings and
achievements are shared by the launchers of these recompilations (Guitar Hero: Metallica, Kameo, Pirates of the
Caribbean: At World's End, Ghost Recon Advanced Warfighter) and live outside the game's folder.

## What changes

Before, each game had one profile, "User", and kept its saves and achievements in its own `userdata\` folder (or
`Documents\ghmetallica\` when the game's folder could not be written).

Now:
- The launcher asks for a profile the first time it opens: a name (15 characters at most, as an Xbox 360 gamertag:
  the game shows it whole, without accents), an avatar or a photo, a color.
- The profiles are in `%APPDATA%\ElGuslml Recomps\`, the same for every launcher. Each profile's folder holds its
  saves, its settings in each game and its achievement unlocks. Downloadable content, caches and the launcher's
  settings stay with the game.
- The saves and achievements the game had before are copied into the profile the first time (each file checked). The
  game's `userdata\` keeps them as they were, with a note, `SAVES MOVED.txt`, that says where they went. If the
  profile already has saves of this game (from another installation), the launcher asks which to keep; the others are
  kept in `%APPDATA%\ElGuslml Recomps\backups\`.
- The game runs with the current profile, also when started without the launcher.

## How it works

Two options of the SDK (changes 47 and 49 of its `recomp` branch): `profile_root`, the profile's folder (the saves,
the settings in each game and the unlocks go there), and `profile_name`, the name the game gets. The game sets them
at start from the launchers' current profile (`src/startup.cpp`, `ConfigureProfile`, with RecompLauncher's
`profile_store.h`). Every profile has the same Xbox 360 user ID, so a save works in any profile.

## Turning it off

Start the game with `--profile_root=<folder>` to use another folder. Without the `%APPDATA%\ElGuslml Recomps\`
folder the game uses its `userdata\` folder as before.
