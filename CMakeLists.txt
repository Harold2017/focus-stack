cmake_minimum_required(VERSION 3.17)

if(WIN32)
    set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE INTERNAL "" FORCE)
endif()

project(focus-stack)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)
find_package(Qt6 CONFIG REQUIRED COMPONENTS Core Widgets)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(PROJECT_LIB ${PROJECT_NAME}_lib)
set(PROJECT_TEST ${PROJECT_NAME}_test)
set(PROJECT_UI_DIR ${CMAKE_SOURCE_DIR}/ui)
set(PROJECT_UI ${PROJECT_NAME}_ui)

option(BUILD_TESTS "build tests" ON)

file(GLOB_RECURSE PROJECT_LIB_SOURCES LIST_DIRECTORIES false CONFIGURE_DEPENDS
    ${SRC_DIR}/*.cc ${SRC_DIR}/*.hh ${SRC_DIR}/*cl
)
list(FILTER PROJECT_LIB_SOURCES EXCLUDE REGEX .*_tests.cc|.*main.cc)

add_library(${PROJECT_LIB}
    STATIC # set to SHARED if you want dll
    ${PROJECT_LIB_SOURCES}
)
target_link_libraries(${PROJECT_LIB}
    PUBLIC ${OpenCV_LIBS}
)
if(MSVC)
    target_compile_definitions(${PROJECT_LIB} PRIVATE _USE_MATH_DEFINES)
endif()

add_executable(${PROJECT_NAME} ${SRC_DIR}/main.cc)
target_link_libraries(${PROJECT_NAME}
    PRIVATE ${PROJECT_LIB}
)

add_executable(${PROJECT_UI}
    ${PROJECT_UI_DIR}/main.cpp
    ${PROJECT_UI_DIR}/MainWindow.cpp ${PROJECT_UI_DIR}/MainWindow.h ${PROJECT_UI_DIR}/MainWindow.ui
)
target_link_libraries(${PROJECT_UI}
    PRIVATE Qt${QT_VERSION_MAJOR}::Core
    PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
)

if (BUILD_TESTS)
    enable_testing()
    find_package(GTest CONFIG REQUIRED)
    file(GLOB_RECURSE PROJECT_TEST_SOURCES LIST_DIRECTORIES false CONFIGURE_DEPENDS
        ${SRC_DIR}/*_tests.cc
    )
    add_executable(${PROJECT_TEST} ${PROJECT_TEST_SOURCES} ${SRC_DIR}/gtest_main.cc)
    target_link_libraries(${PROJECT_TEST}
        PRIVATE ${PROJECT_LIB}
        PRIVATE GTest::gtest_main
    )
    add_test(NAME ${PROJECT_TEST} COMMAND ${PROJECT_TEST})
endif()