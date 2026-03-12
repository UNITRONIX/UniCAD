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
# include <BRepBuilderAPI_MakeFace.hxx>
# include <BRepBuilderAPI_MakeWire.hxx>
# include <BRepOffsetAPI_MakeOffset.hxx>
# include <BRepPrimAPI_MakePrism.hxx>
# include <BRepAdaptor_Surface.hxx>
# include <BRepAlgoAPI_Fuse.hxx>
# include <BRepBndLib.hxx>
# include <BRep_Tool.hxx>
# include <Bnd_Box.hxx>
# include <GeomAbs_SurfaceType.hxx>
# include <GeomLProp_SLProps.hxx>
# include <Geom_Surface.hxx>
# include <Geom_Plane.hxx>
# include <TopExp_Explorer.hxx>
# include <TopoDS.hxx>
# include <TopoDS_Wire.hxx>
# include <TopoDS_Face.hxx>
# include <TopoDS_Compound.hxx>
# include <gp_Pln.hxx>
# include <gp_Vec.hxx>
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
        // Get surface adaptor to extract plane information
        BRepAdaptor_Surface surfAdaptor(face);
        
        // Check if surface is planar
        if (surfAdaptor.GetType() != GeomAbs_Plane) {
            // For non-planar faces, we need different approach
            // For now, return original face
            return face;
        }
        
        gp_Pln plane = surfAdaptor.Plane();
        
        // Get the outer wire of the face
        TopoDS_Wire outerWire;
        for (TopExp_Explorer explorer(face, TopAbs_WIRE); explorer.More(); explorer.Next()) {
            outerWire = TopoDS::Wire(explorer.Current());
            break; // Take only the outer wire
        }
        
        if (outerWire.IsNull()) {
            return face;
        }
        
        // Create offset of the wire using the face plane context
        BRepOffsetAPI_MakeOffset offsetMaker(face);
        offsetMaker.Perform(offset);
        
        if (!offsetMaker.IsDone()) {
            return face;
        }
        
        TopoDS_Shape offsetResult = offsetMaker.Shape();
        
        // Extract offset wire from result - can be wire or compound
        TopoDS_Wire offsetWire;
        
        if (offsetResult.ShapeType() == TopAbs_WIRE) {
            offsetWire = TopoDS::Wire(offsetResult);
        }
        else if (offsetResult.ShapeType() == TopAbs_COMPOUND) {
            // Take first wire from compound
            for (TopExp_Explorer exp(offsetResult, TopAbs_WIRE); exp.More(); exp.Next()) {
                offsetWire = TopoDS::Wire(exp.Current());
                break;
            }
        }
        
        if (offsetWire.IsNull()) {
            return face;
        }
        
        // Create face from offset wire using the same plane
        BRepBuilderAPI_MakeFace faceMaker(plane, offsetWire, Standard_True);
        if (faceMaker.IsDone()) {
            return faceMaker.Face();
        }
        
        // Alternative: try creating face without specifying plane
        BRepBuilderAPI_MakeFace faceMaker2(offsetWire, Standard_True);
        if (faceMaker2.IsDone()) {
            return faceMaker2.Face();
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
        BRepAdaptor_Surface surface(face);
        
        // Get UV bounds using BRepAdaptor_Surface methods
        double u1 = surface.FirstUParameter();
        double u2 = surface.LastUParameter();
        double v1 = surface.FirstVParameter();
        double v2 = surface.LastVParameter();
        
        // Sample at center
        double uMid = (u1 + u2) / 2.0;
        double vMid = (v1 + v2) / 2.0;
        
        // Get normal at center point
        GeomLProp_SLProps props(surface.Surface().Surface(), uMid, vMid, 1, Precision::Confusion());
        if (props.IsNormalDefined()) {
            gp_Dir normal = props.Normal();
            
            // Flip if face is reversed
            if (face.Orientation() == TopAbs_REVERSED) {
                normal.Reverse();
            }
            
            return gp_Vec(normal);
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
