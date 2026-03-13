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

#ifndef _PreComp_
# include <BRepBuilderAPI_MakeEdge.hxx>
# include <BRepBuilderAPI_MakeFace.hxx>
# include <BRepBuilderAPI_MakeWire.hxx>
# include <BRepBuilderAPI_Transform.hxx>
# include <BRepOffsetAPI_MakeOffset.hxx>
# include <BRepPrimAPI_MakePrism.hxx>
# include <BRepPrimAPI_MakeBox.hxx>
# include <BRepFilletAPI_MakeFillet2d.hxx>
# include <BRepAdaptor_Surface.hxx>
# include <BRepAdaptor_Curve.hxx>
# include <BRepAlgoAPI_Cut.hxx>
# include <BRepAlgoAPI_Fuse.hxx>
# include <BRepBndLib.hxx>
# include <BRepProj_Projection.hxx>
# include <BRep_Tool.hxx>
# include <Bnd_Box.hxx>
# include <GeomAbs_SurfaceType.hxx>
# include <GeomLProp_SLProps.hxx>
# include <Geom_Surface.hxx>
# include <Geom_Plane.hxx>
# include <GeomAPI_ProjectPointOnSurf.hxx>
# include <GC_MakeArcOfCircle.hxx>
# include <Geom_TrimmedCurve.hxx>
# include <TopExp_Explorer.hxx>
# include <TopoDS.hxx>
# include <TopoDS_Wire.hxx>
# include <TopoDS_Face.hxx>
# include <TopoDS_Compound.hxx>
# include <TopoDS_Edge.hxx>
# include <TopoDS_Vertex.hxx>
# include <gp_Pln.hxx>
# include <gp_Vec.hxx>
# include <gp_Ax2.hxx>
# include <gp_Trsf.hxx>
# include <gp_GTrsf.hxx>
# include <ShapeAnalysis_FreeBounds.hxx>
# include <TopTools_HSequenceOfShape.hxx>
# include <cmath>
# include <algorithm>
# include <vector>
#endif

#include <App/Document.h>

#include "FeatureClearanceVolume.h"

using namespace Part;

PROPERTY_SOURCE(Part::ClearanceVolume, Part::Feature)

ClearanceVolume::ClearanceVolume()
{
    // Source faces that define the clearance outline
    ADD_PROPERTY_TYPE(SourceFaces, (nullptr, std::vector<std::string>()), 
                      "Clearance", App::Prop_None,
                      "Face(s) that define the clearance shape");
    
    // Offset/tolerance (default 0.2mm - typical for FDM)
    ADD_PROPERTY_TYPE(Offset, (0.2), 
                      "Clearance", App::Prop_None,
                      "Tolerance margin around face outline (mm)");
    
    // Offset for reverse/inner part (default 0 = use main Offset)
    ADD_PROPERTY_TYPE(OffsetReverse, (0.0), 
                      "Clearance", App::Prop_None,
                      "Tolerance for inner part (0 = use main Offset)");
    
    // Depth of extrusion (default 5mm)
    ADD_PROPERTY_TYPE(Depth, (5.0), 
                      "Clearance", App::Prop_None,
                      "Extrusion depth of clearance volume (mm)");
    
    // Reverse depth - extends into the connector (default 0mm)
    ADD_PROPERTY_TYPE(DepthReverse, (0.0), 
                      "Clearance", App::Prop_None,
                      "Depth extending into the port/connector (mm)");
    
    // Symmetric extrusion
    ADD_PROPERTY_TYPE(Symmetric, (false), 
                      "Clearance", App::Prop_None,
                      "Extrude in both directions from face");
    
    // Custom direction
    ADD_PROPERTY_TYPE(Direction, (Base::Vector3d(0, 0, 1)), 
                      "Clearance", App::Prop_None,
                      "Custom extrusion direction");
    
    // Use face normal
    ADD_PROPERTY_TYPE(UseNormal, (true), 
                      "Clearance", App::Prop_None,
                      "Use face normal as extrusion direction");
    
    // Flip direction
    ADD_PROPERTY_TYPE(FlipDirection, (false), 
                      "Clearance", App::Prop_None,
                      "Flip extrusion direction 180 degrees");
    
    // Auto-subtract from intersecting solids
    ADD_PROPERTY_TYPE(AutoSubtract, (true), 
                      "Clearance", App::Prop_None,
                      "Automatically subtract from intersecting solids");
    
    // Port name for identification
    ADD_PROPERTY_TYPE(PortName, (""), 
                      "Clearance", App::Prop_None,
                      "Name for identification (e.g., USB-C, HDMI)");
    
    // Display color (semi-transparent orange by default)
    ADD_PROPERTY_TYPE(DisplayColor, (0xFF9900),  // Orange
                      "Display", App::Prop_None,
                      "Color for visualization");
}

