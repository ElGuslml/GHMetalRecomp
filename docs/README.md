# Documentation

Short notes on each problem found while making the game run, and what was done about it. Each one gives
the symptom, the cause, the fix and how to turn the fix off, so that you can check it on your own machine.

To build the project and work on it: [Building and developing](development.md).

| Note | Status |
|---|---|
| [Profiles shared by the launchers](profiles.md) | Working: a profile per player in %APPDATA%, the saves and achievements copied into it |
| [NativeXRenderer in Guitar Hero: Metallica](native-x-renderer-ghmetallica.md) | Working: draws the game as the Xenos path does, with SMAA and ultrawide, at about half the GPU time (off by default) |
| [Game language](language.md) | Working: launcher language row (German, English, Spanish, French, Italian) |
| [Quitting from the title screen and the main menu](quit-prompt.md) | Working: B asks whether to quit (A quits, B cancels) |
| [Frame rates above 60](frame-rate.md) | Working, experimental: 120 to 240 FPS at the game's speed, the launcher's frame rate row |
| [Ultrawide displays](ultrawide.md) | Working: the stage seen wider (Hor+), menus, HUD and note highway kept 16:9 in the center |
| [The crowd claps out of time on star power](crowd-claps.md) | Fixed: the claps follow the audio calibration, as the music does |
| [The audio drifts out of sync during a song](song-sync.md) | Fixed: the game's clock at the timebase's real rate; 21 ms less audio delay |
| [CPU usage: five cores at 100 %](cpu-usage.md) | Fixed: the game's threads sleep while they wait; registers as local variables in the generated code |
| [Playing with a controller or the keyboard as a guitar](guitar-controller.md) | Working: the controller is reported as a guitar; the keyboard's frets on D F J K L, and a switch to strum on each fret |
| [Downloadable content (DLC)](dlc.md) | Working: the player's Death Magnetic package plays in Quickplay; the launcher's DLC page installs packages |
| [An empty screen after pressing a button on a second controller](second-controller.md) | Fixed: the controller that presses a button at the title screen plays with the profile, the others are its guests, as on the console |
| [Only the guitar and the kick drum heard in the songs](song-audio.md) | Fixed: every track of a song plays |
| [Noise instead of music from the title screen on](menu-music.md) | Fixed: the game's audio output is called at the console's pace |
