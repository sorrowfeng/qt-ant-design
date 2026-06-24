set(CMAKE_AUTOMOC ON)

option(BUILD_WIDGET_SMOKE_TESTS "Build one target per public Ant widget smoke test" ON)

function(add_ant_test TEST_NAME)
    add_executable(${TEST_NAME} ${TEST_NAME}.cpp)
    target_link_libraries(${TEST_NAME} PRIVATE qt-ant-design Qt${QT_VERSION_MAJOR}::Test)
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endfunction()

function(add_ant_inventory_test TEST_NAME)
    add_ant_test(${TEST_NAME})
    target_compile_definitions(${TEST_NAME} PRIVATE
        ANT_WIDGETS_DIR="${CMAKE_SOURCE_DIR}/src/widgets"
        ANT_TESTS_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
    )
endfunction()

function(add_ant_widget_smoke_test TEST_TYPE TEST_HEADER TEST_CONSTRUCT)
    set(ANT_WIDGET_SMOKE_TYPE "${TEST_TYPE}")
    set(ANT_WIDGET_SMOKE_HEADER "${TEST_HEADER}")
    set(ANT_WIDGET_SMOKE_CONSTRUCT "${TEST_CONSTRUCT}")
    set(ANT_WIDGET_SMOKE_MOC "TestAntWidgetSmoke_${TEST_TYPE}.moc")

    set(TEST_SOURCE "${CMAKE_CURRENT_BINARY_DIR}/widget_smoke/TestAntWidgetSmoke_${TEST_TYPE}.cpp")
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/WidgetSmokeTest.cpp.in
        ${TEST_SOURCE}
        @ONLY
    )

    set(TEST_TARGET "TestAntWidgetSmoke_${TEST_TYPE}")
    add_executable(${TEST_TARGET} ${TEST_SOURCE})
    target_link_libraries(${TEST_TARGET} PRIVATE qt-ant-design Qt${QT_VERSION_MAJOR}::Test)
    add_test(NAME "WidgetSmoke.${TEST_TYPE}" COMMAND ${TEST_TARGET})
    set_tests_properties("WidgetSmoke.${TEST_TYPE}" PROPERTIES LABELS "widget-smoke;${TEST_TYPE}")
    set_property(GLOBAL APPEND PROPERTY ANT_WIDGET_SMOKE_TARGETS "${TEST_TARGET}")
endfunction()

