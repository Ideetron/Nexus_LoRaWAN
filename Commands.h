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

#ifndef COMMANDS_H
#define COMMANDS_H

#include "Struct.h"
#include <EEPROM.h>

/*
*****************************************************************************************
* FUNCTION PROTOTYPES
*****************************************************************************************
*/

void UART_Send_Newline();
void UART_Send_Data(unsigned char *Data, unsigned char Length);
void Store_Config(sBuffer *UART_Buffer, unsigned char *UART_Data, byte datalen, int eepromstartaddr);
void Mac_Data(sBuffer *UART_Buffer, sBuffer *RFM_Buffer);
void EEPROMER(unsigned char *Data, unsigned char Length, char action, int eepromstartaddr);

#endif


