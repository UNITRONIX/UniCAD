// SPDX-License-Identifier: LGPL-2.1-or-later
// FusionCAD by UNITRONIX — Delete Face + Heal feature (direct edit).
// Uses BRepAlgoAPI_Defeaturing to remove selected faces and heal the solid.

#include "PreCompiled.h"
#ifndef _PreComp_
#include <BRepAlgoAPI_Defeaturing.hxx>
#include <Precision.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#endif

#include "FeatureDeleteFace.h"

using namespace PartDesign;

PROPERTY_SOURCE(PartDesign::DeleteFace, PartDesign::DressUp)

DeleteFace::DeleteFace() = default;

short DeleteFace::mustExecute() const
{
    return DressUp::mustExecute();
}

App::DocumentObjectExecReturn* DeleteFace::execute()
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

    try {
        BRepAlgoAPI_Defeaturing defeat;
        defeat.SetRunParallel(true);
        defeat.SetShape(baseShape.getShape());

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
            defeat.AddFaceToRemove(TopoDS::Face(face));
        }

        defeat.Build();
        if (!defeat.IsDone()) {
            return new App::DocumentObjectExecReturn(
                QT_TRANSLATE_NOOP("Exception",
                    "Failed to remove faces and heal the solid"));
        }

        TopoDS_Shape result = defeat.Shape();
        if (result.IsNull()) {
            return new App::DocumentObjectExecReturn(
                QT_TRANSLATE_NOOP("Exception", "Delete face result is null"));
        }

        Part::TopoShape resultShape(result);
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
