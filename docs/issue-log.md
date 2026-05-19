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
  1. `nativeEvent` 收到 resize hit-test 的 `WM_NCLBUTTONDOWN` 或 `WM_SIZING` 时设标志、隐藏阴影、强制一次方角绘制
  2. `resizeEvent` / `moveEvent` 在该标志下跳过 DWM frame 重应用和阴影 reposition，只更新 corner smoother
  3. `WM_EXITSIZEMOVE` 后延迟 16ms 一次性恢复 mask + DWM frame + 阴影
- **改动文件**：`src/widgets/AntWindow.h`、`src/widgets/AntWindow.cpp`

### 5. AntDockWidget 浮动后跨屏切换 DPI 时阴影 DPR 错位（初版方案）

- **现象**：把 AntWindow 主窗口拖到不同 DPI 的副屏时，软件阴影的栅格化比例和窗口轮廓对不齐。
- **根因**：`AntWindowLegacySoftwareShadow` 是一个独立 top-level HWND，它的 `QScreen::devicePixelRatio()` 来源于自己当前所在屏幕，**不会随主窗口跨屏自动同步**。Qt 给主窗口投递 `ScreenChangeInternal` / `DevicePixelRatioChange` 后主窗口正确按新 DPR 重建，但阴影 HWND 的 QScreen 还是旧的。
- **解决**：
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
  3. **DWM 兜底**：进入边缘缩放（resize hit-test 或 `WM_SIZING`）时主动调用 `DwmExtendFrameIntoClientArea(hwnd, {-1, -1, -1, -1})`（"sheet of glass"），让 DWM 接管整个客户区作为扩展 frame，alpha=0 区域显示 DWM 自己的 backdrop 而不是桌面
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
- **解决**：嵌回布局时不再调用 `QDockWidget::setFloating(false)`；新增 `prepareDockWidgetForEmbedding()`，在 dock 嵌回布局前释放 Qt mouse grab 和当前进程 Win32 mouse capture、隐藏并递归销毁旧 floating HWND 及其子 native HWND、清理 native owner / 透明输入 flags / window opacity，并通过 `setParent(parent, Qt::Widget)` 强制恢复普通子控件窗口类型。Windows 下不再把回嵌 dock 归一化成 embedded child HWND，也不再强制给 dock area 创建 native parent HWND；如果回嵌后仍检测到残留 native handle，会立即隐藏并销毁，保持 dock 走 Qt backing-store 子控件绘制路径。Dock 页重新加入 `QTabWidget` 时先由 `addTab()` 接管布局，再显示 dock，避免旧浮窗几何短暂作为 native child 暴露。嵌回后再次隐藏 drop guide / drag preview / drop preview，并在 Win32 层强制 `ShowWindow(SW_HIDE)` 收起透明 tool window；Dock / AntWindow 软件阴影窗口也设置为 native click-through 并在隐藏时强制收起，避免透明或阴影 HWND 残留在 example 主窗口上方吃掉鼠标。停靠状态由 `AntDockManager` 自己的 area 映射决定。回归测试覆盖 embedded 后的 Qt window flags、非 native backing-store 状态、真实 `WindowFromPoint()` 命中、透明输入属性、overlay 隐藏、Qt / Win32 mouse grab 清理和客户区 hit-test；Windows 可用真实输入时，还会用 `SendInput` 拖动浮窗回布局并点击嵌回后的 `AntSwitch`。
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
- **解决**：嵌入态 dock 的 guided drop 现在在 release 事件返回前同步应用新布局，不再等 queued meta-call；浮动窗口回嵌这类生命周期更复杂的路径继续使用 queued apply，并保留 drop preview 直到布局切换真正应用，避免空白间隙。`restorePerspective()` 进入批量恢复模式，临时关闭 manager/workspace/dock 更新、延后 placeholder 刷新，并把仍会嵌入的 dock 暂挂到 manager child 上，不再临时顶层化；恢复 area 时批量添加 tab，仅最后设置当前 tab；`updateTheme()` 只有样式内容变化时才重新 `setStyleSheet()`。回归测试覆盖 restore 期间保留嵌入 dock、area 重建计数、stylesheet 不重复应用，以及嵌入态 guided drop 在 release 后立即完成布局切换。
- **改动文件**：`src/widgets/AntDockManager.h`、`src/widgets/AntDockManager.cpp`、`tests/TestAntQtExtensions.cpp`

### 34. DockWidget 不可见时浮动窗口仍能嵌回布局

