#include "app_controller.h"

#include <stddef.h>

void mg_app_controller_init(mg_app_controller_t *controller,
                            const mg_bsp_t *bsp,
                            const mg_fault_thresholds_t *thresholds)
{
    if (controller == NULL) {
        return;
    }

    controller->bsp = bsp;
    mg_fault_detector_init(&controller->detector, thresholds);
    controller->active_fault = MG_FAULT_NONE;
    controller->output_enabled = false;

    if (bsp != NULL && bsp->set_relay != NULL) {
        bsp->set_relay(false);
    }
}

mg_fault_code_t mg_app_controller_update(mg_app_controller_t *controller,
                                         const mg_signal_features_t *current_features,
                                         const mg_signal_features_t *vibration_features,
                                         float temperature_c,
                                         float rpm,
                                         bool current_valid,
                                         bool vibration_valid)
{
    mg_fault_code_t fault;

    if (controller == NULL) {
        return MG_FAULT_NONE;
    }
    if (controller->bsp == NULL) {
        controller->active_fault = MG_FAULT_SENSOR;
        controller->output_enabled = false;
        return MG_FAULT_SENSOR;
    }

    fault = mg_fault_detector_update(&controller->detector,
                                     current_features,
                                     vibration_features,
                                     temperature_c,
                                     rpm,
                                     current_valid,
                                     vibration_valid);
    controller->active_fault = fault;
    controller->output_enabled = (fault == MG_FAULT_NONE);

    if (controller->bsp->set_relay != NULL) {
        controller->bsp->set_relay(controller->output_enabled);
    }
    if (!controller->output_enabled && controller->bsp->set_motor_duty != NULL) {
        controller->bsp->set_motor_duty(0.0f);
    }

    return fault;
}