#include "Sonnenaufgang.h"

#include <Arduino.h>
#include <NeoPixelAnimator.h>
#include <NeoPixelBus.h>

#include "Speicher.h"
#include "main.h"

// an welchem PIN hängt die Lichterkette
// #define KETTE_PIN 14  // Sonoff Basic: GPIO 14
#define KETTE_PIN 2    // Sonoff Basic von ITEAD, V2: GPIO 2, ESP01(S): GPIO 2
#define RELAIS_PIN 12  // Sonoff Basic: GPIO 12

// Meine LED-Kette ist 1m mit 30 LEDs
#define NUM_LEDS 30
// Gute Default-Werte
// #define LAENGE 1.0f           // [m] Länge des LED-Strips
// #define GESCHWINDIGKEIT 0.1f  // [m/s] Ausbreitungsgeschwindigkeit v
// #define DAUER 20.0f           // [s] wie lange dauert der "Sonnenaufgang"
// #define NACHLEUCHTEN 10.0f    // [s] Nach Max. weiterleuchten
// #define SNOOZE 30.f           // [s] wie lange pausiert  ein Snooze
// #define RELAIS 100            // [ms] Zeit bis Netzteil sicher 5V liefert
#define BLINKDAUER 0.5f  // [s] wie lange dauert ein Blink (Nachricht)

// Uart1: GPIO 2 (TX von Serial1)
NeoPixelBus<NeoGrbFeature, NeoEsp8266Uart1800KbpsMethod> __strip(NUM_LEDS,
                                                                 KETTE_PIN);
// Uart0: GPIO 3 (RX von Serial0)
// NeoPixelBus<NeoGrbFeature, NeoEsp8266Uart0800KbpsMethod> __strip(NUM_LEDS,
//                                                                  KETTE_PIN);

RgbColor Sonnenaufgang::Lichtfarbe(float t, float x) {
  // Kurvenverlauf für Lichtfarbe(t-x/v):
  //        ____
  //       /
  //      /
  //  ___/
  //
  // x ausserhalb der Wert macht keine Sinn
  float _x = max(0.0f, min(_speicher->SA_laenge(), x));
  // wenn von der Mitte angefangen werden soll
  /*
    float _x_prime = (2.0 * _x - _konfig_laenge) / _konfig_v;
    if (_x_prime < 0)
      _x_prime = -_x_prime;*/
  // Wenn vom Anfang angefangen werden soll
  /*
    float _x_prime = _x / _konfig_v; */
  // Wenn vom Ende angefangen werden soll
  float _x_prime = (_speicher->SA_laenge() - _x) / _speicher->SA_v();
  float _t_x = t - _x_prime;
  _t_x = max(0.0f, min(_speicher->SA_dauer(), _t_x));
  _t_x /= _speicher->SA_dauer();

  float _h;
  if (_t_x < 0.9)
    _h = 10. / 360. + _t_x * 50. / 360.;
  else
    _h = -1610. / 360. + _t_x * 1850. / 360.;
  float _s;
  if (_t_x < 0.9)
    _s = 1. - _t_x / 2.;
  else
    _s = 0.55 + 4 * 0.9 - _t_x * 4;
  float _v = _t_x;
  return HsbColor(_h, _s, _v);
}

Sonnenaufgang::Sonnenaufgang() {
#ifdef IST_SONOFF
  pinMode(RELAIS_PIN, OUTPUT);
#endif  // IST_SONOFF
  _modus = nix;
  _startzeit = 0;
}

void Sonnenaufgang::Beginn(Speicher *sp) {
  _speicher = sp;
  __strip.Begin();
  __strip.Show();  // Initialiere alle auf "Aus"

#ifdef IST_SONOFF
  D_PRINTF(LOG_DEBUG, "RELAIS AUS");
  digitalWrite(RELAIS_PIN, LOW);
  _status_relais = false;
#endif  // IST_SONOFF
}

bool Sonnenaufgang::Snooze() {
  if (_startzeit == 0)
    return false;  // es läuft kein Aufgang, als kein Snooze..
  // Snooze bricht den Vorgang nicht ab sondern schiebt ihn nach vorne.
  // Dann ist "Lichtfarbe" erstmal "0,0,0"
  _startzeit = millis() + round(_speicher->SA_snooze() * 1000);
  return true;
}

void Sonnenaufgang::Stop() {
  D_PRINTF(LOG_DEBUG, "Stoppe Sonnenaufgang bei %lu (nach %ld)", millis(),
           (long)(millis() - _startzeit));
  // Stop löscht das Licht und setzt _startzeit wieder auf 0
  for (uint16_t _n = 0; _n < __strip.PixelCount(); _n++) {
    __strip.SetPixelColor(_n, RgbColor(0));
  }
  __strip.Show();
  _startzeit = 0;
  _modus = nix;
  digitalWrite(LED_BUILTIN, LED_AUS);  // bei Sonoff Basic HIGH = OFF
#ifdef IST_SONOFF
  digitalWrite(RELAIS_PIN, LOW);  // Relais aus
  _status_relais = false;
#endif  // IST_SONOFF
}

