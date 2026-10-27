// ghmetallica - frame rates above 60: the emulated vertical blank at the chosen rate.
//
// No patch for a higher frame rate exists for any Guitar Hero (Xenia Canary's game patches, RPCS3's patches,
// 2026-10-25): they run at 60 on the consoles. The game's own pacing (sub_8241C840, the render thread presenting a
// frame) is all vertical blanks:
// - one frame in flight: it waits until the swap counter 0x82A925A0 (sub_822D0E70; the swap callback sub_822D0CC8
//   adds one when a frame is shown) passes the previous frame's;
// - then the present interval, from the global 0x82848540 (1 in the image, written by no code): 1 = every vertical
//   blank (the vblank callback sub_822D0C80 counts them at 0x82A92590), 2 = every second one, 0 = immediate.
// The engine's clock is real time: the main thread measures each frame with the time base (sub_822E4588) and the
// game scales by the frame's duration in seconds. So with the SDK's emulated vblank at a higher rate
// (video_mode_refresh_rate, which src/options.cpp sets from ghmetallica_frame_rate) the game presents at that rate
// and keeps its speed: at 120 Hz it ran at 120 FPS (8.33 ms per frame in a song), and the career's first song took as
// long from its start to the bot's failure as at 60 (74.5 s and 75.8 s, a screenshot per second). With vsync off
// (the vblank every millisecond) it ran at about 660 FPS in the menus: the game has no cap of its own.
//
// The waits of src/thread_waits.cpp follow the vblank's rate (VblankPeriod).

#include <rex/cvar.h>
#include <rex/logging.h>
#include <rex/ppc/context.h>
#include <rex/system/kernel_state.h>
#include <rex/system/xmemory.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstring>
#include <mutex>
#include <unordered_map>

REXCVAR_DEFINE_INT32(ghmetallica_frame_rate, 60, "GHMetallica",
                     "Frame rate: 60 as on the console, or 120, 144, 160, 165 or 240: the emulated vertical blank runs "
                     "at that rate (video_mode_refresh_rate, set at startup) and the game presents on every one; its "
                     "clock is real time, so it keeps its speed");

REXCVAR_DEFINE_BOOL(ghmetallica_frame_rate_fix, true, "GHMetallica",
                    "With the vertical blank above 60 Hz: what the game counts in frames keeps the console's speed "
                    "(script waits in game frames, the whammy on sustains, the loading screen's animations, clocks "
                    "counted in vertical blanks) "
                    "and scripts are given the button presses of the frames they skip (false = those run faster, to "
                    "compare)");

// The engine keeps its vertical blanks per second in the global 0x82839118: 60, or 50 for a PAL video mode
// (sub_822D96C0, sub_823821C8; a script can set it, sub_823EBBD0). Some of its clocks count vertical blanks and
// divide by it, so they would run as many times faster as the vblank's rate is above 60 (no visible case known: the
// star and the calibration Gustavo saw run fast were the loading screen and script waits, below). Those divisions use
// the global times the vblank's rate over 60. The global itself is left alone: other code compares it with 60
// (sub_82168D20 takes a 50 Hz branch otherwise) or uses it as the console's 60ths for per-frame steps measured in real
// time.
namespace {

// The emulated vertical blanks per second (the SDK's vblank thread): video_mode_refresh_rate with vsync, one a
// millisecond without. Read once a second.
double VblankRate() {
  thread_local double rate = 60.0;
  thread_local std::chrono::steady_clock::time_point read;
  const auto now = std::chrono::steady_clock::now();
  if (read == std::chrono::steady_clock::time_point{} || now - read > std::chrono::seconds(1)) {
    read = now;
    rate = rex::cvar::Query<bool>("vsync") ? std::clamp(rex::cvar::Query<double>("video_mode_refresh_rate"), 24.0, 240.0)
                                           : 1000.0;
  }
  return rate;
}

// The script waits counted in game frames ("Wait N gameframe(s)", 313 in the game's scripts; the script function
// sub_82392F58 sets type 1 through sub_8242A018, or type 8 through sub_82429FC8): when each began and its count.
struct FrameWait {
  std::chrono::steady_clock::time_point start;  // when it would begin at 60 frames a second
  std::chrono::steady_clock::time_point end;    // start + frames / 60 s
};
std::mutex g_frame_waits_mutex;
std::unordered_map<uint32_t, FrameWait> g_frame_waits;  // by the wait's guest address (the script + 168)

bool HighRate() { return REXCVAR_GET(ghmetallica_frame_rate_fix) && VblankRate() != 60.0; }

// The game's vertical blanks per second scaled to the emulated rate (unchanged at 60 Hz).
void ScaleVblankRate(PPCRegister& rate) {
  if (!REXCVAR_GET(ghmetallica_frame_rate_fix)) {
    return;
  }
  const double vblank_rate = VblankRate();
  if (vblank_rate != 60.0) {
    rate.u64 = uint64_t(int64_t(std::lround(double(int32_t(rate.u32)) * vblank_rate / 60.0)));
  }
}

}  // namespace

