#ifndef _NTP
#define _NTP

#include <ezTime.h>

class NTP_Helfer {
 public:
  NTP_Helfer();
  void Beginn();
  void Tick();
  time_t now();

 protected:
  Timezone lokale_zeit_;
};

#endif  // _NTP
