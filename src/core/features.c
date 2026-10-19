#include "mg/features.h"
#include <math.h>
#include <stddef.h>

#include "mg/fft.h"
static float safe_divide(float numerator, float denominator)
{
    return (fabsf(denominator) > 1e-12f) ? (numerator / denominator) : 0.0f;
}

static size_t nearest_bin(float frequency_hz, size_t fft_size, float sample_rate_hz)
{
    float bin;
    size_t result;
    size_t max_bin;

    if (fft_size < 4u || frequency_hz <= 0.0f || sample_rate_hz <= 0.0f) {
        return 0u;
    }

    bin = frequency_hz * (float)fft_size / sample_rate_hz;
    result = (size_t)lroundf(bin);
    max_bin = fft_size / 2u - 1u;
    if (result < 1u) {
        result = 1u;
    }
    if (result > max_bin) {
        return 0u;
    }
    return result;
}

void mg_features_init(mg_signal_features_t *features)
{
    if (features == NULL) {
        return;
    }
    *features = (mg_signal_features_t){0};
}

void mg_features_time(const float *samples, size_t count, mg_signal_features_t *out)
{
    size_t i;
    float sum = 0.0f;
    float mean;
    float variance = 0.0f;
    float fourth_moment = 0.0f;
    float peak = 0.0f;
    float rms;

    if (out == NULL) {
        return;
    }

    mg_features_init(out);
    out->sample_count = (uint32_t)count;
    if (samples == NULL || count == 0u) {
        return;
    }

    for (i = 0u; i < count; ++i) {
        sum += samples[i];
    }
    mean = sum / (float)count;

    for (i = 0u; i < count; ++i) {
        const float centered = samples[i] - mean;
        const float abs_value = fabsf(samples[i]);
        variance += centered * centered;
        fourth_moment += centered * centered * centered * centered;
        if (abs_value > peak) {
            peak = abs_value;
        }
    }

    variance /= (float)count;
    fourth_moment /= (float)count;
    rms = sqrtf(variance + mean * mean);

    out->mean = mean;
    out->rms = rms;
    out->peak = peak;
    out->crest_factor = safe_divide(peak, rms);
    out->kurtosis = (variance > 1e-12f) ? (fourth_moment / (variance * variance)) : 0.0f;
    out->valid = true;
}

void mg_features_spectrum(const float *magnitude,
                          size_t fft_size,
                          float sample_rate_hz,
                          float fundamental_hz,
                          float high_freq_start_hz,
                          mg_signal_features_t *out)
{
    size_t i;
    size_t half;
    size_t peak_bin = 0u;
    size_t high_start_bin;
    float peak_magnitude = 0.0f;
    float total_energy = 0.0f;
    float high_energy = 0.0f;
    float bin_hz;

    if (out == NULL) {
        return;
    }
    if (magnitude == NULL || fft_size < 4u || sample_rate_hz <= 0.0f) {
        out->dominant_freq_hz = 0.0f;
        out->harmonic_1x = 0.0f;
        out->harmonic_2x = 0.0f;
        out->harmonic_1x_ratio = 0.0f;
        out->high_freq_ratio = 0.0f;
        out->valid = false;
        return;
    }

    half = fft_size / 2u;
    bin_hz = sample_rate_hz / (float)fft_size;

    for (i = 1u; i < half; ++i) {
        const float magnitude_squared = magnitude[i] * magnitude[i];
        total_energy += magnitude_squared;
        if (magnitude[i] > peak_magnitude) {
            peak_magnitude = magnitude[i];
            peak_bin = i;
        }
    }

    out->dominant_freq_hz = mg_bin_frequency(peak_bin, fft_size, sample_rate_hz);

    if (fundamental_hz > 0.0f) {
        const size_t bin_1x = nearest_bin(fundamental_hz, fft_size, sample_rate_hz);
        const size_t bin_2x = nearest_bin(2.0f * fundamental_hz, fft_size, sample_rate_hz);
        const float scale = 2.0f / (float)fft_size;

        if (bin_1x != 0u) {
            out->harmonic_1x = magnitude[bin_1x] * scale;
        }
        if (bin_2x != 0u) {
            out->harmonic_2x = magnitude[bin_2x] * scale;
        }
        out->harmonic_1x_ratio = safe_divide(out->harmonic_1x, out->peak);
    } else {
        out->harmonic_1x = 0.0f;
        out->harmonic_2x = 0.0f;
        out->harmonic_1x_ratio = 0.0f;
    }

    high_start_bin = 1u;
    if (high_freq_start_hz > 0.0f) {
        high_start_bin = (size_t)ceilf(high_freq_start_hz / bin_hz);
    }
    if (high_start_bin < 1u) {
        high_start_bin = 1u;
    }
    for (i = high_start_bin; i < half; ++i) {
        high_energy += magnitude[i] * magnitude[i];
    }

    out->high_freq_ratio = safe_divide(high_energy, total_energy);
    out->sample_count = (out->sample_count == 0u) ? (uint32_t)fft_size : out->sample_count;
    out->valid = true;
}
