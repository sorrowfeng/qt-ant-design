#!/usr/bin/env python3
"""
Build the GitHub Pages static site for qt-ant-design.

The dev branch keeps this generator as the source of truth. The generated files
are written under build/gh-pages-site and can be copied into the gh_page branch.
"""

from __future__ import annotations

import datetime as _dt
import re
import shutil
from pathlib import Path

from generate_component_api_overview import (
    CATEGORY_INTRO,
    CATEGORY_ORDER,
    ROOT,
    build_data,
    esc,
    image_path_for_component,
    render_html,
)


SITE_DIR = ROOT / "build" / "gh-pages-site"
PAGES_URL = "http://www.sorrowfeng.top/qt-ant-design/"
GITHUB_URL = "https://github.com/sorrowfeng/qt-ant-design"
ANT_DESIGN_URL = "https://ant.design/index-cn"


SITE_CSS = r"""
:root {
  color-scheme: light;
  --brand: #1677ff;
  --brand-hover: #4096ff;
  --qt: #41cd52;
  --ink: rgba(0, 0, 0, 0.88);
  --text: rgba(0, 0, 0, 0.72);
  --muted: rgba(0, 0, 0, 0.48);
  --line: #f0f0f0;
  --line-strong: #d9d9d9;
  --bg: #fff;
  --surface: #fff;
  --soft: #f5f8ff;
  --code: #f6f8fa;
  --radius: 8px;
  --shadow: 0 8px 24px rgba(18, 35, 64, 0.08);
}
* { box-sizing: border-box; }
html { scroll-behavior: smooth; }
body {
  margin: 0;
  font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", "PingFang SC", "Microsoft YaHei", "Helvetica Neue", Arial, sans-serif;
  color: var(--ink);
  background: var(--bg);
  font-size: 14px;
  line-height: 1.7;
}
a { color: inherit; text-decoration: none; }
a:hover { color: var(--brand); }
.site-header {
  position: sticky;
  top: 0;
  z-index: 50;
  height: 64px;
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 0 32px;
  background: rgba(255, 255, 255, 0.94);
  border-bottom: 1px solid var(--line);
  backdrop-filter: blur(10px);
}
.brand {
  display: inline-flex;
  align-items: center;
  gap: 10px;
  min-width: 190px;
  font-weight: 700;
  font-size: 18px;
}
.brand img { width: 32px; height: 32px; }
.nav {
  display: flex;
  align-items: center;
  gap: 4px;
}
.nav a {
  padding: 8px 12px;
  border-radius: 6px;
  color: var(--text);
}
.nav a.active,
.nav a:hover {
  color: var(--brand);
  background: #e6f4ff;
}
.hero {
  position: relative;
  min-height: clamp(500px, 74vh, 720px);
  display: flex;
  align-items: center;
  padding: 72px max(32px, calc((100vw - 1180px) / 2)) 82px;
  overflow: hidden;
  isolation: isolate;
  background-image: url("resources/images/showcase-light.png");
  background-size: cover;
  background-position: center;
}
.hero::before {
  content: "";
  position: absolute;
  inset: 0;
  z-index: -1;
  background: rgba(255, 255, 255, 0.78);
}
.hero-copy {
  width: min(720px, 100%);
}
.eyebrow {
  display: inline-flex;
  align-items: center;
  gap: 8px;
  color: var(--brand);
  font-size: 13px;
  font-weight: 650;
  letter-spacing: 0;
}
h1 {
  margin: 18px 0 18px;
  font-size: clamp(44px, 7vw, 82px);
  line-height: 1.04;
  letter-spacing: 0;
}
.hero p {
  margin: 0;
  max-width: 660px;
  color: rgba(0, 0, 0, 0.68);
  font-size: 18px;
}
.hero-actions {
  display: flex;
  flex-wrap: wrap;
  gap: 12px;
  margin-top: 34px;
}
.btn {
  display: inline-flex;
  align-items: center;
  justify-content: center;
  min-height: 40px;
  padding: 8px 18px;
  border: 1px solid var(--line-strong);
  border-radius: 6px;
  background: #fff;
  color: var(--ink);
  font-weight: 600;
}
.btn.primary {
  border-color: var(--brand);
  background: var(--brand);
  color: #fff;
  box-shadow: 0 8px 18px rgba(22, 119, 255, 0.18);
}
.btn.primary:hover { background: var(--brand-hover); color: #fff; }
.section {
  max-width: 1180px;
  margin: 0 auto;
  padding: 68px 32px;
}
.section.compact { padding-top: 46px; }
.section-head {
  display: flex;
  align-items: end;
  justify-content: space-between;
  gap: 24px;
  margin-bottom: 24px;
}
.section h2 {
  margin: 0;
  font-size: 30px;
  line-height: 1.25;
  letter-spacing: 0;
}
.section-head p,
.lead {
  margin: 8px 0 0;
  max-width: 760px;
  color: var(--text);
}
.stats {
  display: grid;
  grid-template-columns: repeat(4, minmax(0, 1fr));
  gap: 16px;
}
.stat,
.card,
.doc-card,
.shot {
  border: 1px solid var(--line);
  border-radius: var(--radius);
  background: var(--surface);
  box-shadow: var(--shadow);
}
.stat {
  padding: 20px;
}
.stat strong {
  display: block;
  font-size: 32px;
  line-height: 1.1;
}
.stat span { color: var(--muted); }
.feature-grid,
.doc-grid {
  display: grid;
  grid-template-columns: repeat(4, minmax(0, 1fr));
  gap: 16px;
}
.card,
.doc-card {
  padding: 22px;
}
.card h3,
.doc-card h3 {
  margin: 0 0 8px;
  font-size: 18px;
}
.card p,
.doc-card p {
  margin: 0;
  color: var(--text);
}
.showcase-grid {
  display: grid;
  grid-template-columns: repeat(2, minmax(0, 1fr));
  gap: 18px;
}
.shot {
  overflow: hidden;
}
.shot img {
  display: block;
  width: 100%;
  aspect-ratio: 16 / 9;
  object-fit: cover;
  background: #f5f5f5;
}
.shot figcaption {
  padding: 12px 14px;
  color: var(--muted);
}
.component-strip {
  display: grid;
  grid-template-columns: repeat(6, minmax(0, 1fr));
  gap: 14px;
}
.component-strip a {
  display: block;
  overflow: hidden;
  border: 1px solid var(--line);
  border-radius: var(--radius);
  background: #fff;
}
.component-strip img {
  display: block;
  width: 100%;
  aspect-ratio: 16 / 10;
  object-fit: cover;
  border-bottom: 1px solid var(--line);
}
.component-strip span {
  display: block;
  padding: 9px 10px;
  color: var(--text);
}
.page-hero {
  border-bottom: 1px solid var(--line);
  background: #fff;
}
.page-hero .section {
  padding-top: 58px;
  padding-bottom: 48px;
}
.page-hero h1 {
  margin: 12px 0 10px;
  font-size: clamp(36px, 5vw, 56px);
}
.content-grid {
  display: grid;
  grid-template-columns: 250px minmax(0, 1fr);
  gap: 32px;
  align-items: start;
}
.side {
  position: sticky;
  top: 88px;
  border-right: 1px solid var(--line);
  padding-right: 18px;
}
.side a {
  display: block;
  padding: 8px 0;
  color: var(--muted);
}
.article {
  min-width: 0;
}
.article section {
  padding-bottom: 28px;
  border-bottom: 1px solid var(--line);
  margin-bottom: 28px;
}
.article h2 {
  margin: 0 0 12px;
  font-size: 26px;
}
.article h3 {
  margin: 22px 0 8px;
  font-size: 18px;
}
.article p,
.article li {
  color: var(--text);
}
.article ul,
.article ol {
  padding-left: 20px;
}
pre {
  margin: 14px 0;
  overflow: auto;
  border: 1px solid var(--line);
  border-radius: var(--radius);
  background: var(--code);
}
code {
  font-family: "SFMono-Regular", Consolas, "Liberation Mono", Menlo, monospace;
  font-size: 12px;
}
pre code {
  display: block;
  padding: 16px;
  line-height: 1.65;
}
.callout {
  padding: 16px 18px;
  border: 1px solid #91caff;
  border-radius: var(--radius);
  background: #e6f4ff;
  color: rgba(0, 0, 0, 0.72);
}
.footer {
  border-top: 1px solid var(--line);
  padding: 24px 32px;
  color: var(--muted);
  text-align: center;
}
@media (max-width: 980px) {
  .site-header { padding: 0 18px; }
  .nav { gap: 0; }
  .nav a { padding: 7px 9px; }
  .stats,
  .feature-grid,
  .doc-grid,
  .component-strip {
    grid-template-columns: repeat(2, minmax(0, 1fr));
  }
  .content-grid {
    grid-template-columns: 1fr;
  }
  .side {
    position: static;
    border-right: 0;
    border-bottom: 1px solid var(--line);
    padding: 0 0 14px;
  }
}
@media (max-width: 680px) {
  .brand span { display: none; }
  .nav a:nth-last-child(-n + 2) { display: none; }
  .hero { padding: 54px 20px 62px; }
  .hero p { font-size: 16px; }
  .section { padding: 44px 20px; }
  .section-head { display: block; }
  .stats,
  .feature-grid,
  .doc-grid,
  .showcase-grid,
  .component-strip {
    grid-template-columns: 1fr;
  }
}
"""


