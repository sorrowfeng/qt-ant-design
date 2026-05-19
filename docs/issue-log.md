# 项目问题记录

整个项目的已解决问题与待解决问题清单。每条记录给出现象、根因（已解决）或推测原因（待解决）和解决方法 / 排查方向，便于回归排查时直接定位。新问题按发现时间追加；解决后从「未解决」搬到「已解决」并补完根因和最终方案。

## 已解决

### 1. AntDockWidget 浮出后，主窗口控件点击无响应（owner 链断裂）

- **现象**：`AntDockManager` 中的 `AntDockWidget` 浮出后，主窗口里的 `AntSwitch`、`AntSegmented` 等控件 mouse press / 点击不会切换到正确状态。
- **根因**：`setFloatingDockOwner()` 调用 `ownerWidget->winId()` 时传入的是 `AntDockManager`，但在示例代码里 `AntDockManager` 是 `page` 的子控件而非顶层窗口。强制 `winId()` 创建了一个 `WS_CHILD` HWND，再把它作为浮动 dock 的 `GWLP_HWNDPARENT`。Win32 要求顶层窗口的 owner 必须也是顶层窗口，指向子 HWND 会破坏整个 HWND 树的激活/焦点链路。
- **解决**：`setFloatingDockOwner()` 内部先 `ownerWidget->window()` 解析到真正的顶层 widget，再用它的 `winId()` 设置 `GWLP_HWNDPARENT` 和 `QWindow::setTransientParent`。
- **改动文件**：`src/widgets/AntDockManager.cpp`

### 2. AntWindow 圆角在 Win10 上锯齿明显

- **现象**：Win10 下 AntWindow 四个圆角呈 1bit 阶梯状。
- **根因**：Win10 没有 DWM 圆角 API，只能走 `setMask(QRegion)` fallback。`QRegion` 是 1bit 单色区域，没有 anti-alias 信息，所以即使 `AntWindowStyle::drawWindow` 用 `Antialiasing` 绘制圆角矩形，那些半透明 AA 像素也没有 alpha 通道可以承载。
- **解决**：
  1. AntWindow 启用 `Qt::WA_TranslucentBackground`，让 backing store 拥有真正的 alpha 通道
  2. 新增私有 child overlay `AntWindowCornerSmoother`，挂在 widget 最顶层、`WA_TransparentForMouseEvents`，paint 时用 `QPainter::CompositionMode_DestinationIn` 配合 AA 圆角形状把四个角"擦"成平滑 alpha
  3. 在 `setCornerRadius`、`resizeEvent`、`showEvent`、`changeEvent`、`setCentralWidget`、`setRibbon`、主题动画结束后等关键节点都重新 raise smoother
- **改动文件**：`src/widgets/AntWindow.h`、`src/widgets/AntWindow.cpp`

### 3. AntWindow 左/上边缘有一条 1px 横/竖线

- **现象**：启用 corner smoother 后，AntWindow 左边缘和上边缘出现 1px 边框线。
- **根因**：`AntWindowStyle::drawWindow` 末尾有一段 `painter->drawRoundedRect(outlineRect, ...)` 内描边，原本依靠 `setMask` 1px frame-inset 在右下挡住、左上和 mask 边重合所以肉眼看不见。启用 `WA_TranslucentBackground` 后描边裸露出来。
- **解决**：直接删除该内描边段——AntWindow 已经有软件阴影 + DWM border color + AA 圆角 smoother，多画一圈轮廓属于冗余。
- **改动文件**：`src/styles/AntWindowStyle.cpp`

### 4. AntWindow 缩放时窗口闪动

- **现象**：拖拽 AntWindow 边缘改变尺寸时整窗闪动。
- **根因**：Win10 上每次 `WM_SIZE` 都同步做了三件重活：`applyLegacyRoundedMask` 重设 1bit mask、`reapplyDwmFrameMargins` 同步 + 异步 queue 两次重应用、`updateLegacySoftwareShadow` 调 `SetWindowPos` 给独立的阴影 HWND 重定位。三者各自走不同的 HWND 合成路径，跨 HWND 不同步导致每帧错位 1-2 帧。
- **解决**：参考 `AntDockWidget` 的策略，引入 `m_legacyLiveResize` 标志：
  1. `nativeEvent` 收到 `WM_ENTERSIZEMOVE` / `WM_SIZING` 时设标志、隐藏阴影、强制一次方角绘制
  2. `resizeEvent` / `moveEvent` 在该标志下跳过 DWM frame 重应用和阴影 reposition，只更新 corner smoother
  3. `WM_EXITSIZEMOVE` 后延迟 16ms 一次性恢复 mask + DWM frame + 阴影
- **改动文件**：`src/widgets/AntWindow.h`、`src/widgets/AntWindow.cpp`

### 5. AntDockWidget 浮动后跨屏切换 DPI 时阴影 DPR 错位（初版方案）

