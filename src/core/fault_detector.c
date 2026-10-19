#include "mg/fault_detector.h"
#include <math.h>
#include <stddef.h>

void mg_fault_thresholds_default(mg_fault_thresholds_t *thresholds)
{
    if (thresholds == NULL) {
        return;
    }

    thresholds->overload_current_a = 4.0f;
    thresholds->overtemp_c = 75.0f;
    thresholds->overspeed_rpm = 3500.0f;
    thresholds->stall_rpm = 100.0f;
    thresholds->stall_current_a = 1.0f;
    thresholds->vibration_rms_limit_g = 0.15f;
    thresholds->imbalance_1x_ratio = 0.45f;
    thresholds->bearing_high_freq_ratio = 0.20f;
    thresholds->persistence_frames = 5u;
    thresholds->clear_frames = 10u;
}

void mg_fault_detector_init(mg_fault_detector_t *detector, const mg_fault_thresholds_t *thresholds)
{
    if (detector == NULL) {
        return;
    }

    if (thresholds != NULL) {
        detector->thresholds = *thresholds;
    } else {
        mg_fault_thresholds_default(&detector->thresholds);
    }

    detector->active_fault = MG_FAULT_NONE;
    detector->candidate_fault = MG_FAULT_NONE;
    detector->candidate_count = 0u;
    detector->clear_count = 0u;
}

static bool inputs_are_finite(const mg_signal_features_t *current_features,
                              const mg_signal_features_t *vibration_features,
                              float temperature_c,
                              float rpm)
{
    return isfinite(current_features->rms) &&
           isfinite(current_features->peak) &&
           isfinite(vibration_features->rms) &&
           isfinite(vibration_features->peak) &&
           isfinite(vibration_features->harmonic_1x_ratio) &&
           isfinite(vibration_features->high_freq_ratio) &&
           isfinite(temperature_c) &&
           isfinite(rpm);
}

static mg_fault_code_t classify(const mg_fault_detector_t *detector,
                                const mg_signal_features_t *current_features,
                                const mg_signal_features_t *vibration_features,
                                float temperature_c,
                                float rpm,
                                bool current_valid,
                                bool vibration_valid)
{
    const mg_fault_thresholds_t *thresholds = &detector->thresholds;

    if (!current_valid || !vibration_valid || current_features == NULL || vibration_features == NULL) {
        return MG_FAULT_SENSOR;
    }
    if (!current_features->valid || !vibration_features->valid ||
        !inputs_are_finite(current_features, vibration_features, temperature_c, rpm)) {
        return MG_FAULT_SENSOR;
    }

    if (current_features->rms > thresholds->overload_current_a) {
        return MG_FAULT_OVERLOAD;
    }
    if (temperature_c > thresholds->overtemp_c) {
        return MG_FAULT_OVERTEMP;
    }
    if (rpm > thresholds->overspeed_rpm) {
        return MG_FAULT_OVERSPEED;
    }
    if (rpm < thresholds->stall_rpm && current_features->rms > thresholds->stall_current_a) {
        return MG_FAULT_STALL;
    }

    if (vibration_features->rms > thresholds->vibration_rms_limit_g) {
        if (vibration_features->high_freq_ratio > thresholds->bearing_high_freq_ratio) {
            return MG_FAULT_BEARING;
        }
        if (vibration_features->harmonic_1x_ratio > thresholds->imbalance_1x_ratio) {
            return MG_FAULT_IMBALANCE;
        }
    }

    return MG_FAULT_NONE;
}

mg_fault_code_t mg_fault_detector_update(mg_fault_detector_t *detector,
                                         const mg_signal_features_t *current_features,
                                         const mg_signal_features_t *vibration_features,
                                         float temperature_c,
                                         float rpm,
                                         bool current_valid,
                                         bool vibration_valid)
{
    mg_fault_code_t candidate;
    uint32_t persistence_frames;
    uint32_t clear_frames;

    if (detector == NULL) {
        return MG_FAULT_NONE;
    }

    persistence_frames = (detector->thresholds.persistence_frames == 0u)
                             ? 1u
                             : detector->thresholds.persistence_frames;
    clear_frames = (detector->thresholds.clear_frames == 0u)
                       ? 1u
                       : detector->thresholds.clear_frames;
    candidate = classify(detector,
                         current_features,
                         vibration_features,
                         temperature_c,
                         rpm,
                         current_valid,
                         vibration_valid);

    if (detector->active_fault == MG_FAULT_NONE) {
        if (candidate == MG_FAULT_NONE) {
            detector->candidate_fault = MG_FAULT_NONE;
            detector->candidate_count = 0u;
            detector->clear_count = 0u;
            return MG_FAULT_NONE;
        }

        if (candidate == detector->candidate_fault) {
            ++detector->candidate_count;
        } else {
            detector->candidate_fault = candidate;
            detector->candidate_count = 1u;
        }

        if (detector->candidate_count >= persistence_frames) {
            detector->active_fault = candidate;
            detector->candidate_fault = MG_FAULT_NONE;
            detector->candidate_count = 0u;
            detector->clear_count = 0u;
        }
        return detector->active_fault;
    }

    if (candidate == detector->active_fault) {
        detector->candidate_fault = MG_FAULT_NONE;
        detector->candidate_count = 0u;
        detector->clear_count = 0u;
        return detector->active_fault;
    }

    if (candidate == MG_FAULT_NONE) {
        detector->candidate_fault = MG_FAULT_NONE;
        detector->candidate_count = 0u;
        ++detector->clear_count;
        if (detector->clear_count >= clear_frames) {
            detector->active_fault = MG_FAULT_NONE;
            detector->clear_count = 0u;
        }
        return detector->active_fault;
    }

    detector->clear_count = 0u;
    if (candidate == detector->candidate_fault) {
        ++detector->candidate_count;
    } else {
        detector->candidate_fault = candidate;
        detector->candidate_count = 1u;
    }

    if (detector->candidate_count >= persistence_frames) {
        detector->active_fault = candidate;
        detector->candidate_fault = MG_FAULT_NONE;
        detector->candidate_count = 0u;
        detector->clear_count = 0u;
    }

    return detector->active_fault;
}

const char *mg_fault_code_name(mg_fault_code_t code)
{
    switch (code) {
    case MG_FAULT_NONE:
        return "NONE";
    case MG_FAULT_SENSOR:
        return "SENSOR";
    case MG_FAULT_OVERLOAD:
        return "OVERLOAD";
    case MG_FAULT_OVERTEMP:
        return "OVERTEMP";
    case MG_FAULT_OVERSPEED:
        return "OVERSPEED";
    case MG_FAULT_STALL:
        return "STALL";
    case MG_FAULT_BEARING:
        return "BEARING";
    case MG_FAULT_IMBALANCE:
        return "IMBALANCE";
    default:
        return "UNKNOWN";
    }
}
