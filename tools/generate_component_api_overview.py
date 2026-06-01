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
    "danger": "危险状态，启用后使用错误/危险色强调高风险操作。",
    "ghost": "幽灵按钮状态，启用后按钮背景透明并适合叠放在有色背景上。",
    "block": "块级按钮状态，启用后按钮横向填满父布局可用宽度。",
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
    "strong": "是否使用加粗文本样式。",
    "underline": "是否使用下划线文本样式。",
    "delete_": "是否使用删除线文本样式。",
    "code": "是否使用行内代码文本样式。",
    "mark": "是否使用高亮标记文本样式。",
    "italic": "是否使用斜体文本样式。",
    "copyable": "是否显示复制入口并允许一键复制文本。",
    "ellipsis": "是否在文本溢出时使用省略显示。",
    "ellipsisRows": "多行省略时允许显示的最大行数。",
    "alignment": "内容对齐方式。",
    "autoPlay": "是否自动播放轮播内容。",
    "interval": "自动轮播间隔。",
    "arrowsVisible": "是否显示手动切换箭头。",
    "dotsVisible": "是否显示轮播指示点。",
    "sourceItems": "Source 源列表中的字符串项，表示尚未转移到 Target 目标侧的数据。",
    "targetItems": "Target 目标列表中的字符串项，表示已经从 Source 源侧转移过来的数据。",
    "manualNavigationEnabled": "是否允许通过上一张、下一张按钮或指示点手动切换轮播页。",
    "mouseDragScrollEnabled": "是否允许在滚动区域中按住鼠标拖动内容滚动。",
    "autoExclusive": "是否启用互斥选择，启用后同一导航容器内只保留一个当前项。",
    "multiple": "是否允许多选，适用于列表、选择器或导航项的多项选择场景。",
    "selectedKey": "当前选中的 key，用于菜单、导航或选项集合的业务标识同步。",
    "openKeys": "当前展开的 key 列表，用于控制菜单、树或折叠项的展开状态。",
    "currentPage": "当前页码，用于分页控件的数据页同步。",
    "currentPageIndex": "当前页面索引，用于页面栈、轮播或分页组件定位当前页。",
    "currentPageKey": "当前页面 key，用于通过稳定业务标识切换页面。",
    "currentData": "当前选中项携带的 QVariant 数据。",
    "options": "可选项集合，用于选择器、级联选择或自动完成的候选数据。",
    "itemLabels": "条目显示文本列表，用于批量设置列表、分段控制或选项控件的标签。",
    "columns": "表格或描述列表的列定义。",
    "rows": "表格、列表或骨架屏的行数配置。",
    "rowSelection": "表格行选择配置，用于启用或读取行级选中状态。",
    "treeCheckable": "树节点是否显示复选框。",
    "showSearch": "是否显示搜索入口，用于在选项或树节点中快速过滤。",
    "showQuickJumper": "分页器是否显示快速跳页输入框。",
    "showSizeChanger": "分页器是否显示每页条数选择器。",
    "showTotal": "是否显示总数文案。",
    "showZero": "数量为 0 时是否仍显示徽标。",
    "checkedText": "开关或选项选中时显示的文本。",
    "uncheckedText": "开关或选项未选中时显示的文本。",
    "okText": "确认按钮文本。",
    "cancelText": "取消按钮文本。",
    "closeConfirmationTitle": "关闭确认弹窗标题。",
    "closeConfirmationContent": "关闭确认弹窗正文。",
    "closeConfirmationOkText": "关闭确认弹窗确认按钮文本。",
    "closeConfirmationCancelText": "关闭确认弹窗取消按钮文本。",
    "preview": "是否启用图片预览能力。",
    "imagePath": "图片文件路径，用于加载本地图片内容。",
    "src": "图片或媒体资源地址。",
    "alt": "图片替代文本。",
    "href": "链接地址，点击 Link 类型文本时会发出 linkActivated。",
    "passwordMode": "是否按密码输入框方式隐藏文本。",
    "readOnly": "是否只读，只读时可选择文本但不允许编辑。",
    "maxLength": "允许输入的最大字符数。",
    "echoMode": "输入框回显模式，用于普通文本、密码等显示方式。",
    "caseSensitivity": "搜索或匹配时使用的大小写敏感策略。",
    "required": "表单项是否必填。",
    "validateStatus": "表单校验状态，用于显示成功、警告或错误反馈。",
    "helpText": "表单项辅助说明或校验错误文本。",
    "labelAlign": "表单标签对齐方式。",
    "labelWidth": "表单标签列宽度。",
    "colon": "表单标签后是否显示冒号。",
    "formLayout": "表单布局方式，用于水平、垂直或行内排列。",
    "pickerSize": "日期或时间选择器尺寸。",
    "selectedDate": "当前选中的日期。",
    "selectedTime": "当前选中的时间。",
    "minimumDate": "允许选择的最早日期。",
    "maximumDate": "允许选择的最晚日期。",
    "minimumTime": "允许选择的最早时间。",
    "maximumTime": "允许选择的最晚时间。",
    "displayFormat": "日期、时间或数值的显示格式。",
    "hourStep": "小时列步进。",
    "minuteStep": "分钟列步进。",
    "secondStep": "秒钟列步进。",
    "calendarMode": "日历面板模式，用于日、月、年视图切换。",
    "countdownMode": "是否按倒计时方式显示统计数值。",
    "countdownFormat": "倒计时文本格式。",
    "percent": "进度百分比。",
    "progressType": "进度条类型，用于线形、圆形或仪表盘展示。",
    "strokeWidth": "线条宽度，用于进度、二维码或描边类绘制。",
    "included": "滑块是否显示已选范围轨道。",
    "reverse": "是否反向显示或反向排列。",
    "pageStep": "键盘 PageUp/PageDown 或分页滚动的步长。",
    "singleStep": "单次键盘或按钮调节的步长。",
    "allowHalf": "评分是否允许半星选择。",
    "rateSize": "评分图标尺寸。",
    "maxTagCount": "多选选择器最多直接显示的标签数。",
    "maxVisibleItems": "最多可见条目数，超过后进入折叠、滚动或省略显示。",
    "overflowCount": "徽标超过该数值后使用加号形式显示。",
    "draggerMode": "上传控件是否使用拖拽上传区域。",
    "listType": "上传列表展示类型。",
    "fileMode": "文件选择模式。",
    "dialogWidth": "对话框宽度。",
    "drawerWidth": "抽屉宽度。",
    "drawerHeight": "抽屉高度。",
    "maskClosable": "点击遮罩时是否关闭弹层。",
    "keyboard": "是否允许通过键盘快捷键关闭或操作弹层。",
    "duration": "提示、通知或动画的持续时间。",
    "pauseOnHover": "鼠标悬停时是否暂停自动关闭或自动播放计时。",
    "openDelay": "悬停触发弹层前的延迟时间。",
    "delay": "延迟显示或延迟触发的时间。",
    "dropGuideEnabled": "是否启用停靠布局的拖放引导。",
    "dropGuideVisible": "当前拖放引导是否可见。",
    "alwaysOnTop": "窗口是否保持置顶。",
    "cornerRadius": "窗口或面板圆角半径。",
    "titleBarVisible": "是否显示自定义标题栏。",
    "titleBarHeight": "自定义标题栏高度。",
    "pinButtonVisible": "标题栏置顶按钮是否可见。",
    "themeButtonVisible": "标题栏主题切换按钮是否可见。",
    "minimizeButtonVisible": "标题栏最小化按钮是否可见。",
    "maximizeButtonVisible": "标题栏最大化按钮是否可见。",
    "closeButtonVisible": "标题栏关闭按钮是否可见。",
    "closeConfirmationEnabled": "关闭窗口前是否弹出确认对话框。",
    "ribbonText": "徽标丝带显示文本。",
    "ribbonColor": "徽标丝带背景色。",
    "avatarVisible": "头像区域是否显示。",
    "avatarShape": "头像形状，用于圆形、方形等展示。",
    "avatarSize": "头像尺寸。",
    "imageVisible": "图片区域是否显示。",
    "imageSize": "图片显示尺寸。",
    "backgroundColor": "组件背景色覆盖值。",
    "primaryColor": "主色覆盖值。",
    "currentColor": "当前选中或当前状态使用的颜色。",
    "bgColor": "背景色。",
    "prefix": "前缀内容，通常显示在输入框、统计数值或文本前。",
    "suffix": "后缀内容，通常显示在输入框、统计数值或文本后。",
    "prefixText": "输入框前缀文本。",
    "suffixText": "输入框后缀文本。",
    "separator": "分隔符文本或图形。",
    "groupSeparator": "数字千分位分隔符。",
    "spacing": "子项间距。",
    "gap": "弹性布局或栅格中的间隔。",
    "gutter": "栅格列间距。",
    "span": "栅格列占据的 24 栅格份数。",
    "offset": "栅格列左侧偏移份数。",
    "wrap": "内容是否允许换行。",
    "vertical": "是否使用垂直排列。",
    "collapsible": "是否允许折叠。",
    "collapsed": "当前是否处于折叠状态。",
    "collapsedWidth": "折叠后的侧边栏宽度。",
    "accordion": "折叠面板是否使用手风琴模式，同一时间只展开一个面板。",
    "activeKey": "当前激活面板或标签的 key。",
    "activeIndex": "当前激活项索引。",
    "active": "是否处于激活状态。",
    "animated": "是否启用切换动画。",
    "transitionProgress": "切换动画进度，供样式绘制和测试观察使用。",
    "indicatorRect": "当前指示器矩形区域，供样式绘制和测试观察使用。",
    "handleProgress": "滑块手柄交互动画进度。",
    "handleScale": "滑块手柄缩放比例。",
    "focusProgress": "聚焦动画进度。",
    "controlsProgress": "控制区显隐动画进度。",
    "pressProgress": "按压动画进度。",
    "arrowRotation": "下拉箭头旋转角度。",
    "sliderPosition": "滑块当前位置。",
    "revision": "内部修订号，用于内容变化后触发刷新或测试断言。",
    "spinnerAngle": "加载旋转图标当前角度，主要供样式绘制和动画测试使用。",
    "spinAngle": "图标旋转动画当前角度，主要供样式绘制和动画测试使用。",
    "progressRatio": "通知进度条当前比例，取值通常在 0 到 1 之间。",
    "tokens": "当前主题 token 引用，用于读取颜色、圆角、间距等设计变量。",
    "contentWidget": "对话框或窗口的内容宿主控件。",
    "titleBarRect": "标题栏整体矩形区域，供绘制和命中测试使用。",
    "titleBarTextRect": "标题栏文字矩形区域，供绘制和命中测试使用。",
    "titleBarCloseButtonRect": "标题栏关闭按钮矩形区域，供绘制和命中测试使用。",
    "buttonRect": "按钮绘制和命中测试使用的矩形区域。",
    "usesRoundedCorners": "当前窗口策略是否使用透明圆角路径。",
    "usesLegacyOpaquePath": "当前窗口策略是否使用 Windows 10 兼容的不透明路径。",
    "scrollTarget": "BackTop 或滚动联动时操作的目标滚动控件。",
    "scrollDuration": "回到顶部或滚动动画持续时间。",
}

