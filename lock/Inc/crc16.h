#ifndef __CRC16_H__
#define __CRC16_H__

#include "stdint.h"

uint16_t CRC16_IBM(uint8_t *Msg, uint8_t Len);
uint8_t crc8_chk_value(uint8_t *message, uint8_t len);

#endif



