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

/*
*****************************************************************************************
* FUNCTION PROTOTYPES
*****************************************************************************************
*/

void UART_Send_Newline();
void UART_Send_Data(unsigned char *Data, unsigned char Length);
void Mac_DevAddr(sBuffer *UART_Buffer, unsigned char *DevAddr);
void Mac_NwkSKey(sBuffer *UART_Buffer, unsigned char *NwkSKey);
void Mac_AppSKey(sBuffer *UART_Buffer, unsigned char *AppSKey);
void Mac_AppKey(sBuffer *UART_Buffer, unsigned char *AppKey);
void Mac_AppEUI(sBuffer *UART_Buffer, unsigned char *AppEUI);
void Mac_DevEUI(sBuffer *UART_Buffer, unsigned char *DevEUI);
void Mac_DrTx(sBuffer *UART_Buffer, unsigned char *Datarate);
void Mac_DrRx(sBuffer *UART_Buffer, unsigned char *Datarate);
void Mac_ChTx(sBuffer *UART_Buffer, unsigned char *Channel);
void Mac_ChRx(sBuffer *UART_Buffer, unsigned char *Channel);
void Mac_Power(sBuffer *UART_Buffer, unsigned char *Power);
void Mac_Confirm(sBuffer *UART_Buffer, unsigned char *Confirm);
void Mac_Channel_Hopping(sBuffer *UART_Buffer, unsigned char *Channel_Hopping);
void Mac_Class(sBuffer *UART_Buffer, sSettings *LoRa_Settings);
void Mac_Data(sBuffer *UART_Buffer, sBuffer *RFM_Buffer);

#endif

