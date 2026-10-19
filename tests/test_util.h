#ifndef MG_TEST_UTIL_H
#define MG_TEST_UTIL_H

#include <math.h>
#include <stddef.h>
#include <stdio.h>

#define TEST_PI 3.14159265358979323846f

extern int mg_test_failures;

static inline void mg_test_check(int condition, const char *message)
{
    if (!condition) {
        puts("  FAIL:");
        puts(message);
        ++mg_test_failures;
    }
}

static inline void mg_test_check_near(float actual, float expected, float tolerance, const char *message)
{
    if (fabsf(actual - expected) > tolerance) {
        puts("  FAIL (near):");
        puts(message);
        ++mg_test_failures;
    }
}

#define MG_CHECK(condition, message) mg_test_check((condition), (message))
#define MG_CHECK_NEAR(actual, expected, tolerance, message) mg_test_check_near((float)(actual), (float)(expected), (float)(tolerance), (message))

static inline void fill_sine(float *samples, size_t count, float frequency_hz, float sample_rate_hz)
{
    size_t i;
    for (i = 0u; i < count; ++i) {
        samples[i] = sinf(2.0f * TEST_PI * frequency_hz * (float)i / sample_rate_hz);
    }
}

void test_ring_buffer(void);
void test_crc16(void);
void test_fft(void);
void test_features(void);
void test_fault_detector(void);
void test_modbus(void);
void test_app_controller(void);

#endif