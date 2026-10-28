# Singing with a PC microphone

**Status: experimental (2026-10-28, the `microphone` branch).** The game finds the microphone, opens it as the
console's USB microphone (48 kHz, 1 ms packets) and reads it at its real rate from the start; Gustavo sang Quickplay
songs with a HyperX QuadCast S and scored. With the microphone at the full Windows level the pitch line jumped
(the first test): the microphone is now 12 dB quieter by default (the Gain row). Not checked yet: two singers.

## What was missing

The game sings with an Xbox 360 USB microphone (a Logitech one, and Rock Band's and Lips' kind) or a headset. It
finds a USB microphone with `XamUserGetDeviceContext(index, 4)` and talks to it through the kernel's
`MicDeviceRequest`, which the SDK did not answer: no microphone, and Quickplay offered no vocals.

## How it works

Two SDK changes (the `recomp` branch's numbering):

- **55** (`usb_microphone_1`, `usb_microphone_1_channel`): a recording device of the PC answers as the game's USB
  microphone 1. The SDK plays the device's side of each request of the XDK's microphone library: its state, opening
  it, its sample rate, its volume control and reads of 1 ms packets (16-bit, 48 kHz, recorded with SDL), each ending
  in the request's completion routine, as the console's USB stack does. At a song's start the game turns its own
  microphone down to the bottom of its range (made for its Logitech microphone); a PC microphone gets
  `usb_microphone_1_gain` instead, -12 dB in `ghmetallica.toml` (`src/startup.cpp`): at the full Windows level a
  condenser microphone also hears the room and the speakers. A microphone unplugged and plugged in again is found
  again.
- **56** (`gamepad_device_subtypes`): the singer needs an ordinary controller, which the game gives the vocals, while
  `gamepad_subtype = 6` makes every controller a guitar ([guitar-controller.md](guitar-controller.md)). This gives
  chosen controllers their own kind, by their serial number (a DualSense's) or their GUID; `keyboard` is the
  keyboard's controller.

GH Metallica Launcher sets both (Controls tab): **Microphone**, under the keyboard, chooses the device (or the
Windows default), the channel (an audio interface's input) and the gain, and shows the level the game hears (sing:
the bar should reach the yellow and stay out of the red); each controller's **Instrument** row chooses Guitar or
Vocals (a controller that sings has no buttons to assign).

## Turning it off

`usb_microphone_1 = ""` (the default; the launcher's Microphone: None) and `gamepad_device_subtypes = ""`. While a
microphone is chosen, Windows shows that the game is using it, from the start: the game reads its microphone all
the time.
