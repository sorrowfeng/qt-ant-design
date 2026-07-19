# 代码审计待优化列表

> 审计日期：2026-07-16
> P0/P1/P2 整改与复验日期：2026-07-16
> 审计基线：`dev` / `5d60094cb76f5581f031245100ce9dd044481388`
> 当前版本：`0.1.2`
> 文档用途：记录本轮代码审计确认的问题、后续整改状态、验证证据、待确认风险和结构性改进。

## 结论与使用规则

- 本轮未发现具备完整攻击路径的已验证安全漏洞；安全问题计数为严重 `0`、高 `0`、中 `0`、低 `0`。下文 P0/P1/P2 是工程整改优先级，不是漏洞严重度。
- P0 `5 / 5`、P1 `8 / 8`、P2 `5 / 5` 已于 2026-07-16 全部完成，并通过 Qt6 全量、Qt5 定向、静态/动态库安装消费者、父工程集成和 MSVC ASan 验证。
- “待确认安全风险”只有在宿主应用、部署权限或真实输入来源满足前置条件后，才可升级为漏洞。
- 优先修复已确认的崩溃、未定义行为、错误输出和构建门禁问题，再处理防御性加固与结构优化。
- 完成项目后，应同时勾选工作项、补充回归测试，并更新相关 README、AGENTS 或状态文档。

状态约定：

- `[ ]` 待处理
- `[~]` 处理中
- `[x]` 已完成并验证
- `[-]` 经确认不适用，并已记录理由

## 优先级汇总

| 优先级 | 数量 | 已完成 | 处理目标 |
| --- | ---: | ---: | --- |
| P0：立即处理 | 5 | 5 | 消除 UAF/越界、修复错误输出和失效门禁、澄清公开 API 契约 |
| P1：近期处理 | 8 | 8 | 修复数值边界、资源治理、构建集成和条件性安全风险 |
| P2：中长期优化 | 5 | 5 | 完善测试矩阵、文档一致性和模块可维护性 |

## P0：立即处理

### [x] AUD-001 统一修复外部 QObject 悬空指针

- 类型：已验证的正确性与稳定性问题
- 影响：高
- 置信度：高
- 目标：所有非 owning `QObject` / `QWidget` 引用使用 `QPointer` 或等价的销毁跟踪；对象销毁时同步取消 queued callback、事件过滤器、拖拽状态和缓存引用。

审计基线受影响位置（行号对应文首基线 SHA）：

- Dock 拖拽：`src/widgets/AntDockManager.h:211,216`，`src/widgets/AntDockManager.cpp:2643-2648,3198-3226,3315-3318,3691-3711,3718,3749-3754`
- Image PreviewGroup：`src/widgets/AntImage.h:87`，`src/widgets/AntImage.cpp:294-324`
- Tour target / overlay：`src/widgets/AntTour.h:11-17,39-40`，`src/widgets/AntTour.cpp:144-158,239-253,290-319,333-360,386-399`
- Affix 延迟检查：`src/widgets/AntAffix.h:57-63`，`src/widgets/AntAffix.cpp:15-18,47-77,172-204`
- FloatButton BackTop / children：`src/widgets/AntFloatButton.h:123,132`，`src/widgets/AntFloatButton.cpp:118-166,220-231,576-612`
- Anchor scroll area：`src/widgets/AntAnchor.h:51`，`src/widgets/AntAnchor.cpp:48-60`
- MenuBar hovered action：`src/widgets/AntMenuBar.h:38-40`，`src/widgets/AntMenuBar.cpp:34-126`
- FormProvider form registry：`src/widgets/AntForm.h:118-123`，`src/widgets/AntForm.cpp:24-57`

验收标准：

- 外部对象先于 controller 销毁后，setter、close、queued check、MouseMove、Escape 和 preview 操作均安全退出。
- Dock 销毁时清理拖拽标志、opacity effect、drop preview 和应用级事件过滤器。
- 容器中的已销毁对象不会被返回、遍历或传给 Qt API。
- 可用的 sanitizer 不报告 UAF、invalid vptr 或 double delete；缺失的 sanitizer 必须在限制中明确记录。

建议回归测试：

- `deleteLater()` + `QEvent::DeferredDelete` 后继续发送事件。
- 拖拽期间删除 Dock，再发送 Escape，并继续拖动第二个 Dock。
- 删除 PreviewGroup 成员后从存活成员打开预览。
- 删除 Tour target/host 后重复 `start()`、`next()`、`close()`。
- 删除 hovered QAction 后发送 move/leave。

整改结果：

- Dock 拖拽对象、opacity effect 与事件过滤器观察对象，Image PreviewGroup，Tour target/overlay，Affix target/viewport/widget，FloatButton child/BackTop target，Anchor scroll area，MenuBar hovered action 及 FormProvider registry 均改为 `QPointer` 或显式 `destroyed` 连接清理。
- queued Affix 检查及 target/viewport 销毁回调使用 generation 失效机制，信号槽内重绑的新监控不会再被旧清理覆盖；独立控件树坐标改为经 global coordinate 映射，兼容 Qt5 的祖先关系限制。
- `QAction::hovered` / `QMenuBar::hovered` 在 Qt 基类仍借用 action 指针期间不再向外重入，同时保留 disabled action 和 synthesized/touch move 的 Qt 原生信号语义；Dock 右键菜单延迟到当前事件分发结束后打开，避免回调同步删除整个事件目标树。
- `tests/TestAntStressLifecycle.cpp` 增加 controller/target 逆序销毁、Affix 销毁信号内重绑、Dock 拖拽和菜单回调删除、Preview 嵌套事件循环、MenuBar/Action 同步删除及 disabled/synthesized hover 等回归路径。

验证证据：

