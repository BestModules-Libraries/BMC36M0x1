
/***********************************************************
File:           BMC36M0x1.cpp
Author:         BESTMODULES
Description:    UART communication with the BMC36M0x1   
Version:        V1.0.2   --2023-09-06
***********************************************************/
#include  "BMC36M0x1.h"

/**********************************************************
Description: Constructor
Parameters:  *theSerial: Wire object if your board has more than one UART interface      
                         parameter range:&Serial、&Serial1、&Serial2、&Serial3、&Serial4
Return:          
Others:     
**********************************************************/
BMC36M0x1::BMC36M0x1(HardwareSerial *theSerial)
{
     _softSerial = NULL;
     _hardSerial = theSerial;
}

/**********************************************************
Description: Constructor
Parameters:  rxPin : Receiver pin of the UART
             txPin : Send signal pin of UART         
Return:          
Others:   
**********************************************************/
BMC36M0x1::BMC36M0x1(uint8_t rxPin,uint8_t txPin)
{
    _hardSerial = NULL;
    _rxPin = rxPin;
    _txPin = txPin;
    _softSerial = new SoftwareSerial(_rxPin,_txPin);   
}

/**********************************************************
Description: Initialize in uart mode
Parameters:  baud: 
                 BDR_9600:9600bps
                 BDR_19200:19200bps
                 BDR_38400:38400bps     
Return:          
Others:      If the _softSerial pointer is empty, the software serial port is initialized,
             otherwise the hardware serial port is initialized.      
**********************************************************/
void BMC36M0x1::begin(uint8_t baud)
{
   if(_softSerial != NULL)
    {
      _softSerial->begin(38400);
      setBaudRate(baud);
    }
     else
    {
     _hardSerial->begin(38400);  
     setBaudRate(baud);
    }
}

/**********************************************************
Description: Gets the module pairing status
Parameters:  
Return:   
        TRUE: paired    
        FALSE: nopaired
Others:           
**********************************************************/
bool BMC36M0x1::isPaired()
{
   uint8_t sendBuf[4] = {0x01, 0x99, 0x00};
   uint8_t buff[6] = {0};
   sendBuf[3] = checksum(3,sendBuf);
   writeBytes(sendBuf,4);
   if(readBytes(buff,6)== CHECK_OK)
   {
    if(buff[4]==1)
    {
     return  TRUE;
    }
    return  FALSE;
   } 
   return FALSE;
}

/**********************************************************
Description: Send the data as an Pair Package
Parameters:  shortAddr: short address
Return:  
Others:      0: The command is executed successfully. 1: Command execution fails.
             2: The command is not supported. 3: Format error; 4: The data is too long.    
**********************************************************/
uint8_t BMC36M0x1::writePairPackage(uint32_t shortAddr)
{
   uint8_t sendBuf[37] = {0};
   sendBuf[0] = 0x10;
   sendBuf[1] = 0x00;
   sendBuf[2] = 4;
   sendBuf[3] = (uint8_t)(shortAddr>>8);
   sendBuf[4] = (uint8_t)shortAddr;
   sendBuf[5] = 0x55;
   sendBuf[6] = 0xAA;
   sendBuf[7] = checksum(7,sendBuf);
   uint8_t buff[4] = {0};
   writeBytes(sendBuf,8);
   if(readBytes(buff,5)== CHECK_OK)
    {
     return buff[3];
    }
   else 
    {
      return 1; 
    }
}

/**********************************************************
Description: get Pair Status
Parameters:  
Return:      0: Pairing 
             1: Pair Success
             2: pair fail   
             3: pair timout
Others:           
**********************************************************/
uint8_t BMC36M0x1::getPairStatus()
{
  uint8_t datalength = 0;
  uint8_t dataBuff[50] = {0};
  uint8_t flag = 0;
  uint8_t a;
  if(_softSerial != NULL)
    {
      if(_softSerial->available() > 5)
      {
        delay(100);
        datalength = _softSerial->available();
        if(readBytes(dataBuff,datalength)== CHECK_OK)
         {
          flag = 1;
         }
      }
      else 
      {
        flag = 0;
      }
    }
   else
    {
      if(_hardSerial->available() > 5)
      {
        delay(100);
        datalength = _hardSerial->available();
        if(readBytes(dataBuff,datalength)== CHECK_OK)
         {
          flag = 1;
         }
      }
      else 
      {
        flag = 0;
      }
    }
    
   if( flag==1) 
   {
    if(dataBuff[0] == 0x40)
    {
      _nodeShortAddr = ((dataBuff[3]*256)+dataBuff[4]);
      _nodeShortAddr = (_nodeShortAddr & 0xff3f);
      return 1;
    }
    else if(dataBuff[0] == 0x04)
          {
           if((dataBuff[3]==0x05))
            {
             return 2;
            }
            if((dataBuff[3]==0x06))
            {
             return 3;
            }
          }
     else
     {
      return FALSE;
     }
   }
  return FALSE;
}

