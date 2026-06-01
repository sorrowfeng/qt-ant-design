# Component Reliability Coverage

Last verified: `2026-06-01`

This inventory tracks reliability coverage for every public `Ant*` widget header, excluding Qt-style alias headers and the internal non-installed `AntSelectPopup` helper.

Coverage dimensions:

- `Behavior/API tests`: component-specific tests that exercise public API, signals, user interaction, popup behavior, motion, stress paths, or visual regression guards.
- `Lifecycle`: covered by `TestAntObjectTree`.
- `Meta`: covered by `TestAntMetaProperties`.
- `Theme`: covered by `TestAntThemeLifecycle`.
- `Render`: covered by `TestAntRenderSmoke`.
- `Qt version visual parity`: representative Qt5/Qt6 visual atlas export and comparison is covered by `TestAntQtVersionVisualParity`; the current atlas contains 26 scenes spanning basic controls, interaction states, a consolidated acceptance-state matrix, top-level popup surfaces, advanced data entry, navigation, feedback, rich data display, scroll/carousel, structural layout, desktop, dock, file-dialog, and window surfaces. The test also guards public-header atlas coverage, with only utility/popup-controller headers explicitly deferred from static scene rendering. The real example app also has a 352-frame Qt5-vs-Qt6 traversal/export comparison path. Details are in `docs/qt5-visual-adaptation-report.md`.
- `Qt version metric audit`: `TestAntQtVersionMetricAudit` exports and compares Qt default `QStyle` metrics, default draw fingerprints, palette roles, font metrics, complex-control geometry, and adapted Ant widget metrics. Qt default rows are audit-only root-cause evidence; adapted Ant rows are strict comparison rows, including representative success-standard controls such as Menu, Pagination, Tag, Badge, Table, and ToolTip.
- `Windows High DPI scaling`: startup policy and logical render geometry at 1.0, 1.25, and 1.5 scale factors are covered by `TestAntHighDpiScaling_1_0`, `TestAntHighDpiScaling_1_25`, and `TestAntHighDpiScaling_1_5`; the full visual atlas is also smoke-rendered at 1.25 and 1.5 by `TestAntQtVersionVisualParity_Scale_1_25` and `TestAntQtVersionVisualParity_Scale_1_5`.
- `No QSS/QStyleSheet guard`: `TestAntNoStyleSheetUsage` scans `src`, `examples`, `tests`, and `resources` and fails on `setStyleSheet()`, `QStyleSheet`, `styleSheet:` in UI files, or committed `.qss` files.

The guard `TestAntCoverageInventory::publicWidgetHeadersAreInBehaviorReliabilityTests()` fails if any public component is missing from component-specific behavior/API coverage. `publicWidgetHeadersAreInCoverageTests()` also fails if any public component is missing from lifecycle, meta-property, theme-lifecycle, or render-smoke coverage.

Latest validation:

```powershell
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
cmake --build build --config Debug --target qt-ant-design-example
```

Result on `2026-05-30`: `37 / 37` CTest targets passed and `qt-ant-design-example` Debug build succeeded. Current configured CTest entry count is `46` after adding `TestAntQtVersionVisualParity`, two `TestAntQtVersionVisualParity_Scale` entries, `TestAntQtVersionMetricAudit`, three `TestAntHighDpiScaling` scale-factor entries, `TestAntNoStyleSheetUsage`, and `TestAntExamplePageTraversal`; the latest combined adaptation-focused run passed `9 / 9` entries in both Qt6 and Qt5 build trees on `2026-06-01`. Default CTest uses Qt event-level interaction checks; Win32 `SendInput` desktop-input checks in `TestAntQtExtensions` are opt-in via `QT_ANT_DESIGN_ENABLE_NATIVE_INPUT_TESTS=1`.

