# Project Status

Updated: `2026-05-21`

This snapshot records the current state after the Showcase, ColorPicker popup, AntWindow outline and desktop-window polish, official Ant Design Icon resource work, the 2026-04-30 interaction/motion parity pass, Qt5/Qt6 static/shared build-system support, installed package coverage, lifecycle stress coverage, and README component screenshot gallery.

## Summary

| Area | Status |
| --- | --- |
| Ant Design standard coverage | `70 / 70` top-level components covered |
| Public Qt component count | `84` public components |
| Widget headers | `105` headers in `src/widgets`: `84` public component headers, `20` Qt-style alias headers, and the internal popup helper `AntSelectPopup` |
| Qt / desktop extensions | `14` components |
| Style architecture | `62` `Ant*Style` classes, plus custom-paint/helper components where a style class is not useful |
| Example coverage | `84 / 84` public components, plus the standalone `Showcase` page; `AntDockManager` is demonstrated on the DockWidget page |
| Dedicated examples intentionally absent | None |
| Tests | `37` CTest targets configured; latest full component reliability sweep passed `37 / 37` in Debug on `2026-05-10` |
| Official icon resources | `831` SVG files from `@ant-design/icons-svg@4.4.2` |
| README component gallery | `166` committed PNGs: light/dark screenshots for `83` visual component rows; `AntDockManager` is demonstrated through the DockWidget page |
| Reliability coverage | Per-component matrix in `docs/reliability-coverage.md`; every public component has behavior/API, lifecycle, meta, theme, and render coverage |
| Performance optimization | Initial per-component plan, progress matrix, and test matrix in `docs/performance-optimization.md`; `84 / 84` public components have a defined optimization and validation path, with `79` controls optimized in the current pass |

## Recent Completed Work