if(BUILD_WIDGET_SMOKE_TESTS)
    set(ANT_WIDGET_SMOKE_TESTS
        "AntAffix|AntAffix.h|new AntAffix()"
        "AntAlert|AntAlert.h|new AntAlert()"
        "AntAnchor|AntAnchor.h|new AntAnchor()"
        "AntApp|AntApp.h|new AntApp(&rootWidget)"
        "AntAutoComplete|AntAutoComplete.h|new AntAutoComplete()"
        "AntAvatar|AntAvatar.h|new AntAvatar()"
        "AntAvatarGroup|AntAvatar.h|new AntAvatarGroup()"
        "AntBadge|AntBadge.h|new AntBadge()"
        "AntBreadcrumb|AntBreadcrumb.h|new AntBreadcrumb()"
        "AntButton|AntButton.h|new AntButton()"
        "AntCalendar|AntCalendar.h|new AntCalendar()"
        "AntCard|AntCard.h|new AntCard()"
        "AntCarousel|AntCarousel.h|new AntCarousel()"
        "AntCascader|AntCascader.h|new AntCascader()"
        "AntCheckBox|AntCheckBox.h|new AntCheckBox()"
        "AntCollapse|AntCollapse.h|new AntCollapse()"
        "AntCollapsePanel|AntCollapse.h|new AntCollapsePanel(QStringLiteral(\"Panel\"))"
        "AntColorPicker|AntColorPicker.h|new AntColorPicker()"
        "AntConfigProvider|AntConfigProvider.h|new AntConfigProvider()"
        "AntDatePicker|AntDatePicker.h|new AntDatePicker()"
        "AntDescriptions|AntDescriptions.h|new AntDescriptions()"
        "AntDescriptionsItem|AntDescriptions.h|new AntDescriptionsItem()"
        "AntDialog|AntDialog.h|new AntDialog()"
        "AntDivider|AntDivider.h|new AntDivider()"
        "AntDockManager|AntDockManager.h|new AntDockManager()"
        "AntDockWidget|AntDockWidget.h|new AntDockWidget()"
        "AntDrawer|AntDrawer.h|new AntDrawer()"
        "AntDropdown|AntDropdown.h|new AntDropdown()"
        "AntEmpty|AntEmpty.h|new AntEmpty()"
        "AntFileDialog|AntFileDialog.h|new AntFileDialog()"
        "AntFlex|AntFlex.h|new AntFlex()"
        "AntFloatButton|AntFloatButton.h|new AntFloatButton()"
        "AntForm|AntForm.h|new AntForm()"
        "AntFormItem|AntForm.h|new AntFormItem()"
        "AntFormList|AntForm.h|new AntFormList()"
        "AntFormProvider|AntForm.h|new AntFormProvider()"
        "AntCol|AntGrid.h|new AntCol()"
        "AntRow|AntGrid.h|new AntRow()"
        "AntIcon|AntIcon.h|new AntIcon()"
        "AntImage|AntImage.h|new AntImage()"
        "AntInput|AntInput.h|new AntInput()"
        "AntInputDialog|AntInputDialog.h|new AntInputDialog()"
        "AntInputNumber|AntInputNumber.h|new AntInputNumber()"
        "AntLayout|AntLayout.h|new AntLayout()"
        "AntLayoutContent|AntLayout.h|new AntLayoutContent()"
        "AntLayoutFooter|AntLayout.h|new AntLayoutFooter()"
        "AntLayoutHeader|AntLayout.h|new AntLayoutHeader()"
        "AntLayoutSider|AntLayout.h|new AntLayoutSider()"
        "AntList|AntList.h|new AntList()"
        "AntListItem|AntList.h|new AntListItem()"
        "AntListItemMeta|AntList.h|new AntListItemMeta()"
        "AntLog|AntLog.h|new AntLog()"
        "AntMasonry|AntMasonry.h|new AntMasonry()"
        "AntMentions|AntMentions.h|new AntMentions()"
        "AntMenu|AntMenu.h|new AntMenu()"
        "AntMenuBar|AntMenuBar.h|new AntMenuBar()"
        "AntMessage|AntMessage.h|new AntMessage()"
        "AntModal|AntModal.h|new AntModal()"
        "AntNav|AntNav.h|new AntNav()"
        "AntNavItem|AntNavItem.h|new AntNavItem(QStringLiteral(\"Item\"))"
        "AntNotification|AntNotification.h|new AntNotification()"
        "AntPagination|AntPagination.h|new AntPagination()"
        "AntPlainTextEdit|AntPlainTextEdit.h|new AntPlainTextEdit()"
        "AntPopconfirm|AntPopconfirm.h|new AntPopconfirm()"
        "AntPopover|AntPopover.h|new AntPopover()"
        "AntProgress|AntProgress.h|new AntProgress()"
        "AntQRCode|AntQRCode.h|new AntQRCode()"
        "AntRadio|AntRadio.h|new AntRadio()"
        "AntRate|AntRate.h|new AntRate()"
        "AntResult|AntResult.h|new AntResult()"
        "AntRibbon|AntRibbon.h|new AntRibbon()"
        "AntRibbonGroup|AntRibbon.h|new AntRibbonGroup()"
        "AntRibbonPage|AntRibbon.h|new AntRibbonPage()"
        "AntScrollArea|AntScrollArea.h|new AntScrollArea()"
        "AntScrollBar|AntScrollBar.h|new AntScrollBar()"
        "AntSegmented|AntSegmented.h|new AntSegmented()"
        "AntSelect|AntSelect.h|new AntSelect()"
        "AntSkeleton|AntSkeleton.h|new AntSkeleton()"
        "AntSlider|AntSlider.h|new AntSlider()"
        "AntSpace|AntSpace.h|new AntSpace()"
        "AntSpin|AntSpin.h|new AntSpin()"
        "AntSplitter|AntSplitter.h|new AntSplitter()"
        "AntStackedWidget|AntStackedWidget.h|new AntStackedWidget()"
        "AntStatistic|AntStatistic.h|new AntStatistic()"
        "AntStatusBar|AntStatusBar.h|new AntStatusBar()"
        "AntSteps|AntSteps.h|new AntSteps()"
        "AntSwitch|AntSwitch.h|new AntSwitch()"
        "AntTable|AntTable.h|new AntTable()"
        "AntTabs|AntTabs.h|new AntTabs()"
        "AntTag|AntTag.h|new AntTag()"
        "AntTimeline|AntTimeline.h|new AntTimeline()"
        "AntTimePicker|AntTimePicker.h|new AntTimePicker()"
        "AntToolBar|AntToolBar.h|new AntToolBar()"
        "AntToolButton|AntToolButton.h|new AntToolButton()"
        "AntToolTip|AntToolTip.h|new AntToolTip()"
        "AntTour|AntTour.h|new AntTour()"
        "AntTransfer|AntTransfer.h|new AntTransfer()"
        "AntTree|AntTree.h|new AntTree()"
        "AntTreeSelect|AntTreeSelect.h|new AntTreeSelect()"
        "AntTypography|AntTypography.h|new AntTypography()"
        "AntUpload|AntUpload.h|new AntUpload()"
        "AntWatermark|AntWatermark.h|new AntWatermark()"
        "AntWidget|AntWidget.h|new AntWidget()"
        "AntWindow|AntWindow.h|new AntWindow()"
    )

    foreach(ANT_WIDGET_SMOKE_ENTRY IN LISTS ANT_WIDGET_SMOKE_TESTS)
        string(REPLACE "|" ";" ANT_WIDGET_SMOKE_FIELDS "${ANT_WIDGET_SMOKE_ENTRY}")
        list(GET ANT_WIDGET_SMOKE_FIELDS 0 ANT_WIDGET_SMOKE_TYPE)
        list(GET ANT_WIDGET_SMOKE_FIELDS 1 ANT_WIDGET_SMOKE_HEADER)
        list(GET ANT_WIDGET_SMOKE_FIELDS 2 ANT_WIDGET_SMOKE_CONSTRUCT)
        add_ant_widget_smoke_test(
            "${ANT_WIDGET_SMOKE_TYPE}"
            "${ANT_WIDGET_SMOKE_HEADER}"
            "${ANT_WIDGET_SMOKE_CONSTRUCT}"
        )
    endforeach()

    get_property(ANT_WIDGET_SMOKE_TARGETS GLOBAL PROPERTY ANT_WIDGET_SMOKE_TARGETS)
    add_custom_target(TestAntWidgetSmoke_All DEPENDS ${ANT_WIDGET_SMOKE_TARGETS})
