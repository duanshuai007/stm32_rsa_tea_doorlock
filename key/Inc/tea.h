/**
  ******************************************************************************
  * �ļ����ƣ�tea.h
  * ��    �ܣ�TEA�ӽ���
  * ��    �ߣ�һֱ���ܵ�
  * ��    �ڣ�2016/12/28
  ******************************************************************************
  */
#ifndef __TEA_H
#define __TEA_H


/*
    ͷ�ļ�����
*/
#include "stdint.h"

#define COMMON_TEA_PASSWORD   "WAMINGTECHNOLOGY"
    
extern void TEA_Encipher(uint32_t * crypt, uint32_t * plain,  uint32_t * key);
extern void TEA_Decrypt(uint32_t * crypt,  uint32_t * key);


#endif

