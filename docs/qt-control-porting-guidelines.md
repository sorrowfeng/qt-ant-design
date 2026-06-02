# Qt 控件移植为 Ant 控件规范

本文档用于指导后续将 Qt Widgets 控件移植、包装或重构为 `qt-ant-design` 的 `Ant*` 控件时必须遵守的工程规范。目标是让新增控件在 API、主题、绘制、测试、示例和安装层面都与现有项目框架保持一致，并避免重新引入 QSS / stylesheet 依赖。

## 适用范围

- 新增一个公开 `Ant*` 控件。
- 将已有 `paintEvent` 控件迁移到 `QProxyStyle`。
- 将 Qt 原生控件包装成 Ant Design 风格控件。
- 为复杂控件补充内部 Qt 子控件，例如 `QLineEdit`、`QTabWidget`、`QTabBar`、`QToolButton`、`QScrollArea`、`QTableView`。
- 清理已有控件中的 `setStyleSheet()`、硬编码颜色、散落 palette 或重复主题刷新逻辑。

## 总体原则

1. 优先使用 `QProxyStyle` 架构承载主绘制链路。
2. 组件类只负责公开 API、状态维护、信号、子控件管理和交互逻辑。
3. `Ant[Component]Style` 负责绘制、尺寸、子控件外观和主题驱动刷新。
4. 所有核心视觉值必须来自 `AntTheme` / `AntPalette` / token，不在组件中散落硬编码颜色。
5. 生产源码禁止用 `setStyleSheet()` 实现核心外观。
6. 示例页保持零样式操作，不在示例代码中调用 `setStyleSheet()`、`setFont()`、`QPalette` 或 `setAutoFillBackground()` 来修饰控件。
7. 新增或重构后必须同步测试、示例、文档和安装导出状态。

## 命名与文件布局

- 公开控件命名为 `AntXxx`，文件放在 `src/widgets/AntXxx.h` 与 `src/widgets/AntXxx.cpp`。
- 对应 Style 命名为 `AntXxxStyle`，文件放在 `src/styles/AntXxxStyle.h` 与 `src/styles/AntXxxStyle.cpp`。
- 组件源文件引用 Style 时使用：

```cpp
#include "../styles/AntXxxStyle.h"
```

- 公共枚举、尺寸、状态、变体等通用类型优先放入 `src/core/AntTypes.h`。
- Windows 动态库公开类使用 `QT_ANT_DESIGN_EXPORT`。
- 仅内部使用的 helper 不导出 ABI，不安装公开头文件，也不进入 README 公开组件清单。
- 与 Qt 常用控件对应但命名不直观时，可提供轻量别名头；仅大小写差异时优先采用 Qt 风格命名，不保留重复 alias。

## 绘制架构选择

项目使用三类绘制模式，新增或重构时按以下优先级选择。

| 优先级 | 模式 | 适用场景 | 要求 |
| --- | --- | --- | --- |
| 1 | `AntStyleBase` + `eventFilter` | 自定义 `QWidget` 子类，主绘制可整体接管 | Style 安装 paint filter，重写 `drawWidget()` |
| 2 | `QProxyStyle::drawControl` / `drawComplexControl` / `drawPrimitive` | 继承 Qt 原生控件或需要接管 Qt 子控件外观 | 使用标准 `QStyleOption`，保留 Qt 交互和可访问性 |
| 3 | 控件自身 `paintEvent` | 简单容器、工具类、复杂自绘场景，独立 Style 收益较低 | 仍必须使用 token，且主题刷新路径清晰 |

新增控件默认选择 Pattern A。继承自 `QPushButton`、`QCheckBox`、`QMenuBar`、`QToolButton`、`QTabBar` 等标准 Qt 控件时，优先选择 Pattern B。

## Style 实现规范

### Pattern A 模板

```cpp
class QT_ANT_DESIGN_EXPORT AntXxxStyle : public AntStyleBase
{
    Q_OBJECT
public:
    explicit AntXxxStyle(QStyle* style = nullptr);

    void polish(QWidget* widget) override;
    void unpolish(QWidget* widget) override;
    bool drawWidget(QWidget* widget, QPaintEvent* event) override;
};
```

```cpp
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
```

### Pattern B 要求

- 使用 Qt 已有的 `QStyleOptionButton`、`QStyleOptionComboBox`、`QStyleOptionTab`、`QStyleOptionToolButton` 等，不自行猜测状态。
- 根据需要重写：
  - `drawPrimitive()`
  - `drawControl()`
  - `drawComplexControl()`
  - `subControlRect()`
  - `sizeFromContents()`
  - `pixelMetric()`
- 对 `QTabWidget`、`QTabBar`、`QToolButton`、`QScrollBar`、`QSplitter` 这类内部 Qt 子控件，应安装专用 Style 或让父控件 Style 能识别管理范围。
- Style 不应修改业务状态；只允许在 `polish()` / `unpolish()` 中做绘制所需的轻量属性、hover、filter、palette 或 size policy 设置。

