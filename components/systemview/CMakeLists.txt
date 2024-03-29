file(GLOB_RECURSE  SYSTEMVIEW_SRCS  ${COMP_SYSTEMVIEW_DIR}  "*.c")

set(systemview_static_target   "systemview")
add_library(${systemview_static_target}  STATIC  ${SYSTEMVIEW_SRCS})
add_library(components::systemview  ALIAS  ${systemview_static_target})

target_include_directories(${systemview_static_target}
    PUBLIC
    ${CMAKE_SOURCE_DIR}/project/${USER_PROJECT}/cfg
    ${COMP_SYSTEMVIEW_DIR}/Config
    ${COMP_SYSTEMVIEW_DIR}/Sample/FreeRTOSV9
    ${COMP_SYSTEMVIEW_DIR}/SEGGER
    ${MCU_LN882X_DIR}/ln88xx
    ${MCU_LN882X_DIR}/CMSIS_5.3.0
    ${COMP_KERNEL_DIR}
    ${COMP_KERNEL_DIR}/FreeRTOS/Source/include
    ${COMP_KERNEL_DIR}/FreeRTOS/Source/portable/GCC/ARM_CM4F
)
