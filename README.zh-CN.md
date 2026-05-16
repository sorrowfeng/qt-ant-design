# qt-ant-design

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![Qt](https://img.shields.io/badge/Qt-6%20%7C%205-green.svg)](https://www.qt.io)
[![CMake](https://img.shields.io/badge/CMake-3.16+-blue.svg)](https://cmake.org)
[![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com)
[![GitHub Stars](https://img.shields.io/github/stars/sorrowfeng/qt-ant-design?style=social)](https://github.com/sorrowfeng/qt-ant-design/stargazers)
[![Last Commit](https://img.shields.io/github/last-commit/sorrowfeng/qt-ant-design)](https://github.com/sorrowfeng/qt-ant-design/commits/main)
[![Ask DeepWiki](https://deepwiki.com/badge.svg)](https://deepwiki.com/sorrowfeng/qt-ant-design)

[English](README.md) | 简体中文

`qt-ant-design` 是一个基于 Qt Widgets 的 C++ 组件库，配置时可自动识别 Qt6 或 Qt5，目标是将 Ant Design 设计系统移植到原生桌面组件中。

项目强调：

- 亮暗主题动态切换
- 尽可能贴近 Ant Design 的交互与状态表现
- 使用 `QPainter` / `QProxyStyle` 构建可维护的桌面绘制体系

> 当前实现与视觉审计状态记录在 [docs/project-status.md](docs/project-status.md) 和 [docs/visual-audit.md](docs/visual-audit.md)。

> 欢迎提交 Issue 和 PR：视觉差异、交互缺失、Qt 集成问题、文档遗漏、组件修复、测试、示例和文档改进都很欢迎。

## Showcase

| 亮色 | 暗色 |
| --- | --- |
| ![Qt Showcase 亮色](resources/images/showcase-light.png) | ![Qt Showcase 暗色](resources/images/showcase-dark.png) |

## 特性

- 基于 Qt Widgets，轻量、易集成，可作为静态库或动态库接入现有项目
- 内置 Design Token 系统，支持亮色 / 暗色主题实时切换
- 当前已移植 `83` 个公开组件（Ant Design 标准组件 `70 / 70` 全覆盖，另含 `13` 个 Qt / 桌面扩展组件）
- 当前 `62` 个组件使用 `QProxyStyle` 架构绘制
- 示例程序当前覆盖 `83 / 83` 个公开组件，另有独立 Ant Design 首页风格 `Showcase`
- `AntIcon` 已内置 `831` 个来自 `@ant-design/icons-svg@4.4.2` 的官方 SVG 资源
- 可对比的标准组件已在视觉审计矩阵中标记为 `Pass`，Qt-only 桌面扩展标记为 `Local Pass`
- 代码结构清晰，`core / styles / widgets / examples` 分层明确，便于扩展

## 当前状态

- 状态总览：[docs/project-status.md](docs/project-status.md)
- 逐控件可靠性覆盖：[docs/reliability-coverage.md](docs/reliability-coverage.md)
- 视觉审计矩阵：[docs/visual-audit.md](docs/visual-audit.md)
- 官方图标清单：[docs/ant-design-icons.md](docs/ant-design-icons.md)
- 当前 CTest 目标数：`37`；最近一次全控件可靠性巡检：`37 / 37` 通过（`2026-05-10`）

## 最近 Ant Design 对齐更新

2026-04-30 的交互与动效对齐批次补齐了多处用户可见细节：

- 弹层反馈：`AntPopover`、`AntMessage`、`AntNotification` 的悬停/关闭行为更稳定，阴影层级更清晰，并补齐了按 placement 进入/退出的动效。被动浮层不会抢走下层控件点击：Message 点击时转发到底层控件，Tooltip、Slider 数值浮标和 Watermark 保持鼠标透明。
- 弹层外壳：共享弹层阴影改为围绕面板向外绘制柔和多层羽化，`AntDropdown`、`AntMenu`、选择类弹层、`AntColorPicker`、`AntDatePicker`、`AntTimePicker`、`AntModal` 在亮色/暗色主题下都保留接近 AntD 的阴影层级。
- 动效表现：`AntCarousel`、`AntTabs`、`AntSkeleton`、`AntSpin`、`AntInputNumber`、`AntSwitch` 和 loading button 的方向、节奏、状态反馈更贴近 Ant Design。
- 数据交互：`AntTransfer` 支持正常滚动和顶部全选，`AntTable` 表头排序点击会真正重排行数据。
- 输入反馈：`AntPlainTextEdit` 支持 TextArea 式右下角拖拽缩放，`AntSlider` 拖动时显示数值浮标。

## 最近桌面窗口更新

2026-05-07 的 `AntWindow` 批次增强了原生桌面行为和标题栏细节：

- 无边框窗口支持 Windows 11 Snap：四边/四角缩放、标题栏拖拽、最大化按钮 Snap Layout hover、边缘吸附和最大化后拖拽还原。
- Windows 下接入 DWM 圆角、边框/阴影，并提供 `cornerRadius` API；平台相关实现均通过 Qt/Win32 宏隔离。
- Windows 10 走无 native caption 的窗口样式，并使用 legacy rounded mask 与透明软件阴影宿主窗口，避免最大化/还原后露出原生标题栏按钮，同时让普通窗口在缩放前后都保持从窗口边缘直接外扩、轻量、更接近 Win11 且圆角更干净的四周阴影。
- Windows 已显示窗口切换置顶/取消置顶时改用 native `SetWindowPos` 原地更新，避免 Qt flags 重建窗口造成可见闪烁。
- 标题栏新增置顶和亮暗主题切换按钮，使用内置官方 Ant Design 图标；所有标题栏按钮均可通过公开 API 控制显示或隐藏。
- 内置主题按钮使用全窗口截图 overlay 和柔和揭示动画，让 Light/Dark 全局切换更连续。

## 最近 Qt API 兼容更新

2026-05-07 的 API 批次增强了 Qt 对象树接入和常见 Qt 控件习惯：

- `AntInput`、`AntInputNumber`、`AntCheckBox`、`AntRadio`、`AntSlider`、`AntProgress`、`AntStatusBar` 补充更多 Qt 风格常用方法和信号；`AntInputNumber` 默认整数显示，通过 `setDecimals()` 或 `setPrecision()` 开启 QDoubleSpinBox 风格的小数 value/range/step 行为。
- `AntSelect` 支持 QComboBox 风格 item 管理，空列表首次加入数据时默认选中首项，并补充 `setOptionText`、`removeOption`、`optionData`、`findData`、`currentData`、`activated` 和 highlighted 信号。
- Qt Layout 自适应行为已按官方控件基准对齐：LineEdit 类控件横向扩展，ComboBox 类选择器保持 `Preferred/Fixed`，Spin/Date/Time 编辑器保持 `Minimum/Fixed`，List/Table/Tree 等视图双向扩展，`AntTypography` 对齐 `QLabel` 的 height-for-width 换行策略。
- `AntTabs` 添加内容页时会清理页面根布局的默认 Qt margins，避免 Tab 页与 `AntCard` / `AntWidget` 内部 padding 叠加；显式自定义 margins 会保留，需要强制清零时可调用 `AntTabs::useTabContentLayout()`。
- `AntDatePicker` / `AntTimePicker` 支持 QDateEdit / QTimeEdit 风格的 `date` / `time` 别名，以及最小 / 最大范围 API。
- `AntList` / `AntListWidget` 补充字符串 add/insert/find/sort、item 数据、current/selection、内部滚动和 `scrollToItem` 等 `QListWidget` 风格接口；`AntTable` 补充 `rows()`、`selectRow()`、`currentRowIndex()` 和行级 tooltip；`AntTree` 继续覆盖 tree 风格辅助接口。
- `AntMenu` 支持 QWidget `QAction` 的添加、变更、移除和触发流程同步；`AntToolButton` / `AntToolBar` 的继承 QAction 行为已加入测试覆盖。
- `AntTypography` 默认垂直居中，并提供 alignment、word-wrap、clear 和 `setPixelSize()` 控制；`setEnabled()` / `setDisabled()` 会同步 Typography 的 disabled 视觉与交互状态。
- `AntDesign::initialize(&app)` 提供统一启动入口，一次性完成 Qt 资源注册、内置字体应用和主题单例初始化，外部项目不再需要分别调用 `Q_INIT_RESOURCE`、`AntFont::applyToApplication` 和 `AntTheme::instance`。
- `AntRibbon` 增加轻量 Ribbon 区域，支持 Page、Group、大/小 QAction、嵌入 Ant/Qt 控件、折叠弹出模式，以及 `AntWindow::setRibbon()` 顶部集成。

## 安装与集成

### 环境要求

- Qt `6.5+`
- CMake `3.16+`
- C++17

### 方式一：作为子目录接入 CMake 项目

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

### 方式二：安装并使用 CMake package

```bash
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=/path/to/install
cmake --build build --config Release
cmake --install build --config Release
```

然后让你的消费项目指向安装前缀：

```cmake
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets Svg)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Svg)
find_package(qt-ant-design CONFIG REQUIRED)

add_executable(my-qt-app main.cpp)
target_link_libraries(my-qt-app PRIVATE
    qt-ant-design::qt-ant-design
)
```

如果该安装前缀不在 CMake package 搜索路径中，请在配置消费项目时传入 `-DCMAKE_PREFIX_PATH=/path/to/install`。

Windows 下也可以直接使用安装目录中的示例程序：

```powershell
.\install\bin\qt-ant-design-example.exe
```

## 快速开始

```bash
git clone https://github.com/sorrowfeng/qt-ant-design.git
cd qt-ant-design
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH=/path/to/Qt
cmake --build .
```

传入 `-DBUILD_SHARED_LIBS=ON` 可构建 `qt-ant-design` 动态库；不传或设为 `OFF` 时保持默认静态库构建。

在 Windows / 多配置生成器下，推荐使用：

```powershell
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=D:/Project/GitProject/qt-ant-design/install
cmake --build build --config Debug
cmake --install build --config Debug
.\install\bin\qt-ant-design-example.exe
```

### 第一个 `AntButton`

创建 `QApplication` 后、创建 Ant 控件前调用一次 `AntDesign::initialize(&app)` 即可。

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

## 已移植组件

当前已实现公开组件总数：`83`

`src/widgets` 当前包含 `104` 个 `Ant*.h` 头文件：`83` 个公开组件头、`20` 个 Qt 风格别名头，以及内部弹层 helper `AntSelectPopup`。

Ant Design 标准组件按 [`ant-design/ant-design`](https://github.com/ant-design/ant-design) 仓库 `components/` 顶层目录统计，并将 `row / col` 并入 `grid`、`back-top` 并入 `float-button`、`qrcode` 视为 `qr-code` 兼容别名，因此当前标准组件口径为 `70`。

### Qt 风格别名

当 Ant Design 命名与常用 Qt 控件名不直观对应时，组件也提供 Qt 风格别名头：`AntLabel` → `AntTypography`、`AntLineEdit` → `AntInput`、`AntComboBox` → `AntSelect`、`AntSpinBox` / `AntDoubleSpinBox` → `AntInputNumber`、`AntPushButton` → `AntButton`、`AntProgressBar` → `AntProgress`、`AntCalendarWidget` → `AntCalendar`、`AntTabWidget` → `AntTabs`、`AntDialog` → `AntModal`、`AntMainWindow` → `AntWindow`，以及 List / Table / Tree 的 view-style 别名。

如果只是大小写与 Qt 不一致，则以 Qt 命名为准：使用 `AntCheckBox` 和 `AntToolTip`。

| 分类 | 组件 | 当前绘制方式 |
| --- | --- | --- |
| 通用 | `AntButton` `AntFloatButton` `AntIcon` `AntTypography` | `QProxyStyle` |
| 导航 | `AntAnchor` `AntBreadcrumb` `AntDropdown` `AntMenu` `AntPagination` `AntSteps` `AntTabs` | 混合（`QProxyStyle` / 自绘） |
| 数据录入 | `AntAutoComplete` `AntCascader` `AntCheckBox` `AntColorPicker` `AntDatePicker` `AntDescriptions` `AntForm` `AntInput` `AntInputNumber` `AntMentions` `AntRadio` `AntRate` `AntSegmented` `AntSelect` `AntSlider` `AntSwitch` `AntTimePicker` `AntTransfer` `AntTreeSelect` `AntUpload` | 混合（`QProxyStyle` / 自绘） |
| 反馈 | `AntAlert` `AntDrawer` `AntMessage` `AntModal` `AntNotification` `AntPopconfirm` `AntPopover` `AntProgress` `AntResult` `AntSkeleton` `AntSpin` `AntToolTip` `AntTour` `AntWatermark` | 混合（`QProxyStyle` / 自绘） |
| 数据展示 | `AntAvatar` `AntBadge` `AntCalendar` `AntCard` `AntCarousel` `AntCollapse` `AntEmpty` `AntImage` `AntList` `AntQRCode` `AntStatistic` `AntTable` `AntTag` `AntTimeline` `AntTree` | 混合（`QProxyStyle` / 自绘） |
| 布局与其他 | `AntAffix` `AntApp` `AntConfigProvider` `AntDivider` `AntFlex` `AntGrid` `AntLayout` `AntMasonry` `AntSpace` `AntSplitter` `AntWidget` `AntWindow` | 混合（`QProxyStyle` / 自绘 / QObject 工具） |
| Qt / 桌面扩展 | `AntDockWidget` `AntLog` `AntMenuBar` `AntPlainTextEdit` `AntRibbon` `AntScrollArea` `AntScrollBar` `AntStatusBar` `AntToolBar` `AntToolButton` | 混合（`QProxyStyle` / 自绘） |

### 组件截图

以下 Light / Dark 缩略图由示例页生成；涉及弹层或反馈态的控件会截取代表性的打开/激活状态。

| 分类 | 组件 | Light | Dark |
| --- | --- | --- | --- |
| 通用 | `AntButton` | <img src="resources/images/components/ant-button-light.png" width="360"> | <img src="resources/images/components/ant-button-dark.png" width="360"> |
| 通用 | `AntIcon` | <img src="resources/images/components/ant-icon-light.png" width="360"> | <img src="resources/images/components/ant-icon-dark.png" width="360"> |
| 通用 | `AntTypography` | <img src="resources/images/components/ant-typography-light.png" width="360"> | <img src="resources/images/components/ant-typography-dark.png" width="360"> |
| 布局 | `AntDivider` | <img src="resources/images/components/ant-divider-light.png" width="360"> | <img src="resources/images/components/ant-divider-dark.png" width="360"> |
| 布局 | `AntFlex` | <img src="resources/images/components/ant-flex-light.png" width="360"> | <img src="resources/images/components/ant-flex-dark.png" width="360"> |
| 布局 | `AntGrid` | <img src="resources/images/components/ant-grid-light.png" width="360"> | <img src="resources/images/components/ant-grid-dark.png" width="360"> |
| 布局 | `AntLayout` | <img src="resources/images/components/ant-layout-light.png" width="360"> | <img src="resources/images/components/ant-layout-dark.png" width="360"> |
| 布局 | `AntSpace` | <img src="resources/images/components/ant-space-light.png" width="360"> | <img src="resources/images/components/ant-space-dark.png" width="360"> |
| 布局 | `AntSplitter` | <img src="resources/images/components/ant-splitter-light.png" width="360"> | <img src="resources/images/components/ant-splitter-dark.png" width="360"> |
| 导航 | `AntAffix` | <img src="resources/images/components/ant-affix-light.png" width="360"> | <img src="resources/images/components/ant-affix-dark.png" width="360"> |
| 导航 | `AntAnchor` | <img src="resources/images/components/ant-anchor-light.png" width="360"> | <img src="resources/images/components/ant-anchor-dark.png" width="360"> |
| 导航 | `AntBreadcrumb` | <img src="resources/images/components/ant-breadcrumb-light.png" width="360"> | <img src="resources/images/components/ant-breadcrumb-dark.png" width="360"> |
| 导航 | `AntDropdown` | <img src="resources/images/components/ant-dropdown-light.png" width="360"> | <img src="resources/images/components/ant-dropdown-dark.png" width="360"> |
| 导航 | `AntMenu` | <img src="resources/images/components/ant-menu-light.png" width="360"> | <img src="resources/images/components/ant-menu-dark.png" width="360"> |
| 导航 | `AntPagination` | <img src="resources/images/components/ant-pagination-light.png" width="360"> | <img src="resources/images/components/ant-pagination-dark.png" width="360"> |
| 导航 | `AntSteps` | <img src="resources/images/components/ant-steps-light.png" width="360"> | <img src="resources/images/components/ant-steps-dark.png" width="360"> |
| 数据录入 | `AntAutoComplete` | <img src="resources/images/components/ant-auto-complete-light.png" width="360"> | <img src="resources/images/components/ant-auto-complete-dark.png" width="360"> |
| 数据录入 | `AntCascader` | <img src="resources/images/components/ant-cascader-light.png" width="360"> | <img src="resources/images/components/ant-cascader-dark.png" width="360"> |
| 数据录入 | `AntCheckBox` | <img src="resources/images/components/ant-checkbox-light.png" width="360"> | <img src="resources/images/components/ant-checkbox-dark.png" width="360"> |
| 数据录入 | `AntColorPicker` | <img src="resources/images/components/ant-color-picker-light.png" width="360"> | <img src="resources/images/components/ant-color-picker-dark.png" width="360"> |
| 数据录入 | `AntDatePicker` | <img src="resources/images/components/ant-date-picker-light.png" width="360"> | <img src="resources/images/components/ant-date-picker-dark.png" width="360"> |
| 数据录入 | `AntForm` | <img src="resources/images/components/ant-form-light.png" width="360"> | <img src="resources/images/components/ant-form-dark.png" width="360"> |
| 数据录入 | `AntInput` | <img src="resources/images/components/ant-input-light.png" width="360"> | <img src="resources/images/components/ant-input-dark.png" width="360"> |
| 数据录入 | `AntInputNumber` | <img src="resources/images/components/ant-input-number-light.png" width="360"> | <img src="resources/images/components/ant-input-number-dark.png" width="360"> |
| 数据录入 | `AntMentions` | <img src="resources/images/components/ant-mentions-light.png" width="360"> | <img src="resources/images/components/ant-mentions-dark.png" width="360"> |
| 数据录入 | `AntRadio` | <img src="resources/images/components/ant-radio-light.png" width="360"> | <img src="resources/images/components/ant-radio-dark.png" width="360"> |
| 数据录入 | `AntRate` | <img src="resources/images/components/ant-rate-light.png" width="360"> | <img src="resources/images/components/ant-rate-dark.png" width="360"> |
| 数据录入 | `AntSelect` | <img src="resources/images/components/ant-select-light.png" width="360"> | <img src="resources/images/components/ant-select-dark.png" width="360"> |
| 数据录入 | `AntSlider` | <img src="resources/images/components/ant-slider-light.png" width="360"> | <img src="resources/images/components/ant-slider-dark.png" width="360"> |
| 数据录入 | `AntSwitch` | <img src="resources/images/components/ant-switch-light.png" width="360"> | <img src="resources/images/components/ant-switch-dark.png" width="360"> |
| 数据录入 | `AntTimePicker` | <img src="resources/images/components/ant-time-picker-light.png" width="360"> | <img src="resources/images/components/ant-time-picker-dark.png" width="360"> |
| 数据录入 | `AntTransfer` | <img src="resources/images/components/ant-transfer-light.png" width="360"> | <img src="resources/images/components/ant-transfer-dark.png" width="360"> |
| 数据录入 | `AntTreeSelect` | <img src="resources/images/components/ant-tree-select-light.png" width="360"> | <img src="resources/images/components/ant-tree-select-dark.png" width="360"> |
| 数据录入 | `AntUpload` | <img src="resources/images/components/ant-upload-light.png" width="360"> | <img src="resources/images/components/ant-upload-dark.png" width="360"> |
| 数据展示 | `AntAvatar` | <img src="resources/images/components/ant-avatar-light.png" width="360"> | <img src="resources/images/components/ant-avatar-dark.png" width="360"> |
| 数据展示 | `AntBadge` | <img src="resources/images/components/ant-badge-light.png" width="360"> | <img src="resources/images/components/ant-badge-dark.png" width="360"> |
| 数据展示 | `AntCalendar` | <img src="resources/images/components/ant-calendar-light.png" width="360"> | <img src="resources/images/components/ant-calendar-dark.png" width="360"> |
| 数据展示 | `AntCard` | <img src="resources/images/components/ant-card-light.png" width="360"> | <img src="resources/images/components/ant-card-dark.png" width="360"> |
| 数据展示 | `AntCarousel` | <img src="resources/images/components/ant-carousel-light.png" width="360"> | <img src="resources/images/components/ant-carousel-dark.png" width="360"> |
| 数据展示 | `AntCollapse` | <img src="resources/images/components/ant-collapse-light.png" width="360"> | <img src="resources/images/components/ant-collapse-dark.png" width="360"> |
| 数据展示 | `AntDescriptions` | <img src="resources/images/components/ant-descriptions-light.png" width="360"> | <img src="resources/images/components/ant-descriptions-dark.png" width="360"> |
| 数据展示 | `AntEmpty` | <img src="resources/images/components/ant-empty-light.png" width="360"> | <img src="resources/images/components/ant-empty-dark.png" width="360"> |
| 数据展示 | `AntImage` | <img src="resources/images/components/ant-image-light.png" width="360"> | <img src="resources/images/components/ant-image-dark.png" width="360"> |
| 数据展示 | `AntList` | <img src="resources/images/components/ant-list-light.png" width="360"> | <img src="resources/images/components/ant-list-dark.png" width="360"> |
| 数据展示 | `AntPopover` | <img src="resources/images/components/ant-popover-light.png" width="360"> | <img src="resources/images/components/ant-popover-dark.png" width="360"> |
| 数据展示 | `AntQRCode` | <img src="resources/images/components/ant-qr-code-light.png" width="360"> | <img src="resources/images/components/ant-qr-code-dark.png" width="360"> |
| 数据展示 | `AntSegmented` | <img src="resources/images/components/ant-segmented-light.png" width="360"> | <img src="resources/images/components/ant-segmented-dark.png" width="360"> |
| 数据展示 | `AntStatistic` | <img src="resources/images/components/ant-statistic-light.png" width="360"> | <img src="resources/images/components/ant-statistic-dark.png" width="360"> |
| 数据展示 | `AntTable` | <img src="resources/images/components/ant-table-light.png" width="360"> | <img src="resources/images/components/ant-table-dark.png" width="360"> |
| 数据展示 | `AntTabs` | <img src="resources/images/components/ant-tabs-light.png" width="360"> | <img src="resources/images/components/ant-tabs-dark.png" width="360"> |
| 数据展示 | `AntTag` | <img src="resources/images/components/ant-tag-light.png" width="360"> | <img src="resources/images/components/ant-tag-dark.png" width="360"> |
| 数据展示 | `AntTimeline` | <img src="resources/images/components/ant-timeline-light.png" width="360"> | <img src="resources/images/components/ant-timeline-dark.png" width="360"> |
| 数据展示 | `AntToolTip` | <img src="resources/images/components/ant-tooltip-light.png" width="360"> | <img src="resources/images/components/ant-tooltip-dark.png" width="360"> |
| 数据展示 | `AntTree` | <img src="resources/images/components/ant-tree-light.png" width="360"> | <img src="resources/images/components/ant-tree-dark.png" width="360"> |
| 反馈 | `AntAlert` | <img src="resources/images/components/ant-alert-light.png" width="360"> | <img src="resources/images/components/ant-alert-dark.png" width="360"> |
| 反馈 | `AntDrawer` | <img src="resources/images/components/ant-drawer-light.png" width="360"> | <img src="resources/images/components/ant-drawer-dark.png" width="360"> |
| 反馈 | `AntMessage` | <img src="resources/images/components/ant-message-light.png" width="360"> | <img src="resources/images/components/ant-message-dark.png" width="360"> |
| 反馈 | `AntModal` | <img src="resources/images/components/ant-modal-light.png" width="360"> | <img src="resources/images/components/ant-modal-dark.png" width="360"> |
| 反馈 | `AntNotification` | <img src="resources/images/components/ant-notification-light.png" width="360"> | <img src="resources/images/components/ant-notification-dark.png" width="360"> |
| 反馈 | `AntPopconfirm` | <img src="resources/images/components/ant-popconfirm-light.png" width="360"> | <img src="resources/images/components/ant-popconfirm-dark.png" width="360"> |
| 反馈 | `AntProgress` | <img src="resources/images/components/ant-progress-light.png" width="360"> | <img src="resources/images/components/ant-progress-dark.png" width="360"> |
| 反馈 | `AntResult` | <img src="resources/images/components/ant-result-light.png" width="360"> | <img src="resources/images/components/ant-result-dark.png" width="360"> |
| 反馈 | `AntSkeleton` | <img src="resources/images/components/ant-skeleton-light.png" width="360"> | <img src="resources/images/components/ant-skeleton-dark.png" width="360"> |
| 反馈 | `AntSpin` | <img src="resources/images/components/ant-spin-light.png" width="360"> | <img src="resources/images/components/ant-spin-dark.png" width="360"> |
| 反馈 | `AntTour` | <img src="resources/images/components/ant-tour-light.png" width="360"> | <img src="resources/images/components/ant-tour-dark.png" width="360"> |
| 反馈 | `AntWatermark` | <img src="resources/images/components/ant-watermark-light.png" width="360"> | <img src="resources/images/components/ant-watermark-dark.png" width="360"> |
| 其他 | `AntApp` | <img src="resources/images/components/ant-app-light.png" width="360"> | <img src="resources/images/components/ant-app-dark.png" width="360"> |
| 其他 | `AntConfigProvider` | <img src="resources/images/components/ant-config-provider-light.png" width="360"> | <img src="resources/images/components/ant-config-provider-dark.png" width="360"> |
| 其他 | `AntFloatButton` | <img src="resources/images/components/ant-float-button-light.png" width="360"> | <img src="resources/images/components/ant-float-button-dark.png" width="360"> |
| Qt / 桌面扩展 | `AntWindow` | <img src="resources/images/components/ant-window-light.png" width="360"> | <img src="resources/images/components/ant-window-dark.png" width="360"> |
| Qt / 桌面扩展 | `AntWidget` | <img src="resources/images/components/ant-widget-light.png" width="360"> | <img src="resources/images/components/ant-widget-dark.png" width="360"> |
| Qt / 桌面扩展 | `AntScrollArea` | <img src="resources/images/components/ant-scroll-area-light.png" width="360"> | <img src="resources/images/components/ant-scroll-area-dark.png" width="360"> |
| Qt / 桌面扩展 | `AntScrollBar` | <img src="resources/images/components/ant-scroll-bar-light.png" width="360"> | <img src="resources/images/components/ant-scroll-bar-dark.png" width="360"> |
| Qt / 桌面扩展 | `AntStatusBar` | <img src="resources/images/components/ant-status-bar-light.png" width="360"> | <img src="resources/images/components/ant-status-bar-dark.png" width="360"> |
| Qt / 桌面扩展 | `AntRibbon` | <img src="resources/images/components/ant-ribbon-light.png" width="360"> | <img src="resources/images/components/ant-ribbon-dark.png" width="360"> |
| Qt / 桌面扩展 | `AntMenuBar` | <img src="resources/images/components/ant-menu-bar-light.png" width="360"> | <img src="resources/images/components/ant-menu-bar-dark.png" width="360"> |
| Qt / 桌面扩展 | `AntToolBar` | <img src="resources/images/components/ant-tool-bar-light.png" width="360"> | <img src="resources/images/components/ant-tool-bar-dark.png" width="360"> |
| Qt / 桌面扩展 | `AntToolButton` | <img src="resources/images/components/ant-tool-button-light.png" width="360"> | <img src="resources/images/components/ant-tool-button-dark.png" width="360"> |
| Qt / 桌面扩展 | `AntDockWidget` | <img src="resources/images/components/ant-dock-widget-light.png" width="360"> | <img src="resources/images/components/ant-dock-widget-dark.png" width="360"> |
| Qt / 桌面扩展 | `AntPlainTextEdit` | <img src="resources/images/components/ant-plain-text-edit-light.png" width="360"> | <img src="resources/images/components/ant-plain-text-edit-dark.png" width="360"> |
| Qt / 桌面扩展 | `AntLog` | <img src="resources/images/components/ant-log-light.png" width="360"> | <img src="resources/images/components/ant-log-dark.png" width="360"> |
| Qt / 桌面扩展 | `AntNavItem` | <img src="resources/images/components/ant-nav-item-light.png" width="360"> | <img src="resources/images/components/ant-nav-item-dark.png" width="360"> |
| Qt / 桌面扩展 | `AntMasonry` | <img src="resources/images/components/ant-masonry-light.png" width="360"> | <img src="resources/images/components/ant-masonry-dark.png" width="360"> |

### 组件概览

- `AntButton`：五种类型、三种尺寸、三种形状、`loading / danger / ghost / block`
- `AntIcon`：`831` 个官方 SVG 图标、字符串名称 API、`Outlined / Filled / TwoTone`、旋转、spin、自定义路径
- `AntInput`：尺寸、状态、`addonBefore / addonAfter / allowClear / password`
- `AntInputNumber`：尺寸、状态、变体、前后缀、QDoubleSpinBox 风格小数/精度、小步进、显隐控制按钮
- `AntDescriptions`：标题、extra、列数、bordered、vertical、自定义值控件
- `AntForm`：`AntForm / AntFormItem`、横向/纵向/行内布局、标签对齐、必填标记、说明和校验提示
- `AntEmpty`：默认插画、`simple` 模式、描述文案、自定义插画尺寸和 extra action
- `AntDropdown`：`hover / click / contextMenu` 触发、placement、箭头、自动翻转
- `AntSteps`：水平/垂直布局、当前步骤、错误态、点击切换、标题/说明/副标题
- `AntSelect`：尺寸、状态、变体、`allowClear / loading / popup`、option 文本/数据管理
- `AntAlert`：`success / info / warning / error`、图标、描述、关闭、横幅、自定义 action
- `AntModal`：遮罩层、标题、正文、自定义内容、自定义 footer、确认/取消、居中或顶部偏移布局，以及不会在对话框边缘被裁切的柔和外阴影
- `AntResult`：状态图标（success / error / warning / info）、暗色透明图标背景、标题、描述、自定义 extra 操作区
- `AntList`：`header / footer / bordered / split / size`，`AntListItem` 支持 `Meta`（头像、标题、描述）、操作区、内部滚动和 QListWidget 风格文本/数据/选择 helper
- `AntStatistic`：数值展示、千分位分隔、前缀后缀、精度控制
- `AntPopover`：标题、正文、action、点击/悬停触发、placement、箭头
- `AntPopconfirm`：确认标题、说明、确认/取消按钮、禁用态、placement，以及箭头与弹层主体一体化绘制
- `AntSkeleton`：支持动态 `active` shimmer、头像占位、标题/段落配置、圆角风格以及 `loading` 切换真实内容
- `AntToolTip`：常用 `placement`、箭头、颜色、延迟显示、自动翻转，以及鼠标透明的被动提示显示
- `AntSlider`：横竖向、`reverse / dots / included`、Range、marks、拖动时在当前 handle 上方显示一体化箭头浮标且不阻挡指针输入
- `AntRibbon`：Page / Group 结构，支持大/小 action、嵌入 Ant/Qt 控件、折叠弹出模式和 `AntWindow` 顶部集成
- `AntSwitch`：`checked / loading / small / text`、点击 Wave 反馈
- `AntSpin`：`small / middle / large / percent / delay`、更平滑的高频动画
- `AntDatePicker` / `AntTimePicker`：自绘弹层选择器
- `AntMessage` / `AntNotification`：带浮层阴影、进入/退出动效、Message 点击透传到底层控件，以及 Notification loading 进度倒计时的全局反馈组件
- `AntCard` / `AntTag` / `AntBadge` / `AntAvatar`：常用展示组件
- `AntMenu` / `AntTabs` / `AntBreadcrumb` / `AntPagination`：导航组件；`AntPagination` 支持可输入页码的 quick jumper 跳页，`AntTabs` 提供 Tab 内容页布局 margins 归一化 helper
- `AntTable`：数据表格，支持列排序、行选择（复选框/单选框）、程序化选中、行 tooltip、分页、加载状态
- `AntTree`：树形控件，支持展开/收起、节点选择、复选框、连接线
- `AntUpload`：文件上传，支持文本列表/图片列表/图片卡片三种模式
- `AntCascader`：级联选择器，多列弹出面板，支持点击/悬停展开
- `AntTreeSelect`：树形选择器，下拉框内展示树形结构
- `AntRate`：评分组件，`count / value / allowHalf / allowClear / disabled / size`，hover 放大和选中星缩放动效，键盘左右箭头操作
- `AntWidget`：基础 QWidget 子类，自动处理主题切换
- `AntTypography`：主题感知文本组件，Title(H1-H5)/Text/Paragraph，支持类型/装饰/复制/省略/对齐策略/像素字号
- `AntWindow`：无边框窗口，自定义标题栏，置顶/主题/最小化/最大化/关闭按钮，Windows 11 Snap 支持，Windows 10/11 DWM 边框阴影，以及平滑主题切换遮罩动画
- `AntDrawer`：滑动面板，支持 Left/Right/Top/Bottom 四个方向、动画、遮罩层
- `AntStatusBar`：状态栏，左右项、分隔符、消息区、size grip
- `AntScrollBar`：自定义滚动条，8px 细滚动条、自动隐藏、无箭头按钮
- `AntSegmented`：分段控制器，选项块均衡分布，滑动指示器动画，支持图标/禁用/提示，并补齐完整视觉轨道点击命中
- `AntFloatButton`：浮动操作按钮，圆形/方形，Primary/Default，Group 展开/收起,BackTop 返回顶部，Badge
- `AntWatermark`：鼠标透明的水印叠加层，旋转文本平铺，多行内容，自定义字体/颜色/间距/偏移/角度
- `AntQRCode`：二维码展示，嵌入式 QR 生成器（无外部依赖），默认内容指向仓库地址，状态叠加层（过期/加载/已扫描），图标，无边框
- `AntAffix`：固钉工具，QObject 辅助类，监听滚动容器，自动吸附/解除，占位保持布局
- `AntAutoComplete`：自动完成输入，弹出建议列表，键盘导航
- `AntCalendar`：日历面板，Day/Month/Year 三态切换，日期选择
- `AntCarousel`：轮播图，自动播放，圆点指示器，滑动切换动效，点击翻页
- `AntCollapse`：折叠面板/手风琴，InOutCubic 展开动画，accordion 互斥模式
- `AntColorPicker`：内联颜色触发器，可显示文本，弹窗内提供 HS field + value slider + RGB/HSV 输入、预设/自定义颜色
- `AntImage`：图片展示，placeholder fallback，点击全屏预览
- `AntTransfer`：穿梭框，双列表滚动、顶部全选、批量转移
- `AntTour`：遮罩式分步引导，目标高亮，支持指定步骤启动，Prev/Next/Finish
- `AntMentions`：@提及输入，输入 @ 弹出建议
- `AntGrid` (Row/Col)：24 列栅格布局，span/offset/gutter
- `AntFlex`：弹性布局容器，gap/wrap/vertical
- `AntMasonry`：瀑布流布局，最短列优先
- `AntSplitter`：可拖拽分割面板，主题色手柄
- `AntAnchor`：滚动锚点导航，active 链接高亮
- `AntApp`：应用包裹器，message/modal/notification 上下文
- `AntConfigProvider`：主题/主色/字号/圆角全局配置
- `AntToolButton`：QToolButton + QProxyStyle，dropdown 箭头动画
- `AntMenuBar`：QMenuBar 主题化
- `AntToolBar`：QToolBar 主题化，浮动阴影
- `AntDockWidget`：可停靠面板，自定义标题栏，Win32 resize 边缘
- `AntScrollArea`：QScrollArea + AntScrollBar + QScroller 手势滚动
- `AntPlainTextEdit`：多行文本编辑器，3 种变体，TextArea 式右下角缩放柄，上下文菜单
- `AntLog`：5 级别彩色日志输出（Debug/Info/Success/Warning/Error），时间戳

## 使用示例

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

### 主题切换

```cpp
#include "core/AntTheme.h"

AntTheme::instance()->setThemeMode(Ant::ThemeMode::Dark);
```

目前主题切换会触发所有 `QProxyStyle` 组件的 `polish / updateGeometry / update`。`AntWindow` 内置主题按钮会用全窗口截图 overlay 和柔和揭示动画包裹这次重绘，让 Light/Dark 全局切换更连续。

## 开发指南与贡献

项目使用 `AGENTS.md` 作为 AI 协作规范与项目同步文档，记录：

- 已移植组件清单
- 视觉审查清单
- 当前架构约定
- 示例覆盖情况
- 构建与安装说明

逐控件视觉审查清单位于 `docs/visual-audit.md`。

添加新组件时，推荐遵循以下流程：

1. 阅读 [`ant-design/ant-design`](https://github.com/ant-design/ant-design) 仓库 `components/<component>/` 的 API 与样式
2. 新增 `src/widgets/Ant<Name>.h/.cpp`
3. 如需样式解耦，新增 `src/styles/Ant<Name>Style.h/.cpp`
4. 在 `examples/ExampleWindow.cpp` 中补齐展示页
5. 更新 `AGENTS.md` 与 `README.md`

欢迎提交 Issue 和 PR。

## Star History

<a href="https://www.star-history.com/?repos=sorrowfeng%2Fqt-ant-design&type=date&legend=top-left">
 <picture>
   <source media="(prefers-color-scheme: dark)" srcset="https://api.star-history.com/chart?repos=sorrowfeng/qt-ant-design&type=date&theme=dark&legend=top-left" />
   <source media="(prefers-color-scheme: light)" srcset="https://api.star-history.com/chart?repos=sorrowfeng/qt-ant-design&type=date&legend=top-left" />
   <img alt="Star History Chart" src="https://api.star-history.com/chart?repos=sorrowfeng/qt-ant-design&type=date&legend=top-left" />
 </picture>
</a>

## 致谢

- 感谢 Ant Design 提供设计系统、组件规范与 token 基础：[ant-design/ant-design](https://github.com/ant-design/ant-design)
- 感谢 ElaWidgetTools 提供 Qt 控件参考：[Liniyous/ElaWidgetTools](https://github.com/Liniyous/ElaWidgetTools)
- 开发说明：本项目 90% 以上由 Codex GPT-5.5 开发，其余由 Claude Code 与 Mimo v2.5 Pro 协助完成。

## 许可证

MIT License
