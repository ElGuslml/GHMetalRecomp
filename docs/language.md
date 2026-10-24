# Game language

**Status: working (2026-10-24).** Checked: the main menu in Spanish, the title screen in German.

## Symptom

The game always started in English.

## Cause

The game has no language menu: at start it asks the console for the dashboard language (`XGetLanguage`) and loads
the matching text pack (`DATA/COMPRESSED/PAK/qs_f`, `qs_g`, `qs_i`, `qs_s`: French, German, Italian, Spanish;
English otherwise). The SDK answers with its `user_language` setting, English by default.

## Fix

GH Metallica Launcher has a **Language** row in its Game page (Deutsch, English, Español, Français, Italiano), which
writes `user_language` to `ghmetallica_options.toml`. The first time, it picks the Windows display language. The
quit dialog (quit-prompt.md) follows the same language.

## Turning it off

Choose English, or set `user_language = 1`.
