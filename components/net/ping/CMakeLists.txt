file(GLOB_RECURSE  PING_SRC  ${COMP_PING_DIR}  "*.c")

set(ping_static_target "ping")
add_library(${ping_static_target} STATIC ${PING_SRC})
add_library(components::net::ping ALIAS ${ping_static_target})

target_link_libraries(${ping_static_target} components::net::lwip)
target_include_directories(${ping_static_target}
    PUBLIC
    ${COMP_PING_DIR}
)
