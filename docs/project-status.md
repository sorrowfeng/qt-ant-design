# Project Status

Updated: `2026-04-30`

This snapshot records the current state after the Showcase, ColorPicker popup, AntWindow outline, and official Ant Design Icon resource work.

## Summary

| Area | Status |
| --- | --- |
| Ant Design standard coverage | `70 / 70` top-level components covered |
| Public Qt component count | `82` public components |
| Widget headers | `83` headers in `src/widgets`; `AntSelectPopup` is an internal helper and is not counted as a public component |
| Qt / desktop extensions | `12` components |
| Style architecture | `62` `Ant*Style` classes, plus custom-paint/helper components where a style class is not useful |
| Example coverage | `80 / 82` public components, plus the standalone `Showcase` page |
| Dedicated examples intentionally absent | `AntWidget`, `AntNavItem`; both are infrastructure widgets used by the example shell |
| Tests | `20` CTest targets, all passing in Debug on `2026-04-30` |
| Official icon resources | `831` SVG files from `@ant-design/icons-svg@4.4.2` |

## Recent Completed Work

- Added a pure-control Showcase page matching the Ant Design homepage showcase without the website shell, theme sidebar, or decorative background.
- Reworked `AntColorPicker` from a modal editor to an AntD-like click-open popup panel.
- Added a non-maximized `AntWindow` outline so frameless windows remain distinguishable on similarly colored desktop backgrounds.
- Bundled all official Ant Design SVG icons and connected them to `AntIcon` through a string-name API.
- Added `docs/ant-design-icons.md` with the full icon inventory.
- Updated visual audit notes for Showcase, ColorPicker popup, Icon resources, and Qt extension consistency.

## Visual Audit State

The component visual audit matrix in `docs/visual-audit.md` is current:

- Comparable Ant Design standard components are marked `Pass`.
- Qt-only desktop extensions are marked `Local Pass`.
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
cmake --build build --config Debug --target TestAntIcon qt-ant-design-example -- /m:1
ctest --test-dir build -C Debug --output-on-failure
```

Result: `20 / 20` tests passed.

## Remaining Notes

- `build/` contains temporary capture helpers and screenshots only; keep those untracked.
- `docs/ant-design-reference.html` remains the stable component comparison reference.
- The official `https://ant.design/index-cn` homepage was useful for the Showcase target, but it produced hydration/resource errors during capture on `2026-04-29`; repeatable audits use the local HTML reference instead.
- Exact pixel parity is not guaranteed forever; component fixes should continue to use the documented screenshot comparison loop.
