#ifndef __LORA_DATAPOOL_H__
#define __LORA_DATAPOOL_H__

#include "stdint.h"
#include "user_config.h"

#pragma pack(1)
typedef struct {
  uint16_t u16Start;    //���ݳصĵȴ���ȡ�Ŀ�ʼ��ַ
  uint16_t u16End;      //�ȴ���ȡ�Ľ�����ַ
  uint16_t u16MaxSize;  //���ռ�
  bool  FullFlag;       //���ݳش�����־
  uint8_t  *pool;       //���ݳص����ݻ���ָ��
} DataPool;
#pragma pack()

/*
*   ���ݳس�ʼ��
*/
DataPool *DataPoolInit(uint16_t size);

/*
*   ������д�뵽���ݳ��У��ɹ�����true��ʧ�ܷ���false
*/
bool DataPoolWrite(DataPool *ldp, uint8_t *buf, uint16_t len);

/*
*   �����ݳ��ж�ȡ��ǰ�Ŀ�ʼ���ֽ�����
*   ���ı�start��λ��
*/
bool DataPoolLookByte(DataPool *ldp, uint8_t *data);
/*
*   �����ݳ��л�ȡ��ǰstart��������
*   startָ����һ������
*/
bool DataPoolGetByte(DataPool *ldp, uint8_t *data);
/*
*   �����ݳ���ȡ���������
*   return: ����ȡ�������ݳ���
*/
uint8_t DataPoolGetNumByte(DataPool *ldp, uint8_t *buf, uint8_t len);

/*
*   �ָ����ݳ��е�����(�ƶ�start��λ��)
*/
void DataPoolResume(DataPool *ldp, uint8_t len);

#endif



