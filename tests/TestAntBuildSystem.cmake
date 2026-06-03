if(NOT DEFINED ANT_SOURCE_DIR)
    message(FATAL_ERROR "ANT_SOURCE_DIR is required")
endif()

set(files_to_scan
    "${ANT_SOURCE_DIR}/CMakeLists.txt"
    "${ANT_SOURCE_DIR}/src/CMakeLists.txt"
    "${ANT_SOURCE_DIR}/examples/CMakeLists.txt"
    "${ANT_SOURCE_DIR}/examples/qt-ant-design-example.rc"
    "${ANT_SOURCE_DIR}/tests/CMakeLists.txt"
    "${ANT_SOURCE_DIR}/cmake/qt-ant-designConfig.cmake.in"
)

foreach(file_path IN LISTS files_to_scan)
    if(NOT EXISTS "${file_path}")
        message(FATAL_ERROR "Missing expected build file: ${file_path}")
    endif()
endforeach()

file(READ "${ANT_SOURCE_DIR}/CMakeLists.txt" root_cmake)
file(READ "${ANT_SOURCE_DIR}/src/CMakeLists.txt" src_cmake)
file(READ "${ANT_SOURCE_DIR}/examples/CMakeLists.txt" examples_cmake)
file(READ "${ANT_SOURCE_DIR}/examples/qt-ant-design-example.rc" example_rc)
file(READ "${ANT_SOURCE_DIR}/tests/CMakeLists.txt" tests_cmake)
file(READ "${ANT_SOURCE_DIR}/cmake/qt-ant-designConfig.cmake.in" package_config)

if(NOT EXISTS "${ANT_SOURCE_DIR}/VERSION")
    message(FATAL_ERROR "Missing root VERSION file")
endif()
file(STRINGS "${ANT_SOURCE_DIR}/VERSION" version_lines LIMIT_COUNT 1)
list(GET version_lines 0 release_version)
string(STRIP "${release_version}" release_version)
if(NOT release_version MATCHES "^[0-9]+\\.[0-9]+\\.[0-9]+$")
    message(FATAL_ERROR "VERSION must use MAJOR.MINOR.PATCH format")
endif()
if(NOT root_cmake MATCHES "file[ \t\r\n]*\\([ \t\r\n]*STRINGS[^\n]+VERSION")
    message(FATAL_ERROR "Top-level CMake must read the project version from VERSION")
endif()
if(NOT package_config MATCHES "QT_ANT_DESIGN_VERSION")
    message(FATAL_ERROR "Installed package config must expose QT_ANT_DESIGN_VERSION")
endif()

if(src_cmake MATCHES "add_library[ \t\r\n]*\\([ \t\r\n]*qt-ant-design[ \t\r\n]+STATIC")
    message(FATAL_ERROR "qt-ant-design must not hard-code STATIC; use BUILD_SHARED_LIBS / library type options")
endif()

foreach(content IN ITEMS "${root_cmake}" "${src_cmake}" "${tests_cmake}" "${package_config}")
    if(content MATCHES "find_package[ \t\r\n]*\\([ \t\r\n]*Qt6" OR content MATCHES "Qt6::" OR content MATCHES "Qt6_DIR")
        message(FATAL_ERROR "Build files must use detected Qt major version instead of hard-coded Qt6 references")
    endif()
endforeach()

if(NOT root_cmake MATCHES "find_package[ \t\r\n]*\\([ \t\r\n]*QT[ \t\r\n]+NAMES[ \t\r\n]+Qt6[ \t\r\n]+Qt5")
    message(FATAL_ERROR "Top-level CMake must detect Qt with find_package(QT NAMES Qt6 Qt5 ...)")
endif()

if(NOT package_config MATCHES "find_dependency[ \t\r\n]*\\([ \t\r\n]*Qt\\$\\{QT_ANT_DESIGN_QT_MAJOR_VERSION\\}")
    message(FATAL_ERROR "Installed package config must detect the same Qt major version used to build qt-ant-design")
endif()

if(src_cmake MATCHES "WINDOWS_EXPORT_ALL_SYMBOLS")
    message(FATAL_ERROR "Shared Windows builds must use explicit export macros instead of auto-exporting every symbol")
endif()

if(NOT src_cmake MATCHES "QT_ANT_DESIGN_LIBRARY" OR NOT src_cmake MATCHES "QT_ANT_DESIGN_STATIC_DEFINE")
    message(FATAL_ERROR "Static/shared builds must define the qt-ant-design export macro state")
endif()

if(examples_cmake MATCHES "MANIFESTINPUT")
    message(FATAL_ERROR "Example app must not pass /MANIFESTINPUT; embed the manifest as an RC resource to avoid mt.exe merge failures")
endif()

if(NOT example_rc MATCHES "1[ \t]+24[ \t]+\"qt-ant-design-example\\.exe\\.manifest\"")
    message(FATAL_ERROR "Example app must embed its Windows manifest as RT_MANIFEST resource id 1")
endif()

if(NOT EXISTS "${ANT_SOURCE_DIR}/src/core/QtAntDesignExport.h")
    message(FATAL_ERROR "Missing public export header for shared-library consumers")
endif()
