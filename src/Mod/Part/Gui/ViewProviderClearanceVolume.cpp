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

#include "PreCompiled.h"

#include "ViewProviderClearanceVolume.h"

#include <Mod/Part/App/FeatureClearanceVolume.h>

#include <Gui/BitmapFactory.h>

#include <QMenu>

using namespace PartGui;

PROPERTY_SOURCE(PartGui::ViewProviderClearanceVolume, PartGui::ViewProviderPartExt)

ViewProviderClearanceVolume::ViewProviderClearanceVolume()
{
    // Default transparency - semi-transparent
    ADD_PROPERTY_TYPE(Transparency, (70), 
                      "Display", App::Prop_None,
                      "Transparency level (0=opaque, 100=invisible)");
    
    // Show outline
    ADD_PROPERTY_TYPE(ShowOutline, (true), 
                      "Display", App::Prop_None,
                      "Show outline of clearance volume");
    
    // Outline width
    ADD_PROPERTY_TYPE(OutlineWidth, (2.0f), 
                      "Display", App::Prop_None,
                      "Width of outline lines");
    
    // Set default color (orange/amber for clearance zones) - RGBA format
    const unsigned int orangeColor = 0xFF990080; // Orange with some transparency
    ShapeAppearance.setDiffuseColor(orangeColor);
    
    // Make it transparent by default
    ViewProviderPartExt::Transparency.setValue(70);
    
    // Different selection style
    SelectionStyle.setValue(1); // Bounding box selection
}

ViewProviderClearanceVolume::~ViewProviderClearanceVolume()
{
}

Part::ClearanceVolume* ViewProviderClearanceVolume::getObject() const
{
    return dynamic_cast<Part::ClearanceVolume*>(pcObject);
}

void ViewProviderClearanceVolume::attach(App::DocumentObject* obj)
{
    ViewProviderPartExt::attach(obj);
    applyVisualizationSettings();
}

void ViewProviderClearanceVolume::updateData(const App::Property* prop)
{
    ViewProviderPartExt::updateData(prop);
    
    auto* clearance = getObject();
    if (!clearance) {
        return;
    }
    
    // Update color from feature property
    if (prop == &clearance->DisplayColor) {
        const Base::Color& color = clearance->DisplayColor.getValue();
        ShapeAppearance.setDiffuseColor(color.getPackedValue());
    }
    
    // Update on any geometry change - Shape is inherited from Part::Feature
    if (strcmp(prop->getName(), "Shape") == 0) {
        applyVisualizationSettings();
    }
}

std::vector<std::string> ViewProviderClearanceVolume::getDisplayModes() const
{
    std::vector<std::string> modes = ViewProviderPartExt::getDisplayModes();
    modes.push_back("Clearance Zone");
    return modes;
}

void ViewProviderClearanceVolume::setDisplayMode(const char* ModeName)
{
    if (strcmp(ModeName, "Clearance Zone") == 0) {
        // Custom mode - semi-transparent with outline
        ViewProviderPartExt::Transparency.setValue(Transparency.getValue());
        applyVisualizationSettings();
    }
    
    ViewProviderPartExt::setDisplayMode(ModeName);
}

void ViewProviderClearanceVolume::setupContextMenu(QMenu* menu, QObject* receiver, const char* member)
{
    ViewProviderPartExt::setupContextMenu(menu, receiver, member);
    
    menu->addSeparator();
    
    QAction* editAction = menu->addAction(QObject::tr("Edit Clearance..."));
    QObject::connect(editAction, SIGNAL(triggered()), receiver, member);
    
    QAction* applyAction = menu->addAction(QObject::tr("Apply to Selection"));
    QObject::connect(applyAction, SIGNAL(triggered()), receiver, member);
}

QIcon ViewProviderClearanceVolume::getIcon() const
{
    // Use a distinctive icon for clearance volumes
    return Gui::BitmapFactory().iconFromTheme("Part_Offset3D");
}

void ViewProviderClearanceVolume::applyVisualizationSettings()
{
    // Apply transparency
    int trans = Transparency.getValue();
    ViewProviderPartExt::Transparency.setValue(trans);
    
    // Apply outline if enabled
    if (ShowOutline.getValue()) {
        createOutlineVisualization();
    }
}

void ViewProviderClearanceVolume::createOutlineVisualization()
{
    // The outline is already part of the shape visualization
    // This could be extended to add additional edge highlighting
    
    // Set line width for edges
    LineWidth.setValue(OutlineWidth.getValue());
    
    // Use a different line color for outline (cyan to contrast with orange)
    const unsigned int cyanColor = 0x00CCFFFF; // Cyan
    LineColor.setValue(cyanColor);
}
