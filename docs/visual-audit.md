# Visual Audit Checklist

This file tracks the next phase: comparing each Qt widget page against the Ant Design reference page in `docs/ant-design-reference.html`.

## Workflow

Use this loop for one component at a time.

1. Confirm the component row in the matrix and inspect the Ant Design source under `submodules/ant-design/components/<component>/`.
2. Build the Qt example and target tests before taking screenshots:
   ```powershell
   cmake --build build --config Debug --target TestAntButton qt-ant-design-example -- /m:1
   ```
   Replace `TestAntButton` with the closest test target for the component under review.
3. Capture the Ant Design reference page with Playwright:
   ```powershell
   npx playwright screenshot --wait-for-timeout=4000 --viewport-size "1280,900" "file:///D:/Project/GitProject/qt-ant-design/docs/ant-design-reference.html" build/<component>-reference-full.png
   ```
4. Capture the matching Qt example page to `build/<component>-qt.png`. Prefer a small temporary capture helper under `build/visual-capture/` so the screenshot contains only the audited page/content. Do not commit the capture helper or PNG outputs.
5. Create a side-by-side image such as `build/<component>-compare-wide.png` and compare both screenshots at the same scale.
6. Attribute each visible difference before editing:
   - `Component`: button/control body, text/icon, state color, border, radius, shadow, size, padding.
   - `Container`: card/body padding, page margins, section title, scroll area, sidebar, or surrounding layout.
   - `Reference/demo gap`: a state exists in AntD but is missing from the Qt example page.
7. Fix only component-owned differences in the audited component's widget/style/test files. Leave container differences for that component's own audit row.
8. Rebuild and run tests:
   ```powershell
   cmake --build build --config Debug --target TestAntButton qt-ant-design-example -- /m:1
   ctest -C Debug --output-on-failure
   ```
9. Update the matrix status and notes:
   - `Pass`: screenshot compared and no component-owned mismatches remain.
   - `Needs visual QA`: states are present, but screenshot review is still pending or inconclusive.
   - `Needs fix`: component-owned mismatch remains.
   - `Blocked`: cannot compare because reference/demo/capture is missing.
   - `N/A`: Qt-only extension or API wrapper without an official AntD visual reference.

### Capture Notes

