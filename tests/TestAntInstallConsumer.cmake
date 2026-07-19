if(NOT DEFINED ANT_BINARY_DIR)
    message(FATAL_ERROR "ANT_BINARY_DIR is required")
endif()
if(NOT DEFINED ANT_TEST_BINARY_DIR)
    message(FATAL_ERROR "ANT_TEST_BINARY_DIR is required")
endif()
if(NOT DEFINED ANT_CONFIG OR ANT_CONFIG STREQUAL "")
    set(ANT_CONFIG Debug)
endif()
if(NOT DEFINED ANT_QT_PACKAGE OR ANT_QT_PACKAGE STREQUAL "")
    set(ANT_QT_PACKAGE Qt6)
endif()
if(NOT DEFINED ANT_QT_PACKAGE_DIR)
    set(ANT_QT_PACKAGE_DIR "")
endif()
if(NOT DEFINED ANT_EXPECTED_VERSION OR
   NOT ANT_EXPECTED_VERSION MATCHES "^[0-9]+\\.[0-9]+\\.[0-9]+$")
    message(FATAL_ERROR "ANT_EXPECTED_VERSION must use MAJOR.MINOR.PATCH format")
endif()

string(REPLACE "." ";" expected_version_parts "${ANT_EXPECTED_VERSION}")
list(GET expected_version_parts 0 ANT_EXPECTED_VERSION_MAJOR)
list(GET expected_version_parts 1 ANT_EXPECTED_VERSION_MINOR)
list(GET expected_version_parts 2 ANT_EXPECTED_VERSION_PATCH)

set(work_dir "${ANT_TEST_BINARY_DIR}/install-consumer")
set(prefix_dir "${work_dir}/prefix")
set(consumer_source_dir "${work_dir}/consumer-src")
set(consumer_build_dir "${work_dir}/consumer-build")

file(REMOVE_RECURSE "${work_dir}")
file(MAKE_DIRECTORY "${consumer_source_dir}")

execute_process(
    COMMAND "${CMAKE_COMMAND}" --install "${ANT_BINARY_DIR}" --config "${ANT_CONFIG}" --prefix "${prefix_dir}"
    RESULT_VARIABLE install_result
    OUTPUT_VARIABLE install_output
    ERROR_VARIABLE install_error
)
if(NOT install_result EQUAL 0)
    message(FATAL_ERROR "Install failed:\n${install_output}\n${install_error}")
endif()

file(WRITE "${consumer_source_dir}/CMakeLists.txt" [=[
cmake_minimum_required(VERSION 3.16)
project(qt_ant_design_consumer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT DEFINED ANT_EXPECTED_VERSION)
    message(FATAL_ERROR "ANT_EXPECTED_VERSION is required")
endif()

find_package(${ANT_QT_PACKAGE} REQUIRED COMPONENTS Core Widgets Svg)
find_package(qt-ant-design ${ANT_EXPECTED_VERSION} EXACT CONFIG REQUIRED)
if(NOT QT_ANT_DESIGN_VERSION VERSION_EQUAL ANT_EXPECTED_VERSION)
    message(FATAL_ERROR "Installed package reports ${QT_ANT_DESIGN_VERSION}, expected ${ANT_EXPECTED_VERSION}")
endif()

add_executable(qt-ant-design-consumer main.cpp)
target_link_libraries(qt-ant-design-consumer PRIVATE qt-ant-design::qt-ant-design)
]=])

