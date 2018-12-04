#ifndef __MY_LIST_H__
#define __MY_LIST_H__

#include "stdint.h"
#include "user_config.h"

//�������״̬��Ϣ��������CMD�ǵ������ʱ��resp���ڸ�ֵ
#define MOTOR_RUNING          99   //���������ʼ������

//�豸��ָ��ִ��״̬������server����cmd״̬�ĸ���
#define CMD_STATUS_IDLE       100   //����״̬
#define CMD_STATUS_RUN        101   //��������
#define CMD_STATUS_DONE       102   //�������
#define CMD_STATUS_STOP       103   //��ʱֹͣ����
#define CMD_STATUS_STANDBY    104   //����ָ��ȴ�����


typedef struct DeviceNode {
  uint32_t u32Identify;     //ָ��identify
  uint32_t u32Time;         //��ʱ��ʱ��1:����ʱ�䣬2:ָ��ִ�г�ʱ
  
  uint16_t u16ID;           //�豸ID
  uint8_t u8CHAN;           //�ŵ�
  uint8_t u8CMD;            //�豸��ָ��  
  
  uint8_t u8CMDSTATUS;      //ָ��ִ��״̬
  uint8_t u8RESP;           //��������Ӧ
  uint8_t u8CMDRetry;       //cmdû��Ӱ�����ԵĴ���
} DeviceNode;

typedef struct List {
  struct List *next;
  struct DeviceNode *Device;
} List;

/*
*   �����ʼ��
*/
void ListInit(void);

/*
*   ����豸��������
*   �ɹ�:true
*   ʧ��:false
*/
bool AddDeviceToList(uint16_t id);

/*
*   ����ָ��ID�豸�ڵ��CMD��IDENTIFY
*   �豸�����ڷ���false
*   �豸æ���豸��������true
*/
bool SendCMDToList(uint16_t id, uint8_t cmd, uint32_t identify);

/*
*   ���������豸����Ӧ��Ϣ
*   �ɹ�:true
*   ʧ��:false
*/
void SendCMDRespToList(uint16_t id, uint8_t cmd, uint32_t identify, uint8_t resp);

/*
*   �豸������̣�����ѭ���ڵ��ã�ѭ���ж�������ÿ���豸��״̬
*   ����Ӧ��u8CMDSTATUS״̬Ϊ:
*     CMD_STATUS_IDLE       �豸����
*     CMD_STATUS_STANDBY    �豸��ָ��ȴ�ִ��
*     CMD_STATUS_RUN        �豸ָ����������
*     CMD_STATUS_DONE       �豸ָ���������
*/
void ListTask(void);


/*
*   ��ӡ������
*/
void LookAllList(void);

#endif



