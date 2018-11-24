#ifndef _SONNENAUFGANG
#define _SONNENAUFGANG

// an welchem PIN hängt die Lichterkette
//#define PIN 6
#define PIN 14 // Sonoff GPIO 14

// Meine LED-Kette ist 1m mit 30 LEDs
#define NUM_LEDS 60
#define LAENGE 1.0f          // [m] Länge des LED-Strips
#define GESCHWINDIGKEIT 0.2f // [m/s] Ausbreitungsgeschwindigkeit v 
#define DAUER 20.0f          // [s] wie lange dauert der "Sonnenaufgang"
#define NACHLEUCHTEN 2.0f    // [s] wie lange bleibt das Licht nach dem "Sonnenaufgang" an
#define SNOOZE  5.f          // [s] wie lange dauert ein Snooze (Pause, nach einen Snooze-Call)
#define BLINKDAUER  0.5f     // [s] wie lange dauert ein Blink (Nachricht)

class Sonnenaufgang {
  public:
    Sonnenaufgang();

    void Beginn();

    void Setze_Laenge(float l) {
      _konfig_laenge = l;
    }
    void Setze_v(float v) {
      _konfig_v = v;
    }
    void Setze_Dauer(float d) {
      _konfig_dauer = d;
    }
    void Setze_Nachleuchten(float n) {
      _konfig_nachleuchten = n;
    }
    void Setze_Snooze(float s) {
      _konfig_snooze = s;
    }

    void Start();
    bool Snooze();
    void Stop();

    typedef enum _Farb_t {
      gruen,
      gelb,
      rot
    } Farb_t;

    typedef enum _Dauer_t {
      kurz,
      lang
    } Dauer_t;

    void Nachricht(Farb_t farbe, Dauer_t dauer);

    bool Laeuft();

    void Tick();

  private:
    uint32_t Lichtfarbe(float t, float x);

    void Tick_Aufgang(long ms);
    void Tick_Nachricht(long ms);

    enum Modus_t {
      aufgang,
      nachricht
    } _Modus;
    Farb_t _Farbe;

    long _Startzeit;    // [ms] - läuft, bzw. seit wann läuft ein S.A.
    long _Nachlaufzeit; // [ms] - wie lange bleibt das Licht nach der Dauer des Sonnenuntergangs an?
    long _Dauer;        // [ms] - wie lange dauert es, bis bei einem S.A. das Licht auf MAX ist

    float _konfig_laenge;
    float _konfig_v;
    float _konfig_dauer;
    float _konfig_nachleuchten;
    float _konfig_snooze;
};

#endif // _SONNENAUFGANG

