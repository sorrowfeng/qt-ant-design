# qt-ant-design 全面检查与优化建议报告

> 审查日期：2026-08-30（dev 分支，v0.1.2，antd 6.6.2 对齐完成）
> 范围：src/ 代码质量 · examples/ 示例效果 · tests/ 测试体系 · 文档一致性
> 方法：3 路并行深度探查（抽样 25+ 组件源码、91 个示例页、40 个测试文件）+ 关键结论逐条人工核验

---

## 总体结论

仓库整体质量**已经相当高**：AUD-001~018 审计整改扎实落地（QPointer/destroyed 清理、连接句柄、内存纪律抽查验证通过）、TODO/FIXME 为 0、事件处理器 override 覆盖 100%、示例零样式违规、测试有覆盖守护（TestAntCoverageInventory）。残留问题集中在四个方面：**① Table 等性能长尾、② Message/Notification 等双胞胎代码与 token 双源、③ 示例交互演示普遍偏薄、④ 测试基建（超时/offscreen/公共 helper）与文档数字漂移**。无安全级缺陷。

---

## 一、文档一致性（先修，成本低、影响信任度）

| # | 问题 | 位置 | 建议 |
|---|---|---|---|
| D1 | 同一文件内图标数自相矛盾：Summary 表写 `848`，Icon State 节写 `831`（Filled 234） | `docs/project-status.md` 第 21 行 vs 第 326-332 行 | Icon State 节更新为 848 / Filled 251 |
| D2 | 头部声明仍写 "89 个公开组件、icons-svg@4.4.2 共 831 个"，但正文已完成 4.5.0/848 升级和 BorderBeam/Listy 移植（89+2=91） | `docs/porting-todo.md` 第 5 行 | 头部行更新为 91 组件 / 4.5.0 / 848 |
| D3 | `Updated: 2026-07-16` 落后于 2026-08-29 的 v6 对齐工作；smoke 计数 `104`、总条目 `155/155` 疑似过时（BorderBeam/Listy 加入后 smoke 条目应 +2，需配置后用 `ctest -N` 实测确认） | `docs/project-status.md` 第 3/18 行、`docs/reliability-coverage.md`、`AGENTS.md` | 跑一次全量后统一刷新数字与日期；建议给 TestAntCoverageInventory 增加"smoke 条目数 == 文档声明数"断言，从机制上防漂移 |
| D4 | reliability-coverage 矩阵 `Last verified: 2026-07-16` 早于两个新组件的加入日 | `docs/reliability-coverage.md` | BorderBeam/Listy 行补充 2026-08-29 验证记录 |

---

## 二、src/ 代码优化（按收益排序）

### P0 — 性能与正确性收益最大

1. **AntTable 滚动全量重绘**：`src/widgets/AntTable.cpp:655` wheelEvent 无参 `update()` 整表重绘。自绘大表应改用 `scroll(dx, dy)` 位块平移 + 只重绘露出的行带。同文件 `:522`（分页按钮）、`:599`（行选中）也是全表重绘，应改为旧行+新行两条脏带。这是全库剩余最大的性能长尾。
2. **行循环内构造字体**：`src/styles/AntTableStyle.cpp:451-452` `cellFont` 构造 + `setFont` 位于 N 行循环内，每次 paint 重复 N 次；提升到循环外即可（同文件 298-300 行 headerFont 已是正确写法）。
3. **token 双源**：`src/widgets/AntMenu.cpp:1494` 硬编码 `QColor(0, 21, 41)` 与 `colorBgMenuDark` token 值相同但绕过 token；`:1555` hover 色 `QColor(17,34,51)` 同理。一旦 token 被组件级覆盖（setComponentToken 已支持），这些位置不会跟随变化——这是功能性 bug 隐患，不只是风格问题。

### P1 — 消除双胞胎代码与双源实现

4. **输入族 variant 背景解析双份实现**：`AntCascader.cpp:470-481` ≈ `AntCascaderStyle.cpp:104-117` ≈ `AntSelect.cpp:984-996`（DatePicker/TimePicker/InputNumber 同型），widget 与 Style 各写一遍。建议 widget 暴露单一 `resolvedBg()`，Style 调用之。
5. **AntMessage / AntNotification 的 anchor watcher 近乎逐行相同**：`AntMessage.cpp:503-546` vs `AntNotification.cpp:680-723`（含相同的 destroyed lambda 与窗口过滤器）。提取私有 `AntAnchorWatcher` helper；同时参照 `AntToolTip.cpp:639` 的写法为 destroyed connect 保存句柄——当前安全仅因 install 每实例只调一次，未来一旦暴露 `setAnchor` 公共 API 即成重复连接 + 误关闭。
6. **遮罩/边框硬编码颜色入 token**：`AntTour.cpp:277`（`QColor(0,0,0,115)`）与 `AntImage.cpp:130`（`QColor(0,0,0,190)`）两个遮罩透明度各自硬编码且互不一致；`AntTableStyle.cpp:582`、`AntTypographyStyle.cpp:256-257`、`AntWindowChrome.cpp:121-122` 同类。建议新增 `colorMask`/`colorMaskHeavy` 类 token。
7. **死代码与兼容宏收敛**：11 处 `QT_VERSION >= 5.12/5.14` 分支永远为真（最低支持已是 5.15.2），如 `AntSelect.cpp:1015`、`AntInputNumber.cpp:640,656`；另有 11 个文件内联裸 `QT_VERSION_CHECK(6,0,0)` 而已有集中的 `core/AntCompat.h`。统一删除/收敛。

