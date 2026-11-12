# Stutters at 60 FPS in crowded venues on slower PCs

**Status: fixed in 0.5.0-beta.1 (2026-11-12), NativeXRenderer only;** waiting for the reporting player's test.

**Symptom.** On a GTX 1650 laptop with an i5-10300H, the game stuttered at 60 FPS at Tushino Air Field when much of the
crowd or the fog was on screen, while neither the GPU nor any CPU core was busy, and ran at about 80 FPS uncapped.

**Cause.** NativeXRenderer draws on a thread of its own, after the game has issued its calls. So that the game never
reuses memory the renderer has not read yet, the SDK's command processor holds every signal the GPU gives the game
(fences, swaps) until the renderer has caught up (NativeXRenderer D71). It waited for everything the game had queued
by then, not only for what came before the signal: the fence of the previous frame, which the game's present waits
for, waited for part of the current frame too. The game and the renderer then took turns instead of working at the
same time: neither saturates a core, and on a slower CPU their turns add up past a 60 Hz frame. At Tushino on a fast
PC those waits were about 3 ms a frame (0.3 ms in The Forum); the crowd itself costs 0.3-0.55 ms of the GPU's ~3.5 ms.

**Fix (SDK change 88, NativeXRenderer D142).** The command processor knows which of the game's submissions each signal
came in, and a signal waits only for the renderer's work on what was submitted before it. Measured at 240 FPS in the
career's first song: those waits went from up to 2.6 ms a frame to none, with the same images.

**How to turn it off.** `native_x_renderer_precise_gate = false` in `ghmetallica.toml` (or
`--native_x_renderer_precise_gate=false`): every signal waits for everything again.
