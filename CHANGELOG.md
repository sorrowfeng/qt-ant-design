# Changelog

All notable changes to `qt-ant-design` are documented here.

The project follows [Semantic Versioning](https://semver.org/): `MAJOR.MINOR.PATCH`.

## [0.1.3] - 2026-08-29

### Added

- Deep behavioral test coverage for all public widgets: the historically smoke-level widgets (`AntToolBar`, `AntFileDialog`, `AntAutoComplete`, `AntBreadcrumb`, `AntMenuBar`, `AntMentions`, `AntConfigProvider`, `AntRibbon`) now assert signals, properties, and interaction flows in `TestAntDeepCoverage`, and all 104 widget smoke tests are guarded by a 30-second timeout.
- Qt meta-object value-type registration for public option structs, enabling `QVariant` / `QMetaType` interoperability.
- Status bar item update APIs with richer status bar states.
- Linux sanitizer CI matrix coverage with QTest diagnostics embedded in CTest logs.

### Changed

- Centralized dark-mode detection in `AntTheme::isDarkMode()` as the single entry point, replacing 38 inline `themeMode()` checks.
- Migrated 57 per-widget style event filters into the shared `AntStyleBase` paint-filter pattern, hoisting button-family shadow/focus helpers, input-family focus glow, and empty-illustration drawing into the shared base.
- Unified placement enums into the `Ant::Placement` superset and prefixed size setters into the canonical `setSize(Ant::Size)`.
- Converged CheckBox-family state-change signals, selection value API naming, and font setup into `AntStyleBase::withPixelSize`.
- `AntResult` now exposes its own `ResultStatus` enum instead of an ad-hoc status type.
- Style colors, radii, and spinner rendering are now derived from theme tokens; spinner timer and drawing are unified in `AntSpinner`.
- `AntInput::setError` is deprecated in favor of the registered variant property API.

### Fixed

- Corrected `Q_PROPERTY` NOTIFY bindings in Slider, Progress, and Typography.
- Synchronized the disabled state with `QWidget::setEnabled` in four widgets.
- Fixed the legacy window frame and enhanced status bar states.
- Stabilized the cascader geometry cache assertion and Qt5 destruction tracking under hosted CI, and improved GCC/Linux test build portability.

### Performance

- Cached official SVG icon rendering through `QPixmapCache` and stopped the `AntIcon` spin timer while the icon is hidden.

### Docs

- Archived stale project documentation, added porting guidelines and API gap audit references, and refreshed README project banners.

## [0.1.2] - 2026-06-03

### Fixed

- Fixed Windows example builds with Qt6 / Visual Studio / newer Windows SDKs by removing the manual `/MANIFESTINPUT` linker manifest merge path that could trigger `mt.exe : general error`.
- Embedded the example Windows compatibility and PerMonitorV2 DPI manifest as an `.rc` `RT_MANIFEST` resource instead, while disabling MSVC auto manifest generation for the example target.
- Added a build-system guard to prevent `/MANIFESTINPUT` from being reintroduced for the example application.

## [0.1.1] - 2026-06-02

### Added

- Added manually controlled `AntNotification` progress support for download/task scenarios through `ProgressMode::Manual`, `progress`, `setProgress()`, and `progress()` APIs.
- Added a `Download Progress` notification example that demonstrates updating progress and switching to a success notification when complete.
- Added project logo resources to the example application sidebar, runtime window icon, and Windows `.exe` icon.

### Changed

- `AntDesign::initialize()` now performs early High DPI pre-configuration when called before `QApplication`, simplifying Qt5 startup setup for consumers.
- Updated feedback and project-status documentation for Notification countdown/manual progress behavior.

### Fixed

- Fixed Qt5 input-number frame rendering in `AntInputNumber` / `AntInputDialog` integer and double modes.
- Fixed Showcase modal preview button clipping by sizing the footer from the actual `AntButton` height.

## [0.1.0] - 2026-06-02

### Added

- First public release of the Qt Widgets Ant Design component library.
- Qt 5.15.2 and Qt 6 build support through CMake auto-detection.
- 89 public Ant-style widgets, 19 Qt-style alias headers, and full example coverage.
- 831 bundled official Ant Design SVG icons with `AntIcon` string-name APIs.
- Installable CMake package config, exported targets, headers, examples, and Windows deployment support.
- Local and GitHub Pages component/API documentation.
- Version management through the root `VERSION` file, CMake `PROJECT_VERSION`, generated `QtAntDesignVersion.h`, git tags, and this changelog.

### Fixed

- Qt5 numeric editor frame rendering in `AntInputNumber`, including `AntInputDialog` integer and double input modes.

