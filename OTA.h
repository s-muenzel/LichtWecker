#ifndef _OTA
#define _OTA

class OTA {
  public:
    OTA();

    void Beginn();
	void Bereit();
	
	void Tick();

  private:

	bool _OTA_An; // Wurde OTA schon "angeschltet"
};

#endif // _OTA
