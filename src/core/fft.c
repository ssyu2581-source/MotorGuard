#include "mg/fft.h"
#include <math.h>
#include <stdbool.h>
#include <stddef.h>

#define MG_PI 3.14159265358979323846f

static bool is_power_of_two(size_t n)
{
    return n != 0u && (n & (n - 1u)) == 0u;
}

mg_fft_status_t mg_fft_radix2(float *real, float *imag, size_t n)
{
    size_t i;
    size_t j;
    size_t len;

    if (real == NULL || imag == NULL || n < 2u) {
        return MG_FFT_ERR_SIZE;
    }
    if (!is_power_of_two(n)) {
        return MG_FFT_ERR_NOT_POWER_OF_TWO;
    }

    j = 0u;
    for (i = 1u; i < n; ++i) {
        size_t bit = n >> 1u;
        while ((j & bit) != 0u) {
            j ^= bit;
            bit >>= 1u;
        }
        j ^= bit;
        if (i < j) {
            float temp_real = real[i];
            float temp_imag = imag[i];
            real[i] = real[j];
            imag[i] = imag[j];
            real[j] = temp_real;
            imag[j] = temp_imag;
        }
    }

    for (len = 2u; len <= n; len <<= 1u) {
        const float angle = -2.0f * MG_PI / (float)len;
        const float wlen_real = cosf(angle);
        const float wlen_imag = sinf(angle);
        size_t start;

        for (start = 0u; start < n; start += len) {
            float w_real = 1.0f;
            float w_imag = 0.0f;
            size_t k;

            for (k = 0u; k < len / 2u; ++k) {
                const size_t even = start + k;
                const size_t odd = even + len / 2u;
                const float odd_real = real[odd] * w_real - imag[odd] * w_imag;
                const float odd_imag = real[odd] * w_imag + imag[odd] * w_real;
                const float even_real = real[even];
                const float even_imag = imag[even];
                const float next_w_real = w_real * wlen_real - w_imag * wlen_imag;
                const float next_w_imag = w_real * wlen_imag + w_imag * wlen_real;

                real[even] = even_real + odd_real;
                imag[even] = even_imag + odd_imag;
                real[odd] = even_real - odd_real;
                imag[odd] = even_imag - odd_imag;
                w_real = next_w_real;
                w_imag = next_w_imag;
            }
        }
    }

    return MG_FFT_OK;
}

void mg_fft_magnitude(const float *real, const float *imag, size_t n, float *magnitude)
{
    size_t i;

    if (real == NULL || imag == NULL || magnitude == NULL) {
        return;
    }

    for (i = 0u; i < n; ++i) {
        magnitude[i] = sqrtf(real[i] * real[i] + imag[i] * imag[i]);
    }
}

void mg_hann_window(float *data, size_t n)
{
    size_t i;

    if (data == NULL || n == 0u) {
        return;
    }
    if (n == 1u) {
        data[0] = 1.0f;
        return;
    }

    for (i = 0u; i < n; ++i) {
        const float phase = 2.0f * MG_PI * (float)i / (float)(n - 1u);
        data[i] *= 0.5f * (1.0f - cosf(phase));
    }
}

void mg_apply_window(float *data, const float *window, size_t n)
{
    size_t i;

    if (data == NULL || window == NULL) {
        return;
    }

    for (i = 0u; i < n; ++i) {
        data[i] *= window[i];
    }
}

float mg_bin_frequency(size_t bin, size_t fft_size, float sample_rate_hz)
{
    if (fft_size == 0u) {
        return 0.0f;
    }
    return (float)bin * sample_rate_hz / (float)fft_size;
}
