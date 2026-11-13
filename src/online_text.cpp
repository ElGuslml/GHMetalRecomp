// ghmetallica - the main menu's online item named as on the PlayStation 3 ("ONLINE"), not "Xbox LIVE".
//
// Online play here is through Steam between friends, not Xbox Live. The script that builds the main menu (its items
// added with add_mainmenu_item) picks the online item's text by platform:
// `if isXenon online_text = qs(2725B21F) else online_text = qs(E60E3E4A)`. Both texts are in every language's pack on
// the Xbox disc: "Xbox LIVE", and the PlayStation 3's "ONLINE" ("EN LIGNE", "EN LÍNEA"). The game turns a text's key
// into its text in the player's language with sub_823AA990 (r3 = the key; sub_822B5FA8, a script structure's
// localized string, and the other 25 callers): asked for "Xbox LIVE", it is given the key of "ONLINE". The other texts
// that name Xbox LIVE (its messages and hints, keys of their own) are left as they are.

#include <rex/cvar.h>
#include <rex/logging.h>
#include <rex/ppc/context.h>

#include <atomic>
#include <cstdint>

REXCVAR_DEFINE_BOOL(ghmetallica_online_text, true, "GHMetallica",
                    "The main menu's online item named \"ONLINE\" (the game's PlayStation 3 text, in its language), "
                    "not \"Xbox LIVE\" (false: as on the Xbox 360)");

namespace {

constexpr uint32_t kXboxLiveText = 0x2725B21F;  // "Xbox LIVE"
constexpr uint32_t kOnlineText = 0xE60E3E4A;    // "ONLINE", the PlayStation 3's

std::atomic<bool> logged{false};

}  // namespace

// [[midasm_hook]] at 0x823AA99C (stw r3,116(r1)) in sub_823AA990, the text of a key: r3 = the key.
void GHMetallicaTextKeyHook(PPCRegister& r3) {
  if (r3.u32 != kXboxLiveText || !REXCVAR_GET(ghmetallica_online_text)) {
    return;
  }
  r3.u64 = kOnlineText;
  if (!logged.exchange(true)) {
    REXLOG_INFO("[online_text] \"Xbox LIVE\" shown as the game's \"ONLINE\" (ghmetallica_online_text)");
  }
}
