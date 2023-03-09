#include "main.h"

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>

#include "Knopf.h"
#include "NTP.h"
#include "OTA.h"
#include "Sonnenaufgang.h"
#include "Speicher.h"
#include "WebS.h"
#include "Zugangsinfo.h"

// Globale Variablen starten immer mit __[a-z]
// lokale Variable starten mit _[a-z]
// Argumente starten mit [a-z]

Speicher __WZ;
Sonnenaufgang __SA;
Knopf __Knopf;
NTP_Helfer __NTP;
OTA __OTA;
WebS __WebS;

void setup() {
  // Seriellen Output enablen
  D_BEGIN(115200);
  D_PRINTF(LOG_DEBUG, "Starte...");

  // die interne LED konfigurieren
#ifndef IST_ESP01
  pinMode(LED_BUILTIN, OUTPUT);        // bei Sonoff Basic Pin 13
  digitalWrite(LED_BUILTIN, LED_AUS);  // bei Sonoff Basic HIGH = OFF
  D_PRINTF(LOG_DEBUG, " interne LED");
#endif  // IST_ESP01

  // EEPROM "Speicher" auslesen
  __WZ.Beginn(&__NTP);
  D_PRINTF(LOG_DEBUG, " gespeicherte Werte ok");

  // Sonnenaufgang Objekt initialisieren
  __SA.Beginn(&__WZ);
  D_PRINTF(LOG_DEBUG, " SA_Objekt");

  // Wifi
  WiFi.hostname(__WZ.SA_hostname());
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    D_PRINTF(LOG_DEBUG, "Keine Wifi-Verbindung! Neustart in 5 Sekunden...");
    delay(5000);
    ESP.restart();
  }
  D_PRINTF(LOG_DEBUG, " Wifi %s (IP Address %s)", ssid,
           WiFi.localIP().toString().c_str());

  if (MDNS.begin(__WZ.SA_hostname())) {
    D_PRINTF(LOG_DEBUG, " MDNS responder");
  }

  // Zeitserver konfigurieren und starten
  __NTP.Beginn();
  D_PRINTF(LOG_DEBUG, " Zeitservice");

  // Webserver konfigurieren
  __WebS.Beginn(&__NTP, &__SA, &__WZ, &__OTA);
  D_PRINTF(LOG_DEBUG, " Webserver laeuft");

  // Wert vom Knopf setzen
  __Knopf.Beginn();
  D_PRINTF(LOG_DEBUG, " Taster");

  // OTA Initialisieren
  __OTA.Beginn(&__WZ, &__SA);
  D_PRINTF(LOG_DEBUG, " OTA vorbereitet");

  delay(500);
  // Beim ESP01S bleibt nach dem Reset ein Pixel an :-( ??
  __SA.Stop();

  D_PRINTF(LOG_DEBUG, "Fertig");
}

void loop() {
  __OTA.Tick();

  if (millis() > 2100000000UL) {  // alle ~24 Tage einen Neustart, damit
                                  // millis() nicht überläuft
    // ist etwas weniger als MAX_UNSIGNED_LONG (4294967295UL) / 2
    if (!__SA.Laeuft()) {  // Nur Neustart, wenn grade kein Weckvorgang läuft
      ESP.restart();
    }
  }

  time_t t = __NTP.Jetzt();  // Zeit holen

  switch (__Knopf.Status()) {
    case Knopf::nix:
      break;
    case Knopf::kurz:  // Kurz heisst entweder Snooze, oder aktuellen Status
                       // anzeigen
      if (__SA.Laeuft()) {
        D_PRINTF(LOG_DEBUG, "Snooze");
        __SA.Snooze();  // keine Nachricht, will ja erst noch schlafen
      } else {
        D_PRINTF(LOG_DEBUG, "Status");
        __SA.Nachricht(Sonnenaufgang::weckzeit);
      }
      break;
    case Knopf::lang:  // Lang: Wecker 24h abschalten (wenn aktuell weckt,
                       // sonst generell an/ausschalten.
                       // Ausnahme: OTA einschalten
      if (__SA.Laeuft()) {
        D_PRINTF(LOG_INFO, "Wecken abgebrochen");
        __SA.Nachricht(Sonnenaufgang::weck_abbruch);
      } else {
        if (__WZ.Wecker_Aktiv()) {  // Wecker aktiv --> auf inaktiv setzen und
                                    // rot blinken
          D_PRINTF(LOG_DEBUG, "Wecker AUS");
          __SA.Nachricht(Sonnenaufgang::wecker_aus);
          __WZ.Wecker_Aktiv(false);
        } else {  // Wecker inaktiv --> auf aktiv setzen und grün blinken
          D_PRINTF(LOG_DEBUG, "Wecker AN");
          __SA.Nachricht(Sonnenaufgang::wecker_an);
          __WZ.Wecker_Aktiv(true);
        }
      }
      if (__Knopf.WieLang() == 6) {  // jetzt lang gedrückt (~10s)
                                     // schalte OTA ein
        D_PRINTF(LOG_NOTICE, "OTA AN");
        __SA.Nachricht(Sonnenaufgang::ota_ein);
        __OTA.Bereit();
        __WebS.Admin_Mode();  // ab jetzt kann konfiguriert werden und
                              // Dateioperationen sind möglich
      }
      break;
  }

  if (__WZ.jetztWecken(t)) {
    if (!__SA.Laeuft()) {
      D_PRINTF(LOG_INFO, "Wecke jetzt");
      __SA.Nachricht(Sonnenaufgang::aufgang);
    }
  }

  __SA.Tick();
  __WebS.Tick();
  __NTP.Tick();

  delay(20);
}
