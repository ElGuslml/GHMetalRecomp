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
//
// Sleeping in slices still woke each waiting thread thousands of times a second, and each wake costs the kernel a
// timer and a context switch: in a song the waiting threads spent more time in the kernel than in the game. So the
// waits that can are smarter (docs/cpu-usage.md):
// - A job worker with an empty queue blocks on the job event itself, which the game pulses (NtPulseEvent) when it
//   queues a job: it wakes at once for a job instead of within a slice, and otherwise once a millisecond.
// - The main thread waiting for a group of jobs to finish (sub_822BAD30: while (pending[group] != 0) run the
//   group's jobs, sub_82425D30) took the queue's critical section again and again while the workers ran them, the
//   lock they need to finish: between its passes it now watches the count without the lock.
// - The render thread sleeps in one go until shortly before the vertical blank it waits for, which comes every
//   1/60 s.

#include <rex/cvar.h>
#include <rex/ppc/context.h>
#include <rex/system/kernel_state.h>
#include <rex/system/xevent.h>
#include <rex/system/xmemory.h>
#include <rex/thread.h>

#include <immintrin.h>

#include <algorithm>
#include <chrono>
#include <cstring>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

REXCVAR_DEFINE_INT32(ghmetallica_wait_sleep_us, 200, "GHMetallica",
                     "Game threads waiting for work: sleep this many microseconds per check once the wait "
                     "is longer than ghmetallica_wait_spin_us (0 = busy wait, as on the console)");
REXCVAR_DEFINE_INT32(ghmetallica_wait_spin_us, 100, "GHMetallica",
                     "Game threads waiting for work: busy-wait this long before sleeping");
REXCVAR_DEFINE_BOOL(ghmetallica_wait_smart, true, "GHMetallica",
                    "Job workers block on the game's job event, the main thread waits for its jobs without taking "
                    "the queue's lock, the render thread sleeps until shortly before the vertical blank (false = "
                    "every wait sleeps in slices of ghmetallica_wait_sleep_us)");

