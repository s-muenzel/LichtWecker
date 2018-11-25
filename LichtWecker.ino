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


Speicher __WZ;
Sonnenaufgang __SA;
Knopf __Knopf;
NTP_Helfer __NTP;

void handleRoot() {
  char temp[2000];
  time_t t = now(); // Store the current time in time
  time_t w[7];
  bool a[7];
  for (int i = 0; i < 7; i++) {
    w[i] = __WZ.Weckzeit(i);
    a[i] = __WZ.Wecker_An(i);
  }
  Serial.printf("Webaufruf / um %2d:%02d:%02d\n", hour(t), minute(t), second(t));
  snprintf(temp, 2000,
           "<html><head><title>Lichtwecker</title></head>\
<body><h1>Lichtwecker</h1><p>Zeit: <iframe src='/LokaleZeit' style='border:none;height:26px;width:100px;' name='LokaleZeit'>%2d:%02d:%02d</iframe></p>\n\
<form action='/Setze_WZ' method='POST'><table>\
<tr><th>Wecker an</th><th></th><th><input type='checkbox' name='Aktiv' %s></th></tr>\
<tr><td>Montag</td><td><input type='time' name='Mo' value='%02d:%02d'></td><td><input type='checkbox' name='AnMo' %s></td></tr>\
<tr><td>Dienstag</td><td><input type='time' name='Di' value='%02d:%02d'></td><td><input type='checkbox' name='AnDi' %s></td></tr>\
<tr><td>Mittwoch</td><td><input type='time' name='Mi' value='%02d:%02d'></td><td><input type='checkbox' name='AnMi' %s></td></tr>\
<tr><td>Donnerstag</td><td><input type='time' name='Do' value='%02d:%02d'></td><td><input type='checkbox' name='AnDo' %s></td></tr>\
<tr><td>Freitag</td><td><input type='time' name='Fr' value='%02d:%02d'></td><td><input type='checkbox' name='AnFr' %s></td></tr>\
<tr><td>Samstag</td><td><input type='time' name='Sa' value='%02d:%02d'></td><td><input type='checkbox' name='AnSa' %s></td></tr>\
<tr><td>Sonntag</td><td><input type='time' name='So' value='%02d:%02d'></td><td><input type='checkbox' name='AnSo' %s></td></tr>\
<tr><td></td><td></td><td><input type='submit' name='ok' value='ok'></td></tr></table></form>\
<form action='/StartStop' method='POST'>Jetzt testen<input type='submit' name='Schalten' value='%s'></form>\
</html>",
           hour(t), minute(t), second(t),
           __WZ.Wecker_Aktiv() ? "checked" : "",
           hour(w[2]), minute(w[2]), a[2] ? "checked" : "",
           hour(w[3]), minute(w[3]), a[3] ? "checked" : "",
           hour(w[4]), minute(w[4]), a[4] ? "checked" : "",
           hour(w[5]), minute(w[5]), a[5] ? "checked" : "",
           hour(w[6]), minute(w[6]), a[6] ? "checked" : "",
           hour(w[0]), minute(w[0]), a[0] ? "checked" : "",
           hour(w[1]), minute(w[1]), a[1] ? "checked" : "",
           __SA.Laeuft() ? "stop" : "start"
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

uint8_t parseZeit_Stunde(String s) {
  return min(23, max(0, int(s.toInt())));
}
uint8_t parseZeit_Minute(String s) {
  return min(59, max(0, int(s.substring(3).toInt())));
}

void handleSetzeWeckzeit() {
  time_t t = now(); // Store the current time in time
  Serial.printf("Webaufruf /Setze_WZ um %2d:%2d:%2d\n", hour(t), minute(t), second(t));
  tmElements_t tmSet;
  tmSet.Year = 1;
  tmSet.Month = 1;
  tmSet.Day = 1;
  tmSet.Second = 0;
  bool _Aktiv = false;
  time_t w[7];
  bool a[7];
  bool u[7]; // Zeit für Wochentag gefunden..

  for (int i = 0; i < 7; i++) {
    a[i] = false;
    u[7] = false;
  }

  for (int i = 0; i < server.args(); i++) {
    if (server.argName(i) == "Aktiv") {
      _Aktiv = true;
    } else if (server.argName(i) == "Mo") {
      u[2] = true;
      tmSet.Hour = parseZeit_Stunde(server.arg(i));
      tmSet.Minute = parseZeit_Minute(server.arg(i));
      w[2] = makeTime(tmSet);
    } else if (server.argName(i) == "Di") {
      u[3] = true;
      tmSet.Hour = parseZeit_Stunde(server.arg(i));
      tmSet.Minute = parseZeit_Minute(server.arg(i));
      w[3] = makeTime(tmSet);
    } else if (server.argName(i) == "Mi") {
      u[4] = true;
      tmSet.Hour = parseZeit_Stunde(server.arg(i));
      tmSet.Minute = parseZeit_Minute(server.arg(i));
      w[4] = makeTime(tmSet);
    } else if (server.argName(i) == "Do") {
      u[5] = true;
      tmSet.Hour = parseZeit_Stunde(server.arg(i));
      tmSet.Minute = parseZeit_Minute(server.arg(i));
      w[5] = makeTime(tmSet);
    } else if (server.argName(i) == "Fr") {
      u[6] = true;
      tmSet.Hour = parseZeit_Stunde(server.arg(i));
      tmSet.Minute = parseZeit_Minute(server.arg(i));
      w[6] = makeTime(tmSet);
    } else if (server.argName(i) == "Sa") {
      u[0] = true;
      tmSet.Hour = parseZeit_Stunde(server.arg(i));
      tmSet.Minute = parseZeit_Minute(server.arg(i));
      w[0] = makeTime(tmSet);
    } else if (server.argName(i) == "So") {
      u[1] = true;
      tmSet.Hour = parseZeit_Stunde(server.arg(i));
      tmSet.Minute = parseZeit_Minute(server.arg(i));
      w[1] = makeTime(tmSet);
    } else if (server.argName(i) == "AnMo") {
      a[2] = true;
    } else if (server.argName(i) == "AnDi") {
      a[3] = true;
    } else if (server.argName(i) == "AnMi") {
      a[4] = true;
    } else if (server.argName(i) == "AnDo") {
      a[5] = true;
    } else if (server.argName(i) == "AnFr") {
      a[6] = true;
    } else if (server.argName(i) == "AnSa") {
      a[0] = true;
    } else if (server.argName(i) == "AnSo") {
      a[1] = true;
    }
  }
  Serial.printf("Neue Weckzeiten:\nAktiv: %s\n", _Aktiv ? "Ja" : "Nein");
  __WZ.Wecker_Aktiv(_Aktiv);
  for (int i = 0; i < 7; i++) {
    if (u[i]) {
      Serial.printf("%d: %02d:%02d %s\n", i, hour(w[i]), minute(w[i]), a[i] ? "An" : "Aus");
      __WZ.setze_Weckzeit(i, w[i], a[i]);
    }
  }
  __WZ.speichern();
  server.sendHeader("Location", "/");
  server.send(303, "text/html", "Location:/");
}

void handleKonfig() {
  char temp[2000];
  time_t t = now(); // Store the current time in time
  Serial.printf("Webaufruf /Konfig um %2d:%02d:%02d\n", hour(t), minute(t), second(t));
  snprintf(temp, 2000,
           "<html><head><title>Lichtwecker</title></head>\
<body><h1>Lichtwecker</h1><h2>Konfiguration</h2>\n\
<form action='/Setze_Konfig' method='POST'><table>\
<tr><td>Laenge</td><td><input type='number' min='0.5' max='2.0' step='0.1'  name='L' value='%f'></td></tr>\
<tr><td>Geschwindigkeit</td><td><input type='number' min='0.01' max='1.0' step='0.01' name='V' value='%f'></td></tr>\
<tr><td>Dauer Aufgang</td><td><input type='number' min='10' max='300' step='1' name='D' value='%f'></td></tr>\
<tr><td>Dauer Hell</td><td><input type='number' min='2' max='300' step='1'  name='N' value='%f'></td></tr>\
<tr><td>Snooze-Zeit</td><td><input type='number' min='10' max='600' step='1'  name='S' value='%f'></td></tr>\
<tr><td></td><td></td><td><input type='submit' name='ok' value='ok'></td></tr></table></form>\
</html>",
           __WZ.lese_SA_laenge(),
           __WZ.lese_SA_v(),
           __WZ.lese_SA_dauer(),
           __WZ.lese_SA_nachleuchten(),
           __WZ.lese_SA_snooze()
          );
  server.send(200, "text/html", temp);
}

void handleSetzeKonfig() {
  time_t t = now(); // Store the current time in time
  Serial.printf("Webaufruf /Setze_Konfig um %2d:%2d:%2d\n", hour(t), minute(t), second(t));

  for (int i = 0; i < server.args(); i++) {
    if (server.argName(i) == "L") {
      float f = server.arg(i).toFloat();
      Serial.printf("Konfig: Laenge: %f\n", f);
      __WZ.setze_SA_laenge(f);
      __SA.Setze_Laenge(f);
    } else if (server.argName(i) == "V") {
      float f = server.arg(i).toFloat();
      Serial.printf("Konfig: V: %f\n", f);
      __WZ.setze_SA_v(f);
      __SA.Setze_v(f);
    } else if (server.argName(i) == "D") {
      float f = server.arg(i).toFloat();
      Serial.printf("Konfig: Dauer: %f\n", f);
      __WZ.setze_SA_dauer(f);
      __SA.Setze_Dauer(f);
    } else if (server.argName(i) == "N") {
      float f = server.arg(i).toFloat();
      Serial.printf("Konfig: Nachleuchten: %f\n", f);
      __WZ.setze_SA_nachleuchten(f);
      __SA.Setze_Nachleuchten(f);
    } else if (server.argName(i) == "S") {
      float f = server.arg(i).toFloat();
      Serial.printf("Konfig: Snooze: %f\n", f);
      __WZ.setze_SA_snooze(f);
      __SA.Setze_Snooze(f);
    }
  }
  __WZ.speichern();
  server.sendHeader("Location", "/");
  server.send(303, "text/html", "Location:/");
}

void handleLokaleZeit() {
  char temp[1000];
  time_t t = now(); // Store the current time in time
  char style[60];
  switch (timeStatus()) {
    case timeNotSet:
      strncpy(style, "color:red;background-color:yellow;", 59);
      break;
    case timeNeedsSync:
      strncpy(style, "color:yellow;", 59);
      break;
    case timeSet:
    default:
      strncpy(style, "color:black;", 59);
      break;
  }
  //  Serial.printf("Webaufruf /LokaleZeit um %2d:%02d:%02d\n", hour(t), minute(t), second(t));
  snprintf(temp, 1000,
           "<html><head><meta http-equiv='refresh' content='1'/></head><body style='%s'>%2d:%02d:%02d</body></html>",
           style, hour(t), minute(t), second(t));
  server.send(200, "text/html", temp);
}

void handleFavIcon() {
  server.send(404, "text/plain", "no favicon");
}

void handleNotFound() {
  time_t t = now(); // Store the current time in time
  Serial.printf("Webaufruf - unbekannte Seite %s um %2d:%02d:%02d\n", server.uri().c_str(), hour(t), minute(t), second(t));
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ":" + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

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

  if (MDNS.begin("esp8266")) {
    Serial.print(" MDNS responder");
  }

  // Zeitserver konfigurieren und starten
  __NTP.Beginn();
  Serial.print(" Zeitservice");

  // Webserver konfigurieren
  server.on("/", handleRoot); // Anzeige Weckzeiten und Möglichkeit Weckzeiten zu setzen. Auch Link zu Konfig
  server.on("/StartStop", handleStartStop); // nur noch zu Testzwecken
  server.on("/Setze_WZ", handleSetzeWeckzeit); // Speichert die neuen Weckzeiten ab
  server.on("/Konfig", handleKonfig); // Zeigt die Konfig-Daten an
  server.on("/Setze_Konfig", handleSetzeKonfig); // Speichert die neuen Weckzeiten ab
  server.on("/LokaleZeit", handleLokaleZeit); // zeigt "nur" die aktuelle lokale Zeit an
  server.on("/favicon.ico", handleFavIcon); // dummy
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println(" Webserver laeuft");

  // Wert vom Knopf setzen
  __Knopf.Beginn();
  Serial.print(" Taster");


  Serial.println("Fertig");
}

void loop() {
  time_t t = now(); // Store the current time in time

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
    case Knopf::kurz:
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
    case Knopf::lang:
      Serial.printf("Knopf lang - start Sonnenaufgang um %d:%02d:02d\n", hour(t), minute(t), second(t));
      if (__SA.Laeuft()) {
        __SA.Nachricht(Sonnenaufgang::gelb, Sonnenaufgang::lang); // Gelb zeigt, dass jetzt 24 h Ruhe ist
      } else {
        if (__WZ.Wecker_Aktiv()) { // Wecker aktiv --> auf inaktiv setzen und rot blinken
          __SA.Nachricht(Sonnenaufgang::rot, Sonnenaufgang::lang); // Rot zeigt, dass jetzt Weckzeiten de-aktiviert sind
          __WZ.Wecker_Aktiv(false);
        } else { // Wecker inaktiv --> auf aktiv setzen und grün blinken
          __SA.Nachricht(Sonnenaufgang::gruen, Sonnenaufgang::lang); // Rot zeigt, dass jetzt Weckzeiten de-aktiviert sind
          __WZ.Wecker_Aktiv(true);
        }
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
  server.handleClient();

  delay(20);
}

