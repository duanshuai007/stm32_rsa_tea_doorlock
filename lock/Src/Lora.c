#include "Lora.h"
#include "stdint.h"
#include "stm32f1xx.h"
#include "user_config.h"
#include "string.h"
#include "rsa_16.h"
#include "flash.h"

extern UART_HandleTypeDef huart1;
extern DMA_HandleTypeDef hdma_usart1_rx;

static LoraModule gLoraModule;
static uint8_t dma_uart1_rbuff[UART1_RX_DMA_LEN] = {0};

/*
*   通过串口获取loramodule结构体
*   usart1对应接收lora，usart3对应发送lora
*/
static LoraModule *get_loramodule(void)
{
  return &gLoraModule;
}

/*
*   根据串口获取对应的模块指针
*/
static UartModule *get_uartmodule(void)
{
  return &gLoraModule.muart;
}

/*
*   通过串口获取对应lora模块工作状态
*   返回的是AUX电平状态，低电平表示繁忙，高电平表示空闲
*/
static bool get_loramodule_idle_flag(void)
{
  return gLoraModule.isIdle;
}

/*
*   判断lora模块的引脚电平来设置lora的空闲状态
*/
void SetLoraModuleIdleFlagHandler(uint16_t pin)
{
  LoraModule *lm = get_loramodule();
  
  if ( lm->gpio.AUX.Pin == pin ) {
    if ( HAL_GPIO_ReadPin(lm->gpio.AUX.GPIOX, lm->gpio.AUX.Pin) == GPIO_PIN_SET ) {
      lm->isIdle = true;
    } else {
      lm->isIdle = false;
    }
  }
}

