/***************************************************************************
 *   Copyright (c) 2026 UNITRONIX                                          *
 *   UniCAD - A fork of FreeCAD                                            *
 *                                                                         *
 *   This file is part of UniCAD.                                          *
 *                                                                         *
 *   UniCAD is free software; you can redistribute it and/or modify        *
 *   it under the terms of the GNU Lesser General Public License (LGPL)    *
 *   as published by the Free Software Foundation; either version 2.1 of   *
 *   the License, or (at your option) any later version.                   *
 ***************************************************************************/

#ifndef GUI_VIEWPORTSTYLEMANAGER_H
#define GUI_VIEWPORTSTYLEMANAGER_H

#include <string>
#include <vector>

#include <FCGlobal.h>

namespace Gui
{

class View3DInventorViewer;

/**
 * Applies Shapr3D-inspired viewport presets (background, lighting,
 * default materials/edges, ground grid, SSAO/outline flags).
 *
 * Active style is stored in:
 *   User parameter:BaseApp/Preferences/View/ViewportStyle
 * Default: "ShaprDark"
 */
class GuiExport ViewportStyleManager
{
public:
    static constexpr const char* StyleShaprDark = "ShaprDark";
    static constexpr const char* StyleShaprLight = "ShaprLight";
    static constexpr const char* StyleStudio = "Studio";

    static ViewportStyleManager& instance();

    /// Known preset ids
    static std::vector<std::string> availablePresets();

    /// Human-readable label for UI
    static const char* presetLabel(const std::string& id);

    /// Currently stored style id (defaults to ShaprDark)
    std::string activePreset() const;

    /// Write preferences and apply to all open 3D viewers
    void applyPreset(const std::string& id);

    /// Apply active preference values (background/lights already in View params)
    /// plus grid and post-FX to all open viewers
    void applyPreferencesToViewers() const;

    /// Apply active preset to a single viewer (grid colors, post-FX flags)
    void applyToViewer(View3DInventorViewer* viewer) const;

    /// Ensure a default style exists and apply it once at startup
    void ensureDefaultApplied();

    /// Render Studio mode: SSAO, outline, ground plane, three-point lights.
    /// Enabling saves current prefs so disabling can restore them.
    bool isStudioModeActive() const;
    void applyStudioMode(bool enable);

    /// Toggle ground plane (opaque XY plane + optional grid) across viewers
    void setGroundPlaneVisible(bool visible);
    bool isGroundPlaneVisible() const;

    /// Toggle SSAO / edge outline and push to viewers
    void setSSAOEnabled(bool enabled);
    void setEdgeOutlineEnabled(bool enabled);

private:
    ViewportStyleManager() = default;

    void writePresetPreferences(const std::string& id) const;
    void saveStudioBackup() const;
    void restoreStudioBackup() const;
    void writeStudioPreferences() const;
};

}  // namespace Gui

#endif  // GUI_VIEWPORTSTYLEMANAGER_H
