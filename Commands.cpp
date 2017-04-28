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

void Mac_DevAddr(sBuffer *UART_Buffer, unsigned char *UART_Data)
{

  byte datalen = 4;
  unsigned char eepromdata[4];
   byte i;
   int eepromstartaddr = 0;
  //Check if it is a set command and there is enough data sent
  if(UART_Buffer->Data[4] == 's' && UART_Buffer->Counter >= 24)
  {
    UART_Data[0] = ASCII2Hex(UART_Buffer->Data[16],UART_Buffer->Data[17]);
    UART_Data[1] = ASCII2Hex(UART_Buffer->Data[18],UART_Buffer->Data[19]);
    UART_Data[2] = ASCII2Hex(UART_Buffer->Data[20],UART_Buffer->Data[21]);
    UART_Data[3] = ASCII2Hex(UART_Buffer->Data[22],UART_Buffer->Data[23]);
    //for(i = start; i<datalen; i++)EEPROM.update(i, UART_Data[i]);
    EEPROMER(UART_Data, datalen,'w', eepromstartaddr);
  }
  
  //for(i = start; i<datalen; i++)UART_Data[i] = EEPROM.read(i);
  EEPROMER(UART_Data, datalen,'r', eepromstartaddr);
  Serial.write("DevAddr: ");
  UART_Send_Data(UART_Data, datalen);
  UART_Send_Newline();
}

void Mac_NwkSKey(sBuffer *UART_Buffer, unsigned char *UART_Data)
{
  unsigned char i,j;
  byte datalen = 0x10;
  int eepromstartaddr = 4;
  //Check if it is a set command and there is enough data sent
  if(UART_Buffer->Data[4] == 's' && UART_Buffer->Counter >= 48)
  {
    for(i = 0,j=0; i < 16; i++,j++)
    {
      UART_Data[i] = ASCII2Hex(UART_Buffer->Data[(j*2)+16],UART_Buffer->Data[(j*2)+17]);
    }
     EEPROMER(UART_Data, datalen, 'w', eepromstartaddr);
  }

  //Send NwkSkey
   EEPROMER(UART_Data, datalen, 'r', eepromstartaddr);
  Serial.write("NwkSKey: ");
  UART_Send_Data(UART_Data,0x10);
  UART_Send_Newline();
}

void Mac_AppSKey(sBuffer *UART_Buffer, unsigned char *UART_Data)    //here
{
  unsigned char i, j;
  byte datalen = 0x10;
  int eepromstartaddr = 20;

  //Check if it is a set command and there is enough data sent
  if(UART_Buffer->Data[4] == 's' && UART_Buffer->Counter == 48)
  {
    for(i = 0,j=0; i < 16; i++,j++)
    {
      UART_Data[i] = ASCII2Hex(UART_Buffer->Data[(j*2)+16],UART_Buffer->Data[(j*2)+17]);
    }
     EEPROMER(UART_Data, datalen, 'w', eepromstartaddr);
  }

   EEPROMER(UART_Data, datalen, 'r', eepromstartaddr);
  //Send AppSkey
  Serial.write("AppSKey: ");
  UART_Send_Data(UART_Data,0x10);
  UART_Send_Newline();
}
/**/
void Mac_AppKey(sBuffer *UART_Buffer, unsigned char *UART_Data)
{
  unsigned char i, j;
  byte datalen = 0x10;
  int eepromstartaddr = 36;

  //Check if it is a set command and there is enough data sent
  if(UART_Buffer->Data[4] == 's' && UART_Buffer->Counter == 47)
  {
    for(i = 0,j=0; i < 16; i++,j++)
    {
      UART_Data[i] = ASCII2Hex(UART_Buffer->Data[(j*2)+15],UART_Buffer->Data[(j*2)+16]);
    }
     EEPROMER(UART_Data, datalen, 'w', eepromstartaddr);
  }

  //Send AppKey
   EEPROMER(UART_Data, datalen, 'r', eepromstartaddr);
  Serial.write("AppKey: ");
  UART_Send_Data(UART_Data,0x10 );
  UART_Send_Newline();

}
/**/
/*
void Mac_AppKey(sBuffer *UART_Buffer, unsigned char *AppKey)
{
  unsigned char i, j;
  unsigned char startIndex = 20;

  //Check if it is a set command and there is enough data sent
  if(UART_Buffer->Data[4] == 's' && UART_Buffer->Counter == 47)
  {
    for(i = 0; i < 16; i++)
    {
      AppKey[i] = ASCII2Hex(UART_Buffer->Data[(i*2)+15],UART_Buffer->Data[(i*2)+16]);
    }
  }

  Serial.println(sizeof(AppKey));
  //Send AppKey
  Serial.write("AppKey: ");
  UART_Send_Data(AppKey,0x10,startIndex);
  UART_Send_Newline();

}
/**/

