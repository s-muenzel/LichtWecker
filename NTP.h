#ifndef _NTP
#define _NTP

#define USE_EZ_TIME

#ifdef USE_EZ_TIME
#include <ezTime.h>
#else
#include <Time.h>
#include <TimeLib.h>
#endif // ifdef 

class NTP_Helfer {
  public:
    NTP_Helfer();
    void Beginn();
    void Tick();
    time_t now();
  protected:
#ifdef USE_EZ_TIME
    Timezone _TZ;
#endif // ifdef USE_EZ_TIME
};

#endif // _NTP