- Added `AntDockManager` as the themed docking workspace companion for `AntDockWidget`, with a custom splitter/tab dock tree instead of Qt's native dock layout, center tab placement, serialized splitter/tab/floating named perspectives, draggable tab reordering, tab/title context menus, programmatic floating and dock feature APIs, threshold-activated translucent drag previews, toggleable center/edge drop guide squares, deterministic guided drop placement, manager-owned floating dock windows using the AntWindow-style Windows native frame/DWM rounded-corner/shadow path, double-click maximize/restore, and drag-back-to-layout support.
- Added `docs/performance-optimization.md` as the starting point for the performance pass, with conservative optimization rules, validation expectations, priority order, and per-component progress/test tables covering all `84` public components.
- Optimized `AntButton` by pausing hidden loading spinners and scoping spinner animation frames to the spinner indicator region.
- Optimized `AntFloatButton` by skipping unchanged positioning, reusing child group layout geometry and visibility, and avoiding duplicate style-level theme refresh.
- Optimized `AntTypography` by caching repeated text measurements and copy hit rectangles, while scoping copy hover/press repaint to the copy icon region.
- Optimized `AntAnchor` by caching link indicator geometry, applying label visual state outside paint, scoping indicator animation repaint, and coalescing scroll-value bursts.
- Optimized `AntBreadcrumb` by caching item/separator geometry across size, paint, and hit-test paths, with hover repaint scoped to changed item regions.
- Optimized `AntDropdown` by caching popup content width and skipping repeated popup margin, menu-width, and geometry applications when target placement inputs are unchanged.
- Optimized `AntMenu` by caching visible item rectangles for paint, hit testing, keyboard navigation, size hints, and submenu popup placement, while scoping hover, press, selection, popup-parent highlight, and action-state repaint to affected rows.
- Optimized `AntPagination` by caching page-button geometry for size hints, hit testing, quick jumper placement, and style painting, with hover repaint scoped to changed button regions.
- Optimized `AntSteps` by caching step item, icon, and text rectangles for hit testing and style painting, with current, hover, and status repaint scoped to affected step regions.
- Optimized `AntTabs` by caching tab, close, add, text, icon, and card-path layout for paint and hit testing, with hover, add-button, active-tab, and indicator repaint scoped to affected regions.
- Optimized `AntAutoComplete` by caching filtered suggestions, reusing popup suggestion widgets, skipping unchanged popup geometry, and limiting keyboard highlight repaint to old/new rows.
- Optimized `AntCascader` by caching popup column state and popup metrics, skipping unchanged popup geometry, and scoping hover plus child-column expansion repaint to affected rows/columns.
- Optimized `AntCheckBox` by caching indicator/text geometry, size hints, and mark paths, with checked, hover, and press repaint scoped to the indicator region.
- Optimized `AntDatePicker` by caching popup day-cell geometry/state, scoping hover repaint to old/new date cells, invalidating on month/range changes, and reusing widget-owned input metrics in the style.
- Optimized `AntDescriptions` by updating header and item text in place, rebuilding the grid only for structural changes, applying theme changes to existing cells, and caching size hints.
- Optimized `AntForm` by refreshing item label/help/extra/required/colon/status state in place, skipping repeated unchanged form settings, and applying spacing/item changes incrementally.
- Optimized `AntInput` by caching metrics and size hints, skipping unchanged addon/icon/widget layout rebuilds, and scoping action/status repaint work to smaller regions.
- Optimized `AntInputNumber` by caching metrics and size hints, keeping control-reveal animation to inset/overlay/control-region updates, skipping unchanged overlay geometry and progress work, and scoping hover/focus/status repaint to frame/control regions.
- Optimized `AntMentions` by coalescing suggestion refreshes, caching filtered results by suggestions revision and filter text, reusing popup rows, skipping unchanged popup geometry, and scoping keyboard/mouse highlight repaint to changed rows.
- Optimized `AntRadio` by caching size hints, indicator/text rectangles, and ButtonStyle segment paths, reusing rounded-edge resolution, and scoping checked/hover/press repaint to indicator or button regions.
- Optimized `AntRate` by caching star metrics, size hints, star rectangles, and painter paths, scoping value/hover/focus repaint to affected star or outline regions, and keeping selection pulse frames to one star.
- Optimized `AntSegmented` by caching segment geometry, size hints, text metrics, icon/text drawing rectangles, and radii, while scoping hover, press, selection, and thumb animation repaint to affected regions.
- Optimized `AntSelect` by caching metrics and size hints, reusing popup option rows across rebuilds/editable filtering, skipping unchanged popup geometry, and repainting only changed visible option rows for current/highlight/selection updates.
- Optimized `AntSlider` by caching slider metrics, size hints, groove/track/handle geometry, dot centers, and mark layouts, while scoping value, range, handle-scale, and focus animation repaint to affected regions.
- Optimized `AntSwitch` by caching switch metrics and track/handle geometry, scoping handle and loading animation repaint to small regions, and pausing loading timers while hidden.
- Optimized `AntTimePicker` by caching popup panel/column/row geometry, scoping hover updates to old/new rows, repainting only changed time columns, and skipping repeated popup placement work.
- Optimized `AntTransfer` by caching pane, header, visible-row, checkbox, and scrollbar geometry, keeping source/target data in widget-owned lists, and scoping selection, scroll, and move repaint work to rows or panes.
- Optimized `AntTreeSelect` by caching trigger geometry and title lookup, reusing the popup tree across opens, skipping unchanged tree-data rebuilds and popup geometry/size applications, and resizing the popup from active tree expansion state.
- Optimized `AntUpload` by caching trigger/list/card geometry, reusing widget-owned file data in style painting, caching thumbnail pixmaps by path, and scoping hover/status/progress repaint to changed upload regions.
- Optimized `AntAlert` by caching alert layout/size metrics and icon pixmaps, removing duplicate style hover tracking, and scoping close-hover repaint to the close button rectangle.
- Optimized `AntDrawer` by caching overlay/panel geometry, skipping redundant theme/header and overlay geometry work, removing an extra open-time panel placement pass, and making overlay paint respect scoped dirty regions.
- Optimized `AntMessage` by caching message layout and shadow pixmaps, scoping loading-spinner repaint to the icon region, and skipping unchanged stack size/position work during relayout.
- Optimized `AntModal` by caching dialog geometry, theme/body/footer sync state, and DPR-aware panel shadow pixmaps, while scoping mask animation repaint to the active paint region and preserving the `AntWindow` close-confirmation path.
- Optimized `AntNotification` by caching notification layout and shadow pixmaps, scoping progress/loading/close-hover repaint to small regions, and skipping unchanged stack size/position work during relayout.
- Optimized `AntPopconfirm` by reusing the popup action container/buttons across footer text and visibility changes, skipping unchanged content sync, and removing the no-op self-paint filter path.
- Optimized `AntPopover` by caching popup size hints, bubble/header/body/action rectangles, and arrow geometry on the widget, sharing that cache with the style, skipping repeated target placement work, and avoiding unnecessary hover close-timer churn when already closed.
- Optimized `AntProgress` by caching line/circle geometry, status colors, status info text, and size hints on the widget, repainting line progress changes and active shine frames only in the affected regions, and pausing active animation while hidden.
- Optimized `AntResult` by caching result text/extra layout and status icon pixmaps on the widget, with status changes scoped to the icon region and dark-mode transparent icon rendering preserved.
- Optimized `AntSkeleton` by caching placeholder geometry/paths and theme colors on the widget, scoping shimmer frames to the old/new highlight bands, skipping unchanged content geometry, and pausing shimmer while hidden.
- Optimized `AntSpin` by caching indicator/text/arc geometry and theme colors on the widget, repainting animation ticks only in the spinner bounds, and stopping the precise timer for hidden, disabled, or percent-mode states.
- Optimized `AntToolTip` by caching bubble/text/arrow geometry and colors on the widget, skipping unchanged target positioning work, and guarding delayed-open timers against repeated enter/focus and target destruction.
- Optimized `AntTour` by caching overlay target/spotlight rectangles, tooltip geometry, and step content state, then repainting only old/new spotlight regions during step changes.
- Optimized `AntWatermark` by caching the tiled rotated watermark pattern as a transparent DPR-aware pixmap and blitting it on paint while preserving mouse transparency.
- Optimized `AntAvatar` by caching clipped and scaled image avatar pixmaps by source, size, DPR, shape, and border radius, so image-mode avatars no longer reload or reclip the same source during repeated paints.
- Optimized `AntBadge` by caching indicator/status/ribbon geometry and colors on the widget, letting processing animation repaint only the overlay status-dot region, and stopping its timer when hidden or no longer processing.
- Optimized `AntCalendar` by skipping full model resets for same-month selected-date changes, refreshing only old/new selected day cells, caching row-height metrics, and skipping unchanged header button synchronization.
- Optimized `AntCard` by caching frame/header/action/spinner paint geometry on the widget, using scoped spinner-region updates during loading animation, pausing the spinner timer while hidden/disabled, and skipping unchanged chrome visibility/margin/height applications.
- Optimized `AntNavItem` by moving label style updates out of paint and caching hover/active background plus indicator geometry.
- Optimized `AntPlainTextEdit` by caching visual state application and scoping resize-grip cursor/paint updates to real hover-state changes across the editor's internal mouse-event targets.
- Optimized `AntRibbon` by caching tab layout rectangles and repainting only affected tab, collapse button, and indicator regions during hover and indicator animation.
- Optimized `AntScrollArea` by caching theme surface colors and content-widget palette state so content replacement does not repaint or repalette the viewport unnecessarily.
- Optimized `AntScrollBar` by scoping hover, press, auto-hide, enabled-state, and theme repaint work to the slider handle region.
- Optimized `AntStatusBar` by caching item, divider, and message layout rectangles and scoping hover/message repaint to changed regions.
- Optimized `AntToolBar` by syncing action-created buttons incrementally, skipping unchanged button chrome work, and caching button text metrics across size and paint paths.
- Optimized `AntToolButton` by pausing hidden loading spinners and scoping spinner/arrow animation frames to their indicator regions.
- Optimized `AntMenuBar` by caching action geometry and menu item text metrics, with hover movement repainting only old/new action regions.
- Optimized `AntDockWidget` title-bar work by caching button icon pixmaps and skipping redundant chrome/theme refreshes when dock state inputs are unchanged.
- Optimized `AntDockManager` drag hit testing by caching visible dock-area hit zones and reusing same-position drop-target queries across drag move/release handling.
- Optimized `AntWindow` title-bar interaction by caching button rectangles and repainting only affected title-bar button regions during hover, press, and visibility changes.
- Optimized `AntWidget` theme handling by caching style, palette, and size-hint inputs so normal theme switches avoid repolish and geometry refresh unless those inputs changed.
- Optimized `AntSplitter` by caching handle colors at the parent splitter level and invalidating handle paint caches only on hover or theme revision changes.
- Optimized `AntSpace` by caching size hints, invalidating them from child layout events, and appending items without rebuilding the whole layout.
- Optimized `AntMasonry` by caching column placement state and incrementally laying out newly appended widgets into the current shortest column.
- Optimized `AntLayout` by caching region geometry, skipping unchanged `setGeometry()` calls, and refreshing content immediately when a sider's width/collapse state changes.
- Optimized `AntGrid` by incrementally placing added columns, initializing column stretch once, and keeping gutter changes to spacing-only updates.
- Optimized `AntFlex` by caching horizontal wrap layout geometry and size hints across repeated layout queries.
- Optimized `AntDivider` by caching title metrics, line pen, text rectangle, and divider line geometry across repeated paints.
- Optimized `AntConfigProvider` by coalescing multiple property setter calls into one queued configuration refresh signal and by skipping redundant global theme applies.
- Optimized `AntApp` by caching feedback host resolution for message/modal/notification entry calls and by restoring nested app instances safely when wrappers are destroyed.
- Optimized `AntAffix` by coalescing high-frequency scroll/resize checks, skipping unchanged geometry, and updating affixed geometry without duplicate state transitions.
- Optimized `AntTree` by caching flattened visible nodes across paint, hit testing, size hints, and scroll bounds, and by scoping hover repaint to changed rows.
- Optimized `AntTimeline` by caching vertical item-height layout and parsed dot colors, while reusing title/content font metrics during paint.
- Optimized `AntTag` by caching size-hint metrics and parsed tag colors, and by skipping mouse-move repaint work when hover state has not changed.
- Optimized `AntStatistic` formatting by caching the displayed number/countdown text in the widget and letting the style reuse it instead of recalculating on every paint.
- Optimized `AntQRCode` rendering by caching the module layer as a DPR-aware pixmap keyed by matrix revision, size, DPR, and foreground color while keeping status overlays, borders, and center icons separately painted.
- Optimized `AntList` bulk insertion paths so `addItems()` and `insertItems()` coalesce item adoption, layout, geometry, and repaint work into one pass, with targeted coverage for batch insertion and internal scrolling.
- Optimized the first `AntTable` hot path: hover and leave now repaint only the previous/current row dirty rectangles, with a targeted QTest verifying row-scoped updates while keeping sorting, selection, and pagination coverage intact.
- Added a pure-control Showcase page matching the Ant Design homepage showcase without the website shell, theme sidebar, or decorative background.
- Reworked `AntColorPicker` from a modal editor to an AntD-like click-open popup panel.
- Added a non-maximized `AntWindow` outline so frameless windows remain distinguishable on similarly colored desktop backgrounds.
- Bundled all official Ant Design SVG icons and connected them to `AntIcon` through a string-name API.
- Added `docs/ant-design-icons.md` with the full icon inventory.
- Updated visual audit notes for Showcase, ColorPicker popup, Icon resources, and Qt extension consistency.
- Completed an issue-driven Ant Design parity pass for interactive behavior and motion:
  - Stabilized `AntPopover` hover popups.
  - Aligned loading button spinner direction and visible arc.
  - Added TextArea-style resize handling to `AntPlainTextEdit`.
  - Animated `AntInputNumber` handler reveal on hover/focus.
  - Added `AntSlider` drag value bubble.
  - Added grey click wave feedback to `AntSwitch`.
  - Fixed `AntTransfer` scrolling, visible-row selection, and header select-all.
  - Added slide transitions to `AntCarousel`.
  - Enabled actual row reordering for `AntTable` sorter clicks.
  - Reworked `AntTabs` line/card visuals and active indicator motion.
  - Strengthened `AntNotification` elevation and placement-aware enter/exit motion.
  - Aligned `AntMessage` popup motion and elevated bubble styling.
  - Restored animated shimmer movement for `AntSkeleton`.
  - Improved `AntSpin` animation smoothness with a precise 16 ms timer.
