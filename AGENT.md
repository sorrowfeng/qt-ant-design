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

- 同步日期：`2026-04-26`
- 已实现公开组件总数：`82`
- Ant Design 标准组件覆盖率：`70 / 70`（100%）
- 子组件/变体完整度：`15 / 15`（100%）
- Qt / 桌面扩展组件：`12`（AntWindow、AntWidget、AntStatusBar、AntScrollBar、AntMenuBar、AntToolBar、AntToolButton、AntScrollArea、AntPlainTextEdit、AntDockWidget、AntLog、AntNavItem）
- 已迁移至 `QProxyStyle` 的组件数：`~62`
- 不依赖独立 Style 类的组件：`AntAffix`、`AntAnchor`、`AntApp`、`AntCarousel`、`AntCollapse`、`AntColorPicker`、`AntConfigProvider`、`AntDockWidget`、`AntFlex`、`AntGrid`、`AntImage`、`AntLog`、`AntMasonry`、`AntMentions`、`AntNavItem`、`AntScrollArea`、`AntSplitter`、`AntTour`、`AntTransfer`、`AntWidget`
- 示例程序覆盖：`80 / 82`；当前未单独提供示例页的组件：`AntWidget`、`AntNavItem`（NavItem 用于示例程序自身导航）
- 示例程序架构：`ExampleWindow` 继承 `AntWindow`，使用 `AntWidget` 构建布局，`AntNavItem` 实现侧边栏导航，`AntCard` 作为各示例区块容器，`AntTypography` 替代 `QLabel` 实现主题感知文本，示例页面零样式操作（无 QPalette/setAutoFillBackground/setFont/setStyleSheet）

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

## 子组件/变体完整度（2026-04-26）

### Phase 1: 简单变体（6 项）
- `Typography.Link` — `TypographyType::Link`、`href` 属性、`linkActivated` 信号、自动下划线 + 手型光标
- `Message.placement` — `Ant::Placement` 枚举（Top/TopLeft/TopRight/Bottom/BottomLeft/BottomRight）
- `Card.Meta` — `setMetaAvatar()`/`setMetaTitle()`/`setMetaDescription()` 方法
- `Card.Grid` — `addGridItem()` 方法，body 转 QGridLayout（3 列）
- `Statistic.Countdown` — `countdownMode`/`countdownFormat` 属性、`countdownFinished` 信号
- `Skeleton.Element` — `Ant::SkeletonElement` 枚举（Button/Avatar/Input/Image/Node）

### Phase 2: 中等复杂度（6 项）
- `Avatar.Group` — `AntAvatarGroup` 类，`maxCount`、重叠布局、"+N" 溢出
- `Badge.Ribbon` — `Ant::BadgeMode::Ribbon`、`ribbonText`/`ribbonColor`、折叠丝带绘制
- `Upload.Dragger` — `draggerMode`、拖放文件支持、虚线边框区域
- `Image.PreviewGroup` — `setPreviewGroup()`、左右导航、键盘支持
- `Modal 命令式 API` — `info()`/`success()`/`warning()`/`error()`/`confirm()` 静态方法
- `Form.Provider` — `AntFormProvider` 类，注册表单、信号转发

### Phase 3: 高复杂度（3 项）
- `Select multiple/tags` — `Ant::SelectMode` 枚举、`selectedIndices`、tag 渲染、`addTag()`、Backspace 删除
- `Form.List` — `AntFormList` 类、动态增删行、`minCount`/`maxCount`、工厂回调
- `Skeleton.Node` — 作为 `Skeleton.Element::Node` 已在 Phase 1 实现

## 当前组件状态

### 通用

