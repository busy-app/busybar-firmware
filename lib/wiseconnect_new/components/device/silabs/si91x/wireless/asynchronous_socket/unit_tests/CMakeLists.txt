# Project name
project(sl_async_socket)

# Include directories
include_directories(
    ../../../../../../device/silabs/si91x/wireless/firmware_upgrade
    ../../../../../../freertos/cmsis/Include
    ../../../../../../common/inc
    ../../../../../../gsdk/common/inc
    ../../../../../../gsdk/cmsis/RTOS2/Include
    ../../../../third_party/fff
    ./inc
    ../../socket/inc
    ../../sl_net/inc
    ../../../../../../service/bsd_socket/inc
    ../../../../../../tests/unit_tests/inc
    ../inc
    ../../inc
    ../../../../../../protocol/wifi/inc
    ../../../../../service/bsd_socket/inc
    ../../../../../../sli_wifi/inc
    ../../../../../service/mdns/inc/
)
# Add source files for the test executable
add_executable(${PROJECT_NAME}
    src/sli_async_socket_unit_tests.cpp
    src/sli_async_socket_fake_functions.c
    ../src/sl_si91x_socket.c
    ../../../../../../../tests/unit_tests/src/stubs.c
)

# Add unit being tested here\
target_link_libraries(${PROJECT_NAME} PUBLIC 
                      gtest
                      gtest_main
                      fff
)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
target_link_libraries(${PROJECT_NAME} PUBLIC 
                      gcov
)

endif()