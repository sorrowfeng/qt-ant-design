#!/usr/bin/env python3
"""
Generate the local Chinese component/API reference page.

The page is intentionally generated from the public widget headers so it stays
close to the exported C++ surface. It uses AGENTS.md only as human-maintained
category and description metadata.
"""

from __future__ import annotations

import datetime as _dt
import html
import re
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
WIDGETS_DIR = ROOT / "src" / "widgets"
ANT_TYPES = ROOT / "src" / "core" / "AntTypes.h"
AGENTS = ROOT / "AGENTS.md"
OUTPUT = ROOT / "docs" / "component-api-overview-cn.html"


CATEGORY_ORDER = [
    "通用",
    "布局及其他",
    "导航",
    "数据录入",
    "数据展示",
    "反馈",
    "Qt / 桌面扩展组件",
    "子组件 / 辅助类",
    "Qt 风格别名",
]

CATEGORY_INTRO = {
    "通用": "基础能力和全局体验入口，包含按钮、文字、图标和浮动入口。",
    "布局及其他": "用于组织页面结构、栅格、弹性排列、间距、分割线和应用级包装。",
    "导航": "用于在页面、步骤、菜单和锚点之间切换，强调当前位置和层级。",
    "数据录入": "收集用户输入，覆盖文本、选择、日期时间、上传、穿梭框等场景。",
    "数据展示": "展示结构化或非结构化数据，包含列表、表格、树、统计、卡片等。",
    "反馈": "表达系统状态、结果、通知、弹层确认和加载过程。",
    "Qt / 桌面扩展组件": "面向 Qt Widgets 桌面应用的窗口、对话框、工具栏、Dock、滚动等扩展。",
    "子组件 / 辅助类": "随父组件一起使用的公开子控件或辅助 API 类。",
    "Qt 风格别名": "为了贴近 Qt 原生命名习惯提供的轻量类型别名。",
}

ANT_CN_NAMES = {
    "affix": "固钉",
    "alert": "警告提示",
    "anchor": "锚点",
    "app": "应用包裹",
    "auto-complete": "自动完成",
    "avatar": "头像",
    "badge": "徽标数",
    "breadcrumb": "面包屑",
    "button": "按钮",
    "calendar": "日历",
    "card": "卡片",
    "carousel": "走马灯",
    "cascader": "级联选择",
    "checkbox": "多选框",
    "collapse": "折叠面板",
    "color-picker": "颜色选择器",
    "config-provider": "全局配置",
    "date-picker": "日期选择框",
    "descriptions": "描述列表",
    "divider": "分割线",
    "drawer": "抽屉",
    "dropdown": "下拉菜单",
    "empty": "空状态",
    "flex": "弹性布局",
    "float-button": "悬浮按钮",
    "form": "表单",
    "grid": "栅格",
    "icon": "图标",
    "image": "图片",
    "input": "输入框",
    "input-number": "数字输入框",
    "layout": "布局",
    "list": "列表",
    "masonry": "瀑布流",
    "mentions": "提及",
    "menu": "导航菜单",
    "message": "全局提示",
    "modal": "对话框",
    "notification": "通知提醒框",
    "pagination": "分页",
    "popconfirm": "气泡确认框",
    "popover": "气泡卡片",
    "progress": "进度条",
    "qr-code": "二维码",
    "rate": "评分",
    "result": "结果",
    "segmented": "分段控制器",
    "select": "选择器",
    "skeleton": "骨架屏",
    "slider": "滑动输入条",
    "space": "间距",
    "spin": "加载中",
    "splitter": "分隔面板",
    "statistic": "统计数值",
    "steps": "步骤条",
    "switch": "开关",
    "table": "表格",
    "tabs": "标签页",
    "tag": "标签",
    "time-picker": "时间选择框",
    "timeline": "时间轴",
    "tooltip": "文字提示",
    "tour": "漫游式引导",
    "transfer": "穿梭框",
    "tree": "树形控件",
    "tree-select": "树选择",
    "typography": "排版",
    "upload": "上传",
    "watermark": "水印",
}

LOCAL_CN_NAMES = {
    "AntDialog": "无边框对话框",
    "AntDockManager": "停靠管理器",
    "AntDockWidget": "停靠面板",
    "AntFileDialog": "文件对话框",
    "AntLog": "日志面板",
    "AntMenuBar": "菜单栏",
    "AntNav": "侧边导航",
    "AntNavItem": "导航项",
    "AntPlainTextEdit": "多行文本",
    "AntRibbon": "功能区",
    "AntScrollArea": "滚动区域",
    "AntScrollBar": "滚动条",
    "AntStackedWidget": "页面栈",
    "AntStatusBar": "状态栏",
    "AntToolBar": "工具栏",
    "AntToolButton": "工具按钮",
    "AntWidget": "主题容器",
    "AntWindow": "应用窗口",
}

IMAGE_OVERRIDES = {
    "AntCheckBox": "ant-checkbox",
    "AntQRCode": "ant-qr-code",
    "AntToolTip": "ant-tooltip",
    "AntDockManager": "ant-dock-widget",
    "AntListWidget": "ant-list",
    "AntRow": "ant-grid",
    "AntCol": "ant-grid",
    "AntAvatarGroup": "ant-avatar",
    "AntFormItem": "ant-form",
    "AntFormList": "ant-form",
    "AntFormProvider": "ant-form",
    "AntTimelineItem": "ant-timeline",
    "AntNavItem": "ant-nav-item",
    "AntLayoutHeader": "ant-layout",
    "AntLayoutFooter": "ant-layout",
    "AntLayoutContent": "ant-layout",
    "AntLayoutSider": "ant-layout",
    "AntSplitterHandle": "ant-splitter",
    "AntDescriptionsItem": "ant-descriptions",
    "AntCollapsePanel": "ant-collapse",
    "AntListItem": "ant-list",
    "AntListItemMeta": "ant-list",
    "AntRibbonPage": "ant-ribbon",
    "AntRibbonGroup": "ant-ribbon",
}

