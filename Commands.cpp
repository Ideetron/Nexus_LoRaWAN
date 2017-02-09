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
* File:     Commands.cpp
* Author:   Gerben den Hartog
* Compagny: Ideetron B.V.
* Website:  http://www.ideetron.nl/LoRa
* E-mail:   info@ideetron.nl
****************************************************************************************/
/****************************************************************************************
* Created on:         12-01-2017
* Supported Hardware: ID150119-02 Nexus board with RFM95
****************************************************************************************/

/*
*****************************************************************************************
* INCLUDE FILES
*****************************************************************************************
*/

#include <SPI.h>
#include "Commands.h"
#include "Conversions.h"
#include "RFM95.h"
#include "Struct.h"

void UART_Send_Newline()
{
  Serial.write(0x0D); //Carriage return
  Serial.write(0x0A); //Line feed
}

void UART_Send_Data(unsigned char *Data, unsigned char Length)
{
  unsigned char i;
  unsigned char Upper_Nibble;
  unsigned char Lower_Nibble;

  for(i = 0; i < Length; i++)
  {
    Hex2ASCII(Data[i], &Upper_Nibble, &Lower_Nibble);

    //Send the data
    Serial.write(Upper_Nibble);
    Serial.write(Lower_Nibble);
  }
}

void UART_Send_Datarate(unsigned char *Datarate)
{

  switch(*Datarate)
  {
      case 0x00:
          Serial.write("SF 12 BW 125");
          break;
      case 0x01:
          Serial.write("SF 11 BW 125");
          break;
      case 0x02:
          Serial.write("SF 10 BW 125");
          break;
      case 0x03:
          Serial.write("SF 9 BW 125");
          break;
      case 0x04:
          Serial.write("SF 8 BW 125");
          break;
      case 0x05:
          Serial.write("SF 7 BW 125");
          break;
      case 0x06:
          Serial.write("SF 7 BW 250");
          break;
  }

  UART_Send_Newline();
}

void UART_Send_Channel(unsigned char *Channel)
{
  switch(*Channel)
  {
    case 0x00:
      Serial.write("868.100");
      break;
    case 0x01:
      Serial.write("868.300");
      break;
    case 0x02:
      Serial.write("868.500");
      break;
    case 0x03:
      Serial.write("867.100");
      break;
    case 0x04:
      Serial.write("867.300");
      break;
    case 0x05:
      Serial.write("867.500");
      break;
    case 0x06:
      Serial.write("867.700");
      break;
    case 0x07:
      Serial.write("867.900");
      break;
    case 0x10:
      Serial.write("869.525");
      break;
    }

    UART_Send_Newline();

}

void Mac_DevAddr(sBuffer *UART_Buffer, unsigned char *DevAddr)
{

  //Check if it is a set command and there is enough data sent
  if(UART_Buffer->Data[4] == 's' && UART_Buffer->Counter == 24)
  {
    DevAddr[0] = ASCII2Hex(UART_Buffer->Data[16],UART_Buffer->Data[17]);
    DevAddr[1] = ASCII2Hex(UART_Buffer->Data[18],UART_Buffer->Data[19]);
    DevAddr[2] = ASCII2Hex(UART_Buffer->Data[20],UART_Buffer->Data[21]);
    DevAddr[3] = ASCII2Hex(UART_Buffer->Data[22],UART_Buffer->Data[23]);
  }

  //Send set DevAddr
  Serial.write("DevAddr: ");
  UART_Send_Data(DevAddr, 0x04);
  UART_Send_Newline();
}

void Mac_NwkSKey(sBuffer *UART_Buffer, unsigned char *NwkSKey)
{
  unsigned char i;

  //Check if it is a set command and there is enough data sent
  if(UART_Buffer->Data[4] == 's' && UART_Buffer->Counter == 48)
  {
    for(i = 0; i < 16; i++)
    {
      NwkSKey[i] = ASCII2Hex(UART_Buffer->Data[(i*2)+16],UART_Buffer->Data[(i*2)+17]);
    }
  }

  //Send NwkSkey
  Serial.write("NwkSKey: ");
  UART_Send_Data(NwkSKey,0x10);
  UART_Send_Newline();
}

