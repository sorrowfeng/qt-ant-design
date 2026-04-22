# qt-ant-design Agent Notes

## 项目定位

`qt-ant-design` 是一个 Qt6 Widgets 组件库项目，目标是使用 `QPainter` 手绘方式实现 Ant Design 设计规范中的常用桌面端组件。当前项目以静态库 `qt-ant-design` 输出，并提供 `qt-ant-design-example` 示例程序用于验证主题、交互状态和组件变体。

## 参考来源

- 绘制实现参考：`D:\Project\GitProject\ElaWidgetTools`
  - 重点参考 `ElaTheme` 的单例主题管理、`themeModeChanged` 信号通知、控件 `paintEvent` 自绘方式，以及 `QTimer` 驱动的逐帧动画。
- 设计规范参考：`D:\Project\GitProject\qt-ant-design\submodules\ant-design`
  - 设计 token 主要参考 `components/theme/themes/seed.ts`、`components/theme/themes/default`、`components/theme/themes/dark`。
  - 组件样式主要参考 `components/button/`、`components/checkbox/`、`components/input/`、`components/radio/`、`components/switch/`、`components/card/`。

## 本次主要变更说明

- 搭建了 Qt6 Widgets 静态库项目骨架，包含 `src/core`、`src/styles`、`src/widgets`、`examples`、`resources` 目录。
- 新增 `AntTypes`，定义主题模式、按钮类型/尺寸/形状、输入框尺寸/状态、卡片尺寸等公共枚举。
- 新增 `AntTheme` 主题系统，支持默认亮色和暗黑模式、主题切换信号、核心颜色/字体/圆角/间距 token 获取。
- 新增 `AntPalette` 颜色工具，提供基础色派生、hover/active/background/border/disabled 等颜色计算。
- 新增 `AntButton`、`AntCheckbox`、`AntDatePicker`、`AntInput`、`AntProgress`、`AntRadio`、`AntSelect`、`AntSlider`、`AntSpin`、`AntSwitch`、`AntTimePicker`、`AntCard` 十二个组件，均使用 Qt Widgets 与 `QPainter` 自绘实现，不依赖 QSS 绘制主体外观。
- 新增 `qt-ant-design-example` 示例程序，包含无边框窗口、左侧导航、右侧组件展示页和亮色/暗色主题切换。
- 本次新增 `AntSwitch` 组件，支持 checked、loading、disabled、small/middle size、checkedChildren/unCheckedChildren 文本展示、键盘切换和滑块动画。
- 本次新增 `AntCheckbox` 组件，支持 checked、indeterminate、disabled、文本标签、键盘切换和焦点描边。
- 本次新增 `AntDatePicker` 组件，支持 selectedDate、displayFormat、placeholder、allowClear、disabled、large/middle/small size、error/warning status、outlined/filled/underlined/borderless variant 和自绘日历 popup。
- 本次新增 `AntProgress` 组件，支持 percent、line/circle/dashboard type、normal/success/exception/active status、showInfo、strokeWidth 和 circleSize。
- 本次新增 `AntRadio` 组件，支持 checked、disabled、value、文本标签、同父级自动互斥、键盘切换和焦点描边。
- 本次新增 `AntSelect` 组件，支持 options、placeholder、allowClear、loading、disabled、large/middle/small size、error/warning status、outlined/filled/underlined/borderless variant 和键盘选择。
- 本次新增 `AntSlider` 组件，支持 min/max/value/step、水平/垂直方向、reverse、dots、included、disabled、键盘调节和 handle hover/focus 动画。
- 本次新增 `AntSpin` 组件，支持 spinning、delay、description、percent 和 small/middle/large size，自绘旋转点阵和进度环。
- 本次新增 `AntTimePicker` 组件，支持 selectedTime、displayFormat、placeholder、allowClear、hour/minute/second step、showNow、disabled、large/middle/small size、error/warning status、outlined/filled/underlined/borderless variant 和自绘三列时间 popup。
- CMake 增加安装规则，安装到仓库根目录 `install/` 时会输出示例程序、静态库、头文件和 CMake targets；Windows 下会尝试调用 `windeployqt` 部署 Qt 运行依赖。

