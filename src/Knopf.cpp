#include <Arduino.h>

#include "main.h"
#include "Knopf.h"

#ifdef IST_SONOFF
#define KNOPFSTATUS digitalRead(KNOPF_PIN)
#else  // IST_SONOFF
#define KNOPFSTATUS HIGH
#endif  // IST_SONOFF

#define KNOPF_PIN 0  // Sonoff Taster
#define LANG 1500    // [ms] ab welcher Dauer zählt der Tastendruck als "lang"

Knopf::Knopf() {}

void Knopf::Beginn() {
#ifdef IST_SONOFF
  pinMode(KNOPF_PIN, INPUT);
#endif  // IST_SONOFF
}

Knopf::_Event_t Knopf::Status() {
  if (KNOPFSTATUS == HIGH) {
    // Knopf nicht gedrückt
    _start = millis();
    _kurz = false;
    _lang = 0;
  } else {
    // Knopf gedrückt
    if (!_kurz) {
      _kurz = true;
      return kurz;
    }
    int anzahl_langs = (millis() - _start) / LANG;
    if (anzahl_langs > _lang) {
      _lang = anzahl_langs;
      return lang;
    }
    return nix;
  }
  return nix;
}

int Knopf::WieLang() { return _lang; }
