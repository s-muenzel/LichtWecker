#ifndef _SONNENAUFGANG
#define _SONNENAUFGANG

#include <NeoPixelBus.h>

class Speicher;

class Sonnenaufgang {
 public:
  Sonnenaufgang();

  void Beginn(Speicher *sp);

  void Setze_Laenge(float l) { _konfig_laenge = l; }
  void Setze_v(float v) { _konfig_v = v; }
  void Setze_Dauer(float d) { _konfig_dauer = d; }
  void Setze_Nachleuchten(float n) { _konfig_nachleuchten = n; }
  void Setze_Snooze(float s) { _konfig_snooze = s; }
  void Setze_Relais(unsigned int n) { _konfig_relais = n; }

  bool Snooze();
  void Stop();


  typedef enum _Modus_t { aufgang, weckzeit, weck_abbruch,  wecker_aus, wecker_an, ota_ein, nix } Modus_t;
  void Nachricht(Modus_t modus);

  bool Laeuft();

  void Tick();

 private:
  RgbColor Lichtfarbe(float t, float x);

  void Tick_Aufgang(long ms);
  void Tick_Weckzeit(long ms);

  void Tick_Farbe(long ms, RgbColor f);
  Modus_t _Modus;


  long _Startzeit;     // [ms] - läuft, bzw. seit wann läuft ein S.A.
  long _Nachlaufzeit;  // [ms] - wie lange bleibt das Licht nach der Dauer des
                       // Sonnenuntergangs an?
  long _Dauer;  // [ms] - wie lange dauert es, bis bei einem S.A. das Licht auf
                // MAX ist

  float _konfig_laenge;
  float _konfig_v;
  float _konfig_dauer;
  float _konfig_nachleuchten;
  float _konfig_snooze;
  unsigned int _konfig_relais;
  bool _status_Relais;

  Speicher *_speicher;
};

#endif  // _SONNENAUFGANG