## Style 安装与生命周期

- 每个控件构造时安装自己的 Style：

```cpp
setStyle(new AntXxxStyle(style()));
```

- 当 Style 需要 parent 管理生命周期时，创建后显式 `setParent(widget)`。
- 不要在多个不同控件间共享同一个带状态的 Style 实例。
- Style 的刷新路径优先使用 `AntStyleBase::connectThemeUpdate<T>()`，保证只刷新本 Style 已 polish 的目标控件或本地子树。
- 主题切换时默认顺序为：
  1. 缓存旧 `sizeHint()` / `minimumSizeHint()`。
  2. `polish` 或更新必要子控件状态。
  3. 仅在尺寸 hint 变化时调用 `updateGeometry()`。
  4. 调用 `update()` 或局部 `update(rect)`。
- 避免在主题切换时全局扫描所有 widget；只有无法解析局部目标时才作为兜底。

## 主题与视觉 Token

- 颜色、圆角、阴影、间距、字体尺寸、状态色优先从 `AntTheme::instance().tokens()` 或现有 `AntPalette` 获取。
- 支持 Light / Dark 两套主题，不能只验证亮色。
- hover、pressed、focus、disabled、checked、selected、loading、error、warning、success 等状态必须有明确 token 映射。
- 不新增一次性硬编码颜色；确需新增视觉值时，应先判断是否应进入 token 或组件局部常量。
- 内部 Qt 编辑器或文本控件可用 `QPalette` 同步文本、placeholder、选区、base、window 等颜色，但 palette 只用于承载 Qt 原生文本/选择机制，不替代主绘制。

## 禁止 QSS / Stylesheet

生产源码中不得使用以下方式实现控件核心外观：

```cpp
setStyleSheet(...);
child->setStyleSheet(...);
```

替代方案：

- 背景、边框、圆角：放入 `AntXxxStyle` 或控件 `paintEvent`。
- 内部 `QLineEdit` / `QTextEdit` 无边框透明底：使用 `setFrame(false)`、`QPalette`、`Qt::WA_TranslucentBackground`、`setAutoFillBackground(false)`。
- `QTabWidget` / `QTabBar` / `QToolButton` 外观：使用专用 `QProxyStyle`。
- hover / active / focus：用事件、属性、Qt state flag 或控件状态字段驱动 repaint。
- 示例页视觉：通过 Ant 控件自身 API 表达，不在示例中用 QSS 修饰。

清理完成后至少运行：

```powershell
rg -n "setStyleSheet\(" src\widgets src\styles src\core
```

该命令在生产源码范围内应无输出。

## API 与 Qt 兼容性

- 公开 API 尽量贴近 Ant Design 概念，同时补齐 Qt 用户自然预期的常用接口。
- 常用属性应提供 getter / setter / signal，并在必要时声明 `Q_PROPERTY`。
- 与 Qt 原生控件相同语义的 API 应保持命名直观，例如 `setCurrentIndex()`、`currentText()`、`addItem()`、`setReadOnly()`、`setPlaceholderText()`。
- 选中、当前项、激活、编辑完成、文本变化等行为应发出 Qt 用户预期的信号。
- 保持二进制兼容意识：公开头文件新增类型和导出类前，确认它们确实属于公共 API。

## 子控件与组合控件

- 子控件优先使用现有 Ant 控件，例如文本用 `AntTypography`，滚动条用 `AntScrollBar`。
- 必须使用 Qt 原生子控件时，应把它们的视觉纳入父组件 Style、专用 Style 或 palette 统一管理。
- Popup、dropdown、tooltip、modal 等浮层应保持：
  - 跟随 anchor 移动、隐藏、销毁。
  - 多窗口作用域隔离。
  - 关闭和销毁路径不访问已释放对象。
  - 阴影和透明边距不被裁切。
- 不把内部 helper 当成公开组件安装或导出，除非已经明确纳入 ABI。

## 布局与尺寸

- `sizeHint()`、`minimumSizeHint()`、`heightForWidth()` 与 Qt 同类控件预期一致。
- `sizePolicy` 应参考对应 Qt 原生控件，不随意设置双向扩展。
- 间距、padding、边框宽度从 token 或组件局部 metrics 统一计算。
- 动态内容、hover、loading、图标变化不得导致布局抖动。
- Tab、列表、表格、树、分页等复杂控件应缓存布局矩形，避免 paint、hit-test、size hint 各算一套。

## 绘制细节

