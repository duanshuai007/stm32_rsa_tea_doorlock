#ifndef __MY_LIST_H__
#define __MY_LIST_H__

#include "stdint.h"
#include "user_config.h"

//电机运行状态信息，当发送CMD是电机动作时，resp等于该值
#define MOTOR_RUNING          99   //地锁电机开始运行了

//设备的指令执行状态，用于server本身cmd状态的跟踪
#define CMD_STATUS_IDLE       100   //空闲状态
#define CMD_STATUS_RUN        101   //正在运行
#define CMD_STATUS_DONE       102   //运行完成
#define CMD_STATUS_STOP       103   //超时停止运行
#define CMD_STATUS_STANDBY    104   //有新指令，等待运行


typedef struct DeviceNode {
  uint32_t u32Identify;     //指令identify
  uint32_t u32Time;         //超时计时：1:心跳时间，2:指令执行超时
  
  uint16_t u16ID;           //设备ID
  uint8_t u8CHAN;           //信道
  uint8_t u8CMD;            //设备的指令  
  
  uint8_t u8CMDSTATUS;      //指令执行状态
  uint8_t u8RESP;           //地锁的响应
  uint8_t u8CMDRetry;       //cmd没有影响重试的次数
} DeviceNode;

typedef struct List {
  struct List *next;
  struct DeviceNode *Device;
} List;

/*
*   链表初始化
*/
void ListInit(void);

/*
*   添加设备到链表中
*   成功:true
*   失败:false
*/
bool AddDeviceToList(uint16_t id);

/*
*   设置指令ID设备节点的CMD和IDENTIFY
*   设备不存在返回false
*   设备忙或设备正常返回true
*/
bool SendCMDToList(uint16_t id, uint8_t cmd, uint32_t identify);

/*
*   向链表发送设备的响应信息
*   成功:true
*   失败:false
*/
void SendCMDRespToList(uint16_t id, uint8_t cmd, uint32_t identify, uint8_t resp);

/*
*   设备链表进程，在主循环内调用，循环判断链表内每个设备的状态
*   当对应的u8CMDSTATUS状态为:
*     CMD_STATUS_IDLE       设备空闲
*     CMD_STATUS_STANDBY    设备有指令等待执行
*     CMD_STATUS_RUN        设备指令正在运行
*     CMD_STATUS_DONE       设备指令运行完成
*/
void ListTask(void);


/*
*   打印整个表
*/
void LookAllList(void);

#endif



