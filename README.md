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
- 当前已移植 `66` 个核心组件
- 全部 `62` 个组件均已迁移到 `QProxyStyle` 架构（另 3 个为容器/对话框，1 个为 QObject 工具类）
- 示例程序已覆盖全部 `66` 个已实现组件
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

当前已实现组件总数：`66`

| 分类 | 组件 | 当前绘制方式 |
| --- | --- | --- |
| 通用 | `AntButton` `AntFloatButton` `AntIcon` | `QProxyStyle` |
| 导航 | `AntBreadcrumb` `AntDropdown` `AntMenu` `AntPagination` `AntSteps` `AntTabs` | `QProxyStyle` |
| 数据录入 | `AntCheckbox` `AntDatePicker` `AntDescriptions` `AntForm` `AntInput` `AntInputNumber` `AntRadio` `AntRate` `AntSegmented` `AntSelect` `AntSlider` `AntSwitch` `AntTimePicker` `AntUpload` `AntCascader` `AntTreeSelect` | `QProxyStyle` |
| 反馈 | `AntAlert` `AntMessage` `AntModal` `AntNotification` `AntPopconfirm` `AntPopover` `AntProgress` `AntSpin` `AntTooltip` `AntResult` `AntWatermark` | `QProxyStyle` |
| 数据展示 | `AntAvatar` `AntBadge` `AntCard` `AntEmpty` `AntList` `AntQRCode` `AntSkeleton` `AntStatistic` `AntTag` `AntTable` `AntTree` | `QProxyStyle` |
| 布局与其他 | `AntAffix` `AntDivider` `AntSpace` `AntLayout` `AntTimeline` `AntTypography` `AntWidget` `AntWindow` `AntDrawer` `AntStatusBar` `AntScrollBar` | `QProxyStyle` |

### 组件概览

- `AntButton`：五种类型、三种尺寸、三种形状、`loading / danger / ghost / block`
- `AntIcon`：`Outlined / Filled / TwoTone`、旋转、spin、自定义路径
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
- `AntSkeleton`：支持 `active` shimmer、头像占位、标题/段落配置、圆角风格以及 `loading` 切换真实内容
- `AntTooltip`：常用 `placement`、箭头、颜色、延迟显示、自动翻转
- `AntSlider`：横竖向、`reverse / dots / included`
- `AntSwitch`：`checked / loading / small / text`
- `AntSpin`：`small / middle / large / percent / delay`
- `AntDatePicker` / `AntTimePicker`：自绘弹层选择器
- `AntMessage` / `AntNotification`：全局反馈组件
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
- `AntFloatButton`：浮动操作按钮，圆形/方形，Primary/Default，Group 展开/收起，BackTop 返回顶部，Badge
- `AntWatermark`：水印叠加层，旋转文本平铺，多行内容，自定义字体/颜色/间距/偏移/角度
- `AntQRCode`：二维码展示，嵌入式 QR 生成器（无外部依赖），状态叠加层（过期/加载/已扫描），图标，无边框
- `AntAffix`：固钉工具，QObject 辅助类，监听滚动容器，自动吸附/解除，占位保持布局

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

目前主题切换会触发所有 `QProxyStyle` 组件的 `polish / updateGeometry / update`。

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

（暂无）

后续还会继续推进：

- `AntCalendar`（日历）
- `AntImage`（图片）
- `AntCarousel`（轮播图）
- `AntColorPicker`（颜色选择器）
- `AntTransfer`（穿梭框）
- `AntAutoComplete`（自动完成）
- `AntTour`（漫游式引导）
- `AntAnchor`（锚点）
- `AntSplitter`（分割面板）
- `AntMentions`（提及）

## 致谢

- 感谢 Ant Design 团队提供完整的设计系统、组件规范与 token 体系
- 感谢 ElaWidgetTools 项目提供 Qt 主题和手绘控件实现灵感

## 许可证

MIT License
