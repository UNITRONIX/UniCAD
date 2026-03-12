// SPDX-License-Identifier: LGPL-2.1-or-later

/***************************************************************************
 *   Copyright (c) 2024 UniCAD Team                                        *
 *                                                                         *
 *   This file is part of the UniCAD CAx development system.               *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This library  is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this library; see the file COPYING.LIB. If not,    *
 *   write to the Free Software Foundation, Inc., 59 Temple Place,         *
 *   Suite 330, Boston, MA  02111-1307, USA                                *
 *                                                                         *
 ***************************************************************************/

#ifndef GUI_SOFCUNIVERSALGRID_H
#define GUI_SOFCUNIVERSALGRID_H

#include <Inventor/SbColor.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoSubNode.h>
#include <FCGlobal.h>


class SoBaseColor;
class SoCoordinate3;
class SoGetBoundingBoxAction;
class SoLineSet;
class SoPointSet;

namespace Gui
{

/**
 * @brief Universal 3D grid visible in both 3D view and sketch mode
 * 
 * This node renders a Fusion 360-style infinite grid on the XY plane
 * with graduation marks, adaptive spacing, and origin marker.
 */
class GuiExport SoFCUniversalGrid : public SoSeparator
{
    using inherited = SoSeparator;

    SO_NODE_HEADER(Gui::SoFCUniversalGrid);

public:
    static void initClass();
    static void finish();
    
    SoFCUniversalGrid();
    
    void getBoundingBox(SoGetBoundingBoxAction* action) override;
    
    // Grid configuration
    void setGridSize(float size);
    float getGridSize() const;
    
    void setSubdivisions(int subdiv);
    int getSubdivisions() const;
    
    void setGridVisible(bool visible);
    bool isGridVisible() const;
    
    void setOriginVisible(bool visible);
    bool isOriginVisible() const;
    
    void setGridColor(const SbColor& color);
    SbColor getGridColor() const;
    
    void setSubGridColor(const SbColor& color);
    SbColor getSubGridColor() const;
    
    void setAxisXColor(const SbColor& color);
    void setAxisYColor(const SbColor& color);
    
    // Update grid based on camera zoom
    void updateForCamera(float cameraDistance);

protected:
    ~SoFCUniversalGrid() override;

private:
    void buildSceneGraph();
    void rebuildGrid();
    void buildSubGrid(float subGridSize, float halfExtent);
    void buildMainGrid(float adaptiveGridSize, float halfExtent);
    void buildAxisLines(float halfExtent);
    void buildOrigin();
    
    float computeAdaptiveGridSize(float cameraDistance);
    
    // Grid state
    float gridSize;
    int subdivisions;
    bool gridVisible;
    bool originVisible;
    float currentCameraDistance;
    
    // Colors
    SbColor gridColor;
    SbColor subGridColor;
    SbColor axisXColor;
    SbColor axisYColor;
    SbColor originColor;
    
    // Grid extent (computed from camera)
    float gridExtent;
    
    // Adaptive grid level
    int adaptiveLevel;
    
    // Coin3D nodes for sub-grid
    SoSeparator* pcSubGridSep = nullptr;
    SoBaseColor* pcSubGridColor = nullptr;
    SoCoordinate3* pcSubGridCoords = nullptr;
    SoLineSet* pcSubGridLines = nullptr;
    
    // Coin3D nodes for main grid
    SoSeparator* pcMainGridSep = nullptr;
    SoBaseColor* pcMainGridColor = nullptr;
    SoCoordinate3* pcMainGridCoords = nullptr;
    SoLineSet* pcMainGridLines = nullptr;
    
    // Coin3D nodes for X axis
    SoSeparator* pcAxisXSep = nullptr;
    SoBaseColor* pcAxisXColor = nullptr;
    SoCoordinate3* pcAxisXCoords = nullptr;
    SoLineSet* pcAxisXLines = nullptr;
    
    // Coin3D nodes for Y axis
    SoSeparator* pcAxisYSep = nullptr;
    SoBaseColor* pcAxisYColor = nullptr;
    SoCoordinate3* pcAxisYCoords = nullptr;
    SoLineSet* pcAxisYLines = nullptr;
    
    // Coin3D nodes for origin point
    SoSeparator* pcOriginSep = nullptr;
    SoBaseColor* pcOriginColor = nullptr;
    SoCoordinate3* pcOriginCoords = nullptr;
    SoPointSet* pcOriginPoints = nullptr;
};

}  // namespace Gui

#endif  // GUI_SOFCUNIVERSALGRID_H
