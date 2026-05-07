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

find_package(${ANT_QT_PACKAGE} REQUIRED COMPONENTS Core Widgets Svg)
find_package(qt-ant-design CONFIG REQUIRED)

add_executable(qt-ant-design-consumer main.cpp)
target_link_libraries(qt-ant-design-consumer PRIVATE qt-ant-design::qt-ant-design)
]=])

file(WRITE "${consumer_source_dir}/main.cpp" [=[
#include <QApplication>
#include <QStringList>
#include <type_traits>

#include "widgets/AntCalendarWidget.h"
#include "widgets/AntButton.h"
#include "widgets/AntCheckBox.h"
#include "widgets/AntComboBox.h"
#include "widgets/AntDateEdit.h"
#include "widgets/AntDialog.h"
#include "widgets/AntDoubleSpinBox.h"
#include "widgets/AntIcon.h"
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

static_assert(std::is_same_v<AntCalendarWidget, AntCalendar>);
static_assert(std::is_same_v<AntComboBox, AntSelect>);
static_assert(std::is_same_v<AntDateEdit, AntDatePicker>);
static_assert(std::is_same_v<AntDialog, AntModal>);
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

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    AntButton button(QStringLiteral("Install Consumer"));
    button.setButtonType(Ant::ButtonType::Primary);

    AntIcon icon;
    icon.setIconType(Ant::IconType::Search);

    AntLabel label(QStringLiteral("Alias Label"));
    AntPushButton aliasButton(QStringLiteral("Alias Button"));

    return button.text() == QStringLiteral("Install Consumer") &&
           label.text() == QStringLiteral("Alias Label") &&
           aliasButton.text() == QStringLiteral("Alias Button") ? 0 : 1;
}
]=])

set(configure_args
    "${CMAKE_COMMAND}"
    -S "${consumer_source_dir}"
    -B "${consumer_build_dir}"
    "-DCMAKE_PREFIX_PATH=${prefix_dir}"
    "-DANT_QT_PACKAGE=${ANT_QT_PACKAGE}"
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