namespace {

using Clock = std::chrono::steady_clock;
using namespace std::chrono_literals;

// A check that comes more than this long after the previous one on the same thread starts a new wait: the loop
// did other work in between (or the thread went on with something else and came back).
constexpr auto kNewWaitGap = std::chrono::milliseconds(1);

struct WaitState {
  Clock::time_point start;
  Clock::time_point last;
};

bool Smart() { return REXCVAR_GET(ghmetallica_wait_smart) && REXCVAR_GET(ghmetallica_wait_sleep_us) > 0; }

uint8_t* GuestBase() {
  auto* kernel = rex::system::kernel_state();
  return kernel && kernel->memory() ? kernel->memory()->virtual_membase() : nullptr;
}

uint32_t LoadWord(const uint8_t* base, uint32_t address) {
  uint32_t value;
  std::memcpy(&value, base + address, 4);
  return __builtin_bswap32(value);
}

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

// The job queue's event: sub_82425A18 loads its handle with lwz r3,-10892(r29), r29 = 0x82A90000.
constexpr uint32_t kJobEventHandle = 0x82A8D574;
// The longest a worker blocks on it: a pulse that comes between its check of the queue and its wait is missed,
// and the job then waits this long for it (or the main thread, waiting for its group, runs it itself).
constexpr auto kJobEventTimeout = 1ms;

struct JobWait {
  HANDLE timer = nullptr;  // high resolution: a plain wait's timeout has the system timer's 15.6 ms steps
  int immediate = 0;       // waits the event ended at once in a row
  ~JobWait() {
    if (timer) {
      CloseHandle(timer);
    }
  }
};
thread_local JobWait t_job_event;

// XEvent's host event, which the SDK keeps protected (only its own waits use it): a pointer to the member, formed in
// a derived class that is never instantiated.
struct EventHandleAccess : rex::system::XEvent {
  static rex::thread::WaitHandle* Get(rex::system::XEvent* event) {
    return (event->*&EventHandleAccess::GetWaitHandle)();
  }
};

// Blocks until the game pulses the job event or kJobEventTimeout passes. False if it cannot (the caller sleeps a
// slice instead), or now and then when the event stays signaled, which would end every wait at once.
bool WaitForJob(JobWait& state) {
  auto* kernel = rex::system::kernel_state();
  uint8_t* base = GuestBase();
  if (!kernel || !base) {
    return false;
  }
  if (state.immediate >= 8) {
    state.immediate = 0;
    return false;
  }
  auto event = kernel->object_table()->LookupObject<rex::system::XEvent>(LoadWord(base, kJobEventHandle));
  rex::thread::WaitHandle* wait = event ? EventHandleAccess::Get(event.get()) : nullptr;
  if (!wait) {
    return false;
  }
  if (!state.timer) {
    state.timer = CreateWaitableTimerExW(nullptr, nullptr, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, TIMER_ALL_ACCESS);
    if (!state.timer) {
      return false;
    }
  }
  LARGE_INTEGER due;
  due.QuadPart = -std::chrono::duration_cast<std::chrono::nanoseconds>(kJobEventTimeout).count() / 100;
  if (!SetWaitableTimerEx(state.timer, &due, 0, nullptr, nullptr, nullptr, 0)) {
    return false;
  }
  HANDLE handles[2] = {wait->native_handle(), state.timer};
  const Clock::time_point start = Clock::now();
  const DWORD result = WaitForMultipleObjects(2, handles, FALSE, INFINITE);
  if (result == WAIT_OBJECT_0 && Clock::now() - start < 5us) {
    ++state.immediate;
  } else {
    state.immediate = 0;
  }
  return result == WAIT_OBJECT_0 || result == WAIT_OBJECT_0 + 1;
}

// A vertical blank counter's last advance seen while the render thread was checking it closely, the anchor of
// when the next ones come: the SDK's vblank thread advances it every 1/60 s, within a millisecond (it checks the
// time every millisecond).
struct Vblank {
  uint64_t value = 0;
  Clock::time_point checked;   // the previous check
  uint64_t anchor_value = 0;   // 0: no anchor yet
  Clock::time_point anchor_time;
  WaitState wait;
};
thread_local Vblank t_vblank[2];

constexpr auto kVblankPeriod = std::chrono::nanoseconds(1'000'000'000 / 60);
// How long before the expected advance the long sleep ends, for the vblank thread's millisecond and the sleep's
// own lateness; checked in slices from there.
constexpr auto kVblankMargin = 2ms;

void VblankWait(Vblank& state, uint64_t counter, uint64_t target) {
  const Clock::time_point now = Clock::now();
  if (counter != state.value) {
    // An advance seen within a slice of the previous check is a good anchor; one found after other work is not
    // (it may have come long before).
    if (now - state.checked < 500us) {
      state.anchor_value = counter;
      state.anchor_time = now;
    }
    state.value = counter;
  }
  state.checked = now;
  if (counter >= target) {
    return;
  }
  if (Smart() && state.anchor_value && target > state.anchor_value) {
    const Clock::time_point due = state.anchor_time + (target - state.anchor_value) * kVblankPeriod;
    if (due - now > kVblankMargin + 1ms) {
      rex::thread::PreciseSleep(std::chrono::duration_cast<std::chrono::microseconds>(due - now - kVblankMargin));
      state.checked = state.wait.last = Clock::now();
      return;
    }
  }
  SpinOrSleep(state.wait);
  state.checked = Clock::now();
}

}  // namespace

// [[midasm_hook]] 0x82425A78 (sub_82425A18, a job worker with an empty queue): before "li r4,0" and the
// WaitForSingleObject(event, 0) that follows, once per check of the queue.
void GHMetallicaJobWaitHook() {
  if (Smart() && WaitForJob(t_job_event)) {
    return;
  }
  SpinOrSleep(t_job_wait);
}

// [[midasm_hook]] 0x822BADA4 (sub_822BAD30, the main thread waiting for a job group: while (pending[group] != 0)
// sub_82425D30 runs the group's queued jobs): after a pass, r29 = the job system (0x82A8D980), r30 = group * 32,
// the group's count of jobs not finished at [[r29 + 20] + r30]. While the workers run them, the count is watched
// without the queue's critical section, for up to 20 us or until a job finishes; then the next pass.
void GHMetallicaJobGroupWaitHook(PPCRegister& r29, PPCRegister& r30) {
  uint8_t* base = GuestBase();
  if (!base || !Smart()) {
    return;
  }
  const uint32_t pending_address = LoadWord(base, r29.u32 + 20) + r30.u32;
  const uint32_t pending = LoadWord(base, pending_address);
  if (pending == 0) {
    return;
  }
  const Clock::time_point start = Clock::now();
  while (LoadWord(base, pending_address) == pending && Clock::now() - start < 20us) {
    _mm_pause();
  }
}

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

// [[midasm_hook]] 0x8241C954 (sub_8241C840, the render thread presenting a frame): before "cmpld cr6,r3,r11",
// r3 = the vertical blank counter 0x82A925A0 just read and r11 the value it waits for (go on once r3 reaches it).
void GHMetallicaVblankWaitHook(PPCRegister& r3, PPCRegister& r11) { VblankWait(t_vblank[0], r3.u64, r11.u64); }

// [[midasm_hook]] 0x8241CA9C (sub_8241C840): before "cmpld cr6,r3,r30", r3 = the counter 0x82A92590 and r30 the
// frame's last vertical blank.
void GHMetallicaVblankWait2Hook(PPCRegister& r3, PPCRegister& r30) { VblankWait(t_vblank[1], r3.u64, r30.u64); }
