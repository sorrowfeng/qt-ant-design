# Qt 5.15.2 Visual Adaptation Report

Updated: `2026-06-01`

This report tracks evidence for the Qt 5.15.2 visual-adaptation goal. It is not a claim that the entire long-running objective is complete; it records the current verified pass and the fixes made during this pass.

## Current Evidence

Qt 5.15.2 and Qt 6.9.1 now share a repeatable cross-version visual parity check:

- Test target: `TestAntQtVersionVisualParity`
- Default mode: renders 26 critical Ant Design scenes in both light and dark themes, for 52 themed atlas snapshots, and fails on blank/invalid output.
- Export mode: set `ANT_QT_PARITY_EXPORT_DIR` to write one PNG atlas per themed scene plus `manifest.tsv`. Filenames use `light-*` and `dark-*` prefixes.
- Compare mode: set `ANT_QT_PARITY_BASELINE_DIR` to compare current renders against another Qt version's exported atlas and write `comparison.tsv`.
- Implementation uses only QWidget/QPainter rendering, token palettes, and existing widget APIs. No QSS/QStyleSheet is used.
- The atlas test also guards public-header coverage: each public widget header must be covered by a scene or be explicitly listed as a non-static-atlas utility/controller (`AntAffix`, `AntApp`, `AntConfigProvider`).

A separate metric audit now records the non-visual root causes called out by the Qt5 adaptation goal:

- Test target: `TestAntQtVersionMetricAudit`
- Export mode: set `ANT_QT_METRIC_EXPORT_DIR` to write `metric-audit.tsv`.
- Compare mode: set `ANT_QT_METRIC_BASELINE_DIR` to compare the current Qt version against a previous export and write `metric-comparison.tsv`.
- The audit records Qt default `QStyle::pixelMetric()` values, Qt default `QPalette` roles, resolved font metrics, direct default `drawControl()` / `drawPrimitive()` / `drawComplexControl()` render fingerprints, `QComboBox` / `QTabBar` / `QHeaderView` / `QScrollBar` default geometry, and adapted Ant widget size/font/palette/style geometry.
- Qt default values are audit-only because they document root-cause drift; adapted Ant values are compared with tight tolerances.
- Latest Qt5-vs-Qt6 metric comparison passed `216` rows: `105` Qt-default audit rows and `111` adapted Ant strict rows, with `0` failures. Notable default drift included style identity (`windowsvista` vs `windows11`), `PM_ScrollBarExtent` (`17` vs `12`), indicator metrics (`13` vs `16`), and different default render hashes for `CE_PushButton`, `CE_CheckBox`, `PE_PanelLineEdit`, `CC_ComboBox`, `CE_TabBarTab`, `CE_Header`, and `CC_ScrollBar`, while Ant adapted metrics remained within tolerance. Strict Ant rows now include the success-standard controls `AntMenu`, `AntPagination`, `AntTag`, `AntBadge`, `AntTable`, and `AntToolTip` in addition to the earlier Button/Input/Select/CheckBox/Progress/Tabs/ScrollBar/List/Tree sample.

Windows scale-factor coverage is also automated:

- Test entries: `TestAntHighDpiScaling_1_0`, `TestAntHighDpiScaling_1_25`, and `TestAntHighDpiScaling_1_5`.
- Each entry starts a fresh process with `QT_SCALE_FACTOR` set before `QApplication`, then checks the High DPI startup policy, representative Ant widgets, `AntList` pixmap items, `AntNav`, and `AntWindow` grab geometry.
- Full visual-atlas smoke entries `TestAntQtVersionVisualParity_Scale_1_25` and `TestAntQtVersionVisualParity_Scale_1_5` also run the 26-scene light/dark atlas under fractional scale factors, so public-header atlas coverage and nonblank rendering are checked at 125% and 150% scaling.
- Qt5 example smoke launches passed at `QT_SCALE_FACTOR=1.25` and `QT_SCALE_FACTOR=1.5`.

The real example program is now covered by a separate traversal/export path:

- `qt-ant-design-example --smoke-traverse-pages` selects all 88 example pages in light and dark themes.
- Export mode writes stable top/bottom page grabs and `manifest.tsv`; compare mode compares those grabs against another Qt version and writes `comparison.tsv`.
- The latest full-page Qt5-vs-Qt6 example comparison covered 352 frames (`88` pages x `2` themes x `2` scroll probes). It passed with mean pixel deltas from `0.018` to `0.576`, average mean delta `0.043136`, maximum `changed32Ratio` `0.0043`, and maximum `changed64Ratio` `0.0032`.

Latest local comparison used Qt 6.9.1 as the baseline and Qt 5.15.2 as the actual renderer. Both light and dark theme renders passed:

