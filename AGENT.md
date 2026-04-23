# qt-ant-design Agent Notes

## 项目定位

`qt-ant-design` 是一个基于 Qt6 Widgets 的 C++ 组件库，目标是以 `QPainter` 手绘方式复刻 Ant Design 设计系统，并逐步将复杂控件的主绘制链路迁移到 `QProxyStyle` 架构。

当前仓库输出：

- 静态库 `qt-ant-design`
- 示例程序 `qt-ant-design-example`
- 可安装头文件、CMake targets 和 Windows 下的运行依赖

## 参考仓库

- 绘制实现参考：`D:\Project\GitProject\qt-ant-design\submodules\ElaWidgetTools`
- 设计规范参考：`D:\Project\GitProject\qt-ant-design\submodules\ant-design`

当前两个参考仓库都以 Git submodule 方式维护在项目内的 `submodules/` 目录下，便于版本对齐和协同开发。

重点参考内容：

- `ElaTheme` / 自绘控件的状态管理、动画、主题切换
- Ant Design 的 token、组件状态、尺寸、交互和视觉层级

## 本次同步

- 同步日期：`2026-04-23`
- 已实现组件总数：`22`
- 已迁移至 `QProxyStyle` 的组件数：`8`
- 仍使用 `paintEvent` 的组件数：`14`
- 示例程序覆盖：`22 / 22`，当前所有已实现组件均已在 `examples/ExampleWindow.cpp` 中展示

## 近期更新摘要

根据最近 20 条提交记录，近期主要改动如下：

- 新增组件：
  - `AntMenu`
  - `AntTabs`
  - `AntBreadcrumb`
  - `AntPagination`
  - `AntTag`
  - `AntBadge`
  - `AntAvatar`
  - `AntDivider`
- 文档与工程整理：
  - 新增并完善 `README.md`
  - 补充开发说明文档
  - 将 `AntButtonStyle` 统一归档到 `src/styles/`
- 架构迁移到 `QProxyStyle`：
  - `AntButton`
  - `AntInput`
  - `AntCheckbox`
  - `AntRadio`
  - `AntSwitch`
  - `AntSlider`
  - `AntSelect`
  - `AntSpin`

## 当前组件状态

### 通用

| 组件 | Ant Design 对应目录 | 绘制方式 | 示例覆盖 | 说明 |
| --- | --- | --- | --- | --- |
| `AntButton` | `button` | `QProxyStyle` | 是 | 已迁移至 `src/styles/AntButtonStyle` |

### 导航

| 组件 | Ant Design 对应目录 | 绘制方式 | 示例覆盖 | 说明 |
| --- | --- | --- | --- | --- |
| `AntBreadcrumb` | `breadcrumb` | `paintEvent` | 是 | 路径项、分隔符、禁用项 |
| `AntMenu` | `menu` | `paintEvent` | 是 | `vertical / horizontal / inline`、明暗主题 |
| `AntPagination` | `pagination` | `paintEvent` | 是 | `simple / showQuickJumper / showSizeChanger` |
| `AntTabs` | `tabs` | `paintEvent` | 是 | `line / card / editable-card` |

### 数据录入

| 组件 | Ant Design 对应目录 | 绘制方式 | 示例覆盖 | 说明 |
| --- | --- | --- | --- | --- |
| `AntCheckbox` | `checkbox` | `QProxyStyle` | 是 | 已迁移至 `src/styles/AntCheckboxStyle` |
| `AntDatePicker` | `date-picker` | `paintEvent` | 是 | 自绘日期弹层 |
| `AntInput` | `input` | `QProxyStyle` | 是 | 已迁移至 `src/styles/AntInputStyle` |
| `AntRadio` | `radio` | `QProxyStyle` | 是 | 已迁移至 `src/styles/AntRadioStyle` |
| `AntSelect` | `select` | `QProxyStyle` | 是 | 主控件迁移到 `src/styles/AntSelectStyle`，popup row 保持自绘 |
| `AntSlider` | `slider` | `QProxyStyle` | 是 | 已迁移至 `src/styles/AntSliderStyle` |
| `AntSwitch` | `switch` | `QProxyStyle` | 是 | 已迁移至 `src/styles/AntSwitchStyle` |
| `AntTimePicker` | `time-picker` | `paintEvent` | 是 | 自绘时间弹层 |

### 反馈

| 组件 | Ant Design 对应目录 | 绘制方式 | 示例覆盖 | 说明 |
| --- | --- | --- | --- | --- |
| `AntMessage` | `message` | `paintEvent` | 是 | `Qt::ToolTip` 浮层消息 |
| `AntNotification` | `notification` | `paintEvent` | 是 | 多 placement 通知 |
| `AntProgress` | `progress` | `paintEvent` | 是 | `line / circle / dashboard` |
| `AntSpin` | `spin` | `QProxyStyle` | 是 | 已迁移至 `src/styles/AntSpinStyle` |

### 数据展示

