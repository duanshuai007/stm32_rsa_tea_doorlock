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
    
    setRSAKey(n, e, d);
    
    return id;
  }
  
  return 0;
}
