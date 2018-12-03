#include "Sonnenaufgang.h"
#include "Knopf.h"
#include "NTP.h"
#include "Speicher.h"

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#include <Time.h>
#include <TimeLib.h>

// Globale Variablen starten immer mit __[a-z]
// lokale Variable starten mit _[a-z]
// Argumente starten mit [a-z]


ESP8266WebServer server(80);
const char* ssid = "0024A5C6D897";
const char* password = "u5rr1xembpu1c";

/*typedef struct WeckZeit_S {
  uint8_t _h;
  uint8_t _m;
  uint8_t _s;
  bool _An;
  } WZ_S;

  WZ_S __Weckzeit;
*/

Speicher __WZ;

Sonnenaufgang __SA;
Knopf __Knopf;
NTP_Helfer __NTP;

void handleRoot() {
  char temp[1000];
  time_t t = now(); // Store the current time in time
  time_t w = __WZ.Weckzeit(0);
  bool a = __WZ.Wecker_An(0);
  Serial.printf("Webaufruf / um %2d:%02d:%02d\n", hour(t), minute(t), second(t));
  //<meta http-equiv='refresh' content='5'/>
  //iframe width='30em' height='10'
  snprintf(temp, 1000,
           "<html><head><title>Lichtwecker</title></head>\
<body><h1>Lichtwecker</h1><p>Zeit: <iframe src='/LokaleZeit' seamless name='LokaleZeit'>%02d:%02d:%02d</p>\n\
<form action='/Weckzeit' method='POST'>Weckzeit\
<input type='number' name='Stunde' min='0' max='23' value='%d'>\
<input type='number' name='Minute'  min='0' max='59' value='%02d'>\
<input type='number' name='Sekunde' min='0' max='59' value='%02d'>\
<input type='checkbox' name='An' %s><label for='An'>An</label>\
<input type='submit' name='ok' value='ok'></form>\
<form action='/StartStop' method='POST'>Jetzt testen<input type='submit' name='Schalten' value='%s'></form>\
</html>",
           hour(t), minute(t), second(t), hour(w), minute(w), second(w), a ? "checked" : "", __SA.Laeuft() ? "stop" : "start"
          );
  server.send(200, "text/html", temp);
}

void handleStartStop() {
  time_t t = now(); // Store the current time in time
  Serial.printf("Webaufruf /StartStop um %2d:%2d:%2d\n", hour(t), minute(t), second(t));
  if ((server.args() == 1) && (server.argName(0) == "Schalten")) {
    if (server.arg(0) == "start") {
      Serial.printf("Starte Sonnenaufgang\n");
      __SA.Start();
      server.sendHeader("Location", "/");
      server.send(303, "text/html", "Location: /");
    } else {
      Serial.printf("Stoppe Sonnenaufgang\n");
      __SA.Stop();
      server.sendHeader("Location", "/");
      server.send(303, "text/html", "Location: /");
    }
  } else {
    Serial.printf("Fehler in Args\n");
    String message = "Fehler in den Args\n\n";
    for (uint8_t i = 0; i < server.args(); i++) {
      message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    }
    server.send(400, "text/plain", message);
  }
}

void handleWeckzeit() {
  time_t t = now(); // Store the current time in time
  Serial.printf("Webaufruf /Weckzeit um %2d:%2d:%2d\n", hour(t), minute(t), second(t));
  if ((((server.args() == 5) && (server.argName(3) == "An")) || (server.args() == 4)) &&
      (server.argName(0) == "Stunde") && (server.argName(1) == "Minute") && (server.argName(2) == "Sekunde")
     ) {
    tmElements_t tmSet;
    tmSet.Year = 1;
    tmSet.Month = 1;
    tmSet.Day = 1;
    tmSet.Hour = min(23, max(0, int(server.arg(0).toInt())));
    tmSet.Minute = min(59, max(0, int(server.arg(1).toInt())));
    tmSet.Second = min(59, max(0, int(server.arg(2).toInt())));

    time_t w = makeTime(tmSet);
    __WZ.setze_Weckzeit(0, w, (server.arg(3) == "on"));
    __WZ.speichern();
    Serial.printf("Neue Weckzeit: %d:%02d:%02d %s\n", hour(t), minute(t), second(t), (server.arg(3) == "on") ? "An" : "Aus");
    server.sendHeader("Location", "/");
    server.send(303, "text/html", "Location: /");
  } else {
    Serial.printf("Fehler in Args\n");
    String message = "Fehler in den Args\n\n";
    for (uint8_t i = 0; i < server.args(); i++) {
      message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    }
    server.send(400, "text/plain", message);
  }

}