| Scene | Covered controls | Mean delta | Limit | Changed >32 | Limit |
| --- | --- | ---: | ---: | ---: | ---: |
| `light-buttons` | Button states, standalone Icon, icon-only text button | `1.030` | `8.000` | `0.0092` | `0.1800` |
| `light-selection-controls` | CheckBox, Radio, Switch | `1.322` | `8.000` | `0.0155` | `0.2000` |
| `light-input-controls` | Input, InputNumber, Select, DatePicker, TimePicker | `1.178` | `12.000` | `0.0122` | `0.3000` |
| `light-navigation` | Menu, Tabs, Pagination | `0.700` | `14.000` | `0.0068` | `0.3200` |
| `light-feedback` | Alert, Tag, Badge, Progress, ToolTip | `0.991` | `12.000` | `0.0118` | `0.2800` |
| `light-popover-surfaces` | Popover arrow, shadow, icon header, action area | `1.751` | `16.000` | `0.0174` | `0.3600` |
| `light-modal-surface` | Modal mask, rounded card, shadow, footer buttons | `1.338` | `22.000` | `0.0121` | `0.4600` |
| `light-drawer-surface` | Drawer mask, edge shadow, header, body action | `0.288` | `22.000` | `0.0035` | `0.4600` |
| `light-dropdown-surface` | Dropdown trigger, real top-level popup, arrow, menu rows | `1.513` | `18.000` | `0.0159` | `0.4200` |
| `light-popconfirm-surface` | Popconfirm warning icon, Popover shell, action buttons | `2.117` | `18.000` | `0.0220` | `0.4200` |
| `light-tour-surface` | Tour mask, spotlight clear, tooltip, navigation buttons | `1.390` | `22.000` | `0.0144` | `0.4600` |
| `light-interaction-states` | Button hover/down, Input focus/status, CheckBox hover, Radio pressed, Slider focus, Menu/Tabs/Pagination/Table hover | `0.501` | `18.000` | `0.0060` | `0.4200` |
| `light-acceptance-state-matrix` | Button, CheckBox, Radio, Input, Progress, Tag, Badge, ToolTip, Menu, Tabs, Pagination, List, Table, Tree, ScrollBar state matrix | `1.228` | `18.000` | `0.0128` | `0.4200` |
| `light-data-display` | List, Table | `0.570` | `16.000` | `0.0069` | `0.3600` |
| `light-tree` | Tree expand/check/show-line rendering | `0.231` | `10.000` | `0.0031` | `0.2800` |
| `light-scroll-carousel` | ScrollArea, ScrollBar, Carousel arrows/dots | `1.381` | `14.000` | `0.0144` | `0.3000` |
| `light-desktop-extensions` | Nav, Dialog, FloatButton | `0.609` | `18.000` | `0.0067` | `0.4000` |
| `light-advanced-entry` | AutoComplete, Mentions, Cascader, TreeSelect, ColorPicker, Slider range/marks, Rate, Segmented, Transfer, Upload | `0.733` | `18.000` | `0.0089` | `0.4200` |
| `light-rich-display` | AvatarGroup, Card, Calendar, Descriptions, Empty, QRCode, Timeline, Statistic, Image | `1.835` | `22.000` | `0.0159` | `0.4600` |
| `light-rich-feedback` | Result, Notification, Message, Spin, Skeleton, Collapse | `0.228` | `22.000` | `0.0027` | `0.4600` |
| `light-layout-extensions` | Breadcrumb, Steps, Divider, Layout, Splitter, StackedWidget, ToolBar, ToolButton, PlainTextEdit, StatusBar, Log | `1.099` | `20.000` | `0.0091` | `0.4200` |
| `light-structural-layout` | Anchor, Flex, Space, Grid, Masonry, Widget, Form, Watermark | `0.510` | `22.000` | `0.0052` | `0.4600` |
| `light-desktop-surfaces` | MenuBar, Ribbon | `0.451` | `18.000` | `0.0051` | `0.4000` |
| `light-dock-surfaces` | DockManager, DockWidget | `0.304` | `20.000` | `0.0032` | `0.4200` |
| `light-file-dialog` | FileDialog | `1.117` | `20.000` | `0.0119` | `0.4200` |
| `light-window-frame` | Window | `0.119` | `20.000` | `0.0012` | `0.4200` |
| `dark-buttons` | Button states, standalone Icon, icon-only text button | `1.027` | `8.000` | `0.0105` | `0.1800` |
| `dark-selection-controls` | CheckBox, Radio, Switch | `1.353` | `8.000` | `0.0155` | `0.2000` |
| `dark-input-controls` | Input, InputNumber, Select, DatePicker, TimePicker | `1.048` | `12.000` | `0.0113` | `0.3000` |
| `dark-navigation` | Menu, Tabs, Pagination | `0.746` | `14.000` | `0.0104` | `0.3200` |
| `dark-feedback` | Alert, Tag, Badge, Progress, ToolTip | `0.905` | `12.000` | `0.0109` | `0.2800` |
| `dark-popover-surfaces` | Popover arrow, shadow, icon header, action area | `1.582` | `16.000` | `0.0162` | `0.3600` |
| `dark-modal-surface` | Modal mask, rounded card, shadow, footer buttons | `1.364` | `22.000` | `0.0107` | `0.4600` |
| `dark-drawer-surface` | Drawer mask, edge shadow, header, body action | `0.244` | `22.000` | `0.0028` | `0.4600` |
| `dark-dropdown-surface` | Dropdown trigger, real top-level popup, arrow, menu rows | `1.383` | `18.000` | `0.0152` | `0.4200` |
| `dark-popconfirm-surface` | Popconfirm warning icon, Popover shell, action buttons | `1.873` | `18.000` | `0.0210` | `0.4200` |
| `dark-tour-surface` | Tour mask, spotlight clear, tooltip, navigation buttons | `1.183` | `22.000` | `0.0131` | `0.4600` |
| `dark-interaction-states` | Button hover/down, Input focus/status, CheckBox hover, Radio pressed, Slider focus, Menu/Tabs/Pagination/Table hover | `0.457` | `18.000` | `0.0053` | `0.4200` |
| `dark-acceptance-state-matrix` | Button, CheckBox, Radio, Input, Progress, Tag, Badge, ToolTip, Menu, Tabs, Pagination, List, Table, Tree, ScrollBar state matrix | `1.099` | `18.000` | `0.0124` | `0.4200` |
| `dark-data-display` | List, Table | `0.544` | `16.000` | `0.0063` | `0.3600` |
| `dark-tree` | Tree expand/check/show-line rendering | `0.203` | `10.000` | `0.0019` | `0.2800` |
| `dark-scroll-carousel` | ScrollArea, ScrollBar, Carousel arrows/dots | `1.244` | `14.000` | `0.0140` | `0.3000` |
| `dark-desktop-extensions` | Nav, Dialog, FloatButton | `0.582` | `18.000` | `0.0063` | `0.4000` |
| `dark-advanced-entry` | AutoComplete, Mentions, Cascader, TreeSelect, ColorPicker, Slider range/marks, Rate, Segmented, Transfer, Upload | `0.652` | `18.000` | `0.0075` | `0.4200` |
| `dark-rich-display` | AvatarGroup, Card, Calendar, Descriptions, Empty, QRCode, Timeline, Statistic, Image | `4.383` | `22.000` | `0.0341` | `0.4600` |
| `dark-rich-feedback` | Result, Notification, Message, Spin, Skeleton, Collapse | `0.231` | `22.000` | `0.0029` | `0.4600` |
| `dark-layout-extensions` | Breadcrumb, Steps, Divider, Layout, Splitter, StackedWidget, ToolBar, ToolButton, PlainTextEdit, StatusBar, Log | `2.075` | `20.000` | `0.0091` | `0.4200` |
| `dark-structural-layout` | Anchor, Flex, Space, Grid, Masonry, Widget, Form, Watermark | `0.505` | `22.000` | `0.0052` | `0.4600` |
| `dark-desktop-surfaces` | MenuBar, Ribbon | `0.444` | `18.000` | `0.0048` | `0.4000` |
| `dark-dock-surfaces` | DockManager, DockWidget | `0.275` | `20.000` | `0.0030` | `0.4200` |
| `dark-file-dialog` | FileDialog | `5.942` | `20.000` | `0.0418` | `0.4200` |
| `dark-window-frame` | Window | `0.091` | `20.000` | `0.0010` | `0.4200` |

