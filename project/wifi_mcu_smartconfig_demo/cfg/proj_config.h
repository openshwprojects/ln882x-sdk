
#ifndef _PROJ_CONFIG_H_
#define _PROJ_CONFIG_H_

#include "common_def_ln88xx.h"

#define __CONFIG_OS_KERNEL                      RTOS_FREERTOS

/*
 * Clock settings section
 */
#define USE_PLL                                 ENABLE

#if (USE_PLL == ENABLE)
  #define SOURCE_CLOCK                          PLL_CLOCK
#else
  #define SOURCE_CLOCK                          XTAL_CLOCK
#endif

#define SYSTEM_CLOCK                            (SOURCE_CLOCK)
#define AHBUS_CLOCK                             (SYSTEM_CLOCK)
#define APBUS0_CLOCK                            (AHBUS_CLOCK/2)
#define APBUS1_CLOCK                            (AHBUS_CLOCK/4)
#define APBUS2_CLOCK                            (AHBUS_CLOCK)
#if (USE_PLL == ENABLE)
  #define QSPI_CLK                              (APBUS2_CLOCK/4)
#else
  #define QSPI_CLK                              (APBUS2_CLOCK/2)
#endif

#if ((AHBUS_CLOCK % APBUS0_CLOCK) || (AHBUS_CLOCK % APBUS1_CLOCK))
  #error "AHBUS_CLOCK % APBUS0_CLOCK != 0 or AHBUS_CLOCK % APBUS1_CLOCK != 0"
#endif


/*
 * Module enable/disable control
 */
#define FLASH_XIP                             ENABLE

#define CHIP_ROLE                             CHIP_MCU

#define FULL_ASSERT                           ENABLE

#define PRINTF_OMIT                           DISABLE     // when release software, set 1 to omit all printf logs

#define WIFI_TRACK                            DISABLE     // enable when for wifi locate apply
#define AT_LOG_MERGE_TO_UART0                 DISABLE     // LOG and AT both use UART0

#define WIFI_SWITCH                           DISABLE

#define OS_TICK_COMPENSATE


//Check big and endian mode
#if defined ( __CC_ARM )
  #if defined (__BIG_ENDIAN)
    #error "Please set the compiler to little-endian mode"
  #endif
#elif defined ( __GNUC__ )
  #if (__BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__)
    #error "Please set the compiler to little-endian mode"
  #endif // __BYTE_ORDER__
#else
  #error "Unsupported compiler"
#endif

#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN 1234
#endif

#define BLE_SUPPORT DISABLE
#if BLE_SUPPORT == ENABLE//ble_normal_use and ble_mesh should support only one at the same time.
#define BLE_MESH_SUPPORT    DISABLE//DISABLE//ENABLE//

#if BLE_MESH_SUPPORT==ENABLE
#define BLE_MESH_SUPPORT_SERVER  ENABLE//DISABLE // //Enable: server, Disable: client
#endif

#endif//BLE_SUPPORT == ENABLE


/*
 * Hardware config
 */
#define CFG_UART0_TX_BUF_SIZE  256
#define CFG_UART0_RX_BUF_SIZE  256
#define CFG_UART1_TX_BUF_SIZE  128
#define CFG_UART1_RX_BUF_SIZE  256

#define CFG_UART_BAUDRATE_LOG      921600
#define CFG_UART_BAUDRATE_CONSOLE  115200

#define TCP_TEST_DATA_FROM_CONSOLE		0
#endif /* _PROJ_CONFIG_H_ */

