GHMetalRecomp {VERSION} - Guitar Hero: Metallica (Xbox 360), for Windows
========================================================================

A static recompilation of the Xbox 360 game to native Windows. The game's code is already translated and
compiled into GHMetallicaRecomp.exe: you do not need to compile anything. You do need your own copy of the
game, because this package contains no game files.

This is a beta. Not affiliated with Microsoft, Activision, Neversoft or Metallica.


REQUIREMENTS

- Windows 10 or 11, 64-bit.
- A GPU with up-to-date drivers that support Vulkan (NativeXRenderer, the default renderer) or
  Direct3D 12 (the Xenos renderer).
- A CPU with SSE4.2 (practically any 64-bit CPU from 2009 on).
- Microsoft Visual C++ Redistributable 2015-2022 (x64). Most PCs already have it. If Windows says that
  MSVCP140.dll or VCRUNTIME140.dll is missing, install it from:
  https://aka.ms/vs/17/release/vc_redist.x64.exe
- Your Guitar Hero: Metallica disc as an ISO file (the USA / Europe disc; about 7.2 GB once installed).
- An Xbox 360 guitar. Any other controller works as a guitar too, and so does the keyboard.
- To sing (experimental): any microphone connected to the PC, and a controller for the singer.


INSTALLATION

1. Either run the installer (GHMetalRecomp-{VERSION}-setup.exe: it installs for your Windows user, without
   administrator rights), or extract this zip to any folder, for example C:\Games\GHMetalRecomp.

2. Run "GH Metallica Launcher.exe". The first time, it asks for your Xbox 360 ISO, checks that it is the right
   game and extracts its files next to the launcher, then asks you to create your profile (your saves,
   achievements and playtime, shared with the other ElGus Recomps launchers). You only do this once.

   You can also copy the game files yourself instead: put the contents of the disc into the "assets"
   folder, so that assets\default.xex exists (extract-xiso, https://github.com/XboxDev/extract-xiso/releases,
   gets them out of an ISO).

3. Choose your options and press PLAY. Next time you can start GHMetallicaRecomp.exe directly: it uses the
   options you saved.

4. Calibrate: in the game's options, run the audio and video calibration once, so that the notes match what
   you hear on your own speakers or headphones.

If your default.xex is not the version this build was made from (CRC32 A7C93B87), the game warns you: a
different version will most likely crash.


"WINDOWS PROTECTED YOUR PC" / UNKNOWN PUBLISHER

The executables are not code-signed, so Windows SmartScreen warns about an unknown publisher the first time
you run them. To avoid it with the zip, unblock it before you extract it: right-click it, Properties, tick
"Unblock" at the bottom of the General tab, OK, and then extract it. Otherwise click "More info" and then
"Run anyway" in the warning (once for each program).


GH METALLICA LAUNCHER

The launcher works with the mouse, the keyboard or a controller. Changes are saved automatically and apply
when you press PLAY.

- Play: starts the game, with your current settings, achievements and updates at a glance.
- Your profile (your picture at the top right): hours played, streaks, the week's activity, your games
  and your favorite achievements; edit your name, picture and color, or switch profiles.
- Achievements: the game's 50 achievements, unlocked or not. In the game, F7 shows the same list.
- Controls: your guitars, controllers, the keyboard and the microphone.
  - A guitar or a controller: what each of its buttons does. A guitar's frets are A, B, Y, X and LB of an
    Xbox controller, strum is the D-pad, the whammy bar the right stick and Star Power Back. A controller's
    Instrument: Guitar, or Vocals for the singer (see SINGING).
  - Keyboard (experimental): turn on "Play with keyboard and mouse" to play with the keyboard as a guitar
    (frets D F J K L, strum with the Up and Down arrows, Star Power with Space, Start with Enter), and change
    any key. "Strum when a fret is pressed" makes each fret strum as you press it, in songs.
  - Microphone (experimental): the microphone you sing with, its channel and gain, and its level (see
    SINGING).