- Added installed CMake package consumer coverage with `find_package(qt-ant-design CONFIG REQUIRED)`.
- Added lifecycle stress coverage for repeated theme switching, popup open/close cycles, transient feedback cleanup, and example auto-close behavior.
- Expanded automated visual regression guards for InputNumber handler visibility, selection primary fills, Tag/Badge status colors, Message/Notification elevated surfaces, Card/List/Table/Descriptions data-display borders, and Menu/Pagination/Steps/Layout/Popover/Modal/Drawer structure.
- Completed an `AntWindow` desktop behavior pass:
  - Added Windows 11 Snap support for the frameless window path, including resize hit-testing, title-bar drag, maximize-button Snap Layout hover, edge snapping, and drag-to-restore behavior.
  - Added DWM rounded corners, border/shadow integration, and a public `cornerRadius` API for Windows builds.
  - Added title-bar pin and light/dark theme buttons with bundled official Ant Design icons, plus public visibility APIs for every title-bar button.
  - Reworked title-bar hover state cleanup so hover colors clear reliably when leaving title/content/native areas.
  - Added a captured-frame `AntWindow` theme transition overlay with an 8 ms timer, 320 ms duration, smootherstep easing, high-DPI-safe captures, and a soft reveal that avoids black-hole artifacts.
  - Embedded the Windows 10/11 compatibility manifest in the example app so the native Snap Layout flyout can appear on the maximize button.
  - Split the Windows native frame policy by OS build: Windows 11 keeps the caption-backed Snap Layout path, while Windows 10 removes `WS_CAPTION`, uses a legacy rounded mask for non-maximized corners, preserves a 1 px DWM extended frame for the outer shadow, and leaves maximized `WM_NCCALCSIZE` rectangles unshrunk so the work area is fully covered.
  - Reworked visible Windows topmost toggles to use native `SetWindowPos(HWND_TOPMOST/HWND_NOTOPMOST)`, preserving the Qt window flags without forcing a hide/show cycle.
