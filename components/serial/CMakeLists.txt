file(GLOB_RECURSE  SERIAL_SRC  ${COMP_SERIAL_DIR}  "*.c")

set(serial_static_target "serial")
add_library(${serial_static_target} STATIC ${SERIAL_SRC})
add_library(components::serial ALIAS ${serial_static_target})

target_include_directories(${serial_static_target}
    PUBLIC
    ${CMAKE_SOURCE_DIR}/project/${USER_PROJECT}/bsp
    ${CMAKE_SOURCE_DIR}/project/${USER_PROJECT}/cfg
    ${MCU_LN882X_DIR}/ln88xx
    ${MCU_LN882X_DIR}/CMSIS_5.3.0
    ${MCU_LN882X_DIR}/driver_ln882x/inc
    ${COMP_SERIAL_DIR}
    ${CMAKE_SOURCE_DIR}/components
)
