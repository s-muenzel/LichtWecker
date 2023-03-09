#include <Arduino.h>
// #include <HTTPClient.h>
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>
#include <WiFiClient.h>

#include "NTP.h"
#include "main.h"

NTP_Helfer::NTP_Helfer() {
  _feiertage_jahr = 0;
  for (uint8_t i = 0; i < MAX_FEIERTAGE; i++) {
    _ft_monat[i] = 0;
  }
}

void NTP_Helfer::Beginn() {
  _lokale_zeit.setLocation("Europe/Berlin");
  D_PRINTF(LOG_DEBUG, "NTP Update %s erfolgreich, Zeit: %s",
           waitForSync(10) ? "" : "nicht", _lokale_zeit.dateTime().c_str());
}

void NTP_Helfer::Tick() {
  events();
  if (year(Jetzt())  != _feiertage_jahr) Hole_Feiertage();
}

time_t NTP_Helfer::Jetzt() { return _lokale_zeit.now(); }

bool NTP_Helfer::Ist_Feiertag(time_t) {
  uint8_t monat = month(Jetzt());
  uint8_t tag = day(Jetzt());
  for (uint8_t i = 0; i < MAX_FEIERTAGE; i++) {
    if ((_ft_monat[i] == monat) && (_ft_tag[i] == tag)) {
      return true;
    }
    if (_ft_monat[i] == 0) return false;
  }
  return false;
}

void NTP_Helfer::Hole_Feiertage() {
#define FEIERTAGE_HOST "feiertage-api.de"
#define FEIERTAGE_URL "http://feiertage-api.de/api/?nur_land=BY&jahr="

  uint8_t jahr = year(Jetzt());

  if (jahr != _feiertage_jahr) {  // Habe noch nicht die Feiertage dieses Jahres
    WiFiClient wifi_c;
    HttpClient client = HttpClient(wifi_c, FEIERTAGE_HOST, 80);
    String url = FEIERTAGE_URL;
    client.get(url + jahr);
    int result = client.responseStatusCode();
    if (result == 0) {  // 0 == fail
      D_PRINTF(LOG_DEBUG, "Webcall kein Connect zu feiertage-api.de");
      return;
    }
    String HttpMsg;
    HttpMsg = client.responseBody();
    DynamicJsonDocument doc(3072);
    DeserializationError call_erg = deserializeJson(doc, HttpMsg);
    if (call_erg) {
      D_PRINTF(LOG_WARNING, "Webcall %s Deserialize Error:%s", HttpMsg.c_str(),
               call_erg.c_str());
      return;
    }
    JsonObject obj = doc.as<JsonObject>();
    uint8_t anz_ft = 0;
    for (JsonPair p : obj) {
      JsonVariant ft = p.value();
      String datum = ft["datum"];
      _ft_monat[anz_ft] = datum.substring(5).toInt();
      _ft_tag[anz_ft] = datum.substring(8).toInt();
      D_PRINTF(LOG_DEBUG, "Feiertag: %s:%s  %02d.%02d ", p.key().c_str(),
               datum.c_str(), _ft_monat[anz_ft], _ft_tag[anz_ft]);
      // Ausnahmen: 8.August: Augsburger Friedenstag
      if ((_ft_monat[anz_ft] == 8) && (_ft_tag[anz_ft] == 8)) {
        _ft_monat[anz_ft] = 0;
        _ft_monat[anz_ft] = 0;
        continue;
      }
      // Ausnahmen: Buß und Bettag: nur Schulfrei
      // Einziger Feiertag vom  16.11. bis 22.11.
      if ((_ft_monat[anz_ft] == 11) && (_ft_tag[anz_ft] > 15) &&
          (_ft_tag[anz_ft] < 23)) {
        _ft_monat[anz_ft] = 0;
        _ft_monat[anz_ft] = 0;
        continue;
      }
      anz_ft++;
    }
    for (; anz_ft < MAX_FEIERTAGE; anz_ft++) {
      _ft_monat[anz_ft] = 0;
      _ft_monat[anz_ft] = 0;
    }
    _feiertage_jahr = jahr;
  }
}