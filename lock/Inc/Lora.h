#ifndef _LORA_H_
#define _LORA_H_

#include "stdint.h"
#include "stm32f1xx.h"
#include "lora_datapool.h"
#include "user_config.h"

//#define UART1_SEND_USE_DMA
#define UART1_RECV_USE_DMA

//#define GPIO_BEEP                       GPIOB
//#define GPIO_BEEP_Pin                   GPIO_PIN_10

#define GPIO_LOCK                       GPIOB  
#define GPIO_LOCK_Pin                   GPIO_PIN_10  
//LORA所占用的引脚
#define GPIO_Lora_M0        GPIOA
#define GPIO_Lora_M1        GPIOB
#define GPIO_Lora_AUX       GPIOB
#define GPIO_Lora_M0_Pin    GPIO_PIN_8
#define GPIO_Lora_M1_Pin    GPIO_PIN_15
#define GPIO_Lora_AUX_Pin   GPIO_PIN_14

//PA7 led
#define GPIO_LED            GPIOA
#define GPIO_LED_Pin        GPIO_PIN_7

typedef enum { 
  LoraMode_Normal,
  LoraMode_WakeUp,
  LoraMode_LowPower,
  LoraMode_Sleep,
}Lora_Mode;

#pragma pack(1)

/*
*     lora模块参数结构体
*     设置lora模块参数的命令一共有6字节
*     byte0...byte1...byte2...byte3...byte4...byte5
*     [0xC0]  [    addr   ]  [    ]  [chan]  [   ]   
*/
typedef struct {
  uint16_t u16Addr;   //Byte1-Byte2 对应模块地址信息     
  
  //Byte3 是设置串口和空中速率
  //将Byte3的内容拆分出来是以下三个内容
  uint8_t u8Parity;     //位于Byte3的6-7bit
  uint8_t u8Baud;       //位于Byte3的3-5bit        
  uint8_t u8Speedinair; //位于Byte3的0-2bit
  
  //Byte4 设置模块的信道
  uint8_t u8Channel;
  
  //Byte5 设置模块唤醒时间工作模式等
  uint8_t u8TranferMode;      //位于Byte5的7bit 
  uint8_t u8IOMode;           //位于Byte5的6bit
  uint8_t u8WakeUpTime;       //位于Byte5的3-5bit
  uint8_t u8FEC;              //位于Byte5的2bit
  uint8_t u8SendDB;           //位于Byte5的0-1bit
} LoraPar;

//gpio 结构体
typedef struct {
  GPIO_TypeDef  *GPIOX;   //端口
  uint16_t      Pin;      //引脚  
} LoraPin;

//lora引脚配置结构体
typedef struct {
  LoraPin AUX;            //aux引脚
  LoraPin M0;             //m0功能引脚
  LoraPin M1;             //m1功能引脚
} LoraGPIO;

//串口结构体
typedef struct {

  UART_HandleTypeDef  *uart;    //串口

#ifdef UART1_SEND_USE_DMA
  DMA_HandleTypeDef   *uart_tx_hdma;  //串口发送dma句柄
  uint16_t dma_sbuff_size;  //dma发送缓冲区的大小
  uint8_t *dma_sbuff;         //指向dma发送缓冲区
  DataPool *txDataPool;   //发送数据池
#endif
  
#ifdef UART1_RECV_USE_DMA
  volatile uint16_t   pos;    //buffer current postion
  DMA_HandleTypeDef   *uart_rx_hdma;  //串口接收dma句柄
  uint16_t dma_rbuff_size;  //dma接收缓冲区的大小
  uint8_t *dma_rbuff;         //指向dma接收缓冲区
  DataPool *rxDataPool;   //接收数据池
#endif

} UartModule;

//保存本机lora设备相关信息
typedef struct {
  volatile bool  isIdle; //IDLE FLAG
  Lora_Mode       mode;   //work mode
  LoraPar         paramter;//lora参数结构体
  LoraGPIO        gpio;     //lora gpio结构体
  UartModule      muart;    //lora 串口结构体
} LoraModule;

#pragma pack()

/*
*   Lora模块发送间隔定时器
*   因为lora模块是工作在唤醒模式，不同的设备id之间需要保留空闲市场才能够
*   保证每一帧数据都添加唤醒码
*/
void LoraSendTimerHandler(void);

/*
*   判断lora模块的引脚电平来设置lora的空闲状态
*/
void SetLoraModuleIdleFlagHandler(uint16_t pin);

/*
*   设置lora模块所用到的引脚
*/
void LoraModuleGPIOInit(UART_HandleTypeDef *huart);

/*
*   设置lora模块的dma，dma接收发送缓冲区，datapool等参数
*/
void LoraModuleDMAInit(UART_HandleTypeDef *huart);

/*
*   设置Lora模块参数
*/
void SetLoraParamter(UART_HandleTypeDef *huart, LoraPar *lp);

/*
*   读取Lora模块参数
*/
void ReadLoraParamter(UART_HandleTypeDef *huart);

/*
*   中断函数：当被调用时会将对应的串口DMA缓冲区中的
*             数据复制到对应串口自己的数据池中。
*   只有uart1和uart2的空闲中断中会调用。
*/
bool CopyDataFromDMAHandler(UART_HandleTypeDef *huart);

/*
*   中断函数:用于DMA数据移动和重新使能dma接收功能
*   在串口接收完成中断中调用
*/
void LoraModuleReceiveHandler(UART_HandleTypeDef *huart); 

/*
*   Lora模块的数据处理
*/
void LoraModuleTask(void);

#endif



