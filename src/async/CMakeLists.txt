# ==========================================
# @file src/async/CMakeLists.txt
# Build async library
# ==========================================

set(ASYNC_NAME async)

add_library(${ASYNC_NAME} SHARED
    async.cpp
    async_command_processor.cpp
    async_context.cpp
    async_runtime.cpp
)

target_include_directories(${ASYNC_NAME} PUBLIC
    ${PROJECT_INCLUDE_DIR}
)

target_link_libraries(${ASYNC_NAME} PRIVATE
    ${CORE_NAME}
    Threads::Threads
)

configure_compile_target(${ASYNC_NAME})
configure_link_target(${ASYNC_NAME})

install(TARGETS ${ASYNC_NAME} LIBRARY DESTINATION lib)
install(FILES async.h DESTINATION include)
