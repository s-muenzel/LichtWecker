#include "Sonnenaufgang.h"
#include "Knopf.h"
#include "NTP.h"
#include "Speicher.h"
#include "OTA.h"
#include "WebS.h"

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>


#include <Time.h>
#include <TimeLib.h>

// Globale Variablen starten immer mit __[a-z]
// lokale Variable starten mit _[a-z]
// Argumente starten mit [a-z]


const char* ssid = "0024A5C6D897";
const char* password = "u5rr1xembpu1c";


Speicher __WZ;
Sonnenaufgang __SA;
Knopf __Knopf;
NTP_Helfer __NTP;
OTA __OTA;
WebS __WebS;

void setup() {
  // Seriellen Output enablen
  Serial.begin(115200);
  Serial.print("Starte...");

  // die interne LED konfigurieren
  pinMode(LED_BUILTIN, OUTPUT);// bei Sonoff Basic Pin 13
  digitalWrite(LED_BUILTIN, HIGH); // bei Sonoff Basic HIGH = OFF
  Serial.print(" interneLED");

  // EEPROM "Speicher" auslesen
  __WZ.Beginn();
  Serial.print(" gespeicherte Werte ok");

  // Sonnenaufgang Objekt initialisieren
  __SA.Beginn();
  __SA.Setze_Laenge(__WZ.lese_SA_laenge());
  __SA.Setze_v(__WZ.lese_SA_v());
  __SA.Setze_Dauer(__WZ.lese_SA_dauer());
  __SA.Setze_Nachleuchten(__WZ.lese_SA_nachleuchten());
  __SA.Setze_Snooze(__WZ.lese_SA_snooze());
  Serial.println(" SA_Objekt");

  // Wifi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Keine Wifi-Verbindung! Neustart in 5 Sekunden...");
    delay(5000);
    ESP.restart();
  }
  Serial.printf(" Wifi %s (IP Address %s)", ssid, WiFi.localIP().toString().c_str());

  if (MDNS.begin("Lichtwecker")) {
    Serial.print(" MDNS responder");
  }

  // Zeitserver konfigurieren und starten
  __NTP.Beginn();
  Serial.print(" Zeitservice");

  // Webserver konfigurieren
  __WebS.Beginn();
  Serial.println(" Webserver laeuft");

  // Wert vom Knopf setzen
  __Knopf.Beginn();
  Serial.print(" Taster");

  // OTA Initialisieren
  __OTA.Beginn();
  Serial.println(" OTA vorbereitet");

  Serial.println("Fertig");
}

void loop() {
  time_t t = now(); // Zeit holen

  // Nur noch für Testzwecke
  if (Serial.available() > 0) {
    // read the incoming byte:
    while (Serial.available() > 0) {
      /*char _c  =*/ Serial.read();
    }
    if (__SA.Laeuft()) {
      Serial.printf("Stoppe Sonnenaufgang um %d:%02d:02d\n", hour(t), minute(t), second(t));
      __SA.Stop();
    } else {
      Serial.printf("Starte Sonnenaufgang um %d:%02d:02d\n", hour(t), minute(t), second(t));
      __SA.Start();
    }
  }

  switch (__Knopf.Status()) {
    case Knopf::nix:
      break;
    case Knopf::kurz: // Kurz heisst entweder Snooze, oder aktuellen Status anzeigen
      Serial.printf("Knopf kurz, Nachricht gelb um %d:%02d:02d\n", hour(t), minute(t), second(t));
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
		Serial.printf("Knopf lang - 24h Pause %d:%02d:02d\n", hour(t), minute(t), second(t));
        __SA.Nachricht(Sonnenaufgang::gelb, Sonnenaufgang::lang); // Gelb zeigt, dass jetzt 24 h Ruhe ist
      } else {
        if (__WZ.Wecker_Aktiv()) { // Wecker aktiv --> auf inaktiv setzen und rot blinken
			Serial.printf("Knopf lang - Wecker AUS  %d:%02d:02d\n", hour(t), minute(t), second(t));
          __SA.Nachricht(Sonnenaufgang::rot, Sonnenaufgang::lang); // Rot zeigt, dass jetzt Weckzeiten de-aktiviert sind
          __WZ.Wecker_Aktiv(false);
        } else { // Wecker inaktiv --> auf aktiv setzen und grün blinken
			Serial.printf("Knopf lang - Wecker AN  %d:%02d:02d\n", hour(t), minute(t), second(t));
          __SA.Nachricht(Sonnenaufgang::gruen, Sonnenaufgang::lang); // Rot zeigt, dass jetzt Weckzeiten de-aktiviert sind
          __WZ.Wecker_Aktiv(true);
        }
      }
      if  (__Knopf.WieLang() == 6) { // jetzt wirklich lang gedrückt (~10s) --> schalte OTA ein
		Serial.printf("Knopf GAAANZ lang - OTA AN  %d:%02d:02d\n", hour(t), minute(t), second(t));
        __SA.Nachricht(Sonnenaufgang::lila, Sonnenaufgang::lang); // lila zeigt, dass jetzt OTA aktiviert ist
        __OTA.Bereit();
      }
      break;
  }

  if (__WZ.jetztWecken(t)) {
    if (!__SA.Laeuft()) {
      Serial.printf("Weckzeit erreicht, starte Sonnenaufgang um %d:%02d:%02d\n", hour(t), minute(t), second(t));
      __SA.Start();
    }
  }

  __SA.Tick();
  __OTA.Tick();
  __WebS.Tick();

  delay(20);
}
