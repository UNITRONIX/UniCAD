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

#ifndef PARTGUI_GEOMETRY_RECOGNITION_H
#define PARTGUI_GEOMETRY_RECOGNITION_H

#include <Mod/Part/PartGlobal.h>
#include <Base/Vector3D.h>

#include <TopoDS_Face.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Shape.hxx>

#include <QString>
#include <vector>
#include <optional>

namespace PartGui {

/**
 * @brief Type of recognized geometric feature
 */
enum class FeatureType {
    None,
    CircleCenter,      ///< Center of a circular edge
    ArcCenter,         ///< Center of an arc edge
    CylinderAxis,      ///< Axis of a cylindrical face
    ConeApex,          ///< Apex of a conical face
    SphereCenter,      ///< Center of a spherical face
    TorusCenter,       ///< Center of a torus face
    HoleCenter,        ///< Center of a detected hole feature
    BoundingBoxCenter  ///< Center of bounding box (fallback)
};

/**
 * @brief Recognized geometric feature with position and metadata
 */
struct RecognizedFeature {
    FeatureType type = FeatureType::None;
    Base::Vector3d position;           ///< Feature point (center, apex, etc.)
    Base::Vector3d direction;          ///< Direction/axis (for cylinders, cones)
    double radius = 0.0;               ///< Radius (for circles, cylinders, spheres)
    double secondaryRadius = 0.0;      ///< Secondary radius (for torus major radius)
    bool isInternalFeature = false;    ///< True for holes, internal cylinders
    
    /// Human-readable description for UI
    QString description() const;
};

/**
 * @brief Automatic geometry recognition for snap points and feature detection
 * 
 * This class provides Fusion 360-style automatic detection of geometric
 * features like circle centers, cylinder axes, hole centers, etc.
 * 
 * Designed to work with imported STEP/IGES models where parametric
 * information is not available.
 * 
 * Usage:
 * @code
 *   GeometryRecognition recognizer;
 *   
 *   // Analyze a face during preselection
 *   auto features = recognizer.analyzeFace(face);
 *   for (const auto& f : features) {
 *       // Display snap marker at f.position
 *   }
 *   
 *   // Find all hole centers in a shape
 *   auto holes = recognizer.findHoles(shape);
 * @endcode
 */
class PartGuiExport GeometryRecognition {
public:
    GeometryRecognition() = default;
    ~GeometryRecognition() = default;
    
    // -------------------------------------------------------------------------
    // Face analysis
    // -------------------------------------------------------------------------
    
    /**
     * @brief Analyze a face and return all recognized features
     * @param face The TopoDS_Face to analyze
     * @return Vector of recognized features (may be empty)
     */
    std::vector<RecognizedFeature> analyzeFace(const TopoDS_Face& face) const;
    
    /**
     * @brief Get the center/axis point of a cylindrical face
     * @param face The face to analyze
     * @return Feature with cylinder axis info, or empty if not cylindrical
     */
    std::optional<RecognizedFeature> getCylinderAxis(const TopoDS_Face& face) const;
    
    /**
     * @brief Get the apex of a conical face
     * @param face The face to analyze
     * @return Feature with cone apex, or empty if not conical
     */
    std::optional<RecognizedFeature> getConeApex(const TopoDS_Face& face) const;
    
    /**
     * @brief Get the center of a spherical face
     * @param face The face to analyze
     * @return Feature with sphere center, or empty if not spherical
     */
    std::optional<RecognizedFeature> getSphereCenter(const TopoDS_Face& face) const;
    
    // -------------------------------------------------------------------------
    // Edge analysis
    // -------------------------------------------------------------------------
    
    /**
     * @brief Analyze an edge and return recognized features
     * @param edge The TopoDS_Edge to analyze
     * @return Vector of recognized features
     */
    std::vector<RecognizedFeature> analyzeEdge(const TopoDS_Edge& edge) const;
    
    /**
     * @brief Get the center of a circular edge
     * @param edge The edge to analyze
     * @return Feature with circle center, or empty if not circular
     */
    std::optional<RecognizedFeature> getCircleCenter(const TopoDS_Edge& edge) const;
    
    /**
     * @brief Get the center of an arc edge
     * @param edge The edge to analyze
     * @return Feature with arc center, or empty if not an arc
     */
    std::optional<RecognizedFeature> getArcCenter(const TopoDS_Edge& edge) const;
    
    // -------------------------------------------------------------------------
    // Hole detection
    // -------------------------------------------------------------------------
    
    /**
     * @brief Find all holes (through or blind) in a shape
     * @param shape The shape to analyze
     * @param minRadius Minimum hole radius to detect (default 0.1mm)
     * @param maxRadius Maximum hole radius to detect (default 100mm)
     * @return Vector of hole features with centers and radii
     */
    std::vector<RecognizedFeature> findHoles(
        const TopoDS_Shape& shape,
        double minRadius = 0.1,
        double maxRadius = 100.0
    ) const;
    
    /**
     * @brief Check if a cylindrical face represents a hole (internal feature)
     * @param face Cylindrical face to check
     * @param shape Parent shape for context
     * @return True if face is part of a hole
     */
    bool isHoleFace(const TopoDS_Face& face, const TopoDS_Shape& shape) const;
    
    // -------------------------------------------------------------------------
    // Shape-wide analysis
    // -------------------------------------------------------------------------
    
    /**
     * @brief Find all snap-able feature points in a shape
     * 
     * This is used to pre-compute snap targets for faster interaction.
     * 
     * @param shape The shape to analyze
     * @return All recognized features (centers, axes, etc.)
     */
    std::vector<RecognizedFeature> findAllFeatures(const TopoDS_Shape& shape) const;
    
    /**
     * @brief Find the closest snap point to a given position
     * @param shape The shape to search
     * @param point Query point
     * @param maxDistance Maximum snap distance
     * @return Closest feature within maxDistance, or empty
     */
    std::optional<RecognizedFeature> findClosestSnapPoint(
        const TopoDS_Shape& shape,
        const Base::Vector3d& point,
        double maxDistance = 10.0
    ) const;
    
    // -------------------------------------------------------------------------
    // Utility
    // -------------------------------------------------------------------------
    
    /**
     * @brief Get human-readable name for feature type
     */
    static QString featureTypeName(FeatureType type);
    
    /**
     * @brief Project a point onto a face surface
     * @param face Target face
     * @param point Point to project
     * @return Projected point on face, or empty if projection failed
     */
    static std::optional<Base::Vector3d> projectPointOnFace(
        const TopoDS_Face& face,
        const Base::Vector3d& point
    );
    
private:
    /// Check if surface normal points inward (hole) or outward
    bool isInternalSurface(const TopoDS_Face& face, const TopoDS_Shape& parentShape) const;
};

} // namespace PartGui

#endif // PARTGUI_GEOMETRY_RECOGNITION_H