| 组件 | Ant Design 对应目录 | 绘制方式 | 示例覆盖 | 说明 |
| --- | --- | --- | --- | --- |
| `AntButton` | `button` | `QProxyStyle` | 是 | 五种类型、三种尺寸、三种形状 |
| `AntFloatButton` | `float-button` | `QProxyStyle` | 是 | 圆形/方形、Group/BackTop、Badge |
| `AntIcon` | `icon` | `QProxyStyle` | 是 | Outlined/Filled/TwoTone、旋转、spin |
| `AntTypography` | `typography` | `QProxyStyle` | 是 | Title(H1-H5)/Text/Paragraph/Link |

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
| `AntForm` | `form` | `QProxyStyle` | 是 | 横向/纵向/行内布局、校验提示、Provider、List |
| `AntInput` | `input` | `QProxyStyle` | 是 | 尺寸、状态、Password/Search、addon |
| `AntInputNumber` | `input-number` | `QProxyStyle` | 是 | 精度、小步进、前后缀 |
| `AntMentions` | `mentions` | `QProxyStyle` | 是 | @提及输入，弹出建议 |
| `AntRadio` | `radio` | `QProxyStyle` | 是 | Radio.Group |
| `AntRate` | `rate` | `QProxyStyle` | 是 | count/value/allowHalf/hover 放大 |
| `AntSegmented` | `segmented` | `QProxyStyle` | 是 | 滑动指示器动画、图标/禁用 |
| `AntSelect` | `select` | `QProxyStyle` | 是 | 尺寸、状态、变体、可编辑模式、Multiple/Tags |
| `AntSlider` | `slider` | `QProxyStyle` | 是 | |
| `AntSwitch` | `switch` | `QProxyStyle` | 是 | |
| `AntTimePicker` | `time-picker` | `QProxyStyle` | 是 | 自绘时间弹层、RangePicker |
| `AntTransfer` | `transfer` | 自绘 | 是 | 穿梭框、双列表 |
| `AntTreeSelect` | `tree-select` | `QProxyStyle` | 是 | 下拉树形结构 |
| `AntUpload` | `upload` | `QProxyStyle` | 是 | 文本/图片/卡片三种模式、Dragger |

### 反馈

| 组件 | Ant Design 对应目录 | 绘制方式 | 示例覆盖 | 说明 |
| --- | --- | --- | --- | --- |
| `AntAlert` | `alert` | `QProxyStyle` | 是 | type/icon/description/closable/banner |
| `AntDrawer` | `drawer` | `QProxyStyle` | 是 | Left/Right/Top/Bottom、动画、遮罩 |
| `AntMessage` | `message` | `QProxyStyle` | 是 | Qt::ToolTip 浮层消息、6 种 placement |
| `AntModal` | `modal` | `QProxyStyle` | 是 | 遮罩层、标题/正文、自定义 footer、命令式 API |
| `AntNotification` | `notification` | `QProxyStyle` | 是 | 多 placement 通知 |
| `AntPopconfirm` | `popconfirm` | `QProxyStyle` | 是 | title/description/ok/cancel/placement |
| `AntPopover` | `popover` | `QProxyStyle` | 是 | title/content/action/hover/click/placement |
| `AntProgress` | `progress` | `QProxyStyle` | 是 | line/circle/dashboard |
| `AntResult` | `result` | `QProxyStyle` | 是 | status/title/subTitle/extra |
| `AntSkeleton` | `skeleton` | `QProxyStyle` | 是 | active shimmer、头像/标题/段落占位、Element 变体 |
| `AntSpin` | `spin` | `QProxyStyle` | 是 | small/middle/large/percent/delay |
| `AntTooltip` | `tooltip` | `QProxyStyle` | 是 | title/placement/color/arrow/delay |
| `AntWatermark` | `watermark` | `QProxyStyle` | 是 | 旋转文本平铺、多行、自定义间距 |
| `AntTour` | `tour` | 自绘 | 是 | 遮罩式分步引导、目标高亮 |

### 数据展示

