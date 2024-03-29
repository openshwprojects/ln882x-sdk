set(PHONY_SRC
    ${MCU_LN882X_DIR}/driver_ln882x/src/hal/cache.c
    ${MCU_LN882X_DIR}/driver_ln882x/src/hal/exflash.c
    ${MCU_LN882X_DIR}/driver_ln882x/src/hal/flash.c
    ${MCU_LN882X_DIR}/driver_ln882x/src/hal/hal_adc.c
    ${MCU_LN882X_DIR}/driver_ln882x/src/hal/hal_aes.c
    ${MCU_LN882X_DIR}/driver_ln882x/src/hal/hal_common.c
    ${MCU_LN882X_DIR}/driver_ln882x/src/hal/hal_dma.c
    ${MCU_LN882X_DIR}/driver_ln882x/src/hal/hal_efuse.c
    ${MCU_LN882X_DIR}/driver_ln882x/src/hal/hal_gpio.c
    ${MCU_LN882X_DIR}/driver_ln882x/src/hal/hal_i2c.c
    ${MCU_LN882X_DIR}/driver_ln882x/src/hal/hal_i2s.c
    ${MCU_LN882X_DIR}/driver_ln882x/src/hal/hal_interrupt.c
    ${MCU_LN882X_DIR}/driver_ln882x/src/hal/hal_pwm.c
    ${MCU_LN882X_DIR}/driver_ln882x/src/hal/hal_rtc.c
    ${MCU_LN882X_DIR}/driver_ln882x/src/hal/hal_sdio.c
    ${MCU_LN882X_DIR}/driver_ln882x/src/hal/hal_sleep.c
    ${MCU_LN882X_DIR}/driver_ln882x/src/hal/hal_spim.c
    ${MCU_LN882X_DIR}/driver_ln882x/src/hal/hal_spis.c
    ${MCU_LN882X_DIR}/driver_ln882x/src/hal/hal_syscon.c
    ${MCU_LN882X_DIR}/driver_ln882x/src/hal/hal_timer.c
    ${MCU_LN882X_DIR}/driver_ln882x/src/hal/hal_trng.c
    ${MCU_LN882X_DIR}/driver_ln882x/src/hal/hal_uart.c
    ${MCU_LN882X_DIR}/driver_ln882x/src/hal/hal_wdt.c
    ${MCU_LN882X_DIR}/driver_ln882x/src/hal/qspi.c
    ${MCU_LN882X_DIR}/driver_ln882x/src/hal/simi2c.c
    ${MCU_LN882X_DIR}/driver_ln882x/src/ll/ll_gpio.c  # exclude from libdriver_ln882x.a
    ${MCU_LN882X_DIR}/ln88xx/system_ln88xx.c
    ${COMP_WIFI_DIR}/wifi_manager/wifi_manager.c
    ${COMP_WIFI_DIR}/wifi_port/wifi_port.c
)

set(phony_static_target   "phony")
add_library(${phony_static_target}  STATIC  ${PHONY_SRC})
add_library(components::phony  ALIAS  ${phony_static_target})

target_link_libraries(${phony_static_target}
    PUBLIC
    components::utils
)

target_include_directories(${phony_static_target}
    PUBLIC
    ${CMAKE_SOURCE_DIR}/project/${USER_PROJECT}/bsp
    ${CMAKE_SOURCE_DIR}/project/${USER_PROJECT}/cfg
    ${MCU_LN882X_DIR}/ln88xx
    ${MCU_LN882X_DIR}/CMSIS_5.3.0
    ${MCU_LN882X_DIR}/driver_ln882x/inc
    ${MCU_LN882X_DIR}/driver_ln882x/inc/reg
    ${CMAKE_SOURCE_DIR}/components
    ${COMP_KERNEL_DIR}
    ${COMP_KERNEL_DIR}/FreeRTOS/Source/include
    ${COMP_KERNEL_DIR}/FreeRTOS/Source/portable/GCC/ARM_CM4F
    ${COMP_LWIP_DIR}/include
    ${COMP_LWIP_DIR}/port
    ${COMP_LWIP_DIR}/port/arch
    ${COMP_DHCPD_DIR}
    ${COMP_KV_DIR}/kv
    ${COMP_KV_DIR}/kv_port
    ${COMP_NVDS_DIR}
    ${COMP_ATCMD_DIR}
    ${EXTRA_INCLUDE_PATH}
)
