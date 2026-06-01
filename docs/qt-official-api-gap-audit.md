# Qt 官方替代控件 API / 信号缺口审计

更新日期：2026-06-01

## 范围与方法

本次审计对比的是当前 `src/widgets` 中明确用于替代 Qt Widgets 的 Ant 控件、Qt 风格别名头文件，以及 Qt 5.15.2 的官方 Widgets 头文件：

- Ant 侧：`src/widgets/*.h`、别名头如 `AntLineEdit.h`、`AntComboBox.h`、`AntTableWidget.h`。
- Qt 侧：`C:\Qt\5.15.2\msvc2019_64\include\QtWidgets\*.h`。
- 重点：公开方法、`Q_PROPERTY`、信号、常用槽函数和常用 item/model/view 接口。
- 不包含：视觉一致性、运行时行为细节、Ant Design 独有组件的完整 AntD API。

直接继承 Qt 官方控件的类，继承来的 Qt API 视为已存在；自绘或组合封装的控件，则按“是否能作为 Qt 官方控件替代品直接使用”来判断缺口。

## 总体结论

当前项目已经补齐了一批常用 Qt 风格接口，尤其是 `AntInput`、`AntSelect`、`AntList`、`AntDatePicker`、`AntTimePicker`、`AntProgress` 等控件的基础值、状态和常用信号。不过若目标是“替代 Qt 官方控件”或让别名类接近 drop-in replacement，仍有几类明显缺口：

- `AntTableWidget` / `AntTreeWidget` / `AntListView` / `AntTableView` / `AntTreeView` 的差距最大。它们当前是 `AntTable`、`AntTree`、`AntList` 的别名，并不是 `QAbstractItemView` 体系，缺少 model/index/delegate/selectionModel 和大量 item API。
- `AntComboBox` 当前是 `AntSelect` 别名，覆盖了常用 item 管理，但缺少 `QComboBox` 的 model/delegate/view、editable lineEdit、icon/separator、insertPolicy 和 `editTextChanged` 等接口。
- `AntLineEdit` 当前是 `AntInput` 别名，内部暴露 `QLineEdit* lineEdit()`，但许多 `QLineEdit` 常用接口仍需要用户手动访问子控件，直接调用会编译失败。
- `AntFileDialog` 已覆盖核心静态函数和选文件流程，但缺少 `QFileDialog` 的高级属性、URL/mime/filter/sidebar/history 接口和信号。
- `AntStatusBar` 是自绘状态栏，不继承 `QStatusBar`，缺少 widget 插入类 API。
- `AntSpinBox` 当前别名到 `AntInputNumber`，底层是 `QDoubleSpinBox`，与真正 `QSpinBox` 的 int value/signals、`displayIntegerBase` 不完全一致。

## 替代映射概览

