// This game's own texts (game.h): the first run's screens and the Instruments and gamepad page in the game's spirit
// (GH Metallica's mockup, local_docs/launcher_design), the Game page's switches, where the game shows its DLC, and the
// guitar's functions on the Keyboard and Gamepad pages, in the launcher's languages (English, German, French, Spanish,
// Italian).

#include "game.h"

#include "settings.h"

namespace launcher::game {

const GameTexts& GameTextsFor(uint32_t language) {
  static const GameTexts kEnglish{
      .welcome_title = "Welcome backstage",
      .welcome_text = "Before you hit the stage, the game has to be installed. The launcher extracts its files from "
                      "your Xbox 360 ISO; you only have to do this once.",
      .toggles = {
          "Bloom", "The glow around lights and bright surfaces.",
          "Depth of field", "The background blurred behind what is in focus.",
          "Color correction", "Each venue's look; off turns the grain off too.",
          "Film grain", "The grain over the image. Needs the color correction.",
          "Vignette", "Darker corners, where a venue turns it on.",
          "Ink blotches", "The ink effect a song adds over the image.",
      },
      .toggle_help = {
          "The halo of light around the stage's lights, the pyrotechnics and the brightest surfaces. Off, the image is sharper and flatter.",
          "Blurs the background in some camera shots and behind the menus and the pause menu, as a camera's lens does. Off, everything stays sharp.",
          "The brightness, contrast and saturation each venue sets, the fades to black and the grey image while the rock meter is red. Off, the colors are the scene's own and the film grain goes off with it: without this pass the grain is far stronger.",
          "A fine moving grain over the image, as on film. It only shows with the color correction on.",
          "Darkens the corners of the image in the venues and moments that use it.",
          "The black ink blotches the game paints over the image during a song.",
      },
      .done_title = "The stage is yours!",
      .done_text = "The game was installed. Go play now, or first review the video, graphics and instrument settings; "
                   "you can always change them in the launcher.",
      .dlc_where = "The game lists the songs in Quickplay: in the song list, with the blue button.",
      .extracting_title = "Tuning up the stage",
      .extracting_text = "The game is being extracted. It can take a few minutes; do not close the launcher.",
      .keyboard_tap_title = "Strum when a fret is pressed",
      .keyboard_tap_hint = "Each fret key strums as you press it; frets pressed within 20 ms of each other are one chord, one strum.",
      .part_instrument = "Guitar",
      .part_vocals = "Vocals",
  };
  static const GameTexts kGerman{
      .welcome_title = "Willkommen im Backstage",
      .welcome_text = "Bevor es auf die Bühne geht, muss das Spiel installiert werden. Der Launcher entpackt die Dateien "
                      "aus deinem Xbox-360-ISO; das musst du nur einmal tun.",
      .toggles = {
          "Bloom", "Das Leuchten um Lichter und helle Flächen.",
          "Tiefenschärfe", "Der unscharfe Hintergrund hinter dem Fokus.",
          "Farbkorrektur", "Der Look jedes Ortes; aus schaltet auch die Körnung aus.",
          "Filmkörnung", "Die Körnung über dem Bild. Braucht die Farbkorrektur.",
          "Vignette", "Dunklere Ecken, wo ein Ort sie einschaltet.",
          "Tintenflecken", "Der Tinteneffekt, den ein Song über das Bild legt.",
      },
      .toggle_help = {
          "Der Lichtschein um die Bühnenlichter, die Pyrotechnik und die hellsten Flächen. Aus wirkt das Bild schärfer und flacher.",
          "Macht den Hintergrund in manchen Kameraeinstellungen sowie hinter den Menüs und dem Pausenmenü unscharf, wie ein Kameraobjektiv. Aus bleibt alles scharf.",
          "Helligkeit, Kontrast und Sättigung jedes Auftrittsortes, die Abblenden ins Schwarze und das graue Bild, solange die Rock-Anzeige rot ist. Aus zeigt die eigenen Farben der Szene, und die Filmkörnung geht mit aus: ohne diesen Durchgang ist sie viel stärker.",
          "Eine feine, bewegte Körnung über dem Bild, wie bei Film. Nur mit eingeschalteter Farbkorrektur sichtbar.",
          "Dunkelt die Bildecken an den Orten und in den Momenten ab, die sie verwenden.",
          "Die schwarzen Tintenflecken, die das Spiel während eines Songs über das Bild malt.",
      },
      .done_title = "Die Bühne gehört dir!",
      .done_text = "Das Spiel wurde installiert. Leg gleich los oder sieh dir zuerst die Video-, Grafik- und "
                   "Instrumenteinstellungen an; du kannst sie jederzeit im Launcher ändern.",
      .dlc_where = "Das Spiel zeigt die Songs im Schnellspiel: in der Songliste mit der blauen Taste.",
      .extracting_title = "Die Bühne wird gestimmt",
      .extracting_text = "Das Spiel wird entpackt. Das kann ein paar Minuten dauern; schließe den Launcher nicht.",
      .keyboard_tap_title = "Anschlag beim Drücken eines Bunds",
      .keyboard_tap_hint = "Jede Bundtaste schlägt an, wenn du sie drückst; Bünde, die du innerhalb von 20 ms drückst, sind ein Akkord, ein Anschlag.",
      .part_instrument = "Gitarre",
      .part_vocals = "Gesang",
  };
  static const GameTexts kFrench{
      .welcome_title = "Bienvenue en coulisses",
      .welcome_text = "Avant de monter sur scène, il faut installer le jeu. Le launcher extrait ses fichiers de votre ISO "
                      "Xbox 360 ; vous n'aurez à le faire qu'une fois.",
      .toggles = {
          "Bloom", "La lueur autour des lumières et des surfaces claires.",
          "Profondeur de champ", "L'arrière-plan flou derrière le sujet net.",
          "Correction des couleurs", "L'ambiance de chaque salle ; désactivée, le grain l'est aussi.",
          "Grain", "Le grain sur l'image. Nécessite la correction des couleurs.",
          "Vignettage", "Des coins plus sombres, là où une salle l'active.",
          "Taches d'encre", "L'effet d'encre qu'un morceau ajoute sur l'image.",
      },
      .toggle_help = {
          "Le halo autour des lumières de la scène, de la pyrotechnie et des surfaces les plus claires. Désactivé, l'image est plus nette et plus plate.",
          "Floute l'arrière-plan dans certains plans de caméra et derrière les menus et le menu pause, comme un objectif. Désactivée, tout reste net.",
          "La luminosité, le contraste et la saturation de chaque salle, les fondus au noir et l'image grise tant que la jauge rock est dans le rouge. Désactivée, les couleurs sont celles de la scène et le grain s'éteint avec elle : sans cette passe, il est bien plus fort.",
          "Un grain fin et animé sur l'image, comme sur pellicule. Il n'apparaît qu'avec la correction des couleurs activée.",
          "Assombrit les coins de l'image dans les salles et les moments qui l'utilisent.",
          "Les taches d'encre noire que le jeu peint sur l'image pendant un morceau.",
      },
      .done_title = "La scène est à vous !",
      .done_text = "Le jeu a été installé. Allez jouer tout de suite, ou vérifiez d'abord les réglages vidéo, "
                   "graphiques et de vos instruments ; vous pourrez toujours les changer dans le launcher.",
      .dlc_where = "Le jeu affiche les chansons en Partie rapide : dans la liste des chansons, avec le bouton bleu.",
      .extracting_title = "On accorde la scène",
      .extracting_text = "Le jeu est en cours d'extraction. Cela peut prendre quelques minutes ; ne fermez pas le "
                         "launcher.",
      .keyboard_tap_title = "Gratter en appuyant sur une frette",
      .keyboard_tap_hint = "Chaque touche de frette gratte quand vous l'appuyez ; les frettes appuyées à moins de 20 ms d'intervalle forment un accord, un seul grattage.",
      .part_instrument = "Guitare",
      .part_vocals = "Chant",
  };
  static const GameTexts kSpanish{
      .welcome_title = "Bienvenido al backstage",
      .welcome_text = "Antes de subir al escenario hay que instalar el juego. El launcher extrae los archivos desde tu "
                      "ISO de Xbox 360; solo tendrás que hacerlo una vez.",
      .toggles = {
          "Bloom", "El resplandor alrededor de las luces y superficies claras.",
          "Profundidad de campo", "El fondo desenfocado detrás de lo enfocado.",
          "Corrección de color", "El aspecto de cada escenario; al desactivarla se apaga el grano.",
          "Grano de película", "El grano sobre la imagen. Necesita la corrección de color.",
          "Viñeta", "Esquinas más oscuras, donde un escenario la activa.",
          "Manchas de tinta", "El efecto de tinta que una canción añade sobre la imagen.",
      },
      .toggle_help = {
          "El halo de luz alrededor de las luces del escenario, la pirotecnia y las superficies más brillantes. Desactivado, la imagen se ve más nítida y plana.",
          "Desenfoca el fondo en algunas tomas de cámara y detrás de los menús y la pausa, como el lente de una cámara. Desactivada, todo se ve nítido.",
          "El brillo, el contraste y la saturación de cada escenario, los fundidos a negro y la imagen gris mientras el medidor de rock está en rojo. Desactivada, se ven los colores propios de la escena y el grano de película se apaga con ella: sin esta pasada el grano es mucho más fuerte.",
          "Un grano fino y en movimiento sobre la imagen, como en una película. Solo se ve con la corrección de color activada.",
          "Oscurece las esquinas de la imagen en los escenarios y momentos que la usan.",
          "Las manchas de tinta negra que el juego pinta sobre la imagen durante una canción.",
      },
      .done_title = "¡El escenario es tuyo!",
      .done_text = "El juego se instaló correctamente. Puedes salir a tocar ya o revisar primero los ajustes de video, "
                   "gráficos y tus instrumentos; siempre podrás cambiarlos desde el launcher.",
      .dlc_where = "El juego muestra las canciones en Partida rápida: en el repertorio, el botón azul (Descarga de "
                   "contenido).",
      .extracting_title = "Afinando el escenario",
      .extracting_text = "Estamos extrayendo el juego. Puede tardar unos minutos; no cierres el launcher.",
      .keyboard_tap_title = "Activar rasgueo al presionar botón",
      .keyboard_tap_hint = "Cada tecla de traste rasguea al presionarla; los trastes que presiones con menos de 20 ms de diferencia cuentan como un acorde: un solo rasgueo.",
      .part_instrument = "Guitarra",
      .part_vocals = "Voz",
  };
  static const GameTexts kItalian{
      .welcome_title = "Benvenuto nel backstage",
      .welcome_text = "Prima di salire sul palco bisogna installare il gioco. Il launcher estrae i file dalla tua ISO "
                      "Xbox 360; dovrai farlo una sola volta.",
      .toggles = {
          "Bloom", "Il bagliore attorno alle luci e alle superfici chiare.",
          "Profondità di campo", "Lo sfondo sfocato dietro ciò che è a fuoco.",
          "Correzione del colore", "L'aspetto di ogni locale; disattivata spegne anche la grana.",
          "Grana", "La grana sull'immagine. Richiede la correzione del colore.",
          "Vignettatura", "Angoli più scuri, dove un locale la attiva.",
          "Macchie d'inchiostro", "L'effetto d'inchiostro che una canzone aggiunge all'immagine.",
      },
      .toggle_help = {
          "L'alone di luce attorno alle luci del palco, alla pirotecnica e alle superfici più luminose. Disattivato, l'immagine è più nitida e piatta.",
          "Sfoca lo sfondo in alcune inquadrature e dietro i menu e il menu di pausa, come l'obiettivo di una telecamera. Disattivata, tutto resta nitido.",
          "La luminosità, il contrasto e la saturazione di ogni locale, le dissolvenze al nero e l'immagine grigia mentre l'indicatore rock è nel rosso. Disattivata, si vedono i colori propri della scena e la grana si spegne con essa: senza questo passaggio è molto più forte.",
          "Una grana fine e in movimento sull'immagine, come su pellicola. Si vede solo con la correzione del colore attiva.",
          "Scurisce gli angoli dell'immagine nei locali e nei momenti che la usano.",
          "Le macchie d'inchiostro nero che il gioco dipinge sull'immagine durante una canzone.",
      },
      .done_title = "Il palco è tuo!",
      .done_text = "Il gioco è stato installato. Vai a suonare subito o controlla prima le impostazioni video, grafiche e "
                   "dei tuoi strumenti; potrai sempre cambiarle nel launcher.",
      .dlc_where = "Il gioco mostra i brani in Partita veloce: nella scaletta, con il tasto blu.",
      .extracting_title = "Accordiamo il palco",
      .extracting_text = "Stiamo estraendo il gioco. Può richiedere qualche minuto; non chiudere il launcher.",
      .keyboard_tap_title = "Plettrata alla pressione di un tasto",
      .keyboard_tap_hint = "Ogni tasto della chitarra plettra quando lo premi; i tasti premuti a meno di 20 ms l'uno dall'altro sono un accordo, una sola plettrata.",
      .part_instrument = "Chitarra",
      .part_vocals = "Voce",
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


namespace {

// The frets' colors (the mockup's) and the chips' text colors.
constexpr uint32_t kGreen = 0x3FAE5F;
constexpr uint32_t kRed = 0xD9443A;
constexpr uint32_t kYellow = 0xE8C547;
constexpr uint32_t kBlue = 0x3E82D6;
constexpr uint32_t kOrange = 0xEB8A35;
constexpr uint32_t kGrey = 0x2A2A30;
constexpr uint32_t kOnFret = 0x0E0E10;
constexpr uint32_t kOnGrey = 0xC9C9D0;
constexpr uint32_t kOnStarPower = 0xC5DC3A;

// One language's words for the guitar's functions.
struct ControlWords {
  const char* frets;
  const char* strum_group;
  const char* menus;
  const char* green;
  const char* red;
  const char* yellow;
  const char* blue;
  const char* orange;
  const char* strum_up;
  const char* strum_down;
  const char* whammy;
  const char* whammy_chip;
  const char* dpad_up;
  const char* dpad_down;
  const char* right_stick;
};

// The game's guitar on the Xbox 360 controller (what the game reads from a guitar, and from a controller or the
// keyboard reported as one, src/startup.cpp's gamepad_subtype): the frets on A, B, Y, X and LB, strumming on the
// D-pad, the whammy bar on the right stick, Star Power on Back. Back is Star Power in the menus too, so it is listed
// once.
std::vector<GameControlGroup> Guitar(const ControlWords& w) {
  return {
      {w.frets,
       {
           {int(Bind::kA), "", kGreen, kOnFret, w.green},
           {int(Bind::kB), "", kRed, kOnFret, w.red},
           {int(Bind::kY), "", kYellow, kOnFret, w.yellow},
           {int(Bind::kX), "", kBlue, kOnFret, w.blue},
           {int(Bind::kLeftShoulder), "", kOrange, kOnFret, w.orange},
       }},
      {w.strum_group,
       {
           {int(Bind::kDpadUp), "\xE2\x86\x91", kGrey, kOnGrey, w.strum_up, w.dpad_up},
           {int(Bind::kDpadDown), "\xE2\x86\x93", kGrey, kOnGrey, w.strum_down, w.dpad_down},
           {int(Bind::kRStickRight), w.whammy_chip, kGrey, kOnGrey, w.whammy, w.right_stick},
           {int(Bind::kBack), "SP", kGrey, kOnStarPower, "Star Power"},
       }},
      {w.menus,
       {
           {int(Bind::kStart), "START", kGrey, kOnGrey, "Start"},
       }},
  };
}

}  // namespace

const std::vector<GameControlGroup>& GameControlsFor(uint32_t language) {
  static const std::vector<GameControlGroup> kEnglish = Guitar({
      "Guitar \xC2\xB7 Frets", "Guitar \xC2\xB7 Strum and effects", "Menus", "Green fret", "Red fret", "Yellow fret",
      "Blue fret", "Orange fret", "Strum up", "Strum down", "Whammy bar", "WHAM", "D-pad up", "D-pad down",
      "Right stick"});
  static const std::vector<GameControlGroup> kGerman = Guitar({
      "Gitarre \xC2\xB7 Bünde", "Gitarre \xC2\xB7 Anschlag und Effekte", "Menüs", "Grüner Bund", "Roter Bund",
      "Gelber Bund", "Blauer Bund", "Oranger Bund", "Anschlag hoch", "Anschlag runter", "Whammy-Hebel", "WHAM",
      "Steuerkreuz hoch", "Steuerkreuz runter", "Rechter Stick"});
  static const std::vector<GameControlGroup> kFrench = Guitar({
      "Guitare \xC2\xB7 Frettes", "Guitare \xC2\xB7 Grattage et effets", "Menus", "Frette verte", "Frette rouge",
      "Frette jaune", "Frette bleue", "Frette orange", "Grattage haut", "Grattage bas", "Levier de vibrato", "VIBR",
      "Croix haut", "Croix bas", "Stick droit"});
  static const std::vector<GameControlGroup> kSpanish = Guitar({
      "Guitarra \xC2\xB7 Trastes", "Guitarra \xC2\xB7 Rasgueo y efectos", "Menús", "Traste verde", "Traste rojo",
      "Traste amarillo", "Traste azul", "Traste naranja", "Rasgueo arriba", "Rasgueo abajo", "Palanca de trémolo",
      "TRÉM", "Cruceta arriba", "Cruceta abajo", "Stick derecho"});
  static const std::vector<GameControlGroup> kItalian = Guitar({
      "Chitarra \xC2\xB7 Tasti", "Chitarra \xC2\xB7 Plettro ed effetti", "Menu", "Tasto verde", "Tasto rosso",
      "Tasto giallo", "Tasto blu", "Tasto arancione", "Plettrata su", "Plettrata giù", "Leva del vibrato", "VIBR",
      "Croce su", "Croce giù", "Levetta destra"});
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
