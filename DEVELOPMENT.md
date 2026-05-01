## AI 迭代开发指令

当前项目状态见 `docs/project-status.md`，视觉审计矩阵见 `docs/visual-audit.md`，官方图标清单见 `docs/ant-design-icons.md`。

截至 `2026-05-01`：

- Ant Design 标准组件：`70 / 70` 已覆盖。
- 公开组件：`82` 个；`src/widgets/AntSelectPopup.h` 是内部 helper，不计入公开组件。
- 示例页：覆盖 `82 / 82` 个公开组件，另有独立 `Showcase` 页面。
- 测试：`34 / 34` 个 CTest 目标通过。
- 图标：`AntIcon` 已内置 `831` 个官方 SVG 资源。

当我说：`@移植下一个组件` 或 `@implement [组件名]` 时，当前默认含义应理解为“新增尚不存在的扩展组件或补一个缺失能力”，而不是继续从 Ant Design 标准组件队列取项。请遵循以下流程：

1. **确认范围**：先查阅 `AGENT.md` 和 `docs/project-status.md`，确认它是新增能力、视觉修复、示例补全，还是测试/文档补全。
2. **实现**：参考 `https://github.com/ant-design/ant-design` 和 `https://github.com/Liniyous/ElaWidgetTools`，沿用现有 `widgets / styles / core` 分层。
3. **示例**：如是可见控件或能力，更新 `examples/pages` 与 `PageRegistry`。
4. **文档**：更新 `AGENT.md`、`docs/project-status.md`，必要时更新 `docs/visual-audit.md`。
5. **验证**：至少构建相关测试和 `qt-ant-design-example`，风险较大时运行 `ctest --test-dir build -C Debug --output-on-failure`。
6. **提交**：仅在用户要求时执行 commit/push。

请执行。
