project(MQTTSi91x)

include_directories(./inc
                    ../../../../../fff
                    ../../../../../../tests/unit_tests/inc
                    ../../../../../../components/common/inc
                    ../../../../../googletest/googletest/include
                    ../../../../../googletest/googlemock/include
                    ../../../../../../components/sli_wifi/inc
                    ../../../../../../components/gsdk/common/inc
                    ../../../../../../components/protocol/wifi/inc
                    ../../../../../../components/service/bsd_socket/inc
                    ../../../../../../components/gsdk/cmsis/RTOS2/Include
                    ../../../../../../components/service/network_manager/inc
                    ../../../../../../components/service/bsd_socket/si91x_socket
                    ../../../../../../components/service/sl_websocket_client/inc
                    ../../../../../../components/device/silabs/si91x/wireless/inc
                    ../../../../../../components/device/silabs/si91x/wireless/socket/inc
                    ../../../../../../components/device/silabs/si91x/wireless/sl_net/inc
                    ../../../../../../components/sli_wifi/inc
                    ../../../../../../components/service/mdns/inc
)

# Add unit test cpp here
add_executable(${PROJECT_NAME}
                    ../MQTTSi91x.c
                    src/MQTTSi91x_unit_tests.cpp
                    src/MQTTSi91x_fake_functions.c
                    ../../../../../../tests/unit_tests/src/stubs.c
)

# Add unit being tested here\
target_link_libraries(${PROJECT_NAME} PUBLIC 
                    fff
                    gtest
                    gtest_main
)
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
target_link_libraries(${PROJECT_NAME} PUBLIC 
                    gcov
)
endif()