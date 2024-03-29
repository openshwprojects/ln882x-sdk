cmake_minimum_required(VERSION  3.0.0)

################################################################################
#######################  NOTE: select user project first    ####################
################################################################################
set(USER_PROJECT   wifi_mcu_basic_example)
# set(USER_PROJECT   wifi_mcu_airkiss_demo)
# set(USER_PROJECT   wifi_mcu_ota_demo)
# set(USER_PROJECT   wifi_mcu_qcloud_misc_demo)
# set(USER_PROJECT   wifi_mcu_smartconfig_demo)
# set(USER_PROJECT   wifi_mcu_smartliving_demo)
# set(USER_PROJECT   wifi_mcu_softapconfig_demo)

#########################  set up cross compiler toolchain  ####################
include(${CMAKE_SOURCE_DIR}/project/${USER_PROJECT}/gcc/gcc-arm-none-eabi.cmake)

################################################################################
###########################  project language selection   ######################
################################################################################
project(${USER_PROJECT}
    VERSION    1.0.0
    LANGUAGES  C  ASM
)

################################################################################
##############################  Debug/Release  #################################
################################################################################
# set(CMAKE_BUILD_TYPE Release) # default value is Debug.

if(NOT  CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE  Debug)
endif(NOT  CMAKE_BUILD_TYPE)

message(STATUS  "Build type: ${CMAKE_BUILD_TYPE}")


################################################################################
#############################   misc settings   ################################
################################################################################

