# Component Performance Optimization Progress

Updated: `2026-05-21`

This document starts the performance pass for all `84` public components. The first pass is intentionally conservative: define the optimization target for every component, measure before changing behavior, and keep the existing interaction, motion, shadow, theme, and layout experience intact.

## Goals

- Preserve current visual and interaction parity. Any user-visible compromise must be explicitly requested and approved.
- Preserve all visible animations and motion feedback. Performance work must not remove animations, disable transitions, or silently shorten motion timing to hide cost.
- Prefer local optimizations: smaller dirty regions, cached static paint assets, coalesced high-frequency updates, lazy popup setup, and avoiding unnecessary layout/style refreshes.
- Keep theme switching correct while reducing work when a component only needs repaint and not geometry recalculation.
- Keep popup and overlay windows responsive without reintroducing input-blocking transparent windows.
- Test only the changed control and its directly affected shared helper first; reserve full CTest sweeps for milestone validation.

## Progress Legend

| Status | Meaning |
| --- | --- |
| `Plan ready` | Static review complete and a per-component optimization path is defined. |
| `Baseline needed` | Add or run a component-level timing/paint/update measurement before code changes. |
| `In progress` | Implementation started but not fully verified. |
| `Optimized` | Code optimized and targeted tests/example build passed. |
| `Watching` | Recently optimized or high-risk; keep regression checks around it. |

Current summary: `84 / 84` components have an initial plan. `43` components are optimized from this pass, and `41` remain in planned or watching states.

Latest completed optimization:

