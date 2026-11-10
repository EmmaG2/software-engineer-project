# Qt Component Showcase

## Objective
Create a static Qt Widgets component showcase that demonstrates the new high-contrast design system and centralize UI tokens in `src/style`.

## Scope
- Add exported UI token declarations/definitions in `src/style/constants.h` and `src/style/constants.cpp`.
- Move reusable visual styling decisions out of `mainwindow.cpp` and consume the centralized tokens.
- Add a rich static Dashboard showcase with representative controls, data display, feedback, navigation, and layout components.
- Preserve existing navigation, icon resources, and collapse behavior.

## Constraints
- Qt 6 Widgets and C++17 only.
- No business logic, persistence, authentication, validation, or database access.
- Demo interactions may be presentational only; no stateful domain behavior.
- Keep the design Uber-inspired, not an official Uber implementation.

## Tasks
- [x] **T1 — Centralize design tokens**
  - Allowed edit surfaces: `src/style/constants.h`, `src/style/constants.cpp`, `CMakeLists.txt`, `src/mainwindow.cpp`
  - Outcome: exported palette, semantic colors, 4px spacing scale, radii, control sizing, icon sizing, typography, and weights from `src/style/constants.*`.
  - Evidence: shell and component styles consume the exported token namespace; `git diff --check` passed.
- [x] **T2 — Build the static component showcase**
  - Allowed edit surfaces: `src/mainwindow.cpp`
  - Outcome: Dashboard includes static controls, metric cards, feedback, tabs, table, list, tree, and layout examples without business logic.
  - Evidence: source inspection confirms the requested Qt Widget categories.
- [x] **T3 — Verify the demo application**
  - Allowed edit surfaces: none
  - Outcome: clean configure/build and offscreen launch completed without SVG/resource errors.
  - Evidence: `git diff --check` and CMake exited 0; runtime ran for 2 seconds and was intentionally terminated.
  - Note: non-fatal warnings remain for Vulkan headers and the local `Sans Serif` font alias.

## Progress
- Current task: none; all planned tasks complete.
- Commits: none.

## Verification Evidence
- T1/T2: centralized tokens and static Dashboard showcase implemented.
- T3: build and runtime smoke test passed; no SVG/resource errors.

## Next Step
Review the component showcase and authorize a semantic commit if desired.
