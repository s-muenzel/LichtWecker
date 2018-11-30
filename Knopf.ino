#include "Knopf.h"

Knopf::Knopf() {
}

void Knopf::Beginn() {
  pinMode(KNOPF_PIN, INPUT);
}

Knopf::_Event_t Knopf::Status() {
  if (digitalRead(0) == HIGH) {
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
