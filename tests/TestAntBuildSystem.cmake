if(NOT DEFINED ANT_SOURCE_DIR)
    message(FATAL_ERROR "ANT_SOURCE_DIR is required")
endif()
if(NOT DEFINED ANT_TEST_BINARY_DIR)
    message(FATAL_ERROR "ANT_TEST_BINARY_DIR is required")
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

# Keep the explicit source manifest complete without reintroducing configure-time
# GLOB discovery into the production build.
file(GLOB_RECURSE disk_source_candidates
    RELATIVE "${ANT_SOURCE_DIR}/src"
    "${ANT_SOURCE_DIR}/src/*.h"
    "${ANT_SOURCE_DIR}/src/*.cpp"
)
list(SORT disk_source_candidates)
string(REGEX MATCHALL "[A-Za-z0-9_./-]+\\.(h|cpp)" manifest_candidates "${src_cmake}")
set(manifest_sources)
foreach(candidate IN LISTS manifest_candidates)
    if(EXISTS "${ANT_SOURCE_DIR}/src/${candidate}")
        list(APPEND manifest_sources "${candidate}")
    endif()
endforeach()
list(REMOVE_DUPLICATES manifest_sources)
list(SORT manifest_sources)
set(unlisted_sources ${disk_source_candidates})
set(missing_sources ${manifest_sources})
foreach(candidate IN LISTS manifest_sources)
    list(REMOVE_ITEM unlisted_sources "${candidate}")
endforeach()
foreach(candidate IN LISTS disk_source_candidates)
    list(REMOVE_ITEM missing_sources "${candidate}")
endforeach()
if(unlisted_sources OR missing_sources)
    message(FATAL_ERROR
        "src/CMakeLists.txt explicit manifest is out of sync. "
        "Unlisted files: ${unlisted_sources}; missing files: ${missing_sources}"
    )
endif()

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
if(NOT package_config MATCHES "QT_ANT_DESIGN_MIN_QT_VERSION")
    message(FATAL_ERROR "Installed package config must expose and enforce the minimum Qt version")
endif()

foreach(option_name IN ITEMS
        QT_ANT_DESIGN_BUILD_EXAMPLES
        QT_ANT_DESIGN_BUILD_TESTS
        QT_ANT_DESIGN_DEPLOY_EXAMPLE
        QT_ANT_DESIGN_ENABLE_ADDRESS_SANITIZER
        QT_ANT_DESIGN_ENABLE_UNDEFINED_SANITIZER)
    if(NOT root_cmake MATCHES "option[ \\t\\r\\n]*\\([ \\t\\r\\n]*${option_name}")
        message(FATAL_ERROR "Missing project-scoped build option: ${option_name}")
    endif()
endforeach()
if(NOT src_cmake MATCHES "MSVC AND QT_ANT_DESIGN_ENABLE_ADDRESS_SANITIZER" OR
   NOT src_cmake MATCHES "RTC\[1csu\]")
    message(FATAL_ERROR "MSVC Debug AddressSanitizer must remove the incompatible /RTC flags")
endif()
if(NOT root_cmake MATCHES "PROJECT_IS_TOP_LEVEL")
    message(FATAL_ERROR "Example and test defaults must depend on PROJECT_IS_TOP_LEVEL")
endif()
if(root_cmake MATCHES "if[ \\t\\r\\n]*\\([ \\t\\r\\n]*BUILD_(EXAMPLES|TESTS)[ \\t\\r\\n]*\\)")
    message(FATAL_ERROR "Build logic must not consume generic host BUILD_EXAMPLES/BUILD_TESTS options")
endif()
if(tests_cmake MATCHES "CMAKE_SOURCE_DIR")
    message(FATAL_ERROR "Tests must use PROJECT_SOURCE_DIR/CMAKE_CURRENT_SOURCE_DIR when embedded")
endif()
if(NOT root_cmake MATCHES "6\\.5\\.0" OR NOT root_cmake MATCHES "5\\.15\\.2")
    message(FATAL_ERROR "Source configuration must enforce Qt 6.5+ and Qt 5.15.2+")
endif()
if(NOT root_cmake MATCHES "REQUIRED[ \\t\\r\\n]+COMPONENTS[ \\t\\r\\n]+Test")
    message(FATAL_ERROR "Explicitly enabled tests must require Qt Test instead of silently skipping")
endif()
if(NOT root_cmake MATCHES "windeployqt failed with exit code" OR
   NOT root_cmake MATCHES "message\\(FATAL_ERROR")
    message(FATAL_ERROR "Example deployment failures must be fatal")
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
if(src_cmake MATCHES "target_compile_options[ \t\r\n]*\\([ \t\r\n]*qt-ant-design[ \t\r\n]+PUBLIC")
    message(FATAL_ERROR "Sanitizer compile instrumentation must not leak into arbitrary parent targets")
endif()
if(NOT src_cmake MATCHES "_DISABLE_STRING_ANNOTATION" OR
   NOT src_cmake MATCHES "_DISABLE_VECTOR_ANNOTATION")
    message(FATAL_ERROR "MSVC instrumented libraries must remain link-compatible with uninstrumented parents")
endif()

function(run_checked description)
    execute_process(
        COMMAND ${ARGN}
        RESULT_VARIABLE command_result
        OUTPUT_VARIABLE command_stdout
        ERROR_VARIABLE command_stderr
    )
    if(NOT command_result EQUAL 0)
        message(FATAL_ERROR
            "${description} failed with exit code ${command_result}\n"
            "stdout:\n${command_stdout}\n"
            "stderr:\n${command_stderr}"
        )
    endif()
endfunction()

