#ifndef _SONNENAUFGANG
#define _SONNENAUFGANG

// Globale Variablen starten immer mit __[a-z]
// lokale Variable starten mit _[a-z]
// Argumente starten mit [a-z]

// an welchem PIN hängt die Lichterkette
//#define PIN 6
#define PIN 14 // Sonoff GPIO 14

// Meine LED-Kette ist 1m mit 30 LEDs
#define NUM_LEDS 60
#define LAENGE 1.0f          // [m] Länge des LED-Strips
#define GESCHWINDIGKEIT 0.2f // [m/s] Ausbreitungsgeschwindigkeit v 
#define DAUER 20.0f          // [s] wie lange dauert der "Sonnenaufgang"
#define NACHLEUCHTEN 2.0f    // [s] wie lange bleibt das Licht nach dem "Sonnenaufgang" an
#define BLINKDAUER	0.5f     // [s] wie lange dauert ein Blink (Nachricht)
#define SNOOZE  5.f          // [s] wie lange dauert ein Snooze (Pause, nach einen Snooze-Call)

class Sonnenaufgang {
  public:
    Sonnenaufgang();

    void Beginn();
    void Start();
    bool Snooze();
    void Stop();

    typedef enum _Farb_t {
      gruen,
      gelb,
      rot
    } Farb_t;

    void Nachricht(Farb_t farbe);

    bool Laeuft();

    void Tick();

  private:

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
};

#endif // _SONNENAUFGANG