| Ant 控件 / 别名 | 对比 Qt 官方控件 | 实现形态 | 兼容状态 |
| --- | --- | --- | --- |
| `AntPushButton` / `AntButton` | `QPushButton` | 继承 `QPushButton` | Qt API 基本保留 |
| `AntToolButton` | `QToolButton` | 继承 `QToolButton` | Qt API 基本保留 |
| `AntLineEdit` / `AntInput` | `QLineEdit` | `QWidget` + 内部 `QLineEdit` | 常用接口已覆盖，仍缺直接 wrapper |
| `AntComboBox` / `AntSelect` | `QComboBox` | 自绘组合控件 | 常用 item 接口已覆盖，model/view/editable API 缺口较多 |
| `AntSpinBox` / `AntInputNumber` | `QSpinBox` | 继承 `QDoubleSpinBox` | 不是严格 int spin box |
| `AntDoubleSpinBox` / `AntInputNumber` | `QDoubleSpinBox` | 继承 `QDoubleSpinBox` | Qt API 基本保留 |
| `AntCheckBox` | `QCheckBox` | 自绘 `QWidget` | 基础 check API 已覆盖，缺部分 `QAbstractButton` API |
| `AntRadioButton` / `AntRadio` | `QRadioButton` | 自绘 `QWidget` | 基础 radio API 已覆盖，缺部分 `QAbstractButton` API |
| `AntSlider` | `QSlider` | 自绘 `QWidget` | 基础 slider API 已覆盖，缺 action/tick/invertedControls |
| `AntProgressBar` / `AntProgress` | `QProgressBar` | 自绘 `QWidget` | value/range 已覆盖，缺 format/orientation 等 |
| `AntLabel` / `AntTypography` | `QLabel` | 自绘 `QWidget` | 文本标签覆盖较好，图片/交互文本 API 缺口较多 |
| `AntListWidget` / `AntList` | `QListWidget` | 自绘 `QWidget` | 常用 list item 已覆盖，缺 editor/widget/model/drag API |
| `AntListView` / `AntList` | `QListView` | 别名到 `AntList` | 不兼容 model/view API |
| `AntTableWidget` / `AntTable` | `QTableWidget` | 自绘 `QWidget` | 数据表 API 与 Qt item table 差异较大 |
| `AntTableView` / `AntTable` | `QTableView` | 别名到 `AntTable` | 不兼容 model/view API |
| `AntTreeWidget` / `AntTree` | `QTreeWidget` | 自绘 `QWidget` | key-based tree，缺 Qt item tree API |
| `AntTreeView` / `AntTree` | `QTreeView` | 别名到 `AntTree` | 不兼容 model/view API |
| `AntCalendarWidget` / `AntCalendar` | `QCalendarWidget` | 自绘组合控件 | 日期选择覆盖基础，缺格式/导航/header API |
| `AntDateEdit` / `AntDatePicker` | `QDateEdit` | 自绘组合控件 | 日期值/range 覆盖，缺 `QDateTimeEdit` section/spin API |
| `AntTimeEdit` / `AntTimePicker` | `QTimeEdit` | 自绘组合控件 | 时间值/range 覆盖，缺 `QDateTimeEdit` section/spin API |
| `AntTabWidget` / `AntTabs` | `QTabWidget` | 自绘 `QWidget` + 内部 `QStackedWidget` | key-based tabs，缺大量 index/widget API |
| `AntStatusBar` | `QStatusBar` | 自绘 `QWidget` | message 覆盖，缺 widget 插入 API |
| `AntFileDialog` | `QFileDialog` | `AntDialog` + 自定义 file view | 核心流程覆盖，缺高级属性和信号 |
| `AntPlainTextEdit` | `QPlainTextEdit` | 继承 `QPlainTextEdit` | Qt API 基本保留 |
| `AntScrollArea` | `QScrollArea` | 继承 `QScrollArea` | Qt API 基本保留 |
| `AntScrollBar` | `QScrollBar` | 继承 `QScrollBar` | Qt API 基本保留 |
| `AntStackedWidget` | `QStackedWidget` | 继承 `QStackedWidget` | Qt API 基本保留 |
| `AntSplitter` | `QSplitter` | 继承 `QSplitter` | Qt API 基本保留 |
| `AntMenuBar` | `QMenuBar` | 继承 `QMenuBar` | Qt API 基本保留 |
| `AntToolBar` | `QToolBar` | 继承 `QToolBar` | Qt API 基本保留 |
| `AntDialog` | `QDialog` | 继承 `QDialog` | Qt API 基本保留 |
| `AntMainWindow` / `AntWindow` | `QMainWindow` | 继承 `QMainWindow` | Qt API 基本保留，窗口 frame 行为自定义 |
| `AntDockWidget` | `QDockWidget` | 继承 `QDockWidget` | Qt API 基本保留 |
| `AntDockManager` | `QMainWindow` / dock manager | 继承 `QMainWindow`，自研 dock 树 | 不是 Qt 原生 dock layout 的 drop-in 替代 |

## 详细缺口

### AntInput / AntLineEdit vs QLineEdit

已覆盖：

- `text` / `setText` / `clear` / `placeholderText`。
- `readOnly`、`maxLength`、`echoMode`、`alignment`。
- `cursorPosition`、`setSelection`、`selectAll`、`deselect`、`selectedText`、`hasSelectedText`。
- `copy` / `cut` / `paste` / `undo` / `redo`。
- `textChanged`、`textEdited`、`returnPressed`、`editingFinished`、`selectionChanged`、`inputRejected`。
- 公开 `QLineEdit* lineEdit() const`，可绕过 Ant wrapper 访问原生子控件。

主要缺口：

