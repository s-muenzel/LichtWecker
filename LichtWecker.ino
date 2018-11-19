#include "Sonnenaufgang.h"

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>


#include <Time.h>
#include <TimeLib.h>

#include <EEPROM.h>


// Globale Variablen starten immer mit __[a-z]
// lokale Variable starten mit _[a-z]
// Argumente starten mit [a-z]


ESP8266WebServer server(80);
IPAddress timeServer(192, 168, 2, 1); // unsere Fritzbox
const int timeZone = 1;     // Central European Time
WiFiUDP Udp;
unsigned int localPort = 8888;  // local port to listen for UDP packets

const char* ssid = "0024A5C6D897";
const char* password = "u5rr1xembpu1c";

typedef struct WeckZeit_S {
  uint8_t _h;
  uint8_t _m;
  uint8_t _s;
  bool _An;
} WZ_S;

WZ_S __Weckzeit;

Sonnenaufgang __SA;

void handleRoot() {
  char temp[600];
  time_t t = now(); // Store the current time in time
  Serial.printf("Webaufruf / um %2d:%02d:%02d\n", hour(t), minute(t), second(t));
  //<style>body{background-color: #cccccc; Color: #000088; }</style>
  snprintf(temp, 700,
           "<html><head><meta http-equiv='refresh' content='5'/><title>Lichtwecker</title></head>\
<body><h1>Lichtwecker</h1><p>Zeit: %02d:%02d:%02d</p>\n\
<form action='/Weckzeit' method='POST'>Weckzeit\
<input type='number' name='Stunde' min='0' max='23' value='%d'>\
<input type='number' name='Minute'  min='0' max='59' value='%02d'>\
<input type='number' name='Sekunde' min='0' max='59' value='%02d'>\
<input type='checkbox' name='An' %s><label for='An'>An</label>\
<input type='submit' name='ok' value='ok'></form>\
<form action='/StartStop' method='POST'>Jetzt testen<input type='submit' name='Schalten' value='%s'></form>\
</html>",
           hour(t), minute(t), second(t), __Weckzeit._h, __Weckzeit._m, __Weckzeit._s, __Weckzeit._An ? "checked" : "", __SA.Laeuft() ? "stop" : "start"
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
  if (((server.args() == 5) && (server.argName(3) == "An") || (server.args() == 4)) &&
      (server.argName(0) == "Stunde") && (server.argName(1) == "Minute") && (server.argName(2) == "Sekunde")
     ) {
    __Weckzeit._h = min(23, max(0, int(server.arg(0).toInt())));
    __Weckzeit._m = min(59, max(0, int(server.arg(1).toInt())));
    __Weckzeit._s = min(59, max(0, int(server.arg(2).toInt())));
    __Weckzeit._An = (server.arg(3) == "on");
    Serial.printf("Neue Weckzeit: %d:%02d:%02d %s\n", __Weckzeit._h, __Weckzeit._m, __Weckzeit._s, __Weckzeit._An ? "An" : "Aus");
    EEPROM.put(0, __Weckzeit);
    EEPROM.commit();
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

/*-------- NTP code ----------*/

const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

time_t getNtpTime()
{
  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  Serial.println("Transmit NTP Request");
  sendNTPpacket(timeServer);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
  Serial.println("No NTP Response :-(");
  return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}

bool __KnopfStatus;

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
  server.on("/favicon.ico", handleFavIcon);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println(" Webserver laeuft");

  // Schritt 6: Zeitserver konfigurieren und starten
  Udp.begin(localPort);
  setSyncProvider(getNtpTime);
  Serial.println(" Ntp-Service gestartet\n");

  // Schritt 6: Wert vom Knopf setzen
  pinMode(0, INPUT);
  __KnopfStatus = (digitalRead(0) == LOW);
  if (__KnopfStatus) {
    Serial.println("Knopf gedrückt");
  } else {
    Serial.println("Knopf nicht gedrückt");
  }

  // Schritt 7: EEPROM
  EEPROM.begin(sizeof(WZ_S));
  EEPROM.get(0, __Weckzeit);
  Serial.printf("_Weckzeit: %d:%02d:%02d \n", __Weckzeit._h, __Weckzeit._m, __Weckzeit._s);
  Serial.printf("_Weckzeit: (%s)\n", __Weckzeit._An ? "An" : "Aus");

  Serial.println("Fertig");
}

void loop() {
  time_t t = now(); // Store the current time in time
  if (Serial.available() > 0) {
    // read the incoming byte:
    char _c  = Serial.read();
    if (__SA.Laeuft()) {
      Serial.printf("Stoppe Sonnenaufgang um %d:%02d:02d\n", hour(t), minute(t), second(t));
      __SA.Start();
    } else {
      Serial.printf("Starte Sonnenaufgang um %d:%02d:02d\n", hour(t), minute(t), second(t));
      __SA.Start();
    }
  }
  bool _knopf = (digitalRead(0) == LOW);
  if (_knopf) { // Knopf gedrueckt
    if (!__KnopfStatus) { // vorher nicht gedrueckt --> tue was
      if (__SA.Laeuft()) {
        Serial.printf("Stoppe Sonnenaufgang um %d:%02d:02d\n", hour(t), minute(t), second(t));
        __SA.Stop();
      } else {
        Serial.printf("Starte Sonnenaufgang um %d:%02d:02d\n", hour(t), minute(t), second(t));
        __SA.Start();
      }
      __KnopfStatus = true;
    }
  } else { // Knopf nicht gedrueckt, vorher immer auf nicht gedrueckt setzen
    __KnopfStatus = false;
  }

  if (__Weckzeit._An && (hour(t) == __Weckzeit._h) && (minute(t) == __Weckzeit._m) && (second(t) == __Weckzeit._s)) {
    if (!__SA.Laeuft()) {
      Serial.printf("Weckzeit erreicht, starte Sonnenaufgang um %d:%02d:02d\n", hour(t), minute(t), second(t));
      __SA.Start();
    }
  }
  __SA.Tick();
  server.handleClient();
  delay(20);
}

