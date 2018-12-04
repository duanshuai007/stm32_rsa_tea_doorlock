#ifndef _USER_CONFIG_H_
#define _USER_CONFIG_H_

#include "stdint.h"
#include "stm32f1xx.h"
#include "stm32f1xx_hal_def.h"
#include "lora_paramter.h"

typedef enum {
  false,
  true,
}bool;

#if 0
#define PRINT(...) printf(__VA_ARGS__)
#else
#define PRINT(...)
#endif

//默认的服务器信道
#define DEFAULT_CHANNEL 0x1e

//lora receive module
#define UART1_RX_DMA_LEN        64
#define UART1_RX_DATAPOOL_SIZE  256


//目标设备的信息
#pragma pack(1)
typedef struct {
  uint8_t u8Year;
  uint8_t u8Mon;
  uint8_t u8Day;
  uint8_t u8Hour;
  uint8_t u8Min;
  uint8_t u8Sec;
} TimeStamp;
#pragma pack()

#endif
