#define TTN_OR_KCS A1
unsigned long int pulses = 0;


int configvalueAddr(int addr)
{
  if(digitalRead(TTN_OR_KCS) == 0)return addr;
  return  addr += 69;
}

int configvalueAddrfromletter(char addr)
{
  return 61 + (addr - 'a') + 69;
}

/*
 * 0   1    N/A     N/A     Fixed value: 0x24
1..12  12   N/A     N/A     Filler
13..14  2   MSB first Signed    Value of analog input 1. For default situation: current trough 50 ohms resistor, measured as 11 bits value such that 2048 counts correspond to 1.024V. 1 count corresponds to 0.0005V, which corresponds to 0.01mA. To get the value in milliamps, divide by 100.
15      1   N/A     N/A     Status of digital input 1. Value: 0x43 when open or high; 0x41 when closed or low.
16..17  2   MSB first Signed    Value of analog input 2. For default situation: current trough 50 ohms resistor, measured as 11 bits value such that 2048 counts correspond to 1.024V. 1 count corresponds to 0.0005V, which corresponds to 0.01mA. To get the value in milliamps, divide by 100.
18      1   N/A     N/A     Status of digital input 2. Value: 0x43 when open or high; 0x41 when closed or low.
19..22  4   MSB first Unsigned  Digital input 1. Counter value of the pulse counter.
23..24  2   MSB first Unsigned  Digital input 2. Counter value of the pulse counter.
25..26  2   MSB first Signed    Temperature. Multiply by 0.0625 to get the temperature in degrees Celcius. Special value: 0x8000 indicates that no temperature sensor was found.
27..28  2   LSB first Unsigned  Battery voltage. Multiply by 6.35 to get the voltage in millivolts.
 */

void transmitpacket(sBuffer *RFM_Buffer)        //create the packet to TX...
{
  unsigned char i;
  RFM_Buffer->Counter = 29;
  for(i = 0x01; i < 29; i++)
  {
    RFM_Buffer->Data[i] = 0x00;
  }

  Serial.println(sizeof (long int));
  Serial.println(pulses);
  //add pulses...
  RFM_Buffer->Data[19] = (pulses & 0xff000000) >> 24; 
  RFM_Buffer->Data[20] = (pulses & 0x00ff0000) >> 16;
  RFM_Buffer->Data[21] = (pulses & 0x0000ff00) >> 8;
  RFM_Buffer->Data[22] = (pulses & 0x000000ff) >> 0;

  
 // RFM_Buffer->Counter = 0x1D;
  #ifdef SERIALDEBUG
 // UART_Send_Data(pulses,29);
  Serial.write("Data ");  
  UART_Send_Data(RFM_Buffer->Data,29);
  UART_Send_Newline();
  #endif
}


void transmitdata()
  {

  cli();
  //Serial.print("is Tx...");
 // Serial.println(Tx_done);
  //count_enable = false;
  if(Tx_done == false)return;
    /*

EEPROMER(NwkSKey, 16, 'r', 4);
  EEPROMER(AppSKey, 16, 'r', 20);
  EEPROMER(Address_Tx, 4, 'r', configvalueAddr(0));
  EEPROMER(DevEUI, 8, 'r', 60);
  EEPROMER(AppEUI, 8, 'r',52);
   EEPROMER(AppKey, 16, 'r',36);
EEPROMER(AppSKey, 16, 'r', 20);*/
    //ABP/OTAA
   unsigned char abporotaa[1];
      EEPROMER(Address_Tx, 4, 'r', configvalueAddr(0));
      EEPROMER(NwkSKey, 16, 'r', configvalueAddr(4));
      EEPROMER(AppSKey, 16, 'r', configvalueAddr(20));
      Session_Data = {NwkSKey, AppSKey, Address_Tx, &Frame_Counter_Tx};
      EEPROMER(DevEUI, 8, 'r', configvalueAddr(60));
      EEPROMER(AppEUI, 8, 'r', configvalueAddr(52));
      EEPROMER(AppKey, 16, 'r', configvalueAddr(36));
      OTAA_Data = {DevEUI, AppEUI, AppKey, DevNonce, AppNonce, NetID};

      Data_Tx[0] = 0x24;
      Buffer_Tx.Data = Data_Tx;//{Data_Tx, 0x00};
      transmitpacket(&Buffer_Tx);

      //ABP/OTAA
      if(abporotaa[0] == 0x00)//ABP
      {
        RFM_Command_Status = NEW_RFM_COMMAND;
        //Type A mote transmit receive cycle
       // if((RFM_Command_Status == NEW_RFM_COMMAND || RFM_Command_Status == JOIN) && LoRa_Settings.Mote_Class == 0x00)
        //{
          //LoRa cycle
          
          LORA_Cycle(&Buffer_Tx, &Buffer_Rx, &RFM_Command_Status, &Session_Data, &OTAA_Data, &Message_Rx, &LoRa_Settings);
    
          RFM_Command_Status = NO_RFM_COMMAND;
       // }
      }else///////////////rq OTAA
      {
          //Serial.println("in OTAA....");
          //Type C mote transmit and receive handling
          if(LoRa_Settings.Mote_Class == 0x01)
          {
            if(RFM_Command_Status == JOIN)
            {
              //Start join precedure
              LoRa_Send_JoinReq(&OTAA_Data, &LoRa_Settings);
      
              //Clear RFM_Command
              RFM_Command_Status = NO_RFM_COMMAND;
            }
      
            //Transmit
            if(RFM_Command_Status == NEW_RFM_COMMAND)
            {
              //Lora send data
              LORA_Send_Data(&Buffer_Tx, &Session_Data, &LoRa_Settings);
      
              RFM_Command_Status = NO_RFM_COMMAND;
            }
      
            //Receive
            if(digitalRead(DIO0) == HIGH)
            {
              //Get data
              LORA_Receive_Data(&Buffer_Rx, &Session_Data, &OTAA_Data, &Message_Rx, &LoRa_Settings);
      
              Rx_Status = NEW_RX;
            }
          }
          
        
      }
  
      //If there is new data
      if(Rx_Status == NEW_RX)
      {
      //Check if there is data in the received message
          if(Buffer_Rx.Counter != 0x00)
      {
        UART_Send_Data(Buffer_Rx.Data,Buffer_Rx.Counter);
      }
          else
          {
            Serial.write("No data");
          }
  
          UART_Send_Newline();
          UART_Send_Newline();
  
          Rx_Status = NO_RX;
      }
    Tx_done = true; //done transmitting;
    //count_enable = true;
    sei();

    //  cleartimetotransmit();
      
      //j1s 00000000
  }
