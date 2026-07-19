# qt-ant-design Agent Notes

## 项目定位

`qt-ant-design` 是一个基于 Qt Widgets 的 C++ 组件库，CMake 配置时自动识别 Qt6 或 Qt5，目标是以 `QPainter` 手绘方式复刻 Ant Design 设计系统，并逐步将复杂控件的主绘制链路迁移到 `QProxyStyle` 架构。

当前仓库输出：

- 静态库或动态库 `qt-ant-design`（由 `BUILD_SHARED_LIBS` 控制）
- 示例程序 `qt-ant-design-example`
- 可安装头文件、CMake targets 和 Windows 下的运行依赖

## 参考仓库

- 绘制实现参考：`https://github.com/Liniyous/ElaWidgetTools`
- 设计规范参考：`https://github.com/ant-design/ant-design`

参考仓库不再以 Git submodule 方式维护在项目内，需要对照上游实现时直接查看对应 GitHub 仓库。

重点参考内容：

- `ElaTheme` / 自绘控件的状态管理、动画、主题切换
- Ant Design 的 token、组件状态、尺寸、交互和视觉层级

## 项目状态

- 同步日期：`2026-07-16`
- 当前版本：`0.1.2`（根目录 `VERSION` 为唯一版本源；CMake 生成并安装 `core/QtAntDesignVersion.h`；发布记录见 `CHANGELOG.md`，流程见 `docs/versioning.md`）
- 状态总览：`docs/project-status.md`
- 代码审计与整改清单：`docs/code-audit-optimization.md`
- 历史问题记录：`docs/archive/issue-log.md`；当前新增问题优先使用 GitHub Issues 跟踪
- 已实现公开组件总数：`89`（`src/widgets` 有 `110` 个 `Ant*.h`，包含 `89` 个公开组件头、`19` 个 Qt 风格别名头、安装的非组件窗口帧 helper `AntWindowFrame`，以及内部非安装弹层 helper `AntSelectPopup`）
- Ant Design 标准组件覆盖率：`70 / 70`（100%）
- 子组件/变体完整度：`15 / 15`（100%）
- Qt / 桌面扩展组件：`19`（AntWindow、AntDialog、AntInputDialog、AntRibbon、AntWidget、AntStatusBar、AntScrollBar、AntMenuBar、AntToolBar、AntToolButton、AntScrollArea、AntStackedWidget、AntFileDialog、AntPlainTextEdit、AntDockManager、AntDockWidget、AntLog、AntNav、AntNavItem）
- 已迁移至 `QProxyStyle` 的组件数：`67` 个 `Ant*Style` 类
- 不依赖独立 Style 类的组件：`AntAffix`、`AntAnchor`、`AntApp`、`AntCarousel`、`AntCollapse`、`AntColorPicker`、`AntConfigProvider`、`AntDockWidget`、`AntFlex`、`AntGrid`、`AntImage`、`AntLog`、`AntMasonry`、`AntMentions`、`AntNav`、`AntNavItem`、`AntRibbon`、`AntScrollArea`、`AntSplitter`、`AntTour`、`AntTransfer`、`AntWidget`
- 示例程序覆盖：`89 / 89` 个公开组件，另有独立 `Showcase` 页面；`AntDockManager` 合并在 DockWidget 示例页展示，其余公开组件均已提供独立示例页
- Qt 风格别名：名字与常用 Qt 控件无法直观对应的组件提供轻量头文件别名（如 `AntLabel.h` → `AntTypography`）；仅大小写差异时以 Qt 命名为准（如 `AntCheckBox`、`AntToolTip`），不保留旧拼写兼容 alias。
- 示例程序架构：`ExampleWindow` 继承 `AntWindow`，使用 `AntWidget` 构建布局，`AntNav` / `AntNavItem` 实现侧边栏导航，`AntCard` 作为各示例区块容器，`AntTypography` 替代 `QLabel` 实现主题感知文本，示例页面零样式操作（无 QPalette/setAutoFillBackground/setFont/setStyleSheet）
- 视觉审计状态：可对比的 Ant Design 标准组件均记录为 `Pass`，Qt-only 扩展记录为 `Local Pass`，详情见 `docs/visual-audit.md`
- README 组件截图画廊：`resources/images/components/` 提交 `176` 张 Light/Dark PNG，覆盖 `88` 个视觉组件条目；`AntDockManager` 通过 DockWidget 示例页展示，弹层/反馈类控件截图使用代表性的打开或激活状态
- Icon 状态：内置 `831` 个官方 `@ant-design/icons-svg@4.4.2` SVG 资源，清单见 `docs/ant-design-icons.md`
- 测试状态：当前 Windows 顶层默认配置注册 `51` 个深度/系统 CTest 条目，启用 `QT_ANT_DESIGN_BUILD_WIDGET_SMOKE_TESTS=ON` 时另有 `104` 个逐控件 smoke 条目（合计 `155`）；`2026-07-16` 最新 Qt6 Debug 全量 `155 / 155` 通过，总耗时 `379.87s`。最终 Qt5 P1/P2 定向矩阵 `5 / 5` 通过，链接到 MSVC ASan 插桩库的 9 个关键目标 `9 / 9` 通过；本地尚未覆盖 UBSan
- 逐控件可靠性覆盖矩阵：`docs/reliability-coverage.md`，列出 89 个公开组件的专项行为/API、生命周期、Meta 属性、主题切换和渲染烟测覆盖情况

## P0-P2 代码审计整改（2026-07-16）

- 外部借用的 `QObject` / `QWidget` 引用统一采用 `QPointer`、`destroyed` 清理或等价的销毁感知跟踪；安装 event filter 的 owner 必须在成员析构前显式解除 watcher，避免 watcher 晚于 owner 存活时回调半析构对象。
- `AntApp` 的 `showMessage()` / `showModal()` / `showNotification()` 创建真实反馈控件，`lastMessage()` / `lastModal()` / `lastNotification()` 和 `*Shown` 信号暴露成功结果；host 不可见或尺寸为零时通过 `feedbackFailed` 显式失败。
- `AntConfigProvider::apply()` 一次性原子发布主题模式、主色、基础字号和基础圆角；`AntTheme::themeAboutToChange` / `themeChanged` 是模式变化与仅 token 变化共用的生命周期信号。
- `AntFormItem` 负责命名字段和值属性绑定，`AntForm` 汇总 `values()`、转发 `fieldChanged` 并通过 `finish()` / `finished` 发布完成快照；`AntFormProvider` 用 `QPointer` 注册表单并转发带表单名的字段与完成信号。
- `AntQRGenerator` 明确支持 UTF-8 byte mode V1-V10，按纠错等级和版本精确检查容量，拒绝截断；`AntQRCode` 通过 `encodingValid` / `encodingError` / `encodingFailed` 显式报告失败。`TestAntQRGenerator` 用独立解码逻辑覆盖 40 组最大容量 round-trip 和 UTF-8 round-trip。
- 当前格式 Dock perspective 在解析前后限制 1 MiB 状态、64 层深度、4096 节点、4096 个 Dock ID、1024 字符标识符和 1024 个浮窗快照，并校验节点结构不变量；无效替换不会修改已保存状态或当前布局。legacy 签名快照可导入和存储以供识别或迁移，但当前版本不恢复，恢复请求返回 `unsupported-legacy-format` 并保持布局不变。
- P1/P2 还完成数值极值安全运算、不可变有界图片快照与 Upload LRU、子项目/最低 Qt/CI 契约、System32 DLL 与 URL policy、sanitizer/property 回归、显式源清单、Dock restorer 拆分和 AntLog 有界跨线程队列；完整清单见 `docs/code-audit-optimization.md`。

