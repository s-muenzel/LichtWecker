#include "Sonnenaufgang.h"

#include <Adafruit_NeoPixel.h>

#ifdef IST_SONOFF
#define LED_AN LOW
#define LED_AUS HIGH
#else
#define LED_AN HIGH
#define LED_AUS LOW
#endif

Adafruit_NeoPixel __strip = Adafruit_NeoPixel(NUM_LEDS, KETTE_PIN, NEO_GRB + NEO_KHZ800);

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

uint32_t Sonnenaufgang::Lichtfarbe(float t, float x) {
  // Kurvenverlauf für Lichtfarbe(t-x/v):
  //        ____
  //       /
  //      /
  //  ___/
  //
  // x ausserhalb der Wert macht keine Sinn
  float _x = max(0.0f, min(_konfig_laenge, x));
  // wenn von der Mitte angefangen werden soll
  /*
    float _x_prime = (2.0 * _x - _konfig_laenge) / _konfig_v;
    if (_x_prime < 0)
      _x_prime = -_x_prime;*/
  // Wenn vom Anfang angefangen werden soll
  /*
    float _x_prime = _x / _konfig_v; */
  // Wenn vom Ende angefangen werden soll
  float _x_prime = (_konfig_laenge - _x) / _konfig_v;
  float _t_x = t - _x_prime;
  _t_x = max(0.0f, min(_konfig_dauer, _t_x));
  _t_x /= _konfig_dauer;

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
  return __strip.Color(_r, _g, _b);
}

Sonnenaufgang::Sonnenaufgang() {
  _konfig_laenge = LAENGE;
  _konfig_v = GESCHWINDIGKEIT;
  _konfig_dauer = DAUER;
  _konfig_nachleuchten = NACHLEUCHTEN;
  _konfig_snooze = SNOOZE;
#ifdef IST_SONOFF
  _konfig_relais = RELAIS;
  pinMode(RELAIS_PIN, OUTPUT);
#endif // IST_SONOFF
}

void Sonnenaufgang::Beginn() {
  __strip.begin();
  __strip.setBrightness(255);
  __strip.show(); // Initialiere alle auf "Aus"

#ifdef IST_SONOFF
  D_PRINTF("RELAIS AUS");
  digitalWrite(RELAIS_PIN, LOW);
  _status_Relais = false;
#endif // IST_SONOFF
}

void Sonnenaufgang::Start() {
  // Start merkt sich die Startzeit (jetzt).
  // Wenn Startzeit > 0 ist, läuft ein Sonnenaufgang
  // Sollte ein Sonnenaufgang bereits laufen --> von neuem anfangen

  // immer zuerst sicherstellen, dass das Relais an ist
#ifdef IST_SONOFF
  if (!_status_Relais) {
    D_PRINTF("RELAIS AN");
    digitalWrite(RELAIS_PIN, HIGH);
    delay(_konfig_relais); // mal kurz warten, damit das Relais auch sicher angezogen hat
    _status_Relais = true;
  }
#endif // IST_SONOFF
  _Nachlaufzeit = round(_konfig_nachleuchten * 1000);
  _Dauer = round(_konfig_dauer * 1000);
  _Modus = aufgang;
  _Startzeit = millis();
  digitalWrite(LED_BUILTIN, LED_AN); // bei Sonoff Basic HIGH = OFF
  D_PRINTF("Starte Sonnenaufgang bei %ld, Dauer %ld Nachlaufzeit %ld\n", _Startzeit, _Dauer, _Nachlaufzeit);
}

bool Sonnenaufgang::Snooze() {
  if (_Startzeit == 0)
    return false; // es läuft kein Aufgang, als kein Snooze..
  _Startzeit = millis() + round(_konfig_snooze * 1000);
  return true;
}

void Sonnenaufgang::Stop() {
  D_PRINTF("Stoppe Sonnenaufgang bei %lu (nach %ld)\n", millis(), (long)(millis() - _Startzeit));
  // Stop löscht das Licht und setzt _Startzeit wieder auf 0
  for (uint16_t _n = 0; _n < __strip.numPixels(); _n++) {
    __strip.setPixelColor(_n, 0);
  }
  __strip.show();
  _Startzeit = 0;
  digitalWrite(LED_BUILTIN, LED_AUS); // bei Sonoff Basic HIGH = OFF
#ifdef IST_SONOFF
  digitalWrite(RELAIS_PIN, LOW); // Relais aus
  _status_Relais = false;
#endif // IST_SONOFF
}

void Sonnenaufgang::Nachricht(Farb_t farbe, Dauer_t dauer, uint8_t prozent) {
  // immer zuerst sicherstellen, dass das Relais an ist
#ifdef IST_SONOFF
  if (!_status_Relais) {
    D_PRINTF("RELAIS AN");
    digitalWrite(RELAIS_PIN, HIGH);
    delay(_konfig_relais); // mal kurz warten, damit das Relais auch sicher angezogen hat
    _status_Relais = true;
  }
#endif // IST_SONOFF
  _Nachlaufzeit = 0;
  switch (dauer) {
    case kurz:
      _Dauer = round(BLINKDAUER * 1000);
      break;
    case lang:
      _Dauer = round(3 * BLINKDAUER * 1000);
    default:
      break;
  }
  _Modus = nachricht;
  _Prozent = prozent;
  _Startzeit = millis();
  _Farbe = farbe;
  digitalWrite(LED_BUILTIN, LED_AN); // bei Sonoff Basic HIGH = OFF
  D_PRINTF("Starte Nachricht bei %ld, Dauer %ld, Farbe #%d\n", _Startzeit, _Dauer, _Farbe);
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
  } else {
#ifdef IST_SONOFF
    if (_status_Relais) {
      D_PRINTF("RELAIS AUS");
      digitalWrite(RELAIS_PIN, LOW);
      _status_Relais = false;
    }
#endif // IST_SONOFF
  }
}

void Sonnenaufgang::Tick_Aufgang(long ms) {
  for (uint16_t _n = 0; _n < __strip.numPixels(); _n++) {
    float _x = (float)_n * _konfig_laenge / __strip.numPixels();
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
      case lila:
        _f = __strip.Color(255, 0, 255);
        break;
      default:
        _f = __strip.Color(255, 255, 255);
        break;
    }
    uint16_t numP = (__strip.numPixels() * _Prozent ) / 100;
    uint16_t _n;
    for (_n = 0; _n < numP; _n++) {
      __strip.setPixelColor(_n, _f);
    }
    for (; _n < __strip.numPixels(); _n++) {
      __strip.setPixelColor(_n, 0);
    }
    __strip.show();
  }
}
