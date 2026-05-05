# qt-ant-design 跨电脑续做通用提示词

下面这段提示词用于在新电脑上继续 `qt-ant-design` 项目的开发工作。  
使用前请先完成仓库拉取：

```bash
git clone <your-repo-url>
cd qt-ant-design
```

---

## 可直接复制使用的提示词

```text
你正在继续开发 `qt-ant-design` 项目。

## 项目背景
- 项目名称：qt-ant-design
- 目标：使用 Qt6 Widgets，将 Ant Design 设计系统移植为 C++ 组件库
- 当前绘制架构：项目处于 `paintEvent` 与 `QProxyStyle` 并存阶段，新的架构迁移优先采用 `QProxyStyle`

## 外部参考仓库
- ElaWidgetTools 参考仓库：`https://github.com/Liniyous/ElaWidgetTools`
- Ant Design 参考仓库：`https://github.com/ant-design/ant-design`

## 开始工作前必须执行
1. 阅读项目根目录下的 `AGENTS.md`
2. 阅读项目根目录下的 `README.md`
3. 扫描以下目录，了解当前代码状态：
   - `src/core/`
   - `src/styles/`
   - `src/widgets/`
   - `examples/`
4. 需要对照上游实现时，查看外部参考仓库：
   - `https://github.com/Liniyous/ElaWidgetTools`
   - `https://github.com/ant-design/ant-design`
5. 总结当前项目状态：
   - 已实现组件数量
   - 已迁移至 `QProxyStyle` 的组件
   - 仍使用 `paintEvent` 的组件
   - 示例程序当前覆盖的组件

## 开发约束
1. 所有核心视觉参数必须通过 `AntTheme` / `AntPalette` 获取
2. 新增或重构组件时，优先使用 `QProxyStyle`
3. 所有 `Ant[Component]Style` 文件统一放在 `src/styles/`
4. 组件 API 风格保持与 Ant Design 官方尽量一致
5. 不依赖 QSS 实现核心外观
6. 每完成一个阶段性任务后，同步更新：
   - `AGENTS.md`
   - `README.md`（如果功能范围发生变化）
   - `examples/ExampleWindow.cpp`（如果新增了组件或展示页）

## 如果当前任务是“继续迁移组件到 QProxyStyle”
请按以下流程执行：
1. 读取 `AGENTS.md`，找出尚未迁移到 `QProxyStyle` 的已实现组件
2. 优先选择复杂度适中的组件继续迁移
3. 在 `src/styles/` 中创建对应 `Ant[Component]Style.h/.cpp`
4. 将原组件中的主绘制逻辑迁移到 Style
5. 保持组件公开 API 不变
6. 更新 `CMakeLists.txt`
7. 更新 `AGENTS.md`
8. 编译验证：
   - `cmake -S . -B build-codex -DCMAKE_INSTALL_PREFIX=<install-path>`
   - `cmake --build build-codex --config Debug`
   - 如需要：`cmake --install build-codex --config Debug`
9. 提交并推送

## 如果当前任务不明确
请先输出一份“当前项目状态摘要”和“下一步建议”，然后从 `AGENTS.md` 中优先级最高的未完成事项开始执行。

## Git 要求
完成修改后请执行：
- `git add`
- `git commit -m "<合适的提交信息>"`
- `git push`

请先从扫描 `AGENTS.md`、`README.md` 和 `src/widgets/` 开始，并告诉我你建议优先处理什么。
```

---

## 说明

这份提示词是“通用续做入口”，适合以下场景：

- 换电脑后继续开发
- 让新的 AI 会话快速接手项目
- 在不清楚当前进度时，先做状态扫描再继续推进

如果后续你希望，我还可以继续补两类更细的提示词：

- “继续迁移 QProxyStyle”的专项提示词
- “新增一个 Ant Design 组件”的专项提示词