## 本轮新增组件（2026-04-25，第 2-4 批）

### 第一批：Qt 基础设施（9 个）
- `AntToolButton` — QToolButton + QProxyStyle，dropdown 箭头动画
- `AntScrollArea` — QScrollArea 包裹器，AntScrollBar + 触摸手势滚动 + 可开关鼠标拖动滚动
- `AntPlainTextEdit` — QPlainTextEdit + eventFilter Style，3 变体
- `AntMenuBar` — QMenuBar + drawControl Style
- `AntToolBar` — QToolBar + drawControl Style，浮动阴影
- `AntDockWidget` — QDockWidget，自定义标题栏，Win32 resize；`AntDockManager` 使用自研 splitter/tab 停靠树而非 Qt 原生 dock layout，提供中心标签页停靠、带解析预算和原子拒绝的当前格式命名布局快照、tab 拖动排序、tab/标题栏右键菜单、浮动和 Dock 特性的程序化 API、超过拖动阈值后激活的半透明拖动预览、可通过 `setDropGuideEnabled()` 开关的中心/边缘停靠小方格、按引导位置确定落位、manager-owned 浮动 Dock 原生窗口、AntWindow-style Windows native frame/DWM 圆角阴影和拖回布局；legacy 签名快照只可导入/存储供识别或迁移，不执行恢复
- `AntAutoComplete` — QWidget 组合，弹出建议，键盘导航
- `AntCalendar` — QTableView + Model/View，Day/Month/Year 三态
- `AntColorPicker` — inline trigger + 弹窗编辑器，showText，HS field，RGB/HSV，预设/自定义颜色

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
- `AntApp` — 真实 Message/Modal/Notification 入口、`last*` 句柄和成功/失败信号
- `AntConfigProvider` — 主题模式/主色/字号/圆角四类 token 暂存与原子 `apply()`

### 已有组件增强
- AntInput：新增 `searchMode` + `searchTriggered` 信号
- AntSelect：新增 `editable` 模式，内联过滤
- AntDatePicker：新增 `rangeMode`/`startDate`/`endDate`
- AntTimePicker：新增 `rangeMode`/`startTime`/`endTime`
- AntButtonStyle：修复 `adjusted(0,0,-1,-1)` 导致右/下边框 1px 缺失
- AntAutoComplete：修复 Qt::Popup 抢占焦点问题

## 交互与动效对齐批次（2026-04-30）

本批次按用户发现的问题逐项修复，并在每项完成后单独提交和推送。所有改动均通过 `qt-ant-design-example` Debug 构建和 `20 / 20` CTest 全量验证。

- `AntPopover`：修复 hover 触发时浮窗反复显示/隐藏的问题。
- `AntButton`：loading spinner 改为顺时针旋转，并将可见弧段调整到约 30%。
- `AntPlainTextEdit`：补齐 TextArea 式右下角拖拽缩放。
- `AntInputNumber`：鼠标进入/聚焦时上下箭头控制区以动画显示。
- `AntSlider`：拖动时在当前 handle 上方显示数值浮标并跟随 handle，浮窗箭头与圆角面板使用一体化轮廓绘制且鼠标透明，marks 场景保留标签高度，Range 拖动不再在最左侧绘制多余 pressed/focus 滑块。
- 弹层阴影：`AntTheme::drawEffectShadow()` 改为围绕面板向外绘制柔和多层羽化阴影，并扩大 Dropdown、Menu、Cascader、ColorPicker、Select、DatePicker、TimePicker 等弹层的透明留白，`AntModal` 也保留足够透明阴影边距，避免阴影在弹层边缘被裁出边界线。
- `AntSwitch`：点击时触发灰色 Wave 边缘动效。
- `AntTransfer`：修复列表滚动、滚动后行点击和顶部全选。
- `AntCarousel`：补齐轮播图滑动切换动效。
- `AntTable`：表头排序点击会真正按列重排行数据，并支持升序/降序/取消排序。
- `AntTabs`：Line/Card 样式和 active indicator 动效贴近官方 Ant Design。
- `AntNotification`：增强浮层阴影，补齐按 placement 进入/退出动效，并在销毁清理时保护已销毁 anchor。
- `AntMessage`：补齐 AntD-like move-up/move-down 显示与消失动效，强化气泡阴影，并在点击关闭时透传到底层控件。
- `AntSkeleton`：修复 shimmer 偏移量未参与绘制导致占位符不动的问题。
- `AntSpin`：使用 16ms precise timer 和小角度步进提升动画流畅度。

## AntWindow 桌面体验增强（2026-05-07）

