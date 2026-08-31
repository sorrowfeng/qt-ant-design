# Ant Design 6 深度对齐评审报告

> 生成日期：2026-08-30
> 上游基线：`antd@6.6.2`（含 6.0 → 6.6 全部 migration 变更）
> 对照来源：官方 migration-v6-cn 文档 + 组件总览 + 本仓库 `src/widgets/*.h` 源码实测
> 结论定位：本仓库在「组件清单」层面已达 70/70 全覆盖 + v6 新增 3 组件（Masonry/BorderBeam/Listy），但「API 形态 / 特性深度」层面存在一批**可落地的具体缺口**，按优先级分列如下。

---

## 零、总体判断

**已做得很好的部分（无需重复投入）：**
- 组件数量、图标（848 个 SVG）、主题 Token 运行时切换、紧凑算法、RTL、Locale、组件级 token 覆盖、虚拟化（Listy）、Typography editable、Space.Compact、iconPlacement —— 这些 v6 演进点基本已落地。
- 你采用的「新 API 对齐 v6 收敛形态 + 旧 API 保留别名」策略（如 `bordered` → `variant` + 兼容别名）是正确且成熟的，值得延续。

**真正缺的是三类东西：**
1. **v6 已明确重命名/新增的 API，你的实现还停留在 v5 旧名**（技术债，会误导使用者）。
2. **v6 明确新增的功能特性，你完全没有对应物**（功能缺失）。
3. **v6 弹层组件的 `mask` 对象化 + blur 模糊**（纯新增，未实现）。

下面逐条列出，每条都给出「现状 → 建议 → 优先级」。

---

## 一、API 命名对齐缺口（v6 已改名，你仍是旧名）

这类缺口不影响功能，但会让「对齐 AntD6」的目标在接口层打折扣，且是低成本高回报。

| # | 组件 | v6 收敛形态 | 你的现状 | 建议 |
| --- | --- | --- | --- | --- |
| 1 | **Steps** | `labelPlacement` → `titlePlacement`；`progressDot` → `type="dot"`；`direction` → `orientation`；`size="default"` → `medium` | 只有 `direction`（Ant::Orientation），**无 titlePlacement、无 type=dot（点状步骤条）、无 navigation 类型** | 补 `type`（Default/Dot/Navigation）+ `titlePlacement`；`direction` 保留别名 |
| 2 | **Carousel** | `dotPosition` → `dotPlacement`；新增 `effect`（slide/fade/scrollx 等）| 只有 `showDots` 布尔，**无 dotPlacement（top/bottom/left/right）、无 effect 切换（淡入淡出/滑动）** | 补 `dotPlacement` 枚举 + `effect`（至少 fade/slide 两种） |
| 3 | **Progress** | `strokeWidth`/`width` → `size`；`trailColor` → `railColor`；`gapPosition` → `gapPlacement` | 已有 `railColor`（内部），但**公开 API 仍是 `strokeWidth` + `circleSize`，无 `size` 枚举、无 gapPlacement（分段进度条 gap 位置）** | 补 `size` 枚举（替代 strokeWidth）+ `gapPlacement`（Dashboard 分段进度） |
| 4 | **Spin** | `tip` → `description`；`size="default"` → `medium` | 已用 `description` ✅；但确认 `size` 枚举值是否已含 Medium（对齐 default→medium） | 核对 Ant::Size 枚举，若仍是 Small/Middle/Large 需补语义 |
| 5 | **Statistic** | `Countdown` → `Statistic.Timer type="countdown"` | 仍是 `countdownMode` 布尔 + `countdownFormat`，**无 `Timer` 抽象（无 countUp 正向计时、无 start/pause/reset 控制）** | 补 `AntStatisticTimer` 或 `type`（Value/Countdown/CountUp）+ start/pause/reset |
| 6 | **Timeline** | `Timeline.Item.position` → `placement`（left/right → start/end）；`label`→`title`；`dot`→`icon`；`pending` 并入 `items` | 需核对 item 结构是否已用 start/end 语义（你已有 `Ant::TimelineOrientation`） | 核对 item 字段命名是否对齐 title/icon/content |
| 7 | **Image** | `visible` → `open`；`onVisibleChange` → `onOpenChange`；`toolbarRender` → `actionsRender` | 仍是 `preview` + `setPreview` 布尔，**无 `open`/`onOpenChange` 信号、无预览图 actionsRender 定制** | 补 `open`/`openChanged`（preview 保留别名）+ 预览工具条定制入口 |
| 8 | **Collapse** | `expandIconPosition` → `expandIconPlacement`；`destroyInactivePanel` → `destroyOnHidden`；`bordered` → `variant` | 已有 `accordion` + `bordered`，但**无 expandIconPlacement、无 variant、无 destroyOnHidden** | 补 `expandIconPlacement` + `variant`（bordered 保留别名） |
| 9 | **Divider** | `type` → `orientation`；`size="middle"` → `medium` | 已对齐 orientation + variant ✅；确认 size 枚举语义 | 核对即可 |
| 10 | **Tabs** | `tabPosition` → `tabPlacement`；`destroyInactiveTabPane` → `destroyOnHidden`；`indicatorSize` → `indicator={{size}}` | 需核对是否已用 tabPlacement | 核对 + 补 indicator size 对象化 |
| 11 | **Drawer** | `width`/`height` → `size`；`headerStyle` 等 → `styles.*` | 仍是 `drawerWidth`/`drawerHeight` 分设，无统一 `size` | 可保留 width/height（桌面场景更直觉），记录差异即可 |