- Expanded Qt-style compatibility for `AntList` / `AntListWidget` to cover common `QListWidget` workflows: string item insertion, lookup/sorting, item data roles, current row/item state, selection state, internal scrolling, `scrollToItem`, and item/current/selection signals.
- Expanded `AntTable` helper coverage with `rows()`, `selectRow()`, `currentRowIndex()`, and row-level tooltip data/display support.
- Added `AntTypography::setPixelSize()` / `pixelSize()` for direct label-style font sizing while keeping theme-aware rendering.
- Aligned `AntTypography` enabled state with Qt conventions: `setEnabled(false)` now drives Typography's disabled palette/interaction state, and `setDisabled()` keeps QWidget enabled state in sync.
- Added `AntSelect` option-style list management helpers: `setOptionText()`, `removeOption()`, and `optionData()`, alongside the existing `findData()` lookup path.
- Aligned `AntSelect` with `QComboBox` for the empty-to-populated path: the first enabled option added in single-select mode becomes the current item automatically.
- Tuned `AntList` selected-row highlight geometry so left/right inset matches the existing top/bottom inset for a balanced row highlight.
- Fixed `AntCard` title and Meta title label palettes after Light/Dark theme changes so dark-mode card headers stay legible.
- Fixed issue-driven popup details for `AntMenu` horizontal submenu panel edges and `AntCascader` outside-click dismissal.
- Audited Qt layout adaptivity against native widget baselines: line-edit-like controls, combo-like selectors, spin/date/time editors, slider/progress, list/table/tree views, scroll/text/status controls, and typography now carry native-like `QSizePolicy` / height-for-width behavior with targeted render-smoke coverage.
- Added `AntTabs` content-page layout margin normalization: pages added with default Qt root-layout margins are reset to zero to avoid double spacing with Ant containers, explicit custom margins are preserved, and `AntTabs::useTabContentLayout()` is available for forced tab-pane layout setup.
- Aligned `AntInputNumber` with the desired integer-first decimal flow: the default display remains integer (`decimals() == 0`), while `setDecimals()` / `setPrecision()` enables decimal value/range/step behavior, preserves quarter-step increments, and updates the public `precision()` state with `precisionChanged`.
- Fixed `AntSlider` marked and range interaction details: marked sliders reserve label height under Qt layouts, drag value bubbles anchor above the visual handle, and range drags no longer paint a phantom primary handle at the minimum edge.
- Added `AntDesign::initialize(&app)` as the one-call consumer startup path for `qt_ant_design` resource registration, bundled font application, and theme singleton initialization.
- Added `AntRibbon` as a lightweight Ribbon component with pages, groups, large/small actions, embedded Ant/Qt widgets, collapsed popup mode, and `AntWindow` top-area integration.
- Added a README component screenshot gallery with light/dark thumbnails for all public components, and refreshed popup/feedback-heavy controls to show representative open or active states instead of only trigger buttons.
- Fixed the `AntTour` example page so the Step 1 / 2 / 3 buttons launch the corresponding tour step through the new `AntTour::start(index)` path.
- Fixed `AntResult` dark-mode status icon rendering so the icon is drawn directly with a transparent background instead of rendering an opaque widget tile.
- Added an `AntNotification` example entry for loading notifications with a bottom countdown progress bar that closes automatically when the duration completes.
- Fixed `AntSteps` icon geometry so the first circular step indicator keeps a small left inset and is no longer clipped by the widget edge.
- Fixed `AntColorPicker` trigger border geometry so normal gray and focused/active primary borders are drawn fully inside the widget frame instead of being clipped at the edges.
- Aligned `AntRadio` ButtonStyle click feedback with `AntButton` by using the full edge-expansion Wave duration instead of the short indicator-style wave.
- Added the missing `AntRate` selected-star scale pulse and a small internal paint inset so the animated star edge is not clipped.
- Reworked the `AntSlider` drag value bubble as a single rounded-rect-plus-arrow path so the arrow no longer appears visually separated from the body.
- Fixed `AntPagination` More Options quick jumper so the input box is backed by a real `QLineEdit`, accepts page numbers, clamps to the valid page range, and emits the normal page-change signals.
- Reworked the Popover surface path used by `AntPopconfirm` so the arrow and rounded panel are painted as one joined shape without an internal seam.
- Expanded `AntModal`'s transparent dialog shadow margin so the multi-layer feather fades out before the dialog widget edge instead of leaving a clipped boundary line.
- Reworked the `AntWindow` outer shadow on the Windows 10 no-caption path to use a transparent software shadow host behind the main window, with a lightweight 14px Win11-like feather that starts directly at the window edge, capped corner opacity, symmetric shadow pixels on all four sides, and geometry that tracks visible resizes consistently.
- Fixed shared popup elevation shadows by making `AntTheme::drawEffectShadow()` paint a softer multi-layer feather outside the panel body, and widened popup transparent margins for Dropdown/Menu/Cascader/ColorPicker/Select/DatePicker/TimePicker-style panels so the shadow fades before the popup edge instead of being visibly clipped.
- Fixed `AntSegmented` click hit testing so the whole visible segmented track, including its padded edge pixels, maps to the intended option in horizontal and vertical modes; added signal/API assertions for the real mouse-click path.
- Stabilized interaction reliability tests for Cascader and DatePicker popup selection by deriving click points from the current popup geometry instead of stale pre-shadow coordinates.
- Added `docs/reliability-coverage.md` and a `TestAntCoverageInventory` guard so every public component must remain covered by at least one component-specific behavior/API test in addition to lifecycle, meta-property, theme-lifecycle, and render-smoke coverage.
- Fixed `AntMessage` so a toast that visually covers controls still closes on click while forwarding that click to the underlying anchor widget, preventing covered controls such as `AntSegmented` from missing the selection change.
- Completed a passive-overlay hit-test audit: interactive popups keep their normal input capture, while non-interactive overlays no longer steal covered-control clicks. `AntToolTip`, `AntWatermark`, and the `AntSlider` drag value bubble are mouse-transparent, and `AntNotification` cleanup now guards relayout against a destroyed anchor.
- Refreshed the README dark thumbnails for `AntResult`, `AntSpin`, and `AntTour`; `AntResult` now shows transparent dark-mode status icons, `AntSpin` uses a token-driven dark content surface, and `AntTour` captures the active Step 1 overlay state.
- Refreshed and corrected the README dark thumbnails for `AntCalendar`, `AntAnchor`, `AntSplitter`, `AntLayout`, and `AntScrollBar`; `AntCalendar` now themes its internal `QTableView` viewport, `AntLayoutFooter` uses `colorBgLayout`, and the Anchor/Splitter/ScrollBar demo surfaces no longer keep hard-coded light fills in dark mode.
- Replaced the shared `image-basic` demo asset and refreshed the affected README component thumbnails for `AntAvatar` and `AntImage` in both light and dark modes.
- Updated `AntQRCode`'s default value, example, and README light/dark thumbnails so the default QR code scans to the repository URL: `https://github.com/sorrowfeng/qt-ant-design`; the QRCode example now also exposes an editable `AntInput` and primary Regenerate button that call `AntQRCode::setValue()` to rebuild the embedded byte-mode + Reed-Solomon QR matrix.

