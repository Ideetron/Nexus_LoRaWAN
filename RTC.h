//#include <avr/pgmspace.h>
//#include <elapsedMillis.h>
#include <Time.h>
#include <Wire.h>
#include <MCP7940RTC.h>

//#define LED_BUILTIN 9 // mini-wireless board LED pin #
//#define LED_ON digitalWrite(LED_BUILTIN,1)
//#define LED_OFF digitalWrite(LED_BUILTIN,0)

//#define RTC_INTERRUPT_PIN 3                     // arduino hardware interrupt pin, (i.e, D3 for miniWireless)


#define YY 2017  // no need to change these; they're just used for testing. 
#define MO 1  //Note the number sequences for easy reading screens
#define DD 2
#define HH 3
#define MM 4
#define SEC 0

MCP7940RTC *RTCchip; // see the call to new() and the comments above.

//elapsedMillis elapsedms; // relatively new in Arduino

volatile boolean rtcInterruptFlag=0; // volatile:stevech

//void ISRrtc() {  // interrupt service routine, just set flag
//  rtcInterruptFlag = true;
//}


void setAlarmNextSecond(int seconds)  {
  //uint8_t s = RTCchip->getSecond();   //get current second 0..59

  //if (seconds > 1)
  //  while (s == RTCchip->getSecond()) // for the sake of this test program, alarm on start of a seconds
   // {
    //  Serial.println("...");
     // delay(1); // don't pound the RTC with I2C commands
    //}
  RTCchip->setAlarm0(now() + seconds);  // set next alarm time  
}

void setNewTimeRTC(int yr, int mo, int dy, int hr, int mn, int sec) {
  tmElements_t tm1;
  tm1.Year     =(yr-1970);
  tm1.Month     =mo;
  tm1.Day       =dy;
  tm1.Hour      =hr;
  tm1.Minute    =mn;
  tm1.Second    =sec;
  time_t t = makeTime(tm1);
  RTCchip->setTimeRTC(t);
}

void RTCsetup() {
  pinMode(RTC_INTERRUPT_PIN, INPUT_PULLUP);     // INPUT also sufficient, if we have our own on-board discrete pullup
 // pinMode(LED_BUILTIN,  OUTPUT);
 // LED_ON;
  
  RTCchip = new MCP7940RTC();  // see the comment block in setup() on this non-static class
  setNewTimeRTC(YY, MO, DD, HH, MM, SEC);  // Set RTC chip time, (year, month, day, hour, minute, second)
  setTime(HH,MM,SEC,DD,MO,YY);       // Set Arduino library system time (not RTC chip)
  
  delay(250);
 // LED_OFF;
}