- **现象**：`AntDockManager` 所在页面或 dock 工作区不可见时，已经浮动的 `AntDockWidget` 仍可能在拖动释放后嵌回隐藏布局。
- **根因**：`showDropGuideAt()` 在不可见 surface 下会隐藏预览，但 `dropTargetAt()` 和 release fallback 仍按 manager 的历史 geometry 计算 drop target；隐藏页面虽然屏幕上不可见，`mapFromGlobal()` / `rect()` 仍可能让松手点落在旧布局范围内，最终生成有效落位。
- **解决**：新增 `isDockingSurfaceAvailable()`，统一检查 manager、top-level window 和 workspace 是否实际可见且未最小化；`dropTargetAt()`、remembered target 和 release fallback 在 surface 不可用时都返回无效，隐藏 surface 下只允许浮动窗口继续移动，不允许 dock 回布局。回归测试覆盖隐藏 manager 后拖动浮窗：不会发出 docked 信号，不会有 queued 回嵌，浮窗仍保持 floating。
- **改动文件**：`src/widgets/AntDockManager.h`、`src/widgets/AntDockManager.cpp`、`tests/TestAntQtExtensions.cpp`

### 35. Win10 示例窗口内控件不自动刷新绘制

- **现象**：Windows 10 legacy frame 路径下，示例主窗口中的控件状态变化后不会稳定自动刷新，表现为任意控件 hover/click/update 都像被整窗卡住，需要其他窗口事件后才看到新绘制。
- **根因**：`AntWindowCornerSmoother` 是覆盖整窗的 child overlay，用于在 Win10 无 DWM 圆角时做 alpha 圆角柔化；但 `updateCornerSmoother()` 为了设置 Win32 click-through 调用了 `winId()`，把这个整窗 overlay 强制提升成 native child HWND。Win10 对透明 native child 的合成与 Qt backing-store 子控件刷新不同步，导致下面的控件 repaint 不能稳定呈现。
- **解决**：`AntWindowCornerSmoother` 保持非 native child，不再主动创建整窗 HWND；只有外部已经创建了 native handle 时才设置 Win32 透明扩展样式。无 native handle 时仅通过 Qt `WA_TransparentForMouseEvents` 保持输入穿透。回归测试强制 Win10 legacy frame policy，验证 smoother 没有 native HWND，并验证中心子控件 `update()` 后窗口截图能看到新绘制像素。
- **改动文件**：`src/widgets/AntWindow.cpp`、`tests/TestAntQtExtensions.cpp`

### 36. Win10 DockWidget 浮窗回嵌后示例窗口动画刷新停止

- **现象**：Windows 10 下把 `AntDockWidget` 浮窗拖回主布局后，示例窗口客户区动画刷新停止或明显卡住，控件状态变化需要额外窗口事件才会显示。
- **根因**：浮窗回嵌时旧逻辑会把 dock 或 dock area 继续提升成 native child HWND；同时拖动过程中的 drop guide / drop preview 是透明 top-level layered tool window。Win10 legacy frame 的透明窗口、layered 工具窗和 Qt backing-store 子控件混合时，native 窗口栈会破坏主窗口客户区的合成刷新节奏。
- **解决**：`normalizeEmbeddedDockNativeWindow()` 改为纯 backing-store 收敛路径：回嵌 dock 设置回普通 Qt child widget，不再调用 `parentWidget->winId()`，也不再保留或重设 embedded child HWND；如果仍发现残留 native handle，会隐藏并销毁。drop guide 和 drop preview 改成 `AntDockManager` 内部普通 child overlay，只通过 `raise()` 保持层级，不再创建 Win10 透明 top-level HWND；拖动结束时临时透明窗口也会隐藏并销毁 native handle。回归测试强制 AntWindow legacy frame policy，验证回嵌后的 dock、guide 和 preview 都没有残留 native handle，并在同一窗口内用可绘制探针控件触发 `update()`，从窗口截图采样确认新颜色已经真实刷新。
- **改动文件**：`src/widgets/AntDockManager.cpp`、`tests/TestAntQtExtensions.cpp`

### 37. AntDockWidget 暗色模式层级和背景异常

- **现象**：暗色模式下 `AntDockWidget` 页面容易出现浅色条带、边缘浅线和 tab/pane 层级不清晰，内容区域也可能露出系统浅色背景。
- **根因**：嵌入态 dock 仍调用 `QDockWidget::paintEvent()`，Qt 原生 dock frame 会在隐藏标题栏后继续绘制一段系统浅色标题/框架区域；`DockArea` 只依赖全局样式表，缺少自己的暗色 palette 和边缘补绘；普通 QWidget dock 内容没有强制使用 token 背景填充。
- **解决**：`AntDockWidget` 在 manager 嵌入态改为自绘 token 容器背景，不再走原生 dock paint；dock 内容 QWidget 同步 `Window/Base/Button/Text/Placeholder` palette 并启用 `autoFillBackground`。`AntDockManager::DockArea` 增加暗色 palette、外框补绘、tab/pane/splitter token 化样式和 manager 自身背景填充，selected tab 与 pane 连成一体，inactive tab 使用 elevated/fill 层级。回归测试切换 dark 后检查 dock/content/area palette，并渲染 manager 截图确认 container/elevated 暗色 surface 可见且主体区域没有 stale light layout pixels。
- **改动文件**：`src/widgets/AntDockManager.h`、`src/widgets/AntDockManager.cpp`、`src/widgets/AntDockWidget.cpp`、`tests/TestAntQtExtensions.cpp`

