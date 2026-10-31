# macOS

GHMetalRecomp builds and runs on macOS: the recompiled game, NativeXRenderer and GH Metallica Launcher, packaged as
`Guitar Hero Metallica.app` in a disk image. It is experimental: tested on one Mac (Apple M3 Pro, macOS 27), with the
known issues below.

## Building

Requirements: the Xcode Command Line Tools, CMake, Ninja, Python 3 (`brew install cmake ninja python`), and your
game files in `assets/` (see [`development.md`](development.md)).

```sh
git clone --recursive https://github.com/ElGuslml/GHMetalRecomp
cd GHMetalRecomp
installer/macos/make_dmg.sh                # Apple Silicon: out/package/GHMetalRecomp-<version>-macos-arm64.dmg
ARCH=amd64 installer/macos/make_dmg.sh     # Intel (or Rosetta)
```

`make_dmg.sh` configures and builds the `mac-<arch>-release` preset (the game and the launcher), assembles the app
bundle, signs it ad hoc and makes the disk image. Without `assets/default.xex` it builds the **shell** instead
(`GHMETALLICA_SHELL`): the executable without the recompiled game, which starts, shows its window and the launcher's
first run, but cannot play. It lets the packaging be built and tried on a machine (or CI) without the game.

Optional: `installer/macos/fetch_kosmickrisp.sh` downloads the LunarG Vulkan SDK and takes only KosmicKrisp out of
it (the installer is never run); `make_dmg.sh` then bundles it next to MoltenVK (see Graphics below).

## The app

| Where | What |
|---|---|
| `Contents/MacOS/GH Metallica Launcher` | the app's executable: options, the first run from the ISO, profiles, updates; Play starts the game |
| `Contents/MacOS/GHMetallicaRecomp` | the game, and the SDK's libraries next to it |
| `Contents/Resources/vulkan/` | the Vulkan loader and MoltenVK (and KosmicKrisp, when bundled) |
| `~/Library/Application Support/GHMetalRecomp/` | settings (`ghmetallica.toml`, `ghmetallica_options.toml`), `logs/`, `userdata/`, the game files (`assets/`) |
| `~/Library/Application Support/ElGuslml Recomps/` | the launchers' profiles, shared with the other ElGuslml recompilations |

Nothing is written into the bundle: it is signed, and in `/Applications` may be read-only (`DataFolder`,
`src/startup.cpp`; the launcher's `platform::DataFolder`).

The app is signed ad hoc, not notarized: on another Mac, open it the first time with right click > Open.

## What differs from Windows

Everything outside these is the Windows code; the shared files only have small `#ifdef` blocks.

- **Graphics.** NativeXRenderer draws on a Metal view on the game's window (its decision D108), through Vulkan on
  Metal. Two drivers: MoltenVK (the SDK's, the default) and KosmicKrisp (LunarG's, when bundled, with
  `ghmetallica_vulkan_driver = "kosmickrisp"`; `src/macos_vulkan_driver.cpp`). KosmicKrisp is lighter on the GPU but
  lost its device once on the Xenos path. DLSS is Windows-only.
- **glslang twice.** It is linked into the game (NativeXRenderer's SDK sources) and into the GPU plugin; dyld merges
  weak symbols across images, so the plugin ran the game's copy against state it never set up (a fault at the first
  shader, retried forever: a spinning wheel). The game does not export them (`CMakeLists.txt`).
- **Fullscreen** is macOS's own, in a Space of its own (`fullscreen_exclusive = false`,
  `SDL_VIDEO_MAC_FULLSCREEN_SPACES`).
- **Audio.** The audio threads run at the highest quality of service (`src/fmod_mix.cpp`; Windows' "Pro Audio" class
  is Windows-only in the SDK), CoreAudio's buffer is one console frame (256 samples), and the game's queue is 6 frames
  instead of 4 (macOS's shared output takes sound in larger bursts).
- **MoltenVK's debug messages** (one per pipeline: Metal cannot disable primitive restart) are not logged unless Vulkan
  validation is on.
- **Build:** Objective-C++ enabled for DiligentCore, DiligentCore told MoltenVK's SPIRV-Cross namespace, and the SDK's
  `REXGLUE_ROOT` set for its MoltenVK staging (`CMakeLists.txt`).

## Known issues

- **Kirk's hair** is stretched across the screen (in songs and loading screens). The same on the Xenos path, with
  MoltenVK and KosmicKrisp, natively and as an Intel build under Rosetta, and at a steady 60 FPS: not the renderer, the
  graphics driver, the CPU architecture or the frame rate. Not yet known whether Windows shows it too.
- **The renderer thread slows down through a song** (0.5 ms per frame at the start, 40 ms and more later): its guest
  memory pages are hashed and uploaded again far more often than on Windows. Apple Silicon's pages are 16 KB, four
  times Windows', which is the first suspect. A lower internal resolution helps the GPU side only.
- **The Xenos path is slow:** its command processor is one thread at its limit in songs. Use NativeXRenderer.
- **KosmicKrisp** lost its device in a song on the Xenos path (the SDK then ends the game).
- **Updates** look for a `...-macos-<arch>.dmg` asset on the release; until releases have one, there is none to offer.
- The SDK's `Clock::QueryHostUptimeMillis` overflows its 64-bit multiplication after about 8.9 days of uptime on Apple
  Silicon (24 MHz ticks), and after about 5 hours under Rosetta (1 GHz ticks).
