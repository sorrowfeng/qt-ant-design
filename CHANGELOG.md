# Changelog

All notable changes to `qt-ant-design` are documented here.

The project follows [Semantic Versioning](https://semver.org/): `MAJOR.MINOR.PATCH`.

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