- Qt6 `TestAntStressLifecycle`：通过；Qt5 同目标：通过。
- 最新代码的 MSVC AddressSanitizer `TestAntStressLifecycle`：通过，未出现 `AddressSanitizer: ERROR`、`SUMMARY`、`DEADLYSIGNAL` 或 `CHECK failed`。
- 限制：当前 Windows/MSVC ASan 对少量运行库指令报告 interception failure 并使用 continue 模式，且本环境未提供 UBSan，因此不能据此宣称无任何 sanitizer 盲区。

### [x] AUD-002 修复 QR 非法枚举与容量边界

- 类型：已验证的越界读取、错误输出和资源放大问题
- 影响：高
- 置信度：高
- 审计基线位置（行号对应文首基线 SHA）：`src/widgets/AntQRCode.cpp:28-34,68-76,207-210`，`src/core/AntQRGenerator.cpp:38-118,526-571`
- 整改后主要实现：`src/core/AntQRGenerator.h:21-49`，`src/core/AntQRGenerator.cpp:560-675`，`src/widgets/AntQRCode.cpp:217-252`，`tests/TestAntQRGenerator.cpp:478-541`

工作项：

- 在索引 `capTable` 前验证 QR 版本和 `QRCodeErrorLevel`。
- 按 UTF-8 字节数、版本和纠错等级精确计算容量。
- 在展开 bit 数组前拒绝超容量输入，不能静默使用 V10 截断。
- 为失败提供明确状态、返回值或信号；失败时不得保留看似有效但不可解码的矩阵。
- 决定是明确只支持 V1-V10，还是完整实现 V11-V40。

验收标准：

- `static_cast<QRCodeErrorLevel>(-1/4)` 不产生越界读取。
- max 容量可正确解码，max+1 明确失败。
- 多字节 UTF-8 的容量按字节而不是字符数计算。
- 超大输入不会先分配与输入长度成 8 倍 `int` 数量的 bit 数组。

建议回归测试：非法 enum、各纠错等级 max/max+1、多字节文本、指定非法版本，以及使用独立 QR 解码器做 round-trip。

整改结果：

- `AntQRGenerator::tryGenerate()` 现在返回结构化 `GenerationResult` / `GenerationError`，在查表和 bit 展开前验证纠错枚举、版本与 UTF-8 字节容量；支持范围明确为 byte mode V1-V10，超限时返回失败而不再静默截断。
- 修正 timing function modules、两份 format information、V7+ version information、mask 评分以及 Reed-Solomon GF(256) 生成链路；`AntQRCode` 在失败时清空矩阵并公开 `encodingValid`、`encodingError` 与 `encodingFailed`。
- 新增 `tests/TestAntQRGenerator.cpp` 独立参考解码器，验证 finder/separator/timing/alignment/dark module、BCH format/version、8 种 mask、zigzag/反掩码、去交织和每个 RS block syndrome。
- 覆盖 V1-V10 × L/M/Q/H 共 40 组最大容量 round-trip、max+1 拒绝、UTF-8、非法枚举/版本和 4 MiB 输入早拒绝。

验证证据：Qt6 与 Qt5 的 `TestAntQRGenerator`、`TestAntDataDisplayB` 均通过；相关目标也通过 MSVC AddressSanitizer。

### [x] AUD-003 修复安装消费者测试的版本硬编码

- 类型：已验证的构建与发布门禁问题
- 影响：高
- 置信度：高
- 审计基线位置（行号对应文首基线 SHA）：`VERSION:1`，`tests/TestAntInstallConsumer.cmake:102-136`，`tests/CMakeLists.txt:270-283`
- 整改后主要实现：`tests/CMakeLists.txt:275-283`，`tests/TestAntInstallConsumer.cmake:16-24,52-59,118-152,187-258`

问题：仓库版本为 `0.1.2`，安装消费者测试仍断言 patch 为 `0` 并比较字符串 `0.1.0`；当前测试进入 consumer 编译阶段时必然失败。

工作项：

- 从根 `VERSION` 或生成的 `QtAntDesignVersion.h` 自动获取测试预期值。
- 删除测试中的第二份手工版本源。
- 在 CI 中执行安装、consumer configure/build/run，并检查导出包版本。

验收标准：修改 `VERSION` 后无需修改测试源码；安装消费者测试能自动验证相同版本并通过。

整改结果：

- `tests/CMakeLists.txt:281` 将 CMake `PROJECT_VERSION` 作为唯一预期版本传入安装消费者测试。
- 生成的 consumer 使用 `find_package(qt-ant-design <version> EXACT CONFIG REQUIRED)`，同时验证 package version、编译期 major/minor/patch、版本字符串和运行时 `versionString()`。
- 测试现在执行 install、独立 configure、build 和 consumer executable；Windows 运行阶段显式使用同一 Qt 的 `bin/plugins` 与 `minimal` 平台插件，避免误加载其他 Qt 运行时。

验证证据：`ctest --test-dir build -C Debug -R "^TestAntInstallConsumer$" --output-on-failure` 通过，并在最终 155 项全量中再次通过；Release shared 配置的独立安装消费者也通过。

### [x] AUD-004 明确并兑现 controller 公开 API 契约

- 类型：已验证的 API/文档契约不一致
- 影响：中
- 置信度：高

审计基线受影响位置（行号对应文首基线 SHA）：

- `AntApp::showMessage/showModal/showNotification` 无实际行为：`src/widgets/AntApp.cpp:75-100`
- ConfigProvider 仅应用 themeMode：`src/widgets/AntConfigProvider.cpp:21-56`
- FormProvider 未转发已声明信号：`src/widgets/AntForm.cpp:24-57`，`src/widgets/AntForm.h:102-169`
- 对应文档：`README.md:391-392`，`AGENTS.md:147`

决策项：

- 完整实现这些 API，并增加端到端行为测试；或
- 将其标记为实验性/未支持，返回明确失败，并修正文档；或
- 在下一个允许破坏兼容性的版本移除无效 API。