set(consumer_main [=[
#include <QApplication>
#include <QStringList>
#include <type_traits>

#include "core/AntDesign.h"
#include "core/QtAntDesignVersion.h"
#include "widgets/AntCalendarWidget.h"
#include "widgets/AntButton.h"
#include "widgets/AntCheckBox.h"
#include "widgets/AntComboBox.h"
#include "widgets/AntDateEdit.h"
#include "widgets/AntDialog.h"
#include "widgets/AntDoubleSpinBox.h"
#include "widgets/AntIcon.h"
#include "widgets/AntInputDialog.h"
#include "widgets/AntLabel.h"
#include "widgets/AntLineEdit.h"
#include "widgets/AntListView.h"
#include "widgets/AntListWidget.h"
#include "widgets/AntMainWindow.h"
#include "widgets/AntProgressBar.h"
#include "widgets/AntPushButton.h"
#include "widgets/AntRadioButton.h"
#include "widgets/AntSpinBox.h"
#include "widgets/AntTabWidget.h"
#include "widgets/AntTableView.h"
#include "widgets/AntTableWidget.h"
#include "widgets/AntTimeEdit.h"
#include "widgets/AntToolTip.h"
#include "widgets/AntTreeView.h"
#include "widgets/AntTreeWidget.h"
#include "widgets/AntWindowFrame.h"

static_assert(std::is_same_v<AntCalendarWidget, AntCalendar>);
static_assert(std::is_same_v<AntComboBox, AntSelect>);
static_assert(std::is_same_v<AntDateEdit, AntDatePicker>);
static_assert(std::is_same_v<AntDoubleSpinBox, AntInputNumber>);
static_assert(std::is_same_v<AntLabel, AntTypography>);
static_assert(std::is_same_v<AntLineEdit, AntInput>);
static_assert(std::is_same_v<AntListView, AntList>);
static_assert(std::is_same_v<AntListWidget, AntList>);
static_assert(std::is_same_v<AntMainWindow, AntWindow>);
static_assert(std::is_same_v<AntProgressBar, AntProgress>);
static_assert(std::is_same_v<AntPushButton, AntButton>);
static_assert(std::is_same_v<AntRadioButton, AntRadio>);
static_assert(std::is_same_v<AntSpinBox, AntInputNumber>);
static_assert(std::is_same_v<AntTabWidget, AntTabs>);
static_assert(std::is_same_v<AntTableView, AntTable>);
static_assert(std::is_same_v<AntTableWidget, AntTable>);
static_assert(std::is_same_v<AntTimeEdit, AntTimePicker>);
static_assert(std::is_same_v<AntTreeView, AntTree>);
static_assert(std::is_same_v<AntTreeWidget, AntTree>);
static_assert(QT_ANT_DESIGN_VERSION_MAJOR == @ANT_EXPECTED_VERSION_MAJOR@);
static_assert(QT_ANT_DESIGN_VERSION_MINOR == @ANT_EXPECTED_VERSION_MINOR@);
static_assert(QT_ANT_DESIGN_VERSION_PATCH == @ANT_EXPECTED_VERSION_PATCH@);
static_assert(QtAntDesign::versionMajor() == QT_ANT_DESIGN_VERSION_MAJOR);
static_assert(!std::is_copy_constructible_v<AntWindowFrame::LegacySoftwareShadowHandle>);
static_assert(!std::is_move_constructible_v<AntWindowFrame::LegacySoftwareShadowHandle>);
static_assert(std::is_trivially_destructible_v<AntWindowFrame::LegacySoftwareShadowHandle>);
static_assert(sizeof(AntWindowFrame::LegacySoftwareShadowHandle) == sizeof(QWidget*));

int main(int argc, char** argv)
{
    AntDesign::configureHighDpi();

    QApplication app(argc, argv);
    AntDesign::initialize(&app);

    AntButton button(QStringLiteral("Install Consumer"));
    button.setButtonType(Ant::ButtonType::Primary);

    AntIcon icon;
    icon.setIconType(Ant::IconType::Search);
    const bool resourcesAvailable = AntIcon::builtinIconNames().contains(QStringLiteral("GithubFilled"));

    AntLabel label(QStringLiteral("Alias Label"));
    AntPushButton aliasButton(QStringLiteral("Alias Button"));
    AntDialog dialog;
    dialog.setWindowTitle(QStringLiteral("Install Consumer Dialog"));
    AntInputDialog inputDialog;
    inputDialog.setLabelText(QStringLiteral("Name"));
    inputDialog.setTextValue(QStringLiteral("qt-ant-design"));
    AntWindowFrame::LegacySoftwareShadowHandle shadowHandle;
    QWidget shadowOwner;
    AntWindowFrame::LegacySoftwareShadowOptions shadowOptions;
    shadowOptions.enabledProperty = QByteArrayLiteral("installConsumerShadowEnabled");
    shadowOptions.enabled = false;
    const auto shadowUpdateResult =
        AntWindowFrame::updateLegacySoftwareShadow(&shadowOwner, shadowHandle, shadowOptions);
    const auto shadowHideResult = AntWindowFrame::hideLegacySoftwareShadow(
        &shadowOwner,
        shadowHandle,
        QByteArrayLiteral("installConsumerShadowEnabled"),
        QByteArray());

    return button.text() == QStringLiteral("Install Consumer") &&
           resourcesAvailable &&
           label.text() == QStringLiteral("Alias Label") &&
           aliasButton.text() == QStringLiteral("Alias Button") &&
           dialog.contentWidget() != nullptr &&
           inputDialog.textValue() == QStringLiteral("qt-ant-design") &&
           shadowHandle.isNull() &&
           shadowUpdateResult == AntWindowFrame::LegacySoftwareShadowResult::Hidden &&
           shadowHideResult == AntWindowFrame::LegacySoftwareShadowResult::Hidden &&
           QString::fromLatin1(QT_ANT_DESIGN_VERSION_STR) == QStringLiteral("@ANT_EXPECTED_VERSION@") &&
           QString::fromLatin1(QtAntDesign::versionString()) == QStringLiteral("@ANT_EXPECTED_VERSION@") ? 0 : 1;
}
]=])
string(CONFIGURE "${consumer_main}" consumer_main @ONLY)
file(WRITE "${consumer_source_dir}/main.cpp" "${consumer_main}")

set(configure_args
    "${CMAKE_COMMAND}"
    -S "${consumer_source_dir}"
    -B "${consumer_build_dir}"
    "-DCMAKE_PREFIX_PATH=${prefix_dir}"
    "-DANT_QT_PACKAGE=${ANT_QT_PACKAGE}"
    "-DANT_EXPECTED_VERSION=${ANT_EXPECTED_VERSION}"
)

