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

## 项目状态

- 同步日期：`2026-04-25`
- 已实现组件总数：`80`
- Ant Design 标准组件覆盖率：`74 / 74`（100%）
- Qt 专有组件：`6`（AntWindow、AntWidget、AntStatusBar、AntScrollBar、AntMenuBar、AntToolBar）
- 已迁移至 `QProxyStyle` 的组件数：`~62`
- 不依赖 Style 类的组件：`AntScrollArea`、`AntColorPicker`、`AntDockWidget`、`AntWidget`、`AntAffix`、`AntApp`、`AntConfigProvider`
- 示例程序覆盖：`80 / 80`，所有组件均可在 `examples/ExampleWindow.cpp` 中查看
- 示例程序架构：`ExampleWindow` 继承 `AntWindow`，使用 `AntWidget` 构建布局，`AntTypography` 替代 `QLabel` 实现主题感知文本

## 本轮新增组件（2026-04-25，第 2-4 批）

### 第一批：Qt 基础设施（9 个）
- `AntToolButton` — QToolButton + QProxyStyle，dropdown 箭头动画
- `AntScrollArea` — QScrollArea 包裹器，AntScrollBar + QScroller
- `AntPlainTextEdit` — QPlainTextEdit + eventFilter Style，3 变体
- `AntMenuBar` — QMenuBar + drawControl Style
- `AntToolBar` — QToolBar + drawControl Style，浮动阴影
- `AntDockWidget` — QDockWidget，自定义标题栏，Win32 resize
- `AntAutoComplete` — QWidget 组合，弹出建议，键盘导航
- `AntCalendar` — QTableView + Model/View，Day/Month/Year 三态
- `AntColorPicker` — QDialog，HS field，RGB/HSV，预设/自定义颜色

### 第二批：数据展示与布局（4 个）
- `AntImage` — 图片展示 + 全屏预览
- `AntCollapse` — 折叠面板/手风琴，InOutCubic 动画
- `AntSplitter` — QSplitter 主题化手柄
- `AntLog` — 5 级别日志输出，彩色时间戳

### 第三批：Ant Design 标准组件补齐（10 个）
- `AntCarousel` — 轮播图，自动播放，圆点指示器
- `AntGrid` (Row/Col) — 24 列栅格布局，span/offset
- `AntFlex` — Flex 布局，gap/wrap/vertical
- `AntAnchor` — 滚动锚点导航，active 高亮
- `AntTransfer` — 穿梭框，双列表转移
- `AntTour` — 遮罩式分步引导
- `AntMentions` — @提及输入
- `AntMasonry` — 瀑布流布局
- `AntApp` — 应用包裹器
- `AntConfigProvider` — 主题配置提供者

### 已有组件增强
- AntInput：新增 `searchMode` + `searchTriggered` 信号
- AntSelect：新增 `editable` 模式，内联过滤
- AntDatePicker：新增 `rangeMode`/`startDate`/`endDate`
- AntTimePicker：新增 `rangeMode`/`startTime`/`endTime`
- AntButtonStyle：修复 `adjusted(0,0,-1,-1)` 导致右/下边框 1px 缺失
- AntAutoComplete：修复 Qt::Popup 抢占焦点问题

## 当前组件状态

### 通用

| 组件 | Ant Design 对应目录 | 绘制方式 | 示例覆盖 | 说明 |
| --- | --- | --- | --- | --- |
| `AntButton` | `button` | `QProxyStyle` | 是 | 五种类型、三种尺寸、三种形状 |
| `AntFloatButton` | `float-button` | `QProxyStyle` | 是 | 圆形/方形、Group/BackTop、Badge |
| `AntIcon` | `icon` | `QProxyStyle` | 是 | Outlined/Filled/TwoTone、旋转、spin |
| `AntTypography` | `typography` | `QProxyStyle` | 是 | Title(H1-H5)/Text/Paragraph |

### 导航

| 组件 | Ant Design 对应目录 | 绘制方式 | 示例覆盖 | 说明 |
| --- | --- | --- | --- | --- |
| `AntAnchor` | `anchor` | 自绘 | 是 | 滚动锚点，active 高亮 |
| `AntBreadcrumb` | `breadcrumb` | `QProxyStyle` | 是 | 路径项、分隔符、禁用项 |
| `AntDropdown` | `dropdown` | `QProxyStyle` | 是 | hover/click/contextMenu、placement、arrow |
| `AntMenu` | `menu` | `QProxyStyle` | 是 | vertical/horizontal/inline、明暗主题 |
| `AntPagination` | `pagination` | `QProxyStyle` | 是 | simple/showQuickJumper/showSizeChanger |
| `AntSteps` | `steps` | `QProxyStyle` | 是 | 水平/垂直、当前步骤、错误态 |
| `AntTabs` | `tabs` | `QProxyStyle` | 是 | line/card/editable-card |