Generated evidence path for the latest local run:

- `Testing/qt-version-visual-parity-popups/qt6/manifest.tsv`
- `Testing/qt-version-visual-parity-popups/qt5/manifest.tsv`
- `Testing/qt-version-visual-parity-popups/qt5/comparison.tsv`
- `Testing/qt-version-metric-audit/qt6/metric-audit.tsv`
- `Testing/qt-version-metric-audit/qt5/metric-audit.tsv`
- `Testing/qt-version-metric-audit/qt5/metric-comparison.tsv`
- `Testing/example-page-traversal/qt6/manifest.tsv`
- `Testing/example-page-traversal/qt5/manifest.tsv`
- `Testing/example-page-traversal/qt5/comparison.tsv`

`Testing/` is ignored and is intentionally not committed.

## Requirement Evidence Matrix

| Objective requirement | Current evidence | Status |
| --- | --- | --- |
| Inspect all component styling, not only build success | `TestAntQtVersionVisualParity` guards public widget header coverage and renders 26 representative scenes in light/dark, covering every public visual header except utility/controller-only `AntAffix`, `AntApp`, and `AntConfigProvider`. | Verified for the current atlas pass |
| Compare Qt5 and Qt6 visual output | Qt6 atlas export in `Testing/qt-version-visual-parity-popups/qt6` is used as the baseline for Qt5 export in `Testing/qt-version-visual-parity-popups/qt5`; `comparison.tsv` passed all 52 snapshots. | Verified |
| Analyze `QStyle::pixelMetric()` drift | `TestAntQtVersionMetricAudit` records Qt default pixel metrics and adapted Ant widget metrics; Qt default drift is audit-only, while strict Ant rows passed. | Verified |
| Analyze default `drawControl()` / `drawPrimitive()` / `drawComplexControl()` drift | Metric audit records direct default render fingerprints for push button, checkbox, line edit panel, combo box, tab, header, and scrollbar probes. All default probes differ, documenting why Ant widgets must rely on their own style/token paths. | Verified |
| Analyze `QPalette` role drift | Metric audit records Qt default palette role differences; adapted Ant palette rows passed strict comparison because widgets resolve colors from `AntTheme` tokens. | Verified |
| Analyze font rendering drift | Metric audit records resolved font metrics and strict adapted Ant font rows, which passed within tolerance. | Verified |
| Analyze High DPI effects | `AntDesign::configureHighDpi()` is called before `QApplication`; `TestAntHighDpiScaling_*` covers 1.0, 1.25, 1.5; `TestAntQtVersionVisualParity_Scale_1_25` and `_1_5` smoke-render the full atlas at fractional scaling. | Verified |
| Analyze complex child-control geometry drift | Metric audit records default `QComboBox`, `QTabBar`, `QHeaderView`, and `QScrollBar` complex geometry; adapted Ant geometry rows passed strict comparison. | Verified |
| Preserve Qt6 behavior | All adaptation tests run in both Qt6 and Qt5 build trees; shared fixes avoid Qt-version branches unless the root cause is startup API availability. | Verified |
| Forbid QSS/QStyleSheet | `TestAntNoStyleSheetUsage` scans production sources, examples, tests, and resources for stylesheet usage and passed in both Qt6 and Qt5 build trees. | Verified |
| Real example visual traversal | `qt-ant-design-example --smoke-traverse-pages` renders all 88 pages in light/dark and compares 352 Qt5 frames against Qt6 baseline frames. | Verified |
| Success-standard controls | `acceptance-state-matrix`, `interaction-states`, `navigation`, `feedback`, `data-display`, `tree`, and `scroll-carousel` scenes cover the named Button, CheckBox, Radio, Progress, Tabs, ScrollBar, Menu, ToolTip, Input, Table, List, Tree, Pagination, Tag, and Badge checks. | Verified |
| Detailed report and maintenance advice | This report lists changed files, affected controls, drift causes, adaptation mechanism, version-isolation approach, verification commands, and maintenance notes. | Verified for this pass |