App::DocumentObjectExecReturn* ClearanceVolume::execute()
{
    try {
        TopoDS_Shape clearanceShape = buildClearanceShape();
        
        if (clearanceShape.IsNull()) {
            return new App::DocumentObjectExecReturn(
                "Failed to create clearance volume: invalid source faces");
        }
        
        this->Shape.setValue(clearanceShape);
        return App::DocumentObject::StdReturn;
    }
    catch (Standard_Failure& e) {
        return new App::DocumentObjectExecReturn(e.GetMessageString());
    }
    catch (...) {
        return new App::DocumentObjectExecReturn(
            "Unknown error creating clearance volume");
    }
}

const char* ClearanceVolume::getViewProviderName() const
{
    return "PartGui::ViewProviderClearanceVolume";
}

TopoDS_Shape ClearanceVolume::buildClearanceShape()
{
    // Get source object and sub-elements
    App::DocumentObject* sourceObj = SourceFaces.getValue();
    if (!sourceObj) {
        return {};
    }
    
    // Get the source shape
    auto* partFeature = dynamic_cast<Part::Feature*>(sourceObj);
    if (!partFeature) {
        return {};
    }
    
    TopoDS_Shape sourceShape = partFeature->Shape.getValue();
    if (sourceShape.IsNull()) {
        return {};
    }
    
    // Get sub-element names (faces)
    const std::vector<std::string>& subNames = SourceFaces.getSubValues();
    if (subNames.empty()) {
        return {};
    }
    
    double offset = Offset.getValue();
    double offsetReverse = OffsetReverse.getValue();
    // If offsetReverse is 0 or negative, use main offset
    if (offsetReverse < Precision::Confusion()) {
        offsetReverse = offset;
    }
    double depth = Depth.getValue();
    double depthReverse = DepthReverse.getValue();
    
    TopoDS_Shape resultShape;
    bool firstShape = true;
    
    // Process each selected face
    for (const auto& subName : subNames) {
        // Get the face
        TopoDS_Shape subShape = partFeature->Shape.getShape().getSubShape(subName.c_str());
        if (subShape.IsNull() || subShape.ShapeType() != TopAbs_FACE) {
            continue;
        }
        
        TopoDS_Face face = TopoDS::Face(subShape);
        
        // Get extrusion direction
        gp_Vec extrusionDir;
        if (UseNormal.getValue()) {
            extrusionDir = getFaceNormal(face);
        }
        else {
            Base::Vector3d dir = Direction.getValue();
            extrusionDir = gp_Vec(dir.x, dir.y, dir.z);
        }
        
        if (extrusionDir.Magnitude() < Precision::Confusion()) {
            continue;
        }
        extrusionDir.Normalize();
        
        // Flip direction if requested by user
        if (FlipDirection.getValue()) {
            extrusionDir.Reverse();
        }
        
        // Create offset outline for forward extrusion
        TopoDS_Shape offsetOutline = offsetFaceOutline(face, offset);
        if (offsetOutline.IsNull()) {
            // If offset fails, use original face boundary
            offsetOutline = face;
        }
        
        // Create offset outline for reverse extrusion (may use different offset)
        TopoDS_Shape offsetOutlineReverse = offsetFaceOutline(face, offsetReverse);
        if (offsetOutlineReverse.IsNull()) {
            offsetOutlineReverse = face;
        }
        
        // Extrude the outline
        // Forward extrusion (away from connector, into enclosure wall)
        TopoDS_Shape extrudedShape;
        if (depth > Precision::Confusion()) {
            extrudedShape = extrudeOutline(offsetOutline, extrusionDir, depth);
        }
        
        // Reverse extrusion (into the connector, to protect entire port depth)
        if (depthReverse > Precision::Confusion()) {
            TopoDS_Shape reverseExtrusion = extrudeOutline(offsetOutlineReverse, -extrusionDir, depthReverse);
            if (!reverseExtrusion.IsNull()) {
                if (extrudedShape.IsNull()) {
                    extrudedShape = reverseExtrusion;
                } else {
                    BRepAlgoAPI_Fuse fuser(extrudedShape, reverseExtrusion);
                    if (fuser.IsDone()) {
                        extrudedShape = fuser.Shape();
                    }
                }
            }
        }
        
        if (extrudedShape.IsNull()) {
            continue;
        }
        
        // Handle symmetric extrusion (additional option)
        if (Symmetric.getValue()) {
            TopoDS_Shape backExtrusion = extrudeOutline(offsetOutline, -extrusionDir, depth);
            if (!backExtrusion.IsNull()) {
                BRepAlgoAPI_Fuse fuser(extrudedShape, backExtrusion);
                if (fuser.IsDone()) {
                    extrudedShape = fuser.Shape();
                }
            }
        }
        
        // Combine with result
        if (firstShape) {
            resultShape = extrudedShape;
            firstShape = false;
        }
        else {
            BRepAlgoAPI_Fuse fuser(resultShape, extrudedShape);
            if (fuser.IsDone()) {
                resultShape = fuser.Shape();
            }
        }
    }
    
    return resultShape;
}

