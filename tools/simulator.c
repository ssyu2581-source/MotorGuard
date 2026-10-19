#include <math.h>
#include <stdio.h>
#include <string.h>

#include "mg/fault_detector.h"
#include "mg/features.h"
#include "mg/fft.h"

#define SIM_FS 20000.0f
#define SIM_N 2048u
#define SIM_F1 996.09375f
#define SIM_HF_START 5000.0f

static void fill_sine(float *samples, float frequency_hz, float amplitude)
{
    size_t i;
    for (i = 0u; i < SIM_N; ++i) {
        samples[i] = amplitude * sinf(2.0f * 3.14159265358979323846f * frequency_hz * (float)i / SIM_FS);
    }
}

static void fill_vibration(float *samples, float one_x_amplitude, float high_freq_amplitude)
{
    size_t i;
    for (i = 0u; i < SIM_N; ++i) {
        const float t = (float)i / SIM_FS;
        samples[i] = one_x_amplitude * sinf(2.0f * 3.14159265358979323846f * SIM_F1 * t) +
                     high_freq_amplitude * sinf(2.0f * 3.14159265358979323846f * 6000.0f * t);
    }
}

static void run_case(const char *name, float current_amplitude, float one_x_amplitude, float high_freq_amplitude)
{
    float current_samples[SIM_N];
    float vibration_samples[SIM_N];
    float real[SIM_N];
    float imag[SIM_N];
    float magnitude[SIM_N];
    mg_signal_features_t current_features;
    mg_signal_features_t vibration_features;
    mg_fault_detector_t detector;
    mg_fault_thresholds_t thresholds;
    mg_fault_code_t fault = MG_FAULT_NONE;
    size_t i;

    fill_sine(current_samples, 50.0f, current_amplitude);
    fill_vibration(vibration_samples, one_x_amplitude, high_freq_amplitude);

    mg_features_time(current_samples, SIM_N, &current_features);
    mg_features_time(vibration_samples, SIM_N, &vibration_features);

    memcpy(real, vibration_samples, sizeof(vibration_samples));
    memset(imag, 0, sizeof(imag));
    (void)mg_fft_radix2(real, imag, SIM_N);
    mg_fft_magnitude(real, imag, SIM_N, magnitude);
    mg_features_spectrum(magnitude, SIM_N, SIM_FS, SIM_F1, SIM_HF_START, &vibration_features);

    mg_fault_thresholds_default(&thresholds);
    thresholds.persistence_frames = 3u;
    thresholds.clear_frames = 3u;
    mg_fault_detector_init(&detector, &thresholds);
    for (i = 0u; i < 3u; ++i) {
        fault = mg_fault_detector_update(&detector,
                                         &current_features,
                                         &vibration_features,
                                         30.0f,
                                         1000.0f,
                                         true,
                                         true);
    }

    printf("%-10s current_rms=%.3f vib_rms=%.3f 1x_ratio=%.3f hf_ratio=%.3f fault=%s\n",
           name,
           current_features.rms,
           vibration_features.rms,
           vibration_features.harmonic_1x_ratio,
           vibration_features.high_freq_ratio,
           mg_fault_code_name(fault));
}

int main(void)
{
    puts("MotorGuard end-to-end simulator");
    run_case("normal", 1.0f, 0.05f, 0.0f);
    run_case("imbalance", 1.0f, 0.25f, 0.0f);
    run_case("bearing", 1.0f, 0.03f, 0.30f);
    run_case("overload", 6.0f, 0.05f, 0.0f);
    return 0;
}