- 属性/API：`inputMask`、`displayText`、`frame`、`acceptableInput`、`modified`、`cursorMoveStyle`、`clearButtonEnabled`。
- validator/completer：`setValidator` / `validator`、`setCompleter` / `completer`。
- 选择与光标：`selectionStart`、`selectionEnd`、`selectionLength`、`cursorPositionAt`、`cursorForward`、`cursorBackward`、`cursorWordForward`、`cursorWordBackward`、`backspace`、`del`、`home`、`end`。
- 状态：`isUndoAvailable`、`isRedoAvailable`、`setModified`、`isModified`、`setDragEnabled`、`dragEnabled`。
- 边距/action：`setTextMargins`、`textMargins`、`addAction(QAction*, ActionPosition)`、`addAction(QIcon, ActionPosition)`。
- 信号：`cursorPositionChanged(int oldPos, int newPos)`。

建议：

- P1：先补 `validator`、`completer`、`inputMask`、`displayText`、`hasAcceptableInput`、`cursorPositionChanged`、`selectionStart/End/Length`。
- P2：再补 cursor 操作、textMargins、addAction、modified/undo 状态。
- 如果继续暴露 `lineEdit()`，文档里应明确哪些高级 API 需要通过内部 `QLineEdit` 调用。

### AntSelect / AntComboBox vs QComboBox

已覆盖：

- `editable`、`currentIndex`、`currentText`、`currentData`、`setCurrentText`。
- `addItem` / `addItems` / `insertItem` / `insertItems` / `removeItem` / `clear`。
- `itemText` / `setItemText` / `itemData` / `setItemData`。
- `findText` / `findData`。
- `maxVisibleItems`。
- `activated`、`textActivated`、`highlighted`、`textHighlighted`、`currentIndexChanged`、`currentTextChanged`。

主要缺口：

- model/view/delegate：`setModel` / `model`、`setView` / `view`、`setItemDelegate` / `itemDelegate`、`setRootModelIndex`、`setModelColumn`。
- editable 相关：`lineEdit()`、`setLineEdit()`、`setValidator()`、`validator()`、`setCompleter()`、`completer()`、`setEditText()`、`clearEditText()`、`editTextChanged(QString)`。
- item 视觉：`setItemIcon()` / `itemIcon()`、`iconSize`、`insertSeparator()`。
- 策略属性：`insertPolicy`、`sizeAdjustPolicy`、`minimumContentsLength`、`duplicatesEnabled`、`frame`、`maxCount`。
- 弹层函数：`showPopup()`、`hidePopup()` 的 Qt 命名接口；当前对应 `setOpen(bool)`。

建议：

- P1：补 `lineEdit()`、`setEditText()`、`clearEditText()`、`editTextChanged`、`itemIcon`、`iconSize`、`insertSeparator`、`showPopup/hidePopup`。
- P2：补 `insertPolicy`、`duplicatesEnabled`、`minimumContentsLength`、`sizeAdjustPolicy`。
- P3：是否支持 `setModel` / `setView` / delegate 需要架构决策；若不支持，应在 `AntComboBox` 文档声明它是 AntD Select，不是完整 `QComboBox` model/view 替代。

### AntInputNumber / AntDoubleSpinBox / AntSpinBox vs QDoubleSpinBox / QSpinBox

已覆盖：

- `AntInputNumber` 继承 `QDoubleSpinBox`，因此 `QDoubleSpinBox` / `QAbstractSpinBox` 的多数 API 和信号天然可用。
- Ant 侧额外提供 `inputSize`、`status`、`variant`、`controlsVisible`、`placeholderText`、`prefixText`、`suffixText`、`precision`。

主要缺口：

- `AntDoubleSpinBox` 作为 `QDoubleSpinBox` 替代基本没有明显接口缺口。
- `AntSpinBox` 当前也是 `AntInputNumber` 别名，实际是 double-spinbox-backed，不是 `QSpinBox`：
  - 缺少 `int valueChanged(int)` / `int value()` 语义。
  - 缺少 `displayIntegerBase`。
  - `minimum` / `maximum` / `singleStep` 使用 double 语义，而不是严格 int。

建议：

- P1：若 `AntSpinBox` 要作为 `QSpinBox` 替代，应新增真正 `AntSpinBox` 类，或至少提供 int API wrapper 和 int 信号。
- P2：文档明确 `AntInputNumber` / `AntDoubleSpinBox` 是 double spin box 兼容，`AntSpinBox` 不是完整 `QSpinBox` drop-in。

### AntCheckBox vs QCheckBox

已覆盖：

- `checked`、`checkState`、`tristate`、`indeterminate`、`text`。
- `toggle()`、`click()`。
- `checkedChanged`、`checkStateChanged`、`stateChanged(int)`、`toggled(bool)`、`clicked(bool)`。