PROPERTY_PURPOSES = {
    "buttonType": "按钮视觉类型，用于在默认、主按钮、虚线、文本和链接形态之间切换。",
    "buttonSize": "组件尺寸，用于匹配 Large、Middle、Small 三种密度。",
    "buttonShape": "按钮轮廓形状，用于切换普通、圆形或圆角胶囊按钮。",
    "icon": "显示图标名称或图标内容，用于在控件中展示辅助视觉符号。",
    "iconName": "官方 Ant Design SVG 图标名称，会结合主题后缀解析到内置资源。",
    "iconType": "内置图标枚举，适合常用图标的类型安全调用。",
    "iconTheme": "图标主题风格，用于 Outlined、Filled、TwoTone 间切换。",
    "iconSize": "图标逻辑像素尺寸，影响绘制区域和布局占位。",
    "twoToneColor": "双色图标的主色，次色会按 Ant Design 规则推导。",
    "color": "前景色或主题色覆盖值，用于定制组件局部颜色。",
    "text": "显示文本或当前文本值。",
    "title": "标题文本，用于弹层、卡片、分组或窗口标题。",
    "content": "正文内容或主要显示内容。",
    "value": "当前值，通常由用户输入、选择或进度状态决定。",
    "currentIndex": "当前选中索引，用于列表、标签页或堆叠页面切换。",
    "currentText": "当前选中文本，用于和业务数据保持同步。",
    "checked": "选中状态，用于复选框、单选框、开关或可勾选条目。",
    "disabled": "禁用状态，禁用后控件不再响应交互并呈现禁用视觉。",
    "loading": "加载状态，启用后显示加载反馈并可阻止重复操作。",
    "open": "展开状态，用于弹层、分组、下拉或浮动按钮组。",
    "visible": "可见状态，用于控制局部 UI 是否显示。",
    "placement": "弹层或浮动元素位置，用于控制相对目标的显示方向。",
    "trigger": "触发方式，用于点击、悬停或上下文菜单等交互入口。",
    "variant": "视觉变体，用于 Outlined、Filled、Borderless、Underlined 等边框/背景风格。",
    "status": "状态语义，用于错误、警告、成功、加载等反馈色。",
    "size": "组件尺寸，用于控制间距、高度和字体等视觉密度。",
    "mode": "工作模式，用于在单选、多选、范围、标签或视图模式之间切换。",
    "rangeMode": "范围选择模式，启用后同时维护起止值。",
    "startDate": "范围日期的起始值。",
    "endDate": "范围日期的结束值。",
    "startTime": "范围时间的起始值。",
    "endTime": "范围时间的结束值。",
    "minimum": "允许输入或选择的最小值。",
    "maximum": "允许输入或选择的最大值。",
    "precision": "数值显示精度，用于控制小数位。",
    "decimals": "小数位数量，用于输入框和数值格式化。",
    "step": "步进值，用于键盘、按钮或拖动调节。",
    "pageSize": "分页每页条数。",
    "total": "数据总量，用于分页和统计展示。",
    "count": "数量值，用于徽标、评分、列表或统计。",
    "bordered": "是否显示边框，用于在卡片、表格、描述列表等场景强调边界。",
    "closable": "是否显示关闭入口。",
    "editable": "是否允许直接编辑输入内容。",
    "searchMode": "搜索模式，启用后会显示搜索交互并发出搜索信号。",
    "wordWrap": "文本是否自动换行。",
    "alignment": "内容对齐方式。",
    "autoPlay": "是否自动播放轮播内容。",
    "interval": "自动轮播间隔。",
    "arrowsVisible": "是否显示手动切换箭头。",
    "dotsVisible": "是否显示轮播指示点。",
}


def esc(value: object) -> str:
    return html.escape(str(value), quote=True)


def strip_comments(text: str) -> str:
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.S)
    return re.sub(r"//.*", "", text)


def find_matching_brace(text: str, open_index: int) -> int:
    depth = 0
    for index in range(open_index, len(text)):
        char = text[index]
        if char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                return index
    return -1


def normalize_decl(decl: str) -> str:
    decl = " ".join(decl.replace("\n", " ").split())
    decl = decl.rstrip(";").strip()
    return decl


def method_name(decl: str) -> str:
    cleaned = decl
    cleaned = re.sub(r"=\s*delete|=\s*default", "", cleaned)
    match = re.search(r"([~A-Za-z_]\w*)\s*\(", cleaned)
    return match.group(1) if match else cleaned


def property_info(raw: str) -> dict[str, str]:
    tokens = raw.split()
    info = {"type": "", "name": "", "read": "", "write": "", "notify": ""}
    if len(tokens) >= 2:
        info["type"] = tokens[0]
        info["name"] = tokens[1]
    for key in ("READ", "WRITE", "NOTIFY"):
        if key in tokens:
            index = tokens.index(key)
            if index + 1 < len(tokens):
                info[key.lower()] = tokens[index + 1]
    return info


def split_camel_words(value: str) -> list[str]:
    text = re.sub(r"([a-z0-9])([A-Z])", r"\1 \2", value)
    text = re.sub(r"([A-Z]+)([A-Z][a-z])", r"\1 \2", text)
    return [part for part in re.split(r"[\s_]+", text) if part]


def slugify_component_name(name: str) -> str:
    if name in IMAGE_OVERRIDES:
        return IMAGE_OVERRIDES[name]
    base = name[3:] if name.startswith("Ant") else name
    base = base.replace("CheckBox", "Checkbox")
    base = base.replace("QRCode", "QrCode")
    base = base.replace("ToolTip", "Tooltip")
    return "ant-" + "-".join(word.lower() for word in split_camel_words(base))


def image_path_for_component(name: str) -> str:
    slug = slugify_component_name(name)
    light = ROOT / "resources" / "images" / "components" / f"{slug}-light.png"
    if light.exists():
        return f"../resources/images/components/{slug}-light.png"
    return "../assets/qt-ant-design-icon.png"


def component_cn_name(component: dict[str, object]) -> str:
    name = str(component["name"])
    ant = str(component.get("ant", ""))
    if ant in ANT_CN_NAMES:
        return ANT_CN_NAMES[ant]
    if name in LOCAL_CN_NAMES:
        return LOCAL_CN_NAMES[name]
    return str(component.get("category", "组件"))


def property_label(name: str) -> str:
    if name in PROPERTY_PURPOSES:
        return PROPERTY_PURPOSES[name]
    lower = name.lower()
    if lower.endswith("visible"):
        return "控制对应区域或入口是否可见。"
    if lower.endswith("enabled"):
        return "控制对应功能是否启用。"
    if lower.endswith("color"):
        return "颜色配置，用于覆盖默认主题色或状态色。"
    if lower.endswith("text") or lower.endswith("title") or lower.endswith("content"):
        return "文本内容配置，用于展示或同步业务文案。"
    if lower.endswith("index") or lower.endswith("row") or lower.endswith("column"):
        return "位置索引，用于定位当前项、行列或页面。"
    if lower.endswith("count") or lower.endswith("number"):
        return "数量配置，用于显示统计、徽标或计数状态。"
    if "selected" in lower or "current" in lower:
        return "当前选择状态，用于读取或同步用户选择。"
    if "hover" in lower or "press" in lower:
        return "内部交互状态，主要供样式绘制和测试验证使用。"
    return "组件公开状态，可通过 Qt 元对象系统读取、写入或绑定。"


def signal_subject(signal_name: str) -> str:
    if signal_name.endswith("Changed"):
        return signal_name[: -len("Changed")]
    if signal_name.endswith("Clicked"):
        return signal_name[: -len("Clicked")]
    if signal_name.endswith("Requested"):
        return signal_name[: -len("Requested")]
    return signal_name


def first_param_type(decl: str) -> str:
    match = re.search(r"\((.*)\)", decl)
    if not match:
        return ""
    params = match.group(1).strip()
    if not params or params == "void":
        return ""
    first = params.split(",")[0].strip()
    first = first.split("=")[0].strip()
    parts = first.split()
    if len(parts) <= 1:
        return first
    return " ".join(parts[:-1]).replace("&", "&").strip()


def return_type(decl: str, class_name: str) -> str:
    name = method_name(decl)
    if name == class_name or name == f"~{class_name}":
        return "构造 / 析构"
    prefix = decl.split(name + "(", 1)[0].strip()
    prefix = re.sub(r"\b(static|virtual|explicit|inline|constexpr)\b", "", prefix).strip()
    return prefix or "void"


