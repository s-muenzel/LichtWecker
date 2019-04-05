#include "Sonnenaufgang.h"
#include "Knopf.h"
#include "NTP.h"
#include "Speicher.h"
#include "OTA.h"
#include "WebS.h"

//#define DEBUG_SERIAL
#ifdef DEBUG_SERIAL
#define D_BEGIN(speed)   Serial.begin(speed)
#define D_PRINT(...)     Serial.print(__VA_ARGS__)
#define D_PRINTLN(...)   Serial.println(__VA_ARGS__)
#define D_PRINTF(...)    Serial.printf(__VA_ARGS__)
#else
#define D_BEGIN(speed)
#define D_PRINT(...)
#define D_PRINTLN(...)
#define D_PRINTF(...)
#endif

#define MAX_UNSIGNED_LONG 4294967295UL

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>

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
  D_PRINT("Starte...");

  // die interne LED konfigurieren
  pinMode(LED_BUILTIN, OUTPUT);// bei Sonoff Basic Pin 13
  digitalWrite(LED_BUILTIN, HIGH); // bei Sonoff Basic HIGH = OFF
  D_PRINT(" interne LED");

  // EEPROM "Speicher" auslesen
  __WZ.Beginn();
  D_PRINT(" gespeicherte Werte ok");

  // Sonnenaufgang Objekt initialisieren
  __SA.Beginn();
  __SA.Setze_Laenge(__WZ.lese_SA_laenge());
  __SA.Setze_v(__WZ.lese_SA_v());
  __SA.Setze_Dauer(__WZ.lese_SA_dauer());
  __SA.Setze_Nachleuchten(__WZ.lese_SA_nachleuchten());
  __SA.Setze_Snooze(__WZ.lese_SA_snooze());
  __SA.Setze_Relais(__WZ.lese_SA_relais());
  D_PRINT(" SA_Objekt");

  // Wifi
  WiFi.hostname(__WZ.lese_hostname());
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    D_PRINTLN("Keine Wifi-Verbindung! Neustart in 5 Sekunden...");
    delay(5000);
    ESP.restart();
  }
  D_PRINTF(" Wifi %s (IP Address %s)", ssid, WiFi.localIP().toString().c_str());

  if (MDNS.begin(__WZ.lese_hostname())) {
    D_PRINT(" MDNS responder");
  }

  // Zeitserver konfigurieren und starten
  __NTP.Beginn();
  D_PRINT(" Zeitservice");

  // Webserver konfigurieren
  __WebS.Beginn();
  D_PRINT(" Webserver laeuft");

  // Wert vom Knopf setzen
  __Knopf.Beginn();
  D_PRINT(" Taster");

  // OTA Initialisieren
  __OTA.Beginn();
  D_PRINT(" OTA vorbereitet");

  D_PRINTLN("Fertig");
}

void loop() {

  if (millis() > MAX_UNSIGNED_LONG - 12UL * 3600UL * 1000UL) { // Zeit für einen Neustart
    if (!__SA.Laeuft()) { // Nur Neustart, wenn grade kein Weckvorgang läuft
      ESP.restart();
    }
  }

  time_t t = __NTP.now(); // Zeit holen

#ifdef DEBUG_SERIAL
  // Nur noch für Testzwecke
  if (Serial.available() > 0) {
    // read the incoming byte:
    while (Serial.available() > 0) {
      /*char _c  =*/ Serial.read();
    }
    if (__SA.Laeuft()) {
      D_PRINTF("Stoppe Sonnenaufgang um %d:%02d:02d\n", hour(t), minute(t), second(t));
      __SA.Stop();
    } else {
      D_PRINTF("Starte Sonnenaufgang um %d:%02d:02d\n", hour(t), minute(t), second(t));
      __SA.Start();
    }
  }
#endif

  switch (__Knopf.Status()) {
    case Knopf::nix:
      break;
    case Knopf::kurz: // Kurz heisst entweder Snooze, oder aktuellen Status anzeigen
      D_PRINTF("Knopf kurz, Nachricht gelb um %d:%02d:%02d\n", hour(t), minute(t), second(t));
      if (__SA.Laeuft()) {
        __SA.Snooze();
      }
      else {
        if (__WZ.Wecker_Aktiv()) {
          __SA.Nachricht(Sonnenaufgang::gruen, Sonnenaufgang::kurz); // gruen zeigt, dass jetzt Weckzeiten aktiviert sind
        } else {
          __SA.Nachricht(Sonnenaufgang::rot, Sonnenaufgang::kurz); // Rot zeigt, dass jetzt Weckzeiten de-aktiviert sind
        }
      }
      break;
    case Knopf::lang: // Lang: Wecker 24h abschalten oder generell an/aus schalten. Ausnahme: OTA einschalten
      if (__SA.Laeuft()) {
        D_PRINTF("Knopf lang - 24h Pause %d:%02d:%02d\n", hour(t), minute(t), second(t));
        __SA.Nachricht(Sonnenaufgang::gelb, Sonnenaufgang::lang); // Gelb zeigt, dass jetzt 24 h Ruhe ist
      } else {
        if (__WZ.Wecker_Aktiv()) { // Wecker aktiv --> auf inaktiv setzen und rot blinken
          D_PRINTF("Knopf lang - Wecker AUS  %d:%02d:%02d\n", hour(t), minute(t), second(t));
          __SA.Nachricht(Sonnenaufgang::rot, Sonnenaufgang::lang); // Rot zeigt, dass jetzt Weckzeiten de-aktiviert sind
          __WZ.Wecker_Aktiv(false);
        } else { // Wecker inaktiv --> auf aktiv setzen und grün blinken
          D_PRINTF("Knopf lang - Wecker AN  %d:%02d:02d\n", hour(t), minute(t), second(t));
          __SA.Nachricht(Sonnenaufgang::gruen, Sonnenaufgang::lang); // Rot zeigt, dass jetzt Weckzeiten de-aktiviert sind
          __WZ.Wecker_Aktiv(true);
        }
      }
      if  (__Knopf.WieLang() == 6) { // jetzt wirklich lang gedrückt (~10s) --> schalte OTA ein
        D_PRINTF("Knopf GAAANZ lang - OTA AN  %d:%02d:%02d\n", hour(t), minute(t), second(t));
        __SA.Nachricht(Sonnenaufgang::lila, Sonnenaufgang::lang); // lila zeigt, dass jetzt OTA aktiviert ist
        __OTA.Bereit();
        __WebS.Admin_Mode(); // ab jetzt kann konfiguriert werden und Dateioperationen sind möglich
      }
      break;
  }

  if (__WZ.jetztWecken(t)) {
    if (!__SA.Laeuft()) {
      D_PRINTF("Weckzeit erreicht, starte Sonnenaufgang um %d:%02d:%02d\n", hour(t), minute(t), second(t));
      __SA.Start();
    }
  }

  __SA.Tick();
  __OTA.Tick();
  __WebS.Tick();
  __NTP.Tick();

  delay(20);
}
