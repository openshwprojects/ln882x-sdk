set(PRO_DIR  ${CMAKE_CURRENT_SOURCE_DIR})

set(PRO_SRC
    ${PRO_DIR}/app/main.c
    ${PRO_DIR}/app/usr_app.c
    ${PRO_DIR}/bsp/serial_hw.c
    ${PRO_DIR}/bsp/drv_adc_measure.c
    ${PRO_DIR}/startup/startup_ln882x_gcc.c
    ${CMAKE_SOURCE_DIR}/components/libc/newlib_stub.c
)

link_directories(${CMAKE_SOURCE_DIR}/lib/gcclib)

set(pro_executable_target  ${TARGET_ELF_NAME})
add_executable(${pro_executable_target}  ${PRO_SRC})

set(COMP_ATCMD_SUPPORT      ON  PARENT_SCOPE)
set(COMP_SMARTCFG_SUPPORT   ON  PARENT_SCOPE)

#--------------------------  systemview  settings  -----------------------------
set(USER_SYSTEMVIEW_SUPPORT         OFF)
if(USER_SYSTEMVIEW_SUPPORT)
    set(COMP_SYSTEMVIEW_SUPPORT     ON  PARENT_SCOPE) # global variable -- ON
    add_definitions(-DSYSTEMVIEW_ENABLED)
    set(EXTRA_LIB   ${EXTRA_LIB}  components::systemview)
endif(USER_SYSTEMVIEW_SUPPORT)


target_link_libraries(${pro_executable_target}
    PUBLIC
    components::smartlink::smartcfg
    wifi
    components::kernel
    components::fs::kv
    components::utils
    components::atcmd
    components::serial
    components::phony
    ${EXTRA_LIB}
    driver_ln882x
    -lc -lm -lnosys

    PRIVATE
    -T${LINKER_SCRIPT}
    ${EXTRA_LINK_FLAGS}
)

target_include_directories(${pro_executable_target}
    PRIVATE
    ${CMAKE_SOURCE_DIR}/project/${USER_PROJECT}/bsp
    ${CMAKE_SOURCE_DIR}/project/${USER_PROJECT}/app
    ${CMAKE_SOURCE_DIR}/project/${USER_PROJECT}/cfg
    ${COMP_SMARTCFG_DIR}
    ${MCU_LN882X_DIR}/driver_ln882x/inc/reg
)

include(${CMAKE_SOURCE_DIR}/project/${USER_PROJECT}/gcc/gcc-custom-build-stage.cmake)