### 数据录入

| 组件 | Ant Design 对应目录 | 绘制方式 | 示例覆盖 | 说明 |
| --- | --- | --- | --- | --- |
| `AntAutoComplete` | `auto-complete` | `QProxyStyle` | 是 | 建议弹出、键盘导航 |
| `AntCascader` | `cascader` | `QProxyStyle` | 是 | 多列弹出面板、点击/悬停展开 |
| `AntCheckbox` | `checkbox` | `QProxyStyle` | 是 | |
| `AntColorPicker` | `color-picker` | 自绘 | 是 | HS field、RGB/HSV、预设、static getColor() |
| `AntDatePicker` | `date-picker` | `QProxyStyle` | 是 | 自绘日期弹层、RangePicker |
| `AntDescriptions` | `descriptions` | `QProxyStyle` | 是 | 标题、extra、bordered、vertical |
| `AntForm` | `form` | `QProxyStyle` | 是 | 横向/纵向/行内布局、校验提示 |
| `AntInput` | `input` | `QProxyStyle` | 是 | 尺寸、状态、Password/Search、addon |
| `AntInputNumber` | `input-number` | `QProxyStyle` | 是 | 精度、小步进、前后缀 |
| `AntMentions` | `mentions` | `QProxyStyle` | 是 | @提及输入，弹出建议 |
| `AntRadio` | `radio` | `QProxyStyle` | 是 | Radio.Group |
| `AntRate` | `rate` | `QProxyStyle` | 是 | count/value/allowHalf/hover 放大 |
| `AntSegmented` | `segmented` | `QProxyStyle` | 是 | 滑动指示器动画、图标/禁用 |
| `AntSelect` | `select` | `QProxyStyle` | 是 | 尺寸、状态、变体、可编辑模式 |
| `AntSlider` | `slider` | `QProxyStyle` | 是 | |
| `AntSwitch` | `switch` | `QProxyStyle` | 是 | |
| `AntTimePicker` | `time-picker` | `QProxyStyle` | 是 | 自绘时间弹层、RangePicker |
| `AntTransfer` | `transfer` | 自绘 | 是 | 穿梭框、双列表 |
| `AntTreeSelect` | `tree-select` | `QProxyStyle` | 是 | 下拉树形结构 |
| `AntUpload` | `upload` | `QProxyStyle` | 是 | 文本/图片/卡片三种模式 |

### 反馈

| 组件 | Ant Design 对应目录 | 绘制方式 | 示例覆盖 | 说明 |
| --- | --- | --- | --- | --- |
| `AntAlert` | `alert` | `QProxyStyle` | 是 | type/icon/description/closable/banner |
| `AntDrawer` | `drawer` | `QProxyStyle` | 是 | Left/Right/Top/Bottom、动画、遮罩 |
| `AntMessage` | `message` | `QProxyStyle` | 是 | Qt::ToolTip 浮层消息 |
| `AntModal` | `modal` | `QProxyStyle` | 是 | 遮罩层、标题/正文、自定义 footer |
| `AntNotification` | `notification` | `QProxyStyle` | 是 | 多 placement 通知 |
| `AntPopconfirm` | `popconfirm` | `QProxyStyle` | 是 | title/description/ok/cancel/placement |
| `AntPopover` | `popover` | `QProxyStyle` | 是 | title/content/action/hover/click/placement |
| `AntProgress` | `progress` | `QProxyStyle` | 是 | line/circle/dashboard |
| `AntResult` | `result` | `QProxyStyle` | 是 | status/title/subTitle/extra |
| `AntSkeleton` | `skeleton` | `QProxyStyle` | 是 | active shimmer、头像/标题/段落占位 |
| `AntSpin` | `spin` | `QProxyStyle` | 是 | small/middle/large/percent/delay |
| `AntTooltip` | `tooltip` | `QProxyStyle` | 是 | title/placement/color/arrow/delay |
| `AntWatermark` | `watermark` | `QProxyStyle` | 是 | 旋转文本平铺、多行、自定义间距 |
| `AntTour` | `tour` | 自绘 | 是 | 遮罩式分步引导、目标高亮 |

### 数据展示

