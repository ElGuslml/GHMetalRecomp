<p align="center">
  <img src="res/icon.svg" width="200" alt="GHMetalRecomp icon">
</p>

<h1 align="center">GHMetalRecomp</h1>

<p align="center">
  <a href="https://github.com/ElGuslml/GHMetalRecomp/releases/latest"><img alt="Latest release" src="https://img.shields.io/github/v/release/ElGuslml/GHMetalRecomp?style=for-the-badge&label=Latest%20release&color=c5dc3a"></a>
  <a href="https://discord.gg/XQtM5TmNs8"><img alt="Discord: join the server" src="https://img.shields.io/badge/Discord-Join%20the%20server-5865F2?style=for-the-badge&logo=discord&logoColor=white"></a>
  <a href="https://github.com/ElGuslml/ElGusRecomps"><img alt="More games: ElGus Recomps" src="https://img.shields.io/badge/More%20games-ElGus%20Recomps-2f6b45?style=for-the-badge&logo=github"></a>
</p>

🎸 **Play Guitar Hero: Metallica on your PC, natively.** The Xbox 360 game by Neversoft (2009), recompiled to run
directly on Windows: no emulator to set up, sharp graphics, wide screens, high frame rates, and your Xbox 360
guitar.

> [!IMPORTANT]
> 💿 **You need your own original copy of the game.** This project does not include any of the game's files, and
> never will.
> - To play, you need an Xbox 360 copy of *Guitar Hero: Metallica* that you own (the USA / Europe disc).
> - You make an image (ISO) of your own disc, and the launcher installs the game from it.
> - Please do not ask for the game's files, and do not share them or links to them.
>
> This project is not affiliated with or endorsed by Microsoft, Activision, Neversoft or Metallica.

