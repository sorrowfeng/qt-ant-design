# Changelog

All notable changes to `qt-ant-design` are documented here.

The project follows [Semantic Versioning](https://semver.org/): `MAJOR.MINOR.PATCH`.

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

