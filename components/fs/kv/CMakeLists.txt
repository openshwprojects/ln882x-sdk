file(GLOB_RECURSE  KV_SRC  ${COMP_KV_DIR}  "*.c")

set(kv_static_target "kv")
add_library(${kv_static_target} STATIC ${KV_SRC})
add_library(components::fs::kv ALIAS ${kv_static_target})

target_link_libraries(${kv_static_target}
    PUBLIC
    components::phony
)

target_include_directories(${kv_static_target}
    PUBLIC
    ${COMP_KV_DIR}/kv
    ${COMP_KV_DIR}/kv_port
    ${CMAKE_SOURCE_DIR}/project/${USER_PROJECT}/bsp
    ${CMAKE_SOURCE_DIR}/project/${USER_PROJECT}/cfg
    ${MCU_LN882X_DIR}/CMSIS_5.3.0
    ${MCU_LN882X_DIR}/driver_ln882x/inc
    ${MCU_LN882X_DIR}/ln88xx
    ${CMAKE_SOURCE_DIR}/components/
    ${COMP_KERNEL_DIR}
    ${COMP_KERNEL_DIR}/FreeRTOS/Source/include
    ${COMP_KERNEL_DIR}/FreeRTOS/Source/portable/GCC/ARM_CM4F
)
