#include <Arduino.h>
#include <ArduinoOTA.h>
#include <WiFiUdp.h>

#include "OTA.h"

#include "Sonnenaufgang.h"
#include "Speicher.h"
#include "Zugangsinfo.h"
#include "main.h"

//Sonnenaufgang *_Sa_p;
bool ota_laeuft;

OTA::OTA() {
  _ota_An = false;
  ota_laeuft = false;
}

void OTA::Beginn(Speicher *Sp, Sonnenaufgang *Sa) {
//  _Sa_p = Sa;

  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);
  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname(
      Sp->SA_hostname());  // überschreibt wohl den Aufruf von MDNS.begin()
  // No authentication by default
  //  ArduinoOTA.setPassword("...");
  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  ArduinoOTA.setPasswordHash(ota_hash);
  ArduinoOTA.setRebootOnSuccess(true);
  ArduinoOTA.onStart([]() {
    ota_laeuft = true;
    D_PRINTF(LOG_DEBUG, "Start updating ");
  });
  ArduinoOTA.onEnd([]() {
    ota_laeuft = false;
    D_PRINTF(LOG_DEBUG, "End");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
  });
  ArduinoOTA.onError([](ota_error_t error) {
    ota_laeuft = false;
    D_PRINTF(LOG_DEBUG, "Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      D_PRINTF(LOG_DEBUG, "Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      D_PRINTF(LOG_DEBUG, "Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      D_PRINTF(LOG_DEBUG, "Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      D_PRINTF(LOG_DEBUG, "Receive Failed");
    } else if (error == OTA_END_ERROR) {
      D_PRINTF(LOG_DEBUG, "End Failed");
    }
  });
}

void OTA::Bereit() {
  _ota_An = true;
  ArduinoOTA.begin();
};

bool OTA::Tick() {
  if (_ota_An) {
    ArduinoOTA.handle();
  }
  return ota_laeuft;
};