Latest Win10 AntWindow opaque-path validation:

```powershell
cmake --build build --config Debug --target qt-ant-design-example TestAntQtExtensions
"build/tests/Debug/TestAntQtExtensions.exe" dockManager windowLegacyFramePolicyRestoresShadowAfterResize windowNativeHitTestSupportsSnapZones windowDwmFrameMarginsPreserveShadow
```

Result: on `2026-05-20`, Win10 AntWindow now takes a fully opaque path (no `WA_TranslucentBackground`, no `AntWindowCornerSmoother`, no DWM glass extension during edge resize, no `DWMWA_BORDER_COLOR` gray frame, and `WM_NCACTIVATE` / `WM_NCPAINT` consumed). The window paints square corners with the legacy software shadow drawing a square outline. The Win11 caption path is unchanged. User-reported symptoms cleared on real Win10 hardware: dock float/embed stutter, page-switch stutter, repeated-resize black screen, four-edge gray frame, refocus focus rectangle, and "corners stuck square after an interrupted resize loop". Follow-up issues for theme-transition speed and AntColorPicker popup/drag responsiveness are resolved in `docs/issue-log.md` items #43 and #44.

Latest targeted AntWindow transition / AntColorPicker popup validation:

```powershell
cmake --build build --config Debug --target TestAntQtExtensions
ctest --test-dir build -C Debug -R "^TestAntQtExtensions$" --output-on-failure
```

Result: `TestAntQtExtensions` passed on `2026-05-20`. The test now verifies that `AntWindow` theme transition capture uses synchronous `render()` without event-loop capture, uses 220 ms / 16 ms transition timing, switches Win10 opaque-path windows to a light crossfade overlay, and keeps the Win11 caption path on circular reveal. The same target verifies `AntColorPicker` popup windows have no QFrame/native shadow edge, their bottom software shadow fades without a hard stacked line, and the hue/saturation field reuses its cached background while dragging so only the indicator region repaints.

Follow-up: the theme transition path now activates the visible window layout tree before capturing the new themed frame, preventing Showcase wrapped text from being painted with new font metrics inside stale geometry. The targeted `windowThemeButtonShowsTransitionOverlay` subtest checks that a theme-dependent `sizeHint()` change is applied immediately when the theme button click returns.

Follow-up: `AntColorPicker` popup shadow margin is now tightened to 28px for the 12px software shadow, removing the remaining bottom transparent top-level edge while preserving the soft shadow fade. The targeted `colorPicker` subtest checks the margin, native-shadow flags, bottom alpha fade, and cached hue/saturation drag path.

Follow-up: `AntColorPicker` drag refresh now coalesces HEX / alpha slider / preview / owner trigger / public signal updates into a 16ms live-refresh timer while keeping the HS cursor repaint immediate. The targeted `colorPicker` subtest verifies repeated drag events reuse the cached field background and collapse into one live refresh.

Follow-up: the hot HS drag field now declares opaque/no-system-background painting and fills its cached pixmap with the popup elevated surface before drawing the color field. This keeps cursor repaint on a small opaque backing-store region instead of forcing transparent popup background recomposition; the targeted `colorPicker` subtest verifies the opaque paint attributes and fully opaque corner pixel.

Follow-up: on Windows the HS drag field is now an isolated native child surface (`WA_NativeWindow` with no native ancestors) while remaining opaque/static. This keeps the high-frequency drag repaint out of the transparent popup layered-window upload path; the targeted `colorPicker` subtest verifies the native drag surface flag.

Follow-up correction: the native child drag surface caused the HS cursor to stop visually following the mouse in the real example. The first correction used a dedicated cursor overlay, but that could leave white trail fragments inside the transparent popup. The current fix draws the cursor in the HS field itself while keeping cached-background dirty repaint for only the old/new cursor rectangles. The `colorPickerDragSmoothness` subtest sends 240 drag events, verifies cursor position on every event, keeps dispatch below 80ms in Debug, confirms live refresh is still coalesced, and scans the saturated field area for stray white pixels.

Follow-up: `AntColorPicker` popup now uses a manual outside-click close path instead of relying on native popup auto-hide, so both trigger-close and outside-close go through the same 10px fade/slide enter and leave animation. The targeted `colorPicker` subtest verifies the motion properties and that close keeps the popup visible until the leave animation finishes.

Follow-up: `AntWindow` close confirmation is now opt-in through `setCloseConfirmationEnabled(true)`, and public APIs customize the Modal title, content, exit text, and cancel-exit text. `forceClose()` is available for automation or programmatic shutdown paths that should bypass confirmation. The targeted `window` subtest verifies the default close path skips the Modal, cancel keeps the window open after opt-in, confirm closes it, and custom text is propagated to the Modal.

Follow-up: the performance pass started with `AntIcon`. Built-in enum path generation is now cached, and resource SVG icons are rendered into `QPixmapCache` entries keyed by icon name, color pair, size, and device pixel ratio. Rotation and spin remain painter transforms, so visible animation is preserved while repeated paints avoid qrc file reads and `QSvgRenderer` setup. Targeted validation covered `TestAntIcon` plus `TestAntButton` as a representative icon consumer.

Follow-up: `AntLog` now appends through a `QTextDocument` cursor instead of moving the visible `QPlainTextEdit` cursor, disables undo history for log output, caches per-level text formats on theme changes, and trims overflow entries in one batch. The targeted `log` subtest now covers bulk append, max-entry trimming, document block count bounds, theme refresh, and the optimized document-cursor path.

Latest targeted Win10 dock re-embed repaint cadence validation:

```powershell
cmake --build build --config Debug --target TestAntQtExtensions
for /L %i in (1,1,5) do "build/tests/Debug/TestAntQtExtensions.exe" dockManager
```