def rel(path: str, base: str) -> str:
    return f"{base}{path}"


def site_nav(active: str, base: str) -> str:
    links = [
        ("home", "首页", rel("index.html", base)),
        ("components", "组件", rel("components/", base)),
        ("guide", "指南", rel("docs/getting-started.html", base)),
        ("design", "设计", rel("docs/design.html", base)),
        ("compatibility", "兼容性", rel("docs/compatibility.html", base)),
        ("github", "GitHub", GITHUB_URL),
        ("ant", "Ant Design", ANT_DESIGN_URL),
    ]
    items = []
    for key, label, href in links:
        css = ' class="active"' if key == active else ""
        items.append(f'<a{css} href="{esc(href)}">{esc(label)}</a>')
    return "".join(items)


def shell(title: str, description: str, active: str, base: str, body: str) -> str:
    nav = site_nav(active, base)
    favicon_svg = rel("assets/qt-ant-design-icon.svg", base)
    favicon_png = rel("assets/qt-ant-design-icon.png", base)
    logo = rel("assets/qt-ant-design-icon.png", base)
    return (
        "<!doctype html>\n"
        '<html lang="zh-CN">\n'
        "<head>\n"
        '  <meta charset="utf-8">\n'
        '  <meta name="viewport" content="width=device-width, initial-scale=1">\n'
        f"  <title>{esc(title)}</title>\n"
        f'  <meta name="description" content="{esc(description)}">\n'
        f'  <link rel="icon" type="image/svg+xml" href="{esc(favicon_svg)}">\n'
        f'  <link rel="alternate icon" type="image/png" href="{esc(favicon_png)}">\n'
        f'  <link rel="apple-touch-icon" href="{esc(favicon_png)}">\n'
        "  <style>\n"
        f"{SITE_CSS}\n"
        "  </style>\n"
        "</head>\n"
        "<body>\n"
        '  <header class="site-header">\n'
        f'    <a class="brand" href="{esc(rel("index.html", base))}"><img src="{esc(logo)}" alt="qt-ant-design logo"><span>qt-ant-design</span></a>\n'
        f'    <nav class="nav" aria-label="主导航">{nav}</nav>\n'
        "  </header>\n"
        f"{body}\n"
        '  <footer class="footer">qt-ant-design · Qt Widgets components inspired by Ant Design · <a href="https://github.com/sorrowfeng/qt-ant-design">GitHub</a></footer>\n'
        "</body>\n"
        "</html>\n"
    )


