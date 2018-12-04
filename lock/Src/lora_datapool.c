#include "lora_datapool.h"
#include "stdint.h"
#include "stm32f1xx.h"
#include "stdlib.h"
#include "string.h"
#include "user_config.h"

/*
*   数据池初始化
*/
DataPool *DataPoolInit(uint16_t size) 
{
  DataPool *ldp = (DataPool *)malloc(sizeof(DataPool));
  if ( ldp == NULL ) {
    PRINT("ldp null\r\n");
    return NULL;
  }
  memset(ldp, 0, sizeof(DataPool));
  
  ldp->pool = (uint8_t *)malloc(sizeof(uint8_t) * size);
  if ( ldp->pool == NULL ) {
    PRINT("ldp->pool null\r\n");
    free(ldp);
    return NULL;
  }
  memset(ldp->pool, 0, size * sizeof(uint8_t));
  
  ldp->u16End = 0;
  ldp->u16Start = 0;
  ldp->u16MaxSize = size;
  ldp->FullFlag = false;
  
  return ldp;
}

/*
*   将数据写入到数据池中
*/
bool DataPoolWrite(DataPool *ldp, uint8_t *buf, uint16_t len)
{
  //如果end在start之前
  if ( ldp->u16End < ldp->u16Start ) {
    //如果剩余的空间不能容纳想要写入的数据长度
    if (ldp->u16End + len > ldp->u16Start ) 
      return false;
  } else if ( ldp->u16End > ldp->u16Start ) {
    //如果end在start之后
    if ( ldp->u16End + len > ldp->u16MaxSize ) {
      //如果end加上待写入的数据长度后超出最大范围
      if ( len - (ldp->u16MaxSize - ldp->u16End) > ldp->u16Start ) {
        //如果数据长度超过了start的位置，则失败
        return false;
      }
    }
  } else {
    if (ldp->FullFlag)
      return false;
  }
  
  for (uint16_t i = 0; i < len; i++ ) {
    ldp->pool[ldp->u16End++] = buf[i];
    if ( ldp->u16End == ldp->u16MaxSize ) {
      //到达缓冲池的末尾
      ldp->u16End = 0;
    }
    if ( ldp->u16End == ldp->u16Start ) {
      //数据已满，不能存入新的数据,返回已经存入的数据长度
      ldp->FullFlag = true;
      break;
    }
  }
  
  return true;
}

/*
*   从数据池中读取当前的开始处字节数据
*   不改变start的位置
*   返回值0:未读取到数据
*   返回值1:读取到数据
*/
bool DataPoolLookByte(DataPool *ldp, uint8_t *data)
{   
  if ((ldp->u16Start == ldp->u16End) && (ldp->FullFlag == false))
    return false;
  
  *data = ldp->pool[ldp->u16Start];
  
  return true;
}

/*
*   从数据池中获取当前start处的数据
*   start指向下一个数据
*/
bool DataPoolGetByte(DataPool *ldp, uint8_t *data)
{
  if ((ldp->u16Start == ldp->u16End) && (ldp->FullFlag == false))
    return false;
  
  *data = ldp->pool[ldp->u16Start++];
  
  if (ldp->u16Start == ldp->u16MaxSize) {
    ldp->u16Start = 0;
  }
  if(ldp->FullFlag == true)
    ldp->FullFlag = false;
  
  return true;
}

/*
*   从数据池中取出多个数据
*   return: 返回取出的数据长度,0表示出错
*/
uint8_t DataPoolGetNumByte(DataPool *ldp, uint8_t *buf, uint8_t len)
{
  uint16_t u16End = ldp->u16End;
  uint8_t i;
  
  if ((ldp->u16Start == u16End) && (ldp->FullFlag == false)) {
    return 0;
  } else if (ldp->u16Start < u16End) {
    //正常的写入数据，此刻start在end之前
    if (u16End - ldp->u16Start < len) { 
      //数据不满足长度
      return 0;
    }
  } else if (ldp->u16Start > u16End) {
    if ( ldp->u16MaxSize - ldp->u16Start + u16End < len ) {
      //数据长度不够
      return 0;
    }
  }
  
  for(i = 0; i < len; i++) {
    
    buf[i] = ldp->pool[ldp->u16Start++];
    
    if(ldp->u16Start == ldp->u16MaxSize)
      ldp->u16Start = 0;
    
    if(ldp->u16Start == ldp->u16End) {
      i++;
      break;
    }
  }
  
  if(ldp->FullFlag == true)
    ldp->FullFlag = false;
  
  return i;
}

/*
*   恢复数据池中的数据(移动start的位置)
*/
void DataPoolResume(DataPool *ldp, uint8_t len)
{
  if (ldp->u16Start >= len) {
    ldp->u16Start -= len;
  } else {
    ldp->u16Start = (ldp->u16MaxSize - (len - ldp->u16Start));
  }
  
  if (ldp->u16Start == ldp->u16End )
    ldp->FullFlag = true;
}

