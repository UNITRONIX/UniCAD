// SPDX-License-Identifier: LGPL-2.1-or-later
// UniCAD by UNITRONIX â€” Fusion 360-style unified Revolve feature.
// Single command for Join (Fuse), Cut, Intersect, and NewBody operations.

#ifndef PARTDESIGN_UNIFIED_REVOLVE_H
#define PARTDESIGN_UNIFIED_REVOLVE_H

#include "FeatureRevolution.h"

namespace PartDesign
{

class PartDesignExport UnifiedRevolve : public Revolution
{
    PROPERTY_HEADER_WITH_OVERRIDE(PartDesign::UnifiedRevolve);

public:
    UnifiedRevolve();

    // UniCAD: Operation type â€” Fusion 360-style result type
    App::PropertyEnumeration Operation;

    App::DocumentObjectExecReturn* execute() override;
    short mustExecute() const override;

    const char* getViewProviderName() const override
    {
        return "PartDesignGui::ViewProviderUnifiedRevolve";
    }

private:
    static const char* OperationEnums[];
};

}  // namespace PartDesign

#endif  // PARTDESIGN_UNIFIED_REVOLVE_H