### 38. AntWindow Win10 缩放比例下软件阴影位置/尺寸异常

- **现象**：Windows 10 legacy frame 路径下，系统缩放为 125% / 150% 等非 100% 时，`AntWindow` 外侧软件阴影可能出现偏移、尺寸不贴合或拖动跨屏后与窗口轮廓不同步。
- **根因**：`updateLegacySoftwareShadow()` 先用 Qt `setGeometry()` 设置阴影窗口几何，又把同一个 `QRect` 传给 Win32 `SetWindowPos()`。Qt 的 `QWidget::geometry()` 是逻辑像素，而 `SetWindowPos()` 在 per-monitor DPI aware 进程中使用物理像素，导致非 100% 缩放下阴影 HWND 被二次用错误单位定位/缩放。另一个问题是 `WM_ENTERSIZEMOVE` 同时覆盖移动和缩放，旧逻辑会把普通移动也标记为 live resize，拖动期间直接隐藏阴影并抑制 `moveEvent()` 跟随。
- **解决**：
  1. 阴影窗口的 move/size 只通过 Qt `setGeometry()` 完成，由 Qt 负责按目标屏幕 DPR 转换到 native 坐标
  2. 后续 Win32 `SetWindowPos()` 仅带 `SWP_NOMOVE | SWP_NOSIZE`，只维护阴影位于 owner 后方的 z-order 和显示状态，不再传逻辑像素坐标
  3. `m_legacyLiveResize` 只在 resize hit-test 或 `WM_SIZING` 时进入；普通窗口移动期间保持阴影可见，并由 `moveEvent()` 连续刷新阴影几何
  4. 增加 `antWindowLegacySoftwareShadowGeometryMode` / DPR 诊断属性和回归测试，验证 Qt 逻辑几何、native HWND 物理尺寸和 live-move 跟随行为
- **改动文件**：`src/widgets/AntWindow.cpp`、`tests/TestAntQtExtensions.cpp`

### 39. AntDockWidget 浮动窗口在 Win10 缩放比例下软件阴影异常

- **现象**：`AntDockWidget` 浮动为独立窗口后，在 Windows 10 legacy frame 路径和 125% / 150% 等系统缩放下，软件阴影可能与浮窗轮廓错位或尺寸不一致。
- **根因**：`AntDockWidget::updateLegacySoftwareShadow()` 与旧的 `AntWindow` 路径一样，先用 Qt `setGeometry()` 设置阴影窗口，再把同一个逻辑像素 `QRect` 传给 Win32 `SetWindowPos()`。`SetWindowPos()` 在 per-monitor DPI aware 进程中按物理像素解释坐标和尺寸，导致阴影 HWND 在非 100% 缩放时被错误定位 / 缩放。浮窗阴影还有一个 native ring region，虽然它已经基于 `GetWindowRect()` 反推 DPR，但后续错误的 `SetWindowPos()` 会让 region 和最终窗口尺寸再次不一致。
- **解决**：
  1. 浮窗阴影的 move/size 只通过 Qt `setGeometry()` 完成，由 Qt 按当前屏幕 DPR 转换 native 几何
  2. `SetWindowPos()` 只带 `SWP_NOMOVE | SWP_NOSIZE`，仅维护阴影位于浮窗 owner 后方的 z-order / show 状态
  3. 在 `ScreenChangeInternal` / `DevicePixelRatioChange` / `WM_DPICHANGED` 时同步阴影 `QWindow::screen()`，再刷新 native frame 和阴影几何
  4. 增加 `antDockForceLegacyFramePolicy` 内部诊断属性，让当前系统也能强制覆盖 Win10 legacy shadow 路径；回归测试验证 Qt 逻辑几何、native HWND 物理尺寸和 ring region 状态
- **改动文件**：`src/widgets/AntDockWidget.h`、`src/widgets/AntDockWidget.cpp`、`tests/TestAntQtExtensions.cpp`

### 40. Win10 DockWidget 浮窗回嵌后动画/渲染更新仍会卡顿