/**********************************************************
Description: Obtain the RF communication short address
Parameters:  
Return:      _nodeShortAddr: short address
Others:           
**********************************************************/
uint16_t BMC36M0x1::getShortAddress()
{
   return _nodeShortAddr;
}

/**********************************************************
Description: Obtain the RF communication short address
Parameters:  shrotAddress[10]:max of 10 groups can be read from Con_of_Star, other role can be read 1 group shortaddress
Return:      quantity of groups
Others:           
**********************************************************/
uint8_t BMC36M0x1::getShortAddress(uint16_t shrotAddress[])
{
   uint8_t sendBuf[4] = {0x01, 0x85, 0x00};
   uint8_t buff[24] = {0};
   uint8_t datalen=0;
   uint8_t groups=0;
   uint8_t a;
   sendBuf[3] = checksum(3,sendBuf);
   writeBytes(sendBuf,4);
   readBytes(buff,3);
   datalen = buff[2];
   readBytes(buff+3,datalen+1);
   if(checksum(datalen+4,buff)==CHECK_OK)
   {
      groups = buff[4];
      for(a=0;a<groups;a++)
      {
        shrotAddress[a] = buff[5+(2*a)+1]+(buff[5+(2*a)]<<8);
      }
   }
    return groups;
}
/**********************************************************
Description: Send the data as an RF packet
Parameters:  shortAddr: short address
             len: data length(=<60)
             data[]: The starting store to send data
Return:  
Others:           
**********************************************************/
uint8_t BMC36M0x1::writeRFData(uint32_t shortAddr,uint8_t len,uint8_t data[])
{
   uint8_t sendBuf[37] = {0};
   sendBuf[0] = 0x10;
   sendBuf[1] = 0x00;
   sendBuf[2] = (len+2);
   sendBuf[3] = (uint8_t)(shortAddr>>8);
   sendBuf[4] = (uint8_t)shortAddr;
   for(uint8_t i=0;i<len;i++)
   {
    sendBuf[i+5] = data[i];
   }
   sendBuf[len+5] = checksum(len+5,sendBuf);
   uint8_t buff[4] = {0};
   writeBytes(sendBuf,(len+6));
   if(readBytes(buff,5)== CHECK_OK)
    {
     return buff[3];
    }
   else 
    {
      return 1; 
    }
}

/**********************************************************
Description: Query whether the packets sent by the module are received
Parameters:    
Return:      FALSE: No data received
             TRUE: Data received  
Others:           
**********************************************************/
bool BMC36M0x1::isInfoAvailable()
{
  if(_softSerial != NULL)
    {
      if(_softSerial->available() > 3)
      {
        return  true;
      }
      else 
      {
        return   false;
      }
    }
   else
    {
      if(_hardSerial->available() > 3)
      {
        return   true;
      }
      else 
      {
        return  false;
      }
    }
}

