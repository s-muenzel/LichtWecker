#include "Speicher.h"
#include <EEPROM.h>

#define POS_WZ            0
#define POS_AN            sizeof(_WZ)
#define POS_AKTIV         POS_AN + sizeof(_An)
#define POS_LAENGE        POS_AKTIV + sizeof(_Aktiv)
#define POS_V             POS_LAENGE + sizeof(float)
#define POS_DAUER         POS_V + sizeof(float)
#define POS_NACHLEUCHTEN  POS_DAUER + sizeof(float)
#define POS_SNOOZE        POS_NACHLEUCHTEN + sizeof(float)
#define POS_HOSTNAME      POS_SNOOZE + sizeof(float)
#define POS_RELAIS        POS_HOSTNAME + sizeof(char[64])
#define GROESSE_ALLES     POS_RELAIS + sizeof(unsigned int)

Speicher::Speicher() {
}

void Speicher::Beginn() {
  EEPROM.begin(GROESSE_ALLES);
  EEPROM.get(POS_WZ, _WZ);
  EEPROM.get(POS_AN, _An);
  EEPROM.get(POS_AKTIV, _Aktiv);
  EEPROM.get(POS_LAENGE, _konfig_laenge);
  EEPROM.get(POS_V, _konfig_v);
  EEPROM.get(POS_DAUER, _konfig_dauer);
  EEPROM.get(POS_NACHLEUCHTEN, _konfig_nachleuchten);
  EEPROM.get(POS_SNOOZE, _konfig_snooze);
  EEPROM.get(POS_HOSTNAME, _hostname);
  if ((strnlen(_hostname, 63) == 0) || (strnlen(_hostname, 63) >= 63)) {
    strcpy(_hostname, host_name);
    EEPROM.put(POS_HOSTNAME, _hostname);
  }
  EEPROM.get(POS_RELAIS, _konfig_relais);
  if (_konfig_relais > 5000) {
    _konfig_relais = 100;
    EEPROM.put(POS_RELAIS, _konfig_relais);
  }
  D_PRINTF("Wecker ist %s\n", _Aktiv ? "An" : "Aus");
  for (uint8_t i = 0; i < 7; i++) {
    D_PRINTF("_Weckzeit %d: %d:%02d:%02d (%s)\n", i, hour(_WZ[i]), minute(_WZ[i]), second(_WZ[i]), _An[i] ? "An" : "Aus");
  }
  D_PRINTF("L=%f, v=%f, d=%f n=%f, s=%f, r=%u\n", _konfig_laenge, _konfig_v, _konfig_dauer, _konfig_nachleuchten, _konfig_snooze, _konfig_relais);
  D_PRINTF("Hostname: %s\n", _hostname);
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

float Speicher::lese_SA_laenge() {
  EEPROM.get(POS_LAENGE, _konfig_laenge);
  return _konfig_laenge;
}

float Speicher::lese_SA_v() {
  EEPROM.get(POS_V, _konfig_v);
  return _konfig_v;
}

float Speicher::lese_SA_dauer() {
  EEPROM.get(POS_DAUER, _konfig_dauer);
  return _konfig_dauer;
}

float Speicher::lese_SA_nachleuchten() {
  EEPROM.get(POS_NACHLEUCHTEN, _konfig_nachleuchten);
  return _konfig_nachleuchten;
}

float Speicher::lese_SA_snooze() {
  EEPROM.get(POS_SNOOZE, _konfig_snooze);
  return _konfig_snooze;
}

unsigned int Speicher::lese_SA_relais() {
  EEPROM.get(POS_RELAIS, _konfig_relais);
  return _konfig_relais;
}

void Speicher::setze_SA_laenge(float f) {
  _konfig_laenge = f;
  EEPROM.put(POS_LAENGE, _konfig_laenge);
}

void Speicher::setze_SA_v(float f) {
  _konfig_v = f;
  EEPROM.put(POS_V, _konfig_v);
}
void Speicher::setze_SA_dauer(float f) {
  _konfig_dauer = f;
  EEPROM.put(POS_DAUER, _konfig_dauer);
}

void Speicher::setze_SA_nachleuchten(float f) {
  _konfig_nachleuchten = f;
  EEPROM.put(POS_NACHLEUCHTEN, _konfig_nachleuchten);
}

void Speicher::setze_SA_snooze(float f) {
  _konfig_snooze = f;
  EEPROM.put(POS_SNOOZE, _konfig_snooze);
}

void Speicher::setze_SA_relais(unsigned int n) {
  _konfig_relais = n;
  EEPROM.put(POS_RELAIS, _konfig_relais);
}

const char *Speicher::lese_hostname() {
  return _hostname;
}

void Speicher::setze_hostname(const char* n) {
  strncpy(_hostname, n, 63);
  D_PRINTF("Hostname: %s (size: %d)\n", _hostname, sizeof(_hostname));
  EEPROM.put(POS_HOSTNAME, _hostname);
}