file(TO_CMAKE_PATH "${ANT_SOURCE_DIR}" ant_source_cmake)
set(subproject_root "${ANT_TEST_BINARY_DIR}/subproject-integration")
file(REMOVE_RECURSE "${subproject_root}")
file(MAKE_DIRECTORY "${subproject_root}/parent")

file(WRITE "${subproject_root}/parent/CMakeLists.txt" [=[
cmake_minimum_required(VERSION 3.16)
project(AntParentIntegration LANGUAGES CXX)
enable_testing()
option(ANT_EXPLICIT_CHILD_FEATURES "Enable qt-ant-design examples and tests" OFF)
if(ANT_EXPLICIT_CHILD_FEATURES)
    set(QT_ANT_DESIGN_BUILD_EXAMPLES ON CACHE BOOL "" FORCE)
    set(QT_ANT_DESIGN_BUILD_TESTS ON CACHE BOOL "" FORCE)
    set(QT_ANT_DESIGN_BUILD_WIDGET_SMOKE_TESTS OFF CACHE BOOL "" FORCE)
endif()
add_subdirectory("@ANT_SOURCE@" qt-ant-design)
if(NOT TARGET qt-ant-design)
    message(FATAL_ERROR "Embedded qt-ant-design library target is missing")
endif()
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/parent_consumer.cpp" [==[
#include "core/AntUrlPolicy.h"
int main()
{
    return AntUrlPolicy::allowedSchemes().isEmpty() ? 1 : 0;
}
]==])
add_executable(AntParentConsumer "${CMAKE_CURRENT_BINARY_DIR}/parent_consumer.cpp")
target_link_libraries(AntParentConsumer PRIVATE qt-ant-design)
if(ANT_EXPLICIT_CHILD_FEATURES)
    if(NOT TARGET qt-ant-design-example OR NOT TARGET TestAntTypes)
        message(FATAL_ERROR "Explicit embedded build must add the requested example and tests")
    endif()
elseif(TARGET qt-ant-design-example OR TARGET TestAntTypes)
    message(FATAL_ERROR "Default embedded build must not add examples or tests")
endif()
]=])
file(READ "${subproject_root}/parent/CMakeLists.txt" parent_contents)
string(REPLACE "@ANT_SOURCE@" "${ant_source_cmake}" parent_contents "${parent_contents}")
file(WRITE "${subproject_root}/parent/CMakeLists.txt" "${parent_contents}")

set(generator_args)
if(DEFINED ANT_CMAKE_GENERATOR AND NOT ANT_CMAKE_GENERATOR STREQUAL "")
    list(APPEND generator_args -G "${ANT_CMAKE_GENERATOR}")
endif()
if(DEFINED ANT_CMAKE_GENERATOR_PLATFORM AND NOT ANT_CMAKE_GENERATOR_PLATFORM STREQUAL "")
    list(APPEND generator_args -A "${ANT_CMAKE_GENERATOR_PLATFORM}")
endif()
if(DEFINED ANT_CMAKE_GENERATOR_TOOLSET AND NOT ANT_CMAKE_GENERATOR_TOOLSET STREQUAL "")
    list(APPEND generator_args -T "${ANT_CMAKE_GENERATOR_TOOLSET}")
endif()
set(toolchain_args)
if(DEFINED ANT_CMAKE_MAKE_PROGRAM AND NOT ANT_CMAKE_MAKE_PROGRAM STREQUAL "")
    list(APPEND toolchain_args "-DCMAKE_MAKE_PROGRAM=${ANT_CMAKE_MAKE_PROGRAM}")
endif()
if(DEFINED ANT_QT_PACKAGE AND NOT ANT_QT_PACKAGE STREQUAL "" AND
   DEFINED ANT_QT_PACKAGE_DIR AND NOT ANT_QT_PACKAGE_DIR STREQUAL "")
    list(APPEND toolchain_args "-D${ANT_QT_PACKAGE}_DIR=${ANT_QT_PACKAGE_DIR}")
endif()
set(build_config_args)
if(DEFINED ANT_CONFIG AND NOT ANT_CONFIG STREQUAL "")
    list(APPEND build_config_args --config "${ANT_CONFIG}")
endif()

run_checked("default parent configure"
    "${CMAKE_COMMAND}" -S "${subproject_root}/parent"
    -B "${subproject_root}/build" ${generator_args} ${toolchain_args})

run_checked("default parent consumer build"
    "${CMAKE_COMMAND}" --build "${subproject_root}/build"
    --target AntParentConsumer ${build_config_args} --parallel 2)

run_checked("explicit parent configure"
    "${CMAKE_COMMAND}" -S "${subproject_root}/parent"
    -B "${subproject_root}/build" ${generator_args} ${toolchain_args}
    -DANT_EXPLICIT_CHILD_FEATURES=ON)

run_checked("explicit parent TestAntTypes build"
    "${CMAKE_COMMAND}" --build "${subproject_root}/build"
    --target TestAntTypes ${build_config_args} --parallel 2)

set(ctest_config_args)
if(DEFINED ANT_CONFIG AND NOT ANT_CONFIG STREQUAL "")
    list(APPEND ctest_config_args -C "${ANT_CONFIG}")
endif()
execute_process(
    COMMAND "${CMAKE_CTEST_COMMAND}" --test-dir "${subproject_root}/build" ${ctest_config_args} -N
    RESULT_VARIABLE discovery_result
    OUTPUT_VARIABLE discovery_output
    ERROR_VARIABLE discovery_error
)
if(NOT discovery_result EQUAL 0 OR NOT discovery_output MATCHES "TestAntTypes")
    message(FATAL_ERROR
        "Parent build directory did not discover TestAntTypes after enable_testing().\n"
        "stdout:\n${discovery_output}\n"
        "stderr:\n${discovery_error}"
    )
endif()
