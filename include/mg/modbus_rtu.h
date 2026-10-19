#ifndef MG_MODBUS_RTU_H
#define MG_MODBUS_RTU_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    MG_MODBUS_OK = 0,
    MG_MODBUS_ERR_NULL,
    MG_MODBUS_ERR_LENGTH,
    MG_MODBUS_ERR_CRC,
    MG_MODBUS_ERR_FUNCTION,
    MG_MODBUS_ERR_RANGE
} mg_modbus_status_t;

typedef struct {
    uint8_t slave;
    uint8_t function;
    uint16_t start;
    uint16_t count;
} mg_modbus_request_t;

mg_modbus_status_t mg_modbus_parse_read_request(const uint8_t *frame,
                                                size_t len,
                                                mg_modbus_request_t *out);

size_t mg_modbus_build_read_response(uint8_t slave,
                                     uint8_t function,
                                     const uint16_t *registers,
                                     uint16_t count,
                                     uint8_t *out,
                                     size_t out_capacity);

size_t mg_modbus_build_exception(uint8_t slave,
                                 uint8_t function,
                                 uint8_t exception_code,
                                 uint8_t *out,
                                 size_t out_capacity);

const char *mg_modbus_status_name(mg_modbus_status_t status);

#ifdef __cplusplus
}
#endif

#endif