- **现象**：把 AntWindow 主窗口拖到不同 DPI 的副屏时，软件阴影的栅格化比例和窗口轮廓对不齐。
- **根因**：`AntWindowLegacySoftwareShadow` 是一个独立 top-level HWND，它的 `QScreen::devicePixelRatio()` 来源于自己当前所在屏幕，**不会随主窗口跨屏自动同步**。Qt 给主窗口投递 `ScreenChangeInternal` / `DevicePixelRatioChange` 后主窗口正确按新 DPR 重建，但阴影 HWND 的 QScreen 还是旧的。
- **解决**（局部解决，仍存在 "拖动时跟随" 问题，见下方未解决项）：
  1. `AntWindow::event()` 拦截 `QEvent::ScreenChangeInternal` 和 `QEvent::DevicePixelRatioChange`，把阴影 `QWindow::setScreen(newScreen)` 跟到主窗口的新屏幕，再 `applyNativeWindowFrame()` + `updateLegacySoftwareShadow()` 让 backing store 用新 DPR 重建
  2. `nativeEvent` 拦截 `WM_DPICHANGED`（Win32 权威 DPI 切换通知）做兜底，0-timer 异步同步阴影 screen
  3. `updateLegacySoftwareShadow()` 内每次更新都主动检查 `shadowWindow->screen() != hostScreen`，不一致就 `setScreen()`
- **改动文件**：`src/widgets/AntWindow.cpp`

### 6. AntWindow 放大窗口时新区域显示不全

- **现象**：拖右下角向外放大，新增的区域要等松手后才补全。
- **根因**：上一轮"缩放抑制"把 `applyLegacyRoundedMask` 也跳过了。`setMask` 决定 backing store 哪些像素被合成器显示，不更新就一直按旧矩形裁剪。`AntDockWidget::applyNativeWindowFrame` 也有同条 bug（`m_legacyLiveResize && !useNativeCaption` 分支提前 return，越过了 mask 重应用）。
- **解决**：把成本拆开——
  - mask 更新便宜，必须每帧做：始终调用
  - DWM frame margin 重应用 + 异步 queue 贵且闪：延迟到 `WM_EXITSIZEMOVE`
  - 阴影 HWND `SetWindowPos` 跨 HWND 不同步：延迟到 `WM_EXITSIZEMOVE`
- **改动文件**：`src/widgets/AntWindow.cpp`、`src/widgets/AntDockWidget.cpp`

### 7. AntWindow 缩小时边缘 1-2 帧透明闪烁

- **现象**：拖边缘向内缩小，新尺寸的边缘瞬间显示为完全透明（看到桌面），下一帧恢复。
- **根因**：`WA_TranslucentBackground = true` 的窗口，DWM 合成时拿到的 backing store 是 Qt 异步重画的——`WM_SIZE` 到达时 backing store 还是旧尺寸，新尺寸里 Qt 还没画的区域 alpha=0，DWM 直接透到桌面。
- **解决**：
  1. **画方角**：`AntWindowStyle::drawWindow` 检查 widget 的 `antWindowLegacyLiveResize` 属性，true 时不做圆角 clip，整个客户区完全不透明
  2. **关 smoother**：`updateCornerSmoother` 在 live-resize 时把半径设为 0，避免 `CompositionMode_DestinationIn` 产生 alpha=0 角落
  3. **DWM 兜底**：进入 `WM_ENTERSIZEMOVE` 时主动调用 `DwmExtendFrameIntoClientArea(hwnd, {-1, -1, -1, -1})`（"sheet of glass"），让 DWM 接管整个客户区作为扩展 frame，alpha=0 区域显示 DWM 自己的 backdrop 而不是桌面
  4. `WM_EXITSIZEMOVE` 通过 `applyNativeWindowFrame()` 把 margins 恢复为 `{0, 0, 0, 0}`
- **改动文件**：`src/widgets/AntWindow.cpp`、`src/styles/AntWindowStyle.cpp`

### 8. AntDockWidget 浮动后缩放四边出现闪动线条

- **现象**：浮动 dock 缩放时整窗不闪了，但四边出现颤动的细线。
- **根因**：`AntDockWidget::paintEvent` 用 `QRectF(panel).adjusted(0.5, 0.5, -0.5, -0.5)` 画 1px 半透明描边。`WM_SIZE` 改变 `rect()` → 新描边画在新位置 1px 内侧、旧描边在更外侧的 backing store 像素没被覆盖，两条线同时存在。
- **解决**：`paintEvent` 检查 `kDockLegacyLiveResizeProperty`，true 时 `fillRect(rect(), fill)` 整窗填纯色，不画描边。`WM_EXITSIZEMOVE` 后正常路径恢复带描边的圆角面板。
- **改动文件**：`src/widgets/AntDockWidget.cpp`

### 9. AntDockWidget 浮动后主窗口控件点击只触发动画不切换

