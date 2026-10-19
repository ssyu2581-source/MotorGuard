#ifndef MG_FAULT_DETECTOR_H
#define MG_FAULT_DETECTOR_H

#include <stdbool.h>
#include <stdint.h>

#include "mg/features.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    MG_FAULT_NONE = 0,
    MG_FAULT_SENSOR,
    MG_FAULT_OVERLOAD,
    MG_FAULT_OVERTEMP,
    MG_FAULT_OVERSPEED,
    MG_FAULT_STALL,
    MG_FAULT_BEARING,
    MG_FAULT_IMBALANCE
} mg_fault_code_t;

typedef struct {
    float overload_current_a;
    float overtemp_c;
    float overspeed_rpm;
    float stall_rpm;
    float stall_current_a;
    float vibration_rms_limit_g;
    float imbalance_1x_ratio;
    float bearing_high_freq_ratio;
    uint32_t persistence_frames;
    uint32_t clear_frames;
} mg_fault_thresholds_t;

typedef struct {
    mg_fault_thresholds_t thresholds;
    mg_fault_code_t active_fault;
    mg_fault_code_t candidate_fault;
    uint32_t candidate_count;
    uint32_t clear_count;
} mg_fault_detector_t;

void mg_fault_thresholds_default(mg_fault_thresholds_t *thresholds);
void mg_fault_detector_init(mg_fault_detector_t *detector, const mg_fault_thresholds_t *thresholds);
mg_fault_code_t mg_fault_detector_update(mg_fault_detector_t *detector,
                                         const mg_signal_features_t *current_features,
                                         const mg_signal_features_t *vibration_features,
                                         float temperature_c,
                                         float rpm,
                                         bool current_valid,
                                         bool vibration_valid);
const char *mg_fault_code_name(mg_fault_code_t code);

#ifdef __cplusplus
}
#endif

#endif