/**********************************************************
Description: Read the data inside the RF packet
Parameters:  rxData[]: Used to store packets 
             &len :  Used to store packets
Return:      0: No packets received
             1: received data  
Others:           
**********************************************************/
uint8_t BMC36M0x1::readRFData(uint8_t rxData[],uint8_t &len)
{
  uint8_t datalength = 0;
  uint8_t dataBuff[50] = {0};
  uint8_t flag = 0;
  if(_softSerial != NULL)
    {
      readBytes(dataBuff,3);
      datalength = dataBuff[2];
      len = dataBuff[2]-2;
      readBytes(dataBuff+3,datalength+1);
      if(checksum(datalength+4,dataBuff)==CHECK_OK)
       {
          flag = 1;
       }
    }
   else
    {
      readBytes(dataBuff,3);
      datalength = dataBuff[2];
      len = dataBuff[2]-2;
      readBytes(dataBuff+3,datalength+1);
      if(checksum(datalength+4,dataBuff)==CHECK_OK)
       {
          flag = 1;
       }
    }
    
    if(dataBuff[0] == 0x40 && flag == 1)
     {
      for(int i=0;i<datalength-2;i++)
      {
       rxData[i]= dataBuff[i+5];
      } 
      return 1;
     }
   return FALSE;  
}

/**********************************************************
Description: Gets the current signal strength
Parameters:  
Return:   
        rssi: Current signal strength
Others:           
**********************************************************/
uint8_t BMC36M0x1::getRSSI()
{
   uint8_t sendBuf[4] = {0x01, 0x95, 0x00};
   uint8_t buff[6] = {0};
   uint8_t rssi = 0;
   sendBuf[3] = checksum(3,sendBuf);
   writeBytes(sendBuf,4);
   if(readBytes(buff,6)== CHECK_OK)
   {
    rssi = buff[4];
   } 
   return rssi;
}

/**********************************************************
Description: Gets the signal strength at the time the packet is received
Parameters:  
Return:   
        pktRSSI: The signal strength when the packet is received
Others:           
**********************************************************/
uint8_t BMC36M0x1::getPktRSSI()
{
   uint8_t sendBuf[4] = {0x01, 0x96, 0x00};
   uint8_t buff[6] = {0};
   uint8_t pktRSSI = 0;
   sendBuf[3] = checksum(3,sendBuf);
   writeBytes(sendBuf,4);
   if(readBytes(buff,6)== CHECK_OK)
   {
    pktRSSI = buff[4];
   } 
   return pktRSSI;
}


/**********************************************************
Description: Written information
Parameters:  len: length of the deviInfo[] 
             deviInfo[]:Information that needs to be written(=<32bytes) 
Return:      0: The command is executed successfully. 1: Command execution fails.
             2: The command is not supported. 3: Format error; 4: The data is too long.
             5: The pairing fails. 6: pairing timeout; 7: failed to send. 8: The message is sent successfully.    
Others:           
**********************************************************/
uint8_t BMC36M0x1::writeEEPROM(uint8_t len,uint8_t deviInfo[])
{
   uint8_t sendBuf[37] = {0};
   sendBuf[0] = 0x01;
   sendBuf[1] = 0x03;
   sendBuf[2] = len;
   for(uint8_t i=0;i<len;i++)
   {
    sendBuf[i+3] = deviInfo[i];
   }
   uint8_t buff[4] = {0};
   sendBuf[len+3] = checksum(len+3,sendBuf);
   writeBytes(sendBuf,(len+4));
   if(readBytes(buff,5) == CHECK_OK)
     return buff[2];
   else 
     return 1; 
}

/**********************************************************
Description: Get information about the module
Parameters:  deviInfo[]: Used to store the obtained information (=<32bytes)
             len: Valid information data length  
Return:      0: The command is executed successfully. 1: Command execution fails.
             2: The command is not supported. 3: Format error; 4: The data is too long.
             5: The pairing fails. 6: pairing timeout; 7: failed to send. 8: The message is sent successfully.    
Others:                    
**********************************************************/
uint8_t BMC36M0x1::readEEPROM(uint8_t deviInfo[],uint8_t &len)
{
  uint8_t sendBuf[4] = {0x01, 0x83, 0x00};
  uint8_t buff[36] = {0};
  uint8_t leng = 0;
  sendBuf[3] = checksum(3,sendBuf);
  writeBytes(sendBuf,4);
   delay(40);
   if(_softSerial != NULL)
    {
     leng = _softSerial->available(); 
    } 
    else
    {
     leng = _hardSerial->available();  
    }
   if(readBytes(buff,leng) == CHECK_OK)
    {
     len = (buff[2]-1);
     for(uint8_t i=0;i<(buff[2]-1);i++)
      {
       deviInfo[i] = buff[i+4];
      }
      return buff[3];
    } 
   else 
     return 1;
}

