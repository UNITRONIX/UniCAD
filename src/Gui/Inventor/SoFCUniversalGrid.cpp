// SPDX-License-Identifier: LGPL-2.1-or-later

/***************************************************************************
 *   Copyright (c) 2026 UniCAD Team                                        *
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

#include <FCConfig.h>

#include <cmath>
#include <algorithm>
#include <vector>

#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/actions/SoGetBoundingBoxAction.h>
#include <Inventor/nodes/SoBaseColor.h>
#include <Inventor/nodes/SoCoordinate3.h>
#include <Inventor/nodes/SoDrawStyle.h>
#include <Inventor/nodes/SoLightModel.h>
#include <Inventor/nodes/SoLineSet.h>
#include <Inventor/nodes/SoMaterial.h>
#include <Inventor/nodes/SoPickStyle.h>
#include <Inventor/nodes/SoPointSet.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoTranslation.h>

#include "SoFCUniversalGrid.h"


using namespace Gui;

SO_NODE_SOURCE(SoFCUniversalGrid)

void SoFCUniversalGrid::initClass()
{
    SO_NODE_INIT_CLASS(SoFCUniversalGrid, SoSeparator, "Separator");
}

void SoFCUniversalGrid::finish()
{
    atexit_cleanup();
}

SoFCUniversalGrid::SoFCUniversalGrid()
    : gridSize(10.0f)           // Minor grid every 10 units
    , subdivisions(5)           // 5 minor lines per major (so major every 50)
    , gridVisible(false)        // Disabled by default - not ready yet
    , originVisible(false)      // Disabled by default
    , currentCameraDistance(1000.0f)
    , gridExtent(500.0f)        // Fixed grid: -500 to +500
    , adaptiveLevel(0)
{
    SO_NODE_CONSTRUCTOR(SoFCUniversalGrid);
    
    // Fusion 360 Blueprint-style colors
    // Main grid lines - light blue, visible
    gridColor.setValue(0.40f, 0.60f, 0.80f);
    // Sub-grid lines - subtle but visible
    subGridColor.setValue(0.30f, 0.50f, 0.70f);
    // X axis - red
    axisXColor.setValue(0.90f, 0.30f, 0.30f);
    // Y axis - green
    axisYColor.setValue(0.30f, 0.80f, 0.40f);
    // Origin point - dark
    originColor.setValue(0.2f, 0.2f, 0.2f);
    
    // Build scene graph structure
    buildSceneGraph();
}

SoFCUniversalGrid::~SoFCUniversalGrid() = default;

void SoFCUniversalGrid::setGridSize(float size)
{
    gridSize = std::max(0.001f, size);
}

float SoFCUniversalGrid::getGridSize() const
{
    return gridSize;
}

void SoFCUniversalGrid::setSubdivisions(int subdiv)
{
    subdivisions = std::max(1, std::min(subdiv, 20));
}

int SoFCUniversalGrid::getSubdivisions() const
{
    return subdivisions;
}

void SoFCUniversalGrid::setGridVisible(bool visible)
{
    gridVisible = visible;
}

bool SoFCUniversalGrid::isGridVisible() const
{
    return gridVisible;
}

void SoFCUniversalGrid::setOriginVisible(bool visible)
{
    originVisible = visible;
}

bool SoFCUniversalGrid::isOriginVisible() const
{
    return originVisible;
}

void SoFCUniversalGrid::setGridColor(const SbColor& color)
{
    gridColor = color;
}

SbColor SoFCUniversalGrid::getGridColor() const
{
    return gridColor;
}

void SoFCUniversalGrid::setSubGridColor(const SbColor& color)
{
    subGridColor = color;
}

SbColor SoFCUniversalGrid::getSubGridColor() const
{
    return subGridColor;
}

void SoFCUniversalGrid::setAxisXColor(const SbColor& color)
{
    axisXColor = color;
}

void SoFCUniversalGrid::setAxisYColor(const SbColor& color)
{
    axisYColor = color;
    // Rebuild dynamically if needed
}

float SoFCUniversalGrid::computeAdaptiveGridSize(float /*cameraDistance*/)
{
    // Return major grid spacing (subdivisions * minor spacing)
    return gridSize * subdivisions;  // 10 * 5 = 50 units
}

void SoFCUniversalGrid::updateForCamera(float cameraDistance)
{
    currentCameraDistance = std::max(100.0f, cameraDistance);
    // Keep fixed grid extent - don't change with camera
    // gridExtent stays at 500
    rebuildGrid();
}

void SoFCUniversalGrid::getBoundingBox(SoGetBoundingBoxAction* action)
{
    // Don't include grid in bounding box calculations
    action->resetCenter();
}