| Date | Component | Change | Validation |
| --- | --- | --- | --- |
| `2026-05-21` | `AntAutoComplete` | Popup suggestion widgets are now reused across filtering and keyboard navigation instead of being destroyed and rebuilt for every input or highlight change. Filtering is cached by input, case-sensitivity, and suggestion-list revision, repeated popup geometry applications are skipped, and keyboard highlight movement updates only the old/new visible rows. | `cmake --build build --config Debug --target TestAntDataEntryA`, `TestAntDataEntryA.exe`, `cmake --build build --config Debug --target qt-ant-design-example` |
| `2026-05-21` | `AntTabs` | Tab rectangles, close/add geometry, text/icon rectangles, and card paths are now cached on the widget and reused by paint, hit testing, add-button placement, and indicator targeting. Hover, add-button hover, active tab changes, and indicator animation frames repaint only the affected tab/add/indicator regions, while theme/font/style/tab text changes invalidate the cache. | `cmake --build build --config Debug --target TestAntNavigation`, `TestAntNavigation.exe`, `cmake --build build --config Debug --target TestAntAdvancedInteractions`, `TestAntAdvancedInteractions.exe tabsKeyboardAddAndCloseFlow`, `cmake --build build --config Debug --target qt-ant-design-example` |
| `2026-05-21` | `AntSteps` | Step item/icon/text rectangles are now cached on the widget and shared by hit testing and style painting. Current, hover, and explicit status changes repaint only the affected step range/rows, theme/font/style changes invalidate the cache, and the style no longer keeps a duplicate step-layout builder. | `cmake --build build --config Debug --target TestAntNavigation`, `TestAntNavigation.exe`, `cmake --build build --config Debug --target qt-ant-design-example` |
| `2026-05-21` | `AntPagination` | Page-button rectangles and text metrics are now cached on the widget and shared by size hints, hit testing, quick jumper placement, and style painting. Hover enter/leave repaints only old/new button dirty regions, theme/font/style changes invalidate the cache, and the style no longer keeps a duplicate page-item builder. | `cmake --build build --config Debug --target TestAntNavigation`, `TestAntNavigation.exe`, `cmake --build build --config Debug --target qt-ant-design-example` |
| `2026-05-21` | `AntMenu` | Visible item rectangles are now cached and reused by paint, hit testing, keyboard navigation, size hints, and submenu popup placement. Hover, press, selection, popup-parent highlight, and non-geometry action state changes repaint only affected rows, while theme refresh is owned by the widget instead of an extra style-level refresh path. | `cmake --build build --config Debug --target TestAntNavigation`, `TestAntNavigation.exe`, `cmake --build build --config Debug --target TestAntInteractions`, `TestAntInteractions.exe`, `cmake --build build --config Debug --target qt-ant-design-example` |
| `2026-05-21` | `AntDropdown` | Popup content width is now cached by labels, font, and token metrics. Repeated target resize/move with unchanged placement skips popup `setGeometry()`/`update()`, popup margins and menu fixed width are applied only when changed, and item changes invalidate popup sizing before reuse. | `cmake --build build --config Debug --target TestAntNavigation`, `TestAntNavigation.exe`, `cmake --build build --config Debug --target qt-ant-design-example` |
| `2026-05-21` | `AntBreadcrumb` | Item widths, item rectangles, separator spacing, and total size are now cached by font, token metrics, separator, height, and item list. Paint and hit testing reuse the same cached geometry, separator changes invalidate the cache, and hover enter/leave repaint only the old/new item dirty regions. | `cmake --build build --config Debug --target TestAntNavigation`, `TestAntNavigation.exe`, `cmake --build build --config Debug --target qt-ant-design-example` |
| `2026-05-21` | `AntAnchor` | Link indicator rectangles are now cached and invalidated only on link layout, resize, or theme changes. Active label font/palette state is applied on state/theme changes instead of during every paint, indicator animation frames repaint only the old/new indicator union, and scroll value bursts coalesce into one active-section resolve. | `cmake --build build --config Debug --target TestAntNavigation`, `TestAntNavigation.exe`, `cmake --build build --config Debug --target qt-ant-design-example` |
| `2026-05-21` | `AntTypography` | Repeated text measurement now reuses a width/font/token-aware cache across `sizeHint()` and `heightForWidth()`, copy-button hit rectangles are cached for paint and mouse interaction, copy hover/press updates repaint only the icon region, and the style now uses the widget's shared font construction path. | `cmake --build build --config Debug --target TestAntTypography`, `TestAntTypography.exe`, `cmake --build build --config Debug --target qt-ant-design-example` |
| `2026-05-21` | `AntFloatButton` | Position updates now skip unchanged moves, open group children reuse cached geometry and visibility state instead of repeating `setGeometry()`/`show()`/`raise()`, repeated badge/content setters avoid redundant updates, and theme refresh is widget-owned instead of style-level global refresh. | `cmake --build build --config Debug --target TestAntFloatButton`, `TestAntFloatButton.exe`, `cmake --build build --config Debug --target qt-ant-design-example` |
| `2026-05-21` | `AntButton` | Loading spinner frames now repaint only the spinner indicator region, the spinner timer pauses while the button is hidden, and theme refresh remains widget-owned instead of also using a style-level global refresh path. | `cmake --build build --config Debug --target TestAntButton`, `TestAntButton.exe`, `cmake --build build --config Debug --target qt-ant-design-example` |
| `2026-05-21` | `AntToolButton` | Loading spinner frames now repaint only the spinner indicator region, dropdown arrow animation repaints only the arrow region, and the spinner timer pauses while the button is hidden. Theme refresh is owned by the widget instead of an additional style-level global refresh path. | `cmake --build build --config Debug --target TestAntQtExtensions`, `TestAntQtExtensions.exe toolButton`, `cmake --build build --config Debug --target qt-ant-design-example` |
| `2026-05-21` | `AntToolBar` | Action add/change now synchronizes only the affected toolbar button instead of scanning every direct button. Theme refresh remains toolbar-owned, button updates skip unchanged palette/style/geometry work, and toolbar button text metrics are cached across size and paint paths. | `cmake --build build --config Debug --target TestAntQtExtensions`, `TestAntQtExtensions.exe toolBar`, `cmake --build build --config Debug --target qt-ant-design-example` |
| `2026-05-21` | `AntStatusBar` | Status-bar item, permanent-item, divider, and message rectangles are now cached and shared by paint and hit testing. Hover and message changes repaint only the cached item/message regions instead of the full bar. | `cmake --build build --config Debug --target TestAntQtExtensions`, `TestAntQtExtensions.exe statusBar`, `cmake --build build --config Debug --target qt-ant-design-example` |
| `2026-05-21` | `AntScrollBar` | Hover, press, auto-hide, enabled-state, and theme refresh paths now repaint only the slider handle region instead of the full scrollbar. The style no longer registers its own global theme repolish path; the widget handles theme refresh with a scoped handle update. | `cmake --build build --config Debug --target TestAntQtExtensions`, `TestAntQtExtensions.exe scrollBar`, `cmake --build build --config Debug --target qt-ant-design-example` |
| `2026-05-21` | `AntScrollArea` | Theme application now caches surface colors and the current content widget. Replacing content only applies the content palette, while unchanged scroll-area and viewport palette work is skipped; real theme changes still update the scroll area, viewport, and content, then repaint the viewport once. | `cmake --build build --config Debug --target TestAntQtExtensions`, `TestAntQtExtensions.exe scrollArea`, `cmake --build build --config Debug --target qt-ant-design-example` |
| `2026-05-21` | `AntRibbon` | Tab layout rectangles are now cached by width, collapse-button visibility, font, and page titles, then reused across paint, hover hit testing, and indicator targeting. Hover and indicator animation paths repaint only the affected tab/button/indicator regions instead of scheduling full-ribbon updates. | `cmake --build build --config Debug --target TestAntQtExtensions`, `TestAntQtExtensions.exe ribbon`, `cmake --build build --config Debug --target qt-ant-design-example` |
| `2026-05-21` | `AntPlainTextEdit` | Visual font/palette application is now cached by theme tokens so content edits do not restyle the editor. Resize-grip hover now routes mouse events from the editor's internal children through one coordinate path, updates cursor state only on real hover changes, and repaints only the grip dirty region. | `cmake --build build --config Debug --target TestAntQtExtensions`, `TestAntQtExtensions.exe plainTextEdit`, `cmake --build build --config Debug --target qt-ant-design-example` |
| `2026-05-21` | `AntNavItem` | Label palette/font updates now run only when active or theme inputs change instead of during every paint. The self-painted background, active indicator colour, and indicator rectangle are cached by size, active, hover, and theme state, with enter/leave/resize/theme changes invalidating the cache. | `cmake --build build --config Debug --target TestAntQtExtensions`, `TestAntQtExtensions.exe navItem`, `cmake --build build --config Debug --target qt-ant-design-example` |
| `2026-05-21` | `AntMenuBar` | Menu-bar action geometry is now cached and invalidated only on action, resize, font, style, layout-direction, or theme changes, so hover movement can repaint the old/new action rectangles instead of scheduling an extra full-bar update. Menu item display text and text widths are cached by raw text and font for both sizing and paint paths. | `cmake --build build --config Debug --target TestAntQtExtensions`, `TestAntQtExtensions.exe menuBar`, `cmake --build build --config Debug --target qt-ant-design-example` |
| `2026-05-21` | `AntDockWidget` | Title-bar button icons now render into a DPR-aware pixmap cache keyed by icon, color, size, and DPR instead of rereading SVG resources on every button paint. Title-bar chrome/theme refreshes now skip redundant palette, font, margin, button-size, and geometry work when floating/closable/theme inputs are unchanged. | `cmake --build build --config Debug --target TestAntQtExtensions`, `TestAntQtExtensions.exe dockWidget`, `cmake --build build --config Debug --target qt-ant-design-example` |
| `2026-05-21` | `AntDockManager` | Drag hit testing now caches visible dock-area global hit zones by layout state and reuses the last same-position drop-target query when the guide state is unchanged. Move and release paths no longer rescan dock areas or recompute preview placement for the same cursor point. | `cmake --build build --config Debug --target TestAntQtExtensions`, `TestAntQtExtensions.exe dockManager`, `cmake --build build --config Debug --target qt-ant-design-example` |
| `2026-05-21` | `AntWindow` | Title-bar button rectangles are now cached by window width and visible-button mask, so paint, hover, mouse, and native hit-test paths reuse the same layout. Button press/hover/visibility changes now repaint only the affected title-bar button strip instead of the full title bar. | `cmake --build build --config Debug --target TestAntQtExtensions`, `TestAntQtExtensions.exe window`, `TestAntQtExtensions.exe windowTitleBarButtonsHandleChildDeliveredClicks windowTitleBarButtonsTriggerOnRelease windowTitleBarHoverStateClearsOnLeave`, `cmake --build build --config Debug --target qt-ant-design-example` |
| `2026-05-21` | `AntWidget` | Theme changes now use a lightweight surface update by default. The widget caches its style, palette key, size hint, and minimum size hint, then repolishes only when style/palette inputs changed and calls `updateGeometry()` only when cached hints changed. | `cmake --build build --config Debug --target TestAntQtExtensions`, `TestAntQtExtensions.exe widget`, `cmake --build build --config Debug --target qt-ant-design-example` |
| `2026-05-21` | `AntSplitter` | Splitter handle colors are now cached on the parent splitter with a theme revision. Handles no longer connect to theme changes individually; repeated paints reuse the cached color until hover state or the parent theme revision changes, and theme refreshes update only handle widgets. | `cmake --build build --config Debug --target TestAntQtExtensions`, `TestAntQtExtensions.exe splitter`, `cmake --build build --config Debug --target qt-ant-design-example` |
| `2026-05-21` | `AntSpace` | Size hints and minimum size hints are now cached until item/spacing/alignment or child layout events invalidate them. Appending items uses the existing box layout directly instead of rebuilding every child, while insert/remove/orientation/separator changes still rebuild the structure intentionally. Separator copies are tracked and cleaned during rebuilds. | `cmake --build build --config Debug --target TestAntLayout`, `TestAntLayout.exe space`, `cmake --build build --config Debug --target qt-ant-design-example` |
| `2026-05-21` | `AntMasonry` | Column heights, column width, and layout inputs are now cached after a full placement pass. Appending a widget reuses the current shortest-column state and lays out only the new item; same-width resize events are skipped, while width/columns/spacing changes still trigger a full measured pass. | `cmake --build build --config Debug --target TestAntLayout`, `TestAntLayout.exe masonry`, `cmake --build build --config Debug --target qt-ant-design-example` |
| `2026-05-21` | `AntLayout` | Header, sider, content, and footer geometry is now cached by layout size and dirty state, unchanged regions skip `setGeometry()`, and same-size resize events avoid redundant relayout work. Sider width/collapse changes now mark the parent layout dirty so content moves immediately while repeated layout requests stay cheap. | `cmake --build build --config Debug --target TestAntLayout`, `TestAntLayout.exe layout`, `cmake --build build --config Debug --target qt-ant-design-example` |
| `2026-05-21` | `AntGrid` | Row placement is now calculated incrementally as columns are added, column stretch setup runs once, and gutter changes update spacing without tearing down/re-adding existing grid items. The layout keeps counters for targeted tests so future changes can detect accidental full relayouts on append/gutter paths. | `cmake --build build --config Debug --target TestAntLayout`, `TestAntLayout.exe grid`, `cmake --build build --config Debug --target qt-ant-design-example` |
| `2026-05-20` | `AntFlex` | The horizontal wrap layout now caches geometry passes by rect, spacing, margins, wrap state, and item count, reusing computed child rects for repeated `setGeometry()` and `heightForWidth()` calls until the layout is invalidated. Size hints are also cached for unchanged item counts and spacing. | `cmake --build build --config Debug --target TestAntLayout`, `TestAntLayout.exe flex`, `cmake --build build --config Debug --target qt-ant-design-example` |
| `2026-05-20` | `AntDivider` | Divider title metrics, title font, line pen, text rect, and line segment geometry are now cached by rect, text, font, placement, variant, orientation, and theme tokens. Repeated paints reuse the cached layout until a real visual/layout input changes. | `cmake --build build --config Debug --target TestAntLayout`, `TestAntLayout.exe divider`, `cmake --build build --config Debug --target qt-ant-design-example` |
| `2026-05-20` | `AntConfigProvider` | Property setters now schedule one queued `configChanged` refresh per event turn, exposing a revision counter for consumers/tests while keeping individual property signals intact. `apply()` skips the global theme path when the requested mode is already active. | `cmake --build build --config Debug --target TestAntQtExtensions`, `TestAntQtExtensions.exe configProvider`, `cmake --build build --config Debug --target qt-ant-design-example`, `cmake --build build --config Debug --target TestAntMetaProperties`, `TestAntMetaProperties.exe` |
| `2026-05-20` | `AntApp` | Feedback host lookup is now cached through a `QPointer`-backed path, so message/modal/notification entry calls reuse the resolved root context instead of repeating host resolution. The singleton instance stack now restores nested wrappers on destruction and clears stale instances. | `cmake --build build --config Debug --target TestAntQtExtensions`, `TestAntQtExtensions.exe app`, `cmake --build build --config Debug --target qt-ant-design-example` |
| `2026-05-20` | `AntAffix` | Scroll, wheel, move, resize, and scrollbar value changes now schedule one coalesced affix check for the next event turn. Repeated unchanged geometry checks are skipped, affixed state uses the placeholder as the reference anchor, and affixed geometry updates on viewport resize without forcing duplicate state transitions. | `cmake --build build --config Debug --target TestAntLayout`, `TestAntLayout.exe affix`, `cmake --build build --config Debug --target qt-ant-design-example` |
| `2026-05-20` | `AntTree` | Visible-node flattening is now cached and reused across paint, hit testing, size hints, and scroll clamping until tree data or expansion state changes. Hover transitions repaint only the previous/current row instead of invalidating the whole tree. | `cmake --build build --config Debug --target TestAntDataDisplayB TestAntAdvancedInteractions`, `TestAntDataDisplayB.exe treeCachesFlattenedVisibleNodes`, `TestAntAdvancedInteractions.exe treeExpandCheckAndSelectFlow`, `cmake --build build --config Debug --target qt-ant-design-example` |
| `2026-05-20` | `AntTimeline` | Vertical paint now reuses cached item heights keyed by width, reverse order, token font sizes, spacing, and dot size. Dot colors are parsed once per item data/theme refresh, and title/content fonts and metrics are built once per paint instead of once per item. | `cmake --build build --config Debug --target TestAntDataDisplayB`, `TestAntDataDisplayB.exe timelineCachesPaintLayoutAndColors`, `cmake --build build --config Debug --target qt-ant-design-example` |
| `2026-05-20` | `AntTag` | Tag size hints now cache text/icon/close width calculations until layout-affecting inputs, font, style, or theme changes. Custom and semantic color parsing is cached per color/theme refresh, and mouse-move repaint is skipped unless hover or close-hover state actually changes. | `cmake --build build --config Debug --target TestAntTag`, `TestAntTag.exe`, `cmake --build build --config Debug --target qt-ant-design-example` |
| `2026-05-20` | `AntStatistic` | Formatted display text is now owned by the widget and refreshed only when value, precision, separator, countdown mode, or countdown format changes. The style reads the cached text instead of recomputing number/countdown formatting on every paint. | `cmake --build build --config Debug --target TestAntDataDisplayA`, `TestAntDataDisplayA.exe statisticFormattedValueCache`, `TestAntDataDisplayA.exe propertiesAndSignals` |
| `2026-05-20` | `AntQRCode` | QR modules now render into a DPR-aware transparent pixmap cache keyed by matrix revision, logical size, DPR, and foreground color. Status overlays, borders, and center icons remain separate so visual states still repaint normally without rebuilding the module layer. | `cmake --build build --config Debug --target TestAntDataDisplayB`, `TestAntDataDisplayB.exe qrCodeReusesRenderedModuleCache`, `TestAntDataDisplayB.exe propertiesAndSignals` |
| `2026-05-20` | `AntList` | Bulk `addItems()` and `insertItems()` now adopt all new rows first and coalesce layout, geometry, and repaint work into one pass instead of repeating the full list update for every item. | `cmake --build build --config Debug --target TestAntDataDisplayB`, `TestAntDataDisplayB.exe listBulkInsertionCoalescesLayout`, `TestAntDataDisplayB.exe listInternalScrolling` |
| `2026-05-20` | `AntTable` | Hover and leave transitions now compute row dirty rectangles and repaint only the previous/current row instead of scheduling a full table update. A targeted test asserts the row-scoped update path directly. | `cmake --build build --config Debug --target TestAntDataDisplayB`, `TestAntDataDisplayB.exe tableHoverUsesRowScopedUpdates`, `TestAntAdvancedInteractions.exe tableSortSelectionAndPaginationFlow` |
| `2026-05-20` | `AntLog` | Appends now use a document cursor instead of moving the visible editor cursor, level text formats are cached per theme, undo/redo storage is disabled for log output, and entry trimming removes overflow in one batch. | `cmake --build build --config Debug --target TestAntQtExtensions`, `TestAntQtExtensions.exe log` |
| `2026-05-20` | `AntIcon` | Cached enum path generation and resource SVG rendered pixmaps by icon name, colors, size, and DPR. Rotation and spin still use painter transforms, so visible motion is preserved while repeated paints avoid qrc reads and SVG renderer setup. | `cmake --build build --config Debug --target TestAntIcon`, `TestAntIcon.exe`, `cmake --build build --config Debug --target TestAntButton`, `TestAntButton.exe` |

