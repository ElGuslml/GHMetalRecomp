# Documentation

Short notes on each problem found while making the game run, and what was done about it. Each one gives
the symptom, the cause, the fix and how to turn the fix off, so that you can check it on your own machine.

| Note | Status |
|---|---|
| [NativeXRenderer in Guitar Hero: Metallica](native-x-renderer-ghmetallica.md) | In progress: the observer at 100 % in the menus and songs; draws the menus and songs (off by default) |
| [Game language](language.md) | Working: launcher language row (German, English, Spanish, French, Italian) |
| [Quitting from the title screen and the main menu](quit-prompt.md) | Working: B asks whether to quit (A quits, B cancels) |
| [Ultrawide displays](ultrawide.md) | Working: wider stage, menus, HUD and highway kept 16:9 in the center (a dark band around the highway) |
| [The audio drifts out of sync during a song](song-sync.md) | Fixed: the game's clock at the timebase's real rate; 21 ms less audio delay |
| [CPU usage: five cores at 100 %](cpu-usage.md) | Fixed: the game's threads sleep while they wait; registers as local variables in the generated code |
| [Playing with a controller or the keyboard as a guitar](guitar-controller.md) | Working: the controller is reported as a guitar (the keyboard's keys still the SDK's) |
| [Only the guitar and the kick drum heard in the songs](song-audio.md) | Fixed: every track of a song plays |
| [Noise instead of music from the title screen on](menu-music.md) | Fixed: the game's audio output is called at the console's pace |
