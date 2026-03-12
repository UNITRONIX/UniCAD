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

#include "SnapPointVisualizer.h"

#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoSwitch.h>
#include <Inventor/nodes/SoCoordinate3.h>
#include <Inventor/nodes/SoMaterial.h>
#include <Inventor/nodes/SoDrawStyle.h>
#include <Inventor/nodes/SoMarkerSet.h>
#include <Inventor/nodes/SoLineSet.h>
#include <Inventor/nodes/SoAnnotation.h>
#include <Inventor/nodes/SoFont.h>
#include <Inventor/nodes/SoText2.h>
#include <Inventor/nodes/SoTranslation.h>
#include <Inventor/nodes/SoPointSet.h>
#include <Inventor/nodes/SoPickStyle.h>

#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace PartGui {

SnapPointVisualizer::SnapPointVisualizer()
{
    initializeSceneGraph();
}

SnapPointVisualizer::~SnapPointVisualizer()
{
    if (m_root) {
        m_root->unref();
    }
}

void SnapPointVisualizer::initializeSceneGraph()
{
    // Create root node
    m_root = new SoSeparator();
    m_root->ref();
    m_root->setName("SnapPointVisualizer");
    
    // Create switch for visibility control
    m_switch = new SoSwitch();
    m_switch->whichChild = SO_SWITCH_NONE; // Start hidden
    m_root->addChild(m_switch);
    
    // Make markers non-pickable so they don't interfere with selection
    auto* pickStyle = new SoPickStyle();
    pickStyle->style = SoPickStyle::UNPICKABLE;
    m_switch->addChild(pickStyle);
    
    // Create marker group
    m_markerGroup = new SoSeparator();
    m_switch->addChild(m_markerGroup);
    
    // Translation for positioning
    m_translation = new SoTranslation();
    m_markerGroup->addChild(m_translation);
    
    // Material
    m_material = new SoMaterial();
    m_material->diffuseColor.setValue(m_config.markerColor[0],
                                       m_config.markerColor[1],
                                       m_config.markerColor[2]);
    m_material->emissiveColor.setValue(m_config.markerColor[0],
                                        m_config.markerColor[1],
                                        m_config.markerColor[2]);
    m_markerGroup->addChild(m_material);
    
    // Draw style
    m_drawStyle = new SoDrawStyle();
    m_drawStyle->lineWidth = m_config.lineWidth;
    m_drawStyle->pointSize = m_config.markerSize;
    m_markerGroup->addChild(m_drawStyle);
    
    // Coordinates for geometry
    m_coords = new SoCoordinate3();
    m_markerGroup->addChild(m_coords);
    
    // Create marker geometry based on style
    createMarkerGeometry();
    
    // Create label group
    m_labelGroup = new SoSeparator();
    m_switch->addChild(m_labelGroup);
    
    m_labelTranslation = new SoTranslation();
    m_labelGroup->addChild(m_labelTranslation);
    
    m_labelMaterial = new SoMaterial();
    m_labelMaterial->diffuseColor.setValue(1.0f, 1.0f, 1.0f);
    m_labelGroup->addChild(m_labelMaterial);
    
    m_font = new SoFont();
    m_font->size = 12;
    m_font->name = "Arial";
    m_labelGroup->addChild(m_font);
    
    m_labelText = new SoText2();
    m_labelGroup->addChild(m_labelText);
}

