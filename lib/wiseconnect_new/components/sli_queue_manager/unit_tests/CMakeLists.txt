project(sli_queue_manager)

include_directories(../../../tests/unit_tests/inc
                    ../inc
                    inc
                    ../../gsdk/common/inc
                    ../../gsdk/cmsis/RTOS2/Include
                    ../../gsdk/service/mem_pool/inc
                    ../../common/inc
                    ../../protocol/wifi/inc
                    ../../sli_buffer_manager/inc
)

# Add unit test cpp here
add_executable(${PROJECT_NAME}
               src/sli_queue_manager_fake_function.c
               ../src/sli_queue_manager.c
               ../unit_tests/src/sli_queue_manager_unit_tests.cpp
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