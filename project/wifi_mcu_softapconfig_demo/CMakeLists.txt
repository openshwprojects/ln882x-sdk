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
set(COMP_SOFTAPCFG_SUPPORT  ON  PARENT_SCOPE)

target_link_libraries(${pro_executable_target}
    PUBLIC
    components::smartlink::softapcfg
    wifi
    components::kernel
    components::fs::kv
    components::atcmd
    components::utils
    components::serial
    components::phony
    driver_ln882x
    -lc -lm -lnosys

    PRIVATE
    -T${LINKER_SCRIPT}
    ${EXTRA_LINK_FLAGS}
)

target_include_directories(${pro_executable_target}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/app
    ${CMAKE_CURRENT_SOURCE_DIR}/bsp
    ${CMAKE_CURRENT_SOURCE_DIR}/cfg
    ${COMP_SOFTAPCFG_DIR}
    ${MCU_LN882X_DIR}/driver_ln882x/inc/reg
)

include(${CMAKE_SOURCE_DIR}/project/${USER_PROJECT}/gcc/gcc-custom-build-stage.cmake)
