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
* File:     RFM95.cpp
* Author:   Gerben den Hartog
* Compagny: Ideetron B.V.
* Website:  http://www.ideetron.nl/LoRa
* E-mail:   info@ideetron.nl
****************************************************************************************/
/****************************************************************************************
* Created on:         06-01-2017
* Supported Hardware: ID150119-02 Nexus board with RFM95
****************************************************************************************/

#include <SPI.h>
#include "LoRaMAC.h"
#include "RFM95.h"
#include "Nexus_LoRaWAN.h"
#include "Arduino.h"
#include "Waitloop.h"
#include "Struct.h"
#include "Commands.h"

/*
*****************************************************************************************
* Description: Function used to initialize the RFM module on startup
*****************************************************************************************
*/

void RFM_Init()
{
  //Switch RFM to sleep
  //DONT USE Switch mode function
  RFM_Write(0x01,0x00);

  //Wait until RFM is in sleep mode
  WaitLoop(10);

  //Set RFM in LoRa mode
  //DONT USE Switch mode function
  RFM_Write(0x01,0x80);

  //Swtich RFM to standby
  RFM_Switch_Mode(0x01);

  //Set channel to channel 0 868.100 MHz
  RFM_Change_Channel(0x00);

  //PA pin (minimal power)
  RFM_Write(0x09,0xF0);

  //Switch LNA boost on
  RFM_Write(0x0C,0x23);

  //Set RFM To datarate 0 SF12 BW 125 kHz
  RFM_Change_Datarate(0x00);

  //Rx Timeout set to 37 symbols
  RFM_Write(0x1F,0x25);

  //Preamble length set to 8 symbols
  //0x0008 + 4 = 12
  RFM_Write(0x20,0x00);
  RFM_Write(0x21,0x08);

  //Set LoRa sync word
  RFM_Write(0x39,0x34);

  //Set FIFO pointers
  //TX base adress
  RFM_Write(0x0E,0x80);
  //Rx base adress
  RFM_Write(0x0F,0x00);

}

/*
*****************************************************************************************
* Description : Function for sending a package with the RFM
*
* Arguments   : *RFM_Tx_Package Pointer to arry with data to be send
*               Package_Length  Length of the package to send
*****************************************************************************************
*/

void RFM_Send_Package(sBuffer *RFM_Tx_Package, sSettings *LoRa_Settings)
{
  unsigned char i;
  unsigned char RFM_Tx_Location = 0x00;

  //Set RFM in Standby mode
  RFM_Switch_Mode(0x01);

  //Switch Datarate
  RFM_Change_Datarate(LoRa_Settings->Datarate_Tx);

  //Switch Channel
  RFM_Change_Channel(LoRa_Settings->Channel_Tx);

  //Switch DIO0 to TxDone
  RFM_Write(0x40,0x40);

  //Set IQ to normal values
  RFM_Write(0x33,0x27);
  RFM_Write(0x3B,0x1D);

  //Set payload length to the right length
  RFM_Write(0x22,RFM_Tx_Package->Counter);

  //Get location of Tx part of FiFo
  RFM_Tx_Location = RFM_Read(0x0E);

  //Set SPI pointer to start of Tx part in FiFo
  RFM_Write(0x0D,RFM_Tx_Location);

  //Write Payload to FiFo
  for (i = 0;i < RFM_Tx_Package->Counter; i++)
  {
    RFM_Write(0x00, RFM_Tx_Package->Data[i]);
  }

  //Switch RFM to Tx
  RFM_Write(0x01,0x83);

  //Wait for TxDone
  while(digitalRead(DIO0) == LOW)
  {
  }
  
  //Change DIO 0 back to RxDone
  RFM_Write(0x40,0x00);

  //Invert IQ Back
  RFM_Write(0x33,0x67);
  RFM_Write(0x3B,0x19);

  //Switch RFM back to receive if it is a type C mote
  if(LoRa_Settings->Mote_Class == 0x01)
  {
    //Switch Back to Continuous receive
    RFM_Continuous_Receive(LoRa_Settings);
  }
}

/*
*****************************************************************************************
* Description  : Switches the RFM module to receive
*****************************************************************************************
*/