void Mac_AppEUI(sBuffer *UART_Buffer, unsigned char *UART_Data)
{
  unsigned char i, j;
  byte datalen = 0x08;
  int eepromstartaddr = 52;

  //Check if it is a set command and there is enough data sent
  if(UART_Buffer->Data[4] == 's' && UART_Buffer->Counter == 31)
  {
    for(i = 0,j=0; i < 8; i++,j++)
    {
      UART_Data[i] = ASCII2Hex(UART_Buffer->Data[(j*2)+15],UART_Buffer->Data[(j*2)+16]);
    }
     EEPROMER(UART_Data, datalen, 'w', eepromstartaddr);
  }

  //Send AppEUI
   EEPROMER(UART_Data, datalen, 'r', eepromstartaddr);
  Serial.write("AppEUI: ");
  UART_Send_Data(UART_Data,0x08 );
  UART_Send_Newline();
}

void Mac_DevEUI(sBuffer *UART_Buffer, unsigned char *UART_Data)
{
  unsigned char i, j;
  byte datalen = 0x08;
  int eepromstartaddr = 60;

  //Check if it is a set command and there is enough data sent
  if(UART_Buffer->Data[4] == 's' && UART_Buffer->Counter == 31)
  {
    for(i = 0; i < 8; i++)
    {
      UART_Data[i] = ASCII2Hex(UART_Buffer->Data[(i*2)+15],UART_Buffer->Data[(i*2)+16]);
    }
     EEPROMER(UART_Data, datalen, 'w', eepromstartaddr);
  }

  //Send DevEUI
   EEPROMER(UART_Data, datalen, 'r', eepromstartaddr);
  Serial.write("DevEUI: ");
  UART_Send_Data(UART_Data,0x08 );
  UART_Send_Newline();
}

void Mac_DrTx(sBuffer *UART_Buffer, unsigned char *UART_Data)
{
  unsigned char Datarate_Temp;
  byte datalen = 0x01;
  int eepromstartaddr = 61;

  //Check if it is a set command and ther is enough data sent
  if(UART_Buffer->Data[4] == 's' && UART_Buffer->Counter == 15)
  {
    //Convert to temp
    Datarate_Temp = ASCII2Hex(UART_Buffer->Data[13],UART_Buffer->Data[14]);

    //Check if the value is oke
    if(Datarate_Temp <= 0x06)
    {
     // *Datarate = Datarate_Temp;
       UART_Data[0]= Datarate_Temp;
        EEPROMER(UART_Data, datalen, 'w', eepromstartaddr);
    }
    
  }

   EEPROMER(UART_Data, datalen, 'r', eepromstartaddr);
  Serial.write("Datarate Tx: ");

  //unsigned char tmp = UART_Data[0];
  UART_Send_Datarate(&UART_Data[0]);
  //UART_Send_Data(UART_Data,0x01 );
  //UART_Send_Newline();
}

void Mac_DrRx(sBuffer *UART_Buffer, unsigned char *UART_Data)
{
  unsigned char Datarate_Temp;
  byte datalen = 0x01;
  int eepromstartaddr = 62;

  //Check if it is a set command and ther is enough data sent
  if(UART_Buffer->Data[4] == 's' && UART_Buffer->Counter == 15)
  {
    //Convert to temp
    Datarate_Temp = ASCII2Hex(UART_Buffer->Data[13],UART_Buffer->Data[14]);

    //Check if the value is oke
    if(Datarate_Temp <= 0x06)
    {
      //*Datarate = Datarate_Temp;
      UART_Data[0]= Datarate_Temp;
       EEPROMER(UART_Data, datalen, 'w', eepromstartaddr);
    }
  }

  EEPROMER(UART_Data, datalen,'r', eepromstartaddr);
  Serial.write("Datarate Rx: ");

  //UART_Send_Datarate(Datarate);
  UART_Send_Datarate(&UART_Data[0]);
  //UART_Send_Data(UART_Data,0x01 );
  //UART_Send_Newline();
}

void Mac_ChTx(sBuffer *UART_Buffer, unsigned char *UART_Data)
{
 unsigned char Channel_Temp;
 byte datalen = 0x01;
 int eepromstartaddr = 63;

  //Check if it is a set command and ther is enough data sent
  if(UART_Buffer->Data[4] == 's' && UART_Buffer->Counter == 15)
  {
    //Convert to temp
    Channel_Temp = ASCII2Hex(UART_Buffer->Data[13],UART_Buffer->Data[14]);

    //Check if the value is oke
    if(Channel_Temp <= 0x07 || Channel_Temp == 0x10)
    {
//      *Channel = Channel_Temp;
      UART_Data[0]= Channel_Temp;
      EEPROMER(UART_Data, datalen, 'w', eepromstartaddr);
    }
  }
  EEPROMER(UART_Data, datalen, 'r', eepromstartaddr);

  Serial.write("Channel Tx: ");

  //UART_Send_Channel(Channel);
   UART_Send_Datarate(&UART_Data[0]);
}

