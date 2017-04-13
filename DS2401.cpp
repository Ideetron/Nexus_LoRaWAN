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
* File:     DS2401.cpp
* Author:   Gerben den Hartog
* Compagny: Ideetron B.V.
* Website:  http://www.ideetron.nl/LoRa
* E-mail:   info@ideetron.nl
****************************************************************************************/
/****************************************************************************************
* Created on:         13-01-2017
* Supported Hardware: ID150119-02 Nexus board with RFM95
****************************************************************************************/

#include "Arduino.h"
#include "Nexus_LoRaWAN.h"
#include "DS2401.h"

/*
*****************************************************************************************
* Description:  Function is used to read the all the bytes provide by the DS2401
*
* Arguments:    *DS_Bytes pointer to an array of 8 unsigned chars
*****************************************************************************************
*/
void DS_Read(unsigned char *DS_Bytes)
{
  unsigned char i;

  //Preform reset pulse
  pinMode(DS2401,OUTPUT);
  digitalWrite(DS2401,LOW);
  delayMicroseconds(700);
  pinMode(DS2401,INPUT);
  delayMicroseconds(700);

  //Send command 0x33 LSB
  DS_WR1();
  DS_WR1();
  DS_WR0();
  DS_WR0();
  DS_WR1();
  DS_WR1();
  DS_WR0();
  DS_WR0();

  //Read DS bytes
  for (i = 0; i < 8; i++)
  {
    *DS_Bytes = DS_ReadByte();
    DS_Bytes++;
  }

  //Shutdonw DS2401
  digitalWrite(DS2401,HIGH);  
}

/*
*****************************************************************************************
* Description:  Function is used to write a logical 1 to the DS2401
*****************************************************************************************
*/
void DS_WR1()
{
  pinMode(DS2401,OUTPUT);
  digitalWrite(DS2401,LOW);
  delayMicroseconds(2);
  pinMode(DS2401,INPUT);
  delayMicroseconds(80);
}

/*
*****************************************************************************************
* Description:  Function is used to write a logical 0 to the DS2401
*****************************************************************************************
*/
void DS_WR0()
{
  pinMode(DS2401,OUTPUT);
  digitalWrite(DS2401,LOW);
  delayMicroseconds(80);
  pinMode(DS2401,INPUT);
  delayMicroseconds(2);
}

/*
*****************************************************************************************
* Description:  Function is used to read the one byte provided by the DS2401
*
* Return:       Returns the byte received from the DS2401
*****************************************************************************************
*/
unsigned char DS_ReadByte()
{
  unsigned char DS_Byte = 0x00;
  unsigned char i, t = 0x01;

  for (i = 0; i < 8; i++)
  {
    pinMode(DS2401,OUTPUT);
    digitalWrite(DS2401,LOW);
    delayMicroseconds(2);
    pinMode(DS2401,INPUT);
    delayMicroseconds(10);
    if(digitalRead(DS2401) == 1)
    {
      DS_Byte = DS_Byte | t;
    }

    t = t * 2;
    delayMicroseconds(80);
  }
  return DS_Byte;
}

/*
*****************************************************************************************
* Description  : This function does a CRC check on the received data from the DS2401
*
* Arguments   : *DS_bytes pointer to the arry that holds the DS bytes
*
* Return    : Returns 0x01 when CRC check is OK
*****************************************************************************************
*/
unsigned char DS_CheckCRC(unsigned char *DS_bytes)
{
  unsigned char DS_status       = 0x00;
  unsigned char DS_current_byte = 0x00;
  unsigned char DS_crc          = 0x00;
  unsigned char DS_Polynom      = 0x0C;
  unsigned char DS_crc_carry    = 0x00;
  unsigned char i               = 0x00;
  unsigned char j               = 0x00;

  /*Loop for all 6 DS bytes*/
  for (i = 0; i < 7; i++)
  {
    DS_current_byte = *DS_bytes; //Get first byte

    /*Calculate CRC for all bits*/
    for (j = 0; j < 8; j++)
    {
      /*XOR bit 0 with bit 0 of current CRC*/
      if ((DS_crc & 0x01) != (DS_current_byte & 0x01))
      {
        DS_crc_carry = 0x80;
      }
      else
      {
        DS_crc_carry = 0x00;
      }

      /*shift CRC*/
      DS_crc = DS_crc >> 1;

      /*Check for carry*/
      if (DS_crc_carry == 0x80)
      {
        DS_crc = DS_crc | DS_crc_carry;

        DS_crc = DS_crc ^ DS_Polynom;
      }

      DS_current_byte = DS_current_byte >> 1;
    }

    DS_bytes++;
  }

  if (DS_crc == *DS_bytes)
  {
    DS_status = 0x01;
  }

  return DS_status;
}


