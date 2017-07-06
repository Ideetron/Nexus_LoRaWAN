/******************************************************************************************
* Copyright 2017 Ideetron B.V.
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************************/
/****************************************************************************************
* File:     LoRaWAN.ino
* Author:   Gerben den Hartog
* Compagny: Ideetron B.V.
* Website:  http://www.ideetron.nl/LoRa
* E-mail:   info@ideetron.nl
****************************************************************************************/
/****************************************************************************************
* Created on:         06-01-2017
* Supported Hardware: ID150119-02 Nexus board with RFM95
*
* Description
*
* This firmware demonstrates a LoRaWAN mote.
* It is written specificly for a Nexus board that is connected to PC running a terminal application.
* There are a few commands to make settings.
*
*This fimrware supports
*Over The Air Activation
*Activation By Personalization
*Class switching between Class A and Class C motes
*Channel hopping
*
*The following settings kan be done
*Channel Receive and Transmit
*Datarate Receive and Transmit
*Tranmit power
*Confirmed or unconfirmed messages
*Device Addres
*Application Session Key
*Network Session Key
*Device EUI
*Application EUI
*Application key
*Mote Class
****************************************************************************************/

/*
*****************************************************************************************
* INCLUDE FILES
*****************************************************************************************
*/
#include <SPI.h>
#include "AES-128.h"
#include "Encrypt.h"
#include "Nexus_LoRaWAN.h"
#include "RFM95.h"
#include "LoRaMAC.h"
#include "Waitloop.h"
#include "Commands.h"
#include "DS2401.h"
#include "Struct.h"

#include <EEPROM.h>


long initmillis = 0;//
byte configcycle = 0;
/*
*****************************************************************************************
* GLOBAL VARIABLES
*****************************************************************************************
*/
unsigned char NwkSkey[16] = {
  0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6,
  0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C
};

unsigned char AppSkey[16] = {
  0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6,
  0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C
};

void configuration();
uart_t        UART_Status         = NO_UART_DATA;
RFM_command_t RFM_Command_Status  = NO_RFM_COMMAND;
rx_t          Rx_Status           = NO_RX;

