file(GLOB_RECURSE  NVDS_SRC  ${COMP_NVDS_DIR}  "*.c")

set(nvds_static_target "nvds")
add_library(${nvds_static_target} STATIC ${NVDS_SRC})
add_library(components::fs::nvds ALIAS ${nvds_static_target})

target_link_libraries(${nvds_static_target}
    PUBLIC
    components::phony
)

target_include_directories(${nvds_static_target}
    PUBLIC
    ${COMP_NVDS_DIR}
    ${CMAKE_SOURCE_DIR}/project/${USER_PROJECT}/cfg
    ${MCU_LN882X_DIR}/driver_ln882x/inc
    ${MCU_LN882X_DIR}/ln88xx
    ${MCU_LN882X_DIR}/CMSIS_5.3.0
)
