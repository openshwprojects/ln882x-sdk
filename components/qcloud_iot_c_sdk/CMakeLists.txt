file(GLOB_RECURSE   QCLOUD_SRC  ${COMP_QCLOUD_SDK_DIR}  "*.c")

set(COMP_MBEDTLS_SUPPORT  ON  PARENT_SCOPE)

set(qcloud_static_target   "qcloud")

add_library(${qcloud_static_target}    STATIC  ${QCLOUD_SRC})
add_library(components::qcloud  ALIAS   ${qcloud_static_target})

target_include_directories(${qcloud_static_target}
    PUBLIC
    ${CMAKE_SOURCE_DIR}/project/${USER_PROJECT}/bsp
    ${CMAKE_SOURCE_DIR}/project/${USER_PROJECT}/cfg
    ${MCU_LN882X_DIR}/ln88xx
    ${MCU_LN882X_DIR}/CMSIS_5.3.0
    ${COMP_KERNEL_DIR}
    ${COMP_KERNEL_DIR}/FreeRTOS/Source/include
    ${COMP_KERNEL_DIR}/FreeRTOS/Source/portable/GCC/ARM_CM4F
    ${COMP_QCLOUD_SDK_DIR}/include
    ${COMP_QCLOUD_SDK_DIR}/include/exports
    ${COMP_QCLOUD_SDK_DIR}/sdk_src/internal_inc
    ${COMP_LWIP_DIR}/include
    ${COMP_LWIP_DIR}/port
    ${COMP_MBEDTLS_DIR}/include
    ${CMAKE_SOURCE_DIR}/components
    ${COMP_UTILS_DIR}
    ${EXTRA_INCLUDE_PATH}
)
