#ifndef _LICHTWECKER_H
#define _LICHTWECKER_H

//#define DEBUG_SERIAL

//#define IST_SONOFF
#define IST_ESP01


#ifdef IST_SONOFF
# define LED_AN LOW
# define LED_AUS HIGH
#else
#  define LED_AN HIGH
#  define LED_AUS LOW
#endif

#endif // _LICHTWECKER_H
