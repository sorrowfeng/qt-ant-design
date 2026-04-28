cmake_minimum_required(VERSION 3.16)

project(qt-ant-design
    VERSION 0.1.0
    LANGUAGES CXX
)

include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/FS)
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Svg)

# Options
option(BUILD_EXAMPLES "Build example application" ON)
option(BUILD_TESTS "Build unit tests (requires Qt6::Test)" ON)

# Library
add_subdirectory(src)

# Examples
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Tests
if(BUILD_TESTS)
    find_package(Qt6 COMPONENTS Test)
    if(Qt6Test_FOUND)
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

# Windows deployment
if(BUILD_EXAMPLES AND WIN32)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${Qt6_DIR}/../../../bin")
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