def method_params(decl: str) -> str:
    match = re.search(r"\((.*)\)", decl)
    if not match:
        return "无参数"
    params = match.group(1).strip()
    return params if params else "无参数"


def setter_property_name(name: str) -> str:
    prop = name[3:] if name.startswith("set") else name
    return prop[:1].lower() + prop[1:] if prop else prop


def value_for_param(param_type: str) -> str | None:
    normalized = param_type.replace("const ", "").replace("&", "").replace("*", "*").strip()
    if normalized in {"QString"}:
        return 'QStringLiteral("示例")'
    if normalized in {"bool"}:
        return "true"
    if normalized in {"int", "qsizetype"}:
        return "1"
    if normalized in {"qreal", "double", "float"}:
        return "1.0"
    if normalized in {"QColor"}:
        return 'QColor("#1677ff")'
    if normalized in {"QSize"}:
        return "QSize(160, 40)"
    if normalized in {"QDate"}:
        return "QDate::currentDate()"
    if normalized in {"QTime"}:
        return "QTime::currentTime()"
    if normalized.startswith("Ant::"):
        enum_name = normalized.split("::", 1)[1]
        default_values = {
            "ButtonType": "Primary",
            "Size": "Middle",
            "Variant": "Outlined",
            "Status": "Normal",
            "Placement": "Top",
            "Orientation": "Horizontal",
            "Trigger": "Click",
            "IconType": "Search",
            "IconTheme": "Outlined",
            "SelectMode": "Single",
            "TabsType": "Line",
            "MenuMode": "Vertical",
            "MenuTheme": "Light",
            "DropdownPlacement": "Bottom",
            "DropdownTrigger": "Click",
            "TooltipPlacement": "Top",
            "DrawerPlacement": "Right",
            "MessageType": "Info",
            "AlertType": "Info",
            "FormLayout": "Horizontal",
            "FormLabelAlign": "Right",
            "TimelineMode": "Start",
            "TimelineOrientation": "Vertical",
            "ThemeMode": "Default",
        }
        return f"Ant::{enum_name}::{default_values.get(enum_name, 'Default')}"
    return None


def usage_code(component: dict[str, object]) -> str:
    name = str(component["name"])
    header = str(component["header"])
    methods = list(component["methods"])
    signals = list(component["signals"])
    lines = [
        f'#include "{header}"',
        "",
        f"auto* widget = new {name}(parent);",
    ]
    used = 0
    for decl in methods:
        method = method_name(decl)
        if not method.startswith("set") or "," in method_params(decl):
            continue
        value = value_for_param(first_param_type(decl))
        if value is None:
            continue
        lines.append(f"widget->{method}({value});")
        used += 1
        if used >= 2:
            break
    for decl in signals:
        signal = method_name(decl)
        if signal in {"clicked", "activated", "currentChanged", "valueChanged", "textChanged"} or signal.endswith("Changed"):
            lines.append(f"connect(widget, &{name}::{signal}, this, []() {{")
            lines.append("    // 处理组件交互或状态同步")
            lines.append("});")
            break
    return "\n".join(lines)


def split_public_api(body: str) -> dict[str, list[str]]:
    result = {"methods": [], "slots": [], "signals": []}
    section: str | None = None
    statement = ""

    for raw_line in strip_comments(body).splitlines():
        line = raw_line.strip()
        if not line:
            continue
        normalized = line.replace(" ", "")
        if normalized in {"public:", "publicslots:", "publicQ_SLOTS:"}:
            section = "methods" if normalized == "public:" else "slots"
            statement = ""
            continue
        if normalized in {"Q_SIGNALS:", "signals:"}:
            section = "signals"
            statement = ""
            continue
        if line.endswith(":") and (
            line.startswith("protected")
            or line.startswith("private")
            or line.startswith("friend")
        ):
            section = None
            statement = ""
            continue
        if section is None:
            continue
        if line.startswith(("Q_", "friend ", "using ", "typedef ", "enum ", "struct ", "class ")):
            continue

        statement = f"{statement} {line}".strip()
        if ";" not in line:
            continue

        decl = normalize_decl(statement)
        statement = ""
        if not decl or "(" not in decl or ")" not in decl:
            continue
        if decl.startswith(("void paintEvent", "void mouse", "void enterEvent", "void leaveEvent")) and section == "methods":
            continue
        result[section].append(decl)

    return result


def parse_properties(body: str) -> list[dict[str, str]]:
    properties: list[dict[str, str]] = []
    for line in body.splitlines():
        stripped = line.strip()
        if stripped.startswith("Q_PROPERTY(") and stripped.endswith(")"):
            raw = stripped[len("Q_PROPERTY(") : -1]
            properties.append(property_info(raw))
    return properties


def parse_structs(text: str) -> list[dict[str, object]]:
    structs = []
    for match in re.finditer(r"\bstruct\s+(Ant\w+)\s*\{", text):
        name = match.group(1)
        close = find_matching_brace(text, match.end() - 1)
        if close < 0:
            continue
        body = text[match.end() : close]
        fields = []
        for raw_line in strip_comments(body).splitlines():
            line = normalize_decl(raw_line)
            if not line or "(" in line or line.startswith(("enum ", "struct ", "class ")):
                continue
            if ";" in raw_line:
                fields.append(line)
        structs.append({"name": name, "fields": fields})
    return structs


def parse_alias(text: str) -> tuple[str, str] | None:
    match = re.search(r"\busing\s+(Ant\w+)\s*=\s*(Ant\w+)\s*;", text)
    if not match:
        return None
    return match.group(1), match.group(2)


def parse_classes(path: Path) -> list[dict[str, object]]:
    text = path.read_text(encoding="utf-8")
    classes = []
    for match in re.finditer(r"\bclass\s+QT_ANT_DESIGN_EXPORT\s+(\w+)\s*:\s*public\s*([^{]+)\{", text):
        name = match.group(1)
        bases = " ".join(match.group(2).split())
        open_index = match.end() - 1
        close_index = find_matching_brace(text, open_index)
        if close_index < 0:
            continue
        body = text[open_index + 1 : close_index]
        api = split_public_api(body)
        classes.append(
            {
                "name": name,
                "bases": bases,
                "header": path.relative_to(ROOT).as_posix(),
                "properties": parse_properties(body),
                "methods": api["methods"],
                "slots": api["slots"],
                "signals": api["signals"],
            }
        )
    return classes


def parse_ant_enums() -> list[dict[str, object]]:
    text = ANT_TYPES.read_text(encoding="utf-8")
    enums = []
    for match in re.finditer(r"enum\s+class\s+(\w+)\s*\{(.*?)\};", text, flags=re.S):
        values = []
        for item in match.group(2).split(","):
            value = item.strip()
            if value:
                values.append(value)
        enums.append({"name": match.group(1), "values": values})
    return enums


