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
| [A lower internal resolution looks soft](scaling.md) | Added, off by default: AMD FSR 1 or NVIDIA Image Scaling to the screen, and a sharpness row (NativeXRenderer) |
| [Stutters the first time something is shown](compiling-shaders.md) | Fixed: a pipeline list ships with the game; the launcher compiles it for your graphics card at the first PLAY ("Compiling shaders"; NativeXRenderer) |
| [A lattice in the bloom at high resolutions](bloom-lattice.md) | Fixed: the bloom's and the depth of field's reduced copies of the scene at the game's resolution (NativeXRenderer) |
| [The game's own debug menu](debug-menu.md) | Added, off by default: Neversoft's debug menu in the main and pause menus (the launcher's Settings → Game row), without its save-changing shortcuts |
| [Skipping the videos at the start](intro.md) | Added, off by default: the game starts at its title screen (the launcher's Settings → Game row) |
| [Turning off the game's screen effects](screen-effects.md) | Switches in the launcher's Settings → Graphics: bloom, depth of field, grain, color correction, vignette, ink blotches (all on by default) |
| [The crowd claps out of time on star power](crowd-claps.md) | Fixed: the claps follow the audio calibration, as the music does |
| [The audio drifts out of sync during a song](song-sync.md) | Fixed: the game's clock at the timebase's real rate; 21 ms less audio delay |
| [The music falls behind the notes after stutters](song-stutters.md) | Fixed: FMOD mixes on the audio thread, the silence of a late frame is made up, the audio threads at "Pro Audio" priority |
| [Crackling, or the sound muting with low latency audio, on slower PCs](audio-crackling.md) | Fixed: the audio calls no longer spaced, the catch-up never adds silence |
| [Low latency audio](audio-latency.md) | Added, off by default: the launcher's Settings > Audio row, the device in exclusive mode (77 -> 20 ms on a USB interface) |
| [Choosing the audio output and the graphics card](audio-and-gpu.md) | Added: the launcher's Settings → Audio (output device, stereo or mono, volume, mute in the background, low latency) and Settings → Graphics → Graphics card |
| [Stutters at 60 FPS in crowded venues on slower PCs](crowd-stutters.md) | Fixed (0.5.0-beta.1): the GPU's signals to the game wait only for the renderer's work on what came before each (NativeXRenderer) |
| [CPU usage: five cores at 100 %](cpu-usage.md) | Fixed: the game's threads sleep while they wait; registers as local variables in the generated code |
| [Singing with a PC microphone](microphone.md) | Experimental: a PC microphone as the game's USB microphone, a controller's instrument (guitar or vocals) in the launcher |
| [Shift, Ctrl, Alt and the F keys on the keyboard's controls](keyboard-keys.md) | Added: a modifier alone is a key of its own (Alt as Star Power); F3, F4 and ` do nothing unless the launcher's Debug keys row is on; a key on a control only presses it |
| [The keyboard playing guitar taken for the singer](controller-kind.md) | Fixed: each player's kind of controller (guitar, drums, singer) follows the controller they play with, outside a song |
| [Playing with a controller or the keyboard as a guitar](guitar-controller.md) | Working: the controller is reported as a guitar; the keyboard's frets on D F J K L, a switch to strum on each fret; assignments per controller, a whammy bar's calibration |
| [Drums on the keyboard or a controller](drums.md) | Added: the launcher's Instrument row (Guitar, Drums, Vocals) and Drum kit row: Guitar Hero's 5 lanes or the game's own Rock Band 4-lane highway |
| [V-Sync](vsync.md) | Added: the launcher's V-Sync row, on by default: FIFO and the game's vertical blank on the monitor's (4 ms from present to display at 160 Hz), or each frame as it is ready |
| [Downloadable content (DLC)](dlc.md) | Working: the player's Death Magnetic package plays in Quickplay; the launcher's DLC page takes the player's DLC folder, read in place (0.4.1-beta.1) |
| [An empty screen after pressing a button on a second controller](second-controller.md) | Fixed: the controller that presses a button at the title screen plays with the profile, the others are its guests, as on the console |
| [Only the guitar and the kick drum heard in the songs](song-audio.md) | Fixed: every track of a song plays |
| [Noise instead of music from the title screen on](menu-music.md) | Fixed: the game's audio output is called at the console's pace |
