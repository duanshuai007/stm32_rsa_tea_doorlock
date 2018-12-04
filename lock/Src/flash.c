#include "flash.h"
#include "stm32f1xx.h"
#include "stdlib.h"
#include "rsa_16.h"
#include "tea.h"

/*
*   FLASH�ڱ����豸����Կ��Ϣ
*   ��Կ�Լ��ܷ�ʽ������FLASH��
*   ��Ҫ��tea���ܷ�ʽ���н���
*   
*   [head]    [message]
*   AABBCDDC  .......
*
*   [   m   e  s   s   a   g   e   ]
*   [title][��������]
*   �Լ������ݽ��ܺ���Ҫ����title�Խ��ܺ������
*   ����ժ�������õ�ʵ�ʵ���Կ
*/

static void strtohex(uint8_t *dst, uint32_t *src, uint8_t srclen)
{
  uint8_t i, j;
  uint8_t dat;
  
  for(i=0; i < srclen; i++) {
    for ( j = 0; j < 4; j++) {
    
      //src��32Ϊ��ַָ�룬+1��Ӧһ��32λ���ͣ�ָ����һ������Ԫ�ص�λ��
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
  uint32_t key[4];  //������Կ
  uint32_t keyd[8]; //ͨ����Կ
  uint8_t i;
  uint16_t n, e, d, id;
  
  uint8_t pw_suiji[16] = {0};
  uint8_t miyao[16] = {0};
  
  uint32_t dat;
  
  dat = READ_FLASH_WORD(FLASH_DATABASE_START);
  if( FLASH_MSG_HEAD == dat) {
    //���flash���Ѿ�������Ч������
    //��ȡ���ܺ���������
    for(i=0;i<4;i++) {
      key[i] = READ_FLASH_WORD(FLASH_DATABASE_START + 4 + (i*4));
    }
    //��ȡ���ܺ����Կ
    for(i=0;i<8;i++) {
      keyd[i] = READ_FLASH_WORD(FLASH_DATABASE_START + 20 + (i*4));
    }
    //�����ܺ���������ת��Ϊʮ��������
    strtohex(pw_suiji, key, 4);
    //�����ܺ����Կת��Ϊʮ��������
    strtohex(miyao, keyd, 8);
    
    //����������룬ʹ��ͨ��������н��ܣ����ܺ�õ�������Կ��������
    TEA_Decrypt((uint32_t *)pw_suiji, (uint32_t *)COMMON_TEA_PASSWORD);
    //ʹ����һ���õ����������Լ��ܵ���Կ���н��ܣ��õ�ʵ�ʵ���Կ
    TEA_Decrypt((uint32_t *)miyao, (uint32_t *)pw_suiji);
    TEA_Decrypt((uint32_t *)(miyao + 8), (uint32_t *)pw_suiji);
    
    //�ӽ��ܺ����Կ�ַ�������ȡ��n��d��e��id��Ϣ
    d = chartohex(miyao);
    e = chartohex(miyao + 4);
    n = chartohex(miyao + 8);
    id = chartohex(miyao + 12);
    
//    printf("n=%d,e=%d,d=%d, id=%04x\r\n", n, e, d, id);
    
    setRSAKey(n, e, d);
    
    return id;
    
  } else {
    
    return 0;
    //���flash�ڻ��ǿտյ�
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
*   ��������ȡ������д��flash
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
  
  //��Ϊ�ڳ�ʼ����ʱ���Ѿ����豸��Ϣ��ȡ�������浽�������ˣ������������ֱ��
  //���и���ʽд�롣
  
  if ( HAL_OK != HAL_FLASHEx_Erase(&f, &pageerror)) {
    PRINT("FLASH:erase %08x failed\r\n", pageerror);
    return;
  }
  
  //д����֤ͷ
  HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, FLASH_DATABASE_START, FLASH_MSG_HEAD);
  //д�����豸��
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
      //��Чʱ��
      return checktime(timen);
    } else {
      //��Чʱ��
      return false;
    }
  } else {
   
    //������ڷ�Χ����
    if(dateo > daten)
      return false;
   
    //���ڷ�Χ��ȷ
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

