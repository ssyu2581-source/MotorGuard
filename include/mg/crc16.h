#ifndef MG_CRC16_H
#define MG_CRC16_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint16_t mg_crc16_modbus_update(uint16_t crc, const uint8_t *data, size_t len);
uint16_t mg_crc16_modbus(const uint8_t *data, size_t len);

#ifdef __cplusplus
}
#endif

#endif