## Shared Optimization Rules

- Paint path: cache expensive `QPixmap`, `QPainterPath`, gradients, text layout, and icon render output by theme mode, DPR, size, state, and input data.
- Update path: replace full-widget `update()` calls with region updates where hover, cursor, handle, row, tab, or item geometry is known.
- Animation path: keep user-visible motion. Optimize by caching animation geometry, reducing dirty regions, coalescing frame work, and pausing timers only when the animation is not visible or the animated state is inactive.
- Popup path: create heavy popup content lazily, reuse popup widgets where lifecycle is stable, and close through one code path so enter/leave motion stays cheap and predictable.
- Theme path: use `updateGeometry()` only when theme metrics affect size; otherwise prefer `update()` and palette/token refresh.
- Data path: separate model/data recalculation from paint. Cache visible row/item geometry and invalidate only on data, size, font, or theme changes.
- Windows path: avoid creating native child windows or transparent top-level helpers unless a component really needs platform behavior.

## Baseline Checklist

Use the existing `build` directory. Do not create temporary build directories.

| Check | Purpose |
| --- | --- |
| Targeted QTest case | Correctness guard for the component being changed. |
| Example Debug build | Confirms the public example still links and launches. |
| Smoke launch | Confirms app startup/shutdown behavior after UI changes. |
| Manual interaction timing | Drag, hover, popup open, animation, and theme switch perception. |
| Optional Release timing | Use when Debug timings are too noisy for paint/update cost. |

