/***************************************************************************
 *   Copyright (c) 2026 UniCAD Project                                     *
 *                                                                         *
 *   This file is part of UniCAD.                                          *
 *                                                                         *
 *   UniCAD is free software: you can redistribute it and/or modify it     *
 *   under the terms of the GNU Lesser General Public License as           *
 *   published by the Free Software Foundation, either version 2.1 of the  *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   UniCAD is distributed in the hope that it will be useful, but         *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU      *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with UniCAD. If not, see                                *
 *   <https://www.gnu.org/licenses/>.                                      *
 ***************************************************************************/

#ifndef PARTGUI_VIEWPROVIDER_CLEARANCE_VOLUME_H
#define PARTGUI_VIEWPROVIDER_CLEARANCE_VOLUME_H

#include <Mod/Part/PartGlobal.h>
#include <Mod/Part/App/FeatureClearanceVolume.h>
#include "ViewProviderExt.h"

namespace PartGui
{

/**
 * @brief ViewProvider for ClearanceVolume - displays as semi-transparent "ghost" shape
 * 
 * This ViewProvider renders the clearance volume with:
 * - Semi-transparent fill (configurable alpha)
 * - Distinctive outline color  
 * - Optional dashed edges
 * - "Virtual" label in 3D view
 * 
 * The volume is clearly distinguishable from real geometry while being
 * visible enough to show the keep-out zone for ports/connectors.
 */
class PartGuiExport ViewProviderClearanceVolume : public ViewProviderPartExt
{
    PROPERTY_HEADER_WITH_OVERRIDE(PartGui::ViewProviderClearanceVolume);

public:
    ViewProviderClearanceVolume();
    ~ViewProviderClearanceVolume() override;
    
    // Properties for clearance visualization
    App::PropertyPercent Transparency;
    App::PropertyBool ShowOutline;
    App::PropertyFloat OutlineWidth;
    
    /// Get the feature
    Part::ClearanceVolume* getObject() const;
    
    /// Update visual from feature properties
    void updateData(const App::Property* prop) override;
    
    /// Attach to object
    void attach(App::DocumentObject* obj) override;
    
    /// Get display modes
    std::vector<std::string> getDisplayModes() const override;
    
    /// Set display mode
    void setDisplayMode(const char* ModeName) override;
    
    /// Context menu
    void setupContextMenu(QMenu* menu, QObject* receiver, const char* member) override;
    
    /// Icon
    QIcon getIcon() const override;
    
    /// Don't include in STL export by default
    bool canExportToSTL() const { return false; }

protected:
    /// Apply transparency and color settings
    void applyVisualizationSettings();
    
    /// Create outline visualization
    void createOutlineVisualization();
};

} // namespace PartGui

#endif // PARTGUI_VIEWPROVIDER_CLEARANCE_VOLUME_H
