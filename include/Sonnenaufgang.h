#ifndef _SONNENAUFGANG
#define _SONNENAUFGANG

#include <NeoPixelBus.h>

class Speicher;

class Sonnenaufgang {
 public:
  Sonnenaufgang();

  void Beginn(Speicher *sp);

  bool Snooze();
  void Stop();

  typedef enum _Modus_t {
    aufgang,
    weckzeit,
    weck_abbruch,
    wecker_aus,
    wecker_an,
    ota_ein,
    nix
  } Modus_t;
  void Nachricht(Modus_t modus);

  bool Laeuft();

  void Tick();

 private:
  RgbColor Lichtfarbe(float t, float x);

  void Tick_Aufgang(long ms);
  void Tick_Weckzeit(long ms);

  void Tick_Farbe(long ms, RgbColor f);
  Modus_t _modus;

  long _startzeit;     // [ms] - läuft, bzw. seit wann läuft ein S.A.
  long _nachlaufzeit;  // [ms] - Zeit, die es nach Max. weiterleuchtet
  long _dauer;         // [ms] - Zeit, bis bei einem S.A. das Licht auf MAX ist

  bool _status_relais;

  Speicher *_speicher;
};

#endif  // _SONNENAUFGANG