- **现象**：dock 浮动后点击主窗口 AntSwitch / AntSegmented，按下动画会动一下，但抬起后并没有完整切换或发出 `clicked` / `valueChanged` 信号。
- **根因**：不是 owner 失效（press 能到说明 owner 是对的），而是 `leaveEvent` 错误地清掉了 `m_pressed` / `m_pressedIndex`。当浮动 dock 这种新顶层 HWND 出现时，Qt 在 press 和 release 之间偶尔会给主窗口子控件投递一次 spurious leave。AntSwitch / AntSegmented 在 leave 里把 press 状态清零 → release 到达时（Qt 的 implicit mouse grab 保证 release 仍然路由回原 widget）`if (m_pressed)` 失败，跳过 `setChecked` / `setValue` / 信号发射。
- **解决**：`leaveEvent` 只清 hover 状态、不清 press 状态。press 状态只能在 `mouseReleaseEvent` 中清理，这与 Qt 的 implicit grab 语义一致。
- **改动文件**：`src/widgets/AntSwitch.cpp`、`src/widgets/AntSegmented.cpp`
- **副作用收益**：这个 leave-clears-press 的 bug 不止在 dock 浮出后出现，任何能在 press 和 release 之间触发 leave 的场景（光标 1px 偏离、tooltip 弹出、其他 widget 抢 hover）都会导致点击丢失。这次修复顺手解决了一整类潜在问题。

### 10. AntDockWidget 右键菜单未适配 AntDesign 风格

- **现象**：在 `AntDockWidget` 上右键弹出的上下文菜单使用 Qt 原生 `QMenu` 外观，未与 Ant Design 设计系统对齐（缺少主题色、圆角、阴影、字号、间距、暗色模式 token）。
- **根因**：`AntDockManager::showDockContextMenu()` 直接创建 `QMenu` 并添加 `QAction`，该路径不经过项目内 `AntMenu` 的主题、绘制和菜单项状态逻辑。
- **解决**：新增 manager-owned `AntDockContextMenuPopup` 轻量弹层，内部承载 `AntMenu`；右键命令改为 `AntMenuItem` key 分发，保留浮动、tab 移动、拆分、关闭等原有行为，同时复用 `AntMenu` 的紧凑布局、亮/暗主题、disabled / danger 状态。弹层使用无装饰、无原生窗口阴影的 tool 型窗口，并用贴合面板外沿的单层柔和羽化阴影绘制，避免原生阴影和手绘阴影叠加成多重层次。
- **改动文件**：`src/widgets/AntDockManager.cpp`、`src/widgets/AntDockWidget.cpp`、`src/widgets/AntDockWidget.h`、`tests/TestAntQtExtensions.cpp`

### 11. AntDockWidget 在 Win11 上浮动后嵌回布局导致客户区点击失效

- **现象**：`AntDockWidget` 先浮动，再拖回主布局后，主窗口除标题栏外的客户区控件无法响应点击。
- **根因**：自研停靠树把浮动 dock 放回 `QTabWidget` 后仍调用 `QDockWidget::setFloating(false)`，这会重新进入 Qt 原生 dock 的内部状态机。真实拖动场景下 Win32 还可能把 mouse capture 留在浮动 dock / titlebar / Qt 内部窗口 HWND 上，Qt 的 `QWidget::mouseGrabber()` 看不到这个 native capture，后续客户区鼠标消息会继续被旧窗口接走。
- **解决**：嵌回布局时不再调用 `QDockWidget::setFloating(false)`；新增 `prepareDockWidgetForEmbedding()`，在 dock 嵌回布局前释放 Qt mouse grab 和当前进程 Win32 mouse capture、隐藏并递归销毁旧 floating HWND 及其子 native HWND、清理 native owner / 透明输入 flags / window opacity，并通过 `setParent(parent, Qt::Widget)` 强制恢复普通子控件窗口类型。Windows 下额外归一化重新创建后的真实 HWND：强制 `WS_CHILD`，清除 `WS_POPUP`、`WS_EX_TOPMOST`、`WS_EX_TOOLWINDOW`、`WS_EX_TRANSPARENT`、`WS_EX_NOACTIVATE` 等顶层 / 透明输入样式，并显式 `SetParent()` 到嵌入区域的真实 HWND。Dock 页重新加入 `QTabWidget` 时先由 `addTab()` 接管布局，再显示 dock，避免旧浮窗几何短暂作为 native child 暴露。嵌回后再次隐藏 drop guide / drag preview / drop preview，并在 Win32 层强制 `ShowWindow(SW_HIDE)` 收起透明 tool window；Dock / AntWindow 软件阴影窗口也设置为 native click-through 并在隐藏时强制收起，避免透明或阴影 HWND 残留在 example 主窗口上方吃掉鼠标。停靠状态由 `AntDockManager` 自己的 area 映射决定。回归测试覆盖 embedded 后的 Qt window flags、Win32 style、真实 `WindowFromPoint()` 命中、透明输入属性、overlay 隐藏、Qt / Win32 mouse grab 清理和客户区 hit-test；Windows 可用真实输入时，还会用 `SendInput` 拖动浮窗回布局并点击嵌回后的 `AntSwitch`。
- **改动文件**：`src/widgets/AntDockManager.cpp`、`src/widgets/AntDockWidget.cpp`、`src/widgets/AntWindow.cpp`、`tests/TestAntQtExtensions.cpp`

### 12. AntDockWidget 嵌回 AntWindow 后页面控件仍可能被覆盖层拦截

