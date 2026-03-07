// SPDX-License-Identifier: LGPL-2.1-or-later
// FusionCAD by UNITRONIX — Fusion 360-style unified Revolve feature.

#include <BRepPrimAPI_MakeRevol.hxx>
#include <BRepFeat_MakeRevol.hxx>
#include <gp_Lin.hxx>
#include <Precision.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>

#include <App/Document.h>
#include <Base/Exception.h>
#include <Base/Tools.h>

#include "FeatureUnifiedRevolve.h"
#include "Mod/Part/App/TopoShapeOpCode.h"

using namespace PartDesign;

/* TRANSLATOR PartDesign::UnifiedRevolve */

const char* UnifiedRevolve::OperationEnums[]
    = {"Join", "Cut", "Intersect", "NewBody", nullptr};

PROPERTY_SOURCE(PartDesign::UnifiedRevolve, PartDesign::Revolution)

UnifiedRevolve::UnifiedRevolve()
{
    // Default to Additive; execute() adjusts based on Operation
    addSubType = FeatureAddSub::Additive;

    ADD_PROPERTY_TYPE(Operation, (0L), "Revolve", App::Prop_None,
        "Boolean operation: Join (fuse), Cut (subtract), Intersect, or NewBody");
    Operation.setEnums(OperationEnums);
}

short UnifiedRevolve::mustExecute() const
{
    if (Operation.isTouched()) {
        return 1;
    }
    return Revolution::mustExecute();
}

