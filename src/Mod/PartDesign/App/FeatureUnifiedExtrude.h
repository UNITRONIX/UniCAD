// SPDX-License-Identifier: LGPL-2.1-or-later
// FusionCAD by UNITRONIX — Fusion 360-style unified Extrude feature.
// Single command for Join (Fuse), Cut, Intersect, and NewBody operations.

#ifndef PARTDESIGN_UNIFIED_EXTRUDE_H
#define PARTDESIGN_UNIFIED_EXTRUDE_H

#include "FeatureExtrude.h"

namespace PartDesign
{

class PartDesignExport Extrude : public FeatureExtrude
{
    PROPERTY_HEADER_WITH_OVERRIDE(PartDesign::Extrude);

public:
    Extrude();

    // FusionCAD: Operation type — Fusion 360-style result type
    // "Join" = Fuse with body (like Pad)
    // "Cut" = Subtract from body (like Pocket)
    // "Intersect" = Common with body
    // "NewBody" = Create separate body
    App::PropertyEnumeration Operation;

    App::DocumentObjectExecReturn* execute() override;

    short mustExecute() const override;

    const char* getViewProviderName() const override
    {
        return "PartDesignGui::ViewProviderUnifiedExtrude";
    }

    Base::Vector3d getProfileNormal() const override;

private:
    static const char* TypeEnums[];
    static const char* OperationEnums[];
};

}  // namespace PartDesign

#endif  // PARTDESIGN_UNIFIED_EXTRUDE_H