## Priority Order

| Priority | Focus |
| --- | --- |
| P0 | Dragging, scrolling, popup open/close, theme switching, large-data widgets, persistent animations. |
| P1 | Hover/active repaint churn, icon/text/path caching, repeated layout invalidation. |
| P2 | Static simple controls where savings are mostly cleanup or shared helper reuse. |

## Component Progress

### General

| Component | Priority | Optimization plan | Progress |
| --- | --- | --- | --- |
| `AntButton` | P1 | Run the loading timer only while visible and loading, repaint spinner frames through the spinner indicator region, and keep theme refresh widget-owned instead of duplicating the style global refresh path. | Optimized |
| `AntFloatButton` | P1 | Skip unchanged fixed-size/content updates, reuse button/group geometry and visibility during open layout, skip unchanged positioning, and keep theme refresh widget-owned instead of duplicating a style-level global refresh path. | Optimized |
| `AntIcon` | P0 | Cache enum paths and rendered resource pixmaps keyed by icon name, colors, size, and DPR. Keep rotation/spin outside the cache through painter transforms so motion remains unchanged. | Optimized |
| `AntTypography` | P1 | Cache repeated text measurements by width, font, token metrics, and mode; cache copy hit rectangles for paint and mouse interaction; repaint only copy-icon dirty regions for copy hover/press state. | Optimized |

### Navigation

| Component | Priority | Optimization plan | Progress |
| --- | --- | --- | --- |
| `AntAnchor` | P1 | Cache link indicator rectangles, apply label visual state only on active/theme changes, repaint only indicator dirty regions during animation, and coalesce scroll bursts into one active-section resolve. | Optimized |
| `AntBreadcrumb` | P2 | Cache item widths, item rectangles, separator spacing, and total size for size, paint, and hit-test paths; repaint only old/new hover item dirty regions. | Optimized |
| `AntDropdown` | P0 | Cache popup content width, apply popup margins/menu width only when changed, skip repeated popup geometry applications, and keep outside-click closing on one lightweight path. | Optimized |
| `AntMenu` | P0 | Cache visible item layout for paint, hit testing, size hints, keyboard navigation, and submenu placement; repaint only affected rows during hover/press/selection/action state changes; keep theme refresh widget-owned. | Optimized |
| `AntPagination` | P1 | Cache page-button rectangles and text metrics across size hints, hit testing, quick jumper placement, and style painting; recalculate only when pagination state or visual metrics change, and repaint only old/new hover buttons. | Optimized |
| `AntSteps` | P1 | Cache step item/icon/text rectangles across hit testing and style painting; dirty-update current/previous/current-hover/status steps; invalidate on direction/theme/font/style changes. | Optimized |
| `AntTabs` | P0 | Cache tab rectangles, close/add geometry, text/icon rectangles, and card paths across paint, hit testing, add-button placement, and indicator targeting; repaint only hover/add/active/indicator dirty regions and invalidate on theme/font/style/tab text changes. | Optimized |

### Data Entry

| Component | Priority | Optimization plan | Progress |
| --- | --- | --- | --- |
| `AntAutoComplete` | P0 | Cache filtered suggestions by input/case/list revision, reuse visible popup suggestion widgets, skip unchanged popup geometry, and repaint only old/new highlighted rows during keyboard navigation. | Optimized |
| `AntCascader` | P0 | Lazy-create child columns, cache option row metrics per level, update only changed column on hover/selection, and reuse popup shell. | Plan ready |
| `AntCheckBox` | P1 | Cache indicator geometry and icon path; limit hover/press repaint to indicator/text bounds; skip redundant update when checked state is unchanged. | Plan ready |
| `AntColorPicker` | P0 | Keep HS field background cache and dirty cursor repaint; add baseline for popup open/close animation and drag latency; ensure live-refresh coalescing remains under one frame. | Watching |
| `AntDatePicker` | P0 | Cache calendar cell rectangles/text, update only changed date/range cells, lazily build popup pages, and avoid full popup repaint on hover. | Plan ready |
| `AntDescriptions` | P1 | Cache table cell layout and label/value metrics; invalidate only when items, column count, font, or width changes. | Plan ready |
| `AntForm` | P1 | Batch form-item validation/layout updates, cache label widths, and update only affected item help/status area. | Plan ready |
| `AntInput` | P1 | Cache prefix/suffix/clear/password/search button geometry; reduce hover/focus updates to frame/action regions; avoid full repaint for cursor-independent state changes. | Plan ready |
| `AntInputNumber` | P0 | Keep handler reveal animation but update only control strip/frame; cache prefix/suffix text and handler hit rectangles; pause timers only when hidden or the reveal state is inactive. | Plan ready |
| `AntMentions` | P0 | Debounce suggestion filtering, cache mention popup rows, and update only highlighted suggestion rows during keyboard navigation. | Plan ready |
| `AntRadio` | P1 | Cache indicator path/text metrics; dirty-update old/new checked or hover radio bounds; reuse wave overlay path where possible. | Plan ready |
| `AntRate` | P1 | Cache star paths per size/DPR, repaint only old/new hover or selected star range, and keep pulse animation scoped to one star. | Plan ready |
| `AntSegmented` | P0 | Cache segment rects and text/icon metrics; animate indicator dirty union only; avoid full-track repaint except on resize or option changes. | Plan ready |
| `AntSelect` | P0 | Virtualize or window popup rows for large lists, cache option metrics, coalesce editable filtering, and repaint only changed tag/row/active regions. | Plan ready |
| `AntSlider` | P0 | Cache track/mark geometry, update old/new handle and tooltip bounds during drag, and keep passive value bubble mouse-transparent without forcing parent repaint. | Plan ready |
| `AntSwitch` | P0 | Scope handle/progress/loading spinner repaints, stop loading timer when hidden, and reuse wave overlay only for click feedback. | Plan ready |
| `AntTimePicker` | P0 | Cache time-column item geometry, repaint only old/new highlighted rows, reuse popup shell, and avoid rebuilding columns on simple value changes. | Plan ready |
| `AntTransfer` | P0 | Cache visible row geometry and checkbox rects, update only rows affected by scroll/selection/move, and avoid rebuilding both panes on one-side changes. | Plan ready |
| `AntTreeSelect` | P0 | Reuse tree popup, cache visible node layout, lazy-expand children, and repaint only changed nodes/tags. | Plan ready |
| `AntUpload` | P1 | Cache file item layout and thumbnail/icon pixmaps; update only changed file row/card and progress region. | Plan ready |

