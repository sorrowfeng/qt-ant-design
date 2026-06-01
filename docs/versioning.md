# Versioning And Release Process

`qt-ant-design` uses Semantic Versioning: `MAJOR.MINOR.PATCH`.

## Version Source

- The root `VERSION` file is the single source for the release version.
- Top-level CMake reads `VERSION` and exposes it as `PROJECT_VERSION`.
- CMake generates `core/QtAntDesignVersion.h` for consumers. Include it with:

```cpp
#include "core/QtAntDesignVersion.h"

static_assert(QT_ANT_DESIGN_VERSION_MAJOR == 0);
static_assert(QT_ANT_DESIGN_VERSION_MINOR == 1);
static_assert(QT_ANT_DESIGN_VERSION_PATCH == 0);
```

## Release Checklist

1. Update `VERSION`.
2. Add a matching section to `CHANGELOG.md`.
3. Build and run the targeted tests for the changed surface.
4. Commit the version change on `main`.
5. Create an annotated tag named `vX.Y.Z`.
6. Push `main` and the tag.
7. Create the GitHub Release from that tag, using the matching changelog section as release notes.

## Current Release

- Current version: `0.1.0`
- Tag: `v0.1.0`