主要缺口：

- `QAbstractButton` 属性/API：`icon`、`iconSize`、`shortcut`、`checkable`、`down`、`autoRepeat`、`autoRepeatDelay`、`autoRepeatInterval`。
- 槽函数：`animateClick(int)`。
- 信号：`pressed()`、`released()`。

建议：

- P1：补 `pressed` / `released`、`animateClick`、`setDown/isDown`。
- P2：补 `icon/iconSize`、`shortcut`、`autoRepeat` 系列；如果 AntD CheckBox 不计划支持图标，应在文档声明。

### AntRadio / AntRadioButton vs QRadioButton

已覆盖：

- `checked`、`text`、`value`、`autoExclusive`、`buttonStyle`。
- `toggle()`、`click()`。
- `toggled(bool)`、`clicked(bool)`。

主要缺口：

- 与 `QAbstractButton` 相同：`icon`、`iconSize`、`shortcut`、`checkable`、`down`、`autoRepeat`、`animateClick`。
- 信号：`pressed()`、`released()`。

建议：

- P1：补 `pressed` / `released`、`animateClick`、`setDown/isDown`。
- P2：补 `icon/iconSize`、`shortcut`、`autoRepeat`。

### AntSlider vs QSlider / QAbstractSlider

已覆盖：

- `minimum`、`maximum`、`range`、`value`、`sliderPosition`。
- `singleStep`、`pageStep`、`orientation`、`tracking`、`invertedAppearance`。
- `valueChanged`、`sliderMoved`、`sliderPressed`、`sliderReleased`、`rangeChanged`。
- Ant 侧额外支持 range slider、marks、dots、included、value bubble。

主要缺口：

- `QAbstractSlider`：`invertedControls`、`sliderDown`、`triggerAction(SliderAction)`、`setRepeatAction()`、`repeatAction()`、`actionTriggered(int)`。
- `QSlider`：`tickPosition`、`tickInterval`。

建议：

- P1：补 `invertedControls`、`sliderDown`、`actionTriggered` 和 `triggerAction`，这些会影响键盘和程序化控制兼容性。
- P2：补 `tickPosition/tickInterval`，可映射到当前 marks/dots 绘制策略。

### AntProgress / AntProgressBar vs QProgressBar

已覆盖：

- `minimum`、`maximum`、`range`、`value`、`reset()`。
- `textVisible` / `showInfo`。
- `valueChanged`、`rangeChanged`。
- Ant 侧额外支持 circle/dashboard/status/strokeWidth/circleSize。

主要缺口：

- 文本：`text()`、`format()`、`setFormat()`、`resetFormat()`。
- 布局：`alignment`、`orientation`。
- 方向：`invertedAppearance`、`textDirection`。
- 命名兼容：Qt 使用 `isTextVisible()`，当前是 `textVisible()`。

建议：

- P1：补 `format` / `setFormat` / `resetFormat` / `text()`，这是 `QProgressBar` 最常用差异。
- P2：补 `alignment`、`orientation`、`invertedAppearance`、`textDirection`、`isTextVisible()` alias。

### AntTypography / AntLabel vs QLabel

已覆盖：

- `text`、`clear`、`alignment`、`wordWrap`。
- `linkActivated`。
- Ant Typography 的 title/paragraph/strong/underline/code/mark/copyable/ellipsis/href。

主要缺口：

- 媒体内容：`setPixmap` / `pixmap`、`setPicture`、`setMovie` / `movie`。
- 数值槽：`setNum(int)`、`setNum(double)`。
- 布局属性：`scaledContents`、`margin`、`indent`。
- 富文本/交互：`textFormat`、`openExternalLinks`、`textInteractionFlags`、`setSelection`、`selectedText`、`selectionStart`。
- buddy：`setBuddy` / `buddy`。
- 信号：`linkHovered(QString)`。

建议：

- P1：如果 `AntLabel` 要替代 `QLabel`，先补 `setPixmap/pixmap`、`setNum`、`textFormat`、`openExternalLinks`、`textInteractionFlags`、`linkHovered`。
- P2：补 `margin`、`indent`、`scaledContents`、selection 和 buddy。
- 若定位为 Ant Typography 而非 QLabel，则建议将 `AntLabel` 文档标为“文本标签替代，不覆盖 QLabel 媒体能力”。

### AntList / AntListWidget / AntListView vs QListWidget / QListView

已覆盖：