- **现象**：`AntDockWidget` 从浮动窗口嵌回 `AntWindow` 宿主页面后，`QApplication::widgetAt()` 能命中 `AntSwitch`，但 Windows 真实输入偶尔命中 `AntWindowCornerSmoother` 或点击 wave 动效层，导致页面控件看起来仍然无响应。
- **根因**：这些覆盖层只设置了 Qt 层面的 `WA_TransparentForMouseEvents`。当它们因为宿主窗口、动画或测试命中路径拥有 native child HWND 时，`WindowFromPoint()` 仍可能优先返回覆盖层 HWND，真实鼠标消息无法继续落到底层控件。
- **解决**：把 `AntWindowCornerSmoother` 和通用 `AntWave` 都补齐 Win32 原生命中穿透：设置 `WS_EX_TRANSPARENT` / `WS_EX_NOACTIVATE`，并在 `WM_NCHITTEST` 返回 `HTTRANSPARENT`。`TestAntQtExtensions::dockManager()` 新增 `AntWindow` 宿主场景，覆盖浮动、嵌回、`WindowFromPoint()` 不再命中覆盖层，以及 `SendInput` 再次点击 `AntSwitch` 必须切换。
- **改动文件**：`src/widgets/AntWindow.cpp`、`src/core/AntWave.h`、`src/core/AntWave.cpp`、`tests/TestAntQtExtensions.cpp`

### 13. AntDockWidget 浮动窗口在主布局管理器不可见时无法拖动

- **现象**：`AntDockWidget` 已经浮动后，如果 `AntDockManager` 所在的主布局管理器被隐藏、最小化或进入不可见链路，拖动浮动窗口标题栏不会移动窗口。
- **根因**：浮动 dock 的标题栏拖动复用 `showDropGuideAt()`；该函数开头用 manager 可见性作为总开关。manager 不可见时会直接返回，导致已经浮动的 dock 连自身 `move()` 都不会执行，后续 release 也只是停止 tracking。
- **解决**：拆分浮动窗口移动和停靠引导显示：只要正在拖动的 dock 已经是浮动窗口，就先按鼠标全局位置更新浮窗位置并保持半透明拖动反馈；只有 manager 停靠面板可见时才继续显示小方格和嵌入预览。对于隐藏状态下的嵌入 dock，不激活预览或浮动逻辑，避免程序化事件误触发。回归测试覆盖 manager 隐藏后浮动 dock 仍能通过标题栏移动，且不会显示 drop preview 或 active guide。
- **改动文件**：`src/widgets/AntDockManager.cpp`、`tests/TestAntQtExtensions.cpp`

### 14. AntDockManager 程序化移除浮动 Dock 后留下孤立浮窗

- **现象**：外部直接调用 `AntDockManager::removeDockWidget()` 移除一个已经浮动的 `AntDockWidget` 时，manager 会移除记录和 owner，但浮动窗口本身仍可能保持可见，形成一个不再受 manager 管理的孤立窗口。
- **根因**：浮动 dock 不在 `DockArea` 里，`removeDockFromArea()` 对它是 no-op；旧逻辑只清理集合、owner 和 event filter，没有统一停止拖动状态、恢复 opacity 或隐藏浮动窗口。
- **解决**：`removeDockWidget()` 在目标正处于拖动状态时先停止 tracking；记录移除前的 floating 状态，清理 owner / area / event filter 后，对浮动目标恢复 `windowOpacity(1.0)` 并 `hide()`，让 `AntDockWidget::hideEvent()` 同步收起软件阴影。回归测试覆盖程序化移除浮动 dock 后 manager 列表、floating 列表、owner 属性和窗口可见性都正确收敛。
- **改动文件**：`src/widgets/AntDockManager.cpp`、`tests/TestAntQtExtensions.cpp`

### 15. AntDockWidget 右键菜单 Close other tabs 忽略其他 tab 的 closable 状态

- **现象**：在一个 tab area 里右键 `Close other tabs` 时，菜单项只根据当前 tab 是否 closable 决定启用状态；如果其他 tab 被设为不可关闭，仍可能被批量关闭。反过来，如果当前 tab 不可关闭但其他 tab 可关闭，该菜单项会被错误禁用。
- **根因**：`showDockContextMenu()` 构建菜单时复用了当前 dock 的 `closable` 布尔值，执行时也遍历全部 other docks，没有逐个检查 `DockWidgetClosable` feature。
- **解决**：构建右键菜单时提前筛出 `closableOtherDocks`，`Close other tabs` 只有在存在可关闭的其他 tab 时才启用；执行该动作时也只移除这个列表。回归测试覆盖“其他 tab 不可关闭”和“当前 tab 不可关闭但其他 tab 可关闭”两个方向。
- **改动文件**：`src/widgets/AntDockManager.cpp`、`tests/TestAntQtExtensions.cpp`

### 16. AntDockWidget 禁用 floatable 后浮动窗口无法通过菜单回到布局

- **现象**：`AntDockWidget` 已经处于浮动状态时，如果外部调用 `setDockWidgetFloatable(dock, false)`，右键菜单里的 `Dock to workspace` 也会被禁用，用户无法从菜单把浮动窗口放回布局。
- **根因**：右键菜单的第一项同时表示 `Float` 和 `Dock to workspace`，旧逻辑无论当前是否已经浮动，都直接用 `!floatable` 决定 disabled。`DockWidgetFloatable` 应该限制“从布局浮出去”，不应该阻止已经浮动的窗口回到布局。
- **解决**：菜单第一项的禁用条件改为 `!floating && !floatable`：未浮动时继续遵守 floatable feature，已浮动时始终允许 `Dock to workspace`。回归测试覆盖浮动窗口禁用 floatable 后右键菜单仍可显示可用的 `Dock to workspace`。
- **改动文件**：`src/widgets/AntDockManager.cpp`、`tests/TestAntQtExtensions.cpp`

