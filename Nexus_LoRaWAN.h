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
* File:     Nexus_LoRaWAN.h
* Author:   Gerben den Hartog
* Compagny: Ideetron B.V.
* Website:  http://www.ideetron.nl/LoRa
* E-mail:   info@ideetron.nl
****************************************************************************************/
/****************************************************************************************
* Created on:         06-01-2017
* Supported Hardware: ID150119-02 Nexus board with RFM95
****************************************************************************************/

#ifndef LORAWAN_H
#define LORAWAN_H

/*
********************************************************************************************
* TYPE DEFINITION
********************************************************************************************
*/

typedef enum {NO_UART_DATA, NEW_UART_DATA, UART_DATA_DONE} uart_t;

/*
*****************************************************************************************
* DEFINES
*****************************************************************************************
*/

#define DS2401  2
#define MFP     3
#define DIO0    4
#define DIO1    5
#define DIO5    6
#define DIO2    7
#define CS      8
#define LED     9

#endif