验收标准：公开调用不再无声成功；测试验证最终反馈 UI、主题 token 变化和表单事件，而不只验证属性或信号计数。

整改结果：

- `AntApp::showMessage/showModal/showNotification` 会创建真实反馈对象，公开 `last*`、shown/failed 信号并对不可呈现 host 返回明确失败；Modal 回调和关闭后的释放行为已覆盖。
- `AntTheme::applyConfiguration()` 原子应用 mode、primary color、font size 与 border radius，并提供通用 `themeAboutToChange/themeChanged`；依赖 token 的 Style/Widget 已迁移到通用主题信号。
- `AntConfigProvider::apply()` 发布完整配置；字体和圆角参数设有可计算上限，避免极值参与 token 运算时溢出。
- `AntFormItem`、`AntForm` 与 `AntFormProvider` 实现命名字段取值、变化通知、finish values、provider 转发、重名注册和对象销毁清理；`AntSelect::currentValue` 补为可写元属性。
- README、AGENTS、项目状态、可靠性矩阵和示例页已同步实际契约。

验证证据：`TestAntQtExtensions`、`TestAntMetaProperties`、`TestAntSelect`、`TestAntThemeLifecycle` 与 `TestAntObjectTree` 在 Qt6 通过，Qt5 定向矩阵覆盖对应目标。

### [x] AUD-005 为 Dock perspective 反序列化设置全局预算

- 类型：待确认安全风险，同时是必要的健壮性改进
- 条件性严重度：中
- 置信度：代码路径高，宿主可控性待确认
- CWE：CWE-674
- 审计基线位置（行号对应文首基线 SHA）：`src/widgets/AntDockManager.cpp:2153-2171,2422-2429`，`src/private/AntDockLayoutSerializer.cpp:40-146,213-261`
- 整改后主要实现：`src/widgets/AntDockManager.cpp:2430-2450`，`src/private/AntDockLayoutSerializer.h:15-20`，`src/private/AntDockLayoutSerializer.cpp:31-66,92-225,322-350`，`tests/TestAntDockPerspectiveLimits.cpp:150-247`

工作项：

- 限制状态总字节数、递归深度、总节点数、总 Dock ID 数和单个字符串长度。
- 优先改为迭代解析；若保留递归，必须显式传递并检查 depth/budget。
- 解析失败时不得改变当前布局或已保存 perspective。
- 调查宿主是否从文件、IPC、云同步或低权限可写位置加载该状态。

验收标准：depth=limit 成功、limit+1 失败；极宽树、截断流、巨大字符串和非法 count 均快速失败且布局保持不变。

整改结果：

- 当前格式增加 1 MiB state、64 层深度、4096 节点、4096 Dock ID、1024 UTF-16 字符/标识符和 1024 浮窗快照的全局预算。
- 字符串在 `QDataStream` 分配前读取并检查长度；递归解析显式传递 depth/node/ID budget，并拒绝非法 count、截断流和 trailing bytes。
- `setPerspectiveState()` 在写入 perspective map 前完成解析，失败时保持已保存 state、运行时布局、Dock area 与 tab 状态不变。
- 新增 `tests/TestAntDockPerspectiveLimits.cpp` 覆盖 limit/limit+1、极宽树、超限 ID/字符串/浮窗、超大/截断/尾随状态和原子拒绝。

验证证据：Qt6 与 Qt5 的 `TestAntDockPerspectiveLimits` 均通过；该目标也通过 MSVC AddressSanitizer。Legacy 签名快照仍可导入和存储，供识别或迁移使用；当前版本按 AUD-011 的最终决策明确拒绝恢复，不再报告虚假成功。

## P1：近期处理

### [x] AUD-006 收紧导出的 AntWindowFrame 阴影 API

- 类型：已验证的公共 API 内存安全问题
- 影响：中
- 置信度：高
- 审计基线位置（行号对应文首基线 SHA）：`src/widgets/AntWindowFrame.h:32-51`，`src/widgets/AntWindowFrame.cpp:278-323,386,541-550`，`src/CMakeLists.txt:48-54`
- 整改后主要实现：`src/widgets/AntWindowFrame.h:37-87,121-126`，`src/widgets/AntWindowFrame.cpp:436-441,693-839`，`tests/TestAntQtExtensions.cpp:5658-5805`，`tests/TestAntInstallConsumer.cmake:123-149`

工作项：

- 不再跨事件保存调用方借用的 `const char*`；复制为 `QByteArray`。
- 不对任意非空 `QWidget*` 执行无检查 `static_cast<LegacySoftwareShadow*>`。
- 将内部阴影 handle 改为私有强类型或不可由调用者伪造的 opaque handle。
- 评估该头文件是否应继续作为安装的公开 API。

验收标准：临时 property 名释放后，show/native hit-test 仍安全；传入错误对象时明确拒绝而不是产生 UB。

整改结果：

- `AntWindow` 与 `AntDialog` 已迁移到不可复制、不可移动的 `LegacySoftwareShadowHandle`；句柄保持一个指针大小和 trivial destructor，以兼容原私有 `QWidget*` 槽位的对象布局/析构 ABI。内部地址只能由库写入，所有读取均先查询私有 live-shadow 注册表，因此调用方无法把任意或已失效 `QWidget*` 伪造成有效句柄。
- 阴影对象将 click-through property 名复制为 `QByteArray`，后续 `showEvent()` 与 `nativeEvent()` 不再访问调用方借用的 `const char*`。
- 原 `QWidget*&` 导出签名作为源代码/二进制兼容 wrapper 保留；新增 `tryUpdateLegacySoftwareShadow()` 返回 `InvalidShadowWidget` / `OwnerMismatch` 等明确结果。兼容入口先查询私有阴影注册表并校验 owner，不再对任意或已失效地址做 `static_cast`、RTTI 解引用或副作用写入。
- `AntWindowFrame.h` 继续作为安装的低层窗口集成 API：`AntWindow.h` / `AntDialog.h` 的安全句柄状态直接依赖该定义，且安装消费者现在会编译、链接并实例化该句柄。内部阴影具体类型仍只存在于 `.cpp`。

