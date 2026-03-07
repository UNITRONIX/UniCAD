// SPDX-License-Identifier: LGPL-2.1-or-later
// FusionCAD by UNITRONIX — Fusion 360-style unified Sweep feature.

#include "FeatureUnifiedSweep.h"

using namespace PartDesign;

/* TRANSLATOR PartDesign::UnifiedSweep */

const char* UnifiedSweep::OperationEnums[]
    = {"Join", "Cut", "Intersect", "NewBody", nullptr};

PROPERTY_SOURCE(PartDesign::UnifiedSweep, PartDesign::Pipe)

UnifiedSweep::UnifiedSweep()
{
    addSubType = FeatureAddSub::Additive;

    ADD_PROPERTY_TYPE(Operation, (0L), "Sweep", App::Prop_None,
        "Boolean operation: Join (fuse), Cut (subtract), Intersect, or NewBody");
    Operation.setEnums(OperationEnums);
}

short UnifiedSweep::mustExecute() const
{
    if (Operation.isTouched()) {
        return 1;
    }
    return Pipe::mustExecute();
}

App::DocumentObjectExecReturn* UnifiedSweep::execute()
{
    // FusionCAD: Set addSubType based on Operation before calling Pipe::execute().
    // Pipe::execute() uses getAddSubType() for the boolean decision.
    const char* op = Operation.getValueAsString();

    if (strcmp(op, "Cut") == 0) {
        addSubType = FeatureAddSub::Subtractive;
    }
    else {
        addSubType = FeatureAddSub::Additive;
    }

    // Pipe::execute() reads getAddSubType() to decide Fuse vs Cut
    return Pipe::execute();
}
