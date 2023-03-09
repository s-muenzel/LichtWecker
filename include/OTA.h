#ifndef _OTA
#define _OTA

class Speicher;
class Sonnenaufgang;

class OTA {
 public:
  OTA();

  void Beginn(Speicher *Sp, Sonnenaufgang *Sa);
  void Bereit();

  bool Tick();

 private:
  bool _ota_An;  // Wurde OTA schon "angeschltet"
};

#endif  // _OTA
