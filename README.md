<p align="center">
  <img src="/src/Gui/Icons/freecad.svg" height="120px" width="120px" alt="FusionCAD Logo">
</p>

<h1 align="center">FusionCAD by UNITRONIX</h1>

<p align="center">
  <strong>Open-source parametric 3D CAD with Fusion 360-style workflow</strong><br>
  A fork of <a href="https://github.com/FreeCAD/FreeCAD">FreeCAD</a> focused on modern, intuitive modeling experience.
</p>

<p align="center">
  <a href="https://github.com/UNITRONIX/FusionCAD/releases/latest"><img src="https://img.shields.io/github/v/release/UNITRONIX/FusionCAD?style=flat-square&label=Release" alt="Release"></a>
  <a href="https://github.com/UNITRONIX/FusionCAD/blob/main/LICENSE"><img src="https://img.shields.io/badge/License-LGPL_2.1-blue?style=flat-square" alt="License"></a>
  <a href="https://github.com/UNITRONIX/FusionCAD/issues"><img src="https://img.shields.io/github/issues/UNITRONIX/FusionCAD?style=flat-square" alt="Issues"></a>
</p>

---

<p align="center">
  <img src="/.github/images/partdesign.png" width="800" alt="FusionCAD PartDesign workbench screenshot"/>
</p>

## What is FusionCAD?

FusionCAD is a fork of [FreeCAD](https://www.freecad.org) that brings **Fusion 360-style unified modeling** to an open-source parametric CAD environment. Instead of separate additive/subtractive commands, FusionCAD offers single commands with an **Operation** selector (Join / Cut / Intersect / New Body), making the workflow faster and more intuitive.

Built on the solid foundation of FreeCAD 1.2.0-dev, it retains full compatibility with FreeCAD projects while adding modern interaction patterns.

## Key Features

### Unified Modeling Commands

| Command | Shortcut | Description |
|---------|----------|-------------|
| **Extrude** | `E` | Unified Pad + Pocket. One command with Join/Cut/Intersect/NewBody selector |
| **Revolve** | `R` | Unified Revolution + Groove |
| **Sweep** | `Shift+S` | Unified Additive/Subtractive Pipe |
| **Loft** | `L` | Unified Additive/Subtractive Loft |

All four commands support the same **Operation** property — no need to remember separate commands for adding or removing material.

### Smart PressPull (`Q`)

Adaptive meta-command inspired by Fusion 360:
- **Sketch selected** → launches Extrude
- **Face selected** → launches OffsetFace
- **No selection** → launches OffsetFace for interactive face picking

### Auto-Switch Join ↔ Cut

When dragging the extrude gizmo through zero, the operation **automatically switches** between Join and Cut — just like in Fusion 360. Drag up to add material, drag below the surface to cut.

### Face Manipulation Tools

| Command | Shortcut | Description |
|---------|----------|-------------|
| **OffsetFace** | — | Offset selected faces by a distance |
| **DeleteFace** | `Shift+D` | Delete faces and heal the solid |
| **ReplaceFace** | `Shift+R` | Replace faces with a target surface |
| **SplitFace** | `Shift+S` | Split faces with a tool shape |
| **MoveFace** | `M` | Translate/rotate selected faces |

### Additional Improvements

- **Fusion 360-inspired keyboard shortcuts** for common operations
- **Sketch face rendering** — closed sketch wires display as filled faces in 3D
- **Individual face selection** for direct editing workflows
- Cross-platform: **Windows**, **macOS**, **Linux**

## Underlying Technology

| Component | Description |
|-----------|-------------|
| **OpenCASCADE (OCCT 7.8+)** | Geometry kernel — boolean ops, BREP, face manipulation |
| **Coin3D** | Open Inventor-compliant 3D scene graph |
| **Qt 6.8+** | GUI framework |
| **Python 3.11+** | Scripting and automation API |
| **PySide 6** | Python-Qt bindings |

---

## Building from Source

### Prerequisites

FusionCAD uses [pixi](https://pixi.sh) for dependency management. Install pixi first:

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
git clone https://github.com/UNITRONIX/FusionCAD.git
cd FusionCAD
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
FusionCAD/
├── src/
│   ├── App/                    # Application core
│   ├── Base/                   # Foundation classes
│   ├── Gui/                    # Main GUI (Fusion UI components here)
│   ├── Main/                   # Entry points (MainGui, MainCmd)
│   └── Mod/
│       └── PartDesign/
│           ├── App/            # Feature classes (FeatureUnified*, Feature*Face)
│           └── Gui/            # Task panels, view providers, commands
├── cMake/                      # CMake modules and helpers
├── pixi.toml                   # Dependency management
├── CMakeLists.txt              # Top-level build config
└── CMakePresets.json           # Build presets (debug/release)
```

## Reporting Issues

1. Search [existing issues](https://github.com/UNITRONIX/FusionCAD/issues) for duplicates
2. Use the latest build
3. Include version info from `Help > About FusionCAD > Copy to clipboard`
4. Provide step-by-step reproduction instructions
5. Attach example files (`.FCStd` as ZIP) when relevant

## Contributing

Contributions are welcome! See [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

FusionCAD follows the same coding standards and contribution process as FreeCAD.

## License

FusionCAD is licensed under the [LGPL 2.1+](LICENSE) license. It is a fork of [FreeCAD](https://github.com/FreeCAD/FreeCAD), and all original FreeCAD code retains its original license.

## Acknowledgments

FusionCAD would not be possible without the [FreeCAD](https://www.freecad.org) project and its contributors. We are grateful for their work on the open-source CAD ecosystem.

---

<p align="center">
  <sub>Made with ❤️ by <a href="https://github.com/UNITRONIX">UNITRONIX</a></sub>
</p>
