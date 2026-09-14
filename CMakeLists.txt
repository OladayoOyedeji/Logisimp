cmake_minimum_required(VERSION 3.31)

project(logisimp VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
    src/main.cpp
)

add_executable(logisimp ${SOURCES})

target_include_directories(logisimp PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

add_custom_target(r
    COMMAND ./logisimp
    DEPENDS logisimp
)

add_custom_target(c
    COMMAND ${CMAKE_COMMAND} --build . --target clean
)