| 组件 | Ant Design 对应目录 | 绘制方式 | 示例覆盖 | 说明 |
| --- | --- | --- | --- | --- |
| `AntAvatar` | `avatar` | `paintEvent` | 是 | 文本、图标、图片头像 |
| `AntBadge` | `badge` | `paintEvent` | 是 | `count / dot / status / processing` |
| `AntCard` | `card` | `paintEvent` | 是 | 封面、额外区、操作区、loading |
| `AntTag` | `tag` | `paintEvent` | 是 | `closable / checkable / variant` |

### 布局与其他

| 组件 | Ant Design 对应目录 | 绘制方式 | 示例覆盖 | 说明 |
| --- | --- | --- | --- | --- |
| `AntDivider` | `divider` | `paintEvent` | 是 | 水平/垂直、带标题、虚线/点线 |

## 待移植组件

以下清单基于 `submodules/ant-design/components/` 扫描，并扣除当前已实现组件后整理。仅保留与 Qt Widgets 组件库相关度较高的部分，按优先级排序。

### 高优先级

- [ ] `AntIcon`
- [ ] `AntInputNumber`
- [ ] `AntForm`
- [ ] `AntModal`
- [ ] `AntTooltip`
- [ ] `AntDropdown`
- [ ] `AntPopover`
- [ ] `AntPopconfirm`
- [ ] `AntAlert`
- [ ] `AntSkeleton`

### 中优先级

- [ ] `AntEmpty`
- [ ] `AntSteps`
- [ ] `AntTimeline`
- [ ] `AntDescriptions`
- [ ] `AntList`
- [ ] `AntStatistic`
- [ ] `AntResult`
- [ ] `AntSpace`
- [ ] `AntLayout`
- [ ] `AntTypography`

### 后续扩展

- [ ] `AntTable`（复杂，稍后处理）
- [ ] `AntTree`（复杂，稍后处理）
- [ ] `AntTreeSelect`
- [ ] `AntCalendar`
- [ ] `AntUpload`
- [ ] `AntDrawer`
- [ ] `AntImage`
- [ ] `AntCascader`
- [ ] `AntAutoComplete`
- [ ] `AntMentions`
- [ ] `AntTransfer`
- [ ] `AntRate`
- [ ] `AntSegmented`
- [ ] `AntSplitter`
- [ ] `AntColorPicker`
- [ ] `AntQRCode`
- [ ] `AntCarousel`
- [ ] `AntTour`
- [ ] `AntWatermark`
- [ ] `AntAffix`
- [ ] `AntAnchor`
- [ ] `AntFloatButton`

## 开发规范

- 仅支持 Qt6，CMake 使用：
  - `find_package(Qt6 REQUIRED COMPONENTS Core Widgets)`
- C++ 标准为 `C++17`
- 所有核心视觉值优先从 `AntTheme` / `AntPalette` 获取，不直接散落硬编码
- 组件公共枚举与通用类型统一放在 `src/core/AntTypes.h`
- 新增或重构组件时，优先采用 `QProxyStyle` 架构：
  - 组件类负责公开 API、状态维护、信号、子控件管理
  - `Ant[Component]Style` 负责 `drawPrimitive` / `drawControl` / `drawComplexControl` / `sizeFromContents`
- 所有 `Ant[Component]Style` 文件统一放在 `src/styles/`
- 组件源文件引用 Style 时使用：
  - `#include "../styles/Ant[Component]Style.h"`
- CMake 安装时，公开 Style 头文件需安装到：
  - `install/include/qt-ant-design/styles/`
- 已迁移到 `QProxyStyle` 的组件，应在构造函数中安装独立 Style，并在主题切换时触发：
  - `polish`
  - `updateGeometry`
  - `update`
- 尚未迁移的组件可继续使用 `paintEvent`，但新开发优先按 `QProxyStyle` 设计
- 主题切换统一监听：
  - `AntTheme::themeChanged`
  - 或 `AntTheme::themeModeChanged`
- 每次新增组件或架构迁移后，必须同步更新：
  - `AGENT.md`
  - `README.md`
  - `examples/ExampleWindow.cpp`

## 示例程序

当前 `examples/ExampleWindow.cpp` 已覆盖全部 22 个已实现组件，左侧导航与右侧页面一一对应，当前没有“已实现但未展示”的组件。

当前示例页包括：

- `Button`
- `Breadcrumb`
- `Checkbox`
- `DatePicker`
- `Input`
- `Message`
- `Menu`
- `Tabs`
- `Badge`
- `Avatar`
- `Tag`
- `Notification`
- `Pagination`
- `Progress`
- `Radio`
- `Select`
- `Slider`
- `Spin`
- `Switch`
- `TimePicker`
- `Card`
- `Divider`

## 构建与安装

```powershell
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=D:/Project/GitProject/qt-ant-design/install
cmake --build build --config Debug
cmake --install build --config Debug
.\install\bin\qt-ant-design-example.exe
```

当前安装产物位于：

- `install/bin/qt-ant-design-example.exe`
- `install/lib/qt-ant-design.lib`
- `install/include/qt-ant-design/`
- `install/lib/cmake/qt-ant-design/`
