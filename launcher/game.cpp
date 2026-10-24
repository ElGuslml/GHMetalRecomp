// This game's own texts (game.h): the first run's welcome and the Game page's switches, in the launcher's languages
// (English, German, French, Spanish, Italian). None yet: the launcher's own welcome.

#include "game.h"

namespace launcher::game {

const GameTexts& GameTextsFor(uint32_t language) {
  static const GameTexts kEnglish{nullptr, nullptr, {}};
  switch (language) {
    default:
      return kEnglish;
  }
}

}  // namespace launcher::game
