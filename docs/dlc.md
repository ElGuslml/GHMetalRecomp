# Downloadable content (DLC)

**Status: working (2026-10-25; the DLC folder 2026-11-10).** Metallica's *Death Magnetic* album, from the player's own
Xbox 360 package, plays in Quickplay. Since 0.4.1-beta.1 the game reads the package where it is, in the player's DLC
folder; before, GH Metallica Launcher extracted it.

## Your DLC folder (0.4.1-beta.1)

GH Metallica Launcher's DLC page takes the folder where the player keeps their packages (STFS files: LIVE, PIRS or CON,
in it and its subfolders), as many games' as they like; the other ElGus Recomps launchers can use the same folder. The
launcher reads each package's header and the names at its root, checks it against this game's rule (below, "Packages
of other games"), and writes the game's list, `userdata\0000000000000000\41560830\content_packages.txt`, with the
packages on. The game (the SDK's change 80) lists those packages as its downloadable content and reads them from their
files: nothing is copied, written or deleted. The page groups the packages by the game they were made for, each with a
switch (a group's switch turns all of it); those made for another game come turned off. The folder and the player's
choices are in `dlc_library.txt` beside the list; the folder is read again each time the launcher opens (`kDlcMode =
kFolder` in `launcher/game.h`, launcher 3.17).

Packages installed by an earlier launcher (extracted into `userdata\0000000000000000\41560830\00000002\<package>\`) keep
working and show as "Installed copies". One that is also in the DLC folder (the same file name, or the same name and
size) shows once, as the copy the game reads; "Remove copies" deletes those copies after a confirmation, and the
folder's packages take their place.

## What the game reads

The game looks for downloadable content through the system software (`XamContentAggregateCreateEnumerator`, which it
resolves by ordinal at run time) and mounts each package it lists: the DLC folder's packages that are on, and the
folders extracted by earlier launchers in

    userdata\0000000000000000\41560830\00000002\<package name>\

Checked: the *Death Magnetic* package made for **Guitar Hero III** (title 415607F7, "Metallica 'Death Magnetic'
Album", 1.4 GB). The game lists its 10 songs in Quickplay's song list (the blue button: downloaded content), under
"Death Magnetic downloadable content", and plays them. "All Nightmare Long" is in the game's own list. On the console,
the songs bought for Guitar Hero III or World Tour were playable in this game too.

## Installing, before 0.4.1-beta.1

GH Metallica Launcher's DLC page extracted each package with the game itself: `GHMetallicaRecomp.exe
--install_content=<package>` loads the game's executable without a window (for its title ID), extracts the package
into the folder above with a header that holds its name (the SDK's `ContentManager::InstallContent`), and quits. The
package's own title ID is not checked, as the *Death Magnetic* one shows. By hand, a package's extracted files in a
folder of their own there still work. `kDlcMode = kInstall` in `launcher/game.h` brings that page back.

## Many packages: "Disc Read Error" at start (fixed, SDK change 78)

Found in GH Warriors of Rock (2026-11-09), whose collection of 190 packages showed the "Disc Read Error" dialog a few
seconds after starting. The game lists its content one package at a time with the XDK's asynchronous `XEnumerate`,
which ends each call with `ObDereferenceObject` on the enumerator. The SDK looked that object up through a handle it
keeps in a dispatch header, which an enumerator does not have: after the first call it released whatever object had
taken a freed handle, sometimes a file the game had just opened, and the game's next read of the disc failed. Now the
object is found by its guest address. Every game uses this path; more packages only made it likelier.

## Packages of other games: turned off (launcher 3.15.0)

The game's DLC script (`5A4B6A0C`) looks for one file in the content folders, `adl1.pak`, Death Magnetic's manifest;
a package without it (World Tour's `adl2`, Guitar Hero 5's `bmanifest_*`) is passed by. Nothing harmful here: in Guitar
Hero 5 such a package made the game report a damaged package and turn off every DLC (its `docs/dlc.md`). GH Metallica
Launcher 3.15.0 turns off a package without `adl1.pak.xen` (`kDlcRule` in `launcher/game.h`), with the note "Not made
for this game: it adds nothing here": its folder moves to `userdata\0000000000000000\41560830\Disabled DLC`, where the
game does not look. Each package has a switch; one turned on by the player stays on.

With the DLC folder the same rule decides which of its packages are on: a package without `adl1.pak.xen` at its root
comes turned off (in a collection of 191 packages of every Guitar Hero, one is on: Death Magnetic).

## Turning it off

Turn the package (or its game's group) off on the DLC page. Without a DLC folder chosen, the game's list is not
written and the game reads only what is extracted into `00000002`.
