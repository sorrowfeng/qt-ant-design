# qt-ant-design

[English](README.md) | 简体中文

`qt-ant-design` 是一个基于 Qt6 Widgets 的 C++ 组件库，目标是将 Ant Design 设计系统移植到原生桌面组件中。

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

- 基于 Qt6 Widgets，轻量、易集成，可直接作为静态库接入现有项目
- 内置 Design Token 系统，支持亮色 / 暗色主题实时切换
- 当前已移植 `82` 个公开组件（Ant Design 标准组件 `70 / 70` 全覆盖，另含 `12` 个 Qt / 桌面扩展组件）
- 当前 `62` 个组件使用 `QProxyStyle` 架构绘制
- 示例程序当前覆盖 `82 / 82` 个公开组件，另有独立 Ant Design 首页风格 `Showcase`
- `AntIcon` 已内置 `831` 个来自 `@ant-design/icons-svg@4.4.2` 的官方 SVG 资源
- 可对比的标准组件已在视觉审计矩阵中标记为 `Pass`，Qt-only 桌面扩展标记为 `Local Pass`
- 代码结构清晰，`core / styles / widgets / examples` 分层明确，便于扩展

## 当前状态

- 状态总览：[docs/project-status.md](docs/project-status.md)
- 视觉审计矩阵：[docs/visual-audit.md](docs/visual-audit.md)
- 官方图标清单：[docs/ant-design-icons.md](docs/ant-design-icons.md)
- 最近一次 Debug 全量验证：`34 / 34` 个 CTest 目标通过（`2026-05-01`）

## 最近 Ant Design 对齐更新

2026-04-30 的交互与动效对齐批次补齐了多处用户可见细节：

- 弹层反馈：`AntPopover`、`AntMessage`、`AntNotification` 的悬停/关闭行为更稳定，阴影层级更清晰，并补齐了按 placement 进入/退出的动效。
- 动效表现：`AntCarousel`、`AntTabs`、`AntSkeleton`、`AntSpin`、`AntInputNumber`、`AntSwitch` 和 loading button 的方向、节奏、状态反馈更贴近 Ant Design。
- 数据交互：`AntTransfer` 支持正常滚动和顶部全选，`AntTable` 表头排序点击会真正重排行数据。
- 输入反馈：`AntPlainTextEdit` 支持 TextArea 式右下角拖拽缩放，`AntSlider` 拖动时显示数值浮标。

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

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

add_subdirectory(third_party/qt-ant-design)

add_executable(my-qt-app main.cpp)
target_link_libraries(my-qt-app PRIVATE Qt6::Core Qt6::Widgets qt-ant-design)
```

### 方式二：安装并使用 CMake package

```bash
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=/path/to/install
cmake --build build --config Release
cmake --install build --config Release
```

然后让你的消费项目指向安装前缀：

```cmake
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Svg)
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
cmake .. -DCMAKE_PREFIX_PATH=/path/to/Qt6
cmake --build .
```

在 Windows / 多配置生成器下，推荐使用：

```powershell
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=D:/Project/GitProject/qt-ant-design/install
cmake --build build --config Debug
cmake --install build --config Debug
.\install\bin\qt-ant-design-example.exe
```

### 第一个 `AntButton`

```cpp
#include <QApplication>
#include <QVBoxLayout>
#include <QWidget>

#include "widgets/AntButton.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

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

当前已实现公开组件总数：`82`

`src/widgets` 当前包含 `83` 个 `Ant*.h` 头文件，其中 `AntSelectPopup` 是内部弹层 helper，不计入公开组件。

