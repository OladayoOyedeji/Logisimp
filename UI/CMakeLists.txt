cmake_minimum_required(VERSION 3.16)

project(Logisimp VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Keep this prototype's compilation database and diagrams separate from core.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
include("${CMAKE_CURRENT_SOURCE_DIR}/../cmake/Uml.cmake")
logisimp_add_uml_targets("${CMAKE_CURRENT_SOURCE_DIR}/../.clang-uml-ui" gui_prototype)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets OpenGLWidgets)

# Logic engine located one directory above UI
file(GLOB LOGIC_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/*.cpp"
)

# Exclude standalone programs and old tests
list(REMOVE_ITEM LOGIC_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/main.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/Test.cpp"
)

add_library(logic_engine STATIC ${LOGIC_SOURCES})

target_include_directories(logic_engine PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/../include"
)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(Logisimp
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        canvas.h canvas.cpp
        circuit.h circuit.cpp
        gate.h gate.cpp
        gates.h
        port.h port.cpp
        wire.h wire.cpp
        spritesheet.h spritesheet.cpp
		circuitcontroller.h circuitcontroller.cpp	
    )

# Define target properties for Android with Qt 6 as:
#    set_property(TARGET Logisimp APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(Logisimp SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(Logisimp
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(Logisimp PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::OpenGLWidgets
	logic_engine
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.Logisimp)
endif()
set_target_properties(Logisimp PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS Logisimp
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(Logisimp)
endif()

# clang-uml needs ui_mainwindow.h and the other Qt-generated headers to exist.
add_dependencies(uml Logisimp_autogen)
