file(GLOB_RECURSE  ATCMD_SRC  ${COMP_ATCMD_DIR}  "*.c")

set(atcmd_static_target "atcmd")

add_library(${atcmd_static_target} STATIC ${ATCMD_SRC})
add_library(components::atcmd ALIAS ${atcmd_static_target})

set(COMP_KV_SUPPORT     ON  PARENT_SCOPE)
set(COMP_NVDS_SUPPORT   ON  PARENT_SCOPE)
set(COMP_PING_SUPPORT   ON  PARENT_SCOPE)
set(COMP_IPERF_SUPPORT  ON  PARENT_SCOPE)
set(COMP_DHCPD_SUPPORT  ON  PARENT_SCOPE)

target_link_libraries(${atcmd_static_target}
    components::fs::kv
    components::fs::nvds
    components::net::ping
    components::net::iperf
    components::net::dhcpd
    components::phony
)

target_include_directories(${atcmd_static_target}
    PUBLIC
    ${COMP_ATCMD_DIR}
    ${CMAKE_SOURCE_DIR}/project/${USER_PROJECT}/bsp
    ${CMAKE_SOURCE_DIR}/project/${USER_PROJECT}/cfg
    ${MCU_LN882X_DIR}/driver_ln882x/inc
    ${MCU_LN882X_DIR}/ln88xx
    ${MCU_LN882X_DIR}/CMSIS_5.3.0
    ${CMAKE_SOURCE_DIR}/components
    ${COMP_KERNEL_DIR}
    ${COMP_KERNEL_DIR}/FreeRTOS/Source/include
    ${COMP_KERNEL_DIR}/FreeRTOS/Source/portable/GCC/ARM_CM4F
    ${COMP_WIFI_DIR}
    ${COMP_WIFI_DIR}/wifi_manager
    ${COMP_LWIP_DIR}/include
    ${COMP_LWIP_DIR}/port
    ${COMP_PING_DIR}
    ${COMP_IPERF_DIR}
    ${COMP_DHCPD_DIR}
    ${COMP_NVDS_DIR}
    ${COMP_KV_DIR}/kv
    ${COMP_KV_DIR}/kv_port
    ${COMP_SERIAL_DIR}
)
