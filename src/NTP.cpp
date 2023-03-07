#include <Arduino.h>

#include "NTP.h"

#include "main.h"

NTP_Helfer::NTP_Helfer() {}

void NTP_Helfer::Beginn() {
  lokale_zeit_.setLocation("Europe/Berlin");
  D_PRINTF(LOG_DEBUG, "NTP Update %s erfolgreich, Zeit: %s",
           waitForSync(10) ? "" : "nicht", lokale_zeit_.dateTime().c_str());
}

void NTP_Helfer::Tick() { events(); }

time_t NTP_Helfer::now() { return lokale_zeit_.now(); }