void handleFavIcon() {
  server.send(404, "text/plain", "no favicon");
}

void handleLokaleZeit() {
  char temp[1000];
  time_t t = now(); // Store the current time in time
  Serial.printf("Webaufruf /LokaleZeit um %2d:%02d:%02d\n", hour(t), minute(t), second(t));
  snprintf(temp, 1000,
           "<html><head><meta http-equiv='refresh' content='1'/></head><body>%02d:%02d:%02d</body></html>",
           hour(t), minute(t), second(t));
  server.send(200, "text/html", temp);
}

void handleNotFound() {
  time_t t = now(); // Store the current time in time
  Serial.printf("Webaufruf -unbekannte Seite %s um %2d:%2d:%2d\n", server.uri().c_str(), hour(t), minute(t), second(t));
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void setup() {
  // Schritt 0: Seriellen Output enablen
  Serial.begin(115200);
  Serial.print("Starte...");

  // Schritt 1: die interne LED konfigurieren
  pinMode(LED_BUILTIN, OUTPUT);// bei Sonoff Basic Pin 13
  digitalWrite(LED_BUILTIN, HIGH); // bei Sonoff Basic HIGH = OFF
  Serial.print(" interneLED");

  // Schritt 3: die LED-Kette
  __SA.Beginn();
  Serial.println(" LED-Kette");

  // Schritt 4: Wifi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Keine Wifi-Verbingung! Neustart in 5 Sekunden...");
    delay(5000);
    ESP.restart();
  }
  Serial.printf(" Wifi %s (IP Address %s)", ssid, WiFi.localIP().toString().c_str());

  if (MDNS.begin("esp8266")) {
    Serial.print(" MDNS responder");
  }

  // Schritt 5: Webserver konfigurieren
  server.on("/", handleRoot);
  server.on("/StartStop", handleStartStop);
  server.on("/Weckzeit", handleWeckzeit);
  server.on("/LokaleZeit", handleLokaleZeit);
  server.on("/favicon.ico", handleFavIcon);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println(" Webserver laeuft");

  // Schritt 6: Zeitserver konfigurieren und starten
  __NTP.Beginn();
  Serial.print(" Zeitservice");

  // Schritt 6: Wert vom Knopf setzen
  __Knopf.Beginn();
  Serial.print(" Taster");

  // Schritt 7: EEPROM
  __WZ.Beginn();
  Serial.print(" gespeicherte Werte ok");

  Serial.println("Fertig");
}

void loop() {
  time_t t = now(); // Store the current time in time
  if (Serial.available() > 0) {
    // read the incoming data:
    while (Serial.available() > 0) {
      char _c  = Serial.read();
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
    case Knopf::kurz:
      Serial.printf("Knopf kurz, Nachricht gelb um %d:%02d:02d\n", hour(t), minute(t), second(t));
      __SA.Snooze();
      break;
    case Knopf::lang:
      Serial.printf("Knopf lang - start Sonnenaufgang um %d:%02d:02d\n", hour(t), minute(t), second(t));
      if (__SA.Laeuft()) {
        __SA.Nachricht(Sonnenaufgang::gelb); // Gelb zeigt, dass jetzt 24 h Ruhe ist
      } else {
        time_t w = __WZ.Weckzeit(0);
        if (__WZ.Wecker_An(0)) {
          __SA.Nachricht(Sonnenaufgang::rot); // Rot zeigt, dass jetzt Weckzeiten de-aktiviert sind
          // ist nur ein Hack, muss noch richtig gemacht werden (eigener Status)
          __WZ.setze_Weckzeit(0, w, false);
        } else {
          __SA.Nachricht(Sonnenaufgang::gruen); // Rot zeigt, dass jetzt Weckzeiten de-aktiviert sind
          // ist nur ein Hack, muss noch richtig gemacht werden (eigener Status)
          __WZ.setze_Weckzeit(0, w, true);
        }
      }
      break;
  }

  if (__WZ.jetztWecken(t)) {
    if (!__SA.Laeuft()) {
      Serial.printf("Weckzeit erreicht, starte Sonnenaufgang um %d:%02d:02d\n", hour(t), minute(t), second(t));
      __SA.Start();
    }
  }

  __SA.Tick();
  server.handleClient();
  delay(20);
}