App::DocumentObjectExecReturn* UnifiedRevolve::execute()
{
    if (onlyHaveRefined()) {
        return App::DocumentObject::StdReturn;
    }

    // Determine boolean operation from Operation property
    const char* op = Operation.getValueAsString();
    bool isCut = (strcmp(op, "Cut") == 0);
    bool isIntersect = (strcmp(op, "Intersect") == 0);
    bool isNewBody = (strcmp(op, "NewBody") == 0);

    if (isCut) {
        addSubType = FeatureAddSub::Subtractive;
    }
    else {
        addSubType = FeatureAddSub::Additive;
    }

    // --- Revolution logic (adapted from Revolution::execute()) ---

    constexpr double maxDegree = 360.0;
    auto method = methodFromString(Type.getValueAsString());

    double angleDeg = Angle.getValue();
    if (angleDeg > maxDegree) {
        return new App::DocumentObjectExecReturn(
            QT_TRANSLATE_NOOP("Exception", "Angle of revolution too large"));
    }

    double angle = Base::toRadians<double>(angleDeg);
    if (angle < Precision::Angular() && method == RevolMethod::Angle) {
        return new App::DocumentObjectExecReturn(
            QT_TRANSLATE_NOOP("Exception", "Angle of revolution too small"));
    }

    double angle2 = Base::toRadians(Angle2.getValue());
    if (std::fabs(angle + angle2) < Precision::Angular() && method == RevolMethod::TwoAngles) {
        return new App::DocumentObjectExecReturn(
            QT_TRANSLATE_NOOP("Exception", "Angles of revolution nullify each other"));
    }

    TopoShape sketchshape = getTopoShapeVerifiedFace();

    TopoShape base;
    try {
        base = getBaseTopoShape();
    }
    catch (const Base::Exception&) {
    }

    try {
        updateAxis();
    }
    catch (const Base::Exception& e) {
        return new App::DocumentObjectExecReturn(e.what());
    }

    try {
        Base::Vector3d b = Base.getValue();
        gp_Pnt pnt(b.x, b.y, b.z);
        Base::Vector3d v = Axis.getValue();

        if (v.IsNull()) {
            return new App::DocumentObjectExecReturn(
                QT_TRANSLATE_NOOP("Exception", "Reference axis is invalid"));
        }

        gp_Dir dir(v.x, v.y, v.z);

        if (sketchshape.isNull()) {
            return new App::DocumentObjectExecReturn(
                QT_TRANSLATE_NOOP("Exception", "Creating a face from sketch failed"));
        }

        this->positionByPrevious();
        auto invObjLoc = getLocation().Inverted();
        pnt.Transform(invObjLoc.Transformation());
        dir.Transform(invObjLoc.Transformation());
        base.move(invObjLoc);
        sketchshape.move(invObjLoc);

        TopExp_Explorer xp;
        xp.Init(sketchshape.getShape(), TopAbs_FACE);
        for (; xp.More(); xp.Next()) {
            if (checkLineCrossesFace(gp_Lin(pnt, dir), TopoDS::Face(xp.Current()))) {
                return new App::DocumentObjectExecReturn(
                    QT_TRANSLATE_NOOP("Exception", "Revolve axis intersects the sketch"));
            }
        }

        TopoShape result(0);
        TopoShape supportface(0);
        try {
            supportface = getSupportFace();
        }
        catch (...) {
        }
        supportface.move(invObjLoc);

        if (method == RevolMethod::ToFace || method == RevolMethod::ToFirst) {
            TopoShape upToFace;
            if (method == RevolMethod::ToFace) {
                getUpToFaceFromLinkSub(upToFace, UpToFace);
                upToFace.move(invObjLoc);
            }
            else {
                return new App::DocumentObjectExecReturn("Revolve up to first is not yet supported");
            }

            if (Reversed.getValue()) {
                dir.Reverse();
            }

            TopExp_Explorer Ex(supportface.getShape(), TopAbs_WIRE);
            if (!Ex.More()) {
                supportface = TopoDS_Face();
            }

            // FusionCAD: Use CutFromBase for Cut, FuseWithBase for Join
            auto revolMode = isCut ? Part::RevolMode::CutFromBase : Part::RevolMode::FuseWithBase;

            try {
                result = base.makeElementRevolution(
                    base,
                    TopoDS::Face(sketchshape.getShape()),
                    gp_Ax1(pnt, dir),
                    TopoDS::Face(supportface.getShape()),
                    TopoDS::Face(upToFace.getShape()),
                    nullptr,
                    revolMode,
                    Standard_True);
            }
            catch (Standard_Failure&) {
                return new App::DocumentObjectExecReturn("Could not revolve the sketch!");
            }
        }
        else {
            bool midplane = Midplane.getValue();
            bool reversed = Reversed.getValue();
            generateRevolution(
                result, sketchshape, gp_Ax1(pnt, dir),
                angle, angle2, midplane, reversed, method);
        }

        if (!result.isNull()) {
            this->rawShape = result;
            result = refineShapeIfActive(result);
            this->AddSubShape.setValue(result);

            if (!base.isNull() && !isNewBody) {
                // FusionCAD: Boolean operation based on Operation property
                const char* maker;
                if (isCut) {
                    maker = Part::OpCodes::Cut;
                }
                else if (isIntersect) {
                    maker = Part::OpCodes::Common;
                }
                else {
                    maker = Part::OpCodes::Fuse;
                }

                TopoShape boolOp(0, getDocument()->getStringHasher());
                try {
                    boolOp.makeElementBoolean(maker, {base, result});
                }
                catch (Standard_Failure&) {
                    return new App::DocumentObjectExecReturn(
                        QT_TRANSLATE_NOOP("Exception", "Failed to perform boolean operation"));
                }

                this->rawShape = boolOp;
                result = refineShapeIfActive(boolOp);
            }

            if (!isSingleSolidRuleSatisfied(result.getShape())) {
                return new App::DocumentObjectExecReturn(QT_TRANSLATE_NOOP(
                    "Exception",
                    "Result has multiple solids: enable 'Allow Compound' in the active body."));
            }
            result = getSolid(result);
            this->Shape.setValue(result);
        }
        else {
            return new App::DocumentObjectExecReturn(
                QT_TRANSLATE_NOOP("Exception", "Could not revolve the sketch!"));
        }

        updateProperties(method);
        return App::DocumentObject::StdReturn;
    }
    catch (Standard_Failure& e) {
        if (std::string(e.GetMessageString()) == "TopoDS::Face") {
            return new App::DocumentObjectExecReturn(QT_TRANSLATE_NOOP(
                "Exception",
                "Could not create face from sketch.\n"
                "Intersecting sketch entities in a sketch are not allowed."));
        }
        return new App::DocumentObjectExecReturn(e.GetMessageString());
    }
    catch (Base::Exception& e) {
        return new App::DocumentObjectExecReturn(e.what());
    }
}
