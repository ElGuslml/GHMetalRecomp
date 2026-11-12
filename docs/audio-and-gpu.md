# Choosing the audio output and the graphics card

**Status: added (2026-11-11), launcher 3.19:** Settings → Audio, and Settings → Graphics → Graphics card. Every
option keeps the game as it was until it is changed.

## Settings → Audio

| Row | Option file key | What it does |
|---|---|---|
| Output device | `audio_output_device` | Windows default (followed when it changes in Windows), or a device by its name as Windows shows it. A device not connected when the game starts plays on Windows default. |
| Channels | `audio_channels` | `auto`: the game's 5.1 mix on surround speakers, folded to stereo on the rest; `stereo`: always folded (headsets that report virtual 7.1); `mono`: the same sound on both sides. |
| Volume | `audio_volume` | 0-100 %, on top of Windows' volume and the game's own sliders. |
| Mute in the background | `audio_mute_unfocused` | Silence while another window has the focus. |
| Low latency audio | `audio_wasapi_exclusive` | Moved here from Settings → Game ([Low latency audio](audio-latency.md)); the device chosen above is the one taken in exclusive mode. |

These are the SDK's (changes 85 and 64), for both of its outputs (the shared one through SDL and the exclusive one
through WASAPI). The log names the device: `audio endpoint '<name>': ... submitting 2 ch (mono)`.

## Settings → Graphics → Graphics card

`gpu_adapter` (the SDK's change 86): Automatic, or a card by its PCI vendor and device IDs (`"10DE:2782"`), the same
for Direct3D 12 and Vulkan on the Xenos path and for NativeXRenderer (its D141). For PCs with two graphics cards, as a
laptop's integrated and dedicated ones. Automatic leaves each renderer's own choice: NativeXRenderer the dedicated
card, Xenos the first one Windows lists. A card not found draws on that one, with a warning in the log
(`gpu_adapter ... not found`).

## Turning them off

Reset in the category (X), or the keys back to their defaults: `audio_output_device = ""`, `audio_channels = "auto"`,
`audio_volume = 100`, `audio_mute_unfocused = false`, `gpu_adapter = ""`.