- `addItem` / `addItems` / `insertItem` / `insertItems` / `takeItem` / `clear`。
- `item` / `itemAt` / `row` / `findItems` / `sortItems`。
- `currentItem` / `currentRow` / `setCurrentItem` / `setCurrentRow`。
- `selectionMode`、`selectedItems`、`setItemSelected`。
- `scrollToItem`。
- 信号：`itemClicked`、`itemDoubleClicked`、`itemActivated`、`itemChanged`、`currentItemChanged`、`currentRowChanged`、`itemSelectionChanged`。
- `AntListItem` 已支持 text、icon、data、checkState、flags、selected，以及 AntIcon / `QPixmap` / `QImage` 媒体内容。

主要缺口：

- `QListWidget`：`setSortingEnabled` / `isSortingEnabled`、`itemPressed`、`itemEntered`、`currentTextChanged`。
- editor：`editItem`、`openPersistentEditor`、`closePersistentEditor`、`isPersistentEditorOpen`。
- item widget：`setItemWidget`、`itemWidget`、`removeItemWidget`。
- index bridge：`indexFromItem`、`itemFromIndex`。
- drag/drop/mime：`mimeData`、`dropMimeData`、`supportedDropActions`、`items(const QMimeData*)`。
- `QListView` / model-view：`setModel`、`model`、`selectionModel`、`setItemDelegate`、`rootIndex`、`scrollTo(QModelIndex)`、`clicked(QModelIndex)` 等完全没有。

建议：

- P1：为 `AntListWidget` 补 `setSortingEnabled`、`itemPressed`、`itemEntered`、`currentTextChanged`、`setItemWidget/itemWidget/removeItemWidget`。
- P2：补 editor 和 index bridge。
- P1 文档决策：`AntListView` 目前不应宣称为 `QListView` drop-in；若需要 model/view 替代，应新增真正 view-backed 控件或公开内部 model。

### AntTable / AntTableWidget / AntTableView vs QTableWidget / QTableView

已覆盖：

- Ant Table 风格 columns/rows 管理。
- `rowCount`、`columnCount`、`rows()`、`selectRow()`、`currentRowIndex()`、`selectedRowKeys()`。
- `cellData(row, dataIndex)`、`setData(row, dataIndex, value)`。
- 排序、分页、loading、bordered、rowSelection。
- 信号：`rowClicked`、`selectionChanged`、`sortChanged`、`pageChanged`、`cellDataChanged`。

主要缺口：

- `QTableWidget` item API：`setRowCount`、`setColumnCount`、`item`、`setItem`、`takeItem`、`clearContents`。
- header item API：`horizontalHeaderItem`、`verticalHeaderItem`、`setHorizontalHeaderItem`、`setVerticalHeaderItem`、`setHorizontalHeaderLabels`、`setVerticalHeaderLabels`。
- current cell API：`currentRow`、`currentColumn`、`currentItem`、`setCurrentCell`、`setCurrentItem`。
- row/column mutation：`insertRow`、`insertColumn`、`removeColumn`。
- widget/editor：`setCellWidget`、`cellWidget`、`removeCellWidget`、`editItem`、persistent editor。
- selection：`selectedItems`、`selectedRanges`、`setRangeSelected`。
- 查找/定位：`findItems`、`itemAt`、`visualItemRect`、`visualRow`、`visualColumn`、`scrollToItem`。
- 信号：`itemPressed`、`itemClicked`、`itemDoubleClicked`、`itemActivated`、`itemChanged`、`cellClicked`、`cellDoubleClicked`、`cellActivated`、`cellChanged`、`currentCellChanged`、`currentItemChanged`、`itemSelectionChanged`。
- `QTableView` / model-view：`setModel`、`model`、`selectionModel`、`horizontalHeader`、`verticalHeader`、`setItemDelegate`、`sortByColumn` 等没有。

建议：

- P1：若 `AntTableWidget` 是目标替代名，应补最常用的 `setRowCount/setColumnCount`、`setHorizontalHeaderLabels`、`setItem/item`、`currentRow/currentColumn`、`cellClicked/currentCellChanged`。
- P2：补 cell widget、selection ranges、find/visual/scroll API。
- P1 文档决策：`AntTableView` 不应标为 `QTableView` drop-in，除非后续接入 model/view。

### AntTree / AntTreeWidget / AntTreeView vs QTreeWidget / QTreeView

已覆盖：

