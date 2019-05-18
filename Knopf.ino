#include "LichtWecker.h"

#include "Knopf.h"

#ifdef IST_SONOFF
#define KNOPFSTATUS digitalRead(KNOPF_PIN)
#else // IST_SONOFF
#define KNOPFSTATUS HIGH
#endif // IST_SONOFF

Knopf::Knopf() {
}

void Knopf::Beginn() {
#ifdef IST_SONOFF
  pinMode(KNOPF_PIN, INPUT);
#endif // IST_SONOFF
}

Knopf::_Event_t Knopf::Status() {
  if (KNOPFSTATUS == HIGH) {
    // Knopf nicht gedrückt
    _Start = millis();
    _Kurz = false;
    _Lang = 0;
  } else {
    // Knopf gedrückt
    if (!_Kurz) {
      _Kurz = true;
      return kurz;
    }
    int anzahl_Langs = (millis() - _Start) / LANG;
    if (anzahl_Langs > _Lang) {
      _Lang = anzahl_Langs;
      return lang;
    }
    return nix;
  }
  return nix;
}

int Knopf::WieLang() {
  return _Lang;
}
