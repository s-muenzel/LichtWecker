#ifndef _WEBS
#define _WEBS

class NTP_Helfer;
class Sonnenaufgang;
class Speicher;
class OTA;

class WebS {
 public:
  WebS();
  void Beginn(NTP_Helfer *ntp, Sonnenaufgang *sa, Speicher *sp, OTA *ota);
  void Admin_Mode();
  void Tick();

 private:
};

#endif  // _WEBS