- key-based tree data：`setTreeData`、`addNode`、`removeNode`、`findNode`、`containsNode`。
- expanded/selected/checked key 管理。
- `setNodeExpanded`、`setNodeChecked`。
- 信号：`nodeExpanded`、`nodeSelected`、`nodeChecked`。

主要缺口：

- `QTreeWidget` item API：`topLevelItem`、`addTopLevelItem`、`insertTopLevelItem`、`takeTopLevelItem`、`indexOfTopLevelItem`。
- header/column：`setColumnCount`、`columnCount`、`headerItem`、`setHeaderItem`、`setHeaderLabel(s)`。
- current/selection：`currentItem`、`currentColumn`、`setCurrentItem`、`selectedItems`。
- hit/visual：`itemAt`、`visualItemRect`、`scrollToItem`。
- expand/collapse：`expandItem`、`collapseItem`。
- widget/editor：`setItemWidget`、`itemWidget`、`removeItemWidget`、`editItem`。
- sorting：`sortItems`、`sortColumn`。
- 信号：`itemPressed`、`itemClicked`、`itemDoubleClicked`、`itemActivated`、`itemEntered`、`itemChanged`、`itemExpanded`、`itemCollapsed`、`currentItemChanged`、`itemSelectionChanged`。
- `QTreeView` / model-view：`setModel`、`model`、`setRootIndex`、`selectionModel`、`setItemDelegate`、`expanded(QModelIndex)`、`clicked(QModelIndex)` 等没有。

建议：

- P1：若要覆盖 `AntTreeWidget`，建议先补 item-style wrapper 或新增 `AntTreeWidgetItem`。
- P2：补 header/column、current/selection、item signals。
- P1 文档决策：`AntTreeView` 不应标为 `QTreeView` drop-in，除非后续接入 model/view。

### AntDatePicker / AntDateEdit vs QDateEdit / QDateTimeEdit

已覆盖：

- `date` / `setDate`、`selectedDate`。
- `minimumDate`、`maximumDate`、`setDateRange`、clear min/max。
- `displayFormat`、`placeholderText`。
- `dateChanged`、`selectedDateChanged`、`dateRangeChanged`、`cleared`。
- Ant 侧 range picker：`startDate`、`endDate`。

主要缺口：

- `QDateTimeEdit` 通用：`dateTime`、`minimumDateTime`、`maximumDateTime`、`setDateTimeRange`。
- section API：`currentSection`、`displayedSections`、`currentSectionIndex`、`sectionCount`、`sectionText`、`setSelectedSection`。
- calendar API：`calendarPopup`、`calendarWidget`、`setCalendarWidget`、`setCalendar`。
- spinbox API：`readOnly`、`alignment`、`buttonSymbols`、`keyboardTracking`、`wrapping`、`specialValueText`、`accelerated`、`correctionMode`、`editingFinished`、`stepUp`、`stepDown`、`selectAll`、`lineEdit`。
- 信号：`userDateChanged(QDate)`。

建议：

- P1：补 `userDateChanged`、`dateTime` alias、`readOnly`、`alignment`、`editingFinished`，满足常见表单迁移。
- P2：补 section API 或明确 Ant DatePicker 不支持 spin-section 编辑。

### AntTimePicker / AntTimeEdit vs QTimeEdit / QDateTimeEdit

已覆盖：

- `time` / `setTime`、`selectedTime`。
- `minimumTime`、`maximumTime`、`setTimeRange`、clear min/max。
- `displayFormat`、`placeholderText`。
- `hourStep`、`minuteStep`、`secondStep`、`showNow`。
- `timeChanged`、`selectedTimeChanged`、`timeRangeChanged`、`accepted`、`cleared`。
- Ant 侧 range picker：`startTime`、`endTime`。

主要缺口：

- 与 `AntDatePicker` 相同的 `QDateTimeEdit` 通用接口：`dateTime`、date-time range、section API、spinbox API。
- 信号：`userTimeChanged(QTime)`。

建议：

- P1：补 `userTimeChanged`、`dateTime` alias、`readOnly`、`alignment`、`editingFinished`。
- P2：补 section API 或明确 Ant TimePicker 不支持 spin-section 编辑。

### AntCalendar / AntCalendarWidget vs QCalendarWidget

已覆盖：

- `selectedDate`、`minimumDate`、`maximumDate`。
- `calendarMode`。
- `clicked(QDate)`、`selectedDateChanged(QDate)`。

主要缺口：

