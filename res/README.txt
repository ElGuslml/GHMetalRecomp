GHMetalRecomp {VERSION} - Guitar Hero: Metallica (Xbox 360), for Windows
========================================================================

A static recompilation of the Xbox 360 game to native Windows. The game's code is already translated and
compiled into GHMetallicaRecomp.exe: you do not need to compile anything. You do need your own copy of the game,
because this package contains no game files.

This is an early test build. Not affiliated with Microsoft or the game's developer and publisher.


REQUIREMENTS

- Windows 10 or 11, 64-bit.
- A GPU with up-to-date drivers that support Direct3D 12 or Vulkan.
- A CPU with SSE4.2 (practically any 64-bit CPU from 2009 on).
- Microsoft Visual C++ Redistributable 2015-2022 (x64). Most PCs already have it. If Windows says that
  MSVCP140.dll or VCRUNTIME140.dll is missing, install it from:
  https://aka.ms/vs/17/release/vc_redist.x64.exe


INSTALLATION

1. Extract this zip to any folder, for example C:\Games\GHMetalRecomp.

2. Copy the contents of your Guitar Hero: Metallica (Xbox 360) disc into the "assets" folder, so that this file
   exists:

       assets\default.xex

   Copy everything from the disc. To get the files from a disc image (ISO), use extract-xiso
   (https://github.com/XboxDev/extract-xiso/releases) or ABGX360.

3. Run "GH Metallica Launcher.exe", choose your options and press Play. Next time you can start GHMetallicaRecomp.exe directly:
   it uses the options you saved.

If the game files are missing, GHMetallicaRecomp.exe tells you where it expected them. If your default.xex is not the version
this build was made from (CRC32 A7C93B87), it warns you: a different version will most likely crash.


"WINDOWS PROTECTED YOUR PC" / UNKNOWN PUBLISHER

The executables are not code-signed, so Windows SmartScreen warns about an unknown publisher the first time
you run them. To avoid it, unblock the zip before you extract it: right-click it, Properties, tick "Unblock"
at the bottom of the General tab, OK, and then extract it. If you already extracted it, click "More info"
and then "Run anyway" in the warning (once for each program).


OPTIONS IN GH Metallica Launcher

- Language: the game's languages.
- Internal resolution: the game's image rendered at 2x or 3x and fitted to the screen.
- Anisotropic filtering: 16x by default (sharper textures at oblique angles).
- Graphics API: DirectX 12 (recommended) or Vulkan.


DOWNLOADABLE CONTENT (DLC)

Your own Xbox 360 DLC packages can be installed: Metallica's "Death Magnetic" album (the package made for
Guitar Hero III, which this game also reads) adds 10 songs to Quickplay's song list (its blue button shows
the downloaded content). In GH Metallica Launcher's DLC page, "Choose folder..." looks for packages in a
folder and installs them; your files stay where they are. "Open the DLC folder" opens the folder where they
go (userdata\0000000000000000\41560830\00000002), for copying a package's extracted files by hand.


STEAM BIG PICTURE

Add GHMetallicaRecomp.exe as a non-Steam game: it starts straight into the game with the options saved by
GH Metallica Launcher. You can also add "GH Metallica Launcher.exe" to change options with a controller.


FILES

- ghmetallica.toml           Base settings. Created with the defaults this game needs if it is missing.
- ghmetallica_options.toml   What GH Metallica Launcher saves. Loaded after ghmetallica.toml.
- logs\                 Log files. Attach the latest one when you report a problem.
- userdata\             Saves, profile and shader cache. Keep this folder when you update to a new
                        version (extract the new version over the old one, or copy userdata\ across).
                        If the game folder cannot be written (for example under Program Files), they
                        go to Documents\ghmetallica instead.

Command line options (--name=value, for example in Steam launch options) win over both files, e.g.
--user_language=1 (English).


KNOWN ISSUES

- The first time an effect appears, it may show up a few frames late.


MORE INFORMATION

{MORE_INFO}

GHMetalRecomp's own code is under the MIT license (LICENSE). It is built on the ReXGlue SDK and other
components under their own licenses: see THIRD_PARTY_NOTICES.md and the "licenses" folder.
