#ifndef _KNOPF
#define _KNOPF

class Knopf {
 public:
  Knopf();

  void Beginn();

  typedef enum _Event { nix, kurz, lang } _Event_t;
  _Event_t Status();
  int WieLang();

 private:
  long _start;  // [ms] - wann wurde der Knopf gedrückt
  bool _kurz;   // wurde ein kurz bereits gemeldet?
  int _lang;    // bei _lang wird mitgezählt
};

#endif  // _KNOPF
