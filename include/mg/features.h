#ifndef MG_FEATURES_H
#define MG_FEATURES_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float mean;
    float rms;
    float peak;
    float crest_factor;
    float kurtosis;
    float dominant_freq_hz;
    float harmonic_1x;
    float harmonic_2x;
    float harmonic_1x_ratio;
    float high_freq_ratio;
    uint32_t sample_count;
    bool valid;
} mg_signal_features_t;

void mg_features_init(mg_signal_features_t *features);
void mg_features_time(const float *samples, size_t count, mg_signal_features_t *out);
void mg_features_spectrum(const float *magnitude,
                          size_t fft_size,
                          float sample_rate_hz,
                          float fundamental_hz,
                          float high_freq_start_hz,
                          mg_signal_features_t *out);

#ifdef __cplusplus
}
#endif

#endif