- **现象**：Windows 10 下，把 `AntDockWidget` 从布局拖成 float 浮动窗口，再重新嵌入 Dock 布局后，示例窗口内控件的动画、hover、点击状态和自动 repaint / update 仍可能出现明显卡顿或延迟刷新。表现不是完全冻结（已解决问题 #36 已处理过一轮确定性"刷新停止"），而是动画/渲染更新节奏不稳定。
- **根因**：浮动 Dock 在 Win10 legacy frame 路径下会牵涉两类残留：
  1. **`AntDockLegacySoftwareShadow` widget**：`updateLegacySoftwareShadow()` 懒加载一个 `Qt::Tool` 顶级 widget，其 HWND 携带 `WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE`，登记在 DWM 合成跟踪表中。回嵌时旧的 `resetNativeFloatingWindowForEmbedding()` 只调用 `hideLegacySoftwareShadow()` 隐藏 HWND，又通过 `destroy(true, true)` 销毁 Dock 自身 HWND，但 `m_legacySoftwareShadow` widget 实例（包括隐藏的 layered HWND 和 Qt 平台层的 QWindow 状态）继续留在进程中。该隐藏 layered 顶级窗口在 Win10 下会扰动同进程其他顶级窗口（特别是宿主 `AntWindow`）的 backing store 提交节奏，导致页面控件的动画/重绘节奏不稳定。
  2. **`queueDwmFrameRefresh` 延迟回调**：`applyNativeWindowFrame()` 通过 `QTimer::singleShot(0, ...)` 排队一次 DWM frame 重应用。如果在 timer 触发前 Dock 已经从浮动状态回嵌成普通 child widget，旧回调依然会执行 `guard->winId()` —— 而嵌入态 Dock 的 `WA_NativeWindow=false` 让 `winId()` **强制为已经嵌入到 manager 的 child 重新创建一个本地子 HWND**，并把它再次 enroll 进 DWM frame extension。这恰好制造了"已经清掉一个残留 layered 顶级，又通过回调凭空生成一个新的本地子 HWND" 的循环。
  Win11 不进入该路径（`useNativeCaption=true` 时 `updateLegacySoftwareShadow()` 直接 early-return 不创建 shadow widget，`queueDwmFrameRefresh` 携带的 `useNativeCaption=true` 也走另一条 DWM 路径），因此不受影响。
- **解决**：
  1. `AntDockWidget::resetNativeFloatingWindowForEmbedding()` 在 `hideLegacySoftwareShadow()` 之后显式 `delete m_legacySoftwareShadow` 并清空指针，让阴影 widget、QWindow 与底层 layered HWND 一并消失；下一次浮动时再由 `updateLegacySoftwareShadow()` 懒重建。同时立刻清掉 `kDockLegacyLiveResizeProperty` / `kDockNativeFrameEnabledProperty` / `kDockUsesNativeCaptionFrameProperty` / `kDockDwmFrameRefreshQueuedProperty` 四个属性，避免与已排队的回调发生混读。
  2. `queueDwmFrameRefresh` 内部 timer 回调新增三道闸：必须仍 `isWindow()`、Dock 必须 `isFloating()`、必须有 `internalWinId()`（不再用 `winId()` 隐式创建 HWND）。任一条件不满足直接 return，不再触碰 DWM。这彻底杜绝了"回调触发时 Dock 已经嵌入，结果反而给嵌入态 child 生成本地 HWND" 的回路。
  这两点改动都仅在 Win10 legacy frame 路径下生效（Win11 上 `m_legacySoftwareShadow` 始终为 `nullptr`，`delete nullptr` 为空操作；`queueDwmFrameRefresh` 在 Win11 路径上的所有 dock 嵌入态判断也是 short-circuit 的）。
- **验证**：`tests/TestAntQtExtensions.cpp::dockManager()` 中已有的 forced-legacy-frame AntWindow 宿主回嵌场景：
  1. `PaintProbeWidget` 扩展为记录每两次 `paintEvent` 之间的真实毫秒间隔，提供 `paintIntervalsMs()` / `maxPaintIntervalMs()` / `resetIntervalSamples()` API。
  2. 在 `setDockWidgetFloating` 之前对宿主页面跑一次 12 帧 / 16 ms-per-frame 的 `pageProbe.setFillColor` 动画 cadence 作为**基线**，记下基线 max frame interval。
  3. `dragFloatingDockBack` 回嵌之后立即断言：`antDockLegacyShadowDestroyedOnEmbed=true`、`findChild<AntDockLegacySoftwareShadow>` 为空、`antDockNativeWindowFrameEnabled=false`、`antDockUsesNativeCaptionFrame=false`、`antDockLegacyLiveResize=false`，并枚举 `windowExplorer->findChildren<QWidget*>()` 全部断言 `!desc->internalWinId()`。
  4. 然后重复同一个 12 帧 / 16 ms-per-frame cadence，断言 post-embed 的 max frame interval ≤ max(80 ms, 3 × baseline)。该阈值能容忍 OS 级调度抖动，但仍能捕获回调导致的卡顿回归（残留 layered HWND 或回调凭空生成的本地 HWND 会让 worst frame gap 膨胀到基线的多倍）。
  在我的环境上跑 `TestAntQtExtensions::dockManager` 5 / 5 通过。
- **改动文件**：`src/widgets/AntDockWidget.cpp`、`tests/TestAntQtExtensions.cpp`

### 41. Win10 AntWindow 走 opaque 路径(直角+阴影),修复拖回卡顿/黑屏/灰边/focus 框/圆角卡死

- **现象**：上一轮 #40 修复后,Win10 真机依然能复现一连串症状:
  1. Dock 浮动→回嵌后,主窗口控件 hover/click 动效卡顿
  2. 切换主页(侧栏导航)卡顿
  3. 多次缩放主窗体后,整个窗口黑屏
  4. 主窗口四边出现灰色方框(Windows native 边框颜色)
  5. 从其他应用切回主窗口时出现 focus 框
  6. 缩放后圆角有时会卡死成直角,不再恢复