void Mac_ChRx(sBuffer *UART_Buffer, unsigned char *UART_Data)
{
  unsigned char Channel_Temp;
 byte datalen = 0x01;
 int eepromstartaddr = 64;

  //Check if it is a set command and ther is enough data sent
  if(UART_Buffer->Data[4] == 's' && UART_Buffer->Counter == 15)
  {
    //Convert to temp
    Channel_Temp = ASCII2Hex(UART_Buffer->Data[13],UART_Buffer->Data[14]);

    //Check if the value is oke
    if(Channel_Temp <= 0x07 || Channel_Temp == 0x10)
    {
      //*Channel = Channel_Temp;
      UART_Data[0]= Channel_Temp;
      EEPROMER(UART_Data, datalen, 'w', eepromstartaddr);
    }
  }
  EEPROMER(UART_Data, datalen, 'r', eepromstartaddr);

  Serial.write("Channel Rx: ");
  //UART_Send_Channel(Channel);
  UART_Send_Datarate(&UART_Data[0]);
}

void Mac_Power(sBuffer *UART_Buffer, unsigned char *UART_Data)                                                                        //here
{

  int eepromstartaddr = 65;
  //Check if it is a set command and there is enough data sent
  if(UART_Buffer->Data[4] == 's' && UART_Buffer->Counter == 17)
  {
    Serial.println("Setting power");
    unsigned char RFM_Data;

    *UART_Data = ASCII2Hex(UART_Buffer->Data[15],UART_Buffer->Data[16]);

    //Check if power is not over 0x0F
    if(*UART_Data > 0x0F)
    {
      *UART_Data = 0x0F;
    }
    EEPROMER(UART_Data, 0x01, 'w', eepromstartaddr);
    //Set all ther correct bits for the RFM register
    RFM_Data = *UART_Data + 0xF0;
    //Write power to RFM module
    RFM_Write(0x09,RFM_Data); 
  }

  //Send answer
  EEPROMER(UART_Data, 0x01, 'r', eepromstartaddr);
  Serial.write("Power: ");
  UART_Send_Data(UART_Data,0x01);
  UART_Send_Newline();
}

void Mac_Confirm(sBuffer *UART_Buffer, unsigned char *UART_Data)
{
  int eepromstartaddr = 66;
  //Check if it is a set command and there is enough data sent
  if(UART_Buffer->Data[4] == 's' && UART_Buffer->Counter == 14)
  {
    *UART_Data = ASCII2Hex(UART_Buffer->Data[12],UART_Buffer->Data[13]);

    if(*UART_Data >= 0x01)
    {
      *UART_Data = 0x01;
    }
    EEPROMER(UART_Data, 0x01, 'w', eepromstartaddr);
  }

  //Send answer
  EEPROMER(UART_Data, 0x01, 'r', eepromstartaddr);
  Serial.write("Confirm: ");
  UART_Send_Data(UART_Data,0x01);
  UART_Send_Newline();
}

void Mac_Channel_Hopping(sBuffer *UART_Buffer, unsigned char *UART_Data)
{
  int eepromstartaddr = 67;
  //Check if it is a set command and there is enough data sent
  if(UART_Buffer->Data[4] == 's' && UART_Buffer->Counter == 16)
  {
    *UART_Data = ASCII2Hex(UART_Buffer->Data[14],UART_Buffer->Data[15]);

    if(*UART_Data >= 0x01)
    {
      *UART_Data = 0x01;
    }
    EEPROMER(UART_Data, 0x01, 'w', eepromstartaddr);
  }

  //Send answer
  EEPROMER(UART_Data, 0x01, 'r', eepromstartaddr);
  Serial.write("Channel Hopping: ");
  UART_Send_Data(UART_Data,0x01);
  UART_Send_Newline();
}

void Mac_Class(sBuffer *UART_Buffer, sSettings *LoRa_Settings)
{
   int eepromstartaddr = 68;
   unsigned char tmp[1];
  //Check if it is a set command and there is enough data sent
  if(UART_Buffer->Data[4] == 's' && UART_Buffer->Counter == 16)
  {
    LoRa_Settings->Mote_Class = ASCII2Hex(UART_Buffer->Data[14],UART_Buffer->Data[15]);

    if(LoRa_Settings->Mote_Class >= 0x01)
    {
      LoRa_Settings->Mote_Class = 0x01;
    }
     tmp[0] = LoRa_Settings->Mote_Class;
    EEPROMER(tmp, 0x01, 'w', eepromstartaddr);
    
  }

  //Send answer and switch rfm to standby or receive
  tmp[0] = LoRa_Settings->Mote_Class;
  EEPROMER(tmp, 0x01, 'r', eepromstartaddr);
  Serial.write("Mote Class: ");
  if(LoRa_Settings->Mote_Class == 0x00)
  {

    //Switch RFM to standby
    RFM_Switch_Mode(0x01);
    
    Serial.write("A");
  }
  else
  {
    //Switch RFM to continuou receive
    RFM_Continuous_Receive(LoRa_Settings);
    
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




