#include "Speicher.h"
#include <EEPROM.h>


Speicher::Speicher() {
}

void Speicher::Beginn() {
  EEPROM.begin(sizeof(_WZ) + sizeof(_An));
  EEPROM.get(0, _WZ);
  EEPROM.get(sizeof(_WZ), _An);
  for (uint8_t i = 0; i < 7; i++) {
    Serial.printf("_Weckzeit %d: %d:%02d:%02d (%s)\n", i, hour(_WZ[i]), minute(_WZ[i]), second(_WZ[i]), _An[i] ? "An" : "Aus");
  }
}

time_t Speicher::Weckzeit(int Tag) {
  return _WZ[Tag];
}

bool Speicher::Wecker_An(int Tag) {
  return _An[Tag];
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
  int _Tag = weekday(Jetzt) - 1;
  _Tag = 0; // aktuell nur ein Wert für alle Tage
  return (hour(Jetzt) == hour(_WZ[_Tag])) && (minute(Jetzt) == minute(_WZ[_Tag])) && (second(Jetzt) == second(_WZ[_Tag]));
}

