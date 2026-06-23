# Contributing

Thanks for helping improve `qt-ant-design`. The project aims to keep Qt5 and Qt6 support healthy while matching Ant Design's visual language in native Qt Widgets.

## Development Flow

1. Work from `dev` for normal changes. `main` is kept for releases.
2. Keep changes scoped to the widget, style, example, test, and documentation surface affected by the issue.
3. Follow the existing `src/widgets`, `src/styles`, `src/core`, and `examples/pages` organization.
4. Prefer `QProxyStyle` for new widget painting unless the existing component pattern clearly uses direct painting.
5. Update docs when public APIs, examples, screenshots, versioning, or generated site content changes.

## Build

Use a CMake preset when it matches your environment:

```powershell
cmake --preset windows-msvc-qt6-debug
cmake --build --preset qt6-debug
ctest --preset qt6-debug
```

For Qt5 on Windows, set `QT5_CMAKE_PREFIX_PATH` first if CMake would otherwise find Qt6:

```powershell
$env:QT5_CMAKE_PREFIX_PATH = "C:/Qt/5.15.2/msvc2019_64/lib/cmake"
cmake --preset windows-msvc-qt5-debug
cmake --build --preset qt5-debug
ctest --preset qt5-debug
```

## Tests

Run targeted tests for the changed controls first. Broaden to full CTest when shared behavior, windowing, theme, install, or Qt5/Qt6 compatibility is touched.

Useful focused commands:

```powershell
cmake --build build --config Debug --target qt-ant-design-example
ctest --test-dir build -C Debug -R "TestAntInput|TestAntQtExtensions" --output-on-failure
```

## Documentation Site

Development branches keep generators and source data. Generated HTML belongs under `build/` locally or in the `gh_page` branch.

```powershell
python tools/generate_component_api_overview.py
python tools/generate_gh_pages_site.py
```

## Pull Requests

- Describe the user-visible behavior change.
- Mention Qt5 / Qt6 coverage when relevant.
- Include screenshots for visual changes.
- Keep generated build outputs out of `main` and `dev`.