### Feedback

| Component | Priority | Optimization plan | Progress |
| --- | --- | --- | --- |
| `AntAlert` | P2 | Cache icon pixmap/path and close rect; repaint only close button on hover and full alert only on content/status changes. | Plan ready |
| `AntDrawer` | P0 | Keep slide animation but render only drawer/mask dirty regions; lazy-build body content and avoid repeated mask geometry recalculation. | Plan ready |
| `AntMessage` | P0 | Coalesce stack relayout during burst messages, reuse shadow metrics, and keep click-through overlay hit testing while reducing top-level repaint. | Plan ready |
| `AntModal` | P0 | Cache dialog shadow/mask geometry, update only button hover/press regions, and avoid remeasuring content unless text/custom body changes. | Plan ready |
| `AntNotification` | P0 | Coalesce stack position updates, repaint only progress/loading region per tick, and stop timers only after a notice has finished closing or is hidden. | Plan ready |
| `AntPopconfirm` | P0 | Reuse popup shell, cache arrow/body geometry, and repaint only footer buttons on hover/press. | Plan ready |
| `AntPopover` | P0 | Reuse popup shell, cache placement/arrow/content layout, and avoid hover show/hide churn with stable delayed state. | Plan ready |
| `AntProgress` | P1 | Cache static track path; repaint only changed progress arc/bar and percent text region. | Plan ready |
| `AntResult` | P2 | Cache status icon pixmap/path and text layout; repaint only when status/content/theme changes. | Plan ready |
| `AntSkeleton` | P0 | Keep shimmer timer only while visible/loading; cache placeholder paths and repaint shimmer band dirty region where possible. | Plan ready |
| `AntSpin` | P0 | Keep precise animation timer only when visible/spinning; update spinner bounds only; share cached dot/arc geometry by size. | Plan ready |
| `AntToolTip` | P0 | Reuse tooltip popup, cache content/arrow geometry, stop delayed-open timer on target hide/destroy, and keep passive hit-test cheap. | Plan ready |
| `AntTour` | P0 | Cache mask target geometry, update only previous/new highlight areas during step change, and avoid rebuilding bubble controls on unchanged content. | Plan ready |
| `AntWatermark` | P1 | Cache tiled watermark pixmap by text/font/color/angle/gap/DPR and blit instead of redrawing every tile. | Plan ready |

### Data Display

| Component | Priority | Optimization plan | Progress |
| --- | --- | --- | --- |
| `AntAvatar` | P1 | Cache scaled/clipped avatar pixmap per size/DPR/source; reuse default icon paths; invalidate only on source or size changes. | Plan ready |
| `AntBadge` | P0 | Scope processing animation repaint to badge overlay, cache ribbon/count text geometry, and stop timers when hidden or not processing. | Plan ready |
| `AntCalendar` | P0 | Cache month/year/day cell geometry and text; update only changed cells on hover/current/selected date changes. | Plan ready |
| `AntCard` | P1 | Cache title/body/action layout and loading skeleton geometry; stop spinner timer unless loading and visible. | Plan ready |
| `AntCarousel` | P0 | Keep slide animation but cache page pixmaps for static pages where safe, pause autoplay when hidden, and repaint only viewport. | Plan ready |
| `AntCollapse` | P0 | Animate height with coalesced layout invalidation, cache header/body geometry, and repaint only animating panel. | Plan ready |
| `AntEmpty` | P2 | Cache illustration pixmap/path and description layout; repaint only on theme/content changes. | Plan ready |
| `AntImage` | P1 | Cache scaled image pixmap per target size/DPR and reuse preview overlay; avoid rescaling during unchanged paints. | Plan ready |
| `AntList` | P0 | Bulk add/insert coalesces row adoption into one layout, geometry, and repaint pass. Continue with cached item heights/visible range and row-scoped selection updates in a later pass. | Optimized |
| `AntQRCode` | P1 | Cache generated QR module pixmaps by matrix revision, logical size, DPR, and foreground color. Keep status overlays, borders, and center icons separate so state-only repaint does not rebuild the module layer. | Optimized |
| `AntStatistic` | P2 | Cache formatted value/countdown text in the widget and refresh it only when value, precision, separator, countdown mode, or countdown format changes; keep text layout caching as a later pass. | Optimized |
| `AntTable` | P0 | Hover and leave now repaint only the previous/current row dirty rectangles. Continue with cached visible geometry, sorter/selection rects, and header-cell dirty updates in a later pass. | Optimized |
| `AntTag` | P2 | Cache size-hint text/icon/close metrics and parsed tag colors; repaint mouse hover only when hover or close-hover state changes. Continue with text-layout rect caching if future profiling shows paint cost in dense tag lists. | Optimized |
| `AntTimeline` | P1 | Cache vertical item height layout and parsed dot colors; reuse title/content fonts and metrics inside paint. Continue with item/connector dirty-region repaint if future interaction states are added. | Optimized |
| `AntTree` | P0 | Cache flattened visible nodes for paint, hit testing, size hints, and scroll bounds; repaint only old/new hover rows. Continue with connector/checkbox rect caches for the next dense-tree pass. | Optimized |

### Layout And Misc

| Component | Priority | Optimization plan | Progress |
| --- | --- | --- | --- |
| `AntAffix` | P1 | Coalesce scroll/resize/move/scrollbar events into one queued affix check, skip unchanged geometry, and update affixed geometry only when the viewport changes. | Optimized |
| `AntApp` | P2 | Cache feedback host resolution through a guarded root/parent/active-window path and restore nested app instances on destruction. Continue with real message/modal/notification plumbing when those entrypoints are expanded. | Optimized |
| `AntConfigProvider` | P1 | Property setters coalesce into one queued `configChanged` signal and revision increment per event turn; `apply()` avoids redundant global theme refresh when the requested mode is already active. | Optimized |
| `AntDivider` | P2 | Cache title font/text metrics, line pen, text rect, and horizontal/vertical line geometry; invalidate only when text, font, placement, variant, orientation, rect, or theme tokens change. | Optimized |
| `AntFlex` | P1 | Cache wrap-layout geometry and size hints across repeated layout queries; invalidate on item add/remove, spacing, wrap, orientation, or parent geometry changes. Continue with child size-hint event filtering if dense dynamic forms need a deeper pass. | Optimized |
| `AntGrid` | P1 | Incrementally cache row/column placement on append, initialize the 24-column stretch model once, and keep gutter updates to spacing-only invalidation. Continue with responsive breakpoint APIs if future grid variants add width-dependent span rules. | Optimized |
| `AntLayout` | P1 | Cache region geometry for header/sider/content/footer, skip unchanged `setGeometry()` calls, ignore same-size resize relayouts, and dirty only the parent layout when sider width/collapse changes require a new content region. | Optimized |
| `AntMasonry` | P0 | Cache column width and shortest-column heights after full placement, append new widgets incrementally into the cached shortest column, and skip same-width resize events; width/columns/spacing changes still perform a full measured pass. | Optimized |
| `AntSpace` | P2 | Cache `sizeHint()` and `minimumSizeHint()` until spacing/items/alignment or child layout events change, append items incrementally into the current box layout, and reserve full rebuilds for structural changes such as insert/remove/orientation/separator. | Optimized |
| `AntSplitter` | P0 | Cache handle normal/hover colors at the parent splitter level, reuse resolved handle paint color until hover/theme revision changes, and scope theme refreshes to handle widgets instead of repainting the whole splitter. Continue watching drag-heavy layouts for child-resize throttling needs. | Optimized |
| `AntWidget` | P1 | Cache style, palette key, size hint, and minimum size hint across theme changes; default to a lightweight surface repaint, repolish only when style/palette inputs changed, and call `updateGeometry()` only when cached hints changed. | Optimized |
| `AntWindow` | P0 | Cache title-bar button rectangles by width and visible-button mask, reuse them in paint/hover/mouse/native hit-test paths, and scope hover/press/visibility repaint to affected title-bar button regions. Continue watching Win10 shadow and theme-transition capture cost. | Optimized |