// [[midasm_hook]] 0x824B1C68 and 0x824B1EF0 (sub_824B1B00): before "extsw r8,r30" / "extsw r11,r30", r30 = the
// game's vertical blanks per second, which divides a clock counted in vertical blanks (the vblank counter
// sub_822D0E80 less the clock's start and its time paused, kept by sub_824B0240) into seconds, and turns seconds back
// into vertical blanks. The clock is advanced by sub_824B4BC8 for an object's virtual update (sub_823B8F68).
void GHMetallicaVblankClockHook(PPCRegister& r30) { ScaleVblankRate(r30); }

// [[midasm_hook]] 0x822D0E00 (sub_822D0DE0): before "extsw r5,r10", r10 = the game's vertical blanks per second,
// which divides the frame's duration in vertical blanks (the swap callback's history) into its time at +48 of the
// timing block 0x82A92458.
void GHMetallicaVblankFrameTimeHook(PPCRegister& r10) { ScaleVblankRate(r10); }

// Script waits in game frames: sub_8242A318, each frame, takes one from the count at +4 of a type 1 or 8 wait and ends
// the wait in the frame after it reaches 0; its first update comes in the frame the script set the wait, so "Wait N
// gameframes" lasts N frames: N / 60 s on the console, half that at 120 FPS (Gustavo: the calibration's beeps and
// targets ran fast; the calibration's beeps are 64 waits of 1 game frame each, 1.066 s).
// Above 60 Hz the count is held above 0 and let reach 0 in the frame before the one nearest the wait's end at 60
// frames a second, so the wait ends within half a frame of it. A wait set right as the previous one on the same script
// ended (a loop) begins at that one's end at 60 frames a second, not at the frame it was set in, so a loop keeps the
// console's pace on average whatever the frame rate (at 160 Hz a 1-frame wait can only last 12.5 or 18.75 ms).
// Measured with a temporary probe of PlaySound: the calibration's beeps every 1.066 s at 60; 0.533 s at 120 without
// this; 1.33 to 1.46 s with its first version, which ended each wait in the first frame after its end (late by up to a
// frame, and by a whole one whenever a frame came a little early).
namespace {

double FrameSeconds() { return 1.0 / VblankRate(); }

// The script that drains star power (below), and the name of the script a wait belongs to (+48 of the wait).
constexpr uint32_t kStarPowerDrainScript = 0x74B936AB;

uint32_t WaitingScript(uint32_t wait) {
  auto* kernel = rex::system::kernel_state();
  const uint8_t* base = kernel && kernel->memory() ? kernel->memory()->virtual_membase() : nullptr;
  if (!base) {
    return 0;
  }
  uint32_t name;
  std::memcpy(&name, base + wait + 48, 4);
  return __builtin_bswap32(name);
}

}  // namespace

// [[midasm_hook]] at the entry of sub_8242A018 and sub_82429FC8 (setting a game-frame wait): r3 = the wait, r4 = N.
void GHMetallicaFrameWaitSetHook(PPCRegister& r3, PPCRegister& r4) {
  if (!HighRate()) {
    return;
  }
  // A script that measures its own time runs every frame, as the game wrote it. The star power's drain (script
  // 0x74B936AB) waits one game frame per pass and takes current_deltatime (the frame's time, sub_82195548) off the meter
  // each pass, or GetFrameLength's in a band: held to 60 passes a second it drained 60 frames' time a second, and at
  // 160 FPS star power lasted 160 / 60 times as long (Gustavo). It is the only script of the game's 7822 that waits in
  // game frames and reads either. The waiting script's name is at +48 of its wait (+216 of the script).
  if (WaitingScript(r3.u32) == kStarPowerDrainScript) {
    std::lock_guard lock(g_frame_waits_mutex);
    g_frame_waits.erase(r3.u32);
    return;
  }
  const auto now = std::chrono::steady_clock::now();
  const auto frame = std::chrono::duration<double>(FrameSeconds());
  std::lock_guard lock(g_frame_waits_mutex);
  FrameWait& wait = g_frame_waits[r3.u32];
  const bool follows = wait.end != std::chrono::steady_clock::time_point{} &&
                       std::chrono::abs(std::chrono::duration<double>(now - wait.end)) <= 1.5 * frame;
  wait.start = follows ? wait.end : now;
  wait.end = wait.start + std::chrono::duration_cast<std::chrono::steady_clock::duration>(
                              std::chrono::duration<double>(double(int32_t(r4.u32)) / 60.0));
}

