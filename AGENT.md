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

- 同步日期：`2026-04-24`
- 已实现组件总数：`52`
- 已迁移至 `QProxyStyle` 的组件数：`51`
- 仍使用 `paintEvent` 的组件数：`1`（AntWidget 为基础类，无需 Style）
- 示例程序覆盖：`50 / 50`，当前所有已实现组件均已在 `examples/ExampleWindow.cpp` 中展示
- 示例程序架构：`ExampleWindow` 继承 `AntWindow`，使用 `AntWidget` 构建布局，`AntTypography` 替代 `QLabel` 实现主题感知文本

## 近期更新摘要

根据最近 20 条提交记录，近期主要改动如下：

- 新增组件：
  - `AntWidget`（基础类，自动处理主题切换）
  - `AntTable`（数据表格，支持排序、选择、分页）
  - `AntTree`（树形控件，支持展开/收起、选择、复选框）
  - `AntUpload`（文件上传，支持文本/图片/卡片模式）
  - `AntCascader`（级联选择器，多列弹出）
  - `AntTreeSelect`（树形选择器，下拉树形结构）
  - `AntWindow`（无边框窗口，自定义标题栏、拖拽、最小化/最大化/关闭按钮）
  - `AntDrawer`（滑动面板，支持四个方向、动画、遮罩层）
  - `AntStatusBar`（状态栏，左右项、分隔符、消息区、size grip）
  - `AntScrollBar`（自定义滚动条，8px 细滚动条、自动隐藏、无箭头按钮）
  - `AntList`
  - `AntStatistic`
  - `AntResult`
  - `AntAlert`
  - `AntDropdown`
  - `AntDescriptions`
  - `AntEmpty`
  - `AntForm`
  - `AntIcon`
  - `AntInputNumber`
  - `AntModal`
  - `AntSkeleton`
  - `AntSteps`
  - `AntPopover`
  - `AntPopconfirm`
  - `AntTooltip`
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
  - 全部 42 个组件已完成迁移
- 示例程序重构：
  - `ExampleWindow` 改为继承 `AntWindow`，移除手动标题栏和鼠标拖拽逻辑
  - 使用 `AntWidget` 作为布局容器替代裸 `QWidget`
  - 使用 `AntTypography` 替代 `QLabel`，实现主题切换下的文字颜色自适应
  - 使用 `AntScrollBar` 替代原生 `QScrollBar`
  - 移除所有 `setStyleSheet` 调用（仅保留 Layout 演示页的区域背景色）

## 当前组件状态

### 通用

| 组件 | Ant Design 对应目录 | 绘制方式 | 示例覆盖 | 说明 |
| --- | --- | --- | --- | --- |
| `AntButton` | `button` | `QProxyStyle` | 是 | 已迁移至 `src/styles/AntButtonStyle` |
| `AntIcon` | `icon` | `QProxyStyle` | 是 | 已迁移至 `src/styles/AntIconStyle`；支持 `Outlined / Filled / TwoTone`、旋转、spin、自定义路径 |

### 导航

| 组件 | Ant Design 对应目录 | 绘制方式 | 示例覆盖 | 说明 |
| --- | --- | --- | --- | --- |
| `AntBreadcrumb` | `breadcrumb` | `QProxyStyle` | 是 | 已迁移至 `src/styles/AntBreadcrumbStyle`；路径项、分隔符、禁用项 |
| `AntDropdown` | `dropdown` | `QProxyStyle` | 是 | 已迁移至 `src/styles/AntDropdownStyle`；`hover / click / contextMenu`、placement、arrow、auto flip |
| `AntMenu` | `menu` | `QProxyStyle` | 是 | 已迁移至 `src/styles/AntMenuStyle`；`vertical / horizontal / inline`、明暗主题 |
| `AntPagination` | `pagination` | `QProxyStyle` | 是 | 已迁移至 `src/styles/AntPaginationStyle`；`simple / showQuickJumper / showSizeChanger` |
| `AntSteps` | `steps` | `QProxyStyle` | 是 | 已迁移至 `src/styles/AntStepsStyle`；水平/垂直、当前步骤、错误态、点击切换 |
| `AntTabs` | `tabs` | `QProxyStyle` | 是 | 已迁移至 `src/styles/AntTabsStyle`；`line / card / editable-card` |

### 数据录入

