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
* File:     LoRaMAC.cpp
* Author:   Gerben den Hartog
* Compagny: Ideetron B.V.
* Website:  http://www.ideetron.nl/LoRa
* E-mail:   info@ideetron.nl
****************************************************************************************/
/****************************************************************************************
* Created on:         06-01-2017
* Supported Hardware: ID150119-02 Nexus board with RFM95
****************************************************************************************/
/*
*****************************************************************************************
* INCLUDE FILES
*****************************************************************************************
*/

#include "RFM95.h"
#include "Encrypt.h"
#include "LoRaMAC.h"
#include "Waitloop.h"
#include "Struct.h"

/*
*****************************************************************************************
* FUNCTIONS
*****************************************************************************************
*/


/*
*****************************************************************************************
* Description : Function that handles one cycle of sending and receiving with the LoRaWAN protocol.
*               In this function the timing of receive slot 2 is handeld
*
* Arguments   : *Data_Tx pointer to the array of data that will be transmitted
*               *Data_Rx pointer to the array where the received data will be stored
*               Data_Length_Tx nuber of bytes to be transmitted
*
* Returns     : Number of bytes received
*****************************************************************************************
*/

void LORA_Cycle(sBuffer *Data_Tx, sBuffer *Data_Rx, RFM_command_t *RFM_Command, sLoRa_Session *Session_Data, sLoRa_OTAA *OTAA_Data, sLoRa_Message *Message_Rx, sSettings *LoRa_Settings)
{
  unsigned char i;

	//Time to wait in ms for receive slot 2
	unsigned char Receive_Delay_2 = 17;
  unsigned char Recieve_Delay_JoinAck = 57; //NEED TO CHECK THIS NUMBER

  if(RFM_Command == JOIN)
  {
    //Send join Req
    LoRa_Send_JoinReq(OTAA_Data);
    
    for(i = 0; i <= Receive_Delay_JoinAck; i ++ )
    {
      WaitLoop(100);
    }
  }

  if(RFM_Command == NEW_RFM_COMMAND)
  {

	  LORA_Send_Data(Data_Tx, Session_Data, LoRa_Setting);

    for(i = 0; i <= Receive_Delay_2; i ++ )
    {
      WaitLoop(100);
    }
  }

	LORA_Receive_Data(Data_Rx, Session_Data, OTAA_Data, Message_Rx);
}

/*
*****************************************************************************************
* Description : Function contstructs a LoRaWAN package and sends it
*
* Arguments   : *Data pointer to the array of data that will be transmitted
*               Data_Length nuber of bytes to be transmitted
*               Frame_Counter_Up  Frame counter of upstream frames
*****************************************************************************************
*/
void LORA_Send_Data(sBuffer *Data_Tx, sLoRa_Session *Session_Data, sSettings *LoRa_Settings)
{
	//Define variables
	unsigned char i;

	//Direction of frame is up
	unsigned char Direction = 0x00;

	unsigned char RFM_Data[256];
	unsigned char RFM_Package_Length;

	unsigned char MIC[4];

	//Unconfirmed data up
	unsigned char Mac_Header = 0x80;

	unsigned char Frame_Control = 0x00;
	unsigned char Frame_Port = 0x01;

	//Encrypt the data
	Encrypt_Payload(Data, Data_Length, Frame_Counter_Up, Direction, Address);

	//Build the Radio Package
	RFM_Data[0] = Mac_Header;

	RFM_Data[1] = Address[3];
	RFM_Data[2] = Address[2];
	RFM_Data[3] = Address[1];
	RFM_Data[4] = Address[0];

	RFM_Data[5] = Frame_Control;

	RFM_Data[6] = (Frame_Counter_Up & 0x00FF);
	RFM_Data[7] = ((Frame_Counter_Up >> 8) & 0x00FF);

	RFM_Data[8] = Frame_Port;

	//Set Current package length
	RFM_Package_Length = 9;

	//Load Data
	for(i = 0; i < Data_Length; i++)
	{
		RFM_Data[RFM_Package_Length + i] = Data[i];
	}

	//Add data Lenth to package length
	RFM_Package_Length = RFM_Package_Length + Data_Length;

	//Calculate MIC
	Calculate_MIC(RFM_Data, MIC, RFM_Package_Length, Frame_Counter_Up, Direction, Address);

	//Load MIC in package
	for(i = 0; i < 4; i++)
	{
		RFM_Data[i + RFM_Package_Length] = MIC[i];
	}

	//Add MIC length to RFM package length
	RFM_Package_Length = RFM_Package_Length + 4;

	//Send Package
	RFM_Send_Package(RFM_Data, RFM_Package_Length);

  //Raise frame counter
  if(*Session_Data->Frame_Counter != 0xFFFF)
  {
    //Raise frame counter
    *Session_Data->Frame_Counter = *Session_Data->Frame_Counter + 1;
  }
  else
  {
    *Session_Data->Frame_Counter = 0x0000;
  } 
}


