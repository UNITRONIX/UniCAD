// SPDX-License-Identifier: LGPL-2.1-or-later
// FusionCAD by UNITRONIX — Replace Face feature (direct edit).
// Replaces selected faces on a solid with a target surface using
// shell reconstruction via BRepBuilderAPI_Sewing.

#include "PreCompiled.h"
#ifndef _PreComp_
#include <BRepBuilderAPI_MakeSolid.hxx>
#include <BRepBuilderAPI_Sewing.hxx>
#include <Precision.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shell.hxx>
#include <TopoDS_Solid.hxx>
#endif

#include "FeatureReplaceFace.h"

using namespace PartDesign;

PROPERTY_SOURCE(PartDesign::ReplaceFace, PartDesign::DressUp)

ReplaceFace::ReplaceFace()
{
    ADD_PROPERTY_TYPE(TargetFace, (nullptr), "ReplaceFace", App::Prop_None,
        "The replacement surface / face");
}

short ReplaceFace::mustExecute() const
{
    if (TargetFace.isTouched()) {
        return 1;
    }
    return DressUp::mustExecute();
}

App::DocumentObjectExecReturn* ReplaceFace::execute()
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

    // Resolve target face
    App::DocumentObject* targetObj = TargetFace.getValue();
    const std::vector<std::string>& targetSubs = TargetFace.getSubValues(true);

    if (!targetObj || targetSubs.empty()) {
        return new App::DocumentObjectExecReturn(
            QT_TRANSLATE_NOOP("Exception", "No replacement face selected"));
    }

    this->Placement.setValue(Base::Placement());

    try {
        // Collect source faces to replace
        std::vector<TopoDS_Face> sourceFaces;
        for (const auto& sub : subStrings) {
            TopoDS_Shape face;
            try {
                face = baseShape.getSubShape(sub.c_str());
            }
            catch (...) {
            }
            if (face.IsNull() || face.ShapeType() != TopAbs_FACE) {
                return new App::DocumentObjectExecReturn(
                    QT_TRANSLATE_NOOP("Exception", "Invalid source face reference"));
            }
            sourceFaces.push_back(TopoDS::Face(face));
        }

        // Get the target replacement face from the linked object
        auto targetProp = targetObj->getPropertyByName("Shape");
        if (!targetProp || !targetProp->isDerivedFrom(App::PropertyComplexGeoData::getClassTypeId())) {
            return new App::DocumentObjectExecReturn(
                QT_TRANSLATE_NOOP("Exception", "Target object has no valid shape"));
        }
        auto* targetShapeProp = static_cast<Part::PropertyPartShape*>(targetProp);
        Part::TopoShape targetTopoShape = targetShapeProp->getShape();

        TopoDS_Face replacementFace;
        for (const auto& tsub : targetSubs) {
            TopoDS_Shape tface;
            try {
                tface = targetTopoShape.getSubShape(tsub.c_str());
            }
            catch (...) {
            }
            if (!tface.IsNull() && tface.ShapeType() == TopAbs_FACE) {
                replacementFace = TopoDS::Face(tface);
                break;
            }
        }

        if (replacementFace.IsNull()) {
            return new App::DocumentObjectExecReturn(
                QT_TRANSLATE_NOOP("Exception", "Invalid replacement face reference"));
        }

        // Shell reconstruction: Remove source faces, insert target face, sew
        double tol = Precision::Confusion();
        BRepBuilderAPI_Sewing sew(tol);

        // Add all faces from the solid EXCEPT source faces
        for (TopExp_Explorer exp(baseShape.getShape(), TopAbs_FACE); exp.More(); exp.Next()) {
            TopoDS_Face currentFace = TopoDS::Face(exp.Current());
            bool isSource = false;
            for (const auto& src : sourceFaces) {
                if (currentFace.IsSame(src)) {
                    isSource = true;
                    break;
                }
            }
            if (!isSource) {
                sew.Add(currentFace);
            }
        }

        // Add the replacement face
        sew.Add(replacementFace);

        sew.Perform();
        TopoDS_Shape sewedShape = sew.SewedShape();

        if (sewedShape.IsNull()) {
            return new App::DocumentObjectExecReturn(
                QT_TRANSLATE_NOOP("Exception", "Sewing failed during face replacement"));
        }

        // Try to extract or build a solid from the sewed result
        TopoDS_Solid resultSolid;

        // Check if the result is already a solid
        if (sewedShape.ShapeType() == TopAbs_SOLID) {
            resultSolid = TopoDS::Solid(sewedShape);
        }
        else {
            // Try to make a solid from the shells
            BRepBuilderAPI_MakeSolid solidMaker;
            for (TopExp_Explorer shellExp(sewedShape, TopAbs_SHELL); shellExp.More();
                 shellExp.Next()) {
                solidMaker.Add(TopoDS::Shell(shellExp.Current()));
            }
            if (solidMaker.IsDone()) {
                resultSolid = solidMaker.Solid();
            }
        }

        if (resultSolid.IsNull()) {
            return new App::DocumentObjectExecReturn(
                QT_TRANSLATE_NOOP("Exception",
                    "Could not build a solid from the replaced faces"));
        }

        Part::TopoShape resultShape(resultSolid);
        auto solid = getSolid(resultShape);
        if (solid.isNull()) {
            return new App::DocumentObjectExecReturn(
                QT_TRANSLATE_NOOP("Exception", "Resulting shape is not a solid"));
        }

        if (!isSingleSolidRuleSatisfied(resultShape.getShape())) {
            return new App::DocumentObjectExecReturn(QT_TRANSLATE_NOOP(
                "Exception",
                "Result has multiple solids: enable 'Allow Compound' in the active body."));
        }

        this->Shape.setValue(getSolid(resultShape));
        return App::DocumentObject::StdReturn;
    }
    catch (Standard_Failure& e) {
        return new App::DocumentObjectExecReturn(e.GetMessageString());
    }
    catch (Base::Exception& e) {
        return new App::DocumentObjectExecReturn(e.what());
    }
}
