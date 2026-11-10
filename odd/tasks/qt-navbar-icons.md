# Qt Navbar Icons

## Objective
Replace compact navbar letters with bundled modern SVG icons from Lucide and register them as Qt resources.

## Scope
- Add official Lucide SVG assets under `src/assets`.
- Register all assets in a Qt resource file.
- Update CMake and the navbar to use `QIcon` while preserving collapsed/expanded behavior.

## Constraints
- No business logic, forms, validation, persistence, or authentication.
- Keep Qt 6 Widgets and C++17.
- Verify downloaded assets are valid SVG files and use the official Lucide source.

## Tasks
- [x] **T1 — Add and register SVG icon assets**
  - Allowed edit surfaces: `src/assets/*.svg`, `src/resources.qrc`, `CMakeLists.txt`
  - Outcome: added twelve official Lucide SVG assets and registered them in `src/resources.qrc`; linked Qt Svg.
  - Evidence: assets are 24×24 SVGs with neutral navbar strokes and appear in the resource manifest.
- [x] **T2 — Connect icons to the collapsible navbar**
  - Allowed edit surfaces: `src/mainwindow.cpp`
  - Outcome: expanded mode shows icon plus label; collapsed mode shows icon-only buttons with tooltips; toggle switches between directional panel icons.
  - Evidence: source inspection confirms icon mapping and state-specific behavior; `git diff --check` passed.
- [x] **T3 — Verify the Qt build**
  - Allowed edit surfaces: none
  - Outcome: installed Nix package `qt6Packages.qtsvg` and verified a clean configure/build.
  - Evidence: `nix profile install nixpkgs#qt6Packages.qtsvg` succeeded; CMake configure and build completed successfully in `/tmp/qt-navbar-icons-build` and the repository `build/` directory.
  - Runtime evidence: the rebuilt executable starts without SVG resource errors; only the non-fatal font alias notice remains.
  - Note: configuration still reports the non-fatal `WrapVulkanHeaders` notice.

## Progress
- Current task: none; all planned tasks complete.
- Commits: none.

## Verification Evidence
- T1/T2: implementation complete; twelve 24×24 SVG assets exist and are registered in `src/resources.qrc`.
- T3: Qt configure/build passed after installing Qt Svg; `git diff --check` passed.
- Runtime: all SVG resources load successfully after enabling `CMAKE_AUTORCC` and using resource aliases in `src/resources.qrc`.

## Next Step
Review the icon appearance and authorize a commit if desired.
