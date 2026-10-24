// ghmetallica - the game's threads wait without spinning.
//
// Three loops of the game wait by checking a condition again and again, which on the console costs a
// hardware thread each and nothing else. Here each one kept a core at 100 % (5 cores in the menus, measured
// with recomp-tools' threadcpu.ps1 and stacksample.ps1):
// - The job system's three workers (sub_82425AC8 -> sub_82425D30): while the job queue is empty
//   (sub_82425A18), they call WaitForSingleObject(event, 0) on the event the game pulses when it queues a job,
//   a check that returns at once.
// - The main thread (sub_822E4588), at the start of each frame: while (sub_8243C3D8(0x82C129E8) != 0) {},
//   a getter of the object's field 108, without a pause; and before it (sub_822E0D68 -> sub_822E0C10)
//   while (frames[24] == frames[20]) {} on 0x82AA0C20, until the render thread has finished a frame (it counts
//   them in sub_822E0CE8). In the menus the render thread is ahead and this wait is short; in a song it is not.
// - The render thread (sub_823A4C90 -> sub_823A4588 -> sub_822DB7C0 -> sub_8241C840, the frame's
//   presentation), waiting for the vertical blank twice per frame: until the counter sub_822D0E70 returns
//   (0x82A925A0) reaches the next value, and until sub_822D0E80's (0x82A92590) reaches the frame's last one;
//   the counters are advanced by the game's vertical blank interrupt.
// The hooks in ghmetallica_functions.toml leave each loop as it is and slow its checks down: a wait first
// spins briefly (short waits are unchanged), and then the thread sleeps in short slices between checks.

#include <rex/cvar.h>
#include <rex/ppc/context.h>
#include <rex/thread.h>

#include <immintrin.h>

#include <algorithm>
#include <chrono>

REXCVAR_DEFINE_INT32(ghmetallica_wait_sleep_us, 200, "GHMetallica",
                     "Game threads waiting for work: sleep this many microseconds per check once the wait "
                     "is longer than ghmetallica_wait_spin_us (0 = busy wait, as on the console)");
REXCVAR_DEFINE_INT32(ghmetallica_wait_spin_us, 100, "GHMetallica",
                     "Game threads waiting for work: busy-wait this long before sleeping");

namespace {

using Clock = std::chrono::steady_clock;

// A check that comes more than this long after the previous one on the same thread starts a new wait: the loop
// did other work in between (or the thread went on with something else and came back).
constexpr auto kNewWaitGap = std::chrono::milliseconds(1);

struct WaitState {
  Clock::time_point start;
  Clock::time_point last;
};

// One check of a wait whose condition is still false.
void SpinOrSleep(WaitState& state) {
  const int32_t sleep_us = REXCVAR_GET(ghmetallica_wait_sleep_us);
  if (sleep_us <= 0) {
    return;
  }
  const Clock::time_point now = Clock::now();
  if (now - state.last > kNewWaitGap) {
    state.start = now;
  }
  state.last = now;
  const int32_t spin_us = std::max(0, REXCVAR_GET(ghmetallica_wait_spin_us));
  if (now - state.start < std::chrono::microseconds(spin_us)) {
    _mm_pause();
    return;
  }
  rex::thread::PreciseSleep(std::chrono::microseconds(sleep_us));
  state.last = Clock::now();
}

thread_local WaitState t_job_wait;
thread_local WaitState t_frame_wait;
thread_local WaitState t_render_wait;
thread_local WaitState t_vblank_wait;

}  // namespace

// [[midasm_hook]] 0x82425A78 (sub_82425A18, a job worker with an empty queue): before "li r4,0" and the
// WaitForSingleObject(event, 0) that follows, once per check of the queue.
void GHMetallicaJobWaitHook() { SpinOrSleep(t_job_wait); }

// [[midasm_hook]] 0x822E4600 (sub_822E4588, the main thread's frame): before "cmplwi cr6,r3,0", r3 = the
// field the loop waits on (0 = go on).
void GHMetallicaFrameWaitHook(PPCRegister& r3) {
  if (r3.u32 != 0) {
    SpinOrSleep(t_frame_wait);
  }
}

// [[midasm_hook]] 0x822E0C44 (sub_822E0C10, the main thread waiting for the render thread): before
// "cmplw cr6,r11,r10", r11 and r10 = the frame counters (go on once they differ).
void GHMetallicaRenderWaitHook(PPCRegister& r11, PPCRegister& r10) {
  if (r11.u32 == r10.u32) {
    SpinOrSleep(t_render_wait);
  }
}

// [[midasm_hook]] 0x8241C954 and 0x8241CA9C (sub_8241C840, the render thread presenting a frame): before
// "cmpld cr6,r3,r11" / "cmpld cr6,r3,r30", r3 = the vertical blank counter just read and the second register
// the value it waits for (go on once r3 reaches it).
void GHMetallicaVblankWaitHook(PPCRegister& r3, PPCRegister& target) {
  if (r3.u64 < target.u64) {
    SpinOrSleep(t_vblank_wait);
  }
}
