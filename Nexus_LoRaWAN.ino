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
   //Initialize the UART on 9600 baud 8N1
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

  //Initialise timer 2 for 1ms waitloops
  WaitLoop_Init();

  //Wait until RFM module is started
  WaitLoop(20);
}

void loop()
{
  unsigned char i;

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

  LoRa_Settings.Mote_Class = 0x00; //0x00 is type A, 0x01 is type C

  LoRa_Settings.Datarate_Rx = 0x03;
  LoRa_Settings.Channel_Rx = 0x10;
  LoRa_Settings.Datarate_Tx = 0x00;
  LoRa_Settings.Channel_Tx = 0x00;

  LoRa_Settings.Confirm = 0x00; //0x00 unconfirmed, 0x01 confirmed
  LoRa_Settings.Channel_Hopping = 0x00; //0x00 no channel hopping, 0x01 channel hopping

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

  Message_Rx.Direction = 0x01; //Set down direction for Rx message

  //Initialise DS2401 bytes
  unsigned char DS_Bytes[8];
  unsigned char DS_Status = 0x00;

  //Get unique ID from the DS2401 and check the CRC
  while(DS_Status == 0x00)
  {
    DS_Read(DS_Bytes);

    DS_Status = DS_CheckCRC(DS_Bytes);

    WaitLoop(10);
  }

  //Load First 4 bytes in Device ID
  Address_Tx[0] = DS_Bytes[4];
  Address_Tx[1] = DS_Bytes[3];
  Address_Tx[2] = DS_Bytes[2];
  Address_Tx[3] = DS_Bytes[1];

  //Initialize RFM module
  RFM_Init();

  while(1)
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
      //Check for commands
      //MAC command type
      if(UART_Data[0] == 'm' && UART_Data[1] == 'a' && UART_Data[2] == 'c')
      {
        //Check for a join command
        if(UART_Data[4] == 'j' && UART_Data[5] == 'o' && UART_Data[6] == 'i' && UART_Data[7] == 'n')
        {
          //Check if there is no command pending
          if(RFM_Command_Status == NO_RFM_COMMAND)
          {
            Serial.write("Join");

            UART_Send_Newline();

            //Set join command
            RFM_Command_Status = JOIN;
          }
        }

        //Check for a data command
        if(UART_Data[4] == 'd' && UART_Data[5] == 'a' && UART_Data[6] == 't' && UART_Data[7] == 'a')
        {
          //Check if there is no command pending
          if(RFM_Command_Status == NO_RFM_COMMAND)
          {
            //Set new command for RFM
            RFM_Command_Status = NEW_RFM_COMMAND;

            Mac_Data(&UART_Rx_Buffer, &Buffer_Tx);
          }
        }

        //Check for reset command
        if(UART_Data[4] == 'r' && UART_Data[5] == 'e' && UART_Data[6] == 's' && UART_Data[7] == 'e' && UART_Data[8] == 't')
        {

        }

        //Check for a set or get command
        if((UART_Data[4] == 's' || UART_Data[4] == 'g') && UART_Data[5] == 'e' && UART_Data[6] == 't')
        {
          //mac set/get devaddr command
          if(UART_Data[8] == 'd' && UART_Data[9] == 'e' && UART_Data[10] == 'v' && UART_Data[11] == 'a' && UART_Data[12] == 'd' && UART_Data[13] == 'd' && UART_Data[14] == 'r')
          {
            Mac_DevAddr(&UART_Rx_Buffer, Address_Tx);

            //Reset frame counter
            Frame_Counter_Tx = 0x0000;

            //Reset RFM command status
            RFM_Command_Status = NO_RFM_COMMAND;
          }

          //mac set/get nwkskey
          if(UART_Data[8] == 'n' && UART_Data[9] == 'w' && UART_Data[10] == 'k' && UART_Data[11] == 's' && UART_Data[12] == 'k' && UART_Data[13] == 'e' && UART_Data[14] == 'y')
          {
            Mac_NwkSKey(&UART_Rx_Buffer, NwkSKey);

            //Reset frame counter
            Frame_Counter_Tx = 0x0000;

            //Reset RFM commando
            RFM_Command_Status = NO_RFM_COMMAND;
          }

          //mac set/get appskey
          if(UART_Data[8] == 'a' && UART_Data[9] == 'p' && UART_Data[10] == 'p' && UART_Data[11] == 's' && UART_Data[12] == 'k' && UART_Data[13] == 'e' && UART_Data[14] == 'y')
          {
            Mac_AppSKey(&UART_Rx_Buffer, AppSKey);

            //Reset frame counter
            Frame_Counter_Tx = 0x0000;

            //Reset rfm command
            RFM_Command_Status = NO_RFM_COMMAND;
          }

          //mac set/get appkey
          if(UART_Data[8] == 'a' && UART_Data[9] == 'p' && UART_Data[10] == 'p' && UART_Data[11] == 'k' && UART_Data[12] == 'e' && UART_Data[13] == 'y')
          {
            Mac_AppKey(&UART_Rx_Buffer, AppKey);

            //Reset RFM command
            RFM_Command_Status = NO_RFM_COMMAND;
          }

          //mac set/get appeui
          if(UART_Data[8] == 'a' && UART_Data[9] == 'p' && UART_Data[10] == 'p' && UART_Data[11] == 'e' && UART_Data[12] == 'u' && UART_Data[13] == 'i')
          {
            Mac_AppEUI(&UART_Rx_Buffer, AppEUI);

            //Reset RFM command
            RFM_Command_Status = NO_RFM_COMMAND;
          }

          //mac set/get deveui
          if(UART_Data[8] == 'd' && UART_Data[9] == 'e' && UART_Data[10] == 'v' && UART_Data[11] == 'e' && UART_Data[12] == 'u' && UART_Data[13] == 'i')
          {
            Mac_DevEUI(&UART_Rx_Buffer, DevEUI);

            //Reset RFM command
            RFM_Command_Status = NO_RFM_COMMAND;
          }

          //mac set/get drtx
          if(UART_Data[8] == 'd' && UART_Data[9] == 'r' && UART_Data[10] == 't' && UART_Data[11] == 'x')
          {
            Mac_DrTx(&UART_Rx_Buffer, &LoRa_Settings.Datarate_Tx);

            //Reset RFM command
            RFM_Command_Status = NO_RFM_COMMAND;
          }

          //mac set/get drrx
          if(UART_Data[8] == 'd' && UART_Data[9] == 'r' && UART_Data[10] == 'r' && UART_Data[11] == 'x')
          {
            Mac_DrRx(&UART_Rx_Buffer, &LoRa_Settings.Datarate_Rx);

            //Reset RFM command
            RFM_Command_Status = NO_RFM_COMMAND;
          }

          //mac set/get chtx
          if(UART_Data[8] == 'c' && UART_Data[9] == 'h' && UART_Data[10] == 't' && UART_Data[11] == 'x')
          {
            Mac_ChTx(&UART_Rx_Buffer, &LoRa_Settings.Channel_Tx);

            //Reset RFM command
            RFM_Command_Status = NO_RFM_COMMAND;
          }

          //mac set/get chrx
          if(UART_Data[8] == 'c' && UART_Data[9] == 'h' && UART_Data[10] == 'r' && UART_Data[11] == 'x')
          {
            Mac_ChRx(&UART_Rx_Buffer, &LoRa_Settings.Channel_Rx);

            //Reset RFM command
            RFM_Command_Status = NO_RFM_COMMAND;
          }

          //mac set/get pwridx
          if(UART_Data[8] == 'p' && UART_Data[9] == 'w' && UART_Data[10] == 'r' && UART_Data[11] == 'i' && UART_Data[12] == 'd' && UART_Data[13] == 'x')
          {
            Mac_Power(&UART_Rx_Buffer, &LoRa_Settings.Transmit_Power);
          }

          //mac set/get cnf
          if(UART_Data[8] == 'c' && UART_Data[9] == 'n' && UART_Data[10] == 'f')
          {
            Mac_Confirm(&UART_Rx_Buffer, &LoRa_Settings.Confirm);

            //Reset RFM command
            RFM_Command_Status = NO_RFM_COMMAND;
          }

          //mac set/get chhop
          if(UART_Data[8] == 'c' && UART_Data[9] == 'h' && UART_Data[10] == 'h' && UART_Data[11] == 'o' && UART_Data[12] == 'p')
          {
            Mac_Channel_Hopping(&UART_Rx_Buffer, &LoRa_Settings.Channel_Hopping);
          }

          //mac set/get class
          if(UART_Data[8] == 'c' && UART_Data[9] == 'l' && UART_Data[10] == 'a' && UART_Data[11] == 's' && UART_Data[12] == 's')
          {
            Mac_Class(&UART_Rx_Buffer, &LoRa_Settings);

            //Reset RFM command
            RFM_Command_Status = NO_RFM_COMMAND;
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
      UART_Rx_Buffer.Counter = 0;

      //Clear UART
      while(Serial.available() != 0)
      {
        UART_Data[0] = Serial.read();
      }
    }

    //Type A mote transmit receive cycle
    if((RFM_Command_Status == NEW_RFM_COMMAND || RFM_Command_Status == JOIN) && LoRa_Settings.Mote_Class == 0x00)
    {
      //LoRa cycle
      LORA_Cycle(&Buffer_Tx, &Buffer_Rx, &RFM_Command_Status, &Session_Data, &OTAA_Data, &Message_Rx, &LoRa_Settings);

      RFM_Command_Status = NO_RFM_COMMAND;
    }

    //Type C mote transmit and receive handling
    if(LoRa_Settings.Mote_Class == 0x01)
    {
      if(RFM_Command_Status == JOIN)
      {
        //Start join precedure
        LoRa_Send_JoinReq(&OTAA_Data, &LoRa_Settings);

        //Clear RFM_Command
        RFM_Command_Status = NO_RFM_COMMAND;
      }

      //Transmit
      if(RFM_Command_Status == NEW_RFM_COMMAND)
      {
        //Lora send data
        LORA_Send_Data(&Buffer_Tx, &Session_Data, &LoRa_Settings);

        RFM_Command_Status = NO_RFM_COMMAND;
      }

      //Receive
      if(digitalRead(DIO0) == HIGH)
      {
        //Get data
        LORA_Receive_Data(&Buffer_Rx, &Session_Data, &OTAA_Data, &Message_Rx, &LoRa_Settings);

        Rx_Status = NEW_RX;
      }
    }

    //If there is new data
    if(Rx_Status == NEW_RX)
    {
		//Check if there is data in the received message
      	if(Buffer_Rx.Counter != 0x00)
		{
			UART_Send_Data(Buffer_Rx.Data,Buffer_Rx.Counter);
		}
      	else
      	{
      	  Serial.write("No data");
      	}

      	UART_Send_Newline();
      	UART_Send_Newline();

      	Rx_Status = NO_RX;
    }
  }//While(1)
}