- `AntWindow` 标题栏新增置顶和 Light/Dark 一键切换按钮，图标使用内置官方 Ant Design SVG；pin/theme/minimize/maximize/close 按钮均提供公开显示隐藏 API。
- Windows 下通过 `nativeEvent` + Win32/DWM 路径支持无边框窗口缩放命中测试、标题栏拖拽、最大化按钮 Windows 11 Snap Layout hover、拖拽到屏幕边缘吸附、最大化后标题栏拖拽还原，并用平台宏隔离非 Windows 构建。
- Windows 下接入 DWM 圆角、边框颜色和阴影，新增 `cornerRadius` API 控制窗口圆角大小。
- Windows 10 下 `AntWindow` 使用无 `WS_CAPTION` 的 legacy opaque 直角路径，避免最大化/吸附还原后露出原生最小化/最大化/关闭按钮；该路径不启用 `WA_TranslucentBackground` 或 rounded mask，并通过透明软件阴影宿主窗口绘制外部阴影，以规避旧透明圆角路径在 resize/嵌套弹窗场景中的合成抖动或黑屏；最大化 `WM_NCCALCSIZE` 不再重复扣除 resize border。Windows 11 的 DWM 圆角与 Snap 路径不变。
- `AntWindow` 在 Windows 已显示状态下切换置顶/取消置顶时使用 `SetWindowPos(HWND_TOPMOST/HWND_NOTOPMOST)` 原地更新，避免 `setWindowFlag(Qt::WindowStaysOnTopHint)` 触发 hide/show 导致闪烁。
- 标题栏按钮 hover 状态改为 `AntWindow` 统一维护，content/title/native leave 均会清理旧 hover，避免 hover 或离开主窗口后颜色残留。
- 主题按钮切换 Light/Dark 时使用全窗口截图 crossfade overlay、captured new-frame 和 smootherstep 动画，避免全量主题刷新期间卡顿、高 DPI 放大、黑色圆洞或生硬的纯色扩散；Win10 / Win11 使用同一套 crossfade 动画。
- 示例程序通过 `.rc` 的 RT_MANIFEST 资源嵌入 Windows 10/11 manifest，并在 MSVC 链接阶段使用 `/MANIFEST:NO` 避免 `/MANIFESTINPUT` 与 Visual Studio / Qt 自动 manifest 合并触发 `mt.exe` 失败；`ExampleWindow` 中启用全部标题栏按钮，去掉独立 dark 切换按钮。
- 相关 targeted 验证覆盖 `TestAntQtExtensions|TestAntExampleCloseStress`，包含 Snap hit-test、标题栏 hover 清理、主题切换 crossfade overlay、16ms 动画帧率、220ms 时长、高 DPI 截图比例和无黑洞路径。

## Qt 官方常用接口兼容批次（2026-05-07）

- `AntTypography` 默认垂直居中，新增 alignment 策略、wordWrap、clear 和 `setPixelSize()` API，并让 `setEnabled()` / `setDisabled()` 双向同步 disabled 视觉与交互状态。
- `AntInput` 补齐常用 `QLineEdit` 风格 API/信号，包括 placeholder、readOnly、maxLength、echoMode、alignment、selection、clipboard、undo/redo、return/editing/selection/inputRejected 等。
- `AntInputNumber` / `AntCheckBox` / `AntRadio` / `AntSlider` / `AntProgress` / `AntStatusBar` 补齐常用 Qt 风格状态、交互和消息 API；`AntInputNumber` 默认整数显示，通过 `setDecimals()` / `setPrecision()` 开启小数，并让 decimal value/range/step 与 `precisionChanged` 同步测试覆盖。
- `AntSelect` 补齐常用 `QComboBox` / option 风格 API/信号，包括 add/insert/remove/find、setOptionText/removeOption/optionData/findData、itemText/itemData、currentData、setCurrentText、空列表首次加入数据默认选中首项、activated/textActivated/highlighted/textHighlighted。
- Qt Layout 自适应策略按官方控件基准对齐：`AntInput` / `AntAutoComplete` / `AntMentions` 跟随 `QLineEdit` 横向扩展，`AntSelect` / `AntCascader` / `AntTreeSelect` 跟随 `QComboBox` 的 `Preferred/Fixed`，`AntInputNumber` / `AntDatePicker` / `AntTimePicker` 跟随 Spin/Date/Time 编辑器的 `Minimum/Fixed`，`AntList` / `AntTable` / `AntTree` 跟随 Qt view 双向扩展，`AntTypography` 跟随 `QLabel` 的 height-for-width 换行策略。
- `AntTabs` 添加页面时会自动清理页面根布局的 Qt 默认 margins，避免 Tab 内容页与 `AntCard` / `AntWidget` 内部 padding 形成双重间距；显式设置的自定义 margins 会保留，需要强制清零时可调用 `AntTabs::useTabContentLayout()`。
- `AntDatePicker` / `AntTimePicker` 补齐 `QDateEdit` / `QTimeEdit` 风格 date/time 别名、minimum/maximum range API、range changed 信号，并对越界输入做边界收敛。
- `AntList` / `AntListWidget` 补齐常用 `QListWidget` 风格 API/信号，包括字符串 add/insert/addItems/insertItems、item/text/icon/data/checkState/flags、findItems/sortItems、currentItem/currentRow、selectionMode/selectedItems/setItemSelected、内部滚动、scrollToItem、itemClicked/itemDoubleClicked/itemActivated/itemChanged/current/itemSelection 信号；`AntTable` 补齐 rows/selectRow/currentRowIndex 和行级 tooltip；`AntTree` 继续覆盖 tree 风格 helper。
- `AntMenu` 接入 QWidget `QAction` 体系：`addAction/removeAction` 会同步自绘菜单项，action text/enabled/shortcut 变更会刷新显示，点击菜单项会触发对应 QAction；`AntToolButton` / `AntToolBar` 的默认 action 和 toolbar action 触发行为已有测试保护。
- `AntDesign::configureHighDpi()` + `AntDesign::initialize(&app)` 作为外部项目统一启动入口：前者必须在 `QApplication` 创建前调用，用于 Qt5/Windows 逻辑 High DPI 缩放和 High DPI pixmap；`initialize()` 若在无 `QApplication` 实例时被调用会自动补 High DPI 预配置并返回，创建 app 后仍需再次调用 `initialize(&app)` 注册 `qt_ant_design` 资源、应用内置字体并初始化主题单例，替代分散的 `Q_INIT_RESOURCE` / `AntFont::applyToApplication` / `AntTheme::instance` 调用。
- 相关 targeted 验证覆盖 `TestAntInput|TestAntCheckBox|TestAntDataEntryA|TestAntDataEntryB|TestAntDataDisplayB|TestAntFeedback|TestAntNavigation|TestAntQtExtensions|TestAntTypography|TestAntSelect|TestAntMetaProperties|TestAntRenderSmoke`。

## 子组件/变体完整度（完成于 2026-04-26，状态复核 2026-04-30）

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
- `Form.Provider` — `AntFormProvider` 使用 `QPointer` 注册表单并转发命名字段变化与完成快照；`AntFormItem` / `AntForm` 提供字段绑定、值汇总和 finish 契约

### Phase 3: 高复杂度（3 项）
- `Select multiple/tags` — `Ant::SelectMode` 枚举、`selectedIndices`、tag 渲染、`addTag()`、Backspace 删除
- `Form.List` — `AntFormList` 类、动态增删行、`minCount`/`maxCount`、工厂回调
- `Skeleton.Node` — 作为 `Skeleton.Element::Node` 已在 Phase 1 实现

## 当前组件状态

### 通用