## Qt Default Drift Checklist

These rows are intentionally audit-only: they document Qt5/Qt6 defaults that would cause drift if Ant controls delegated visual output to platform defaults. The adapted Ant rows in the same audit are strict and passed with `0` failures.

| Area | Probe | Qt 5.15.2 | Qt 6.9.1 | Adaptation implication |
| --- | --- | ---: | ---: | --- |
| Platform style | `QApplication::style()->objectName()` | `windowsvista` | `windows11` | Ant widgets must not depend on platform style identity for visual tokens. |
| Pixel metric | `PM_ScrollBarExtent` | `17` | `12` | Custom `AntScrollBar` geometry remains token-controlled instead of inheriting default extent. |
| Pixel metric | `PM_IndicatorWidth` / `PM_IndicatorHeight` | `13` | `16` | CheckBox/Radio indicator sizes are drawn by Ant styles rather than Qt defaults. |
| Pixel metric | `PM_ExclusiveIndicatorWidth` / `PM_ExclusiveIndicatorHeight` | `13` | `16` | Radio indicator geometry needs Ant-owned metrics. |
| Palette | active `Window` | `#FFF0F0F0` | `#FFF3F3F3` | Container backgrounds should come from `AntTheme` tokens. |
| Palette | active `Base` | `#FFFFFFFF` | `#B3FFFFFF` | Input/list/table surfaces should not rely on Qt default Base. |
| Palette | active `Text` / `ButtonText` | `#FF000000` | `#E4000000` | Text color opacity should be token-derived. |
| Palette | active `Highlight` | `#FF0078D4` | `#FF0067C0` | Selection/highlight fills should use Ant token colors. |
| Default drawing | `CE_PushButton`, `CE_CheckBox`, `PE_PanelLineEdit`, `CC_ComboBox`, `CE_TabBarTab`, `CE_Header`, `CC_ScrollBar` | hashes differ | hashes differ | Direct Qt default drawing is not visually stable across Qt5/Qt6; the atlas verifies Ant-owned paint paths instead. |
| Complex geometry | `QTabBar::tabRect()` | tab width `130` | tab width `320` | Tabs should use Ant geometry/cache paths. |
| Complex geometry | `QScrollBar::SC_ScrollBarSlider` | y `66`, h `24` | y `64`, h `26` | Scrollbar handle geometry should be Ant-owned and High-DPI checked. |

## Code Changes In This Pass