TopoDS_Shape ClearanceVolume::offsetFaceOutline(const TopoDS_Face& face, double offset)
{
    if (offset < Precision::Confusion()) {
        // No offset needed, return the face itself
        return face;
    }
    
    try {
        // Get face normal and bounding box
        gp_Vec faceNormal = getFaceNormal(face);
        if (faceNormal.Magnitude() < Precision::Confusion()) {
            return face;
        }
        faceNormal.Normalize();
        
        // Get bounding box of the face
        Bnd_Box bbox;
        BRepBndLib::Add(face, bbox);
        if (bbox.IsVoid()) {
            return face;
        }
        
        double xMin, yMin, zMin, xMax, yMax, zMax;
        bbox.Get(xMin, yMin, zMin, xMax, yMax, zMax);
        gp_Pnt center((xMin + xMax) / 2.0, (yMin + yMax) / 2.0, (zMin + zMax) / 2.0);
        
        // Calculate characteristic size (average of width and height in face plane)
        double dx = xMax - xMin;
        double dy = yMax - yMin;
        double dz = zMax - zMin;
        
        // Find the two largest dimensions (those perpendicular to normal)
        double dims[3] = {dx, dy, dz};
        double normalComps[3] = {std::abs(faceNormal.X()), std::abs(faceNormal.Y()), std::abs(faceNormal.Z())};
        
        // The dimension most aligned with normal is the "thickness"
        // The other two are width and height
        double width = 0, height = 0;
        int normalAxis = 0;
        if (normalComps[1] > normalComps[normalAxis]) normalAxis = 1;
        if (normalComps[2] > normalComps[normalAxis]) normalAxis = 2;
        
        if (normalAxis == 0) { width = dy; height = dz; }
        else if (normalAxis == 1) { width = dx; height = dz; }
        else { width = dx; height = dy; }
        
        // Use average of width and height as characteristic size
        double charSize = (width + height) / 2.0;
        if (charSize < Precision::Confusion()) {
            charSize = std::max({dx, dy, dz});
        }
        
        // Calculate scale factor to add 'offset' to each side
        // New size = old size + 2*offset (offset on each side)
        double scaleFactor = (charSize + 2.0 * offset) / charSize;
        
        // For planar faces, try wire offset first (more accurate)
        BRepAdaptor_Surface surfAdaptor(face);
        if (surfAdaptor.GetType() == GeomAbs_Plane) {
            gp_Pln plane = surfAdaptor.Plane();
            
            // Try offset - test both directions and pick larger result
            for (int sign = 1; sign >= -1; sign -= 2) {
                BRepOffsetAPI_MakeOffset offsetMaker(face);
                offsetMaker.Perform(sign * offset);
                
                if (offsetMaker.IsDone()) {
                    TopoDS_Shape offsetResult = offsetMaker.Shape();
                    TopoDS_Wire offsetWire;
                    
                    if (offsetResult.ShapeType() == TopAbs_WIRE) {
                        offsetWire = TopoDS::Wire(offsetResult);
                    }
                    else if (offsetResult.ShapeType() == TopAbs_COMPOUND) {
                        for (TopExp_Explorer exp(offsetResult, TopAbs_WIRE); exp.More(); exp.Next()) {
                            offsetWire = TopoDS::Wire(exp.Current());
                            break;
                        }
                    }
                    
                    if (!offsetWire.IsNull()) {
                        BRepBuilderAPI_MakeFace faceMaker(plane, offsetWire, Standard_True);
                        if (faceMaker.IsDone()) {
                            // Check if result is larger
                            Bnd_Box resultBbox;
                            BRepBndLib::Add(faceMaker.Face(), resultBbox);
                            double rxMin, ryMin, rzMin, rxMax, ryMax, rzMax;
                            resultBbox.Get(rxMin, ryMin, rzMin, rxMax, ryMax, rzMax);
                            double resultSize = (rxMax - rxMin) + (ryMax - ryMin) + (rzMax - rzMin);
                            double origSize = dx + dy + dz;
                            
                            if (resultSize > origSize) {
                                return faceMaker.Face();
                            }
                        }
                    }
                }
            }
        }
        
        // For non-planar faces or if planar offset failed:
        // 1. Project wire onto plane (flatten it while preserving curve types)
        // 2. Use BRepOffsetAPI_MakeOffset on the planar wire (correctly handles arcs)
        
        // Create projection plane perpendicular to face normal
        gp_Pln projPlane(center, gp_Dir(faceNormal));
        
        // Get the outer wire from the face
        TopoDS_Wire outerWire;
        for (TopExp_Explorer explorer(face, TopAbs_WIRE); explorer.More(); explorer.Next()) {
            outerWire = TopoDS::Wire(explorer.Current());
            break;
        }
        
        if (outerWire.IsNull()) {
            // Fallback: scale the entire face
            gp_Trsf scaleTrsf;
            scaleTrsf.SetScale(center, scaleFactor);
            BRepBuilderAPI_Transform transformer(face, scaleTrsf, Standard_True);
            if (transformer.IsDone()) {
                return transformer.Shape();
            }
            return face;
        }
        
        // Step 1: Project wire onto plane, preserving curve types
        BRepBuilderAPI_MakeWire projWireMaker;
        
        for (TopExp_Explorer edgeExp(outerWire, TopAbs_EDGE); edgeExp.More(); edgeExp.Next()) {
            TopoDS_Edge edge = TopoDS::Edge(edgeExp.Current());
            BRepAdaptor_Curve curve(edge);
            
            GeomAbs_CurveType curveType = curve.GetType();
            double first = curve.FirstParameter();
            double last = curve.LastParameter();
            
            // Project start and end points onto plane
            gp_Pnt startPt = curve.Value(first);
            gp_Pnt endPt = curve.Value(last);
            
            double startDist = gp_Vec(center, startPt).Dot(faceNormal);
            double endDist = gp_Vec(center, endPt).Dot(faceNormal);
            
            gp_Pnt projStart = startPt.Translated(-faceNormal * startDist);
            gp_Pnt projEnd = endPt.Translated(-faceNormal * endDist);
            
            TopoDS_Edge newEdge;
            
            if (curveType == GeomAbs_Line) {
                if (projStart.Distance(projEnd) > Precision::Confusion()) {
                    newEdge = BRepBuilderAPI_MakeEdge(projStart, projEnd).Edge();
                }
            }
            else if (curveType == GeomAbs_Circle) {
                // For arcs, project 3 points and recreate arc
                double midParam = (first + last) / 2.0;
                gp_Pnt midPt = curve.Value(midParam);
                double midDist = gp_Vec(center, midPt).Dot(faceNormal);
                gp_Pnt projMid = midPt.Translated(-faceNormal * midDist);
                
                try {
                    Handle(Geom_TrimmedCurve) arc = GC_MakeArcOfCircle(projStart, projMid, projEnd).Value();
                    newEdge = BRepBuilderAPI_MakeEdge(arc).Edge();
                }
                catch (...) {
                    if (projStart.Distance(projEnd) > Precision::Confusion()) {
                        newEdge = BRepBuilderAPI_MakeEdge(projStart, projEnd).Edge();
                    }
                }
            }
            else {
                // Other curves: sample and create line segments
                std::vector<gp_Pnt> pts;
                for (int i = 0; i <= 10; ++i) {
                    double param = first + (last - first) * i / 10.0;
                    gp_Pnt pt = curve.Value(param);
                    double dist = gp_Vec(center, pt).Dot(faceNormal);
                    gp_Pnt projPt = pt.Translated(-faceNormal * dist);
                    if (pts.empty() || pts.back().Distance(projPt) > Precision::Confusion()) {
                        pts.push_back(projPt);
                    }
                }
                for (size_t i = 0; i + 1 < pts.size(); ++i) {
                    if (pts[i].Distance(pts[i + 1]) > Precision::Confusion()) {
                        projWireMaker.Add(BRepBuilderAPI_MakeEdge(pts[i], pts[i + 1]).Edge());
                    }
                }
                continue;
            }
            
            if (!newEdge.IsNull()) {
                projWireMaker.Add(newEdge);
            }
        }
        
        if (!projWireMaker.IsDone()) {
            // Fallback to scaling
            gp_Trsf scaleTrsf;
            scaleTrsf.SetScale(center, scaleFactor);
            BRepBuilderAPI_Transform transformer(face, scaleTrsf, Standard_True);
            if (transformer.IsDone()) {
                return transformer.Shape();
            }
            return face;
        }
        
        TopoDS_Wire projectedWire = projWireMaker.Wire();
        
        // Step 2: Use BRepOffsetAPI_MakeOffset on the projected planar wire
        // This correctly handles arc expansion!
        BRepOffsetAPI_MakeOffset offsetMaker(projectedWire, GeomAbs_Arc);
        offsetMaker.Perform(offset);  // Use the actual offset value, not scale factor
        
        if (offsetMaker.IsDone()) {
            TopoDS_Shape offsetResult = offsetMaker.Shape();
            TopoDS_Wire offsetWire;
            
            // Extract wire from result
            if (offsetResult.ShapeType() == TopAbs_WIRE) {
                offsetWire = TopoDS::Wire(offsetResult);
            }
            else {
                for (TopExp_Explorer exp(offsetResult, TopAbs_WIRE); exp.More(); exp.Next()) {
                    offsetWire = TopoDS::Wire(exp.Current());
                    break;
                }
            }
            
            if (!offsetWire.IsNull()) {
                // Create face from offset wire
                BRepBuilderAPI_MakeFace faceMaker(projPlane, offsetWire, Standard_True);
                if (faceMaker.IsDone()) {
                    return faceMaker.Face();
                }
            }
        }
        
        // Fallback: try negative offset (in case positive didn't work)
        offsetMaker.Perform(-offset);
        if (offsetMaker.IsDone()) {
            TopoDS_Shape offsetResult = offsetMaker.Shape();
            TopoDS_Wire offsetWire;
            
            if (offsetResult.ShapeType() == TopAbs_WIRE) {
                offsetWire = TopoDS::Wire(offsetResult);
            }
            else {
                for (TopExp_Explorer exp(offsetResult, TopAbs_WIRE); exp.More(); exp.Next()) {
                    offsetWire = TopoDS::Wire(exp.Current());
                    break;
                }
            }
            
            if (!offsetWire.IsNull()) {
                // Check if this is larger than original
                Bnd_Box origBox, offsetBox;
                BRepBndLib::Add(projectedWire, origBox);
                BRepBndLib::Add(offsetWire, offsetBox);
                
                double oxMin, oyMin, ozMin, oxMax, oyMax, ozMax;
                double nxMin, nyMin, nzMin, nxMax, nyMax, nzMax;
                origBox.Get(oxMin, oyMin, ozMin, oxMax, oyMax, ozMax);
                offsetBox.Get(nxMin, nyMin, nzMin, nxMax, nyMax, nzMax);
                
                double origSize = (oxMax - oxMin) + (oyMax - oyMin);
                double newSize = (nxMax - nxMin) + (nyMax - nyMin);
                
                if (newSize > origSize) {
                    BRepBuilderAPI_MakeFace faceMaker(projPlane, offsetWire, Standard_True);
                    if (faceMaker.IsDone()) {
                        return faceMaker.Face();
                    }
                }
            }
        }
        
        // Last resort: scale the projected wire
        gp_Trsf scaleTrsf;
        scaleTrsf.SetScale(center, scaleFactor);
        BRepBuilderAPI_Transform transformer(projectedWire, scaleTrsf, Standard_True);
        if (transformer.IsDone()) {
            TopoDS_Wire scaledWire = TopoDS::Wire(transformer.Shape());
            BRepBuilderAPI_MakeFace faceMaker(projPlane, scaledWire, Standard_True);
            if (faceMaker.IsDone()) {
                return faceMaker.Face();
            }
        }
        
        return face;
    }
    catch (...) {
        return face;
    }
}