def component_image(component_name: str, base: str) -> str:
    path = image_path_for_component(component_name)
    if path.startswith("../"):
        path = path[3:]
    return rel(path, base)


def render_home(components: list[dict[str, object]], aliases: list[dict[str, str]], enums: list[dict[str, object]]) -> str:
    generated = _dt.date.today().isoformat()
    cards = [
        ("AntButton", "按钮"),
        ("AntInput", "输入"),
        ("AntTable", "表格"),
        ("AntFileDialog", "文件对话框"),
        ("AntWindow", "窗口"),
        ("AntIcon", "图标"),
    ]
    component_cards = "".join(
        f'<a href="components/#{esc(name.lower())}"><img src="{esc(component_image(name, ""))}" alt="{esc(name)} 示例"><span>{esc(label)}</span></a>'
        for name, label in cards
    )
    body = f"""
  <main>
    <section class="hero">
      <div class="hero-copy">
        <span class="eyebrow">Qt Widgets x Ant Design</span>
        <h1>qt-ant-design</h1>
        <p>一个面向 Qt5 / Qt6 Widgets 应用的 Ant Design 风格组件库，用 QPainter 与 QProxyStyle 复刻桌面端可用的现代 UI 体验。</p>
        <div class="hero-actions">
          <a class="btn primary" href="components/">查看组件</a>
          <a class="btn" href="docs/getting-started.html">快速开始</a>
          <a class="btn" href="{esc(GITHUB_URL)}">GitHub</a>
        </div>
      </div>
    </section>
    <section class="section compact" aria-label="项目状态">
      <div class="stats">
        <div class="stat"><strong>88</strong><span>公开组件覆盖</span></div>
        <div class="stat"><strong>70 / 70</strong><span>Ant Design 标准组件</span></div>
        <div class="stat"><strong>{len(components)}</strong><span>文档 API 类</span></div>
        <div class="stat"><strong>{len(enums)}</strong><span>共享枚举</span></div>
      </div>
    </section>
    <section class="section">
      <div class="section-head">
        <div>
          <span class="eyebrow">Design System</span>
          <h2>把 Ant Design 带到原生桌面</h2>
          <p>参考 Ant Design 官网的文档层级组织站点：先给出清晰入口，再提供组件索引、开发指南、设计准则和兼容性说明。</p>
        </div>
      </div>
      <div class="feature-grid">
        <article class="card"><h3>原生 Qt Widgets</h3><p>输出 C++ 控件库，可作为静态库或动态库集成到现有 Qt 工程。</p></article>
        <article class="card"><h3>Ant Design 视觉</h3><p>使用 token、状态、间距、圆角和动效对齐 Ant Design 的桌面化表达。</p></article>
        <article class="card"><h3>Qt5 / Qt6 兼容</h3><p>CMake 自动识别 Qt 版本，并覆盖 Qt5.15.2 与 Qt6 的视觉和度量差异。</p></article>
        <article class="card"><h3>桌面扩展</h3><p>额外提供 AntWindow、AntDialog、AntFileDialog、Dock、Ribbon、Nav 等桌面组件。</p></article>
      </div>
    </section>
    <section class="section">
      <div class="section-head">
        <div>
          <span class="eyebrow">Showcase</span>
          <h2>亮色与暗色主题</h2>
          <p>示例程序覆盖所有公开组件，截图资源会跟随站点一起发布。</p>
        </div>
      </div>
      <div class="showcase-grid">
        <figure class="shot"><img src="resources/images/showcase-light.png" alt="qt-ant-design 亮色主题截图"><figcaption>Light mode</figcaption></figure>
        <figure class="shot"><img src="resources/images/showcase-dark.png" alt="qt-ant-design 暗色主题截图"><figcaption>Dark mode</figcaption></figure>
      </div>
    </section>
    <section class="section">
      <div class="section-head">
        <div>
          <span class="eyebrow">Components</span>
          <h2>常用控件预览</h2>
          <p>组件页提供完整搜索、截图、头文件、继承关系、属性、方法、slots、signals 和共享枚举说明。</p>
        </div>
        <a class="btn" href="components/">全部组件</a>
      </div>
      <div class="component-strip">{component_cards}</div>
    </section>
    <section class="section">
      <div class="section-head">
        <div>
          <span class="eyebrow">Docs</span>
          <h2>文档入口</h2>
          <p>围绕“怎么接入、怎么设计、怎么验证”补齐组件总览之外的页面。</p>
        </div>
      </div>
      <div class="doc-grid">
        <a class="doc-card" href="docs/getting-started.html"><h3>快速开始</h3><p>CMake 集成、初始化入口、示例程序和安装包使用方式。</p></a>
        <a class="doc-card" href="docs/design.html"><h3>设计与架构</h3><p>主题 token、QProxyStyle 绘制策略、组件分类和桌面扩展原则。</p></a>
        <a class="doc-card" href="docs/compatibility.html"><h3>兼容性与质量</h3><p>Qt5/Qt6、高 DPI、视觉对齐和自动化测试覆盖。</p></a>
        <a class="doc-card" href="components/"><h3>组件 API</h3><p>{len(components)} 个 API 类、{len(aliases)} 个 Qt 风格别名、{len(enums)} 个共享枚举。</p></a>
      </div>
      <p class="lead">站点生成时间：{esc(generated)}。发布目标：<a href="{esc(PAGES_URL)}">{esc(PAGES_URL)}</a></p>
    </section>
  </main>"""
    return shell(
        "qt-ant-design - Qt Widgets Ant Design 组件库",
        "qt-ant-design 是一个支持 Qt5 与 Qt6 的 Ant Design 风格 Qt Widgets 组件库。",
        "home",
        "",
        body,
    )