| File | Control / area | Difference or risk found | Fix | Version isolation |
| --- | --- | --- | --- | --- |
| `src/core/AntDesign.h`, `src/core/AntDesign.cpp` | Application startup / Windows High DPI | Qt5 needs High DPI attributes before `QApplication`; otherwise layouts, fonts, hit regions, grabs, and pixmaps can be interpreted in physical pixels and vary between 100%, 125%, and 150% displays. | Added `AntDesign::configureHighDpi()` to enable `AA_EnableHighDpiScaling`, `AA_UseHighDpiPixmaps`, and pass-through scale-factor rounding before app creation. `initialize()` warns on Qt5 if the startup path was skipped. | Qt5-only application attributes are behind `#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)`; pass-through rounding is guarded by `QT_VERSION >= 5.14`. |
| `examples/main.cpp` | Example startup and page traversal smoke | The example embedded a PerMonitorV2 manifest but did not enable Qt5 logical High DPI scaling before constructing `QApplication`. The previous example smoke only proved startup/close, not that every demo page can be selected and rendered under Qt5. | Calls `AntDesign::configureHighDpi()` before `QApplication`, then `AntDesign::initialize(&app)` after app creation. Added `--smoke-traverse-pages`, which traverses every example page in light and dark mode, scrolls pages with vertical content to top/bottom, grabs the real `ExampleWindow`, and exits non-zero on invalid/blank frames. Export/baseline options write stable PNG manifests and Qt5-vs-Qt6 comparisons; the traversal now explicitly exits the Qt event loop when complete so GUI-subsystem runs do not leave background processes writing partial evidence. | Shared source; Qt6 keeps its default High DPI behavior while Qt5 gets the required attributes. The smoke traversal uses only public QWidget grab/render paths. |
| `examples/ExampleWindow.h`, `examples/ExampleWindow.cpp` | Example page shell | The actual success criterion requires traversing all example pages. `ExampleWindow` previously kept the navigation and page stack private with no stable way for the command-line smoke path to drive pages by index. | Added lightweight example-only page inspection/selection helpers and stable object names for the sidebar, navigation, content, and stack. | Example-only API; no library ABI or Qt-version branch. |
| `src/widgets/AntInput.cpp`, `tests/TestAntInput.cpp` | `AntInput` / `AntLineEdit` wrapper | Qt5 `QLineEdit` can still erase its own widget rect after the parent `AntInput` frame has painted. Because the child edit previously occupied the full input height, the middle of the top/bottom outline could be cleared, leaving only the rounded corner segments visible. | Keeps the child edit, addon labels, and search button inside the 1px frame by using the token line width as the internal vertical inset, adds `WA_NoSystemBackground` to the child edit, and adds a regression test that verifies the child geometry no longer covers the frame and that focused top/bottom border pixels remain visible. | Shared layout/paint correctness fix; no Qt-version branch required. The regression runs under both Qt5 and Qt6. |
| `src/styles/AntInputNumberStyle.cpp`, `src/widgets/AntInputNumber.cpp`, `tests/TestAntDataEntryA.cpp` | `AntInputNumber` / `AntInputDialog` numeric input | Qt5 `QDoubleSpinBox` could continue the native spin-box paint path after Ant's custom frame and its internal `QLineEdit` could erase the top/bottom outline. In `AntInputDialog` integer/double mode this appeared as incomplete blue bracket-like borders around the numeric editor. | Stops the native spin-box paint after AntInputNumber draws its `CC_SpinBox`, keeps the edit field inset by the token line width, marks the internal editor with `WA_NoSystemBackground`, constrains the editor height inside the frame, and adds a regression test that verifies focused top/bottom border pixels remain visible. | Shared paint/layout correctness fix; no Qt-version branch required. The regression runs under both Qt5 and Qt6. |
| `src/widgets/AntList.cpp` | `AntListItem` QPixmap icons | Pixmap icons were scaled to logical icon rectangles without accounting for the paint device DPR, which could make custom item images softer or uneven at fractional Windows scale factors. | Scales the source pixmap to `logicalSize * DPR`, sets the pixmap DPR, and centers it back in logical coordinates before drawing. | Shared DPR-correct drawing path; no version branch required. |
| `src/styles/AntTreeStyle.cpp` | `AntTree` QIcon rendering | Tree icons requested a fixed 16x16 pixmap regardless of the paint device DPR, which reduced sharpness on 125%/150% displays. | Requests a DPR-sized icon pixmap, sets its device pixel ratio, and draws into the existing logical icon rect. | Shared DPR-correct drawing path; no version branch required. |
| `src/widgets/AntTree.cpp` | `AntTree` | `setTreeData()` could build the flattened-node cache while `m_data` still shared storage with the caller's `QVector`; later checked-state mutation could detach the vector and leave cached node pointers stale before paint. This surfaced during Qt5/Qt6 atlas rendering and is a real visual/runtime stability risk. | Re-invalidated the flat cache after `clampScrollY()` in `setTreeData()`, and invalidated the flat cache after checked-state mutations in `setCheckedKeys()`, `setNodeChecked()`, and checkbox mouse toggles. | No `QT_VERSION` branch: the root cause is Qt implicit-sharing semantics and can affect both Qt5 and Qt6. |
| `tests/TestAntQtVersionVisualParity.cpp` | Visual parity infrastructure | Existing tests proved token visibility and nonblank rendering, but did not compare Qt5 output against Qt6 output or guard whether public widgets had entered the atlas. It also needed dark-theme, interaction-state, and popup-surface parity coverage because Qt5/Qt6 palette, shadow, font, hover, pressed, focus, translucent mask, native top-level popup, and popup-window handling can diverge outside the default static state. | Added a cross-version atlas exporter/comparator and expanded it to 26 representative scenes covering the main acceptance areas plus a consolidated acceptance state matrix, interaction states, Popover, Modal, Drawer, Dropdown, Popconfirm, Tour, advanced entry, rich display, feedback, structural layout, desktop, dock, file-dialog, and window surfaces. Each scene now renders in both light and dark themes (`light-*`/`dark-*` atlas names), so the comparator checks 52 themed snapshots. The interaction-state and acceptance matrix scenes use a render-before-prepare hook to apply button hover/down, input focus/status, checkbox/radio states, slider focus, and menu/tabs/pagination/table/scrollbar hover states after layout has settled. Popover uses a test-only embedded `Qt::Widget` flag for its standalone surface, while Dropdown, Popconfirm, and Tour are captured from their real top-level tooltip/dialog windows by compositing root-owned visible top-level overlays into the atlas. Modal and Drawer scenes open after layout and wait for their animations before capture. The test still fails if a public widget header is neither covered by a scene nor explicitly deferred as a utility/controller. | Test-only code; runs under both Qt5 and Qt6. Qt6-only event constructors are guarded with `#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)`. |
| `tests/TestAntQtVersionMetricAudit.cpp` | Qt5/Qt6 metric and palette root-cause audit | The goal explicitly calls out `QStyle::pixelMetric()`, `QStyle::drawControl()` / `drawPrimitive()`, `QPalette`, font rendering, High DPI, and complex child-control geometry as sources of Qt5/Qt6 visual drift. Visual screenshots prove final output, but they do not expose which defaults differ. | Added a metric exporter/comparator covering Qt default style metrics, palette roles, font metrics, direct default draw fingerprints for `CE_PushButton`, `CE_CheckBox`, `PE_PanelLineEdit`, `CC_ComboBox`, `CE_TabBarTab`, `CE_Header`, and `CC_ScrollBar`, `QComboBox` / `QTabBar` / `QHeaderView` / `QScrollBar` default geometry, and adapted Ant widget size/font/palette/style geometry. The strict adapted-Ant sample includes Button, Input, Select, CheckBox, Progress, Menu, Pagination, Tag, Badge, Tabs, ScrollBar, List, Table, Tree, and ToolTip. Qt default rows are audit-only; Ant adapted rows are strict comparison rows with small tolerances. | Test-only code; runs under both Qt5 and Qt6. It uses `AntDesign::configureHighDpi()` before `QApplication` and `AntDesign::initialize(&app)` after app creation. |
| `tests/TestAntHighDpiScaling.cpp`, `tests/CMakeLists.txt` | High DPI regression coverage | Scale-factor behavior was previously validated only indirectly through manual example use and scattered DPR checks. | Added a dedicated test executable and three CTest entries for 1.0, 1.25, and 1.5 scale factors, plus two full visual-atlas smoke entries that run `TestAntQtVersionVisualParity` at 125% and 150% scaling. | Test-only code; runs under both Qt5 and Qt6. |
| `tests/TestAntNoStyleSheetUsage.cmake`, `tests/CMakeLists.txt` | No-QSS styling guard | The Qt5 adaptation rule forbids QSS/QStyleSheet. Manual `rg` checks can be skipped or become stale as files are added. | Added `TestAntNoStyleSheetUsage`, which scans `src`, `examples`, `tests`, and `resources` for `setStyleSheet()`, `QStyleSheet`, `styleSheet:` in UI files, and committed `.qss` files. | CMake-script test; independent of Qt version and runs in both Qt5 and Qt6 build trees. |
| `tests/CMakeLists.txt` | Test registry | New parity, High DPI, no-QSS, and real example traversal checks needed CTest entries. | Added `TestAntQtVersionVisualParity`, `TestAntQtVersionVisualParity_Scale_1_25`, `TestAntQtVersionVisualParity_Scale_1_5`, the three `TestAntHighDpiScaling_*` entries, `TestAntNoStyleSheetUsage`, and `TestAntExamplePageTraversal`. | CMake uses existing Qt-major abstraction. |
| `tests/TestAntDataDisplayB.cpp` | `AntTree` regression coverage, `AntCarousel` test fixtures | Existing cache test assumed `setTreeData()` could leave the flat cache hot. The safer behavior is to rebuild lazily after implicit detach risk has passed. The carousel test fixture also used `setStyleSheet()` for solid test slides, which weakened the no-QSS adaptation evidence even though it was test-only. | Updated expectations, added checked-state cache invalidation coverage, and replaced stylesheet-backed carousel slides with a small `paintEvent`-based `SolidColorTestWidget`. | Shared behavior, no version branch. The test fixture now follows the same QPainter-only styling rule as production widgets. |