### 17. AntDockManager 直接浮动未添加 Dock 时不会注册到 manager

- **现象**：外部直接调用 `AntDockManager::setDockWidgetFloating(dock, true)`，如果该 dock 还没有通过 `addDockWidget()` 添加过，会显示一个浮动窗口，但 `dockWidgets()` / `floatingDockWidgets()` 不包含它，保存布局、事件过滤、后续移除等管理能力都不完整。
- **根因**：`setDockWidgetFloating(true)` 直接进入 `floatDockWidget()`，而旧的 `floatDockWidget()` 假设 dock 已经在 `m_docks` 中，只负责从旧 area detach 并显示浮窗，没有调用 `prepareDockWidget()` 注册、安装连接和补齐 object name。
- **解决**：`floatDockWidget()` 现在先调用 `prepareDockWidget()`，对首次浮动的 dock 完成 manager 注册，并在成功浮动后发出 `dockWidgetAdded`，再发出 `dockWidgetFloated`。回归测试覆盖 detached dock 直接浮动后进入 `dockWidgets()` / `floatingDockWidgets()`，并可通过 `removeDockWidget()` 正常收起。
- **改动文件**：`src/widgets/AntDockManager.cpp`、`tests/TestAntQtExtensions.cpp`

### 18. AntDockManager 首次接管 Dock 时覆盖预配置 features

- **现象**：调用方在 `addDockWidget()` 或首次 `setDockWidgetFloating()` 之前预先关闭 `DockWidgetFloatable` / `DockWidgetMovable` / `DockWidgetClosable`，manager 首次接管时仍会把这些 feature 全部重新打开；对未注册且不可浮动的 dock 调用浮动 API 时，还会出现半注册状态。
- **根因**：`prepareDockWidget()` 在 `added` 分支里无条件 OR 三个默认 features；上一轮把首次浮动注册放进 `floatDockWidget()` 后，注册发生在 floatable 检查之前，导致不可浮动的 detached dock 也可能进入 manager 集合。
- **解决**：首次接管 dock 时不再覆盖调用方已设置的 features；`floatDockWidget()` 先检查 `DockWidgetFloatable`，通过后才注册到 manager。回归测试覆盖预配置不可浮动/不可移动的 dock 加入后保持原 feature，以及未注册且不可浮动的 dock 调用浮动 API 不会注册或显示。
- **改动文件**：`src/widgets/AntDockManager.cpp`、`tests/TestAntQtExtensions.cpp`

### 19. 浮动 Dock 直接 close 后仍残留在 manager 状态中

- **现象**：浮动 `AntDockWidget` 通过窗口关闭按钮或外部 `close()` 关闭后，窗口虽然隐藏，但仍保留在 `dockWidgets()` / `floatingDockWidgets()` 和布局快照状态中；关闭被禁用的 Dock 也可能被程序化 close 隐藏。
- **根因**：manager 的事件过滤器收到 `QEvent::Close` 时只停止拖动，没有走统一的 `removeDockWidget()` 清理路径，也没有按 `DockWidgetClosable` feature 拦截关闭。
- **解决**：`AntDockManager` 现在在 Close 事件中统一处理受管 Dock：closable 关闭时移除并隐藏，非 closable 关闭时忽略事件。回归测试覆盖非 closable 浮动窗口 close 被拦截，以及 closable 浮动窗口 close 后从 manager 和 floating 列表中移除。
- **改动文件**：`src/widgets/AntDockManager.cpp`、`tests/TestAntQtExtensions.cpp`

### 20. Manager 析构后浮动 Dock 可能残留为孤立窗口

- **现象**：`AntDockManager` 销毁时，如果还有由它管理的浮动 `AntDockWidget`，该 Dock 已经是独立顶层窗口，可能在 manager 销毁后仍然可见并保留旧 owner 状态。
- **根因**：析构函数只移除事件过滤器和信号连接，没有像显式 `removeDockWidget()` 一样收敛浮动窗口的可见性、透明度和 owner 属性。
- **解决**：析构时识别受管浮动 Dock，先清理 floating owner、恢复 `windowOpacity(1.0)` 并隐藏窗口，再移除事件过滤器和连接。回归测试覆盖删除 manager 后浮动 Dock 被隐藏且 owner 属性清空。
- **改动文件**：`src/widgets/AntDockManager.cpp`、`tests/TestAntQtExtensions.cpp`

### 21. Dock 移除后复用会叠加旧 manager 连接

