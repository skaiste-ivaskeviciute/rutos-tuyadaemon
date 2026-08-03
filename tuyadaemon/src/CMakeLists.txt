cmake_minimum_required(VERSION 3.28)
project(TuyaDaemonProject LANGUAGES C)

set(TARGET tuyadaemon)
set(SOURCES main.c make_daemon.c message_handler.c read_network_data.c read_system_data.c argument_parsing.c cloud_connection.c interfaces_list.c)

add_executable(${TARGET} ${SOURCES})

target_link_libraries(
    ${TARGET}
    PUBLIC
        utils_modules
        platform_port
        middleware_implementation
        link_core
        ubus
        ubox
        blobmsg_json
        argp
        uci
)