void Sonnenaufgang::Nachricht(Modus_t modus) {
  // immer zuerst sicherstellen, dass das Relais an ist
#ifdef IST_SONOFF
  if (!_status_relais) {
    D_PRINTF(LOG_DEBUG, "RELAIS AN");
    digitalWrite(RELAIS_PIN, HIGH);
    delay(_speicher->SA_relais());  // mal kurz warten, damit das Relais auch
                                    // sicher angezogen hat
    _status_relais = true;
  }
#endif  // IST_SONOFF
  _modus = modus;
  _startzeit = millis();
  switch (_modus) {
    case aufgang:
      _nachlaufzeit = round(_speicher->SA_nachleuchten() * 1000);
      _dauer = round(_speicher->SA_dauer() * 1000);
      break;
    case weckzeit:
    case weck_abbruch:
    case wecker_aus:
    case wecker_an:
    case ota_ein:
      _nachlaufzeit = 0;
      _dauer = round(3 * BLINKDAUER * 1000);
      break;
    default:
      _dauer = 0;
      _nachlaufzeit = 0;
      Stop();
      break;
  }
#ifndef IST_ESP01
  digitalWrite(LED_BUILTIN, LED_AN);  // bei Sonoff Basic HIGH = OFF
#endif                                // IST_ESP01
  D_PRINTF(LOG_DEBUG, "Starte Nachricht Typ %d bei %ld", _modus, _startzeit);
}

bool Sonnenaufgang::Laeuft() { return _startzeit > 0; }

void Sonnenaufgang::Tick() {
  // Sollte ein Sonnenaufgang laufen, das Licht entsprechend anpassen..
  if (_startzeit > 0) {
    long _ms = millis() - _startzeit;
    switch (_modus) {
      case aufgang:
        Tick_Aufgang(_ms);
        break;
      case weckzeit:
        Tick_Weckzeit(_ms);
        break;
      case weck_abbruch:
        Tick_Farbe(_ms, RgbColor(255, 255, 0));
        break;
      case wecker_aus:
        Tick_Farbe(_ms, RgbColor(255, 0, 0));
        break;
      case wecker_an:
        Tick_Farbe(_ms, RgbColor(0, 255, 0));
        break;
      case ota_ein:
        Tick_Farbe(_ms, RgbColor(255, 0, 255));
        break;

      default:
        break;
    }
  } else {
#ifdef IST_SONOFF
    if (_status_relais) {
      D_PRINTF(LOG_DEBUG, "RELAIS AUS");
      digitalWrite(RELAIS_PIN, LOW);
      _status_relais = false;
    }
#endif  // IST_SONOFF
  }
}

void Sonnenaufgang::Tick_Aufgang(long ms) {
  if (ms > _dauer + _nachlaufzeit) {
    Stop();
  } else {
    for (uint16_t _n = 0; _n < __strip.PixelCount(); _n++) {
      float _x = (float)_n * _speicher->SA_laenge() / __strip.PixelCount();
      __strip.SetPixelColor(_n, Lichtfarbe(ms / 1000., _x));
    }
    __strip.Show();
  }
}

void Sonnenaufgang::Tick_Weckzeit(long ms) {
  long dauer = round(3 * BLINKDAUER * 1000);
  if (ms > 2 * dauer) {
    Stop();
    return;
  }

  uint16_t numP = __strip.PixelCount();
  RgbColor _f;

  if (!_speicher->Wecker_Aktiv()) {  // Wecker Daueraus - lang rot
    D_PRINTF(LOG_DEBUG, "Wecker komplett aus, rot");
    _f = RgbColor(255, 0, 0);
    for (uint16_t _n = 0; _n < numP; _n++) {
      __strip.SetPixelColor(_n, _f);
    }
    __strip.Show();
    return;
  } else {
    time_t weckzeit = _speicher->Naechste_Weckzeit();

    if (weckzeit == (time_t)0) {
      D_PRINTF(LOG_DEBUG, "keine naechste Weckzeit, rot/gelb");
      for (uint16_t _n = 0; _n < numP; _n++) {
        _f = RgbColor(255, (_n % 2) * 255, 0);
        __strip.SetPixelColor(_n, _f);
      }
      __strip.Show();
    } else {
      // Zuerst x Sekunden Stunden anzeigen
      uint8_t h = hour(weckzeit);
      uint8_t m = minute(weckzeit);
      D_PRINTF(LOG_DEBUG, "Weckzeit %d:%02d", h, m);
      if (ms < dauer) {
        uint16_t numP = __strip.PixelCount();
        uint16_t _n;
        for (_n = 0; _n < h; _n++) {
          _f = RgbColor(200, 200, (_n % 6) == 5 ? 100 : 0);
          __strip.SetPixelColor(_n, _f);
        }
        _f = RgbColor(2, 2, 0);
        for (; _n < 24; _n++) {
          __strip.SetPixelColor(_n, _f);
        }
        for (; _n < numP; _n++) {
          __strip.SetPixelColor(_n, 0);
        }
        // Dann x Sekunden Minuten anzeigen
      } else if (ms < 2 * dauer) {
        uint16_t numP = __strip.PixelCount();
        uint16_t _n;
        for (_n = 0; _n < m / 5; _n++) {
          _f = RgbColor(200, (_n % 3) == 2 ? 0 : 200, 200);
          __strip.SetPixelColor(_n, _f);
        }
        if (5 * _n < m) {
          _f = RgbColor(200, (m - 5 * _n) % 5 * 30, (m - 5 * _n) % 5 * 30);
          __strip.SetPixelColor(_n, _f);
          _n++;
        }
        _f = RgbColor(0, 2, 2);
        for (; _n < 60 / 5; _n++) {
          __strip.SetPixelColor(_n, _f);
        }
        for (; _n < numP; _n++) {
          __strip.SetPixelColor(_n, 0);
        }
      }
      __strip.Show();
    }
  }
}

void Sonnenaufgang::Tick_Farbe(long ms, RgbColor f) {
  if (ms > _dauer) {
    Stop();
  } else {
    uint8_t numP = __strip.PixelCount();
    for (uint8_t _n = 0; _n < numP; _n++) {
      __strip.SetPixelColor(_n, f);
    }
    __strip.Show();
  }
}