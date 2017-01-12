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
* Minimal Uplink for LoRaWAN
* 
* This code demonstrates a LoRaWAN connection on a Nexus board. This code sends a messege every minute
* on chanell 0 (868.1 MHz) Spreading factor 7.
* On every message the frame counter is raised
* 
* This code does not include
* Receiving packets and handeling
* Channel switching
* MAC control messages
* Over the Air joining* 
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

void setup() 
{
   //Initialize the UART
  Serial.begin(9600);

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

  WaitLoop_Init();

  //Wait until RFM module is started
  WaitLoop(20);   
}

void loop() 
{
  unsigned char i;
  
  unsigned char DevAddr[4] = {0xAA, 0xBB, 0xCC, 0xDD};

  unsigned char Datarate_Tx = 0x00; //set to SF12 BW 125 kHz
  unsigned char Datarate_Rx = 0x03; //set to SF9 BW 125 kHz
  unsigned char Channel_Tx = 0x00; //set to 868.100 MHz
  unsigned char Channel_Rx = 0x10; //set tot 869.525 MHz

  unsigned char Sleep_Sec = 0x00;
  unsigned char Sleep_Time = 0x01;

  unsigned char Data_Tx[256];
  unsigned char Data_Rx[64];
  unsigned char Data_Length_Tx;
  unsigned char Data_Length_Rx = 0x00;

  unsigned int UART_Timer = 0;
  unsigned char UART_Data[64];
  unsigned char UART_Nb_Bytes = 0;
  uart_t UART_Status = NO_UART_DATA;

  

  //Initialize RFM module
  RFM_Init();

  while(1)
  {
    //Raise timers on the hearthbeat of 1 ms
    //Check for compare flag of tiemr 2
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
      if(UART_Nb_Bytes < 64)
      {
        //Get data
        UART_Data[UART_Nb_Bytes] = Serial.read();

        //Raise counter
        UART_Nb_Bytes++;

        //Check if it is a new message
        if(UART_Nb_Bytes == 0x01)
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
    if(UART_Status == NEW_UART_DATA && UART_Timer >= 200)
    {
      //Check for commands
      //MAC command type
      if(UART_Data[0] == 'm' && UART_Data[1] == 'a' && UART_Data[2] == 'c')
      {
        //Check for a set or get command
        if((UART_Data[4] == 's' || UART_Data[4] == 'g') && UART_Data[5] == 'e' && UART_Data[6] == 't')
        {
          //mac set/get devaddr command
          if(UART_Data[8] == 'd' && UART_Data[9] == 'e' && UART_Data[10] == 'v' && UART_Data[11] == 'a' && UART_Data[12] == 'd' && UART_Data[13] == 'd' && UART_Data[14] == 'r')
          {
            Mac_DevAddr(DevAddr, UART_Data, UART_Nb_Bytes);
          }
        }
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
      UART_Nb_Bytes = 0;

      //Clear UART
      while(Serial.available() != 0)
      {
        UART_Data[0] = Serial.read();
      }      
    }

    
  }//While(1)
}