回归证据：Qt6 `TestAntQtExtensions` 覆盖 opaque handle 的 ABI 断言、阴影外部销毁后的安全重建、错误/失效对象和 typed/compat owner mismatch 无副作用拒绝、临时 property 缓冲区释放后的再次 show，以及真实 `WM_NCHITTEST -> HTTRANSPARENT`；默认静态配置的 `TestAntInstallConsumer` 验证安装头和新增链接符号，二者均通过。

剩余限制：当前通过 `sizeof` / `alignof` / trivial-destructor 约束保持原私有指针槽位布局，当前 shared DLL 的安装头、导出符号和 Release consumer 已动态验证；但尚未执行“用整改前 0.1.2 头编译 consumer，再仅替换为当前 shared DLL”的真实跨版本二进制兼容实验，因此不把当前验证扩大表述为覆盖全部历史 ABI 场景。`LegacySoftwareShadowHandle::widget()` 仅用于诊断或显式删除，reparent 和修改 native window flags 不属于支持契约。

### [x] AUD-007 修复 Slider、Progress、Pagination 整数溢出

- 类型：已验证的数值边界问题
- 影响：中
- 置信度：高
- 审计基线位置（行号对应文首基线 SHA）：`src/widgets/AntSlider.cpp:678-885`，`src/widgets/AntProgress.cpp:505-522`，`src/widgets/AntPagination.cpp:335-361`
- 整改后主要实现：`src/widgets/AntSlider.cpp:22-26,512-549,685-897`，`src/widgets/AntProgress.cpp:512-529`，`src/widgets/AntPagination.cpp:21-25,200-203,270-380`；回归测试位于 `tests/TestAntDataEntryA.cpp:389-454`、`tests/TestAntFeedback.cpp:978-1025`、`tests/TestAntNavigation.cpp:327-375`

工作项：范围差值、比例、步进和页码加法先提升到 `qint64` 或 `double`，最终结果再夹取到公开 `int` 范围。

验收标准：`INT_MIN..INT_MAX`、跨零范围、大 singleStep、`total=INT_MAX/pageSize=1/current=INT_MAX` 均不触发 UBSan，且 UI 状态可预测。

整改结果：

- `AntSlider` 的范围差值、像素映射、步进和 range handle 运算先提升为 `qint64` / `double`，再按公开范围夹取；`AntProgress` 的百分比和弧度计算不再在 `int` 中先溢出；`AntPagination` 的总页数和页码移动使用提升后的中间值。
- 回归测试覆盖 `INT_MIN..INT_MAX`、跨零范围、极大 step、极值 percent，以及 `total=INT_MAX/pageSize=1/current=INT_MAX` 的显示和交互结果。

验证证据：相关 `TestAntDataEntryA`、`TestAntFeedback`、`TestAntNavigation` 在 Qt6 全量中通过，关键目标也通过 MSVC ASan。当前环境没有本地 UBSan，因此“无有符号整数 UB”的结论来自提升后的代码路径与边界断言，而不是本地 UBSan 动态证据。

### [x] AUD-008 建立图片解码和 Upload 缓存预算

- 类型：已验证的性能问题；低信任图片场景下为待确认 CWE-400 风险
- 影响：中
- 审计基线位置（行号对应文首基线 SHA）：`src/widgets/AntImage.cpp:175-183`，`src/widgets/AntUpload.cpp:196-245,767-784`，`src/widgets/AntUpload.h:155`
- 整改后主要实现：`src/private/AntImageDecodeUtils.h:17-24,112-270`，`src/widgets/AntImage.cpp:539-565`，`src/widgets/AntUpload.cpp:860-1084`，`src/widgets/AntUpload.h:74-81,160-205`；回归测试位于 `tests/TestAntDataDisplayA.cpp:560-705`、`tests/TestAntDataEntryB.cpp:810-1116`

工作项：

- 使用 `QImageReader` 在解码前读取尺寸、格式和预计内存。
- 设置文件大小、像素数、解码后字节数和目标缩略图尺寸上限。
- 缩略图缓存改为受字节数限制的 LRU。
- 删除文件、替换列表、源文件变化时驱逐对应缓存。
- 评估是否需要后台解码和取消机制。

验收标准：反复添加/显示/删除不同大图时，缓存内存保持在预算内；超限图片快速失败且 GUI 不长时间阻塞。

整改结果：

- `AntImageDecodeUtils` 最多读取 `32 MiB + 1` 编码字节到不可变 `QByteArray`，再通过只读 `QBuffer` / `QImageReader` 完成 metadata 检查与解码；当前解码不再在检查后重新打开路径，关闭了文件内容检查与解码之间的 TOCTOU。
- 解码前同时限制编码大小、宽高、像素数和最坏 `16 bytes/pixel` 的 `256 MiB` 输出预算；`4096 x 4096` 恰好位于预算边界，并以真实 RGBA64 图片验证高位深结果可超过 `4 bytes/pixel`、但仍被最坏估算覆盖。
- Upload 缩略图缓存改为受总字节数限制的 LRU；命中会更新 recency，列表替换、文件 stamp 变化和显式 `invalidateThumbnail()` / `clearThumbnailCache()` 会驱逐缓存。失败回调使用完整 stamp 区分旧请求，A 到 B 的同事件循环切换只报告最新失败一次。
- 组件增加可观察的加载状态、错误与重载路径；超限输入明确失败。当前仍在 GUI 调用链内完成有界解码，没有引入后台线程和取消协议；本轮保证资源上界与超限早拒绝，不保证预算内复杂图片绝无可见 GUI stall。