void Mac_AppSKey(sBuffer *UART_Buffer, unsigned char *AppSKey)
{
  unsigned char i;

  //Check if it is a set command and there is enough data sent
  if(UART_Buffer->Data[4] == 's' && UART_Buffer->Counter == 48)
  {
    for(i = 0; i < 16; i++)
    {
      AppSKey[i] = ASCII2Hex(UART_Buffer->Data[(i*2)+16],UART_Buffer->Data[(i*2)+17]);
    }
  }

  //Send AppSkey
  Serial.write("AppSKey: ");
  UART_Send_Data(AppSKey,0x10);
  UART_Send_Newline();
}

void Mac_AppKey(sBuffer *UART_Buffer, unsigned char *AppKey)
{
  unsigned char i;

  //Check if it is a set command and there is enough data sent
  if(UART_Buffer->Data[4] == 's' && UART_Buffer->Counter == 47)
  {
    for(i = 0; i < 16; i++)
    {
      AppKey[i] = ASCII2Hex(UART_Buffer->Data[(i*2)+15],UART_Buffer->Data[(i*2)+16]);
    }
  }

  //Send AppKey
  Serial.write("AppKey: ");
  UART_Send_Data(AppKey,0x10);
  UART_Send_Newline();

}

void Mac_AppEUI(sBuffer *UART_Buffer, unsigned char *AppEUI)
{
  unsigned char i;

  //Check if it is a set command and there is enough data sent
  if(UART_Buffer->Data[4] == 's' && UART_Buffer->Counter == 31)
  {
    for(i = 0; i < 8; i++)
    {
      AppEUI[i] = ASCII2Hex(UART_Buffer->Data[(i*2)+15],UART_Buffer->Data[(i*2)+16]);
    }
  }

  //Send AppEUI
  Serial.write("AppEUI: ");
  UART_Send_Data(AppEUI,0x08);
  UART_Send_Newline();
}

void Mac_DevEUI(sBuffer *UART_Buffer, unsigned char *DevEUI)
{
  unsigned char i;

  //Check if it is a set command and there is enough data sent
  if(UART_Buffer->Data[4] == 's' && UART_Buffer->Counter == 31)
  {
    for(i = 0; i < 8; i++)
    {
      DevEUI[i] = ASCII2Hex(UART_Buffer->Data[(i*2)+15],UART_Buffer->Data[(i*2)+16]);
    }
  }

  //Send DevEUI
  Serial.write("DevEUI: ");
  UART_Send_Data(DevEUI,0x08);
  UART_Send_Newline();
}

void Mac_DrTx(sBuffer *UART_Buffer, unsigned char *Datarate)
{
  unsigned char Datarate_Temp;

  //Check if it is a set command and ther is enough data sent
  if(UART_Buffer->Data[4] == 's' && UART_Buffer->Counter == 15)
  {
    //Convert to temp
    Datarate_Temp = ASCII2Hex(UART_Buffer->Data[13],UART_Buffer->Data[14]);

    //Check if the value is oke
    if(Datarate_Temp <= 0x06)
    {
      *Datarate = Datarate_Temp;
    }
  }

  Serial.write("Datarate Tx: ");

  UART_Send_Datarate(Datarate);
}

void Mac_DrRx(sBuffer *UART_Buffer, unsigned char *Datarate)
{
  unsigned char Datarate_Temp;

  //Check if it is a set command and ther is enough data sent
  if(UART_Buffer->Data[4] == 's' && UART_Buffer->Counter == 15)
  {
    //Convert to temp
    Datarate_Temp = ASCII2Hex(UART_Buffer->Data[13],UART_Buffer->Data[14]);

    //Check if the value is oke
    if(Datarate_Temp <= 0x06)
    {
      *Datarate = Datarate_Temp;
    }
  }

  Serial.write("Datarate Rx: ");

  UART_Send_Datarate(Datarate);
}

void Mac_ChTx(sBuffer *UART_Buffer, unsigned char *Channel)
{
 unsigned char Channel_Temp;

  //Check if it is a set command and ther is enough data sent
  if(UART_Buffer->Data[4] == 's' && UART_Buffer->Counter == 15)
  {
    //Convert to temp
    Channel_Temp = ASCII2Hex(UART_Buffer->Data[13],UART_Buffer->Data[14]);

    //Check if the value is oke
    if(Channel_Temp <= 0x07 || Channel_Temp == 0x10)
    {
      *Channel = Channel_Temp;
    }
  }

  Serial.write("Channel Tx: ");

  UART_Send_Channel(Channel);
}

