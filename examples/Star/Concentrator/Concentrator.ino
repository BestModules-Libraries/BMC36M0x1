/*****************************************************************
File:         Concentrator.ino
Description: 
              1. Press KEY1(D8) on both modules to enter the pairing mode: pairing time is 10 seconds, then the MRX indicator flashes;
              2. Press KEY3(D10) on the Node end to send a pairing request packet. If the pairing is successful, the MRX indicator is off, 
                 and the Serial Monitor on the Node end and Concentrator end shows that the pairing is successful.
              3.KEY2(D9) is used to switch between RX mode and Sleep mode. Press KEY2(D9) to switch. When the module is in RX mode,
                the MRX indicator will be steady on. When the module is in Sleep mode, the MRX indicator will be off. 
                After the two boards are paired successfully, use KEY2(D9) to make the sender enter RX mode and the receiver enter Sleep mode.
              4.Node sends data to Concentrator: Users can send data packets by pressing KEY3(D10) on Node and MTX indicator flashes once to indicate 
                that data packets are being sent.The Concentrator displays the received data on the Serial Monitor when it is received.
              5.Concentrator sends data to Node: Users can send data packets by pressing KEY3 to KEY4(D10 to D11) on Concentrator in the matching sequence. 
                The MTX indicator on Concentrator flashes once to indicate that data packets are sent. When receiving data, the Node displays 
                the received data on the Serial Monitor.        
Note:  rxPin:D5 txPin:D4  
******************************************************************/

#include  "BMC36M0x1.h"

BMC36M0x1     BMC36(5,4);         //rxPin,txPin,Please comment out this line of code if you don't use SW Serial
//BMC36M0x1     BMC36(&Serial);     //Please uncomment out this line of code if you use HW Serial on BMduino
//BMC36M0x1     BMC36(&Serial1);      //Please uncomment out this line of code if you use HW Serial1 on BMduino
//BMC36M0x1     BMC36(&Serial2);    //Please uncomment out this line of code if you use HW Serial2 on BMduino
//BMC36M0x1     BMC36(&Serial3);    //Please uncomment out this line of code if you use HW Serial3 on BMduino
//BMC36M0x1     BMC36(&Serial4);    //Please uncomment out this line of code if you use HW Serial4 on BMduino

#define   KEY1_Pin (8)
#define   KEY2_Pin (9)
#define   KEY3_Pin (10)
#define   KEY4_Pin (11)
bool Flag_Pairing,Flag_PairSuccess;
uint16_t   shortAddr[2] = {0};
uint8_t number = 0;
uint8_t TXDATA[16] = {0},RXDATA[32] = {0};
uint8_t DATA,STATUS,len;
uint8_t Flag_RX = 1;
uint8_t keyFlag = 0;
uint8_t keyFlagTemp = 0;

uint8_t Sys_KEY(void);
void RFMessage_Process();
void Handle_RFPacket_Process();
void setup()
{
  pinMode(KEY1_Pin, INPUT_PULLUP);  
  pinMode(KEY2_Pin, INPUT_PULLUP); 
  pinMode(KEY3_Pin, INPUT_PULLUP); 
  pinMode(KEY4_Pin, INPUT_PULLUP); 
  Serial.begin(115200); 
  BMC36.begin(BDR_38400);          //set BMduino and BMC36 uart baudrate 
  BMC36.setDeviceRole(Con_of_Star);      
}

void loop() 
{              
 RFMessage_Process();                        
 Handle_RFPacket_Process();      
}


/******************************************************
Description: Scan KEY         
Input:  none            
Output: none         
Return: KEY Value         
Others: put this funtion to loop,circularly Scan KEY         
*******************************************************/
uint8_t Sys_KEY(void)
{
  uint8_t result=0;
  
  if(!digitalRead(KEY1_Pin))
    keyFlagTemp |= 0x01;
  else if(!digitalRead(KEY2_Pin))
    keyFlagTemp |= 0x02;
  else if(!digitalRead(KEY3_Pin))
    keyFlagTemp |= 0x04;
  else if(!digitalRead(KEY4_Pin))
    keyFlagTemp |= 0x08;
  else
    keyFlagTemp = 0x00;
    
  if((keyFlagTemp!=keyFlag) && (keyFlag==0))
  {
    delay(10);
    if(!digitalRead(KEY1_Pin))
      result = 0x01;
    else if(!digitalRead(KEY2_Pin))
      result = 0x02;
    else if(!digitalRead(KEY3_Pin))
      result = 0x03;
    else if(!digitalRead(KEY4_Pin))
      result = 0x04;
  }
  keyFlag = keyFlagTemp;
  
  return result;
}


/******************************************************
Description: Send commands according to keyvalue        
Input:  none            
Output: none         
Return: none         
Others: put this funtion to loop         
*******************************************************/
void RFMessage_Process()
{
  switch(Sys_KEY())
  { 
   case 0x01: 
      BMC36.setMode(Pairing_Mode);                                                 //send pairing mode command        
      Flag_Pairing = TRUE; 
      Flag_PairSuccess = FALSE;  
    break;
          
   case 0x02:   
      if(Flag_RX ==1)
         {
          BMC36.setMode(Rx_Mode);
          Flag_RX =0;
         } 
         else if(Flag_RX ==0)
         {
          BMC36.setMode(Sleep_Mode);
          Flag_RX =1;
         }     
      break;
      
   case 0x03:     
      if(Flag_PairSuccess)     
       {
        for(uint8_t temp=0;temp<16;temp++)
         {
          TXDATA[temp] = DATA++;
         }
        BMC36.writeRFData(shortAddr[0],16,TXDATA);            //Send  data packet after the pairing is successful  
       }
      break;   

    case 0x04:     
      if(Flag_PairSuccess)     
       {
        for(uint8_t temp=0;temp<16;temp++)
         {
          TXDATA[temp] = DATA++;
         }
        BMC36.writeRFData(shortAddr[1],16,TXDATA);            //Send  data packet after the pairing is successful  
       }
      break;   
  }       
}

/******************************************************
Description: dispose RF Packet         
Input:  none            
Output: none         
Return: none         
Others: put this funtion to loop         
*******************************************************/
void Handle_RFPacket_Process()
{ 
    if(Flag_Pairing)
      {
       STATUS = BMC36.getPairStatus(); 
       if(STATUS == 1) //pairing success
        {
          Flag_Pairing = FALSE;
          Flag_PairSuccess = TRUE;
          shortAddr[number] = BMC36.getShortAddress();
          number++;
          Serial.println("Pair Success");                                                             
        }  
       if(STATUS == 2)  //pairing fail          
        {
          Flag_Pairing = FALSE;
          Flag_PairSuccess = FALSE;
        }   
        if(STATUS == 3) //pairing timout
        {
          Flag_Pairing = FALSE;
          Flag_PairSuccess = FALSE;                                                           
        }                                              
      }
   
      if(Flag_PairSuccess)
      {
      if(BMC36.isInfoAvailable())  
      {
       STATUS = BMC36.readRFData(RXDATA,len);  
       if(STATUS == 1)                    //After the match is successful, data is received
        { 
         Serial.print("RXDATA[]:");
         for(uint8_t temp=0;temp<len;temp++)
         {
          Serial.print(RXDATA[temp],HEX);
          Serial.print(" ");
         }
         Serial.println(" ");
        }
      }
     }
}