if(NOT ANT_QT_PACKAGE_DIR STREQUAL "")
    list(APPEND configure_args "-D${ANT_QT_PACKAGE}_DIR=${ANT_QT_PACKAGE_DIR}")
endif()

if(DEFINED ANT_CMAKE_GENERATOR AND NOT ANT_CMAKE_GENERATOR STREQUAL "")
    list(APPEND configure_args -G "${ANT_CMAKE_GENERATOR}")
endif()
if(DEFINED ANT_CMAKE_GENERATOR_PLATFORM AND NOT ANT_CMAKE_GENERATOR_PLATFORM STREQUAL "")
    list(APPEND configure_args -A "${ANT_CMAKE_GENERATOR_PLATFORM}")
endif()
if(DEFINED ANT_CMAKE_GENERATOR_TOOLSET AND NOT ANT_CMAKE_GENERATOR_TOOLSET STREQUAL "")
    list(APPEND configure_args -T "${ANT_CMAKE_GENERATOR_TOOLSET}")
endif()
if(DEFINED ANT_CMAKE_MAKE_PROGRAM AND NOT ANT_CMAKE_MAKE_PROGRAM STREQUAL "")
    list(APPEND configure_args "-DCMAKE_MAKE_PROGRAM=${ANT_CMAKE_MAKE_PROGRAM}")
endif()
if(NOT ANT_CONFIG STREQUAL "")
    list(APPEND configure_args "-DCMAKE_BUILD_TYPE=${ANT_CONFIG}")
endif()

execute_process(
    COMMAND ${configure_args}
    RESULT_VARIABLE configure_result
    OUTPUT_VARIABLE configure_output
    ERROR_VARIABLE configure_error
)
if(NOT configure_result EQUAL 0)
    message(FATAL_ERROR "Consumer configure failed:\n${configure_output}\n${configure_error}")
endif()

execute_process(
    COMMAND "${CMAKE_COMMAND}" --build "${consumer_build_dir}" --config "${ANT_CONFIG}"
    RESULT_VARIABLE build_result
    OUTPUT_VARIABLE build_output
    ERROR_VARIABLE build_error
)
if(NOT build_result EQUAL 0)
    message(FATAL_ERROR "Consumer build failed:\n${build_output}\n${build_error}")
endif()

set(consumer_executable_candidates
    "${consumer_build_dir}/qt-ant-design-consumer"
    "${consumer_build_dir}/qt-ant-design-consumer.exe"
    "${consumer_build_dir}/${ANT_CONFIG}/qt-ant-design-consumer"
    "${consumer_build_dir}/${ANT_CONFIG}/qt-ant-design-consumer.exe"
)
set(consumer_executable "")
foreach(candidate IN LISTS consumer_executable_candidates)
    if(EXISTS "${candidate}")
        set(consumer_executable "${candidate}")
        break()
    endif()
endforeach()
if(consumer_executable STREQUAL "")
    message(FATAL_ERROR "Built consumer executable was not found under ${consumer_build_dir}")
endif()

if(WIN32)
    if(ANT_QT_PACKAGE_DIR STREQUAL "")
        message(FATAL_ERROR "ANT_QT_PACKAGE_DIR is required to run the Windows consumer headlessly")
    endif()
    get_filename_component(qt_prefix_dir "${ANT_QT_PACKAGE_DIR}/../../.." ABSOLUTE)
    set(qt_bin_dir "${qt_prefix_dir}/bin")
    set(qt_plugins_dir "${qt_prefix_dir}/plugins")
    if(NOT EXISTS "${qt_bin_dir}" OR NOT EXISTS "${qt_plugins_dir}/platforms")
        message(FATAL_ERROR "Unable to resolve the Qt runtime from ${ANT_QT_PACKAGE_DIR}")
    endif()
    execute_process(
        COMMAND "${CMAKE_COMMAND}" -E env
                "PATH=${qt_bin_dir};${prefix_dir}/bin;$ENV{PATH}"
                "QT_PLUGIN_PATH=${qt_plugins_dir}"
                "QT_QPA_PLATFORM=minimal"
                "${consumer_executable}"
        RESULT_VARIABLE run_result
        OUTPUT_VARIABLE run_output
        ERROR_VARIABLE run_error
    )
else()
    set(consumer_run_environment "PATH=${prefix_dir}/bin:$ENV{PATH}")
    if(UNIX AND NOT APPLE AND "$ENV{DISPLAY}" STREQUAL "")
        list(APPEND consumer_run_environment "QT_QPA_PLATFORM=offscreen")
    endif()
    execute_process(
        COMMAND "${CMAKE_COMMAND}" -E env ${consumer_run_environment} "${consumer_executable}"
        RESULT_VARIABLE run_result
        OUTPUT_VARIABLE run_output
        ERROR_VARIABLE run_error
    )
endif()
if(NOT run_result EQUAL 0)
    message(FATAL_ERROR "Consumer run failed with exit code ${run_result}:\n${run_output}\n${run_error}")
endif()
