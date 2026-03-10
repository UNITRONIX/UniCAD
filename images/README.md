<p align="center">
  <img src="/unicad_icon.png" height="120px" width="120px" alt="UniCAD Logo">
</p>

<h1 align="center">UniCAD by UNITRONIX</h1>

<p align="center">
  <strong>Open-source parametric 3D CAD with Fusion 360-style workflow</strong><br>
  A fork of <a href="https://github.com/FreeCAD/FreeCAD">FreeCAD</a> focused on modern, intuitive modeling experience.
</p>

<p align="center">
  <a href="https://github.com/UNITRONIX/UniCAD/releases/latest"><img src="https://img.shields.io/github/v/release/UNITRONIX/UniCAD?style=flat-square&label=Release" alt="Release"></a>
  <a href="https://github.com/UNITRONIX/UniCAD/blob/main/LICENSE"><img src="https://img.shields.io/badge/License-LGPL_2.1-blue?style=flat-square" alt="License"></a>
  <a href="https://github.com/UNITRONIX/UniCAD/issues"><img src="https://img.shields.io/github/issues/UNITRONIX/UniCAD?style=flat-square" alt="Issues"></a>
</p>

---

<p align="center">
  <img src="images/unicad-sketch-tab.png" width="900" alt="UniCAD Fusion 360-style interface"/>
</p>

## What is UniCAD?