Result: the `dockManager` subtest passed `5 / 5` runs on `2026-05-19` with two Win10-only fixes against the float→re-embed stutter:

1. `AntDockWidget::resetNativeFloatingWindowForEmbedding()` deletes `m_legacySoftwareShadow` outright on embed so no hidden top-level `WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE` HWND remains enrolled in DWM tracking, and clears the live-resize / DWM-frame / queued-frame-refresh property set so already-queued callbacks short-circuit.
2. `queueDwmFrameRefresh()` timer guards: it now requires the widget to still be a window, the dock to still report `isFloating()`, and an existing `internalWinId()` (never calls `winId()`, which would force-create a new native child HWND on the embedded dock). Any other state skips the DWM re-extension.

`PaintProbeWidget` was extended to record per-frame intervals; the dock re-embed scenario captures a baseline 12-frame / 16 ms-per-frame `pageProbe.setFillColor` cadence on the fresh host, floats then re-embeds, repeats the cadence, and asserts the post-embed max frame interval ≤ max(80 ms, 3 × baseline) plus zero residual native handles across `windowExplorer->findChildren<QWidget*>()` and zero `antDockNativeWindowFrameEnabled` / `antDockUsesNativeCaptionFrame` / `antDockLegacyLiveResize` flags. The new max-interval guard catches a stutter regression even when every paint is delivered (the previous test only checked `paintCount()` growth).

Latest targeted DockWidget floating-window validation:

```powershell
cmake --build build --config Debug --target TestAntQtExtensions
ctest --test-dir build -C Debug -R "^TestAntQtExtensions$" --output-on-failure
cmake --build build --config Debug --target qt-ant-design-example
```

Result: the `dockManager` targeted test passed and the example Debug build succeeded on `2026-05-19`; the test now also covers an `AntWindow` hosted DockWidget page where a floating dock is embedded back into the layout, the embedded dock and manager-owned guide/preview overlays stay non-native on the Windows 10 legacy-frame path, a child repaint pixel check remains visible in the host grab, and Windows `SendInput` can still toggle the page `AntSwitch` after `WindowFromPoint()` skips the native corner smoother and click-wave overlays. The same target now validates DockWidget dark-mode surfaces: embedded docks avoid native light title/frame painting, content widgets inherit token palettes with `autoFillBackground`, DockArea tabs/panes expose container and elevated dark surfaces, and rendered manager output avoids stale light layout pixels in the body. It also validates that an already-floating dock can still be dragged when its manager surface is hidden, without showing a drop preview or active guide, and that hidden docking surfaces reject release-time drop targets so floating docks keep floating instead of embedding into invisible layouts. It also validates that programmatic removal of a floating dock hides the orphan window while clearing manager ownership, that context-menu close actions respect each dock tab's closable feature, that disabling floatable does not block a floating dock from returning to the workspace through its context menu, that directly floating a detached dock registers it with the manager, that preconfigured dock features are preserved when a dock is first managed, that direct close events on floating docks either respect closable=false or remove the dock from manager state, that manager destruction hides any still-managed floating dock while clearing owner state, that removing then re-adding the same dock instance does not leave duplicate manager-owned top-level or tab-area title sync connections, that the floating title-bar close button follows dynamic closable feature changes, that direct title-bar double click respects DockWidgetFloatable, that disabling movable during an active drag through either manager API or direct setFeatures cancels previews and guide state while forwarding manager feature-change signals, that the drop guide overlay is a visible manager child overlay with painted guide-square pixels above the drop preview, that embedded guided right/center drops apply the new dock tree before the release event returns, and that perspective restore keeps still-embedded docks out of the temporary top-level/native reset path while avoiding redundant DockManager stylesheet reapplies.

Latest targeted `image-basic` asset validation:

```powershell
cmake --build build --config Debug --target TestAntDataDisplayA TestAntRenderSmoke qt-ant-design-example
ctest --test-dir build -C Debug -R "TestAnt(DataDisplayA|RenderSmoke)$" --output-on-failure
.\build\examples\Debug\qt-ant-design-example.exe --smoke-exit-ms 800
cmake --build build\visual-capture-build --config Debug --target readme_dark_capture
.\build\visual-capture-build\Debug\readme_dark_capture.exe resources\images\components
```

Result: `2 / 2` targeted tests passed, the example Debug build succeeded, the example smoke launch exited cleanly, and the `AntAvatar` / `AntImage` README light/dark thumbnails were regenerated at `960x540` on `2026-05-17`.

Latest targeted QRCode editable example validation:

```powershell
cmake --build build --config Debug --target TestAntDataDisplayB TestAntRenderSmoke qt-ant-design-example
ctest --test-dir build -C Debug -R "TestAnt(DataDisplayB|RenderSmoke)$" --output-on-failure
.\build\examples\Debug\qt-ant-design-example.exe --smoke-exit-ms 800
cmake --build build\visual-capture-build --config Debug --target readme_dark_capture
.\build\visual-capture-build\Debug\readme_dark_capture.exe resources\images\components
```

Result: `2 / 2` targeted tests passed, the example Debug build succeeded, the example smoke launch exited cleanly, and the README `AntQRCode` light/dark thumbnails were regenerated at `960x540` on `2026-05-17`.

Latest targeted passive-overlay hit-test validation:

```powershell
cmake --build build --config Debug --target TestAntAdvancedInteractions TestAntDataEntryA TestAntDataDisplayB TestAntFeedback TestAntPopupLifecycle
ctest --test-dir build -C Debug -R "TestAnt(AdvancedInteractions|DataEntryA|DataDisplayB|Feedback|PopupLifecycle)$" --output-on-failure
cmake --build build --config Debug --target qt-ant-design-example
.\build\examples\Debug\qt-ant-design-example.exe --smoke-exit-ms 800
```

Result: `5 / 5` targeted tests passed, the example Debug build succeeded, and the example smoke launch exited cleanly on `2026-05-15`.

## Visual Audit State

The component visual audit matrix in `docs/visual-audit.md` is current:

