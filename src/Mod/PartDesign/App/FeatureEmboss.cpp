// SPDX-License-Identifier: LGPL-2.1-or-later
// UniCAD: Emboss/Deboss feature â€” project sketch onto face and extrude

#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <BRepProj_Projection.hxx>
#include <BRep_Tool.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Wire.hxx>
#include <TopExp_Explorer.hxx>
#include <gp_Vec.hxx>

#include <Base/Exception.h>
#include <Mod/Part/App/TopoShape.h>

#include "FeatureEmboss.h"

using namespace PartDesign;

PROPERTY_SOURCE(PartDesign::Emboss, PartDesign::ProfileBased)

Emboss::Emboss()
{
    addSubType = FeatureAddSub::Additive;

    ADD_PROPERTY_TYPE(Depth, (1.0), "Emboss", App::Prop_None, "Depth of the emboss/deboss");
    Depth.setUnit(Base::Unit::Length);

    ADD_PROPERTY_TYPE(
        Reversed,
        (false),
        "Emboss",
        App::Prop_None,
        "If true, create a deboss (cut) instead of an emboss (fuse)"
    );

    ADD_PROPERTY_TYPE(TargetFace, (nullptr), "Emboss", App::Prop_None, "Target face for projection");
}

short Emboss::mustExecute() const
{
    if (Depth.isTouched() || Reversed.isTouched() || TargetFace.isTouched()) {
        return 1;
    }
    return ProfileBased::mustExecute();
}

App::DocumentObjectExecReturn* Emboss::execute()
{
    // Get base shape
    Part::TopoShape baseShape;
    try {
        baseShape = getBaseTopoShape();
    }
    catch (Base::Exception& e) {
        return new App::DocumentObjectExecReturn(e.what());
    }

    // Get sketch wires
    Part::TopoShape profileShape;
    try {
        profileShape = getTopoShapeVerifiedFace();
    }
    catch (Base::Exception& e) {
        return new App::DocumentObjectExecReturn(e.what());
    }

    double depth = Depth.getValue();
    if (depth <= 0) {
        return new App::DocumentObjectExecReturn(
            QT_TRANSLATE_NOOP("Exception", "Emboss depth must be greater than zero")
        );
    }

    bool isDeboss = Reversed.getValue();

    // Get the face normal for extrusion direction
    gp_Dir normal;
    try {
        auto faceShape = getTopoShapeVerifiedFace();
        TopoDS_Face face;
        // Get the first face
        for (TopExp_Explorer exp(faceShape.getShape(), TopAbs_FACE); exp.More(); exp.Next()) {
            face = TopoDS::Face(exp.Current());
            break;
        }
        if (face.IsNull()) {
            return new App::DocumentObjectExecReturn(
                QT_TRANSLATE_NOOP("Exception", "Cannot determine face normal for emboss direction")
            );
        }

        BRepAdaptor_Surface adapt(face);
        gp_Pnt pnt;
        gp_Vec u, v;
        adapt.D1(
            (adapt.FirstUParameter() + adapt.LastUParameter()) / 2.0,
            (adapt.FirstVParameter() + adapt.LastVParameter()) / 2.0,
            pnt,
            u,
            v
        );
        gp_Vec n = u.Crossed(v);
        if (n.Magnitude() < 1e-10) {
            return new App::DocumentObjectExecReturn(
                QT_TRANSLATE_NOOP("Exception", "Face normal is degenerate")
            );
        }
        normal = gp_Dir(n);
    }
    catch (Standard_Failure& e) {
        return new App::DocumentObjectExecReturn(e.GetMessageString());
    }

    // Create the prism (extrusion along face normal)
    gp_Vec prismVec(normal);
    prismVec.Multiply(isDeboss ? -depth : depth);

    try {
        TopoDS_Shape prism = BRepPrimAPI_MakePrism(profileShape.getShape(), prismVec).Shape();

        if (prism.IsNull()) {
            return new App::DocumentObjectExecReturn(
                QT_TRANSLATE_NOOP("Exception", "Failed to create emboss prism")
            );
        }

        // Fuse or cut with base
        Part::TopoShape result(0);
        if (isDeboss) {
            addSubType = FeatureAddSub::Subtractive;
            BRepAlgoAPI_Cut mkCut(baseShape.getShape(), prism);
            if (!mkCut.IsDone()) {
                return new App::DocumentObjectExecReturn(
                    QT_TRANSLATE_NOOP("Exception", "Deboss boolean cut failed")
                );
            }
            result.setShape(mkCut.Shape());
        }
        else {
            addSubType = FeatureAddSub::Additive;
            BRepAlgoAPI_Fuse mkFuse(baseShape.getShape(), prism);
            if (!mkFuse.IsDone()) {
                return new App::DocumentObjectExecReturn(
                    QT_TRANSLATE_NOOP("Exception", "Emboss boolean fuse failed")
                );
            }
            result.setShape(mkFuse.Shape());
        }

        if (result.isNull()) {
            return new App::DocumentObjectExecReturn(
                QT_TRANSLATE_NOOP("Exception", "Emboss/Deboss produced an empty shape")
            );
        }

        this->Shape.setValue(result);
    }
    catch (Standard_Failure& e) {
        return new App::DocumentObjectExecReturn(e.GetMessageString());
    }

    return App::DocumentObject::StdReturn;
}