def render_getting_started() -> str:
    body = """
  <main>
    <section class="page-hero">
      <div class="section">
        <span class="eyebrow">Guide</span>
        <h1>快速开始</h1>
        <p class="lead">使用 CMake 将 qt-ant-design 接入 Qt Widgets 项目，并完成主题、资源、字体和高 DPI 初始化。</p>
      </div>
    </section>
    <section class="section content-grid">
      <aside class="side">
        <a href="#requirements">环境要求</a>
        <a href="#subdir">子目录接入</a>
        <a href="#package">安装包接入</a>
        <a href="#startup">启动初始化</a>
        <a href="#example">运行示例</a>
      </aside>
      <article class="article">
        <section id="requirements">
          <h2>环境要求</h2>
          <ul>
            <li>Qt 5.15.2 或 Qt 6.x，至少需要 Core、Widgets、Svg 模块。</li>
            <li>CMake 3.16+，C++17 编译器。</li>
            <li>Windows 下推荐启用高 DPI 感知，项目已提供统一启动入口。</li>
          </ul>
        </section>
        <section id="subdir">
          <h2>作为子目录接入</h2>
          <p>适合和业务工程一起构建，库类型通过标准 <code>BUILD_SHARED_LIBS</code> 控制。</p>
          <pre><code>add_subdirectory(qt-ant-design)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets Svg)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Svg)

target_link_libraries(my-qt-app PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    qt-ant-design
)</code></pre>
        </section>
        <section id="package">
          <h2>安装包接入</h2>
          <pre><code>cmake -S . -B build -DCMAKE_PREFIX_PATH=/path/to/Qt
cmake --build build --config Release --target install</code></pre>
          <p>消费项目中使用 CMake package：</p>
          <pre><code>find_package(qt-ant-design CONFIG REQUIRED)
target_link_libraries(my-qt-app PRIVATE qt-ant-design::qt-ant-design)</code></pre>
        </section>
        <section id="startup">
          <h2>启动初始化</h2>
          <p>推荐在创建 <code>QApplication</code> 前配置高 DPI，创建应用后初始化资源、字体和主题。</p>
          <pre><code>int main(int argc, char* argv[])
{
    AntDesign::configureHighDpi();
    QApplication app(argc, argv);
    AntDesign::initialize(&app);

    auto* button = new AntButton;
    button->setText(QStringLiteral("Primary"));
    button->setButtonType(Ant::ButtonType::Primary);
    button->show();

    return app.exec();
}</code></pre>
        </section>
        <section id="example">
          <h2>运行示例</h2>
          <pre><code>cmake -S . -B build -DCMAKE_PREFIX_PATH=/path/to/Qt
cmake --build build --config Debug --target qt-ant-design-example</code></pre>
          <div class="callout">示例程序使用 AntWindow、AntNav 和 AntScrollArea 组织所有组件页面，是检查主题、滚动、缩放比例和 Qt5/Qt6 视觉一致性的主入口。</div>
        </section>
      </article>
    </section>
  </main>"""
    return shell("快速开始 - qt-ant-design", "qt-ant-design 的 CMake 集成、初始化和示例程序运行说明。", "guide", "../", body)


