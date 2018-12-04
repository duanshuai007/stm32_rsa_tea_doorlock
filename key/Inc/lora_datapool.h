#ifndef __LORA_DATAPOOL_H__
#define __LORA_DATAPOOL_H__

#include "stdint.h"
#include "user_config.h"

#pragma pack(1)
typedef struct {
  uint16_t u16Start;    //数据池的等待读取的开始地址
  uint16_t u16End;      //等待读取的结束地址
  uint16_t u16MaxSize;  //最大空间
  bool  FullFlag;       //数据池存满标志
  uint8_t  *pool;       //数据池的数据缓冲指针
} DataPool;
#pragma pack()

/*
*   数据池初始化
*/
DataPool *DataPoolInit(uint16_t size);

/*
*   将数据写入到数据池中，成功返回true，失败返回false
*/
bool DataPoolWrite(DataPool *ldp, uint8_t *buf, uint16_t len);

/*
*   从数据池中读取当前的开始处字节数据
*   不改变start的位置
*/
bool DataPoolLookByte(DataPool *ldp, uint8_t *data);
/*
*   从数据池中获取当前start处的数据
*   start指向下一个数据
*/
bool DataPoolGetByte(DataPool *ldp, uint8_t *data);
/*
*   从数据池中取出多个数据
*   return: 返回取出的数据长度
*/
uint8_t DataPoolGetNumByte(DataPool *ldp, uint8_t *buf, uint8_t len);

/*
*   恢复数据池中的数据(移动start的位置)
*/
void DataPoolResume(DataPool *ldp, uint8_t len);

#endif