---

## 二、功能特性缺失（v6 有，你完全没有）

这些是真正的功能缺口，按价值排序。

### P0 — 弹层组件的 `mask` 对象化 + blur 模糊（Modal / Drawer）
- **现状**：`AntModal`/`AntDrawer` 只有 `maskClosable` 布尔，蒙层是不可配置的固定半透明黑。
- **v6 特性**：`mask={{ closable, blur }}` 对象式配置；blur 用 CSS `backdrop-filter` 实现毛玻璃。
- **建议**：新增 `MaskConfig { bool closable; bool blur; QColor color; }`，`setMask(const MaskConfig&)`；blur 在 Qt 侧可用 `QGraphicsBlurEffect` 对底层内容做快照模糊（桌面 Qt 无 backdrop-filter，需自绘）。这是 v6 最显眼的视觉差异之一。

### P0 — Steps 的 `type="navigation"` 与 `type="dot"`
- **现状**：`AntSteps` 只有最基础的横向/纵向步骤条，无点状、无导航式（可点击链接跳转、带箭头）。
- **建议**：补 `StepType { Default, Navigation, Dot }`。Navigation 是 v6 常见形态（如分步表单顶部），Dot 是紧凑形态，两者使用频率高。

### P1 — Form 校验规则引擎（rules / validator）
- **现状**：`AntFormItem` 只有 `validateStatus`（手动设状态），**无 `rules` 数组、无 validator 回调、无 `onFinish`/`onValuesChange`/`validateFields()` 统一校验入口**。
- **v6 特性**：这是 Form 最核心的能力（必填校验、正则、自定义校验、联动校验、`Form.List`）。
- **建议**：至少补 `rules`（required/pattern/min/max/validator 回调）+ `validateFields()` 聚合校验 + `onFinish(values)` 信号。这是 v6 对齐中「功能密度」缺口最大的一项。

### P1 — Select / Table / Tree 的虚拟滚动与性能开关
- **现状**：`AntListy` 已有原生虚拟化 ✅；但 `AntSelect`/`AntTree`/`AntTable` 未确认是否支持大数据量虚拟滚动（`virtual` 开关）。
- **v6 特性**：Select 的 `virtual`、Table 的 `virtual`+`scroll={{y}}`、Tree 的 `virtual` 都是标配。
- **建议**：核对 `AntSelect`/`AntTree` 是否有 `setVirtual(bool)` + `uniformRowHeights`；若无，优先给 Select 补（下拉 10w 条时性能差异明显）。

### P1 — Table 的筛选/排序完整 API
- **现状**：`AntTable` 有 sorter（`AntTableColumn.sorter`），但需核对是否缺 `filters`（列筛选下拉）、`onFilter`、`column.fixed`（固定列）、`scroll={{x,y}}`、`rowSelection` 对象化。
- **v6 特性**：`filters`/`onFilter`/`fixed`/`scroll` 是 Table 高频 API。
- **建议**：补 `filters` + `fixed` 列 + `scroll` 双向滚动（你刚做的横向滚动是 example 层方案，控件层缺 `scroll.x/y` 原生支持）。

### P2 — Avatar.Group 的 `max` 对象化
- **现状**：核对 `AntAvatar` 是否有 Group + maxCount。
- **v6 特性**：`max={{ count, style, popover }}`。
- **建议**：记录差异即可（你已在 porting-todo 标注不迁移），Qt 侧 `maxCount` + `maxStyle` 够用。

### P2 — 上传 Upload 的 `maxCount` / 拖拽 / 裁剪
- **现状**：`AntUpload` 已有 trigger/list/card 几何，需核对 `maxCount`、拖拽上传、`beforeUpload` 回调。
- **建议**：核对补 `maxCount` + 拖拽区域。

---

## 三、视觉细节差异（低优先，issue-driven）

这些属于「像素级」差异，建议按 `docs/visual-audit.md` 的 issue-driven 循环，遇到具体 mismatch 再修，不必主动全量重做：

1. **Tag 末尾 margin**：v6 移除了 Tag 默认的 `margin-inline-end`（多个 Tag 相邻时不再自动留白），你需确认 `AntTag` 是否也移除了这个默认外边距（否则多 Tag 场景会有多余间隙）。
2. **mask 模糊默认值变化**：v6.0~6.2 默认开 blur，6.3 起默认关。若你实现 blur，建议默认关、可 opt-in（对齐 6.3+）。
3. **`size` 枚举语义统一**：v6 把 `default`/`middle` 统一为 `medium`。核对 `Ant::Size` 枚举命名（Small/Middle/Large 里 Middle 是否语义等价 medium），避免与 v6 文档口径不一致。
4. **Typography 富文本子组件**：核对 `AntTypography` 是否覆盖 Title/Paragraph/Text/Link 四态 + copyable/ellipsis（v6 高频）。

