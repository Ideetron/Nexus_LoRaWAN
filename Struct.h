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
* File:     Struct.h
* Author:   Gerben den Hartog
* Compagny: Ideetron B.V.
* Website:  http://www.ideetron.nl/LoRa
* E-mail:   info@ideetron.nl
****************************************************************************************/
/****************************************************************************************
* Created on:         06-02-2017
* Supported Hardware: ID150119-02 Nexus board with RFM95
****************************************************************************************/


#ifndef STRUCT_H
#define STRUCT_H

/*
********************************************************************************************
* STRUCT DEFENITIONS
********************************************************************************************
*/

typedef struct {
    unsigned char *Data;
    unsigned char Counter;
} sBuffer;

typedef struct {
    unsigned char *NwkSKey;
    unsigned char *AppSKey;
    unsigned char *DevAddr;
    unsigned int  *Frame_Counter;
} sLoRa_Session;

typedef struct {
    unsigned char *DevEUI;
    unsigned char *AppEUI;
    unsigned char *AppKey;
    unsigned char *DevNonce;
    unsigned char *AppNonce;
    unsigned char *NetID;
} sLoRa_OTAA;

typedef struct{
    unsigned char MAC_Header;
    unsigned char DevAddr[4];
    unsigned char Frame_Control;
    unsigned int  Frame_Counter;
    unsigned char Frame_Port;
    unsigned char Frame_Options[15];
    unsigned char MIC[4];
    unsigned char Direction;
} sLoRa_Message;

typedef struct {
    unsigned char Confirm;
    unsigned char Mote_Type;
    unsigned char Datarate_Tx;
    unsigned char Datarate_Rx;
    unsigned char Channel_Tx;
    unsigned char Channel_Rx;
    unsigned char Channel_Hopping;
    unsigned char Transmit_Power;
} sSettings;



#endif

