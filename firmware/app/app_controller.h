#ifndef MG_APP_CONTROLLER_H
#define MG_APP_CONTROLLER_H

#include <stdbool.h>

#include "bsp/bsp.h"
#include "mg/fault_detector.h"
#include "mg/features.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const mg_bsp_t *bsp;
    mg_fault_detector_t detector;
    mg_fault_code_t active_fault;
    bool output_enabled;
} mg_app_controller_t;

void mg_app_controller_init(mg_app_controller_t *controller,
                            const mg_bsp_t *bsp,
                            const mg_fault_thresholds_t *thresholds);

mg_fault_code_t mg_app_controller_update(mg_app_controller_t *controller,
                                         const mg_signal_features_t *current_features,
                                         const mg_signal_features_t *vibration_features,
                                         float temperature_c,
                                         float rpm,
                                         bool current_valid,
                                         bool vibration_valid);

#ifdef __cplusplus
}
#endif

#endif