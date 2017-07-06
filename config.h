//#define SERIALDEBUG

#define pulseInterruptPin 2 //pin3 is available only for the RTC

#define PULSECOUNTER   //#define doorSensor or #define PULSECOUNTER
//#define doorSensor


//#define RTC
#ifdef RTC
  #define RTC_INTERRUPT_PIN 3 
 // #define YY 2017
 // #define MO 06
 // #define DD 21
 // #define HH 12
 // #define MM 00
 // #define SEC 00
#endif


//#include "transmitConfig.h"
#ifdef doorSensor
  #include "doorSensorConfig.h"
#endif

//#define TEMPERATURE

#define POWERMAN

//#define LIGHTSENSORS


#define TRUE 1
#define FALSE 0

//#define E_EPROM   //store config to EEPROM

//#define TESTING



