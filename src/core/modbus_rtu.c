#include "mg/modbus_rtu.h"
#include <stddef.h>

#include "mg/crc16.h"
static uint16_t read_u16_be(const uint8_t *data)
{
    return (uint16_t)(((uint16_t)data[0] << 8u) | (uint16_t)data[1]);
}

static void write_u16_be(uint8_t *data, uint16_t value)
{
    data[0] = (uint8_t)(value >> 8u);
    data[1] = (uint8_t)(value & 0xFFu);
}

mg_modbus_status_t mg_modbus_parse_read_request(const uint8_t *frame,
                                                size_t len,
                                                mg_modbus_request_t *out)
{
    uint16_t expected_crc;
    uint16_t actual_crc;
    uint16_t count;

    if (frame == NULL || out == NULL) {
        return MG_MODBUS_ERR_NULL;
    }
    if (len != 8u) {
        return MG_MODBUS_ERR_LENGTH;
    }
    if (frame[1] != 0x03u && frame[1] != 0x04u) {
        return MG_MODBUS_ERR_FUNCTION;
    }

    expected_crc = mg_crc16_modbus(frame, 6u);
    actual_crc = (uint16_t)((uint16_t)frame[6] | ((uint16_t)frame[7] << 8u));
    if (expected_crc != actual_crc) {
        return MG_MODBUS_ERR_CRC;
    }

    count = read_u16_be(&frame[4]);
    if (count == 0u || count > 125u) {
        return MG_MODBUS_ERR_RANGE;
    }

    out->slave = frame[0];
    out->function = frame[1];
    out->start = read_u16_be(&frame[2]);
    out->count = count;
    return MG_MODBUS_OK;
}

size_t mg_modbus_build_read_response(uint8_t slave,
                                     uint8_t function,
                                     const uint16_t *registers,
                                     uint16_t count,
                                     uint8_t *out,
                                     size_t out_capacity)
{
    size_t frame_size;
    size_t index;
    uint16_t crc;

    if (out == NULL || (registers == NULL && count != 0u)) {
        return 0u;
    }
    if (function != 0x03u && function != 0x04u) {
        return 0u;
    }
    if (count == 0u || count > 125u) {
        return 0u;
    }

    frame_size = 5u + (size_t)count * 2u;
    if (out_capacity < frame_size) {
        return 0u;
    }

    out[0] = slave;
    out[1] = function;
    out[2] = (uint8_t)(count * 2u);
    for (index = 0u; index < (size_t)count; ++index) {
        write_u16_be(&out[3u + index * 2u], registers[index]);
    }

    crc = mg_crc16_modbus(out, frame_size - 2u);
    out[frame_size - 2u] = (uint8_t)(crc & 0xFFu);
    out[frame_size - 1u] = (uint8_t)(crc >> 8u);
    return frame_size;
}

size_t mg_modbus_build_exception(uint8_t slave,
                                 uint8_t function,
                                 uint8_t exception_code,
                                 uint8_t *out,
                                 size_t out_capacity)
{
    uint16_t crc;

    if (out == NULL || out_capacity < 5u) {
        return 0u;
    }

    out[0] = slave;
    out[1] = (uint8_t)(function | 0x80u);
    out[2] = exception_code;
    crc = mg_crc16_modbus(out, 3u);
    out[3] = (uint8_t)(crc & 0xFFu);
    out[4] = (uint8_t)(crc >> 8u);
    return 5u;
}

const char *mg_modbus_status_name(mg_modbus_status_t status)
{
    switch (status) {
    case MG_MODBUS_OK:
        return "OK";
    case MG_MODBUS_ERR_NULL:
        return "NULL";
    case MG_MODBUS_ERR_LENGTH:
        return "LENGTH";
    case MG_MODBUS_ERR_CRC:
        return "CRC";
    case MG_MODBUS_ERR_FUNCTION:
        return "FUNCTION";
    case MG_MODBUS_ERR_RANGE:
        return "RANGE";
    default:
        return "UNKNOWN";
    }
}
