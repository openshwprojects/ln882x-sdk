file(GLOB_RECURSE  IPERF_SRC  ${COMP_IPERF_DIR}  "*.c")

set(iperf_static_target "iperf")
add_library(${iperf_static_target} STATIC ${IPERF_SRC})
add_library(components::net::iperf ALIAS ${iperf_static_target})

target_link_libraries(${iperf_static_target} components::net::lwip)
target_include_directories(${iperf_static_target}
    PUBLIC
    ${COMP_IPERF_DIR}
)
