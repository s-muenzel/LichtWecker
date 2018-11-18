
// Globale Variablen starten immer mit __[a-z]
// lokale Variable starten mit _[a-z]
// Argumente starten mit [a-z]

#include <Adafruit_NeoPixel.h>

// an welchem PIN hängt die Lichterkette
#define PIN 6
// Meine LED-Kette ist 1m mit 30 LEDs
//#define NUM_LEDS 30
#define NUM_LEDS 6
#define DAUER 20.0          // [s] wie lange dauert der "Sonnenaufgang"
#define LAENGE 1.0          // [m] Länge des LED-Strips
#define GESCHWINDIGKEIT 0.5 // [m/s] Ausbreitungsgeschwindigkeit v 

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

// Für einen "Sonnenaufgang" muss die Lichtfarbe von rötlich nach weiss oder sogar leicht bläulich gehen
uint32_t Lichtfarbe(uint8_t w) {
  float _h = 10. + ( w * 50. / 255.);
  float _s = (255. - w / 2.) / 255. ;
  float _v = w / 255.;
  uint8_t _r;
  uint8_t _g;
  uint8_t _b;
  HSV_to_RGB(_h, _s, _v, &_r, &_g, &_b);
  Serial.print("w="); Serial.print(w);
  Serial.print("      h/s/v=");
  Serial.print(_h); Serial.print("/");
  Serial.print(_s); Serial.print("/");
  Serial.print(_v);
  Serial.print(" r/g/b=");
  Serial.print(_r); Serial.print("/");
  Serial.print(_g); Serial.print("/");
  Serial.print(_b); Serial.println("");
  return __strip.Color(_r, _g, _b);
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
  _t_x /= DAUER; //map(_t_x,0.,DAUER,0.,1.);

  float _h = 10. +  _t_x * 50. ;
  float _s = 1. - _t_x / 2. ;
  float _v = _t_x ;
  uint8_t _r;
  uint8_t _g;
  uint8_t _b;
  HSV_to_RGB(_h, _s, _v, &_r, &_g, &_b);
//  Serial.print(" _t_x="); Serial.print(_t_x);
//  Serial.print(" h/s/v=");
//  Serial.print(_h); Serial.print("/");
//  Serial.print(_s); Serial.print("/");
//  Serial.print(_v);
//  Serial.print(" r/g/b=");
//  Serial.print(_r); Serial.print("/");
//  Serial.print(_g); Serial.print("/");
//  Serial.print(_b); Serial.println("");
  return __strip.Color(_r, _g, _b);

}

void setup() {
  Serial.begin(115200);
  Serial.print("Starte...");

  __strip.begin();
  __strip.setBrightness(255);
  __strip.show(); // Initialiere alle auf "Aus"
  Serial.println(" ok");
}

uint16_t __i = 0;

void loop() {
  __i++;
/*  if (__i == 256 + __strip.numPixels()) {
    Serial.println("Zyklus komplett, reset");
    delay(500);
    for (uint16_t _n = 0; _n < __strip.numPixels(); _n++) {
      __strip.setPixelColor(_n, 0);
    }
    __strip.show();
    __i = 0;
  }
  Serial.print("__i=");  Serial.println(__i);*/
  //  __i %= 256;
  float _t_l =  (millis() % int(DAUER*1000*2));
  Serial.print(" millis%=");  Serial.print (_t_l);
  _t_l -= DAUER*1000 / 2;
  Serial.print(" _t_l=");  Serial.println(_t_l/1000);
  for (uint16_t _n = 0; _n < __strip.numPixels(); _n++) {
    float _x = (float)_n * LAENGE / __strip.numPixels(); 
    __strip.setPixelColor(_n, Lichtfarbe(_t_l/1000., _x));
  }
  /*  uint32_t c = Lichtfarbe((float)__i/255.*DAUER, 0.);
    //  c = Lichtfarbe(min(__i, 255));

    /*  for (uint16_t _n = 0; _n < __strip.numPixels(); _n++) {
        __strip.setPixelColor(_n, c);
      }

    for (uint8_t _n = __strip.numPixels() - 1; _n > 0 ; _n--) {
      __strip.setPixelColor(_n, __strip.getPixelColor(_n - 1));
    }
    __strip.setPixelColor(0, c);
  */
  __strip.show();
  delay(20);
}

