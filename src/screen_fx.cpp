// ghmetallica - the game's post-processing effects, each of which can be turned off.
//
// The engine's screen effects (the scripts' ScreenFX_AddFXInstance and ScreenFX_UpdateFXInstanceParams; the
// instances are made by sub_824B90E8) are created once per scene and then given new parameters by the scripts: at
// boot and at each venue Bloom2, Bright_Sat, DOF3, Noise2, Vignette, a radial blur and one more, plus InkBlotch in a
// song; the menus and the pause menu blur their 3D background with another DOF3. All of them work on the 3D scene:
// the note highway and the HUD are drawn after them.
//
// The final composite, sub_8251EC80, applies six of them in one pass: it picks a variant of the shader
// _Uber_ScreenFX by a mask of the effects that are on, one bit each (_bloom1 1, _dof1 2, _noise1 4, _color1 8,
// _vignette1 16, _inkblot1 32), and sets each effect's constants by the same bits. The hook takes the bits of the
// effects turned off out of that mask: the composite is then the variant without them. The passes that feed them
// (the bloom's and the depth of field's blurred copies) still run; their result is not used.

#include <rex/cvar.h>
#include <rex/logging.h>
#include <rex/ppc/context.h>

#include <atomic>
#include <cstdint>

REXCVAR_DEFINE_BOOL(ghmetallica_fx_bloom, true, "GHMetallica",
                    "Screen effects: the bloom, the glow around lights and bright surfaces; false = off");
REXCVAR_DEFINE_BOOL(ghmetallica_fx_depth_of_field, true, "GHMetallica",
                    "Screen effects: the depth of field, the background blurred in some camera shots and behind the "
                    "menus and the pause menu; false = off");
REXCVAR_DEFINE_BOOL(ghmetallica_fx_grain, true, "GHMetallica",
                    "Screen effects: the film grain over the image; false = off (always off while the color "
                    "correction is off)");
REXCVAR_DEFINE_BOOL(ghmetallica_fx_color, true, "GHMetallica",
                    "Screen effects: the color correction (brightness, contrast, hue, saturation): each venue's look, "
                    "fades to black, the image greyed while the rock meter is red; false = off, and the grain with it");
REXCVAR_DEFINE_BOOL(ghmetallica_fx_vignette, true, "GHMetallica",
                    "Screen effects: the vignette, the corners darkened where a venue or a moment turns it on; "
                    "false = off");
REXCVAR_DEFINE_BOOL(ghmetallica_fx_ink, true, "GHMetallica",
                    "Screen effects: the ink blotches a song adds over the image; false = off");

namespace {

constexpr uint32_t kBloom = 1;
constexpr uint32_t kDepthOfField = 2;
constexpr uint32_t kGrain = 4;
constexpr uint32_t kColor = 8;
constexpr uint32_t kVignette = 16;
constexpr uint32_t kInk = 32;

uint32_t EffectsOff() {
  uint32_t off = 0;
  off |= REXCVAR_GET(ghmetallica_fx_bloom) ? 0 : kBloom;
  off |= REXCVAR_GET(ghmetallica_fx_depth_of_field) ? 0 : kDepthOfField;
  off |= REXCVAR_GET(ghmetallica_fx_grain) ? 0 : kGrain;
  off |= REXCVAR_GET(ghmetallica_fx_color) ? 0 : kColor;
  off |= REXCVAR_GET(ghmetallica_fx_vignette) ? 0 : kVignette;
  off |= REXCVAR_GET(ghmetallica_fx_ink) ? 0 : kInk;
  // The color pass also tones the grain down: without it the grain is far stronger, so it goes with the color.
  if (off & kColor) {
    off |= kGrain;
  }
  return off;
}

}  // namespace

// [[midasm_hook]] 0x8251EEA8 (sub_8251EC80, the screen effects' composite), after "lwz r31,12(r25)": r31 = the mask of
// the effects the composite applies; the shader variant (table[mask]) and each effect's constants follow from it.
void GHMetallicaScreenFxMaskHook(PPCRegister& r31) {
  const uint32_t off = EffectsOff();
  if (!off) {
    return;
  }
  static std::atomic<uint32_t> logged{0};
  if (logged.exchange(off) != off) {
    REXLOG_INFO("[screen fx] effects turned off: mask 0x{:02X} (ghmetallica_fx_*)", off);
  }
  r31.u32 &= ~off;
}