## Adaptation Mechanism Summary

| Mechanism | Files / evidence | What it proves or changes |
| --- | --- | --- |
| Qt5 version-isolated startup attributes | `src/core/AntDesign.h`, `src/core/AntDesign.cpp`, `examples/main.cpp` | Qt5 gets logical High DPI scaling, High DPI pixmaps, and pass-through scale rounding before `QApplication`; Qt6 keeps its native startup behavior. |
| `QStyle` / `QProxyStyle` paint-path verification | `src/styles/AntInputNumberStyle.cpp`, `src/styles/AntTreeStyle.cpp`, `tests/TestAntQtVersionVisualParity.cpp`, `tests/TestAntQtVersionMetricAudit.cpp` | The adapted Ant style output is compared across Qt5/Qt6 for the success-standard controls and popup/window surfaces. InputNumber now owns the spin-box paint path end-to-end, and Tree icon drawing requests DPR-sized pixmaps inside the style paint path. No broad Qt5-only `QProxyStyle` metric override was needed because adapted Ant strict metric rows compare cleanly. |
| Widget `paintEvent` / QPainter corrections | `src/widgets/AntList.cpp`, `tests/TestAntDataDisplayB.cpp` | `AntListItem` custom image media is DPR-correct at fractional scaling. The carousel test fixture no longer uses QSS and instead paints with QPainter, matching the production styling rule. |
| Data / lifecycle correctness affecting rendering | `src/widgets/AntTree.cpp`, `tests/TestAntDataDisplayB.cpp` | Tree rendering no longer depends on cached pointers that can become stale after Qt implicit sharing detaches. This prevents Qt-version-sensitive paint instability. |
| Palette / token evidence | `tests/TestAntQtVersionMetricAudit.cpp`, `tests/TestAntQtVersionVisualParity.cpp` | The audit records default Qt palette drift as root-cause evidence, while strict adapted-Ant palette/style metrics stay within tolerance. No new production `QPalette` branch was required in this pass because rendered controls continue to resolve colors from `AntTheme` tokens. |
| Layout / geometry evidence | `examples/ExampleWindow.*`, `examples/main.cpp`, `tests/TestAntHighDpiScaling.cpp` | The real example shell exposes stable traversal hooks, every page is rendered in light/dark, and representative widgets plus the full atlas are checked at 100%, 125%, and 150% scaling. |
| No-QSS invariant | `tests/TestAntNoStyleSheetUsage.cmake`, `tests/CMakeLists.txt` | The repository is guarded against `setStyleSheet()`, `QStyleSheet`, UI `styleSheet:` properties, and committed `.qss` files across production sources, examples, tests, and resources. |