- **根因**:Win10 上 `AntWindow` 同时启用 `Qt::WA_TranslucentBackground=true`、`AntWindowCornerSmoother` 用 `CompositionMode_DestinationIn` 擦角、并在 `beginLegacyLiveResize` 时调用 `DwmExtendFrameIntoClientArea({-1,-1,-1,-1})` "全玻璃扩展"。这三件事组合起来,在 Win10 DWM 合成器上是不稳定状态——反复 resize 会让 DWM 合成器拒绝接受新帧(黑屏),hover 路径上多余的 alpha-erase pass 让 backing store 提交节奏抖动(动效卡顿),并放大 Dock 浮窗 lifecycle 残留对主窗口的扰动(#40 修了一部分,但还是抑制不住根因)。同时 `setWindowAttribute(hwnd, DWMWA_BORDER_COLOR, ...)` 给 `WS_THICKFRAME` 没 `WS_CAPTION` 的窗口画了灰色边框,`WM_NCACTIVATE` / `WM_NCPAINT` 在 focus 切换时也会重绘 NC sizing border。圆角"卡死"则源自 `m_legacyLiveResize` 状态机依赖 `WM_EXITSIZEMOVE` 复位——`WM_CAPTURECHANGED` / `WM_CANCELMODE` 等异常 modal 终止路径会让 flag 卡在 true,后续 paint 全部画方角。Win11 caption 路径完全不进入这套机制,所以不受影响。
- **解决**:让 Win10 走"直角 + 阴影"opaque 路径,与 Win11 的"圆角 + alpha 角 + DWM glass"路径完全分离:
  1. `AntWindow` 构造函数检测 `supportsNativeCaptionSnapLayouts()`,Win10 下 `m_useTranslucentBackground=false`,**不**设 `WA_TranslucentBackground`,**不**创建 `AntWindowCornerSmoother`。新增公共接口 `bool usesLegacyOpaquePath() const` 供 style/shadow 查询。
  2. `AntWindowStyle::drawWindow` 在 `usesLegacyOpaquePath()` 时强制 `cornerRadius=0`,直接画方角,不再 `setClipPath`。
  3. `AntWindowLegacySoftwareShadow::setCornerRadius` 在 opaque 路径传 0,画方形阴影贴合方角窗口轮廓;阴影本身保留(提供窗口可见外沿)。
  4. `beginLegacyLiveResize` lambda 在 opaque 路径 short-circuit return:不进入 `m_legacyLiveResize=true` 状态,不藏阴影,不调用 `DwmExtendFrameIntoClientArea({-1,-1,-1,-1})`。整条 live-resize 状态机在 Win10 上变成 no-op,从根本消除"圆角卡死"。
  5. `applyNativeWindowFrame` 在 opaque 路径用 `DWMWA_COLOR_NONE` (`0xFFFFFFFE`) 显式让 DWM 不画边框,消除四边灰色方框。
  6. `nativeEvent` 在 opaque 路径拦截 `WM_NCACTIVATE`(透传给 `DefWindowProcW` 但 `lParam=-1` 跳过 NC 重绘)和 `WM_NCPAINT`(直接吞掉返回 0),消除 focus 框。
  7. `WM_EXITSIZEMOVE | WM_CAPTURECHANGED | WM_CANCELMODE` 三个消息共用同一条复位路径,并立即同步 `update()` 一次,作为 alpha-corner 路径上 modal loop 异常终止的兜底(opaque 路径上 flag 永远是 false,这段代码是 dead code,但保留以备将来恢复 Win10 圆角)。
  Win11 caption 路径完全不变(`m_useTranslucentBackground=true`)。
- **验证**:
  - `TestAntQtExtensions::dockManager` + `windowLegacyFramePolicyRestoresShadowAfterResize` + `windowNativeHitTestSupportsSnapZones` + `windowDwmFrameMarginsPreserveShadow` 全部通过
  - `windowLegacyFramePolicyRestoresShadowAfterResize` 测试更新:smoother 在 opaque 路径上不存在,把"必须存在"改为"存在时检查 click-through 与非 native"
  - `dockManager` 内部那段"forced-legacy 宿主"场景同样跳过 smoother 必现断言
  - example 真机验证(用户回报):卡顿、黑屏、灰边、focus 框、圆角卡死全部消失。视觉代价:Win10 主窗口圆角降级为方角(阴影保留)
- **改动文件**:`src/widgets/AntWindow.h`、`src/widgets/AntWindow.cpp`、`src/styles/AntWindowStyle.cpp`、`tests/TestAntQtExtensions.cpp`

### 42. AntWindow 缩小尺寸时偶发闪动

- **现象**:已解决问题 #7 引入的 DWM `{-1,-1,-1,-1}` 兜底后已大幅改善,但缩小时偶尔(约几帧出现一次)仍能看到极短闪动。
- **根因**:Win10 上 `WA_TranslucentBackground=true` + `DwmExtendFrameIntoClientArea({-1,-1,-1,-1})` "全玻璃扩展"在 DWM backdrop 接管和 Qt backing store flush 之间存在帧级竞争,会导致缩小时新尺寸边缘 1-2 帧透明闪烁。
- **解决**:本症状原本是"未解决",在 #41 把 Win10 切到 opaque 路径(无 `WA_TranslucentBackground`、无 DWM glass 扩展)后已被一并消除;Win11 caption 路径下用户在真机验证未复现,可视为已解决。
- **改动文件**:已包含在 #41 的改动里(`src/widgets/AntWindow.h`、`src/widgets/AntWindow.cpp`、`src/styles/AntWindowStyle.cpp`)。

### 43. AntWindow 主题切换速度变慢

- **现象**:点击标题栏的 Light/Dark 切换按钮,从主题真正变化到 overlay 揭示动画结束,整体耗时变长,过渡不像之前那么干脆。
- **适用范围**:Win10 / Win11 似乎都能感觉到,Win10 opaque 路径上(#41 修复后)更明显。
- **根因**:
  1. `startThemeModeTransition()` 为了拿到新旧两帧,在显示旧帧 overlay 后调用两次 `QCoreApplication::processEvents()` 再 `grab()`。这会把主题切换期间整棵 widget tree 的 repaint 全部同步压进点击处理栈里,用户体感就是"点下去之后先卡一下,动画才开始"。
  2. `AntWindowThemeTransitionOverlay` 的 reveal 羽化每帧用 24 层 `QPainterPath::subtracted()` 环形裁剪绘制新帧,窗口越大每帧路径布尔运算越重。
  3. Win10 opaque 路径已经不再需要 alpha 圆角揭示,继续使用 translucent reveal overlay 会让 opaque 宿主和 translucent child 走两套合成路径。
- **解决**:
  1. 新增同步 `render()` 捕获路径 `captureAntWindowFrame()` 替代 `grab()` + `processEvents()`。主题切换前后都直接 render 当前 widget tree 到带 DPR 的 `QPixmap`,不再通过事件循环强制刷新两次。
  2. overlay 动画从 320 ms / 8 ms tick 调整为 220 ms / 16 ms tick,减少总时长和每秒绘制次数。
  3. Win10 opaque 路径改用轻量 crossfade 模式:旧帧铺底,新帧按 progress 淡入,不再做圆形 reveal / feather path 运算。Win11 caption 路径仍保留 circular reveal。
  4. overlay 增加诊断属性 `transitionCaptureMethod=render`、`transitionUsesEventLoopCapture=false`、`transitionMode`。回归测试验证不再走事件循环捕获,并按 Win10/Win11 路径检查 crossfade / circular-reveal 模式。
- **改动文件**:`src/widgets/AntWindow.cpp`、`tests/TestAntQtExtensions.cpp`

### 44. AntColorPicker 弹窗下方多重边缘 + 拖动选色卡顿

- **现象**:点击 `AntColorPicker` 触发器弹出颜色面板时:
  1. 弹窗的下边缘(可能也包括左右下角)出现明显的多层 / 重影边框,看起来像是阴影裁切边界 + 面板边框 + 系统边框叠加
  2. 在 HS 取色区域(色相 / 饱和度大方块)按住鼠标拖动选色时,鼠标位置与采样指示器更新明显跟不上,有可见延迟
- **适用范围**:Win10 / Win11 都能复现(待用户细分)。
- **根因**:
  1. 弹层虽然设置了 `Qt::NoDropShadowWindowHint`,但 `QFrame` 默认 frame 状态没有显式清零,同时手绘 shadow 强度偏高,下边缘会把 panel border、手绘阴影层和潜在平台边缘叠在一起。
  2. HS 取色区域每次拖动都重画整块 234×160 的 hue/saturation/value 渐变,即使 hue 没变也会重新生成三层渐变背景;同时 `setCurrentColor()` 每帧都会 `updateGeometry()`,拖动时额外触发布局请求。
- **解决**:
  1. `ColorPickerPopup` 显式 `setFrameShape(QFrame::NoFrame)`、lineWidth/midLineWidth 归零、设置 `WA_NoSystemBackground`、保留 `Qt::NoDropShadowWindowHint`,并暴露 `antColorPickerPopupNoNativeShadow=true` 诊断属性。
  2. ColorPicker 弹层手绘阴影改为 12px / 0.30 strength,保留柔和 elevation,但降低下边缘多层叠影。
  3. `HueSatField` 增加按 hue + size + DPR 缓存的背景 pixmap。拖动时只局部刷新旧/新指示器区域,不再每帧重算整块渐变。
  4. Hue/Alpha/Preview/HEX 更新都增加"值未变化则不 update/setText"保护;`AntColorPicker::setCurrentColor()` 移除拖动期间不必要的 `updateGeometry()`。
  5. 回归测试验证 popup 无原生阴影/无 QFrame frame、底部透明边缘不被阴影裁切、下方阴影 alpha 逐步衰减,并验证 HS field 连续拖动不会重建背景缓存。
- **改动文件**:`src/widgets/AntColorPicker.cpp`、`tests/TestAntQtExtensions.cpp`

### 45. AntWindow 主题切换后 Showcase 文字被截断

- **现象**:点击 `AntWindow` 标题栏 Light/Dark 切换后,Showcase 首屏里的弹窗预览文字顶部显示不完整,像是新主题字体已经生效但布局高度仍停留在旧状态。
- **根因**:#43 为了提速移除了主题切换中的完整 `processEvents()`。这避免了绘制/动画/输入事件被同步塞进点击流程,但也让 `themeChanged` 触发的 `updateGeometry()` / `LayoutRequest` 尚未应用时就捕获了新主题帧。结果 overlay 的 new-frame 可能使用"新字体度量 + 旧 layout geometry",多行文本在固定卡片里出现截断。
- **解决**:在 `AntWindow::startThemeModeTransition()` 切换主题后、捕获 new-frame 前,递归激活当前窗口的布局树:对子 widget `ensurePolished()`,对布局执行 `invalidate()` + `activate()`。这样只刷新布局,不恢复完整事件循环,既保留 #43 的速度优化,又确保新主题帧和真实界面都使用更新后的 geometry。
- **验证**:`TestAntQtExtensions::windowThemeButtonShowsTransitionOverlay` 新增一个随主题切换改变 `sizeHint()` 的控件,断言点击主题按钮返回时布局高度已从 32 立即更新到 96;同时保留 transition overlay 的 render 捕获和模式断言。
- **改动文件**:`src/widgets/AntWindow.cpp`、`tests/TestAntQtExtensions.cpp`

### 46. AntColorPicker 弹窗下方残留透明边缘

- **现象**:#44 后下方多重阴影明显改善,但弹窗底部仍能看到一点多余透明边缘。
- **根因**:`ColorPickerPopup` 仍按旧阴影方案保留 40px 外部透明窗口边距,而当前软件阴影宽度已缩到 12px。底部剩余的完全透明 top-level 区域在 Windows 合成下会被感知成一条额外边缘。
- **解决**:将 `kColorPickerPopupShadowMargin` 从 40 收紧到 28,仍覆盖 12px 软阴影的最大扩散范围,但移除底部多余透明窗口区域;新增 `antColorPickerPopupShadowMargin` 诊断属性,测试按实际 margin 取样并限制 margin 不超过 shadowWidth 的 3 倍。
- **验证**:`TestAntQtExtensions::colorPicker` 通过,继续验证 popup 无原生阴影、底部 alpha 衰减、最外侧边缘透明以及 HS field 拖动缓存。
- **改动文件**:`src/widgets/AntColorPicker.cpp`、`tests/TestAntQtExtensions.cpp`

### 47. AntColorPicker 拖动选色实时刷新卡顿

- **现象**:HS 取色区域拖动时,指示器位置虽然能动,但实时颜色刷新仍有明显卡顿。
- **根因**:#44 只缓存了 HS 色块背景,但每个 `mouseMove` 仍同步执行整条刷新链:更新 alpha slider、写入 HEX `QLineEdit`、重绘预览色块、刷新外部 trigger,并立即发送 `currentColorChanged` / `colorSelected`。这些外部刷新如果触发布局或业务联动,会反过来拖慢鼠标事件处理。
- **解决**:将拖动链路拆成两级:
  1. HS 指示器仍立即局部重绘,保证鼠标跟手。
  2. HEX / alpha slider / preview / owner trigger / 对外信号合并到 16ms live refresh 定时器里,连续 `mouseMove` 只保留最新颜色并合帧刷新。弹窗隐藏前会 flush 一次 pending refresh,避免丢失最终颜色。
- **验证**:`TestAntQtExtensions::colorPicker` 连续发送多次 HS field 拖动事件,断言背景缓存不重建,并验证多次拖动只合并成一次 live refresh / 一次 `colorSelected` / 一次 `currentColorChanged`。
- **改动文件**:`src/widgets/AntColorPicker.cpp`、`tests/TestAntQtExtensions.cpp`

### 48. AntColorPicker 拖动选色仍卡顿的渲染回归

- **现象**:#47 合并了外部刷新后,拖动选色仍有卡顿;用户反馈上周该路径不卡,说明瓶颈更像近期透明弹层绘制路径带来的渲染回归,不是 HSV 计算或业务信号本身。
- **根因**:HS 取色区域位于 `WA_TranslucentBackground` 顶层 popup 内,自身没有声明为不透明绘制。拖动时虽然只 `update()` 指示器 dirty rect,Qt 仍可能为了透明子控件向父级追溯背景并参与 layered window 合成,进而把本该很小的 cursor repaint 放大成透明 popup 的背景/阴影合成成本。
- **解决**:将 `HueSatField` 标记为 `WA_OpaquePaintEvent` + `WA_NoSystemBackground`,缓存背景 pixmap 改为用 `colorBgElevated` 填满整块矩形后再绘制圆角色域。这样 HS field 的每次拖动重绘都是不透明小区域,不再依赖父 popup 背景重建;缓存 key 增加 theme mode,避免主题切换后角落背景颜色陈旧。
- **验证**:`TestAntQtExtensions::colorPicker` 验证 HS field 走 opaque paint、最外角像素 alpha 为 255、背景缓存不随连续拖动重建,并继续验证 live refresh 合帧行为。
- **改动文件**:`src/widgets/AntColorPicker.cpp`、`tests/TestAntQtExtensions.cpp`

### 49. AntColorPicker 透明 popup 内拖动仍有轻微顿感

- **现象**:#48 将 HS field 改为不透明绘制后,example 中拖动仍能感觉到轻微卡顿。
- **根因**:即使 HS field 本身 opaque,它仍是 `WA_TranslucentBackground` 顶层 popup 的普通子控件。Windows 下透明顶层窗口通常走 layered backing store,普通子控件 repaint 仍可能触发顶层 layered surface 的区域上传,拖动时就会残留合成成本。
- **解决**:在 Windows 上把 HS field 标记为独立 native 子窗口(`WA_DontCreateNativeAncestors` + `WA_NativeWindow`),并保留 `WA_OpaquePaintEvent` / `WA_NoSystemBackground` / `WA_StaticContents`。这样最高频的拖动 repaint 落在一个不透明 native drag surface 上,避免通过透明 popup 的整块 backing store 合成路径。
- **验证**:`TestAntQtExtensions::colorPicker` 验证 HS field 的 opaque/static paint 属性,并在 Windows 下验证 `antColorPickerNativeDragSurface=true` 与 `WA_NativeWindow` 生效。
- **改动文件**:`src/widgets/AntColorPicker.cpp`、`tests/TestAntQtExtensions.cpp`

### 50. AntColorPicker native drag surface 导致圆点不动

- **现象**:#49 把 HS field 改为 native 子窗口后,拖动刷新略快,但取色小圆点在 example 中不再跟随鼠标移动。
- **根因**:透明 popup 内嵌 native 子窗口会绕开 Qt 普通 QWidget 合成路径,在实际窗口中容易出现 native child 与透明 top-level backing store 的同步/裁剪问题。该方案虽然减少了顶层透明合成压力,但破坏了圆点的可见更新。
- **解决**:撤掉 `WA_NativeWindow` 方案,改为普通 QWidget + 独立 `AntColorPickerHueSatCursor` overlay 子控件。HS field 只绘制并缓存不透明背景;拖动时同步移动 22x22 cursor overlay,只让旧/新小区域刷新,不再重画整块色域。鼠标按下时停止 popup 进入动画,避免刚打开就拖动时动画与取色拖动抢刷新。
- **验证**:新增 `TestAntQtExtensions::colorPickerDragSmoothness` 专项测试,连续发送 240 次拖动事件,逐次断言 cursor position 同步到鼠标点,Debug 下事件分发低于 80ms,背景缓存不重建,并验证外部刷新仍只合并为一次 live refresh。
- **改动文件**:`src/widgets/AntColorPicker.cpp`、`tests/TestAntQtExtensions.cpp`

### 51. AntColorPicker 拖动后 HS 区域出现白线

- **现象**:#50 使用透明 cursor overlay 后,example 中拖动取色会在 HS 色域内残留白色碎线。
- **根因**:透明子控件在透明 popup 内高频移动时,旧位置的白色圆点描边会参与父/子 backing store 的局部合成;部分旧 dirty 区域没有被稳定清空,形成白线拖影。
- **解决**:移除透明 cursor 子控件,改回 HS field 自身绘制圆点,但保留缓存背景和 dirty repaint:每次拖动只刷新旧/新圆点外接矩形的并集,不重绘整块色域,同时继续在鼠标按下时停止 popup 进入动画。
- **验证**:`TestAntQtExtensions::colorPicker` 与 `colorPickerDragSmoothness` 验证不再创建 cursor overlay、圆点位置仍逐事件同步、240 次拖动 Debug 分发低于 80ms、背景缓存不重建,并在最终渲染中扫描饱和蓝色区域没有额外白色残留像素。
- **改动文件**:`src/widgets/AntColorPicker.cpp`、`tests/TestAntQtExtensions.cpp`

## 未解决

当前暂无记录。

## 关联测试

- `tests/TestAntQtExtensions.cpp`：覆盖 AntWindow / AntDockWidget 大部分行为
- `tests/TestAntSwitch.cpp`、`tests/TestAntDataEntryA.cpp`：覆盖 AntSwitch / AntSegmented
- 改动这些控件时按 `AGENTS.md` "测试范围规则" 只跑对应 CTest target，不要默认全量 ctest
