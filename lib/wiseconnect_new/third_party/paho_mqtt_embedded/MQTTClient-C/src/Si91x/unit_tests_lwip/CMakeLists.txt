project(MQTTSi91x_lwip)


# Disable TLS for unit tests (GSDK mbedtls is used in actual builds)
add_definitions(-DMQTT_TLS_ENABLE=0)

# Define necessary macros for LwIP
add_definitions(-DLWIP_SOCKET=1)
add_definitions(-DLWIP_NETCONN=1)

# Handle errno TLS issues on Linux - prevent LwIP errno redefinition
if(UNIX)
    add_definitions(-D_GNU_SOURCE)
    # Critical fixes based on lwip/sockets.h -> lwip/errno.h inclusion
    add_definitions(-DLWIP_PROVIDE_ERRNO=0)     # Tell LwIP not to provide errno
    add_definitions(-D__LWIP_ERRNO_H__)         # Prevent lwip/errno.h inclusion
    add_definitions(-DERRNO_H_)                 # Mark errno.h as already included
    # Force system errno.h to be included first
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -include errno.h")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -include errno.h")
endif()



include_directories(
	./inc
	../../../../../fff
    ../../../../../../tests/unit_tests_lwip/inc
    ../../../../../../components/common/inc
	../../../../../googletest/googletest/include
	../../../../../googletest/googlemock/include
    ../../../../../../components/lwip/src/include
    ../../../../../../components/lwip
    ../../../../../../components/lwip/hosts/freertos/include
    ../../../../../../components/lwip/src/include/compat/posix
	../../../../../../components/sli_wifi/inc
	../../../../../../components/gsdk/common/inc
	../../../../../../components/protocol/wifi/inc
	../../../../../../components/service/bsd_socket/inc
	../../../../../../components/gsdk/cmsis/RTOS2/Include
	../../../../../../components/service/network_manager/inc
	../../../../../../components/service/bsd_socket/si91x_socket
	../../../../../../components/device/silabs/si91x/wireless/inc
	../../../../../../components/device/silabs/si91x/wireless/socket/inc
	../../../../../../components/device/silabs/si91x/wireless/sl_net/inc
    ../../../../../../components/sli_wifi/inc
	../../../../../../components/service/mdns/inc/

)

add_executable(${PROJECT_NAME}
	../MQTTSi91x_lwip.c
	src/MQTTSi91x_lwip_unit_tests.cpp
	src/MQTTSi91x_lwip_fake_functions.c
    ../../../../../../tests/unit_tests/src/stubs.c
)

# Add unit being tested here
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