def render_design_page(components: list[dict[str, object]]) -> str:
    category_cards = []
    for category in CATEGORY_ORDER:
        count = sum(1 for item in components if item.get("category") == category)
        if not count:
            continue
        category_cards.append(
            f'<article class="card"><h3>{esc(category)}</h3><p>{esc(CATEGORY_INTRO.get(category, "公开组件分类。"))} 当前 {count} 个 API 类。</p></article>'
        )
    body = f"""
  <main>
    <section class="page-hero">
      <div class="section">
        <span class="eyebrow">Design</span>
        <h1>设计与架构</h1>
        <p class="lead">站点和组件库都以 Ant Design 的信息架构为参考，同时保留 Qt Widgets 的原生事件、对象树和布局模型。</p>
      </div>
    </section>
    <section class="section content-grid">
      <aside class="side">
        <a href="#principles">设计原则</a>
        <a href="#paint">绘制架构</a>
        <a href="#categories">组件分类</a>
        <a href="#desktop">桌面扩展</a>
      </aside>
      <article class="article">
        <section id="principles">
          <h2>设计原则</h2>
          <p>qt-ant-design 不是简单套皮，而是把 Ant Design 的视觉层级、状态反馈、组件密度和 token 思路转译为 Qt Widgets 可维护的实现。</p>
          <ul>
            <li>核心视觉值优先来自 AntTheme / AntPalette，减少散落硬编码。</li>
            <li>示例页面避免 QSS，确保控件本身承担主题响应和绘制职责。</li>
            <li>公开 API 尽量贴近 Qt 常用控件习惯，让迁移成本更低。</li>
          </ul>
        </section>
        <section id="paint">
          <h2>绘制架构</h2>
          <p>组件优先采用 QProxyStyle，把 API、状态维护、子控件管理和绘制职责拆开。</p>
          <div class="feature-grid">
            <article class="card"><h3>Pattern A</h3><p>自定义 QWidget 通过 eventFilter 交给 Style 绘制。</p></article>
            <article class="card"><h3>Pattern B</h3><p>标准 Qt 控件子类通过 drawControl / drawComplexControl 绘制。</p></article>
            <article class="card"><h3>Pattern C</h3><p>简单容器或特殊控件保留 paintEvent 自绘。</p></article>
            <article class="card"><h3>主题刷新</h3><p>Style 局部收集目标控件，避免主题切换时全局重扫。</p></article>
          </div>
        </section>
        <section id="categories">
          <h2>组件分类</h2>
          <div class="feature-grid">{"".join(category_cards)}</div>
        </section>
        <section id="desktop">
          <h2>桌面扩展</h2>
          <p>在 Ant Design 标准组件之外，项目还补充 AntWindow、AntDialog、AntFileDialog、AntDockManager、AntRibbon、AntNav 等 Qt 桌面应用常用结构。</p>
          <div class="showcase-grid">
            <figure class="shot"><img src="../resources/images/components/ant-window-light.png" alt="AntWindow 示例"><figcaption>AntWindow</figcaption></figure>
            <figure class="shot"><img src="../resources/images/components/ant-file-dialog-light.png" alt="AntFileDialog 示例"><figcaption>AntFileDialog</figcaption></figure>
          </div>
        </section>
      </article>
    </section>
  </main>"""
    return shell("设计与架构 - qt-ant-design", "qt-ant-design 的设计原则、绘制架构、组件分类和桌面扩展说明。", "design", "../", body)


