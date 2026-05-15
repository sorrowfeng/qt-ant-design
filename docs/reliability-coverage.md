# Component Reliability Coverage

Last verified: `2026-05-15`

This inventory tracks reliability coverage for every public `Ant*` widget header, excluding Qt-style alias headers and the internal `AntSelectPopup` helper.

Coverage dimensions:

- `Behavior/API tests`: component-specific tests that exercise public API, signals, user interaction, popup behavior, motion, stress paths, or visual regression guards.
- `Lifecycle`: covered by `TestAntObjectTree`.
- `Meta`: covered by `TestAntMetaProperties`.
- `Theme`: covered by `TestAntThemeLifecycle`.
- `Render`: covered by `TestAntRenderSmoke`.

The guard `TestAntCoverageInventory::publicWidgetHeadersAreInBehaviorReliabilityTests()` fails if any public component is missing from component-specific behavior/API coverage. `publicWidgetHeadersAreInCoverageTests()` also fails if any public component is missing from lifecycle, meta-property, theme-lifecycle, or render-smoke coverage.

Latest validation:

```powershell
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
cmake --build build --config Debug --target qt-ant-design-example
```

Result on `2026-05-10`: `37 / 37` CTest targets passed and `qt-ant-design-example` Debug build succeeded. Latest targeted passive-overlay hit-test validation on `2026-05-15`: `TestAntAdvancedInteractions`, `TestAntDataEntryA`, `TestAntDataDisplayB`, `TestAntFeedback`, and `TestAntPopupLifecycle` passed, with the example Debug build and smoke launch succeeding.

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
| `AntDivider` | `TestAntLayout` | Yes | Yes | Yes | Yes |
| `AntDockWidget` | `TestAntChildOwnership`, `TestAntQtExtensions` | Yes | Yes | Yes | Yes |
| `AntDrawer` | `TestAntChildOwnership`, `TestAntFeedback`, `TestAntVisualRegression` | Yes | Yes | Yes | Yes |
| `AntDropdown` | `TestAntInteractions`, `TestAntNavigation`, `TestAntPopupLifecycle`, `TestAntStressLifecycle` | Yes | Yes | Yes | Yes |
| `AntEmpty` | `TestAntChildOwnership`, `TestAntDataDisplayA` | Yes | Yes | Yes | Yes |
| `AntFlex` | `TestAntChildOwnership`, `TestAntLayout` | Yes | Yes | Yes | Yes |
| `AntFloatButton` | `TestAntFloatButton` | Yes | Yes | Yes | Yes |
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
