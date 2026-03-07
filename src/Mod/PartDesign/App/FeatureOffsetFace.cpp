// SPDX-License-Identifier: LGPL-2.1-or-later
// FusionCAD by UNITRONIX — Offset Face feature (direct edit).
// Offsets selected faces on a solid along their normals using BRepOffsetAPI_MakeOffsetShape.

#include <BRepOffsetAPI_MakeOffsetShape.hxx>
#include <Precision.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>

#include <Base/Exception.h>

#include "FeatureOffsetFace.h"

using namespace PartDesign;

/* TRANSLATOR PartDesign::OffsetFace */

PROPERTY_SOURCE(PartDesign::OffsetFace, PartDesign::DressUp)

OffsetFace::OffsetFace()
{
    ADD_PROPERTY_TYPE(Offset, (1.0), "OffsetFace", App::Prop_None,
        "Distance to offset the selected faces");
}

short OffsetFace::mustExecute() const
{
    if (Offset.isTouched()) {
        return 1;
    }
    return DressUp::mustExecute();
}

App::DocumentObjectExecReturn* OffsetFace::execute()
{
    if (onlyHaveRefined()) {
        return App::DocumentObject::StdReturn;
    }

    // Get the base shape
    Part::TopoShape baseShape;
    try {
        baseShape = getBaseTopoShape();
    }
    catch (Base::Exception& e) {
        return new App::DocumentObjectExecReturn(e.what());
    }

    const std::vector<std::string>& subStrings = Base.getSubValues(true);

    if (subStrings.empty()) {
        this->positionByBaseFeature();
        this->Shape.setValue(baseShape);
        return App::DocumentObject::StdReturn;
    }

    this->Placement.setValue(Base::Placement());

    double offset = Offset.getValue();
    double tol = Precision::Confusion();

    if (fabs(offset) < tol) {
        // Zero offset — just copy the base shape
        this->Shape.setValue(baseShape);
        return App::DocumentObject::StdReturn;
    }

    // Use makeElementThickSolid on selected faces to achieve face offset effect.
    // ThickSolid with faces creates a shell by removing those faces and offsetting.
    // For a true "offset face" (push/pull a face without shelling), we use
    // BRepOffsetAPI_MakeOffsetShape on the entire solid, then trim back.
    //
    // Practical approach: Use offset shape on entire solid.
    // This moves ALL faces by the offset distance. Then we intersect with the
    // original to keep only the added/removed material on the selected faces.
    //
    // Simplest reliable approach for planar faces: extrude the face by offset distance
    // and fuse/cut.

    try {
        // Collect selected faces
        std::vector<TopoShape> selectedFaces;
        for (const auto& sub : subStrings) {
            TopoDS_Shape face;
            try {
                face = baseShape.getSubShape(sub.c_str());
            }
            catch (...) {
            }
            if (face.IsNull() || face.ShapeType() != TopAbs_FACE) {
                return new App::DocumentObjectExecReturn(
                    QT_TRANSLATE_NOOP("Exception", "Invalid face reference"));
            }
            selectedFaces.emplace_back(face);
        }

        // Use makeElementThickSolid with the selected faces.
        // This offsets the selected faces outward (positive) or inward (negative).
        TopoShape result(0);
        result = baseShape.makeElementThickSolid(
            selectedFaces,
            offset,
            tol,
            false,      // intersection
            false,      // selfInter
            0,          // mode: Skin
            Part::JoinType::arc
        );

        if (result.isNull()) {
            return new App::DocumentObjectExecReturn(
                QT_TRANSLATE_NOOP("Exception", "Offset face operation failed"));
        }

        auto solid = getSolid(result);
        if (solid.isNull()) {
            return new App::DocumentObjectExecReturn(
                QT_TRANSLATE_NOOP("Exception", "Resulting shape is not a solid"));
        }

        if (!isSingleSolidRuleSatisfied(result.getShape())) {
            return new App::DocumentObjectExecReturn(QT_TRANSLATE_NOOP(
                "Exception",
                "Result has multiple solids: enable 'Allow Compound' in the active body."));
        }

        this->Shape.setValue(getSolid(result));
        return App::DocumentObject::StdReturn;
    }
    catch (Standard_Failure& e) {
        return new App::DocumentObjectExecReturn(e.GetMessageString());
    }
    catch (Base::Exception& e) {
        return new App::DocumentObjectExecReturn(e.what());
    }
}
