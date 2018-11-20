#ifndef _KNOPF
#define _KNOPF

// Globale Variablen starten immer mit __[a-z]
// lokale Variable starten mit _[a-z]
// Argumente starten mit [a-z]

#define KNOPF_PIN 0		// Sonoff Taster
#define LANG	  1500	// [ms] ab welcher Dauer zählt der Tastendruck als "lang"	

class Knopf {
  public:
    Knopf();

    void Beginn();

    typedef enum _Event {
      nix,
      kurz,
      lang
    } _Event_t;

    _Event_t Status();

  private:

    long _Start;		// [ms] - wann wurde der Knopf gedrückt
    bool _Kurz;			// wurde ein kurz bereits gemeldet?
    int  _Lang;			// bei _Lang wird mitgezählt
};

#endif // _KNOPF
