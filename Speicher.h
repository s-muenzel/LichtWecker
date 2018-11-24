#ifndef _SPEICHER
#define _SPEICHER

class Speicher {
  public:
    Speicher();

    void Beginn();

    time_t Weckzeit(int Tag);
    bool Wecker_An(int Tag);
    void setze_Weckzeit(int Tag, time_t Zeit, bool An);

    void Wecker_Aktiv(bool aktiv);
    bool Wecker_Aktiv();

    void speichern();

    bool jetztWecken(time_t Jetzt);

    float lese_SA_laenge();
    float lese_SA_v();
    float lese_SA_dauer();
    float lese_SA_nachleuchten();
    float lese_SA_snooze();
    void setze_SA_laenge(float f);
    void setze_SA_v(float f);
    void setze_SA_dauer(float f);
    void setze_SA_nachleuchten(float f);
    void setze_SA_snooze(float f);

  private:
    time_t _WZ[7];	// für jeden Wochentag (erstmal egal)
    bool _An[7];			// Wecker ebenfalls pro Tag ein/ausschaltbar (erstmal egal)
    bool _Aktiv;
    float _konfig_laenge;
    float _konfig_v;
    float _konfig_dauer;
    float _konfig_nachleuchten;
    float _konfig_snooze;

};

#endif // _SPEICHER