TopoDS_Shape ClearanceVolume::extrudeOutline(const TopoDS_Shape& outline,
                                              const gp_Vec& direction,
                                              double depth)
{
    if (outline.IsNull() || depth < Precision::Confusion()) {
        return {};
    }
    
    try {
        gp_Vec extVec = direction.Normalized() * depth;
        
        BRepPrimAPI_MakePrism prismMaker(outline, extVec);
        if (prismMaker.IsDone()) {
            return prismMaker.Shape();
        }
    }
    catch (...) {
        // Extrusion failed
    }
    
    return {};
}

gp_Vec ClearanceVolume::getFaceNormal(const TopoDS_Face& face) const
{
    try {
        // Use BRepAdaptor_Surface with Standard_True to respect face orientation
        BRepAdaptor_Surface surface(face, Standard_True);
        
        // Get UV bounds
        double u1 = surface.FirstUParameter();
        double u2 = surface.LastUParameter();
        double v1 = surface.FirstVParameter();
        double v2 = surface.LastVParameter();
        
        // Sample at center of face
        double uMid = (u1 + u2) / 2.0;
        double vMid = (v1 + v2) / 2.0;
        
        // Get point and tangent vectors at center
        gp_Pnt pnt;
        gp_Vec d1u, d1v;
        surface.D1(uMid, vMid, pnt, d1u, d1v);
        
        // Normal is cross product of tangent vectors
        // This automatically respects face orientation due to BRepAdaptor_Surface
        gp_Vec normal = d1u.Crossed(d1v);
        
        if (normal.Magnitude() > Precision::Confusion()) {
            normal.Normalize();
            return normal;
        }
        
        // Fallback: try using surface normal from GeomLProp
        GeomLProp_SLProps props(surface.Surface().Surface(), uMid, vMid, 1, Precision::Confusion());
        if (props.IsNormalDefined()) {
            gp_Dir surfNormal = props.Normal();
            
            // Flip if face is reversed
            if (face.Orientation() == TopAbs_REVERSED) {
                surfNormal.Reverse();
            }
            
            return gp_Vec(surfNormal);
        }
    }
    catch (...) {
        // Failed to get normal
    }
    
    return gp_Vec(0, 0, 1); // Default to Z
}

bool ClearanceVolume::intersectsWith(const TopoDS_Shape& shape) const
{
    if (shape.IsNull()) {
        return false;
    }
    
    TopoDS_Shape clearance = this->Shape.getValue();
    if (clearance.IsNull()) {
        return false;
    }
    
    // Check bounding box intersection first (fast)
    Bnd_Box box1, box2;
    BRepBndLib::Add(clearance, box1);
    BRepBndLib::Add(shape, box2);
    
    return !box1.IsOut(box2);
}

TopoDS_Shape ClearanceVolume::getClearanceShape() const
{
    return this->Shape.getValue();
}

std::vector<ClearanceVolume*> ClearanceVolume::getAllInDocument(App::Document* doc)
{
    std::vector<ClearanceVolume*> result;
    
    if (!doc) {
        return result;
    }
    
    for (auto* obj : doc->getObjects()) {
        if (auto* cv = dynamic_cast<ClearanceVolume*>(obj)) {
            result.push_back(cv);
        }
    }
    
    return result;
}
