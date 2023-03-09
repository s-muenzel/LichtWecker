#include <Arduino.h>
#include <EEPROM.h>
// #include <ezTime.h>

#include "Speicher.h"
#include "Zugangsinfo.h"
#include "main.h"

#define POS_WZ 0
#define POS_AN sizeof(_wz)
#define POS_AKTIV POS_AN + sizeof(_an)
#define POS_LAENGE POS_AKTIV + sizeof(_aktiv)
#define POS_V POS_LAENGE + sizeof(float)
#define POS_DAUER POS_V + sizeof(float)
#define POS_NACHLEUCHTEN POS_DAUER + sizeof(float)
#define POS_SNOOZE POS_NACHLEUCHTEN + sizeof(float)
#define POS_HOSTNAME POS_SNOOZE + sizeof(float)
#define POS_RELAIS POS_HOSTNAME + sizeof(char[64])
#define GROESSE_ALLES POS_RELAIS + sizeof(unsigned int)

Speicher::Speicher() {}

void Speicher::Beginn(NTP_Helfer* ntp) {
  _ntp_p = ntp;
  EEPROM.begin(GROESSE_ALLES);
  EEPROM.get(POS_WZ, _wz);
  EEPROM.get(POS_AN, _an);
  EEPROM.get(POS_AKTIV, _aktiv);
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
  D_PRINTF(LOG_DEBUG, "Wecker ist %s", _aktiv ? "An" : "Aus");
  for (uint8_t i = 0; i < 7; i++) {
    D_PRINTF(LOG_DEBUG, "_Weckzeit %d: %d:%02d:%02d (%s)", i, hour(_wz[i]),
             minute(_wz[i]), second(_wz[i]), _an[i] ? "An" : "Aus");
  }
  D_PRINTF(LOG_DEBUG, "L=%f, v=%f, d=%f n=%f, s=%f, r=%u", _konfig_laenge,
           _konfig_v, _konfig_dauer, _konfig_nachleuchten, _konfig_snooze,
           _konfig_relais);
  D_PRINTF(LOG_DEBUG, "Hostname: %s", _hostname);
}

time_t Speicher::Weckzeit_Heute() {
  if (Wecker_Aktiv()) {  // Wecker (generell an)
    time_t Jetzt = _ntp_p->Jetzt();
    int _Tag;  // Wochentag bestimmen
    if (_ntp_p->Ist_Feiertag(Jetzt)) {
      _Tag = 1;  // Feiertag, dann Sonntag (==1)
    } else {
      _Tag = weekday(Jetzt) % 7;  // (So == 1, Mo == 2, ..., Sa == 7%7 == 0)
    }
    if (Wecker_An(_Tag)) {  // An dem Tag Wecker an
      return _wz[_Tag];
    } else {  // An dem Tag Wecker nicht an
      return (time_t)0;
    }
  } else {  // Wecker aus --> keine Weckzeit
    return (time_t)0;
  }
}

time_t Speicher::Weckzeit_Morgen() {
  if (Wecker_Aktiv()) {  // Wecker (generell an)
    time_t Jetzt = _ntp_p->Jetzt();
    tmElements_t jetzt_elements;
    breakTime(Jetzt, jetzt_elements);
    jetzt_elements.Day++;
    Jetzt = makeTime(jetzt_elements);
    int _Tag;  // Wochentag bestimmen
    if (_ntp_p->Ist_Feiertag(Jetzt)) {
      _Tag = 1;  // Feiertag, dann Sonntag (==1)
    } else {
      _Tag = weekday(Jetzt) % 7;  // (So == 1, Mo == 2, ..., Sa == 7%7 == 0)
    }
    if (Wecker_An(_Tag)) {  // An dem Tag Wecker an
      return _wz[_Tag];
    } else {  // An dem Tag Wecker nicht an
      return (time_t)0;
    }
  } else {  // Wecker aus --> keine Weckzeit
    return (time_t)0;
  }
}

time_t Speicher::Naechste_Weckzeit() {
  if (Wecker_Aktiv()) {
    time_t Jetzt = _ntp_p->Jetzt();
    int _Tag = weekday(Jetzt) % 7;
    if (_ntp_p->Ist_Feiertag(Jetzt)) _Tag = 1;  // Feiertag, dann Sonntag (==1)
    uint32_t WZ_Zeit =
        hour(_wz[_Tag]) * 60 + minute(_wz[_Tag]);  // Weckzeit heute
    uint32_t Jetzt_NurZeit = hour(Jetzt) * 60 + minute(Jetzt) + second(Jetzt);
    if (Jetzt_NurZeit > WZ_Zeit) {  // Heute Zeit schon durch, schaue morgen
      _Tag = (weekday(Jetzt) + 1) % 7;
      tmElements_t jetzt_elements;
      breakTime(Jetzt, jetzt_elements);
      jetzt_elements.Day++;
      Jetzt = makeTime(jetzt_elements);  // Jetzt : morgen, gleiche Zeit
      if (_ntp_p->Ist_Feiertag(Jetzt))
        _Tag = 1;  // Feiertag, dann Sonntag (==1)
      if (Wecker_An(_Tag)) {
        return _wz[_Tag];
      } else {
        return (time_t)0;
      }
    } else {
      if (Wecker_An(_Tag)) {
        return _wz[_Tag];
      } else {
        return (time_t)0;
      }
    }
  } else {
    return (time_t)0;
  }
}

void Speicher::setze_Weckzeit(int Tag, time_t Zeit, bool An) {
  _wz[Tag] = Zeit;
  _an[Tag] = An;
  EEPROM.put(POS_WZ, _wz);
  EEPROM.put(sizeof(_wz), _an);
}

bool Speicher::jetztWecken(time_t Jetzt) {
  if (Wecker_Aktiv()) {
    int _Tag = weekday(Jetzt) % 7;
    if (Wecker_An(_Tag)) {
      return (hour(Jetzt) == hour(_wz[_Tag])) &&
             (minute(Jetzt) == minute(_wz[_Tag])) &&
             (second(Jetzt) == second(_wz[_Tag]));
    } else
      return false;
  } else  // Wecker ist inaktiv
    return false;
}

void Speicher::Wecker_Aktiv(bool aktiv) {
  _aktiv = aktiv;
  EEPROM.put(POS_AKTIV, _aktiv);
}

void Speicher::SA_laenge(float f) {
  _konfig_laenge = f;
  EEPROM.put(POS_LAENGE, _konfig_laenge);
}

void Speicher::SA_v(float f) {
  _konfig_v = f;
  EEPROM.put(POS_V, _konfig_v);
}

void Speicher::SA_dauer(float f) {
  _konfig_dauer = f;
  EEPROM.put(POS_DAUER, _konfig_dauer);
}

void Speicher::SA_nachleuchten(float f) {
  _konfig_nachleuchten = f;
  EEPROM.put(POS_NACHLEUCHTEN, _konfig_nachleuchten);
}

void Speicher::SA_snooze(float f) {
  _konfig_snooze = f;
  EEPROM.put(POS_SNOOZE, _konfig_snooze);
}

void Speicher::SA_relais(unsigned int n) {
  _konfig_relais = n;
  EEPROM.put(POS_RELAIS, _konfig_relais);
}

void Speicher::SA_hostname(const char* n) {
  strncpy(_hostname, n, 63);
  EEPROM.put(POS_HOSTNAME, _hostname);
}

void Speicher::speichern() { EEPROM.commit(); }
