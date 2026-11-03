// ghmetallica - the profile follows the controller that starts the game, and each slot's kind of controller follows
// the one played.
//
// On the console the controller that presses a button at the title screen ("PRESS ANY BUTTON TO ROCK") plays with
// its own profile. Here there is one profile, the first controller's (the order Windows connected them in), and
// another controller that started the game got the game's "not signed in" message. While the title screen is shown
// (src/quit_prompt.cpp finds its text element each frame), the SDK's primary press window is open: the controller
// that presses first becomes user 0, the profile's, and the other controllers are its guests ("Name (1)", the SDK's
// signin_guests, set in src/startup.cpp), as when guests sign in on the console. docs/second-controller.md.
//
// The game asks a controller port for its kind (sub_8239D688: a guitar, the drums, or the singer for a plain gamepad)
// only when the port's controller connects. The keyboard's pad shares slot 0 with the first controller, and the SDK
// answers for the one in the player's hands, which at the start is the controller: with a controller set to sing in
// the launcher (or a kind the game gives the vocals) and the keyboard playing guitar, the keyboard's player was the
// singer (a player's report, 2026-11-02; docs/controller-kind.md). When the SDK finds the slot played on another kind
// of controller, the port's update asks again, outside a song. The same after the title screen's press swapped two
// controllers (the game takes no notice of XN_SYS_INPUTDEVICESCHANGED).

#include "players.h"

#include <rex/cvar.h>
#include <rex/input/input_system.h>
#include <rex/logging.h>
#include <rex/ppc/context.h>
#include <rex/system/kernel_state.h>
#include <rex/system/xmemory.h>

#include <cstdint>
#include <cstring>

#include "widescreen.h"

REXCVAR_DEFINE_BOOL(ghmetallica_profile_follows_press, true, "GHMetallica",
                    "The controller that presses a button at the title screen plays with the profile; off: the "
                    "first controller connected does, as before");
REXCVAR_DEFINE_BOOL(ghmetallica_kind_follows_controller, true, "GHMetallica",
                    "A slot played on another kind of controller than the game was told of (the keyboard playing "
                    "guitar where the slot's controller sings) is asked for its kind again, outside a song; off: "
                    "the kind of the controller the slot had when it connected, as before");

namespace {

// sub_8239DFB8's port: the guest user it reads, and its state (1: waiting for its controller, which the game then
// asks for its kind; 5: one more read; 2: reading it each frame).
constexpr uint32_t kPortUser = 12;
constexpr uint32_t kPortState = 228;
constexpr uint32_t kStateWaiting = 1;
constexpr uint32_t kStateReading = 2;

uint8_t* GuestBase() {
  auto* kernel = rex::system::kernel_state();
  return kernel && kernel->memory() ? kernel->memory()->virtual_membase() : nullptr;
}

uint32_t Load32(const uint8_t* base, uint32_t address) {
  uint32_t value;
  std::memcpy(&value, base + address, 4);
  return __builtin_bswap32(value);
}

void Store32(uint8_t* base, uint32_t address, uint32_t value) {
  value = __builtin_bswap32(value);
  std::memcpy(base + address, &value, 4);
}

}  // namespace

// [[midasm_hook]] 0x8239DFB8: the entry of sub_8239DFB8, a controller port's update (each frame for each port, from
// sub_822E90D0; r3 = the port). A port reading its controller goes back to waiting, so that this update reads it
// again and asks for its kind; its input is not read for two frames.
void GHMetallicaControllerPortHook(PPCRegister& r3) {
  uint8_t* base = GuestBase();
  if (!base || !REXCVAR_GET(ghmetallica_kind_follows_controller) || ghmetallica::SongShown()) {
    return;
  }
  if (Load32(base, r3.u32 + kPortState) != kStateReading) {
    return;
  }
  const uint32_t user = Load32(base, r3.u32 + kPortUser);
  if (!rex::input::TakeDeviceKindChange(user)) {
    return;
  }
  Store32(base, r3.u32 + kPortState, kStateWaiting);
  REXLOG_INFO("[players] slot {}: the game asks for its kind of controller again", user);
}

namespace ghmetallica {

void SetTitleScreen(bool shown) {
  static bool open = false;
  const bool want = shown && REXCVAR_GET(ghmetallica_profile_follows_press);
  if (want == open) {
    return;
  }
  open = want;
  rex::input::SetPrimaryPressWindow(open);
  REXLOG_DEBUG("[players] the title screen's press window {}", open ? "open" : "closed");
}

}  // namespace ghmetallica
