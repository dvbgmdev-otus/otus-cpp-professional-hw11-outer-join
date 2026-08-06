# ==========================================
# @file src/server/CMakeLists.txt
# Build TCP server library
# ==========================================

set(SERVER_NAME server)

add_library(${SERVER_NAME} STATIC
    tcp_server.cpp
    tcp_session.cpp
)

target_include_directories(${SERVER_NAME} PUBLIC
    ${PROJECT_INCLUDE_DIR}
)

target_link_libraries(${SERVER_NAME} PRIVATE
    async
    Boost::system
    Threads::Threads
)

configure_compile_target(${SERVER_NAME})
configure_link_target(${SERVER_NAME})