| 组件 | Ant Design 对应目录 | 绘制方式 | 示例覆盖 | 说明 |
| --- | --- | --- | --- | --- |
| `AntAvatar` | `avatar` | `QProxyStyle` | 是 | 文本、图标、图片头像、Group |
| `AntBadge` | `badge` | `QProxyStyle` | 是 | count/dot/status/processing/Ribbon |
| `AntCalendar` | `calendar` | `QProxyStyle` | 是 | Day/Month/Year 三态 |
| `AntCard` | `card` | `QProxyStyle` | 是 | 封面、extra、action 区、loading、Meta、Grid |
| `AntCarousel` | `carousel` | 自绘 | 是 | 自动播放、圆点指示器 |
| `AntCollapse` | `collapse` | 自绘 | 是 | 折叠面板、accordion 模式、动画 |
| `AntEmpty` | `empty` | `QProxyStyle` | 是 | 默认插画、simple 模式 |
| `AntImage` | `image` | 自绘 | 是 | 图片展示、全屏预览、PreviewGroup |
| `AntList` | `list` | `QProxyStyle` | 是 | header/footer/bordered/split/size |
| `AntPopover` | — | `QProxyStyle` | 是 | 已在反馈类 |
| `AntQRCode` | `qr-code` | `QProxyStyle` | 是 | 嵌入式 QR 生成、状态叠加 |
| `AntStatistic` | `statistic` | `QProxyStyle` | 是 | title/value/precision/prefix/suffix/Countdown |
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

### Qt / 桌面扩展组件

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
| `AntNavItem` | 自绘 | 是 | 侧边栏导航项，active/hover 状态，clicked 信号 |
| `AntWave` | — | — | 内部涟漪动画 overlay（core/），不计入 `src/widgets` 公开组件统计 |

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

### 绘制模式层级

项目使用三种绘制模式，**优先级从高到低**：

| 模式 | 适用场景 | 实现方式 | 使用组件数 |
| --- | --- | --- | --- |
| **Pattern A: eventFilter** | 自定义 QWidget 子类（`AntResult`、`AntAlert` 等） | Style 类通过 `eventFilter` 拦截 `QEvent::Paint`，调用 `drawWidget()` | ~55 |
| **Pattern B: drawControl** | 标准 Qt 子类（`QPushButton`、`QCheckBox`、`QMenuBar`） | Style 类重写 `drawControl()` / `drawComplexControl()` | ~5 |
| **Pattern C: paintEvent** | 简单组件或不需要独立 Style 的组件 | Widget 自身重写 `paintEvent()` | ~19 |

**新增组件应优先使用 Pattern A**，除非组件继承自标准 Qt 控件（此时用 Pattern B）。

#### Pattern A 实现模板

```cpp
// AntXxxStyle.h
class AntXxxStyle : public AntStyleBase
{
    Q_OBJECT
public:
    explicit AntXxxStyle(QStyle* style = nullptr);
    void polish(QWidget* widget) override;
    void unpolish(QWidget* widget) override;
    bool drawWidget(QWidget* widget, QPaintEvent* event) override;
};

// AntXxxStyle.cpp
AntXxxStyle::AntXxxStyle(QStyle* style)
    : AntStyleBase(style)
{
    connectThemeUpdate<AntXxx>();
}

void AntXxxStyle::polish(QWidget* widget)
{
    QProxyStyle::polish(widget);
    installPaintFilter<AntXxx>(widget);
}

void AntXxxStyle::unpolish(QWidget* widget)
{
    removePaintFilter<AntXxx>(widget);
    QProxyStyle::unpolish(widget);
}

bool AntXxxStyle::drawWidget(QWidget* widget, QPaintEvent* event)
{
    auto* xxx = qobject_cast<AntXxx*>(widget);
    if (!xxx) return false;
    QPainter painter(xxx);
    // ... painting logic ...
    return true;
}
```

`AntStyleBase` 提供的辅助方法：
- `installPaintFilter<T>(widget)` — 安装 eventFilter + WA_Hover
- `removePaintFilter<T>(widget)` — 移除 eventFilter
- `drawWidget(widget, event)` — 虚方法，子类重写实现绘制
- `connectThemeUpdate<T>()` — 连接主题切换信号
- `onThemeUpdate(w)` — 主题切换时的默认行为（updateGeometry + update）
- `drawCrispRoundedRect(painter, rect, pen, brush, rx, ry)` — 0.5px 子像素偏移绘制圆角矩形，解决边框锯齿问题

## 示例程序

当前 `examples/ExampleWindow.cpp` 已覆盖 `80 / 81` 个公开组件，左侧导航与右侧页面一一对应。

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

## 测试

### 概述