void SnapPointVisualizer::createMarkerGeometry()
{
    switch (m_config.style) {
        case SnapMarkerStyle::CrossHair:
            createCrosshair(m_config.crosshairSize);
            break;
            
        case SnapMarkerStyle::Circle:
            createCircleMarker(m_config.markerSize / 2.0f);
            break;
            
        case SnapMarkerStyle::CircleWithDot:
        default:
            // Create circle with center dot (Fusion 360 style)
            createCircleMarker(m_config.markerSize / 2.0f);
            // Add center point
            {
                auto* centerPoint = new SoCoordinate3();
                centerPoint->point.setNum(1);
                centerPoint->point.set1Value(0, 0, 0, 0);
                m_markerGroup->addChild(centerPoint);
                
                auto* pointSet = new SoPointSet();
                m_markerGroup->addChild(pointSet);
            }
            break;
            
        case SnapMarkerStyle::Diamond:
            // Create diamond shape
            {
                float s = m_config.markerSize / 2.0f;
                m_coords->point.setNum(5);
                m_coords->point.set1Value(0, 0, s, 0);    // Top
                m_coords->point.set1Value(1, s, 0, 0);    // Right
                m_coords->point.set1Value(2, 0, -s, 0);   // Bottom
                m_coords->point.set1Value(3, -s, 0, 0);   // Left
                m_coords->point.set1Value(4, 0, s, 0);    // Back to top
                
                m_lines = new SoLineSet();
                m_lines->numVertices.setNum(1);
                m_lines->numVertices.set1Value(0, 5);
                m_markerGroup->addChild(m_lines);
            }
            break;
            
        case SnapMarkerStyle::Target:
            // Create target/bullseye (circle + crosshair)
            createCircleMarker(m_config.markerSize / 2.0f);
            createCrosshair(m_config.crosshairSize);
            break;
    }
}

void SnapPointVisualizer::createCrosshair(float size)
{
    float half = size / 2.0f;
    
    // Horizontal line
    int baseIndex = m_coords->point.getNum();
    m_coords->point.setNum(baseIndex + 4);
    m_coords->point.set1Value(baseIndex + 0, -half, 0, 0);
    m_coords->point.set1Value(baseIndex + 1, half, 0, 0);
    m_coords->point.set1Value(baseIndex + 2, 0, -half, 0);
    m_coords->point.set1Value(baseIndex + 3, 0, half, 0);
    
    if (!m_lines) {
        m_lines = new SoLineSet();
        m_markerGroup->addChild(m_lines);
    }
    
    int numLines = m_lines->numVertices.getNum();
    m_lines->numVertices.setNum(numLines + 2);
    m_lines->numVertices.set1Value(numLines, 2);     // Horizontal line
    m_lines->numVertices.set1Value(numLines + 1, 2); // Vertical line
}

void SnapPointVisualizer::createCircleMarker(float radius, int segments)
{
    int baseIndex = m_coords->point.getNum();
    m_coords->point.setNum(baseIndex + segments + 1);
    
    for (int i = 0; i <= segments; ++i) {
        float angle = (2.0f * M_PI * i) / segments;
        float x = radius * std::cos(angle);
        float y = radius * std::sin(angle);
        m_coords->point.set1Value(baseIndex + i, x, y, 0);
    }
    
    if (!m_lines) {
        m_lines = new SoLineSet();
        m_markerGroup->addChild(m_lines);
    }
    
    int numLines = m_lines->numVertices.getNum();
    m_lines->numVertices.setNum(numLines + 1);
    m_lines->numVertices.set1Value(numLines, segments + 1);
}

void SnapPointVisualizer::showFeature(const RecognizedFeature& feature, bool active)
{
    m_currentFeatures.clear();
    m_currentFeatures.push_back(feature);
    
    // Update position
    updateMarkerPosition(feature.position);
    
    // Update color based on feature type and active state
    const float* color;
    if (active) {
        color = m_config.activeColor;
    } else if (feature.isInternalFeature || feature.type == FeatureType::HoleCenter) {
        color = m_config.holeColor;
    } else {
        color = m_config.markerColor;
    }
    updateMarkerColor(color);
    
    // Update label
    if (m_config.showLabels) {
        QString label = feature.description();
        updateLabel(label, feature.position);
    }
    
    // Show
    m_switch->whichChild = SO_SWITCH_ALL;
}

