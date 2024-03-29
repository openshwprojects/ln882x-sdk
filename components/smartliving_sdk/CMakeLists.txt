if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "in-tree build not allowed")
endif(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})

# add_definitions (-DDLL_IOT_EXPORTS)  # NOTE: not supported for embedded OS.

set(COMP_MBEDTLS_SUPPORT  ON  PARENT_SCOPE)

add_definitions (-DAWSS_DISABLE_ENROLLEE)
add_definitions (-DAWSS_DISABLE_REGISTRAR)
# 这个选项来自 default_settings.mk
add_definitions (-DAWSS_REGISTRAR_LOWPOWER_EN)
# add_definitions (-DAWSS_SUPPORT_ADHA)
# add_definitions (-DAWSS_SUPPORT_AHA)
add_definitions (-DAWSS_SUPPORT_APLIST)
add_definitions (-DAWSS_SUPPORT_DEV_AP)
# add_definitions (-DAWSS_SUPPORT_PHONEASAP)
# add_definitions (-DAWSS_SUPPORT_ROUTER)
add_definitions (-DAWSS_SUPPORT_SMARTCONFIG)
add_definitions (-DAWSS_SUPPORT_SMARTCONFIG_MCAST)
# add_definitions (-DAWSS_SUPPORT_SMARTCONFIG_WPS)
add_definitions (-DAWSS_SUPPORT_ZEROCONFIG)
add_definitions (-DCOAP_SERV_MULTITHREAD)
add_definitions (-DCONFIG_HTTP_AUTH_TIMEOUT=10000)
add_definitions (-DCONFIG_MID_HTTP_TIMEOUT=10000)
add_definitions (-DCONFIG_GUIDER_AUTH_TIMEOUT=10000)
add_definitions (-DDEVICE_MODEL_ENABLED)
add_definitions (-DDEV_BIND_ENABLED)
add_definitions (-DFORCE_SSL_VERIFY)
add_definitions (-DMQTT_COMM_ENABLED)
add_definitions (-DMQTT_DIRECT)
add_definitions (-DOTA_ENABLED)
add_definitions (-DOTA_SIGNAL_CHANNEL=1)
add_definitions (-DSUPPORT_TLS)
add_definitions (-DDEV_STATEMACHINE_ENABLE)
add_definitions (-DWIFI_PROVISION_ENABLED)
add_definitions (-DWITH_MQTT_ZIP_TOPIC=1)
add_definitions (-DWITH_MQTT_SUB_SHORTCUT=1)
add_definitions (-DWITH_MQTT_DYN_BUF=1)

add_subdirectory (${COMP_SMARTLIVING_DIR}/components/timer_service)
add_subdirectory (${COMP_SMARTLIVING_DIR}/src/infra/utils)
add_subdirectory (${COMP_SMARTLIVING_DIR}/src/infra/log)
add_subdirectory (${COMP_SMARTLIVING_DIR}/src/infra/system)
add_subdirectory (${COMP_SMARTLIVING_DIR}/src/sdk-impl)
add_subdirectory (${COMP_SMARTLIVING_DIR}/src/protocol/mqtt)
add_subdirectory (${COMP_SMARTLIVING_DIR}/src/services/awss)
add_subdirectory (${COMP_SMARTLIVING_DIR}/src/services/dev_diagnosis)
add_subdirectory (${COMP_SMARTLIVING_DIR}/src/services/dev_bind)
add_subdirectory (${COMP_SMARTLIVING_DIR}/src/services/linkkit/cm)
add_subdirectory (${COMP_SMARTLIVING_DIR}/src/services/linkkit/dm)
add_subdirectory (${COMP_SMARTLIVING_DIR}/src/services/linkkit/dev_reset)
add_subdirectory (${COMP_SMARTLIVING_DIR}/src/services/ota)
add_subdirectory (${COMP_SMARTLIVING_DIR}/src/protocol/coap/local)
add_subdirectory (${COMP_SMARTLIVING_DIR}/src/ref-impl/hal)

# NOTE: it's a mbedtls of smartliving version, be careful it conflicts with LN mbedtls.
# add_subdirectory (${COMP_SMARTLIVING_DIR}/src/ref-impl/tls)

add_library (iot_sdk STATIC
    $<TARGET_OBJECTS:iot_timer_service>
    $<TARGET_OBJECTS:iot_utils>
    $<TARGET_OBJECTS:iot_log>
    $<TARGET_OBJECTS:iot_system>
    $<TARGET_OBJECTS:sdk_impl>
    $<TARGET_OBJECTS:iot_mqtt>
    $<TARGET_OBJECTS:iot_ota>
    $<TARGET_OBJECTS:iot_cm>
    $<TARGET_OBJECTS:iot_dm>
    $<TARGET_OBJECTS:iot_reset>
    $<TARGET_OBJECTS:iot_coap_local>
    $<TARGET_OBJECTS:iot_awss>
    $<TARGET_OBJECTS:iot_dev_diagnosis>
    $<TARGET_OBJECTS:iot_bind>
)

target_link_directories(iot_sdk
    PUBLIC
    ${COMP_SMARTLIVING_DIR}/lib/Cortex-M4/
)

target_link_libraries(iot_sdk  awss_security)
target_link_libraries(iot_sdk  iot_hal)
