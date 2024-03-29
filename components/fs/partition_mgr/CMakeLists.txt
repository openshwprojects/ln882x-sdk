file(GLOB_RECURSE  PARTITION_MGR_SRC  ${COMP_PARTMGR_DIR}  "*.c")

set(partition_mgr_static_target "partition_mgr")
add_library(${partition_mgr_static_target} STATIC ${PARTITION_MGR_SRC})
add_library(components::fs::partition_mgr ALIAS ${partition_mgr_static_target})

target_include_directories(${partition_mgr_static_target}
    PUBLIC
    ${CMAKE_SOURCE_DIR}/project/${USER_PROJECT}/bsp
    ${CMAKE_SOURCE_DIR}/project/${USER_PROJECT}/cfg
    ${MCU_LN882X_DIR}/CMSIS_5.3.0
    ${MCU_LN882X_DIR}/ln88xx
    ${MCU_LN882X_DIR}/driver_ln882x/inc
    ${COMP_PARTMGR_DIR}
    ${CMAKE_SOURCE_DIR}/components
)
