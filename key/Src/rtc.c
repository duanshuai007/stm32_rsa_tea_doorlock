#include "rtc.h"
#include "stdint.h"
#include "stm32f1xx.h"

extern RTC_HandleTypeDef hrtc;

/*
*   返回系统启动以来的秒数，最大是23×60×60即一天的秒总数
*/
uint32_t GetRTCTime(void)
{
  uint32_t time;
  RTC_TimeTypeDef sTime;
  
  HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
  
//  printf("rtc:%d-%d-%d\r\n", sTime.Hours, sTime.Minutes, sTime.Seconds);

  time = ((sTime.Minutes * 60) + sTime.Seconds);

  return time;
}

/*
*   获取系统时间，返回的是特殊格式时间
*   0xaabb
*   aa 是16进制的分钟数
*   bb 是16进制的秒数
*/
uint16_t GetRTCTimeMinAndSec(void)
{
  uint16_t time;
  
  RTC_TimeTypeDef sTime;
	
  HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);

  time = (((uint16_t)(sTime.Minutes) << 4) | sTime.Seconds);

  return time;
}