## Completion Audit Snapshot

| Audit item | Current proof |
| --- | --- |
| Qt5 target version | Qt5 evidence was generated with `actualQtVersion` `5.15.2` in the visual and metric comparison TSV files. |
| Qt6 baseline | Visual atlas and metric audit use the Qt6 build as the baseline; the latest local baseline was Qt `6.9.1`. |
| CTest registration | Both Qt6 and Qt5 build trees list `46` CTest entries, including the default visual parity test, two fractional-scale visual parity tests, the metric audit, three High DPI tests, no-QSS guard, and example traversal. |
| Combined adaptation gate | The latest combined adaptation-focused CTest run passed `9 / 9` entries in each build tree. |
| Qt6-only API isolation | Production startup changes guard Qt5-specific attributes with `QT_VERSION < QT_VERSION_CHECK(6, 0, 0)` and pass-through rounding with `QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)`. Test-only Qt6 event constructors are guarded in `TestAntQtVersionVisualParity`; existing unguarded `QEnterEvent` usage compiles under Qt5.15.2. |
| No stylesheet regression | `TestAntNoStyleSheetUsage` is part of the combined gate and passes in both build trees. |
| Binary artifact policy | `Testing/` contains generated PNG/TSV evidence and remains ignored; source, docs, and test code record how to regenerate it. |
| Remaining warning | Qt5 builds still emit MSVC STL4043 warnings from Qt 5.15.2 `QVector` internals. The warnings do not correspond to failed tests or visual comparison failures. |

## Verification Commands

```powershell
cmake --build build --target TestAntQtVersionVisualParity TestAntDataDisplayB --config Debug
ctest --test-dir build -C Debug -R "TestAntQtVersionVisualParity|TestAntQtVersionMetricAudit|TestAntDataDisplayB" --output-on-failure
cmake --build build --target TestAntDataEntryA TestAntQtExtensions --config Debug
ctest --test-dir build -C Debug -R "TestAntDataEntryA|TestAntQtExtensions" --output-on-failure
ctest --test-dir build -C Debug -R "TestAntInput" --output-on-failure
cmake --build build --target TestAntHighDpiScaling qt-ant-design-example --config Debug
ctest --test-dir build -C Debug -R "TestAntQtVersionVisualParity_Scale" --output-on-failure
ctest --test-dir build -C Debug -R "TestAntHighDpiScaling" --output-on-failure
ctest --test-dir build -C Debug -R "TestAntNoStyleSheetUsage" --output-on-failure
ctest --test-dir build -C Debug -R "TestAntExamplePageTraversal" --output-on-failure

cmake --build build-qt5-msvc --target TestAntQtVersionVisualParity TestAntDataDisplayB --config Debug
$env:PATH='C:\Qt\5.15.2\msvc2019_64\bin;' + $env:PATH
ctest --test-dir build-qt5-msvc -C Debug -R "TestAntQtVersionVisualParity|TestAntQtVersionMetricAudit|TestAntDataDisplayB" --output-on-failure
cmake --build build-qt5-msvc --target TestAntDataEntryA TestAntQtExtensions --config Debug
ctest --test-dir build-qt5-msvc -C Debug -R "TestAntDataEntryA|TestAntQtExtensions" --output-on-failure
ctest --test-dir build-qt5-msvc -C Debug -R "TestAntInput" --output-on-failure
cmake --build build-qt5-msvc --target TestAntHighDpiScaling qt-ant-design-example --config Debug
ctest --test-dir build-qt5-msvc -C Debug -R "TestAntQtVersionVisualParity_Scale" --output-on-failure
ctest --test-dir build-qt5-msvc -C Debug -R "TestAntHighDpiScaling" --output-on-failure
ctest --test-dir build-qt5-msvc -C Debug -R "TestAntNoStyleSheetUsage" --output-on-failure
ctest --test-dir build-qt5-msvc -C Debug -R "TestAntExamplePageTraversal" --output-on-failure
```

Atlas comparison:

