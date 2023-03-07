#ifndef _LICHTWECKER_H
#define _LICHTWECKER_H

//#define DEBUG_SERIAL
//#define DEBUG_SYSLOG

#define IST_SONOFF
// #define IST_ESP01

#include "logging.h"

#ifdef IST_SONOFF
#define LED_AN LOW
#define LED_AUS HIGH
#else
#ifdef IST_ESP01
#define LED_AN LOW
#define LED_AUS HIGH
#else
#define LED_AN HIGH
#define LED_AUS LOW
#endif
#endif

#endif  // _LICHTWECKER_H