def parse_agents_metadata() -> dict[str, dict[str, str]]:
    text = AGENTS.read_text(encoding="utf-8")
    metadata: dict[str, dict[str, str]] = {}
    in_status = False
    current_category: str | None = None
    known = set(CATEGORY_ORDER)

    for line in text.splitlines():
        if line.startswith("## 当前组件状态"):
            in_status = True
            continue
        if in_status and line.startswith("## ") and not line.startswith("## 当前组件状态"):
            break
        if not in_status:
            continue
        if line.startswith("### "):
            title = line[4:].strip()
            current_category = title if title in known else None
            continue
        if current_category is None or not line.startswith("|"):
            continue
        names = re.findall(r"`(Ant\w+)`", line)
        if not names:
            continue
        cells = [cell.strip() for cell in line.strip().strip("|").split("|")]
        if len(cells) < 4 or cells[0] == "---":
            continue
        note = cells[-1] if cells[-1] not in {"是", "说明"} else ""
        rendering = ""
        ant_dir = ""
        if current_category == "Qt / 桌面扩展组件":
            rendering = cells[1] if len(cells) > 1 else ""
        else:
            ant_dir = cells[1] if len(cells) > 1 else ""
            rendering = cells[2] if len(cells) > 2 else ""
        for name in names:
            metadata[name] = {
                "category": current_category,
                "ant": ant_dir.replace("`", ""),
                "rendering": rendering.replace("`", ""),
                "note": note.replace("`", ""),
            }
    return metadata


def infer_component_metadata(name: str, header_stem: str, metadata: dict[str, dict[str, str]]) -> dict[str, str]:
    if name in metadata:
        return dict(metadata[name])
    if header_stem in metadata:
        parent = metadata[header_stem]
        return {
            "category": parent["category"],
            "ant": parent.get("ant", ""),
            "rendering": parent.get("rendering", ""),
            "note": f"{header_stem} 的公开子组件 / 辅助 API 类，通常随父组件组合使用。",
        }
    return {
        "category": "子组件 / 辅助类",
        "ant": "",
        "rendering": "",
        "note": "公开辅助 API 类，供组合控件或高级场景使用。",
    }


def api_description(decl: str, kind: str, class_name: str = "") -> str:
    name = method_name(decl)
    if kind == "signal":
        if name.endswith("Changed"):
            subject = signal_subject(name)
            return f"当 {subject} 状态被接口调用或用户交互改变后发出，用于外部同步 UI、刷新业务数据或触发后续流程。"
        if name.endswith("Clicked"):
            subject = signal_subject(name)
            return f"当用户点击 {subject or '组件'} 时发出，用于连接业务动作。"
        if name.endswith("Requested"):
            subject = signal_subject(name)
            return f"当组件请求执行 {subject} 行为时发出，外部可以选择接受、拦截或补充处理。"
        return "组件交互或生命周期事件通知，可连接到外部槽函数处理业务逻辑。"
    if class_name and (name == class_name or name == f"~{class_name}"):
        return "创建或销毁组件实例。构造函数通常接收 parent 以纳入 Qt 对象树和布局生命周期管理。"
    if name.startswith("set"):
        prop = setter_property_name(name)
        return f"设置 {prop}。{property_label(prop)}"
    if name.startswith(("is", "has", "can")):
        return "读取布尔状态，常用于根据当前状态决定是否允许交互、显示入口或同步业务逻辑。"
    if name.startswith(("add", "insert", "remove", "clear", "take")):
        return "管理组件内部条目、页面、列、动作或集合数据，适合动态构建界面。"
    if name.startswith(("current", "selected", "value", "text", "title", "content")):
        return "读取当前值、选中项或显示内容，用于表单提交、状态同步和界面回填。"
    if name in {"sizeHint", "minimumSizeHint", "heightForWidth"}:
        return "返回 Qt 布局系统使用的尺寸建议，让控件在不同 DPI、字体和内容下保持合理占位。"
    if name.startswith(("show", "hide", "open", "close", "toggle")):
        return "控制显示、展开、关闭或切换状态，通常会触发重绘、动画或对应状态信号。"
    if name.startswith(("scroll", "move", "resize")):
        return "执行位置、滚动或尺寸相关操作，用于和窗口/视图交互联动。"
    if name.startswith(("find", "indexOf", "item", "row", "column")):
        return "查询内部数据或定位条目，适合在已有模型/选项中查找目标。"
    if decl.startswith("static "):
        return "静态辅助接口，可不创建组件实例直接获取工具数据或弹出命令式界面。"
    return "公开 C++ 接口，用于读取组件状态、执行组件行为或对接 Qt 布局/绘制体系。"


def property_description(prop: dict[str, str]) -> str:
    bits = [property_label(prop.get("name", ""))]
    if prop.get("read"):
        bits.append(f"读取：{prop['read']}")
    if prop.get("write"):
        bits.append(f"设置：{prop['write']}")
    if prop.get("notify"):
        bits.append(f"变化信号：{prop['notify']}")
    return "；".join(bits)


def api_meta(decl: str, class_name: str) -> str:
    params = method_params(decl)
    ret = return_type(decl, class_name)
    if ret == "void":
        ret_text = "无返回值"
    elif ret == "构造 / 析构":
        ret_text = ret
    else:
        ret_text = f"返回 {ret}"
    return f"参数：{params}<br>{ret_text}"


def component_keywords(component: dict[str, object]) -> str:
    parts = [
        str(component["name"]),
        str(component["category"]),
        str(component.get("ant", "")),
        str(component.get("note", "")),
    ]
    return " ".join(parts).lower()


def table_or_empty(headers: list[str], rows: list[list[str]], empty: str) -> str:
    if not rows:
        return f'<p class="empty">{esc(empty)}</p>'
    head = "".join(f"<th>{esc(header)}</th>" for header in headers)
    body = []
    for row in rows:
        body.append("<tr>" + "".join(f"<td>{cell}</td>" for cell in row) + "</tr>")
    return f"<table><thead><tr>{head}</tr></thead><tbody>{''.join(body)}</tbody></table>"