void SoFCUniversalGrid::buildSceneGraph()
{
    // Disable picking on grid
    auto* pickStyle = new SoPickStyle();
    pickStyle->style = SoPickStyle::UNPICKABLE;
    addChild(pickStyle);
    
    // Disable lighting for flat colors
    auto* lightModel = new SoLightModel();
    lightModel->model = SoLightModel::BASE_COLOR;
    addChild(lightModel);
    
    // Create sub-grid separator
    pcSubGridSep = new SoSeparator();
    pcSubGridColor = new SoBaseColor();
    pcSubGridCoords = new SoCoordinate3();
    pcSubGridLines = new SoLineSet();
    
    auto* subGridStyle = new SoDrawStyle();
    subGridStyle->lineWidth = 1.0f;
    
    pcSubGridSep->addChild(subGridStyle);
    pcSubGridSep->addChild(pcSubGridColor);
    pcSubGridSep->addChild(pcSubGridCoords);
    pcSubGridSep->addChild(pcSubGridLines);
    addChild(pcSubGridSep);
    
    // Create main grid separator
    pcMainGridSep = new SoSeparator();
    pcMainGridColor = new SoBaseColor();
    pcMainGridCoords = new SoCoordinate3();
    pcMainGridLines = new SoLineSet();
    
    auto* mainGridStyle = new SoDrawStyle();
    mainGridStyle->lineWidth = 1.5f;
    
    pcMainGridSep->addChild(mainGridStyle);
    pcMainGridSep->addChild(pcMainGridColor);
    pcMainGridSep->addChild(pcMainGridCoords);
    pcMainGridSep->addChild(pcMainGridLines);
    addChild(pcMainGridSep);
    
    // Create X axis separator
    pcAxisXSep = new SoSeparator();
    pcAxisXColor = new SoBaseColor();
    pcAxisXCoords = new SoCoordinate3();
    pcAxisXLines = new SoLineSet();
    
    auto* axisXStyle = new SoDrawStyle();
    axisXStyle->lineWidth = 2.0f;
    
    pcAxisXSep->addChild(axisXStyle);
    pcAxisXSep->addChild(pcAxisXColor);
    pcAxisXSep->addChild(pcAxisXCoords);
    pcAxisXSep->addChild(pcAxisXLines);
    addChild(pcAxisXSep);
    
    // Create Y axis separator
    pcAxisYSep = new SoSeparator();
    pcAxisYColor = new SoBaseColor();
    pcAxisYCoords = new SoCoordinate3();
    pcAxisYLines = new SoLineSet();
    
    auto* axisYStyle = new SoDrawStyle();
    axisYStyle->lineWidth = 2.0f;
    
    pcAxisYSep->addChild(axisYStyle);
    pcAxisYSep->addChild(pcAxisYColor);
    pcAxisYSep->addChild(pcAxisYCoords);
    pcAxisYSep->addChild(pcAxisYLines);
    addChild(pcAxisYSep);
    
    // Create origin separator
    pcOriginSep = new SoSeparator();
    pcOriginColor = new SoBaseColor();
    pcOriginCoords = new SoCoordinate3();
    pcOriginPoints = new SoPointSet();
    
    auto* originStyle = new SoDrawStyle();
    originStyle->pointSize = 8.0f;
    
    pcOriginSep->addChild(originStyle);
    pcOriginSep->addChild(pcOriginColor);
    pcOriginSep->addChild(pcOriginCoords);
    pcOriginSep->addChild(pcOriginPoints);
    addChild(pcOriginSep);
    
    // Initial rebuild
    rebuildGrid();
}

void SoFCUniversalGrid::rebuildGrid()
{
    if (!pcSubGridCoords || !pcMainGridCoords) {
        return;
    }
    
    // Simple fixed grid: 
    // - Minor grid every 10mm, from -200 to +200
    // - Major grid every 50mm 
    float minorSpacing = 10.0f;
    float majorSpacing = 50.0f;
    float extent = 200.0f;  // Grid goes from -200 to +200
    
    // Build all grid lines
    buildSubGrid(minorSpacing, extent);
    buildMainGrid(majorSpacing, extent);
    buildAxisLines(extent);
    buildOrigin();
}

