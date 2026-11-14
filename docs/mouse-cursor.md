# The mouse cursor

**Symptom (before):** the mouse cursor stayed over the game, in a window and in fullscreen.

**Now:** it hides over the game after 2 seconds without moving the mouse, as most games played with a controller do.
It shows again at once when the mouse moves, while another window has the focus (over the game too), and over the
SDK's overlays that use the mouse (the quit prompt). Outside the game's window Windows shows its own cursor.

**How:** the SDK's change 103 (`cursor_auto_hide`, `cursor_auto_hide_ms`), set by `src/startup.cpp`.

**Turning it off:** `cursor_auto_hide = false` in `ghmetallica.toml` (the cursor always shown); `cursor_auto_hide_ms`
changes the delay (milliseconds).