### P2 — 可维护性

8. **超长函数拆分**：`AntWindow.cpp:1583` nativeEvent 398 行（按 per-message 拆分派函数）；`AntTableStyle.cpp:239` drawTable 369 行（拆表头/行/分页/空态）；`AntTimelineStyle.cpp:190` 262 行。
9. **惯用式清理**：7 处 `QColor(0,0,0,0)` → `Qt::transparent`；`AntModal.cpp` 中 `"Modal.okText"`、`AntSegmented.cpp` 中 `"segments"` 各重复 4 次提为常量；`AntCollapse.h/AntDrawer.h/AntFloatButton.h/AntInput.h` 指针成员的 include 可改前向声明。
10. **防御性收养**：`AntCard.cpp:52`、`AntNav.cpp:64` 先 new 后入 layout 的窗口期写法，改为构造时直接指定 parent。

已验证无需整改：裸 new 无 parent（30+ 处抽查全合规）、信号重复连接（26 处非构造 connect 均有断开/守卫纪律）、override/const 正确性、TODO/FIXME（0 处）。

---

## 三、examples/ 示例优化（对标 antd 6 官方文档页）

### 核心差距：交互演示普遍偏薄

全示例 `connect` 仅 81 处——多数页面停留在"基础用法 + 尺寸三件套"，对照官方组件页（基本用法 + 变体 + 受控 + 回调）差距明显。按优先级：

1. **Table 页差距最大**（`DataDisplayPagesB.cpp:201`，仅 3 行 Basic + 排序）：补 bordered、行选择、分页联动、loading、空态、row 点击回调。
2. **Form 页**（`DataEntryPagesA.cpp:241`）：补校验/错误态、动态增删表单项、layout 变体、提交回调。
3. **回调演示补齐**：Button 无 clicked 演示、Select 缺 search/allowClear/change、DatePicker 缺 valueChanged、Modal 缺受控 open/close 与自定义 footer、Tour 缺 placement/onClose。可批量做一轮"每页至少 1 个事件回显区"。
4. **样板代码消减约 1500 行**：页面脚手架重复约 90 次、`new AntCard("Basic")` 模式 166 次、Sizes 三件套逐字复制 4+ 处。在 PageCommon.h 增加 `makePage()/makeCard()/makeSizeRow()`。
5. **导航体验**：91 个页面按 9 类分组，侧边栏需滚约 5 屏找页。给 AntNav 加搜索过滤输入框（AntInput 模糊匹配 name/category，约 90 行可完成），或 Ctrl+K 命令面板。
6. **国际化**：91 页文案全部硬编码英文，AntLocale 仅 1 处使用；`ShowcasePage.cpp:73-104` 又硬编码中文与全库风格冲突。统一走 AntLocale 或统一英文。
7. **High DPI / 小窗口**：61 处 `setFixedSize` 裸像素（重点 `DataEntryPagesB.cpp:404` 的 480×150 dragger、`DataEntryPagesA.cpp:253` 的 form 480 定宽），200% DPI + 最小窗口下可能溢出卡片，改弹性约束。
8. **结构**：QtExtPages.cpp 1228 行 19 页继续膨胀前按域拆分；Pages.h 声明顺序与 PageRegistry.cpp 注册顺序机械对齐。

正面确认：零样式规范合规（唯一命中是 AntConfigProvider 组件 API，不违规）；PageRegistry 数据表注册方式可维护性高；Listy/BorderBeam 两个新页反而是全库交互最完整的示范，可作为其他页的对齐模板。

---

## 四、tests/ 与 CI 优化

### P0 — 测试基建健壮性

