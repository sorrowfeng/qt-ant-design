# qt-ant-design

`qt-ant-design` 是一个基于 Qt6 Widgets 的 C++ 组件库，目标是将 Ant Design 设计系统移植到原生桌面组件中。

项目强调：

- 亮暗主题动态切换
- 尽可能贴近 Ant Design 的交互与状态表现
- 使用 `QPainter` / `QProxyStyle` 构建可维护的桌面绘制体系

> 截图预留：后续会补充示例程序的亮色 / 暗色界面截图与组件画廊。

## 特性

- 基于 Qt6 Widgets，轻量、易集成，可直接作为静态库接入现有项目
- 内置 Design Token 系统，支持亮色 / 暗色主题实时切换
- 当前已移植 `25` 个核心组件
- 其中 `9` 个组件已经迁移到 `QProxyStyle` 架构，剩余组件仍采用 `paintEvent`，将继续逐步迁移
- 示例程序已覆盖全部 `25` 个已实现组件
- 代码结构清晰，`core / styles / widgets / examples` 分层明确，便于扩展

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

### 方式二：直接编译并链接静态库

```bash
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=/path/to/install
cmake --build build --config Release
cmake --install build --config Release
```

然后在你的项目中引用安装目录：

```cmake
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

add_executable(my-qt-app main.cpp)
target_include_directories(my-qt-app PRIVATE /path/to/install/include/qt-ant-design)
target_link_libraries(my-qt-app PRIVATE
    Qt6::Core
    Qt6::Widgets
    /path/to/install/lib/qt-ant-design.lib
)
```

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

当前已实现组件总数：`25`

| 分类 | 组件 | 当前绘制方式 |
| --- | --- | --- |
| 通用 | `AntButton` `AntIcon` | 混合（`QProxyStyle` + `paintEvent`） |
| 导航 | `AntBreadcrumb` `AntMenu` `AntPagination` `AntTabs` | `paintEvent` |
| 数据录入 | `AntCheckbox` `AntDatePicker` `AntInput` `AntInputNumber` `AntRadio` `AntSelect` `AntSlider` `AntSwitch` `AntTimePicker` | 混合（`QProxyStyle` + `paintEvent`） |
| 反馈 | `AntAlert` `AntMessage` `AntNotification` `AntProgress` `AntSpin` | 混合（`QProxyStyle` + `paintEvent`） |
| 数据展示 | `AntAvatar` `AntBadge` `AntCard` `AntTag` | `paintEvent` |
| 布局与其他 | `AntDivider` | `paintEvent` |

### 组件概览

- `AntButton`：五种类型、三种尺寸、三种形状、`loading / danger / ghost / block`
- `AntIcon`：`Outlined / Filled / TwoTone`、旋转、spin、自定义路径
- `AntInput`：尺寸、状态、`addonBefore / addonAfter / allowClear / password`
- `AntInputNumber`：尺寸、状态、变体、前后缀、精度、小步进、显隐控制按钮
- `AntSelect`：尺寸、状态、变体、`allowClear / loading / popup`
- `AntAlert`：`success / info / warning / error`、图标、描述、关闭、横幅、自定义 action
- `AntSlider`：横竖向、`reverse / dots / included`
- `AntSwitch`：`checked / loading / small / text`
- `AntSpin`：`small / middle / large / percent / delay`
- `AntDatePicker` / `AntTimePicker`：自绘弹层选择器
- `AntMessage` / `AntNotification`：全局反馈组件
- `AntCard` / `AntTag` / `AntBadge` / `AntAvatar`：常用展示组件
- `AntMenu` / `AntTabs` / `AntBreadcrumb` / `AntPagination`：导航组件

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
#include <QLabel>

#include "widgets/AntCard.h"

auto* card = new AntCard("User Profile");
card->setExtra("More");
card->setHoverable(true);
card->bodyLayout()->addWidget(new QLabel("Card content"));
```

### 主题切换

```cpp
#include "core/AntTheme.h"

AntTheme::instance()->setThemeMode(Ant::ThemeMode::Dark);
```

目前主题切换会触发已迁移 `QProxyStyle` 组件的 `polish / updateGeometry / update`，以及传统 `paintEvent` 组件的重绘。

## 开发指南与贡献

项目使用 `AGENT.md` 作为 AI 协作规范与项目同步文档，记录：

- 已移植组件清单
- 待移植组件清单
- 当前架构约定
- 示例覆盖情况
- 构建与安装说明

添加新组件时，推荐遵循以下流程：

1. 阅读 `submodules/ant-design/components/<component>/` 的 API 与样式
2. 查阅 ElaWidgetTools 中相似控件的实现方式
3. 新增 `src/widgets/Ant<Name>.h/.cpp`
4. 如需样式解耦，新增 `src/styles/Ant<Name>Style.h/.cpp`
5. 在 `examples/ExampleWindow.cpp` 中补齐展示页
6. 更新 `AGENT.md` 与 `README.md`

欢迎提交 Issue 和 PR。

## 路线图

当前高优先级待开发组件：

1. `AntForm`
2. `AntModal`
3. `AntTooltip`
4. `AntDropdown`
5. `AntPopover`
6. `AntPopconfirm`
7. `AntSkeleton`

后续还会继续推进：

- `AntEmpty`
- `AntSpace`
- `AntLayout`
- `AntTable`
- `AntTree`

## 致谢

- 感谢 Ant Design 团队提供完整的设计系统、组件规范与 token 体系
- 感谢 ElaWidgetTools 项目提供 Qt 主题和手绘控件实现灵感

## 许可证

MIT License