- 布局/导航：`firstDayOfWeek`、`gridVisible`、`selectionMode`、`navigationBarVisible`、`dateEditEnabled`、`dateEditAcceptDelay`。
- header：`horizontalHeaderFormat`、`verticalHeaderFormat`。
- 当前页：`yearShown`、`monthShown`、`setCurrentPage`。
- 导航槽：`showNextMonth`、`showPreviousMonth`、`showNextYear`、`showPreviousYear`、`showToday`、`showSelectedDate`。
- 文本格式：`headerTextFormat`、`weekdayTextFormat`、`dateTextFormat`。
- 信号：`selectionChanged()`、`activated(QDate)`、`currentPageChanged(int year, int month)`。

建议：

- P1：补 `firstDayOfWeek`、`gridVisible`、`selectionMode`、`yearShown/monthShown`、`setCurrentPage`、`currentPageChanged`。
- P2：补 text format 和 header format。

### AntTabs / AntTabWidget vs QTabWidget

已覆盖：

- `addTab(QWidget*, key, label, iconText, disabled, closable)`。
- `removeTab(key)`、`clearTabs()`、`setTabText(key)`、`setTabEnabled(key)`。
- `activeKey`。
- 信号：`currentChanged(int)`、`tabClicked(key)`、`tabCloseRequested(key)`、`tabAddRequested()`。

主要缺口：

- index/widget API：`addTab(QWidget*, QString)`、`addTab(QWidget*, QIcon, QString)`、`insertTab`、`removeTab(int)`、`count`、`currentIndex`、`setCurrentIndex`、`currentWidget`、`setCurrentWidget`、`widget(index)`、`indexOf(widget)`。
- tab data：`tabText(index)`、`tabIcon`、`setTabIcon`、`tabToolTip`、`tabWhatsThis`。
- behavior：`setTabVisible`、`tabsClosable`、`movable`、`iconSize`、`elideMode`、`usesScrollButtons`、`documentMode`、`tabBarAutoHide`。
- corner：`setCornerWidget`、`cornerWidget`。
- 信号：`tabCloseRequested(int)`、`tabBarClicked(int)`、`tabBarDoubleClicked(int)`。

建议：

- P1：补 index/widget API 和 int-based `tabCloseRequested(int)`，否则 `AntTabWidget` 迁移成本较高。
- P2：补 `tabIcon`、`iconSize`、`tabsClosable`、`movable`、`setTabVisible`。

### AntFileDialog vs QFileDialog

已覆盖：

- 静态函数：`getOpenFileName`、`getOpenFileNames`、`getSaveFileName`、`getExistingDirectory`。
- `acceptMode`、`fileMode`、`nameFilters`、`selectedNameFilter`。
- `directory`、`selectFile`、`selectedFiles`。
- `options`、`defaultSuffix`。

主要缺口：

- view/label：`viewMode`、`setLabelText`、`labelText`。
- history/sidebar：`setHistory`、`history`、`setSidebarUrls`、`sidebarUrls`。
- provider/delegate/proxy：`setIconProvider`、`iconProvider`、`setItemDelegate`、`itemDelegate`、`setProxyModel`、`proxyModel`。
- URL/mime：`selectUrl`、`selectedUrls`、`directoryUrl`、`setDirectoryUrl`、`mimeTypeFilters`、`selectMimeTypeFilter`、`selectedMimeTypeFilter`、`supportedSchemes`。
- state：`saveState`、`restoreState`。
- async：`open(QObject*, const char*)`。
- 信号：`fileSelected`、`filesSelected`、`currentChanged`、`directoryEntered`、`filterSelected`、`urlSelected`、`urlsSelected`、`currentUrlChanged`、`directoryUrlEntered`。

建议：

- P1：补 `fileSelected/filesSelected/currentChanged/directoryEntered/filterSelected` 信号，以及 `history/sidebarUrls/viewMode/labelText`。
- P2：补 mime/url/proxy/iconProvider/saveState。
- 如果不支持 native QFileDialog 的 URL/mime/proxy 能力，文档应明确是 Ant Design file picker，不是完整 QFileDialog。

### AntStatusBar vs QStatusBar

已覆盖：

- `message`、`currentMessage`、`showMessage`、`clearMessage`。
- `messageChanged`。
- 文本 item / permanent item 的添加和移除。
- `sizeGrip`，但命名为 `hasSizeGrip()` / `setSizeGrip()`。

主要缺口：