- **现象**：同一个 `AntDockWidget` 实例被 `removeDockWidget()` 移除后再添加回来，旧的 dock 到 manager 的 `visibilityChanged` / `topLevelChanged` 连接仍然存在，后续浮动时 owner 设置和 placeholder 刷新会重复触发。
- **根因**：`removeDockWidget()` 只移除事件过滤器，没有断开 `prepareDockWidget()` 首次接管时建立的 dock→manager 信号连接。
- **解决**：移除 Dock 时同步断开 dock 指向当前 manager 的连接；新增内部 `antDockFloatingOwnerApplyCount` 计数用于回归验证，确保移除后复用并浮动时 owner 设置次数不会因旧连接叠加而增长。
- **改动文件**：`src/widgets/AntDockManager.cpp`、`tests/TestAntQtExtensions.cpp`

### 22. DockArea 移除 Dock 后复用会叠加 tab 同步连接

- **现象**：Dock 被从某个 tab area 移除后，如果同一个实例再添加回该 area，标题或图标变化会触发多条旧的 tab 文本/图标同步连接。
- **根因**：`DockArea::addDock()` 为每个 Dock 建立 `windowTitleChanged` / `windowIconChanged` 到当前 area 的连接，但 `DockArea::removeDock()` 只移除 tab，没有断开 dock 到该 area 的连接。
- **解决**：`DockArea::removeDock()` 现在同步断开 dock 指向该 area 的连接；新增内部 `antDockAreaTitleSyncCount` 回归计数，验证移除后复用同一 Dock 时标题同步只触发一次。
- **改动文件**：`src/widgets/AntDockManager.cpp`、`tests/TestAntQtExtensions.cpp`

### 23. 非 closable 浮动 Dock 标题栏仍显示关闭按钮

- **现象**：关闭 `DockWidgetClosable` 后，浮动窗口标题栏仍显示关闭按钮，点击后没有效果，容易被误认为窗口无响应。
- **根因**：自绘标题栏只在按钮触发时检查 closable feature，没有把按钮可见性和 `featuresChanged` 同步到标题栏 chrome。
- **解决**：标题栏 close 按钮现在跟随 `DockWidgetClosable` 隐藏/显示，并监听 `featuresChanged` 动态刷新；回归测试覆盖浮动 Dock 的 close 按钮在 manager API 切换 closable 时即时同步。
- **改动文件**：`src/widgets/AntDockWidget.cpp`、`tests/TestAntQtExtensions.cpp`

### 24. 标题栏双击浮动绕过 DockWidgetFloatable

- **现象**：`AntDockWidget` 作为普通 `QDockWidget` 使用时，即使关闭 `DockWidgetFloatable`，双击自绘标题栏仍可能直接调用 `setFloating(true)`。
- **根因**：标题栏双击处理只区分当前是否浮动，没有在非浮动分支检查 `DockWidgetFloatable` feature。
- **解决**：非浮动标题栏双击现在先检查 `DockWidgetFloatable`，禁用时保持嵌入；回归测试覆盖禁用/启用 floatable 后标题栏双击的不同结果。
- **改动文件**：`src/widgets/AntDockWidget.cpp`、`tests/TestAntQtExtensions.cpp`

### 25. 拖动中关闭 movable 后仍继续显示落位预览

- **现象**：Dock tab / 标题栏拖动已经开始后，如果外部调用 `setDockWidgetMovable(dock, false)`，当前拖动状态仍会继续响应后续鼠标移动，可能继续显示 drop preview / guide。
- **根因**：`setDockWidgetFeatureEnabled()` 只更新 `QDockWidget::features()`，没有在动态关闭 `DockWidgetMovable` 时收敛已有的 `m_draggingDockTitle` / `m_draggedDock` 状态。
- **解决**：关闭当前拖动 Dock 的 `DockWidgetMovable` 时立即调用 `stopDockDragTracking()`，同步清理 drag preview、drop guide、记忆 drop target 和全局事件过滤器；回归测试覆盖拖动开始后关闭 movable，后续移动不会激活 drop preview。
- **改动文件**：`src/widgets/AntDockManager.cpp`、`tests/TestAntQtExtensions.cpp`

### 26. 外部直接 setFeatures 关闭 movable 不会取消当前拖动

- **现象**：如果拖动已经开始，调用方绕过 manager API，直接用 `dock->setFeatures()` 清除 `DockWidgetMovable`，当前拖动仍可能继续响应后续鼠标移动。
- **根因**：manager 只在 `setDockWidgetFeatureEnabled()` 中处理拖动取消，没有监听受管 Dock 的 `featuresChanged`。
- **解决**：`prepareDockWidget()` 现在连接 `featuresChanged`，当当前拖动 Dock 的 `DockWidgetMovable` 被外部直接关闭时，同样调用 `stopDockDragTracking()` 清理预览和引导状态。回归测试覆盖 manager API 和直接 `setFeatures()` 两条路径。
- **改动文件**：`src/widgets/AntDockManager.cpp`、`tests/TestAntQtExtensions.cpp`

### 27. 外部直接 setFeatures 不触发 manager feature 变更信号

- **现象**：受管 Dock 被外部直接调用 `dock->setFeatures()` 修改 feature 后，`AntDockManager::dockWidgetFeatureChanged` 不会发出，外部监听 manager 信号时感知不到状态变化。
- **根因**：manager 只在 `setDockWidgetFeatureEnabled()` 中手动发信号，未把受管 Dock 自身的 `featuresChanged` 作为统一变更入口转发。
- **解决**：`prepareDockWidget()` 监听 `featuresChanged` 后统一发出 `dockWidgetFeatureChanged`，`setDockWidgetFeatureEnabled()` 只负责更新 features，避免 manager API 路径重复发信号；回归测试覆盖直接 `setFeatures()` 清除/恢复 movable 的信号计数。
- **改动文件**：`src/widgets/AntDockManager.cpp`、`tests/TestAntQtExtensions.cpp`