UniCAD is a fork of [FreeCAD](https://www.freecad.org) that brings **Fusion 360-style unified modeling** to an open-source parametric CAD environment. Instead of separate additive/subtractive commands, UniCAD offers single commands with an **Operation** selector (Join / Cut / Intersect / New Body), making the workflow faster and more intuitive.

Built on the solid foundation of FreeCAD 1.2.0-dev, it retains full compatibility with FreeCAD projects while adding modern interaction patterns.

## Screenshots

### Fusion 360-Style Unified Tab Toolbar

UniCAD features a modern tabbed toolbar inspired by Fusion 360, with unified tabs that organize tools logically regardless of workbench:

| Tab | Description |
|-----|-------------|
| **SKETCH** | All 2D sketching tools: lines, rectangles, circles, arcs, constraints |
| **SOLID** | 3D modeling operations: Extrude, Revolve, Boolean, Patterns |
| **SURFACE** | Surface modeling tools for complex organic shapes |
| **SHEET METAL** | Sheet metal design tools (when available) |
| **MESH** | Mesh import, export, and manipulation |
| **INSPECT** | Measurement and analysis tools |
| **TOOLS** | File operations, settings, macros |

<p align="center">
  <img src="images/unicad-surface-tab.png" width="800" alt="SURFACE tab with surface tools"/>
  <br><em>SURFACE tab - Surface modeling tools</em>
</p>

<p align="center">
  <img src="images/unicad-mesh-tab.png" width="800" alt="MESH tab with mesh tools"/>
  <br><em>MESH tab - Mesh manipulation tools</em>
</p>

<p align="center">
  <img src="images/unicad-tools-tab.png" width="800" alt="TOOLS tab"/>
  <br><em>TOOLS tab - File operations, settings, and macros</em>
</p>

### Sketch Editing with Grid

<p align="center">
  <img src="images/unicad-sketch-editing.png" width="800" alt="Sketch editing mode with grid"/>
  <br><em>Sketch editing mode with visible grid and constraint panel</em>
</p>

## Key Features

### Unified Modeling Commands

| Command | Shortcut | Description |
|---------|----------|-------------|
| **Extrude** | `E` | Unified Pad + Pocket. One command with Join/Cut/Intersect/NewBody selector |
| **Revolve** | `R` | Unified Revolution + Groove |
| **Sweep** | `Shift+S` | Unified Additive/Subtractive Pipe |
| **Loft** | `L` | Unified Additive/Subtractive Loft |

All four commands support the same **Operation** property - no need to remember separate commands for adding or removing material.

### Smart PressPull (`Q`)

Adaptive meta-command inspired by Fusion 360:
- **Sketch selected** -> launches Extrude
- **Face selected** -> launches OffsetFace
- **No selection** -> launches OffsetFace for interactive face picking

### Auto-Switch Join â†” Cut

When dragging the extrude gizmo through zero, the operation **automatically switches** between Join and Cut â€” just like in Fusion 360. Drag up to add material, drag below the surface to cut.

### Face Manipulation Tools

| Command | Shortcut | Description |
|---------|----------|-------------|
| **OffsetFace** | â€” | Offset selected faces by a distance |
| **DeleteFace** | `Shift+D` | Delete faces and heal the solid |
| **ReplaceFace** | `Shift+R` | Replace faces with a target surface |
| **SplitFace** | `Shift+S` | Split faces with a tool shape |
| **MoveFace** | `M` | Translate/rotate selected faces |

### Additional Improvements

- **Fusion 360-style unified tab toolbar** — SKETCH, SOLID, SURFACE, MESH, INSPECT, TOOLS tabs
- **Dark theme UI** with blue accent colors inspired by Fusion 360
- **Grid visible by default** in Sketch mode
- **Dynamic command loading** — toolbar adapts to available workbenches
- **Fusion 360-inspired keyboard shortcuts** for common operations
- **Sketch face rendering** — closed sketch wires display as filled faces in 3D
- **Individual face selection** for direct editing workflows
- Cross-platform: **Windows**, **macOS**, **Linux**

## Underlying Technology

| Component | Description |
|-----------|-------------|
| **OpenCASCADE (OCCT 7.8+)** | Geometry kernel â€” boolean ops, BREP, face manipulation |
| **Coin3D** | Open Inventor-compliant 3D scene graph |
| **Qt 6.8+** | GUI framework |
| **Python 3.11+** | Scripting and automation API |
| **PySide 6** | Python-Qt bindings |

---

## Building from Source

### Prerequisites

UniCAD uses [pixi](https://pixi.sh) for dependency management. Install pixi first:

```powershell
# Windows (PowerShell)
iwr -useb https://pixi.sh/install.ps1 | iex
```

```bash
# Linux / macOS
curl -fsSL https://pixi.sh/install.sh | bash
```

### Clone & Configure

```bash
git clone https://github.com/UNITRONIX/UniCAD.git
cd UniCAD
pixi run -e default cmake --preset debug
```

### Build

```bash
# Full build (all cores)
pixi run -e default cmake --build build/debug

# Faster: build only specific targets
pixi run -e default cmake --build build/debug --target PartDesignGui -- -j 4
```

### Install & Run

```bash
pixi run -e default cmake --install build/debug
pixi run -e default freecad
```

### Build Presets

| Preset | Description |
|--------|-------------|
| `debug` | Debug build with full symbols (default for development) |
| `release` | Optimized release build |

### CMake Configure Options

Standard FreeCAD CMake options apply. Refer to the [FreeCAD Developer Handbook](https://freecad.github.io/DevelopersHandbook/gettingstarted/) for platform-specific details.

---

## Project Structure

```
UniCAD/
â”śâ”€â”€ src/
â”‚   â”śâ”€â”€ App/                    # Application core
â”‚   â”śâ”€â”€ Base/                   # Foundation classes
â”‚   â”śâ”€â”€ Gui/                    # Main GUI (Fusion UI components here)
â”‚   â”śâ”€â”€ Main/                   # Entry points (MainGui, MainCmd)
â”‚   â””â”€â”€ Mod/
â”‚       â””â”€â”€ PartDesign/
â”‚           â”śâ”€â”€ App/            # Feature classes (FeatureUnified*, Feature*Face)
â”‚           â””â”€â”€ Gui/            # Task panels, view providers, commands
â”śâ”€â”€ cMake/                      # CMake modules and helpers
â”śâ”€â”€ pixi.toml                   # Dependency management
â”śâ”€â”€ CMakeLists.txt              # Top-level build config
â””â”€â”€ CMakePresets.json           # Build presets (debug/release)
```

## Reporting Issues

1. Search [existing issues](https://github.com/UNITRONIX/UniCAD/issues) for duplicates
2. Use the latest build
3. Include version info from `Help > About UniCAD > Copy to clipboard`
4. Provide step-by-step reproduction instructions
5. Attach example files (`.FCStd` as ZIP) when relevant

## Contributing

Contributions are welcome! See [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

UniCAD follows the same coding standards and contribution process as FreeCAD.

## License

UniCAD is licensed under the [LGPL 2.1+](LICENSE) license. It is a fork of [FreeCAD](https://github.com/FreeCAD/FreeCAD), and all original FreeCAD code retains its original license.

## Acknowledgments

UniCAD would not be possible without the [FreeCAD](https://www.freecad.org) project and its contributors. We are grateful for their work on the open-source CAD ecosystem.

### đź¤– Built with AI

This project is developed with the assistance of **AI (GitHub Copilot / Claude)**. AI is used for code generation, architecture design, feature implementation, and code review. All AI-generated code is reviewed and tested by the UNITRONIX team before being merged.

---

<p align="center">
  <sub>Made with âť¤ď¸Ź by <a href="https://github.com/UNITRONIX">UNITRONIX</a> &amp; AI</sub>
</p>