| 组件 | Ant Design 对应目录 | 绘制方式 | 示例覆盖 | 说明 |
| --- | --- | --- | --- | --- |
| `AntButton` | `button` | `QProxyStyle` | 是 | 五种类型、三种尺寸、三种形状 |
| `AntFloatButton` | `float-button` | `QProxyStyle` | 是 | 圆形/方形、Group/BackTop、Badge，点击反馈和不裁切阴影 |
| `AntIcon` | `icon` | `QProxyStyle` | 是 | 831 个官方 SVG 图标资源、字符串名称 API、Outlined/Filled/TwoTone、旋转、spin |
| `AntTypography` | `typography` | `QProxyStyle` | 是 | Title(H1-H5)/Text/Paragraph/Link，对齐策略，setPixelSize |

### 导航

| 组件 | Ant Design 对应目录 | 绘制方式 | 示例覆盖 | 说明 |
| --- | --- | --- | --- | --- |
| `AntAnchor` | `anchor` | 自绘 | 是 | 滚动锚点，active 高亮，暗色示例内容块跟随主题 |
| `AntBreadcrumb` | `breadcrumb` | `QProxyStyle` | 是 | 路径项、分隔符、禁用项 |
| `AntDropdown` | `dropdown` | `QProxyStyle` | 是 | hover/click/contextMenu、placement、arrow |
| `AntMenu` | `menu` | `QProxyStyle` | 是 | vertical/horizontal/inline、明暗主题 |
| `AntPagination` | `pagination` | `QProxyStyle` | 是 | simple/showQuickJumper/showSizeChanger，More Options quick jumper 支持输入页码跳转 |
| `AntSteps` | `steps` | `QProxyStyle` | 是 | 水平/垂直、当前步骤、错误态，首个图标边缘不裁切 |
| `AntTabs` | `tabs` | `QProxyStyle` | 是 | line/card/editable-card，内容页布局 helper |

### 数据录入

| 组件 | Ant Design 对应目录 | 绘制方式 | 示例覆盖 | 说明 |
| --- | --- | --- | --- | --- |
| `AntAutoComplete` | `auto-complete` | `QProxyStyle` | 是 | 建议弹出、键盘导航 |
| `AntCascader` | `cascader` | `QProxyStyle` | 是 | 多列弹出面板、点击/悬停展开 |
| `AntCheckBox` | `checkbox` | `QProxyStyle` | 是 | |
| `AntColorPicker` | `color-picker` | 自绘 | 是 | inline trigger/showText、HS field、RGB/HSV、预设、static getColor()，触发器边框内收防裁切 |
| `AntDatePicker` | `date-picker` | `QProxyStyle` | 是 | 自绘日期弹层、RangePicker |
| `AntDescriptions` | `descriptions` | `QProxyStyle` | 是 | 标题、extra、bordered、vertical |
| `AntForm` | `form` | `QProxyStyle` | 是 | 横向/纵向/行内布局、校验提示、命名字段绑定、值/完成转发、Provider、List |
| `AntInput` | `input` | `QProxyStyle` | 是 | 尺寸、状态、Password/Search、addon |
| `AntInputNumber` | `input-number` | `QProxyStyle` | 是 | QDoubleSpinBox 风格小数/精度、小步进、前后缀 |
| `AntMentions` | `mentions` | `QProxyStyle` | 是 | @提及输入，弹出建议 |
| `AntRadio` | `radio` | `QProxyStyle` | 是 | Radio.Group，ButtonStyle 点击边缘 Wave 扩散 |
| `AntRate` | `rate` | `QProxyStyle` | 是 | count/value/allowHalf/hover 放大/选中星缩放动效 |
| `AntSegmented` | `segmented` | `QProxyStyle` | 是 | 滑动指示器动画、图标/禁用、value/index 选中 API，完整视觉轨道点击命中 |
| `AntSelect` | `select` | `QProxyStyle` | 是 | 尺寸、状态、变体、可编辑模式、Multiple/Tags，option 管理 API |
| `AntSlider` | `slider` | `QProxyStyle` | 是 | Range、marks、拖动浮窗一体化箭头且鼠标透明 |
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
| `AntMessage` | `message` | `QProxyStyle` | 是 | Qt::ToolTip 浮层消息、6 种 placement，点击关闭时透传到底层控件 |
| `AntModal` | `modal` | `QProxyStyle` | 是 | 遮罩层、标题/正文、自定义 footer、命令式 API，柔和外阴影无裁切边界 |
| `AntNotification` | `notification` | `QProxyStyle` | 是 | 多 placement 通知，loading + 自动关闭倒计时进度条 + 手动下载/任务进度条 |
| `AntPopconfirm` | `popconfirm` | `QProxyStyle` | 是 | title/description/ok/cancel/placement，箭头与弹层主体一体化绘制 |
| `AntPopover` | `popover` | `QProxyStyle` | 是 | title/content/action/hover/click/placement |
| `AntProgress` | `progress` | `QProxyStyle` | 是 | line/circle/dashboard |
| `AntResult` | `result` | `QProxyStyle` | 是 | status/title/subTitle/extra，暗色透明状态图标 |
| `AntSkeleton` | `skeleton` | `QProxyStyle` | 是 | active shimmer、头像/标题/段落占位、Element 变体 |
| `AntSpin` | `spin` | `QProxyStyle` | 是 | small/middle/large/percent/delay |
| `AntToolTip` | `tooltip` | `QProxyStyle` | 是 | title/placement/color/arrow/delay，提示浮层鼠标透明 |
| `AntWatermark` | `watermark` | `QProxyStyle` | 是 | 旋转文本平铺、多行、自定义间距，鼠标透明覆盖层 |
| `AntTour` | `tour` | 自绘 | 是 | 遮罩式分步引导、目标高亮，支持从指定步骤启动 |

### 数据展示