void SoFCUniversalGrid::buildSubGrid(float spacing, float extent)
{
    if (!gridVisible || spacing < 0.001f) {
        pcSubGridCoords->point.setNum(0);
        pcSubGridLines->numVertices.setNum(0);
        return;
    }
    
    pcSubGridColor->rgb.setValue(subGridColor);
    
    std::vector<SbVec3f> coords;
    std::vector<int32_t> numVerts;
    
    // Generate vertical lines (parallel to Y axis)
    for (float x = -extent; x <= extent; x += spacing) {
        // Skip major grid lines (at multiples of 50)
        if (std::fmod(std::fabs(x), 50.0f) < 0.001f) continue;
        
        coords.emplace_back(x, -extent, 0.0f);
        coords.emplace_back(x, extent, 0.0f);
        numVerts.push_back(2);
    }
    
    // Generate horizontal lines (parallel to X axis)
    for (float y = -extent; y <= extent; y += spacing) {
        // Skip major grid lines (at multiples of 50)
        if (std::fmod(std::fabs(y), 50.0f) < 0.001f) continue;
        
        coords.emplace_back(-extent, y, 0.0f);
        coords.emplace_back(extent, y, 0.0f);
        numVerts.push_back(2);
    }
    
    pcSubGridCoords->point.setNum(static_cast<int>(coords.size()));
    SbVec3f* pts = pcSubGridCoords->point.startEditing();
    for (size_t i = 0; i < coords.size(); ++i) {
        pts[i] = coords[i];
    }
    pcSubGridCoords->point.finishEditing();
    
    pcSubGridLines->numVertices.setNum(static_cast<int>(numVerts.size()));
    int32_t* nvs = pcSubGridLines->numVertices.startEditing();
    for (size_t i = 0; i < numVerts.size(); ++i) {
        nvs[i] = numVerts[i];
    }
    pcSubGridLines->numVertices.finishEditing();
}

void SoFCUniversalGrid::buildMainGrid(float spacing, float extent)
{
    if (!gridVisible || spacing < 0.001f) {
        pcMainGridCoords->point.setNum(0);
        pcMainGridLines->numVertices.setNum(0);
        return;
    }
    
    pcMainGridColor->rgb.setValue(gridColor);
    
    std::vector<SbVec3f> coords;
    std::vector<int32_t> numVerts;
    
    // Generate vertical lines (parallel to Y axis)
    for (float x = -extent; x <= extent; x += spacing) {
        // Skip origin (drawn as axis)
        if (std::fabs(x) < 0.001f) continue;
        
        coords.emplace_back(x, -extent, 0.0f);
        coords.emplace_back(x, extent, 0.0f);
        numVerts.push_back(2);
    }
    
    // Generate horizontal lines (parallel to X axis)
    for (float y = -extent; y <= extent; y += spacing) {
        // Skip origin (drawn as axis)
        if (std::fabs(y) < 0.001f) continue;
        
        coords.emplace_back(-extent, y, 0.0f);
        coords.emplace_back(extent, y, 0.0f);
        numVerts.push_back(2);
    }
    
    pcMainGridCoords->point.setNum(static_cast<int>(coords.size()));
    SbVec3f* pts = pcMainGridCoords->point.startEditing();
    for (size_t i = 0; i < coords.size(); ++i) {
        pts[i] = coords[i];
    }
    pcMainGridCoords->point.finishEditing();
    
    pcMainGridLines->numVertices.setNum(static_cast<int>(numVerts.size()));
    int32_t* nvs = pcMainGridLines->numVertices.startEditing();
    for (size_t i = 0; i < numVerts.size(); ++i) {
        nvs[i] = numVerts[i];
    }
    pcMainGridLines->numVertices.finishEditing();
}

void SoFCUniversalGrid::buildAxisLines(float halfExtent)
{
    if (!gridVisible) {
        pcAxisXCoords->point.setNum(0);
        pcAxisXLines->numVertices.setNum(0);
        pcAxisYCoords->point.setNum(0);
        pcAxisYLines->numVertices.setNum(0);
        return;
    }
    
    // X axis
    pcAxisXColor->rgb.setValue(axisXColor);
    pcAxisXCoords->point.setNum(2);
    SbVec3f* xpts = pcAxisXCoords->point.startEditing();
    xpts[0] = SbVec3f(-halfExtent, 0.0f, 0.0f);
    xpts[1] = SbVec3f(halfExtent, 0.0f, 0.0f);
    pcAxisXCoords->point.finishEditing();
    
    pcAxisXLines->numVertices.setNum(1);
    pcAxisXLines->numVertices.set1Value(0, 2);
    
    // Y axis
    pcAxisYColor->rgb.setValue(axisYColor);
    pcAxisYCoords->point.setNum(2);
    SbVec3f* ypts = pcAxisYCoords->point.startEditing();
    ypts[0] = SbVec3f(0.0f, -halfExtent, 0.0f);
    ypts[1] = SbVec3f(0.0f, halfExtent, 0.0f);
    pcAxisYCoords->point.finishEditing();
    
    pcAxisYLines->numVertices.setNum(1);
    pcAxisYLines->numVertices.set1Value(0, 2);
}

void SoFCUniversalGrid::buildOrigin()
{
    if (!originVisible) {
        pcOriginCoords->point.setNum(0);
        return;
    }
    
    // Dark origin point
    pcOriginColor->rgb.setValue(0.1f, 0.1f, 0.1f);
    pcOriginCoords->point.setNum(1);
    pcOriginCoords->point.set1Value(0, SbVec3f(0.0f, 0.0f, 0.01f));
}
