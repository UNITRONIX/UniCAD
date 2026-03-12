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

#ifndef PARTGUI_SNAP_POINT_VISUALIZER_H
#define PARTGUI_SNAP_POINT_VISUALIZER_H

#include <Mod/Part/PartGlobal.h>
#include <Base/Vector3D.h>
#include "GeometryRecognition.h"

#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoSwitch.h>

#include <vector>
#include <memory>

class SoCoordinate3;
class SoMaterial;
class SoDrawStyle;
class SoMarkerSet;
class SoLineSet;
class SoAnnotation;
class SoFont;
class SoText2;
class SoTranslation;

namespace PartGui {

/**
 * @brief Visual style for snap point markers
 */
enum class SnapMarkerStyle {
    CrossHair,      ///< Simple crosshair (+)
    Circle,         ///< Circle around the point
    CircleWithDot,  ///< Circle with center dot (Fusion 360 style)
    Diamond,        ///< Diamond shape
    Target          ///< Target/bullseye
};

/**
 * @brief Configuration for snap point visualization
 */
struct SnapVisualizerConfig {
    // Colors (Fusion 360 style defaults)
    float markerColor[3] = {0.0f, 0.9f, 1.0f};      // Cyan (#00E5FF)
    float activeColor[3] = {1.0f, 0.08f, 0.58f};    // Magenta (#FF1493)
    float holeColor[3] = {1.0f, 0.55f, 0.0f};       // Orange (#FF8C00)
    
    // Sizes
    float markerSize = 8.0f;
    float crosshairSize = 12.0f;
    float lineWidth = 2.0f;
    
    // Behavior
    bool showLabels = true;
    bool showRadius = true;
    float labelOffset = 15.0f;
    
    SnapMarkerStyle style = SnapMarkerStyle::CircleWithDot;
};

/**
 * @brief Visualizes snap points (centers, axes) in 3D view using Coin3D
 * 
 * This class creates and manages Coin3D scene graph nodes to display
 * visual markers for recognized geometric features like circle centers,
 * cylinder axes, hole centers, etc.
 * 
 * Designed to integrate with preselection system for real-time feedback.
 * 
 * Usage:
 * @code
 *   // Create visualizer and attach to scene
 *   SnapPointVisualizer visualizer;
 *   rootNode->addChild(visualizer.getSceneGraph());
 *   
 *   // Show snap points for a recognized feature
 *   visualizer.showFeature(feature);
 *   
 *   // Hide all markers
 *   visualizer.hideAll();
 * @endcode
 */
class PartGuiExport SnapPointVisualizer {
public:
    SnapPointVisualizer();
    ~SnapPointVisualizer();
    
    // -------------------------------------------------------------------------
    // Scene graph
    // -------------------------------------------------------------------------
    
    /**
     * @brief Get the root scene graph node
     * 
     * Add this to your view's scene graph to display snap markers.
     */
    SoSeparator* getSceneGraph() const { return m_root; }
    
    // -------------------------------------------------------------------------
    // Display methods
    // -------------------------------------------------------------------------
    
    /**
     * @brief Show marker for a single recognized feature
     * @param feature The feature to visualize
     * @param active If true, use active/selected highlight color
     */
    void showFeature(const RecognizedFeature& feature, bool active = false);
    
    /**
     * @brief Show markers for multiple features
     * @param features Vector of features to visualize
     */
    void showFeatures(const std::vector<RecognizedFeature>& features);
    
    /**
     * @brief Update the active/highlighted feature
     * @param position Position to highlight (nearest feature)
     */
    void setActivePoint(const Base::Vector3d& position);
    
    /**
     * @brief Hide all snap markers
     */
    void hideAll();
    
    /**
     * @brief Check if any markers are currently visible
     */
    bool isVisible() const;
    
    // -------------------------------------------------------------------------
    // Configuration
    // -------------------------------------------------------------------------
    
    /**
     * @brief Set visualization configuration
     */
    void setConfig(const SnapVisualizerConfig& config);
    
    /**
     * @brief Get current configuration
     */
    const SnapVisualizerConfig& getConfig() const { return m_config; }
    
    // -------------------------------------------------------------------------
    // Convenience methods for single point display
    // -------------------------------------------------------------------------
    
    /**
     * @brief Show a circle center marker
     */
    void showCircleCenter(const Base::Vector3d& center, 
                          const Base::Vector3d& normal,
                          double radius,
                          bool active = false);
    
    /**
     * @brief Show a cylinder axis marker
     */
    void showCylinderCenter(const Base::Vector3d& center,
                            const Base::Vector3d& axis,
                            double radius,
                            bool isHole = false,
                            bool active = false);
    
    /**
     * @brief Show a generic point marker
     */
    void showPoint(const Base::Vector3d& point,
                   const QString& label = QString(),
                   bool active = false);
    
private:
    void initializeSceneGraph();
    void createMarkerGeometry();
    void updateMarkerPosition(const Base::Vector3d& pos);
    void updateMarkerColor(const float* color);
    void updateLabel(const QString& text, const Base::Vector3d& pos);
    
    // Create crosshair lines centered at origin
    void createCrosshair(float size);
    
    // Create circle geometry
    void createCircleMarker(float radius, int segments = 24);
    
    SnapVisualizerConfig m_config;
    
    // Coin3D nodes
    SoSeparator* m_root = nullptr;
    SoSwitch* m_switch = nullptr;
    
    // Main marker
    SoSeparator* m_markerGroup = nullptr;
    SoMaterial* m_material = nullptr;
    SoDrawStyle* m_drawStyle = nullptr;
    SoCoordinate3* m_coords = nullptr;
    SoLineSet* m_lines = nullptr;
    SoMarkerSet* m_points = nullptr;
    SoTranslation* m_translation = nullptr;
    
    // Label
    SoSeparator* m_labelGroup = nullptr;
    SoFont* m_font = nullptr;
    SoText2* m_labelText = nullptr;
    SoTranslation* m_labelTranslation = nullptr;
    SoMaterial* m_labelMaterial = nullptr;
    
    // Currently displayed features for hit-testing
    std::vector<RecognizedFeature> m_currentFeatures;
};

} // namespace PartGui

#endif // PARTGUI_SNAP_POINT_VISUALIZER_H