| 组件 | Ant Design 对应目录 | 绘制方式 | 示例覆盖 | 说明 |
| --- | --- | --- | --- | --- |
| `AntAvatar` | `avatar` | `QProxyStyle` | 是 | 文本、图标、图片头像、Group |
| `AntBadge` | `badge` | `QProxyStyle` | 是 | count/dot/status/processing/Ribbon |
| `AntCalendar` | `calendar` | `QProxyStyle` | 是 | Day/Month/Year 三态，内部 QTableView viewport 跟随主题 |
| `AntCard` | `card` | `QProxyStyle` | 是 | 封面、extra、action 区、loading、Meta、Grid，主题切换标题 palette |
| `AntCarousel` | `carousel` | 自绘 | 是 | 自动播放、圆点指示器、左右箭头、手动切换、点击事件 |
| `AntCollapse` | `collapse` | 自绘 | 是 | 折叠面板、accordion 模式、动画 |
| `AntEmpty` | `empty` | `QProxyStyle` | 是 | 默认插画、simple 模式 |
| `AntImage` | `image` | 自绘 | 是 | 图片展示、全屏预览、PreviewGroup |
| `AntList` | `list` | `QProxyStyle` | 是 | header/footer/bordered/split/size，item 支持 AntIcon 与 QPixmap/QImage 媒体，内部滚动，平衡选中高亮 inset，`AntListWidget` 覆盖常用 QListWidget-style API |
| `AntPopover` | — | `QProxyStyle` | 是 | 已在反馈类 |
| `AntQRCode` | `qr-code` | `QProxyStyle` | 是 | UTF-8 byte mode V1-V10 + Reed-Solomon、精确容量校验、显式编码失败、独立解码 round-trip 测试 |
| `AntStatistic` | `statistic` | `QProxyStyle` | 是 | title/value/precision/prefix/suffix/Countdown |
| `AntTable` | `table` | `QProxyStyle` | 是 | 排序、选择、分页、行 tooltip、空态插画 |
| `AntTag` | `tag` | `QProxyStyle` | 是 | 13 色预设、closable/checkable/variant |
| `AntTimeline` | `timeline` | `QProxyStyle` | 是 | 垂直/水平、outlined/filled、颜色 |
| `AntToolTip` | — | `QProxyStyle` | 是 | 已在反馈类 |
| `AntTree` | `tree` | `QProxyStyle` | 是 | 展开/收起、选择、复选框、连接线 |

### 布局及其他

| 组件 | Ant Design 对应目录 | 绘制方式 | 示例覆盖 | 说明 |
| --- | --- | --- | --- | --- |
| `AntAffix` | `affix` | QObject 工具 | 是 | 滚动吸附/解除 |
| `AntApp` | `app` | QObject 工具 | 是 | 真实 feedback 入口、`last*`、shown/failed 信号，hidden/zero-size host 显式失败 |
| `AntConfigProvider` | `config-provider` | QObject 工具 | 是 | 四类全局 token 通过 `apply()` 原子发布 |
| `AntDivider` | `divider` | `QProxyStyle` | 是 | 水平/垂直、带标题、虚线 |
| `AntFlex` | `flex` | 自绘 | 是 | 弹性布局、gap/wrap/vertical |
| `AntGrid` (Row/Col) | `grid` | 自绘 | 是 | 24 列栅格、span/offset |
| `AntLayout` | `layout` | `QProxyStyle` | 是 | Header/Footer/Content/Sider，Footer 跟随 layout 背景 token |
| `AntMasonry` | `masonry` | 自绘 | 是 | 瀑布流、最短列优先 |
| `AntSpace` | `space` | `QProxyStyle` | 是 | 水平/垂直间距容器 |
| `AntSplitter` | `splitter` | 自绘 | 是 | 可拖拽分割面板，暗色示例面板使用主题 tint |

### Qt / 桌面扩展组件

| 组件 | 绘制方式 | 示例覆盖 | 说明 |
| --- | --- | --- | --- |
| `AntWidget` | — | 是 | 基础 QWidget，自动主题切换 |
| `AntWindow` | `QProxyStyle` | 是 | 无边框窗口，自定义标题栏，Win11 DWM 圆角/Snap 路径，Win10 opaque 直角 legacy frame + 软件阴影宿主，标题栏按钮 API，主题切换遮罩动画 |
| `AntDialog` | `QProxyStyle` | 是 | 无边框 QDialog 替代控件，Ant token 标题栏可响应主题切换，提供 contentWidget 内容宿主和关闭按钮 hover 状态 |
| `AntInputDialog` | `AntDialog` + `QProxyStyle` | 是 | QInputDialog 替代控件，复用 AntDialog 标题栏/阴影/主题同步，支持文本、整数、浮点和下拉项输入、按钮文案、NoButtons / PlainText 选项与 changed/selected 信号 |
| `AntDockWidget` | 自绘 | 是 | 可停靠面板，Win32 resize；配套 `AntDockManager` 使用自研 splitter/tab 停靠树，当前格式命名快照带资源预算和无效替换原子拒绝，另提供中心标签页停靠、tab 拖动排序、tab/标题栏右键菜单、程序化浮动和 Dock 特性 API、阈值激活的半透明拖动预览、可开关的中心/边缘停靠小方格、按引导位置确定落位、manager-owned 浮动 Dock 原生窗口、AntWindow-style Windows native frame/DWM 圆角阴影和拖回布局；legacy 签名快照可导入/存储供识别或迁移，但当前版本明确不恢复 |
| `AntStatusBar` | `QProxyStyle` | 是 | 状态栏 |
| `AntRibbon` | 自绘 | 是 | Ribbon 顶部命令区，Page/Group、大/小 action、嵌入 Ant/Qt 控件，支持 Tab 指示条/折叠动画、折叠弹出并可接入 AntWindow |
| `AntScrollBar` | `QProxyStyle` | 是 | 8px 细滚动条，自动隐藏，示例 QScrollArea 暗色 surface 跟随主题 |
| `AntScrollArea` | — | 是 | QScrollArea + AntScrollBar + 触摸手势滚动 + 可开关鼠标拖动滚动 |
| `AntStackedWidget` | `QProxyStyle` | 是 | QStackedWidget 风格页面栈，Ant token 背景/边框，Outlined/Filled/Borderless 变体 |
| `AntFileDialog` | `AntDialog` + `QProxyStyle` | 是 | 完全自定义 Ant Design 文件对话框，左侧常用位置 + 默认折叠目录树，右侧文件列表使用 QFileSystemModel/QTreeView，配套 Ant 输入/下拉/按钮和 token 绘制面板 |
| `AntMenuBar` | `QProxyStyle` | 是 | 菜单栏 |
| `AntToolBar` | `QProxyStyle` | 是 | 工具栏 |
| `AntToolButton` | `QProxyStyle` | 是 | 带下拉菜单的按钮 |
| `AntPlainTextEdit` | `QProxyStyle` | 是 | 多行文本编辑器 |
| `AntLog` | 自绘 | 是 | 日志输出控件 |
| `AntNav` | 自绘组合 | 是 | 侧边栏/选择栏容器，管理分组标题、AntNavItem 图标/媒体、选中状态、当前项信号和 AntScrollBar |
| `AntNavItem` | 自绘 | 是 | 侧边栏导航项，支持 AntIcon 与 QPixmap/QImage 媒体，active/hover 状态，clicked 信号 |
| `AntWave` | — | — | 内部涟漪动画 overlay（core/），不计入 `src/widgets` 公开组件统计 |