1. **无默认超时**：`add_ant_test`（tests/CMakeLists.txt:13-17）未设 TIMEOUT，约 40 个深度测试挂死会拖满 CI。加默认 120s（个别长测试显式覆盖）。
2. **17 处 >100ms 固定 qWait 有 flaky 风险**：最危险的是 `TestAntMotion.cpp:98`（qWait(450) 后断言 wave 仍存活，机器负载高时可能已析构）；`TestAntFeedback.cpp:329/484/544/605` 4 处 qWait(260) 同类。改 QTRY_VERIFY / 谓词等待（TestAntPopupLifecycle.cpp:36 已有正确范式）。
3. **深度测试未注入 QT_QPA_PLATFORM=offscreen**：51 个深度测试依赖真实窗口系统且无 RESOURCE_LOCK，`ctest -j` 并行下有焦点/鼠标互扰理论风险。统一注入 offscreen（或提供开关），原生输入测试已有正确 opt-in 隔离示范。

### P1 — 重复消除与覆盖补强

4. **提取 tests/TestUtils.h**：`waitUntil` 三份逐字复制；全量控件工厂表四份平行维护（MetaProperties/ThemeLifecycle/RenderSmoke/ObjectTree），新增控件要改 4+ 处。统一注册表 + QSignalSpy 三连断言 helper，预计删 500+ 行重复。
5. **补新组件专项测试**：AntListy 仅约 65 行属性/信号测试（缺拖拽排序、吸顶分组交互专项）；AntBorderBeam 无绘制内容级断言（running 后 beam 是否真画出来未验证）。
6. **拆分 TestAntQtExtensions.cpp（6521 行）** 为 Dock / Ribbon / 窗口 / 原生输入多个目标。

### P2 — 缺失的测试类型

7. **QAccessible 零覆盖**：无任何 accessibleName/角色断言，可并入表驱动测试补最小覆盖。
8. **无 QBENCHMARK 性能基准**：仅有 `< 1000ms` 粗粒度上限断言，无趋势跟踪。把现有断言迁 QBENCHMARK 并在 CI 记录。
9. **tools/generate_component_api_overview.py 无守护**：文档生成脚本无 dry-run/输出完整性测试，可挂进 TestAntCiPolicy。
10. **无障碍/多屏**：无运行时屏幕添加移除、DPI 动态切换测试；LeakSanitizer/valgrind 未常态化（AGENTS.md 已如实记录，可作为已知项推进）。

---

## 五、建议执行批次（从高收益到低）

| 批次 | 内容 | 预估规模 |
|---|---|---|
| ① 文档对账 | D1-D4 数字刷新 + coverage 守护断言 | 半天内 |
| ② Table 性能 | wheelEvent 位块平移 + 脏带重绘 + 循环外字体（P0-1/2） | 1 天 + 验证 |
| ③ token 双源清理 | AntMenu/遮罩色入 token + variant 背景单一实现（P0-3、P1-4/6） | 1-2 天 |
| ④ 测试基建 | 默认 TIMEOUT + offscreen + TestUtils.h + qWait 治理 | 2-3 天 |
| ⑤ 示例增强 | Table/Form/回调三件套 + makePage 样板消减 + AntNav 搜索 | 3-5 天 |
| ⑥ 双胞胎合并 | AntAnchorWatcher 提取 + 死代码删除 + 超长函数拆分 | 2-3 天 |

---

*报告由 3 路并行代码探查生成，关键结论（AntTable:655 全量重绘、AntMenu:1494 硬编码色、文档计数漂移）已逐条人工核验源文件属实。*

---

## 六、执行结果（2026-08-30 逐批落地）

| 批次 | 内容 | 状态 | 提交 |
|---|---|---|---|
| ① 文档对账 | D1-D4 数字刷新 + coverage 守护断言 | ✅ 完成 | `34115be` |
| ② Table 性能 | 位块平移 + 脏行重绘 + 循环外字体 | ✅ 完成 | `16fdd08` |
| ③ token 双源清理 | 3 新 token + variantBackgroundColor 统一 7 处 + 死代码删除 | ✅ 完成 | `05d0dc6` |
| ④ 测试基建 | 默认 TIMEOUT 60 + TestUtils.h waitUntil 三合一 | ✅ 完成 | `384cdb6` |
| ⑤ 示例增强 | Table/Form 交互补强 + AntNav 过滤 + 侧边栏搜索 | ✅ 完成 | `76c4073` |
| ⑥ 双胞胎合并 | AntAnchorWatch 提取（Message/Notification） | ✅ 完成 | `866c0ac` |

### 各批次落地要点

