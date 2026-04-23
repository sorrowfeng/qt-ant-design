## AI 迭代开发指令

当我说：`@移植下一个组件` 或 `@implement [组件名]` 时，请严格遵循以下流程：

1. **组件名**：若未指定，请查阅 AGENT.md 中的“待移植组件”列表，选择优先级最高的一个（例如基础表单控件）。
2. **实现**：参考 `[ant-design路径]` 和 `[ElaWidgetTools路径]`，生成 `Ant[组件名].h/.cpp`。
3. **示例**：更新 `examples/ExampleWindow.cpp` 增加展示页。
4. **文档**：重新扫描 `src/widgets` 与 ant-design 官方组件列表，更新 AGENT.md 的清单。
5. **提交**：执行 `git add` 和 `git commit -m "feat: add Ant[组件名]"` 并推送。

请执行。
