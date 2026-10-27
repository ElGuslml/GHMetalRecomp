// ghmetallica - the crowd's claps on star power follow the audio calibration.
//
// While a player's star power is on, the crowd claps on the beat. The game plays the claps from a script spawned for
// each fretbar: the fretbar's handler (script 0x5E4AD8C0, registered on the song's fretbar events by script 0x2324FE5A)
// waits 0.18 s, then spawns the crowd's beat script (0x65DEB5EC), which plays a clap (SoundEvent, one of four events
// of six sounds) when any player has star_power_used. The fretbar events come as late as the player's calibration (the
// offset they are registered with takes off audio_offset and the video offset), and the 0.18 s does not take it back:
// with Gustavo's 104 ms of audio calibration the claps came 104 ms after the music's beat, as on the console. The wait
// now takes the calibration off: the music and the claps in time (Gustavo, 2026-10-27, with 104 ms taken off by hand
// first). The calibration is read from the script globals the same script sets: 0x9D1315C9 = 0xEFEE50D3 + the audio
// offset + the video offset.

#include <rex/cvar.h>
#include <rex/logging.h>
#include <rex/ppc/context.h>
#include <rex/system/kernel_state.h>
#include <rex/system/xmemory.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <optional>

REXCVAR_DEFINE_BOOL(ghmetallica_crowd_claps_fix, true, "GHMetallica",
                    "The crowd's claps on star power keep time with the music whatever the audio and video "
                    "calibration (the game claps late by the calibration); false: as on the console");

namespace {

constexpr uint32_t kFretbarHandlerScript = 0x5E4AD8C0;
constexpr uint32_t kGlobalsTable = 0x82A8D9E0;    // the script globals' hash table (a pointer), sub_822BB900's
constexpr uint32_t kCalibratedOffset = 0x9D1315C9;  // 0xEFEE50D3 + audio offset + video offset (script 0x2324FE5A)
constexpr uint32_t kBaseOffset = 0xEFEE50D3;

const uint8_t* GuestBase() {
  auto* kernel = rex::system::kernel_state();
  return kernel && kernel->memory() ? kernel->memory()->virtual_membase() : nullptr;
}

uint32_t Load32(const uint8_t* base, uint32_t address) {
  uint32_t value;
  std::memcpy(&value, base + address, 4);
  return __builtin_bswap32(value);
}

// A script global's number, as sub_822BB900 reads it: buckets by the key's low 15 bits, entries {+2 type, +4 key,
// +12 value, +16 next}; type 1 an integer, 2 a float, 26 another global's key.
std::optional<double> ScriptGlobal(const uint8_t* base, uint32_t key) {
  const uint32_t table = Load32(base, kGlobalsTable);
  if (!table) {
    return std::nullopt;
  }
  for (int alias = 0; alias < 4; ++alias) {
    uint32_t entry = Load32(base, table + ((key & 0x7FFF) << 2));
    for (int length = 0; entry && length < 4096 && Load32(base, entry + 4) != key; ++length) {
      entry = Load32(base, entry + 16);
    }
    if (!entry) {
      return std::nullopt;
    }
    const uint8_t type = base[entry + 2];
    const uint32_t value = Load32(base, entry + 12);
    if (type == 1) {
      return double(int32_t(value));
    }
    if (type == 2) {
      float f;
      std::memcpy(&f, &value, 4);
      return double(f);
    }
    if (type != 26) {
      return std::nullopt;
    }
    key = value;
  }
  return std::nullopt;
}

uint32_t ScriptName(const uint8_t* base, uint32_t script) { return script ? Load32(base, script + 216) : 0; }

}  // namespace

// [[midasm_hook]] 0x8239318C (the script function Wait, sub_82392F58, its seconds branch): r4 = the wait in
// milliseconds, r30 = the script. The fretbar handler's wait is shortened by the calibration.
void GHMetallicaCrowdBeatWaitHook(PPCRegister& r4, PPCRegister& r30) {
  if (!REXCVAR_GET(ghmetallica_crowd_claps_fix)) {
    return;
  }
  const uint8_t* base = GuestBase();
  if (!base || ScriptName(base, r30.u32) != kFretbarHandlerScript) {
    return;
  }
  const auto calibrated = ScriptGlobal(base, kCalibratedOffset);
  const auto plain = ScriptGlobal(base, kBaseOffset);
  if (!calibrated || !plain) {
    return;
  }
  const int32_t calibration = int32_t(std::lround(*calibrated - *plain));
  const int32_t wait = int32_t(r4.u32);
  r4.u64 = uint64_t(uint32_t(std::clamp(wait - calibration, 0, wait)));
  static int32_t logged = INT32_MIN;
  if (calibration != logged) {
    logged = calibration;
    REXLOG_INFO("[crowd] the claps' wait {} ms less the calibration {} ms (globals {} and {})", wait, calibration,
                *calibrated, *plain);
  }
}
