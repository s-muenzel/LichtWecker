#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>


#include <Time.h>
#include <TimeLib.h>


// Globale Variablen starten immer mit __[a-z]
// lokale Variable starten mit _[a-z]
// Argumente starten mit [a-z]

#include <Adafruit_NeoPixel.h>

// an welchem PIN hängt die Lichterkette
//#define PIN 6
#define PIN 14 // Sonoff GPIO 14

// Meine LED-Kette ist 1m mit 30 LEDs
#define NUM_LEDS 60
#define LAENGE 1.0f          // [m] Länge des LED-Strips
#define GESCHWINDIGKEIT 0.2f // [m/s] Ausbreitungsgeschwindigkeit v 
#define DAUER 20.0f          // [s] wie lange dauert der "Sonnenaufgang"
#define NACHLEUCHTEN 2.0f    // [s] wie lange bleibt das Licht nach dem "Sonnenaufgang" an

ESP8266WebServer server(80);
IPAddress timeServer(192, 168, 2, 1); // unsere Fritzbox
const int timeZone = 1;     // Central European Time
WiFiUDP Udp;
unsigned int localPort = 8888;  // local port to listen for UDP packets

const char* ssid = "0024A5C6D897";
const char* password = "u5rr1xembpu1c";

Adafruit_NeoPixel __strip = Adafruit_NeoPixel(NUM_LEDS, PIN, NEO_GRB + NEO_KHZ800);

// Kudus an hdznrrd au Github (fehlerkorrigiert)
void HSV_to_RGB(float h, float s, float v, uint8_t *r, uint8_t *g, uint8_t *b)
{
  int i;
  float f, p, q, t;

  h = max(0.0f, min(360.0f, h));
  s = max(0.0f, min(1.0f, s));
  v = max(0.0f, min(1.0f, v));

  if (s == 0) {
    // Achromatic (grey)
    *r = *g = *b = round(v * 255);
    return;
  }

  h /= 60; // sector 0 to 5
  i = floor(h);
  f = h - i; // factorial part of h
  p = v * (1 - s);
  q = v * (1 - s * f);
  t = v * (1 - s * (1 - f));
  switch (i) {
    case 0:
      *r = round(255 * v);
      *g = round(255 * t);
      *b = round(255 * p);
      break;
    case 1:
      *r = round(255 * q);
      *g = round(255 * v);
      *b = round(255 * p);
      break;
    case 2:
      *r = round(255 * p);
      *g = round(255 * v);
      *b = round(255 * t);
      break;
    case 3:
      *r = round(255 * p);
      *g = round(255 * q);
      *b = round(255 * v);
      break;
    case 4:
      *r = round(255 * t);
      *g = round(255 * p);
      *b = round(255 * v);
      break;
    default: // case 5:
      *r = round(255 * v);
      *g = round(255 * p);
      *b = round(255 * q);
  }
}

uint32_t Lichtfarbe(float t, float x) {
  // Kurvenverlauf für Lichtfarbe(t-x/v):
  //        ____
  //       /
  //      /
  //  ___/
  //
  // x ausserhalb der Wert macht keine Sinn
  float _x = max(0.0f, min(LAENGE, x));
  float _t_x = t - _x / GESCHWINDIGKEIT;
  _t_x = max(0.0f, min(DAUER, _t_x));
  _t_x /= DAUER;

  float _h;
  if (_t_x < 0.9)
    _h = 10. +  _t_x * 50. ;
  else
    _h = -1610. +  _t_x * 1850. ;
  float _s;
  if (_t_x < 0.9)
    _s = 1. - _t_x / 2. ;
  else
    _s = 0.55 + 4 * 0.9 - _t_x * 4;
  float _v = _t_x ;
  uint8_t _r;
  uint8_t _g;
  uint8_t _b;
  HSV_to_RGB(_h, _s, _v, &_r, &_g, &_b);
  static float _deb_zeit = 0;
  if (t > _deb_zeit) {
    _deb_zeit = t;
    Serial.print(" t="); Serial.print(t);
    Serial.print(" h/s/v=");
    Serial.print(_h); Serial.print("/");
    Serial.print(_s); Serial.print("/");
    Serial.print(_v);
    Serial.print(" r/g/b=");
    Serial.print(_r); Serial.print("/");
    Serial.print(_g); Serial.print("/");
    Serial.print(_b); Serial.println("");
  }
  return __strip.Color(_r, _g, _b);

}

class Sonnenaufgang {
  public:
    Sonnenaufgang(float dauer = DAUER, float nachleuchten = NACHLEUCHTEN) {
      _Nachlaufzeit = round(nachleuchten * 1000);
      _Dauer = round(dauer * 1000);
    }