def render_compatibility_page() -> str:
    body = """
  <main>
    <section class="page-hero">
      <div class="section">
        <span class="eyebrow">Compatibility</span>
        <h1>兼容性与质量</h1>
        <p class="lead">项目同时面向 Qt5 与 Qt6，重点处理控件默认样式、图标渲染、高 DPI、窗口系统和多分辨率显示差异。</p>
      </div>
    </section>
    <section class="section content-grid">
      <aside class="side">
        <a href="#qt">Qt 版本</a>
        <a href="#dpi">高 DPI</a>
        <a href="#quality">质量门禁</a>
        <a href="#pages">Pages 发布</a>
      </aside>
      <article class="article">
        <section id="qt">
          <h2>Qt 版本</h2>
          <p>CMake 配置时使用 <code>find_package(QT NAMES Qt6 Qt5 ...)</code> 自动识别 Qt6 或 Qt5。当前重点验证环境包含 Qt 5.15.2 和 Qt 6.x。</p>
          <ul>
            <li>Qt5 / Qt6 AntIcon 渲染保持同一 SVG 资源路径和视觉结果。</li>
            <li>输入框、树、列表、滚动区域、对话框和文件对话框覆盖专项视觉回归。</li>
            <li>安装包消费工程通过 <code>find_package(qt-ant-design CONFIG REQUIRED)</code> 验证。</li>
          </ul>
        </section>
        <section id="dpi">
          <h2>高 DPI 与多分辨率</h2>
          <p><code>AntDesign::configureHighDpi()</code> 在 Qt5 下启用逻辑高 DPI 缩放和高 DPI pixmap，并使用 pass-through scale rounding 策略减少 125%、150% 缩放下的尺寸漂移。</p>
          <div class="callout">示例程序在主屏幕分辨率和缩放比例之外，还通过自动化测试覆盖 1.0、1.25、1.5 等比例的逻辑截图尺寸。</div>
        </section>
        <section id="quality">
          <h2>质量门禁</h2>
          <div class="feature-grid">
            <article class="card"><h3>组件可靠性</h3><p>公开组件覆盖 API、信号、生命周期、主题切换和渲染烟测。</p></article>
            <article class="card"><h3>视觉对齐</h3><p>组件截图和 Qt5/Qt6 atlas 对比用于发现渲染差异。</p></article>
            <article class="card"><h3>无 QSS 示例</h3><p>示例程序保持零样式表操作，避免绕开控件自身绘制。</p></article>
            <article class="card"><h3>真实页面遍历</h3><p>示例程序会遍历全部页面并对滚动和空白渲染做检查。</p></article>
          </div>
        </section>
        <section id="pages">
          <h2>Pages 发布</h2>
          <p>GitHub Pages 使用独立 <code>gh_page</code> 分支承载静态产物，开发分支只保留生成脚本和源文档。项目页目标路径为 <code>www.sorrowfeng.top/qt-ant-design</code>。</p>
        </section>
      </article>
    </section>
  </main>"""
    return shell("兼容性与质量 - qt-ant-design", "qt-ant-design 的 Qt5/Qt6、高 DPI、测试和 GitHub Pages 发布说明。", "compatibility", "../", body)


