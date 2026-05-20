# Component Performance Optimization Progress

Updated: `2026-05-20`

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

Current summary: `84 / 84` components have an initial plan. `9` components are optimized from this pass, and `75` remain in planned or watching states.

Latest completed optimization:

| Date | Component | Change | Validation |
| --- | --- | --- | --- |
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
| `AntButton` | P1 | Cache rendered icon paths/spinner geometry per size/state; only run loading timer while visible and loading; update spinner arc region instead of whole button when possible. | Plan ready |
| `AntFloatButton` | P1 | Reuse button/group geometry, cache badge/icon render output, and limit expand/collapse repaint to the affected group bounds. | Plan ready |
| `AntIcon` | P0 | Cache enum paths and rendered resource pixmaps keyed by icon name, colors, size, and DPR. Keep rotation/spin outside the cache through painter transforms so motion remains unchanged. | Optimized |
| `AntTypography` | P1 | Cache text layout/metrics and copy/ellipsis hit regions by text, width, font, and mode; avoid geometry refresh when only visual state changes. | Plan ready |

### Navigation

| Component | Priority | Optimization plan | Progress |
| --- | --- | --- | --- |
| `AntAnchor` | P1 | Cache link rectangles and indicator geometry; animate only the indicator dirty rect; throttle scroll-container active-section recalculation. | Plan ready |
| `AntBreadcrumb` | P2 | Cache item text widths and separator positions; repaint only changed hover item and previous hover item. | Plan ready |
| `AntDropdown` | P0 | Lazily build popup menu content, reuse popup shell, cache placement/arrow geometry, and keep outside-click closing on one lightweight path. | Plan ready |
| `AntMenu` | P0 | Cache item layout, submenu placement, and shortcut text metrics; repaint only affected menu rows during hover/selection; avoid style repolish for action text changes. | Plan ready |
| `AntPagination` | P1 | Cache page-button rectangles and text metrics; recalculate only when page count, current page, or size changes. | Plan ready |
| `AntSteps` | P1 | Cache step connector/title/description layout; dirty-update current/previous/current-hover steps instead of full widget. | Plan ready |
| `AntTabs` | P0 | Cache tab rectangles and card paths, coalesce tab drag/reorder layout updates, and repaint only indicator plus old/new active tab during animation. | Plan ready |

### Data Entry

| Component | Priority | Optimization plan | Progress |
| --- | --- | --- | --- |
| `AntAutoComplete` | P0 | Debounce filtering for large option lists, cache visible suggestion row geometry, reuse popup, and repaint only highlighted rows. | Plan ready |
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
| `AntAffix` | P1 | Throttle scroll-event geometry checks, skip work when pinned state and target rect are unchanged, and update only on relevant container move/resize/scroll. | Plan ready |
| `AntApp` | P2 | Keep as a lightweight context wrapper; ensure message/modal/notification lookups are cached and do not scan object trees repeatedly. | Plan ready |
| `AntConfigProvider` | P1 | Batch theme/config changes and emit one consolidated refresh when multiple tokens change together. | Plan ready |
| `AntDivider` | P2 | Cache text metrics and line positions; repaint only on text/orientation/theme changes. | Plan ready |
| `AntFlex` | P1 | Cache child layout pass results and invalidate only on child size hint, gap, wrap, orientation, or geometry changes. | Plan ready |
| `AntGrid` | P1 | Cache row/column width calculations and invalidate only on span/gutter/container-width changes. | Plan ready |
| `AntLayout` | P1 | Cache region geometry for header/sider/content/footer and repaint only changed regions on collapse/theme changes. | Plan ready |
| `AntMasonry` | P0 | Cache column assignment and item heights; recompute incrementally when one item changes instead of rebuilding all columns. | Plan ready |
| `AntSpace` | P2 | Cache spacing layout and skip relayout when child count/visibility/size hints are unchanged. | Plan ready |
| `AntSplitter` | P0 | Dirty-update handle rect during drag and defer expensive child layout persistence until release when possible. | Plan ready |
| `AntWidget` | P1 | Reduce theme-change repolish to cases where palette/metrics really changed; otherwise update only its own surface. | Plan ready |
| `AntWindow` | P0 | Keep native behavior intact; measure theme transition capture cost, cache title-button icon paths, update title-bar subregions, and keep Win10 shadow updates throttled during live resize. | Watching |

### Qt And Desktop Extensions