message_t RFM_Single_Receive(sSettings *LoRa_Settings)
{
  message_t Message_Status = NO_MESSAGE;

  //Change Datarate
  RFM_Change_Datarate(LoRa_Settings->Datarate_Rx);

  //Change Channel
  RFM_Change_Channel(LoRa_Settings->Channel_Rx);

  //Clear interrupt register
  //To force the timeout pin low
  RFM_Write(0x12,0xE0);

  //Switch RFM to Single reception
  RFM_Switch_Mode(0x06);

  //Wait until RxDone or Timeout
  //Wait until timeout or RxDone interrupt
  while((digitalRead(DIO0) == LOW) && (digitalRead(DIO1) == LOW))
  {
  }

  //Check for Timeout
  if(digitalRead(DIO1) == HIGH)
  {
    Message_Status = TIMEOUT;

    Serial.write("Timeout");

  }

  //Check for RxDone
  if(digitalRead(DIO0) == HIGH)
  {
	  Message_Status = NEW_MESSAGE;

   Serial.write("New message");
  }

  return Message_Status;
}

void RFM_Continuous_Receive(sSettings *LoRa_Settings)
{
	//Change Datarate
	RFM_Change_Datarate(LoRa_Settings->Datarate_Rx);

	//Change Channel
	RFM_Change_Channel(LoRa_Settings->Channel_Rx);

	//Switch to continuous receive
	RFM_Switch_Mode(0x05);

}

/*
*****************************************************************************************
* Description : This function will check for a CRC error and get the received data
          from the RFM.
*
* Arguments   : *RFM_Rxd_package  Pointer to arry for the received data
*
* Returns   : Lenght of package
*****************************************************************************************
*/

message_t RFM_Get_Package(sBuffer *RFM_Rx_Package)
{
  unsigned char i;
  unsigned char RFM_Interrupts = 0x00;
  unsigned char RFM_Package_Location = 0x00;
  message_t Message_Status;

  //Get interrupt register
  RFM_Interrupts = RFM_Read(0x12);
    
  //Clear interrupt register
  RFM_Write(0x12,0xE0);

  //UART_Send_Data(RFM_Interrupts,0x01);

  //Check CRC
  if((RFM_Interrupts & 0x20) != 0x20)
  {
	  Message_Status = CRC_OK;

	  Serial.write("CRC OK");
   
	  UART_Send_Newline();
  }
  else
  {
	  Message_Status = WRONG_MESSAGE;

	  Serial.write("CRC NOK");

	  UART_Send_Newline();
  }

  RFM_Package_Location = RFM_Read(0x10); /*Read start position of received package*/
  RFM_Rx_Package->Counter = RFM_Read(0x13); /*Read length of received package*/

  RFM_Write(0x0D,RFM_Package_Location); /*Set SPI pointer to start of package*/

  for (i = 0x00; i < RFM_Rx_Package->Counter; i++)
  {
    RFM_Rx_Package->Data[i] = RFM_Read(0x00);
  }
  
  return Message_Status;
}

/*
*****************************************************************************************
* Description : Funtion that reads a register from the RFM and returns the value
*
* Arguments   : RFM_Address Address of register to be read
*
* Returns   : Value of the register
*****************************************************************************************
*/

unsigned char RFM_Read(unsigned char RFM_Address)
{
  unsigned char RFM_Data;

  //Set NSS pin low to start SPI communication
  digitalWrite(10,LOW);

  //Send Address
  SPI.transfer(RFM_Address);
  //Send 0x00 to be able to receive the answer from the RFM
  RFM_Data = SPI.transfer(0x00);

  //Set NSS high to end communication
  digitalWrite(10,HIGH);

  //Return received data
  return RFM_Data;
}

/*
*****************************************************************************************
* Description : Funtion that writes a register from the RFM
*
* Arguments   : RFM_Address Address of register to be written
*         RFM_Data    Data to be written
*****************************************************************************************
*/

void RFM_Write(unsigned char RFM_Address, unsigned char RFM_Data)
{
  //Set NSS pin Low to start communication
  digitalWrite(10,LOW);

  //Send Addres with MSB 1 to make it a writ command
  SPI.transfer(RFM_Address | 0x80);
  //Send Data
  SPI.transfer(RFM_Data);

  //Set NSS pin High to end communication
  digitalWrite(10,HIGH);
}

