#include "test_util.h"

int mg_test_failures = 0;

int main(void)
{
    test_ring_buffer();
    test_crc16();
    test_fft();
    test_features();
    test_fault_detector();
    test_modbus();
    test_app_controller();

    if (mg_test_failures != 0) {
        puts("tests failed");
        return 1;
    }

    puts("all tests passed");
    return 0;
}