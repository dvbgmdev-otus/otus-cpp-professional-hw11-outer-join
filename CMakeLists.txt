cmake_minimum_required(VERSION 3.12)

if(1) # PROJECT CONFIG BLOCK

    set(PATCH_VERSION "1" CACHE INTERNAL "Patch version")

    project(bulk_server VERSION 0.0.${PATCH_VERSION})

    # === Путь для модулей CMake ===
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
    # === Путь до исходников ===
    set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")
    # Корень покрываемых исходников
    set(COVERAGE_BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

    set(PROJECT_INCLUDE_DIR
        ${SRC_DIR}
        ${SRC_DIR}/async
        ${SRC_DIR}/core
        ${SRC_DIR}/core/cli
        ${SRC_DIR}/core/format
        ${SRC_DIR}/core/io
        ${SRC_DIR}/core/utils
        ${SRC_DIR}/database
        ${SRC_DIR}/main
        ${SRC_DIR}/server
    )

    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

    include(compiler_standard)
    include(warnings_config)
    include(build_type_config)
    include(options_config)
    include(target_config)

endif()

if(1) # PROJECT DEPENDENCIES BLOCK
    find_package(Boost REQUIRED COMPONENTS system)
    find_package(SQLite3 REQUIRED)
    find_package(Threads REQUIRED)
endif()

if(1) # COVERAGE CONFIG BLOCK
    set(COVERAGE_SRC
        ${SRC_DIR}/database/database.cpp
        ${SRC_DIR}/async/async_command_processor.cpp
        ${SRC_DIR}/async/async_context.cpp
        ${SRC_DIR}/async/async_runtime.cpp
        ${SRC_DIR}/async/async.cpp
        ${SRC_DIR}/core/cli/cli_args.cpp
        ${SRC_DIR}/core/format/bulk_format.cpp
        ${SRC_DIR}/core/io/bulk_writer.cpp
        ${SRC_DIR}/core/io/command_reader.cpp
        ${SRC_DIR}/core/bulk_processor.cpp
        ${SRC_DIR}/main/main.cpp
        ${SRC_DIR}/server/tcp_server.cpp
        ${SRC_DIR}/server/tcp_session.cpp
    )
endif()

if(1) # DATABASE LIBRARY BLOCK
    add_subdirectory(${SRC_DIR}/database ${CMAKE_BINARY_DIR}/database)
endif()

if(1) # CORE LIBRARY BLOCK
    set(CORE_NAME ${PROJECT_NAME}_CORE)
    add_subdirectory(${SRC_DIR}/core ${CMAKE_BINARY_DIR}/core)
endif()

if(1) # ASYNC LIBRARY BLOCK
    add_subdirectory(${SRC_DIR}/async ${CMAKE_BINARY_DIR}/async)
endif()

if(1) # SERVER LIBRARY BLOCK
    add_subdirectory(${SRC_DIR}/server ${CMAKE_BINARY_DIR}/server)
endif()

if(1) # MAIN APPLICATION BLOCK
    add_subdirectory(${SRC_DIR}/main ${CMAKE_BINARY_DIR}/main)
endif()

if(1) # PROJECT METADATA BLOCK
    # Генерация файла с именем бинарного файла (нужно будет для скриптов)
    file(GENERATE
        OUTPUT "${CMAKE_BINARY_DIR}/project_binary_name.txt"
        CONTENT "${PROJECT_NAME}\n"
    )
endif()

if(1) # ACTIVATE BUILD TESTS BLOCK
    if (BUILD_TESTING)
        enable_testing()
        add_subdirectory(tests)
    endif()
endif()

if(1) # CPACK BLOCK
    set(CPACK_GENERATOR DEB)

    set(CPACK_PACKAGE_NAME bulk_server)

    set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
    set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
    set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})

    set(CPACK_PACKAGE_CONTACT "d.v.bogomaz@gmail.com")

    include(CPack)
endif()

include(summary_config)
#include(post_build)