/*
*****************************************************************************************
* Description : Function that handles received data. Checks the MIC and deconstructs the LoRaWAN package
*
* Arguments   : *Data pointer to the array where the received data will be stored
*
* Returns     : Number of bytes received
*****************************************************************************************
*/
void LORA_Receive_Data(sBuffer *Data_Rx, sLoRa_Session *Session_Data, sLoRa_OTAA *OTAA_Data, sLoRa_Message *Message)
{
	unsigned char i;

	unsigned char Data_Length = 0x00;

	unsigned char RFM_Data[64];
	unsigned char RFM_Package_Length = 0x00;
	unsigned char RFM_Interrupt;

	unsigned char MIC[4];
	unsigned char MIC_Check;

	unsigned int Frame_Counter_Down;
	unsigned char Frame_Control;
	unsigned char Frame_Options_Length;

	unsigned char Data_Location;

	unsigned char Direction;

	message_t Message_Status = NO_MESSAGE;


	Message_Status = RFM_Receive();


	//if CRC ok breakdown package
	if(Message_Status == CRC_OK)
	{
		//Get Message
		RFM_Package_Length = RFM_Get_Package(RFM_Data);

		//Set Direction for downlink frame
		Direction = 0x01;

		//Get frame counter
		Frame_Counter_Down = RFM_Data[7];
		Frame_Counter_Down = (Frame_Counter_Down << 8) + RFM_Data[6];

		Calculate_MIC(RFM_Data, MIC, (RFM_Package_Length - 4), Frame_Counter_Down, Direction, Address);

		MIC_Check = 0x00;

		for(i = 0; i < 4; i++)
		{
			if(RFM_Data[(RFM_Package_Length - 4) + i] == MIC[i])
			{
				MIC_Check++;
			}
		}

		if(MIC_Check == 0x04)
		{
			Message_Status = MIC_OK;
		}
		else
		{
			Message_Status = WRONG_MESSAGE;
		}
	}

	//if MIC is OK then decrypt the data
	if(Message_Status == MIC_OK)
	{
		Data_Location = 8;

		//Get frame control
		Frame_Control = RFM_Data[5];

		//Get length of frame options field
		Frame_Options_Length = (Frame_Control & 0x0F);

		//Add length of frame options field to data location
		Data_Location = Data_Location + Frame_Options_Length;

		//Check if ther is data in the package
		if(RFM_Package_Length == (Data_Location + 4))
		{
			Data_Length = 0x00;
		}
		else
		{
			Data_Length = (RFM_Package_Length - Data_Location -1 -4);
			Data_Location = (Data_Location + 1);
		}

		//Decrypt the data
		if(Data_Length != 0x00)
		{
			Encrypt_Payload(&RFM_Data[Data_Location], Data_Length, Frame_Counter_Down, Direction, Address);

			Message_Status = MESSAGE_DONE;
		}
	}

	if(Message_Status == MESSAGE_DONE)
	{
		for(i = 0; i < Data_Length; i++)
		{
			Data[i] = RFM_Data[Data_Location + i];
		}
	}

	if(Message_Status == WRONG_MESSAGE)
	{
		Data_Length = 0x00;
	}

	return Data_Length;
}

void LoRa_Send_JoinReq(sLoRa_OTAA *OTAA_Data)
{
    unsigned char i;
    
    unsigned char RFM_Data[23];
    sBuffer RFM_Package = { &RFM_Data[0], 0x00};
    
    sLoRa_Message Message;
    
    Message.MAC_Header = 0x00; //Join request
    Message.Direction = 0x00; //Set up Direction
    
    //Construct OTAA Request message
    //Load Header in package
    RFM_Data[0] = Message.MAC_Header;
    
    //Load AppEUI in package
    for(i = 0x00; i < 8; i++)
    {
        RFM_Data[i+1] = OTAA_Data->AppEUI[7-i];
    }
    
    //Load DevEUI in package
    for(i= 0x00; i < 8; i++)
    {
        RFM_Data[i+9] = OTAA_Data->DevEUI[7-i];
    }
    
    //Load DevNonce in package
    RFM_Data[17] = OTAA_Data->DevNonce[0];
    RFM_Data[18] = OTAA_Data->DevNonce[1];

    //Set length of package
    RFM_Package.Counter = 19;
    
    //Get MIC
    Calculate_MIC(&RFM_Package, OTAA_Data->AppKey, &Message);
    
    //Load MIC in package
    RFM_Data[19] = Message.MIC[0];
    RFM_Data[20] = Message.MIC[1];
    RFM_Data[21] = Message.MIC[2];
    RFM_Data[22] = Message.MIC[3];
    
    //Set lenght of package to the right length
    RFM_Package.Counter = 23;
    
    //Send Package
    RFM_Send_Package(&RFM_Package, Message.Direction);
}
