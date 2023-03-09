#ifndef _SPEICHER
#define _SPEICHER

#include "NTP.h"

class Speicher {
 public:
  Speicher();

  void Beginn(NTP_Helfer* ntp);

  time_t Weckzeit(int Tag) { return _wz[Tag]; };
  time_t Weckzeit_Heute();
  time_t Weckzeit_Morgen();
  time_t Naechste_Weckzeit();

  bool Wecker_An(int Tag){ return _an[Tag]; };
  void setze_Weckzeit(int Tag, time_t Zeit, bool An);

  bool jetztWecken(time_t Jetzt);

  void Wecker_Aktiv(bool aktiv);
  bool Wecker_Aktiv(){ return _aktiv; };
  
  float SA_laenge() { return _konfig_laenge; };
  void SA_laenge(float f);
  float SA_v() { return _konfig_v; };
  void SA_v(float f);
  float SA_dauer() { return _konfig_dauer; };
  void SA_dauer(float f);
  float SA_nachleuchten() { return _konfig_nachleuchten; };
  void SA_nachleuchten(float f);
  float SA_snooze() { return _konfig_snooze; };
  void SA_snooze(float f);
  unsigned int SA_relais() { return _konfig_relais; };
  void SA_relais(unsigned int n);
  const char* SA_hostname() { return _hostname; };
  void SA_hostname(const char* n);

  void speichern();

 private:
  time_t _wz[7];  // für jeden Wochentag (erstmal egal)
  bool _an[7];    // Wecker ebenfalls pro Tag ein/ausschaltbar (erstmal egal)
  bool _aktiv;
  float _konfig_laenge;
  float _konfig_v;
  float _konfig_dauer;
  float _konfig_nachleuchten;
  float _konfig_snooze;
  unsigned int _konfig_relais;
  char _hostname[64];

  NTP_Helfer* _ntp_p;
};

#endif  // _SPEICHER