### Qt And Desktop Extensions

| Component | Priority | Optimization plan | Progress |
| --- | --- | --- | --- |
| `AntDockManager` | P0 | Cache dock-area hit zones and same-position drop-target queries during drag, while keeping embedded layout changes synchronous. Continue watching full dock-tree rebuilds and floating-window churn under complex workspace changes. | Optimized |
| `AntDockWidget` | P0 | Cache title-bar button icon pixmaps and skip redundant title-bar chrome/theme refreshes when inputs are unchanged. Continue watching floating-window native churn and drag/embedded repaint scope under Windows resize paths. | Optimized |
| `AntLog` | P0 | Use document-cursor appends, cached level formats, disabled undo history, and batched trim operations so large append bursts avoid moving the visible cursor or repeatedly shifting entries one by one. | Optimized |
| `AntMenuBar` | P1 | Cache action rectangles and text metrics; repaint only old/new hover action during pointer movement and invalidate geometry on action/layout/theme inputs. | Optimized |
| `AntNavItem` | P1 | Move label palette/font updates out of paint, cache hover/active background and indicator geometry, and invalidate on hover, active, resize, and theme inputs. | Optimized |
| `AntPlainTextEdit` | P1 | Cache visual font/palette application by theme tokens, route resize-grip mouse events from internal children through one coordinate path, and dirty-update the grip region only when hover state changes. | Optimized |
| `AntRibbon` | P0 | Cache tab layout rectangles by width, collapse-button visibility, font, and page titles; reuse the cache across paint/hit-test/indicator paths and repaint only affected tab, collapse button, and indicator regions during hover and indicator animation. | Optimized |
| `AntScrollArea` | P1 | Cache theme surface colors and the current content widget so content replacement applies only content palette work, while real theme changes update scroll area, viewport, and content once. Gesture setup remains independent from repaint work. | Optimized |
| `AntScrollBar` | P0 | Dirty-update only the slider handle region for hover, press, auto-hide, enabled-state, and theme refresh paths; theme changes are handled by the widget without style repolish. | Optimized |
| `AntStatusBar` | P1 | Cache item, permanent-item, divider, and message rectangles for both paint and hit testing; repaint only changed item/message regions. | Optimized |
| `AntToolBar` | P1 | Synchronize action-created buttons incrementally, skip unchanged palette/style/geometry work, keep theme refresh toolbar-owned, and cache button text metrics for size and paint paths. | Optimized |
| `AntToolButton` | P1 | Pause the spinner timer while hidden, repaint only spinner and arrow indicator regions during animation, and keep theme refresh widget-owned instead of duplicating the style global refresh path. | Optimized |

## Per-Component Test Plan

For every optimization, run the listed QTest target(s), build `qt-ant-design-example`, and perform the listed interaction/performance validation. If the optimization touches shared code, also run one representative target from each affected category.

### General Tests

| Component | Automated target(s) | Interaction and performance validation |
| --- | --- | --- |
| `AntButton` | `TestAntButton`, `TestAntFeedback` when wave/loading is touched | Verify hover/press/focus/disabled, icon buttons, visible loading spinner motion, hidden-state timer pause, and spinner-region repaint. |
| `AntFloatButton` | `TestAntFloatButton` | Verify group expand/collapse, BackTop, badge, hover motion, unchanged-position skips, child geometry cache reuse, and no extra repaint outside the floating group bounds. |
| `AntIcon` | `TestAntIcon`, plus one representative consumer such as `TestAntButton` | Verify all icon modes still render, theme color changes invalidate cache, DPR/size changes are crisp, and repeated icon paint avoids reparsing. |
| `AntTypography` | `TestAntTypography` | Verify title/text/paragraph/link, wrapping, ellipsis, copy feedback, theme switching, cached repeated measurements, cached copy hit rectangles, and layout metrics after cached text layout changes. |

### Navigation Tests

| Component | Automated target(s) | Interaction and performance validation |
| --- | --- | --- |
| `AntAnchor` | `TestAntNavigation` | Verify scroll tracking, active link changes, cached link indicator geometry, label visual state reuse after repaint, coalesced scroll bursts, indicator animation, and no missed active updates after throttling. |
| `AntBreadcrumb` | `TestAntNavigation` | Verify item hover/click, separators, disabled items, cached layout reuse, separator-driven invalidation, theme switching, and dirty repaint only for changed breadcrumb items. |
| `AntDropdown` | `TestAntInteractions`, `TestAntNavigation`, `TestAntPopupLifecycle` | Verify click/hover/context triggers, placement/arrow, outside close, popup motion, popup content-width cache reuse, geometry skip behavior, and repeated open/close latency. |
| `AntMenu` | `TestAntInteractions`, `TestAntNavigation`, `TestAntVisualRegression` | Verify action sync, hover/selection, submenu behavior, shortcut text, visible-layout cache reuse, action state changes without layout rebuild, large menu movement, and row-scoped repaint. |
| `AntPagination` | `TestAntNavigation`, `TestAntVisualRegression` | Verify page changes, quick jumper, page size changes, disabled states, page-item cache reuse, hover dirty-region updates, and button-geometry cache invalidation. |
| `AntSteps` | `TestAntNavigation`, `TestAntVisualRegression` | Verify current/error/clickable steps, horizontal/vertical modes, connector drawing, layout cache reuse, status/current scoped repaint, and old/new step dirty updates. |
| `AntTabs` | `TestAntAdvancedInteractions`, `TestAntNavigation`, `TestAntStressLifecycle`, `TestAntVisualRegression` | Verify active indicator animation, card/line/editable modes, tab close/disable fallback, cached tab/add/close layout reuse, scoped hover/add/active/indicator repaint, and content layout stability. |

### Data Entry Tests

