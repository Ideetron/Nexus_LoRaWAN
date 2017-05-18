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
* File:     Commands.h
* Author:   Gerben den Hartog
* Compagny: Ideetron B.V.
* Website:  http://www.ideetron.nl/LoRa
* E-mail:   info@ideetron.nl
****************************************************************************************/
/****************************************************************************************
* Created on:         13-01-2017
* Supported Hardware: ID150119-02 Nexus board with RFM95
****************************************************************************************/


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

void EEPROMER(unsigned char *Data, unsigned char Length, char action, int eepromstartaddr){
  int i;
  switch(action)
  {
    case 'w':                                         
      for(i = 0; i<Length; i++)EEPROM.update(i+eepromstartaddr, Data[i]);
      break;
    case 'r':
      for(i = 0; i<Length; i++)Data[i] = EEPROM.read(i+eepromstartaddr);
  }
}

void UART_Send_Datarate(unsigned char *Datarate)
{
/*
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
  */
}

void UART_Send_Channel(unsigned char *Channel)
{/*
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
*/
}


void Store_Config(sBuffer *UART_Buffer, unsigned char *UART_Data, byte datalen, int eepromstartaddr)
{

   byte i;
   byte tmp = UART_Data[0];
   if(tmp == '0' || tmp == '1' )return;
   if(UART_Data[1] == '1')eepromstartaddr += 69;// 0->TTN, 1-> KCS. 2 sets of credentials
   
  //Check if it is a set command and there is enough data sent
  if(UART_Buffer->Data[2] == 's' && UART_Buffer->Counter >= (datalen*2 + 4))
  {
    for(i = 0; i < datalen; i++)
    {
      UART_Data[i] = ASCII2Hex(UART_Buffer->Data[(i*2)+4],UART_Buffer->Data[(i*2)+5]);
    }
    switch(tmp)
      {
        case 'e':
          if(UART_Data[0] > 0x0F)UART_Data[0] = 0x0F;
          break;
        case 'f':
        case 'g':
        case 'h':
          if(UART_Data[0] > 0x01)UART_Data[0] = 0x01;
          break;
      }    
    EEPROMER(UART_Data, datalen,'w', eepromstartaddr);
  }
  
    //for(i = 0; i<datalen; i++)UART_Data[i] = EEPROM.read(i+eepromstartaddr);
    EEPROMER(UART_Data, datalen,'r', eepromstartaddr);

   //power
    if(tmp == 'e')//power
    {
      unsigned char RFM_Data;
      RFM_Data = UART_Data[0] + 0xF0;
      //Write power to RFM module
      RFM_Write(0x09,RFM_Data);
    }

    Serial.write(tmp);
    Serial.write(": ");
    UART_Send_Data(UART_Data, datalen);
    UART_Send_Newline();
}
void Mac_Data(sBuffer *UART_Buffer, sBuffer *RFM_Buffer)
{
  unsigned char i;

  //Remove the "mac data " bytes
 // UART_Buffer->Counter = UART_Buffer->Counter - 9;
  UART_Buffer->Counter = UART_Buffer->Counter - 2;

  Serial.println(UART_Buffer->Counter);

  //Check for an even number of received data bytes other wise fill whit 0x00
  if(UART_Buffer->Counter % 2)
  {
    UART_Buffer->Data[UART_Buffer->Counter + 2] = 0x00;
    UART_Buffer->Counter++;
  }

  RFM_Buffer->Counter = UART_Buffer->Counter / 2;

  for(i = 0x00; i < RFM_Buffer->Counter; i++)
  {
    RFM_Buffer->Data[i] = ASCII2Hex(UART_Buffer->Data[(i*2)+2],UART_Buffer->Data[(i*2)+3]);
  }

  Serial.write("Data ");
  UART_Send_Data(RFM_Buffer->Data,RFM_Buffer->Counter);
  UART_Send_Newline();
}




