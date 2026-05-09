# qt-ant-design

English | [简体中文](README.zh-CN.md)

`qt-ant-design` is a C++ component library built on Qt Widgets that auto-detects Qt 6 or Qt 5 at configure time and ports the Ant Design system to native desktop widgets.

The project focuses on:

- Dynamic light / dark theme switching
- Faithful reproduction of Ant Design's interactions and state styles
- A maintainable desktop rendering stack built on `QPainter` / `QProxyStyle`

> Current implementation and visual-audit status are tracked in [docs/project-status.md](docs/project-status.md) and [docs/visual-audit.md](docs/visual-audit.md).

> Issues and PRs are welcome: visual mismatches, missing interactions, Qt integration problems, documentation gaps, component fixes, tests, examples, and docs improvements are all appreciated.

## Showcase

| Light | Dark |
| --- | --- |
| ![Qt showcase light](resources/images/showcase-light.png) | ![Qt showcase dark](resources/images/showcase-dark.png) |

## Features

- Built on Qt Widgets — lightweight, easy to embed, and consumable as either a static or shared library in existing projects
- Built-in Design Token system with real-time light / dark theme switching
- `83` public components ported so far (full coverage of Ant Design's `70 / 70` standard components, plus `13` Qt / desktop extension components)
- `62` style-driven components are rendered through a `QProxyStyle` architecture
- The example app currently demos `83 / 83` public components, plus a standalone Ant Design homepage-style `Showcase`
- `AntIcon` bundles `831` official SVG resources from `@ant-design/icons-svg@4.4.2`
- Comparable standard components are tracked as visual-audit `Pass`; Qt-only desktop extensions are tracked as `Local Pass`
- Clean code structure — `core / styles / widgets / examples` layering keeps the project easy to extend

## Current Status

- Status snapshot: [docs/project-status.md](docs/project-status.md)
- Visual audit matrix: [docs/visual-audit.md](docs/visual-audit.md)
- Official icon inventory: [docs/ant-design-icons.md](docs/ant-design-icons.md)
- Current CTest target count: `37`; latest targeted AntWindow verification: `1 / 1` passed on `2026-05-09`

## Recent Ant Design Parity Updates

The 2026-04-30 interaction and motion pass tightened several user-visible details:

- Popup feedback: `AntPopover`, `AntMessage`, and `AntNotification` now have more stable hover/close behavior, stronger elevation, and placement-aware enter/exit motion.
- Popup shells: shared popup elevation now uses a softer multi-layer feather outside the panel body, so `AntDropdown`, `AntMenu`, selector popups, `AntColorPicker`, `AntDatePicker`, `AntTimePicker`, and `AntModal` keep AntD-like shadows in light and dark themes.
- Motion: `AntCarousel`, `AntTabs`, `AntSkeleton`, `AntSpin`, `AntInputNumber`, `AntSwitch`, and loading buttons now better match Ant Design timing, direction, and state feedback.
- Data interaction: `AntTransfer` now supports scrolling and header select-all correctly, while `AntTable` sorter clicks reorder rows instead of only changing the icon state.
- Input feedback: `AntPlainTextEdit` supports TextArea-style bottom-right resizing, and `AntSlider` shows a value bubble while dragging.

## Recent Desktop Window Updates

The 2026-05-07 `AntWindow` pass improved native desktop behavior and title-bar polish:

- Windows 11 Snap support for frameless windows: resize edges/corners, draggable title bar, maximize-button Snap Layout hover, edge snapping, and drag-to-restore.
- DWM-backed rounded corners, border/shadow integration, and a `cornerRadius` API for Windows builds while keeping platform-specific code behind Qt/Win32 guards.
- Windows 10 uses a no-caption native frame path plus a legacy rounded mask and a transparent software shadow host, so maximized/restored windows do not expose native title buttons and normal windows keep a light Win11-like four-sided outer shadow that starts at the window edge with cleaner rounded corners before and after resize.
- Topmost toggles on visible Windows windows now use native `SetWindowPos` in place, avoiding the hide/show cycle that caused a visible flash.
- Title-bar pin and light/dark theme buttons use bundled official Ant Design icons, and every title-bar button can be shown or hidden through public APIs.
- The built-in theme button uses a captured-frame overlay with a soft reveal animation so full-window light/dark switching feels continuous.

## Recent Qt API Compatibility Updates

The 2026-05-07 API pass improves use with Qt object trees and familiar Qt widget conventions:

- `AntInput`, `AntInputNumber`, `AntCheckBox`, `AntRadio`, `AntSlider`, `AntProgress`, and `AntStatusBar` expose more common Qt-style methods and signals; `AntInputNumber` defaults to integer display and enables QDoubleSpinBox-style decimal value/range/step behavior through `setDecimals()` or `setPrecision()`.
- `AntSelect` supports QComboBox-style item management, default first-item selection when an empty select receives data, plus option-style helpers such as `setOptionText`, `removeOption`, `optionData`, `findData`, `currentData`, `activated`, and highlighted signals.
- Qt layout behavior now follows native widget baselines: line-edit-like controls expand horizontally, combo-like selectors stay `Preferred/Fixed`, spin/date/time editors stay `Minimum/Fixed`, view widgets expand in both directions, and `AntTypography` mirrors `QLabel` height-for-width word wrapping.
- `AntTabs` clears default root layout margins on added content pages so tab panes do not double up with `AntCard` / `AntWidget` padding, while preserving explicitly customized margins; `AntTabs::useTabContentLayout()` can force zero margins when needed.
- `AntDatePicker` / `AntTimePicker` expose QDateEdit / QTimeEdit-style `date` / `time` aliases plus minimum / maximum range APIs.
- `AntList` / `AntListWidget` now cover common QListWidget-style flows: string item add/insert/find/sort, item text/icon/data/check state/flags, current row/item, selection mode, selected items, internal scrolling, `scrollToItem`, and item/current/selection signals. `AntTable` exposes `rows()`, `selectRow()`, `currentRowIndex()`, and row tooltips; `AntTree` exposes matching tree helper APIs.
- `AntMenu` now mirrors QWidget `QAction` additions, changes, removals, and trigger flow; `AntToolButton` / `AntToolBar` keep their inherited QAction behavior covered by tests.
- `AntTypography` defaults to vertical center alignment and exposes alignment, word-wrap, clear, and `setPixelSize()` controls; `setEnabled()` and `setDisabled()` stay synchronized with its disabled visual and interaction state.
- `AntDesign::initialize(&app)` provides one-call startup for Qt resources, bundled fonts, and the theme singleton, so consumer apps no longer need separate `Q_INIT_RESOURCE`, `AntFont::applyToApplication`, and `AntTheme::instance` calls.
- `AntRibbon` adds a lightweight Ribbon surface with pages, groups, balanced large/small actions, embedded Ant/Qt widgets, animated tab/collapse transitions, collapsed popup mode, and `AntWindow::setRibbon()` integration.

## Installation & Integration

### Requirements

- Qt `6.5+`
- CMake `3.16+`
- C++17

### Option 1: Add as a CMake subdirectory

```bash
git submodule add https://github.com/sorrowfeng/qt-ant-design.git third_party/qt-ant-design
git submodule update --init --recursive
```

```cmake
cmake_minimum_required(VERSION 3.16)
project(my-qt-app LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets)

add_subdirectory(third_party/qt-ant-design)

add_executable(my-qt-app main.cpp)
target_link_libraries(my-qt-app PRIVATE Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Widgets qt-ant-design)
```

### Option 2: Install and use the CMake package

```bash
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=/path/to/install
cmake --build build --config Release
cmake --install build --config Release
```

Then point your consumer project at the install prefix:

```cmake
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets Svg)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Svg)
find_package(qt-ant-design CONFIG REQUIRED)

add_executable(my-qt-app main.cpp)
target_link_libraries(my-qt-app PRIVATE
    qt-ant-design::qt-ant-design
)
```

Configure the consumer with `-DCMAKE_PREFIX_PATH=/path/to/install` if the prefix is not already on CMake's package search path.

On Windows you can also run the example app from the install directory directly:

```powershell
.\install\bin\qt-ant-design-example.exe
```

## Quick Start

```bash
git clone https://github.com/sorrowfeng/qt-ant-design.git
cd qt-ant-design
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH=/path/to/Qt
cmake --build .
```

Use `-DBUILD_SHARED_LIBS=ON` to build `qt-ant-design` as a shared library; omit it or set it to `OFF` for the default static build.

On Windows / multi-config generators, the recommended workflow is:

```powershell
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=D:/Project/GitProject/qt-ant-design/install
cmake --build build --config Debug
cmake --install build --config Debug
.\install\bin\qt-ant-design-example.exe
```

### Your first `AntButton`

Call `AntDesign::initialize(&app)` once after creating `QApplication` and before creating Ant widgets.

```cpp
#include <QApplication>
#include <QVBoxLayout>
#include <QWidget>

#include "core/AntDesign.h"
#include "widgets/AntButton.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    AntDesign::initialize(&app);

    QWidget window;
    auto* layout = new QVBoxLayout(&window);

    auto* button = new AntButton("Primary");
    button->setButtonType(Ant::ButtonType::Primary);
    layout->addWidget(button);

    window.resize(360, 200);
    window.show();

    return app.exec();
}
```

## Ported Components

Total public components implemented: `83`

`src/widgets` currently contains `104` `Ant*.h` headers: `83` public component headers, `20` Qt-style alias headers, and the internal popup helper `AntSelectPopup`.

Ant Design standard components are counted by the top-level directories under [`ant-design/ant-design`](https://github.com/ant-design/ant-design)'s `components/` directory, with `row / col` rolled into `grid`, `back-top` rolled into `float-button`, and `qrcode` treated as a compatibility alias for `qr-code` — yielding a baseline of `70` standard components.

### Qt-Style Aliases

Several components can also be included with Qt-style names when the Ant Design name is not a natural match: `AntLabel` → `AntTypography`, `AntLineEdit` → `AntInput`, `AntComboBox` → `AntSelect`, `AntSpinBox` / `AntDoubleSpinBox` → `AntInputNumber`, `AntPushButton` → `AntButton`, `AntProgressBar` → `AntProgress`, `AntCalendarWidget` → `AntCalendar`, `AntTabWidget` → `AntTabs`, `AntDialog` → `AntModal`, `AntMainWindow` → `AntWindow`, plus list/table/tree view-style aliases.

For names that only differ by casing from Qt, the Qt casing is canonical: use `AntCheckBox` and `AntToolTip`.

| Category | Components | Rendering |
| --- | --- | --- |
| General | `AntButton` `AntFloatButton` `AntIcon` `AntTypography` | `QProxyStyle` |
| Navigation | `AntAnchor` `AntBreadcrumb` `AntDropdown` `AntMenu` `AntPagination` `AntSteps` `AntTabs` | Mixed (`QProxyStyle` / custom paint) |
| Data Entry | `AntAutoComplete` `AntCascader` `AntCheckBox` `AntColorPicker` `AntDatePicker` `AntDescriptions` `AntForm` `AntInput` `AntInputNumber` `AntMentions` `AntRadio` `AntRate` `AntSegmented` `AntSelect` `AntSlider` `AntSwitch` `AntTimePicker` `AntTransfer` `AntTreeSelect` `AntUpload` | Mixed (`QProxyStyle` / custom paint) |
| Feedback | `AntAlert` `AntDrawer` `AntMessage` `AntModal` `AntNotification` `AntPopconfirm` `AntPopover` `AntProgress` `AntResult` `AntSkeleton` `AntSpin` `AntToolTip` `AntTour` `AntWatermark` | Mixed (`QProxyStyle` / custom paint) |
| Data Display | `AntAvatar` `AntBadge` `AntCalendar` `AntCard` `AntCarousel` `AntCollapse` `AntEmpty` `AntImage` `AntList` `AntQRCode` `AntStatistic` `AntTable` `AntTag` `AntTimeline` `AntTree` | Mixed (`QProxyStyle` / custom paint) |
| Layout & Misc | `AntAffix` `AntApp` `AntConfigProvider` `AntDivider` `AntFlex` `AntGrid` `AntLayout` `AntMasonry` `AntSpace` `AntSplitter` `AntWidget` `AntWindow` | Mixed (`QProxyStyle` / custom paint / QObject helper) |
| Qt / Desktop Extensions | `AntDockWidget` `AntLog` `AntMenuBar` `AntPlainTextEdit` `AntRibbon` `AntScrollArea` `AntScrollBar` `AntStatusBar` `AntToolBar` `AntToolButton` | Mixed (`QProxyStyle` / custom paint) |

### Component Screenshots

Light and dark thumbnails are generated from the example pages; interactive controls capture a representative open/active state where useful.

| Category | Component | Light | Dark |
| --- | --- | --- | --- |
| General | `AntButton` | <img src="resources/images/components/ant-button-light.png" width="360"> | <img src="resources/images/components/ant-button-dark.png" width="360"> |
| General | `AntIcon` | <img src="resources/images/components/ant-icon-light.png" width="360"> | <img src="resources/images/components/ant-icon-dark.png" width="360"> |
| General | `AntTypography` | <img src="resources/images/components/ant-typography-light.png" width="360"> | <img src="resources/images/components/ant-typography-dark.png" width="360"> |
| Layout | `AntDivider` | <img src="resources/images/components/ant-divider-light.png" width="360"> | <img src="resources/images/components/ant-divider-dark.png" width="360"> |
| Layout | `AntFlex` | <img src="resources/images/components/ant-flex-light.png" width="360"> | <img src="resources/images/components/ant-flex-dark.png" width="360"> |
| Layout | `AntGrid` | <img src="resources/images/components/ant-grid-light.png" width="360"> | <img src="resources/images/components/ant-grid-dark.png" width="360"> |
| Layout | `AntLayout` | <img src="resources/images/components/ant-layout-light.png" width="360"> | <img src="resources/images/components/ant-layout-dark.png" width="360"> |
| Layout | `AntSpace` | <img src="resources/images/components/ant-space-light.png" width="360"> | <img src="resources/images/components/ant-space-dark.png" width="360"> |
| Layout | `AntSplitter` | <img src="resources/images/components/ant-splitter-light.png" width="360"> | <img src="resources/images/components/ant-splitter-dark.png" width="360"> |
| Navigation | `AntAffix` | <img src="resources/images/components/ant-affix-light.png" width="360"> | <img src="resources/images/components/ant-affix-dark.png" width="360"> |
| Navigation | `AntAnchor` | <img src="resources/images/components/ant-anchor-light.png" width="360"> | <img src="resources/images/components/ant-anchor-dark.png" width="360"> |
| Navigation | `AntBreadcrumb` | <img src="resources/images/components/ant-breadcrumb-light.png" width="360"> | <img src="resources/images/components/ant-breadcrumb-dark.png" width="360"> |
| Navigation | `AntDropdown` | <img src="resources/images/components/ant-dropdown-light.png" width="360"> | <img src="resources/images/components/ant-dropdown-dark.png" width="360"> |
| Navigation | `AntMenu` | <img src="resources/images/components/ant-menu-light.png" width="360"> | <img src="resources/images/components/ant-menu-dark.png" width="360"> |
| Navigation | `AntPagination` | <img src="resources/images/components/ant-pagination-light.png" width="360"> | <img src="resources/images/components/ant-pagination-dark.png" width="360"> |
| Navigation | `AntSteps` | <img src="resources/images/components/ant-steps-light.png" width="360"> | <img src="resources/images/components/ant-steps-dark.png" width="360"> |
| Data Entry | `AntAutoComplete` | <img src="resources/images/components/ant-auto-complete-light.png" width="360"> | <img src="resources/images/components/ant-auto-complete-dark.png" width="360"> |
| Data Entry | `AntCascader` | <img src="resources/images/components/ant-cascader-light.png" width="360"> | <img src="resources/images/components/ant-cascader-dark.png" width="360"> |
| Data Entry | `AntCheckBox` | <img src="resources/images/components/ant-checkbox-light.png" width="360"> | <img src="resources/images/components/ant-checkbox-dark.png" width="360"> |
| Data Entry | `AntColorPicker` | <img src="resources/images/components/ant-color-picker-light.png" width="360"> | <img src="resources/images/components/ant-color-picker-dark.png" width="360"> |
| Data Entry | `AntDatePicker` | <img src="resources/images/components/ant-date-picker-light.png" width="360"> | <img src="resources/images/components/ant-date-picker-dark.png" width="360"> |
| Data Entry | `AntForm` | <img src="resources/images/components/ant-form-light.png" width="360"> | <img src="resources/images/components/ant-form-dark.png" width="360"> |
| Data Entry | `AntInput` | <img src="resources/images/components/ant-input-light.png" width="360"> | <img src="resources/images/components/ant-input-dark.png" width="360"> |
| Data Entry | `AntInputNumber` | <img src="resources/images/components/ant-input-number-light.png" width="360"> | <img src="resources/images/components/ant-input-number-dark.png" width="360"> |
| Data Entry | `AntMentions` | <img src="resources/images/components/ant-mentions-light.png" width="360"> | <img src="resources/images/components/ant-mentions-dark.png" width="360"> |
| Data Entry | `AntRadio` | <img src="resources/images/components/ant-radio-light.png" width="360"> | <img src="resources/images/components/ant-radio-dark.png" width="360"> |
| Data Entry | `AntRate` | <img src="resources/images/components/ant-rate-light.png" width="360"> | <img src="resources/images/components/ant-rate-dark.png" width="360"> |
| Data Entry | `AntSelect` | <img src="resources/images/components/ant-select-light.png" width="360"> | <img src="resources/images/components/ant-select-dark.png" width="360"> |
| Data Entry | `AntSlider` | <img src="resources/images/components/ant-slider-light.png" width="360"> | <img src="resources/images/components/ant-slider-dark.png" width="360"> |
| Data Entry | `AntSwitch` | <img src="resources/images/components/ant-switch-light.png" width="360"> | <img src="resources/images/components/ant-switch-dark.png" width="360"> |
| Data Entry | `AntTimePicker` | <img src="resources/images/components/ant-time-picker-light.png" width="360"> | <img src="resources/images/components/ant-time-picker-dark.png" width="360"> |
| Data Entry | `AntTransfer` | <img src="resources/images/components/ant-transfer-light.png" width="360"> | <img src="resources/images/components/ant-transfer-dark.png" width="360"> |
| Data Entry | `AntTreeSelect` | <img src="resources/images/components/ant-tree-select-light.png" width="360"> | <img src="resources/images/components/ant-tree-select-dark.png" width="360"> |
| Data Entry | `AntUpload` | <img src="resources/images/components/ant-upload-light.png" width="360"> | <img src="resources/images/components/ant-upload-dark.png" width="360"> |
| Data Display | `AntAvatar` | <img src="resources/images/components/ant-avatar-light.png" width="360"> | <img src="resources/images/components/ant-avatar-dark.png" width="360"> |
| Data Display | `AntBadge` | <img src="resources/images/components/ant-badge-light.png" width="360"> | <img src="resources/images/components/ant-badge-dark.png" width="360"> |
| Data Display | `AntCalendar` | <img src="resources/images/components/ant-calendar-light.png" width="360"> | <img src="resources/images/components/ant-calendar-dark.png" width="360"> |
| Data Display | `AntCard` | <img src="resources/images/components/ant-card-light.png" width="360"> | <img src="resources/images/components/ant-card-dark.png" width="360"> |
| Data Display | `AntCarousel` | <img src="resources/images/components/ant-carousel-light.png" width="360"> | <img src="resources/images/components/ant-carousel-dark.png" width="360"> |
| Data Display | `AntCollapse` | <img src="resources/images/components/ant-collapse-light.png" width="360"> | <img src="resources/images/components/ant-collapse-dark.png" width="360"> |
| Data Display | `AntDescriptions` | <img src="resources/images/components/ant-descriptions-light.png" width="360"> | <img src="resources/images/components/ant-descriptions-dark.png" width="360"> |
| Data Display | `AntEmpty` | <img src="resources/images/components/ant-empty-light.png" width="360"> | <img src="resources/images/components/ant-empty-dark.png" width="360"> |
| Data Display | `AntImage` | <img src="resources/images/components/ant-image-light.png" width="360"> | <img src="resources/images/components/ant-image-dark.png" width="360"> |
| Data Display | `AntList` | <img src="resources/images/components/ant-list-light.png" width="360"> | <img src="resources/images/components/ant-list-dark.png" width="360"> |
| Data Display | `AntPopover` | <img src="resources/images/components/ant-popover-light.png" width="360"> | <img src="resources/images/components/ant-popover-dark.png" width="360"> |
| Data Display | `AntQRCode` | <img src="resources/images/components/ant-qr-code-light.png" width="360"> | <img src="resources/images/components/ant-qr-code-dark.png" width="360"> |
| Data Display | `AntSegmented` | <img src="resources/images/components/ant-segmented-light.png" width="360"> | <img src="resources/images/components/ant-segmented-dark.png" width="360"> |
| Data Display | `AntStatistic` | <img src="resources/images/components/ant-statistic-light.png" width="360"> | <img src="resources/images/components/ant-statistic-dark.png" width="360"> |
| Data Display | `AntTable` | <img src="resources/images/components/ant-table-light.png" width="360"> | <img src="resources/images/components/ant-table-dark.png" width="360"> |
| Data Display | `AntTabs` | <img src="resources/images/components/ant-tabs-light.png" width="360"> | <img src="resources/images/components/ant-tabs-dark.png" width="360"> |
| Data Display | `AntTag` | <img src="resources/images/components/ant-tag-light.png" width="360"> | <img src="resources/images/components/ant-tag-dark.png" width="360"> |
| Data Display | `AntTimeline` | <img src="resources/images/components/ant-timeline-light.png" width="360"> | <img src="resources/images/components/ant-timeline-dark.png" width="360"> |
| Data Display | `AntToolTip` | <img src="resources/images/components/ant-tooltip-light.png" width="360"> | <img src="resources/images/components/ant-tooltip-dark.png" width="360"> |
| Data Display | `AntTree` | <img src="resources/images/components/ant-tree-light.png" width="360"> | <img src="resources/images/components/ant-tree-dark.png" width="360"> |
| Feedback | `AntAlert` | <img src="resources/images/components/ant-alert-light.png" width="360"> | <img src="resources/images/components/ant-alert-dark.png" width="360"> |
| Feedback | `AntDrawer` | <img src="resources/images/components/ant-drawer-light.png" width="360"> | <img src="resources/images/components/ant-drawer-dark.png" width="360"> |
| Feedback | `AntMessage` | <img src="resources/images/components/ant-message-light.png" width="360"> | <img src="resources/images/components/ant-message-dark.png" width="360"> |
| Feedback | `AntModal` | <img src="resources/images/components/ant-modal-light.png" width="360"> | <img src="resources/images/components/ant-modal-dark.png" width="360"> |
| Feedback | `AntNotification` | <img src="resources/images/components/ant-notification-light.png" width="360"> | <img src="resources/images/components/ant-notification-dark.png" width="360"> |
| Feedback | `AntPopconfirm` | <img src="resources/images/components/ant-popconfirm-light.png" width="360"> | <img src="resources/images/components/ant-popconfirm-dark.png" width="360"> |
| Feedback | `AntProgress` | <img src="resources/images/components/ant-progress-light.png" width="360"> | <img src="resources/images/components/ant-progress-dark.png" width="360"> |
| Feedback | `AntResult` | <img src="resources/images/components/ant-result-light.png" width="360"> | <img src="resources/images/components/ant-result-dark.png" width="360"> |
| Feedback | `AntSkeleton` | <img src="resources/images/components/ant-skeleton-light.png" width="360"> | <img src="resources/images/components/ant-skeleton-dark.png" width="360"> |
| Feedback | `AntSpin` | <img src="resources/images/components/ant-spin-light.png" width="360"> | <img src="resources/images/components/ant-spin-dark.png" width="360"> |
| Feedback | `AntTour` | <img src="resources/images/components/ant-tour-light.png" width="360"> | <img src="resources/images/components/ant-tour-dark.png" width="360"> |
| Feedback | `AntWatermark` | <img src="resources/images/components/ant-watermark-light.png" width="360"> | <img src="resources/images/components/ant-watermark-dark.png" width="360"> |
| Other | `AntApp` | <img src="resources/images/components/ant-app-light.png" width="360"> | <img src="resources/images/components/ant-app-dark.png" width="360"> |
| Other | `AntConfigProvider` | <img src="resources/images/components/ant-config-provider-light.png" width="360"> | <img src="resources/images/components/ant-config-provider-dark.png" width="360"> |
| Other | `AntFloatButton` | <img src="resources/images/components/ant-float-button-light.png" width="360"> | <img src="resources/images/components/ant-float-button-dark.png" width="360"> |
| Qt Extensions | `AntWindow` | <img src="resources/images/components/ant-window-light.png" width="360"> | <img src="resources/images/components/ant-window-dark.png" width="360"> |
| Qt Extensions | `AntWidget` | <img src="resources/images/components/ant-widget-light.png" width="360"> | <img src="resources/images/components/ant-widget-dark.png" width="360"> |
| Qt Extensions | `AntScrollArea` | <img src="resources/images/components/ant-scroll-area-light.png" width="360"> | <img src="resources/images/components/ant-scroll-area-dark.png" width="360"> |
| Qt Extensions | `AntScrollBar` | <img src="resources/images/components/ant-scroll-bar-light.png" width="360"> | <img src="resources/images/components/ant-scroll-bar-dark.png" width="360"> |
| Qt Extensions | `AntStatusBar` | <img src="resources/images/components/ant-status-bar-light.png" width="360"> | <img src="resources/images/components/ant-status-bar-dark.png" width="360"> |
| Qt Extensions | `AntRibbon` | <img src="resources/images/components/ant-ribbon-light.png" width="360"> | <img src="resources/images/components/ant-ribbon-dark.png" width="360"> |
| Qt Extensions | `AntMenuBar` | <img src="resources/images/components/ant-menu-bar-light.png" width="360"> | <img src="resources/images/components/ant-menu-bar-dark.png" width="360"> |
| Qt Extensions | `AntToolBar` | <img src="resources/images/components/ant-tool-bar-light.png" width="360"> | <img src="resources/images/components/ant-tool-bar-dark.png" width="360"> |
| Qt Extensions | `AntToolButton` | <img src="resources/images/components/ant-tool-button-light.png" width="360"> | <img src="resources/images/components/ant-tool-button-dark.png" width="360"> |
| Qt Extensions | `AntDockWidget` | <img src="resources/images/components/ant-dock-widget-light.png" width="360"> | <img src="resources/images/components/ant-dock-widget-dark.png" width="360"> |
| Qt Extensions | `AntPlainTextEdit` | <img src="resources/images/components/ant-plain-text-edit-light.png" width="360"> | <img src="resources/images/components/ant-plain-text-edit-dark.png" width="360"> |
| Qt Extensions | `AntLog` | <img src="resources/images/components/ant-log-light.png" width="360"> | <img src="resources/images/components/ant-log-dark.png" width="360"> |
| Qt Extensions | `AntNavItem` | <img src="resources/images/components/ant-nav-item-light.png" width="360"> | <img src="resources/images/components/ant-nav-item-dark.png" width="360"> |
| Qt Extensions | `AntMasonry` | <img src="resources/images/components/ant-masonry-light.png" width="360"> | <img src="resources/images/components/ant-masonry-dark.png" width="360"> |

### Component Highlights

- `AntButton`: five types, three sizes, three shapes, `loading / danger / ghost / block`
- `AntIcon`: `831` official SVG icons, string-name API, `Outlined / Filled / TwoTone`, rotation, spin, custom paths
- `AntInput`: sizes, states, `addonBefore / addonAfter / allowClear / password`
- `AntInputNumber`: sizes, states, variants, prefix/suffix, QDoubleSpinBox-style decimals/precision, fine-grained step, optional control buttons
- `AntDescriptions`: title, extra, columns, bordered, vertical, custom value widgets
- `AntForm`: `AntForm / AntFormItem`, horizontal / vertical / inline layouts, label alignment, required marker, help and validation hints
- `AntEmpty`: default illustration, `simple` mode, description text, custom illustration size and extra action
- `AntDropdown`: `hover / click / contextMenu` triggers, placement, arrow, auto flip
- `AntSteps`: horizontal / vertical layout, current step, error state, click to switch, title / description / subtitle
- `AntSelect`: sizes, states, variants, `allowClear / loading / popup`, option text/data management
- `AntAlert`: `success / info / warning / error`, icon, description, closable, banner, custom action
- `AntModal`: mask, title, body, custom content, custom footer, confirm / cancel, centered or top-offset layout, and a soft outer shadow that fades before the dialog edge
- `AntResult`: status icons (success / error / warning / info), transparent icon background in dark mode, title, description, custom extra actions area
- `AntList`: `header / footer / bordered / split / size`; `AntListItem` supports `Meta` (avatar, title, description), action areas, internal scrolling, and QListWidget-style text/data/selection helpers through the `AntListWidget` alias
- `AntStatistic`: numeric display, thousands separators, prefix / suffix, precision control
- `AntPopover`: title, body, action, click / hover triggers, placement, arrow
- `AntPopconfirm`: confirm title, description, confirm / cancel buttons, disabled state, placement, and a one-piece popup arrow surface
- `AntSkeleton`: moving `active` shimmer, avatar placeholder, title / paragraph configuration, rounded style, and `loading` toggle to swap in real content
- `AntToolTip`: common `placement`, arrow, color, delayed display, auto flip
- `AntSlider`: horizontal / vertical, `reverse / dots / included`, range, marks, drag value bubble above the active handle with a one-piece arrow surface
- `AntRibbon`: pages and groups for large/small actions, embedded Ant/Qt widgets, animated tab/collapse transitions, collapsed popup mode, and `AntWindow` top-area integration
- `AntSwitch`: `checked / loading / small / text`, click wave feedback
- `AntSpin`: `small / middle / large / percent / delay`, smoother high-frequency animation
- `AntDatePicker` / `AntTimePicker`: hand-painted popup pickers
- `AntMessage` / `AntNotification`: global feedback components with elevated surfaces, enter / exit motion, and Notification loading progress countdown
- `AntCard` / `AntTag` / `AntBadge` / `AntAvatar`: common display components
- `AntMenu` / `AntTabs` / `AntBreadcrumb` / `AntPagination`: navigation components; `AntPagination` includes editable quick-jumper paging, and `AntTabs` includes tab-pane layout margin normalization helpers
- `AntTable`: data table with column sorting, row selection (checkbox / radio), programmatic row selection, row tooltips, pagination, loading state
- `AntTree`: tree control with expand / collapse, node selection, checkboxes, connector lines
- `AntUpload`: file upload supporting text list, picture list, and picture card modes
- `AntCascader`: cascading selector with multi-column popup, click / hover expansion
- `AntTreeSelect`: tree selector that renders a tree inside a dropdown
- `AntRate`: rating component with `count / value / allowHalf / allowClear / disabled / size`, hover scaling, selected-star scale pulse, left / right keyboard control
- `AntWidget`: base QWidget subclass that handles theme switching automatically
- `AntTypography`: theme-aware text component, Title (H1-H5) / Text / Paragraph, with type / decoration / copy / ellipsis / alignment / pixel-size support
- `AntWindow`: frameless window with custom title bar, pin / theme / minimize / maximize / close buttons, Windows 11 Snap support, Windows 10/11 DWM border shadow, and a smooth theme transition overlay
- `AntDrawer`: sliding panel with Left / Right / Top / Bottom placement, animation, and mask
- `AntStatusBar`: status bar with left / right items, separators, message area, and size grip
- `AntScrollBar`: custom 8 px slim scrollbar with auto-hide and no arrow buttons
- `AntSegmented`: segmented control with value and index selection APIs, evenly distributed options, animated indicator, and icon / disabled / tooltip support
- `AntFloatButton`: floating action button — circle / square, Primary / Default, expandable Group, BackTop, Badge
- `AntWatermark`: watermark overlay with rotated tiled text, multi-line content, and customizable font / color / spacing / offset / angle
- `AntQRCode`: QR code display with embedded generator (no external dependency), status overlays (expired / loading / scanned), icon, no border
- `AntAffix`: pin helper — a QObject utility that watches the scroll container and auto-pins / un-pins while preserving layout
- `AntAutoComplete`: autocomplete input with popup suggestions and keyboard navigation
- `AntCalendar`: calendar panel with Day / Month / Year mode switching and date selection
- `AntCarousel`: carousel with autoplay, dot indicators, animated slide transitions, and click-to-page
- `AntCollapse`: collapse panel / accordion with InOutCubic expand animation and accordion exclusivity
- `AntColorPicker`: inline color trigger with optional text, plus popup HS field + value slider + RGB / HSV inputs, preset and custom colors
- `AntImage`: image display with placeholder fallback and click-to-fullscreen preview
- `AntTransfer`: transfer component with two scrollable lists, header select-all, and batch movement
- `AntTour`: masked step-by-step guide with target highlighting, direct step launch, and Prev / Next / Finish
- `AntMentions`: `@` mentions input that pops suggestions on `@`
- `AntGrid` (Row/Col): 24-column grid with span / offset / gutter
- `AntFlex`: flex layout container with gap / wrap / vertical
- `AntMasonry`: masonry layout (shortest-column-first)
- `AntSplitter`: draggable splitter with theme-colored handle
- `AntAnchor`: scroll anchor navigation with active link highlighting
- `AntApp`: application wrapper providing message / modal / notification context
- `AntConfigProvider`: global configuration for theme / primary color / font size / border radius
- `AntToolButton`: QToolButton + QProxyStyle with dropdown arrow animation
- `AntMenuBar`: themed QMenuBar
- `AntToolBar`: themed QToolBar with floating shadow
- `AntDockWidget`: dockable panel with custom title bar and Win32 resize edges
- `AntScrollArea`: QScrollArea + AntScrollBar + QScroller gesture scrolling
- `AntPlainTextEdit`: multi-line text editor with 3 variants, TextArea-style resize grip, and a context menu
- `AntLog`: 5-level colored log output (Debug / Info / Success / Warning / Error) with timestamps

## Usage Examples

### AntButton

```cpp
#include "widgets/AntButton.h"

auto* primary = new AntButton("Save");
primary->setButtonType(Ant::ButtonType::Primary);
primary->setButtonSize(Ant::ButtonSize::Middle);

auto* danger = new AntButton("Delete");
danger->setDanger(true);
```

### AntInput

```cpp
#include "widgets/AntInput.h"

auto* input = new AntInput();
input->setPlaceholderText("Please enter a name");
input->setAllowClear(true);
input->setInputSize(Ant::InputSize::Large);
```

### AntCard

```cpp
#include "widgets/AntCard.h"
#include "widgets/AntTypography.h"

auto* card = new AntCard("User Profile");
card->setExtra("More");
card->setHoverable(true);
card->bodyLayout()->addWidget(new AntTypography("Card content"));
```

### Theme switching

```cpp
#include "core/AntTheme.h"

AntTheme::instance()->setThemeMode(Ant::ThemeMode::Dark);
```

A theme switch currently triggers `polish / updateGeometry / update` on every `QProxyStyle`-based component. `AntWindow`'s built-in theme button wraps the repaint in a captured-frame overlay with a soft reveal animation so full-window light/dark switches stay visually continuous.

## Development Guide & Contributing

The project uses `AGENTS.md` as the AI collaboration spec and project sync document, tracking:

- Ported component list
- Visual audit checklist
- Current architectural conventions
- Example coverage
- Build and install instructions

The per-component visual audit tracker lives in `docs/visual-audit.md`.

When adding a new component, the recommended flow is:

1. Read the API and styles in [`ant-design/ant-design`](https://github.com/ant-design/ant-design) under `components/<component>/`
2. Add `src/widgets/Ant<Name>.h/.cpp`
3. If style decoupling is needed, add `src/styles/Ant<Name>Style.h/.cpp`
4. Add a demo page in `examples/ExampleWindow.cpp`
5. Update `AGENTS.md` and `README.md`

Issues and PRs are welcome.

## Acknowledgements

- Thanks to Ant Design for the design system, component specs, and token foundation: [ant-design/ant-design](https://github.com/ant-design/ant-design)
- Thanks to ElaWidgetTools for Qt widget references: [Liniyous/ElaWidgetTools](https://github.com/Liniyous/ElaWidgetTools)
- Development note: 90%+ of this project was developed with Codex GPT-5.5; the rest was completed with Claude Code and Mimo v2.5 Pro.

## License

MIT License
