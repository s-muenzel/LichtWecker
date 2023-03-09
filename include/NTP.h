#ifndef _NTP
#define _NTP

#include <ezTime.h>

#define MAX_FEIERTAGE 18
class NTP_Helfer {
 public:
  NTP_Helfer();
  void Beginn();
  void Tick();
  time_t Jetzt();

  bool Ist_Feiertag(time_t);

 protected:
  void Hole_Feiertage();

  Timezone _lokale_zeit;
  uint64_t _feiertage_jahr; // Jahr, füer das die Feiertage gelten
  uint8_t _ft_monat[MAX_FEIERTAGE];
  uint8_t _ft_tag[MAX_FEIERTAGE];
};

#endif  // _NTP
