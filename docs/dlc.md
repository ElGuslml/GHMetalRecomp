# Downloadable content (DLC)

**Status: working (2026-10-25).** Metallica's *Death Magnetic* album, from the player's own Xbox 360 package, plays in
Quickplay. GH Metallica Launcher installs packages from its DLC page.

## What the game reads

The game looks for downloadable content through the system software (`XamContentAggregateCreateEnumerator`, which it
resolves by ordinal at run time) and mounts each package it lists. The recompilation's system software lists the
folders in

    userdata\0000000000000000\41560830\00000002\<package name>\

where each package is the extracted contents of an Xbox 360 content package (an STFS file: LIVE, PIRS or CON).

Checked: the *Death Magnetic* package made for **Guitar Hero III** (title 415607F7, "Metallica 'Death Magnetic'
Album", 1.4 GB). The game lists its 10 songs in Quickplay's song list (the blue button: downloaded content), under
"Death Magnetic downloadable content", and plays them. "All Nightmare Long" is in the game's own list. On the console,
the songs bought for Guitar Hero III or World Tour were playable in this game too.

## Installing

- **GH Metallica Launcher, DLC page:** "Choose folder…" (or a folder dropped on the window) looks for packages in the
  folder and its subfolders, shows each one's name, the game it was made for and its size, and installs the new ones.
  The player's files are not moved. Installed content is listed with a button to remove it.
- **By hand:** "Open the DLC folder" opens the folder above; copy a package's extracted files into a folder of their
  own there.

The launcher installs a package with the game itself: `GHMetallicaRecomp.exe --install_content=<package>` loads the
game's executable without a window (for its title ID), extracts the package into the folder above with a header that
holds its name (the SDK's `ContentManager::InstallContent`), and quits. The package's own title ID is not checked, as
the *Death Magnetic* one shows.

## Turning it off

Remove the package from the DLC page, or delete its folder.
