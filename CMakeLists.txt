cmake_minimum_required(VERSION 3.31)

project(logisimp VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
)

add_executable(logisimp ${SOURCES})

target_include_directories(logisimp PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_compile_options(${PROJECT_NAME}
    PRIVATE
    $<$<CONFIG:Debug>:-g -fsanitize=address>
)

target_link_options(${PROJECT_NAME}
    PRIVATE
    $<$<CONFIG:Debug>:-fsanitize=address>
)

add_custom_target(r
    COMMAND ./logisimp
    DEPENDS logisimp
)

add_custom_target(c
    COMMAND ${CMAKE_COMMAND} --build . --target clean
)