// [[midasm_hook]] 0x8242A35C (sub_8242A318, a game-frame wait's count above 0): before "addi r11,r11,-1" and its
// store to +4, r3 = the wait, r11 = the count. Set to 1 (it becomes 0, the wait ends next frame) once the wait's end is
// less than a frame and a half away, so that next frame is the one nearest to it; else to 2 (it stays waiting).
void GHMetallicaFrameWaitCountHook(PPCRegister& r3, PPCRegister& r11) {
  if (!HighRate()) {
    return;
  }
  std::chrono::steady_clock::time_point end;
  {
    std::lock_guard lock(g_frame_waits_mutex);
    auto it = g_frame_waits.find(r3.u32);
    if (it == g_frame_waits.end()) {
      return;
    }
    end = it->second.end;
  }
  static std::atomic<bool> logged{false};
  if (!logged.exchange(true)) {
    REXLOG_INFO("[frame rate] script waits in game frames last as at 60 FPS (vblank at {:.0f} Hz)", VblankRate());
  }
  const double left = std::chrono::duration<double>(end - std::chrono::steady_clock::now()).count();
  r11.u64 = left < 1.5 * FrameSeconds() ? 1 : 2;
}

// Button presses polled by scripts: the script function ControllerMake (sub_823B6D50; GuitarControllerMake calls it)
// tests the bits of the buttons pressed since the previous update (+52 of a controller's input state; sub_8239BB88
// updates the four controllers' once a frame, +48 = held, +52 = pressed, +56 = released), so a press shows for one
// frame. A script whose game-frame waits are held (above) runs only every second frame or so at 120 Hz and missed the
// presses of the frames between (Gustavo: the video calibration's strums counted only sometimes; a probe at 160: the
// calibration's script polled every 2 or 3 frames and 3 of 7 strums fell in frames between, 7 of 7 seen with this; the
// audio calibration polls the same way). Each script is given the presses of the updates since its previous run that
// it did not see, once, from the last 60th of a second and a frame: they are added to the state's pressed bits while
// its ControllerMake runs, then the bits are put back.
namespace {

constexpr int kKeptUpdates = 8;

struct InputUpdates {
  uint32_t state = 0;  // the input state's guest address
  uint32_t seq = 0;    // updates so far
  uint32_t pressed[kKeptUpdates] = {};
  std::chrono::steady_clock::time_point when[kKeptUpdates] = {};
};

struct ScriptPolls {
  uint32_t run_seq = 0;   // the state's update at the script's current run
  uint32_t prev_seq = 0;  // and at its previous run
  std::chrono::steady_clock::time_point last;
};

struct PatchedState {
  uint32_t state, written, original;
};

std::mutex g_input_mutex;
InputUpdates g_input[4];
std::unordered_map<uint64_t, ScriptPolls> g_script_polls;  // by script << 32 | state
thread_local PatchedState g_patched[4];
thread_local int g_patched_count = 0;

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

}  // namespace

// [[midasm_hook]] 0x8239BD4C (sub_8239BB88, after the input state's held, pressed and released bits are stored):
// r30 = the state, r9 = the pressed bits.
void GHMetallicaInputUpdateHook(PPCRegister& r30, PPCRegister& r9) {
  if (!HighRate()) {
    return;
  }
  std::lock_guard lock(g_input_mutex);
  for (InputUpdates& input : g_input) {
    if (input.state == r30.u32 || input.state == 0) {
      input.state = r30.u32;
      const int slot = int(++input.seq % kKeptUpdates);
      input.pressed[slot] = r9.u32;
      input.when[slot] = std::chrono::steady_clock::now();
      return;
    }
  }
}