PROPERTY_NOUNS = {
    "sourceItems": "Source 源列表的字符串项",
    "targetItems": "Target 目标列表的字符串项",
    "buttonType": "按钮视觉类型",
    "buttonSize": "按钮尺寸",
    "buttonShape": "按钮形状",
    "buttonIconType": "按钮图标类型",
    "buttonIconColor": "按钮图标颜色",
    "floatButtonType": "悬浮按钮类型",
    "floatButtonShape": "悬浮按钮形状",
    "tabsType": "标签页类型",
    "tabsSize": "标签页尺寸",
    "selectMode": "选择器模式",
    "selectSize": "选择器尺寸",
    "inputSize": "输入框尺寸",
    "segmentedSize": "分段控制器尺寸",
    "switchSize": "开关尺寸",
    "spinSize": "加载指示器尺寸",
    "tableSize": "表格尺寸",
    "listSize": "列表尺寸",
    "cardSize": "卡片尺寸",
    "qrSize": "二维码尺寸",
    "iconText": "图标旁文本",
    "twoToneColor": "双色图标主色",
    "titleLevel": "标题层级",
    "delete_": "删除线状态",
    "isOpen": "打开状态",
    "hasSider": "是否包含侧边栏",
    "sizeGrip": "右下角尺寸拖拽柄",
    "spinnerAngle": "加载旋转图标角度",
    "spinAngle": "图标旋转动画角度",
    "progressRatio": "进度比例",
    "contentWidget": "内容宿主控件",
    "buttonRect": "按钮矩形区域",
    "titleBarRect": "标题栏矩形区域",
    "titleBarTextRect": "标题栏文本矩形区域",
    "titleBarCloseButtonRect": "标题栏关闭按钮矩形区域",
    "usesRoundedCorners": "透明圆角路径状态",
    "usesLegacyOpaquePath": "兼容不透明路径状态",
    "scrollTarget": "滚动目标控件",
    "scrollDuration": "滚动动画持续时间",
    "iconName": "图标资源名称",
    "resolvedIconName": "实际解析到的图标资源名称",
    "customPrimaryPath": "自定义图标主路径",
    "customSecondaryPath": "自定义图标辅助路径",
    "affixedWidget": "需要固钉的目标控件",
    "isAffixed": "当前固钉状态",
    "rootWidget": "应用根控件",
    "feedbackHost": "反馈弹层宿主控件",
    "borderRadius": "边框圆角半径",
    "heightForWidth": "高度随宽度变化的布局计算",
    "hasHeightForWidth": "高度随宽度变化的布局能力",
    "watermarkFont": "水印字体配置",
    "dockWidgets": "已管理的停靠面板列表",
    "floatingDockWidgets": "当前浮动的停靠面板列表",
    "dockWidgetArea": "停靠面板所在区域",
    "tabifiedDockWidgets": "与指定停靠面板同组的标签页停靠面板",
    "dockWidgetTabIndex": "停靠面板标签页索引",
    "activeDropGuide": "当前命中的拖放引导位置",
    "dropPreviewRect": "拖放预览矩形区域",
    "isDockingSurfaceAvailable": "当前是否存在可停靠表面",
    "perspectiveNames": "已保存布局快照名称列表",
    "perspectiveState": "指定布局快照的序列化状态",
}

