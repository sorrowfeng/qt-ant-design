cmake_minimum_required(VERSION 3.16)

project(qt-ant-design
    VERSION 0.1.0
    LANGUAGES CXX
)

include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

set(QT_ANT_DESIGN_HEADERS
    src/core/AntTypes.h
    src/core/AntTheme.h
    src/styles/AntButtonStyle.h
    src/styles/AntCardStyle.h
    src/styles/AntCheckboxStyle.h
    src/styles/AntDatePickerStyle.h
    src/styles/AntDescriptionsStyle.h
    src/styles/AntDividerStyle.h
    src/styles/AntDropdownStyle.h
    src/styles/AntBreadcrumbStyle.h
    src/styles/AntFormStyle.h
    src/styles/AntInputStyle.h
    src/styles/AntInputNumberStyle.h
    src/styles/AntListStyle.h
    src/styles/AntMenuStyle.h
    src/styles/AntPaginationStyle.h
    src/styles/AntPalette.h
    src/styles/AntRadioStyle.h
    src/styles/AntSelectStyle.h
    src/styles/AntSliderStyle.h
    src/styles/AntSpinStyle.h
    src/styles/AntStepsStyle.h
    src/styles/AntSwitchStyle.h
    src/styles/AntTabsStyle.h
    src/styles/AntTimePickerStyle.h
    src/styles/AntAlertStyle.h
    src/styles/AntAvatarStyle.h
    src/styles/AntBadgeStyle.h
    src/styles/AntEmptyStyle.h
    src/styles/AntIconStyle.h
    src/styles/AntMessageStyle.h
    src/styles/AntModalStyle.h
    src/styles/AntNotificationStyle.h
    src/styles/AntPopoverStyle.h
    src/styles/AntPopconfirmStyle.h
    src/styles/AntProgressStyle.h
    src/styles/AntResultStyle.h
    src/styles/AntSkeletonStyle.h
    src/styles/AntStatisticStyle.h
    src/styles/AntSpaceStyle.h
    src/styles/AntTagStyle.h
    src/styles/AntTimelineStyle.h
    src/styles/AntTooltipStyle.h
    src/styles/AntTypographyStyle.h
    src/styles/AntLayoutStyle.h
    src/widgets/AntAlert.h
    src/widgets/AntAvatar.h
    src/widgets/AntBadge.h
    src/widgets/AntBreadcrumb.h
    src/widgets/AntButton.h
    src/widgets/AntCheckbox.h
    src/widgets/AntDatePicker.h
    src/widgets/AntDescriptions.h
    src/widgets/AntDropdown.h
    src/widgets/AntDivider.h
    src/widgets/AntForm.h
    src/widgets/AntEmpty.h
    src/widgets/AntIcon.h
    src/widgets/AntInput.h
    src/widgets/AntInputNumber.h
    src/widgets/AntList.h
    src/widgets/AntMessage.h
    src/widgets/AntMenu.h
    src/widgets/AntNotification.h
    src/widgets/AntModal.h
    src/widgets/AntPopover.h
    src/widgets/AntPopconfirm.h
    src/widgets/AntPagination.h
    src/widgets/AntProgress.h
    src/widgets/AntRadio.h
    src/widgets/AntResult.h
    src/widgets/AntSelect.h
    src/widgets/AntSkeleton.h
    src/widgets/AntSlider.h
    src/widgets/AntSpin.h
    src/widgets/AntStatistic.h
    src/widgets/AntSteps.h
    src/widgets/AntSwitch.h
    src/widgets/AntTabs.h
    src/widgets/AntTag.h
    src/widgets/AntTooltip.h
    src/widgets/AntTimePicker.h
    src/widgets/AntTypography.h
    src/widgets/AntTimeline.h
    src/widgets/AntLayout.h
    src/widgets/AntCard.h
    src/widgets/AntSpace.h
)

