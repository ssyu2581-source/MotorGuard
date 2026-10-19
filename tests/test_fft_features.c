#include "test_util.h"

#include <string.h>

#include "mg/features.h"
#include "mg/fft.h"

void test_fft(void)
{
    enum { N = 1024 };
    float real[N];
    float imag[N];
    float magnitude[N];
    size_t i;
    size_t peak_bin = 0u;
    float peak = 0.0f;

    puts("test_fft");
    fill_sine(real, N, 1000.0f, 20000.0f);
    memset(imag, 0, sizeof(imag));

    MG_CHECK(mg_fft_radix2(real, imag, N) == MG_FFT_OK, "FFT runs");
    mg_fft_magnitude(real, imag, N, magnitude);

    for (i = 1u; i < N / 2u; ++i) {
        if (magnitude[i] > peak) {
            peak = magnitude[i];
            peak_bin = i;
        }
    }
    MG_CHECK(peak_bin >= 50u && peak_bin <= 52u, "1 kHz peak near bin 51");
    MG_CHECK(peak > 400.0f, "sine FFT magnitude");
    MG_CHECK_NEAR(mg_bin_frequency(peak_bin, N, 20000.0f), 1000.0f, 20.0f, "peak frequency");

    for (i = 0u; i < N; ++i) {
        real[i] = 1.0f;
        imag[i] = 0.0f;
    }
    MG_CHECK(mg_fft_radix2(real, imag, N) == MG_FFT_OK, "DC FFT runs");
    MG_CHECK_NEAR(real[0], (float)N, 0.5f, "DC bin");
    MG_CHECK(fabsf(real[1]) < 0.01f, "DC has no leakage into bin 1");
}

void test_features(void)
{
    enum { N = 2048 };
    float samples[N];
    float real[N];
    float imag[N];
    float magnitude[N];
    mg_signal_features_t features;
    const float fundamental = 996.09375f;

    puts("test_features");
    fill_sine(samples, N, fundamental, 20000.0f);
    mg_features_time(samples, N, &features);
    MG_CHECK(features.valid, "time features valid");
    MG_CHECK_NEAR(features.rms, 0.70710678f, 0.01f, "sine RMS");
    MG_CHECK_NEAR(features.peak, 1.0f, 0.01f, "sine peak");
    MG_CHECK_NEAR(features.crest_factor, 1.41421356f, 0.02f, "sine crest factor");
    MG_CHECK_NEAR(features.kurtosis, 1.5f, 0.05f, "sine kurtosis");

    memcpy(real, samples, sizeof(samples));
    memset(imag, 0, sizeof(imag));
    MG_CHECK(mg_fft_radix2(real, imag, N) == MG_FFT_OK, "feature FFT runs");
    mg_fft_magnitude(real, imag, N, magnitude);
    mg_features_spectrum(magnitude, N, 20000.0f, fundamental, 5000.0f, &features);
    MG_CHECK_NEAR(features.dominant_freq_hz, fundamental, 1.0f, "dominant frequency");
    MG_CHECK_NEAR(features.harmonic_1x, 1.0f, 0.02f, "1x amplitude");
    MG_CHECK_NEAR(features.harmonic_1x_ratio, 1.0f, 0.03f, "1x ratio");
    MG_CHECK(features.high_freq_ratio < 0.01f, "high frequency ratio for pure sine");
}