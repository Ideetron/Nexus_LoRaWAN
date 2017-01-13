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
#include "RMF95.h"

/*
*****************************************************************************************
* INCLUDE GLOBAL VARIABLES
*****************************************************************************************
*/

extern unsigned char NwkSkey[16];
extern unsigned char AppSkey[16];

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

void Mac_DevAddr(unsigned char *DevAddr, unsigned char *Data, unsigned char Nb_Bytes)
{
  
  //Check if it is a set command and there is enough data sent
  if(Data[4] == 's' && Nb_Bytes == 24)
  {
    DevAddr[0] = ASCII2Hex(Data[16],Data[17]);
    DevAddr[1] = ASCII2Hex(Data[18],Data[19]);
    DevAddr[2] = ASCII2Hex(Data[20],Data[21]);
    DevAddr[3] = ASCII2Hex(Data[22],Data[23]);    
  }

  //Send set DevAddr
  Serial.write("DevAddr: ");
  UART_Send_Data(DevAddr, 0x04);
  UART_Send_Newline();  
}

void Mac_NwkSkey(unsigned char *Data, unsigned char Nb_Bytes)
{
  unsigned char i;
  
  //Check if it is a set command and there is enough data sent
  if(Data[4] == 's' && Nb_Bytes == 48)
  {
    for(i = 0; i < 16; i++)
    {
      NwkSkey[i] = ASCII2Hex(Data[(i*2)+16],Data[(i*2)+17]);
    }
  }

  //Send NwkSkey
  Serial.write("NwkSkey: ");
  UART_Send_Data(NwkSkey,0x10);
  UART_Send_Newline();  
}

void Mac_AppSkey(unsigned char *Data, unsigned char Nb_Bytes)
{
  unsigned char i;
  
  //Check if it is a set command and there is enough data sent
  if(Data[4] == 's' && Nb_Bytes == 48)
  {
    for(i = 0; i < 16; i++)
    {
      AppSkey[i] = ASCII2Hex(Data[(i*2)+16],Data[(i*2)+17]);
    }
  }

  //Send AppSkey
  Serial.write("AppSkey: ");
  UART_Send_Data(AppSkey,0x10);
  UART_Send_Newline();
}

void Mac_Appkey(unsigned char *AppKey, unsigned char *Data, unsigned char Nb_Bytes)
{
  unsigned char i;
  
  //Check if it is a set command and there is enough data sent
  if(Data[4] == 's' && Nb_Bytes == 47)
  {
    for(i = 0; i < 16; i++)
    {
      AppKey[i] = ASCII2Hex(Data[(i*2)+15],Data[(i*2)+16]);
    }
  }

  //Send AppKey
  Serial.write("AppKey: ");
  UART_Send_Data(AppKey,0x10);
  UART_Send_Newline();
  
}

void Mac_AppEUI(unsigned char *AppEUI, unsigned char *Data, unsigned char Nb_Bytes)
{
  unsigned char i;

  //Check if it is a set command and there is enough data sent
  if(Data[4] == 's' && Nb_Bytes == 31)
  {
    for(i = 0; i < 8; i++)
    {
      AppEUI[i] = ASCII2Hex(Data[(i*2)+15],Data[(i*2)+16]);
    }
  }

  //Send AppEUI
  Serial.write("AppEUI: ");
  UART_Send_Data(AppEUI,0x08);
  UART_Send_Newline();
}

void Mac_DevEUI(unsigned char *DevEUI, unsigned char *Data, unsigned char Nb_Bytes)
{
  unsigned char i;

  //Check if it is a set command and there is enough data sent
  if(Data[4] == 's' && Nb_Bytes == 31)
  {
    for(i = 0; i < 8; i++)
    {
      DevEUI[i] = ASCII2Hex(Data[(i*2)+15],Data[(i*2)+16]);
    }
  }

  //Send DevEUI
  Serial.write("DevEUI: ");
  UART_Send_Data(DevEUI,0x08);
  UART_Send_Newline();  
}

void Mac_DrTx(unsigned char *Datarate, unsigned char *Data, unsigned char Nb_Bytes)
{
  unsigned char Datarate_Temp;

  //Check if it is a set command and ther is enough data sent
  if(Data[4] == 's' && Nb_Bytes == 15)
  {
    //Convert to temp
    Datarate_Temp = ASCII2Hex(Data[13],Data[14]);

    //Check if the value is oke
    if(Datarate_Temp <= 0x06)
    {
      *Datarate = Datarate_Temp;
    }    
  }

  Serial.write("Datarate Tx: ");
  
  UART_Send_Datarate(Datarate);
}

void Mac_DrRx(unsigned char *Datarate, unsigned char *Data, unsigned char Nb_Bytes)
{
  unsigned char Datarate_Temp;

  //Check if it is a set command and ther is enough data sent
  if(Data[4] == 's' && Nb_Bytes == 15)
  {
    //Convert to temp
    Datarate_Temp = ASCII2Hex(Data[13],Data[14]);

    //Check if the value is oke
    if(Datarate_Temp <= 0x06)
    {
      *Datarate = Datarate_Temp;
    }    
  }

  Serial.write("Datarate Rx: ");
  
  UART_Send_Datarate(Datarate);
}

void Mac_ChTx(unsigned char *Channel, unsigned char *Data, unsigned char Nb_Bytes)
{
 unsigned char Channel_Temp;

  //Check if it is a set command and ther is enough data sent
  if(Data[4] == 's' && Nb_Bytes == 15)
  {
    //Convert to temp
    Channel_Temp = ASCII2Hex(Data[13],Data[14]);

    //Check if the value is oke
    if(Channel_Temp <= 0x07 || Channel_Temp == 0x10)
    {
      *Channel = Channel_Temp;
    }    
  }

  Serial.write("Channel Tx: ");
  
  UART_Send_Channel(Channel);
}

void Mac_ChRx(unsigned char *Channel, unsigned char *Data, unsigned char Nb_Bytes)
{
  unsigned char Channel_Temp;

  //Check if it is a set command and ther is enough data sent
  if(Data[4] == 's' && Nb_Bytes == 15)
  {
    //Convert to temp
    Channel_Temp = ASCII2Hex(Data[13],Data[14]);

    //Check if the value is oke
    if(Channel_Temp <= 0x07 || Channel_Temp == 0x10)
    {
      *Channel = Channel_Temp;
    }    
  }

  Serial.write("Channel Rx: ");
  
  UART_Send_Channel(Channel);
}

void Mac_Power(unsigned char *Data, unsigned char Nb_Bytes)
{
  
}

