/***********************************************************
File:               BMC36M0x1.h
Author:             XIAO, BESTMODULES
Description:        Define classes and required variables
History:      
V1.0.0   -- initial version；2023-07-19；Arduino IDE : v1.8.16
***********************************************************/
#ifndef _BMC36M0x1_H_
#define _BMC36M0x1_H_

#include "Arduino.h"
#include <SoftwareSerial.h>

#define TRUE 1
#define FALSE 0
#define   CHECK_OK        0
#define   CHECK_ERROR     1
#define   TIMEOUT_ERROR   2
#define   SUCCESS         0
#define   FAIL            1
#define   BDR_9600        0
#define   BDR_19200       1
#define   BDR_38400       2

class BMC36M0x1
{
   public:   
      BMC36M0x1(HardwareSerial *theSerial = &Serial);
      BMC36M0x1(uint8_t rxPin,uint8_t txPin);
      void begin(uint8_t baud = BDR_38400);   
      bool isPaired();              
      uint8_t writePairPackage(uint32_t shortAddr);
      uint8_t getPairStatus();
      uint16_t getShortAddress();
      uint8_t getShortAddress(uint16_t shrotAddress[]);
      uint8_t writeRFData(uint32_t shortAddr,uint8_t len,uint8_t data[]);
      bool isInfoAvailable();
      uint8_t readRFData(uint8_t rxData[],uint8_t &len);
      uint8_t getRSSI();                           
      uint8_t getPktRSSI();
      uint8_t writeEEPROM(uint8_t len,uint8_t deviInfo[]);   
      uint8_t readEEPROM(uint8_t deviInfo[],uint8_t &len); 
      uint8_t getFWVer(uint8_t number[]); 
      uint8_t getSN(uint8_t id[]);
                                                                                              
      uint8_t getDeviceRole();                       
      uint8_t getMode();                             
      unsigned long getFrequency();                       
      uint8_t getRFPower();                          
      uint8_t getDataRate();                        
      uint8_t getBaudRate();                         
      uint8_t setDeviceRole(uint8_t role);               
      uint8_t setMode(uint8_t mode);                     
      uint8_t setFrequency(unsigned long frequency);             
      uint8_t setRFPower(uint8_t power);                     
      uint8_t setDataRate(uint8_t rate);                  
                            
   private:                                          
      uint8_t setBaudRate(uint8_t baudRate);
      uint8_t setRFAddress(uint8_t address[]);
      uint8_t getRFAddress(uint8_t address[]);  
      uint8_t checksum(uint8_t len,uint8_t data[]);  
      void writeBytes(uint8_t wbuf[], uint8_t wlen);
      uint8_t readBytes(uint8_t rbuf[], uint8_t rlen, uint16_t timeOut = 150); 
      uint16_t _nodeShortAddr; 
      uint16_t _rxPin;
      uint16_t _txPin;
      HardwareSerial *_hardSerial = NULL;
      SoftwareSerial *_softSerial = NULL ;   
           
};




/**
  * @enum     Device_Role_TYPE
  * @brief    設備端角色類型
  */
enum Device_Role_TYPE
{
  Peer              = 0x00,             //!< 自由節點
  Node_of_Star  ,                       //!< 星狀網絡節點
  Con_of_Star   ,                       //!< 星狀網絡集中器
  MAX_of_RoleValue,                     //!< Role選項最大值
};
/**
  * @enum     Device_Mode_TYPE
  * @brief    設備端工作模式類型
  */
enum Device_Mode_TYPE
{
  DeepSleep_Mode    = 0x00,             //!< 設備端深睡眠模式
  Sleep_Mode,                     //!< 設備端睡眠模式
  Rx_Mode     ,                         //!< 設備端接收模式
  Pairing_Mode  ,                       //!< 設備端配對模式
  MAX_of_ModeValue,                     //!< Mode選項最大值
};
/**
  * @enum     Device_Power_TYPE
  * @brief    設備端RF輸出功率
  */
enum Device_Power_TYPE
{
  MIN_of_PowerValue   = 0x00,           //!< power選項最小值
  RF_0dBm           = 0x01,             //!< 設備端RF輸出功率0 dBm
  RF_5dBm     ,                         //!< 設備端RF輸出功率5 dBm
  RF_10dBm    ,                         //!< 設備端RF輸出功率10 dBm
  RF_13dBm    ,                         //!< 設備端RF輸出功率13 dBm
  MAX_of_PowerValue,                    //!< power選項最大值
};
/**
  * @enum     Device_Power_TYPE
  * @brief    設備端RF輸出功率
  */
enum Device_BitRate_TYPE
{
  MIN_of_BRValue    = 0x01,             //!< bit rate選項最大值
  RF_10kbps         = 0x02,             //!< 設備端RF通訊速率10 kbps
  RF_25kbps   ,                         //!< 設備端RF通訊速率25 kbps
  RF_50kbps   ,                         //!< 設備端RF通訊速率50 kbps
  RF_125kbps    ,                       //!< 設備端RF通訊速率125 kbps
  RF_250kbps    ,                       //!< 設備端RF通訊速率250 kbps
  MAX_of_BRValue,                        //!< bit rate選項最大值
};
/**
  * @enum     Short_Addr_Type
  * @brief    Short Address 種類
  */
enum Short_Addr_Type
{
  SA_Broadcast      = 0x0000,           //!< 廣播通訊專用shor address
  SA_Star_Pair      = 0x0E3F,           //!< 星狀網絡配對專用shor address
  SA_Peer_Pair      = 0x0D3F,           //!< Peer配對專用shor address
  SA_Peer           = 0xEF3F,           //!< Peer通訊專用shor address
  SA_Concentrator   = 0xFF3F,           //!< Concentrator通訊專用shor address
  SA_Pairing        = 0x0040,           //!< 配對         
  SA_Unpairing      = 0x0080,           //!< 解除配對
};
#endif
