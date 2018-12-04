#include "flash.h"
#include "stm32f1xx.h"
#include "stdlib.h"
#include "rsa_16.h"
#include "tea.h"

/*
*   FLASH内保存设备的密钥信息
*   密钥以加密方式保存在FLASH被
*   需要用tea解密方式进行解密
*   
*   [head]    [message]
*   AABBCDDC  .......
*
*   [   m   e  s   s   a   g   e   ]
*   [title][加密内容]
*   对加密内容解密后，需要根据title对解密后的内容
*   进行摘除，最后得到实际的密钥
*/

static void strtohex(uint8_t *dst, uint32_t *src, uint8_t srclen)
{
  uint8_t i, j;
  uint8_t dat;
  
  for(i=0; i < srclen; i++) {
    for ( j = 0; j < 4; j++) {
    
      //src是32为地址指针，+1对应一个32位类型，指向下一个数组元素的位置
      dat = (uint8_t)((*(src + i)) >> (j*8));
      
      if ((dat >= 'a') && (dat <= 'z')) {
        dat -= ('a' - 0x0a);
      } else if ((dat >= '0') && (dat <= '9')) {
        dat -= '0';
      } else if ((dat >= 'A') && (dat <= 'Z')) {
        dat -= ('A' - 0x0a);
      }
      
      if ((j % 2) == 0)
        *(dst + j/2 + (i*2)) |= (dat << 4);
      else
        *(dst + j/2 + (i*2)) |= dat;
    }
  }
}

static uint16_t chartohex(uint8_t *src)
{
  uint8_t dat;
  uint8_t i;
  uint16_t ret = 0;
  
  for ( i = 0; i < 4; i++) {
    
    dat = *(src + i);
    
    if ((dat >= 'a') && (dat <= 'z')) {
      dat -= ('a' - 0x0a);
    } else if ((dat >= '0') && (dat <= '9')) {
      dat -= '0';
    } else if ((dat >= 'A') && (dat <= 'Z')) {
      dat -= ('A' - 0x0a);
    }
    
    ret |= (dat << ((3 - i)*4));
  }
  
  return ret;
}

uint16_t FLASH_Init(void)
{
  uint32_t key[4];  //解密密钥
  uint32_t keyd[8]; //通信密钥
  uint8_t i;
  uint16_t n, e, d, id;
  
  uint8_t pw_suiji[16] = {0};
  uint8_t miyao[16] = {0};
  
  uint32_t dat;
  
  dat = READ_FLASH_WORD(FLASH_DATABASE_START);
  if( FLASH_MSG_HEAD == dat) {
    //如果flash内已经有了有效的数据
    //读取加密后的随机密码
    for(i=0;i<4;i++) {
      key[i] = READ_FLASH_WORD(FLASH_DATABASE_START + 4 + (i*4));
    }
    //读取加密后的密钥
    for(i=0;i<8;i++) {
      keyd[i] = READ_FLASH_WORD(FLASH_DATABASE_START + 20 + (i*4));
    }
    //将加密后的随机密码转换为十六进制数
    strtohex(pw_suiji, key, 4);
    //将加密后的密钥转换为十六进制数
    strtohex(miyao, keyd, 8);
    
    //解密随机密码，使用通用密码进行解密，解密后得到的是密钥解密密码
    TEA_Decrypt((uint32_t *)pw_suiji, (uint32_t *)COMMON_TEA_PASSWORD);
    //使用上一步得到的随机密码对加密的密钥进行解密，得到实际的密钥
    TEA_Decrypt((uint32_t *)miyao, (uint32_t *)pw_suiji);
    TEA_Decrypt((uint32_t *)(miyao + 8), (uint32_t *)pw_suiji);
    
    //从解密后的密钥字符串中提取出n，d，e，id信息
    d = chartohex(miyao);
    e = chartohex(miyao + 4);
    n = chartohex(miyao + 8);
    id = chartohex(miyao + 12);
    
//    printf("n=%d,e=%d,d=%d, id=%04x\r\n", n, e, d, id);
    
    setRSAKey(n, e, d);
    
    return id;
    
  } else {
    
    return 0;
    //如果flash内还是空空的
//    HAL_FLASH_Unlock();
//    
//    uint32_t pageerror = 0;
//    FLASH_EraseInitTypeDef f;
//    
//    f.TypeErase = FLASH_TYPEERASE_PAGES;
//    f.PageAddress = FLASH_DATABASE_START;
//    f.NbPages = 1;
//    
//    if ( HAL_OK != HAL_FLASHEx_Erase(&f, &pageerror)) {
//      PRINT("FLASH:erase %08x failed\r\n", pageerror);
//      return;
//    }
//    
//    HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, FLASH_DATABASE_START, SAVE_MESSAGE_HEAD);
//    HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, FLASH_DATABASE_START + 4, 0);
//    HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, FLASH_DATABASE_START + 8, 0);
//    HAL_FLASH_Lock();
  }
}

