#include <Arduino.h>
#include <Syslog.h>
#include <WiFiUdp.h>

#include "Zugangsinfo.h"
#include "main.h"

#define SYSLOG_SERVER "diskstation.fritz.box"

#define SYSLOG_PORT 514
#define DEVICE_HOSTNAME host_name
#define APP_NAME "lichtwecker"

#ifdef DEBUG_SYSLOG
WiFiUDP udpClient;
Syslog syslog(udpClient, SYSLOG_SERVER, SYSLOG_PORT, DEVICE_HOSTNAME, APP_NAME,
              LOG_USER);
#endif  // DEBUG_SYSLOG

char __Logg_Puffer[LOGG_PUFFER_SIZE][LOGG_PUFFER_LENGTH];
char __Logg_Stelle[LOGG_PUFFER_SIZE][LOGG_STELLE_LENGTH];
uint8_t __Logg_Pri[LOGG_PUFFER_SIZE];
uint32_t __Logg_Timer[LOGG_PUFFER_SIZE];
uint8_t __Logg_Zeile = 0;

void init_log_puffer(unsigned long baud) {
  for (uint8_t f = 0; f < LOGG_PUFFER_SIZE; f++) {
    __Logg_Puffer[f][0] = 0;
    __Logg_Pri[f] = 0;
  }
#ifdef DEBUG_SERIAL
  Serial.begin(baud);
#endif  // DEBUG_SERIAL
#ifdef DEBUG_SYSLOG
  syslog.logMask(LOG_UPTO(LOG_INFO));
#endif  // DEBUG_SYSLOG
}

char* next_log_puffer(uint8_t pri, const char* file, const uint16_t line) {
  __Logg_Zeile = (__Logg_Zeile + 1) % LOGG_PUFFER_SIZE;
  snprintf(__Logg_Stelle[__Logg_Zeile], LOGG_STELLE_LENGTH - 1, "%s:%d", file,
           line);
  __Logg_Pri[__Logg_Zeile] = pri;
  __Logg_Timer[__Logg_Zeile] = millis();
  return __Logg_Puffer[__Logg_Zeile];
}

const char Level[8][6]{"EMERG", "ALERT", "CRIT ", "ERR  ",
                       "WARN ", "NOTE ", "INFO ", "DEBUG"};

char* get_log_puffer(uint8_t zeile) {
  static char log_zeile[LOGG_PUFFER_LENGTH + 120];
  uint8_t i = (__Logg_Zeile + LOGG_PUFFER_SIZE - zeile) % LOGG_PUFFER_SIZE;
  uint32_t delta_t = __Logg_Timer[__Logg_Zeile] - __Logg_Timer[i];
  uint16_t MilSek = delta_t % 1000;
  delta_t /= 1000;
  uint8_t Seks = delta_t % 60;
  uint8_t Mins = (delta_t / 60) % 60;
  uint8_t Stus = delta_t / 60 / 60;
  snprintf(
      log_zeile, LOGG_PUFFER_LENGTH + 120 - 1,
      "<tr><td>%02d:%02d:%02d.%03d</td><td>%s</td><td>%s</td><td>%s</td></tr>",
      Stus, Mins, Seks, MilSek, Level[LOG_PRI(__Logg_Pri[__Logg_Zeile])],
      __Logg_Stelle[__Logg_Zeile], __Logg_Puffer[i]);
  return log_zeile;
}

uint32_t get_log_puffer_h() { return __Logg_Timer[__Logg_Zeile]; }

uint32_t get_log_puffer_t(uint8_t zeile) {
  uint8_t i = (__Logg_Zeile + LOGG_PUFFER_SIZE - zeile) % LOGG_PUFFER_SIZE;
  uint32_t delta_t = __Logg_Timer[__Logg_Zeile] - __Logg_Timer[i];
  return delta_t;  // [mS]
}

uint8_t get_log_puffer_s(uint8_t zeile) {
  uint8_t i = (__Logg_Zeile + LOGG_PUFFER_SIZE - zeile) % LOGG_PUFFER_SIZE;
  return __Logg_Pri[i];
}

const char* get_log_puffer_l(uint8_t zeile) {
  uint8_t i = (__Logg_Zeile + LOGG_PUFFER_SIZE - zeile) % LOGG_PUFFER_SIZE;
  return (const char*)__Logg_Stelle[i];
}

const char* get_log_puffer_m(uint8_t zeile) {
  uint8_t i = (__Logg_Zeile + LOGG_PUFFER_SIZE - zeile) % LOGG_PUFFER_SIZE;
  return (const char*)__Logg_Puffer[i];
}

void print_log_puffer() {
#ifdef DEBUG_SERIAL
  uint32_t delta_t = millis();
  uint16_t MilSek = delta_t % 1000;
  delta_t /= 1000;
  uint8_t Seks = delta_t % 60;
  uint8_t Mins = (delta_t / 60) % 60;
  uint8_t Stus = delta_t / 60 / 60;
  Serial.printf("%02d:%02d:%02d.%03d %s %s %s\n", Stus, Mins, Seks, MilSek,
                Level[LOG_PRI(__Logg_Pri[__Logg_Zeile])],
                __Logg_Stelle[__Logg_Zeile], __Logg_Puffer[__Logg_Zeile]);
#endif  // DEBUG_SERIAL
#ifdef DEBUG_SYSLOG
  syslog.log(__Logg_Pri[__Logg_Zeile], __Logg_Puffer[__Logg_Zeile]);
#endif  // DEBUG_SYSLOG
}