### 28. AntWindow 外侧边缘无法触发缩放

- **现象**：AntWindow 显示为无边框窗口后，从窗口外侧贴近边缘拖动时没有进入系统缩放，表现为窗口无法调整大小。
- **根因**：`WM_NCHITTEST` 的 resize 判断只接受客户区内部坐标，Windows 在系统 resize frame 上发来的坐标可能落在 `-border` 到 `0` 或 `width/height` 外侧，旧逻辑会把这些位置返回为 `HTCLIENT`。
- **解决**：resize hit-test 扩展到窗口外侧的系统边框范围，同时继续覆盖内部边缘；回归测试补齐外侧四边和四角的命中结果。
- **改动文件**：`src/widgets/AntWindow.cpp`、`tests/TestAntQtExtensions.cpp`

### 29. DockWidget 拖动时 drop guides 不显示

- **现象**：拖动 DockWidget 时布局预览还能出现，但中心/边缘的小方格引导层不可见。
- **根因**：drop preview 是独立透明工具窗，层级高于 manager 内部 child overlay；在部分 Windows 组合路径下，child guide overlay 会被预览工具窗盖住或变得不可见。
- **解决**：drop guide overlay 改为独立透明 top-level 工具窗，并在每次更新 drop preview 后重新 `raise()`，确保小方格在预览层之上；回归测试直接验证 overlay 可见、置顶、原生命中穿透，并通过渲染像素确认 active guide square 被实际绘制。
- **改动文件**：`src/widgets/AntDockManager.cpp`、`tests/TestAntQtExtensions.cpp`

### 30. DockWidget 拖动落位后新布局慢一拍刷新

- **现象**：拖动 DockWidget 改变布局时，松开鼠标后预览先消失，新 dock tree 下一轮事件循环才刷新，视觉上有明显滞后。
- **根因**：`finishDockDragTracking()` 对已命中 drop target 的情况使用 `QTimer::singleShot(0)` 延后调用 `applyDropTarget()`；同时新 root / splitter tree 没有在插入后主动激活布局。
- **解决**：有明确落位的 drop 现在改用 queued meta-call，避免 timer tick 带来的慢一拍，同时避开在鼠标事件分发中直接销毁 / 重排 tab area 的生命周期风险；`insertDockWidget()` 插入 dock 后立即激活 workspace layout 并触发重绘。回归测试覆盖右侧容器落位和中心 tab 落位通过 `QEvent::MetaCall` 即可完成 dock tree 更新，不再依赖 timer 轮询。
- **改动文件**：`src/widgets/AntDockManager.h`、`src/widgets/AntDockManager.cpp`、`tests/TestAntQtExtensions.cpp`

### 31. AntWindow 真实拖拽边缘仍无法缩放

- **现象**：`WM_NCHITTEST` 单元路径能返回 `HTRIGHT` / `HTBOTTOM` 等 resize hit-test，但实际用鼠标拖 AntWindow 边缘时窗口尺寸不变。
- **根因**：`AntWindow` 的内部 content widget 拥有子 HWND 并覆盖到窗口边缘，真实鼠标命中落在子 HWND 上，顶层 `AntWindow::nativeEvent()` 收不到 `WM_NCHITTEST` / `WM_NCLBUTTONDOWN`，因此系统 resize loop 没有启动。
- **解决**：Windows 下 content widget 改为私有 `AntWindowContentWidget`，在标题栏和 resize 边缘的 `WM_NCHITTEST` 返回 `HTTRANSPARENT`，让命中继续传给顶层 AntWindow；顶层窗口对 resize / caption 的 `WM_NCLBUTTONDOWN` 显式交给 `DefWindowProcW`。回归测试新增 worker 线程真实 `SendInput` 拖右边缘，验证窗口宽度实际增长。
- **改动文件**：`src/widgets/AntWindow.cpp`、`tests/TestAntQtExtensions.cpp`

### 32. DockWidget 回嵌后 AntWindow 左右/底边无法缩放

- **现象**：`AntDockWidget` 从浮动窗口拖回 `AntWindow` 宿主布局后，顶边仍可缩放，但左边、右边和底边拖拽不再触发窗口 resize。
- **根因**：回嵌后的 Dock 页面里存在更深层的 native child HWND，真实鼠标命中可能落在这些子窗口上，而不是只落在 `AntWindowContentWidget` 上。旧修复只让 content widget 在 resize band 返回 `HTTRANSPARENT`，没有覆盖任意后代 HWND，所以顶层 `AntWindow` 仍可能收不到左右/底边的 `WM_NCHITTEST`。
- **解决**：`AntWindow` 在 Windows 下安装 `QAbstractNativeEventFilter`，当任意属于当前窗口的后代 HWND 在标题栏或 resize band 收到 `WM_NCHITTEST` 时统一返回 `HTTRANSPARENT`，让命中继续回到顶层 `AntWindow` 的系统 resize/caption 路径。回归测试在 DockWidget float 后拖回布局，再通过 worker 线程真实 `SendInput` 分别拖右边、左边和底边，验证尺寸实际增长并覆盖子 HWND 命中转发计数。
- **改动文件**：`src/widgets/AntWindow.h`、`src/widgets/AntWindow.cpp`、`tests/TestAntQtExtensions.cpp`