> [!NOTE]
> 🤘 **Beta.** Songs have been played from start to finish with an Xbox 360 guitar, on guitar and bass parts, and sung
> with a PC microphone (new and experimental). Drums have not been tested yet. If something goes wrong, please
> [tell us](#-reporting-a-problem)!

## 🎮 How to play

1. **Download** the latest version from [Releases](https://github.com/ElGuslml/GHMetalRecomp/releases/latest):
   - **the installer** (`GHMetalRecomp-...-setup.exe`): installs for your Windows user, no administrator rights
     needed, with a Start menu shortcut; or
   - **the zip** (`GHMetalRecomp-win64.zip`): extract it into any folder you can write to (not `Program Files`).
2. **Open GH Metallica Launcher.** The first time, it asks for the ISO of your game, checks it and installs the game
   files (about 7.2 GB), then asks you to create your profile. You only do this once.
3. **Pick your settings and press PLAY.** 🎉
4. **Calibrate:** in the game's options, run the audio and video calibration once, so that the notes match what you
   hear on your own speakers or headphones.

From then on, the launcher keeps itself up to date: when a new version comes out, it offers to install it for you.

### 💻 What you need

- Windows 10 or 11 (64-bit).
- A graphics card with up-to-date drivers that support Vulkan (practically any card from the last ten years).
- The [Microsoft Visual C++ Redistributable](https://aka.ms/vs/17/release/vc_redist.x64.exe) (most PCs already
  have it).
- An Xbox 360 guitar. Any other controller works as a guitar too, and so does the keyboard.
- To sing (experimental): any microphone connected to the PC, and a controller for the singer.

## ✨ Features

- 🎸 **Your guitar:** an Xbox 360 guitar plays as on the console, whammy bar and Star Power included. Any other
  controller is a guitar too, and the keyboard can be one (experimental: turn it on in the launcher's Keyboard page;
  frets on D F J K L).
- 🎤 **Sing (experimental):** any microphone on your PC is the game's USB microphone. Choose it in the launcher, and
  choose which controller sings.
- 🖥️ **Sharp, native graphics:** the game is drawn by your graphics card at your screen's resolution, with smooth
  edges (antialiasing) and sharper textures.
- ⚡ **High frame rates:** 60 FPS as on the console, or 120, 144, 160, 165 or 240 FPS if your monitor supports it
  (experimental). The songs keep their speed and stay in sync.
- 📺 **Wide and ultrawide screens:** the stage fills 16:9, 21:9 and wider screens, while the note highway, the HUD and
  the menus keep their shape in the center.
- 🎶 **Downloadable content:** install your own Xbox 360 DLC packages (the *Death Magnetic* album) from the launcher.
- 👤 **Profiles:** your saves, achievements and playtime in a profile shared by all the ElGus Recomps launchers.
- 👥 **Play together:** the controller that presses a button at the title screen plays with your profile; the other
  controllers join as its guests, as on the Xbox 360.
- 🏆 **Achievements:** a notification when you unlock one, and all 50 in the launcher or with F7 in the game.
- 🌍 **Languages:** English, French, German, Italian and Spanish.
- 🔄 **Automatic updates** from this page.

## 🧭 GH Metallica Launcher

Everything is set up from **GH Metallica Launcher**, with the mouse, the keyboard or a controller:

| Tab | What you find there |
|---|---|
| ▶️ **Play** | Start the game, and see your settings, achievements and updates at a glance. |
| 🎶 **DLC** | Install your downloadable content packages, and see the installed ones. |
| 🏆 **Achievements** | All 50 achievements, the ones you have unlocked and when. |
| 🎮 **Controls** | Your guitars, controllers and keyboard: what each button or key does, and which controller sings. The keyboard as a guitar (experimental), with frets that strum as you press them. Your microphone (experimental): device, gain and level. |
| ⚙️ **Settings** | **Game:** the language. **Video:** monitor, fullscreen or window, resolution, image format (fill a wide screen or keep 16:9). **Graphics:** renderer, frame rate, antialiasing, texture sharpness, the game's screen effects. |
| 🔄 **Updates** | Your version, and new ones when they come out. |
| 👤 **Profile** | Your picture at the top right: hours played, streaks, your week, your games and your favorite achievements. |

Your choices are saved as soon as you make them. When you quit the game, the launcher comes back.

**Tips:**
- 🎤 **To sing:** in **Controls → Microphone**, choose your microphone and sing as loud as in a song: the level bar
  should reach the yellow without touching the red (change **Gain** if not). Then choose the singer's controller and
  set its **Instrument** to **Vocals**. Headphones help: with speakers, the microphone hears the song too.
- 🚪 On the title screen or in the main menu, press **B** (the red fret) to quit the game.
- 🎮 **Steam Big Picture:** add `GH Metallica Launcher.exe` as a non-Steam game and play from your couch.
- 🛡️ **"Windows protected your PC":** the programs are not signed by a company, so Windows may warn you the first
  time. Click **More info**, then **Run anyway**.

## 🐞 Reporting a problem

Something looks wrong, sounds wrong, or crashes? Tell us on [Discord](https://discord.gg/XQtM5TmNs8) or open an
[issue](https://github.com/ElGuslml/GHMetalRecomp/issues), with:
- where in the game it happened, and a screenshot if you can;
- the latest file in the `logs` folder next to the game.

If something looks wrong, try the other renderer: in the launcher, **Graphics → Xenos** (it emulates the Xbox 360's
graphics instead), and tell us whether it happens there too.

### Known issues

- 🥁 Drums have not been tested yet: only guitar and bass parts are known to work.
- 🎤 Singing is experimental (tested with one microphone): if the pitch line jumps around, lower the microphone's
  **Gain** in the launcher; if the game does not hear you, raise it. While a microphone is chosen, Windows shows that
  the game is using it.
- ⌨️ Playing with the keyboard is experimental.
- 🔊 Some USB audio interfaces add delay in Windows' shared audio mode (a Focusrite Scarlett about 20 ms more than
  a motherboard's audio): calibrate the audio in the game's options.
- ⚡ Above 60 FPS is experimental: something may still run too fast.
- 📺 In an ultrawide window, the second controller's "not signed in" screen shows a colored band at its right edge.
- ✨ The first time an effect appears, it may show up a moment late.

## 🛠️ For developers

GHMetalRecomp is a *static recompilation*: the game's original Xbox 360 code is translated to C++ and compiled for
Windows, on top of the [ReXGlue SDK](https://github.com/rexglue/rexglue-sdk). It is drawn by
[NativeXRenderer](https://github.com/ElGuslml/NativeXRenderer), a renderer that draws the game's graphics calls
natively on Vulkan.

- [Building and developing](docs/development.md): requirements, building from source, configuration, how the SDK is
  modified.
- [`docs/`](docs/README.md): a short note on each problem found and how it was solved.

## 🙏 Credits

- [ReXGlue SDK](https://github.com/rexglue/rexglue-sdk), whose runtime is derived from [Xenia](https://xenia.jp).
- [extract-xiso](https://github.com/XboxDev/extract-xiso), for extracting game files.
- Developed by Gustavo Garcia with the help of Claude (Anthropic).

## 📜 License

MIT for this project's own code (see [`LICENSE`](LICENSE)). The ReXGlue SDK and the other components keep their
own licenses (see [`THIRD_PARTY_NOTICES.md`](THIRD_PARTY_NOTICES.md)). Guitar Hero: Metallica, its code and its
assets belong to their respective owners.