endif()

# Core types
add_ant_test(TestAntTypes)

# General (4 widgets)
add_ant_test(TestAntButton)
add_ant_test(TestAntIcon)
add_ant_test(TestAntTypography)
add_ant_test(TestAntFloatButton)

# Data Entry A (6 widgets)
add_ant_test(TestAntCheckBox)
add_ant_test(TestAntSwitch)
add_ant_test(TestAntSelect)
add_ant_test(TestAntInput)
add_ant_test(TestAntDataEntryA)

# Data Entry B (7 widgets)
add_ant_test(TestAntDataEntryB)

# Data Display A (8 widgets)
add_ant_test(TestAntBadge)
add_ant_test(TestAntDataDisplayA)

# Data Display B (10 widgets)
add_ant_test(TestAntDataDisplayB)
add_ant_test(TestAntTag)

# Feedback (13 widgets)
add_ant_test(TestAntFeedback)
add_ant_test(TestAntModal)

# Navigation (7 widgets)
add_ant_test(TestAntNavigation)

# Layout (7 widgets)
add_ant_test(TestAntLayout)

# Qt Extensions (19 widgets)
add_ant_test(TestAntQtExtensions)

# Qt-style widget aliases
add_ant_test(TestAntAliases)

# Object tree ownership
add_ant_test(TestAntObjectTree)

# Assigned child ownership
add_ant_test(TestAntChildOwnership)

# Theme switching lifecycle
add_ant_test(TestAntThemeLifecycle)

# Meta-property coverage
add_ant_test(TestAntMetaProperties)

# Complex interaction coverage
add_ant_test(TestAntInteractions)
add_ant_test(TestAntAdvancedInteractions)

# Motion and animation coverage
add_ant_test(TestAntMotion)

# Popup lifecycle coverage
add_ant_test(TestAntPopupLifecycle)

# Repeated lifecycle stress coverage
add_ant_test(TestAntStressLifecycle)

# Render smoke coverage
add_ant_test(TestAntRenderSmoke)

# Automated visual regression coverage
add_ant_test(TestAntVisualRegression)
add_ant_inventory_test(TestAntQtVersionVisualParity)
foreach(ANT_VISUAL_PARITY_SCALE IN ITEMS 1.25 1.5)
    string(REPLACE "." "_" ANT_VISUAL_PARITY_SCALE_NAME "${ANT_VISUAL_PARITY_SCALE}")
    add_test(NAME TestAntQtVersionVisualParity_Scale_${ANT_VISUAL_PARITY_SCALE_NAME}
        COMMAND $<TARGET_FILE:TestAntQtVersionVisualParity>
    )
    set_tests_properties(TestAntQtVersionVisualParity_Scale_${ANT_VISUAL_PARITY_SCALE_NAME}
        PROPERTIES
            ENVIRONMENT "QT_SCALE_FACTOR=${ANT_VISUAL_PARITY_SCALE};QT_ENABLE_HIGHDPI_SCALING=1"
            TIMEOUT 45
    )