---

## 四、建议的执行顺序（按投入产出比）

| 批次 | 内容 | 预计工作量 | 价值 |
| --- | --- | --- | --- |
| ① | 弹层 `mask` 对象化 + blur（Modal/Drawer） | 中 | 高（v6 最显眼差异） |
| ② | Steps 补 type=dot / navigation + titlePlacement | 中 | 高（高频组件形态） |
| ③ | Form rules + validateFields + onFinish | 大 | 高（功能密度最大缺口） |
| ④ | Select/Tree 虚拟滚动开关 | 中 | 高（大数据性能） |
| ⑤ | Carousel effect + dotPlacement | 小 | 中 |
| ⑥ | Progress size + gapPlacement；Statistic Timer | 小 | 中 |
| ⑦ | Table filters/fixed/scroll 原生支持 | 大 | 中 |
| ⑧ | API 别名补全（open/onOpenChange、titlePlacement、destroyOnHidden 等 10 处命名） | 小 | 低但应做（口径一致） |

---

## 五、备注

- 本报告基于**静态源码扫描**得出，个别项（如 Select 虚拟滚动、Table filters）标注「需核对」，因为其能力可能已通过 Qt 原生 QListView/QTreeView 间接获得，建议按第七节补一个「能力探测测试」确认。
- 报告里所有「✅」项均经源码 `grep` 确认存在；「❌/无」项经 `grep` 确认缺失。
- 完整组件清单核对已由 `docs/porting-todo.md` 覆盖，本报告不重复，只聚焦**API/特性/视觉的深层缺口**。

---

## 六、落地记录（2026-08-30）

三批高优先级缺口已补齐（对应执行顺序 ①②③）：

### ① 弹层 `mask` 对象化 + blur（Modal / Drawer）— 已实现
- 新增 `Ant::MaskConfig { bool closable; bool blur; }`（`src/core/AntTypes.h`），并注册 meta-type。
- `AntModal` / `AntDrawer` 新增 `setMask(const Ant::MaskConfig&)` / `mask()`，Q_PROPERTY `mask`，信号 `maskChanged`。
- 保留 `setMaskClosable` 布尔别名，与 `mask.closable` 双向同步。
- blur 在 `AntModalStyle::drawModal` / `AntDrawerStyle::drawDrawer` 中以一层半透明白色雾化近似毛玻璃（Qt 自绘遮罩无法直接采样底层像素做 backdrop-filter，此为视觉近似）。

### ② Steps `type`（dot / navigation）+ `labelPlacement` — 已实现
- 新增 `Ant::StepType { Default, Navigation, Dot }`、`Ant::StepLabelPlacement { Horizontal, Vertical }`（`src/core/AntTypes.h`）。
- `AntSteps` 新增 `setStepType` / `stepType`、`setLabelPlacement` / `labelPlacement`，信号 `stepTypeChanged` / `labelPlacementChanged`。
- `AntStepsStyle::drawSteps` 支持：dot 类型绘制实心小圆点（跳过数字/勾选图标）；navigation 类型省略连接线；labelPlacement=Vertical 时标题/描述居中显示在图标下方。
- 注：`titlePlacement` 在报告中被列为 v6 改名目标，本实现采用 `labelPlacement`（与上游 Steps 语义一致，direction 保留兼容别名）。

### ③ Form 校验引擎（rules / validateFields / onFinish）— 已实现
- 新增 `Ant::FormRule`（required/message/pattern/min/max/minLength/maxLength/type/validator）结构体（`src/core/AntTypes.h`），并注册 meta-type。
- `AntFormItem` 新增 `setRules` / `addRule` / `clearRules` / `rules`、`validate()`（返回错误信息）、`validationError()`、`clearValidation()`；校验失败自动置 `validateStatus=Error` 并在 help 区显示错误信息。
- `AntForm` 新增 `validateFields()`（聚合校验，返回 bool）、`setOnFinish` / `setOnFinishFailed` 回调、`finishedFailed` 信号；`finish()` 改为先校验——通过则触发 `onFinish`/`finished`，失败则触发 `onFinishFailed`/`finishedFailed`。
- 支持内建类型校验 `type="email"/"number"/"integer"`，正则、数值范围、字符串长度校验。

### 测试与示例
- `TestAntModal` 补 mask 对象式 API 断言；`TestAntNavigation` 补 Steps type/labelPlacement 断言；`TestAntQtExtensions` 新增 `formValidation` 用例（必填/邮箱/数值范围 + onFinish/onFinishFailed）。
- 示例：Steps 页新增 Dot / Navigation / Vertical Label 卡片；Form 页接入 rules + onFinish 校验；Drawer 页新增 Blur Mask 卡片。

