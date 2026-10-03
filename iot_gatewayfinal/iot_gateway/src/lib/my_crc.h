
#ifndef _CRC_H
#define _CRC_H
#include "common.h"

#define CRC16_SEED      ((uint16_t)0xFFFF)
#define CRC_GPOLY_CCITT ((uint16_t)0x1021)
#define CRC8_SEED       ((uint16_t)0x0000)
#define CRC8_GPOLY_CCITT ((uint16_t)0x531)

extern uint16_t CRCCalc16( uint8_t aucData[], uint16_t usInputLength);
extern uint8_t CRCCheck16( uint8_t *aucInput, uint16_t usInputLength, uint16_t usOriginalCRC);
uint16_t UpdateCRCCalc16(uint16_t  usCRC16,  uint8_t *aucData, uint16_t usInputLength);
uint8_t validate_crc(uint8_t *date_ptr, uint16_t length);
void calculate_and_fill_crc(uint8_t *data_ptr, uint16_t length);
#endif
