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
    
    setRSAKey(n, e, d);
    
    return id;
  }
  
  return 0;
}