TOKEN_CN = {
    "accept": "确认",
    "active": "激活",
    "add": "添加",
    "align": "对齐",
    "alignment": "对齐方式",
    "allow": "允许",
    "always": "始终",
    "animated": "动画",
    "arrow": "箭头",
    "auto": "自动",
    "avatar": "头像",
    "back": "返回",
    "badge": "徽标",
    "bar": "栏",
    "bg": "背景",
    "block": "块级",
    "border": "边框",
    "bordered": "边框",
    "bottom": "底部",
    "button": "按钮",
    "calendar": "日历",
    "cancel": "取消",
    "card": "卡片",
    "case": "大小写",
    "centered": "居中",
    "check": "勾选",
    "checked": "选中",
    "child": "子项",
    "children": "子项",
    "circle": "圆形",
    "clear": "清空",
    "click": "点击",
    "clickable": "可点击",
    "close": "关闭",
    "collapsed": "折叠",
    "collapsible": "可折叠",
    "color": "颜色",
    "column": "列",
    "columns": "列",
    "confirmation": "确认",
    "content": "内容",
    "control": "控制",
    "controls": "控制区",
    "code": "行内代码",
    "copy": "复制",
    "copyable": "可复制",
    "count": "数量",
    "current": "当前",
    "custom": "自定义",
    "danger": "危险",
    "data": "数据",
    "date": "日期",
    "delay": "延迟",
    "delete": "删除线",
    "description": "描述",
    "dialog": "对话框",
    "direction": "方向",
    "disabled": "禁用",
    "display": "显示",
    "divider": "分割线",
    "dot": "圆点",
    "dots": "指示点",
    "drag": "拖拽",
    "dragger": "拖拽上传",
    "drawer": "抽屉",
    "drop": "拖放",
    "duration": "持续时间",
    "echo": "回显",
    "editable": "可编辑",
    "ellipsis": "省略",
    "enabled": "启用",
    "error": "错误",
    "exclusive": "互斥",
    "expand": "展开",
    "expanded": "展开",
    "extra": "额外内容",
    "file": "文件",
    "focus": "聚焦",
    "font": "字体",
    "form": "表单",
    "format": "格式",
    "ghost": "幽灵",
    "grip": "拖拽柄",
    "group": "分组",
    "guide": "引导",
    "gutter": "栅格间距",
    "handle": "手柄",
    "height": "高度",
    "help": "帮助",
    "hide": "隐藏",
    "hover": "悬停",
    "hovered": "悬停",
    "hoverable": "可悬停",
    "hour": "小时",
    "href": "链接地址",
    "icon": "图标",
    "image": "图片",
    "included": "包含范围",
    "index": "索引",
    "indent": "缩进",
    "info": "信息",
    "inline": "内联",
    "input": "输入",
    "interval": "间隔",
    "italic": "斜体",
    "item": "条目",
    "items": "条目列表",
    "key": "key",
    "keys": "key 列表",
    "keyboard": "键盘",
    "label": "标签",
    "labels": "标签列表",
    "layout": "布局",
    "length": "长度",
    "less": "精简",
    "line": "连接线",
    "list": "列表",
    "loading": "加载",
    "manual": "手动",
    "mark": "标记",
    "mask": "遮罩",
    "max": "最大",
    "maximum": "最大值",
    "message": "消息",
    "min": "最小",
    "minimum": "最小值",
    "minute": "分钟",
    "mode": "模式",
    "mouse": "鼠标",
    "multiple": "多选",
    "navigation": "导航",
    "now": "现在",
    "offset": "偏移",
    "ok": "确认",
    "open": "打开",
    "option": "选项",
    "options": "选项集合",
    "orientation": "方向",
    "overflow": "溢出",
    "page": "页",
    "pagination": "分页",
    "panel": "面板",
    "paragraph": "段落",
    "password": "密码",
    "pause": "暂停",
    "percent": "百分比",
    "picker": "选择器",
    "pin": "置顶",
    "placeholder": "占位文本",
    "placement": "位置",
    "plain": "纯文本",
    "prefix": "前缀",
    "press": "按压",
    "pressed": "按压",
    "preview": "预览",
    "primary": "主色",
    "progress": "进度",
    "ratio": "比例",
    "quick": "快速",
    "range": "范围",
    "rate": "评分",
    "read": "只读",
    "required": "必填",
    "reverse": "反向",
    "revision": "修订号",
    "rect": "矩形区域",
    "ribbon": "丝带",
    "rotate": "旋转角度",
    "rotation": "旋转角度",
    "round": "圆角",
    "row": "行",
    "rows": "行数",
    "scale": "缩放",
    "scroll": "滚动",
    "search": "搜索",
    "second": "秒",
    "select": "选择",
    "selected": "选中",
    "selectable": "可选择",
    "selection": "选择",
    "separator": "分隔符",
    "shape": "形状",
    "show": "显示",
    "sider": "侧边栏",
    "simple": "简洁",
    "single": "单次",
    "size": "尺寸",
    "slider": "滑块",
    "source": "Source 源侧",
    "spacing": "间距",
    "span": "栅格跨度",
    "spin": "旋转",
    "spinner": "加载旋转图标",
    "spinning": "加载中",
    "split": "分割线",
    "src": "资源地址",
    "start": "开始",
    "state": "状态",
    "status": "状态",
    "step": "步进",
    "strong": "加粗",
    "success": "成功",
    "stroke": "描边",
    "sub": "副",
    "suffix": "后缀",
    "switch": "开关",
    "tab": "标签页",
    "tabs": "标签页",
    "table": "表格",
    "tag": "标签",
    "target": "Target 目标侧",
    "text": "文本",
    "theme": "主题",
    "time": "时间",
    "title": "标题",
    "toggle": "切换",
    "tooltip": "提示",
    "top": "顶部",
    "total": "总数",
    "tracking": "跟踪",
    "transition": "过渡动画",
    "tree": "树",
    "trigger": "触发方式",
    "type": "类型",
    "unchecked": "未选中",
    "underline": "下划线",
    "warning": "警告",
    "all": "全部",
    "angle": "角度",
    "ant": "Ant",
    "closable": "可关闭",
    "copied": "已复制",
    "activated": "激活",
    "affix": "固钉",
    "affixed": "已固钉",
    "available": "可用",
    "central": "中心",
    "dock": "停靠面板",
    "docking": "停靠",
    "floatable": "可浮动",
    "floating": "浮动",
    "footer": "页脚",
    "header": "页眉",
    "host": "宿主",
    "link": "链接",
    "modal": "模态框",
    "movable": "可移动",
    "name": "名称",
    "notification": "通知",
    "perspective": "布局快照",
    "perspectives": "布局快照列表",
    "pixel": "像素",
    "prev": "上一步",
    "previous": "上一个",
    "next": "下一个",
    "resolved": "解析后的",
    "restore": "恢复",
    "root": "根",
    "save": "保存",
    "secondary": "辅助",
    "shimmer": "流光动画",
    "surface": "表面",
    "watermark": "水印",
    "widget": "控件",
    "tokens": "主题 token",
    "uses": "使用",
    "rounded": "圆角",
    "corners": "圆角",
    "legacy": "兼容",
    "opaque": "不透明",
    "path": "路径",
    "style": "样式",
    "validate": "校验",
    "value": "值",
    "variant": "视觉变体",
    "vertical": "垂直",
    "visibility": "可见阈值",
    "visible": "可见",
    "width": "宽度",
    "window": "窗口",
    "word": "单词",
    "wrap": "换行",
    "zero": "零值",
}

