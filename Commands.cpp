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

void Mac_DevAddr(unsigned char *DevAddr, unsigned char *Data, unsigned char Nb_Bytes)
{
  
  //Check if it is a set command and there is enough data send
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

