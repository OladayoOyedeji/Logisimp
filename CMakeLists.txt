
cmake_minimum_required(VERSION 3.31)

project(logisimp VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Generate compilation database for clang-uml.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Expose compiler standard library include paths to clang-uml.
# Fixes missing headers such as <ostream> when using MinGW on Windows.
set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES
    ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES}
)

# UML generation
include(cmake/Uml.cmake)
logisimp_add_uml_targets("${CMAKE_CURRENT_SOURCE_DIR}/.clang-uml" simulator_full simulator_architecture)

# Source files
file(GLOB SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
)

# Exclude Emacs lock files.
list(FILTER SOURCES EXCLUDE REGEX "/[.]#")

# Archived examples use a retired API.
list(REMOVE_ITEM SOURCES "${CMAKE_SOURCE_DIR}/src/Test.cpp")

# Main executable
add_executable(logisimp ${SOURCES})

target_include_directories(logisimp PRIVATE
    "${CMAKE_SOURCE_DIR}/include"
)

# Debug and AddressSanitizer
target_compile_options(logisimp PRIVATE
    $<$<CONFIG:Debug>:-g>
    $<$<CONFIG:Debug>:-fsanitize=address>
)

target_link_options(logisimp PRIVATE
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
