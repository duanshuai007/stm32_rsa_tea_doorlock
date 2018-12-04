#include "maincontrol.h"
#include "stdint.h"
#include "stm32f1xx.h"
#include "stm32f1xx_hal.h"
#include "user_config.h"
#include "stdlib.h"
#include "crc16.h"
#include "flash.h"

extern UART_HandleTypeDef huart2;
extern DMA_HandleTypeDef hdma_usart2_rx;
extern DMA_HandleTypeDef hdma_usart2_tx;

static UartModule *gUartMx;      //f405�����豸�ṹ��
static uint8_t uart2_dma_rbuff[UART2_RX_DMA_LEN] = {0};
static uint8_t uart2_dma_sbuff[UART2_TX_DMA_LEN] = {0};

/*
*   ��ʼ����F405ͨ�ŵĴ�����ؽṹ��
*/
void UARTF405_Init(UART_HandleTypeDef *uart)
{
  gUartMx = (UartModule *)malloc(sizeof(UartModule));
  if(gUartMx == NULL) {
    PRINT("gUartMx == NULL\r\n");
    return;
  }
  
  gUartMx->pos = 0;
  gUartMx->uart = uart;
  
  gUartMx->uart_rx_hdma = &hdma_usart2_rx;
  gUartMx->uart_tx_hdma = &hdma_usart2_tx;
  
  gUartMx->dma_rbuff_size = UART2_RX_DMA_LEN;
  gUartMx->dma_sbuff_size = UART2_TX_DMA_LEN;
  
  gUartMx->dma_rbuff = &uart2_dma_rbuff[0];
  gUartMx->dma_sbuff = &uart2_dma_sbuff[0];
  
  gUartMx->rxDataPool = DataPoolInit(UART2_RX_DATAPOOL_SIZE);
  if ( gUartMx->rxDataPool == NULL ) {
    PRINT("uart2 rxdatapool null\r\n");
    free(gUartMx);
    return;
  }
  
  gUartMx->txDataPool = DataPoolInit(UART2_TX_DATAPOOL_SIZE);
  if ( gUartMx->txDataPool == NULL ) {
    PRINT("uart2 txdatapool null\r\n");
    free(gUartMx->rxDataPool);
    free(gUartMx);
    return;
  }
  
  HAL_UART_Receive_DMA(gUartMx->uart, gUartMx->dma_rbuff, gUartMx->dma_rbuff_size);
  //ʹ�ܿ����жϣ����Խ���������
  __HAL_UART_ENABLE_IT(gUartMx->uart, UART_IT_IDLE);
}

