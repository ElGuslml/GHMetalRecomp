// This game's own texts (game.h): the first run's welcome, the Game page's switches and where the game shows its DLC, in
// the launcher's languages (English, German, French, Spanish, Italian). The launcher's own welcome for now.

#include "game.h"

namespace launcher::game {

const GameTexts& GameTextsFor(uint32_t language) {
  static const GameTexts kEnglish{
      .welcome_title = nullptr,
      .welcome_text = nullptr,
      .toggles = {},
      .dlc_where = "The game lists the songs in Quickplay: in the song list, with the blue button.",
  };
  static const GameTexts kGerman{
      .welcome_title = nullptr,
      .welcome_text = nullptr,
      .toggles = {},
      .dlc_where = "Das Spiel zeigt die Songs im Schnellspiel: in der Songliste mit der blauen Taste.",
  };
  static const GameTexts kFrench{
      .welcome_title = nullptr,
      .welcome_text = nullptr,
      .toggles = {},
      .dlc_where = "Le jeu affiche les chansons en Partie rapide : dans la liste des chansons, avec le bouton bleu.",
  };
  static const GameTexts kSpanish{
      .welcome_title = nullptr,
      .welcome_text = nullptr,
      .toggles = {},
      .dlc_where = "El juego muestra las canciones en Partida rápida: en el repertorio, el botón azul (Descarga de contenido).",
  };
  static const GameTexts kItalian{
      .welcome_title = nullptr,
      .welcome_text = nullptr,
      .toggles = {},
      .dlc_where = "Il gioco mostra i brani in Partita veloce: nella scaletta, con il tasto blu.",
  };
  switch (language) {
    case 3:
      return kGerman;
    case 4:
      return kFrench;
    case 5:
      return kSpanish;
    case 6:
      return kItalian;
    default:
      return kEnglish;
  }
}

}  // namespace launcher::game