    void Start() {
      // Start merkt sich die Startzeit (jetzt).
      // Wenn Startzeit > 0 ist, läuft ein Sonnenaufgang
      // Sollte ein Sonnenaufgang bereits laufen --> von neuem anfangen
      _Startzeit = millis();
      digitalWrite(LED_BUILTIN, LOW); // bei Sonoff Basic HIGH = OFF
      Serial.printf("Starte Sonnenaufgang bei %d\n", _Startzeit);
      Serial.printf("Dauer %d Nachlaufzeit %d\n", _Dauer, _Nachlaufzeit);
    }

    void Stop() {
      Serial.printf("Stoppe Sonnenaufgang bei %d (nach %d)\n", millis(), millis() - _Startzeit);
      // Stop löscht das Licht und setzt _Startzeit wieder auf 0
      for (uint16_t _n = 0; _n < __strip.numPixels(); _n++) {
        __strip.setPixelColor(_n, 0);
      }
      __strip.show();
      _Startzeit = 0;
      digitalWrite(LED_BUILTIN, HIGH); // bei Sonoff Basic HIGH = OFF
    }

    bool Laeuft() {
      return _Startzeit > 0;
    }

    void Tick() {
      // Sollte ein Sonnenaufgang laufen, das Licht entsprechend anpassen..
      if (_Startzeit > 0) {
        if (millis() > _Startzeit + _Dauer + _Nachlaufzeit) {
          Stop();
        } else {
          for (uint16_t _n = 0; _n < __strip.numPixels(); _n++) {
            float _x = (float)_n * LAENGE / __strip.numPixels();
            __strip.setPixelColor(_n, Lichtfarbe(_ms / 1000., _x));
          }
          __strip.show();
        }
      }
    }
  private:
    long _Startzeit;    // [ms] - läuft, bzw. seit wann läuft ein S.A.
    long _Nachlaufzeit; // [ms] - wie lange bleibt das Licht nach der Dauer des Sonnenuntergangs an?
    long _Dauer;        // [ms] - wie lange dauert es, bis bei einem S.A. das Licht auf MAX ist
};

Sonnenaufgang __SA;

void handleRoot() {
  char temp[600];
  time_t t = now(); // Store the current time in time
  Serial.printf("Webaufruf / um %2d:%2d:%2d\n", hour(t), minute(t), second(t));

  snprintf(temp, 600,
           "<html><head><meta http-equiv='refresh' content='5'/><title>ESP8266 Demo</title><style>body{background-color: #cccccc; Color: #000088; }</style></head>\
<body><h1>Lichtwecker </h1><p>Zeit: %02d:%02d:%02d</p>\n\
<form action='/StartStop' method='POST'>%s<input type='submit' name='Schalten' value='%s'></form>\
</html>",
           hour(t), minute(t), second(t), __SA.Laeuft() ? "stop" : "start", __SA.Laeuft() ? "stop" : "start"
          );
  Serial.print(" Msg fertig");
  server.send(200, "text/html", temp);
  Serial.println(" und gesendet");
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


void setup() {
  // Schritt 0: Seriellen Output enablen
  Serial.begin(115200);
  Serial.print("Starte...");

  // Schritt 1: die interne LED konfigurieren
  pinMode(LED_BUILTIN, OUTPUT);// bei Sonoff Basic Pin 13
  digitalWrite(LED_BUILTIN, HIGH); // bei Sonoff Basic HIGH = OFF
  Serial.print(" interneLED");

  // Schritt 3: die LED-Kette
  __strip.begin();
  __strip.setBrightness(255);
  __strip.show(); // Initialiere alle auf "Aus"

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
  server.on("/favicon.ico",handleFavIcon);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println(" Webserver laeuft");

  // Schritt 6: Zeitserver konfigurieren und starten
  Udp.begin(localPort);
  setSyncProvider(getNtpTime);
  Serial.println(" Ntp-Service gestartet\n");

    Serial.println("Fertig");
}

void loop() {
  if (Serial.available() > 0) {
    // read the incoming byte:
    char _c  = Serial.read();
    Serial.print("Starte Sonnenaufgang: ");
    Serial.println(_c, DEC);
    time_t t = now(); // Store the current time in time
    Serial.print(hour(t));
    Serial.print(":");
    Serial.print(minute(t));
    Serial.print(":");
    Serial.print(second(t));
    __SA.Start();
  }
  __SA.Tick();
  server.handleClient();
  delay(20);
}