def render_404() -> str:
    body = """
  <main>
    <section class="page-hero">
      <div class="section">
        <span class="eyebrow">404</span>
        <h1>页面没有找到</h1>
        <p class="lead">你可以回到首页，或从组件文档继续浏览。</p>
        <div class="hero-actions">
          <a class="btn primary" href="index.html">回到首页</a>
          <a class="btn" href="components/">组件文档</a>
        </div>
      </div>
    </section>
  </main>"""
    return shell("404 - qt-ant-design", "qt-ant-design Pages 404 页面。", "home", "", body)


def render_components_site(components: list[dict[str, object]], aliases: list[dict[str, str]], enums: list[dict[str, object]]) -> str:
    html = render_html(components, aliases, enums)
    html = html.replace("qt-ant-design 组件与 API 总览", "组件 - qt-ant-design", 1)
    html = html.replace('<a class="brand" href="#top">', '<a class="brand" href="../index.html">')
    replacement_nav = (
        '<nav class="topnav" aria-label="主导航">'
        '<a href="../index.html">首页</a>'
        '<a class="active" href="./">组件</a>'
        '<a href="../docs/getting-started.html">指南</a>'
        '<a href="../docs/design.html">设计</a>'
        '<a href="../docs/compatibility.html">兼容性</a>'
        '<a href="https://github.com/sorrowfeng/qt-ant-design">GitHub</a>'
        '<a href="https://ant.design/components/overview-cn/">Ant Design</a>'
        "</nav>"
    )
    html = re.sub(r'<nav class="topnav" aria-label="主导航">.*?</nav>', replacement_nav, html, flags=re.S)
    html = html.replace("本地文档", "组件文档")
    html = html.replace('href="./project-status.md"', 'href="../docs/compatibility.html"')
    html = html.replace('href="./qt-control-porting-guidelines.md"', 'href="../docs/design.html"')
    html = html.replace('href="./visual-audit.md"', 'href="../docs/compatibility.html"')
    return html