| Component | Automated target(s) | Interaction and performance validation |
| --- | --- | --- |
| `AntAutoComplete` | `TestAntDataEntryA` | Verify filtering, keyboard navigation, popup item reuse, filtered-result cache hits, highlighted-row repaint, geometry skip behavior, and large suggestion lists. |
| `AntCascader` | `TestAntDataEntryB`, `TestAntInteractions`, `TestAntPopupLifecycle`, `TestAntStressLifecycle` | Verify hover/click expansion, multi-column popup, outside close, lazy column creation, and large option trees. |
| `AntCheckBox` | `TestAntCheckBox`, `TestAntVisualRegression` | Verify checked/unchecked/indeterminate/disabled, hover/press wave if applicable, and indicator/text dirty repaint. |
| `AntColorPicker` | `TestAntAdvancedInteractions`, `TestAntQtExtensions`, `TestAntPopupLifecycle`, `TestAntVisualRegression` | Verify popup enter/leave animation, HS drag smoothness, cursor movement, live color coalescing, shadow edges, and no white drag artifacts. |
| `AntDatePicker` | `TestAntDataEntryB`, `TestAntInteractions`, `TestAntPopupLifecycle`, `TestAntStressLifecycle` | Verify open/close motion, date/range/month/year selection, keyboard/mouse hover, and cell-scoped repaint. |
| `AntDescriptions` | `TestAntDataDisplayB`, `TestAntVisualRegression` | Verify bordered/vertical layouts, label/value widgets, column changes, and cached cell layout invalidation. |
| `AntForm` | `TestAntQtExtensions`, `TestAntChildOwnership` | Verify validation state changes, help text, label alignment, dynamic form rows, and batched item layout refresh. |
| `AntInput` | `TestAntInput`, `TestAntStressLifecycle` | Verify typing, selection, prefix/suffix/addons, password/search/clear, focus/hover, and action-region repaint. |
| `AntInputNumber` | `TestAntDataEntryA`, `TestAntInteractions`, `TestAntVisualRegression` | Verify handler reveal animation, stepping, precision/decimals, prefix/suffix, keyboard input, and control-strip dirty updates. |
| `AntMentions` | `TestAntDataEntryB` | Verify `@` trigger, suggestion filtering, keyboard navigation, popup row repaint, and large mention list latency. |
| `AntRadio` | `TestAntDataEntryA`, `TestAntMotion`, `TestAntVisualRegression` | Verify radio/group/button modes, keyboard selection, wave feedback, and old/new checked region updates. |
| `AntRate` | `TestAntDataEntryA` | Verify hover scaling, selected-star pulse, half selection, keyboard changes, and star path caching. |
| `AntSegmented` | `TestAntDataEntryA`, `TestAntAdvancedInteractions` | Verify full-track hit testing, indicator animation, disabled options, icons/tooltips, and indicator dirty-union repaint. |
| `AntSelect` | `TestAntSelect`, `TestAntInteractions`, `TestAntPopupLifecycle`, `TestAntStressLifecycle` | Verify single/multiple/tags/editable modes, filtering, keyboard navigation, large popup lists, tag updates, and popup row repaint. |
| `AntSlider` | `TestAntDataEntryA` | Verify range/single drag, marks, value bubble motion, vertical/reverse modes, and handle/tooltip dirty updates. |
| `AntSwitch` | `TestAntSwitch`, `TestAntStressLifecycle`, `TestAntVisualRegression` | Verify checked/loading/small/text states, click wave, handle animation, hidden-state timer behavior, and fast repeated toggles. |
| `AntTimePicker` | `TestAntDataEntryB`, `TestAntInteractions`, `TestAntPopupLifecycle`, `TestAntStressLifecycle` | Verify popup motion, hour/min/sec scrolling, range mode, hover/selection row repaint, and value-change invalidation. |
| `AntTransfer` | `TestAntDataEntryB` | Verify scroll, visible-row selection, header select-all, move operations, and pane-local update after data movement. |
| `AntTreeSelect` | `TestAntDataEntryB`, `TestAntPopupLifecycle`, `TestAntStressLifecycle` | Verify popup tree expansion, selection, tag rendering if used, outside close, and visible-node cache invalidation. |
| `AntUpload` | `TestAntDataEntryB`, `TestAntInteractions` | Verify dragger/list/picture-card modes, progress/status changes, thumbnail/icon cache, and row/card dirty updates. |

### Feedback Tests

| Component | Automated target(s) | Interaction and performance validation |
| --- | --- | --- |
| `AntAlert` | `TestAntFeedback`, `TestAntVisualRegression` | Verify status/icon/description/action/closable states, close hover, and close-region repaint. |
| `AntDrawer` | `TestAntFeedback`, `TestAntVisualRegression` | Verify all placements, mask, enter/leave slide animation, body interaction, and drawer/mask dirty regions. |
| `AntMessage` | `TestAntAdvancedInteractions`, `TestAntFeedback`, `TestAntStressLifecycle`, `TestAntVisualRegression` | Verify stack motion, close, click-through behavior, burst messages, and coalesced stack relayout. |
| `AntModal` | `TestAntModal`, `TestAntVisualRegression` | Verify mask, centered/top-offset, confirm/cancel, custom footer/content, close confirmation consumers, and button-region repaint. |
| `AntNotification` | `TestAntAdvancedInteractions`, `TestAntFeedback`, `TestAntStressLifecycle`, `TestAntVisualRegression` | Verify placements, stack motion, close, loading progress countdown, burst notifications, and progress-region repaint. |
| `AntPopconfirm` | `TestAntFeedback`, `TestAntPopupLifecycle` | Verify title/description, confirm/cancel, disabled state, placement/arrow, popup motion, and footer button dirty updates. |
| `AntPopover` | `TestAntFeedback`, `TestAntPopupLifecycle`, `TestAntVisualRegression` | Verify hover/click triggers, title/content/action, placement/arrow, stable delayed close, and popup reuse. |
| `AntProgress` | `TestAntFeedback`, `TestAntVisualRegression` | Verify line/circle/dashboard, percent text, status colors, and progress-only dirty repaint. |
| `AntResult` | `TestAntFeedback`, `TestAntVisualRegression` | Verify all statuses, title/subtitle/extra, dark theme icon transparency, and status icon cache invalidation. |
| `AntSkeleton` | `TestAntFeedback` | Verify loading/content swap, active shimmer motion, element variants, hidden-state timer behavior, and shimmer dirty band. |
| `AntSpin` | `TestAntFeedback` | Verify all sizes, delay, percent mode, visible spinner motion, hidden-state timer behavior, and spinner-bound repaint. |
| `AntToolTip` | `TestAntAdvancedInteractions`, `TestAntFeedback`, `TestAntPopupLifecycle` | Verify delayed show, placement/arrow/color, passive mouse transparency, target destruction, and popup reuse. |
| `AntTour` | `TestAntFeedback` | Verify target highlight, step navigation, mask, bubble placement, and previous/new highlight dirty updates. |
| `AntWatermark` | `TestAntAdvancedInteractions`, `TestAntDataDisplayB` | Verify tiled text, multi-line, angle/gap/offset, mouse transparency, and pixmap cache invalidation on theme/font/text changes. |

### Data Display Tests