## 开发规范

- 支持 Qt6 / Qt5 自动识别；最低版本为 Qt 6.5.0 或 Qt 5.15.2，源码配置与安装包 `find_package(qt-ant-design CONFIG REQUIRED)` 都会执行同等检查。CMake 使用：
  - `find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets Svg)`
  - `find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Svg)`
- 库类型通过标准 `BUILD_SHARED_LIBS` 控制；默认静态库，传入 `-DBUILD_SHARED_LIBS=ON` 构建动态库
- 示例、测试和逐控件 smoke 使用 `QT_ANT_DESIGN_BUILD_EXAMPLES`、`QT_ANT_DESIGN_BUILD_TESTS`、`QT_ANT_DESIGN_BUILD_WIDGET_SMOKE_TESTS`；顶层工程默认开启，作为 `add_subdirectory()` 子项目时默认关闭，只构建库。宿主若显式开启子项目测试并希望从宿主构建根通过 CTest 发现它们，必须在 `add_subdirectory()` 前调用 `enable_testing()`。Windows 顶层安装示例的部署由 `QT_ANT_DESIGN_DEPLOY_EXAMPLE` 控制
- sanitizer 使用 `QT_ANT_DESIGN_ENABLE_ADDRESS_SANITIZER` 与 `QT_ANT_DESIGN_ENABLE_UNDEFINED_SANITIZER`；两者默认关闭，MSVC 不支持本项目的 UBSan 配置。编译插桩只应用于 `qt-ant-design` 库编译单元，必要链接选项传播给 build-tree consumer，但测试/宿主不会自动获得编译插桩；MSVC 仅对库移除 `/RTC` 并禁用库内 STL string/vector annotations，以兼容未插桩父目标
- Windows 动态库使用 `core/QtAntDesignExport.h` 中的 `QT_ANT_DESIGN_EXPORT` 显式导出公开类
- C++ 标准为 `C++17`
- 所有核心视觉值优先从 `AntTheme` / `AntPalette` 获取，不直接散落硬编码
- 组件公共枚举与通用类型统一放在 `src/core/AntTypes.h`
- 新增或重构组件时，优先采用 `QProxyStyle` 架构：
  - 组件类负责公开 API、状态维护、信号、子控件管理
  - `Ant[Component]Style` 负责 `drawPrimitive` / `drawControl` / `drawComplexControl` / `sizeFromContents`
- Qt 控件移植为 Ant 控件的详细落地规范见：`docs/qt-control-porting-guidelines.md`
- 所有 `Ant[Component]Style` 文件统一放在 `src/styles/`
- 组件源文件引用 Style 时使用：
  - `#include "../styles/Ant[Component]Style.h"`
- CMake 安装时，公开 Style 头文件需安装到：
  - `install/include/qt-ant-design/styles/`
- 已迁移到 `QProxyStyle` 的组件，应在构造函数中安装独立 Style，并通过 `AntStyleBase::connectThemeUpdate<T>()` 接入局部主题刷新：
  - 通用 `themeAboutToChange` 阶段缓存旧 `sizeHint` / `minimumSizeHint`，同时覆盖模式切换与 ConfigProvider token-only 更新
  - 通用 `themeChanged` 阶段只刷新该 Style 已 polish 的目标控件；共享 Style 可扫描其 QWidget parent 子树，只有无法解析本地目标时才兜底扫描全局 widgets
  - `onThemeUpdate()` 默认执行 `update`，仅当主题前后的尺寸 hint 变化时才调用 `updateGeometry`
- 纯容器/自绘非 QProxyStyle 组件（如 AntScrollArea、AntColorPicker）可不含独立 Style 类
- 主题切换统一监听：
  - Style 类优先使用 `AntStyleBase::connectThemeUpdate<T>()`
  - 组件自身凡需响应颜色、字号、圆角等 token 变化，应监听 `AntTheme::themeChanged`；只关心模式枚举时才额外使用 `themeModeChanged`
  - 需要比较主题更新前后尺寸或状态时监听通用 `AntTheme::themeAboutToChange`，不要仅监听 `themeModeAboutToChange`
- 每次新增组件后，必须同步更新：
  - `AGENTS.md`
  - `README.md`
  - `examples/ExampleWindow.cpp`

### 顶层窗口圆角策略

`AntWindow` 和 `AntDialog` 的窗口边角处理需要保持同一套平台策略：

- Windows 11 及以上：允许使用 `WA_TranslucentBackground` 和 alpha-painted rounded corners。`AntWindow` 还会结合 DWM corner preference、Snap Layout native caption 路径和 corner smoother；`AntDialog` 作为轻量 frameless dialog，使用同等 Win11 判定并由 `AntDialogStyle` 绘制透明圆角。
- Windows 10：走 legacy opaque path，默认不启用 `WA_TranslucentBackground`，窗口本体保持直角，避免 Win10 DWM 在透明背景、圆角裁剪和反复 resize/嵌套弹窗组合下出现合成抖动或黑屏类问题。
- 非 Windows：按透明圆角路径处理，由 Style 的圆角绘制和透明角像素完成视觉裁切。
- 测试或诊断需要强制 legacy path 时，`AntWindow` 使用动态属性 `antWindowForceLegacyFramePolicy`，`AntDialog` 使用动态属性 `antDialogForceLegacyFramePolicy`；环境变量 `QT_ANT_DESIGN_FORCE_LEGACY_FRAME=1` 也会让 `AntDialog` 使用 Win10-style 直角路径。
- 后续新增顶层 frameless 控件或弹窗时，优先复用上述策略，不要在 Win10 上单独开启透明圆角或 `setMask` 圆角裁切。

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
- `connectThemeUpdate<T>()` — 连接主题切换信号并局部收集目标控件，避免大窗口主题切换时每个 Style 全局扫描所有 widgets
- `onThemeUpdate(w)` — 主题切换时的默认行为（尺寸 hint 变化时 `updateGeometry`，始终 `update`）
- `drawCrispRoundedRect(painter, rect, pen, brush, rx, ry)` — 0.5px 子像素偏移绘制圆角矩形，解决边框锯齿问题

## 示例程序

当前 `examples/ExampleWindow.cpp` 已覆盖 `89 / 89` 个公开组件，另有 `Showcase` 页面用于首页展示控件对标。`AntDockManager` 合并在 DockWidget 示例页展示，左侧导航与右侧页面一一对应。

