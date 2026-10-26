// This game's palette, from Gustavo's mockup: every color the launcher draws, by name (ui.h, the pages). Each game
// has its own, like game.h; the names are the same in every game. GH Metallica's (local_docs/launcher_design, 2026-10-25):
// neutral blacks and a lime accent.

#pragma once

#include "ui/color.h"

namespace ui::col {

inline constexpr ImU32 kBackground = Hex(0x0E0E10);
inline constexpr ImU32 kSidebar = Hex(0x131316);
inline constexpr ImU32 kSidebarBorder = Hex(0x24242A);
inline constexpr ImU32 kCard = Hex(0x17171A);
inline constexpr ImU32 kCardRaised = Hex(0x1D1D21);
inline constexpr ImU32 kCardSelected = Hex(0x1F1F24);
inline constexpr ImU32 kChip = Hex(0x222227);
inline constexpr ImU32 kField = Hex(0x222227);
inline constexpr ImU32 kFieldBorder = Hex(0x2E2E35);
inline constexpr ImU32 kFieldDisabled = Hex(0x1A1A1E);
inline constexpr ImU32 kDivider = Hex(0x222227);
inline constexpr ImU32 kTrack = Hex(0x2A2A30);
inline constexpr ImU32 kSegment = Hex(0x1F1F24);
inline constexpr ImU32 kOutline = Hex(0x2E2E35);
inline constexpr ImU32 kDashed = Hex(0x3A3A42);
inline constexpr ImU32 kGlyph = Hex(0x1D1D21);
inline constexpr ImU32 kRadioOff = Hex(0x5A5A64);
inline constexpr ImU32 kText = Hex(0xEDEDEF);
inline constexpr ImU32 kTextSoft = Hex(0xC9C9D0);
inline constexpr ImU32 kTextSecondary = Hex(0xBDBDC4);
inline constexpr ImU32 kTextMuted = Hex(0xA9A9B2);
inline constexpr ImU32 kLabel = Hex(0x93939C);
inline constexpr ImU32 kTextDim = Hex(0x80808A);
inline constexpr ImU32 kAccent = Hex(0xC5DC3A);
inline constexpr ImU32 kAccentLight = Hex(0xDCEB7A);
inline constexpr ImU32 kOnAccent = Hex(0x151A03);
inline constexpr ImU32 kAccentTint = Hex(0x262B12);
inline constexpr ImU32 kDonate = Hex(0xEF5B4C);
inline constexpr ImU32 kOnDonate = Hex(0x1F0605);
inline constexpr ImU32 kSuccess = Hex(0x5BC489);
inline constexpr ImU32 kSuccessTint = Hex(0x1B3A2A);
inline constexpr ImU32 kError = Hex(0xF07B6B);
inline constexpr ImU32 kErrorTint = Hex(0x3A1F1C);
inline constexpr ImU32 kGlyphA = Hex(0x5BC489);
inline constexpr ImU32 kGlyphB = Hex(0xF07B6B);
inline constexpr ImU32 kGlyphX = Hex(0x6AA6F2);
inline constexpr ImU32 kGlyphY = Hex(0xE8C547);

// The colors of a single place each.
inline constexpr ImU32 kHeroFill = Hex(0x111113);          // under the Play page's art
inline constexpr ImU32 kHeroButton = Hex(0x131316);        // the Play page's Exit button, over the art
inline constexpr ImU32 kHeroButtonBorder = Hex(0x2E2E35);
inline constexpr ImU32 kNoticeTint = Hex(0x17281F);        // a success notice's background (the Play page)
inline constexpr ImU32 kInstallArtFill = Hex(0x111113);    // under the first run's art
inline constexpr ImU32 kDropZone = Hex(0x111114);          // the first run's ISO drop zone
inline constexpr ImU32 kValidBorder = Hex(0x2F5A42);       // the card of an ISO that was verified
inline constexpr ImU32 kErrorPanel = Hex(0x1C1415);        // the card of an ISO that was not
inline constexpr ImU32 kErrorPanelBorder = Hex(0x5A2C26);
inline constexpr ImU32 kErrorBadge = Hex(0x3A1E1B);
inline constexpr ImU32 kDoneTile = Hex(0x1B3326);          // the check of the finished installation
inline constexpr ImU32 kScrollbarActive = Hex(0x4A4A54);   // a scrollbar's thumb while dragged

// The black of the Play page's title bar and of the first run's title card, over the art (game::kHeroTitleBar,
// game::kInstallTitleCard), and the line of accent above them.
inline constexpr ImU32 kTitleBar = IM_COL32(10, 10, 12, 240);

// Every corner's radius is the mockup's times this (ui.cpp): GH Metallica's mockup rounds cards 8 px and controls 6 px
// where the launcher's first one rounded them 12 and 8.
inline constexpr float kRoundness = 0.7f;
// The first run's step markers' corners (before kRoundness): squares rounded 4 px; 14 makes them circles.
inline constexpr float kStepRadius = 6.0f;

}  // namespace ui::col