- Settings:
  - Video: the monitor, fullscreen, borderless or window, the resolution, the image format (fill a screen
    wider than 16:9, where the stage is seen wider while the note highway, the HUD and the menus stay 16:9
    in the center, or keep 16:9 with bars) and the internal resolution the game is drawn at.
  - Graphics: the renderer, the frame rate, antialiasing and anisotropic filtering. NativeXRenderer (the
    default) draws the game natively with Vulkan; Xenos emulates the Xbox 360 GPU instead (DirectX 12 or
    Vulkan): choose it if something looks wrong with NativeXRenderer, and tell us where. Frame rate: 60 FPS
    (original), and 120, 144, 160, 165 or 240 FPS where your monitor reaches them (experimental). The songs
    keep their speed and stay in sync at every frame rate. The game's screen effects can be turned off one by
    one.
  - Game: the language (English, French, German, Italian or Spanish; the game has no language menu of its
    own).
- DLC: install your own Xbox 360 downloadable content. "Choose folder..." looks for packages in a folder and
  installs them; your files stay where they are. Metallica's "Death Magnetic" album (the package made for
  Guitar Hero III, which this game also reads) adds 10 songs to Quickplay's song list (its blue button shows
  the downloaded content).
- Updates: the launcher checks for a new version when it starts (you can turn that off) and installs it
  for you, keeping your saves, settings and game files.


SINGING (EXPERIMENTAL)

Any microphone connected to the PC works as the game's Xbox 360 USB microphone:
1. In the launcher's Controls tab, choose Microphone (under the keyboard) and pick your microphone, or the
   Windows default. With an audio interface, pick the channel of the input your microphone is on.
2. Sing as loud as you will in a song and watch the Level bar: it should reach the yellow without touching
   the red. Change the Gain if it does not (it starts at -12 dB).
3. Choose the controller that sings and set its Instrument to Vocals. In Quickplay, that controller sings.
If the pitch line jumps around, lower the Gain; if the game does not hear you, raise it. Headphones help: with
speakers, the microphone hears the song too. While a microphone is chosen, Windows shows that the game is using
it.


MORE THAN ONE CONTROLLER

The controller that presses a button at the title screen ("PRESS ANY BUTTON TO ROCK") plays with your profile
and its saves. The other controllers join as its guests, as on the Xbox 360 ("Name (1)", "Name (2)"): they play
without saves of their own.


QUITTING THE GAME

On the title screen or in the main menu, press B (the red fret): the game asks whether to quit. A (the green
fret, or Enter) quits, B (or Escape) goes back.


STEAM BIG PICTURE

Add GHMetallicaRecomp.exe as a non-Steam game: it starts straight into the game with the options saved by
GH Metallica Launcher. You can also add "GH Metallica Launcher.exe" to change options with a controller.


FILES

- ghmetallica.toml           Base settings. Created with the defaults this game needs if it is missing.
- ghmetallica_options.toml   What GH Metallica Launcher saves. Loaded after ghmetallica.toml.
- logs\                      Log files. Attach the latest one when you report a problem.
- userdata\                  The shader cache and the installed DLC. Keep this folder if you move the game to
                             another folder. If the game folder cannot be written (for example under Program
                             Files), it goes to Documents\ghmetallica instead.
- Your saves, achievements and playtime are in your profile: %APPDATA%\ElGuslml Recomps.

Command line options (--name=value, for example in Steam launch options) win over both files, e.g.
--user_language=1 (English), --ghmetallica_frame_rate=120 or --native_x_renderer=false (the Xenos renderer).


REPORTING A PROBLEM

Tell us on Discord (https://discord.gg/XQtM5TmNs8) or open an issue on the project's page, with where in the
game it happened, a screenshot if you can, and the latest file in the logs folder.


KNOWN ISSUES

- Songs have been played with an Xbox 360 guitar on guitar and bass parts, and sung with a PC microphone.
  Drums have not been tested yet.
- Singing is experimental (tested with one microphone): if the pitch line jumps or the game misses your
  voice, change the microphone's Gain in the launcher.
- Some USB audio interfaces add delay in Windows' shared audio mode (a Focusrite Scarlett about 20 ms more
  than a motherboard's audio): calibrate the audio in the game's options.
- Above 60 FPS is experimental: something may still run too fast. Tell us what and where.
- Playing with the keyboard is experimental.
- In an ultrawide window, the second controller's "not signed in" screen shows a colored band at its right
  edge.
- The first time an effect appears, it may show up a few frames late.


MORE INFORMATION

{MORE_INFO}

GHMetalRecomp's own code is under the MIT license (LICENSE). It is built on the ReXGlue SDK and other
components under their own licenses: see THIRD_PARTY_NOTICES.md and the "licenses" folder.