/**********************************************************
Description: Gets the version number
Parameters:  number[]: Used to store the obtained version number  (16bytes))
       
Return:      0: The command is executed successfully. 1: Command execution fails.
             2: The command is not supported. 3: Format error; 4: The data is too long.
             5: The pairing fails. 6: pairing timeout; 7: failed to send. 8: The message is sent successfully.    
Others:           
**********************************************************/
uint8_t BMC36M0x1::getFWVer(uint8_t number[])
{
   uint8_t sendBuf[4] = {0x01, 0x80, 0x00};
   uint8_t buff[21] = {0};
   uint8_t leng = 0;
   sendBuf[3] = checksum(3,sendBuf);
   writeBytes(sendBuf,4);
   if(readBytes(buff,21)== CHECK_OK)
    {
     for(uint8_t i=0;i<16;i++)
      {
       number[i] = buff[i+4];
      }
      return buff[3];
    } 
    return 1;
}

/**********************************************************
Description: Get the ID number
Parameters:  id[]: Used to store the obtained ID number (4bytes)
Return:      0: The command is executed successfully. 1: Command execution fails.
             2: The command is not supported. 3: Format error; 4: The data is too long.
             5: The pairing fails. 6: pairing timeout; 7: failed to send. 8: The message is sent successfully.    
Others:           
**********************************************************/
uint8_t BMC36M0x1::getSN(uint8_t id[])
{
   uint8_t sendBuf[4] = {0x01, 0x82, 0x00};
   uint8_t buff[9] = {0};
   sendBuf[3] = checksum(3,sendBuf);
   writeBytes(sendBuf,4);
   if(readBytes(buff,9) == CHECK_OK)
     {
      id[0] = buff[4];
      id[1] = buff[5];
      id[2] = buff[6];
      id[3] = buff[7];
      return buff[3];
     }
    return 1;
}

/**********************************************************
Description: Gets the device role
Parameters:  
Return:      0:Peer  
             1:Node_of_Star
             2:Con_of_Star
Others:           
**********************************************************/
uint8_t BMC36M0x1::getDeviceRole()
{
   uint8_t sendBuf[4] = {0x01, 0x90, 0x00};
   uint8_t buff[6] = {0};
   uint8_t deviceRole = 0;
   sendBuf[3] = checksum(3,sendBuf);
   writeBytes(sendBuf,4);
  if(readBytes(buff,6)== CHECK_OK)
   {
    deviceRole = buff[4];
   }
   return deviceRole;
}

/**********************************************************
Description: Gets the module operating mode
Parameters:  
Return:      0:DeepSleep_Mode 
             1:LightnSleep_Mode
             2:Rx_Mode
             3:Pairing_Mode
Others:           
**********************************************************/
uint8_t BMC36M0x1::getMode()
{
   uint8_t sendBuf[4] = {0x01, 0x91, 0x00};
   uint8_t buff[6] = {0};
   uint8_t mode = 0;
   sendBuf[3] = checksum(3,sendBuf);
   writeBytes(sendBuf,4);
   if(readBytes(buff,6)== CHECK_OK)
   {
    mode = buff[4];
   }
   return mode;
}

/**********************************************************
Description: Get the RF Frequency Band
Parameters:  
                           
Return:      frequencyBand:
                           315000000：315MHZ
                           433920000：433MHz 
                           868350000：868MHZ
                           915000000：915MHZ    
Others:        
**********************************************************/
unsigned long BMC36M0x1::getFrequency()
{
   uint8_t sendBuf[4] = {0x01, 0x92, 0x00};
   uint8_t buff[9] = {0};
   unsigned long frequency = 0;
   sendBuf[3] = checksum(3,sendBuf);
   writeBytes(sendBuf,4);
   if(readBytes(buff,9) == CHECK_OK)
     {
      frequency = (buff[7]*16777216+buff[6]*65536+buff[5]*256+buff[4]);
     }
   return frequency;
}