//Initialise session data struct
  unsigned char NwkSKey[16] = {
      0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6,
      0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C
  };
  unsigned char AppSKey[16] = {
      0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6,
      0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C
  };
  unsigned int Frame_Counter_Tx = 0x0000;
  unsigned char Address_Tx[4] = {0x00, 0x00, 0x00, 0x00};
  //EEPROMER(Address_Tx, 4, 'r', 0);
  
  
  sLoRa_Session Session_Data = {NwkSKey, AppSKey, Address_Tx, &Frame_Counter_Tx};

  //Initialize OTAA data struct
  unsigned char DevEUI[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  
  unsigned char AppEUI[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  
  unsigned char AppKey[16] = {
      0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6,
      0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C
  };
 
  unsigned char DevNonce[2] = {0x00, 0x00};
  unsigned char AppNonce[3] = {0x00, 0x00, 0x00};
  unsigned char NetID[3] = {0x00, 0x00, 0x00};

  sLoRa_OTAA OTAA_Data = {DevEUI, AppEUI, AppKey, DevNonce, AppNonce, NetID};

  //Initialise LoRA settings struct
  
  unsigned char Datarate_Tx = 0x00; //set to SF12 BW 125 kHz
  unsigned char Datarate_Rx = 0x03; //set to SF9 BW 125 kHz
  unsigned char Channel_Tx = 0x00; //set to 868.100 MHz
  unsigned char Channel_Rx = 0x10; //set tot 869.525 MHz

  sSettings LoRa_Settings;

  //Initialise buffer for data to transmit
  unsigned char Data_Tx[64];
  sBuffer Buffer_Tx = {Data_Tx, 0x00};

  //Initialise buffer for data to receive
  unsigned char Data_Rx[64];
  sBuffer Buffer_Rx = {Data_Rx, 0x00};

  //Initialise UART receive buffer
  unsigned int UART_Timer = 0;
  unsigned char UART_Data[111];
  sBuffer UART_Rx_Buffer = { UART_Data, 0x00 };

  //Initialise Receive message
  sLoRa_Message Message_Rx;


  byte count_enable = false;
  byte Tx_done = true;

  #include "config.h"
#include "loader.h"
  

void setup()
{
   //Initialize the UART on 9600 baud 8N1
  Serial.begin(9600);
  /**/
  //Initialise the SPI port
  SPI.begin();
  SPI.beginTransaction(SPISettings(4000000,MSBFIRST,SPI_MODE0));

  //Initialize I/O pins
  pinMode(DS2401,OUTPUT);
  pinMode(MFP,INPUT);
  pinMode(DIO0,INPUT);
  pinMode(DIO1,INPUT);
  pinMode(DIO5,INPUT);
  pinMode(DIO2,INPUT);
  pinMode(CS,OUTPUT);
  pinMode(LED,OUTPUT);

  digitalWrite(DS2401,HIGH);
  digitalWrite(CS,HIGH);

  //Initialise timer 2 for 1ms waitloops
  WaitLoop_Init();

  //Wait until RFM module is started
  WaitLoop(20);
      /**/

   #ifdef POWERMAN
    powermanSetup();
  #endif
  #ifdef doorSensor
    doorSensorSetup();
  #endif
  #ifdef PULSECOUNTER
    pulseCounterSetup();
  #endif
  #ifdef RTC
    RTCsetup();
  #endif
  #ifdef LIGHTSENSORS
    lightSetup();
  #endif

  readsettings();
  Message_Rx.Direction = 0x01; //Set down direction for Rx message

  //Initialise DS2401 bytes
  //unsigned char DS_Bytes[8];
  //unsigned char DS_Status = 0x00;

  //
  //Initialize RFM module
  RFM_Init();
  
  initmillis = millis();
  //give time(3 secs) for configuration to be read and written. Values chosen empirically
  while(millis() - initmillis < 3000)configuration();
  //if device is paused for configuration
  while(configcycle == true)configuration();
  transmitdata();// transmit data after atleast 3 secs
  count_enable = true;      //enable interrupts at this point. Using this because cli() sei() are still failing.
  
}

void configuration()
{
  //Raise timers on the hearthbeat of 1 ms
    //Check for compare flag of timer 2
    if((TIFR2 & 0x02) == 0x02)
    {
      //Raise UART timer if ther is a new message
      if(UART_Status == NEW_UART_DATA)
      {
        UART_Timer++;
      }

      //Clear Timer 2 and compare flag
      TCNT2 = 0x00;
      TIFR2 = 0x02;

    }

    //Check for Serail data
    if(Serial.available() != 0)
    {
      if(UART_Rx_Buffer.Counter < 111)
      {
        //Get data
        UART_Data[UART_Rx_Buffer.Counter] = Serial.read();

        //Raise counter
        UART_Rx_Buffer.Counter++;

        //Check if it is a new message
        if(UART_Rx_Buffer.Counter == 0x01)
        {
          //Reset timer
          UART_Timer = 0;

          //Change status flag
          UART_Status = NEW_UART_DATA;

          //Clear Timer 2 and compare flag
          TCNT2 = 0x00;
          TIFR2 = 0x02;
        }
      }
      else
      {
        UART_Status = UART_DATA_DONE;
      }
    }

    //Check if ther is data and timer has run out
    if(UART_Status == NEW_UART_DATA && UART_Timer >= 250)
    {
      byte datalen;
      int eepromaddr;
      char tmp = UART_Data[0];
      char whichdevice = UART_Data[1];
      /*
       * UART_Data[0]
       * 0: mac join
       * 1: Mac data
       * 2: reset
       * 3: mac set/get devaddr
       */
      switch(UART_Data[0])
      {
        case '0':
          Serial.println("mac join");
          //Check if there is no command pending
          if(RFM_Command_Status == NO_RFM_COMMAND)
          {
            Serial.write("Join");
            
            UART_Send_Newline();
            
            //Set join command
            RFM_Command_Status = JOIN;
          }
          break;
        case '1':
          Serial.println("Mac data");
          //Check if there is no command pending
          if(RFM_Command_Status == NO_RFM_COMMAND)
          {
            //Set new command for RFM
            RFM_Command_Status = NEW_RFM_COMMAND;

            Mac_Data(&UART_Rx_Buffer, &Buffer_Tx);
          }
          break;
         case '2':
              //reset
          break;
         case '3':
           //mac set/get devaddr command
          datalen = 4;
          eepromaddr = 0;
          break;
         case '4':
         //mac set/get nwkskey
          datalen = 0x10;
          eepromaddr = 4;
          break; 
         case '5':
         //mac set/get appskey
          datalen = 0x10;
          eepromaddr = 20;
          break;
         case '6':
         //mac set/get appkey
          datalen = 0x10;
          eepromaddr = 36;
          break;
         case '7':
         //mac set/get appeui
          datalen = 0x08;
          eepromaddr = 52;
          break;
         case '8':
         //mac set/get deveui
          datalen = 0x08;
          eepromaddr = 60;
          break;
         /*case '9':
         //mac set/get drrx
          datalen = 0x01;
          eepromaddr = 61;
          break;
         case 'a':
         //mac set/get drtx
          datalen = 0x01;
          eepromaddr = 62;
          break;
         case 'b':
         //mac set/get chtx
          datalen = 0x01;
          eepromaddr = 63;
          break;
         case 'c':
         //mac set/get chrx
          datalen = 0x01;
          eepromaddr = 64;
          break;
         case 'd':
         //mac set/get pwridx
          datalen = 0x01;
          eepromaddr = 65;
          //Mac_Power(&UART_Rx_Buffer, &LoRa_Settings.Transmit_Power);
          break;
         case 'e':
         //mac set/get cnf
          datalen = 0x01;
          eepromaddr = 66;
          break;
         case 'f':
         //mac set/get chhop
          datalen = 0x01;
          eepromaddr = 67;
          break;
         case 'g':
         //mac set/get class
          datalen = 0x01;
          eepromaddr = 68;
          break;
         case 'h':
         //mac set/get class
          datalen = 0x01;
          eepromaddr = 68;
          break;
         case 'i':
         //mac set/get class
          datalen = 0x01;
          eepromaddr = 68;
          break;
         case 'j':
         //mac set/get class
          datalen = 0x01;
          eepromaddr = 68;
          break;
         case 'k':
         //mac set/get class
          datalen = 0x01;
          eepromaddr = 68;
          break;
         case 'l':
         //mac set/get class
          datalen = 0x01;
          eepromaddr = 68;
          break;
          */
         case 'a':
         case 'b':
         case 'c':
         case 'd':
         case 'e':
         case 'f':
         case 'g':
         case 'h'://68
         case 'i':
         //case 'j':
         //case 'k':
         //case 'l':
         //case 'm':
          datalen = 0x01;
          eepromaddr = 61 + (tmp - 'a');
          break;
         case 'j'://transmit interval
         case 'k'://firmware version
          datalen = 0x04;
          eepromaddr = 61 + (tmp - 'a');
          break;
         
          
          
      }
      Store_Config(&UART_Rx_Buffer, UART_Data, datalen, eepromaddr);
      switch(tmp)
      {
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
          //Reset frame counter
          Frame_Counter_Tx = 0x0000;
          
      }
      switch(tmp)
      {
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case 'a':
        case 'b':
        case 'c':
        case 'd':
        case 'f':
        case 'g':
        case 'h':
          RFM_Command_Status = NO_RFM_COMMAND;
          
      }
      switch(tmp)
      {
        case 'e':
          EEPROMER(UART_Data, 0x01,'r', 65+(whichdevice=='0'?0:69));
          LoRa_Settings.Transmit_Power = UART_Data[0];
          break;
        case 'h':
          EEPROMER(UART_Data, 0x01,'r', 68+(whichdevice=='0'?0:69));
          LoRa_Settings.Mote_Class = UART_Data[0];
          if(LoRa_Settings.Mote_Class == 0x00)
          {
            //Switch RFM to standby
            RFM_Switch_Mode(0x01);
            
            Serial.write("A");
          }
          else
          {
            //Switch RFM to continuou receive
            RFM_Continuous_Receive(&LoRa_Settings);
            
            Serial.write("C");
          }
          UART_Send_Newline();
           break;
          //
         case 'z':
          Serial.println("configuring");
          if(UART_Data[4] == '0')
          {
            configcycle = false;
            Serial.println("CCOFF");//config cycle on
          }
          else 
          {
            configcycle = true;
            Serial.println("CCON");//config cycle off
          }
          break;
          
      }
      //Set UART status to data done to clear for next command
      UART_Status = UART_DATA_DONE;
    }

    //Clear uart to get ready for new command
    if(UART_Status == UART_DATA_DONE)
    {
      //Reset timer
      UART_Timer = 0;

      //Set status to no data
      UART_Status = NO_UART_DATA;

      //Reset number of bytes
      UART_Rx_Buffer.Counter = 0;

      //Clear UART
      while(Serial.available() != 0)
      {
        UART_Data[0] = Serial.read();
      }
    }
}

void readsettings()
{
  //read from EEPROM
  //LoRa_Settings.Mote_Class = 0x00; //0x00 is type A, 0x01 is type C
  EEPROMER(&LoRa_Settings.Mote_Class, 0x01, 'r', 68);
 // LoRa_Settings.Datarate_Rx = 0x03;
  EEPROMER( &LoRa_Settings.Datarate_Rx, 0x01, 'r', 62);
 // LoRa_Settings.Channel_Rx = 0x10;
  EEPROMER( &LoRa_Settings.Channel_Rx, 0x01, 'r', 64);
  //LoRa_Settings.Datarate_Tx = 0x00;
  EEPROMER( &LoRa_Settings.Datarate_Tx, 0x01, 'r', 61);
 // LoRa_Settings.Channel_Tx = 0x00;
  EEPROMER( &LoRa_Settings.Channel_Tx, 0x01, 'r', 63);

  //LoRa_Settings.Confirm = 0x00; //0x00 unconfirmed, 0x01 confirmed
  EEPROMER( &LoRa_Settings.Confirm, 0x01, 'r', 66);
 // LoRa_Settings.Channel_Hopping = 0x00; //0x00 no channel hopping, 0x01 channel hopping
  EEPROMER( &LoRa_Settings.Channel_Tx, 0x01, 'r', 67); 
}





void loop()
{
 //go to sleep
 #ifndef TESTING
  powerthisDownNow();
 #endif
 #ifdef TESTING
  Serial.println("in ere..");
  Serial.println(Tx_done);
  delay(10000);
  transmitdata();
 #endif
}