/*
*   ����resp��Ϣ
*/
static bool generalRespInfo(DeviceNode *pdev, T_Resp_Info *pRespInfo)
{
  if(NULL == pdev || NULL == pRespInfo)
  {
    PRINT("generalRespInfo failed : pdev = %p, pRespInfo = %p\n", pdev, pRespInfo);
    return false;
  }
  
  switch(pdev->u8CMD)
  {
  case CMD_MOTOR_UP:
  case CMD_MOTOR_DOWN:
    switch(pdev->u8RESP)
    {
    case 0:
      pRespInfo->resp_code = NORMAL_SUCCESS;
      break;
    case 1:
      pRespInfo->resp_code = NORMAL_DOWN;
      break;
    case 2:
      pRespInfo->resp_code = NORMAL_FORWARD;
      break;
    case 3:
      pRespInfo->resp_code = NORMAL_UP;
      break;
    case 4:
      pRespInfo->resp_code = NORMAL_BACK;
      break;
    case 9:
      pRespInfo->resp_code = NORMAL_BUSY;
      break;
    case 99:
      pRespInfo->resp_code = NORMAL_MOTOR_RUNNING;
      break;
    default:
      PRINT("generalRespInfo error : pdev->u8CMD = %d, pdev->u8Resp = %d\n", pdev->u8CMD, pdev->u8RESP);
      return false;
    }
    
    pRespInfo->resp_data.identity = pdev->u32Identify;
    break;
  case CMD_MOTOR_STATUS_GET:
    switch(pdev->u8RESP)
    {
    case 1:
      pRespInfo->resp_code = NORMAL_DOWN;
      break;
    case 2:
      pRespInfo->resp_code = NORMAL_FORWARD;
      break;
    case 3:
      pRespInfo->resp_code = NORMAL_UP;
      break;
    case 4:
      pRespInfo->resp_code = NORMAL_BACK;
      break;
    case 9:
      pRespInfo->resp_code = NORMAL_BUSY;
      break;
    default:
      PRINT("generalRespInfo error : pdev->u8Cmd = %d, pdev->u8Resp = %d\n", pdev->u8CMD, pdev->u8RESP);          
      return false;
    }
    
    pRespInfo->resp_data.identity = pdev->u32Identify;
    break;
  case CMD_BEEP_ON:
    switch(pdev->u8RESP)
    {
    case 0:
      pRespInfo->resp_code = NORMAL_SUCCESS;
      break;
    case 1:
      pRespInfo->resp_code = NORMAL_BEEP_OPEN_FAILED;
      break;
    default:
      PRINT("generalRespInfo error : pdev->u8Cmd = %d, pdev->u8Resp = %d\n", pdev->u8CMD, pdev->u8RESP);          
      return false;
    }
    pRespInfo->resp_data.identity = pdev->u32Identify;
    
    break;
  case CMD_BEEP_OFF:
    switch(pdev->u8RESP)
    {
    case 0:
      pRespInfo->resp_code = NORMAL_SUCCESS;
      break;
    case 1:
      pRespInfo->resp_code = NORMAL_BEEP_CLOSE_FAILED;
      break;
    default:
      PRINT("generalRespInfo error : pdev->u8Cmd = %d, pdev->u8Resp = %d\n", pdev->u8CMD, pdev->u8RESP);          
      return false;
    }
    pRespInfo->resp_data.identity = pdev->u32Identify;
    
    break;
  case CMD_BEEP_STATUS_GET:
    switch(pdev->u8RESP)
    {
    case 0:
      pRespInfo->resp_code = NORMAL_BEEP_STATUS_OPEN;
      break;
    case 1:
      pRespInfo->resp_code = NORMAL_BEEP_STATUS_CLOSED;
      break;
    default:
      PRINT("generalRespInfo error : pdev->u8Cmd = %d, pdev->u8Resp = %d\n", pdev->u8CMD, pdev->u8RESP);          
      return false;
    }
    
    pRespInfo->resp_data.identity = pdev->u32Identify;
    
    break;
  case CMD_ADC_GET:
//    if(pdev->u8RESP > 100)
//    {
//      PRINT("generalRespInfo error : pdev->u8Cmd = %d, pdev->u8Resp = %d\n", pdev->u8CMD, pdev->u8RESP);          
//      return false;        
//    }
//    
    pRespInfo->resp_code = pdev->u8RESP;
    pRespInfo->resp_data.identity = pdev->u32Identify;
    break;
    
  case DEVICE_ABNORMAL:
    switch(pdev->u8RESP)
    {
    case 1:
      pRespInfo->resp_code = INTERUPT_DOWN;
      break;
    case 2:
      pRespInfo->resp_code = INTERUPT_FORWARD;
      break;
    case 3:
      pRespInfo->resp_code = INTERUPT_UP;
      break;
    case 4:
      pRespInfo->resp_code = INTERUPT_BACK;
      break;
    default:
      PRINT("generalRespInfo error : pdev->u8Cmd = %d, pdev->u8Resp = %d\n", pdev->u8CMD, pdev->u8RESP);          
      return false;
    }
    pRespInfo->resp_data.endpointId = pdev->u16ID;
    break;
    
  default:
    PRINT("generalRespInfo error : pdev->u8Cmd = %d\n", pdev->u8CMD);          
    return false;
  } /* end switch(pdev->u8CMD)*/
  
  pRespInfo->crc = crc8_chk_value((uint8_t *)pRespInfo, 5);
  
  return true;
}