验证证据：`TestAntDataEntryB` 与 `TestAntDataDisplayA` 覆盖静态编码预算、尺寸/字节预算、4096 轮预算属性、RGBA64、加载失败恢复、A/B 竞态以及严格的 A、B、touch A、C LRU 淘汰序列；Qt6、Qt5 定向和 MSVC ASan 均通过。

### [x] AUD-009 修复 CMake 子项目集成默认值和路径

- 类型：已验证的构建集成问题
- 影响：中
- 审计基线位置（行号对应文首基线 SHA）：`CMakeLists.txt:49-74`，`tests/CMakeLists.txt:3-40,270-296`，`README.md:106-126`
- 整改后主要实现：`CMakeLists.txt:18-24,80-129`，`tests/TestAntBuildSystem.cmake:31-60,82-168,180-269`

工作项：

- 使用 `PROJECT_IS_TOP_LEVEL` 控制示例和测试默认值。
- 使用项目命名选项或标准 `BUILD_TESTING`，避免污染宿主选项。
- 将 `CMAKE_SOURCE_DIR` 改为 `PROJECT_SOURCE_DIR` / `CMAKE_CURRENT_SOURCE_DIR`。
- 增加真实 parent project 调用 `add_subdirectory()` 的 configure/build 测试。

验收标准：默认作为子项目时只构建库，不添加示例、测试或安装示例；显式开启后所有路径仍指向本仓库。

整改结果：

- 示例、测试、smoke 和安装部署改为 `QT_ANT_DESIGN_*` 命名选项；默认值由 `PROJECT_IS_TOP_LEVEL` 决定，子项目默认只生成库。测试脚本不再依赖 `CMAKE_SOURCE_DIR`。
- `TestAntBuildSystem` 生成真实 parent project：先验证默认 `add_subdirectory()` 只构建库和父 consumer，再在同一构建树显式开启子项目测试并构建 `TestAntTypes`，最后从父构建根验证 CTest 发现。
- 若宿主显式启用子项目测试并希望从宿主构建根通过 CTest 发现它们，宿主必须在 `add_subdirectory()` 前调用 `enable_testing()`；否则测试目标仍可构建，但顶层 `ctest -N` 不保证遍历子目录。

验证证据：最终 Qt6 全量中的 `TestAntBuildSystem` 通过，耗时 `247.04s`；其中 parent consumer 完成 configure/build，显式子项目测试也被父级 CTest 发现。

### [x] AUD-010 在构建和安装包中强制最低 Qt 版本

- 类型：已验证的构建契约问题
- 影响：中
- 审计基线位置（行号对应文首基线 SHA）：`README.md:100-104`，`CMakeLists.txt:33-34`，`cmake/qt-ant-designConfig.cmake.in:5-10`
- 整改后主要实现：`CMakeLists.txt:42-65`，`cmake/qt-ant-designConfig.cmake.in:1-13`，`tests/TestAntBuildSystem.cmake:103-130`

工作项：按 Qt major 强制 Qt 6.5+ 或 Qt 5.15.2，并在安装包配置中执行同等检查。

验收标准：不支持的 Qt 在 configure 阶段以清晰错误失败；支持版本的源码构建和 `find_package()` consumer 均通过。

整改结果：源码配置和安装包配置按 Qt major 统一强制 Qt `6.5.0+` 或 Qt `5.15.2+`，并在错误信息中报告检测版本与最低版本。Qt5 5.15.2、Qt6 6.9.1、静态/动态库和安装后 `find_package()` consumer 均完成验证。

### [x] AUD-011 修复 legacy Dock restore 的虚假成功

- 类型：已验证的正确性问题
- 影响：低至中
- 审计基线位置（行号对应文首基线 SHA）：`src/private/AntDockLayoutSerializer.cpp:264-267`，`src/widgets/AntDockManager.cpp:2159-2163`
- 整改后主要实现：`src/private/AntDockPerspectiveRestorer.cpp:8-48`，`src/private/AntDockLayoutSerializer.cpp:92-255,352-380`，`src/widgets/AntDockManager.cpp:2163-2215`，`tests/TestAntDockPerspectiveLimits.cpp:205-480`

工作项：实现旧格式迁移，或返回明确的 unsupported/failure；未恢复布局时不得发出 `perspectiveRestored`。

验收标准：返回成功后，恢复后的 Dock 树、当前 tab、splitter sizes 和浮窗状态与快照一致。

整改结果：

- 本轮选择“明确不支持恢复”，而不是伪造迁移结果：legacy 签名快照可导入并存储，供识别或后续迁移，但 `restorePerspective()` 返回 `false`，发出 `perspectiveRestoreFailed(name, "unsupported-legacy-format")`，不改变当前布局，也不发出 `perspectiveRestored`。
- 当前格式解析进一步验证 Empty/Area/Splitter 节点的字段组合、child/sizes 数量、非负 size、非空且唯一 Dock ID、currentIndex 和 runtime `objectName` 唯一性；所有检查在布局变更前完成。
- 节点和 ID 的 limit+1 测试分别构造结构合法、仅超出目标全局预算的数据，避免因更早的局部限制而产生假覆盖。

验证证据：`TestAntDockPerspectiveLimits` 在 Qt6、Qt5 和 MSVC ASan 下通过；legacy 失败语义、当前格式结构校验、重复 runtime ID 和状态不变均有回归测试。

### [x] AUD-012 限定 DLL 和 URL 外部处理边界

- 类型：待确认安全风险加固
- 条件性严重度：DLL 高、URL 低

审计基线位置（行号对应文首基线 SHA）：

- DLL：`src/widgets/AntWindowFrame.cpp:70-90`，`src/widgets/AntWindow.cpp:553-573`，`src/widgets/AntDockWidget.cpp:523-543`
- URL：`src/widgets/AntTypography.cpp:307-317,548-556`，`src/widgets/AntUpload.cpp:756-765`

