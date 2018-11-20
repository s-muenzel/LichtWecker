#include "Sonnenaufgang.h"

#include <Adafruit_NeoPixel.h>

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
  float _x_prime = (2.0 * _x - LAENGE) / GESCHWINDIGKEIT;
  if (_x_prime < 0)
    _x_prime = -_x_prime;
  float _t_x = t - _x_prime;
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
  /*  static float _deb_zeit = 0;
    if (t > _deb_zeit) {
      _deb_zeit = t;
      Serial.print(" t="); Serial.print(t);
      Serial.print(" x="); Serial.print(x);
      Serial.print(" h/s/v=");
      Serial.print(_h); Serial.print("/");
      Serial.print(_s); Serial.print("/");
      Serial.print(_v);
      Serial.print(" r/g/b=");
      Serial.print(_r); Serial.print("/");
      Serial.print(_g); Serial.print("/");
      Serial.print(_b); Serial.println("");
    }*/
  return __strip.Color(_r, _g, _b);
}

Sonnenaufgang::Sonnenaufgang() {
}

void Sonnenaufgang::Beginn() {
  __strip.begin();
  __strip.setBrightness(255);
  __strip.show(); // Initialiere alle auf "Aus"
}

void Sonnenaufgang::Start() {
  // Start merkt sich die Startzeit (jetzt).
  // Wenn Startzeit > 0 ist, läuft ein Sonnenaufgang
  // Sollte ein Sonnenaufgang bereits laufen --> von neuem anfangen
  _Nachlaufzeit = round(NACHLEUCHTEN * 1000);
  _Dauer = round(DAUER * 1000);
  _Modus = aufgang;
  _Startzeit = millis();
  digitalWrite(LED_BUILTIN, LOW); // bei Sonoff Basic HIGH = OFF
  Serial.printf("Starte Sonnenaufgang bei %d, Dauer %d Nachlaufzeit %d\n", _Startzeit, _Dauer, _Nachlaufzeit);
}

bool Sonnenaufgang::Snooze() {
  if (_Startzeit == 0)
    return false; // es läuft kein Aufgang, als kein Snooze..
  _Startzeit = millis()+ round(SNOOZE*1000);
  return true;
}

void Sonnenaufgang::Stop() {
  Serial.printf("Stoppe Sonnenaufgang bei %d (nach %d)\n", millis(), millis() - _Startzeit);
  // Stop löscht das Licht und setzt _Startzeit wieder auf 0
  for (uint16_t _n = 0; _n < __strip.numPixels(); _n++) {
    __strip.setPixelColor(_n, 0);
  }
  __strip.show();
  _Startzeit = 0;
  digitalWrite(LED_BUILTIN, HIGH); // bei Sonoff Basic HIGH = OFF
}

void Sonnenaufgang::Nachricht(Farb_t farbe) {
  _Nachlaufzeit = 0;
  _Dauer = round(BLINKDAUER * 1000);
  _Modus = nachricht;
  _Startzeit = millis();
  _Farbe = farbe;
  digitalWrite(LED_BUILTIN, LOW); // bei Sonoff Basic HIGH = OFF
  Serial.printf("Starte Nachricht bei %d, Dauer %d, Farbe #%d\n", _Startzeit, _Dauer, _Farbe);
}
bool Sonnenaufgang::Laeuft() {
  return _Startzeit > 0;
}

void Sonnenaufgang::Tick() {
  // Sollte ein Sonnenaufgang laufen, das Licht entsprechend anpassen..
  if (_Startzeit > 0) {
    long _ms =  millis() - _Startzeit;
    if ( _ms > _Dauer + _Nachlaufzeit) {
      Stop();
    } else {
      switch (_Modus) {
        case aufgang:
          Tick_Aufgang(_ms);
          break;
        case nachricht:
          Tick_Nachricht(_ms);
          break;
      }
    }
  }
}

void Sonnenaufgang::Tick_Aufgang(long ms) {
  for (uint16_t _n = 0; _n < __strip.numPixels(); _n++) {
    float _x = (float)_n * LAENGE / __strip.numPixels();
    __strip.setPixelColor(_n, Lichtfarbe(ms / 1000., _x));
  }
  __strip.show();
}

void Sonnenaufgang::Tick_Nachricht(long ms) {
  uint32_t _f;
  if (ms < 100) {
    switch (_Farbe) {
      case rot:
        _f = __strip.Color(255, 0, 0);
        break;
      case gelb:
        _f = __strip.Color(255, 255, 0);
        break;
      case gruen:
        _f = __strip.Color(0, 255, 0);
        break;
    }
    for (uint16_t _n = 0; _n < __strip.numPixels(); _n++) {
      float _x = (float)_n * LAENGE / __strip.numPixels();
      __strip.setPixelColor(_n, _f);
    }
    __strip.show();
  }
}