// [[midasm_hook]] at the entry of sub_823B6D50 (ControllerMake): r4 = the script.
void GHMetallicaButtonPollHook(PPCRegister& r4) {
  g_patched_count = 0;
  uint8_t* base = GuestBase();
  if (!HighRate() || !base) {
    return;
  }
  const auto now = std::chrono::steady_clock::now();
  const auto window = std::chrono::duration<double>(1.0 / 60.0 + FrameSeconds());
  std::lock_guard lock(g_input_mutex);
  if (g_script_polls.size() > 256) {
    std::erase_if(g_script_polls, [&](const auto& entry) { return now - entry.second.last > std::chrono::seconds(1); });
  }
  for (const InputUpdates& input : g_input) {
    if (input.state == 0) {
      continue;
    }
    ScriptPolls& polls = g_script_polls[uint64_t(r4.u32) << 32 | input.state];
    if (polls.last == std::chrono::steady_clock::time_point{} || now - polls.last > window * 4) {
      polls.run_seq = polls.prev_seq = input.seq;  // a script new to polling: only the current update
    } else if (polls.run_seq != input.seq) {
      polls.prev_seq = polls.run_seq;
      polls.run_seq = input.seq;
    }
    polls.last = now;
    uint32_t missed = 0;
    const uint32_t kept = polls.run_seq >= kKeptUpdates ? polls.run_seq - kKeptUpdates + 1 : 1;
    for (uint32_t seq = std::max(polls.prev_seq + 1, kept); seq < polls.run_seq; ++seq) {
      const int slot = int(seq % kKeptUpdates);
      if (now - input.when[slot] <= window) {
        missed |= input.pressed[slot];
      }
    }
    const uint32_t original = LoadWord(base, input.state + 52);
    if (missed & ~original) {
      StoreWord(base, input.state + 52, original | missed);
      g_patched[g_patched_count++] = {input.state, original | missed, original};
    }
  }
}

// [[midasm_hook]] 0x823B6E10 (sub_823B6D50's exit, every path): the pressed bits put back.
void GHMetallicaButtonPollEndHook() {
  uint8_t* base = GuestBase();
  for (int i = 0; i < g_patched_count && base; ++i) {
    const PatchedState& patched = g_patched[i];
    if (LoadWord(base, patched.state + 52) == patched.written) {
      StoreWord(base, patched.state + 52, patched.original);
    }
  }
  g_patched_count = 0;
}

// The whammy on a sustain (the note hold update sub_82148DA8 -> sub_82146590, run every frame for each player:
// sub_82137F18 -> sub_8214AC00 -> sub_8214A748, 120 calls a second at 120 FPS): sub_82143530 (the whammy bar;
// sub_821463B0 for the touch strip) compares the bar's position (0 at rest to 1) with the previous frame's and, if it
// moved more than 0.03, sets a countdown at +49628 to 5; sub_82146590 takes one from it each frame and, while above 0,
// the player is whammying (star power grows by the frame's time); after 5 frames in a row of whammying or not
// (+49632, +49636) it tells the held frets (sub_82130380), and a star power sustain turns blue or back. Above 60 Hz a
// frame's movement is smaller and 5 frames are shorter (Gustavo at 120: the blue came and went while he moved the
// bar). The position is compared with the one from a 60th of a second before, and the 5 frames are 5 / 60 s.
namespace {

struct WhammySamples {
  std::chrono::steady_clock::time_point when[8] = {};
  float position[8] = {};
  int next = 0;
};

std::mutex g_whammy_mutex;
std::unordered_map<uint64_t, WhammySamples> g_whammy;  // by the note hold object << 1 | axis

// The position the previous frame's is replaced by: the sample nearest a 60th of a second before (the newest at
// least that less half a frame old, if not more than a frame and a half over it); else the game's own.
void CompareWhammyOverSixtieth(int axis, uint32_t object, PPCRegister& position, PPCRegister& previous) {
  if (!HighRate()) {
    return;
  }
  const auto now = std::chrono::steady_clock::now();
  const double frame = FrameSeconds();
  std::lock_guard lock(g_whammy_mutex);
  WhammySamples& samples = g_whammy[uint64_t(object) << 1 | uint64_t(axis)];
  for (int i = 1; i <= 8; ++i) {
    const int slot = (samples.next - i) & 7;
    const double age = std::chrono::duration<double>(now - samples.when[slot]).count();
    if (age >= 1.0 / 60.0 - 0.5 * frame) {
      if (age <= 1.0 / 60.0 + 1.5 * frame) {
        previous.f64 = samples.position[slot];
      }
      break;
    }
  }
  samples.when[samples.next] = now;
  samples.position[samples.next] = float(position.f64);
  samples.next = (samples.next + 1) & 7;
}

int WhammyFrames(int frames) { return int(std::lround(frames * VblankRate() / 60.0)); }

}  // namespace