| 组件 | Ant Design 对应目录 | 绘制方式 | 示例覆盖 | 说明 |
| --- | --- | --- | --- | --- |
| `AntCheckbox` | `checkbox` | `QProxyStyle` | 是 | 已迁移至 `src/styles/AntCheckboxStyle` |
| `AntDatePicker` | `date-picker` | `QProxyStyle` | 是 | 已迁移至 `src/styles/AntDatePickerStyle`；自绘日期弹层 |
| `AntDescriptions` | `descriptions` | `QProxyStyle` | 是 | 已迁移至 `src/styles/AntDescriptionsStyle`；标题、extra、bordered、vertical、自定义值控件 |
| `AntForm` | `form` | `QProxyStyle` | 是 | 已迁移至 `src/styles/AntFormStyle`；`AntForm / AntFormItem`、横向/纵向/行内布局、说明和校验提示 |
| `AntInput` | `input` | `QProxyStyle` | 是 | 已迁移至 `src/styles/AntInputStyle` |
| `AntInputNumber` | `input-number` | `QProxyStyle` | 是 | 已迁移至 `src/styles/AntInputNumberStyle` |
| `AntRadio` | `radio` | `QProxyStyle` | 是 | 已迁移至 `src/styles/AntRadioStyle` |
| `AntSelect` | `select` | `QProxyStyle` | 是 | 主控件迁移到 `src/styles/AntSelectStyle`，popup row 保持自绘 |
| `AntSlider` | `slider` | `QProxyStyle` | 是 | 已迁移至 `src/styles/AntSliderStyle` |
| `AntSwitch` | `switch` | `QProxyStyle` | 是 | 已迁移至 `src/styles/AntSwitchStyle` |
| `AntTimePicker` | `time-picker` | `QProxyStyle` | 是 | 已迁移至 `src/styles/AntTimePickerStyle`；自绘时间弹层 |
| `AntUpload` | `upload` | `QProxyStyle` | 是 | 已迁移至 `src/styles/AntUploadStyle`；文本/图片/卡片三种模式，文件列表管理 |
| `AntCascader` | `cascader` | `QProxyStyle` | 是 | 已迁移至 `src/styles/AntCascaderStyle`；多列弹出面板，点击/悬停展开 |
| `AntTreeSelect` | `tree-select` | `QProxyStyle` | 是 | 已迁移至 `src/styles/AntTreeSelectStyle`；下拉树形结构，支持多选和搜索 |

### 反馈

| 组件 | Ant Design 对应目录 | 绘制方式 | 示例覆盖 | 说明 |
| --- | --- | --- | --- | --- |
| `AntMessage` | `message` | `QProxyStyle` | 是 | 已迁移至 `src/styles/AntMessageStyle`；`Qt::ToolTip` 浮层消息 |
| `AntModal` | `modal` | `QProxyStyle` | 是 | 已迁移至 `src/styles/AntModalStyle`；遮罩层、标题/正文、自定义内容、自定义 footer、确认/取消 |
| `AntNotification` | `notification` | `QProxyStyle` | 是 | 已迁移至 `src/styles/AntNotificationStyle`；多 placement 通知 |
| `AntProgress` | `progress` | `QProxyStyle` | 是 | 已迁移至 `src/styles/AntProgressStyle`；`line / circle / dashboard` |
| `AntSpin` | `spin` | `QProxyStyle` | 是 | 已迁移至 `src/styles/AntSpinStyle` |
| `AntAlert` | `alert` | `QProxyStyle` | 是 | 已迁移至 `src/styles/AntAlertStyle`；`type / icon / description / closable / banner / action` |
| `AntTooltip` | `tooltip` | `QProxyStyle` | 是 | 已迁移至 `src/styles/AntTooltipStyle`；`title / placement / color / arrow / delay / auto flip` |
| `AntPopover` | `popover` | `QProxyStyle` | 是 | 已迁移至 `src/styles/AntPopoverStyle`；`title / content / action / hover / click / placement` |
| `AntPopconfirm` | `popconfirm` | `QProxyStyle` | 是 | 已迁移至 `src/styles/AntPopconfirmStyle`；`title / description / ok / cancel / disabled / placement` |
| `AntResult` | `result` | `QProxyStyle` | 是 | 已迁移至 `src/styles/AntResultStyle`；`status / title / subTitle / extra / iconVisible` |

### 数据展示

| 组件 | Ant Design 对应目录 | 绘制方式 | 示例覆盖 | 说明 |
| --- | --- | --- | --- | --- |
| `AntAvatar` | `avatar` | `QProxyStyle` | 是 | 已迁移至 `src/styles/AntAvatarStyle`；文本、图标、图片头像 |
| `AntList` | `list` | `QProxyStyle` | 是 | 已迁移至 `src/styles/AntListStyle`；`header / footer / bordered / split / size / AntListItem / AntListItemMeta` |
| `AntStatistic` | `statistic` | `QProxyStyle` | 是 | 已迁移至 `src/styles/AntStatisticStyle`；`title / value / precision / prefix / suffix / groupSeparator` |
| `AntBadge` | `badge` | `QProxyStyle` | 是 | 已迁移至 `src/styles/AntBadgeStyle`；`count / dot / status / processing` |
| `AntCard` | `card` | `QProxyStyle` | 是 | 已迁移至 `src/styles/AntCardStyle`；封面、额外区、操作区、loading |
| `AntEmpty` | `empty` | `QProxyStyle` | 是 | 已迁移至 `src/styles/AntEmptyStyle`；默认插画、simple 模式、描述、自定义尺寸、extra action |
| `AntSkeleton` | `skeleton` | `QProxyStyle` | 是 | 已迁移至 `src/styles/AntSkeletonStyle`；`active / avatar / title / paragraph / round / loading` |
| `AntTag` | `tag` | `QProxyStyle` | 是 | 已迁移至 `src/styles/AntTagStyle`；`closable / checkable / variant` |
| `AntTable` | `table` | `QProxyStyle` | 是 | 已迁移至 `src/styles/AntTableStyle`；列排序、行选择、分页、加载状态 |
| `AntTree` | `tree` | `QProxyStyle` | 是 | 已迁移至 `src/styles/AntTreeStyle`；展开/收起、节点选择、复选框、连接线 |