## 已移植组件

### 通用

- [x] `AntButton`
  - 对应 Ant Design Button。
  - 支持类型：`primary`、`default`、`dashed`、`text`、`link`。
  - 支持尺寸：`large`、`middle`、`small`。
  - 支持形状：`default`、`circle`、`round`。
  - 支持状态和属性：`loading`、`disabled`、`danger`、`ghost`、`block`。
  - 绘制方式：继承 `QPushButton`，在 `paintEvent` 中绘制背景、边框、文本、焦点描边和 loading spinner。

### 数据录入

- [x] `AntCheckbox`
  - 对应 Ant Design Checkbox。
  - 支持状态和属性：`checked`、`indeterminate`、`disabled`、`text`。
  - 支持信号：`checkedChanged`、`indeterminateChanged`、`stateChanged`、`toggled`、`clicked`。
  - 绘制方式：继承 `QWidget`，在 `paintEvent` 中绘制圆角选择框、选中勾、半选横线、文本和焦点描边。
  - 交互方式：鼠标点击切换，支持 Space/Enter 键盘切换。

- [x] `AntDatePicker`
  - 对应 Ant Design DatePicker。
  - 支持属性：`selectedDate`、`displayFormat`、`placeholderText`、`pickerSize`、`status`、`variant`、`allowClear`、`open`。
  - 支持信号：`selectedDateChanged`、`dateStringChanged`、`openChanged`、`cleared`。
  - 绘制方式：继承 `QWidget`，在 `paintEvent` 中绘制输入框、状态边框、清除按钮和日历图标；popup 中绘制月份标题、星期栏和日期网格。
  - 交互方式：支持鼠标展开、翻月、选日、清除，以及 Space/Enter/Escape/Left/Right 键盘操作。
- [x] `AntInput`
  - 对应 Ant Design Input。
  - 内部使用 `QLineEdit` 处理文本输入，外层 `AntInput` 负责绘制背景、边框和焦点外发光。
  - 支持尺寸：`large`、`middle`、`small`。
  - 支持状态：普通、错误、警告、禁用。
  - 支持功能：`addonBefore`、`addonAfter`、前缀图标、后缀图标、自定义前后缀 widget、清除按钮、密码模式。

- [x] `AntProgress`
  - 对应 Ant Design Progress。
  - 支持属性：`percent`、`progressType`、`status`、`showInfo`、`strokeWidth`、`circleSize`。
  - 支持类型：`line`、`circle`、`dashboard`。
  - 支持状态：`normal`、`success`、`exception`、`active`。
  - 绘制方式：继承 `QWidget`，在 `paintEvent` 中绘制线性进度、圆形进度、仪表盘进度和状态文本。
- [x] `AntRadio`
  - 对应 Ant Design Radio。
  - 支持状态和属性：`checked`、`disabled`、`text`、`value`、`autoExclusive`。
  - 支持信号：`checkedChanged`、`valueChanged`、`toggled`、`clicked`。
  - 绘制方式：继承 `QWidget`，在 `paintEvent` 中绘制圆形指示器、选中圆点、文本和焦点描边。
  - 交互方式：鼠标点击切换，支持 Space/Enter 键盘切换；默认同父级 `AntRadio` 自动互斥。

- [x] `AntSelect`
  - 对应 Ant Design Select。
  - 支持属性：`selectSize`、`status`、`variant`、`placeholderText`、`allowClear`、`loading`、`open`、`currentIndex`、`maxVisibleItems`。
  - 支持信号：`currentIndexChanged`、`currentTextChanged`、`currentValueChanged`、`optionSelected`、`openChanged`、`cleared`。
  - 绘制方式：继承 `QWidget`，在 `paintEvent` 中绘制选择框、状态边框、清除按钮、loading spinner 和展开箭头；下拉项使用自绘 popup row。
  - 交互方式：支持鼠标展开/选择/清除，支持 Space/Enter/Escape/Up/Down 键盘操作。