/*
*   ��F405����ָ����Ӧ��Ϣ���ɹ�����true��ʧ�ܷ���false
*/
bool UartSendRespToF405(DeviceNode *devn)
{
  UartModule *um = gUartMx;
  T_Resp_Info resp;
  
  if (generalRespInfo(devn, &resp)) {
    if (DataPoolWrite(um->txDataPool, (uint8_t *)&resp, sizeof(T_Resp_Info))) {
      return true;
    }
  }
  
  return false;
}

/*
*   ��F405����֪ͨ��Ϣ
*/
void UartSendMSGToF405(uint8_t status, uint16_t id, uint32_t identify)
{
  UartModule *um = gUartMx;
  T_Resp_Info resp;
  
  resp.resp_code = status;
  
  switch(status)
  {
  case NODE_ONLINE:
  case NODE_OFFLINE:
  case NODE_NOTEXIST:
    resp.resp_data.endpointId = id;
    break;
  case NORMAL_BUSY:
    resp.resp_data.identity = identify;
    break;
  default:
    break;
  }
  
  resp.crc = crc8_chk_value((uint8_t *)&resp, 5);
  DataPoolWrite(um->txDataPool, (uint8_t *)&resp, sizeof(T_Resp_Info));
}

/*
*   ��ȡuart module�ṹ��
*/
UartModule *GetF405UartModule(UART_HandleTypeDef *huart)
{
  if ( huart->Instance == gUartMx->uart->Instance ) {
    return gUartMx;
  } else 
    return NULL;
}

/*
*   �жϺ���:F405���պ���
*   �ڴ��ڽ�������ж��б�����
*/
void F405ReveiveHandler(UART_HandleTypeDef *huart)
{
  if ( huart->Instance != gUartMx->uart->Instance ) 
    return;
  
  CopyDataFromDMAHandler(gUartMx->uart);
  HAL_UART_Receive_DMA(gUartMx->uart, 
                       gUartMx->dma_rbuff,
                       gUartMx->dma_rbuff_size);
}

/*
*   F405������߳�
*   F405ֻ�ܿ����ɱ������ͳ�ȥ���豸ID����������ID�����в���
*/
void F405Task(void)
{
  DataPool *dp = gUartMx->rxDataPool;
  DataPool *tdp = gUartMx->txDataPool;
  
  T_Control_Cmd cmd;
  
  //F405������մ���
  if ( DataPoolGetNumByte(dp, (uint8_t *)(&cmd), sizeof(T_Control_Cmd)) ) {
    
    uint8_t crc = crc8_chk_value((uint8_t *)(&cmd), 7);
    
    if(crc != cmd.crc) {
      PRINT("receive cmd for F405 error: crc check error, crc = %02x, cmd.crc = %02x\r\n", crc, cmd.crc);
      return;
    }
    
    PRINT("rec from F405:id:%04x, cmd:%d, identify:%08x\r\n",
           cmd.endpointId,cmd.action,cmd.identity);
    
    if ( CMD_GET_ALL_NODE == cmd.action ) {
      //���յ�f405��������Ӧ��Ϣ�������������߽ڵ��405
      SendALLDeviceNodeToF405();
    } else {
      if (SendCMDToList(cmd.endpointId, cmd.action, cmd.identity)  != true) {
        //send device not exist msg to f405
        UartSendMSGToF405(NODE_NOTEXIST, cmd.endpointId, 0);
      }
    }
  }
  
  //F405�������ݴ���
  if ( gUartMx->uart->gState == HAL_UART_STATE_READY ) {
    if ( DataPoolGetNumByte(tdp, gUartMx->dma_sbuff, sizeof(T_Resp_Info))) {
      PRINT("send resp to f405, resp=%d\r\n",gUartMx->dma_sbuff[0]);
      HAL_UART_Transmit_DMA(gUartMx->uart, gUartMx->dma_sbuff, sizeof(T_Resp_Info));
    }
  }
}
