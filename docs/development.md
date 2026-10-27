# Building and developing GHMetalRecomp

GHMetalRecomp is a static recompilation of **Guitar Hero: Metallica** (Xbox 360, 2009) to native Windows, built on
the [ReXGlue SDK](https://github.com/rexglue/rexglue-sdk). The game's PowerPC code is translated to C++ and
compiled. It is drawn by [NativeXRenderer](native-x-renderer-ghmetallica.md) (Vulkan), which draws the game's
Direct3D calls on the PC's GPU; ReXGlue's emulation of the Xbox 360 GPU, Xenos (derived from
[Xenia](https://xenia.jp)), is the alternative. The other notes in [`docs/`](README.md) describe each problem found
and what was done about it.

## Requirements

- Git, [CMake](https://cmake.org/) 3.25 or newer, [Ninja](https://ninja-build.org/),
  [LLVM/Clang](https://github.com/llvm/llvm-project/releases) (MSVC alone does not build this) and the Windows SDK.
- Python 3 (glslang needs it to configure the Vulkan backend).
- Your own copy of the game: extract the content of your disc or ISO with
  [extract-xiso](https://github.com/XboxDev/extract-xiso/releases) and copy all of it into `assets/`, so that
  `assets/default.xex` exists. The expected executable is the USA / Europe disc's, CRC32 `A7C93B87`;
  `GHMetallicaRecomp.exe` warns at startup if yours is a different version. See
  [`assets/README.md`](../assets/README.md).

## Building

```powershell
git clone --recursive https://github.com/ElGuslml/GHMetalRecomp
cd GHMetalRecomp
# copy your game files into assets\ first: the build recompiles assets\default.xex
cmake --preset win-amd64-release
cmake --build --preset win-amd64-release
```

- If you cloned without `--recursive`: `git submodule update --init --recursive`.
- The first build translates `default.xex` into C++ (`generated/`) and builds the SDK: it takes a while. If it stops
  with "precompiled header ... has been modified", run the build command again.
- Presets: `win-amd64-release`, `win-amd64-relwithdebinfo`, `win-amd64-debug`.

## Running

```powershell
.\out\build\win-amd64-release\"GH Metallica Launcher.exe"   # choose the options, then Play
.\out\build\win-amd64-release\GHMetallicaRecomp.exe          # or start the game directly with the saved options
```

- Game files are found in `assets\` next to `GHMetallicaRecomp.exe`, otherwise in the repository's `assets\`.
  `--game_data_root=<folder>` points somewhere else. `"GH Metallica Launcher.exe" --launcher_first_run` installs
  them again from an ISO.
- The saves, the achievements and the playtime go to the current profile of the launchers' shared store
  (`%APPDATA%\ElGuslml Recomps`, [`profiles.md`](profiles.md)); the shader cache and the DLC to `userdata\` next to
  `GHMetallicaRecomp.exe` (if that folder cannot be written, for example under Program Files, to
  `Documents\ghmetallica`). Logs go to `logs\`.

## Configuration

| File (next to `GHMetallicaRecomp.exe`) | What it holds |
|---|---|
| `ghmetallica.toml` | Base settings. Created with the defaults if it is missing; a default setting the file lacks is appended at startup. |
| `ghmetallica_options.toml` | What GH Metallica Launcher saves. Loaded after `ghmetallica.toml`. |

Command-line options (`--name=value`, for example Steam launch options) win over both files, for example
`--fullscreen=false --window_width=1920 --window_height=1080`, `--native_x_renderer=false` (the Xenos path),
`--ghmetallica_frame_rate=120` (60, 120, 144, 160, 165 or 240; [`frame-rate.md`](frame-rate.md)),
`--ghmetallica_ultrawide=true` ([`ultrawide.md`](ultrawide.md)), `--anisotropic_override=0` (off; 5 = 16×),
`--user_language=4` (French; [`language.md`](language.md)), `--quit_prompt=false` (B on the title screen does not ask
to quit).

## The launcher

GH Metallica Launcher is [RecompLauncher](https://github.com/ElGuslml/RecompLauncher) (`recomp_launcher/`, a
submodule: the launcher of every one of these recompilations, SDL3 + Dear ImGui) with this game's profile in
`launcher/`: `launcher/game.h` (names, the disc, the look, the options), `launcher/game.cpp` (the game's own texts),
`launcher/palette.h` (its colors) and `launcher/art/`, and its resources in `res/launcher.rc`.
`installer/ghmetallica.iss` is its Inno Setup installer.

## How the SDK is modified

`thirdparty/rexglue-sdk` is [ElGuslml/rexglue-sdk](https://github.com/ElGuslml/rexglue-sdk), branch `recomp`: the
official ReXGlue SDK's nightly of 2026-10-02 (v0.10.0 and 24 later commits) with the changes this project,
KameoRecomp, GRAWRecomp (Ghost Recon Advanced Warfighter), POTCAWERecomp (Pirates of the Caribbean: At World's End)
and [NativeXRenderer](https://github.com/ElGuslml/NativeXRenderer) need, one commit each. None of the general fixes
is specific to one game, and the options they add are off by default (this game sets the ones it needs in
`src/startup.cpp`); the ones for NativeXRenderer change nothing while it is off. The branch's `RECOMP.md` lists them,
and each commit message explains its change.

`native_x_renderer/` is NativeXRenderer, a submodule: a native renderer that hooks the game's Direct3D library. This
game's side of it (its hooks and the map of the game's Direct3D library) is in
`src/native_x_renderer_ghmetallica.cpp` and [`native-x-renderer-ghmetallica.md`](native-x-renderer-ghmetallica.md).
