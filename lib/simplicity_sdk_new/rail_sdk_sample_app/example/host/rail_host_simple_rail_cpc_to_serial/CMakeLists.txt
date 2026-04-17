cmake_minimum_required(VERSION 3.13.0)

project(${PROJ_NAME} LANGUAGES C)

set(PROJ_NAME rail_host_cpc_to_serial)
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

set(GSDK_PATH ../../../../..)
set(CPC_DIR ${GSDK_PATH}/platform/service/cpc)

add_executable(
    ${PROJ_NAME}
    cpc_to_serial.c
)
include_directories(
    ${CPC_DIR}/daemon/lib
)

target_link_libraries(
    ${PROJ_NAME}
    PUBLIC
    cpc
    Threads::Threads
    util
)
