#include "test_util.h"

#include "app_controller.h"
#include "mg/features.h"

static bool relay_closed = true;
static bool motor_cut_off = false;

static void fake_set_relay(bool closed)
{
    relay_closed = closed;
}

static void fake_set_motor_duty(float duty)
{
    if (duty == 0.0f) {
        motor_cut_off = true;
    }
}

void test_app_controller(void)
{
    mg_bsp_t bsp = {0};
    mg_app_controller_t controller;
    mg_fault_thresholds_t thresholds;
    mg_signal_features_t current;
    mg_signal_features_t vibration;
    mg_fault_code_t fault;

    puts("test_app_controller");
    bsp.set_relay = fake_set_relay;
    bsp.set_motor_duty = fake_set_motor_duty;

    mg_fault_thresholds_default(&thresholds);
    thresholds.persistence_frames = 1u;
    thresholds.clear_frames = 1u;

    mg_features_init(&current);
    current.valid = true;
    current.rms = 1.0f;
    current.peak = 1.5f;

    mg_features_init(&vibration);
    vibration.valid = true;
    vibration.rms = 0.05f;
    vibration.peak = 0.08f;
    vibration.harmonic_1x_ratio = 0.5f;
    vibration.high_freq_ratio = 0.02f;

    relay_closed = true;
    motor_cut_off = false;
    mg_app_controller_init(&controller, &bsp, &thresholds);
    MG_CHECK(!relay_closed, "relay opens on init");

    fault = mg_app_controller_update(&controller, &current, &vibration, 30.0f, 1000.0f, true, true);
    MG_CHECK(fault == MG_FAULT_NONE, "normal state keeps output enabled");
    MG_CHECK(relay_closed, "relay closes in normal state");

    current.rms = 5.0f;
    fault = mg_app_controller_update(&controller, &current, &vibration, 30.0f, 1000.0f, true, true);
    MG_CHECK(fault == MG_FAULT_OVERLOAD, "overload detected");
    MG_CHECK(!relay_closed, "relay opens on fault");
    MG_CHECK(motor_cut_off, "motor duty forced to zero on fault");
}