#include <Time.h>
#include <TimeLib.h>


// Globale Variablen starten immer mit __[a-z]
// lokale Variable starten mit _[a-z]
// Argumente starten mit [a-z]

#include <Adafruit_NeoPixel.h>

// an welchem PIN hängt die Lichterkette
#define PIN 6
// Meine LED-Kette ist 1m mit 30 LEDs
#define NUM_LEDS 30
#define LAENGE 1.0          // [m] Länge des LED-Strips
#define GESCHWINDIGKEIT 0.2 // [m/s] Ausbreitungsgeschwindigkeit v 
#define DAUER 20.0          // [s] wie lange dauert der "Sonnenaufgang"
#define NACHLEUCHTEN 2.0    // [s] wie lange bleibt das Licht nach dem "Sonnenaufgang" an

Adafruit_NeoPixel __strip = Adafruit_NeoPixel(NUM_LEDS, PIN, NEO_GRB + NEO_KHZ800);

// Kudus an hdznrrd au Github (fehlerkorrigiert)
void HSV_to_RGB(float h, float s, float v, uint8_t *r, uint8_t *g, uint8_t *b)
{
  int i;
  float f, p, q, t;

  h = max(0.0, min(360.0, h));
  s = max(0.0, min(1.0, s));
  v = max(0.0, min(1.0, v));

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
  float _x = max(0, min(LAENGE, x));
  float _t_x = t - _x / GESCHWINDIGKEIT;
  _t_x = max(0, min(DAUER, _t_x));
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
  if (_t_x > _deb_zeit) {
    _deb_zeit = _t_x;
    Serial.print(" _t_x="); Serial.print(_t_x);
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
    }

    void Stop() {
      // Stop löscht das Licht und setzt _Startzeit wieder auf 0
      for (uint16_t _n = 0; _n < __strip.numPixels(); _n++) {
        __strip.setPixelColor(_n, 0);
      }
      __strip.show();
      _Startzeit = 0;
    }

    void Tick() {
      // Sollte ein Sonnenaufgang laufen, das Licht entsprechend anpassen..
      if (_Startzeit > 0) {
        long _ms = millis() - _Startzeit;
        if (_ms > _Startzeit + _Dauer + _Nachlaufzeit) {
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

void setup() {
  Serial.begin(115200);
  Serial.print("Starte...");

  __strip.begin();
  __strip.setBrightness(255);
  __strip.show(); // Initialiere alle auf "Aus"
  Serial.println(" ok");
}

Sonnenaufgang __SA;

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
  /*  float _t_l =  (millis() % int(DAUER * 1000 * 2));
    Serial.print(" millis%=");  Serial.print (_t_l);
    _t_l -= DAUER * 1000 / 2;
    Serial.print(" _t_l=");  Serial.println(_t_l / 1000);
    for (uint16_t _n = 0; _n < __strip.numPixels(); _n++) {
      float _x = (float)_n * LAENGE / __strip.numPixels();
      __strip.setPixelColor(_n, Lichtfarbe(_t_l / 1000., _x));
    }
    __strip.show();*/
  delay(20);
}

