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


class Sonnenaufgang {
  public:
    Sonnenaufgang(float dauer = DAUER, float nachleuchten = NACHLEUCHTEN);

    void Beginn();
    void Start();
    void Stop();

    bool Laeuft();

    void Tick();

  private:
    long _Startzeit;    // [ms] - läuft, bzw. seit wann läuft ein S.A.
    long _Nachlaufzeit; // [ms] - wie lange bleibt das Licht nach der Dauer des Sonnenuntergangs an?
    long _Dauer;        // [ms] - wie lange dauert es, bis bei einem S.A. das Licht auf MAX ist
};

#endif // _SONNENAUFGANG

