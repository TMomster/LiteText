cmake_minimum_required(VERSION 3.20)
project(LiteText VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

if(WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mwindows")
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

set(SRCS
    src/main.cpp
    src/MainWindow.cpp
    src/Editor.cpp
    src/SyntaxHighlighter.cpp
    src/FindReplaceDialog.cpp
    src/SettingsDialog.cpp
)

set(HEADERS
    src/MainWindow.h
    src/Editor.h
    src/SyntaxHighlighter.h
    src/FindReplaceDialog.h
    src/SettingsDialog.h
)

add_executable(${PROJECT_NAME} ${SRCS} ${HEADERS})

if(WIN32 AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/resources/app.rc")
    target_sources(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/resources/app.rc")
endif()

target_link_libraries(${PROJECT_NAME} Qt6::Core Qt6::Widgets)

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

message(STATUS "Build configuration for ${PROJECT_NAME}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Qt version: ${Qt6_VERSION}")
