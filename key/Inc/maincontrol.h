#ifndef _MAINCONTROL_H_
#define _MAINCONTROL_H_

#include "stdint.h"
#include "user_config.h"
#include "lora.h"
#include "list.h"

//�豸����ʶ���cmd
typedef enum 
{
  CMD_NONE              = 0,
  CMD_MOTOR_UP          = 1,
  CMD_MOTOR_DOWN        = 2,
  CMD_MOTOR_STATUS_GET  = 3,
  CMD_BEEP_ON           = 4,
  CMD_BEEP_OFF          = 5,
  CMD_BEEP_STATUS_GET   = 6,
  CMD_ADC_GET           = 7,
  CMD_GET_ALL_NODE      = 8,
}CMD_TYPE;

//server�ڲ���endpoint֮���ͨ�������405 cmd��ռ����Դ
#define DEVICE_REGISTER     100
#define DEVICE_HEART        101
#define DEVICE_ABNORMAL     102

typedef enum
{
  //Normal
  NORMAL_SUCCESS            = 101,
  NORMAL_DOWN               = 102,
  NORMAL_FORWARD            = 103,
  NORMAL_UP                 = 104,
  NORMAL_BACK               = 105,
  NORMAL_BUSY               = 106,
  NORMAL_BEEP_OPEN_FAILED   = 107,
  NORMAL_BEEP_CLOSE_FAILED  = 108,
  NORMAL_BEEP_STATUS_OPEN   = 109,
  NORMAL_BEEP_STATUS_CLOSED = 110,
  NORMAL_MOTOR_RUNNING      = 111,

  NODE_ONLINE               = 120,
  NODE_OFFLINE              = 121,
  NODE_NOTEXIST             = 122,
  //interupt
  INTERUPT_DOWN             = 201,
  INTERUPT_FORWARD          = 202,    
  INTERUPT_UP               = 203,
  INTERUPT_BACK             = 204,
}RESP_CODE;

/*
*   ��ʼ����F405ͨ�ŵĴ�����ؽṹ��
*/
void UARTF405_Init(UART_HandleTypeDef *uart);

/*
*   ��F405����֪ͨ��Ϣ
*/
void UartSendMSGToF405(uint8_t status, uint16_t id, uint32_t identify);

/*
*   ��F405����ָ����Ӧ��Ϣ���ɹ�����true��ʧ�ܷ���false
*/
bool UartSendRespToF405(DeviceNode *devn);

/*
*   ��ȡuart module�ṹ��
*/
UartModule *GetF405UartModule(UART_HandleTypeDef *huart);

/*
*   �жϺ���:F405���պ���
*   �ڴ��ڽ�������ж��б�����
*/
void F405ReveiveHandler(UART_HandleTypeDef *huart);

/*
*   F405������߳�
*   F405ֻ�ܿ����ɱ������ͳ�ȥ���豸ID����������ID�����в���
*/
void F405Task(void);

#endif