endforeach()
add_ant_test(TestAntQtVersionMetricAudit)

# Windows / Qt high-DPI logical geometry coverage
add_executable(TestAntHighDpiScaling TestAntHighDpiScaling.cpp)
target_link_libraries(TestAntHighDpiScaling PRIVATE qt-ant-design Qt${QT_VERSION_MAJOR}::Test)
foreach(ANT_HIGH_DPI_SCALE IN ITEMS 1.0 1.25 1.5)
    string(REPLACE "." "_" ANT_HIGH_DPI_SCALE_NAME "${ANT_HIGH_DPI_SCALE}")
    add_test(NAME TestAntHighDpiScaling_${ANT_HIGH_DPI_SCALE_NAME}
        COMMAND $<TARGET_FILE:TestAntHighDpiScaling>
    )
    set_tests_properties(TestAntHighDpiScaling_${ANT_HIGH_DPI_SCALE_NAME}
        PROPERTIES ENVIRONMENT "QT_SCALE_FACTOR=${ANT_HIGH_DPI_SCALE};QT_ENABLE_HIGHDPI_SCALING=1"
    )
endforeach()
add_test(NAME TestAntHighDpiScaling_InitializeDefault
    COMMAND $<TARGET_FILE:TestAntHighDpiScaling>
)
set_tests_properties(TestAntHighDpiScaling_InitializeDefault
    PROPERTIES ENVIRONMENT "ANT_TEST_INITIALIZE_CONFIGURES_HIGHDPI=1;QT_SCALE_FACTOR=1.25;QT_ENABLE_HIGHDPI_SCALING=1"
)

# Installed package consumer coverage
add_test(NAME TestAntInstallConsumer
    COMMAND ${CMAKE_COMMAND}
        -DANT_BINARY_DIR=${CMAKE_BINARY_DIR}
        -DANT_TEST_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}
        -DANT_CONFIG=$<CONFIG>
        -DANT_QT_PACKAGE=${QT_ANT_DESIGN_QT_PACKAGE}
        -DANT_QT_PACKAGE_DIR=${${QT_ANT_DESIGN_QT_PACKAGE}_DIR}
        -DANT_CMAKE_GENERATOR=${CMAKE_GENERATOR}
        -DANT_CMAKE_GENERATOR_PLATFORM=${CMAKE_GENERATOR_PLATFORM}
        -DANT_CMAKE_GENERATOR_TOOLSET=${CMAKE_GENERATOR_TOOLSET}
        -DANT_CMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/TestAntInstallConsumer.cmake
)

# Build-system option coverage
add_test(NAME TestAntBuildSystem
    COMMAND ${CMAKE_COMMAND}
        -DANT_SOURCE_DIR=${CMAKE_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/TestAntBuildSystem.cmake
)

# No-QSS / no-QStyleSheet styling guard
add_test(NAME TestAntNoStyleSheetUsage
    COMMAND ${CMAKE_COMMAND}
        -DANT_SOURCE_DIR=${CMAKE_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/TestAntNoStyleSheetUsage.cmake
)

if(TARGET qt-ant-design-example)
    if(WIN32)
        add_test(NAME TestAntExampleGuiSubsystem
            COMMAND ${CMAKE_COMMAND}
                -DEXAMPLE_EXE=$<TARGET_FILE:qt-ant-design-example>
                -P ${CMAKE_CURRENT_SOURCE_DIR}/TestAntExampleGuiSubsystem.cmake
        )
    endif()

    add_test(NAME TestAntExampleCloseStress
        COMMAND $<TARGET_FILE:qt-ant-design-example>
            --stress-theme-cycles 16
            --stress-theme-interval-ms 20
            --smoke-exit-ms 800
    )
    set_tests_properties(TestAntExampleCloseStress PROPERTIES TIMEOUT 10)

    add_test(NAME TestAntExamplePageTraversal
        COMMAND $<TARGET_FILE:qt-ant-design-example>
            --smoke-traverse-pages
            --smoke-traverse-step-ms 1
    )
    set_tests_properties(TestAntExamplePageTraversal PROPERTIES TIMEOUT 45)
endif()

# Coverage inventory guard
add_ant_inventory_test(TestAntCoverageInventory)
