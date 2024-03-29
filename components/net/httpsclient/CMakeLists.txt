file(GLOB_RECURSE  HTTPSCLIENT_SRC  ${COMP_HTTPSCLIENT_DIR}  "*.c")

set(httpsclient_static_target  "httpsclient")
add_library(${httpsclient_static_target}  STATIC  ${HTTPSCLIENT_SRC})
add_library(components::net::httpsclient  ALIAS  ${httpsclient_static_target})

target_link_libraries(${httpsclient_static_target}
    PUBLIC
    components::net::lwip
    components::net::mbedtls
)

target_include_directories(${httpsclient_static_target}
    PUBLIC
    ${COMP_HTTPSCLIENT_DIR}/inc
    ${COMP_MBEDTLS_DIR}/include
)
