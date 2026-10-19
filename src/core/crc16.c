#include "mg/crc16.h"
uint16_t mg_crc16_modbus_update(uint16_t crc, const uint8_t *data, size_t len)
{
    size_t i;

    if (data == NULL) {
        return crc;
    }

    for (i = 0u; i < len; ++i) {
        uint8_t bit;
        crc ^= (uint16_t)data[i];
        for (bit = 0u; bit < 8u; ++bit) {
            if ((crc & 0x0001u) != 0u) {
                crc = (uint16_t)((crc >> 1u) ^ 0xA001u);
            } else {
                crc >>= 1u;
            }
        }
    }

    return crc;
}

uint16_t mg_crc16_modbus(const uint8_t *data, size_t len)
{
    return mg_crc16_modbus_update(0xFFFFu, data, len);
}