void RFM_Change_Datarate(unsigned char Datarate)
{
	switch(Datarate)
	{
		case 0x00: //SF12 BW 125 kHz
			RFM_Write(0x1E,0xC4); //SF12 CRC On
			RFM_Write(0x1D,0x72); //125 kHz 4/5 coding rate explicit header mode
			RFM_Write(0x26,0x0C); //Low datarate optimization on AGC auto on
			break;
		case 0x01: //SF11 BW 125 kHz
			RFM_Write(0x1E,0xB4); //SF11 CRC On
			RFM_Write(0x1D,0x72); //125 kHz 4/5 coding rate explicit header mode
			RFM_Write(0x26,0x0C); //Low datarate optimization on AGC auto on
			break;
		case 0x02: //SF10 BW 125 kHz
			RFM_Write(0x1E,0xA4); //SF10 CRC On
			RFM_Write(0x1D,0x72); //125 kHz 4/5 coding rate explicit header mode
			RFM_Write(0x26,0x04); //Low datarate optimization off AGC auto on
			break;
		case 0x03: //SF9 BW 125 kHz
			RFM_Write(0x1E,0x94); //SF9 CRC On
			RFM_Write(0x1D,0x72); //125 kHz 4/5 coding rate explicit header mode
			RFM_Write(0x26,0x04); //Low datarate optimization off AGC auto on
			break;
		case 0x04: //SF8 BW 125 kHz
			RFM_Write(0x1E,0x84); //SF8 CRC On
			RFM_Write(0x1D,0x72); //125 kHz 4/5 coding rate explicit header mode
			RFM_Write(0x26,0x04); //Low datarate optimization off AGC auto on
			break;
		case 0x05: //SF7 BW 125 kHz
			RFM_Write(0x1E,0x74); //SF7 CRC On
			RFM_Write(0x1D,0x72); //125 kHz 4/5 coding rate explicit header mode
			RFM_Write(0x26,0x04); //Low datarate optimization off AGC auto on
    		break;
        case 0x06: //SF7 BW 250kHz
			RFM_Write(0x1E,0x74); //SF7 CRC On
			RFM_Write(0x1D,0x82); //250 kHz 4/5 coding rate explicit header mode
			RFM_Write(0x26,0x04); //Low datarate optimization off AGC auto on
            break;
	}
}

void RFM_Change_Channel(unsigned char Channel)
{
	switch(Channel)
	{
		case 0x00: //Channel 0 868.100 MHz / 61.035 Hz = 14222987 = 0xD9068B
			RFM_Write(0x06,0xD9);
			RFM_Write(0x07,0x06);
			RFM_Write(0x08,0x8B);
			break;
		case 0x01: //Channel 1 868.300 MHz / 61.035 Hz = 14226264 = 0xD91358
			RFM_Write(0x06,0xD9);
			RFM_Write(0x07,0x13);
			RFM_Write(0x08,0x58);
			break;
		case 0x02: //Channel 2 868.500 MHz / 61.035 Hz = 14229540 = 0xD92024
			RFM_Write(0x06,0xD9);
			RFM_Write(0x07,0x20);
			RFM_Write(0x08,0x24);
			break;
		case 0x03: //Channel 3 867.100 MHz / 61.035 Hz = 14206603 = 0xD8C68B
			RFM_Write(0x06,0xD8);
			RFM_Write(0x07,0xC6);
			RFM_Write(0x08,0x8B);
			break;
		case 0x04: //Channel 4 867.300 MHz / 61.035 Hz = 14209880 = 0xD8D358
			RFM_Write(0x06,0xD8);
			RFM_Write(0x07,0xD3);
			RFM_Write(0x08,0x58);
			break;
		case 0x05: //Channel 5 867.500 MHz / 61.035 Hz = 14213156 = 0xD8E024
			RFM_Write(0x06,0xD8);
			RFM_Write(0x07,0xE0);
			RFM_Write(0x08,0x24);
			break;
		case 0x06: //Channel 6 867.700 MHz / 61.035 Hz = 14216433 = 0xD8ECF1
			RFM_Write(0x06,0xD8);
			RFM_Write(0x07,0xEC);
			RFM_Write(0x08,0xF1);
			break;
		case 0x07: //Channel 7 867.900 MHz / 61.035 Hz = 14219710 = 0xD8F9BE
			RFM_Write(0x06,0xD8);
			RFM_Write(0x07,0xF9);
			RFM_Write(0x08,0xBE);
			break;
		case 0x10: //Receive channel 869.525 MHz / 61.035 Hz = 14246334 = 0xD961BE
			RFM_Write(0x06,0xD9);
			RFM_Write(0x07,0x61);
			RFM_Write(0x08,0xBE);
			break;
	}
}

void RFM_Switch_Mode(unsigned char Mode)
{
    Mode = Mode | 0x80; //Set high bit for LoRa mode

    //Switch mode on RFM module
    RFM_Write(0x01,Mode);

	//Wait on mode ready
	while(digitalRead(DIO5) == LOW)
	{
	}
}