| 组件 | Ant Design 对应目录 | 绘制方式 | 示例覆盖 | 说明 |
| --- | --- | --- | --- | --- |
| `AntAvatar` | `avatar` | `QProxyStyle` | 是 | 文本、图标、图片头像 |
| `AntBadge` | `badge` | `QProxyStyle` | 是 | count/dot/status/processing |
| `AntCalendar` | `calendar` | `QProxyStyle` | 是 | Day/Month/Year 三态 |
| `AntCard` | `card` | `QProxyStyle` | 是 | 封面、extra、action 区、loading |
| `AntCarousel` | `carousel` | 自绘 | 是 | 自动播放、圆点指示器 |
| `AntCollapse` | `collapse` | 自绘 | 是 | 折叠面板、accordion 模式、动画 |
| `AntEmpty` | `empty` | `QProxyStyle` | 是 | 默认插画、simple 模式 |
| `AntImage` | `image` | 自绘 | 是 | 图片展示、全屏预览 |
| `AntList` | `list` | `QProxyStyle` | 是 | header/footer/bordered/split/size |
| `AntPopover` | — | `QProxyStyle` | 是 | 已在反馈类 |
| `AntQRCode` | `qr-code` | `QProxyStyle` | 是 | 嵌入式 QR 生成、状态叠加 |
| `AntStatistic` | `statistic` | `QProxyStyle` | 是 | title/value/precision/prefix/suffix |
| `AntTable` | `table` | `QProxyStyle` | 是 | 排序、选择、分页、空态插画 |
| `AntTag` | `tag` | `QProxyStyle` | 是 | 13 色预设、closable/checkable/variant |
| `AntTimeline` | `timeline` | `QProxyStyle` | 是 | 垂直/水平、outlined/filled、颜色 |
| `AntTooltip` | — | `QProxyStyle` | 是 | 已在反馈类 |
| `AntTree` | `tree` | `QProxyStyle` | 是 | 展开/收起、选择、复选框、连接线 |

### 布局及其他

| 组件 | Ant Design 对应目录 | 绘制方式 | 示例覆盖 | 说明 |
| --- | --- | --- | --- | --- |
| `AntAffix` | `affix` | QObject 工具 | 是 | 滚动吸附/解除 |
| `AntApp` | `app` | QObject 工具 | 是 | 应用包裹器 |
| `AntConfigProvider` | `config-provider` | QObject 工具 | 是 | 主题配置 |
| `AntDivider` | `divider` | `QProxyStyle` | 是 | 水平/垂直、带标题、虚线 |
| `AntFlex` | `flex` | 自绘 | 是 | 弹性布局、gap/wrap/vertical |
| `AntGrid` (Row/Col) | `grid` | 自绘 | 是 | 24 列栅格、span/offset |
| `AntLayout` | `layout` | `QProxyStyle` | 是 | Header/Footer/Content/Sider |
| `AntMasonry` | `masonry` | 自绘 | 是 | 瀑布流、最短列优先 |
| `AntSpace` | `space` | `QProxyStyle` | 是 | 水平/垂直间距容器 |
| `AntSplitter` | `splitter` | 自绘 | 是 | 可拖拽分割面板 |

### Qt 专有组件

| 组件 | 绘制方式 | 示例覆盖 | 说明 |
| --- | --- | --- | --- |
| `AntWidget` | — | 是 | 基础 QWidget，自动主题切换 |
| `AntWindow` | `QProxyStyle` | 是 | 无边框窗口，自定义标题栏 |
| `AntDockWidget` | 自绘 | 是 | 可停靠面板，Win32 resize |
| `AntStatusBar` | `QProxyStyle` | 是 | 状态栏 |
| `AntScrollBar` | `QProxyStyle` | 是 | 8px 细滚动条，自动隐藏 |
| `AntScrollArea` | — | 是 | QScrollArea + AntScrollBar + QScroller |
| `AntMenuBar` | `QProxyStyle` | 是 | 菜单栏 |
| `AntToolBar` | `QProxyStyle` | 是 | 工具栏 |
| `AntToolButton` | `QProxyStyle` | 是 | 带下拉菜单的按钮 |
| `AntPlainTextEdit` | `QProxyStyle` | 是 | 多行文本编辑器 |
| `AntLog` | 自绘 | 是 | 日志输出控件 |
| `AntWave` | — | — | 涟漪动画 overlay（core/） |

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
  - `polish` → `updateGeometry` → `update`
- 纯容器/对话框组件（如 AntScrollArea、AntColorPicker）可不含独立 Style 类
- 主题切换统一监听：
  - `AntTheme::themeChanged`
  - 或 `AntTheme::themeModeChanged`
- 每次新增组件后，必须同步更新：
  - `AGENT.md`
  - `README.md`
  - `examples/ExampleWindow.cpp`

## 示例程序

当前 `examples/ExampleWindow.cpp` 已覆盖全部 80 个组件，左侧导航与右侧页面一一对应。

示例程序架构：
- `ExampleWindow` 继承 `AntWindow`（无边框窗口，自定义标题栏）
- 使用 `AntWidget` 作为侧边栏和内容区容器
- 使用 `AntTypography` 替代 `QLabel`，通过 `setTitle()` / `setParagraph()` / `setType()` 实现主题感知
- 使用 `AntScrollBar` 替代原生滚动条

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
