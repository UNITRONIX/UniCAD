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

#include "SnapPointManager.h"

#include <App/Document.h>
#include <App/DocumentObject.h>
#include <Gui/Application.h>
#include <Gui/Document.h>
#include <Gui/MDIView.h>
#include <Gui/View3DInventor.h>
#include <Gui/View3DInventorViewer.h>
#include <Gui/ViewProviderDocumentObject.h>

#include <Mod/Part/App/PartFeature.h>
#include <Mod/Part/App/TopoShape.h>

#include <TopoDS.hxx>
#include <TopExp_Explorer.hxx>

#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoNode.h>

#include <regex>

namespace PartGui {

// Singleton instance
SnapPointManager& SnapPointManager::instance()
{
    static SnapPointManager instance;
    return instance;
}

SnapPointManager::SnapPointManager()
    : SelectionObserver(true, Gui::ResolveMode::NoResolve)
    , m_visualizer(std::make_unique<SnapPointVisualizer>())
{
}

SnapPointManager::~SnapPointManager()
{
    detachFromView();
}

void SnapPointManager::setEnabled(bool enabled)
{
    m_enabled = enabled;
    if (!enabled) {
        clearVisualization();
    }
}

void SnapPointManager::setVisualizerConfig(const SnapVisualizerConfig& config)
{
    m_visualizer->setConfig(config);
}

void SnapPointManager::attachToView(Gui::View3DInventorViewer* viewer)
{
    if (m_viewer == viewer) {
        return;
    }
    
    // Detach from previous view if any
    detachFromView();
    
    m_viewer = viewer;
    
    if (m_viewer) {
        // Get the scene root and add our visualizer
        SoNode* sceneNode = m_viewer->getSceneGraph();
        m_sceneRoot = dynamic_cast<SoSeparator*>(sceneNode);
        if (m_sceneRoot && m_visualizer->getSceneGraph()) {
            m_sceneRoot->addChild(m_visualizer->getSceneGraph());
        }
    }
}

void SnapPointManager::detachFromView()
{
    if (m_sceneRoot && m_visualizer->getSceneGraph()) {
        m_sceneRoot->removeChild(m_visualizer->getSceneGraph());
    }
    m_sceneRoot = nullptr;
    m_viewer = nullptr;
}

std::optional<Base::Vector3d> SnapPointManager::getClosestSnapPoint(
    const Base::Vector3d& pos, double tolerance) const
{
    if (m_currentFeatures.empty()) {
        return std::nullopt;
    }
    
    double minDist = tolerance;
    const RecognizedFeature* closest = nullptr;
    
    for (const auto& feature : m_currentFeatures) {
        double dist = (feature.position - pos).Length();
        if (dist < minDist) {
            minDist = dist;
            closest = &feature;
        }
    }
    
    if (closest) {
        return closest->position;
    }
    
    return std::nullopt;
}

void SnapPointManager::onSelectionChanged(const Gui::SelectionChanges& msg)
{
    if (!m_enabled) {
        return;
    }
    
    // Lazy attach to active 3D view if not attached
    if (!m_viewer) {
        // Try to get active 3D view
        Gui::Document* guiDoc = Gui::Application::Instance->activeDocument();
        if (guiDoc) {
            Gui::MDIView* mdiView = guiDoc->getActiveView();
            if (mdiView) {
                auto* view3d = qobject_cast<Gui::View3DInventor*>(mdiView);
                if (view3d) {
                    attachToView(view3d->getViewer());
                }
            }
        }
    }
    
    if (!m_viewer) {
        return;
    }
    
    switch (msg.Type) {
        case Gui::SelectionChanges::SetPreselect:
        case Gui::SelectionChanges::MovePreselect:
            analyzePreselection(msg);
            break;
            
        case Gui::SelectionChanges::RmvPreselect:
            clearVisualization();
            break;
            
        default:
            break;
    }
}

void SnapPointManager::analyzePreselection(const Gui::SelectionChanges& msg)
{
    m_currentFeatures.clear();
    
    // Try to get a face first
    TopoDS_Face face = getPreselectedFace(msg);
    if (!face.IsNull()) {
        m_currentFeatures = m_recognizer.analyzeFace(face);
    }
    else {
        // Try to get an edge
        TopoDS_Edge edge = getPreselectedEdge(msg);
        if (!edge.IsNull()) {
            m_currentFeatures = m_recognizer.analyzeEdge(edge);
        }
    }
    
    // Update visualization
    if (!m_currentFeatures.empty()) {
        m_visualizer->showFeatures(m_currentFeatures);
    }
    else {
        m_visualizer->hideAll();
    }
}

void SnapPointManager::clearVisualization()
{
    m_currentFeatures.clear();
    m_visualizer->hideAll();
}

TopoDS_Shape SnapPointManager::getPreselectedShape(const Gui::SelectionChanges& msg)
{
    // Get the document
    App::Document* doc = App::GetApplication().getDocument(msg.pDocName);
    if (!doc) {
        return {};
    }
    
    // Get the object
    App::DocumentObject* obj = doc->getObject(msg.pObjectName);
    if (!obj) {
        return {};
    }
    
    // Check if it's a Part feature
    auto* partFeature = dynamic_cast<Part::Feature*>(obj);
    if (!partFeature) {
        return {};
    }
    
    return partFeature->Shape.getValue();
}

TopoDS_Face SnapPointManager::getPreselectedFace(const Gui::SelectionChanges& msg)
{
    // Parse the subname to check if it's a face (e.g., "Face123")
    std::string subName = msg.pSubName ? msg.pSubName : "";
    
    std::regex faceRegex("Face(\\d+)");
    std::smatch match;
    
    if (!std::regex_search(subName, match, faceRegex)) {
        return {};
    }
    
    int faceIndex = std::stoi(match[1].str());
    
    // Get the shape
    TopoDS_Shape shape = getPreselectedShape(msg);
    if (shape.IsNull()) {
        return {};
    }
    
    // Find the face by index
    int currentIndex = 1;
    for (TopExp_Explorer explorer(shape, TopAbs_FACE); explorer.More(); explorer.Next()) {
        if (currentIndex == faceIndex) {
            return TopoDS::Face(explorer.Current());
        }
        currentIndex++;
    }
    
    return {};
}

TopoDS_Edge SnapPointManager::getPreselectedEdge(const Gui::SelectionChanges& msg)
{
    // Parse the subname to check if it's an edge (e.g., "Edge45")
    std::string subName = msg.pSubName ? msg.pSubName : "";
    
    std::regex edgeRegex("Edge(\\d+)");
    std::smatch match;
    
    if (!std::regex_search(subName, match, edgeRegex)) {
        return {};
    }
    
    int edgeIndex = std::stoi(match[1].str());
    
    // Get the shape
    TopoDS_Shape shape = getPreselectedShape(msg);
    if (shape.IsNull()) {
        return {};
    }
    
    // Find the edge by index
    int currentIndex = 1;
    for (TopExp_Explorer explorer(shape, TopAbs_EDGE); explorer.More(); explorer.Next()) {
        if (currentIndex == edgeIndex) {
            return TopoDS::Edge(explorer.Current());
        }
        currentIndex++;
    }
    
    return {};
}

} // namespace PartGui
