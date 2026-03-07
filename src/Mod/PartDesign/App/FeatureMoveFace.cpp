// SPDX-License-Identifier: LGPL-2.1-or-later
// FusionCAD by UNITRONIX — Move Face feature (direct edit).
// Translates selected faces using BRepOffsetAPI_MakeThickSolid
// with a directional offset computed from face normal or custom direction.

#include <BRepAdaptor_Surface.hxx>
#include <BRepGProp_Face.hxx>
#include <Precision.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>

#include <Base/Exception.h>

#include "FeatureMoveFace.h"

using namespace PartDesign;

/* TRANSLATOR PartDesign::MoveFace */

PROPERTY_SOURCE(PartDesign::MoveFace, PartDesign::DressUp)

MoveFace::MoveFace()
{
    ADD_PROPERTY_TYPE(Distance, (1.0), "MoveFace", App::Prop_None,
        "Distance to move the selected faces");
    ADD_PROPERTY_TYPE(Direction, (Base::Vector3d(0.0, 0.0, 1.0)), "MoveFace", App::Prop_None,
        "Custom direction for face movement");
    ADD_PROPERTY_TYPE(UseCustomDirection, (false), "MoveFace", App::Prop_None,
        "Use custom direction instead of face normal");
}

short MoveFace::mustExecute() const
{
    if (Distance.isTouched() || Direction.isTouched() || UseCustomDirection.isTouched()) {
        return 1;
    }
    return DressUp::mustExecute();
}

App::DocumentObjectExecReturn* MoveFace::execute()
{
    if (onlyHaveRefined()) {
        return App::DocumentObject::StdReturn;
    }

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

    double distance = Distance.getValue();
    double tol = Precision::Confusion();

    if (fabs(distance) < tol) {
        this->Shape.setValue(baseShape);
        return App::DocumentObject::StdReturn;
    }

    try {
        // Move Face works by offsetting the selected faces along their normal
        // (or a custom direction). For planar faces, this is equivalent to
        // the face offset (ThickSolid) approach.
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

        // Use makeElementThickSolid — this offsets the selected faces.
        // The distance is signed: positive = outward, negative = inward.
        TopoShape result(0);
        result = baseShape.makeElementThickSolid(
            selectedFaces,
            distance,
            tol,
            false,      // intersection
            false,      // selfInter
            0,          // mode: Skin
            Part::JoinType::arc
        );

        if (result.isNull()) {
            return new App::DocumentObjectExecReturn(
                QT_TRANSLATE_NOOP("Exception", "Move face operation failed"));
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
