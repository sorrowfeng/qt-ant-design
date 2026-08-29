# AI 迭代开发指令

当前项目状态以 [AGENTS.md](AGENTS.md) 为准；面向用户和维护者的状态总览见 [docs/project-status.md](docs/project-status.md)，视觉审计矩阵见 [docs/visual-audit.md](docs/visual-audit.md)，官方图标清单见 [docs/ant-design-icons.md](docs/ant-design-icons.md)。

截至 `2026-06-24`：

- 当前版本：`0.1.2`，唯一版本源为根目录 [VERSION](VERSION)。
- Ant Design 标准组件：`70 / 70` 已覆盖。
- 公开组件：`89` 个；`src/widgets/AntSelectPopup.h` 是内部 helper，不计入公开组件。
- 示例页：覆盖 `89 / 89` 个公开组件，另有独立 `Showcase` 页面。
- 测试：当前维护 `47` 个深度/系统 CTest 条目，并可通过 `BUILD_WIDGET_SMOKE_TESTS=ON` 启用 `104` 个逐控件 smoke 条目。
- 图标：`AntIcon` 内置 `831` 个官方 SVG 资源。
- 文档站：静态页面由 `tools/generate_gh_pages_site.py` 生成并发布到独立 `gh_page` 分支。

当我说：`@移植下一个组件` 或 `@implement [组件名]` 时，当前默认含义应理解为“新增尚不存在的扩展组件或补一个缺失能力”，而不是继续从 Ant Design 标准组件队列取项。请遵循以下流程：

1. **确认范围**：先查阅 `AGENTS.md` 和 `docs/project-status.md`，确认它是新增能力、视觉修复、示例补全，还是测试/文档补全。
2. **实现**：参考 `https://github.com/ant-design/ant-design` 和 `https://github.com/Liniyous/ElaWidgetTools`，沿用现有 `widgets / styles / core` 分层。
3. **示例**：如是可见控件或能力，更新 `examples/pages` 与 `PageRegistry`。
4. **文档**：更新 `AGENTS.md`、`docs/project-status.md`、`README.md` / `README.zh-CN.md`，必要时更新 `docs/visual-audit.md`、`docs/reliability-coverage.md` 和 GitHub Pages 生成内容。
5. **验证**：至少构建相关测试和 `qt-ant-design-example`，风险较大时运行 `ctest --test-dir build -C Debug --output-on-failure`；Qt5 / Qt6 或高 DPI 相关改动需做对应定向验证。
6. **提交**：仅在用户要求时执行 commit / push；发布版本时同步更新 `VERSION`、`CHANGELOG.md` 和 `docs/versioning.md`。

请执行。
