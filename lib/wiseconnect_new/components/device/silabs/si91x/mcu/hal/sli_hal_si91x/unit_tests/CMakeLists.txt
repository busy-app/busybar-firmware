project(sli_hal_si91x)

include_directories(../../../../../../../../tests/unit_tests/inc
                    ../inc
                    inc
                    ../../../../../../../sli_routing_utility/inc
                    ../../../../../../../sli_queue_manager/inc
                    ../../../../../../../gsdk/common/inc
                    ../../../../../../../gsdk/cmsis/RTOS2/Include
                    ../../../../../../../gsdk/service/mem_pool/inc
                    ../../../../../../../common/inc
                    ../../../../../../../sli_wifi/inc
                    ../../../../../../../protocol/wifi/inc
                    ../../../../../../../sli_buffer_manager/inc
                    ../../../../../../../device/silabs/si91x/wireless/inc
                    ../../../../../../../device/silabs/si91x/wireless/firmware_upgrade                    
)

# Add unit test cpp here
add_executable(${PROJECT_NAME}
               src/sli_hal_si91x_fake_functions.c
               ../src/sli_hal_si91x.c
               ../unit_tests/src/sli_hal_si91x_unit_tests.cpp
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