- 使用 `QPainter` 时按需开启 `Antialiasing`、`TextAntialiasing`、`SmoothPixmapTransform`。
- 边框优先使用项目已有 crisp helper，例如 `drawCrispRoundedRect()`。
- 图标优先使用内置 Ant Design SVG 资源或 `AntIcon`，不要新建临时 SVG 字符串。
- 高 DPI 下 pixmap、阴影、截图 overlay 必须使用 device pixel ratio 安全的尺寸：按 `logicalSize * devicePixelRatioF()` 建立像素缓存，`setDevicePixelRatio()` 后再绘制回 Qt 逻辑坐标。
- Qt5 / Windows 消费端必须在 `QApplication` 前调用 `AntDesign::configureHighDpi()`；`AntDesign::initialize()` 会在无 `QApplication` 实例时自动补一次 High DPI 预配置，但创建 app 后仍需再次调用 `initialize(&app)` 完成资源、字体和主题初始化。新增 example 或测试入口也必须保持这个启动顺序。
- 文本颜色、禁用态透明度、选中态背景与状态色要同时覆盖亮暗主题。
- 动效使用 `QPropertyAnimation`、`QVariantAnimation` 或定时器时，应在隐藏、禁用、析构路径停止或断开。

## 性能要求

- 主题切换、大批量窗口、大量子控件场景不能依赖全局 repolish。
- 尺寸和绘制 metrics 应缓存，状态变化后按需失效。
- hover、press、selection、loading、progress 动画优先局部 repaint。
- 大批量 append、列表刷新、popup filter 等场景应批处理或 coalesce 到下一轮事件循环。
- 隐藏控件不应继续跑高频动画 timer。
- 不在 paint 中分配大量对象、读取文件、重建复杂布局或修改 widget 树。

## 示例要求

新增公开控件或可见能力后必须同步 `examples/ExampleWindow.cpp`。

- 示例页面使用 `AntWidget` / `AntCard` / `AntTypography` 等项目控件承载。
- 示例页不得通过样式表、palette、手动字体或背景填充来修饰控件。
- 每个公开组件应有独立示例页；组合展示可复用已有页，但 README 和状态文档要说明归属。
- 示例应覆盖常用尺寸、状态、禁用、交互和暗色主题下可见差异。

## 测试要求

每次移植或重构至少评估以下测试面：

- API / property / signal：getter、setter、边界值、信号次数。
- 交互：鼠标、键盘、focus、hover、popup 打开关闭。
- 主题：Light / Dark 切换后颜色、palette、geometry 和 repaint。
- 生命周期：parent-owned 删除、popup anchor 销毁、重复 show/hide、对象树析构。
- 渲染烟测：控件非空绘制，亮暗主题不崩溃。
- 安装消费方：公开头、CMake target、动态库导出。

优先扩展已有测试文件，不为窄场景创建重复测试可执行文件。涉及桌面输入时避免默认 CTest 依赖真实 `SendInput` 路径，真实桌面路径应被显式开关保护。

## 文档与状态同步

完成新增控件或迁移后，根据影响范围同步：

- `AGENTS.md`
- `README.md`
- `docs/project-status.md`
- `docs/reliability-coverage.md`
- `docs/visual-audit.md`
- `docs/issue-log.md`
- README 组件截图资源

如果只是内部实现迁移，至少更新对应 issue 或状态记录，说明旧实现、迁移方式、验证命令和剩余风险。

## 安装与构建

- 新增 `.h/.cpp` 必须加入 CMake 构建。
- 公开 widget 头安装到 `install/include/qt-ant-design/widgets/`。
- 公开 style 头安装到 `install/include/qt-ant-design/styles/`。
- 内部 helper 不安装，不导出，不进入 public CMake interface。
- Qt5 / Qt6 都应能编译，平台相关代码必须用平台宏隔离。
- 静态库和动态库路径都要考虑，动态库公开类必须带 `QT_ANT_DESIGN_EXPORT`。

## 推荐移植流程

1. 对照 Qt 原生控件 API、Ant Design 规范和现有同类 `Ant*` 控件。
2. 明确控件边界：公开 API、内部 helper、子控件、popup、数据模型。
3. 选择绘制模式，优先设计 `AntXxxStyle`。
4. 在 widget 中保留状态和交互，把绘制、metrics 和子控件外观迁入 Style。
5. 删除 QSS、硬编码颜色和示例层样式修饰。
6. 接入主题刷新，确保只刷新本控件或局部子树。
7. 补齐示例和测试。
8. 运行源码搜索、目标测试、必要的 CTest 和安装消费方验证。
9. 更新状态文档并记录验证结果。

## 合并前检查清单

- [ ] 生产源码中没有新增 `setStyleSheet()`。
- [ ] 核心视觉值来自 `AntTheme` / `AntPalette` / token。
- [ ] 主绘制链路优先在 `AntXxxStyle` 中实现。
- [ ] Light / Dark 主题切换后无布局抖动、无残留旧颜色。
- [ ] 内部 Qt 子控件外观已通过 Style 或 palette 纳入主题。
- [ ] 示例页无 QSS、palette、手动字体和背景修饰。
- [ ] 公共 API、信号、meta property 与 Qt 用户预期一致。
- [ ] 新增公开头和 Style 头已纳入 CMake 安装。
- [ ] 相关 QTest、render smoke、生命周期和安装消费方测试已覆盖。
- [ ] README、AGENTS 和相关 docs 已按影响范围同步。