- Comparable Ant Design standard components are marked `Pass`.
- Qt-only desktop extensions are marked `Local Pass`.
- `TestAntVisualRegression` now guards stable pixel-level regressions across token fills, semantic status colors, selection controls, feedback surfaces, data-display structure, navigation/layout structure, popup surfaces, shared external popup shadows, clipped popup-shadow edges, and light/dark surface contrast.
- The homepage Showcase audit is marked `Pass` against the isolated local HTML and Qt control pages.
- README gallery screenshots are committed under `resources/images/components/`; popup and feedback controls use representative open/active captures while single-component audit scratch captures remain under `build/`.
- Future visual work should be issue-driven: when a mismatch is found, re-run the single-component capture loop documented in `docs/visual-audit.md`.

## Icon State

`AntIcon` now supports:

- Existing enum API, for example `AntIcon(Ant::IconType::Search)`.
- Official string names, for example `AntIcon(QStringLiteral("GithubFilled"))`.
- Full icon inventory lookup through `AntIcon::builtinIconNames()`.
- Official resource rendering through QtSvg from `:/qt-ant-design/icons/antd/*.svg`.

Inventory:

- Total: `831`
- Outlined: `447`
- Filled: `234`
- TwoTone: `150`

## Verification

Last full verification run:

```powershell
cmake --build build --config Debug
ctest -C Debug --output-on-failure
```

Configured tests after alias, build-system, and example subsystem guards: `37`.

Latest full component reliability validation:

```powershell
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
cmake --build build --config Debug --target qt-ant-design-example
```

Result: `37 / 37` CTest targets passed and `qt-ant-design-example` Debug build succeeded on `2026-05-10`, including public component API / getter-setter / signal coverage, real mouse/keyboard/popup interactions, lifecycle ownership, theme lifecycle, render smoke, visual regression guards, install consumer, build-system, and example subsystem checks.

Latest build-system / install targeted validation:

```powershell
ctest --test-dir build -C Debug -R "TestAntBuildSystem|TestAntInstallConsumer|TestAntIcon|TestAntAliases" --output-on-failure
```

Result: `4 / 4` targeted tests passed.

Latest targeted popup shadow validation:

```powershell
cmake --build build --config Debug --target TestAntSelect TestAntDataEntryB TestAntNavigation TestAntQtExtensions TestAntFeedback TestAntVisualRegression TestAntDataDisplayA qt-ant-design-example
ctest --test-dir build -C Debug -R "TestAnt(Select|DataEntryB|Navigation|QtExtensions|Feedback|VisualRegression|DataDisplayA)$" --output-on-failure
.\build\examples\Debug\qt-ant-design-example.exe --smoke-exit-ms 800
```

Result: `7 / 7` targeted tests passed, the example Debug build succeeded, and the example smoke launch exited cleanly on `2026-05-09`.

Latest targeted AntWindow verification:

```powershell
cmake --build build --config Debug --target TestAntQtExtensions qt-ant-design-example
ctest --test-dir build -C Debug -R "TestAntQtExtensions$" --output-on-failure
.\build\examples\Debug\qt-ant-design-example.exe --smoke-exit-ms 800
```

Result: `1 / 1` targeted test passed and the example Debug build succeeded on `2026-05-19`, including the Windows 10 no-caption software shadow host, 14px lightweight Win11-like shadow margin, zero inner clear band, capped corner alpha, four-sided shadow pixel coverage, resize-following and live-move-following shadow geometry, DPI-safe Qt-logical shadow geometry with native physical HWND size validation, maximized `WM_NCCALCSIZE`, visible topmost toggles without hide/show recreation, inner/outer `WM_NCHITTEST` resize bands for all four edges and corners, child-HWND hit-test forwarding from the content widget and any descendant native HWND when present, a worker-thread `SendInput` drag that verifies the real right edge grows the window width, the Win10 corner smoother staying non-native so it does not cover/freeze child repaint composition, a forced legacy-frame child repaint pixel check, a forced-legacy floating DockWidget software shadow with DPI-safe Qt-logical geometry, native physical HWND size validation, and ring-region coverage, plus an AntWindow-hosted DockWidget float/re-embed scenario that keeps the embedded dock plus guide/preview overlays non-native, verifies a post-embed child repaint pixel, and checks right, left, and bottom edge drags still resize the host.

Latest targeted AntList / AntTable / AntTypography API validation:

```powershell
ctest -C Debug -R "TestAnt(DataDisplayB|Typography)$" --output-on-failure
```

Result: `2 / 2` targeted tests passed on `2026-05-08`.

Latest targeted AntTypography enabled-state validation:

```powershell
ctest --test-dir build -C Debug -R "TestAntTypography$" --output-on-failure
```

Result: `1 / 1` targeted test passed on `2026-05-08`.

Latest targeted AntSelect API validation:

```powershell
ctest -C Debug -R "TestAntSelect$" --output-on-failure
```

Result: `1 / 1` targeted test passed on `2026-05-08`.

Latest targeted Qt layout adaptivity validation:

```powershell
ctest --test-dir build -C Debug -R "TestAnt(RenderSmoke|Select|DataEntryA|Typography)$" --output-on-failure
```

Result: `4 / 4` targeted tests passed on `2026-05-08`.

Latest targeted AntInputNumber decimal validation:

```powershell
ctest --test-dir build -C Debug -R "TestAntDataEntryA$" --output-on-failure
```

Result: `1 / 1` targeted test passed on `2026-05-08`.

Latest targeted AntSlider layout / bubble / range validation:

```powershell
cmake --build build --config Debug --target TestAntDataEntryA qt-ant-design-example
ctest --test-dir build -C Debug -R "TestAntDataEntryA$" --output-on-failure
.\build\examples\Debug\qt-ant-design-example.exe --smoke-exit-ms 800
```

Result: `1 / 1` targeted test passed, the example Debug build succeeded, and the example smoke launch exited cleanly on `2026-05-09`.

Latest targeted AntTabs content layout / AntTypography multiline validation:

```powershell
ctest --test-dir build -C Debug -R "TestAnt(Navigation|Typography)$" --output-on-failure
```

Result: `2 / 2` targeted tests passed on `2026-05-08`.

Latest targeted AntCard theme validation:

```powershell
ctest -C Debug -R "TestAntDataDisplayA$" --output-on-failure
```

Result: `1 / 1` targeted test passed on `2026-05-08`.

Latest targeted one-call initialization validation:

```powershell
cmake --build build --config Debug --target qt-ant-design-example
ctest --test-dir build -C Debug -R "TestAntInstallConsumer$" --output-on-failure
```