项目使用 QTest 框架进行单元测试，覆盖所有 82 个组件的属性、getter/setter 和信号验证。

- **测试框架**：Qt6::Test（QTest + QSignalSpy）
- **测试数量**：17 个测试可执行文件
- **覆盖组件**：82 个组件全部覆盖
- **运行方式**：`ctest -C Debug --output-on-failure`

### 测试文件结构

```
tests/
├── CMakeLists.txt              # 测试目标注册
├── TestAntTypes.cpp            # 核心枚举值验证
├── TestAntButton.cpp           # Button 属性/信号
├── TestAntIcon.cpp             # Icon 属性/信号
├── TestAntTypography.cpp       # Typography 属性/信号
├── TestAntFloatButton.cpp      # FloatButton 属性/信号
├── TestAntCheckbox.cpp         # Checkbox 属性/信号
├── TestAntSwitch.cpp           # Switch 属性/信号
├── TestAntSelect.cpp           # Select 单选/多选/标签
├── TestAntInput.cpp            # Input 属性/信号
├── TestAntDataEntryA.cpp       # InputNumber, Radio, Slider, Rate, Segmented, AutoComplete
├── TestAntDataEntryB.cpp       # Cascader, DatePicker, TimePicker, Mentions, Transfer, TreeSelect, Upload
├── TestAntDataDisplayA.cpp     # Avatar, Card, Statistic, Calendar, Image, Empty
├── TestAntDataDisplayB.cpp     # List, Table, Tree, Timeline, Descriptions, QRCode, Watermark, Carousel, Collapse
├── TestAntFeedback.cpp         # Alert, Drawer, Message, Notification, Popconfirm, Popover, Progress, Result, Skeleton, Spin, Tooltip, Tour
├── TestAntNavigation.cpp       # Breadcrumb, Dropdown, Menu, Pagination, Steps, Tabs, Anchor
├── TestAntLayout.cpp           # Divider, Flex, Grid, Space, Layout, Masonry, Affix
└── TestAntQtExtensions.cpp     # App, ConfigProvider, Form, Log, NavItem, PlainTextEdit, ScrollArea, ScrollBar, Splitter, StatusBar, ToolButton, ToolBar, MenuBar, DockWidget, Widget, Window, ColorPicker
```

### 测试模式

每个测试文件遵循以下模式：

```cpp
#include <QSignalSpy>
#include <QTest>
#include "widgets/AntXxx.h"

class TestAntXxx : public QObject
{
    Q_OBJECT
private slots:
    void propertiesAndSignals();
};

void TestAntXxx::propertiesAndSignals()
{
    auto* w = new AntXxx;           // 堆分配，不 delete（避免 QProxyStyle 析构崩溃）
    QCOMPARE(w->property(), default);  // 验证默认值

    QSignalSpy spy(w, &AntXxx::propertyChanged);
    w->setProperty(newValue);           // 设置新值
    QCOMPARE(w->property(), newValue);  // 验证 setter 生效
    QCOMPARE(spy.count(), 1);           // 验证信号发射
}

QTEST_MAIN(TestAntXxx)
#include "TestAntXxx.moc"
```

### 关键注意事项

1. **单测试函数模式**：由于 QProxyStyle 析构在 QTest 环境中会崩溃，所有测试必须使用单个 `propertiesAndSignals()` 函数，避免多个测试槽之间 widget 构造/析构的冲突。

2. **堆分配不删除**：widget 必须用 `new` 创建且不调用 `delete`，让进程退出时统一清理。

3. **信号验证**：设置新值必须与当前值不同，否则 setter 会提前返回不发射信号。

4. **分页/约束属性**：某些属性有依赖约束（如 `AntTable::setCurrentPage` 受 `totalPages()` 约束，`AntPagination::setCurrent` 受 `total` 约束），需要先设置依赖属性。

### 运行测试

```powershell
# 构建
cmake -B build
cmake --build build --config Debug

# 运行所有测试
cd build
ctest -C Debug --output-on-failure

# 运行单个测试
.\tests\Debug\TestAntButton.exe

# 运行单个测试（通过 ctest）
ctest -C Debug -R TestAntButton --output-on-failure
```
