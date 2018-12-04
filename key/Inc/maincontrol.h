#ifndef _MAINCONTROL_H_
#define _MAINCONTROL_H_

#include "stdint.h"
#include "user_config.h"
#include "lora.h"
#include "list.h"

//设备所能识别的cmd
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

//server内部与endpoint之间的通信命令，与405 cmd共占用资源
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
*   初始化与F405通信的串口相关结构体
*/
void UARTF405_Init(UART_HandleTypeDef *uart);

/*
*   向F405发送通知消息
*/
void UartSendMSGToF405(uint8_t status, uint16_t id, uint32_t identify);

/*
*   向F405发送指令响应信息，成功返回true，失败返回false
*/
bool UartSendRespToF405(DeviceNode *devn);

/*
*   获取uart module结构体
*/
UartModule *GetF405UartModule(UART_HandleTypeDef *huart);

/*
*   中断函数:F405接收函数
*   在串口接收完成中断中被调用
*/
void F405ReveiveHandler(UART_HandleTypeDef *huart);

/*
*   F405命令处理线程
*   F405只能控制由本机发送出去的设备ID，对于其他ID不进行操作
*/
void F405Task(void);

#endif