### 33. DockWidget 切换到新布局响应偏慢

- **现象**：DockWidget 拖拽落位或恢复已保存布局时，新布局出现不够跟手，视觉上像是旧布局与新布局之间慢了一拍。
- **根因**：
  1. 拖拽落位为了避开 mouse release 事件分发中的树重排风险，使用 queued meta-call 应用新布局；但旧逻辑先隐藏 drop preview，再等 queued call 才重建布局，中间会露出一帧旧布局空档。
  2. `restorePerspective()` 会先从旧 `DockArea` 移除全部 dock，并把 dock `setParent(nullptr)`。即使 dock 在新 perspective 中仍然是嵌入状态，也会短暂变成 top-level，Windows 下随后进入 `resetNativeFloatingWindowForEmbedding()` / `destroy(true, true)` / native style 归一化路径，造成不必要的 HWND 销毁重建。
  3. restore 末尾无条件 `setStyleSheet()`，每次恢复布局都会让 Qt stylesheet 重新解析并 repolish 整个 DockManager 子树。
- **解决**：有引导目标的拖拽落位会保留 drop preview 直到 queued 布局切换真正应用，避免空白间隙；`restorePerspective()` 进入批量恢复模式，临时关闭 manager/workspace/dock 更新、延后 placeholder 刷新，并把仍会嵌入的 dock 暂挂到 manager child 上，不再临时顶层化；恢复 area 时批量添加 tab，仅最后设置当前 tab；`updateTheme()` 只有样式内容变化时才重新 `setStyleSheet()`。回归测试覆盖 restore 期间保留嵌入 dock、area 重建计数、stylesheet 不重复应用，以及 queued 布局切换前 drop preview 仍保持可见。
- **改动文件**：`src/widgets/AntDockManager.h`、`src/widgets/AntDockManager.cpp`、`tests/TestAntQtExtensions.cpp`

## 未解决

### A. AntWindow 跨屏拖动时阴影错位（动态跟随问题）

- **现象**：把 AntWindow 拖到其他屏幕时，阴影位置/尺寸的更新和窗口轮廓对不齐——阴影应该实时跟着主窗口画，而不是在跨屏后才补正。
- **与已解决问题 #5 的区别**：#5 只处理了 `WM_DPICHANGED` / `ScreenChangeInternal` 这类**离散事件**，阴影 HWND 在 DPI 切换的那一帧才同步。但 AntWindow `moveEvent` 在 live-drag 期间也被抑制（`if (m_legacyLiveResize) return`），所以拖拽中的连续 move 事件阴影完全不跟随。
- **潜在排查方向**：
  1. 拖动 AntWindow（不是 resize）时不应该走 `m_legacyLiveResize` 抑制——拖动期间 `WM_ENTERSIZEMOVE` 是 move 还是 resize 触发？需要区分这两种 size-move 模式
  2. 或者让 `moveEvent` 始终更新阴影 HWND 位置，只在 resize 时跳过（拖动主要是 reposition，阴影 `SetWindowPos` 在同 DPI 内是便宜的）
  3. 检查 `WM_DPICHANGED` 的处理是否在跨屏首帧就跟上了阴影 HWND 的 `setScreen()`

### B. AntWindow 缩小尺寸时偶尔仍有闪动

- **现象**：已解决问题 #7 引入的 DWM `{-1,-1,-1,-1}` 兜底后大幅改善，但缩小时偶尔（约几帧出现一次）还能看到极短的闪动。
- **推测原因**：
  1. `DwmExtendFrameIntoClientArea` 设置生效本身也需要一帧——`WM_ENTERSIZEMOVE` 到第一个 `WM_SIZE` 之间的窗口可能 DWM backdrop 还没接管
  2. Qt 在 `WA_TranslucentBackground=true` 下的 backing store flush 时机和 DWM 合成时钟之间仍有概率性错位
- **潜在排查方向**：
  1. 在 `WM_NCCREATE` / `showEvent` 就预设 `{-1,-1,-1,-1}`，常态生效，只在显示时通过 paint 路径让背景不透明
  2. 尝试 `WM_WINDOWPOSCHANGING` 拦截，比 `WM_SIZE` 更早，可以在新尺寸到达 DWM 之前先确保 backdrop
  3. 评估完全放弃 `WA_TranslucentBackground`，圆角在 Win10 上接受方角降级（视觉妥协换稳定不闪）

## 关联测试

- `tests/TestAntQtExtensions.cpp`：覆盖 AntWindow / AntDockWidget 大部分行为
- `tests/TestAntSwitch.cpp`、`tests/TestAntDataEntryA.cpp`：覆盖 AntSwitch / AntSegmented
- 改动这些控件时按 `AGENTS.md` "测试范围规则" 只跑对应 CTest target，不要默认全量 ctest