Result: example Debug build succeeded and `1 / 1` targeted install-consumer test passed on `2026-05-08`.

Latest targeted AntRibbon validation:

```powershell
cmake --build build --config Debug --target TestAntQtExtensions TestAntRenderSmoke qt-ant-design-example
ctest --test-dir build -C Debug -R "TestAnt(QtExtensions|RenderSmoke)$" --output-on-failure
cmake --build build --config Debug --target qt-ant-design-example
.\build\examples\Debug\qt-ant-design-example.exe --smoke-exit-ms 800
```

Result: `2 / 2` targeted tests passed, example Debug build succeeded, the example smoke launch exited cleanly, and temporary visual captures for expanded/hover-tab/tab-animation/collapse/collapsed Ribbon states were checked on `2026-05-08`.

Latest README component gallery validation:

```powershell
# Validate every committed gallery PNG is present and 960x540.
Add-Type -AssemblyName System.Drawing
# Validate every README / README.zh-CN image reference resolves.
Select-String -Path README.md,README.zh-CN.md -Pattern 'resources/images/components/[^" ]+\.png' -AllMatches
```

Result: `166` committed PNG screenshots validated at `960x540`, and `166` unique README image references resolved on `2026-05-08`.

Latest targeted README dark thumbnail refresh validation:

```powershell
cmake --build build --config Debug --target TestAntFeedback qt-ant-design-example
ctest --test-dir build -C Debug -R "TestAntFeedback$" --output-on-failure
.\build\examples\Debug\qt-ant-design-example.exe --smoke-exit-ms 800
```

Result: `AntResult`, `AntSpin`, and `AntTour` dark thumbnails were regenerated at `960x540`, `1 / 1` targeted feedback test passed, the example Debug build succeeded, and the example smoke launch exited cleanly on `2026-05-17`.

Latest targeted Calendar / Anchor / Splitter / Layout / ScrollBar dark thumbnail validation:

```powershell
cmake --build build --config Debug --target TestAntDataDisplayA TestAntNavigation TestAntLayout TestAntQtExtensions TestAntVisualRegression qt-ant-design-example
ctest --test-dir build -C Debug -R "TestAnt(DataDisplayA|Navigation|Layout|VisualRegression)$" --output-on-failure
.\build\tests\Debug\TestAntQtExtensions.exe scrollBar splitter
.\build\examples\Debug\qt-ant-design-example.exe --smoke-exit-ms 800
```

Result: the five dark thumbnails were regenerated at `960x540`; `TestAntDataDisplayA`, `TestAntNavigation`, `TestAntLayout`, targeted `TestAntQtExtensions::scrollBar/splitter`, and `TestAntVisualRegression` passed, the example Debug build succeeded, and the example smoke launch exited cleanly on `2026-05-17`.

Latest targeted AntTour / AntResult issue validation:

```powershell
cmake --build build --config Debug --target TestAntFeedback TestAntVisualRegression qt-ant-design-example
ctest --test-dir build -C Debug -R "TestAnt(Feedback|VisualRegression)$" --output-on-failure
.\build\examples\Debug\qt-ant-design-example.exe --smoke-exit-ms 800
```

Result: `2 / 2` targeted tests passed, the example Debug build succeeded, and the example smoke launch exited cleanly on `2026-05-08`.

Latest targeted AntNotification loading-progress example validation:

```powershell
cmake --build build --config Debug --target TestAntFeedback qt-ant-design-example
ctest --test-dir build -C Debug -R "TestAntFeedback$" --output-on-failure
.\build\examples\Debug\qt-ant-design-example.exe --smoke-exit-ms 800
```

Result: `1 / 1` targeted test passed, the example Debug build succeeded, and the example smoke launch exited cleanly on `2026-05-09`.

Latest targeted AntSteps edge-clip validation:

```powershell
cmake --build build --config Debug --target TestAntNavigation TestAntVisualRegression qt-ant-design-example
ctest --test-dir build -C Debug -R "TestAnt(Navigation|VisualRegression)$" --output-on-failure
.\build\examples\Debug\qt-ant-design-example.exe --smoke-exit-ms 800
```

Result: `2 / 2` targeted tests passed, the example Debug build succeeded, and the example smoke launch exited cleanly on `2026-05-09`.

Latest targeted AntColorPicker border validation:

```powershell
cmake --build build --config Debug --target TestAntQtExtensions TestAntVisualRegression qt-ant-design-example
ctest --test-dir build -C Debug -R "TestAnt(QtExtensions|VisualRegression)$" --output-on-failure
.\build\examples\Debug\qt-ant-design-example.exe --smoke-exit-ms 800
```

Result: `2 / 2` targeted tests passed, the example Debug build succeeded, and the example smoke launch exited cleanly on `2026-05-09`.

Latest targeted AntRadio ButtonStyle wave validation:

```powershell
cmake --build build --config Debug --target TestAntMotion TestAntDataEntryA qt-ant-design-example
ctest --test-dir build -C Debug -R "TestAnt(Motion|DataEntryA)$" --output-on-failure
.\build\examples\Debug\qt-ant-design-example.exe --smoke-exit-ms 800
```

Result: `2 / 2` targeted tests passed, the example Debug build succeeded, and the example smoke launch exited cleanly on `2026-05-09`.

Latest targeted AntRate selected-star animation validation:

```powershell
cmake --build build --config Debug --target TestAntDataEntryA qt-ant-design-example
ctest --test-dir build -C Debug -R "TestAntDataEntryA$" --output-on-failure
.\build\examples\Debug\qt-ant-design-example.exe --smoke-exit-ms 800
```

Result: `1 / 1` targeted test passed, the example Debug build succeeded, and the example smoke launch exited cleanly on `2026-05-09`.

## Remaining Notes

- `build/` contains temporary capture helpers and screenshots only; keep those untracked.
- README gallery image assets live in `resources/images/components/` and are intentionally tracked.
- `docs/ant-design-reference.html` remains the stable component comparison reference.
- The official `https://ant.design/index-cn` homepage was useful for the Showcase target, but it produced hydration/resource errors during capture on `2026-04-29`; repeatable audits use the local HTML reference instead.
- Exact pixel parity is not guaranteed forever; component fixes should continue to use the documented screenshot comparison loop.
