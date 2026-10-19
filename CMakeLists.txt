cmake_minimum_required(VERSION 3.20)
project(MotorGuard C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_library(mg_core
    src/core/ring_buffer.c
    src/core/crc16.c
    src/core/fft.c
    src/core/features.c
    src/core/fault_detector.c
    src/core/modbus_rtu.c
)

target_include_directories(mg_core PUBLIC include)
target_compile_options(mg_core PRIVATE -Wall -Wextra -Wpedantic)

if(NOT MSVC)
    target_link_libraries(mg_core PUBLIC m)
endif()

add_library(mg_app
    firmware/app/app_controller.c
)
target_include_directories(mg_app PUBLIC firmware firmware/app)
target_link_libraries(mg_app PUBLIC mg_core)
target_compile_options(mg_app PRIVATE -Wall -Wextra -Wpedantic)

add_executable(mg_tests
    tests/test_main.c
    tests/test_ring_crc.c
    tests/test_fft_features.c
    tests/test_fault_modbus.c
    tests/test_app_controller.c
)
target_link_libraries(mg_tests PRIVATE mg_core mg_app)
target_compile_options(mg_tests PRIVATE -Wall -Wextra -Wpedantic)

add_executable(mg_simulator tools/simulator.c)
target_link_libraries(mg_simulator PRIVATE mg_core)
target_compile_options(mg_simulator PRIVATE -Wall -Wextra -Wpedantic)

enable_testing()
add_test(NAME mg_tests COMMAND mg_tests)