set(QT_ANT_DESIGN_SOURCES
    src/core/AntTheme.cpp
    src/styles/AntButtonStyle.cpp
    src/styles/AntCardStyle.cpp
    src/styles/AntCheckboxStyle.cpp
    src/styles/AntDatePickerStyle.cpp
    src/styles/AntDescriptionsStyle.cpp
    src/styles/AntDividerStyle.cpp
    src/styles/AntDropdownStyle.cpp
    src/styles/AntBreadcrumbStyle.cpp
    src/styles/AntFormStyle.cpp
    src/styles/AntInputStyle.cpp
    src/styles/AntInputNumberStyle.cpp
    src/styles/AntListStyle.cpp
    src/styles/AntMenuStyle.cpp
    src/styles/AntPaginationStyle.cpp
    src/styles/AntPalette.cpp
    src/styles/AntRadioStyle.cpp
    src/styles/AntSelectStyle.cpp
    src/styles/AntSliderStyle.cpp
    src/styles/AntSpinStyle.cpp
    src/styles/AntStepsStyle.cpp
    src/styles/AntSwitchStyle.cpp
    src/styles/AntTabsStyle.cpp
    src/styles/AntTimePickerStyle.cpp
    src/styles/AntAvatarStyle.cpp
    src/styles/AntBadgeStyle.cpp
    src/styles/AntEmptyStyle.cpp
    src/styles/AntResultStyle.cpp
    src/styles/AntSkeletonStyle.cpp
    src/styles/AntStatisticStyle.cpp
    src/styles/AntSpaceStyle.cpp
    src/styles/AntTagStyle.cpp
    src/styles/AntTimelineStyle.cpp
    src/styles/AntLayoutStyle.cpp
    src/styles/AntAlertStyle.cpp
    src/styles/AntIconStyle.cpp
    src/styles/AntMessageStyle.cpp
    src/styles/AntModalStyle.cpp
    src/styles/AntNotificationStyle.cpp
    src/styles/AntPopoverStyle.cpp
    src/styles/AntPopconfirmStyle.cpp
    src/styles/AntProgressStyle.cpp
    src/styles/AntTooltipStyle.cpp
    src/styles/AntTypographyStyle.cpp
    src/widgets/AntAlert.cpp
    src/widgets/AntAvatar.cpp
    src/widgets/AntBadge.cpp
    src/widgets/AntBreadcrumb.cpp
    src/widgets/AntButton.cpp
    src/widgets/AntCheckbox.cpp
    src/widgets/AntDatePicker.cpp
    src/widgets/AntDescriptions.cpp
    src/widgets/AntDropdown.cpp
    src/widgets/AntDivider.cpp
    src/widgets/AntForm.cpp
    src/widgets/AntEmpty.cpp
    src/widgets/AntIcon.cpp
    src/widgets/AntInput.cpp
    src/widgets/AntList.cpp
    src/widgets/AntInputNumber.cpp
    src/widgets/AntMessage.cpp
    src/widgets/AntMenu.cpp
    src/widgets/AntNotification.cpp
    src/widgets/AntModal.cpp
    src/widgets/AntPopover.cpp
    src/widgets/AntPopconfirm.cpp
    src/widgets/AntPagination.cpp
    src/widgets/AntProgress.cpp
    src/widgets/AntRadio.cpp
    src/widgets/AntResult.cpp
    src/widgets/AntSelect.cpp
    src/widgets/AntSkeleton.cpp
    src/widgets/AntSlider.cpp
    src/widgets/AntSpin.cpp
    src/widgets/AntStatistic.cpp
    src/widgets/AntSteps.cpp
    src/widgets/AntSwitch.cpp
    src/widgets/AntTabs.cpp
    src/widgets/AntTag.cpp
    src/widgets/AntTooltip.cpp
    src/widgets/AntTimePicker.cpp
    src/widgets/AntTypography.cpp
    src/widgets/AntTimeline.cpp
    src/widgets/AntLayout.cpp
    src/widgets/AntCard.cpp
    src/widgets/AntSpace.cpp
    resources/qt-ant-design.qrc
)

add_library(qt-ant-design STATIC
    ${QT_ANT_DESIGN_HEADERS}
    ${QT_ANT_DESIGN_SOURCES}
)

target_include_directories(qt-ant-design
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/qt-ant-design>
)

target_link_libraries(qt-ant-design
    PUBLIC
        Qt6::Core
        Qt6::Widgets
)

add_executable(qt-ant-design-example
    examples/main.cpp
    examples/ExampleWindow.h
    examples/ExampleWindow.cpp
)

target_link_libraries(qt-ant-design-example
    PRIVATE
        qt-ant-design
)

install(TARGETS qt-ant-design qt-ant-design-example
    EXPORT qt-ant-design-targets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(FILES
    src/core/AntTypes.h
    src/core/AntTheme.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/qt-ant-design/core
)

install(FILES
    src/styles/AntButtonStyle.h
    src/styles/AntCheckboxStyle.h
    src/styles/AntInputStyle.h
    src/styles/AntInputNumberStyle.h
    src/styles/AntPalette.h
    src/styles/AntRadioStyle.h
    src/styles/AntSelectStyle.h
    src/styles/AntSliderStyle.h
    src/styles/AntSpaceStyle.h
    src/styles/AntSpinStyle.h
    src/styles/AntSwitchStyle.h
    src/styles/AntTypographyStyle.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/qt-ant-design/styles
)

install(FILES
    src/widgets/AntAlert.h
    src/widgets/AntAvatar.h
    src/widgets/AntBadge.h
    src/widgets/AntBreadcrumb.h
    src/widgets/AntButton.h
    src/widgets/AntCheckbox.h
    src/widgets/AntDatePicker.h
    src/widgets/AntDescriptions.h
    src/widgets/AntDropdown.h
    src/widgets/AntDivider.h
    src/widgets/AntForm.h
    src/widgets/AntEmpty.h
    src/widgets/AntIcon.h
    src/widgets/AntInput.h
    src/widgets/AntInputNumber.h
    src/widgets/AntList.h
    src/widgets/AntMessage.h
    src/widgets/AntMenu.h
    src/widgets/AntNotification.h
    src/widgets/AntModal.h
    src/widgets/AntPopover.h
    src/widgets/AntPopconfirm.h
    src/widgets/AntPagination.h
    src/widgets/AntProgress.h
    src/widgets/AntRadio.h
    src/widgets/AntResult.h
    src/widgets/AntSelect.h
    src/widgets/AntSkeleton.h
    src/widgets/AntSlider.h
    src/widgets/AntSpin.h
    src/widgets/AntStatistic.h
    src/widgets/AntSteps.h
    src/widgets/AntSwitch.h
    src/widgets/AntTabs.h
    src/widgets/AntTag.h
    src/widgets/AntTooltip.h
    src/widgets/AntTimePicker.h
    src/widgets/AntTypography.h
    src/widgets/AntCard.h
    src/widgets/AntSpace.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/qt-ant-design/widgets
)

install(EXPORT qt-ant-design-targets
    NAMESPACE qt-ant-design::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/qt-ant-design
)

if(WIN32)
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
        message(WARNING \"windeployqt was not found. Installed example may require Qt DLLs in PATH.\")
    endif()
endif()
