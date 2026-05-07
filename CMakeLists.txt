cmake_minimum_required(VERSION 3.16)

project(qt-ant-design
    VERSION 0.1.0
    LANGUAGES CXX
)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/FS)
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets Svg)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Svg)

set(QT_ANT_DESIGN_QT_MAJOR_VERSION ${QT_VERSION_MAJOR})
set(QT_ANT_DESIGN_QT_PACKAGE Qt${QT_VERSION_MAJOR})

message(STATUS "qt-ant-design using ${QT_ANT_DESIGN_QT_PACKAGE}")

# Options
option(BUILD_EXAMPLES "Build example application" ON)
option(BUILD_TESTS "Build unit tests (requires Qt Test)" ON)

# Library
add_subdirectory(src)

# Examples
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Tests
if(BUILD_TESTS)
    find_package(${QT_ANT_DESIGN_QT_PACKAGE} COMPONENTS Test)
    if(${QT_ANT_DESIGN_QT_PACKAGE}Test_FOUND)
        enable_testing()
        add_subdirectory(tests)
    endif()
endif()

# Install targets
set(INSTALL_TARGETS qt-ant-design)
if(BUILD_EXAMPLES)
    list(APPEND INSTALL_TARGETS qt-ant-design-example)
endif()

install(TARGETS ${INSTALL_TARGETS}
    EXPORT qt-ant-design-targets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(EXPORT qt-ant-design-targets
    NAMESPACE qt-ant-design::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/qt-ant-design
)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/qt-ant-designConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/qt-ant-designConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/qt-ant-design
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/qt-ant-designConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/qt-ant-designConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/qt-ant-designConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/qt-ant-design
)

# Windows deployment
if(BUILD_EXAMPLES AND WIN32)
    set(QT_ANT_DESIGN_QT_DIR_VARIABLE "${QT_ANT_DESIGN_QT_PACKAGE}_DIR")
    set(QT_ANT_DESIGN_QT_DIR "${${QT_ANT_DESIGN_QT_DIR_VARIABLE}}")
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${QT_ANT_DESIGN_QT_DIR}/../../../bin")
    if(WINDEPLOYQT_EXECUTABLE)
        install(CODE "
            execute_process(
                COMMAND \"${WINDEPLOYQT_EXECUTABLE}\" \"\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}/qt-ant-design-example${CMAKE_EXECUTABLE_SUFFIX}\"
                RESULT_VARIABLE windeployqt_result
            )
            if(NOT windeployqt_result EQUAL 0)
                message(WARNING \"windeployqt failed with exit code \${windeployqt_result}\")
            endif()
        ")
    else()
        message(WARNING "windeployqt was not found. Installed example may require Qt DLLs in PATH.")
    endif()
endif()
