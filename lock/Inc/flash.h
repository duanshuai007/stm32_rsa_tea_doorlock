#ifndef _STM32_FLASH_H_
#define _STM32_FLASH_H_

#include "stdint.h"
#include "user_config.h"

#define FLASH_DATABASE_START  0x0800f000
#define FLASH_MSG_HEAD        0x474f5246        //"FROG"

#define READ_FLASH_WORD(addr) (*(volatile uint32_t *)(addr))

/*
*   Flashl链表初始化
*   从存储地区读取已有的设备信息写入flash链表内
*/
uint16_t FLASH_Init(void);

#endif