- [x] `AntSlider`
  - 对应 Ant Design Slider。
  - 支持属性：`minimum`、`maximum`、`value`、`singleStep`、`orientation`、`reverse`、`dots`、`included`、`keyboard`。
  - 支持信号：`valueChanged`、`sliderMoved`、`sliderPressed`、`sliderReleased`、`changeComplete`。
  - 绘制方式：继承 `QWidget`，在 `paintEvent` 中绘制 rail、track、handle、ticks 和 focus outline。
  - 交互方式：支持鼠标点击/拖拽、方向键/Page/Home/End 键盘调节，并通过 `QPropertyAnimation` 绘制 handle 和焦点动画。
- [x] `AntSpin`
  - 对应 Ant Design Spin。
  - 支持属性：`spinning`、`spinSize`、`description`、`delay`、`percent`。
  - 支持信号：`spinningChanged`、`spinSizeChanged`、`descriptionChanged`、`delayChanged`、`percentChanged`。
  - 绘制方式：继承 `QWidget`，在 `paintEvent` 中绘制旋转点阵、进度环和描述文本。
  - 交互方式：使用 `QTimer` 逐帧驱动 loading 动画，支持延迟显示和隐藏状态。
- [x] `AntSwitch`
  - 对应 Ant Design Switch。
  - 支持尺寸：`middle`、`small`。
  - 支持状态和属性：`checked`、`loading`、`disabled`、`checkedText`、`uncheckedText`。
  - 支持信号：`checkedChanged`、`toggled`、`clicked`。
  - 绘制方式：继承 `QWidget`，在 `paintEvent` 中绘制胶囊轨道、白色滑块、文本、焦点描边和 loading spinner。
  - 交互方式：鼠标点击切换，支持 Space/Enter 键盘切换，使用 `QPropertyAnimation` 绘制滑块位置和按压拉伸动画。

- [x] `AntTimePicker`
  - 对应 Ant Design TimePicker。
  - 支持属性：`selectedTime`、`displayFormat`、`placeholderText`、`pickerSize`、`status`、`variant`、`allowClear`、`open`、`hourStep`、`minuteStep`、`secondStep`、`showNow`。
  - 支持信号：`selectedTimeChanged`、`timeStringChanged`、`openChanged`、`cleared`、`accepted`。
  - 绘制方式：继承 `QWidget`，在 `paintEvent` 中绘制输入框、状态边框、清除按钮和时钟图标；popup 中绘制 hour/minute/second 三列、Now 和 OK。
  - 交互方式：支持鼠标展开、滚轮调节列、选择时间、Now/OK 和 Space/Enter/Escape 键盘操作。
### 数据展示

- [x] `AntCard`
  - 对应 Ant Design Card。
  - 支持标题、额外内容、封面 widget、内容区 widget/layout、操作区 widget。
  - 支持属性：`bordered`、`hoverable`、`loading`、`cardSize`。
  - 绘制方式：继承 `QFrame`，在 `paintEvent` 中绘制容器背景、边框、分割线、hover 阴影、loading 遮罩和 spinner。

## 待移植组件

以下列表参考 `submodules/ant-design/components/` 中的常用组件，并按当前 Qt Widgets 桌面组件库的优先级排序。

### 高优先级

- [ ] `AntMenu`：导航菜单，后续可替换 example 左侧导航。
- [ ] `AntIcon`：统一图标接口，供按钮、输入框、菜单、提示类组件复用。
- [ ] `AntInputNumber`：数字输入框。
- [ ] `AntForm`：表单布局、校验状态和 label/control 对齐。
- [ ] `AntModal`：模态对话框。
- [ ] `AntMessage`：全局轻提示。
- [ ] `AntNotification`：通知提醒框。
- [ ] `AntTooltip`：文字提示。

### 中优先级

- [ ] `AntTabs`：标签页。
- [ ] `AntDropdown`：下拉菜单。
- [ ] `AntPopover`：气泡卡片。
- [ ] `AntPopconfirm`：气泡确认框。
- [ ] `AntAlert`：警告提示。
- [ ] `AntTag`：标签。
- [ ] `AntBadge`：徽标数。
- [ ] `AntAvatar`：头像。
- [ ] `AntSkeleton`：骨架屏。
- [ ] `AntEmpty`：空状态。
- [ ] `AntPagination`：分页。

