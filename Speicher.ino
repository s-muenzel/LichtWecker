#include "Speicher.h"
#include <EEPROM.h>


Speicher::Speicher() {
}

void Speicher::Beginn() {
  EEPROM.begin(sizeof(_WZ) + sizeof(_An) + sizeof(_Aktiv));
  EEPROM.get(0, _WZ);
  EEPROM.get(sizeof(_WZ), _An);
  EEPROM.get(sizeof(_WZ) + sizeof(_An), _Aktiv);
  for (uint8_t i = 0; i < 7; i++) {
    Serial.printf(" Wecker ist %s\n", _Aktiv ? "An" : "Aus");
    Serial.printf("_Weckzeit %d: %d:%02d:%02d (%s)\n", i, hour(_WZ[i]), minute(_WZ[i]), second(_WZ[i]), _An[i] ? "An" : "Aus");
  }
}

time_t Speicher::Weckzeit(int Tag) {
  return _WZ[Tag];
}

bool Speicher::Wecker_An(int Tag) {
  return _An[Tag];
}

void Speicher::Wecker_Aktiv(bool aktiv) {
  _Aktiv = aktiv;
  EEPROM.put(sizeof(_WZ) + sizeof(_An), _Aktiv);
}

bool Speicher::Wecker_Aktiv() {
  return _Aktiv;
}

void Speicher::setze_Weckzeit(int Tag, time_t Zeit, bool An) {
  _WZ[Tag] = Zeit;
  _An[Tag] = An;
  EEPROM.put(0, _WZ);
  EEPROM.put(sizeof(_WZ), _An);
}

void Speicher::speichern() {
  EEPROM.commit();
}

bool Speicher::jetztWecken(time_t Jetzt) {
  if (Wecker_Aktiv()) {
    int _Tag = weekday(Jetzt) % 7;    
    return (hour(Jetzt) == hour(_WZ[_Tag])) && (minute(Jetzt) == minute(_WZ[_Tag])) && (second(Jetzt) == second(_WZ[_Tag]));
  } else // Wecker ist inaktiv
    return false;
}