#-------------------------------   output path   -------------------------------
set(EXECUTABLE_OUTPUT_PATH    ${CMAKE_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH       ${CMAKE_BINARY_DIR}/lib)

#----------------------------   project output elf   ---------------------------
set(TARGET_ELF_NAME  ${PROJECT_NAME}.elf)

################################################################################
##########################   default module selection   ########################
option(COMP_MCU_LN882X_SUPPORT  "MCU LN882X driver."                                            OFF)
option(COMP_WIFI_SUPPORT        "WiFi firmware."                                                OFF)
option(COMP_ATCMD_SUPPORT       "For WiFi operation && Iperf && Ping test."                     OFF)
option(COMP_FOTA_SUPPORT        "OTA image process."                                            OFF)
option(COMP_KV_SUPPORT          "Lightweight persisent storage module."                         ON)
option(COMP_NVDS_SUPPORT        "Non Volatile Data Storage driver."                             OFF)
option(COMP_PARTMGR_SUPPORT     "Partition Manager."                                            OFF)
option(COMP_KERNEL_SUPPORT      "OS and adapter layer."                                         ON)
option(COMP_DHCPD_SUPPORT       "DHCP client."                                                  ON)
option(COMP_HTTPSCLIENT_SUPPORT "A simple HTTP client."                                         OFF)
option(COMP_IPERF_SUPPORT       "Iperf-liked network performance tool."                         OFF)
option(COMP_LWIP_SUPPORT        "Light weight TCP/IP stack."                                    ON)
option(COMP_MBEDTLS_SUPPORT     "MbedTLS library."                                              OFF)
option(COMP_PING_SUPPORT        "Ping tool."                                                    ON)
option(COMP_SERIAL_SUPPORT      "Serial driver."                                                ON)
option(COMP_AIRKISS_SUPPORT     "AirKiss for device network setup."                             OFF)
option(COMP_SMARTCFG_SUPPORT    "SmartConfig for device network setup."                         OFF)
option(COMP_SOFTAPCFG_SUPPORT   "SoftAP for device network setup."                              OFF)
option(COMP_SYSTEMVIEW_SUPPORT  "A toolkit for visual analysis of any embedded system"          OFF)
option(COMP_UTILS_SUPPORT       "Utils for misc operations, must be built."                     ON)
option(COMP_QCLOUD_SDK_SUPPORT  "Tencent Qcloud SDK."                                           OFF)

################################################################################
###########################   define module top dir   ##########################
################################################################################
set(MCU_LN882X_DIR        ${CMAKE_SOURCE_DIR}/mcu)
set(COMP_ATCMD_DIR        ${CMAKE_SOURCE_DIR}/components/atcmd)
set(COMP_FOTA_DIR         ${CMAKE_SOURCE_DIR}/components/fota)
set(COMP_KV_DIR           ${CMAKE_SOURCE_DIR}/components/fs/kv)
set(COMP_NVDS_DIR         ${CMAKE_SOURCE_DIR}/components/fs/nvds)
set(COMP_PARTMGR_DIR      ${CMAKE_SOURCE_DIR}/components/fs/partition_mgr)
set(COMP_KERNEL_DIR       ${CMAKE_SOURCE_DIR}/components/kernel)
set(COMP_DHCPD_DIR        ${CMAKE_SOURCE_DIR}/components/net/dhcpd)
set(COMP_HTTPSCLIENT_DIR  ${CMAKE_SOURCE_DIR}/components/net/httpsclient)
set(COMP_IPERF_DIR        ${CMAKE_SOURCE_DIR}/components/net/iperf)
set(COMP_LWIP_DIR         ${CMAKE_SOURCE_DIR}/components/net/lwip-2.0.3/src)
set(COMP_MBEDTLS_DIR      ${CMAKE_SOURCE_DIR}/components/net/mbedtls)
set(COMP_PING_DIR         ${CMAKE_SOURCE_DIR}/components/net/ping)
set(COMP_SERIAL_DIR       ${CMAKE_SOURCE_DIR}/components/serial)
set(COMP_AIRKISS_DIR      ${CMAKE_SOURCE_DIR}/components/smartlink/airkiss)
set(COMP_SMARTCFG_DIR     ${CMAKE_SOURCE_DIR}/components/smartlink/smartcfg)
set(COMP_SOFTAPCFG_DIR    ${CMAKE_SOURCE_DIR}/components/smartlink/softapcfg)
set(COMP_SYSTEMVIEW_DIR   ${CMAKE_SOURCE_DIR}/components/systemview)
set(COMP_UTILS_DIR        ${CMAKE_SOURCE_DIR}/components/utils)
set(COMP_WIFI_DIR         ${CMAKE_SOURCE_DIR}/components/wifi)
set(COMP_QCLOUD_SDK_DIR   ${CMAKE_SOURCE_DIR}/components/qcloud_iot_c_sdk)
set(COMP_SMARTLIVING_DIR  ${CMAKE_SOURCE_DIR}/components/smartliving_SDK)
set(COMP_PHONY_DIR        ${CMAKE_SOURCE_DIR}/components/phony)

################################################################################
#############################    user project    ###############################
################################################################################
include(${CMAKE_SOURCE_DIR}/project/${USER_PROJECT}/gcc/gcc-flags.cmake)
add_subdirectory(${CMAKE_SOURCE_DIR}/project/${USER_PROJECT})


################################################################################
#############################   module settings   ##############################
################################################################################

#--------------------------   systemview settings   ----------------------------
if(COMP_SYSTEMVIEW_SUPPORT)
  add_definitions(-DSYSTEMVIEW_ENABLED)
endif(COMP_SYSTEMVIEW_SUPPORT)

# NOTE: header path MUST be provided whether systemview enabled or disabled.
set(EXTRA_INCLUDE_PATH
  ${EXTRA_INCLUDE_PATH}
  ${COMP_SYSTEMVIEW_DIR}/Config
  ${COMP_SYSTEMVIEW_DIR}/Sample/FreeRTOSV9
  ${COMP_SYSTEMVIEW_DIR}/SEGGER
)

################################################################################
###########################  select module libraries  ##########################
################################################################################
if(COMP_MCU_LN882X_SUPPORT)
  add_subdirectory(${MCU_LN882X_DIR})
endif(COMP_MCU_LN882X_SUPPORT)

if(COMP_ATCMD_SUPPORT)
  add_subdirectory(${COMP_ATCMD_DIR})
endif(COMP_ATCMD_SUPPORT)

if(COMP_FOTA_SUPPORT)
  add_subdirectory(${COMP_FOTA_DIR})
endif(COMP_FOTA_SUPPORT)

if(COMP_KV_SUPPORT)
  add_subdirectory(${COMP_KV_DIR})
endif(COMP_KV_SUPPORT)

if(COMP_NVDS_SUPPORT)
  add_subdirectory(${COMP_NVDS_DIR})
endif(COMP_NVDS_SUPPORT)

if(COMP_PARTMGR_SUPPORT)
  add_subdirectory(${COMP_PARTMGR_DIR})
endif(COMP_PARTMGR_SUPPORT)

if(COMP_KERNEL_SUPPORT)
  add_subdirectory(${COMP_KERNEL_DIR})
endif(COMP_KERNEL_SUPPORT)

if(COMP_DHCPD_SUPPORT)
  add_subdirectory(${COMP_DHCPD_DIR})
endif(COMP_DHCPD_SUPPORT)

if(COMP_HTTPSCLIENT_SUPPORT)
  add_subdirectory(${COMP_HTTPSCLIENT_DIR})
endif(COMP_HTTPSCLIENT_SUPPORT)

if(COMP_IPERF_SUPPORT)
  add_subdirectory(${COMP_IPERF_DIR})
endif(COMP_IPERF_SUPPORT)

if(COMP_LWIP_SUPPORT)
  add_subdirectory(${COMP_LWIP_DIR})
endif(COMP_LWIP_SUPPORT)

if(COMP_QCLOUD_SDK_SUPPORT)
  add_subdirectory(${COMP_QCLOUD_SDK_DIR})
endif(COMP_QCLOUD_SDK_SUPPORT)

if(COMP_SMARTLIVING_SUPPORT)
  add_subdirectory(${COMP_SMARTLIVING_DIR})
endif(COMP_SMARTLIVING_SUPPORT)

if(COMP_MBEDTLS_SUPPORT)
  add_subdirectory(${COMP_MBEDTLS_DIR})
endif(COMP_MBEDTLS_SUPPORT)

if(COMP_PING_SUPPORT)
  add_subdirectory(${COMP_PING_DIR})
endif(COMP_PING_SUPPORT)

if(COMP_SERIAL_SUPPORT)
  add_subdirectory(${COMP_SERIAL_DIR})
endif(COMP_SERIAL_SUPPORT)

if(COMP_AIRKISS_SUPPORT)
  add_subdirectory(${COMP_AIRKISS_DIR})
endif(COMP_AIRKISS_SUPPORT)

if(COMP_SMARTCFG_SUPPORT)
  add_subdirectory(${COMP_SMARTCFG_DIR})
endif(COMP_SMARTCFG_SUPPORT)

if(COMP_SOFTAPCFG_SUPPORT)
  add_subdirectory(${COMP_SOFTAPCFG_DIR})
endif(COMP_SOFTAPCFG_SUPPORT)

if(COMP_SYSTEMVIEW_SUPPORT)
  add_subdirectory(${COMP_SYSTEMVIEW_DIR})
endif(COMP_SYSTEMVIEW_SUPPORT)

if(COMP_UTILS_SUPPORT)
  add_subdirectory(${COMP_UTILS_DIR})
endif(COMP_UTILS_SUPPORT)

if(COMP_WIFI_SUPPORT)
  add_subdirectory(${COMP_WIFI_DIR})
endif(COMP_WIFI_SUPPORT)

add_subdirectory(${COMP_PHONY_DIR})