示例程序架构：
- `ExampleWindow` 继承 `AntWindow`（无边框窗口，自定义标题栏）
- 使用 `AntWidget` 作为侧边栏和内容区容器
- 使用 `AntTypography` 替代 `QLabel`，通过 `setTitle()` / `setParagraph()` / `setType()` 实现主题感知
- 使用 `AntScrollBar` 替代原生滚动条

## 视觉审查

- 审查入口：`docs/visual-audit.md`
- 参考来源：官方 Ant Design 页面；需要固定快照时放到 `build/` 临时目录
- 审查顺序：先基础组件（Typography/Icon/Button/Tag/Badge），再输入、弹层反馈、数据展示、导航布局、复杂与 Qt 扩展组件
- 每个控件需覆盖亮色/暗色、默认/hover/active/focus/disabled、状态色、尺寸、间距、圆角、阴影和弹层行为
- 视觉对比按单控件闭环推进：先读 Ant Design 源码/token，再编译 Qt 示例，分别截图参考页和 Qt 页，生成 side-by-side 对比图，归因差异，修复控件自身问题，最后更新 `docs/visual-audit.md`
- 第一轮静态对比完成后，进入二轮状态态审计：为单控件生成 light/dark 状态矩阵，覆盖 hover/active/focus-visible/disabled/loading/status 等真实视觉态，证据记录在 `docs/visual-audit.md` 的 `Second-Pass State Audit`
- 参考页截图使用 Playwright，例如：
  - `npx playwright screenshot --wait-for-timeout=4000 --viewport-size "1280,900" "https://ant.design/components/overview-cn/" build/<component>-reference-full.png`
- Qt 侧截图优先使用 `build/visual-capture/` 下的临时 helper 输出 `build/<component>-qt.png`；截图 helper、PNG、拼图都属于 `build/` 产物，不提交
- README 组件截图画廊除外：`resources/images/components/` 下的 Light/Dark PNG 是文档资产，需要提交；弹层/反馈控件应截取打开或激活态，而不是只展示触发按钮
- Windows 下 Qt `offscreen` 平台可能把文字渲染成方块；遇到时使用原生 Windows 平台截图。
- 差异必须先归因：控件本体问题在当前控件修；容器、页面边距、卡片留白等归到对应组件审查；参考示例缺状态则先补示例/记录 `Needs fix`
- 状态含义：`Pass` 表示已截图对比且无控件本体差异；`Needs visual QA` 表示状态已覆盖但待截图确认；`Needs fix` 表示仍有控件差异；`Blocked` 表示无法截图或参考缺失
- 当前矩阵状态：可对比的 Ant Design 标准组件为 `Pass`，Qt 桌面扩展为 `Local Pass`。后续视觉工作按用户发现的问题逐项复核，不再从“待审计队列”推进。

## 构建与安装

- 手工 `cmake -S/-B` 构建支持 CMake 3.16+；`CMakePresets.json` 使用 schema v3，需要 CMake 3.21+。
- 当前 CI 定义的矩阵为：Windows Qt 5.15.2 Debug static、Qt 6.7.3 Debug static、Qt 6.7.3 Release shared；Linux Qt 5.15.2 Debug static + ASan/UBSan、Qt 5.15.2 Release shared；macOS Qt 6.7.3 Release shared。所有非 sanitizer 配置执行安装；sanitizer 配置只构建和测试、刻意跳过安装。Windows hosted runner 的非交互会话会无诊断终止聚合 GUI 套件 `TestAntDataEntryB`、`TestAntDataDisplayB`、`TestAntQtExtensions`，因此远端 Windows 作业排除这三项，其跨平台路径由 Linux sanitizer 作业覆盖，完整套件仍保留在本地 Windows 验证中。Windows 作业还执行安装示例 smoke 与独立 installed consumer。该描述表示工作流已配置，不代表尚未实际触发的新远程矩阵已通过。

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

项目使用 QTest 与 CTest 脚本进行自动化测试，覆盖所有 89 个公开组件的属性、getter/setter、信号验证、生命周期压力场景和安装消费方验证。

- **测试框架**：Qt Test（QTest + QSignalSpy，跟随自动检测到的 Qt 主版本）
- **测试数量**：Windows 顶层默认配置当前有 51 个深度/系统 CTest 条目；其中 `TestAntHighDpiScaling` 注册 4 个缩放/初始化条目，`TestAntQtVersionVisualParity` 另注册 2 个 atlas scale 条目，并包含 QR、Dock 限制、System32 DLL 路径和 CI policy 等门禁。启用 `QT_ANT_DESIGN_BUILD_WIDGET_SMOKE_TESTS=ON` 时追加 104 个 `WidgetSmoke.<Type>` 逐控件编译/构造/基础渲染条目，共 155 个。非 Windows 不注册 `TestAntExampleGuiSubsystem`，因此相同顶层选项下少 1 项。
- **覆盖组件**：89 个公开组件全部覆盖，内部 helper 随宿主组件测试；逐控件覆盖矩阵见 `docs/reliability-coverage.md`
- **运行方式**：`ctest -C Debug --output-on-failure`
- **验证结果**：`2026-05-30` 深度/系统基线 `37 / 37` CTest 目标通过；`2026-06-01` Qt5/Qt6 视觉一致性、视觉 atlas、度量审计、Windows High DPI、无 QSS 门禁和 example 页面遍历定向验证通过；`2026-06-24` 逐控件 `widget-smoke` CTest `104 / 104` 通过；`2026-07-16` P0 历史全量 `153 / 153` 通过，P1/P2 完成后的当前 Qt6 Debug 全量 `155 / 155` 在 `379.87s` 内通过，Qt5 最终定向 `5 / 5` 通过，MSVC ASan 关键目标 `9 / 9` 通过，shared Release 安装 consumer `1 / 1` 通过。本地 UBSan 尚未覆盖；新 CI 矩阵是否通过以对应 GitHub Actions 运行结果为准。
- **原生输入专项**：`TestAntQtExtensions` 的 Win32 `SendInput` 桌面输入路径默认关闭；需要真实桌面输入验证时显式设置 `QT_ANT_DESIGN_ENABLE_NATIVE_INPUT_TESTS=1`

### 测试文件结构