def safe_reset_site_dir() -> None:
    build_root = (ROOT / "build").resolve()
    target = SITE_DIR.resolve()
    if target != build_root and build_root not in target.parents:
        raise RuntimeError(f"Refusing to delete unexpected path: {target}")
    if SITE_DIR.exists():
        shutil.rmtree(SITE_DIR)
    SITE_DIR.mkdir(parents=True)


def copy_assets() -> None:
    assets_target = SITE_DIR / "assets"
    assets_target.mkdir(parents=True, exist_ok=True)
    for name in ("qt-ant-design-icon.png", "qt-ant-design-icon.svg"):
        shutil.copy2(ROOT / "assets" / name, assets_target / name)

    resources_target = SITE_DIR / "resources" / "images"
    (resources_target / "components").mkdir(parents=True, exist_ok=True)
    for name in ("showcase-light.png", "showcase-dark.png"):
        shutil.copy2(ROOT / "resources" / "images" / name, resources_target / name)
    for image in (ROOT / "resources" / "images" / "components").glob("*.png"):
        shutil.copy2(image, resources_target / "components" / image.name)


def write(path: Path, text: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text, encoding="utf-8", newline="\n")


def main() -> None:
    components, aliases, enums = build_data()
    safe_reset_site_dir()
    copy_assets()
    write(SITE_DIR / ".nojekyll", "")
    write(SITE_DIR / "index.html", render_home(components, aliases, enums))
    write(SITE_DIR / "components" / "index.html", render_components_site(components, aliases, enums))
    write(SITE_DIR / "docs" / "getting-started.html", render_getting_started())
    write(SITE_DIR / "docs" / "design.html", render_design_page(components))
    write(SITE_DIR / "docs" / "compatibility.html", render_compatibility_page())
    write(SITE_DIR / "404.html", render_404())
    print(f"Generated {SITE_DIR.relative_to(ROOT)}")
    print(f"Pages: index, components, guide, design, compatibility")


if __name__ == "__main__":
    main()
