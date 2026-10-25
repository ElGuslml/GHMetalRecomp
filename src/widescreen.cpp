// ghmetallica - ultrawide displays (see widescreen.h).

#include "widescreen.h"

#include <native_x_renderer/native_x_renderer.h>
#include <rex/cvar.h>
#include <rex/logging.h>
#include <rex/ppc/context.h>
#include <rex/system/kernel_state.h>
#include <rex/system/xmemory.h>

#include <atomic>
#include <cmath>
#include <cstring>

REXCVAR_DEFINE_BOOL(ghmetallica_ultrawide, false, "GHMetallica",
                    "Ultrawide on the Xenos path: the game's 3D views see as much more at the sides as the window is "
                    "wider than 16:9 (anamorphic, the presenter stretches the 1280x720 image; needs "
                    "present_letterbox = false). With NativeXRenderer, native_x_renderer_fill_screen does this instead");
REXCVAR_DEFINE_BOOL(ghmetallica_ultrawide_hud, true, "GHMetallica",
                    "With ghmetallica_ultrawide: keep the HUD, the menus and the videos at 16:9 in the center "
                    "(false = stretched)");

namespace ghmetallica {
namespace {

constexpr double kGuestAspect = 16.0 / 9.0;

// Factor the 16:9 views are widened by (1 = unchanged).
std::atomic<double> g_aspect_factor{1.0};

uint8_t* GuestBase() {
  auto* kernel = rex::system::kernel_state();
  return kernel && kernel->memory() ? kernel->memory()->virtual_membase() : nullptr;
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
  }
}

}  // namespace ghmetallica

namespace {

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

void StoreWord(uint8_t* base, uint32_t address, uint32_t value) {
  value = __builtin_bswap32(value);
  std::memcpy(base + address, &value, 4);
}

}  // namespace

namespace {

// The D3D device ([0x82AA1574], the renderer block 0x82AA0C80 + 2292) and the fields the hooks read: render target 0
// (+12448), a surface's size (+36: width - 1 in bits 18-31, height - 1 in bits 3-17), and what BeginVertices saved for
// the inline draw that EndVertices commits: its vertex data (+13448) and its size in dwords (+13456).
constexpr uint32_t kDevicePointer = 0x82AA1574;
constexpr uint32_t kRenderTarget0 = 12448;
constexpr uint32_t kVsConstants = 1920;  // VS float constants, c0 first
constexpr uint32_t kSurfaceSize = 36;
constexpr uint32_t kInlineVertexData = 13448;
constexpr uint32_t kInlineVertexDwords = 13456;

// The factor the 2D layer is narrowed by, or 1 when it is not: ultrawide on, its HUD option on, and the draw going to
// the screen (render target 0 of 1280x720), not to a texture.
double NarrowFactor(const uint8_t* base) {
  double factor = ghmetallica::g_aspect_factor.load(std::memory_order_relaxed);
  if (factor == 1.0 || !REXCVAR_GET(ghmetallica_ultrawide_hud) || !base) {
    return 1.0;
  }
  uint32_t device = LoadWord(base, kDevicePointer);
  uint32_t surface = device ? LoadWord(base, device + kRenderTarget0) : 0;
  if (!surface) {
    return 1.0;
  }
  uint32_t size = LoadWord(base, surface + kSurfaceSize);
  uint32_t width = ((size >> 18) & 0x3FFF) + 1, height = ((size >> 3) & 0x7FFF) + 1;
  return width == 1280 && height == 720 ? factor : 1.0;
}

// The vertices' x (the first float of each, in pixels of the 1280x720 canvas) narrowed around the center.
void NarrowVertices(uint8_t* base, double factor, uint32_t data, uint32_t count, uint32_t stride) {
  static std::atomic<bool> logged{false};
  if (!logged.exchange(true)) {
    REXLOG_INFO("[ultrawide] 2D layer kept at 16:9 in the center ({:.4f})", factor);
  }
  for (uint32_t i = 0; i < count; ++i) {
    uint32_t x = data + i * stride;
    StoreFloat(base, x, float(640.0 + (LoadFloat(base, x) - 640.0) / factor));
  }
}

}  // namespace

// The screen elements (the menus, the HUD, the note highway and its notes, the text) are drawn in pixels of a 1280x720
// canvas. Their vertices are narrowed around the center just before they are drawn, so the whole 2D layer stays 16:9 in
// the middle of the stretched image, whatever its positions came from: the note highway's gems and its dark backdrop
// are placed by the highway's own code, not through root_window, and narrowing root_window left them stretched.
//
// [[midasm_hook]] at 0x82421D78 in sub_82421B60, a sprite's quad: r6 = its 4 vertices (0x82BE20E0), r7 = their stride
// (28 or 36), before DrawVerticesUP.
void GHMetallicaSpriteHook(PPCRegister& r6, PPCRegister& r7) {
  uint8_t* base = ghmetallica::GuestBase();
  double factor = NarrowFactor(base);
  if (factor != 1.0) {
    NarrowVertices(base, factor, r6.u32, 4, r7.u32);
  }
}