def render_component(component: dict[str, object], related_structs: list[dict[str, object]]) -> str:
    name = str(component["name"])
    category = str(component["category"])
    cn_name = component_cn_name(component)
    image = image_path_for_component(name)
    props = component["properties"]
    methods = component["methods"]
    slots = component["slots"]
    signals = component["signals"]
    keyword = esc(component_keywords(component))
    anchor = esc(name.lower())

    prop_rows = [
        [
            f"<code>{esc(prop['name'])}</code>",
            esc(property_description(prop)),
            f"<code>{esc(prop['type'])}</code>",
            "<br>".join(
                esc(part)
                for part in [
                    f"读 {prop['read']}" if prop.get("read") else "",
                    f"写 {prop['write']}" if prop.get("write") else "",
                    f"通知 {prop['notify']}" if prop.get("notify") else "",
                ]
                if part
            )
            or "只读 / 内部状态",
        ]
        for prop in props
    ]
    method_rows = [
        [f"<code>{esc(decl)}</code>", esc(api_description(decl, "method", name)), api_meta(decl, name)]
        for decl in methods
    ]
    slot_rows = [
        [f"<code>{esc(decl)}</code>", esc(api_description(decl, "method", name)), api_meta(decl, name)]
        for decl in slots
    ]
    signal_rows = [
        [f"<code>{esc(decl)}</code>", esc(api_description(decl, "signal", name)), api_meta(decl, name)]
        for decl in signals
    ]

    struct_html = ""
    if related_structs:
        items = []
        for struct in related_structs:
            fields = struct["fields"]
            field_html = "".join(f"<li><code>{esc(field)}</code></li>" for field in fields[:16])
            if len(fields) > 16:
                field_html += f"<li>另有 {len(fields) - 16} 个字段</li>"
            items.append(
                f'<details class="struct"><summary><code>{esc(struct["name"])}</code></summary>'
                f"<ul>{field_html}</ul></details>"
            )
        struct_html = f'<div class="api-block"><h4>相关数据结构</h4>{"".join(items)}</div>'

    api_count = len(props) + len(methods) + len(slots) + len(signals)
    ant = str(component.get("ant", "")) or "Qt / 本地扩展"
    rendering = str(component.get("rendering", "")) or "组件自有实现"
    note = str(component.get("note", "")) or "提供 Ant Design 风格的 Qt Widgets 控件能力。"
    usage = usage_code(component)

    return f"""
    <article class="component doc-card" id="{anchor}" data-category="{esc(category)}" data-keywords="{keyword}">
      <div class="component-head">
        <div>
          <p class="eyebrow">{esc(category)}</p>
          <h3>{esc(name)} <span>{esc(cn_name)}</span></h3>
          <p class="desc">{esc(note)}</p>
        </div>
        <div class="api-count"><strong>{api_count}</strong><span>API 条目</span></div>
      </div>
      <div class="component-body">
        <figure class="component-preview">
          <img src="{esc(image)}" alt="{esc(name)} 组件截图" loading="lazy">
        </figure>
        <div class="usage-panel">
          <h4>何时使用</h4>
          <p>{esc(CATEGORY_INTRO.get(category, '用于 Qt Widgets 应用中复用 Ant Design 风格能力。'))} {esc(note)}</p>
          <h4>基本用法</h4>
          <pre><code>{esc(usage)}</code></pre>
        </div>
      </div>
      <div class="meta-grid">
        <div><span>头文件</span><code>{esc(component["header"])}</code></div>
        <div><span>继承</span><code>{esc(component["bases"])}</code></div>
        <div><span>Ant Design 对应</span><code>{esc(ant)}</code></div>
        <div><span>绘制方式</span><code>{esc(rendering)}</code></div>
      </div>
      <div class="api-block">
        <h4>属性</h4>
        {table_or_empty(["属性", "说明", "类型", "读写 / 通知"], prop_rows, "没有声明 Q_PROPERTY。")}
      </div>
      <div class="api-block">
        <h4>公开方法</h4>
        {table_or_empty(["声明", "用途", "参数 / 返回"], method_rows, "没有额外公开方法。")}
      </div>
      <div class="api-block">
        <h4>公开 slots</h4>
        {table_or_empty(["声明", "用途", "参数 / 返回"], slot_rows, "没有公开 slots。")}
      </div>
      <div class="api-block">
        <h4>信号</h4>
        {table_or_empty(["声明", "触发时机", "参数"], signal_rows, "没有声明信号。")}
      </div>
      {struct_html}
    </article>
    """


def render_alias(alias: dict[str, str]) -> str:
    return f"""
    <article class="component alias-card doc-card" id="{esc(alias['alias'].lower())}" data-category="Qt 风格别名"
      data-keywords="{esc((alias['alias'] + ' ' + alias['target']).lower())}">
      <div class="component-head">
        <div>
          <p class="eyebrow">Qt 风格别名</p>
          <h3>{esc(alias['alias'])}</h3>
          <p class="desc">轻量类型别名，方便按 Qt 原生命名习惯引用对应 Ant 控件。</p>
        </div>
      </div>
      <div class="meta-grid">
        <div><span>头文件</span><code>{esc(alias['header'])}</code></div>
        <div><span>等价类型</span><code>{esc(alias['target'])}</code></div>
      </div>
    </article>
    """


def render_gallery_card(component: dict[str, object]) -> str:
    name = str(component["name"])
    cn_name = component_cn_name(component)
    note = str(component.get("note", "")) or "提供 Ant Design 风格的 Qt Widgets 控件能力。"
    image = image_path_for_component(name)
    ant = str(component.get("ant", "")) or "Qt 扩展"
    api_count = (
        len(component["properties"])
        + len(component["methods"])
        + len(component["slots"])
        + len(component["signals"])
    )
    return f"""
          <a class="overview-card component-card" href="#{esc(name.lower())}"
             data-category="{esc(component['category'])}"
             data-keywords="{esc(component_keywords(component))}">
            <div class="overview-card-title">
              <strong>{esc(name)} <span>{esc(cn_name)}</span></strong>
              <small>{esc(ant)}</small>
            </div>
            <div class="overview-card-art"><img src="{esc(image)}" alt="{esc(name)} 示例截图" loading="lazy"></div>
            <p>{esc(note)}</p>
            <em>{api_count} API</em>
          </a>
    """


def build_data() -> tuple[list[dict[str, object]], list[dict[str, str]], list[dict[str, object]]]:
    metadata = parse_agents_metadata()
    structs_by_header: dict[str, list[dict[str, object]]] = {}
    components: list[dict[str, object]] = []
    aliases: list[dict[str, str]] = []

    for path in sorted(WIDGETS_DIR.glob("Ant*.h")):
        if path.name == "AntSelectPopup.h":
            continue
        text = path.read_text(encoding="utf-8")
        alias = parse_alias(text)
        if alias:
            aliases.append(
                {
                    "alias": alias[0],
                    "target": alias[1],
                    "header": path.relative_to(ROOT).as_posix(),
                }
            )
            continue
        structs = parse_structs(text)
        structs_by_header[path.relative_to(ROOT).as_posix()] = structs
        header_stem = path.stem
        for cls in parse_classes(path):
            cls.update(infer_component_metadata(str(cls["name"]), header_stem, metadata))
            components.append(cls)

    category_rank = {category: index for index, category in enumerate(CATEGORY_ORDER)}
    components.sort(key=lambda item: (category_rank.get(str(item["category"]), 99), str(item["name"])))
    aliases.sort(key=lambda item: item["alias"])
    enums = parse_ant_enums()
    return components, aliases, enums


