//#include <LowPower.h>
#include <avr/sleep.h>
#include <avr/wdt.h>

bool hz_flag;
int secs = 0;
int nextIntr;
//byte timetotransmit = 1; //true

void powermanSetup()
{
  wdt_disable();
}

void systemSleep()
{
  ADCSRA &= ~_BV(ADEN);         // ADC disabled

  /*
  * Possible sleep modes are (see sleep.h):
  #define SLEEP_MODE_IDLE         (0)
  #define SLEEP_MODE_ADC          _BV(SM0)
  #define SLEEP_MODE_PWR_DOWN     _BV(SM1)
  #define SLEEP_MODE_PWR_SAVE     (_BV(SM0) | _BV(SM1))
  #define SLEEP_MODE_STANDBY      (_BV(SM1) | _BV(SM2))
  #define SLEEP_MODE_EXT_STANDBY  (_BV(SM0) | _BV(SM1) | _BV(SM2))
  */
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);

  /*
   * This code is from the documentation in avr/sleep.h
   */
  cli();
  // Only go to sleep if there was no watchdog interrupt.
  //if (!hz_flag)
 // {
    sleep_enable();
    sei();
    sleep_cpu();
    sleep_disable();
  //}
  sei();

  ADCSRA |= _BV(ADEN);          // ADC enabled
}

#define my_wdt_enable(value)   \
__asm__ __volatile__ (  \
    "in __tmp_reg__,__SREG__" "\n\t"    \
    "cli" "\n\t"    \
    "wdr" "\n\t"    \
    "sts %0,%1" "\n\t"  \
    "out __SREG__,__tmp_reg__" "\n\t"   \
    "sts %0,%2" "\n\t" \
    : /* no outputs */  \
    : "M" (_SFR_MEM_ADDR(_WD_CONTROL_REG)), \
      "r" (_BV(_WD_CHANGE_BIT) | _BV(WDE)), \
      "r" ((uint8_t) (((value & 0x08) ? _WD_PS3_MASK : 0x00) | \
          _BV(WDE) | _BV(WDIE) | (value & 0x07)) ) \
    : "r0"  \
)

void setupWatchdog()
{
  /*
   * If you change this, remember to change secs += 8; in ISR(WDT_vect)
   */
  my_wdt_enable(WDTO_8S);
}

ISR(WDT_vect)
{

 /**/
 cli();
 secs += 8;
 #ifndef RTC
 
  if(secs >= nextIntr)
  {
    secs = 0;
    wdt_disable();
//    timetotransmit = 1;
    transmitdata();
  }else
  {
    
    interrupts();
    wdt_reset();
  }
  #endif
 #ifdef RTC
     interrupts();
    wdt_reset();
    setupWatchdog();
    systemSleep();
   #endif
 
  #ifdef SERIALDEBUG
     // Serial.println("....................ISR(WDT_vect).................");
      Serial.println(secs);
      delay(10);
      //Serial.println(":...ISR(WDT_vect)");
  #endif
  //setupWatchdog();
  
}

void powerthisDownNow()
{
    powerthisDownNow_start:
      interrupts();
      setupWatchdog();
      systemSleep();
    goto powerthisDownNow_start;    //system is only woken up by interrupts.
    /*
     * Try if this setup works this way...........................................ok
     * Set watchdog timer to 8 secs. Does it still work?
     * Try if it works with RTC,,,
     * Add door sensor,
     * Add pulse counter,
     * Set it up and leave for the weekend and observe the battery level.
     * Clean up code,
     * Push 
     * Document...
     */
    
}

void powerthisDown()
{
     powerthisDownNow();
}