// [[midasm_hook]] at 0x82454A68 in sub_82454990, a batch of quads (the text): r5 = the vertex count, r6 = the vertices
// (0x82C29CE0), r7 = their stride, before DrawVerticesUP.
void GHMetallicaSpriteBatchHook(PPCRegister& r5, PPCRegister& r6, PPCRegister& r7) {
  uint8_t* base = ghmetallica::GuestBase();
  double factor = NarrowFactor(base);
  if (factor != 1.0) {
    NarrowVertices(base, factor, r6.u32, r5.u32, r7.u32);
  }
}

// [[midasm_hook]] at 0x82420B84 in sub_82420A20, a screen element's clip window (used by the text and the sprites of
// elements that scroll or roll, as the star rating's number in the score box): r4 = its rectangle {x1, y1, x2, y2}
// (ints, pixels of the 1280x720 target), before SetScissorRect. The vertices it clips are narrowed around the center,
// so the rectangle is narrowed as well; left as it was, it cut away what moved out of it (a small window, as the
// star's number, showed nothing).
void GHMetallicaClipWindowHook(PPCRegister& r4) {
  uint8_t* base = ghmetallica::GuestBase();
  double factor = NarrowFactor(base);
  if (factor == 1.0) {
    return;
  }
  // The engine clamps the window to the target: one wholly off the screen arrives empty (1279 to 1279) and stays so.
  int32_t x1 = int32_t(LoadWord(base, r4.u32)), x2 = int32_t(LoadWord(base, r4.u32 + 8));
  int32_t n1 = int32_t(std::floor(640.0 + (x1 - 640.0) / factor));
  int32_t n2 = x2 > x1 ? int32_t(std::ceil(640.0 + (x2 - 640.0) / factor)) : n1;
  StoreWord(base, r4.u32, uint32_t(n1));
  StoreWord(base, r4.u32 + 8, uint32_t(n2));
}

// [[midasm_hook]] at 0x821B5A74 in sub_821B4FB0, a sustained note's tail (a triangle strip, before DrawVerticesUP: r5 =
// the vertex count, r6 = the vertices at 0x82A0E440, 24 bytes each, built by sub_821B34B0). Its vertices are in the
// lane's own space (x: half its width, from 5 at the far end to 21 at the strike line; y: along the highway), and its
// vertex shader places them on the canvas with VS constant c0, which sub_821B4FB0 writes for each tail just before:
// x = c0.x (the lane's x at the far end) + x + c0.w (the lane's x from there to the strike line) * c0.z * (y - c0.y).
// Narrowing that x around the center: the vertices' x divided by the factor, c0.x moved toward 640, c0.w divided.
void GHMetallicaTailHook(PPCRegister& r5, PPCRegister& r6, PPCRegister& r7) {
  uint8_t* base = ghmetallica::GuestBase();
  double factor = NarrowFactor(base);
  if (factor == 1.0) {
    return;
  }
  for (uint32_t i = 0; i < r5.u32; ++i) {
    uint32_t x = r6.u32 + i * r7.u32;
    StoreFloat(base, x, float(LoadFloat(base, x) / factor));
  }
  uint32_t c0 = LoadWord(base, kDevicePointer) + kVsConstants;
  StoreFloat(base, c0, float(640.0 + (LoadFloat(base, c0) - 640.0) / factor));
  StoreFloat(base, c0 + 12, float(LoadFloat(base, c0 + 12) / factor));
}

// [[midasm_hook]] at 0x821B3498 in sub_821B2ED8, a sprite drawn as a grid of quads (the highway's board): r3 = the
// device, before EndVertices; the vertices BeginVertices handed out are 20 bytes each (x, y, color, u, v).
void GHMetallicaMeshSpriteHook(PPCRegister& r3) {
  uint8_t* base = ghmetallica::GuestBase();
  double factor = NarrowFactor(base);
  if (factor != 1.0) {
    uint32_t data = LoadWord(base, r3.u32 + kInlineVertexData);
    uint32_t dwords = LoadWord(base, r3.u32 + kInlineVertexDwords);
    NarrowVertices(base, factor, data, dwords / 5, 20);
  }
}

// [[midasm_hook]] at the entry of sub_82356538, which builds a perspective projection from its frustum at the near
// plane (f1, f2 = left, right; f3, f4 = bottom, top; the row-vector convention). Its callers (the views of
// sub_82404950 and sub_82497998, among others) take the frustum's width from the camera's horizontal field of view and
// its height from the view's aspect ratio, its size (1280x720 for the screen's views): a wider screen would just
// stretch it. A 16:9 frustum is widened by the factor (Hor+: the vertical field of view kept, more seen at the sides),
// the same narrowing the 2D layer gets in the stretched image, so 3D drawn to match it (the note highway's depth mask)
// still matches. Other shapes (the crowd's impostors, 1:2) are left alone.
void GHMetallicaFrustumHook(PPCRegister& f1, PPCRegister& f2, PPCRegister& f3, PPCRegister& f4) {
  double factor = ghmetallica::g_aspect_factor.load(std::memory_order_relaxed);
  double height = std::fabs(f4.f64 - f3.f64);
  if (factor == 1.0 || height == 0.0) {
    return;
  }
  double aspect = std::fabs(f2.f64 - f1.f64) / height;
  if (aspect < 1.70 || aspect > 1.86) {
    return;
  }
  static std::atomic<bool> logged{false};
  if (!logged.exchange(true)) {
    REXLOG_INFO("[ultrawide] 3D views widened ({:.4f})", factor);
  }
  f1.f64 *= factor;
  f2.f64 *= factor;
}