### 后续扩展

- [ ] `AntTable`：表格。
- [ ] `AntTree`：树形控件。
- [ ] `AntTreeSelect`：树选择。
- [ ] `AntCalendar`：日历。
- [ ] `AntUpload`：上传。
- [ ] `AntDrawer`：抽屉。
- [ ] `AntBreadcrumb`：面包屑。
- [ ] `AntSteps`：步骤条。
- [ ] `AntTimeline`：时间轴。
- [ ] `AntCollapse`：折叠面板。
- [ ] `AntDescriptions`：描述列表。
- [ ] `AntList`：列表。
- [ ] `AntStatistic`：统计数值。
- [ ] `AntResult`：结果页。
- [ ] `AntDivider`：分割线。
- [ ] `AntSpace`：间距布局。
- [ ] `AntFlex`：弹性布局。
- [ ] `AntLayout`：布局容器。
- [ ] `AntTypography`：排版。

## 开发规范

- 仅支持 Qt6，CMake 使用 `find_package(Qt6 REQUIRED COMPONENTS Core Widgets)`。
- C++ 标准为 C++17。
- 组件主体视觉绘制优先使用 `QPainter`，不要依赖 QSS 实现核心外观。
- 主题、颜色、字体、圆角、间距等视觉值应从 `AntTheme` 和 `AntPalette` 获取，避免在组件中散落硬编码。
- 组件应连接 `AntTheme::themeChanged` 或 `AntTheme::themeModeChanged`，主题切换时自动刷新布局和绘制。
- 自绘组件应保留 Qt 原生语义能力：
  - 按钮类优先继承 `QPushButton`。
  - 输入类可组合 `QLineEdit` 来保留文本编辑、选择、输入法和快捷键能力。
  - 容器类可组合 `QWidget`/`QLayout` 暴露插槽区域。
- loading、hover、pressed、focused 等交互状态应由控件自身状态驱动，并在 `paintEvent` 中统一绘制。
- 新增组件时同步更新 `AGENT.md` 的“已移植组件”和“待移植组件”章节。

## 示例程序

当前 example 展示了以下组件：

- `AntButton`：类型、尺寸、形状、danger、ghost、loading、disabled、block。
- `AntCheckbox`：未选、选中、半选、禁用和“全选”受控示例。
- `AntDatePicker`：基础选择、自定义格式、尺寸、状态、变体、禁用和清除。
- `AntInput`：大/中/小尺寸、allowClear、addonBefore/addonAfter、password、error、disabled。
- `AntProgress`：线性进度、active/exception/success 状态、circle 和 dashboard。
- `AntRadio`：基础横向组、禁用、禁用选中和纵向组。
- `AntSelect`：基础选择、allowClear、尺寸、状态、变体、loading、disabled 和禁用选项。
- `AntSlider`：基础滑动输入、step/dots、reverse、垂直方向、disabled 和 included=false。
- `AntSpin`：small/middle/large、描述文本、percent 进度和嵌入式加载块。
- `AntSwitch`：checked/unchecked、小尺寸、文本、loading、disabled。
- `AntTimePicker`：基础选择、自定义格式、尺寸、状态、变体、step、showNow 和 disabled。
- `AntCard`：默认卡片、hoverable 卡片、loading 卡片、操作区卡片。

运行方式：

```powershell
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=D:/Project/GitProject/qt-ant-design/install
cmake --build build --config Debug
cmake --install build --config Debug
.\install\bin\qt-ant-design-example.exe
```

如果已经执行过安装，也可以直接运行：

```powershell
.\install\bin\qt-ant-design-example.exe
```

## 当前安装产物

安装目录为 `D:\Project\GitProject\qt-ant-design\install`。

- `install/bin/qt-ant-design-example.exe`：示例程序。
- `install/lib/qt-ant-design.lib`：静态库。
- `install/include/qt-ant-design/`：对外头文件。
- `install/lib/cmake/qt-ant-design/`：CMake targets。

Windows Debug 构建安装时会尝试通过 `windeployqt` 复制 Qt Debug DLL 和插件到 `install/bin`，便于直接运行示例程序。