| Component | Automated target(s) | Interaction and performance validation |
| --- | --- | --- |
| `AntAvatar` | `TestAntDataDisplayA` | Verify text/icon/image/group modes, source/size changes, DPR scaling, and cached clipped pixmap invalidation. |
| `AntBadge` | `TestAntBadge`, `TestAntVisualRegression` | Verify count/dot/status/processing/ribbon, processing animation, hidden-state timer behavior, and overlay repaint scope. |
| `AntCalendar` | `TestAntDataDisplayA` | Verify day/month/year modes, selection, today/current date, theme switching, and cell-local repaint. |
| `AntCard` | `TestAntDataDisplayA`, `TestAntVisualRegression` | Verify title/extra/actions/loading/meta/grid, spinner visibility, and layout cache invalidation. |
| `AntCarousel` | `TestAntDataDisplayB` | Verify autoplay, manual dot click, slide animation remains visible, hidden pause/resume, and viewport-only repaint. |
| `AntCollapse` | `TestAntDataDisplayB` | Verify accordion/multiple panels, expand/collapse animation, dynamic content, and animating-panel repaint. |
| `AntEmpty` | `TestAntDataDisplayA` | Verify default/simple/custom description, extra action, theme switching, and illustration cache invalidation. |
| `AntImage` | `TestAntDataDisplayA` | Verify placeholder/fallback, preview open/close, scaled image cache, and resize invalidation. |
| `AntList` | `TestAntDataDisplayB`, `TestAntVisualRegression` | Verify item/meta/action rows, selection helpers, scrolling, large lists, visible-row cache, and row-local repaint. |
| `AntQRCode` | `TestAntDataDisplayB` | Verify payload regeneration, icon/status overlays, size/DPR changes, and QR matrix/image cache invalidation. |
| `AntStatistic` | `TestAntDataDisplayA` | Verify precision/prefix/suffix/countdown, formatted value changes, and text-layout cache invalidation. |
| `AntTable` | `TestAntAdvancedInteractions`, `TestAntDataDisplayB`, `TestAntVisualRegression` | Verify sorting, selection, pagination/loading, row tooltips, large row sets, and row/header-cell repaint scope. |
| `AntTag` | `TestAntTag`, `TestAntVisualRegression` | Verify color presets, closable/checkable/disabled variants, hover/close, and tag-bound repaint. |
| `AntTimeline` | `TestAntDataDisplayB` | Verify vertical/horizontal, custom colors, item content changes, and item/connector layout cache invalidation. |
| `AntTree` | `TestAntAdvancedInteractions`, `TestAntDataDisplayB` | Verify expand/collapse, selection/checking, connectors, large trees, visible-node cache, and node-row repaint. |

### Layout And Misc Tests

| Component | Automated target(s) | Interaction and performance validation |
| --- | --- | --- |
| `AntAffix` | `TestAntLayout` | Verify scroll pin/unpin, container resize/move, target visibility, and throttled geometry checks. |
| `AntApp` | `TestAntQtExtensions` | Verify message/modal/notification context access, nested app wrappers, and no repeated object-tree scans. |
| `AntConfigProvider` | `TestAntQtExtensions`, `TestAntThemeLifecycle` | Verify theme/token/font/radius propagation, batched config updates, and no duplicated refresh signals. |
| `AntDivider` | `TestAntLayout` | Verify horizontal/vertical, dashed/text positions, theme switching, and cached text metrics. |
| `AntFlex` | `TestAntLayout` | Verify wrap/gap/vertical/alignment, child visibility changes, and layout cache invalidation. |
| `AntGrid` | `TestAntLayout` | Verify span/offset/gutter/responsive width changes and cached column calculations. |
| `AntLayout` | `TestAntLayout`, `TestAntVisualRegression` | Verify header/sider/content/footer, collapse/theme changes, and region-scoped repaint. |
| `AntMasonry` | `TestAntLayout`, `TestAntQtExtensions` | Verify shortest-column placement, resize, item add/remove/height changes, and incremental column assignment. |
| `AntSpace` | `TestAntLayout` | Verify horizontal/vertical/wrap/alignment, child visibility changes, and no redundant relayout. |
| `AntSplitter` | `TestAntQtExtensions` | Verify drag handles, nested panes, theme switching, and handle-rect dirty updates during drag. |
| `AntWidget` | `TestAntQtExtensions`, `TestAntThemeLifecycle` | Verify theme palette propagation, child surfaces, and reduced repolish without stale colors. |
| `AntWindow` | `TestAntQtExtensions` | Verify title-bar controls, theme transition animation, resize/move, close confirmation opt-in, Win10/Win11 frame paths, and shadow update throttling. |

### Qt And Desktop Extension Tests

| Component | Automated target(s) | Interaction and performance validation |
| --- | --- | --- |
| `AntDockManager` | `TestAntQtExtensions` | Verify embedded drag/drop, float/embed, guides/previews, layout switching latency, hidden-manager drop rejection, and no redundant dock-tree rebuilds. |
| `AntDockWidget` | `TestAntQtExtensions`, `TestAntChildOwnership` | Verify title-bar actions, floating window behavior, drag-back-to-layout, context menu, dark mode, and title/content repaint scope. |
| `AntLog` | `TestAntQtExtensions` | Verify large append batches, level filtering if present, scrolling to bottom, visible-line rendering, and no full-history remeasure per paint. |
| `AntMenuBar` | `TestAntQtExtensions` | Verify action hover/trigger/menu open, theme switching, and old/new action dirty repaint. |
| `AntNavItem` | `TestAntVisualRegression` | Verify hover/active/click states, sidebar navigation, icon/text layout cache, and item-local repaint. |
| `AntPlainTextEdit` | `TestAntQtExtensions` | Verify text editing, context menu, resize grip drag, variants/theme, no visual restyle on content edits, and no repeated cursor/dirty-region updates for unchanged grip hover. |
| `AntRibbon` | `TestAntQtExtensions` | Verify page/group/action layout, collapse/expand animation, popup mode, embedded controls, cached tab layout reuse, and tab/collapse/indicator-region repaint. |
| `AntScrollArea` | `TestAntQtExtensions` | Verify custom scrollbars, gesture toggles, theme switching, content replacement, cached palette reuse, and no redundant viewport repaint on content-only updates. |
| `AntScrollBar` | `TestAntQtExtensions` | Verify hover, press/release, auto-hide, orientation, theme refresh, repeated-state no-op handling, and handle-region dirty updates. |
| `AntStatusBar` | `TestAntQtExtensions` | Verify left/right items, separators, messages, size grip, cached layout reuse, and item/message-region repaint. |
| `AntToolBar` | `TestAntQtExtensions` | Verify action add/remove/trigger, floating shadow, embedded buttons, action-local synchronization, and cached button text metrics. |
| `AntToolButton` | `TestAntQtExtensions` | Verify menu/default action, arrow animation, loading spinner, hidden-state timer behavior, and spinner/arrow dirty-region animation updates. |

## First Implementation Candidates

| Order | Component group | Reason |
| --- | --- | --- |
| 1 | `AntIcon` shared cache | Broad payoff across buttons, menus, tabs, result, feedback, and tool surfaces. |
| 2 | `AntTable`, `AntTree`, `AntList`, `AntLog` | Largest benefit for real data and repeated paints. |
| 3 | `AntSelect`, `AntCascader`, `AntDatePicker`, `AntTimePicker`, `AntTreeSelect` | Popup row/layout caching improves common data-entry interactions. |
| 4 | `AntDockManager`, `AntDockWidget`, `AntWindow` | User-visible drag, resize, shadow, and layout latency; high impact but requires careful Windows verification. |
| 5 | `AntSpin`, `AntSkeleton`, `AntBadge`, `AntSwitch`, `AntCarousel` | Persistent animation costs can be reduced without changing appearance. |

## Validation Notes

- Every optimized component should record the targeted command and result in this document or in `docs/project-status.md`.
- Prefer adding a small targeted perf guard beside the existing behavior test when a regression is easy to reproduce with QTest.
- Motion verification is required for components with animations: the optimized control must still show the same visible transition, loading, hover, drag, popup, or feedback motion.
- When optimizing shared helpers, test at least one representative component from each affected category.
- Do not mark a component `Optimized` until the example builds and the relevant targeted test passes.
