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

#include "GeometryRecognition.h"

#include <BRepAdaptor_Surface.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRep_Tool.hxx>
#include <BRepGProp.hxx>
#include <BRepBndLib.hxx>
#include <GProp_GProps.hxx>
#include <Bnd_Box.hxx>

#include <GeomAbs_SurfaceType.hxx>
#include <GeomAbs_CurveType.hxx>
#include <gp_Cylinder.hxx>
#include <gp_Cone.hxx>
#include <gp_Sphere.hxx>
#include <gp_Torus.hxx>
#include <gp_Circ.hxx>
#include <gp_Pnt.hxx>
#include <gp_Dir.hxx>
#include <gp_Vec.hxx>

#include <TopoDS.hxx>
#include <TopoDS_Iterator.hxx>
#include <TopExp_Explorer.hxx>
#include <TopAbs_ShapeEnum.hxx>

#include <GeomAPI_ProjectPointOnSurf.hxx>
#include <BRepClass3d_SolidClassifier.hxx>

#include <QString>
#include <QObject>

namespace PartGui {

// =============================================================================
// RecognizedFeature implementation
// =============================================================================

QString RecognizedFeature::description() const
{
    QString desc = GeometryRecognition::featureTypeName(type);
    
    if (radius > 0) {
        desc += QStringLiteral(" (R=%1)").arg(radius, 0, 'f', 2);
    }
    
    if (isInternalFeature) {
        desc += QObject::tr(" [Internal]");
    }
    
    return desc;
}

// =============================================================================
// GeometryRecognition implementation
// =============================================================================

QString GeometryRecognition::featureTypeName(FeatureType type)
{
    switch (type) {
        case FeatureType::CircleCenter:
            return QObject::tr("Circle Center");
        case FeatureType::ArcCenter:
            return QObject::tr("Arc Center");
        case FeatureType::CylinderAxis:
            return QObject::tr("Cylinder Center");
        case FeatureType::ConeApex:
            return QObject::tr("Cone Apex");
        case FeatureType::SphereCenter:
            return QObject::tr("Sphere Center");
        case FeatureType::TorusCenter:
            return QObject::tr("Torus Center");
        case FeatureType::HoleCenter:
            return QObject::tr("Hole Center");
        case FeatureType::BoundingBoxCenter:
            return QObject::tr("Center");
        default:
            return QObject::tr("Point");
    }
}

// -----------------------------------------------------------------------------
// Face analysis
// -----------------------------------------------------------------------------

std::vector<RecognizedFeature> GeometryRecognition::analyzeFace(const TopoDS_Face& face) const
{
    std::vector<RecognizedFeature> features;
    
    if (face.IsNull()) {
        return features;
    }
    
    // Try each surface type
    if (auto cyl = getCylinderAxis(face)) {
        features.push_back(*cyl);
    }
    else if (auto cone = getConeApex(face)) {
        features.push_back(*cone);
    }
    else if (auto sphere = getSphereCenter(face)) {
        features.push_back(*sphere);
    }
    
    // Also analyze edges of the face for circular features
    TopExp_Explorer edgeExp(face, TopAbs_EDGE);
    for (; edgeExp.More(); edgeExp.Next()) {
        TopoDS_Edge edge = TopoDS::Edge(edgeExp.Current());
        auto edgeFeatures = analyzeEdge(edge);
        for (auto& ef : edgeFeatures) {
            // Avoid duplicates - check if similar position already exists
            bool duplicate = false;
            for (const auto& existing : features) {
                double dist = (existing.position - ef.position).Length();
                if (dist < 0.01) { // 0.01mm tolerance
                    duplicate = true;
                    break;
                }
            }
            if (!duplicate) {
                features.push_back(ef);
            }
        }
    }
    
    return features;
}

std::optional<RecognizedFeature> GeometryRecognition::getCylinderAxis(const TopoDS_Face& face) const
{
    if (face.IsNull()) {
        return std::nullopt;
    }
    
    try {
        BRepAdaptor_Surface surface(face);
        
        if (surface.GetType() != GeomAbs_Cylinder) {
            return std::nullopt;
        }
        
        gp_Cylinder cylinder = surface.Cylinder();
        gp_Ax1 axis = cylinder.Axis();
        gp_Pnt location = axis.Location();
        gp_Dir direction = axis.Direction();
        
        // Get the center point on the face (middle of cylinder segment)
        // Use parametric bounds to find middle
        double uMin = surface.FirstUParameter();
        double uMax = surface.LastUParameter();
        double vMin = surface.FirstVParameter();
        double vMax = surface.LastVParameter();
        
        // V parameter is along the axis for cylinder
        double vMid = (vMin + vMax) / 2.0;
        
        // Calculate center at mid-height
        gp_Pnt centerPoint = location.Translated(gp_Vec(direction) * vMid);
        
        RecognizedFeature feature;
        feature.type = FeatureType::CylinderAxis;
        feature.position = Base::Vector3d(centerPoint.X(), centerPoint.Y(), centerPoint.Z());
        feature.direction = Base::Vector3d(direction.X(), direction.Y(), direction.Z());
        feature.radius = cylinder.Radius();
        
        // Check if internal (will be refined when we have parent shape context)
        // For now, use face orientation
        feature.isInternalFeature = (face.Orientation() == TopAbs_REVERSED);
        
        return feature;
    }
    catch (...) {
        return std::nullopt;
    }
}

std::optional<RecognizedFeature> GeometryRecognition::getConeApex(const TopoDS_Face& face) const
{
    if (face.IsNull()) {
        return std::nullopt;
    }
    
    try {
        BRepAdaptor_Surface surface(face);
        
        if (surface.GetType() != GeomAbs_Cone) {
            return std::nullopt;
        }
        
        gp_Cone cone = surface.Cone();
        gp_Pnt apex = cone.Apex();
        gp_Dir direction = cone.Axis().Direction();
        
        RecognizedFeature feature;
        feature.type = FeatureType::ConeApex;
        feature.position = Base::Vector3d(apex.X(), apex.Y(), apex.Z());
        feature.direction = Base::Vector3d(direction.X(), direction.Y(), direction.Z());
        feature.radius = cone.RefRadius();
        
        return feature;
    }
    catch (...) {
        return std::nullopt;
    }
}

std::optional<RecognizedFeature> GeometryRecognition::getSphereCenter(const TopoDS_Face& face) const
{
    if (face.IsNull()) {
        return std::nullopt;
    }
    
    try {
        BRepAdaptor_Surface surface(face);
        
        if (surface.GetType() != GeomAbs_Sphere) {
            return std::nullopt;
        }
        
        gp_Sphere sphere = surface.Sphere();
        gp_Pnt center = sphere.Location();
        
        RecognizedFeature feature;
        feature.type = FeatureType::SphereCenter;
        feature.position = Base::Vector3d(center.X(), center.Y(), center.Z());
        feature.radius = sphere.Radius();
        
        return feature;
    }
    catch (...) {
        return std::nullopt;
    }
}

// -----------------------------------------------------------------------------
// Edge analysis
// -----------------------------------------------------------------------------

std::vector<RecognizedFeature> GeometryRecognition::analyzeEdge(const TopoDS_Edge& edge) const
{
    std::vector<RecognizedFeature> features;
    
    if (auto circle = getCircleCenter(edge)) {
        features.push_back(*circle);
    }
    else if (auto arc = getArcCenter(edge)) {
        features.push_back(*arc);
    }
    
    return features;
}

std::optional<RecognizedFeature> GeometryRecognition::getCircleCenter(const TopoDS_Edge& edge) const
{
    if (edge.IsNull()) {
        return std::nullopt;
    }
    
    try {
        BRepAdaptor_Curve curve(edge);
        
        if (curve.GetType() != GeomAbs_Circle) {
            return std::nullopt;
        }
        
        gp_Circ circle = curve.Circle();
        gp_Pnt center = circle.Location();
        gp_Dir normal = circle.Axis().Direction();
        
        // Check if it's a full circle or just an arc
        double firstParam = curve.FirstParameter();
        double lastParam = curve.LastParameter();
        double paramRange = lastParam - firstParam;
        
        // Full circle is approximately 2*PI
        bool isFullCircle = (paramRange > 6.0); // ~2*PI = 6.28...
        
        RecognizedFeature feature;
        feature.type = isFullCircle ? FeatureType::CircleCenter : FeatureType::ArcCenter;
        feature.position = Base::Vector3d(center.X(), center.Y(), center.Z());
        feature.direction = Base::Vector3d(normal.X(), normal.Y(), normal.Z());
        feature.radius = circle.Radius();
        
        return feature;
    }
    catch (...) {
        return std::nullopt;
    }
}

std::optional<RecognizedFeature> GeometryRecognition::getArcCenter(const TopoDS_Edge& edge) const
{
    // Arc detection is handled by getCircleCenter
    // This is here for explicit arc-only queries if needed
    auto result = getCircleCenter(edge);
    if (result && result->type == FeatureType::ArcCenter) {
        return result;
    }
    return std::nullopt;
}

// -----------------------------------------------------------------------------
// Hole detection
// -----------------------------------------------------------------------------

std::vector<RecognizedFeature> GeometryRecognition::findHoles(
    const TopoDS_Shape& shape,
    double minRadius,
    double maxRadius) const
{
    std::vector<RecognizedFeature> holes;
    
    if (shape.IsNull()) {
        return holes;
    }
    
    // Find all cylindrical faces
    TopExp_Explorer faceExp(shape, TopAbs_FACE);
    for (; faceExp.More(); faceExp.Next()) {
        TopoDS_Face face = TopoDS::Face(faceExp.Current());
        
        auto cylFeature = getCylinderAxis(face);
        if (!cylFeature) {
            continue;
        }
        
        // Filter by radius
        if (cylFeature->radius < minRadius || cylFeature->radius > maxRadius) {
            continue;
        }
        
        // Check if it's an internal feature (hole)
        if (isHoleFace(face, shape)) {
            cylFeature->type = FeatureType::HoleCenter;
            cylFeature->isInternalFeature = true;
            
            // Avoid duplicate holes (same position)
            bool duplicate = false;
            for (const auto& existing : holes) {
                double dist = (existing.position - cylFeature->position).Length();
                if (dist < 0.1) { // 0.1mm tolerance
                    duplicate = true;
                    break;
                }
            }
            
            if (!duplicate) {
                holes.push_back(*cylFeature);
            }
        }
    }
    
    return holes;
}

bool GeometryRecognition::isHoleFace(const TopoDS_Face& face, const TopoDS_Shape& shape) const
{
    if (face.IsNull() || shape.IsNull()) {
        return false;
    }
    
    try {
        // Method 1: Check face orientation
        // Internal faces of solids are typically REVERSED
        if (face.Orientation() == TopAbs_REVERSED) {
            return true;
        }
        
        // Method 2: Check if surface normal points inward
        // Get a point on the surface and check if normal points toward solid interior
        BRepAdaptor_Surface surface(face);
        
        double uMid = (surface.FirstUParameter() + surface.LastUParameter()) / 2.0;
        double vMid = (surface.FirstVParameter() + surface.LastVParameter()) / 2.0;
        
        gp_Pnt surfPoint;
        gp_Vec du, dv;
        surface.D1(uMid, vMid, surfPoint, du, dv);
        
        gp_Vec normal = du.Crossed(dv);
        if (normal.Magnitude() < 1e-10) {
            return false;
        }
        normal.Normalize();
        
        // Move a small distance along the normal
        double offset = 0.1; // 0.1mm
        gp_Pnt testPoint = surfPoint.Translated(normal * offset);
        
        // Check if test point is inside the solid
        BRepClass3d_SolidClassifier classifier(shape, testPoint, 1e-6);
        TopAbs_State state = classifier.State();
        
        // If the point offset along normal is INSIDE, the normal points inward -> hole
        return (state == TopAbs_IN);
    }
    catch (...) {
        // Fallback to orientation check
        return (face.Orientation() == TopAbs_REVERSED);
    }
}

// -----------------------------------------------------------------------------
// Shape-wide analysis
// -----------------------------------------------------------------------------

std::vector<RecognizedFeature> GeometryRecognition::findAllFeatures(const TopoDS_Shape& shape) const
{
    std::vector<RecognizedFeature> features;
    
    if (shape.IsNull()) {
        return features;
    }
    
    // Analyze all faces
    TopExp_Explorer faceExp(shape, TopAbs_FACE);
    for (; faceExp.More(); faceExp.Next()) {
        TopoDS_Face face = TopoDS::Face(faceExp.Current());
        auto faceFeatures = analyzeFace(face);
        
        for (auto& ff : faceFeatures) {
            // Mark as hole if appropriate
            if (ff.type == FeatureType::CylinderAxis && isHoleFace(face, shape)) {
                ff.type = FeatureType::HoleCenter;
                ff.isInternalFeature = true;
            }
            
            // Avoid duplicates
            bool duplicate = false;
            for (const auto& existing : features) {
                double dist = (existing.position - ff.position).Length();
                if (dist < 0.01) {
                    duplicate = true;
                    break;
                }
            }
            
            if (!duplicate) {
                features.push_back(ff);
            }
        }
    }
    
    return features;
}

std::optional<RecognizedFeature> GeometryRecognition::findClosestSnapPoint(
    const TopoDS_Shape& shape,
    const Base::Vector3d& point,
    double maxDistance) const
{
    auto features = findAllFeatures(shape);
    
    std::optional<RecognizedFeature> closest;
    double minDist = maxDistance;
    
    for (const auto& f : features) {
        double dist = (f.position - point).Length();
        if (dist < minDist) {
            minDist = dist;
            closest = f;
        }
    }
    
    return closest;
}

// -----------------------------------------------------------------------------
// Utility
// -----------------------------------------------------------------------------

std::optional<Base::Vector3d> GeometryRecognition::projectPointOnFace(
    const TopoDS_Face& face,
    const Base::Vector3d& point)
{
    if (face.IsNull()) {
        return std::nullopt;
    }
    
    try {
        gp_Pnt pnt(point.x, point.y, point.z);
        
        GeomAPI_ProjectPointOnSurf projector(pnt, BRep_Tool::Surface(face));
        
        if (projector.NbPoints() > 0) {
            gp_Pnt projected = projector.NearestPoint();
            return Base::Vector3d(projected.X(), projected.Y(), projected.Z());
        }
    }
    catch (...) {
        // Projection failed
    }
    
    return std::nullopt;
}

bool GeometryRecognition::isInternalSurface(
    const TopoDS_Face& face,
    const TopoDS_Shape& parentShape) const
{
    return isHoleFace(face, parentShape);
}

} // namespace PartGui