/*
*   从链表中取出数据写入flash
*/
void FlashWrite(TimeStamp *ts)
{
  uint32_t pageerror = 0;
  uint32_t dat;
//  uint8_t i = 0;
  FLASH_EraseInitTypeDef f;
  
  dat = READ_FLASH_WORD(FLASH_DATABASE_START);
  
  if ( FLASH_MSG_HEAD != dat ) {
    PRINT("FLASH:cant't read flash head\r\n");
    return;
  }
  
  HAL_FLASH_Unlock();
  
  f.TypeErase = FLASH_TYPEERASE_PAGES;
  f.PageAddress = FLASH_DATABASE_START;
  f.NbPages = 1;
  
  //因为在初始化的时候已经把设备信息读取出来保存到链表中了，所以这里可以直接
  //进行覆盖式写入。
  
  if ( HAL_OK != HAL_FLASHEx_Erase(&f, &pageerror)) {
    PRINT("FLASH:erase %08x failed\r\n", pageerror);
    return;
  }
  
  //写入验证头
  HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, FLASH_DATABASE_START, FLASH_MSG_HEAD);
  //写入总设备数
  dat = ((((uint32_t)(ts->u8Year)) << 16) | (((uint32_t)(ts->u8Mon)) << 8) | ts->u8Day);
  HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, FLASH_DATABASE_START + 4, dat);
  dat = ((((uint32_t)(ts->u8Hour)) << 16) | (((uint32_t)(ts->u8Min)) << 8) | ts->u8Sec);
  HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, FLASH_DATABASE_START + 8, dat);
  
  PRINT("FLASH:write done\r\n");
  HAL_FLASH_Lock();
}

#if 0
static bool checktime(uint32_t time)
{
  uint8_t hour,min,sec;
  
  hour = (uint8_t)(time >> 16);
  min = (uint8_t)(time >> 8);
  sec = (uint8_t)(time);
  if(hour > 23)
    return false;
  if(min > 59)
    return false;
  if(sec > 59)
    return false;
  
  return true;
}

static bool checkdate(uint32_t date)
{
  uint8_t mon,day;
  
//  year = (uint8_t)(date >> 4);
  mon = (uint8_t)(date >> 8);
  day = (uint8_t)(date);

  if(mon > 12)
    return false;
  if(day > 31)
    return false;
  
  return true;
}

bool IsValidTimeStamp(TimeStamp *ts)
{
  uint32_t daten, dateo;
  uint32_t timen, timeo;
  uint32_t dat;
  
  dat = READ_FLASH_WORD(FLASH_DATABASE_START);
  
  if ( SAVE_MESSAGE_HEAD != dat ) {
    PRINT("FLASH:cant't read flash head\r\n");
    return false;
  }
  
  daten = ((((uint32_t)(ts->u8Year)) << 16) | (((uint32_t)(ts->u8Mon)) << 8) | ts->u8Day);
  timen = ((((uint32_t)(ts->u8Hour)) << 16) | (((uint32_t)(ts->u8Min)) << 8) | ts->u8Sec);
  
  dateo = READ_FLASH_WORD(FLASH_DATABASE_START + 4);
  timeo = READ_FLASH_WORD(FLASH_DATABASE_START + 8);
  
  if ((dateo == 0) && (timeo == 0)) {
    if ( checkdate(daten) ) {
      return checktime(timen);
    }
    
    return false;
  }
  
  if (dateo == daten) {
    if ( timeo < timen ) {
      //有效时间
      return checktime(timen);
    } else {
      //无效时间
      return false;
    }
  } else {
   
    //如果日期范围错误
    if(dateo > daten)
      return false;
   
    //日期范围正确
//    uint8_t ynew,yold,mnew,mold;
//    yold = (uint8_t)(dateo>>16);
//    mold = (uint8_t)(dateo>>8);
//
//    ynew = (uint8_t)(daten>>16);
//    mnew = (uint8_t)(daten>>8);

    if (checkdate(daten)) {
      return checktime(timen);
    }
    
    return false;
  }
}
#endif

