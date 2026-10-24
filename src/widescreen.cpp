// ghmetallica - ultrawide displays (see widescreen.h).

#include "widescreen.h"

#include <native_x_renderer/native_x_renderer.h>
#include <rex/cvar.h>
#include <rex/logging.h>
#include <rex/ppc/context.h>
#include <rex/system/kernel_state.h>
#include <rex/system/xmemory.h>

#include <atomic>
#include <cstring>

REXCVAR_DEFINE_BOOL(ghmetallica_ultrawide, false, "GHMetallica",
                    "Ultrawide on the Xenos path: the game's cameras use the window's aspect ratio (anamorphic, the "
                    "presenter stretches the 16:9 image; needs present_letterbox = false). With NativeXRenderer, "
                    "native_x_renderer_fill_screen does this instead");
REXCVAR_DEFINE_BOOL(ghmetallica_ultrawide_hud, true, "GHMetallica",
                    "With ghmetallica_ultrawide: keep the HUD, the menus and the videos at 16:9 in the center "
                    "(false = stretched)");

namespace ghmetallica {
namespace {

constexpr double kGuestAspect = 16.0 / 9.0;

// The engine's screen aspect ratio (SetScreen { aspect = ... }, sub_8238F918 stores it), which its cameras read.
constexpr uint32_t kScreenAspect = 0x82AEF688;

// Factor applied to the 16:9 aspect ratio (1 = unchanged).
std::atomic<double> g_aspect_factor{1.0};
// The aspect ratio the game set, 0 until it does.
std::atomic<float> g_game_aspect{0.0f};

uint8_t* GuestBase() {
  auto* kernel = rex::system::kernel_state();
  return kernel && kernel->memory() ? kernel->memory()->virtual_membase() : nullptr;
}

// The aspect ratio the cameras get for the one the game set: a 16:9 one is widened by the factor.
float Widen(float aspect) {
  double factor = g_aspect_factor.load(std::memory_order_relaxed);
  if (factor == 1.0 || aspect < 1.70f || aspect > 1.86f) {
    return aspect;
  }
  return float(aspect * factor);
}

void StoreScreenAspect(float aspect) {
  uint8_t* base = GuestBase();
  if (!base) {
    return;
  }
  uint32_t bits;
  std::memcpy(&bits, &aspect, 4);
  bits = __builtin_bswap32(bits);
  std::memcpy(base + kScreenAspect, &bits, 4);
}

}  // namespace

void SetOutputSize(uint32_t width, uint32_t height) {
  double factor = 1.0;
  if (native_x_renderer::Renders()) {
    // NativeXRenderer renders at the output's aspect ratio when it fills it (the profile says these hooks widen the
    // cameras): the hooks follow its factor, whatever ghmetallica_ultrawide says. They handle wider outputs only.
    double native = native_x_renderer::AspectFactor();
    if (native > 1.0) {
      factor = native;
    } else if (native < 1.0) {
      static std::atomic<bool> warned{false};
      if (!warned.exchange(true)) {
        REXLOG_WARN("[ultrawide] NativeXRenderer fills an output taller than 16:9 ({:.4f}); the game's hooks only "
                    "widen views, so the image is stretched vertically (native_x_renderer_fill_screen = false "
                    "keeps 16:9 with bars)",
                    native);
      }
    }
  } else if (REXCVAR_GET(ghmetallica_ultrawide) && width > 0 && height > 0) {
    double aspect = double(width) / double(height);
    if (aspect > kGuestAspect) {
      factor = aspect / kGuestAspect;
    }
  }
  if (g_aspect_factor.exchange(factor) != factor) {
    REXLOG_INFO("[ultrawide] output {}x{}: aspect factor {:.4f}", width, height, factor);
    // The window changed after the game set its aspect ratio: the cameras read the global every frame.
    float game_aspect = g_game_aspect.load(std::memory_order_relaxed);
    if (game_aspect > 0.0f) {
      StoreScreenAspect(Widen(game_aspect));
    }
  }
}

}  // namespace ghmetallica

namespace {

constexpr uint32_t kRootWindow = 0x56A1EAE3;  // QBKey("root_window")

float LoadFloat(const uint8_t* base, uint32_t address) {
  uint32_t bits;
  std::memcpy(&bits, base + address, 4);
  bits = __builtin_bswap32(bits);
  float value;
  std::memcpy(&value, &bits, 4);
  return value;
}

void StoreFloat(uint8_t* base, uint32_t address, float value) {
  uint32_t bits;
  std::memcpy(&bits, &value, 4);
  bits = __builtin_bswap32(bits);
  std::memcpy(base + address, &bits, 4);
}

uint32_t LoadWord(const uint8_t* base, uint32_t address) {
  uint32_t value;
  std::memcpy(&value, base + address, 4);
  return __builtin_bswap32(value);
}

}  // namespace

// [[midasm_hook]] at 0x82376084 in sub_82375E08, a screen element's properties summed with its parent's (r31 = the
// element, its parent at +100): after the branch for an element without a parent, which copies its own properties
// (+160..+236) to the summed ones (+400..+476: +440 the absolute position, +456 the scale). The screen elements (the
// menus, the HUD, the note highway) are laid out on a 1280x720 canvas under root_window, and every child adds its
// position scaled by its parent's scale to its parent's absolute position. Narrowing root_window's scale and moving
// its position to the center keeps the whole 2D layer at 16:9 in the middle of the stretched image.
void GHMetallicaRootElementHook(PPCRegister& r31) {
  double factor = ghmetallica::g_aspect_factor.load(std::memory_order_relaxed);
  if (factor == 1.0 || !REXCVAR_GET(ghmetallica_ultrawide_hud)) {
    return;
  }
  uint8_t* base = ghmetallica::GuestBase();
  uint32_t element = r31.u32;
  if (!base || !element || LoadWord(base, element + 100) != 0 || LoadWord(base, element + 44) != kRootWindow) {
    return;
  }
  static std::atomic<bool> logged{false};
  if (!logged.exchange(true)) {
    REXLOG_INFO("[ultrawide] 2D layer kept at 16:9 in the center ({:.4f})", factor);
  }
  float x = LoadFloat(base, element + 440);
  StoreFloat(base, element + 440, float(640.0 - 640.0 / factor + x / factor));
  StoreFloat(base, element + 456, float(LoadFloat(base, element + 456) / factor));
}

// [[midasm_hook]] at the entry of sub_8238F918, which stores the screen's aspect ratio (f1) for the cameras
// (SetScreen { aspect = ... }): a 16:9 one is widened by the window's factor.
void GHMetallicaScreenAspectHook(PPCRegister& f1) {
  float aspect = float(f1.f64);
  ghmetallica::g_game_aspect.store(aspect, std::memory_order_relaxed);
  float widened = ghmetallica::Widen(aspect);
  if (widened != aspect) {
    REXLOG_INFO("[ultrawide] screen aspect {:.4f} -> {:.4f}", aspect, widened);
    f1.f64 = widened;
  }
}