整改后主要实现：`src/private/AntWindowsSystemLibrary.cpp:55-99`，`src/core/AntUrlPolicy.cpp:45-97`，`src/widgets/AntTypography.cpp:528-588`，`src/widgets/AntUpload.cpp:825-858`；回归测试位于 `tests/TestAntWindowsSystemLibrary.cpp:12-31`、`tests/TestAntTypography.cpp:328-426`、`tests/TestAntDataEntryB.cpp:1035-1116`

工作项：

- 使用 `LoadLibraryExW(..., LOAD_LIBRARY_SEARCH_SYSTEM32)` 或静态系统导入。
- 默认仅允许 `http/https` 外部链接；本地文件预览使用独立 API。
- 允许宿主提供 scheme allowlist 或审批回调。

验收标准：实际加载的 `dwmapi.dll` 路径来自 System32；不允许的 scheme 不会触发系统 handler。

整改结果：

- Windows 系统 DLL 统一经私有 helper 从 System32 加载，并验证模块最终路径；`TestAntWindowsSystemLibrary` 覆盖允许的裸系统模块名，并拒绝任何带路径的输入（相对或绝对路径都不接受）。
- 新增 `AntUrlPolicy`，默认只允许 `http` / `https`，宿主可配置 scheme allowlist 或同步审批回调；Typography 外部链接和 Upload 外部预览统一使用该策略，本地文件预览通过独立信号交给宿主。
- 所有外部信号、审批回调和剪贴板写入后的路径均以 `QPointer` 保护同步删除场景；Typography 与 Upload 不会在回调删除 sender 后继续访问对象。

验证证据：`TestAntWindowsSystemLibrary`、`TestAntTypography`、`TestAntDataEntryB` 在 Qt6 全量通过；后三类同步删除、阻止 scheme 和 A/B 回调竞态有定向回归。未发现可达的任意 DLL 或 scheme 打开路径。

### [x] AUD-013 加固 CI 供应链和失败语义

- 类型：待确认供应链风险和构建可靠性改进
- 影响：中
- 审计基线位置（行号对应文首基线 SHA）：`.github/workflows/ci.yml:18-82`，`CMakeLists.txt:61-68,106-123`
- 整改后主要实现：`.github/workflows/ci.yml:18-209`，`.github/dependabot.yml:1-7`，`tests/TestAntCiPolicy.cmake:1-52`，`tests/CMakeLists.txt:314-318`

工作项：

- GitHub Actions 固定完整 commit SHA，并用 Dependabot/Renovate 管理更新。
- 设置 `permissions: contents: read`；checkout 设置 `persist-credentials: false`。
- 显式启用测试但缺少 Qt Test 时失败，而不是静默跳过。
- 发布/打包场景中 `windeployqt` 失败应导致作业失败。
- 增加安装后运行示例或最小 consumer 的验证。

整改结果：

- GitHub Actions 固定到完整 commit SHA，权限收紧为 `contents: read`，checkout 禁止持久化凭据，并增加 Dependabot action 更新配置。
- 显式启用测试时 Qt Test 缺失会在 configure 阶段失败；Windows 部署、安装示例 smoke 和独立安装 consumer 的失败都会传播为作业失败。
- 当前矩阵覆盖 Windows Qt5/Qt6 static/shared、Linux Qt5 shared 与 Qt6 ASan+UBSan、macOS Qt6 shared。所有非 sanitizer 配置执行安装；sanitizer 配置只构建和测试，刻意跳过安装。
- `TestAntCiPolicy` 对 action pinning、权限以及选定的矩阵、sanitizer 和安装策略标记进行静态门禁。

验证证据：`TestAntCiPolicy` 包含在 155 项全量结果中并通过；本轮未触发远程 GitHub Actions，因此仅确认工作流定义与本地门禁，不宣称远程矩阵已通过。

## P2：中长期优化

### [x] AUD-014 增加 sanitizer 和异常输入测试矩阵

- 类型：必要的测试体系改进

建议矩阵：

- Windows/Linux/macOS（至少库级别跨平台构建）
- Qt5/Qt6
- Debug/Release
- static/shared
- ASan/UBSan；Windows 可增加 MSVC AddressSanitizer 配置

重点场景：外部对象逆序销毁、非法 enum、数值极值、Dock malformed 状态、图片/缓存预算和长时间生命周期压力。

整改结果：

- CI 已配置 Windows Qt5/Qt6、Linux Qt5/Qt6、macOS Qt6，涵盖 Debug/Release、static/shared，并在 Linux Qt6 Debug 配置 ASan+UBSan；本地 MSVC 配置支持可选 ASan，明确拒绝不受支持的 MSVC UBSan。
- sanitizer 编译插桩只应用于 `qt-ant-design` 库编译单元；必要链接选项传播给 build-tree consumer，但测试和宿主目标不会自动获得编译插桩。MSVC 仅对库移除 `/RTC`，并在库内禁用 STL string/vector annotations 以兼容未插桩宿主，项目源代码仍由 ASan 插桩。
- 本地 MSVC ASan 的 9 个关键目标通过：`TestAntTypography`、`TestAntDataEntryA`、`TestAntDataEntryB`、`TestAntDataDisplayA`、`TestAntQRGenerator`、`TestAntFeedback`、`TestAntNavigation`、`TestAntQtExtensions`、`TestAntDockPerspectiveLimits`，总耗时 `31.09s`。
- 真实 `add_subdirectory()` 父工程在开启 ASan 后成功构建并运行 `AntParentConsumer`。生成工程确认宿主保留 Debug `/RTC` 且无 `/fsanitize`，库 target 使用 `/fsanitize=address`、移除 `/RTC` 并只在库内设置兼容宏。

