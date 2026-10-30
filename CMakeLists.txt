
cmake_minimum_required(VERSION 3.31)

project(logisimp VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files
file(GLOB SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
)

# Exclude Emacs lock files.
list(FILTER SOURCES EXCLUDE REGEX "/[.]#")

# Archived examples use a retired API.
list(REMOVE_ITEM SOURCES "${CMAKE_SOURCE_DIR}/src/Test.cpp")

# Keep the backend usable by applications without the example main.
list(REMOVE_ITEM SOURCES "${CMAKE_SOURCE_DIR}/src/main.cpp")
add_library(logisimp_backend ${SOURCES})
target_include_directories(logisimp_backend PUBLIC
    "${CMAKE_SOURCE_DIR}/include"
)

# Main executable demonstrates circuit editing and simulation through sessions.
add_executable(logisimp "${CMAKE_SOURCE_DIR}/src/main.cpp")
target_link_libraries(logisimp PRIVATE logisimp_backend)

# Debug and AddressSanitizer. Consumers link the same sanitizer runtime.
target_compile_options(logisimp_backend PUBLIC
    $<$<CONFIG:Debug>:-g>
    $<$<CONFIG:Debug>:-fsanitize=address>
)
target_link_options(logisimp_backend PUBLIC
    $<$<CONFIG:Debug>:-fsanitize=address>
)

# Run executable
add_custom_target(r
    COMMAND "$<TARGET_FILE:logisimp>"
    DEPENDS logisimp
)

# Clean build files
add_custom_target(c
    COMMAND ${CMAKE_COMMAND} --build . --target clean
)
