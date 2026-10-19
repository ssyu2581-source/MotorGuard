#ifndef MG_BSP_H
#define MG_BSP_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    bool (*read_current)(float *amps);
    bool (*read_vibration)(float *g);
    bool (*read_rpm)(float *rpm);
    bool (*read_temperature)(float *celsius);
    void (*set_motor_duty)(float duty);
    void (*set_relay)(bool closed);
} mg_bsp_t;

#ifdef __cplusplus
}
#endif

#endif