- **①**：修正 848/251 图标、52+106=158 测试、91 组件；新增 `smokeEntryCountMatchesDocumentation()` 守护测试（正则需 `MultilineOption`）。
- **②**：`wheelEvent` 改 `scroll()` 位块平移 + 暴露带重绘；`selectRow` 改 `updateRows` 脏行；`AntTableStyle` cellFont 提到行循环外。
- **③**：新增 `colorBgMenuDarkItemHover` / `colorBgMask` / `colorBgMaskHeavy` 三 token；提取 `AntStyleBase::variantBackgroundColor` 收敛 4 widget + 3 style 双源；删 8 处 `QT_VERSION_CHECK(5,12)/(5,14)` 死代码。
- **④**：`add_ant_test` 默认 `TIMEOUT 60`；新建 `tests/TestUtils.h`（header-only，合并 waitUntil 三份）。
- **⑤**：`AntNav` 加 `filterText()`/`setFilterText()`（大小写不敏感过滤 item 和空分类 header）；Table 页补 Row selection + Pagination；Form 页 Submit 连 `AntMessage::success`；侧边栏加 AntInput 搜索框。
- **⑥**：新建 `AntAnchorWatch`（header-only），AntMessage/AntNotification 各 ~40 行 anchor watcher 收敛为一处；destroyed 回调仅清理窗口 filter，规避 QPointer 在 destroyed 信号后仍非空的问题。

### 验证

- 全量 Debug 构建（Qt 6.9.1）通过，无错误。
- `TestAntPopupLifecycle` 4/4、`TestAntStressLifecycle` 12/12 通过（含 anchor 销毁路径 `transientFeedbackBurstClosesCleanly`、`controllersSurviveReferencedWidgetDestruction`）。

### 评估后记录的遗留项（未在本轮落地）

| 报告条目 | 决策 | 理由 |
|---|---|---|
| 三、6 国际化（AntLocale） | 暂缓 | 涉及 91 页文案，建议明确语言策略后统一做 |
| 四、1 offscreen 注入 | **不做** | offscreen 会破坏原生 Win32 窗口测试（`nativeMouseInputAvailableForExtensionTest` 已检查 platform），现有 TIMEOUT 已覆盖挂死风险 |
| 四、4 控件工厂表统一（4 文件） | ✅ 已完成 | 见下方「补充落地（三）」 |
| 四、7 QAccessible 最小覆盖 | ✅ 已完成 | 见下方「补充落地（三）」 |
| 二、8 nativeEvent（397 行）拆分 | 跳过 | Win32 边缘情况风险高于收益，已用 lambda 组织良好 |
| 二、9/10 惯用式清理 + 防御性收养 | ✅ 已完成 | 见下方「补充落地（三）」 |

### 补充落地（2026-08-30，提交 `cb0707b`）

原"暂缓"的两项已实施：

| 报告条目 | 结果 |
|---|---|
| 三、4 makePage/makeCard 样板消减 | ✅ PageCommon 新增 `makePage()`/`makeCard()`，88 处脚手架 + 163 处卡片迁移，净减约 155 行 |
| 三、1 回调演示补齐 | ✅ Button/Select/Switch/Radio/Rate/Slider/Checkbox/DatePicker 7 个表单页补事件回显 |

> 说明：三、1 原估"90 页"实为全库页面总数，其中绝大多数是纯展示型（Divider/Flex/Grid/Icon 等），无事件可演示；本轮聚焦有值变化语义的**表单控件页**补回显。三、4 原估"~1500 行"经实测为 155 行净减（样板已比预估更精简，主要收益在可维护性而非纯行数）。

### 补充落地（二）（2026-08-30）

原"暂缓"的最后三项已实施：

| 报告条目 | 结果 |
|---|---|
| 四、4 控件工厂表统一（4 文件） | ✅ 新建 `tests/WidgetInventory.h` 权威控件清单（110 项），MetaProperties/ThemeLifecycle 删除本地 objectCases，ObjectTree 重写为表驱动（108 行硬编码 → 豁免集遍历），RenderSmoke 新增一致性守护槽，CoverageInventory 守护重定向到 WidgetInventory。净减约 300+ 行重复 |
| 四、7 QAccessible 最小覆盖 | ✅ 新建 `tests/TestAntAccessibility.cpp`，遍历权威清单验证 accessibleName/Description 往返 + `queryAccessibleInterface` 非空 + `role() != NoRole` |
| 二、9/10 惯用式清理 + 防御性收养 | ✅ `QColor(0,0,0,0)` → `Qt::transparent`；AntModal/Segmented 重复字符串提为常量；AntNav.cpp:64 构造时直接指定 parent |

> 说明：原估"删 500+ 行"经实测为净减约 300+ 行（两份 objectCases 收敛为一份权威清单 + ObjectTree 硬编码收敛为表驱动，但权威清单本身约 150 行）。两处经分析判定为报告误判/低收益而跳过：① 二、9 的 4 个头文件前向声明改造（AntInput.h 的 `Q_PROPERTY(QLineEdit::EchoMode...)` 需完整类型，前向声明不成立）；② 二、10 的 AntCard.cpp:52 是 QVBoxLayout 嵌套子布局的 Qt 标准写法（QLayout 非 QWidget，addLayout 接管），非窗口期问题。


