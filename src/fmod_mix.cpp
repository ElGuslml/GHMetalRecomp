// ghmetallica - FMOD mixes each block of sound as soon as the sound device has read the one before.
//
// The game's music and sounds go through FMOD's polled output (sub_82280600): FMOD's mixer thread calls it about every
// millisecond, and it mixes 256-sample blocks into a ring of two, from its cursor up to the block the game's render
// client is reading (a position it reads before taking its lock). The render client (sub_82228A38) runs on the SDK's
// audio worker, once for each frame the sound device plays: it mixes its voices, FMOD's ring among them, and submits
// the frame. When FMOD's thread came more than one block (5.3 ms) late, the reader had gone round the ring and the
// blocks in between were never mixed: the song's streams stayed behind the notes by them until a pause (8 ms late
// once a second: half a second in a minute; local_docs/22). Here FMOD's update runs on the audio worker right after
// each frame the render client reads, so each block is mixed as soon as it is free whatever FMOD's thread does, and
// FMOD's thread only polls. A lock keeps the two from mixing at once while the worker takes over (both mixing from
// the same position mixed blocks twice: the music ran ahead).

#include <rex/cvar.h>
#include <rex/hook.h>
#include <rex/ppc/context.h>

#include <atomic>
#include <cstdint>
#include <mutex>

REXCVAR_DEFINE_BOOL(ghmetallica_fmod_on_audio_thread, true, "GHMetallica",
                    "FMOD mixes each block of sound on the SDK's audio worker as soon as the sound device has read "
                    "the one before, so FMOD's own thread coming late loses nothing; false: FMOD's thread mixes, as on "
                    "the console, and a late one leaves the music behind the notes until a pause");

namespace {

std::atomic<uint32_t> g_output{0};      // FMOD's polled output (sub_82280600's r3), from FMOD's thread's calls
std::atomic<bool> g_on_worker{false};  // the audio worker mixes: FMOD's thread no longer does
std::mutex g_mix;

}  // namespace

REX_EXTERN(__imp__sub_82280600);
REX_EXTERN(__imp__sub_82228A38);

// The render client (sub_82228A38), on the SDK's audio worker for each frame the sound device plays.
REX_HOOK_RAW(sub_82228A38) {
  __imp__sub_82228A38(ctx, base);
  const uint32_t output = g_output.load(std::memory_order_acquire);
  if (!output || !REXCVAR_GET(ghmetallica_fmod_on_audio_thread)) {
    g_on_worker.store(false, std::memory_order_release);
    return;
  }
  const uint64_t r3 = ctx.r3.u64;
  ctx.r3.u64 = output;
  {
    std::scoped_lock lock(g_mix);
    __imp__sub_82280600(ctx, base);
  }
  ctx.r3.u64 = r3;
  g_on_worker.store(true, std::memory_order_release);
}

// FMOD's polled output's update (sub_82280600, r3 the output), on FMOD's mixer thread.
REX_HOOK_RAW(sub_82280600) {
  g_output.store(ctx.r3.u32, std::memory_order_release);
  if (g_on_worker.load(std::memory_order_acquire)) {
    ctx.r3.u64 = 0;  // FMOD_OK
    return;
  }
  std::scoped_lock lock(g_mix);
  __imp__sub_82280600(ctx, base);
}
