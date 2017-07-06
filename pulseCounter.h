#define pulseCounterPin pulseInterruptPin
//unsigned long int pulses = 0;
//byte waspulse = true;
long startedmillis = 0;
long openingmillis = 0;
byte doortouched = false;
byte doorstatus = 0;
long openmillis = 0;

#define TRANSMITPIN 12
#define DOOR_OR_PULSE A2

#define DOORPIN 2

void pulseCounterDebug()
{
  #ifdef SERIALDEBUG
   // sprintf(buffer, "pulses:%d", pulses);
   // Serial.println(buffer);
    //Serial.print("pulses:"); Serial.println(pulses);
    //Serial.print("A2:"); Serial.println(digitalRead(A2));
    //Serial.print("TRANSMITPIN:"); Serial.println(digitalRead(TRANSMITPIN));
  #endif
}

void pulseCounterISR()
{
  #ifdef TESTING
   // Serial.println("pulse ISR...");
  #endif
  //use RTC to count...
  wdt_disable();
  //if(millis() - startedmillis <50 /*(started == 0*/)
  if(count_enable == false /*(started == 0*/)
    {
      
    }
   else
   {
      #ifdef TESTING
           // Serial.println("pulse ISR ++...");
     #endif
      if(digitalRead(TRANSMITPIN) == true)    //is TX button
      {
        //transmit
//        if(timetotransmit == 0)//no timer ISR
 //         timetotransmit = 2;
          #ifndef TESTING
              transmitdata();
              Frame_Counter_Tx++;         //frame counter not incremented when TX is triggered from here
              Serial.println(Frame_Counter_Tx);
          #endif
          #ifdef TESTING
            Serial.println("is transmit...");
            transmitdata();
            
            Serial.println(Frame_Counter_Tx);
          #endif
      }else
      {
        
        //door or pulse
        //Serial.println(digitalRead(DOOR_OR_PULSE));
       if(digitalRead(DOOR_OR_PULSE) == true) //pulse
       {
          
          #ifdef TESTING
            //Serial.println("is PULSE...");
          #endif
          pulses++;
          //waspulse = true;
       }else  //door........................................................require...............................
       {
            #ifdef TESTING
              Serial.println("is door...");
            #endif
            if(doortouched == false)startedmillis = 0;
            doortouched = true;
            if(startedmillis ==0 )startedmillis = millis();       //when first interrupt was received
            if(millis() - startedmillis < 200);                   //debounce. Value obtained from empirical data
            else
            {
              //after debouncing, we can now get the door status
              doorstatus = digitalRead(DOORPIN);
              Serial.print("doorstatus: ");
              Serial.println(doorstatus);
              if(doorstatus == 1)     //using NC switch and door is NC, doorstatus is 1 when door is opened
              {
                openingmillis = startedmillis;
                startedmillis = 0;
                pulses++;
              }else //if door is closing
              {
                startedmillis = 0 ;
                 unsigned long tmpmillis = millis();
                 openmillis += millis() - openingmillis;
              }
              
            }
       }
        
      }
   }
  
  
}

void setTxInterval()
{
  ///int nextIntr;
  
  EEPROM.get(70+69, nextIntr);
 // cli();
  Serial.print("tx interval: ");
  Serial.println(nextIntr);
  //Serial.println("..");
  #ifdef RTC
  setAlarmNextSecond(nextIntr);
  #endif
  //interrupts();
}

void ISRrtc() {  // interrupt service routine
  wdt_reset();
  cli();
  wdt_disable();
   setTxInterval();
  //timetotransmit = 1;
  #ifdef SERIALDEBUG
  Serial.println("***");
 // Serial.println(nextIntr);
  #endif
}



void pulseCounterSetup()
{
  //pinMode(pulseCounterPin, INPUT_PULLUP);
  EEPROM.get(70+69, nextIntr);
  startedmillis = millis();
  pinMode(pulseCounterPin, INPUT_PULLUP);
  pinMode(DOOR_OR_PULSE, INPUT_PULLUP);
  pinMode(TTN_OR_KCS , INPUT_PULLUP);
  pinMode(TRANSMITPIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(pulseCounterPin), pulseCounterISR, RISING);


  //transmission
  #ifdef RTC
  attachInterrupt(digitalPinToInterrupt(RTC_INTERRUPT_PIN), ISRrtc, FALLING);
   //int nextIntr = (int) TRANSMITINTERVALSECS;
 // Serial.println(nextIntr);
 // setAlarmNextSecond(nextIntr);
  ///int pass;
 // int n;
  //uint32_t millisStart, mark;

  //Serial.println(F("Begin loop()"));
  
  setNewTimeRTC(YY, MO, DD, HH, MM, SEC);  // Set RTC chip time, (year, month, day, hour, minute, second)
  setTime(HH,MM,SEC,DD,MO,YY);       // Set system time (not RTC chip)
  setTxInterval();
  #endif
}





/*
byte istimetoTransmit()
{
  return timetotransmit;
}
*/

byte whichchannel()
{
  return digitalRead(TTN_OR_KCS);
}


/*
void cleartimetotransmit()
{
  #ifdef RTC
  //Serial.println(timetotransmit);
  if(timetotransmit != 2) setTxInterval();
  #endif
  timetotransmit = false;
}
*/

