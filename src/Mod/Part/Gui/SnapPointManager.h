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

#ifndef PARTGUI_SNAP_POINT_MANAGER_H
#define PARTGUI_SNAP_POINT_MANAGER_H

#include <Mod/Part/PartGlobal.h>
#include <Gui/Selection/Selection.h>

#include "GeometryRecognition.h"
#include "SnapPointVisualizer.h"

#include <memory>

class SoSeparator;

namespace Gui {
class View3DInventorViewer;
}

namespace PartGui {

/**
 * @brief Snap point manager that integrates geometry recognition with the selection system
 * 
 * This class observes preselection events and automatically detects and visualizes
 * snap points (circle centers, hole centers, cylinder axes) when the user hovers
 * over imported geometry.
 * 
 * Usage:
 * @code
 *   // Create and enable snap point detection
 *   auto manager = SnapPointManager::instance();
 *   manager->setEnabled(true);
 *   
 *   // Connect to a 3D view
 *   manager->attachToView(viewer);
 * @endcode
 */
class PartGuiExport SnapPointManager : public Gui::SelectionObserver
{
public:
    /// Get the singleton instance
    static SnapPointManager& instance();
    
    /// Destructor
    ~SnapPointManager() override;
    
    // -------------------------------------------------------------------------
    // Configuration
    // -------------------------------------------------------------------------
    
    /// Enable or disable snap point detection
    void setEnabled(bool enabled);
    
    /// Check if snap point detection is enabled
    bool isEnabled() const { return m_enabled; }
    
    /// Set the visualizer configuration
    void setVisualizerConfig(const SnapVisualizerConfig& config);
    
    // -------------------------------------------------------------------------
    // View management
    // -------------------------------------------------------------------------
    
    /// Attach snap point visualization to a 3D view
    void attachToView(Gui::View3DInventorViewer* viewer);
    
    /// Detach from the current 3D view
    void detachFromView();
    
    /// Check if attached to a view
    bool isAttached() const { return m_viewer != nullptr; }
    
    // -------------------------------------------------------------------------
    // Feature access
    // -------------------------------------------------------------------------
    
    /// Get currently detected features (for external tools to use)
    const std::vector<RecognizedFeature>& getCurrentFeatures() const { return m_currentFeatures; }
    
    /// Get the closest snap point to a given position (for snapping logic)
    std::optional<Base::Vector3d> getClosestSnapPoint(const Base::Vector3d& pos, double tolerance = 5.0) const;

protected:
    /// Selection change callback
    void onSelectionChanged(const Gui::SelectionChanges& msg) override;

private:
    /// Private constructor (singleton)
    SnapPointManager();
    
    /// Analyze preselected element and detect snap points
    void analyzePreselection(const Gui::SelectionChanges& msg);
    
    /// Clear current snap point visualization
    void clearVisualization();
    
    /// Get shape from preselection info
    TopoDS_Shape getPreselectedShape(const Gui::SelectionChanges& msg);
    
    /// Get face from preselection info (e.g., "Face123")
    TopoDS_Face getPreselectedFace(const Gui::SelectionChanges& msg);
    
    /// Get edge from preselection info (e.g., "Edge45")
    TopoDS_Edge getPreselectedEdge(const Gui::SelectionChanges& msg);

private:
    bool m_enabled = true;
    Gui::View3DInventorViewer* m_viewer = nullptr;
    
    GeometryRecognition m_recognizer;
    std::unique_ptr<SnapPointVisualizer> m_visualizer;
    
    std::vector<RecognizedFeature> m_currentFeatures;
    SoSeparator* m_sceneRoot = nullptr;
};

} // namespace PartGui

#endif // PARTGUI_SNAP_POINT_MANAGER_H
