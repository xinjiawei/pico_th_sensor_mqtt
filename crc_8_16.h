//
// Created by XINJIAWEI on 24-7-31.
//

#ifndef __CRC8_16_H
#define __CRC8_16_H
#include <cstdint>

uint16_t Get_Crc16(uint8_t *puchMsg,uint16_t usDataLen);
uint8_t Get_Crc8(uint8_t *ptr,uint16_t len);
uint16_t CRC16(uint8_t* pdata, uint16_t datalen);
#endif