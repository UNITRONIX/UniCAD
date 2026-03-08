// SPDX-License-Identifier: LGPL-2.1-or-later
// UniCAD by UNITRONIX â€” Fusion 360-style unified Loft feature.

#include "FeatureUnifiedLoft.h"

using namespace PartDesign;

/* TRANSLATOR PartDesign::UnifiedLoft */

const char* UnifiedLoft::OperationEnums[]
    = {"Join", "Cut", "Intersect", "NewBody", nullptr};

PROPERTY_SOURCE(PartDesign::UnifiedLoft, PartDesign::Loft)

UnifiedLoft::UnifiedLoft()
{
    addSubType = FeatureAddSub::Additive;

    ADD_PROPERTY_TYPE(Operation, (0L), "Loft", App::Prop_None,
        "Boolean operation: Join (fuse), Cut (subtract), Intersect, or NewBody");
    Operation.setEnums(OperationEnums);
}

short UnifiedLoft::mustExecute() const
{
    if (Operation.isTouched()) {
        return 1;
    }
    return Loft::mustExecute();
}

App::DocumentObjectExecReturn* UnifiedLoft::execute()
{
    // UniCAD: Set addSubType based on Operation before calling Loft::execute().
    // Loft::execute() uses getAddSubType() for the boolean decision.
    const char* op = Operation.getValueAsString();

    if (strcmp(op, "Cut") == 0) {
        addSubType = FeatureAddSub::Subtractive;
    }
    else {
        addSubType = FeatureAddSub::Additive;
    }

    // Loft::execute() reads getAddSubType() to decide Fuse vs Cut
    return Loft::execute();
}
