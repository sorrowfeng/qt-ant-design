# Visual Audit Checklist

This file tracks the next phase: comparing each Qt widget page against the Ant Design reference page in `docs/ant-design-reference.html`.

## Workflow

1. Open the Ant Design reference page and the Qt example page for the same component.
2. Check both light and dark themes at the same viewport size.
3. Compare default, hover, active, focus, disabled, error, warning, loading, and empty states where applicable.
4. Record the result as `Pass`, `Needs fix`, or `Blocked`.
5. Keep fixes scoped to the audited component and rerun `ctest -C Debug --output-on-failure`.

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
| General | Button | ButtonPage | Button | Not started | |
| General | Icon | IconPage | Icon | Not started | |
| General | Typography | TypographyPage | Typography | Not started | |
| Layout | Divider | DividerPage | Divider | Not started | |
| Layout | Flex | FlexPage | Flex | Not started | |
| Layout | Grid | GridPage | Grid | Not started | |
| Layout | Layout | LayoutPage | Layout | Not started | |
| Layout | Space | SpacePage | Space | Not started | |
| Layout | Splitter | SplitterPage | Splitter | Not started | |
| Navigation | Affix | AffixPage | Affix | Not started | |
| Navigation | Anchor | AnchorPage | Anchor | Not started | |
| Navigation | Breadcrumb | BreadcrumbPage | Breadcrumb | Not started | |
| Navigation | Dropdown | DropdownPage | Dropdown | Not started | |
| Navigation | Menu | MenuPage | Menu | Not started | |
| Navigation | Pagination | PaginationPage | Pagination | Not started | |
| Navigation | Steps | StepsPage | Steps | Not started | |
| Data Entry | AutoComplete | AutoCompletePage | AutoComplete | Not started | |
| Data Entry | Cascader | CascaderPage | Cascader | Not started | |
| Data Entry | Checkbox | CheckboxPage | Checkbox | Not started | |
| Data Entry | ColorPicker | ColorPickerPage | ColorPicker | Not started | |
| Data Entry | DatePicker | DatePickerPage | DatePicker | Not started | |
| Data Entry | Form | FormPage | Form | Not started | |
| Data Entry | Input | InputPage | Input | Not started | |
| Data Entry | InputNumber | InputNumberPage | InputNumber | Not started | |
| Data Entry | Mentions | MentionsPage | Mentions | Not started | |
| Data Entry | Radio | RadioPage | Radio | Not started | |
| Data Entry | Rate | RatePage | Rate | Not started | |
| Data Entry | Select | SelectPage | Select | Not started | |
| Data Entry | Slider | SliderPage | Slider | Not started | |
| Data Entry | Switch | SwitchPage | Switch | Not started | |
| Data Entry | TimePicker | TimePickerPage | TimePicker | Not started | |
| Data Entry | Transfer | TransferPage | Transfer | Not started | |
| Data Entry | TreeSelect | TreeSelectPage | TreeSelect | Not started | |
| Data Entry | Upload | UploadPage | Upload | Not started | |
| Data Display | Avatar | AvatarPage | Avatar | Not started | |
| Data Display | Badge | BadgePage | Badge | Not started | |
| Data Display | Calendar | CalendarPage | Calendar | Not started | |
| Data Display | Card | CardPage | Card | Not started | |
| Data Display | Carousel | CarouselPage | Carousel | Not started | |
| Data Display | Collapse | CollapsePage | Collapse | Not started | |
| Data Display | Descriptions | DescriptionsPage | Descriptions | Not started | |
| Data Display | Empty | EmptyPage | Empty | Not started | |
| Data Display | Image | ImagePage | Image | Not started | |
| Data Display | List | ListPage | List | Not started | |
| Data Display | Popover | PopoverPage | Popover | Not started | Also audited under Feedback behavior |
| Data Display | QRCode | QRCodePage | QRCode | Not started | |
| Data Display | Segmented | SegmentedPage | Segmented | Not started | |
| Data Display | Statistic | StatisticPage | Statistic | Not started | |
| Data Display | Table | TablePage | Table | Not started | |
| Data Display | Tabs | TabsPage | Tabs | Not started | |
| Data Display | Tag | TagPage | Tag | Not started | |
| Data Display | Timeline | TimelinePage | Timeline | Not started | |
| Data Display | Tooltip | TooltipPage | Tooltip | Not started | Also audited under Feedback behavior |
| Data Display | Tree | TreePage | Tree | Not started | |
| Feedback | Alert | AlertPage | Alert | Not started | |
| Feedback | Drawer | DrawerPage | Drawer | Not started | |
| Feedback | Message | MessagePage | Message | Not started | |
| Feedback | Modal | ModalPage | Modal | Not started | |
| Feedback | Notification | NotificationPage | Notification | Not started | |
| Feedback | Popconfirm | PopconfirmPage | Popconfirm | Not started | |
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
