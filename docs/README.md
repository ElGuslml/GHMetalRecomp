# Documentation

Short notes on each problem found while making the game run, and what was done about it. Each one gives
the symptom, the cause, the fix and how to turn the fix off, so that you can check it on your own machine.

| Note | Status |
|---|---|
| [Game language](language.md) | Working: launcher language row (German, English, Spanish, French, Italian) |
| [The audio drifts out of sync during a song](song-sync.md) | Fixed: the game's clock at the timebase's real rate; 21 ms less audio delay |
| [CPU usage: five cores at 100 %](cpu-usage.md) | Fixed: the game's threads sleep while they wait |
| [Playing with a controller or the keyboard as a guitar](guitar-controller.md) | Working: the controller is reported as a guitar (the keyboard's keys still the SDK's) |
| [Only the guitar and the kick drum heard in the songs](song-audio.md) | Fixed: every track of a song plays |
| [Noise instead of music from the title screen on](menu-music.md) | Fixed: the game's audio output is called at the console's pace |