/**********************************************************
Description: Gets  RF transmit power
Parameters:  
Return:   
          0X01:0dBm 
          0X02:5dBm 
          0X03:10dBm
          0X04:13dBm  
Others:           
**********************************************************/
uint8_t BMC36M0x1::getRFPower()
{
   uint8_t sendBuf[4] = {0x01, 0x93, 0x00};
   uint8_t buff[6] = {0};
   uint8_t rfPower = 0;
   sendBuf[3] = checksum(3,sendBuf);
   writeBytes(sendBuf,4);
   if(readBytes(buff,6)== CHECK_OK)
   {
    rfPower = buff[4];
   }
   return rfPower;
}

/**********************************************************
Description: Gets the RF communication rate
Parameters:  
Return:   
       0x02:10Kbps 
       0x03:25Kbps
       0x04:50Kbps
       0x05:125Kbps
       0x06:250Kbps   
Others:           
**********************************************************/
uint8_t BMC36M0x1::getDataRate()
{
   uint8_t sendBuf[4] = {0x01, 0x94, 0x00};
   uint8_t buff[6] = {0};
   uint8_t dataRate = 0;
   sendBuf[3] = checksum(3,sendBuf);
   writeBytes(sendBuf,4);
   if(readBytes(buff,6)== CHECK_OK)
   {
    dataRate = buff[4];
   }
   return dataRate;
}

/**********************************************************
Description: Gets the UART communication baud rate
Parameters: 
Return:      0: 9600bps
             1: 19200bps
             2: 38400bps  
Others:           
**********************************************************/
uint8_t BMC36M0x1::getBaudRate()
{
   uint8_t sendBuf[4] = {0x01, 0x81, 0x00};
   uint8_t buff[6] = {0};
   uint8_t baudRate = 0;
   sendBuf[3] = checksum(3,sendBuf);
   writeBytes(sendBuf,4);
   if(readBytes(buff,6)== CHECK_OK)
   {
    baudRate = buff[4];
   }
   return baudRate;
}

/**********************************************************
Description: Set device roles
Parameters:  role:
                  Peer: Peer Mode  
                  Node_of_Star: Star Mode(Node)
                  Concentrator_of_Star: Star Mode(Concentrator)
Return:      0: The command is executed successfully. 1: Command execution fails.
             2: The command is not supported. 3: Format error; 4: The data is too long.
             5: The pairing fails. 6: pairing timeout; 7: failed to send. 8: The message is sent successfully.     
Others:           
**********************************************************/
uint8_t BMC36M0x1::setDeviceRole(uint8_t role)
{
   uint8_t sendBuf[5] = {0x01, 0x10, 0x01, 0x00};
   sendBuf[3] = role;
   uint8_t buff[4] = {0};
   sendBuf[4] = checksum(4,sendBuf);
   writeBytes(sendBuf,5);
   if(readBytes(buff,5)== CHECK_OK)
     return buff[3];
   else 
     return 1; 
}

/**********************************************************
Description: Sets the current operating mode
Parameters:  mode:
                   DeepSleep_Mode: Set to deep sleep mode
                   LightSleep_Mode: Set to light sleep mode
                   Rx_Mode: Set to receive mode
                   Pairing_Mode: Set to pairing mode
Return:      0: The command is executed successfully. 1: Command execution fails.
             2: The command is not supported. 3: Format error; 4: The data is too long.
             5: The pairing fails. 6: pairing timeout; 7: failed to send. 8: The message is sent successfully.     
Others:           
**********************************************************/
uint8_t BMC36M0x1::setMode(uint8_t mode)
{
   uint8_t sendBuf[5] = {0x01, 0x11, 0x01, 0x00};
   sendBuf[3] = mode;
   uint8_t buff[4] = {0};
   sendBuf[4] = checksum(4,sendBuf);
   writeBytes(sendBuf,5);
   if(readBytes(buff,5)== CHECK_OK)
     return buff[3];
   else 
     return 1;
}