def render_html(components: list[dict[str, object]], aliases: list[dict[str, str]], enums: list[dict[str, object]]) -> str:
    by_category: dict[str, list[dict[str, object]]] = {category: [] for category in CATEGORY_ORDER}
    for component in components:
        by_category.setdefault(str(component["category"]), []).append(component)

    nav_links = []
    component_nav = []
    gallery_html = []
    for category in CATEGORY_ORDER:
        count = len(by_category.get(category, [])) if category != "Qt 风格别名" else len(aliases)
        if count:
            nav_links.append(f'<a href="#overview-{esc(category)}"><span>{esc(category)}</span><b>{count}</b></a>')
        items = by_category.get(category, [])
        if category != "Qt 风格别名" and items:
            component_nav.append(f'<div class="side-group"><p>{esc(category)}</p>')
            for component in items:
                component_nav.append(
                    f'<a href="#{esc(str(component["name"]).lower())}"><span>{esc(component["name"])}</span>'
                    f'<small>{esc(component_cn_name(component))}</small></a>'
                )
            component_nav.append("</div>")

    for category in CATEGORY_ORDER:
        if category == "Qt 风格别名":
            continue
        items = by_category.get(category, [])
        if not items:
            continue
        cards = "".join(render_gallery_card(component) for component in items)
        gallery_html.append(
            f'<section class="overview-section" id="overview-{esc(category)}">'
            f'<div class="overview-section-head"><h2>{esc(category)} <span>{len(items)}</span></h2>'
            f'<p>{esc(CATEGORY_INTRO.get(category, "公开 API 分类。"))}</p></div>'
            f'<div class="overview-grid">{cards}</div></section>'
        )

    structs_by_header: dict[str, list[dict[str, object]]] = {}
    for path in sorted(WIDGETS_DIR.glob("Ant*.h")):
        if path.name == "AntSelectPopup.h":
            continue
        structs_by_header[path.relative_to(ROOT).as_posix()] = parse_structs(path.read_text(encoding="utf-8"))

    component_html = []
    for category in CATEGORY_ORDER:
        if category == "Qt 风格别名":
            continue
        items = by_category.get(category, [])
        if not items:
            continue
        component_html.append(f'<h2 class="section-title">{esc(category)}</h2>')
        for component in items:
            component_html.append(render_component(component, structs_by_header.get(str(component["header"]), [])))

    if aliases:
        component_html.append('<h2 class="section-title">Qt 风格别名</h2>')
        component_html.extend(render_alias(alias) for alias in aliases)

    enum_cards = []
    for enum in enums:
        values = ", ".join(f"<code>{esc(value)}</code>" for value in enum["values"])
        enum_cards.append(f"<li><strong>{esc(enum['name'])}</strong><span>{values}</span></li>")

    generated = _dt.date.today().isoformat()
    total_api = sum(
        len(component["properties"]) + len(component["methods"]) + len(component["slots"]) + len(component["signals"])
        for component in components
    )

    return f"""<!doctype html>
<html lang="zh-CN">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>qt-ant-design 组件与 API 总览</title>
  <link rel="icon" type="image/svg+xml" href="../assets/qt-ant-design-icon.svg">
  <link rel="alternate icon" type="image/png" href="../assets/qt-ant-design-icon.png">
  <link rel="apple-touch-icon" href="../assets/qt-ant-design-icon.png">
  <style>
    :root {{
      color-scheme: light;
      --ant-blue: #1677ff;
      --ant-blue-bg: #e6f4ff;
      --ant-blue-border: #91caff;
      --text: rgba(0, 0, 0, 0.88);
      --muted: rgba(0, 0, 0, 0.45);
      --subtle: rgba(0, 0, 0, 0.65);
      --line: #f0f0f0;
      --line-strong: #d9d9d9;
      --bg: #ffffff;
      --surface: #ffffff;
      --surface-2: #fafafa;
      --radius: 8px;
      --shadow: 0 6px 16px rgba(0, 0, 0, 0.08);
    }}
    * {{ box-sizing: border-box; }}
    html {{ scroll-behavior: smooth; }}
    body {{
      margin: 0;
      font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", "PingFang SC", "Microsoft YaHei", "Helvetica Neue", Arial, sans-serif;
      color: var(--text);
      background: var(--bg);
      font-size: 14px;
    }}
    a {{ color: inherit; text-decoration: none; }}
    a:hover {{ color: var(--ant-blue); }}
    code {{
      font-family: "SFMono-Regular", Consolas, "Liberation Mono", Menlo, monospace;
      font-size: 12px;
      color: #c41d7f;
      background: #fff0f6;
      border: 1px solid #ffd6e7;
      border-radius: 4px;
      padding: 2px 5px;
      white-space: normal;
      word-break: break-word;
    }}
    pre code {{
      display: block;
      color: rgba(0, 0, 0, 0.82);
      background: #f6f8fa;
      border: 1px solid var(--line);
      padding: 14px 16px;
      overflow-x: auto;
      line-height: 1.65;
    }}
    .topbar {{
      position: sticky;
      top: 0;
      z-index: 20;
      height: 64px;
      display: flex;
      align-items: center;
      justify-content: space-between;
      padding: 0 32px;
      background: rgba(255, 255, 255, 0.96);
      border-bottom: 1px solid var(--line);
      backdrop-filter: blur(8px);
    }}
    .brand {{
      display: flex;
      align-items: center;
      gap: 10px;
      min-width: 220px;
    }}
    .brand img {{ width: 32px; height: 32px; }}
    .brand strong {{ font-size: 18px; font-weight: 700; }}
    .topnav {{
      display: flex;
      align-items: center;
      gap: 28px;
      color: var(--subtle);
    }}
    .topnav a.active {{
      color: var(--ant-blue);
      position: relative;
    }}
    .topnav a.active::after {{
      content: "";
      position: absolute;
      left: 0;
      right: 0;
      bottom: -22px;
      height: 2px;
      background: var(--ant-blue);
    }}
    .version-pill {{
      border: 1px solid var(--line);
      background: var(--surface-2);
      border-radius: 6px;
      padding: 4px 10px;
      color: var(--subtle);
    }}
    .layout {{
      display: grid;
      grid-template-columns: clamp(240px, 16vw, 280px) minmax(0, 1fr) clamp(140px, 10vw, 180px);
      gap: 24px;
      width: 100%;
      max-width: none;
      margin: 0;
      padding: 32px 20px 64px;
    }}
    .sidebar {{
      position: sticky;
      top: 88px;
      height: calc(100vh - 104px);
      overflow: auto;
      padding: 0 14px 24px 0;
      border-right: 1px solid var(--line);
    }}
    .nav-title {{
      color: var(--muted);
      font-size: 13px;
      margin: 0 0 10px;
    }}
    .category-nav a {{
      display: flex;
      justify-content: space-between;
      align-items: center;
      padding: 10px 14px;
      border-radius: 6px;
      color: var(--subtle);
      margin-bottom: 4px;
    }}
    .category-nav a:hover,
    .category-nav a:first-child {{
      color: var(--ant-blue);
      background: var(--ant-blue-bg);
    }}
    .category-nav b {{
      font-size: 13px;
      font-weight: 500;
      color: var(--muted);
    }}
    .side-group {{
      margin-top: 22px;
      padding-top: 12px;
      border-top: 1px solid var(--line);
    }}
    .side-group p {{
      color: var(--muted);
      margin: 0 0 8px;
      font-size: 13px;
    }}
    .side-group a {{
      display: grid;
      grid-template-columns: minmax(0, 1fr) auto;
      gap: 8px;
      padding: 7px 12px;
      border-radius: 6px;
      color: var(--subtle);
    }}
    .side-group small {{
      color: var(--muted);
      white-space: nowrap;
    }}
    main {{ min-width: 0; }}
    .hero {{
      padding-bottom: 30px;
      border-bottom: 1px solid var(--line);
    }}
    .hero h1 {{
      margin: 0 0 18px;
      font-size: 38px;
      line-height: 1.25;
      font-weight: 700;
      letter-spacing: 0;
    }}
    .hero p {{ margin: 0; color: var(--subtle); line-height: 1.9; }}
    .hero .eyebrow {{
      display: inline-block;
      margin-bottom: 12px;
      color: var(--ant-blue);
      background: var(--ant-blue-bg);
      border: 1px solid #bae0ff;
      border-radius: 4px;
      padding: 2px 8px;
      font-size: 13px;
    }}
    .toolbar {{
      display: flex;
      align-items: center;
      gap: 16px;
      margin-top: 28px;
      border-top: 1px solid var(--line);
      border-bottom: 1px solid var(--line);
      height: 64px;
    }}
    .search {{
      width: 100%;
      height: 48px;
      border: 0;
      padding: 0 4px;
      font-size: 20px;
      outline: none;
      color: var(--text);
      background: transparent;
    }}
    .search::placeholder {{ color: rgba(0, 0, 0, 0.28); }}
    .source-links {{
      display: flex;
      gap: 10px;
      flex-wrap: wrap;
      margin-top: 18px;
    }}
    .source-links a {{
      border: 1px solid var(--line);
      background: white;
      border-radius: 6px;
      padding: 7px 12px;
      color: var(--subtle);
      font-size: 13px;
    }}
    .stats-line {{
      display: flex;
      flex-wrap: wrap;
      gap: 8px;
      margin-top: 18px;
    }}
    .stats-line span {{
      color: var(--subtle);
      background: var(--surface-2);
      border: 1px solid var(--line);
      border-radius: 999px;
      padding: 4px 10px;
      font-size: 13px;
    }}
    .overview-section {{ padding-top: 40px; }}
    .overview-section-head {{
      display: flex;
      align-items: baseline;
      justify-content: space-between;
      gap: 18px;
      margin-bottom: 18px;
    }}
    .overview-section h2,
    .section-title {{
      margin: 0;
      font-size: 26px;
      line-height: 1.35;
      font-weight: 700;
      letter-spacing: 0;
    }}
    .overview-section h2 span {{
      display: inline-flex;
      align-items: center;
      justify-content: center;
      min-width: 24px;
      height: 24px;
      margin-left: 6px;
      border-radius: 6px;
      background: var(--surface-2);
      color: var(--muted);
      font-size: 14px;
      font-weight: 500;
    }}
    .overview-section-head p {{
      margin: 0;
      color: var(--muted);
      line-height: 1.7;
    }}
    .overview-grid {{
      display: grid;
      grid-template-columns: repeat(auto-fill, minmax(230px, 1fr));
      gap: 24px;
    }}
    .overview-card {{
      position: relative;
      display: block;
      background: var(--surface);
      border: 1px solid var(--line);
      border-radius: var(--radius);
      overflow: hidden;
      color: var(--text);
      transition: border-color .2s, box-shadow .2s, transform .2s;
    }}
    .overview-card:hover {{
      border-color: var(--ant-blue-border);
      box-shadow: var(--shadow);
      transform: translateY(-2px);
    }}
    .overview-card-title {{
      height: 56px;
      padding: 14px 16px;
      border-bottom: 1px solid var(--line);
    }}
    .overview-card-title strong {{
      display: block;
      font-size: 15px;
      font-weight: 600;
    }}
    .overview-card-title span,
    .component h3 span {{
      color: var(--subtle);
      font-weight: 400;
      margin-left: 4px;
    }}
    .overview-card-title small {{
      display: block;
      color: var(--muted);
      margin-top: 4px;
    }}
    .overview-card-art {{
      height: 150px;
      display: flex;
      align-items: center;
      justify-content: center;
      background: #fff;
    }}
    .overview-card-art img {{
      max-width: 88%;
      max-height: 112px;
      object-fit: contain;
    }}
    .overview-card p {{
      height: 64px;
      margin: 0;
      padding: 0 16px 14px;
      color: var(--muted);
      line-height: 1.55;
      font-size: 13px;
      overflow: hidden;
    }}
    .overview-card em {{
      position: absolute;
      right: 12px;
      bottom: 10px;
      color: var(--ant-blue);
      font-style: normal;
      font-size: 12px;
      background: #fff;
      padding-left: 8px;
    }}
    .detail-intro {{
      margin-top: 54px;
      padding-top: 32px;
      border-top: 1px solid var(--line);
    }}
    .detail-intro p {{ color: var(--muted); line-height: 1.8; }}
    .component[hidden],
    .overview-card[hidden] {{ display: none; }}
    .doc-card {{
      background: var(--surface);
      border: 1px solid var(--line);
      border-radius: var(--radius);
      margin: 24px 0 32px;
      overflow: hidden;
      content-visibility: auto;
      contain-intrinsic-size: 900px;
    }}
    .component-head {{
      display: flex;
      justify-content: space-between;
      gap: 20px;
      padding: 24px;
      border-bottom: 1px solid var(--line);
      background: #fff;
    }}
    .component h3 {{ margin: 4px 0 10px; font-size: 28px; line-height: 1.3; }}
    .eyebrow {{ margin: 0; color: var(--ant-blue); font-size: 13px; }}
    .desc {{ margin: 0; color: var(--subtle); line-height: 1.8; }}
    .api-count {{
      min-width: 96px;
      border: 1px solid #bae0ff;
      border-radius: 6px;
      background: var(--ant-blue-bg);
      padding: 10px 12px;
      text-align: center;
      align-self: flex-start;
    }}
    .api-count strong {{ display: block; color: var(--ant-blue); font-size: 22px; }}
    .api-count span {{ color: var(--muted); font-size: 12px; }}
    .component-body {{
      display: grid;
      grid-template-columns: minmax(240px, 38%) minmax(0, 1fr);
      gap: 24px;
      padding: 24px;
      border-bottom: 1px solid var(--line);
      background: #fff;
    }}
    .component-preview {{
      margin: 0;
      min-height: 220px;
      display: flex;
      align-items: center;
      justify-content: center;
      border: 1px solid var(--line);
      border-radius: var(--radius);
      background: #fff;
    }}
    .component-preview img {{
      max-width: 92%;
      max-height: 190px;
      object-fit: contain;
    }}
    .usage-panel h4 {{
      margin: 0 0 8px;
      font-size: 16px;
    }}
    .usage-panel p {{
      margin: 0 0 18px;
      color: var(--subtle);
      line-height: 1.8;
    }}
    .usage-panel pre {{ margin: 0; }}
    .meta-grid {{
      display: grid;
      grid-template-columns: repeat(4, minmax(0, 1fr));
      gap: 1px;
      background: var(--line);
      border-bottom: 1px solid var(--line);
    }}
    .meta-grid div {{ background: var(--surface-2); padding: 12px 14px; min-width: 0; }}
    .meta-grid span {{ display: block; color: var(--muted); font-size: 12px; margin-bottom: 6px; }}
    .api-block {{ padding: 22px 24px 4px; }}
    .api-block h4 {{ margin: 0 0 12px; font-size: 18px; }}
    table {{ width: 100%; border-collapse: collapse; margin-bottom: 18px; font-size: 13px; }}
    th, td {{ border: 1px solid var(--line); padding: 11px 13px; text-align: left; vertical-align: top; line-height: 1.65; }}
    th {{ background: var(--surface-2); color: var(--text); font-weight: 600; }}
    td:nth-child(1) {{ width: 38%; }}
    .empty {{ color: var(--muted); margin: 0 0 12px; font-size: 13px; }}
    .struct {{
      border: 1px solid var(--line);
      border-radius: 6px;
      margin-bottom: 10px;
      padding: 10px 12px;
      background: var(--surface-2);
    }}
    .struct summary {{ cursor: pointer; }}
    .struct ul {{ margin: 10px 0 0; padding-left: 20px; }}
    .enum-section {{
      background: var(--surface);
      border: 1px solid var(--line);
      border-radius: var(--radius);
      padding: 24px;
      margin-top: 36px;
    }}
    .enum-section h2 {{ margin: 0 0 12px; }}
    .enum-section ul {{ list-style: none; padding: 0; margin: 0; display: grid; gap: 8px; }}
    .enum-section li {{
      display: grid;
      grid-template-columns: 180px minmax(0, 1fr);
      gap: 10px;
      border-top: 1px solid var(--line);
      padding-top: 8px;
    }}
    .enum-section span {{ display: flex; flex-wrap: wrap; gap: 5px; }}
    .footer {{
      color: var(--muted);
      font-size: 13px;
      padding: 24px 0 0;
      border-top: 1px solid var(--line);
    }}
    .toc {{
      position: sticky;
      top: 88px;
      align-self: start;
      border-left: 1px solid var(--line);
      padding-left: 18px;
      color: var(--muted);
      font-size: 13px;
    }}
    .toc strong {{
      display: block;
      color: var(--text);
      margin-bottom: 10px;
    }}
    .toc a {{
      display: block;
      padding: 6px 0;
      color: var(--muted);
    }}
    @media (max-width: 1180px) {{
      .layout {{ grid-template-columns: 1fr; }}
      .sidebar, .toc {{ position: static; height: auto; border: 0; padding: 0; }}
      .overview-grid {{ grid-template-columns: repeat(2, minmax(0, 1fr)); }}
      .meta-grid {{ grid-template-columns: repeat(2, minmax(0, 1fr)); }}
      .component-body {{ grid-template-columns: 1fr; }}
    }}
    @media (max-width: 680px) {{
      .topbar {{ padding: 0 16px; }}
      .brand {{ min-width: 0; }}
      .topnav {{ display: none; }}
      .layout {{ padding: 22px 16px 44px; }}
      .sidebar, .toc {{ display: none; }}
      .overview-grid, .meta-grid {{ grid-template-columns: 1fr; }}
      .component-head {{ display: block; }}
      .api-count {{ margin-top: 12px; }}
      table {{ display: block; overflow-x: auto; }}
      .enum-section li {{ grid-template-columns: 1fr; }}
    }}
  </style>
</head>
<body>
  <header class="topbar">
    <a class="brand" href="#top">
      <img src="../assets/qt-ant-design-icon.png" alt="qt-ant-design logo">
      <strong>qt-ant-design</strong>
    </a>
    <nav class="topnav" aria-label="主导航">
      <a href="./project-status.md">设计</a>
      <a href="./qt-control-porting-guidelines.md">开发</a>
      <a class="active" href="#overview">组件</a>
      <a href="./visual-audit.md">视觉审计</a>
      <a href="https://ant.design/components/overview-cn/">Ant Design</a>
      <span class="version-pill">{esc(generated)}</span>
    </nav>
  </header>
  <div class="layout">
    <aside class="sidebar">
      <p class="nav-title">组件分类</p>
      <nav class="category-nav">{"".join(nav_links)}</nav>
      {"".join(component_nav)}
    </aside>
    <main>
      <section class="hero" id="top">
        <span class="eyebrow">本地文档</span>
        <h1>组件总览</h1>
        <p>
          qt-ant-design 为 Qt Widgets 提供 Ant Design 风格组件。页面参考 Ant Design 官方文档的组件总览结构：
          左侧按分类导航，正文先浏览组件卡片，再查看每个控件的使用说明、截图、头文件、继承关系、属性、方法、slots、信号和相关数据结构。
        </p>
        <div class="stats-line">
          <span>{len(components)} 个公开 API 类</span>
          <span>{len(aliases)} 个 Qt 风格别名</span>
          <span>{total_api} 个属性 / 方法 / 信号条目</span>
          <span>{len(enums)} 个共享枚举</span>
        </div>
        <div class="toolbar">
          <input class="search" id="search" placeholder="搜索组件、API、分类，例如 AntSelect / valueChanged / 数据录入">
          <span aria-hidden="true">⌕</span>
        </div>
        <div class="source-links">
          <a href="https://ant.design/components/overview-cn/">Ant Design 官方总览</a>
          <a href="./project-status.md">项目状态</a>
          <a href="./visual-audit.md">视觉审计</a>
        </div>
      </section>
      <section class="overview" id="overview">{"".join(gallery_html)}</section>
      <section class="detail-intro" id="api-details">
        <h2 class="section-title">组件详细说明与 API</h2>
        <p>下面的接口表来自公开头文件，说明文本按属性名、接口行为、参数与返回值生成，适合作为本地使用手册和 Qt 替代控件对照文档。</p>
      </section>
      <section class="content" id="components">{"".join(component_html)}
        <section class="enum-section" id="shared-enums">
          <h2>共享枚举 Ant::*</h2>
          <ul>{"".join(enum_cards)}</ul>
        </section>
      </section>
      <p class="footer">生成时间：{esc(generated)}。源数据：<code>src/widgets/*.h</code>、<code>src/core/AntTypes.h</code>、<code>AGENTS.md</code>。</p>
    </main>
    <aside class="toc">
      <strong>本页目录</strong>
      <a href="#overview">组件总览</a>
      <a href="#api-details">详细说明与 API</a>
      <a href="#shared-enums">共享枚举</a>
    </aside>
  </div>
  <script>
    const input = document.getElementById('search');
    const components = Array.from(document.querySelectorAll('.component'));
    const overviewCards = Array.from(document.querySelectorAll('.component-card'));
    const sectionTitles = Array.from(document.querySelectorAll('.section-title'));
    const overviewSections = Array.from(document.querySelectorAll('.overview-section'));
    function applyFilter() {{
      const term = input.value.trim().toLowerCase();
      components.forEach(card => {{
        const keywords = ((card.dataset.keywords || '') + ' ' + card.textContent).toLowerCase();
        card.hidden = term && !keywords.includes(term);
      }});
      overviewCards.forEach(card => {{
        const keywords = ((card.dataset.keywords || '') + ' ' + card.textContent).toLowerCase();
        card.hidden = term && !keywords.includes(term);
      }});
      overviewSections.forEach(section => {{
        const cards = Array.from(section.querySelectorAll('.component-card'));
        section.hidden = term && !cards.some(card => !card.hidden);
      }});
      sectionTitles.forEach(title => {{
        let next = title.nextElementSibling;
        let visible = false;
        while (next && !next.classList.contains('section-title')) {{
          if (next.classList && next.classList.contains('component') && !next.hidden) {{
            visible = true;
            break;
          }}
          next = next.nextElementSibling;
        }}
        title.hidden = term && !visible;
      }});
    }}
    input.addEventListener('input', applyFilter);
  </script>
</body>
</html>
"""


def main() -> None:
    components, aliases, enums = build_data()
    OUTPUT.write_text(render_html(components, aliases, enums), encoding="utf-8", newline="\n")
    print(f"Generated {OUTPUT.relative_to(ROOT)}")
    print(f"API classes: {len(components)}, aliases: {len(aliases)}, enums: {len(enums)}")


if __name__ == "__main__":
    main()