void Mac_ChRx(sBuffer *UART_Buffer, unsigned char *Channel)
{
  unsigned char Channel_Temp;

  //Check if it is a set command and ther is enough data sent
  if(UART_Buffer->Data[4] == 's' && UART_Buffer->Counter == 15)
  {
    //Convert to temp
    Channel_Temp = ASCII2Hex(UART_Buffer->Data[13],UART_Buffer->Data[14]);

    //Check if the value is oke
    if(Channel_Temp <= 0x07 || Channel_Temp == 0x10)
    {
      *Channel = Channel_Temp;
    }
  }

  Serial.write("Channel Rx: ");

  UART_Send_Channel(Channel);
}

void Mac_Power(sBuffer *UART_Buffer, unsigned char *Power)
{

  //Check if it is a set command and there is enough data sent
  if(UART_Buffer->Data[4] == 's' && UART_Buffer->Counter == 17)
  {
    unsigned char RFM_Data;
    
    *Power = ASCII2Hex(UART_Buffer->Data[15],UART_Buffer->Data[16]);

    //Check if power is not over 0x0F
    if(*Power > 0x0F)
    {
      *Power = 0x0F;
    }

    //Set all ther correct bits for the RFM register
    RFM_Data = *Power + 0xF0;

    //Write power to RFM module
    RFM_Write(0x09,RFM_Data);
  }

  //Send answer
  Serial.write("Power: ");
  UART_Send_Data(Power,0x01);
  UART_Send_Newline();
}

void Mac_Confirm(sBuffer *UART_Buffer, unsigned char *Confirm)
{
  //Check if it is a set command and there is enough data sent
  if(UART_Buffer->Data[4] == 's' && UART_Buffer->Counter == 14)
  {
    *Confirm = ASCII2Hex(UART_Buffer->Data[12],UART_Buffer->Data[13]);

    if(*Confirm >= 0x01)
    {
      *Confirm = 0x01;
    }
  }

  //Send answer
  Serial.write("Confirm: ");
  UART_Send_Data(Confirm,0x01);
  UART_Send_Newline();  
}

void Mac_Channel_Hopping(sBuffer *UART_Buffer, unsigned char *Channel_Hopping)
{
  //Check if it is a set command and there is enough data sent
  if(UART_Buffer->Data[4] == 's' && UART_Buffer->Counter == 16)
  {
    *Channel_Hopping = ASCII2Hex(UART_Buffer->Data[14],UART_Buffer->Data[15]);

    if(*Channel_Hopping >= 0x01)
    {
      *Channel_Hopping = 0x01;
    }
  }

  //Send answer
  Serial.write("Channel Hopping: ");
  UART_Send_Data(Channel_Hopping,0x01);
  UART_Send_Newline();
}

void Mac_Class(sBuffer *UART_Buffer, unsigned char *Class)
{
  //Check if it is a set command and there is enough data sent
  if(UART_Buffer->Data[4] == 's' && UART_Buffer->Counter == 16)
  {
    *Class = ASCII2Hex(UART_Buffer->Data[14],UART_Buffer->Data[15]);

    if(*Class >= 0x01)
    {
      *Class = 0x01;
    }
  }

  //Send answer
  Serial.write("Mote Class: ");
  if(*Class == 0x00)
  {
    Serial.write("A");
  }
  else
  {
    Serial.write("C");
  }
  UART_Send_Newline();
}

void Mac_Data(sBuffer *UART_Buffer, sBuffer *RFM_Buffer)
{
  unsigned char i;

  //Remove the "mac data " bytes
  UART_Buffer->Counter = UART_Buffer->Counter - 9;

  //Check for an even number of received data bytes other wise fill whit 0x00
  if(UART_Buffer->Counter % 2)
  {
    UART_Buffer->Data[UART_Buffer->Counter + 9] = 0x00;
    UART_Buffer->Counter++;
  }

  RFM_Buffer->Counter = UART_Buffer->Counter / 2;

  for(i = 0x00; i < RFM_Buffer->Counter; i++)
  {
    RFM_Buffer->Data[i] = ASCII2Hex(UART_Buffer->Data[(i*2)+9],UART_Buffer->Data[(i*2)+10]);
  }

  Serial.write("Data ");
  UART_Send_Data(RFM_Buffer->Data,RFM_Buffer->Counter);
  UART_Send_Newline();
}



