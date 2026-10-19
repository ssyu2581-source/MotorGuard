#ifndef MG_FFT_H
#define MG_FFT_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    MG_FFT_OK = 0,
    MG_FFT_ERR_SIZE,
    MG_FFT_ERR_NOT_POWER_OF_TWO
} mg_fft_status_t;

mg_fft_status_t mg_fft_radix2(float *real, float *imag, size_t n);
void mg_fft_magnitude(const float *real, const float *imag, size_t n, float *magnitude);
void mg_hann_window(float *data, size_t n);
void mg_apply_window(float *data, const float *window, size_t n);
float mg_bin_frequency(size_t bin, size_t fft_size, float sample_rate_hz);

#ifdef __cplusplus
}
#endif

#endif
