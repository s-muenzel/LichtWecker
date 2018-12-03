#include "OTA.h"
#include <ArduinoOTA.h>
#include <WiFiUdp.h>

OTA::OTA() {
	_OTA_An = false;
}

void OTA::Beginn() {


  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);
  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname("Lichtwecker"); // überschreibt wohl den Aufruf von MDNS.begin()
  // No authentication by default
  //  ArduinoOTA.setPassword("...");
  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  ArduinoOTA.setPasswordHash("30eb558f1c129d200efbfed6eb6d8466");
  ArduinoOTA.setRebootOnSuccess(true);
  ArduinoOTA.onStart([]() {
    __SA.Stop();
    Serial.println("Start updating ");
  });
  ArduinoOTA.onEnd([]() {
    __SA.Stop();
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    static Sonnenaufgang::Farb_t _farbe = Sonnenaufgang::gruen;
    switch (_farbe) {
      case Sonnenaufgang::gruen:
        __SA.Nachricht(Sonnenaufgang::gruen, Sonnenaufgang::lang, (progress * 100) / total);
        _farbe = Sonnenaufgang::gelb;
        break;
      case Sonnenaufgang::gelb:
        __SA.Nachricht(Sonnenaufgang::gelb, Sonnenaufgang::lang, (progress * 100) / total);
        _farbe = Sonnenaufgang::rot;
        break;
      case Sonnenaufgang::rot:
        __SA.Nachricht(Sonnenaufgang::rot, Sonnenaufgang::lang, (progress * 100) / total);
        _farbe = Sonnenaufgang::gruen;
        break;
    }
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    __SA.Nachricht(Sonnenaufgang::rot, Sonnenaufgang::lang);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
	
}

void OTA::Bereit() {
	_OTA_An = true;
    ArduinoOTA.begin();
};

void OTA::Tick() {
  if (_OTA_An) {
    ArduinoOTA.handle();
  }
};