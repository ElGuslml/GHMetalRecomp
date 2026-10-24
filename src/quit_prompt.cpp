// ghmetallica - quitting from the title screen and the main menu.
//
// The game has no way out of itself (on the console the dashboard was). On the title screen ("PRESS ANY BUTTON TO
// ROCK") and in the main menu (Career, Quickplay, Head to Head...), B now asks whether to quit the game: the SDK's
// quit prompt (rex/ui/overlay/quit_prompt.h, on the recomp branch), a dialog in the game's language where A quits
// (the window closes, as with its close button) and B cancels. The game does not see those presses. Both screens:
// once past the title screen the game never goes back to it, and B does nothing in the main menu.
//
// The game's screens are made of screen elements that its scripts create and destroy (CreateScreenElement,
// sub_823F54E8; DestroyScreenElement, sub_823F5148), each with an id, the checksum of its name. Two of them are on
// one of these screens only (found in the scripts, DATA/COMPRESSED/PAK/qb.pab.xen):
// - 0xA9869543, the "PRESS ANY BUTTON TO ROCK" text, which ui_create_boot_iis (the title screen) puts in its
//   current_menu container;
// - 0xA7988D2C, the container the main menu's script creates (next to destroy_main_menu).
// The engine finds an element by id in a hash table (sub_82174720, the table object at [0x82AAFAF8]); the hook reads
// the same table once per frame on the main thread, which runs the scripts, and offers the prompt while either
// element exists and no popup does (0x5F4DC94F, popupelement: a message box over the menu takes B itself).
// quit_prompt = false (an SDK cvar) disables it.

#include <rex/logging.h>
#include <rex/ppc/context.h>
#include <rex/system/kernel_state.h>
#include <rex/system/xmemory.h>
#include <rex/ui/overlay/quit_prompt.h>

#include <cstdint>
#include <cstring>

namespace {

constexpr uint32_t kElementTable = 0x82AAFAF8;  // the screen elements' table object (a pointer)
constexpr uint32_t kTitleText = 0xA9869543;
constexpr uint32_t kMainMenuContainer = 0xA7988D2C;
constexpr uint32_t kPopupElement = 0x5F4DC94F;  // QBKey("popupelement")

const uint8_t* GuestBase() {
  auto* kernel = rex::system::kernel_state();
  return kernel && kernel->memory() ? kernel->memory()->virtual_membase() : nullptr;
}

uint32_t Load32(const uint8_t* base, uint32_t address) {
  uint32_t value;
  std::memcpy(&value, base + address, 4);
  return __builtin_bswap32(value);
}

// sub_82174720: the element with this id, or 0. The table object holds two hash tables (+8, +12), each
// {uint32 bits, entry* buckets, ..., uint8 keep_empty at +21}; an entry is {uint32 key, uint32 value, entry* next}
// and the chain starts at buckets[id & ((1 << bits) - 1)].
uint32_t FindElement(const uint8_t* base, uint32_t id) {
  const uint32_t table = Load32(base, kElementTable);
  if (!table) {
    return 0;
  }
  for (uint32_t offset : {8u, 12u}) {
    const uint32_t hash = Load32(base, table + offset);
    if (!hash) {
      continue;
    }
    const uint32_t bits = Load32(base, hash);
    const uint32_t buckets = Load32(base, hash + 4);
    if (!buckets || bits > 24) {
      continue;
    }
    uint32_t entry = buckets + (id & ((1u << bits) - 1)) * 12;
    for (int length = 0; entry && length < 4096; ++length) {
      if (Load32(base, entry) == id) {
        if (const uint32_t value = Load32(base, entry + 4)) {
          return value;
        }
        break;
      }
      entry = Load32(base, entry + 8);
    }
  }
  return 0;
}

}  // namespace

// [[midasm_hook]] 0x822E45F4 (sub_822E4588, the main thread's frame, before it waits for the previous one): once per
// frame.
void GHMetallicaQuitPromptHook() {
  const uint8_t* base = GuestBase();
  if (!base) {
    return;
  }
  const bool title = FindElement(base, kTitleText) != 0;
  const bool main_menu = FindElement(base, kMainMenuContainer) != 0;
  const bool popup = FindElement(base, kPopupElement) != 0;
  static int last_state = -1;
  const int state = (title ? 1 : 0) | (main_menu ? 2 : 0) | (popup ? 4 : 0);
  if (state != last_state) {
    REXLOG_DEBUG("[quit_prompt] title {} main menu {} popup {}", title, main_menu, popup);
    last_state = state;
  }
  if ((title || main_menu) && !popup) {
    rex::ui::OfferQuitPrompt();
  }
}