/**********************************************************
Description: Configuring the RF Frequency Band
Parameters:  frequencyBand：
                           315000000：315MHZ
                           433920000：433MHz 
                           868350000：868MHZ
                           915000000：915MHZ
Return:      0: The command is executed successfully. 1: Command execution fails.
             2: The command is not supported. 3: Format error; 4: The data is too long.
             5: The pairing fails. 6: pairing timeout; 7: failed to send. 8: The message is sent successfully.    
Others:        
**********************************************************/
uint8_t BMC36M0x1::setFrequency(unsigned long frequency)
{
   uint8_t sendBuf[8] = {0x01, 0x12, 0x04, 0x00, 0x00, 0x00, 0x00};
   sendBuf[3] = (frequency)&0xff;
   sendBuf[4] = (frequency>>8)&0xff;
   sendBuf[5] = (frequency>>16)&0xff;
   sendBuf[6] = (frequency>>24)&0xff;
   uint8_t buff[4] = {0};
   sendBuf[7] = checksum(7,sendBuf);
   writeBytes(sendBuf,8);
   if(readBytes(buff,5)== CHECK_OK)
     return buff[3];
   else 
     return 1;
}

/**********************************************************
Description: Set the RF transmit power
Parameters:  power:
                    RF_0dBm:0dBm 
                    RF_5dBm:5dBm 
                    RF_10dBm:10dBm
                    RF_13dBm:13dBm         
Return:      0: The command is executed successfully. 1: Command execution fails.
             2: The command is not supported. 3: Format error; 4: The data is too long.
             5: The pairing fails. 6: pairing timeout; 7: failed to send. 8: The message is sent successfully.     
Others:           
**********************************************************/
uint8_t BMC36M0x1::setRFPower(uint8_t power)
{
   uint8_t sendBuf[5] = {0x01, 0x13, 0x01, 0x00};
   sendBuf[3] = power;
   uint8_t buff[4] = {0};
   sendBuf[4] = checksum(4,sendBuf);
   writeBytes(sendBuf,5);
   if(readBytes(buff,5)== CHECK_OK)
     return buff[3];
   else 
     return 1;
}

/**********************************************************
Description: Set the RF communication rate
Parameters:  rate:
                  RF_10kbps: 10Kbps 
                  RF_25kbps: 25Kbps
                  RF_50kbps: 50Kbps
                  RF_125kbps: 125Kbps
                  RF_250kbps: 250Kbps     
Return:      0: The command is executed successfully. 1: Command execution fails.
             2: The command is not supported. 3: Format error; 4: The data is too long.
             5: The pairing fails. 6: pairing timeout; 7: failed to send. 8: The message is sent successfully.    
Others:           
**********************************************************/
uint8_t BMC36M0x1::setDataRate(uint8_t rate)
{
   uint8_t sendBuf[5] = {0x01, 0x14, 0x01, 0x00};
   sendBuf[3] = rate;
   uint8_t buff[4] = {0};
   sendBuf[4] = checksum(4,sendBuf);
   writeBytes(sendBuf,5);
   if(readBytes(buff,5) == CHECK_OK)
     return buff[3];
   else 
     return 1;
}

/**********************************************************
Description: Set the RF communication address
Parameters:  address[]: The address that needs to be set(4bytes)  
Return:      0: The command is executed successfully. 1: Command execution fails.
             2: The command is not supported. 3: Format error; 4: The data is too long.
             5: The pairing fails. 6: pairing timeout; 7: failed to send. 8: The message is sent successfully.    
Others:           
**********************************************************/
uint8_t BMC36M0x1::setRFAddress(uint8_t address[])
{
   uint8_t sendBuf[8] = {0x01, 0x04, 0x04, 0x00, 0x00, 0x00, 0x00};
   sendBuf[3] = address[0];
   sendBuf[4] = address[1];
   sendBuf[5] = address[2];
   sendBuf[6] = address[3];
   uint8_t buff[4] = {0};
   sendBuf[7] = checksum(7,sendBuf);
   writeBytes(sendBuf,8);
   if(readBytes(buff,5)== CHECK_OK)
     return buff[3];
   else 
     return 1; 
}
/**********************************************************
Description: Set the UART communication baud rate
Parameters:  baudRate:
                      BDR_9600: 9600bps
                      BDR_19200: 19200bps
                      BDR_38400: 38400bps     
Return:      0: The command is executed successfully. 1: Command execution fails.
             2: The command is not supported. 3: Format error; 4: The data is too long.
             5: The pairing fails. 6: pairing timeout; 7: failed to send. 8: The message is sent successfully.    
Others:                        
**********************************************************/
uint8_t BMC36M0x1::setBaudRate(uint8_t baudRate)
{
   uint32_t BR_Parameter[3] = {9600,19200,38400};
   uint8_t sendBuf[5] = {0x01, 0x01, 0x01, 0x00};
   sendBuf[3] = baudRate;
   uint8_t buff[4] = {0};
   sendBuf[4] = checksum(4,sendBuf);
   writeBytes(sendBuf,5);
   if(readBytes(buff,5) == CHECK_OK)
    {
      if(_softSerial != NULL)
     {
      _softSerial->begin(BR_Parameter[baudRate]);
     }
     else
     {
     _hardSerial->begin(BR_Parameter[baudRate]);  
     }
      return buff[3];
    }
   else 
     {
      return 1; 
     }
}

