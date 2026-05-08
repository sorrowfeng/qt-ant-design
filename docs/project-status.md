# Project Status

Updated: `2026-05-08`

This snapshot records the current state after the Showcase, ColorPicker popup, AntWindow outline and desktop-window polish, official Ant Design Icon resource work, the 2026-04-30 interaction/motion parity pass, Qt5/Qt6 static/shared build-system support, installed package coverage, and lifecycle stress coverage.

## Summary

| Area | Status |
| --- | --- |
| Ant Design standard coverage | `70 / 70` top-level components covered |
| Public Qt component count | `82` public components |
| Widget headers | `103` headers in `src/widgets`: `82` public component headers, `20` Qt-style alias headers, and the internal popup helper `AntSelectPopup` |
| Qt / desktop extensions | `12` components |
| Style architecture | `62` `Ant*Style` classes, plus custom-paint/helper components where a style class is not useful |
| Example coverage | `82 / 82` public components, plus the standalone `Showcase` page |
| Dedicated examples intentionally absent | None |
| Tests | `37` CTest targets configured; latest build-system / install targeted verification passed in Debug on `2026-05-08` |
| Official icon resources | `831` SVG files from `@ant-design/icons-svg@4.4.2` |

## Recent Completed Work

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
- Expanded Qt-style compatibility for `AntList` / `AntListWidget` to cover common `QListWidget` workflows: string item insertion, lookup/sorting, item data roles, current row/item state, selection state, internal scrolling, `scrollToItem`, and item/current/selection signals.
- Expanded `AntTable` helper coverage with `rows()`, `selectRow()`, `currentRowIndex()`, and row-level tooltip data/display support.
- Added `AntTypography::setPixelSize()` / `pixelSize()` for direct label-style font sizing while keeping theme-aware rendering.
- Added `AntSelect` option-style list management helpers: `setOptionText()`, `removeOption()`, and `optionData()`, alongside the existing `findData()` lookup path.
- Aligned `AntSelect` with `QComboBox` for the empty-to-populated path: the first enabled option added in single-select mode becomes the current item automatically.
- Tuned `AntList` selected-row highlight geometry so left/right inset matches the existing top/bottom inset for a balanced row highlight.
- Fixed `AntCard` title and Meta title label palettes after Light/Dark theme changes so dark-mode card headers stay legible.
- Fixed issue-driven popup details for `AntMenu` horizontal submenu panel edges and `AntCascader` outside-click dismissal.
- Audited Qt layout adaptivity against native widget baselines: line-edit-like controls, combo-like selectors, spin/date/time editors, slider/progress, list/table/tree views, scroll/text/status controls, and typography now carry native-like `QSizePolicy` / height-for-width behavior with targeted render-smoke coverage.
- Aligned `AntInputNumber` with the desired integer-first decimal flow: the default display remains integer (`decimals() == 0`), while `setDecimals()` / `setPrecision()` enables decimal value/range/step behavior, preserves quarter-step increments, and updates the public `precision()` state with `precisionChanged`.
- Fixed `AntSlider` marked and range interaction details: marked sliders reserve label height under Qt layouts, drag value bubbles anchor above the visual handle, and range drags no longer paint a phantom primary handle at the minimum edge.

## Visual Audit State

The component visual audit matrix in `docs/visual-audit.md` is current:

- Comparable Ant Design standard components are marked `Pass`.
- Qt-only desktop extensions are marked `Local Pass`.
- `TestAntVisualRegression` now guards stable pixel-level regressions across token fills, semantic status colors, selection controls, feedback surfaces, data-display structure, navigation/layout structure, popup surfaces, and light/dark surface contrast.
- The homepage Showcase audit is marked `Pass` against the isolated local HTML and Qt control pages.
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

Latest build-system / install targeted validation:

```powershell
ctest --test-dir build -C Debug -R "TestAntBuildSystem|TestAntInstallConsumer|TestAntIcon|TestAntAliases" --output-on-failure
```

Result: `4 / 4` targeted tests passed.

Latest targeted AntWindow verification:

```powershell
ctest -C Debug -R "TestAntQtExtensions|TestAntExampleCloseStress" --output-on-failure
```

Result: `2 / 2` targeted tests passed during the AntWindow desktop behavior pass.

Latest targeted AntList / AntTable / AntTypography API validation:

```powershell
ctest -C Debug -R "TestAnt(DataDisplayB|Typography)$" --output-on-failure
```

Result: `2 / 2` targeted tests passed on `2026-05-08`.

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
ctest --test-dir build -C Debug -R "TestAntDataEntryA$" --output-on-failure
```

Result: `1 / 1` targeted test passed on `2026-05-08`.

Latest targeted AntCard theme validation:

```powershell
ctest -C Debug -R "TestAntDataDisplayA$" --output-on-failure
```

Result: `1 / 1` targeted test passed on `2026-05-08`.

## Remaining Notes

- `build/` contains temporary capture helpers and screenshots only; keep those untracked.
- `docs/ant-design-reference.html` remains the stable component comparison reference.
- The official `https://ant.design/index-cn` homepage was useful for the Showcase target, but it produced hydration/resource errors during capture on `2026-04-29`; repeatable audits use the local HTML reference instead.
- Exact pixel parity is not guaranteed forever; component fixes should continue to use the documented screenshot comparison loop.
