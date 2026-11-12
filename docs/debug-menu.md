# The game's own debug menu

**Status: added, off by default (2026-11-11):** `ghmetallica_debug_menu = true` in `ghmetallica.toml` (or
`--ghmetallica_debug_menu=true`). Not in the launcher.

## What it is

The game ships Neversoft's debug menu (`scripts\guitar\guitar_debug_menu.qb`). With the option on, the main menu gets a
**Debug Menu** item under Estudio de música / Music Studio, and the pause menu gets one too. It has, among others:
Unlock All, Repeat Last Song, Play Song, Skip Into Song, Character Select, Play Credits, Difficulty Analyzer, Toggle FPS
Display, Increase Band Money by 100k Credits, Bankrupt Band, Skip Career Intros, Color Calibration, Toggle Lightshow
debug, Rockstar Creator, and **Settings**: Toggle Bot (per player), Toggle Star Power, Set Hyperspeed, Show Song Time,
Show Measures, Show Song Stars, Show Camera Name, Show Input Log, Rock Meter Debug, Change Venue / Guitar / Bass,
Toggle visibility, Select Slomo, Select CameraCut, Toggle GPU Time / CPU Time, Force Score, Leftyflip, the lag tests.
Its texts are in English whatever the game's language. Some items need Neversoft's development tools (Model Viewer,
Autotesting, the replays, Check CAS Assets, Dump Heaps) and may do nothing. Unlock All, the money items and the bot's
results change the save like playing does.

## How

The menus add the item when the script global `enable_button_cheats` is 1. The game sets it to 1, then to 0 at start
on the retail disc (its script function `CD`). The same global also turned on development shortcuts that change the
save without asking: left or L3 in the setlist unlocked every song, right completed one, LB (a guitar's orange fret) on
the career's gig posters completed a song with its stars. So the global stays 0, and `src/debug_menu.cpp` (hooks in
`ghmetallica_functions.toml`) makes it read as 1 only in the three scripts that build the main menu and the pause
menu: the item is there, the shortcuts are not. The log names each script that read it ("offers the debug menu" or
"left at 0").

## Turning it off

`ghmetallica_debug_menu = false` (the default): the menus are as on the console.