/**********************************************************
Description: Obtain the RF communication address
Parameters:  address[]: Used to store the obtained Mailing address (4bytes)
Return:      0: The command is executed successfully. 1: Command execution fails.
             2: The command is not supported. 3: Format error; 4: The data is too long.
             5: The pairing fails. 6: pairing timeout; 7: failed to send. 8: The message is sent successfully.    
Others:                 
**********************************************************/
uint8_t BMC36M0x1::getRFAddress(uint8_t address[])
{
   uint8_t sendBuf[4] = {0x01, 0x84, 0x00};
   uint8_t buff[9] = {0};
   sendBuf[3] = checksum(3,sendBuf);
   writeBytes(sendBuf,4);
   if(readBytes(buff,9) == CHECK_OK)
     {
      address[0] = buff[4];
      address[1] = buff[5];
      address[2] = buff[6];
      address[3] = buff[7];
      return buff[3];
     }
      return 1; 
}

/**********************************************************
Description: 
Parameters:  
Return:  
Others:           
**********************************************************/
uint8_t BMC36M0x1::checksum(uint8_t len,uint8_t data[])
{
  uint8_t a=0;
  uint8_t result=0;
  
  for(a=0;a<len;a++)
    result  += data[a];
  result = ~result+1;
  
  return result;
}

/**********************************************************
Description: writeBytes
Parameters:  wbuf[]:Variables for storing Data to be sent
             wlen:Length of data sent  
Return:   
Others:
**********************************************************/
void BMC36M0x1::writeBytes(uint8_t wbuf[], uint8_t wlen)
{
  /* Select SoftwareSerial Interface */
  if (_softSerial != NULL)
  {
    while (_softSerial->available() > 0)
    {
      _softSerial->read();
    }
    _softSerial->write(wbuf, wlen);
  }
  /* Select HardwareSerial Interface */
  else
  {
    while (_hardSerial->available() > 0)
    {
      _hardSerial->read();
    }
    _hardSerial->write(wbuf, wlen);
  }
}

/**********************************************************
Description: readBytes
Parameters:  rbuf[]:Variables for storing Data to be obtained
             rlen:Length of data to be obtained
Return:   
Others:
**********************************************************/
uint8_t BMC36M0x1::readBytes(uint8_t rbuf[], uint8_t rlen, uint16_t timeOut)
{
  uint8_t i = 0, delayCnt = 0, checkSum = 0, checkSum1 = 0;
/* Select SoftwareSerial Interface */
  if (_softSerial != NULL)
  {
    for (i = 0; i < rlen; i++)
    {
      delayCnt = 0;
      while (_softSerial->available() == 0)
      {
        if (delayCnt > timeOut)
        {
          return TIMEOUT_ERROR; // Timeout error
        }
        delay(1);
        delayCnt++;
      }
      rbuf[i] = _softSerial->read();
    }
  }
/* Select HardwareSerial Interface */
  else
  {
    for (i = 0; i < rlen; i++)
    {
      delayCnt = 0;
      while (_hardSerial->available() == 0)
      {
        if (delayCnt > timeOut)
        {
          return TIMEOUT_ERROR; // Timeout error
        }
        delay(1);
        delayCnt++;
      }
      rbuf[i] = _hardSerial->read();
    }
  }

  /* check Sum */
  for (i = 0; i < (rlen - 1); i++)
  {
    checkSum += rbuf[i];
  }
  checkSum = ((~checkSum)+1);
  if (checkSum == rbuf[rlen - 1])
  {
    return CHECK_OK; // Check correct
  }
  else
  {
    return CHECK_ERROR; // Check error
  }
}
