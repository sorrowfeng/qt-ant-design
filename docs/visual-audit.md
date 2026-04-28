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
| Layout | Divider | DividerPage | Divider | Not started | |
| Layout | Flex | FlexPage | Flex | Not started | |
| Layout | Grid | GridPage | Grid | Not started | |
| Layout | Layout | LayoutPage | Layout | Not started | |
| Layout | Space | SpacePage | Space | Not started | |
| Layout | Splitter | SplitterPage | Splitter | Not started | |
| Navigation | Affix | AffixPage | Affix | Not started | |
| Navigation | Anchor | AnchorPage | Anchor | Not started | |
| Navigation | Breadcrumb | BreadcrumbPage | Breadcrumb | Not started | |
| Navigation | Dropdown | DropdownPage | Dropdown | Needs visual QA | Resolved placement now drives popup arrow/body direction; compare menu density, arrow alignment, hover/click/context behavior. |
| Navigation | Menu | MenuPage | Menu | Not started | |
| Navigation | Pagination | PaginationPage | Pagination | Not started | |
| Navigation | Steps | StepsPage | Steps | Not started | |
| Data Entry | AutoComplete | AutoCompletePage | AutoComplete | Not started | |
| Data Entry | Cascader | CascaderPage | Cascader | Not started | |
| Data Entry | Checkbox | CheckboxPage | Checkbox | Needs visual QA | Disabled checked mark adjusted; compare basic, group spacing, checked, indeterminate, and disabled states. |
| Data Entry | ColorPicker | ColorPickerPage | ColorPicker | Not started | |
| Data Entry | DatePicker | DatePickerPage | DatePicker | Needs visual QA | Range display, hover border, filled hover, and clear visibility fixed; compare popup grid and range interaction. |
| Data Entry | Form | FormPage | Form | Not started | |
| Data Entry | Input | InputPage | Input | Needs fix | Disabled clear visibility fixed; TextArea remains missing from the Qt Input page/API coverage. |
| Data Entry | InputNumber | InputNumberPage | InputNumber | Needs visual QA | Reference states are represented via prefix/suffix and controls; compare handler width, addon-like suffix, and filled/underlined variants. |
| Data Entry | Mentions | MentionsPage | Mentions | Not started | |
| Data Entry | Radio | RadioPage | Radio | Needs fix | Checked radio rendering fixed; AntD button-style radio group remains missing. |
| Data Entry | Rate | RatePage | Rate | Not started | |
| Data Entry | Select | SelectPage | Select | Needs visual QA | Basic, multiple, sizes, loading, disabled, and tags examples are present; compare popup density, tags, and clear/loading icons. |
| Data Entry | Slider | SliderPage | Slider | Needs fix | Basic, dots, vertical, reverse, and disabled are present; range slider and labeled marks remain missing. |
| Data Entry | Switch | SwitchPage | Switch | Needs visual QA | Basic, text, loading, small, and disabled states are present; compare track color, handle motion, and text placement. |
| Data Entry | TimePicker | TimePickerPage | TimePicker | Needs visual QA | Range display, hover border, filled hover, and clear visibility fixed; compare popup columns and footer controls. |
| Data Entry | Transfer | TransferPage | Transfer | Not started | |
| Data Entry | TreeSelect | TreeSelectPage | TreeSelect | Not started | |
| Data Entry | Upload | UploadPage | Upload | Not started | |
| Data Display | Avatar | AvatarPage | Avatar | Not started | |
| Data Display | Badge | BadgePage | Badge | Needs visual QA | Processing pulse animation fixed; compare overlay position, count pill, dot, and status text in screenshots. |
| Data Display | Calendar | CalendarPage | Calendar | Not started | |
| Data Display | Card | CardPage | Card | Pass | Basic card and cover/meta/actions card compared against `build/card-compare-wide.png`; component-owned gaps fixed for extra color, Meta padding, action centering, and action separators. Shared demo-container spacing remains out of scope. |
| Data Display | Carousel | CarouselPage | Carousel | Needs visual QA | Qt demo now matches the reference source for the Basic autoplay carousel: 160px gradient slides, centered white label, rounded viewport, and AntD pill dots overlaid above slide content. `docs/ant-design-reference.html` currently renders Carousel collapsed in Playwright, so final screenshot pass is pending a reference CSS/render fix. |
| Data Display | Collapse | CollapsePage | Collapse | Pass | Basic and Accordion sections compared against `build/collapse-compare-wide.png`; default non-accordion behavior, expanded first panel, rounded border, header background, content padding, separators, and arrow placement aligned. Shared demo-container spacing remains out of scope. |
| Data Display | Descriptions | DescriptionsPage | Descriptions | Pass | Basic bordered two-column table compared against `build/descriptions-compare-wide.png`; demo coverage, item padding, vertical sizing, label background, and processing Badge content aligned. Shared demo-container spacing remains out of scope. |
| Data Display | Empty | EmptyPage | Empty | Pass | Basic and Custom Description compared against `build/empty-compare-wide.png`; default description casing, official demo coverage, default illustration structure, image sizing, description color, and image/description spacing aligned. Shared demo-container spacing remains out of scope. |
| Data Display | Image | ImagePage | Image | Pass | Basic image compared against `build/image-compare-wide.png`; demo now uses the official sample image, loaded images render without placeholder chrome, width-only sizing preserves aspect ratio, and hover preview mask uses an AntD-like eye/text treatment. Shared demo-container spacing remains out of scope. |
| Data Display | List | ListPage | List | Not started | |
| Data Display | Popover | PopoverPage | Popover | Needs visual QA | Resolved placement now drives arrow direction; optional title icon supports Popconfirm; compare title/body spacing, shadow, action area, and hover/click dismissal. |
| Data Display | QRCode | QRCodePage | QRCode | Not started | |
| Data Display | Segmented | SegmentedPage | Segmented | Not started | |
| Data Display | Statistic | StatisticPage | Statistic | Not started | |
| Data Display | Table | TablePage | Table | Not started | |
| Data Display | Tabs | TabsPage | Tabs | Not started | |
| Data Display | Tag | TagPage | Tag | Needs visual QA | Reference variants are represented; compare preset tint/border and closable hover in screenshots. |
| Data Display | Timeline | TimelinePage | Timeline | Not started | |
| Data Display | Tooltip | TooltipPage | Tooltip | Needs visual QA | Resolved placement is separated from requested placement; compare dark bubble color, arrow alignment, custom color contrast, and delay. |
| Data Display | Tree | TreePage | Tree | Not started | |
| Feedback | Alert | AlertPage | Alert | Not started | |
| Feedback | Drawer | DrawerPage | Drawer | Needs visual QA | Default width/height and content padding aligned closer to AntD; compare header height, edge shadow, mask opacity, and placements. |
| Feedback | Message | MessagePage | Message | Needs visual QA | Loading spinner uses live angle and placements stack independently; compare top/bottom offsets, shadow, and icon geometry. |
| Feedback | Modal | ModalPage | Modal | Needs visual QA | Command API icons now render with status colors; compare title/body/footer spacing, mask opacity, and centered/top-offset layouts. |
| Feedback | Notification | NotificationPage | Notification | Needs visual QA | Progress bar now reflects remaining duration and loading icon uses live angle; compare card width, close hover, and placement stacks. |
| Feedback | Popconfirm | PopconfirmPage | Popconfirm | Needs visual QA | Disabled trigger now blocks the internal popover and warning title icon is present; compare icon alignment, button spacing, and placement arrows. |
| Feedback | Progress | ProgressPage | Progress | Not started | |
| Feedback | Result | ResultPage | Result | Not started | |
| Feedback | Skeleton | SkeletonPage | Skeleton | Not started | |
| Feedback | Spin | SpinPage | Spin | Not started | |
| Feedback | Tour | TourPage | Tour | Not started | |
| Feedback | Watermark | WatermarkPage | Watermark | Not started | |
| Other | App | AppPage | App | Not started | API wrapper, visual scope is limited |
| Other | ConfigProvider | ConfigProviderPage | ConfigProvider | Not started | API wrapper, visual scope is limited |
| Other | FloatButton | FloatButtonPage | FloatButton | Not started | |
| Qt Extension | Window | N/A | Window | Not started | Desktop extension |
| Qt Extension | ScrollArea | N/A | ScrollArea | Not started | Desktop extension |
| Qt Extension | ScrollBar | N/A | ScrollBar | Not started | Desktop extension |
| Qt Extension | StatusBar | N/A | StatusBar | Not started | Desktop extension |
| Qt Extension | MenuBar | N/A | MenuBar | Not started | Desktop extension |
| Qt Extension | ToolBar | N/A | ToolBar | Not started | Desktop extension |
| Qt Extension | ToolButton | N/A | ToolButton | Not started | Desktop extension |
| Qt Extension | DockWidget | N/A | DockWidget | Not started | Desktop extension |
| Qt Extension | PlainTextEdit | N/A | PlainTextEdit | Not started | Desktop extension |
| Qt Extension | Log | N/A | Log | Not started | Desktop extension |
| Qt Extension | Masonry | N/A | Masonry | Not started | Desktop extension |
