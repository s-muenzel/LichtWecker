#ifndef LOGGING_H
#define LOGGING_H

#include <Syslog.h>

#define LOGG_PUFFER_SIZE 40
#define LOGG_PUFFER_LENGTH 100
#define LOGG_STELLE_LENGTH 40
void init_log_puffer(unsigned long baud);
char* next_log_puffer(uint8_t pri, const char* file, const uint16_t line);
char* get_log_puffer(uint8_t zeile);
uint32_t get_log_puffer_h();
uint32_t get_log_puffer_t(uint8_t zeile);
uint8_t get_log_puffer_s(uint8_t zeile);
const char* get_log_puffer_l(uint8_t zeile);
const char* get_log_puffer_m(uint8_t zeile);
void print_log_puffer();

#define D_BEGIN(speed) \
  { init_log_puffer(speed); }
#define D_PRINTF(x, ...)                                                     \
  {                                                                          \
    snprintf(next_log_puffer(x, __FILE__, __LINE__), LOGG_PUFFER_LENGTH - 1, \
             __VA_ARGS__);                                                   \
    print_log_puffer();                                                      \
  }

#endif  // MAIN_H
