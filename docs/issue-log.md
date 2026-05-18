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
- **解决**：新增 manager-owned `AntDockContextMenuPopup` 轻量弹层，内部承载 `AntMenu`；右键命令改为 `AntMenuItem` key 分发，保留浮动、tab 移动、拆分、关闭等原有行为，同时复用 `AntMenu` 的紧凑布局、亮/暗主题、disabled / danger 状态、圆角面板和阴影绘制。
- **改动文件**：`src/widgets/AntDockManager.cpp`、`tests/TestAntQtExtensions.cpp`

## 未解决

### A. AntDockWidget 浮动窗口在主布局管理器不在前台时无法拖动

- **现象**：当 `AntDockManager` 所在的主布局管理器不在前台显示（被其他窗口遮挡或最小化）时，浮动出来的 `AntDockWidget` 无法被拖动。
- **目前怀疑**：浮动 dock 的拖拽是 manager 的 `eventFilter` + 应用级 event filter 共同实现的——manager 不可见时它对应的事件可能被 Qt 优化掉、或者 owner HWND 失活导致拖动消息被 Win32 路由到别处。
- **潜在排查方向**：
  1. 在 `installDockEventFilters` / `handleDockTitleMouseEvent` 加日志，确认 manager 不可见时这些路径是否仍然走到
  2. 检查 `m_appEventFilterInstalled` 路径是否依赖 manager 自己 visible
  3. 浮动 dock 在 manager 不可见时是否需要切换为 self-owned 顶层窗口（牺牲 owner 关系换可拖动）

### B. AntWindow 跨屏拖动时阴影错位（动态跟随问题）

- **现象**：把 AntWindow 拖到其他屏幕时，阴影位置/尺寸的更新和窗口轮廓对不齐——阴影应该实时跟着主窗口画，而不是在跨屏后才补正。
- **与已解决问题 #5 的区别**：#5 只处理了 `WM_DPICHANGED` / `ScreenChangeInternal` 这类**离散事件**，阴影 HWND 在 DPI 切换的那一帧才同步。但 AntWindow `moveEvent` 在 live-drag 期间也被抑制（`if (m_legacyLiveResize) return`），所以拖拽中的连续 move 事件阴影完全不跟随。
- **潜在排查方向**：
  1. 拖动 AntWindow（不是 resize）时不应该走 `m_legacyLiveResize` 抑制——拖动期间 `WM_ENTERSIZEMOVE` 是 move 还是 resize 触发？需要区分这两种 size-move 模式
  2. 或者让 `moveEvent` 始终更新阴影 HWND 位置，只在 resize 时跳过（拖动主要是 reposition，阴影 `SetWindowPos` 在同 DPI 内是便宜的）
  3. 检查 `WM_DPICHANGED` 的处理是否在跨屏首帧就跟上了阴影 HWND 的 `setScreen()`

### C. AntWindow 缩小尺寸时偶尔仍有闪动

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