METHOD_PURPOSES = {
    ("AntTransfer", "sourceItems"): "返回 Source 源列表中的字符串集合，即尚未转移到 Target 目标列表的数据。",
    ("AntTransfer", "targetItems"): "返回 Target 目标列表中的字符串集合，即已经从 Source 源列表转移完成的数据。",
    ("AntTransfer", "setSourceItems"): "替换 Source 源列表的全部字符串项，并刷新穿梭框左侧列表和转移按钮状态。",
    ("AntTransfer", "setTargetItems"): "替换 Target 目标列表的全部字符串项，并刷新穿梭框右侧列表和转移按钮状态。",
    ("AntIcon", "builtinIconNames"): "返回内置的官方 Ant Design SVG 图标名称列表，可用于图标选择器、搜索和自动完成。",
    ("AntIcon", "iconNameForType"): "把内置图标枚举转换为对应的官方 Ant Design 图标资源名称。",
    ("AntIcon", "builtinPaths"): "返回指定内置图标在目标主题下的主路径和双色辅助路径。",
    ("AntIcon", "transformPath"): "把图标路径按目标矩形缩放和平移，供自定义绘制时复用 AntIcon 的坐标转换规则。",
    ("AntWindow", "forceClose"): "跳过关闭确认流程并直接关闭窗口，适合业务已经完成保存或确认后的强制退出。",
    ("AntWindow", "moveToCenter"): "把窗口移动到当前屏幕中心，常用于窗口创建后或尺寸变化后重新居中。",
    ("AntFloatButton", "backTopClicked"): "当 BackTop 模式的悬浮按钮被点击时发出，可用于自定义回到顶部逻辑。",
    ("AntTypography", "copied"): "文本复制成功后发出，参数携带已经写入剪贴板的文本。",
    ("AntTypography", "linkActivated"): "Link 类型文本被点击后发出，参数携带当前 href 链接地址。",
    ("AntMessage", "info"): "显示信息类型全局提示，并返回创建出的消息实例以便继续控制。",
    ("AntMessage", "success"): "显示成功类型全局提示，并返回创建出的消息实例以便继续控制。",
    ("AntMessage", "warning"): "显示警告类型全局提示，并返回创建出的消息实例以便继续控制。",
    ("AntMessage", "error"): "显示错误类型全局提示，并返回创建出的消息实例以便继续控制。",
    ("AntMessage", "loading"): "显示加载中的全局提示，并返回创建出的消息实例以便后续关闭或更新。",
    ("AntMessage", "closeAll"): "关闭当前所有全局提示消息。",
    ("AntNotification", "info"): "显示信息类型通知，并返回创建出的通知实例以便继续控制。",
    ("AntNotification", "success"): "显示成功类型通知，并返回创建出的通知实例以便继续控制。",
    ("AntNotification", "warning"): "显示警告类型通知，并返回创建出的通知实例以便继续控制。",
    ("AntNotification", "error"): "显示错误类型通知，并返回创建出的通知实例以便继续控制。",
    ("AntNotification", "closeAll"): "关闭当前所有通知提醒框。",
    ("AntModal", "info"): "弹出信息类型命令式对话框。",
    ("AntModal", "success"): "弹出成功类型命令式对话框。",
    ("AntModal", "warning"): "弹出警告类型命令式对话框。",
    ("AntModal", "error"): "弹出错误类型命令式对话框。",
    ("AntModal", "confirm"): "弹出确认类型命令式对话框，并按用户选择触发回调或信号。",
    ("AntAffix", "affixedWidget"): "返回正在被固钉管理的目标控件。",
    ("AntAffix", "setAffixedWidget"): "设置需要固钉的目标控件，滚动时会根据 offsetTop / offsetBottom 调整其位置。",
    ("AntAffix", "isAffixed"): "返回目标控件当前是否已经进入固钉状态。",
    ("AntAffix", "affixStateChanged"): "目标控件进入或离开固钉状态时发出。",
    ("AntApp", "rootWidget"): "返回 AntApp 管理的应用根控件。",
    ("AntApp", "feedbackHost"): "返回 Message、Modal、Notification 等反馈组件挂载的宿主控件。",
    ("AntApp", "instance"): "返回当前应用级 AntApp 单例，便于从业务代码触发全局反馈能力。",
    ("AntApp", "showMessage"): "在应用反馈宿主上显示一条全局消息提示。",
    ("AntApp", "showModal"): "在应用反馈宿主上显示一个模态对话框，并可传入确认/取消回调。",
    ("AntApp", "showNotification"): "在应用反馈宿主上显示一条通知提醒。",
    ("AntTour", "next"): "切换到下一步引导内容。",
    ("AntTour", "prev"): "切换到上一步引导内容。",
    ("AntTour", "finished"): "漫游式引导完成或关闭后发出。",
    ("AntConfigProvider", "apply"): "把当前全局配置应用到主题系统，并触发相关控件刷新。",
    ("AntDockManager", "addDockWidget"): "把停靠面板加入管理器，并按区域、相对面板或停靠位置放入布局。",
    ("AntDockManager", "splitDockWidget"): "把指定停靠面板按水平或垂直方向拆分，并插入新的停靠面板。",
    ("AntDockManager", "tabifyDockWidget"): "把两个停靠面板合并到同一个标签页组中。",
    ("AntDockManager", "dockWidgetArea"): "返回指定停靠面板当前所在的 Qt 停靠区域。",
    ("AntDockManager", "tabifiedDockWidgets"): "返回与指定停靠面板同组显示的标签页停靠面板列表。",
    ("AntDockManager", "dockWidgets"): "返回当前由管理器维护的全部停靠面板。",
    ("AntDockManager", "floatingDockWidgets"): "返回当前处于浮动窗口状态的停靠面板列表。",
    ("AntDockManager", "isDockWidgetFloating"): "返回指定停靠面板当前是否浮动显示。",
    ("AntDockManager", "setDockWidgetFloating"): "设置指定停靠面板是否浮动，并可指定浮动窗口的全局几何区域。",
    ("AntDockManager", "isDockWidgetClosable"): "返回指定停靠面板是否允许关闭。",
    ("AntDockManager", "setDockWidgetClosable"): "设置指定停靠面板是否允许关闭。",
    ("AntDockManager", "isDockWidgetFloatable"): "返回指定停靠面板是否允许从布局中浮动出来。",
    ("AntDockManager", "setDockWidgetFloatable"): "设置指定停靠面板是否允许浮动。",
    ("AntDockManager", "isDockWidgetMovable"): "返回指定停靠面板是否允许拖动换位或重新停靠。",
    ("AntDockManager", "setDockWidgetMovable"): "设置指定停靠面板是否允许拖动移动。",
    ("AntDockManager", "moveDockWidgetTab"): "把指定停靠面板移动到当前标签页组中的目标索引。",
    ("AntDockManager", "savePerspective"): "保存当前停靠布局为命名快照。",
    ("AntDockManager", "restorePerspective"): "按名称恢复之前保存的停靠布局快照。",
    ("AntDockManager", "removePerspective"): "删除指定名称的停靠布局快照。",
    ("AntDockManager", "clearPerspectives"): "清空所有已保存的停靠布局快照。",
    ("AntDockManager", "perspectiveNames"): "返回所有已保存的停靠布局快照名称。",
    ("AntDockManager", "perspectiveState"): "返回指定布局快照的序列化状态数据。",
    ("AntDockManager", "setPerspectiveState"): "写入指定名称的布局快照序列化状态，供之后 restorePerspective 使用。",
    ("AntInputDialog", "getText"): "弹出文本输入对话框，用户确认后返回输入文本，并通过 ok 参数告知是否接受。",
    ("AntInputDialog", "getInt"): "弹出整数输入对话框，按给定范围和步进约束输入，用户确认后返回整数值。",
    ("AntInputDialog", "getDouble"): "弹出浮点数输入对话框，按给定范围和小数位约束输入，用户确认后返回浮点值。",
    ("AntInputDialog", "getItem"): "弹出下拉项输入对话框，从字符串列表中选择或编辑一项，用户确认后返回文本。",
    ("AntInputDialog", "inputMode"): "返回当前输入模式：文本、整数或浮点数。",
    ("AntInputDialog", "setInputMode"): "切换输入模式，并显示对应的 AntInput、AntInputNumber 或下拉选择控件。",
    ("AntInputDialog", "labelText"): "返回输入区域上方的说明标签文本。",
    ("AntInputDialog", "setLabelText"): "设置输入区域上方的说明标签文本，通常用于描述用户需要填写的内容。",
    ("AntInputDialog", "textValue"): "返回当前文本输入值；当处于 item 模式时返回下拉框当前文本。",
    ("AntInputDialog", "setTextValue"): "设置文本输入值，并同步单行输入、多行输入或下拉选择控件的显示。",
    ("AntInputDialog", "textEchoMode"): "返回文本输入的回显模式，例如普通文本或密码输入。",
    ("AntInputDialog", "setTextEchoMode"): "设置文本输入的回显模式，常用于密码或隐藏输入内容。",
    ("AntInputDialog", "placeholderText"): "返回输入控件的占位提示文本。",
    ("AntInputDialog", "setPlaceholderText"): "设置输入控件的占位提示文本。",
    ("AntInputDialog", "inputMethodHints"): "返回传递给内部输入控件的 Qt 输入法提示。",
    ("AntInputDialog", "setInputMethodHints"): "设置输入法提示，并同步到内部文本、数字和下拉编辑控件。",
    ("AntInputDialog", "intValue"): "返回整数输入模式下的当前值。",
    ("AntInputDialog", "setIntValue"): "设置整数输入值，并按当前最小值和最大值自动收敛。",
    ("AntInputDialog", "intMinimum"): "返回整数输入允许的最小值。",
    ("AntInputDialog", "setIntMinimum"): "设置整数输入允许的最小值。",
    ("AntInputDialog", "intMaximum"): "返回整数输入允许的最大值。",
    ("AntInputDialog", "setIntMaximum"): "设置整数输入允许的最大值。",
    ("AntInputDialog", "setIntRange"): "同时设置整数输入允许的最小值和最大值。",
    ("AntInputDialog", "intStep"): "返回整数输入框每次步进调整的值。",
    ("AntInputDialog", "setIntStep"): "设置整数输入框每次步进调整的值。",
    ("AntInputDialog", "doubleValue"): "返回浮点输入模式下的当前值。",
    ("AntInputDialog", "setDoubleValue"): "设置浮点输入值，并按当前最小值和最大值自动收敛。",
    ("AntInputDialog", "doubleMinimum"): "返回浮点输入允许的最小值。",
    ("AntInputDialog", "setDoubleMinimum"): "设置浮点输入允许的最小值。",
    ("AntInputDialog", "doubleMaximum"): "返回浮点输入允许的最大值。",
    ("AntInputDialog", "setDoubleMaximum"): "设置浮点输入允许的最大值。",
    ("AntInputDialog", "setDoubleRange"): "同时设置浮点输入允许的最小值和最大值。",
    ("AntInputDialog", "doubleDecimals"): "返回浮点输入显示和编辑时使用的小数位数。",
    ("AntInputDialog", "setDoubleDecimals"): "设置浮点输入显示和编辑时使用的小数位数。",
    ("AntInputDialog", "comboBoxItems"): "返回 item 输入模式下的候选字符串列表。",
    ("AntInputDialog", "setComboBoxItems"): "设置 item 输入模式下的候选字符串列表，并切换到下拉选择显示。",
    ("AntInputDialog", "isComboBoxEditable"): "返回 item 输入模式下是否允许用户输入列表之外的文本。",
    ("AntInputDialog", "setComboBoxEditable"): "设置 item 输入模式下是否允许用户输入列表之外的文本。",
    ("AntInputDialog", "okButtonText"): "返回确认按钮文本。",
    ("AntInputDialog", "setOkButtonText"): "设置确认按钮文本。",
    ("AntInputDialog", "cancelButtonText"): "返回取消按钮文本。",
    ("AntInputDialog", "setCancelButtonText"): "设置取消按钮文本。",
    ("AntInputDialog", "setOption"): "开启或关闭单个输入对话框选项，例如隐藏按钮或使用多行文本输入。",
    ("AntInputDialog", "testOption"): "检查指定输入对话框选项当前是否启用。",
    ("AntInputDialog", "setOptions"): "一次性替换输入对话框选项集合。",
    ("AntInputDialog", "options"): "返回当前启用的输入对话框选项集合。",
    ("AntInputDialog", "refreshAntStyle"): "刷新 AntInputDialog 的 AntDialog 外观、内部输入控件同步状态和主题相关显示。",
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


def lower_first(value: str) -> str:
    return value[:1].lower() + value[1:] if value else value


def normalized_property_name(name: str) -> str:
    cleaned = name.strip().rstrip("_")
    if cleaned.startswith("is") and len(cleaned) > 2 and cleaned[2].isupper():
        return lower_first(cleaned[2:])
    if cleaned.startswith("has") and len(cleaned) > 3 and cleaned[3].isupper():
        return lower_first(cleaned[3:])
    if cleaned.startswith("can") and len(cleaned) > 3 and cleaned[3].isupper():
        return lower_first(cleaned[3:])
    return cleaned


def property_noun(name: str, class_name: str = "") -> str:
    key = f"{class_name}.{name}" if class_name else name
    if key in PROPERTY_NOUNS:
        return PROPERTY_NOUNS[key]
    if name in PROPERTY_NOUNS:
        return PROPERTY_NOUNS[name]
    normalized = normalized_property_name(name)
    if normalized in PROPERTY_NOUNS:
        return PROPERTY_NOUNS[normalized]

    words = split_camel_words(normalized.replace("_", " "))
    labels = [TOKEN_CN.get(word.lower(), word) for word in words]
    phrase = "".join(labels).strip()
    if not phrase:
        return "状态"
    if phrase.endswith(("状态", "文本", "内容", "数量", "尺寸", "颜色", "列表", "集合", "索引", "模式", "类型", "位置", "方向")):
        return phrase
    return f"{phrase}状态" if len(labels) == 1 and labels[0] in {"加载", "禁用", "打开", "选中", "折叠"} else phrase


def property_label(name: str, class_name: str = "") -> str:
    key = f"{class_name}.{name}" if class_name else name
    if key in PROPERTY_PURPOSES:
        return PROPERTY_PURPOSES[key]
    normalized = normalized_property_name(name)
    if name in PROPERTY_PURPOSES:
        return PROPERTY_PURPOSES[name]
    if normalized in PROPERTY_PURPOSES:
        return PROPERTY_PURPOSES[normalized]

    lower = normalized.lower()
    noun = property_noun(name, class_name)
    if lower.endswith("visible"):
        return f"控制{noun}是否显示。"
    if lower.endswith("enabled"):
        return f"控制{noun}是否启用。"
    if lower.endswith("color"):
        return f"{noun}，用于覆盖默认主题色或状态色。"
    if lower.endswith("text") or lower.endswith("title") or lower.endswith("content"):
        return f"{noun}，用于展示、编辑或同步业务文案。"
    if lower.endswith("index") or lower.endswith("row") or lower.endswith("column"):
        return f"{noun}，用于定位当前项、行列或页面。"
    if lower.endswith("count") or lower.endswith("number"):
        return f"{noun}，用于显示统计、限制数量或同步计数状态。"
    if "selected" in lower or "current" in lower:
        return f"{noun}，用于读取或同步用户当前选择。"
    if "hover" in lower or "press" in lower:
        return f"{noun}，主要供样式绘制、交互动效和测试验证使用。"
    if lower.startswith(("min", "max")) or "minimum" in lower or "maximum" in lower:
        return f"{noun}，用于限制可输入、可选择或可显示的范围。"
    if lower.endswith("size") or lower.endswith("width") or lower.endswith("height") or lower.endswith("radius"):
        return f"{noun}，用于控制控件布局占位或绘制尺寸。"
    if lower.endswith("mode") or lower.endswith("type") or lower.endswith("variant") or lower.endswith("shape"):
        return f"{noun}，用于切换控件的工作模式或视觉形态。"
    return f"{noun}，用于控制控件的{noun}表现或同步对应数据。"


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


def component_name_for_doc(component: dict[str, object] | str) -> str:
    if isinstance(component, dict):
        return str(component["name"])
    return str(component)


def component_cn_for_doc(component: dict[str, object] | str) -> str:
    if isinstance(component, dict):
        return component_cn_name(component)
    return str(component)


def action_subject(method: str, prefix: str, class_name: str = "") -> str:
    raw = method[len(prefix) :]
    if not raw:
        return "内容"
    return property_noun(lower_first(raw), class_name)


def api_description(decl: str, kind: str, component: dict[str, object] | str = "") -> str:
    name = method_name(decl)
    class_name = component_name_for_doc(component)
    cn_name = component_cn_for_doc(component) or class_name
    exact_key = (class_name, name)
    if exact_key in METHOD_PURPOSES:
        return METHOD_PURPOSES[exact_key]

    if kind == "signal":
        if name.endswith("Changed"):
            subject = signal_subject(name)
            return f"当{property_noun(subject, class_name)}变化时发出，用于外部同步 UI、刷新业务数据或触发后续流程。"
        if name.endswith("Clicked"):
            subject = signal_subject(name)
            return f"当用户点击{property_noun(subject, class_name) if subject else cn_name}时发出，用于连接业务动作。"
        if name.endswith("Requested"):
            subject = signal_subject(name)
            return f"当{cn_name}请求执行{property_noun(subject, class_name)}行为时发出，外部可以选择接受、拦截或补充处理。"
        if name in {"accepted", "rejected"}:
            return f"当{cn_name}{'确认' if name == 'accepted' else '取消'}完成时发出。"
        return f"当{property_noun(name, class_name)}事件发生时发出，供外部连接槽函数响应。"

    if class_name and (name == class_name or name == f"~{class_name}"):
        if name.startswith("~"):
            return f"销毁{cn_name}实例并释放自身维护的动画、弹层或子控件资源。"
        params = method_params(decl)
        if params in {"QWidget* parent = nullptr", "QObject* parent = nullptr", "无参数"}:
            return f"创建一个空的{cn_name}实例；parent 用于纳入 Qt 对象树和布局生命周期管理。"
        return f"创建{cn_name}实例，并用构造参数初始化主要显示内容、类型或父对象。"

    if name.startswith("set"):
        prop = setter_property_name(name)
        return f"设置{property_noun(prop, class_name)}。{property_label(prop, class_name)}"

    if name.startswith(("is", "has", "can")):
        return f"返回{cn_name}是否处于{property_noun(name, class_name)}，用于按当前状态决定交互、显示或业务分支。"

    if name.startswith("add"):
        subject = action_subject(name, "add", class_name)
        return f"向{cn_name}添加{subject}，用于运行时动态构建内容或动作集合。"
    if name.startswith("insert"):
        subject = action_subject(name, "insert", class_name)
        return f"在指定位置向{cn_name}插入{subject}，保持既有条目顺序并更新显示。"
    if name.startswith("remove"):
        subject = action_subject(name, "remove", class_name)
        return f"从{cn_name}移除指定{subject}，并同步内部状态、布局和重绘。"
    if name.startswith("clear"):
        subject = action_subject(name, "clear", class_name)
        return f"清空{cn_name}的{subject}，用于重置内容、选择或自定义状态。"
    if name.startswith("take"):
        subject = action_subject(name, "take", class_name)
        return f"从{cn_name}取出并移除指定{subject}，调用方负责继续持有或释放返回对象。"

    if name.startswith(("current", "selected", "value", "text", "title", "content")):
        return f"返回{property_noun(name, class_name)}，用于表单提交、状态同步或界面回填。"
    if name in {"sizeHint", "minimumSizeHint", "heightForWidth"}:
        return f"返回{cn_name}提供给 Qt 布局系统的尺寸建议，让控件在不同 DPI、字体和内容下保持合理占位。"

    ret = return_type(decl, class_name)
    params = method_params(decl)
    if ret != "void" and params == "无参数" and not decl.startswith("static "):
        return f"返回{cn_name}当前使用的{property_noun(name, class_name)}。"

    if name.startswith("show"):
        return f"显示{action_subject(name, 'show', class_name)}，通常会触发弹层、信息区或辅助入口出现。"
    if name.startswith("hide"):
        return f"隐藏{action_subject(name, 'hide', class_name)}，通常会关闭弹层、信息区或辅助入口。"
    if name.startswith(("open", "close", "toggle")):
        verb = "打开" if name.startswith("open") else "关闭" if name.startswith("close") else "切换"
        prefix = "open" if name.startswith("open") else "close" if name.startswith("close") else "toggle"
        return f"{verb}{cn_name}的{action_subject(name, prefix, class_name)}状态，并同步动画、重绘或对应信号。"
    if name.startswith("scroll"):
        return f"控制{cn_name}滚动到指定位置或目标条目，用于和视图、回到顶部或滚动容器联动。"
    if name.startswith("move"):
        return f"移动{cn_name}到指定位置，用于窗口、弹层或布局位置调整。"
    if name.startswith("resize"):
        return f"调整{cn_name}尺寸或尺寸相关缓存，用于响应布局和 DPI 变化。"
    if name.startswith(("find", "indexOf")):
        return f"在{cn_name}中查找{action_subject(name, 'find' if name.startswith('find') else 'indexOf', class_name)}，返回匹配索引或对象位置。"
    if name.startswith(("item", "row", "column")):
        return f"返回{cn_name}中指定{property_noun(name, class_name)}，用于读取条目、行列或单元格数据。"
    if name.startswith(("select", "deselect")):
        return f"更新{cn_name}的{action_subject(name, 'select' if name.startswith('select') else 'deselect', class_name)}选择状态。"
    if name.startswith("sort"):
        return f"按指定规则重新排序{cn_name}的数据项，并刷新显示结果。"
    if name.startswith(("start", "stop", "restart")):
        verb = "启动" if name.startswith("start") else "停止" if name.startswith("stop") else "重新启动"
        prefix = "start" if name.startswith("start") else "stop" if name.startswith("stop") else "restart"
        return f"{verb}{cn_name}的{action_subject(name, prefix, class_name)}流程，例如动画、计时器或自动播放。"
    if name.startswith(("update", "refresh", "rebuild", "relayout")):
        return f"刷新{cn_name}的{action_subject(name, name.split('_', 1)[0] if '_' in name else re.match(r'[a-z]+', name).group(0), class_name)}缓存、布局或绘制状态。"
    if decl.startswith("static "):
        return f"不创建{cn_name}实例，直接执行{property_noun(name, class_name)}命令或返回对应工具数据。"

    if ret != "void":
        return f"返回{cn_name}当前使用的{property_noun(name, class_name)}。"
    return f"执行{cn_name}的{property_noun(name, class_name)}操作，用于控制该控件的内容、交互或显示状态。"


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
    return f"参数：{esc(params)}<br>{esc(ret_text)}"


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
        [f"<code>{esc(decl)}</code>", esc(api_description(decl, "method", component)), api_meta(decl, name)]
        for decl in methods
    ]
    slot_rows = [
        [f"<code>{esc(decl)}</code>", esc(api_description(decl, "method", component)), api_meta(decl, name)]
        for decl in slots
    ]
    signal_rows = [
        [f"<code>{esc(decl)}</code>", esc(api_description(decl, "signal", component)), api_meta(decl, name)]
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