### 布局与其他

| 组件 | Ant Design 对应目录 | 绘制方式 | 示例覆盖 | 说明 |
| --- | --- | --- | --- | --- |
| `AntDivider` | `divider` | `QProxyStyle` | 是 | 已迁移至 `src/styles/AntDividerStyle`；水平/垂直、带标题、虚线/点线 |
| `AntSpace` | `space` | `QProxyStyle` | 是 | `src/styles/AntSpaceStyle`；水平/垂直间距容器，Small/Middle/Large，支持自定义间距 |
| `AntLayout` | `layout` | `QProxyStyle` | 是 | `src/styles/AntLayoutStyle`；Header/Footer/Content/Sider 布局，Sider 可折叠 |
| `AntTimeline` | `timeline` | `QProxyStyle` | 是 | `src/styles/AntTimelineStyle`；垂直/水平时间轴，outlined/filled，颜色预设 |
| `AntTypography` | `typography` | `QProxyStyle` | 是 | `src/styles/AntTypographyStyle`；Title(H1-H5)/Text/Paragraph，类型/装饰/复制 |
| `AntWidget` | — | — | 是 | 基础 QWidget 子类，自动处理主题切换，提供 tokens() 和 onThemeChanged() |
| `AntWindow` | — | `QProxyStyle` | 是 | 已迁移至 `src/styles/AntWindowStyle`；无边框窗口，自定义标题栏、拖拽、最小化/最大化/关闭按钮 |
| `AntDrawer` | `drawer` | `QProxyStyle` | 是 | 已迁移至 `src/styles/AntDrawerStyle`；滑动面板，Left/Right/Top/Bottom、动画、遮罩层 |
| `AntStatusBar` | — | `QProxyStyle` | 是 | 已迁移至 `src/styles/AntStatusBarStyle`；左右项、分隔符、消息区、size grip |
| `AntScrollBar` | — | `QProxyStyle` | 是 | 已迁移至 `src/styles/AntScrollBarStyle`；8px 细滚动条、自动隐藏、无箭头按钮 |

## 待移植组件

以下清单基于 `submodules/ant-design/components/` 扫描，并扣除当前已实现组件后整理。仅保留与 Qt Widgets 组件库相关度较高的部分，按优先级排序。

### 高优先级

### 中优先级

### 后续扩展

- [ ] `AntCalendar`
- [ ] `AntImage`
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
- 所有组件均已迁移到 `QProxyStyle`，新组件开发必须按 `QProxyStyle` 架构设计
- 主题切换统一监听：
  - `AntTheme::themeChanged`
  - 或 `AntTheme::themeModeChanged`
- 每次新增组件或架构迁移后，必须同步更新：
  - `AGENT.md`
  - `README.md`
  - `examples/ExampleWindow.cpp`

## 示例程序

当前 `examples/ExampleWindow.cpp` 已覆盖全部 50 个已实现组件，左侧导航与右侧页面一一对应，当前没有”已实现但未展示”的组件。

示例程序架构：
- `ExampleWindow` 继承 `AntWindow`（无边框窗口，自定义标题栏）
- 使用 `AntWidget` 作为侧边栏和内容区容器
- 使用 `AntTypography` 替代 `QLabel`，通过 `setTitle()` / `setParagraph()` / `setType()` 实现主题感知
- 使用 `AntScrollBar` 替代原生滚动条
- 仅保留 3 处 `QLabel`（需要 `setAlignment(Qt::AlignCenter)` 的场景）

当前示例页包括：

- `Button`
- `Breadcrumb`
- `Checkbox`
- `DatePicker`
- `Descriptions`
- `Dropdown`
- `Input`
- `Message`
- `Menu`
- `Tabs`
- `Badge`
- `Avatar`
- `Tag`
- `Notification`
- `Popover`
- `Popconfirm`
- `Modal`
- `Pagination`
- `Progress`
- `Radio`
- `Select`
- `Slider`
- `Spin`
- `Steps`
- `Switch`
- `TimePicker`
- `Card`
- `Skeleton`
- `Divider`
- `Icon`
- `InputNumber`
- `Alert`
- `Tooltip`
- `Form`
- `Empty`
- `Result`
- `List`
- `Statistic`
- `Timeline`
- `Space`
- `Layout`
- `Typography`
- `Table`
- `Tree`
- `Upload`
- `Cascader`
- `TreeSelect`
- `Window`
- `Drawer`
- `StatusBar`
- `ScrollBar`

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