/*
*   设置Lora模块的工作状态
*/
static bool set_loramodule_workmode(LoraModule *lp, Lora_Mode mode)
{
#define LORA_SET_MAX_TIME 	500
  
  uint16_t delay = 0;
  
  lp->mode = mode;
  
  HAL_Delay(5);       //必须的延时，至少5ms
  
  switch(lp->mode)
  {
  case LoraMode_Normal:
    HAL_GPIO_WritePin(lp->gpio.M0.GPIOX, lp->gpio.M0.Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(lp->gpio.M1.GPIOX, lp->gpio.M1.Pin, GPIO_PIN_RESET);
    break;
  case LoraMode_WakeUp:
    HAL_GPIO_WritePin(lp->gpio.M0.GPIOX, lp->gpio.M0.Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(lp->gpio.M1.GPIOX, lp->gpio.M1.Pin, GPIO_PIN_RESET);
    break;
  case LoraMode_LowPower:
    HAL_GPIO_WritePin(lp->gpio.M0.GPIOX, lp->gpio.M0.Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(lp->gpio.M1.GPIOX, lp->gpio.M1.Pin, GPIO_PIN_SET);
    break;
  case LoraMode_Sleep:
    HAL_GPIO_WritePin(lp->gpio.M0.GPIOX, lp->gpio.M0.Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(lp->gpio.M1.GPIOX, lp->gpio.M1.Pin, GPIO_PIN_SET);
    break;
  default:
    return false;
  }
  
  do {
    HAL_Delay(2);   //必须的do{}while(...)循环，先执行一遍延时
    delay++;
  } while((!get_loramodule_idle_flag()) && (delay < LORA_SET_MAX_TIME));
  
  if(delay >= LORA_SET_MAX_TIME)
    return false;
  else
    return true;
}

/*
*   设置lora模块所用到的引脚
*/
void LoraModuleGPIOInit(UART_HandleTypeDef *huart)
{
  LoraModule *lm = get_loramodule();
  
  memset(lm, 0, sizeof(LoraModule));
  
  lm->gpio.AUX.GPIOX      = GPIO_Lora_AUX;
  lm->gpio.AUX.Pin        = GPIO_Lora_AUX_Pin;
  lm->gpio.M0.GPIOX       = GPIO_Lora_M0;
  lm->gpio.M0.Pin         = GPIO_Lora_M0_Pin;
  lm->gpio.M1.GPIOX       = GPIO_Lora_M1;
  lm->gpio.M1.Pin         = GPIO_Lora_M1_Pin;
  
  lm->muart.uart = huart;
}

/*
*   设置lora模块的dma，dma接收发送缓冲区，datapool等参数
*/
void LoraModuleDMAInit(UART_HandleTypeDef *huart)
{
  LoraModule *lm = get_loramodule();
  /*
  *   UART1 作为Lora接收串口
  */
#ifdef UART1_RECV_USE_DMA
  lm->muart.dma_rbuff_size      = UART1_RX_DMA_LEN;
  lm->muart.dma_rbuff           = &dma_uart1_rbuff[0];
  lm->muart.uart_rx_hdma        = &hdma_usart1_rx;
  lm->muart.rxDataPool = DataPoolInit(UART1_RX_DATAPOOL_SIZE);
  if ( lm->muart.rxDataPool == NULL ) {
    PRINT("uart1 rxdatapool NULL\r\n");
  }
  HAL_UART_Receive_DMA(lm->muart.uart, lm->muart.dma_rbuff, lm->muart.dma_rbuff_size);
  //使能空闲中断，仅对接收起作用
  __HAL_UART_ENABLE_IT(lm->muart.uart, UART_IT_IDLE);
#endif
  
#ifdef UART1_SEND_USE_DMA
  lm->muart.dma_sbuff_size      = UART1_TX_DMA_LEN;
  lm->muart.dma_sbuff           = &dma_uart1_sbuff[0];
  lm->muart.uart_tx_hdma        = &hdma_usart1_tx;
  lm->muart.txDataPool = DataPoolInit(UART1_TX_DATAPOOL_SIZE);
  if ( lm->muart.rxDataPool == NULL ) {
    PRINT("uart1 rxdatapool NULL\r\n");
  }
#endif
  
  set_loramodule_workmode( lm, LoraMode_LowPower);
}

/*
*   Generate Lora Paramter
*	  根据参数生成数据：成功返回0，失败返回1
*/
static bool generate_loramodule_paramter(LoraPar *lp, uint8_t *buff, SaveType st)
{
  if (SAVE_IN_FLASH == st) {
    buff[0] = 0xC0;
  } else {
    buff[0] = 0xC2;
  }
  
  buff[1] = (uint8_t)((lp->u16Addr & 0xff00) >> 8);
  buff[2] = (uint8_t)(lp->u16Addr);
  
  switch(lp->u8Parity)
  {
  case PARITY_8N1:
  case PARITY_8N1_S:
    buff[3] |= 0 << 6;
    break;
  case PARITY_8O1:
    buff[3] |= 1 << 6;
    break;
  case PARITY_8E1:
    buff[3] |= 2 << 6;
    break;
  default:
    return false;
  }
  
  switch(lp->u8Baud)
  {
  case BAUD_1200:
    buff[3] |= 0 << 3;
    break;
  case BAUD_2400:
    buff[3] |= 1 << 3;
    break;
  case BAUD_4800:
    buff[3] |= 2 << 3;
    break;
  case BAUD_9600:
    buff[3] |= 3 << 3;
    break;
  case BAUD_19200:
    buff[3] |= 4 << 3;
    break;
  case BAUD_38400:
    buff[3] |= 5 << 3;
    break;
  case BAUD_57600:
    buff[3] |= 6 << 3;
    break;
  case BAUD_115200:
    buff[3] |= 7 << 3;
    break;
  default:
    return false;
  }
  
  switch(lp->u8Speedinair)
  {
  case SPEED_IN_AIR_0_3K:
    buff[3] |= 0;
    break;
  case SPEED_IN_AIR_1_2K:
    buff[3] |= 1;
    break;
  case SPEED_IN_AIR_2_4K:
    buff[3] |= 2;
    break;
  case SPEED_IN_AIR_4_8K:
    buff[3] |= 3;
    break;
  case SPEED_IN_AIR_9_6K:
    buff[3] |= 4;
    break;
  case SPEED_IN_AIR_19_2K:
  case SPEED_IN_AIR_19_2K_1:
  case SPEED_IN_AIR_19_2K_2:
    buff[3] |= 5;
    break;
  default:
    return false;
  }
  
  if(lp->u8Channel > CHAN_441MHZ)
    return false;
  
  buff[4] = lp->u8Channel;
  
  switch(lp->u8TranferMode)
  {
  case TRANSFER_MODE_TOUMING:
    buff[5] |= 0 << 7;
    break;
  case TRANSFER_MODE_DINGDIAN:
    buff[5] |= 1 << 7;
    break;
  default:
    return false;
  }
  
  switch(lp->u8IOMode)
  {
  case IOMODE_PP:
    buff[5] |= 1 << 6;
    break;
  case IOMODE_OD:
    buff[5] |= 0 << 6;
    break;
  default:
    return false;
  }
  
  switch(lp->u8WakeUpTime)
  {
  case WAKEUP_TIME_250MS:
    buff[5] |= 0 << 3;
    break;
  case WAKEUP_TIME_500MS:
    buff[5] |= 1 << 3;
    break;
  case WAKEUP_TIME_750MS:
    buff[5] |= 2 << 3;
    break;
  case WAKEUP_TIME_1000MS:
    buff[5] |= 3 << 3;
    break;
  case WAKEUP_TIME_1250MS:
    buff[5] |= 4 << 3;
    break;
  case WAKEUP_TIME_1500MS:
    buff[5] |= 5 << 3;
    break;
  case WAKEUP_TIME_1750MS:
    buff[5] |= 6 << 3;
    break;
  case WAKEUP_TIME_2000MS:
    buff[5] |= 7 << 3;
    break;
  default:
    return false;
  }
  
  if (lp->u8FEC) {
    buff[5] |= 1 << 2;
  } else {
    buff[5] |= 0 << 2;
  }
  
  switch(lp->u8SendDB)
  {
  case SEND_20DB:
    buff[5] |= 0;
    break;
  case SEND_17DB:
    buff[5] |= 1;
    break;
  case SEND_14DB:
    buff[5] |= 2;
    break;
  case SEND_10DB:
    buff[5] |= 3;
    break;
  default:
    return false;
  }
  
  return true;
}

/*
*   设置lora模块的参数
*/
static bool set_loramodule_paramter(LoraModule *lp)
{
#define LORA_SET_PAR_MAX_TIME 	100
  uint8_t config[6] = {0};
  
  if(lp == NULL || &lp->paramter == NULL)
    return false;
  
  if (!generate_loramodule_paramter(&lp->paramter, config, SAVE_IN_FLASH))
    return false;
  
  //必要的延时函数
  HAL_Delay(10);
  
  if (HAL_UART_Transmit(lp->muart.uart, config, 6, 100) == HAL_OK) {
    memset( config, 0, 6);
    if (HAL_UART_Receive(lp->muart.uart, config, 6, 1000) == HAL_OK) { 
      PRINT("rec:%02x-%02x-%02x-%02x-%02x-%02x\r\n",
             config[0], config[1],config[2],
             config[3],config[4],config[5]);
      return true;
    }
    return false;
  }
  
  return false;
}

/*
*     解析接受到的lora模块的参数
*/
static void get_loramodule_paramter(LoraPar *lp, uint8_t *buff)
{
  lp->u16Addr         = (((uint16_t)buff[1] << 8) | buff[2]);
  /*
  *   7       6       5       4       3       2       1       0
  *   [ parity ]      [   ttl   baud  ]       [ speed in air  ]
  */
  lp->u8Parity        = (buff[3] & 0xC0) >> 6;
  lp->u8Baud          = (buff[3] & 0x38) >> 3;
  lp->u8Speedinair    = (buff[3] & 0x07);
  
  lp->u8Channel       = buff[4];
  /*
  *       7         6       5  4  3       2        1    0
  * [TransMode] [IOMode] [ WakeUp time]  [fec]   [ send db]
  */
  lp->u8TranferMode   = (buff[5] & 0x80) >> 7;
  lp->u8IOMode        = (buff[5] & 0x40) >> 6;
  lp->u8WakeUpTime    = (buff[5] & 0x38) >> 3;
  lp->u8FEC           = (buff[5] & 0x04) >> 2;
  lp->u8SendDB        = (buff[5] & 0x03);
}

/*
*       Read Lora Version or Paramter
*       
*       no = LoraReadPar:读取模块参数
*       no = LoraReadVer:读取模块版本信息
*       PS:在发送了读取版本号命令之后不能再发送命令
*/
static bool read_loramodule(uint8_t no, LoraModule *lp)
{
#define LORA_READ_PAR_MAX_TIME 100
  uint8_t cmd[3] = {0xC1, 0xC1, 0xC1};
  uint8_t buffer[6] = {0};
 
  if (HAL_UART_Transmit(lp->muart.uart, cmd, 3, 100) == HAL_OK) {
    if ( HAL_UART_Receive(lp->muart.uart, buffer, 6, 1000) == HAL_OK ) {
      PRINT("read:%02x-%02x-%02x-%02x-%02x-%02x\r\n",
             buffer[0],buffer[1],buffer[2],
             buffer[3],buffer[4],buffer[5]);
      get_loramodule_paramter(&lp->paramter, buffer);
      return true;
    }
    return false;
  }
  
  return false;
}

static void uart_set_9600_8n1(LoraModule *lm)
{
  __HAL_UART_DISABLE(lm->muart.uart);
  lm->muart.uart->Init.BaudRate = 9600;
  lm->muart.uart->Init.WordLength = UART_WORDLENGTH_8B;
  lm->muart.uart->Init.Parity = UART_PARITY_NONE;
  if (HAL_UART_Init(lm->muart.uart) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }
  HAL_Delay(10);
}

static void uart_set_baud_parity(LoraModule *lm)
{
  uint32_t baud_array[8] = {1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200};
  
  __HAL_UART_DISABLE(lm->muart.uart);
  lm->muart.uart->Init.BaudRate = baud_array[lm->paramter.u8Baud];
  if (( PARITY_8N1 == lm->paramter.u8Parity) || ( PARITY_8N1_S == lm->paramter.u8Parity)) {
    //无校验
    lm->muart.uart->Init.WordLength = UART_WORDLENGTH_8B;
    lm->muart.uart->Init.Parity = UART_PARITY_NONE;
  } else if ( PARITY_8O1 == lm->paramter.u8Parity ) {
    lm->muart.uart->Init.WordLength = UART_WORDLENGTH_9B;
    lm->muart.uart->Init.Parity = UART_PARITY_ODD;
  } else if ( PARITY_8E1 == lm->paramter.u8Parity ) {
    lm->muart.uart->Init.WordLength = UART_WORDLENGTH_9B;
    lm->muart.uart->Init.Parity = UART_PARITY_EVEN;
  }
  if (HAL_UART_Init(lm->muart.uart) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }
  HAL_Delay(10);
}

/*
*   设置Lora模块参数
*/
void SetLoraParamter(UART_HandleTypeDef *huart, LoraPar *lp)
{
  
  LoraModule *lm = get_loramodule();
  
  uart_set_9600_8n1(lm);
  //进入配置模式,在配置模式下，不能带校验
  set_loramodule_workmode(lm, LoraMode_Sleep);
  //写入配置参数
  memcpy(&lm->paramter, (void *)lp, sizeof(LoraPar));
  
  while(!set_loramodule_paramter(lm));
  
  uart_set_baud_parity(lm);
  set_loramodule_workmode(lm, LoraMode_Normal);
  memset(&lm->paramter, 0, sizeof(LoraPar));
}

/*
*   读取Lora模块参数
*/
void ReadLoraParamter(UART_HandleTypeDef *huart)
{
  LoraModule *lm = get_loramodule();
  
  uart_set_9600_8n1(lm);
  set_loramodule_workmode(lm, LoraMode_Sleep);
  while(!read_loramodule(LoraReadPar, lm));
  uart_set_baud_parity(lm);
  set_loramodule_workmode(lm, LoraMode_Normal);
}

#ifdef UART1_RECV_USE_DMA
/*
*   中断函数：当被调用时会将对应的串口DMA缓冲区中的
*             数据复制到对应串口自己的数据池中。
*/
bool CopyDataFromDMAHandler(UART_HandleTypeDef *huart)
{
  uint16_t len;
  uint16_t pos;
  UartModule *um = get_uartmodule();
  
  //len 等于已经用于存放数据的长度(空间)
  len = (um->dma_rbuff_size - (uint16_t)__HAL_DMA_GET_COUNTER(um->uart_rx_hdma));

  if (((0 == len) && (0 == um->pos)) || (len == um->pos))
    return true;
  
  if (( 0 == len ) && ( um->pos > 0)) {
    //数据接收完成中断，将数据写入pool中后，将pos重置。
    pos = um->pos;

    if (DataPoolWrite(um->rxDataPool, um->dma_rbuff + pos, um->dma_rbuff_size - pos )) {
      um->pos = 0;
      return true;
    }
  } else  if (len > um->pos) {
    //空闲中断，取出当前接收的数据，然后重新设置pos值
    pos = um->pos;
    //len是整个数据空间已有数据的总长度
    //pos是上一次取完数据后的指针位置
    //所有本次数据长度就是len-pos
    if (DataPoolWrite(um->rxDataPool, um->dma_rbuff + pos, len - pos)) { 
      um->pos += (len - pos);
      if( um->pos == um->dma_rbuff_size ) 
        um->pos = 0;
      return true;
    }
  }
  
  return false;
}
#endif

/*
*   中断函数:用于DMA数据移动和重新使能dma接收功能
*   在串口接收完成中断中调用
*/
void LoraModuleReceiveHandler(UART_HandleTypeDef *huart) 
{
  LoraModule *lm = get_loramodule();
  
  if ( huart->Instance != lm->muart.uart->Instance )
    return;
  
#ifdef UART1_RECV_USE_DMA
  CopyDataFromDMAHandler(lm->muart.uart);
  HAL_UART_Receive_DMA(lm->muart.uart,
                       lm->muart.dma_rbuff,
                       lm->muart.dma_rbuff_size);
#endif
}

static uint8_t parsingdata(char *buff)
{
  char *str;
  
  if ((str = strstr(buff, "WM+"))) {
    if((strstr(str, "OD+"))) {
      //open door
      return 1;
    } else if (strstr(buff, "CD+")) {
      //close door
      return 2;
    } else {
      return 0;
    }
  }
  
  return 0;
}

/*
*   Lora模块的数据处理
*/
void LoraModuleTask(void)
{
  static uint8_t count = 0; //用来统计数据不足情况下的时间
  uint8_t buff[64] = {0};   //保存从数据池中取得的数据  
  
  //接收数据的处理
  LoraModule *lm = &gLoraModule;
  DataPool *dp = lm->muart.rxDataPool;
  
  if (DataPoolGetNumByte(dp, &buff[0], 38)) {
    char bull[128] = {0};     //接收到的数据是去零数据，需要在对应位置填充零
    char plaintext[128] = {0};//解密后得到的字符串数据
    uint8_t i, j, ret;    
    count = 0;
    for(i=0,j=0; i < 38; i+=2,j++) {
      bull[j*4 + 0] = buff[i];
      bull[j*4 + 1] = buff[i+1];
      bull[j*4 + 2] = 0;
      bull[j*4 + 3] = 0;
    }
    RSA_Decrypt((int *)bull, 80, plaintext );
    ret = parsingdata(plaintext);
    if (ret == 1) {
      //led ctrl
      HAL_GPIO_WritePin(GPIO_LED, GPIO_LED_Pin, GPIO_PIN_SET);
      HAL_GPIO_WritePin(GPIO_LOCK, GPIO_LOCK_Pin, GPIO_PIN_SET);
    } else if ( ret == 2 ) {
      HAL_GPIO_WritePin(GPIO_LED, GPIO_LED_Pin, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(GPIO_LOCK, GPIO_LOCK_Pin, GPIO_PIN_RESET);
    }
  } else {
    count++;
    HAL_Delay(1);
    if (count > 100) {
      count = 0;
      dp->u16Start = dp->u16End;
    }
  }
}