- widget API：`addWidget`、`insertWidget`、`addPermanentWidget`、`insertPermanentWidget`、`removeWidget`。
- Qt 命名：`isSizeGripEnabled()`、`setSizeGripEnabled()`。
- `reformat()`。

建议：

- P1：补 widget 插入和移除 API，这是 `QStatusBar` 最常见用法。
- P2：补 `isSizeGripEnabled/setSizeGripEnabled` alias。

## 直接继承 Qt 的控件

以下控件继承 Qt 官方控件，接口兼容风险较低，主要关注 Ant 自定义视觉是否影响行为即可：

- `AntButton` -> `QPushButton`
- `AntToolButton` -> `QToolButton`
- `AntInputNumber` / `AntDoubleSpinBox` -> `QDoubleSpinBox`
- `AntPlainTextEdit` -> `QPlainTextEdit`
- `AntScrollArea` -> `QScrollArea`
- `AntScrollBar` -> `QScrollBar`
- `AntStackedWidget` -> `QStackedWidget`
- `AntSplitter` -> `QSplitter`
- `AntMenuBar` -> `QMenuBar`
- `AntToolBar` -> `QToolBar`
- `AntDialog` -> `QDialog`
- `AntWindow` / `AntMainWindow` -> `QMainWindow`
- `AntDockWidget` -> `QDockWidget`
- `AntDockManager` -> `QMainWindow`，但 dock 布局是项目自研体系，不等同于 Qt 原生 dock layout。

## 建议优先级

| 优先级 | 建议项 | 原因 |
| --- | --- | --- |
| P1 | 补 `AntInput` 的 validator/completer/inputMask/displayText/cursorPositionChanged | 表单迁移最常见，且可直接转发内部 `QLineEdit` |
| P1 | 补 `AntSelect` 的 lineEdit/editTextChanged/itemIcon/showPopup/hidePopup | `AntComboBox` 作为 QComboBox 替代时最容易踩坑 |
| P1 | 补 `AntStatusBar` 的 widget 插入 API | `QStatusBar` 的核心用法之一 |
| P1 | 补 `AntFileDialog` 的核心信号和 history/sidebar/viewMode/labelText | QFileDialog 迁移常用 |
| P1 | 明确 `AntListView` / `AntTableView` / `AntTreeView` 不是 model/view drop-in | 当前别名会让用户误以为支持 `setModel()` |
| P1 | 明确或重做 `AntSpinBox` | 当前实际是 `QDoubleSpinBox` 语义 |
| P2 | 补 `AntTabs` 的 index/widget API | `QTabWidget` 迁移常用 |
| P2 | 补 `AntProgress` 的 format/text/orientation/invertedAppearance | `QProgressBar` 常用属性 |
| P2 | 补 `AntSlider` 的 invertedControls/sliderDown/action/tick API | 程序化控制和键盘行为兼容 |
| P2 | 补 `AntCalendar` 的 firstDayOfWeek/grid/currentPage/header/textFormat | `QCalendarWidget` 常用配置 |
| P2 | 为 `AntTableWidget` / `AntTreeWidget` 设计 item wrapper | 完整补齐工作量大，但这是 drop-in 兼容的核心 |

## 建议的实现策略

1. 对内部已经持有 Qt 子控件的 wrapper，优先做薄转发，例如 `AntInput` 转发到内部 `QLineEdit`，`AntSelect` editable 模式转发到内部 `QLineEdit`。
2. 对直接继承 Qt 的控件，不要重复封装 Qt 已有 API，只补 Ant 特有属性和行为测试。
3. 对 `AntListView` / `AntTableView` / `AntTreeView` 这类 model/view 名称，建议二选一：
   - 改文档：声明它们是 Ant 自绘数据展示控件，不是 Qt model/view drop-in。
   - 改架构：新增真正基于 `QAbstractItemView` 的 wrapper 或公开内部 model/view 层。
4. 对 `AntTableWidget` / `AntTreeWidget`，如果要追求 Qt item-widget 兼容，应新增 `AntTableWidgetItem` / `AntTreeWidgetItem` 风格对象；直接把所有 `QTableWidgetItem` / `QTreeWidgetItem` API 映射到当前 data struct 会比较别扭。
5. 后续补接口时建议给每个替代别名加编译期 API smoke test，测试内容只写“Qt 官方常用代码能否用 Ant 别名编译通过”，避免视觉测试和 API 兼容混在一起。