```powershell
$env:PATH='C:\Qt\6.9.1\msvc2022_64\bin;' + $env:PATH
$env:ANT_QT_PARITY_EXPORT_DIR='D:\Project\GitProject\qt-ant-design\Testing\qt-version-visual-parity-popups\qt6'
build\tests\Debug\TestAntQtVersionVisualParity.exe

$env:PATH='C:\Qt\5.15.2\msvc2019_64\bin;' + $env:PATH
$env:ANT_QT_PARITY_EXPORT_DIR='D:\Project\GitProject\qt-ant-design\Testing\qt-version-visual-parity-popups\qt5'
$env:ANT_QT_PARITY_BASELINE_DIR='D:\Project\GitProject\qt-ant-design\Testing\qt-version-visual-parity-popups\qt6'
build-qt5-msvc\tests\Debug\TestAntQtVersionVisualParity.exe
```

Metric audit comparison:

```powershell
$env:PATH='C:\Qt\6.9.1\msvc2022_64\bin;' + $env:PATH
$env:ANT_QT_METRIC_EXPORT_DIR='D:\Project\GitProject\qt-ant-design\Testing\qt-version-metric-audit\qt6'
build\tests\Debug\TestAntQtVersionMetricAudit.exe

$env:PATH='C:\Qt\5.15.2\msvc2019_64\bin;' + $env:PATH
$env:ANT_QT_METRIC_EXPORT_DIR='D:\Project\GitProject\qt-ant-design\Testing\qt-version-metric-audit\qt5'
$env:ANT_QT_METRIC_BASELINE_DIR='D:\Project\GitProject\qt-ant-design\Testing\qt-version-metric-audit\qt6'
build-qt5-msvc\tests\Debug\TestAntQtVersionMetricAudit.exe
```

Real example full-page comparison:

```powershell
$base='D:\Project\GitProject\qt-ant-design\Testing\example-page-traversal'
$env:PATH='C:\Qt\6.9.1\msvc2022_64\bin;' + $env:PATH
Start-Process -FilePath 'D:\Project\GitProject\qt-ant-design\build\examples\Debug\qt-ant-design-example.exe' -ArgumentList @('--smoke-traverse-pages','--smoke-traverse-step-ms','1','--smoke-traverse-export-dir', (Join-Path $base 'qt6')) -Wait -WindowStyle Hidden

$env:PATH='C:\Qt\5.15.2\msvc2019_64\bin;' + $env:PATH
Start-Process -FilePath 'D:\Project\GitProject\qt-ant-design\build-qt5-msvc\examples\Debug\qt-ant-design-example.exe' -ArgumentList @('--smoke-traverse-pages','--smoke-traverse-step-ms','1','--smoke-traverse-export-dir', (Join-Path $base 'qt5'),'--smoke-traverse-baseline-dir', (Join-Path $base 'qt6')) -Wait -WindowStyle Hidden
```

Result: Qt6 targeted tests passed, Qt5 targeted tests passed, and the latest combined targeted CTest run passed `9 / 9` entries in each build tree for `TestAntQtVersionVisualParity`, `TestAntQtVersionVisualParity_Scale_1_25`, `TestAntQtVersionVisualParity_Scale_1_5`, `TestAntQtVersionMetricAudit`, the three `TestAntHighDpiScaling_*` entries, `TestAntNoStyleSheetUsage`, and `TestAntExamplePageTraversal`. The Qt5-vs-Qt6 themed atlas comparison passed for 52 snapshots, the full visual atlas smoke passed at `QT_SCALE_FACTOR=1.25` and `QT_SCALE_FACTOR=1.5` in both Qt6 and Qt5 build trees, Qt5-vs-Qt6 metric audit comparison passed for 216 rows, Qt5 example smoke launches passed at `QT_SCALE_FACTOR=1.25` and `QT_SCALE_FACTOR=1.5`, `TestAntExamplePageTraversal` passed on both Qt6 and Qt5 by traversing all 88 example pages in light and dark themes, and the full real-example Qt5-vs-Qt6 comparison passed for 352 exported frames. On `2026-06-02`, the focused `AntInputNumber` / `AntInputDialog` numeric-border regression was also verified by building and passing `TestAntDataEntryA` and `TestAntQtExtensions` in both Qt6 and Qt5 build trees, then launching the Qt5 example smoke. Qt5 builds still emit MSVC STL4043 warnings from Qt 5.15.2 `QVector` internals; these are warnings, not visual failures.

## Maintenance Notes

- Keep Qt5-only rendering fixes behind `#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)` when the root cause is a real Qt5/Qt6 API or style difference.
- Do not add QSS/QStyleSheet. Visual fixes should stay in `QStyle`/`QProxyStyle`, widget paint paths, palettes, metrics, and layout/size-hint code.
- Keep `TestAntNoStyleSheetUsage` broad enough to cover new source, example, test, and resource locations whenever project layout changes.
- When a fix is shared correctness, prefer a shared code path instead of a version branch, as with the `AntTree` cache invalidation fix above.
- Call `AntDesign::configureHighDpi()` before `QApplication` in consumer apps, especially on Qt5/Windows. Calling it after app creation is intentionally warned because Qt cannot retroactively switch the process into the same logical scaling mode.
- DPR-sensitive pixmap paths should render or cache at `logicalSize * devicePixelRatioF()` and set the pixmap DPR before drawing into logical widget coordinates.
- For future visual changes, export Qt6 atlas first, then run Qt5 with `ANT_QT_PARITY_BASELINE_DIR` to catch font, spacing, icon, border, and token-color drift before manual review.