void SnapPointVisualizer::showFeatures(const std::vector<RecognizedFeature>& features)
{
    if (features.empty()) {
        hideAll();
        return;
    }
    
    m_currentFeatures = features;
    
    // For now, show just the first feature
    // TODO: Create multiple marker instances for showing all features
    showFeature(features.front(), false);
}

void SnapPointVisualizer::setActivePoint(const Base::Vector3d& position)
{
    // Find closest feature to position
    double minDist = 1e10;
    const RecognizedFeature* closest = nullptr;
    
    for (const auto& f : m_currentFeatures) {
        double dist = (f.position - position).Length();
        if (dist < minDist) {
            minDist = dist;
            closest = &f;
        }
    }
    
    if (closest && minDist < 5.0) { // 5mm threshold
        showFeature(*closest, true);
    }
}

void SnapPointVisualizer::hideAll()
{
    m_switch->whichChild = SO_SWITCH_NONE;
    m_currentFeatures.clear();
}

bool SnapPointVisualizer::isVisible() const
{
    return m_switch->whichChild.getValue() != SO_SWITCH_NONE;
}

void SnapPointVisualizer::setConfig(const SnapVisualizerConfig& config)
{
    m_config = config;
    
    // Update draw style
    m_drawStyle->lineWidth = config.lineWidth;
    m_drawStyle->pointSize = config.markerSize;
    
    // Update colors
    m_material->diffuseColor.setValue(config.markerColor[0],
                                       config.markerColor[1],
                                       config.markerColor[2]);
    m_material->emissiveColor.setValue(config.markerColor[0],
                                        config.markerColor[1],
                                        config.markerColor[2]);
    
    // Recreate geometry if style changed
    // (would need to clear and recreate marker geometry)
}

void SnapPointVisualizer::showCircleCenter(const Base::Vector3d& center,
                                           const Base::Vector3d& normal,
                                           double radius,
                                           bool active)
{
    RecognizedFeature feature;
    feature.type = FeatureType::CircleCenter;
    feature.position = center;
    feature.direction = normal;
    feature.radius = radius;
    
    showFeature(feature, active);
}

void SnapPointVisualizer::showCylinderCenter(const Base::Vector3d& center,
                                              const Base::Vector3d& axis,
                                              double radius,
                                              bool isHole,
                                              bool active)
{
    RecognizedFeature feature;
    feature.type = isHole ? FeatureType::HoleCenter : FeatureType::CylinderAxis;
    feature.position = center;
    feature.direction = axis;
    feature.radius = radius;
    feature.isInternalFeature = isHole;
    
    showFeature(feature, active);
}

void SnapPointVisualizer::showPoint(const Base::Vector3d& point,
                                     const QString& label,
                                     bool active)
{
    RecognizedFeature feature;
    feature.type = FeatureType::BoundingBoxCenter;
    feature.position = point;
    
    showFeature(feature, active);
    
    if (!label.isEmpty()) {
        updateLabel(label, point);
    }
}

void SnapPointVisualizer::updateMarkerPosition(const Base::Vector3d& pos)
{
    m_translation->translation.setValue(
        static_cast<float>(pos.x),
        static_cast<float>(pos.y),
        static_cast<float>(pos.z)
    );
}

void SnapPointVisualizer::updateMarkerColor(const float* color)
{
    m_material->diffuseColor.setValue(color[0], color[1], color[2]);
    m_material->emissiveColor.setValue(color[0], color[1], color[2]);
}

void SnapPointVisualizer::updateLabel(const QString& text, const Base::Vector3d& pos)
{
    if (!m_config.showLabels) {
        return;
    }
    
    // Position label slightly offset from marker
    m_labelTranslation->translation.setValue(
        static_cast<float>(pos.x + m_config.labelOffset),
        static_cast<float>(pos.y + m_config.labelOffset),
        static_cast<float>(pos.z)
    );
    
    // Set text
    m_labelText->string.setValue(text.toUtf8().constData());
}

} // namespace PartGui
