# Profiles shared by the launchers

**Status: working (2026-10-26), GH Metallica Launcher 2.4.0.** The player's profile, its saves, settings and
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
- The profile's card at the bottom of the launcher's sidebar opens its menu: the profile's page, editing it,
  switching to another profile (for every launcher) and creating one. The profile's page shows the level (one XP per
  Gamerscore point, 100 XP a level), the achievements of every recompilation whose launcher ran on this computer, and
  a showcase of up to three pinned achievements. Editing changes the name, a motto, the avatar or photo and the
  color; a profile other than the last one can be deleted (to the Recycle Bin, with its saves).
- The time played counts from Play to the game's exit, only when the game starts from its launcher (not the
  computer's sleep): hours per game, the streaks of days with play and the week's activity on the profile's page.
- Play or Continue on another game's card closes this launcher and opens that game's.
- The launcher's logo and each game's tile in the profile's pages show the game's own icon, the one the Xbox 360
  dashboard shows (launcher 2.5.0), once the game is installed; before that, or for a game whose launcher has not run
  with 2.5.0 yet, the game's mark as before.

## How it works

Two options of the SDK (changes 47 and 49 of its `recomp` branch): `profile_root`, the profile's folder (the saves,
the settings in each game and the unlocks go there), and `profile_name`, the name the game gets. The game sets them
at start from the launchers' current profile (`src/startup.cpp`, `ConfigureProfile`, with RecompLauncher's
`profile_store.h`). Every profile has the same Xbox 360 user ID, so a save works in any profile.

Each launcher writes its game into `%APPDATA%\ElGuslml Recomps\games\<title ID>\` (`game.toml`: name, color, mark,
the launcher's path, the version, when it last ran; `icon.png`, the game's icon; and a copy of the game's achievement
list), which is how the other launchers show it. The icon is never carried by the launcher: it is read from the
player's game files, the disc's `nxeart` package (the dashboard's art for the game, whose header has the icon), or,
for a disc without it, the icon the game writes with its achievement list
(`userdata\achievements\<title ID>\title.png`, the SDK's change 52: the executable's own, the same picture). The
playtime is in each profile's `games\<title ID>.toml` (seconds, sessions, seconds per day), written by the game's
launcher each minute while the game runs.

## Turning it off

Start the game with `--profile_root=<folder>` to use another folder. Without the `%APPDATA%\ElGuslml Recomps\`
folder the game uses its `userdata\` folder as before.