// [[midasm_hook]] 0x821436A0 and 0x8214379C (sub_82143530) and 0x8214652C (sub_821463B0): before "fsubs" of the
// position and the previous frame's: r30 / r31 = the note hold object, f30 / f0 = the position, f13 / f0 = the previous.
void GHMetallicaWhammyHook(PPCRegister& object, PPCRegister& position, PPCRegister& previous) {
  CompareWhammyOverSixtieth(0, object.u32, position, previous);
}

// [[midasm_hook]] 0x821437B0 (sub_82143530, a controller's second axis): r30, f29 = the position, f0 = the previous.
void GHMetallicaWhammy2Hook(PPCRegister& object, PPCRegister& position, PPCRegister& previous) {
  CompareWhammyOverSixtieth(1, object.u32, position, previous);
}

// [[midasm_hook]] 0x821436D0, 0x821437D0 (r10) and 0x82146558 (r9): the countdown of 5 frames before it is stored.
void GHMetallicaWhammyCountdownHook(PPCRegister& frames) {
  if (HighRate()) {
    frames.u64 = uint64_t(WhammyFrames(int32_t(frames.u32)));
  }
}

// [[midasm_hook]] 0x82146790 and 0x8214686C (sub_82146590): before "cmpwi cr6,r11,5", r11 = the frames in a row of
// whammying or not (dead after the comparison): 5 when they reach 5 / 60 s.
void GHMetallicaWhammyStateHook(PPCRegister& r11) {
  if (HighRate()) {
    r11.u64 = int32_t(r11.u32) == WhammyFrames(5) ? 5 : 0;
  }
}

// The loading screen (sub_824C3028, drawn every frame while the byte 0x81BDC850 is set: sub_823A4AB0 ->
// sub_823F7DF0) animates by the frame, not by time:
// - the boot's spinning Metallica star (sub_824C2CB0: the four vertices of the mesh at [0x82C79164 + 80] turned about
//   their center by 0.06 radians each frame, 3.6 a second at 60);
// - the icon the script function DisplayLoadingScreen can give it (sub_823F82F0, its state at 0x82847B80, set through
//   sub_823F7E78) turns by a whole number of degrees each frame (+48, added to the angle at +44; not seen);
// - in its mode 2 (not seen) a grid waves with two phases that advance by constants each frame.
// Above 60 Hz the steps are scaled by 60 / the rate; the icon's fraction of a degree is carried to the next frame.
// (Its flipbook, sub_824C2F00, already runs on the time base.)
namespace {

double FrameStepScale() { return 60.0 / VblankRate(); }

}  // namespace

// [[midasm_hook]] 0x824C2DC4 (sub_824C2CB0): after "lfd f31,4208(r10)", f31 = the star's turn in radians each frame.
void GHMetallicaLoadingStarHook(PPCRegister& f31) {
  if (HighRate()) {
    f31.f64 *= FrameStepScale();
  }
}

// [[midasm_hook]] 0x823F833C (sub_823F82F0): before "add r11,r10,r11", r10 = the icon's step in degrees, r11 = its angle.
void GHMetallicaLoadingSpinnerHook(PPCRegister& r10) {
  if (!HighRate()) {
    return;
  }
  static double carry = 0.0;  // only the render thread draws the loading screen
  const double step = double(int32_t(r10.u32)) * FrameStepScale() + carry;
  const double whole = std::floor(step);
  carry = step - whole;
  r10.u64 = uint64_t(int64_t(whole));
}

// [[midasm_hook]] 0x824C3250 and 0x824C3260 (sub_824C3028's mode 2): before "fadds f0,f0,f29" and "fadds f0,f13,f0",
// f29 / f0 = the step each wave phase advances by.
void GHMetallicaLoadingWaveHook(PPCRegister& step) {
  if (HighRate()) {
    step.f64 = double(float(step.f64 * FrameStepScale()));
  }
}
