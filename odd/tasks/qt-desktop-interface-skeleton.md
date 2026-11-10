# Qt Desktop Interface Skeleton

## Objective
Create a navigable Qt Widgets desktop interface skeleton for the café inventory application without forms, validations, persistence, or business logic.

## Problem
The project currently opens a placeholder label and does not expose the modules identified in the requirements.

## Scope
- Replace the placeholder window with a desktop shell: navigation sidebar, header, and stacked content area.
- Add static module pages for Dashboard, Sales, Inventory & Lots, Lot Receipts, Waste, Recipes, Alerts, Purchase Orders, Reports, and Users & Roles.
- Use only Qt Widgets components and static descriptive content.

## Constraints
- No database access, business logic, validations, data-entry forms, or user authentication.
- Preserve the existing Qt 6 Widgets build configuration.
- Source code and UI copy remain in English.

## Tasks
- [x] **T1 — Build the static desktop shell and module pages**
  - Allowed edit surfaces: `src/mainwindow.h`, `src/mainwindow.cpp`
  - Outcome: added sidebar navigation, header, and ten static stacked module pages in `src/mainwindow.cpp`.
  - Evidence: source inspection confirms each navigation button changes the matching `QStackedWidget` page; `git diff --check` passed.
  - Check: CMake build deferred to T2.
- [x] **T2 — Verify the Qt Widgets build**
  - Allowed edit surfaces: none
  - Outcome: QtDesktopApp configured and compiled successfully in an external temporary build directory.
  - Evidence: `cmake -S /Users/emmanuelgranados/dev/cpp/software-engineer-project -B /tmp/qt-desktop-interface-build.H3facj` exited 0; `cmake --build /tmp/qt-desktop-interface-build.H3facj` exited 0.
  - Note: configuration emitted the non-fatal `Could NOT find WrapVulkanHeaders` notice.

## Progress
- Current task: none; all planned tasks complete.
- Commits: none; commits require explicit user authorization.

## Verification Evidence
- T1: `git diff --check` passed; navigation and static pages inspected.
- T2: CMake configure and build passed; no compiler errors or warnings observed.

## Next Step
Review the static interface and authorize a commit if a work-unit commit is desired.