剩余限制：Windows/MSVC 运行时仍会报告少量 interception failure，本地使用 continue 模式完成测试，因此存在已知检测盲区；本地未运行 UBSan，已配置的 Linux ASan+UBSan 和其他远程矩阵也尚未在本轮触发。

### [x] AUD-015 为 Dock、QR 和图片入口增加 fuzz/property 测试

- 类型：可维护性与防回归改进

工作项：

- Dock：任意字节串、截断、深树、宽树、重复 ID、超限字符串。
- QR：任意 UTF-8、版本、纠错等级和容量边界，验证编码失败或 round-trip。
- 图片：仅 fuzz 元数据/预算决策层，实际解码器漏洞由 Qt 依赖治理覆盖。

整改结果：

- QR 使用固定种子执行 512 轮随机输入 property 测试：成功结果由独立解码器 round-trip，非法 level/version、空输入和超容量结果验证明确失败语义；另保留 40 组精确最大容量、max+1 和 UTF-8 边界。
- Dock 对 512 组任意字节串执行解析/原子状态属性测试，并单独覆盖深度与标识符的 limit/limit+1、节点/Dock ID/浮窗/总状态超限、宽树、重复 ID 和结构不变量。
- 图片预算决策层执行 4096 轮尺寸、像素数和最坏解码输出预算属性测试；编码大小使用 `32 MiB + 1` 边界样本，实际 decoder 仅用受控结构样本验证。没有把 Qt 图片插件当成本项目自有 fuzz 目标。

验证证据：上述 property 测试均包含在 Qt6 155 项全量中并通过。未运行 libFuzzer 或持续 fuzz campaign，因此不把固定种子 property 回归表述为完整模糊测试覆盖。

### [x] AUD-016 为公开 controller 增加端到端测试

- 类型：测试缺口

工作项：

- AntApp：真实创建并关闭 Message、Modal、Notification。
- ConfigProvider：验证 token 和现有控件视觉/度量发生预期变化。
- FormProvider：验证字段变化、完成事件、form 删除和重名行为。

整改结果：上述场景已并入 `TestAntQtExtensions`、`TestAntThemeLifecycle` 和 `TestAntMetaProperties`；除正常成功路径外，还覆盖隐藏 host 的明确失败、反馈对象关闭释放、token-only 更新、字段元属性变化、重复注册和已注册 form 先销毁后的清理。当前没有把 Tour controller 的逆序销毁测试误记为 FormProvider 覆盖。

### [x] AUD-017 同步并精简状态文档

- 类型：已验证的文档一致性问题

工作项：

- 修正 `README.md:391-392` 和 `AGENTS.md:147` 对未实现 controller 能力的描述。
- 统一 Win10 当前为 opaque square path：旧描述位于 `AGENTS.md:110`、`README.md:78`、`docs/project-status.md:156`；当前规范位于 `AGENTS.md:319-323`、`docs/project-status.md:197-204`。
- 将 `docs/visual-audit.md:46` 的 QProxyStyle 析构崩溃标为历史问题或补充当前可复现条件。
- 明确手工构建支持 CMake 3.16，而 presets 需要 CMake 3.21。
- 区分“历史验证结果”和“当前 CI 持续执行的测试”。

整改结果：

- `README.md`、`README.zh-CN.md`、`AGENTS.md`、`docs/project-status.md` 和 `docs/reliability-coverage.md` 统一记录源码/安装包最低 Qt 版本（Qt 6.5.0 或 Qt 5.15.2）、项目命名 CMake 选项及子项目默认只构建库的契约，并明确手工 CMake 3.16+ 与 presets CMake 3.21+ 的边界。
- Windows 10 顶层窗口统一描述为 no-caption、opaque、square-corner legacy path + 软件阴影宿主；`docs/project-status.md` 将 2026-05-07 rounded-mask 实现标为历史阶段，并指向 2026-05-20 已取代它的当前策略。
- `docs/visual-audit.md` 将 QProxyStyle 析构崩溃改为无当前稳定复现条件的历史记录；若再次出现，需要保留调用栈并按缺陷处理，不再建议把强制退出当作正常流程。
- 按 `tests/CMakeLists.txt` 当前注册逻辑，Windows 顶层清单为 `51` 个深度/系统条目，开启 `QT_ANT_DESIGN_BUILD_WIDGET_SMOKE_TESTS` 后为 `155` 项；本地 `ctest --test-dir build -C Debug -N` 报告 `Total Tests: 155`，随后当前清单已完成 `155 / 155` 全量复验。
- 文档同步记录当前工作流定义的 Windows/Linux/macOS、Qt5/Qt6、Debug/Release、static/shared 和 Linux ASan/UBSan 矩阵，并明确“已配置”不等于新远程 GitHub Actions 已通过。
- legacy snapshot、宿主 `enable_testing()`、sanitizer 插桩范围以及 sanitizer job 跳过安装的口径已在 README、AGENTS、项目状态与可靠性矩阵中统一。

文档验证：本轮涉及的 7 个 Markdown 文件的相对链接检查通过，未发现需要解析的仓库内 fragment anchor；旧 Win10 rounded-mask 当前态表述和旧 `BUILD_WIDGET_SMOKE_TESTS` 推荐用法已清理；`git diff --check` 通过。未触发远程 CI，因此 CI 结果仍以之后的 GitHub Actions 运行记录为准。

### [x] AUD-018 降低复杂组件的维护成本

- 类型：可选结构优化，不是当前缺陷

建议：

- 将 `AntDockManager.cpp` 的拖拽状态机、perspective、浮窗管理和布局构建拆为私有协作类。
- 评估把 `src/CMakeLists.txt` 的递归 glob 改为显式源文件清单，提高构建输入可预测性。
- 清理 `src/widgets/AntColorPicker.cpp:411-478` 中未实例化的 `ColorGrid` 死代码。
- 为 GUI 线程专用 API（例如日志与 UI controller）明确线程契约，必要时提供 queued 入口。

