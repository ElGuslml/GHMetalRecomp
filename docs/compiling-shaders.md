# Stutters the first time something is shown

**Symptom.** The first time the game shows something (the first song, a venue, an effect), it stutters, or an object
appears a few frames late. The next time it plays smoothly.

**Cause.** NativeXRenderer translates the game's shaders to Vulkan and the graphics driver compiles each combination of
them with the drawing state (a pipeline) the first time a draw needs it: 10 to 500 ms each. The renderer keeps every
pipeline it compiled (`userdata\cache\native_x_renderer\41560830.pipelines`) and prepares them at the next start, so only
a first time stutters.

**Fix (NativeXRenderer D129, launcher 3.14.0).** The game ships a list of the pipelines found while playing it,
`41560830.pipelines.list` beside `GHMetallicaRecomp.exe`. It names each shader by a hash of its code: the code itself
comes from the player's own game files (`DATA\FXFILES\MaterialLibrary.bin.xen`, where the game keeps all of its shaders),
so nothing of the game is in it. The first time PLAY is pressed, the launcher shows "Compiling shaders" while the game's
executable, without a window, rebuilds those shaders, compiles every listed pipeline and the ones stored before for this
graphics card and driver, and saves the driver's binaries (`41560830.vkcache`). The game then creates all of them in
about a tenth of a second at its start. On an RTX 4070 Ti the screen takes under a second (473 pipelines). It shows again
only after an update of the game or of the graphics driver.

**How to turn it off.** Skip on that screen starts the game at once; it then compiles at its draws, as before. Without the
list file, or with the Xenos renderer, the launcher does not show it. Started without the launcher (Steam, the
executable directly), the game does what it did before. The tool can be run by hand:
`GHMetallicaRecomp.exe --native_x_renderer_prepare_pipelines=progress.txt`.
