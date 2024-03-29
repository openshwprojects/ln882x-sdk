file(GLOB_RECURSE  DHCPD_SRC  ${COMP_DHCPD_DIR}  "*.c")

set(dhcpd_static_target "dhcpd")
add_library(${dhcpd_static_target} STATIC ${DHCPD_SRC})
add_library(components::net::dhcpd ALIAS ${dhcpd_static_target})

target_link_libraries(${dhcpd_static_target} components::net::lwip)
target_include_directories(${dhcpd_static_target}
    PUBLIC
    ${DHCPD_DIR}
)
