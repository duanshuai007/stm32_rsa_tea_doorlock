#include "crc16.h"
#include "stdint.h"
#include "stm32f1xx.h"
#include "stm32f1xx_hal_def.h"

static void InvertUint8(uint8_t *dBuf,uint8_t *srcBuf)
{
  uint8_t i;
  uint8_t tmp = 0;
  for(i=0;i< 8;i++)
  {
    if(srcBuf[0] & (1 << i))          
      tmp |= 1 << (7-i);
  }
  dBuf[0] = tmp;
}

static void InvertUint16(uint16_t *dBuf,uint16_t *srcBuf)
{
  uint8_t i;
  uint16_t tmp = 0;
  
  for(i=0;i< 16;i++)
  {
    if(srcBuf[0]& (1 << i))
      tmp|=1<<(15 - i);
  }
  dBuf[0] = tmp;
}

#if 0
static void InvertUint32(uint32_t *dBuf,uint32_t *srcBuf)
{
  uint8_t i;
  uint32_t tmp = 0;
  
  for(i=0;i< 32;i++)
  {
    if(srcBuf[0]& (1 << i))
      tmp|=1<<(15 - i);
  }
  dBuf[0] = tmp;
}
#endif

uint16_t CRC16_IBM(uint8_t *puchMsg, uint8_t usDataLen)
{
  uint16_t wCRCin = 0x0000;
  uint16_t wCPoly = 0x8005;
  uint8_t wChar = 0;
  uint8_t i;
  
  while (usDataLen--) 	
  {
    wChar = *(puchMsg++);
    InvertUint8(&wChar,&wChar);
    wCRCin ^= (wChar << 8);
    for(i = 0;i < 8;i++)
    {
      if(wCRCin & 0x8000)
        wCRCin = (wCRCin << 1) ^ wCPoly;
      else
        wCRCin = wCRCin << 1;
    }
  }
  InvertUint16(&wCRCin,&wCRCin);
  
  return wCRCin;
}

uint8_t crc8_chk_value(uint8_t *message, uint8_t len)
{
  uint8_t crc;
  uint8_t i;
  crc = 0;
  while(len--)
  {
    crc ^= *message++;
    for(i = 0;i < 8;i++)
    {
      if(crc & 0x01)
      {
        crc = (crc >> 1) ^ 0x8c;
      }
      else crc >>= 1;
    }
  }
  return crc; 
}