| Component | Priority | Optimization plan | Progress |
| --- | --- | --- | --- |
| `AntDockManager` | P0 | Cache dock tree geometry, drop-guide hit zones, and preview rectangles; apply embedded layout changes synchronously but avoid redundant full tree rebuilds during embedded drags. | Watching |
| `AntDockWidget` | P0 | Cache title-bar button/title geometry, minimize floating-window native churn, and scope drag/float/embedded repaint to affected panes. | Watching |
| `AntLog` | P0 | Use document-cursor appends, cached level formats, disabled undo history, and batched trim operations so large append bursts avoid moving the visible cursor or repeatedly shifting entries one by one. | Optimized |
| `AntMenuBar` | P1 | Cache action rectangles and text metrics; repaint only old/new hover or active action. | Plan ready |
| `AntNavItem` | P1 | Cache icon/text/active indicator geometry, repaint only old/new hover or active item regions, and avoid full sidebar updates during navigation hover. | Plan ready |
| `AntPlainTextEdit` | P1 | Keep theme/style wrapper cheap, update resize grip only on hover/drag, and avoid repolish on plain text content edits. | Plan ready |
| `AntRibbon` | P0 | Cache page/group/action layout, repaint only active tab/group on hover, and defer collapsed popup layout until opened. | Plan ready |
| `AntScrollArea` | P1 | Avoid redundant scrollbar replacement/palette updates on theme changes; keep gesture scrolling independent from full viewport repaint. | Plan ready |
| `AntScrollBar` | P0 | Dirty-update handle/track hover rects, stop auto-hide timers when not needed, and avoid full parent viewport invalidation. | Plan ready |
| `AntStatusBar` | P1 | Cache item widths and message layout; repaint only changed item/message region. | Plan ready |
| `AntToolBar` | P1 | Cache action/button layout and repaint only affected action on hover/press/theme icon change. | Plan ready |
| `AntToolButton` | P1 | Cache icon/arrow geometry, pause spinner only when hidden or not loading, and update only arrow/spinner/button dirty region during animation. | Plan ready |

## Per-Component Test Plan

For every optimization, run the listed QTest target(s), build `qt-ant-design-example`, and perform the listed interaction/performance validation. If the optimization touches shared code, also run one representative target from each affected category.

### General Tests

| Component | Automated target(s) | Interaction and performance validation |
| --- | --- | --- |
| `AntButton` | `TestAntButton`, `TestAntFeedback` when wave/loading is touched | Verify hover/press/focus/disabled, icon buttons, loading spinner motion, and repeated state toggles without full-button repaint churn. |
| `AntFloatButton` | `TestAntFloatButton` | Verify group expand/collapse, BackTop, badge, hover motion, and no extra repaint outside the floating group bounds. |
| `AntIcon` | `TestAntIcon`, plus one representative consumer such as `TestAntButton` | Verify all icon modes still render, theme color changes invalidate cache, DPR/size changes are crisp, and repeated icon paint avoids reparsing. |
| `AntTypography` | `TestAntTypography` | Verify title/text/paragraph/link, wrapping, ellipsis, copy feedback, theme switching, and layout metrics after cached text layout changes. |

### Navigation Tests

| Component | Automated target(s) | Interaction and performance validation |
| --- | --- | --- |
| `AntAnchor` | `TestAntNavigation` | Verify scroll tracking, active link changes, indicator animation, many-anchor scroll latency, and no missed active updates after throttling. |
| `AntBreadcrumb` | `TestAntNavigation` | Verify item hover/click, separators, disabled items, theme switching, and dirty repaint only for changed breadcrumb items. |
| `AntDropdown` | `TestAntInteractions`, `TestAntNavigation`, `TestAntPopupLifecycle` | Verify click/hover/context triggers, placement/arrow, outside close, popup motion, and repeated open/close latency. |
| `AntMenu` | `TestAntInteractions`, `TestAntNavigation`, `TestAntVisualRegression` | Verify action sync, hover/selection, submenu behavior, shortcut text, large menu movement, and row-scoped repaint. |
| `AntPagination` | `TestAntNavigation`, `TestAntVisualRegression` | Verify page changes, quick jumper, page size changes, disabled states, and button-geometry cache invalidation. |
| `AntSteps` | `TestAntNavigation`, `TestAntVisualRegression` | Verify current/error/clickable steps, horizontal/vertical modes, connector drawing, and old/new step dirty updates. |
| `AntTabs` | `TestAntAdvancedInteractions`, `TestAntNavigation`, `TestAntStressLifecycle`, `TestAntVisualRegression` | Verify active indicator animation, card/line/editable modes, tab close/disable fallback, tab drag/reorder, and content layout stability. |

### Data Entry Tests

| Component | Automated target(s) | Interaction and performance validation |
| --- | --- | --- |
| `AntAutoComplete` | `TestAntDataEntryA` | Verify filtering, keyboard navigation, popup reuse, highlighted-row repaint, and large suggestion lists. |
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
| `AntPlainTextEdit` | `TestAntQtExtensions` | Verify text editing, context menu, resize grip, variants/theme, and no style repolish on content edits. |
| `AntRibbon` | `TestAntQtExtensions` | Verify page/group/action layout, collapse/expand animation, popup mode, embedded controls, and active-group repaint. |
| `AntScrollArea` | `TestAntQtExtensions` | Verify custom scrollbars, gesture scrolling, theme switching, and no redundant viewport repaint. |
| `AntScrollBar` | `TestAntQtExtensions` | Verify hover/drag/autohide, orientation, parent scrolling, and handle/track dirty updates. |
| `AntStatusBar` | `TestAntQtExtensions` | Verify left/right items, separators, messages, size grip, and item/message-region repaint. |
| `AntToolBar` | `TestAntQtExtensions` | Verify action add/remove/trigger, floating shadow, embedded buttons, and action-local repaint. |
| `AntToolButton` | `TestAntQtExtensions` | Verify menu/default action, arrow animation, loading spinner, hidden-state timer behavior, and dirty-region animation updates. |

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
