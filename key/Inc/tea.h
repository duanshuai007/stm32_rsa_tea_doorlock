/**
  ******************************************************************************
  * 文件名称：tea.h
  * 功    能：TEA加解密
  * 作    者：一直在跑灯
  * 日    期：2016/12/28
  ******************************************************************************
  */
#ifndef __TEA_H
#define __TEA_H


/*
    头文件包含
*/
#include "stdint.h"

#define COMMON_TEA_PASSWORD   "WAMINGTECHNOLOGY"
    
extern void TEA_Encipher(uint32_t * crypt, uint32_t * plain,  uint32_t * key);
extern void TEA_Decrypt(uint32_t * crypt,  uint32_t * key);


#endif

