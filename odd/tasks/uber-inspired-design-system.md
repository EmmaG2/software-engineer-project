# Uber-Inspired Qt Design System

## Objective
Apply a consistent high-contrast black-and-white design system to the Qt Widgets desktop shell, inspired by public Uber Base Web principles.

## Scope
- Centralize visual tokens for colors, spacing, typography, borders, and states.
- Restyle the sidebar, header, module pages, cards, buttons, and status text.
- Preserve existing navigation and collapse behavior.

## Constraints
- Qt 6 Widgets and C++17 only.
- No business logic, forms, validation, persistence, or authentication.
- Use an Uber-inspired visual language without presenting it as official Uber branding.

## Tasks
- [x] **T1 — Apply centralized visual tokens and component styles**
  - Allowed edit surfaces: `src/mainwindow.cpp`
  - Outcome: centralized black/white palette, 4px spacing rhythm, restrained radius, typography hierarchy, and hover/active/focus states across the shell, navigation, cards, header, and status.
  - Evidence: source inspection confirms token reuse and preserved collapse/navigation behavior; `git diff --check` passed.
- [x] **T2 — Verify the styled Qt shell**
  - Allowed edit surfaces: none
  - Outcome: clean configure/build and offscreen launch completed without SVG resource errors.
  - Evidence: configure and build exited 0; offscreen app remained running for 3 seconds and was intentionally terminated.
  - Note: non-fatal warnings remain for missing Vulkan headers and the local `Sans Serif` font alias.

## Progress
- Current task: none; all planned tasks complete.
- Commits: none.

## Verification Evidence
- T1: centralized visual tokens and component styles applied in `src/mainwindow.cpp`.
- T2: `git diff --check`, clean CMake configure/build, and offscreen runtime passed.

## Next Step
Review the visual system and authorize a commit if desired.