```
tests/
├── CMakeLists.txt              # 测试目标注册
├── TestAntTypes.cpp            # 核心枚举值验证
├── TestAntButton.cpp           # Button 属性/信号
├── TestAntIcon.cpp             # Icon 属性/信号
├── TestAntTypography.cpp       # Typography 属性/信号
├── TestAntFloatButton.cpp      # FloatButton 属性/信号
├── TestAntBadge.cpp            # Badge 属性/信号
├── TestAntCheckBox.cpp         # Checkbox 属性/信号
├── TestAntSwitch.cpp           # Switch 属性/信号
├── TestAntSelect.cpp           # Select 单选/多选/标签
├── TestAntInput.cpp            # Input 属性/信号
├── TestAntDataEntryA.cpp       # InputNumber, Radio, Slider, Rate, Segmented, AutoComplete
├── TestAntDataEntryB.cpp       # Cascader, DatePicker, TimePicker, Mentions, Transfer, TreeSelect, Upload
├── TestAntDataDisplayA.cpp     # Avatar, Card, Statistic, Calendar, Image, Empty
├── TestAntDataDisplayB.cpp     # List, Table, Tree, Timeline, Descriptions, QRCode, Watermark, Carousel, Collapse
├── TestAntQRGenerator.cpp      # Independent QR matrix decoder, V1-V10 maximum-capacity and UTF-8 round-trip coverage
├── TestAntTag.cpp              # Tag 属性/信号
├── TestAntFeedback.cpp         # Alert, Drawer, Message, Notification, Popconfirm, Popover, Progress, Result, Skeleton, Spin, Tooltip, Tour
├── TestAntModal.cpp            # Modal 属性/命令式 API
├── TestAntNavigation.cpp       # Breadcrumb, Dropdown, Menu, Pagination, Steps, Tabs, Anchor
├── TestAntLayout.cpp           # Divider, Flex, Grid, Space, Layout, Masonry, Affix
├── TestAntQtExtensions.cpp     # App, ConfigProvider, Form, Log, Nav, NavItem, PlainTextEdit, ScrollArea, ScrollBar, Splitter, StatusBar, ToolButton, ToolBar, MenuBar, DockWidget, Widget, Window, ColorPicker
├── TestAntDockPerspectiveLimits.cpp # Current-format perspective resource budgets, malformed streams, and atomic rejection
├── TestAntAliases.cpp          # Qt-style alias headers and type mapping coverage
├── TestAntObjectTree.cpp       # Public widget parent ownership, style ownership, and parent-driven destruction
├── TestAntChildOwnership.cpp   # Assigned child QWidget adoption and host-driven destruction coverage
├── TestAntThemeLifecycle.cpp   # Global theme switching, destruction, and open-popup theme lifecycle coverage
├── TestAntMetaProperties.cpp   # Every public control's own Q_PROPERTY read/write + NOTIFY coverage through Qt meta-object APIs
├── TestAntInteractions.cpp     # Complex popup/input/upload interactions through real QTest mouse, keyboard, drag/drop events
├── TestAntAdvancedInteractions.cpp # Advanced data-display, navigation, color, and feedback interaction flows
├── TestAntMotion.cpp           # Popup motion lifecycle, placement mapping, and Wave-triggered interaction coverage
├── TestAntPopupLifecycle.cpp   # Popup open/close and owner/target destruction lifecycle coverage
├── TestAntStressLifecycle.cpp  # Repeated theme, popup, and transient feedback lifecycle stress coverage
├── TestAntRenderSmoke.cpp      # Public widget render smoke coverage with nonblank checks for direct paint surfaces
├── TestAntVisualRegression.cpp # Token-color, input handler, data display, selection, tag/badge, feedback surface, navigation/layout/popup, and light/dark visual guards
├── TestAntQtVersionVisualParity.cpp # Optional Qt5-vs-Qt6 atlas export/comparison plus public-header atlas coverage guard
├── TestAntQtVersionMetricAudit.cpp # Optional Qt5-vs-Qt6 QStyle/QPalette/font/geometry metric export/comparison
├── TestAntWindowsSystemLibrary.cpp # Windows DLL loader rejects non-System32 paths; non-Windows no-op contract
├── TestAntHighDpiScaling.cpp   # Windows/Qt High DPI startup, initialize default, and logical-geometry checks at 1.0, 1.25, and 1.5 scale factors
├── TestAntInstallConsumer.cmake # Installed package can be found and linked by an external CMake consumer
├── TestAntBuildSystem.cmake # Build options cover Qt major detection and static/shared library settings
├── TestAntCiPolicy.cmake    # CI action pinning, permissions, matrix, sanitizer, and install-smoke policy guard
├── TestAntNoStyleSheetUsage.cmake # Source/example/test/resource guard against QSS/QStyleSheet usage
├── TestAntExampleGuiSubsystem.cmake # Windows example executable uses GUI subsystem, not console subsystem
├── TestAntExampleCloseStress   # Example executable theme-cycle and auto-close CTest target
├── TestAntExamplePageTraversal # Real example page traversal smoke target
└── TestAntCoverageInventory.cpp # Public Ant*.h coverage guard for lifecycle and meta-property tests
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
    auto* w = new AntXxx;           // 属性测试保持简单生命周期，对象树析构由 TestAntObjectTree 覆盖
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

1. **单测试函数模式**：属性测试优先保持单个 `propertiesAndSignals()` 函数，减少 UI 状态、主题状态和事件队列互相影响。

2. **对象树析构**：`AntStyleBase` 会为 `QProxyStyle` 创建独立 base style；组件 style 应挂到对应控件 parent。新增生命周期测试应优先使用 parent-owned 控件并删除父对象，参考 `TestAntObjectTree`。

3. **信号验证**：设置新值必须与当前值不同，否则 setter 会提前返回不发射信号。

4. **分页/约束属性**：某些属性有依赖约束（如 `AntTable::setCurrentPage` 受 `totalPages()` 约束，`AntPagination::setCurrent` 受 `total` 约束），需要先设置依赖属性。

5. **外部 QObject 生命周期**：不拥有的 target、anchor、viewport、content widget 或 Dock watcher 不得保存为裸成员指针；使用 `QPointer` / `destroyed` 清理，并在 owner 析构前解除由 owner 安装的 event filter。专项回归应覆盖“外部对象先销毁”和“owner 在活动交互中销毁”两种顺序，参考 `TestAntStressLifecycle`。

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

### 测试范围规则

- **修改控件后只测试该控件对应的测试目标**，不要默认运行 `ctest -C Debug` 全量。例如改动 `AntWindow`/`AntDockManager` 时只跑 `TestAntQtExtensions`；改动 `AntButton` 时只跑 `TestAntButton`。
- 多控件改动时，只跑与改动控件直接相关的 CTest 目标集合（参考上面的“测试文件结构”查目标归属）。
- 仅当用户明确要求“跑全量测试”或改动确实涉及全局基础设施（`AntTheme`、`AntStyleBase`、`core/` 公共头文件等）时才执行 `ctest -C Debug --output-on-failure`。
- 同样适用于 build：优先 `cmake --build build --config Debug --target <对应测试目标>`，避免触发全部测试目标的链接。