- Playwright expects Chromium headless shell at `%LOCALAPPDATA%\ms-playwright\chromium_headless_shell-1217\chrome-headless-shell-win64\chrome-headless-shell.exe`.
- If Playwright cannot download the browser, manually download `chrome-headless-shell-win64.zip` for version `147.0.7727.15` and extract it to `%LOCALAPPDATA%\ms-playwright\chromium_headless_shell-1217\`.
- Qt `offscreen` capture may render text as square glyphs on Windows. Use the native Windows platform for capture when this happens.
- Temporary Qt capture programs may hit the known QProxyStyle destructor crash on process teardown. Save the PNG first, then exit the helper process immediately if needed.
- Build artifacts and screenshots belong under `build/` and should remain untracked.

## Shared Criteria

| Area | What to Check |
| --- | --- |
| Tokens | Primary color, text hierarchy, border color, fill color, disabled color, shadow, radius |
| Geometry | Control height, padding, icon/text spacing, popup width, table/list density |
| Interaction | Hover/press/focus/open animation, keyboard behavior, popup dismissal |
| Theme | Light/dark parity, contrast, inherited text colors, transparent surfaces |
| Example | Uses Ant components for visible text, no stale "missing" notes, no stylesheet-only styling |

## Second-Pass State Audit

After the first static pass, audit interactive states with a compact state matrix for one component at a time. Capture both light and dark mode where the component has token-driven state colors.

| Component | Status | Evidence | Notes |
| --- | --- | --- | --- |
| Button | Pass | `build/button-state-compare-light.png`, `build/button-state-compare-dark.png` | Compared primary/default hover, active, focus-visible, danger hover, text hover, disabled, and loading states. Ant theme primary/status tokens now match AntD 5.24.7 light/dark values, and `AntButtonStyle` uses explicit token hover/active/focus colors instead of approximate palette generation. |
| Tag | Pass | `build/tag-state-compare-light.png`, `build/tag-state-compare-dark.png` | Compared default, closable/close-hover, preset/status color tags, checkable unchecked/hover/active/checked/checked-hover, disabled unchecked/checked, and processing states. `AntTag` now has a pressed state, defers click/toggle to release, uses AntD checkable hover/active fills, disabled checked fill, and independent close-icon normal/hover colors. |

## Priority Order

1. Foundations: Typography, Icon, Button, Tag, Badge
2. Inputs: Input, InputNumber, Select, DatePicker, TimePicker, Checkbox, Radio, Switch, Slider
3. Popups and feedback: Tooltip, Popover, Popconfirm, Dropdown, Message, Notification, Modal, Drawer
4. Data display: Card, List, Table, Tree, Descriptions, Empty, Statistic
5. Navigation and layout: Menu, Tabs, Pagination, Steps, Breadcrumb, Layout, Space, Flex, Grid
6. Complex and extension widgets: Upload, Transfer, Tour, ColorPicker, Calendar, Window, DockWidget

## Component Matrix

| Category | Component | Reference Page | Qt Example Page | Status | Notes |
| --- | --- | --- | --- | --- | --- |
| General | Button | ButtonPage | Button | Pass | Static screenshot compared against AntD reference; padding, transparent solid border, loading opacity/cursor, spinner arc, ghost, disabled, and subtle bottom shadow are aligned. Container card spacing differs and should be handled with Card. |
| General | Icon | IconPage | Icon | Pass | Static screenshot compared against AntD reference; IconPage now covers the 16 official outlined icons, and AntIcon renders those glyphs from the official SVG paths via QtSvg. Extra theme/spin/custom examples remain Qt-only coverage below the reference section; container card spacing belongs to Card. |
| General | Typography | TypographyPage | Typography | Pass | Static screenshot compared against AntD reference; title levels, text types, decorations, paragraph text, default link styling, mark/code backgrounds, and copyable icon affordance are aligned. Container card spacing belongs to Card. |
| Layout | Divider | DividerPage | Divider | Pass | Horizontal, With Title, and Dashed & Vertical demos compared against `build/divider-compare-wide.png`; official demo labels, 1px split line, dashed/vertical variants, title typography, orientation rails, and title spacing aligned. Shared Card chrome remains out of scope. |
| Layout | Flex | FlexPage | Flex | Pass | Horizontal, Vertical, and Wrap demos compared against `build/flex-compare-wide.png`; official demo labels, middle/small/custom gaps, horizontal intrinsic child sizing, vertical cross-axis stretch, and wrapping row flow aligned. Shared Card chrome remains out of scope. |
| Layout | Grid | GridPage | Grid | Pass | 24-Column Grid and Offset demos compared against `build/grid-compare-wide.png`; official reference now uses `antd.Row`/`antd.Col`, demo labels/colors, 8px gutter, 6/8/16 spans, and 8-column offset aligned. Shared Card body height remains out of scope. |
| Layout | Layout | LayoutPage | Layout | Pass | Basic Layout and With Sider demos compared against `build/layout-compare-wide.png`; official dark header/sider colors, 64px header, 160px sider, footer/content backgrounds, nav padding, and nested header/content structure aligned. Shared Card body spacing and rounded clipping remain out of scope. |
| Layout | Space | SpacePage | Space | Pass | Horizontal, Vertical, and Size demos compared against `build/space-compare-wide.png`; official demo labels, default/small/middle/large gaps, vertical direction, and intrinsic inline child sizing aligned. Shared Card body spacing remains out of scope. |
| Layout | Splitter | SplitterPage | Splitter | Pass | Basic Splitter compared against `build/splitter-compare-wide.png`; official demo title, 200px split area, 50/50 panels, panel tints, centered labels, 4px divider, and border-color handle aligned. Shared Card body vertical spacing remains out of scope. |
| Navigation | Affix | AffixPage | Affix | Pass | Static Affix demo compared against `build/affix-compare-wide.png`; official demo title, primary child button label, offsetTop=10 binding, and secondary instruction text aligned. Shared Card body vertical spacing remains out of scope. |
| Navigation | Anchor | AnchorPage | Anchor | Pass | Basic Anchor compared against `build/anchor-compare-wide.png`; official Part 1/2/3 links, default text color, left split rail, and three 100px tinted content blocks aligned. Shared Card body vertical spacing remains out of scope. |
| Navigation | Breadcrumb | BreadcrumbPage | Breadcrumb | Pass | Basic and With Separator demos compared against `build/breadcrumb-compare-wide.png`; official item labels, custom `>` separator, muted link color, current text color, and separator spacing aligned. Shared Card body height remains out of scope. |
| Navigation | Dropdown | DropdownPage | Dropdown | Pass | Basic trigger row compared against `build/dropdown-compare-wide.png`; official trigger labels, primary context-menu button, trailing down chevron, and target ordering aligned. Popup menu density still uses shared AntMenu rendering. |
| Navigation | Menu | MenuPage | Menu | Pass | Horizontal and Vertical / Inline demos compared against `build/menu-compare-wide.png`; official item labels, selected keys, open inline submenu, vector icons where available, horizontal underline without selected fill, and inline selected indicator aligned. |
| Navigation | Pagination | PaginationPage | Pagination | Pass | Basic, More Options, and Simple demos compared against `build/pagination-compare-wide.png`; official totals text, quick jumper input shape, size changer placement, simple current-page input, and page item sequence aligned. |
| Navigation | Steps | StepsPage | Steps | Pass | Basic, With Description, and Error demos compared against `build/steps-compare-wide.png`; official labels, descriptions, finish/process/wait/error colors, check/error glyphs, and horizontal/vertical states aligned. Vertical small-size compactness remains out of scope. |
| Data Entry | AutoComplete | AutoCompletePage | AutoComplete | Pass | Basic AutoComplete compared against `build/autocomplete-compare-wide.png`; official demo title, 280px input width, placeholder text, default border/radius, and closed empty state aligned. Popup suggestion filtering remains for interactive QA. |
| Data Entry | Cascader | CascaderPage | Cascader | Pass | Basic Cascader compared against `build/cascader-compare-wide.png`; official demo title, 280px width, placeholder, default border/radius, arrow position, and option tree data aligned. Popup column interaction remains for interactive QA. |
| Data Entry | Checkbox | CheckboxPage | Checkbox | Pass | Example aligned to official Basic and Group demos; checked/unchecked box geometry, label spacing, primary check mark, and group spacing verified against `build/checkbox-compare-wide.png`. |
| Data Entry | ColorPicker | ColorPickerPage | ColorPicker | Pass | Inline trigger added and example aligned to official Basic and With Text demos; 32px trigger, 22px swatch, `#52C41A` showText label, colors, and spacing verified against `build/colorpicker-compare-wide.png`. Popup editor remains available via click/static `getColor()`. |
| Data Entry | DatePicker | DatePickerPage | DatePicker | Pass | Basic and Sizes closed states compared against `build/datepicker-compare-wide.png`; official empty DatePicker/RangePicker placeholders, control widths, large/middle/small heights, Range separator, and calendar icon weight aligned. Popup grid and range interaction remain for interactive QA. |
| Data Entry | Form | FormPage | Form | Pass | Basic horizontal form compared against `build/form-compare-wide.png`; official Basic Form fields, 480px width, 6/18 label-wrapper rhythm, required marks, right-aligned labels, item spacing, Remember checkbox row, and submit offset aligned. Password icon rendering remains Input scope. |
| Data Entry | Input | InputPage | Input | Pass | Basic, Sizes, Search & Password, TextArea, and With Addon demos compared against `build/input-compare-wide.png`; placeholders, disabled state, size heights, search suffix button, password eye icon, TextArea grip/placeholder, and addon segment rounding aligned. TextArea visual is represented with `AntPlainTextEdit`; a dedicated `Input.TextArea` API alias remains optional API polish. |
| Data Entry | InputNumber | InputNumberPage | InputNumber | Pass | Basic and With Prefix/Suffix demos compared against `build/inputnumber-compare-wide.png`; official values, compact width, hidden static handlers, `$ ` prefix, and addonAfter `%` segment aligned. Hover handlers remain interactive QA. |
| Data Entry | Mentions | MentionsPage | Mentions | Pass | Basic multiline Mentions compared against `build/mentions-compare-wide.png`; 400px rows=3 box, placeholder text/color, border/radius, and top padding aligned. Popup filtering remains interactive QA. |
| Data Entry | Radio | RadioPage | Radio | Pass | Basic and Button Style demos compared against `build/radio-compare-wide.png`; radio indicator state, label spacing, solid selected Radio.Button segment, grouped borders, and labels aligned. |
| Data Entry | Rate | RatePage | Rate | Pass | Basic and Half & Disabled demos compared against `build/rate-compare-wide.png`; star count, active/disabled colors, half-star state, and row spacing aligned. |
| Data Entry | Select | SelectPage | Select | Pass | Basic, Multiple, and Sizes demos compared against `build/select-compare-wide.png`; official values, 160/280/120px widths, single/multiple rendering, arrows, and large/middle/small heights aligned. Popup density remains interactive QA. |
| Data Entry | Slider | SliderPage | Slider | Pass | Basic, Range, and With Step demos compared against `build/slider-compare-wide.png`; 400px rail, range handles/track, mark dots, mark labels, and active rail color aligned. |
| Data Entry | Switch | SwitchPage | Switch | Pass | Basic, With Text, and Disabled demos compared against `build/switch-compare-wide.png`; checked/unchecked track colors, handle position, text labels, and disabled checked state aligned. |
| Data Entry | TimePicker | TimePickerPage | TimePicker | Pass | Basic TimePicker/RangePicker closed states compared against `build/timepicker-compare-wide.png`; placeholders, range separator, widths, clock icon, and empty range text aligned. Popup columns and footer controls remain interactive QA. |
| Data Entry | Transfer | TransferPage | Transfer | Pass | Basic Transfer compared against `build/transfer-compare-wide.png`; official item set, two-panel layout, header checkbox/count, item checkboxes, disabled arrow buttons, empty target text, and scrollbar aligned. |
| Data Entry | TreeSelect | TreeSelectPage | TreeSelect | Pass | Basic TreeSelect compared against `build/treeselect-compare-wide.png`; 240px trigger, placeholder, border/radius, arrow placement, and option tree data aligned. Popup tree interaction remains interactive QA. |
| Data Entry | Upload | UploadPage | Upload | Pass | Default and Dragger demos compared against `build/upload-compare-wide.png`; default upload button, trigger text, dragger border/fill, upload icon, and dragger instruction aligned. File list states remain interactive QA. |
| Data Display | Avatar | AvatarPage | Avatar | Pass | Basic, Type, and Group demos compared against `build/avatar-compare-wide.png`; official size sequence, custom background colors, user/bell-style avatar icons, image avatar, and `Avatar.Group` overflow `+N` aligned. Shared Card body height remains out of scope. |
| Data Display | Badge | BadgePage | Badge | Pass | Basic and Status demos compared against `build/badge-compare-wide.png`; count pill placement, dot overlay, standalone status dot colors, processing pulse, and compact badge content reserve aligned. Shared Card body height remains out of scope. |
| Data Display | Calendar | CalendarPage | Calendar | Pass | Basic `fullscreen={false}` calendar compared against `build/calendar-compare-wide.png`; demo title, right-aligned year/month controls, Month/Year switch, English month label, adaptive seven-column grid, muted adjacent dates, compact selected-day square, and border-only panel aligned closer to the AntD reference. Shared Card body height remains out of scope. |
| Data Display | Card | CardPage | Card | Pass | Basic card and cover/meta/actions card compared against `build/card-compare-wide.png`; component-owned gaps fixed for extra color, Meta padding, action centering, and action separators. Shared demo-container spacing remains out of scope. |
| Data Display | Carousel | CarouselPage | Carousel | Pass | Basic carousel compared against `build/carousel-compare-wide.png`; 160px gradient slide, centered white label, rounded viewport, and AntD-style pill dots are aligned. The generated reference snapshot uses a static equivalent because the source AntD Carousel collapses under file:// Playwright capture. |
| Data Display | Collapse | CollapsePage | Collapse | Pass | Basic and Accordion sections compared against `build/collapse-compare-wide.png`; default non-accordion behavior, expanded first panel, rounded border, header background, content padding, separators, and arrow placement aligned. Shared demo-container spacing remains out of scope. |
| Data Display | Descriptions | DescriptionsPage | Descriptions | Pass | Basic bordered two-column table compared against `build/descriptions-compare-wide.png`; demo coverage, item padding, vertical sizing, label background, and processing Badge content aligned. Shared demo-container spacing remains out of scope. |
| Data Display | Empty | EmptyPage | Empty | Pass | Basic and Custom Description compared against `build/empty-compare-wide.png`; default description casing, official demo coverage, default illustration structure, image sizing, description color, and image/description spacing aligned. Shared demo-container spacing remains out of scope. |
| Data Display | Image | ImagePage | Image | Pass | Basic image compared against `build/image-compare-wide.png`; demo now uses the official sample image, loaded images render without placeholder chrome, width-only sizing preserves aspect ratio, and hover preview mask uses an AntD-like eye/text treatment. Shared demo-container spacing remains out of scope. |
| Data Display | List | ListPage | List | Pass | Basic and With Actions compared against `build/list-compare-wide.png`; demo coverage, bordered row padding, row height, split lines, meta typography, description sizing, and inline action placement aligned. Shared Card body spacing remains out of scope. |
| Data Display | Popover | PopoverPage | Popover | Pass | Basic hover/click trigger demo and opened click popover compared against `build/popover-compare-wide.png`; demo labels, title/body typography, min width, padding, shadow, arrow gap, and placement aligned. Shared Card body height remains out of scope. |
| Data Display | QRCode | QRCodePage | QRCode | Pass | Basic QRCode compared against `build/qrcode-compare-wide.png`; matrix generation now preserves finder/timing modules, modules render as solid cells without grid gaps, and bordered 160px QR surface aligns visually. Exact data mask may differ from AntD's renderer. Shared Card body height remains out of scope. |
| Data Display | Segmented | SegmentedPage | Segmented | Pass | Basic and With Icons demos compared against `build/segmented-compare-wide.png`; official options, intrinsic non-block width, selected thumb, separators, text colors, and simple app/settings icon rendering aligned. Shared Card body height remains out of scope. |
| Data Display | Statistic | StatisticPage | Statistic | Pass | Basic and Countdown demos compared against `build/statistic-compare-wide.png`; official title/value labels, prefix/precision formatting, normal 24px value weight, and total-hour countdown format aligned. Shared Card body height remains out of scope. |
| Data Display | Table | TablePage | Table | Pass | Basic table compared against `build/table-compare-wide.png`; official data set, Basic demo title, header/body row rhythm, default unbordered body, row dividers, column sizing, and sorter icon placement aligned. Shared Card body height remains out of scope. |
| Data Display | Tabs | TabsPage | Tabs | Pass | Line and Card demos compared against `build/tabs-compare-wide.png`; AntTabs now paints its own tab bar instead of being swallowed by the style event filter, and official labels/content, active underline, card tabs, baseline, and compact height are aligned. Shared Card body height remains out of scope. |
| Data Display | Tag | TagPage | Tag | Pass | Basic and Checkable & Closable demos compared against `build/tag-compare-wide.png`; preset tint/border colors, checked tag fill, closable affordance, processing/success tags, and fixed tag height aligned. Shared Card body height remains out of scope. |
| Data Display | Timeline | TimelinePage | Timeline | Pass | Basic Timeline compared against `build/timeline-compare-wide.png`; official step text, blue/green/red dot colors, normal-weight labels, expanded text width, and compact item height aligned. Shared Card body height remains out of scope. |
| Data Display | Tooltip | TooltipPage | Tooltip | Pass | Static Basic trigger row compared against `build/tooltip-compare-wide.png`; official Top/Bottom/Left/Right labels and default button trigger visuals aligned. Hover bubble placement/color/arrow remain available for interactive QA because the reference static page does not render an open tooltip. |
| Data Display | Tree | TreePage | Tree | Pass | Basic Tree compared against `build/tree-compare-wide.png`; official default-expanded hierarchy, no checkbox/no icon/no showLine state, 28px row rhythm, indentation, and filled expand arrows aligned. Shared Card body height remains out of scope. |
| Feedback | Alert | AlertPage | Alert | Pass | Types and Banner demos compared against `build/alert-compare-wide.png`; official labels, showIcon/closable states, status fill/border colors, close affordance, banner text, no-radius/no-border banner style, and fixed vertical sizing aligned. Shared Card body height remains out of scope. |
| Feedback | Drawer | DrawerPage | Drawer | Pass | Open Basic drawer compared against `build/drawer-compare-wide.png`; official demo labels/body text, 378px right placement, start-side close affordance, header/body padding, mask opacity, and outer edge shadow aligned. Shared Card body height remains out of scope. |
| Feedback | Message | MessagePage | Message | Pass | Open Success/Info/Warning/Error messages compared against `build/message-compare-wide.png`; official demo buttons/text, 8px top container offset, 56px stacked rhythm, content-sized white bubbles, filled status icons, no-border surface, and AntD-like shadow aligned. Loading/alternate placements remain available for interactive QA. |
| Feedback | Modal | ModalPage | Modal | Pass | Basic and Confirm demos compared against `build/modal-compare-basic-wide.png` and `build/modal-compare-confirm-wide.png`; official labels/content, 520px basic width, 416px command width, 100px visible top offset, no command close affordance, main text color, footer spacing, mask opacity, and capture path aligned. |
| Feedback | Notification | NotificationPage | Notification | Pass | Open Basic notification compared against `build/notification-compare-wide.png`; official no-icon `open` call, title/description text, close affordance, top-right placement, and Basic/Success trigger row aligned. Progress/loading behavior remains covered by API tests and interactive QA. |
| Feedback | Popconfirm | PopconfirmPage | Popconfirm | Pass | Open Basic popconfirm compared against `build/popconfirm-compare-wide.png`; official title/description, warning icon, Yes/No small buttons, arrow, and danger trigger aligned. The Qt helper crop differs from the full reference sidebar layout but no control-owned visual mismatch remains. |
| Feedback | Progress | ProgressPage | Progress | Pass | Line and Circle demos compared against `build/progress-compare-wide.png`; official 80/60 active/100/50 exception values, rails, status colors, and success/error glyphs aligned. Shared demo-container width remains out of scope. |
| Feedback | Result | ResultPage | Result | Pass | Success and Error results compared against `build/result-compare-wide.png`; official icon colors, titles, subtitles, centered layout, and primary Go Home action aligned. Shared Card body height remains out of scope. |
| Feedback | Skeleton | SkeletonPage | Skeleton | Pass | Basic and Avatar & Title skeletons compared against `build/skeleton-compare-wide.png`; active shimmer, avatar circle, title/paragraph bar rhythm, and muted fill color aligned. Shared Card body width remains out of scope. |
| Feedback | Spin | SpinPage | Spin | Pass | Basic and With Content spin demos compared against `build/spin-compare-wide.png`; small/default spin dots, large content spinner, Loading text, and muted content panel aligned. Animation phase is expected to vary frame-to-frame. |
| Feedback | Tour | TourPage | Tour | Pass | Static Basic reference compared against `build/tour-compare-wide.png`; instructional copy and Step 1/2/3 trigger buttons aligned. Open step overlay remains interactive QA because the current reference page does not render an active Tour. |
| Feedback | Watermark | WatermarkPage | Watermark | Pass | Basic watermark compared against `build/watermark-compare-wide.png`; default Ant Design text, -22 degree rotation, repeat gap, overlay alpha, bordered content surface, and centered content text aligned. |
| Other | App | AppPage | App | Pass | Static API-wrapper reference compared against `build/app-compare-wide.png`; official App Component section title and explanatory paragraph aligned. Inline code pill styling belongs to Typography; App behavior remains API-level and covered by tests. |
| Other | ConfigProvider | ConfigProviderPage | ConfigProvider | Pass | Static API-wrapper reference compared against `build/configprovider-compare-wide.png`; official ConfigProvider section title and explanatory paragraph aligned. Theme/locale application remains API-level and covered by tests. |
| Other | FloatButton | FloatButtonPage | FloatButton | Pass | Basic FloatButton compared against `build/floatbutton-compare-wide.png`; fixed bottom-right placement, circular elevated surface, and Home icon rendering aligned. Shared full-layout/sidebar crop difference remains out of scope. |
| Qt Extension | Window | N/A | Window | N/A | Qt-only desktop extension; no official AntD visual reference. Keep covered by local example and `TestAntQtExtensions`. |
| Qt Extension | ScrollArea | N/A | ScrollArea | N/A | Qt-only desktop extension; no official AntD visual reference. Keep covered by local example and `TestAntQtExtensions`. |
| Qt Extension | ScrollBar | N/A | ScrollBar | N/A | Qt-only desktop extension; no official AntD visual reference. Keep covered by local example and `TestAntQtExtensions`. |
| Qt Extension | StatusBar | N/A | StatusBar | N/A | Qt-only desktop extension; no official AntD visual reference. Keep covered by local example and `TestAntQtExtensions`. |
| Qt Extension | MenuBar | N/A | MenuBar | N/A | Qt-only desktop extension; no official AntD visual reference. Keep covered by local example and `TestAntQtExtensions`. |
| Qt Extension | ToolBar | N/A | ToolBar | N/A | Qt-only desktop extension; no official AntD visual reference. Keep covered by local example and `TestAntQtExtensions`. |
| Qt Extension | ToolButton | N/A | ToolButton | N/A | Qt-only desktop extension; no official AntD visual reference. Keep covered by local example and `TestAntQtExtensions`. |
| Qt Extension | DockWidget | N/A | DockWidget | N/A | Qt-only desktop extension; no official AntD visual reference. Keep covered by local example and `TestAntQtExtensions`. |
| Qt Extension | PlainTextEdit | N/A | PlainTextEdit | N/A | Qt-only desktop extension; no official AntD visual reference. Keep covered by local example and `TestAntQtExtensions`. |
| Qt Extension | Log | N/A | Log | N/A | Qt-only desktop extension; no official AntD visual reference. Keep covered by local example and `TestAntQtExtensions`. |
| Qt Extension | Masonry | N/A | Masonry | N/A | Qt-only desktop extension; no official AntD visual reference. Keep covered by local example and `TestAntQtExtensions`. |