整改结果：

- `src/CMakeLists.txt` 改为 369 个显式构建输入（198 个头文件、171 个源文件），并由 `TestAntBuildSystem` 执行 inventory gate；新增/删除源码未同步清单时该门禁测试失败。未实例化的 `ColorGrid` 已删除。
- Dock perspective 的解析、迁移决策和运行时恢复前校验拆到私有 `AntDockPerspectiveRestorer`，缩小 `AntDockManager.cpp` 的职责面；拖拽与浮窗管理仍保留在 manager，避免本轮引入更大行为重构。
- `AntLog` 增加线程安全的有界 PendingCommand 队列：最多 1024 条、每次 drain 处理 256 条且同一时刻只安排一个 queued drain；队列满时控制命令可驱逐一条日志，普通日志计入可观察 dropped 计数。
- 内部 append/clear 事务同时阻断 view 和 document 信号，避免同步槽删除对象时观察到半完成状态；测试覆盖内容/清空结果、5000 条跨线程洪泛、队列上限、drain 数和丢弃计数。

验证证据：`TestAntBuildSystem` 的显式源清单门禁、`TestAntDockPerspectiveLimits` 和 `TestAntQtExtensions` 均通过；AntLog 洪泛路径也包含在本地 MSVC ASan 的 `TestAntQtExtensions` 中。

## 整改验证记录

2026-07-16 P1/P2 最终代码实际执行并通过：

- `cmake --build build --config Debug --parallel 2`：Qt6 Debug 当前全目标增量构建成功。
- `ctest --test-dir build -C Debug --output-on-failure`：当前 Windows 顶层清单 `155 / 155` 通过，总耗时 `379.87s`；其中 `TestAntBuildSystem` 的真实父工程 configure/build/CTest 发现耗时 `247.04s`。
- `cmake --build build-qt5-msvc --config Debug --target TestAntTypography TestAntDataEntryB TestAntDataDisplayA TestAntQtExtensions TestAntDockPerspectiveLimits --parallel 2`；显式加入 Qt5 runtime PATH 后运行对应 CTest：`5 / 5` 通过，总耗时 `11.97s`。
- `cmake --build build-shared-p1p2 --config Release --target qt-ant-design --parallel 2`，随后运行 Release `TestAntInstallConsumer`：`1 / 1` 通过，总耗时 `7.77s`。
- `build-asan-p1p2` 中 9 个关键测试链接 MSVC ASan 插桩库并运行：`9 / 9` 通过，总耗时 `31.09s`。
- 对 `build/tests/subproject-integration/parent` 新建 ASan 父构建树，配置、构建并运行 `AntParentConsumer` 成功；生成 `.vcxproj` 检查确认 sanitizer 编译选项和 STL 注解兼容宏仅在库 target，宿主保留 `/RTC` 且无 `/fsanitize`。

P0 历史验证保留如下：当时配置的 Qt6 Debug `153 / 153` 在 `129.64s` 内通过，Qt5 P0/覆盖矩阵 `11 / 11` 在 `30.08s` 内通过，安装 consumer 以及 5 个生命周期/controller/QR/Dock ASan 目标通过。当前 `155 / 155` 结果已取代它作为最新全量基线，但不改写历史记录。

验证说明：一次未显式提供 Qt5 runtime PATH 的定向 CTest 启动在 `TestAntTypography` 处超时并留下本地测试进程；终止该验证进程后，两个相关测试函数单独通过，补齐 Qt5 PATH 的完整 5 项复跑也全部通过。Windows/MSVC ASan consumer 运行时仍输出已知 interception failure 警告；在 `ASAN_WIN_CONTINUE_ON_INTERCEPTION_FAILURE=1` 下退出码为 0，因此该结果保留对应盲区说明。

## 关闭清单

完成一项优化前，至少确认：

- [x] 已补充能在修复前失败、修复后通过的回归测试。
- [x] 已运行相关 targeted tests。
- [x] 生命周期或内存安全改动已运行 sanitizer 测试。
- [x] 公共 API 行为、错误语义和兼容性已确认。
- [x] README、AGENTS、项目状态和示例已按需要同步。
- [x] `git diff --check` 通过。
- [x] 对构建/安装改动运行源码构建、安装和独立 consumer 验证。

## 审计限制

- 已执行当前 Qt6 全量 155 项、Qt5 定向、static/shared 安装 consumer、真实父工程、MSVC ASan 和固定种子 property 回归；未执行完整顶层 clean build、本地 UBSan 或 libFuzzer/持续 fuzz campaign。
- Windows/MSVC ASan 对少量运行库指令存在 interception failure，continue 模式会形成检测盲区；不能把本次通过外推为所有内存路径均无问题。Linux ASan+UBSan 和 macOS 作业已配置但未在本轮远程执行。
- 没有联网查询 Qt、图片 decoder 插件、GitHub Actions 或工具链 CVE，也没有验证新增远程 CI 矩阵的实际运行结果。
- 未执行“整改前 0.1.2 头文件编译的 consumer + 当前 shared DLL”的跨版本替换实验；shared 当前头/库/安装 consumer 已通过，但历史二进制兼容仍不能视为动态证明。
- 图片 helper 限制本项目可见的编码快照和解码输出预算，但 Qt decoder 插件内部临时分配不完全受控；低信任 UNC/远程文件路径是否允许、缓存基于 size+mtime 是否需要额外失效，仍由宿主信任边界决定。
- 未结合具体宿主确认 perspective、图片、URL 和本地文件输入是否来自低权限用户、IPC、同步目录或其他不可信来源；这些条件性风险不能在库级静态审计中升级为漏洞。
- System32 DLL 加载已在当前 Windows 环境验证；Win10/Win11 原生窗口、DWM 和第三方 Qt 平台/图片插件行为仍需在实际部署组合持续验证。
