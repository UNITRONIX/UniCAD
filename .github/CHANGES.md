# UniCAD — Changelog & New Features

**UniCAD** is a customized build of FreeCAD 1.2.0-dev by [UNITRONIX](https://github.com/UNITRONIX), introducing a modern Fusion 360-inspired user interface, unified parametric modeling commands, and direct-edit face operations.

> Based on FreeCAD `1.2.0-dev` (upstream commit `e3857b11`).

---

## Overview of Changes

| Area | Summary |
|---|---|
| UI Layer | Full Fusion 360-style ribbon toolbar, timeline, navigation bar, marking menu, command search |
| Navigation | New mouse navigation style matching Fusion 360 (MMB orbit, Shift+MMB pan) |
| Theme | Dark-blue stylesheet (`UniCAD Dark.qss`) |
| PartDesign | Unified Extrude/Revolve/Sweep/Loft with Join/Cut/Intersect modes |
| Direct Edit | OffsetFace, DeleteFace, ReplaceFace, SplitFace, SplitBody, MoveFace |
| Adaptive Tool | Press/Pull (`Q`) — context-aware: Sketch→Extrude, Edge→Fillet, Face→Offset |
| Sketcher | Face preselection, InternalFace rendering, snap indicator visualization |
| Build System | External SMESH support, CMake 4.x compatibility, restored FEM/BIM/MeshPart/OpenSCAD modules |
| Branding | FreeCAD → FusionCAD → **UniCAD** |

---

## 1. Fusion 360-Style User Interface

### 1.1 Tab Toolbar (`FusionTabToolbar`)

Replaces the traditional floating toolbars with a horizontal ribbon organized into workbench-specific tabs and panels.

**PartDesign workbench tabs:**

| Tab | Panels | Commands |
|---|---|---|
| **SOLID** | CREATE | New Sketch, Extrude (E), Revolve (R), Sweep, Loft |
| | DIRECT EDIT | Press/Pull (Q), OffsetFace, MoveFace, DeleteFace, ReplaceFace |
| | BOOLEAN | Boolean, Fuse, Cut, Common |
| **MODIFY** | FILLET & CHAMFER | Fillet, Chamfer |
| | EMBOSS | Emboss |
| | PATTERN | Linear Pattern, Polar Pattern, Mirrored, MultiTransform |
| **CONSTRUCT** | DATUM | Datum Plane, Datum Line, Datum Point |
| | CONNECT | Shape Binder, Sub-Shape Binder |
| **INSPECT** | — | Measure Distance, Measure Angle |
| **TOOLS** | — | Edit Preferences |

- 32×32 icons, panel labels at bottom
- Accent color: `#0696D7` (Fusion blue) on active tab
- Automatic switching when workbench changes

### 1.2 Feature Timeline (`FusionTimeline`)

Horizontal bottom dock widget that displays document features as sequential "chips":

- Current (tip) feature highlighted in blue (`#0696D7`), others in gray
- Chips connected by horizontal lines to show history flow
- Clicking a chip selects the corresponding feature in the model tree
- Auto-scrolls to keep the tip feature visible

### 1.3 Navigation Bar (`FusionNavigationBar`)

Bottom toolbar providing quick access to view controls:

- **View Presets**: Fit/Home, Front, Top, Right, Isometric
- **Display Styles**: Shaded, Wireframe
- **Toggle Tools**: Show/Hide, Perspective/Orthographic
- UniCAD branding label on the right side

### 1.4 Marking Menu (`FusionMarkingMenu`)

8-position radial (pie) context menu activated by right-click in the 3D viewport:

- Positions: N, NE, E, SE, S, SW, W, NW
- Overflow submenu for additional items
- Context-sensitive content based on selection type

### 1.5 Command Search (`CommandSearchDialog`)

Floating search bar activated with **Ctrl+/** (or **Ctrl+F**):

- Instant filtering across all registered commands
- Uses the existing `CommandCompleter` infrastructure
- Keyboard-driven: type to filter, Enter to execute

### 1.6 UI Manager (`FusionUIManager`)

Singleton that orchestrates all Fusion UI components:

- Hooks into workbench activation and document changes
- Hides traditional toolbars and menu bars when Fusion UI is active
- Preference toggle: `BaseApp/Preferences/View/FusionUIEnabled`

---

## 2. Fusion 360 Navigation Style

New `FusionNavigationStyle` providing mouse/keyboard mapping that matches Fusion 360:

| Input | Action |
|---|---|
| Middle Mouse Button (drag) | Orbit |
| Shift + MMB (drag) | Pan |
| Scroll Wheel | Zoom (at cursor position) |
| Double-click MMB | Fit All |

---

## 3. UniCAD Dark Theme

Custom Qt stylesheet (`UniCAD Dark.qss`, 539 lines) defining a dark-blue color palette:

| Token | Color |
|---|---|
| Toolbar Background | `#374150` |
| Canvas Background | `#465564` |
| Deep Background | `#2D3340` |
| Accent Blue | `#0696D7` |
| Accent Hover | `#6ECDFA` |
| Text Primary | `#F1F3F3` |
| Text Secondary | `#AFBDC5` |

---

## 4. Unified Parametric Modeling Commands

Traditional FreeCAD split Pad/Pocket, Revolution/Groove into separate operations. UniCAD unifies them:

### 4.1 Unified Extrude (`E`)

Single `PartDesign_Extrude` command with an **Operation** selector:

- **Join** (Pad) — adds material
- **Cut** (Pocket) — removes material
- **Intersect** — keeps common volume
- **New Body** — creates a separate body

Uses `FeatureUnifiedExtrude` internally.

### 4.2 Unified Revolve (`R`)

Single `PartDesign_Revolve` command with the same Join/Cut/Intersect/NewBody operations.

Uses `FeatureUnifiedRevolve` internally.

### 4.3 Unified Sweep & Loft

- `FeatureUnifiedSweep` — profile along a path with operation selector
- `FeatureUnifiedLoft` — transition between multiple profiles with operation selector

### 4.4 Press/Pull (`Q`)

Adaptive meta-command `PartDesign_PressPull` that inspects the current selection:

| Selection | Action |
|---|---|
| Sketch | → Creates Unified Extrude |
| Edge | → Creates Fillet |
| Face | → Creates OffsetFace |

---

## 5. Direct Edit Face Operations

New PartDesign features for direct manipulation of solid faces:

### 5.1 Offset Face (`PartDesign_OffsetFace`)

Offsets selected faces along their normals by a specified distance.

- **Algorithm**: For each face → compute outward normal → create prism → fuse (positive offset) or cut (negative offset) with base shape
- Uses `makeElementPrism` + boolean approach instead of `BRepOffsetAPI_MakeThickSolidByJoin` (avoids OCCT crash on complex geometry)
- Property: `Offset` (Double, default 1.0 mm)

### 5.2 Delete Face (`Shift+D`)

Removes selected faces and heals the resulting solid (`FeatureDeleteFace`).

### 5.3 Replace Face (`Shift+R`)

Replaces a face with a target surface (`FeatureReplaceFace`).

### 5.4 Split Face (`Shift+S`)

Splits faces using a tool shape (`FeatureSplitFace`).

### 5.5 Split Body

Splits the active body into multiple bodies.

### 5.6 Move Face

Moves selected faces by a distance along their normals (`FeatureMoveFace`).

---

## 6. Additional PartDesign Commands

| Command | Shortcut | Description |
|---|---|---|
| `PartDesign_Emboss` | — | Emboss/deboss text or shapes onto faces |
| `PartDesign_SelectFacePriority` | — | Switch selection filter to face priority |
| `PartDesign_SelectEdgePriority` | — | Switch selection filter to edge priority |
| `PartDesign_SelectBodyPriority` | — | Switch selection filter to body priority |
| `PartDesign_MoveBody` | — | Move body in the assembly |
| `PartDesign_CopyBody` | — | Copy body as a new instance |
| `PartDesign_SelectThrough` | — | Toggle select-through mode |

---

## 7. Sketcher Enhancements

### 7.1 Adaptive Grid (Enabled by Default)

UniCAD enables the Sketcher grid by default (`ShowGrid = true`, upstream default was `false`). The grid features **auto-spacing** — it automatically adjusts its cell size based on the current zoom level:

- **Zoomed out** → larger grid cells (coarse spacing)
- **Zoomed in** → smaller grid cells (fine spacing)

This makes it easy to place geometry at convenient coordinates regardless of scale. Grid vertices serve as snap targets for precise positioning.

**Z-offset fix**: Grid lines are rendered with a small z-offset (`0.001`) above the sketch plane. This prevents z-fighting where the depth buffer would hide grid lines behind coplanar face fills (sketch internal faces, body surfaces). Without this fix, the grid becomes invisible as soon as face shading is active.

### 7.2 Face Preselection & InternalFace Rendering

- UniCAD adds face preselection to sketches, allowing direct clicking on constraint-bounded regions
- Sketch faces are visually shaded, providing Fusion 360-style feedback

### 7.3 Snap Indicators & Face Center Snap

- Visual markers appear during sketch operations showing snap type (point, edge, grid, midpoint, face center)
- **New snap type: FaceCenter** — snaps to the center of sketch face regions
- Snap indicator visualization uses colored dots rendered via Coin3D scene graph

---

## 8. Build System Fixes

### 8.1 External SMESH Support

- Configured `FREECAD_USE_EXTERNAL_SMESH=ON` to use conda-forge's pre-built `smesh 9.9.0.0` package
- Eliminates dependency on MEDFile (unavailable on Windows conda-forge)
- Restores: **FEM**, **MeshPart**, **FlatMesh**, **BIM**, **OpenSCAD** modules

### 8.2 CMake 4.x Compatibility

- Patched `SMESH-targets.cmake`: `cmake_policy(VERSION 2.8.12...3.28)` → `cmake_policy(VERSION 3.5...3.28)`
- Required for CMake 4.0+ which rejects policy versions below 3.5

### 8.3 Icon Fix for New ViewProviders

- Made `ViewProviderDressUp::featureIcon()` `virtual`
- Added `featureIcon()` overrides in 6 new ViewProviders (OffsetFace, MoveFace, DeleteFace, ReplaceFace, SplitFace, SplitBody)
- Fixed `sPixmap` to omit `.svg` extension (FreeCAD icon system expects bare names)

---

## 9. New Source Files

The main feature commit (`b70db7ee`) added **150 files** with **+12,734 lines**. Key additions:

### Gui (UI Layer)
- `FusionTabToolbar.cpp / .h` — Ribbon toolbar
- `FusionTimeline.cpp / .h` — Feature timeline
- `FusionNavigationBar.cpp / .h` — Bottom navigation bar
- `FusionUIManager.cpp / .h` — UI orchestration singleton
- `FusionMarkingMenu.cpp / .h` — Radial context menu
- `CommandSearchDialog.cpp / .h` — Command search bar
- `Navigation/FusionNavigationStyle.cpp` — Fusion 360 mouse navigation
- `Stylesheets/UniCAD Dark.qss` — Dark theme stylesheet

### PartDesign App (Features)
- `FeatureOffsetFace.cpp / .h`
- `FeatureDeleteFace.cpp / .h`
- `FeatureMoveFace.cpp / .h`
- `FeatureReplaceFace.cpp / .h`
- `FeatureSplitFace.cpp / .h`
- `FeatureUnifiedExtrude.cpp / .h`
- `FeatureUnifiedRevolve.cpp / .h`
- `FeatureUnifiedSweep.cpp / .h`
- `FeatureUnifiedLoft.cpp / .h`

### PartDesign Gui (Task Panels)
- `TaskOffsetFaceParameters.cpp / .h / .ui`
- `TaskDeleteFaceParameters.cpp / .h / .ui`
- `TaskMoveFaceParameters.cpp / .h / .ui`
- `TaskReplaceFaceParameters.cpp / .h / .ui`
- `TaskSplitFaceParameters.cpp / .h / .ui`
- `TaskUnifiedExtrudeParameters.cpp / .h / .ui`
- `TaskUnifiedRevolveParameters.cpp / .h / .ui`
- `TaskUnifiedSweepParameters.cpp / .h / .ui`
- `TaskUnifiedLoftParameters.cpp / .h / .ui`

### PartDesign Gui (ViewProviders)
- `ViewProviderOffsetFace.cpp / .h`
- `ViewProviderDeleteFace.cpp / .h`
- `ViewProviderMoveFace.cpp / .h`
- `ViewProviderReplaceFace.cpp / .h`
- `ViewProviderSplitFace.cpp / .h`
- `ViewProviderSplitBody.cpp / .h`

### Sketcher
- `FusionSketchHelpers.py` — Python helper for sketch operations

---

## 10. Branding

| Step | Name | Commit |
|---|---|---|
| Upstream | FreeCAD 1.2.0-dev | — |
| Phase 1 | FusionCAD | `b70db7ee` |
| Phase 2 | **UniCAD** | `ed0c5def` |

Branding changes include: application title, about dialog, splash screen references, stylesheet names, README, and navigation bar label.

---

## Build Environment

| Component | Version |
|---|---|
| Base | FreeCAD 1.2.0-dev |
| CMake | 4.x |
| Generator | Ninja |
| OCCT | 7.8.1 |
| Qt | 6.8.3 |
| Python | 3.11 |
| PySide | 6.8.3 |
| Coin3D | 4.0.3 |
| VTK | 9.3.0 |
| SMESH | 9.9.0.0 (external, conda-forge) |
| Eigen | 3.4.0 |
| Package Manager | pixi (conda-forge) |
| Platform | Windows (MSVC) |

---

*Document generated from UniCAD project analysis — UNITRONIX, 2026.*