| Component | Behavior/API tests | Lifecycle | Meta | Theme | Render |
| --- | --- | --- | --- | --- | --- |
| `AntAffix` | `TestAntLayout` | Yes | Yes | Yes | Yes |
| `AntAlert` | `TestAntChildOwnership`, `TestAntFeedback`, `TestAntVisualRegression` | Yes | Yes | Yes | Yes |
| `AntAnchor` | `TestAntNavigation` | Yes | Yes | Yes | Yes |
| `AntApp` | `TestAntQtExtensions` | Yes | Yes | Yes | Yes |
| `AntAutoComplete` | `TestAntDataEntryA` | Yes | Yes | Yes | Yes |
| `AntAvatar` | `TestAntDataDisplayA` | Yes | Yes | Yes | Yes |
| `AntBadge` | `TestAntBadge`, `TestAntChildOwnership`, `TestAntVisualRegression` | Yes | Yes | Yes | Yes |
| `AntBreadcrumb` | `TestAntNavigation` | Yes | Yes | Yes | Yes |
| `AntButton` | `TestAntAliases`, `TestAntButton`, `TestAntFeedback`, `TestAntStressLifecycle`, `TestAntVisualRegression` | Yes | Yes | Yes | Yes |
| `AntCalendar` | `TestAntAliases`, `TestAntDataDisplayA` | Yes | Yes | Yes | Yes |
| `AntCard` | `TestAntChildOwnership`, `TestAntDataDisplayA`, `TestAntVisualRegression` | Yes | Yes | Yes | Yes |
| `AntCarousel` | `TestAntChildOwnership`, `TestAntDataDisplayB` | Yes | Yes | Yes | Yes |
| `AntCascader` | `TestAntDataEntryB`, `TestAntInteractions`, `TestAntPopupLifecycle`, `TestAntStressLifecycle` | Yes | Yes | Yes | Yes |
| `AntCheckBox` | `TestAntAliases`, `TestAntCheckBox`, `TestAntVisualRegression` | Yes | Yes | Yes | Yes |
| `AntCollapse` | `TestAntChildOwnership`, `TestAntDataDisplayB` | Yes | Yes | Yes | Yes |
| `AntColorPicker` | `TestAntAdvancedInteractions`, `TestAntPopupLifecycle`, `TestAntQtExtensions`, `TestAntStressLifecycle`, `TestAntVisualRegression` | Yes | Yes | Yes | Yes |
| `AntConfigProvider` | `TestAntQtExtensions` | Yes | Yes | Yes | Yes |
| `AntDatePicker` | `TestAntAliases`, `TestAntDataEntryB`, `TestAntInteractions`, `TestAntPopupLifecycle`, `TestAntStressLifecycle` | Yes | Yes | Yes | Yes |
| `AntDescriptions` | `TestAntChildOwnership`, `TestAntDataDisplayB`, `TestAntVisualRegression` | Yes | Yes | Yes | Yes |
| `AntDialog` | `TestAntChildOwnership`, `TestAntQtExtensions` | Yes | Yes | Yes | Yes |
| `AntDivider` | `TestAntLayout` | Yes | Yes | Yes | Yes |
| `AntDockManager` | `TestAntQtExtensions` | Yes | Yes | Yes | Yes |
| `AntDockWidget` | `TestAntChildOwnership`, `TestAntQtExtensions` | Yes | Yes | Yes | Yes |
| `AntDrawer` | `TestAntChildOwnership`, `TestAntFeedback`, `TestAntVisualRegression` | Yes | Yes | Yes | Yes |
| `AntDropdown` | `TestAntInteractions`, `TestAntNavigation`, `TestAntPopupLifecycle`, `TestAntStressLifecycle` | Yes | Yes | Yes | Yes |
| `AntEmpty` | `TestAntChildOwnership`, `TestAntDataDisplayA` | Yes | Yes | Yes | Yes |
| `AntFlex` | `TestAntChildOwnership`, `TestAntLayout` | Yes | Yes | Yes | Yes |
| `AntFloatButton` | `TestAntFloatButton` | Yes | Yes | Yes | Yes |
| `AntFileDialog` | `TestAntQtExtensions` | Yes | Yes | Yes | Yes |
| `AntForm` | `TestAntChildOwnership`, `TestAntQtExtensions` | Yes | Yes | Yes | Yes |
| `AntGrid` | `TestAntChildOwnership`, `TestAntLayout` | Yes | Yes | Yes | Yes |
| `AntIcon` | `TestAntIcon` | Yes | Yes | Yes | Yes |
| `AntImage` | `TestAntDataDisplayA` | Yes | Yes | Yes | Yes |
| `AntInput` | `TestAntAliases`, `TestAntChildOwnership`, `TestAntInput`, `TestAntStressLifecycle` | Yes | Yes | Yes | Yes |
| `AntInputNumber` | `TestAntAliases`, `TestAntDataEntryA`, `TestAntInteractions`, `TestAntStressLifecycle`, `TestAntVisualRegression` | Yes | Yes | Yes | Yes |
| `AntLayout` | `TestAntChildOwnership`, `TestAntLayout`, `TestAntVisualRegression` | Yes | Yes | Yes | Yes |
| `AntList` | `TestAntAliases`, `TestAntChildOwnership`, `TestAntDataDisplayB`, `TestAntVisualRegression` | Yes | Yes | Yes | Yes |
| `AntLog` | `TestAntQtExtensions` | Yes | Yes | Yes | Yes |
| `AntMasonry` | `TestAntChildOwnership`, `TestAntLayout`, `TestAntQtExtensions` | Yes | Yes | Yes | Yes |
| `AntMentions` | `TestAntDataEntryB` | Yes | Yes | Yes | Yes |
| `AntMenu` | `TestAntInteractions`, `TestAntNavigation`, `TestAntVisualRegression` | Yes | Yes | Yes | Yes |
| `AntMenuBar` | `TestAntQtExtensions` | Yes | Yes | Yes | Yes |
| `AntMessage` | `TestAntAdvancedInteractions`, `TestAntFeedback`, `TestAntStressLifecycle`, `TestAntVisualRegression` | Yes | Yes | Yes | Yes |
| `AntModal` | `TestAntAliases`, `TestAntChildOwnership`, `TestAntModal`, `TestAntVisualRegression` | Yes | Yes | Yes | Yes |
| `AntNav` | `TestAntQtExtensions` | Yes | Yes | Yes | Yes |
| `AntNavItem` | `TestAntVisualRegression` | Yes | Yes | Yes | Yes |
| `AntNotification` | `TestAntAdvancedInteractions`, `TestAntFeedback`, `TestAntStressLifecycle`, `TestAntVisualRegression` | Yes | Yes | Yes | Yes |
| `AntPagination` | `TestAntNavigation`, `TestAntVisualRegression` | Yes | Yes | Yes | Yes |
| `AntPlainTextEdit` | `TestAntQtExtensions` | Yes | Yes | Yes | Yes |
| `AntPopconfirm` | `TestAntFeedback`, `TestAntPopupLifecycle` | Yes | Yes | Yes | Yes |
| `AntPopover` | `TestAntChildOwnership`, `TestAntFeedback`, `TestAntPopupLifecycle`, `TestAntVisualRegression` | Yes | Yes | Yes | Yes |
| `AntProgress` | `TestAntAliases`, `TestAntFeedback`, `TestAntVisualRegression` | Yes | Yes | Yes | Yes |
| `AntQRCode` | `TestAntDataDisplayB` | Yes | Yes | Yes | Yes |
| `AntRadio` | `TestAntAliases`, `TestAntDataEntryA`, `TestAntMotion`, `TestAntVisualRegression` | Yes | Yes | Yes | Yes |
| `AntRate` | `TestAntDataEntryA` | Yes | Yes | Yes | Yes |
| `AntResult` | `TestAntChildOwnership`, `TestAntFeedback`, `TestAntVisualRegression` | Yes | Yes | Yes | Yes |
| `AntRibbon` | `TestAntQtExtensions` | Yes | Yes | Yes | Yes |
| `AntScrollArea` | `TestAntChildOwnership`, `TestAntQtExtensions` | Yes | Yes | Yes | Yes |
| `AntScrollBar` | `TestAntQtExtensions` | Yes | Yes | Yes | Yes |
| `AntSegmented` | `TestAntDataEntryA`, `TestAntAdvancedInteractions` | Yes | Yes | Yes | Yes |
| `AntSelect` | `TestAntAliases`, `TestAntInteractions`, `TestAntPopupLifecycle`, `TestAntSelect`, `TestAntStressLifecycle` | Yes | Yes | Yes | Yes |
| `AntSkeleton` | `TestAntChildOwnership`, `TestAntFeedback` | Yes | Yes | Yes | Yes |
| `AntSlider` | `TestAntDataEntryA` | Yes | Yes | Yes | Yes |
| `AntSpace` | `TestAntChildOwnership`, `TestAntLayout` | Yes | Yes | Yes | Yes |
| `AntSpin` | `TestAntFeedback` | Yes | Yes | Yes | Yes |
| `AntSplitter` | `TestAntChildOwnership`, `TestAntQtExtensions` | Yes | Yes | Yes | Yes |
| `AntStackedWidget` | `TestAntChildOwnership`, `TestAntQtExtensions` | Yes | Yes | Yes | Yes |
| `AntStatistic` | `TestAntChildOwnership`, `TestAntDataDisplayA` | Yes | Yes | Yes | Yes |
| `AntStatusBar` | `TestAntQtExtensions` | Yes | Yes | Yes | Yes |
| `AntSteps` | `TestAntNavigation`, `TestAntVisualRegression` | Yes | Yes | Yes | Yes |
| `AntSwitch` | `TestAntStressLifecycle`, `TestAntSwitch`, `TestAntVisualRegression` | Yes | Yes | Yes | Yes |
| `AntTable` | `TestAntAdvancedInteractions`, `TestAntAliases`, `TestAntDataDisplayB`, `TestAntVisualRegression` | Yes | Yes | Yes | Yes |
| `AntTabs` | `TestAntAdvancedInteractions`, `TestAntAliases`, `TestAntChildOwnership`, `TestAntNavigation`, `TestAntStressLifecycle`, `TestAntVisualRegression` | Yes | Yes | Yes | Yes |
| `AntTag` | `TestAntTag`, `TestAntVisualRegression` | Yes | Yes | Yes | Yes |
| `AntTimeline` | `TestAntDataDisplayB` | Yes | Yes | Yes | Yes |
| `AntTimePicker` | `TestAntAliases`, `TestAntDataEntryB`, `TestAntInteractions`, `TestAntPopupLifecycle`, `TestAntStressLifecycle` | Yes | Yes | Yes | Yes |
| `AntToolBar` | `TestAntQtExtensions` | Yes | Yes | Yes | Yes |
| `AntToolButton` | `TestAntQtExtensions` | Yes | Yes | Yes | Yes |
| `AntToolTip` | `TestAntAdvancedInteractions`, `TestAntAliases`, `TestAntFeedback`, `TestAntPopupLifecycle` | Yes | Yes | Yes | Yes |
| `AntTour` | `TestAntFeedback` | Yes | Yes | Yes | Yes |
| `AntTransfer` | `TestAntDataEntryB` | Yes | Yes | Yes | Yes |
| `AntTree` | `TestAntAdvancedInteractions`, `TestAntAliases`, `TestAntDataDisplayB` | Yes | Yes | Yes | Yes |
| `AntTreeSelect` | `TestAntDataEntryB`, `TestAntPopupLifecycle`, `TestAntStressLifecycle` | Yes | Yes | Yes | Yes |
| `AntTypography` | `TestAntAliases`, `TestAntTypography` | Yes | Yes | Yes | Yes |
| `AntUpload` | `TestAntDataEntryB`, `TestAntInteractions` | Yes | Yes | Yes | Yes |
| `AntWatermark` | `TestAntAdvancedInteractions`, `TestAntDataDisplayB` | Yes | Yes | Yes | Yes |
| `AntWidget` | `TestAntQtExtensions` | Yes | Yes | Yes | Yes |
| `AntWindow` | `TestAntAliases`, `TestAntChildOwnership`, `TestAntQtExtensions` | Yes | Yes | Yes | Yes |
