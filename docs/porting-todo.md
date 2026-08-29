# Ant Design 上游移植待办

> 生成日期：2026-08-29（dev 分支）
> 上游基线：`antd@6.6.2`、`@ant-design/icons@6.3.2`、`@ant-design/icons-svg@4.5.0`
> 当前仓库：`v0.1.2`，91 个公开组件（70/70 Ant Design 标准组件 + v6 新增 `AntBorderBeam`/`AntListy`），内置 `@ant-design/icons-svg@4.5.0` 共 848 个 SVG
> 参考来源：<https://github.com/ant-design/ant-design>（master）、<https://github.com/ant-design/ant-design-icons>、<https://ant.design/docs/react/migration-v6-cn>

## 一、新增组件（上游有、本仓库没有）

| 优先级 | 上游组件 | 引入版本 | 说明 | 状态 |
| --- | --- | --- | --- | --- |
| P0 | `BorderBeam` 边框光束 | antd 6.4.0 | 为容器边框提供流动光效装饰反馈。API：`color`、`count`、`duration`、`beamLength`、`lineWidth`、`borderRadius`、`running`、`activeOnHover`、内容托管 | ✅ 已移植为 `AntBorderBeam`（Pattern A，光束沿圆角边框路径运动 + 辉光） |
| P0 | `Listy` 高性能列表 | antd 6.6.0 | 虚拟滚动、分组头、拖拽排序、无限加载、滚动定位 | ✅ 已移植为 `AntListy`（QAbstractListModel + QListView 原生虚拟化；分组头行、吸顶分组 chip、InternalMove 拖拽、`loadMoreRequested`、`scrollToKey`/`scrollToRow`） |

上游组件目录核对结果：antd master 的组件目录中除 `border-beam`、`listy` 外，其余均已被本仓库覆盖（`qr-code`/`qrcode` 是 v6 目录更名，本仓库 `AntQRCode` 已覆盖；`back-top` 在 v6 已废弃并由 `FloatButton.BackTop` 承载，本仓库已覆盖；`row`/`col`/`grid`/`locale`/`theme`/`style`/`overview`/`_util` 为文档或工具目录，非独立组件）。

## 二、图标资源同步

- [x] 将内置 SVG 从 `@ant-design/icons-svg@4.4.2`（831 个）升级到 `@4.5.0`（848 个，Outlined 447 / Filled 251 / TwoTone 150）——已完成（2026-08-29，dev）
- [x] 新增 17 个 **Filled** 图标（全部为 AI 厂商与社交品牌图标，Outlined/TwoTone 数量不变）：
  `Anthropic`、`Claude`、`DeepSeek`、`ElevenLabs`、`Gemini`、`HuggingFace`、`Mastodon`、`Meta`、`Mistral`、`Netflix`、`Ollama`、`Perplexity`、`Qwen`、`Replicate`、`Snapchat`、`Telegram`、`Threads`——已同步 `resources/icons/antd/`、`resources/qt-ant-design.qrc`、`docs/ant-design-icons.md`
- [x] 在 `TestAntIcon` 中加入新图标名的加载验证——已加入 `builtinIconNames` 计数（848）与逐个渲染断言

## 三、v6 API 演进对齐（命名/结构约定）

v6 做了一批 API 统一（详见上游 migration-v6 文档）。本仓库是 Qt 移植，不照搬破坏式改名，新 API 对齐 v6 收敛形态、旧 API 保留别名：

