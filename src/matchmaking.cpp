// ghmetallica - online matchmaking without its wait for strangers.
//
// Online play here is between friends (the SDK's online_play): the players are a party of friends, who follow its
// leader into the match at once. The game's matchmaking also waits for strangers to fill the match's empty places, as
// on Xbox Live: its matchmaker (built by sub_823F7800) hosts the match for net_hosting_duration_secs (28, in
// scripts\game\net\net_live_settings.qb) plus up to 4 random seconds (+88, in ms, from its start at +84), then
// searches for other hosts to merge with (XSessionSearchEx), up to net_reservation_max_cycles (3, +96) times, with
// net_hosting_game_acceptable_wait (45 s, +100, ms) as its other limit. sub_823F6F18 is its hosting tick (now - start
// past +88: the hosting ends; someone connecting: 2 more seconds); sub_823F6B60 shows the time left. With a band of
// friends (Band Quickplay) the song came about 46 s after the match was made, waiting for no one.
//
// So the hosting time and the acceptable wait are made ghmetallica_online_host_wait seconds (and that + 2) when the
// matchmaker is built. Two separate groups of friends that want to find each other (Band vs Band, or two parties
// each in a duel) meet in that window: 0 keeps the game's own times.

#include <rex/cvar.h>
#include <rex/logging.h>
#include <rex/ppc/context.h>
#include <rex/system/kernel_state.h>
#include <rex/system/xmemory.h>

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <cstring>

REXCVAR_DEFINE_INT32(ghmetallica_online_host_wait, 3, "GHMetallica",
                     "Online play: the seconds the game's matchmaking hosts a match waiting for players who are not "
                     "in the party (strangers on Xbox Live; here, another group of friends). 0 = the game's own (28 s "
                     "plus up to 4, and up to 45 s in all), for two groups that look for each other (Band vs Band)");

namespace {

uint8_t* GuestBase() {
  auto* kernel = rex::system::kernel_state();
  return kernel && kernel->memory() ? kernel->memory()->virtual_membase() : nullptr;
}

uint32_t LoadWord(const uint8_t* base, uint32_t address) {
  uint32_t value;
  std::memcpy(&value, base + address, 4);
  return __builtin_bswap32(value);
}

void StoreWord(uint8_t* base, uint32_t address, uint32_t value) {
  value = __builtin_bswap32(value);
  std::memcpy(base + address, &value, 4);
}

// The matchmaker's fields (sub_823F7800).
constexpr uint32_t kHostingDurationMs = 88;
constexpr uint32_t kAcceptableWaitMs = 100;

}  // namespace

// [[midasm_hook]] at 0x823F78A4 in sub_823F7800 (the matchmaker's constructor), after its times are read from the
// settings: r31 = the matchmaker.
void GHMetallicaMatchmakerHook(PPCRegister& r31) {
  const int32_t wait = REXCVAR_GET(ghmetallica_online_host_wait);
  uint8_t* base = GuestBase();
  if (wait <= 0 || !base || !r31.u32) {
    return;
  }
  const uint32_t hosting = LoadWord(base, r31.u32 + kHostingDurationMs);
  const uint32_t acceptable = LoadWord(base, r31.u32 + kAcceptableWaitMs);
  const uint32_t new_hosting = uint32_t(wait) * 1000;
  const uint32_t new_acceptable = new_hosting + 2000;
  if (new_hosting < hosting) {
    StoreWord(base, r31.u32 + kHostingDurationMs, new_hosting);
  }
  if (new_acceptable < acceptable) {
    StoreWord(base, r31.u32 + kAcceptableWaitMs, new_acceptable);
  }
  REXLOG_INFO("[matchmaking] the match hosted {} ms for players outside the party (the game's {} ms), {} ms in all "
              "(the game's {} ms)",
              std::min(new_hosting, hosting), hosting, std::min(new_acceptable, acceptable), acceptable);
}