Ant Design 标准组件按 [`ant-design/ant-design`](https://github.com/ant-design/ant-design) 仓库 `components/` 顶层目录统计，并将 `row / col` 并入 `grid`、`back-top` 并入 `float-button`、`qrcode` 视为 `qr-code` 兼容别名，因此当前标准组件口径为 `70`。

| 分类 | 组件 | 当前绘制方式 |
| --- | --- | --- |
| 通用 | `AntButton` `AntFloatButton` `AntIcon` `AntTypography` | `QProxyStyle` |
| 导航 | `AntAnchor` `AntBreadcrumb` `AntDropdown` `AntMenu` `AntPagination` `AntSteps` `AntTabs` | 混合（`QProxyStyle` / 自绘） |
| 数据录入 | `AntAutoComplete` `AntCascader` `AntCheckbox` `AntColorPicker` `AntDatePicker` `AntDescriptions` `AntForm` `AntInput` `AntInputNumber` `AntMentions` `AntRadio` `AntRate` `AntSegmented` `AntSelect` `AntSlider` `AntSwitch` `AntTimePicker` `AntTransfer` `AntTreeSelect` `AntUpload` | 混合（`QProxyStyle` / 自绘） |
| 反馈 | `AntAlert` `AntDrawer` `AntMessage` `AntModal` `AntNotification` `AntPopconfirm` `AntPopover` `AntProgress` `AntResult` `AntSkeleton` `AntSpin` `AntTooltip` `AntTour` `AntWatermark` | 混合（`QProxyStyle` / 自绘） |
| 数据展示 | `AntAvatar` `AntBadge` `AntCalendar` `AntCard` `AntCarousel` `AntCollapse` `AntEmpty` `AntImage` `AntList` `AntQRCode` `AntStatistic` `AntTable` `AntTag` `AntTimeline` `AntTree` | 混合（`QProxyStyle` / 自绘） |
| 布局与其他 | `AntAffix` `AntApp` `AntConfigProvider` `AntDivider` `AntFlex` `AntGrid` `AntLayout` `AntMasonry` `AntSpace` `AntSplitter` `AntWidget` `AntWindow` | 混合（`QProxyStyle` / 自绘 / QObject 工具） |
| Qt / 桌面扩展 | `AntDockWidget` `AntLog` `AntMenuBar` `AntPlainTextEdit` `AntScrollArea` `AntScrollBar` `AntStatusBar` `AntToolBar` `AntToolButton` | 混合（`QProxyStyle` / 自绘） |

### 组件概览

- `AntButton`：五种类型、三种尺寸、三种形状、`loading / danger / ghost / block`
- `AntIcon`：`831` 个官方 SVG 图标、字符串名称 API、`Outlined / Filled / TwoTone`、旋转、spin、自定义路径
- `AntInput`：尺寸、状态、`addonBefore / addonAfter / allowClear / password`
- `AntInputNumber`：尺寸、状态、变体、前后缀、精度、小步进、显隐控制按钮
- `AntDescriptions`：标题、extra、列数、bordered、vertical、自定义值控件
- `AntForm`：`AntForm / AntFormItem`、横向/纵向/行内布局、标签对齐、必填标记、说明和校验提示
- `AntEmpty`：默认插画、`simple` 模式、描述文案、自定义插画尺寸和 extra action
- `AntDropdown`：`hover / click / contextMenu` 触发、placement、箭头、自动翻转
- `AntSteps`：水平/垂直布局、当前步骤、错误态、点击切换、标题/说明/副标题
- `AntSelect`：尺寸、状态、变体、`allowClear / loading / popup`
- `AntAlert`：`success / info / warning / error`、图标、描述、关闭、横幅、自定义 action
- `AntModal`：遮罩层、标题、正文、自定义内容、自定义 footer、确认/取消、居中或顶部偏移布局
- `AntResult`：状态图标（success / error / warning / info）、标题、描述、自定义 extra 操作区
- `AntList`：`header / footer / bordered / split / size`，`AntListItem` 支持 `Meta`（头像、标题、描述）和操作区
- `AntStatistic`：数值展示、千分位分隔、前缀后缀、精度控制
- `AntPopover`：标题、正文、action、点击/悬停触发、placement、箭头
- `AntPopconfirm`：确认标题、说明、确认/取消按钮、禁用态、placement
- `AntSkeleton`：支持动态 `active` shimmer、头像占位、标题/段落配置、圆角风格以及 `loading` 切换真实内容
- `AntTooltip`：常用 `placement`、箭头、颜色、延迟显示、自动翻转
- `AntSlider`：横竖向、`reverse / dots / included`、拖动数值浮标
- `AntSwitch`：`checked / loading / small / text`、点击 Wave 反馈
- `AntSpin`：`small / middle / large / percent / delay`、更平滑的高频动画
- `AntDatePicker` / `AntTimePicker`：自绘弹层选择器
- `AntMessage` / `AntNotification`：带浮层阴影和进入/退出动效的全局反馈组件
- `AntCard` / `AntTag` / `AntBadge` / `AntAvatar`：常用展示组件
- `AntMenu` / `AntTabs` / `AntBreadcrumb` / `AntPagination`：导航组件
- `AntTable`：数据表格，支持列排序、行选择（复选框/单选框）、分页、加载状态
- `AntTree`：树形控件，支持展开/收起、节点选择、复选框、连接线
- `AntUpload`：文件上传，支持文本列表/图片列表/图片卡片三种模式
- `AntCascader`：级联选择器，多列弹出面板，支持点击/悬停展开
- `AntTreeSelect`：树形选择器，下拉框内展示树形结构
- `AntRate`：评分组件，`count / value / allowHalf / allowClear / disabled / size`，hover 放大效果，键盘左右箭头操作
- `AntWidget`：基础 QWidget 子类，自动处理主题切换
- `AntTypography`：主题感知文本组件，Title(H1-H5)/Text/Paragraph，支持类型/装饰/复制/省略
- `AntWindow`：无边框窗口，自定义标题栏、拖拽、最小化/最大化/关闭按钮
- `AntDrawer`：滑动面板，支持 Left/Right/Top/Bottom 四个方向、动画、遮罩层
- `AntStatusBar`：状态栏，左右项、分隔符、消息区、size grip
- `AntScrollBar`：自定义滚动条，8px 细滚动条、自动隐藏、无箭头按钮
- `AntSegmented`：分段控制器，选项块均衡分布，滑动指示器动画，支持图标/禁用/提示
- `AntFloatButton`：浮动操作按钮，圆形/方形，Primary/Default，Group 展开/收起,BackTop 返回顶部，Badge
- `AntWatermark`：水印叠加层，旋转文本平铺，多行内容，自定义字体/颜色/间距/偏移/角度
- `AntQRCode`：二维码展示，嵌入式 QR 生成器（无外部依赖），状态叠加层（过期/加载/已扫描），图标，无边框
- `AntAffix`：固钉工具，QObject 辅助类，监听滚动容器，自动吸附/解除，占位保持布局
- `AntAutoComplete`：自动完成输入，弹出建议列表，键盘导航
- `AntCalendar`：日历面板，Day/Month/Year 三态切换，日期选择
- `AntCarousel`：轮播图，自动播放，圆点指示器，滑动切换动效，点击翻页
- `AntCollapse`：折叠面板/手风琴，InOutCubic 展开动画，accordion 互斥模式
- `AntColorPicker`：内联颜色触发器，可显示文本，弹窗内提供 HS field + value slider + RGB/HSV 输入、预设/自定义颜色
- `AntImage`：图片展示，placeholder fallback，点击全屏预览
- `AntTransfer`：穿梭框，双列表滚动、顶部全选、批量转移
- `AntTour`：遮罩式分步引导，目标高亮，Prev/Next/Finish
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

目前主题切换会触发所有 `QProxyStyle` 组件的 `polish / updateGeometry / update`。

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
2. 需要时参考 [`Liniyous/ElaWidgetTools`](https://github.com/Liniyous/ElaWidgetTools) 中相似 Qt 控件的实现方式
3. 新增 `src/widgets/Ant<Name>.h/.cpp`
4. 如需样式解耦，新增 `src/styles/Ant<Name>Style.h/.cpp`
5. 在 `examples/ExampleWindow.cpp` 中补齐展示页
6. 更新 `AGENTS.md` 与 `README.md`

欢迎提交 Issue 和 PR。

## 致谢

- 感谢 Ant Design 提供设计系统、组件规范与 token 基础：[ant-design/ant-design](https://github.com/ant-design/ant-design)
- 感谢 ElaWidgetTools 提供 Qt 控件参考：[Liniyous/ElaWidgetTools](https://github.com/Liniyous/ElaWidgetTools)
- 开发说明：本项目 90% 以上由 Codex GPT-5.5 开发，其余由 Claude Code 与 Mimo v2.5 Pro 协助完成。

## 许可证

MIT License