- [x] `variant` 统一变体语义：`AntCard` / `AntDescriptions` 已提供 `variant`（Outlined/Borderless），`bordered` 保留为兼容别名——已完成（2026-08-29，dev）；其余控件的 `bordered` 混用盘点待后续
- [ ] `items` 驱动取代 children 组装：Breadcrumb/Anchor/Menu/Dropdown/Steps 在上游均为 `items` 数组驱动；本仓库保留现有 add 式 API 的同时，可提供 `setItems()` 批量入口
- [x] `iconPlacement`（start/end）取代 `iconPosition`：`Ant::IconPlacement` + `AntButton::iconPlacement` 已实现——已完成（2026-08-29，dev）
- [x] `Space.Compact` 取代 `Button.Group`：`AntSpace::compact` 已实现（零间距拼接）——已完成（2026-08-29，dev）
- [ ] `Avatar.Group` 的 `maxCount` 在上游收敛为 `max={{ count, style, popover }}` 对象式配置——本仓库 Qt API 保持 `maxCount`，记录差异不迁移
- [ ] `size="default"` 在上游改名为 `size="medium"`——本仓库使用 `Ant::Size` 枚举，无字符串尺寸名，无需迁移
- [ ] 弹层 API 统一：`popupMatchSelectWidth` / `popupRender` / `onOpenChange`——本仓库新增弹层组件时直接采用 popup 系命名

## 四、主题与定制能力

- [x] **组件级 token 覆盖**：`AntTheme::setComponentToken(component, token, value)` / `componentToken()` / `clearComponentTokens()` 已实现，修改触发 themeAboutToChange/themeChanged 生命周期；首个消费端为 `AntButton` 的 `Button.borderRadius` 覆盖——已完成（2026-08-29，dev）；其余样式类按需逐步接入
- [x] **紧凑算法（compactAlgorithm）**：`Ant::ThemeDensity::{Default, Compact}`，`AntTheme::applyConfiguration` 第五参密度、`AntConfigProvider::density`；compact 下 controlHeight 系 -4、padding/margin 系收缩到 75%——已完成（2026-08-29，dev）
- [x] **语义化结构定制（semantic DOM）**：Qt 侧落地形态即上述组件级 token 覆盖机制——已完成机制，按组件逐步接入消费端
- [ ] CSS variables 模式：上游 v6 默认启用 cssVar 实现运行时换肤；本仓库 `AntTheme` 已是运行时 token 切换，无需动作，仅作记录

## 五、其他差距

- [x] **RTL（从右向左）布局**：`AntConfigProvider::direction` 已实现，`apply()` 时写入 `QApplication::setLayoutDirection`——已完成（2026-08-29，dev）；各组件内部 RTL 细节适配待按视觉审计逐步复核
- [x] **国际化（Locale）**：`AntLocale` 单例（`core/AntLocale.h`）已实现，内置 English / ChineseSimplified 文案表，`languageChanged` 信号驱动；`AntModal` / `AntPopconfirm` 默认按钮文案已接入且未被用户覆盖时跟随切换——已完成首版（2026-08-29，dev）；更多语言包和 Pagination/Table 等消费端待后续扩充
- [ ] **虚拟滚动**：上游 Table/List/Select/Tree 均支持虚拟滚动；`AntListy` 已具备原生虚拟化，其余列表类组件待复核 `uniformRowHeights` 等性能开关并补齐文档说明
- [x] **Typography editable**：`AntTypography::editable` 已实现（双击进入行内编辑，Enter/失焦提交，Esc 取消，`edited` 信号）——已完成（2026-08-29，dev）
- [ ] **Ant Design X（AI 组件生态）**：上游独立包 `@ant-design/x`（Bubble、Conversations、Prompts、Sender、ThoughtChain 等 AI 对话组件）；不属于 antd 核心库，如项目要做 AI 桌面应用方向可另行评估

## 维护建议

- 每次发布前用上游 `components/` 目录清单与本仓库 `src/widgets/Ant*.h` 做一次 diff（可用 GitHub API 拉目录列表）
- 图标升级跟随 `@ant-design/icons-svg` 的 release，升级后必须同步 `docs/ant-design-icons.md` 并跑 `TestAntIcon`
- 上游 v6 的 Deprecated API 清单是本仓库 API 设计的"避坑指南"，新 API 命名直接采用 v6 收敛后的形态
