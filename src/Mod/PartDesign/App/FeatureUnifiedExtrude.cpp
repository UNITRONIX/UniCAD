// SPDX-License-Identifier: LGPL-2.1-or-later
// UniCAD by UNITRONIX â€” Fusion 360-style unified Extrude feature.

#include <Precision.hxx>

#include <App/Document.h>
#include <App/DocumentObject.h>
#include <Base/Console.h>

#include "FeatureUnifiedExtrude.h"
#include "Mod/Part/App/TopoShapeOpCode.h"

using namespace PartDesign;

/* TRANSLATOR PartDesign::Extrude */

const char* Extrude::TypeEnums[]
    = {"Length", "UpToLast", "UpToFirst", "UpToFace", "?TwoLengths", "UpToShape", nullptr};

const char* Extrude::OperationEnums[]
    = {"Join", "Cut", "Intersect", "NewBody", nullptr};

PROPERTY_SOURCE(PartDesign::Extrude, PartDesign::FeatureExtrude)

Extrude::Extrude()
{
    // Default to Additive; execute() adjusts based on Operation
    addSubType = FeatureAddSub::Additive;

    ADD_PROPERTY_TYPE(Operation, (0L), "Extrude", App::Prop_None,
        "Boolean operation: Join (fuse), Cut (subtract), Intersect, or NewBody");
    Operation.setEnums(OperationEnums);

    ADD_PROPERTY_TYPE(SideType, (0L), "Extrude", App::Prop_None, "Type of sides definition");
    ADD_PROPERTY_TYPE(Type, (0L), "Side1", App::Prop_None, "Extrude type for side 1");
    ADD_PROPERTY_TYPE(Type2, (0L), "Side2", App::Prop_None, "Extrude type for side 2");
    SideType.setEnums(SideTypesEnums);
    Type.setEnums(TypeEnums);
    Type2.setEnums(TypeEnums);

    ADD_PROPERTY_TYPE(Length, (10.0), "Side1", App::Prop_None, "Extrude length");
    ADD_PROPERTY_TYPE(Length2, (10.0), "Side2", App::Prop_None, "Extrude length in 2nd direction");
    ADD_PROPERTY_TYPE(UseCustomVector, (false), "Extrude", App::Prop_None,
        "Use custom vector for extrude direction");
    ADD_PROPERTY_TYPE(Direction, (Base::Vector3d(1.0, 1.0, 1.0)), "Extrude", App::Prop_None,
        "Extrude direction vector");
    ADD_PROPERTY_TYPE(ReferenceAxis, (nullptr), "Extrude", App::Prop_None,
        "Reference axis of direction");
    ADD_PROPERTY_TYPE(AlongSketchNormal, (true), "Extrude", App::Prop_None,
        "Measure extrude length along the sketch normal direction");
    ADD_PROPERTY_TYPE(UpToFace, (nullptr), "Side1", App::Prop_None,
        "Face where extrusion will end");
    ADD_PROPERTY_TYPE(UpToShape, (nullptr), "Side1", App::Prop_None,
        "Faces or shape(s) where extrusion will end");
    ADD_PROPERTY_TYPE(UpToFace2, (nullptr), "Side2", App::Prop_None,
        "Face where extrusion will end on side 2");
    ADD_PROPERTY_TYPE(UpToShape2, (nullptr), "Side2", App::Prop_None,
        "Faces or shape(s) where extrusion will end on side 2");
    ADD_PROPERTY_TYPE(Offset, (0.0), "Side1", App::Prop_None,
        "Offset from face where extrusion will end");
    ADD_PROPERTY_TYPE(Offset2, (0.0), "Side2", App::Prop_None,
        "Offset from face where extrusion will end on side 2");
    Offset.setConstraints(&signedLengthConstraint);
    Offset2.setConstraints(&signedLengthConstraint);
    ADD_PROPERTY_TYPE(TaperAngle, (0.0), "Side1", App::Prop_None, "Taper angle");
    TaperAngle.setConstraints(&floatAngle);
    ADD_PROPERTY_TYPE(TaperAngle2, (0.0), "Side2", App::Prop_None,
        "Taper angle for 2nd direction");
    TaperAngle2.setConstraints(&floatAngle);

    // Allow negative values for bidirectional extrusion
    Length.setConstraints(nullptr);
    Length2.setConstraints(nullptr);
}

short Extrude::mustExecute() const
{
    if (Operation.isTouched()) {
        return 1;
    }
    return FeatureExtrude::mustExecute();
}

App::DocumentObjectExecReturn* Extrude::execute()
{
    // UniCAD: Set addSubType based on Operation before building extrusion.
    // This controls the boolean operation in buildExtrusion().
    const char* op = Operation.getValueAsString();

    bool isIntersect = (strcmp(op, "Intersect") == 0);
    bool isNewBody = (strcmp(op, "NewBody") == 0);

    ExtrudeOptions options(ExtrudeOption::MakeFace | ExtrudeOption::MakeFuse);

    if (strcmp(op, "Cut") == 0) {
        addSubType = FeatureAddSub::Subtractive;
        // Direction reversal handled by getProfileNormal() override (like Pocket).
    }
    else if (isIntersect || isNewBody) {
        // For Intersect and NewBody: build extrusion without boolean (no MakeFuse).
        // We'll handle the boolean (or lack thereof) manually after.
        addSubType = FeatureAddSub::Additive;
        options = ExtrudeOptions(ExtrudeOption::MakeFace);
    }
    else {
        // "Join" â€” standard Additive with Fuse
        addSubType = FeatureAddSub::Additive;
    }

    auto* ret = buildExtrusion(options);
    if (ret != App::DocumentObject::StdReturn) {
        return ret;
    }

    // Post-processing for Intersect and NewBody
    if (isIntersect || isNewBody) {
        TopoShape base;
        try {
            base = getBaseTopoShape();
        }
        catch (const Base::Exception&) {
            // No base â€” shape is already correct (standalone extrusion)
        }

        if (!base.isNull() && isIntersect) {
            // Intersect: Common boolean between base and extrusion result
            TopoShape prism = this->Shape.getShape();
            TopoShape result(0, getDocument()->getStringHasher());
            try {
                result.makeElementBoolean(Part::OpCodes::Common, {base, prism});
            }
            catch (Standard_Failure&) {
                return new App::DocumentObjectExecReturn(
                    QT_TRANSLATE_NOOP("Exception", "Intersect with base feature failed"));
            }
            auto solid = getSolid(result);
            if (solid.isNull()) {
                return new App::DocumentObjectExecReturn(
                    QT_TRANSLATE_NOOP("Exception", "Resulting shape is not a solid"));
            }
            this->rawShape = result;
            result = refineShapeIfActive(result);
            this->Shape.setValue(getSolid(result));
        }
        // NewBody: keep the standalone shape as-is (no boolean with base)
    }

    return App::DocumentObject::StdReturn;
}

Base::Vector3d Extrude::getProfileNormal() const
{
    Base::Vector3d normal = FeatureExtrude::getProfileNormal();

    // For Cut operation, reverse the normal direction (same as Pocket behavior)
    if (strcmp(Operation.getValueAsString(), "Cut") == 0) {
        return normal * -1;
    }
    return normal;
}
