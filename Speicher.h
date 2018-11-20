#ifndef _SPEICHER
#define _SPEICHER

// Globale Variablen starten immer mit __[a-z]
// lokale Variable starten mit _[a-z]
// Argumente starten mit [a-z]


class Speicher {
  public:
    Speicher();

    void Beginn();

    time_t Weckzeit(int Tag);
    bool Wecker_An(int Tag);
    void setze_Weckzeit(int Tag, time_t Zeit, bool An);
    void speichern();

    bool jetztWecken(time_t Jetzt);
  private:
    time_t _WZ[7];	// für jeden Wochentag (erstmal egal)
    bool _An[7];			// Wecker ebenfalls pro Tag ein/ausschaltbar (erstmal egal)
};

#endif // _SPEICHER
