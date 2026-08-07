# ==========================================
# @file src/database/CMakeLists.txt
# Build SQLite database library
# ==========================================

set(DATABASE_NAME database)

add_library(${DATABASE_NAME} STATIC
    database.cpp
)

target_include_directories(${DATABASE_NAME} PUBLIC
    ${PROJECT_INCLUDE_DIR}
)

target_link_libraries(${DATABASE_NAME} PRIVATE
    SQLite::SQLite3
)

configure_compile_target(${DATABASE_NAME})
configure_link_target(${DATABASE_NAME})
