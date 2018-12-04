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
//LORA��ռ�õ�����
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
*     loraģ������ṹ��
*     ����loraģ�����������һ����6�ֽ�
*     byte0...byte1...byte2...byte3...byte4...byte5
*     [0xC0]  [    addr   ]  [    ]  [chan]  [   ]   
*/
typedef struct {
  uint16_t u16Addr;   //Byte1-Byte2 ��Ӧģ���ַ��Ϣ     
  
  //Byte3 �����ô��ںͿ�������
  //��Byte3�����ݲ�ֳ�����������������
  uint8_t u8Parity;     //λ��Byte3��6-7bit
  uint8_t u8Baud;       //λ��Byte3��3-5bit        
  uint8_t u8Speedinair; //λ��Byte3��0-2bit
  
  //Byte4 ����ģ����ŵ�
  uint8_t u8Channel;
  
  //Byte5 ����ģ�黽��ʱ�乤��ģʽ��
  uint8_t u8TranferMode;      //λ��Byte5��7bit 
  uint8_t u8IOMode;           //λ��Byte5��6bit
  uint8_t u8WakeUpTime;       //λ��Byte5��3-5bit
  uint8_t u8FEC;              //λ��Byte5��2bit
  uint8_t u8SendDB;           //λ��Byte5��0-1bit
} LoraPar;

//gpio �ṹ��
typedef struct {
  GPIO_TypeDef  *GPIOX;   //�˿�
  uint16_t      Pin;      //����  
} LoraPin;

//lora�������ýṹ��
typedef struct {
  LoraPin AUX;            //aux����
  LoraPin M0;             //m0��������
  LoraPin M1;             //m1��������
} LoraGPIO;

//���ڽṹ��
typedef struct {

  UART_HandleTypeDef  *uart;    //����

#ifdef UART1_SEND_USE_DMA
  DMA_HandleTypeDef   *uart_tx_hdma;  //���ڷ���dma���
  uint16_t dma_sbuff_size;  //dma���ͻ������Ĵ�С
  uint8_t *dma_sbuff;         //ָ��dma���ͻ�����
  DataPool *txDataPool;   //�������ݳ�
#endif
  
#ifdef UART1_RECV_USE_DMA
  volatile uint16_t   pos;    //buffer current postion
  DMA_HandleTypeDef   *uart_rx_hdma;  //���ڽ���dma���
  uint16_t dma_rbuff_size;  //dma���ջ������Ĵ�С
  uint8_t *dma_rbuff;         //ָ��dma���ջ�����
  DataPool *rxDataPool;   //�������ݳ�
#endif

} UartModule;

//���汾��lora�豸�����Ϣ
typedef struct {
  volatile bool  isIdle; //IDLE FLAG
  Lora_Mode       mode;   //work mode
  LoraPar         paramter;//lora�����ṹ��
  LoraGPIO        gpio;     //lora gpio�ṹ��
  UartModule      muart;    //lora ���ڽṹ��
} LoraModule;

#pragma pack()

/*
*   Loraģ�鷢�ͼ����ʱ��
*   ��Ϊloraģ���ǹ����ڻ���ģʽ����ͬ���豸id֮����Ҫ���������г����ܹ�
*   ��֤ÿһ֡���ݶ���ӻ�����
*/
void LoraSendTimerHandler(void);

/*
*   �ж�loraģ������ŵ�ƽ������lora�Ŀ���״̬
*/
void SetLoraModuleIdleFlagHandler(uint16_t pin);

/*
*   ����loraģ�����õ�������
*/
void LoraModuleGPIOInit(UART_HandleTypeDef *huart);

/*
*   ����loraģ���dma��dma���շ��ͻ�������datapool�Ȳ���
*/
void LoraModuleDMAInit(UART_HandleTypeDef *huart);

/*
*   ����Loraģ�����
*/
void SetLoraParamter(UART_HandleTypeDef *huart, LoraPar *lp);

/*
*   ��ȡLoraģ�����
*/
void ReadLoraParamter(UART_HandleTypeDef *huart);

/*
*   �жϺ�������������ʱ�Ὣ��Ӧ�Ĵ���DMA�������е�
*             ���ݸ��Ƶ���Ӧ�����Լ������ݳ��С�
*   ֻ��uart1��uart2�Ŀ����ж��л���á�
*/
bool CopyDataFromDMAHandler(UART_HandleTypeDef *huart);

/*
*   �жϺ���:����DMA�����ƶ�������ʹ��dma���չ���
*   �ڴ��ڽ�������ж��е���
*/
void LoraModuleReceiveHandler(UART_HandleTypeDef *huart); 

/*
*   Loraģ������ݴ���
*/
void LoraModuleTask(void);

#endif



