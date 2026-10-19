#include "test_util.h"

#include "mg/crc16.h"
#include "mg/fault_detector.h"
#include "mg/features.h"
#include "mg/modbus_rtu.h"

static mg_signal_features_t normal_current(void)
{
    mg_signal_features_t features;
    mg_features_init(&features);
    features.valid = true;
    features.rms = 1.0f;
    features.peak = 1.5f;
    return features;
}

static mg_signal_features_t normal_vibration(void)
{
    mg_signal_features_t features;
    mg_features_init(&features);
    features.valid = true;
    features.rms = 0.05f;
    features.peak = 0.08f;
    features.harmonic_1x = 0.05f;
    features.harmonic_1x_ratio = 0.6f;
    features.high_freq_ratio = 0.02f;
    return features;
}

void test_fault_detector(void)
{
    mg_fault_detector_t detector;
    mg_fault_thresholds_t thresholds;
    mg_signal_features_t current = normal_current();
    mg_signal_features_t vibration = normal_vibration();
    mg_fault_code_t fault = MG_FAULT_NONE;
    int i;

    puts("test_fault_detector");
    mg_fault_thresholds_default(&thresholds);
    thresholds.persistence_frames = 3u;
    thresholds.clear_frames = 3u;
    mg_fault_detector_init(&detector, &thresholds);

    for (i = 0; i < 5; ++i) {
        fault = mg_fault_detector_update(&detector, &current, &vibration, 30.0f, 1000.0f, true, true);
    }
    MG_CHECK(fault == MG_FAULT_NONE, "normal state");

    current.rms = 5.0f;
    fault = mg_fault_detector_update(&detector, &current, &vibration, 30.0f, 1000.0f, true, true);
    MG_CHECK(fault == MG_FAULT_NONE, "overload needs persistence");
    fault = mg_fault_detector_update(&detector, &current, &vibration, 30.0f, 1000.0f, true, true);
    fault = mg_fault_detector_update(&detector, &current, &vibration, 30.0f, 1000.0f, true, true);
    MG_CHECK(fault == MG_FAULT_OVERLOAD, "overload fault");

    current = normal_current();
    for (i = 0; i < 3; ++i) {
        fault = mg_fault_detector_update(&detector, &current, &vibration, 30.0f, 1000.0f, true, true);
    }
    MG_CHECK(fault == MG_FAULT_NONE, "overload clears");

    vibration.rms = 0.2f;
    vibration.high_freq_ratio = 0.02f;
    vibration.harmonic_1x_ratio = 0.8f;
    for (i = 0; i < 3; ++i) {
        fault = mg_fault_detector_update(&detector, &current, &vibration, 30.0f, 1000.0f, true, true);
    }
    MG_CHECK(fault == MG_FAULT_IMBALANCE, "imbalance fault");

    vibration = normal_vibration();
    for (i = 0; i < 3; ++i) {
        fault = mg_fault_detector_update(&detector, &current, &vibration, 30.0f, 1000.0f, true, true);
    }
    vibration.rms = 0.2f;
    vibration.high_freq_ratio = 0.5f;
    for (i = 0; i < 3; ++i) {
        fault = mg_fault_detector_update(&detector, &current, &vibration, 30.0f, 1000.0f, true, true);
    }
    MG_CHECK(fault == MG_FAULT_BEARING, "bearing fault");

    vibration = normal_vibration();
    for (i = 0; i < 3; ++i) {
        fault = mg_fault_detector_update(&detector, &current, &vibration, 30.0f, 1000.0f, false, true);
    }
    MG_CHECK(fault == MG_FAULT_SENSOR, "sensor fault");
}

void test_modbus(void)
{
    uint8_t request[8] = {0x01u, 0x03u, 0x00u, 0x00u, 0x00u, 0x02u, 0x00u, 0x00u};
    mg_modbus_request_t parsed;
    mg_modbus_status_t status;
    uint16_t crc;
    uint16_t registers[2] = {0x1234u, 0x5678u};
    uint8_t response[9];
    size_t response_size;
    uint8_t exception[5];
    size_t exception_size;

    puts("test_modbus");
    crc = mg_crc16_modbus(request, 6u);
    request[6] = (uint8_t)(crc & 0xFFu);
    request[7] = (uint8_t)(crc >> 8u);

    status = mg_modbus_parse_read_request(request, sizeof(request), &parsed);
    MG_CHECK(status == MG_MODBUS_OK, "parse valid request");
    MG_CHECK(parsed.slave == 1u && parsed.function == 0x03u, "request slave/function");
    MG_CHECK(parsed.start == 0u && parsed.count == 2u, "request address/count");

    request[7] ^= 0xFFu;
    MG_CHECK(mg_modbus_parse_read_request(request, sizeof(request), &parsed) == MG_MODBUS_ERR_CRC,
             "reject bad CRC");

    response_size = mg_modbus_build_read_response(1u, 0x03u, registers, 2u, response, sizeof(response));
    MG_CHECK(response_size == 9u, "response size");
    MG_CHECK(response[0] == 1u && response[1] == 0x03u && response[2] == 4u, "response header");
    MG_CHECK(response[3] == 0x12u && response[4] == 0x34u, "register 0");
    MG_CHECK(response[5] == 0x56u && response[6] == 0x78u, "register 1");
    crc = mg_crc16_modbus(response, 7u);
    MG_CHECK(response[7] == (uint8_t)(crc & 0xFFu) && response[8] == (uint8_t)(crc >> 8u),
             "response CRC");

    exception_size = mg_modbus_build_exception(1u, 0x03u, 0x02u, exception, sizeof(exception));
    MG_CHECK(exception_size == 5u, "exception size");
    MG_CHECK(exception[1] == 0x83u && exception[2] == 0x02u, "exception payload");
    crc = mg_crc16_modbus(exception, 3u);
    MG_CHECK(exception[3] == (uint8_t)(crc & 0xFFu) && exception[4] == (uint8_t)(crc >> 8u),
             "exception CRC");
}